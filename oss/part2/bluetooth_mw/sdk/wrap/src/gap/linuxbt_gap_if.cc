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

/* FILE NAME:  linuxbt_gap_if.c
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
 */

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "bt_mw_common.h"
#include "bluetooth.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_gap.h"

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
#include "mtk_bluetooth.h"
#endif
#include"bt_mw_message_queue.h"

using bluetooth::Uuid;

#define UNUSED_ATTR __attribute__((unused))

#define LINUXBT_GAP_PROTOCOL_ATT             0x0007

/* Define common 16-bit service class UUIDs
*/
#define LINUXBT_GAP_SERVICE_DISCOVERY_SERVER 0X1000
#define LINUXBT_GAP_BROWSE_GROUP_DESCRIPTOR  0X1001
#define LINUXBT_GAP_PUBLIC_BROWSE_GROUP      0X1002
#define LINUXBT_GAP_SERIAL_PORT              0X1101
#define LINUXBT_GAP_LAN_ACCESS_USING_PPP     0X1102
#define LINUXBT_GAP_DIALUP_NETWORKING        0X1103
#define LINUXBT_GAP_IRMC_SYNC                0X1104
#define LINUXBT_GAP_OBEX_OBJECT_PUSH         0X1105
#define LINUXBT_GAP_OBEX_FILE_TRANSFER       0X1106
#define LINUXBT_GAP_IRMC_SYNC_COMMAND        0X1107
#define LINUXBT_GAP_HEADSET                  0X1108
#define LINUXBT_GAP_CORDLESS_TELEPHONY       0X1109
#define LINUXBT_GAP_AUDIO_SOURCE             0X110A
#define LINUXBT_GAP_AUDIO_SINK               0X110B
#define LINUXBT_GAP_AV_REM_CTRL_TARGET       0X110C  /* Audio/Video Control profile */
#define LINUXBT_GAP_ADV_AUDIO_DISTRIBUTION   0X110D  /* Advanced Audio Distribution profile */
#define LINUXBT_GAP_AV_REMOTE_CONTROL        0X110E  /* Audio/Video Control profile */
#define LINUXBT_GAP_AV_REM_CTRL_CONTROL      0X110F  /* Audio/Video Control profile */
#define LINUXBT_GAP_INTERCOM                 0X1110
#define LINUXBT_GAP_FAX                      0X1111
#define LINUXBT_GAP_HEADSET_AUDIO_GATEWAY    0X1112
#define LINUXBT_GAP_WAP                      0X1113
#define LINUXBT_GAP_WAP_CLIENT               0X1114
#define LINUXBT_GAP_PANU                     0X1115  /* PAN profile */
#define LINUXBT_GAP_NAP                      0X1116  /* PAN profile */
#define LINUXBT_GAP_GN                       0X1117  /* PAN profile */
#define LINUXBT_GAP_DIRECT_PRINTING          0X1118  /* BPP profile */
#define LINUXBT_GAP_REFERENCE_PRINTING       0X1119  /* BPP profile */
#define LINUXBT_GAP_IMAGING                  0X111A  /* Imaging profile */
#define LINUXBT_GAP_IMAGING_RESPONDER        0X111B  /* Imaging profile */
#define LINUXBT_GAP_IMAGING_AUTO_ARCHIVE     0X111C  /* Imaging profile */
#define LINUXBT_GAP_IMAGING_REF_OBJECTS      0X111D  /* Imaging profile */
#define LINUXBT_GAP_HF_HANDSFREE             0X111E  /* Handsfree profile */
#define LINUXBT_GAP_AG_HANDSFREE             0X111F  /* Handsfree profile */
#define LINUXBT_GAP_DIR_PRT_REF_OBJ_SERVICE  0X1120  /* BPP profile */
#define LINUXBT_GAP_REFLECTED_UI             0X1121  /* BPP profile */
#define LINUXBT_GAP_BASIC_PRINTING           0X1122  /* BPP profile */
#define LINUXBT_GAP_PRINTING_STATUS          0X1123  /* BPP profile */
#define LINUXBT_GAP_HUMAN_INTERFACE          0X1124  /* HID profile */
#define LINUXBT_GAP_CABLE_REPLACEMENT        0X1125  /* HCRP profile */
#define LINUXBT_GAP_HCRP_PRINT               0X1126  /* HCRP profile */
#define LINUXBT_GAP_HCRP_SCAN                0X1127  /* HCRP profile */
#define LINUXBT_GAP_COMMON_ISDN_ACCESS       0X1128  /* CAPI Message Transport Protocol*/
#define LINUXBT_GAP_VIDEO_CONFERENCING_GW    0X1129  /* Video Conferencing profile */
#define LINUXBT_GAP_UDI_MT                   0X112A  /* Unrestricted Digital Information profile */
#define LINUXBT_GAP_UDI_TA                   0X112B  /* Unrestricted Digital Information profile */
#define LINUXBT_GAP_VCP                      0X112C  /* Video Conferencing profile */
#define LINUXBT_GAP_SAP                      0X112D  /* SIM Access profile */
#define LINUXBT_GAP_PBAP_PCE                 0X112E  /* Phonebook Access - PCE */
#define LINUXBT_GAP_PBAP_PSE                 0X112F  /* Phonebook Access - PSE */
#define LINUXBT_GAP_PHONE_ACCESS             0x1130
#define LINUXBT_GAP_HEADSET_HS               0x1131  /* Headset - HS, from HSP v1.2 */
#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)
#define LINUXBT_GAP_3D_DISPLAY               0x1137  /* B3DS_INCLUDED */
#define LINUXBT_GAP_3D_GLASSES               0x1138  /* B3DS_INCLUDED */
#define LINUXBT_GAP_3D_SYNCHRONIZATION       0x1139  /* B3DS_INCLUDED */
#endif
#define LINUXBT_GAP_PNP_INFORMATION          0X1200  /* Device Identification */
#define LINUXBT_GAP_GENERIC_NETWORKING       0X1201
#define LINUXBT_GAP_GENERIC_FILETRANSFER     0X1202
#define LINUXBT_GAP_GENERIC_AUDIO            0X1203
#define LINUXBT_GAP_GENERIC_TELEPHONY        0X1204
#define LINUXBT_GAP_UPNP_SERVICE             0X1205  /* UPNP_Service [ESDP] */
#define LINUXBT_GAP_UPNP_IP_SERVICE          0X1206  /* UPNP_IP_Service [ESDP] */
#define LINUXBT_GAP_ESDP_UPNP_IP_PAN         0X1300  /* UPNP_IP_PAN [ESDP] */
#define LINUXBT_GAP_ESDP_UPNP_IP_LAP         0X1301  /* UPNP_IP_LAP [ESDP] */
#define LINUXBT_GAP_ESDP_UPNP_IP_L2CAP       0X1302  /* UPNP_L2CAP [ESDP] */
#define LINUXBT_GAP_VIDEO_SOURCE             0X1303  /* Video Distribution Profile (VDP) */
#define LINUXBT_GAP_VIDEO_SINK               0X1304  /* Video Distribution Profile (VDP) */
#define LINUXBT_GAP_VIDEO_DISTRIBUTION       0X1305  /* Video Distribution Profile (VDP) */
#define LINUXBT_GAP_HDP_PROFILE              0X1400  /* Health Device profile (HDP) */
#define LINUXBT_GAP_HDP_SOURCE               0X1401  /* Health Device profile (HDP) */
#define LINUXBT_GAP_HDP_SINK                 0X1402  /* Health Device profile (HDP) */
#define LINUXBT_GAP_MAP_PROFILE              0X1134  /* MAP profile UUID */
#define LINUXBT_GAP_MESSAGE_ACCESS           0X1132  /* Message Access Service UUID */
#define LINUXBT_GAP_MESSAGE_NOTIFICATION     0X1133  /* Message Notification Service UUID */

