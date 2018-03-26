LIBS += drmhelperS

ifndef DEVILIB_ROOT
DEVILIB_ROOT=$(PROJECT_ROOT)/lib
endif

EXTRA_LIBVPATH+=$(DEVILIB_ROOT)/drmhelper/$(OS)/$(CPU)/a.shared.le.v7
