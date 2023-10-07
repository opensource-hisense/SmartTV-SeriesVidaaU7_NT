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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_BT_SERVICE_GATTC_WRAPPER_H_
#define _MTK_BT_SERVICE_GATTC_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_gattc.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppGATTCCbk)(BT_GATTC_EVENT_PARAM *param, VOID *pv_tag);

/* FUNCTION NAME: a_mtkapi_bt_gattc_register
 * PURPOSE:
 *      this is used to register a GATT client. it will get a client interface
 * in BT_GATTC_EVENT_REGISTER event. It will be a input parameter in other
 * functions.
 *
 * INPUT:
 *      client_name  -- the GATT client name, max length is BT_GATT_MAX_NAME_LEN.
 *                      include '\0'. recommend: <app_name>.<client_name>
 *      func         -- the GATT client callback function.
 *      pv_tag       -- this will be the parameter of the callback. It can be
 *                      NULL pointer.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_DONE -- the client has been registered
 *      BT_ERR_STATUS_PARM_INVALID -- client name too long
 *      BT_ERR_STATUS_NOMEM -- no more free client can be registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_gattc_register(CHAR *client_name,
    mtkrpcapi_BtAppGATTCCbk func, void* pv_tag);


/* FUNCTION NAME: a_mtkapi_bt_gattc_unregister
 * PURPOSE:
 *      this is used to unregister a uselesss GATT client.
 * INPUT:
 *      client_if   -- the client interface of the useless GATT client.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- unregister success.
 *      BT_ERR_STATUS_PARM_INVALID -- client_if is invalid or not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_gattc_unregister(INT32 client_if);

/* FUNCTION NAME: a_mtkapi_bt_gattc_connect
 * PURPOSE:
 *      connect or hold a connection if app need it.
 * INPUT:
 *      conn_param  -- connection parameter.
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .is_direct      -- 1: direct connect, if connect fail(timeout), it
 *                                will notify app.
 *                             0: background connect, if the remote device send
 *                                connectable advertisment, it will connect, it
 *                                will not timeout.
 *          .transport      -- connect over BR/EDR or BLE.
 *          .opportunistic  -- just try to connect, if has connected already, it
 *                              will connect succesfufly, or it will fail.
 *          .initiating_phys-- bit0-1M, bit1-2M, bit2-Coded, reserver for future
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- connect request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_disconnect
 * PURPOSE:
 *      disconnect or release a connection if app don't need it.
 * INPUT:
 *      disc_param  -- disconnect parameter
 *          .client_if      -- server interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- disconnect request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_refresh
 * PURPOSE:
 *      refresh the GATT services of an specified device. After call this function,
 *  stack will discover all the services on the remote device if it connected.
 *  if the device is not connected, it will clear the service database.
 * INPUT:
 *      refresh_param  -- refresh parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- disconnect request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  after service discover done, the new database will report by
 *  BT_GATTC_EVENT_GET_GATT_DB event.
 */
extern INT32 a_mtkapi_bt_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_read_char
 * PURPOSE:
 *      read the specified characteristic value. the device should be connected.
 *  APP don't care if the value is a long value.
 * INPUT:
 *      read_param  -- read parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .handle         --  characteristic handle
 *          .auth_req       -- the read authentication request type
 *                              BT_GATTC_AUTH_REQ_NONE: no encryption
 *                              BT_GATTC_AUTH_REQ_NO_MITM: un-authenticated encryption
 *                              BT_GATTC_AUTH_REQ_MITM: authenticated encryption
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  after read, the read value will return by BT_GATTC_EVENT_READ_CHAR_RSP.
 *  if the authentication fail, APP should use high level authentication setting.
 *  if auth_req level is more than current authentication level, stack will
 *  prompt the level and request a SMP.
 */
extern INT32 a_mtkapi_bt_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_read_char_by_uuid
 * PURPOSE:
 *      read the specified characteristic value from a specified handle range.
 *  the device should be connected. APP don't care if the value is a long value.
 * INPUT:
 *      read_param  -- read parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .char_uuid      -- characteristic uuid string
 *          .s_handle       -- start handle
 *          .e_handle       -- end handle
 *          .auth_req       -- the read authentication request type
 *                              BT_GATTC_AUTH_REQ_NONE: no encryption
 *                              BT_GATTC_AUTH_REQ_NO_MITM: un-authenticated encryption
 *                              BT_GATTC_AUTH_REQ_MITM: authenticated encryption
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    characteristic if not registered or
 *                                    service_uuid string is invalid or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the read value will return by BT_GATTC_EVENT_READ_CHAR_RSP.
 *  if the authentication fail, APP should use high level authentication setting.
 *  if auth_req level is more than current authentication level, stack will
 *  prompt the level and request a SMP.
 */
