# This is the top level Makefile for all source packages. 
# It makes all the code in the "src" directory, then installs it
# to the "install" directory", then makes the images in
# the images directory (if present)

ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
unexport ROOT_DIR

.PHONY: all install clean make_builds clean_builds dummy images prebuilt

# Expands to a single newline character
define NEWLINE


endef

SUFFIXES := .mk

all:
	$(if $(wildcard prebuilt/*), $(MAKE) prebuilt)
	$(if $(wildcard src/*), $(MAKE) -Csrc hinstall)
	$(if $(wildcard src/*), $(MAKE) -Csrc install)
	$(MAKE) make_builds
	$(MAKE) images
	@echo done

subdirs:=$(subst /Makefile,,$(wildcard */[Mm]akefile))

clean:
	$(foreach dir,$(subdirs), $(MAKE) -C$(dir) clean $(NEWLINE))
	-$(RM_HOST) -rv install/*

install: all

make_builds:
	@$(foreach file,$(wildcard install/*/boot/build/*), \
		$(if $(wildcard images/$(notdir $(file))), \
			echo Skip $(wildcard images/$(notdir $(file))), \
			echo Copy $(file) to images/$(notdir $(file)); $(CP_HOST) -n $(file) images/$(notdir $(file)));)

clean_builds:
	@$(if $(wildcard install/*/boot/build/*), \
	$(foreach file,$(wildcard install/*/boot/build/*), \
			echo Delete images/$(notdir $(file)); $(RM_HOST) images/$(notdir $(file)); ), \
	$(foreach file,$(wildcard prebuilt/*/boot/build/*), \
			echo Delete images/$(notdir $(file)); $(RM_HOST) images/$(notdir $(file)); ))
	

images:
	$(if $(wildcard images/*.build), $(MAKE) -Cimages)

prebuilt:
	-$(RM_HOST) -rf install/*
	$(CP_HOST) -r prebuilt/* install/
