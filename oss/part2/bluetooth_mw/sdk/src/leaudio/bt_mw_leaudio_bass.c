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
#include "bt_mw_leaudio_bass.h"
#include "bt_mw_common.h"
#include "linuxbt_leaudio_bass_if.h"
#include "u_bt_mw_leaudio_bass.h"
#include "bt_mw_gap.h"
#include "bt_mw_message_queue.h"

static BT_BASS_EVENT_HANDLE_CB g_bt_mw_bass_event_handle_cb = NULL;
static BT_BASS_CONNECTED_DEVICE g_bass_connected_devices[MAX_BASS_DEV_NUM] = {0};

static VOID bluetooth_bass_init_profile(VOID);
static VOID bluetooth_bass_deinit_profile(VOID);
static INT32 bluetooth_bass_init(VOID);
static INT32 bluetooth_bass_deinit(VOID);
static VOID bluetooth_bass_msg_handle(tBTMW_MSG *p_msg);
static VOID bluetooth_bass_notify_handle(tBTMW_MSG *p_msg);
static VOID bluetooth_bass_connection_state_cb(BT_BASS_EVENT_CONN_STATE_DATA *p_conn_state);
static VOID bluetooth_bass_device_avail_cb(BT_BASS_EVENT_DEV_AVAIL_DATA *p_dev_avail);
static VOID bluetooth_bass_broadcast_scanning_changed_cb(BT_BASS_EVENT_SCANNING_STATE_DATA *p_scanning_state);
static VOID bluetooth_bass_broadcast_announce_recv_cb(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA *p_announce_received);
static VOID bluetooth_bass_broadcast_recv_state_cb(BT_BASS_EVENT_RECV_STATE_DATA *p_recv_state);
static VOID bluetooth_bass_builtin_mode_changed_cb(BT_BASS_EVENT_BUILTIN_MODE_DATA *p_builtin_mode);
static VOID bluetooth_bass_sync_lost_cb(BT_BASS_EVENT_SYNC_LOST_DATA *p_sync_lost);

static BOOL bluetooth_bass_find_device(CHAR *bt_addr, UINT8 *index)
{
    for (UINT8 i = 0; i < MAX_BASS_DEV_NUM; i++)
    {
        if ((TRUE == g_bass_connected_devices[i].in_used) &&
            (0 == strncasecmp(bt_addr, g_bass_connected_devices[i].bt_addr, MAX_BDADDR_LEN)))
        {
            *index = i;
            return TRUE;
        }
    }
    for (UINT8 i = 0; i < MAX_BASS_DEV_NUM; i++)
    {
        if (TRUE == g_bass_connected_devices[i].in_used)
        {
            continue;
        }
        else
        {
            *index = i;
            return FALSE;
        }
    }
    BT_DBG_WARNING(BT_DEBUG_LEAUDIO_BASS, "bluetooth_bass_find_device db is full\n");
    return FALSE;
}

static VOID bluetooth_bass_init_profile(VOID)
{
    FUNC_ENTRY;

    memset(g_bass_connected_devices, 0, sizeof(g_bass_connected_devices));
    for (int i = 0; i < MAX_BASS_DEV_NUM; i++)
    {
        strncpy(g_bass_connected_devices[i].bt_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN);
        g_bass_connected_devices[i].bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    }
    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BASS, bluetooth_bass_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BASS, bluetooth_bass_notify_handle);
    linuxbt_bass_init();
}

static VOID bluetooth_bass_deinit_profile(VOID)
{
    FUNC_ENTRY;
    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BASS, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BASS, NULL);
    linuxbt_bass_deinit();
}

static INT32 bluetooth_bass_init(VOID)
{
    FUNC_ENTRY;
    profile_operator_t bass_op;
    memset(&bass_op, 0, sizeof(bass_op));
    bass_op.init = bluetooth_bass_init_profile;
    bass_op.deinit = bluetooth_bass_deinit_profile;
    bass_op.notify_acl_state = NULL;
    bass_op.facotry_reset = NULL;

    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BASS, &bass_op);
    return BT_SUCCESS;
}

