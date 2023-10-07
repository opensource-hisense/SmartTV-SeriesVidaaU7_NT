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

#include <array>
#include <vector>
#include <map>
#include <experimental/optional>

#include "raw_address.h"
#define BIS_SUPPORT_NUM (2)

namespace bluetooth {
namespace le_audio {

enum class ConnectionState {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED,
  DISCONNECTING
};

enum class GroupStatus {
  IDLE = 0,
  STREAMING,
  SUSPENDED,
  RECONFIGURED,
  DESTROYED,
};

enum class GroupNodeStatus {
  ADDED = 1,
  REMOVED,
};

enum class GroupLockStatus {
  SUCCESS = 0,
  FAILED_INVALID_GROUP,
  FAILED_GROUP_EMPTY,
  FAILED_GROUP_NOT_CONNECTED,
  FAILED_LOCKED_BY_OTHER,
  FAILED_OTHER_REASON,
};

class LeAudioClientCallbacks {
 public:
  virtual ~LeAudioClientCallbacks() = default;

  /** Callback for profile connection state change */
  virtual void OnConnectionState(ConnectionState state,
                                 const RawAddress& address) = 0;

  /* Callback with group status update */
  virtual void OnGroupStatus(uint8_t group_id, GroupStatus group_status,
                             uint8_t group_flags) = 0;

  /* Callback with node status update */
  virtual void OnGroupNodeStatus(const RawAddress& bd_addr, uint8_t group_id,
                                 GroupNodeStatus node_status) = 0;

  /* Callback for newly recognized or reconfigured existing le audio device */
  virtual void OnAudioConf(const RawAddress& addr, uint8_t direction,
                           uint8_t group_id, uint32_t snk_audio_location,
                           uint32_t src_audio_location) = 0;

  /* Callback for available set member  */
  virtual void OnSetMemberAvailable(const RawAddress& address,
                                    uint8_t group_id) = 0;

  /* Callback for lock changed in the group */
  virtual void OnGroupLockChanged(uint8_t group_id, bool locked,
                                  GroupLockStatus status) = 0;
};

class LeAudioClientInterface {
 public:
  virtual ~LeAudioClientInterface() = default;

  /* Register the LeAudio callbacks */
  virtual void Initialize(LeAudioClientCallbacks* callbacks) = 0;

  /** Connect to LEAudio */
  virtual void Connect(const RawAddress& address) = 0;

  /** Disconnect from LEAudio */
  virtual void Disconnect(const RawAddress& address) = 0;

  /* Cleanup the LeAudio */
  virtual void Cleanup(void) = 0;

  /* Attach le audio node to group */
  virtual void GroupAddNode(uint8_t group_id, const RawAddress& addr) = 0;

  /* Detach le audio node from a group */
  virtual void GroupRemoveNode(uint8_t group_id, const RawAddress& addr) = 0;

  /* Request to stream audio */
  virtual void GroupStream(uint8_t group_id, uint16_t content_type) = 0;

  /* Request to suspend audio */
  virtual void GroupSuspend(uint8_t group_id) = 0;

  /* Request to stop streaming audio */
  virtual void GroupStop(uint8_t group_id) = 0;

  /* Destroy le audio group */
  virtual void GroupDestroy(uint8_t group_id) = 0;

