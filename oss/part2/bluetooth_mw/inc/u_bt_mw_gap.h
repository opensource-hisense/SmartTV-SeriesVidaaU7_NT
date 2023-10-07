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

/* FILE NAME:  u_bt_mw_gap.h
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
#ifndef U_BT_MW_GAP_H
#define U_BT_MW_GAP_H

#include "u_bt_mw_common.h"

#define MAX_PIN_LEN                      ((UINT32)     16)

#define BT_INQUIRY_FILTER_TYPE_ALL 0xFF
#define BT_INQUIRY_FILTER_TYPE_A2DP_SRC (1<<0)
#define BT_INQUIRY_FILTER_TYPE_A2DP_SNK (1<<1)
#define BT_INQUIRY_FILTER_TYPE_HID (1<<2)
#define BT_INQUIRY_FILTER_TYPE_HFP (1<<3)
#define BT_INQUIRY_FILTER_TYPE_SPP (1<<4)
#define BT_INQUIRY_FILTER_TYPE_BMR (1<<5)
#define BT_INQUIRY_FILTER_TYPE_UMR (1<<6)

#define BT_GAP_RES_SERVICE_ID      0           /* Reserved */
#define BT_GAP_SPP_SERVICE_ID      1           /* Serial port profile. */
#define BT_GAP_DUN_SERVICE_ID      2           /* Dial-up networking profile. */
#define BT_GAP_A2DP_SOURCE_SERVICE_ID      3   /* A2DP Source profile. */
#define BT_GAP_LAP_SERVICE_ID      4           /* LAN access profile. */
#define BT_GAP_HSP_SERVICE_ID      5           /* Headset profile. */
#define BT_GAP_HFP_SERVICE_ID      6           /* Hands-free profile. */
#define BT_GAP_OPP_SERVICE_ID      7           /* Object push  */
#define BT_GAP_FTP_SERVICE_ID      8           /* File transfer */
#define BT_GAP_CTP_SERVICE_ID      9           /* Cordless Terminal */
#define BT_GAP_ICP_SERVICE_ID      10          /* Intercom Terminal */
#define BT_GAP_SYNC_SERVICE_ID     11          /* Synchronization */
#define BT_GAP_BPP_SERVICE_ID      12          /* Basic printing profile */
#define BT_GAP_BIP_SERVICE_ID      13          /* Basic Imaging profile */
#define BT_GAP_PANU_SERVICE_ID     14          /* PAN User */
#define BT_GAP_NAP_SERVICE_ID      15          /* PAN Network access point */
#define BT_GAP_GN_SERVICE_ID       16          /* PAN Group Ad-hoc networks */
#define BT_GAP_SAP_SERVICE_ID      17          /* SIM Access profile */
#define BT_GAP_A2DP_SINK_SERVICE_ID        18  /* A2DP Sink */
#define BT_GAP_AVRCP_SERVICE_ID    19          /* A/V remote control */
#define BT_GAP_HID_SERVICE_ID      20          /* HID */
#define BT_GAP_VDP_SERVICE_ID      21          /* Video distribution */
#define BT_GAP_PBAP_SERVICE_ID     22          /* PhoneBook Access Server*/
#define BT_GAP_HSP_HS_SERVICE_ID   23          /* HFP HS role */
#define BT_GAP_HFP_HS_SERVICE_ID   24          /* HSP HS role */
#define BT_GAP_MAP_SERVICE_ID      25          /* Message Access Profile */
#define BT_GAP_MN_SERVICE_ID       26          /* Message Notification Service */
#define BT_GAP_HDP_SERVICE_ID      27          /* Health Device Profile */
#define BT_GAP_PCE_SERVICE_ID      28          /* PhoneBook Access Client*/
#define BT_GAP_SDP_SERVICE_ID      29          /* SDP Search*/
#define BT_GAP_BLE_SERVICE_ID      30          /* GATT profile */

#define BT_LE_AUDIO_BASS_SERVICE_ID      0     /* BMR role. */
#define BT_LE_AUDIO_ASCS_SERVICE_ID      1     /* UMR role. */

#define BT_GAP_DEVICE_TYPE_BREDR   0x01        /* For device type BREDR only */
#define BT_GAP_DEVICE_TYPE_BLE     0x02        /* For device type BLE only */
#define BT_GAP_DEVICE_TYPE_DUMO    0x03        /* For device type dual mode */

