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


/* FILE NAME:  u_bt_mw_avrcp.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth avrcp structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_AVRCP_H_
#define _U_BT_MW_AVRCP_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_bt_mw_common.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define BT_AVRCP_UID_SIZE           (8)
#define BT_AVRCP_MAX_APP_SETTINGS   (8)
#define BT_AVRCP_MAX_APP_ATTR_SIZE  (16)
#define BT_AVRCP_MAX_APP_ATTR_STRLEN (32)

#define BT_AVRCP_MAX_ATTR_CNT       (7)
#define BT_AVRCP_MAX_NAME_LEN       (255)
#define BT_AVRCP_MAX_NUM_MEDIA_LIST  (100)
#define BTRC_MW_MAX_ATTR_STR_LEN        (255)
#define BTRC_MW_FEATURE_BIT_MASK_SIZE   (16)
#define MAX_COUNT_FOLDER_ITEM        (1)
#define BT_AVRCP_MAX_DEVICES  (3)

typedef enum
{
  BT_MW_AVRCP_ROLE_CT,
  BT_MW_AVRCP_ROLE_TG,
  BT_MW_AVRCP_ROLE_MAX
} BT_MW_AVRCP_ROLE;

typedef enum
{
    AVRCP_PLAY_STATUS_STOPPED = 0x00,       /* Stopped */
    AVRCP_PLAY_STATUS_PLAYING = 0x01,       /* Playing */
    AVRCP_PLAY_STATUS_PAUSED = 0x02,        /* Paused  */
    AVRCP_PLAY_STATUS_FORWARDSEEK= 0x03,    /* Fwd Seek*/
    AVRCP_PLAY_STATUS_REWINDSEEK= 0x04,     /* Rev Seek*/
    AVRCP_PLAY_STATUS_MAX = 0xFF,           /* Error   */
}BT_AVRCP_PLAY_STATUS;

typedef enum
{
    BT_AVRCP_KEY_STATE_PRESS = 0,   /* key is press   */
    BT_AVRCP_KEY_STATE_RELEASE,     /* key is release */
    BT_AVRCP_KEY_STATE_AUTO,        /* key is press and release */
    BT_AVRCP_KEY_STATE_MAX
}BT_AVRCP_KEY_STATE;

typedef enum
{
    BT_AVRCP_CMD_TYPE_PLAY = 0,     /* play          */
    BT_AVRCP_CMD_TYPE_PAUSE,        /* pause         */
    BT_AVRCP_CMD_TYPE_FWD,          /* forward/next  */
    BT_AVRCP_CMD_TYPE_BWD,          /* backward      */
    BT_AVRCP_CMD_TYPE_FFWD,         /* fast forward  */
    BT_AVRCP_CMD_TYPE_RWD,          /* reward        */
    BT_AVRCP_CMD_TYPE_STOP,         /* stop          */
    BT_AVRCP_CMD_TYPE_VOL_UP,       /* volume up     */
    BT_AVRCP_CMD_TYPE_VOL_DOWN,     /* volume down   */
    BT_AVRCP_CMD_TYPE_CHN_UP,       /* channel up    */
    BT_AVRCP_CMD_TYPE_CHN_DOWN,     /* channel down  */
    BT_AVRCP_CMD_TYPE_MUTE,         /* mute          */
    BT_AVRCP_CMD_TYPE_POWER,        /* power         */
    BT_AVRCP_CMD_TYPE_MAX
} BT_AVRCP_CMD_TYPE;

typedef enum
{
    AVRCP_BATTERY_STATUS_NORMAL = 0x00,       /* Normal */
    AVRCP_BATTERY_STATUS_WARNING = 0x01,       /* Warning */
    AVRCP_BATTERY_STATUS_CRITICAL = 0x02,        /* Critical  */
    AVRCP_BATTERY_STATUS_EXTERNAL= 0x03,    /* External*/
    AVRCP_BATTERY_STATUS_FULL_CHARGE= 0x04,     /* Full_charge*/
    AVRCP_BATTERY_STATUS_MAX = 0x05,           /* Error   */
}BT_AVRCP_BATTERY_STATUS;

