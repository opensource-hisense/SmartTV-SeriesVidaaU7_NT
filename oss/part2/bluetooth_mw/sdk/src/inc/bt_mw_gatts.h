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


/* FILE NAME:  bt_mw_gatts.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS API to c_bt_mw_gatts and other mw layer modules.
 * NOTES:
 */


#ifndef __BT_MW_GATTS_H__
#define __BT_MW_GATTS_H__
#include <stdint.h>

#include "u_bt_mw_gatts.h"
__BEGIN_DECLS


/**
 * FUNCTION NAME: bt_mw_gatts_nty_handle
 * PURPOSE:
 *      The function is used to handle notify  APP callback function.
 * INPUT:
 *      gatts_msg           -- gatt server message
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bt_mw_gatts_nty_handle(BT_GATTS_EVENT_PARAM *gatts_msg);

/*GATT server*/


INT32 bluetooth_gatts_register(CHAR *server_name,
    BT_GATTS_EVENT_HANDLE_CB gatts_handle);

INT32 bluetooth_gatts_unregister(INT32 server_if);

INT32 bluetooth_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param);

INT32 bluetooth_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param);

INT32 bluetooth_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param);

INT32 bluetooth_gatts_delete_service(BT_GATTS_SERVICE_DEL_PARAM *del_param);

INT32 bluetooth_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param);

INT32 bluetooth_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param);

INT32 bluetooth_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param);

INT32 bluetooth_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param);

INT32 bt_mw_gatt_dump_info(VOID);

VOID bt_mw_gatts_notify_app(BT_GATTS_EVENT_PARAM *gatts_msg);


__END_DECLS

#endif/* __BT_MW_GATTS_H__ */
