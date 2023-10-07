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

#ifndef _BT_MW_GAP_H_
#define _BT_MW_GAP_H_

#include "u_bt_mw_gap.h"
#include "bt_mw_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BT_MW_GAP_CASE_RETURN_STR(_const,str) case _const: return (CHAR*)str

typedef struct  {
  void (*init)(void);
  void (*deinit)(void);
  void (*notify_acl_state)(BTMW_GAP_EVT *gap_evt);
  void (*facotry_reset)(void);
  void (*rm_dev)(CHAR *addr);
} profile_operator_t;

/*----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
INT32 bt_mw_gap_base_init(BT_APP_CB_FUNC *func);
INT32 bt_mw_gap_on_off(BOOL fg_on);
INT32 bt_mw_gap_factory_reset(VOID);
INT32 bt_mw_gap_set_local_name(CHAR *name);
INT32 bt_mw_gap_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc);
INT32 bt_mw_gap_get_device_info(BLUETOOTH_DEVICE *dev_info, CHAR *bd_addr);
INT32 bt_mw_gap_get_bond_state(CHAR *bd_addr);
INT32 bt_mw_gap_get_local_dev_info(BT_LOCAL_DEV *local_dev);
INT32 bt_mw_gap_start_inquiry(UINT32 ui4_filter_type);
INT32 bt_mw_gap_stop_inquiry(VOID);
INT32 bt_mw_gap_pair(CHAR *bd_addr, int transport);
INT32 bt_mw_gap_unpair(CHAR *bd_addr);
INT32 bt_mw_gap_cancel_pair(CHAR *bd_addr);
INT32 bt_mw_gap_interop_database_clear(VOID);
INT32 bt_mw_gap_interop_database_add(CHAR *bd_addr, BTMW_GAP_INTEROP_FEATURE feature, UINT8 len);
INT32 bt_mw_gap_get_rssi(CHAR *bd_addr, INT16 *rssi_value);
INT32 bt_mw_gap_send_hci(CHAR *buffer);
VOID bt_mw_gap_register_profile(UINT8 id, profile_operator_t *op);
VOID bt_mw_gap_get_bonded_dev(VOID);

#ifdef __cplusplus
}
#endif

#endif /*  _BT_MW_GAP_H_ */

