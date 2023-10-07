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

/* FILE NAME:  bt_a2dp_mtal_uploader.c
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

#include "mtauddec.h"
#include "mtcommon.h"
#include "u_bt_mw_a2dp.h"

#include "bt_a2dp_mtal_uploader.h"
#include "queue.h"
#include "bt_mw_log.h"
#include "mtk_audio.h"

#define BT_QUEUE_NUM    4*8

static sem_t g_bt_pcm_lock;
#define SUPPORT_A2DP_LATENCY_MEASUREMENT 0
#if !SUPPORT_A2DP_LATENCY_MEASUREMENT
typedef struct _BT_PCM_MSG_T
{
    UINT32 m_len;
    CHAR *mp_buff;
}BT_PCM_MSG_T;
#endif

typedef struct _BT_PCM_QUEUE_T
{
    UINT8 currLen;
    UINT8 maxLen;
    UINT8 head;
    BT_PCM_MSG_T* msg[BT_QUEUE_NUM];
}BT_PCM_QUEUE_T;

static BT_PCM_QUEUE_T t_bt_pcm_queue = {0};
static BOOL g_bt_pcm_lock_init = FALSE;
static BOOL bt_pcm_handle_is_exist = FALSE;
static BOOL bt_get_pcm_handle_is_exist = FALSE;
static BOOL bt_kill_pcm_handle = FALSE;
static BOOL g_bt_mute = FALSE;
INT32 g_pcm_remained_buf_size = 0;
UINT32 g_last_tx_time = 0;

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    BT_MTAL_UPL_LOG_ERROR = 0,
    BT_MTAL_UPL_LOG_WARNING,
    BT_MTAL_UPL_LOG_NORMAL,
    BT_MTAL_UPL_LOG_NOTICE,
    BT_MTAL_UPL_LOG_INFO,
    BT_MTAL_UPL_LOG_MINOR = 5,
    BT_MTAL_UPL_LOG_MAX,
}BT_MTAL_UPL_LOG_T;

/* MACRO FUNCTION DECLARATIONS
 */

//#define BT_MTAL_UPL_DEBUG

#define BT_MTAL_UPL_LOG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_UPL, s, ## __VA_ARGS__)
#define BT_MTAL_UPL_LOG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_UPL, s, ## __VA_ARGS__)
#define BT_MTAL_UPL_LOG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_UPL, s, ## __VA_ARGS__)
#define BT_MTAL_UPL_LOG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_UPL, s, ## __VA_ARGS__)
#define BT_MTAL_UPL_LOG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_UPL, s, ## __VA_ARGS__)
#define BT_MTAL_UPL_LOG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define MW_AUD_UPLOAD_BUF_SIZE_IN_BYTE         (4*1024)
#define MW_AUD_UPLOAD_BUF_CNT                  (1)
#define MW_AUD_UPLOAD_BUF_TOTAL_SIZE_IN_BYTE   (MW_AUD_UPLOAD_BUF_CNT*MW_AUD_UPLOAD_BUF_SIZE_IN_BYTE)
#define MW_32K_AUD_UPLOAD_BUF_TOTAL_SIZE_IN_BYTE   (MW_AUD_UPLOAD_BUF_TOTAL_SIZE_IN_BYTE*3/2)
#define MW_AUD_UPLOAD_BLOCK_BYTE 3072

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    int inited;
    BOOL a2dp_connected;
} BT_MTAL_UPL_CTX;

hw_device_t* device = NULL;
struct audio_hw_device *hw_dev = NULL;
struct audio_stream_out *stream = NULL;
static BOOL g_fg_audio_path_init = FALSE;
static pthread_mutex_t g_adev_lock = PTHREAD_MUTEX_INITIALIZER;

extern int adev_open_output_stream(struct audio_hw_device *dev,
                                       audio_io_handle_t handle,
                                       audio_devices_t devices,
                                       audio_output_flags_t flags,
                                       struct audio_config *config,
                                       struct audio_stream_out **stream_out,
                                       const char *address);
extern void adev_close_output_stream(struct audio_hw_device *dev,
                                         struct audio_stream_out *stream);


/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static INT32 bt_a2dp_mtal_uploader_init(INT32 freq, INT32 channel);
static INT32 bt_a2dp_mtal_uploader_deinit(VOID);
static INT32 bt_a2dp_mtal_uploader_start(INT32 delay_ms);
static INT32 bt_a2dp_mtal_uploader_stop(VOID);
static INT32 bt_a2dp_mtal_uploader_mute(BOOL mute);

