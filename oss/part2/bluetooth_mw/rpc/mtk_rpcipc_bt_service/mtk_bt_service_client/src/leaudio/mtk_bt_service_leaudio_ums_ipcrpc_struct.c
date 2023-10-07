/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Wed Dec  1 15:46:45 2021'. */
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

#include "u_bt_mw_leaudio_ums.h"
#include "mtk_bt_service_leaudio_ums_ipcrpc_struct.h"

/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_UMS_REMOTE_ALLOCATION;
static const RPC_DESC_T t_rpc_decl_BT_UMS_CONFIG;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_CONNECTION_STATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_NODE_STATUS_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_STATUS_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_SOCKET_INDEX_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_AUDIO_CONF_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_CONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_DISCONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_START_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_START_EXT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_STOP_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_UPDATE_METADATA_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_LOCK_SET_PARAM;



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

static const RPC_DESC_T t_rpc_decl_BT_UMS_REMOTE_ALLOCATION =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_REMOTE_ALLOCATION),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_CONFIG =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_CONFIG),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_CONNECTION_STATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_CONNECTION_STATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_NODE_STATUS_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_GROUP_NODE_STATUS_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_STATUS_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_GROUP_STATUS_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_SOCKET_INDEX_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_SOCKET_INDEX_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_AUDIO_CONF_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_AUDIO_CONF_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_UMS_EVENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_EVENT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_CONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_CONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_DISCONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_DISCONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_START_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_GROUP_START_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_START_EXT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_GROUP_START_EXT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_STOP_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_GROUP_STOP_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_UPDATE_METADATA_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_UPDATE_METADATA_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_UMS_GROUP_LOCK_SET_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_UMS_GROUP_LOCK_SET_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_UMS_REMOTE_ALLOCATION,
    &t_rpc_decl_BT_UMS_CONFIG,
    &t_rpc_decl_BT_UMS_EVENT_CONNECTION_STATE_DATA,
    &t_rpc_decl_BT_UMS_EVENT_GROUP_NODE_STATUS_DATA,
    &t_rpc_decl_BT_UMS_EVENT_GROUP_STATUS_DATA,
    &t_rpc_decl_BT_UMS_EVENT_SOCKET_INDEX_DATA,
    &t_rpc_decl_BT_UMS_EVENT_AUDIO_CONF_DATA,
    &t_rpc_decl_BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA,
    &t_rpc_decl_BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA,
    &t_rpc_decl_BT_UMS_EVENT_DATA,
    &t_rpc_decl_BT_UMS_EVENT_PARAM,
    &t_rpc_decl_BT_UMS_CONNECT_PARAM,
    &t_rpc_decl_BT_UMS_DISCONNECT_PARAM,
    &t_rpc_decl_BT_UMS_GROUP_START_PARAM,
    &t_rpc_decl_BT_UMS_GROUP_START_EXT_PARAM,
    &t_rpc_decl_BT_UMS_GROUP_STOP_PARAM,
    &t_rpc_decl_BT_UMS_UPDATE_METADATA_PARAM,
    &t_rpc_decl_BT_UMS_GROUP_LOCK_SET_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_leaudio_ums_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 20) ? at_rpc_desc_list [ui4_idx] : NULL);
}


