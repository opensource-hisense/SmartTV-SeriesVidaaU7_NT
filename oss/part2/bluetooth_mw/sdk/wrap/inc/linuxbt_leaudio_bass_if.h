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

#ifndef __LINUXBT_LEAUDIO_BASS_IF_H__
#define __LINUXBT_LEAUDIO_BASS_IF_H__

#include "bt_mw_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BASS_MAX_METADATA_LEN (251)
#define BASS_MAX_SUBGROUP_NUM (4)

typedef struct
{
    UINT32 sync_bis;
    UINT8 meta_data_len;
    UINT8 meta_data[BASS_MAX_METADATA_LEN];
} BASS_BROADCAST_SOURCE_SUBGROUP;

typedef struct
{
    UINT8 sync_pa;
    UINT8 num_subgroups;
    BASS_BROADCAST_SOURCE_SUBGROUP subgroup[BASS_MAX_SUBGROUP_NUM];
} BASS_BROADCAST_SOURCE;

int linuxbt_bass_init(void);
int linuxbt_bass_deinit(void);
int linuxbt_bass_connect(char *bt_addr);
int linuxbt_bass_disconnect(char *bt_addr);
int linuxbt_bass_set_broadcast_scan(char *bt_addr, bool scan, UINT8 duration);
int linuxbt_bass_stop_broadcast_observing(void);
int linuxbt_bass_get_broadcast_receiver_state(char *bt_addr, int receiver_id);
int linuxbt_bass_set_broadcast_code(char *bt_addr, int receiver_id, UINT8 *code);
int linuxbt_bass_set_broadcast_source(char *bt_addr, char *adv_addr, UINT8 addr_type, UINT8 adv_sid, BASS_BROADCAST_SOURCE *data);
int linuxbt_bass_modify_broadcast_source(char *bt_addr, int receiver_id, BASS_BROADCAST_SOURCE *data);
int linuxbt_bass_remove_broadcast_source(char *bt_addr, int receiver_id);
int linuxbt_bass_set_builtin_mode(char *bt_addr, bool enable, UINT32 sync_bis);

#ifdef __cplusplus
}
#endif

#endif /* __LINUXBT_LEAUDIO_BASS_IF_H__ */
