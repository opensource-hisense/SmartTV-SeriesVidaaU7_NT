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


#include "c_bt_mw_a2dp_common.h"
#include "c_bt_mw_a2dp_src.h"
#include "bt_mw_a2dp_common.h"
#include "bt_mw_a2dp_src.h"

/* FUNCTION NAME: c_btm_a2dp_src_get_dev_list
 * PURPOSE:
 *      it is used to get sink device list which have connected before. The
 *  lastest device is the last one.
 * INPUT:
 *      N/A
 * OUTPUT:
 *      dev_list  -- the sink device list
 * RETURN:
 *      BT_SUCCESS  -- get successfully
 *      others      -- get fail
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    BT_CHECK_POINTER(BT_DEBUG_A2DP, dev_list);
    return bt_mw_a2dp_src_get_dev_list(dev_list);
}


/* FUNCTION NAME: c_btm_a2dp_src_send_stream_data
 * PURPOSE:
 *      it is used to send PCM data to BT A2DP sink device.
 * INPUT:
 *      data  -- PCM data
 *      len   -- PCM data length
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- send successfully
 *      others      -- send fail
 * NOTES:
 *      this function is used to send PCM data by BT MW. If user use BT audio
 *  device to send PCM data, this function should not be used.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_send_stream_data(const CHAR *data, UINT32 len)
{
    BT_CHECK_POINTER(BT_DEBUG_A2DP, data);
    return bt_mw_a2dp_src_send_stream_data(data, len);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_active_sink(CHAR *addr)
{
    return bt_mw_a2dp_src_active_sink(addr);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_set_silence_device(CHAR *addr, BOOL enable)
{
    return bt_mw_a2dp_src_set_silence_device(addr, enable);
}

EXPORT_SYMBOL BOOL c_btm_a2dp_src_is_in_silence_mode(CHAR *addr)
{
    return bt_mw_a2dp_src_is_in_silence_mode(addr);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_get_active_sink(CHAR *addr)
{
    return bt_mw_a2dp_get_active_device(addr);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_config_codec_info(CHAR *addr, BT_A2DP_SET_CODEC_CONFIG *p_src_set_codec_config)
{
    return  bt_mw_a2dp_src_config_codec_info(addr, p_src_set_codec_config);
}


/* FUNCTION NAME: c_btm_a2dp_src_register_uploader
 * PURPOSE:
 *      it is used to register a uploader to BT MW. The uploader is used to get
 *  PCM data and send it out to remote A2DP sink device.
 * INPUT:
 *      uploader  -- uploader
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- register successfully
 *      others      -- register fail
 * NOTES:
 *      If user use BT audio device to send PCM data,this function should not
 *  be used.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_register_uploader(BT_A2DP_UPLOADER *uploader)
{
    BT_CHECK_POINTER(BT_DEBUG_A2DP, uploader);
    return bt_mw_a2dp_src_register_uploader(uploader);
}

/* FUNCTION NAME: c_btm_a2dp_src_enable
 * PURPOSE:
 *      enable  -- TRUE: enable A2DP source function
 *                 FALSE: disable A2DP source function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- set successfully
 *      BT_ERR_STATUS_BUSY -- there is connection, pending it.
 *      others             -- set fail
 * NOTES:
 *      If there is no connection to sink device, disable operation will
 * return with BT_SUCCESS.
 *      If there is connections to source device, disable will pending and BT MW
 * will disconnect all A2DP sink connections. When all A2DP sink connectons
 * are disconnected, then BT MW will disable/enable source role and report
 * BT_A2DP_EVENT_ROLE_CHANGED event to APP.
 *      Please set scan mode as 0 to protect disable operation. After this, APP
 * can restore scan mode.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_enable(BOOL enable,
    BT_A2DP_SRC_INIT_CONFIG* p_src_init_config)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "enable=%u", enable);
    return bt_mw_a2dp_src_enable(enable, p_src_init_config);
}

/* FUNCTION NAME: c_btm_a2dp_src_pause_uploader
 * PURPOSE:
 *      pause uploader, then it will not send streaming data out.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- pause successfully
 *      BT_ERR_STATUS_UNSUPPORTED -- uploader is not enable
 *      others             -- pause fail
 * NOTES:
 *      N/A
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_pause_uploader(VOID *param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "param=%p", param);
    return bt_mw_a2dp_src_pause_uploader(param);
}

/* FUNCTION NAME: c_btm_a2dp_src_resume_uploader
 * PURPOSE:
 *      resume uploader, then it will send streaming data out.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- resume successfully
 *      BT_ERR_STATUS_UNSUPPORTED -- uploader is not enable
 *      others             -- resume fail
 * NOTES:
 *      N/A
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_src_resume_uploader(VOID *param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "param=%p", param);
    return bt_mw_a2dp_src_resume_uploader(param);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_mute_uploader(BOOL mute)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "mute=%d", mute);
    return bt_mw_a2dp_src_mute_uploader(mute);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_uploader_load(CHAR* uploader_so_path)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    return bt_mw_a2dp_src_uploader_load(uploader_so_path);
}

EXPORT_SYMBOL INT32 c_btm_a2dp_src_uploader_unload(CHAR *uploader_name) // "mtal_uploader"
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "uploader_name=%s", uploader_name);
    return bt_mw_a2dp_src_uploader_unload(uploader_name);
}
