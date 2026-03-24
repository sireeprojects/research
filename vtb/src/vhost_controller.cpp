#include "config_manager.h"
#include "logger.h"
#include "messenger.h"

#include <csignal>
#include <cstdio>
#include <cinttypes>
#include <stdexcept>
#include <unistd.h>

#include <rte_log.h>

#include "vhost_controller.h"

namespace vtb {


#define RTE_LOGTYPE_VLOOP RTE_LOGTYPE_USER1

/* ------------------------------------------------------------------ */
/* Static instance pointer (supports one backend per process)         */
/* ------------------------------------------------------------------ */
VhostController* VhostController::instance_ = nullptr;

/* ------------------------------------------------------------------ */
/* Construction / destruction                                         */
/* ------------------------------------------------------------------ */
VhostController::VhostController(std::string socket_path)
    : socket_path_(std::move(socket_path))
{
    if (instance_)
        throw std::runtime_error("only one VhostController instance allowed");
    instance_ = this;
}

VhostController::~VhostController()
{
    stop();
    if (producer_thread_.joinable()) producer_thread_.join();
    if (consumer_thread_.joinable()) consumer_thread_.join();

    if (driver_registered_)
        rte_vhost_driver_unregister(socket_path_.c_str());
    if (ring_)
        rte_ring_free(ring_);
    if (eal_initialised_)
        rte_eal_cleanup();
    instance_ = nullptr;
}

/* ------------------------------------------------------------------ */
/* EAL + mbuf pool + ring initialisation                              */
/* ------------------------------------------------------------------ */
void VhostController::init(int argc, char* argv[])
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        throw std::runtime_error("EAL init failed");
    eal_initialised_ = true;

    mbuf_pool_ = rte_pktmbuf_pool_create("VLOOP_MBUF_POOL",
                                          MBUF_POOL_SIZE,
                                          MBUF_CACHE_SIZE,
                                          0,
                                          RTE_MBUF_DEFAULT_BUF_SIZE,
                                          rte_socket_id());
    if (!mbuf_pool_)
        throw std::runtime_error("cannot create mbuf pool");

    /* SP/SC ring: single-producer (dequeue thread), single-consumer (enqueue thread) */
    ring_ = rte_ring_create("VLOOP_RING", RING_SIZE, rte_socket_id(),
                            RING_F_SP_ENQ | RING_F_SC_DEQ);
    if (!ring_)
        throw std::runtime_error("cannot create inter-thread ring");
}

/* ------------------------------------------------------------------ */
/* Register socket, negotiate features, start listening               */
/* ------------------------------------------------------------------ */
void VhostController::start()
{
    const char* path = socket_path_.c_str();

    if (rte_vhost_driver_register(path, 0) != 0)
        throw std::runtime_error("vhost driver register failed: " + socket_path_);
    driver_registered_ = true;

    rte_vhost_driver_disable_features(path, 1ULL << VIRTIO_NET_F_MQ);
    rte_vhost_driver_enable_features(path, 1ULL << VIRTIO_NET_F_MRG_RXBUF);

    static const struct rte_vhost_device_ops ops = {
        .new_device          = cb_new_device,
        .destroy_device      = cb_destroy_device,
        .vring_state_changed = cb_vring_state_changed,
        .features_changed    = nullptr,
        .new_connection      = nullptr,
        .destroy_connection  = nullptr,
        .guest_notified      = nullptr,
        .guest_notify        = nullptr
    };

    if (rte_vhost_driver_callback_register(path, &ops) != 0)
        throw std::runtime_error("vhost callback register failed");

    if (rte_vhost_driver_start(path) != 0)
        throw std::runtime_error("vhost driver start failed");

    RTE_LOG(INFO, VLOOP, "vhost-user backend ready on %s, waiting for guest...\n", path);
}

/* ------------------------------------------------------------------ */
/* Producer: dequeue from guest TX virtqueue → rte_ring               */
/*   Lossless: retries ring enqueue until space is available.         */
/* ------------------------------------------------------------------ */
void VhostController::producer_loop()
{
    RTE_LOG(INFO, VLOOP, "producer thread started\n");

    while (!quit_.load(std::memory_order_relaxed)) {
        if (!ready_.load(std::memory_order_acquire)) {
            usleep(100000);
            continue;
        }

        struct rte_mbuf* pkts[PKT_BURST_SZ];
        uint16_t nb_rx = rte_vhost_dequeue_burst(vid_, VIRTIO_TXQ,
                                                  mbuf_pool_, pkts, PKT_BURST_SZ);
        if (nb_rx == 0) {
            usleep(1);
            continue;
        }

        /* Optional per-packet hook (e.g. modify, inspect) */
        for (uint16_t i = 0; i < nb_rx; i++)
            on_packet(pkts[i]);

        /* Lossless enqueue into ring — retry until all are in */
        uint16_t sent = 0;
        while (sent < nb_rx) {
            unsigned int pushed = rte_ring_sp_enqueue_burst(
                ring_,
                reinterpret_cast<void**>(&pkts[sent]),
                nb_rx - sent,
                nullptr);
            sent += pushed;
            if (sent < nb_rx && !quit_.load(std::memory_order_relaxed))
                usleep(1);  /* backpressure: ring full, let consumer drain */
        }

        rx_pkts_ += nb_rx;

        if ((rx_pkts_ & 0xFFFFF) == 0 && rx_pkts_ > 0)
            RTE_LOG(INFO, VLOOP,
                    "producer: dequeued %" PRIu64 " pkts from guest\n",
                    rx_pkts_);
    }

    RTE_LOG(INFO, VLOOP, "producer thread exiting (%" PRIu64 " pkts)\n", rx_pkts_);
}

