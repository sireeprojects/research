#include <iostream>
#include <vector>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

int main(int argc, char *argv[]) {
    // 1. Initialize EAL
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    // 2. Create the Virtio-User device programmatically
    // This is equivalent to passing --vdev on the command line
    // 'path=/dev/vhost-net' tells DPDK to use the kernel backend (creating a TAP)
    // 'iface=tap0' names the interface in the Linux kernel
    const char *vdev_name = "net_virtio_user0";
    const char *vdev_args = "path=/dev/vhost-net,queues=1,queue_size=1024,iface=tap0";
    
    if (rte_eal_hotplug_add("vdev", vdev_name, vdev_args) < 0) {
        rte_exit(EXIT_FAILURE, "Failed to create virtio-user vdev\n");
    }

    uint16_t port_id = 0; // Usually 0 if it's the only device
    
    // 3. Setup Memory Pool
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    // 4. Configure Port
    struct rte_eth_conf port_conf = {};
    rte_eth_dev_configure(port_id, 1, 1, &port_conf);
    
    // 5. Setup RX and TX Queues
    rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
    rte_eth_tx_queue_setup(port_id, 0, TX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL);
    
    // 6. Start the Port
    rte_eth_dev_start(port_id);
    std::cout << "DPDK Port Started. Interface 'tap0' created in kernel." << std::endl;
    std::cout << "Run: 'sudo ip link set tap0 up' to begin." << std::endl;

    // 7. Main Loop: Read from TAP and Write back to TAP
    struct rte_mbuf *bufs[BURST_SIZE];
    while (true) {
        // Receive from Kernel (TAP -> DPDK)
        const uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
        
        if (nb_rx > 0) {
            std::cout << "Received " << nb_rx << " packets from kernel." << std::endl;
            
            // Send back to Kernel (DPDK -> TAP)
            uint16_t nb_tx = rte_eth_tx_burst(port_id, 0, bufs, nb_rx);
            
            // Free any packets that couldn't be sent
            if (unlikely(nb_tx < nb_rx)) {
                for (uint16_t buf = nb_tx; buf < nb_rx; buf++)
                    rte_pktmbuf_free(bufs[buf]);
            }
        }
    }

    return 0;
}
