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

#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_gattc_event_cbk(RPC_ID_T     t_rpc_id,
                                               const CHAR*  ps_cb_type,
                                               void          *pv_cb_addr,
                                               UINT32       ui4_num_args,
                                               ARG_DESC_T*  pt_args,
                                               ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("_hndlr_bt_app_ble_reg_scanner_cbk, pv_cb_addr = %p", pv_cb_addr);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCCbk)pv_cb_addr)((BT_GATTC_EVENT_PARAM *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_register(CHAR *client_name,
    mtkrpcapi_BtAppGATTCCbk func, void* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_register");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, client_name);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_register");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_unregister(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattsc_unregister");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_unregister");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_connect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, conn_param,
                    RPC_DESC_BT_GATTC_CONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, conn_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, disc_param,
                    RPC_DESC_BT_GATTC_DISCONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, disc_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_refresh");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, refresh_param,
                    RPC_DESC_BT_GATTC_REFRESH_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, refresh_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_refresh");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

#if 0
EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_discover_by_uuid(BT_GATTC_DISCOVER_BY_UUID_PARAM *discover_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_discover_by_uuid");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, discover_param,
                    RPC_DESC_BT_GATTC_DISCOVER_BY_UUID_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, discover_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_discover_by_uuid");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}
#endif

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_char");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, read_param,
                    RPC_DESC_BT_GATTC_READ_CHAR_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, read_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_char");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_char_by_uuid");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, read_param,
                    RPC_DESC_BT_GATTC_READ_BY_UUID_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, read_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_char_by_uuid");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_desc");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, read_param,
                    RPC_DESC_BT_GATTC_READ_DESC_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, read_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_desc");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_write_char");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, write_param,
                    RPC_DESC_BT_GATTC_WRITE_CHAR_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, write_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_write_char");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_write_desc");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, write_param,
                    RPC_DESC_BT_GATTC_WRITE_DESC_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, write_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_write_desc");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_exec_write");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, exec_write_param,
                    RPC_DESC_BT_GATTC_EXEC_WRITE_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, exec_write_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_exec_write");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_reg_notification");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, reg_notif_param,
                    RPC_DESC_BT_GATTC_REG_NOTIF_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, reg_notif_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_reg_notification");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_rssi");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, read_rssi_param,
                    RPC_DESC_BT_GATTC_READ_RSSI_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, read_rssi_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_rssi");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_dev_type");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, get_dev_type_param,
                    RPC_DESC_BT_GATTC_GET_DEV_TYPE_PARAM , NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, get_dev_type_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_dev_type");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *chg_mtu_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_change_mtu");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, chg_mtu_param,
                    RPC_DESC_BT_GATTC_CHG_MTU_PARAM , NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, chg_mtu_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_change_mtu");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_update_paramt)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_conn_update");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, conn_update_paramt,
                    RPC_DESC_BT_GATTC_CONN_UPDATE_PARAM , NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, conn_update_paramt);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_conn_update");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *phy_set_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_set_prefer_phy");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, phy_set_param,
                    RPC_DESC_BT_GATTC_PHY_SET_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, phy_set_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_prefer_phy");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_phy");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, read_phy_param,
                    RPC_DESC_BT_GATTC_PHY_READ_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, read_phy_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_phy");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_gatt_db");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, get_gatt_db_param,
                    RPC_DESC_BT_GATTC_GET_GATT_DB_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, get_gatt_db_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_gatt_db");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

INT32 c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_gattc_event_cbk);
    return RPCR_OK;
}

