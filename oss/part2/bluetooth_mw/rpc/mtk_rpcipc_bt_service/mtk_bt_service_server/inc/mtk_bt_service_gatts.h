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


#ifndef _MTK_BT_SERVICE_GATTS_H_
#define _MTK_BT_SERVICE_GATTS_H_
#include "ri_common.h"
#include "mtk_bt_service_gatts_wrapper.h"

extern VOID x_mtkapi_bt_gatts_init(VOID);

VOID x_mtkapi_bt_gatts_ipc_close_notify(RPC_CB_NFY_TAG_T* pv_nfy_tag);
INT32 x_mtkapi_bt_gatts_register(CHAR *server_name,
    mtkrpcapi_BtAppGATTSCbk func, RPC_CB_NFY_TAG_T* pv_nfy_tag);
INT32 x_mtkapi_bt_gatts_unregister(INT32 server_if);
INT32 x_mtkapi_bt_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param);
INT32 x_mtkapi_bt_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param);
INT32 x_mtkapi_bt_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param);
INT32 x_mtkapi_bt_gatts_del_service(BT_GATTS_SERVICE_DEL_PARAM *del_param);
INT32 x_mtkapi_bt_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param);
INT32 x_mtkapi_bt_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param);
INT32 x_mtkapi_bt_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param);
INT32 x_mtkapi_bt_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_param);

#endif