/* STATIC VARIABLE DECLARATIONS
 */
static BT_A2DP_UPLOADER s_bt_a2dp_mtal_uploader =
{
    "mtal_uploader",
    bt_a2dp_mtal_uploader_init,
    bt_a2dp_mtal_uploader_start,
    bt_a2dp_mtal_uploader_stop,
    bt_a2dp_mtal_uploader_deinit,
    NULL,
    NULL,
    bt_a2dp_mtal_uploader_mute,
};

static BT_MTAL_UPL_CTX s_bt_a2dp_mtal_upl_ctx = {0};

#ifdef BT_MTAL_UPL_DEBUG
static UINT32 s_bt_a2dp_dump_enable = 1; /* enable pcm dump */
static FILE *s_bt_a2dp_outputPcmSampleFile = NULL;
static char s_bt_a2dp_outputFilename[128] = "/tmp/src_output_sample.pcm";
#endif

VOID init_audio_path(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("[A2DP] Enter+++");

    pthread_mutex_lock(&g_adev_lock);
    if (0 > adev_open(NULL, AUDIO_HARDWARE_INTERFACE, &device))
    {
        BT_MTAL_UPL_LOG_ERROR("open bt adev fail\n");
    }
    hw_dev = (struct audio_hw_device *)device;
    if (0 > hw_dev->open_output_stream(hw_dev, 0, 0, 0, 0, &stream, 0))
    {
        BT_MTAL_UPL_LOG_ERROR("open out stream fail\n");
    }
    g_fg_audio_path_init = TRUE;
    pthread_mutex_unlock(&g_adev_lock);
    BT_MTAL_UPL_LOG_NORMAL("[A2DP] Exit---");
}

VOID uninit_audio_path(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("[A2DP] Enter+++");

    pthread_mutex_lock(&g_adev_lock);
    if (NULL != stream)
    {
        stream->common.set_parameters(&stream->common, "closing=true");
        stream->common.standby(&stream->common);

        if (NULL != hw_dev)
        {
            hw_dev->close_output_stream(hw_dev, stream);
        }
        if (NULL != device)
        {
            device->close(device);
        }

        stream = NULL;
        g_fg_audio_path_init = FALSE;
    }
    pthread_mutex_unlock(&g_adev_lock);

    BT_MTAL_UPL_LOG_NORMAL("[A2DP] Exit---");
}

INT32 bt_a2dp_send_stream_data(const CHAR *data, INT32 len)
{
    INT32 i4_ret = 0;
    BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] send data length:%ld begin", (long)len);

    if (0 == pthread_mutex_trylock(&g_adev_lock))
    {
        if (g_fg_audio_path_init && (NULL != stream) && (NULL != stream->write))
        {
            i4_ret = stream->write(stream, data, len);
            BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] i4_ret:%ld", (long)i4_ret);
        }
        pthread_mutex_unlock(&g_adev_lock);

        BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] send data length:%ld end", (long)len);
    }
    else
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] audio path not init, drop it.");
    }
    return BT_SUCCESS;
}

BOOL bBtPCMQueueEmpty(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter %s\n", __FUNCTION__);
    return (t_bt_pcm_queue.currLen == 0)?TRUE:FALSE;
}

BOOL bBtPCMQueueFull(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter %s\n", __FUNCTION__);
    return (t_bt_pcm_queue.currLen == t_bt_pcm_queue.maxLen )?TRUE:FALSE;
}

