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

#include "mtk_bt_service_common.h"
#include "mtk_bt_service_leaudio_bms.h"
#include "mtk_bt_service_leaudio_bms_ipcrpc_struct.h"
#include "mtk_bt_service_leaudio_bms_handle.h"
#include "u_rpc.h"
#include "ri_common.h"
#include "_ipc.h"

BMSRegCbLinkList* pBMSLinkList = NULL;

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static pthread_mutex_t g_bt_mw_rpc_bms_lock;
#define BT_MW_RPC_BMS_LOCK() \
        do { \
            pthread_mutex_lock(&(g_bt_mw_rpc_bms_lock)); \
        }while(0)
#define BT_MW_RPC_BMS_UNLOCK() \
        do { \
            pthread_mutex_unlock(&(g_bt_mw_rpc_bms_lock));\
        }while(0)


static VOID bt_app_bms_event_cbk_wrapper_agent(BT_BMS_EVENT_PARAM *param,
    void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BMS_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    //RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_a2dp_event_cbk", pt_nfy_tag->pv_cb_addr);

    BT_MW_RPC_BMS_LOCK();
    if (pBMSLinkList && (pBMSLinkList->i4_size > 0))
    {
        BMSRegCbNode* pCurrent = pBMSLinkList->pHead->next;
        while(pCurrent != NULL)
        {
            //BT_RH_LOG("current bt_event_cb = %p.", pCurrent->bms_info.app_func);

            if (NULL != pCurrent->bms_info.app_func)
            {
                RPC_DO_CB(pCurrent->bms_info.t_id, "bt_app_bms_event_cbk", pCurrent->bms_info.app_func);
            }
            else
            {
                BT_RH_LOG("skip NULL bt_event_cb.");
            }

            pCurrent = pCurrent->next;
        }
    }
    BT_MW_RPC_BMS_UNLOCK();

    //BT_RH_LOG("bt_app_bms_event_cbk_wrapper_agent done.");
    RPC_RETURN_VOID;
}


static BMSRegCbLinkList* BMSRegCbLinkList_Create()
{
    BT_RH_LOG("Multi bms create linklist start...");
    BMSRegCbLinkList* pLinkList = (BMSRegCbLinkList*)malloc(sizeof(BMSRegCbLinkList));
    if (NULL == pLinkList)
    {
        BT_RH_LOG("New BMSRegCbLinkList fail!");
        return NULL;
    }
    pLinkList->i4_size = 0;
    pLinkList->pHead = (BMSRegCbNode*)malloc(sizeof(BMSRegCbNode));
    if (NULL == pLinkList->pHead)
    {
        BT_RH_LOG("New BMSRegCbNode fail!");
        if (NULL != pLinkList)
        {
            free(pLinkList);
        }
        return NULL;
    }
    memset(&pLinkList->pHead->bms_info, 0, sizeof(BMSRegInfo));
    pLinkList->pHead->next = NULL;
    BT_RH_LOG("Multi bms create linklist complete.");

    return pLinkList;
}

