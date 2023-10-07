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
#include <experimental/optional>
#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_leaudio_bass_if.h"
#include "bt_mw_message_queue.h"
#include "bluetooth.h"
#include "bt_bass.h"

using bluetooth::bass::BroadcastAudioScanInterface;
using bluetooth::bass::BroadcastAudioScanCallbacks;
using bluetooth::bass::BroadcastId;
using bluetooth::bass::BroadcastSourceData;
using bluetooth::bass::BroadcastReceiveData;
using bluetooth::bass::BasicAudioAnnouncementData;
using bluetooth::bass::BasicAudioAnnouncementCodecConfig;
using bluetooth::bass::BasicAudioAnnouncementSubgroup;
using bluetooth::bass::BasicAudioAnnouncementBisConfig;
using bluetooth::bass::BroadcastCode;
using bluetooth::bass::ConnectionState;
using bluetooth::bass::State;
using bluetooth::bass::SyncPA;

static void linuxbt_bass_covert_codec_config(const BasicAudioAnnouncementCodecConfig& src,
    BT_BASS_ANNOUNCE_CODEC_CONFIG* dst);

class BroadcastAudioScanCallbacksImpl : public BroadcastAudioScanCallbacks {
 public:
  ~BroadcastAudioScanCallbacksImpl() = default;

