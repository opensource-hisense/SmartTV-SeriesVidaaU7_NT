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
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <asoundlib.h>

#include "mtk_audio.h"
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_a2dp_src_if.h"
#include "mtk_bt_service_a2dp_wrapper.h"

#include <dlfcn.h>
#include "mtk_bt_service_leaudio_bms_wrapper.h"
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#include "aud_a2dp_uploader.h"
#include "x_aud_dec.h"
#include "mtauddec.h"
#include "drv_common.h"
#include "u_common.h"
#endif

#define BTMW_RPC_BMS_CASE_RETURN_STR(const) case const: return #const;
#define BTMW_RPC_A2DP_CASE_RETURN_STR(const) case const: return #const;
#define BTMW_RPC_TEST_CMD_KEY_A2DP_SRC        "MW_RPC_A2DP_SRC"
#define PCM_BUFFER_SIZE 512*8     // at least 512
/* 0:pause, 1: play */
struct audio_hw_device *gp_rpc_audio_dev = NULL;
static struct audio_stream_out *g_rpc_stream_out = NULL;
struct audio_module *g_rpc_audio_module;

static pthread_t btmw_rpc_test_stream_handle = -1;
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
static const char a2dphw_so_patch[128] = "/data/bluedroid/lib/libaudio.a2dp.default.so";
static CHAR btmw_rpc_test_pcm_file[128] = "/data/usb/music/48000/input_48000.pcm";
static volatile struct hw_device_t* gdevice = NULL;
static void *handle = NULL;  // handel for dlopen libaudio.a2dp.default.so
#else
static void *a2dp_hw_module_handle = NULL;
static CHAR btmw_rpc_test_pcm_file[128] = "/tmp/input_48000.pcm";
#endif
static INT32 btmw_rpc_test_sample_rate = 48000;
static INT32 btmw_rpc_test_bitdepth = 0;
static INT32 btmw_rpc_test_samplebytes = 0;

static INT32 g_bt_src_stream_is_suspend = 0;
static INT32  g_fg_audio_path_runing = 0;

//dlopen 3 shared lib for update conn state to app
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#define MAX_NUM_HANDLES 4096

static void *fHandle1 = NULL;
static void *fHandle2 = NULL;
static void *app_handle = NULL;
static INT32 a2dp_binit = 0;

INT32 (*a2dp_c_rpc_init_client)(VOID);
INT32 (*a2dp_c_rpc_start_client)(VOID);
INT32 (*a2dp_os_init)(const VOID *pv_addr, UINT32 z_size);
INT32 (*a2dp_handle_init) (UINT16   ui2_num_handles,
                 VOID**   ppv_mem_addr,
                 UINT32*  pz_mem_size);
INT32 (*a2dp_x_rtos_init) (GEN_CONFIG_T*  pt_config);

typedef INT32 (*dl_a_mtktvapi_bluetooth_agent_update_conn_state_func)(BOOL b_connected);
dl_a_mtktvapi_bluetooth_agent_update_conn_state_func dl_a_mtktvapi_bluetooth_agent_update_conn_state = NULL;
#endif
extern int adev_open(const hw_module_t* module, const char* name,
                         hw_device_t** device); //need refactor

extern CHAR g_rpc_a2dp_addr_test[18];
extern BT_A2DP_STREAM_STATE g_rpc_a2dp_stream_state;

// for auto test connect and disconnect
#define MAX_CONENCT_DISCONENCT_INTERVAL (3*1000)
extern CHAR g_rpc_a2dp_connect_test_addr[18];
extern int connect_state;
static pthread_t g_a2dp_auto_test_thread = 0;

#if defined(MTK_BT_PLAYBACK_DEFAULT_ALSA)
#define ALSA_DEVICE_PLAYER "default" /* for MTK branch tree */
#else
#define ALSA_DEVICE_PLAYER "main"
#endif
#define FRAGMENT_SAMPLES    (4096*4)

#define BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(const) case const: return #const;

static snd_pcm_t *s_alsa_handle = NULL;

static snd_pcm_uframes_t chunk_size = 0;

static UINT32 u4buffer_time = 0; /* ring buffer length in us */
static UINT32 u4period_time = 0; /* period time in us */

static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static INT32 i4avail_min = -1;
static INT32 i4start_delay = 200000;
static INT32 i4stop_delay = 0;
static BT_A2DP_SRC_INIT_CONFIG src_init_config;
static INT32 atuo_test_cnt = 0;
static INT32 atuo_connecet_thread_exit = 0;

struct a2dp_leaudio_uploader_config_t auc = {0};
extern struct socketidx_map_uploaderhdl_t smu_t[BT_BROADCAST_SOCKET_INDEX_NUM];

// CLI handler
static int btmw_rpc_test_a2dp_src_connect_int_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_disconnect_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_active_handler(int argc, char *argv[]);
static int  btmw_rpc_test_a2dp_src_set_init_codec_info(int argc, char *argv[]);
static int  btmw_rpc_test_a2dp_src_config_codec_info(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_write_stream_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[]);


#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */
static int load_bt_a2dp_module(const char path[], audio_hw_device_t **dev)
{
    if (gdevice != NULL) {
        BTMW_RPC_TEST_Logd("already got leaudio lib device!!");
        return 0;
    }
    int status = -EINVAL;
    //void *handle = NULL;
    struct hw_module_t *hmi = NULL;
    //const hw_module_t *mod;
    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    if (handle == NULL) {
        handle = dlopen(path, RTLD_LAZY);
        if (handle == NULL) {
            char const *err_str = dlerror();
            BTMW_RPC_TEST_Logd("load: module=%s\n%s", path, err_str?err_str:"unknown");
            status = -EINVAL;
            goto done;
        }
    }

    /* Get the address of the struct hal_module_info. */
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(handle, sym);
    if (hmi == NULL) {
        BTMW_RPC_TEST_Logd("load: couldn't find symbol %s", sym);
        status = -EINVAL;
        goto done;
    }

    /* Check that the id matches */
    if (strcmp(AUDIO_HARDWARE_MODULE_ID, hmi->id) != 0) {
        BTMW_RPC_TEST_Logd("load: id=%s != hmi->id=%s", AUDIO_HARDWARE_MODULE_ID, hmi->id);
        status = -EINVAL;
        goto done;
    }

    hmi->dso = handle;

    /* success */
    status = 0;

done:
    if (status != 0) {
        hmi = NULL;
        if (handle != NULL) {
            dlclose(handle);
            handle = NULL;
        }
    } else {
        BTMW_RPC_TEST_Logd("loaded HAL id=%s path=%s hmi=%p handle=%p",
                AUDIO_HARDWARE_MODULE_ID, path, hmi, handle);
    }

    status = audio_hw_device_open(hmi, dev);
    if (status) {
        BTMW_RPC_TEST_Logd("%s couldn't open audio leaudio hw device ", __func__);
        goto out;
    }
    if ((*dev)->common.version < AUDIO_DEVICE_API_VERSION_MIN) {
        BTMW_RPC_TEST_Logd("%s wrong audio hw device version %04x", __func__, (*dev)->common.version);
        status = -1;
        audio_hw_device_close(*dev);
        goto out;
    }
    return 0;

out:
    *dev = NULL;
    return status;
}
#endif

static CHAR* btmw_rpc_test_a2dp_src_app_role(BT_A2DP_ROLE local_role)
{
    switch((int)local_role)
    {
        BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(BT_A2DP_ROLE_SRC)
        BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(BT_A2DP_ROLE_SINK)
        default: return "UNKNOWN_ROLE";
   }
}

UINT32 bt_get_microseconds(VOID)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000000 + now.tv_nsec / 1000);
}

static int btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[])
{
    BT_A2DP_DBG_PARAM param;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] audio_hw_log <0~6>");
        return -1;
    }

    param.hw_audio_log_lv = atoi(argv[0]);
    if (0 <= param.hw_audio_log_lv && 6 >= param.hw_audio_log_lv)
    {
        a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV, &param);
    }
    else
    {
        BTMW_RPC_TEST_Loge("input error\n");
        BTMW_RPC_TEST_Loge("please input audio_hw_log <0~6>\n");
    }
    return 0;
}

static int btmw_rpc_test_a2dp_src_show_dev_info_handler(int argc, char *argv[])
{
    BT_A2DP_DBG_PARAM param;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_SHOW_INFO, &param);

    return 0;
}