static INT32 BMSRegCbLinkList_FreeLinkList(BMSRegCbLinkList* pLinkList)
{
    BT_RH_LOG("Remove link list start...");
    if (NULL == pLinkList)
    {
        BT_RH_LOG("Invalid parameter.");
        return -1;
    }
    if (NULL == pLinkList->pHead)
    {
        free(pLinkList);
    }
    else
    {
        BMSRegCbNode* pCurrent = pLinkList->pHead->next;
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
    BT_RH_LOG("Remove link list complete!");
    return 0;
}


INT32 BMSRegCbLinkList_Add(BMSRegCbLinkList* pLinkList, BMSRegInfo* p_bms_info)
{
    BT_RH_LOG("\n");
    if(NULL == pLinkList || NULL == p_bms_info)
    {
        BT_RH_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > BMSRegMax)
    {
        BT_RH_LOG("bms register exceeds the maximum. \n");
        return -1;
    }

    BMSRegCbNode* pNode = (BMSRegCbNode*)malloc(sizeof(BMSRegCbNode));
    if(NULL == pNode)
    {
        BT_RH_LOG("New BMSRegCbNode fail. \n");
        return -1;
    }
    memcpy(&pNode->bms_info, p_bms_info, sizeof(BMSRegInfo));
    pNode->next = NULL;

    BMSRegCbNode* pCurrent = pLinkList->pHead;
    while(pCurrent->next != NULL)
    {
        pCurrent = pCurrent->next;
    }
    pCurrent->next = pNode;
    pLinkList->i4_size++;

    return 0;
}

INT32 BMSRegCbLinkList_Remove(BMSRegCbLinkList* pLinkList, BMSRegInfo* p_bms_info)
{
    BT_RH_LOG("\n");
    if(NULL == pLinkList || NULL == p_bms_info)
    {
        BT_RH_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > BMSRegMax)
    {
        BT_RH_LOG("BMS register exceeds the maximum. \n");
        return -1;
    }

    BMSRegCbNode* pPre = pLinkList->pHead;
    BMSRegCbNode* pCurrent = pPre->next;
    while(pCurrent != NULL)
    {
        if (p_bms_info->app_func == pCurrent->bms_info.app_func)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                free(pCurrent);
            }
            BT_RH_LOG("Remove BMS info success. callback = %p", p_bms_info->app_func);
            pLinkList->i4_size--;
            return 0;
        }
        pPre = pCurrent;
        pCurrent = pCurrent->next;
    }

    return -1;
}

INT32 BMSRegCbLinkList_RemoveBytid(BMSRegCbLinkList* pLinkList, BMSRegInfo* p_bms_info)
{
    BT_RH_LOG("\n");
    if(NULL == pLinkList || NULL == p_bms_info)
    {
        BT_RH_LOG("Invalid parameter. \n");
        return -1;
    }
    if(pLinkList->i4_size > BMSRegMax)
    {
        BT_RH_LOG("BMS register exceeds the maximum. \n");
        return -1;
    }

    BMSRegCbNode* pPre = pLinkList->pHead;
    BMSRegCbNode* pCurrent = pPre->next;
    while(pCurrent != NULL)
    {
        if (p_bms_info->t_id == pCurrent->bms_info.t_id)
        {
            pPre->next = pCurrent->next;
            if(NULL != pCurrent)
            {
                BT_RH_LOG("Remove bms info success. callback = %p", pCurrent->bms_info.app_func);
                free(pCurrent);
            }
            pLinkList->i4_size--;
            return 0;
        }
        pPre = pCurrent;
        pCurrent = pCurrent->next;
    }

    return -1;
}


static INT32 BMSRegCbLinkList_GetSize(BMSRegCbLinkList* pLinkList)
{
    if(NULL == pLinkList)
    {
        BT_RH_LOG("Invalid parameter.");
        return 0;
    }
    BT_RH_LOG("Get multi bms linklist size. size=%d", pLinkList->i4_size);
    return pLinkList->i4_size;
}

static VOID BMSRegCbLinkList_Show(BMSRegCbLinkList* pLinkList)
{
    BT_RH_LOG("\n");
    if(NULL == pLinkList)
    {
        BT_RH_LOG("Invalid parameter. \n");
        return;
    }
    BT_RH_LOG("bms registed number: %d \n", pLinkList->i4_size);
    BMSRegCbNode* pCurrent = pLinkList->pHead->next;
    while(pCurrent)
    {
        BT_RH_LOG("register_cb = %p \n", pCurrent->bms_info.app_func);
        pCurrent = pCurrent->next;
    }
    return;

}

