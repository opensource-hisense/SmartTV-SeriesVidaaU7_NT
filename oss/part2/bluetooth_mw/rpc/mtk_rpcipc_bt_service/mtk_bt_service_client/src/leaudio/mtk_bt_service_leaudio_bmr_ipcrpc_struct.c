/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Dec  2 13:37:58 2021'. */
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

#include "u_bt_mw_leaudio_bmr.h"
#include "mtk_bt_service_leaudio_bmr_ipcrpc_struct.h"

/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_BMR_CODEC_CONFIGURATION_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_METADATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_CODEC_ID_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_BIS_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_SUBGROUP_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_SRC_INFO_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_CONN_STATE_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SCAN_STATE_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SRC_INFO_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_SOCKET_INDEX_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_STREAMING_EVENT_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_STREAMING_EVENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BMR_INIT_PARAMS;
static const RPC_DESC_T t_rpc_decl_BT_BMR_DISCOVERY_PARAMS;
static const RPC_DESC_T t_rpc_decl_BT_BMR_SOLICITATION_REQUEST_PARAMS;
static const RPC_DESC_T t_rpc_decl_BT_BMR_SET_BROADCAST_CODE_PARAMS;



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

static const RPC_DESC_T t_rpc_decl_BT_BMR_CODEC_CONFIGURATION_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_CODEC_CONFIGURATION_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_METADATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_METADATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_METADATA_T, vendor_metadata)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_CODEC_ID_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_CODEC_ID_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_BIS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_BIS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_SUBGROUP_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_SUBGROUP_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_BMR_METADATA_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_SUBGROUP_T, metadata)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_SRC_INFO_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_SRC_INFO_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_BMR_SUBGROUP_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_SRC_INFO_T, subgroups)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_CONN_STATE_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_CONN_STATE_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SCAN_STATE_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_SCAN_STATE_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SRC_INFO_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_SRC_INFO_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_BMR_SRC_INFO_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_EVENT_SRC_INFO_DATA_T, info)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_SOCKET_INDEX_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_SOCKET_INDEX_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_STREAMING_EVENT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_STREAMING_EVENT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_STREAMING_EVENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_STREAMING_EVENT_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_BMR_EVENT_DATA),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_BMR_EVENT_SRC_INFO_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "source_info"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_EVENT_PARAM),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_BMR_EVENT_DATA,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_EVENT_PARAM, data)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_INIT_PARAMS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_INIT_PARAMS),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_DISCOVERY_PARAMS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_DISCOVERY_PARAMS),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_SOLICITATION_REQUEST_PARAMS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_SOLICITATION_REQUEST_PARAMS),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_BMR_SOLICITATION_REQUEST_PARAMS, user_data)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_BMR_SET_BROADCAST_CODE_PARAMS =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BMR_SET_BROADCAST_CODE_PARAMS),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_BMR_CODEC_CONFIGURATION_T,
    &t_rpc_decl_BT_BMR_METADATA_T,
    &t_rpc_decl_BT_BMR_CODEC_ID_T,
    &t_rpc_decl_BT_BMR_BIS_T,
    &t_rpc_decl_BT_BMR_SUBGROUP_T,
    &t_rpc_decl_BT_BMR_SRC_INFO_T,
    &t_rpc_decl_BT_BMR_EVENT_CONN_STATE_DATA_T,
    &t_rpc_decl_BT_BMR_EVENT_SCAN_STATE_DATA_T,
    &t_rpc_decl_BT_BMR_EVENT_SRC_INFO_DATA_T,
    &t_rpc_decl_BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T,
    &t_rpc_decl_BT_BMR_SOCKET_INDEX_T,
    &t_rpc_decl_BT_BMR_STREAMING_EVENT_T,
    &t_rpc_decl_BT_BMR_EVENT_STREAMING_EVENT_DATA_T,
    &t_rpc_decl_BT_BMR_EVENT_DATA,
    &t_rpc_decl_BT_BMR_EVENT_PARAM,
    &t_rpc_decl_BT_BMR_INIT_PARAMS,
    &t_rpc_decl_BT_BMR_DISCOVERY_PARAMS,
    &t_rpc_decl_BT_BMR_SOLICITATION_REQUEST_PARAMS,
    &t_rpc_decl_BT_BMR_SET_BROADCAST_CODE_PARAMS
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_leaudio_bmr_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 21) ? at_rpc_desc_list [ui4_idx] : NULL);
}