static INT32 bluetooth_bass_deinit(VOID)
{
    FUNC_ENTRY;
    bt_mw_gap_register_profile(BTWM_ID_LEAUDIO_BASS, NULL);

    linuxbt_hdl_register(BTWM_ID_LEAUDIO_BASS, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_LEAUDIO_BASS, NULL);
    return BT_SUCCESS;
}

static VOID bluetooth_bass_msg_handle(tBTMW_MSG *p_msg)
{
    tBT_MW_BASS_MSG *bass_msg = &p_msg->data.bass_msg;

    switch(p_msg->hdr.event)
    {
    case BTMW_BASS_CONNECTION_CB_EVT:
        bluetooth_bass_connection_state_cb(&bass_msg->data.conn_state);
        break;

    case BTMW_BASS_DEVICE_AVAIL_CB_EVT:
        bluetooth_bass_device_avail_cb(&bass_msg->data.dev_avail);
        break;

    case BTMW_BASS_BROADCAST_SCANNING_CHANGED_CB_EVT:
        bluetooth_bass_broadcast_scanning_changed_cb(&bass_msg->data.scanning_state);
        break;

    case BTMW_BASS_BROADCAST_ANNOUNCE_RECV_CB_EVT:
        bluetooth_bass_broadcast_announce_recv_cb(&bass_msg->data.announce_received);
        break;

    case BTMW_BASS_BROADCAST_RECV_STATE_CB_EVT:
        bluetooth_bass_broadcast_recv_state_cb(&bass_msg->data.recv_state);
        break;

    case BTMW_BASS_BUILTIN_MODE_CHANGED_CB_EVT:
        bluetooth_bass_builtin_mode_changed_cb(&bass_msg->data.builtin_mode);
        break;

    case BTMW_BASS_SYNC_LOST_CB_EVT:
        bluetooth_bass_sync_lost_cb(&bass_msg->data.sync_lost);
        break;

    default:
        break;
    }
}

static VOID  bluetooth_bass_notify_handle(tBTMW_MSG *p_msg)
{
    BT_BASS_EVENT_PARAM *bass_event = &p_msg->data.bass_event;

    switch(p_msg->hdr.event)
    {
    case BTMW_BASS_CONNECTION_CB_EVT:
    case BTMW_BASS_DEVICE_AVAIL_CB_EVT:
    case BTMW_BASS_BROADCAST_SCANNING_CHANGED_CB_EVT:
    case BTMW_BASS_BROADCAST_ANNOUNCE_RECV_CB_EVT:
    case BTMW_BASS_BROADCAST_RECV_STATE_CB_EVT:
    case BTMW_BASS_BUILTIN_MODE_CHANGED_CB_EVT:
    case BTMW_BASS_SYNC_LOST_CB_EVT:
        if (g_bt_mw_bass_event_handle_cb)
        {
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BASS, "notify to app event:%d",
                bass_event->event);
            g_bt_mw_bass_event_handle_cb(bass_event);
        }
        break;

    default:
        break;
    }
}

