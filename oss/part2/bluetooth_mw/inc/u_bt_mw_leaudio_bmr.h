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

#ifndef _U_BT_MW_LEAUDIO_BMR_H_
#define _U_BT_MW_LEAUDIO_BMR_H_

#include "u_bt_mw_common.h"
#include "u_bt_mw_leaudio_common.h"

#define MAX_BIS_NUM     (31)
#define BMR_MAX_BLACKLIST_SIZE    (128)
#define BIS_SUPPORT_NUM (2)
#define BMR_BROADCAST_CODE_SIZE (16)

#define BIS_SYNC_NO_PREFERENCE  (0xFFFFFFFF)
#define MAX_BIS_INDEX  (31)

typedef enum
{
    BT_BMR_PAC_TYPE_SINK_PAC = 0,
    BT_BMR_PAC_TYPE_SINK_AUDIO_LOCATIONS,
    BT_BMR_PAC_TYPE_SOURCE_PAC,
    BT_BMR_PAC_TYPE_SOURCE_AUDIO_LOCATIONS,
    BT_BMR_PAC_TYPE_AVAI_SINK_CONTEXTS,
    BT_BMR_PAC_TYPE_AVAI_SOURCE_CONTEXTS,
    BT_BMR_PAC_TYPE_SUPP_SINK_CONTEXTS,
    BT_BMR_PAC_TYPE_SUPP_SOURCE_CONTEXTS,
} BT_BMR_PAC_TYPE_E;

typedef enum
{
    BT_BMR_CONN_STATE_DISCONNECTED = 0,
    BT_BMR_CONN_STATE_CONNECTING,
    BT_BMR_CONN_STATE_CONNECTED,
    BT_BMR_CONN_STATE_DISCONNECTING,
} BT_BMR_CONN_STATE_E;

typedef enum
{
    BT_BMR_AUTONOMOUSLY_SCAN_STOPPED = 0,
    BT_BMR_AUTONOMOUSLY_SCAN_STARTED,
    BT_BMR_REMOTE_SCAN_STARTED, //BSA remote scan started, autonomously scan will stop
    BT_BMR_REMOTE_SCAN_STOPPED,
} BT_BMR_SCAN_STATE_E;

typedef enum
{
    BT_BMR_SOLICITATION_REQUEST_STOPPED = 0,    //connectable advertising is enabled
    BT_BMR_SOLICITATION_REQUEST_STARTED,    //connectable advertising is disabled
} BT_BMR_SOLICITATION_REQUEST_STATE_E;

typedef enum
{
    BT_BMR_SRC_ADD = 0,
    BT_BMR_SRC_UPDATE,
    BT_BMR_SRC_REMOVE,
} BT_BMR_SRC_ACTION_E;

typedef enum
{
    BT_BMR_SRC_STATE_IDLE = 0,                  //PA is not synced
    BT_BMR_SRC_STATE_PA_SYNCING = 1,            //start to sync PA
    BT_BMR_SRC_STATE_PA_SYNCED = 2,             //PA is synced
    BT_BMR_SRC_STATE_READY_FOR_STREAMING = 3,   //BIGInfo is available
    BT_BMR_SRC_STATE_STREAM_STARTING = 4,       //Start the streaming-start procedure
    BT_BMR_SRC_STATE_STREAM_STARTED = 5,        //BIS packet is received
    BT_BMR_SRC_STATE_STREAM_STOPPING = 6,       //Start the streaming-stop procedure, if success, the NEXT state shall goto READY_FOR_STREAMING, if fail, could be IDLE(sync fail)
} BT_BMR_SRC_STATE_E;

typedef enum
{
    BT_BMR_STERAMING_START_SUCCESS = 0,
    BT_BMR_STERAMING_START_FAIL,
    BT_BMR_STERAMING_STOP_SUCCESS,
    BT_BMR_STERAMING_STOP_FAIL,
}BT_BMR_STREAMING_EVENT_E;

