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
#include <stdlib.h>
#include <pthread.h>

#include "mtk_bt_service_leaudio_bass.h"
#include "mtk_bt_service_leaudio_bass_ipcrpc_struct.h"
#include "mtk_bt_service_leaudio_bass_handle.h"
#include "mtk_bt_service_common.h"
#include "u_rpc.h"
#include "ri_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static BOOL g_bt_bass_agent_cbk_registered = FALSE;
static struct dl_list g_bt_mw_rpc_bass_cb_list =
    {&g_bt_mw_rpc_bass_cb_list, &g_bt_mw_rpc_bass_cb_list};
static pthread_mutex_t g_bt_mw_rpc_bass_lock;

#define BT_MW_RPC_BASS_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_bass_lock)); \
    } while(0)

#define BT_MW_RPC_BASS_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_bass_lock));\
    } while(0)

static INT32 bt_bass_alloc_cb(BT_BASS_CALLBACK_INFO *info)
{
    BT_BASS_CALLBACK_NODE *cb = (BT_BASS_CALLBACK_NODE *)malloc(sizeof(BT_BASS_CALLBACK_NODE));

    if (NULL == cb)
    {
        BT_RH_LOG("alloc fail");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_BASS_CALLBACK_NODE));
    memcpy(&cb->cb_info, info, sizeof(BT_BASS_CALLBACK_INFO));
    BT_MW_RPC_BASS_LOCK();
    dl_list_add(&g_bt_mw_rpc_bass_cb_list, &cb->node);
    BT_MW_RPC_BASS_UNLOCK();
    return 0;
}

static VOID bt_bass_free_cb(BT_BASS_CALLBACK_INFO *info)
{
    BT_BASS_CALLBACK_NODE *cb = NULL;
    BT_BASS_CALLBACK_NODE *tmp = NULL;

    BT_MW_RPC_BASS_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_bass_cb_list, BT_BASS_CALLBACK_NODE, node)
    {
        if (0 == memcmp(&cb->cb_info, info, sizeof(BT_BASS_CALLBACK_INFO)))
        {
            dl_list_del(&cb->node);
            free(cb);
            BT_MW_RPC_BASS_UNLOCK();
            return;
        }
    }
    BT_MW_RPC_BASS_UNLOCK();
}

static VOID bt_app_bass_event_cbk_agent(BT_BASS_EVENT_PARAM *param, void* pv_tag)
{
    RPC_DECL_VOID(2);
    BT_BASS_CALLBACK_NODE *cb = NULL;
    BT_BASS_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param, RPC_DESC_BT_BASS_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_MW_RPC_BASS_LOCK();
    BT_RH_LOG("bt_app_bass_event_cbk_agent size=%d.", dl_list_len(&g_bt_mw_rpc_bass_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_bass_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_bass_cb_list, BT_BASS_CALLBACK_NODE, node)
        {
            BT_RH_LOG("current app_func = %p.", cb->cb_info.app_func);
            pt_nfy_tag->apv_cb_addr_ex[0] = cb->cb_info.app_func;
            if (NULL != pt_nfy_tag->apv_cb_addr_ex[0])
            {
                RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_bass_event_cbk", pt_nfy_tag->apv_cb_addr_ex[0]);
            }
            else
            {
                BT_RH_LOG("skip NULL app_func.");
            }
        }
    }
    BT_MW_RPC_BASS_UNLOCK();

    BT_RH_LOG("bt_app_bass_event_cbk_agent done.");
    RPC_RETURN_VOID;
}

static VOID bt_app_bass_ipc_close_handle(VOID* pv_tag)
{
    BT_BASS_CALLBACK_INFO *bass_info = (BT_BASS_CALLBACK_INFO *)pv_tag;

    BT_RH_LOG("[_hndlr_] free bass_info(0x%p) rpc_id = %d\n", bass_info, bass_info->rpc_id);
    bt_bass_free_cb(bass_info);
    free(bass_info);
}

static INT32 _hndlr_x_mtkapi_bt_bass_connect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_connect((BT_BASS_CONN_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_disconnect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_disconnect((BT_BASS_DISC_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_set_broadcast_scan(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_set_broadcast_scan((BT_BASS_BROADCAST_SCAN_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_stop_broadcast_observing(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_stop_broadcast_observing();
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_get_broadcast_receiver_state(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_get_broadcast_receiver_state((BT_BASS_BROADCAST_RECV_STATE_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_set_broadcast_code(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_set_broadcast_code((BT_BASS_BROADCAST_CODE_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_set_broadcast_source(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_set_broadcast_source((BT_BASS_SET_BROADCAST_SRC_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_modify_broadcast_source(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_modify_broadcast_source((BT_BASS_MODIFY_BROADCAST_SRC_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_remove_broadcast_source(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_remove_broadcast_source((BT_BASS_REMOVE_BROADCAST_SRC_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_set_builtin_mode(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_set_builtin_mode((BT_BASS_SET_BUILTIN_MODE_PARAM *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_get_connection_status(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bass_get_connection_status((BT_BASS_CONNECTION_INFO *)pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bass_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    RPC_CB_NFY_TAG_T *pt_nfy_tag = NULL;
    VOID *apv_cb_addr[1] = {0};
    BT_BASS_CALLBACK_INFO *p_bass_info = NULL;
    unsigned int list_len = 0;

    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("[_hndlr_]bt_bass_register_callback, rpc_id=%d, arg_0 = %p\n", t_rpc_id, pt_args[0].u.pv_func);
    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    BT_MW_RPC_BASS_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_bass_cb_list);
    BT_MW_RPC_BASS_UNLOCK();
    if ((0 == list_len) && (FALSE == g_bt_bass_agent_cbk_registered))
    {
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, pt_args[1].u.pv_arg);
        pt_return->u.i4_arg = x_mtkapi_bt_bass_register_callback(bt_app_bass_event_cbk_agent, pt_nfy_tag);
        if (0 == pt_return->u.i4_arg)
        {
            g_bt_bass_agent_cbk_registered = TRUE;
        }
        else if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }

    p_bass_info = (BT_BASS_CALLBACK_INFO *)malloc(sizeof(BT_BASS_CALLBACK_INFO));
    if (NULL == p_bass_info)
    {
        BT_RH_LOG("New BT_BASS_CALLBACK_INFO fail. \n");
        return -1;
    }
    memset(p_bass_info, 0, sizeof(BT_BASS_CALLBACK_INFO));
    p_bass_info->rpc_id = t_rpc_id;
    p_bass_info->app_func = (mtkrpcapi_BtAppBassCbk)pt_args[0].u.pv_desc;
    bt_bass_alloc_cb(p_bass_info);

    BT_RH_LOG("bt_register_app_ipc_close_handle. pv_tag(%p) rpc_id = %d", p_bass_info, p_bass_info->rpc_id);
    bt_register_app_ipc_close_handle(t_rpc_id, p_bass_info, bt_app_bass_ipc_close_handle);
    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_bass_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_bass_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    g_bt_bass_agent_cbk_registered = FALSE;

    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_set_broadcast_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_stop_broadcast_observing);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_get_broadcast_receiver_state);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_set_broadcast_code);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_set_broadcast_source);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_modify_broadcast_source);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_remove_broadcast_source);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_set_builtin_mode);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_get_connection_status);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bass_register_callback);
    return RPCR_OK;
}
