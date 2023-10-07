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

#include "mtk_bt_service_gap.h"
#include "mtk_bt_service_gap_ipcrpc_struct.h"
#include "mtk_bt_service_gap_handle.h"
#include "ri_common.h"
#include "mtk_bt_service_common.h"

#define BT_GAP_RH_LOG(_stmt...) \
    do{ \
        if(0) {    \
            printf("[Handle]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
            printf(_stmt); \
            printf("\n"); \
        }        \
    }   \
    while(0)

void *g_gap_ptag = NULL;

static struct dl_list g_bt_mw_rpc_gap_cb_list =
    {&g_bt_mw_rpc_gap_cb_list, &g_bt_mw_rpc_gap_cb_list};

static pthread_mutex_t g_bt_mw_rpc_gap_lock;
#define BT_MW_RPC_GAP_LOCK() \
    do { \
        pthread_mutex_lock(&(g_bt_mw_rpc_gap_lock)); \
    } while(0)
#define BT_MW_RPC_GAP_UNLOCK() \
    do { \
        pthread_mutex_unlock(&(g_bt_mw_rpc_gap_lock));\
    } while(0)

static INT32 bt_gap_alloc_cb(BT_GAP_CALLBACK_INFO *info)
{
    BT_GAP_CALLBACK_NODE *cb = (BT_GAP_CALLBACK_NODE *)malloc(sizeof(BT_GAP_CALLBACK_NODE));

    if (NULL == cb)
    {
        BT_GAP_RH_LOG("alloc fail");
        return -1;
    }

    memset((void*)cb, 0, sizeof(BT_GAP_CALLBACK_NODE));
    memcpy(&cb->cb_info, info, sizeof(BT_GAP_CALLBACK_INFO));
    BT_MW_RPC_GAP_LOCK();
    dl_list_add(&g_bt_mw_rpc_gap_cb_list, &cb->node);
    BT_MW_RPC_GAP_UNLOCK();
    return 0;
}

static VOID bt_gap_free_cb(BT_GAP_CALLBACK_INFO *info)
{
    BT_GAP_CALLBACK_NODE *cb = NULL;
    BT_GAP_CALLBACK_NODE *tmp = NULL;

    BT_MW_RPC_GAP_LOCK();
    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gap_cb_list, BT_GAP_CALLBACK_NODE, node)
    {
        if (0 == memcmp(&cb->cb_info, info, sizeof(BT_GAP_CALLBACK_INFO)))
        {
            dl_list_del(&cb->node);
            free(cb);
            BT_MW_RPC_GAP_UNLOCK();
            return;
        }
    }
    BT_MW_RPC_GAP_UNLOCK();
}

static VOID bt_app_gap_event_cbk_agent(BTMW_GAP_EVT *bt_event, void* pv_tag)
{
    RPC_DECL_VOID(2);
    BT_GAP_CALLBACK_NODE *cb = NULL;
    BT_GAP_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;

    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, bt_event, RPC_DESC_BTMW_GAP_EVT, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, bt_event);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_MW_RPC_GAP_LOCK();
    BT_GAP_RH_LOG("bt_app_gap_event_cbk_agent size=%d.", dl_list_len(&g_bt_mw_rpc_gap_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_gap_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gap_cb_list, BT_GAP_CALLBACK_NODE, node)
        {
            BT_GAP_RH_LOG("current bt_event_cb = %p.", cb->cb_info.app_func.bt_event_cb);
            pt_nfy_tag->apv_cb_addr_ex[APP_EVENT_CB_IDX] = cb->cb_info.app_func.bt_event_cb;
            if (NULL != pt_nfy_tag->apv_cb_addr_ex[APP_EVENT_CB_IDX])
            {
                RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_gap_event_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_EVENT_CB_IDX]);
            }
            else
            {
                BT_GAP_RH_LOG("skip NULL bt_event_cb.");
            }
        }
    }
    BT_MW_RPC_GAP_UNLOCK();

    BT_GAP_RH_LOG("bt_app_gap_event_cbk_agent done.");
    RPC_RETURN_VOID;
}