static int btmw_rpc_test_a2dp_src_set_init_codec_info(int argc, char *argv[])
{
    UINT8 test_num = 0;

    memset(&src_init_config, 0, sizeof(BT_A2DP_SRC_INIT_CONFIG));
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_init_codec_info test <num>");
        return -1;
    }

    if (strcmp(argv[0],"test") == 0)
    {
        test_num = atoi(argv[1]);
    }
    // test 1: only sbc ,codec_priority : default
    if (test_num == 1)
    {
        src_init_config.codec_config_list.codec_config_num = 1;
        src_init_config.codec_config_list.codec_config_list[0].codec_type =
                                BT_A2DP_CODEC_TYPE_SBC;
        src_init_config.codec_config_list.codec_config_list[0].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DEFAULT;

    }
    // test 2:  sbc  ,codec_priority : default and aac codec_priority : default
    else if (test_num == 2)
    {
        src_init_config.codec_config_list.codec_config_num = 2;
        src_init_config.codec_config_list.codec_config_list[0].codec_type =
                                BT_A2DP_CODEC_TYPE_SBC;
        src_init_config.codec_config_list.codec_config_list[0].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DEFAULT;

        src_init_config.codec_config_list.codec_config_list[1].codec_type =
                                BT_A2DP_CODEC_TYPE_AAC;
        src_init_config.codec_config_list.codec_config_list[1].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DEFAULT;
    }
    // test 3:  sbc  ,codec_priority : default and aac codec_priority : default
    else if (test_num == 3)
    {
        src_init_config.codec_config_list.codec_config_num = 2;
        src_init_config.codec_config_list.codec_config_list[0].codec_type =
                                BT_A2DP_CODEC_TYPE_SBC;
        src_init_config.codec_config_list.codec_config_list[0].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DEFAULT;

        src_init_config.codec_config_list.codec_config_list[1].codec_type =
                                BT_A2DP_CODEC_TYPE_AAC;
        src_init_config.codec_config_list.codec_config_list[1].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_HIGHEST;
    }
    // test 3:  sbc  ,codec_priority : default and aac codec_priority : disable
    else if (test_num == 4)
    {
        src_init_config.codec_config_list.codec_config_num = 2;
        src_init_config.codec_config_list.codec_config_list[0].codec_type =
                                BT_A2DP_CODEC_TYPE_SBC;
        src_init_config.codec_config_list.codec_config_list[0].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DEFAULT;

        src_init_config.codec_config_list.codec_config_list[1].codec_type =
                                BT_A2DP_CODEC_TYPE_AAC;
        src_init_config.codec_config_list.codec_config_list[1].codec_priority =
                                BT_A2DP_CODEC_PRIORITY_DISABLED;
    }
    else if (test_num == 5)
    {
        src_init_config.codec_config_list.codec_config_num = 0;
    }
    return 0;
}

static int btmw_rpc_test_a2dp_src_config_codec_info(int argc, char *argv[])
{
    UINT8 count = 0;
    CHAR* ptr = NULL;
    UINT32 bit_mask = 0;
    BT_A2DP_SET_CODEC_CONFIG config_codec_info;
    BT_A2DP_CODEC_TYPE  codec_type = BT_A2DP_CODEC_TYPE_SBC;
    BT_A2DP_CODEC_CHANNEL_MODE channel_mode = BT_A2DP_CODEC_CHANNEL_MODE_NONE;
    BT_A2DP_CODEC_PRIORITY codec_priority = BT_A2DP_CODEC_PRIORITY_DEFAULT;
    BT_A2DP_CODEC_SAMPLE_RATE sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_48000;
    BT_A2DP_CODEC_BITS_PER_SAMPLE bits_per_sample = BT_A2DP_CODEC_BITS_PER_SAMPLE_16;
    memset(&config_codec_info, 0, sizeof(BT_A2DP_SET_CODEC_CONFIG));
    BTMW_RPC_TEST_Logi("%s()argc =%d\n", __func__, argc);

    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        return -1;
    }
    if (argc < 2)

    {
        BTMW_RPC_TEST_Loge("[USAGE] config_codec_info <addr> codec_type <type> priority <priority>] channel_mode <channel_mode> sample_rate <sample_rate> bits_per_sample <bits_per_sample>");
        return -1;
    }
    ptr = argv[0];
    count++;
    while (count < argc - 1)
    {
        BTMW_RPC_TEST_Logi("%s parser cli\n", __func__);
        if (strcmp(argv[count], "codec_type") == 0)
        {
            count++;
            codec_type = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("%s codec_type =%d\n", __func__, codec_type);
            bit_mask |= 1;
            continue;
        }
        else if (strcmp(argv[count], "priority") == 0)
        {
            count++;

            if (strcmp(argv[count], "disable") == 0)
            {
                codec_priority = BT_A2DP_CODEC_PRIORITY_DISABLED;
            }
            else
            {
                codec_priority = atoi(argv[count]);
            }

            bit_mask |= 2;
            BTMW_RPC_TEST_Logi("%s codec_priority =%d\n", __func__, codec_priority);
            count++;
            continue;
        }
        else if (strcmp(argv[count], "channel_mode") == 0)
        {
            count++;
            if (strcmp(argv[count], "mono") == 0)
            {
                channel_mode = BT_A2DP_CODEC_CHANNEL_MODE_MONO;
            }
            else if (strcmp(argv[count], "stereo")== 0)
            {
                channel_mode = BT_A2DP_CODEC_CHANNEL_MODE_STEREO;
            }
            else
            {
                channel_mode = BT_A2DP_CODEC_CHANNEL_MODE_NONE;
            }
            bit_mask |= 4;
            count++;
            BTMW_RPC_TEST_Logi("%s channel_mode =%d\n", __func__, channel_mode);
            continue;
        }
        else if (strcmp(argv[count], "sample_rate") == 0)
        {
            count++;
            if (strcmp(argv[count], "44100") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_44100;
            }
            else if(strcmp(argv[count], "48000") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_48000;
            }
            else if(strcmp(argv[count], "88200") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_88200;
            }
            else if(strcmp(argv[count], "96000")== 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_96000;
            }
            else if(strcmp(argv[count], "176400") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_176400;
            }
            else if(strcmp(argv[count], "192000") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_192000;
            }
            else if(strcmp(argv[count], "16000") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_16000;
            }
            else if(strcmp(argv[count], "24000") == 0)
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_24000;
            }
            else
            {
                sample_rate = BT_A2DP_CODEC_SAMPLE_RATE_NONE;
            }
            bit_mask |= 8;
            count++;
            BTMW_RPC_TEST_Logi("%s sample_rate =%d\n", __func__, sample_rate);
            continue;
        }
        else if (strcmp(argv[count], "bits_per_sample") == 0)
        {
            count++;
            if (strcmp(argv[count], "16") == 0)
            {
                bits_per_sample = BT_A2DP_CODEC_BITS_PER_SAMPLE_16;
            }
            else if (strcmp(argv[count], "24") == 0)
            {
                bits_per_sample = BT_A2DP_CODEC_BITS_PER_SAMPLE_24;
            }
            else if (strcmp(argv[count], "32") == 0)
            {
                bits_per_sample = BT_A2DP_CODEC_BITS_PER_SAMPLE_32;
            }
            else
            {
                bits_per_sample = BT_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
            }
            bit_mask |= 16;
            count++;
            BTMW_RPC_TEST_Logi("%s bits_per_sample =%d\n", __func__, bits_per_sample);
            continue;
        }
        count += 2;;
    }

    config_codec_info.codec_type = codec_type;
    config_codec_info.bit_mask = bit_mask;
    config_codec_info.codec_priority = codec_priority;
    config_codec_info.ch_mode = channel_mode;
    config_codec_info.sample_rate = sample_rate;
    config_codec_info.bits_per_sample = bits_per_sample;
    a_mtkapi_a2dp_src_config_codec_info(ptr, &config_codec_info);
    return 0;
}

static int btmw_rpc_test_a2dp_src_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    UINT8 u1_max_dev = 0;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active_src <1:enable|0:disable> max_dev <max_dev>");
        return -1;
    }

    u1_enable = atoi(argv[0]);
    if (strcmp(argv[1],"max_dev") == 0)
    {
        u1_max_dev = atoi(argv[2]);
    }
    src_init_config.max_connected_audio_devices = u1_max_dev;
    if (src_init_config.codec_config_list.codec_config_num != 0)
    {
        if (u1_enable != 0)
        {
            BTMW_RPC_TEST_Logi("%s() set auto connect test thread exit\n", __func__);
            atuo_connecet_thread_exit = 1;
        }
        a_mtkapi_a2dp_src_enable(u1_enable, &src_init_config);
    }
    else
    {
        a_mtkapi_a2dp_src_enable(u1_enable, NULL);
    }
    return 0;
}


