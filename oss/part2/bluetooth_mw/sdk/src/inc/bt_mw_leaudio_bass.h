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

#ifndef _BT_MW_LEAUDIO_BASS_H_
#define _BT_MW_LEAUDIO_BASS_H_

#include "bt_mw_common.h"
#include "u_bt_mw_leaudio_bass.h"

/*****************************************************************************
**  Callback Event used in tBTMW_MSG.BTMW_HDR.event
*****************************************************************************/
enum
{
    BTMW_BASS_CONNECTION_CB_EVT = BTMW_EVT_START(BTWM_ID_LEAUDIO_BASS),
    BTMW_BASS_DEVICE_AVAIL_CB_EVT,
    BTMW_BASS_BROADCAST_SCANNING_CHANGED_CB_EVT,
    BTMW_BASS_BROADCAST_ANNOUNCE_RECV_CB_EVT,
    BTMW_BASS_BROADCAST_RECV_STATE_CB_EVT,
    BTMW_BASS_BUILTIN_MODE_CHANGED_CB_EVT,
    BTMW_BASS_SYNC_LOST_CB_EVT,
};

typedef struct
{
    BT_BASS_EVENT_DATA    data;
} tBT_MW_BASS_MSG;

INT32 bluetooth_bass_connect(BT_BASS_CONN_PARAM *param);
INT32 bluetooth_bass_disconnect(BT_BASS_DISC_PARAM *param);
INT32 bluetooth_bass_set_broadcast_scan(BT_BASS_BROADCAST_SCAN_PARAM *param);
INT32 bluetooth_bass_stop_broadcast_observing(VOID);
INT32 bluetooth_bass_get_broadcast_receiver_state(BT_BASS_BROADCAST_RECV_STATE_PARAM *param);
INT32 bluetooth_bass_set_broadcast_code(BT_BASS_BROADCAST_CODE_PARAM *param);
INT32 bluetooth_bass_set_broadcast_source(BT_BASS_SET_BROADCAST_SRC_PARAM *param);
INT32 bluetooth_bass_modify_broadcast_source(BT_BASS_MODIFY_BROADCAST_SRC_PARAM *param);
INT32 bluetooth_bass_remove_broadcast_source(BT_BASS_REMOVE_BROADCAST_SRC_PARAM *param);
INT32 bluetooth_bass_set_builtin_mode(BT_BASS_SET_BUILTIN_MODE_PARAM *param);
INT32 bluetooth_bass_get_connection_status(BT_BASS_CONNECTION_INFO *conn_info);
INT32 bluetooth_bass_register_callback(BT_BASS_EVENT_HANDLE_CB bass_handle);

#endif /* _BT_MW_LEAUDIO_BASS_H_ */