static VOID bt_app_gap_inquiry_response_cbk_agent(BTMW_GAP_DEVICE_INFO *pt_result, void* pv_tag)
{
    RPC_DECL_VOID(2);
    BT_GAP_CALLBACK_NODE *cb = NULL;
    BT_GAP_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_result, RPC_DESC_BTMW_GAP_DEVICE_INFO, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, pt_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_MW_RPC_GAP_LOCK();
    BT_GAP_RH_LOG("bt_app_gap_inquiry_response_cbk_agent size=%d.", dl_list_len(&g_bt_mw_rpc_gap_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_gap_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gap_cb_list, BT_GAP_CALLBACK_NODE, node)
        {
            BT_GAP_RH_LOG("current bt_dev_info_cb = %p.", cb->cb_info.app_func.bt_dev_info_cb);
            pt_nfy_tag->apv_cb_addr_ex[APP_GAP_INQUIRY_RESPONSE_CB_IDX] = cb->cb_info.app_func.bt_dev_info_cb;
            if (NULL != pt_nfy_tag->apv_cb_addr_ex[APP_GAP_INQUIRY_RESPONSE_CB_IDX])
            {
                RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_gap_inquiry_response_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_GAP_INQUIRY_RESPONSE_CB_IDX]);
            }
            else
            {
                BT_GAP_RH_LOG("skip NULL bt_event_cb.");
            }
        }
    }
    BT_MW_RPC_GAP_UNLOCK();

    BT_GAP_RH_LOG("bt_app_gap_inquiry_response_cbk_agent done.");
    RPC_RETURN_VOID;
}

static VOID bt_app_gap_get_pairing_key_cbk_agent(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{
    RPC_DECL_VOID(3);
    BT_GAP_CALLBACK_NODE *cb = NULL;
    BT_GAP_CALLBACK_NODE *tmp = NULL;
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, bt_pairing_key, RPC_DESC_pairing_key_value_t, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, bt_pairing_key);
    RPC_ARG_IO(ARG_TYPE_REF_UINT8, fg_accept);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_GAP_RH_LOG("[_hndlr_]bt_app_gap_get_pairing_key_cbk_agent, key_type = %d, pt_nfy_tag->apv_cb_addr_ex[%d] = %p\n",
        bt_pairing_key->key_type, APP_GAP_GET_PAIRING_KEY_CB_IDX,
        pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX]);

    BT_MW_RPC_GAP_LOCK();
    BT_GAP_RH_LOG("[_hndlr_]bt_app_gap_get_pairing_key_cbk_agent size=%d.", dl_list_len(&g_bt_mw_rpc_gap_cb_list));
    if (dl_list_len(&g_bt_mw_rpc_gap_cb_list) > 0)
    {
        dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_gap_cb_list, BT_GAP_CALLBACK_NODE, node)
        {
            BT_GAP_RH_LOG("current bt_get_pairing_key_cb = %p.", cb->cb_info.app_func.bt_get_pairing_key_cb);
            pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX] = cb->cb_info.app_func.bt_get_pairing_key_cb;
            if (NULL != pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX])
            {
                RPC_DO_CB(cb->cb_info.rpc_id, "bt_app_gap_get_pairing_key_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX]);
            }
            else
            {
                BT_GAP_RH_LOG("skip NULL bt_get_pairing_key_cb.");
            }
        }
    }
    BT_MW_RPC_GAP_UNLOCK();

    BT_GAP_RH_LOG("bt_app_gap_get_pairing_key_cbk_agent done.");
    RPC_RETURN_VOID;
}