  /* Lock/Unlock le audio group */
  virtual void GroupLockSet(uint8_t group_id, bool lock) = 0;
};

static constexpr uint8_t INSTANCE_ID_UNDEFINED = 0xFF;

/* Represents the broadcast source state. */
enum class BroadcastState {
  STOPPED = 0,
  PAUSED,
  STREAMING,
};

/* A general hint for the codec configuration process. */
enum class BroadcastAudioProfile {
  SONIFICATION = 0,
  MEDIA,
};

enum class AudioBroadcastStreamConfigSet {
    BASE_CONFIG_16_2_1_LL = 1,  //16_2_1 LOW_LATENCY
    BASE_CONFIG_16_2_2_HR = 2,  //16_2_2 HIGH_RELIABILITY (SONIFICATION_DEFAULT)
    BASE_CONFIG_48_1_1_LL = 3,  //48_1_1 LOW_LATENCY
    BASE_CONFIG_48_1_2_HR = 4,  //48_1_2 HIGH_RELIABILITY
    BASE_CONFIG_48_2_1_LL = 5,  //48_2_1 LOW_LATENCY
    BASE_CONFIG_48_2_2_HR = 6,  //48_2_2 HIGH_RELIABILITY (MEDIA_DEFAULT)
    BASE_CONFIG_48_3_1_LL = 7,  //48_3_1 LOW_LATENCY
    BASE_CONFIG_48_3_2_HR = 8,  //48_3_2 HIGH_RELIABILITY
    BASE_CONFIG_48_4_1_LL = 9,  //48_4_1 LOW_LATENCY
    BASE_CONFIG_48_4_2_HR = 10, //48_4_2 HIGH_RELIABILITY
    BASE_CONFIG_48_5_1_LL = 11, //48_5_1 LOW_LATENCY
    BASE_CONFIG_48_5_2_HR = 12, //48_5_2 HIGH_RELIABILITY
    BASE_CONFIG_48_6_1_LL = 13, //48_6_1 LOW_LATENCY
    BASE_CONFIG_48_6_2_HR = 14, //48_6_2 HIGH_RELIABILITY
    BASE_CONFIG_24_2_1_LL = 15, //24_2_1 LOW_LATENCY
    BASE_CONFIG_24_2_2_HR = 16, //24_2_2 HIGH_RELIABILITY
};

enum class AudioCodecConfigSet {
    CODEC_CONFIG_NOT_SET = 0,
    CODEC_CONFIG_16_2 = 1,  //SONIFICATION_DEFAULT
    CODEC_CONFIG_48_1 = 2,
    CODEC_CONFIG_48_2 = 3, //MEDIA_DEFAULT
    CODEC_CONFIG_48_3 = 4,
    CODEC_CONFIG_48_4 = 5,
    CODEC_CONFIG_48_5 = 6,
    CODEC_CONFIG_48_6 = 7,
    CODEC_CONFIG_24_2 = 8,
};

struct LeAudioBroadcastAnnouncementBisParam {
    uint32_t audio_channel_allocation = 0xFFFF; //bitfield mask, default 0xFFFF means not set
};

struct LeAudioBroadcastAnnouncementSubgroupParam {
    AudioCodecConfigSet codec_config_name;
    uint32_t audio_channel_allocation = 0xFFFF; //bitfield mask, default 0xFFFF means not set
    std::vector<uint8_t> metadata;
    std::vector<LeAudioBroadcastAnnouncementBisParam> bis_params;
};

struct LeAudioBroadcastAnnouncementParam {
    bool pbas_on;
    AudioBroadcastStreamConfigSet bas_config_name;
    std::vector<LeAudioBroadcastAnnouncementSubgroupParam> subgroup_params;
};

struct LeAudioSocketChannelMap {
    uint8_t socket_index;
    uint8_t channel_num;
    std::vector<uint8_t> subgroup_ids;
};

/* Represents the Le Audio getting stream data path type */
enum class LeAudioDataPathType : uint8_t {
  LE_AUIDO_DATA_PATH_ONE = 0,
  LE_AUIDO_DATA_PATH_MULTI,
};

struct LeAudioBigDataPathConfig {
  std::vector<uint8_t> subgroup;
  LeAudioDataPathType type;
};

struct LeAudioDataPathConfig {
  std::map<uint8_t, LeAudioDataPathType> instance_cfg;
  LeAudioDataPathType type;
};

class LeAudioBroadcasterCallbacks {
 public:
  virtual ~LeAudioBroadcasterCallbacks() = default;

  /* Callback for the newly created broadcast event. */
  virtual void OnBroadcastCreated(uint8_t instance_id, bool success) = 0;

  /* Callback for the destroyed broadcast event. */
  virtual void OnBroadcastDestroyed(uint8_t instance_id) = 0;

