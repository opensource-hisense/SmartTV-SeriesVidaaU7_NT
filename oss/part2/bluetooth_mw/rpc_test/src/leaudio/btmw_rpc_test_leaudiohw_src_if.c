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
#include <sys/prctl.h>

#include "mtk_audio.h" //need refactor
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
//#include "mtk_bt_service_a2dp_wrapper.h"

#include "btmw_rpc_test_leaudiohw_src_if.h"

#define BTMW_RPC_TEST_CMD_KEY_LEAUDIO_SRC        "MW_RPC_LEAUDIOHW_SRC"
#define PCM_BUFFER_SIZE 512*8     // at least 512
#define MAX_DATA_PATH 6

typedef enum {
  STREAM_STATUS_IDLE,
  STREAM_STATUS_STARTED,
  STREAM_STATUS_SUSPENDED,
} STREAM_STATUS;


typedef struct stream_test_t {
    bool in_use;
    audio_io_handle_t handle;
    CHAR test_pcm[128];
    pthread_t test_stream_handle;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    FILE *fInputPCM;//must diff from each other
    struct audio_stream_out *stream;
    struct audio_config *config;
    STREAM_STATUS st;
} stream_test_t;


static struct stream_test_t str_test_t[MAX_DATA_PATH] = {{0}};

//static pthread_t btmw_rpc_test_stream_handle = -1;
//static CHAR btmw_rpc_test_pcm_file[128] = "/data/bluedroid/lib/input_48000.pcm";
static const char leaudio_so_patch[128] = "libaudio.leaudio.default.so";
static struct hw_device_t* gdevice = NULL;

static void init_stream_test (void) {
    for (uint8_t i=0; i<MAX_DATA_PATH; i++) {
        str_test_t[i].in_use = false;
        str_test_t[i].handle = i;
        str_test_t[i].test_stream_handle = -1;
        //str_test_t[i].test_pcm = {0};
        memset(str_test_t[i].test_pcm, 0, 128);
        str_test_t[i].fInputPCM = NULL;
        str_test_t[i].config = NULL;
        str_test_t[i].stream = NULL;
        //str_test_t[i].mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&str_test_t[i].mutex, NULL);
        //str_test_t[i].cond = PTHREAD_COND_INITIALIZER;
        pthread_cond_init(&str_test_t[i].cond, NULL);
        pthread_mutex_lock(&str_test_t[i].mutex);
        str_test_t[i].st = STREAM_STATUS_IDLE;
        pthread_mutex_unlock(&str_test_t[i].mutex);
    }
}

static struct stream_test_t* get_stream_test_by_handle(audio_io_handle_t handle) {
    BTMW_RPC_TEST_Logd("handle %d into\n", handle);
    struct stream_test_t*  stt = NULL;
    if ((uint8_t)handle < MAX_DATA_PATH) {
        pthread_mutex_lock(&str_test_t[(uint8_t)handle].mutex);
        str_test_t[(uint8_t)handle].in_use = true;
        stt = &(str_test_t[(uint8_t)handle]);
        pthread_mutex_unlock(&str_test_t[(uint8_t)handle].mutex);
    }
    else
    {
        BTMW_RPC_TEST_Loge("can not Got handle: %d\n", handle);
        return NULL;
    }

    BTMW_RPC_TEST_Logd("Got by handle: %d\n", handle);
    return stt;
}

static struct stream_test_t* unalloc_stream_test_by_handle(audio_io_handle_t handle) {
    BTMW_RPC_TEST_Logd("handle %d into\n", handle);
    struct stream_test_t*  stt = NULL;
    if ((uint8_t)handle < MAX_DATA_PATH) {
        if(str_test_t[(uint8_t)handle].in_use)
            str_test_t[(uint8_t)handle].in_use = false;
        stt = &(str_test_t[(uint8_t)handle]);
    }
    else
    {
        BTMW_RPC_TEST_Loge("can not Got handle: %d\n", handle);
        return NULL;
    }

    BTMW_RPC_TEST_Logd("Got by handle: %d\n", handle);
    return stt;
}

