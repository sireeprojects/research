#if !defined __AVIP_METADATA_H__
#define __AVIP_METADATA_H__

#include <stdint.h>

#define AVIP_SIG_PRESENT 0x01
#define PACKED __attribute__ ((__packed__))

typedef enum PACKED {
    TX = 1,   // From STC: a Tx frame, with IFG
    RX,       // To STC: an Rx frame, with latency and time
    TIME,     // To STC: emulation time update (no frame)
    TX_END    // From STC: end of Tx
} avip_mdata_type;

typedef struct PACKED {
    avip_mdata_type type;
    uint8_t flags;
    uint8_t pad1[6];
    union {
        struct {                        // From STC
            uint64_t ifg;               // Octets, counted from end of FCS to start of next preamble.
            uint16_t start_group_id;
            uint16_t start_group_count;
            uint32_t spare;
        };
        struct {                        // To STC
            uint64_t latency;           // Picosec.
            uint64_t time;              // Picosec after some arbitrary epoch.
        };
    };
    uint64_t pad2;                      // Pad to 32 bytes so that the frame proper is 16 byte aligned.
} avip_mdata_t;

// Conversion factor from emulation times to STC times (2.5 nanoseconds).
#define EMU2STC_TIME    0.0004

// Conversion factor from emulation latencies to STC units (2.5 nanoseconds).
#define EMU2STC_LATENCY 0.0004

#endif
