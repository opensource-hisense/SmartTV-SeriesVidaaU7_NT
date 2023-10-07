/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:23:48 2021'. */
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

#include "mtk_bt_service_ble_scanner_ipcrpc_struct.h"
#include "u_bt_mw_ble_scanner.h"


/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REG_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_ADD_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_TRACK_ADV_EVT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_STOP_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_UNREG_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_START_SCAN_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_CALLBACK_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_APP_FILTER_DATA;



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

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REG_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_REG_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_ADD_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_FILT_ADD_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_TRACK_ADV_EVT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_TRACK_ADV_EVT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_STOP_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_STOP_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_UNREG_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_UNREG_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_SCAN_FILT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_SCAN_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_REGULAR_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCH_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_START_SCAN_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_START_SCAN_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_CALLBACK_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_CALLBACK_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_SCANNER_APP_FILTER_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_SCANNER_APP_FILTER_DATA),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_BLE_SCANNER_REG_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_FILT_ADD_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_TRACK_ADV_EVT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_STOP_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_UNREG_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_DATA,
    &t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_SCAN_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_START_SCAN_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_CALLBACK_PARAM,
    &t_rpc_decl_BT_BLE_SCANNER_APP_FILTER_DATA
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_ble_scanner_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 31) ? at_rpc_desc_list [ui4_idx] : NULL);
}


