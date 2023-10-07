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

#include "mtk_bt_service_leaudio_bms_wrapper.h"
#include "mtk_bt_service_leaudio_bms_ipcrpc_struct.h"
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

static INT32 _hndlr_bt_app_bms_event_cbk(
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

    ((mtkrpcapi_BtAppBmsCbk)pv_cb_addr)
        ((BT_BMS_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

/*******************************************************************************
**
** Function         a_mtkapi_bt_bms_register_callback
**
** Description      The function is used by user to register a bms event callback.
**                      User can receive bms event by this handler.
** Returns          BT_SUCCESS -- register succesfully
**                      Others     -- register failed
*******************************************************************************/
EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_register_callback(mtkrpcapi_BtAppBmsCbk bms_cb, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_bms_register_callback");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, bms_cb);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_create_broadcast");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_CREATE_BROADCAST_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_create_broadcast");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_create_broadcast_ext");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_CREATE_BROADCAST_EXT_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_create_broadcast_ext");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_update_base_announcement");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_update_base_announcement");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_update_subgroup_metadata");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_update_subgroup_metadata");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_start_broadcast");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_START_BROADCAST_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_start_broadcast");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_start_broadcast_multi_thread");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_start_broadcast_multi_thread");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_pause_broadcast");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_PAUSE_BROADCAST_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_pause_broadcast");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_stop_broadcast");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_STOP_BROADCAST_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_stop_broadcast");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
    BT_RW_LOG("a_mtkapi_bt_bms_get_own_address");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                     RPC_DESC_BT_BMS_GET_OWN_ADDRESS_PARAM, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_get_own_address");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_get_all_broadcasts_states(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_bms_get_all_broadcasts_states");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_get_all_broadcasts_states");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bms_stop_all_broadcast(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_bms_stop_all_broadcast");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bms_stop_all_broadcast");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 c_rpc_reg_mtk_bt_service_bms_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_bms_event_cbk);
    return RPCR_OK;
}
