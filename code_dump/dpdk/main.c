#include <time.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/virtio_net.h>
#include <linux/virtio_ring.h>
#include <strings.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>

#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>
#include <rte_log.h>
#include <rte_string_fns.h>
#include <rte_malloc.h>
#include <rte_vhost.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include "metadata.h"

#define MBUF_CACHE_SIZE 128
#define MBUF_DATA_SIZE  16000
#define MAX_PORTS 1

static const char *sockpath = "/tmp/emulation/sock";
static struct rte_mempool *mbuf_pool;

static struct port_struct {
    volatile int tx_up;
    volatile int rx_up;
    volatile uint64_t rx_frames;
    volatile uint64_t tx_frames;
    volatile uint64_t rx_octets;
    volatile uint64_t tx_octets;
} ports[MAX_PORTS];

// VID up / down handling
enum {VIRTIO_RXQ, VIRTIO_TXQ, VIRTIO_QNUM};

static void destroy_vid(int vid) {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    printf ("%4ld.%06ld Destroy device vid %d\n", tv.tv_sec % 10000, tv.tv_usec, vid);
    ports[vid].tx_up = 0;
    ports[vid].rx_up = 0;
}

static int new_vid (int vid) {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    printf ("%4ld.%06ld New device vid %d\n", tv.tv_sec % 10000, tv.tv_usec, vid);
    return 0;
}

static int vring_state_changed (int vid, uint16_t qid, int enable) {
    const char *tag = "??";
    struct timeval tv;
    gettimeofday (&tv, NULL);

    if (qid == VIRTIO_TXQ) tag = "Tx";
    if (qid == VIRTIO_RXQ) tag = "Rx";
    
    printf ("%4ld.%06ld %s vring state changed, vid = %d, queue id = %d, enable = %d\n", 
        tv.tv_sec % 10000, tv.tv_usec, tag, vid, qid, enable);

    if (qid == VIRTIO_TXQ) ports[vid].rx_up = enable;
    if (qid == VIRTIO_RXQ) ports[vid].tx_up = enable;
    
    return 0;
}

// These callback allow devices to be added to the data core when configuration
// has been fully complete.
static const struct vhost_device_ops virtio_net_device_ops = {
    .new_device =  new_vid,
    .destroy_device = destroy_vid,
    .vring_state_changed = vring_state_changed,
};

// When we receive a INT signal, unregister vhost driver 
static void sigint_handler(__rte_unused int signum) {
    // Unregister vhost driver
    int ret = rte_vhost_driver_unregister(sockpath);
    if (ret != 0)
        rte_exit(EXIT_FAILURE, "vhost driver unregister failure.\n");
    exit(0);
}

static void create_mbuf_pool(uint32_t mbuf_size, uint32_t nr_mbuf_cache) {
    uint32_t nr_mbufs;
    nr_mbufs = 256;

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", nr_mbufs,
            nr_mbuf_cache, 0, mbuf_size, rte_socket_id());
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
}

#define UPDATE_INTERVAL 5     // seconds
static void* perf_stats (void __attribute ((unused)) *arg) {
    struct port_struct prev[MAX_PORTS];
    struct port_struct now[MAX_PORTS];
    uint64_t tot_rx_frames, tot_rx_octets, tot_tx_frames, tot_tx_octets;
    memcpy (prev, ports, sizeof (ports));

    for (;;) {
        sleep (UPDATE_INTERVAL);
        memcpy (now, ports, sizeof (ports));
        tot_rx_frames = tot_rx_octets = tot_tx_frames = tot_tx_octets = 0;

        printf ("Port      Rx fps     Rx Bps      Tx fps     Tx Bps\n");
        for (int port = 0; port < MAX_PORTS; port++) {
            if (!ports[port].rx_up) {
                continue;
            }
            printf (" %2d   %10lu %10lu  %10lu %10lu\n",
                port,
                (now[port].rx_frames - prev[port].rx_frames) / UPDATE_INTERVAL, 
                (now[port].rx_octets - prev[port].rx_octets) / UPDATE_INTERVAL, 
                (now[port].tx_frames - prev[port].tx_frames) / UPDATE_INTERVAL, 
                (now[port].tx_octets - prev[port].tx_octets) / UPDATE_INTERVAL);
            tot_rx_frames += now[port].rx_frames - prev[port].rx_frames;
            tot_rx_octets += now[port].rx_octets - prev[port].rx_octets;
            tot_tx_frames += now[port].tx_frames - prev[port].tx_frames;
            tot_tx_octets += now[port].tx_octets - prev[port].tx_octets;
        }
        printf ("Tot   %10lu %10lu  %10lu %10lu\n",
            tot_rx_frames / UPDATE_INTERVAL, 
            tot_rx_octets / UPDATE_INTERVAL, 
            tot_tx_frames / UPDATE_INTERVAL, 
            tot_tx_octets / UPDATE_INTERVAL);
        printf ("\n");
        memcpy (prev, now, sizeof (prev));
    }
    return 0;
}