INT32 i4InitBtPCMQueue(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    INT32 i = 0;
    t_bt_pcm_queue.currLen = 0;
    t_bt_pcm_queue.maxLen = BT_QUEUE_NUM;
    t_bt_pcm_queue.head = 0;
    while (i<BT_QUEUE_NUM)
    {
        if (NULL != t_bt_pcm_queue.msg[i])
        {
            if (NULL != t_bt_pcm_queue.msg[i]->mp_buff)
            {
                free(t_bt_pcm_queue.msg[i]->mp_buff);
                t_bt_pcm_queue.msg[i]->mp_buff = NULL;
            }
            free(t_bt_pcm_queue.msg[i]);
        }
        t_bt_pcm_queue.msg[i] = (BT_PCM_MSG_T*)malloc(sizeof(BT_PCM_MSG_T));
        if (NULL == t_bt_pcm_queue.msg[i])
        {
            BT_MTAL_UPL_LOG_ERROR("cannot malloc pcm msg\n");
            return -1;
        }
        t_bt_pcm_queue.msg[i]->mp_buff = (CHAR*)malloc(MW_32K_AUD_UPLOAD_BUF_TOTAL_SIZE_IN_BYTE);
        if (NULL == t_bt_pcm_queue.msg[i]->mp_buff)
        {
            BT_MTAL_UPL_LOG_ERROR("cannot malloc pcm queue buffer\n");
            return -1;
        }
        i++;
    }
    return 0;
}

INT32 i4UnInitBtPCMQueue(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    INT32 i = 0;
    while (i<BT_QUEUE_NUM)
    {
        if (NULL != t_bt_pcm_queue.msg[i] && NULL != t_bt_pcm_queue.msg[i]->mp_buff)
        {
            free(t_bt_pcm_queue.msg[i]->mp_buff);
            t_bt_pcm_queue.msg[i]->mp_buff = NULL;
            free(t_bt_pcm_queue.msg[i]);
            t_bt_pcm_queue.msg[i] = (BT_PCM_MSG_T*)NULL;
        }
        i++;
    }
    t_bt_pcm_queue.currLen = 0;
    t_bt_pcm_queue.maxLen = BT_QUEUE_NUM;
    t_bt_pcm_queue.head = 0;
    return 0;
}

INT32 i4BtPCMEnqueue(CHAR *pcm_buf, INT32 pcm_len)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");

    UINT8 u1InsPos;

    if ( NULL == pcm_buf )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm_buf is null.\n");
        return -1;
    }

    if (bBtPCMQueueFull())
    {
        BT_MTAL_UPL_LOG_ERROR("pcm bt queue is full.\n");
        return -1;
    }

    //maybe need mutex
    u1InsPos = (t_bt_pcm_queue.head + t_bt_pcm_queue.currLen) % t_bt_pcm_queue.maxLen;
    t_bt_pcm_queue.currLen++;
    if (NULL != t_bt_pcm_queue.msg[u1InsPos] && NULL != t_bt_pcm_queue.msg[u1InsPos]->mp_buff)
    {
        memcpy(t_bt_pcm_queue.msg[u1InsPos]->mp_buff, pcm_buf, pcm_len);
        t_bt_pcm_queue.msg[u1InsPos]->m_len = pcm_len;

        BT_MTAL_UPL_LOG_MINOR("<bt>msg enqueue, position: %d\n", u1InsPos);
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("<bt>pcm msg enqueue %d is null\n", u1InsPos);
        return -1;
    }
    BT_MTAL_UPL_LOG_MINOR("Exit\n");
    return 0;
}

BT_PCM_MSG_T* ptBtPCMDequeue(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    BT_PCM_MSG_T* pt_pcm_msg = NULL;

    if (bBtPCMQueueEmpty())
    {
        BT_MTAL_UPL_LOG_ERROR("bt queue is empty. \n" );
        return NULL;
    }

    BT_MTAL_UPL_LOG_MINOR("<bt>msg dequeue, position: %d \n", t_bt_pcm_queue.head );
    pt_pcm_msg = t_bt_pcm_queue.msg[t_bt_pcm_queue.head];
    //t_bt_pcm_queue.msg[t_bt_pcm_queue.head] = NULL;

    t_bt_pcm_queue.currLen--;
    t_bt_pcm_queue.head++;
    if (t_bt_pcm_queue.head >= t_bt_pcm_queue.maxLen )
    {
        t_bt_pcm_queue.head %= t_bt_pcm_queue.maxLen;
    }
    BT_MTAL_UPL_LOG_MINOR("Exit\n");

    return pt_pcm_msg;
}

VOID BtPCMEmptyQueue(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    t_bt_pcm_queue.currLen = 0;
    t_bt_pcm_queue.head = 0;
}

INT32 _bt_pcm_sem_lock(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    if ( g_bt_pcm_lock_init)
    {
        if ( 0 != sem_wait( &g_bt_pcm_lock))
        {
            BT_MTAL_UPL_LOG_NORMAL("sem_lock failed, errno: %ld.\n", (long)errno );
            return -1;
        }
    }

    return 0;
}

