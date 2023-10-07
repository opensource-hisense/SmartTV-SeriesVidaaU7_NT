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

/* FILE NAME:  bt_mw_avrcp.c
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
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "bt_mw_common.h"
#include "bt_mw_avrcp.h"
#include "c_mw_config.h"

#include "linuxbt_avrcp_ct_if.h"
#include "linuxbt_avrcp_tg_if.h"

#include "bt_mw_message_queue.h"

//need refactor
//#include "mtk_bt_rc.h"
/* NAMING CONSTANT DECLARATIONS
 */
#define BT_MW_AVRCP_MAX_DEVICES  (3)

typedef enum
{
    BT_MW_AVRCP_CONNECT_STATUS_DISCONNECTED,
    BT_MW_AVRCP_CONNECT_STATUS_CONNECTED,
    BT_MW_AVRCP_CONNECT_STATUS_MAX
} BT_MW_AVRCP_CONNECT_STATUS;

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_AVRCP_LOCK() do{                           \
    if(g_bt_mw_avrcp_cb.inited)                          \
        pthread_mutex_lock(&g_bt_mw_avrcp_cb.lock);      \
    } while(0)

#define BT_MW_AVRCP_UNLOCK() do{                         \
    if(g_bt_mw_avrcp_cb.inited)                          \
        pthread_mutex_unlock(&g_bt_mw_avrcp_cb.lock);    \
    } while(0)


#define BT_MW_AVRCP_IS_VALID_ADDR(addr) do{                                         \
        if ((NULL == addr) || (MAX_BDADDR_LEN - 1 != strlen(addr)))                 \
        {                                                                           \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid addr(%s)", addr==NULL?"NULL":addr);\
            return FALSE;                                                               \
        }                                                                           \
    }while(0)

#define BT_MW_AVRCP_IS_VALID_ADDR_RETURN(addr) do{                                      \
        if ((NULL == addr) || (MAX_BDADDR_LEN - 1 != strlen(addr)))                     \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid addr(%s)", addr==NULL?"NULL":addr);   \
            return;                                                                     \
        }                                                                               \
    }while(0)

#define BT_MW_AVRCP_IS_VALID_RETURN_VAL(addr, val) do{                                  \
        if ((NULL == addr) || (MAX_BDADDR_LEN - 1 != strlen(addr)))                     \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid addr(%s)", addr==NULL?"NULL":addr);   \
            return val;                                                                 \
        }                                                                               \
    }while(0)

#define BT_MW_AVRCP_REPORT_EVENT(param) bt_mw_avrcp_notify_app(&param)

#define BT_MW_AVRCP_CHECK_INITED_RETURN    do {              \
        if (FALSE == g_bt_mw_avrcp_cb.inited)                \
        {                                                    \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "avrcp not init");  \
            return;                                          \
        }                                                    \
    }while(0)

#define BT_MW_AVRCP_CHECK_INITED(ret)    do {                \
        if (FALSE == g_bt_mw_avrcp_cb.inited)                \
        {                                                    \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "avrcp not init");  \
            return ret;                                      \
        }                                                    \
    }while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_MW_AVRCP_ROLE role;
    BT_MW_AVRCP_CONNECT_STATUS conn_status;
    UINT32 ct_feature; /* local is CT and remote support feature */
    UINT32 tg_feature; /* local is TG and remote support feature */
    UINT32 reg_event;
    UINT8 abs_vol_lable;

    /* for sqc check */
    BT_AVRCP_MEDIA_INFO element_attr;

    BOOL in_use;
} BT_MW_AVRCP_DEV;


typedef struct
{
    BT_MW_AVRCP_DEV devices[BT_MW_AVRCP_MAX_DEVICES];
    BT_AVRCP_PLAYER_STATUS player_status;
    BT_AVRCP_PLAYER_MEDIA_INFO player_media_info;
    BT_AVRCP_PLAYER_SETTING player_setting;
    BT_AVRCP_ADDR_PLAYER addr_player;
    UINT8 abs_volume; /* unit: 1% */
    BOOL inited;
    pthread_mutex_t lock;
} BT_MW_AVRCP_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static INT32 bt_mw_avrcp_init(VOID);

static INT32 bt_mw_avrcp_deinit(VOID);

static BOOL bt_mw_avrcp_is_connected(CHAR *addr);

static INT32 bt_mw_avrcp_get_dev_index(CHAR *addr);

static INT32 bt_mw_avrcp_alloc_dev_index(CHAR *addr);

static INT32 bt_mw_avrcp_free_dev(INT32 index);

static VOID bt_mw_avrcp_handle_connect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role,
    BT_AVRCP_CONNECTION_CB *connection_cb);

static VOID bt_mw_avrcp_handle_disconnect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role,
    BT_AVRCP_CONNECTION_CB *connection_cb);

static VOID bt_mw_avrcp_handle_feature_cb(CHAR *addr,
    BT_MW_AVRCP_ROLE role, UINT32 feature);

static VOID bt_mw_avrcp_handle_set_volume_req(CHAR *addr,
    UINT8 label, UINT8 volume);


static VOID bt_mw_avrcp_handle_reg_event_req(CHAR *addr, UINT8 label,
    BT_MW_AVRCP_REG_EVENT_REQ *reg_event);

static VOID bt_mw_avrcp_handle_track_change(CHAR *addr,
    BT_AVRCP_MEDIA_INFO *media_info);

static VOID bt_mw_avrcp_handle_player_setting_change(CHAR *addr,
    BT_AVRCP_PLAYER_SETTING *player_setting);

static VOID bt_mw_avrcp_handle_player_setting_rsp(CHAR *addr,
    UINT8 accepted);

static VOID bt_mw_avrcp_handle_list_player_setting_rsp(CHAR *addr,
   BT_AVRCP_LIST_PLAYERSETTING_RSP *list_playersetting_rsp);

