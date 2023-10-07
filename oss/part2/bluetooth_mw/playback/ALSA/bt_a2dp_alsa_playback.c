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

/* FILE NAME:  bt_a2dp_alsa_playback.c
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
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/prctl.h>

#include "u_bt_mw_common.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_a2dp_snk.h"
#include "osi/include/log.h"
#include "bt_mw_log.h"
#include "bt_mw_common.h"
#include "alsa/asoundlib.h"
//#include <asoundlib.h>



/* NAMING CONSTANT DECLARATIONS
 */
#define BT_ALSA_PLAYBACK_DETACH_IN_THREAD /* do dsp in one thread */

typedef enum
{
    BT_A2DP_ALSA_PB_STATUS_UNINIT = 0,
    BT_A2DP_ALSA_PB_STATUS_OPENED,
    BT_A2DP_ALSA_PB_STATUS_PLAYED,
    BT_A2DP_ALSA_PB_STATUS_PAUSED,
    BT_A2DP_ALSA_PB_STATUS_STOPED,
    BT_A2DP_ALSA_PB_STATUS_MAX
}BT_A2DP_ALSA_PB_STATUS;

/* MACRO FUNCTION DECLARATIONS
 */

#define ALSA_DBG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_DUMP(to_file, s, ...) BT_DBG_DUMP_2_FILE(to_file, BT_DEBUG_PB, s, ## __VA_ARGS__)


#define BT_A2DP_ALSA_PB_REPORT_EVENT(event) do{          \
        if(g_bt_a2dp_alsa_pb_event_cb)                   \
        {                                                \
            ALSA_DBG_NORMAL("report event:%d", event);   \
            g_bt_a2dp_alsa_pb_event_cb(event);           \
        }                                                \
    }while(0)

#define BT_A2DP_ALSA_PB_CASE_RETURN_STR(_const,str) case _const: return str

#define BT_A2DP_ALSA_PB_DIFF_TM_MS(a, b) \
    (((a).tv_sec-(b).tv_sec) * 1000L+((a).tv_nsec-(b).tv_nsec) /1000000)

#define BT_A2DP_ALSA_PB_DIFF_TM_US(a, b) \
    (((a).tv_sec-(b).tv_sec) * 1000000L+((a).tv_nsec-(b).tv_nsec) /1000)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    BOOL fgPlayBackInit;    /* if playback is inited */
    BOOL fgAudioReset;      /* if reset audio        */
}BT_PLAYBACK_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */

static BT_A2DP_PLAYER_EVENT_CB g_bt_a2dp_alsa_pb_event_cb = NULL;
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */

#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

//#define BT_ALSA_PLAYBACK_TEST


//#define BT_ALSA_PLAYBACK_MEM_CHECK

#define BT_ALSA_PLAYBACK_STAT_SPEED

#define BT_ALSA_ENABLE_FADE

#ifdef BT_ALSA_ENABLE_FADE
#define BT_ALSA_ENABLE_FADE_IN
//#define BT_ALSA_ENABLE_FADE_OUT //it's not ready
#endif

#define BT_ALSA_PLAYBACK_BUF_SIZE (4096)

/* if there is not enough data write to alsa, write dummy data */
//#define BT_ALSA_PLAYBACK_WRITE_DUMMY

/*
 * when to buffer audio data: once playback audio_status is BT_A2DP_ALSA_PB_STATUS_PLAYED
 * and audio data is sent to the alsa playback
 */
// #define BT_ALSA_PLAYBACK_ADD_BUFFER
#define BT_ALSA_PLAYBACK_BUF_CNT  (100)  /* it same as btif_media queue size */

#define BT_ALSA_PLAYBACK_CMD_Q_SIZE (10)
#define BT_ALSA_PLAYBACK_DATA_Q_SIZE BT_ALSA_PLAYBACK_BUF_CNT

#define BT_ALSA_PLAYBACK_CMD_OPEN       (0)
#define BT_ALSA_PLAYBACK_CMD_CLOSE      (1)
#define BT_ALSA_PLAYBACK_CMD_PLAY       (2)
#define BT_ALSA_PLAYBACK_CMD_PAUSE      (3)
#define BT_ALSA_PLAYBACK_CMD_DATA       (4)
#define BT_ALSA_PLAYBACK_CMD_QUIT       (5)
#define BT_ALSA_PLAYBACK_CMD_DUMP       (6)

#define BT_ALSA_PLAYBACK_WAIT_MS        (50)

typedef struct
{
    int type;       /* item type */
    union
    {
        struct
        {
            int fs; /* save freq sample/dump flag/... */
            int channels;
            int bitdepth;
        }param;
    }u;
}BT_ALSA_PLAYBACK_QUEUE_ITEM;


typedef struct
{
    unsigned int w_pos; /* write position */
    unsigned int r_pos; /* read position */
    unsigned int w_cnt; /* write counter */
    unsigned int r_cnt; /* read counter */

    unsigned int capacity; /* queue size */
    BT_ALSA_PLAYBACK_QUEUE_ITEM *items; /* queue items */
}BT_ALSA_PLAYBACK_QUEUE;


typedef struct
{
    unsigned char *buf;
    unsigned int data_len;
}BT_ALSA_PLAYBACK_MEM_ITEM;

typedef struct
{
    unsigned int w_pos; /* write position */
    unsigned int r_pos;  /* read position */
    unsigned int w_cnt; /* write counter */
    unsigned int r_cnt;  /* read counter */


    unsigned char *data_mem;
    BT_ALSA_PLAYBACK_MEM_ITEM mem_items[BT_ALSA_PLAYBACK_BUF_CNT];
}BT_ALSA_PLAYBACK_MEM_QUEUE;

typedef struct
{
    int inited;
    int has_q_data;  /* 0-no data in cmd_q/data_q, 1-has data in cmd_q/data_q */
    pthread_t work_thread; /* work thread, process dsp_open/close/write */
    pthread_mutex_t lock;
    pthread_mutex_t mem_lock; /* memory pool lock */
    pthread_cond_t signal;

    UINT8 *dummy_buf; /* it's used to write dummy pcm to alsa to avoid underrun */
    UINT32 dummy_len;

    BT_ALSA_PLAYBACK_QUEUE *cmd_q;
    BT_ALSA_PLAYBACK_MEM_QUEUE *data_q;
}BT_ALSA_PLAYBACK_CB;


typedef struct
{
    UINT32 sample_rate; /* sample rate */
    UINT32 bitdepth; /* sample rate */
    UINT64 in_frm; /* write in frames, 1 frame=4 bytes */
    UINT64 fail_frm; /* write fail frames */
    UINT64 drop_frm; /* buffer dropped because queue if full */
    struct timespec start_in_tm; /* first start write in tm */
    struct timespec last_in_tm; /* first start write in tm */

    struct timespec start_tm; /* write in tm at open */
    UINT64 start_frm; /* write in frames at open, 1 frame=4 bytes */
    UINT64 max_in_us; /* max write time */
    UINT64 max_write_interval; /* max write interval */

    struct timespec last_write_end_tm;
    UINT32 last_write_diff; /* last write interval */
}BT_ALSA_PLAYBACK_DBG_STAT;

typedef struct
{
    BT_ALSA_PLAYBACK_DBG_STAT dsp_sta; /* write stat by playback  */
    BT_ALSA_PLAYBACK_DBG_STAT pb_sta;  /* write stat by audio track */
}BT_ALSA_PLAYBACK_DBG_STAT_CB;

static BT_ALSA_PLAYBACK_CB s_bt_alsa_pb_cb;

static BT_ALSA_PLAYBACK_DBG_STAT_CB s_bt_alsa_pb_dbg_cb;

static void bt_a2dp_alsa_pb_enq_cmd(BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd);
static int bt_a2dp_alsa_pb_enq_data(unsigned char *data, int len, int prepared);
static int bt_a2dp_alsa_pb_init(BT_A2DP_PLAYER_EVENT_CB event_cb);
static int bt_a2dp_alsa_pb_deinit(void);

#ifdef BT_ALSA_PLAYBACK_TEST
static void bt_a2dp_alsa_pb_test(void);
#endif /* BT_ALSA_PLAYBACK_TEST */

#endif /* BT_ALSA_PLAYBACK_DETACH_IN_THREAD */


#if defined(MTK_BT_PLAYBACK_DEFAULT_ALSA)
#define ALSA_DEVICE_PLAYER "default" /* for MTK branch tree */
#else
#define ALSA_DEVICE_PLAYER "main"
#endif
#define FRAGMENT_SAMPLES    (4096*4)


#ifdef BT_ALSA_ENABLE_FADE
#define BT_A2DP_ALSA_FADE_IN_FRAME 2048
#define BT_A2DP_ALSA_FADE_OUT_FRAME 1024
#endif

static snd_pcm_t *s_alsa_handle = NULL;

static snd_pcm_uframes_t chunk_size = 0;

static UINT32 u4buffer_time = 0; /* ring buffer length in us */
static UINT32 u4period_time = 20000; /* period time in us */

static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static INT32 i4avail_min = -1;
static INT32 i4start_delay = 20000; /* start threshold, unit: us */
static INT32 i4stop_delay = 0;

static UINT32 u4wait_time_ms = 0;
static UINT32 u4zero_time_ms = 0; /* after alsa open, write 0 data, unit:ms */

static UINT32 s_a2dp_sample_bytes = 4;
static UINT32 s_a2dp_bitdepth = 16;
static UINT32 s_a2dp_sample_rate = 44100;

