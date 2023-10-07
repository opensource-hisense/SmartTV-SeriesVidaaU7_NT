// <MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
// <MStar Software>

#ifndef __MSTAR__TYPES_H__
#define __MSTAR__TYPES_H__

#include <core/layers.h>

#include "mtk-v4l2-g2d.h"
//#include "videodev2.h"

// add drm for sti.
#include "xf86drm.h"
#include "xf86drmMode.h"

#define MSTARGFX_MAX_LAYER_BUFFER        24
#define MAX_PALETTE_ENTRY_CNT  256
#define MAX_PALETTE_INTENSITY_I4_CNT  16
#define MAX_PALETTE_INTENSITY_I2_CNT  4
#define MAX_PALETTE_INTENSITY_I1_CNT  2

#define MSTAR_MAX_GOP_COUNT 6
#define MSTAR_MAX_OUTPUT_LAYER_COUNT MSTAR_MAX_GOP_COUNT

typedef enum {
    SLF_SHADOW_LAYER_BOOLEAN    =  0x80000000,
    SLF_SHADOW_LAYER_INDEX0     =  0X00000001,
    SLF_SHADOW_LAYER_INDEX1     =  0X00000002,
    SLF_SHADOW_LAYER_INDEX2     =  0X00000004,
    SLF_SHADOW_LAYER_INDEX3     =  0X00000008,
    SLF_SHADOW_LAYER_INDEXALL   =  0X0000000F,
}ShadowLayerFlag;


typedef struct {
     u32  Hstart;
     u32  Vstart;
     u32  width;
     u32  height;
}ScreenSize;


typedef struct {
     u8            layer_index;
     u8            gop_index;
     u8            gop_index_r;
     u8            gop_dst;
     ScreenSize    screen_size;
     FusionReactor      *reactor;   /* event dispatcher */
     FusionCall         call;   /* driver call via master */
     u8                 VideoFiledMode;
     ShadowLayerFlag    ShadowFlags; /*if the layer has shadow layer, show all the shadow layers*/
     int                layer_displayMode;
     u8                 GopDstMode;
} MSTARLayerData;

typedef struct {
     int                      magic;
     CoreLayerRegionConfig    config;
     CoreLayerRegionConfigFlags config_dirtyFlag;   // which configuration need to update to hw
} MSTARRegionData;

typedef struct {
     //FusionSHMPoolShared *shmpool;

     bool                 mirror_outputs;
     bool                 clone_outputs;
     bool                 multihead;
     int                  plane_limit;

     char                 device_name[256];

     DFBRectangle         primary_rect;

     u32                  primary_fb;

     drmModeModeInfo      mode;
     //DFBDimension         primary_dimension[8];

     int                  enabled_crtcs;

     uint32_t             cloned_connectors[8];
     int                  cloned_count;


     // define for drm.
     
     int                 drm_fd;

     drmModeRes          *resources;
     drmModePlaneRes     *plane_resources;

     uint32_t property_crtc_id;
     uint32_t property_fb_id;
     uint32_t property_crtc_x_id;
     uint32_t property_crtc_y_id;
     uint32_t property_crtc_w_id;
     uint32_t property_crtc_h_id;
     uint32_t property_src_x_id;
     uint32_t property_src_y_id;
     uint32_t property_src_w_id;
     uint32_t property_src_h_id;
     uint32_t property_hstretch_id;
     uint32_t property_vstretch_id;
     uint32_t property_H_Mirror_id;
     uint32_t property_V_Mirror_id;
     uint32_t property_out_fence_id;

     uint32_t crtc_id;
	 
     uint32_t plane_idx;
     uint32_t plane_num;
     uint32_t afbc_idx;
} MTKDRMData;

typedef struct {
    CoreSurface              *pCoDFBCoreSurface;
    CoreSurfaceBuffer        *pCoDFBBuffer;
    hal_phy                physAddr;
    u16                      u16SlotUsed;
    DFBSurfacePixelFormat    format;
    unsigned long            pitch;
    u32  size;
    u8 layer_index;

    /* for drm flip */
    u32 fb_id;
}LayerBufferInfo;


