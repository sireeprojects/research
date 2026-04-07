#include <rte_bus.h>
#include <rte_dev.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <unistd.h>

#include <csignal>
#include <iostream>

static constexpr uint32_t MBUF_POOL_SIZE = 8191;
static constexpr uint32_t MBUF_CACHE_SIZE = 256;
static constexpr uint16_t PKT_BURST_SZ = 32;
static constexpr uint16_t PRIV_DATA_SIZE = 48;
static constexpr uint16_t VIRTIO_RXQ = 0;
static constexpr uint16_t VIRTIO_TXQ = 1;
static constexpr uint32_t RING_SIZE = 4096;
static constexpr uint32_t MAX_ENQUEUE_RETRIES = 1000;

static bool keep_running = true;
void signal_handler(int s) { keep_running = false; }

int main(int argc, char* argv[]) {
   int ret = rte_eal_init(argc, argv);
   if (ret < 0) rte_exit(EXIT_FAILURE, "EAL init failed\n");

   signal(SIGINT, signal_handler);

   uint16_t port_id;
   struct rte_mempool* mbuf_pool_;

   mbuf_pool_ =
       rte_pktmbuf_pool_create("FRONTEND_POOL", MBUF_POOL_SIZE, MBUF_CACHE_SIZE,
                               0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
   if (!mbuf_pool_) throw std::runtime_error("Mbuf Pool Allocation Failed");

   // This iterates through all ports initialized by EAL
   RTE_ETH_FOREACH_DEV(port_id) {
      struct rte_eth_dev_info dev_info;
      char dev_name[RTE_ETH_NAME_MAX_LEN];

      struct rte_eth_conf port_conf = {};

      // Get the hardware/driver info
      rte_eth_dev_info_get(port_id, &dev_info);

      // Get the name using the API we verified earlier
      if (rte_eth_dev_get_name_by_port(port_id, dev_name) == 0) {
         std::cout << "Found Device: " << dev_name
                   << " TXQ:" << dev_info.max_tx_queues
                   << " RXQ:" << dev_info.max_rx_queues
                   << " MinMTU:" << dev_info.min_mtu
                   << " MaxMTU:" << dev_info.max_mtu << std::endl;

         if (rte_eth_dev_configure(port_id, dev_info.max_rx_queues,
                                   dev_info.max_tx_queues, &port_conf) < 0)
            throw std::runtime_error("Failed to configure port");


         for (int i = 0; i < dev_info.max_rx_queues; i++) {
            if (rte_eth_rx_queue_setup(port_id, i, 1024,
                                       rte_eth_dev_socket_id(port_id), NULL,
                                       mbuf_pool_) < 0)
               throw std::runtime_error("Failed to setup RX queue");

            if (rte_eth_tx_queue_setup(
                    port_id, i, 1024, rte_eth_dev_socket_id(port_id), NULL) < 0)
               throw std::runtime_error("Failed to setup TX queue");
         }

         if (rte_eth_dev_start(port_id) < 0)
            throw std::runtime_error("Failed to start port");
      }
   }

   sleep(3);
   RTE_ETH_FOREACH_DEV(port_id) {
      std::cout << "Stopping Device: " << port_id << std::endl;
      rte_eth_dev_stop(port_id);
      std::cout << "Closing Device: " << port_id << std::endl;
      rte_eth_dev_close(port_id);
   }
   if (rte_eal_cleanup() < 0) {
      std::cout << "EAL cleanup failed!" << std::endl;
   }
   return 0;

   //   if (!rte_eth_dev_is_valid_port(port_id)) {
   //      rte_exit(EXIT_FAILURE, "No Ethernet ports found. Did you forget
   //      --vdev?\n");
   //   }
   //
   //   // uint16_t port_id;
   //   rte_eth_dev_get_port_by_name(vdev_name, &port_id);
   //
   //   // 4. Configure Port
   //   struct rte_eth_conf port_conf = {};
   //   port_conf.rxmode.mtu = 1500;
   //   rte_eth_dev_configure(port_id, 1, 1, &port_conf);
   //   rte_eth_rx_queue_setup(port_id, 0, 1024, rte_socket_id(), NULL,
   //   mbuf_pool); rte_eth_tx_queue_setup(port_id, 0, 1024, rte_socket_id(),
   //   NULL); rte_eth_dev_start(port_id);
   //
   //   std::cout << "Frontend connected to /tmp/vhost-user0. Sending
   //   packets..." << std::endl;

   // 5. Traffic Loop (Simulating a VM sending data)
   // while (keep_running) {
   //    // rte_delay_ms(1000); // Send 1 packet per second for testing
   //    sleep(100);
   // }
   //
   // rte_eal_cleanup();
   // return 0;
}

/*
while (keep_running) {
    struct rte_mbuf *m = rte_pktmbuf_alloc(mbuf_pool);
    if (m) {
        // Fill mbuf with dummy data (Ethernet header)
        struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
        memset(&eth->dst_addr, 0xFF, 6); // Broadcast
        memset(&eth->src_addr, 0xAA, 6); // Fake Source
        eth->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
        m->data_len = 64;
        m->pkt_len = 64;

        // Send to Backend
        rte_eth_tx_burst(port_id, 0, &m, 1);
    }
    rte_delay_ms(1000); // Send 1 packet per second for testing
}



   // 2. Setup Memory
   struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("FRONT_POOL",
NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());


*/