static bool is_any_streaming() {
    bool ret = false;
    for (uint8_t i=0; i<MAX_DATA_PATH; i++) {
        if (str_test_t[i].in_use &&
            (str_test_t[i].test_stream_handle != -1)) {
            ret = true;
            break;
        }
    }

    BTMW_RPC_TEST_Logd("done, ret=%d\n", ret);
    return ret;
}

// CLI handler
static int btmw_rpc_test_leaudio_src_write_stream_handler(int argc, char *argv[]);
static int btmw_rpc_test_leaudio_src_suspend_stream_handler(int argc, char *argv[]);
static int btmw_rpc_test_leaudio_src_stop_stream_handler(int argc, char *argv[]);

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */
static int load_leaudio(const char path[], audio_hw_device_t **dev)
{
    if (gdevice != NULL) {
        BTMW_RPC_TEST_Logd("already got leaudio lib device!!");
        return 0;
    }
    int status = -EINVAL;
    void *handle = NULL;
    struct hw_module_t *hmi = NULL;
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    handle = dlopen(path, RTLD_LAZY);
    if (handle == NULL) {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Logd("load: module=%s\n%s", path, err_str?err_str:"unknown");
        status = -EINVAL;
        goto done;
    }

    /* Get the address of the struct hal_module_info. */
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
        }
    } else {
        BTMW_RPC_TEST_Logd("loaded HAL id=%s path=%s hmi=%p handle=%p",
                AUDIO_HARDWARE_MODULE_ID, path, hmi, handle);
    }

    if (hmi == NULL) {
      goto out;
    }

    status = audio_hw_device_open(hmi, (struct audio_hw_device **)dev);
    if (status) {
        BTMW_RPC_TEST_Logd("%s couldn't open audio leaudio hw device ", __func__);
        goto out;
    }
    if ((*dev)->common.version < AUDIO_DEVICE_API_VERSION_MIN) {
        BTMW_RPC_TEST_Logd("%s wrong audio hw device version %04x", __func__, (*dev)->common.version);
        status = -1;
        audio_hw_device_close((struct audio_hw_device *)*dev);
        goto out;
    }
    return 0;

out:
    *dev = NULL;
    return status;
}

static VOID* btmw_rpc_test_leaudio_write_audio_data_thread(VOID *arg)
{
    UINT8 *pcm_buffer = NULL;
    assert(arg);
    struct stream_test_t* in  = (struct stream_test_t*)arg;

    char thread_name[16] = {};
    (void)sprintf(thread_name, "la_wt_%d", in->handle);
    prctl(PR_SET_NAME, (unsigned long)thread_name, 0, 0, 0);

    pthread_mutex_lock(&in->mutex);
    //CHAR *local_test_pcm_file = in->test_pcm;
    //audio_io_handle_t handle = in->handle;
    //FILE *fInputPCM;
    //INT32 result=0;
    INT32 pcm_frame_len = PCM_BUFFER_SIZE;
    INT32 total_pcm_len;
    //UINT32 wlength = 2;
    //UINT32 channel_num = 2;
    //UINT32 sample_rate = 48000;
    int read_cnt = 0;
    //struct hw_device_t* device = NULL;
    struct audio_hw_device *hw_dev = NULL;
    //struct audio_stream_out *stream = NULL;

    BTMW_RPC_TEST_Logd("Input file name: %s, handle %d\n", in->test_pcm, in->handle);
    /* open file & allocate memory */
    in->fInputPCM = fopen(in->test_pcm, "rb");
    if (in->fInputPCM == NULL)
    {
        BTMW_RPC_TEST_Loge("Can't open input PCM file!\n");
        unalloc_stream_test_by_handle(in->handle);
        in->test_stream_handle = -1;
        pthread_mutex_unlock(&in->mutex);
        return NULL;
    }
    BTMW_RPC_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        (void)fclose(in->fInputPCM);
        BTMW_RPC_TEST_Loge("Can't allocat buffer\n");
        unalloc_stream_test_by_handle(in->handle);
        in->test_stream_handle = -1;
        pthread_mutex_unlock(&in->mutex);
        return NULL;
    }
    if (0 > load_leaudio(leaudio_so_patch, (audio_hw_device_t **)&gdevice))//hw_device_t
    {
        BTMW_RPC_TEST_Loge("open bt adev fail\n");
        goto send_audio_data_end;
    }
    hw_dev = (struct audio_hw_device *)gdevice;
    if (0 > hw_dev->open_output_stream(hw_dev, in->handle, 0, 0, in->config, &in->stream, 0))
    {
        BTMW_RPC_TEST_Loge("open out stream fail\n");
        goto send_audio_data_end;
    }

    in->st = STREAM_STATUS_STARTED;
    pthread_mutex_unlock(&in->mutex);
    while(read_cnt < 1 )
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, in->fInputPCM);
        if (total_pcm_len == 0)
        {
            //BTMW_RPC_TEST_Loge("total_pcm_len==0\n");
            (void)fseek(in->fInputPCM, 0L, SEEK_SET);
            //read_cnt++;

            continue;
        }
        //BTMW_RPC_TEST_Logd("write total_pcm_len %d!!\n", total_pcm_len);

        if (in->st == STREAM_STATUS_SUSPENDED) {
            BTMW_RPC_TEST_Logd("handle:%d SUSPENDED!! so cond wait!!\n", in->handle);
            pthread_cond_wait(&in->cond, &in->mutex);
        }

        if (in->st == STREAM_STATUS_IDLE) {
            BTMW_RPC_TEST_Logd("handle:%d STOPPED!!\n", in->handle);
            break;
        }

        in->stream->write(in->stream, pcm_buffer, total_pcm_len);
        //BTMW_RPC_TEST_Logd("write total_pcm_len %d!! --done\n", total_pcm_len);
    }
