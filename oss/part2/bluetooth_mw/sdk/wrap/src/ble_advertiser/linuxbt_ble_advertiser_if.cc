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

/* FILE NAME:  linuxbt_ble_adv_if.cc
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <mutex>
#include <vector>
#include <base/bind.h>
#include <base/callback.h>

#include "bluetooth.h"
#include "ble_advertiser.h"

#include "linuxbt_ble_advertiser_if.h"
#include "linuxbt_common.h"
#include "bt_mw_ble_advertiser.h"
#include "bt_mw_gap.h"
#include "bt_mw_log.h"

using bluetooth::Uuid;
using base::Bind;
using LockGuard = std::lock_guard<std::mutex>;
/* NAMING CONSTANT DECLARATIONS
 */

#define LINUBT_BLE_ADV_MAX_ADV_SET_NUM 8
/* MACRO FUNCTION DECLARATIONS
 */

#define LINUXBT_BLE_ADV_CHECK_ADDR(addr, ret) do{                                         \
        if ((NULL == (addr)) || ((MAX_BDADDR_LEN - 1) != strlen(addr)))                 \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "invalid addr(%s)", addr==NULL?"NULL":(addr));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)


#define LINUXBT_BLE_ADV_CHECK_NAME(name, ret) do{                                         \
        if ((NULL == (name)) || (0 == strlen(name)) || (BT_GATT_MAX_NAME_LEN - 1 < strlen(name)))   \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "invalid name(%s)", name==NULL?"NULL":(name));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)

#define LINUXBT_BLE_ADV_CHECK_INITED(ret)    do {                     \
        if (NULL == linuxbt_ble_adv_interface)                        \
        {                                                           \
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "ble adv not init");    \
            return ret;                                             \
        }                                                           \
    }while(0)


#define LINUXBT_BLE_ADV_SET_MSG_LEN(msg) do{  \
            msg.hdr.len = sizeof(BT_BLE_ADV_EVENT_PARAM);      \
            }while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    BOOL in_use;
    CHAR adv_name[BT_GATT_MAX_NAME_LEN];
    INT32 adv_id;
    BOOL enable;
} LINUXBT_BLE_ADV_SET_CB;

typedef struct
{
    BOOL inited;
    INT32 adv_set_cnt;
    std::vector<LINUXBT_BLE_ADV_SET_CB> adv_sets;
} LINUXBT_BLE_ADV_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void linuxbt_ble_adv_set_started_cb(int reg_id, uint8_t advertiser_id,
                                           int8_t tx_power, uint8_t status);

static void linuxbt_ble_adv_set_timeout_cb(uint8_t advertiser_id,
                                           uint8_t status) ;
static void linuxbt_ble_adv_set_adv_data_cb(UINT8 advertiser_id, UINT8 status);
static void linuxbt_ble_adv_set_scan_rsp_data_cb(UINT8 advertiser_id, UINT8 status);
static void linuxbt_ble_adv_enable_adv_set_cb(uint8_t advertiser_id,
    bool enable, uint8_t status);
static void linuxbt_ble_adv_set_adv_param_cb(uint8_t advertiser_id,
                                             uint8_t status, int8_t tx_power);
static void linuxbt_ble_adv_enable_periodic_cb(uint8_t advertiser_id, bool enable,
                                uint8_t status);
static void linuxbt_ble_adv_set_periodic_adv_param_cb(UINT8 advertiser_id, UINT8 status);
static void linuxbt_ble_adv_set_periodic_adv_data_cb(UINT8 advertiser_id, UINT8 status);

static void linuxbt_ble_adv_get_own_addr_cb(uint8_t advertiser_id, uint8_t address_type,
                            RawAddress address);

static bt_status_t linuxbt_ble_adv_parse_param(BT_BLE_ADV_PARAM &adv_param,
    AdvertiseParameters &params);

static bt_status_t linuxbt_ble_adv_check_param_data(BT_BLE_ADV_PARAM &adv_param,
    UINT32 adv_data_len, UINT32 scan_rsp_len);

static bt_status_t linuxbt_ble_adv_parse_periodic_param(BT_BLE_ADV_PERIODIC_PARAM &peri_param,
    PeriodicAdvertisingParameters &params);

static bt_status_t linuxbt_ble_adv_check_periodic_param_data(BT_BLE_ADV_PARAM &adv_param,
    BT_BLE_ADV_PERIODIC_PARAM peri_param, UINT32 periodic_data_len);

