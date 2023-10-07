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

#include "mtk_bt_service_leaudio_bmr_wrapper.h"
#include "mtk_bt_service_leaudio_bmr_ipcrpc_struct.h"
#include "client_common.h"
#include "u_bt_mw_leaudio_bmr.h"

#define BT_RW_LOG(stmt, ...) \
        do{ \
            printf("[BMR]Func:%s Line:%d--->: "stmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
        }   \
        while(0)

static INT32 _hndlr_bt_app_bmr_event_cbk(
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

    ((mtkrpcapi_BtAppBmrCbk)pv_cb_addr)
        ((BT_BMR_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_discover(BT_BMR_DISCOVERY_PARAMS *params)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_discover");

    if (params == NULL) {
        BT_RW_LOG("a_mtkapi_bt_bmr_discover invalid params");
        return RPCR_INV_ARGS;
    }
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_BMR_DISCOVERY_PARAMS,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_discover");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_solicitation_request(BT_BMR_SOLICITATION_REQUEST_PARAMS *params)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_solicitation_request");

    if (params == NULL) {
        BT_RW_LOG("a_mtkapi_bt_bmr_solicitation_request invalid params");
        return RPCR_INV_ARGS;
    }
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_BMR_SOLICITATION_REQUEST_PARAMS,
                    NULL));
    if (params->user_data != NULL && params->user_data_len > 0) {
        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                        params->user_data,
                        params->user_data_len));
    }
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_solicitation_request");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_disconnect(CHAR *bdaddr)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bdaddr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_refresh_source(UINT8 source_id)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_refresh_source");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, source_id);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_refresh_source");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_remove_source(BOOL all, UINT8 source_id)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_remove_source");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, all);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, source_id);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_remove_source");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_set_broadcast_code(BT_BMR_SET_BROADCAST_CODE_PARAMS *params)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_set_broadcast_code");

    if (params == NULL) {
        BT_RW_LOG("a_mtkapi_bt_bmr_set_broadcast_code invalid params");
        return RPCR_INV_ARGS;
    }
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_BMR_SET_BROADCAST_CODE_PARAMS,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_set_broadcast_code");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_streaming_start(UINT8 source_id, UINT32 bis_mask)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_streaming_start");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, source_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, bis_mask);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_streaming_start");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_streaming_stop(UINT8 source_id, UINT32 bis_mask)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_streaming_stop");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, source_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, bis_mask);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_streaming_stop");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_set_pac_config(UINT8 pac_type, UINT32 value)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_set_pac_config");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, pac_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, value);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_set_pac_config");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_close(BOOL open_to_bsa)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_close");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, open_to_bsa);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_close");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_dump(BOOL all, UINT8 source_id, BOOL full_info)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_dump");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, all);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, source_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, full_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_dump");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/*******************************************************************************
**
** Function         a_mtkapi_bt_bmr_register_callback
**
** Description      The function is used by user to register a BMR event callback.
**                      User can receive BMR event by this handler.
** Returns          BT_SUCCESS -- register succesfully
**                      Others     -- register failed
*******************************************************************************/
EXPORT_SYMBOL INT32 a_mtkapi_bt_bmr_register_callback(mtkrpcapi_BtAppBmrCbk bmr_handle, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_bmr_register_callback");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, bmr_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bmr_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 c_rpc_reg_mtk_bt_service_bmr_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_bmr_event_cbk);
    return RPCR_OK;
}