send_audio_data_end:
    BTMW_RPC_TEST_Logd("handle:%d(%d), send_audio_data_end\n", in->handle, in->st);
    if (NULL != hw_dev)
    {
        in->st = STREAM_STATUS_IDLE;
        hw_dev->close_output_stream(hw_dev, in->stream);
        BTMW_RPC_TEST_Logd("close_output_stream done!!\n");
    }

    (void)fclose(in->fInputPCM);
    in->fInputPCM = NULL;
    free(pcm_buffer);
    unalloc_stream_test_by_handle(in->handle);
    in->test_stream_handle = -1;

    if (!is_any_streaming() && NULL != gdevice)
    {
        BTMW_RPC_TEST_Logd("audio_hw_device_close!!\n");
        //gdevice->close(gdevice);
        audio_hw_device_close((struct audio_hw_device *)gdevice);
        gdevice = NULL;
    }

    BTMW_RPC_TEST_Logd("handle:%d(%d), Send audio finished\n", in->handle, in->st);
    pthread_mutex_unlock(&in->mutex);
    return NULL;
}


static int btmw_rpc_test_leaudio_src_write_stream_handler(int argc, char *argv[])
{
    INT32 result;
    struct stream_test_t* in = NULL;
    audio_io_handle_t handle;

    BTMW_RPC_TEST_Logd("argc=%d\n", argc);
    if (argc == 2)
    {
        handle = (audio_io_handle_t)atoi(argv[0]);
        in = get_stream_test_by_handle(handle);
        if (NULL == in) {
            BTMW_RPC_TEST_Loge("input wrong handle = %d.\n", handle);
            return -1;
        }
        else if (in->st == STREAM_STATUS_STARTED) {
            BTMW_RPC_TEST_Loge("input handle = %d is streaming, exist.\n", handle);
            return -1;
        }
        strncpy(in->test_pcm, argv[1], sizeof(in->test_pcm));
        in->test_pcm[127] = '\0';
    }
    else
    {
        BTMW_RPC_TEST_Logd("Useage: write_stream <handle> <file-path>\n");
        return -1;
    }
    pthread_mutex_lock(&in->mutex);
    BTMW_RPC_TEST_Logd("in.test_pcm:%s, in.handle=%d\n", in->test_pcm, in->handle);

    if(-1 == in->test_stream_handle)
    {
        result = pthread_create(&in->test_stream_handle, NULL, btmw_rpc_test_leaudio_write_audio_data_thread, (void*)in);
        if (result)
        {
            BTMW_RPC_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_RPC_TEST_Logw("streaming thread has been created, if suspended, resume the write thread by cond signal!\n");
        pthread_cond_signal(&in->cond);
        in->st = STREAM_STATUS_STARTED;
    }

    pthread_mutex_unlock(&in->mutex);
    BTMW_RPC_TEST_Logw("Exist!!!\n");
    return 0;
}

static int btmw_rpc_test_leaudio_src_suspend_stream_handler(int argc, char *argv[])
{
    audio_io_handle_t handle;
    struct stream_test_t* in = NULL;
    if (argc == 1) {
        handle = (audio_io_handle_t)atoi(argv[0]);
        in = get_stream_test_by_handle(handle);
        if (!in || !in->in_use || in->stream == NULL) {
            BTMW_RPC_TEST_Logw("this stream test not in use or not open success, just return!\n");
            return -1;
        }
    } else {
        BTMW_RPC_TEST_Logd("Useage: pause_stream <handle>\n");
        return -1;
    }

    pthread_mutex_lock(&in->mutex);
    BTMW_RPC_TEST_Logd("set suspended for handle:%d(%d)\n", in->handle, in->st);
    in->st = STREAM_STATUS_SUSPENDED;
    in->stream->common.standby(&(in->stream->common));

    pthread_mutex_unlock(&in->mutex);
    BTMW_RPC_TEST_Logd(" done\n");
    return 0;
}

static int btmw_rpc_test_leaudio_src_stop_stream_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logd("into\n");
    audio_io_handle_t handle;
    struct stream_test_t* in = NULL;
    if (argc == 1) {
        handle = (audio_io_handle_t)atoi(argv[0]);
        in = get_stream_test_by_handle(handle);
        if (!in || !in->in_use || in->stream == NULL) {
            BTMW_RPC_TEST_Logw("this stream test not in use or not open success, just return!\n");
            return -1;
        }

        pthread_mutex_lock(&in->mutex);
        BTMW_RPC_TEST_Logd("set idle for handle:%d(%d)\n", in->handle, in->st);

        if(in->st == STREAM_STATUS_SUSPENDED)
        {
            in->st = STREAM_STATUS_IDLE;
            BTMW_RPC_TEST_Logw("streaming thread has been created, if suspended, resume the write thread by cond signal!\n");
            pthread_cond_signal(&in->cond);
        }

        in->st = STREAM_STATUS_IDLE;
        BTMW_RPC_TEST_Logd("set idle for handle:%d(%d) done\n", in->handle, in->st);
        pthread_mutex_unlock(&in->mutex);
    } else {
        BTMW_RPC_TEST_Logd("Useage: stop_stream <handle>\n");
        return -1;
    }

    BTMW_RPC_TEST_Logd("done\n");
    //in->stream->common.standby(&(in->stream->common));
    return 0;
}


