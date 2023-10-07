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
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/* FILE NAME:  mtk_bt_service_gattc.c
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

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gattc.h"
#include "c_bt_mw_gattc.h"
#include "list.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */

#define BT_RC_LOG(_stmt...) \
    do{ \
        if(1){    \
            printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)

#define BT_GATTC_ERR(_stmt...) \
    do{ \
        if(1){    \
            printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)

#define BT_MW_RPC_GATTC_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_gattc_lock)); \
    }while(0)


#define BT_MW_RPC_GATTC_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_gattc_lock));\
    }while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct {
    RPC_CB_NFY_TAG_T *pv_nfy_tag;
    INT32 client_if;
    CHAR client_name[BT_GATT_MAX_UUID_LEN];
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    struct dl_list node;
} BT_MW_GATTC_CALLBACK_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
static struct dl_list g_bt_mw_rpc_gattc_cb_list =
    {&g_bt_mw_rpc_gattc_cb_list, &g_bt_mw_rpc_gattc_cb_list};

static pthread_mutex_t g_bt_mw_rpc_gattc_lock;

static mtkrpcapi_BtAppGATTCCbk mtkrpcapi_BtGATTCEventCbk = NULL;

/* EXPORTED SUBPROGRAM BODIES
 */


VOID x_mtkapi_bt_gattc_ipc_close_notify(RPC_CB_NFY_TAG_T* pv_nfy_tag)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;
    BT_MW_GATTC_CALLBACK_CB *tmp = NULL;
    if (NULL == pv_nfy_tag) return;

    BT_MW_RPC_GATTC_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gattc_cb_list, BT_MW_GATTC_CALLBACK_CB, node)
    {
        if (NULL != cb && NULL != cb->pv_nfy_tag)
        {
            if (pv_nfy_tag->t_id == cb->pv_nfy_tag->t_id)
            {
                BT_RC_LOG("free client %d\n", cb->client_if);
                c_btm_gattc_unregister(cb->client_if);
                dl_list_del(&cb->node);
                free(cb);
                cb = NULL;
            }
        }
    }
    BT_MW_RPC_GATTC_UNLOCK();

    return;
}

/* LOCAL SUBPROGRAM BODIES
 */

INT32 mtkrpcapi_bt_alloc_client_cb(char *uuid, RPC_CB_NFY_TAG_T *pv_nfy_tag)
{
    BT_MW_GATTC_CALLBACK_CB *cb =
        (BT_MW_GATTC_CALLBACK_CB*)malloc(sizeof(BT_MW_GATTC_CALLBACK_CB));
    if (NULL == cb)
    {
        BT_GATTC_ERR("alloc fail\n");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_MW_GATTC_CALLBACK_CB));

    cb->pv_nfy_tag = pv_nfy_tag;
    memcpy(cb->client_name, uuid, BT_GATT_MAX_NAME_LEN);
    cb->client_if = 0;

    dl_list_add(&g_bt_mw_rpc_gattc_cb_list, &cb->node);

    return 0;
}

VOID mtkrpcapi_bt_free_client_cb(INT32 client_if)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;
    BT_MW_GATTC_CALLBACK_CB *tmp = NULL;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gattc_cb_list, BT_MW_GATTC_CALLBACK_CB, node)
    {
        if (client_if == cb->client_if)
        {
            dl_list_del(&cb->node);
            free(cb);
            //cb = NULL;
            return;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_client_cbs(BT_GATTC_EVENT_PARAM *param)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;
    while (!dl_list_empty(&g_bt_mw_rpc_gattc_cb_list))
    {
        cb = dl_list_first(&g_bt_mw_rpc_gattc_cb_list, BT_MW_GATTC_CALLBACK_CB, node);
        if (NULL != cb)
        {
            dl_list_del(&cb->node);

            if (mtkrpcapi_BtGATTCEventCbk)
            {
                mtkrpcapi_BtGATTCEventCbk(param, cb->pv_nfy_tag);
                ri_free_cb_tag(cb->pv_nfy_tag);
            }
            free(cb);
            cb = NULL;
        }
    }

    return;
}

VOID mtkrpcapi_bt_free_cb_by_client_name(char *client_name)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gattc_cb_list,
        BT_MW_GATTC_CALLBACK_CB, node)
    {
        if (!strncasecmp(cb->client_name,
            client_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            dl_list_del(&cb->node);
            free(cb);
            //cb = NULL;
            return;
        }
    }

    return;
}


