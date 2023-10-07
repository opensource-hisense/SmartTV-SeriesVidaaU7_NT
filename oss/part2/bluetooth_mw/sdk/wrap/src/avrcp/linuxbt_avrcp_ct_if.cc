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

/* FILE NAME:  linuxbt_avrcp_ct_if.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "mtk_bluetooth.h"
#include "bt_mw_avrcp.h"
#include "bt_mw_message_queue.h"
#include "linuxbt_avrcp_ct_if.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_AVRCP_CT_SET_MSG_LEN(msg) do{       \
    msg.hdr.len = sizeof(tBT_MW_AVRCP_MSG);         \
    }while(0)
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
extern CHAR g_bt_avrcp_addr[MAX_BDADDR_LEN];
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void linuxbt_rc_passthrough_rsp_cb(const RawAddress& bd_addr, int id, int key_state);

static void linuxbt_rc_connection_state_cb(bool rc_connect, bool bt_connect, const RawAddress& bd_addr);

static void linuxbt_rc_ctrl_groupnavigation_rsp_cb(int id, int key_state);

static void linuxbt_rc_ctrl_setabsvol_cmd_cb(const RawAddress& bd_addr,
    uint8_t abs_vol, uint8_t label);
static void linuxbt_rc_ctrl_registernotification_abs_vol_cb (const RawAddress& bd_addr,
    uint8_t label);
static void linuxbt_rc_ctrl_play_position_changed_cb(const RawAddress& bd_addr,
    uint32_t song_len, uint32_t song_pos);
static void linuxbt_rc_ctrl_play_status_changed_cb(const RawAddress& bd_addr,
    btrc_play_status_t play_status);

static void linuxbt_rc_ctrl_get_folder_items_cb(
    const RawAddress& bd_addr, btrc_status_t status,
    const btrc_folder_items_t* folder_items, uint8_t count);

static void linuxbt_rc_ctrl_change_folder_path_cb(const RawAddress& bd_addr,
                                               uint32_t count);
static void linuxbt_rc_ctrl_set_browsed_player_cb(const RawAddress& bd_addr,
                                                      uint8_t num_items,
                                                      uint8_t depth);
static void linuxbt_rc_ctrl_set_addressed_player_cb(const RawAddress& bd_addr,
                                                      uint8_t status);

static void linuxbt_rc_ctrl_addressed_player_changed_cb(const RawAddress& bd_addr,
                                                      uint16_t id);

static void linuxbt_rc_ctrl_now_playing_contents_changed_cb(
    const RawAddress& bd_addr);

static void linuxbt_rc_ctrl_remote_features_cb(const RawAddress& bd_addr, int features);
static void linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb(const RawAddress& bd_addr,
    uint8_t accepted);
static void linuxbt_rc_ctrl_playerapplicationsetting_cb(const RawAddress& bd_addr,
     uint8_t num_attr,
     btrc_player_app_attr_t *app_attrs,
     uint8_t num_ext_attr,
     btrc_player_app_ext_attr_t *ext_attrs);

static void linuxbt_rc_ctrl_playerapplicationsetting_changed_cb(const RawAddress& bd_addr,
    const btrc_player_settings_t &p_vals);
static void linuxbt_rc_ctrl_track_changed_cb(const RawAddress& bd_addr,
    uint8_t num_attr, btrc_element_attr_val_t *p_attrs);

static void linuxbt_rc_tg_passthrough_volume_cmd_cb (const RawAddress& bd_addr,
    int id, int is_press);

static CHAR* linuxbt_rc_get_play_status_str(btrc_play_status_t status);

/* STATIC VARIABLE DECLARATIONS
 */
static btrc_ctrl_interface_t *g_bt_rc_ct_interface = NULL;

