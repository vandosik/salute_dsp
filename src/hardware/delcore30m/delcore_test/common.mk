ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=delcore-test

USEFILE=$(PROJECT_ROOT)/delcore_test.c
INSTALLDIR = bin

EXTRA_INCVPATH+=$(PROJECT_ROOT)/../elcore
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../dma/sdma

-include $(PROJECT_ROOT)/roots.mk
include $(MKFILES_ROOT)/qtargets.mk
