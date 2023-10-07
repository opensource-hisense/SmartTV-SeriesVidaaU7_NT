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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_BT_SERVICE_BLE_ADVERTISER_WRAPPER_H_
#define _MTK_BT_SERVICE_BLE_ADVERTISER_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_ble_advertiser.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppBleAdvCbk)(BT_BLE_ADV_EVENT_PARAM *param, VOID *pv_tag);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_start_set
 * PURPOSE:
 *      start an advertisement.
 * INPUT:
 *      start_adv_set_param  -- advertisement parameter
 *          .adv_name           -- adv identical name, max length is
 *                                  BT_GATT_MAX_NAME_LEN, include '\0',
 *                                  recommend: <app_name>.<adv_name>
 *          .adv_param              -- adv parameter
 *              .adv_id             -- ignore for this API.
 *              .props              -- adv properties
 *              .min_interval       -- minimum adv interval
 *              .max_interval       -- maximum adv interval
 *              .tx_power           -- tx power, only for extended adv.
 *              .primary_adv_phy    -- primary adv PHY
 *              .secondary_adv_phy  -- secondary adv PHY, only for extended adv
 *          .adv_data           -- adv data, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 *          .scan_rsp           -- scan response, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 *          .peri_param         -- periodic adv parameter
 *              .adv_id             -- ignore for this API
 *              .enable             -- this parameter is valid.
 *              .include_tx_power   -- if include tx power in the adv packet.
 *              .min_interval       -- minimum adv interval
 *              .max_interval       -- maximum adv interval
 *          .peri_data          -- periodic adv data, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 *          .duration           -- duaration for extended adv
 *          .max_ext_adv_events -- max adv events number for extended adv
 *      func                -- the adv set callback function.
 *      pv_tag              -- this will be the parameter of the callback.
 *                              It can be NULL pointer.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- start adv set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_DONE         -- the name has registered.
 *      BT_ERR_STATUS_PARM_INVALID -- some parameter is invalid
 *      BT_ERR_STATUS_NOMEM        -- no more adv set can be used, or the data
 *                                      is too long.
 *      BT_ERR_STATUS_UNSUPPORTED  -- some adv parameter don't support by the
 *                                      controller
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  APP can use a_mtkapi_bt_gap_get_local_dev_info() to get the le features to
 *  know how many adv set support, maximum adv data length, etc.
 *  adv interval recommend:
 *          low frequency: min-1600, max-1650
 *          medium frequency: min-400, max-450
 *          high frequency: min-160, max-210
 *  tx power recommend:
 *          ultra low: -21
 *          low: -15
 *          medium: -7
 *          high: 1
 *  primary_adv_phy recommend: 1M
 *  secondary_adv_phy recommend: 1M
 */
