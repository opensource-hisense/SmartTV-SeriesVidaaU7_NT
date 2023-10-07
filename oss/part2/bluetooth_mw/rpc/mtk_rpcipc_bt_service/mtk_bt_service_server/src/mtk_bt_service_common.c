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
#include <string.h>

#include "list.h"
#include "mtk_bt_service_common.h"

typedef struct
{
    IPC_ID_T t_rpc_id;
    void *pv_tag;
    ipc_close_fct pf_close;
} BT_CLOSE_CB_HANDLE_INFO;

typedef struct
{
    BT_CLOSE_CB_HANDLE_INFO cb_info;
    struct dl_list node;
} BT_CLOSE_CB_HANDLE_NODE;

static struct dl_list g_bt_mw_rpc_close_cb_handle_list =
    {&g_bt_mw_rpc_close_cb_handle_list, &g_bt_mw_rpc_close_cb_handle_list};


#define BT_COMMON_RH_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Handle]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)


static void bt_app_ipc_close_handle(void * pv_tag)
{
    BT_CLOSE_CB_HANDLE_NODE *cb = NULL;
    BT_CLOSE_CB_HANDLE_NODE *tmp = NULL;
    IPC_ID_T t_rpc_id = (IPC_ID_T)pv_tag;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_close_cb_handle_list, BT_CLOSE_CB_HANDLE_NODE, node)
    {
        if (cb->cb_info.t_rpc_id == t_rpc_id)
        {
            BT_COMMON_RH_LOG("call new handle: t_rpc_id = %d, pv_tag = %p, pf_close = %p",
                      t_rpc_id, cb->cb_info.pv_tag, cb->cb_info.pf_close);
            cb->cb_info.pf_close(cb->cb_info.pv_tag);
            dl_list_del(&cb->node);
            free(cb);
        }
    }
}

void bt_register_app_ipc_close_handle(IPC_ID_T t_rpc_id, void * pv_tag, ipc_close_fct pf_close)
{
    int is_reg = 0;
    BT_CLOSE_CB_HANDLE_NODE *cb = NULL;
    BT_CLOSE_CB_HANDLE_NODE *tmp = NULL;
    BT_CLOSE_CB_HANDLE_NODE *new_cb = NULL;

    dl_list_for_each_safe(cb, tmp, &g_bt_mw_rpc_close_cb_handle_list, BT_CLOSE_CB_HANDLE_NODE, node)
    {
        if (cb->cb_info.t_rpc_id == t_rpc_id)
        {
            is_reg = 1;
            break;
        }
    }

    new_cb = (BT_CLOSE_CB_HANDLE_NODE *)malloc(sizeof(BT_CLOSE_CB_HANDLE_NODE));
    if (NULL == new_cb)
    {
        BT_COMMON_RH_LOG("malloc BT_CLOSE_CB_HANDLE_NODE fail. \n");
        return;
    }

    memset((void*)new_cb, 0, sizeof(BT_CLOSE_CB_HANDLE_NODE));
    new_cb->cb_info.t_rpc_id = t_rpc_id;
    new_cb->cb_info.pv_tag = pv_tag;
    new_cb->cb_info.pf_close = pf_close;
    dl_list_add(&g_bt_mw_rpc_close_cb_handle_list, &new_cb->node);
    BT_COMMON_RH_LOG("reg new handle: t_rpc_id = %d, pv_tag = %p, pf_close = %p",
                      t_rpc_id, pv_tag, pf_close);

    if (is_reg == 0) {
        BT_COMMON_RH_LOG("ipc_set_close_fct: t_rpc_id = %d", t_rpc_id);
        ipc_set_close_fct(t_rpc_id, (void *)t_rpc_id, bt_app_ipc_close_handle);
    }
}