static VOID bt_mw_avrcp_handle_play_position_change(CHAR *addr,
    BT_AVRCP_POS_CHANGE *pos_change);

static VOID bt_mw_avrcp_handle_play_status_change(CHAR *addr,
    BT_AVRCP_PLAY_STATUS status);

static VOID bt_mw_avrcp_handle_get_folder_items_cb(CHAR *addr,
    BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA *get_folder_data);

static VOID bt_mw_avrcp_handle_change_folder_path_cb(CHAR *addr,
    BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA *change_folder_path_data);

static VOID bt_mw_avrcp_set_browsed_player_cb(CHAR *addr,
    BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA *set_browsed_player_data);

static VOID bt_mw_avrcp_set_addressed_player_cb(CHAR *addr,
    BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA *set_addressed_player_data);

static VOID bt_mw_avrcp_now_playing_contens_changed_cb(CHAR *addr);

static VOID bt_mw_avrcp_handle_passthrough_cmd_req(CHAR *addr,
    BT_AVRCP_PASSTHROUGH_CMD_REQ *key);

static VOID bt_mw_avrcp_handle_volume_change_cb(CHAR *addr, UINT8 volume);

static INT32 bt_mw_avrcp_volume_change(CHAR *addr, BOOL interim, UINT8 volume);

static VOID bt_mw_avrcp_handle_battery_change(CHAR *addr, BT_AVRCP_BATTERY_STATUS status);


static VOID bt_mw_avrcp_msg_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_avrcp_notify_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_avrcp_notify_app(BT_AVRCP_EVENT_PARAM *param);

static CHAR* bt_mw_avrcp_get_key_str(BT_AVRCP_CMD_TYPE cmd_type);

static CHAR* bt_mw_avrcp_get_key_state_str(BT_AVRCP_KEY_STATE key_state);

static CHAR* bt_mw_avrcp_get_reg_event_str(BT_AVRCP_REG_EVT_ID event);

static CHAR* bt_mw_avrcp_get_event_str(UINT32 event);

static CHAR* bt_mw_avrcp_get_app_event_str(UINT32 event);

static CHAR* bt_mw_avrcp_get_play_status_str(BT_AVRCP_PLAY_STATUS status);

static INT32 bt_mw_avrcp_get_connected_devices(BT_AVRCP_CONNECTED_DEV_INFO_LIST *dev_list,
    BT_MW_AVRCP_DEV *devices);


/* STATIC VARIABLE DECLARATIONS
 */
static BT_MW_AVRCP_CB g_bt_mw_avrcp_cb;
static BT_AVRCP_EVENT_HANDLE_CB g_bt_mw_avrcp_event_handle_cb = NULL;

/* EXPORTED SUBPROGRAM BODIES
 */

INT32 bt_mw_avrcp_register_callback(BT_AVRCP_EVENT_HANDLE_CB avrcp_handle)
{
    if (NULL != avrcp_handle)
    {
        bt_mw_avrcp_init();
    }
    else
    {
        bt_mw_avrcp_deinit();
    }
    g_bt_mw_avrcp_event_handle_cb = avrcp_handle;
    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_change_player_app_setting(CHAR *addr, BT_AVRCP_PLAYER_SETTING *player_setting)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_setting);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "num attirbute =%d",
            player_setting->num_attr);

    return linuxbt_rc_change_player_app_setting_handler(addr, player_setting);

}

INT32 bt_mw_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "%s cmd_type: %s, action: %s", addr,
        bt_mw_avrcp_get_key_str(cmd_type),
        bt_mw_avrcp_get_key_state_str(key_state));

    if ((BT_AVRCP_CMD_TYPE_VOL_UP == cmd_type)
        || (BT_AVRCP_CMD_TYPE_VOL_DOWN == cmd_type))
    {
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        return linuxbt_rc_tg_send_passthrough_cmd_handler(addr, cmd_type, key_state);
#endif
    }
    else
    {
        return linuxbt_rc_send_passthrough_cmd_handler(addr, cmd_type, key_state);
    }
    return BT_SUCCESS;

} /*bt_mw_avrcp_send_passthrough_cmd*/

INT32 bt_mw_avrcp_send_vendor_unique_cmd(CHAR *addr, UINT8 key,
    BT_AVRCP_KEY_STATE key_state)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_send_vendor_unique_cmd_handler(addr, key, key_state);
} /*bt_mw_avrcp_send_passthrough_cmd*/


INT32 bt_mw_avrcp_change_volume(CHAR *addr, UINT8 volume)
{
    INT32 idx = 0;

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_MW_AVRCP_UNLOCK();

    return linuxbt_rc_tg_set_abs_volume(addr, volume * 127 / 100);
}

INT32 bt_mw_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *play_status)
{
    INT32 ret = 0;
    bool play_status_change = FALSE;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, play_status);

    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"old pos=%d, len=%d, status=%d",
        g_bt_mw_avrcp_cb.player_status.song_pos,
        g_bt_mw_avrcp_cb.player_status.song_len,
        g_bt_mw_avrcp_cb.player_status.play_status);
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"new pos=%d, len=%d, status=%d",
        play_status->song_pos,
        play_status->song_len,
        play_status->play_status);

    if ((play_status->play_status != g_bt_mw_avrcp_cb.player_status.play_status)
        || (play_status->song_pos != g_bt_mw_avrcp_cb.player_status.song_pos))
    {
        play_status_change = TRUE;
        ret = linuxbt_rc_tg_send_play_status_change(play_status, play_status_change);
        if (ret != 0)
        {
            BT_DBG_WARNING(BT_DEBUG_AVRCP,"send_play_status_change fail");
            return ret;
        }
    }
    memcpy(&g_bt_mw_avrcp_cb.player_status,
        play_status, sizeof(g_bt_mw_avrcp_cb.player_status));

    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_get_playback_state(CHAR *addr)
{
    INT32 idx = 0;

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_MW_AVRCP_UNLOCK();

    return linuxbt_rc_send_get_playstatus_cmd_handler(addr);
}