static VOID bluetooth_bass_connection_state_cb(BT_BASS_EVENT_CONN_STATE_DATA *p_conn_state)
{
    tBTMW_MSG msg;
    UINT8 index = 0;
    BOOL ret = FALSE;

    if (NULL == p_conn_state)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s\n", p_conn_state->bt_addr);
    switch (p_conn_state->state)
    {
    case BT_BASS_CONN_STATE_DISCONNECTED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "BSA DISCONNECTED\n");
        ret = bluetooth_bass_find_device(p_conn_state->bt_addr, &index);
        if (TRUE == ret)
        {
            g_bass_connected_devices[index].in_used = FALSE;
            g_bass_connected_devices[index].is_builtin = FALSE;
            g_bass_connected_devices[index].builtin_sync_bis = 0xFFFFFFFF;
            memset(g_bass_connected_devices[index].bt_addr, 0, MAX_BDADDR_LEN);
        }
        else
        {
            BT_DBG_WARNING(BT_DEBUG_LEAUDIO_BASS, "not find device in db\n");
        }
        break;

    case BT_BASS_CONN_STATE_CONNECTING:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "BSA CONNECTING\n");
        break;

    case BT_BASS_CONN_STATE_CONNECTED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "BSA CONNECTED\n");
        ret = bluetooth_bass_find_device(p_conn_state->bt_addr, &index);
        if (MAX_BASS_DEV_NUM == index)
        {
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bass_connected_devices num is up to max %d\n", MAX_BASS_DEV_NUM);
        }
        else if (FALSE == ret)
        {
            g_bass_connected_devices[index].in_used = TRUE;
            g_bass_connected_devices[index].is_builtin = FALSE;
            g_bass_connected_devices[index].builtin_sync_bis = 0xFFFFFFFF;
            memcpy(g_bass_connected_devices[index].bt_addr, p_conn_state->bt_addr, MAX_BDADDR_LEN);
        }
        break;

    case BT_BASS_CONN_STATE_DISCONNECTING:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "BSA DISCONNECTING\n");
        break;

    default:
        break;
    }
    memset(&msg, 0, sizeof(msg));
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_CONN_STATE_DATA);
    msg.hdr.event = BTMW_BASS_CONNECTION_CB_EVT;
    msg.data.bass_event.event = BT_BASS_EVENT_CONN_STATE;
    memcpy((BT_BASS_EVENT_CONN_STATE_DATA *)&msg.data.bass_event.data.conn_state,
        p_conn_state, sizeof(BT_BASS_EVENT_CONN_STATE_DATA));
    bt_mw_nty_send_msg(&msg);
}

static VOID bluetooth_bass_device_avail_cb(BT_BASS_EVENT_DEV_AVAIL_DATA *p_dev_avail)
{
    tBTMW_MSG msg;

    if (NULL == p_dev_avail)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, num_receivers=%d\n", p_dev_avail->bt_addr, p_dev_avail->num_receivers);
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_DEVICE_AVAIL_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_DEV_AVAIL_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_DEV_AVAIL;
    memcpy((BT_BASS_EVENT_DEV_AVAIL_DATA *)&msg.data.bass_event.data.dev_avail,
        p_dev_avail, sizeof(BT_BASS_EVENT_DEV_AVAIL_DATA));
    bt_mw_nty_send_msg(&msg);
}


static VOID bluetooth_bass_broadcast_scanning_changed_cb(BT_BASS_EVENT_SCANNING_STATE_DATA *p_scanning_state)
{
    tBTMW_MSG msg;

    if (NULL == p_scanning_state)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, scan=%d\n", p_scanning_state->bt_addr, p_scanning_state->scan);
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_BROADCAST_SCANNING_CHANGED_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_SCANNING_STATE_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_SCANNING_STATE;
    memcpy((BT_BASS_EVENT_SCANNING_STATE_DATA *)&msg.data.bass_event.data.scanning_state,
        p_scanning_state, sizeof(BT_BASS_EVENT_SCANNING_STATE_DATA));
    bt_mw_nty_send_msg(&msg);
}

static VOID bluetooth_bass_broadcast_announce_recv_cb(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA *p_announce_received)
{
    tBTMW_MSG msg;

    if (NULL == p_announce_received)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, broadcast id: bt_addr=%s, addr_type=%d, adv_sid=%d\n",
        p_announce_received->bt_addr, p_announce_received->broadcast_id.bt_addr,
        p_announce_received->broadcast_id.addr_type, p_announce_received->broadcast_id.adv_sid);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "[announce] presentation_delay=%d, subgroup_size=%d\n",
        p_announce_received->announce_data.presentation_delay,
        p_announce_received->announce_data.subgroup_size);
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_BROADCAST_ANNOUNCE_RECV_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_ANNOUNCE_RECEIVED;
    memcpy((BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA *)&msg.data.bass_event.data.announce_received,
        p_announce_received, sizeof(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA));
    bt_mw_nty_send_msg(&msg);
}