#define LINUXBT_GAP_GAP_SERVER               0x1800
#define LINUXBT_GAP_GATT_SERVER              0x1801
#define LINUXBT_GAP_IMMEDIATE_ALERT          0x1802      /* immediate alert */
#define LINUXBT_GAP_LINKLOSS                 0x1803      /* Link Loss Alert */
#define LINUXBT_GAP_TX_POWER                 0x1804      /* TX power */
#define LINUXBT_GAP_CURRENT_TIME             0x1805      /* Link Loss Alert */
#define LINUXBT_GAP_DST_CHG                  0x1806      /* DST Time change */
#define LINUXBT_GAP_REF_TIME_UPD             0x1807      /* reference time update */
#define LINUXBT_GAP_THERMOMETER              0x1809      /* Thermometer UUID */
#define LINUXBT_GAP_DEVICE_INFO              0x180A      /* device info service */
#define LINUXBT_GAP_NWA                      0x180B      /* Network availability */
#define LINUXBT_GAP_HEART_RATE               0x180D      /* Heart Rate service */
#define LINUXBT_GAP_PHALERT                  0x180E      /* phone alert service */
#define LINUXBT_GAP_BATTERY                  0x180F     /* battery service */
#define LINUXBT_GAP_BPM                      0x1810      /*  blood pressure service */
#define LINUXBT_GAP_ALERT_NOTIFICATION       0x1811      /* alert notification service */
#define LINUXBT_GAP_LE_HID                   0x1812     /*  HID over LE */
#define LINUXBT_GAP_SCAN_PARAM               0x1813      /* Scan Parameter service */
#define LINUXBT_GAP_GLUCOSE                  0x1808      /* Glucose Meter Service */
#define LINUXBT_GAP_RSC                      0x1814      /* RUNNERS SPEED AND CADENCE SERVICE      */
#define LINUXBT_GAP_CSC                      0x1816      /* Cycling SPEED AND CADENCE SERVICE      */

#define LINUXBT_LE_AUDIO_BASS                0x184F  /* Broadcast Audio Scan Service */
#define LINUXBT_LE_AUDIO_ASCS                0x184E  /* Audio Stream Control Service */

static void _linuxbt_gap_state_changed_cb(bt_state_t state);
static void _linuxbt_gap_properties_cb(bt_status_t status,
                               int num_properties,
                               bt_property_t *properties);
static void _linuxbt_gap_remote_device_properties_cb(bt_status_t status,
        RawAddress* bd_addr,
        int num_properties,
        bt_property_t *properties);
static void _linuxbt_gap_device_found_cb(int num_properties,
                                 bt_property_t *properties);
static void _linuxbt_gap_discovery_state_changed_cb(bt_discovery_state_t state);
static void _linuxbt_gap_pin_request_cb(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bool min_16_digit);
static void _linuxbt_gap_ssp_request_cb(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bt_ssp_variant_t pairing_variant,
                                     uint32_t pass_key);
static void _linuxbt_gap_bond_state_changed_cb(bt_status_t status,
                                            RawAddress* remote_bd_addr,
                                            bt_bond_state_t state);
static void _linuxbt_gap_acl_state_changed_cb(bt_status_t status,
                                           RawAddress* remote_bd_addr,
                                           bt_acl_state_t state);
static void _linuxbt_gap_acl_state_changed_ex_callback(RawAddress* remote_bd_addr,
                                        bt_acl_state_t state,
                                        uint8_t reason,
                                        uint8_t link_type);
static void _linuxbt_gap_get_rssi_cb(bt_status_t status, RawAddress* remote_bd_addr, int rssi_value);
static void _linuxbt_gap_ext_feature_support_callback(bt_local_ext_feature_support_t features);
static CHAR* _linuxbt_gap_get_service_str(UINT16 uuid);
static CHAR* _linuxbt_gap_get_property_type_str(bt_property_type_t type);
static CHAR* _linuxbt_gap_get_dev_type_str(UINT32 type);
static CHAR* _linuxbt_gap_get_bond_state_str(UINT32 bond_state);

static bool _linuxbt_gap_set_wake_alarm(uint64_t delay_millis, bool should_wake, alarm_cb cb, void *data)
{
    return true;
}

static int _linuxbt_gap_acquire_wake_lock(const char *lock_name)
{
    return 0;
}

static int _linuxbt_gap_release_wake_lock(const char *lock_name)
{
    return 0;
}