extern INT32 a_mtkapi_bt_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_read_desc
 * PURPOSE:
 *      read the specified description value. the device should be connected.
 *  APP don't care if the value is a long value.
 * INPUT:
 *      read_param  -- read parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .handle         --  description handle
 *          .auth_req       -- the read authentication request type
 *                              BT_GATTC_AUTH_REQ_NONE: no encryption
 *                              BT_GATTC_AUTH_REQ_NO_MITM: un-authenticated encryption
 *                              BT_GATTC_AUTH_REQ_MITM: authenticated encryption
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- write request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the read value will return by BT_GATTC_EVENT_READ_DESC_RSP.
 *  if the authentication fail, APP should use high level authentication setting.
 *  if auth_req level is more than current authentication level, stack will
 *  prompt the level and request a SMP.
 */
extern INT32 a_mtkapi_bt_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_write_char
 * PURPOSE:
 *      write the specified characteristic value. the device should be connected.
 *  APP can write by command, request or prepare write.
 *      APP don't care if the value is a long value, if write type is write request,
 *  stack will fragment it by prepare write.
 *      if APP want to reliable write, it should set the write type as prepare write.
 * INPUT:
 *      write_param  -- write parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .handle         --  characteristic handle
 *          .write_type     -- write type
 *                              BT_GATTC_WRITE_TYPE_CMD: write command
 *                              BT_GATTC_WRITE_TYPE_REQ: write request
 *                              BT_GATTC_WRITE_TYPE_PREPARE: prepare wirte
 *          .auth_req       -- the read authentication request type
 *                              BT_GATTC_AUTH_REQ_NONE: no encryption
 *                              BT_GATTC_AUTH_REQ_NO_MITM: un-authenticated encryption
 *                              BT_GATTC_AUTH_REQ_MITM: authenticated encryption
 *          .value_len      -- write value length.
 *          .value[]        -- the write value.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- write request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the write result will return by BT_GATTC_EVENT_WRITE_CHAR_RSP.
 *  if the authentication fail, APP should use high level authentication setting.
 *  if auth_req level is more than current authentication level, stack will
 *  prompt the level and request a SMP.
 */
extern INT32 a_mtkapi_bt_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_write_desc
 * PURPOSE:
 *      write the specified description value. the device should be connected.
 *  APP can only write description by write request.
 *  APP don't care if the value is a long value, if write type is write request,
 *  stack will fragment it by prepare write.
 * INPUT:
 *      write_param  -- write parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .handle         --  characteristic handle
 *          .auth_req       -- the read authentication request type
 *                              BT_GATTC_AUTH_REQ_NONE: no encryption
 *                              BT_GATTC_AUTH_REQ_NO_MITM: un-authenticated encryption
 *                              BT_GATTC_AUTH_REQ_MITM: authenticated encryption
 *          .value_len      -- write value length.
 *          .value[]        -- the write value.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- write request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the write result will return by BT_GATTC_EVENT_WRITE_DESC_RSP.
 *  if the authentication fail, APP should use high level authentication setting.
 *  if auth_req level is more than current authentication level, stack will
 *  prompt the level and request a SMP.
 */
extern INT32 a_mtkapi_bt_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_exec_write
 * PURPOSE:
 *      execute or cancel the reliable write(prepare write).
 * INPUT:
 *      exec_write_param  -- write parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .execute        --  0: cancel, 1: execute
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  after execute/cancel write, the write result will return by
 *  BT_GATTC_EVENT_EXEC_WRITE_RSP.
 */
extern INT32 a_mtkapi_bt_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_reg_notification
 * PURPOSE:
 *      if APP want to receive the indication or notification of a specified
 *  characteritic, it can use this function to register for it.
 *      if APP don't want to receive it, it can deregister for it.
 * INPUT:
 *      reg_notif_param  -- register parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .handle         --  characteristic handle
 *          .registered     -- 0: deregister, 1: register
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- register request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the notification/indication will return by BT_GATTC_EVENT_NOTIFY.
 *  if the remote device happen service changed, APP should register it again.
 */
