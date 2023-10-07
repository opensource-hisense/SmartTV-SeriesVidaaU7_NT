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

#include "mtk_bt_service_avrcp.h"
#include "mtk_bt_service_avrcp_handle.h"
#include "mtk_bt_service_avrcp_ipcrpc_struct.h"
#include "u_rpc.h"
#include "ri_common.h"
#include "mtk_bt_service_common.h"

AvrcpRegCbLinkList* pAvrcpLinkList = NULL;
static pthread_mutex_t g_bt_mw_rpc_avrcp_lock;
#define BT_MW_RPC_AVRCP_LOCK() \
        do { \
            pthread_mutex_lock(&(g_bt_mw_rpc_avrcp_lock)); \
        } while(0)
#define BT_MW_RPC_AVRCP_UNLOCK() \
        do { \
            pthread_mutex_unlock(&(g_bt_mw_rpc_avrcp_lock));\
        } while(0)

#define BT_AVRCP_HDL_LOG(_stmt, ...) \
    do{ \
        if(1){    \
            printf("[AVRCP-Hdl][%s@%d]"_stmt"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);   \
        }        \
    }   \
    while(0)

static VOID bt_app_avrcp_event_cbk_wrapper_agent(BT_AVRCP_EVENT_PARAM *param,
    void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_AVRCP_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    //RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_avrcp_event_cbk", pt_nfy_tag->pv_cb_addr);
    BT_MW_RPC_AVRCP_LOCK();
    if(pAvrcpLinkList == NULL)
    {
        BT_AVRCP_HDL_LOG("bt_app_avrcp_event_cbk_wrapper_agent pAvrcpLinkList is null.");
        BT_MW_RPC_AVRCP_UNLOCK();
        RPC_RETURN_VOID;
    }
    if (pAvrcpLinkList->i4_size > 0)
    {
        AvrcpRegCbNode* pCurrent = pAvrcpLinkList->pHead->next;
        while(pCurrent != NULL)
        {
            //BT_AVRCP_HDL_LOG("current bt_event_cb = %p.", pCurrent->avrcp_info.app_func);

            if (NULL != pCurrent->avrcp_info.app_func)
            {
                RPC_DO_CB(pCurrent->avrcp_info.t_id, "bt_app_avrcp_event_cbk", pCurrent->avrcp_info.app_func);
            }
            else
            {
                BT_AVRCP_HDL_LOG("skip NULL bt_event_cb.");
            }
            pCurrent = pCurrent->next;
        }
    }
    BT_MW_RPC_AVRCP_UNLOCK();
    //BT_AVRCP_HDL_LOG("bt_app_avrcp_event_cbk_wrapper_agent done.");

    RPC_RETURN_VOID;
}

static AvrcpRegCbLinkList* AvrcpRegCbLinkList_Create()
{
    BT_AVRCP_HDL_LOG("Multi Avrcp create linklist start...");
    AvrcpRegCbLinkList* pLinkList = (AvrcpRegCbLinkList*)malloc(sizeof(AvrcpRegCbLinkList));
    if (NULL == pLinkList)
    {
        BT_AVRCP_HDL_LOG("New AvrcpRegCbLinkList fail!");
        return NULL;
    }
    pLinkList->i4_size = 0;
    pLinkList->pHead = (AvrcpRegCbNode*)malloc(sizeof(AvrcpRegCbNode));
    if (NULL == pLinkList->pHead)
    {
        BT_AVRCP_HDL_LOG("New AvrcpRegCbNode fail!");
        if (NULL != pLinkList)
        {
            free(pLinkList);
        }
        return NULL;
    }
    memset(&pLinkList->pHead->avrcp_info, 0, sizeof(AvrcpRegInfo));
    pLinkList->pHead->next = NULL;
    BT_AVRCP_HDL_LOG("Multi avrcp create linklist complete.");

    return pLinkList;
}

static INT32 AvrcpRegCbLinkList_FreeLinkList(AvrcpRegCbLinkList* pLinkList)
{
    BT_AVRCP_HDL_LOG("Remove link list start...");
    if (NULL == pLinkList)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter.");
        return -1;
    }
    if (NULL == pLinkList->pHead)
    {
        free(pLinkList);
    }
    else
    {
        AvrcpRegCbNode* pCurrent = pLinkList->pHead->next;
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
    BT_AVRCP_HDL_LOG("Remove link list complete!");
    return 0;
}


INT32 AvrcpRegCbLinkList_Add(AvrcpRegCbLinkList* pLinkList, AvrcpRegInfo* p_avrcp_info)
{
    BT_AVRCP_HDL_LOG("\n");
    if(NULL == pLinkList || NULL == p_avrcp_info)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > AvrcpRegMax)
    {
        BT_AVRCP_HDL_LOG("avrcp register exceeds the maximum. \n");
        return -1;
    }

    AvrcpRegCbNode* pNode = (AvrcpRegCbNode*)malloc(sizeof(AvrcpRegCbNode));
    if(NULL == pNode)
    {
        BT_AVRCP_HDL_LOG("New AvrcpRegCbNode fail. \n");
        return -1;
    }
    memcpy(&pNode->avrcp_info, p_avrcp_info, sizeof(AvrcpRegInfo));
    pNode->next = NULL;

    AvrcpRegCbNode* pCurrent = pLinkList->pHead;
    while(pCurrent->next != NULL)
    {
        pCurrent = pCurrent->next;
    }
    pCurrent->next = pNode;
    pLinkList->i4_size++;

    return 0;
}

INT32 AvrcpRegCbLinkList_Remove(AvrcpRegCbLinkList* pLinkList, AvrcpRegInfo* p_avrcp_info)
{
    BT_AVRCP_HDL_LOG("\n");
    if(NULL == pLinkList || NULL == p_avrcp_info)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > AvrcpRegMax)
    {
        BT_AVRCP_HDL_LOG("Avrcp register exceeds the maximum. \n");
        return -1;
    }

    AvrcpRegCbNode* pPre = pLinkList->pHead;
    AvrcpRegCbNode* pCurrent = pPre->next;
    while(pCurrent != NULL)
    {
        if (p_avrcp_info->app_func == pCurrent->avrcp_info.app_func)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                free(pCurrent);
            }
            BT_AVRCP_HDL_LOG("Remove avrcp info success. callback = %p", p_avrcp_info->app_func);
            pLinkList->i4_size--;
            return 0;
        }
        pPre = pCurrent;
        pCurrent = pCurrent->next;
    }

    return -1;
}