INT32 bt_mw_avrcp_send_playitem(CHAR *addr, BT_AVRCP_PLAYITEM *playitem)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, playitem);
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_send_playitem_handler(addr, playitem);
}

INT32 bt_mw_avrcp_get_now_playing_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_get_now_playing_list_handler(addr, list_range);
}

INT32 bt_mw_avrcp_get_folder_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_get_folder_list_handler(addr, list_range);
}

INT32 bt_mw_avrcp_get_player_list(CHAR *addr, BT_AVRCP_LIST_RANGE *list_range)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, list_range);
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_get_player_list_handler(addr, list_range);
}

INT32 bt_mw_avrcp_change_folder_path(CHAR *addr, BT_AVRCP_FOLDER_PATH *folder_path)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, folder_path);
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_change_folder_path_handler(addr, folder_path);
}

INT32 bt_mw_avrcp_set_browsed_player(CHAR *addr, UINT16 playerId)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_set_browsed_player_handler(addr, playerId);
}

INT32 bt_mw_avrcp_set_addressed_player(CHAR *addr, UINT16 playerId)
{
     INT32 idx = 0;
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_set_addressed_player_handler(addr, playerId);
}

INT32 bt_mw_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info)
{
    INT32 ret = BT_SUCCESS;
    bool track_changed = FALSE;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_media_info);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "title = %s track=0x%x 0x%x",
            player_media_info->media_info.title,
            player_media_info->track[0],
            player_media_info->track[BT_AVRCP_UID_SIZE-1]);

    if (0 == memcmp(&g_bt_mw_avrcp_cb.player_media_info,
        player_media_info, sizeof(BT_AVRCP_PLAYER_MEDIA_INFO)))
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP,"player media info no change");
    }
    else
    {
        track_changed = TRUE;
        ret = linuxbt_rc_tg_send_media_change(player_media_info, track_changed);
    }

    memcpy(&g_bt_mw_avrcp_cb.player_media_info,
        player_media_info, sizeof(g_bt_mw_avrcp_cb.player_media_info));

    return ret;
}

INT32 bt_mw_avrcp_update_absolute_volume(const UINT8 abs_volume)
{
    INT32 i = 0;

    if (g_bt_mw_avrcp_cb.abs_volume == abs_volume)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP,"abs_volume no change");
        return BT_ERR_STATUS_UNHANDLED;
    }

    for(i=0;i<BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            if (g_bt_mw_avrcp_cb.devices[i].reg_event
                 & (1 << BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED))
            {
                bt_mw_avrcp_volume_change(g_bt_mw_avrcp_cb.devices[i].addr,
                    FALSE, abs_volume);
            }
        }
    }
    g_bt_mw_avrcp_cb.abs_volume = abs_volume;

    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_dump_info(VOID)
{
    INT32 dev_idx = 0;
    BT_MW_AVRCP_DEV *ptr_dev = NULL;
    BT_MW_AVRCP_CHECK_INITED(BT_ERR_STATUS_FAIL);

    bt_mw_dump_info_begin("bt_avrcp_dump.log");

    BT_MW_AVRCP_LOCK();
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "==========local info==========");

    BT_DBG_DUMP(BT_DEBUG_AVRCP, "play status: %s",
        bt_mw_avrcp_get_play_status_str(g_bt_mw_avrcp_cb.player_status.play_status));
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "song_pos: %u",
        g_bt_mw_avrcp_cb.player_status.song_pos);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "song_len: %u",
        g_bt_mw_avrcp_cb.player_status.song_len);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "abs_volume: %u ",
        g_bt_mw_avrcp_cb.abs_volume);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "------------------------------");
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "title: %s",
        g_bt_mw_avrcp_cb.player_media_info.media_info.title);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "artist: %s",
        g_bt_mw_avrcp_cb.player_media_info.media_info.artist);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "album: %s",
        g_bt_mw_avrcp_cb.player_media_info.media_info.album);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "current_track_number: %u",
        g_bt_mw_avrcp_cb.player_media_info.media_info.current_track_number);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "number_of_tracks: %u",
        g_bt_mw_avrcp_cb.player_media_info.media_info.number_of_tracks);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "genre: %s",
        g_bt_mw_avrcp_cb.player_media_info.media_info.genre);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "position: %u",
        g_bt_mw_avrcp_cb.player_media_info.media_info.position);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "track id[0]: 0x%02x",
        g_bt_mw_avrcp_cb.player_media_info.track[0]);
    BT_DBG_DUMP(BT_DEBUG_AVRCP, "------------------------------");

    for(dev_idx=0;dev_idx<BT_MW_AVRCP_MAX_DEVICES;dev_idx++)
    {
        ptr_dev = &g_bt_mw_avrcp_cb.devices[dev_idx];
        if (TRUE == ptr_dev->in_use)
        {
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "==========device[%d]==========", dev_idx);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "addr: %s", ptr_dev->addr);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "role: %d", ptr_dev->role);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "conn_status: %d", ptr_dev->conn_status);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "ct_feature: 0x%x", ptr_dev->ct_feature);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "tg_feature: 0x%x", ptr_dev->tg_feature);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "reg_event: 0x%x", ptr_dev->reg_event);

            BT_DBG_DUMP(BT_DEBUG_AVRCP, "------------------------------");
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "title: %s",
                ptr_dev->element_attr.title);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "artist: %s",
                ptr_dev->element_attr.artist);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "album: %s",
                ptr_dev->element_attr.album);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "current_track_number: %u",
                ptr_dev->element_attr.current_track_number);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "number_of_tracks: %u",
                ptr_dev->element_attr.number_of_tracks);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "genre: %s",
                ptr_dev->element_attr.genre);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "position: %u",
                ptr_dev->element_attr.position);
            BT_DBG_DUMP(BT_DEBUG_AVRCP, "------------------------------");
        }
    }
    BT_MW_AVRCP_UNLOCK();

    bt_mw_dump_info_end();

    return BT_SUCCESS;
}