static btrc_ctrl_callbacks_t g_bt_rc_callbacks =
{
    sizeof(btrc_ctrl_callbacks_t),
    linuxbt_rc_passthrough_rsp_cb,
    linuxbt_rc_ctrl_groupnavigation_rsp_cb, //btrc_groupnavigation_rsp_callback //need refactor
    linuxbt_rc_connection_state_cb, //need refactor
    linuxbt_rc_ctrl_remote_features_cb, //btrc_ctrl_getrcfeatures_callback
    linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb, //btrc_ctrl_setplayerapplicationsetting_rsp_callback
    linuxbt_rc_ctrl_playerapplicationsetting_cb, //btrc_ctrl_playerapplicationsetting_callback
    linuxbt_rc_ctrl_playerapplicationsetting_changed_cb, //btrc_ctrl_playerapplicationsetting_changed_callback //need refactor
    linuxbt_rc_ctrl_setabsvol_cmd_cb, //btrc_ctrl_setabsvol_cmd_callback
    linuxbt_rc_ctrl_registernotification_abs_vol_cb, //btrc_ctrl_registernotification_abs_vol_callback
    linuxbt_rc_ctrl_track_changed_cb, //btrc_ctrl_track_changed_callback
    linuxbt_rc_ctrl_play_position_changed_cb, //btrc_ctrl_play_position_changed_callback
    linuxbt_rc_ctrl_play_status_changed_cb, //btrc_ctrl_play_status_changed_callback
    linuxbt_rc_ctrl_get_folder_items_cb, //btrc_ctrl_get_folder_items_callback
    linuxbt_rc_ctrl_change_folder_path_cb, //btrc_ctrl_change_path_callback
    linuxbt_rc_ctrl_set_browsed_player_cb, //btrc_ctrl_set_browsed_player_callback
    linuxbt_rc_ctrl_set_addressed_player_cb, //btrc_ctrl_set_addressed_player_callback
    linuxbt_rc_ctrl_addressed_player_changed_cb, //btrc_ctrl_addressed_player_changed_callback
    linuxbt_rc_ctrl_now_playing_contents_changed_cb, //btrc_ctrl_now_playing_contents_changed_callback
    /** M: add for pts AVRCP_TG_PTT_BV_02_I @{ */
    linuxbt_rc_tg_passthrough_volume_cmd_cb // add for pts
  /** @} */
};

/* EXPORTED SUBPROGRAM BODIES
 */

INT32 linuxbt_rc_init(void)
{
    int ret = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    g_bt_rc_ct_interface =
        (btrc_ctrl_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    // Init AVRCP interface
    ret = g_bt_rc_ct_interface->init(&g_bt_rc_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, " success to init AVRCP CT interface");
        return BT_SUCCESS;
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, " already init AVRCP CT interface");
        return BT_SUCCESS;
    }
    BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to init AVRCP CT interface");

    return ret;
}

INT32 linuxbt_rc_deinit(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    g_bt_rc_ct_interface->cleanup();

    g_bt_rc_ct_interface = NULL;

    return BT_SUCCESS;

}

