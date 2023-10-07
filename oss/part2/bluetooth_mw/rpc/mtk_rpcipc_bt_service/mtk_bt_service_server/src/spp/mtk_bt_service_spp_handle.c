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
#include <stdlib.h>
#include <pthread.h>

#include "mtk_bt_service_spp.h"
#include "mtk_bt_service_spp_handle.h"
#include "mtk_bt_service_spp_ipcrpc_struct.h"
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Handle]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static struct dl_list g_bt_mw_rpc_spp_cb_list =
    {&g_bt_mw_rpc_spp_cb_list, &g_bt_mw_rpc_spp_cb_list};

static pthread_mutex_t g_bt_mw_rpc_spp_lock;
#define BT_MW_RPC_SPP_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_spp_lock)); \
    } while(0)
#define BT_MW_RPC_SPP_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_spp_lock));\
    } while(0)

static INT32 bt_spp_alloc_cb(BT_SPP_CALLBACK_INFO *info)
{
    BT_SPP_CALLBACK_NODE *cb = (BT_SPP_CALLBACK_NODE *)malloc(sizeof(BT_SPP_CALLBACK_NODE));

    if (NULL == cb)
    {
        BT_RH_LOG("alloc spp cb node fail");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_SPP_CALLBACK_NODE));
    memcpy(&cb->cb_info, info, sizeof(BT_SPP_CALLBACK_INFO));
    BT_MW_RPC_SPP_LOCK();
    dl_list_add(&g_bt_mw_rpc_spp_cb_list, &cb->node);
    BT_MW_RPC_SPP_UNLOCK();
    return 0;
}

static VOID bt_spp_free_cb(BT_SPP_CALLBACK_INFO *info)
{
    BT_SPP_CALLBACK_NODE *cb = NULL;
    BT_SPP_CALLBACK_NODE *tmp = NULL;
    BT_MW_RPC_SPP_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_spp_cb_list, BT_SPP_CALLBACK_NODE, node)
    {
        if (0 == memcmp(&cb->cb_info, info, sizeof(BT_SPP_CALLBACK_INFO)))
        {
            dl_list_del(&cb->node);
            free(cb);
            BT_MW_RPC_SPP_UNLOCK();
            return;
        }
    }
    BT_MW_RPC_SPP_UNLOCK();
}

static BOOL bt_spp_find_cb_by_tid(UINT8 t_rpc_id)
{
    BT_SPP_CALLBACK_NODE *cb = NULL;
    BT_SPP_CALLBACK_NODE *tmp = NULL;
    BT_MW_RPC_SPP_LOCK();
    if (dl_list_len(&g_bt_mw_rpc_spp_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_spp_cb_list, BT_SPP_CALLBACK_NODE, node)
        {
            BT_RH_LOG("SPP t_rpc_id %d, cb rpc_id %d\n", t_rpc_id, cb->cb_info.rpc_id);
            if (t_rpc_id == (UINT8)cb->cb_info.rpc_id)
            {
                BT_MW_RPC_SPP_UNLOCK();
                return TRUE;
            }
            else
            {
                BT_MW_RPC_SPP_UNLOCK();
                return FALSE;
            }
        }
    }
    else
    {
        BT_RH_LOG("SPP find no registered callback\n");
    }
    BT_MW_RPC_SPP_UNLOCK();
    return FALSE;
}

static VOID bt_app_spp_ipc_close_handle(VOID* pt_nfy_tag)
{
    BT_SPP_CALLBACK_NODE *cb = NULL;
    BT_SPP_CALLBACK_NODE *tmp = NULL;
    RPC_ID_T t_id = (RPC_ID_T)pt_nfy_tag;

    BT_MW_RPC_SPP_LOCK();
    if (dl_list_len(&g_bt_mw_rpc_spp_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_spp_cb_list, BT_SPP_CALLBACK_NODE, node)
        {
            if (cb->cb_info.rpc_id == t_id)
            {
                break;
            }
        }
    }
    BT_MW_RPC_SPP_UNLOCK();
    bt_spp_free_cb(&cb->cb_info);
}

