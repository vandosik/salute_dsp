ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
include $(MKFILES_ROOT)/qmacros.mk ###

LIBS += drvr

define PINFO
PINFO DESCRIPTION=vm14 timer library
endef

include $(MKFILES_ROOT)/qtargets.mk

-include $(PROJECT_ROOT)/roots.mk
