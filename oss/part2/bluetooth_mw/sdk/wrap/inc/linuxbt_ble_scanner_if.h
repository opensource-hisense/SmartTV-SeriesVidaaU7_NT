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

#ifndef __LINUXBT_BLE_SCANNER_IF_H__
#define __LINUXBT_BLE_SCANNER_IF_H__

#include "u_bt_mw_ble_scanner.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * FUNCTION NAME: linuxbt_ble_scanner_register_app
 * PURPOSE:
 *      The function is used to register ble scanner APP.
 * INPUT:
 *      uuid               -- app uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_register(char *pt_uuid);



/**
 * FUNCTION NAME: linuxbt_ble_scan
 * PURPOSE:
 *      The function is used to start or stop ble scan.
 * INPUT:
 *       [in]start              -- start scan or stop scan. (1:start scan, 0:stop  scan)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 linuxbt_ble_scanner_scan(BOOL start);

/**
 * FUNCTION NAME: linuxbt_ble_scanner_scan_filter_param_setup
 * PURPOSE:
 *      The function is used to set scan filter param.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]action: 0x00 is add, 0x01 is delete, 0x02 is clear
         [in]filt_index: filter index
         [in]params: scan filter parameter
             [in]feat_seln: bit masks for the selected features
                  bit 0: set to enable Broadcast Address filter
                  bit 1: set to enable Service Data Change filter
                  bit 2: set to enable Service UUID check
                  bit 3: set to enable Service Soliciation UUID check
                  bit 4: set to enable Local Name check
                  bit 5: set to enable Manufacturer Data Check
                  bit 6: set to enable Service Data Check
             [in]list_logic_type: logic operation for each feature selection(per bit position) specified in feat_seln
                  bit position value: 0 is OR, 1 is AND
             [in]filt_logic_type: the logic to filter for each filter field, it only applicable for(bit 3 ~ bit 6) for fields feat_seln
                  0x00 is OR, 0x01 is AND
             [in]rssi_high_thres: in dbm the advertiser is deemed seen ONLY if the signal is higher than rssi hign threshold
             [in]rssi_low_thres: in dbm the advertiser packet is considered as not seen, if the rssi of the received packet is not above the rssi low threshold
             [in]dely_mode: 0x00 is immediate, 0x01 is on_found, 0x02 is batched
             [in]found_timeout: time for firmware to linger and collect additional advertisements before reporting, valid only if dely_mode is on_found
             [in]lost_timeout: An advertisemend, after found, is not seen contiguously for lost_timeout period, it will be reported lost.
                  Reporting of lost is immediate
             [in]found_timeout_cnt: match mode for Bluetooth LE scan filters hardward match
                 In Aggressive mode, hw will determine a match sooner even with feeble signal strength
                 In sticky mode, higher threshold of signal strength and sightings is required before reporting by hw
             [in]num_of_tracking_entries: how many advertisements to match per filter
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_scan_filter_param_setup(UINT8 scanner_id, UINT8 action, UINT8 filt_index,
                                                                    BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filter_param);

/**
 * FUNCTION NAME: linuxbt_ble_scan_filter_add
 * PURPOSE:
 *      The function is used to add scan filter.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]filt_index: filter index
         [in]p_apcf_cmd: BLE APCF Command
             [in]type: APCF Command type
             [in]bt_addr: 6-byte device address
             [in]addr_type: 0x00 is Public, 0x01 is Random, 0x02 is NA(addresses type not applicable)
             [in]uuid: the service UUID(16 bit,32bit, or 128 bit) to add to or delete from the list
             [in]uuid_mask: the service UUID Mask(16 bit,32bit, or 128 bit) to add to the list, it should have the same length as APCF_UUID
             [in]name: a character string for local name
                  note: currently the max number of characters in a local name string is 29
             [in]company: A character string for manufacturer data
             [in]company_mask: the manufacture data mask to add to the list, it should have same length as company
             [in]data: A character string for service data
             [in]data_mask: the service data mask to add to the list, it should have same length as service data
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_scan_filter_add(UINT8 scanner_id, UINT8 filter_index, BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data);


/**
 * FUNCTION NAME: linuxbt_ble_scan_filter_clear
 * PURPOSE:
 *      The function is used to Clear all scan filter conditions for specific filter index.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]filt_index: filter index
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_scan_filter_clear(UINT8 scanner_id, UINT8 filt_index);


/**
 * FUNCTION NAME: linuxbt_ble_scan_filter_enable
 * PURPOSE:
 *      The function is used to enable scan filter.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]enable: 0 is disable, 1 is enable
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_scan_filter_enable(UINT8 scanner_id, BOOL enable);


/**
 * FUNCTION NAME: linuxbt_ble_set_scan_parameters
 * PURPOSE:
 *      The function is used to set scan parameters.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]scan_interval: how frequently the controller should scan
         [in]scan_window: how long the Controller should scan
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_set_scan_param(UINT8 scanner_id, INT32 scan_interval, INT32 scan_window);


/**
 * FUNCTION NAME: linuxbt_ble_batchscan_cfg_storage
 * PURPOSE:
 *      The function is used to set batch scan storage parameters.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]batch_scan_full_max: max storage space(in %) allocated to full style [Range: 0 ~ 100]
         [in]batch_scan_trunc_max:  max storage space(in %) allocated to truncated style [Range: 0 ~ 100]
         [in]batch_scan_notify_threshold: setup notification level(in %) for individual storage pool[Range:0~ =100]
              setting it 0 will disable notification, Vendor specific HCI event generated(storage threshold breach sub event)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_batchscan_cfg_storage(UINT8 scanner_id, UINT8 batch_scan_full_max,
                                                  UINT8 batch_scan_trunc_max,
                                                  UINT8 batch_scan_notify_threshold);


/**
 * FUNCTION NAME: linuxbt_ble_batchscan_enable
 * PURPOSE:
 *      The function is used to enable batch scan.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]scan_mode: scan mode
              0x00: batch scan is disabled
              0x01: truncated mode enable
              0x02: full mode enabled
              0x03: truncated & full mode enabled
         [in]scan_interval: batch scan scan time
         [in]scan_window: batch scan interval period
         [in]addr_type: 0x00 is Public device address, 0x01 is Random device address
         [in]discard_rule: 0: discard oldest advertisement,  1: discard advertisement with weakest RSSSI
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_batchscan_enable(UINT8 scanner_id, UINT8 scan_mode, INT32 scan_interval,
                                                   INT32 scan_window, UINT8 addr_type, UINT8 discard_rule);


/**
 * FUNCTION NAME: linuxbt_ble_batchscan_disable
 * PURPOSE:
 *      The function is used to disable batch scan.
 * INPUT:
         [in]scanner_id: registered scanner_id
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_batchscan_disable(UINT8 scanner_id);

/**
 * FUNCTION NAME: linuxbt_ble_batchscan_read_reports
 * PURPOSE:
 *      The function is used to read batchscan report.
 * INPUT:
         [in]scanner_id: registered scanner_id
         [in]scan_mode: 0x01: truncated mode, 0x02: full mode
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_ble_scanner_batchscan_read_reports(UINT8 scanner_id, UINT8 scan_mode);

int linuxbt_ble_scanner_unregister(UINT8 scanner_id);


#ifdef __cplusplus
}
#endif

#endif /* __LINUXBT_BLE_SCANNER_IF_H__ */
