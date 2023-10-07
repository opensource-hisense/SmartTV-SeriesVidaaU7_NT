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

#include "mtk_bt_service_a2dp.h"
#include "mtk_bt_service_a2dp_handle.h"
#include "mtk_bt_service_a2dp_ipcrpc_struct.h"
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_common.h"

A2dpRegCbLinkList* pA2dpLinkList = NULL;

static pthread_mutex_t g_bt_mw_rpc_a2dp_lock;
#define BT_MW_RPC_A2DP_LOCK() \
        do { \
            pthread_mutex_lock(&(g_bt_mw_rpc_a2dp_lock)); \
        } while(0)
#define BT_MW_RPC_A2DP_UNLOCK() \
        do { \
            pthread_mutex_unlock(&(g_bt_mw_rpc_a2dp_lock));\
        } while(0)

#define BT_A2DP_HDL_LOG(_stmt, ...) \
        do{ \
            if(1){    \
                printf("[A2DP-Hdl][%s@%d]"_stmt"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);   \
            }        \
        }   \
        while(0)

static VOID bt_app_a2dp_event_cbk_wrapper_agent(BT_A2DP_EVENT_PARAM *param,
    void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_A2DP_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    //RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_a2dp_event_cbk", pt_nfy_tag->pv_cb_addr);

    BT_MW_RPC_A2DP_LOCK();
    if (pA2dpLinkList->i4_size > 0)
    {
        A2dpRegCbNode* pCurrent = pA2dpLinkList->pHead->next;
        while(pCurrent != NULL)
        {
            //BT_A2DP_HDL_LOG("current bt_event_cb = %p.", pCurrent->a2dp_info.app_func);

            if (NULL != pCurrent->a2dp_info.app_func)
            {
                RPC_DO_CB(pCurrent->a2dp_info.t_id, "bt_app_a2dp_event_cbk", pCurrent->a2dp_info.app_func);
            }
            else
            {
                BT_A2DP_HDL_LOG("skip NULL bt_event_cb.");
            }

            pCurrent = pCurrent->next;
        }
    }
    BT_MW_RPC_A2DP_UNLOCK();
    //BT_A2DP_HDL_LOG("bt_app_a2dp_event_cbk_wrapper_agent done.");

    RPC_RETURN_VOID;
}


static A2dpRegCbLinkList* A2dpRegCbLinkList_Create()
{
    BT_A2DP_HDL_LOG("Multi a2dp create linklist start...");
    A2dpRegCbLinkList* pLinkList = (A2dpRegCbLinkList*)malloc(sizeof(A2dpRegCbLinkList));
    if (NULL == pLinkList)
    {
        BT_A2DP_HDL_LOG("New A2dpRegCbLinkList fail!");
        return NULL;
    }
    pLinkList->i4_size = 0;
    pLinkList->pHead = (A2dpRegCbNode*)malloc(sizeof(A2dpRegCbNode));
    if (NULL == pLinkList->pHead)
    {
        BT_A2DP_HDL_LOG("New A2dpRegCbNode fail!");
        if (NULL != pLinkList)
        {
            free(pLinkList);
        }
        return NULL;
    }
    memset(&pLinkList->pHead->a2dp_info, 0, sizeof(A2dpRegInfo));
    pLinkList->pHead->next = NULL;
    BT_A2DP_HDL_LOG("Multi a2dp create linklist complete.");

    return pLinkList;
}

static INT32 A2dpRegCbLinkList_FreeLinkList(A2dpRegCbLinkList* pLinkList)
{
    BT_A2DP_HDL_LOG("Remove link list start...");
    if (NULL == pLinkList)
    {
        BT_A2DP_HDL_LOG("Invalid parameter.");
        return -1;
    }
    if (NULL == pLinkList->pHead)
    {
        free(pLinkList);
    }
    else
    {
        A2dpRegCbNode* pCurrent = pLinkList->pHead->next;
        while (pCurrent != NULL)
        {
            pLinkList->pHead->next = pCurrent->next;
            if (NULL != pCurrent)
            {
                free(pCurrent);
                pCurrent = NULL;
                pLinkList->i4_size--;
            }
            pCurrent = pLinkList->pHead->next;
        }
        if (NULL != pLinkList->pHead)
        {
            free(pLinkList->pHead);
            pLinkList->pHead = NULL;
        }
        if (NULL != pLinkList && NULL == pLinkList->pHead)
        {
            free(pLinkList);
        }
    }
    BT_A2DP_HDL_LOG("Remove link list complete!");
    return 0;
}


INT32 A2dpRegCbLinkList_Add(A2dpRegCbLinkList* pLinkList, A2dpRegInfo* p_a2dp_info)
{
    BT_A2DP_HDL_LOG("\n");
    if(NULL == pLinkList || NULL == p_a2dp_info)
    {
        BT_A2DP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > A2dpRegMax)
    {
        BT_A2DP_HDL_LOG("a2dp register exceeds the maximum. \n");
        return -1;
    }

    A2dpRegCbNode* pNode = (A2dpRegCbNode*)malloc(sizeof(A2dpRegCbNode));
    if(NULL == pNode)
    {
        BT_A2DP_HDL_LOG("New A2dpRegCbNode fail. \n");
        return -1;
    }
    memcpy(&pNode->a2dp_info, p_a2dp_info, sizeof(A2dpRegInfo));
    pNode->next = NULL;

    A2dpRegCbNode* pCurrent = pLinkList->pHead;
    while(pCurrent->next != NULL)
    {
        pCurrent = pCurrent->next;
    }
    pCurrent->next = pNode;
    pLinkList->i4_size++;

    return 0;
}

INT32 A2dpRegCbLinkList_Remove(A2dpRegCbLinkList* pLinkList, A2dpRegInfo* p_a2dp_info)
{
    BT_A2DP_HDL_LOG("\n");
    if(NULL == pLinkList || NULL == p_a2dp_info)
    {
        BT_A2DP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > A2dpRegMax)
    {
        BT_A2DP_HDL_LOG("A2dp register exceeds the maximum. \n");
        return -1;
    }

    A2dpRegCbNode* pPre = pLinkList->pHead;
    A2dpRegCbNode* pCurrent = pPre->next;
    while(pCurrent != NULL)
    {
        if (p_a2dp_info->app_func == pCurrent->a2dp_info.app_func)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                free(pCurrent);
            }
            BT_A2DP_HDL_LOG("Remove a2dp info success. callback = %p", p_a2dp_info->app_func);
            pLinkList->i4_size--;
            return 0;
        }
        pPre = pCurrent;
        pCurrent = pCurrent->next;
    }

    return -1;
}

INT32 A2dpRegCbLinkList_RemoveBytid(A2dpRegCbLinkList* pLinkList, A2dpRegInfo* p_a2dp_info)
{
    BT_A2DP_HDL_LOG("\n");
    if(NULL == pLinkList || NULL == p_a2dp_info)
    {
        BT_A2DP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > A2dpRegMax)
    {
        BT_A2DP_HDL_LOG("A2dp register exceeds the maximum. \n");
        return -1;
    }

    A2dpRegCbNode* pPre = pLinkList->pHead;
    A2dpRegCbNode* pCurrent = pPre->next;
    A2dpRegCbNode* pFree = NULL;
    while(pCurrent != NULL)
    {
        if (p_a2dp_info->t_id == pCurrent->a2dp_info.t_id)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                pFree = pCurrent;
                BT_A2DP_HDL_LOG("Remove a2dp info success. callback = %p", pCurrent->a2dp_info.app_func);
                pCurrent = pCurrent->next;
                free(pFree);
            }
            pLinkList->i4_size--;
        }
        else
        {
            pPre = pCurrent;
            pCurrent = pCurrent->next;
        }
    }

    return -1;
}


