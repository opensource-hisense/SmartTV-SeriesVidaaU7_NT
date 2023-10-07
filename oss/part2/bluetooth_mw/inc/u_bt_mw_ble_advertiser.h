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


#ifndef _U_BT_MW_BLE_ADVERTISER_H_
#define _U_BT_MW_BLE_ADVERTISER_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_bt_mw_common.h"
#include "u_bt_mw_gatt.h"

#define BT_BLE_ADV_MAX_ADV_DATA_LEN 1024

#define BT_BLE_ADV_LOCAL_NAME_MAX_LEN 26

#define BT_BLE_ADV_MAX_LEGACY_DATA_LEN 31

#define BT_BLE_ADV_INVALID_ADV_ID (0xFF)

typedef enum
{
    BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_UUIDS = 0x03,
    BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_32_BIT_SERVICE_UUIDS = 0x05,
    BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_128_BIT_SERVICE_UUIDS = 0x07,
    BT_BLE_ADV_AD_TYPE_SHORTENED_LOCAL_NAME = 0x08,
    BT_BLE_ADV_AD_TYPE_COMPLETE_LOCAL_NAME = 0x09,
    BT_BLE_ADV_AD_TYPE_TX_POWER_LEVEL = 0x0A,
    BT_BLE_ADV_AD_TYPE_SERVICE_DATA_16_BIT_UUID = 0x16,
    BT_BLE_ADV_AD_TYPE_SERVICE_DATA_32_BIT_UUID = 0x20,
    BT_BLE_ADV_AD_TYPE_SERVICE_DATA_128_BIT_UUID = 0x21,
    BT_BLE_ADV_AD_TYPE_MANUFACTURER_SPECIFIC_DATA = 0xFF,
} BT_BLE_ADV_AD_TYPE;

typedef enum
{
    BT_BLE_ADV_PHY_1M = 1,
    BT_BLE_ADV_PHY_2M = 2,
    BT_BLE_ADV_PHY_LE_CODED = 3,
} BT_BLE_ADV_PHY;

/******************** ble adv API parameter ***********************/
typedef struct
{
    BOOL connectable;/* TRUE-connectable adv, FALSE-non-connectable adv */
    BOOL scannable; /* TRUE-scannable adv, FALSE-non-scannable adv */
    BOOL legacy;    /* TRUE-legacy adv, FALSE-extended adv */
    BOOL anonymous; /* anonymouse undirect advertising */
    BOOL include_tx_power; /* only invalid for extended advertising */
} BT_BLE_ADV_PROPS_PARAM;

typedef struct
{
    UINT8 adv_id;
    BT_BLE_ADV_PROPS_PARAM props;
    UINT32 min_interval;        /* 0x20 ~ 0xFFFFFF, unit: 0.625ms */
    UINT32 max_interval;        /* 0x20 ~ 0xFFFFFF, unit: 0.625ms */
    INT8 tx_power;              /* -127 ~ 20, unit: dBm */
    UINT8 primary_adv_phy;      /* 1-1M, 3-LE Coded, LE coded only for extended adv */
    UINT8 secondary_adv_phy;    /* 1-1M, 2-2M, 3-LE Coded, for extended adv */
} BT_BLE_ADV_PARAM;

typedef struct
{
    UINT8 adv_id;
    UINT32 len;
    UINT8  data[BT_BLE_ADV_MAX_ADV_DATA_LEN];
} BT_BLE_ADV_DATA_PARAM;

typedef struct
{
    UINT8 adv_id;
    BOOL enable;            /* true: this parameter is valid, false: invalid */
    BOOL include_tx_power;  /* true: include tx power, false: don't include */
    UINT16 min_interval;    /* 0x6 ~ 0xFFFF, unit: 1.25ms */
    UINT16 max_interval;    /* 0x6 ~ 0xFFFF, unit: 1.25ms */
} BT_BLE_ADV_PERIODIC_PARAM;

typedef struct
{
    UINT8 adv_id;
    BOOL enable;
} BT_BLE_ADV_PERIODIC_ENABLE_PARAM;

typedef struct
{
    UINT8 adv_id;
    BOOL enable;
    UINT16 duration;
    UINT8 max_ext_adv_events;
} BT_BLE_ADV_ENABLE_PARAM;

typedef struct
{
    UINT8 adv_id;
} BT_BLE_ADV_GET_ADDR_PARAM;