  void OnConnectionState(ConnectionState state, const RawAddress& addr) override {
      tBTMW_MSG btmw_msg;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, state=%d\n", addr.ToString().c_str(), (int)state);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_CONNECTION_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_CONN_STATE_DATA);
      linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bass_msg.data.conn_state.bt_addr);
      btmw_msg.data.bass_msg.data.conn_state.state = (BT_BASS_CONN_STATE)state;
      linuxbt_send_msg(&btmw_msg);
  }

  void OnDeviceAvailable(const RawAddress& addr, int num_receivers) override {
      tBTMW_MSG btmw_msg;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, num_receivers=%d\n", addr.ToString().c_str(), num_receivers);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_DEVICE_AVAIL_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_DEV_AVAIL_DATA);
      linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bass_msg.data.dev_avail.bt_addr);
      btmw_msg.data.bass_msg.data.dev_avail.num_receivers = num_receivers;
      linuxbt_send_msg(&btmw_msg);
  }

  void OnBroadcastScanningChanged(const RawAddress& on_behalf_addr, bool scan) override {
      tBTMW_MSG btmw_msg;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, scan=%d\n", on_behalf_addr.ToString().c_str(), scan);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_BROADCAST_SCANNING_CHANGED_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_SCANNING_STATE_DATA);
      linuxbt_btaddr_htos((RawAddress *)&on_behalf_addr, btmw_msg.data.bass_msg.data.scanning_state.bt_addr);
      btmw_msg.data.bass_msg.data.scanning_state.scan = scan;
      linuxbt_send_msg(&btmw_msg);
  }

  void OnBroadcastAnnouncementReceived(
      const std::vector<uint8_t> &local_name, bool encryption,
      const RawAddress& addr, const BroadcastId& broadcast_id,
      const BasicAudioAnnouncementData& broadcast_data) override {
      tBTMW_MSG btmw_msg;
      BT_BASS_ANNOUNCE_SUBGROUP *p_dst_subgroup = NULL;
      int subgroup_idx = 0;
      unsigned int bis_idx = 0;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, adv_sid=%d, subgroup size=%d\n",
          addr.ToString().c_str(), broadcast_id.adv_sid, broadcast_data.subgroup_configs.size());
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_BROADCAST_ANNOUNCE_RECV_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA);
      linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bass_msg.data.announce_received.bt_addr);

      uint8_t len = (local_name.size() < (MAX_NAME_LEN - 1)) ? local_name.size() : (MAX_NAME_LEN - 1);
      btmw_msg.data.bass_msg.data.announce_received.local_name_len = len;
      std::copy(local_name.begin(), local_name.begin() + len, btmw_msg.data.bass_msg.data.announce_received.local_name);
      btmw_msg.data.bass_msg.data.announce_received.local_name[len] = '\0';

      btmw_msg.data.bass_msg.data.announce_received.encryption = encryption;

      linuxbt_btaddr_htos((RawAddress *)&broadcast_id.broadcaster_addr.addr,
          btmw_msg.data.bass_msg.data.announce_received.broadcast_id.bt_addr);
      btmw_msg.data.bass_msg.data.announce_received.broadcast_id.addr_type = broadcast_id.broadcaster_addr.addr_type;
      btmw_msg.data.bass_msg.data.announce_received.broadcast_id.adv_sid = broadcast_id.adv_sid;
      btmw_msg.data.bass_msg.data.announce_received.announce_data.presentation_delay = broadcast_data.presentation_delay;
      btmw_msg.data.bass_msg.data.announce_received.announce_data.subgroup_size = broadcast_data.subgroup_configs.size();
      for (auto const& src_subgroup : broadcast_data.subgroup_configs)
      {
          p_dst_subgroup = &btmw_msg.data.bass_msg.data.announce_received.announce_data.subgroup_configs[subgroup_idx++];
          linuxbt_bass_covert_codec_config(src_subgroup.codec_config, &(p_dst_subgroup->codec_config));
          p_dst_subgroup->meta_data_len = src_subgroup.metadata.size();
          if (src_subgroup.metadata.size())
          {
              memcpy(p_dst_subgroup->meta_data,
                  src_subgroup.metadata.data(), src_subgroup.metadata.size());
          }
          p_dst_subgroup->bis_size = src_subgroup.bis_configs.size();
          bis_idx = 0;
          for (auto const& src_bisconfig : src_subgroup.bis_configs)
          {
              p_dst_subgroup->bis_configs[bis_idx].bis_index = src_bisconfig.bis_index;
              p_dst_subgroup->bis_configs[bis_idx].codec_param.sampling_frequency = src_bisconfig.codec_param.sampling_frequency;
              p_dst_subgroup->bis_configs[bis_idx].codec_param.frame_duration = src_bisconfig.codec_param.frame_duration;
              p_dst_subgroup->bis_configs[bis_idx].codec_param.octets_per_codec_frame = src_bisconfig.codec_param.octets_per_codec_frame;
              p_dst_subgroup->bis_configs[bis_idx].codec_param.codec_frame_blocks_per_sdu = src_bisconfig.codec_param.codec_frame_blocks_per_sdu;
              p_dst_subgroup->bis_configs[bis_idx].codec_param.audio_channel_allocation = src_bisconfig.codec_param.audio_channel_allocation;
              bis_idx++;
              if (bis_idx >= BT_BROADCAST_MAX_BIS_NUM)
              {
                  break;
              }
          }
      }

      linuxbt_send_msg(&btmw_msg);
  }

  void OnBroadcastReceiverState(
      const RawAddress& addr, int receiver_id, State state,
      const BroadcastId& broadcast_id,
      const BroadcastReceiveData& data) override {
      tBTMW_MSG btmw_msg;
      int idx = 0;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, receiver_id=%d, state=%d, adv_sid=%d\n",
          addr.ToString().c_str(), receiver_id, (int)state, broadcast_id.adv_sid);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_BROADCAST_RECV_STATE_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_RECV_STATE_DATA);
      linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bass_msg.data.recv_state.bt_addr);
      btmw_msg.data.bass_msg.data.recv_state.receiver_id = receiver_id;
      btmw_msg.data.bass_msg.data.recv_state.state = (BT_BASS_RECV_STATE)state;
      linuxbt_btaddr_htos((RawAddress *)&broadcast_id.broadcaster_addr.addr,
          btmw_msg.data.bass_msg.data.recv_state.broadcast_id.bt_addr);
      btmw_msg.data.bass_msg.data.recv_state.broadcast_id.addr_type = broadcast_id.broadcaster_addr.addr_type;
      btmw_msg.data.bass_msg.data.recv_state.broadcast_id.adv_sid = broadcast_id.adv_sid;
      btmw_msg.data.bass_msg.data.recv_state.data.num_subgroups = data.subgroup_data.size();
      for (auto const& subgroup : data.subgroup_data)
      {
          BT_BASS_BROADCAST_SRC_SUBGROUP *tmp = &btmw_msg.data.bass_msg.data.recv_state.data.subgroup[idx];
          tmp->sync_bis = subgroup.sync_bis;
          tmp->meta_data_len = subgroup.metadata.size();
          memcpy(tmp->meta_data, subgroup.metadata.data(), subgroup.metadata.size());
          idx++;
      }
      linuxbt_send_msg(&btmw_msg);
  }

  void OnBuiltinModeChanged(const RawAddress& addr, bool enable, uint32_t sync_bis) override {
      tBTMW_MSG btmw_msg;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "addr=%s, enable=%d\n", addr.ToString().c_str(), enable);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_BUILTIN_MODE_CHANGED_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_RECV_STATE_DATA);
      linuxbt_btaddr_htos((RawAddress *)&addr, btmw_msg.data.bass_msg.data.builtin_mode.bt_addr);
      btmw_msg.data.bass_msg.data.builtin_mode.enable = enable;
      btmw_msg.data.bass_msg.data.builtin_mode.sync_bis = sync_bis;
      linuxbt_send_msg(&btmw_msg);
  }

  void OnPeriodicSyncLost(const BroadcastId& broadcast_id) override {
      tBTMW_MSG btmw_msg;

      BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, addr_type=%d, adv_sid=%d\n",
                        broadcast_id.broadcaster_addr.addr.ToString().c_str(),
                        broadcast_id.broadcaster_addr.addr_type, broadcast_id.adv_sid);
      memset(&btmw_msg, 0, sizeof(btmw_msg));
      btmw_msg.hdr.event = BTMW_BASS_SYNC_LOST_CB_EVT;
      btmw_msg.hdr.len = sizeof(BT_BASS_EVENT_SYNC_LOST_DATA);
      linuxbt_btaddr_htos((RawAddress *)&broadcast_id.broadcaster_addr.addr,
          btmw_msg.data.bass_msg.data.sync_lost.broadcast_id.bt_addr);
      btmw_msg.data.bass_msg.data.sync_lost.broadcast_id.addr_type = broadcast_id.broadcaster_addr.addr_type;
      btmw_msg.data.bass_msg.data.sync_lost.broadcast_id.adv_sid = broadcast_id.adv_sid;
      linuxbt_send_msg(&btmw_msg);
  }
};

