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

/* FILE NAME:  linuxbt_a2dp_src_if.c
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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_mw_a2dp_common.h"
#include "linuxbt_common.h"
#include "mtk_bluetooth.h"

#include "bt_av.h"

#include "c_mw_config.h"
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif
#include "bt_audio_track.h"

#include "linuxbt_gap_if.h"
#include "linuxbt_a2dp_src_if.h"

#include "bt_mw_message_queue.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_A2DP_SRC_SET_MSG_LEN(msg) do{   \
    msg.hdr.len = sizeof(tBT_MW_A2DP_MSG);      \
    }while(0)
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */
extern uint8_t audio_set_trace_level(uint8_t new_level);


// Callback functions
static void linuxbt_a2dp_src_connection_state_cb(const RawAddress& bd_addr, btav_connection_state_t state);
static void linuxbt_a2dp_src_audio_state_cb(const RawAddress& bd_addr, btav_audio_state_t state);
static void linuxbt_a2dp_src_audio_config_cb(const RawAddress& bd_addr,
    btav_a2dp_codec_config_t codec_config,
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities,
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities);
static void linuxbt_a2dp_src_audio_delay_cb(const RawAddress& bd_addr,
                                                         uint16_t delay);

static btav_source_interface_t *g_bt_a2dp_src_interface = NULL;
static btav_source_callbacks_t g_bt_a2dp_src_callbacks =
{
    sizeof(btav_source_callbacks_t),
    linuxbt_a2dp_src_connection_state_cb,
    linuxbt_a2dp_src_audio_state_cb,
    linuxbt_a2dp_src_audio_config_cb,
    linuxbt_a2dp_src_audio_delay_cb, // mtk add ext
};

#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
static btav_src_ext_interface_t *g_bt_a2dp_src_ext_interface = NULL;
static btav_src_ext_callbacks_t g_bt_a2dp_src_ext_callbacks =
{
    sizeof(btav_src_ext_callbacks_t),
    linuxbt_a2dp_src_audio_delay_cb,
};
#endif


static void linuxbt_a2dp_src_codec_config_parser_param(BT_A2DP_CODEC_CONFIG *p_bt_mw_a2dp_codec_config,
                                                                     btav_a2dp_codec_config_t *p_stack_codec_config)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP,"codec_type: %d", p_stack_codec_config->codec_type);

    p_bt_mw_a2dp_codec_config->codec_type = (BT_A2DP_CODEC_TYPE)p_stack_codec_config->codec_type;
    p_bt_mw_a2dp_codec_config->sample_rate = (BT_A2DP_CODEC_SAMPLE_RATE)p_stack_codec_config->sample_rate;
    p_bt_mw_a2dp_codec_config->bits_per_sample = (BT_A2DP_CODEC_BITS_PER_SAMPLE)p_stack_codec_config->bits_per_sample;
    p_bt_mw_a2dp_codec_config->channel_mode = (BT_A2DP_CODEC_CHANNEL_MODE)p_stack_codec_config->channel_mode;
    p_bt_mw_a2dp_codec_config->codec_priority = (BT_A2DP_CODEC_PRIORITY)p_stack_codec_config->codec_priority;
    p_bt_mw_a2dp_codec_config->codec_specific_1 = p_stack_codec_config->codec_specific_1;
    p_bt_mw_a2dp_codec_config->codec_specific_2 = p_stack_codec_config->codec_specific_2;
    p_bt_mw_a2dp_codec_config->codec_specific_3 = p_stack_codec_config->codec_specific_3;
    p_bt_mw_a2dp_codec_config->codec_specific_4 = p_stack_codec_config->codec_specific_4;
}

