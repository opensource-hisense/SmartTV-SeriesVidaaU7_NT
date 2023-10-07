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

#include "mtk_bt_service_spp_wrapper.h"
#include "mtk_bt_service_spp_ipcrpc_struct.h"
#include "client_common.h"


#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_spp_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppSppCbk)pv_cb_addr)
        ((BT_SPP_CBK_STRUCT*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_register_callback(mtkrpcapi_BtAppSppCbk spp_cb, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_spp_register_callback");
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, spp_cb);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    BT_RW_LOG("spp_cb=%p, pv_tag=%p", spp_cb, pv_tag);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_unregister_callback(mtkrpcapi_BtAppSppCbk spp_cb, VOID* pv_tag)
{
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, spp_cb);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    BT_RW_LOG("spp_cb=%p, pv_tag=%p", spp_cb, pv_tag);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_unregister_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_connect(BT_SPP_CONNECT_PARAM *spp_connect_param)
{
    BT_RW_LOG("a_mtkapi_spp_connect");
    RPC_CLIENT_DECL(1, INT32);

    if (NULL == spp_connect_param) return BT_ERR_STATUS_NULL_POINTER;
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_connect_param,
        RPC_DESC_BT_SPP_CONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, spp_connect_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_disconnect(BT_SPP_DISCONNECT_PARAM *spp_disconnect_param)
{
    BT_RW_LOG("a_mtkapi_spp_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_disconnect_param,
        RPC_DESC_BT_SPP_DISCONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, spp_disconnect_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_send_data(BT_SPP_SEND_DATA_PARAM *spp_send_data_param)
{
    BT_RW_LOG("a_mtkapi_spp_send_data");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_send_data_param,
        RPC_DESC_BT_SPP_SEND_DATA_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, spp_send_data_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_send_data");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_start_server(BT_SPP_START_SVR_PARAM *spp_start_svr_param)
{
    BT_RW_LOG("a_mtkapi_spp_start_server");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_start_svr_param,
        RPC_DESC_BT_SPP_START_SVR_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, spp_start_svr_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_start_server");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_stop_server(BT_SPP_STOP_SVR_PARAM *spp_stop_svr_param)
{
    BT_RW_LOG("a_mtkapi_spp_stop_server");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_stop_svr_param,
        RPC_DESC_BT_SPP_STOP_SVR_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, spp_stop_svr_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_stop_server");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_spp_get_connection_info(BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db)
{
    BT_RW_LOG("a_mtkapi_spp_get_connection_info");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, spp_connection_info_db,
        RPC_DESC_BT_SPP_CONNECTION_INFO_DB, NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, spp_connection_info_db);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_spp_get_connection_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 c_rpc_reg_mtk_bt_service_spp_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_spp_cbk);
    return RPCR_OK;
}