/* LOCAL SUBPROGRAM BODIES
 */


INT32 bt_mw_avrcp_init(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    if (TRUE == g_bt_mw_avrcp_cb.inited)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP,"has inited");
        return BT_SUCCESS;
    }

    memset((void*)&g_bt_mw_avrcp_cb, 0, sizeof(g_bt_mw_avrcp_cb));


    pthread_mutex_init(&g_bt_mw_avrcp_cb.lock, NULL);

    linuxbt_hdl_register(BTWM_ID_AVRCP, bt_mw_avrcp_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_AVRCP, bt_mw_avrcp_notify_handle);

    g_bt_mw_avrcp_cb.inited = TRUE;

    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    linuxbt_hdl_register(BTWM_ID_AVRCP, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_AVRCP, NULL);
    if (FALSE == g_bt_mw_avrcp_cb.inited)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP,"has deinited");
        return BT_SUCCESS;
    }
    pthread_mutex_destroy(&g_bt_mw_avrcp_cb.lock);

    memset((void*)&g_bt_mw_avrcp_cb, 0, sizeof(g_bt_mw_avrcp_cb));

    return BT_SUCCESS;
}

static BOOL bt_mw_avrcp_is_connected(CHAR *addr)
{
    BOOL ret = FALSE;
    INT32 idx = 0;

    BT_MW_AVRCP_IS_VALID_ADDR(addr);
    BT_MW_AVRCP_CHECK_INITED(FALSE);

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,"%s not connected", addr);
        BT_MW_AVRCP_UNLOCK();
        return ret;
    }
    if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
    {
        return ret;
    }
    if (BT_MW_AVRCP_CONNECT_STATUS_CONNECTED ==
        g_bt_mw_avrcp_cb.devices[idx].conn_status)
    {
        ret = TRUE;
    }
    BT_MW_AVRCP_UNLOCK();
    return ret;
}

static INT32 bt_mw_avrcp_get_dev_index(CHAR *addr)
{
    int i = 0;
    BT_MW_AVRCP_IS_VALID_RETURN_VAL(addr, -1);

    for(i = 0;i < BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_AVRCP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_avrcp_cb.devices[i].addr, g_bt_mw_avrcp_cb.devices[i].in_use);
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use
            && 0 == strcasecmp(g_bt_mw_avrcp_cb.devices[i].addr, addr))
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "found at %d", i);
            return i;
        }
    }

    BT_DBG_WARNING(BT_DEBUG_AVRCP, "%s not found", addr);
    return -1;
}

static INT32 bt_mw_avrcp_alloc_dev_index(CHAR *addr)
{
    int i = 0;

    for(i = 0;i < BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_AVRCP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_avrcp_cb.devices[i].addr, g_bt_mw_avrcp_cb.devices[i].in_use);
        if (FALSE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "found free at %d", i);
            g_bt_mw_avrcp_cb.devices[i].in_use = TRUE;
            strncpy(g_bt_mw_avrcp_cb.devices[i].addr, addr, MAX_BDADDR_LEN - 1);
            g_bt_mw_avrcp_cb.devices[i].addr[MAX_BDADDR_LEN - 1] = '\0';

            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_AVRCP, "not free found");
    return -1;
}

static INT32 bt_mw_avrcp_free_dev(INT32 index)
{
    if (index >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid index %d", index);
        return -1;
    }

    memset((void*)&g_bt_mw_avrcp_cb.devices[index], 0, sizeof(BT_MW_AVRCP_DEV));

    BT_DBG_INFO(BT_DEBUG_AVRCP, "not free found");
    return -1;
}

static VOID bt_mw_avrcp_handle_connect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role, BT_AVRCP_CONNECTION_CB *connection_cb)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 != idx)
    {
        if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
        {
            BT_DBG_WARNING(BT_DEBUG_AVRCP, "%d is invalid idx", idx);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
        if (BT_MW_AVRCP_CONNECT_STATUS_CONNECTED == g_bt_mw_avrcp_cb.devices[idx].conn_status && 
            g_bt_mw_avrcp_cb.devices[idx].role == role)
        {
            BT_DBG_WARNING(BT_DEBUG_AVRCP, "%s has connected", addr);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
    }
    else
    {
        idx = bt_mw_avrcp_alloc_dev_index(addr);
        if (-1 == idx)
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "no device resource dev %s", addr);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
        if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
        {
            BT_DBG_WARNING(BT_DEBUG_AVRCP, "%d is invalid idx", idx);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
    }
    g_bt_mw_avrcp_cb.devices[idx].role = role;
    g_bt_mw_avrcp_cb.devices[idx].conn_status =
            BT_MW_AVRCP_CONNECT_STATUS_CONNECTED;

    BT_MW_AVRCP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CONNECTED;
    param.data.connection_cb.role = role;
    if (role == BT_MW_AVRCP_ROLE_CT &&  connection_cb != NULL)
    {
        param.data.connection_cb.rc_connect = connection_cb->rc_connect;
        param.data.connection_cb.bt_connect = connection_cb->bt_connect;
    }
    BT_MW_AVRCP_REPORT_EVENT(param);
}

static VOID bt_mw_avrcp_handle_disconnect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role, BT_AVRCP_CONNECTION_CB *connection_cb)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }

    bt_mw_avrcp_free_dev(idx);
    BT_MW_AVRCP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_DISCONNECTED;
    if (role == BT_MW_AVRCP_ROLE_CT &&  connection_cb != NULL)
    {
        param.data.connection_cb.rc_connect = connection_cb->rc_connect;
        param.data.connection_cb.bt_connect = connection_cb->bt_connect;
    }

    BT_MW_AVRCP_REPORT_EVENT(param);
}

