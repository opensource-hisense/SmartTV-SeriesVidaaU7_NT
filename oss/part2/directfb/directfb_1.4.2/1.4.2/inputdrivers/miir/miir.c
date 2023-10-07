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

#include <config.h>

#include <directfb.h>
#include <directfb_keynames.h>

#include "directfb.h"

#include <core/coretypes.h>

#include <core/input.h>

#include <direct/debug.h>
#include <direct/util.h>

#include <core/input_driver.h>

#include <misc/conf.h>


//------IR & Key Setting--------------------------------------------------------
#define IR_TYPE_MSTAR_DTV               2
#define IR_TYPE_SEL                     IR_TYPE_MSTAR_DTV   // IR_TYPE_MSTAR_DTV // IR_TYPE_CUS03_DTV // IR_TYPE_NEW

// IR Header code define
#define IR_HEADER_CODE0         0x10    // Custom 0
#define IR_HEADER_CODE1         0x00    // Custom 1
#define MI_SUPPORT_LP64 0
#define MI_ENABLE_DBG 1
#define TIME2GET1STRPTKEY 800
#include "mi_common.h"
#include "mi_ir.h"

DFB_INPUT_DRIVER( miir )

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define IR_HEADER_CODE (IR_HEADER_CODE0<<8 |IR_HEADER_CODE1)

#define IR_64BIT_HEADER_CODE (IR_HEADER_CODE0<<8 |IR_HEADER_CODE1) //MStar IR

static MI_HANDLE _hIrHandle=NULL;
static MI_BOOL _bGetRawData = TRUE;
static CoreInputDevice *pIrDevice=NULL;

static int
driver_get_available()
{
    /* If use mi system,mikeypad will be init successfully */
    if (dfb_config->mst_using_mi_system)
        return 1;

    return 0;
}

static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     snprintf( info->name,
               DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "MI IR Driver" );

     snprintf( info->vendor,
               DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semi" );

     info->version.major = 0;
     info->version.minor = 2;
}


typedef struct {
     CoreInputDevice  *device;
     int           repeat_time;
} MStarIrData;

static void IR_GetKeyCallback(MI_U32 u32KeyCode)
{
    DBG_INPUT_MSG("[DFB_IR]callback key: 0x%08x\n", (unsigned int)u32KeyCode);

    DFBInputEvent evt;
    evt.clazz = DFEC_INPUT;
    evt.flags      = DIEF_KEYCODE;
    evt.type       = DIET_KEYPRESS;
    evt.key_id    = DIKI_UNKNOWN;
    evt.key_symbol = DIKS_NULL;

    MI_U16 u16HeadCode=0;
    MI_U8 u8KeyCode=0;
    MI_U8 u8KeyState=0;

    if(_bGetRawData==TRUE)
    {
        u16HeadCode = u32KeyCode>>16;
        u8KeyCode = (u32KeyCode>>8)&0xFF;
        u8KeyState = u32KeyCode&0x0F;

        DBG_INPUT_MSG("[DFB_IR]u16HeadCode: 0x%04x\n", u16HeadCode);
        if(u16HeadCode != IR_HEADER_CODE)
        {
            DBG_INPUT_MSG("[DFB_IR]Error: Expected Header code: 0x%04x\n", IR_HEADER_CODE);
            return;
        }
        DBG_INPUT_MSG("[DFB_IR]u8KeyCode: 0x%02x\n", u8KeyCode);
        DBG_INPUT_MSG("[DFB_IR]u8KeyState: 0x%02x\n", u8KeyState);
        if(u8KeyState == MI_IR_KEY_STATE_PRESS)
        {
            DBG_INPUT_MSG("[DFB_IR]=====> Key Press\n");
        }
        else if(u8KeyState == MI_IR_KEY_STATE_REPEAT)
        {
            DBG_INPUT_MSG("[DFB_IR]=====> Key Repeat\n");
            evt.flags |= DIEF_REPEAT;
        }
        else if(u8KeyState == MI_IR_KEY_STATE_RELEASE)
        {
            DBG_INPUT_MSG("[DFB_IR]=====> Key Release\n");
            evt.type = DIET_KEYRELEASE;
        }
        else
        {
            DBG_INPUT_MSG("[DFB_IR]=====> unknown\n");
        }
    }

    evt.key_code = u8KeyCode;
    dfb_input_dispatch( pIrDevice, &evt);

}