static const UINT32 s_a2dp_alsa_pb_prepare_ms = 80;
static UINT32 s_a2dp_alsa_pb_preparing = 0;
static INT32 s_a2dp_alsa_pb_prepare_frames = 0; /* it's the prepared frames */

#ifdef BT_ALSA_ENABLE_FADE
static INT32 s_a2dp_fade_in_frame_max = BT_A2DP_ALSA_FADE_IN_FRAME;
static INT32 s_a2dp_fade_in_frame_idx = 0;

static INT32 s_a2dp_fade_out_frame_max = BT_A2DP_ALSA_FADE_OUT_FRAME;
static INT32 s_a2dp_fade_out_frame_idx = 0;

static UINT32 s_a2dp_fade_channels = 0;
#endif

BT_PLAYBACK_CB_T g_bt_playback_cb = {0};
BT_A2DP_ALSA_PB_STATUS audio_status = BT_A2DP_ALSA_PB_STATUS_UNINIT;
static BT_A2DP_PLAYER g_bt_a2dp_alsa_player = {0};

FILE *s_bt_mw_a2dp_pb_pcm_fp = NULL;

INT32 bt_a2dp_alsa_player_start(INT32 trackFreq, INT32 channelType, INT32 bitDepth);
INT32 bt_a2dp_alsa_player_play(VOID);
INT32 bt_a2dp_alsa_player_pause(VOID);
INT32 bt_a2dp_alsa_player_stop(VOID);
INT32 bt_a2dp_alsa_player_write(VOID *data, INT32 datalen);
INT32 bt_a2dp_alsa_player_dump(BT_A2DP_SINK_DUMP_FLAG flag);
static INT32 bt_a2dp_alsa_player_adjust_buf_time(UINT32 buffer_time);
static CHAR* bt_a2dp_alsa_pb_get_cmd_str(INT32 cmd);
static CHAR* bt_a2dp_alsa_pb_get_status_str(BT_A2DP_ALSA_PB_STATUS status);

#ifdef BT_ALSA_ENABLE_FADE
static int bt_a2dp_alsa_fade_in(void *frame_buf, int frame_num, int bitdepth, int channel);
static int bt_a2dp_alsa_fade_out(void *frame_buf, int frame_num, int bitdepth, int channel);
#endif

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
static VOID bt_a2dp_alsa_pb_dump_pb_stat(VOID);
static VOID bt_a2dp_alsa_pb_dump_alsa_stat(CHAR *title);
static VOID bt_a2dp_alsa_pb_alsa_stat_check(VOID);
static VOID bt_a2dp_alsa_pb_dump_stat_2_file(CHAR *title, INT32 to_file);
#endif

static INT32 bt_a2dp_alsa_player_adjust_buf_time(UINT32 buffer_time)
{
    ALSA_DBG_ERROR("<addbuf> ADD_BUFFER is disabled!");
    return 0;
}

EXPORT_SYMBOL BT_A2DP_PLAYER* bt_a2dp_alsa_get_player(VOID)
{
    strncpy(g_bt_a2dp_alsa_player.name, "alsa_player", 31);
    g_bt_a2dp_alsa_player.name[31] = '\0';
    g_bt_a2dp_alsa_player.support_codec_mask = (1 << BT_A2DP_CODEC_TYPE_STE) - 1;
    g_bt_a2dp_alsa_player.init = bt_a2dp_alsa_pb_init;
    g_bt_a2dp_alsa_player.deinit = bt_a2dp_alsa_pb_deinit;
    g_bt_a2dp_alsa_player.start = bt_a2dp_alsa_player_start;
    g_bt_a2dp_alsa_player.stop = bt_a2dp_alsa_player_stop;
    g_bt_a2dp_alsa_player.play = bt_a2dp_alsa_player_play;
    g_bt_a2dp_alsa_player.pause = bt_a2dp_alsa_player_pause;
    g_bt_a2dp_alsa_player.write = bt_a2dp_alsa_player_write;
    g_bt_a2dp_alsa_player.adjust_buf_time = bt_a2dp_alsa_player_adjust_buf_time;
    g_bt_a2dp_alsa_player.dump = bt_a2dp_alsa_player_dump;

#ifdef BT_ALSA_PLAYBACK_TEST
    sleep(1);
    bt_a2dp_alsa_pb_test();
#endif

    return &g_bt_a2dp_alsa_player;
}


static INT32 set_params(INT32 fs, INT32 channel_num, INT32 bitdepth)
{
    INT32 i4ret;
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
        ALSA_DBG_ERROR("Broken configuration for playback: no configurations available: %s", snd_strerror(i4ret));
        return i4ret;
    }

    ALSA_DBG_NORMAL("bitdepth:%d", bitdepth);
    /* set the sample format */
    if (32 == bitdepth)
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S32_LE);
    }
    else if (24 == bitdepth)
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S24_LE);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S16_LE);
    }
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Sample format not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the interleaved read/write format */
    i4ret = snd_pcm_hw_params_set_access(s_alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Access type not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the count of channels */
    i4ret = snd_pcm_hw_params_set_channels(s_alsa_handle, hwparams, channel_num);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Channels count (%i) not available for playbacks: %s", channel_num, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the stream sampling rate */
    i4ret = snd_pcm_hw_params_set_rate(s_alsa_handle, hwparams, fs, 0);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }

    u4rate = fs;
    if ((u4buffer_time == 0) && (buffer_frames == 0))
    {
        i4ret = snd_pcm_hw_params_get_buffer_time_max(hwparams, &u4buffer_time, 0);
        if (i4ret < 0)
        {
            ALSA_DBG_ERROR("fail to get max buffer time:%d, %s", i4ret, snd_strerror(i4ret));
            return i4ret;
        }
        ALSA_DBG_NORMAL("u4buffer_time:%d", u4buffer_time);
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
        ALSA_DBG_ERROR("fail to get period size:%d, %s", i4ret, snd_strerror(i4ret));
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
        ALSA_DBG_ERROR("fail to get buffer size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    i4ret = snd_pcm_hw_params(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Unable to install hw params");
        return i4ret;
    }

    snd_pcm_hw_params_get_period_size(hwparams, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    ALSA_DBG_NORMAL("chunk_size:%lu, buffer_size:%lu", chunk_size, buffer_size);
    if (chunk_size == buffer_size)
    {
        ALSA_DBG_ERROR("Can't use period equal to buffer size (%lu == %lu)",
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
    i4ret = snd_pcm_sw_params_set_avail_min(s_alsa_handle, swparams, n);

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
        ALSA_DBG_ERROR("fail to set start threshold:%d, %s", i4ret, snd_strerror(i4ret));
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
        ALSA_DBG_ERROR("fail to set stop threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    /* write the parameters to the playback device */
    if ((i4ret = snd_pcm_sw_params(s_alsa_handle, swparams)) < 0)
    {
        ALSA_DBG_ERROR("unable to install sw params");
        return i4ret;
    }

    snd_pcm_sw_params_get_start_threshold(swparams, &start_threshold);
    snd_pcm_sw_params_get_stop_threshold(swparams, &stop_threshold);
    ALSA_DBG_NORMAL("start_threshold:%lu, stop_threshold:%lu", start_threshold, stop_threshold);
    //snd_pcm_hw_params_free(hwparams);
    //snd_pcm_sw_params_free(swparams);
    ALSA_DBG_NORMAL("---exit");
    return 0;
}

static void dsp_start_all_cpu(void)
{
#if 0
    ALSA_DBG_NORMAL("start all cpu");
#ifdef ENABLE_IPCD
    ipcd_exec("echo 0 > /proc/hps/enabled", NULL);
    ipcd_exec("echo 1 > /sys/devices/system/cpu/cpu1/online", NULL);
    ipcd_exec("echo 1 > /sys/devices/system/cpu/cpu2/online", NULL);
    ipcd_exec("echo 1 > /sys/devices/system/cpu/cpu3/online", NULL);
#endif
#endif
}

static void dsp_stop_all_cpu(void)
{
#if 0
    ALSA_DBG_NORMAL("stop all cpu");
#ifdef ENABLE_IPCD
    ipcd_exec("echo 1 > /proc/hps/enabled", NULL);
#endif
#endif
}


static INT32 dsp_open(INT32 fs, INT32 channel_num, INT32 bitdepth)
{
    ALSA_DBG_NORMAL("+++into");
    INT32 i4ret = 0;
    INT32 write_cnt = 0;

    //snd_pcm_hw_params_t *hwparams;

    if (s_alsa_handle != NULL)
    {
        ALSA_DBG_WARNING("---exit already opened s_alsa_handle");
        return 0;
    }
    dsp_start_all_cpu();

    if (bitdepth != 0)
    {
        if (bitdepth == 24)
        {
            s_a2dp_bitdepth = 32;
            s_a2dp_sample_bytes = 8;
        }
        else
        {
            s_a2dp_sample_bytes = 4;
            s_a2dp_bitdepth = 16;
        }
    }
    else
    {
        s_a2dp_sample_bytes = 4;
        s_a2dp_bitdepth = 16;
    }
    s_a2dp_sample_rate = fs;

    if (s_a2dp_alsa_pb_prepare_ms > 0)
    {
        s_a2dp_alsa_pb_preparing = 1;
        s_a2dp_alsa_pb_prepare_frames =
            s_a2dp_alsa_pb_prepare_ms * s_a2dp_sample_rate / 1000;
    }

    i4ret = snd_pcm_open(&s_alsa_handle, ALSA_DEVICE_PLAYER, SND_PCM_STREAM_PLAYBACK, 0 );
    ALSA_DBG_NORMAL("fs %d, channel num %d bitdepth=%d sample_bytes=%d, prepare=%d, start_us=%d",
        fs, channel_num, bitdepth, s_a2dp_sample_bytes,
        s_a2dp_alsa_pb_prepare_frames, i4start_delay);
    ALSA_DBG_NORMAL("dsp_open %s i4ret=%d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Cannot open %s ERROR %d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
        s_alsa_handle = NULL;
        return  -1;
    }

    set_params(fs, channel_num, bitdepth);
    snd_pcm_prepare(s_alsa_handle);

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    memset((void*)&s_bt_alsa_pb_dbg_cb.dsp_sta, 0, sizeof(s_bt_alsa_pb_dbg_cb.dsp_sta));
    s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate = fs;
    s_bt_alsa_pb_dbg_cb.dsp_sta.bitdepth = bitdepth;
#endif

    write_cnt = u4zero_time_ms * fs / 1000 * s_a2dp_sample_bytes;
    if (0 < write_cnt)
    {
        UINT8 *buf = NULL;
        UINT64 diff_us;
        struct timespec tv1, tv2;
        clock_gettime(CLOCK_MONOTONIC, &tv1);
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm = tv1;
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_frm = write_cnt;
#endif
        buf = (UINT8 *)malloc(write_cnt);
        if (buf)
        {
            memset(buf, 0, sizeof(write_cnt));
            i4ret = snd_pcm_writei(s_alsa_handle, buf, write_cnt/s_a2dp_sample_bytes);

            int ret = snd_pcm_drop(s_alsa_handle);
            ALSA_DBG_NORMAL("alsa snd_pcm_drop %d", ret);
            ret = snd_pcm_prepare(s_alsa_handle);
            ALSA_DBG_NORMAL("alsa snd_pcm_prepare %d", ret);
        }
        free(buf);
        clock_gettime(CLOCK_MONOTONIC, &tv2);
        diff_us = (tv2.tv_sec - tv1.tv_sec) * 1000000ULL + (tv2.tv_nsec - tv1.tv_nsec)/1000;
        ALSA_DBG_ERROR("alsa write zero time %lld us", diff_us);
        if (i4ret < 0)
        {
            ALSA_DBG_ERROR("ALSA ERROR in dsp open %d[%s]", i4ret, snd_strerror(i4ret));
        }
    }

#ifdef BT_ALSA_ENABLE_FADE
    /* prepare fade in */
    s_a2dp_fade_in_frame_idx = 0;
    s_a2dp_fade_out_frame_idx = -1;
    s_a2dp_fade_channels = channel_num;

    if (s_a2dp_alsa_pb_prepare_frames > 0)
    {
        s_a2dp_fade_in_frame_max = s_a2dp_alsa_pb_prepare_frames;
    }

    ALSA_DBG_NORMAL("wait %d ms, zero %d ms, fade in %u frames",
        u4wait_time_ms, u4zero_time_ms, s_a2dp_fade_in_frame_max);
#else
    ALSA_DBG_NORMAL("wait %d ms, zero %d ms", u4wait_time_ms, u4zero_time_ms);
#endif

    audio_status = BT_A2DP_ALSA_PB_STATUS_OPENED;

    BT_A2DP_ALSA_PB_REPORT_EVENT(BT_A2DP_ALSA_PB_EVENT_START);

    ALSA_DBG_NORMAL("---exit");
    return 0;
}

static void bt_a2dp_alsa_check_delay(snd_pcm_t *handle)
{
    snd_pcm_sframes_t buf_frame_avail = 0;
    snd_pcm_status_t *status = NULL;
    INT32 i4ret = 0;

    if (handle == NULL) return;

    snd_pcm_status_alloca(&status);

    i4ret = snd_pcm_status(handle, status);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("ALSA snd_pcm_status ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        return;
    }

    buf_frame_avail = (snd_pcm_sframes_t)snd_pcm_status_get_delay(status);
    ALSA_DBG_NORMAL("alsa buf %ld, left buf:%d",
        buf_frame_avail, s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);

    return;
}

static INT64 bt_a2dp_alsa_get_delay(snd_pcm_t *handle)
{
    snd_pcm_sframes_t buf_frame_avail = 0;
    snd_pcm_status_t *status = NULL;
    INT32 i4ret = 0;

    if (handle == NULL) return 0;

    snd_pcm_status_alloca(&status);

    i4ret = snd_pcm_status(handle, status);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("ALSA snd_pcm_status ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        return 0;
    }

    buf_frame_avail = (snd_pcm_sframes_t)snd_pcm_status_get_delay(status);

    return buf_frame_avail;
}

static INT32 dsp_write(UINT8 *buf, UINT32 size)
{
    INT32 i4ret = 0;

    if (s_alsa_handle == NULL)
    {
        ALSA_DBG_ERROR("s_alsa_handle == NULL");
        return -1;
    }

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    if (0 == s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm)
    {
        clock_gettime(CLOCK_MONOTONIC, &s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm);
        clock_gettime(CLOCK_MONOTONIC, &s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm);
        {
            ALSA_DBG_NORMAL("write 1st buffer, time=%lld.%06d, buf=%d",
                (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_sec,
                (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_nsec/1000,
                s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);
        }
    }
    else
    {
        struct timespec last_in_tm = s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm;
        UINT32 diff = 0;
        clock_gettime(CLOCK_MONOTONIC, &s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm);
        diff = BT_A2DP_ALSA_PB_DIFF_TM_US(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
            last_in_tm);
        if (diff > s_bt_alsa_pb_dbg_cb.dsp_sta.max_write_interval)
        {
            s_bt_alsa_pb_dbg_cb.dsp_sta.max_write_interval = diff;
        }

#if 0
        UINT64 play_diff = 0;
        UINT32 schedule_diff = 0;
        schedule_diff = BT_A2DP_ALSA_PB_DIFF_TM_US(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
            s_bt_alsa_pb_dbg_cb.dsp_sta.last_write_end_tm);
        play_diff = (UINT64)(size/s_a2dp_sample_bytes*1000000)/(UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate;
        if (diff > 50000 /*play_diff */)
        {
            ALSA_DBG_ERROR("diff(%u) is long, > %llu us, last write diff: %u us, schedule diff: %u us, left_buf: %d, pb_write_diff: %d",
                diff, play_diff, s_bt_alsa_pb_dbg_cb.dsp_sta.last_write_diff, schedule_diff,
                s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt,
                s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval);
            s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval = 0; /* clear it */
            bt_a2dp_alsa_check_delay(s_alsa_handle);
        }
#endif
        //bt_a2dp_alsa_pb_alsa_stat_check();
    }

    struct timespec prev;
    clock_gettime(CLOCK_MONOTONIC, &prev);
#endif

    if (s_bt_mw_a2dp_pb_pcm_fp)
    {
        fwrite ((buf), 1, (size_t)size, s_bt_mw_a2dp_pb_pcm_fp);
    }

    i4ret = snd_pcm_writei(s_alsa_handle, buf, size/s_a2dp_sample_bytes);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("ALSA ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        snd_pcm_prepare(s_alsa_handle);
        if ((i4ret = snd_pcm_prepare(s_alsa_handle))<0)
        {
            ALSA_DBG_ERROR("ALSA snd_pcm_prepare ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        }
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        s_bt_alsa_pb_dbg_cb.dsp_sta.fail_frm += (size / s_a2dp_sample_bytes);
        bt_a2dp_alsa_pb_dump_alsa_stat("write error");
#endif
    }
    else
    {
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        struct timespec now;
        UINT32 diff = 0;
        clock_gettime(CLOCK_MONOTONIC, &now);
        diff = BT_A2DP_ALSA_PB_DIFF_TM_US(now, prev);
        if (diff > s_bt_alsa_pb_dbg_cb.dsp_sta.max_in_us)
        {
            s_bt_alsa_pb_dbg_cb.dsp_sta.max_in_us = diff;
        }
        s_bt_alsa_pb_dbg_cb.dsp_sta.last_write_diff = diff;
        s_bt_alsa_pb_dbg_cb.dsp_sta.last_write_end_tm = now;
        s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm += (size / s_a2dp_sample_bytes);

        if (diff > 50000)
        {
            ALSA_DBG_NORMAL("write long %u us, write=%u, in_frm=%llu, time=(%lld.%06d-%lld.%06d), buf=%d",
                diff, (size / s_a2dp_sample_bytes), s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm,
                (UINT64)prev.tv_sec, (UINT32)prev.tv_nsec/1000,
                (UINT64)now.tv_sec, (UINT32)now.tv_nsec/1000,
                s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);
        }
#endif
    }
    ALSA_DBG_MINOR("alsa write i4ret = %d samples", i4ret);

#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
    i4ret = bt_a2dp_alsa_get_delay(s_alsa_handle) * 1000000 / s_a2dp_sample_rate;
    if (i4ret > 1000)
    {
        i4ret -= 1000;
    }
#endif

    return i4ret;
}

static INT32 dsp_close(VOID)
{
    INT32 i4ret = 0;
    ALSA_DBG_NORMAL("+++into");
    if (s_alsa_handle == NULL)
    {
        ALSA_DBG_ERROR("---exit s_alsa_handle == NULL");
        return -1;
    }

    dsp_stop_all_cpu();

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    bt_a2dp_alsa_pb_dump_alsa_stat("dsp close");
#endif

    if (s_alsa_handle != NULL)
    {
        i4ret = snd_pcm_close(s_alsa_handle);
        if (i4ret == 0)
        {
            ALSA_DBG_NORMAL("dsp_close success");
            BT_A2DP_ALSA_PB_REPORT_EVENT(BT_A2DP_ALSA_PB_EVENT_STOP);
        }
        else
        {
            ALSA_DBG_ERROR("dsp_close fail i4ret=%d[%s]", i4ret, snd_strerror(i4ret));
        }
        s_alsa_handle = NULL;
    }
    s_a2dp_alsa_pb_preparing = 0;
    audio_status = BT_A2DP_ALSA_PB_STATUS_UNINIT;

    ALSA_DBG_NORMAL("---exit");
    return i4ret;
}

INT32 bt_a2dp_alsa_player_start(INT32 trackFreq, INT32 channelNum, INT32 bitDepth)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into trackFreq=%d, channelNum=%d, bitDepth=%d", trackFreq, channelNum, bitDepth);

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    memset((void*)&s_bt_alsa_pb_dbg_cb.pb_sta, 0, sizeof(s_bt_alsa_pb_dbg_cb.pb_sta));
    s_bt_alsa_pb_dbg_cb.pb_sta.sample_rate = trackFreq;
#endif

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_OPEN;
    cmd.u.param.fs = trackFreq;
    cmd.u.param.channels = channelNum;
    cmd.u.param.bitdepth = bitDepth;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);

    return BT_SUCCESS;
#else
    INT32 i4_ret = BT_SUCCESS;

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        i4_ret = dsp_open(trackFreq, channelNum, bitDepth);
        g_bt_playback_cb.fgPlayBackInit = TRUE;
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have init,no need init again");
    }
    return i4_ret;
#endif
}

INT32 bt_a2dp_alsa_player_stop(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    //bt_a2dp_alsa_pb_dump_pb_stat();
#endif

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_CLOSE;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    ALSA_DBG_NORMAL("+++into");
    INT32 i4_ret = BT_SUCCESS;


    if (TRUE == g_bt_playback_cb.fgPlayBackInit)
    {
        g_bt_playback_cb.fgPlayBackInit = FALSE;
        i4_ret = dsp_close();
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have not init,no need deinit");
    }

    ALSA_DBG_NORMAL("---exit");
    return i4_ret;
#endif
}


static INT32 bt_a2dp_alsa_pb_write_data(UINT8 *pPcmBuf, UINT32 u4PcmLen)
{
    if ((NULL == pPcmBuf) || (0 >= u4PcmLen))
    {
        ALSA_DBG_ERROR("invalid data");
        return BT_ERR_STATUS_PARM_INVALID;
    }

#ifdef BT_ALSA_ENABLE_FADE_OUT
    if (s_a2dp_fade_out_frame_idx >= 0)
    {
        ALSA_DBG_NORMAL("fade out %d", s_a2dp_fade_out_frame_idx);
        bt_a2dp_alsa_fade_out(pPcmBuf, u4PcmLen/s_a2dp_sample_bytes,
            s_a2dp_bitdepth, s_a2dp_fade_channels);
    }
#endif

    ALSA_DBG_MINOR("send %ld data", (long)u4PcmLen);
    return dsp_write(pPcmBuf, u4PcmLen);
}

INT32 bt_a2dp_alsa_player_play(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_PLAY;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if ((audio_status != BT_A2DP_ALSA_PB_STATUS_OPENED)
            && (audio_status != BT_A2DP_ALSA_PB_STATUS_PAUSED)
            && (audio_status != BT_A2DP_ALSA_PB_STATUS_STOPED))
    {
        ALSA_DBG_NORMAL("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PLAYED;
        ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_a2dp_alsa_player_pause(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_PAUSE;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    ALSA_DBG_NORMAL("+++into");

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PAUSED;
        ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_a2dp_alsa_player_write(VOID *data, INT32 datalen)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    if (0 == s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm)
    {
        clock_gettime(CLOCK_MONOTONIC, &s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm);
        clock_gettime(CLOCK_MONOTONIC, &s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm);
    }
    else
    {
        struct timespec now_ts;
        UINT32 diff = 0;

        clock_gettime(CLOCK_MONOTONIC, &now_ts);

        diff = BT_A2DP_ALSA_PB_DIFF_TM_US(now_ts,
            s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm);
        if (diff > s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval)
        {
            s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval = diff;
        }

        s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm = now_ts;
    }
    s_bt_alsa_pb_dbg_cb.pb_sta.in_frm += (datalen / s_a2dp_sample_bytes);
#endif

    if (s_a2dp_alsa_pb_preparing && s_a2dp_alsa_pb_prepare_ms > 0)
    {
        ALSA_DBG_INFO("preparing, prepare_frames=%d, rx_frames=%d, buf=%d",
            s_a2dp_alsa_pb_prepare_frames, (datalen / s_a2dp_sample_bytes),
            s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);

        s_a2dp_alsa_pb_prepare_frames -= (datalen / s_a2dp_sample_bytes);

        if (s_a2dp_alsa_pb_prepare_frames <= 0)
        {
            s_a2dp_alsa_pb_preparing = 0;
            ALSA_DBG_NORMAL("prepare ready, buf=%d",
                s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);
        }
    }

#ifdef BT_ALSA_ENABLE_FADE_IN
    if (s_a2dp_fade_in_frame_idx >= 0)
    {
        ALSA_DBG_INFO("fade in %d", s_a2dp_fade_in_frame_idx);
        bt_a2dp_alsa_fade_in(data, datalen/s_a2dp_sample_bytes,
            s_a2dp_bitdepth, s_a2dp_fade_channels);
    }
#endif

    return bt_a2dp_alsa_pb_enq_data(data, datalen, !s_a2dp_alsa_pb_preparing);

#else
    UINT32 u4PcmLen = datalen;
    UINT8 *pu1PcmBuf = (UINT8 *)data;
    INT32 i4_ret = BT_SUCCESS;

    if (NULL == data || 0 == datalen)
    {
        ALSA_DBG_ERROR("data is null(%p) or data len=%d", data, datalen);
        return BT_ERR_STATUS_FAIL;
    }

    ALSA_DBG_INFO("data=%p, datalen=%u", data, datalen);

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("bt playback does not init");
        return i4_ret;
    }


    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("audio_status is not played %d", audio_status);
        return i4_ret;
    }

    if (BT_SUCCESS > bt_a2dp_alsa_pb_write_data(pu1PcmBuf, u4PcmLen))
    {
        ALSA_DBG_ERROR("play pcm failed, pcmbuf:%p, len:%lu", pu1PcmBuf, (long)u4PcmLen);
        return i4_ret;
    }

    return i4_ret;
#endif
}

INT32 bt_a2dp_alsa_player_dump(BT_A2DP_SINK_DUMP_FLAG flag)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_DUMP;
    cmd.u.param.fs = flag;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
#endif

    return BT_SUCCESS;
}


/******************************************************************************
 */



#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD

static BT_ALSA_PLAYBACK_QUEUE* bt_a2dp_alsa_pb_alloc_q(int capacity)
{
    BT_ALSA_PLAYBACK_QUEUE* q = NULL;
    q = (BT_ALSA_PLAYBACK_QUEUE *)malloc(sizeof(BT_ALSA_PLAYBACK_QUEUE));
    if (NULL == q)
    {
        ALSA_DBG_ERROR("allocate queue fail");
        return NULL;
    }

    q->w_pos = 0;
    q->r_pos = 0;
    q->w_cnt = 0;
    q->r_cnt = 0;
    q->capacity = capacity;
    q->items = (BT_ALSA_PLAYBACK_QUEUE_ITEM *)malloc(sizeof(BT_ALSA_PLAYBACK_QUEUE_ITEM)*capacity);
    if (NULL == q->items)
    {
        free(q);
        ALSA_DBG_ERROR("allocate queue items fail");
        return NULL;
    }

    return q;
}

static void bt_a2dp_alsa_pb_free_q(BT_ALSA_PLAYBACK_QUEUE *q)
{
    if (NULL == q)
    {
        return;
    }

    if (NULL != q->items)
    {
        free(q->items);
        q->items = NULL;
    }

    free(q);

    return;
}

static int bt_a2dp_alsa_pb_enq(BT_ALSA_PLAYBACK_QUEUE *q, BT_ALSA_PLAYBACK_QUEUE_ITEM *items)
{
    if (NULL == q)
    {
        return -1;
    }

    if ((q->w_cnt - q->r_cnt) >= q->capacity)
    {
        ALSA_DBG_ERROR("queue is full, w_cnt=%u, r_cnt=%u, capcity=%u",
            q->w_cnt, q->r_cnt, q->capacity);
        return -1;
    }

    q->items[q->w_pos] = *items;
    q->w_pos++;
    q->w_cnt++;
    if (q->w_pos >= q->capacity)
    {
        q->w_pos = 0;
    }
    return 0;
}

static int bt_a2dp_alsa_pb_deq(BT_ALSA_PLAYBACK_QUEUE *q, BT_ALSA_PLAYBACK_QUEUE_ITEM *items)
{
    if (NULL == q)
    {
        return -1;
    }

    if (q->w_cnt == q->r_cnt)
    {
        //ALSA_DBG_ERROR("queue is empty, w_cnt=%u, r_cnt=%u, capcity=%u",
        //    q->w_cnt, q->r_cnt, q->capacity);
        return -1;
    }

    *items = q->items[q->r_pos];
    q->r_pos++;
    q->r_cnt++;
    if (q->r_pos >= q->capacity)
    {
        q->r_pos = 0;
    }

    return 0;
}

#ifdef BT_ALSA_PLAYBACK_MEM_CHECK
static void bt_a2dp_alsa_pb_mem_pool_check(BT_ALSA_PLAYBACK_MEM_NODE **mem_pool)
{
    BT_ALSA_PLAYBACK_MEM_NODE *node = NULL;
    char *temp_byte = 0;
    int i = 0;
    if ((NULL == mem_pool) || (NULL == *mem_pool))
    {
        return;
    }

    node = *mem_pool;
    while(NULL != node)
    {
        if (node < s_bt_alsa_pb_cb.data_mem
            || node >= s_bt_alsa_pb_cb.data_mem + BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT)
        {
            ALSA_DBG_ERROR("error data addr: %p over scope(%p~%p)", node,
                s_bt_alsa_pb_cb.data_mem, s_bt_alsa_pb_cb.data_mem
                + BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT);
            assert("error data");
        }
#if 0
        temp_byte = (char*)(node + 1);
        for (i=0;i<BT_ALSA_PLAYBACK_BUF_SIZE-sizeof(*node);i++)
        {
            if(temp_byte[i] != 0xaa)
            {
                ALSA_DBG_ERROR("error data[%d] %x", i, temp_byte[i]);
                assert("error data");
            }
        }
#endif
        node = node->next;
    }
}

#endif


static void bt_a2dp_alsa_pb_enq_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q,
    unsigned char *data, unsigned int data_len)
{
    if (NULL == mem_q || NULL == data || 0 == data_len)
    {
        ALSA_DBG_ERROR("mem_q(%p) or data(%p) or data_len(%u) is invalid",
            mem_q, data, data_len);
        return;
    }

    if ((mem_q->w_cnt - mem_q->r_cnt) >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        ALSA_DBG_ERROR("queue is full, w_cnt=%u, r_cnt=%u, capcity=%u, drop(len=%d)",
            mem_q->w_cnt, mem_q->r_cnt, BT_ALSA_PLAYBACK_BUF_CNT, data_len);
        if (s_a2dp_sample_bytes)
        {
            s_bt_alsa_pb_dbg_cb.dsp_sta.drop_frm += data_len/s_a2dp_sample_bytes;
        }

        bt_a2dp_alsa_check_delay(s_alsa_handle);
        return;
    }

    if (mem_q->w_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        ALSA_DBG_ERROR("invalid w_pos=%u", mem_q->w_pos);
        return;
    }

    if (NULL == mem_q->mem_items[mem_q->w_pos].buf)
    {
        ALSA_DBG_ERROR("mem_items[%u] buff null", mem_q->w_pos);
        return;
    }

    if ( data_len > BT_ALSA_PLAYBACK_BUF_SIZE)
    {
        ALSA_DBG_ERROR("data_len =%u to large", data_len);
        data_len = BT_ALSA_PLAYBACK_BUF_SIZE;
    }


    memcpy(mem_q->mem_items[mem_q->w_pos].buf, data, data_len);
    mem_q->mem_items[mem_q->w_pos].data_len = data_len;
    ALSA_DBG_MINOR("mem_items[%d], buf=%p, data_len =%u ", mem_q->w_pos,
        mem_q->mem_items[mem_q->w_pos].buf, data_len);

    mem_q->w_pos++;
    mem_q->w_cnt++;
    if (mem_q->w_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        mem_q->w_pos = 0;
    }

    return;
}



static int bt_a2dp_alsa_pb_deq_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        ALSA_DBG_ERROR("mem_q(%p) is invalid", mem_q);
        return -1;
    }

    if (mem_q->w_cnt == mem_q->r_cnt)
    {
        return -1;
    }

    mem_q->r_pos++;
    mem_q->r_cnt++;
    if (mem_q->r_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        mem_q->r_pos = 0;
    }

    return 0;
}

static int bt_a2dp_alsa_pb_peak_front_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q,
    BT_ALSA_PLAYBACK_MEM_ITEM *mem_item)
{
    if (NULL == mem_q || NULL == mem_item)
    {
        ALSA_DBG_ERROR("mem_q(%p) or mem_item(%p) is invalid", mem_q, mem_item);
        return -1;
    }

    if (mem_q->w_cnt == mem_q->r_cnt)
    {
        return -1;
    }

    *mem_item = mem_q->mem_items[mem_q->r_pos];

    ALSA_DBG_MINOR("mem_items[%d], buf=%p, data_len =%u ", mem_q->r_pos,
        mem_item->buf, mem_item->data_len);

    return 0;
}


static void bt_a2dp_alsa_pb_reset_mem_q(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        return;
    }

    while(0 == bt_a2dp_alsa_pb_deq_mem(mem_q));

    return;
}

static BT_ALSA_PLAYBACK_MEM_QUEUE * bt_a2dp_alsa_pb_alloc_mem_q(void)
{
    BT_ALSA_PLAYBACK_MEM_QUEUE *tmp_mem_q = NULL;
    int i = 0;

    tmp_mem_q = (BT_ALSA_PLAYBACK_MEM_QUEUE *)malloc(sizeof(BT_ALSA_PLAYBACK_MEM_QUEUE));
    if (NULL == tmp_mem_q)
    {
        ALSA_DBG_NORMAL("alloc buf fail");
        return NULL;
    }

    memset((void*)tmp_mem_q, 0, sizeof(BT_ALSA_PLAYBACK_MEM_QUEUE));

    tmp_mem_q->data_mem = malloc(BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT);
    if (NULL == tmp_mem_q->data_mem)
    {
        free(tmp_mem_q);
        return NULL;
    }

    for(i=0;i<BT_ALSA_PLAYBACK_BUF_CNT;i++)
    {
        tmp_mem_q->mem_items[i].data_len = 0;
        tmp_mem_q->mem_items[i].buf = tmp_mem_q->data_mem + i * BT_ALSA_PLAYBACK_BUF_SIZE;
    }

    return tmp_mem_q;
}

static void bt_a2dp_alsa_pb_free_mem_q(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        return;
    }

    free(mem_q->data_mem);

    free(mem_q);
    return;
}

