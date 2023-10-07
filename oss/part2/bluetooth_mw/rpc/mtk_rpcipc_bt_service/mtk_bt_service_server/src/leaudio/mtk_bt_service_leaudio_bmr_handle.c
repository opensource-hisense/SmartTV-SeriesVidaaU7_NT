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
#include "mtk_bt_service_leaudio_bmr.h"
#include "mtk_bt_service_leaudio_bmr_ipcrpc_struct.h"
#include "mtk_bt_service_leaudio_bmr_handle.h"
#include "u_bt_mw_leaudio_bmr.h"

#include "u_rpc.h"
#include "ri_common.h"

#define BT_RH_LOG(stmt, ...) printf("Func:%s Line:%d--->: " stmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

static VOID bt_app_bmr_event_cbk_wrapper(BT_BMR_EVENT_PARAM *param, void* pv_tag)
{
    if (param == NULL) {
        BT_RH_LOG("Invalid NULL ARGS\n");
        return;
    }
    BT_RH_LOG("event: %d\n", param->event);

    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param, RPC_DESC_BT_BMR_EVENT_PARAM, NULL));
    if (param->event == BT_BMR_EVENT_SRC_INFO)
    {
        BT_BMR_SRC_INFO_T *source_info = &param->data.source_info.info;
        if (source_info->subgroups != NULL)
        {
            BT_RH_LOG("memory of subgroups: %p\n", source_info->subgroups);
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, source_info, RPC_DESC_BT_BMR_SRC_INFO_T, NULL));
            RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, source_info->subgroups,
                sizeof(BT_BMR_SUBGROUP_T) * source_info->num_subgroups));

            for (UINT8 i = 0; i < source_info->num_subgroups; i++)
            {
                if((source_info->subgroups[i].metadata.vendor_metadata != NULL)
                    && (source_info->subgroups[i].metadata.vendor_metadata_length > 0))
                {
                    BT_RH_LOG("memory of metadata[%d]: %p\n", i, source_info->subgroups[i].metadata.vendor_metadata);
                    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(source_info->subgroups[i].metadata), RPC_DESC_BT_BMR_METADATA_T, NULL));
                    RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, source_info->subgroups[i].metadata.vendor_metadata,
                        source_info->subgroups[i].metadata.vendor_metadata_length));
                }
            }
        } else {
            BT_RH_LOG("Error subgroup is null\n");
        }
        RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
        RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
        RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_bmr_event_cbk", pt_nfy_tag->pv_cb_addr);
        if (source_info->subgroups != NULL) {
            for (size_t idx = 0; idx < source_info->num_subgroups; idx++) {
                if (source_info->subgroups[idx].metadata.vendor_metadata != NULL) {
                    BT_RH_LOG("free memory of subgroup metadata[%d]: %p\n", idx, source_info->subgroups[idx].metadata.vendor_metadata);
                    free(source_info->subgroups[idx].metadata.vendor_metadata);
                }
            }
            BT_RH_LOG("free memory of subgroups: %p\n", source_info->subgroups);
            free(source_info->subgroups);
        }
    }else {
        RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
        RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
        RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_bmr_event_cbk", pt_nfy_tag->pv_cb_addr);
    }

    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_disconnect(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_disconnect(pt_args[0].u.ps_str);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_discover(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_discover(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_solicitation_request(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_solicitation_request(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_refresh_source(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_refresh_source(pt_args[0].u.ui1_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_remove_source(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_remove_source(pt_args[0].u.b_arg, pt_args[1].u.ui1_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_set_broadcast_code(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_set_broadcast_code(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_streaming_start(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_streaming_start(pt_args[0].u.ui1_arg, pt_args[1].u.ui4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_streaming_stop(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_streaming_stop(pt_args[0].u.ui1_arg, pt_args[1].u.ui4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_set_pac_config(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_set_pac_config(pt_args[0].u.ui1_arg, pt_args[1].u.ui4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_close(
                         RPC_ID_T     t_rpc_id,
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

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_close(pt_args[0].u.ui1_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_dump(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bmr_dump(pt_args[0].u.b_arg, pt_args[1].u.ui1_arg, pt_args[2].u.b_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bmr_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    mtkrpcapi_BtAppBmrCbk p_bt_bmr_app_cb_func = NULL;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("\n");
    p_bt_bmr_app_cb_func = (mtkrpcapi_BtAppBmrCbk)pt_args[0].u.pv_func;
    if (NULL != p_bt_bmr_app_cb_func)
    {
        apv_cb_addr[0] = (VOID*)p_bt_bmr_app_cb_func;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
            pt_args[1].u.pv_arg);

        BT_RH_LOG("pv_func=%p, pv_arg=%p, pt_nfy_tag=%p",
            p_bt_bmr_app_cb_func, pt_args[1].u.pv_arg, pt_nfy_tag);

        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg =
            x_mtkapi_bt_bmr_register_callback(bt_app_bmr_event_cbk_wrapper,
            pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }
    else
    {
        pt_return->u.i4_arg = x_mtkapi_bt_bmr_register_callback(NULL, NULL);
    }
    return RPCR_OK;

}

INT32 c_rpc_reg_mtk_bt_service_bmr_op_hndlrs(VOID)
{
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_discover);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_solicitation_request);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_refresh_source);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_remove_source);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_set_broadcast_code);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_streaming_start);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_streaming_stop);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_set_pac_config);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_close);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_dump);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bmr_register_callback);
    return RPCR_OK;
}
