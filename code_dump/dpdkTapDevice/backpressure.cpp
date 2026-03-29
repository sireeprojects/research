#include <iostream>
#include <chrono>
#include <csignal>
#include <cstring>
#include <thread>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_cycles.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

// Global for signal handling
bool keep_running = true;
uint16_t g_port_id = 0;

void signal_handler(int signum) {
    std::cout << "\n[SIGNAL] Cleaning up and exiting..." << std::endl;
    keep_running = false;
}

// Helper to modify TCP window and send ACK
void send_tcp_window_update(uint16_t port_id, struct rte_mbuf *m, uint16_t window_val) {
    struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    if (rte_be_to_cpu_16(eth->ether_type) != RTE_ETHER_TYPE_IPV4) return;

    struct rte_ipv4_hdr *ip = (struct rte_ipv4_hdr *)(eth + 1);
    if (ip->next_proto_id != IPPROTO_TCP) return;

    struct rte_tcp_hdr *tcp = (struct rte_tcp_hdr *)((unsigned char *)ip + ((ip->version_ihl & 0x0f) * 4));

    // Swap MACs
    struct rte_ether_addr tmp_mac;
    rte_ether_addr_copy(&eth->src_addr, &tmp_mac);
    rte_ether_addr_copy(&eth->dst_addr, &eth->src_addr);
    rte_ether_addr_copy(&tmp_mac, &eth->dst_addr);

    // Swap IPs
    uint32_t tmp_ip = ip->src_addr;
    ip->src_addr = ip->dst_addr;
    ip->dst_addr = tmp_ip;

    // Swap Ports
    uint16_t tmp_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = tmp_port;

    // Update TCP State
    uint32_t sent_seq = rte_be_to_cpu_32(tcp->sent_seq);
    tcp->recv_ack = rte_cpu_to_be_32(sent_seq + 1); 
    tcp->tcp_flags = RTE_TCP_ACK_FLAG;
    tcp->rx_win = rte_cpu_to_be_16(window_val);

    // Recalculate Checksums
    ip->hdr_checksum = 0;
    ip->hdr_checksum = rte_ipv4_cksum(ip);
    tcp->cksum = 0;
    tcp->cksum = rte_ipv4_udptcp_cksum(ip, tcp);

    rte_eth_tx_burst(port_id, 0, &m, 1);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    // 1. Initialize EAL
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) rte_exit(EXIT_FAILURE, "EAL init failed\n");

    // 2. Create Virtio-User Device (The TAP interface)
    const char *vdev_name = "net_virtio_user0";
    const char *vdev_args = "path=/dev/vhost-net,queues=1,queue_size=1024,iface=tap0";
    
    if (rte_eal_hotplug_add("vdev", vdev_name, vdev_args) < 0) {
        rte_exit(EXIT_FAILURE, "Failed to create vdev. Run 'sudo ip link delete tap0' first.\n");
    }

    if (rte_eth_dev_get_port_by_name(vdev_name, &g_port_id) < 0) {
        rte_exit(EXIT_FAILURE, "Port ID not found\n");
    }

    // 3. Setup Memory Pool
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    // 4. Configure Port
    struct rte_eth_conf port_conf = {};
    if (rte_eth_dev_configure(g_port_id, 1, 1, &port_conf) < 0)
        rte_exit(EXIT_FAILURE, "Config failed\n");

    if (rte_eth_rx_queue_setup(g_port_id, 0, RX_RING_SIZE, rte_eth_dev_socket_id(g_port_id), NULL, mbuf_pool) < 0)
        rte_exit(EXIT_FAILURE, "RX Setup failed\n");

    if (rte_eth_tx_queue_setup(g_port_id, 0, TX_RING_SIZE, rte_eth_dev_socket_id(g_port_id), NULL) < 0)
        rte_exit(EXIT_FAILURE, "TX Setup failed\n");

    // 5. Start Port (The Fix for the "Not Ready" error)
    if (rte_eth_dev_start(g_port_id) < 0)
        rte_exit(EXIT_FAILURE, "Start failed\n");

    std::cout << "DPDK App Ready. Interface: tap0\n";
    std::cout << "Run: sudo ip addr add 192.168.10.1/24 dev tap0 && sudo ip link set tap0 up\n";

    // 6. Main Processing Loop with Backpressure logic
    int pkt_count = 0;
    bool throttled = false;
    auto start_time = std::chrono::steady_clock::now();
    struct rte_mbuf *saved_mbuf = nullptr;

    while (keep_running) {
        struct rte_mbuf *bufs[BURST_SIZE];
        uint16_t nb_rx = rte_eth_rx_burst(g_port_id, 0, bufs, BURST_SIZE);

        for (uint16_t i = 0; i < nb_rx; i++) {
            if (!throttled) {
                pkt_count++;
                std::cout << "Captured Packet #" << pkt_count << std::endl;

                if (pkt_count >= 5) {
                    std::cout << ">>> BACKPRESSURE: Sending Window Zero ACK\n";
                    send_tcp_window_update(g_port_id, bufs[i], 0);
                    saved_mbuf = rte_pktmbuf_clone(bufs[i], bufs[i]->pool);
                    throttled = true;
                    start_time = std::chrono::steady_clock::now();
                }
            }
            rte_pktmbuf_free(bufs[i]);
        }

        // Release backpressure after 60 seconds
        if (throttled) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() >= 60) {
                std::cout << "<<< RESUMING: Restoring Window Size\n";
                send_tcp_window_update(g_port_id, saved_mbuf, 16384);
                rte_pktmbuf_free(saved_mbuf);
                throttled = false;
                pkt_count = 0;
            }
        }
    }

    // 7. Proper Shutdown
    rte_eth_dev_stop(g_port_id);
    rte_eth_dev_close(g_port_id);
    return 0;
}
