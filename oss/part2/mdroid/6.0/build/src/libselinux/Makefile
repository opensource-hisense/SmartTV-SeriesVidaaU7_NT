#
# File			Makefile
# Title			Make the selinux module in mdroid SDK.
# Author		Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

TOPDIR := ../../..
BUILD_SYSTEM := $(TOPDIR)/build
include $(BUILD_SYSTEM)/make_platform.mk
include $(BUILD_SYSTEM)/make_project.mk
include $(BUILD_SYSTEM)/definitions.mk
#include $(SWCFGPATH_DAILEO)/config.mk
#include $(SWCFGPATH_DAILEO)/default_config.mk

# SDK/Module name.
SDK_NAME := mdroid
MODULE_NAME := libselinux

# SDK directory.
SDK_DIR := $(TOPDIR)

# Source/Object/Library/Header directory.
SOURCE_DIR  := $(SDK_DIR)/src/$(MODULE_NAME)/src/
OBJECT_DIR  := $(SDK_DIR)/obj/$(PLATFORM)/$(MODULE_NAME)
LIBRARY_DIR := $(SDK_DIR)/lib/$(PLATFORM)
HEADER_DIR  := $(SDK_DIR)/include

# Library file.
LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(STATIC_LIB_SUFFIX)
#(S)LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(SHARE_LIB_SUFFIX)

# Source/Object files.
CPP_SOURCES    := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.c))
CPP_OBJECTS_CO := $(notdir $(patsubst %.c, %.o, $(CPP_SOURCES)))
CPP_OBJECTS    := $(foreach file, $(CPP_OBJECTS_CO), $(OBJECT_DIR)/$(file))

# Include path.
INCLUDE_PATH := $(HEADER_DIR) \
                $(HEADER_DIR)/system/core/include \
                $(HEADER_DIR)/bionic/libc \
		$(HEADER_DIR)/../src/$(MODULE_NAME)/include \
                $(HEADER_DIR)/bionic/libc/include

INCLUDES := $(foreach dir, $(INCLUDE_PATH), -I$(dir))
# Define flag.
ifeq ($(BINDER_64BIT), 1)
DFLAGS := -DBINDER_64BIT=1
else
DFLAGS := -DBINDER_64BIT=0
endif
# Compiler flag.
LOCAL_CFLAGS := $(DFLAGS) $(INCLUDES)
LOCAL_CFLAGS += -DAUDITD_LOG_TAG=1003 -DHOST
# Link path/library.
#(S)ifneq ($(PROJ_LIBRARY_PATH), )
#(S)    LINKS := -L$(PROJ_LIBRARY_PATH)
#(S)endif
#(S)LINKS += -L$(LIBRARY_DIR)
# Link flag.
LOCAL_LDFLAGS := $(LINKS)





all: check $(LIBRARY)
	@echo "*********************************************************************"
	@echo "* Platform: $(PLATFORM), Project: $(PROJECT)  MODULE: $(MODULE_NAME)"
	@echo "*********************************************************************"

clean: check
	@echo ">> Remove object and library files."
	@rm -rf $(OBJECT_DIR) $(LIBRARY)
	@echo "*********************************************************************"
	@echo "* Platform: $(PLATFORM), Project: $(PROJECT)  MODULE: $(MODULE_NAME)"
	@echo "*********************************************************************"

VPATH = $(SOURCE_DIR)
$(CPP_OBJECTS):  $(OBJECT_DIR)/%.o: %.c
	$(transform-c-to-o)

$(LIBRARY): $(all_objects)
	$(transform-o-to-static-lib)
#(S)	$(transform-o-to-shared-lib)

check:
ifeq ($(PLATFORM), )
	@echo "**************************************************************"
	@echo "* Please assign PLATFORM/PROJECT(option) variable to make.   *"
	@echo "*    PLATFORM can be linux-arm, linux-mips, linux-x86        *"
	@echo "*    PROJECT can be supernova/lg/...                         *"
	@echo "*                                                            *"
	@echo "* e.g: make PLATFORM=linux-arm                               *"
	@echo "*      make PLATFORM=linux-arm clean                         *"
	@echo "**************************************************************"
	@exit 99
endif
