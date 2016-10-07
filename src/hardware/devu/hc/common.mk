ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

EXCLUDE_HCLIST=isp1161

LIB_VARIANT = $(patsubst dll.,dll,$(subst $(space),.,so $(filter wcc be le,$(VARIANTS))))

EXTRA_CCDEPS = $(wildcard $(PROJECT_ROOT)/*.h $(PRODUCT_ROOT)/public/sys/io-usb.h )

CCFLAGS +=
LDFLAGS += -M

EXTRA_INCVPATH += $(PROJECT_ROOT)/$(SECTION)/private

define PINFO
PINFO DESCRIPTION=
endef

include $(MKFILES_ROOT)/qmacros.mk

-include $(PROJECT_ROOT)/roots.mk

TYPE = $(firstword $(filter a o, $(VARIANTS)) o)

NAME = devu

USEFILE_a = 
USEFILE_o = $(SECTION_ROOT)/$(NAME)-$(SECTION).use
ifndef USEFILE
	USEFILE   = $(USEFILE_$(TYPE))
endif

CCFLAGS_o =
CCFLAGS_a = -Dmain=main_$(SECTION)
CCFLAGS   = $(CCFLAGS_$(TYPE))

-include $(SECTION_ROOT)/override.mk
-include $(SECTION_ROOT)/pinfo.mk
-include $(SECTION_ROOT)/extra.mk

include $(MKFILES_ROOT)/qtargets.mk
