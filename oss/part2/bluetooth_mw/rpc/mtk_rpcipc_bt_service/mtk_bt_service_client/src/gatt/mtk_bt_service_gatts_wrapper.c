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
#include <pthread.h>

#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"

static pthread_mutex_t mtk_bt_service_gatts_wrapper_lock;

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("%s@%d: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

#define MTK_BT_SERVICE_GATTS_WRAPPER_LOCK() \
    do { \
        pthread_mutex_lock(&(mtk_bt_service_gatts_wrapper_lock)); \
    }while(0)


#define MTK_BT_SERVICE_GATTS_WRAPPER_UNLOCK() \
        do { \
            pthread_mutex_unlock(&(mtk_bt_service_gatts_wrapper_lock));\
        }while(0)

/* FUNCTION NAME: a_mtkapi_bt_gatts_register
 * PURPOSE:
 *      register a gatt server for app, then app can put gatt service into it.
 * INPUT:
 *      server_name  -- gatt server name, it should not be same in gatt servers.
 *                      the MAX length of name is BT_GATT_MAX_NAME_LEN.
 *      func      -- the gatt server callback, it's used to noify app.
 *      pv_tag    -- this will be put in callback function. it can be null.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- register success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_DONE -- the server has been registered
 *      BT_ERR_STATUS_PARM_INVALID -- server name too long
 *      BT_ERR_STATUS_NOMEM -- no more free server can be registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *      when register successfully, app will get a server interface for future use.
 *  the pv_tag can be null pointer if app don't want to use it.
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_register(CHAR *server_name,
    mtkrpcapi_BtAppGATTSCbk func, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_register");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, server_name);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_register");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_unregister
 * PURPOSE:
 *      app unregister the server when it don't want use it again.
 * INPUT:
 *      server_if  -- server interface of getting from register.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- unregister success.
 *      BT_ERR_STATUS_PARM_INVALID -- server_if is invalid or not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_unregister(INT32 server_if)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_unregister");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, server_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_unregister");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_connect
 * PURPOSE:
 *      connect or hold a connection if app need it.
 * INPUT:
 *      conn_param  -- connection parameter.
 *          .server_if      -- server interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .is_direct      -- 1: direct connect, if connect fail(timeout), it
 *                                will notify app.
 *                             0: background connect, if the remote device send
 *                                connectable advertisment, it will connect, it
 *                                will not timeout.
 *          .transport      -- connect over BR/EDR or BLE.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- connect request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_connect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, conn_param,
                    RPC_DESC_BT_GATTS_CONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, conn_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_disconnect
 * PURPOSE:
 *      connect or release a connection if app don't need it.
 * INPUT:
 *      disc_param  -- disconnect parameter
 *          .server_if      -- server interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- disconnect request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, disc_param,
                    RPC_DESC_BT_GATTS_DISCONNECT_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, disc_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_add_service
 * PURPOSE:
 *      add a gatt service to a gatt server. this gatt service should be a
 * complete service.
 * INPUT:
 *      service_param  -- service parameter
 *          .server_if      -- server interface
 *          .service_uuid   -- the service uuid string.
 *                              he MAX length of name is BT_GATT_MAX_UUID_LEN.
 *          .attrs          -- attribute array pointer, app should allocate
 *                              and free it, MW will only copy the data.
 *          .attr_cnt       -- attribute count in attrs.
 *          .is_last        -- app don't care it, it is used to fragment attributes
 *                              to RPC server.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- add service request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- attribute count 0 or
 *                                    server_if not registered or
 *                                    uuid is invalid string.
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param)
{
    INT32 cnt = 0, total_cnt = 0;
    BT_GATT_ATTR *attrs_buf = NULL;
    RPC_CLIENT_DECL(1, INT32);

    if (NULL == add_param) return BT_ERR_STATUS_NULL_POINTER;
    if (0 == add_param->attr_cnt) return BT_ERR_STATUS_PARM_INVALID;

    MTK_BT_SERVICE_GATTS_WRAPPER_LOCK();

    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, add_param,
                    RPC_DESC_BT_GATTS_SERVICE_ADD_PARAM, NULL));

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, add_param);

    /* backup it for restore */
    attrs_buf = add_param->attrs;
    total_cnt = add_param->attr_cnt;

    /* the RPC buffer is 4K, so it should limit the data size */
    do
    {

        if (cnt + BT_GATT_MAX_ATTR_CNT >= total_cnt)
        {
            add_param->attr_cnt = total_cnt - cnt;
            add_param->is_last = 1;
        }
        else
        {
            add_param->attr_cnt = BT_GATT_MAX_ATTR_CNT;
            add_param->is_last = 0;
        }

        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                        add_param->attrs,
                        sizeof(BT_GATT_ATTR)*add_param->attr_cnt));

        //BT_RW_LOG("attr_cnt=%d, sent_cnt=%d, service=%s",
        //    add_param->attr_cnt, cnt, add_param->attrs[0].uuid);

        RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_add_service");

        add_param->attrs += add_param->attr_cnt;
        cnt += add_param->attr_cnt;

        if(__rpc_ret != RPCR_OK || __t_ret.e_type != (ARG_TYPE_INT32))
        {
            break;
        }
    }while(cnt < total_cnt);

    /* restore it */
    add_param->attrs = attrs_buf;

    MTK_BT_SERVICE_GATTS_WRAPPER_UNLOCK();

    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


