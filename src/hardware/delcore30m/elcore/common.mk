ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=delcore30m

USEFILE=$(PROJECT_ROOT)/delcore30m.use
INSTALLDIR = sbin

# EXTRA_SRCVPATH+=$(PROJECT_ROOT)/elcore_funcs


-include $(PROJECT_ROOT)/roots.mk
include $(MKFILES_ROOT)/qtargets.mk
