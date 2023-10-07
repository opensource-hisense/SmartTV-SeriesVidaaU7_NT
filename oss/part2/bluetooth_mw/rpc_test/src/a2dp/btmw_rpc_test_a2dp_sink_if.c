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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include "mtk_audio.h"
#include <pthread.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_a2dp_sink_if.h"
#include "mtk_bt_service_a2dp_wrapper.h"
//newly add for rpctest directly call audioplayback so
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#include "aud_a2dp_playback.h"
#endif
#include "mtk_bt_service_leaudio_bms_wrapper.h"
#include "u_bt_mw_avrcp.h"

#define BTMW_RPC_TEST_CMD_KEY_A2DP_SINK        "MW_RPC_A2DP_SINK"
#define PCM_BUFFER_SIZE 512*8     // 512x8 = 4096
#define BTMW_RPC_BMS_CASE_RETURN_STR(const) case const: return #const;

#define MAX_OUTPUT_FILE_NAME 128
FILE *outputPcmSampleFile = NULL;
static const char* defaultOutputFilename = "/tmp/bluedroid_output_sample.pcm";
CHAR outputFilename[MAX_OUTPUT_FILE_NAME];
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#else
extern struct audio_module *g_rpc_audio_module;
#endif

static struct audio_stream_in *gp_rpc_stream_in = NULL;
static pthread_t g_rpc_streamIn_thread = 0;
static BOOL g_rpc_streamThread_running = FALSE;

//extern struct audio_module *g_rpc_audio_module;
extern struct audio_hw_device *gp_rpc_audio_dev;
extern struct a2dp_leaudio_uploader_config_t auc;
extern struct socketidx_map_uploaderhdl_t smu_t[BT_BROADCAST_SOCKET_INDEX_NUM];
extern BT_BMS_EVENT_SOCKET_INDEX_DATA _gbms_socket_t;


#define BTMW_RPC_A2DP_CASE_RETURN_STR(const) case const: return #const;

CHAR g_rpc_a2dp_connect_test_addr[18] = {0};
int connect_state = -1;
static INT32 atuo_sink_test_cnt = 0;
static INT32 atuo_sink_connecet_thread_exit = 0;
static pthread_t g_a2dp_auto_sink_test_thread = 0;
static pthread_mutex_t g_write_char_mutex = PTHREAD_MUTEX_INITIALIZER;


// CLI handler
static int btmw_rpc_test_a2dp_sink_connect_int_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_disconnect_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_active_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_get_src_dev_list_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_codec_enable_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_active_src_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_get_active_src_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_set_flag_value_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_test_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_set_link_num(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_set_delay_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_low_power(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_dump_handler(int argc, char *argv[]);
static void btmw_rpc_test_a2dp_sink_close_input_stream(void);
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
static VOID btmw_rpc_test_a2dp_sink_report_player_event(BT_A2DP_PLAYER_EVENT event);
extern long start_leaudio_uploader(UINT8 socket_idx, UINT8 channel_num);
#endif

static void* gattc_a2dp_auto_sink_test_thread(void *arg)
{
    while (1)
    {
        usleep(1000*1000);
        if (TRUE == atuo_sink_connecet_thread_exit)
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
            atuo_sink_test_cnt++;
            if (atuo_sink_test_cnt < 1000)
            {
                a_mtkapi_a2dp_connect(g_rpc_a2dp_connect_test_addr, BT_A2DP_ROLE_SINK);
            }
            else
            {
                BTMW_RPC_TEST_Loge("auto test connect success!");
                break;
            }
        }
    };
    g_a2dp_auto_sink_test_thread = -1;
    atuo_sink_connecet_thread_exit = false;
    return NULL;
}

static int btmw_rpc_test_a2dp_sink_connect_auto_test_int_handler(int argc, char *argv[])
{
    CHAR *ptr;
    INT32 i4_ret = 0;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);
    atuo_sink_connecet_thread_exit = 0;
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
        if (0 != (i4_ret = pthread_create(&g_a2dp_auto_sink_test_thread,
                                          &attr,
                                          gattc_a2dp_auto_sink_test_thread,
                                          NULL)))
        {
            //BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_create i4_ret:%ld", (long)i4_ret);
            printf("[GATTC] pthread_create i4_ret:%d\n", i4_ret);
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
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SINK);
    return 0;
}