static VOID bt_app_bms_ipc_close_handle(VOID* p_tid)
{
    INT32 i4_ret;
    BMSRegInfo  bms_reg_info;
    memset(&bms_reg_info, 0, sizeof(BMSRegInfo));
    bms_reg_info.t_id = ((RPC_ID_T)(*(RPC_ID_T*)p_tid));

    BT_RH_LOG("p_tid=%p tid=%d\n", p_tid, ((RPC_ID_T)(*(RPC_ID_T*)p_tid)));
    BT_RH_LOG("BMSRegCbLinkList_RemoveBytid bms_reg_info.t_id=%d \n", bms_reg_info.t_id);
    BT_MW_RPC_BMS_LOCK();
    i4_ret = BMSRegCbLinkList_RemoveBytid(pBMSLinkList, &bms_reg_info);
    if(i4_ret != 0)
    {
        BT_RH_LOG("BMSRegCbLinkList_RemoveBytid fail. \n");
        BT_MW_RPC_BMS_UNLOCK();
        return;
    }
    BMSRegCbLinkList_Show(pBMSLinkList);
    if(0 == BMSRegCbLinkList_GetSize(pBMSLinkList))
    {
        BMSRegCbLinkList_FreeLinkList(pBMSLinkList);
        pBMSLinkList = NULL;
        x_mtkapi_bt_bms_unregister_callback();
    }
    BT_MW_RPC_BMS_UNLOCK();
}

static RPC_ID_T t_rpc_id_for_free = -1;
static INT32 _hndlr_x_mtkapi_bt_bms_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    mtkrpcapi_BtAppBmsCbk p_bt_bms_app_cb_func = NULL;
    VOID * apv_cb_addr[1] = {0};
    t_rpc_id_for_free = t_rpc_id;
    BT_RH_LOG("t_rpc_id=%d", t_rpc_id_for_free);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    BT_RH_LOG("pv_func=%p, t_rpc_id=%d",
                pt_args[0].u.pv_func, t_rpc_id);

/*******************************************************************/
/*      The first client need create pBMSLinkList when register callback             */
/*******************************************************************/
    BT_MW_RPC_BMS_LOCK();
    if (NULL == pBMSLinkList)
    {
        pBMSLinkList = BMSRegCbLinkList_Create();
        RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;

        p_bt_bms_app_cb_func = (mtkrpcapi_BtAppBmsCbk)pt_args[0].u.pv_func;
        if (NULL != p_bt_bms_app_cb_func)
        {
            apv_cb_addr[0] = (VOID*)p_bt_bms_app_cb_func;
            pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
                pt_args[1].u.pv_arg);

            BT_RH_LOG("pv_func=%p, pv_arg=%p, pt_nfy_tag=%p",
                p_bt_bms_app_cb_func, pt_args[1].u.pv_arg, pt_nfy_tag);

            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg =
                x_mtkapi_bt_bms_register_callback(bt_app_bms_event_cbk_wrapper_agent,
                pt_nfy_tag);
            if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
            {
                ri_free_cb_tag(pt_nfy_tag);
            }
        }
        else
        {
            pt_return->u.i4_arg = x_mtkapi_bt_bms_register_callback(NULL, NULL);
        }
    }
    BT_MW_RPC_BMS_UNLOCK();

/*******************************************************************/
/* Add client callback to pBMSLinkList  */
/*******************************************************************/
    p_bt_bms_app_cb_func = (mtkrpcapi_BtAppBmsCbk)pt_args[0].u.pv_func;
    INT32 i4_ret;
    BMSRegInfo* p_bms_info = (BMSRegInfo*)malloc(sizeof(BMSRegInfo));
    if(NULL == p_bms_info)
    {
        BT_RH_LOG("New BMSRegInfo fail. \n");
        return -1;
    }
    memset(p_bms_info, 0, sizeof(BMSRegInfo));

    p_bms_info->t_id = t_rpc_id;
    p_bms_info->app_func = p_bt_bms_app_cb_func;


    BT_MW_RPC_BMS_LOCK();
    i4_ret = BMSRegCbLinkList_Add(pBMSLinkList, p_bms_info);
    BT_MW_RPC_BMS_UNLOCK();
    free(p_bms_info);
    if(i4_ret != 0)
    {
        BT_RH_LOG("BMSRegCbLinkList_Add fail. \n");
        return -1;
    }
    else
    {
        BT_RH_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d", t_rpc_id);
        bt_register_app_ipc_close_handle(t_rpc_id, &t_rpc_id_for_free, bt_app_bms_ipc_close_handle);
    }
    BT_MW_RPC_BMS_LOCK();
    BMSRegCbLinkList_Show(pBMSLinkList);
    BT_MW_RPC_BMS_UNLOCK();

    return RPCR_OK;

}