static VOID bt_mw_avrcp_handle_feature_cb(CHAR *addr, BT_MW_AVRCP_ROLE role, UINT32 feature)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, role=%d, feature=0x%x", addr, role, feature);
    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }
    if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, "%d is invalid idx", idx);
        BT_MW_AVRCP_UNLOCK();
        return;
    }

    if (BT_MW_AVRCP_ROLE_CT == role)
    {
        g_bt_mw_avrcp_cb.devices[idx].ct_feature = feature;
    }
    else
    {
        g_bt_mw_avrcp_cb.devices[idx].tg_feature = feature;
    }

    BT_MW_AVRCP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_FEATURE;
    param.data.feature.role = role;
    param.data.feature.feature = feature;
    BT_MW_AVRCP_REPORT_EVENT(param);
}

static VOID bt_mw_avrcp_handle_set_volume_req(CHAR *addr, UINT8 label, UINT8 volume)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, label=%d, volume=0x%x", addr, label, volume);

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }

    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_SET_VOLUME_REQ;

    param.data.set_vol_req.abs_volume = volume * 100 / 127;
    param.data.set_vol_req.ori_abs_volume = volume ;  // 0~127
    BT_MW_AVRCP_REPORT_EVENT(param);

    linuxbt_rc_set_volume_rsp(addr, label, volume);
    return;
}

static VOID bt_mw_avrcp_reg_event_interim_rsp(CHAR *addr,
    UINT8 label, BT_MW_AVRCP_REG_EVENT_REQ *reg_event_req)
{
    BT_AVRCP_EVENT_CHANGE_RSP data;

    memset((void*)&data, 0, sizeof(data));

    switch (reg_event_req->event_id)
    {
        case BT_AVRCP_REG_EVT_TRACK_CHANGED:
            BT_DBG_NORMAL(BT_DEBUG_AVRCP, "title = %s track[0]=0x%x,track[0]=0x%x",
                g_bt_mw_avrcp_cb.player_media_info.media_info.title,
                g_bt_mw_avrcp_cb.player_media_info.track[0],
                g_bt_mw_avrcp_cb.player_media_info.track[BT_AVRCP_UID_SIZE-1]);
            memcpy(&data.track_change.track_uid,
                g_bt_mw_avrcp_cb.player_media_info.track,
                sizeof(data.track_change.track_uid));
            break;
        case BT_AVRCP_REG_EVT_PLAY_POS_CHANGED:
            data.pos_change.song_pos = g_bt_mw_avrcp_cb.player_status.song_pos;
            break;
        case BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED:
            data.play_status_change.play_status =
                g_bt_mw_avrcp_cb.player_status.play_status;
            break;
        case BT_AVRCP_REG_EVT_TRACK_REACHED_END:
            break;
        case BT_AVRCP_REG_EVT_TRACK_REACHED_START:
            break;
        case BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED:
            data.play_setting_change.setting = g_bt_mw_avrcp_cb.player_setting;
            break;
        case BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED:
            break;
        case BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED:
            data.addr_player_change.player = g_bt_mw_avrcp_cb.addr_player;
            break;
        case BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED:
            bt_mw_avrcp_volume_change(addr, TRUE, g_bt_mw_avrcp_cb.abs_volume);
            return;
        default:
            return;
    }
    return;
}

static VOID bt_mw_avrcp_handle_reg_event_req(CHAR *addr,
    UINT8 label, BT_MW_AVRCP_REG_EVENT_REQ *reg_event)
{
    INT32 idx = 0;
    BT_MW_AVRCP_IS_VALID_ADDR_RETURN(addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, reg_event);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "event=%s(%d)",
        bt_mw_avrcp_get_reg_event_str(reg_event->event_id),
        reg_event->event_id);

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }
    if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid idx  %d", idx);
        return;
    }
    if (BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED == reg_event->event_id)
    {
        g_bt_mw_avrcp_cb.devices[idx].abs_vol_lable = label;
    }

    g_bt_mw_avrcp_cb.devices[idx].reg_event |= (1 << reg_event->event_id);
    BT_MW_AVRCP_UNLOCK();
    bt_mw_avrcp_reg_event_interim_rsp(addr, label, reg_event);

    return;
}

static VOID bt_mw_avrcp_handle_track_change(CHAR *addr, BT_AVRCP_MEDIA_INFO *media_info)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_TRACK_CHANGE;

    memcpy(&param.data.track_change.element_attr,
        media_info, sizeof(BT_AVRCP_MEDIA_INFO));

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }
    if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid idx  %d", idx);
        return;
    }
    memcpy(&g_bt_mw_avrcp_cb.devices[idx].element_attr,
            media_info, sizeof(BT_AVRCP_MEDIA_INFO));
    BT_MW_AVRCP_UNLOCK();

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_battery_change(CHAR *addr, BT_AVRCP_BATTERY_STATUS status)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_BATTERY_CHANGE;
    param.data.battery_change.battery_status = status;


    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }
    BT_MW_AVRCP_UNLOCK();

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_abs_supported(CHAR *addr, BT_AVRCP_ABS_SUPPORTED *abs_support)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "up abs support");
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_ABS_SUPPORT;
    param.data.abs_supported.supported = abs_support->supported;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_player_setting_change(CHAR *addr, BT_AVRCP_PLAYER_SETTING *player_setting)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';;
    param.event = BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE;

    memcpy(&param.data.player_setting_change.player_setting,
        player_setting, sizeof(BT_AVRCP_PLAYER_SETTING));

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_player_setting_rsp(CHAR *addr, UINT8 accepted)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_PLAYER_SETTING_RSP;

    param.data.player_setting_rsp.accepted = accepted;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}


