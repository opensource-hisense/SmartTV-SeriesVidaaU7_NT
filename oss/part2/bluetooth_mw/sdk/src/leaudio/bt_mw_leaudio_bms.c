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

#include "bt_mw_leaudio_bms.h"
#include "bt_mw_common.h"
#include "linuxbt_leaudio_bms_if.h"
#include "bt_mw_message_queue.h"
#include "bt_mw_gap.h"


BtAppBmsCbk BmsCbk = NULL;
BOOL B_Bms_Init = FALSE;

static INT32 bt_mw_bms_init(void);
static INT32 bt_mw_bms_deinit(void);
static VOID bt_mw_bms_init_profile(void);
static VOID bt_mw_bms_deinit_profile(void);
static VOID bt_mw_bms_nty_handle(tBTMW_MSG *p_msg);
VOID bt_mw_bms_nty_state_handle(tBTMW_MSG *p_msg);

static INT32 bt_mw_bms_init(void)
{
    if (TRUE == B_Bms_Init)
    {
        return BT_SUCCESS;
    }

    profile_operator_t bms_op;
    memset(&bms_op, 0, sizeof(bms_op));
    bms_op.init = bt_mw_bms_init_profile;
    bms_op.deinit = bt_mw_bms_deinit_profile;
    bms_op.notify_acl_state = NULL;
    bms_op.facotry_reset = NULL;

    B_Bms_Init = TRUE;

    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BMS, &bms_op);

    return BT_SUCCESS;
}

static INT32 bt_mw_bms_deinit(void)
{
    B_Bms_Init = FALSE;

    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BMS, NULL);
    bt_mw_bms_deinit_profile();

    return BT_SUCCESS;
}

static VOID bt_mw_bms_init_profile(void)
{
    FUNC_ENTRY;
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BMS, bt_mw_bms_nty_handle);
    linuxbt_bms_init();
}

static VOID bt_mw_bms_deinit_profile(void)
{
    FUNC_ENTRY;
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BMS, NULL);
    linuxbt_bms_deinit();
}

static VOID bt_mw_bms_nty_handle(tBTMW_MSG *p_msg)
{
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "bluetooth bms notify msg handle, event:%d", p_msg->hdr.event);

    if (p_msg->hdr.event == BTMW_BMS_STATE_EVT)
    {
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "bluetooth bms msg handle, state event");
        bt_mw_bms_nty_state_handle(p_msg);
    }
    else
    {
       BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "bluetooth bms msg handle, no define event");
    }
}

VOID bt_mw_bms_nty_state_handle(tBTMW_MSG *p_msg)
{
    FUNC_ENTRY;

    BT_BMS_EVENT_PARAM t_bms_struct_data;
    memcpy(&t_bms_struct_data, &p_msg->data.bms_event, sizeof(BT_BMS_EVENT_PARAM));
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "event:%d, bmsevent:%d", p_msg->hdr.event, p_msg->data.bms_event.event);

    /*  call the app callback function*/
    if(BmsCbk)
    {
        BmsCbk(&t_bms_struct_data);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "bms callback is NULL.");
    }
}

INT32 bt_mw_bms_register_callback(BtAppBmsCbk bms_cb)
{
    FUNC_ENTRY;
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "bms_cb=%p", bms_cb);
    if (bms_cb)
    {
        BmsCbk = bms_cb;
        bt_mw_bms_init();
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "bms callback func is null!");
        bt_mw_bms_deinit();
        return BT_ERR_STATUS_NULL_POINTER;
    }

    return BT_SUCCESS;
}

INT32 bt_mw_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_create_broadcast(param);
}

INT32 bt_mw_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_create_broadcast_ext(param);
}

INT32 bt_mw_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_update_base_announcement(param);
}

INT32 bt_mw_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_update_subgroup_metadata(param);
}

INT32 bt_mw_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_start_broadcast(param);
}

INT32 bt_mw_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_start_broadcast_multi_thread(param);
}

INT32 bt_mw_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_pause_broadcast(param);
}

INT32 bt_mw_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_stop_broadcast(param);
}

INT32 bt_mw_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_bms_get_own_address(param);
}

INT32 bt_mw_bms_get_all_broadcasts_states(VOID)
{
    FUNC_ENTRY;
    return linuxbt_bms_get_all_broadcasts_states();
}


INT32 bt_mw_bms_stop_all_broadcast(VOID)
{
    FUNC_ENTRY;
    return linuxbt_bms_stop_all_broadcast();
}