static VOID bluetooth_bass_broadcast_recv_state_cb(BT_BASS_EVENT_RECV_STATE_DATA *p_recv_state)
{
    tBTMW_MSG msg;

    if (NULL == p_recv_state)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, receiver_id=%d\n", p_recv_state->bt_addr, p_recv_state->receiver_id);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "broadcast id: bt_addr=%s, addr_type=%d, adv_sid=%d\n",
        p_recv_state->broadcast_id.bt_addr, p_recv_state->broadcast_id.addr_type, p_recv_state->broadcast_id.adv_sid);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "num_subgroups=%d\n", p_recv_state->data.num_subgroups);
    for (int i = 0; i < p_recv_state->data.num_subgroups; i++)
    {
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "subgroup[%d] sync_bis=0x%08X, meta_data_len=%d\n",
            i, p_recv_state->data.subgroup[i].sync_bis, p_recv_state->data.subgroup[i].meta_data_len);
    }
    switch (p_recv_state->state)
    {
    case BT_BASS_RECV_STATE_IDLE:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "IDLE\n");
        break;

    case BT_BASS_RECV_STATE_SET_SOURCE_FAILED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "SET SOURCE FAILED\n");
        break;

    case BT_BASS_RECV_STATE_SYNCING:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "SYNCING\n");
        break;

    case BT_BASS_RECV_STATE_SYNC_PA_FAILED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "SYNC PA FAILED\n");
        break;

    case BT_BASS_RECV_STATE_SYNC_BIS_STOPPED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "SYNC BIS STOPPED\n");
        break;

    case BT_BASS_RECV_STATE_SYNCED_TO_PA:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "SYNCED TO PA\n");
        break;

    case BT_BASS_RECV_STATE_BROADCAST_CODE_REQUIRED:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "BROADCAST CODE REQUIRED\n");
        break;

    case BT_BASS_RECV_STATE_RECEIVING_BROADCAST:
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "RECEIVING BROADCAST\n");
        break;

    default:
        break;
    }
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_BROADCAST_RECV_STATE_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_RECV_STATE_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_RECEIVER_STATE;
    memcpy((BT_BASS_EVENT_RECV_STATE_DATA *)&msg.data.bass_event.data.recv_state,
        p_recv_state, sizeof(BT_BASS_EVENT_RECV_STATE_DATA));
    bt_mw_nty_send_msg(&msg);
}

static VOID bluetooth_bass_builtin_mode_changed_cb(BT_BASS_EVENT_BUILTIN_MODE_DATA *p_builtin_mode)
{
    tBTMW_MSG msg;
    UINT8 index = 0;
    BOOL ret = FALSE;

    if (NULL == p_builtin_mode)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, enable=%d\n", p_builtin_mode->bt_addr, p_builtin_mode->enable);
    ret = bluetooth_bass_find_device(p_builtin_mode->bt_addr, &index);
    if (TRUE == ret)
    {
        g_bass_connected_devices[index].is_builtin = p_builtin_mode->enable;
        g_bass_connected_devices[index].builtin_sync_bis = p_builtin_mode->sync_bis;
    }
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_BUILTIN_MODE_CHANGED_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_BUILTIN_MODE_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_BUILTIN_MODE;
    memcpy((BT_BASS_EVENT_BUILTIN_MODE_DATA *)&msg.data.bass_event.data.builtin_mode,
        p_builtin_mode, sizeof(BT_BASS_EVENT_BUILTIN_MODE_DATA));
    bt_mw_nty_send_msg(&msg);
}

static VOID bluetooth_bass_sync_lost_cb(BT_BASS_EVENT_SYNC_LOST_DATA *p_sync_lost)
{
    tBTMW_MSG msg;

    if (NULL == p_sync_lost)
    {
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, addr_type=%d, adv_sid=%d\n",
                        p_sync_lost->broadcast_id.bt_addr, p_sync_lost->broadcast_id.addr_type,
                        p_sync_lost->broadcast_id.adv_sid);
    memset(&msg, 0, sizeof(msg));
    msg.hdr.event = BTMW_BASS_SYNC_LOST_CB_EVT;
    msg.hdr.len = sizeof(BT_BASS_EVENT) + sizeof(BT_BASS_EVENT_SYNC_LOST_DATA);
    msg.data.bass_event.event = BT_BASS_EVENT_SYNC_LOST;
    memcpy((BT_BASS_EVENT_SYNC_LOST_DATA *)&msg.data.bass_event.data.sync_lost, p_sync_lost,
                    sizeof(BT_BASS_EVENT_SYNC_LOST_DATA));
    bt_mw_nty_send_msg(&msg);
}

