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

#include "mtk_bt_service_hidh.h"
#include "mtk_bt_service_hidh_handle.h"
#include "mtk_bt_service_hidh_ipcrpc_struct.h"
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[HIDH]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static struct dl_list g_bt_mw_rpc_hidh_cb_list =
        {&g_bt_mw_rpc_hidh_cb_list, &g_bt_mw_rpc_hidh_cb_list};

static pthread_mutex_t g_bt_mw_rpc_hidh_lock;
#define BT_MW_RPC_HIDH_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_hidh_lock)); \
    } while(0)
#define BT_MW_RPC_HIDH_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_hidh_lock)); \
    } while(0)

static INT32 bt_hidh_alloc_cb(BT_HIDH_CALLBACK_INFO *info)
{
    BT_HIDH_CALLBACK_NODE *cb = (BT_HIDH_CALLBACK_NODE *)malloc(sizeof(BT_HIDH_CALLBACK_NODE));

    if (NULL == cb)
    {
        BT_RH_LOG("alloc hidh cb node fail");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_HIDH_CALLBACK_NODE));
    memcpy(&cb->cb_info, info, sizeof(BT_HIDH_CALLBACK_INFO));
    BT_MW_RPC_HIDH_LOCK();
    dl_list_add(&g_bt_mw_rpc_hidh_cb_list, &cb->node);
    BT_MW_RPC_HIDH_UNLOCK();
    return 0;
}

static VOID bt_hidh_free_cb(BT_HIDH_CALLBACK_INFO *info)
{
    BT_HIDH_CALLBACK_NODE *cb = NULL;
    BT_HIDH_CALLBACK_NODE *tmp = NULL;

    BT_MW_RPC_HIDH_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_hidh_cb_list, BT_HIDH_CALLBACK_NODE, node)
    {
        if (0 == memcmp(&cb->cb_info, info, sizeof(BT_HIDH_CALLBACK_INFO)))
        {
            dl_list_del(&cb->node);
            free(cb);
            BT_MW_RPC_HIDH_UNLOCK();
            return;
        }
    }
    BT_MW_RPC_HIDH_UNLOCK();
}

static VOID bt_app_hidh_ipc_close_handle(void* pt_nfy_tag)
{
    BT_HIDH_CALLBACK_NODE *cb = NULL;
    BT_HIDH_CALLBACK_NODE *tmp = NULL;
    RPC_ID_T t_id = (RPC_ID_T)pt_nfy_tag;
    BT_RH_LOG("bt_app_hidh_ipc_close_handle enter, t_id %d!\n", t_id);

    BT_MW_RPC_HIDH_LOCK();
    if (dl_list_len(&g_bt_mw_rpc_hidh_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_hidh_cb_list, BT_HIDH_CALLBACK_NODE, node)
        {
            if (cb->cb_info.rpc_id == t_id)
            {
                break;
            }
        }
    }
    BT_MW_RPC_HIDH_UNLOCK();
    bt_hidh_free_cb(&cb->cb_info);
}