INT32 AvrcpRegCbLinkList_RemoveBytid(AvrcpRegCbLinkList* pLinkList, AvrcpRegInfo* p_avrcp_info)
{
    BT_AVRCP_HDL_LOG("\n");
    INT32 ret = 0;
    if(NULL == pLinkList || NULL == p_avrcp_info)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > AvrcpRegMax)
    {
        BT_AVRCP_HDL_LOG("Avrcp register exceeds the maximum. \n");
        return -1;
    }

    AvrcpRegCbNode* pPre = pLinkList->pHead;
    AvrcpRegCbNode* pCurrent = pPre->next;
    AvrcpRegCbNode* pFree = NULL;
    while(pCurrent != NULL)
    {
        if (p_avrcp_info->t_id == pCurrent->avrcp_info.t_id)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                pFree = pCurrent;
                BT_AVRCP_HDL_LOG("Remove avrcp info success. callback = %p", pCurrent->avrcp_info.app_func);
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
    return ret;
}


static INT32 AvrcpRegCbLinkList_GetSize(AvrcpRegCbLinkList* pLinkList)
{
    if(NULL == pLinkList)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter.");
        return 0;
    }
    BT_AVRCP_HDL_LOG("Get multi avrcp linklist size. size=%d", pLinkList->i4_size);
    return pLinkList->i4_size;
}

static VOID AvrcpRegCbLinkList_Show(AvrcpRegCbLinkList* pLinkList)
{
    BT_AVRCP_HDL_LOG("\n");
    if(NULL == pLinkList)
    {
        BT_AVRCP_HDL_LOG("Invalid parameter. \n");
        return;
    }
    BT_AVRCP_HDL_LOG("avrcp registed number: %d \n", pLinkList->i4_size);
    AvrcpRegCbNode* pCurrent = pLinkList->pHead->next;
    while(pCurrent)
    {
        BT_AVRCP_HDL_LOG("register_cb = %p \n", pCurrent->avrcp_info.app_func);
        pCurrent = pCurrent->next;
    }
    return;

}

