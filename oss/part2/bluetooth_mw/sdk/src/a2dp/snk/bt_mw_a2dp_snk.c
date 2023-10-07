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

/* FILE NAME:  bt_mw_a2dp_snk.c
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
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <bt_audio_track.h>
#include "bt_mw_a2dp_snk.h"
#include "c_mw_config.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_message_queue.h"

#include <dlfcn.h>

extern bool g_start_record;
extern UINT32 g_first_time;
bool g_mw_start_record = false;
UINT32 g_mw_first_time;

/* NAMING CONSTANT DECLARATIONS
 */
#define BT_A2DP_PLAYBACK_RE_PLAY_TIMEOUT_MS (10000)

#define BT_A2DP_PLAYER_MAX                  (3)

#define BT_A2DP_SINK_DUMP_PCM_FILE BT_TMP_PATH"/sink_pcm_dump.pcm"

#define BT_A2DP_SINK_STACK_DELAY_VALUE (20)  // 20ms

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_A2DP_SINK_LOCK() do{                           \
    if(g_bt_mw_a2dp_snk_cb.inited)                           \
        pthread_mutex_lock(&g_bt_mw_a2dp_snk_cb.lock);       \
    } while(0)

#define BT_MW_A2DP_SINK_UNLOCK() do{                         \
    if(g_bt_mw_a2dp_snk_cb.inited)                           \
        pthread_mutex_unlock(&g_bt_mw_a2dp_snk_cb.lock);     \
    } while(0)

#define BT_MW_A2DP_SINK_CHECK_INITED(ret)    do {                \
        if (FALSE == g_bt_mw_a2dp_snk_cb.inited)                 \
        {                                                        \
            BT_DBG_ERROR(BT_DEBUG_A2DP, "a2dp sink not init");   \
            return ret;                                          \
        }                                                        \
    }while(0)

typedef enum
{
    BT_MW_A2DP_SINK_PLAYER_STOPPED,
    BT_MW_A2DP_SINK_PLAYER_STOPPING,
    BT_MW_A2DP_SINK_PLAYER_STARTED,
    BT_MW_A2DP_SINK_PLAYER_STARTING,
    BT_MW_A2DP_SINK_PLAYER_PENDING, /* sample rate is not update, so pending it */
} BT_MW_A2DP_SINK_PLAYER_STATUS;
/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    BOOL inited;

    INT32 ChannelCnt;
    INT32 SampleRate;
    INT32 BitDepth;

    BOOL ratio_set; /* true: bt/wifi ratio is set, false: default */

    BT_MW_A2DP_SINK_PLAYER_STATUS status;
    struct timespec stop_ts; /* player stop timestamp */

    BT_A2DP_PLAYER *cur_player;
    CHAR active_addr[MAX_BDADDR_LEN];
    pthread_mutex_t lock;
} BT_MW_A2DP_SNK_CB;

typedef struct
{
    VOID *dlhandle;
    BT_A2DP_PLAYER player;
} BT_MW_A2DP_SNK_PLAYER;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
 
//static CHAR* bt_mw_a2dp_sink_get_status_str(BT_MW_A2DP_SINK_PLAYER_STATUS status);

/* STATIC VARIABLE DECLARATIONS
 */
static BT_MW_A2DP_SNK_CB g_bt_mw_a2dp_snk_cb = {0};

FILE *s_bt_mw_a2dp_sink_pcm_fp = NULL;
char s_bt_mw_a2dp_sink_pcm_file[128];

pthread_mutex_t s_bt_mw_a2dp_sink_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_bt_mw_a2dp_sink_signal = PTHREAD_COND_INITIALIZER;


/* EXPORTED SUBPROGRAM BODIES
 */

/**
 * FUNCTION NAME: bt_mw_a2dp_sink_adjust_buffer_time
 * PURPOSE:
 *      The function is used for adjust buffer time when send data to playback
 * INPUT:
 *      buffer time
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
INT32 bt_mw_a2dp_sink_adjust_buffer_time(UINT32 buffer_time)
{
    BT_MW_A2DP_SINK_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "buffer_time=%u", buffer_time);
    if (g_bt_mw_a2dp_snk_cb.cur_player &&
        g_bt_mw_a2dp_snk_cb.cur_player->adjust_buf_time)
    {
        g_bt_mw_a2dp_snk_cb.cur_player->adjust_buf_time(buffer_time);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "callback is NULL");
    }
    return BT_SUCCESS;
}

INT32 bt_mw_a2dp_sink_init(VOID)
{
    pthread_mutexattr_t attr;
    if (TRUE == g_bt_mw_a2dp_snk_cb.inited)
    {
        return BT_SUCCESS;
    }
    memset(&g_bt_mw_a2dp_snk_cb, 0, sizeof(g_bt_mw_a2dp_snk_cb));

    g_mw_start_record = false;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&g_bt_mw_a2dp_snk_cb.lock, &attr);

    g_bt_mw_a2dp_snk_cb.inited = TRUE;
    return BT_SUCCESS;
}

INT32 bt_mw_a2dp_sink_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "inited=%d", g_bt_mw_a2dp_snk_cb.inited);
    if (FALSE == g_bt_mw_a2dp_snk_cb.inited)
    {
        return BT_SUCCESS;
    }

    pthread_mutex_destroy(&g_bt_mw_a2dp_snk_cb.lock);
    memset(&g_bt_mw_a2dp_snk_cb, 0, sizeof(g_bt_mw_a2dp_snk_cb));

    return BT_SUCCESS;
}

VOID bt_mw_a2dp_sink_dump_pcm_start(char *dump_file_name)
{
    if (s_bt_mw_a2dp_sink_pcm_fp != NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "dump is started, skip it(%s)",
            dump_file_name==NULL?"NULL":dump_file_name);
        return;
    }

    if (dump_file_name == NULL)
    {
        strncpy(s_bt_mw_a2dp_sink_pcm_file, BT_A2DP_SINK_DUMP_PCM_FILE, 127);
    }
    else
    {
        if (strlen(dump_file_name) == 0)
        {
            strncpy(s_bt_mw_a2dp_sink_pcm_file, BT_A2DP_SINK_DUMP_PCM_FILE, 127);
        }
        else
        {
            strncpy(s_bt_mw_a2dp_sink_pcm_file, dump_file_name, 127);
        }
    }
#if 0
    BT_MW_A2DP_SINK_LOCK();
    strncat(s_bt_mw_a2dp_sink_pcm_file, g_bt_mw_a2dp_snk_cb.active_addr, 127);
    BT_MW_A2DP_SINK_UNLOCK();
#endif
    s_bt_mw_a2dp_sink_pcm_file[127] = 0;

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "dump pcm file:%s", s_bt_mw_a2dp_sink_pcm_file);

    s_bt_mw_a2dp_sink_pcm_fp = fopen(s_bt_mw_a2dp_sink_pcm_file, "wb+");
    if (s_bt_mw_a2dp_sink_pcm_fp == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "open(%s) fail", s_bt_mw_a2dp_sink_pcm_file);
        return;
    }

    return;
}

VOID bt_mw_a2dp_sink_dump_pcm_stop(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (s_bt_mw_a2dp_sink_pcm_fp)
    {
        (void)fclose(s_bt_mw_a2dp_sink_pcm_fp);
        s_bt_mw_a2dp_sink_pcm_fp = NULL;
    }
    return;
}

INT32 bt_mw_a2dp_sink_get_stack_delay(VOID)
{
    INT32 value = 0;
    //value = g_mw_first_time - g_first_time;
    value = BT_A2DP_SINK_STACK_DELAY_VALUE;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "delay value %d ms", value);

    return value;
}

/* LOCAL SUBPROGRAM BODIES
 */
#if 0
static CHAR* bt_mw_a2dp_sink_get_status_str(BT_MW_A2DP_SINK_PLAYER_STATUS status)
{
     switch(status)
     {
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_SINK_PLAYER_STOPPED, "stopped");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_SINK_PLAYER_STOPPING, "stopping");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_SINK_PLAYER_STARTED, "started");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_SINK_PLAYER_STARTING, "starting");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_SINK_PLAYER_PENDING, "start pending");

         default: return "unknown";
    }
}
#endif
