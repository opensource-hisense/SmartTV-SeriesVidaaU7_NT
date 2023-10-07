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

/* FILE NAME:  bt_a2dp_stereo_playback.c
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

#include "u_bt_mw_common.h"
#include "bt_a2dp_stereo_playback.h"
#include "c_bt_mw_a2dp_snk.h"
#include "osi/include/log.h"
#include "bt_mw_log.h"
#include "bt_mw_common.h"
#include <asoundlib.h>
#include "stereo_app.h"
#include <malloc.h>

/* NAMING CONSTANT DECLARATIONS
 */
//#define MTK_LINUX_A2DP_DUMP_TX_STEREO TRUE
#if defined(MTK_LINUX_A2DP_DUMP_TX_STEREO) && (MTK_LINUX_A2DP_DUMP_TX_STEREO == TRUE)
    FILE *outputSteSampleFile = NULL;
    char outputSteFilename[128] = "/data/misc/bluetooth/bluedroid_output_sample.ste";
#endif


typedef enum
{
    BT_A2DP_STEREO_PB_STATUS_UNINIT = 0,
    BT_A2DP_STEREO_PB_STATUS_OPENED,
    BT_A2DP_STEREO_PB_STATUS_PLAYED,
    BT_A2DP_STEREO_PB_STATUS_PAUSED,
    BT_A2DP_STEREO_PB_STATUS_STOPED,
    BT_A2DP_STEREO_PB_STATUS_MAX
}BT_A2DP_STEREO_PB_STATUS;

/* MACRO FUNCTION DECLARATIONS
 */

#define STEREO_DBG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define STEREO_DBG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define STEREO_DBG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define STEREO_DBG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define STEREO_DBG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define STEREO_DBG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define BT_A2DP_STEREO_PB_REPORT_EVENT(event) do{          \
        if(g_bt_a2dp_stereo_pb_event_cb)                   \
        {                                                \
            STEREO_DBG_NORMAL("report event:%d", event);   \
            g_bt_a2dp_stereo_pb_event_cb(event);           \
        }                                                \
    }while(0)



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

static BT_A2DP_PLAYER_EVENT_CB g_bt_a2dp_stereo_pb_event_cb = NULL;

BT_PLAYBACK_CB_T g_bt_playback_cb = {0};
BT_A2DP_STEREO_PB_STATUS audio_status = BT_A2DP_STEREO_PB_STATUS_UNINIT;
static BT_A2DP_PLAYER g_bt_a2dp_stereo_player = {0};
#define PCMDATA_BUFLEN 512 * 5 + 8  /*frame length=512,frame number=5, pts size = 8 bytes*/
static UINT8 *pcmdata_buffer = NULL; /*fixed length buffer send to stereo APP*/
UINT32 pcmdata_buffer_len = 0;
UINT32 discard_pcmdata_len = 0;


INT32 bt_a2dp_stereo_player_init(BT_A2DP_PLAYER_EVENT_CB event_cb);
INT32 bt_a2dp_stereo_player_deinit(void);
INT32 bt_a2dp_stereo_player_start(INT32 trackFreq, INT32 channelNum, INT32 bitDepth);
INT32 bt_a2dp_stereo_player_play(VOID);
INT32 bt_a2dp_stereo_player_pause(VOID);
INT32 bt_a2dp_stereo_player_stop(VOID);
INT32 bt_a2dp_stereo_player_write(VOID *data, INT32 datalen);
INT32 bt_a2dp_stereo_player_adjust_buf_time(UINT32 buffer_time);

EXPORT_SYMBOL INT32 bt_a2dp_stereo_player_adjust_buf_time(UINT32 buffer_time)
{
    STEREO_DBG_ERROR("<addbuf> ADD_BUFFER is disabled!");
    return 0;
}

