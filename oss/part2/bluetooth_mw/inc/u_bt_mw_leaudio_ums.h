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


/* FILE NAME:  u_bt_mw_leaudio_ums.h
 * AUTHOR:
 * PURPOSE:
 *      It provides bluetooth leaudio ums profile structure to APP.
 * NOTES:
 */
#ifndef _U_BT_MW_LEAUDIO_UMS_H_
#define _U_BT_MW_LEAUDIO_UMS_H_

#include "u_bt_mw_common.h"
#include "u_bt_mw_leaudio_common.h"

typedef BT_LE_AUDIO_BASE_CONFIG_NAME BT_UMS_BASE_CONFIG_NAME;

typedef enum
{
    BT_UMS_CONN_STATE_DISCONNECTED = 0,
    BT_UMS_CONN_STATE_CONNECTED = 1
} BT_UMS_CONN_STATE;

typedef enum
{
    BT_UMS_GROUP_NODE_ADDED = 0,
    BT_UMS_GROUP_NODE_REMOVED = 1,
    BT_UMS_GROUP_NODE_REJECTED = 2
} BT_UMS_GROUP_NODE_STATUS;

typedef enum
{
    BT_UMS_GROUP_STATUS_IDLE = 0,
    BT_UMS_GROUP_STATUS_SUSPEND = 1,
    BT_UMS_GROUP_STATUS_STREAMING = 2,
    BT_UMS_GROUP_STATUS_DESTORYED = 3
} BT_UMS_GROUP_STATUS;

typedef enum
{
    BT_UMS_GROUP_LOCK_STATUS_SUCCESS = 0,
    BT_UMS_GROUP_LOCK_STATUS_FALIED_INVALID_GROUP = 1,
    BT_UMS_GROUP_LOCK_STATUS_FALIED_GROUP_EMPTY = 2,
    BT_UMS_GROUP_LOCK_STATUS_FALIED_GROUP_NOT_CONNECTD = 3,
    BT_UMS_GROUP_LOCK_STATUS_FALIED_LOCKED_BY_OTHER = 4,
    BT_UMS_GROUP_LOCK_STATUS_FALIED_OTHER_REASON = 5
} BT_UMS_GROUP_LOCK_STATUS;


typedef enum
{
    BT_UMS_EVENT_CONNECTION_STATE = 0,
    BT_UMS_EVENT_GROUP_NODE_STATUS = 1,
    BT_UMS_EVENT_GROUP_STATUS = 2,
    BT_UMS_EVENT_SOCKET_INDEX_NOTIFY = 3,
    BT_UMS_EVENT_AUDIO_CONF = 4,
    BT_UMS_EVENT_SET_MEMBER_AVAILABLE = 5,
    BT_UMS_EVENT_GROUP_LOCK_CHANGED = 6,
    BT_UMS_EVENT_MAX
} BT_UMS_EVENT;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    UINT8 allocation_len;
    UINT32 allocation[BT_UNICAST_DEV_MAX_ASE_SIZE];
} BT_UMS_REMOTE_ALLOCATION;

typedef struct
{
    BT_UMS_BASE_CONFIG_NAME bas_config_name;
    UINT8 remote_allocation_len;
    BT_UMS_REMOTE_ALLOCATION remote_allocation[BT_UNICAST_MAX_DEVICE_SIZE];
    UINT8 metadata_len;
    UINT8 metadata[BT_UNICAST_METADATA_SIZE];
} BT_UMS_CONFIG;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BT_UMS_CONN_STATE state;
} BT_UMS_EVENT_CONNECTION_STATE_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 group_id;
    BT_UMS_GROUP_NODE_STATUS status;
} BT_UMS_EVENT_GROUP_NODE_STATUS_DATA;

typedef struct
{
    UINT8 group_id;
    BT_UMS_GROUP_NODE_STATUS status;
} BT_UMS_EVENT_GROUP_STATUS_DATA;

typedef struct
{
    UINT8 group_id;
    UINT8 socket_index;
    UINT8 channel_num;
} BT_UMS_EVENT_SOCKET_INDEX_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 direction;
    UINT8 group_id;
    UINT32 snk_audio_location;
    UINT32 src_audio_location;
    UINT16 snk_avail_context;
    UINT16 src_avail_context;
} BT_UMS_EVENT_AUDIO_CONF_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 group_id;
} BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA;

typedef struct
{
    UINT8 group_id;
    BOOL lock;
    BT_UMS_GROUP_LOCK_STATUS group_lock_status;
} BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA;

typedef union
{
    BT_UMS_EVENT_CONNECTION_STATE_DATA conn_state;
    BT_UMS_EVENT_GROUP_NODE_STATUS_DATA group_node_status;
    BT_UMS_EVENT_GROUP_STATUS_DATA group_status;
    BT_UMS_EVENT_SOCKET_INDEX_DATA socket_index;
    BT_UMS_EVENT_AUDIO_CONF_DATA audio_conf;
    BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA set_member_avail;
    BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA group_lock_changed;
} BT_UMS_EVENT_DATA;

typedef struct
{
    BT_UMS_EVENT event;
    BT_UMS_EVENT_DATA data;
} BT_UMS_EVENT_PARAM;

typedef VOID (*BtAppUmsCbk)(BT_UMS_EVENT_PARAM *param);


/* ums api params*/
typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
} BT_UMS_CONNECT_PARAM;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
} BT_UMS_DISCONNECT_PARAM;

typedef struct
{
    UINT8 group_id;
} BT_UMS_GROUP_START_PARAM;

typedef struct
{
    UINT8 group_id;
    BT_UMS_CONFIG config;
} BT_UMS_GROUP_START_EXT_PARAM;

typedef struct
{
    UINT8 group_id;
} BT_UMS_GROUP_STOP_PARAM;

typedef struct
{
    UINT8 group_id;
    UINT8 metadata_len;
    UINT8 metadata[BT_UNICAST_METADATA_SIZE];
} BT_UMS_UPDATE_METADATA_PARAM;

typedef struct
{
    UINT8 group_id;
    BOOL lock;
} BT_UMS_GROUP_LOCK_SET_PARAM;
/* ums api params*/

#endif /*  _U_BT_MW_LEAUDIO_UMS_H_ */
