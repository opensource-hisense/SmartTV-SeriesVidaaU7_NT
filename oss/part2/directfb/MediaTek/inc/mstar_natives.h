/*------------------------------------------------------------------------------
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 *----------------------------------------------------------------------------*/

#ifndef _MSTAR_NATIVES_H_
#define _MSTAR_NATIVES_H_

#include <stdint.h>

#define MSTAR_NATIVES_MAKE_VERSION(major, minor)    (((major) << 8) | (minor))
#define MSTAR_NATIVES_VERSION                       MSTAR_NATIVES_MAKE_VERSION(1, 1)

typedef enum mst_buffer_format_t
{
                                    /* 31 ............. 16 15 .............. 0 */
    MST_BUFFER_FORMAT_ARGB8888 = 0, /* AAAA AAAA RRRR RRRR GGGG GGGG BBBB BBBB */
    MST_BUFFER_FORMAT_ABGR8888,     /* AAAA AAAA BBBB BBBB GGGG GGGG RRRR RRRR */
    MST_BUFFER_FORMAT_BGRA8888,     /* BBBB BBBB GGGG GGGG RRRR RRRR AAAA AAAA */
    MST_BUFFER_FORMAT_RGBA8888,     /* RRRR RRRR GGGG GGGG BBBB BBBB AAAA AAAA */

    MST_BUFFER_FORMAT_XRGB8888,     /* XXXX XXXX RRRR RRRR GGGG GGGG BBBB BBBB */
    MST_BUFFER_FORMAT_XBGR8888,     /* XXXX XXXX BBBB BBBB GGGG GGGG RRRR RRRR */
    MST_BUFFER_FORMAT_BGRX8888,     /* BBBB BBBB GGGG GGGG RRRR RRRR XXXX XXXX */
    MST_BUFFER_FORMAT_RGBX8888,     /* RRRR RRRR GGGG GGGG BBBB BBBB XXXX XXXX */

    MST_BUFFER_FORMAT_RGB888,       /* ---- ---- RRRR RRRR GGGG GGGG BBBB BBBB */
    MST_BUFFER_FORMAT_BGR888,       /* ---- ---- BBBB BBBB GGGG GGGG RRRR RRRR */

    MST_BUFFER_FORMAT_ARGB4444,     /* ---- ---- ---- ---- AAAA RRRR GGGG BBBB */
    MST_BUFFER_FORMAT_ABGR4444,     /* ---- ---- ---- ---- AAAA BBBB GGGG RRRR */
    MST_BUFFER_FORMAT_BGRA4444,     /* ---- ---- ---- ---- BBBB GGGG RRRR AAAA */
    MST_BUFFER_FORMAT_RGBA4444,     /* ---- ---- ---- ---- RRRR GGGG BBBB AAAA */

    MST_BUFFER_FORMAT_ARGB1555,     /* ---- ---- ---- ---- ARRR RRGG GGGB BBBB */
    MST_BUFFER_FORMAT_ABGR1555,     /* ---- ---- ---- ---- ABBB BBGG GGGR RRRR */
    MST_BUFFER_FORMAT_BGRA5551,     /* ---- ---- ---- ---- BBBB BGGG GGRR RRRA */
    MST_BUFFER_FORMAT_RGBA5551,     /* ---- ---- ---- ---- RRRR RGGG GGBB BBBA */

    MST_BUFFER_FORMAT_RGB565,       /* ---- ---- ---- ---- RRRR RGGG GGGB BBBB */
    MST_BUFFER_FORMAT_BGR565,       /* ---- ---- ---- ---- BBBB BGGG GGGR RRRR */

    MST_BUFFER_FORMAT_AL88,         /* ---- ---- ---- ---- AAAA AAAA LLLL LLLL */
    MST_BUFFER_FORMAT_LA88,         /* ---- ---- ---- ---- LLLL LLLL AAAA AAAA */
    MST_BUFFER_FORMAT_L8,           /* ---- ---- ---- ---- ---- ---- LLLL LLLL */
    MST_BUFFER_FORMAT_A8,           /* ---- ---- ---- ---- ---- ---- AAAA AAAA */
    MST_BUFFER_FORMAT_FP16,         /* ABGR16161616_FLOAT */

    MST_BUFFER_FORMAT_YUYV,         /* YUV 4:2:2 interleaved, YUY2 */
    MST_BUFFER_FORMAT_YVYU,         /* YUV 4:2:2 interleaved */
    MST_BUFFER_FORMAT_UYVY,         /* YUV 4:2:2 interleaved */
    MST_BUFFER_FORMAT_VYUY,         /* YUV 4:2:2 interleaved */

    MST_BUFFER_FORMAT_YV16,         /* YUV 4:2:2 planar (3 planes: Y, V, U) */
    MST_BUFFER_FORMAT_YV61,         /* YUV 4:2:2 planar (3 planes: Y, U, V) */

    MST_BUFFER_FORMAT_NV16,         /* YUV 4:2:2 semi-planar (2 planes: Y, U/V) */
    MST_BUFFER_FORMAT_NV61,         /* YUV 4:2:2 semi-planar (2 planes: Y, V/U) */

    MST_BUFFER_FORMAT_YV12,         /* YUV 4:2:0 planar (3 planes: Y, V, U) */
    MST_BUFFER_FORMAT_YV21,         /* YUV 4:2:0 planar (3 planes: Y, U, V) */

    MST_BUFFER_FORMAT_NV12,         /* YUV 4:2:0 semi-planar (2 planes: Y, U/V) */
    MST_BUFFER_FORMAT_NV21,         /* YUV 4:2:0 semi-planar (2 planes: Y, V/U) */

    MST_BUFFER_FORMAT_COUNT
} mst_buffer_format_t;

#define MAX_PLANE_NUM 3
#define MST_BUFFER_USE_BUS_ADDRESS               (1 << 0)
#define MST_BUFFER_USE_FILE_DESCRIPTOR           (1 << 1) /* use phys instead of fd if flag not present, phys only support single(packed) planar wrapping */
#define MST_BUFFER_FORMAT_SRGB                   (1 << 2) /* linear RGB if flag not present */
#define MST_BUFFER_FORMAT_YUV_COLOR_SPACE_BT709  (1 << 3) /* BT601 if flag not present */
#define MST_BUFFER_FORMAT_YUV_COLOR_SPACE_BT2020 (1 << 4) /* return EGL_BAD_PARAMETER if this flag is set but not supported by GPU */
#define MST_BUFFER_FORMAT_YUV_COLOR_RANGE_WIDE   (1 << 5) /* narrow range if flag not present */
#define MST_BUFFER_FORMAT_AFBC                   (1 << 6) /* normal formats if flag not present */

typedef struct mst_native_buffer_t
{
    uint32_t version;
    uint32_t flags;
    mst_buffer_format_t format;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    void* addr;
    uint64_t phys;
    int fd[MAX_PLANE_NUM];
} mst_native_buffer_t;

#endif  /* _MSTAR_NATIVES_H_ */
