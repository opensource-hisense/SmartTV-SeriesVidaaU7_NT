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
#include <pthread.h>
#include <experimental/optional>
#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_leaudio_bms_if.h"

#if MTK_LEAUDIO_BMS
#include "linuxbt_leaudio_bass_if.h"
#include "u_bt_mw_leaudio_bms.h"
#include "bt_mw_leaudio_bms.h"
#include "bt_mw_message_queue.h"
#include "bluetooth.h"
#include "bt_le_audio.h"

using bluetooth::le_audio::LeAudioBroadcasterInterface;
using bluetooth::le_audio::LeAudioBroadcasterCallbacks;
using bluetooth::le_audio::LeAudioBroadcastAnnouncementParam;
using bluetooth::le_audio::LeAudioBroadcastAnnouncementSubgroupParam;
using bluetooth::le_audio::LeAudioBroadcastAnnouncementBisParam;
using bluetooth::le_audio::LeAudioDataPathConfig;
using bluetooth::le_audio::BroadcastAudioProfile;
using bluetooth::le_audio::BroadcastState;
using bluetooth::le_audio::AudioCodecConfigSet;
using bluetooth::le_audio::AudioBroadcastStreamConfigSet;
using bluetooth::le_audio::AudioCodecConfigSet;
using bluetooth::le_audio::LeAudioDataPathType;
using bluetooth::le_audio::LeAudioSocketChannelMap;

#define LINUXBT_BMS_MAX_WRITTEN_TIME_SECOND 30

static std::map<uint8_t, BroadcastState> g_broadcasts_state;
static pthread_cond_t g_stop_signal;
static pthread_mutex_t g_stop_lock;

static void linuxbt_bms_covert_base_announcement_param(
    const BT_BMS_ANNOUNCEMENT& src, LeAudioBroadcastAnnouncementParam* dst);
static int linuxbt_bms_is_vaild_broadcast();

class LeAudioBroadcasterCallbacksImpl : public LeAudioBroadcasterCallbacks {
 public:
  ~LeAudioBroadcasterCallbacksImpl() = default;

