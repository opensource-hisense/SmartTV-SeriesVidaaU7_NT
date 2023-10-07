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

#include "mtk_bt_service_leaudio_bms.h"
#include "c_bt_mw_leaudio_bms.h"
#include "ri_common.h"

#define BT_RC_LOG(_stmt...) \
    do{ \
        if(1){    \
            printf("[BMS IPC Server]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)

static void *g_bms_app_pvtag = NULL;

static mtkrpcapi_BtAppBmsCbk mtkrpcapi_BtBmsEventCbk = NULL;

static VOID MWBtBmsEventCbk(BT_BMS_EVENT_PARAM *param)
{
    if (mtkrpcapi_BtBmsEventCbk)
    {
        mtkrpcapi_BtBmsEventCbk(param, g_bms_app_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtBmsEventCbk is null\n");
    }

}

INT32 x_mtkapi_bt_bms_register_callback(mtkrpcapi_BtAppBmsCbk bms_cb, VOID* pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != bms_cb)
    {
        g_bms_app_pvtag = pv_tag;
        mtkrpcapi_BtBmsEventCbk = bms_cb;

        i4_ret = c_btm_bt_bms_register_callback(MWBtBmsEventCbk);
    }
    else
    {
        if (NULL != g_bms_app_pvtag)
        {
            ri_free_cb_tag(g_bms_app_pvtag);
            g_bms_app_pvtag = NULL;
        }
        i4_ret = c_btm_bt_bms_register_callback(NULL);
    }

    if (i4_ret != 0)
    {
        BT_RC_LOG("x_mtkapi_bms_register_callback fail\n");
    }

    return i4_ret;
}

INT32 x_mtkapi_bt_bms_unregister_callback(VOID)
{
    INT32 i4_ret = 0;
    if (NULL != g_bms_app_pvtag)
    {
        ri_free_cb_tag(g_bms_app_pvtag);
        g_bms_app_pvtag = NULL;
    }
    i4_ret = c_btm_bt_bms_register_callback(NULL);
    if (i4_ret != 0)
    {
        BT_RC_LOG("x_mtkapi_bms_unregister_callback fail\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_create_broadcast(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_create_broadcast fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_create_broadcast success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_create_broadcast_ext(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_create_broadcast_ext fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_create_broadcast_ext success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_update_base_announcement(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_update_base_announcement fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_update_base_announcement success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_update_subgroup_metadata(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_update_subgroup_metadata fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_update_subgroup_metadata success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_start_broadcast(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_start_broadcast fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_start_broadcast success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_start_broadcast_multi_thread(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_start_broadcast_multi_thread fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_start_broadcast_multi_thread success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_pause_broadcast(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_pause_broadcast fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_pause_broadcast success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_stop_broadcast(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_stop_broadcast fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_stop_broadcast success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_get_own_address(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_get_own_address fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_get_own_address success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_get_all_broadcasts_states(VOID)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_get_all_broadcasts_states();
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_get_all_broadcasts_states fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_get_all_broadcasts_states success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bms_stop_all_broadcast(VOID)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bt_bms_stop_all_broadcast();
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_bms_stop_all_broadcast fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_bms_stop_all_broadcast success\n");
    }
    return i4_ret;
}

