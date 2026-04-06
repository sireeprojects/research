#include <iostream>
#include <csignal>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_dev.h>

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static bool keep_running = true;
void signal_handler(int s) { keep_running = false; }

int main(int argc, char *argv[]) {
   int ret = rte_eal_init(argc, argv);
   if (ret < 0) rte_exit(EXIT_FAILURE, "EAL init failed\n");
   
   signal(SIGINT, signal_handler);
   
   uint16_t port_id;

   // This iterates through all ports initialized by EAL
   RTE_ETH_FOREACH_DEV(port_id) {
//		struct rte_eth_dev_info dev_info;
//    	rte_eth_dev_info_get(port_id, &dev_info);
//    
//    	// Option A: Using the struct (requires rte_dev.h)
//    	std::cout << "Device Name: " << dev_info.device->name << std::endl;
//
//    	// Option B: Using the API (cleaner)
//    	char name[RTE_ETH_NAME_MAX_LEN];
//    	rte_eth_dev_get_name_by_port(port_id, name);
//    	std::cout << "Port Name: " << name << " ID: " << port_id << std::endl;

      struct rte_eth_dev_info dev_info;
      rte_eth_dev_info_get(port_id, &dev_info);
		if (dev_info.device) {
      	std::cout << "Found Device: " << dev_info.max_tx_queues << " (Port ID: " << port_id << ")" << std::endl;
      	std::cout << "Found Device: " << dev_info.device->name << " (Port ID: " << port_id << ")" << std::endl;
		}
   }
   return 0;
   
 //   if (!rte_eth_dev_is_valid_port(port_id)) {
 //      rte_exit(EXIT_FAILURE, "No Ethernet ports found. Did you forget --vdev?\n");
 //   }
 //   
 //   // uint16_t port_id;
 //   rte_eth_dev_get_port_by_name(vdev_name, &port_id);
 //   
 //   // 4. Configure Port
 //   struct rte_eth_conf port_conf = {};
 //   port_conf.rxmode.mtu = 1500;
 //   rte_eth_dev_configure(port_id, 1, 1, &port_conf);
 //   rte_eth_rx_queue_setup(port_id, 0, 1024, rte_socket_id(), NULL, mbuf_pool);
 //   rte_eth_tx_queue_setup(port_id, 0, 1024, rte_socket_id(), NULL);
 //   rte_eth_dev_start(port_id);
 //   
 //   std::cout << "Frontend connected to /tmp/vhost-user0. Sending packets..." << std::endl;
   
   // 5. Traffic Loop (Simulating a VM sending data)
   while (keep_running) {
      rte_delay_ms(1000); // Send 1 packet per second for testing
   }
   
   rte_eal_cleanup();
   return 0;
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
   struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("FRONT_POOL", NUM_MBUFS,
      MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
   

*/
