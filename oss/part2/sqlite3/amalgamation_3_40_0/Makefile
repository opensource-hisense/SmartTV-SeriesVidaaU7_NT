#-----------------------------------------------------------------------------
# Copyright (c) 2010, MediaTek Inc.
# All rights reserved.
#
# Unauthorized use, practice, perform, copy, distribution, reproduction,
# or disclosure of this information in whole or in part is prohibited.
#-----------------------------------------------------------------------------
# $RCSfile:  $
# $Revision:
# $Date:
# $Author:  $
# $CCRevision:  $
# $SWAuthor:  $
# $MD5HEX:  $
#
# Description:
#        A set of files describing the framebuffer interface and core types.
#---------------------------------------------------------------------------*/

DEBUG_ON=0

ifeq ($(DEBUG_ON), 1)
    CFLAGS = -fPIC -g -mapcs-frame -O0 -fsigned-char -fshort-wchar -Wno-strict-aliasing
else
    CFLAGS = -fPIC -g -O2 -fsigned-char -fshort-wchar -Wno-strict-aliasing -march=armv7-a -mtune=cortex-a9 -fPIC -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s
endif

NCT5_ENABLE ?= false
ifeq ($(NCT5_ENABLE), true)
OSS_ROOT ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))
include $(OSS_ROOT)/../build_eng/oss_version.mk
else
VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss
include $(OSS_ROOT)/source/mak/oss_version.mak
endif

TOOL_CHAIN ?= 5.5.0
VFP_SUFFIX ?= _neon_ca9
OSS_LIB_ROOT = $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_TC ?= gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
TOOLCHAIN_ROOT = $(OSS_ROOT)/../tools/mtk_toolchain

ifeq "$(TOOL_CHAIN)" "5.5.0"
    CROSS_COMPILE := $(TOOLCHAIN_ROOT)/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
    PLATFORM_ARCH := arm-linux-gnueabi
    CFLAGS += -mfloat-abi=softfp -mfpu=vfpv3-d16
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    CROSS_COMPILE := $(TOOLCHAIN_ROOT)/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
    PLATFORM_ARCH := arm-none-linux-gnueabihf
    CFLAGS += -mfloat-abi=hard -mfpu=neon-vfpv4
endif

CC=${CROSS_COMPILE}gcc

# For MT5396 above (old)
#ENABLE_CA9 ?= true

# Select tool chain and library
#ifeq "$(ENABLE_VFP)" "true"
#    THIRDPARTY_LIB_ROOT=$(VM_LINUX_ROOT)/third_party/library/gnuarm-$(TOOL_CHAIN)_vfp
#else
#    THIRDPARTY_LIB_ROOT=$(VM_LINUX_ROOT)/third_party/library/gnuarm-$(TOOL_CHAIN)
#endif


SQLITE3_SO=libsqlite3.so.3.40.0

SQLLITE_OBJ=sqlite3.o

THIS_DIR := $(shell pwd)


$(SQLITE3_SO) : $(SQLLITE_OBJ)
	$(CC) -fPIC -shared $(CFLAGS) -o $@ $(SQLLITE_OBJ)

.PHONY: all install clean

all: $(SQLITE3_SO)
	@echo 'Create Sqlite3 library!'

.c.o: .c
	$(CC) $(CFLAGS) $< -c -o $@
	
clean:
	-rm -f $(SQLLITE_OBJ) $(SQLITE3_SO)

install:
	@echo "Install sqlite3 library. OSS_LIB_ROOT=$(OSS_LIB_ROOT) THIS_DIR=$(THIS_DIR)"
	rm -rf $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/lib
	rm -rf $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/include
	mkdir -p $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/include
	mkdir -p $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/lib
	cp -r $(THIS_DIR)/*.h $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/include
	cp -r $(THIS_DIR)/$(SQLITE3_SO) $(OSS_LIB_ROOT)/sqlite3/$(SQLITE3_VERSION)/lib
	@echo 'Release sqlite library finished.'
