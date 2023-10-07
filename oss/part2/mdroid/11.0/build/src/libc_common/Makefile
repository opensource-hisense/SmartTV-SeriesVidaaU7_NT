#
# File			Makefile
# Title			Make the log module in mdroid SDK.
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
MODULE_NAME := libc_common

# SDK directory.
SDK_DIR := $(TOPDIR)

# Source/Object/Library/Header directory.
SOURCE_DIR  := $(SDK_DIR)/src/$(MODULE_NAME)
OBJECT_DIR  := $(SDK_DIR)/obj/$(PLATFORM)/$(MODULE_NAME)
LIBRARY_DIR := $(SDK_DIR)/lib/$(PLATFORM)
HEADER_DIR  := $(SDK_DIR)/include

# Library file.
LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(STATIC_LIB_SUFFIX)
#(S)LIBRARY := $(LIBRARY_DIR)/$(MODULE_NAME)$(SHARE_LIB_SUFFIX)

# Source/Object files.


CXX_SOURCES := bionic/system_properties.cpp
#CXX_SOURCES += bionic/libc_logging.cpp
CXX_SOURCES += bionic/prop_area.cpp
CXX_SOURCES += bionic/system_property_api.cpp
CXX_SOURCES += bionic/system_property_set.cpp
CXX_SOURCES += bionic/android_set_abort_message.cpp
CXX_SOURCES += bionic/async_safe_log.cpp
CXX_SOURCES += bionic/prop_info.cpp
CXX_SOURCES += bionic/contexts_serialized.cpp
CXX_SOURCES += bionic/contexts_split.cpp
CXX_SOURCES += bionic/context_node.cpp
#CXX_SOURCES += bionic/property_info_parser.cpp

#ifeq ($(PLATFORM),linux-arm)
#    ASM_SOURCES += arch-arm/bionic/futex_arm.S
#else
#ifeq ($(PLATFORM),linux-mips)
#    ASM_SOURCES += arch-arm/bionic/futex_mips.S
#else
#ifeq ($(PLATFORM),linux-x86)
#    ASM_SOURCES += arch-x86/bionic/futex_x86.S
#endif # linux-mips
#endif # linux-x86
#endif # linux-arm
CXX_OBJECTS_CO := $(patsubst %.cpp, %.o, $(CXX_SOURCES))
CPP_OBJECTS_CO := $(patsubst %.c, %.o, $(CPP_SOURCES))
ASM_OBJECTS_CO := $(patsubst %.S, %.o, $(ASM_SOURCES))
CXX_OBJECTS    := $(foreach file, $(CXX_OBJECTS_CO), $(OBJECT_DIR)/$(file))
CPP_OBJECTS    := $(foreach file, $(CPP_OBJECTS_CO), $(OBJECT_DIR)/$(file))
ASM_OBJECTS    := $(foreach file, $(ASM_OBJECTS_CO), $(OBJECT_DIR)/$(file))


# Include path.
INCLUDE_PATH := $(HEADER_DIR) \
                $(HEADER_DIR)/system_properties/ \
                $(HEADER_DIR)/bionic/libc/kernel/uapi \
                $(HEADER_DIR)/bionic/libc/include \
                $(HEADER_DIR)/bionic/libc/include/system_properties \
                $(HEADER_DIR)/bionic/libc/ \
                $(HEADER_DIR)/bionic/libc/private

ifeq ($(PLATFORM),linux-arm)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/arch-arm/include
else
ifeq ($(PLATFORM),linux-x86)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/arch-x86/include
endif # linux-x86
endif # linux-arm
INCLUDES := $(foreach dir, $(INCLUDE_PATH), -I$(dir))
# Define flag.
#DFLAGS :=
# Compiler flag.
LOCAL_CFLAGS := $(DFLAGS) $(INCLUDES)

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
$(CXX_OBJECTS): $(OBJECT_DIR)/%.o: %.cpp
	$(transform-cpp-to-o)

$(CPP_OBJECTS): $(OBJECT_DIR)/%.o: %.c
	$(transform-c-to-o)

$(ASM_OBJECTS): $(OBJECT_DIR)/%.o: %.S
	$(transform-s-to-o)

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
