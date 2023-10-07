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
#include <stdlib.h>
#include <pthread.h>
#include <asoundlib.h>
#include <dlfcn.h>
#include <sys/prctl.h>

#include "mtk_audio.h"

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_leaudio_bmr_if.h"
#include "mtk_bt_service_leaudio_bmr_wrapper.h"
#include "u_bt_mw_leaudio_bmr.h"

#define BMR_LOG_TAG     "[BMR_RPC]"

#define MAX_OUTPUT_FILE_NAME 128
#define BTMW_RPC_BMR_CASE_RETURN_STR(const) case const: return #const;
#define PCM_BUFFER_SIZE 512*8     // 512x8 = 4096

#define SOURCE_NUM_MAX  (100)
BT_BMR_SRC_INFO_T g_sources[SOURCE_NUM_MAX];
UINT8 g_source_num = 0;
static UINT8 g_scan_state = BT_BMR_AUTONOMOUSLY_SCAN_STOPPED;
static UINT8 g_solicitation_req_state = BT_BMR_SOLICITATION_REQUEST_STOPPED;
static UINT8 g_active_source_id = 0;

static const char leaudio_so_path[128] = "/usr/lib/libaudio.leaudio.default.so";
static void *g_leaudio_so_handle = NULL;
static struct hw_device_t* audio_hw_device = NULL;
static struct hw_module_t *hmi = NULL;
//static struct audio_stream_in *gstreamIn = NULL;

//static FILE *leaudioReadFile = NULL;
//static const char* leaudioReadFilename = "/data/misc/bluetooth/logs/leaudio_read.pcm";
//static pthread_t s_bmr_streamIn_thread = 0;
//static BOOL s_bmr_streamThread_running = FALSE;

typedef struct bmr_stream_test_t {
    BOOL in_use;
    audio_io_handle_t handle;// == socketidx from streaming event data
    CHAR leaudioReadFilename[128];
    pthread_t s_bmr_streamIn_thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    FILE *leaudioReadFile;//must diff from each other
    struct audio_stream_in *stream;
    BOOL thread_running;
    //struct audio_config *config;
    //STREAM_STATUS st;
} bmr_stream_test_t;

static struct bmr_stream_test_t bmr_stt[BIS_SUPPORT_NUM] = {{0}};

static int btmw_rpc_test_bmr_discover_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_solicitation_request_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_disconnect_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_refresh_source_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_remove_source_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_set_broadcast_code_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_streaming_start_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_streaming_stop_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_set_pac_config_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_close_handler(int argc, char **argv);
static int btmw_rpc_test_bmr_dump_handler(int argc, char **argv);

static VOID _btmw_rpc_test_bmr_dump_source_info(UINT8 action, BT_BMR_SRC_INFO_T *p_src)
{
    if (p_src == NULL) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid audio source params!!!\n");
        return;
    }
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"action = %d\n", action);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"source_id: %d\n", p_src->source_id);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  broadcast_id: 0x%08x\n", p_src->broadcast_id);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  encryption: %d\n", p_src->encryption);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  addr: %s\n", p_src->addr);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  addr_type: %d\n", p_src->addr_type);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  adv_sid: %d\n", p_src->adv_sid);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  state: %d\n", p_src->state);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  bis_sync_req_mask: 0x%x\n", p_src->bis_sync_req_mask);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  presentation_delay: %d\n", p_src->presentation_delay);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"  num_subgroups: %d\n", p_src->num_subgroups);
    for (size_t i = 0; i < p_src->num_subgroups; i++) {
        BT_BMR_SUBGROUP_T *p_cur_subgroup = &(p_src->subgroups[i]);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG" subgroup #%d:\n", p_cur_subgroup->subgroup_id);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   num_bis: %d\n", p_cur_subgroup->num_bis);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   coding_format: 0x%x\n", p_cur_subgroup->codec_id.coding_format);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   company_id: 0x%x\n", p_cur_subgroup->codec_id.company_id);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   vendor_codec_id: 0x%x\n", p_cur_subgroup->codec_id.vendor_codec_id);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   sampling_freq: 0x%x\n", p_cur_subgroup->codec_configs.sampling_freq);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   frame_duration: 0x%x\n", p_cur_subgroup->codec_configs.frame_duration);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   channel_allocation: 0x%x\n", p_cur_subgroup->codec_configs.channel_allocation);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   octets_per_codec_frame: 0x%x\n", p_cur_subgroup->codec_configs.octets_per_codec_frame);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   blocks_per_sdu: 0x%x\n", p_cur_subgroup->codec_configs.blocks_per_sdu);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   prefered_audio_contexts: 0x%x\n", p_cur_subgroup->metadata.prefered_audio_contexts);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   streaming_audio_contexts: 0x%x\n", p_cur_subgroup->metadata.streaming_audio_contexts);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   vendor_metadata_length: 0x%x\n", p_cur_subgroup->metadata.vendor_metadata_length);
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   memory of vendor_specific_metadata: %p\n", p_cur_subgroup->metadata.vendor_metadata);

        for (size_t j = 0; j < p_cur_subgroup->num_bis; j++) {
            BT_BMR_BIS_T bis = p_cur_subgroup->bis_info[j];
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"   BIS index #%d:\n", bis.bis_index);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     sampling_freq: 0x%x\n", bis.codec_configs.sampling_freq);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     frame_duration: 0x%x\n", bis.codec_configs.frame_duration);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     channel_allocation: 0x%x\n", bis.codec_configs.channel_allocation);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     octets_per_codec_frame: 0x%x\n", bis.codec_configs.octets_per_codec_frame);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     blocks_per_sdu: 0x%x\n", bis.codec_configs.blocks_per_sdu);
            BTMW_RPC_TEST_Logd(BMR_LOG_TAG"     b_playable: %d\n", bis.b_playable);
        }
    }
}