BT_MW_GATTC_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_client_name(char *client_name)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gattc_cb_list,
        BT_MW_GATTC_CALLBACK_CB, node)
    {
        if (!strncasecmp(cb->client_name,
            client_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            return cb;
        }
    }

    return NULL;
}

BT_MW_GATTC_CALLBACK_CB* mtkrpcapi_bt_find_cb_by_client_if(INT32 client_if)
{
    BT_MW_GATTC_CALLBACK_CB *cb = NULL;

    dl_list_for_each(cb, &g_bt_mw_rpc_gattc_cb_list,
        BT_MW_GATTC_CALLBACK_CB, node)
    {
        if (client_if == cb->client_if)
        {
            return cb;
        }
    }

    return NULL;
}


VOID MWBtGATTCEventCbk(BT_GATTC_EVENT_PARAM *param)
{
    void *g_gattc_pvtag = NULL;
    BT_MW_RPC_GATTC_LOCK();
    //BT_GATTC_ERR("client_if=%d, event=%d\n", param->client_if, param->event);
    if (BT_GATTC_EVENT_REGISTER == param->event)
    {
        BT_MW_GATTC_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_client_name(param->data.register_data.client_name);
        if (NULL == cb)
        {
            BT_GATTC_ERR("callback not found by client_name=%s\n",
                param->data.register_data.client_name);
            BT_MW_RPC_GATTC_UNLOCK();
            return;
        }

        if (param->data.register_data.status == 0)
        {
            cb->client_if = param->client_if;
            g_gattc_pvtag = cb->pv_nfy_tag;
            strncpy(param->data.register_data.client_name,
                cb->client_name, BT_GATT_MAX_NAME_LEN);
            param->data.register_data.client_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';
        }
        else
        {
            BT_GATTC_ERR("register client fail, status:%d\n",
                param->data.register_data.status);

            if (mtkrpcapi_BtGATTCEventCbk)
            {
                mtkrpcapi_BtGATTCEventCbk(param, g_gattc_pvtag);
            }
            mtkrpcapi_bt_free_cb_by_client_name(param->data.register_data.client_name);
            BT_MW_RPC_GATTC_UNLOCK();
            return;
        }
    }
    else if (BT_GATTC_EVENT_MAX == param->event) /* BT off, free all client */
    {
        mtkrpcapi_bt_free_client_cbs(param);
        BT_MW_RPC_GATTC_UNLOCK();
        return;
    }
    else
    {
        BT_MW_GATTC_CALLBACK_CB *cb = NULL;
        cb = mtkrpcapi_bt_find_cb_by_client_if(param->client_if);

        if (NULL == cb)
        {
            BT_GATTC_ERR("callback not found by client_if=%d\n",
                param->client_if);
            BT_MW_RPC_GATTC_UNLOCK();
            return;
        }
        g_gattc_pvtag = cb->pv_nfy_tag;
    }

    if (NULL == g_gattc_pvtag)
    {
        BT_GATTC_ERR("MWBtGATTCEventCbk is null\n");
        BT_MW_RPC_GATTC_UNLOCK();
        return;
    }

    if (mtkrpcapi_BtGATTCEventCbk)
    {
        mtkrpcapi_BtGATTCEventCbk(param, g_gattc_pvtag);
    }
    else
    {
        BT_GATTC_ERR("MWBtGATTCEventCbk is null\n");
    }
    BT_MW_RPC_GATTC_UNLOCK();

    return;
}

VOID x_mtkapi_bt_gattc_init(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_gattc_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    return;
}

