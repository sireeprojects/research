#define VHOST_PATH "/tmp/arun_ixia_ctrl.sock"
#define NUM_RX_FRAMES 1000000 // nof pkts that will be sent to RX of load module

#define _GNU_SOURCE
#include <stdio.h>       // FILE, printf, perror
#include <stdlib.h>      // malloc
#include <sys/types.h>   // fd_set
#include <sys/socket.h>  // socket
#include <sys/un.h>      // sockaddr_un
#include <errno.h>       // sockaddr_un
#include <pthread.h>     // pthread_t pthread_create, pthread_setname_np, pthread_setaffinity_np
#include <sys/eventfd.h> // CPU_SET, CPU_ZERO, cpu_set_t
#include <stddef.h>      // offsetof
#include <assert.h>      // assert
#include <sys/mman.h>    // MAP_FAILED
#include <time.h>        // clock_t clock()
#include <unistd.h> 

#define MAX_LEN                         10240
#define VHOST_USER_PROTOCOL_F_MQ        0
#define VHOST_USER_PROTOCOL_F_LOG_SHMFD 1
#define VHOST_USER_PROTOCOL_F_RARP      2
#define VHOST_MEMORY_MAX_NREGIONS       8
#define VHOST_USER_VERSION_MASK         0x3
#define VHOST_USER_REPLY_MASK           (0x1 << 2)
#define VHOST_USER_VRING_IDX_MASK       0xff
#define VHOST_USER_VRING_NOFD_MASK      (0x1<<8)
#define VHOST_USER_HDR_SIZE             offsetof(vh_msg, payload.u64)
#define VHOST_VRING_F_LOG               0
#define VHOST_USER_VERSION              0x1
#define VHOST_F_LOG_ALL                 26
#define VHOST_USER_F_PROTOCOL_FEATURES  30
#define VHOST_LOG_PAGE                  4096
#define VIRTIO_NET_F_MRG_RXBUF          15
#define VIRTIO_NET_F_GUEST_ANNOUNCE     21
#define VIRTIO_F_VERSION_1              32
#define MAX_NR_VIRTQUEUE                8
#define VRING_AVAIL_F_NO_INTERRUPT      1
#define VRING_DESC_F_WRITE              2
#define VRING_DESC_F_NEXT               1
#define PROT_READ                       0x1
#define PROT_WRITE                      0x2
#define PROT_EXEC                       0x4
#define PROT_NONE                       0x0
#define MAP_SHARED                      0x01
#define MAP_PRIVATE                     0x02
#define PALLADIUM_LATENCY               0x2ull
#define PALLADIUM_IFG                   0x1ull
#define PALLADIUM_ET                    0x4ull
#define PALLADIUM_PORT_DISABLED         0x8ull
#define PACKED                          __attribute__((__packed__))
#define VHOST_USER_NEED_REPLY           (0x1 << 3)
#define VHOST_USER_HDR_SIZE             offsetof(vh_msg, payload.u64)
#define PROT_READ                       0x1
#define PROT_WRITE                      0x2
#define PROT_EXEC                       0x4
#define PROT_NONE                       0x0
#define MAP_SHARED                      0x01
#define MAP_PRIVATE                     0x02
#define VHOST_MEMORY_MAX_NREGIONS       8
#define MIN(a, b)                       (((a) < (b)) ? (a) : (b))
#define barrier()                       ({asm volatile("" ::: "memory"); (void)0;})
#define atomic_xchg(ptr, i)             (barrier(), __sync_lock_test_and_set(ptr, i))

#define MSG(...) \
    printf (__VA_ARGS__); \
    printf("\n"); \
    fprintf (logfile, __VA_ARGS__); \
    fprintf (logfile, "\n"); \
    fflush(stdout); \
    fflush(logfile);

#ifdef DBG 
    #define DMSG(...) \
    printf (__VA_ARGS__); \
    printf("\n"); \
    fprintf (logfile, __VA_ARGS__); \
    fprintf (logfile, "\n"); \
    fflush(stdout); \
    fflush(logfile);
#else 
    #define DMSG(...) {}
#endif

FILE *logfile;
extern int errno;

