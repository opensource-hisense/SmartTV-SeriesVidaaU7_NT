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


#ifndef _U_BT_MW_BLE_SCANNER_H_
#define _U_BT_MW_BLE_SCANNER_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_bt_mw_common.h"


#define BT_BLE_SCANNER_MAX_SCAN_FILT_NUM 12
#define BT_BLE_SCANNER_MAX_DATA_LEN 29
#define BT_BLE_SCANNER_MAX_FILT_NAME_LEN 30
#define BT_BLE_SCANNER_MAX_UUID_LEN 37
#define BT_BLE_SCANNER_MAX_ATTR_LE 600
#define BT_BLE_SCANNER_MAX_REG_NUM 64

#define SCAN_MODE_REGULAR_LOW_POWER_WINDOW_MS 819
#define SCAN_MODE_REGULAR_LOW_POWER_INTERVAL_MS 8192
#define SCAN_MODE_REGULAR_BALANCED_WINDOW_MS 1638
#define SCAN_MODE_REGULAR_BALANCED_INTERVAL_MS 6553
#define SCAN_MODE_REGULAR_LOW_LATENCY_WINDOW_MS 6553
#define SCAN_MODE_REGULAR_LOW_LATENCY_INTERVAL_MS 6553

#define SCAN_MODE_BATCH_LOW_POWER_WINDOW_MS 2400
#define SCAN_MODE_BATCH_LOW_POWER_INTERVAL_MS 16000
#define SCAN_MODE_BATCH_BALANCED_WINDOW_MS 2400
#define SCAN_MODE_BATCH_BALANCED_INTERVAL_MS 12000
#define SCAN_MODE_BATCH_LOW_LATENCY_WINDOW_MS 2400
#define SCAN_MODE_BATCH_LOW_LATENCY_INTERVAL_MS 8000

#define  DATA_TYPE_FLAGS 0x01
#define  DATA_TYPE_SERVICE_UUIDS_16_BIT_PARTIAL 0x02
#define  DATA_TYPE_SERVICE_UUIDS_16_BIT_COMPLETE 0x03
#define  DATA_TYPE_SERVICE_UUIDS_32_BIT_PARTIAL 0x04
#define  DATA_TYPE_SERVICE_UUIDS_32_BIT_COMPLETE 0x05
#define  DATA_TYPE_SERVICE_UUIDS_128_BIT_PARTIAL 0x06
#define  DATA_TYPE_SERVICE_UUIDS_128_BIT_COMPLETE 0x07
#define  DATA_TYPE_LOCAL_NAME_SHORT 0x08
#define  DATA_TYPE_LOCAL_NAME_COMPLETE 0x09
#define  DATA_TYPE_TX_POWER_LEVEL 0x0A
#define  DATA_TYPE_SERVICE_DATA_16_BIT 0x16
#define  DATA_TYPE_SERVICE_DATA_32_BIT 0x20
#define  DATA_TYPE_SERVICE_DATA_128_BIT 0x21
#define  DATA_TYPE_SERVICE_SOLICITATION_UUIDS_16_BIT 0x14
#define  DATA_TYPE_SERVICE_SOLICITATION_UUIDS_32_BIT 0x1F
#define  DATA_TYPE_SERVICE_SOLICITATION_UUIDS_128_BIT 0x15
#define  DATA_TYPE_MANUFACTURER_SPECIFIC_DATA 0xFF

#define  LINUX_BLE_SCANNER_MATCH_FILTER TRUE
#define  BLE_SCANNER_MATCH_FILTER_DATA_TYPE 10


enum {
  APCF_ACTION_ADD = 0x00,
  APCF_ACTION_DELETE,
  APCF_ACTION_CLEAR
};

enum {
  CONFIG_ADD_FILTE_DATA_FAIL = 0x00,
  CONFIG_ADD_FILTE_DATA_CONTINUE,
  CONFIG_ADD_FILTE_DATA_DONE
};


typedef struct
{
    UINT8 status;
    CHAR app_uuid[BT_BLE_SCANNER_MAX_UUID_LEN];
} BT_BLE_SCANNER_REG_EVT_DATA;

typedef struct
{
    UINT16 event_type;
    UINT8 addr_type;
    CHAR bd_addr[MAX_BDADDR_LEN];
    UINT8 primary_phy;
    UINT8 secondary_phy;
    UINT8 advertising_sid;
    INT8 tx_power;
    INT8 rssi;
    UINT16 periodic_adv_int;
    UINT32 adv_data_len;
    UINT8 adv_data[BT_BLE_SCANNER_MAX_ATTR_LE];
} BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA;