static void linuxbt_a2dp_src_connection_state_cb(const RawAddress& bd_addr, btav_connection_state_t state)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));

    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr.address[0], btaddr.address[1], btaddr.address[2],
                      btaddr.address[3], btaddr.address[4], btaddr.address[5]);
    linuxbt_btaddr_htos(&btaddr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", btmw_msg.data.a2dp_msg.addr);

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] src state: %s(%d) ",
                  state==BTAV_CONNECTION_STATE_DISCONNECTED?"DISCONNECTED":
                  state==BTAV_CONNECTION_STATE_CONNECTING?"CONNECTING":
                  state==BTAV_CONNECTION_STATE_CONNECTED?"CONNECTED":
                  state==BTAV_CONNECTION_STATE_DISCONNECTING?"DISCONNECTING":
                  "UNKNOWN",
                  state);



    if (state == BTAV_CONNECTION_STATE_DISCONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTED;
    }
    else if (state == BTAV_CONNECTION_STATE_CONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTED;
    }
    else if (state == BTAV_CONNECTION_STATE_DISCONNECTING)
    {
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTING;

    }
    else if (state==BTAV_CONNECTION_STATE_CONNECTING)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTING;
    }
    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_src_audio_state_cb(const RawAddress& bd_addr, btav_audio_state_t state)
{

    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));

    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr.address[0], btaddr.address[1], btaddr.address[2],
                      btaddr.address[3], btaddr.address[4], btaddr.address[5]);
    linuxbt_btaddr_htos(&btaddr, btmw_msg.data.a2dp_msg.addr);

    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] state: %s(%d) ",
                  state==BTAV_AUDIO_STATE_STARTED?"STARTED":
                  state==BTAV_AUDIO_STATE_STOPPED?"STOPPED":
                  state==BTAV_AUDIO_STATE_REMOTE_SUSPEND?"SUSPEND":"UNKNOWN",
                  state);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", btmw_msg.data.a2dp_msg.addr);

    if (BTAV_AUDIO_STATE_STARTED == state)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_START;
    }
    else if (BTAV_AUDIO_STATE_REMOTE_SUSPEND == state)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_SUSPEND;
    }
    else if (BTAV_AUDIO_STATE_STOPPED == state)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_SUSPEND;
    }
    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_src_audio_config_cb(const RawAddress& bd_addr,
    btav_a2dp_codec_config_t codec_config,
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities,
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities)
{
    INT32 i = 0;
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    btmw_msg.hdr.event = BTMW_A2DP_STREAM_CONFIG;

    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr.address[0], btaddr.address[1], btaddr.address[2],
                      btaddr.address[3], btaddr.address[4], btaddr.address[5]);
    linuxbt_btaddr_htos(&btaddr, btmw_msg.data.a2dp_msg.addr);

    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_DBG_NORMAL(BT_DEBUG_A2DP,"addr: %s", btmw_msg.data.a2dp_msg.addr);

    linuxbt_a2dp_src_codec_config_parser_param(&btmw_msg.data.a2dp_msg.data.src_codec_config.audio_config, &codec_config);

    for (auto cp : codecs_local_capabilities) {
        BT_DBG_INFO(BT_DEBUG_A2DP, "codecs_local_capabilities=%s", cp.ToString().c_str());
        linuxbt_a2dp_src_codec_config_parser_param(&btmw_msg.data.a2dp_msg.data.src_codec_config.local_codec_capabilities.codec_config_list[i], &cp);
        i++;
    }
    btmw_msg.data.a2dp_msg.data.src_codec_config.local_codec_capabilities.codec_config_num = i;

    i = 0;
    for (auto cp : codecs_selectable_capabilities) {
        BT_DBG_INFO(BT_DEBUG_A2DP, "codecs_selectable_capabilities=%s", cp.ToString().c_str());
        linuxbt_a2dp_src_codec_config_parser_param(&btmw_msg.data.a2dp_msg.data.src_codec_config.codec_selectable_capabilities.codec_config_list[i], &cp);
        i++;
    }
    btmw_msg.data.a2dp_msg.data.src_codec_config.codec_selectable_capabilities.codec_config_num = i;

    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_src_audio_delay_cb(const RawAddress& bd_addr,
                                                         uint16_t delay)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, " delay: %d", delay);
    RawAddress btaddr;
    memset(&btaddr, 0, sizeof(RawAddress));
    memcpy(&btaddr, &bd_addr, sizeof(RawAddress));
    BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr.address[0], btaddr.address[1], btaddr.address[2],
                      btaddr.address[3], btaddr.address[4], btaddr.address[5]);

    btmw_msg.hdr.event = BTMW_A2DP_DELAY;
    linuxbt_btaddr_htos(&btaddr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_DBG_NORMAL(BT_DEBUG_A2DP," addr: %s", btmw_msg.data.a2dp_msg.addr);

    btmw_msg.data.a2dp_msg.data.delay.delay = delay;

    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

INT32 linuxbt_a2dp_src_connect(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);

    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->connect)
    {
        g_bt_a2dp_src_interface->connect(bdaddr); //need refactor
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_disconnect(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);


    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->disconnect)
    {
        status = g_bt_a2dp_src_interface->disconnect(bdaddr); //need refactor
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_set_channel_allocation_for_lrmode(CHAR *pbt_addr, int channel)
{
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);


    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->set_channel_allocation_for_lrmode)
    {
         g_bt_a2dp_src_interface->set_channel_allocation_for_lrmode(bdaddr, channel);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC no set_channel_allocation_for_lrmode");
        return BT_ERR_STATUS_FAIL;
    }

    return BT_SUCCESS;
}

