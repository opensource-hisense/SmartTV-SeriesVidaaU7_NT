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


/* FILE NAME:  c_bt_mw_gattc.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTC API to APP.
 * NOTES:
 */

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "c_bt_mw_gattc.h"
#include "bt_mw_gattc.h"
#include "bt_mw_log.h"


/******************** gattc ********************/

EXPORT_SYMBOL INT32 c_btm_gattc_register(CHAR *client_name, BT_GATTC_EVENT_HANDLE_CB gattc_handle)
{
    return bluetooth_gattc_register(client_name, gattc_handle);
}

EXPORT_SYMBOL INT32 c_btm_gattc_unregister(INT32 client_if)
{
    return bluetooth_gattc_unregister(client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param)
{
    return bluetooth_gattc_connect(conn_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param)
{
    return bluetooth_gattc_disconnect(disc_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param)
{
    return bluetooth_gattc_refresh(refresh_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_discover_by_uuid(BT_GATTC_DISCOVER_BY_UUID_PARAM *discover_param)
{
    return bluetooth_gattc_discover_by_uuid(discover_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param)
{
    return bluetooth_gattc_read_char(read_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param)
{
    return bluetooth_gattc_read_char_by_uuid(read_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param)
{
    return bluetooth_gattc_read_desc(read_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param)
{
    return bluetooth_gattc_write_char(write_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param)
{
    return bluetooth_gattc_write_desc(write_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param)
{
    return bluetooth_gattc_exec_write(exec_write_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param)
{
    return bluetooth_gattc_reg_notification(reg_notif_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param)
{
    return bluetooth_gattc_read_rssi(read_rssi_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param)
{
    return bluetooth_gattc_get_dev_type(get_dev_type_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *chg_mtu_param)
{
    return bluetooth_gattc_change_mtu(chg_mtu_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_update_param)
{
    return bluetooth_gattc_conn_update(conn_update_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *phy_set_param)
{
    return bluetooth_gattc_set_prefer_phy(phy_set_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param)
{
    return bluetooth_gattc_read_phy(read_phy_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param)
{
    return bluetooth_gattc_get_gatt_db(get_gatt_db_param);
}

