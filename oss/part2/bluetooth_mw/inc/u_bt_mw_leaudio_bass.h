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

#ifndef _U_BT_MW_LEAUDIO_BASS_H_
#define _U_BT_MW_LEAUDIO_BASS_H_

#include "u_bt_mw_common.h"
#include "u_bt_mw_leaudio_common.h"

#define MAX_BASS_DEV_NUM (4)

typedef enum
{
    BT_BASS_CONN_STATE_DISCONNECTED = 0,
    BT_BASS_CONN_STATE_CONNECTING,
    BT_BASS_CONN_STATE_CONNECTED,
    BT_BASS_CONN_STATE_DISCONNECTING,
} BT_BASS_CONN_STATE;

typedef enum
{
    BT_BASS_RECV_STATE_IDLE = 0,
    BT_BASS_RECV_STATE_SET_SOURCE_FAILED,
    BT_BASS_RECV_STATE_SYNCING,
    BT_BASS_RECV_STATE_SYNC_PA_FAILED,
    BT_BASS_RECV_STATE_SYNC_BIS_FAILED,
    BT_BASS_RECV_STATE_SYNC_BIS_STOPPED,
    BT_BASS_RECV_STATE_SYNCED_TO_PA,
    BT_BASS_RECV_STATE_BROADCAST_CODE_REQUIRED,
    BT_BASS_RECV_STATE_BAD_BROADCAST_CODE,
    BT_BASS_RECV_STATE_RECEIVING_BROADCAST,
} BT_BASS_RECV_STATE;

typedef struct
{
    UINT32 sync_bis;
    UINT8 meta_data_len;
    UINT8 meta_data[BT_BROADCAST_METADATA_SIZE];
} BT_BASS_BROADCAST_SRC_SUBGROUP;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 sync_pa;
    UINT8 num_subgroups;
    BT_BASS_BROADCAST_SRC_SUBGROUP subgroup[BT_BROADCAST_MAX_SUBGROUP_NUM];
} BT_BASS_BROADCAST_SRC;

typedef struct
{
    UINT8 num_subgroups;
    BT_BASS_BROADCAST_SRC_SUBGROUP subgroup[BT_BROADCAST_MAX_SUBGROUP_NUM];
} BT_BASS_BROADCAST_RECV;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 addr_type;
    UINT8 adv_sid;
} BT_BASS_BROADCAST_ID;

typedef struct
{
    // Codec-Specific Configuration
    UINT8 sampling_frequency;
    UINT8 frame_duration;
    UINT16 octets_per_codec_frame;
    UINT8 codec_frame_blocks_per_sdu;
    UINT32 audio_channel_allocation;
} BT_BASS_LC3_CODEC_CONFIG;

typedef struct
{
    UINT8 codec_id;
    UINT16 vendor_company_id;
    UINT16 vendor_codec_id;
    BT_BASS_LC3_CODEC_CONFIG codec_param;
} BT_BASS_ANNOUNCE_CODEC_CONFIG;

typedef struct
{
    UINT8 bis_index;
    BT_BASS_LC3_CODEC_CONFIG codec_param;
} BT_BASS_ANNOUNCE_BIS_CONFIG;

typedef struct
{
    BT_BASS_ANNOUNCE_CODEC_CONFIG codec_config;
    UINT8 meta_data_len;
    UINT8 meta_data[BT_BROADCAST_METADATA_SIZE];
    UINT8 bis_size;
    BT_BASS_ANNOUNCE_BIS_CONFIG bis_configs[BT_BROADCAST_MAX_BIS_NUM];
} BT_BASS_ANNOUNCE_SUBGROUP;

