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


/*-----------------------------------------------------------------------------
                        include files
-----------------------------------------------------------------------------*/
#include <stdio.h>

#include "mtk_bt_service_ble_scanner_wrapper.h"
#include "mtk_bt_service_ble_scanner_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Ble_Scanner]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

EXPORT_SYMBOL INT16 get_feature_selection(const BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data)
{
    int selc = 0;
    if (0 != strlen(scan_filt_data->bt_addr))
    {
        selc |= (1 << SCAN_FILT_TYPE_ADDRESS);
    }
    if (0 != strlen(scan_filt_data->srvc_uuid) && 0 != strlen(scan_filt_data->srvc_uuid_mask))
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_UUID);
    }
    if (0 != strlen(scan_filt_data->srvc_sol_uuid) && 0 != strlen(scan_filt_data->srvc_sol_uuid_mask))
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_SOL_UUID);
    }
    if (0 != strlen(scan_filt_data->local_name))
    {
        selc |= (1 << SCAN_FILT_TYPE_LOCAL_NAME);
    }
    if (0 != scan_filt_data->manu_data_len)
    {
        selc |= (1 << SCAN_FILT_TYPE_MANU_DATA);
    }
    if (0 != scan_filt_data->srvc_data_len)
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_DATA);
    }
    return selc;
}


EXPORT_SYMBOL INT32 regular_start_simple_scan(BT_BLE_SCANNER_START_SCAN_PARAM  **start_scan_param,
                                           BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param)
{
    BT_RW_LOG("regular_start_simple_scan");
    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param_t = *start_scan_param;

    UINT16 scan_windows = 0;
    UINT16 scan_interval = 0;
    switch (regular_scan_param->scan_mode) {
      case SCAN_MODE_LOW_POWER:
          scan_windows = SCAN_MODE_REGULAR_LOW_POWER_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_LOW_POWER_INTERVAL_MS;
          break;
      case SCAN_MODE_BALANCED:
          scan_windows = SCAN_MODE_REGULAR_BALANCED_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_BALANCED_INTERVAL_MS;
          break;
      case SCAN_MODE_LOW_LATENCY:
          scan_windows = SCAN_MODE_REGULAR_LOW_LATENCY_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_LOW_LATENCY_INTERVAL_MS;
          break;
      default:
          BT_RW_LOG("Unknown scan_mode of regular scan in regular_start_simple_scan");
          return -1;
    }
    start_scan_param_t->scan_type = REGULAR_SCAN;
    //regular_scan_param
    BT_BLE_SCANNER_REGULAR_SCAN_PARAM *start_scan_scan_param =
                    &start_scan_param_t->scan_param.regular_scan_param;
    start_scan_scan_param->scan_windows = scan_windows;
    start_scan_scan_param->scan_interval= scan_interval;

    return 0;
}