typedef enum
{
    BT_BMR_ERROR_CODE_SUCCESS = 0,
    BT_BMR_ERROR_CODE_PA_SYNC_FAIL,
    BT_BMR_ERROR_CODE_INVALID_BASE,
    BT_BMR_ERROR_CODE_BIG_SYNC_FAIL,
    BT_BMR_ERROR_CODE_ISO_DATA_PATH_SETUP_FAIL,
    BT_BMR_ERROR_CODE_PA_SYNC_LOST,
    BT_BMR_ERROR_CODE_BIG_SYNC_LOST,
    BT_BMR_ERROR_CODE_BIS_DECRYPTION_ERROR,
    BT_BMR_ERROR_CODE_UNKNOWN_SOURCE,
    BT_BMR_ERROR_CODE_INVALID_PARAMS,
    BT_BMR_ERROR_CODE_INVALID_STATE,
    BT_BMR_ERROR_CODE_BUSY,
    //TODO add more
} BT_BMR_ERROR_CODE_E;

typedef enum
{
    BT_BMR_BIG_UNENCRYPTED = 0,
    BT_BMR_BIG_ENCRYPTED,
} BT_BMR_BIG_ENCRYPTION_E;

/*****************************************************************************
**  Callback Event to app
*****************************************************************************/
typedef enum
{
    BT_BMR_EVENT_CONN_STATE = 0,
    BT_BMR_EVENT_SOLICITATION_REQUEST_STATE,
    BT_BMR_EVENT_SCAN_STATE,
    BT_BMR_EVENT_SRC_INFO,
    BT_BMR_EVENT_SRC_STATE_CHANGE,
    BT_BMR_EVENT_STREAMING_EVENT,
} BT_BMR_EVENT_E;

/*****************************************************************************
**  Callback Data to app
*****************************************************************************/
typedef struct {
    UINT8 sampling_freq;
    UINT8 frame_duration;
    UINT32 channel_allocation;
    UINT16 octets_per_codec_frame;
    UINT8  blocks_per_sdu;
} BT_BMR_CODEC_CONFIGURATION_T;

typedef struct {
    UINT8 prefered_audio_contexts;
    UINT8 streaming_audio_contexts;
    UINT8 vendor_metadata_length;  //0~255
    UINT8 *vendor_metadata;    //the buf length shall be vendor_specific_metadata_length
} BT_BMR_METADATA_T;

typedef struct {
    UINT8 coding_format;
    UINT16 company_id;
    UINT16 vendor_codec_id;
} BT_BMR_CODEC_ID_T;

typedef struct
{
    UINT8 bis_index;    //1~31
    UINT8 subgroup_id;  //0~30
    BT_BMR_CODEC_CONFIGURATION_T codec_configs;
    BOOL b_playable;    //stack match BIS codec config and local capability, and set the bis playable or not
} BT_BMR_BIS_T;

typedef struct {
    UINT8 subgroup_id;  //0~30
    UINT8 num_bis;  //1~31
    BT_BMR_CODEC_ID_T codec_id;
    BT_BMR_CODEC_CONFIGURATION_T codec_configs;
    BT_BMR_METADATA_T metadata;
    BT_BMR_BIS_T bis_info[MAX_BIS_NUM]; //the valid element number in this array shall be num_bis
} BT_BMR_SUBGROUP_T;

typedef struct
{
    UINT8 source_id;    //this is a entry in stack source list.
    UINT32 broadcast_id;    //this is unique id for a source
    BT_BMR_BIG_ENCRYPTION_E encryption;    //source/BIG is encrypted or not
    CHAR addr[MAX_BDADDR_LEN];
    UINT8 addr_type;    //0 - Public;  1 - Random
    UINT8 adv_sid;
    BT_BMR_SRC_STATE_E state;
    UINT32 bis_sync_req_mask;
    UINT32 presentation_delay;
    UINT8 num_subgroups;    //1~31
    BT_BMR_SUBGROUP_T *subgroups;  //the subgroups length shall be num_subgropus*sizeof(BT_BMR_SUBGROUP_INFO_T)
} BT_BMR_SRC_INFO_T;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BT_BMR_CONN_STATE_E state;
} BT_BMR_EVENT_CONN_STATE_DATA_T;

