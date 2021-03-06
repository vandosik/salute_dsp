ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=dma-test

USEFILE=$(PROJECT_ROOT)/dma_test.c
INSTALLDIR = bin

#EXTRA_INCVPATH+=$(PROJECT_ROOT)/../elcore
#EXTRA_SRCVPATH+=$(PROJECT_ROOT)/../elcore

#SRCS+=sdma.c dma_test.c

EXTRA_LIBVPATH+=$(PROJECT_ROOT)/../sdma/arm/a.le.v7/
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../../elcore
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../sdma

LIBS+=sdma

-include $(PROJECT_ROOT)/roots.mk
include $(MKFILES_ROOT)/qtargets.mk