// vhost user msg type
typedef enum {
    VHOST_USER_NONE                  =  0,
    VHOST_USER_GET_FEATURES          =  1,
    VHOST_USER_SET_FEATURES          =  2,
    VHOST_USER_SET_OWNER             =  3,
    VHOST_USER_RESET_OWNER           =  4,
    VHOST_USER_SET_MEM_TABLE         =  5,
    VHOST_USER_SET_LOG_BASE          =  6,
    VHOST_USER_SET_LOG_FD            =  7,
    VHOST_USER_SET_VRING_NUM         =  8,
    VHOST_USER_SET_VRING_ADDR        =  9,
    VHOST_USER_SET_VRING_BASE        = 10,
    VHOST_USER_GET_VRING_BASE        = 11,
    VHOST_USER_SET_VRING_KICK        = 12,
    VHOST_USER_SET_VRING_CALL        = 13,
    VHOST_USER_SET_VRING_ERR         = 14,
    VHOST_USER_GET_PROTOCOL_FEATURES = 15,
    VHOST_USER_SET_PROTOCOL_FEATURES = 16,
    VHOST_USER_GET_QUEUE_NUM         = 17,
    VHOST_USER_SET_VRING_ENABLE      = 18,
    VHOST_USER_SEND_RARP             = 19,
    VHOST_USER_MAX
} vhost_req;

static const char *vhost_req_str[] = {
    "vhost_user_none"                  ,
    "vhost_user_get_features"          ,
    "vhost_user_set_features"          ,
    "vhost_user_set_owner"             ,
    "vhost_user_reset_owner"           ,
    "vhost_user_set_mem_table"         ,
    "vhost_user_set_log_base"          ,
    "vhost_user_set_log_fd"            ,
    "vhost_user_set_vring_num"         ,
    "vhost_user_set_vring_addr"        ,
    "vhost_user_set_vring_base"        ,
    "vhost_user_get_vring_base"        ,
    "vhost_user_set_vring_kick"        ,
    "vhost_user_set_vring_call"        ,
    "vhost_user_set_vring_err"         ,
    "vhost_user_get_protocol_features" ,
    "vhost_user_set_protocol_features" ,
    "vhost_user_get_queue_num"         ,
    "vhost_user_set_vring_enable"      ,
    "vhost_user_send_rarp"             ,
    "vhost_user_max"                   ,
};

// metadata to carry info to calculate latency
struct PACKED palladium_metadata {
    uint64_t metadata_type;
    union {
        uint64_t ifg;
        uint64_t emulator_time;
    };
    uint64_t latency;
    char reserved[21];
};

// ixverify metadata
struct PACKED ixverify_metadata_hdr {
   uint16_t mrg_num_buffers;
   uint8_t generic_flags;
   struct palladium_metadata pd;
};

// available ring buffer (guest->host)
struct vring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[0];
};

// fields to desribe a buffer
struct vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct vring_used_elem {
    uint32_t id;
    uint32_t len;
};

// used ring buffer (host->guest)
struct vring_used {
    uint16_t flags;
    uint16_t idx;
    struct vring_used_elem ring[0];
};

struct vring_state {
    unsigned int index;
    unsigned int num;
};

struct vring_addr {
    unsigned int index;
    unsigned int flags;
    uint64_t desc_user_addr;
    uint64_t used_user_addr;
    uint64_t avail_user_addr;
};

struct mem_region {
    uint64_t guest_phys_addr;
    uint64_t memory_size;
    uint64_t userspace_addr;
    uint64_t mmap_offset;
};

struct mem {
    uint32_t nregions;
    uint32_t padding;
    struct mem_region regions[VHOST_MEMORY_MAX_NREGIONS];
};

struct log {
    uint64_t mmap_size;
    uint64_t mmap_offset;
};

typedef struct PACKED vhost_msg {
    vhost_req request;
    uint32_t flags;
    uint32_t size;
    union {
        uint64_t u64;
        struct vring_state state;
        struct vring_addr addr;
        struct mem memory;
        struct log log;
    } payload;
    int fds[VHOST_MEMORY_MAX_NREGIONS];
} vh_msg;

struct vh_virtqueue {
    struct vring_desc *desc;
    struct vring_avail *avail;
    struct vring_used *used;
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    uint32_t size;
    int call_fd;
    int kick_fd;
    int enable;
};