static INT32 linuxbt_ble_adv_alloc_adv_set(CHAR *adv_name);
static INT32 linuxbt_ble_adv_find_adv_set_by_name(const CHAR *adv_name);
static INT32 linuxbt_ble_adv_find_adv_set_by_id(const INT32 adv_id);
static VOID linuxbt_ble_adv_free_adv_set_by_id(const INT32 adv_id);
static VOID linuxbt_ble_adv_free_adv_set_by_name(const CHAR *adv_name);

/* STATIC VARIABLE DECLARATIONS
 */
static BleAdvertiserInterface *linuxbt_ble_adv_interface = NULL;

static std::mutex s_linuxbt_ble_adv_mutex;

static BTMW_LE_FEATURES s_linuxbt_ble_adv_le_featrues;

static LINUXBT_BLE_ADV_CB s_linuxbt_ble_adv_cb;


/* EXPORTED SUBPROGRAM BODIES
 */
int linuxbt_ble_adv_init(BleAdvertiserInterface *pt_interface)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    BT_LOCAL_DEV local_dev;

    INT32 index = 0;
    LINUXBT_BLE_ADV_SET_CB adv_set;
    LockGuard lock(s_linuxbt_ble_adv_mutex);
    if (TRUE == s_linuxbt_ble_adv_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "");
    linuxbt_ble_adv_interface = pt_interface;

    bt_mw_gap_get_local_dev_info(&local_dev);
    s_linuxbt_ble_adv_le_featrues = local_dev.le_featrues;

    if (0 == s_linuxbt_ble_adv_le_featrues.max_adv_instance)
    {
        s_linuxbt_ble_adv_le_featrues.max_adv_instance =
            LINUBT_BLE_ADV_MAX_ADV_SET_NUM;
    }

    s_linuxbt_ble_adv_cb.adv_set_cnt =
        s_linuxbt_ble_adv_le_featrues.max_adv_instance;

    s_linuxbt_ble_adv_cb.adv_sets.clear();
    memset((VOID*)&adv_set, 0, sizeof(adv_set));
    for (index = 0; index < s_linuxbt_ble_adv_cb.adv_set_cnt; index++)
    {
        s_linuxbt_ble_adv_cb.adv_sets.emplace_back(adv_set);
    }

    s_linuxbt_ble_adv_cb.inited = TRUE;

    return ret;
}

