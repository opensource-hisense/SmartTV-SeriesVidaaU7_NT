/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "squashfs_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/kernel.h>


#ifdef SQUASHFS_NO_KLOG
#include <stdio.h>
#define ERROR(x...)   fprintf(stderr, x)
#else
#define ERROR(x...)   KLOG_ERROR("squashfs_utils", x)
#endif

#define SQUASHFS_MAGIC                  0x73717368

struct squashfs_super_block {
        __le32                  s_magic;
        __le32                  inodes;
        __le32                  mkfs_time;
        __le32                  block_size;
        __le32                  fragments;
        __le16                  compression;
        __le16                  block_log;
        __le16                  flags;
        __le16                  no_ids;
        __le16                  s_major;
        __le16                  s_minor;
        __le64                  root_inode;
        __le64                  bytes_used;
        __le64                  id_table_start;
        __le64                  xattr_table_start;
        __le64                  inode_table_start;
        __le64                  directory_table_start;
        __le64                  fragment_table_start;
        __le64                  lookup_table_start;
};

size_t squashfs_get_sb_size()
{
    return sizeof(struct squashfs_super_block);
}

int squashfs_parse_sb_buffer(const void *buf, struct squashfs_info *info)
{
    const struct squashfs_super_block *sb =
        (const struct squashfs_super_block *)buf;

    if (sb->s_magic != SQUASHFS_MAGIC) {
        return -1;
    }

    info->block_size = sb->block_size;
    info->inodes = sb->inodes;
    info->bytes_used = sb->bytes_used;
    // by default mksquashfs pads the filesystem to 4K blocks
    info->bytes_used_4K_padded =
        sb->bytes_used + (4096 - (sb->bytes_used & (4096 - 1)));

    return 0;
}
