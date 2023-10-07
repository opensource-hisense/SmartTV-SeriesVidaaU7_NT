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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatts.h"
#include "c_bt_mw_gatts.h"
#include "list.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

#define BT_GATTS_ERR(_stmt...) \
    do{ \
        if(1){    \
            printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)

#define BT_MW_RPC_GATTS_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_gatts_lock)); \
    }while(0)


#define BT_MW_RPC_GATTS_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_gatts_lock));\
    }while(0)


typedef struct {
    RPC_CB_NFY_TAG_T *pv_nfy_tag;
    INT32 server_if;
    CHAR server_name[BT_GATT_MAX_UUID_LEN];
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    struct dl_list node;
} BT_MW_GATTS_CALLBACK_CB;


static struct dl_list g_bt_mw_rpc_gatts_cb_list =
    {&g_bt_mw_rpc_gatts_cb_list, &g_bt_mw_rpc_gatts_cb_list};

static pthread_mutex_t g_bt_mw_rpc_gatts_lock;

static mtkrpcapi_BtAppGATTSCbk mtkrpcapi_BtGATTSEventCbk = NULL;


VOID x_mtkapi_bt_gatts_ipc_close_notify(RPC_CB_NFY_TAG_T* pv_nfy_tag)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;
    BT_MW_GATTS_CALLBACK_CB *tmp = NULL;

    if (NULL == pv_nfy_tag) return;

    BT_MW_RPC_GATTS_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gatts_cb_list, BT_MW_GATTS_CALLBACK_CB, node)
    {
        if (NULL != cb && NULL != cb->pv_nfy_tag)
        {
            if (pv_nfy_tag->t_id == cb->pv_nfy_tag->t_id)
            {
                BT_RC_LOG("free server %d\n", cb->server_if);
                c_btm_gatts_unregister(cb->server_if);
                dl_list_del(&cb->node);
                free(cb);
                cb = NULL;
            }
        }
    }
    BT_MW_RPC_GATTS_UNLOCK();

    return;
}

INT32 mtkrpcapi_bt_alloc_server_cb(char *uuid, RPC_CB_NFY_TAG_T *pv_nfy_tag)
{
    BT_MW_GATTS_CALLBACK_CB *cb =
        (BT_MW_GATTS_CALLBACK_CB*)malloc(sizeof(BT_MW_GATTS_CALLBACK_CB));
    if (NULL == cb)
    {
        BT_GATTS_ERR("alloc fail\n");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_MW_GATTS_CALLBACK_CB));

    cb->pv_nfy_tag = pv_nfy_tag;
    memcpy(cb->server_name, uuid, BT_GATT_MAX_NAME_LEN);
    cb->server_if = 0;

    dl_list_add(&g_bt_mw_rpc_gatts_cb_list, &cb->node);

    return 0;
}

