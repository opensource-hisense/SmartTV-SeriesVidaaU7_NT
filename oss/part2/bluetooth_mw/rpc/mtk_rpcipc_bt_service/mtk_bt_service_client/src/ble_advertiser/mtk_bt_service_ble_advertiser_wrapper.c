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

#include "mtk_bt_service_ble_advertiser_wrapper.h"
#include "mtk_bt_service_ble_advertiser_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Ble_Adv]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_ble_adv_event_cbk(RPC_ID_T     t_rpc_id,
                                               const CHAR*  ps_cb_type,
                                               void          *pv_cb_addr,
                                               UINT32       ui4_num_args,
                                               ARG_DESC_T*  pt_args,
                                               ARG_DESC_T*  pt_return)
{
    //BT_RW_LOG(", pv_cb_addr = %p", pv_cb_addr);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppBleAdvCbk)pv_cb_addr)((BT_BLE_ADV_EVENT_PARAM *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static UINT32 a_mtkapi_bt_ble_adv_convert_uuid(CHAR *uuid_str, UINT8 *uuid)
{
    INT32 uuid_str_len = 0;
    if ((NULL == uuid) || (NULL == uuid_str))
    {
        return 0;
    }

    uuid_str_len = strlen(uuid_str);
    if (uuid_str_len == 4) /* HHHH */
    {
        INT32 c = 0;
        INT32 rc = sscanf(uuid_str, "%02hhx%02hhx%n",
            &uuid[1], &uuid[0], &c);
        if (rc != 2) return 0;
        if (c != 4) return 0;
        return 2;
    }
    else if (uuid_str_len == 8) /* HHHHHHHH */
    {
        INT32 c = 0;
        INT32 rc = sscanf(uuid_str, "%02hhx%02hhx%02hhx%02hhx%n",
            &uuid[3], &uuid[2],
            &uuid[1], &uuid[0], &c);
        if (rc != 4) return 0;
        if (c != 8) return 0;
        return 4;
    }
    else /* 128b uuid */
    {
        if (uuid_str[8] != '-'
            || uuid_str[13] != '-'
            || uuid_str[18] != '-'
            || uuid_str[23] != '-') {
          return 0;
        }

        int c;
        int rc = sscanf(uuid_str,
                        "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx"
                        "-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%n",
                        &uuid[15], &uuid[14], &uuid[13], &uuid[12],
                        &uuid[11], &uuid[10], &uuid[9], &uuid[8],
                        &uuid[7], &uuid[6], &uuid[5], &uuid[4],
                        &uuid[3], &uuid[2], &uuid[1], &uuid[0], &c);
        if (rc != 16) return 0;
        if (c != 36) return 0;
        return 16;
    }
    return 0;
}



EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_start_set(
    BT_BLE_ADV_START_SET_PARAM *start_adv_set_param,
    mtkrpcapi_BtAppBleAdvCbk func,
    VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_start_set");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, start_adv_set_param,
                    RPC_DESC_BT_BLE_ADV_START_SET_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, start_adv_set_param);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_start_set");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_stop_set(BT_BLE_ADV_STOP_SET_PARAM *stop_adv_set_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_stop_set");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, stop_adv_set_param,
                    RPC_DESC_BT_BLE_ADV_STOP_SET_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, stop_adv_set_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_stop_set");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_set_param(BT_BLE_ADV_PARAM *adv_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_set_param");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, adv_param,
                    RPC_DESC_BT_BLE_ADV_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, adv_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_set_param");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_set_data(BT_BLE_ADV_DATA_PARAM *adv_data_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_set_data");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, adv_data_param,
                    RPC_DESC_BT_BLE_ADV_DATA_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, adv_data_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_set_data");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_set_scan_rsp(BT_BLE_ADV_DATA_PARAM *scan_rsp)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_set_scan_rsp");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, scan_rsp,
                    RPC_DESC_BT_BLE_ADV_DATA_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, scan_rsp);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_set_scan_rsp");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_enable(BT_BLE_ADV_ENABLE_PARAM *adv_enable_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_enable");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, adv_enable_param,
                    RPC_DESC_BT_BLE_ADV_ENABLE_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, adv_enable_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_set_periodic_param(BT_BLE_ADV_PERIODIC_PARAM *periodic_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_set_periodic_param");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, periodic_param,
                    RPC_DESC_BT_BLE_ADV_PERIODIC_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, periodic_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_set_periodic_param");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_set_periodic_data(BT_BLE_ADV_DATA_PARAM *periodic_data)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_set_periodic_data");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, periodic_data,
                    RPC_DESC_BT_BLE_ADV_DATA_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, periodic_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_set_periodic_data");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_periodic_enable(BT_BLE_ADV_PERIODIC_ENABLE_PARAM *periodic_enable_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_periodic_enable");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, periodic_enable_param,
                    RPC_DESC_BT_BLE_ADV_PERIODIC_ENABLE_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, periodic_enable_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_periodic_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_get_own_address(BT_BLE_ADV_GET_ADDR_PARAM *get_adv_addr_param)
{
    BT_RW_LOG("a_mtkapi_bt_ble_adv_get_own_address");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, get_adv_addr_param,
                    RPC_DESC_BT_BLE_ADV_GET_ADDR_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, get_adv_addr_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_ble_adv_get_own_address");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_build_name(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *local_name)
{
    UINT32 name_len = 0;
    BT_BLE_ADV_AD_TYPE ad_type = BT_BLE_ADV_AD_TYPE_COMPLETE_LOCAL_NAME;
    if ((NULL == data) || (NULL == local_name))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    name_len = strlen(local_name);
    if (name_len > BT_BLE_ADV_LOCAL_NAME_MAX_LEN)
    {
        name_len = BT_BLE_ADV_LOCAL_NAME_MAX_LEN;
        ad_type = BT_BLE_ADV_AD_TYPE_SHORTENED_LOCAL_NAME;
    }

    if (data->len + 2 + name_len > BT_BLE_ADV_MAX_ADV_DATA_LEN)
    {
        return BT_ERR_STATUS_NOMEM;
    }

    data->data[data->len++] = name_len + 1;
    data->data[data->len++] = ad_type & 0xFF;
    if (data->len + name_len <= BT_BLE_ADV_MAX_ADV_DATA_LEN
        && name_len > 0)
    {
        memcpy((VOID*)&data->data[data->len], (VOID*)local_name, name_len);
        data->len += name_len;
    }

    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_build_service_uuid(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *service_uuid_str)
{
    UINT32 uuid_len = 0;
    UINT8 uuid[16];
    BT_BLE_ADV_AD_TYPE ad_type = BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_UUIDS;
    if ((NULL == data) || (NULL == service_uuid_str))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    uuid_len = a_mtkapi_bt_ble_adv_convert_uuid(service_uuid_str, uuid);
    if (0 == uuid_len)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (data->len + 2 + uuid_len > BT_BLE_ADV_MAX_ADV_DATA_LEN)
    {
        return BT_ERR_STATUS_NOMEM;
    }

    if (uuid_len == 2)
    {
        ad_type = BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_UUIDS;
    }
    else if (uuid_len == 4)
    {
        ad_type = BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_32_BIT_SERVICE_UUIDS;
    }
    else
    {
        ad_type = BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_128_BIT_SERVICE_UUIDS;
    }

    data->data[data->len++] = uuid_len + 1;
    data->data[data->len++] = ad_type & 0xFF;
    memcpy((VOID*)&data->data[data->len], (VOID*)&uuid, uuid_len);
    data->len += uuid_len;

    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_build_tx_power(BT_BLE_ADV_DATA_PARAM *data,
    INT8 tx_power)
{
    BT_BLE_ADV_AD_TYPE ad_type = BT_BLE_ADV_AD_TYPE_TX_POWER_LEVEL;
    if (NULL == data)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if (data->len + 3 > BT_BLE_ADV_MAX_ADV_DATA_LEN)
    {
        return BT_ERR_STATUS_NOMEM;
    }

    data->data[data->len++] = 2;
    data->data[data->len++] = ad_type & 0xFF;
    data->data[data->len++] = tx_power;

    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_build_service_data(BT_BLE_ADV_DATA_PARAM *data,
    CHAR *service_uuid_str, UINT8 *sevice_data, UINT16 len)
{
    UINT32 uuid_len = 0;
    UINT32 service_data_len = 0;
    UINT8 uuid[16];
    BT_BLE_ADV_AD_TYPE ad_type = BT_BLE_ADV_AD_TYPE_SERVICE_DATA_16_BIT_UUID;
    if ((NULL == data) || (NULL == service_uuid_str))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    uuid_len = a_mtkapi_bt_ble_adv_convert_uuid(service_uuid_str, uuid);
    if (0 == uuid_len)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (NULL != sevice_data)
    {
        service_data_len = len;
    }

    if (data->len + 2 + uuid_len + service_data_len > BT_BLE_ADV_MAX_ADV_DATA_LEN)
    {
        return BT_ERR_STATUS_NOMEM;
    }

    if (uuid_len == 2)
    {
        ad_type = BT_BLE_ADV_AD_TYPE_SERVICE_DATA_16_BIT_UUID;
    }
    else if (uuid_len == 4)
    {
        ad_type = BT_BLE_ADV_AD_TYPE_SERVICE_DATA_32_BIT_UUID;
    }
    else
    {
        ad_type = BT_BLE_ADV_AD_TYPE_SERVICE_DATA_128_BIT_UUID;
    }

    data->data[data->len++] = uuid_len + service_data_len + 1;
    data->data[data->len++] = ad_type & 0xFF;
    memcpy((VOID*)&data->data[data->len], (VOID*)&uuid, uuid_len);
    data->len += uuid_len;
    if (service_data_len > 0 && sevice_data != NULL)
    {
        memcpy((VOID*)&data->data[data->len], (VOID*)sevice_data, service_data_len);
        data->len += service_data_len;
    }

    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_ble_adv_build_manu_data(BT_BLE_ADV_DATA_PARAM *data,
    UINT16 manu_id, UINT8 *manu_data, UINT16 len)
{
    UINT32 manu_data_len = 0;
    BT_BLE_ADV_AD_TYPE ad_type = BT_BLE_ADV_AD_TYPE_MANUFACTURER_SPECIFIC_DATA;
    if (NULL == data)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if (NULL != manu_data)
    {
        manu_data_len = len;
    }

    if (data->len + 2 + 2 + manu_data_len > BT_BLE_ADV_MAX_ADV_DATA_LEN)
    {
        return BT_ERR_STATUS_NOMEM;
    }

    data->data[data->len++] = manu_data_len + 1 + 2;
    data->data[data->len++] = ad_type & 0xFF;
    data->data[data->len++] = manu_id & 0xFF;
    data->data[data->len++] = (manu_id >> 8) & 0xFF;
    if (manu_data_len > 0 && manu_data != NULL)
    {
        memcpy((VOID*)&data->data[data->len], (VOID*)manu_data, manu_data_len);
        data->len += manu_data_len;
    }

    return BT_SUCCESS;
}


INT32 c_rpc_reg_mtk_bt_service_ble_adv_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_ble_adv_event_cbk);
    return RPCR_OK;
}