static INT32 _hndlr_x_mtkapi_avrcp_change_player_app_setting(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_change_player_app_setting(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_send_passthrough_cmd(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (3 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_send_passthrough_cmd(pt_args[0].u.ps_str,
        pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_send_vendor_unique_cmd(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (3 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg =
        x_mtkapi_avrcp_send_vendor_unique_cmd(pt_args[0].u.ps_str,
        pt_args[1].u.ui1_arg, pt_args[2].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_change_volume(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_change_volume(pt_args[0].u.ps_str,
        pt_args[1].u.ui1_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_update_player_status(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_update_player_status(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_get_playback_state(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_get_playback_state(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_send_playitem(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_send_playitem(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_get_now_playing_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_get_now_playing_list(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_get_folder_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_get_folder_list(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_get_player_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_get_player_list(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_change_folder_path(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_change_folder_path(pt_args[0].u.ps_str,
        pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_set_browsed_player(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_set_browsed_player(pt_args[0].u.ps_str,
        pt_args[1].u.ui2_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_set_addressed_player(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (2 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_set_addressed_player(pt_args[0].u.ps_str,
        pt_args[1].u.ui2_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_update_player_media_info(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_update_player_media_info(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_avrcp_update_absolute_volume(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_update_absolute_volume(pt_args[0].u.ui1_arg);

    return RPCR_OK;
}

static VOID bt_app_avrcp_ipc_close_handle(VOID* p_tid)
{
    INT32 i4_ret;
    AvrcpRegInfo avrcp_reginfo;
    memset(&avrcp_reginfo, 0, sizeof(AvrcpRegInfo));
    avrcp_reginfo.t_id = ((RPC_ID_T)(*(RPC_ID_T*)p_tid));

    BT_AVRCP_HDL_LOG("p_tid=%p tid=%d\n", p_tid, ((RPC_ID_T)(*(RPC_ID_T*)p_tid)));
    BT_AVRCP_HDL_LOG("AvrcpRegCbLinkList_RemoveBytid avrcp_reginfo.t_id=%d \n", avrcp_reginfo.t_id);
    BT_MW_RPC_AVRCP_LOCK();
    i4_ret = AvrcpRegCbLinkList_RemoveBytid(pAvrcpLinkList, &avrcp_reginfo);
    if(i4_ret != 0)
    {
        BT_AVRCP_HDL_LOG("A2dpRegCbLinkList_RemoveBytid fail. \n");
        BT_MW_RPC_AVRCP_UNLOCK();
        return;
    }
    AvrcpRegCbLinkList_Show(pAvrcpLinkList);
    if(0 == AvrcpRegCbLinkList_GetSize(pAvrcpLinkList))
    {
        AvrcpRegCbLinkList_FreeLinkList(pAvrcpLinkList);
        pAvrcpLinkList = NULL;
    }
    BT_MW_RPC_AVRCP_UNLOCK();
}

static INT32 _hndlr_x_mtkapi_avrcp_get_connected_dev_list(
                        RPC_ID_T     t_rpc_id,
                        const CHAR*  ps_cb_type,
                        UINT32       ui4_num_args,
                        ARG_DESC_T*  pt_args,
                        ARG_DESC_T*  pt_return)
{
    BT_AVRCP_HDL_LOG("");
    if (1 != ui4_num_args)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_avrcp_get_connected_dev_list(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static RPC_ID_T t_rpc_id_for_free = -1;
static INT32 _hndlr_x_mtkapi_avrcp_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_AVRCP_HDL_LOG("");

    mtkrpcapi_BtAppAvrcpCbk p_bt_avrcp_app_cb_func = NULL;
    VOID * apv_cb_addr[1] = {0};
    t_rpc_id_for_free = t_rpc_id;
    BT_AVRCP_HDL_LOG("t_rpc_id=%d", t_rpc_id_for_free);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

/*******************************************************************/
/*      The first need create pAvrcpLinkList when register callback             */
/*******************************************************************/
    BT_MW_RPC_AVRCP_LOCK();
    if (NULL == pAvrcpLinkList)
    {
        pAvrcpLinkList = AvrcpRegCbLinkList_Create();
        RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;

         p_bt_avrcp_app_cb_func = (mtkrpcapi_BtAppAvrcpCbk)pt_args[0].u.pv_func;
        if (NULL != p_bt_avrcp_app_cb_func)
        {
            apv_cb_addr[0] = (VOID*)p_bt_avrcp_app_cb_func;
            pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
                pt_args[1].u.pv_arg);

            BT_AVRCP_HDL_LOG("pv_func=%p, pv_arg=%p, pt_nfy_tag=%p",
                p_bt_avrcp_app_cb_func, pt_args[1].u.pv_arg, pt_nfy_tag);

            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg =
                x_mtkapi_avrcp_register_callback(bt_app_avrcp_event_cbk_wrapper_agent,
                pt_nfy_tag);
            if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
            {
                ri_free_cb_tag(pt_nfy_tag);
            }
        }
        else
        {
            pt_return->u.i4_arg = x_mtkapi_avrcp_register_callback(NULL, NULL);
        }
    }
    BT_MW_RPC_AVRCP_UNLOCK();
/*******************************************************************/
/* Add client callback to pAvrcpLinkList  */
/*******************************************************************/
    p_bt_avrcp_app_cb_func = (mtkrpcapi_BtAppAvrcpCbk)pt_args[0].u.pv_func;
    INT32 i4_ret;
    AvrcpRegInfo* p_avrcp_info = (AvrcpRegInfo*)malloc(sizeof(AvrcpRegInfo));
    if(NULL == p_avrcp_info)
    {
        BT_AVRCP_HDL_LOG("New A2dpRegInfo fail. \n");
        return -1;
    }
    memset(p_avrcp_info, 0, sizeof(AvrcpRegInfo));

    p_avrcp_info->t_id = t_rpc_id;
    p_avrcp_info->app_func = p_bt_avrcp_app_cb_func;

    BT_MW_RPC_AVRCP_LOCK();
    i4_ret = AvrcpRegCbLinkList_Add(pAvrcpLinkList, p_avrcp_info);
    if(i4_ret != 0)
    {
        BT_AVRCP_HDL_LOG("AvrcpRegCbLinkList_Add fail. \n");
        if (p_avrcp_info != NULL)
        {
            free(p_avrcp_info);
        }
        BT_MW_RPC_AVRCP_UNLOCK();
        return -1;
    }
    else
    {
        bt_register_app_ipc_close_handle(t_rpc_id, &t_rpc_id_for_free, bt_app_avrcp_ipc_close_handle);
    }
    if (p_avrcp_info != NULL)
    {
        free(p_avrcp_info);
    }
    AvrcpRegCbLinkList_Show(pAvrcpLinkList);
    BT_MW_RPC_AVRCP_UNLOCK();
    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_avrcp_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_AVRCP_HDL_LOG("[_hndlr_]_hndlr_x_mtkapi_avrcp_register_callback, arg_1 = %d\n", pt_args[0].u.i4_arg);

    mtkrpcapi_BtAppAvrcpCbk p_bt_avrcp_app_cb_func = NULL;

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    p_bt_avrcp_app_cb_func = (mtkrpcapi_BtAppAvrcpCbk)pt_args[0].u.pv_func;

    if (p_bt_avrcp_app_cb_func == NULL)
    {
        BT_AVRCP_HDL_LOG("unregister callback is NULL. \n");
        return RPCR_INV_ARGS;
    }

/*******************************************************************/
/*  remove  avrcp callback from pAvrcpLinkList  */
/*******************************************************************/
    INT32 i4_ret;
    AvrcpRegInfo* p_avrcp_info = (AvrcpRegInfo*)malloc(sizeof(AvrcpRegInfo));
    if(NULL == p_avrcp_info)
    {
        BT_AVRCP_HDL_LOG("New AvrcpRegInfo fail. \n");
        return -1;
    }
    memset(p_avrcp_info, 0, sizeof(AvrcpRegInfo));

    p_avrcp_info->t_id = t_rpc_id;
    p_avrcp_info->app_func = p_bt_avrcp_app_cb_func;
    BT_MW_RPC_AVRCP_LOCK();
    i4_ret = AvrcpRegCbLinkList_Remove(pAvrcpLinkList, p_avrcp_info);
    if(i4_ret != 0)
    {
        BT_AVRCP_HDL_LOG("AvrcpRegCbLinkList_Remove fail. \n");
        if (p_avrcp_info != NULL)
        {
            free(p_avrcp_info);
        }
        BT_MW_RPC_AVRCP_UNLOCK();
        return -1;
    }
    if (p_avrcp_info != NULL)
    {
        free(p_avrcp_info);
    }
    AvrcpRegCbLinkList_Show(pAvrcpLinkList);
    if(0 == AvrcpRegCbLinkList_GetSize(pAvrcpLinkList))
    {
        AvrcpRegCbLinkList_FreeLinkList(pAvrcpLinkList);
        pAvrcpLinkList = NULL;
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_avrcp_unregister_callback();
    }
    BT_MW_RPC_AVRCP_UNLOCK();
    return RPCR_OK;

}

INT32 c_rpc_reg_mtk_bt_service_avrcp_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_avrcp_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_change_player_app_setting);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_send_passthrough_cmd);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_send_vendor_unique_cmd);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_change_volume);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_update_player_status);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_get_playback_state);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_send_playitem);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_get_now_playing_list);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_get_folder_list);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_get_player_list);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_change_folder_path);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_set_browsed_player);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_set_addressed_player);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_update_player_media_info);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_update_absolute_volume);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_unregister_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_avrcp_get_connected_dev_list);
    return RPCR_OK;
}


