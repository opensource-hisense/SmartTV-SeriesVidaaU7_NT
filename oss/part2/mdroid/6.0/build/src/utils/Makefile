#
# File			Makefile
# Title			Make the utils module in mdroid SDK.
# Author		Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

TOPDIR := ../../..
BUILD_SYSTEM := $(TOPDIR)/build
include $(BUILD_SYSTEM)/make_platform.mk
include $(BUILD_SYSTEM)/make_project.mk
include $(BUILD_SYSTEM)/definitions.mk

# SDK/Module name.
SDK_NAME := mdroid
MODULE_NAME := libutils

# SDK directory.
SDK_DIR := $(TOPDIR)

# Source/Object/Library/Header directory.
SOURCE_DIR  := $(SDK_DIR)/src/$(patsubst lib%,%,$(MODULE_NAME))
OBJECT_DIR  := $(SDK_DIR)/obj/$(PLATFORM)/$(MODULE_NAME)
LIBRARY_DIR := $(SDK_DIR)/lib/$(PLATFORM)
HEADER_DIR  := $(SDK_DIR)/include

# Library file.
LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(STATIC_LIB_SUFFIX)
#(S)LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(SHARE_LIB_SUFFIX)

# Source/Object files.
CPP_SOURCES    := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.cpp))
CPP_OBJECTS_CO := $(notdir $(patsubst %.cpp, %.o, $(CPP_SOURCES)))
CPP_OBJECTS    := $(foreach file, $(CPP_OBJECTS_CO), $(OBJECT_DIR)/$(file))

# Include path.
INCLUDE_PATH := $(HEADER_DIR) \
                $(HEADER_DIR)/bionic/libc/kernel/uapi \
                $(HEADER_DIR)/bionic/libc \
                $(HEADER_DIR)/bionic/libc/include \
                $(HEADER_DIR)/system/core/include \
                $(HEADER_DIR)/nativehelper

ifeq ($(PLATFORM),linux-arm)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/kernel/uapi/arm-asm
else
ifeq ($(PLATFORM),linux-mips)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/kernel/uapi/arm-mips
else
ifeq ($(PLATFORM),linux-x86)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/kernel/uapi/arm-x86
endif # linux-mips
endif # linux-x86
endif # linux-arm

INCLUDES := $(foreach dir, $(INCLUDE_PATH), -I$(dir))
# Define flag.
#DFLAGS :=
# Compiler flag.
LOCAL_CFLAGS := $(DFLAGS) $(INCLUDES)

# Link path/library.
#(S)LOCAL_SHARED_LIBRARIES := libcutils liblog
#(S)ifneq ($(PROJ_LIBRARY_PATH), )
#(S)    LINKS := -L$(PROJ_LIBRARY_PATH)
#(S)endif
#(S)LINKS += -L$(LIBRARY_DIR)
#(S)LINKS += -lpthread
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
$(CPP_OBJECTS): $(OBJECT_DIR)/%.o: %.cpp
	$(transform-cpp-to-o)

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
