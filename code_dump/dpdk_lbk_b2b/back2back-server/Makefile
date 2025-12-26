APP = lbk

SRCS-y := main.c
PKGCONF ?= pkg-config

CFLAGS += -O3 $(shell $(PKGCONF) --cflags libdpdk)
LDFLAGS += $(shell $(PKGCONF) --libs libdpdk)

# Required for low-level vhost APIs in some DPDK versions
CFLAGS += -DALLOW_EXPERIMENTAL_API

$(APP): $(SRCS-y)
	@$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS)

clean:
	rm -rf lbk