static VOID bt_app_hidh_event_cbk_agent(BT_HIDH_CBK_STRUCT *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_hidh_event_cbk_agent enter!\n");
    RPC_DECL_VOID(2);
    BT_HIDH_CALLBACK_NODE *cb = NULL;
    BT_HIDH_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param, RPC_DESC_BT_HIDH_CBK_STRUCT, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    switch (param->event)
    {
    case BT_HIDH_CONNECTED:
        BT_RH_LOG("bt_app_hidh_event_cbk_agent %s connected", param->addr);
        break;
    case BT_HIDH_DISCONNECTED:
        BT_RH_LOG("bt_app_hidh_event_cbk_agent %s disconnected", param->addr);
        break;
    default:
        BT_RH_LOG("bt_app_hidh_event_cbk_agent ignore\n");
        break;
    }

    BT_MW_RPC_HIDH_LOCK();
    BT_RH_LOG("bt_app_hidh_event_cbk_agent size=%d.\n", dl_list_len(&g_bt_mw_rpc_hidh_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_hidh_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_hidh_cb_list, BT_HIDH_CALLBACK_NODE, node)
        {
            BT_RH_LOG("current bt_hidh_event_cb = %p", cb->cb_info.hidh_cb_func);
            pt_nfy_tag->pv_cb_addr = cb->cb_info.hidh_cb_func;
            if (NULL != pt_nfy_tag->pv_cb_addr)
            {
                BT_RH_LOG("rpc do bt_hidh_event_cb rpc_id %d.", cb->cb_info.rpc_id);
                RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_hidh_event_cbk", pt_nfy_tag->pv_cb_addr);
            }
            else
            {
                BT_RH_LOG("skip NULL bt_hidh_event_cb.");
            }
        }
    }
    BT_MW_RPC_HIDH_UNLOCK();
    BT_RH_LOG("bt_app_hidh_event_cbk_agent done.");
    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_hidh_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_register_callback, rpc_id = %d, arg_1 = %d\n",
        t_rpc_id, pt_args[0].u.i4_arg);
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_register_callback, pt_args[0].u.pv_func = %p\n",
        pt_args[0].u.pv_func);
    unsigned int list_len = 0;

    mtkrpcapi_BtAppHidhCbk p_bt_hidh_app_cb_func = NULL;
    BT_HIDH_CALLBACK_INFO *p_hidh_info = NULL;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    BT_MW_RPC_HIDH_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_hidh_cb_list);
    BT_MW_RPC_HIDH_UNLOCK();

    if (0 == list_len)
    {
        apv_cb_addr[0] = (VOID*)bt_app_hidh_event_cbk_agent;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, pt_args[1].u.pv_arg);
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_hidh_register_callback(bt_app_hidh_event_cbk_agent, pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }

    p_bt_hidh_app_cb_func = (mtkrpcapi_BtAppHidhCbk)pt_args[0].u.pv_func;
    p_hidh_info = (BT_HIDH_CALLBACK_INFO *)malloc(sizeof(BT_HIDH_CALLBACK_INFO));
    if (NULL == p_hidh_info)
    {
        BT_RH_LOG("New BT_HIDH_CALLBACK_INFO fail. \n");
        return -1;
    }
    BT_RH_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d\n", t_rpc_id );
    bt_register_app_ipc_close_handle(t_rpc_id, (void *)t_rpc_id, bt_app_hidh_ipc_close_handle);
    memset(p_hidh_info, 0, sizeof(BT_HIDH_CALLBACK_INFO));
    p_hidh_info->rpc_id = t_rpc_id;
    p_hidh_info->hidh_cb_func = p_bt_hidh_app_cb_func;
    bt_hidh_alloc_cb(p_hidh_info);
    if (p_hidh_info)
    {
        free(p_hidh_info);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_unregister_callback, rpc_id = %d, arg_1 = %d\n",
        t_rpc_id, pt_args[0].u.i4_arg);
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_unregister_callback, pt_args[0].u.pv_func = %p\n",
        pt_args[0].u.pv_func);

    mtkrpcapi_BtAppHidhCbk p_bt_hidh_app_cb_func = NULL;
    BT_HIDH_CALLBACK_INFO *p_hidh_info = NULL;
    unsigned int list_len = 0;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    BT_MW_RPC_HIDH_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_hidh_cb_list);
    BT_MW_RPC_HIDH_UNLOCK();
    if (0 == list_len)
    {
        return RPCR_NOT_OPENED;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    p_bt_hidh_app_cb_func = (mtkrpcapi_BtAppHidhCbk)pt_args[0].u.pv_func;
    p_hidh_info = (BT_HIDH_CALLBACK_INFO *)malloc(sizeof(BT_HIDH_CALLBACK_INFO));
    if (NULL == p_hidh_info)
    {
        BT_RH_LOG("New BT_HIDH_CALLBACK_INFO fail. \n");
        return -1;
    }
    memset(p_hidh_info, 0, sizeof(BT_HIDH_CALLBACK_INFO));
    p_hidh_info->rpc_id = t_rpc_id;
    p_hidh_info->hidh_cb_func = p_bt_hidh_app_cb_func;
    bt_hidh_free_cb(p_hidh_info);
    if (p_hidh_info)
    {
        free(p_hidh_info);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_connect(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_connect, arg_1 = %s\n", pt_args[0].u.pc_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_connect(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_disconnect(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_disconnect, arg_1 = %s\n", pt_args[0].u.pc_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_disconnect(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_set_output_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_set_output_report, arg_1 = %s, arg_2 = %s\n",
        pt_args[0].u.pc_arg, pt_args[1].u.pc_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_set_output_report(pt_args[0].u.pc_arg, pt_args[1].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_get_input_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_get_input_report, arg_1 = %s, arg_2 = %d, arg_3 = %d\n",
        pt_args[0].u.pc_arg, pt_args[1].u.ui1_arg, pt_args[2].u.i4_arg);

    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_get_input_report(pt_args[0].u.pc_arg,
                                                         pt_args[1].u.ui1_arg,
                                                         pt_args[2].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_get_output_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_get_output_report, arg_1 = %s, arg_2 = %d, arg_3 = %d\n",
        pt_args[0].u.pc_arg, pt_args[1].u.ui1_arg, pt_args[2].u.i4_arg);

    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_get_output_report(pt_args[0].u.pc_arg,
                                                          pt_args[1].u.ui1_arg,
                                                          pt_args[2].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_set_input_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_set_input_report, arg_1 = %s, arg_2 = %s\n",
       pt_args[0].u.pc_arg, pt_args[1].u.pc_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_set_input_report(pt_args[0].u.pc_arg,
                                                         pt_args[1].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_get_feature_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_get_feature_report, arg_1 = %s, arg_2 = %d, arg_3 = %d\n",
        pt_args[0].u.pc_arg, pt_args[1].u.ui1_arg, pt_args[2].u.i4_arg);

    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_get_feature_report(pt_args[0].u.pc_arg,
                                                           pt_args[1].u.ui1_arg,
                                                           pt_args[2].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_set_feature_report(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_set_feature_report, arg_1 = %s, arg_2 = %s\n",
        pt_args[0].u.pc_arg, pt_args[1].u.pc_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_set_feature_report(pt_args[0].u.pc_arg,
                                                           pt_args[1].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_virtual_unplug(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_virtual_unplug, arg_1 = %s\n",
        pt_args[0].u.pc_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_virtual_unplug(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_send_data(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_send_data, arg_1 = %s, arg_2 = %s\n",
        pt_args[0].u.pc_arg, pt_args[1].u.pc_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_send_data(pt_args[0].u.pc_arg,
                                                  pt_args[1].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_get_protocol(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_get_protocol, arg_1 = %s\n",
        pt_args[0].u.pc_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_get_protocol(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_set_protocol(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_hidh_set_protocol, arg_1 = %s, arg_2 = %d\n",
        pt_args[0].u.pc_arg, pt_args[1].u.ui1_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_set_protocol(pt_args[0].u.pc_arg,
                                                     pt_args[1].u.ui1_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_hidh_get_connection_status(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_get_connection_status\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_hidh_get_connection_status(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_hidh_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_hidh_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_hidh_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_unregister_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_set_output_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_get_input_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_get_output_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_set_input_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_get_feature_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_set_feature_report);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_virtual_unplug);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_send_data);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_get_protocol);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_set_protocol);
    RPC_REG_OP_HNDLR(x_mtkapi_hidh_get_connection_status);
    return RPCR_OK;
}
