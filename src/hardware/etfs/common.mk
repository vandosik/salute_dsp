ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = sbin

include $(MKFILES_ROOT)/qmacros.mk

-include $(PROJECT_ROOT)/roots.mk

NAME = fs-etfs
EXTRA_SILENT_VARIANTS+=$(VARIANT2)
USEFILE = $(PROJECT_ROOT)/etfs.use
EXTRA_INCVPATH += $(PRODUCT_ROOT)

LIBS = 	etfs

# If the etfs headers/libraries aren't present on the system, kill
# the compiles.
ifeq ($(call FIND_HDR_DIR,nto,usr/include,fs/etfs.h),)
TARGET_BUILD=@$(ECHO_HOST) ETFS headers/libraries not present on system
TARGET_INSTALL=
SRCS=
endif

include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk
include $(MKFILES_ROOT)/qtargets.mk
