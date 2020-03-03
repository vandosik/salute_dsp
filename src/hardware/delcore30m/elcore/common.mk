ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=delcore30m

USEFILE=$(PROJECT_ROOT)/delcore30m.use
INSTALLDIR = sbin

EXTRA_INCVPATH+=$(PROJECT_ROOT)/../dma/sdma
EXTRA_LIBVPATH+=$(PROJECT_ROOT)/../dma/sdma/arm/a.le.v7/

LIBS+=sdma

-include $(PROJECT_ROOT)/roots.mk
include $(MKFILES_ROOT)/qtargets.mk