extern BOOL fg_bt_scan_ongoing;
extern bt_interface_t bluetoothInterface;
const bt_interface_t *g_bt_interface = NULL;
BT_GAP_CB_FUNC *g_bt_cb_func = NULL;
extern void linuxbt_ble_adv_set_adv_name(CHAR *local_name);

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
static const btgap_ex_interface_t *linuxbt_gap_ex_interface = NULL;

static btgap_ex_callbacks_t linuxbt_gap_ex_callbacks =
{
    sizeof(btgap_ex_callbacks_t),
    _linuxbt_gap_get_rssi_cb,
    _linuxbt_gap_acl_state_changed_ex_callback,
    _linuxbt_gap_ext_feature_support_callback,
};
#endif

static bt_os_callouts_t g_callouts =
{
    sizeof(bt_os_callouts_t),
    _linuxbt_gap_set_wake_alarm,
    _linuxbt_gap_acquire_wake_lock,
    _linuxbt_gap_release_wake_lock,
};

static bt_callbacks_t g_bt_callbacks =
{
    sizeof(bt_callbacks_t),
    _linuxbt_gap_state_changed_cb,
    _linuxbt_gap_properties_cb,
    _linuxbt_gap_remote_device_properties_cb,
    _linuxbt_gap_device_found_cb,
    _linuxbt_gap_discovery_state_changed_cb,
    _linuxbt_gap_pin_request_cb,
    _linuxbt_gap_ssp_request_cb,
    _linuxbt_gap_bond_state_changed_cb,
    _linuxbt_gap_acl_state_changed_cb,
    NULL,
    NULL,
    NULL,
    NULL,
};


const UINT16 linuxbt_gap_service_id_to_uuid_lkup_tbl [32] =
{
    LINUXBT_GAP_PNP_INFORMATION,         /* Reserved */
    LINUXBT_GAP_SERIAL_PORT,             /* BTA_SPP_SERVICE_ID */
    LINUXBT_GAP_DIALUP_NETWORKING,       /* BTA_DUN_SERVICE_ID */
    LINUXBT_GAP_AUDIO_SOURCE,            /* BTA_A2DP_SOURCE_SERVICE_ID */
    LINUXBT_GAP_LAN_ACCESS_USING_PPP,    /* BTA_LAP_SERVICE_ID */
    LINUXBT_GAP_HEADSET,                 /* BTA_HSP_HS_SERVICE_ID */
    LINUXBT_GAP_HF_HANDSFREE,            /* BTA_HFP_HS_SERVICE_ID */
    LINUXBT_GAP_OBEX_OBJECT_PUSH,        /* BTA_OPP_SERVICE_ID */
    LINUXBT_GAP_OBEX_FILE_TRANSFER,      /* BTA_FTP_SERVICE_ID */
    LINUXBT_GAP_CORDLESS_TELEPHONY,      /* BTA_CTP_SERVICE_ID */
    LINUXBT_GAP_INTERCOM,                /* BTA_ICP_SERVICE_ID */
    LINUXBT_GAP_IRMC_SYNC,               /* BTA_SYNC_SERVICE_ID */
    LINUXBT_GAP_DIRECT_PRINTING,         /* BTA_BPP_SERVICE_ID */
    LINUXBT_GAP_IMAGING_RESPONDER,       /* BTA_BIP_SERVICE_ID */
    LINUXBT_GAP_PANU,                    /* BTA_PANU_SERVICE_ID */
    LINUXBT_GAP_NAP,                     /* BTA_NAP_SERVICE_ID */
    LINUXBT_GAP_GN,                      /* BTA_GN_SERVICE_ID */
    LINUXBT_GAP_SAP,                     /* BTA_SAP_SERVICE_ID */
    LINUXBT_GAP_AUDIO_SINK,              /* BTA_A2DP_SERVICE_ID */
    LINUXBT_GAP_AV_REMOTE_CONTROL,       /* BTA_AVRCP_SERVICE_ID */
    LINUXBT_GAP_HUMAN_INTERFACE,         /* BTA_HID_SERVICE_ID */
    LINUXBT_GAP_VIDEO_SINK,              /* BTA_VDP_SERVICE_ID */
    LINUXBT_GAP_PBAP_PSE,                /* BTA_PBAP_SERVICE_ID */
    LINUXBT_GAP_HEADSET_AUDIO_GATEWAY,   /* BTA_HSP_SERVICE_ID */
    LINUXBT_GAP_AG_HANDSFREE,            /* BTA_HFP_SERVICE_ID */
    LINUXBT_GAP_MESSAGE_ACCESS,          /* BTA_MAP_SERVICE_ID */
    LINUXBT_GAP_MESSAGE_NOTIFICATION,    /* BTA_MN_SERVICE_ID */
    LINUXBT_GAP_HDP_PROFILE,             /* BTA_HDP_SERVICE_ID */
    LINUXBT_GAP_PBAP_PCE,                /* BTA_PCE_SERVICE_ID */
    LINUXBT_GAP_PROTOCOL_ATT             /* BTA_GATT_SERVICE_ID */
};

static CHAR* _linuxbt_gap_get_service_str(UINT16 uuid)
{
    switch (uuid)
    {
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_PNP_INFORMATION, "pnp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_SERIAL_PORT, "spp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_DIALUP_NETWORKING, "dun");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_AUDIO_SOURCE, "a2dp_source");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_LAN_ACCESS_USING_PPP, "lap");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_HEADSET, "hsp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_HF_HANDSFREE, "hfp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_OBEX_OBJECT_PUSH, "opp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_OBEX_FILE_TRANSFER, "ftp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_CORDLESS_TELEPHONY, "ctp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_INTERCOM, "icp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_IRMC_SYNC, "sync");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_DIRECT_PRINTING, "bpp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_IMAGING_RESPONDER, "bip");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_PANU, "panu");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_NAP, "nap");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_GN, "gn");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_SAP, "sap");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_AUDIO_SINK, "a2dp_sink");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_AV_REMOTE_CONTROL, "avrcp_ct");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_HUMAN_INTERFACE, "hid");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_VIDEO_SINK, "vdp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_PBAP_PSE, "pbap");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_HEADSET_AUDIO_GATEWAY, "hsp_gw");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_AG_HANDSFREE, "hfp_ag");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_MESSAGE_ACCESS, "map");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_MESSAGE_NOTIFICATION, "mn");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_HDP_PROFILE, "hdp");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_PBAP_PCE, "pbap_pce");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_GAP_PROTOCOL_ATT, "gatt");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_LE_AUDIO_BASS, "bass");
        BT_MW_GAP_CASE_RETURN_STR(LINUXBT_LE_AUDIO_ASCS, "ascs");
        default: return (CHAR*)"unknown";
    }
}