typedef enum
{
    BT_AVRCP_ROLE_CT,
    BT_AVRCP_ROLE_TG,
    BT_AVRCP_ROLE_MAX
} BT_AVRCP_ROLE;


typedef enum
{
    BT_AVRCP_EVENT_CONNECTED,               /* AVRCP connected: only addr param*/
    BT_AVRCP_EVENT_DISCONNECTED,            /* AVRCP disconnected: only addr param*/

    BT_AVRCP_EVENT_FEATURE,                 /* AVRCP feature */
    BT_AVRCP_EVENT_TRACK_CHANGE,            /* remote track change */
    BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE,   /* remote player setting change */
    BT_AVRCP_EVENT_PLAYER_SETTING_RSP,      /* remote player setting rsp */
    BT_AVRCP_EVENT_LIST_PLAYER_SETTING_RSP, /* list remote player setting rsp  */
    BT_AVRCP_EVENT_POS_CHANGE,              /* remote position change */
    BT_AVRCP_EVENT_PLAY_STATUS_CHANGE,      /* remote play status change */
    BT_AVRCP_EVENT_VOLUME_CHANGE,           /* remote absolute volume change */

    BT_AVRCP_EVENT_SET_VOLUME_REQ,          /* remote set local absolute volume */
    BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ,     /* remote send passthrough command */
    BT_AVRCP_EVENT_BATTERY_CHANGE,          /* remote battery status change */

    BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB,     /* get folder items cb*/
    BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB,       /* change path cb*/
    BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB,   /* set browsed player cb*/
    BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB,    /* set addressed player cb*/
    BT_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB,  /* addressed_player_changed cb*/
    BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB, /* now playing contents changed cb: only addr parameter*/
    BT_AVRCP_EVENT_ABS_SUPPORT,                     /* abs volume support */
    BT_AVRCP_EVENT_MAX
} BT_AVRCP_EVENT;

typedef enum
{
    BT_AVRCP_REG_EVT_TRACK_CHANGED,          /* track change */
    BT_AVRCP_REG_EVT_PLAY_POS_CHANGED,       /* position change */
    BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED,    /* play status change */
    BT_AVRCP_REG_EVT_TRACK_REACHED_END,      /* track reach end */
    BT_AVRCP_REG_EVT_TRACK_REACHED_START,    /* track reach start */
    BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED,   /* app setting change */
    BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED,   /* available player change */
    BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED,    /* addressed player change */
    BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED,     /* absolute volume change */
    BT_AVRCP_REG_EVT_MAX,
} BT_AVRCP_REG_EVT_ID;

typedef enum {
    BT_AVRCP_MEDIA_ATTR_TITLE = 0x01,       /* title */
    BT_AVRCP_MEDIA_ATTR_ARTIST = 0x02,      /* artist */
    BT_AVRCP_MEDIA_ATTR_ALBUM = 0x03,       /* album */
    BT_AVRCP_MEDIA_ATTR_TRACK_NUM = 0x04,   /* track number */
    BT_AVRCP_MEDIA_ATTR_NUM_TRACKS = 0x05,  /* number of the track */
    BT_AVRCP_MEDIA_ATTR_GENRE = 0x06,       /* genre */
    BT_AVRCP_MEDIA_ATTR_PLAYING_TIME = 0x07,/* play time */
} BT_AVRCP_MEDIA_ATTR;

/* player setting attribute */
typedef enum {
    BT_AVRCP_PLAYER_EQUAL_VAL_OFF = 0x01,   /* equalizer OFF */
    BT_AVRCP_PLAYER_EQUAL_VAL_ON = 0x02,    /* equalizer ON */
} BT_AVRCP_PLAYER_EQUAL_VAL;