typedef struct
{
    CHAR adv_name[BT_GATT_MAX_NAME_LEN];    /* adv set name, should be unique in sys */
    BT_BLE_ADV_PARAM adv_param;
    BT_BLE_ADV_DATA_PARAM adv_data;               /* for legacy adv or extended connectable adv */
    BT_BLE_ADV_DATA_PARAM scan_rsp;               /* for scannable adv */
    BT_BLE_ADV_PERIODIC_PARAM peri_param;   /* for periodic adv */
    BT_BLE_ADV_DATA_PARAM peri_data;              /* for periodic adv */
    UINT16 duration;                        /* unit: 10ms, for extended adv */
    UINT8 max_ext_adv_events;               /* for extended adv */
} BT_BLE_ADV_START_SET_PARAM;

typedef struct
{
    UINT8 adv_id;
} BT_BLE_ADV_STOP_SET_PARAM;

/******************** ble adv callback parameter ***********************/
typedef struct{
    UINT8 status;
    INT8 tx_power;
} BT_BLE_ADV_EVENT_SET_PARAM_DATA;

typedef struct
{
    UINT8 status;
} BT_BLE_ADV_EVENT_SET_ADV_DATA;


typedef struct
{
    UINT8 status;
} BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA;


typedef struct
{
    UINT8 status;
} BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA;


typedef struct
{
    UINT8 status;
} BT_BLE_ADV_EVENT_SET_PERIODIC_DATA;


typedef struct
{
    UINT8 status;
    UINT8 enable;
} BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA;


typedef struct
{
    UINT8 status;
    UINT8 enable;
} BT_BLE_ADV_EVENT_ENABLE_DATA;


typedef struct{
    UINT8 address_type; /* 0-public, 1-random */
    CHAR addr[MAX_BDADDR_LEN];
} BT_BLE_ADV_EVENT_GET_ADDR_DATA;


typedef struct
{
    UINT8 status;
    UINT8 enable;
} BT_BLE_ADV_EVENT_ENABLE_TIMEOUT_DATA;


typedef struct
{
    UINT8 status;
    INT8 tx_power;
    CHAR adv_name[BT_GATT_MAX_NAME_LEN];
} BT_BLE_ADV_EVENT_START_SET_DATA;

typedef enum
{
    BT_BLE_ADV_EVENT_START_ADV_SET,   /* Ble Adv start adv set         */
    BT_BLE_ADV_EVENT_SET_PARAM,       /* Ble Adv set adv param         */
    BT_BLE_ADV_EVENT_SET_DATA,        /* Ble Adv adv data              */
    BT_BLE_ADV_EVENT_SET_SCAN_RSP,    /* Ble Adv scan rsp data         */
    BT_BLE_ADV_EVENT_SET_PERI_PARAM,  /* Ble Adv set peri param        */
    BT_BLE_ADV_EVENT_SET_PERI_DATA,   /* Ble Adv set peri data         */
    BT_BLE_ADV_EVENT_ENABLE,          /* Ble Adv adv enable            */
    BT_BLE_ADV_EVENT_ENABLE_PERI,     /* Ble Adv adv enable            */
    BT_BLE_ADV_EVENT_GET_ADDR,        /* Ble Adv get adv addr          */
    BT_BLE_ADV_EVENT_MAX,             /* Ble Adv BT off                */
} BT_BLE_ADV_EVENT;

typedef struct
{
    BT_BLE_ADV_EVENT event;        /* event id */
    UINT8 adv_id;                  /* adv_id */
    union
    {
        BT_BLE_ADV_EVENT_START_SET_DATA             start_set_data;
        BT_BLE_ADV_EVENT_SET_PARAM_DATA             set_param_data;
        BT_BLE_ADV_EVENT_SET_ADV_DATA               set_adv_data;
        BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA          set_scan_rsp_data;
        BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA    set_periodic_param_data;
        BT_BLE_ADV_EVENT_SET_PERIODIC_DATA          set_periodic_data;
        BT_BLE_ADV_EVENT_ENABLE_DATA                enable_data;
        BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA       enable_periodic_data;
        BT_BLE_ADV_EVENT_GET_ADDR_DATA              get_addr_data;
    } data;/* event data */
} BT_BLE_ADV_EVENT_PARAM;

typedef void (*BT_BLE_ADV_EVENT_HANDLE_CB)(BT_BLE_ADV_EVENT_PARAM *param);

#endif /*  _U_BT_MW_BLE_ADVERTISER_H_ */