static int btmw_rpc_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = a_mtkapi_a2dp_src_get_dev_list(&device_list);
    BTMW_RPC_TEST_Logi("get paired sink device list result:\n", __func__);
    BTMW_RPC_TEST_Logi("======================================\n");
    if(BT_SUCCESS == ret)
    {
        if (0 == device_list.dev_num)
        {
            BTMW_RPC_TEST_Logi("no paired sink device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_RPC_TEST_Logi("device[%d]: %s, name:%s, role:%s\n",
                    i, device_list.dev[i].addr, device_list.dev[i].name,
                    btmw_rpc_test_a2dp_src_app_role(device_list.dev[i].role));
                BTMW_RPC_TEST_Logi("======================================\n");
            }
        }
    }
    else
    {
        BTMW_RPC_TEST_Logi("get paired sink device failed: %d\n", ret);
    }

    return 0;
}

static int btmw_rpc_test_a2dp_src_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SRC);

    return 0;
}

static void* a2dp_auto_test_thread(void *arg)
{
    while (1)
    {
        usleep(1000*1000);
        if (true == atuo_connecet_thread_exit)
        {
            BTMW_RPC_TEST_Loge("disable src exit thread!");
            break;
        }
        if (connect_state == 1)
        {
            connect_state = -1;
            a_mtkapi_a2dp_disconnect(g_rpc_a2dp_connect_test_addr);
        }
        else if (connect_state == 0)
        {
            connect_state = -1;
            atuo_test_cnt++;
            if (atuo_test_cnt < 1000)
            {
                a_mtkapi_a2dp_connect(g_rpc_a2dp_connect_test_addr, BT_A2DP_ROLE_SRC);
            }
            else
            {
                BTMW_RPC_TEST_Loge("auto test connect success!");
                break;
            }
        }
    };
    g_a2dp_auto_test_thread = -1;
    atuo_connecet_thread_exit = false;
    return NULL;
}

static int btmw_rpc_test_a2dp_src_connect_auto_test_int_handler(int argc, char *argv[])
{
    CHAR *ptr;
    INT32 i4_ret = 0;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);
    atuo_connecet_thread_exit = 0;
    memset(g_rpc_a2dp_connect_test_addr, 0, 18);
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        printf("[GATTC] pthread_attr_init i4_ret:%d\n", i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&g_a2dp_auto_test_thread,
                                          &attr,
                                          a2dp_auto_test_thread,
                                          NULL)))
        {
            //BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_create i4_ret:%ld", (long)i4_ret);
            printf("[GATTC] pthread_create i4_ret:%d\n", i4_ret);
            assert(0);
        }
    }
    else
    {
        //BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
        printf("[GATTC] pthread_attr_setdetachstate i4_ret:%d\n", i4_ret);
    }

    pthread_attr_destroy(&attr);
    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    memcpy(g_rpc_a2dp_connect_test_addr, ptr, 17);
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SRC);
    return 0;
}

static int btmw_rpc_test_a2dp_src_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }

    ptr = argv[0];
    a_mtkapi_a2dp_disconnect(ptr);
    return 0;
}

static VOID* btmw_rpc_test_write_audio_data_thread(VOID *arg)
{
#if 1 //need refactor
    UINT8 *pcm_buffer = NULL;
    CHAR *local_test_pcm_file = (CHAR *)arg;
    FILE *fInputPCM;
    INT32 pcm_frame_len = PCM_BUFFER_SIZE;
    INT32 total_pcm_len;
    int ret;
    //UINT32 sample_rate = 0;
    hw_device_t* device = NULL;

    g_bt_src_stream_is_suspend = 0;

    BTMW_RPC_TEST_Logd("Input file name  : %s\n", local_test_pcm_file);
    /* open file & allocate memory */
    fInputPCM = fopen(local_test_pcm_file, "rb");
    if (fInputPCM == NULL)
    {
        BTMW_RPC_TEST_Loge("Can't open input PCM file!\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }
    BTMW_RPC_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        (void)fclose(fInputPCM);
        BTMW_RPC_TEST_Loge("Can't allocat buffer\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }
    g_fg_audio_path_runing = 1;
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (0 > load_bt_a2dp_module(a2dphw_so_patch, &gdevice))//hw_device_t
    {
        BTMW_RPC_TEST_Loge("open bt adev fail\n");
        goto send_audio_data_end;
    }
    if (gp_rpc_audio_dev == NULL)
    {
        gp_rpc_audio_dev = (struct audio_hw_device *)gdevice;
    }
#else
    if (gp_rpc_audio_dev == NULL)
    {
        if (0 > g_rpc_audio_module->common.methods->open(NULL, AUDIO_HARDWARE_INTERFACE, &device))
        {
            BTMW_RPC_TEST_Loge("open bt adev fail\n");
            goto send_audio_data_end;
        }
        gp_rpc_audio_dev = (struct audio_hw_device *)device;
    }
#endif
    ret = gp_rpc_audio_dev->open_output_stream(gp_rpc_audio_dev, 0, 0, AUDIO_OUTPUT_FLAG_NONE, NULL, &g_rpc_stream_out, NULL);
    if (ret < 0) {
        BTMW_RPC_TEST_Loge("Open output stream fail (err = %d)!\n", ret);
        goto send_audio_data_end;
    }

    BTMW_RPC_TEST_Logd("g_fg_audio_path_runing %d\n", g_fg_audio_path_runing);

    //sample_rate = (UINT32)g_rpc_stream_out->common.get_sample_rate(g_rpc_stream_out);
    //BTMW_RPC_TEST_Logd("out_get_sample_rate sample_rate=%d\n", sample_rate);
    while(g_fg_audio_path_runing)
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, fInputPCM );
        if (total_pcm_len == 0)
        {
            BTMW_RPC_TEST_Loge("total_pcm_len==0\n");
            (void)fseek(fInputPCM, 0L, SEEK_SET);

            continue;
        }


#if 0
        gettimeofday(&tv, NULL);
        if (((tv.tv_sec - last.tv_sec) * 1000000 + (long)tv.tv_usec - (long)last.tv_usec) > 500000)
        {
#if 0
            hw_dev->close_output_stream(hw_dev, stream);
            device->close(device);
            adev_open(NULL, AUDIO_HARDWARE_INTERFACE, &device);
            hw_dev = (struct audio_hw_device *)device;
            hw_dev->open_output_stream(hw_dev, 0, 0, 0, 0, &stream, 0);
#else
            hw_dev->set_parameters(hw_dev, "A2dpSuspended=true");
            hw_dev->set_parameters(hw_dev, "A2dpSuspended=false");
#endif
            last = tv;
        }
#endif


        g_rpc_stream_out->write(g_rpc_stream_out, pcm_buffer, total_pcm_len);
        usleep(5000); //titan test to prevent busy loop

    }
    BTMW_RPC_TEST_Logd("[A2DP] fclose %p\n", fInputPCM);
    //fclose(fInputPCM);
send_audio_data_end:
    BTMW_RPC_TEST_Logd("free BEFORE gp_rpc_audio_dev \n");
    if (NULL != g_rpc_stream_out)
    {
        BTMW_RPC_TEST_Logd("free g_rpc_stream_out \n");
        gp_rpc_audio_dev->close_output_stream(gp_rpc_audio_dev, g_rpc_stream_out);
        g_rpc_stream_out = NULL;
    }

    g_fg_audio_path_runing = 0;

    if (NULL != fInputPCM)
    {
        BTMW_RPC_TEST_Logd("free device \n");
        (void)fclose(fInputPCM);
    }
    if ( NULL != pcm_buffer)
    {
        BTMW_RPC_TEST_Logd("free pcm_buffer \n");
        free(pcm_buffer);
    }

    btmw_rpc_test_stream_handle = -1;
    BTMW_RPC_TEST_Logd("Send audio finished\n");
#endif
    return NULL;
}


static int btmw_rpc_test_a2dp_src_write_stream_handler(int argc, char *argv[])
{
    INT32 result;

    BTMW_RPC_TEST_Logd("%s(), file:%s\n", __func__, argv[0]);
    strncpy(btmw_rpc_test_pcm_file, argv[0], sizeof(btmw_rpc_test_pcm_file));
    btmw_rpc_test_pcm_file[127] = '\0';

    if(-1 == btmw_rpc_test_stream_handle)
    {
        result = pthread_create(&btmw_rpc_test_stream_handle, NULL,
            btmw_rpc_test_write_audio_data_thread, btmw_rpc_test_pcm_file);
        if (result)
        {
            BTMW_RPC_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_RPC_TEST_Logw("streaming thread has been created!\n");
    }

    return 0;
}



static INT32 btmw_rpc_test_play_audio_set_params(INT32 fs, INT32 channel_num)
{
    BTMW_RPC_TEST_Logd("+++into");
    INT32 i4ret = 0;
    size_t n;
    UINT32 u4rate;
    snd_pcm_uframes_t start_threshold;
    snd_pcm_uframes_t stop_threshold;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    /* choose all parameters */
    i4ret = snd_pcm_hw_params_any(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Broken configuration for playback: no configurations available: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the sample format */
    if (32 == btmw_rpc_test_bitdepth)
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S32_LE);
    }
    else if (24 == btmw_rpc_test_bitdepth)
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S24_LE);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S16_LE);
    }
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Sample format not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the interleaved read/write format */
    i4ret = snd_pcm_hw_params_set_access(s_alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Access type not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the count of channels */
    i4ret = snd_pcm_hw_params_set_channels(s_alsa_handle, hwparams, channel_num);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Channels count (%i) not available for playbacks: %s", channel_num, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the stream sampling rate */
    i4ret = snd_pcm_hw_params_set_rate(s_alsa_handle, hwparams, fs, 0);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }

    u4rate = fs;
    if ((u4buffer_time == 0) && (buffer_frames == 0))
    {
        i4ret = snd_pcm_hw_params_get_buffer_time_max(hwparams, &u4buffer_time, 0);
        if (i4ret < 0)
        {
            BTMW_RPC_TEST_Loge("fail to get max buffer time:%d, %s", i4ret, snd_strerror(i4ret));
            return i4ret;
        }
        BTMW_RPC_TEST_Logd("u4buffer_time:%d", u4buffer_time);
        if (u4buffer_time > 500000)
        {
            u4buffer_time = 500000;
        }
    }
    if ((u4period_time == 0) && (period_frames == 0))
    {
        if (u4buffer_time > 0)
        {
            u4period_time = u4buffer_time / 4;
        }
        else
        {
            period_frames = buffer_frames / 4;
        }
    }
    if (u4period_time > 0)
    {
        i4ret = snd_pcm_hw_params_set_period_time_near(s_alsa_handle, hwparams,
                &u4period_time, 0);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_period_size_near(s_alsa_handle, hwparams,
                &period_frames, 0);
    }
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("fail to get period size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }
    if (u4buffer_time > 0)
    {
        i4ret = snd_pcm_hw_params_set_buffer_time_near(s_alsa_handle, hwparams,
                &u4buffer_time, 0);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_buffer_size_near(s_alsa_handle, hwparams,
                &buffer_frames);
    }
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("fail to get buffer size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    i4ret = snd_pcm_hw_params(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Unable to install hw params");
        return i4ret;
    }

    snd_pcm_hw_params_get_period_size(hwparams, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    BTMW_RPC_TEST_Logd("chunk_size:%lu, buffer_size:%lu", chunk_size, buffer_size);
    if (chunk_size == buffer_size)
    {
        BTMW_RPC_TEST_Loge("Can't use period equal to buffer size (%lu == %lu)",
                       chunk_size, buffer_size);
        return i4ret;
    }

    /* get the current swparams */
    snd_pcm_sw_params_current(s_alsa_handle, swparams);
    if (i4avail_min < 0)
    {
        n = chunk_size;
    }
    else
    {
        n = (double) u4rate * i4avail_min / 1000000;
    }
    snd_pcm_sw_params_set_avail_min(s_alsa_handle, swparams, n);

    /* round up to closest transfer boundary */
    n = buffer_size;
    if (i4start_delay <= 0)
    {
        start_threshold = n + (double) u4rate * i4start_delay / 1000000;
    }
    else
    {
        start_threshold = (double) u4rate * i4start_delay / 1000000;
    }
    if (start_threshold < 1)
    {
        start_threshold = 1;
    }
    if (start_threshold > n)
    {
        start_threshold = n;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    i4ret = snd_pcm_sw_params_set_start_threshold(s_alsa_handle, swparams, start_threshold);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("fail to set start threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }
    if (i4stop_delay <= 0)
    {
        stop_threshold = buffer_size + (double) u4rate * i4stop_delay / 1000000;
    }
    else
    {
        stop_threshold = (double) u4rate * i4stop_delay / 1000000;
    }
    i4ret = snd_pcm_sw_params_set_stop_threshold(s_alsa_handle, swparams, stop_threshold);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("fail to set stop threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    /* write the parameters to the playback device */
    if ((i4ret = snd_pcm_sw_params(s_alsa_handle, swparams)) < 0)
    {
        BTMW_RPC_TEST_Loge("unable to install sw params");
        return i4ret;
    }

    snd_pcm_sw_params_get_start_threshold(swparams, &start_threshold);
    snd_pcm_sw_params_get_stop_threshold(swparams, &stop_threshold);
    BTMW_RPC_TEST_Logd("start_threshold:%lu, stop_threshold:%lu", start_threshold, stop_threshold);
    //snd_pcm_hw_params_free(hwparams);
    //snd_pcm_sw_params_free(swparams);
    BTMW_RPC_TEST_Logd("---exit");
    return 0;
}

static INT32 btmw_rpc_test_play_audio_dsp_open(INT32 fs, INT32 channel_num)
{
    BTMW_RPC_TEST_Logd("+++into");
    INT32 i4ret = 0;

    //snd_pcm_hw_params_t *hwparams;

    if (s_alsa_handle != NULL)
    {
        BTMW_RPC_TEST_Logw("---exit already opened s_alsa_handle");
        return 0;
    }

    i4ret = snd_pcm_open(&s_alsa_handle, ALSA_DEVICE_PLAYER, SND_PCM_STREAM_PLAYBACK, 0 );
    BTMW_RPC_TEST_Logd("fs %d, channel num %d samplebytes:%d, bitdepth:%d",
        fs, channel_num, btmw_rpc_test_samplebytes, btmw_rpc_test_bitdepth);
    BTMW_RPC_TEST_Logd("dsp_open %s i4ret=%d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Cannot open %s ERROR %d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
        s_alsa_handle = NULL;
        return  -1;
    }

    btmw_rpc_test_play_audio_set_params(fs, channel_num);


    snd_pcm_prepare(s_alsa_handle);

    BTMW_RPC_TEST_Logd("---exit");
    return 0;
}

static INT32 btmw_rpc_test_play_audio_dsp_write(UINT8 *buf, UINT32 size)
{
    INT32 i4ret = 0;

    if (s_alsa_handle == NULL)
    {
        BTMW_RPC_TEST_Loge("s_alsa_handle == NULL");
        return -1;
    }

    i4ret = snd_pcm_writei(s_alsa_handle, buf, size/btmw_rpc_test_samplebytes);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("ALSA ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        snd_pcm_prepare(s_alsa_handle);
        if ((i4ret = snd_pcm_prepare(s_alsa_handle))<0)
        {
            BTMW_RPC_TEST_Loge("ALSA snd_pcm_prepare ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        }
    }
    //BTMW_RPC_TEST_Logv("alsa write i4ret = %d", i4ret);
    return i4ret;
}

static INT32 btmw_rpc_test_play_audio_dsp_close(VOID)
{
    INT32 i4ret = 0;
    BTMW_RPC_TEST_Logd("+++into");
    if (s_alsa_handle == NULL)
    {
        BTMW_RPC_TEST_Loge("---exit s_alsa_handle == NULL");
        return -1;
    }
    if (s_alsa_handle != NULL)
    {
        i4ret = snd_pcm_close(s_alsa_handle);
        if (i4ret == 0)
        {
            BTMW_RPC_TEST_Logd("dsp_close success");
        }
        else
        {
            BTMW_RPC_TEST_Loge("dsp_close fail i4ret=%d[%s]", i4ret, snd_strerror(i4ret));
        }
        s_alsa_handle = NULL;
    }

    BTMW_RPC_TEST_Logd("---exit");
    return i4ret;
}



static VOID* btmw_rpc_test_play_audio_data_thread(VOID *arg)
{
    UINT8 *pcm_buffer = NULL;
    CHAR *local_test_pcm_file = (CHAR *)arg;
    FILE *fInputPCM;
    INT32 pcm_frame_len = PCM_BUFFER_SIZE;
    INT32 total_pcm_len;

    int read_cnt = 0;

    BTMW_RPC_TEST_Logd("Input file name  : %s\n", local_test_pcm_file);
    /* open file & allocate memory */
    fInputPCM = fopen(local_test_pcm_file, "rb");
    if (fInputPCM == NULL)
    {
        BTMW_RPC_TEST_Loge("Can't open input PCM file!\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }
    BTMW_RPC_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        (void)fclose(fInputPCM);
        BTMW_RPC_TEST_Loge("Can't allocat buffer\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }

    if (0 > btmw_rpc_test_play_audio_dsp_open(btmw_rpc_test_sample_rate, 2))
    {
        BTMW_RPC_TEST_Loge("dsp_open fail !!!\n");
        (void)fclose(fInputPCM);
        free(pcm_buffer);
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }

    g_bt_src_stream_is_suspend = 0;
    BTMW_RPC_TEST_Logd("addr[0]=%d, cli_mode=%d, suspend=%d\n",
        g_rpc_a2dp_addr_test[0], g_cli_pts_mode, g_bt_src_stream_is_suspend);

    while(((g_rpc_a2dp_addr_test[0] != 0 && 0 == g_cli_pts_mode && (!g_bt_src_stream_is_suspend))
            || (0 != g_cli_pts_mode)) && (!g_bt_src_stream_is_suspend))
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, fInputPCM );
        if (total_pcm_len == 0)
        {
            BTMW_RPC_TEST_Loge("total_pcm_len==0\n");
            (void)fseek(fInputPCM, 0L, SEEK_SET);
            read_cnt++;

            continue;
        }

        btmw_rpc_test_play_audio_dsp_write(pcm_buffer, total_pcm_len);
    }

    btmw_rpc_test_play_audio_dsp_close();

    (void)fclose(fInputPCM);
    free(pcm_buffer);

    btmw_rpc_test_stream_handle = -1;
    BTMW_RPC_TEST_Logd("Send audio finished\n");
    return NULL;
}


static int btmw_rpc_test_a2dp_src_active_sink_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active sink <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] active sink <addr>");
        if (strcmp(argv[0], "null") == 0)
        {
            BTMW_RPC_TEST_Loge("[USAGE] active sink null>");
            a_mtkapi_a2dp_src_active_sink(ptr);
            return 0;
        }
        return -1;
    }
    ptr = argv[0];

    BTMW_RPC_TEST_Logi("%s src set active sink \n", ptr);
    a_mtkapi_a2dp_src_active_sink(ptr);

    return 0;
}

static int btmw_rpc_test_a2dp_src_get_active_sink_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    CHAR addr[18] = {0};
    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    ptr = addr;

    BTMW_RPC_TEST_Logi("src get active sink \n");
    a_mtkapi_a2dp_src_get_active_sink(ptr);
    BTMW_RPC_TEST_Logi("%s src get active sink \n", ptr);

    return 0;
}

static int btmw_rpc_test_a2dp_src_set_silence_device_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    UINT8 u1_enable = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_silence_device <addr> <enable>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] active sink <addr>");
        return -1;
    }

    ptr = argv[0];
    u1_enable = atoi(argv[1]);

    BTMW_RPC_TEST_Logi("%s src set silence device %d \n", ptr, u1_enable);
    a_mtkapi_a2dp_src_set_silence_device(ptr, u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_src_is_in_silence_mode_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    BOOL result = FALSE;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] is_in_silence_mode <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] active sink <addr>");
        return -1;
    }

    ptr = argv[0];

    result = a_mtkapi_a2dp_src_is_in_silence_mode(ptr);
    BTMW_RPC_TEST_Logi("%s src is in %d silence mode\n", ptr, result);
    return 0;
}

static int btmw_rpc_test_a2dp_src_play_stream_handler(int argc, char *argv[])
{
    INT32 result;

    strncpy(btmw_rpc_test_pcm_file, argv[0], sizeof(btmw_rpc_test_pcm_file));
    btmw_rpc_test_pcm_file[127] = '\0';
    btmw_rpc_test_sample_rate = 48000;
    btmw_rpc_test_bitdepth = 16;
    if (argc > 1)
    {
        btmw_rpc_test_sample_rate = atoi(argv[1]);
    }
    if (argc > 2)
    {
        btmw_rpc_test_bitdepth = atoi(argv[2]);
    }
    BTMW_RPC_TEST_Logd("%s(), file:%s, samplerate:%d, bitdepth:%d\n",
        __func__, argv[0], btmw_rpc_test_sample_rate, btmw_rpc_test_bitdepth);
    if (btmw_rpc_test_bitdepth == 24 || btmw_rpc_test_bitdepth == 32)
    {
        btmw_rpc_test_samplebytes = 8;
    }
    else
    {
        btmw_rpc_test_samplebytes = 4;
    }
    if(-1 == btmw_rpc_test_stream_handle)
    {
        result = pthread_create(&btmw_rpc_test_stream_handle, NULL,
            btmw_rpc_test_play_audio_data_thread, btmw_rpc_test_pcm_file);
        if (result)
        {
            BTMW_RPC_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_RPC_TEST_Logw("streaming thread has been created!\n");
    }

    return 0;
}
static int btmw_rpc_test_a2dp_src_suspend_stream_handler(int argc, char *argv[])
{
    g_bt_src_stream_is_suspend = 1;

    if (btmw_rpc_test_stream_handle != 0) {
        g_fg_audio_path_runing = false;
        btmw_rpc_test_stream_handle = -1;
    }

    g_fg_audio_path_runing = 0;
    return 0;
}

static int btmw_rpc_test_a2dp_src_pause_uploader_handler(int argc, char *argv[])
{
    //a_mtkapi_a2dp_src_pause_uploader();
    return 0;
}

static int btmw_rpc_test_a2dp_src_resume_uploader_handler(int argc, char *argv[])
{
    //a_mtkapi_a2dp_src_resume_uploader();
    return 0;
}
static int btmw_rpc_test_a2dp_src_mute_uploader_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] mute [1:enable|0:disable]");
        return -1;
    }

    //a_mtkapi_a2dp_src_mute_uploader(u1_enable);
    return 0;
}

static int btmw_rpc_test_a2dp_src_load_uploader_handler(int argc, char *argv[])
{
    CHAR *uploader_path;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] load_upl <so_path>");
        return -1;
    }

    uploader_path = argv[0];
    BTMW_RPC_TEST_Logi("load_up %s", uploader_path);
    //a_mtkapi_a2dp_src_uploader_load(uploader_path);
    return 0;
}

static int btmw_rpc_test_a2dp_src_unload_uploader_handler(int argc, char *argv[])
{
    CHAR *uploader_name;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] unload_upl <uploader_name> (default:mtal_uploader)");
        return -1;
    }

    uploader_name = argv[0];
    BTMW_RPC_TEST_Logi("unload_up %s", uploader_name);
    //a_mtkapi_a2dp_src_uploader_unload(uploader_name);
    return 0;
}

static int btmw_rpc_test_a2dp_src_set_lrchannel_forlrmode(int argc, char *argv[])
{
    CHAR *addr;
    INT32 channel = 0;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_lrchannel <addr> <channel>, channel:1-left channel, 2-right channel,other-just return at stack");
        return -1;
    }

    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] active sink <addr>");
        return -1;
    }

    addr = argv[0];
    channel = atoi(argv[1]);

    BTMW_RPC_TEST_Logi("addr %s, channel %d", addr, channel);
    a_mtkapi_a2dp_src_set_channel_allocation_for_lrmode(addr, channel);
    return 0;
}

static int btmw_rpc_test_a2dp_src_set_audiomode(int argc, char *argv[])
{
    INT32 audio_mode;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_audiomode <audio_mode>, 0:normal mode, 1:same mode, 2:LR mode");
        return -1;
    }

    audio_mode = atoi(argv[0]);
    BTMW_RPC_TEST_Logi("audio_mode %d", audio_mode);
    if (audio_mode > 2)
    {
        BTMW_RPC_TEST_Loge(" error audio_mode!");
        return -1;
    }
    a_mtkapi_a2dp_src_set_audiomode(audio_mode);
    int i = 0;
    BT_A2DP_CONNECT_DEV_INFO_LIST dev_list;
    BT_A2DP_SET_CODEC_CONFIG config_codec_info;
    memset(&dev_list, 0, sizeof(BT_A2DP_CONNECT_DEV_INFO_LIST));
    memset(&config_codec_info, 0, sizeof(BT_A2DP_SET_CODEC_CONFIG));

    config_codec_info.bit_mask = 4;
    config_codec_info.ch_mode = BT_A2DP_CODEC_CHANNEL_MODE_STEREO;
    if (audio_mode == 2)
    {
        config_codec_info.ch_mode = BT_A2DP_CODEC_CHANNEL_MODE_MONO;
    }

    a_mtkapi_a2dp_get_connected_dev_list(&dev_list);
    BTMW_RPC_TEST_Logi("[A2DP] %s() dev_num=%d\n", __func__, dev_list.dev_num);

    for (i = 0; i < dev_list.dev_num; i++)
    {
        BTMW_RPC_TEST_Logi("[A2DP] dec addr=%s\n",
            dev_list.a2dp_connected_dev_list[i].addr);
        BTMW_RPC_TEST_Logi("[A2DP] stream_status=%d\n",
            dev_list.a2dp_connected_dev_list[i].stream_status);
        BTMW_RPC_TEST_Logi("[A2DP] conn_status=%d\n",
            dev_list.a2dp_connected_dev_list[i].conn_status);
        BTMW_RPC_TEST_Logi("[A2DP] channel_mode=%d\n",
            dev_list.a2dp_connected_dev_list[i].codec_config.channel_mode);
        BTMW_RPC_TEST_Logi("[A2DP] sample_rate=%d\n",
            dev_list.a2dp_connected_dev_list[i].codec_config.sample_rate);

        if (dev_list.a2dp_connected_dev_list[i].local_role == BT_A2DP_ROLE_SRC)
        {
            a_mtkapi_a2dp_src_config_codec_info(dev_list.a2dp_connected_dev_list[i].addr, &config_codec_info);
        }
    }
    return 0;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_a2dp_src_cli_commands[] =
{
    {"connect",             btmw_rpc_test_a2dp_src_connect_int_handler,               " = connect <addr>"},
    {"connect_auto_test",   btmw_rpc_test_a2dp_src_connect_auto_test_int_handler,     " = connect_auto_test <addr>"},
    {"disconnect",          btmw_rpc_test_a2dp_src_disconnect_handler,                " = disconnect <addr>"},
    {"active_src",          btmw_rpc_test_a2dp_src_active_handler,                    " = active_src <1:enable|0:disable> max_dev <max_dev>"},
    {"set_init_codec_info", btmw_rpc_test_a2dp_src_set_init_codec_info,      " = set_init_codec_info test <num>"},
    {"config_codec_info",   btmw_rpc_test_a2dp_src_config_codec_info,      " = config_codec_info <addr> codec_type <type> [priority <priority>] [channel_mode <channel_mode>] [sample_rate  <sample_rate>] [bits_per_sample <bits_per_sample>]"},
    {"get_paired_dev",      btmw_rpc_test_a2dp_get_sink_dev_list_handler,             " = get_paired_dev"},
    {"write_stream",        btmw_rpc_test_a2dp_src_write_stream_handler,              " = write_stream <file-path>"},
    {"audio_hw_log",        btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler,      " = audio_hw_log <0~6>"},
    {"show_info",           btmw_rpc_test_a2dp_src_show_dev_info_handler,             " = show_info"},
    {"active_sink",         btmw_rpc_test_a2dp_src_active_sink_handler,               " = active_sink <addr>"},
    {"get_active_sink",     btmw_rpc_test_a2dp_src_get_active_sink_handler,       " = get_active_sink"},
    {"set_silence_device",  btmw_rpc_test_a2dp_src_set_silence_device_handler,   " = set_silence_device <addr> <enable>"},
    {"is_in_silence_mode",  btmw_rpc_test_a2dp_src_is_in_silence_mode_handler,   " = is_in_silence_mode <addr>"},
    {"play_stream",         btmw_rpc_test_a2dp_src_play_stream_handler,               " = play_stream"},
    {"suspend_stream",      btmw_rpc_test_a2dp_src_suspend_stream_handler,         " = suspend_stream"},
    {"pause_upl",           btmw_rpc_test_a2dp_src_pause_uploader_handler,              " = pause_upl"},
    {"resume_upl",          btmw_rpc_test_a2dp_src_resume_uploader_handler,            " = resume_upl"},
    {"mute_upl",            btmw_rpc_test_a2dp_src_mute_uploader_handler,              " = mute_upl"},
    {"load_upl",            btmw_rpc_test_a2dp_src_load_uploader_handler,             " = load_upl"},
    {"unload_upl",          btmw_rpc_test_a2dp_src_unload_uploader_handler,           " = unload_upl"},
    {"set_audiomode",       btmw_rpc_test_a2dp_src_set_audiomode,                     " = set_audiomode <audio_mode>"},
    {"set_lrchannel",       btmw_rpc_test_a2dp_src_set_lrchannel_forlrmode,           " = set_lrchannel <addr> <channel>"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_a2dp_src_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_rpc_test_a2dp_src_cli_commands;
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

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_A2DP_SRC, btmw_rpc_test_a2dp_src_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
static CHAR* btmw_rpc_test_a2dp_src_app_event(BT_A2DP_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECTED)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_DISCONNECTED)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECT_TIMEOUT)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_SUSPEND)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_START)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECT_COMING)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_PLAYER_EVENT)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_ROLE_CHANGED)
        default: return "UNKNOWN_EVENT";
   }
}
#endif

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
INT32 start_a2dp_uploader(UINT32 sample_rate, UINT32 channel_num)
{
    /* start uploader */
    BTMW_RPC_TEST_Logd("init&start uploader\n");
    bt_a2dp_mtal_uploader_init((int)sample_rate, (int)channel_num);
    if (bt_a2dp_mtal_uploader_start(0) != 0)
    {
        return -1;
    }
#if 0
    // set bluetooth volume 100
    int i4_ret = 0;
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
#endif
    return 0;
}
#endif

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
static VOID btmw_rpc_test_a2dp_src_app_cbk(BT_A2DP_EVENT_PARAM *param, VOID *pv_tag)
{
    //UINT8 codec_type = 0;
    //UINT8 i = 0;
    UINT8 codec_config_num = 0;

    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_rpc_test_a2dp_src_app_event(param->event));
    switch (param->event)
    {
        case BT_A2DP_EVENT_CONNECTED:
            if (param->data.connected_data.local_role == BT_A2DP_ROLE_SRC)
            {
                a_mtkapi_a2dp_src_active_sink(param->addr);
 #if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
                BTMW_RPC_TEST_Logd("update_conn_state before true\n");
                if (dl_a_mtktvapi_bluetooth_agent_update_conn_state != NULL)
                {
                    dl_a_mtktvapi_bluetooth_agent_update_conn_state(TRUE);
                    BTMW_RPC_TEST_Logd("update_conn_state after true\n");
                }
 #endif
                auc.src_st = BT_A2DP_EVENT_CONNECTED;
                auc.channel_num = param->data.connected_data.channel_num;
                auc.sample_rate = param->data.connected_data.sample_rate;
                if (auc.bst != BT_BMS_STREAMING && !auc.leaudio_uploader_in_use && !auc.a2dpsrc_uploader_in_use)
                {
                    BTMW_RPC_TEST_Logd("A2DP SRC connected(%s) and broadcast not streaming, %d-%d\n",
                        param->addr, param->data.connected_data.sample_rate, param->data.connected_data.channel_num);
                    /* start uploader */
                    auc.channel_num = param->data.connected_data.channel_num;
                    auc.sample_rate = param->data.connected_data.sample_rate;
                    if (start_a2dp_uploader(param->data.connected_data.sample_rate,param->data.connected_data.channel_num) == 0)
                    {
                        auc.a2dpsrc_uploader_in_use = TRUE;
                    }
                }
                else
                {
                    BTMW_RPC_TEST_Logd("A2DP SRC connected(%s) and broadcast already streaming\n", param->addr);
                }
            }
            break;
        case BT_A2DP_EVENT_DISCONNECTED:
            BTMW_RPC_TEST_Logd("A2DP disconnected(%s)\n", param->addr);
            int i = 0;
            BT_A2DP_CONNECT_DEV_INFO_LIST dev_list;
            memset(&dev_list, 0, sizeof(BT_A2DP_CONNECT_DEV_INFO_LIST));
            a_mtkapi_a2dp_get_connected_dev_list(&dev_list);
            BTMW_RPC_TEST_Logi("[A2DP SRC] %s() dev_num=%d\n", __func__, dev_list.dev_num);
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            CHAR addr[18] = {0};
            CHAR *ptr = addr;
            a_mtkapi_a2dp_src_get_active_sink(ptr);
            BTMW_RPC_TEST_Logd("active device addr(%s)\n", addr);
            /* active device disconncet */
            if (!memcmp(addr, param->addr, MAX_BDADDR_LEN-1))
            {
                /* is no device connected, need notify */
                if (dev_list.dev_num == 0)
                {
                    if (dl_a_mtktvapi_bluetooth_agent_update_conn_state != NULL)
                    {
                        dl_a_mtktvapi_bluetooth_agent_update_conn_state(FALSE);
                        BTMW_RPC_TEST_Logd("update_conn_state after true\n");
                    }
                }
                /* if exit other device connected, set new active */
                else
                {
                    usleep(1000*500);;
                    a_mtkapi_a2dp_src_active_sink(dev_list.a2dp_connected_dev_list[0].addr);
                    BTMW_RPC_TEST_Logd("set new active device as active when active disconnected\n");
                }
            }
            /* no active device disconnect*/
            else
            {
                if (dev_list.dev_num == 0)
                {
                    if (dl_a_mtktvapi_bluetooth_agent_update_conn_state != NULL)
                    {
                        dl_a_mtktvapi_bluetooth_agent_update_conn_state(FALSE);
                        BTMW_RPC_TEST_Logd("update_conn_state after true\n");
                    }
                }
                /* if exit other device connected, do nothing */
                else
                {
                    BTMW_RPC_TEST_Logd("no active device disconnected and exist other device, no need notify\n");
                }
            }
#endif
            if (dev_list.dev_num == 0)
            {
                auc.src_st = BT_A2DP_EVENT_DISCONNECTED;
            }
            if (param->data.connected_data.local_role == BT_A2DP_ROLE_SRC && auc.a2dpsrc_uploader_in_use && auc.src_st == BT_A2DP_EVENT_DISCONNECTED)
            {
                //auc.src_st = BT_A2DP_EVENT_DISCONNECTED;
                /* stop uploader */
                BTMW_RPC_TEST_Logd("stop&deinit uploader\n");
                bt_a2dp_mtal_uploader_stop();
                bt_a2dp_mtal_uploader_deinit();
                auc.a2dpsrc_uploader_in_use = FALSE;
            }
            break;
        case BT_A2DP_EVENT_CONNECT_TIMEOUT:
            BTMW_RPC_TEST_Logd("A2DP Connect Timeout(%s)\n", param->addr);
            break;
        case BT_A2DP_EVENT_STREAM_SUSPEND:
            break;
        case BT_A2DP_EVENT_STREAM_START:
            break;
        case BT_A2DP_EVENT_CONNECT_COMING:
            break;
        case BT_A2DP_EVENT_PLAYER_EVENT:
            break;
        case BT_A2DP_EVENT_ROLE_CHANGED:
            break;
        case BT_A2DP_EVENT_SRC_AUDIO_CONFIG:
            BTMW_RPC_TEST_Logd("A2DP SRC audio config codec_type(%d)\n",
                param->data.audio_config.audio_config.codec_type);
            BTMW_RPC_TEST_Logd("A2DP SRC audio config channel_mode(%d)\n",
                param->data.audio_config.audio_config.channel_mode);
            BTMW_RPC_TEST_Logd("A2DP SRC audio config sample_rate(%d)\n",
                param->data.audio_config.audio_config.sample_rate);
            BTMW_RPC_TEST_Logd("A2DP SRC audio config bits_per_sample(%d)\n",
                param->data.audio_config.audio_config.bits_per_sample);

            codec_config_num =
                param->data.audio_config.codec_selectable_capabilities.codec_config_num;

            BTMW_RPC_TEST_Logd("A2DP audio config codec_selectable_capabilities.codec_config_num(%d)\n",
                codec_config_num);
            for (i = 0; i < codec_config_num; i++)
            {
                BTMW_RPC_TEST_Logd("A2DP codec_selectable_capabilities.codec_type(%d)\n",
                    param->data.audio_config.codec_selectable_capabilities.codec_config_list[i].codec_type);
                BTMW_RPC_TEST_Logd("A2DP codec_selectable_capabilities.codec_priority(%d)\n",
                    param->data.audio_config.codec_selectable_capabilities.codec_config_list[i].codec_priority);
                BTMW_RPC_TEST_Logd("A2DP  codec_selectable_capabilities.sample_rate(%d)\n",
                    param->data.audio_config.codec_selectable_capabilities.codec_config_list[i].sample_rate);
                BTMW_RPC_TEST_Logd("A2DP codec_selectable_capabilities.bits_per_sample(%d)\n",
                    param->data.audio_config.codec_selectable_capabilities.codec_config_list[i].bits_per_sample);
                BTMW_RPC_TEST_Logd("A2DP codec_selectable_capabilities.channel_mode(%d)\n",
                    param->data.audio_config.codec_selectable_capabilities.codec_config_list[i].channel_mode);
            }
            break;
        case BT_A2DP_EVENT_ACTIVE_CHANGED:
            BTMW_RPC_TEST_Logd("A2DP  BT_A2DP_EVENT_ACTIVE_CHANGED(%s)\n", param->addr);
            break;
        default:
            break;
    }
    return;
}
#endif

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
static CHAR* btmw_rpc_test_bms_app_event(BT_BMS_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_CREATED)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_DESTORYED)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_STATE)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_SOCKET_INDEX_NOTIFY)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_ISO_STATUS)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_OWN_ADDRESS)
        default: return "UNKNOWN_BMS_EVENT";
   }
}