typedef enum {
    BT_AVRCP_PLAYER_REPEAT_VAL_OFF = 0x01,      /* repeat mode OFF */
    BT_AVRCP_PLAYER_REPEAT_VAL_SINGLE = 0x02,   /* repeat mode Single */
    BT_AVRCP_PLAYER_REPEAT_VAL_ALL = 0x03,      /* repeat mode ALL */
    BT_AVRCP_PLAYER_REPEAT_VAL_GROUP = 0x04     /* repeate mode GROUP */
} BT_AVRCP_PLAYER_REPEAT_VAL;

typedef enum {
    BT_AVRCP_PLAYER_SHUFFLE_VAL_OFF = 0x01, /* shuffle ON */
    BT_AVRCP_PLAYER_SHUFFLE_VAL_ALL = 0x02, /* shuffle OFF */
    BT_AVRCP_PLAYER_SHUFFLE_VAL_GROUP = 0x03 /* shuffle GROUP */
} BT_AVRCP_PLAYER_SHUFFLE_VAL;

typedef enum {
    BT_AVRCP_PLAYER_SCAN_VAL_OFF = 0x01,    /* scan OFF */
    BT_AVRCP_PLAYER_SCAN_VAL_ALL = 0x02,    /* scan ALL */
    BT_AVRCP_PLAYER_SCAN_VAL_GROUP = 0x03   /* scan GROUP */
} BT_AVRCP_PLAYER_SCAN_VAL;


typedef enum {
    BT_AVRCP_PLAYER_ATTR_EQUALIZER = 0x01,  /* equalizer */
    BT_AVRCP_PLAYER_ATTR_REPEAT = 0x02,     /* repeat mode */
    BT_AVRCP_PLAYER_ATTR_SHUFFLE = 0x03,    /* shuffle status */
    BT_AVRCP_PLAYER_ATTR_SCAN = 0x04,       /* scan status */
} BT_AVRCP_PLAYER_ATTR;

typedef enum {
    BT_AVRCP_SCOPE_MEDIA_PLAYER_LIST = 0x00,  /* media player list */
    BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM = 0x01,     /* player virtual filesystem */
    BT_AVRCP_SCOPE_SEARCH = 0x02,    /* search */
    BT_AVRCP_SCOPE_NOW_PLAYING = 0x03,       /* now playing */
} BT_AVRCP_SCOPE;

typedef enum {
    BT_AVRCP_FOLDER_UP = 0x00,  /* media player list */
    BT_AVRCP_FOLDER_DOWN = 0x01,     /* player virtual filesystem */
} BT_AVRCP_FOLDER_DIRECTION;


typedef struct
{
    CHAR media_id[BT_AVRCP_MAX_NAME_LEN];
    CHAR title[BT_AVRCP_MAX_NAME_LEN];
    CHAR artist[BT_AVRCP_MAX_NAME_LEN];
    CHAR album[BT_AVRCP_MAX_NAME_LEN];
    UINT32 current_track_number;
    UINT32 number_of_tracks;
    CHAR genre[BT_AVRCP_MAX_NAME_LEN];
    UINT32 position;
}BT_AVRCP_MEDIA_INFO;

typedef struct
{
    UINT8 num_attr;                         /* number of attribute in attrs */
    UINT8 attr_ids[BT_AVRCP_MAX_APP_SETTINGS];      /* attribute ID, BT_AVRCP_PLAYER_ATTR */
    UINT8 attr_values[BT_AVRCP_MAX_APP_SETTINGS];
} BT_AVRCP_PLAYER_SETTING;

typedef struct {
    UINT64 uid;
    UINT8 scope;                             /* scope:BT_AVRCP_SCOPE */
    UINT8 uid_counter;
} BT_AVRCP_PLAYITEM;

