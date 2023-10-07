// <MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
// <MStar Software>

// #ifdef MSTAR_DEBUG_SCREEN
#define DIRECT_ENABLE_DEBUG
// #endif


#include <config.h>

#include <stdio.h>

#include <sys/mman.h>

#include <asm/types.h>

#include <directfb.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>


#include <MsCommon.h>
#include <drvSEM.h>
#include  <apiGFX.h>
#include  <apiGOP.h>
#include  <drvMMIO.h>

#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <drvMVOP.h>
#include <apiXC.h>
#include <apiPNL.h>

#include "mstar.h"
#include "mstar_types.h"
#include "mstar_screen.h"

#pragma weak  MApi_VE_GetDstInfo


D_DEBUG_DOMAIN(MSTAR_Screen, "MSTAR/Screen", "MSTAR Screen");

/**********************************************************************************************************************/

static DFBResult
mstarInitScreen(CoreScreen              *screen,
                CoreGraphicsDevice      *device,
                void                    *driver_data,
                void                    *screen_data,
                DFBScreenDescription    *description) {
     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf(description->name, DFB_SCREEN_DESC_NAME_LENGTH, "MSTAR Screen");

     return DFB_OK;
}

static DFBResult
mstarGetOPScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height) {
     // MSTARDriverData *sdrv = driver_data;
     MS_PNL_DST_DispInfo dstDispInfo;
     unsigned char ret_val = 0xff;

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);

     // printf("\nget the op screen\n");

    ret_val = MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
    *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

    D_INFO("[DFB] %s : MApi_PNL_GetDstInfo width = %d, height = %d\n", __FUNCTION__, *ret_width, *ret_height );
    if (dfb_config->mst_gop_interlace_adjust && dstDispInfo.bInterlaceMode)
    {
        (*ret_height) <<= 1;  //for mantis:1226868
        D_INFO("[DFB] %s : After adjust MApi_PNL_GetDstInfo width = %d, height = %d\n", __FUNCTION__, *ret_width, *ret_height );
    }

    D_INFO("[DFB] %s : MApi_PNL_GetDstInfo ReturnValue = %d\n", __FUNCTION__, ret_val);

     return DFB_OK;
}

static DFBResult
mstarGetIP0ScreenSize(CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height) {
     // MSTARDriverData *sdrv = driver_data;
     MS_XC_DST_DispInfo dstDispInfo;
     unsigned char ret_val = 0xff;

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);
     //printf("\nget ip0 screen size\n");

    if (dfb_config->mst_do_xc_ip1_patch == true) //XC IP1 is no bet support in STB, Enable patch to fix it.
    {
         if (MApi_XC_GetDynamicScalingStatus()) // DS Enable, use original case
         {
             ret_val = MApi_XC_GetDstInfo(&dstDispInfo, sizeof(MS_XC_DST_DispInfo), E_GOP_XCDST_IP1_MAIN);
             *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
             *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
             D_INFO("[DFB] %s : MApi_XC_GetDstInfo width = %d, height = %d\n", __FUNCTION__, *ret_width, *ret_height );
         }
    else // DS disable, use MApi_XC_GetStatusEx to get info
         {
             XC_ApiStatusEx stXCStatusEx;
             memset(&stXCStatusEx, 0, sizeof(XC_ApiStatusEx));
             stXCStatusEx.u16ApiStatusEX_Length = sizeof(XC_ApiStatusEx);
             stXCStatusEx.u32ApiStatusEx_Version = API_STATUS_EX_VERSION;
             if(MApi_XC_GetStatusEx(&stXCStatusEx, MAIN_WINDOW) == sizeof(XC_ApiStatusEx))
             {
                 *ret_width = stXCStatusEx.stCapWin.width;
                 *ret_height = stXCStatusEx.stCapWin.height;
                 D_INFO("[DFB] %s : MApi_XC_GetStatusEx width = %d, height = %d\n", __FUNCTION__, *ret_width, *ret_height );
             }
         }
    }
    else
    {
         ret_val = MApi_XC_GetDstInfo(&dstDispInfo, sizeof(MS_XC_DST_DispInfo), E_GOP_XCDST_IP1_MAIN);
         *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
         *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
         D_INFO("[DFB] %s : MApi_XC_GetStatusEx width = %d, height = %d\n", __FUNCTION__, *ret_width, *ret_height );
    }

    if ((*ret_width < 64) ||
        (*ret_height < 64) ||
        (*ret_width >= 2048) ||
        (*ret_height >= 2048)) {
      *ret_width  = 640;
      *ret_height = 480;
    }

    D_INFO("[DFB] %s : MApi_XC_GetDstInfo ReturnValue = %d\n", __FUNCTION__, ret_val);

    return DFB_OK;
}


static DFBResult
mstarGetVEScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height ) {
     // MSTARDriverData *sdrv = driver_data;
     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);
     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);
     MS_VE_DST_DispInfo dstDispInfo;
     // printf("\nget ve screen size \n");

     unsigned char ret_val = 0xff;

       if (MApi_VE_GetDstInfo != NULL)
            ret_val = MApi_VE_GetDstInfo(&dstDispInfo,sizeof(MS_VE_DST_DispInfo));

    *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

    D_INFO("[DFB] %s : MApi_VE_GetDstInfo ReturnValue = %d\n", __FUNCTION__, ret_val);

     return DFB_OK;
}


static DFBResult
mstarGetOCScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height ) {
     // MSTARDriverData *sdrv = driver_data;
     MS_OSDC_DST_DispInfo dstDispInfo;
     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);
     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);

     E_APIXC_ReturnValue ret_val = E_APIXC_RET_FAIL;

     memset(&dstDispInfo, 0, sizeof(MS_OSDC_DST_DispInfo));
     dstDispInfo.ODSC_DISPInfo_Version = ODSC_DISPINFO_VERSIN;
     ret_val = MApi_XC_OSDC_GetDstInfo(&dstDispInfo, sizeof(MS_OSDC_DST_DispInfo));

     *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
     *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

     D_INFO("[DFB] %s : MApi_XC_OSDC_GetDstInfo ReturnValue = %d\n", __FUNCTION__, ret_val);

     return DFB_OK;
}


ScreenFuncs mstarOPScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetOPScreenSize
};

ScreenFuncs mstarIP0ScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetIP0ScreenSize
};

ScreenFuncs mstarVEScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetVEScreenSize
};

ScreenFuncs mstarOCScreenFuncs ={
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetOCScreenSize
};


