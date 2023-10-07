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


/* FILE NAME:  u_bt_mw_leaudio_bms.h
 * AUTHOR:
 * PURPOSE:
 *      It provides bluetooth leaudio bms profile structure to APP.
 * NOTES:
 */
#ifndef _U_BT_MW_LEAUDIO_BMS_H_
#define _U_BT_MW_LEAUDIO_BMS_H_

#include "u_bt_mw_common.h"
#include "u_bt_mw_leaudio_common.h"

typedef enum
{
    BT_BMS_EVENT_BROADCAST_CREATED = 0,
    BT_BMS_EVENT_BROADCAST_DESTORYED = 1,
    BT_BMS_EVENT_BROADCAST_STATE = 2,
    BT_BMS_EVENT_SOCKET_INDEX_NOTIFY = 3,
    BT_BMS_EVENT_ISO_STATUS = 4,
    BT_BMS_EVENT_OWN_ADDRESS = 5,
    BT_BMS_EVENT_MAX
} BT_BMS_EVENT;

typedef enum
{
    BT_BMS_STOPPED = 0,
    BT_BMS_PAUSED = 1,
    BT_BMS_STREAMING = 2
} BT_BMS_BROADCAST_STATE;

typedef enum {
    BT_BMS_SONIFICATION = 0,
    BT_BMS_MEDIA,
} BT_BMS_PROFILE_NAME;

typedef enum {
    BASE_CONFIG_16_2_1_LL = 1,  //16_2_1 LOW_LATENCY
    BASE_CONFIG_16_2_2_HR = 2,  //16_2_2 HIGH_RELIABILITY (SONIFICATION_DEFAULT)
    BASE_CONFIG_48_1_1_LL = 3,  //48_1_1 LOW_LATENCY
    BASE_CONFIG_48_1_2_HR = 4,  //48_1_2 HIGH_RELIABILITY
    BASE_CONFIG_48_2_1_LL = 5,  //48_2_1 LOW_LATENCY
    BASE_CONFIG_48_2_2_HR = 6,  //48_2_2 HIGH_RELIABILITY (MEDIA_DEFAULT)
    BASE_CONFIG_48_3_1_LL = 7,  //48_3_1 LOW_LATENCY
    BASE_CONFIG_48_3_2_HR = 8,  //48_3_2 HIGH_RELIABILITY
    BASE_CONFIG_48_4_1_LL = 9,  //48_4_1 LOW_LATENCY
    BASE_CONFIG_48_4_2_HR = 10, //48_4_2 HIGH_RELIABILITY
    BASE_CONFIG_48_5_1_LL = 11, //48_5_1 LOW_LATENCY
    BASE_CONFIG_48_5_2_HR = 12, //48_5_2 HIGH_RELIABILITY
    BASE_CONFIG_48_6_1_LL = 13, //48_6_1 LOW_LATENCY
    BASE_CONFIG_48_6_2_HR = 14, //48_6_2 HIGH_RELIABILITY
} BT_BMS_BASE_CONFIG_NAME;

typedef enum {
    CODEC_CONFIG_NOT_SET = 0,
    CODEC_CONFIG_16_2 = 1,  //SONIFICATION_DEFAULT
    CODEC_CONFIG_48_1 = 2,
    CODEC_CONFIG_48_2 = 3, //MEDIA_DEFAULT
    CODEC_CONFIG_48_3 = 4,
    CODEC_CONFIG_48_4 = 5,
    CODEC_CONFIG_48_5 = 6,
    CODEC_CONFIG_48_6 = 7,
    CODEC_CONFIG_24_2 = 8,
} BT_BMS_CODEC_CONFIG_NAME;

typedef struct
{
    UINT32 audio_channel_allocation; //bitfield mask UINT32, ref BT_LE_AUDIO_ALLOCATION_SET
} BT_BMS_ANNOUNCEMENT_BIS;

typedef struct
{
    BT_BMS_CODEC_CONFIG_NAME codec_config_name; //MW use profile name to set bas config name and codec config name to stack
    UINT32 audio_channel_allocation; //bitfield mask UINT32, ref BT_LE_AUDIO_ALLOCATION_SET
    UINT8 metadata_len;
    UINT8 metadata[BT_BROADCAST_METADATA_SIZE];
    UINT8 bis_num;
    BT_BMS_ANNOUNCEMENT_BIS bis[BT_BROADCAST_MAX_BIS_NUM];
} BT_BMS_ANNOUNCEMENT_SUBGROUP;