typedef struct
{
    UINT8 avbl_space;
    UINT8 action;
    UINT8 status;
} BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA;

typedef struct
{
    UINT8 filt_type;
    UINT8 avbl_space;
    UINT8 action;
    UINT8 status;
} BT_BLE_SCANNER_FILT_ADD_EVT_DATA;

typedef struct
{
    UINT8 filt_type;
    UINT8 avbl_space;
    UINT8 action;
    UINT8 status;
} BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA;


typedef struct
{
    UINT8 action;
    UINT8 status;
} BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA;

typedef struct
{
    UINT8 status;
} BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA;

typedef struct
{
    UINT8 status;
} BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA;

typedef struct
{
    UINT8 status;
} BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA;

typedef struct
{
    UINT8 status;
} BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA;

typedef struct
{
    INT32 status;
    INT32 report_format;
    INT32 num_records;
    INT32 data_len;
    UINT8 data[BT_BLE_SCANNER_MAX_ATTR_LE];
} BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA;

typedef struct
{
    UINT8 filt_index;
    UINT8 advertiser_state;
    UINT8 advertiser_info_present;
    UINT8 addr_type;
    UINT8 tx_power;
    INT8 rssi_value;
    UINT16 time_stamp;
    CHAR bd_addr[MAX_BDADDR_LEN];
    UINT8 adv_pkt_len;
    UINT8 p_adv_pkt_data[BT_BLE_SCANNER_MAX_ATTR_LE];
    UINT8 scan_rsp_len;
    UINT8 p_scan_rsp_data[BT_BLE_SCANNER_MAX_ATTR_LE];
} BT_BLE_SCANNER_TRACK_ADV_EVT_DATA;


typedef struct
{
    UINT8 scanner_id;
}BT_BLE_SCANNER_STOP_SCAN_PARAM;

typedef struct
{
    UINT8 scanner_id;
}BT_BLE_SCANNER_UNREG_PARAM;


enum
{
    SCAN_MODE_LOW_POWER = 0,
    SCAN_MODE_BALANCED,
    SCAN_MODE_LOW_LATENCY,
};
typedef UINT8 BT_BLE_SCANNER_SIMPLE_SCAN_MODE;

enum
{
    BATCH_SCAN_RESULT_TYPE_FULL = 0,
    BATCH_SCAN_RESULT_TYPE_TRUNCATED
};
typedef UINT8 BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_RESULT;

enum
{
    MATCH_NUM_ONE_ADVERTISEMENT = 1,
    MATCH_NUM_FEW_ADVERTISEMENT,
    MATCH_NUM_MAX_ADVERTISEMENT,
};
typedef UINT8 BT_BLE_SCANNER_SIMPLE_MATCH_PER_FILT_NUM;

enum
{
    MATCH_MODE_AGGRESSIVE = 1,
    MATCH_MODE_STICKY
};
typedef UINT8 BT_BLE_SCANNER_SIMPLE_FILT_MATCH_MODE;

enum
{
    FILT_DELY_MODE_IMMEDIATE = 0,
    FILT_DELY_MODE_ON_FOUND,
    FILT_DELY_MODE_BATCHED
};
typedef UINT8 BT_BLE_SCANNER_SIMPLE_FILT_DELY_MODE;


typedef struct
{
    BT_BLE_SCANNER_SIMPLE_FILT_DELY_MODE dely_mode;
    BT_BLE_SCANNER_SIMPLE_MATCH_PER_FILT_NUM num_of_matches;
    BT_BLE_SCANNER_SIMPLE_FILT_MATCH_MODE filt_match_mode;
} BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM;

enum
{
    SCAN_FILT_PUBLIC_TYPE = 0,
    SCAN_FILT_RANDOM_TYPE,
    SCAN_FILT_ALL_TYPE,
};
typedef UINT8 BT_BLE_SCANNER_SCAN_FILT_ADDR_TYPE;


typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BT_BLE_SCANNER_SCAN_FILT_ADDR_TYPE addr_type;
    CHAR srvc_uuid[BT_BLE_SCANNER_MAX_UUID_LEN];
    CHAR srvc_uuid_mask[BT_BLE_SCANNER_MAX_UUID_LEN];
    CHAR srvc_sol_uuid[BT_BLE_SCANNER_MAX_UUID_LEN];
    CHAR srvc_sol_uuid_mask[BT_BLE_SCANNER_MAX_UUID_LEN];
    CHAR local_name[BT_BLE_SCANNER_MAX_FILT_NAME_LEN];
    UINT16 company;
    UINT16 company_mask;
    UINT8 manu_data[BT_BLE_SCANNER_MAX_DATA_LEN];
    UINT8 manu_data_mask[BT_BLE_SCANNER_MAX_DATA_LEN];
    UINT16 manu_data_len; /* manufacturer data/mask should same length */
    UINT8 srvc_data[BT_BLE_SCANNER_MAX_DATA_LEN];
    UINT8 srvc_data_mask[BT_BLE_SCANNER_MAX_DATA_LEN];
    UINT16 srvc_data_len; /* service data/mask should same length */
} BT_BLE_SCANNER_SCAN_FILT_DATA;


typedef struct
{
    BT_BLE_SCANNER_SIMPLE_SCAN_MODE scan_mode;
} BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM;

typedef struct
{
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM regular_scan_param;
    BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM scan_filt_param;
    UINT8 scan_filt_num;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
} BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM;

typedef struct
{
    BT_BLE_SCANNER_SIMPLE_SCAN_MODE scan_mode;
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_RESULT scan_result_type;
} BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM;

typedef struct
{
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM batch_scan_param;
    UINT8 scan_filt_num;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
} BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM;

enum
{
    REGULAR_SCAN = 0,
    REGULAR_SCAN_WITH_FILT,
    BATCH_SCAN,
    BATCH_SCAN_WITH_FILT,
};
typedef UINT8 BT_BLE_SCANNER_SCAN_TYPE;

typedef struct
{
    UINT8 scanner_id;
    BT_BLE_SCANNER_SCAN_TYPE scan_type;
    union
    {
        BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM regular_scan_param;
        BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM regular_scan_filt_param;
        BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM batch_scan_param;
        BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_WITH_FILT_PARAM batch_scan_filt_param;
    } scan_param;
} BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM;


typedef struct
{
    UINT16 feat_seln;
    UINT16 list_logic_type;
    UINT8 filt_logic_type;
    UINT8 rssi_high_thres;
    UINT8 rssi_low_thres;
    UINT8 dely_mode;
    UINT16 found_timeout;
    UINT16 lost_timeout;
    UINT8 found_timeout_cnt;
    UINT16 num_of_tracking_entries;
} BT_BLE_SCANNER_SCAN_FILT_PARAM;

enum
{
    SCAN_FILT_TYPE_ADDRESS = 0,
    SCAN_FILT_TYPE_SRVC_DATA_CHANGED = 1,
    SCAN_FILT_TYPE_SRVC_UUID = 2,
    SCAN_FILT_TYPE_SRVC_SOL_UUID = 3,
    SCAN_FILT_TYPE_LOCAL_NAME = 4,
    SCAN_FILT_TYPE_MANU_DATA = 5,
    SCAN_FILT_TYPE_SRVC_DATA = 6,
};
typedef UINT8 BT_BLE_SCANNER_SCAN_FILT_TYPE;

enum
{
    BATCH_SCAN_MODE_TRUNCATED = 1,
    BATCH_SCAN_MODE_FULL,
    BATCH_SCAN_MODE_FULL_AND_TRUNCATED
};
typedef UINT8 BT_BLE_SCANNER_BATCH_SCAN_MODE;

enum
{
    PUBLIC_DEVICE_ADDR = 0,
    RANDOM_DEVICE_ADDR
};
typedef UINT8 BT_BLE_SCANNER_BATCH_SCAN_OWN_ADDR_TYPE;

enum
{
    BATCH_SCAN_DISCARD_OLDEST_ADV = 0,
    BATCH_SCAN_DISCARD_ADV_WITH_WEAKEST_RSSI
};
typedef UINT8 BT_BLE_SCANNER_BATCH_SCAN_DISCARD_RULE;

enum
{
    BATCH_SCAN_READ_REPORT_MODE_TRUNCATED = 1,
    BATCH_SCAN_READ_REPORT_MODE_FULL
};
typedef UINT8 BT_BLE_SCANNER_BATCH_SCAN_READ_REPORT_MODE;

typedef struct
{
    INT32 scan_windows;
    INT32 scan_interval;
} BT_BLE_SCANNER_REGULAR_SCAN_PARAM;

