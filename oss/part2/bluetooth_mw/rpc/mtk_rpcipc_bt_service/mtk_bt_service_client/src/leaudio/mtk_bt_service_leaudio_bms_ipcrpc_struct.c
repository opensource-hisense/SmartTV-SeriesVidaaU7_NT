/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Fri Nov 26 16:44:11 2021'. */
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

#include "u_bt_mw_leaudio_bms.h"
#include "mtk_bt_service_leaudio_bms_ipcrpc_struct.h"

/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BTMW_EXT_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT_BIS;
static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT_SUBGROUP;
static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_CREATED_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_DESTORYED_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_STATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_SOCKET_CHANNEL_MAP;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_SOCKET_INDEX_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_ISO_STATUS_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_OWN_ADDRESS_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_CREATE_BROADCAST_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_CREATE_BROADCAST_EXT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_START_BROADCAST_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_PAUSE_BROADCAST_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_STOP_BROADCAST_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_GET_OWN_ADDRESS_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM;



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

static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT_BIS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_ANNOUNCEMENT_BIS),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT_SUBGROUP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_ANNOUNCEMENT_SUBGROUP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_ANNOUNCEMENT =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_ANNOUNCEMENT),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_CREATED_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_BROADCAST_CREATED_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_DESTORYED_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_BROADCAST_DESTORYED_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_BROADCAST_STATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_BROADCAST_STATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_SOCKET_CHANNEL_MAP =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_SOCKET_CHANNEL_MAP),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_SOCKET_INDEX_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_SOCKET_INDEX_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_ISO_STATUS_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_ISO_STATUS_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_OWN_ADDRESS_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_OWN_ADDRESS_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_BMS_EVENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_EVENT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_CREATE_BROADCAST_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_CREATE_BROADCAST_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_CREATE_BROADCAST_EXT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_CREATE_BROADCAST_EXT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_START_BROADCAST_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_START_BROADCAST_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_PAUSE_BROADCAST_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_PAUSE_BROADCAST_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_STOP_BROADCAST_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_STOP_BROADCAST_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_GET_OWN_ADDRESS_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_GET_OWN_ADDRESS_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BTMW_EXT_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_BMS_ANNOUNCEMENT_BIS,
    &t_rpc_decl_BT_BMS_ANNOUNCEMENT_SUBGROUP,
    &t_rpc_decl_BT_BMS_ANNOUNCEMENT,
    &t_rpc_decl_BT_BMS_EVENT_BROADCAST_CREATED_DATA,
    &t_rpc_decl_BT_BMS_EVENT_BROADCAST_DESTORYED_DATA,
    &t_rpc_decl_BT_BMS_EVENT_BROADCAST_STATE_DATA,
    &t_rpc_decl_BT_BMS_EVENT_SOCKET_CHANNEL_MAP,
    &t_rpc_decl_BT_BMS_EVENT_SOCKET_INDEX_DATA,
    &t_rpc_decl_BT_BMS_EVENT_ISO_STATUS_DATA,
    &t_rpc_decl_BT_BMS_EVENT_OWN_ADDRESS_DATA,
    &t_rpc_decl_BT_BMS_EVENT_DATA,
    &t_rpc_decl_BT_BMS_EVENT_PARAM,
    &t_rpc_decl_BT_BMS_CREATE_BROADCAST_PARAM,
    &t_rpc_decl_BT_BMS_CREATE_BROADCAST_EXT_PARAM,
    &t_rpc_decl_BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM,
    &t_rpc_decl_BT_BMS_START_BROADCAST_PARAM,
    &t_rpc_decl_BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM,
    &t_rpc_decl_BT_BMS_PAUSE_BROADCAST_PARAM,
    &t_rpc_decl_BT_BMS_STOP_BROADCAST_PARAM,
    &t_rpc_decl_BT_BMS_GET_OWN_ADDRESS_PARAM,
    &t_rpc_decl_BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_leaudio_bms_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 24) ? at_rpc_desc_list [ui4_idx] : NULL);
}


