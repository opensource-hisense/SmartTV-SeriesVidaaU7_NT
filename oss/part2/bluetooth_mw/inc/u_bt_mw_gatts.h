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


#ifndef U_BT_MW_GATTS_H
#define U_BT_MW_GATTS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "u_bt_mw_common.h"
#include "u_bt_mw_gatt.h"

/* NAMING CONSTANT DECLARATIONS
 */

typedef enum
{
    BT_GATTS_EVENT_REGISTER,            /* GATTS register             */
    BT_GATTS_EVENT_CONNECTION,          /* GATTS connection           */
    BT_GATTS_EVENT_SERVICE_ADD,         /* GATTS service add          */
    BT_GATTS_EVENT_READ_REQ,            /* GATTS read req             */
    BT_GATTS_EVENT_WRITE_REQ,           /* GATTS write req            */
    BT_GATTS_EVENT_WRITE_EXE_REQ,       /* GATTS write execute req    */
    BT_GATTS_EVENT_IND_SENT,            /* GATTS indication sent      */
    BT_GATTS_EVENT_MTU_CHANGE,          /* GATTS mtu change           */
    BT_GATTS_EVENT_PHY_READ,            /* GATTS phy read             */
    BT_GATTS_EVENT_PHY_UPDATE,          /* GATTS phy update           */
    BT_GATTS_EVENT_CONN_UPDATE,         /* GATTS connection update    */
    BT_GATTS_EVENT_MAX,                 /* GATTS BT off               */
} BT_GATTS_EVENT;

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
    INT32 is_direct; /* 0-bg connection, 1-direct connect */
    GATT_TRANSPORT_TYPE transport;
} BT_GATTS_CONNECT_PARAM;

typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTS_DISCONNECT_PARAM;


typedef struct
{
    INT32 server_if;
    CHAR service_uuid[BT_GATT_MAX_UUID_LEN];
    BT_GATT_ATTR *attrs;
    UINT32 attr_cnt;
    UINT32 is_last; /* indicate it's the last add param for service */
} BT_GATTS_SERVICE_ADD_PARAM;

typedef struct
{
    INT32 server_if;
    INT32 service_handle;
} BT_GATTS_SERVICE_DEL_PARAM;

typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
    INT32 char_handle;
    INT32 need_confirm; /* 0-notification, 1-indication */
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT32 value_len;
} BT_GATTS_IND_PARAM;

typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
    INT32 status; /* BT_GATT_STATUS */
    INT32 handle;
    INT32 trans_id;
    INT32 offset;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT32 len;
} BT_GATTS_RESPONSE_PARAM;

typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
} BT_GATTS_PHY_READ_PARAM;


typedef struct
{
    INT32 server_if;
    CHAR addr[MAX_BDADDR_LEN];
    INT32 tx_phy; /* bit0-1M, bit1-2M, bit2-Coded */
    INT32 rx_phy; /* bit0-1M, bit1-2M, bit2-Coded */
    GATT_PHY_OPT_CODED phy_options;
} BT_GATTS_PHY_SET_PARAM;

/* when report a BT_GATTS_EVENT_REGISTER_DATA, app can get the register info.
 */
typedef struct
{
    BT_GATT_STATUS status;
    CHAR server_name[BT_GATT_MAX_NAME_LEN];
} BT_GATTS_EVENT_REGISTER_DATA;


/* when report a BT_GATTS_EVENT_CONNECTION, app can get the connection info.
 */
typedef struct
{
    INT32 connected;            /* 0-disconnected, 1-connected */
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
} BT_GATTS_EVENT_CONNECTION_DATA;

typedef struct
{
    CHAR service_uuid[BT_GATT_MAX_UUID_LEN];
    BT_GATT_ATTR attrs[BT_GATT_MAX_ATTR_CNT];
    UINT32 attr_cnt;
    INT32 status; /* BT_GATT_STATUS */
} BT_GATTS_EVENT_SERVICE_ADD_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 status; /* BT_GATT_STATUS */
} BT_GATTS_EVENT_IND_SENT_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 handle;
    INT32 trans_id;
    INT32 offset;
} BT_GATTS_EVENT_READ_REQ_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 handle;
    INT32 trans_id; /* transaction id, need keep same in response */
    INT32 offset;
    INT32 need_rsp; /* 0-write command, 1-write request */
    INT32 is_prep;  /* 0-write request, 1-prepare write request*/
    UINT32 value_len;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
} BT_GATTS_EVENT_WRITE_REQ_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 trans_id; /* transaction id, need keep same in response */
    INT32 exec_write; /* 0-cancel prepare write, 1-execute prepare write */
} BT_GATTS_EVENT_WRITE_EXE_REQ_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 mtu;
} BT_GATTS_EVENT_MTU_CHG_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 rx_phy;   /* GATT_PHY_SETTING */
    INT32 tx_phy;   /* GATT_PHY_SETTING */
    INT32 status;   /* BT_GATT_STATUS */
} BT_GATTS_EVENT_READ_PHY_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 rx_phy;   /* GATT_PHY_SETTING */
    INT32 tx_phy;   /* GATT_PHY_SETTING */
    INT32 status;   /* BT_GATT_STATUS */
} BT_GATTS_EVENT_PHY_UPDATE_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    INT32 interval; /* unit: 1.25ms, 7.5ms ~ 4000ms */
    INT32 latency;  /* unit: connection event, 0~499 */
    INT32 timeout;  /* unit: 10ms, 100~32000ms */
    INT32 status;   /* BT_GATT_STATUS */
} BT_GATTS_EVENT_CONN_UPDATE_DATA;


/* when report a BT_GATTS_EVENT_ to app, some event will report some data
 */
typedef union
{
    BT_GATTS_EVENT_REGISTER_DATA register_data;  /* register data */
    BT_GATTS_EVENT_CONNECTION_DATA connection_data;
    BT_GATTS_EVENT_SERVICE_ADD_DATA add_service_data;
    BT_GATTS_EVENT_IND_SENT_DATA ind_sent_data;
    BT_GATTS_EVENT_READ_REQ_DATA read_req_data;
    BT_GATTS_EVENT_WRITE_REQ_DATA write_req_data;
    BT_GATTS_EVENT_WRITE_EXE_REQ_DATA write_exe_req_data;
    BT_GATTS_EVENT_MTU_CHG_DATA mtu_chg_data;
    BT_GATTS_EVENT_READ_PHY_DATA read_phy_data;
    BT_GATTS_EVENT_PHY_UPDATE_DATA phy_upd_data;
    BT_GATTS_EVENT_CONN_UPDATE_DATA conn_upd_data;
} BT_GATTS_EVENT_DATA;

/* when report a GATTS_EVENT_ to app, this paramter is passed to APP
*/
typedef struct
{
    BT_GATTS_EVENT event;        /* event id */
    INT32 server_if;
    BT_GATTS_EVENT_DATA data;    /* event data */
}BT_GATTS_EVENT_PARAM;

typedef void (*BT_GATTS_EVENT_HANDLE_CB)(BT_GATTS_EVENT_PARAM *param);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


#endif /* End of U_BT_MW_GATTS_H */