static BT_BMR_SRC_INFO_T* _btmw_bmr_find_source_by_id(UINT8 id)
{
    UINT8 num = 0;
    if (g_source_num > 0) {
        for (UINT8 i = 0; i < SOURCE_NUM_MAX; i++) {
            if (g_sources[i].source_id == id)
                return &g_sources[i];
            if (g_sources[i].source_id != 0) {
                num++;
                if (num >= g_source_num)
                    break;
            }
        }
    }
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no source with id %d is found\n", id);
    return NULL;
}

static BT_BMR_BIS_T* _btmw_bmr_find_bis_by_index(BT_BMR_SRC_INFO_T* p_src, UINT8 bis_index)
{
    if (p_src->num_subgroups > 0) {
        for (UINT8 i = 0; i < p_src->num_subgroups; i++) {
            BT_BMR_SUBGROUP_T *p_sg = &(p_src->subgroups[i]);
            for (UINT8 j = 0; j < p_sg->num_bis; j++) {
                if (p_sg->bis_info[j].bis_index == bis_index) {
                    return &(p_sg->bis_info[j]);
                }
            }
        }
    }
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no bis with index %d is found\n", bis_index);
    return NULL;
}

static UINT8 _btmw_bmr_get_bis_channel_num(BT_BMR_SRC_INFO_T* p_src, BT_BMR_BIS_T* p_bis)
{
    UINT8 ch_num = 0;
    BT_BMR_SUBGROUP_T *sg = &p_src->subgroups[p_bis->subgroup_id];
    UINT32 ch_allocation = (p_bis->codec_configs.channel_allocation != 0) ? \
        p_bis->codec_configs.channel_allocation : sg->codec_configs.channel_allocation;
    while (ch_allocation > 0) {
        if (ch_allocation & (0x00000001)) {
            ch_num++;
        }
        ch_allocation >>= 1;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"bis %d, channel num:%d\n", p_bis->bis_index, ch_num);
    return ch_num;
}

static void _btmw_bmr_streaming_check_start(BT_BMR_SRC_INFO_T *p_src) {
    UINT32 ui4_streaming_bis_mask = 0;
    UINT8 bis_count = 0;
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"state %d, bis_sync_req_mask: 0x%08x\n", \
        p_src->state, p_src->bis_sync_req_mask);
    if (BT_BMR_SRC_STATE_READY_FOR_STREAMING == p_src->state) {
        if (0xFFFFFFFF == p_src->bis_sync_req_mask) {
            //bis sync request is not preferred, start stream bis index 1 by default.
            ui4_streaming_bis_mask = 1;
        } else if (0 != p_src->bis_sync_req_mask) {
            //streaming case1: single stereo BIS; case2: dual mono BISes
            UINT32 ui4_tmp_mask = p_src->bis_sync_req_mask;
            UINT8 bis_index = 1;
            while (ui4_tmp_mask > 0) {
                if ((ui4_tmp_mask & (1 << (bis_index - 1))) != 0) {
                    BT_BMR_BIS_T* bis = _btmw_bmr_find_bis_by_index(p_src, bis_index);
                    UINT8 ch_num = _btmw_bmr_get_bis_channel_num(p_src, bis);
                    if (ch_num > 2) {
                        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"skip unsupported BIS %d, ch_num:%d\n", bis->bis_index, ch_num);
                        ui4_tmp_mask >>= 1;
                        bis_index++;
                        continue;
                    } else if (ch_num == 2) {
                        if (0 == bis_count) {
                            ui4_streaming_bis_mask |= (1 << (bis_index - 1));
                            BTMW_RPC_TEST_Logi(BMR_LOG_TAG"stereo bis %d is selected to stream\n", bis->bis_index);
                            break;
                        } else {
                            BTMW_RPC_TEST_Logi(BMR_LOG_TAG"skip stereo bis %d as mono BIS is selected previously\n", bis->bis_index);
                            ui4_tmp_mask >>= 1;
                            bis_index++;
                            continue;
                        }
                    } else {
                        ui4_streaming_bis_mask |= (1 << (bis_index - 1));
                        bis_count++;
                        if (bis_count == 2) {
                            BTMW_RPC_TEST_Logi(BMR_LOG_TAG"2 mono bis are selected to stream\n");
                            break;
                        }
                    }
                }
                ui4_tmp_mask >>= 1;
                bis_index++;
            }
        }
        if (0 != ui4_streaming_bis_mask) {
            a_mtkapi_bt_bmr_streaming_start(p_src->source_id, ui4_streaming_bis_mask);
            //clear source bis_sync_req_mask which is oneshot valid
            p_src->bis_sync_req_mask = 0;
        }
    }
}