static VOID bt_app_spp_event_cbk_agent(BT_SPP_CBK_STRUCT *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_spp_event_cbk_agent enter!\n");
    RPC_DECL_VOID(2);
    BT_SPP_CALLBACK_NODE *cb = NULL;
    BT_SPP_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_SPP_CBK_STRUCT, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_MW_RPC_SPP_LOCK();
    BT_RH_LOG("bt_app_spp_event_cbk_agent size=%d.\n", dl_list_len(&g_bt_mw_rpc_spp_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_spp_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_spp_cb_list, BT_SPP_CALLBACK_NODE, node)
        {
            if (param->spp_if == cb->cb_info.rpc_id)
            {
                BT_RH_LOG("current bt_spp_event_cb = %p", cb->cb_info.spp_cb_func);
                pt_nfy_tag->pv_cb_addr = cb->cb_info.spp_cb_func;
                if (NULL != pt_nfy_tag->pv_cb_addr)
                {
                    BT_RH_LOG("rpc do bt_spp_event_cb rpc_id %d.", cb->cb_info.rpc_id);
                    RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_spp_cbk", pt_nfy_tag->pv_cb_addr);
                }
                else
                {
                    BT_RH_LOG("skip NULL bt_spp_event_cb.");
                }
            }
        }
    }
    BT_MW_RPC_SPP_UNLOCK();
    BT_RH_LOG("bt_app_spp_event_cbk_agent done.");
    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_spp_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_spp_register_callback, rpc_id = %d, arg_1 = %d\n",
        t_rpc_id, pt_args[0].u.i4_arg);
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_spp_register_callback, pt_args[0].u.pv_func = %p\n",
        pt_args[0].u.pv_func);

    mtkrpcapi_BtAppSppCbk p_bt_spp_app_cb_func = NULL;
    BT_SPP_CALLBACK_INFO *p_spp_info = NULL;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};
    unsigned int list_len = 0;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    BT_MW_RPC_SPP_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_spp_cb_list);
    BT_MW_RPC_SPP_UNLOCK();

    if (0 == list_len)
    {
        apv_cb_addr[0] = (VOID*)bt_app_spp_event_cbk_agent;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, pt_args[1].u.pv_arg);
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_register_callback(bt_app_spp_event_cbk_agent, pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = BT_ERR_STATUS_FAIL;
        }
    }
    pt_return->u.i4_arg = t_rpc_id;

    BT_RH_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d\n", t_rpc_id);
    bt_register_app_ipc_close_handle(t_rpc_id, (void *)t_rpc_id, bt_app_spp_ipc_close_handle);

    p_bt_spp_app_cb_func = (mtkrpcapi_BtAppSppCbk)pt_args[0].u.pv_func;
    if (NULL == p_bt_spp_app_cb_func)
    {
        BT_RH_LOG("No BT_SPP_CALLBACK FUNC fail. \n");
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_NULL_POINTER;;
        return BT_ERR_STATUS_NULL_POINTER;
    }
    p_spp_info = (BT_SPP_CALLBACK_INFO *)malloc(sizeof(BT_SPP_CALLBACK_INFO));
    if (NULL == p_spp_info)
    {
        BT_RH_LOG("New BT_SPP_CALLBACK_INFO fail. \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    memset(p_spp_info, 0, sizeof(BT_SPP_CALLBACK_INFO));
    p_spp_info->rpc_id = t_rpc_id;
    p_spp_info->spp_cb_func = p_bt_spp_app_cb_func;
    bt_spp_alloc_cb(p_spp_info);
    if (p_spp_info)
    {
        free(p_spp_info);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_spp_unregister_callback, rpc_id = %d, arg_1 = %d\n",
        t_rpc_id, pt_args[0].u.i4_arg);
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_spp_unregister_callback, pt_args[0].u.pv_func = %p\n",
        pt_args[0].u.pv_func);
    unsigned int list_len = 0;

    mtkrpcapi_BtAppSppCbk p_bt_spp_app_cb_func = NULL;
    BT_SPP_CALLBACK_INFO *p_spp_info = NULL;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    BT_MW_RPC_SPP_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_spp_cb_list);
    BT_MW_RPC_SPP_UNLOCK();
    if (0 == list_len)
    {
        return RPCR_NOT_OPENED;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    p_bt_spp_app_cb_func = (mtkrpcapi_BtAppSppCbk)pt_args[0].u.pv_func;
    p_spp_info = (BT_SPP_CALLBACK_INFO *)malloc(sizeof(BT_SPP_CALLBACK_INFO));
    if (NULL == p_spp_info)
    {
        BT_RH_LOG("New BT_SPP_CALLBACK_INFO fail. \n");
        return -1;
    }
    memset(p_spp_info, 0, sizeof(BT_SPP_CALLBACK_INFO));
    p_spp_info->rpc_id = t_rpc_id;
    p_spp_info->spp_cb_func = p_bt_spp_app_cb_func;
    bt_spp_free_cb(p_spp_info);
    if (p_spp_info)
    {
        free(p_spp_info);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_connect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_connect\n");
    BT_SPP_CONNECT_PARAM* param = (BT_SPP_CONNECT_PARAM*)pt_args[0].u.pv_desc;

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    if (bt_spp_find_cb_by_tid(param->spp_if))
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_connect(pt_args[0].u.pv_desc);
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_PARM_INVALID;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_disconnect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_disconnect\n");
    BT_SPP_DISCONNECT_PARAM* param = (BT_SPP_DISCONNECT_PARAM*)pt_args[0].u.pv_desc;

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    if (bt_spp_find_cb_by_tid(param->spp_if))
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_disconnect(pt_args[0].u.pv_desc);
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_PARM_INVALID;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_send_data(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_send_data\n");
    BT_SPP_SEND_DATA_PARAM* param = (BT_SPP_SEND_DATA_PARAM*)pt_args[0].u.pv_desc;

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    if (bt_spp_find_cb_by_tid(param->spp_if))
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_send_data(pt_args[0].u.pv_desc);
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_PARM_INVALID;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_start_server(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_enable_devb\n");
    BT_SPP_START_SVR_PARAM* param = (BT_SPP_START_SVR_PARAM*)pt_args[0].u.pv_desc;

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    if (bt_spp_find_cb_by_tid(param->spp_if))
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_start_server(pt_args[0].u.pv_desc);
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_PARM_INVALID;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_stop_server(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_disable_devb\n");
    BT_SPP_STOP_SVR_PARAM* param = (BT_SPP_STOP_SVR_PARAM*)pt_args[0].u.pv_desc;

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    if (bt_spp_find_cb_by_tid(param->spp_if))
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_spp_stop_server(pt_args[0].u.pv_desc);
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_PARM_INVALID;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_spp_get_connection_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]x_mtkapi_spp_get_connection_info\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_spp_get_connection_info(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_spp_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_spp_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_spp_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_unregister_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_get_connection_info);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_send_data);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_start_server);
    RPC_REG_OP_HNDLR(x_mtkapi_spp_stop_server);

    return RPCR_OK;
}


