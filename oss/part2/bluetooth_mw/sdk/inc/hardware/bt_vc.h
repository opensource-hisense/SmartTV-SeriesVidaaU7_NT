/*
 * Copyright 2019 HIMSA II K/S - www.himsa.com.
 * Represented by EHIMA - www.ehima.com
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

namespace bluetooth {
namespace vc {

enum class ConnectionState {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED,
  DISCONNECTING
};

class VolumeControlCallbacks {
 public:
  virtual ~VolumeControlCallbacks() = default;

  /** Callback for profile connection state change */
  virtual void OnConnectionState(ConnectionState state,
                                 const RawAddress& address) = 0;

  /* Callback for the new available device */
  virtual void OnDeviceAvailable(const RawAddress& address, uint8_t num_input,
                                 uint8_t num_offset) = 0;

  virtual void OnVolumeStateChanged(const RawAddress& address, uint8_t volume,
                                    bool mute) = 0;

  /* Callbacks for Audio Input Stream (AIS) - Extended Audio Inputs */
  virtual void OnExtAudioInStateChanged(const RawAddress& address,
                                        uint8_t ext_input_id, int8_t gain_val,
                                        uint8_t gain_mode_auto, bool mute) = 0;

  virtual void OnExtAudioInStatusChanged(const RawAddress& address,
                                         uint8_t ext_input_id,
                                         uint8_t status) = 0;

  virtual void OnExtAudioInTypeChanged(const RawAddress& address,
                                       uint8_t ext_input_id, uint8_t type) = 0;

  virtual void OnExtAudioInGainPropsChanged(const RawAddress& address,
                                            uint8_t ext_input_id, uint8_t unit,
                                            int8_t min, int8_t max) = 0;

  virtual void OnExtAudioInDescriptionChanged(const RawAddress& address,
                                              uint8_t ext_input_id,
                                              std::string descr) = 0;

  /* Callbacks for Volume Offset Control Service (VOCS) - Extended Audio Outputs
   */

  virtual void OnExtAudioOutVolumeOffsetChanged(const RawAddress& address,
                                                uint8_t ext_output_id,
                                                int16_t offset) = 0;
  virtual void OnExtAudioOutLocationChanged(const RawAddress& address,
                                            uint8_t ext_output_id,
                                            uint16_t location) = 0;
  virtual void OnExtAudioOutDescriptionChanged(const RawAddress& address,
                                               uint8_t ext_output_id,
                                               std::string descr) = 0;
};

class VolumeControlInterface {
 public:
  virtual ~VolumeControlInterface() = default;

  /** Register the Volume Control callbacks */
  virtual void Init(VolumeControlCallbacks* callbacks) = 0;

  /** Connect to Volume Control */
  virtual void Connect(const RawAddress& address) = 0;

  /** Disconnect from Volume Control */
  virtual void Disconnect(const RawAddress& address) = 0;

  /** Set the volume */
  virtual void SetVolume(int8_t volume) = 0;

  /** Increase the volume */
  virtual void VolumeUp(bool unmute) = 0;

  /** Decrease the volume */
  virtual void VolumeDown(bool unmute) = 0;

  /** Mute the volume */
  virtual void Mute(void) = 0;

  /** Unmute the volume */
  virtual void Unmute(void) = 0;

  /** Closes the interface */
  virtual void Cleanup(void) = 0;

  /** Called when Volume control devices is unbonded */
  virtual void RemoveDevice(const RawAddress& address) = 0;

  virtual void GetExtAudioOutVolumeOffset(const RawAddress& address,
                                          uint8_t ext_output_id) = 0;
  virtual void SetExtAudioOutVolumeOffset(const RawAddress& address,
                                          uint8_t ext_output_id,
                                          int16_t offset_val) = 0;
  virtual void GetExtAudioOutLocation(const RawAddress& address,
                                      uint8_t ext_output_id) = 0;
  virtual void SetExtAudioOutLocation(const RawAddress& address,
                                      uint8_t ext_output_id,
                                      uint16_t location) = 0;
  virtual void GetExtAudioOutDescription(const RawAddress& address,
                                         uint8_t ext_output_id) = 0;
  virtual void SetExtAudioOutDescription(const RawAddress& address,
                                         uint8_t ext_output_id,
                                         std::string descr) = 0;

  virtual void GetExtAudioInState(const RawAddress& address,
                                  uint8_t ext_input_id) = 0;
  virtual void GetExtAudioInStatus(const RawAddress& address,
                                   uint8_t ext_input_id) = 0;
  virtual void GetExtAudioInType(const RawAddress& address,
                                 uint8_t ext_input_id) = 0;
  virtual void GetExtAudioInGainProps(const RawAddress& address,
                                      uint8_t ext_input_id) = 0;
  virtual void GetExtAudioInDescription(const RawAddress& address,
                                        uint8_t ext_input_id) = 0;
  virtual void SetExtAudioInDescription(const RawAddress& address,
                                        uint8_t ext_input_id,
                                        std::string descr) = 0;
  virtual void SetExtAudioInGainValue(const RawAddress& address,
                                      uint8_t ext_input_id, int8_t value) = 0;
  virtual void SetExtAudioInGainMode(const RawAddress& address,
                                     uint8_t ext_input_id, bool automatic) = 0;
  virtual void SetExtAudioInGainMute(const RawAddress& address,
                                     uint8_t ext_input_id, bool mute) = 0;
};

} /* namespace vc */
} /* namespace bluetooth */
