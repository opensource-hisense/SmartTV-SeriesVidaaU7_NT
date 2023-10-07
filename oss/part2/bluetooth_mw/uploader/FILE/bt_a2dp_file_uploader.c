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

/* FILE NAME:  bt_a2dp_file_uploader.c
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
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <time.h>
#include <asoundlib.h>

#include "bt_a2dp_file_uploader.h"
#include "bt_mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    BT_FILE_UPL_LOG_ERROR = 0,
    BT_FILE_UPL_LOG_WARNING,
    BT_FILE_UPL_LOG_NORMAL,
    BT_FILE_UPL_LOG_NOTICE,
    BT_FILE_UPL_LOG_INFO,
    BT_FILE_UPL_LOG_MINOR = 5,
    BT_FILE_UPL_LOG_MAX,
}BT_FILE_UPL_LOG_T;

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_FILE_CAP_FILE_NAME "/data/sda1/music/input_48000_20s.pcm"

#define BT_FILE_UPL_DEBUG

#define BT_FILE_CAP_QUEUE_SIZE          (4*8*2)
#define BT_FILE_CAP_QUEUE_ITEM_SIZE     (4*1024)

#define BT_FILE_CAP_CHUNK_BYTES         (3072+1024)
#define BT_FILE_PUSH_BYTES              (4096)

#define BT_FILE_UPL_LOG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_FILE_UPL_LOG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_FILE_UPL_LOG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_FILE_UPL_LOG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_FILE_UPL_LOG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_FILE_UPL_LOG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_UPL, s, ## __VA_ARGS__)


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    int inited;
    int mute; /* 0-not mute, 1-mute */
    FILE *cap_file;
    int need_quit;
    int suspend_state; /* 0-no suspend, 1-suspending, 2-suspended */
    int drop_ms; /* after starting, drop some data */
    int sample_rate;
    int channels;
    pthread_t cap_thread;
    pthread_cond_t pause_cond;
    pthread_mutex_t pause_mutex;
    pthread_mutex_t resume_mutex;
} BT_FILE_UPL_CTX;


/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static INT32 bt_a2dp_file_uploader_init(INT32 freq, INT32 channel);
static INT32 bt_a2dp_file_uploader_deinit(VOID);
static INT32 bt_a2dp_file_uploader_start(INT32 delay_ms);
static INT32 bt_a2dp_file_uploader_stop(VOID);
static INT32 bt_a2dp_file_uploader_pause(VOID *param);
static INT32 bt_a2dp_file_uploader_resume(VOID *param);
static INT32 bt_a2dp_file_uploader_mute(BOOL mute);

/* STATIC VARIABLE DECLARATIONS
 */
static BT_A2DP_UPLOADER s_bt_a2dp_file_uploader =
{
    "file_upl",
    bt_a2dp_file_uploader_init,
    bt_a2dp_file_uploader_start,
    bt_a2dp_file_uploader_stop,
    bt_a2dp_file_uploader_deinit,
    bt_a2dp_file_uploader_pause,
    bt_a2dp_file_uploader_resume,
    bt_a2dp_file_uploader_mute,
};

static BT_A2DP_FILE_OUTPUT_CALLBACK s_bt_a2dp_file_output_cb = NULL;

static BT_FILE_UPL_CTX s_bt_a2dp_file_upl_ctx = {0};


#ifdef BT_FILE_UPL_DEBUG
static UINT32 s_bt_a2dp_file_wait_us = 0; /* if wait some time after open dev */

static UINT32 s_bt_a2dp_read_frames = 0; /* if it is 0, default is chunk size */
static UINT32 s_bt_a2dp_dump_enable = 0; /* enable pcm dump */
static FILE *s_bt_a2dp_outputPcmSampleFile = NULL;
static char s_bt_a2dp_outputFilename[128] = "/tmp/src_sample.pcm";

static UINT32 s_bt_a2dp_file_send_wait_us = 0;

static UINT32 s_bt_a2dp_file_drop_us = 0;
#endif

static UINT32 s_bt_a2dp_push_bytes = BT_FILE_PUSH_BYTES; /* the data length put in data queue */


#define bt_a2dp_file_timersub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_usec += 1000000; \
    } \
} while (0)

#define timermsub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
        if ((result)->tv_nsec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_nsec += 1000000000L; \
        } \
    } while (0)

/* EXPORTED SUBPROGRAM BODIES
 */

EXPORT_SYMBOL BT_A2DP_UPLOADER* bt_a2dp_file_get_uploader(VOID)
{
    return &s_bt_a2dp_file_uploader;
}

EXPORT_SYMBOL VOID bt_a2dp_file_register_output_callback(BT_A2DP_FILE_OUTPUT_CALLBACK output_cb)
{
    s_bt_a2dp_file_output_cb = output_cb;

    return;
}

EXPORT_SYMBOL VOID bt_a2dp_file_uploader_set_debug_flag(INT32 flag, UINT32 value)
{
    switch (flag)
    {
#ifdef BT_FILE_UPL_DEBUG
        case 0:
            s_bt_a2dp_file_wait_us = value;
            break;
        case 1:
            s_bt_a2dp_read_frames = value;
            break;
        case 2:
            s_bt_a2dp_dump_enable = value;
            break;
        case 3:
            s_bt_a2dp_push_bytes = value > BT_FILE_PUSH_BYTES ? BT_FILE_PUSH_BYTES : value;
            break;
        case 4:
            s_bt_a2dp_file_send_wait_us = value;
            break;
        case 5:
            s_bt_a2dp_file_drop_us = value;
            break;
#endif
        default:
            break;
    }

    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

static FILE* bt_a2dp_file_openPcmFile(const CHAR *pPcmFile)
{
    return fopen(pPcmFile, "r");
}

static INT32 bt_a2dp_file_closePcmFile(FILE *pPcmFile)
{
    return fclose(pPcmFile);
}



static void* bt_a2dp_file_cap_handler(VOID * args)
{
    INT32 i4_ret = -1;
    struct sched_param param;
    INT32 policy;
    char rd_buf[BT_FILE_CAP_CHUNK_BYTES];
    int rd_cnt = 0;

#ifdef BT_FILE_UPL_DEBUG
    struct timeval tv, tv1, diff;
#endif
    BT_FILE_UPL_CTX *upl_ctx = (BT_FILE_UPL_CTX*)args;

    i4_ret = pthread_getschedparam(pthread_self(), &policy, &param);
    if (0 != i4_ret)
    {
        BT_FILE_UPL_LOG_ERROR("pthread_getschedparam i4_ret:%d", i4_ret);
    }
    BT_FILE_UPL_LOG_NORMAL("priority:%ld, Policy:%ld, cap_handle=%p",
        (long)param.sched_priority, (long)policy, upl_ctx->cap_file);

    prctl(PR_SET_NAME, "alsa_get_pcm_handle", 0, 0, 0);

    while (!upl_ctx->need_quit)
    {
        pthread_mutex_lock(&upl_ctx->pause_mutex);
        if (1 == upl_ctx->suspend_state)
        {
            upl_ctx->suspend_state = 2; /* suspended */
            pthread_cond_signal(&upl_ctx->pause_cond);
            pthread_mutex_unlock(&upl_ctx->pause_mutex);

            BT_FILE_UPL_LOG_NORMAL("wait resume_mutex");
            pthread_mutex_lock(&upl_ctx->resume_mutex);
            pthread_mutex_unlock(&upl_ctx->resume_mutex);
            BT_FILE_UPL_LOG_NORMAL("wait resume_mutex done");
            continue;
        }
        pthread_mutex_unlock(&upl_ctx->pause_mutex);

        rd_cnt = fread(rd_buf, 1, BT_FILE_CAP_CHUNK_BYTES, upl_ctx->cap_file);
        BT_FILE_UPL_LOG_MINOR("read cap dev success read bytes=%d", rd_cnt);
        if (rd_cnt < BT_FILE_CAP_CHUNK_BYTES)
        {
            //BT_FILE_UPL_LOG_ERROR("read cap dev fail %d", err);
            fseek(upl_ctx->cap_file, 0L, SEEK_SET);
            continue;
        }

        if (s_bt_a2dp_file_upl_ctx.mute)
        {
            memset(rd_buf, 0, rd_cnt);
        }

        if (NULL != s_bt_a2dp_file_output_cb)
        {
            s_bt_a2dp_file_output_cb(rd_buf, rd_cnt);
        }
        else
        {
            BT_FILE_UPL_LOG_ERROR("output is NULL");
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

static INT32 bt_a2dp_file_startCapThread(BT_FILE_UPL_CTX *upl_ctx)
{
    INT32 i4_ret = 0;
    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 5;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_FILE_UPL_LOG_ERROR("pthread_attr_init i4_ret:%d", i4_ret);
        return -1;
    }
    i4_ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (0 != i4_ret)
    {
        BT_FILE_UPL_LOG_ERROR("pthread_attr_setschedpolicy i4_ret:%d", i4_ret);
    }
    i4_ret = pthread_attr_setschedparam(&attr, &param);
    if (0 != i4_ret)
    {
        BT_FILE_UPL_LOG_ERROR("pthread_attr_setschedparam i4_ret:%d", i4_ret);
    }
    i4_ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (0 != i4_ret)
    {
        BT_FILE_UPL_LOG_ERROR("pthread_attr_setinheritsched i4_ret:%d", i4_ret);
    }

    if (0 != (i4_ret = pthread_create(&upl_ctx->cap_thread,
                                      &attr,
                                      bt_a2dp_file_cap_handler,
                                      (void*)upl_ctx)))
    {
        BT_FILE_UPL_LOG_ERROR("pthread_create i4_ret:%d", i4_ret);
        assert(0);
    }

    pthread_attr_destroy(&attr);
    return 0;
}



static INT32 bt_a2dp_file_uploader_init(INT32 freq, INT32 channel)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter ---");
    if (s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("inited");
        return 0;
    }

    if (NULL != s_bt_a2dp_file_upl_ctx.cap_file)
    {
        BT_FILE_UPL_LOG_ERROR("alsa dev %s is openned", BT_FILE_CAP_FILE_NAME);
        return 0;
    }

    if ((s_bt_a2dp_file_upl_ctx.cap_file = bt_a2dp_file_openPcmFile(BT_FILE_CAP_FILE_NAME)) == NULL)
    {
        BT_FILE_UPL_LOG_ERROR("open alsa dev %s fail", BT_FILE_CAP_FILE_NAME);
        return -1;
    }
    BT_FILE_UPL_LOG_NORMAL("open alsa dev %s success", BT_FILE_CAP_FILE_NAME);

#ifdef BT_FILE_UPL_DEBUG
    usleep(s_bt_a2dp_file_wait_us);

    BT_FILE_UPL_LOG_ERROR("open alsa then wait %u ms", s_bt_a2dp_file_wait_us);
#endif

#ifdef BT_FILE_UPL_DEBUG
    if (s_bt_a2dp_dump_enable)
    {
        static int file_no = 0;
        char file_name[128];
        sprintf(file_name, "%s%d", s_bt_a2dp_outputFilename, file_no);
        s_bt_a2dp_outputPcmSampleFile = fopen(file_name, "wb+");

        BT_FILE_UPL_LOG_NORMAL("open dump file %s fd %p", file_name, s_bt_a2dp_outputPcmSampleFile);
    }
#endif
    s_bt_a2dp_file_upl_ctx.need_quit = 0;
    s_bt_a2dp_file_upl_ctx.suspend_state = 0;

    pthread_cond_init(&s_bt_a2dp_file_upl_ctx.pause_cond, NULL);
    pthread_mutex_init(&s_bt_a2dp_file_upl_ctx.pause_mutex, NULL);
    pthread_mutex_init(&s_bt_a2dp_file_upl_ctx.resume_mutex, NULL);

    s_bt_a2dp_file_upl_ctx.sample_rate = freq;
    s_bt_a2dp_file_upl_ctx.channels = channel;
    s_bt_a2dp_file_upl_ctx.inited = 1;

    return 0;
}

static INT32 bt_a2dp_file_uploader_deinit(VOID)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("uninited");
        return 0;
    }

    if (NULL != s_bt_a2dp_file_upl_ctx.cap_file)
    {
        pthread_cond_destroy(&s_bt_a2dp_file_upl_ctx.pause_cond);
        pthread_mutex_destroy(&s_bt_a2dp_file_upl_ctx.pause_mutex);
        pthread_mutex_destroy(&s_bt_a2dp_file_upl_ctx.resume_mutex);

        bt_a2dp_file_closePcmFile(s_bt_a2dp_file_upl_ctx.cap_file);
        s_bt_a2dp_file_upl_ctx.cap_file = NULL;
    }
#ifdef BT_FILE_UPL_DEBUG
    if (s_bt_a2dp_outputPcmSampleFile)
    {
        fclose(s_bt_a2dp_outputPcmSampleFile);
    }
    s_bt_a2dp_outputPcmSampleFile = NULL;
#endif
    s_bt_a2dp_file_upl_ctx.inited = 0;
    return 0;
}