typedef struct
{
    BOOL pbas_on;
    BT_BMS_BASE_CONFIG_NAME bas_config_name;
    UINT8 subgroup_num;
    BT_BMS_ANNOUNCEMENT_SUBGROUP subgroup[BT_BROADCAST_MAX_SUBGROUP_NUM];
} BT_BMS_ANNOUNCEMENT;

typedef struct
{
    UINT8 instance_id;
    BOOL success;  /* broadcast created success or not*/
} BT_BMS_EVENT_BROADCAST_CREATED_DATA;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_EVENT_BROADCAST_DESTORYED_DATA;

typedef struct
{
    UINT8 instance_id;
    BT_BMS_BROADCAST_STATE state;  /* broadcast state */
} BT_BMS_EVENT_BROADCAST_STATE_DATA;

typedef struct
{
    UINT8 socket_index;
    UINT8 channel_num;
    UINT8 subgroup_num;
    UINT8 subgroup_ids[BT_BROADCAST_MAX_SUBGROUP_NUM];
    BOOL iso_status;  /* iso setup or removed*/
} BT_BMS_EVENT_SOCKET_CHANNEL_MAP;

typedef struct
{
    UINT8 instance_id;
    UINT8 socket_index_num;
    BT_BMS_EVENT_SOCKET_CHANNEL_MAP socket_index_list[BT_BROADCAST_SOCKET_INDEX_NUM];
} BT_BMS_EVENT_SOCKET_INDEX_DATA;

typedef struct
{
    UINT8 instance_id;
    UINT8 socket_index;
    BOOL up;  /* iso setup or removed*/
} BT_BMS_EVENT_ISO_STATUS_DATA;

typedef struct
{
    UINT8 instance_id;
    UINT8 addr_type;
    CHAR addr[MAX_BDADDR_LEN];
} BT_BMS_EVENT_OWN_ADDRESS_DATA;

typedef union
{
    BT_BMS_EVENT_BROADCAST_CREATED_DATA broadcast_created;
    BT_BMS_EVENT_BROADCAST_DESTORYED_DATA broadcast_destroyed;
    BT_BMS_EVENT_BROADCAST_STATE_DATA broadcast_state;
    BT_BMS_EVENT_SOCKET_INDEX_DATA socket_index;
    BT_BMS_EVENT_ISO_STATUS_DATA iso_status;
    BT_BMS_EVENT_OWN_ADDRESS_DATA own_address;
} BT_BMS_EVENT_DATA;

typedef struct
{
    BT_BMS_EVENT event;
    BT_BMS_EVENT_DATA data;
} BT_BMS_EVENT_PARAM;

typedef VOID (*BtAppBmsCbk)(BT_BMS_EVENT_PARAM *pt_bms_struct);


/* bms api params*/
typedef struct
{
    UINT8 localname_len;
    UINT8 localname[MAX_NAME_LEN];
    UINT8 metadata_len;
    UINT8 metadata[BT_BROADCAST_METADATA_SIZE];
    BT_BMS_PROFILE_NAME profile;
    UINT8 broadcast_code_len;
    UINT8 broadcast_code[BT_BROADCAST_BROADCAST_CODE_SIZE];
} BT_BMS_CREATE_BROADCAST_PARAM;

typedef struct
{
    UINT8 localname_len;
    UINT8 localname[MAX_NAME_LEN];
    BT_BMS_ANNOUNCEMENT announcement;
    UINT8 broadcast_code_len;
    UINT8 broadcast_code[BT_BROADCAST_BROADCAST_CODE_SIZE];
} BT_BMS_CREATE_BROADCAST_EXT_PARAM;

typedef struct
{
    UINT8 instance_id;
    BT_BMS_ANNOUNCEMENT announcement;
} BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_START_BROADCAST_PARAM;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_PAUSE_BROADCAST_PARAM;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_STOP_BROADCAST_PARAM;

typedef struct
{
    UINT8 instance_id;
} BT_BMS_GET_OWN_ADDRESS_PARAM;

typedef struct
{
    UINT8 instance_id;
    UINT8 subgroup_id; //subgroup_id is the index of subgroup array in BT_BMS_ANNOUNCEMENT
    UINT8 metadata_len;
    UINT8 metadata[BT_BROADCAST_METADATA_SIZE];
} BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM;
/* bms api params*/

#endif /*  _U_BT_MW_LEAUDIO_BMS_H_ */