static BroadcastAudioScanInterface *g_bt_bass_interface = NULL;
static BroadcastAudioScanCallbacksImpl g_bt_bass_callbacks;

static void linuxbt_bass_covert_codec_config(const BasicAudioAnnouncementCodecConfig& src,
    BT_BASS_ANNOUNCE_CODEC_CONFIG* dst)
{
    dst->codec_id = src.codec_id;
    dst->vendor_company_id = src.vendor_company_id;
    dst->vendor_codec_id = src.vendor_codec_id;
    dst->codec_param.sampling_frequency = src.codec_param.sampling_frequency;
    dst->codec_param.frame_duration = src.codec_param.frame_duration;
    dst->codec_param.octets_per_codec_frame = src.codec_param.octets_per_codec_frame;
    dst->codec_param.codec_frame_blocks_per_sdu = src.codec_param.codec_frame_blocks_per_sdu;
    dst->codec_param.audio_channel_allocation = src.codec_param.audio_channel_allocation;
}

int linuxbt_bass_init(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "\n");
    g_bt_bass_interface = (BroadcastAudioScanInterface *)linuxbt_gap_get_profile_interface(BT_PROFILE_LE_AUDIO_BASS_CLIENT_ID);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "Failed to get BASS Client interface\n");
        return BT_ERR_STATUS_FAIL;
    }

    g_bt_bass_interface->Init(&g_bt_bass_callbacks);
    return BT_STATUS_SUCCESS;
}

int linuxbt_bass_deinit(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "\n");
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bass_interface->Cleanup();
    return BT_SUCCESS;
}

int linuxbt_bass_connect(char *bt_addr)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s\n", bt_addr);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->Connect(addr);
    return BT_SUCCESS;
}

int linuxbt_bass_disconnect(char *bt_addr)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s\n", bt_addr);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->Disconnect(addr);
    return BT_SUCCESS;
}

int linuxbt_bass_set_broadcast_scan(char *bt_addr, bool scan, UINT8 duration)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, scan=%d, duration=%d\n", bt_addr, scan, duration);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->SetBroadcastScan(addr, scan, duration);
    return BT_SUCCESS;
}

int linuxbt_bass_stop_broadcast_observing(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "\n");
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bass_interface->StopBroadcastObserving();
    return BT_SUCCESS;
}

int linuxbt_bass_get_broadcast_receiver_state(char *bt_addr, int receiver_id)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, receiver_id=%d\n", bt_addr, receiver_id);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->GetBroadcastReceiverState(addr, receiver_id);
    return BT_SUCCESS;
}