extern INT32 a_mtkapi_bt_ble_adv_start_set(
    BT_BLE_ADV_START_SET_PARAM *start_adv_set_param,
    mtkrpcapi_BtAppBleAdvCbk func,
    VOID* pv_tag);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_stop_set
 * PURPOSE:
 *      stop an advertisement and remove it from controller.
 * INPUT:
 *      stop_adv_set_param  -- stop parameter
 *          .adv_id - advertisment ID
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- stop request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv not started.
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_stop_set(BT_BLE_ADV_STOP_SET_PARAM *stop_adv_set_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_set_param
 * PURPOSE:
 *      change a advertisment parameter.
 * INPUT:
 *      adv_param              -- adv parameter
 *              .adv_id             -- must specified in this API.
 *              .props              -- adv properties
 *              .min_interval       -- minimum adv interval
 *              .max_interval       -- maximum adv interval
 *              .tx_power           -- tx power, only for extended adv.
 *              .primary_adv_phy    -- primary adv PHY
 *              .secondary_adv_phy  -- secondary adv PHY, only for extended adv
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOMEM        -- the data is too long.
 *      BT_ERR_STATUS_UNSUPPORTED  -- some adv parameter don't support by the
 *                                      controller
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *  before change parameter, APP should disable to it.
 */
extern INT32 a_mtkapi_bt_ble_adv_set_param(BT_BLE_ADV_PARAM *adv_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_set_data
 * PURPOSE:
 *      change a advertisment data.
 * INPUT:
 *      adv_data_param           -- adv data, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *      before change data, APP don't have disable it.
 */
extern INT32 a_mtkapi_bt_ble_adv_set_data(BT_BLE_ADV_DATA_PARAM *adv_data_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_set_scan_rsp
 * PURPOSE:
 *      change a scan response data, this only for scanable advertisement.
 * INPUT:
 *      scan_rsp           -- scan response, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *      before change scan response data, APP don't have to disable it.
 */
extern INT32 a_mtkapi_bt_ble_adv_set_scan_rsp(BT_BLE_ADV_DATA_PARAM *scan_rsp);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_set_periodic_param
 * PURPOSE:
 *      change a periodic advertisment parameter.
 * INPUT:
 *      peri_param         -- periodic adv parameter
 *              .adv_id             -- must specified in this API.
 *              .enable             -- this parameter is valid.
 *              .include_tx_power   -- if include tx power in the adv packet.
 *              .min_interval       -- minimum adv interval
 *              .max_interval       -- maximum adv interval
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_UNSUPPORTED  -- some adv parameter don't support by the
 *                                      controller
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *  before change parameter, APP should disable to it.
 */
extern INT32 a_mtkapi_bt_ble_adv_set_periodic_param(BT_BLE_ADV_PERIODIC_PARAM *periodic_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_set_periodic_data
 * PURPOSE:
 *      change a periodic advertisment data.
 * INPUT:
 *      adv_data_param           -- adv data, APP can use
 *                                  a_mtkapi_bt_ble_adv_build_xxx API to fill
 *                                  data into the adv_data buffer.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *      before change data, APP don't have disable it.
 */
extern INT32 a_mtkapi_bt_ble_adv_set_periodic_data(BT_BLE_ADV_DATA_PARAM *periodic_data);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_enable
 * PURPOSE:
 *      enable/disable advertisement.
 * INPUT:
 *      adv_enable_param           -- enable parameter
 *              .adv_id             -- must specified in this API.
 *              .enable             -- TRUE: enable, FALSE: disable
 *              .duration           -- duaration for extended adv
 *              .max_ext_adv_events -- max adv events number for extended adv
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_enable(BT_BLE_ADV_ENABLE_PARAM *adv_enable_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_periodic_enable
 * PURPOSE:
 *      enable/disable periodic advertisement.
 * INPUT:
 *      periodic_enable_param   -- enable parameter
 *              .adv_id             -- must specified in this API.
 *              .enable             -- TRUE: enable, FALSE: disable
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_periodic_enable(BT_BLE_ADV_PERIODIC_ENABLE_PARAM *periodic_enable_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_get_own_address
 * PURPOSE:
 *      get the advertisiment address.
 * INPUT:
 *      get_adv_addr_param   -- get adv address parameter
 *              .adv_id             -- must specified in this API.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- adv ID not exists
 *      BT_ERR_STATUS_NOT_READY    -- GATT not initialized
 *      BT_ERR_STATUS_FAIL         -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_get_own_address(BT_BLE_ADV_GET_ADDR_PARAM *get_adv_addr_param);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_build_name
 * PURPOSE:
 *      build local name into buffer.
 * INPUT:
 *      local_name   -- local name.
 * OUTPUT:
 *      data -- local name will be build in this data.
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_NOMEM        -- data buffer is not enough
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_build_name(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *local_name);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_build_service_uuid
 * PURPOSE:
 *      build service UUID into buffer.
 * INPUT:
 *      service_uuid_str   -- UUID string, like: XXXX or XXXXXXXX or
 *                              00000000-0000-0000-0000-000000000000.
 * OUTPUT:
 *      data -- local name will be build in this data.
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_NOMEM        -- data buffer is not enough
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_build_service_uuid(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *service_uuid_str);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_build_tx_power
 * PURPOSE:
 *      build tx power into buffer.
 * INPUT:
 *      tx_power   -- tx power.
 * OUTPUT:
 *      data -- tx power will be build in this data.
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_NOMEM        -- data buffer is not enough
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_build_tx_power(BT_BLE_ADV_DATA_PARAM *data,
    INT8 tx_power);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_build_service_data
 * PURPOSE:
 *      build service UUID and service data into buffer.
 * INPUT:
 *      service_uuid_str   -- service UUID string, like: XXXX or XXXXXXXX or
 *                              00000000-0000-0000-0000-000000000000.
 *      sevice_data        -- service data
 *      len                -- service data length
 * OUTPUT:
 *      data -- service UUID and service data will be build in this data.
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_NOMEM        -- data buffer is not enough
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_build_service_data(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *service_uuid_str, UINT8 *sevice_data, UINT16 len);

/* FUNCTION NAME: a_mtkapi_bt_ble_adv_build_manu_data
 * PURPOSE:
 *      build manufactor ID and data into buffer.
 * INPUT:
 *      manu_id   -- manufactor ID.
 *      manu_data -- manufactor data
 *      len       -- manufactor data length
 * OUTPUT:
 *      data -- manufactor ID and data will be build in this data.
 * RETURN:
 *      BT_SUCCESS                 -- set request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_NOMEM        -- data buffer is not enough
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_ble_adv_build_manu_data(BT_BLE_ADV_DATA_PARAM *data,
    UINT16 manu_id, UINT8 *manu_data, UINT16 len);


extern INT32 c_rpc_reg_mtk_bt_service_ble_adv_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