#define BT_GAP_INQUIRY_STOPPED     0
#define BT_GAP_INQUIRY_STARTED     1

typedef enum
{
   GAP_STATE_ON = 100,
   GAP_STATE_OFF ,
   GAP_STATE_ACL_CONNECTED ,
   GAP_STATE_ACL_DISCONNECTED ,
   GAP_STATE_BONDED,
   GAP_STATE_BONDING,
   GAP_STATE_NO_BOND, /* pair fail */
   GAP_STATE_UNPAIR_SUCCESS, /* unpair success */
   GAP_STATE_DISCOVERY_STARTED,
   GAP_STATE_DISCOVERY_STOPED,
   GAP_STATE_RESET,
} BTMW_GAP_STATE;

typedef enum
{
    GAP_SCAN_MODE_NONE,
    GAP_SCAN_MODE_DISCOVERABLE,
    GAP_SCAN_MODE_CONNECTABLE,
    GAP_SCAN_MODE_CONNECTABLE_DISCOVERABLE
} BTMW_GAP_SCAN_MODE;

typedef enum
{
    GAP_TRANSPORT_TYPE_UNKNOWN,
    GAP_TRANSPORT_TYPE_BR_EDR,
    GAP_TRANSPORT_TYPE_BLE
} BTMW_GAP_TRANSPORT_TYPE;

typedef struct
{
    BTMW_GAP_STATE state;
    UINT8 reason;
    UINT8 link_type; /* 0:Unknown,1:BR/EDR, 2:BLE */
    CHAR bd_addr[MAX_BDADDR_LEN];
} BTMW_GAP_EVT;

typedef enum
{
    GAP_PROPERTY_BDNAME = 0x1,
    GAP_PROPERTY_BDADDR,
    GAP_PROPERTY_UUIDS,
    GAP_PROPERTY_CLASS_OF_DEVICE,
    GAP_PROPERTY_TYPE_OF_DEVICE,
    GAP_PROPERTY_SERVICE_RECORD,
    GAP_PROPERTY_ADAPTER_SCAN_MODE,
    GAP_PROPERTY_ADAPTER_BONDED_DEVICES,
    GAP_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
    GAP_PROPERTY_REMOTE_FRIENDLY_NAME,
    GAP_PROPERTY_REMOTE_RSSI,
    GAP_PROPERTY_REMOTE_VERSION_INFO,
    GAP_PROPERTY_LOCAL_LE_FEATURES,
    GAP_PROPERTY_LOCAL_IO_CAPS,
    GAP_PROPERTY_LOCAL_IO_CAPS_BLE,
    GAP_PROPERTY_REMOTE_DEVICE_TIMESTAMP = 0xFF,
} BTMW_GAP_PROPERTY;

