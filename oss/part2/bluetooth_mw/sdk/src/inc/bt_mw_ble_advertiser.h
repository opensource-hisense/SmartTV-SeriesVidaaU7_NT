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


#ifndef __BT_MW_BLE_ADVERTISER_H__
#define __BT_MW_BLE_ADVERTISER_H__

#include "u_bt_mw_ble_advertiser.h"

#ifdef __cplusplus
extern "C" {
#endif

extern INT32 bluetooth_ble_adv_start_adv_set(
    BT_BLE_ADV_START_SET_PARAM *start_adv_set_param,
    BT_BLE_ADV_EVENT_HANDLE_CB adv_handle);

extern INT32 bluetooth_ble_adv_stop_adv_set(BT_BLE_ADV_STOP_SET_PARAM *stop_adv_set_param);
extern INT32 bluetooth_ble_adv_set_param(BT_BLE_ADV_PARAM *adv_param);
extern INT32 bluetooth_ble_adv_set_data(BT_BLE_ADV_DATA_PARAM *adv_data_param);
extern INT32 bluetooth_ble_adv_set_scan_rsp(BT_BLE_ADV_DATA_PARAM *scan_rsp);
extern INT32 bluetooth_ble_adv_set_periodic_param(BT_BLE_ADV_PERIODIC_PARAM *periodic_param);
extern INT32 bluetooth_ble_adv_set_periodic_data(BT_BLE_ADV_DATA_PARAM *periodic_data);
extern INT32 bluetooth_ble_adv_periodic_enable(BT_BLE_ADV_PERIODIC_ENABLE_PARAM *periodic_enable_param);
extern INT32 bluetooth_ble_adv_enable(BT_BLE_ADV_ENABLE_PARAM *adv_enable_param);
extern INT32 bluetooth_ble_adv_get_own_address(BT_BLE_ADV_GET_ADDR_PARAM *get_adv_addr_param);

extern VOID bt_mw_ble_adv_notify_app(BT_BLE_ADV_EVENT_PARAM *ble_adv_msg);
extern VOID bt_mw_ble_adv_nty_handle(BT_BLE_ADV_EVENT_PARAM *ble_adv_msg);

#ifdef __cplusplus
}
#endif

#endif/* __BT_MW_BLE_ADVERTISER_H__ */