static CHAR* _linuxbt_gap_get_property_type_str(bt_property_type_t type)
{
    switch (type)
    {
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_BDNAME, "name");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_BDADDR, "addr");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_UUIDS, "uuids");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_CLASS_OF_DEVICE, "cod");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_TYPE_OF_DEVICE, "tod");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_SERVICE_RECORD, "service_record");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_ADAPTER_SCAN_MODE, "scan_mode");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_ADAPTER_BONDED_DEVICES, "bond_dev");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT, "disc_timeout");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_REMOTE_FRIENDLY_NAME, "alias");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_REMOTE_RSSI, "rssi");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_REMOTE_VERSION_INFO, "version");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_LOCAL_LE_FEATURES, "le_feature");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_LOCAL_IO_CAPS, "io_caps");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_LOCAL_IO_CAPS_BLE, "io_caps_ble");
        BT_MW_GAP_CASE_RETURN_STR(BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP, "remote_timestamp");
        default: return (CHAR*)"unknown";
    }
}

static CHAR* _linuxbt_gap_get_dev_type_str(UINT32 type)
{
    switch (type)
    {
        BT_MW_GAP_CASE_RETURN_STR(BT_GAP_DEVICE_TYPE_BREDR, "bredr");
        BT_MW_GAP_CASE_RETURN_STR(BT_GAP_DEVICE_TYPE_BLE, "ble");
        BT_MW_GAP_CASE_RETURN_STR(BT_GAP_DEVICE_TYPE_DUMO, "dumo");
        default: return (CHAR*)"unknown";
    }
}

static CHAR* _linuxbt_gap_get_bond_state_str(UINT32 bond_state)
{
    switch (bond_state)
    {
        BT_MW_GAP_CASE_RETURN_STR(BT_BOND_STATE_NONE, "none");
        BT_MW_GAP_CASE_RETURN_STR(BT_BOND_STATE_BONDING, "bonding");
        BT_MW_GAP_CASE_RETURN_STR(BT_BOND_STATE_BONDED, "bonded");
        default: return (CHAR*)"unknown";
    }
}

static UINT32 _linuxbt_gap_parse_uuid2service(Uuid *p_uuid, INT32 num_uuid, UINT32 *le_audio_service)
{
    UINT8 xx, yy;
    UINT32 service = 0;
    BT_CHECK_POINTER(BT_DEBUG_GAP, p_uuid);
    for( xx = 0; xx < num_uuid; xx++ )
    {
        for( yy = 0; yy < 32; yy++ )
        {
            if (*p_uuid == Uuid::From16Bit(linuxbt_gap_service_id_to_uuid_lkup_tbl[yy]))
            {
                BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                    _linuxbt_gap_get_service_str(linuxbt_gap_service_id_to_uuid_lkup_tbl[yy]));
                service |= (1 << yy);
                break;
            }
        }

        /* for HSP v1.2 only device */
        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_GAP_HEADSET_HS))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_GAP_HEADSET_HS));
            service |= (1 << BT_GAP_HSP_SERVICE_ID);
        }

        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_GAP_HDP_SOURCE))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_GAP_HDP_SOURCE));
            service |= (1 << BT_GAP_HDP_SERVICE_ID);
        }

        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_GAP_HDP_SINK))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_GAP_HDP_SINK));
            service |= (1 << BT_GAP_HDP_SERVICE_ID);
        }

        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_GAP_LE_HID))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_GAP_LE_HID));
            service |= (1 << BT_GAP_HID_SERVICE_ID);
        }

        /* for LE AUDIO device */
        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_LE_AUDIO_BASS))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_LE_AUDIO_BASS));
            (*le_audio_service)  |= (1 << BT_LE_AUDIO_BASS_SERVICE_ID);
        }

        if (*(p_uuid + xx) == Uuid::From16Bit(LINUXBT_LE_AUDIO_ASCS))
        {
            BT_DBG_INFO(BT_DEBUG_GAP, "service=%s",
                _linuxbt_gap_get_service_str(LINUXBT_LE_AUDIO_ASCS));
            (*le_audio_service)  |= (1 << BT_LE_AUDIO_ASCS_SERVICE_ID);
        }
    }

    return service;
}