INT32 linuxbt_a2dp_src_set_audiomode(int AudioMode)
{
    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->set_audiomode)
    {
         g_bt_a2dp_src_interface->set_audiomode(AudioMode);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC no set_audiomode");
        return BT_ERR_STATUS_FAIL;
    }

    return BT_SUCCESS;
}


INT32 linuxbt_a2dp_src_codec_enable_handler(BT_A2DP_CODEC_TYPE codec_type, BOOL enable)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "codec_type[%d], enable[%d]", codec_type, enable);
    uint8_t codec_type_tmp;

    if (codec_type == BT_A2DP_CODEC_TYPE_AAC)
    {
        codec_type_tmp = MW_BTA_AV_CODEC_TYPE_AAC;
    }
    else if(codec_type == BT_A2DP_CODEC_TYPE_LDAC)
    {
        codec_type_tmp = MW_BTA_AV_CODEC_TYPE_LDAC;
    }
    else if(codec_type == BT_A2DP_CODEC_TYPE_APTX)
    {
        codec_type_tmp = MW_BTA_AV_CODEC_TYPE_APTX;
    }
    else if(codec_type == BT_A2DP_CODEC_TYPE_APTX_HD)
    {
        codec_type_tmp = MW_BTA_AV_CODEC_TYPE_APTXHD;
    }
    else
    {
        return BT_ERR_STATUS_FAIL;
    }

    if (g_bt_a2dp_src_interface != NULL)
    {
        g_bt_a2dp_src_interface->codec_enable(codec_type_tmp, enable);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
    }

    return BT_SUCCESS;
}

static btav_a2dp_codec_index_t linuxbt_a2dp_codec_change_index(BT_A2DP_CODEC_TYPE codec_type)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    switch (codec_type) {
        case BT_A2DP_CODEC_TYPE_SBC:
          return BTAV_A2DP_CODEC_INDEX_SOURCE_SBC;
        case BT_A2DP_CODEC_TYPE_AAC:
          return BTAV_A2DP_CODEC_INDEX_SOURCE_AAC;
        case BT_A2DP_CODEC_TYPE_LDAC:
          return BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC;
        case BT_A2DP_CODEC_TYPE_LHDC_LL:
          return BTAV_A2DP_CODEC_INDEX_SOURCE_APTX;
        case BT_A2DP_CODEC_TYPE_LHDC:
          return BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_HD;
        //case BT_A2DP_CODEC_TYPE_STE:
          //return BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;
        default:
          BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] codec_type=%d", codec_type);
          return BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;

    }
}