typedef struct {
    UINT32 start;
    UINT32 end;
} BT_AVRCP_LIST_RANGE;

typedef struct {
    UINT8   folder_direction;     /* value:BT_AVRCP_FOLDER_DIRECTION */
    UINT64 uid;
} BT_AVRCP_FOLDER_PATH;

typedef struct
{
    BT_AVRCP_PLAYER_SETTING player_setting;
}BT_AVRCP_PLAYERSETTING_CHANGE;

typedef struct
{
    UINT8 accepted;
}BT_AVRCP_PLAYERSETTING_RSP;

typedef struct {
    UINT8   val;
    UINT16  charset_id;
    UINT16  str_len;
    CHAR    p_str[BT_AVRCP_MAX_APP_ATTR_STRLEN];
} BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL;

typedef struct {
    UINT8   attr_id;
    UINT16  charset_id;
    UINT16  str_len;
    CHAR    p_str[BT_AVRCP_MAX_APP_ATTR_STRLEN];
    UINT8   num_val;
    BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL ext_attr_val[BT_AVRCP_MAX_APP_ATTR_SIZE];
} BT_AVRCP_PLAYER_APP_EXT_ATTR;

typedef struct {
    UINT8 attr_id;
    UINT8 num_val;
    UINT8 attr_val[BT_AVRCP_MAX_APP_ATTR_SIZE];
} BT_AVRCP_PLAYER_APP_ATTR;

typedef struct
{
    UINT8 num_attr;
    BT_AVRCP_PLAYER_APP_ATTR player_app_attr;
    UINT8 num_ext_attr;
    BT_AVRCP_PLAYER_APP_EXT_ATTR player_app_ext_attr;
}BT_AVRCP_LIST_PLAYERSETTING_RSP;

typedef struct {
    UINT16 player_id;
    UINT16 uid_counter;
} BT_AVRCP_ADDR_PLAYER;

typedef struct
{
    BOOL rc_connect;  /* rc connect */
    BOOL bt_connect ; /* browsed connect */
    BT_MW_AVRCP_ROLE role;
}BT_AVRCP_CONNECTION_CB;

typedef struct
{
    UINT8 abs_volume; /* unit: 1% */
    UINT8 ori_abs_volume ; /*0~127 */
}BT_AVRCP_SET_VOL_REQ;
typedef struct
{
    BT_AVRCP_MEDIA_INFO element_attr;
}BT_AVRCP_TRACK_CHANGE;

typedef struct
{
    UINT32 song_len; /* unit: ms */
    UINT32 song_pos; /* unit: ms */
}BT_AVRCP_POS_CHANGE;

typedef struct
{
    BT_AVRCP_PLAY_STATUS play_status;
}BT_AVRCP_PLAY_STATUS_CHANGE;

typedef struct
{
    BOOL supported;
}BT_AVRCP_ABS_SUPPORTED;

typedef struct
{
    UINT8 abs_volume; /* unit: 1% */
}BT_AVRCP_VOLUME_CHANGE;

typedef struct
{
    BT_AVRCP_CMD_TYPE cmd_type;
    BT_AVRCP_KEY_STATE action;
}BT_AVRCP_PASSTHROUGH_CMD_REQ;

typedef struct
{
    BT_AVRCP_BATTERY_STATUS battery_status;
}BT_AVRCP_BATTERY_STATUS_CHANGE;

