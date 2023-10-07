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
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss

include $(VM_LINUX_ROOT)/linux_mts/build/mak/host.mak
include $(OSS_ROOT)/source/mak/oss_version.mak

TOOL_CHAIN ?= 4.8.2
VFP_SUFFIX ?= _neon_ca9
OSS_SRC_ROOT = $(OSS_ROOT)/source
LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_LIB_ROOT = $(OSS_ROOT)/$(LIB_SUB_ROOT)/cjson

#
# Set initial values.
#
THIS_ROOT    := $(shell pwd)
PROJECT_ROOT := $(THIS_ROOT)

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

CROSS_COMPILE = armv7a-mediatek482_001_neon-linux-gnueabi-
CROSS_AR=$(CROSS_COMPILE)ar
CROSS_GCC=$(CROSS_COMPILE)gcc
CROSS_CXX=$(CROSS_COMPILE)g++
CROSS_LINK=$(CROSS_COMPILE)g++

.PHONY: all src install clean

all:

src:

install:

clean:

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/cJSON_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/cJSON_INFO.txt
