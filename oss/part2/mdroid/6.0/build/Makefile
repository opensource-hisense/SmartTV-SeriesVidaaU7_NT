#
# File			Makefile
# Title			Make the mdroid SDK.
# Author		Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

TOPDIR := ..
BUILD_SYSTEM := $(TOPDIR)/build
include $(BUILD_SYSTEM)/make_platform.mk
include $(BUILD_SYSTEM)/make_project.mk
include $(BUILD_SYSTEM)/definitions.mk
#include $(SWCFGPATH_DAILEO)/config.mk
#include $(SWCFGPATH_DAILEO)/default_config.mk

# SDK name.
SDK_NAME := mdroid

# SDK directory.
SDK_DIR := $(TOPDIR)

# SDK librarys.
SDK_LIBRARYS := libc_common libcutils libutils liblog libbinder libselinux libmincrypt

# Object/Library/Header directory.
OBJECT_DIR  := $(SDK_DIR)/obj/$(PLATFORM)
LIBRARY_DIR := $(SDK_DIR)/lib/$(PLATFORM)
HEADER_DIR  := $(SDK_DIR)/include

# Release directory.
ifeq ($(PROJ_RELEASE), true)
    RELEASE_LIBRARY_DIR := $(PROJ_LIBRARY_PATH)
#    RELEASE_LIBRARY_DIR := $(DEVETARGET_DIRPATH)/$(CHIP)/static/hard_float
    RELEASE_HEADER_DIR := $(PROJ_HEADER_PATH)/$(SDK_NAME)
endif

# Library name/file.
ifeq ($(LINK_TYPE), static)
    LIBRARY_NAME := lib$(SDK_NAME)$(STATIC_LIB_SUFFIX)
ifneq (,$(filter 1, $(MSTAR_TVOS_MDMPPLAYER) $(GINGA_MWB_ENABLE)))
    LIBRARY_NAME := lib$(SDK_NAME)$(SHARE_LIB_SUFFIX)
endif
else
    LIBRARY_NAME := lib$(SDK_NAME)$(SHARE_LIB_SUFFIX)
endif
LIBRARY := $(LIBRARY_DIR)/$(LIBRARY_NAME)

# Link path/library.
LOCAL_WHOLE_STATIC_LIBRARIES := $(SDK_LIBRARYS)
ifneq ($(LINK_TYPE), static)
    ifneq ($(PROJ_LIBRARY_PATH), )
        LINKS := -L$(PROJ_LIBRARY_PATH)
    endif
    LINKS += -lpthread
else
ifneq (,$(filter 1, $(MSTAR_TVOS_MDMPPLAYER) $(GINGA_MWB_ENABLE)))
    ifneq ($(PROJ_LIBRARY_PATH), )
        LINKS := -L$(PROJ_LIBRARY_PATH)
    endif
    LINKS += -lpthread
endif
endif
# Link flag.
LOCAL_LDFLAGS := $(LINKS)





.PHONY: $(SDK_LIBRARYS) apps
all: check $(LIBRARY) apps
ifeq ($(PROJ_RELEASE), true)
	@echo ">> Copy library and header files to rleased directory."
	@mkdir -p $(RELEASE_LIBRARY_DIR)
	@cp -rf $(LIBRARY) $(RELEASE_LIBRARY_DIR)/
	@mkdir -p $(RELEASE_HEADER_DIR)
	@cp -ruf $(HEADER_DIR)/* $(RELEASE_HEADER_DIR)/
endif

clean: check
	@echo ">> Remove object and library files."
	@rm -rf $(OBJECT_DIR) $(LIBRARY_DIR)
ifeq ($(PROJ_RELEASE), true)
	@echo ">> Remove library and header files on rleased directory."
	@rm -rf $(RELEASE_LIBRARY_DIR)/$(LIBRARY_NAME) $(RELEASE_HEADER_DIR)
endif


libc_common:
	$(MAKE) -C src/libc_common PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
libcutils:
	$(MAKE) -C src/cutils PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
libutils:
	$(MAKE) -C src/utils PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
liblog:
	$(MAKE) -C src/log PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
libbinder:
	$(MAKE) -C src/binder PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
libselinux:
	$(MAKE) -C src/libselinux PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all
libmincrypt:
	$(MAKE) -C src/libmincrypt PLATFORM=$(PLATFORM) PROJECT=$(PROJECT) all

$(LIBRARY): $(SDK_LIBRARYS)
ifeq ($(LINK_TYPE), static)
	$(transform-o-to-static-lib)
ifneq (,$(filter 1, $(MSTAR_TVOS_MDMPPLAYER) $(GINGA_MWB_ENABLE)))
	$(transform-o-to-shared-lib)
endif
else
	$(transform-o-to-shared-lib)
endif

apps: $(SDK_LIBRARYS)
	make -C apps/servicemanager PLATFORM=$(PLATFORM) all

check:
ifeq ($(PLATFORM), )
	@echo "**************************************************************"
	@echo "* Please assign PLATFORM/PROJECT(option) variable to make.   *"
	@echo "*    PLATFORM can be linux-arm, linux-mips, linux-x86        *"
	@echo "*    PROJECT can be supernova/lg/...                         *"
	@echo "*                                                            *"
	@echo "* e.g: make PLATFORM=linux-arm BINDER_64BIT=0                *"
	@echo "*      make PLATFORM=linux-arm clean                         *"
	@echo "**************************************************************"
	@exit 99
endif