typedef enum {
  BT_AVRCP_STS_BAD_CMD = 0x00,       /* Invalid command */
  BT_AVRCP_STS_BAD_PARAM = 0x01,     /* Invalid parameter */
  BT_AVRCP_STS_NOT_FOUND = 0x02,     /* Specified parameter is wrong or not found */
  BT_AVRCP_STS_INTERNAL_ERR = 0x03,  /* Internal Error */
  BT_AVRCP_STS_NO_ERROR = 0x04,      /* Operation Success */
  BT_AVRCP_STS_UID_CHANGED = 0x05,   /* UIDs changed */
  BT_AVRCP_STS_RESERVED = 0x06,      /* Reserved */
  BT_AVRCP_STS_INV_DIRN = 0x07,      /* Invalid direction */
  BT_AVRCP_STS_INV_DIRECTORY = 0x08, /* Invalid directory */
  BT_AVRCP_STS_INV_ITEM = 0x09,      /* Invalid Item */
  BT_AVRCP_STS_INV_SCOPE = 0x0a,     /* Invalid scope */
  BT_AVRCP_STS_INV_RANGE = 0x0b,     /* Invalid range */
  BT_AVRCP_STS_DIRECTORY = 0x0c,     /* UID is a directory */
  BT_AVRCP_STS_MEDIA_IN_USE = 0x0d,  /* Media in use */
  BT_AVRCP_STS_PLAY_LIST_FULL = 0x0e, /* Playing list full */
  BT_AVRCP_STS_SRCH_NOT_SPRTD = 0x0f, /* Search not supported */
  BT_AVRCP_STS_SRCH_IN_PROG = 0x10,   /* Search in progress */
  BT_AVRCP_STS_INV_PLAYER = 0x11,     /* Invalid player */
  BT_AVRCP_STS_PLAY_NOT_BROW = 0x12,  /* Player not browsable */
  BT_AVRCP_STS_PLAY_NOT_ADDR = 0x13,  /* Player not addressed */
  BT_AVRCP_STS_INV_RESULTS = 0x14,    /* Invalid results */
  BT_AVRCP_STS_NO_AVBL_PLAY = 0x15,   /* No available players */
  BT_AVRCP_STS_ADDR_PLAY_CHGD = 0x16, /* Addressed player changed */
} BT_AVRCP_STATUS;

typedef struct
{
    UINT32 feature;
}BT_AVRCP_FEATURE_RSP;

typedef enum {
  BT_RC_FEAT_NONE = 0x00,            /* AVRCP 1.0 */
  BT_RC_FEAT_METADATA = 0x01,        /* AVRCP 1.3 */
  BT_RC_FEAT_ABSOLUTE_VOLUME = 0x02, /* Supports TG role and volume sync */
  BT_RC_FEAT_BROWSE = 0x04, /* AVRCP 1.4 and up, with Browsing support */
} BT_REMOTE_FEATURE;

typedef struct
{
    BT_AVRCP_ROLE role;
    UINT16  feature;             /* BT_REMOTE_FEATURE bitmap*/
}BT_AVRCP_FEATURE;

typedef enum {
    BT_AVRCP_ITEM_TYPE_PLAYER = 0x01,      /* media player */
    BT_AVRCP_ITEM_TYPE_FOLDER = 0x02,      /* folder */
    BT_AVRCP_ITEM_TYPE_MEDIA = 0x03,       /* media file */
} BT_AVRCP_ITEM_TYPE;

typedef struct {
  BT_AVRCP_MEDIA_ATTR attr_id;
  UINT8 text[BTRC_MW_MAX_ATTR_STR_LEN];
} BT_AVRCP_ELEMENT_ATTR_VAL;

typedef enum {
    BT_AVRCP_PLAYER_MJ_TYPE_AUDIO = 0x01,      /* audio */
    BT_AVRCP_PLAYER_MJ_TYPE_VIDEO = 0x02,      /* video */
    BT_AVRCP_PLAYER_MJ_TYPE_BC_AUDIO = 0x04,   /* broadcasting audio */
    BT_AVRCP_PLAYER_MJ_TYPE_BC_VIDEO = 0x08,   /* broadcasting video */
    BT_AVRCP_PLAYER_MJ_TYPE_INVALID = 0xFC,    /* invalid type */
} BT_AVRCP_ITEM_PLAYER_MJ_TYPE;

