/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Feb 15 17:47:52 2022'. */
/* Do NOT modify this source file. */



/* Start of source pre-amble file 'src_header_file.h'. */

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

#include "mtk_bt_service_avrcp_ipcrpc_struct.h"
#include "u_bt_mw_avrcp.h"


/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_MEDIA_INFO;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_SETTING;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYITEM;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_LIST_RANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FOLDER_PATH;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYERSETTING_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYERSETTING_RSP;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_ATTR;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_LIST_PLAYERSETTING_RSP;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDR_PLAYER;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTION_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_VOL_REQ;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_TRACK_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_POS_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAY_STATUS_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ABS_SUPPORTED;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_VOLUME_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PASSTHROUGH_CMD_REQ;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_BATTERY_STATUS_CHANGE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FEATURE_RSP;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FEATURE;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ELEMENT_ATTR_VAL;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_PLAYER;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_FOLDER;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_MEDIA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FOLDER_ITEMS;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_GET_FLODER_ITEMS_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_EVENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_STATUS;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO_LIST;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO;
static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO_LIST;



static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BTMW_LE_FEATURES),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_MEDIA_INFO =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_MEDIA_INFO),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_SETTING =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_SETTING),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYITEM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYITEM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_LIST_RANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_LIST_RANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FOLDER_PATH =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_FOLDER_PATH),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYERSETTING_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYERSETTING_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYERSETTING_RSP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYERSETTING_RSP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_APP_EXT_ATTR),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_APP_ATTR =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_APP_ATTR),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_LIST_PLAYERSETTING_RSP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_LIST_PLAYERSETTING_RSP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDR_PLAYER =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ADDR_PLAYER),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTION_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_CONNECTION_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_VOL_REQ =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_SET_VOL_REQ),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_TRACK_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_TRACK_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_POS_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_POS_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAY_STATUS_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAY_STATUS_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ABS_SUPPORTED =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ABS_SUPPORTED),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_VOLUME_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_VOLUME_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PASSTHROUGH_CMD_REQ =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PASSTHROUGH_CMD_REQ),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_BATTERY_STATUS_CHANGE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_BATTERY_STATUS_CHANGE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FEATURE_RSP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_FEATURE_RSP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FEATURE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_FEATURE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ELEMENT_ATTR_VAL =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ELEMENT_ATTR_VAL),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_PLAYER =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ITEM_PLAYER),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_FOLDER =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ITEM_FOLDER),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ITEM_MEDIA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ITEM_MEDIA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_FOLDER_ITEMS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_FOLDER_ITEMS),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_GET_FLODER_ITEMS_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_GET_FLODER_ITEMS_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_CHANGE_FOLDER_PATH_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_SET_BROWSED_PLAYER_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_SET_ADDRESSED_PLAYER_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_AVRCP_EVENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_EVENT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_STATUS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_STATUS),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_MEDIA_INFO),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO_LIST =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_PLAYER_MEDIA_INFO_LIST),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_CONNECTED_DEV_INFO),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO_LIST =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_AVRCP_CONNECTED_DEV_INFO_LIST),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_AVRCP_MEDIA_INFO,
    &t_rpc_decl_BT_AVRCP_PLAYER_SETTING,
    &t_rpc_decl_BT_AVRCP_PLAYITEM,
    &t_rpc_decl_BT_AVRCP_LIST_RANGE,
    &t_rpc_decl_BT_AVRCP_FOLDER_PATH,
    &t_rpc_decl_BT_AVRCP_PLAYERSETTING_CHANGE,
    &t_rpc_decl_BT_AVRCP_PLAYERSETTING_RSP,
    &t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL,
    &t_rpc_decl_BT_AVRCP_PLAYER_APP_EXT_ATTR,
    &t_rpc_decl_BT_AVRCP_PLAYER_APP_ATTR,
    &t_rpc_decl_BT_AVRCP_LIST_PLAYERSETTING_RSP,
    &t_rpc_decl_BT_AVRCP_ADDR_PLAYER,
    &t_rpc_decl_BT_AVRCP_CONNECTION_CB,
    &t_rpc_decl_BT_AVRCP_SET_VOL_REQ,
    &t_rpc_decl_BT_AVRCP_TRACK_CHANGE,
    &t_rpc_decl_BT_AVRCP_POS_CHANGE,
    &t_rpc_decl_BT_AVRCP_PLAY_STATUS_CHANGE,
    &t_rpc_decl_BT_AVRCP_ABS_SUPPORTED,
    &t_rpc_decl_BT_AVRCP_VOLUME_CHANGE,
    &t_rpc_decl_BT_AVRCP_PASSTHROUGH_CMD_REQ,
    &t_rpc_decl_BT_AVRCP_BATTERY_STATUS_CHANGE,
    &t_rpc_decl_BT_AVRCP_FEATURE_RSP,
    &t_rpc_decl_BT_AVRCP_FEATURE,
    &t_rpc_decl_BT_AVRCP_ELEMENT_ATTR_VAL,
    &t_rpc_decl_BT_AVRCP_ITEM_PLAYER,
    &t_rpc_decl_BT_AVRCP_ITEM_FOLDER,
    &t_rpc_decl_BT_AVRCP_ITEM_MEDIA,
    &t_rpc_decl_BT_AVRCP_FOLDER_ITEMS,
    &t_rpc_decl_BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA,
    &t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA,
    &t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA,
    &t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA,
    &t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA,
    &t_rpc_decl_BT_AVRCP_GET_FLODER_ITEMS_CB,
    &t_rpc_decl_BT_AVRCP_CHANGE_FOLDER_PATH_CB,
    &t_rpc_decl_BT_AVRCP_SET_BROWSED_PLAYER_CB,
    &t_rpc_decl_BT_AVRCP_SET_ADDRESSED_PLAYER_CB,
    &t_rpc_decl_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB,
    &t_rpc_decl_BT_AVRCP_EVENT_DATA,
    &t_rpc_decl_BT_AVRCP_EVENT_PARAM,
    &t_rpc_decl_BT_AVRCP_PLAYER_STATUS,
    &t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO,
    &t_rpc_decl_BT_AVRCP_PLAYER_MEDIA_INFO_LIST,
    &t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO,
    &t_rpc_decl_BT_AVRCP_CONNECTED_DEV_INFO_LIST
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_avrcp_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 47) ? at_rpc_desc_list [ui4_idx] : NULL);
}


