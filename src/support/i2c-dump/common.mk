ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../install
USE_INSTALL_ROOT=1
INSTALLDIR=sbin

EXTRA_INCVPATH+= $(PROJECT_ROOT)/../../../prebuilt/usr/include
EXTRA_LIBVPATH+= $(PROJECT_ROOT)/../../../prebuilt/mips$(filter le be,$(subst ., ,$(VARIANTS)))/lib

USEFILE = $(PROJECT_ROOT)/main.c

include $(MKFILES_ROOT)/qtargets.mk