EXPORT_SYMBOL INT32 regular_start_simple_scan_with_filt(BT_BLE_SCANNER_START_SCAN_PARAM  **start_scan_param,
                                     BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM *regular_scan_with_filt_param)
{
    BT_RW_LOG("regular_start_simple_scan_with_filt");
    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param_t = *start_scan_param;

    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param =
                                &regular_scan_with_filt_param->regular_scan_param;
    INT32 scan_windows = 0;
    INT32 scan_interval = 0;
    UINT8 dely_mode = 0;
    UINT16 found_timeout = 0;
    UINT16 lost_timeout = 0;
    UINT8 found_timeout_cnt = 0;
    UINT8 rssi_high_thres = 128;
    UINT8 rssi_low_thres = 128;
    UINT16 num_of_tracking_entries = 0;
    UINT16 list_logic_type = 0x00ff;
    UINT8 filt_logic_type = 1;

    switch (regular_scan_param->scan_mode) {
      case SCAN_MODE_LOW_POWER:
          scan_windows = SCAN_MODE_REGULAR_LOW_POWER_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_LOW_POWER_INTERVAL_MS;
          break;
      case SCAN_MODE_BALANCED:
          scan_windows = SCAN_MODE_REGULAR_BALANCED_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_BALANCED_INTERVAL_MS;
          break;
      case SCAN_MODE_LOW_LATENCY:
          scan_windows = SCAN_MODE_REGULAR_LOW_LATENCY_WINDOW_MS;
          scan_interval = SCAN_MODE_REGULAR_LOW_LATENCY_INTERVAL_MS;
          break;
      default:
          BT_RW_LOG("Unknown scan_mode of regular_scan_param in regular_start_simple_scan_with_filt");
          return -1;
    }

    BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM *scan_filt_param =
                            &regular_scan_with_filt_param->scan_filt_param;

    switch (scan_filt_param->dely_mode) {
      case FILT_DELY_MODE_IMMEDIATE:
          dely_mode = FILT_DELY_MODE_IMMEDIATE;
          break;
      case FILT_DELY_MODE_ON_FOUND:
          dely_mode = FILT_DELY_MODE_ON_FOUND;

          switch (scan_filt_param->num_of_matches) {
            case MATCH_NUM_ONE_ADVERTISEMENT:
                num_of_tracking_entries = 1;
                break;
            case MATCH_NUM_FEW_ADVERTISEMENT:
                num_of_tracking_entries = 2;
                break;
            case MATCH_NUM_MAX_ADVERTISEMENT:
                num_of_tracking_entries = 5;
                break;
            default:
                BT_RW_LOG("Unknown num_of_matches of scan_filt_param in regular_start_simple_scan_with_filt");
                return -1;
          }

          switch (scan_filt_param->filt_match_mode) {
            case MATCH_MODE_AGGRESSIVE:
                found_timeout = 500;
                lost_timeout = 10000;
                found_timeout_cnt = 1;
                break;
            case MATCH_MODE_STICKY:
                found_timeout = 4500;
                lost_timeout = 10000;
                found_timeout_cnt = 4;
                break;
            default:
                BT_RW_LOG("Unknown num_of_matches of scan_filt_param in regular_start_simple_scan_with_filt");
                return -1;
          }

          break;
      default:
          BT_RW_LOG("Unknown dely_mode of scan_filt_param in regular_start_simple_scan_with_filt");
          return -1;
    }

    start_scan_param_t->scan_type = REGULAR_SCAN_WITH_FILT;
    //regular_scan_param
    BT_BLE_SCANNER_REGULAR_SCAN_PARAM *start_scan_scan_param =
                    &start_scan_param_t->scan_param.regular_scan_filt_param.regular_scan_param;
    start_scan_scan_param->scan_windows = scan_windows;
    start_scan_scan_param->scan_interval= scan_interval;
    //scan_filt_data
    start_scan_param_t->scan_param.regular_scan_filt_param.scan_filt_num =
                                       regular_scan_with_filt_param->scan_filt_num;

    memcpy(start_scan_param_t->scan_param.regular_scan_filt_param.scan_filt_data,
                                    regular_scan_with_filt_param->scan_filt_data,
                                    sizeof(regular_scan_with_filt_param->scan_filt_data));
    //scan_filt_param
    BT_BLE_SCANNER_SCAN_FILT_PARAM *start_scan_filt_param =
                    start_scan_param_t->scan_param.regular_scan_filt_param.scan_filt_param;

    int size = regular_scan_with_filt_param->scan_filt_num < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM
               ? regular_scan_with_filt_param->scan_filt_num : BT_BLE_SCANNER_MAX_SCAN_FILT_NUM;
    for (int i = 0; i < size; i ++)
    {
        start_scan_filt_param[i].feat_seln= get_feature_selection(&(regular_scan_with_filt_param->scan_filt_data)[i]);
        start_scan_filt_param[i].list_logic_type= list_logic_type;
        start_scan_filt_param[i].filt_logic_type= filt_logic_type;
        start_scan_filt_param[i].rssi_high_thres= rssi_high_thres;
        start_scan_filt_param[i].rssi_low_thres= rssi_low_thres;
        start_scan_filt_param[i].dely_mode= dely_mode;
        start_scan_filt_param[i].found_timeout= found_timeout;
        start_scan_filt_param[i].lost_timeout= lost_timeout;
        start_scan_filt_param[i].found_timeout_cnt= found_timeout_cnt;
        start_scan_filt_param[i].num_of_tracking_entries= num_of_tracking_entries;
    }

    return 0;
}


