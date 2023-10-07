/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Jan 25 21:11:33 2022'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_LEAUDIO_BASS_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_LEAUDIO_BASS_IPCRPC_STRUCT__H_




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


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_leaudio_bass_desc__ (0))


#define RPC_DESC_BTMW_EXT_FEATURES  (__rpc_get_leaudio_bass_desc__ (1))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_leaudio_bass_desc__ (2))


#define RPC_DESC_BT_BASS_BROADCAST_SRC_SUBGROUP  (__rpc_get_leaudio_bass_desc__ (3))


#define RPC_DESC_BT_BASS_BROADCAST_SRC  (__rpc_get_leaudio_bass_desc__ (4))


#define RPC_DESC_BT_BASS_BROADCAST_RECV  (__rpc_get_leaudio_bass_desc__ (5))


#define RPC_DESC_BT_BASS_BROADCAST_ID  (__rpc_get_leaudio_bass_desc__ (6))


#define RPC_DESC_BT_BASS_LC3_CODEC_CONFIG  (__rpc_get_leaudio_bass_desc__ (7))


#define RPC_DESC_BT_BASS_ANNOUNCE_CODEC_CONFIG  (__rpc_get_leaudio_bass_desc__ (8))


#define RPC_DESC_BT_BASS_ANNOUNCE_BIS_CONFIG  (__rpc_get_leaudio_bass_desc__ (9))


#define RPC_DESC_BT_BASS_ANNOUNCE_SUBGROUP  (__rpc_get_leaudio_bass_desc__ (10))


#define RPC_DESC_BT_BASS_ANNOUNCE_DATA  (__rpc_get_leaudio_bass_desc__ (11))


#define RPC_DESC_BT_BASS_CONN_PARAM  (__rpc_get_leaudio_bass_desc__ (12))


#define RPC_DESC_BT_BASS_DISC_PARAM  (__rpc_get_leaudio_bass_desc__ (13))


#define RPC_DESC_BT_BASS_BROADCAST_SCAN_PARAM  (__rpc_get_leaudio_bass_desc__ (14))


#define RPC_DESC_BT_BASS_BROADCAST_RECV_STATE_PARAM  (__rpc_get_leaudio_bass_desc__ (15))


#define RPC_DESC_BT_BASS_BROADCAST_CODE_PARAM  (__rpc_get_leaudio_bass_desc__ (16))


#define RPC_DESC_BT_BASS_SET_BROADCAST_SRC_PARAM  (__rpc_get_leaudio_bass_desc__ (17))


#define RPC_DESC_BT_BASS_MODIFY_BROADCAST_SRC_PARAM  (__rpc_get_leaudio_bass_desc__ (18))


#define RPC_DESC_BT_BASS_REMOVE_BROADCAST_SRC_PARAM  (__rpc_get_leaudio_bass_desc__ (19))


#define RPC_DESC_BT_BASS_SET_BUILTIN_MODE_PARAM  (__rpc_get_leaudio_bass_desc__ (20))


#define RPC_DESC_BT_BASS_CONNECTED_DEVICE  (__rpc_get_leaudio_bass_desc__ (21))


#define RPC_DESC_BT_BASS_CONNECTION_INFO  (__rpc_get_leaudio_bass_desc__ (22))


#define RPC_DESC_BT_BASS_EVENT_CONN_STATE_DATA  (__rpc_get_leaudio_bass_desc__ (23))


#define RPC_DESC_BT_BASS_EVENT_DEV_AVAIL_DATA  (__rpc_get_leaudio_bass_desc__ (24))


#define RPC_DESC_BT_BASS_EVENT_SCANNING_STATE_DATA  (__rpc_get_leaudio_bass_desc__ (25))


#define RPC_DESC_BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA  (__rpc_get_leaudio_bass_desc__ (26))


#define RPC_DESC_BT_BASS_EVENT_RECV_STATE_DATA  (__rpc_get_leaudio_bass_desc__ (27))


#define RPC_DESC_BT_BASS_EVENT_BUILTIN_MODE_DATA  (__rpc_get_leaudio_bass_desc__ (28))


#define RPC_DESC_BT_BASS_EVENT_SYNC_LOST_DATA  (__rpc_get_leaudio_bass_desc__ (29))


#define RPC_DESC_BT_BASS_EVENT_DATA  (__rpc_get_leaudio_bass_desc__ (30))


#define RPC_DESC_BT_BASS_EVENT_PARAM  (__rpc_get_leaudio_bass_desc__ (31))



extern const RPC_DESC_T* __rpc_get_leaudio_bass_desc__ (UINT32  ui4_idx);


#endif