INT32 linuxbt_rc_change_player_app_setting_handler(CHAR *addr, BT_AVRCP_PLAYER_SETTING *player_setting)
{
    RawAddress bdaddr;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_setting);

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);


    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s, num_attribute = %d",
        addr, player_setting->num_attr);

    g_bt_rc_ct_interface->set_player_app_setting_cmd(bdaddr,
            player_setting->num_attr, player_setting->attr_ids, player_setting->attr_values);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_passthrough_cmd_handler(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    RawAddress bdaddr;
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);

    INT32 key_map[BT_AVRCP_CMD_TYPE_MAX] = {
          AVRC_ID_PLAY,
          AVRC_ID_PAUSE,
          AVRC_ID_FORWARD,
          AVRC_ID_BACKWARD,
          AVRC_ID_FAST_FOR,
          AVRC_ID_REWIND,
          AVRC_ID_STOP,
          AVRC_ID_VOL_UP,
          AVRC_ID_VOL_DOWN,
          AVRC_ID_CHAN_UP,
          AVRC_ID_CHAN_DOWN,
          AVRC_ID_MUTE,
          AVRC_ID_POWER };

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s, cmd=%d, action=%d",
        addr, cmd_type, key_state);

    if (BT_AVRCP_CMD_TYPE_MAX <= cmd_type)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "cmd_type=%d", cmd_type);
        return -1;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    if (BT_AVRCP_KEY_STATE_PRESS == key_state)
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
    }
    else if (BT_AVRCP_KEY_STATE_RELEASE == key_state)
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }
    else
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
        usleep(200*1000);
        g_bt_rc_ct_interface->send_pass_through_cmd(bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }

    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_vendor_unique_cmd_handler(CHAR *addr, UINT8 key,
    BT_AVRCP_KEY_STATE key_state)
{
    RawAddress bdaddr;

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "key=%d, action=%d", key, key_state);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    if (BT_AVRCP_KEY_STATE_PRESS == key_state)
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(bdaddr,
            key, AVRC_STATE_PRESS);
    }
    else if (BT_AVRCP_KEY_STATE_RELEASE == key_state)
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(bdaddr,
            key, AVRC_STATE_RELEASE);
    }
    else
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(bdaddr,
            key, AVRC_STATE_PRESS);
        usleep(200*1000);
        g_bt_rc_ct_interface->send_group_navigation_cmd(bdaddr,
            key, AVRC_STATE_RELEASE);
    }

    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_get_playstatus_cmd_handler(CHAR *addr)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    g_bt_rc_ct_interface->get_playback_state_cmd(bdaddr);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_playitem_handler(CHAR *addr, BT_AVRCP_PLAYITEM *playitem)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, playitem);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] playitem->scope:%d, uid=%lld, uid_counter = %d",
        playitem->scope, playitem->uid, playitem->uid_counter);
    g_bt_rc_ct_interface->play_item_cmd(bdaddr,
            playitem->scope, (UINT8 *)&(playitem->uid), playitem->uid_counter);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_get_now_playing_list_handler(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);


    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] list_range->start:%d, list_range->end = %d",
        list_range->start, list_range->end);
    g_bt_rc_ct_interface->get_now_playing_list_cmd(bdaddr,
            list_range->start, list_range->end);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_get_folder_list_handler(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] list_range->start:%d, list_range->end = %d",
        list_range->start, list_range->end);
    g_bt_rc_ct_interface->get_folder_list_cmd(bdaddr,
            list_range->start, list_range->end);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_get_player_list_handler(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] list_range->start:%d, list_range->end = %d",
        list_range->start, list_range->end);
    g_bt_rc_ct_interface->get_player_list_cmd(bdaddr,
            list_range->start, list_range->end);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_change_folder_path_handler(CHAR *addr, BT_AVRCP_FOLDER_PATH *folder_path)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, folder_path);
    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] folder_direction:%d, uid = %lld",
        folder_path->folder_direction, folder_path->uid);
    g_bt_rc_ct_interface->change_folder_path_cmd(bdaddr,
            folder_path->folder_direction, (UINT8 *)&(folder_path->uid));

    return BT_SUCCESS;
}

INT32 linuxbt_rc_set_browsed_player_handler(CHAR *addr, UINT16 playerId)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] playerId:%d", playerId);
    g_bt_rc_ct_interface->set_browsed_player_cmd(bdaddr, playerId);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_set_addressed_player_handler(CHAR *addr, UINT16 playerId)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] playerId:%d", playerId);
    g_bt_rc_ct_interface->set_addressed_player_cmd(bdaddr, playerId);

    return BT_SUCCESS;
}

INT32 linuxbt_rc_set_volume_rsp(CHAR *addr, UINT8 label, UINT8 volume)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP interface");
        return -1;
    }
    g_bt_rc_ct_interface->set_volume_rsp(bdaddr, volume, label);
    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_volume_change_rsp_handler(CHAR *addr, INT32 interim,
    UINT8 label, UINT32 volume)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] send volume:%x", volume);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP interface");
        return -1;
    }

    if (interim)
    {
        g_bt_rc_ct_interface->register_abs_vol_rsp(bdaddr,
            BTRC_NOTIFICATION_TYPE_INTERIM, volume, label);
    }
    else
    {
        g_bt_rc_ct_interface->register_abs_vol_rsp(bdaddr,
            BTRC_NOTIFICATION_TYPE_CHANGED, volume, label);
    }

    return BT_SUCCESS;
}

#if defined(BT_RPC_DBG_SERVER)
EXPORT_SYMBOL int dbg_avrcp_get_g_media_info(int array_index, int offset,
char *name, char *data, int length)
{
    switch(offset)
    {
        case 0:
            sprintf(name, "title");
            break;
        case 1:
            sprintf(name, "artist");
            break;
        case 2:
            sprintf(name, "album");
            break;
        case 3:
            sprintf(name, "current_track_number");
            break;
        case 4:
            sprintf(name, "number_of_tracks");
            break;
        case 5:
            sprintf(name, "genre");
            break;
        case 6:
            sprintf(name, "position");
            break;
        default:
            return BT_SUCCESS;
    }

    return offset + 1;
}

