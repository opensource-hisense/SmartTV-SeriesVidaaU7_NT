#ifndef __MSTAR__TYPES_H__
#define __MSTAR__TYPES_H__

#include <core/layers.h>


#define MSTAR_MULTI_GOP_SUPPORT          TRUE
#define MSTARGFX_MAX_LAYER_BUFFER        24
#define MAX_PALETTE_ENTRY_CNT  256
#define MAX_PALETTE_INTENSITY_I4_CNT  16
#define MAX_PALETTE_INTENSITY_I2_CNT  4
#define MAX_PALETTE_INTENSITY_I1_CNT  2

#if MSTAR_MULTI_GOP_SUPPORT
#define MSTAR_MAX_GOP_COUNT 4
#define MSTAR_MAX_OUTPUT_LAYER_COUNT MSTAR_MAX_GOP_COUNT
#else
#define MSTAR_MAX_OUTPUT_LAYER_COUNT  4
#endif


typedef struct{
     u32  Hstart;
     u32  Vstart;
     u32  width;
     u32  height;
}ScreenSize;


typedef struct {
     u8            layer_index;
     u8            gop_index;
     u8            gop_dst;
     ScreenSize    screen_size;
} MSTARLayerData;

typedef struct {
     int                      magic;

     CoreLayerRegionConfig    config;
     CoreLayerRegionConfigFlags config_dirtyFlag; //which configuration need to update to hw

} MSTARRegionData;


typedef struct {
     int                      lcd_width;
     int                      lcd_height;
     int                      lcd_offset;
     int                      lcd_pitch;
     int                      lcd_size;
     DFBSurfacePixelFormat    lcd_format;

     /* locking */
     FusionSkirmish           beu_lock;
     bool                          b_hwclip;

     /* gop */
     int                     gfx_gop_index;
     int                     gfx_h_offset;
     int                     gfx_v_offset;
     int                     gfx_max_width;
     int                     gfx_max_height;
     unsigned long           mst_miu1_hal_offset;
     unsigned long           mst_miu1_cpu_offset;

     int                     layer_refcnt[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     int                     layer_zorder[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     bool                    layer_active[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     int                     layer_gwin_id[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     u8                      layer_gop_index[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     ScreenSize              layer_screen_size[MSTAR_MAX_OUTPUT_LAYER_COUNT];

     bool                   a8_palette;
     DFBColor               a8_palette_color;
     GFX_PaletteEntry       palette_tbl[MAX_PALETTE_ENTRY_CNT];
     int                    num_entries;

     u32                   palette_intensity[MAX_PALETTE_INTENSITY_I4_CNT];
     int                   num_intensity;
} MSTARDeviceData;

typedef struct
{
   CoreSurface *pCoDFBCoreSurface;
   CoreSurfaceBuffer *pCoDFBBuffer;
   unsigned long physAddr;
   u16               u16GOPFBID;
   u16               u16SlotUsed;
}LayerBufferInfo;

typedef struct {
     MSTARDeviceData        *dev;

     u32                      ge_src_colorkey_enabled  : 1;
     u32                      ge_draw_dst_colorkey_enabled  : 1;
     u32                      ge_blit_dst_colorkey_enabled  : 1;
     u32                      ge_draw_alpha_blend_enabled   : 1;
     u32                      ge_blit_alpha_blend_enabled   : 1;
     u32                      ge_palette_enabled   :1;
     u32                      ge_blit_nearestmode_enabled :1;
     u32                      ge_draw_src_over_patch       :1;
     u32                      ge_blit_src_over_patch       :1;
     u32                      ge_blit_xmirror               :1;
     u32                      ge_blit_ymirror               :1;
     u32                      ge_palette_intensity_enabled:1;
     u32                      reserved_mask                 :20;

     /* cached values */
     unsigned long            dst_phys;
     int                      dst_pitch;
     int                      dst_bpp;
     int                      dst_index;
     unsigned int             dst_ge_format ;
     unsigned int             dst_w ;
     unsigned int             dst_h ;

     u32             src_ge_clr_key ;
     u32             dst_ge_clr_key;

     GFX_YUV_422     src_ge_yuv_fmt;
     GFX_YUV_422     dst_ge_yuv_fmt;

     unsigned long            src_phys;
     int                      src_pitch;
     int                      src_bpp;
     int                      src_index;
     unsigned int             src_ge_format ;
     unsigned int             src_w ;
     unsigned int             src_h ;
     DFBRegion                clip;

     DFBSurfaceDrawingFlags   dflags;
     DFBSurfaceBlittingFlags  bflags;

     u8                      ge_last_render_op;

     u32                     ge_draw_render_coef;
     u32                     ge_draw_render_alpha_from;
     u8                      ge_draw_render_alpha_const;

     u32                     ge_blit_render_coef;
     u32                     ge_blit_render_alpha_from;
     u8                      ge_blit_render_alpha_const;

     u32                     ge_render_alpha_cmp;
     bool                    ge_render_alpha_cmp_enabled;

     DFBColor                 color;
     DFBColor                 color2;//for gradient fill

     CoreDFB                 *core;
     CardState               *state;
     CoreGraphicsDevice      *device;

     CoreScreen              *op_screen;
     CoreScreen              *ip0_screen;
     CoreScreen              *ve_screen;

     CoreLayer               *layers[MSTAR_MAX_OUTPUT_LAYER_COUNT];

     LayerBufferInfo  mstarLayerBuffer[MSTARGFX_MAX_LAYER_BUFFER];

     volatile void           *mmio_base;

     int                      num_inputs;

     volatile void           *lcd_virt;
} MSTARDriverData;

#endif

