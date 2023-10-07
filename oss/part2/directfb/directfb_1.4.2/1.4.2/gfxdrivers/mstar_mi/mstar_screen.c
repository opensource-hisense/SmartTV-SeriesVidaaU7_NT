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

#define DIRECT_ENABLE_DEBUG

#include <stdio.h>
#include <directfb.h>
#include <misc/conf.h>
#include <mstar_screen.h>

#include "mi_common.h"
#include "mi_disp.h"

D_DEBUG_DOMAIN(MSTAR_Screen, "MSTAR/Screen", "MSTAR Screen (FBDev)");

/**********************************************************************************************************************/

static DFBResult
mstarInitScreen (CoreScreen *screen, CoreGraphicsDevice *device, void *driver_data, void *screen_data, DFBScreenDescription *description)
{
    D_DEBUG_AT(MSTAR_Screen, "[DFB] %s()\n", __FUNCTION__);

    if(description == NULL)
        return DFB_FAILURE;

    /* Set the screen capabilities. */
    description->caps = DSCCAPS_NONE;

    /* Set the screen name. */
    snprintf(description->name, DFB_SCREEN_DESC_NAME_LENGTH, "MSTAR Screen (FBDev)");

    return DFB_OK;
}

static DFBResult
mstarGetOPScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height) {

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);

     MI_RESULT u32Ret = MI_OK;
     MI_HANDLE hHandle = MI_HANDLE_NULL;
     MI_DISP_QueryHandleParams_t stParams;
     MI_DISP_VidWinRect_t stDispRect;
     char szName[] = "MI_DISP_HD0";

     stParams.pszName = (MI_U8*)szName;

     /* MI_DISP_Init just only call once in process.
        deinit is no needed, because deinit would close disp on screen. */
     u32Ret = MI_DISP_Init(NULL);
     if (u32Ret != MI_OK && u32Ret != MI_HAS_INITED) {
         printf("[DFB] %s, MI_DISP_Init failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     u32Ret = MI_DISP_GetHandle(&stParams, &hHandle);
     if (u32Ret != MI_OK) {
         //printf("[DFB] %s, MI_DISP_GetHandle(pszName = %s) failed, Error : 0x%x:\n", __FUNCTION__, stParams.pszName, u32Ret);
         MI_DISP_OpenParams_t stDispOpenParams;
         memset(&stDispOpenParams, 0, sizeof(stDispOpenParams));
         stDispOpenParams.pszName = (MI_U8*)szName;
         stDispOpenParams.eVidWinType = E_MI_DISP_VIDWIN_HD;
         if(MI_OK != MI_DISP_Open(&stDispOpenParams, &hHandle))
         {
            return DFB_FAILURE;
         }
     }
     memset(&stDispRect, 0x00, sizeof(MI_DISP_VidWinRect_t));
     u32Ret = MI_DISP_GetDispRect(hHandle, &stDispRect);
     if (u32Ret != MI_OK) {
         printf("[DFB] %s, MI_DISP_GetDsipRect failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     *ret_width = stDispRect.u16Width;
     *ret_height = stDispRect.u16Height;

     DBG_LAYER_MSG("[DFB] %s, (%d, %d)\n", __FUNCTION__, *ret_width, *ret_height);

     return DFB_OK;
}

static DFBResult
mstarGetIP0ScreenSize(CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height) {

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);
     MI_RESULT u32Ret = MI_OK;
     MI_HANDLE hHandle = MI_HANDLE_NULL;
     MI_DISP_QueryHandleParams_t stParams;
     MI_DISP_VidWinRect_t stDispRect;
     char szName[] = "MI_DISP_HD0";

     stParams.pszName = (MI_U8*)szName;

     u32Ret = MI_DISP_Init(NULL);
     if (u32Ret != MI_OK && u32Ret != MI_HAS_INITED) {
         printf("[DFB] %s, MI_DISP_Init failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     u32Ret = MI_DISP_GetHandle(&stParams, &hHandle);
     if (u32Ret != MI_OK) {
         //printf("[DFB] %s, MI_DISP_GetHandle(pszName = %s) failed, Error : 0x%x:\n", __FUNCTION__, stParams.pszName, u32Ret);
         MI_DISP_OpenParams_t stDispOpenParams;
         memset(&stDispOpenParams, 0, sizeof(stDispOpenParams));
         stDispOpenParams.pszName = (MI_U8*)szName;
         stDispOpenParams.eVidWinType = E_MI_DISP_VIDWIN_HD;
         if(MI_OK != MI_DISP_Open(&stDispOpenParams, &hHandle))
         {
            return DFB_FAILURE;
         }
     }
     memset(&stDispRect, 0x00, sizeof(MI_DISP_VidWinRect_t));
     u32Ret = MI_DISP_GetDispRect(hHandle, &stDispRect) ;
     if (u32Ret != MI_OK) {
         printf("[DFB] %s, MI_DISP_GetDsipRect failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     *ret_width = stDispRect.u16Width;
     *ret_height = stDispRect.u16Height;

    if ((*ret_width < 64) ||
        (*ret_height < 64) ||
        (*ret_width >= 2048) ||
        (*ret_height >= 2048)) {
      *ret_width  = 640;
      *ret_height = 480;
    }

     DBG_LAYER_MSG("[DFB] %s, (%d, %d)\n", __FUNCTION__, *ret_width, *ret_height);

    return DFB_OK;
}


static DFBResult
mstarGetVEScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height ) {

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);
     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);

     MI_RESULT u32Ret = MI_OK;
     MI_DISP_GetControllerParams_t stGetControllerParams;
     MI_HANDLE hDispController = MI_HANDLE_NULL;
     MI_DISP_TvSystem_e eTvSys;

     u32Ret = MI_DISP_Init(NULL);
     if (u32Ret != MI_OK && u32Ret != MI_HAS_INITED) {
         printf("[DFB] %s, MI_DISP_Init failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     memset(&stGetControllerParams, 0x00, sizeof(MI_DISP_GetControllerParams_t));
     u32Ret = MI_DISP_GetController(&stGetControllerParams, &hDispController);
     if (u32Ret != MI_OK) {
         printf("[DFB], %s, MI_DISP_GetController failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     u32Ret = MI_DISP_GetTvSystem(hDispController,&eTvSys);
     if (u32Ret != MI_OK) {
         printf("[DFB] %s, MI_DISP_GetTvSystem failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
         return DFB_FAILURE;
     }

     if(eTvSys <= E_MI_DISP_TV_SYSTEM_NTSC_J)   //480
     {
         *ret_width = 720;
         *ret_height = 480;
     }
     else if((eTvSys <= E_MI_DISP_TV_SYSTEM_PAL) && (eTvSys >= E_MI_DISP_TV_SYSTEM_PAL_M))    //576
     {
         *ret_width = 720;
         *ret_height = 576;
     }

     DBG_LAYER_MSG("[DFB] %s, (%d, %d)\n", __FUNCTION__, *ret_width, *ret_height);

     return DFB_OK;
}


static DFBResult
mstarGetOCScreenSize(CoreScreen *screen,
                     void       *driver_data,
                     void       *screen_data,
                     int        *ret_width,
                     int        *ret_height ) {

     D_DEBUG_AT(MSTAR_Screen, "%s()\n", __FUNCTION__);

     D_ASSERT(ret_width != NULL);
     D_ASSERT(ret_height != NULL);

     printf("[DFB] %s, UNIMPLEMENTED!\n", __FUNCTION__);

     return DFB_UNIMPLEMENTED;
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
#undef DIRECT_ENABLE_DEBUG