/* FUNCTION NAME: a_mtkapi_bt_gatts_del_service
 * PURPOSE:
 *      delete a service.
 * INPUT:
 *      del_param  -- delelete parameter.
 *          .server_if      -- server interface
 *          .service_handle -- the service handle which will be deleted.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- delete service request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_del_service(BT_GATTS_SERVICE_DEL_PARAM *del_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_del_service");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, del_param,
                    RPC_DESC_BT_GATTS_SERVICE_DEL_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, del_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_del_service");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_send_indication
 * PURPOSE:
 *      send a indication or notification to remote device.
 * INPUT:
 *      ind_param  -- indicatioin/notification parameter
 *          .server_if      -- server interface
 *          .addr           -- the connected device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .char_handle    -- the notification/indication characteristic handle
 *          .need_confirm   -- 0-notification, 1-indication
 *          .value          -- indication/notification data.
 *          .len            -- indication/notification data length.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- send indication/notification request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- device disconnected or other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_send_indication");
    RPC_CLIENT_DECL(1, INT32);

    if (NULL == ind_param) return BT_ERR_STATUS_NULL_POINTER;

    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, ind_param,
                    RPC_DESC_BT_GATTS_IND_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, ind_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_send_indication");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_send_response
 * PURPOSE:
 *      it is used to send response for read/write request.
 * INPUT:
 *      resp_param  -- response parameter
 *          .server_if  -- server interface
 *          .addr           -- the connected device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .status     -- read/write response, if read/write ok, it will 0.
 *                          if there is error, set it as error code in BT_GATT_STATUS.
 *          .handle     -- the attribute handle of write/read.
 *          .trans_id   -- the transaction id in read/write request event.
 *          .offset     -- the response data offset in attribute value.
 *          .value      -- the reponse data.
 *          .len        -- the response data length.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- send response request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_send_response");
    RPC_CLIENT_DECL(1, INT32);

    if (NULL == resp_param) return BT_ERR_STATUS_NULL_POINTER;

    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, resp_param,
                    RPC_DESC_BT_GATTS_RESPONSE_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, resp_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_send_response");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_read_phy
 * PURPOSE:
 *      read a specified connection phy parameter.
 * INPUT:
 *      phy_read_param  -- read phy parameter
 *          .server_if  -- server interface
 *          .addr       -- the connected device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read phy request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- server_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- device disconnected or other reason.
 * NOTES:
 *      after set prefer PHY, app will be notified in BT_GATTS_EVENT_PHY_READ.
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_read_phy");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, phy_read_param,
                    RPC_DESC_BT_GATTS_PHY_READ_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, phy_read_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_read_phy");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_bt_gatts_set_pref_phy
 * PURPOSE:
 *      set prefer phy to a specified connection.
 * INPUT:
 *      phy_param  -- set phy parameter.
 *          .server_if  -- server interface
 *          .addr       -- the connected device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .tx_phy     -- bitmask of GATT_PHY_SETTING
 *          .rx_phy     -- bitmask of GATT_PHY_SETTING
 *          .phy_option -- when there is LE Coded PHY setting in tx_phy, can
 *                          set the S2,S8.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- set prefer phy request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- server_if not registered or
 *                                    phy_option is invalid.
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- device disconnected or other reason.
 * NOTES:
 *      after set prefer PHY, app will be notified in BT_GATTS_EVENT_PHY_UPDATE.
 */
EXPORT_SYMBOL INT32 a_mtkapi_bt_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param)
{
    BT_RW_LOG("a_mtkapi_bt_gatts_set_pref_phy");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, phy_param,
                    RPC_DESC_BT_GATTS_PHY_SET_PARAM, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, phy_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gatts_set_prefer_phy");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

static INT32 _hndlr_bt_app_gatts_event_cbk(
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
    pt_return->e_type   = ARG_TYPE_VOID;
    //BT_RW_LOG("_hndlr_bt_app_gatts_event_cbk");

    ((mtkrpcapi_BtAppGATTSCbk)pv_cb_addr)
        ((BT_GATTS_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}


INT32 c_rpc_reg_mtk_bt_service_gatts_cb_hndlrs(void)
{
    int i4_ret = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mtk_bt_service_gatts_wrapper_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    RPC_REG_CB_HNDLR(bt_app_gatts_event_cbk);
    return RPCR_OK;
}

