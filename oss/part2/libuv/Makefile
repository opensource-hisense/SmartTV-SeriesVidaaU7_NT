##############################################################################
# No Warranty                                                                #
# Except as may be otherwise agreed to in writing, no warranties of any      #
# kind, whether express or implied, are given by MTK with respect to any MTK #
# Deliverables or any use thereof, and MTK Deliverables are provided on an   #
# "AS IS" basis.  MTK hereby expressly disclaims all such warranties,        #
# including any implied warranties of merchantability, non-infringement and  #
# fitness for a particular purpose and any warranties arising out of course  #
# of performance, course of dealing or usage of trade.  Parties further      #
# acknowledge that Company may, either presently and/or in the future,       #
# instruct MTK to assist it in the development and the implementation, in    #
# accordance with Company's designs, of certain softwares relating to        #
# Company's product(s) (the "Services").  Except as may be otherwise agreed  #
# to in writing, no warranties of any kind, whether express or implied, are  #
# given by MTK with respect to the Services provided, and the Services are   #
# provided on an "AS IS" basis.  Company further acknowledges that the       #
# Services may contain errors, that testing is important and Company is      #
# solely responsible for fully testing the Services and/or derivatives       #
# thereof before they are used, sublicensed or distributed.  Should there be #
# any third party action brought against MTK, arising out of or relating to  #
# the Services, Company agree to fully indemnify and hold MTK harmless.      #
# If the parties mutually agree to enter into or continue a business         #
# relationship or other arrangement, the terms and conditions set forth      #
# hereunder shall remain effective and, unless explicitly stated otherwise,  #
# shall prevail in the event of a conflict in the terms in any agreements    #
# entered into between the parties.                                          #
##############################################################################
###########################################################################
#
#  Copyright (c) 2004, CrystalMedia Technology, Inc.
#  All rights reserved
#
#  Unauthorized use, practice, perform, copy, distribution, reproduction,
#  or disclosure of this information in whole or in part is prohibited.
###########################################################################

# Copy from oss/source/mak/Makefile
VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
THIS_ROOT    := $(shell pwd)
OSS_ROOT ?= $(THIS_ROOT)/../..

include $(THIS_ROOT)/../../../third_party/source/mak/host.mak
include $(THIS_ROOT)/../mak/oss_version.mak

TOOL_CHAIN ?= 10.2.1
VFP_SUFFIX ?= _neon_ca9
OSS_SRC_ROOT = $(OSS_ROOT)/source
LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_LIB_ROOT = $(OSS_ROOT)/$(LIB_SUB_ROOT)/libuv

#
# Set initial values.
#

PROJECT_ROOT := $(THIS_ROOT)

LIBUV_VERSION ?= 1.20.3


#
# Set warning error parse log/tools
#
export WARNING_LOG  = $(THIS_ROOT)/build.log
export ERROR_LOG    = $(THIS_ROOT)/build_fail.log
export CHK_ERR_WARN_SCRIPT = $(PROJECT_ROOT)/../mak/chk_warn_err.sh
export CHK_ERR_WARN_PARM = $(WARNING_LOG) $(ERROR_LOG)

#
# Set path to source obj library output path
#
export SOURCE_ROOT        ?= $(PROJECT_ROOT)/source
export SOURCE_LIB_ROOT    ?= $(PROJECT_ROOT)/lib
export OBJ_ROOT           ?= $(THIS_ROOT)/obj
export OUTPUT_ROOT        ?= $(THIS_ROOT)/output
export PREFIX              = $(OUTPUT_ROOT)
export FLPRJ_ROOT         := $(THIS_ROOT)

CROSS_HOST                ?= i686-pc-linux-gnu
CROSS_TARGET              ?= armv6z-mediatek-linux-gnueabi

ifeq "$(TOOL_CHAIN)" "4.8.2"
#4.8.2 tool-chain
CROSS_COMPILE ?= armv7a-mediatek482_001_neon-linux-gnueabi-
else
ifeq "$(TOOL_CHAIN)" "5.5.0"
#5.5.0 32 bit tool-chain
CROSS_COMPILE ?=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
else
ifeq "$(TOOL_CHAIN)" "10.2.1"
CROSS_COMPILE ?= $(THIS_ROOT)/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
else
$(error "Tool chain version($(TOOL_CHAIN)) is invalid!")
endif
endif
endif
$(warning "HUCP TOOL_CHAIN $(CROSS_COMPILE)")

CROSS_AR=$(CROSS_COMPILE)ar
CROSS_GCC=$(CROSS_COMPILE)gcc
CROSS_CXX=$(CROSS_COMPILE)g++
CROSS_LINK=$(CROSS_COMPILE)g++
CFLAG='-fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie'
CPPFLAG='-fstack-protector\ -D_FORTIFY_SOURCE=2\ -Wl,-z,noexecstack\ -Wl,-z,noexecheap\ -Wl,-z,relro\ -Wl,-z,now\ -s\ -fPIE\ -pie'

LIBUV_DIR = libuv-v$(LIBUV_VERSION)

.PHONY: all src install ut clean

all: install
	

#CC="$(CC)" CFLAGS="$(CC_FLAG)" CXX="$(CXX)" CXXFLAGS="$(CC_FLAG)"  LD="$(LD)" CPP="$(CPP)" AS="$(AS)" AR="$(AR)" STRIP="$(STRIP)" NM="$(NM)" RANLIB="$(RANL
init:
	sh $(LIBUV_DIR)/autogen.sh && \
	rm -rf $(LIBUV_DIR)/build && \
	mkdir $(LIBUV_DIR)/build && \
	cd $(LIBUV_DIR) && \
	CC=$(CROSS_GCC) AR=$(CROSS_AR) CXX=$(CROSS_CXX) CFLAGS=$(CFLAG)  LINK=$(CROSS_LINK) PLATFORM=linux ./configure --host=$(CROSS_TARGET) --prefix=$(OSS_LIB_ROOT)/$(LIBUV_VERSION)/

src:init
	$(MAKE) CFLAGS=$(CFLAG) -C $(FLPRJ_ROOT)/$(LIBUV_DIR) 

install:init
	$(MAKE) CFLAGS=$(CFLAG) -C $(FLPRJ_ROOT)/$(LIBUV_DIR) install 
	
	
ut:
	$(MAKE) -C $(FLPRJ_ROOT)/$(LIBUV_DIR) check

clean:
	$(MAKE) -C $(FLPRJ_ROOT)/$(LIBUV_DIR) clean
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/libuv_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/libuv_INFO.txt