static VOID bt_mw_avrcp_handle_list_player_setting_rsp(CHAR *addr,
    BT_AVRCP_LIST_PLAYERSETTING_RSP *list_playersetting_rsp)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_LIST_PLAYER_SETTING_RSP;

    memcpy(&param.data.list_player_setting_rsp, list_playersetting_rsp, sizeof(BT_AVRCP_LIST_PLAYERSETTING_RSP));

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}


static VOID bt_mw_avrcp_handle_play_position_change(CHAR *addr, BT_AVRCP_POS_CHANGE *pos_change)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_POS_CHANGE;

    memcpy(&param.data.pos_change, pos_change, sizeof(BT_AVRCP_POS_CHANGE));

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_play_status_change(CHAR *addr, BT_AVRCP_PLAY_STATUS status)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, status=%s", addr,
        bt_mw_avrcp_get_play_status_str(status));
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_PLAY_STATUS_CHANGE;
    param.data.play_status_change.play_status = status;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_get_folder_items_cb(CHAR *addr, BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA *get_folder_data)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s", addr);
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB;
    param.data.get_folder_items.count = get_folder_data->count;
    param.data.get_folder_items.status = get_folder_data->status;
    memcpy(param.data.get_folder_items.folder_items,
        get_folder_data->folder_items, sizeof(param.data.get_folder_items.folder_items));

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "param.data.get_folder_items.folder_items item_type=%d",
        param.data.get_folder_items.folder_items[0].item_type);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "param.data.get_folder_items.folder_items counter=%d",
        param.data.get_folder_items.count);

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_change_folder_path_cb(CHAR *addr,
    BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA *change_folder_path_data)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s count=%d", addr, change_folder_path_data->count);
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB;
    param.data.change_folder_path.count = change_folder_path_data->count;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_set_browsed_player_cb(CHAR *addr,
    BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA *set_browsed_player_data)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s num_items=%d, depth=%d", addr,
        set_browsed_player_data->num_items, set_browsed_player_data->depth);
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB;
    param.data.set_browsed_player.num_items = set_browsed_player_data->num_items;
    param.data.set_browsed_player.depth = set_browsed_player_data->depth;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_set_addressed_player_cb(CHAR *addr,
    BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA *set_addressed_player_data)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s status=%d", addr,
        set_addressed_player_data->status);
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB;
    param.data.set_addressed_player.status = set_addressed_player_data->status;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_now_playing_contens_changed_cb(CHAR *addr)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s", addr);
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.event = BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_passthrough_cmd_req(CHAR *addr, BT_AVRCP_PASSTHROUGH_CMD_REQ *key)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, key=%s, action=%s", addr,
        bt_mw_avrcp_get_key_str(key->cmd_type),
        bt_mw_avrcp_get_key_state_str(key->action));

    param.event = BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ;
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';
    param.data.passthrough_cmd_req.cmd_type = key->cmd_type;
    param.data.passthrough_cmd_req.action = key->action;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_volume_change_cb(CHAR *addr, UINT8 volume)
{
    BT_AVRCP_EVENT_PARAM param;
    memset(&param, 0, sizeof(BT_AVRCP_EVENT_PARAM));
    param.event = BT_AVRCP_EVENT_VOLUME_CHANGE;
    strncpy(param.addr, addr, MAX_BDADDR_LEN);
    param.addr[MAX_BDADDR_LEN - 1] = '\0';

    param.data.volume_change.abs_volume = volume * 100 / 127;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static INT32 bt_mw_avrcp_volume_change(CHAR *addr, BOOL interim, UINT8 volume)
{
    INT32 idx = 0;
    INT8 lable = 0;
    UINT8 abs_volume = 0;

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, interim=%d, volume=%d",
            addr, interim, volume);
    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return BT_ERR_STATUS_NOT_READY;
    }
    if (idx < 0 || idx >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid idx  %d", idx);
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_MW_AVRCP_UNLOCK();
    lable = g_bt_mw_avrcp_cb.devices[idx].abs_vol_lable;
    abs_volume = volume * 127 / 100;
    return linuxbt_rc_send_volume_change_rsp_handler(addr, interim, lable, abs_volume);
}