INT32 _bt_pcm_sem_unlock(VOID)
{
    BT_MTAL_UPL_LOG_MINOR("Enter\n");
    if ( g_bt_pcm_lock_init )
    {
        if ( 0 != sem_post( &g_bt_pcm_lock ))
        {
            BT_MTAL_UPL_LOG_NORMAL("sem_unlockfailed, errno: %ld.\n", (long)errno );
            return -1;
        }
    }

    return 0;
}

INT32 _bt_pcm_lock_uninit(VOID)
{
    FUNC_ENTRY;
    if ( g_bt_pcm_lock_init )
    {
        if ( 0 != sem_destroy( &g_bt_pcm_lock ))
        {
            BT_MTAL_UPL_LOG_NORMAL("sem_destroy failed, errno: %ld.\n", (long)errno );
            return -1;
        }
        g_bt_pcm_lock_init = FALSE;
    }
    else
    {
        return -1;
    }

    return 0;
}


INT32 _bt_pcm_lock_init(VOID)
{
    FUNC_ENTRY;
    INT32 res;

    if ( !g_bt_pcm_lock_init)
    {
        res = sem_init( &g_bt_pcm_lock, 0, 0 );
        if ( 0 != res )
        {
            BT_MTAL_UPL_LOG_NORMAL("sem_init failed, errno: %ld.\n", (long)errno );
            return -1;
        }
        g_bt_pcm_lock_init = TRUE;
    }
    else
    {
        return -1;
    }

    return 0;
}

INT32 _bt_pcm_queue_init(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = -1;

    i4_ret = _bt_pcm_lock_init();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm lock init error:%ld\n", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = i4InitBtPCMQueue();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm queue init error:%ld\n", (long)i4_ret);
        return i4_ret;
    }
    return i4_ret;
}

INT32 _bt_pcm_queue_uninit(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = -1;

    i4_ret = _bt_pcm_lock_uninit();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm lock uninit error:%ld\n", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = i4UnInitBtPCMQueue();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm queue uninit error:%ld\n", (long)i4_ret);
        return i4_ret;
    }
    return i4_ret;
}

static VOID* _bt_pcm_handler_thread(VOID * args)
{
    FUNC_ENTRY;
    //INT32 i4_ret = -1;
    BT_PCM_MSG_T *pt_pcm_msg = NULL;
    prctl(PR_SET_NAME, "bt_pcm_handle", 0, 0, 0);

    do
    {
        BT_MTAL_UPL_LOG_MINOR("wait for event.\n");
        _bt_pcm_sem_lock();
        BT_MTAL_UPL_LOG_MINOR("receive msg and go to handle it.\n");
        if (!s_bt_a2dp_mtal_upl_ctx.a2dp_connected)
        {
            BT_MTAL_UPL_LOG_MINOR("a2dp disconnected.\n");
            break;
        }
        pt_pcm_msg = ptBtPCMDequeue();
        if (NULL == pt_pcm_msg)
        {
            BT_MTAL_UPL_LOG_ERROR("no pcm data to dequeue\n");
            continue;
        }

        BT_MTAL_UPL_LOG_MINOR("pt_pcm_msg->m_len:%ld\n", (long)pt_pcm_msg->m_len);

        bt_a2dp_send_stream_data(pt_pcm_msg->mp_buff, pt_pcm_msg->m_len);
    }
    while (s_bt_a2dp_mtal_upl_ctx.a2dp_connected);
    bt_pcm_handle_is_exist = FALSE;
    BT_MTAL_UPL_LOG_NORMAL("bt_pcm_handle_is_exist:%d\n",bt_pcm_handle_is_exist);
    FUNC_EXIT;
    pthread_exit(NULL);
    return NULL;
}