static INT32 A2dpRegCbLinkList_GetSize(A2dpRegCbLinkList* pLinkList)
{
    if(NULL == pLinkList)
    {
        BT_A2DP_HDL_LOG("Invalid parameter.");
        return 0;
    }
    BT_A2DP_HDL_LOG("Get multi a2dp linklist size. size=%d", pLinkList->i4_size);
    return pLinkList->i4_size;
}

static VOID A2dpRegCbLinkList_Show(A2dpRegCbLinkList* pLinkList)
{
    BT_A2DP_HDL_LOG("\n");
    if(NULL == pLinkList)
    {
        BT_A2DP_HDL_LOG("Invalid parameter. \n");
        return;
    }
    BT_A2DP_HDL_LOG("a2dp registed number: %d \n", pLinkList->i4_size);
    A2dpRegCbNode* pCurrent = pLinkList->pHead->next;
    while(pCurrent)
    {
        BT_A2DP_HDL_LOG("register_cb = %p \n", pCurrent->a2dp_info.app_func);
        pCurrent = pCurrent->next;
    }
    return;

}

static VOID bt_app_a2dp_ipc_close_handle(VOID* p_tid)
{
    INT32 i4_ret;
    A2dpRegInfo  a2dp_reg_info;
    memset(&a2dp_reg_info, 0, sizeof(A2dpRegInfo));
    a2dp_reg_info.t_id = ((RPC_ID_T)(*(RPC_ID_T*)p_tid));

    BT_A2DP_HDL_LOG("p_tid=%p tid=%d\n", p_tid, ((RPC_ID_T)(*(RPC_ID_T*)p_tid)));
    BT_A2DP_HDL_LOG("A2dpRegCbLinkList_RemoveBytid a2dp_reg_info.t_id=%d \n", a2dp_reg_info.t_id);
    BT_MW_RPC_A2DP_LOCK();
    i4_ret = A2dpRegCbLinkList_RemoveBytid(pA2dpLinkList, &a2dp_reg_info);
    if(i4_ret != 0)
    {
        BT_A2DP_HDL_LOG("A2dpRegCbLinkList_RemoveBytid fail. \n");
        BT_MW_RPC_A2DP_UNLOCK();
        return;
    }
    A2dpRegCbLinkList_Show(pA2dpLinkList);
    if(0 == A2dpRegCbLinkList_GetSize(pA2dpLinkList))
    {
        A2dpRegCbLinkList_FreeLinkList(pA2dpLinkList);
        pA2dpLinkList = NULL;
    }
    BT_MW_RPC_A2DP_UNLOCK();
}


