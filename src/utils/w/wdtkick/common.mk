ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
include $(MKFILES_ROOT)/qmacros.mk

LIBS+=drvr
NAME =wdtkick
EXTRA_SILENT_VARIANTS+=$(SECTION)
USEFILE=$(PROJECT_ROOT)/$(NAME).use

include $(PROJECT_ROOT)/pinfo.mk

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk
