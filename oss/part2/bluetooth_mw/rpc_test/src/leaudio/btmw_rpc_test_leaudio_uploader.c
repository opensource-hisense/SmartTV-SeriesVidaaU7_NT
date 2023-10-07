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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
//#include <sys/time.h>
//btmw_rpc_test_leaudiohw_src_if.c
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <asoundlib.h>
#include <dlfcn.h>

#include "mtk_audio.h" //need refactor
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
//#include "mtk_bt_service_a2dp_wrapper.h"

#include "btmw_rpc_test_leaudiohw_uploader_if.h"
#include "aud_leaudio_uploader.h"
#include "x_aud_dec.h"
#include "mtauddec.h"
#include "drv_common.h"


#define BTMW_RPC_TEST_CMD_KEY_LEAUDIO_UPLOADER        "MW_RPC_LEAUDIOHW_UPL"
#define MAX_DATA_PATH 6

typedef struct upl_stream_test_t {
    bool in_use;
    audio_io_handle_t handle;
    long hUploaderhandle;
} upl_stream_test_t;

static struct upl_stream_test_t upl_str_test_t[MAX_DATA_PATH] = {{0}};
static pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;

static void upl_init_stream_test (void) {
    for (uint8_t i = 0; i < MAX_DATA_PATH; i++) {
        upl_str_test_t[i].in_use = false;
        upl_str_test_t[i].handle = i;
        upl_str_test_t[i].hUploaderhandle = 0;
    }
}

static struct upl_stream_test_t* get_stream_test_by_handle(audio_io_handle_t handle) {
    pthread_mutex_lock(&gmutex);
    struct upl_stream_test_t*  stt = NULL;
    if ((uint8_t)handle < MAX_DATA_PATH) {
        upl_str_test_t[(uint8_t)handle].in_use = true;
        stt = &(upl_str_test_t[(uint8_t)handle]);
    }

    BTMW_RPC_TEST_Logd("%s: Got by handle: %d.\n", __func__, handle);
    pthread_mutex_unlock(&gmutex);
    return stt;
}

// CLI handler
static int btmw_rpc_test_leaudio_upl_write_stream_handler(int argc, char *argv[]);
static int btmw_rpc_test_leaudio_upl_stop_stream_handler(int argc, char *argv[]);

static int btmw_rpc_test_leaudio_upl_write_stream_handler(int argc, char *argv[])
{
    struct upl_stream_test_t* in = NULL;
    long hUploaderhandle;
    audio_io_handle_t handle;
    int channels = 0;
    int i4_ret = 0;
    if (argc == 2)
    {
        handle = (audio_io_handle_t)atoi(argv[0]);
        channels = atoi(argv[1]);
        BTMW_RPC_TEST_Logd(" handle:%d, channels:%d.\n", handle, channels);
        in = get_stream_test_by_handle(handle);
    }
    else
    {
        BTMW_RPC_TEST_Logd(" Useage: write_stream <handle> <channel-count>\n");
        return -1;
    }

    if (LEaudio_uploader_init() != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader init fail.\n");
        return -1;
    }

    if (LEaudio_uploader_create(&hUploaderhandle, handle, 0, channels) != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader create fail.\n");
        return -1;
    }

    in->hUploaderhandle = hUploaderhandle;

    if (LEaudio_uploader_start(hUploaderhandle) != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader start fail.\n");
        LEaudio_uploader_stop(hUploaderhandle);
        LEaudio_uploader_close(hUploaderhandle);
        LEaudio_uploader_deinit();
        return -1;
    }

    // set bluetooth volume 100
    DRV_COMP_ID_T t_comp_id = { .ui2_id = 0};
    AUD_DEC_VOLUME_INFO_EX_T     t_aud_vol_info = {
        .u.ui1_level = 100,
        .e_vol_type = AUD_DEC_ALL_CH,
        .e_out_port = AUD_DEC_OUT_PORT_BLUETOOTH,
    };
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_VOLUME_EX, &t_aud_vol_info, sizeof(AUD_DEC_VOLUME_INFO_EX_T), &i4_ret);

    // 1st, get current out port type
    int ui4_drv_out_port_mask = 0;
    SIZE_T z_outportsize = sizeof(int);
    MTAUDDEC_AudGet(&t_comp_id, AUD_DEC_GET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, &z_outportsize, &i4_ret);

    // 2nd, add bluetooth type to out port type
    ui4_drv_out_port_mask |= AUD_DEC_OUT_PORT_FLAG_BLUETOOTH;
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, sizeof(int), &i4_ret);

    BTMW_RPC_TEST_Logw(" Exist!!!\n");
    return 0;
}