static void _btmw_bmr_free_source(BT_BMR_SRC_INFO_T *p_src) {
    if (NULL == p_src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid p_src\n");
        return;
    }
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"source_id: %d\n", p_src->source_id);
    if (p_src->num_subgroups > 0) {
        for (UINT8 i = 0; i < p_src->num_subgroups; i++) {
            BT_BMR_SUBGROUP_T *p_sg = &(p_src->subgroups[i]);
            if (NULL != p_sg->metadata.vendor_metadata) {
                free(p_sg->metadata.vendor_metadata);
                p_sg->metadata.vendor_metadata = NULL;
            }
        }
        free(p_src->subgroups);
        p_src->subgroups = NULL;
    }
    memset(p_src, 0, sizeof(BT_BMR_SRC_INFO_T));
}
static void _btmw_bmr_add_source(BT_BMR_SRC_INFO_T *p_src) {
    if (NULL == p_src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid p_src\n");
        return;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source_id: %d\n", p_src->source_id);
    if (p_src->source_id > SOURCE_NUM_MAX) {
        BTMW_RPC_TEST_Loge("out of resource\n");
        return;
    }
    BT_BMR_SRC_INFO_T *src = &g_sources[p_src->source_id - 1];
    memcpy(src, p_src, sizeof(BT_BMR_SRC_INFO_T));
    if (p_src->num_subgroups > 0) {
        if (NULL == p_src->subgroups) {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no subgroup info\n");
            return;
        }
        src->subgroups = (BT_BMR_SUBGROUP_T *)malloc(p_src->num_subgroups * sizeof(BT_BMR_SUBGROUP_T));
        if (NULL == src->subgroups) {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"malloc subgroup fail\n");
            return;
        }
        for (UINT8 i = 0; i < p_src->num_subgroups; i++) {
            memcpy(&src->subgroups[i], &p_src->subgroups[i], sizeof(BT_BMR_SUBGROUP_T));
            if (0 < src->subgroups[i].metadata.vendor_metadata_length) {
                src->subgroups[i].metadata.vendor_metadata = (UINT8 *)malloc(src->subgroups[i].metadata.vendor_metadata_length);
                if (NULL == src->subgroups[i].metadata.vendor_metadata) {
                    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"malloc vendor_metadata fail\n");
                    _btmw_bmr_free_source(src);
                    return;
                }
                memcpy(&src->subgroups[i].metadata.vendor_metadata,
                    &p_src->subgroups[i].metadata.vendor_metadata,
                    src->subgroups[i].metadata.vendor_metadata_length);
            }
        }
    }
    g_source_num++;
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source_id: %d, total source number: %d\n", p_src->source_id, g_source_num);
    _btmw_rpc_test_bmr_dump_source_info(BT_BMR_SRC_ADD, src);
}
static void _btmw_bmr_update_source(BT_BMR_SRC_INFO_T *p_src) {
    if (NULL == p_src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid p_src\n");
        return;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source_id: %d\n", p_src->source_id);
    BT_BMR_SRC_INFO_T *src = _btmw_bmr_find_source_by_id(p_src->source_id);
    if (NULL == src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no source %d\n", p_src->source_id);
        return;
    }
    _btmw_bmr_free_source(src);
    src = &g_sources[p_src->source_id - 1];
    memcpy(src, p_src, sizeof(BT_BMR_SRC_INFO_T));
    if (p_src->num_subgroups > 0) {
        if (NULL == p_src->subgroups) {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no subgroup info\n");
            return;
        }
        src->subgroups = (BT_BMR_SUBGROUP_T *)malloc(p_src->num_subgroups * sizeof(BT_BMR_SUBGROUP_T));
        if (NULL == src->subgroups) {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"malloc subgroup fail\n");
            return;
        }
        for (UINT8 i = 0; i < p_src->num_subgroups; i++) {
            memcpy(&src->subgroups[i], &p_src->subgroups[i], sizeof(BT_BMR_SUBGROUP_T));
            if (0 < src->subgroups[i].metadata.vendor_metadata_length) {
                src->subgroups[i].metadata.vendor_metadata = (UINT8 *)malloc(src->subgroups[i].metadata.vendor_metadata_length);
                if (NULL == src->subgroups[i].metadata.vendor_metadata) {
                    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"malloc vendor_metadata fail\n");
                    _btmw_bmr_free_source(src);
                    return;
                }
                memcpy(&src->subgroups[i].metadata.vendor_metadata,
                    &p_src->subgroups[i].metadata.vendor_metadata,
                    src->subgroups[i].metadata.vendor_metadata_length);
            }
        }
    }
    _btmw_rpc_test_bmr_dump_source_info(BT_BMR_SRC_UPDATE, src);
    //todo check streaming request
    _btmw_bmr_streaming_check_start(src);
}
static void _btmw_bmr_remove_source(BT_BMR_SRC_INFO_T *p_src) {
    if (NULL == p_src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid p_src\n");
        return;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source_id: %d\n", p_src->source_id);
    BT_BMR_SRC_INFO_T *src = _btmw_bmr_find_source_by_id(p_src->source_id);
    if (src) {
        _btmw_bmr_free_source(src);
        g_source_num--;
        BTMW_RPC_TEST_Logi("source id: %d, total source num: %d\n", p_src->source_id, g_source_num);
    }
}
static void _btmw_bmr_source_info(UINT8 action, BT_BMR_SRC_INFO_T *p_src) {
    if (NULL == p_src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid source info\n");
        return;
    }
    switch (action) {
        case BT_BMR_SRC_ADD: {
            _btmw_bmr_add_source(p_src);
            break;
        }
        case BT_BMR_SRC_UPDATE: {
            _btmw_bmr_update_source(p_src);
            break;
        }
        case BT_BMR_SRC_REMOVE: {
            _btmw_bmr_remove_source(p_src);
            break;
        }
        default: {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"unknown source info action: %d\n", action);
            return;
        }     
    }
}
static void _btmw_bmr_source_state_changed(BT_BMR_EVENT_SRC_STATE_CHANGE_DATA_T *new_state) {
    UINT32 ui4_streaming_bis_mask = 0;
    UINT8 bis_count = 0;
    if (NULL == new_state) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid new_state info\n");
        return;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source_id: %d\n", new_state->source_id);
    BT_BMR_SRC_INFO_T *src = _btmw_bmr_find_source_by_id(new_state->source_id);
    if (NULL == src) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"no source with id %d is found\n", new_state->source_id);
        return;
    }
    if (src->state == new_state->state) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"ignore same state\n");
        return;
    }
    BTMW_RPC_TEST_Loge(BMR_LOG_TAG"state changed from %d --> %d, bis_sync_req_mask: 0x%08x\n", \
        src->state, new_state->state, src->bis_sync_req_mask);
    src->state = new_state->state;
    //todo check streaming request
    _btmw_bmr_streaming_check_start(src);
}

