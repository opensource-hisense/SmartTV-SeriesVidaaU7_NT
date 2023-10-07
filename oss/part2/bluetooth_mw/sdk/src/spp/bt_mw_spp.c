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


#include "c_mw_config.h"
#include "bt_mw_spp.h"
#include "bt_mw_common.h"
#include "linuxbt_spp_if.h"
#include "bt_mw_message_queue.h"
#include "bt_mw_gap.h"

BtAppSppCbk SppCbk = NULL;
BOOL B_Spp_Init = FALSE;

static INT32 bt_mw_spp_init(void);
static VOID bt_mw_spp_init_profile(void);
static VOID bt_mw_spp_deinit_profile(void);
static VOID bt_mw_spp_nty_handle(tBTMW_MSG *p_msg);
VOID bt_mw_spp_nty_state_handle(tBTMW_MSG *p_msg);

/**
 * FUNCTION NAME: bt_mw_spp_register_callback
 * PURPOSE:
 *      The function is used to register spp callback.
 * INPUT:
 *      spp_cb -- spp callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_NULL_POINTER   -- Input parameter is null pointer.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_register_callback(BtAppSppCbk spp_cb)
{
    FUNC_ENTRY;

    if (spp_cb)
    {
        SppCbk = spp_cb;
        bt_mw_spp_init();
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "spp callback func is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    return BT_SUCCESS;
}

static INT32 bt_mw_spp_init(void)
{
    if (TRUE == B_Spp_Init)
    {
        return BT_SUCCESS;
    }

    profile_operator_t spp_op;
    memset(&spp_op, 0, sizeof(spp_op));
    spp_op.init = bt_mw_spp_init_profile;
    spp_op.deinit = bt_mw_spp_deinit_profile;
    spp_op.notify_acl_state = NULL;
    spp_op.facotry_reset = NULL;

    B_Spp_Init = TRUE;

    bt_mw_gap_register_profile(BTWM_ID_SPP, &spp_op);

    return BT_SUCCESS;
}

static VOID bt_mw_spp_init_profile(void)
{
    FUNC_ENTRY;
    bt_mw_nty_hdl_register(BTWM_ID_SPP, bt_mw_spp_nty_handle);
    linuxbt_spp_init();
    return;
}

static VOID bt_mw_spp_deinit_profile(void)
{
    FUNC_ENTRY;
    B_Spp_Init = FALSE;
    bt_mw_gap_register_profile(BTWM_ID_SPP, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_SPP, NULL);
    linuxbt_spp_deinit();
    return;
}

static VOID bt_mw_spp_nty_handle(tBTMW_MSG *p_msg)
{
    BT_DBG_INFO(BT_DEBUG_SPP, "bluetooth spp notify msg handle, event:%d", p_msg->hdr.event);

    if (p_msg->hdr.event == BTMW_SPP_STATE_EVT)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "bluetooth spp msg handle, state event");
        bt_mw_spp_nty_state_handle(p_msg);
    }
    else
    {
       BT_DBG_ERROR(BT_DEBUG_SPP, "bluetooth spp msg handle, no define event");
    }
}

/**
 * FUNCTION NAME: bt_mw_spp_connect
 * PURPOSE:
 *      The function is used to SPP connect.
 * INPUT:
 *      addr -- the device address to connect.
 *      uuid -- the uuid to connect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_connect(BT_SPP_CONNECT_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_spp_connect(param);
}

/**
 * FUNCTION NAME: bt_mw_spp_disconnect
 * PURPOSE:
 *      The function is used to SPP disconnect.
 * INPUT:
 *      addr -- the device address to disconnect.
 *      uuid -- the uuid to disconnect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_disconnect(BT_SPP_DISCONNECT_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_spp_disconnect(param);
}

/**
 * FUNCTION NAME: bt_mw_spp_send_data
 * PURPOSE:
 *      The function is used to send data.
 * INPUT:
 *      bd_addr -- the device address to send.
 *      uuid -- the uuid to send.
 *      str -- the string to send.
 *      len -- the string length to send.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_send_data(BT_SPP_SEND_DATA_PARAM *param)
{
    FUNC_ENTRY;
    return linuxbt_spp_send_data(param);
}

/**
 * FUNCTION NAME: bt_spp_receive_data
 * PURPOSE:
 *      The function is used to receive data.
 * INPUT:
 *      bd_addr -- the data to receive from which device.
 *      uuid -- the data to receive from which uuid.
 *      uuid_len -- the uuid length.
 *      buffer -- the data to receive.
 *      length -- the data length to receive.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_spp_receive_data(CHAR *bd_addr, CHAR *uuid, INT32 uuid_len, UINT8 spp_if, CHAR *buffer, UINT16 length)
{
    FUNC_ENTRY;
    tBTMW_MSG msg = {0};

    memset(&msg, 0, sizeof(msg));

    msg.hdr.event = BTMW_SPP_STATE_EVT;
    msg.hdr.len = sizeof(BT_SPP_CBK_STRUCT);

    if (NULL == buffer)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "null pointer of buffer");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    strncpy(msg.data.spp_msg.bd_addr, bd_addr, MAX_BDADDR_LEN - 1);
    msg.data.spp_msg.bd_addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(msg.data.spp_msg.uuid, uuid, MAX_UUID_LEN - 1);
    msg.data.spp_msg.uuid[MAX_UUID_LEN - 1] = '\0';
    msg.data.spp_msg.uuid_len = uuid_len;
    msg.data.spp_msg.spp_if = spp_if;
    memcpy(msg.data.spp_msg.spp_data, buffer, length);
    msg.data.spp_msg.spp_data_len = length;
    msg.data.spp_msg.spp_data[length] = '\0';
    msg.data.spp_msg.event= BT_SPP_RECV_DATA;

    for (int i=0; i < length; i++)
    {
        BT_DBG_INFO(BT_DEBUG_SPP, "spp_data[%d] = %02x", i, msg.data.spp_msg.spp_data[i]);
    }

    BT_DBG_NOTICE(BT_DEBUG_SPP, "bd_addr is: %s", msg.data.spp_msg.bd_addr);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "uuid is: %s", msg.data.spp_msg.uuid);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "uuid_len is: %d", msg.data.spp_msg.uuid_len);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "spp_if is: %d", msg.data.spp_msg.spp_if);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "Received Data:%s", msg.data.spp_msg.spp_data);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "include spp_data_len is: %d", msg.data.spp_msg.spp_data_len);

    bt_mw_nty_send_msg(&msg);

    return BT_SUCCESS;
}

/**
 * FUNCTION NAME: bt_spp_event_cb
 * PURPOSE:
 *      The function is used to confirm spp event, eg. connect/disocnnect.
 * INPUT:
 *      bd_addr -- the event from which device.
 *      uuid -- the event from which uuid.
 *      uuid_len -- the uuid length.
 *      event -- the event type.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_spp_event_cb(CHAR *bd_addr, CHAR *uuid, INT32 uuid_len, UINT8 spp_if, INT32 event)
{
    FUNC_ENTRY;
    tBTMW_MSG msg = {0};
    memset(&msg, 0, sizeof(msg));

    msg.hdr.event = BTMW_SPP_STATE_EVT;
    msg.hdr.len = sizeof(BT_SPP_CBK_STRUCT);

    strncpy(msg.data.spp_msg.bd_addr, bd_addr, MAX_BDADDR_LEN - 1);
    msg.data.spp_msg.bd_addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(msg.data.spp_msg.uuid, uuid, MAX_UUID_LEN -1);
    msg.data.spp_msg.uuid[MAX_UUID_LEN - 1] = '\0';
    msg.data.spp_msg.uuid_len = uuid_len;
    msg.data.spp_msg.spp_if = spp_if;
    msg.data.spp_msg.event = event;
    BT_DBG_NOTICE(BT_DEBUG_SPP, "bd_addr is: %s", msg.data.spp_msg.bd_addr);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "uuid is: %s", msg.data.spp_msg.uuid);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "uuid_len is: %d", msg.data.spp_msg.uuid_len);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "spp_if is: %d", msg.data.spp_msg.spp_if);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "event is: %d", msg.data.spp_msg.event);

    bt_mw_nty_send_msg(&msg);

    return BT_SUCCESS;
}

VOID bt_mw_spp_nty_state_handle(tBTMW_MSG *p_msg)
{
    FUNC_ENTRY;

    BT_SPP_CBK_STRUCT t_spp_struct_data;
    memcpy(&t_spp_struct_data, &p_msg->data.spp_msg, sizeof(BT_SPP_CBK_STRUCT));

    /*  call the app callback function*/
    if(SppCbk)
    {
        SppCbk(&t_spp_struct_data);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "spp callback is NULL.");
    }
}

