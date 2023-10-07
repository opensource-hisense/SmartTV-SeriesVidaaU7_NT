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


#include "bt_mw_leaudio_bms.h"

EXPORT_SYMBOL INT32 c_btm_bt_bms_register_callback(BtAppBmsCbk bms_cb)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_LEAUDIO_BMS, "bms_cb=%p", bms_cb);
    return bt_mw_bms_register_callback(bms_cb);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
    return bt_mw_bms_create_broadcast(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
    return bt_mw_bms_create_broadcast_ext(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
    return bt_mw_bms_update_base_announcement(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
    return bt_mw_bms_update_subgroup_metadata(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
    return bt_mw_bms_start_broadcast(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
    return bt_mw_bms_start_broadcast_multi_thread(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
    return bt_mw_bms_pause_broadcast(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
    return bt_mw_bms_stop_broadcast(param);
}
EXPORT_SYMBOL INT32 c_btm_bt_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
    return bt_mw_bms_get_own_address(param);
}

EXPORT_SYMBOL INT32 c_btm_bt_bms_get_all_broadcasts_states(VOID)
{
    return bt_mw_bms_get_all_broadcasts_states();
}

EXPORT_SYMBOL INT32 c_btm_bt_bms_stop_all_broadcast(VOID)
{
    return bt_mw_bms_stop_all_broadcast();
}

