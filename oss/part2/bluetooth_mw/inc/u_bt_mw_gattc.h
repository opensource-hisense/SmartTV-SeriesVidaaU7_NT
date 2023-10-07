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

#ifndef _U_BT_MW_GATTC_H_
#define _U_BT_MW_GATTC_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_bt_mw_common.h"
#include "u_bt_mw_gatt.h"


typedef enum
{
    BT_GATTC_WRITE_TYPE_CMD=1,        /* write cmd no response */
    BT_GATTC_WRITE_TYPE_REQ,        /* write request */
    BT_GATTC_WRITE_TYPE_PREPARE,    /* prepare write */
} BT_GATTC_WRITE_TYPE;

typedef enum
{
    BT_GATTC_AUTH_REQ_NONE,        /* no encryption */
    BT_GATTC_AUTH_REQ_NO_MITM,     /* unauthenticated encryption */
    BT_GATTC_AUTH_REQ_MITM,        /* authenticated encryption */
} BT_GATTC_AUTH_REQ;

typedef enum
{
    BT_GATTC_DEVICE_TYPE_BREDR=1,
    BT_GATTC_DEVICE_TYPE_BLE,
    BT_GATTC_DEVICE_TYPE_DUMO,
} BT_GATTC_DEVICE_TYPE;

/****************************** API parameters ********************************/
typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    BOOL is_direct;
    GATT_TRANSPORT_TYPE transport;
    BOOL opportunistic;
    INT32 initiating_phys;  /* bit0-1M, bit1-2M, bit2-Coded */
} BT_GATTC_CONNECT_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_DISCONNECT_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_REFRESH_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    CHAR service_uuid[BT_GATT_MAX_UUID_LEN];
} BT_GATTC_DISCOVER_BY_UUID_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT16 handle;
    BT_GATTC_AUTH_REQ auth_req;
} BT_GATTC_READ_CHAR_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    CHAR char_uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 s_handle;
    UINT16 e_handle;
    BT_GATTC_AUTH_REQ auth_req;
} BT_GATTC_READ_BY_UUID_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT16 handle;
    BT_GATTC_WRITE_TYPE write_type;
    BT_GATTC_AUTH_REQ auth_req;
    INT32 value_len;
    CHAR value[BT_GATT_MAX_VALUE_LEN];
} BT_GATTC_WRITE_CHAR_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT16 handle;
    BT_GATTC_AUTH_REQ auth_req;
} BT_GATTC_READ_DESC_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT16 handle;
    BT_GATTC_AUTH_REQ auth_req;
    INT32 value_len;
    CHAR value[BT_GATT_MAX_VALUE_LEN];
} BT_GATTC_WRITE_DESC_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    BOOL execute;   /* 0-cancel execute, 1-execute */
} BT_GATTC_EXEC_WRITE_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT16 handle;
    INT32 registered;
} BT_GATTC_REG_NOTIF_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_READ_RSSI_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_GET_DEV_TYPE_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT32 mtu; /* BT_GATT_MIN_MTU_SIZE ~ BT_GATT_MAX_MTU_SIZE */
} BT_GATTC_CHG_MTU_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    INT32 min_interval; /* unit: 1.25ms, 7.5ms ~ 4000ms */
    INT32 max_interval; /* unit: 1.25ms, 7.5ms ~ 4000ms */
    INT32 latency;      /* unit: connection event, 0~499 */
    INT32 timeout;      /* unit: 10ms, 100~32000ms */
    UINT16 min_ce_len;  /* unit: 0.625ms, 0~0xFFFF */
    UINT16 max_ce_len;  /* unit: 0.625ms, 0~0xFFFF */
} BT_GATTC_CONN_UPDATE_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
    UINT8 tx_phy; /* bit0-1M, bit1-2M, bit2-Coded */
    UINT8 rx_phy; /* bit0-1M, bit1-2M, bit2-Coded */
    GATT_PHY_OPT_CODED phy_options;
} BT_GATTC_PHY_SET_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_PHY_READ_PARAM;

typedef struct
{
    UINT32 client_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTC_GET_GATT_DB_PARAM;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    CHAR value[BT_GATT_MAX_VALUE_LEN];
    UINT32              u1;
    UINT32              u2;
    UINT32              u3;
    UINT32              u4;
    UINT32              u5;
} BT_GATTC_TEST_COMMAND_PARAM;

/****************************** callback parameters ********************************/
typedef struct
{
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    CHAR client_name[BT_GATT_MAX_NAME_LEN];
} BT_GATTC_EVENT_REGISTER_DATA;

typedef struct
{
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    CHAR addr[MAX_BDADDR_LEN];
    INT32 connected;            /* 0-disconnected, 1-connected */
} BT_GATTC_EVENT_CONNECTION_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    INT32 registered;
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    UINT16 handle;
} BT_GATTC_EVENT_REG_NOTIF_DATA;

typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    UINT16  handle;
    UINT8   value[BT_GATT_MAX_VALUE_LEN];
    UINT16  value_len;
    UINT8   is_notify;
} BT_GATTC_EVENT_NOTIF_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    UINT16 handle;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 value_len;
} BT_GATTC_EVENT_READ_CHAR_RSP_DATA;


/** Parameters for GATT write operations */
typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    UINT16 handle;
} BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
} BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA;

typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    UINT16 handle;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 value_len;
} BT_GATTC_EVENT_READ_DESC_RSP_DATA;


typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    UINT16 handle;
} BT_GATTC_EVENT_WRITE_DESC_RSP_DATA;


typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    INT32 rssi; /* -127 to 20, 127 */
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
}BT_GATTC_EVENT_READ_RSSI_DATA;


typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    BT_GATT_STATUS status;  /* BT_GATT_STATUS */
    INT32 mtu;
} BT_GATTC_EVENT_MTU_CHG_DATA;

/* this db may be fragment by RPC capability, it may be a complete service
 * or it may be part of service, it will not include more than 1 service
 * attribute list
 */
typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    UINT8 is_first; /* 1 - is the first fragment */
    UINT32 attr_cnt;
    BT_GATT_ATTR attrs[BT_GATT_MAX_ATTR_CNT];
}BT_GATTC_EVENT_GET_GATT_DB_DATA;

typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
}BT_GATTC_EVENT_DISCOVER_COMPL_DATA;


typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    UINT8 tx_phy;   /* GATT_PHY_SETTING */
    UINT8 rx_phy;   /* GATT_PHY_SETTING */
    UINT8 status;   /* BT_GATT_STATUS */
} BT_GATTC_EVENT_PHY_UPDATE_DATA;

typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    UINT8 tx_phy;   /* GATT_PHY_SETTING */
    UINT8 rx_phy;   /* GATT_PHY_SETTING */
    UINT8 status;   /* BT_GATT_STATUS */
} BT_GATTC_EVENT_READ_PHY_DATA;

typedef struct
{
    CHAR  addr[MAX_BDADDR_LEN];
    UINT16 interval;    /* unit: 1.25ms, 7.5ms ~ 4000ms */
    UINT16 latency;     /* unit: connection event, 0~499 */
    UINT16 timeout;     /* unit: 10ms, 100~32000ms */
    UINT8 status;       /* BT_GATT_STATUS */
} BT_GATTC_EVENT_CONN_UPDATE_DATA;


typedef enum
{
    BT_GATTC_EVENT_REGISTER,          /* GATTC register                 */
    BT_GATTC_EVENT_CONNECTION,        /* GATTC connection               */
    BT_GATTC_EVENT_NOTIFY,            /* GATTC notify                   */
    BT_GATTC_EVENT_READ_CHAR_RSP,     /* GATTC read char                */
    BT_GATTC_EVENT_WRITE_CHAR_RSP,    /* GATTC write char               */
    BT_GATTC_EVENT_EXEC_WRITE_RSP,    /* GATTC execute writer           */
    BT_GATTC_EVENT_READ_DESC_RSP,     /* GATTC read descr               */
    BT_GATTC_EVENT_WRITE_DESC_RSP,    /* GATTC write descr              */
    BT_GATTC_EVENT_READ_RSSI_RSP,     /* GATTC read remote rssi         */
    BT_GATTC_EVENT_MTU_CHANGE,        /* GATTC configure mtu            */
    BT_GATTC_EVENT_PHY_UPDATE,        /* GATTC phy updated              */
    BT_GATTC_EVENT_CONN_UPDATE,       /* GATTC conn updated             */
    BT_GATTC_EVENT_PHY_READ,          /* GATTC conn updated             */
    BT_GATTC_EVENT_GET_GATT_DB,       /* GATTC get gatt db              */
    BT_GATTC_EVENT_DISCOVER_COMPL,    /* GATTC service discover done    */
    BT_GATTC_EVENT_MAX,               /* GATTC BT OFF                   */
} BT_GATTC_EVENT;

typedef struct
{
    BT_GATTC_EVENT event;        /* event id */
    UINT32 client_if;
    union
    {
        BT_GATTC_EVENT_REGISTER_DATA              register_data;
        BT_GATTC_EVENT_CONNECTION_DATA            connection_data;
        BT_GATTC_EVENT_REG_NOTIF_DATA             reg_notif_data;
        BT_GATTC_EVENT_NOTIF_DATA                 notif_data;
        BT_GATTC_EVENT_READ_CHAR_RSP_DATA         read_char_rsp_data;
        BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA        write_char_rsp_data;
        BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA        exec_write_rsp_data;
        BT_GATTC_EVENT_READ_DESC_RSP_DATA         read_desc_rsp_data;
        BT_GATTC_EVENT_WRITE_DESC_RSP_DATA        write_desc_rsp_data;
        BT_GATTC_EVENT_READ_RSSI_DATA             read_rssi_data;
        BT_GATTC_EVENT_MTU_CHG_DATA               mtu_chg_data;
        BT_GATTC_EVENT_PHY_UPDATE_DATA            phy_upd_data;
        BT_GATTC_EVENT_READ_PHY_DATA              phy_read_data;
        BT_GATTC_EVENT_CONN_UPDATE_DATA           conn_upd_data;
        BT_GATTC_EVENT_GET_GATT_DB_DATA           get_gatt_db_data;
        BT_GATTC_EVENT_DISCOVER_COMPL_DATA        discover_cmpl_data;
    } data;/* event data */
} BT_GATTC_EVENT_PARAM;

typedef void (*BT_GATTC_EVENT_HANDLE_CB)(BT_GATTC_EVENT_PARAM *param);

#endif /*  _U_BT_MW_GATTC_H_ */