static BOOL _btmw_bmr_load_leaudio(const char path[])
{
    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    g_leaudio_so_handle = dlopen(path, RTLD_LAZY);
    if (g_leaudio_so_handle == NULL) {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"load: module=%s\n", path, err_str?err_str:"unknown");
        return FALSE;
    }

    /* Get the address of the struct hal_module_info. */
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(g_leaudio_so_handle, sym);
    if (hmi == NULL) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"load: couldn't find symbol %s\n", sym);
        dlclose(g_leaudio_so_handle);
        g_leaudio_so_handle = NULL;
        return FALSE;
    }

    /* Check that the id matches */
    if (strcmp(AUDIO_HARDWARE_MODULE_ID, hmi->id) != 0) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"load: id=%s != hmi->id=%s\n", AUDIO_HARDWARE_MODULE_ID, hmi->id);
        dlclose(g_leaudio_so_handle);
        g_leaudio_so_handle = NULL;
        hmi = NULL;
        return FALSE;
    }
    hmi->dso = g_leaudio_so_handle;
    return TRUE;
}

static void* btmw_rpc_test_bmr_data_read_handler(void *arg) {
    UINT8 pcm_buffer[PCM_BUFFER_SIZE];
    INT32 pcm_bytes = PCM_BUFFER_SIZE;
    INT32 pcm_read = 0;

    struct bmr_stream_test_t* bsst  = (struct bmr_stream_test_t*)arg;
    struct audio_hw_device *hw_dev = NULL;
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);
    if (audio_hw_device == NULL)
    {
        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"audio_hw_device is NULL\n");
        return NULL;
    }
    bsst->thread_running = TRUE;
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"ch:%d\n",bsst->stream->common.get_channels(bsst->stream));
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"fs:%d\n",bsst->stream->common.get_sample_rate(bsst->stream));
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"format:%d\n",bsst->stream->common.get_format(bsst->stream));
    while (bsst->thread_running) {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE*sizeof(UINT8));
        pcm_read = bsst->stream->read(bsst->stream, pcm_buffer, pcm_bytes);
        if (pcm_read <= 0) {
            BTMW_RPC_TEST_Logw(BMR_LOG_TAG"socket read error= %d\n", pcm_read);
        }
        if (bsst->leaudioReadFile && pcm_read > 0)
        {
            fwrite(pcm_buffer, 1, (size_t)pcm_read, bsst->leaudioReadFile );
        }
    }
    if(hw_dev != NULL)
    {
        hw_dev->close_input_stream(hw_dev, bsst->stream);
        bsst->stream = NULL;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"exit\n");
    return NULL;
}