static INT32 _hndlr_x_mtkapi_a2dp_connect(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_connect(pt_args[0].u.ps_str,
        pt_args[1].u.i4_arg);

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_a2dp_disconnect(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_disconnect(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static RPC_ID_T t_rpc_id_for_free = -1;
static INT32 _hndlr_x_mtkapi_a2dp_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");

    mtkrpcapi_BtAppA2dpCbk p_bt_a2dp_app_cb_func = NULL;
    VOID * apv_cb_addr[1] = {0};
    t_rpc_id_for_free = t_rpc_id;
    BT_A2DP_HDL_LOG("t_rpc_id=%d", t_rpc_id_for_free);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    BT_A2DP_HDL_LOG("pv_func=%p, t_rpc_id=%d",
                pt_args[0].u.pv_func, t_rpc_id);

/*******************************************************************/
/*      The first client need create pA2dpLinkList when register callback             */
/*******************************************************************/
    BT_MW_RPC_A2DP_LOCK();
    if (NULL == pA2dpLinkList)
    {
        pA2dpLinkList = A2dpRegCbLinkList_Create();
        RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;

        p_bt_a2dp_app_cb_func = (mtkrpcapi_BtAppA2dpCbk)pt_args[0].u.pv_func;
        if (NULL != p_bt_a2dp_app_cb_func)
        {
            apv_cb_addr[0] = (VOID*)p_bt_a2dp_app_cb_func;
            pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
                pt_args[1].u.pv_arg);

            BT_A2DP_HDL_LOG("pv_func=%p, pv_arg=%p, pt_nfy_tag=%p",
                p_bt_a2dp_app_cb_func, pt_args[1].u.pv_arg, pt_nfy_tag);

            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg =
                x_mtkapi_a2dp_register_callback(bt_app_a2dp_event_cbk_wrapper_agent,
                pt_nfy_tag);
            if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
            {
                ri_free_cb_tag(pt_nfy_tag);
            }
        }
        else
        {
            pt_return->u.i4_arg = x_mtkapi_a2dp_register_callback(NULL, NULL);
        }
    }
    BT_MW_RPC_A2DP_UNLOCK();
/*******************************************************************/
/* Add client callback to pA2dpLinkList  */
/*******************************************************************/
    p_bt_a2dp_app_cb_func = (mtkrpcapi_BtAppA2dpCbk)pt_args[0].u.pv_func;
    INT32 i4_ret;
    A2dpRegInfo* p_a2dp_info = (A2dpRegInfo*)malloc(sizeof(A2dpRegInfo));
    if(NULL == p_a2dp_info)
    {
        BT_A2DP_HDL_LOG("New A2dpRegInfo fail. \n");
        return -1;
    }
    memset(p_a2dp_info, 0, sizeof(A2dpRegInfo));

    p_a2dp_info->t_id = t_rpc_id;
    p_a2dp_info->app_func = p_bt_a2dp_app_cb_func;

    BT_MW_RPC_A2DP_LOCK();
    i4_ret = A2dpRegCbLinkList_Add(pA2dpLinkList, p_a2dp_info);
    if(i4_ret != 0)
    {
        BT_A2DP_HDL_LOG("A2dpRegCbLinkList_Add fail. \n");
        if (p_a2dp_info != NULL)
        {
            free(p_a2dp_info);
        }
        BT_MW_RPC_A2DP_UNLOCK();
        return -1;
    }
    else
    {
        BT_A2DP_HDL_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d", t_rpc_id);
        bt_register_app_ipc_close_handle(t_rpc_id, &t_rpc_id_for_free, bt_app_a2dp_ipc_close_handle);
    }
    if(NULL != p_a2dp_info)
    {
        free(p_a2dp_info);
    }
    A2dpRegCbLinkList_Show(pA2dpLinkList);
    BT_MW_RPC_A2DP_UNLOCK();
    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_a2dp_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("[_hndlr_]_hndlr_x_mtkapi_a2dp_register_callback, arg_1 = %d\n", pt_args[0].u.i4_arg);

    mtkrpcapi_BtAppA2dpCbk p_bt_a2dp_app_cb_func = NULL;

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    p_bt_a2dp_app_cb_func = (mtkrpcapi_BtAppA2dpCbk)pt_args[0].u.pv_func;

    if (p_bt_a2dp_app_cb_func == NULL)
    {
        BT_A2DP_HDL_LOG("unregister callback is NULL. \n");
        return RPCR_INV_ARGS;
    }

/*******************************************************************/
/*  remove  a2dp callback from pA2dpLinkList  */
/*******************************************************************/
    INT32 i4_ret;
    A2dpRegInfo* p_a2dp_info = (A2dpRegInfo*)malloc(sizeof(A2dpRegInfo));
    if(NULL == p_a2dp_info)
    {
        BT_A2DP_HDL_LOG("New A2dpRegInfo fail. \n");
        return -1;
    }
    memset(p_a2dp_info, 0, sizeof(A2dpRegInfo));

    p_a2dp_info->t_id = t_rpc_id;
    p_a2dp_info->app_func = p_bt_a2dp_app_cb_func;

    BT_MW_RPC_A2DP_LOCK();
    i4_ret = A2dpRegCbLinkList_Remove(pA2dpLinkList, p_a2dp_info);
    if(i4_ret != 0)
    {
        BT_A2DP_HDL_LOG("A2dpRegCbLinkList_Remove fail. \n");
        if (p_a2dp_info != NULL)
        {
            free(p_a2dp_info);
        }
        BT_MW_RPC_A2DP_UNLOCK();
        return -1;
    }
    if (p_a2dp_info != NULL)
    {
        free(p_a2dp_info);
    }
    A2dpRegCbLinkList_Show(pA2dpLinkList);
    if(0 == A2dpRegCbLinkList_GetSize(pA2dpLinkList))
    {
        A2dpRegCbLinkList_FreeLinkList(pA2dpLinkList);
        pA2dpLinkList = NULL;
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_a2dp_unregister_callback();
    }
    BT_MW_RPC_A2DP_UNLOCK();
    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_a2dp_get_connected_dev_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_get_connected_dev_list(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_adjust_buf_time(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg =
        x_mtkapi_a2dp_sink_adjust_buf_time(pt_args[0].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_enable(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_enable(pt_args[0].u.ui1_arg);

    return RPCR_OK;
}
#if 0

static INT32 _hndlr_x_mtkapi_a2dp_sink_start_player(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_start_player();

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_a2dp_sink_stop_player(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_stop_player();

    return RPCR_OK;
}
#endif
static INT32 _hndlr_x_mtkapi_a2dp_set_dbg_flag(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_set_dbg_flag(pt_args[0].u.i4_arg,
        pt_args[1].u.pv_desc);

    return RPCR_OK;

}


static INT32 _hndlr_x_mtkapi_a2dp_sink_get_dev_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_get_dev_list(pt_args[0].u.pv_desc);

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_a2dp_src_get_dev_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_get_dev_list(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_codec_enable(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_codec_enable(pt_args[0].u.i4_arg,
        pt_args[1].u.ui1_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_set_channel_allocation_for_lrmode(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_set_channel_allocation_for_lrmode(pt_args[0].u.ps_str,
        pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_set_audiomode(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_set_audiomode(pt_args[0].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_set_link_num(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_set_link_num(pt_args[0].u.i4_arg);

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_a2dp_src_enable(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_enable(pt_args[0].u.ui1_arg, pt_args[1].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_config_codec_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_config_codec_info(pt_args[0].u.ps_str, pt_args[1].u.pv_desc);
    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_a2dp_sink_active_src(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_active_src(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_get_active_src(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_get_active_src(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_set_delay_value(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_set_delay_value(pt_args[0].u.ps_str,
        pt_args[1].u.ui2_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_get_stack_delay(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_get_stack_delay();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_lowpower_enable(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_lowpower_enable(pt_args[0].u.ui1_arg);

    return RPCR_OK;
}

#if 0
static INT32 _hndlr_x_mtkapi_a2dp_sink_player_load(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_player_load(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_sink_player_unload(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_sink_player_unload(pt_args[0].u.ps_str);

    return RPCR_OK;
}
#endif
static INT32 _hndlr_x_mtkapi_a2dp_src_active_sink(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_active_sink(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_get_active_sink(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_get_active_sink(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_set_silence_device(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_set_silence_device(pt_args[0].u.ps_str, pt_args[1].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_is_in_silence_mode(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_BOOL;
    pt_return->u.b_arg = x_mtkapi_a2dp_src_is_in_silence_mode(pt_args[0].u.ps_str);

    return RPCR_OK;
}
#if 0
static INT32 _hndlr_x_mtkapi_a2dp_src_pause_uploader(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_pause_uploader(NULL);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_resume_uploader(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_resume_uploader(NULL);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_mute_uploader(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_mute_uploader(pt_args[0].u.ui1_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_uploader_load(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_uploader_load(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_a2dp_src_uploader_unload(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_A2DP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_a2dp_src_uploader_unload(pt_args[0].u.ps_str);

    return RPCR_OK;
}
#endif
INT32 c_rpc_reg_mtk_bt_service_a2dp_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_a2dp_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_unregister_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_get_connected_dev_list);

    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_adjust_buf_time);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_get_dev_list);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_active_src);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_get_active_src);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_set_delay_value);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_get_stack_delay);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_lowpower_enable);

    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_set_dbg_flag);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_get_dev_list);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_active_sink);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_set_silence_device);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_is_in_silence_mode);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_get_active_sink);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_codec_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_config_codec_info);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_sink_set_link_num);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_set_channel_allocation_for_lrmode);
    RPC_REG_OP_HNDLR(x_mtkapi_a2dp_src_set_audiomode);
    return RPCR_OK;
}