INT32 bluetooth_bass_connect(BT_BASS_CONN_PARAM *param)
{
    return linuxbt_bass_connect(param->bt_addr);
}

INT32 bluetooth_bass_disconnect(BT_BASS_DISC_PARAM *param)
{
    return linuxbt_bass_disconnect(param->bt_addr);
}

INT32 bluetooth_bass_set_broadcast_scan(BT_BASS_BROADCAST_SCAN_PARAM *param)
{
    return linuxbt_bass_set_broadcast_scan(param->bt_addr, param->scan, param->duration);
}

INT32 bluetooth_bass_stop_broadcast_observing(VOID)
{
    return linuxbt_bass_stop_broadcast_observing();
}

INT32 bluetooth_bass_get_broadcast_receiver_state(BT_BASS_BROADCAST_RECV_STATE_PARAM *param)
{
    return linuxbt_bass_get_broadcast_receiver_state(param->bt_addr, param->receiver_id);
}

INT32 bluetooth_bass_set_broadcast_code(BT_BASS_BROADCAST_CODE_PARAM *param)
{
    return linuxbt_bass_set_broadcast_code(param->bt_addr, param->receiver_id, param->code);
}

INT32 bluetooth_bass_set_broadcast_source(BT_BASS_SET_BROADCAST_SRC_PARAM *param)
{
    BASS_BROADCAST_SOURCE data;

    memset(&data, 0, sizeof(data));
    data.sync_pa = param->broadcast_src.sync_pa;
    data.num_subgroups = param->broadcast_src.num_subgroups;
    for (int i = 0; i < data.num_subgroups; i++)
    {
        memcpy(data.subgroup, param->broadcast_src.subgroup, sizeof(data.subgroup));
    }
    return linuxbt_bass_set_broadcast_source(param->broadcast_src.bt_addr, param->broadcast_id.bt_addr,
        param->broadcast_id.addr_type, param->broadcast_id.adv_sid, &data);
}

INT32 bluetooth_bass_modify_broadcast_source(BT_BASS_MODIFY_BROADCAST_SRC_PARAM *param)
{
    BASS_BROADCAST_SOURCE data;

    memset(&data, 0, sizeof(data));
    data.sync_pa = param->broadcast_src.sync_pa;
    data.num_subgroups = param->broadcast_src.num_subgroups;
    for (int i = 0; i < data.num_subgroups; i++)
    {
        memcpy(data.subgroup, param->broadcast_src.subgroup, sizeof(data.subgroup));
    }

    return linuxbt_bass_modify_broadcast_source(param->broadcast_src.bt_addr, param->receiver_id, &data);
}

INT32 bluetooth_bass_remove_broadcast_source(BT_BASS_REMOVE_BROADCAST_SRC_PARAM *param)
{
    return linuxbt_bass_remove_broadcast_source(param->bt_addr, param->receiver_id);
}

INT32 bluetooth_bass_set_builtin_mode(BT_BASS_SET_BUILTIN_MODE_PARAM *param)
{
    return linuxbt_bass_set_builtin_mode(param->bt_addr, param->enable, param->sync_bis);
}

INT32 bluetooth_bass_get_connection_status(BT_BASS_CONNECTION_INFO *conn_info)
{
    memcpy(conn_info->device, g_bass_connected_devices, sizeof(g_bass_connected_devices));
    return BT_SUCCESS;
}

INT32 bluetooth_bass_register_callback(BT_BASS_EVENT_HANDLE_CB bass_handle)
{
    int ret = BT_SUCCESS;
    if (NULL != bass_handle)
    {
        ret = bluetooth_bass_init();
    }
    else
    {
        ret = bluetooth_bass_deinit();
    }
    g_bt_mw_bass_event_handle_cb = bass_handle;
    return ret;
}