EXPORT_SYMBOL int dbg_avrcp_get_g_player_status(int array_index, int offset,
    char *name, char *data, int length)
{
    switch(offset)
    {
        case 0:
            sprintf(name, "song_length");
            break;
        case 1:
            sprintf(name, "song_position");
            break;
        case 2:
            sprintf(name, "play_status");
            break;

        default:
            return BT_SUCCESS;
    }

        return offset + 1;
    }


#endif

/* LOCAL SUBPROGRAM BODIES
 */

static void linuxbt_rc_passthrough_rsp_cb(const RawAddress& bd_addr, int id, int key_state)
{
    CHAR addr[MAX_BDADDR_LEN];
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s id = %x, key state = %x ", addr, id, key_state);
}

static void linuxbt_rc_connection_state_cb(bool rc_connect, bool bt_connect, const RawAddress& bd_addr)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));

    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "rc_connect = %x bd_addr = %s",
        rc_connect, btmw_msg.data.avrcp_msg.addr);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    if(FALSE == rc_connect)
    {
        g_bt_avrcp_addr[0] = 0;
        btmw_msg.hdr.event = BTMW_AVRCP_DISCONNECTED;
    }
    else
    {
        strncpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr, MAX_BDADDR_LEN - 1);
        g_bt_avrcp_addr[MAX_BDADDR_LEN - 1] = '\0';
        btmw_msg.hdr.event = BTMW_AVRCP_CONNECTED;
    }
    btmw_msg.data.avrcp_msg.data.connection_cb.rc_connect = rc_connect;
    btmw_msg.data.avrcp_msg.data.connection_cb.bt_connect = bt_connect;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

//need refactor
static void linuxbt_rc_ctrl_groupnavigation_rsp_cb(int id, int key_state)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "id = %d key_state = %d ",  id, key_state);
}

