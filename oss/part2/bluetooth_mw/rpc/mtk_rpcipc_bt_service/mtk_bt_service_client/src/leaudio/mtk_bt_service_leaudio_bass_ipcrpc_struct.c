/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Jan 25 21:11:33 2022'. */
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

#include "u_bt_mw_leaudio_bass.h"
#include "mtk_bt_service_leaudio_bass_ipcrpc_struct.h"

/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BTMW_EXT_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SRC_SUBGROUP;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SRC;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_RECV;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_ID;
static const RPC_DESC_T t_rpc_decl_BT_BASS_LC3_CODEC_CONFIG;
static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_CODEC_CONFIG;
static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_BIS_CONFIG;
static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_SUBGROUP;
static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_CONN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_DISC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_RECV_STATE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_CODE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_SET_BROADCAST_SRC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_MODIFY_BROADCAST_SRC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_REMOVE_BROADCAST_SRC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_SET_BUILTIN_MODE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BASS_CONNECTED_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_BASS_CONNECTION_INFO;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_CONN_STATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_DEV_AVAIL_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_SCANNING_STATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_RECV_STATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_BUILTIN_MODE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_SYNC_LOST_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_PARAM;



static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BTMW_LE_FEATURES),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BTMW_EXT_FEATURES =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BTMW_EXT_FEATURES),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SRC_SUBGROUP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_SRC_SUBGROUP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SRC =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_SRC),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_RECV =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_RECV),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_ID =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_ID),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_LC3_CODEC_CONFIG =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_LC3_CODEC_CONFIG),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_CODEC_CONFIG =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_ANNOUNCE_CODEC_CONFIG),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_BIS_CONFIG =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_ANNOUNCE_BIS_CONFIG),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_SUBGROUP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_ANNOUNCE_SUBGROUP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_ANNOUNCE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_ANNOUNCE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_CONN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_CONN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_DISC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_DISC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_RECV_STATE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_RECV_STATE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_BROADCAST_CODE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_BROADCAST_CODE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_SET_BROADCAST_SRC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_SET_BROADCAST_SRC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_MODIFY_BROADCAST_SRC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_MODIFY_BROADCAST_SRC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_REMOVE_BROADCAST_SRC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_REMOVE_BROADCAST_SRC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_SET_BUILTIN_MODE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_SET_BUILTIN_MODE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_CONNECTED_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_CONNECTED_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_CONNECTION_INFO =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_CONNECTION_INFO),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_CONN_STATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_CONN_STATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_DEV_AVAIL_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_DEV_AVAIL_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_SCANNING_STATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_SCANNING_STATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_RECV_STATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_RECV_STATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_BUILTIN_MODE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_BUILTIN_MODE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_SYNC_LOST_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_SYNC_LOST_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_BASS_EVENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BASS_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BASS_EVENT_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BTMW_EXT_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_BASS_BROADCAST_SRC_SUBGROUP,
    &t_rpc_decl_BT_BASS_BROADCAST_SRC,
    &t_rpc_decl_BT_BASS_BROADCAST_RECV,
    &t_rpc_decl_BT_BASS_BROADCAST_ID,
    &t_rpc_decl_BT_BASS_LC3_CODEC_CONFIG,
    &t_rpc_decl_BT_BASS_ANNOUNCE_CODEC_CONFIG,
    &t_rpc_decl_BT_BASS_ANNOUNCE_BIS_CONFIG,
    &t_rpc_decl_BT_BASS_ANNOUNCE_SUBGROUP,
    &t_rpc_decl_BT_BASS_ANNOUNCE_DATA,
    &t_rpc_decl_BT_BASS_CONN_PARAM,
    &t_rpc_decl_BT_BASS_DISC_PARAM,
    &t_rpc_decl_BT_BASS_BROADCAST_SCAN_PARAM,
    &t_rpc_decl_BT_BASS_BROADCAST_RECV_STATE_PARAM,
    &t_rpc_decl_BT_BASS_BROADCAST_CODE_PARAM,
    &t_rpc_decl_BT_BASS_SET_BROADCAST_SRC_PARAM,
    &t_rpc_decl_BT_BASS_MODIFY_BROADCAST_SRC_PARAM,
    &t_rpc_decl_BT_BASS_REMOVE_BROADCAST_SRC_PARAM,
    &t_rpc_decl_BT_BASS_SET_BUILTIN_MODE_PARAM,
    &t_rpc_decl_BT_BASS_CONNECTED_DEVICE,
    &t_rpc_decl_BT_BASS_CONNECTION_INFO,
    &t_rpc_decl_BT_BASS_EVENT_CONN_STATE_DATA,
    &t_rpc_decl_BT_BASS_EVENT_DEV_AVAIL_DATA,
    &t_rpc_decl_BT_BASS_EVENT_SCANNING_STATE_DATA,
    &t_rpc_decl_BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA,
    &t_rpc_decl_BT_BASS_EVENT_RECV_STATE_DATA,
    &t_rpc_decl_BT_BASS_EVENT_BUILTIN_MODE_DATA,
    &t_rpc_decl_BT_BASS_EVENT_SYNC_LOST_DATA,
    &t_rpc_decl_BT_BASS_EVENT_DATA,
    &t_rpc_decl_BT_BASS_EVENT_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_leaudio_bass_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 32) ? at_rpc_desc_list [ui4_idx] : NULL);
}


