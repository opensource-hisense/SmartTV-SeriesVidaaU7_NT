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

/* FILE NAME:  bt_mw_a2dp_src.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

#include "c_mw_config.h"
#include "bt_mw_common.h"
#include "bt_mw_a2dp_common.h"
#include "bt_mw_a2dp_src.h"
#include "linuxbt_a2dp_src_if.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
static INT32 bt_mw_a2dp_src_get_start_delay(VOID);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
#else
static BOOL g_A2DP_connected = FALSE;
#endif

#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
//static BT_A2DP_UPLOADER bt_mw_a2dp_src_uploader = {0};
typedef struct
{
    VOID *dlhandle;
    BT_A2DP_UPLOADER uploader;
} BT_MW_A2DP_SRC_UPLOADER;

static BT_MW_A2DP_SRC_UPLOADER g_bt_mw_a2dp_src_uploaders = {0};
#endif
/* EXPORTED SUBPROGRAM BODIES
 */

INT32 bt_mw_a2dp_src_register_uploader(BT_A2DP_UPLOADER *uploader)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    if (NULL == uploader)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "callback uploader is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "uploader=%s", uploader->name);
    memset(&g_bt_mw_a2dp_src_uploaders.uploader, 0x0, sizeof(BT_A2DP_UPLOADER));

    g_bt_mw_a2dp_src_uploaders.uploader = *uploader;

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_send_stream_data(const CHAR *data, INT32 len)
{
    return BT_ERR_STATUS_UNSUPPORTED;
}

INT32 bt_mw_a2dp_src_uploader_load(CHAR* uploader_so_path)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "uploader_so_path=%s", uploader_so_path);

    if (g_A2DP_connected == TRUE)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "A2DP is connected!");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_bt_mw_a2dp_src_uploaders.dlhandle)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "so already loaded!");
        return BT_ERR_STATUS_BUSY;
    }

    g_bt_mw_a2dp_src_uploaders.dlhandle = dlopen(uploader_so_path, RTLD_LAZY);
    if (!g_bt_mw_a2dp_src_uploaders.dlhandle)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "dlopen fail(%s)", dlerror());
        return BT_ERR_STATUS_FAIL;
    }
    BT_A2DP_UPLOADER_MODULE *p_module = dlsym(g_bt_mw_a2dp_src_uploaders.dlhandle, "UPLOADER_MODULE_INFO_SYM");
    if (dlerror() != NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "lib.uploader.so get symbol fail(%s)", dlerror());
        dlclose(g_bt_mw_a2dp_src_uploaders.dlhandle);
        g_bt_mw_a2dp_src_uploaders.dlhandle = NULL;
        return BT_ERR_STATUS_FAIL;
    }
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "BT sink uploader lib open success!");

    if (BT_SUCCESS != bt_mw_a2dp_src_register_uploader(p_module->methods->get_uploader()))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "lib.uploader.so get symbol fail(%s)", dlerror());
        dlclose(g_bt_mw_a2dp_src_uploaders.dlhandle);
        g_bt_mw_a2dp_src_uploaders.dlhandle = NULL;
        return BT_ERR_STATUS_NOMEM;
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_uploader_unload(CHAR *uploader_name)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
        return BT_ERR_STATUS_UNSUPPORTED;
#else
    if (g_A2DP_connected == TRUE)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "A2DP is connected!");
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "uploader_name=%s", uploader_name);
    if (!g_bt_mw_a2dp_src_uploaders.dlhandle ||
        strncmp(uploader_name, g_bt_mw_a2dp_src_uploaders.uploader.name, strlen(g_bt_mw_a2dp_src_uploaders.uploader.name)) != 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "dlhandle:%p,name_in:%s,name_exist:%s)", g_bt_mw_a2dp_src_uploaders.dlhandle,
            uploader_name,
            g_bt_mw_a2dp_src_uploaders.uploader.name);
        BT_DBG_ERROR(BT_DEBUG_A2DP, "name_exist len:%d", strlen(g_bt_mw_a2dp_src_uploaders.uploader.name));
        return BT_ERR_STATUS_BUSY;
    }
    dlclose(g_bt_mw_a2dp_src_uploaders.dlhandle);
    g_bt_mw_a2dp_src_uploaders.dlhandle = NULL;
    memset(&g_bt_mw_a2dp_src_uploaders.uploader, 0x0, sizeof(BT_A2DP_UPLOADER));

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_start_uploader(UINT32 freq, UINT8 channel)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    INT32 delay_ms = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "freq=%d, channel=%u", freq, channel);

    g_A2DP_connected = TRUE;

    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.init)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.init(freq, channel);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_init_cb func is null!");
    }
    delay_ms = bt_mw_a2dp_src_get_start_delay();
    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.start)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.start(delay_ms);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_start_cb func is null!");
    }
    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_stop_uploader(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    g_A2DP_connected = FALSE;

    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.stop)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.stop();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_stop_cb func is null!");
    }
    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.deinit)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.deinit();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_deinit_cb func is null!");
    }

    return BT_SUCCESS;
#endif
}


INT32 bt_mw_a2dp_src_pause_uploader(VOID *param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    /*add A2DP SRC handle*/
    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.pause)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.pause(param);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "pause func is null!");
    }
    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_resume_uploader(VOID *param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    /*add A2DP SRC handle*/
    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.resume)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.resume(param);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "resume func is null!");
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_mute_uploader(BOOL mute)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    /*add A2DP SRC handle*/
    if (NULL != g_bt_mw_a2dp_src_uploaders.uploader.mute)
    {
        g_bt_mw_a2dp_src_uploaders.uploader.mute(mute);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "mute func is null!");
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_set_audio_log_lv(UINT8 log_level)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    return linuxbt_a2dp_set_audio_hw_log_lvl(log_level);
#endif
}


/* LOCAL SUBPROGRAM BODIES
 */

#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV

static INT32 bt_mw_a2dp_src_get_start_delay(VOID)
{
    /* SBH50 is no sound when 1st connection and need wait 3s to send AVDTP_START */
    /* In case
     * 1. 1st connection
     * 2. send data after connected(on phone, play music before connecting.
     */
    if (0 == strncmp(/*g_bt_target_dev_info.name*/"a", "SBH50", 6))
    {
        return 3000;
    }

    return 0;
}
#endif




