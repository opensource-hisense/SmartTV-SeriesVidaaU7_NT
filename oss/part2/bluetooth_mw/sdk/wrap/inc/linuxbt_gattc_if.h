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
 ----------------------------------------------------------------------------*/

#ifndef __LINUXBT_GATTC_IF_H__
#define __LINUXBT_GATTC_IF_H__

#include "u_bt_mw_gattc.h"
//need refactor
//nclude "bluetooth.h"
//nclude "bt_gatt_client.h"

//#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
//#include "mtk_bt_gatt_client.h"
//#endif

#ifdef __cplusplus
extern "C" {
#endif


int linuxbt_gattc_deinit(void);

int linuxbt_gattc_register(CHAR *client_name);

int linuxbt_gattc_unregister(int client_if);

int linuxbt_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param);

int linuxbt_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param);

int linuxbt_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param);

int linuxbt_gattc_discover_by_uuid(BT_GATTC_DISCOVER_BY_UUID_PARAM *discover_param);

int linuxbt_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param);

int linuxbt_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param);

int linuxbt_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param);

int linuxbt_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param);

int linuxbt_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param);

int linuxbt_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param);

int linuxbt_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param);

int linuxbt_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param);

int linuxbt_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param);

int linuxbt_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *chg_mtu_param);

int linuxbt_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_update_paramt);

int linuxbt_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *pref_phy_param);

int linuxbt_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param);

int linuxbt_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param);

#ifdef __cplusplus
}
#endif

#endif /* __LINUXBT_GATTC_IF_H__ */
