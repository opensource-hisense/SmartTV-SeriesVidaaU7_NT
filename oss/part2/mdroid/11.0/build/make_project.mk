#
# File			make_project.mk
# Title			Specify compiler flag/link flag/... variables about project.
# Author		Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

# supernova project.
ifeq ($(PROJECT), supernova)
    PROJ_CFLAGS := -DSUPERNOVA
    PROJ_RELEASE := true
    PROJ_HEADER_PATH := $(PHOTOSPHERE_ROOT)/develop/include
ifeq ($(PLATFORM), linux-x86)
    PROJ_LIBRARY_PATH := $(SIM_LIBPATH)
else
    PROJ_LIBRARY_PATH := $(TARGET_DIRPATH)/$(PROJ_MODE).$(CHIP)/mslib
endif # PLATFORM
    PROJ_EXECUTEABLE_PATH :=

# lg project.
else
ifeq ($(PROJECT), lg)
    PROJ_CFLAGS := -DLG
    PROJ_RELEASE := false

# no project.
else
    PROJ_CFLAGS :=
    PROJ_RELEASE := false

endif # lg
endif # supernova
