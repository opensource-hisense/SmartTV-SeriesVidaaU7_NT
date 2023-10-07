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


#ifndef _MTK_BT_SERVICE_A2DP_H_
#define _MTK_BT_SERVICE_A2DP_H_

#include "mtk_bt_service_a2dp_wrapper.h"

INT32 x_mtkapi_a2dp_connect(CHAR *addr, BT_A2DP_ROLE role);
INT32 x_mtkapi_a2dp_disconnect(char *addr);
INT32 x_mtkapi_a2dp_register_callback(mtkrpcapi_BtAppA2dpCbk func, void *pv_tag);
INT32 x_mtkapi_a2dp_unregister_callback(VOID);
INT32 x_mtkapi_a2dp_get_connected_dev_list(BT_A2DP_CONNECT_DEV_INFO_LIST *dev_list);

INT32 x_mtkapi_a2dp_sink_adjust_buf_time(UINT32 buffer_time);
INT32 x_mtkapi_a2dp_sink_enable(BOOL enable);

INT32 x_mtkapi_a2dp_sink_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list);
INT32 x_mtkapi_a2dp_src_enable(BOOL enable, BT_A2DP_SRC_INIT_CONFIG *p_src_init_config);

INT32 x_mtkapi_a2dp_src_config_codec_info(CHAR *addr, BT_A2DP_SET_CODEC_CONFIG *p_src_set_codec_config);

INT32 x_mtkapi_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list);
INT32 x_mtkapi_a2dp_codec_enable(INT32 codec_type, BOOL enable);
INT32 x_mtkapi_a2dp_src_set_audiomode(INT32 audio_mode);
INT32 x_mtkapi_a2dp_src_set_channel_allocation_for_lrmode(CHAR *addr, INT32 channel);
INT32 x_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag, BT_A2DP_DBG_PARAM *param);
INT32 x_mtkapi_a2dp_sink_active_src(CHAR *addr);
INT32 x_mtkapi_a2dp_sink_set_delay_value(CHAR *addr, UINT16 value);
INT32 x_mtkapi_a2dp_sink_get_stack_delay(VOID);
//INT32 x_mtkapi_a2dp_sink_player_load(CHAR* player_so_path);
//INT32 x_mtkapi_a2dp_sink_player_unload(CHAR *player_name);
INT32 x_mtkapi_a2dp_src_active_sink(CHAR *addr);
INT32 x_mtkapi_a2dp_src_get_active_sink(CHAR *addr);
INT32 x_mtkapi_a2dp_sink_get_active_src(CHAR *addr);
INT32 x_mtkapi_a2dp_src_set_silence_device(CHAR *addr, BOOL enable);
BOOL x_mtkapi_a2dp_src_is_in_silence_mode(CHAR *addr);
//INT32 x_mtkapi_a2dp_src_pause_uploader(VOID *param);
//INT32 x_mtkapi_a2dp_src_resume_uploader(VOID *param);
//INT32 x_mtkapi_a2dp_src_mute_uploader(BOOL mute);
//INT32 x_mtkapi_a2dp_src_uploader_load(CHAR* uploader_so_path);
//INT32 x_mtkapi_a2dp_src_uploader_unload(CHAR *uploader_name);
INT32 x_mtkapi_a2dp_sink_set_link_num(INT32 sink_num);
INT32 x_mtkapi_a2dp_sink_lowpower_enable(BOOL enable);
#endif