static void bt_a2dp_alsa_pb_open(INT32 fs, INT32 channel_num, INT32 bit_depth)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        dsp_open(fs, channel_num, bit_depth);
        g_bt_playback_cb.fgPlayBackInit = TRUE;
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have init,no need init again");
    }
}

static void bt_a2dp_alsa_pb_close(void)
{
    if (TRUE == g_bt_playback_cb.fgPlayBackInit)
    {
        dsp_close();
        g_bt_playback_cb.fgPlayBackInit = FALSE;
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have not init,no need deinit");
    }
}

static void bt_a2dp_alsa_pb_play(void)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return;
    }
    if ((audio_status != BT_A2DP_ALSA_PB_STATUS_OPENED)
        && (audio_status != BT_A2DP_ALSA_PB_STATUS_PAUSED)
        && (audio_status != BT_A2DP_ALSA_PB_STATUS_STOPED))
    {
        ALSA_DBG_NOTICE("BT AUDIO wrong status, current status = %s",
            bt_a2dp_alsa_pb_get_status_str(audio_status));
    }
    else
    {
        if (BT_A2DP_ALSA_PB_STATUS_PLAYED != audio_status)
        {
            ALSA_DBG_NORMAL("BT AUDIO current status = %s",
                bt_a2dp_alsa_pb_get_status_str(audio_status));
        }
        audio_status = BT_A2DP_ALSA_PB_STATUS_PLAYED;

#ifdef BT_ALSA_ENABLE_FADE
        s_a2dp_fade_in_frame_idx = 0;
        s_a2dp_fade_out_frame_idx = -1;
#endif

        if (s_alsa_handle != NULL)
        {
            int ret = snd_pcm_prepare(s_alsa_handle);

            ALSA_DBG_NORMAL("alsa prepare ret=%d", ret);
        }

        if (s_a2dp_alsa_pb_preparing == 0 && s_a2dp_alsa_pb_prepare_ms > 0)
        {
            s_a2dp_alsa_pb_preparing = 1;
            s_a2dp_alsa_pb_prepare_frames =
                s_a2dp_alsa_pb_prepare_ms * s_a2dp_sample_rate / 1000;
            ALSA_DBG_NORMAL("to be preparing, prepare_frames=%d",
                s_a2dp_alsa_pb_prepare_frames);
        }
    }

    return;
}

static void bt_a2dp_alsa_pb_pause(void)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return;
    }

    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("BT AUDIO wrong status, current status = %s",
            bt_a2dp_alsa_pb_get_status_str(audio_status));
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PAUSED;
#ifdef BT_ALSA_ENABLE_FADE
        s_a2dp_fade_in_frame_idx = -1;
        s_a2dp_fade_out_frame_idx = 0;
#endif
        if (s_alsa_handle != NULL)
        {
            int ret = snd_pcm_drop(s_alsa_handle);

            ALSA_DBG_NORMAL("alsa drop ret=%d", ret);
        }
        if (s_a2dp_alsa_pb_preparing && s_a2dp_alsa_pb_prepare_ms > 0)
        {
            s_a2dp_alsa_pb_preparing = 0;
        }
        ALSA_DBG_NORMAL("BT AUDIO current status = %s, left buf=%d",
            bt_a2dp_alsa_pb_get_status_str(audio_status),
            s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);
    }

    return;
}

