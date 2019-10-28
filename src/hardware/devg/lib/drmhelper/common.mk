ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#INSTALLDIR_qnx4=lib
#INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)
INSTALLDIR=/dev/null


OS_VERSION=$(shell uname -v)
OS_NAME=$(shell uname -s)
ifeq (425,$(OS_VERSION))
# Exclude QNX6 only objects
EXCLUDE_OBJS += drm_cache.o
endif


SO_VERSION = 2

include $(MKFILES_ROOT)/qmacros.mk
define PINFO
PINFO DESCRIPTION=DRM helper functions used by graphics driver dlls
endef

include $(MKFILES_ROOT)/qtargets.mk


CCFLAGS += -D__KERNEL__


ifeq ($(OS), qnx4)
CCFLAGS += -Otax -D__X86__ -D__LITTLENDIAN__ -D__QNX4__
INCVPATH += $(PRODUCT_ROOT)/emu86/qnx4/emu86 $(PRODUCT_ROOT)/emu86/qnx4/glue $(PRODUCT_ROOT)/emu86/qnx4/include
else
CCFLAGS += -Wno-unused-variable
ifeq ($(origin NDEBUG),undefined)
CCFLAGS += -O0
endif
endif

-include $(PROJECT_ROOT)/roots.mk
ifndef DEVG_ROOT
DEVG_ROOT=../../../../..
endif
EXTRA_INCVPATH+=$(DEVG_ROOT)/public
EXTRA_INCVPATH+=$(DEVG_ROOT)/private
EXTRA_INCVPATH+=$(DEVG_ROOT)/private/linux/include
EXTRA_INCVPATH+=$(DEVG_ROOT)/private/linux/include/uapi
EXTRA_INCVPATH+=$(DEVG_ROOT)/private/linux/arch/$(CPU)
EXTRA_INCVPATH+=$(DEVG_ROOT)/private/linux/arch/$(CPU)/include
EXTRA_INCVPATH+=$(DEVG_ROOT)/private/linux/arch/$(CPU)/include/uapi
