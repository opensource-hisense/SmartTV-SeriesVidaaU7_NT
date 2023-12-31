// Copyright 2020 The Cobalt Authors. All Rights Reserved.
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

#ifndef THIRD_PARTY_CRASHPAD_SNAPSHOT_MODULE_SNAPSHOT_EVERGREEN_H_
#define THIRD_PARTY_CRASHPAD_SNAPSHOT_MODULE_SNAPSHOT_EVERGREEN_H_

#include <stdint.h>
#include <sys/types.h>

#include <map>
#include <string>
#include <vector>

#include "base/macros.h"
#include "client/crashpad_info.h"
#include "snapshot/crashpad_info_client_options.h"
#include "snapshot/crashpad_types/crashpad_info_reader.h"
#include "snapshot/elf/elf_image_reader.h"
#include "snapshot/module_snapshot.h"
#include "util/misc/initialization_state_dcheck.h"

namespace crashpad {

namespace internal {

//! \brief A ModuleSnapshot of a code module (binary image) loaded into a
//!     running (or crashed) process on a system that uses ELF modules.
class ModuleSnapshotEvergreen final : public ModuleSnapshot {
 public:
  //! \param[in] name The pathname used to load the module from disk.
  //! \param[in] elf_reader An image reader for the module.
  //! \param[in] type The module's type.
  //! \param[in] process_memory_range A memory reader giving protected access
  //!     to the target process.
  //! \param[in] process_memory A memory reader for the target process which can
  //!     be used to initialize a MemorySnapshot.
  ModuleSnapshotEvergreen(const std::string& name,
                          ModuleSnapshot::ModuleType type,
                          uint64_t address,
                          uint64_t size,
                          std::vector<uint8_t> build_id);
  ~ModuleSnapshotEvergreen() override;

  //! \brief Initializes the object.
  //!
  //! \return `true` if the snapshot could be created, `false` otherwise with
  //!     an appropriate message logged.
  bool Initialize();

  //! \brief Returns options from the module’s CrashpadInfo structure.
  //!
  //! \param[out] options Options set in the module’s CrashpadInfo structure.
  //! \return `true` if there were options returned. Otherwise `false`.
  bool GetCrashpadOptions(CrashpadInfoClientOptions* options);

  // ModuleSnapshot:

  std::string Name() const override;
  uint64_t Address() const override;
  uint64_t Size() const override;
  time_t Timestamp() const override;
  void FileVersion(uint16_t* version_0,
                   uint16_t* version_1,
                   uint16_t* version_2,
                   uint16_t* version_3) const override;
  void SourceVersion(uint16_t* version_0,
                     uint16_t* version_1,
                     uint16_t* version_2,
                     uint16_t* version_3) const override;
  ModuleType GetModuleType() const override;
  void UUIDAndAge(crashpad::UUID* uuid, uint32_t* age) const override;
  std::string DebugFileName() const override;
  std::vector<uint8_t> BuildID() const override;
  std::vector<std::string> AnnotationsVector() const override;
  std::map<std::string, std::string> AnnotationsSimpleMap() const override;
  std::vector<AnnotationSnapshot> AnnotationObjects() const override;
  std::set<CheckedRange<uint64_t>> ExtraMemoryRanges() const override;
  std::vector<const UserMinidumpStream*> CustomMinidumpStreams() const override;

 private:
  std::string name_;
  uint64_t address_;
  uint64_t size_;
  std::vector<uint8_t> build_id_;
  ModuleType type_;
  InitializationStateDcheck initialized_;

  std::unique_ptr<CrashpadInfoReader> crashpad_info_;

  // Too const-y: https://crashpad.chromium.org/bug/9.
  mutable std::vector<std::unique_ptr<const UserMinidumpStream>> streams_;

  DISALLOW_COPY_AND_ASSIGN(ModuleSnapshotEvergreen);
};

}  // namespace internal
}  // namespace crashpad

#endif  // THIRD_PARTY_CRASHPAD_SNAPSHOT_MODULE_SNAPSHOT_EVERGREEN_H_