struct device_region {
    uint64_t gpa;
    uint64_t size;
    uint64_t qva;
    uint64_t mmap_offset;
    uint64_t mmap_addr;
};

struct buf_vector {
    uint64_t buf_addr;
    uint32_t buf_len;
    uint32_t desc_idx;
};

struct dummy_pkt_with_metadata {
    struct ixverify_metadata_hdr meta;
    char buf[12288];
};

struct backend {
    uint64_t features;
    int hdrlen;
    uint32_t nregions;
    struct device_region regions[VHOST_MEMORY_MAX_NREGIONS];
    struct vh_virtqueue vq[2];
    int en2[2];
    pthread_t tx;
    pthread_t rx;
    uint32_t txcnt;
    uint32_t rxcnt;
    struct buf_vector buf_vec[16];
    clock_t txstart, txend;
    clock_t rxstart, rxend;
};

typedef struct vhserver {
    fd_set fdset;
    pthread_t pt;
    int max_fd;
    int fd;
    struct backend *bk;
} vh_desc;

static void *dequeue(void *cdn); // tx
static void *enqueue(void *cdn); // rx
static void *process_vh_msgs(void *svr);
static void process_vhost_msg(int fd, vh_desc *svr);
static void add_fd(vh_desc *svr, int fd);
static void del_fd(vh_desc *svr, int fd);
static void start_vhost_server(vh_desc *svr);
static void create_vhost_server(char *sock_path, vh_desc *svr);

void start_vhost_server(vh_desc *svr) {
    pthread_create(&svr->pt, NULL, &process_vh_msgs, (void*)svr);
    MSG("Cdn Backend: vhost server active");
}

void create_vhost_server(char *sock_path, vh_desc *svr) {
    int fd;
    struct sockaddr_un addr;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        MSG("vhost socket: Unable to create a socket");
        exit(0);
    }
    socklen_t len;
    len = sizeof(struct sockaddr_un);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, (len - sizeof(short)));
    unlink(addr.sun_path);
    
    if((bind(fd, (struct sockaddr*)&addr, len)) == -1) {
        MSG("vhost socket: bind failed");
        exit(0);
    }
    listen(fd, 256);
    add_fd(svr, fd);
    svr->fd = fd;
    DMSG("vhost socket: socket created (fd:%d)", fd);
}

void add_fd(vh_desc *svr, int fd) {
    FD_SET(fd, &svr->fdset);

    if (fd>svr->max_fd) {
        svr->max_fd = fd;
        DMSG("adding to vhost fdset fd:(%d) maxfd:(%d)", fd, svr->max_fd);
    }
}

void del_fd(vh_desc *svr, int fd) {
    FD_CLR(fd, &svr->fdset);
}

void reset(vh_desc *svr) {
    svr->max_fd = -1;
    svr->fd = -1;
    svr->bk->features = 0;
    svr->bk->hdrlen = 0;
    svr->bk->txcnt = 0;
    svr->bk->rxcnt = 0;
    svr->bk->vq[1].last_used_idx = 0;
    svr->bk->vq[1].last_avail_idx = 0;
    svr->bk->vq[0].last_used_idx = 0;
    svr->bk->vq[0].last_avail_idx = 0;
    FD_ZERO(&svr->fdset);
}

void *process_vh_msgs(void *svr) {
    vh_desc *vh = (vh_desc*)svr;
    struct timeval tv;
    int fd;

    while(1) {
        uint32_t timeout = 200000;
        tv.tv_sec = timeout / 1000000;
        tv.tv_usec = timeout % 1000000;
        fd_set fdset_rd = vh->fdset;
        int rc = select(vh->max_fd+1, &fdset_rd, 0, 0, &tv);

        if (rc==-1) perror ("select");

        for (fd=0; fd<vh->max_fd+1; fd++) {
            if (FD_ISSET(fd, &fdset_rd) && FD_ISSET(fd, &vh->fdset)) {
                if (fd == vh->fd) {
                    struct sockaddr_un un;
                    socklen_t len = sizeof(un);
                    int sfd = accept(fd, (struct sockaddr *) &un, &len);
                    add_fd(svr, sfd);
                } else {
                    process_vhost_msg(fd, svr);
                }
            }
        }
    }
}

