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
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "mtk_bt_service_ble_advertiser_wrapper.h"
#include "mtk_bt_service_ble_advertiser.h"
#include "c_bt_mw_ble_advertiser.h"
#include "ri_common.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

#define BT_BLE_ADV_ERR(_stmt...) \
    do{ \
        if(1){    \
            printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)


#define BT_MW_RPC_BLE_ADV_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_ble_adv_lock)); \
    }while(0)


#define BT_MW_RPC_BLE_ADV_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_ble_adv_lock));\
    }while(0)

typedef struct {
    RPC_CB_NFY_TAG_T *pv_nfy_tag;
    INT32 adv_id;
    CHAR adv_name[BT_GATT_MAX_NAME_LEN];
    struct dl_list node;
} BT_MW_BLE_ADV_CALLBACK_CB;


static mtkrpcapi_BtAppBleAdvCbk mtkrpcapi_BtBleAdvEventCbk = NULL;

static struct dl_list g_bt_mw_rpc_ble_adv_cb_list =
    {&g_bt_mw_rpc_ble_adv_cb_list, &g_bt_mw_rpc_ble_adv_cb_list};

static pthread_mutex_t g_bt_mw_rpc_ble_adv_lock;

/* LOCAL SUBPROGRAM BODIES
 */


INT32 x_mtkapi_bt_ble_adv_ipc_close_notify(RPC_CB_NFY_TAG_T *pv_nfy_tag)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
    BT_MW_BLE_ADV_CALLBACK_CB *tmp = NULL;
    BT_BLE_ADV_STOP_SET_PARAM stop_adv_set_param;

    memset(&stop_adv_set_param, 0, sizeof(stop_adv_set_param));
    BT_MW_RPC_BLE_ADV_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_ble_adv_cb_list, BT_MW_BLE_ADV_CALLBACK_CB, node)
    {
        if (NULL != cb && NULL != cb->pv_nfy_tag)
        {
            if (pv_nfy_tag->t_id == cb->pv_nfy_tag->t_id)
            {
                stop_adv_set_param.adv_id = cb->adv_id;
                BT_RC_LOG("free adv_id %d\n", cb->adv_id);
                c_btm_ble_adv_stop_adv_set(&stop_adv_set_param);
                dl_list_del(&cb->node);
                free(cb);
                cb = NULL;
            }
        }
    }
    BT_MW_RPC_BLE_ADV_UNLOCK();

    return 0;
}

INT32 mtkrpcapi_bt_alloc_ble_adv_cb(char *adv_name, RPC_CB_NFY_TAG_T* pv_nfy_tag)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb =
        (BT_MW_BLE_ADV_CALLBACK_CB*)malloc(sizeof(BT_MW_BLE_ADV_CALLBACK_CB));
    if (NULL == cb)
    {
        BT_BLE_ADV_ERR("alloc fail\n");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_MW_BLE_ADV_CALLBACK_CB));

    cb->pv_nfy_tag = pv_nfy_tag;
    memcpy(cb->adv_name, adv_name, BT_GATT_MAX_NAME_LEN);
    cb->adv_id = 0;

    dl_list_add(&g_bt_mw_rpc_ble_adv_cb_list, &cb->node);

    return 0;
}