static BTMW_RPC_TEST_CLI btmw_rpc_test_leaudio_src_cli_commands[] =
{
    //Ex: write_stream 0 /data/bluedroid/lib/input0_48000.pcm;
    //Ex: write_stream 1 /data/bluedroid/lib/input1_48000.pcm;
    {"write_stream",    btmw_rpc_test_leaudio_src_write_stream_handler,   " = write_stream <handle> <file-path>"},
    {"suspend_stream",  btmw_rpc_test_leaudio_src_suspend_stream_handler, " = suspend_stream <handle>"},
    {"stop_stream",     btmw_rpc_test_leaudio_src_stop_stream_handler,    " = stop_stream <handle>"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_leaudio_src_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_rpc_test_leaudio_src_cli_commands;
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

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_LEAUDIO_SRC, btmw_rpc_test_leaudio_src_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

INT32 btmw_rpc_test_leaudiohw_src_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD leaudio_src_mod = {0};

    leaudio_src_mod.mod_id = BTMW_RPC_TEST_MOD_LEAUDIO_SRC;
    strncpy(leaudio_src_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_LEAUDIO_SRC, sizeof(leaudio_src_mod.cmd_key));
    leaudio_src_mod.cmd_handler = btmw_rpc_test_leaudio_src_cmd_handler;
    leaudio_src_mod.cmd_tbl = btmw_rpc_test_leaudio_src_cli_commands;

    ret = btmw_rpc_test_register_mod(&leaudio_src_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for leaudio returns: %d\n", ret);
    init_stream_test();
    return ret;
}