uint64_t qva_to_va(uint64_t qemu_addr, vh_desc *svr) {
    uint32_t i;

    for (i = 0; i < svr->bk->nregions; i++) {
        struct device_region *r = &svr->bk->regions[i];
        if ((qemu_addr >= r->qva) && (qemu_addr < (r->qva + r->size)))
            return qemu_addr - r->qva + r->mmap_addr + r->mmap_offset;
    }
    assert(!"address not found in regions\n");
    return 0;
}

void send_vhost_reply(int fd, vh_msg *vmsg) {
   int rc;
   do {
      rc = write(fd, vmsg, VHOST_USER_HDR_SIZE + vmsg->size);
   }
   while (rc<0 && errno == EINTR);
   MSG("-> send_vhost_reply");

   if (rc < 0) {
      MSG("Write Error in send_vhost_reply");
      exit(1);
   }
}

// same as send_vhost_reply
void send_vhost_msg(int fd, vh_msg *vmsg) {
   int rc;
   do {
      rc = write(fd, vmsg, VHOST_USER_HDR_SIZE + vmsg->size);
   }
   while (rc<0 && errno == EINTR);
   MSG("-> send_vhost_msg");

   if (rc < 0) {
      MSG("Write Error in send_vhost_msg");
      exit(1);
   }
}