typedef struct
{
    BT_BLE_SCANNER_REGULAR_SCAN_PARAM regular_scan_param;
    BT_BLE_SCANNER_SCAN_FILT_PARAM scan_filt_param[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
    UINT8 scan_filt_num;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
} BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM;

typedef struct
{
    UINT8 batch_scan_full_max;
    UINT8 batch_scan_trunc_max;
    UINT8 batch_scan_notify_threshold;
    BT_BLE_SCANNER_BATCH_SCAN_MODE batch_scan_mode;
    INT32 batch_scan_windows;
    INT32 batch_scan_interval;
    BT_BLE_SCANNER_BATCH_SCAN_OWN_ADDR_TYPE own_address_type;
    BT_BLE_SCANNER_BATCH_SCAN_DISCARD_RULE batch_scan_discard_rule;
    BT_BLE_SCANNER_BATCH_SCAN_READ_REPORT_MODE batch_scan_read_report_mode;
} BT_BLE_SCANNER_BATCH_SCAN_PARAM;

typedef struct
{
    BT_BLE_SCANNER_BATCH_SCAN_PARAM batch_scan_param;
    BT_BLE_SCANNER_SCAN_FILT_PARAM scan_filt_param[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
    UINT8 scan_filt_num;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
} BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM;

typedef struct
{
    UINT8 scanner_id;
    BT_BLE_SCANNER_SCAN_TYPE scan_type;
    union
    {
        BT_BLE_SCANNER_REGULAR_SCAN_PARAM regular_scan_param;
        BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM regular_scan_filt_param;
        BT_BLE_SCANNER_BATCH_SCAN_PARAM batch_scan_param;
        BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM batch_scan_filt_param;
    } scan_param;
} BT_BLE_SCANNER_START_SCAN_PARAM;


enum
{
    BT_BLE_SCANNER_EVT_REGISTER,            /* Ble Scanner register                 */
    BT_BLE_SCANNER_EVT_SCAN_RESULT,         /* Ble Scan result                      */
    BT_BLE_SCANNER_EVT_FILT_PARAM_SETUP,    /* Ble Scanner filt parameter setup     */
    BT_BLE_SCANNER_EVT_FILT_ADD,            /* Ble Scanner filt add                 */
    BT_BLE_SCANNER_EVT_FILT_CLEAR,          /* Ble Scanner filt clear               */
    BT_BLE_SCANNER_EVT_FILT_ENABLE,         /* Ble Scanner filt enable              */
    BT_BLE_SCANNER_EVT_SCAN_PARAM_SETUP,    /* Ble Scanner scan param setup         */
    BT_BLE_SCANNER_EVT_BATCHSCAN_CONFIG,    /* Ble Scanner batchscan config         */
    BT_BLE_SCANNER_EVT_BATCHSCAN_ENABLE,    /* Ble Scanner batchscan enable         */
    BT_BLE_SCANNER_EVT_BATCHSCAN_DISABLE,   /* Ble Scanner batchscan disable        */
    BT_BLE_SCANNER_EVT_BATCHSCAN_REPORT,    /* Ble Scanner batchscan reports        */
    BT_BLE_SCANNER_EVT_BATCHSCAN_THRESHOLD, /* Ble Scanner batchscan threshold      */
    BT_BLE_SCANNER_EVT_TRACK_ADV,           /* Ble Scanner track adv event          */
    BT_BLE_SCANNER_EVT_BT_OFF,              /* Ble Scanner bt off event          */
} ;
typedef UINT8 BT_BLE_SCANNER_CALLBACK_EVT;

typedef struct
{
    BT_BLE_SCANNER_CALLBACK_EVT event;        /* event id */
    UINT8 scanner_id;                         /* scanner id */
    union
    {
        BT_BLE_SCANNER_REG_EVT_DATA                   register_data;
        BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA           scan_result_data;
        BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA      filt_param_setup_data;
        BT_BLE_SCANNER_FILT_ADD_EVT_DATA              filt_add_data;
        BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA            filt_clear_data;
        BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA           filt_enable_data;
        BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA      scan_param_setup_data;
        BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA      batchscan_config_data;
        BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA      batchscan_enable_data;
        BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA     batchscan_disable_data;
        BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA      batchscan_report_data;
        BT_BLE_SCANNER_TRACK_ADV_EVT_DATA             track_adv_data;
    } data;/* event data */
} BT_BLE_SCANNER_CALLBACK_PARAM;

typedef struct
{
    UINT8 scanner_id;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
}BT_BLE_SCANNER_APP_FILTER_DATA;

typedef void (*BT_BLE_SCANNER_EVENT_HANDLE_CB)(BT_BLE_SCANNER_CALLBACK_PARAM *param);

#endif /*  _U_BT_MW_BLE_SCANNER_H_ */