static int btmw_rpc_test_a2dp_get_connected_dev_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[A2DP] %s() \n", __func__);
    int i = 0;
    BT_A2DP_CONNECT_DEV_INFO_LIST dev_list;
    memset(&dev_list, 0, sizeof(BT_A2DP_CONNECT_DEV_INFO_LIST));

    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_connected_dev no param");
        return -1;
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
    }
    return 0;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_a2dp_sink_cli_commands[] =
{
    {"connect",         btmw_rpc_test_a2dp_sink_connect_int_handler, " = connect <addr>"},
    {"auto_connect",    btmw_rpc_test_a2dp_sink_connect_auto_test_int_handler, " = auto_connect <addr>"},
    {"disconnect",      btmw_rpc_test_a2dp_sink_disconnect_handler,  " = disconnect <addr>"},
    {"active_sink",     btmw_rpc_test_a2dp_sink_active_handler,      " = active_sink <1:enable|0:disable>"},
    {"get_paired_dev",  btmw_rpc_test_a2dp_get_src_dev_list_handler, " = get_paired_dev"},
    {"codec_enable",    btmw_rpc_test_a2dp_codec_enable_handler,     " = codec_enable <codec> <1/0>"},
    {"active_src",      btmw_rpc_test_a2dp_sink_active_src_handler,  " = active_src <addr>"},
    {"get_active_src",  btmw_rpc_test_a2dp_sink_get_active_src_handler,  " = get_active_src"},
    {"link_num",        btmw_rpc_test_a2dp_set_link_num,             " = link_num <sink_num>"},
    {"set",             btmw_rpc_test_a2dp_sink_set_flag_value_handler,  " = set <flag> <value>"},
    {"test",            btmw_rpc_test_a2dp_sink_test_handler, " = test <enable|load|unload|...>"},
    {"set_delay",       btmw_rpc_test_a2dp_sink_set_delay_handler, " = set_delay [addr]<value>"},
    {"set_low_power",   btmw_rpc_test_a2dp_sink_low_power,           " = set_low_power <1:enable|0:disable>"},
    /*if dump enable, rpctest read data; otherwise, use btplayback so!*/
    {"dump",            btmw_rpc_test_a2dp_sink_dump_handler,        " = dump ([1:enable 0:disable])"},
    {"get_connected_dev", btmw_rpc_test_a2dp_get_connected_dev_handler,   " get_connected_dev"},
    {NULL, NULL, NULL},
};

static CHAR* btmw_rpc_test_a2dp_sink_app_role(BT_A2DP_ROLE local_role)
{
    switch((int)local_role)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ROLE_SRC)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ROLE_SINK)
        default: return "UNKNOWN_ROLE";
   }
}

static CHAR* btmw_rpc_test_a2dp_sink_player_event_str(BT_A2DP_PLAYER_EVENT player_event)
{
    switch((int)player_event)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP_FAIL)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START_FAIL)
        default: return "UNKNOWN_EVENT";
   }
}

static CHAR* btmw_rpc_test_a2dp_get_codec_str(UINT8 codec_tpye)
{
     switch(codec_tpye)
     {
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_SBC)
         //BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_MP3)
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_AAC)
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_LDAC)
         default: return "UNKNOWN_CODEC_TYPE";
    }
}

CHAR g_rpc_a2dp_addr_test[18];
static BT_A2DP_ROLE g_rpc_a2dp_local_role = BT_A2DP_ROLE_SRC;
//static BT_A2DP_EVENT_SINK_AUDIO_CONFIG_DATA g_rpc_a2dp_sink_cfg = {0};
BT_A2DP_STREAM_STATE g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;

static int btmw_rpc_test_a2dp_sink_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active_sink [1:enable|0:disable]");
        return -1;
    }

    u1_enable = atoi(argv[0]);
    if (!u1_enable)
    {
        BTMW_RPC_TEST_Logi("%s() need exit auo-test!\n", __func__);
        atuo_sink_connecet_thread_exit = 1;
    }
    a_mtkapi_a2dp_sink_enable(u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_get_src_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = a_mtkapi_a2dp_sink_get_dev_list(&device_list);
    BTMW_RPC_TEST_Logi("get paired src device list result:\n", __func__);
    BTMW_RPC_TEST_Logi("======================================\n");
    if(BT_SUCCESS == ret)
    {
        if (device_list.is_snklp)
            BTMW_RPC_TEST_Logi("A2dp Sink Lowpower Mode\n");
        else
            BTMW_RPC_TEST_Logi("A2dp Sink Normal Mode\n");
        BTMW_RPC_TEST_Logi("======================================\n");
        if (0 == device_list.dev_num)
        {
            BTMW_RPC_TEST_Logi("no paired src device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_RPC_TEST_Logi("device[%d]: %s, name:%s, role:%s\n",
                    i, device_list.dev[i].addr, device_list.dev[i].name,
                    btmw_rpc_test_a2dp_sink_app_role(device_list.dev[i].role));
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


static int btmw_rpc_test_a2dp_codec_enable_handler(int argc, char *argv[])
{
    CHAR *codec = NULL;
    INT32 codec_type = 0xFF;
    UINT8 u1_enable = 0;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] codec_enable <aac/aptx> <1/0>");
        return -1;
    }

    codec = argv[0];
    u1_enable = atoi(argv[1]);
    /*
    if (!((strncmp(codec, "aac", sizeof("aac")) == 0) || \
        (strncmp(codec, "aptx", sizeof("aptx")) == 0)))
    {
        BTMW_RPC_TEST_Logi("error codec");
        BTMW_RPC_TEST_Loge("[USAGE] codec_enable <aac/aptx> <1/0>");
        return -1;
    }
*/
    BTMW_RPC_TEST_Logi("codec: %s\n", codec);
    if (strncmp(codec, "aac", sizeof("aac")) == 0)
    {
        codec_type = BT_A2DP_CODEC_TYPE_AAC;
    }
    else
    {
        BTMW_RPC_TEST_Loge("currently not support this codec");
    }

    BTMW_RPC_TEST_Logi("to %s %s(%ld) ......\n", u1_enable ? "enable" : "disable", codec, codec_type);

    a_mtkapi_a2dp_codec_enable(codec_type, u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_low_power(int argc, char *argv[])
{
    UINT8 u1_enable = 0;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_low_power <1/0>");
        return -1;
    }

    u1_enable = atoi(argv[0]);

    BTMW_RPC_TEST_Logi("to %s  ......\n", u1_enable ? "enable" : "disable");

    a_mtkapi_a2dp_sink_lowpower_enable(u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_set_link_num(int argc, char *argv[])
{
    INT32 sink_num = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] link_num <sink_num>");
        return -1;
    }

    sink_num = atoi(argv[0]);
    BTMW_RPC_TEST_Logi("sink_num=%d\n", sink_num);
    a_mtkapi_a2dp_sink_set_link_num(sink_num);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_active_src_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active src <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] active src <addr>");
        return -1;
    }

    ptr = argv[0];
    BTMW_RPC_TEST_Logi("src active %s\n", ptr);
    a_mtkapi_a2dp_sink_active_src(ptr);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_get_active_src_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_active_src");
        return -1;
    }
    CHAR *ptr = NULL;
    CHAR addr[18] = {0};
    ptr = addr;

    BTMW_RPC_TEST_Logi("sink get active src \n");
    a_mtkapi_a2dp_sink_get_active_src(ptr);
    BTMW_RPC_TEST_Logi("%s sink get active src \n", ptr);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_set_flag_value_handler(int argc, char *argv[])
{
    BT_A2DP_DBG_PARAM param;
    INT32 flag = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set <flag> <value>");
        return -1;
    }

    flag = atoi(argv[0]);
    param.wait_time_ms = atoi(argv[1]);
    BTMW_RPC_TEST_Logi("set flag=%d, value=%d\n", flag, param.wait_time_ms);
    a_mtkapi_a2dp_set_dbg_flag(flag, &param);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_test_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (strcmp(argv[0], "enable") == 0)
    {
        INT32 test_cnt = atoi(argv[1]);

        while(test_cnt > 0)
        {
            a_mtkapi_a2dp_sink_enable(1);
            a_mtkapi_a2dp_src_enable(1, NULL);
            a_mtkapi_a2dp_sink_enable(0);
            a_mtkapi_a2dp_src_enable(0, NULL);
            test_cnt --;
        }
        a_mtkapi_a2dp_sink_enable(1);
        a_mtkapi_a2dp_src_enable(1, NULL);
    }
    else if (strcmp(argv[0], "load") == 0)
    {
        //a_mtkapi_a2dp_sink_player_load(argv[1]);
    }
    else if (strcmp(argv[0], "unload") == 0)
    {
        //a_mtkapi_a2dp_sink_player_unload(argv[1]);
    }

    return 0;
}

static int btmw_rpc_test_a2dp_sink_set_delay_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;
    UINT32 delay_value = 0;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if ((argc != 1) && (argc != 2))
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_delay [addr]<value>");
        return -1;
    }

    if (argc == 1)
    {
        delay_value = atoi(argv[0]);
        BTMW_RPC_TEST_Logi("A2DP not set addr, delay:%d\n", delay_value);
    }
    else if (argc == 2)
    {
        if (strlen(argv[0]) < 17)
        {
            BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
            BTMW_RPC_TEST_Loge("[USAGE] set_delay [addr]<value>");
            return -1;
        }

        ptr = argv[0];
        delay_value = atoi(argv[1]);
        BTMW_RPC_TEST_Logi("A2DP set %s, delay:%d\n", ptr, delay_value);
    }

    a_mtkapi_a2dp_sink_set_delay_value(ptr, delay_value);
    return 0;
}

