/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Feb 15 17:47:52 2022'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_AVRCP_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_AVRCP_IPCRPC_STRUCT__H_




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


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_avrcp_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_avrcp_desc__ (1))


#define RPC_DESC_BT_AVRCP_MEDIA_INFO  (__rpc_get_avrcp_desc__ (2))


#define RPC_DESC_BT_AVRCP_PLAYER_SETTING  (__rpc_get_avrcp_desc__ (3))


#define RPC_DESC_BT_AVRCP_PLAYITEM  (__rpc_get_avrcp_desc__ (4))


#define RPC_DESC_BT_AVRCP_LIST_RANGE  (__rpc_get_avrcp_desc__ (5))


#define RPC_DESC_BT_AVRCP_FOLDER_PATH  (__rpc_get_avrcp_desc__ (6))


#define RPC_DESC_BT_AVRCP_PLAYERSETTING_CHANGE  (__rpc_get_avrcp_desc__ (7))


#define RPC_DESC_BT_AVRCP_PLAYERSETTING_RSP  (__rpc_get_avrcp_desc__ (8))


#define RPC_DESC_BT_AVRCP_PLAYER_APP_EXT_ATTR_VAL  (__rpc_get_avrcp_desc__ (9))


#define RPC_DESC_BT_AVRCP_PLAYER_APP_EXT_ATTR  (__rpc_get_avrcp_desc__ (10))


#define RPC_DESC_BT_AVRCP_PLAYER_APP_ATTR  (__rpc_get_avrcp_desc__ (11))


#define RPC_DESC_BT_AVRCP_LIST_PLAYERSETTING_RSP  (__rpc_get_avrcp_desc__ (12))


#define RPC_DESC_BT_AVRCP_ADDR_PLAYER  (__rpc_get_avrcp_desc__ (13))


#define RPC_DESC_BT_AVRCP_CONNECTION_CB  (__rpc_get_avrcp_desc__ (14))


#define RPC_DESC_BT_AVRCP_SET_VOL_REQ  (__rpc_get_avrcp_desc__ (15))


#define RPC_DESC_BT_AVRCP_TRACK_CHANGE  (__rpc_get_avrcp_desc__ (16))


#define RPC_DESC_BT_AVRCP_POS_CHANGE  (__rpc_get_avrcp_desc__ (17))


#define RPC_DESC_BT_AVRCP_PLAY_STATUS_CHANGE  (__rpc_get_avrcp_desc__ (18))


#define RPC_DESC_BT_AVRCP_ABS_SUPPORTED  (__rpc_get_avrcp_desc__ (19))


#define RPC_DESC_BT_AVRCP_VOLUME_CHANGE  (__rpc_get_avrcp_desc__ (20))


#define RPC_DESC_BT_AVRCP_PASSTHROUGH_CMD_REQ  (__rpc_get_avrcp_desc__ (21))


#define RPC_DESC_BT_AVRCP_BATTERY_STATUS_CHANGE  (__rpc_get_avrcp_desc__ (22))


#define RPC_DESC_BT_AVRCP_FEATURE_RSP  (__rpc_get_avrcp_desc__ (23))


#define RPC_DESC_BT_AVRCP_FEATURE  (__rpc_get_avrcp_desc__ (24))


#define RPC_DESC_BT_AVRCP_ELEMENT_ATTR_VAL  (__rpc_get_avrcp_desc__ (25))


#define RPC_DESC_BT_AVRCP_ITEM_PLAYER  (__rpc_get_avrcp_desc__ (26))


#define RPC_DESC_BT_AVRCP_ITEM_FOLDER  (__rpc_get_avrcp_desc__ (27))


#define RPC_DESC_BT_AVRCP_ITEM_MEDIA  (__rpc_get_avrcp_desc__ (28))


#define RPC_DESC_BT_AVRCP_FOLDER_ITEMS  (__rpc_get_avrcp_desc__ (29))


#define RPC_DESC_BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA  (__rpc_get_avrcp_desc__ (30))


#define RPC_DESC_BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA  (__rpc_get_avrcp_desc__ (31))


#define RPC_DESC_BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA  (__rpc_get_avrcp_desc__ (32))


#define RPC_DESC_BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA  (__rpc_get_avrcp_desc__ (33))


#define RPC_DESC_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA  (__rpc_get_avrcp_desc__ (34))


#define RPC_DESC_BT_AVRCP_GET_FLODER_ITEMS_CB  (__rpc_get_avrcp_desc__ (35))


#define RPC_DESC_BT_AVRCP_CHANGE_FOLDER_PATH_CB  (__rpc_get_avrcp_desc__ (36))


#define RPC_DESC_BT_AVRCP_SET_BROWSED_PLAYER_CB  (__rpc_get_avrcp_desc__ (37))


#define RPC_DESC_BT_AVRCP_SET_ADDRESSED_PLAYER_CB  (__rpc_get_avrcp_desc__ (38))


#define RPC_DESC_BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB  (__rpc_get_avrcp_desc__ (39))


#define RPC_DESC_BT_AVRCP_EVENT_DATA  (__rpc_get_avrcp_desc__ (40))


#define RPC_DESC_BT_AVRCP_EVENT_PARAM  (__rpc_get_avrcp_desc__ (41))


#define RPC_DESC_BT_AVRCP_PLAYER_STATUS  (__rpc_get_avrcp_desc__ (42))


#define RPC_DESC_BT_AVRCP_PLAYER_MEDIA_INFO  (__rpc_get_avrcp_desc__ (43))


#define RPC_DESC_BT_AVRCP_PLAYER_MEDIA_INFO_LIST  (__rpc_get_avrcp_desc__ (44))


#define RPC_DESC_BT_AVRCP_CONNECTED_DEV_INFO  (__rpc_get_avrcp_desc__ (45))


#define RPC_DESC_BT_AVRCP_CONNECTED_DEV_INFO_LIST  (__rpc_get_avrcp_desc__ (46))



extern const RPC_DESC_T* __rpc_get_avrcp_desc__ (UINT32  ui4_idx);


#endif