typedef struct
{
    BT_BMR_SCAN_STATE_E state;
    CHAR bt_addr[MAX_BDADDR_LEN];
} BT_BMR_EVENT_SCAN_STATE_DATA_T;

typedef struct
{
    BT_BMR_SRC_ACTION_E action;
    BT_BMR_SRC_INFO_T info;
} BT_BMR_EVENT_SRC_INFO_DATA_T;

typedef struct
{
    UINT8 source_id;
    BT_BMR_SRC_STATE_E state;
    BT_BMR_ERROR_CODE_E err_code;
} BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T;

typedef struct {
    BOOL in_use;
    UINT8 bis_idx;
    UINT8 socket_idx;
} BT_BMR_SOCKET_INDEX_T;

typedef struct  {
    UINT8 source_id;
    UINT8 bis_num;// 1 or 2 for sync bis
    BT_BMR_SOCKET_INDEX_T simap[BIS_SUPPORT_NUM];
} BT_BMR_STREAMING_EVENT_T;

typedef struct
{
    BT_BMR_STREAMING_EVENT_T data;
    BT_BMR_STREAMING_EVENT_E event;
    BT_BMR_ERROR_CODE_E err_code;
} BT_BMR_EVENT_STREAMING_EVENT_DATA_T;

typedef union
{
    BT_BMR_EVENT_CONN_STATE_DATA_T conn_state;
    BT_BMR_SOLICITATION_REQUEST_STATE_E solicitation_req_state;
    BT_BMR_EVENT_SCAN_STATE_DATA_T scan_state;
    BT_BMR_EVENT_SRC_INFO_DATA_T source_info;
    BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T source_state;
    BT_BMR_EVENT_STREAMING_EVENT_DATA_T streaming_event;
} BT_BMR_EVENT_DATA;

typedef struct
{
    BT_BMR_EVENT_E event;
    BT_BMR_EVENT_DATA data;
} BT_BMR_EVENT_PARAM;

typedef struct {
    //bit mask of the audio channels local support.  e.g. BT_LE_AUDIO_ALLOCATION_FRONTLEFT|BT_LE_AUDIO_ALLOCATION_FRONTRIGHT
    UINT32 audio_locations;
    //add more if needed
} BT_BMR_INIT_PARAMS;

typedef enum
{
    BT_BMR_AUTONOMOUSLY_SCAN_STOP = 0,
    BT_BMR_AUTONOMOUSLY_SCAN_START,
} BT_BMR_AUTONOMOUSLY_SCAN_OPCODE_E;

typedef struct {
    BT_BMR_AUTONOMOUSLY_SCAN_OPCODE_E op_code;
    UINT32 scan_duration;
    UINT32 black_list[BMR_MAX_BLACKLIST_SIZE];
} BT_BMR_DISCOVERY_PARAMS;


typedef enum
{
    BT_BMR_SOLICITATION_REQUEST_STOP = 0,
    BT_BMR_SOLICITATION_REQUEST_START,
} BT_BMR_SOLICITATION_REQUEST_OPCODE_E;

typedef struct {
    BT_BMR_SOLICITATION_REQUEST_OPCODE_E op_code;
    UINT32 adv_duration;
    UINT8 *user_data;
    UINT32 user_data_len;
} BT_BMR_SOLICITATION_REQUEST_PARAMS;

typedef struct {
    UINT8 source_id;
    UINT8 broadcast_code[BMR_BROADCAST_CODE_SIZE];
} BT_BMR_SET_BROADCAST_CODE_PARAMS;

typedef void (*BT_BMR_EVENT_HANDLE_CB)(BT_BMR_EVENT_PARAM *param);

#endif /*  _U_BT_MW_LEAUDIO_BMR_H_ */