VOID mtkrpcapi_bt_free_server_cb(INT32 server_if)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;
    BT_MW_GATTS_CALLBACK_CB *tmp = NULL;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gatts_cb_list, BT_MW_GATTS_CALLBACK_CB, node)
    {
        if (server_if == cb->server_if)
        {
            dl_list_del(&cb->node);
            free(cb);
            //cb = NULL;
            return;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_server_cbs(BT_GATTS_EVENT_PARAM *param)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;
    while (!dl_list_empty(&g_bt_mw_rpc_gatts_cb_list))
    {
        cb = dl_list_first(&g_bt_mw_rpc_gatts_cb_list, BT_MW_GATTS_CALLBACK_CB, node);
        if (NULL != cb)
        {
            dl_list_del(&cb->node);

            if (mtkrpcapi_BtGATTSEventCbk)
            {
                mtkrpcapi_BtGATTSEventCbk(param, cb->pv_nfy_tag);

                ri_free_cb_tag(cb->pv_nfy_tag);
            }
            free(cb);
            cb = NULL;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_cb_by_server_name(char *server_name)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gatts_cb_list,
        BT_MW_GATTS_CALLBACK_CB, node)
    {
        if (!strncasecmp(cb->server_name,
            server_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            dl_list_del(&cb->node);
            free(cb);
            //cb = NULL;
            return;
        }
    }

    return;
}


BT_MW_GATTS_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_server_name(char *server_name)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gatts_cb_list,
        BT_MW_GATTS_CALLBACK_CB, node)
    {
        if (!strncasecmp(cb->server_name,
            server_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            return cb;
        }
    }

    return NULL;
}

BT_MW_GATTS_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_server_if(INT32 server_if)
{
    BT_MW_GATTS_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gatts_cb_list,
        BT_MW_GATTS_CALLBACK_CB, node)
    {
        if (server_if == cb->server_if)
        {
            return cb;
        }
    }

    return NULL;
}



VOID MWBtGATTSEventCbk(BT_GATTS_EVENT_PARAM *param)
{
    void *g_gatts_pvtag = NULL;
    BT_MW_RPC_GATTS_LOCK();
    //BT_GATTS_ERR("server_if=%d, event=%d\n", param->server_if, param->event);
    if (BT_GATTS_EVENT_REGISTER == param->event)
    {
        BT_MW_GATTS_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_server_name(param->data.register_data.server_name);
        if (NULL == cb)
        {
            BT_GATTS_ERR("callback not found by server_name=%s\n",
                param->data.register_data.server_name);
            BT_MW_RPC_GATTS_UNLOCK();
            return;
        }

        if (param->data.register_data.status == 0)
        {
            cb->server_if = param->server_if;
            g_gatts_pvtag = cb->pv_nfy_tag;
            strncpy(param->data.register_data.server_name,
                cb->server_name, BT_GATT_MAX_NAME_LEN);
            param->data.register_data.server_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';
        }
        else
        {
            BT_GATTS_ERR("register server fail, status:%d\n",
                param->data.register_data.status);

            if (mtkrpcapi_BtGATTSEventCbk)
            {
                mtkrpcapi_BtGATTSEventCbk(param, g_gatts_pvtag);
            }
            mtkrpcapi_bt_free_cb_by_server_name(param->data.register_data.server_name);
            BT_MW_RPC_GATTS_UNLOCK();
            return;
        }
    }
    else if (BT_GATTS_EVENT_MAX == param->event) /* BT off, free all server */
    {
        mtkrpcapi_bt_free_server_cbs(param);
        BT_MW_RPC_GATTS_UNLOCK();
        return;
    }
    else
    {
        BT_MW_GATTS_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_server_if(param->server_if);

        if (NULL == cb)
        {
            BT_GATTS_ERR("callback not found by server_if=%d\n",
                param->server_if);
            BT_MW_RPC_GATTS_UNLOCK();
            return;
        }
        g_gatts_pvtag = cb->pv_nfy_tag;
    }

    if (NULL == g_gatts_pvtag)
    {
        BT_GATTS_ERR("MWBtGATTSEventCbk is null\n");
        BT_MW_RPC_GATTS_UNLOCK();
        return;
    }

    if (mtkrpcapi_BtGATTSEventCbk)
    {
        mtkrpcapi_BtGATTSEventCbk(param, g_gatts_pvtag);
    }
    else
    {
        BT_GATTS_ERR("MWBtGATTSEventCbk is null\n");
    }
    BT_MW_RPC_GATTS_UNLOCK();

    return;
}

VOID x_mtkapi_bt_gatts_init(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_gatts_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    return;
}

INT32 x_mtkapi_bt_gatts_register(CHAR *server_name,
    mtkrpcapi_BtAppGATTSCbk func, RPC_CB_NFY_TAG_T *pv_nfy_tag)
{
    INT32 ret = 0;
    if ((NULL == server_name) || (NULL == func))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if ((strlen(server_name) >= BT_GATT_MAX_NAME_LEN)
        || (strlen(server_name) == 0))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    BT_RC_LOG("server_name=%s\n", server_name);
    BT_MW_RPC_GATTS_LOCK();

    if (NULL != mtkrpcapi_bt_find_cb_by_server_name(server_name))
    {
        BT_GATTS_ERR("server %s has registed\n", server_name);
        BT_MW_RPC_GATTS_UNLOCK();
        return BT_ERR_STATUS_DONE;
    }

    if (mtkrpcapi_bt_alloc_server_cb(server_name, pv_nfy_tag) < 0)
    {
        BT_GATTS_ERR("no callback resource\n");
        BT_MW_RPC_GATTS_UNLOCK();
        return BT_ERR_STATUS_NOMEM;
    }

    if (NULL == mtkrpcapi_BtGATTSEventCbk)
    {
        mtkrpcapi_BtGATTSEventCbk = func;
    }

    ret = c_btm_gatts_register(server_name, MWBtGATTSEventCbk);
    if (ret != 0)
    {
        mtkrpcapi_bt_free_cb_by_server_name(server_name);
    }
    BT_MW_RPC_GATTS_UNLOCK();
    return ret;
}


INT32 x_mtkapi_bt_gatts_unregister(INT32 server_if)
{
    BT_MW_RPC_GATTS_LOCK();
    mtkrpcapi_bt_free_server_cb(server_if);
    BT_MW_RPC_GATTS_UNLOCK();

    return c_btm_gatts_unregister(server_if);
}

INT32 x_mtkapi_bt_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param)
{
    if (NULL == conn_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    return c_btm_gatts_connect(conn_param);
}

INT32 x_mtkapi_bt_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param)
{
    if (NULL == disc_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_disconnect(disc_param);
}

INT32 x_mtkapi_bt_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param)
{
    if (NULL == add_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_add_service(add_param);
}

INT32 x_mtkapi_bt_gatts_del_service(BT_GATTS_SERVICE_DEL_PARAM *del_param)
{
    if (NULL == del_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_delete_service(del_param);
}

INT32 x_mtkapi_bt_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param)
{
    if (NULL == ind_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_send_indication(ind_param);
}

INT32 x_mtkapi_bt_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param)
{
    if (NULL == resp_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_send_response(resp_param);
}

INT32 x_mtkapi_bt_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param)
{
    if (NULL == phy_read_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_read_phy(phy_read_param);
}

INT32 x_mtkapi_bt_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param)
{
    if (NULL == phy_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gatts_set_prefer_phy(phy_param);
}


