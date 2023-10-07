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

/* FILE NAME:  bt_a2dp_demo_playback.c
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

/*need include head file*/
#include "u_bt_mw_common.h"
#include "bt_a2dp_demo_playback.h"


/*define a demo player*/
static BT_A2DP_PLAYER g_bt_a2dp_demo_player = {0};


/*a sink player need to inplement the following interfaces*/
INT32 bt_a2dp_demo_player_init(BT_A2DP_PLAYER_EVENT_CB event_cb);
INT32 bt_a2dp_demo_player_deinit(VOID);
INT32 bt_a2dp_demo_player_start(INT32 trackFreq, INT32 channelType);
INT32 bt_a2dp_demo_player_play(VOID);
INT32 bt_a2dp_demo_player_pause(VOID);
INT32 bt_a2dp_demo_player_stop(VOID);
INT32 bt_a2dp_demo_player_write(VOID *data, INT32 datalen);
INT32 bt_a2dp_demo_player_adjust_buf_time(UINT32 buffer_time);


/* FUNCTION NAME: bt_a2dp_demo_player_adjust_buf_time
 * PURPOSE:
 *      set buffer time to  buffer a period of time data then start to play when network is bad or some other case
 * INPUT:
 *      buffer time    -- can compute buffer time base on sample rate, channel num and received data
 * OUTPUT:
 *      N/A
 * RETURN:
 *      N/A
 * NOTES:
 * generally not need to implement
 */
EXPORT_SYMBOL INT32 bt_a2dp_demo_player_adjust_buf_time(UINT32 buffer_time)
{

    return 0;
}


/* FUNCTION NAME: bt_a2dp_demo_get_player
 * PURPOSE:
 *      assign a demo player base on current interface
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *      a demo player that has been assignmented
 * NOTES:
 *      N/A
 */
EXPORT_SYMBOL BT_A2DP_PLAYER* bt_a2dp_demo_get_player(VOID)
{
    g_bt_a2dp_demo_player.init = bt_a2dp_demo_player_init;
    g_bt_a2dp_demo_player.deinit = bt_a2dp_demo_player_deinit;
    g_bt_a2dp_demo_player.start = bt_a2dp_demo_player_start;
    g_bt_a2dp_demo_player.stop = bt_a2dp_demo_player_stop;
    g_bt_a2dp_demo_player.play = bt_a2dp_demo_player_play;
    g_bt_a2dp_demo_player.pause = bt_a2dp_demo_player_pause;
    g_bt_a2dp_demo_player.write = bt_a2dp_demo_player_write;
    g_bt_a2dp_demo_player.adjust_buf_time = NULL;
    g_bt_a2dp_demo_player.support_codec_mask = (1+2+4+8+16); //player can support codec, only support STE(1(SBC),2(MP3),4(AAC),8(LDAC),16(STE))
    strncpy(g_bt_a2dp_demo_player.name, "demo player", strlen("demo player")); //player name
    return &g_bt_a2dp_demo_player;
}


/* FUNCTION NAME: bt_a2dp_demo_player_start
 * PURPOSE:
 *      init output device base trackFreq and channel num
 * INPUT:
 *      current sample rate and channel num
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS: output device init success
 *      BT_ERR_STATUS_xx: output device init fail
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_start(INT32 trackFreq, INT32 channelNum, INT32 bitDepth)
{
    INT32 i4_ret = BT_SUCCESS;
    /*set player init state*/
    /*init output device base trackFreq and channel num*/
    /*report Player state to upper layer by callback that registered in player init*/
    return i4_ret;
}


/* FUNCTION NAME: bt_a2dp_demo_player_stop
 * PURPOSE:
 *      deinit output device and reset player current state
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS: output device close success
 *      BT_ERR_STATUS_xx: output device close fail
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_stop(VOID)
{
    INT32 i4ret = BT_SUCCESS;
    /*reset player current state*/
    /*close output device*/
    /*report Player state to upper layer by callback that registered in player init*/
    return i4ret;
}


/* FUNCTION NAME: bt_a2dp_demo_player_play
 * PURPOSE:
 *      set player current state to play,prepare to start play
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *     N/A
 *     N/A
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_play(VOID)
{
    INT32 i4ret = 0;

    /*set player current state to play*/
    return i4ret;
}


/* FUNCTION NAME: bt_a2dp_demo_player_pause
 * PURPOSE:
 *      set player current state to pause, prepare to pause playback
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *     N/A
 *     N/A
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_pause(VOID)
{
    INT32 i4ret = 0;
    /*player current state set to pause*/
    return i4ret;
}


/* FUNCTION NAME: bt_a2dp_demo_player_write
 * PURPOSE:
 *      send data to output device
 * INPUT:
 *      received data and data length
 * OUTPUT:
 *      N/A
 * RETURN:
 *     BT_SUCCESS: data send output device success
 *     BT_ERR_STATUS_xx:data send output device fail
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_write(VOID *data, INT32 datalen)
{
   INT32 i4ret = BT_SUCCESS;
  /*demo playback send data out*/
   return i4ret;
}


/* FUNCTION NAME: bt_a2dp_demo_player_init
 * PURPOSE:
 *      register report player state event callback and do initialized action
 * INPUT:
 *      a2dp player event callback function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      N/A
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_init(BT_A2DP_PLAYER_EVENT_CB event_cb)
{
    /*register report player state event callback, then do player initialization action*/
    return 0;
}


/* FUNCTION NAME: bt_a2dp_demo_player_deinit
 * PURPOSE:
 *     free resource and do deinitialized action
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *      N/A
 * NOTES:
 *      N/A
 */
INT32 bt_a2dp_demo_player_deinit(void)
{
    /*free resource*/
    return 0;
}


/*exported function*/
static BT_A2DP_GET_PLAYER_METHODS get_player_methods = {
    .get_player = bt_a2dp_demo_get_player,
};

/*demo player symbol, when link demo player so dynamicly, will get related function by the symbol*/
__attribute__ ((visibility ("default")))
EXPORT_SYMBOL BT_A2DP_PLAYER_MODULE PLAYER_MODULE_INFO_SYM = {
        .version_major = 1,
        .version_minor = 0, /*player version, ex: 1.0*/
        .methods = &get_player_methods, /**/
};