static void bt_a2dp_alsa_pb_dump(BT_A2DP_SINK_DUMP_FLAG flag)
{
    if (BT_A2DP_SINK_DUMP_PLAYER_STAT == flag
        || BT_A2DP_SINK_DUMP_PLAYER_STAT_2_FILE == flag)
    {
        INT32 save_2_file = 0;
        if (BT_A2DP_SINK_DUMP_PLAYER_STAT_2_FILE == flag)
        {
            save_2_file = 1;
        }
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        bt_mw_dump_info_begin("bt_player_dump.log");
        bt_a2dp_alsa_pb_dump_stat_2_file("player", save_2_file);
        bt_mw_dump_info_end();
#endif
    }
    else if (BT_A2DP_SINK_DUMP_PLAYER_PCM_START == flag)
    {
        ALSA_DBG_NORMAL("start dump pcm");

        s_bt_mw_a2dp_pb_pcm_fp = fopen("/tmp/bt_pb_dump.pcm", "wb+");
        if (s_bt_mw_a2dp_pb_pcm_fp == NULL)
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "open /tmp/bt_pb_dump.pcm fail");
            return;
        }
    }
    else if (BT_A2DP_SINK_DUMP_PLAYER_PCM_STOP == flag)
    {
        ALSA_DBG_NORMAL("stop dump pcm");

        if (s_bt_mw_a2dp_pb_pcm_fp)
        {
            fclose(s_bt_mw_a2dp_pb_pcm_fp);
            s_bt_mw_a2dp_pb_pcm_fp = NULL;
        }
    }


    return;
}


static INT32 bt_a2dp_alsa_pb_push(void *data, int datalen)
{
    UINT32 u4PcmLen = datalen;
    UINT8 *pu1PcmBuf = (UINT8 *)data;
    INT32 delay_ms = 0;

    if ((NULL == data) || (0 == datalen))
    {
        ALSA_DBG_ERROR("data is null(%p) or data len=%d", data, datalen);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ALSA_DBG_INFO("data=%p, datalen=%u", data, datalen);

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("bt playback does not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("audio_status is not played %d", audio_status);
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_SUCCESS > (delay_ms = bt_a2dp_alsa_pb_write_data(pu1PcmBuf, u4PcmLen)))
    {
        ALSA_DBG_ERROR("play pcm failed, pcmbuf:%p, len:%lu", pu1PcmBuf, (long)u4PcmLen);
        return BT_ERR_STATUS_FAIL;
    }

    return delay_ms;
}


static void bt_a2dp_alsa_pb_adjust_priority(int start)
{
    int priority = start ? -19 : -16;
    if (setpriority(PRIO_PROCESS, 0, priority) < 0)
    {
        ALSA_DBG_WARNING("failed to change priority to %d", priority);
    }

    return;
}



static int bt_a2dp_alsa_pb_handle_cmd(BT_ALSA_PLAYBACK_CB *pb_cb,
    BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd_item)
{
    if (NULL == cmd_item)
    {
        return 0;
    }
    ALSA_DBG_NORMAL("cmd type=%s(%d)",
        bt_a2dp_alsa_pb_get_cmd_str(cmd_item->type), cmd_item->type);

    if (BT_ALSA_PLAYBACK_CMD_OPEN == cmd_item->type)
    {
        bt_a2dp_alsa_pb_open(cmd_item->u.param.fs, cmd_item->u.param.channels, cmd_item->u.param.bitdepth);

        bt_a2dp_alsa_pb_adjust_priority(1);
    }
    else if(BT_ALSA_PLAYBACK_CMD_CLOSE == cmd_item->type)
    {
        bt_a2dp_alsa_pb_close();
        bt_a2dp_alsa_pb_reset_mem_q(pb_cb->data_q);
        bt_a2dp_alsa_pb_adjust_priority(0);
    }
    else if(BT_ALSA_PLAYBACK_CMD_PLAY == cmd_item->type)
    {
        bt_a2dp_alsa_pb_play();
    }
    else if(BT_ALSA_PLAYBACK_CMD_PAUSE == cmd_item->type)
    {
        bt_a2dp_alsa_pb_pause();
    }
    else if(BT_ALSA_PLAYBACK_CMD_QUIT == cmd_item->type)
    {
        pthread_exit(NULL);
    }
    else if(BT_ALSA_PLAYBACK_CMD_DUMP == cmd_item->type)
    {
        bt_a2dp_alsa_pb_dump(cmd_item->u.param.fs);
    }
    else
    {
        ALSA_DBG_ERROR("invalid cmd: %u", cmd_item->type);
    }

    return 0;
}

static void* bt_a2dp_alsa_pb_handler(void *data)
{
    BT_ALSA_PLAYBACK_CB *pb_cb = (BT_ALSA_PLAYBACK_CB *)data;
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd_item;
    BT_ALSA_PLAYBACK_MEM_ITEM mem_item;

#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
    INT32 wait_us = 0;
    struct timespec wait_tm;
    INT32 wait_ret = 0;
#endif
    prctl(PR_SET_NAME, "bt_alsa_pb", 0, 0, 0);

    memset((void*)&mem_item, 0, sizeof(mem_item));
    memset((void*)&cmd_item, 0, sizeof(cmd_item));

    while (1)
    {
        pthread_mutex_lock(&pb_cb->lock);
        if(!pb_cb->has_q_data)
        {
#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
            if (wait_us != 0)
            {
                clock_gettime(CLOCK_MONOTONIC, &wait_tm);

                wait_tm.tv_nsec += (UINT64)wait_us * 1000LL;
                if (wait_tm.tv_nsec > 1000000000) {
                    wait_tm.tv_nsec -= 1000000000;
                    wait_tm.tv_sec += 1;
                }

                wait_ret = pthread_cond_timedwait(&pb_cb->signal, &pb_cb->lock, &wait_tm);
            }
            else
            {
                pthread_cond_wait(&pb_cb->signal, &pb_cb->lock);
                wait_ret = 0;
            }
#else
            pthread_cond_wait(&pb_cb->signal, &pb_cb->lock);
#endif
        }
        pb_cb->has_q_data = 0;
        pthread_mutex_unlock(&pb_cb->lock);

        while(1)
        {
            if (0 == bt_a2dp_alsa_pb_deq(pb_cb->cmd_q, &cmd_item))
            {
#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
                wait_ret = 0; /* clear wait flag */
#endif
                if (bt_a2dp_alsa_pb_handle_cmd(pb_cb, &cmd_item) < 0)
                {
                    break;
                }

                /* handle cmd req prior */
                continue;
            }

            if (s_a2dp_alsa_pb_preparing && s_a2dp_alsa_pb_prepare_ms > 0) break;

            if (0 == bt_a2dp_alsa_pb_peak_front_mem(pb_cb->data_q, &mem_item))
            {
#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
                wait_ret = 0; /* clear wait flag */

                memcpy(pb_cb->dummy_buf, mem_item.buf, mem_item.data_len);
                pb_cb->dummy_len = mem_item.data_len;

                wait_us = bt_a2dp_alsa_pb_push(mem_item.buf, mem_item.data_len);
                if (wait_us < 0)
                {
                    wait_us = 0;
                }
#else
                bt_a2dp_alsa_pb_push(mem_item.buf, mem_item.data_len);
#endif
                bt_a2dp_alsa_pb_deq_mem(pb_cb->data_q);
            }
            else
            {
#ifdef BT_ALSA_PLAYBACK_WRITE_DUMMY
                if (wait_ret == ETIMEDOUT && pb_cb->dummy_len > 0)
                {
                    INT32 wait_us_dummy = 0;
                    INT64 left_frm = bt_a2dp_alsa_get_delay(s_alsa_handle);

                    wait_us_dummy = bt_a2dp_alsa_pb_push(pb_cb->dummy_buf, pb_cb->dummy_len);
                    if (wait_us_dummy < 0)
                    {
                        wait_us_dummy = 0;
                    }

                    ALSA_DBG_WARNING("dummy data(%u), timeout: %u us, left:%lld frms, after write: %u us ",
                        pb_cb->dummy_len, wait_us, left_frm, wait_us_dummy);

                    wait_us = wait_us_dummy;
                }
#endif
                break;
            }
        }
    }

    return NULL;
}

static int bt_a2dp_alsa_pb_init(BT_A2DP_PLAYER_EVENT_CB event_cb)
{
    pthread_condattr_t condattr;
    int ret;
    ALSA_DBG_NORMAL("inited=%d", s_bt_alsa_pb_cb.inited);
    if (1 == s_bt_alsa_pb_cb.inited)
    {
        return 0;
    }

    pthread_mutex_init(&s_bt_alsa_pb_cb.lock, NULL);
    pthread_mutex_init(&s_bt_alsa_pb_cb.mem_lock, NULL);

    pthread_cond_init(&s_bt_alsa_pb_cb.signal, NULL);

    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    ret = pthread_condattr_init(&condattr);
    if (ret < 0) {
        ALSA_DBG_ERROR("init condattr fail:%d\n", ret);
    }
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    ret = pthread_cond_init(&s_bt_alsa_pb_cb.signal, &condattr);
    if (ret < 0) {
        ALSA_DBG_ERROR("init cond fail:%d\n", ret);
    }

    s_bt_alsa_pb_cb.cmd_q = bt_a2dp_alsa_pb_alloc_q(BT_ALSA_PLAYBACK_CMD_Q_SIZE);
    if (NULL == s_bt_alsa_pb_cb.cmd_q)
    {
        ALSA_DBG_ERROR("alloc cmd q fail");
        goto __ERROR;
    }

    s_bt_alsa_pb_cb.dummy_buf = (UINT8 *)malloc(BT_ALSA_PLAYBACK_BUF_SIZE);

    s_bt_alsa_pb_cb.data_q = bt_a2dp_alsa_pb_alloc_mem_q();
    if (NULL == s_bt_alsa_pb_cb.data_q)
    {
        ALSA_DBG_ERROR("alloc mem mem_q fail");
        goto __ERROR;
    }

    if(0 < pthread_create(&s_bt_alsa_pb_cb.work_thread, NULL,
        bt_a2dp_alsa_pb_handler, (void*)&s_bt_alsa_pb_cb))
    {
        ALSA_DBG_ERROR("create bt alsa thread fail");
        goto __ERROR;
    }
    g_bt_a2dp_alsa_pb_event_cb = event_cb;
    s_bt_alsa_pb_cb.inited = 1;

    return 0;
__ERROR:
    if (NULL != s_bt_alsa_pb_cb.data_q)
    {
        bt_a2dp_alsa_pb_free_mem_q(s_bt_alsa_pb_cb.data_q);
        s_bt_alsa_pb_cb.data_q = NULL;
    }

    if (NULL != s_bt_alsa_pb_cb.cmd_q)
    {
        bt_a2dp_alsa_pb_free_q(s_bt_alsa_pb_cb.cmd_q);
        s_bt_alsa_pb_cb.cmd_q = NULL;
    }

    pthread_mutex_destroy(&s_bt_alsa_pb_cb.lock);
    pthread_mutex_destroy(&s_bt_alsa_pb_cb.mem_lock);
    pthread_cond_destroy(&s_bt_alsa_pb_cb.signal);
    return -1;
}

static int bt_a2dp_alsa_pb_deinit(void)
{
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd_item = {0};
    ALSA_DBG_NORMAL("inited=%d", s_bt_alsa_pb_cb.inited);
    if (0 == s_bt_alsa_pb_cb.inited)
    {
        return 0;
    }
    memset((void*)&cmd_item, 0, sizeof(cmd_item));
    cmd_item.type = BT_ALSA_PLAYBACK_CMD_QUIT;
    bt_a2dp_alsa_pb_enq(s_bt_alsa_pb_cb.cmd_q, &cmd_item);
    pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
    s_bt_alsa_pb_cb.has_q_data = 1;
    pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
    pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);

    pthread_join(s_bt_alsa_pb_cb.work_thread, NULL);

    free(s_bt_alsa_pb_cb.dummy_buf);

    if (s_bt_alsa_pb_cb.data_q != NULL)
    {
        bt_a2dp_alsa_pb_free_mem_q(s_bt_alsa_pb_cb.data_q);
        s_bt_alsa_pb_cb.data_q = NULL;
    }

    if (NULL != s_bt_alsa_pb_cb.cmd_q)
    {
        bt_a2dp_alsa_pb_free_q(s_bt_alsa_pb_cb.cmd_q);
        s_bt_alsa_pb_cb.cmd_q = NULL;
    }

    pthread_mutex_destroy(&s_bt_alsa_pb_cb.lock);
    pthread_mutex_destroy(&s_bt_alsa_pb_cb.mem_lock);
    pthread_cond_destroy(&s_bt_alsa_pb_cb.signal);

    g_bt_a2dp_alsa_pb_event_cb = NULL;

    s_bt_alsa_pb_cb.inited = 0;
    return 0;
}

