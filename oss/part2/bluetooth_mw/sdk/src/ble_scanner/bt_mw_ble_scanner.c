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


/* FILE NAME:  bt_mw_ble_scanner.c
 * PURPOSE:
 *      It provides BLE SCANNER and API to c_bt_mw_ble_scanner and other mw layer modules.
 * NOTES:
 */

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stddef.h>
#include "linuxbt_ble_scanner_if.h"
#include "bt_mw_gatt.h"
#include "bt_mw_ble_scanner.h"
#include "bt_mw_log.h"

static BT_BLE_SCANNER_EVENT_HANDLE_CB BtBleScannerAppCbk = NULL;

static INT32 bluetooth_ble_scanner_reg_cbk_fct(BT_BLE_SCANNER_EVENT_HANDLE_CB func)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    if (NULL == func)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER, "callback func is null!");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    BtBleScannerAppCbk = func;
    return ret;
}


INT32 bluetooth_ble_scanner_register_callback(BT_BLE_SCANNER_EVENT_HANDLE_CB func)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = bluetooth_ble_scanner_reg_cbk_fct(func);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"bluetooth_ble_scanner_reg_cbk_fct failed");
        return ret;
    }
    //ret = bt_mw_gatt_init();
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"bt_mw_gatt_init failed");
    }
    return ret;
}


INT32 bluetooth_ble_scanner_register(CHAR * app_uuid)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_register(app_uuid);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_register failed");
        return ret;
    }
    return ret;
}


INT32 bluetooth_ble_scanner_scan(BOOL start)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_scan(start);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_scan failed");
        return ret;
    }
    return ret;
}



INT32 bluetooth_ble_scanner_set_scan_param(UINT8 scanner_id, INT32 scan_interval, INT32 scan_window)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_set_scan_param(scanner_id, scan_interval, scan_window);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_set_scan_param failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_scan_filter_param_setup(UINT8 scanner_id, UINT8 action, UINT8 filter_index,
                                                            BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filter_param)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_scan_filter_param_setup(scanner_id, action, filter_index, scan_filter_param);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_scan_filter_param_setup failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_scan_filter_add(UINT8 scanner_id, UINT8 filter_index,
                                                    BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_scan_filter_add(scanner_id, filter_index, filter_data);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_scan_filter_add failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_scan_filter_clear(UINT8 scanner_id, UINT8 filter_index)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_scan_filter_clear(scanner_id, filter_index);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_scan_filter_clear failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_scan_filter_enable(UINT8 scanner_id, BOOL enable)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_scan_filter_enable(scanner_id, enable);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_scan_filter_enable failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_batchscan_cfg_storage(UINT8 scanner_id, UINT8 batch_scan_full_max,
                                                  UINT8 batch_scan_trunc_max,
                                                  UINT8 batch_scan_notify_threshold)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_batchscan_cfg_storage(scanner_id, batch_scan_full_max,
                                                        batch_scan_trunc_max, batch_scan_notify_threshold);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"bluetooth_ble_scanner_set_scan_parameters failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_batchscan_enable(UINT8 scanner_id, UINT8 scan_mode, INT32 scan_interval,
                                                   INT32 scan_window, UINT8 addr_type, UINT8 discard_rule)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_batchscan_enable(scanner_id, scan_mode, scan_interval, scan_window,
                                                                    addr_type, discard_rule);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"bluetooth_ble_scanner_batchscan_enable failed");
        return ret;
    }
    return ret;
}

INT32 bluetooth_ble_scanner_batchscan_disable(UINT8 scanner_id)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_batchscan_disable(scanner_id);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_batchscan_disable failed");
        return ret;
    }
    return ret;
}



INT32 bluetooth_ble_scanner_batchscan_read_reports(UINT8 scanner_id, UINT8 scan_mode)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_batchscan_read_reports(scanner_id, scan_mode);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_batchscan_read_reports failed");
        return ret;
    }
    return ret;
}


INT32 bluetooth_ble_scanner_unregister(UINT8 scanner_id)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    ret = linuxbt_ble_scanner_unregister(scanner_id);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"linuxbt_ble_scanner_unregister failed");
        return ret;
    }
    return ret;
}


/************ble scanner  callback *************/

VOID bt_mw_ble_scanner_notify_app(BT_BLE_SCANNER_CALLBACK_PARAM *ble_scanner_msg)
{
    tBTMW_GATT_MSG msg;
    msg.hdr.event = BTMW_BLE_SCANER_EVENT;
    msg.hdr.len = sizeof(BT_BLE_SCANNER_CALLBACK_PARAM);
    memcpy((void*)&msg.data.ble_scanner_param, ble_scanner_msg, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    bt_mw_gatt_nty_send_msg(&msg);
}

VOID bt_mw_ble_scanner_nty_handle(BT_BLE_SCANNER_CALLBACK_PARAM *ble_scanner_msg)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "bt_mw_ble_scanner_nty_handle: event:%d", ble_scanner_msg->event);
    if (BtBleScannerAppCbk)
    {
        BtBleScannerAppCbk(ble_scanner_msg);
    }
}