static int btmw_rpc_test_a2dp_sink_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

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
    BTMW_RPC_TEST_Logi("A2DP connected to %s\n", ptr);
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SINK);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    btmw_rpc_test_a2dp_sink_close_input_stream();
    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
    }

    ptr = argv[0];
    BTMW_RPC_TEST_Logi("A2DP disconnected to %s\n", ptr);
    a_mtkapi_a2dp_disconnect(ptr);
    return 0;
}

static int  btmw_rpc_test_a2dp_sink_cmd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_rpc_test_a2dp_sink_cli_commands;
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

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_A2DP_SINK, btmw_rpc_test_a2dp_sink_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static CHAR* btmw_rpc_test_a2dp_sink_app_event(BT_A2DP_EVENT event)
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
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_SRC_AUDIO_CONFIG)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_SINK_AUDIO_CONFIG)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_ACTIVE_CHANGED)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_DELAY)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_DATA_IND)
        default: return "UNKNOWN_EVENT";
   }
}

// input stream
static int btmw_rpc_test_a2dp_sink_dump_handler(int argc, char *argv[])
{
    int enable = 0;
    int size;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("  A2DP_SINK dump <0/1> \n");
        BTMW_RPC_TEST_Logi("  or A2DP_SINK dump <0/1> <FileName>\n");
        return -1;
    }
    enable = atoi(argv[0]);

    if (argc == 2)
    {
        size = strlen(argv[1]);
        if (size >= MAX_OUTPUT_FILE_NAME - 1)
        {
            size = MAX_OUTPUT_FILE_NAME - 1;
        }
        strncpy(outputFilename, argv[1], size);
        outputFilename[MAX_OUTPUT_FILE_NAME - 1] = '\0';
    }
    else
    {
        size = sizeof(defaultOutputFilename);
        if (size >= MAX_OUTPUT_FILE_NAME - 1)
        {
            size = MAX_OUTPUT_FILE_NAME - 1;
        }
        strncpy(outputFilename, defaultOutputFilename, size);
        outputFilename[MAX_OUTPUT_FILE_NAME - 1] = '\0';
    }

    if (outputPcmSampleFile)
    {
        (void)fclose(outputPcmSampleFile);
    }
    outputPcmSampleFile = NULL;

    if (enable)
    {
        BTMW_RPC_TEST_Logi("[A2DP] pcm dump at %s\n", outputFilename);
        outputPcmSampleFile = fopen(outputFilename, "wb+");
        if (outputPcmSampleFile == NULL)
        {
            BTMW_RPC_TEST_Loge("Can't open outputFilename!\n");
             return -1;
        }
    } else
    {
        BTMW_RPC_TEST_Logi("[A2DP] pcm dump is not enabled at %s\n", outputFilename);
    }
    return 0;
}

static void* btmw_rpc_test_a2dp_sink_socket_read_handler(void *arg) {
    UINT8 pcm_buffer[PCM_BUFFER_SIZE];
    INT32 pcm_bytes = PCM_BUFFER_SIZE;
    INT32 pcm_read = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    if (gp_rpc_stream_in == NULL)
    {
        BTMW_RPC_TEST_Logi("%s() gp_rpc_stream_in is NULL\n", __func__);
        return NULL;
    }

    g_rpc_streamThread_running = TRUE;

    while (g_rpc_streamThread_running) {
        //pthread_mutex_lock(&g_write_char_mutex);
        //BTMW_RPC_TEST_Logi("read data before %s()\n", __func__);
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE*sizeof(UINT8));
        pcm_read = gp_rpc_stream_in->read(gp_rpc_stream_in, pcm_buffer, pcm_bytes);
        if (pcm_read <= 0) {
            BTMW_RPC_TEST_Logw("[A2DP] socket read error= %d\n", pcm_read);
        }
        //BTMW_RPC_TEST_Logi("read data %s()\n", __func__);
        //pthread_mutex_unlock(&g_write_char_mutex);
        if (outputPcmSampleFile && pcm_read > 0)
        {
            ret = fwrite(pcm_buffer, 1, (size_t)pcm_read, outputPcmSampleFile);
            if (ret <= 0)
            {
                BTMW_RPC_TEST_Logi("%s() fwrite error(%s)\n", __func__, strerror(errno));
                (void)fclose(outputPcmSampleFile);
                return NULL;
            }
        }
    }
    pthread_mutex_lock(&g_write_char_mutex);
    BTMW_RPC_TEST_Logi("exit while %s()\n", __func__);
    if(gp_rpc_stream_in != NULL)
    {
        gp_rpc_audio_dev->close_input_stream(gp_rpc_audio_dev, gp_rpc_stream_in);
        gp_rpc_stream_in = NULL;
    }
    pthread_mutex_unlock(&g_write_char_mutex);
    g_rpc_streamIn_thread = 0;
    return NULL;
}

static void btmw_rpc_test_a2dp_sink_open_input_stream(void) {
    int ret;
    //UINT32 sample_rate = 0;
    hw_device_t* device = NULL;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#else
    if (gp_rpc_audio_dev == NULL)
    {
        if (0 > g_rpc_audio_module->common.methods->open(NULL, AUDIO_HARDWARE_INTERFACE, &device))
        {
            BTMW_RPC_TEST_Loge("open bt adev fail\n");
            return;
        }
        gp_rpc_audio_dev = (struct audio_hw_device *)device;
    }
#endif
    ret = gp_rpc_audio_dev->open_input_stream(gp_rpc_audio_dev, 0, 0, NULL,
                   &gp_rpc_stream_in, AUDIO_INPUT_FLAG_NONE, NULL, AUDIO_SOURCE_DEFAULT);
    if (ret < 0) {
        BTMW_RPC_TEST_Loge("Open input stream fail (err = %d)!\n", ret);
        return;
    }
    //sample_rate = (UINT32)gp_rpc_stream_in->common.get_sample_rate(gp_rpc_stream_in);
    //BTMW_RPC_TEST_Loge("in_get_sample_rate sample_rate=%d!\n", sample_rate);
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (g_rpc_streamIn_thread == 0)
    {
        if ( pthread_create(&g_rpc_streamIn_thread, &thread_attr, btmw_rpc_test_a2dp_sink_socket_read_handler, NULL)!=0 )
        {
            BTMW_RPC_TEST_Loge("pthread_create : %s", strerror(errno));
            return;
        }
        BTMW_RPC_TEST_Logi("pthread_create : %d", g_rpc_streamIn_thread);
    }

    return;

}

static void btmw_rpc_test_a2dp_sink_close_input_stream(void) {

    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    g_rpc_streamThread_running = FALSE;
    if (g_rpc_streamIn_thread) {
        BTMW_RPC_TEST_Loge("set g_rpc_streamThread_running NULL)!\n");
    }
    #if 0
    pthread_mutex_lock(&g_write_char_mutex);
    BTMW_RPC_TEST_Logi("%s()check 0\n", __func__);
    if(gp_rpc_stream_in != NULL)
    {
        gp_rpc_audio_dev->close_input_stream(gp_rpc_audio_dev, gp_rpc_stream_in);
        gp_rpc_stream_in = NULL;
    }
    BTMW_RPC_TEST_Logi("%s()check\n", __func__);
    pthread_mutex_unlock(&g_write_char_mutex);
    #endif
    return;
}

static VOID btmw_rpc_test_a2dp_sink_app_cbk(BT_A2DP_EVENT_PARAM *param, VOID *pv_tag)
{
    UINT8 codec_type = 0;
    UINT8 i = 0;
    UINT8 codec_config_num = 0;

    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_rpc_test_a2dp_sink_app_event(param->event));
    switch (param->event)
    {
        case BT_A2DP_EVENT_CONNECTED:
            connect_state = 1;
            BTMW_RPC_TEST_Logd("A2DP connected(%s)\n", param->addr);
            BTMW_RPC_TEST_Logd("local_role=%s, sample rate=%d, channel num=%d\n",
                btmw_rpc_test_a2dp_sink_app_role(param->data.connected_data.local_role),
                param->data.connected_data.sample_rate,
                param->data.connected_data.channel_num);
            strncpy(g_rpc_a2dp_addr_test, param->addr, sizeof(g_rpc_a2dp_addr_test));
            g_rpc_a2dp_addr_test[17] = '\0';
            g_rpc_a2dp_local_role = param->data.connected_data.local_role;

            codec_type = param->data.connected_data.config.codec_type;
            BTMW_RPC_TEST_Logd("current codec is:%s(%d)\n", \
                btmw_rpc_test_a2dp_get_codec_str(codec_type), \
                codec_type);
            BTMW_RPC_TEST_Logd("call a_mtkapi_a2dp_src_active_sink(%s)\n", param->addr);
            if (g_rpc_a2dp_local_role == BT_A2DP_ROLE_SINK)
            {
                a_mtkapi_a2dp_sink_active_src(param->addr);
                g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
                auc.snk_st = BT_A2DP_EVENT_CONNECTED;
                auc.sample_rate = param->data.connected_data.sample_rate;
                auc.channel_num = param->data.connected_data.channel_num;
                /*if (!g_cli_pts_mode && outputPcmSampleFile == NULL && !auc.leaudio_uploader_in_use && !auc.a2dpsrc_uploader_in_use)
                {
                    BTMW_RPC_TEST_Logd("init&start pb! %d-%d\n", param->data.connected_data.sample_rate, param->data.connected_data.channel_num);
                    bt_a2dp_alsa_pb_init(btmw_rpc_test_a2dp_sink_report_player_event);
                    bt_a2dp_alsa_player_start(auc.sample_rate, auc.channel_num, 16);
                    auc.a2dpsnk_playback_in_use = TRUE;
                }*/
#endif
            }
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#else
            if (g_rpc_a2dp_local_role == BT_A2DP_ROLE_SRC)
            {
                a_mtkapi_a2dp_src_active_sink(param->addr);
            }
#endif

            break;
        case BT_A2DP_EVENT_DISCONNECTED:
            BTMW_RPC_TEST_Logd("A2DP disconnected(%s)\n", param->addr);
            g_rpc_a2dp_addr_test[0] = 0;
            connect_state = 0;
            g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile == NULL)
            {
                auc.snk_st = BT_A2DP_EVENT_DISCONNECTED;
                if (auc.a2dpsnk_playback_in_use) {
                    bt_a2dp_alsa_player_stop();
                    bt_a2dp_alsa_pb_deinit();
                    auc.a2dpsnk_playback_in_use = FALSE;
                    BTMW_RPC_TEST_Logd("Line%d, stop&deinit audio playback!\n", __LINE__);
                }
            }
