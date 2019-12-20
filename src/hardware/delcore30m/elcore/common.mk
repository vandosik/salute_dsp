ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=delcore30m

USEFILE=$(PROJECT_ROOT)/delcore30m.use

EXTRA_SRCVPATH+=$(PROJECT_ROOT)/elcore_funcs



include $(MKFILES_ROOT)/qtargets.mk