VOID mtkrpcapi_bt_free_ble_adv_cb(INT32 adv_id)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
    BT_MW_BLE_ADV_CALLBACK_CB *tmp = NULL;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_ble_adv_cb_list, BT_MW_BLE_ADV_CALLBACK_CB, node)
    {
        if (adv_id == cb->adv_id)
        {
            dl_list_del(&cb->node);
            free(cb);
            BT_BLE_ADV_ERR("free adv_id %d\n", adv_id);
            return;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_ble_adv_cb_by_name(CHAR *adv_name)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
    BT_MW_BLE_ADV_CALLBACK_CB *tmp = NULL;

    if (NULL == adv_name) return;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_ble_adv_cb_list, BT_MW_BLE_ADV_CALLBACK_CB, node)
    {
        if (!strncmp(cb->adv_name, adv_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            dl_list_del(&cb->node);
            free(cb);
            //cb = NULL;
            BT_BLE_ADV_ERR("free adv %s\n", adv_name);
            return;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_ble_adv_cbs(BT_BLE_ADV_EVENT_PARAM *param)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
    while (!dl_list_empty(&g_bt_mw_rpc_ble_adv_cb_list))
    {
        cb = dl_list_first(&g_bt_mw_rpc_ble_adv_cb_list, BT_MW_BLE_ADV_CALLBACK_CB, node);
        if (NULL != cb)
        {
            dl_list_del(&cb->node);
            if (mtkrpcapi_BtBleAdvEventCbk)
            {
                mtkrpcapi_BtBleAdvEventCbk(param, cb->pv_nfy_tag);
            }
            free(cb);
            cb = NULL;
        }
    }

    return;
}

BT_MW_BLE_ADV_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_ble_adv_name(char *adv_name)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_ble_adv_cb_list,
        BT_MW_BLE_ADV_CALLBACK_CB, node)
    {
        if (!strncmp(cb->adv_name,
            adv_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            return cb;
        }
    }

    return NULL;
}

BT_MW_BLE_ADV_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_ble_adv_if(INT32 adv_id)
{
    BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_ble_adv_cb_list,
        BT_MW_BLE_ADV_CALLBACK_CB, node)
    {
        if (adv_id == cb->adv_id)
        {
            return cb;
        }
    }

    return NULL;
}


VOID MWBtBleAdvEventCbk(BT_BLE_ADV_EVENT_PARAM *param)
{
    void *g_ble_adv_pvtag = NULL;
    BT_BLE_ADV_ERR("adv_id=%d, event=%d\n", param->adv_id, param->event);
    BT_MW_RPC_BLE_ADV_LOCK();
    if (BT_BLE_ADV_EVENT_START_ADV_SET == param->event)
    {
        BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_ble_adv_name(param->data.start_set_data.adv_name);
        if (NULL == cb)
        {
            BT_BLE_ADV_ERR("callback not found by adv_name=%s\n",
                param->data.start_set_data.adv_name);
            BT_MW_RPC_BLE_ADV_UNLOCK();
            return;
        }

        if (param->data.start_set_data.status == 0)
        {
            cb->adv_id = param->adv_id;
            g_ble_adv_pvtag = cb->pv_nfy_tag;
            strncpy(param->data.start_set_data.adv_name,
                cb->adv_name, BT_GATT_MAX_NAME_LEN);
            param->data.start_set_data.adv_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';
        }
        else
        {
            BT_BLE_ADV_ERR("start adv set fail, status:%d\n",
                param->data.start_set_data.status);

            if (mtkrpcapi_BtBleAdvEventCbk)
            {
                mtkrpcapi_BtBleAdvEventCbk(param, cb->pv_nfy_tag);
            }
            mtkrpcapi_bt_free_ble_adv_cb_by_name(param->data.start_set_data.adv_name);
            BT_MW_RPC_BLE_ADV_UNLOCK();
            return;
        }
    }
    else if (BT_BLE_ADV_EVENT_MAX == param->event) /* BT off, free all client */
    {
        mtkrpcapi_bt_free_ble_adv_cbs(param);
        BT_MW_RPC_BLE_ADV_UNLOCK();
        return;
    }
    else
    {
        BT_MW_BLE_ADV_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_ble_adv_if(param->adv_id);

        if (NULL == cb)
        {
            BT_BLE_ADV_ERR("callback not found by adv_id=%d\n",
                param->adv_id);
            BT_MW_RPC_BLE_ADV_UNLOCK();
            return;
        }
        g_ble_adv_pvtag = cb->pv_nfy_tag;
    }

    if (NULL == g_ble_adv_pvtag)
    {
        BT_BLE_ADV_ERR("MWBtBleAdvEventCbk is null\n");
        BT_MW_RPC_BLE_ADV_UNLOCK();
        return;
    }

    if (mtkrpcapi_BtBleAdvEventCbk)
    {
        mtkrpcapi_BtBleAdvEventCbk(param, g_ble_adv_pvtag);
    }
    else
    {
        BT_BLE_ADV_ERR("MWBtBleAdvEventCbk is null\n");
    }
    BT_MW_RPC_BLE_ADV_UNLOCK();

    return;
}

VOID x_mtkapi_bt_ble_adv_init(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_ble_adv_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    return;
}

INT32 x_mtkapi_bt_ble_adv_start_set(
    BT_BLE_ADV_START_SET_PARAM *start_adv_set_param,
    mtkrpcapi_BtAppBleAdvCbk func,
    RPC_CB_NFY_TAG_T* pv_nfy_tag)
{
    INT32 ret = 0;
    if ((NULL == start_adv_set_param) || (NULL == func))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    BT_MW_RPC_BLE_ADV_LOCK();
    if (NULL != mtkrpcapi_bt_find_cb_by_ble_adv_name(start_adv_set_param->adv_name))
    {
        BT_BLE_ADV_ERR("adv %s has registed\n", start_adv_set_param->adv_name);
        BT_MW_RPC_BLE_ADV_UNLOCK();
        return BT_ERR_STATUS_DONE;
    }

    if (mtkrpcapi_bt_alloc_ble_adv_cb(start_adv_set_param->adv_name, pv_nfy_tag) < 0)
    {
        BT_BLE_ADV_ERR("no callback resource\n");
        BT_MW_RPC_BLE_ADV_UNLOCK();
        return BT_ERR_STATUS_NOMEM;
    }

    if (NULL == mtkrpcapi_BtBleAdvEventCbk)
    {
        mtkrpcapi_BtBleAdvEventCbk = func;
    }

    ret = c_btm_ble_adv_start_adv_set(start_adv_set_param, MWBtBleAdvEventCbk);
    if (ret != BT_SUCCESS)
    {
        mtkrpcapi_bt_free_ble_adv_cb_by_name(start_adv_set_param->adv_name);
    }
    BT_MW_RPC_BLE_ADV_UNLOCK();
    return ret;
}


INT32 x_mtkapi_bt_ble_adv_stop_set(BT_BLE_ADV_STOP_SET_PARAM *stop_adv_set_param)
{
    BT_MW_RPC_BLE_ADV_LOCK();
    mtkrpcapi_bt_free_ble_adv_cb(stop_adv_set_param->adv_id);
    BT_MW_RPC_BLE_ADV_UNLOCK();

    return c_btm_ble_adv_stop_adv_set(stop_adv_set_param);
}

INT32 x_mtkapi_bt_ble_adv_set_param(BT_BLE_ADV_PARAM *adv_param)
{
    return c_btm_ble_adv_set_param(adv_param);
}

INT32 x_mtkapi_bt_ble_adv_set_data(BT_BLE_ADV_DATA_PARAM *adv_data_param)
{
    return c_btm_ble_adv_set_data(adv_data_param);
}

INT32 x_mtkapi_bt_ble_adv_set_scan_rsp(BT_BLE_ADV_DATA_PARAM *scan_rsp)
{
    return c_btm_ble_adv_set_scan_rsp(scan_rsp);
}

INT32 x_mtkapi_bt_ble_adv_set_periodic_param(BT_BLE_ADV_PERIODIC_PARAM *periodic_param)
{
    return c_btm_ble_adv_set_periodic_param(periodic_param);
}

INT32 x_mtkapi_bt_ble_adv_set_periodic_data(BT_BLE_ADV_DATA_PARAM *periodic_data)
{
    return c_btm_ble_adv_set_periodic_data(periodic_data);
}

INT32 x_mtkapi_bt_ble_adv_periodic_enable(BT_BLE_ADV_PERIODIC_ENABLE_PARAM *periodic_enable_param)
{
    return c_btm_ble_adv_periodic_enable(periodic_enable_param);
}

INT32 x_mtkapi_bt_ble_adv_enable(BT_BLE_ADV_ENABLE_PARAM *adv_enable_param)
{
    return c_btm_ble_adv_enable(adv_enable_param);
}

INT32 x_mtkapi_bt_ble_adv_get_own_address(BT_BLE_ADV_GET_ADDR_PARAM *get_adv_addr_param)
{
    return c_btm_ble_adv_get_own_address(get_adv_addr_param);
}

