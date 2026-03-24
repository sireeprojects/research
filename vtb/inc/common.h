#pragma once

#include <cstdint>

namespace vtb {

static constexpr unsigned MAX_QUEUE_PAIRS=8;

struct vhost_queue_pair {
   uint16_t rxq_id;
   uint16_t txq_id;
   bool rxq_enabled;
   bool txq_enabled;
};

struct vhost_device {
   int vid;
   int nof_queue_pairs;
   struct vhost_queue_pair qp[MAX_QUEUE_PAIRS];
   bool ready;
   uint16_t ctlq_id;
};

struct port_queue_pair {
   int rxq_id;
   int txq_id;
};

struct port_device {
   struct port_queue_pair qp[MAX_QUEUE_PAIRS];
   int ctlq_id;
};

}