static INT32 _hndlr_x_mtkapi_bt_bms_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]_hndlr_x_mtkapi_bt_bms_unregister_callback, arg_1 = %d\n", pt_args[0].u.i4_arg);

    mtkrpcapi_BtAppBmsCbk p_bt_bms_app_cb_func = NULL;

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    p_bt_bms_app_cb_func = (mtkrpcapi_BtAppBmsCbk)pt_args[0].u.pv_func;

    if (p_bt_bms_app_cb_func == NULL)
    {
        BT_RH_LOG("unregister callback is NULL. \n");
        return RPCR_INV_ARGS;
    }

/*******************************************************************/
/*  remove  bms callback from pBMSLinkList  */
/*******************************************************************/
    INT32 i4_ret;
    BMSRegInfo* p_bms_info = (BMSRegInfo*)malloc(sizeof(BMSRegInfo));
    if(NULL == p_bms_info)
    {
        BT_RH_LOG("New BmsRegInfo fail. \n");
        return -1;
    }
    memset(p_bms_info, 0, sizeof(BMSRegInfo));

    p_bms_info->t_id = t_rpc_id;
    p_bms_info->app_func = p_bt_bms_app_cb_func;

    BT_MW_RPC_BMS_LOCK();
    i4_ret = BMSRegCbLinkList_Remove(pBMSLinkList, p_bms_info);
    free(p_bms_info);
    if(i4_ret != 0)
    {
        BT_RH_LOG("BMSRegCbLinkList_Add fail. \n");
        BT_MW_RPC_BMS_UNLOCK();
        return -1;
    }
    BMSRegCbLinkList_Show(pBMSLinkList);
    if(0 == BMSRegCbLinkList_GetSize(pBMSLinkList))
    {
        BMSRegCbLinkList_FreeLinkList(pBMSLinkList);
        pBMSLinkList = NULL;
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = x_mtkapi_bt_bms_unregister_callback();
    }
    BT_MW_RPC_BMS_UNLOCK();
    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_bt_bms_create_broadcast(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_create_broadcast((BT_BMS_CREATE_BROADCAST_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_create_broadcast_ext(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_create_broadcast_ext((BT_BMS_CREATE_BROADCAST_EXT_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_update_base_announcement(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_update_base_announcement((BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_update_subgroup_metadata(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_update_subgroup_metadata((BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_start_broadcast(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_start_broadcast((BT_BMS_START_BROADCAST_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_start_broadcast_multi_thread(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_start_broadcast_multi_thread((BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_pause_broadcast(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_pause_broadcast((BT_BMS_PAUSE_BROADCAST_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_stop_broadcast(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_stop_broadcast((BT_BMS_STOP_BROADCAST_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_get_own_address(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_get_own_address((BT_BMS_GET_OWN_ADDRESS_PARAM *)pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_get_all_broadcasts_states(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_get_all_broadcasts_states();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bms_stop_all_broadcast(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bms_stop_all_broadcast();

    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_bms_op_hndlrs(VOID)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_bms_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_unregister_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_create_broadcast);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_create_broadcast_ext);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_update_base_announcement);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_update_subgroup_metadata);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_start_broadcast);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_start_broadcast_multi_thread);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_pause_broadcast);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_stop_broadcast);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_get_own_address);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_get_all_broadcasts_states);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bms_stop_all_broadcast);
    return RPCR_OK;
}