static INT32 bt_a2dp_file_uploader_pause(VOID *param)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("uninited");
        return 0;
    }

    pthread_mutex_lock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
    if (s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
        BT_FILE_UPL_LOG_NORMAL("is pausing or paused:%d",
            s_bt_a2dp_file_upl_ctx.suspend_state);
        return 0;
    }

    s_bt_a2dp_file_upl_ctx.suspend_state = 1;

    BT_FILE_UPL_LOG_NORMAL("lock resume_mutex");
    pthread_mutex_lock(&s_bt_a2dp_file_upl_ctx.resume_mutex);
    while(1 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        pthread_cond_wait(&s_bt_a2dp_file_upl_ctx.pause_cond, &s_bt_a2dp_file_upl_ctx.pause_mutex);
    }
    pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
    BT_FILE_UPL_LOG_NORMAL("--- exit ---");

    return 0;
}

static INT32 bt_a2dp_file_uploader_resume(VOID *param)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter suspend_state=%d---",
        s_bt_a2dp_file_upl_ctx.suspend_state);
    if (0 == s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("uninited");
        return 0;
    }

    pthread_mutex_lock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
    if (0 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
        BT_FILE_UPL_LOG_NORMAL("is playing:%d",
            s_bt_a2dp_file_upl_ctx.suspend_state);
        return 0;
    }

    if (1 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        s_bt_a2dp_file_upl_ctx.suspend_state = 0; /* suspended */
        pthread_cond_signal(&s_bt_a2dp_file_upl_ctx.pause_cond);
        BT_FILE_UPL_LOG_NORMAL("unlock resume_mutex");
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.resume_mutex);
    }

    if (2 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        BT_FILE_UPL_LOG_NORMAL("unlock resume_mutex");
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.resume_mutex);
    }

    s_bt_a2dp_file_upl_ctx.suspend_state = 0;
    pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
    BT_FILE_UPL_LOG_NORMAL("--- exit ---");

    return 0;
}

static INT32 bt_a2dp_file_uploader_mute(BOOL mute)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter mute=%d---", mute);
    s_bt_a2dp_file_upl_ctx.mute = mute;
    BT_FILE_UPL_LOG_NORMAL("--- exit ---");

    return 0;
}



static INT32 bt_a2dp_file_uploader_start(INT32 delay_ms)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("uninited");
        return 0;
    }

#ifdef BT_FILE_UPL_DEBUG
    s_bt_a2dp_file_upl_ctx.drop_ms = s_bt_a2dp_file_drop_us == 0 ? delay_ms : s_bt_a2dp_file_drop_us / 1000;
#else
    s_bt_a2dp_file_upl_ctx.drop_ms = delay_ms;
#endif

    if (bt_a2dp_file_startCapThread(&s_bt_a2dp_file_upl_ctx) < 0)
    {
        return -1;
    }

    BT_FILE_UPL_LOG_NORMAL("--- exit ---");
    return 0;
}

static INT32 bt_a2dp_file_uploader_stop(VOID)
{
    BT_FILE_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_file_upl_ctx.inited)
    {
        BT_FILE_UPL_LOG_ERROR("uninited");
        return 0;
    }

    pthread_mutex_lock(&s_bt_a2dp_file_upl_ctx.pause_mutex);
    if (1 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        s_bt_a2dp_file_upl_ctx.suspend_state = 0; /* suspended */
        pthread_cond_signal(&s_bt_a2dp_file_upl_ctx.pause_cond);
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.resume_mutex);
    }

    if (2 == s_bt_a2dp_file_upl_ctx.suspend_state)
    {
        pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.resume_mutex);
    }
    pthread_mutex_unlock(&s_bt_a2dp_file_upl_ctx.pause_mutex);

    s_bt_a2dp_file_upl_ctx.need_quit = 1;
    s_bt_a2dp_file_upl_ctx.suspend_state = 0;

    BT_FILE_UPL_LOG_NORMAL("--- wait cap thread ---");
    pthread_join(s_bt_a2dp_file_upl_ctx.cap_thread, NULL);
    s_bt_a2dp_file_upl_ctx.cap_thread = -1;
    BT_FILE_UPL_LOG_NORMAL("--- cap thread exit---");

    s_bt_a2dp_file_upl_ctx.need_quit = 0;
    BT_FILE_UPL_LOG_NORMAL("--- exit ---");
    return 0;
}
