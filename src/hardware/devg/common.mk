#  
# Copyright 2007, 2008, QNX Software Systems. 
#  
# Licensed under the Apache License, Version 2.0 (the "License"). You 
# may not reproduce, modify or distribute this software except in 
# compliance with the License. You may obtain a copy of the License 
# at: http://www.apache.org/licenses/LICENSE-2.0 
#  
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OF ANY KIND, either express or implied.
# 
# This file may contain contributions from others, either as 
# contributors under the License or as licensors under other terms.  
# Please review this entire file for other proprietary rights or license 
# notices, as well as the QNX Development Suite License Guide at 
# http://licensing.qnx.com/license-guide/ for other information.
# #
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=
endef

EXTRA_SILENT_VARIANTS = $(subst -, ,$(SECTION))

ifeq ($(NAME),)
NAME=$(PROJECT)-$(SECTION)
endif

CCVFLAG_fixed=-DDISP_FIXED_POINT

DEVGDRVROOT=$(shell $(PWD_HOST) | sed 's!\/devg.*!\/devg!')/$(shell $(PWD_HOST) | sed 's!.*\/devg[^\/]*\/!!' | sed 's!\/.*!!')

include $(MKFILES_ROOT)/qmacros.mk

-include $(PROJECT_ROOT)/roots.mk

#
# Set this as the default, each devg's pinfo.mk can override if needed.
#
SHARED_LIBS=ffb disputilS

include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk


USEFILE := ../../../$(SECTION).use
CONFIG_PATH := $(PROJECT_ROOT)/$(SECTION)/config

#EXTRA_INCVPATH += $(if $(USE_INSTALL_ROOT),$(INSTALL_ROOT_$(OS)),$(USE_ROOT_$(OS)))/usr/include/graphics
EXTRA_INCVPATH += $(PROJECT_ROOT)/public/graphics
EXTRA_INCVPATH += $(PROJECT_ROOT)/public
EXTRA_INCVPATH += $(PROJECT_ROOT)/private
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux/include
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux/include/uapi
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux/arch/$(CPU)
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux/arch/$(CPU)/include
EXTRA_INCVPATH += $(PROJECT_ROOT)/private/linux/arch/$(CPU)/include/uapi

EXTRA_INCVPATH+=$(INSTALL_ROOT_nto)/usr/include/xilinx
EXTRA_LIBVPATH+=$(INSTALL_ROOT_nto)/usr/lib/xilinx

include $(MKFILES_ROOT)/qtargets.mk

ifneq ($(OS), qnx4)
ifneq ($(COMPOUND_VARIANT),dll)
ifneq ($(COMPOUND_VARIANT),dll.g)
SHARED_LIBDIR = $(OS)/$(CPU)/so.$(patsubst dll.%,%,$(COMPOUND_VARIANT))
STATIC_LIBDIR = $(OS)/$(CPU)/a.shared.$(patsubst dll.%,%,$(COMPOUND_VARIANT))
else
SHARED_LIBDIR = $(OS)/$(CPU)/so.g
STATIC_LIBDIR = $(OS)/$(CPU)/a.shared.g
endif
else
SHARED_LIBDIR = $(OS)/$(CPU)/so
STATIC_LIBDIR = $(OS)/$(CPU)/a.shared
endif

-include $(DEVGDRVROOT)/extrasrc.mk

ifeq ($(origin NDEBUG),undefined)
CCFLAGS += -O0
else
CCFLAGS += -fomit-frame-pointer
endif

else	# qnx4
ifneq ($(COMPOUND_VARIANT), a.g)
SHARED_LIBDIR = $(OS)/$(CPU)/a
STATIC_LIBDIR = $(OS)/$(CPU)/a
SHARED_LIBS += iographics photon3r $(QNX4_LIBS)
else
SHARED_LIBDIR = $(OS)/$(CPU)/a.g
STATIC_LIBDIR = $(OS)/$(CPU)/a.g
SHARED_LIBS += iographics photon3r $(QNX4_LIBS)
endif
SHARED_LIBS += $(QNX4_LIBS)
LDFLAGS += -T1
CCFLAGS += -Otax -D__X86__ -D__LITTLEENDIAN__
endif

ifeq ($(origin NDEBUG),undefined)
ifeq ($(OS), qnx4)
LIBS += $(foreach lib, $(STATIC_LIBS), $(lib)S) $(SHARED_LIBS)
else
LIBS += $(foreach lib, $(STATIC_LIBS), $(lib)S_g) $(foreach lib, $(SHARED_LIBS), $(lib)_g)
endif
else
LIBS += $(foreach lib, $(STATIC_LIBS), $(lib)S) $(SHARED_LIBS)
endif
WIN32_ENVIRON=mingw

ifeq "$(findstring e2k,$(CPU))" "e2k"
CCFLAGS += -D__DEF_BELOW4G_TMEM__=\"/below4G/ram\"
CCFLAGS += -fcontrol-spec
endif