typedef enum {
    BT_AVRCP_PLAYER_SUB_TYPE_NONE = 0x00,       /* none */
    BT_AVRCP_PLAYER_SUB_TYPE_AUDIO_BOOK = 0x01, /* audio book */
    BT_AVRCP_PLAYER_SUB_TYPE_PODCAST = 0x02,    /* podcast */
    BT_AVRCP_PLAYER_SUB_TYPE_INVALID = 0xFC,    /* invalid type */
} BT_AVRCP_ITEM_PLAYER_SUB_TYPE;

typedef struct {
  UINT16 player_id;
  BT_AVRCP_ITEM_PLAYER_MJ_TYPE major_type;
  BT_AVRCP_ITEM_PLAYER_MJ_TYPE sub_type;
  BT_AVRCP_PLAY_STATUS play_status;
  UINT8 features[BTRC_MW_FEATURE_BIT_MASK_SIZE];
  UINT16 charset_id;
  UINT8 name[BTRC_MW_MAX_ATTR_STR_LEN];
} BT_AVRCP_ITEM_PLAYER;

typedef enum {
    BT_AVRCP_FOLDER_TYPE_MIXED =     0x00,
    BT_AVRCP_FOLDER_TYPE_TITLES =    0x01,
    BT_AVRCP_FOLDER_TYPE_ALBUMS =    0x02,
    BT_AVRCP_FOLDER_TYPE_ARTISTS =   0x03,
    BT_AVRCP_FOLDER_TYPE_GENRES =    0x04,
    BT_AVRCP_FOLDER_TYPE_PLAYLISTS = 0x05,
    BT_AVRCP_FOLDER_TYPE_YEARS =     0x06,
} BT_AVRCP_ITEM_FOLDER_TYPE;

typedef struct {
  UINT8 uid[BT_AVRCP_UID_SIZE];
  BT_AVRCP_ITEM_FOLDER_TYPE type;
  UINT8 playable;
  UINT16 charset_id;
  UINT8 name[BTRC_MW_MAX_ATTR_STR_LEN];
} BT_AVRCP_ITEM_FOLDER;

typedef enum {
    BT_AVRCP_MEDIA_TYPE_AUDIO = 0x00,      /* audio */
    BT_AVRCP_MEDIA_TYPE_VIDEO = 0x01,      /* video */
} BT_AVRCP_ITEM_MEDIA_TYPE;

typedef struct {
  UINT8 uid[BT_AVRCP_UID_SIZE];
  BT_AVRCP_ITEM_MEDIA_TYPE type;
  UINT16 charset_id;
  UINT8 name[BTRC_MW_MAX_ATTR_STR_LEN];
  INT32 num_attrs;
  BT_AVRCP_ELEMENT_ATTR_VAL p_attrs[BT_AVRCP_MAX_ATTR_CNT];
} BT_AVRCP_ITEM_MEDIA;

typedef struct {
  BT_AVRCP_ITEM_TYPE item_type;
  union {
    BT_AVRCP_ITEM_PLAYER player;
    BT_AVRCP_ITEM_FOLDER folder;
    BT_AVRCP_ITEM_MEDIA media;
  }item;
} BT_AVRCP_FOLDER_ITEMS;

typedef struct
{
    BT_AVRCP_STATUS status;
    UINT8 count;
    BT_AVRCP_FOLDER_ITEMS folder_items[MAX_COUNT_FOLDER_ITEM];
}BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA;

typedef struct
{
    UINT32 count;
}BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA;

typedef struct
{
    UINT32 num_items;
    UINT32 depth;
}BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA;

 typedef struct
{
    BT_AVRCP_STATUS status;
}BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA;

 typedef struct
{
    UINT32 id;
}BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA;

typedef struct
{
    BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA get_itmes_cb;
}BT_AVRCP_GET_FLODER_ITEMS_CB;

typedef struct
{
    BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA change_folder_path_cb_data;
}BT_AVRCP_CHANGE_FOLDER_PATH_CB;

