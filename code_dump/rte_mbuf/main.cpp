#include <rte_bus.h>
#include <rte_dev.h>
#include <rte_eal.h>
#include <rte_ethdev.h>

#include <rte_hexdump.h>

#include <unistd.h>
#include <csignal>
#include <iostream>
#include <stdio.h>

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

void dump_mbuf_hex_only(struct rte_mbuf *m) {
    uint8_t *data = rte_pktmbuf_mtod(m, uint8_t *);
    uint32_t len = m->data_len;

    printf("Packet Hex (%u bytes):\n", len);
    for (uint32_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        // Optional: add a newline every 16 bytes for readability
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
   int ret = rte_eal_init(argc, argv);
   if (ret < 0) rte_exit(EXIT_FAILURE, "EAL init failed\n");

    unsigned int main_core = rte_get_main_lcore();
    unsigned int count = rte_lcore_count();
    unsigned int current = rte_lcore_id();

    printf("Main Core ID: %u\n", main_core);
    printf("Total enabled lcores: %u\n", count);
    printf("Current core ID: %u\n", current);


   signal(SIGINT, signal_handler);

   uint16_t port_id;
   struct rte_mempool* mbuf_pool_;

   mbuf_pool_ =
       rte_pktmbuf_pool_create("FRONTEND_POOL", MBUF_POOL_SIZE, MBUF_CACHE_SIZE,
                               0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
   if (!mbuf_pool_) throw std::runtime_error("Mbuf Pool Allocation Failed");

   // Create a pkt
   struct rte_mbuf *m = rte_pktmbuf_alloc(mbuf_pool_);
   // struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
   // memset(&eth->dst_addr, 0xFF, 6); // Broadcast
   // memset(&eth->src_addr, 0xAA, 6); // Fake Source
   // eth->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
   // struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
   char *data = (char *)rte_pktmbuf_append(m, 120);
   m->data_len = 120;
   m->pkt_len = 120;
   memset(data, 0xAA, m->pkt_len);
   // rte_pktmbuf_dump(stdout, m, m->pkt_len);

   // rte_hexdump(stdout, "Packet Data", rte_pktmbuf_mtod(m, void *), m->data_len);
   // rte_memdump(stdout, "My Packet Payload", rte_pktmbuf_mtod(m, void *), m->data_len);

   dump_mbuf_hex_only(m)   ;

   sleep(3);
   if (rte_eal_cleanup() < 0) {
      std::cout << "EAL cleanup failed!" << std::endl;
   }
   return 0;
}

/*
while (keep_running) {
    struct rte_mbuf *m = rte_pktmbuf_alloc(mbuf_pool_);
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

*/
