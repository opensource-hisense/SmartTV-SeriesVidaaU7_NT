#
# File			Makefile
# Title			Make the cutils module in mdroid SDK.
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
MODULE_NAME := libcutils

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
#CPP_SOURCES := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.c))
#ASM_SOURCES := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.S))
#CPP_SOURCES += properties.c \
               native_handle.c \
               threads.c \
               ashmem-dev.c  \
               multiuser.c  \
               uevent.c  \
               android_get_control_file.c
#
#GGGGGGGGG
CPP_SOURCES := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.cpp))
#CPP_SOURCES = properties.cpp \
              threads.cpp \
              android_get_control_file.cpp \
              ashmem-dev.cpp  \
              multiuser.cpp  \
              uevent.cpp \
              native_handle.cpp
#
ifeq ($(PLATFORM),linux-arm64)
    ASM_SOURCES += arch-arm64/android_memset.S
else
ifeq ($(PLATFORM),linux-arm)
    ASM_SOURCES += arch-arm/memset32.S
else
ifeq ($(PLATFORM),x86-atom)
    DFLAGS += -DHAVE_MEMSET16 -DHAVE_MEMSET32
    ASM_SOURCES += arch-x86/android_memset16.S arch-x86/android_memset32.S
    CPP_SOURCES += memory.c
else
    CPP_SOURCES += memory.c
endif # x86-atom
endif # linux-arm
endif # linux-arm64
CPP_OBJECTS_CO := $(notdir $(patsubst %.cpp, %.o, $(CPP_SOURCES)))
ASM_OBJECTS_CO := $(patsubst %.S, %.o, $(ASM_SOURCES))
CPP_OBJECTS    := $(foreach file, $(CPP_OBJECTS_CO), $(OBJECT_DIR)/$(file))
ASM_OBJECTS    := $(foreach file, $(ASM_OBJECTS_CO), $(OBJECT_DIR)/$(file))

# Include path.
INCLUDE_PATH := $(HEADER_DIR) \
                $(HEADER_DIR)/bionic/libc/include \
                $(HEADER_DIR)/bionic/libc/kernel/uapi \
                $(HEADER_DIR)/system/core/include \
                $(HEADER_DIR)/system/core/include/log \
                $(HEADER_DIR)/system/core/include/cutils \
                $(HEADER_DIR)/bionic/libc/private \
                $(HEADER_DIR)/hardware

ifeq ($(PLATFORM),linux-arm64)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/arch-arm64/include
endif
ifeq ($(PLATFORM),linux-arm)
INCLUDE_PATH += $(HEADER_DIR)/bionic/libc/arch-arm/include
endif
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
ifeq ($(TARGET_CPU_SMP),true)
DFLAGS := -DANDROID_SMP=1
else
DFLAGS := -DANDROID_SMP=0
endif
# Compiler flag.
LOCAL_CFLAGS := $(DFLAGS) $(INCLUDES)

# Link path/library.
#(S)LOCAL_SHARED_LIBRARIES := libc_common liblog
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
$(CPP_OBJECTS): $(OBJECT_DIR)/%.o: %.cpp
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
