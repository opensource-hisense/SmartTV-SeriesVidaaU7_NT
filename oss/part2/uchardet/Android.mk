#<MStar Software>
#******************************************************************************
# MStar Software
# Copyright (c) 2010 - 2018 MStar Semiconductor, Inc. All rights reserved.
# All software, firmware and related documentation herein ("MStar Software") are
# intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
# law, including, but not limited to, copyright law and international treaties.
# Any use, modification, reproduction, retransmission, or republication of all
# or part of MStar Software is expressly prohibited, unless prior written
# permission has been granted by MStar.
#
# By accessing, browsing and/or using MStar Software, you acknowledge that you
# have read, understood, and agree, to be bound by below terms ("Terms") and to
# comply with all applicable laws and regulations:
#
# 1. MStar shall retain any and all right, ownership and interest to MStar
#    Software and any modification/derivatives thereof.
#    No right, ownership, or interest to MStar Software and any
#    modification/derivatives thereof is transferred to you under Terms.
#
# 2. You understand that MStar Software might include, incorporate or be
#    supplied together with third party's software and the use of MStar
#    Software may require additional licenses from third parties.
#    Therefore, you hereby agree it is your sole responsibility to separately
#    obtain any and all third party right and license necessary for your use of
#    such third party's software.
#
# 3. MStar Software and any modification/derivatives thereof shall be deemed as
#    MStar's confidential information and you agree to keep MStar's
#    confidential information in strictest confidence and not disclose to any
#    third party.
#
# 4. MStar Software is provided on an "AS IS" basis without warranties of any
#    kind. Any warranties are hereby expressly disclaimed by MStar, including
#    without limitation, any warranties of merchantability, non-infringement of
#    intellectual property rights, fitness for a particular purpose, error free
#    and in conformity with any international standard.  You agree to waive any
#    claim against MStar for any loss, damage, cost or expense that you may
#    incur related to your use of MStar Software.
#    In no event shall MStar be liable for any direct, indirect, incidental or
#    consequential damages, including without limitation, lost of profit or
#    revenues, lost or damage of data, and unauthorized system use.
#    You agree that this Section 4 shall still apply without being affected
#    even if MStar Software has been modified by MStar in accordance with your
#    request or instruction for your use, except otherwise agreed by both
#    parties in writing.
#
# 5. If requested, MStar may from time to time provide technical supports or
#    services in relation with MStar Software to you for your use of
#    MStar Software in conjunction with your or your customer's product
#    ("Services").
#    You understand and agree that, except otherwise agreed by both parties in
#    writing, Services are provided on an "AS IS" basis and the warranty
#    disclaimer set forth in Section 4 above shall apply.
#
# 6. Nothing contained herein shall be construed as by implication, estoppels
#    or otherwise:
#    (a) conferring any license or right to use MStar name, trademark, service
#        mark, symbol or any other identification;
#    (b) obligating MStar or any of its affiliates to furnish any person,
#        including without limitation, you and your customers, any assistance
#        of any kind whatsoever, or any information; or
#    (c) conferring any license or right under any intellectual property right.
#
# 7. These terms shall be governed by and construed in accordance with the laws
#    of Taiwan, R.O.C., excluding its conflict of law rules.
#    Any and all dispute arising out hereof or related hereto shall be finally
#    settled by arbitration referred to the Chinese Arbitration Association,
#    Taipei in accordance with the ROC Arbitration Law and the Arbitration
#    Rules of the Association by three (3) arbitrators appointed in accordance
#    with the said Rules.
#    The place of arbitration shall be in Taipei, Taiwan and the language shall
#    be English.
#    The arbitration award shall be final and binding to both parties.
#
#******************************************************************************
#<MStar Software>
ifeq (1, $(strip $(shell expr $(PLATFORM_SDK_VERSION) \<28)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CharDistribution.cpp \
    JpCntx.cpp \
    LangModels/LangArabicModel.cpp \
    LangModels/LangBulgarianModel.cpp \
    LangModels/LangRussianModel.cpp \
    LangModels/LangEsperantoModel.cpp \
    LangModels/LangFrenchModel.cpp \
    LangModels/LangDanishModel.cpp  \
    LangModels/LangGermanModel.cpp \
    LangModels/LangGreekModel.cpp \
    LangModels/LangHungarianModel.cpp \
    LangModels/LangHebrewModel.cpp \
    LangModels/LangSpanishModel.cpp \
    LangModels/LangThaiModel.cpp \
    LangModels/LangTurkishModel.cpp \
    LangModels/LangVietnameseModel.cpp \
    LangModels/LangSwedishModel.cpp \
    LangModels/LangSloveneModel.cpp \
    LangModels/LangLithuanianModel.cpp \
    LangModels/LangRomanianModel.cpp \
    LangModels/LangIrishModel.cpp \
    LangModels/LangEstonianModel.cpp \
    LangModels/LangCroatianModel.cpp \
    LangModels/LangItalianModel.cpp \
    LangModels/LangFinnishModel.cpp \
    LangModels/LangSlovakModel.cpp \
    LangModels/LangCzechModel.cpp \
    LangModels/LangMalteseModel.cpp \
    LangModels/LangPortugueseModel.cpp \
    LangModels/LangPolishModel.cpp \
    LangModels/LangLatvianModel.cpp \
    nsHebrewProber.cpp \
    nsCharSetProber.cpp \
    nsBig5Prober.cpp \
    nsEUCJPProber.cpp \
    nsEUCKRProber.cpp \
    nsEUCTWProber.cpp \
    nsEscCharsetProber.cpp \
    nsEscSM.cpp \
    nsGB2312Prober.cpp \
    nsMBCSGroupProber.cpp \
    nsMBCSSM.cpp \
    nsSBCSGroupProber.cpp \
    nsSBCharSetProber.cpp \
    nsSJISProber.cpp \
    nsUTF8Prober.cpp \
    nsLatin1Prober.cpp \
    nsUniversalDetector.cpp \
    uchardet.cpp

LOCAL_C_INCLUDES := \
    CharDistribution.h \
    JpCntx.h \
    nsBig5Prober.h \
    nsCharSetProber.h \
    nsCodingStateMachine.h \
    nsEscCharsetProber.h \
    nsEUCJPProber.h \
    nsEUCKRProber.h \
    nsEUCTWProber.h \
    nsGB2312Prober.h \
    nsHebrewProber.h \
    nsLatin1Prober.h \
    nsMBCSGroupProber.h \
    nsPkgInt.h \
    nsSBCharSetProber.h \
    nsSBCSGroupProber.h \
    nsSJISProber.h \
    nsUniversalDetector.h \
    nsUTF8Prober.h \
    prmem.h \
    uchardet.h

LOCAL_CFLAGS += -Wno-unused-parameter -Wno-reorder -Wno-sign-compare \
                -Wno-pointer-arith -Wno-parentheses -Wno-strict-aliasing -Wno-enum-compare -O0 -g3

#ifeq ($(filter 19 20 21 22 23 24 25 26 27, $(PLATFORM_SDK_VERSION)),)
#   LOCAL_CFLAGS += \
#       -DENABLE_VENDOR_PROP
#endif

#LOCAL_CFLAGS += \

#LOCAL_STATIC_LIBRARIES += \
#LOCAL_SHARED_LIBRARIES += libz

CHANGELIST := $(shell cd $(LOCAL_PATH) && git rev-parse --short=8 HEAD 2>/dev/null)
.PHONY: make_ver

LOCAL_MODULE := libuchardet
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_MULTILIB := 32

CS_ROOT_PATH := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
endif
