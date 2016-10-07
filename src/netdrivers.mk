
# We link the stack with -E so a lot of the undefined
# references get resolved from the stack itself.  If
# you want them listed at link time, turn off
# --allow-shlib-undefined and replace with --warn-once
# if desired.

#LDFLAGS+=-Wl,--warn-once
LDFLAGS+=-Wl,--allow-shlib-undefined

NETDRVR_ROOT:=$(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DEVNP_ROOT:=$(NETDRVR_ROOT)/hardware/devnp
BSDDRVR_ROOT:=$(DEVNP_ROOT)/bsd
LIBNBDRVR_ROOT:=$(NETDRVR_ROOT)/lib/libnbdrvr

HDR_PATH=$(INSTALL_ROOT_HDR)/io-pkt
PUBLIC_HDR_PATH=$(KPDA_TARGET)/usr/include/io-pkt

# Check for staging area first
EXTRA_INCVPATH+= $(HDR_PATH) $(HDR_PATH)/sys-nto
# Use headers installed in system if staging area not available
EXTRA_INCVPATH+= $(PRODUCT_ROOT) $(PUBLIC_HDR_PATH) $(PUBLIC_HDR_PATH)/sys-nto
ifneq ($(NEED_QNXH),)
CCFLAGS += -Wp,-include -Wp,$(if $(wildcard $(HDR_PATH)),$(HDR_PATH)/qnx.h,$(PUBLIC_HDR_PATH)/qnx.h)
endif
ifneq ($(NEED_LIBNBDRVR),)
EXTRA_INCVPATH+=$(LIBNBDRVR_ROOT)
endif
ifneq ($(ISKERNEL),)
CCFLAGS += -D_KERNEL
endif
CCFLAGS_e2k += -fkernel
CCFLAGS += $(CCFLAGS_$(CPU))

# gcc sometime after 2.95.3 added a builtin log()
CCFLAGS += -fno-builtin-log