#endif
            break;
        case BT_A2DP_EVENT_CONNECT_TIMEOUT:
            BTMW_RPC_TEST_Logd("A2DP Connect Timeout(%s)\n", param->addr);
            g_rpc_a2dp_addr_test[0] = 0;
            break;
        case BT_A2DP_EVENT_STREAM_SUSPEND:
            g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;
            if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile != NULL)
            {
                BTMW_RPC_TEST_Logd("close input stream!\n");

                btmw_rpc_test_a2dp_sink_close_input_stream();
            }
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            else if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile == NULL)
            {
                if (auc.a2dpsnk_playback_in_use) {
                    bt_a2dp_alsa_player_pause();
                    BTMW_RPC_TEST_Logd("pause audio playback!\n");
                }
            }

            if (auc.bst == BT_BMS_STREAMING && auc.a2dpsnk_playback_in_use)
            {
                BTMW_RPC_TEST_Logd("++ %d broadcast state streaming!\n", __LINE__);
                BTMW_RPC_TEST_Logd("stop&deinit audio playback !!\n");
                bt_a2dp_alsa_player_stop();
                bt_a2dp_alsa_pb_deinit();
                auc.a2dpsnk_playback_in_use = FALSE;
                BTMW_RPC_TEST_Logd("++ socket_index_num %d!\n", _gbms_socket_t.socket_index_num);

                for (UINT8 i = 0; i < _gbms_socket_t.socket_index_num; i++) {

                    BTMW_RPC_TEST_Logd("[BMS++] i=%d: socket_index=%d, channel_num=%d, subgroup_num=%d", i,
                      _gbms_socket_t.socket_index_list[i].socket_index,
                      _gbms_socket_t.socket_index_list[i].channel_num,
                      _gbms_socket_t.socket_index_list[i].subgroup_num);

                    for (UINT8 j = 0; j < _gbms_socket_t.socket_index_list[i].subgroup_num; j++)
                    {
                        BTMW_RPC_TEST_Logd("[BMS++] subgroup[%d]=%d", j,
                          _gbms_socket_t.socket_index_list[i].subgroup_ids[j]);
                    }

                    smu_t[i].socketidx = (audio_io_handle_t)_gbms_socket_t.socket_index_list[i].socket_index;
                    if (!auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
                    {
                        long hUploaderhandle = start_leaudio_uploader(
                          _gbms_socket_t.socket_index_list[i].socket_index,
                          _gbms_socket_t.socket_index_list[i].channel_num);
                        if (hUploaderhandle == -1) continue;
                        smu_t[i].hUploaderhandle = hUploaderhandle;
                        smu_t[i].in_use = TRUE;
                        auc.leaudio_uploader_in_use = TRUE;
                    }
                }

                if (!auc.leaudio_uploader_in_use)
                {
                    LEaudio_uploader_deinit();//if cannot start any one uploader, so deinit!
                }
            }
#endif
            break;
        case BT_A2DP_EVENT_STREAM_START:
            g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_PLAYING;
            if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile != NULL)
            {
                BTMW_RPC_TEST_Logd("open input stream!\n");
                //a_mtkapi_a2dp_sink_start_player();
                btmw_rpc_test_a2dp_sink_open_input_stream();
            }
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            else if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile == NULL && auc.bst != BT_BMS_STREAMING)
            {
                if (auc.a2dpsnk_playback_in_use && !auc.leaudio_uploader_in_use) {
                    bt_a2dp_alsa_player_play();
                    BTMW_RPC_TEST_Logd("Line %d, start audio playback!\n", __LINE__);
                } else if (!auc.a2dpsnk_playback_in_use && !auc.leaudio_uploader_in_use) {
                    BTMW_RPC_TEST_Logd("init&start audio playback, %d--%d!\n", auc.sample_rate, auc.channel_num);
                    bt_a2dp_alsa_pb_init(btmw_rpc_test_a2dp_sink_report_player_event);
                    bt_a2dp_alsa_player_start(auc.sample_rate, auc.channel_num, 16);
                    auc.a2dpsnk_playback_in_use = TRUE;
                    bt_a2dp_alsa_player_play();
                }
            }
            else if (!g_cli_pts_mode && (BT_A2DP_ROLE_SINK == g_rpc_a2dp_local_role) && outputPcmSampleFile == NULL && auc.bst == BT_BMS_STREAMING)
            {
                a_mtkapi_avrcp_send_passthrough_cmd(param->addr, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);
                usleep(1000*10);
                a_mtkapi_avrcp_send_passthrough_cmd(param->addr, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);
                BTMW_RPC_TEST_Logd("cannot audio playback and send pause key to src dev!\n");
            }
#endif
            break;
        case BT_A2DP_EVENT_CONNECT_COMING:
            break;
        case BT_A2DP_EVENT_PLAYER_EVENT:
            BTMW_RPC_TEST_Logd("player event %s(%d)\n",
                btmw_rpc_test_a2dp_sink_player_event_str(param->data.player_event),
                param->data.player_event);
            break;
        case BT_A2DP_EVENT_ROLE_CHANGED:
            BTMW_RPC_TEST_Logd("%s role change %s\n",
                btmw_rpc_test_a2dp_sink_app_role(param->data.role_change.role),
                param->data.role_change.enable?"enable":"disable");
            break;
        case BT_A2DP_EVENT_SINK_AUDIO_CONFIG:
            BTMW_RPC_TEST_Logd("A2DP sink audio config(%s)\n", param->addr);

            //g_rpc_a2dp_sink_cfg.channel_num = param->data.sink_audio_config.channel_num;
            //g_rpc_a2dp_sink_cfg.sample_rate = param->data.sink_audio_config.sample_rate;
            BTMW_RPC_TEST_Logd("A2DP SINK audio config addr (%s)\n", param->addr);
            BTMW_RPC_TEST_Logd("A2DP SINK audio config sample rate(%d)\n",
                param->data.sink_audio_config.sample_rate);
            BTMW_RPC_TEST_Logd("A2DP SINK audio config channel(%d)\n",
                param->data.sink_audio_config.channel_num);
            BTMW_RPC_TEST_Logd("A2DP SINK audio config codec type(%d)\n",
                param->data.sink_audio_config.codec_type);
            //bt_a2dp_alsa_player_start(param->data.sink_audio_config.sample_rate, param->data.sink_audio_config.channel_num, 16);
            //bt_a2dp_alsa_player_start(g_rpc_a2dp_sink_cfg.sample_rate, g_rpc_a2dp_sink_cfg.channel_num, 16);

            break;
        case BT_A2DP_EVENT_ACTIVE_CHANGED:
            BTMW_RPC_TEST_Logd("A2DP  BT_A2DP_EVENT_ACTIVE_CHANGED(%s)\n", param->addr);
            break;
        default:
            break;
    }
    return;
}

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
//refer bt_mw_a2dp_sink_report_player_event
static VOID btmw_rpc_test_a2dp_sink_report_player_event(BT_A2DP_PLAYER_EVENT event)
{
    if (BT_A2DP_ALSA_PB_EVENT_START == event)
    {
        BTMW_RPC_TEST_Logd("A2DP SINK  BT_A2DP_ALSA_PB_EVENT_START\n");
    }
    else if (BT_A2DP_ALSA_PB_EVENT_STOP == event)
    {
        BTMW_RPC_TEST_Logd("A2DP SINK  BT_A2DP_ALSA_PB_EVENT_STOP\n");
    }
}

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

