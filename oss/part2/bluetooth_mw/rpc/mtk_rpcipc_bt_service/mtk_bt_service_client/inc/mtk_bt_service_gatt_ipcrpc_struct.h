/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:11:21 2021'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_GATT_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_GATT_IPCRPC_STRUCT__H_




/* Start of header pre-amble file 'preamble_file.h'. */

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

#include "u_rpc.h"


/* End of header pre-amble file 'preamble_file.h'. */


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_gatt_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_gatt_desc__ (1))


#define RPC_DESC_BT_GATT_ATTR  (__rpc_get_gatt_desc__ (2))


#define RPC_DESC_BT_GATTC_CONNECT_PARAM  (__rpc_get_gatt_desc__ (3))


#define RPC_DESC_BT_GATTC_DISCONNECT_PARAM  (__rpc_get_gatt_desc__ (4))


#define RPC_DESC_BT_GATTC_REFRESH_PARAM  (__rpc_get_gatt_desc__ (5))


#define RPC_DESC_BT_GATTC_DISCOVER_BY_UUID_PARAM  (__rpc_get_gatt_desc__ (6))


#define RPC_DESC_BT_GATTC_READ_CHAR_PARAM  (__rpc_get_gatt_desc__ (7))


#define RPC_DESC_BT_GATTC_READ_BY_UUID_PARAM  (__rpc_get_gatt_desc__ (8))


#define RPC_DESC_BT_GATTC_WRITE_CHAR_PARAM  (__rpc_get_gatt_desc__ (9))


#define RPC_DESC_BT_GATTC_READ_DESC_PARAM  (__rpc_get_gatt_desc__ (10))


#define RPC_DESC_BT_GATTC_WRITE_DESC_PARAM  (__rpc_get_gatt_desc__ (11))


#define RPC_DESC_BT_GATTC_EXEC_WRITE_PARAM  (__rpc_get_gatt_desc__ (12))


#define RPC_DESC_BT_GATTC_REG_NOTIF_PARAM  (__rpc_get_gatt_desc__ (13))


#define RPC_DESC_BT_GATTC_READ_RSSI_PARAM  (__rpc_get_gatt_desc__ (14))


#define RPC_DESC_BT_GATTC_GET_DEV_TYPE_PARAM  (__rpc_get_gatt_desc__ (15))


#define RPC_DESC_BT_GATTC_CHG_MTU_PARAM  (__rpc_get_gatt_desc__ (16))


#define RPC_DESC_BT_GATTC_CONN_UPDATE_PARAM  (__rpc_get_gatt_desc__ (17))


#define RPC_DESC_BT_GATTC_PHY_SET_PARAM  (__rpc_get_gatt_desc__ (18))


#define RPC_DESC_BT_GATTC_PHY_READ_PARAM  (__rpc_get_gatt_desc__ (19))


#define RPC_DESC_BT_GATTC_GET_GATT_DB_PARAM  (__rpc_get_gatt_desc__ (20))


#define RPC_DESC_BT_GATTC_TEST_COMMAND_PARAM  (__rpc_get_gatt_desc__ (21))


#define RPC_DESC_BT_GATTC_EVENT_REGISTER_DATA  (__rpc_get_gatt_desc__ (22))


#define RPC_DESC_BT_GATTC_EVENT_CONNECTION_DATA  (__rpc_get_gatt_desc__ (23))


#define RPC_DESC_BT_GATTC_EVENT_REG_NOTIF_DATA  (__rpc_get_gatt_desc__ (24))


#define RPC_DESC_BT_GATTC_EVENT_NOTIF_DATA  (__rpc_get_gatt_desc__ (25))


#define RPC_DESC_BT_GATTC_EVENT_READ_CHAR_RSP_DATA  (__rpc_get_gatt_desc__ (26))


#define RPC_DESC_BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA  (__rpc_get_gatt_desc__ (27))


#define RPC_DESC_BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA  (__rpc_get_gatt_desc__ (28))


#define RPC_DESC_BT_GATTC_EVENT_READ_DESC_RSP_DATA  (__rpc_get_gatt_desc__ (29))


