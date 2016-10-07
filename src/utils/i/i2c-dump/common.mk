ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE = $(PROJECT_ROOT)/main.c

include $(MKFILES_ROOT)/qtargets.mk
