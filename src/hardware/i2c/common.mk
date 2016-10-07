ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=I2C protocol driver
endef

INSTALLDIR = sbin
include $(MKFILES_ROOT)/qmacros.mk

-include $(PROJECT_ROOT)/roots.mk

USEFILE=$(PROJECT_ROOT)/$(SECTION)/$(NAME).use
EXTRA_SRCVPATH = $(EXTRA_SRCVPATH_$(SECTION))
-include $(PROJECT_ROOT)/$(SECTION)/extra.mk

LIBS += drvrS

PRE_SRCVPATH = $(foreach var,$(filter a, $(VARIANTS)),$(CPU_ROOT)/$(subst $(space),.,$(patsubst a,dll,$(filter-out g, $(VARIANTS)))))



include $(MKFILES_ROOT)/qtargets.mk