static VOID bt_app_gap_ipc_close_handle(VOID* pv_tag)
{
    BT_GAP_CALLBACK_INFO *gap_info = (BT_GAP_CALLBACK_INFO *)pv_tag;

    BT_GAP_RH_LOG("[_hndlr_] free gap_info(0x%p) rpc_id = %d\n", gap_info, gap_info->rpc_id);
    bt_gap_free_cb(gap_info);
    free(gap_info);
}

static INT32 _hndlr_x_mtkapi_bt_gap_base_init(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    MTKRPCAPI_BT_APP_CB_FUNC bt_app_cb_func;
    RPC_CB_NFY_TAG_T *pt_nfy_tag = NULL;
    VOID *apv_cb_addr[APP_CB_IDX_NUM] = {0};
    MTKRPCAPI_BT_APP_CB_FUNC *app_cb_func = NULL;
    BT_GAP_CALLBACK_INFO *p_gap_info = NULL;
    unsigned int list_len = 0;

    BT_GAP_RH_LOG("[_hndlr_]bt_gap_base_init, rpc_id=%d, arg_1 = %d\n", t_rpc_id, pt_args[0].u.i4_arg);
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_base_init, pt_args[0].u.pv_desc = %p\n", pt_args[0].u.pv_desc);
    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    app_cb_func = (MTKRPCAPI_BT_APP_CB_FUNC *)pt_args[0].u.pv_desc;
    p_gap_info = (BT_GAP_CALLBACK_INFO *)malloc(sizeof(BT_GAP_CALLBACK_INFO));
    if (NULL == p_gap_info)
    {
        BT_GAP_RH_LOG("New BT_GAP_CALLBACK_INFO fail. \n");
        return -1;
    }
    memset(p_gap_info, 0, sizeof(BT_GAP_CALLBACK_INFO));
    p_gap_info->rpc_id = t_rpc_id;
    p_gap_info->app_func.bt_event_cb = app_cb_func->bt_event_cb;
    p_gap_info->app_func.bt_get_pairing_key_cb = app_cb_func->bt_get_pairing_key_cb;
    p_gap_info->app_func.bt_dev_info_cb = app_cb_func->bt_dev_info_cb;
    BT_MW_RPC_GAP_LOCK();
    bt_gap_alloc_cb(p_gap_info);
#if 0
    if (p_gap_info)
    {
        free(p_gap_info);
    }
#endif
    memset(&bt_app_cb_func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    list_len = dl_list_len(&g_bt_mw_rpc_gap_cb_list);
    if (1 == list_len)
    {
        bt_app_cb_func.bt_event_cb = bt_app_gap_event_cbk_agent;
        bt_app_cb_func.bt_get_pairing_key_cb = bt_app_gap_get_pairing_key_cbk_agent;
        bt_app_cb_func.bt_dev_info_cb = bt_app_gap_inquiry_response_cbk_agent;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, APP_CB_IDX_NUM, pt_args[1].u.pv_arg);
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_bt_gap_base_init(&bt_app_cb_func, pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }
    else
    {
        x_mtkapi_bt_gap_get_bonded_dev();
    }
    BT_MW_RPC_GAP_UNLOCK();

    BT_GAP_RH_LOG("bt_register_app_ipc_close_handle. pv_tag(%p) rpc_id = %d", p_gap_info, p_gap_info->rpc_id);
    bt_register_app_ipc_close_handle(t_rpc_id, p_gap_info, bt_app_gap_ipc_close_handle);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_base_uninit(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    MTKRPCAPI_BT_APP_CB_FUNC bt_app_cb_func;
    MTKRPCAPI_BT_APP_CB_FUNC *app_cb_func = NULL;
    BT_GAP_CALLBACK_INFO *p_gap_info = NULL;
    unsigned int list_len = 0;


    BT_GAP_RH_LOG("[_hndlr_]bt_gap_base_uninit, rpc_id=%d, arg_1 = %d\n", t_rpc_id, pt_args[0].u.i4_arg);
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_base_uninit, pt_args[0].u.pv_desc = %p\n", pt_args[0].u.pv_desc);

    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    BT_MW_RPC_GAP_LOCK();
    list_len = dl_list_len(&g_bt_mw_rpc_gap_cb_list);
    BT_MW_RPC_GAP_UNLOCK();
    if (0 == list_len)
    {
        return RPCR_NOT_OPENED;
    }

    memset(&bt_app_cb_func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    app_cb_func = (MTKRPCAPI_BT_APP_CB_FUNC*)pt_args[0].u.pv_desc;
    p_gap_info = (BT_GAP_CALLBACK_INFO *)malloc(sizeof(BT_GAP_CALLBACK_INFO));
    if (NULL == p_gap_info)
    {
        BT_GAP_RH_LOG("New BT_GAP_CALLBACK_INFO fail. \n");
        return -1;
    }
    memset(p_gap_info, 0, sizeof(BT_GAP_CALLBACK_INFO));
    p_gap_info->rpc_id = t_rpc_id;
    p_gap_info->app_func.bt_event_cb = app_cb_func->bt_event_cb;
    p_gap_info->app_func.bt_get_pairing_key_cb = app_cb_func->bt_get_pairing_key_cb;
    p_gap_info->app_func.bt_dev_info_cb = app_cb_func->bt_dev_info_cb;

    bt_gap_free_cb(p_gap_info);
    if (p_gap_info)
    {
        free(p_gap_info);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_on_off(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_on_off , arg_1 = %d\n", pt_args[0].u.b_arg);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_on_off(pt_args[0].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_factory_reset(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_factory_reset\n");

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_factory_reset();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_set_name(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_set_name , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_name(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_set_connectable_and_discoverable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_set_connectable_and_discoverable , arg_1 = %d, arg_2 = %d\n", pt_args[0].u.b_arg, pt_args[1].u.b_arg);

    if (ui4_num_args != 2)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_connectable_and_discoverable(pt_args[0].u.b_arg, pt_args[1].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_dev_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_get_dev_info\n");

    if (ui4_num_args != 2)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_dev_info(pt_args[0].u.pv_desc, pt_args[1].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_bond_state(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_get_bond_state\n");

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_bond_state(pt_args[0].u.ps_str);

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_gap_get_local_dev_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_get_local_dev_info , arg_1 = %p\n", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_local_dev_info(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_start_inquiry(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_start_inquiry\n");

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_start_inquiry(pt_args[0].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_stop_inquiry(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_stop_inquiry\n");

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_stop_inquiry();

    if (pt_return->u.i4_arg == 0)
    {
        return RPCR_OK;
    }
    else
    {
        return RPCR_ERROR;
    }

}

static INT32 _hndlr_x_mtkapi_bt_gap_pair(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_pair , arg_1 = %s, arg_2 = %d\n", pt_args[0].u.ps_str, pt_args[1].u.i4_arg);

    if (ui4_num_args != 2)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_pair(pt_args[0].u.ps_str, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_unpair(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_unpair_dev_erase , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_unpair(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_cancel_pair(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_cancel_pair_dev_erase , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_cancel_pair(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_rssi(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_get_rssi , arg_1 = %s, arg_2 = %p\n", pt_args[0].u.ps_str, pt_args[1].u.pi2_arg);

    if (ui4_num_args != 2)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_rssi(pt_args[0].u.ps_str, pt_args[1].u.pi2_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_send_hci(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_GAP_RH_LOG("[_hndlr_]bt_gap_send_hci , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_GAP_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_send_hci(pt_args[0].u.ps_str);

    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_gap_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_gap_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_base_init);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_base_uninit);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_on_off);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_factory_reset);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_name);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_connectable_and_discoverable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_dev_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_bond_state);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_local_dev_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_start_inquiry);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_stop_inquiry);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_pair);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_unpair);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_cancel_pair);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_rssi);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_send_hci);
    return RPCR_OK;
}