typedef struct {
     int                      lcd_width;
     int                      lcd_height;
     int                      lcd_offset;
     int                      lcd_pitch;
     int                      lcd_size;
     DFBSurfacePixelFormat    lcd_format;

     /* locking */
     FusionSkirmish           beu_lock;

     bool                           b_hwclip;

     /* gop */
     int                     gfx_gop_index;
     int                     gfx_h_offset;
     int                     gfx_v_offset;
     int                     gfx_max_width;
     int                     gfx_max_height;

     int                     layer_refcnt[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     int                     layer_zorder[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     bool                    layer_active[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     //int                     layer_gwin_id[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     //int                     layer_gwin_id_r[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     bool                    layer_pinpon_mode[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     ScreenSize              layer_screen_size[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     FusionReactor           *reactor[MSTAR_MAX_OUTPUT_LAYER_COUNT];       /* event dispatcher */

     bool                   a8_palette;
     struct v4l2_argb_color      palette_tbl[MAX_PALETTE_ENTRY_CNT];
     int                    num_entries;

     struct v4l2_g2d_intensity      palette_intensity[MAX_PALETTE_INTENSITY_I4_CNT];
     int                    num_intensity;

     //bool                   GOP_support_palette[MSTAR_MAX_OUTPUT_LAYER_COUNT];
	 
     LayerBufferInfo  mstarLayerBuffer[MSTARGFX_MAX_LAYER_BUFFER];
} MSTARDeviceData;

typedef struct {
    u16 tagID;
    hal_phy CurPhysAddr;
} FlipInfo;

typedef struct {
     MSTARDeviceData        *dev;

     u32                      ge_src_colorkey_enabled       : 1;
     u32                      ge_draw_dst_colorkey_enabled  : 1;
     u32                      ge_blit_dst_colorkey_enabled  : 1;
     u32                      ge_draw_alpha_blend_enabled   : 1;
     u32                      ge_blit_alpha_blend_enabled   : 1;
     u32                      ge_palette_enabled            : 1;
     u32                      ge_blit_nearestmode_enabled   : 1;
     u32                      ge_render_alpha_cmp_enabled   : 1;
     u32                      ge_blit_xmirror               : 1;
     u32                      ge_blit_ymirror               : 1;
     u32                      ge_palette_intensity_enabled  : 1;
     u32                      ge_src_rgb32_blit_enabled     : 1;
     u32                      ge_dest_rgb32_blit_enabled    : 1;
     u32                      ge_rop_enable                 : 1;
     u32                      ge_dither_enable              : 1;
     u32                      reserved_mask                 :17;


     /* cached values */
     hal_phy              dst_phys;
     int                      dst_pitch;
     int                      dst_bpp;
     int                      dst_index;
     unsigned int             dst_ge_format;
     unsigned int             dst_w;
     unsigned int             dst_h;

     u32             src_ge_clr_key;
     u32             dst_ge_clr_key;

     enum v4l2_g2d_yuv_422     src_ge_yuv_fmt;
     enum v4l2_g2d_yuv_422     dst_ge_yuv_fmt;

     hal_phy              src_phys;
     int                      src_pitch;
     int                      src_bpp;
     int                      src_index;
     unsigned int             src_ge_format;
     unsigned int             src_w;
     unsigned int             src_h;
     DFBRegion                clip;

     DFBSurfaceDrawingFlags   dflags;
     DFBSurfaceBlittingFlags  bflags;

     u8                      ge_last_render_op;
     /* ge */
     DFBSurfaceBlittingFlags gfx_supported_bld_flags;
     DFBSurfaceDrawingFlags  gfx_supported_draw_flags;

     enum v4l2_g2d_dfbbld_op            ge_src_blend;
     enum v4l2_g2d_dfbbld_op            ge_dst_blend;
     u16                     ge_draw_bld_flags;
     u16                     ge_blit_bld_flags;

     u32                     ge_render_alpha_cmp;

     DFBColor                 color;
     DFBColor                 color2;   // for gradient fill

     CoreDFB                 *core;
     CardState               *state;
     CoreGraphicsDevice      *device;

     CoreScreen              *op_screen;
     CoreScreen              *ip0_screen;
     CoreScreen              *ve_screen;
     CoreScreen              *oc_screen;

     CoreLayer               *layers[MSTAR_MAX_OUTPUT_LAYER_COUNT];

     

     volatile void           *mmio_base;

     int                      num_inputs;

     volatile void           *lcd_virt;

     FlipInfo                 layerFlipInfo[MSTAR_MAX_OUTPUT_LAYER_COUNT];

     enum v4l2_g2d_rop2_op         ge_rop_op;

     // define for sti porting.
     MTKDRMData       DrmData;
 
     int fd; //open device node to control GE drvier
     u16 tagid;
} MSTARDriverData;

typedef enum {
    E_VIDEO_FIELD_INTERLACE = 1,
    E_VIDEO_FIELD_PROGRESSIVE = 2,
}E_VIDEO_FIELD_MODE;
#endif
