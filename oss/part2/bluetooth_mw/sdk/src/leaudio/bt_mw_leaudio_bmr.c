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


#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bt_mw_leaudio_bmr.h"
#include "bt_mw_common.h"
#include "linuxbt_leaudio_bmr_if.h"
#include "u_bt_mw_leaudio_bmr.h"
#include "bt_mw_gap.h"
#include "bt_mw_message_queue.h"

static BT_BMR_EVENT_HANDLE_CB g_bt_mw_bmr_event_handle_cb = NULL;
static INT32 bluetooth_bmr_init_profile(VOID);
static INT32 bluetooth_bmr_deinit_profile(VOID);
static INT32 bluetooth_bmr_init(VOID);
static INT32 bluetooth_bmr_deinit(VOID);
static VOID bluetooth_bmr_msg_handle(tBTMW_MSG *p_msg);
static VOID bluetooth_bmr_notify_handle(tBTMW_MSG *p_msg);

static INT32 bluetooth_bmr_init_profile(VOID)
{
    FUNC_ENTRY;
    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BMR, bluetooth_bmr_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BMR, bluetooth_bmr_notify_handle);

    return linuxbt_bmr_init();
}

static INT32 bluetooth_bmr_deinit_profile(VOID)
{
    FUNC_ENTRY;
    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BMR, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BMR, NULL);

    return linuxbt_bmr_deinit();
}

static INT32 bluetooth_bmr_init(VOID)
{
    FUNC_ENTRY;
    profile_operator_t bmr_op;
    memset(&bmr_op, 0, sizeof(bmr_op));
    bmr_op.init = (void (*)(void))bluetooth_bmr_init_profile;
    bmr_op.deinit = (void (*)(void))bluetooth_bmr_deinit_profile;
    bmr_op.notify_acl_state = NULL;
    bmr_op.facotry_reset = NULL;

    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BMR, &bmr_op);
    return BT_SUCCESS;
}

static INT32 bluetooth_bmr_deinit(VOID)
{
    FUNC_ENTRY;
    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BMR, NULL);

    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BMR, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BMR, NULL);
    return BT_SUCCESS;
}

static VOID bluetooth_bmr_msg_handle(tBTMW_MSG *p_msg)
{
    switch(p_msg->hdr.event)
    {
        case BTMW_BMR_CONNECTION_STATE_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_CONNECTION_STATE_CB_EVT");
            break;
        }
        case BTMW_BMR_SOURCE_DISCOVERY_STATE_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_SOURCE_DISCOVERY_STATE_CB_EVT");
            break;
        }
        case BTMW_BMR_SOLICITATION_REQUEST_STATE_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_SOLICITATION_REQUEST_STATE_CB_EVT");
            break;
        }
        case BTMW_BMR_SOURCE_INFO_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_SOURCE_INFO_CB_EVT");
            break;
        }
        case BTMW_BMR_SOURCE_STATE_CHANGE_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_SOURCE_STATE_CHANGE_CB_EVT");
            break;
        }
        case BTMW_BMR_STREAMING_EVENT_CB_EVT: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "BTMW_BMR_STREAMING_EVENT_CB_EVT");
            break;
        }
        default: {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "unknown event, return");
            return;
        }
    }
    bt_mw_nty_send_msg(p_msg);
}

static VOID  bluetooth_bmr_notify_handle(tBTMW_MSG *p_msg)
{
    BT_BMR_EVENT_PARAM *bmr_event = &p_msg->data.bmr_event;

    switch(p_msg->hdr.event)
    {
        case BTMW_BMR_CONNECTION_STATE_CB_EVT:
        case BTMW_BMR_SOURCE_DISCOVERY_STATE_CB_EVT:
        case BTMW_BMR_SOLICITATION_REQUEST_STATE_CB_EVT:
        case BTMW_BMR_SOURCE_INFO_CB_EVT:
        case BTMW_BMR_SOURCE_STATE_CHANGE_CB_EVT:
        case BTMW_BMR_STREAMING_EVENT_CB_EVT:
            if (g_bt_mw_bmr_event_handle_cb)
            {
                BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "notify to app event:%d", bmr_event->event);
                g_bt_mw_bmr_event_handle_cb(bmr_event);
            }
            break;

        default:
            break;
    }
}

INT32 bluetooth_bmr_discover(BT_BMR_DISCOVERY_PARAMS *params)
{
    return linuxbt_bmr_discover(params);
}

INT32 bluetooth_bmr_solicitation_request(BT_BMR_SOLICITATION_REQUEST_PARAMS *params)
{
    return linuxbt_bmr_solicitation_request(params);
}

INT32 bluetooth_bmr_disconnect(CHAR *bdaddr)
{
    return linuxbt_bmr_disconnect(bdaddr);
}

INT32 bluetooth_bmr_refresh_source(UINT8 source_id)
{
    return linuxbt_bmr_refresh_source(source_id);
}

INT32 bluetooth_bmr_remove_source(BOOL all, UINT8 source_id)
{
    return linuxbt_bmr_remove_source(all, source_id);
}

INT32 bluetooth_bmr_set_broadcast_code(BT_BMR_SET_BROADCAST_CODE_PARAMS *params)
{
    return linuxbt_bmr_set_broadcast_code(params);
}

INT32 bluetooth_bmr_streaming_start(UINT8 source_id, UINT32 bis_mask)
{
    return linuxbt_bmr_streaming_start(source_id, bis_mask);
}

INT32 bluetooth_bmr_streaming_stop(UINT8 source_id, UINT32 bis_mask)
{
    return linuxbt_bmr_streaming_stop(source_id, bis_mask);
}

INT32 bluetooth_bmr_set_pac_config(UINT8 pac_type, UINT32 value)
{
    return linuxbt_bmr_set_pac_config(pac_type, value);
}

INT32 bluetooth_bmr_close(BOOL open_to_bsa)
{
    return linuxbt_bmr_close(open_to_bsa);
}

INT32 bluetooth_bmr_dump(BOOL all, UINT8 source_id, BOOL full_info)
{
    return linuxbt_bmr_dump(all, source_id, full_info);
}

INT32 bluetooth_bmr_register_callback(BT_BMR_EVENT_HANDLE_CB bmr_handle)
{
    int ret = BT_SUCCESS;
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMR, "bmr_handle: %p", bmr_handle);
    if (NULL != bmr_handle)
    {
        ret = bluetooth_bmr_init();
    }
    else
    {
        ret = bluetooth_bmr_deinit();
    }
    g_bt_mw_bmr_event_handle_cb = bmr_handle;
    return ret;
}