EXPORT_SYMBOL INT32 batch_start_simple_scan(BT_BLE_SCANNER_START_SCAN_PARAM  **start_scan_param,
                                           BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM *batch_scan_param)
{
    BT_RW_LOG("batch_start_simple_scan");
    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param_t = *start_scan_param;

    INT32 scan_windows = 0;
    INT32 scan_interval = 0;
    UINT8 batch_scan_full_max = 0;
    UINT8 batch_scan_trunc_max = 0;
    UINT8 batch_scan_mode = 0;
    switch (batch_scan_param->scan_mode) {
      case SCAN_MODE_LOW_POWER:
          scan_windows = SCAN_MODE_BATCH_LOW_POWER_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_LOW_POWER_INTERVAL_MS;
          break;
      case SCAN_MODE_BALANCED:
          scan_windows = SCAN_MODE_BATCH_BALANCED_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_BALANCED_INTERVAL_MS;
          break;
      case SCAN_MODE_LOW_LATENCY:
          scan_windows = SCAN_MODE_BATCH_LOW_LATENCY_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_LOW_LATENCY_INTERVAL_MS;
          break;
      default:
          BT_RW_LOG("Unknown scan_mode of batch_scan_param in batch_start_simple_scan");
          return -1;
    }

    switch (batch_scan_param->scan_result_type) {
      case BATCH_SCAN_RESULT_TYPE_FULL:
          batch_scan_full_max = 100;
          batch_scan_trunc_max = 0;
          batch_scan_mode = BATCH_SCAN_READ_REPORT_MODE_FULL;
          break;
      case BATCH_SCAN_RESULT_TYPE_TRUNCATED:
          batch_scan_full_max = 0;
          batch_scan_trunc_max = 100;
          batch_scan_mode = BATCH_SCAN_READ_REPORT_MODE_TRUNCATED;
          break;
      default:
          BT_RW_LOG("Unknown scan_result_type of batch_scan_param in batch_start_simple_scan");
          return -1;
    }

    start_scan_param_t->scan_type = BATCH_SCAN;
    //batch_scan_param
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *start_scan_scan_param =
                         &start_scan_param_t->scan_param.batch_scan_param;
    start_scan_scan_param->batch_scan_full_max = batch_scan_full_max;
    start_scan_scan_param->batch_scan_trunc_max = batch_scan_trunc_max;
    start_scan_scan_param->batch_scan_notify_threshold = 95;
    start_scan_scan_param->batch_scan_mode = batch_scan_mode;
    start_scan_scan_param->batch_scan_windows = scan_windows;
    start_scan_scan_param->batch_scan_interval = scan_interval;
    start_scan_scan_param->own_address_type = PUBLIC_DEVICE_ADDR;
    start_scan_scan_param->batch_scan_discard_rule = BATCH_SCAN_DISCARD_OLDEST_ADV;
    start_scan_scan_param->batch_scan_read_report_mode = batch_scan_mode;

    return 0;
}