int linuxbt_ble_adv_deinit(void)
{
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LockGuard lock(s_linuxbt_ble_adv_mutex);
    if (TRUE != s_linuxbt_ble_adv_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"not inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "");
    linuxbt_ble_adv_interface = NULL;

    s_linuxbt_ble_adv_cb.adv_sets.clear();

    s_linuxbt_ble_adv_cb.inited = FALSE;

    /* notify the RPC server handle layer to clean all adv cb */
    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_MAX;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
    return BT_SUCCESS;
}

int linuxbt_ble_adv_start_adv_set(BT_BLE_ADV_START_SET_PARAM *start_adv_set_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = 0;
    AdvertiseParameters params;
    PeriodicAdvertisingParameters periodicParams;
    std::vector<uint8_t> periodic_data_vec;
    BT_CHECK_POINTER(BT_DEBUG_BLE_ADV, start_adv_set_param);
    LINUXBT_BLE_ADV_CHECK_NAME(start_adv_set_param->adv_name, BT_ERR_STATUS_PARM_INVALID);
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);
    std::string adv_name = start_adv_set_param->adv_name;

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_name=%s, adv_len=%d, rsp_len=%d, periodic_len=%d",
        start_adv_set_param->adv_name, start_adv_set_param->adv_data.len,
        start_adv_set_param->scan_rsp.len, start_adv_set_param->peri_data.len);

    index = linuxbt_ble_adv_find_adv_set_by_name(start_adv_set_param->adv_name);
    if (0 <= index)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"adv_name %s exists",
            start_adv_set_param->adv_name);
        return BT_ERR_STATUS_DONE;
    }

    index = linuxbt_ble_adv_alloc_adv_set(start_adv_set_param->adv_name);
    if (0 > index)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "alloc cb for %s fail", start_adv_set_param->adv_name);
        return BT_ERR_STATUS_NOMEM;
    }

    ret = linuxbt_ble_adv_check_param_data(start_adv_set_param->adv_param,
        start_adv_set_param->adv_data.len,
        start_adv_set_param->scan_rsp.len);
    if (ret != BT_STATUS_SUCCESS)
    {
        linuxbt_ble_adv_free_adv_set_by_name(start_adv_set_param->adv_name);
        return linuxbt_return_value_convert(ret);
    }

    memset(&params, 0, sizeof(params));
    linuxbt_ble_adv_parse_param(start_adv_set_param->adv_param, params);

    std::vector<uint8_t> data_vec(start_adv_set_param->adv_data.data,
                                     start_adv_set_param->adv_data.data +
                                     start_adv_set_param->adv_data.len);
    std::vector<uint8_t> scan_resp_vec(start_adv_set_param->scan_rsp.data,
                                     start_adv_set_param->scan_rsp.data +
                                     start_adv_set_param->scan_rsp.len);

    memset(&periodicParams, 0, sizeof(periodicParams));
    if (TRUE == start_adv_set_param->peri_param.enable)
    {
        ret = linuxbt_ble_adv_check_periodic_param_data(start_adv_set_param->adv_param,
            start_adv_set_param->peri_param, start_adv_set_param->peri_data.len);

        if (ret != BT_STATUS_SUCCESS)
        {
            linuxbt_ble_adv_free_adv_set_by_name(start_adv_set_param->adv_name);
            return linuxbt_return_value_convert(ret);
        }

        linuxbt_ble_adv_parse_periodic_param(start_adv_set_param->peri_param,
            periodicParams);
        periodic_data_vec = std::vector<uint8_t>(start_adv_set_param->peri_data.data,
                                         start_adv_set_param->peri_data.data +
                                         start_adv_set_param->peri_data.len);
    }
    else
    {
        periodicParams.enable = false;
    }

    linuxbt_ble_adv_interface->StartAdvertisingSet(
      base::Bind(&linuxbt_ble_adv_set_started_cb, index),
      params, data_vec, scan_resp_vec,
      periodicParams, periodic_data_vec,
      start_adv_set_param->duration,
      start_adv_set_param->max_ext_adv_events,
      base::Bind(linuxbt_ble_adv_set_timeout_cb));

    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_stop_adv_set(BT_BLE_ADV_STOP_SET_PARAM *stop_adv_set_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d", stop_adv_set_param->adv_id);

    index = linuxbt_ble_adv_find_adv_set_by_id(stop_adv_set_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", stop_adv_set_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_interface->Unregister(stop_adv_set_param->adv_id);

    linuxbt_ble_adv_free_adv_set_by_id(stop_adv_set_param->adv_id);

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_set_param(BT_BLE_ADV_PARAM *adv_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    AdvertiseParameters params;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d", adv_param->adv_id);

    index = linuxbt_ble_adv_find_adv_set_by_id(adv_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", adv_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_parse_param(*adv_param, params);

    linuxbt_ble_adv_interface->SetParameters(
          adv_param->adv_id, params,
          base::Bind(&linuxbt_ble_adv_set_adv_param_cb, adv_param->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_set_data(BT_BLE_ADV_DATA_PARAM *adv_data_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d, adv_len=%d",
        adv_data_param->adv_id, adv_data_param->len);

    index = linuxbt_ble_adv_find_adv_set_by_id(adv_data_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", adv_data_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    std::vector<uint8_t> data_vec(adv_data_param->data,
                                     adv_data_param->data +
                                     adv_data_param->len);

    linuxbt_ble_adv_interface->SetData(
      adv_data_param->adv_id, false, data_vec,
      base::Bind(&linuxbt_ble_adv_set_adv_data_cb, adv_data_param->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_set_scan_rsp(BT_BLE_ADV_DATA_PARAM *scan_rsp)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d, scan_rsp_len=%d",
        scan_rsp->adv_id, scan_rsp->len);

    index = linuxbt_ble_adv_find_adv_set_by_id(scan_rsp->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", scan_rsp->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    std::vector<uint8_t> scan_resp_vec(scan_rsp->data,
                                     scan_rsp->data +
                                     scan_rsp->len);

    linuxbt_ble_adv_interface->SetData(
      scan_rsp->adv_id, true, scan_resp_vec,
      base::Bind(&linuxbt_ble_adv_set_scan_rsp_data_cb, scan_rsp->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_set_periodic_param(BT_BLE_ADV_PERIODIC_PARAM *periodic_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    PeriodicAdvertisingParameters periodicParams;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d", periodic_param->adv_id);

    index = linuxbt_ble_adv_find_adv_set_by_id(periodic_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", periodic_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_parse_periodic_param(*periodic_param, periodicParams);

    linuxbt_ble_adv_interface->SetPeriodicAdvertisingParameters(
          periodic_param->adv_id, periodicParams,
          base::Bind(&linuxbt_ble_adv_set_periodic_adv_param_cb, periodic_param->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_set_periodic_data(BT_BLE_ADV_DATA_PARAM *periodic_data)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);


    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d, periodic_data_len=%d",
        periodic_data->adv_id, periodic_data->len);

    index = linuxbt_ble_adv_find_adv_set_by_id(periodic_data->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", periodic_data->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    std::vector<uint8_t> periodic_data_vec(periodic_data->data,
                                     periodic_data->data +
                                     periodic_data->len);

    linuxbt_ble_adv_interface->SetPeriodicAdvertisingData(
      periodic_data->adv_id, periodic_data_vec,
      base::Bind(&linuxbt_ble_adv_set_periodic_adv_data_cb,
          periodic_data->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_periodic_enable(BT_BLE_ADV_PERIODIC_ENABLE_PARAM *periodic_enable_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d", periodic_enable_param->adv_id);

    index = linuxbt_ble_adv_find_adv_set_by_id(periodic_enable_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", periodic_enable_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_interface->SetPeriodicAdvertisingEnable(
      periodic_enable_param->adv_id, periodic_enable_param->enable,
      base::Bind(&linuxbt_ble_adv_enable_periodic_cb,
      periodic_enable_param->adv_id, periodic_enable_param->enable));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_enable(BT_BLE_ADV_ENABLE_PARAM *adv_enable_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d enable=%d duration=%d, max_event=%d",
        adv_enable_param->adv_id, adv_enable_param->enable,
        adv_enable_param->duration, adv_enable_param->max_ext_adv_events);

    index = linuxbt_ble_adv_find_adv_set_by_id(adv_enable_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", adv_enable_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_interface->Enable(adv_enable_param->adv_id,
        adv_enable_param->enable,
        base::Bind(&linuxbt_ble_adv_enable_adv_set_cb, adv_enable_param->adv_id,
        adv_enable_param->enable),
        adv_enable_param->duration, adv_enable_param->max_ext_adv_events,
        base::Bind(&linuxbt_ble_adv_enable_adv_set_cb,
        adv_enable_param->adv_id, false));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_adv_get_own_address(BT_BLE_ADV_GET_ADDR_PARAM *get_adv_addr_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_BLE_ADV_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "adv_id=%d",
        get_adv_addr_param->adv_id);

    index = linuxbt_ble_adv_find_adv_set_by_id(get_adv_addr_param->adv_id);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "not adv_id %d", get_adv_addr_param->adv_id);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_ble_adv_interface->GetOwnAddress(
      get_adv_addr_param->adv_id,
      base::Bind(&linuxbt_ble_adv_get_own_addr_cb,
      get_adv_addr_param->adv_id));

    BT_DBG_MINOR(BT_DEBUG_BLE_ADV,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

/* LOCAL SUBPROGRAM BODIES
 */

static void linuxbt_ble_adv_set_started_cb(int reg_id, uint8_t advertiser_id,
                                           int8_t tx_power, uint8_t status) {

    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV,"reg_id %d advertiser_id:%d, status:%d",
        reg_id, advertiser_id, status);

    if (reg_id < 0 && reg_id >= s_linuxbt_ble_adv_cb.adv_set_cnt)
    {
        BT_DBG_NORMAL(BT_DEBUG_BLE_ADV,"invalid reg_id %d", reg_id);
        return;
    }

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_START_ADV_SET;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.start_set_data.status = status;
    ble_adv_msg.data.start_set_data.tx_power = tx_power;
    memcpy(ble_adv_msg.data.start_set_data.adv_name,
        s_linuxbt_ble_adv_cb.adv_sets[reg_id].adv_name,
        sizeof(s_linuxbt_ble_adv_cb.adv_sets[reg_id].adv_name));

    if (0 == status)
    {
        s_linuxbt_ble_adv_cb.adv_sets[reg_id].adv_id = advertiser_id;
        s_linuxbt_ble_adv_cb.adv_sets[reg_id].enable = TRUE;
    }
    else
    {
        linuxbt_ble_adv_free_adv_set_by_id(reg_id);
    }

    bt_mw_ble_adv_notify_app(&ble_adv_msg);

    return;
}

static void linuxbt_ble_adv_set_timeout_cb(uint8_t advertiser_id,
                                           uint8_t status) {

    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    s_linuxbt_ble_adv_cb.adv_sets[index].enable = FALSE;
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_ENABLE;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.enable_data.status = status;
    ble_adv_msg.data.enable_data.enable = FALSE;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);

    return;
}

static void linuxbt_ble_adv_set_adv_data_cb(UINT8 advertiser_id, UINT8 status)
{
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_SET_DATA;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.set_adv_data.status = status;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
}

static void linuxbt_ble_adv_set_scan_rsp_data_cb(UINT8 advertiser_id, UINT8 status)
{
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_SET_SCAN_RSP;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.set_scan_rsp_data.status = status;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
}

static void linuxbt_ble_adv_enable_adv_set_cb(uint8_t advertiser_id,
    bool enable, uint8_t status) {
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    s_linuxbt_ble_adv_cb.adv_sets[index].enable = enable;
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_ENABLE;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.enable_data.status = status;
    ble_adv_msg.data.enable_data.enable = enable;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);

    return;
}

static void linuxbt_ble_adv_set_adv_param_cb(uint8_t advertiser_id,
                                             uint8_t status, int8_t tx_power) {

    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d tx_power=%d",
        advertiser_id, status, tx_power);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_SET_PARAM;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.set_param_data.status = status;
    ble_adv_msg.data.set_param_data.tx_power = tx_power;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);

    return;
}

static void linuxbt_ble_adv_enable_periodic_cb(uint8_t advertiser_id, bool enable,
                                uint8_t status) {
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_ENABLE_PERI;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.enable_periodic_data.status = status;
    ble_adv_msg.data.enable_periodic_data.enable = enable;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);

    return;
}

static void linuxbt_ble_adv_set_periodic_adv_param_cb(UINT8 advertiser_id, UINT8 status)
{
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_SET_PERI_PARAM;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.set_periodic_param_data.status = status;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
}

static void linuxbt_ble_adv_set_periodic_adv_data_cb(UINT8 advertiser_id, UINT8 status)
{
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d status=%d",
        advertiser_id, status);

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_SET_PERI_DATA;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.set_periodic_data.status = status;

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
}

static void linuxbt_ble_adv_get_own_addr_cb(uint8_t advertiser_id, uint8_t address_type,
                            RawAddress address) {
    INT32 index = -1;
    BT_BLE_ADV_EVENT_PARAM ble_adv_msg;
    LINUXBT_BLE_ADV_CHECK_INITED();
    LockGuard lock(s_linuxbt_ble_adv_mutex);

    if (0 > (index = linuxbt_ble_adv_find_adv_set_by_id(advertiser_id)))
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV,"cannot find advertiser_id: %d",
            advertiser_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "advertiser_id=%d address_type=%d address=%s",
        advertiser_id, address_type, address.ToString().c_str());

    memset((void*)&ble_adv_msg, 0, sizeof(ble_adv_msg));
    ble_adv_msg.event = BT_BLE_ADV_EVENT_GET_ADDR;
    ble_adv_msg.adv_id = advertiser_id;
    ble_adv_msg.data.get_addr_data.address_type = address_type;

    strncpy(ble_adv_msg.data.get_addr_data.addr, address.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    ble_adv_msg.data.get_addr_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_ble_adv_notify_app(&ble_adv_msg);
}

static bt_status_t linuxbt_ble_adv_parse_param(BT_BLE_ADV_PARAM &adv_param,
    AdvertiseParameters &params)
{
    UINT16 props = 0;
    if (adv_param.props.connectable) props |= 0x01;
    if (adv_param.props.scannable) props |= 0x02;
    if (adv_param.props.legacy) props |= 0x10;
    if (adv_param.props.anonymous) props |= 0x20;
    if (adv_param.props.include_tx_power) props |= 0x40;

    params.advertising_event_properties = props;
    params.min_interval = adv_param.min_interval;
    params.max_interval = adv_param.max_interval;
    params.channel_map = 0x07; /* all channels */
    params.tx_power = adv_param.tx_power;
    params.primary_advertising_phy = adv_param.primary_adv_phy;
    params.secondary_advertising_phy = adv_param.secondary_adv_phy;
    params.scan_request_notification_enable = false;

    return BT_STATUS_SUCCESS;
}


static bt_status_t linuxbt_ble_adv_check_param_data(BT_BLE_ADV_PARAM &adv_param,
    UINT32 adv_data_len, UINT32 scan_rsp_len)
{
    if (adv_param.props.connectable)
    {
        adv_data_len += 3; /* Flag length, stack will add it */
    }

    if (adv_param.min_interval == adv_param.max_interval)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "min int(%u) shouldnot equal to max int(%u)",
            adv_param.min_interval, adv_param.max_interval);
        return BT_STATUS_PARM_INVALID;
    }

    if (adv_param.props.legacy)
    {
        if (adv_data_len > BT_BLE_ADV_MAX_LEGACY_DATA_LEN)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "adv data too long (%d)",
                adv_data_len);
            return BT_STATUS_NOMEM;
        }

        if (adv_param.props.scannable)
        {
            if (scan_rsp_len > BT_BLE_ADV_MAX_LEGACY_DATA_LEN)
            {
                BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "scan rsp too long (%d)",
                    scan_rsp_len);
                return BT_STATUS_NOMEM;
            }
        }

        /* don't support connectable direct adv */
        if (TRUE == adv_param.props.connectable
            && FALSE == adv_param.props.scannable)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "legacy adv don't support connect direct on this stack");
            return BT_STATUS_UNSUPPORTED;
        }

        if (BT_BLE_ADV_PHY_1M != adv_param.primary_adv_phy)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "legacy adv only support 1M phy");
            return BT_STATUS_UNSUPPORTED;
        }
    }
    else
    {
        if (FALSE == s_linuxbt_ble_adv_le_featrues.le_extended_advertising_supported)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "extended adv don't support");
            return BT_STATUS_UNSUPPORTED;
        }

        if (BT_BLE_ADV_PHY_2M == adv_param.primary_adv_phy)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "primary phy don't support 2M phy");
            return BT_STATUS_UNSUPPORTED;
        }

        if (FALSE == s_linuxbt_ble_adv_le_featrues.le_2m_phy_supported
            && (BT_BLE_ADV_PHY_2M == adv_param.secondary_adv_phy))
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "don't support 2M phy");
            return BT_STATUS_UNSUPPORTED;
        }

        if (FALSE == s_linuxbt_ble_adv_le_featrues.le_coded_phy_supported
            && ((BT_BLE_ADV_PHY_LE_CODED == adv_param.primary_adv_phy)
                || (BT_BLE_ADV_PHY_LE_CODED == adv_param.secondary_adv_phy)))
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "don't support le coded phy");
            return BT_STATUS_UNSUPPORTED;
        }

        /* Extended adv don't support this type */
        if (TRUE == adv_param.props.connectable
            && TRUE == adv_param.props.scannable)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "extended adv don't support conn+scan");
            return BT_STATUS_UNSUPPORTED;
        }

        if (adv_data_len > s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "adv data too long (%d) max len %u",
                adv_data_len,
                s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length);
            return BT_STATUS_NOMEM;
        }

        if (adv_param.props.scannable)
        {
            if (scan_rsp_len > s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length)
            {
                BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "scan rsp too long (%d) max len %u",
                    scan_rsp_len,
                    s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length);
                return BT_STATUS_NOMEM;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t linuxbt_ble_adv_parse_periodic_param(BT_BLE_ADV_PERIODIC_PARAM &peri_param,
    PeriodicAdvertisingParameters &params)
{
    params.enable = true;
    params.min_interval = peri_param.min_interval;
    params.max_interval = peri_param.max_interval;
    UINT16 props = 0;
    if (peri_param.include_tx_power) props |= 0x40;
    params.periodic_advertising_properties = props;

    return BT_STATUS_SUCCESS;
}

static bt_status_t linuxbt_ble_adv_check_periodic_param_data(BT_BLE_ADV_PARAM &adv_param,
    BT_BLE_ADV_PERIODIC_PARAM peri_param, UINT32 periodic_data_len)
{
    if (FALSE == peri_param.enable)
    {
        return BT_STATUS_SUCCESS;
    }

    if (adv_param.props.legacy)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "periodic adv should be extended adv");
        return BT_STATUS_PARM_INVALID;
    }

    if (TRUE == adv_param.props.connectable
        || TRUE == adv_param.props.scannable)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "periodic adv should non-conn & non-scan");
        return BT_STATUS_PARM_INVALID;
    }

    if (periodic_data_len > s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_ADV, "periodic adv data too long (%d) max len %u",
            periodic_data_len,
            s_linuxbt_ble_adv_le_featrues.le_maximum_advertising_data_length);
        return BT_STATUS_NOMEM;
    }

    return BT_STATUS_SUCCESS;
}