extern INT32 a_mtkapi_bt_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_read_rssi
 * PURPOSE:
 *      it is used to read a specified connection RSSI.
 * INPUT:
 *      read_rssi_param  -- read parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the RSSI will return by BT_GATTC_EVENT_READ_RSSI_RSP.
 */
extern INT32 a_mtkapi_bt_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_get_dev_type
 * PURPOSE:
 *      it is used to get the device type.
 * INPUT:
 *      get_dev_type_param  -- read parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      0                    -- read fail
 *      BT_GATTC_DEVICE_TYPE -- device type
 * NOTES:
 *
 */
extern INT32 a_mtkapi_bt_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_change_mtu
 * PURPOSE:
 *      it is used to change MTU of specified connection.
 * INPUT:
 *      config_mtu_param  -- configure MTU parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .mtu            -- MTU, BT_GATT_MIN_MTU_SIZE ~ BT_GATT_MAX_MTU_SIZE
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- change MTU request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected or
 *                                    mut is out of range.
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the MTU result will return by BT_GATTC_EVENT_MTU_CHANGE.
 */
extern INT32 a_mtkapi_bt_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *config_mtu_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_conn_update
 * PURPOSE:
 *      it is used to change connection parameter of specified connection.
 * INPUT:
 *      conn_param_update_param  -- connection parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .min_interval   --  unit: 1.25ms, 7.5ms ~ 4000ms, recommend: 11.25ms
 *          .max_interval   -- unit: 1.25ms, 7.5ms ~ 4000ms, recommend: 11.25ms
 *          .latency        -- unit: connection event, 0~499, recommend: 0
 *          .timeout        -- unit: 10ms, 100~32000ms, recommend: 5000ms
 *          .min_ce_len     -- unit: 0.625ms, 0~0xFFFF, recommend: 0
 *          .max_ce_len     -- unit: 0.625ms, 0~0xFFFF, recommend: 0
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- connection update request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected or
 *                                    connection parameter is out of range
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the connection update result will return by BT_GATTC_EVENT_CONN_UPDATE.
 */
extern INT32 a_mtkapi_bt_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_param_update_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_set_prefer_phy
 * PURPOSE:
 *      it is used to change set prefered PHY of specified connection.
 * INPUT:
 *      prefer_phy_param  -- PHY setting parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 *          .tx_phy         -- bit0-1M, bit1-2M, bit2-Coded
 *          .rx_phy         -- bit0-1M, bit1-2M, bit2-Coded
 *          .phy_options    -- when there is LE coded in tx_phy, it's invalid
 *                              GATT_PHY_OPT_CODED_NO_PREF: no prefered
 *                              GATT_PHY_OPT_CODED_S2: prefer S2
 *                              GATT_PHY_OPT_CODED_S8: prefer S8
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- set PHY request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected or
 *                                    connection parameter is out of range
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the PHY setting result will return by BT_GATTC_EVENT_PHY_UPDATE.
 */
extern INT32 a_mtkapi_bt_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *prefer_phy_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_read_phy
 * PURPOSE:
 *      it is used to change read PHY setting of specified connection.
 * INPUT:
 *      read_phy_param  -- read PHY parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- read PHY request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected or
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  the PHY setting result will return by BT_GATTC_EVENT_PHY_READ.
 */
extern INT32 a_mtkapi_bt_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param);

/* FUNCTION NAME: a_mtkapi_bt_gattc_get_gatt_db
 * PURPOSE:
 *      it is used to get all gatt services of specified device.
 * INPUT:
 *      get_gatt_db_param  -- get database parameter
 *          .client_if      -- client interface
 *          .addr           -- the link device address string, like:
 *                              XX:XX:XX:XX:XX:XX
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- get service db request success.
 *      BT_ERR_STATUS_NULL_POINTER -- some parameter is NULL pointer
 *      BT_ERR_STATUS_PARM_INVALID -- address string is invalid or
 *                                    client_if not registered or
 *                                    device is not connected or
 *      BT_ERR_STATUS_NOT_READY -- GATT not initialized
 *      BT_ERR_STATUS_FAIL -- other reason.
 * NOTES:
 *  all the services will return by BT_GATTC_EVENT_GET_GATT_DB.
 */
extern INT32 a_mtkapi_bt_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param);


extern INT32 c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