  /* Callback for the broadcast source state event. */
  virtual void OnBroadcastStateChanged(uint8_t instance_id,
                                       BroadcastState state) = 0;

  /* Callback for the broadcast socket index notify event. */
  virtual void OnBroadcastSocketIndexNotify(uint8_t instance_id,
          std::vector<LeAudioSocketChannelMap> socket_index_list) = 0;

  /* Callback for the broadcast all iso in one handler setup(up=true) or removed(up=false). */
  virtual void OnBroadcastIsoStatus(uint8_t instance_id, uint8_t socket_index, bool up) = 0;

  /* Callback for the periodic broadcast source address change event. */
  virtual void OnBroadcastsOwnAddress(uint8_t instance_id, uint8_t address_type,
                                      const RawAddress& addr) = 0;
};

class LeAudioBroadcasterInterface {
 public:
  virtual ~LeAudioBroadcasterInterface() = default;

  /* Register the LeAudio Broadcaster callbacks */
  virtual void Initialize(LeAudioBroadcasterCallbacks* callbacks) = 0;

  /* Stop the LeAudio Broadcaster and all active broadcasts */
  virtual void Stop(void) = 0;

  /* Cleanup the LeAudio Broadcaster */
  virtual void Cleanup(void) = 0;

  /* Create Broadcast instance */
  virtual void CreateBroadcast(
      std::vector<uint8_t> local_name, std::vector<uint8_t> metadata,
      BroadcastAudioProfile profile,
      std::experimental::optional<std::array<uint8_t, 16>> broadcast_code) = 0;

  /* Create Broadcast instance*/
  virtual void CreateBroadcastExt(
      std::vector<uint8_t> local_name, LeAudioBroadcastAnnouncementParam param,
      std::experimental::optional<std::array<uint8_t, 16>> broadcast_code) = 0;

  /* Update the ongoing Broadcast BASE Announcement */
  virtual void UpdateBASEAnnouncement(uint8_t instance_id,
                              LeAudioBroadcastAnnouncementParam param) = 0;

  /* Update the ongoing subgroup Broadcast metadata */
  virtual void UpdateSubgroupMetadata(uint8_t instance_id, uint8_t subgroup_id,
                              std::vector<uint8_t> metadata) = 0;


  /* Start the existing Broadcast stream */
  virtual void StartBroadcast(uint8_t instance_id) = 0;

  /* Start the existing Broadcast stream using multi thread */
  virtual void StartBroadcastMultiThread(uint8_t instance_id) = 0;

  /* Pause the ongoing Broadcast stream */
  virtual void PauseBroadcast(uint8_t instance_id) = 0;

  /* Stop the Broadcast (no stream, no periodic advertisements */
  virtual void StopBroadcast(uint8_t instance_id) = 0;

  /* Destroy the existing Broadcast instance */
  virtual void DestroyBroadcast(uint8_t instance_id) = 0;

  /* Get Broadcasts own address */
  virtual void GetBroadcastsOwnAddress(uint8_t instance_id) = 0;