INT32 linuxbt_a2dp_src_init(BT_A2DP_SRC_INIT_CONFIG *p_src_init_config)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    INT32 ret = 0;
    INT32 i = 0;
    INT32 max_num_device = p_src_init_config->max_connected_audio_devices;
    INT32 num_of_codec_in_codec_list = p_src_init_config->codec_config_list.codec_config_num;
    btav_a2dp_codec_config_t codec_config;
    std::vector<btav_a2dp_codec_config_t> codec_priorities;
    codec_priorities.clear();

    for (i = 0; i < num_of_codec_in_codec_list; i++)
    {
        codec_config.codec_type =
        linuxbt_a2dp_codec_change_index(p_src_init_config->codec_config_list.codec_config_list[i].codec_type);
        codec_config.codec_priority =
        (btav_a2dp_codec_priority_t)p_src_init_config->codec_config_list.codec_config_list[i].codec_priority;
        codec_config.sample_rate =
        (btav_a2dp_codec_sample_rate_t)p_src_init_config->codec_config_list.codec_config_list[i].sample_rate;
        codec_config.bits_per_sample =
        (btav_a2dp_codec_bits_per_sample_t)p_src_init_config->codec_config_list.codec_config_list[i].bits_per_sample;
        codec_config.channel_mode=
        (btav_a2dp_codec_channel_mode_t)p_src_init_config->codec_config_list.codec_config_list[i].channel_mode;
        codec_config.codec_specific_1=
        p_src_init_config->codec_config_list.codec_config_list[i].codec_specific_1;
        codec_config.codec_specific_2=
        p_src_init_config->codec_config_list.codec_config_list[i].codec_specific_2;
        codec_config.codec_specific_3=
        p_src_init_config->codec_config_list.codec_config_list[i].codec_specific_3;
        codec_config.codec_specific_4=
        p_src_init_config->codec_config_list.codec_config_list[i].codec_specific_4;
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] codec_config.codec_type=%d", codec_config.codec_type);
        codec_priorities.push_back(codec_config);
    }

    // Get A2DP SRC interface
    g_bt_a2dp_src_interface = (btav_source_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
    if (g_bt_a2dp_src_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_FAIL;
    }

    // Init A2DP interface
    ret = g_bt_a2dp_src_interface->init(&g_bt_a2dp_src_callbacks,
                                  max_num_device, codec_priorities); //need refactor
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] success to init A2DP interface");
    }
    else if (ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] already init A2DP interface");
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to init A2DP interface");
    }
    return linuxbt_return_value_convert(ret);
}

VOID linuxbt_a2dp_src_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    if (g_bt_a2dp_src_interface != NULL)
    {
        g_bt_a2dp_src_interface->cleanup();
    }
    g_bt_a2dp_src_interface = NULL;
    FUNC_EXIT;
}

INT32 linuxbt_a2dp_src_active_sink(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);

    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->set_active_device)
    {
        g_bt_a2dp_src_interface->set_active_device(bdaddr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP src active_sink is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_set_silence_device(CHAR *pbt_addr, BOOL enable)
{
    INT32 status = BT_SUCCESS;
    RawAddress bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);

    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->set_silence_device)
    {
        g_bt_a2dp_src_interface->set_silence_device(bdaddr, enable);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP src active_sink is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_config_codec_info(CHAR *pbt_addr, BT_A2DP_CODEC_CONFIG *p_src_set_codec_config)
{
    INT32 status = BT_SUCCESS;
    RawAddress bdaddr;

    btav_a2dp_codec_config_t codec_config;
    memset(&codec_config, 0, sizeof(btav_a2dp_codec_config_t));
    std::vector<btav_a2dp_codec_config_t> codec_priorities;
    codec_priorities.clear();

    BT_CHECK_POINTER(BT_DEBUG_A2DP, g_bt_a2dp_src_interface);
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);

    codec_config.codec_type =
    (btav_a2dp_codec_index_t)p_src_set_codec_config->codec_type;
    codec_config.codec_priority =
    (btav_a2dp_codec_priority_t)p_src_set_codec_config->codec_priority;
    codec_config.sample_rate =
    (btav_a2dp_codec_sample_rate_t)p_src_set_codec_config->sample_rate;
    codec_config.bits_per_sample =
    (btav_a2dp_codec_bits_per_sample_t)p_src_set_codec_config->bits_per_sample;
    codec_config.channel_mode=
    (btav_a2dp_codec_channel_mode_t)p_src_set_codec_config->channel_mode;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.codec_type =%d", codec_config.codec_type);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.codec_priority =%d", codec_config.codec_priority);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.sample_rate =%d", codec_config.sample_rate);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.bits_per_sample =%d", codec_config.bits_per_sample);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.channel_mode =%d", codec_config.channel_mode);

    codec_priorities.push_back(codec_config);
    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->config_codec)
    {
        g_bt_a2dp_src_interface->config_codec(bdaddr, codec_priorities);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP src active_sink is not enable");
    }
    return status;

}

INT32 linuxbt_a2dp_set_audio_hw_log_lvl(UINT8 log_level)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "log_level=%d", log_level);
    return audio_set_trace_level(log_level);
}