/**
 * FUNCTION NAME: bt_mw_spp_start_server
 * PURPOSE:
 *      The function is used to enable deviceB, the platform is deviceB.
 * INPUT:
 *      servername -- the service name.
 *      uuid -- the uuid to connect/send/disconnect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_start_server(BT_SPP_START_SVR_PARAM *param)
{
    FUNC_ENTRY;

    if (0 == strlen(param->server_name))
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "null pointer of paddr");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (0 == strlen(param->uuid))
    {
        CHAR *uuid_128 = SPP_DEFAULT_UUID;
        strncpy(param->uuid, uuid_128, strlen(uuid_128));
        param->uuid[strlen(uuid_128)] = '\0';
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Using default UUID:%s", param->uuid);
    }

    return linuxbt_spp_activate(param);
}

/**
 * FUNCTION NAME: bt_mw_spp_stop_server
 * PURPOSE:
 *      The function is used to disable deviceB, the platform is deviceB.
 * INPUT:
 *      uuid -- the uuid to disable.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 bt_mw_spp_stop_server(BT_SPP_STOP_SVR_PARAM *param)
{
    FUNC_ENTRY;

    if (0 == strlen(param->uuid))
    {
        CHAR *uuid_128 = SPP_DEFAULT_UUID;
        strncpy(param->uuid, uuid_128, strlen(uuid_128));
        param->uuid[strlen(uuid_128)] = '\0';
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Using default UUID:%s", param->uuid);
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Using Input UUID:%s", param->uuid);
    }

    return linuxbt_spp_deactivate(param);
}

INT32 bt_mw_spp_get_connection_info(BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db)
{
    FUNC_ENTRY;
    return linuxbt_spp_get_connection_info(spp_connection_info_db);
}