EXPORT_SYMBOL BT_A2DP_PLAYER* bt_a2dp_stereo_get_player(VOID)
{
    g_bt_a2dp_stereo_player.init = bt_a2dp_stereo_player_init;
    g_bt_a2dp_stereo_player.deinit = bt_a2dp_stereo_player_deinit;
    g_bt_a2dp_stereo_player.start = bt_a2dp_stereo_player_start;
    g_bt_a2dp_stereo_player.stop = bt_a2dp_stereo_player_stop;
    g_bt_a2dp_stereo_player.play = bt_a2dp_stereo_player_play;
    g_bt_a2dp_stereo_player.pause = bt_a2dp_stereo_player_pause;
    g_bt_a2dp_stereo_player.write = bt_a2dp_stereo_player_write;
    g_bt_a2dp_stereo_player.adjust_buf_time = NULL;
    g_bt_a2dp_stereo_player.support_codec_mask = (1 << BT_A2DP_CODEC_TYPE_STE); //only support STE(1(SBC),2(MP3),4(AAC),8(LDAC),16(STE))
    strncpy(g_bt_a2dp_stereo_player.name, "stereo_player", strlen("stereo_player"));
    return &g_bt_a2dp_stereo_player;
}

static INT32 stereo_open(INT32 fs, INT32 channel_num, INT32 bit_depth)
{
    STEREO_DBG_NORMAL("+++into");
    INT32 i4ret = 0;

    stereo_config_t config;
    memset(&config, 0, sizeof(stereo_config_t));
    config.samplerate = fs;
    config.channels = channel_num;
    config.device_type = 1; //0 for master mode,1 for slave mode
    config.bitdepth = bit_depth;
    i4ret = stereo_app_open(&config);
    STEREO_DBG_NORMAL("fs %d, channel num %d ", fs, channel_num);
    if (i4ret < 0)
    {
        STEREO_DBG_ERROR("Cannot open stereo app ,ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        return  -1;
    }
    BT_A2DP_STEREO_PB_REPORT_EVENT(BT_A2DP_PB_EVENT_START);

    audio_status = BT_A2DP_STEREO_PB_STATUS_OPENED;
    return 0;
}

static INT32 stereo_write(UINT64 pts, UINT8 *buf, UINT32 size)
{
    STEREO_DBG_INFO("enter stereo_write");
    INT32 i4ret = 0;
#if defined(MTK_LINUX_A2DP_DUMP_TX_STEREO) && (MTK_LINUX_A2DP_DUMP_TX_STEREO == TRUE)
    if (outputSteSampleFile)
    {
        fwrite(((UINT8*)buf), 1,
            (size_t)(size), outputSteSampleFile);
        STEREO_DBG_INFO("write ste pcm data that send to stereo app to file");
    }
    else
    {
        STEREO_DBG_INFO("outputSteSampleFile is empty");
    }
#endif //#if defined(MTK_LINUX_A2DP_DUMP_TX_STEREO)
    i4ret = stereo_app_senddata_withpts(pts, (CHAR*)buf, size);
    if (i4ret < 0)
    {
        STEREO_DBG_ERROR("STEREO ERROR %d[%s]", i4ret, snd_strerror(i4ret));
    }
    STEREO_DBG_INFO("exit dsp write");
    return i4ret;
}

static INT32 stereo_close(VOID)
{
    INT32 i4ret = 0;
    STEREO_DBG_NORMAL("+++into");


    i4ret = stereo_app_close();
    if (i4ret == 0)
    {
        STEREO_DBG_NORMAL("stereo_close success");
        BT_A2DP_STEREO_PB_REPORT_EVENT(BT_A2DP_PB_EVENT_STOP);
    }
    else
    {
        STEREO_DBG_ERROR("stereo_close fail i4ret=%d[%s]", i4ret, snd_strerror(i4ret));
    }

    audio_status = BT_A2DP_STEREO_PB_STATUS_UNINIT;

    return i4ret;
}

INT32 bt_a2dp_stereo_player_start(INT32 trackFreq, INT32 channelNum, INT32 bitDepth)
{
    INT32 i4_ret = BT_SUCCESS;

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        i4_ret = stereo_open(trackFreq, channelNum, bitDepth);
        g_bt_playback_cb.fgPlayBackInit = TRUE;
        memset(pcmdata_buffer, 0, sizeof(pcmdata_buffer));
    }
    else
    {
        STEREO_DBG_WARNING("BT playback have init,no need init again");
    }
    return i4_ret;
}

