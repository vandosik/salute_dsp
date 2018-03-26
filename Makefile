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

.PHONY: all install clean links make_links clean_links dummy images prebuilt

# Expands to a single newline character
define NEWLINE


endef

SUFFIXES := .mk

all: install links $(if $(wildcard images/*),images)
	@echo done

subdirs:=$(subst /Makefile,,$(wildcard */[Mm]akefile))

clean: clean_links
	$(foreach dir,$(subdirs), $(MAKE) -C$(dir) clean $(NEWLINE))
	-$(RM_HOST) -r install/*

install: $(if $(wildcard prebuilt/*),prebuilt)
	$(if $(wildcard src/*), $(MAKE) -Csrc hinstall)
	$(if $(wildcard src/*), $(MAKE) -Csrc install)

#
# Have to invoke "make_links" target because the first make expands
# the $(wildcard ...) too soon - we might not have copied things into
# the "install" tree yet.
#
links:
	$(MAKE) make_links

make_links:	
	$(foreach file,$(wildcard install/*/boot/build/*),cd images;$(CP_HOST) -n ../$(file) $(notdir $(file));cd ..; )

clean_links:	
#	$(foreach file,$(wildcard install/*/boot/build/*),cd images;$(RM_HOST) $(notdir $(file));cd ..; )

images:
	$(MAKE) -Cimages

prebuilt:
	mkdir -p install
	cp -r prebuilt/* install