  void OnBroadcastCreated(uint8_t instance_id, bool success) override {
    tBTMW_MSG btmw_msg = {0};

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d success=%d", __FUNCTION__, instance_id, success);
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM);
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_BROADCAST_CREATED;
    btmw_msg.data.bms_event.data.broadcast_created.instance_id = instance_id;
    btmw_msg.data.bms_event.data.broadcast_created.success = success;
    bt_mw_nty_send_msg(&btmw_msg);
  }

  void OnBroadcastDestroyed(uint8_t instance_id) override {
    tBTMW_MSG btmw_msg = {0};

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d", __FUNCTION__, instance_id);
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM);
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_BROADCAST_DESTORYED;
    btmw_msg.data.bms_event.data.broadcast_destroyed.instance_id = instance_id;
    bt_mw_nty_send_msg(&btmw_msg);

    g_broadcasts_state.erase(instance_id);

    pthread_mutex_lock(&g_stop_lock);
    if (!linuxbt_bms_is_vaild_broadcast()) {
        pthread_cond_signal(&g_stop_signal);
    }
    pthread_mutex_unlock(&g_stop_lock);
  }

  void OnBroadcastStateChanged(uint8_t instance_id, BroadcastState state) override {
    tBTMW_MSG btmw_msg = {0};

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d state=%d", __FUNCTION__, instance_id, (int)state);
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM);
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_BROADCAST_STATE;
    btmw_msg.data.bms_event.data.broadcast_state.instance_id = instance_id;
    btmw_msg.data.bms_event.data.broadcast_state.state = (BT_BMS_BROADCAST_STATE)state;
    bt_mw_nty_send_msg(&btmw_msg);

    if (BroadcastState::STOPPED == state) {
      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() STOPPED, destroy broadcast", __FUNCTION__);
      linuxbt_bms_destroy_broadcast(instance_id);
    }

    g_broadcasts_state[instance_id] = state;
  }

  void OnBroadcastSocketIndexNotify(uint8_t instance_id,
              std::vector<LeAudioSocketChannelMap> socket_index_list) override {
    tBTMW_MSG btmw_msg = {0};
    uint8_t socket_index_num = socket_index_list.size();

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d socket_index num=%d",
        __FUNCTION__, instance_id, socket_index_num);
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM);
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_SOCKET_INDEX_NOTIFY;
    btmw_msg.data.bms_event.data.socket_index.instance_id = instance_id;
    btmw_msg.data.bms_event.data.socket_index.socket_index_num = socket_index_num;
    for (uint8_t i = 0; i < socket_index_num; i++)
    {
        btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].socket_index = socket_index_list[i].socket_index;
        btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].channel_num = socket_index_list[i].channel_num;
        uint8_t subgroup_num = socket_index_list[i].subgroup_ids.size();
        btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].subgroup_num = subgroup_num;
        for (uint8_t j = 0; j < subgroup_num; j++)
        {
          btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].subgroup_ids[j] = socket_index_list[i].subgroup_ids[j];
        }
        btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].iso_status = FALSE;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() socket_index %d channel_num %d subgroup_num %d", __FUNCTION__,
            btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].socket_index,
            btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].channel_num,
            btmw_msg.data.bms_event.data.socket_index.socket_index_list[i].subgroup_num);
    }

    bt_mw_nty_send_msg(&btmw_msg);
  }

  void OnBroadcastIsoStatus(uint8_t instance_id, uint8_t socket_index, bool up) override {
    tBTMW_MSG btmw_msg = {0};

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d socket_index=%d up=%d",
                                              __FUNCTION__, instance_id, socket_index, up);
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM);
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_ISO_STATUS;
    btmw_msg.data.bms_event.data.iso_status.instance_id = instance_id;
    btmw_msg.data.bms_event.data.iso_status.socket_index = socket_index;
    btmw_msg.data.bms_event.data.iso_status.up = up;
    bt_mw_nty_send_msg(&btmw_msg);
  }

  void OnBroadcastsOwnAddress(uint8_t instance_id, uint8_t addr_type,
                                      const RawAddress& addr) override {
    tBTMW_MSG btmw_msg = {0};

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() instance_id=%d addr_type=%d", __FUNCTION__, instance_id, addr_type);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() sizeof(BT_BMS_EVENT_PARAM)=%d", __FUNCTION__, sizeof(BT_BMS_EVENT_PARAM));
    memset(&btmw_msg, 0, sizeof(btmw_msg));
    btmw_msg.hdr.event = BTMW_BMS_STATE_EVT;
    btmw_msg.hdr.len = sizeof(BT_BMS_EVENT_PARAM) + 4; //+4 is workaround to pass addr to mw
    btmw_msg.data.bms_event.event = BT_BMS_EVENT_OWN_ADDRESS;
    btmw_msg.data.bms_event.data.own_address.instance_id = instance_id;
    btmw_msg.data.bms_event.data.own_address.addr_type = addr_type;

    linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bms_event.data.own_address.addr);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() addr=%s",
        __FUNCTION__, btmw_msg.data.bms_event.data.own_address.addr);
    bt_mw_nty_send_msg(&btmw_msg);
  }
};

static LeAudioBroadcasterInterface *g_bt_bms_interface = NULL;
static LeAudioBroadcasterCallbacksImpl g_bt_bms_callbacks;

static void linuxbt_bms_covert_base_announcement_param(
    const BT_BMS_ANNOUNCEMENT& src, LeAudioBroadcastAnnouncementParam* dst)
{
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] linuxbt_bms_covert_base_announcement_param");

    dst->pbas_on = src.pbas_on;
    dst->bas_config_name = (AudioBroadcastStreamConfigSet)src.bas_config_name;
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] big bas_config_name %d", (int)dst->bas_config_name);

    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] subgroup num %d", src.subgroup_num);
    for (uint8_t i = 0; i < src.subgroup_num; i++)
    {
        LeAudioBroadcastAnnouncementSubgroupParam subgroup_param;
        subgroup_param.codec_config_name = (AudioCodecConfigSet)src.subgroup[i].codec_config_name;
        subgroup_param.audio_channel_allocation = src.subgroup[i].audio_channel_allocation;
        BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] subgroup[%d].codec_config_name %d", i, (int)subgroup_param.codec_config_name);
        BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] subgroup[%d].audio_channel_allocation 0x%08x", i, subgroup_param.audio_channel_allocation);
        if (src.subgroup[i].metadata_len > 0) {
            subgroup_param.metadata.reserve(src.subgroup[i].metadata_len);
            subgroup_param.metadata.resize(src.subgroup[i].metadata_len);
            memcpy(&subgroup_param.metadata[0], src.subgroup[i].metadata, src.subgroup[i].metadata_len);
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] subgroup[%d] metadata_len %d", i, src.subgroup[i].metadata_len);
#if 0
            for (uint8_t k = 0; k < src.subgroup[i].metadata_len; k++)
            {
                BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] metadata[%d] 0x%02x", k, subgroup_param.metadata[k]);
            }