static CHAR* btmw_rpc_test_bms_broadcast_state(BT_BMS_BROADCAST_STATE state)
{
    switch((int)state)
    {
        case BT_BMS_STOPPED:
          return "STOPPED";
        case BT_BMS_PAUSED:
          return "PAUSED";
        case BT_BMS_STREAMING:
          return "STREAMING";
        default:
          return "UNKNOWN_BROADCAST_STATE";
   }
}

static VOID btmw_rpc_test_a2dp_src_bms_app_cbk(BT_BMS_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    switch (param->event)
    {
    case BT_BMS_EVENT_BROADCAST_STATE:
        BTMW_RPC_TEST_Logd("event=%d - %s, broadcast state %d - %s\n",
                            param->event,
                            btmw_rpc_test_bms_app_event(param->event),
                            param->data.broadcast_state.state,
                            btmw_rpc_test_bms_broadcast_state(param->data.broadcast_state.state));

        if (param->data.broadcast_state.state == BT_BMS_STREAMING && auc.a2dpsrc_uploader_in_use)
        {
            //bt_a2dp_mtal_uploader_pause();//will suspend avdtp ch
            /* stop uploader */
            BTMW_RPC_TEST_Logd("broadcast state streaming!\n");
            BTMW_RPC_TEST_Logd("stop&deinit a2dp uploader\n");
            auc.bst = BT_BMS_STREAMING;
            bt_a2dp_mtal_uploader_stop();
            bt_a2dp_mtal_uploader_deinit();
            auc.a2dpsrc_uploader_in_use = FALSE;
        }
        else if (param->data.broadcast_state.state != BT_BMS_STREAMING && !auc.a2dpsrc_uploader_in_use)
        {
            //bt_a2dp_mtal_uploader_resume();
            BTMW_RPC_TEST_Logd("broadcast state %d!\n", param->data.broadcast_state.state);
            //BTMW_RPC_TEST_Logd("start a2dp uploader\n");
            auc.bst = param->data.broadcast_state.state;
            if (auc.src_st == BT_A2DP_EVENT_CONNECTED && !auc.leaudio_uploader_in_use)
            {
                if (start_a2dp_uploader(auc.sample_rate, auc.channel_num) == 0)
                {
                    auc.a2dpsrc_uploader_in_use = TRUE;
                }
            }
        }
        break;

    default:
        break;
    }
    return;
}
#endif

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
INT32 a2dp_src_init_dl_function(VOID)
{
    fHandle1 = dlopen("libhandle_app.so",RTLD_LAZY|RTLD_DEEPBIND);
    if(!fHandle1)
    {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("load libhandle_app.so fail error:%s", err_str?err_str:"unknown");
        return -1;
    }
    a2dp_handle_init = dlsym(fHandle1, "handle_init");

    fHandle2 = dlopen("libdtv_osai.so",RTLD_LAZY|RTLD_DEEPBIND);
    if(!fHandle2)
    {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("load libdtv_osai.so fail error:%s", err_str?err_str:"unknown");
        (void)dlclose(fHandle1);
        return -1;
    }
    a2dp_c_rpc_start_client = dlsym(fHandle2, "c_rpc_start_client");
    a2dp_os_init = dlsym(fHandle2, "os_init");
    a2dp_x_rtos_init = dlsym(fHandle2, "x_rtos_init");

    app_handle = dlopen("libapp_if_rpc.so", RTLD_LAZY);
    if(!app_handle)
    {
        (void)dlclose(fHandle1);
        (void)dlclose(fHandle2);
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("load libapp_if_rpc.so fail error:%s", err_str?err_str:"unknown");
        return -1;
    }
    a2dp_c_rpc_init_client = dlsym(app_handle, "c_rpc_init_client");
    dl_a_mtktvapi_bluetooth_agent_update_conn_state = dlsym(app_handle, "a_mtktvapi_bluetooth_agent_update_conn_state");
    if (dl_a_mtktvapi_bluetooth_agent_update_conn_state == NULL)
    {
        BTMW_RPC_TEST_Loge("[A2DP] Failed to get update conn state\n");
        return -1;
    }

    return 0;
}

