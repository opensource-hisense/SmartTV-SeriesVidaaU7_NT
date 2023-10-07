/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:23:48 2021'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_BLE_SCANNER_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_BLE_SCANNER_IPCRPC_STRUCT__H_




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


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_ble_scanner_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_ble_scanner_desc__ (1))


#define RPC_DESC_BT_BLE_SCANNER_REG_EVT_DATA  (__rpc_get_ble_scanner_desc__ (2))


#define RPC_DESC_BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA  (__rpc_get_ble_scanner_desc__ (3))


#define RPC_DESC_BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA  (__rpc_get_ble_scanner_desc__ (4))


#define RPC_DESC_BT_BLE_SCANNER_FILT_ADD_EVT_DATA  (__rpc_get_ble_scanner_desc__ (5))


#define RPC_DESC_BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA  (__rpc_get_ble_scanner_desc__ (6))


#define RPC_DESC_BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA  (__rpc_get_ble_scanner_desc__ (7))


#define RPC_DESC_BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA  (__rpc_get_ble_scanner_desc__ (8))


#define RPC_DESC_BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA  (__rpc_get_ble_scanner_desc__ (9))


#define RPC_DESC_BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA  (__rpc_get_ble_scanner_desc__ (10))


#define RPC_DESC_BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA  (__rpc_get_ble_scanner_desc__ (11))


#define RPC_DESC_BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA  (__rpc_get_ble_scanner_desc__ (12))


#define RPC_DESC_BT_BLE_SCANNER_TRACK_ADV_EVT_DATA  (__rpc_get_ble_scanner_desc__ (13))


#define RPC_DESC_BT_BLE_SCANNER_STOP_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (14))


#define RPC_DESC_BT_BLE_SCANNER_UNREG_PARAM  (__rpc_get_ble_scanner_desc__ (15))


#define RPC_DESC_BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (16))


#define RPC_DESC_BT_BLE_SCANNER_SCAN_FILT_DATA  (__rpc_get_ble_scanner_desc__ (17))


#define RPC_DESC_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (18))


#define RPC_DESC_BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (19))


#define RPC_DESC_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (20))


#define RPC_DESC_BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (21))


#define RPC_DESC_BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (22))


#define RPC_DESC_BT_BLE_SCANNER_SCAN_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (23))


#define RPC_DESC_BT_BLE_SCANNER_REGULAR_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (24))


#define RPC_DESC_BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (25))


#define RPC_DESC_BT_BLE_SCANNER_BATCH_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (26))


#define RPC_DESC_BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM  (__rpc_get_ble_scanner_desc__ (27))


#define RPC_DESC_BT_BLE_SCANNER_START_SCAN_PARAM  (__rpc_get_ble_scanner_desc__ (28))


#define RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM  (__rpc_get_ble_scanner_desc__ (29))


#define RPC_DESC_BT_BLE_SCANNER_APP_FILTER_DATA  (__rpc_get_ble_scanner_desc__ (30))



extern const RPC_DESC_T* __rpc_get_ble_scanner_desc__ (UINT32  ui4_idx);


#endif

