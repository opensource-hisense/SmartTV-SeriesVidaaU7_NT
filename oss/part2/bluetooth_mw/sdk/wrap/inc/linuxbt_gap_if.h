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

#ifndef __LINUXBT_GAP_IF_H__
#define __LINUXBT_GAP_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef VOID (*gap_get_pin_code_cb)(UINT8 *pin_code, UINT8 *fg_accept);
typedef VOID (*gap_get_passkey_cb)(UINT32 passkey, UINT8 *fg_accept, UINT8 *addr, UINT8 *name, UINT32 cod);
typedef VOID (*gap_add_bonded_dev_cb)(CHAR *pbt_addr);
typedef VOID (*gap_get_rssi_cb)(INT16 rssi_value);

typedef struct
{
    gap_get_pin_code_cb pin_cb;
    gap_get_passkey_cb passkey_cb;
    gap_add_bonded_dev_cb add_bonded_cb;
    gap_get_rssi_cb get_rssi_cb;
} BT_GAP_CB_FUNC;

typedef enum
{
    BT_GAP_SCAN_MODE_NONE = 0,
    BT_GAP_SCAN_MODE_CONNECTABLE,
    BT_GAP_SCAN_MODE_CONNECTABLE_DISCOVERABLE,
} BT_GAP_SCAN_MODE;

BT_ERR_STATUS_T linuxbt_gap_init(BT_GAP_CB_FUNC *func, VOID *stack_handle);
BT_ERR_STATUS_T linuxbt_gap_deinit(void);
BT_ERR_STATUS_T linuxbt_gap_enable(void);
BT_ERR_STATUS_T linuxbt_gap_disable(void);
BT_ERR_STATUS_T linuxbt_gap_start_discovery(void);
BT_ERR_STATUS_T linuxbt_gap_cancel_discovery(void);
BT_ERR_STATUS_T linuxbt_gap_create_bond(char *pbt_addr, int transport);
BT_ERR_STATUS_T linuxbt_gap_remove_bond(char *pbt_addr);
BT_ERR_STATUS_T linuxbt_gap_cancel_bond(char *pbt_addr);
BT_ERR_STATUS_T linuxbt_gap_set_scan_mode(BT_GAP_SCAN_MODE mode);
BT_ERR_STATUS_T linuxbt_gap_set_local_name(char *pname);
BT_ERR_STATUS_T linuxbt_gap_get_adapter_properties(void);
const void *linuxbt_gap_get_profile_interface(const char *profile_id);
BT_ERR_STATUS_T linuxbt_gap_interop_database_clear(void);
BT_ERR_STATUS_T linuxbt_gap_interop_database_add(UINT16 feature, char *pbt_addr, size_t len);
BT_ERR_STATUS_T linuxbt_gap_get_rssi(char *pbt_addr);
BT_ERR_STATUS_T linuxbt_gap_send_hci(char *ptr);
BT_ERR_STATUS_T linuxbt_gap_set_bt_wifi_ratio(UINT8 bt_ratio, UINT8 wifi_ratio);

#ifdef __cplusplus
}
#endif

#endif /* __LINUXBT_GAP_IF_H__ */
