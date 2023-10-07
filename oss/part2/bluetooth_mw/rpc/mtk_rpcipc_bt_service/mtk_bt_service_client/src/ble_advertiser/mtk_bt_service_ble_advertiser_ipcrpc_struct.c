/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Mon May  3 19:59:44 2021'. */
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

#include "mtk_bt_service_ble_advertiser_ipcrpc_struct.h"
#include "u_bt_mw_ble_advertiser.h"


/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_GATT_ATTR;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PROPS_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_DATA_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PERIODIC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PERIODIC_ENABLE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_ENABLE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_GET_ADDR_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_START_SET_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_STOP_SET_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PARAM_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_ADV_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_GET_ADDR_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_START_SET_DATA;
static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_PARAM;



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

static const RPC_DESC_T t_rpc_decl_BT_GATT_ATTR =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATT_ATTR),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PROPS_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_PROPS_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_DATA_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_DATA_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PERIODIC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_PERIODIC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_PERIODIC_ENABLE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_PERIODIC_ENABLE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_ENABLE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_ENABLE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_GET_ADDR_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_GET_ADDR_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_START_SET_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_START_SET_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_STOP_SET_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_STOP_SET_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PARAM_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_SET_PARAM_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_ADV_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_SET_ADV_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_SET_PERIODIC_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_ENABLE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_GET_ADDR_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_GET_ADDR_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_START_SET_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_START_SET_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_BLE_ADV_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_BLE_ADV_EVENT_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_GATT_ATTR,
    &t_rpc_decl_BT_BLE_ADV_PROPS_PARAM,
    &t_rpc_decl_BT_BLE_ADV_PARAM,
    &t_rpc_decl_BT_BLE_ADV_DATA_PARAM,
    &t_rpc_decl_BT_BLE_ADV_PERIODIC_PARAM,
    &t_rpc_decl_BT_BLE_ADV_PERIODIC_ENABLE_PARAM,
    &t_rpc_decl_BT_BLE_ADV_ENABLE_PARAM,
    &t_rpc_decl_BT_BLE_ADV_GET_ADDR_PARAM,
    &t_rpc_decl_BT_BLE_ADV_START_SET_PARAM,
    &t_rpc_decl_BT_BLE_ADV_STOP_SET_PARAM,
    &t_rpc_decl_BT_BLE_ADV_EVENT_SET_PARAM_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_SET_ADV_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_SET_PERIODIC_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_GET_ADDR_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_START_SET_DATA,
    &t_rpc_decl_BT_BLE_ADV_EVENT_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_ble_advertiser_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 23) ? at_rpc_desc_list [ui4_idx] : NULL);
}


