/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


/*-----------------------------------------------------------------------------
                            include files
-----------------------------------------------------------------------------*/

#include "mtk_bt_service_leaudio_bmr.h"
#include "c_bt_mw_leaudio_bmr.h"
#include "ri_common.h"

static void *g_bmr_app_pvtag = NULL;
static mtkrpcapi_BtAppBmrCbk mtkrpcapi_BtBmrEventCbk = NULL;

static VOID MWBtBmrEventCbk(BT_BMR_EVENT_PARAM *param)
{
    if (mtkrpcapi_BtBmrEventCbk)
    {
        mtkrpcapi_BtBmrEventCbk(param, g_bmr_app_pvtag);
    }
}

INT32 x_mtkapi_bt_bmr_discover(BT_BMR_DISCOVERY_PARAMS *params)
{
    return c_btm_bt_bmr_discover(params);
}

INT32 x_mtkapi_bt_bmr_solicitation_request(BT_BMR_SOLICITATION_REQUEST_PARAMS *params)
{
    return c_btm_bt_bmr_solicitation_request(params);
}

INT32 x_mtkapi_bt_bmr_disconnect(CHAR *bdaddr)
{
    return c_btm_bt_bmr_disconnect(bdaddr);
}

INT32 x_mtkapi_bt_bmr_refresh_source(UINT8 source_id)
{
    return c_btm_bt_bmr_refresh_source(source_id);
}

INT32 x_mtkapi_bt_bmr_remove_source(BOOL all, UINT8 source_id)
{
    return c_btm_bt_bmr_remove_source(all, source_id);
}

INT32 x_mtkapi_bt_bmr_set_broadcast_code(BT_BMR_SET_BROADCAST_CODE_PARAMS *params)
{
    return c_btm_bt_bmr_set_broadcast_code(params);
}

INT32 x_mtkapi_bt_bmr_streaming_start(UINT8 source_id, UINT32 bis_mask)
{
    return c_btm_bt_bmr_streaming_start(source_id, bis_mask);
}

INT32 x_mtkapi_bt_bmr_streaming_stop(UINT8 source_id, UINT32 bis_mask)
{
    return c_btm_bt_bmr_streaming_stop(source_id, bis_mask);
}

INT32 x_mtkapi_bt_bmr_set_pac_config(UINT8 pac_type, UINT32 value)
{
    return c_btm_bt_bmr_set_pac_config(pac_type, value);
}

INT32 x_mtkapi_bt_bmr_close(BOOL open_to_bsa)
{
    return c_btm_bt_bmr_close(open_to_bsa);
}

INT32 x_mtkapi_bt_bmr_dump(BOOL all, UINT8 source_id, BOOL full_info)
{
    return c_btm_bt_bmr_dump(all, source_id, full_info);
}

INT32 x_mtkapi_bt_bmr_register_callback(mtkrpcapi_BtAppBmrCbk func, void *pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != func)
    {
        g_bmr_app_pvtag = pv_tag;
        mtkrpcapi_BtBmrEventCbk = func;
        i4_ret = c_btm_bt_bmr_register_callback(MWBtBmrEventCbk);
    }
    else
    {
        if (NULL != g_bmr_app_pvtag)
        {
            ri_free_cb_tag(g_bmr_app_pvtag);
            g_bmr_app_pvtag = NULL;
        }
        i4_ret = c_btm_bt_bmr_register_callback(NULL);
    }
    return i4_ret;
}

