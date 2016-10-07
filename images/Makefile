ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

ifeq ($(KPDA_TARGET),)
	ifeq ($(NEUTRINO_TARGET),)
		KPDA_TARGET=$(NEUTRINO_TARGET)
	else
		KPDA_TARGET=$(QNX_TARGET)
	endif
endif

ifeq ($(KPDA_HOST),)
	KPDA_HOST=$(QNX_HOST)
endif

MKSHELL			:= $(KPDA_HOST)/usr/bin/ksh
OS				:= $(shell uname)

HOST_MKIFS		:= mkifs
HOST_MKEFS		:= mkefs

PROCESSOR		:= armle-v7
BSP_PREFIX		:= ../install/$(PROCESSOR)/
KPDA_PREFIX		:= $(KPDA_TARGET)/$(PROCESSOR)/

# we define 'space' explicitly to make appropriate substitution
empty			:=
space			:= $(empty) $(empty)

# default 'mkifs' search paths according to its manual
MKIFS_DIRS		:= sbin usr/sbin boot/sys bin usr/bin lib lib/dll usr/lib usr/photon/lib usr/photon/bin
MKIFS_PATH 		:= $(addprefix $(BSP_PREFIX),$(MKIFS_DIRS))
ifneq ($(INSTALL_ROOT_nto),$(KPDA_TARGET))
MKIFS_PATH		+= $(addprefix $(INSTALL_ROOT_nto)/$(PROCESSOR)/,$(MKIFS_DIRS))
endif
ifneq ($(wildcard $(KPDA_PREFIX)),)
MKIFS_PATH 		+= $(addprefix $(KPDA_PREFIX),$(MKIFS_DIRS))
endif


ifeq ($(OS), Workstation )
	MKIFS_PATH:=$(subst /,\,$(subst $(space),;,$(MKIFS_PATH)))
else
	MKIFS_PATH:=$(subst $(space),:,$(MKIFS_PATH))
endif

export MKIFS_PATH PROCESSOR

SUFFIXES := .build .ifs .bin

.PHONY: all clean

base_name=salute
ifneq ($(b),)
	build = $(base_name)-$(b)
else
	build = $(base_name)-el24d1
endif

all: $(if $(wildcard $(build).build), ifs-$(build).elf)

clean: 
	$(RM_HOST) ifs-* efs-* ipl-* *.sym *$(build).elf

ifs-$(build).elf: $(if $(wildcard $(build).build),$(build).build)
	$(if $(wildcard $(build).build), \
	$(HOST_MKIFS) -vvvv $(MKIFSFLAGS) $^ $@;)

build_all:
	@$(foreach buildf,$(BUILDFILES), $(eval b:=$(shell echo $(buildf) | sed 's/$(base_name)-\(.*\).build/\1/')) $(MAKE) b=$(b) all; )

clean_all:
	@$(foreach buildf,$(BUILDFILES), $(eval b:=$(shell echo $(buildf) | sed 's/$(base_name)-\(.*\).build/\1/')) $(MAKE) b=$(b) clean; )

BUILDFILES:=$(wildcard *.build)