/* ------------------------------------------------------------------ */
/* Consumer: rte_ring → enqueue into guest RX virtqueue               */
/*   Lossless: retries vhost enqueue until guest accepts all packets. */
/* ------------------------------------------------------------------ */
void VhostController::consumer_loop()
{
    RTE_LOG(INFO, VLOOP, "consumer thread started\n");

    while (!quit_.load(std::memory_order_relaxed)) {
        if (!ready_.load(std::memory_order_acquire)) {
            usleep(100000);
            continue;
        }

        struct rte_mbuf* pkts[PKT_BURST_SZ];
        unsigned int nb_deq = rte_ring_sc_dequeue_burst(
            ring_,
            reinterpret_cast<void**>(pkts),
            PKT_BURST_SZ,
            nullptr);

        if (nb_deq == 0) {
            usleep(1);
            continue;
        }

        /* Lossless enqueue to guest RX — retry until all accepted */
        uint16_t sent = 0;
        uint32_t retries = 0;
        while (sent < nb_deq && retries < MAX_ENQUEUE_RETRIES) {
            uint16_t nb_tx = rte_vhost_enqueue_burst(
                vid_, VIRTIO_RXQ,
                &pkts[sent], nb_deq - sent);
            sent += nb_tx;
            if (sent < nb_deq) {
                retries++;
                usleep(1);  /* backpressure: guest RX ring full */
            }
        }

        /* Account bytes */
        for (unsigned int i = 0; i < nb_deq; i++)
            tx_bytes_ += pkts[i]->pkt_len;
        tx_pkts_ += sent;

        if (sent < nb_deq) {
            RTE_LOG(WARNING, VLOOP,
                    "consumer: enqueue failed after %u retries, "
                    "%u of %u pkts dropped\n",
                    MAX_ENQUEUE_RETRIES, nb_deq - sent, nb_deq);
            /* Free the mbufs that could not be delivered */
            for (unsigned int i = sent; i < nb_deq; i++)
                rte_pktmbuf_free(pkts[i]);
        }

        /* Free delivered mbufs (vhost enqueue copies data) */
        for (uint16_t i = 0; i < sent; i++)
            rte_pktmbuf_free(pkts[i]);

        if ((tx_pkts_ & 0xFFFFF) == 0 && tx_pkts_ > 0)
            RTE_LOG(INFO, VLOOP,
                    "consumer: enqueued %" PRIu64 " pkts / %" PRIu64 " bytes to guest\n",
                    tx_pkts_, tx_bytes_);
    }

    /* Drain any remaining mbufs in the ring so they are not leaked */
    unsigned int remaining;
    do {
        struct rte_mbuf* pkts[PKT_BURST_SZ];
        remaining = rte_ring_sc_dequeue_burst(
            ring_, reinterpret_cast<void**>(pkts), PKT_BURST_SZ, nullptr);
        for (unsigned int i = 0; i < remaining; i++)
            rte_pktmbuf_free(pkts[i]);
    } while (remaining > 0);

    RTE_LOG(INFO, VLOOP,
            "consumer thread exiting (%" PRIu64 " pkts, %" PRIu64 " bytes)\n",
            tx_pkts_, tx_bytes_);
}

/* ------------------------------------------------------------------ */
/* run() — launches both threads and blocks until stop()              */
/* ------------------------------------------------------------------ */
void VhostController::run()
{
    RTE_LOG(INFO, VLOOP, "launching producer/consumer threads (socket: %s)\n",
            socket_path_.c_str());

    producer_thread_ = std::thread(&VhostController::producer_loop, this);
    consumer_thread_ = std::thread(&VhostController::consumer_loop, this);

    /* Block main thread until shutdown is signalled */
    producer_thread_.join();
    consumer_thread_.join();

    RTE_LOG(INFO, VLOOP,
            "shutdown complete — rx: %" PRIu64 " pkts, tx: %" PRIu64 " pkts / "
            "%" PRIu64 " bytes\n",
            rx_pkts_, tx_pkts_, tx_bytes_);
}

void VhostController::stop()
{
    quit_.store(true, std::memory_order_relaxed);
}

/* ------------------------------------------------------------------ */
/* Device lifecycle hooks                                             */
/* ------------------------------------------------------------------ */
void VhostController::on_new_device(int vid)
{
    RTE_LOG(INFO, VLOOP, "new device vid=%d\n", vid);
    vid_   = vid;
    ready_.store(true, std::memory_order_release);
}

void VhostController::on_destroy_device(int vid)
{
    RTE_LOG(INFO, VLOOP, "destroy device vid=%d\n", vid);
    if (vid_ == vid) {
        ready_.store(false, std::memory_order_release);
        vid_ = -1;
    }
}

void VhostController::on_vring_state_changed(int vid, uint16_t queue_id, int enable)
{
    RTE_LOG(INFO, VLOOP, "vring state changed vid=%d queue=%u enable=%d\n",
            vid, queue_id, enable);
}

/* ------------------------------------------------------------------ */
/* Static C callbacks → instance dispatch                             */
/* ------------------------------------------------------------------ */
int VhostController::cb_new_device(int vid)
{
    instance_->on_new_device(vid);
    return 0;
}

void VhostController::cb_destroy_device(int vid)
{
    instance_->on_destroy_device(vid);
}

int VhostController::cb_vring_state_changed(int vid, uint16_t queue_id, int enable)
{
    instance_->on_vring_state_changed(vid, queue_id, enable);
    return 0;
}

}