static VOID bt_mw_avrcp_msg_handle(tBTMW_MSG *p_msg)
{
    tBT_MW_AVRCP_MSG *avrcp_msg = &p_msg->data.avrcp_msg;
    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "event:%s(%d), addr:%s",
        bt_mw_avrcp_get_event_str(p_msg->hdr.event),
        p_msg->hdr.event, avrcp_msg->addr);
    BT_MW_AVRCP_CHECK_INITED();
    BT_AVRCP_CONNECTION_CB *connection_cb = NULL;

    switch(p_msg->hdr.event)
    {
        case BTMW_AVRCP_CONNECTED:
            if (avrcp_msg->role == BT_MW_AVRCP_ROLE_CT)
            {
                connection_cb = &avrcp_msg->data.connection_cb;
            }
            bt_mw_avrcp_handle_connect_cb(avrcp_msg->addr, avrcp_msg->role, connection_cb);
            break;
        case BTMW_AVRCP_DISCONNECTED:
            if (avrcp_msg->role == BT_MW_AVRCP_ROLE_CT)
            {
                connection_cb = &avrcp_msg->data.connection_cb;
            }
            bt_mw_avrcp_handle_disconnect_cb(avrcp_msg->addr, avrcp_msg->role, connection_cb);
            break;
        case BTMW_AVRCP_FEATURE:
            bt_mw_avrcp_handle_feature_cb(avrcp_msg->addr,
                avrcp_msg->role, avrcp_msg->data.feature);
            break;
        case BTMW_AVRCP_SET_VOLUME_REQ:
            bt_mw_avrcp_handle_set_volume_req(avrcp_msg->addr,
                avrcp_msg->label, avrcp_msg->data.set_vol_req.abs_volume);
            break;
        case BTMW_AVRCP_REG_EVENT_REQ:
            bt_mw_avrcp_handle_reg_event_req(avrcp_msg->addr,
                avrcp_msg->label, &avrcp_msg->data.reg_event_req);
            break;
        case BTMW_AVRCP_TRACK_CHANGE:
            bt_mw_avrcp_handle_track_change(avrcp_msg->addr,
                &avrcp_msg->data.track_change.element_attr);
            break;
        case BTMW_AVRCP_PLAYER_SETTING_CHANGE:
            bt_mw_avrcp_handle_player_setting_change(avrcp_msg->addr,
               &avrcp_msg->data.playersetting_change.player_setting);
            break;
        case BTMW_AVRCP_PLAYER_SETTING_RSP:
            bt_mw_avrcp_handle_player_setting_rsp(avrcp_msg->addr,
                avrcp_msg->data.playersetting_rsp.accepted);
            break;
        case BTMW_AVRCP_LIST_PLAYER_SETTING_RSP:
            bt_mw_avrcp_handle_list_player_setting_rsp(avrcp_msg->addr,
                &avrcp_msg->data.list_playersetting_rsp);
            break;
        case BTMW_AVRCP_PLAY_POS_CHANGE:
            bt_mw_avrcp_handle_play_position_change(avrcp_msg->addr,
                &avrcp_msg->data.pos_change);
            break;
        case BTMW_AVRCP_PLAY_STATUS_CHANGE:
            bt_mw_avrcp_handle_play_status_change(avrcp_msg->addr,
                avrcp_msg->data.play_status_change.play_status);
            break;
        case BTMW_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB:
            bt_mw_avrcp_handle_get_folder_items_cb(avrcp_msg->addr,
                &avrcp_msg->data.get_folder_itmes_cb.get_itmes_cb);
            break;
        case BTMW_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB:
            bt_mw_avrcp_handle_change_folder_path_cb(avrcp_msg->addr,
                &avrcp_msg->data.change_folder_path_cb.change_folder_path_cb_data);
            break;
        case BTMW_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB:
            bt_mw_avrcp_set_browsed_player_cb(avrcp_msg->addr,
                &avrcp_msg->data.set_browsed_player_cb.set_browsed_player_cb_data);
            break;
        case BTMW_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB:
            bt_mw_avrcp_set_addressed_player_cb(avrcp_msg->addr,
                &avrcp_msg->data.set_addressed_player_cb.set_addressed_player_cb_data);
            break;
        case BTMW_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB:
            BT_DBG_WARNING(BT_DEBUG_AVRCP, "No support for event:%s(%d), addr:%s",
                bt_mw_avrcp_get_event_str(p_msg->hdr.event),
                p_msg->hdr.event, avrcp_msg->addr);
            break;
        case BTMW_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB:
            bt_mw_avrcp_now_playing_contens_changed_cb(avrcp_msg->addr);
            break;
        case BTMW_AVRCP_PASSTHROUGH_CMD_REQ:
            bt_mw_avrcp_handle_passthrough_cmd_req(avrcp_msg->addr,
                &avrcp_msg->data.passthrough_cmd_req);
            break;
        case BTMW_AVRCP_VOLUME_CHANGE:
            bt_mw_avrcp_handle_volume_change_cb(avrcp_msg->addr,
                avrcp_msg->data.volume_change.abs_volume);
            break;
        case BTMW_AVRCP_BATTERY_CHANGE:
            bt_mw_avrcp_handle_battery_change(avrcp_msg->addr,
            avrcp_msg->data.battery_change.battery_status);
            break;
        case BTMW_AVRCP_ABS_SUPPORTED:
            bt_mw_avrcp_abs_supported(avrcp_msg->addr,
            &avrcp_msg->data.abs_supported);
            break;
        default:
            break;
    }
}

static VOID bt_mw_avrcp_notify_handle(tBTMW_MSG *p_msg)
{
    BT_AVRCP_EVENT_PARAM *avrcp_event = &p_msg->data.avrcp_event;
    BT_DBG_INFO(BT_DEBUG_AVRCP, "event:%d, addr:%s", p_msg->hdr.event,
        p_msg->data.avrcp_event.addr);
    BT_MW_AVRCP_CHECK_INITED_RETURN;

    switch(p_msg->hdr.event)
    {
        case BTMW_AVRCP_NOTIFY_APP:
            if(g_bt_mw_avrcp_event_handle_cb)
            {
                BT_DBG_NORMAL(BT_DEBUG_AVRCP, "report event: %s(%d)",
                    bt_mw_avrcp_get_app_event_str(avrcp_event->event),
                    avrcp_event->event);
                g_bt_mw_avrcp_event_handle_cb(avrcp_event);
            }
            break;
        default:
            break;
    }
}

static VOID bt_mw_avrcp_notify_app(BT_AVRCP_EVENT_PARAM *param)
{
    tBTMW_MSG msg;
    msg.hdr.event = BTMW_AVRCP_NOTIFY_APP;
    msg.hdr.len = sizeof(*param);
    memcpy((void*)&msg.data.avrcp_event, param, sizeof(*param));
    bt_mw_nty_send_msg(&msg);

}