int linuxbt_bass_set_broadcast_code(char *bt_addr, int receiver_id, UINT8 *code)
{
    RawAddress addr;
    BroadcastCode broadcast_code;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, receiver_id=%d, code=%s\n", bt_addr, receiver_id, code);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if ((NULL == bt_addr) || (NULL == code))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    std::copy(code, code + 16, broadcast_code.data());
    g_bt_bass_interface->SetBroadcastCode(addr, receiver_id, std::move(broadcast_code));
    return BT_SUCCESS;
}

int linuxbt_bass_set_broadcast_source(char *bt_addr, char *adv_addr, UINT8 addr_type, UINT8 adv_sid, BASS_BROADCAST_SOURCE *data)
{
    RawAddress addr;
    BroadcastId broadcast_id;
    BroadcastSourceData src_data;
    int i = 0;

    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if ((NULL == bt_addr) || (NULL == adv_addr) || (NULL == data))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s\n", bt_addr);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "adv_addr=%s, addr_type=%d, adv_sid=%d\n", adv_addr, addr_type, adv_sid);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "sync_pa=%d, num_subgroups=%d\n", data->sync_pa, data->num_subgroups);

    RawAddress::FromString(bt_addr, addr);
    memset(&broadcast_id, 0, sizeof(broadcast_id));
    RawAddress::FromString(adv_addr, broadcast_id.broadcaster_addr.addr);
    broadcast_id.broadcaster_addr.addr_type = addr_type;
    broadcast_id.adv_sid = adv_sid;
    src_data.sync_pa = (SyncPA)data->sync_pa;
    if (data->num_subgroups)
    {
        src_data.subgroup_data.resize(data->num_subgroups);
        for (; i < data->num_subgroups; i++)
        {
            auto &subgroup = src_data.subgroup_data[i];
            subgroup.metadata.resize(data->subgroup[i].meta_data_len);
            std::copy(data->subgroup[i].meta_data, data->subgroup[i].meta_data + data->subgroup[i].meta_data_len,
                subgroup.metadata.data());
            subgroup.sync_bis = data->subgroup[i].sync_bis;
        }
    }
    g_bt_bass_interface->SetBroadcastSource(addr, std::move(broadcast_id), std::move(src_data));
    return BT_SUCCESS;
}

int linuxbt_bass_modify_broadcast_source(char *bt_addr, int receiver_id, BASS_BROADCAST_SOURCE *data)
{
    RawAddress addr;
    BroadcastSourceData src_data;
    int i = 0;

    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if ((NULL == bt_addr) || (NULL == data))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, receiver_id=%d\n", bt_addr, receiver_id);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "sync_pa=%d, num_subgroups=%d\n", data->sync_pa, data->num_subgroups);

    RawAddress::FromString(bt_addr, addr);
    src_data.sync_pa = (SyncPA)data->sync_pa;
    if (data->num_subgroups)
    {
        src_data.subgroup_data.resize(data->num_subgroups);
        for (; i < data->num_subgroups; i++)
        {
            auto &subgroup = src_data.subgroup_data[i];
            subgroup.metadata.resize(data->subgroup[i].meta_data_len);
            std::copy(data->subgroup[i].meta_data, data->subgroup[i].meta_data + data->subgroup[i].meta_data_len,
                subgroup.metadata.data());
            subgroup.sync_bis = data->subgroup[i].sync_bis;
        }
    }
    g_bt_bass_interface->ModifyBroadcastSource(addr, receiver_id, std::move(src_data));
    return BT_SUCCESS;
}

int linuxbt_bass_remove_broadcast_source(char *bt_addr, int receiver_id)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, receiver_id=%d\n", bt_addr, receiver_id);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->RemoveBroadcastSource(addr, receiver_id);
    return BT_SUCCESS;
}

int linuxbt_bass_set_builtin_mode(char *bt_addr, bool enable, UINT32 sync_bis)
{
    RawAddress addr;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BASS, "bt_addr=%s, enable=%d, sync_bis=0x%08X\n", bt_addr, enable, sync_bis);
    if (NULL == g_bt_bass_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BASS, "g_bt_bass_interface not init\n");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (NULL == bt_addr)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    RawAddress::FromString(bt_addr, addr);
    g_bt_bass_interface->SetBuiltinMode(addr, enable, sync_bis);
    return BT_SUCCESS;
}