static VOID btmw_rpc_test_a2dp_sink_bms_app_cbk(BT_BMS_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BT_A2DP_CONNECT_DEV_INFO_LIST dev_list;
    int i=0;

    switch (param->event)
    {
    case BT_BMS_EVENT_BROADCAST_STATE:
      BTMW_RPC_TEST_Logd("event=%d - %s, broadcast state %d - %s\n",
                          param->event,
                          btmw_rpc_test_bms_app_event(param->event),
                          param->data.broadcast_state.state,
                          btmw_rpc_test_bms_broadcast_state(param->data.broadcast_state.state));

        memset(&dev_list, 0, sizeof(BT_A2DP_CONNECT_DEV_INFO_LIST));
        a_mtkapi_a2dp_get_connected_dev_list(&dev_list);
        BTMW_RPC_TEST_Logi("[A2DP SINK] %s() dev_num=%d\n", __func__, dev_list.dev_num);

        for (i = 0; i < dev_list.dev_num; i++)
        {
            BTMW_RPC_TEST_Logi("[A2DP SINK] device %d 's addr=%s\n",
                i, dev_list.a2dp_connected_dev_list[i].addr);
        }

        if (param->data.broadcast_state.state == BT_BMS_STREAMING && auc.a2dpsnk_playback_in_use)
        {
            /* stop a2dp sink playback */
            BTMW_RPC_TEST_Logd("++ %d broadcast state streaming!\n", __LINE__);
            BTMW_RPC_TEST_Logd("stop&deinit audio playback !!\n");
            bt_a2dp_alsa_player_pause();
            bt_a2dp_alsa_player_stop();
            bt_a2dp_alsa_pb_deinit();
            auc.a2dpsnk_playback_in_use = FALSE;

            for (i = 0; i < dev_list.dev_num; i++)
            {
                BTMW_RPC_TEST_Logd("device %d: set active to unactive and send pause cmd\n", i);

                /* set active to unactive */
                a_mtkapi_a2dp_sink_active_src("00:00:00:00:00:00");

                /* send avrcp pause command */
                if (dev_list.a2dp_connected_dev_list[i].local_role == BT_A2DP_ROLE_SINK
                    && dev_list.a2dp_connected_dev_list[i].conn_status == BT_A2DP_CONNECT_STATUS_CONNECTED)
                {
                    a_mtkapi_avrcp_send_passthrough_cmd(dev_list.a2dp_connected_dev_list[i].addr,
                        BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);
                    usleep(1000*10);
                    a_mtkapi_avrcp_send_passthrough_cmd(dev_list.a2dp_connected_dev_list[i].addr,
                        BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);
                }
            }
        }
        else if (param->data.broadcast_state.state != BT_BMS_STREAMING && !auc.a2dpsnk_playback_in_use)
        {
            for (i = 0; i < dev_list.dev_num; i++)
            {
                BTMW_RPC_TEST_Logd("device %d: set active to active and send play cmd\n", i);

                /* set active to active */
                a_mtkapi_a2dp_sink_active_src(dev_list.a2dp_connected_dev_list[i].addr);

                /* send avrcp play command */
                if (dev_list.a2dp_connected_dev_list[i].local_role == BT_A2DP_ROLE_SINK
                    && dev_list.a2dp_connected_dev_list[i].conn_status == BT_A2DP_CONNECT_STATUS_CONNECTED)
                {
                    a_mtkapi_avrcp_send_passthrough_cmd(dev_list.a2dp_connected_dev_list[i].addr,
                        BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);
                    usleep(1000*10);
                    a_mtkapi_avrcp_send_passthrough_cmd(dev_list.a2dp_connected_dev_list[i].addr,
                        BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);
                }
            }

            /* start a2dp sink playback */
            if (!auc.a2dpsnk_playback_in_use && !auc.leaudio_uploader_in_use &&
                (g_rpc_a2dp_stream_state == BT_A2DP_STREAM_STATE_PLAYING) &&
                (auc.snk_st == BT_A2DP_EVENT_CONNECTED)) {
                BTMW_RPC_TEST_Logd("init&start audio playback, %d--%d!\n", auc.sample_rate, auc.channel_num);
                bt_a2dp_alsa_pb_init(btmw_rpc_test_a2dp_sink_report_player_event);
                bt_a2dp_alsa_player_start(auc.sample_rate, auc.channel_num, 16);
                auc.a2dpsnk_playback_in_use = TRUE;
                bt_a2dp_alsa_player_play();
            }
        }
        break;

    default:
        break;
    }
    return;
}

#endif

INT32 btmw_rpc_test_a2dp_sink_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD a2dp_sink_mod = {0};

    a2dp_sink_mod.mod_id = BTMW_RPC_TEST_MOD_A2DP_SINK;
    strncpy(a2dp_sink_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_A2DP_SINK, sizeof(a2dp_sink_mod.cmd_key));
    a2dp_sink_mod.cmd_handler = btmw_rpc_test_a2dp_sink_cmd_handler;
    a2dp_sink_mod.cmd_tbl = btmw_rpc_test_a2dp_sink_cli_commands;

    ret = btmw_rpc_test_register_mod(&a2dp_sink_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for SINK returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        a_mtkapi_a2dp_register_callback(btmw_rpc_test_a2dp_sink_app_cbk, NULL);
        //a_mtkapi_bt_bms_register_callback(btmw_rpc_test_a2dp_sink_bms_app_cbk, NULL);
        a_mtkapi_a2dp_sink_enable(1);

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
        a_mtkapi_bt_bms_register_callback(btmw_rpc_test_a2dp_sink_bms_app_cbk, NULL);
#endif
    }

    return ret;
}


