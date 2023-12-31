// Copyright 2016 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STARBOARD_SHARED_STARBOARD_PLAYER_INPUT_BUFFER_INTERNAL_H_
#define STARBOARD_SHARED_STARBOARD_PLAYER_INPUT_BUFFER_INTERNAL_H_

#include <string>
#include <vector>

#include "starboard/common/ref_counted.h"
#include "starboard/drm.h"
#include "starboard/media.h"
#include "starboard/player.h"
#include "starboard/shared/internal_only.h"
#include "starboard/shared/starboard/media/media_util.h"

namespace starboard {
namespace shared {
namespace starboard {
namespace player {

// This class encapsulate a media buffer.
class InputBuffer : public RefCountedThreadSafe<InputBuffer> {
 public:
  InputBuffer(SbPlayerDeallocateSampleFunc deallocate_sample_func,
              SbPlayer player,
              void* context,
              const SbPlayerSampleInfo& sample_info);

  ~InputBuffer();

  SbMediaType sample_type() const { return sample_type_; }
  const uint8_t* data() const { return data_; }
  int size() const { return size_; }

  const std::vector<uint8_t>& side_data() const { return side_data_; }

  SbTime timestamp() const { return timestamp_; }
  const SbMediaAudioSampleInfo& audio_sample_info() const {
    SB_DCHECK(sample_type_ == kSbMediaTypeAudio);
    return audio_sample_info_;
  }
  const SbMediaVideoSampleInfo& video_sample_info() const {
    SB_DCHECK(sample_type_ == kSbMediaTypeVideo);
    return video_sample_info_;
  }
  const SbDrmSampleInfo* drm_info() const {
    return has_drm_info_ ? &drm_info_ : NULL;
  }
  void SetDecryptedContent(std::vector<uint8_t> decrypted_content);

  std::string ToString() const;

 private:
  void TryToAssignDrmSampleInfo(const SbDrmSampleInfo* sample_drm_info);
  void DeallocateSampleBuffer(const void* buffer);

  SbPlayerDeallocateSampleFunc deallocate_sample_func_;
  SbPlayer player_;
  void* context_;

  SbMediaType sample_type_;
  const uint8_t* data_;
  int size_;
  std::vector<uint8_t> side_data_;
  SbTime timestamp_;

  media::AudioSampleInfo audio_sample_info_;
  media::VideoSampleInfo video_sample_info_;

  bool has_drm_info_;
  SbDrmSampleInfo drm_info_;
  std::vector<SbDrmSubSampleMapping> subsamples_;
  std::vector<uint8_t> flattened_data_;

  InputBuffer(const InputBuffer&) = delete;
  void operator=(const InputBuffer&) = delete;
};

typedef std::vector<scoped_refptr<InputBuffer>> InputBuffers;

}  // namespace player
}  // namespace starboard
}  // namespace shared
}  // namespace starboard

#endif  // STARBOARD_SHARED_STARBOARD_PLAYER_INPUT_BUFFER_INTERNAL_H_