void process_vhost_msg(int fd, vh_desc *svr) {
    vh_msg vmsg; 
    char control[CMSG_SPACE(VHOST_MEMORY_MAX_NREGIONS * sizeof(int))] = { };

    struct iovec iov;
    iov.iov_base = (char *) &vmsg;
    iov.iov_len = offsetof(vh_msg, payload.u64);

    struct msghdr msg;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    
    size_t fd_size;
    struct cmsghdr *cmsg;
    uint32_t rc;
    
    rc = recvmsg(fd, &msg, 0);
    
    if (rc == 0) {
        MSG("Load Module disconnected\n");
        exit(1);
    }
    if (rc < 0) {
        MSG("Execute Request: rc<0");
        exit(1);
    }
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
            fd_size = cmsg->cmsg_len - CMSG_LEN(0);
            memcpy(vmsg.fds, CMSG_DATA(cmsg), fd_size);
            break;
        }
    }
    if (vmsg.size > sizeof(vmsg.payload)) {
        MSG ("Error: too big message request: %d, size: vmsg.size: %u, "
        "while sizeof(vmsg.payload) = %zu\n",
        vmsg.request, vmsg.size, sizeof(vmsg.payload));
        exit(1);
    }
    if (vmsg.size) {
        rc = read(fd, &vmsg.payload, vmsg.size);
        if (rc == 0) {
            MSG("Load Module disconnected\n");
            exit(1);
        }
        if (rc < 0) {
            MSG("Execute Request : rc<0");
            exit(1);
        }
        assert(rc == vmsg.size);
    }
    if (vmsg.request != 0) {
        MSG("-> Vhost msg rcvd (%s)", 
            vhost_req_str[vmsg.request] );
    }
    switch(vmsg.request) {
        case VHOST_USER_GET_FEATURES:{
            vmsg.payload.u64 = (
            (1ULL << VIRTIO_NET_F_MRG_RXBUF) |
            (1ULL << VHOST_USER_F_PROTOCOL_FEATURES));
            vmsg.size = sizeof(vmsg.payload.u64);
            vmsg.flags &= ~VHOST_USER_VERSION_MASK;
            vmsg.flags |= VHOST_USER_VERSION;
            vmsg.flags |= VHOST_USER_REPLY_MASK;
            send_vhost_reply(fd, &vmsg);
            break;
        }
        case VHOST_USER_SET_FEATURES:{
            svr->bk->features = vmsg.payload.u64;

            if ((svr->bk->features & (1ULL << VIRTIO_F_VERSION_1)) ||
                (svr->bk->features & (1ULL << VIRTIO_NET_F_MRG_RXBUF))) {
                svr->bk->hdrlen = 12;
            } else {
                svr->bk->hdrlen = 10;
            }
            break;
        }
        case VHOST_USER_SET_MEM_TABLE:{
            struct mem *memory = &vmsg.payload.memory;
            svr->bk->nregions = memory->nregions;
            uint32_t i;

            for (i = 0; i < svr->bk->nregions; i++) {
                void *mmap_addr;
                struct mem_region *msg_region = &memory->regions[i];
                struct device_region *dev_region = &svr->bk->regions[i];
                dev_region->gpa = msg_region->guest_phys_addr;
                dev_region->size = msg_region->memory_size;
                dev_region->qva = msg_region->userspace_addr;
                dev_region->mmap_offset = msg_region->mmap_offset;
                mmap_addr = mmap(0, 
                    dev_region->size+dev_region->mmap_offset, 
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED, 
                    vmsg.fds[i], 0);

                if (mmap_addr==MAP_FAILED) {
                    MSG("-> mmap failed");
                    exit(1);
                }
                dev_region->mmap_addr = (uint64_t)(uintptr_t)mmap_addr;
                close(vmsg.fds[i]);
            }
            break;
        }
        case VHOST_USER_SET_VRING_NUM:{
            unsigned int index = vmsg.payload.state.index;
            unsigned int num = vmsg.payload.state.num;
            svr->bk->vq[index].size = num;
            break;
        }
        case VHOST_USER_SET_VRING_ADDR:{
            struct vring_addr *vra = &vmsg.payload.addr;
            unsigned int index = vra->index;
            struct vh_virtqueue *vqi = &svr->bk->vq[index];
            vqi->desc = (struct vring_desc *)(uintptr_t)  qva_to_va(vra->desc_user_addr , svr);
            vqi->used = (struct vring_used *)(uintptr_t)  qva_to_va(vra->used_user_addr , svr);
            vqi->avail = (struct vring_avail *)(uintptr_t)qva_to_va(vra->avail_user_addr, svr);
            vqi->last_used_idx = vqi->used->idx;

            if (vqi->last_avail_idx != vqi->used->idx) {
                vqi->last_avail_idx = vqi->used->idx;
            }
            break;
        }
        case VHOST_USER_SET_VRING_BASE:{
            unsigned int index = vmsg.payload.state.index;
            unsigned int num = vmsg.payload.state.num;
            svr->bk->vq[index].last_avail_idx = num;
            break;
        }
        case VHOST_USER_GET_VRING_BASE:{
            unsigned int index = vmsg.payload.state.index;
            vmsg.payload.state.num = svr->bk->vq[index].last_avail_idx;
            vmsg.size = sizeof(vmsg.payload.state);

            if (svr->bk->vq[index].call_fd != -1) {
               close(svr->bk->vq[index].call_fd);
               svr->bk->vq[index].call_fd = -1;
            }
            if (svr->bk->vq[index].kick_fd != -1) {
               close(svr->bk->vq[index].kick_fd);
               svr->bk->vq[index].kick_fd = -1;
            }
            vmsg.flags &= ~VHOST_USER_VERSION_MASK;
            vmsg.flags |= VHOST_USER_VERSION;
            vmsg.flags |= VHOST_USER_REPLY_MASK;
            send_vhost_reply(fd, &vmsg);
            break;
        }
        case VHOST_USER_SET_VRING_KICK:{
            uint64_t u64_arg = vmsg.payload.u64;
            int index = u64_arg & VHOST_USER_VRING_IDX_MASK;
            svr->bk->vq[index].kick_fd = vmsg.fds[0];

            if (index%2==1) {
            }
            if (svr->bk->vq[0].kick_fd != -1 && svr->bk->vq[1].kick_fd != -1) {
               // ready = 1;
            }
            svr->bk->vq[index].avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;
            break;
        }
        case VHOST_USER_SET_VRING_CALL:{
            uint64_t u64_arg = vmsg.payload.u64;
            int index = u64_arg & VHOST_USER_VRING_IDX_MASK;
            assert((u64_arg & VHOST_USER_VRING_NOFD_MASK) == 0);
            svr->bk->vq[index].call_fd = vmsg.fds[0];
            break;
        }
        case VHOST_USER_SET_VRING_ERR:{
            MSG("-> Vring error u64: 0x%llx", vmsg.payload.u64);
            break;
        }
        case VHOST_USER_GET_PROTOCOL_FEATURES:{
            vmsg.payload.u64 = 1ULL << VHOST_USER_PROTOCOL_F_LOG_SHMFD;
            vmsg.size = sizeof(vmsg.payload.u64);
            vmsg.flags &= ~VHOST_USER_VERSION_MASK;
            vmsg.flags |= VHOST_USER_VERSION;
            vmsg.flags |= VHOST_USER_REPLY_MASK;
            send_vhost_reply(fd, &vmsg);
            break;
        }
        case VHOST_USER_SET_VRING_ENABLE:{
            unsigned int index = vmsg.payload.state.index;
            unsigned int enable = vmsg.payload.state.num;
            svr->bk->vq[index].enable = enable;

            // disable kick BEGIN
            vmsg.request = VHOST_USER_SET_VRING_CALL;
            vmsg.payload.u64 = VHOST_USER_VRING_NOFD_MASK;
            vmsg.size = sizeof(vmsg.payload.u64);
            send_vhost_msg(fd, &vmsg);
            // disable kick END

            svr->bk->en2[index]++;
            if(enable && svr->bk->en2[index]>1) {
                MSG ("-> Queue:%d is enabled", index);
                if(index==1) { // tx, start dequeu
                    pthread_create(&svr->bk->tx, NULL, &dequeue, (void*)svr->bk);
                }
            }
            break;
        }
        case VHOST_USER_SET_LOG_FD:{
            // not implemented
            break;
        }
        case VHOST_USER_SET_PROTOCOL_FEATURES:{
            // not implemented
            break;
        }
        case VHOST_USER_GET_QUEUE_NUM:{
            // not implemented
            break;
        }
        case VHOST_USER_SEND_RARP:{
            // not implemented
            break;
        }
        case VHOST_USER_SET_OWNER:{
            // not implemented
            break;
        }
        case VHOST_USER_RESET_OWNER:{
            // not implemented
            break;
        }
        case VHOST_USER_SET_LOG_BASE:{
            // not implemented
            break;
        }
        default:{
            MSG("Invalid vhost message from guest");
            break;
        }
    }
}