static void linuxbt_rc_ctrl_remote_features_cb(const RawAddress& bd_addr, int features)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "[AVRCP]%s CT features = 0x%x ",
        btmw_msg.data.avrcp_msg.addr, features);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.feature = features;

    btmw_msg.hdr.event = BTMW_AVRCP_FEATURE;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb(const RawAddress& bd_addr,
    uint8_t accepted)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s accepted = %d ", btmw_msg.data.avrcp_msg.addr, accepted);

    btmw_msg.hdr.event = BTMW_AVRCP_PLAYER_SETTING_RSP;
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;

    btmw_msg.data.avrcp_msg.data.playersetting_rsp.accepted = accepted;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_playerapplicationsetting_cb(const RawAddress& bd_addr,
     uint8_t num_attr,
     btrc_player_app_attr_t *app_attrs,
     uint8_t num_ext_attr,
     btrc_player_app_ext_attr_t *ext_attrs)
{
    BT_AVRCP_LIST_PLAYERSETTING_RSP *list_playersetting_rsp = NULL;

    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d, num_ext_attr = %d ",
        btmw_msg.data.avrcp_msg.addr, num_attr, num_ext_attr);

    list_playersetting_rsp = &btmw_msg.data.avrcp_msg.data.list_playersetting_rsp;
    btmw_msg.hdr.event = BTMW_AVRCP_LIST_PLAYER_SETTING_RSP;
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;

    list_playersetting_rsp->num_attr = num_attr;
    list_playersetting_rsp->num_ext_attr = num_ext_attr;

    if (app_attrs != NULL)
    {
        memcpy(&list_playersetting_rsp->player_app_attr, app_attrs, sizeof(btrc_player_app_attr_t));
    }

    if (ext_attrs != NULL)
    {
        list_playersetting_rsp->player_app_ext_attr.attr_id = ext_attrs->attr_id;
        list_playersetting_rsp->player_app_ext_attr.charset_id = ext_attrs->charset_id;
        list_playersetting_rsp->player_app_ext_attr.str_len = ext_attrs->str_len;
        list_playersetting_rsp->player_app_ext_attr.num_val = ext_attrs->num_val;

        if (ext_attrs->str_len > BT_AVRCP_MAX_APP_ATTR_STRLEN)
        {
            ext_attrs->str_len = BT_AVRCP_MAX_APP_ATTR_STRLEN;
        }
        memcpy(list_playersetting_rsp->player_app_ext_attr.p_str, ext_attrs->p_str, ext_attrs->str_len);

        for (int ii = 0; ii < ext_attrs->num_val; ii++)
        {
            list_playersetting_rsp->player_app_ext_attr.ext_attr_val[ii].charset_id = ext_attrs->ext_attr_val[ii].charset_id;
            list_playersetting_rsp->player_app_ext_attr.ext_attr_val[ii].val = ext_attrs->ext_attr_val[ii].val;
            list_playersetting_rsp->player_app_ext_attr.ext_attr_val[ii].str_len = ext_attrs->ext_attr_val[ii].str_len;
            if (ext_attrs->ext_attr_val[ii].str_len > BT_AVRCP_MAX_APP_ATTR_STRLEN)
            {
                ext_attrs->ext_attr_val[ii].str_len = BT_AVRCP_MAX_APP_ATTR_STRLEN;
            }
            memcpy(list_playersetting_rsp->player_app_ext_attr.ext_attr_val[ii].p_str,
                  ext_attrs->ext_attr_val[ii].p_str, ext_attrs->ext_attr_val[ii].str_len);
        }
    }
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_playerapplicationsetting_changed_cb(const RawAddress& bd_addr,
    const btrc_player_settings_t &p_vals)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d ", btmw_msg.data.avrcp_msg.addr,
           p_vals.num_attr);

    btmw_msg.hdr.event = BTMW_AVRCP_PLAYER_SETTING_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    memcpy(&btmw_msg.data.avrcp_msg.data.playersetting_change.player_setting, &p_vals, sizeof(btrc_player_settings_t));
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_rc_ctrl_setabsvol_cmd_cb(const RawAddress& bd_addr,
    uint8_t abs_vol, uint8_t label)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s label = %x volume = %d",
        btmw_msg.data.avrcp_msg.addr, label,abs_vol);

    btmw_msg.hdr.event = BTMW_AVRCP_SET_VOLUME_REQ;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.label = label;
    btmw_msg.data.avrcp_msg.data.set_vol_req.abs_volume = abs_vol;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_registernotification_abs_vol_cb(const RawAddress& bd_addr,
    uint8_t label)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s label = %x ",
        btmw_msg.data.avrcp_msg.addr, label);

    btmw_msg.hdr.event = BTMW_AVRCP_REG_EVENT_REQ;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.label = label;
    btmw_msg.data.avrcp_msg.data.reg_event_req.event_id = BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_track_changed_cb(const RawAddress& bd_addr,
    uint8_t num_attr, btrc_element_attr_val_t *p_attrs)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    int i = 0;
    UINT8 *ptr_data = NULL;
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d ",
        btmw_msg.data.avrcp_msg.addr, num_attr);

    btmw_msg.hdr.event = BTMW_AVRCP_TRACK_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    if(p_attrs[0].attr_id == 0xFF)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " ERROR %d", p_attrs[0].attr_id);
        return;
    }

    for ( i = 0 ; i < num_attr ; i++ )
    {
        ptr_data = (UINT8 *)&p_attrs[i].text;

        switch ((int)p_attrs[i].attr_id)
        {
            case BTRC_MEDIA_ATTR_TITLE://BTRC_MEDIA_ATTR_TITLE = 0x01
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.title,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.title[BT_AVRCP_MAX_NAME_LEN-1] = '\0';
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](title):%s",
                    p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.title);
                break;
            }
            case BTRC_MEDIA_ATTR_ARTIST://BTRC_MEDIA_ATTR_ARTIST = 0x02
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.artist,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.artist[BT_AVRCP_MAX_NAME_LEN-1] = '\0';
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](artist):%s",
                    (int)p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.artist);
                break;
            }
            case BTRC_MEDIA_ATTR_ALBUM://BTRC_MEDIA_ATTR_ALBUM = 0x03
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.album,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.album[BT_AVRCP_MAX_NAME_LEN-1] = '\0';
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](album):%s",
                    (int)p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.album);
                break;
            }
            case BTRC_MEDIA_ATTR_TRACK_NUM://BTRC_MEDIA_ATTR_TRACK_NUM = 0x04
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](track_number):%s",
                    (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.current_track_number = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "current_track_number:%ld",
                    (long)btmw_msg.data.avrcp_msg.data.track_change.element_attr.current_track_number);
                break;
            }
            case BTRC_MEDIA_ATTR_NUM_TRACKS://BTRC_MEDIA_ATTR_NUM_TRACKS = 0x05
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](track_number):%s",
                    (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.number_of_tracks = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "number_of_tracks:%ld",
                    (long)btmw_msg.data.avrcp_msg.data.track_change.element_attr.number_of_tracks);
                break;
            }
            case BTRC_MEDIA_ATTR_GENRE://BTRC_MEDIA_ATTR_GENRE = 0x06
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.genre,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.genre[BT_AVRCP_MAX_NAME_LEN-1]= '\0';
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](genre):%s",
                    (int)p_attrs[i].attr_id, btmw_msg.data.avrcp_msg.data.track_change.element_attr.genre);
                break;
            }
            case BTRC_MEDIA_ATTR_PLAYING_TIME://BTRC_MEDIA_ATTR_PLAYING_TIME = 0x07
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](position):%s", (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.position = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "position:%d",
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.position);
                break;
            }
            default:
            {
                BT_DBG_WARNING(BT_DEBUG_AVRCP, "Unknown Attribute_ID : %d",
                    (int)p_attrs[i].attr_id);
                break;
            }
        }
    }
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_rc_ctrl_play_position_changed_cb(const RawAddress& bd_addr,
    uint32_t song_len, uint32_t song_pos)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s play changed, song_len=%d, song_pos=%d",
        btmw_msg.data.avrcp_msg.addr, song_len, song_pos);

    btmw_msg.hdr.event = BTMW_AVRCP_PLAY_POS_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.pos_change.song_len = song_len;
    btmw_msg.data.avrcp_msg.data.pos_change.song_pos = song_pos;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_play_status_changed_cb(const RawAddress& bd_addr,
    btrc_play_status_t play_status)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s play status changed, play_status=%d(%s)",
        btmw_msg.data.avrcp_msg.addr, play_status,
        linuxbt_rc_get_play_status_str(play_status));

    btmw_msg.hdr.event = BTMW_AVRCP_PLAY_STATUS_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.play_status_change.play_status = (BT_AVRCP_PLAY_STATUS)play_status;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_get_folder_items_cb(
    const RawAddress& bd_addr, btrc_status_t status,
    const btrc_folder_items_t* folder_items, uint8_t count)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    UINT8 i = 0;
    UINT32 j = 0;
    UINT32 folder_count = 0;
    UINT32 media_max_attr_num = 0;
    UINT32 name_len = 0;
    UINT32 text_len = 0;
    CHAR * p_name = NULL;
    CHAR * p_text = NULL;
    BT_AVRCP_FOLDER_ITEMS mw_folder_itmes[MAX_COUNT_FOLDER_ITEM];
    memset(mw_folder_itmes, 0, sizeof(mw_folder_itmes));
    BT_AVRCP_FOLDER_ITEMS *p_mw_folder_items = mw_folder_itmes;

    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    if (folder_items == NULL)
    {
        BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "folder_items is NULL count=%d status =%d", count, status);
        btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB;

        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;

        btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.count = count;
        btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.status = (BT_AVRCP_STATUS)status;

        LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);
        return;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "item_type=%d", folder_items[0].item_type);
    folder_count = count;
    if (folder_count > MAX_COUNT_FOLDER_ITEM)
    {
        folder_count = MAX_COUNT_FOLDER_ITEM;
    }
    if (folder_items != NULL)
    {
        for (i = 0; i < folder_count; i++)
        {
            p_mw_folder_items->item_type = (BT_AVRCP_ITEM_TYPE)folder_items->item_type;
            BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "p_mw_folder_items->item_type=%d", p_mw_folder_items->item_type);
            BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "mw_folder_itmessize=%d", sizeof(mw_folder_itmes));
            if (folder_items->item_type == 0x01)
            {
                p_mw_folder_items->item.player.player_id =
                    folder_items->player.player_id;
                p_mw_folder_items->item.player.major_type =
                    (BT_AVRCP_ITEM_PLAYER_MJ_TYPE)folder_items->player.major_type;
                p_mw_folder_items->item.player.sub_type =
                    (BT_AVRCP_ITEM_PLAYER_MJ_TYPE)folder_items->player.sub_type;
                p_mw_folder_items->item.player.play_status =
                    (BT_AVRCP_PLAY_STATUS)folder_items->player.play_status;
                p_mw_folder_items->item.player.charset_id =
                    folder_items->player.charset_id;
                memcpy(p_mw_folder_items->item.player.features,
                    folder_items->player.features, BTRC_MW_FEATURE_BIT_MASK_SIZE);
                p_name = (CHAR*)folder_items->player.name;
                name_len = strlen(p_name);
                if (name_len > BTRC_MW_MAX_ATTR_STR_LEN)
                {
                    name_len = BTRC_MW_MAX_ATTR_STR_LEN;
                }
                memcpy(p_mw_folder_items->item.player.name,
                    folder_items->player.name, name_len);
            }
            else if(folder_items->item_type == 0x02)
            {
                p_mw_folder_items->item.folder.type =
                    (BT_AVRCP_ITEM_FOLDER_TYPE)folder_items->folder.type;
                p_mw_folder_items->item.folder.playable =
                    folder_items->folder.playable;
                p_mw_folder_items->item.folder.charset_id =
                    folder_items->folder.charset_id;
                BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "folder charset_id=%d", p_mw_folder_items->item.folder.charset_id);
                memcpy(p_mw_folder_items->item.folder.uid,
                    folder_items->folder.uid, BT_AVRCP_UID_SIZE);
                BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "folder uid[0]=%d",
                    p_mw_folder_items->item.folder.uid[0]);
                p_name = (CHAR*)folder_items->folder.name;
                name_len = strlen(p_name);
                if (name_len >= BTRC_MW_MAX_ATTR_STR_LEN)
                {
                    name_len = BTRC_MW_MAX_ATTR_STR_LEN - 1;
                }
                BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "name_len=%d",
                    name_len);
                memcpy(p_mw_folder_items->item.folder.name,
                    folder_items->folder.name, name_len);
                p_mw_folder_items->item.folder.name[BTRC_MW_MAX_ATTR_STR_LEN -1] = '\0';
                BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "folder name =%s",
                    p_mw_folder_items->item.folder.name);
            }
            else if (folder_items->item_type == 0x03)
            {
                p_mw_folder_items->item.media.type =
                    (BT_AVRCP_ITEM_MEDIA_TYPE)folder_items->media.type;
                p_mw_folder_items->item.media.charset_id =
                    folder_items->media.charset_id;
                p_mw_folder_items->item.media.num_attrs =
                    folder_items->media.num_attrs;
                memcpy(p_mw_folder_items->item.media.uid,
                    folder_items->media.uid, BT_AVRCP_UID_SIZE);
                p_name = (CHAR*)folder_items->media.name;
                name_len = strlen(p_name);
                if (name_len > BTRC_MW_MAX_ATTR_STR_LEN)
                {
                    name_len = BTRC_MW_MAX_ATTR_STR_LEN;
                }
                memcpy(p_mw_folder_items->item.media.name,
                    folder_items->media.name, name_len);
                media_max_attr_num = folder_items->media.num_attrs;
                BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "media_max_attr_num=%d", media_max_attr_num);
                if (media_max_attr_num > BT_AVRCP_MAX_ATTR_CNT)
                {
                    media_max_attr_num = BT_AVRCP_MAX_ATTR_CNT;
                }
                for (j = 0; j < media_max_attr_num; j++)
                {

                    p_mw_folder_items->item.media.p_attrs[j].attr_id =
                        (BT_AVRCP_MEDIA_ATTR)folder_items->media.p_attrs[j].attr_id;
                    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "attr_id=%d",
                             p_mw_folder_items->item.media.p_attrs[j].attr_id);
                    p_text = (CHAR*)folder_items->media.p_attrs[j].text;
                    text_len = strlen(p_text);

                    if (text_len >= BTRC_MW_MAX_ATTR_STR_LEN)
                    {
                        text_len = BTRC_MW_MAX_ATTR_STR_LEN - 1;
                    }
                    memcpy(p_mw_folder_items->item.media.p_attrs[j].text,
                        folder_items->media.p_attrs[j].text, text_len);
                    p_mw_folder_items->item.media.p_attrs[j].text[BTRC_MW_MAX_ATTR_STR_LEN - 1] = '\0';
                    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "text from stack=%s",
                             folder_items->media.p_attrs[j].text);
                    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "text=%s",
                             p_mw_folder_items->item.media.p_attrs[j].text);
                }
            }
            folder_items++;
            p_mw_folder_items++;
            BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "mw_folder_itmes[%d] tyep=%d", i,
                mw_folder_itmes[i].item_type);

        }

    }

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;

    btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.count = folder_count;
    btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.status = (BT_AVRCP_STATUS)status;
    memcpy(&btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.folder_items,
        mw_folder_itmes, sizeof(mw_folder_itmes));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "folder_count=%d",
                btmw_msg.data.avrcp_msg.data.get_folder_itmes_cb.get_itmes_cb.count);

    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_change_folder_path_cb(const RawAddress& bd_addr,
                                               uint32_t count)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;
    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s count=%d",
        btmw_msg.data.avrcp_msg.addr, count);

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.change_folder_path_cb.change_folder_path_cb_data.count
        = count;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);

}