INT32 x_mtkapi_bt_gattc_register(CHAR *client_name,
    mtkrpcapi_BtAppGATTCCbk func, RPC_CB_NFY_TAG_T* pv_nfy_tag)
{
    INT32 ret = 0;
    if ((NULL == client_name) || (NULL == func))
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if ((strlen(client_name) >= BT_GATT_MAX_NAME_LEN)
        || (strlen(client_name) == 0))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    BT_RC_LOG("client_name=%s\n", client_name);

    BT_MW_RPC_GATTC_LOCK();
    if (NULL != mtkrpcapi_bt_find_cb_by_client_name(client_name))
    {
        BT_GATTC_ERR("client %s has registed\n", client_name);
        BT_MW_RPC_GATTC_UNLOCK();
        return BT_ERR_STATUS_DONE;
    }

    if (mtkrpcapi_bt_alloc_client_cb(client_name, pv_nfy_tag) < 0)
    {
        BT_GATTC_ERR("no callback resource\n");
        BT_MW_RPC_GATTC_UNLOCK();
        return BT_ERR_STATUS_NOMEM;
    }

    if (NULL == mtkrpcapi_BtGATTCEventCbk)
    {
        mtkrpcapi_BtGATTCEventCbk = func;
    }

    ret = c_btm_gattc_register(client_name, MWBtGATTCEventCbk);
    if (ret != 0)
    {
        mtkrpcapi_bt_free_cb_by_client_name(client_name);
    }
    BT_MW_RPC_GATTC_UNLOCK();
    return ret;
}


INT32 x_mtkapi_bt_gattc_unregister(INT32 client_if)
{
    BT_MW_RPC_GATTC_LOCK();
    mtkrpcapi_bt_free_client_cb(client_if);
    BT_MW_RPC_GATTC_UNLOCK();

    return c_btm_gattc_unregister(client_if);
}

INT32 x_mtkapi_bt_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param)
{
    if (NULL == conn_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }

    return c_btm_gattc_connect(conn_param);
}

INT32 x_mtkapi_bt_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param)
{
    if (NULL == disc_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_disconnect(disc_param);
}

INT32 x_mtkapi_bt_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param)
{
    if (NULL == refresh_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_refresh(refresh_param);
}

INT32 x_mtkapi_bt_gattc_discover_by_uuid(BT_GATTC_DISCOVER_BY_UUID_PARAM *discover_param)
{
    if (NULL == discover_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_discover_by_uuid(discover_param);
}

INT32 x_mtkapi_bt_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param)
{
    if (NULL == read_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_read_char(read_param);
}

INT32 x_mtkapi_bt_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param)
{
    if (NULL == read_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_read_char_by_uuid(read_param);
}

INT32 x_mtkapi_bt_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param)
{
    if (NULL == read_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_read_desc(read_param);
}


INT32 x_mtkapi_bt_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param)
{
    if (NULL == write_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_write_char(write_param);
}

INT32 x_mtkapi_bt_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param)
{
    if (NULL == write_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_write_desc(write_param);
}

INT32 x_mtkapi_bt_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param)
{
    if (NULL == exec_write_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_exec_write(exec_write_param);
}

INT32 x_mtkapi_bt_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param)
{
    if (NULL == reg_notif_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_reg_notification(reg_notif_param);
}

INT32 x_mtkapi_bt_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param)
{
    if (NULL == read_rssi_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_read_rssi(read_rssi_param);
}

INT32 x_mtkapi_bt_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param)
{
    if (NULL == get_dev_type_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_get_dev_type(get_dev_type_param);
}

INT32 x_mtkapi_bt_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *chg_mtu_param)
{
    if (NULL == chg_mtu_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_change_mtu(chg_mtu_param);
}

INT32 x_mtkapi_bt_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_update_param)
{
    if (NULL == conn_update_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_conn_update(conn_update_param);
}

INT32 x_mtkapi_bt_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *phy_set_param)
{
    if (NULL == phy_set_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_set_prefer_phy(phy_set_param);
}

INT32 x_mtkapi_bt_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param)
{
    if (NULL == read_phy_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_read_phy(read_phy_param);

}

INT32 x_mtkapi_bt_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param)
{
    if (NULL == get_gatt_db_param)
    {
        return BT_ERR_STATUS_NULL_POINTER;
    }
    return c_btm_gattc_get_gatt_db(get_gatt_db_param);
}

