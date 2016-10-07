ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

ISKERNEL := 1
include ../../../../../netdrivers.mk

LIBS = drvrS  cacheS
LIBS += $(foreach libpath,$(LIBVPATH), $(if $(wildcard $(libpath)/libnetdrvrS.a), netdrvrS) )
CCFLAGS += -DUSE_LIBCACHE 
CCFLAGS += -DENABLE_HASH

NAME = devnp-$(PROJECT)

USEFILE=$(PROJECT_ROOT)/$(NAME).use

define PINFO
PINFO DESCRIPTION=Driver for Gigabit Ethernet on Elvees Salute EL24D1 board
endef

include $(MKFILES_ROOT)/qtargets.mk

