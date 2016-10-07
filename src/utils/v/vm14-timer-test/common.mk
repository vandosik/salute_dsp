ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
include $(MKFILES_ROOT)/qmacros.mk

# LIBS+=drvr
EXTRA_SILENT_VARIANTS+=$(SECTION)
USEFILE = $(PROJECT_ROOT)/vm14-timer-test.use
LIBS += vm14timer
LIBS += drvr
# LDFLAGS += -Bstatic

# include $(PROJECT_ROOT)/pinfo.mk

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk
