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
#ifndef _MTK_BT_SERVICE_GATTS_WRAPPER_H_
#define _MTK_BT_SERVICE_GATTS_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_gatts.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppGATTSCbk)(BT_GATTS_EVENT_PARAM *param, VOID *pv_tag);


/* FUNCTION NAME: a_mtkapi_bt_gatts_register
 * PURPOSE:
 *      register a gatt server for app, then app can put gatt service into it.
 * INPUT:
 *      server_name  -- gatt server name, it should not be same in gatt servers.
 *                      the MAX length of name is BT_GATT_MAX_NAME_LEN.
 *                       recommend: <app_name>.<server_name>
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
extern INT32 a_mtkapi_bt_gatts_register(CHAR *server_name,
    mtkrpcapi_BtAppGATTSCbk func, VOID* pv_tag);



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
extern INT32 a_mtkapi_bt_gatts_unregister(INT32 server_if);

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
extern INT32 a_mtkapi_bt_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param);

/* FUNCTION NAME: a_mtkapi_bt_gatts_disconnect
 * PURPOSE:
 *      disconnect or release a connection if app don't need it.
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
extern INT32 a_mtkapi_bt_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param);

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
extern INT32 a_mtkapi_bt_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param);

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
extern INT32 a_mtkapi_bt_gatts_del_service(BT_GATTS_SERVICE_DEL_PARAM *del_param);

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
extern INT32 a_mtkapi_bt_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param);

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
extern INT32 a_mtkapi_bt_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp);

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
extern INT32 a_mtkapi_bt_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param);

/* FUNCTION NAME: a_mtkapi_bt_gatts_set_pref_phy
 * PURPOSE:
 *      set prefer phy to a specified connection.
 * INPUT:
 *      phy_param  -- set phy parameter.
 *          .server_if  -- server interface
 *          .addr       -- the connected device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .tx_phy     -- bit0-1M, bit1-2M, bit2-Coded
 *          .rx_phy     -- bit0-1M, bit1-2M, bit2-Coded
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
extern INT32 a_mtkapi_bt_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param);


extern INT32 c_rpc_reg_mtk_bt_service_gatts_cb_hndlrs(void);

#ifdef  __cplusplus
}
#endif
#endif