INT32 bt_a2dp_stereo_player_stop(VOID)
{
    STEREO_DBG_NORMAL("+++into");
    INT32 i4_ret = BT_SUCCESS;

    if (TRUE == g_bt_playback_cb.fgPlayBackInit)
    {
        g_bt_playback_cb.fgPlayBackInit = FALSE;
        i4_ret = stereo_close();
    }
    else
    {
        STEREO_DBG_WARNING("BT playback have not init,no need deinit");
    }

    STEREO_DBG_NORMAL("---exit");
    return i4_ret;
}

static INT32 bt_a2dp_stereo_write_data(UINT8 *pPcmBuf, UINT32 u4PcmLen)
{
    STEREO_DBG_INFO("enter bt_a2dp_stereo_write_data");
    if ((NULL == pPcmBuf) || (0 >= u4PcmLen))
    {
      STEREO_DBG_ERROR("invalid data");
      return BT_ERR_STATUS_PARM_INVALID;
    }

    STEREO_DBG_INFO("send %ld data", (long)u4PcmLen);

    /*bt playback does not init; audio_reset ;audio_status is not played*/
    if((FALSE == g_bt_playback_cb.fgPlayBackInit) || (TRUE == g_bt_playback_cb.fgAudioReset) || (audio_status != BT_A2DP_STEREO_PB_STATUS_PLAYED))
    {    /*discard pcmdata as player is not ready*/
      discard_pcmdata_len +=  u4PcmLen;
      STEREO_DBG_INFO("discard_pcmdata_len=%d", discard_pcmdata_len);
      /*if some frames decoder fail*/
      if ((discard_pcmdata_len - 8) % 512 != 0)
      {
        discard_pcmdata_len = 0;
        discard_pcmdata_len += u4PcmLen;
        STEREO_DBG_WARNING("* some frames decoder fail");
      }
      if (discard_pcmdata_len == PCMDATA_BUFLEN)
      {
       discard_pcmdata_len = 0;
       STEREO_DBG_INFO("has discard a full packet pcm data,len =%d", discard_pcmdata_len);
      }
      else
      {
       STEREO_DBG_INFO("has discard %d length data", discard_pcmdata_len);
      }
    }
    /*player is ready */
    else
    {
/*judgment if need discard pcmdata depend on discard_pcmdata_len when player is ready,because need discard data a full packet */
      if (discard_pcmdata_len != 0)
      {
        discard_pcmdata_len +=  u4PcmLen;
         /*if some frames decoder fail*/
        if ((discard_pcmdata_len - 8) % 512 != 0)
        {
          discard_pcmdata_len = 0;
          discard_pcmdata_len += u4PcmLen;
          STEREO_DBG_WARNING("some frames decoder fail");
        }
        if (discard_pcmdata_len == PCMDATA_BUFLEN)
        {
          discard_pcmdata_len = 0;
          STEREO_DBG_INFO("has discard a packet pcm data when player ready,len =%d", discard_pcmdata_len);
        }
      }
      else
      {
         /*if some frames decoder fail*/
        if ((pcmdata_buffer_len + u4PcmLen - 8) % 512 != 0)
        {
          pcmdata_buffer_len = 0;
          STEREO_DBG_WARNING("!some frames decoder fail");
        }

        memcpy(pcmdata_buffer + pcmdata_buffer_len, pPcmBuf, u4PcmLen);

        pcmdata_buffer_len += u4PcmLen;
        STEREO_DBG_INFO("pcmdata_buffer_len = %d", pcmdata_buffer_len);
        if (PCMDATA_BUFLEN ==  pcmdata_buffer_len)
        {
          STEREO_DBG_INFO("send pts and pcmdata");
          pcmdata_buffer_len = 0;

          /*parse sink PTS from pcm data,then send pts + fixed length pcm data*/
          UINT64 pts = 0;
          memcpy(&pts, pcmdata_buffer, sizeof(UINT64));
          STEREO_DBG_INFO(" pts = %d ", pts);
          stereo_write(pts, pcmdata_buffer + 8, PCMDATA_BUFLEN - 8); /*pts length = 8 bytes */
        }
      }
     }
    return BT_SUCCESS;
}

