#
# File			Makefile
# Title			Make the binder client app in mdroid SDK.
# Author		Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#
TOPDIR := ../../..
BUILD_SYSTEM := $(TOPDIR)/build
include $(BUILD_SYSTEM)/make_platform.mk
include $(BUILD_SYSTEM)/make_project.mk
include $(BUILD_SYSTEM)/definitions.mk
# SDK/Application name.
SDK_NAME := mdroid
APP_NAME := binderclient
# SDK directory.
SDK_DIR := $(TOPDIR)
# Source/Object/Executeable directory.
SOURCE_DIR      := $(SDK_DIR)/apps/$(APP_NAME)
OBJECT_DIR      := $(SDK_DIR)/obj/$(PLATFORM)/$(APP_NAME)
LIBRARY_DIR     := $(SDK_DIR)/lib/$(PLATFORM)
EXECUTEABLE_DIR := $(SDK_DIR)/bin/$(PLATFORM)
# Executeable file.
EXECUTEABLE := $(EXECUTEABLE_DIR)/$(APP_NAME)$(EXECUTABLE_SUFFIX)
# Source/Object files.
CPP_SOURCES    := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.cpp))
CPP_OBJECTS_CO := $(notdir $(patsubst %.cpp, %.o, $(CPP_SOURCES)))
CPP_OBJECTS    := $(foreach file, $(CPP_OBJECTS_CO), $(OBJECT_DIR)/$(file))
# Include path.
INCLUDE_PATH := $(SOURCE_DIR) \
                $(SDK_DIR)/include \
                $(SDK_DIR)/include/system/core/include \
                $(SDK_DIR)/include/frameworks/native/include \
                $(SDK_DIR)/bionic/libc/private \
                $(SDK_DIR)/include/bionic/libc/include
INCLUDES := $(foreach dir, $(INCLUDE_PATH), -I$(dir))
# Define flag.
#DFLAGS :=
# Compiler flag.
LOCAL_CFLAGS := $(DFLAGS) $(INCLUDES)
# Link path/library.
ifeq ($(LINK_TYPE), static)
    LOCAL_STATIC_LIBRARIES := libmdroid
else
LOCAL_SHARED_LIBRARIES := libmdroid
endif
ifneq ($(PROJ_LIBRARY_PATH), )
    LINKS := -L$(PROJ_LIBRARY_PATH) \
             -Wl,--rpath-link $(PROJ_LIBRARY_PATH)
endif
LINKS += -L$(LIBRARY_DIR)
LINKS += -lpthread
# Link flag.
LOCAL_LDFLAGS := $(LINKS)
all: check $(EXECUTEABLE)
	@echo "*********************************************************************"
	@echo "* Platform: $(PLATFORM), Project: $(PROJECT)  APP: $(APP_NAME)"
	@echo "*********************************************************************"
clean: check
	@echo ">> Remove object and execute files."
	@rm -rf $(OBJECT_DIR) $(EXECUTEABLE)
	@echo "*********************************************************************"
	@echo "* Platform: $(PLATFORM), Project: $(PROJECT)  APP: $(APP_NAME)"
	@echo "*********************************************************************"
VPATH = $(SOURCE_DIR)
$(CPP_OBJECTS): $(OBJECT_DIR)/%.o: %.cpp
	$(transform-cpp-to-o)
$(EXECUTEABLE): $(all_objects)
	$(transform-o-to-executable)
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