static CHAR* bt_mw_avrcp_get_key_str(BT_AVRCP_CMD_TYPE cmd_type)
{
    switch((int)cmd_type)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_PLAY, "play");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_PAUSE, "pause");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_FWD, "next");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_BWD, "prev");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_FFWD, "fast forward");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_RWD, "reward");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_STOP, "stop");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_VOL_UP, "vol up");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_VOL_DOWN, "vol down");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_CHN_UP, "channel up");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_CHN_DOWN, "channel down");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_MUTE, "mute");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_CMD_TYPE_POWER, "power");
        default: return "unknown";
   }
}

static CHAR* bt_mw_avrcp_get_key_state_str(BT_AVRCP_KEY_STATE key_state)
{
    switch((int)key_state)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_KEY_STATE_PRESS, "press");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_KEY_STATE_RELEASE, "release");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_KEY_STATE_AUTO, "auto");
        default: return "unknown";
   }
}



static CHAR* bt_mw_avrcp_get_reg_event_str(BT_AVRCP_REG_EVT_ID event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_CHANGED, "track_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_PLAY_POS_CHANGED, "pos_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED, "status_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_REACHED_END, "track_reach_end");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_REACHED_START, "track_reach_start");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED, "app_setting_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED, "avail_player_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED, "addr_player_changed");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED, "abs_vol_changed");
        default: return "unknown";
   }
}

static CHAR* bt_mw_avrcp_get_event_str(UINT32 event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_CONNECTED, "connected");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_DISCONNECTED, "disconnected");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_FEATURE, "feature");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_REG_EVENT_RSP, "reg_event_rsp");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_TRACK_CHANGE, "track_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAYER_SETTING_CHANGE, "player_setting_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAYER_SETTING_RSP, "player_setting_rsp");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_LIST_PLAYER_SETTING_RSP, "list_player_setting_rsp");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAY_POS_CHANGE, "pos_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB, "get_folder_items_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB, "change_folder_path_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB, "set_browsed_player_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB, "set_ddressed_player_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB, "addressed_player_change_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB, "now_playing_cotents_changed_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAY_STATUS_CHANGE, "play_status_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_VOLUME_CHANGE, "vol_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_SET_VOLUME_REQ, "set_vol_req");

        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_REG_EVENT_REQ, "reg_event_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_GET_PLAYSTATUS_REQ, "get_play_status_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_GET_ELEMENT_ATTR_REQ, "get_element_attr_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PASSTHROUGH_CMD_REQ, "passthrough_cmd_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_ABS_SUPPORTED, "abs support");
        default: return "unknown";
   }
}

static CHAR* bt_mw_avrcp_get_app_event_str(UINT32 event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CONNECTED, "connected");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_DISCONNECTED, "disconnected");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_TRACK_CHANGE, "track_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE, "player_setting_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAYER_SETTING_RSP, "player_setting_rsp");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_LIST_PLAYER_SETTING_RSP, "player_setting_rsp");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_POS_CHANGE, "pos_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB, "get_folder_items_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB, "change_folder_pat_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB, "set_browsed_player_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB, "set_addressed_player_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB, "addressed_player_changed_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB, "now_playing_contents_changed_cb");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAY_STATUS_CHANGE, "play_status_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_VOLUME_CHANGE, "vol_change");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_SET_VOLUME_REQ, "set_vol_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ, "passthrough_cmd_req");
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_FEATURE, "feature");
        default: return "unknown";
   }
}

static CHAR* bt_mw_avrcp_get_play_status_str(BT_AVRCP_PLAY_STATUS status)
{
    switch((int)status)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(AVRCP_PLAY_STATUS_STOPPED, "stopped");
        BT_MW_AVRCP_CASE_RETURN_STR(AVRCP_PLAY_STATUS_PLAYING, "playing");
        BT_MW_AVRCP_CASE_RETURN_STR(AVRCP_PLAY_STATUS_PAUSED, "paused");
        BT_MW_AVRCP_CASE_RETURN_STR(AVRCP_PLAY_STATUS_FORWARDSEEK, "fwd seek");
        BT_MW_AVRCP_CASE_RETURN_STR(AVRCP_PLAY_STATUS_REWINDSEEK, "rev seek");
        default: return "unknown";
   }
}

INT32 bt_mw_avrcp_get_connected_dev_list(BT_AVRCP_CONNECTED_DEV_INFO_LIST *dev_list)
{
    INT32 ret = BT_SUCCESS;

    BT_MW_AVRCP_LOCK();
    ret = bt_mw_avrcp_get_connected_devices(dev_list, g_bt_mw_avrcp_cb.devices);
    BT_MW_AVRCP_UNLOCK();
    return ret;
}

static INT32 bt_mw_avrcp_get_connected_devices(BT_AVRCP_CONNECTED_DEV_INFO_LIST *dev_list,
    BT_MW_AVRCP_DEV *devices)
{
    UINT32 dev_num = 0;
    UINT32 i = 0;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, dev_list);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, devices);
    BT_MW_AVRCP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    for (i = 0; i < BT_MW_AVRCP_MAX_DEVICES; i++)
    {
        if(devices[i].in_use && devices[i].conn_status == BT_MW_AVRCP_CONNECT_STATUS_CONNECTED)
        {
            dev_num++;
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "dev %d addr %s", i, devices[i].addr);
            memcpy(dev_list->avrcp_connected_dev_list[i].addr, devices[i].addr, MAX_BDADDR_LEN);
            dev_list->avrcp_connected_dev_list[i].conn_status
                = devices[i].conn_status;

            dev_list->avrcp_connected_dev_list[i].ct_feature
                = devices[i].ct_feature;
            dev_list->avrcp_connected_dev_list[i].tg_feature
                = devices[i].tg_feature;
            memcpy(&dev_list->avrcp_connected_dev_list[i].element_attr,
            &devices[i].element_attr, sizeof(BT_AVRCP_MEDIA_INFO));
        }
    }
    dev_list->dev_num = dev_num;
    return BT_SUCCESS;
}

