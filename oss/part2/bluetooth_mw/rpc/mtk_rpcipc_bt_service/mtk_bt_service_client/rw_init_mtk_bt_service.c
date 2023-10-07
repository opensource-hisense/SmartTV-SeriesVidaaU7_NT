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

#include <stdio.h>
#include <string.h>

#include "_ipc.h"
#include "rpc.h"
#include "list.h"
#include "c_mw_config.h"

#include "rw_init_mtk_bt_service.h"
#include "init_mtk_bt_service_client.h"
#include "_rpc_ipc_util.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_spp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"
#include "mtk_bt_service_mesh_wrapper.h"
#include "mtk_bt_service_spp_wrapper.h"
#include "mtk_bt_service_hidh_wrapper.h"
#include "mtk_bt_service_ble_scanner_wrapper.h"
#include "mtk_bt_service_ble_advertiser_wrapper.h"
#include "mtk_bt_service_leaudio_bms_wrapper.h"
#include "mtk_bt_service_leaudio_bass_wrapper.h"
#include "mtk_bt_service_leaudio_bmr_wrapper.h"

static BOOL b_imtk_bt_service_client_sys_init = FALSE;
static RPC_ID_T b_imtk_bt_service_client_rpc_id = 0;

typedef struct
{
    void *pv_tag;
    ipc_close_fct pf_close;
} BT_STACK_CLOSE_CB_HANDLE_INFO;

typedef struct
{
    BT_STACK_CLOSE_CB_HANDLE_INFO cb_info;
    struct dl_list node;
} BT_STACK_CLOSE_CB_HANDLE_NODE;

static struct dl_list g_bt_stack_rpc_close_cb_handle_list =
    {&g_bt_stack_rpc_close_cb_handle_list, &g_bt_stack_rpc_close_cb_handle_list};

EXPORT_SYMBOL RPC_ID_T c_rpc_start_mtk_bt_service_client(void)
{
  return bt_rpc_open_client("mtk_bt_service");
}

EXPORT_SYMBOL RPC_ID_T c_rpc_init_mtk_bt_service_client(void)
{
  return c_rpc_start_mtk_bt_service_client();
}

INT32 c_rpc_uninit_mtk_bt_service_client(RPC_ID_T t_rpc_id)
{
  bt_rpc_close_client(t_rpc_id);
  bt_rpc_del(t_rpc_id);
  return RPCR_OK;
}

EXPORT_SYMBOL void a_mtk_bt_service_init(void)
{
    if (!b_imtk_bt_service_client_sys_init)
    {
        b_imtk_bt_service_client_sys_init = TRUE;
        bt_rpc_init(NULL);

        b_imtk_bt_service_client_rpc_id = c_rpc_init_mtk_bt_service_client();

        bt_rpcu_tl_log_start();
        c_rpc_reg_mtk_bt_service_a2dp_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_avrcp_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_gap_cb_hndlrs();
#if ENABLE_GATT_PROFILE
        c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_gatts_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_ble_scanner_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_ble_adv_cb_hndlrs();
#endif
#if ENABLE_SPP_PROFILE
        c_rpc_reg_mtk_bt_service_spp_cb_hndlrs();
#endif
#if ENABLE_HID_PROFILE_H
        c_rpc_reg_mtk_bt_service_hidh_cb_hndlrs();
#endif
#if MTK_LINUX_HFP
        c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs();
#endif
#if ENABLE_BLE_MESH
        c_rpc_reg_mtk_bt_service_mesh_cb_hndlrs();
#endif
#if MTK_LEAUDIO_BMS
        c_rpc_reg_mtk_bt_service_bms_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_bass_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_bmr_cb_hndlrs();
#endif
    }
}

EXPORT_SYMBOL void a_mtk_bt_service_terminate(void)
{
    BT_STACK_CLOSE_CB_HANDLE_NODE *cb = NULL;
    BT_STACK_CLOSE_CB_HANDLE_NODE *tmp = NULL;

    if (b_imtk_bt_service_client_sys_init)
    {
        b_imtk_bt_service_client_sys_init = FALSE;
        bt_rpcu_tl_log_end();
        c_rpc_uninit_mtk_bt_service_client(b_imtk_bt_service_client_rpc_id);
        bt_rpc_uninit();
        dl_list_for_each_safe(cb, tmp, &g_bt_stack_rpc_close_cb_handle_list, BT_STACK_CLOSE_CB_HANDLE_NODE, node)
        {
            printf("%s free cb: pv_tag = %p, pf_close = %p\n",
                      __FUNCTION__, cb->cb_info.pv_tag, cb->cb_info.pf_close);
            dl_list_del(&cb->node);
            free(cb);
        }

        b_imtk_bt_service_client_rpc_id = 0;
    }
}


static void bt_stack_ipc_close_handle(void * pv_tag)
{
    BT_STACK_CLOSE_CB_HANDLE_NODE *cb = NULL;
    BT_STACK_CLOSE_CB_HANDLE_NODE *tmp = NULL;
    IPC_ID_T t_rpc_id = (IPC_ID_T)pv_tag;

    dl_list_for_each_safe(cb, tmp, &g_bt_stack_rpc_close_cb_handle_list, BT_STACK_CLOSE_CB_HANDLE_NODE, node)
    {
        printf("%s call new handle: t_rpc_id = %d, pv_tag = %p, pf_close = %p\n",
                  __FUNCTION__, t_rpc_id, cb->cb_info.pv_tag, cb->cb_info.pf_close);
        cb->cb_info.pf_close(cb->cb_info.pv_tag);
        dl_list_del(&cb->node);
        free(cb);
    }
}

EXPORT_SYMBOL int a_mtk_bt_register_stack_exit_cb(stack_exit_cb pf_close, void * pv_tag)
{
    int is_reg = 0;
    BT_STACK_CLOSE_CB_HANDLE_NODE *new_cb = NULL;

    if (b_imtk_bt_service_client_rpc_id < 0)
    {
        printf("%s please call a_mtk_bt_service_init first \n", __FUNCTION__);
        return -1;
    }

    is_reg = dl_list_len(&g_bt_stack_rpc_close_cb_handle_list) == 0 ? 0 : 1;

    new_cb = (BT_STACK_CLOSE_CB_HANDLE_NODE *)malloc(sizeof(BT_STACK_CLOSE_CB_HANDLE_NODE));
    if (NULL == new_cb)
    {
        printf("%s malloc BT_STACK_CLOSE_CB_HANDLE_NODE fail. \n", __FUNCTION__);
        return -1;
    }

    memset((void*)new_cb, 0, sizeof(BT_STACK_CLOSE_CB_HANDLE_NODE));
    new_cb->cb_info.pv_tag = pv_tag;
    new_cb->cb_info.pf_close = pf_close;
    dl_list_add(&g_bt_stack_rpc_close_cb_handle_list, &new_cb->node);
    printf("%s reg new handle: pv_tag = %p, pf_close = %p\n",
                      __FUNCTION__, pv_tag, pf_close);

    if (is_reg == 0) {
        printf("%s ipc_set_close_fct: t_rpc_id = %d\n",
            __FUNCTION__, b_imtk_bt_service_client_rpc_id);
        ipc_set_close_fct(b_imtk_bt_service_client_rpc_id,
            (void *)b_imtk_bt_service_client_rpc_id,
            bt_stack_ipc_close_handle);
    }
    return 0;
}