uint64_t gpa_to_va(uint64_t guest_addr, int mode, struct backend *b) {
    uint32_t i;
    for (i=0; i<b->nregions; i++) {
        struct device_region *r = &b->regions[i];
        if ((guest_addr >= r->gpa) && (guest_addr < (r->gpa + r->size)))
            return guest_addr - r->gpa + r->mmap_addr + r->mmap_offset;
    }
    if(mode==1) assert(!"Transmit side address not found in regions\n");
    if(mode==0) assert(!"Receive side address not found in regions\n");
    return 0;
}

void *enqueue(void *bknd) {
    struct backend *b = (struct backend*) bknd;
    struct vh_virtqueue *vq = &b->vq[0]; // rx
    struct vring_avail *avail = vq->avail;
    struct vring_used *used = vq->used;
    struct vring_desc *desc = vq->desc;
    uint32_t size = vq->size;
    uint16_t avail_index = 0;
    uint32_t len = 10240; // fixed size frame for testing
    uint32_t meta_len = 48;

    // dummy pkt 
    struct dummy_pkt_with_metadata *buf;
    buf = (struct dummy_pkt_with_metadata*) malloc(sizeof(struct dummy_pkt_with_metadata));
    memset((char*)buf, 0, len+meta_len);
    DMSG("Memory allocated for enqueue frame");

    // one descriptor to hold one pkt+metadata
    uint32_t nbufs = 3; 
    buf->meta.mrg_num_buffers = nbufs;
    buf->meta.pd.metadata_type = 0x4ull | 0x2ull;
    buf->meta.generic_flags = 0;

    while(b->rxcnt < NUM_RX_FRAMES) {
        avail_index = __sync_fetch_and_add(&avail->idx, 0);
        // send pkts to vlm if free descriptors are available
        if (vq->last_avail_idx != avail_index) { 
            DMSG("-----------------------------");
            DMSG("Buffers available: last_avail_idx:%d avail_index:%d", vq->last_avail_idx, avail_index);
            uint16_t a_idx = vq->last_avail_idx % size;
            uint16_t u_idx = vq->last_used_idx % size;
            uint16_t d_idx = __sync_fetch_and_add(&avail->ring[a_idx], 0); // atomic fetch add

            int i = d_idx;
            int nbufs_cnt = 0;
            int buf_vec_idx = 0;
            do {
                if (!(desc[i].flags & VRING_DESC_F_WRITE)) {
                    MSG("Error: descriptor is not writable. Dropping index: %d", i);
                    break;
                } else {
                    b->buf_vec[buf_vec_idx].buf_addr = desc[i].addr;
                    b->buf_vec[buf_vec_idx].buf_len = desc[i].len;
                    b->buf_vec[buf_vec_idx].desc_idx = i;
                    buf_vec_idx++;
                    nbufs_cnt++;
                }
                if ((desc[i].flags & VRING_DESC_F_NEXT)) {
                    i = __sync_fetch_and_add(&desc[i].next, 0);
                } else {
                    a_idx = a_idx + 1;
                    i = __sync_fetch_and_add(&avail->ring[a_idx%size], 0);
                }
            }
            while (nbufs != nbufs_cnt);
            assert (nbufs_cnt==nbufs);

            DMSG("collected buffers: %d", nbufs_cnt);
            DMSG("desc_addr: %llx", b->buf_vec[0].buf_addr);
            DMSG("desc_len: %llx", b->buf_vec[0].buf_len);
            DMSG("desc_idx: %llx", b->buf_vec[0].desc_idx);

            // uint64_t desc_addr = gpa_to_va(b->buf_vec[0].buf_addr, 0, b);
            //
            // memcpy ((void *)((uintptr_t)(desc_addr)), (char*)buf, len+meta_len); //pkt+meta
            // atomic_xchg(&used->ring[u_idx].id, b->buf_vec[0].desc_idx);
            // atomic_xchg(&used->ring[u_idx].len, (len+meta_len)); // 60+48
            // 
            // vq->last_avail_idx++;
            // vq->last_used_idx++;
            // atomic_xchg(&used->idx, vq->last_used_idx);
            // u_idx = vq->last_used_idx % size;

            uint32_t written_len = 0;
            unsigned int chunk_len;
            unsigned int chunk_to_write;

            uint32_t remaining_len = len;

            int cnt = 0; 
            for (cnt=0; cnt<nbufs; cnt++) {
                uint64_t desc_addr = gpa_to_va (b->buf_vec[cnt].buf_addr, 0, b);
                if (cnt==0) {
                    chunk_to_write = meta_len;
                    // write to desc
                    memcpy ((void *)((uintptr_t)(desc_addr)), (char*)&buf->meta, meta_len);
                    chunk_to_write = MIN(remaining_len, b->buf_vec[cnt].buf_len-meta_len);
                    remaining_len = remaining_len - chunk_to_write;
                    // write to desc
                    memcpy ((void *)((uintptr_t)(desc_addr+meta_len)), buf->buf, chunk_to_write);
                    written_len = written_len + chunk_to_write;
                    atomic_xchg (&used->ring[u_idx].id , b->buf_vec[cnt].desc_idx);
                    atomic_xchg (&used->ring[u_idx].len , chunk_to_write+meta_len);
                } else {
                    chunk_to_write = MIN(remaining_len, b->buf_vec[cnt].buf_len);
                    remaining_len = remaining_len - chunk_to_write;
                    // write to desc
                    memcpy ((void *)((uintptr_t)(desc_addr)), buf->buf+written_len, chunk_to_write);
                    written_len = written_len + chunk_to_write;
                    atomic_xchg (&used->ring[u_idx].id , b->buf_vec[cnt].desc_idx);
                    atomic_xchg (&used->ring[u_idx].len , chunk_to_write);
                }
                vq->last_avail_idx++;
                vq->last_used_idx++;
                atomic_xchg (&used->idx, vq->last_used_idx);
                u_idx = vq->last_used_idx % size;
            }
            // kick begin
            if (!(vq->avail->flags & VRING_AVAIL_F_NO_INTERRUPT)) {
                DMSG("Kicking vlm");
                eventfd_write(vq->call_fd, 1);
            } // kick end
            b->rxcnt++;
            if (b->rxcnt==1) {
                b->rxstart = clock(); 
            } else if (b->rxcnt==NUM_RX_FRAMES) {
                b->rxend = clock();
                double cpu_time_used = ((double) (b->rxend - b->rxstart)) / CLOCKS_PER_SEC;
                MSG("Receive Side: Time taken: %f", cpu_time_used)
                b->rxcnt=0;
                pthread_exit(NULL);
            }
            DMSG("pkt counter:%d", b->rxcnt);
        }
    }
    free(buf);
}