static void bt_a2dp_alsa_pb_enq_cmd(BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd)
{
    if (NULL == cmd)
    {
        return;
    }
    ALSA_DBG_NORMAL("enq cmd=%s", bt_a2dp_alsa_pb_get_cmd_str(cmd->type));

    bt_a2dp_alsa_pb_enq(s_bt_alsa_pb_cb.cmd_q, cmd);

    pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
    s_bt_alsa_pb_cb.has_q_data = 1;
    pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
    pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);

    return;
}

static int bt_a2dp_alsa_pb_enq_data(unsigned char *data, int len, int prepared)
{
    if ((NULL == data) || (0 >= len))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    bt_a2dp_alsa_pb_enq_mem(s_bt_alsa_pb_cb.data_q, data, len);

    if (prepared)
    {
        pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
        s_bt_alsa_pb_cb.has_q_data = 1;
        pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
        pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);
    }
    return BT_SUCCESS;
}

#ifdef BT_ALSA_PLAYBACK_TEST
static void* bt_a2dp_alsa_pb_test_push_handler(void *data)
{
    unsigned char *buf = NULL;
    ALSA_DBG_NORMAL("start");
    sleep(10);
    buf = (unsigned char*)malloc(4800);
    if (NULL == buf)
    {
        ALSA_DBG_ERROR("alloc test buffer fail");
        return NULL;
    }
    bt_a2dp_alsa_pb_adjust_priority(1);

    while (1)
    {
        bt_a2dp_alsa_player_write(buf, 4800);
        usleep(random()%5000+20000);
    }

    return NULL;
}

static void* bt_a2dp_alsa_pb_test_open_close_handler(void *data)
{
    ALSA_DBG_NORMAL("start");
    sleep(10);
    while (1)
    {
        bt_a2dp_alsa_player_start(48000, 2, 16);
        usleep(random() % 1000000 + 100000);
        bt_a2dp_alsa_player_stop();
    }
    return NULL;
}

static void bt_a2dp_alsa_pb_test(void)
{
    pthread_t t1, t2, t3;
    ALSA_DBG_NORMAL("start");

    pthread_create(&t1, NULL, bt_a2dp_alsa_pb_test_open_close_handler, NULL);
    pthread_create(&t2, NULL, bt_a2dp_alsa_pb_test_push_handler, NULL);
    pthread_create(&t3, NULL, bt_a2dp_alsa_pb_test_open_close_handler, NULL);

    return;
}
#endif

#endif

#ifdef BT_ALSA_ENABLE_FADE
static int bt_a2dp_alsa_fade_in(void *frame_buf, int frame_num, int bitdepth, int channel)
{
    int frame_index;
    int channel_index;
    int16_t *buffer_16;
    int32_t *buffer_32;

    if (s_a2dp_fade_in_frame_idx >= s_a2dp_fade_in_frame_max
        || s_a2dp_fade_in_frame_idx < 0)
    {
        s_a2dp_fade_in_frame_idx = -1;
        return 0;
    }
    if (bitdepth == 16)
    {
        buffer_16 = (int16_t *)frame_buf;
        for (frame_index = 0; frame_index < frame_num; frame_index++)
        {
            for (channel_index = 0; channel_index < channel; channel_index++)
            {
                double val = buffer_16[frame_index * channel + channel_index];
                val = val * (double)(frame_index+s_a2dp_fade_in_frame_idx)
                    / (double)s_a2dp_fade_in_frame_max;
                buffer_16[frame_index * channel + channel_index] = (int16_t)val;
            }

            s_a2dp_fade_in_frame_idx++;
            if (s_a2dp_fade_in_frame_idx >= s_a2dp_fade_in_frame_max)
            {
                ALSA_DBG_NORMAL("fade in %d finish", s_a2dp_fade_in_frame_idx);
                s_a2dp_fade_in_frame_idx = -1;
                break;
            }
        }
    }
    if (bitdepth == 32 || bitdepth == 24)
    {
        buffer_32 = (int32_t *)frame_buf;
        for (frame_index = 0; frame_index < frame_num; frame_index++)
        {
            for (channel_index = 0; channel_index < channel; channel_index++)
            {
                double val = buffer_32[frame_index * channel + channel_index];
                val = val * (double)(frame_index+s_a2dp_fade_in_frame_idx)
                    / (double)s_a2dp_fade_in_frame_max;
                buffer_32[frame_index * channel + channel_index] = (int32_t)val;
            }

            s_a2dp_fade_in_frame_idx++;
            if (s_a2dp_fade_in_frame_idx >= s_a2dp_fade_in_frame_max)
            {
                ALSA_DBG_NORMAL("fade in %d finish", s_a2dp_fade_in_frame_idx);
                s_a2dp_fade_in_frame_idx = -1;
                break;
            }
        }
    }

    return 0;
}

static int bt_a2dp_alsa_fade_out(void *frame_buf, int frame_num, int bitdepth, int channel)
{
    int frame_index;
    int channel_index;
    int16_t *buffer_16;
    int32_t *buffer_32;

    if (s_a2dp_fade_out_frame_idx < 0)
    {
        return 0;
    }

    if (bitdepth == 16)
    {
        buffer_16 = (int16_t *)frame_buf;
        for (frame_index = 0; frame_index < frame_num; frame_index++)
        {
            for (channel_index = 0; channel_index < channel; channel_index++)
            {
                double val = buffer_16[frame_index * channel + channel_index];
                int frames = 0;
                if (s_a2dp_fade_out_frame_max >= (frame_index+s_a2dp_fade_out_frame_idx))
                {
                    frames = s_a2dp_fade_out_frame_max - (frame_index+s_a2dp_fade_out_frame_idx);
                }
                val = val * (double)(frames) / (double)s_a2dp_fade_out_frame_max;
                buffer_16[frame_index * channel + channel_index] = (int16_t)val;
            }

            s_a2dp_fade_out_frame_idx++;

            if (s_a2dp_fade_out_frame_idx >= s_a2dp_fade_out_frame_max)
            {
                ALSA_DBG_NORMAL("fade out %d finish", s_a2dp_fade_out_frame_idx);
                s_a2dp_fade_out_frame_idx = -1;
                break;
            }
        }
    }
    if (bitdepth == 32 || bitdepth == 24)
    {
        buffer_32 = (int32_t *)frame_buf;
        for (frame_index = 0; frame_index < frame_num; frame_index++)
        {
            for (channel_index = 0; channel_index < channel; channel_index++)
            {
                double val = buffer_32[frame_index * channel + channel_index];
                int frames = 0;
                if (s_a2dp_fade_out_frame_max >= (frame_index+s_a2dp_fade_out_frame_idx))
                {
                    frames = s_a2dp_fade_out_frame_max - (frame_index+s_a2dp_fade_out_frame_idx);
                    ALSA_DBG_NORMAL("frames = %d", frames);
                }
                buffer_32[frame_index * channel + channel_index] = (int32_t)val;
            }
            s_a2dp_fade_out_frame_idx++;

            if (s_a2dp_fade_out_frame_idx >= s_a2dp_fade_out_frame_max)
            {
                ALSA_DBG_NORMAL("fade out %d finish", s_a2dp_fade_out_frame_idx);
                s_a2dp_fade_out_frame_idx = -1;
                break;
            }
        }
    }

    return 0;
}
#endif