static void linuxbt_rc_ctrl_set_browsed_player_cb(const RawAddress& bd_addr,
                                                      uint8_t num_items,
                                                      uint8_t depth)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;
    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_items=%d, depth=%d",
        btmw_msg.data.avrcp_msg.addr, num_items, depth);

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.set_browsed_player_cb.set_browsed_player_cb_data.depth
        = depth;
    btmw_msg.data.avrcp_msg.data.set_browsed_player_cb.set_browsed_player_cb_data.num_items
        = num_items;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_set_addressed_player_cb(
    const RawAddress& bd_addr, uint8_t status)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;
    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s status=%d",
        btmw_msg.data.avrcp_msg.addr, status);

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.set_addressed_player_cb.set_addressed_player_cb_data.status
        = (BT_AVRCP_STATUS)status;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_addressed_player_changed_cb(
    const RawAddress& bd_addr, uint16_t id)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;
    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s id=%d",
        btmw_msg.data.avrcp_msg.addr, id);

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.addressed_player_changed_cb.addressed_player_changed_cb_data.id
        = id;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_now_playing_contents_changed_cb(
    const RawAddress& bd_addr)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;
    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s", btmw_msg.data.avrcp_msg.addr);

    btmw_msg.hdr.event = BTMW_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

/* just for PTS:AVRCP_TG_PTT_BV_02_I
local role:     A2DP SNK, AVRCP TG
remote role:  A2DP SRC, AVRCP CT
remote send AVRCP VOLUME UP/DOWN passthrough command to local
*/