static BOOL btmw_rpc_test_bmr_sink_open_input_stream(bmr_stream_test_t* bsst, struct audio_hw_device *hw_dev) {
    int ret;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"Enter\n");

    ret = hw_dev->open_input_stream(hw_dev, bsst->handle, 0, NULL,
                   &bsst->stream, AUDIO_INPUT_FLAG_NONE, NULL, AUDIO_SOURCE_DEFAULT);
    if (ret < 0) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"Open input stream fail (err = %d)!\n", ret);
        return FALSE;
    }
    if (bsst->s_bmr_streamIn_thread == -1)
    {
        if ( pthread_create(&bsst->s_bmr_streamIn_thread, NULL, btmw_rpc_test_bmr_data_read_handler, (void*)bsst) != 0 )
        {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"pthread_create fail: %s\n", strerror(errno));
            hw_dev->close_input_stream(hw_dev, bsst->stream);
            return FALSE;
        }
        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"pthread_create success: 0x%x\n", bsst->s_bmr_streamIn_thread);
    }
    return TRUE;
}

static void _btmw_bmr_streaming_event(BT_BMR_EVENT_STREAMING_EVENT_DATA_T *evt) {
    if (NULL == evt) {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"invalid event info\n");
        return;
    }
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"event: %d\n", evt->event);
    switch (evt->event) {
        case BT_BMR_STERAMING_START_SUCCESS: {
            if (g_leaudio_so_handle == NULL) {
                assert(_btmw_bmr_load_leaudio(leaudio_so_path));
            }
            if (0 > audio_hw_device_open(hmi, (audio_hw_device_t **)&audio_hw_device)) {
                BTMW_RPC_TEST_Loge(BMR_LOG_TAG"open audio leaudio hw device fail\n");
                return;
            }
            if (((audio_hw_device_t *)audio_hw_device)->common.version < AUDIO_DEVICE_API_VERSION_MIN) {
                BTMW_RPC_TEST_Loge(BMR_LOG_TAG"wrong audio hw device version %04x\n", ((audio_hw_device_t *)audio_hw_device)->common.version);
                audio_hw_device_close(audio_hw_device);
                audio_hw_device = NULL;
                return;
            }
            for (uint8_t i = 0; i < evt->data.bis_num; i++) {
                if (evt->data.simap[i].in_use) {
                    bmr_stt[i].handle = (audio_io_handle_t)evt->data.simap[i].socket_idx;
                    //bmr_stt[i].leaudioReadFile = fopen(bmr_stt[i].leaudioReadFilename, "wb+");
                    if (bmr_stt[i].leaudioReadFile == NULL) {
                        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"Can't open outputFilename!\n");
                    } else {
                        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"pcm dump at %s\n", bmr_stt[i].leaudioReadFilename);
                    }
                    if (FALSE == btmw_rpc_test_bmr_sink_open_input_stream(&bmr_stt[i], (audio_hw_device_t *)audio_hw_device)) {
                        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"open input stream fail\n");
                        return;
                    }
                    bmr_stt[i].in_use = TRUE;
                }
            }
            break;
        }
        case BT_BMR_STERAMING_START_FAIL: {
            break;
        }
        case BT_BMR_STERAMING_STOP_SUCCESS: {
            for (uint8_t i=0; i<BIS_SUPPORT_NUM; i++) {
                BTMW_RPC_TEST_Logi(BMR_LOG_TAG"bmr_stt[%d].in_use: %d\n", i, bmr_stt[i].in_use);
                if (bmr_stt[i].in_use) {
                    bmr_stt[i].thread_running = FALSE;
                    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"streamIn_thread: %d join start\n", i);
                    pthread_join(bmr_stt[i].s_bmr_streamIn_thread, NULL);
                    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"streamIn_thread: %d join end\n", i);
                    bmr_stt[i].s_bmr_streamIn_thread = -1;
                    bmr_stt[i].in_use = FALSE;
                    if (bmr_stt[i].leaudioReadFile) {
                        //fclose(bmr_stt[i].leaudioReadFile);
                        bmr_stt[i].leaudioReadFile = NULL;
                    }
                    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"stop success for stream %d\n", i);
                }
            }
            if (NULL != audio_hw_device) {
                BTMW_RPC_TEST_Logi(BMR_LOG_TAG"close audio hw device\n");
                audio_hw_device_close(audio_hw_device);
                audio_hw_device = NULL;
            } else {
                BTMW_RPC_TEST_Logw(BMR_LOG_TAG"audio hw device is not opened\n");
            }
            break;
        }
        case BT_BMR_STERAMING_STOP_FAIL: {
            break;
        }
        default: {
            BTMW_RPC_TEST_Loge("unknown streaming event: %d\n", evt->event);
            return;
        }
    }
}

static void init_bmr_stream_test (void) {
    const CHAR* filebase = "/data/misc/bluetooth/logs/leaudio_read.pcm";
    for (uint8_t i=0; i<BIS_SUPPORT_NUM; i++) {
        bmr_stt[i].in_use = FALSE;
        bmr_stt[i].handle = 0xFF;
        bmr_stt[i].leaudioReadFile = NULL;
        bmr_stt[i].stream = NULL;
        bmr_stt[i].s_bmr_streamIn_thread = -1;
        bmr_stt[i].thread_running = FALSE;
        pthread_mutex_init(&bmr_stt[i].mutex, NULL);
        pthread_cond_init(&bmr_stt[i].cond, NULL);
        snprintf(bmr_stt[i].leaudioReadFilename, 128, "%s-%d", filebase, i);
    }
}


/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */

/*static void btmw_rpc_test_bmr_close_input_stream(uint8_t handle) {

    BTMW_RPC_TEST_Logi("%s()\n", __func__);
    s_bmr_streamThread_running = FALSE;
    if (s_bmr_streamIn_thread) {
        BTMW_RPC_TEST_Loge("set g_rpc_streamThread_running NULL)!\n");
    }
    #if 0
    BTMW_RPC_TEST_Logi("%s()check 0\n", __func__);
    if(gp_rpc_stream_in != NULL)
    {
        gp_rpc_audio_dev->close_input_stream(gp_rpc_audio_dev, gp_rpc_stream_in);
        gp_rpc_stream_in = NULL;
    }
    BTMW_RPC_TEST_Logi("%s()check\n", __func__);
    #endif
    return;
}*/


static BTMW_RPC_TEST_CLI btmw_rpc_test_bmr_cli_commands[] =
{
    {(const char *)"discover",          btmw_rpc_test_bmr_discover_handler,         (const char *)" = discover <op_code> <scan_duration> [bl1] [bl2] [...] [bln]"},
    {(const char *)"solicitation_req",  btmw_rpc_test_bmr_solicitation_request_handler,  (const char *)" = solicitation_req <op_code> <adv_duration> <user_data>"},
    {(const char *)"disconnect",        btmw_rpc_test_bmr_disconnect_handler,       (const char *)" = disconnect <addr>"},
    {(const char *)"refresh_source",    btmw_rpc_test_bmr_refresh_source_handler,   (const char *)" = refresh_source <source_id>"},
    {(const char *)"remove_source",     btmw_rpc_test_bmr_remove_source_handler,    (const char *)" = remove_source <all> <source_id>"},
    {(const char *)"set_bc",            btmw_rpc_test_bmr_set_broadcast_code_handler,  (const char *)" = set_bc <source_id> <broadcast_code>"},
    {(const char *)"streaming_start",   btmw_rpc_test_bmr_streaming_start_handler,  (const char *)" = streaming_start <source_id> <BISes bit mask>"},
    {(const char *)"streaming_stop",    btmw_rpc_test_bmr_streaming_stop_handler,   (const char *)" = streaming_stop <source_id> <BISes bit mask>"},
    {(const char *)"set_pac",           btmw_rpc_test_bmr_set_pac_config_handler,   (const char *)" = set_pac <pac_type> <value>"},
    {(const char *)"close",             btmw_rpc_test_bmr_close_handler,            (const char *)" = close <open_to_bsa>"},
    {(const char *)"dump",              btmw_rpc_test_bmr_dump_handler,             (const char *)" = dump <all> <source_id>"},
    {NULL, NULL, NULL},
};

static int btmw_rpc_test_bmr_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd = NULL;
    BTMW_RPC_TEST_CLI *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"argc: %d, argv[0]: %s\n", argc, argv[0]);
    cmd = btmw_rpc_test_bmr_cli_commands;
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"Unknown command\n");
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BMR, btmw_rpc_test_bmr_cli_commands);
        return -1;
    }

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
        BTMW_RPC_TEST_Logi(BMR_LOG_TAG"Unknown command '%s'\n", argv[0]);
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BMR, btmw_rpc_test_bmr_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static VOID btmw_rpc_test_bass_show_conn_state(BT_BMR_EVENT_CONN_STATE_DATA_T *conn_state)
{
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"bt_addr=%s\n", conn_state->bt_addr);
    switch (conn_state->state)
    {
    case BT_BMR_CONN_STATE_DISCONNECTED:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"BMR DISCONNECTED");
        break;

    case BT_BMR_CONN_STATE_CONNECTING:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"BMR CONNECTING");
        break;

    case BT_BMR_CONN_STATE_CONNECTED:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"BMR CONNECTED");
        break;

    case BT_BMR_CONN_STATE_DISCONNECTING:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"BMR DISCONNECTING");
        break;

    default:
        break;
    }
}

static CHAR* btmw_rpc_test_bmr_app_event(BT_BMR_EVENT_E event)
{
    switch((int)event)
    {
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_CONN_STATE)
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_SOLICITATION_REQUEST_STATE)
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_SCAN_STATE)
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_SRC_INFO)
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_SRC_STATE_CHANGE)
        BTMW_RPC_BMR_CASE_RETURN_STR(BT_BMR_EVENT_STREAMING_EVENT)
        default: return "UNKNOWN_BMR_EVENT";
   }
}