static VOID* _get_pcm_handler_thread(VOID * args)
{
    FUNC_ENTRY;

    INT32 i4_ret = -1;
    CHAR mp_buff[MW_AUD_UPLOAD_BUF_TOTAL_SIZE_IN_BYTE];
    UINT32 rd_size = 0;
    UINT32 u4UploadDoneNfyrCnt = 0;
#ifdef BT_ALSA_UPL_DEBUG
    struct timeval tv, tv1, diff;
    struct timeval send_tv, send_tv1, send_diff;
#endif

    prctl(PR_SET_NAME, "get_pcm_handle", 0, 0, 0);

    while (s_bt_a2dp_mtal_upl_ctx.a2dp_connected)
    {
        if(bt_kill_pcm_handle)
        {
            BT_MTAL_UPL_LOG_WARNING("have kill pcm handle\n");
            break;
        }
        if (!s_bt_a2dp_mtal_upl_ctx.a2dp_connected)
        {
            BT_MTAL_UPL_LOG_MINOR("a2dp disconnected.\n");
            break;
        }

        if (0 == MTAUDDEC_RecordPCM_read((UPTR)mp_buff, MW_AUD_UPLOAD_BLOCK_BYTE, &rd_size, 1))
        {
            if (rd_size != MW_AUD_UPLOAD_BLOCK_BYTE)
            {
                BT_MTAL_UPL_LOG_MINOR("read pcm data size:%ld\n", (long)rd_size);
            }

            if (rd_size == 0)
            {
                continue;
            }

            u4UploadDoneNfyrCnt++;
            if (100 <= u4UploadDoneNfyrCnt)
            {
                BT_MTAL_UPL_LOG_MINOR("get PCM data from MT Audio done!\n");
                u4UploadDoneNfyrCnt = 0;
            }

            // if mute is required, send zero data to the peer Sink device
            if (TRUE == g_bt_mute)
            {
                memset(mp_buff, 0, rd_size);
            }

#ifdef BT_MTAL_UPL_DEBUG
            if (s_bt_a2dp_dump_enable && s_bt_a2dp_outputPcmSampleFile)
            {
                fwrite(mp_buff, 1, (size_t)rd_size, s_bt_a2dp_outputPcmSampleFile);
            }
#endif

            i4_ret = i4BtPCMEnqueue(mp_buff, rd_size);
            if ( 0 != i4_ret )
            {
                BT_MTAL_UPL_LOG_ERROR("pcm enqueue error:%ld\n", (long)i4_ret);
                //bt_pcm_sem_unlock();
            }
            else
            {
                _bt_pcm_sem_unlock();
            }

        }
        else
        {
            usleep(100);
            BT_MTAL_UPL_LOG_ERROR("MTAUDDEC_RecordPCM_read error\n");
        }
    }

    bt_kill_pcm_handle = FALSE;
    bt_get_pcm_handle_is_exist = FALSE;
    BT_MTAL_UPL_LOG_NORMAL("bt_kill_pcm_handle:%d\n", bt_kill_pcm_handle);
    BT_MTAL_UPL_LOG_NORMAL("get_pcm_handle_is_exist:%d\n", bt_get_pcm_handle_is_exist);
    FUNC_EXIT;
    pthread_exit(NULL);
    return NULL;
}

