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

#ifndef _MTK_BT_SERVICE_SPP_WRAPPER_H_
#define _MTK_BT_SERVICE_SPP_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_spp.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppSppCbk)(BT_SPP_CBK_STRUCT *pt_spp_struct, VOID *pv_tag);

/**
 * FUNCTION NAME: a_mtkapi_spp_register_callback
 * PURPOSE:
 *      The function is used to register spp callback.
 * INPUT:
 *      spp_cb -- spp callback function
 *      pv_tag -- app can input NULL.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_NULL_POINTER   -- Input parameter is null pointer.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_register_callback(mtkrpcapi_BtAppSppCbk spp_cb, VOID* pv_tag);

/**
 * FUNCTION NAME: a_mtkapi_spp_unregister_callback
 * PURPOSE:
 *      The function is used to unregister spp callback.
 * INPUT:
 *      spp_cb -- spp callback function
 *      pv_tag -- app can input NULL.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_NULL_POINTER   -- Input parameter is null pointer.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_unregister_callback(mtkrpcapi_BtAppSppCbk spp_cb, VOID* pv_tag);

/**
 * FUNCTION NAME: a_mtkapi_spp_connect
 * PURPOSE:
 *      The function is used to SPP connect with uuid.
 * INPUT:
 *      BT_SPP_CONNECT_PARAM* -- connect spp client params.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_connect(BT_SPP_CONNECT_PARAM *spp_connect_param);

/**
 * FUNCTION NAME: a_mtkapi_spp_disconnect
 * PURPOSE:
 *      The function is used to SPP disconnect.
 * INPUT:
 *      BT_SPP_DISCONNECT_PARAM* -- disconnect spp client params.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_disconnect(BT_SPP_DISCONNECT_PARAM *spp_disconnect_param);

/**
 * FUNCTION NAME: a_mtkapi_spp_send_data
 * PURPOSE:
 *      The function is used to send data.
 * INPUT:
 *      BT_SPP_SEND_DATA_PARAM* -- send data params.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_send_data(BT_SPP_SEND_DATA_PARAM *spp_send_data_param);

/**
 * FUNCTION NAME: a_mtkapi_spp_start_server
 * PURPOSE:
 *      The function is used to enable deviceB, the platform is deviceB.
 * INPUT:
 *      BT_SPP_START_SVR_PARAM* -- enable spp server params.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
extern INT32 a_mtkapi_spp_start_server(BT_SPP_START_SVR_PARAM *spp_start_svr_param);

/**
 * FUNCTION NAME: a_mtkapi_spp_stop_server
 * PURPOSE:
 *      The function is used to disable deviceB, the platform is deviceB.
 * INPUT:
 *      BT_SPP_STOP_SVR_PARAM* -- disable spp server params.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      When have connection using this service, after disable devb, the connection also abort.
 */
extern INT32 a_mtkapi_spp_stop_server(BT_SPP_STOP_SVR_PARAM *spp_stop_svr_param);

extern INT32 a_mtkapi_spp_get_connection_info(BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db);

/**
 * FUNCTION NAME: c_rpc_reg_mtk_bt_service_spp_cb_hndlrs
 * PURPOSE:
 *      The function is used to register spp handles.
 * INPUT:
 *      None.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      When have connection using this service, after disable devb, the connection also abort.
 */
extern INT32 c_rpc_reg_mtk_bt_service_spp_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