  /* Get all broadcast states */
  virtual void GetAllBroadcastStates(void) = 0;
};


enum LeAudioBroadcastSourceState : uint8_t {
    IDLE = 0,                  //PA is not synced
    PA_SYNCING = 1,            //start to sync PA
    PA_SYNCED = 2,             //PA is synced
    READY_FOR_STREAMING = 3,   //BIGInfo is available, for encrypted source means(BIG_INFO_AVAILABLE and broadcast_code is known), For unencrypted source means BIG_INFO_AVAILABLE
    STREAM_STARTING = 4,       //Start the streaming-start procedure
    STREAM_STARTED = 5,        //BIS packet is received
    STREAM_STOPPING = 6,       //Start the streaming-stop procedure, if success, the NEXT state shall goto READY_FOR_STREAMING, if fail, could be IDLE(sync fail)
};

enum BroadcastReceiverStreamingEvent : uint8_t {
    STREAMING_START_SUCCESS = 0,
    STREAMING_START_FAIL,
    STREAMING_STOP_SUCCESS,
    STREAMING_STOP_FAIL,
};

enum SolicitationRequestEvent : uint8_t {
    SOLICITATION_REQUEST_STOPPED = 0,
    SOLICITATION_REQUEST_STARTED = 1,
};

enum BroadcastReceiverErrorCode : uint8_t {
    SUCCESS = 0,
    PA_SYNC_FAIL,
    INVALID_BASE,
    BIG_SYNC_FAIL,
    DATA_PATH_SETUP_FAIL,
    PA_SYNC_LOST,
    BIG_SYNC_LOST,
    DECRYPTION_ERROR,
    UNKNOWN_SOURCE,
    INVALID_PARAMS,
    INVALID_STATE,
    BUSY,
    //TODO add more
};

struct CodecSpecificConfiguration_t {
    uint8_t sampling_freq;
    uint8_t frame_duration;
    uint32_t channel_allocation;
    uint16_t octets_per_codec_frame;
    uint8_t  blocks_per_sdu;
};

struct MetaData_t {
    uint16_t prefered_audio_contexts;
    uint16_t streaming_audio_contexts;
    std::vector<uint8_t> vendor_specific_metadata;
};

struct BasicAudioAnnouncementSubgroupCodecConfig_t {
    uint32_t offset_in_BASE;    //data offset in BASE
    uint8_t codec_format;
    uint16_t company_id;
    uint16_t vendor_codec_id;
    uint8_t codec_specific_configuration_length;
    CodecSpecificConfiguration_t codec_configs;
};

struct BasicAudioAnnouncementSubgroupMetadata_t {
    uint32_t offset_in_BASE;    //data offset in BASE
    uint8_t metadata_length;
    MetaData_t metadata;
    std::vector<uint8_t> raw;
    void Clear() {
        offset_in_BASE = 0;
        metadata_length = 0;
        metadata.prefered_audio_contexts = 0;
        metadata.streaming_audio_contexts = 0;
        metadata.vendor_specific_metadata.clear();
        raw.clear();
    }
};

struct BasicAudioAnnouncementBisConfig_t {
    uint32_t offset_in_BASE;    //data offset in BASE
    uint8_t bis_index;
    uint8_t codec_specific_configuration_length;
    CodecSpecificConfiguration_t codec_configs;
    //extra fields for BIS control
    uint8_t b_renderable;   //codec config support or not
    uint8_t sync_order;     //the BIS index order in HCI_LE_BIG_Create_Sync_Command. start from 1, 0 means this BIS is not in the create sync bis list
    uint16_t conn_handle;   //get from BIG sync established event
    uint8_t state;  //BIS state may be same or a sub state of BIG
};

struct BasicAudioAnnouncementSubgroup_t {
    uint32_t offset_in_BASE;    //data offset in BASE
    uint8_t num_bis;
    BasicAudioAnnouncementSubgroupCodecConfig_t codec_configuration;
    BasicAudioAnnouncementSubgroupMetadata_t metadata;
    std::vector<BasicAudioAnnouncementBisConfig_t> bis_config;
    void Clear() {
        offset_in_BASE = 0;
        num_bis = 0;
        memset(&codec_configuration, 0, sizeof(BasicAudioAnnouncementSubgroupCodecConfig_t));
        metadata.Clear();
        bis_config.clear();
    }
};

struct BasicAudioAnnouncement_t {
    uint32_t presentation_delay;
    uint8_t num_subgroups;
    std::vector<BasicAudioAnnouncementSubgroup_t> subgroup_config;
    std::vector<uint8_t> raw; //raw data of BASE
};

struct LeAudioBroadcastAudioSource {
    uint8_t source_id;
    uint32_t broadcast_id;
    uint8_t encryption;
    uint8_t source_addr_type;
    RawAddress source_addr;
    uint8_t adv_sid;
    uint8_t state;
    uint32_t bis_sync_req_mask;
    BasicAudioAnnouncement_t BASE;
};

struct BroadcastReceiverSocketIndexMap{
    bool in_use;
    uint8_t bis_idx;
    uint8_t socket_idx;
};

struct BroadcastReceiverStreamingEventData {
    uint8_t source_id;
    uint8_t bis_num;// 1 or 2 for sync bis
    BroadcastReceiverSocketIndexMap simap[BIS_SUPPORT_NUM];
};

enum LeAudioBroadcastAudioSourceAction {
    ACT_ADD = 0,
    ACT_UPDATE = 1,
    ACT_REMOVE = 2,
};

struct LeAudioBroadcastReceiverInitParams {
    uint32_t audio_locations; //bit mask of the audio channels local support
    //add more if needed
};

class LeAudioBroadcastReceiverCallbacks {
 public:
  virtual ~LeAudioBroadcastReceiverCallbacks() = default;