static UINT32 _linuxbt_gap_parse_device_properties(BLUETOOTH_DEVICE *device,
                                                   int num_properties,
                                                   bt_property_t *properties)
{
    bt_property_t *property;
    CHAR *name;
    RawAddress* btaddr;
    UINT8 bonded_dev_num = 0;
    UINT32 prop_mask = 0;

    BT_CHECK_POINTER(BT_DEBUG_GAP, properties);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "============Propertie num : %d================",num_properties);
    memset(device, 0, sizeof(BLUETOOTH_DEVICE));
    for (UINT8 i = 0; i < num_properties; i++)
    {
        property = &properties[i];
        prop_mask |= (1 << property->type);
        switch (property->type)
        {
        case BT_PROPERTY_BDNAME:
            name = (CHAR *)property->val;
            if (strlen(name) > 0)
            {
                strncpy(device->name, name,
                    (UINT32)property->len > MAX_NAME_LEN ? MAX_NAME_LEN : property->len);
                device->name[(((UINT32)property->len >= MAX_NAME_LEN) ? (MAX_NAME_LEN - 1) : property->len)] = '\0';
                BT_DBG_NORMAL(BT_DEBUG_GAP, "bdname = %s",
                                          device->name);
            }
            else
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "type = %ld, len = %ld, bdname is null",
                                          (long)property->type,
                                          (long)property->len);
            }
            break;
        case BT_PROPERTY_BDADDR:
            btaddr = (RawAddress *)property->val;
            linuxbt_btaddr_htos(btaddr, device->bdAddr);
            BT_DBG_NORMAL(BT_DEBUG_GAP, "bdaddr = %s", device->bdAddr);
            break;
        case BT_PROPERTY_CLASS_OF_DEVICE:
            device->cod= *((UINT32 *)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "cod = 0x%x", (UINT32)(device->cod));
            break;
        case BT_PROPERTY_REMOTE_RSSI:
            device->rssi = *((INT8*)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "rssi = %d", device->rssi);
            break;
        case BT_PROPERTY_TYPE_OF_DEVICE:
            device->devicetype = *((UINT32 *)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "devtype = %s",
                _linuxbt_gap_get_dev_type_str(device->devicetype));
            break;
        case BT_PROPERTY_ADAPTER_SCAN_MODE:
            device->scan_mode= *((UINT32 *)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "scan mode = %d", *((UINT32 *)(property->val)));
            break;
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            BT_DBG_NORMAL(BT_DEBUG_GAP, "disc_timeout = %d", *((UINT32 *)(property->val)));
            break;
        case BT_PROPERTY_UUIDS:
            device->service =
                _linuxbt_gap_parse_uuid2service((Uuid*)property->val,
                    property->len/sizeof(Uuid), &device->le_audio_service);
            BT_DBG_NORMAL(BT_DEBUG_GAP, "uuid2service = 0x%x", device->service);
            BT_DBG_NORMAL(BT_DEBUG_GAP, "le_audio_service = 0x%x", device->le_audio_service);
            break;
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            {
                CHAR s_addr[MAX_BDADDR_LEN];
                bonded_dev_num = property->len / sizeof(RawAddress);
                btaddr = (RawAddress *)property->val;
                for (UINT8 k=0; k<bonded_dev_num; k++)
                {
                    BT_DBG_NORMAL(BT_DEBUG_GAP,
                        "bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X",
                              btaddr[k].address[0], btaddr[k].address[1],
                              btaddr[k].address[2], btaddr[k].address[3],
                              btaddr[k].address[4], btaddr[k].address[5]);

                    linuxbt_btaddr_htos(&btaddr[k], s_addr);
                    if (g_bt_cb_func && g_bt_cb_func->add_bonded_cb)
                    {
                        g_bt_cb_func->add_bonded_cb(s_addr);
                    }
                }
            }
            break;
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
            name = (CHAR *)property->val;
            BT_DBG_INFO(BT_DEBUG_GAP, "alias = %s", name);
            break;
        case BT_PROPERTY_REMOTE_VERSION_INFO:
            {
                bt_remote_version_t *version =
                    (bt_remote_version_t *)property->val;
                BT_DBG_INFO(BT_DEBUG_GAP, "version = %d.%d, manufacturer=%d",
                    version->version, version->sub_ver, version->manufacturer);
            }
            break;
        case BT_PROPERTY_LOCAL_LE_FEATURES:
            {
                bt_local_le_features_t *local_le_features =
                    (bt_local_le_features_t *)property->val;
                device->le_features_updated = TRUE;
                memcpy(&device->le_featrues, local_le_features, sizeof(bt_local_le_features_t));
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] version_supported = 0x%04x, local_privacy_enabled = 0x%02x",
                    local_le_features->version_supported, local_le_features->local_privacy_enabled);
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] max_adv_instance = 0x%02x, rpa_offload_supported = 0x%02x",
                    local_le_features->max_adv_instance, local_le_features->rpa_offload_supported);
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] max_irk_list_size = 0x%04x, max_adv_filter_supported = 0x%02x",
                    local_le_features->max_irk_list_size, local_le_features->max_adv_filter_supported);
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] extended_scan_support = 0x%02x, le_2m_phy_supported = 0x%02x",
                    local_le_features->extended_scan_support, local_le_features->le_2m_phy_supported);
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] le_coded_phy_supported = 0x%02x, le_extended_advertising_supported = 0x%02x",
                    local_le_features->le_coded_phy_supported, local_le_features->le_extended_advertising_supported);
                BT_DBG_INFO(BT_DEBUG_GAP, "[LE_FEATURES] le_periodic_advertising_supported = 0x%02x, le_maximum_advertising_data_length = 0x%04x",
                    local_le_features->le_periodic_advertising_supported, local_le_features->le_maximum_advertising_data_length);
            }
            break;
        default:
            BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %s(%d) len=%d",
                _linuxbt_gap_get_property_type_str(property->type),
                property->type, property->len);
            break;
        }
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP, "============Properties End================");

    return prop_mask;
}

static void _linuxbt_gap_properties_cb(bt_status_t status,
                               int num_properties,
                               bt_property_t *properties)
{

    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " status: %ld", (long)status);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, properties);
    if (BT_STATUS_SUCCESS == status)
    {
        msg.data.device_info.prop_mask =
            _linuxbt_gap_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
        msg.data.device_info.device_kind = BT_DEVICE_LOCAL;
        msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_DEVICE_INFO);
        linuxbt_send_msg(&msg);
    }
}

static void _linuxbt_gap_remote_device_properties_cb(bt_status_t status,
        RawAddress* bd_addr,
        int num_properties,
        bt_property_t *properties)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " status: %ld", (long)status);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, properties);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, bd_addr);
    if (BT_STATUS_SUCCESS != status)
    {
        return;
    }

    msg.data.device_info.prop_mask =
            _linuxbt_gap_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
    if(strlen(msg.data.device_info.device.bdAddr) == 0)
    {
        linuxbt_btaddr_htos(bd_addr, msg.data.device_info.device.bdAddr);
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP, "status:%d, device:%s", status, msg.data.device_info.device.bdAddr);

    if (fg_bt_scan_ongoing)
    {
        // When at the end of scan, host get the address of remote device, will go this.
        msg.data.device_info.device_kind = BT_DEVICE_SCAN;
    }
    else
    {
        // only when BT enable, will go this.
        msg.data.device_info.device_kind = BT_DEVICE_BONDED;
    }
    msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
    msg.hdr.len = sizeof(BTMW_GAP_DEVICE_INFO);

    if(strlen(msg.data.device_info.device.bdAddr) || strlen(msg.data.device_info.device.name))
    {
        linuxbt_send_msg(&msg);
    }
}