static int btmw_rpc_test_leaudio_upl_stop_stream_handler(int argc, char *argv[])
{
    audio_io_handle_t handle;
    int	i4_ret = 0;
    struct upl_stream_test_t* in = NULL;
    if (argc == 1) {
        handle = (audio_io_handle_t)atoi(argv[0]);
        BTMW_RPC_TEST_Logd(" handle:%d.\n", handle);
        in = get_stream_test_by_handle(handle);
        if (!in->in_use) {
            BTMW_RPC_TEST_Logw("this stream test not in use or not open success, just return!\n");
            return -1;
        }
    } else {
        BTMW_RPC_TEST_Logd(" Useage: pause_stream <handle>\n");
        return -1;
    }

    long hUploaderhandle = in->hUploaderhandle;
    if (hUploaderhandle == 0)
    {
        BTMW_RPC_TEST_Loge(" get Le Audio Uploader handle error!!!\n");
        return -1;
    }

    // 1st, get current out port type
    DRV_COMP_ID_T t_comp_id = { .ui2_id = 0};
    int ui4_drv_out_port_mask = 0;
    SIZE_T z_outportsize = sizeof(int);
    MTAUDDEC_AudGet(&t_comp_id, AUD_DEC_GET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, &z_outportsize, &i4_ret);

    // 2nd, remove bluetooth type to out port type
    ui4_drv_out_port_mask &= ~(AUD_DEC_OUT_PORT_FLAG_BLUETOOTH);
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, sizeof(int), &i4_ret);

    if (LEaudio_uploader_stop(hUploaderhandle) != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader stop fail.\n");
    }

    if (LEaudio_uploader_close(hUploaderhandle) != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader close fail.\n");
    }

    if (LEaudio_uploader_deinit() != 0)
    {
        BTMW_RPC_TEST_Loge(" Le Audio Uploader deinit fail.\n");
    }

    BTMW_RPC_TEST_Logw(" Exist!!!\n");
    return 0;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_leaudio_upl_cli_commands[] =
{
    {"write_stream", btmw_rpc_test_leaudio_upl_write_stream_handler, " = write_stream <handle> <channel-count>"},
    {"stop_stream",  btmw_rpc_test_leaudio_upl_stop_stream_handler,  " = stop_stream <handle>"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_leaudio_upl_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd(" argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = btmw_rpc_test_leaudio_upl_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTMW_RPC_TEST_Logd("Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_LEAUDIO_UPLOADER, btmw_rpc_test_leaudio_upl_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

INT32 btmw_rpc_test_leaudiohw_uploader_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD leaudio_upl_mod = {0};

    leaudio_upl_mod.mod_id = BTMW_RPC_TEST_MOD_LEAUDIO_UPLOADER;
    strncpy(leaudio_upl_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_LEAUDIO_UPLOADER, sizeof(leaudio_upl_mod.cmd_key));
    leaudio_upl_mod.cmd_handler = btmw_rpc_test_leaudio_upl_cmd_handler;
    leaudio_upl_mod.cmd_tbl = btmw_rpc_test_leaudio_upl_cli_commands;

    ret = btmw_rpc_test_register_mod(&leaudio_upl_mod);
    BTMW_RPC_TEST_Logd(" register leaudio uploader returns: %d\n", ret);
    upl_init_stream_test();
    return ret;
}