static INT32 a2dp_rpc_env_init( VOID )
{
    INT32 i4_ret = 0;
    GEN_CONFIG_T     t_rtos_config;
    VOID*                    pv_mem_addr = 0;
    UINT32    z_mem_size = 0xc00000;

    if(a2dp_binit == 1)
        return 0;
    BTMW_RPC_TEST_Loge("<RPC> czw a2dp_rpc_env_init     %d\n", i4_ret);
    memset( &t_rtos_config, 0, sizeof( GEN_CONFIG_T ) );
    i4_ret = a2dp_x_rtos_init ( &t_rtos_config );
    if(i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("<RPC> x_rtos_init     %d\n", i4_ret);
        return -1;
    }
    i4_ret = a2dp_handle_init ( MAX_NUM_HANDLES, &pv_mem_addr, &z_mem_size );
    if(i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("<RPC> handle_init  %d\n", i4_ret);
        return -2;
    }
    i4_ret = a2dp_os_init ( pv_mem_addr, z_mem_size );
    if(i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("<RPC> os_init  %d\n", i4_ret);
        return -3;
    }

    i4_ret = a2dp_c_rpc_init_client();
    if(i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("<RPC> c_rpc_init_client     %d\n", i4_ret);
        return -4;
    }
    i4_ret =  a2dp_c_rpc_start_client();
    if(i4_ret <0)
    {
        BTMW_RPC_TEST_Loge("<RPC> c_rpc_start_client  %d\n", i4_ret);
        return -5;
    }

    a2dp_binit = 1;

    return 0;
}
#endif

INT32 btmw_rpc_test_a2dp_src_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD a2dp_src_mod = {0};

    a2dp_src_mod.mod_id = BTMW_RPC_TEST_MOD_A2DP_SRC;
    strncpy(a2dp_src_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_A2DP_SRC, sizeof(a2dp_src_mod.cmd_key));
    a2dp_src_mod.cmd_handler = btmw_rpc_test_a2dp_src_cmd_handler;
    a2dp_src_mod.cmd_tbl = btmw_rpc_test_a2dp_src_cli_commands;

    ret = btmw_rpc_test_register_mod(&a2dp_src_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for SRC returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
        auc.channel_num = 0;
        auc.sample_rate = 0;
        auc.bst = BT_BMS_STOPPED;
        auc.src_st = BT_A2DP_EVENT_DISCONNECTED;
        auc.snk_st = BT_A2DP_EVENT_DISCONNECTED;
        auc.leaudio_uploader_in_use = FALSE;
        auc.a2dpsrc_uploader_in_use = FALSE;
        auc.a2dpsnk_playback_in_use = FALSE;
        a_mtkapi_a2dp_register_callback(btmw_rpc_test_a2dp_src_app_cbk, NULL);
        a_mtkapi_bt_bms_register_callback(btmw_rpc_test_a2dp_src_bms_app_cbk, NULL);
#endif
        #if 1
        BT_A2DP_SRC_INIT_CONFIG init_config;
        memset(&init_config, 0, sizeof(BT_A2DP_SRC_INIT_CONFIG));
        init_config.max_connected_audio_devices = 2;
        init_config.codec_config_list.codec_config_num =1;
        init_config.codec_config_list.codec_config_list[0].bits_per_sample =
        BT_A2DP_CODEC_BITS_PER_SAMPLE_16;
        init_config.codec_config_list.codec_config_list[0].channel_mode =
        BT_A2DP_CODEC_CHANNEL_MODE_STEREO;
        init_config.codec_config_list.codec_config_list[0].channel_num = 1;
        init_config.codec_config_list.codec_config_list[0].sample_rate =
        BT_A2DP_CODEC_SAMPLE_RATE_48000;
        init_config.codec_config_list.codec_config_list[0].codec_priority =
        BT_A2DP_CODEC_PRIORITY_DEFAULT;
        init_config.codec_config_list.codec_config_list[0].codec_type =
        BT_A2DP_CODEC_TYPE_SBC;
        a_mtkapi_a2dp_src_enable(TRUE, &init_config);
        #endif
    }
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK

#else
    a2dp_hw_module_handle = dlopen("libaudio.a2dp.default.so", RTLD_LAZY);
    if (!a2dp_hw_module_handle)
    {
        BTMW_RPC_TEST_Loge("libaudio.a2dp.default.so open fail(%s)", dlerror());
        return -1;
    }
    g_rpc_audio_module = (struct audio_module *)dlsym(a2dp_hw_module_handle, HAL_MODULE_INFO_SYM_AS_STR);
    if (g_rpc_audio_module == NULL)
    {
        BTMW_RPC_TEST_Loge("[A2DP] Failed to get Audio A2DP interface\n");
        return -1;
    }
#endif
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    ret = a2dp_src_init_dl_function();
    if(ret != 0)
    {
        return -1;
    }

    //init rpc
    ret = a2dp_rpc_env_init();
    if(ret != 0)
    {
        BTMW_RPC_TEST_Loge("_rpc_env_init fail %d\n", ret);
        return -1;
    }
#endif
    return ret;
}

int btmw_rpc_test_a2dp_src_deinit(void)
{
    if (!g_cli_pts_mode)
    {
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
        auc.channel_num = 0;
        auc.sample_rate = 0;
        auc.bst = BT_BMS_STOPPED;
        auc.src_st = BT_A2DP_EVENT_DISCONNECTED;
        auc.snk_st = BT_A2DP_EVENT_DISCONNECTED;
        auc.leaudio_uploader_in_use = FALSE;
        auc.a2dpsrc_uploader_in_use = FALSE;
        auc.a2dpsnk_playback_in_use = FALSE;
        a_mtkapi_a2dp_unregister_callback(btmw_rpc_test_a2dp_src_app_cbk, NULL);
#endif
    }
    //Keep adev open in case that sink need it.
    if (gp_rpc_audio_dev)
    {
        gp_rpc_audio_dev->common.close(&gp_rpc_audio_dev->common);
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
        dlclose(handle);
#else
        if(a2dp_hw_module_handle != NULL)
        {
            dlclose(a2dp_hw_module_handle);
        }
#endif
        gp_rpc_audio_dev = NULL;
    }
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (fHandle1 != NULL)
    {
        (void)dlclose(fHandle1);
    }
    if (fHandle2 != NULL)
    {
        (void)dlclose(fHandle2);
    }
    if (app_handle != NULL)
    {
        (void)dlclose(app_handle);
    }
#endif
    a_mtkapi_a2dp_src_enable(FALSE, NULL);

    return 0;
}


