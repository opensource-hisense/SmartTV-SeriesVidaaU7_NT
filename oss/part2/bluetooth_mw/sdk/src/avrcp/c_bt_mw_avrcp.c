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
#include <fcntl.h>
#include <unistd.h>

//#include "bluetooth.h" //need refactor

#include "c_bt_mw_avrcp.h"
#include "bt_mw_common.h"
#include "bt_mw_avrcp.h"



/* CT interface */
EXPORT_SYMBOL INT32 c_btm_avrcp_change_player_app_setting(CHAR *addr, BT_AVRCP_PLAYER_SETTING *player_setting)
{
   BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
   BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_setting);
   return bt_mw_avrcp_change_player_app_setting(addr, player_setting);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    //BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, cmd_type=%d, key_state=%d",
    //    addr, cmd_type, key_state);
    return bt_mw_avrcp_send_passthrough_cmd(addr, cmd_type, key_state);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_send_vendor_unique_cmd(CHAR *addr,
    UINT8 key, BT_AVRCP_KEY_STATE key_state)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, key=%d, key_state=%d",
        addr, key, key_state);
    return bt_mw_avrcp_send_vendor_unique_cmd(addr, key, key_state);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_change_volume(CHAR *addr, UINT8 volume)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, volume=%d", addr, volume);
    return bt_mw_avrcp_change_volume(addr, volume);
}

/* TG interface */
EXPORT_SYMBOL
INT32 c_btm_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *player_status)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_status);
    return bt_mw_avrcp_update_player_status(player_status);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_get_playback_state(CHAR *addr)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s", addr);
    return bt_mw_avrcp_get_playback_state(addr);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_send_playitem(CHAR *addr, BT_AVRCP_PLAYITEM *playitem)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, playitem);
    return bt_mw_avrcp_send_playitem(addr, playitem);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_get_now_playing_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    return bt_mw_avrcp_get_now_playing_list(addr, list_range);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_get_folder_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    return bt_mw_avrcp_get_folder_list(addr, list_range);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_get_player_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    return bt_mw_avrcp_get_player_list(addr, list_range);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_change_folder_path(CHAR *addr,
    BT_AVRCP_FOLDER_PATH *folder_path)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, folder_path);
    return bt_mw_avrcp_change_folder_path(addr, folder_path);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_set_browsed_player(CHAR *addr, UINT16 playerId)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    return bt_mw_avrcp_set_browsed_player(addr, playerId);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_set_addressed_player(CHAR *addr, UINT16 playerId)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    return bt_mw_avrcp_set_addressed_player(addr, playerId);
}

EXPORT_SYMBOL
INT32 c_btm_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_media_info);
    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "title =%s ", player_media_info->media_info.title);
    return bt_mw_avrcp_update_player_media_info(player_media_info);
}

EXPORT_SYMBOL
INT32 c_btm_avrcp_update_absolute_volume(const UINT8 abs_volume)
{

    return bt_mw_avrcp_update_absolute_volume(abs_volume);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_register_callback(BT_AVRCP_EVENT_HANDLE_CB avrcp_handle)
{
    return bt_mw_avrcp_register_callback(avrcp_handle);
}

EXPORT_SYMBOL INT32 c_btm_avrcp_get_connected_dev_list(BT_AVRCP_CONNECTED_DEV_INFO_LIST *dev_list)
{
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, dev_list);
    return bt_mw_avrcp_get_connected_dev_list(dev_list);
}