void *dequeue(void *bknd) {
    struct backend *b = (struct backend*) bknd;
    struct vh_virtqueue *vq = &b->vq[1]; // tx
    struct vring_avail *avail = vq->avail;
    struct vring_used *used = vq->used;
    struct vring_desc *desc = vq->desc;

    int size = b->vq->size;
    uint32_t meta_len = 48;
    uint32_t len = 0;
    uint32_t i = 0;
    struct ixverify_metadata_hdr meta;
    char *buf;
    buf = (char*)malloc(12288);

    // process all available descriptors
    while (1) {
        for(;;) {
            // no more descriptors to process
            if(vq->last_avail_idx ==(volatile uint16_t)avail->idx) {
                break;
            }
            DMSG("-----------------------------");
            DMSG("last idx: %d avail.idx: %d", vq->last_avail_idx, (volatile uint16_t)avail->idx);
            DMSG("desc idx: %d", i);

            uint16_t a_idx = vq->last_avail_idx % size;
            uint16_t d_idx = vq->avail->ring[a_idx];
            uint16_t u_idx = vq->last_used_idx % size;

            // process descriptors
            i = d_idx;
            len = 0;
            for (;;) {
                void *chunk = (void*)(uintptr_t)gpa_to_va(desc[i].addr, 1, b);
                uint32_t chunk_len = desc[i].len;
                DMSG("len in desc: %d", chunk_len);
                assert(!(desc[i].flags & VRING_DESC_F_WRITE));

                if (len+chunk_len<MAX_LEN) {
                    memcpy(buf+len, chunk, chunk_len);
                } else {
                    MSG("Frame too long");
                    exit(1);
                }
                len += chunk_len;

                if (desc[i].flags & VRING_DESC_F_NEXT) {
                    i = desc[i].next;
                } else {
                    break;
                }
            }
            if (!len) {
                DMSG("Error in extracted length");
                exit(1);
            }
            // NOTE: we are not doing anything with the pkt

            // decode control pkt to detect start/stop of streams
            memcpy (&meta, buf, 48);

            // after the tx path completes start the RX enqueue thread
            if (meta.generic_flags) {
                MSG("Received Control (%x)", meta.pd.metadata_type);

                if (meta.pd.metadata_type==8) { // port disable

                    // calculate time taken for transmission
                    b->txend = clock();
                    double cpu_time_used = ((double)(b->txend - b->txstart))/CLOCKS_PER_SEC;
                    MSG("Transmit Side: Time taken: %f", cpu_time_used)
                    b->txcnt=0;

                    // fork off the rx thread
                    // TODO
                    pthread_create(&b->rx, NULL, &enqueue, (void*)b);
                }
            } else {
                // save time at first frame
                if (b->txcnt==0) {
                    b->txstart = clock(); 
                }
                DMSG("frame rcvd:%d length: %d", b->txcnt+1, (len-48));
                b->txcnt++;
            }
            // update used ring 
            used->ring[u_idx].id = d_idx;
            used->ring[u_idx].len = 1;
            DMSG("updating used: idx:%d len:%d", d_idx, len);
            
            // to process next descriptor 
            a_idx = (a_idx+1)%size;
            vq->last_avail_idx++;
            vq->last_used_idx++;
        }
        atomic_xchg(&used->idx, vq->last_used_idx);
    }
    // free(buf);
}

// Bootstrap and begin
int main(int argc, char **argv) {
    // create log file to collect all printf's
    logfile = fopen("run.log", "w");

    // create a backend device
    struct backend *bk = (struct backend*)malloc(sizeof(struct backend));
    vh_desc *vh = (vh_desc*)malloc(sizeof(vh_desc));
    vh->bk = bk;

    // reset device variables
    reset(vh);

    // create and initiate vhost (ioctl) server
    create_vhost_server(VHOST_PATH, vh);
    start_vhost_server(vh);

    pthread_join(vh->pt, NULL);
    return 0;
}
