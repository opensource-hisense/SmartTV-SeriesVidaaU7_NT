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

#include "mtk_bt_service_gattc.h"
#include "mtk_bt_service_gattc_handle.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"
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


static VOID bt_app_gattc_event_cbk_wrapper(BT_GATTC_EVENT_PARAM *param,
    void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_GATTC_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_event_cbk",
        pt_nfy_tag->pv_cb_addr);

    RPC_RETURN_VOID;
}


static VOID bt_app_gattc_ipc_close_handle(VOID*  pt_nfy_tag)
{
    if (NULL == pt_nfy_tag) return;

    x_mtkapi_bt_gattc_ipc_close_notify((RPC_CB_NFY_TAG_T *)pt_nfy_tag);
    ri_free_cb_tag(pt_nfy_tag);
}


static INT32 _hndlr_x_mtkapi_bt_gattc_register(RPC_ID_T t_rpc_id,
                                               const CHAR*  ps_cb_type,
                                               UINT32       ui4_num_args,
                                               ARG_DESC_T*  pt_args,
                                               ARG_DESC_T*  pt_return)
{
    mtkrpcapi_BtAppGATTCCbk p_bt_gattc_app_cb_func = NULL;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};

    if(ui4_num_args != 3)
    {
        return RPCR_INV_ARGS;
    }

    p_bt_gattc_app_cb_func = (mtkrpcapi_BtAppGATTCCbk)pt_args[1].u.pv_func;
    if (NULL != p_bt_gattc_app_cb_func)
    {
        apv_cb_addr[0] = (VOID*)p_bt_gattc_app_cb_func;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
            pt_args[2].u.pv_arg);

        printf("t_rpc_id=%d, pv_func=%p, pv_arg=%p, pt_nfy_tag=%p\n",
            t_rpc_id, p_bt_gattc_app_cb_func, pt_args[2].u.pv_arg, pt_nfy_tag);

        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg =
            x_mtkapi_bt_gattc_register(pt_args[0].u.pc_arg,
                bt_app_gattc_event_cbk_wrapper,
                pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = BT_ERR_STATUS_FAIL;
        }
        else
        {
            BT_RH_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d pt_nfy_tag=%p",
                     t_rpc_id, pt_nfy_tag);
            bt_register_app_ipc_close_handle(t_rpc_id, pt_nfy_tag, bt_app_gattc_ipc_close_handle);
        }
    }
    else
    {
        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg = BT_ERR_STATUS_NULL_POINTER;
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_unregister(RPC_ID_T t_rpc_id,
                                                  const CHAR*  ps_cb_type,
                                                  UINT32       ui4_num_args,
                                                  ARG_DESC_T*  pt_args,
                                                  ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_unregister_app, arg_1 = %d\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_unregister(pt_args[0].u.i4_arg);
    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_gattc_connect(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("_hndlr_x_mtkapi_bt_gattc_connect\n");
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_connect((BT_GATTC_CONNECT_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_disconnect(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("_hndlr_x_mtkapi_bt_gattc_connect\n");
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_disconnect((BT_GATTC_DISCONNECT_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_refresh(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_refresh((BT_GATTC_REFRESH_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_discover_by_uuid(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_discover_by_uuid((BT_GATTC_DISCOVER_BY_UUID_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_char(RPC_ID_T t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_char((BT_GATTC_READ_CHAR_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_char_by_uuid(RPC_ID_T t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_char_by_uuid((BT_GATTC_READ_BY_UUID_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_write_char(RPC_ID_T t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_write_char((BT_GATTC_WRITE_CHAR_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_desc(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_desc((BT_GATTC_READ_DESC_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_gattc_write_desc(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_write_desc((BT_GATTC_WRITE_DESC_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_exec_write(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_exec_write((BT_GATTC_EXEC_WRITE_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_reg_notification(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_reg_notification((BT_GATTC_REG_NOTIF_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_rssi(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_rssi((BT_GATTC_READ_RSSI_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_get_dev_type(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_get_dev_type((BT_GATTC_GET_DEV_TYPE_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_change_mtu(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_change_mtu((BT_GATTC_CHG_MTU_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_conn_update(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_conn_update((BT_GATTC_CONN_UPDATE_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_set_prefer_phy(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_set_prefer_phy((BT_GATTC_PHY_SET_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_phy(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_phy((BT_GATTC_PHY_READ_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_get_gatt_db(RPC_ID_T     t_rpc_id,
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
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_get_gatt_db((BT_GATTC_GET_GATT_DB_PARAM *)pt_args[0].u.pc_arg);
    return RPCR_OK;
}


INT32 c_rpc_reg_mtk_bt_service_gattc_op_hndlrs(VOID)
{
    x_mtkapi_bt_gattc_init();
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_register);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_unregister);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_refresh);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_discover_by_uuid);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_char);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_char_by_uuid);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_desc);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_write_char);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_write_desc);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_exec_write);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_reg_notification);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_rssi);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_dev_type);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_change_mtu);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_conn_update);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_set_prefer_phy);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_phy);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_gatt_db);
    return RPCR_OK;
}


