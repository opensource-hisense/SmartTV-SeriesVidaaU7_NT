/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:11:21 2021'. */
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

#include "u_bt_mw_gattc.h"
#include "u_bt_mw_gatts.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"



/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_GATT_ATTR;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_DISCONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_REFRESH_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_DISCOVER_BY_UUID_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_CHAR_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_BY_UUID_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_CHAR_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_DESC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_DESC_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EXEC_WRITE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_REG_NOTIF_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_RSSI_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_DEV_TYPE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_CHG_MTU_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONN_UPDATE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_PHY_SET_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_PHY_READ_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_GATT_DB_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_TEST_COMMAND_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_REGISTER_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_CONNECTION_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_REG_NOTIF_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_NOTIF_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_CHAR_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_DESC_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_WRITE_DESC_RSP_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_RSSI_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_MTU_CHG_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_GET_GATT_DB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_DISCOVER_COMPL_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_PHY_UPDATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_PHY_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_CONN_UPDATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_CONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_DISCONNECT_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_SERVICE_ADD_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_SERVICE_DEL_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_IND_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_RESPONSE_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_PHY_READ_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_PHY_SET_PARAM;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_REGISTER_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_CONNECTION_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_SERVICE_ADD_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_IND_SENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_READ_REQ_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_WRITE_REQ_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_WRITE_EXE_REQ_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_MTU_CHG_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_READ_PHY_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_PHY_UPDATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_CONN_UPDATE_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_DATA;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_PARAM;



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

static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_CONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_DISCONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_DISCONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_REFRESH_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_REFRESH_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_DISCOVER_BY_UUID_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_DISCOVER_BY_UUID_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_CHAR_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_CHAR_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_BY_UUID_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_BY_UUID_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_CHAR_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_WRITE_CHAR_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_DESC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_DESC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_DESC_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_WRITE_DESC_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EXEC_WRITE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EXEC_WRITE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_REG_NOTIF_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_REG_NOTIF_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_RSSI_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_RSSI_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_DEV_TYPE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_DEV_TYPE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_CHG_MTU_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_CHG_MTU_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONN_UPDATE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_CONN_UPDATE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_PHY_SET_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_PHY_SET_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_PHY_READ_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_PHY_READ_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_GATT_DB_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_GATT_DB_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_TEST_COMMAND_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_TEST_COMMAND_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_REGISTER_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_REGISTER_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_CONNECTION_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_CONNECTION_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_REG_NOTIF_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_REG_NOTIF_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_NOTIF_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_NOTIF_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_CHAR_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_READ_CHAR_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_DESC_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_READ_DESC_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_WRITE_DESC_RSP_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_WRITE_DESC_RSP_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_RSSI_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_READ_RSSI_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_MTU_CHG_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_MTU_CHG_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_GET_GATT_DB_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_GET_GATT_DB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_DISCOVER_COMPL_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_DISCOVER_COMPL_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_PHY_UPDATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_PHY_UPDATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_READ_PHY_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_READ_PHY_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_CONN_UPDATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_CONN_UPDATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_EVENT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_CONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_CONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_DISCONNECT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_DISCONNECT_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_SERVICE_ADD_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_SERVICE_ADD_PARAM),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_GATT_ATTR,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_GATTS_SERVICE_ADD_PARAM, attrs)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_SERVICE_DEL_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_SERVICE_DEL_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_IND_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_IND_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_RESPONSE_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_RESPONSE_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_PHY_READ_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_PHY_READ_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_PHY_SET_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_PHY_SET_PARAM),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_REGISTER_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_REGISTER_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_CONNECTION_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_CONNECTION_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_SERVICE_ADD_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_SERVICE_ADD_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_IND_SENT_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_IND_SENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_READ_REQ_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_READ_REQ_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_WRITE_REQ_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_WRITE_REQ_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_WRITE_EXE_REQ_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_WRITE_EXE_REQ_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_MTU_CHG_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_MTU_CHG_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_READ_PHY_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_READ_PHY_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_PHY_UPDATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_PHY_UPDATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_CONN_UPDATE_DATA =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_CONN_UPDATE_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_GATTS_EVENT_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EVENT_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_GATT_ATTR,
    &t_rpc_decl_BT_GATTC_CONNECT_PARAM,
    &t_rpc_decl_BT_GATTC_DISCONNECT_PARAM,
    &t_rpc_decl_BT_GATTC_REFRESH_PARAM,
    &t_rpc_decl_BT_GATTC_DISCOVER_BY_UUID_PARAM,
    &t_rpc_decl_BT_GATTC_READ_CHAR_PARAM,
    &t_rpc_decl_BT_GATTC_READ_BY_UUID_PARAM,
    &t_rpc_decl_BT_GATTC_WRITE_CHAR_PARAM,
    &t_rpc_decl_BT_GATTC_READ_DESC_PARAM,
    &t_rpc_decl_BT_GATTC_WRITE_DESC_PARAM,
    &t_rpc_decl_BT_GATTC_EXEC_WRITE_PARAM,
    &t_rpc_decl_BT_GATTC_REG_NOTIF_PARAM,
    &t_rpc_decl_BT_GATTC_READ_RSSI_PARAM,
    &t_rpc_decl_BT_GATTC_GET_DEV_TYPE_PARAM,
    &t_rpc_decl_BT_GATTC_CHG_MTU_PARAM,
    &t_rpc_decl_BT_GATTC_CONN_UPDATE_PARAM,
    &t_rpc_decl_BT_GATTC_PHY_SET_PARAM,
    &t_rpc_decl_BT_GATTC_PHY_READ_PARAM,
    &t_rpc_decl_BT_GATTC_GET_GATT_DB_PARAM,
    &t_rpc_decl_BT_GATTC_TEST_COMMAND_PARAM,
    &t_rpc_decl_BT_GATTC_EVENT_REGISTER_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_CONNECTION_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_REG_NOTIF_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_NOTIF_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_READ_CHAR_RSP_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_READ_DESC_RSP_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_WRITE_DESC_RSP_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_READ_RSSI_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_MTU_CHG_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_GET_GATT_DB_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_DISCOVER_COMPL_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_PHY_UPDATE_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_READ_PHY_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_CONN_UPDATE_DATA,
    &t_rpc_decl_BT_GATTC_EVENT_PARAM,
    &t_rpc_decl_BT_GATTS_CONNECT_PARAM,
    &t_rpc_decl_BT_GATTS_DISCONNECT_PARAM,
    &t_rpc_decl_BT_GATTS_SERVICE_ADD_PARAM,
    &t_rpc_decl_BT_GATTS_SERVICE_DEL_PARAM,
    &t_rpc_decl_BT_GATTS_IND_PARAM,
    &t_rpc_decl_BT_GATTS_RESPONSE_PARAM,
    &t_rpc_decl_BT_GATTS_PHY_READ_PARAM,
    &t_rpc_decl_BT_GATTS_PHY_SET_PARAM,
    &t_rpc_decl_BT_GATTS_EVENT_REGISTER_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_CONNECTION_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_SERVICE_ADD_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_IND_SENT_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_READ_REQ_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_WRITE_REQ_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_WRITE_EXE_REQ_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_MTU_CHG_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_READ_PHY_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_PHY_UPDATE_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_CONN_UPDATE_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_DATA,
    &t_rpc_decl_BT_GATTS_EVENT_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_gatt_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 60) ? at_rpc_desc_list [ui4_idx] : NULL);
}