static void _linuxbt_gap_device_found_cb(int num_properties,
                                 bt_property_t *properties)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "device found");
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, properties);

    msg.data.device_info.prop_mask =
        _linuxbt_gap_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
    msg.data.device_info.device_kind = BT_DEVICE_SCAN;
    msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
    msg.hdr.len = sizeof(BTMW_GAP_DEVICE_INFO);
    linuxbt_send_msg(&msg);

}

static void _linuxbt_gap_pin_request_cb(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bool min_16_digit)
{
    bt_pin_code_t pin;
    int ret = BT_STATUS_SUCCESS;
    uint8_t fg_accept = 1;

    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()", __FUNCTION__);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, g_bt_interface);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, remote_bd_addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, bd_name);

    memset(pin.pin, 0, 16);
    if (g_bt_cb_func && g_bt_cb_func->pin_cb)
    {
        g_bt_cb_func->pin_cb(pin.pin, (UINT8 *)&fg_accept);
    }
    if (0 == strlen((CHAR*)&pin.pin))
    {
        if (strncmp("xs-soundbar", (CHAR*)&bd_name->name, 11) == 0)
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "PIN CODE default:0901");
            pin.pin[0] = 0x30;
            pin.pin[1] = 0x39;
            pin.pin[2] = 0x30;
            pin.pin[3] = 0x31;
        }
        else
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "PIN CODE default:0000");
            pin.pin[0] = 0x30;
            pin.pin[1] = 0x30;
            pin.pin[2] = 0x30;
            pin.pin[3] = 0x30;
        }
    }

    ret = g_bt_interface->pin_reply(remote_bd_addr, (uint8_t)fg_accept, 4, &pin);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "pin_reply error!\n");
    }
}

static void _linuxbt_gap_ssp_request_cb(RawAddress* remote_bd_addr,
                                     bt_bdname_t* bd_name, uint32_t cod,
                                     bt_ssp_variant_t pairing_variant,
                                     uint32_t pass_key)
{
    int ret = BT_STATUS_SUCCESS;
    uint8_t fg_accept = 1;

    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()", __FUNCTION__);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, remote_bd_addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, bd_name);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GAP, g_bt_interface);

    if (remote_bd_addr)
    {
        RawAddress *btaddr = remote_bd_addr;

        BT_DBG_INFO(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr->address[0], btaddr->address[1], btaddr->address[2],
                      btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return;
    }
    if (bd_name)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "BDNAME = %s", bd_name->name);
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP, "cod = 0x%08X, pairing_variant = %d, passkey = %d.", cod, pairing_variant, pass_key);
    if (g_bt_cb_func && g_bt_cb_func->passkey_cb)
    {
        g_bt_cb_func->passkey_cb(pass_key, (UINT8 *)&fg_accept, remote_bd_addr->address, bd_name->name, cod);
    }
    ret = g_bt_interface->ssp_reply(remote_bd_addr, pairing_variant, fg_accept, pass_key);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "ssp_reply error!\n");
    }
}

static void _linuxbt_gap_state_changed_cb(bt_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s() state: %ld", __FUNCTION__, (long)state);

    switch (state)
    {
    case BT_STATE_OFF:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT STATE OFF");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_EVT);
        msg.data.gap_evt.state = GAP_STATE_OFF;

        linuxbt_send_msg(&msg);
        break;

    case BT_STATE_ON:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT STATE ON");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_EVT);
        msg.data.gap_evt.state = GAP_STATE_ON;
        linuxbt_send_msg(&msg);
        break;

    case BT_STATE_RESET:
        BT_DBG_WARNING(BT_DEBUG_GAP, "BT STATE RESET");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_EVT);
        msg.data.gap_evt.state = GAP_STATE_RESET;
        linuxbt_send_msg(&msg);
        break;

    default:
        break;
    }
}

static void _linuxbt_gap_discovery_state_changed_cb(bt_discovery_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " state: %ld", (long)state);

    switch (state)
    {
    case BT_DISCOVERY_STOPPED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT Search Device Stop.");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_EVT);
        msg.data.gap_evt.state = GAP_STATE_DISCOVERY_STOPED;
        linuxbt_send_msg(&msg);
        break;

    case BT_DISCOVERY_STARTED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT Search Device Start...");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(BTMW_GAP_EVT);
        msg.data.gap_evt.state = GAP_STATE_DISCOVERY_STARTED;
        linuxbt_send_msg(&msg);
        break;
    default:
        break;
    }
}

static void _linuxbt_gap_bond_state_changed_cb(bt_status_t status,
                                            RawAddress* remote_bd_addr,
                                            bt_bond_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "status=(%s)%d, state =(%s)%d",
        linuxbt_get_bt_status_str(status), status,
        _linuxbt_gap_get_bond_state_str(state), state);

    if (BT_STATUS_SUCCESS == status)
    {
        switch (state)
        {
        case BT_BOND_STATE_NONE:
            msg.data.gap_evt.state = GAP_STATE_UNPAIR_SUCCESS;
            BT_DBG_NOTICE(BT_DEBUG_GAP, "state is unpair success.");
            break;
        case BT_BOND_STATE_BONDING:
            msg.data.gap_evt.state = GAP_STATE_BONDING;
            BT_DBG_NOTICE(BT_DEBUG_GAP, "state is bonding.");
            break;
        case BT_BOND_STATE_BONDED:
            msg.data.gap_evt.state = GAP_STATE_BONDED;
            BT_DBG_NOTICE(BT_DEBUG_GAP, "state is bonded.");
            break;
        default:
            break;
        }
    }
    else
    {
        if (BT_BOND_STATE_NONE == state)
        {
            msg.data.gap_evt.state = GAP_STATE_NO_BOND;
            BT_DBG_NOTICE(BT_DEBUG_GAP, "state is no bond(%s).",
                linuxbt_get_bt_status_str(status));
        }
    }

    if (remote_bd_addr)
    {
        RawAddress *btaddr = remote_bd_addr;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                          btaddr->address[0], btaddr->address[1], btaddr->address[2],
                          btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        linuxbt_btaddr_htos(btaddr, msg.data.gap_evt.bd_addr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }

    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(BTMW_GAP_EVT);
    linuxbt_send_msg(&msg);

}