static CHAR* bt_a2dp_alsa_pb_get_cmd_str(INT32 cmd)
{
     switch(cmd)
     {
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_OPEN, "open");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_CLOSE, "close");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_PLAY, "play");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_PAUSE, "pause");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_DATA, "data");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_QUIT, "quit");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_ALSA_PLAYBACK_CMD_DUMP, "dump");
         default: return "unknown";
    }
}
static CHAR* bt_a2dp_alsa_pb_get_status_str(BT_A2DP_ALSA_PB_STATUS status)
{
     switch(status)
     {
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_A2DP_ALSA_PB_STATUS_UNINIT, "uninit");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_A2DP_ALSA_PB_STATUS_OPENED, "opened");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_A2DP_ALSA_PB_STATUS_PLAYED, "played");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_A2DP_ALSA_PB_STATUS_PAUSED, "paused");
         BT_A2DP_ALSA_PB_CASE_RETURN_STR(BT_A2DP_ALSA_PB_STATUS_STOPED, "stopped");
         default: return "unknown";
    }
}

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
static VOID bt_a2dp_alsa_pb_dump_pb_stat(VOID)
{
    ALSA_DBG_NORMAL("------------------playback state------------");
    ALSA_DBG_NORMAL("sample rate  : %u", s_bt_alsa_pb_dbg_cb.pb_sta.sample_rate);
    ALSA_DBG_NORMAL("written frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.in_frm);
    ALSA_DBG_NORMAL("failed frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.fail_frm);
    ALSA_DBG_NORMAL("start tm     : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("last tm      : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("max write time: %llu (us)", s_bt_alsa_pb_dbg_cb.pb_sta.max_in_us);
    ALSA_DBG_NORMAL("--------------------------------------------");
    ALSA_DBG_NORMAL("play time: %llu (ms)", s_bt_alsa_pb_dbg_cb.pb_sta.sample_rate==0?0:
        s_bt_alsa_pb_dbg_cb.pb_sta.in_frm * 1000LL /s_bt_alsa_pb_dbg_cb.pb_sta.sample_rate);
    ALSA_DBG_NORMAL("write time: %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm));
    ALSA_DBG_NORMAL("============================================");
}

static VOID bt_a2dp_alsa_pb_dump_alsa_stat(CHAR *title)
{
    ALSA_DBG_NORMAL("------------------alsa state(%s)---------------", title);
    ALSA_DBG_NORMAL("sample rate  : %u", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_NORMAL("bit depth    : %u", s_bt_alsa_pb_dbg_cb.dsp_sta.bitdepth);
    ALSA_DBG_NORMAL("written frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm);
    ALSA_DBG_NORMAL("failed frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.fail_frm);
    ALSA_DBG_NORMAL("drop frames  : %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.drop_frm);
    ALSA_DBG_NORMAL("start in tm  : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("last in tm   : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("max write time: %llu (us)", s_bt_alsa_pb_dbg_cb.dsp_sta.max_in_us);
    ALSA_DBG_NORMAL("max write interval: %llu (us)", s_bt_alsa_pb_dbg_cb.dsp_sta.max_write_interval);
    ALSA_DBG_NORMAL("--------------------------------------------");
    ALSA_DBG_NORMAL("play time: %llu (ms)", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate==0?0:
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm * 1000LL / s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_NORMAL("write time: %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm));
    ALSA_DBG_NORMAL("--------------------------------------------");
    ALSA_DBG_NORMAL("start frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.start_frm);
    ALSA_DBG_NORMAL("start tm    : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("start play time: %llu (ms)", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate==0?0:
        (UINT64)(s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm+s_bt_alsa_pb_dbg_cb.dsp_sta.start_frm) * 1000LL /
            s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_NORMAL("start write time: %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm));
    ALSA_DBG_NORMAL("------------------playback stat--------------------------");
    ALSA_DBG_NORMAL("written frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.in_frm);
    ALSA_DBG_NORMAL("failed frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.fail_frm);
    ALSA_DBG_NORMAL("start tm     : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("last tm      : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_nsec/1000);
    ALSA_DBG_NORMAL("max write interval: %llu (us)",
        s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval);
    ALSA_DBG_NORMAL("write diff   : %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm));
    ALSA_DBG_NORMAL("============================================");
}

static VOID bt_a2dp_alsa_pb_alsa_stat_check(VOID)
{
    UINT64 write_diff = BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm);
    UINT64 play_diff = s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate==0?0:
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm * 1000LL /s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate;

    if (play_diff < write_diff)
    {
        bt_a2dp_alsa_pb_dump_alsa_stat("check fail");
    }
}


static VOID bt_a2dp_alsa_pb_dump_stat_2_file(CHAR *title, INT32 to_file)
{
    ALSA_DBG_DUMP(to_file, "------------------alsa state(%s)---------------", title);
    ALSA_DBG_DUMP(to_file, "sample rate  : %u", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_DUMP(to_file, "bitdepth     : %u", s_bt_alsa_pb_dbg_cb.dsp_sta.bitdepth);
    ALSA_DBG_DUMP(to_file, "written frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm);
    ALSA_DBG_DUMP(to_file, "failed frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.fail_frm);
    ALSA_DBG_DUMP(to_file, "start in tm  : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm.tv_nsec/1000);
    ALSA_DBG_DUMP(to_file, "last in tm   : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm.tv_nsec/1000);
    ALSA_DBG_DUMP(to_file, "max write time: %llu (us)", s_bt_alsa_pb_dbg_cb.dsp_sta.max_in_us);
    ALSA_DBG_DUMP(to_file, "--------------------------------------------");
    ALSA_DBG_DUMP(to_file, "play time: %llu (ms)", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate==0?0:
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm * 1000LL / s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_DUMP(to_file, "write time: %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_in_tm));
    ALSA_DBG_DUMP(to_file, "--------------------------------------------");
    ALSA_DBG_DUMP(to_file, "start frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.dsp_sta.start_frm);
    ALSA_DBG_DUMP(to_file, "start tm    : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm.tv_nsec/1000);
    ALSA_DBG_DUMP(to_file, "start play time: %llu (ms)", s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate==0?0:
        (UINT64)(s_bt_alsa_pb_dbg_cb.dsp_sta.in_frm+s_bt_alsa_pb_dbg_cb.dsp_sta.start_frm) * 1000LL /
            s_bt_alsa_pb_dbg_cb.dsp_sta.sample_rate);
    ALSA_DBG_DUMP(to_file, "start write time: %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.dsp_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.dsp_sta.start_tm));
    ALSA_DBG_DUMP(to_file, "--------------------------------------------");
    ALSA_DBG_DUMP(to_file, "left buf in q: %u",
        s_bt_alsa_pb_cb.data_q->w_cnt - s_bt_alsa_pb_cb.data_q->r_cnt);
    ALSA_DBG_DUMP(to_file, "------------------playback stat--------------------------");
    ALSA_DBG_DUMP(to_file, "written frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.in_frm);
    ALSA_DBG_DUMP(to_file, "failed frames: %llu (frames)", s_bt_alsa_pb_dbg_cb.pb_sta.fail_frm);
    ALSA_DBG_DUMP(to_file, "start tm     : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm.tv_nsec/1000);
    ALSA_DBG_DUMP(to_file, "last tm      : %lld.%06d (s)",
        (UINT64)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_sec,
        (UINT32)s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm.tv_nsec/1000);
    ALSA_DBG_DUMP(to_file, "max write interval: %llu (us)",
        s_bt_alsa_pb_dbg_cb.pb_sta.max_write_interval);
    ALSA_DBG_DUMP(to_file, "write diff   : %ld (ms)",
        BT_A2DP_ALSA_PB_DIFF_TM_MS(s_bt_alsa_pb_dbg_cb.pb_sta.last_in_tm,
        s_bt_alsa_pb_dbg_cb.pb_sta.start_in_tm));
    ALSA_DBG_DUMP(to_file, "============================================");
}
#endif


static BT_A2DP_GET_PLAYER_METHODS get_player_methods = {
    .get_player = bt_a2dp_alsa_get_player,
};

__attribute__ ((visibility ("default")))
EXPORT_SYMBOL BT_A2DP_PLAYER_MODULE PLAYER_MODULE_INFO_SYM = {
        .version_major = 1,
        .version_minor = 0, /*player version*/
        .methods = &get_player_methods,
};

/******************************************************************************
 */

