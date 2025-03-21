###############################################################################
# Copyright Statement:                                                        #
#                                                                             #
#   This software/firmware and related documentation ("MediaTek Software")    #
# are protected under international and related jurisdictions'copyright laws  #
# as unpublished works. The information contained herein is confidential and  #
# proprietary to MediaTek Inc. Without the prior written permission of        #
# MediaTek Inc., any reproduction, modification, use or disclosure of         #
# MediaTek Software, and information contained herein, in whole or in part,   #
# shall be strictly prohibited.                                               #
# MediaTek Inc. Copyright (C) 2010. All rights reserved.                      #
#                                                                             #
#   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND      #
# AGREES TO THE FOLLOWING:                                                    #
#                                                                             #
#   1)Any and all intellectual property rights (including without             #
# limitation, patent, copyright, and trade secrets) in and to this            #
# Software/firmware and related documentation ("MediaTek Software") shall     #
# remain the exclusive property of MediaTek Inc. Any and all intellectual     #
# property rights (including without limitation, patent, copyright, and       #
# trade secrets) in and to any modifications and derivatives to MediaTek      #
# Software, whoever made, shall also remain the exclusive property of         #
# MediaTek Inc.  Nothing herein shall be construed as any transfer of any     #
# title to any intellectual property right in MediaTek Software to Receiver.  #
#                                                                             #
#   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its  #
# representatives is provided to Receiver on an "AS IS" basis only.           #
# MediaTek Inc. expressly disclaims all warranties, expressed or implied,     #
# including but not limited to any implied warranties of merchantability,     #
# non-infringement and fitness for a particular purpose and any warranties    #
# arising out of course of performance, course of dealing or usage of trade.  #
# MediaTek Inc. does not provide any warranty whatsoever with respect to the  #
# software of any third party which may be used by, incorporated in, or       #
# supplied with the MediaTek Software, and Receiver agrees to look only to    #
# such third parties for any warranty claim relating thereto.  Receiver       #
# expressly acknowledges that it is Receiver's sole responsibility to obtain  #
# from any third party all proper licenses contained in or delivered with     #
# MediaTek Software.  MediaTek is not responsible for any MediaTek Software   #
# releases made to Receiver's specifications or to conform to a particular    #
# standard or open forum.                                                     #
#                                                                             #
#   3)Receiver further acknowledge that Receiver may, either presently        #
# and/or in the future, instruct MediaTek Inc. to assist it in the            #
# development and the implementation, in accordance with Receiver's designs,  #
# of certain softwares relating to Receiver's product(s) (the "Services").    #
# Except as may be otherwise agreed to in writing, no warranties of any       #
# kind, whether express or implied, are given by MediaTek Inc. with respect   #
# to the Services provided, and the Services are provided on an "AS IS"       #
# basis. Receiver further acknowledges that the Services may contain errors   #
# that testing is important and it is solely responsible for fully testing    #
# the Services and/or derivatives thereof before they are used, sublicensed   #
# or distributed. Should there be any third party action brought against      #
# MediaTek Inc. arising out of or relating to the Services, Receiver agree    #
# to fully indemnify and hold MediaTek Inc. harmless.  If the parties         #
# mutually agree to enter into or continue a business relationship or other   #
# arrangement, the terms and conditions set forth herein shall remain         #
# effective and, unless explicitly stated otherwise, shall prevail in the     #
# event of a conflict in the terms in any agreements entered into between     #
# the parties.                                                                #
#                                                                             #
#   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and     #
# cumulative liability with respect to MediaTek Software released hereunder   #
# will be, at MediaTek Inc.'s sole discretion, to replace or revise the       #
# MediaTek Software at issue.                                                 #
#                                                                             #
#   5)The transaction contemplated hereunder shall be construed in            #
# accordance with the laws of Singapore, excluding its conflict of laws       #
# principles.  Any disputes, controversies or claims arising thereof and      #
# related thereto shall be settled via arbitration in Singapore, under the    #
# then current rules of the International Chamber of Commerce (ICC).  The     #
# arbitration shall be conducted in English. The awards of the arbitration    #
# shall be final and binding upon both parties and shall be entered and       #
# enforceable in any court of competent jurisdiction.                         #
###############################################################################
###########################################################################
# $RCSfile: Makefile,v $
# $Revision: #1 $
# $Date: 2010/06/17 $
# $Author: richard.lai $
# $SWAuthor: Richard Lai $
#
# Description:
#         Makefile to build a "LVM2".
#
#############################################################################
TOOL_CHAIN ?= 5.5.0
CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
PLATFORM_ARCH ?= armv7a-mediatek-linux-gnueabi

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++
export VFP_SUFFIX = _neon_ca9

CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fPIC

VM_LINUX_ROOT := $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT := $(VM_LINUX_ROOT)/oss
OSS_OUTPUT := $(OSS_ROOT)/output
OSS_SRC_ROOT := $(OSS_ROOT)/source
OSS_LIB_ROOT := $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
LVM2_SRC_ROOT := $(OSS_SRC_ROOT)/lvm2/LVM2
LVM2_BUILD_ROOT := $(OSS_OUTPUT)/lvm2/build
LVM2_INSTALL_ROOT := $(OSS_LIB_ROOT)/lvm2

.PHONY: all install clean

all: $(LVM2_BUILD_ROOT)/.configured $(LVM2_BUILD_ROOT)/tools/dmsetup

install: all
	@echo "Install LVM2."
	@mkdir -p $(LVM2_INSTALL_ROOT)/bin
	@mkdir -p $(LVM2_INSTALL_ROOT)/lib
	@cp -f $(LVM2_BUILD_ROOT)/tools/dmsetup $(LVM2_INSTALL_ROOT)/bin
	@cp -f $(LVM2_BUILD_ROOT)/libdm/ioctl/libdevmapper.so.1.02 $(LVM2_INSTALL_ROOT)/lib
	@echo "Done."

clean:
	@echo "Clean LVM2."
	@rm -rf $(LVM2_BUILD_ROOT)
	@echo "Done."

$(LVM2_BUILD_ROOT)/.configured:
	@echo "Configure LVM2."
	@chmod +x $(LVM2_SRC_ROOT)/configure
	@mkdir -p $(LVM2_BUILD_ROOT)
	@(cd $(LVM2_BUILD_ROOT); \
	echo "ac_cv_func_malloc_0_nonnull=yes" > arm-linux.cache; \
	$(LVM2_SRC_ROOT)/configure --build=x86-linux --host=arm-linux --cache-file=arm-linux.cache \
	);
	@touch $(LVM2_BUILD_ROOT)/.configured
	@chmod -x $(LVM2_SRC_ROOT)/configure
	@echo "Done."

$(LVM2_BUILD_ROOT)/tools/dmsetup:
	@echo "Make LVM2."
	@make -C $(LVM2_BUILD_ROOT) device-mapper
	@echo "Done."

notice:
	cp -rf NOTICE.$(LVM2_VERSION).txt $(OSS_NOTICE_OUTPUT)/lvm2_NOTICE.txt

info:
	cp -rf OSS_INFO.$(LVM2_VERSION).txt $(OSS_INFO_OUTPUT)/lvm2_INFO.txt

