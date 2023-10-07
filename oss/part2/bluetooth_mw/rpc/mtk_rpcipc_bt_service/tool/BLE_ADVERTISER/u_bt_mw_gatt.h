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


/* FILE NAME:  u_bt_mw_gatt.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATT COMMON structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_GATT_H_
#define _U_BT_MW_GATT_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_bt_mw_types.h"

#define BT_GATT_MAX_VALUE_LEN (600)
#define BT_GATT_MAX_UUID_LEN (37)
#define BT_GATT_MAX_NAME_LEN (37)

#define BT_GATT_DEFAULT_MTU_SIZE (23)

#define BT_GATT_MIN_MTU_SIZE (23)
#define BT_GATT_MAX_MTU_SIZE (517)

#define BT_GATT_MAX_ATTR_CNT   (70)

#define BT_GATT_MAX_APP_CNT    (60)

#define BT_GATT_MAX_CONNECT_CNT    (16)

typedef enum
{/* Success code and error codes */
    BT_GATT_STATUS_OK,
    BT_GATT_STATUS_INVALID_HANDLE,
    BT_GATT_STATUS_READ_NOT_PERMIT,
    BT_GATT_STATUS_WRITE_NOT_PERMIT,
    BT_GATT_STATUS_INVALID_PDU,
    BT_GATT_STATUS_INSUF_AUTHENTICATION,
    BT_GATT_STATUS_REQ_NOT_SUPPORTED,
    BT_GATT_STATUS_INVALID_OFFSET,
    BT_GATT_STATUS_INSUF_AUTHORIZATION,
    BT_GATT_STATUS_PREPARE_Q_FULL,
    BT_GATT_STATUS_NOT_FOUND,
    BT_GATT_STATUS_NOT_LONG,
    BT_GATT_STATUS_INSUF_KEY_SIZE,
    BT_GATT_STATUS_INVALID_ATTR_LEN,
    BT_GATT_STATUS_ERR_UNLIKELY,
    BT_GATT_STATUS_INSUF_ENCRYPTION,
    BT_GATT_STATUS_UNSUPPORT_GRP_TYPE,
    BT_GATT_STATUS_INSUF_RESOURCE,

    BT_GATT_STATUS_ILLEGAL_PARAMETER=0x87,
    BT_GATT_STATUS_NO_RESOURCES=0x80,
    BT_GATT_STATUS_INTERNAL_ERROR,
    BT_GATT_STATUS_WRONG_STATE,
    BT_GATT_STATUS_DB_FULL,
    BT_GATT_STATUS_BUSY,
    BT_GATT_STATUS_ERROR,
    BT_GATT_STATUS_CMD_STARTED,
    BT_GATT_STATUS_PENDING=0x88,
    BT_GATT_STATUS_AUTH_FAIL,
    BT_GATT_STATUS_MORE,
    BT_GATT_STATUS_INVALID_CFG,
    BT_GATT_STATUS_SERVICE_STARTED,
    BT_GATT_STATUS_ENCRYPED_MITM=BT_GATT_STATUS_OK,
    BT_GATT_STATUS_ENCRYPED_NO_MITM=0x8d,
    BT_GATT_STATUS_NOT_ENCRYPTED,
    BT_GATT_STATUS_CONGESTED,

    BT_GATT_STATUS_DUP_REG,
    BT_GATT_STATUS_ALREADY_OPEN,
    BT_GATT_STATUS_CANCEL,
    /* 0xE0 ~ 0xFC reserved for future use */

    /* Client Characteristic Configuration Descriptor Improperly Configured */
    BT_GATT_STATUS_CCC_CFG_ERR=0xFD,
    /* Procedure Already in progress */
    BT_GATT_STATUS_PRC_IN_PROGRESS,
    /* Attribute value out of range */
    BT_GATT_STATUS_OUT_OF_RANGE,
} BT_GATT_STATUS;


typedef enum
{
  GATT_ATTR_TYPE_PRIMARY_SERVICE,
  GATT_ATTR_TYPE_SECONDARY_SERVICE,
  GATT_ATTR_TYPE_INCLUDED_SERVICE,
  GATT_ATTR_TYPE_CHARACTERISTIC,
  GATT_ATTR_TYPE_DESCRIPTOR,
} GATT_ATTR_TYPE;

typedef enum
{
    GATT_ATTR_PERM_READ,
    GATT_ATTR_PERM_READ_ENCRYPTED,
    GATT_ATTR_PERM_READ_ENC_MITM,
    GATT_ATTR_PERM_WRITE,
    GATT_ATTR_PERM_WRITE_ENCRYPTED,
    GATT_ATTR_PERM_WRITE_ENC_MITM,
    GATT_ATTR_PERM_WRITE_SIGNED,
    GATT_ATTR_PERM_WRITE_SIGNED_MITM,
} GATT_ATTR_PERM;

typedef enum
{
    GATT_ATTR_PROP_BROADCAST,
    GATT_ATTR_PROP_READ,
    GATT_ATTR_PROP_WRITE_WITHOUT_RSP,
    GATT_ATTR_PROP_WRITE,
    GATT_ATTR_PROP_NOTIFY,
    GATT_ATTR_PROP_INDICATE,
    GATT_ATTR_PROP_AUTH_SIGNED_WRITE,
    GATT_ATTR_PROP_EXT_PROP,
} GATTS_ATTR_PROP;

typedef enum {
  GATT_TRANSPORT_TYPE_AUTO,  /* transport type decided by device type */
  GATT_TRANSPORT_TYPE_BREDR, /* BR/EDR connection */
  GATT_TRANSPORT_TYPE_LE     /* BLE connection */
} GATT_TRANSPORT_TYPE;

typedef enum {
  GATT_PHY_SETTING_LE_1M,
  GATT_PHY_SETTING_LE_2M,
  GATT_PHY_SETTING_LE_CODED,
} GATT_PHY_SETTING;


typedef enum {
  GATT_PHY_OPT_CODED_NO_PREF,
  GATT_PHY_OPT_CODED_S2,
  GATT_PHY_OPT_CODED_S8,
} GATT_PHY_OPT_CODED;

/* primary/secondary service provide uuid, type */
/* include service provid handle, type */
/* char provide uuid, type, key size, permissions, property */
/* descriptor provide uuid, type, key size, permissions */
typedef struct
{
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    GATT_ATTR_TYPE type; /* attr type, pri, sec, include, ...*/
    UINT8 properties;   /* bitwise of GATTS_ATTR_PROP */
    UINT8 key_size;
    UINT16 permissions; /* bitwise of GATT_ATTR_PERM */
    UINT16 handle;
} BT_GATT_ATTR;

#endif /*  _U_BT_MW_GATT_H_ */




