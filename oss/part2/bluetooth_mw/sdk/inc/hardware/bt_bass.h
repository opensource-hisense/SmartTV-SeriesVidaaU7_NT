/*
 * Copyright 2019 HIMSA II K/S - www.himsa.com. Represented by EHIMA -
 * www.ehima.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <hardware/bluetooth.h>

#include <array>
#include <experimental/optional>
#include <vector>

namespace bluetooth {
namespace bass {

/** Connection State */
enum class ConnectionState : uint8_t {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED,
  DISCONNECTING,
};

/** Broadcast Receiver State */
enum class State : uint8_t {
  IDLE = 0,
  SET_SOURCE_FAILED,
  SYNCING,
  SYNC_PA_FAILED,
  SYNC_BIS_FAILED,
  SYNC_BIS_STOPPED,
  SYNCED_TO_PA,
  BROADCAST_CODE_REQUIRED,
  BAD_BROADCAST_CODE,
  RECEIVING_BROADCAST,
};

enum class SyncPA : uint8_t {
    NOT_SYNC_PA = 0,
    SYNC_PA_PAST_AVAIL,
    SYNC_PA_PAST_UNAVAIL,
};

enum class Broadcast_LC3_Codec_Config_Mask : uint8_t {
    MASK_CODEC_CONFIG_ALL_INVAILD       = 0b00000,
    MASK_SAMPLING_FREQUENCY             = 0b00001,
    MASK_FRAME_DURATION                 = 0b00010,
    MASK_AUDIO_CHANNEL_ALLOCATION       = 0b00100,
    MASK_OCTETS_PER_CODEC_FRAME         = 0b01000,
    MASK_CODEC_FRAME_BLOCKS_PER_SDU     = 0b10000,
    MASK_CODEC_CONFIG_ALLOCATION_EXCEPT = 0b11011,
    MASK_CODEC_CONFIG_ALL_VAILD         = 0b11111,
};

struct BroadcastLC3CodecConfig {
    // Codec-Specific Configuration
    // mask: sampling_frequency | frame_duration | audio_channel_allocation | octets_per_codec_frame | codec_frame_blocks_per_sdu
    uint8_t mask = (uint8_t)Broadcast_LC3_Codec_Config_Mask::MASK_CODEC_CONFIG_ALL_INVAILD;
    uint8_t sampling_frequency;
    uint8_t frame_duration;
    uint32_t audio_channel_allocation;
    uint16_t octets_per_codec_frame;
    uint8_t codec_frame_blocks_per_sdu;
};

struct BasicAudioAnnouncementCodecConfig {
  // 5 octets for the Codec ID
  uint8_t codec_id;
  uint16_t vendor_company_id;
  uint16_t vendor_codec_id;

  // Codec params - series of LTV formatted triplets
  BroadcastLC3CodecConfig codec_param;
};

struct BasicAudioAnnouncementBisConfig {
  uint8_t bis_index;
  BroadcastLC3CodecConfig codec_param;  //only use audio_channel_allocation
};

struct BasicAudioAnnouncementSubgroup {
  // Subgroup specific codec configuration and metadata
  BasicAudioAnnouncementCodecConfig codec_config;
  std::vector<uint8_t> metadata;
  std::vector<BasicAudioAnnouncementBisConfig> bis_configs;
};

/** Basic Audio Announcement */
struct BasicAudioAnnouncementData {
  // Announcement Header fields
  uint32_t presentation_delay;

  // Subgroup specific configurations
  std::vector<BasicAudioAnnouncementSubgroup> subgroup_configs;

  bool FromRawPacket(const uint8_t* p_value, uint8_t len);
  bool ToRawPacket(std::vector<uint8_t>& data) const;
};

struct BroadcastSourceSubgroupData {
  uint32_t sync_bis;
  std::vector<uint8_t> metadata;
};

struct BroadcastSourceData {
  SyncPA sync_pa;
  std::vector<BroadcastSourceSubgroupData> subgroup_data;
};

struct BroadcastReceiveData {
  std::vector<BroadcastSourceSubgroupData> subgroup_data;

  BroadcastReceiveData(std::vector<BroadcastSourceSubgroupData> subgroup_data)
      : subgroup_data(subgroup_data){};
};

using BroadcastCode = std::array<uint8_t, 16>;

struct BroadcasterAddr {
  uint8_t addr_type;
  RawAddress addr;