typedef enum
{
    // Disable secure connections
    // This is for pre BT 4.1/2 devices that do not handle secure mode
    // very well.
    GAP_INTEROP_DISABLE_LE_SECURE_CONNECTIONS = 0,

    // Some devices have proven problematic during the pairing process, often
    // requiring multiple retries to complete pairing. To avoid degrading the user
    // experience for those devices, automatically re-try pairing if page
    // timeouts are received during pairing.
    GAP_INTEROP_AUTO_RETRY_PAIRING,

    // Devices requiring this workaround do not handle Bluetooth Absolute Volume
    // control correctly, leading to undesirable (potentially harmful) volume
    // levels or general lack of controlability.
    GAP_INTEROP_DISABLE_ABSOLUTE_VOLUME,

    // Disable automatic pairing with headsets/car-kits
    // Some car kits do not react kindly to a failed pairing attempt and
    // do not allow immediate re-pairing. Blacklist these so that the initial
    // pairing attempt makes it to the user instead.
    GAP_INTEROP_DISABLE_AUTO_PAIRING,

    // Use a fixed pin for specific keyboards
    // Keyboards should use a variable pin at all times. However, some keyboards
    // require a fixed pin of all 0000. This workaround enables auto pairing for
    // those keyboards.
    GAP_INTEROP_KEYBOARD_REQUIRES_FIXED_PIN,

    // Some headsets have audio jitter issues because of increased
    // re-transmissions as the 3 Mbps packets have a lower link margin, and are
    // more prone to interference. We can disable 3DH packets (use only 2DH
    // packets) for the ACL link to improve sensitivity when streaming A2DP audio
    // to the headset. Air sniffer logs show reduced re-transmissions after
    // switching to 2DH packets.

    // Disable 3Mbps packets and use only 2Mbps packets for ACL links when
    // streaming audio.
    GAP_INTEROP_2MBPS_LINK_ONLY,

    // Do not use supervision timeout value received from preferred connection
    // parameters, use 3s instead. Use with HID only.
    GAP_INTEROP_HID_PREF_CONN_SUP_TIMEOUT_3S,

    // Do not send service changed indications (GATT client).
    // This should be removed after the characteristic is implmeented b/62088395.
    GAP_INTEROP_GATTC_NO_SERVICE_CHANGED_IND,

    // Do not use AVDTP RECONFIGURE when reconfiguring A2DP streams.
    // Some A2DP Sink devices report SUCCESS to the AVDTP RECONFIGURE command,
    // but fail to play the reconfigured audio stream.
    GAP_INTEROP_DISABLE_AVDTP_RECONFIGURE,

    // Create dynamic blacklist to disable role switch.
    // Some car kits indicate that role switch is supported, but then reject
    // role switch attempts. After rejecting several role switch attempts,
    // such car kits will go into bad state.
    GAP_INTEROP_DYNAMIC_ROLE_SWITCH,

    // Disable role switch for headsets/car-kits.
    // Some car kits allow role switch but when the Phone initiates role switch,
    // the Remote device will go into bad state that will lead to LMP time out.
    GAP_INTEROP_DISABLE_ROLE_SWITCH,

    // Set a very low initial sniff subrating for HID devices that do not
    // set their own sniff interval.
    GAP_INTEROP_HID_HOST_LIMIT_SNIFF_INTERVAL,

    // Disable remote name requst for some devices.
    // The public address of these devices are same as the Random address in ADV.
    // Then will get name by LE_Create_connection, actually fails,
    // but will block pairing.
    GAP_INTEROP_DISABLE_NAME_REQUEST,
} BTMW_GAP_INTEROP_FEATURE;

typedef struct
{
    BTMW_DEVICE_KIND device_kind;
    UINT32           prop_mask; /* property mask */
    BLUETOOTH_DEVICE device;
} BTMW_GAP_DEVICE_INFO;

typedef struct _BT_LOCAL_DEV
{
    CHAR                  addr[MAX_BDADDR_LEN];  /* Bluetooth Address */
    CHAR                  name[MAX_NAME_LEN];    /* Name of device    */
    BTMW_GAP_STATE        state;
    UINT8                 scan_mode;             /* user expectd mode */
    UINT8                 local_scan_mode;       /* local scan mode  */
    UINT32                service;
    UINT8                 inquiry_state;         /* 0-is not inquiry */
    BTMW_LE_FEATURES      le_featrues;
    BOOL                  ext_features_updated;
    BTMW_EXT_FEATURES     ext_features;
} BT_LOCAL_DEV;

typedef enum pairing_key_type_s
{
    PIN_CODE = 0,
    PASSKEY,
    Key_TYPE_END
} pairing_key_type_t;

typedef struct pairing_key_value_s
{
    pairing_key_type_t key_type;
    CHAR pin_code[MAX_PIN_LEN]; /* When key_type=PIN_CODE, this value is available*/
    UINT32 key_value; /* When key_type=PASS_KEY, this value is available*/
    UINT8 addr[6];
    UINT8 name[249];
    UINT32 cod;
} pairing_key_value_t;

typedef VOID (*BtAppGapEventCbk)(BTMW_GAP_EVT *gap_event);
typedef VOID (*BtAppGapInquiryResponseCbk)(BTMW_GAP_DEVICE_INFO* pt_result);
typedef VOID (*BtAppGapGetPairingKeyCbk)(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept);
typedef VOID (*BtAppLogOut)(char *log_str);

typedef struct _BT_APP_CB_FUNC
{
    BtAppGapEventCbk bt_gap_event_cb;
    BtAppGapGetPairingKeyCbk bt_get_pairing_key_cb;
    BtAppGapInquiryResponseCbk bt_dev_info_cb;
    BtAppLogOut bt_app_log_cb;
}BT_APP_CB_FUNC;

#endif /* End of U_BT_MW_GAP_H */