static VOID btmw_rpc_test_bmr_app_cbk(BT_BMR_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"event=%d (%s)\n", param->event, btmw_rpc_test_bmr_app_event(param->event));
    switch (param->event)
    {
    case BT_BMR_EVENT_CONN_STATE:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"state: %d, addr: %s\n",
            param->data.conn_state.state, param->data.conn_state.bt_addr);
        break;
    case BT_BMR_EVENT_SOLICITATION_REQUEST_STATE:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"state: %d\n", param->data.solicitation_req_state);
        g_solicitation_req_state = param->data.solicitation_req_state;
        break;
    case BT_BMR_EVENT_SCAN_STATE:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"state: %d, addr: %s\n", param->data.scan_state.state, param->data.scan_state.bt_addr);
        g_scan_state = param->data.scan_state.state;
        break;
    case BT_BMR_EVENT_SRC_INFO:
        _btmw_bmr_source_info(param->data.source_info.action, &param->data.source_info.info);
        break;
    case BT_BMR_EVENT_SRC_STATE_CHANGE:
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"source_id: %d, state: %d, reason: %d\n",
            param->data.source_state.source_id,
            param->data.source_state.state,
            param->data.source_state.err_code);
        _btmw_bmr_source_state_changed(&param->data.source_state);
        break;
    case BT_BMR_EVENT_STREAMING_EVENT: {
        BTMW_RPC_TEST_Logd(BMR_LOG_TAG"source_id: %d, bis_num: %d, event: %d, reason: %d\n",
            param->data.streaming_event.data.source_id,
            param->data.streaming_event.data.bis_num,
            param->data.streaming_event.event,
            param->data.streaming_event.err_code);
        _btmw_bmr_streaming_event(&param->data.streaming_event);
        break;
    }
    default:
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"unknown event=%d\n", param->event);
        break;
    }
}

static int btmw_rpc_test_bmr_discover_handler(int argc, char **argv)
{
    UINT8  bl_num = 0;
    UINT8 user_data[255] = {0};
    BT_BMR_DISCOVERY_PARAMS params;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    memset(&params, 0, sizeof(BT_BMR_DISCOVERY_PARAMS));

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: discover <op_code: (0/1)> <scan_duration in s> [bl1] [bl2] [...] [bln]\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &params.op_code);
    Util_Transform_Str2u32Num(argv[1], &params.scan_duration);

    if (argc > 2) {
        for (UINT8 i = 2; i < argc && i < BMR_MAX_BLACKLIST_SIZE + 2; i++) {
            Util_Transform_Str2u32Num(argv[i], &params.black_list[bl_num++]);
        }
    }

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"discover, auto_scan:%d, scan_duration:%d, bl:[0x%x][0x%x][0x%x], bl_num: %d\n",
        params.op_code, params.scan_duration, params.black_list[0], params.black_list[1], params.black_list[2], bl_num);
    return a_mtkapi_bt_bmr_discover(&params);
}

static int btmw_rpc_test_bmr_solicitation_request_handler(int argc, char **argv)
{
    UINT8  bl_num = 0;
    UINT8 user_data[255] = {0};
    BT_BMR_SOLICITATION_REQUEST_PARAMS params;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    memset(&params, 0, sizeof(BT_BMR_SOLICITATION_REQUEST_PARAMS));

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: solicitation_req <op_code: (0/1)> <adv_duration in s> [user_data: ad type data]\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &params.op_code);
    Util_Transform_Str2u32Num(argv[1], &params.adv_duration);

    if (argc > 2) {
        params.user_data_len = strlen(argv[2])/2;
        if (params.user_data_len > 254) {
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"too much user data, shall be less than 254 bytes!!!\n");
            BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: solicitation_req <op_code: (0/1)> <adv_duration in s> [user_data: ad type data]\n");
            return 0;
        }
        Util_Transform_Str2u8HexNumArray(argv[2], user_data);
        params.user_data = user_data;
    }

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"solicitation_req, op_code:%d, adv_duration:%d, user_data_len:%d\n",   \
        params.op_code, params.adv_duration, params.user_data_len);
    return a_mtkapi_bt_bmr_solicitation_request(&params);
}

static int btmw_rpc_test_bmr_disconnect_handler(int argc, char **argv)
{
    CHAR bt_addr[MAX_BDADDR_LEN] = {0};
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE : disconnect <addr>\n");
        return -1;
    }

    snprintf(bt_addr, MAX_BDADDR_LEN, "%s", argv[0]);
    return a_mtkapi_bt_bmr_disconnect(bt_addr);
}

static int btmw_rpc_test_bmr_refresh_source_handler(int argc, char **argv)
{
    UINT8 source_id = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: refresh_source <source_id>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &source_id);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"refresh_source, id: %d\n", source_id);
    return a_mtkapi_bt_bmr_refresh_source(source_id);
}

static int btmw_rpc_test_bmr_remove_source_handler(int argc, char **argv)
{
    BOOL all = 0;
    UINT8 source_id = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: remove_source <all> <source_id>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &all);
    Util_Transform_Str2u8Num(argv[1], &source_id);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"remove_source, all: %d, source_id: %d\n", all, source_id);
    return a_mtkapi_bt_bmr_remove_source(all, source_id);
}

static int btmw_rpc_test_bmr_set_broadcast_code_handler(int argc, char **argv)
{
    BT_BMR_SET_BROADCAST_CODE_PARAMS params;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    memset(&params, 0, sizeof(BT_BMR_SET_BROADCAST_CODE_PARAMS));

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: set_bc <source_id> <broadcast_code>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &params.source_id);
    Util_Transform_Str2u8HexNumArray(argv[1], params.broadcast_code);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"set_broadcast_code, source_id:%d, broadcast_code: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        params.source_id, params.broadcast_code[0], params.broadcast_code[1], params.broadcast_code[2], params.broadcast_code[3], \
        params.broadcast_code[4], params.broadcast_code[5], params.broadcast_code[6], params.broadcast_code[7], \
        params.broadcast_code[8], params.broadcast_code[9], params.broadcast_code[10], params.broadcast_code[11], \
        params.broadcast_code[12], params.broadcast_code[13], params.broadcast_code[14], params.broadcast_code[15]);
    return a_mtkapi_bt_bmr_set_broadcast_code(&params);
}