typedef struct
{
    BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA set_browsed_player_cb_data;
}BT_AVRCP_SET_BROWSED_PLAYER_CB;

typedef struct
{
    BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA set_addressed_player_cb_data;
}BT_AVRCP_SET_ADDRESSED_PLAYER_CB;

typedef struct
{
    BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA addressed_player_changed_cb_data;
}BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB;

typedef union
{
    /* CT field */
    BT_AVRCP_CONNECTION_CB connection_cb;
    BT_AVRCP_TRACK_CHANGE track_change;
    BT_AVRCP_PLAYERSETTING_CHANGE player_setting_change;
    BT_AVRCP_PLAYERSETTING_RSP player_setting_rsp;
    BT_AVRCP_LIST_PLAYERSETTING_RSP list_player_setting_rsp;
    BT_AVRCP_POS_CHANGE pos_change;
    BT_AVRCP_PLAY_STATUS_CHANGE play_status_change;
    BT_AVRCP_VOLUME_CHANGE volume_change;
    BT_AVRCP_FEATURE_RSP feature_rsp;
    BT_AVRCP_FEATURE feature;
    /* TG field */
    BT_AVRCP_PASSTHROUGH_CMD_REQ passthrough_cmd_req;
    BT_AVRCP_SET_VOL_REQ set_vol_req;
    BT_AVRCP_BATTERY_STATUS_CHANGE battery_change;
    BT_AVRCP_ABS_SUPPORTED abs_supported;
    /* add for browse*/
    BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA get_folder_items;
    BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA change_folder_path;
    BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA set_browsed_player;
    BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA set_addressed_player;
    BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA addressed_player_changed;
}BT_AVRCP_EVENT_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_AVRCP_EVENT event;
    BT_AVRCP_EVENT_DATA data;
}BT_AVRCP_EVENT_PARAM;

typedef void (*BT_AVRCP_EVENT_HANDLE_CB)(BT_AVRCP_EVENT_PARAM *param);

typedef struct
{
    UINT32 song_pos;    /* unit: ms */
    UINT32 song_len;    /* unit: ms */
    BT_AVRCP_PLAY_STATUS play_status;
} BT_AVRCP_PLAYER_STATUS;

typedef struct
{
    BT_AVRCP_MEDIA_INFO media_info; /* song media information */
    UINT8 track[BT_AVRCP_UID_SIZE]; /* all 0, song selected, all FF, no song selected */
}BT_AVRCP_PLAYER_MEDIA_INFO;

typedef struct
{
    CHAR curr_media_id[BT_AVRCP_MAX_NAME_LEN];
    UINT32 num_of_media_info;
    BT_AVRCP_PLAYER_MEDIA_INFO media_info[BT_AVRCP_MAX_NUM_MEDIA_LIST]; /* song media information */
}BT_AVRCP_PLAYER_MEDIA_INFO_LIST;

typedef enum
{
    BT_AVRCP_CONNECT_STATUS_DISCONNECTED,
    BT_AVRCP_CONNECT_STATUS_CONNECTED,
    BT_AVRCP_CONNECT_STATUS_MAX
} BT_AVRCP_CONNECT_STATUS;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_AVRCP_ROLE role;
    BT_AVRCP_CONNECT_STATUS conn_status;
    UINT32 ct_feature; /* local is CT and remote support feature */
    UINT32 tg_feature; /* local is TG and remote support feature */

    /* for sqc check */
    BT_AVRCP_MEDIA_INFO element_attr;
} BT_AVRCP_CONNECTED_DEV_INFO;

typedef struct
{
    UINT32 dev_num;
    BT_AVRCP_CONNECTED_DEV_INFO avrcp_connected_dev_list[BT_AVRCP_MAX_DEVICES];
}BT_AVRCP_CONNECTED_DEV_INFO_LIST;
#endif /*  _U_BT_MW_AVRCP_H_ */

