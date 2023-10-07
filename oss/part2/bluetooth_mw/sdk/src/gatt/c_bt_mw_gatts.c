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


/* FILE NAME:  c_bt_mw_gatts.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS API to APP.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "c_bt_mw_gatts.h"
#include "bt_mw_gatts.h"



EXPORT_SYMBOL INT32 c_btm_gatts_register(CHAR *server_name,
    BT_GATTS_EVENT_HANDLE_CB gatts_handle)
{
    return bluetooth_gatts_register(server_name, gatts_handle);
}

EXPORT_SYMBOL INT32 c_btm_gatts_unregister(INT32 server_if)
{
    return bluetooth_gatts_unregister(server_if);
}

EXPORT_SYMBOL INT32 c_btm_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param)
{
    return bluetooth_gatts_connect(conn_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param)
{
    return bluetooth_gatts_disconnect(disc_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param)
{
    return bluetooth_gatts_add_service(add_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_delete_service(BT_GATTS_SERVICE_DEL_PARAM *del_param)
{
    return bluetooth_gatts_delete_service(del_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param)
{
    return bluetooth_gatts_send_indication(ind_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param)
{
    return bluetooth_gatts_send_response(resp_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param)
{
    return bluetooth_gatts_read_phy(phy_read_param);
}

EXPORT_SYMBOL INT32 c_btm_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param)
{
    return bluetooth_gatts_set_prefer_phy(phy_param);
}