int main (int argc, char *argv[]) {
    static pthread_t stats_tid;
    int ret;

    memset (ports, 0, sizeof (ports));

    signal(SIGINT, sigint_handler);

    unlink (sockpath);

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
    argc -= ret;
    argv += ret;

    rte_log_set_global_level(RTE_LOG_NOTICE);

    // parse app arguments
    if (rte_lcore_count() > RTE_MAX_LCORE)
        rte_exit(EXIT_FAILURE,"Not enough cores\n");

    RTE_LOG(INFO, USER1, "There are %d ports available and %d cores\n", rte_eth_dev_count_avail(), rte_lcore_count());

    create_mbuf_pool(MBUF_DATA_SIZE, MBUF_CACHE_SIZE);

    ret = rte_vhost_driver_register(sockpath, 0);
    if (ret != 0)
        rte_exit(EXIT_FAILURE, "vhost driver register failure.\n");

    rte_vhost_driver_callback_register (sockpath, &virtio_net_device_ops);
    chmod (sockpath, 0777);
    rte_vhost_driver_start(sockpath);
    chmod (sockpath, 0777);

    // ret = pthread_create (&stats_tid, NULL, (void *)perf_stats, NULL);

    // if (ret != 0)
    //     rte_exit (EXIT_FAILURE, "Cannot create stats thread\n");

    for (;;) {
        for (int port = 0; port < MAX_PORTS; port++) {
            struct rte_mbuf *pkt;

            if (!ports[port].rx_up)
                continue;

            ret = rte_vhost_dequeue_burst (port, VIRTIO_TXQ, mbuf_pool, &pkt, 1);

            if (!ret)
                continue;
            
            avip_mdata_t *mdata = rte_pktmbuf_mtod (pkt, avip_mdata_t *);
            avip_mdata_type type = mdata->type;

            int len = rte_pktmbuf_pkt_len (pkt) - sizeof (avip_mdata_t);
            printf("Length of pkt received is %d\n", len);

            // if (type == TX) {
            //     ports[port].rx_frames += 1;
            //     ports[port].rx_octets += len;

            //     int tx_port = port ^ 1;

            //     if (ports[tx_port].tx_up) {
            //         memset (mdata, 0, sizeof (avip_mdata_t));

            //         mdata->type = RX;
            //         mdata->time = 100000;
            //         mdata->latency = 100000;

            //         int n_tx = rte_vhost_enqueue_burst (tx_port, VIRTIO_RXQ, &pkt, 1);
            //         if (n_tx != 1) {
            //             do {
            //                 rte_pause ();
            //                 n_tx = rte_vhost_enqueue_burst (tx_port, VIRTIO_RXQ, &pkt, 1);
            //                 // The return value can be zero if the peer has gone away. Give up if so.
            //                 if (n_tx == 0 && !ports[tx_port].tx_up) {
            //                     break;
            //                 }
            //             } while (n_tx != 1);
            //         }
            //         ports[tx_port].tx_frames += n_tx;
            //         ports[tx_port].tx_octets += len;
            //     }

            //     rte_pktmbuf_free(pkt);
            //     printf ("%d - %ld %ld %ld %ld\n", port, ports[port].rx_frames,
            //         ports[port].rx_octets, ports[tx_port].tx_frames, ports[tx_port].tx_octets);
            // }
        }
    }
    return 0;
}