static INT32 linuxbt_ble_adv_alloc_adv_set(CHAR *adv_name)
{
    INT32 i = 0;

    for (i = 0; i < s_linuxbt_ble_adv_cb.adv_set_cnt; i++)
    {
        if (s_linuxbt_ble_adv_cb.adv_sets[i].in_use == FALSE)
        {
            s_linuxbt_ble_adv_cb.adv_sets[i].in_use = TRUE;
            strncpy(s_linuxbt_ble_adv_cb.adv_sets[i].adv_name,
                adv_name, BT_GATT_MAX_NAME_LEN - 1);
            s_linuxbt_ble_adv_cb.adv_sets[i].adv_name[BT_GATT_MAX_NAME_LEN-1] = '\0';
            return i;
        }
    }
    return -1;
}

static INT32 linuxbt_ble_adv_find_adv_set_by_name(const CHAR *adv_name)
{
    int i = 0;

    for (i = 0; i < s_linuxbt_ble_adv_cb.adv_set_cnt; i++)
    {
        if ((s_linuxbt_ble_adv_cb.adv_sets[i].in_use == TRUE)
            && (0 == strncasecmp(s_linuxbt_ble_adv_cb.adv_sets[i].adv_name,
                                adv_name, BT_GATT_MAX_NAME_LEN - 1)))
        {
            return i;
        }
    }
    return -1;
}


