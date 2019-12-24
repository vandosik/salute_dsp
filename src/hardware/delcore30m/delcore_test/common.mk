ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=delcore_test

USEFILE=$(PROJECT_ROOT)/delcore_test.c

EXTRA_INCVPATH+=$(PROJECT_ROOT)/../elcore

include $(MKFILES_ROOT)/qtargets.mk