static int btmw_rpc_test_bmr_streaming_start_handler(int argc, char **argv)
{
    UINT8 source_id = 0;
    UINT32 bis_mask = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: streaming_start <source_id> <BISes bit mask>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &source_id);
    Util_Transform_Str2u32Num(argv[1], &bis_mask);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"streaming_start, id: %d, bis:0x%08x\n", source_id, bis_mask);
    return a_mtkapi_bt_bmr_streaming_start(source_id, bis_mask);
}

static int btmw_rpc_test_bmr_streaming_stop_handler(int argc, char **argv)
{
    UINT8 source_id = 0;
    UINT32 bis_mask = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: streaming_stop <source_id> <BISes bit mask>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &source_id);
    Util_Transform_Str2u32Num(argv[1], &bis_mask);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"streaming_stop, id: %d, bis:0x%08x\n", source_id, bis_mask);
    return a_mtkapi_bt_bmr_streaming_stop(source_id, bis_mask);
}

static int btmw_rpc_test_bmr_set_pac_config_handler(int argc, char **argv)
{
    UINT8 pac_type = 0;
    UINT32 value = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: set_pac <pac_type> <value>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &pac_type);
    Util_Transform_Str2u32Num(argv[1], &value);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"set_pac, pac_type:%d, value:0x%08x\n", pac_type, value);
    return a_mtkapi_bt_bmr_set_pac_config(pac_type, value);
}

static int btmw_rpc_test_bmr_close_handler(int argc, char **argv)
{
    BOOL open_to_bsa = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: close <open_to_bsa>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &open_to_bsa);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"close, open_to_bsa: %d\n", open_to_bsa);
    return a_mtkapi_bt_bmr_close(open_to_bsa);
}

static int btmw_rpc_test_bmr_dump_handler(int argc, char **argv)
{
    BOOL all = 0;
    UINT8 source_id = 0;
    BOOL full_info = 0;
    UINT8 temp_num = 0;

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);
    
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"g_source_num: %d\n", g_source_num);

    if (g_source_num > 0) {
        for (UINT8 i = 0; i < SOURCE_NUM_MAX; i++) {
             if (g_sources[i].source_id != 0) {
                temp_num++;
                BTMW_RPC_TEST_Logi(BMR_LOG_TAG"source id: %d, state: %d, brcst_id: 0x%x, encryption: %d, addr: %s\n", \
                    g_sources[i].source_id, g_sources[i].state, g_sources[i].broadcast_id, g_sources[i].encryption, g_sources[i].addr);
                if (temp_num >= g_source_num)
                    break;
             }
        }
    }

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge(BMR_LOG_TAG"USAGE: dump <all> <source_id> <full_info>\n");
        return 0;
    }

    Util_Transform_Str2u8Num(argv[0], &all);
    Util_Transform_Str2u8Num(argv[1], &source_id);
    Util_Transform_Str2u8Num(argv[2], &full_info);

    BTMW_RPC_TEST_Logi(BMR_LOG_TAG"dump, all: %d, source_id: %d, full_info: %d\n", all, source_id, full_info);
    return a_mtkapi_bt_bmr_dump(all, source_id, full_info);
}

int btmw_rpc_test_bmr_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD bmr_mod = {0};

    bmr_mod.mod_id = BTMW_RPC_TEST_MOD_LEAUDIO_BMR;
    snprintf(bmr_mod.cmd_key, sizeof(bmr_mod.cmd_key), "%s", BTMW_RPC_TEST_CMD_KEY_BMR);
    bmr_mod.cmd_handler = btmw_rpc_test_bmr_cmd_handler;
    bmr_mod.cmd_tbl = btmw_rpc_test_bmr_cli_commands;

    ret = btmw_rpc_test_register_mod(&bmr_mod);
    BTMW_RPC_TEST_Logd(BMR_LOG_TAG"btmw_rpc_test_register_mod() returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        init_bmr_stream_test();
        a_mtkapi_bt_bmr_register_callback(btmw_rpc_test_bmr_app_cbk, NULL);
    }

    for (UINT8 i = 0; i < SOURCE_NUM_MAX; i++) {
        memset(&g_sources[i], 0, sizeof(BT_BMR_SRC_INFO_T));
    }
    g_scan_state = BT_BMR_AUTONOMOUSLY_SCAN_STOPPED;
    g_solicitation_req_state = BT_BMR_SOLICITATION_REQUEST_STOPPED;
    return ret;
}

int btmw_rpc_test_bmr_deinit()
{
    BTMW_RPC_TEST_Logi(BMR_LOG_TAG);
    return 0;
}
