# with version 19.11.11

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

ifneq ($(CONFIG_RTE_EXEC_ENV),"linuxapp")
$(info This application can only operate in a linuxapp environment, \
please change the definition of the RTE_TARGET environment variable)
all:
else

# binary name
APP = ../app.x

# all source are stored in SRCS-y
SRCS-y := main.c 

CFLAGS += -Wall -Werror -Wno-unused-parameter -O2 -D_FILE_OFFSET_BITS=64 -Wno-unused-variable -Wno-unused-function
CFLAGS += $(WERROR_FLAGS)
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DEMULATION_AVIP
CFLAGS += -I$(RTE_SDK)/lib/librte_spirent

LDLIBS += -lncurses

include $(RTE_SDK)/mk/rte.extapp.mk

endif