#define RPC_DESC_BT_GATTC_EVENT_WRITE_DESC_RSP_DATA  (__rpc_get_gatt_desc__ (30))


#define RPC_DESC_BT_GATTC_EVENT_READ_RSSI_DATA  (__rpc_get_gatt_desc__ (31))


#define RPC_DESC_BT_GATTC_EVENT_MTU_CHG_DATA  (__rpc_get_gatt_desc__ (32))


#define RPC_DESC_BT_GATTC_EVENT_GET_GATT_DB_DATA  (__rpc_get_gatt_desc__ (33))


#define RPC_DESC_BT_GATTC_EVENT_DISCOVER_COMPL_DATA  (__rpc_get_gatt_desc__ (34))


#define RPC_DESC_BT_GATTC_EVENT_PHY_UPDATE_DATA  (__rpc_get_gatt_desc__ (35))


#define RPC_DESC_BT_GATTC_EVENT_READ_PHY_DATA  (__rpc_get_gatt_desc__ (36))


#define RPC_DESC_BT_GATTC_EVENT_CONN_UPDATE_DATA  (__rpc_get_gatt_desc__ (37))


#define RPC_DESC_BT_GATTC_EVENT_PARAM  (__rpc_get_gatt_desc__ (38))


#define RPC_DESC_BT_GATTS_CONNECT_PARAM  (__rpc_get_gatt_desc__ (39))


#define RPC_DESC_BT_GATTS_DISCONNECT_PARAM  (__rpc_get_gatt_desc__ (40))


#define RPC_DESC_BT_GATTS_SERVICE_ADD_PARAM  (__rpc_get_gatt_desc__ (41))


#define RPC_DESC_BT_GATTS_SERVICE_DEL_PARAM  (__rpc_get_gatt_desc__ (42))


#define RPC_DESC_BT_GATTS_IND_PARAM  (__rpc_get_gatt_desc__ (43))


#define RPC_DESC_BT_GATTS_RESPONSE_PARAM  (__rpc_get_gatt_desc__ (44))


#define RPC_DESC_BT_GATTS_PHY_READ_PARAM  (__rpc_get_gatt_desc__ (45))


#define RPC_DESC_BT_GATTS_PHY_SET_PARAM  (__rpc_get_gatt_desc__ (46))


#define RPC_DESC_BT_GATTS_EVENT_REGISTER_DATA  (__rpc_get_gatt_desc__ (47))


#define RPC_DESC_BT_GATTS_EVENT_CONNECTION_DATA  (__rpc_get_gatt_desc__ (48))


#define RPC_DESC_BT_GATTS_EVENT_SERVICE_ADD_DATA  (__rpc_get_gatt_desc__ (49))


#define RPC_DESC_BT_GATTS_EVENT_IND_SENT_DATA  (__rpc_get_gatt_desc__ (50))


#define RPC_DESC_BT_GATTS_EVENT_READ_REQ_DATA  (__rpc_get_gatt_desc__ (51))


#define RPC_DESC_BT_GATTS_EVENT_WRITE_REQ_DATA  (__rpc_get_gatt_desc__ (52))


#define RPC_DESC_BT_GATTS_EVENT_WRITE_EXE_REQ_DATA  (__rpc_get_gatt_desc__ (53))


#define RPC_DESC_BT_GATTS_EVENT_MTU_CHG_DATA  (__rpc_get_gatt_desc__ (54))


#define RPC_DESC_BT_GATTS_EVENT_READ_PHY_DATA  (__rpc_get_gatt_desc__ (55))


#define RPC_DESC_BT_GATTS_EVENT_PHY_UPDATE_DATA  (__rpc_get_gatt_desc__ (56))


#define RPC_DESC_BT_GATTS_EVENT_CONN_UPDATE_DATA  (__rpc_get_gatt_desc__ (57))


#define RPC_DESC_BT_GATTS_EVENT_DATA  (__rpc_get_gatt_desc__ (58))


#define RPC_DESC_BT_GATTS_EVENT_PARAM  (__rpc_get_gatt_desc__ (59))



extern const RPC_DESC_T* __rpc_get_gatt_desc__ (UINT32  ui4_idx);


#endif

