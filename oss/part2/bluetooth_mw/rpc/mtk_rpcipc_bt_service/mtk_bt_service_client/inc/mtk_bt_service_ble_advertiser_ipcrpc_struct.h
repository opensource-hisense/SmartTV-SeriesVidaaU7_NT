/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Mon May  3 19:59:44 2021'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_BLE_ADVERTISER_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_BLE_ADVERTISER_IPCRPC_STRUCT__H_




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


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_ble_advertiser_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_ble_advertiser_desc__ (1))


#define RPC_DESC_BT_GATT_ATTR  (__rpc_get_ble_advertiser_desc__ (2))


#define RPC_DESC_BT_BLE_ADV_PROPS_PARAM  (__rpc_get_ble_advertiser_desc__ (3))


#define RPC_DESC_BT_BLE_ADV_PARAM  (__rpc_get_ble_advertiser_desc__ (4))


#define RPC_DESC_BT_BLE_ADV_DATA_PARAM  (__rpc_get_ble_advertiser_desc__ (5))


#define RPC_DESC_BT_BLE_ADV_PERIODIC_PARAM  (__rpc_get_ble_advertiser_desc__ (6))


#define RPC_DESC_BT_BLE_ADV_PERIODIC_ENABLE_PARAM  (__rpc_get_ble_advertiser_desc__ (7))


#define RPC_DESC_BT_BLE_ADV_ENABLE_PARAM  (__rpc_get_ble_advertiser_desc__ (8))


#define RPC_DESC_BT_BLE_ADV_GET_ADDR_PARAM  (__rpc_get_ble_advertiser_desc__ (9))


#define RPC_DESC_BT_BLE_ADV_START_SET_PARAM  (__rpc_get_ble_advertiser_desc__ (10))


#define RPC_DESC_BT_BLE_ADV_STOP_SET_PARAM  (__rpc_get_ble_advertiser_desc__ (11))


#define RPC_DESC_BT_BLE_ADV_EVENT_SET_PARAM_DATA  (__rpc_get_ble_advertiser_desc__ (12))


#define RPC_DESC_BT_BLE_ADV_EVENT_SET_ADV_DATA  (__rpc_get_ble_advertiser_desc__ (13))


#define RPC_DESC_BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA  (__rpc_get_ble_advertiser_desc__ (14))


#define RPC_DESC_BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA  (__rpc_get_ble_advertiser_desc__ (15))


#define RPC_DESC_BT_BLE_ADV_EVENT_SET_PERIODIC_DATA  (__rpc_get_ble_advertiser_desc__ (16))


#define RPC_DESC_BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA  (__rpc_get_ble_advertiser_desc__ (17))


#define RPC_DESC_BT_BLE_ADV_EVENT_ENABLE_DATA  (__rpc_get_ble_advertiser_desc__ (18))


#define RPC_DESC_BT_BLE_ADV_EVENT_GET_ADDR_DATA  (__rpc_get_ble_advertiser_desc__ (19))


#define RPC_DESC_BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA  (__rpc_get_ble_advertiser_desc__ (20))


#define RPC_DESC_BT_BLE_ADV_EVENT_START_SET_DATA  (__rpc_get_ble_advertiser_desc__ (21))


#define RPC_DESC_BT_BLE_ADV_EVENT_PARAM  (__rpc_get_ble_advertiser_desc__ (22))



extern const RPC_DESC_T* __rpc_get_ble_advertiser_desc__ (UINT32  ui4_idx);


#endif