#endif
        }
        BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] subgroup[%d] bis num %d", i, src.subgroup[i].bis_num);
        for (uint8_t j = 0; j < src.subgroup[i].bis_num; j++)
        {
            LeAudioBroadcastAnnouncementBisParam bis_param;
            bis_param.audio_channel_allocation = src.subgroup[i].bis[j].audio_channel_allocation;
            subgroup_param.bis_params.push_back(bis_param);
            BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] bis[%d].audio_channel_allocation 0x%08x", j, bis_param.audio_channel_allocation);
        }
        dst->subgroup_params.push_back(subgroup_param);
    }
}

static int linuxbt_bms_is_vaild_broadcast()
{
  for(auto const& it : g_broadcasts_state)
  {
      if (it.second != BroadcastState::STOPPED)
      {
          return 1;
      }
  }
  return 0;
}

int linuxbt_bms_init(void)
{
    FUNC_ENTRY;
    g_bt_bms_interface = (LeAudioBroadcasterInterface *)linuxbt_gap_get_profile_interface(BT_PROFILE_LE_AUDIO_BROADCASTER_ID);
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() Failed to get BMS interface\n", __FUNCTION__);
        return BT_ERR_STATUS_FAIL;
    }

    g_bt_bms_interface->Initialize(&g_bt_bms_callbacks);
    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() g_bt_bms_interface init done\n", __FUNCTION__);

    pthread_condattr_t condattr;
    if (pthread_condattr_init(&condattr) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] init cond fail\n");
    }
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&g_stop_signal, &condattr);
    pthread_mutex_init(&g_stop_lock, NULL);
    pthread_condattr_destroy(&condattr);

    return BT_STATUS_SUCCESS;
}

int linuxbt_bms_deinit(void)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() g_bt_bms_interface not init\n", __FUNCTION__);
        return BT_ERR_STATUS_NOT_READY;
    }

    struct timespec wait_time;
    INT32 wait_ret = 0;

    pthread_mutex_lock(&g_stop_lock);
    if (linuxbt_bms_is_vaild_broadcast()) {
        BT_DBG_WARNING(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() stop all broadcast first\n", __FUNCTION__);
        g_bt_bms_interface->Stop();

        memset(&wait_time, 0, sizeof(wait_time));
        clock_gettime(CLOCK_MONOTONIC, &wait_time);
        wait_time.tv_sec += LINUXBT_BMS_MAX_WRITTEN_TIME_SECOND;
        wait_ret = pthread_cond_timedwait(&g_stop_signal, &g_stop_lock, &wait_time);
        if (wait_ret == ETIMEDOUT)
        {
            BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "wait stop callback timeout!!!");
        }
    }
    pthread_mutex_unlock(&g_stop_lock);

    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() cleanup\n", __FUNCTION__);
    g_bt_bms_interface->Cleanup();

    pthread_cond_destroy(&g_stop_signal);
    pthread_mutex_destroy(&g_stop_lock);

    return BT_SUCCESS;
}

int linuxbt_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    std::array<uint8_t, 16> broadcast_code;

    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() g_bt_bms_interface not init\n", __FUNCTION__);
        return BT_ERR_STATUS_NOT_READY;
    }

    std::vector<uint8_t> localname(param->localname, param->localname + param->localname_len);
    std::vector<uint8_t> metadata(param->metadata, param->metadata + param->metadata_len);

    if (param->broadcast_code_len != 0)
    {
        std::copy(param->broadcast_code, param->broadcast_code + 16, broadcast_code.data());
        g_bt_bms_interface->CreateBroadcast(
          std::move(localname), std::move(metadata),
          (BroadcastAudioProfile)param->profile, std::move(broadcast_code));
    }
    else
    {
        g_bt_bms_interface->CreateBroadcast(
          std::move(localname), std::move(metadata),
          (BroadcastAudioProfile)param->profile, std::experimental::nullopt);
    }

    return BT_SUCCESS;
}