EXPORT_SYMBOL INT32 batch_start_simple_scan_with_filt(BT_BLE_SCANNER_START_SCAN_PARAM  **start_scan_param,
                                  BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM *batch_scan_with_filt_param)
{
    BT_RW_LOG("batch_start_simple_scan_with_filt");
    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param_t = *start_scan_param;

    INT32 scan_windows = 0;
    INT32 scan_interval = 0;
    UINT8 batch_scan_full_max = 0;
    UINT8 batch_scan_trunc_max = 0;
    UINT8 batch_scan_mode = 0;
    switch (batch_scan_with_filt_param->batch_scan_param.scan_mode) {
      case SCAN_MODE_LOW_POWER:
          scan_windows = SCAN_MODE_BATCH_LOW_POWER_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_LOW_POWER_INTERVAL_MS;
          break;
      case SCAN_MODE_BALANCED:
          scan_windows = SCAN_MODE_BATCH_BALANCED_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_BALANCED_INTERVAL_MS;
          break;
      case SCAN_MODE_LOW_LATENCY:
          scan_windows = SCAN_MODE_BATCH_LOW_LATENCY_WINDOW_MS;
          scan_interval = SCAN_MODE_BATCH_LOW_LATENCY_INTERVAL_MS;
          break;
      default:
          BT_RW_LOG("Unknown scan_mode of batch_scan_param in batch_start_simple_scan_with_filt");
          return -1;
    }

    switch (batch_scan_with_filt_param->batch_scan_param.scan_result_type) {
      case BATCH_SCAN_RESULT_TYPE_FULL:
          batch_scan_full_max = 100;
          batch_scan_trunc_max = 0;
          batch_scan_mode = BATCH_SCAN_READ_REPORT_MODE_FULL;
          break;
      case BATCH_SCAN_RESULT_TYPE_TRUNCATED:
          batch_scan_full_max = 0;
          batch_scan_trunc_max = 100;
          batch_scan_mode = BATCH_SCAN_READ_REPORT_MODE_TRUNCATED;
          break;
      default:
          BT_RW_LOG("Unknown scan_result_type of batch_scan_param in batch_start_simple_scan_with_filt");
          return -1;
    }

    start_scan_param_t->scan_type = BATCH_SCAN_WITH_FILT;
    //batch_scan_param
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *start_scan_scan_param =
                         &start_scan_param_t->scan_param.batch_scan_param;
    start_scan_scan_param->batch_scan_full_max = batch_scan_full_max;
    start_scan_scan_param->batch_scan_trunc_max = batch_scan_trunc_max;
    start_scan_scan_param->batch_scan_notify_threshold = 95;
    start_scan_scan_param->batch_scan_mode = batch_scan_mode;
    start_scan_scan_param->batch_scan_windows = scan_windows;
    start_scan_scan_param->batch_scan_interval = scan_interval;
    start_scan_scan_param->own_address_type = PUBLIC_DEVICE_ADDR;
    start_scan_scan_param->batch_scan_discard_rule = BATCH_SCAN_DISCARD_OLDEST_ADV;
    start_scan_scan_param->batch_scan_read_report_mode = batch_scan_mode;


    //scan_filt_data
    start_scan_param_t->scan_param.batch_scan_filt_param.scan_filt_num =
                                       batch_scan_with_filt_param->scan_filt_num;
    memcpy(start_scan_param_t->scan_param.batch_scan_filt_param.scan_filt_data,
                                    batch_scan_with_filt_param->scan_filt_data,
                                    sizeof(batch_scan_with_filt_param->scan_filt_data));
    //scan_filt_param
    BT_BLE_SCANNER_SCAN_FILT_PARAM *start_scan_filt_param =
                    start_scan_param_t->scan_param.batch_scan_filt_param.scan_filt_param;

    int size = batch_scan_with_filt_param->scan_filt_num < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM
               ? batch_scan_with_filt_param->scan_filt_num : BT_BLE_SCANNER_MAX_SCAN_FILT_NUM;
    for (int i = 0; i < size; i ++)
    {
        start_scan_filt_param[i].feat_seln= get_feature_selection(&(batch_scan_with_filt_param->scan_filt_data)[i]);
        start_scan_filt_param[i].list_logic_type= 0x00ff;
        start_scan_filt_param[i].filt_logic_type= 1;
        start_scan_filt_param[i].rssi_high_thres= 128;
        start_scan_filt_param[i].rssi_low_thres= 128;
        start_scan_filt_param[i].dely_mode= FILT_DELY_MODE_BATCHED;
        start_scan_filt_param[i].found_timeout= 0;
        start_scan_filt_param[i].lost_timeout= 0;
        start_scan_filt_param[i].found_timeout_cnt= 0;
        start_scan_filt_param[i].num_of_tracking_entries= 0;
    }

    return 0;
}


