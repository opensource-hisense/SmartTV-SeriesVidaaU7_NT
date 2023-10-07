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

/* FILE NAME:  bt_mw_gatts.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include "u_bt_mw_common.h"
#include "linuxbt_gatts_if.h"
#include "bt_mw_gatt.h"
#include "bt_mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define BT_MW_GATTS_CASE_RETURN_STR(_const,str) case _const: return str
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static CHAR* bluetooth_gatts_service_get_event_str(BT_GATTS_EVENT event);
/* STATIC VARIABLE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */


/*GATT server*/


static BT_GATTS_EVENT_HANDLE_CB g_bt_mw_gatts_event_handle_cb = NULL;


INT32 bluetooth_gatts_register(CHAR *server_name,
    BT_GATTS_EVENT_HANDLE_CB gatts_handle)
{
    g_bt_mw_gatts_event_handle_cb = gatts_handle;
    return linuxbt_gatts_register(server_name);
}

INT32 bluetooth_gatts_unregister(INT32 server_if)
{
    return linuxbt_gatts_unregister(server_if);
}

INT32 bluetooth_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param)
{
    return linuxbt_gatts_connect(conn_param);
}

INT32 bluetooth_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param)
{
    return linuxbt_gatts_disconnect(disc_param);
}

INT32 bluetooth_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param)
{
    return linuxbt_gatts_add_service(add_param);
}

INT32 bluetooth_gatts_delete_service(BT_GATTS_SERVICE_DEL_PARAM *del_param)
{
    return linuxbt_gatts_delete_service(del_param);
}


INT32 bluetooth_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param)
{
    return linuxbt_gatts_send_indication(ind_param);
}

INT32 bluetooth_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param)
{
    return linuxbt_gatts_send_response(resp_param);
}

INT32 bluetooth_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param)
{
    return linuxbt_gatts_read_phy(phy_read_param);
}

INT32 bluetooth_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param)
{
    return linuxbt_gatts_set_prefer_phy(phy_param);
}


/*********GATTS callback**************/

VOID bt_mw_gatts_notify_app(BT_GATTS_EVENT_PARAM *gatts_msg)
{
    tBTMW_GATT_MSG msg;
    msg.hdr.event = BTMW_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_GATTS_EVENT_PARAM);
    memcpy((void*)&msg.data.gatts_param, gatts_msg, sizeof(BT_GATTS_EVENT_PARAM));
    bt_mw_gatt_nty_send_msg(&msg);
}

VOID bt_mw_gatts_nty_handle(BT_GATTS_EVENT_PARAM *gatts_msg)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "event:%s",
        bluetooth_gatts_service_get_event_str(gatts_msg->event));

    if(g_bt_mw_gatts_event_handle_cb)
    {
        g_bt_mw_gatts_event_handle_cb(gatts_msg);
    }
}

static CHAR* bluetooth_gatts_service_get_event_str(BT_GATTS_EVENT event)
{
    switch((int)event)
    {
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_REGISTER, "register");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_CONNECTION, "connection");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_SERVICE_ADD, "service_add");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_READ_REQ, "read_req");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_WRITE_REQ, "write_req");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_WRITE_EXE_REQ, "write_exe_req");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_IND_SENT, "ind_sent");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_MTU_CHANGE, "mtu_change");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_PHY_READ, "phy_read");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_PHY_UPDATE, "phy_update");
        BT_MW_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_CONN_UPDATE, "conn_update");
        default: return "UNKNOWN_EVENT";
   }
}


INT32 bt_mw_gatt_dump_info(VOID)
{
    return BT_SUCCESS;
}
