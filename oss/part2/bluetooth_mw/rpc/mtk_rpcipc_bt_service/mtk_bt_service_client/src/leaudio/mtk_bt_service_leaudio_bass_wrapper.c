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

#include "mtk_bt_service_leaudio_bass_wrapper.h"
#include "mtk_bt_service_leaudio_bass_ipcrpc_struct.h"
#include "client_common.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_bass_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppBassCbk)pv_cb_addr)
        ((BT_BASS_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_connect(BT_BASS_CONN_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_connect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_CONN_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_disconnect(BT_BASS_DISC_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_DISC_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_set_broadcast_scan(BT_BASS_BROADCAST_SCAN_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_set_broadcast_scan");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_BROADCAST_SCAN_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_set_broadcast_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_stop_broadcast_observing(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_bass_stop_broadcast_observing");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_stop_broadcast_observing");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_get_broadcast_receiver_state(BT_BASS_BROADCAST_RECV_STATE_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_get_broadcast_receiver_state");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_BROADCAST_RECV_STATE_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_get_broadcast_receiver_state");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_set_broadcast_code(BT_BASS_BROADCAST_CODE_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_set_broadcast_code");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_BROADCAST_CODE_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_set_broadcast_code");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_set_broadcast_source(BT_BASS_SET_BROADCAST_SRC_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_set_broadcast_source");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_SET_BROADCAST_SRC_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_set_broadcast_source");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_modify_broadcast_source(BT_BASS_MODIFY_BROADCAST_SRC_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_modify_broadcast_source");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_MODIFY_BROADCAST_SRC_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_modify_broadcast_source");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_remove_broadcast_source(BT_BASS_REMOVE_BROADCAST_SRC_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_remove_broadcast_source");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_REMOVE_BROADCAST_SRC_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_remove_broadcast_source");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_set_builtin_mode(BT_BASS_SET_BUILTIN_MODE_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bass_set_builtin_mode");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BASS_SET_BUILTIN_MODE_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_set_builtin_mode");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_get_connection_status(BT_BASS_CONNECTION_INFO *conn_info)
{
    BT_RW_LOG("a_mtkapi_bt_bass_get_connection_status");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, conn_info, RPC_DESC_BT_BASS_CONNECTION_INFO, NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, conn_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_get_connection_status");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bass_register_callback(mtkrpcapi_BtAppBassCbk bass_handle, VOID *pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_bass_register_callback");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, bass_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bass_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 c_rpc_reg_mtk_bt_service_bass_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_bass_event_cbk);
    return RPCR_OK;
}
