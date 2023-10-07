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


#ifndef _MTK_BT_SERVICE_GAP_H_
#define _MTK_BT_SERVICE_GAP_H_

#include "mtk_bt_service_gap_wrapper.h"

INT32 x_mtkapi_bt_gap_base_init(MTKRPCAPI_BT_APP_CB_FUNC* func, void *pv_tag);
INT32 x_mtkapi_bt_gap_base_uninit(MTKRPCAPI_BT_APP_CB_FUNC* func, void *pv_tag);
INT32 x_mtkapi_bt_gap_on_off(BOOL fg_on);
INT32 x_mtkapi_bt_gap_factory_reset();
INT32 x_mtkapi_bt_gap_set_name(CHAR *name);
INT32 x_mtkapi_bt_gap_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc);
INT32 x_mtkapi_bt_gap_get_dev_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr);
INT32 x_mtkapi_bt_gap_get_bond_state(CHAR* bd_addr);
INT32 x_mtkapi_bt_gap_get_local_dev_info(BT_LOCAL_DEV *ps_dev_info);
INT32 x_mtkapi_bt_gap_start_inquiry(UINT32 ui4_filter_type);
INT32 x_mtkapi_bt_gap_stop_inquiry();
INT32 x_mtkapi_bt_gap_pair(CHAR *bd_addr, int transport);
INT32 x_mtkapi_bt_gap_unpair(CHAR *bd_addr);
INT32 x_mtkapi_bt_gap_cancel_pair(CHAR *bd_addr);
INT32 x_mtkapi_bt_gap_interop_database_clear(VOID);
INT32 x_mtkapi_bt_gap_interop_database_add(CHAR *bd_addr, BTMW_GAP_INTEROP_FEATURE feature, UINT8 len);
INT32 x_mtkapi_bt_gap_get_rssi(CHAR *bd_addr, INT16 *rssi_value);
INT32 x_mtkapi_bt_gap_send_hci(CHAR *buffer);
INT32 x_mtkapi_bt_gap_get_bonded_dev();

#endif
