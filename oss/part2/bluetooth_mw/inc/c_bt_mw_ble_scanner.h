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


#ifndef _C_BT_MW_BLE_SCANNER_H_
#define _C_BT_MW_BLE_SCANNER_H_

#include "u_bt_mw_ble_scanner.h"

extern INT32 c_btm_ble_scanner_register_callback(BT_BLE_SCANNER_EVENT_HANDLE_CB func);

extern INT32 c_btm_ble_scanner_register(CHAR * app_uuid);

extern INT32 c_btm_ble_scanner_scan(BOOL start);

extern INT32 c_btm_ble_scanner_set_scan_param(UINT8 scanner_id, INT32 scan_interval, INT32 scan_window);

extern INT32 c_btm_ble_scanner_scan_filter_param_setup(UINT8 scanner_id, UINT8 action, UINT8 filt_index,
                                                                    BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filter_param);

extern INT32 c_btm_ble_scanner_scan_filter_add(UINT8 scanner_id, UINT8 filter_index,
                                                    BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data);

extern INT32 c_btm_ble_scanner_scan_filter_clear(UINT8 scanner_id, UINT8 filter_index);

extern INT32 c_btm_ble_scanner_scan_filter_enable(UINT8 scanner_id, BOOL enable);

extern INT32 c_btm_ble_scanner_batchscan_cfg_storage(UINT8 scanner_id, UINT8 batch_scan_full_max,
                                                  UINT8 batch_scan_trunc_max,
                                                  UINT8 batch_scan_notify_threshold);

extern INT32 c_btm_ble_scanner_batchscan_enable(UINT8 scanner_id, UINT8 scan_mode, INT32 scan_interval,
                                                   INT32 scan_window, UINT8 addr_type, UINT8 discard_rule);

extern INT32 c_btm_ble_scanner_batchscan_disable(UINT8 scanner_id);

extern INT32 c_btm_ble_scanner_batchscan_read_reports(UINT8 scanner_id, UINT8 scan_mode);

extern INT32 c_btm_ble_scanner_unregister(UINT8 scanner_id);

#endif /*  _C_BT_MW_BLE_SCANNER_H_  */