typedef struct
{
    UINT32 presentation_delay;
    UINT8 subgroup_size;
    BT_BASS_ANNOUNCE_SUBGROUP subgroup_configs[BT_BROADCAST_MAX_SUBGROUP_NUM];
} BT_BASS_ANNOUNCE_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
} BT_BASS_CONN_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
} BT_BASS_DISC_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL scan;
    UINT8 duration;
} BT_BASS_BROADCAST_SCAN_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    INT32 receiver_id;
} BT_BASS_BROADCAST_RECV_STATE_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    INT32 receiver_id;
    UINT8 code[BT_BROADCAST_BROADCAST_CODE_SIZE];
} BT_BASS_BROADCAST_CODE_PARAM;

typedef struct
{
    BT_BASS_BROADCAST_SRC broadcast_src;
    BT_BASS_BROADCAST_ID broadcast_id;
} BT_BASS_SET_BROADCAST_SRC_PARAM;

typedef struct
{
    BT_BASS_BROADCAST_SRC broadcast_src;
    INT32 receiver_id;
} BT_BASS_MODIFY_BROADCAST_SRC_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    INT32 receiver_id;
} BT_BASS_REMOVE_BROADCAST_SRC_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL enable;
    UINT32 sync_bis;
} BT_BASS_SET_BUILTIN_MODE_PARAM;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL in_used;
    BOOL is_builtin;
    UINT32 builtin_sync_bis;
} BT_BASS_CONNECTED_DEVICE;

typedef struct
{
    BT_BASS_CONNECTED_DEVICE device[MAX_BASS_DEV_NUM];
} BT_BASS_CONNECTION_INFO;

/*****************************************************************************
**  Callback Event to app
*****************************************************************************/
typedef enum
{
    BT_BASS_EVENT_CONN_STATE = 0,
    BT_BASS_EVENT_DEV_AVAIL,
    BT_BASS_EVENT_SCANNING_STATE,
    BT_BASS_EVENT_ANNOUNCE_RECEIVED,
    BT_BASS_EVENT_RECEIVER_STATE,
    BT_BASS_EVENT_BUILTIN_MODE,
    BT_BASS_EVENT_SYNC_LOST,
} BT_BASS_EVENT;

/*****************************************************************************
**  Callback Data to app
*****************************************************************************/
typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BT_BASS_CONN_STATE state;
} BT_BASS_EVENT_CONN_STATE_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    INT32 num_receivers;
} BT_BASS_EVENT_DEV_AVAIL_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL scan;
} BT_BASS_EVENT_SCANNING_STATE_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 local_name_len;
    CHAR local_name[MAX_NAME_LEN];
    BOOL encryption;
    BT_BASS_BROADCAST_ID broadcast_id;
    BT_BASS_ANNOUNCE_DATA announce_data;
} BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    INT32 receiver_id;
    BT_BASS_RECV_STATE state;
    BT_BASS_BROADCAST_ID broadcast_id;
    BT_BASS_BROADCAST_RECV data;
} BT_BASS_EVENT_RECV_STATE_DATA;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL enable;
    UINT32 sync_bis;
} BT_BASS_EVENT_BUILTIN_MODE_DATA;

typedef struct
{
    BT_BASS_BROADCAST_ID broadcast_id;
} BT_BASS_EVENT_SYNC_LOST_DATA;

typedef union
{
    BT_BASS_EVENT_CONN_STATE_DATA conn_state;
    BT_BASS_EVENT_DEV_AVAIL_DATA dev_avail;
    BT_BASS_EVENT_SCANNING_STATE_DATA scanning_state;
    BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA announce_received;
    BT_BASS_EVENT_RECV_STATE_DATA recv_state;
    BT_BASS_EVENT_BUILTIN_MODE_DATA builtin_mode;
    BT_BASS_EVENT_SYNC_LOST_DATA sync_lost;
} BT_BASS_EVENT_DATA;

typedef struct
{
    BT_BASS_EVENT event;
    BT_BASS_EVENT_DATA data;
} BT_BASS_EVENT_PARAM;

typedef void (*BT_BASS_EVENT_HANDLE_CB)(BT_BASS_EVENT_PARAM *param);

#endif /*  _U_BT_MW_LEAUDIO_BASS_H_ */