  bool operator==(const BroadcasterAddr& other) const {
    return ((addr_type == other.addr_type) && (addr == other.addr));
  }

  static constexpr size_t kLength = sizeof(addr_type) + RawAddress::kLength;
};

struct BroadcastId {
  BroadcasterAddr broadcaster_addr;
  uint8_t adv_sid;

  bool operator==(const BroadcastId& other) const {
    return ((adv_sid == other.adv_sid) &&
            (broadcaster_addr == other.broadcaster_addr));
  }

  static constexpr size_t kLength = BroadcasterAddr::kLength + sizeof(adv_sid);
};

class BroadcastAudioScanCallbacks {
 public:
  virtual ~BroadcastAudioScanCallbacks() = default;

  /** Callback for profile connection state change */
  virtual void OnConnectionState(ConnectionState state,
                                 const RawAddress& addr) = 0;

  /** Callback for the new available device */
  virtual void OnDeviceAvailable(const RawAddress& addr, int num_receivers) = 0;

  /** Offload Scanning state on behalf of the receiver */
  virtual void OnBroadcastScanningChanged(const RawAddress& on_behalf_addr,
                                          bool scan) = 0;

  /** Broadcast scanning results */
  virtual void OnBroadcastAnnouncementReceived(
      const std::vector<uint8_t> &local_name, bool encryption,
      const RawAddress& addr, const BroadcastId& broadcast_id,
      const BasicAudioAnnouncementData& broadcast_data) = 0;

  /** Broadcast Receiver state */
  virtual void OnBroadcastReceiverState(
      const RawAddress& addr, int receiver_id, State state,
      const BroadcastId& broadcast_id,
      const BroadcastReceiveData& data) = 0;

  /** M: add for built-in mode  @{ */
  /** Callback for built-in mode change */
  virtual void OnBuiltinModeChanged(const RawAddress& addr, bool enable, uint32_t sync_bis) = 0;
  /** @} */

  /** M: add for synclost  @{ */
  virtual void OnPeriodicSyncLost(const BroadcastId& broadcast_id) = 0;
  /** @} */
};

class BroadcastAudioScanInterface {
 public:
  virtual ~BroadcastAudioScanInterface() = default;

  /** Register the Broadcast Audio Scan profile callbacks */
  virtual void Init(BroadcastAudioScanCallbacks* callbacks) = 0;

  /** Connect to Broadcast Receiver */
  virtual void Connect(const RawAddress& addr) = 0;

  /** Disconnect from Broadcast Receiver */
  virtual void Disconnect(const RawAddress& addr) = 0;

  /** Unpair from Broadcast Receiver */
  virtual void Forget(const RawAddress& address) = 0;

  /** Initiate Offload Scanning on behalf of the receiver.
   * Note: on_behalf_addr can be invalid */
  virtual void SetBroadcastScan(const RawAddress& on_behalf_addr,
                                bool scan, uint8_t duration) = 0;

  /* Stop all the observed broadcasts */
  virtual void StopBroadcastObserving(void) = 0;

  /** Get the Broadcast Receiver state */
  virtual void GetBroadcastReceiverState(const RawAddress& addr,
                                         int receiver_id) = 0;

  /** Set the Broadcast Code */
  virtual void SetBroadcastCode(const RawAddress& addr, int receiver_id,
                                const BroadcastCode& broadcast_code) = 0;

  /** Set the Broadcast source */
  virtual void SetBroadcastSource(const RawAddress& addr, const BroadcastId& broadcast_id,
                                const BroadcastSourceData& data) = 0;

  /** Set sync and metadata */
  virtual void ModifyBroadcastSource(const RawAddress& addr, int receiver_id,
                                const BroadcastSourceData& data) = 0;

  /** M: add function to remove source and set built-in mode  @{ */
  /** Remove the Broadcast source */
  virtual void RemoveBroadcastSource(const RawAddress& addr, int receiver_id) = 0;

  /** Enable/disable Built-in BMS mode, if enable, auto sync local BMS source without scanning */
  virtual void SetBuiltinMode(const RawAddress& addr, bool enable, uint32_t sync_bis) = 0;
  /** @} */

  /** Called when Broadcast Scan Profile is unbonded */
  virtual void RemoveDevice(const RawAddress& addr) = 0;

  /** Closes the interface */
  virtual void Cleanup(void) = 0;
};

} /* namespace bass */
} /* namespace bluetooth */