static void linuxbt_rc_tg_passthrough_volume_cmd_cb (const RawAddress& bd_addr, int id, int is_press)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));

    BT_AVRCP_CMD_TYPE bt_cmd_type = BT_AVRCP_CMD_TYPE_MAX;
    int i = 0;

    INT32 key_map[BT_AVRCP_CMD_TYPE_MAX] = {
        AVRC_ID_PLAY,
        AVRC_ID_PAUSE,
        AVRC_ID_FORWARD,
        AVRC_ID_BACKWARD,
        AVRC_ID_FAST_FOR,
        AVRC_ID_REWIND,
        AVRC_ID_STOP,
        AVRC_ID_VOL_UP,
        AVRC_ID_VOL_DOWN,
        AVRC_ID_CHAN_UP,
        AVRC_ID_CHAN_DOWN,
        AVRC_ID_MUTE,
        AVRC_ID_POWER };

    RawAddress *p_btaddr = NULL;
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    p_btaddr = &btaddr;

    linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s id=%d, is_press=%d",
        btmw_msg.data.avrcp_msg.addr, id, is_press);

    for (i = 0; i < BT_AVRCP_CMD_TYPE_MAX;i ++)
    {
        if (key_map[i] == id)
        {
            bt_cmd_type = (BT_AVRCP_CMD_TYPE)i;
            break;
        }
    }

    if (BT_AVRCP_CMD_TYPE_MAX == bt_cmd_type)
    {
        return;
    }

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    btmw_msg.hdr.event = BTMW_AVRCP_PASSTHROUGH_CMD_REQ;
    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.cmd_type = bt_cmd_type;

    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.action =
        is_press==0?BT_AVRCP_KEY_STATE_RELEASE:BT_AVRCP_KEY_STATE_PRESS;

    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static CHAR* linuxbt_rc_get_play_status_str(btrc_play_status_t status)
{
    switch((int)status)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BTRC_PLAYSTATE_STOPPED, "stopped");
        BT_MW_AVRCP_CASE_RETURN_STR(BTRC_PLAYSTATE_PLAYING, "playing");
        BT_MW_AVRCP_CASE_RETURN_STR(BTRC_PLAYSTATE_PAUSED, "paused");
        BT_MW_AVRCP_CASE_RETURN_STR(BTRC_PLAYSTATE_FWD_SEEK, "fwd seek");
        BT_MW_AVRCP_CASE_RETURN_STR(BTRC_PLAYSTATE_REV_SEEK, "rev seek");
        default: return (CHAR *)("unknown");
   }
}

