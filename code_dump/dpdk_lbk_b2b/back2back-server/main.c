#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <rte_eal.h>
#include <rte_vhost.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_lcore.h>

#define BURST_SIZE 32
#define RING_SIZE 2048
#define NUM_MBUFS 8191
#define SOCKET_PATH_0 "/tmp/vhost-user0"
#define SOCKET_PATH_1 "/tmp/vhost-user1"

/* Global state for vhost device and flow control */
static volatile int g_vid = -1;
static volatile int quit = 0;
static volatile int ready = 0;
static struct rte_mempool *mbuf_pool;

/* --- VHOST CALLBACKS --- */
static int new_device(int vid) {
    printf("VM Connected: vid %d\n", vid);
    g_vid = vid;
    ready = 1;
    return 0;
}

static void destroy_device(int vid) {
    printf("VM Disconnected: vid %d\n", vid);
    g_vid = -1;
}

static const struct rte_vhost_device_ops vhost_ops = {
    .new_device = new_device,
    .destroy_device = destroy_device,
};

/* --- LCORE LOOPS --- */

/* Producer: Dequeue from VM -> Enqueue to Ring */
static int producer_loop(void *arg) {
    struct rte_ring *r = arg;
    struct rte_mbuf *bufs[BURST_SIZE];
    int cntr=0;

    printf("Producer started on lcore %u\n", rte_lcore_id());

    while (true) {
        while (ready) {
            int vid = g_vid;
            if (unlikely(vid == -1)) {
                rte_pause();
                continue;
            }
            // printf("Waiting...: %d\n", cntr);
            /* Dequeue from Guest TX (Host RX) - Virtqueue 0 */
            // uint16_t nb_rx = rte_vhost_dequeue_burst(vid, 1, mbuf_pool, bufs, BURST_SIZE);
            uint16_t nb_rx = rte_vhost_dequeue_burst(vid, 1, mbuf_pool, bufs, 1);
            
            if (nb_rx > 0) {
                cntr++;
                printf("Pkt Received: %d\n", cntr);
                unsigned int sent = rte_ring_enqueue_burst(r, (void **)bufs, nb_rx, NULL);
                
                /* If ring is full, we must free mbufs to prevent leaks */
                if (unlikely(sent < nb_rx)) {
                    for (uint16_t i = sent; i < nb_rx; i++) {
                        rte_pktmbuf_free(bufs[i]);
                    }
                }
            }
        }
    }
    return 0;
}

/* Consumer: Dequeue from Ring -> Enqueue to VM */
static int consumer_loop(void *arg) {
    struct rte_ring *r = arg;
    struct rte_mbuf *bufs[BURST_SIZE];
    int cntr= 0;

    printf("Consumer started on lcore %u\n", rte_lcore_id());

    while (!quit) {
        int vid = g_vid;
        
        /* Dequeue from SPSC Ring */
        unsigned int nb_dq = rte_ring_dequeue_burst(r, (void **)bufs, BURST_SIZE, NULL);

        if (nb_dq > 0) {
            if (likely(vid != -1)) {
                /* Enqueue to Guest RX (Host TX) - Virtqueue 1 */
                uint16_t nb_tx = rte_vhost_enqueue_burst(vid, 0, bufs, nb_dq);
                cntr++;
                printf("Pkt Transmitted: %d\n", cntr);
                
                /* Free mbufs that were successfully sent or failed */
                /* vHost library requires host to manage mbuf lifecycle after enqueue */
                for (uint16_t i = 0; i < nb_dq; i++) {
                    rte_pktmbuf_free(bufs[i]);
                }
            } else {
                /* VM disconnected while packets were in the ring */
                for (uint16_t i = 0; i < nb_dq; i++) {
                    rte_pktmbuf_free(bufs[i]);
                }
            }
        } else {
            rte_pause();
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    struct rte_ring *ring;

    /* 1. Initialize EAL */
    if (rte_eal_init(argc, argv) < 0) 
        rte_exit(EXIT_FAILURE, "EAL Init failed\n");

    /* 2. Create Mempool */
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, 250, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (!mbuf_pool) rte_exit(EXIT_FAILURE, "Mempool creation failed\n");

    /* 3. Create SPSC Ring */
    ring = rte_ring_create("SPSC_VHOST", RING_SIZE, rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);

    if (!ring) rte_exit(EXIT_FAILURE, "Ring creation failed\n");

    /* 4. Register and Start vHost Driver */
    if (rte_vhost_driver_register(SOCKET_PATH_0, RTE_VHOST_USER_CLIENT) < 0) rte_exit(EXIT_FAILURE, "vHost register failed\n");
    if (rte_vhost_driver_callback_register(SOCKET_PATH_0, &vhost_ops) < 0) rte_exit(EXIT_FAILURE, "vHost callback failed\n");
    if (rte_vhost_driver_start(SOCKET_PATH_0) < 0) rte_exit(EXIT_FAILURE, "vHost start failed\n");

    if (rte_vhost_driver_register(SOCKET_PATH_1, RTE_VHOST_USER_CLIENT) < 0) rte_exit(EXIT_FAILURE, "vHost register failed\n");
    if (rte_vhost_driver_callback_register(SOCKET_PATH_1, &vhost_ops) < 0) rte_exit(EXIT_FAILURE, "vHost callback failed\n");
    if (rte_vhost_driver_start(SOCKET_PATH_1) < 0) rte_exit(EXIT_FAILURE, "vHost start failed\n");

    /* 5. Launch lcore threads */
    unsigned int lcore_p = rte_get_next_lcore(-1, 1, 0);
    unsigned int lcore_c = rte_get_next_lcore(lcore_p, 1, 0);

    rte_eal_remote_launch(producer_loop, ring, lcore_p);
    rte_eal_remote_launch(consumer_loop, ring, lcore_c);

    rte_eal_mp_wait_lcore();
    return 0;
}
