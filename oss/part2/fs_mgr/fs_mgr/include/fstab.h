/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef __CORE_FS_TAB_H
#define __CORE_FS_TAB_H

#include <linux/dm-ioctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <set>
#include <string>

/*
 * The entries must be kept in the same order as they were seen in the fstab.
 * Unless explicitly requested, a lookup on mount point should always
 * return the 1st one.
 */
struct fstab {
    int num_entries;
    struct fstab_rec* recs;
    char* fstab_filename;
};

struct fstab_rec {
    char* partition_name;
    char* blk_device;
    char* mount_point;
    char* fs_type;
    unsigned long flags;
    char* fs_options;
    int fs_mgr_flags;
    char* key_loc;
    char* key_dir;
    char* verity_loc;
    long long length;
    char* label;
    int partnum;
    int swap_prio;
    int max_comp_streams;
    unsigned int zram_size;
    uint64_t reserved_size;
    unsigned int file_contents_mode;
    unsigned int file_names_mode;
    unsigned int erase_blk_size;
    unsigned int logical_blk_size;
    char* sysfs_path;
};

#endif /* __CORE_FS_TAB_H */
