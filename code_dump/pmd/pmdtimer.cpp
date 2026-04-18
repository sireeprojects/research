#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_cycles.h> // Added for high-res timers
#include <iostream>
#include <signal.h>
#include <iomanip>

static volatile bool keep_running = true;
static void signal_handler(int) { keep_running = false; }

int main(int argc, char* argv[]) {
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    signal(SIGINT, signal_handler);

    uint16_t port_id;
    ret = rte_eth_dev_get_port_by_name("net_vhost0", &port_id);
    if (ret < 0) rte_exit(EXIT_FAILURE, "Cannot find vhost port. Did you pass --vdev?\n");

    unsigned int socket_id = rte_eth_dev_socket_id(port_id);
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", 
        262143, 250, 0, RTE_MBUF_DEFAULT_BUF_SIZE, socket_id);

    if (!mbuf_pool) rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    struct rte_eth_conf port_conf = {0};
    if (rte_eth_dev_configure(port_id, 1, 1, &port_conf) < 0)
        rte_exit(EXIT_FAILURE, "Cannot configure device\n");

    if (rte_eth_rx_queue_setup(port_id, 0, 1024, socket_id, NULL, mbuf_pool) < 0)
        rte_exit(EXIT_FAILURE, "RX queue setup failed\n");

    if (rte_eth_tx_queue_setup(port_id, 0, 1024, socket_id, NULL) < 0)
        rte_exit(EXIT_FAILURE, "TX queue setup failed\n");

    if (rte_eth_dev_start(port_id) < 0)
        rte_exit(EXIT_FAILURE, "Device start failed\n");

    std::cout << "Backend started. Polling net_vhost0..." << std::endl;

    // Timer and Counter variables
    uint64_t total_pkts = 0;
    uint64_t prev_pkts = 0;
    uint64_t prev_tsc = rte_get_timer_cycles();
    uint64_t timer_hz = rte_get_timer_hz();
    struct rte_mbuf *bufs[64];

    while (keep_running) {
        uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, 64);
        
        if (nb_rx > 0) {
            total_pkts += nb_rx;
            for (uint16_t i = 0; i < nb_rx; i++) {
                rte_pktmbuf_free(bufs[i]);
            }
        }

        // Calculate PPS every 1 second
        uint64_t cur_tsc = rte_get_timer_cycles();
        if (unlikely(cur_tsc - prev_tsc > timer_hz)) {
            uint64_t diff_pkts = total_pkts - prev_pkts;
            
            // Convert to Mpps (Million packets per second) for readability
            double mpps = (double)diff_pkts / 1000000.0;

            std::cout << "Current Throughput: " 
                      << std::fixed << std::setprecision(3) << mpps 
                      << " Mpps | Total: " << total_pkts << "\r" << std::flush;

            prev_pkts = total_pkts;
            prev_tsc = cur_tsc;
        }
    }

    std::cout << "\nFinal Total: " << total_pkts << std::endl;
    rte_eth_dev_stop(port_id);
    return 0;
}