static DFBResult
driver_open_device( CoreInputDevice *device,
                    unsigned int    number,
                    InputDeviceInfo *info,
                    void            **driver_data )
{

   if (dfb_config == NULL)
          return DFB_FAILURE;

     MStarIrData *data = NULL;

     data = D_CALLOC(1, sizeof(MStarIrData));
     if (data == NULL)
         return DFB_FAILURE;

     data->device = device;
     pIrDevice = device;
     /* set private data pointer */
     *driver_data = data;

     data->repeat_time = dfb_config->mst_ir_repeat_time;

     if (info == NULL){
          D_FREE(data);
          return DFB_FAILURE;
     }

    MI_IR_InitParam_t stInitParam;
    MI_IR_OpenParam_t stOpenParam;

    stOpenParam.stRepeatKey.u32Time2Get1stRptKey= TIME2GET1STRPTKEY;
    stOpenParam.stRepeatKey.u32RepeatKeyDuration= data->repeat_time;
    stOpenParam.u16CustomCode = IR_HEADER_CODE;
    stOpenParam.u32ExtCustomCode = IR_64BIT_HEADER_CODE;
    stOpenParam.bReturnRawData = _bGetRawData;

    if(MI_IR_Init(&stInitParam)!=MI_OK)
        return FALSE;

    if(MI_IR_Open(&stOpenParam, &_hIrHandle)!=MI_OK)
        return FALSE;

    if(MI_IR_RegisterCallback(_hIrHandle, IR_GetKeyCallback)!=MI_OK)
        return FALSE;

    //if(MI_IR_RegisterExtCallback(_hIrHandle, _MI_DEMO_IR_GetKeyExtCallback)!=MI_OK)
        //return FALSE;

     /* fill driver info structure */
     snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "MI IR Device" );

     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "MStar Semi" );

     info->prefered_id = DIDID_MSTARIR;
     info->desc.type   = DIDTF_REMOTE;
     info->desc.caps   = DICAPS_KEYS;
     info->desc.min_keycode = 0;
     info->desc.max_keycode = dfb_config->mst_ir_max_keycode;  //0x7FFF;

     return DFB_OK;
 }

/*
 * Fetch one entry from the device's keymap if supported.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
    return DFB_UNSUPPORTED;
}


static void
driver_close_device( void *driver_data )
{
    MStarIrData *data = (MStarIrData*) driver_data;

    if(MI_IR_Close(_hIrHandle)!=MI_OK)
    {
        DBG_INPUT_MSG("Close MI IR fail\n");
    }
    if(MI_IR_DeInit()!=MI_OK)
    {
        DBG_INPUT_MSG("Deinit MI IR module fail\n");
    }

     /* free private data */
    D_FREE( data );
}

static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                      void                         *driver_data,
                     InputDeviceIoctlData *param)
{
     MStarIrData *data = (MStarIrData*) driver_data;
     MI_IR_RepeatKey_t stRepeatKey;

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     switch(param->request)
     {
       case DFB_DEV_IOC_SET_MSTARIR_REPEAT_TIME:
          {
               if(((int)param->param[0])<=0)
                  return DFB_INVARG;

               data->repeat_time = ((int)param->param[0]);
               stRepeatKey.u32Time2Get1stRptKey = TIME2GET1STRPTKEY;
               stRepeatKey.u32RepeatKeyDuration = data->repeat_time;
               MI_IR_SetRepeatKeyTime(_hIrHandle, &stRepeatKey);
               printf("[DFB] Mstarir Set repeat time:%d (ms)\n", data->repeat_time);
               break;
           }

       case DFB_DEV_IOC_GET_MSTARIR_REPEAT_TIME:
           {
                memset(param->param, 0, sizeof(param->param));
                param->param[0] = data->repeat_time;
                printf("[DFB] Mstarir Get repeat time:%d (ms)\n", data->repeat_time);
                break;
           }
       default:
          printf("[DFB] WARNING!!! Invalid argument input.\n", data->repeat_time);
          return DFB_UNSUPPORTED;
     }

     return  DFB_OK;
}
