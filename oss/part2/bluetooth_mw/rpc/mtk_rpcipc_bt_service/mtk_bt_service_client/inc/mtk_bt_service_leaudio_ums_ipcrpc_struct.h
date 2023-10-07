/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Wed Dec  1 15:46:45 2021'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_LEAUDIO_UMS_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_LEAUDIO_UMS_IPCRPC_STRUCT__H_




/* Start of header pre-amble file 'preamble_file.h'. */

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

#include "u_rpc.h"


/* End of header pre-amble file 'preamble_file.h'. */


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_leaudio_ums_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_leaudio_ums_desc__ (1))


#define RPC_DESC_BT_UMS_REMOTE_ALLOCATION  (__rpc_get_leaudio_ums_desc__ (2))


#define RPC_DESC_BT_UMS_CONFIG  (__rpc_get_leaudio_ums_desc__ (3))


#define RPC_DESC_BT_UMS_EVENT_CONNECTION_STATE_DATA  (__rpc_get_leaudio_ums_desc__ (4))


#define RPC_DESC_BT_UMS_EVENT_GROUP_NODE_STATUS_DATA  (__rpc_get_leaudio_ums_desc__ (5))


#define RPC_DESC_BT_UMS_EVENT_GROUP_STATUS_DATA  (__rpc_get_leaudio_ums_desc__ (6))


#define RPC_DESC_BT_UMS_EVENT_SOCKET_INDEX_DATA  (__rpc_get_leaudio_ums_desc__ (7))


#define RPC_DESC_BT_UMS_EVENT_AUDIO_CONF_DATA  (__rpc_get_leaudio_ums_desc__ (8))


#define RPC_DESC_BT_UMS_EVENT_SET_MEMBER_AVAILABLE_DATA  (__rpc_get_leaudio_ums_desc__ (9))


#define RPC_DESC_BT_UMS_EVENT_GROUP_LOCK_CHANGED_DATA  (__rpc_get_leaudio_ums_desc__ (10))


#define RPC_DESC_BT_UMS_EVENT_DATA  (__rpc_get_leaudio_ums_desc__ (11))


#define RPC_DESC_BT_UMS_EVENT_PARAM  (__rpc_get_leaudio_ums_desc__ (12))


#define RPC_DESC_BT_UMS_CONNECT_PARAM  (__rpc_get_leaudio_ums_desc__ (13))


#define RPC_DESC_BT_UMS_DISCONNECT_PARAM  (__rpc_get_leaudio_ums_desc__ (14))


#define RPC_DESC_BT_UMS_GROUP_START_PARAM  (__rpc_get_leaudio_ums_desc__ (15))


#define RPC_DESC_BT_UMS_GROUP_START_EXT_PARAM  (__rpc_get_leaudio_ums_desc__ (16))


#define RPC_DESC_BT_UMS_GROUP_STOP_PARAM  (__rpc_get_leaudio_ums_desc__ (17))


#define RPC_DESC_BT_UMS_UPDATE_METADATA_PARAM  (__rpc_get_leaudio_ums_desc__ (18))


#define RPC_DESC_BT_UMS_GROUP_LOCK_SET_PARAM  (__rpc_get_leaudio_ums_desc__ (19))



extern const RPC_DESC_T* __rpc_get_leaudio_ums_desc__ (UINT32  ui4_idx);


#endif