static void _linuxbt_gap_acl_state_changed_cb(bt_status_t status,
                                           RawAddress* remote_bd_addr,
                                           bt_acl_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "status=(%s)%d, state = %d",
        linuxbt_get_bt_status_str(status), status, state);

    switch (status)
    {
    case BT_STATUS_SUCCESS:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is successful(%ld), ", (long)status);
        break;
    default:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is failed(%ld), ", (long)status);
        break;
    }
    //bt_gap_state.status = status;

    switch (state)
    {
    case BT_ACL_STATE_CONNECTED:
        msg.data.gap_evt.state = GAP_STATE_ACL_CONNECTED;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "acl is connected.");

        break;
    case BT_ACL_STATE_DISCONNECTED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "acl is disconnected.");
        msg.data.gap_evt.state = GAP_STATE_ACL_DISCONNECTED;
        break;
    default:
        break;
    }

    if (remote_bd_addr)
    {
        RawAddress *btaddr = remote_bd_addr;
        linuxbt_btaddr_htos(btaddr, msg.data.gap_evt.bd_addr);
        BT_DBG_NORMAL(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                          btaddr->address[0], btaddr->address[1], btaddr->address[2],
                          btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }

    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(BTMW_GAP_EVT);
    linuxbt_send_msg(&msg);

}

static void _linuxbt_gap_acl_state_changed_ex_callback(RawAddress* remote_bd_addr,
                                                bt_acl_state_t state,
                                                uint8_t reason,
                                                uint8_t link_type)
{
    tBTMW_MSG msg = {0};
    RawAddress *btaddr = NULL;

    if (remote_bd_addr)
    {
        btaddr = remote_bd_addr;
        linuxbt_btaddr_htos(btaddr, msg.data.gap_evt.bd_addr);
        BT_DBG_NORMAL(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                    btaddr->address[0], btaddr->address[1], btaddr->address[2],
                    btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }

    switch (state)
    {
        case BT_ACL_STATE_CONNECTED:
            msg.data.gap_evt.state = GAP_STATE_ACL_CONNECTED;
            BT_DBG_NORMAL(BT_DEBUG_GAP, "acl state = %ld link_type = %d",
                        (unsigned long)state, link_type);

            break;
        case BT_ACL_STATE_DISCONNECTED:
            msg.data.gap_evt.state = GAP_STATE_ACL_DISCONNECTED;
            BT_DBG_NORMAL(BT_DEBUG_GAP, "acl state = %ld reason = %d link_type = %d",
                        (unsigned long)state, reason, link_type);
            break;
        default:
            BT_DBG_WARNING(BT_DEBUG_GAP, "invalid acl state = %ld", (unsigned long)state);
            break;
    }

    msg.data.gap_evt.reason = reason ;
    msg.data.gap_evt.link_type = link_type ;
    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(BTMW_GAP_EVT);
    linuxbt_send_msg(&msg);
}

static void _linuxbt_gap_get_rssi_cb(bt_status_t status, RawAddress* remote_bd_addr, int rssi_value)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()  %ld ", __FUNCTION__, (long)(rssi_value));
    if (g_bt_cb_func && g_bt_cb_func->get_rssi_cb)
    {
        g_bt_cb_func->get_rssi_cb(rssi_value);
    }
}

static void _linuxbt_gap_ext_feature_support_callback(bt_local_ext_feature_support_t features)
{
  tBTMW_MSG msg = {0};
  BT_DBG_NORMAL(BT_DEBUG_GAP, "ext_feature_support_callback");

  BTMW_EXT_FEATURES ext_features;
  ext_features.le_audio_supported = features.le_audio_supported;
  msg.data.device_info.device.ext_features = ext_features;
  msg.data.device_info.device.ext_features_updated = TRUE;
  msg.data.device_info.device_kind = BT_DEVICE_LOCAL;
  msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
  msg.hdr.len = sizeof(BTMW_GAP_DEVICE_INFO);
  linuxbt_send_msg(&msg);
}

BT_ERR_STATUS_T linuxbt_gap_init(BT_GAP_CB_FUNC *func, VOID *stack_handle)
{
    FUNC_ENTRY;

    int ret = BT_STATUS_SUCCESS;

    if (stack_handle == NULL)
    {
        assert(stack_handle != NULL);
        return BT_ERR_STATUS_FAIL;
    }

    g_bt_interface = (bt_interface_t *)dlsym(stack_handle, "bluetoothInterface");
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get Bluetooth interface");
        return BT_ERR_STATUS_FAIL;
    }
    /*
        1. This will register callback function for OS callouts, and set is_native value as false in wakelock.c.
            And is_native will affact the behvaior of the function alarm_set which will launch a timer.
            When is_native is true, the "/sys/power/wake_lock" and  "/sys/power/wake_unlock" will be used in the execution of alarm_set.
            Once the set_os_callouts not invoked and "/sys/power/wake_lock" or "/sys/power/wake_unlock" not available, alarm_set will fail to launch a timer,
            this will result to no sound come out issue when in A2DP (both Sink & Source) application.
        2. the callback function do nothing.

        3. Very important note: this shouldn't be marked when poring to other project/platform.
      */
    g_bt_interface->set_os_callouts(&g_callouts);

    ret = g_bt_interface->init(&g_bt_callbacks, false, false, false);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "gap init error!");
    }

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    linuxbt_gap_ex_interface = (btgap_ex_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_GAP_EX_ID);
    if (NULL == linuxbt_gap_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get Bluetooth extended interface");
        return BT_ERR_STATUS_FAIL;
    }

    ret = linuxbt_gap_ex_interface->init(&linuxbt_gap_ex_callbacks);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "gap init error!");
    }
#endif

    g_bt_cb_func = func;
    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_deinit(void)
{
    g_bt_cb_func = NULL;
    if (NULL != g_bt_interface)
    {
        g_bt_interface->cleanup();
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }

    return BT_SUCCESS;
}