  /* Connection state change event about a specific BSA device */
  virtual void OnConnectionState(ConnectionState state,
                                 const RawAddress& address) = 0;

  /* BMR(Scan Delegator) solicitation request state*/
  virtual void OnSolicitationRequestState(uint8_t state) = 0;

  /* Remote Scan event(started/stopped) about a spedific BSA device or by self(address = 0) */
  virtual void OnRemoteScanEvent(uint8_t event, const RawAddress& address) = 0;

  /* Report a audio source which should have been synced with BIS info available */
  virtual void OnAudioSourceInfo(uint8_t action, const LeAudioBroadcastAudioSource& source) = 0;

  /* Source state change event, reason(Sync loss, user stop, etc...)*/
  virtual void OnSourceStateChange(uint8_t source_id, uint8_t new_state, uint8_t reason) = 0;

  /* Source streaming event*/
  virtual void OnStreamingEvent(const BroadcastReceiverStreamingEventData& data , uint8_t event, uint8_t reason) = 0;

  /* BMR close procedure done*/
  virtual void OnCloseDone(void) = 0;
};

class LeAudioBroadcastReceiverInterface {
 public:

  virtual ~LeAudioBroadcastReceiverInterface() = default;

  virtual void Initialize(
      bluetooth::le_audio::LeAudioBroadcastReceiverCallbacks* callbacks) = 0;
  virtual void Cleanup(void) = 0;

  /* Enable/disable auto scan mode*/
  virtual void Discover(uint8_t op_code, uint32_t scan_duration, std::vector<uint32_t> blacklist) = 0;

  /*Set to be discoverable&connectable or non-discoverable&connectable*/
  virtual void SolicitationRequest(uint8_t op_code, uint32_t adv_duration, std::vector<uint8_t> user_data) = 0;

  /* Disconnect from LEAudio BSA */
  virtual void Disconnect(const RawAddress& address) = 0;

  /* In case that the audio source sync is lost, We shall create the sync first to get the fresh source info and then start streaming*/
  virtual void SourceRefresh(uint8_t source_id) = 0;

  /* In case that user request to remove or clear sources*/
  virtual void SourceRemove(bool all, uint8_t source_id) = 0;

  /* user set broadcast code for an encrypted source instead of BSA*/
  virtual void SetBroadcastCode(uint8_t source_id, std::vector<uint8_t> broadcast_code) = 0;

  /* start to streaming specific BISes*/
  virtual void StreamingStart(uint8_t source_id, uint32_t bis_mask) = 0;

  /* stop streaming specific BISes*/
  virtual void StreamingStop(uint8_t source_id, uint32_t bis_mask) = 0;

  /*Set the value of public audio capabilites config*/
  virtual void SetPACConfig(uint8_t pac_type, uint32_t value) = 0;

  /* no self-scan, no streaming, remove all the current source, may be disoverable&connectable*/
  virtual void Close(bool open_to_bsa) = 0;

  /* dump info for debug*/
  virtual void Dump(bool all, uint8_t source_id, bool full_info) = 0;
};

} /* namespace le_audio */
} /* namespace bluetooth */