INT32 bt_a2dp_stereo_player_play(VOID)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        STEREO_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if ((audio_status != BT_A2DP_STEREO_PB_STATUS_OPENED)
            && (audio_status != BT_A2DP_STEREO_PB_STATUS_PAUSED)
            && (audio_status != BT_A2DP_STEREO_PB_STATUS_STOPED))
    {
        STEREO_DBG_NORMAL("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_STEREO_PB_STATUS_PLAYED;
        STEREO_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }
    return BT_SUCCESS;
}

INT32 bt_a2dp_stereo_player_pause(VOID)
{
    STEREO_DBG_NORMAL("+++into");

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        STEREO_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if (audio_status != BT_A2DP_STEREO_PB_STATUS_PLAYED)
    {
        STEREO_DBG_NOTICE("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_STEREO_PB_STATUS_PAUSED;
        STEREO_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return BT_SUCCESS;
}

INT32 bt_a2dp_stereo_player_write(VOID *data, INT32 datalen)
{
    UINT32 u4PcmLen = datalen;
    UINT8 *pu1PcmBuf = (UINT8 *)data;
    INT32 i4_ret = BT_SUCCESS;

    if (NULL == data || 0 == datalen)
    {
        STEREO_DBG_ERROR("data is null(%p) or data len=%d", data, datalen);
        return BT_ERR_STATUS_FAIL;
    }

    STEREO_DBG_INFO("data=%p, datalen=%u", data, datalen);

    if (BT_SUCCESS != bt_a2dp_stereo_write_data(pu1PcmBuf, u4PcmLen))
    {
        STEREO_DBG_ERROR("play pcm failed, pcmbuf:%p, len:%lu", pu1PcmBuf, (long)u4PcmLen);
        return i4_ret;
    }
    STEREO_DBG_INFO("exit bt_a2dp_stereo_player_write");
    return i4_ret;
}

INT32 bt_a2dp_stereo_player_init(BT_A2DP_PLAYER_EVENT_CB event_cb)
{
    STEREO_DBG_NORMAL("+++into");
    g_bt_a2dp_stereo_pb_event_cb = event_cb;
    pcmdata_buffer = (UINT8 *)malloc(PCMDATA_BUFLEN);
    if (pcmdata_buffer == NULL)
    {
        STEREO_DBG_ERROR("malloc pcmdata buffer failed");
    }
    pcmdata_buffer_len = 0;
    discard_pcmdata_len = 0;

#if defined(MTK_LINUX_A2DP_DUMP_TX_STEREO) && (MTK_LINUX_A2DP_DUMP_TX_STEREO == TRUE)
    outputSteSampleFile = fopen(outputSteFilename, "wb+");
    if (outputSteSampleFile == NULL)
    {
        STEREO_DBG_ERROR("outputSteSampleFile open fail");
    }
#endif
    return 0;
}

INT32 bt_a2dp_stereo_player_deinit(void)
{
    STEREO_DBG_NORMAL("+++into");
    g_bt_a2dp_stereo_pb_event_cb = NULL;

    free(pcmdata_buffer);
    pcmdata_buffer = NULL;

    pcmdata_buffer_len = 0;
    discard_pcmdata_len = 0;
#if defined(MTK_LINUX_A2DP_DUMP_TX_STEREO) && (MTK_LINUX_A2DP_DUMP_TX_STEREO == TRUE)
    if (outputSteSampleFile)
    {
        fclose(outputSteSampleFile);
        outputSteSampleFile = NULL;
    }
#endif
    return 0;
}

static BT_A2DP_GET_PLAYER_METHODS get_player_methods = {
    .get_player = bt_a2dp_stereo_get_player,
};

__attribute__ ((visibility ("default")))
EXPORT_SYMBOL BT_A2DP_PLAYER_MODULE PLAYER_MODULE_INFO_SYM = {
        .version_major = 1,
        .version_minor = 0, /*player version*/
        .methods = &get_player_methods,
};