BT_ERR_STATUS_T linuxbt_gap_enable(void)
{
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    ret = g_bt_interface->enable();

    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_disable(VOID)
{
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    ret = g_bt_interface->disable();

    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_start_discovery(VOID)
{
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    ret = g_bt_interface->start_discovery();

    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_cancel_discovery(VOID)
{
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    ret = g_bt_interface->cancel_discovery();

    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_create_bond(CHAR *pbt_addr, INT32 transport)
{
    FUNC_ENTRY;
    int ret = BT_STATUS_SUCCESS;
    RawAddress addr;

    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "addr=%s", pbt_addr);
    ret = g_bt_interface->create_bond(&addr, transport);
    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_remove_bond(CHAR *pbt_addr)
{
    int ret = BT_STATUS_SUCCESS;
    RawAddress addr;

    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "addr=%s", pbt_addr);
    ret = g_bt_interface->remove_bond(&addr);
    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_cancel_bond(CHAR *pbt_addr)
{
    int ret = BT_STATUS_SUCCESS;
    RawAddress addr;

    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "addr=%s", pbt_addr);
    ret = g_bt_interface->cancel_bond(&addr);
    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_set_scan_mode(BT_GAP_SCAN_MODE mode)
{
    bt_property_t property;
    bt_property_t *property_p;
    bt_scan_mode_t scan_mode = BT_SCAN_MODE_NONE;
    int ret = BT_STATUS_SUCCESS;

    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "mode=%d", mode);

    memset(&property, 0, sizeof(bt_property_t));
    scan_mode = (bt_scan_mode_t)mode;

    property_p = &property;
    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    ret = g_bt_interface->set_adapter_property(property_p);

    return linuxbt_return_value_convert(ret);
}

BT_ERR_STATUS_T linuxbt_gap_set_local_name(CHAR *pname)
{
    bt_property_t property;
    bt_property_t *property_p;
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, pname);
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "pname=%s", pname);

    memset(&property, 0, sizeof(bt_property_t));

    property_p = &property;

    property_p->type = BT_PROPERTY_BDNAME;
    property_p->len = strlen(pname);
    property_p->val = pname;

    ret = g_bt_interface->set_adapter_property(property_p);
    return linuxbt_return_value_convert(ret);
}


BT_ERR_STATUS_T linuxbt_gap_get_adapter_properties(VOID)
{
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    ret = g_bt_interface->get_adapter_properties();
    return linuxbt_return_value_convert(ret);
}

const void *linuxbt_gap_get_profile_interface(const char *profile_id)
{
    if (NULL != g_bt_interface)
    {
        return g_bt_interface->get_profile_interface(profile_id);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return NULL;
    }
}

BT_ERR_STATUS_T linuxbt_gap_interop_database_clear()
{
    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "");

    g_bt_interface->interop_database_clear();

    return BT_SUCCESS;
}

BT_ERR_STATUS_T linuxbt_gap_interop_database_add(UINT16 feature, char *pbt_addr, size_t len)
{
    RawAddress addr;

    BT_CHECK_POINTER(BT_DEBUG_GAP, g_bt_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "addr=%s", pbt_addr);
    g_bt_interface->interop_database_add(feature, &addr, len);
    return BT_SUCCESS;
}

BT_ERR_STATUS_T linuxbt_gap_get_rssi(CHAR *pbt_addr)
{
#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    int ret = BT_STATUS_SUCCESS;
    RawAddress addr;

    BT_CHECK_POINTER(BT_DEBUG_GAP, linuxbt_gap_ex_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "addr=%s", pbt_addr);
    ret = linuxbt_gap_ex_interface->get_rssi(&addr);
    return linuxbt_return_value_convert(ret);
#else
    return BT_ERR_STATUS_UNSUPPORTED;
#endif
}

BT_ERR_STATUS_T linuxbt_gap_send_hci(CHAR *ptr)
{
#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    int i = 0;
    uint8_t   rpt_size = 0;
    uint8_t   hex_bytes_filled;
    uint8_t hex_buf[200] = {0};
    uint16_t   hex_len = 0;
    int ret = BT_STATUS_SUCCESS;
    BT_CHECK_POINTER(BT_DEBUG_GAP, ptr);
    BT_CHECK_POINTER(BT_DEBUG_GAP, linuxbt_gap_ex_interface);
    BT_MW_FUNC_ENTER(BT_DEBUG_GAP, "hci=%s", ptr);

    rpt_size = strlen(ptr);
    hex_len = (strlen(ptr) + 1) / 2;

    BT_DBG_INFO(BT_DEBUG_GAP, "rpt_size=%ld, hex_len=%ld", (unsigned long)rpt_size, (unsigned long)hex_len);
    hex_bytes_filled = ascii_2_hex(ptr, hex_len, hex_buf);
    BT_DBG_INFO(BT_DEBUG_GAP, "hex_bytes_filled=%ld", (unsigned long)hex_bytes_filled);
    for (i=0;i<hex_len;i++)
    {
        BT_DBG_NOTICE(BT_DEBUG_GAP, "hex values= %02X",hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        ret = linuxbt_gap_ex_interface->send_hci((uint8_t*)hex_buf, hex_bytes_filled);
        BT_DBG_INFO(BT_DEBUG_GAP, "send_hci");
        return linuxbt_return_value_convert(ret);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "hex_bytes_filled <= 0");
        return BT_ERR_STATUS_PARM_INVALID;
    }
#else
    return BT_ERR_STATUS_UNSUPPORTED;
#endif
}

BT_ERR_STATUS_T linuxbt_gap_set_bt_wifi_ratio(UINT8 bt_ratio, UINT8 wifi_ratio)
{
#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    uint8_t cmd[5] = {0};

    cmd[0] = 0xf1;
    cmd[1] = 0xfc;
    cmd[2] = 0x02;
    cmd[3] = bt_ratio;
    cmd[4] = wifi_ratio;
    linuxbt_gap_ex_interface->send_hci(cmd, sizeof(cmd));
    return BT_SUCCESS;
#else
    return BT_ERR_STATUS_UNSUPPORTED;
#endif
}
