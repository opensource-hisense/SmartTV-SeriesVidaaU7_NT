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
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
TOP ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))/..
TOOLCHAIN_ROOT ?= $(abspath $(TOP))/tools/mtk_toolchain
OSS_SRC_ROOT ?= $(abspath $(TOP))/oss/source

ifndef AVB_PURELINUX_ENABLE
export AVB_PURELINUX_ENABLE ?= true
endif

include $(abspath $(TOP))/build/oss_version.mk

ifndef TOOL_CHAIN
export TOOL_CHAIN ?= 10.2.1
endif

ifndef KERNEL_VER
export KERNEL_VER ?= linux-4.9
endif

ENABLE_CA9_NEON ?= true

OSS_LIB_ROOT  ?=
OSS_OUTPUT     ?=

ifeq "$(TOOL_CHAIN)" "5.5.0"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin
    TOOL_CHAIN_HOST := arm-linux-gnueabi
    OSS_LIB_ROOT := $(abspath $(TOP))/oss/library/gnuarm-5.5.0_neon_ca9
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-none-linux-gnueabihf-10.2.1/bin
    TOOL_CHAIN_HOST := arm-none-linux-gnueabihf
    OSS_LIB_ROOT := $(abspath $(TOP))/oss/library/gnuarm-10.2.1_neon_ca9
endif

ifndef CROSS_COMPILE
    ifneq (,$(findstring $(TOOL_CHAIN), 5.5.0))
            CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-linux-gnueabi-
    else ifneq (,$(findstring $(TOOL_CHAIN), 10.2.1))
            CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-none-linux-gnueabihf-
    endif
endif

export PATH := $(TOOL_CHAIN_BIN_PATH):$(PATH)