static INT32 linuxbt_ble_adv_find_adv_set_by_id(const INT32 adv_id)
{
    int i = 0;

    for (i = 0; i < s_linuxbt_ble_adv_cb.adv_set_cnt; i++)
    {
        if ((s_linuxbt_ble_adv_cb.adv_sets[i].in_use == TRUE)
            && (s_linuxbt_ble_adv_cb.adv_sets[i].adv_id == adv_id))
        {
            return i;
        }
    }
    return -1;
}


static VOID linuxbt_ble_adv_free_adv_set_by_id(const INT32 adv_id)
{
    int i = 0;

    for (i = 0; i < s_linuxbt_ble_adv_cb.adv_set_cnt; i++)
    {
        if ((s_linuxbt_ble_adv_cb.adv_sets[i].in_use == TRUE)
            && (s_linuxbt_ble_adv_cb.adv_sets[i].adv_id == adv_id))
        {
            s_linuxbt_ble_adv_cb.adv_sets[i].in_use = FALSE;
            s_linuxbt_ble_adv_cb.adv_sets[i].adv_id = 0;
            s_linuxbt_ble_adv_cb.adv_sets[i].enable = FALSE;
            s_linuxbt_ble_adv_cb.adv_sets[i].adv_name[0] = '\0';

            BT_DBG_NORMAL(BT_DEBUG_BLE_ADV, "free %d", adv_id);
            return;
        }
    }

    return;
}

static VOID linuxbt_ble_adv_free_adv_set_by_name(const CHAR *adv_name)
{
    int i = 0;

    for (i = 0; i < s_linuxbt_ble_adv_cb.adv_set_cnt; i++)
    {
        if ((s_linuxbt_ble_adv_cb.adv_sets[i].in_use == TRUE)
            && (0 == strncasecmp(s_linuxbt_ble_adv_cb.adv_sets[i].adv_name,
                                adv_name, BT_GATT_MAX_NAME_LEN - 1)))
        {
            s_linuxbt_ble_adv_cb.adv_sets[i].in_use = 0;
            s_linuxbt_ble_adv_cb.adv_sets[i].adv_id = 0;
            s_linuxbt_ble_adv_cb.adv_sets[i].adv_name[0] = '\0';
            s_linuxbt_ble_adv_cb.adv_sets[i].enable = FALSE;
            return;
        }
    }
    return;

}