/*
This function create thread for deliver data.
it is called by init_PCM_upload.
*/
INT32 BT_PCM_CreateThread(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = 0;
    pthread_t h_msg_handler_thread;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_MTAL_UPL_LOG_ERROR("pthread_attr_init i4_ret:%ld\n", (long)i4_ret);
        return -1;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_msg_handler_thread,
                                          &attr,
                                          _bt_pcm_handler_thread,
                                          NULL)))
        {
            BT_MTAL_UPL_LOG_ERROR("pthread_create i4_ret:%ld\n", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("pthread_attr_setdetachstate i4_ret:%ld\n", (long)i4_ret);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

INT32 init_PCM_upload(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = -1;

    /* Initialize upload pcm queue */
    i4_ret = _bt_pcm_queue_init();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm queue init error:%ld\n", (long)i4_ret);
        return i4_ret;
    }
    /* create handle pcm data thread */
    if (!bt_pcm_handle_is_exist)
    {
        i4_ret = BT_PCM_CreateThread();
        if ( 0 != i4_ret )
        {
            BT_MTAL_UPL_LOG_ERROR("create handle pcm data thread error:%ld\n", (long)i4_ret);
            return i4_ret;
        }
        bt_pcm_handle_is_exist = TRUE;
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("handle pcm data thread has been created\n");
    }

    return i4_ret;
}

/*
This function uninit queue ;
it is called by btaudio_handle_a2dp_stream_close_cb
*/
INT32 uninit_PCM_upload(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = -1;

    _bt_pcm_sem_unlock();
    /*
    TODO: audio driver upload close
     */

    i4_ret = _bt_pcm_queue_uninit();
    if ( 0 != i4_ret )
    {
        BT_MTAL_UPL_LOG_ERROR("pcm queue uninit error:%ld\n", i4_ret);
        return i4_ret;
    }
    return i4_ret;
}

/*
This  is a callback function.

it is called when audio driver upload pcm data to mw.

Then it push data  to queue.

*/
INT32 BT_CB_GetPCM(void)
{
    FUNC_ENTRY;

    INT32 i4_ret = 0;
    pthread_t h_getpcm_handler_thread;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_MTAL_UPL_LOG_ERROR("pthread_attr_init i4_ret:%d\n", i4_ret);
        return -1;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_getpcm_handler_thread,
                                          &attr,
                                          _get_pcm_handler_thread,
                                          NULL)))
        {
            BT_MTAL_UPL_LOG_ERROR("pthread_create i4_ret:%d\n", i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("pthread_attr_setdetachstate i4_ret:%d\n", i4_ret);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

EXPORT_SYMBOL BT_A2DP_UPLOADER* bt_a2dp_mtal_get_uploader(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");

    return &s_bt_a2dp_mtal_uploader;
}

static INT32 bt_a2dp_mtal_uploader_init(INT32 freq, INT32 channel)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++ freq=%d, channel=%d", freq, channel);
    if (s_bt_a2dp_mtal_upl_ctx.inited)
    {
        BT_MTAL_UPL_LOG_ERROR("inited");
        return 0;
    }

    init_audio_path();

    if (MTR_OK != MTAL_Init())
    {
        BT_MTAL_UPL_LOG_ERROR("open mtal init fail");
        return -1;
    }

    if (MTR_OK != MTAUDDEC_Init())
    {
        BT_MTAL_UPL_LOG_ERROR("open mtauddec init fail");
        return -1;
    }

    init_PCM_upload();

#ifdef BT_MTAL_UPL_DEBUG
    if (s_bt_a2dp_dump_enable)
    {
        static int file_no = 0;
        char file_name[128];
        sprintf(file_name, "%s%d", s_bt_a2dp_outputFilename, file_no);
        s_bt_a2dp_outputPcmSampleFile = fopen(file_name, "wb+");

        BT_MTAL_UPL_LOG_NORMAL("open dump file %s fd %p", file_name, s_bt_a2dp_outputPcmSampleFile);
    }
#endif
    s_bt_a2dp_mtal_upl_ctx.inited = 1;
    s_bt_a2dp_mtal_upl_ctx.a2dp_connected = TRUE;

    return 0;
}

static INT32 bt_a2dp_mtal_uploader_deinit(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");
    if (0 == s_bt_a2dp_mtal_upl_ctx.inited)
    {
        BT_MTAL_UPL_LOG_ERROR("uninited");
        return 0;
    }

    uninit_PCM_upload();

    BT_MTAL_UPL_LOG_NORMAL("check PCM thread exit!\n");
    UINT32 u4Cnt = 30;
    UINT32 Cnt=0;
    while(bt_get_pcm_handle_is_exist)
    {
        u4Cnt++;
        Cnt++;
        if (30 <= u4Cnt)
        {
            BT_MTAL_UPL_LOG_NORMAL("wait for _get_pcm_handler_thread exit!\n");
            u4Cnt = 0;
        }

        if (Cnt > 200)
        {
            BT_MTAL_UPL_LOG_ERROR("wait for _get_pcm_handler_thread too long!\n");
            return -1;
        }
        usleep(30000); // wait for 30 ms
    }

#ifdef BT_MTAL_UPL_DEBUG
    if (s_bt_a2dp_outputPcmSampleFile)
    {
        fclose(s_bt_a2dp_outputPcmSampleFile);
    }
    s_bt_a2dp_outputPcmSampleFile = NULL;
#endif
    s_bt_a2dp_mtal_upl_ctx.inited = 0;
    g_bt_mute = FALSE;

    uninit_audio_path();

    return 0;
}

static INT32 bt_a2dp_mtal_uploader_start(INT32 delay_ms)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");
    if (0 == s_bt_a2dp_mtal_upl_ctx.inited)
    {
        BT_MTAL_UPL_LOG_ERROR("uninited");
        return 0;
    }

    if (MTR_OK == MTAUDDEC_RecordPCM_open(MTAUDDEC_ACAP_SOURCE_TYPE_PCM))  //uploadid =0, uploadtype=0(unused)
    {

        BT_MTAL_UPL_LOG_NORMAL("MTAUDDEC_RecordPCM_open(MTAUDDEC_ACAP_SOURCE_TYPE_PCM) successfully!\n");
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("MTAUDDEC_RecordPCM_open(MTAUDDEC_ACAP_SOURCE_TYPE_PCM) failed!\n");
        return -1 ;
    }

    bt_kill_pcm_handle = FALSE;

    if(!bt_get_pcm_handle_is_exist)
    {
        BT_CB_GetPCM();
        bt_get_pcm_handle_is_exist = TRUE;
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("handle pcm data thread has been created\n");
    }

    BT_MTAL_UPL_LOG_NORMAL("--- exit ---");
    return 0;
}