int linuxbt_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
    FUNC_ENTRY;
    std::array<uint8_t, 16> broadcast_code;
    LeAudioBroadcastAnnouncementParam announcement;

    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() g_bt_bms_interface not init\n", __FUNCTION__);
        return BT_ERR_STATUS_NOT_READY;
    }

    std::vector<uint8_t> localname(param->localname, param->localname + param->localname_len);

    linuxbt_bms_covert_base_announcement_param(param->announcement, &announcement);

    BT_DBG_INFO(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() broadcast_code_len %d\n",
        __FUNCTION__, param->broadcast_code_len);

    if (param->broadcast_code_len != 0)
    {
        std::copy(param->broadcast_code, param->broadcast_code + 16, broadcast_code.data());
        g_bt_bms_interface->CreateBroadcastExt(
          std::move(localname), std::move(announcement), std::move(broadcast_code));
    }
    else
    {
        g_bt_bms_interface->CreateBroadcastExt(
          std::move(localname), std::move(announcement), std::experimental::nullopt);
    }

    return BT_SUCCESS;
}

int linuxbt_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    LeAudioBroadcastAnnouncementParam announcement;
    linuxbt_bms_covert_base_announcement_param(param->announcement, &announcement);

    g_bt_bms_interface->UpdateBASEAnnouncement(param->instance_id, std::move(announcement));
    return BT_SUCCESS;
}

int linuxbt_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    std::vector<uint8_t> metadata(param->metadata, param->metadata + param->metadata_len);

    g_bt_bms_interface->UpdateSubgroupMetadata(param->instance_id, param->subgroup_id, std::move(metadata));
    return BT_SUCCESS;
}

int linuxbt_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->StartBroadcast(param->instance_id);
    return BT_SUCCESS;
}

int linuxbt_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->StartBroadcastMultiThread(param->instance_id);
    return BT_SUCCESS;
}

int linuxbt_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->PauseBroadcast(param->instance_id);
    return BT_SUCCESS;
}

int linuxbt_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->StopBroadcast(param->instance_id);
    return BT_SUCCESS;
}

int linuxbt_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(param->instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, param->instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->GetBroadcastsOwnAddress(param->instance_id);
    return BT_SUCCESS;
}

int linuxbt_bms_get_all_broadcasts_states(void)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bms_interface->GetAllBroadcastStates();
    return BT_SUCCESS;
}

int linuxbt_bms_stop_all_broadcast(void)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bms_interface->Stop();
    return BT_SUCCESS;
}

int linuxbt_bms_destroy_broadcast(UINT8 instance_id)
{
    FUNC_ENTRY;
    if (NULL == g_bt_bms_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "g_bt_bms_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (g_broadcasts_state.count(instance_id) == 0) {
      BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMS, "[BMS] %s() no such instance_id %d\n",
          __FUNCTION__, instance_id);
      return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_bms_interface->DestroyBroadcast(instance_id);
    return BT_SUCCESS;
}
#else /*MTK_LEAUDIO_BMS*/
int linuxbt_bms_init(void)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_deinit(void)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_create_broadcast(BT_BMS_CREATE_BROADCAST_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_create_broadcast_ext(BT_BMS_CREATE_BROADCAST_EXT_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_update_base_announcement(BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_update_subgroup_metadata(BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_start_broadcast(BT_BMS_START_BROADCAST_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_start_broadcast_multi_thread(BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_pause_broadcast(BT_BMS_PAUSE_BROADCAST_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_stop_broadcast(BT_BMS_STOP_BROADCAST_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_get_own_address(BT_BMS_GET_OWN_ADDRESS_PARAM *param)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_get_all_broadcasts_states(void)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_stop_all_broadcast(void)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}

int linuxbt_bms_destroy_broadcast(UINT8 instance_id)
{
  return BT_ERR_STATUS_UNSUPPORTED;
}
#endif /*MTK_LEAUDIO_BMS*/