static INT32 _hndlr_bt_app_ble_scanner_event_cbk(RPC_ID_T     t_rpc_id,
                                               const CHAR*  ps_cb_type,
                                               void          *pv_cb_addr,
                                               UINT32       ui4_num_args,
                                               ARG_DESC_T*  pt_args,
                                               ARG_DESC_T*  pt_return)
{
    //BT_RW_LOG("_hndlr_bt_app_ble_reg_scanner_cbk, pv_cb_addr = %p", pv_cb_addr);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppBleScannerCbk)pv_cb_addr)((BT_BLE_SCANNER_CALLBACK_PARAM *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_scanner_register(mtkrpcapi_BtAppBleScannerCbk *func, void* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_ble_scanner_register");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_scanner_register");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_scanner_start_simple_scan(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM
                                                                             *start_simple_scan_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_scanner_start_simple_scan");
    BT_BLE_SCANNER_START_SCAN_PARAM  *start_scan_param =
            (BT_BLE_SCANNER_START_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));
    if (NULL == start_scan_param)
    {
        BT_RW_LOG("malloc start_scan_param fail!");
        return -1;
    }

    INT32 ret = 0;

    start_scan_param->scanner_id = start_simple_scan_param->scanner_id;

    switch (start_simple_scan_param->scan_type) {
        case REGULAR_SCAN:
            ret = regular_start_simple_scan(&start_scan_param,
                                            &(start_simple_scan_param->scan_param.regular_scan_param));
            break;
        case REGULAR_SCAN_WITH_FILT:
            ret = regular_start_simple_scan_with_filt(&start_scan_param,
                                            &(start_simple_scan_param->scan_param.regular_scan_filt_param));
            break;
        case BATCH_SCAN:
            ret = batch_start_simple_scan(&start_scan_param,
                                            &(start_simple_scan_param->scan_param.batch_scan_param));
            break;
        case BATCH_SCAN_WITH_FILT:
            ret = batch_start_simple_scan_with_filt(&start_scan_param,
                                            &(start_simple_scan_param->scan_param.batch_scan_filt_param));
            break;
        default:
            BT_RW_LOG("Unknown scan_type");
            ret = -1;
            break;
    }

    if (ret != 0)
    {
        BT_RW_LOG("a_mtkapi_bt_ble_scanner_start_simple_scan param error");
        if (NULL != start_scan_param)
        {
            free(start_scan_param);
            start_scan_param = NULL;
        }
        return ret;
    }

    ret = a_mtkapi_bt_ble_scanner_start_scan(start_scan_param);

    if (NULL != start_scan_param)
    {
        free(start_scan_param);
        start_scan_param = NULL;
    }

    return ret;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_scanner_start_scan(BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_scanner_start_scan");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, start_scan_param,
                                         RPC_DESC_BT_BLE_SCANNER_START_SCAN_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, start_scan_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_scanner_start_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_scanner_stop_scan(BT_BLE_SCANNER_STOP_SCAN_PARAM *stop_scan_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_scanner_stop_scan");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, stop_scan_param,
                                         RPC_DESC_BT_BLE_SCANNER_STOP_SCAN_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, stop_scan_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_scanner_stop_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_scanner_unregister(BT_BLE_SCANNER_UNREG_PARAM *unreg_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_scanner_unregister");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, unreg_param,
                                         RPC_DESC_BT_BLE_SCANNER_UNREG_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, unreg_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_scanner_unregister");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

INT32 c_rpc_reg_mtk_bt_service_ble_scanner_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_ble_scanner_event_cbk);
    return RPCR_OK;
}