static INT32 bt_a2dp_mtal_uploader_stop(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");

    s_bt_a2dp_mtal_upl_ctx.a2dp_connected = FALSE;
    bt_kill_pcm_handle = TRUE;

    BT_MTAL_UPL_LOG_NORMAL("check PCM thread exit!\n");
    UINT32 u4Cnt = 30;
    UINT32 Cnt=0;
    while(bt_get_pcm_handle_is_exist)
    {
        u4Cnt++;
        Cnt++;
        if (30 <= u4Cnt)
        {
            BT_MTAL_UPL_LOG_NORMAL("wait for _get_pcm_handler_thread exit!\n");
            u4Cnt = 0;
        }

        if (Cnt > 200)
        {
            BT_MTAL_UPL_LOG_ERROR("wait for _get_pcm_handler_thread too long!\n");
            return -1;
        }
        usleep(30000); // wait for 30 ms
    }

    if (MTR_OK == MTAUDDEC_RecordPCM_close())
    {
        BT_MTAL_UPL_LOG_NORMAL("MTAUDDEC_RecordPCM_close successfully!\n");
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("MTAUDDEC_RecordPCM_close failed!\n");
    }

    BT_MTAL_UPL_LOG_NORMAL("PCM thread exit!\n");
    bt_kill_pcm_handle = FALSE;

    return 0;
}

static INT32 bt_a2dp_mtal_uploader_pause(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");

    if (0 == s_bt_a2dp_mtal_upl_ctx.inited)
    {
        BT_MTAL_UPL_LOG_ERROR("uninited");
        return 0;
    }
    pthread_mutex_lock(&g_adev_lock);
    if (g_fg_audio_path_init && NULL != stream)
    {
        stream->common.set_parameters(&stream->common, "A2dpSuspended=true");
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("g_fg_audio_path_init=%d, stream=%p",
                               g_fg_audio_path_init, stream);
    }
    pthread_mutex_unlock(&g_adev_lock);

    return 0;
}

static INT32 bt_a2dp_mtal_uploader_resume(VOID)
{
    BT_MTAL_UPL_LOG_NORMAL("Enter+++");

    if (0 == s_bt_a2dp_mtal_upl_ctx.inited)
    {
        BT_MTAL_UPL_LOG_ERROR("uninited");
        return 0;
    }
    pthread_mutex_lock(&g_adev_lock);
    if (g_fg_audio_path_init && NULL != stream)
    {
        stream->common.set_parameters(&stream->common, "A2dpSuspended=false");
    }
    else
    {
        BT_MTAL_UPL_LOG_ERROR("g_fg_audio_path_init=%d, stream=%p",
                               g_fg_audio_path_init, stream);
    }
    pthread_mutex_unlock(&g_adev_lock);

    return 0;
}


static INT32 bt_a2dp_mtal_uploader_mute(BOOL mute)
{
    BT_MTAL_UPL_LOG_NORMAL("mute:%s\n", mute ? "TRUE" : "FALSE");

    g_bt_mute = mute ? TRUE : FALSE;

    return 0;
}

static BT_A2DP_GET_UPLOADER_METHODS get_uploader_methods = {
    .get_uploader = bt_a2dp_mtal_get_uploader,
};

__attribute__ ((visibility ("default")))
EXPORT_SYMBOL BT_A2DP_UPLOADER_MODULE UPLOADER_MODULE_INFO_SYM = {
        .ver_major = 1,
        .ver_minor = 0, /*uploader version*/
        .methods = &get_uploader_methods
};
