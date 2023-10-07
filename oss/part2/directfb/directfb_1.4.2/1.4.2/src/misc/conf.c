/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <directfb.h>
#include <directfb_util.h>

#include <direct/conf.h>
#include <direct/log.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <fusion/conf.h>
#include <fusion/vector.h>
#include <core/core.h>

#if DIRECTFB_BUILD_VOODOO
#include <voodoo/conf.h>
#endif

#if !DIRECTFB_BUILD_PURE_VOODOO
#include <core/surface.h>
#endif

#include <misc/conf.h>

#include <search.h>
#include <assert.h>

#define SHIFT10 10
#define GE_HW_LIMIT 16384
#define HW_CURSOR_WIDTH 128
#define HW_CURSOR_HEIGHT 128
#define DEC 10

D_DEBUG_DOMAIN( DirectFB_Config, "DirectFB/Config", "Runtime configuration options for DirectFB" );
static void registerSigalstack(void);


DFBConfig *dfb_config = NULL;
static char directory[125];

static const char *config_usage =
     "DirectFB version " DIRECTFB_VERSION "\n"
     "\n"
     " --dfb-help                      Output DirectFB usage information and exit\n"
     " --dfb:<option>[,<option>]...    Pass options to DirectFB (see below)\n"
     "\n"
     "DirectFB options:\n"
     "\n"
     "  system=<system>                Specify the system (FBDev, SDL, etc.)\n"
     "  fbdev=<device>                 Open <device> instead of /dev/fb0\n"
     "  busid=<id>                     Specify the bus location of the graphics card (default 1:0:0)\n"
     "  mode=<width>x<height>          Set the default resolution\n"
     "  scaled=<width>x<height>        Scale the window to this size for 'force-windowed' apps\n"
     "  depth=<pixeldepth>             Set the default pixel depth\n"
     "  pixelformat=<pixelformat>      Set the default pixel format\n"
     "  surface-shmpool-size=<kb>      Set the size of the shared memory pool used\n"
     "                                 for shared system memory surfaces.\n"
     "  session=<num>                  Select multi app world (zero based, -1 = new)\n"
     "  remote=<host>[:<port>]         Set remote host and port to connect to\n"
     "  primary-layer=<id>             Select an alternative primary layer\n"
     "  primary-only                   Tell application only about the primary layer\n"
     "  resource-id=<id>               Use resource id for surfaces if not specified by application\n"
     "  [no-]banner                    Show DirectFB Banner on startup\n"
     "  [no-]surface-sentinel          Enable surface sentinels at the end of chunks in video memory\n"
     "  force-windowed                 Primary surface always is a window\n"
     "  force-desktop                  Primary surface is the desktop background\n"
     "  [no-]hardware                  Enable/disable hardware acceleration\n"
     "  [no-]software                  Enable/disable software fallbacks\n"
     "  [no-]software-warn             Show warnings when doing/dropping software operations\n"
     "  [no-]software-trace            Show every stage of the software rendering pipeline\n"
     "  [no-]dma                       Enable DMA acceleration\n"
     "  [no-]sync                      Do `sync()' (default=no)\n"
#ifdef USE_MMX
     "  [no-]mmx                       Enable mmx support\n"
#endif
     "  [no-]agp[=<mode>]              Enable AGP support\n"
     "  [no-]thrifty-surface-buffers   Free sysmem instance on xfer to video memory\n"
     "  font-format=<pixelformat>      Set the preferred font format\n"
     "  [no-]font-premult              Enable/disable premultiplied glyph images in ARGB format\n"
     "  [no-]deinit-check              Enable deinit check at exit\n"
     "  [no-]core-sighandler           Enable/disable core signal handler (for emergency shutdowns)\n"
     "  block-all-signals              Block all signals\n"
     "  [no-]vt-switch                 Allocate/switch to a new VT\n"
     "  vt-num=<num>                   Use given VT instead of current/new one\n"
     "  [no-]vt-switching              Allow Ctrl+Alt+<F?> (EXPERIMENTAL)\n"
     "  [no-]graphics-vt               Put terminal into graphics mode\n"
     "  [no-]vt                        Use VT handling code at all?\n"
     "  mouse-source=<device>          Mouse device for serial mouse\n"
     "  [no-]mouse-gpm-source          Enable mouse input repeated by GPM\n"
     "  [no-]motion-compression        Mouse motion event compression\n"
     "  mouse-protocol=<protocol>      Mouse protocol\n"
     "  [no-]lefty                     Swap left and right mouse buttons\n"
     "  [no-]capslock-meta             Map the CapsLock key to Meta\n"
     "  linux-input-ir-only            Ignore all non-IR Linux Input devices\n"
     "  [no-]linux-input-grab          Grab Linux Input devices?\n"
     "  [no-]linux-input-force         Force using linux-input with all system modules\n"
     "  [no-]cursor                    Never create a cursor or handle it\n"
     "  [no-]cursor-automation         Automated cursor show/hide for windowed primary surfaces\n"
     "  [no-]cursor-updates            Never show a cursor, but still handle it\n"
     "  wm=<wm>                        Window manager module ('default' or 'unique')\n"
     "  init-layer=<id>                Initialize layer with ID (following layer- options apply)\n"
     "  layer-size=<width>x<height>    Set the pixel resolution\n"
     "  layer-format=<pixelformat>     Set the pixel format\n"
     "  layer-depth=<pixeldepth>       Set the pixel depth\n"
     "  layer-buffer-mode=(auto|triple|backvideo|backsystem|frontonly|windows)\n"
     "  layer-bg-none                  Disable background clear\n"
     "  layer-bg-color=AARRGGBB        Use background color (hex)\n"
     "  layer-bg-color-index=<index>   Use background color index (decimal)\n"
     "  layer-bg-image=<filename>      Use background image\n"
     "  layer-bg-tile=<filename>       Use tiled background image\n"
     "  layer-src-key=AARRGGBB         Enable color keying (hex)\n"
     "  layer-palette-<index>=AARRGGBB Set palette entry at index (hex)\n"
     "  layer-rotate=<degree>          Set the layer rotation for double buffer mode (0,90,180,270)\n"
     "  [no-]smooth-upscale            Enable/disable smooth upscaling per default\n"
     "  [no-]smooth-downscale          Enable/disable smooth downscaling per default\n"
     "  [no-]translucent-windows       Allow translucent windows\n"
     "  [no-]decorations               Enable window decorations (if supported by wm)\n"
     "  [no-]startstop                 Issue StartDrawing/StopDrawing to driver\n"
     "  [no-]autoflip-window           Auto flip non-flipping windowed primary surfaces\n"
     "  videoram-limit=<amount>        Limit amount of Video RAM in kb\n"
     "  agpmem-limit=<amount>          Limit amount of AGP memory in kb\n"
     "  screenshot-dir=<directory>     Dump screen content on <Print> key presses\n"
     "  video-phys=<hexaddress>        Physical start of video memory (devmem system)\n"
     "  video-length=<bytes>           Length of video memory (devmem system)\n"
     "  mmio-phys=<hexaddress>         Physical start of MMIO area (devmem system)\n"
     "  mmio-length=<bytes>            Length of MMIO area (devmem system)\n"
     "  accelerator=<id>               Accelerator ID selecting graphics driver (devmem system)\n"
     "\n"
     "  x11-borderless[=<x>.<y>]       Disable X11 window borders, optionally position window\n"
     "  [no-]matrox-sgram              Use Matrox SGRAM features\n"
     "  [no-]matrox-crtc2              Experimental Matrox CRTC2 support\n"
     "  matrox-tv-standard=(pal|ntsc|pal-60)\n"
     "                                 Matrox TV standard (default=pal)\n"
     "  matrox-cable-type=(composite|scart-rgb|scart-composite)\n"
     "                                 Matrox cable type (default=composite)\n"
     "  h3600-device=<device>          Use this device for the H3600 TS driver\n"
     "  mut-device=<device>            Use this device for the MuTouch driver\n"
     "  zytronic-device=<device>       Use this device for the Zytronic driver\n"
     "  elo-device=<device>            Use this device for the Elo driver\n"
     "  penmount-device=<device>       Use this device for the PenMount driver\n"
     "  linux-input-devices=<device>[[,<device>]...]\n"
     "                                 Use these devices for the Linux Input driver\n"
     "  tslib-devices=<device>[[,<device>]...]\n"
     "                                 Use these devices for the tslib driver\n"
     "  unichrome-revision=<rev>       Override unichrome hardware revision\n"
     "  i8xx_overlay_pipe_b            Redirect videolayer to pixelpipe B\n"
     "  include=<config file>          Include the specified file, relative to the current file\n"
     "\n"
     "  max-font-rows=<number>         Maximum number of glyph cache rows (total for all fonts)\n"
     "  max-font-row-width=<pixels>    Maximum width of glyph cache row surface\n"
     "\n"
     " Window surface swapping policy:\n"
     "  window-surface-policy=(auto|videohigh|videolow|systemonly|videoonly)\n"
     "     auto:       DirectFB decides depending on hardware.\n"
     "     videohigh:  Swapping system/video with high priority.\n"
     "     videolow:   Swapping system/video with low priority.\n"
     "     systemonly: Window surface is always stored in system memory.\n"
     "     videoonly:  Window surface is always stored in video memory.\n"
     "\n"
     " Desktop buffer mode:\n"
     "  desktop-buffer-mode=(auto|triple|backvideo|backsystem|frontonly|windows)\n"
     "     auto:       DirectFB decides depending on hardware.\n"
     "     triple:     Triple buffering (video only).\n"
     "     backvideo:  Front and back buffer are video only.\n"
     "     backsystem: Back buffer is system only.\n"
     "     frontonly:  There is no back buffer.\n"
     "     windows:    Special mode with window buffers directly displayed.\n"
     "\n"
     " Force synchronization of vertical retrace:\n"
     "  vsync-after:   Wait for the vertical retrace after flipping.\n"
     "  vsync-none:    disable polling for vertical retrace.\n"
     "\n";

/**********************************************************************************************************************/

/* serial mouse device names */
#define DEV_NAME     "/dev/mouse"
#define DEV_NAME_GPM "/dev/gpmdata"

/**********************************************************************************************************************/
#define DFBINFO_DEFAULT_PATH    "/config/libdfbinfo.so"
#define DUMP_X 0
#define DUMP_Y 1
#define DUMP_W 2
#define DUMP_H 3
#define ARG_SIZE 4

////////////////////////////////////////
// start of hashtable
#if USE_HASH_TABLE_SREACH

typedef struct data_struct_s
{
    const char *key_string;
    void (*FuncPtr)(char *value);

} data_struct_t;

data_struct_t* tableData;
struct hsearch_data hash;


void FUN___system ( char *value);
void FUN___wm ( char *value);
void FUN___fbdev ( char *value);
void FUN___busid ( char *value);
void FUN___pci_id ( char *value);
void FUN___screenshot_dir ( char *value);
void FUN___scaled ( char *value);
void FUN___primary_layer ( char *value);
void FUN___primary_only ( char *value);
void FUN___font_format ( char *value);
void FUN___font_premult ( char *value);
void FUN___no_font_premult ( char *value);
void FUN___surface_shmpool_size ( char *value);
void FUN___session ( char *value);
void FUN___remote ( char *value);
void FUN___videoram_limit ( char *value);
void FUN___keep_accumulators ( char *value);
void FUN___banner ( char *value);
void FUN___no_banner ( char *value);
void FUN___surface_sentinel ( char *value);
void FUN___no_surface_sentinel ( char *value);
void FUN___force_windowed ( char *value);
void FUN___force_desktop ( char *value);
void FUN___hardware ( char *value);
void FUN___no_hardware ( char *value);
void FUN___software ( char *value);
void FUN___no_software ( char *value);
void FUN___software_warn ( char *value);
void FUN___no_software_warn ( char *value);
void FUN___software_trace ( char *value);
void FUN___no_software_trace ( char *value);
void FUN___small_font_patch ( char *value);
void FUN___warn ( char *value);
void FUN___no_warn ( char *value);
void FUN___dma ( char *value);
void FUN___no_dma ( char *value);
void FUN___mmx ( char *value);
void FUN___no_mmx ( char *value);
void FUN___agp ( char *value);
void FUN___thrifty_surface_buffers ( char *value);
void FUN___no_thrifty_surface_buffers ( char *value);
void FUN___no_agp ( char *value);
void FUN___agpmem_limit ( char *value);
void FUN___vt ( char *value);
void FUN___no_vt ( char *value);
void FUN___block_all_signals ( char *value);
void FUN___deinit_check ( char *value);
void FUN___no_deinit_check ( char *value);
void FUN___cursor ( char *value);
void FUN___no_cursor ( char *value);
void FUN___cursor_updates ( char *value);
void FUN___no_cursor_updates ( char *value);
void FUN___linux_input_ir_only ( char *value);
void FUN___linux_input_grab ( char *value);
void FUN___no_linux_input_grab ( char *value);
void FUN___motion_compression ( char *value);
void FUN___no_motion_compression ( char *value);
void FUN___mouse_protocol ( char *value);
void FUN___mouse_source ( char *value);
void FUN___mouse_gpm_source ( char *value);
void FUN___no_mouse_gpm_source ( char *value);
void FUN___smooth_upscale ( char *value);
void FUN___no_smooth_upscale ( char *value);
void FUN___smooth_downscale ( char *value);
void FUN___no_smooth_downscale ( char *value);
void FUN___translucent_windows ( char *value);
void FUN___no_translucent_windows ( char *value);
void FUN___decorations ( char *value);
void FUN___no_decorations ( char *value);
void FUN___startstop ( char *value);
void FUN___no_startstop ( char *value);
void FUN___autoflip_window ( char *value);
void FUN___no_autoflip_window ( char *value);
void FUN___vsync_none ( char *value);
void FUN___vsync_after ( char *value);
void FUN___vt_switch ( char *value);
void FUN___no_vt_switch ( char *value);
void FUN___vt_num ( char *value);
void FUN___vt_switching ( char *value);
void FUN___no_vt_switching ( char *value);
void FUN___graphics_vt ( char *value);
void FUN___no_graphics_vt ( char *value);
void FUN___window_surface_policy ( char *value);
void FUN___init_layer ( char *value);
void FUN___no_init_layer ( char *value);
void FUN___mode ( char *value);
void FUN___layer_size ( char *value);
void FUN___depth ( char *value);
void FUN___layer_depth ( char *value);
void FUN___pixelformat ( char *value);
void FUN___layer_format ( char *value);
void FUN___desktop_buffer_mode ( char *value);
void FUN___layer_buffer_mode ( char *value);
void FUN___layer_src_key ( char *value);
void FUN___layer_src_key_index ( char *value);
void FUN___bg_none ( char *value);
void FUN___layer_bg_none ( char *value);
void FUN___bg_image ( char *value);
void FUN___bg_tile ( char *value);
void FUN___layer_bg_image ( char *value);
void FUN___layer_bg_tile ( char *value);
void FUN___bg_color ( char *value);
void FUN___layer_bg_color ( char *value);
void FUN___layer_bg_color_index ( char *value);
void FUN___layer_stacking ( char *value);
void FUN___layer_palette ( char *value);
void FUN___layer_rotate ( char *value);
void FUN___video_phys ( char *value);
void FUN___video_length ( char *value);
void FUN___video_phys_secondary ( char *value);
void FUN___video_length_secondary ( char *value);
void FUN___mmio_phys ( char *value);
void FUN___mmio_length ( char *value);
void FUN___accelerator ( char *value);
void FUN___matrox_tv_standard ( char *value);
void FUN___matrox_cable_type ( char *value);
void FUN___matrox_sgram ( char *value);
void FUN___matrox_crtc2 ( char *value);
void FUN___no_matrox_sgram ( char *value);
void FUN___sync ( char *value);
void FUN___no_sync ( char *value);
void FUN___lefty ( char *value);
void FUN___no_lefty ( char *value);
void FUN___capslock_meta ( char *value);
void FUN___no_capslock_meta ( char *value);
void FUN___h3600_device ( char *value);
void FUN___mut_device ( char *value);
void FUN___zytronic_device ( char *value);
void FUN___elo_device ( char *value);
void FUN___penmount_device ( char *value);
void FUN___linux_input_devices ( char *value);
void FUN___tslib_devices ( char *value);
void FUN___unichrome_revision ( char *value);
void FUN___i8xx_overlay_pipe_b ( char *value);
void FUN___include ( char *value);
void FUN___mst_gfx_width ( char *value);
void FUN___mst_gfx_height ( char *value);
void FUN___mst_lcd_width ( char *value);
void FUN___mst_lcd_height ( char *value);
void FUN___mst_ge_vq_phys_addr ( char *value);
void FUN___mst_ge_vq_phys_len ( char *value);
void FUN___mst_gop_regdma_phys_addr ( char *value);
void FUN___mst_gop_regdma_phys_len ( char *value);
void FUN___mst_miu0_hal_offset ( char *value);
void FUN___mst_miu0_hal_length ( char *value);
void FUN___mst_miu0_cpu_offset ( char *value);
void FUN___mst_miu1_hal_offset ( char *value);
void FUN___mst_miu1_hal_length ( char *value);
void FUN___mst_miu1_cpu_offset ( char *value);
void FUN___mst_miu2_hal_offset ( char *value);
void FUN___mst_miu2_hal_length ( char *value);
void FUN___mst_miu2_cpu_offset ( char *value);
void FUN___mst_gfx_h_offset ( char *value);
void FUN___mst_gfx_v_offset ( char *value);
void FUN___mst_gfx_gop_index ( char *value);
void FUN___default_layer_opacity ( char *value);
void FUN___mst_gop_counts ( char *value);
void FUN___mst_gop_available0 ( char *value);
void FUN___mst_gop_available1 ( char *value);
void FUN___mst_gop_available2 ( char *value);
void FUN___mst_gop_available3 ( char *value);
void FUN___mst_gop_available4 ( char *value);
void FUN___mst_gop_available5 ( char *value);
void FUN___mst_gop_available_r0 ( char *value);
void FUN___mst_gop_available_r1 ( char *value);
void FUN___mst_gop_available_r2 ( char *value);
void FUN___mst_gop_available_r3 ( char *value);
void FUN___mst_gop_available_r4 ( char *value);
void FUN___mst_gop_available_r5 ( char *value);
void FUN___mst_gop_dstPlane0 ( char *value);
void FUN___mst_gop_dstPlane1 ( char *value);
void FUN___mst_gop_dstPlane2 ( char *value);
void FUN___mst_gop_dstPlane3 ( char *value);
void FUN___mst_gop_dstPlane4 ( char *value);
void FUN___mst_gop_dstPlane5 ( char *value);
void FUN___mst_jpeg_readbuff_addr ( char *value);
void FUN___mst_jpeg_readbuff_length ( char *value);
void FUN___mst_jpeg_interbuff_addr ( char *value);
void FUN___mst_jpeg_interbuff_length ( char *value);
void FUN___mst_jpeg_outbuff_addr ( char *value);
void FUN___mst_jpeg_outbuff_length ( char *value);
void FUN___mst_jpeg_hwdecode ( char *value);
void FUN___muxCounts ( char *value);
void FUN___mux0_gopIndex ( char *value);
void FUN___mux1_gopIndex ( char *value);
void FUN___mux2_gopIndex ( char *value);
void FUN___mux3_gopIndex ( char *value);
void FUN___mux4_gopIndex ( char *value);
void FUN___mux5_gopIndex ( char *value);
void FUN___goplayerCount ( char *value);
void FUN___goplayer0_gopIndex ( char *value);
void FUN___goplayer1_gopIndex ( char *value);
void FUN___goplayer2_gopIndex ( char *value);
void FUN___goplayer3_gopIndex ( char *value);
void FUN___goplayer4_gopIndex ( char *value);
void FUN___goplayer5_gopIndex ( char *value);
void FUN___mst_gop_miu_setting ( char *value);
void FUN___mst_disable_hwclip ( char *value);
void FUN___mst_reg_appm ( char *value);
void FUN___mst_enable_GOP_HMirror ( char *value);
void FUN___mst_disable_GOP_HMirror ( char *value);
void FUN___mst_enable_GOP_VMirror ( char *value);
void FUN___mst_disable_GOP_VMirror ( char *value);
void FUN___mst_osd_to_ve ( char *value);
void FUN___keypad_callback ( char *value);
void FUN___keypad_callback2 ( char *value);
void FUN___keypad_internal ( char *value);
void FUN___mst_GOP_Set_YUV ( char *value);
void FUN___stretchdown_patch ( char *value);
void FUN___line_stretchblit_patch ( char *value);
void FUN___stretchdown_enhance ( char *value);
void FUN___stretchdown_patch ( char *value);
void FUN___static_stretchdown_buf ( char *value);
void FUN___mst_png_hwdecode ( char *value);
void FUN___mst_png_disable_hwdecode ( char *value);
void FUN___tvos_mode ( char *value);
void FUN___devmem_dump ( char *value);
void FUN___dfb_using_forcewriteEnable ( char *value);
void FUN___dfb_using_forcewriteDisable ( char *value);
void FUN___dfb_ignore_forcewrite ( char *value);
void FUN___yuvtorgb_patch ( char *value);
void FUN___enable_sw_jpeg ( char *value);
void FUN___enable_dither ( char *value);
void FUN___mst_ir_max_keycode ( char *value);
void FUN___gop_using_tlb ( char *value);
void FUN___prealloc_map_tlb ( char *value);
void FUN___tlb_alignment ( char *value);
void FUN___mboot_gop_index ( char *value);
void FUN___mst_enable_gevq ( char *value);
void FUN___mst_disable_layer_init ( char *value);
void FUN___debug_layer ( char *value);
void FUN___debug_surface ( char *value);
void FUN___debug_input ( char *value);
void FUN___window_double_buffer ( char *value);
void FUN___debug_ion ( char *value);
void FUN___ion_heapmask_by_layer0 ( char *value);
void FUN___ion_heapmask_by_layer1 ( char *value);
void FUN___ion_heapmask_by_layer2 ( char *value);
void FUN___ion_heapmask_by_layer3 ( char *value);
void FUN___ion_heapmask_by_layer4 ( char *value);
void FUN___ion_heapmask_by_layer5 ( char *value);
void FUN___ion_heapmask_by_surface ( char *value);
void FUN___mst_layer_default_width ( char *value);
void FUN___mst_layer_default_height ( char *value);
void FUN___full_update_num ( char *value);
void FUN___full_update_den ( char *value);
void FUN___surface_memory_type ( char *value);
void FUN___disable_decode_small_jpeg_by_sw ( char *value);
void FUN___enable_GOP_Vmirror_Patch ( char *value);
void FUN___disble_GOP_Vmirror_Patch ( char *value);
void FUN___stretchdown_patch_ratio ( char *value);
void FUN___null_driver ( char *value);
void FUN___sw_render ( char *value);
void FUN___enable_jpeg_quality ( char *value);
void FUN___mst_enable_dip ( char *value);
void FUN___mst_dip_mload_addr ( char *value);
void FUN___mst_dip_mload_length ( char *value);
void FUN___mst_dip_select ( char *value);
void FUN___mst_disable_master_pri_high ( char *value);
void FUN___show_freeze_image ( char *value);
void FUN___i8toargb4444_patch ( char *value);
void FUN___mst_enable_ve_init ( char *value);
void FUN___mst_register_gpd ( char *value);
void FUN___hw_jpg_limit_patch ( char *value);
void FUN___dump_backtrace ( char *value);
void FUN___test_ge ( char *value);
void FUN___miu_protect ( char *value);
void FUN___mst_cma_heap_id ( char *value);
void FUN___mapmem_mode ( char *value);
void FUN___mst_sec_cma_heap_id ( char *value);
void FUN___mst_disable_dfbinfo ( char *value);
void FUN___src_color_key_index_patch ( char *value);
void FUN___mst_ir_repeat_time ( char *value);
void FUN___mst_keypad_repeat_time ( char *value);
void FUN___mst_ir_first_time_out ( char *value);
void FUN___mst_handle_wm_key ( char *value);
void FUN___disable_cjk_string_break ( char *value);
void FUN___disable_quick_press ( char *value);
void FUN___mst_disable_window_scale_patch ( char *value);
void FUN___mst_force_flip_wait_tagid ( char *value);
void FUN___enable_cursor_mouse_optimize( char *value);
void FUN___layer_support_palette( char *value); 
void FUN___stretchblit_with_rotate( char *value); 
void FUN___mst_layer_gwin_level( char *value);
void FUN___mst_enable_gop_gwin_partial_update( char *value);
void FUN___mst_enable_new_alphamode(char *value);
void FUN___mst_new_alphamode_on_layerid(char *value);
void FUN___enable_layer_fullupdate0(char *value);
void FUN___enable_layer_fullupdate1(char *value);
void FUN___enable_layer_fullupdate2(char *value);
void FUN___enable_layer_fullupdate3(char *value);
void FUN___enable_layer_fullupdate4(char *value);
void FUN___enable_layer_fullupdate5(char *value);
void FUN___mst_AFBC_enable(char *value);
void FUN___mst_AFBC_mode(char *value);
void FUN___mst_enhance_stretchblit_precision(char *value);
void FUN___mst_gop_miu2_setting_extend(char *value);
void FUN__mst_enable_GLES2(char *value);
void FUN___mst_mem_peak_usage( char *value);
void FUN___mst_enable_gwin_multialpha(char *value);
void FUN___mst_measure_png_performance(char *value);
void FUN___mst_rgb2yuv_mode(char *value);
void FUN___debug_cma(char *value);
void FUN___mst_margine_left(char *value);
void FUN___mst_margine_wright(char *value);
void FUN___mst_margine_top(char *value);
void FUN___mst_margine_bottom(char *value);
void FUN___mst_modify_symbol_by_keymap(char *value);
void FUN___mst_disable_modify_symbol_by_keymap(char *value);
void FUN___mst_memory_use_cma(char *value);
void FUN___mst_gfxdriver(char *value);
void FUN___mst_font_use_video_mem(char *value);
void FUN___dfbinfo_dir ( char *value);
void FUN___disable_bus_address_check ( char *value);
void FUN___mst_layer_flip_blit(char *value);
void FUN___window_single_buffer ( char *value);
void FUN___mst_new_ir( char *value);
void FUN___mst_new_ir_first_repeat_time( char *value);
void FUN___mst_new_ir_repeat_time( char *value);
void FUN___mst_t_stretch_mode( char *value);
void FUN___mst_h_stretch_mode( char *value);
void FUN___mst_v_stretch_mode( char *value);
void FUN___mst_blink_frame_rate( char *value);
void FUN___mst_cursor_swap_mode( char *value);
void FUN___mst_inputevent_layer( char *value);
void FUN___mst_flip_dump_path(char *value);
void FUN___mst_flip_dump_type(char *value);
void FUN___mst_flip_dump_area(char *value);
void FUN___mst_fix_string_break(char *value);
void FUN___mst_argb1555_display(char *value);
void FUN___mst_font_dsblit_src_premultiply(char *value);
void FUN___mst_gles2_sdr2hdr(char *value);
void FUN___mst_ge_hw_limit(char *value);
void FUN___mst_cmdq_phys_addr(char *value);
void FUN___mst_cmdq_phys_len(char *value);
void FUN___mst_cmdq_miusel(char *value);
void FUN___mst_gop_interlace_adjust(char *value);
void FUN___mst_CFD_monitor_path(char *value);
void FUN___mst_layer_buffer_mode(char *value);
void FUN___mst_layer_pixelformat(char *value);
void FUN___mst_do_xc_ip1_patch(char *value);
void FUN___mst_dump_jpeg_buffer(char * value);
void FUN___mst_clip_stretchblit_width_high_nonzero_patch(char * value);
void FUN___mst_bank_force_write(char * value);
void FUN___mst_font_bold_enable(char * value);
void FUN__mst_fixed_mem_leak_patch_enable(char * value);
void FUN___mst_nice(char * value);
void FUN___mst_show_fps(char * value);
void FUN___mst_enable_layer_autoscaledown(char * value);
void FUN___mst_use_dlopen_dlsym(char * value);
void FUN___mst_mem_small_size( char *value);
void FUN___mst_mem_medium_size( char *value);
void FUN___mst_forbid_fragment_merge_to_api_locked_surface( char *value);
void FUN___mst_null_display_driver( char *value);
void FUN___mst_call_setkeypadcfg_in_device( char *value);
void FUN___mst_call_setdfbrccfg_disable( char *value);
void FUN___mst_gwin_disable(char *value);
void FUN___mst_using_mi_system(char *value);
void FUN___mst_force_wait_vsync(char *value);
void FUN___mst_new_mstar_linux_input(char *value);
void FUN___mst_mma_pool_enable(char *value);
void FUN___mst_call_gop_t3d(char *value);
void FUN___mst_gopc_check_video_info(char *value);
void FUN___mst_preinit_fusion_world(char *value);
void FUN___mst_font_outline_enable(char *value);
void FUN___mst_bt601_formula(char *value);
void FUN___mst_PTK_customized_input_driver_enable(char *value);
void FUN___mst_layer_fbdev0(char *value);
void FUN___mst_layer_fbdev1(char *value);
void FUN___mst_layer_fbdev2(char *value);
void FUN___mst_layer_fbdev3(char *value);
void FUN___mst_layer_fbdev4(char *value);
void FUN___mst_layer_fbdev5(char *value);
void FUN___mst_debug_mma(char *value);
void FUN___mst_oom_retry(char *value);
void FUN___mst_use_system_memory_threshold(char *value);
void FUN___mst_GOP_AutoDetectBuf(char *value);
void FUN___mst_call_disable_bootlogo(char *value);
void FUN___mst_GPU_window_compose(char *value);
void FUN___mst_debug_secure_mode(char *value);
void FUN___mst_debug_gles2(char *value);
void FUN___mst_layer_up_scaling(char *value);
void FUN___mst_layer_up_scaling_id(char *value);
void FUN___mst_layer_up_scaling_width(char *value);
void FUN___mst_layer_up_scaling_height(char *value);
void FUN___mst_debug_directory_access(char * value);
void FUN___mst_debug_backtrace_dump(char *value);
void FUN___mst_debug_layer_setConfiguration_return(char *value);
void FUN___mst_debug_surface_clear_return(char *value);
void FUN___mst_debug_surface_fillrectangle_return(char *value);
void FUN___mst_GPU_AFBC(char *value);
void FUN___GPU_AFBC_EXT_SIZE(char *value);
void FUN___mst_input_vendor_id(char *value);
void FUN___mst_input_product_id(char *value);

void FUN___mst_debug_cursor(char *value);
void FUN___mst_cursor_gwin_width(char *value);
void FUN___mst_cursor_gwin_height(char *value);
void FUN___mst_hw_cursor( char *value);

data_struct_t ConfigTable[] =
{
    { "system", FUN___system },
    { "wm", FUN___wm },
    { "fbdev", FUN___fbdev },
    { "busid", FUN___busid },
    { "pci-id", FUN___pci_id },
    { "screenshot-dir", FUN___screenshot_dir },
    { "scaled", FUN___scaled },
    { "primary-layer", FUN___primary_layer },
    { "primary-only", FUN___primary_only },
    { "font-format", FUN___font_format },
    { "font-premult", FUN___font_premult },
    { "no-font-premult", FUN___no_font_premult },
    { "surface-shmpool-size", FUN___surface_shmpool_size },
    { "session", FUN___session },
    { "remote", FUN___remote },
    { "videoram-limit", FUN___videoram_limit },
    { "keep-accumulators", FUN___keep_accumulators },
    { "banner", FUN___banner },
    { "no-banner", FUN___no_banner },
    { "surface-sentinel", FUN___surface_sentinel },
    { "no-surface-sentinel", FUN___no_surface_sentinel },
    { "force-windowed", FUN___force_windowed },
    { "force-desktop", FUN___force_desktop },
    { "hardware", FUN___hardware },
    { "no-hardware", FUN___no_hardware },
    { "software", FUN___software },
    { "no-software", FUN___no_software },
    { "software-warn", FUN___software_warn },
    { "no-software-warn", FUN___no_software_warn },
    { "software-trace", FUN___software_trace },
    { "no-software-trace", FUN___no_software_trace },
    { "small_font_patch", FUN___small_font_patch },
    { "warn", FUN___warn },
    { "no-warn", FUN___no_warn },
    { "dma", FUN___dma },
    { "no-dma", FUN___no_dma },
    { "mmx", FUN___mmx },
    { "no-mmx", FUN___no_mmx },
    { "agp", FUN___agp },
    { "thrifty-surface-buffers", FUN___thrifty_surface_buffers },
    { "no-thrifty-surface-buffers", FUN___no_thrifty_surface_buffers },
    { "no-agp", FUN___no_agp },
    { "agpmem-limit", FUN___agpmem_limit },
    { "vt", FUN___vt },
    { "no-vt", FUN___no_vt },
    { "block-all-signals", FUN___block_all_signals },
    { "deinit-check", FUN___deinit_check },
    { "no-deinit-check", FUN___no_deinit_check },
    { "cursor", FUN___cursor },
    { "no-cursor", FUN___no_cursor },
    { "cursor-updates", FUN___cursor_updates },
    { "no-cursor-updates", FUN___no_cursor_updates },
    { "linux-input-ir-only", FUN___linux_input_ir_only },
    { "linux-input-grab", FUN___linux_input_grab },
    { "no-linux-input-grab", FUN___no_linux_input_grab },
    { "motion-compression", FUN___motion_compression },
    { "no-motion-compression", FUN___no_motion_compression },
    { "mouse-protocol", FUN___mouse_protocol },
    { "mouse-source", FUN___mouse_source },
    { "mouse-gpm-source", FUN___mouse_gpm_source },
    { "no-mouse-gpm-source", FUN___no_mouse_gpm_source },
    { "smooth-upscale", FUN___smooth_upscale },
    { "no-smooth-upscale", FUN___no_smooth_upscale },
    { "smooth-downscale", FUN___smooth_downscale },
    { "no-smooth-downscale", FUN___no_smooth_downscale },
    { "translucent-windows", FUN___translucent_windows },
    { "no-translucent-windows", FUN___no_translucent_windows },
    { "decorations", FUN___decorations },
    { "no-decorations", FUN___no_decorations },
    { "startstop", FUN___startstop },
    { "no-startstop", FUN___no_startstop },
    { "autoflip-window", FUN___autoflip_window },
    { "no-autoflip-window", FUN___no_autoflip_window },
    { "vsync-none", FUN___vsync_none },
    { "vsync-after", FUN___vsync_after },
    { "vt-switch", FUN___vt_switch },
    { "no-vt-switch", FUN___no_vt_switch },
    { "vt-num", FUN___vt_num },
    { "vt-switching", FUN___vt_switching },
    { "no-vt-switching", FUN___no_vt_switching },
    { "graphics-vt", FUN___graphics_vt },
    { "no-graphics-vt", FUN___no_graphics_vt },
    { "window-surface-policy", FUN___window_surface_policy },
    { "init-layer", FUN___init_layer },
    { "no-init-layer", FUN___no_init_layer },
    { "mode", FUN___mode },
    { "layer-size", FUN___layer_size },
    { "depth", FUN___depth },
    { "layer-depth", FUN___layer_depth },
    { "pixelformat", FUN___pixelformat },
    { "layer-format", FUN___layer_format },
    { "desktop-buffer-mode", FUN___desktop_buffer_mode },
    { "layer-buffer-mode", FUN___layer_buffer_mode },
    { "layer-src-key", FUN___layer_src_key },
    { "layer-src-key-index", FUN___layer_src_key_index },
    { "bg-none", FUN___bg_none },
    { "layer-bg-none", FUN___layer_bg_none },
    { "bg-image", FUN___bg_image },
    { "bg-tile", FUN___bg_tile },
    { "layer-bg-image", FUN___layer_bg_image },
    { "layer-bg-tile", FUN___layer_bg_tile },
    { "bg-color", FUN___bg_color },
    { "layer-bg-color", FUN___layer_bg_color },
    { "layer-bg-color-index", FUN___layer_bg_color_index },
    { "layer-stacking", FUN___layer_stacking },
    { "layer-palette", FUN___layer_palette },
    { "layer-rotate", FUN___layer_rotate },
    { "video-phys", FUN___video_phys },
    { "video-length", FUN___video_length },
    { "video-phys-secondary", FUN___video_phys_secondary },
    { "video-length-secondary", FUN___video_length_secondary },
    { "mmio-phys", FUN___mmio_phys },
    { "mmio-length", FUN___mmio_length },
    { "accelerator", FUN___accelerator },
    { "matrox-tv-standard", FUN___matrox_tv_standard },
    { "matrox-cable-type", FUN___matrox_cable_type },
    { "matrox-sgram", FUN___matrox_sgram },
    { "matrox-crtc2", FUN___matrox_crtc2 },
    { "no-matrox-sgram", FUN___no_matrox_sgram },
    { "sync", FUN___sync },
    { "no-sync", FUN___no_sync },
    { "lefty", FUN___lefty },
    { "no-lefty", FUN___no_lefty },
    { "capslock-meta", FUN___capslock_meta },
    { "no-capslock-meta", FUN___no_capslock_meta },
    { "h3600-device", FUN___h3600_device },
    { "mut-device", FUN___mut_device },
    { "zytronic-device", FUN___zytronic_device },
    { "elo-device", FUN___elo_device },
    { "penmount-device", FUN___penmount_device },
    { "linux-input-devices", FUN___linux_input_devices },
    { "tslib-devices", FUN___tslib_devices },
    { "unichrome-revision", FUN___unichrome_revision },
    { "i8xx_overlay_pipe_b", FUN___i8xx_overlay_pipe_b },
    { "include", FUN___include },
    { "mst_gfx_width", FUN___mst_gfx_width },
    { "mst_gfx_height", FUN___mst_gfx_height },
    { "mst_lcd_width", FUN___mst_lcd_width },
    { "mst_lcd_height", FUN___mst_lcd_height },
    { "mst_ge_vq_phys_addr", FUN___mst_ge_vq_phys_addr },
    { "mst_ge_vq_phys_len", FUN___mst_ge_vq_phys_len },
    { "mst_gop_regdma_phys_addr", FUN___mst_gop_regdma_phys_addr },
    { "mst_gop_regdma_phys_len", FUN___mst_gop_regdma_phys_len },
    { "mst_miu0_hal_offset", FUN___mst_miu0_hal_offset },
    { "mst_miu0_hal_length", FUN___mst_miu0_hal_length },
    { "mst_miu0_cpu_offset", FUN___mst_miu0_cpu_offset },
    { "mst_miu1_hal_offset", FUN___mst_miu1_hal_offset },
    { "mst_miu1_hal_length", FUN___mst_miu1_hal_length },
    { "mst_miu1_cpu_offset", FUN___mst_miu1_cpu_offset },
    { "mst_miu2_hal_offset", FUN___mst_miu2_hal_offset },
    { "mst_miu2_hal_length", FUN___mst_miu2_hal_length },
    { "mst_miu2_cpu_offset", FUN___mst_miu2_cpu_offset },
    { "mst_gfx_h_offset", FUN___mst_gfx_h_offset },
    { "mst_gfx_v_offset", FUN___mst_gfx_v_offset },
    { "mst_gfx_gop_index", FUN___mst_gfx_gop_index },
    { "default_layer_opacity", FUN___default_layer_opacity },
    { "mst_gop_counts", FUN___mst_gop_counts },
    { "mst_gop_available[0]", FUN___mst_gop_available0 },
    { "mst_gop_available[1]", FUN___mst_gop_available1 },
    { "mst_gop_available[2]", FUN___mst_gop_available2 },
    { "mst_gop_available[3]", FUN___mst_gop_available3 },
    { "mst_gop_available[4]", FUN___mst_gop_available4 },
    { "mst_gop_available[5]", FUN___mst_gop_available5 },
    { "mst_gop_available_r[0]", FUN___mst_gop_available_r0 },
    { "mst_gop_available_r[1]", FUN___mst_gop_available_r1 },
    { "mst_gop_available_r[2]", FUN___mst_gop_available_r2 },
    { "mst_gop_available_r[3]", FUN___mst_gop_available_r3 },
    { "mst_gop_available_r[4]", FUN___mst_gop_available_r4 },
    { "mst_gop_available_r[5]", FUN___mst_gop_available_r5 },
    { "mst_gop_dstPlane[0]", FUN___mst_gop_dstPlane0 },
    { "mst_gop_dstPlane[1]", FUN___mst_gop_dstPlane1 },
    { "mst_gop_dstPlane[2]", FUN___mst_gop_dstPlane2 },
    { "mst_gop_dstPlane[3]", FUN___mst_gop_dstPlane3 },
    { "mst_gop_dstPlane[4]", FUN___mst_gop_dstPlane4 },
    { "mst_gop_dstPlane[5]", FUN___mst_gop_dstPlane5 },
    { "mst_jpeg_readbuff_addr", FUN___mst_jpeg_readbuff_addr },
    { "mst_jpeg_readbuff_length", FUN___mst_jpeg_readbuff_length },
    { "mst_jpeg_interbuff_addr", FUN___mst_jpeg_interbuff_addr },
    { "mst_jpeg_interbuff_length", FUN___mst_jpeg_interbuff_length },
    { "mst_jpeg_outbuff_addr", FUN___mst_jpeg_outbuff_addr },
    { "mst_jpeg_outbuff_length", FUN___mst_jpeg_outbuff_length },
    { "mst_jpeg_hwdecode", FUN___mst_jpeg_hwdecode },
    { "muxCounts", FUN___muxCounts },
    { "mux0_gopIndex", FUN___mux0_gopIndex },
    { "mux1_gopIndex", FUN___mux1_gopIndex },
    { "mux2_gopIndex", FUN___mux2_gopIndex },
    { "mux3_gopIndex", FUN___mux3_gopIndex },
    { "mux4_gopIndex", FUN___mux4_gopIndex },
    { "mux5_gopIndex", FUN___mux5_gopIndex },
    { "goplayerCount", FUN___goplayerCount },
    { "goplayer0_gopIndex", FUN___goplayer0_gopIndex },
    { "goplayer1_gopIndex", FUN___goplayer1_gopIndex },
    { "goplayer2_gopIndex", FUN___goplayer2_gopIndex },
    { "goplayer3_gopIndex", FUN___goplayer3_gopIndex },
    { "goplayer4_gopIndex", FUN___goplayer4_gopIndex },
    { "goplayer5_gopIndex", FUN___goplayer5_gopIndex },
    { "mst_gop_miu_setting", FUN___mst_gop_miu_setting },
    { "mst_disable_hwclip", FUN___mst_disable_hwclip },
    { "mst_reg_appm", FUN___mst_reg_appm },
    { "mst_enable_GOP_HMirror", FUN___mst_enable_GOP_HMirror },
    { "mst_disable_GOP_HMirror", FUN___mst_disable_GOP_HMirror },
    { "mst_enable_GOP_VMirror", FUN___mst_enable_GOP_VMirror },
    { "mst_disable_GOP_VMirror", FUN___mst_disable_GOP_VMirror },
    { "mst_osd_to_ve", FUN___mst_osd_to_ve },
    { "keypad_callback", FUN___keypad_callback },
    { "keypad_callback2", FUN___keypad_callback2 },
    { "keypad_internal", FUN___keypad_internal },
    { "mst_GOP_Set_YUV", FUN___mst_GOP_Set_YUV },
    { "stretchdown_patch", FUN___stretchdown_patch },
    { "line_stretchblit_patch", FUN___line_stretchblit_patch },
    { "stretchdown_enhance", FUN___stretchdown_enhance },
    { "static_stretchdown_buf", FUN___static_stretchdown_buf },
    { "mst_png_hwdecode", FUN___mst_png_hwdecode },
    { "mst_png_disable_hwdecode", FUN___mst_png_disable_hwdecode },
    { "tvos_mode", FUN___tvos_mode },
    { "devmem_dump", FUN___devmem_dump },
    { "dfb_using_forcewriteEnable", FUN___dfb_using_forcewriteEnable },
    { "dfb_using_forcewriteDisable", FUN___dfb_using_forcewriteDisable },
    { "dfb_ignore_forcewrite", FUN___dfb_ignore_forcewrite },
    { "yuvtorgb_patch", FUN___yuvtorgb_patch },
    { "enable_sw_jpeg", FUN___enable_sw_jpeg },
    { "enable_dither", FUN___enable_dither },
    { "mst_ir_max_keycode", FUN___mst_ir_max_keycode },
    { "gop_using_tlb", FUN___gop_using_tlb },
    { "prealloc_map_tlb", FUN___prealloc_map_tlb },
    { "tlb_alignment", FUN___tlb_alignment },
    { "mboot_gop_index", FUN___mboot_gop_index },
    { "mst_enable_gevq", FUN___mst_enable_gevq },
    { "mst_disable_layer_init", FUN___mst_disable_layer_init },
    { "debug_layer", FUN___debug_layer },
    { "debug_surface", FUN___debug_surface },
    { "debug_input", FUN___debug_input },
    { "window_double_buffer", FUN___window_double_buffer },
    { "debug_ion", FUN___debug_ion },
    { "ion_heapmask_by_layer[0]", FUN___ion_heapmask_by_layer0 },
    { "ion_heapmask_by_layer[1]", FUN___ion_heapmask_by_layer1 },
    { "ion_heapmask_by_layer[2]", FUN___ion_heapmask_by_layer2 },
    { "ion_heapmask_by_layer[3]", FUN___ion_heapmask_by_layer3 },
    { "ion_heapmask_by_layer[4]", FUN___ion_heapmask_by_layer4 },
    { "ion_heapmask_by_layer[5]", FUN___ion_heapmask_by_layer5 },
    { "ion_heapmask_by_surface", FUN___ion_heapmask_by_surface },
    { "mst_layer_default_width", FUN___mst_layer_default_width },
    { "mst_layer_default_height", FUN___mst_layer_default_height },
    { "full_update_num", FUN___full_update_num },
    { "full_update_den", FUN___full_update_den },
    { "surface_memory_type", FUN___surface_memory_type },
    { "disable_decode_small_jpeg_by_sw", FUN___disable_decode_small_jpeg_by_sw },
    { "enable_GOP_Vmirror_Patch", FUN___enable_GOP_Vmirror_Patch },
    { "disble_GOP_Vmirror_Patch", FUN___disble_GOP_Vmirror_Patch },
    { "stretchdown_patch_ratio", FUN___stretchdown_patch_ratio },
    { "null_driver", FUN___null_driver },
    { "sw_render", FUN___sw_render },
    { "enable_jpeg_quality", FUN___enable_jpeg_quality },
    { "mst_enable_dip", FUN___mst_enable_dip },
    { "mst_dip_mload_addr", FUN___mst_dip_mload_addr },
    { "mst_dip_mload_length", FUN___mst_dip_mload_length },
    { "mst_dip_select", FUN___mst_dip_select },
    { "mst_disable_master_pri_high", FUN___mst_disable_master_pri_high },
    { "show_freeze_image", FUN___show_freeze_image },
    { "i8toargb4444_patch", FUN___i8toargb4444_patch },
    { "mst_enable_ve_init", FUN___mst_enable_ve_init },
    { "mst_register_gpd", FUN___mst_register_gpd },
    { "hw_jpg_limit_patch", FUN___hw_jpg_limit_patch },
    { "dump_backtrace", FUN___dump_backtrace },
    { "test_ge", FUN___test_ge },
    { "miu_protect", FUN___miu_protect },
    { "mst_cma_heap_id", FUN___mst_cma_heap_id },
    { "mapmem_mode", FUN___mapmem_mode },
    { "mst_sec_cma_heap_id", FUN___mst_sec_cma_heap_id },    
    { "mst_disable_dfbinfo", FUN___mst_disable_dfbinfo },
    { "src_color_key_index_patch", FUN___src_color_key_index_patch },
    { "mst_ir_repeat_time", FUN___mst_ir_repeat_time },
    { "mst_keypad_repeat_time", FUN___mst_keypad_repeat_time },
    { "mst_usbir_repeat_time", FUN___mst_usbir_repeat_time},
    { "mst_ir_first_time_out", FUN___mst_ir_first_time_out },
    { "mst_handle_wm_key", FUN___mst_handle_wm_key },
    { "disable_cjk_string_break", FUN___disable_cjk_string_break },
    { "disable_quick_press", FUN___disable_quick_press },
    { "mst_disable_window_scale_patch", FUN___mst_disable_window_scale_patch },
    { "mst_force_flip_wait_tagid", FUN___mst_force_flip_wait_tagid },
    { "enable_cursor_mouse_optimize", FUN___enable_cursor_mouse_optimize },
    { "layer_support_palette", FUN___layer_support_palette },
    { "stretchblit_with_rotate", FUN___stretchblit_with_rotate },
    { "mst_layer_gwin_level", FUN___mst_layer_gwin_level },
    { "mst_enable_gop_gwin_partial_update", FUN___mst_enable_gop_gwin_partial_update},
    { "enable_new_alphamode",FUN___mst_enable_new_alphamode},
    { "mst_enable_new_alphamode",FUN___mst_enable_new_alphamode},
    { "new_alphamode_on_layerid",FUN___mst_new_alphamode_on_layerid},
    { "mst_new_alphamode_on_layerid",FUN___mst_new_alphamode_on_layerid},
    { "enable_layer_fullupdate[0]", FUN___enable_layer_fullupdate0 },
    { "enable_layer_fullupdate[1]", FUN___enable_layer_fullupdate1 },
    { "enable_layer_fullupdate[2]", FUN___enable_layer_fullupdate2 },
    { "enable_layer_fullupdate[3]", FUN___enable_layer_fullupdate3 },
    { "enable_layer_fullupdate[4]", FUN___enable_layer_fullupdate4 },
    { "enable_layer_fullupdate[5]", FUN___enable_layer_fullupdate5 },
    { "mst_AFBC_layer_enable", FUN___mst_AFBC_enable},
    { "mst_AFBC_mode", FUN___mst_AFBC_mode},
    { "mst_enhance_stretchblit_precision", FUN___mst_enhance_stretchblit_precision},
    { "mst_gop_miu2_setting_extend", FUN___mst_gop_miu2_setting_extend },
    { "mst_enable_GLES2", FUN__mst_enable_GLES2 },
    { "mst_mem_peak_usage", FUN___mst_mem_peak_usage},
    { "mst_enable_gwin_multialpha", FUN___mst_enable_gwin_multialpha},
    { "mst_measure_png_performance", FUN___mst_measure_png_performance},
    { "mst_rgb2yuv_mode",FUN___mst_rgb2yuv_mode},
    { "debug_cma", FUN___debug_cma },
    { "margine_left", FUN___mst_margine_left },
    { "margine_wright", FUN___mst_margine_wright },
    { "margine_top", FUN___mst_margine_top },
    { "margine_bottom", FUN___mst_margine_bottom },
    { "CTV_patch", FUN___mst_modify_symbol_by_keymap },
    { "disable_modify_symbol_by_keymap", FUN___mst_modify_symbol_by_keymap },
    { "mst_memory_use_cma", FUN___mst_memory_use_cma },
    { "mst_gfxdriver", FUN___mst_gfxdriver},
    { "mst_font_use_video_mem", FUN___mst_font_use_video_mem},
    { "dfbinfo-dir", FUN___dfbinfo_dir},
    { "disable_bus_address_check", FUN___disable_bus_address_check},
    { "mst_layer_flip_blit", FUN___mst_layer_flip_blit},
    { "window_single_buffer", FUN___window_single_buffer},
    { "mst_new_ir", FUN___mst_new_ir},
    { "mst_new_ir_repeat_time", FUN___mst_new_ir_repeat_time},
    { "mst_new_ir_first_repeat_time", FUN___mst_new_ir_first_repeat_time},
    { "mst_t_stretch_mode", FUN___mst_t_stretch_mode},
    { "mst_h_stretch_mode", FUN___mst_h_stretch_mode},
    { "mst_v_stretch_mode", FUN___mst_v_stretch_mode},
    { "mst_blink_frame_rate", FUN___mst_blink_frame_rate},
    { "mst_cursor_swap_mode", FUN___mst_cursor_swap_mode},
    { "mst_inputevent_layer", FUN___mst_inputevent_layer},
    { "mst_flip_dump_path", FUN___mst_flip_dump_path},
    { "mst_flip_dump_type", FUN___mst_flip_dump_type},
    { "mst_flip_dump_area", FUN___mst_flip_dump_area},
    { "mst_fix_string_break", FUN___mst_fix_string_break},
    { "mst_argb1555_display", FUN___mst_argb1555_display },
    { "mst_font_dsblit_src_premultiply", FUN___mst_font_dsblit_src_premultiply },
    { "mst_gles2_sdr2hdr", FUN___mst_gles2_sdr2hdr},
    { "mst_ge_hw_limit", FUN___mst_ge_hw_limit},
    { "mst_cmdq_phys_addr", FUN___mst_cmdq_phys_addr},
    { "mst_cmdq_phys_len", FUN___mst_cmdq_phys_len},
    { "mst_cmdq_miusel", FUN___mst_cmdq_miusel},
    { "mst_gop_interlace_adjust", FUN___mst_gop_interlace_adjust},
    { "mst_CFD_monitor_path", FUN___mst_CFD_monitor_path},
    { "mst_layer_buffer_mode", FUN___mst_layer_buffer_mode},
    { "mst_layer_pixelformat", FUN___mst_layer_pixelformat},
    { "mst_do_xc_ip1_patch", FUN___mst_do_xc_ip1_patch},
    { "mst_dump_jpeg_buffer", FUN___mst_dump_jpeg_buffer},
    { "mst_clip_stretchblit_width_high_nonzero_patch", FUN___mst_clip_stretchblit_width_high_nonzero_patch},
    { "mst_bank_force_write", FUN___mst_bank_force_write},
    { "mst_font_bold_enable", FUN___mst_font_bold_enable},
    { "mst_fixed_mem_leak_patch_enable", FUN__mst_fixed_mem_leak_patch_enable},
    { "mst_nice", FUN___mst_nice},
    { "mst_show_fps", FUN___mst_show_fps},
    { "mst_enable_layer_autoscaledown", FUN___mst_enable_layer_autoscaledown},
    { "mst_use_dlopen_dlsym", FUN___mst_use_dlopen_dlsym},
    { "mst_mem_small_size", FUN___mst_mem_small_size},
    { "mst_mem_medium_size", FUN___mst_mem_medium_size},
    { "mst_forbid_fragment_merge_to_api_locked_surface", FUN___mst_forbid_fragment_merge_to_api_locked_surface},
    { "mst_null_display_driver", FUN___mst_null_display_driver},
    { "null_display_driver", FUN___mst_null_display_driver},
    { "mst_call_setkeypadcfg_in_device", FUN___mst_call_setkeypadcfg_in_device},
    { "mst_call_setdfbrccfg_disable", FUN___mst_call_setdfbrccfg_disable},
    { "mst_gwin_disable", FUN___mst_gwin_disable},
    { "mst_using_mi_system", FUN___mst_using_mi_system},
    { "mst_force_wait_vsync", FUN___mst_force_wait_vsync},
    { "mst_new_mstar_linux_input", FUN___mst_new_mstar_linux_input},
    { "mst_mma_pool_enable", FUN___mst_mma_pool_enable},
    { "mst_call_gop_t3d", FUN___mst_call_gop_t3d},
    { "mst_gopc_check_video_info", FUN___mst_gopc_check_video_info},
    { "mst_preinit_fusion_world", FUN___mst_preinit_fusion_world},
    { "mst_font_outline_enable", FUN___mst_font_outline_enable},
    { "mst_bt601_formula", FUN___mst_bt601_formula},
    { "mst_PTK_customized_input_driver_enable", FUN___mst_PTK_customized_input_driver_enable},
    { "mst_layer_fbdev[0]", FUN___mst_layer_fbdev0 },
    { "mst_layer_fbdev[1]", FUN___mst_layer_fbdev1 },
    { "mst_layer_fbdev[2]", FUN___mst_layer_fbdev2 },
    { "mst_layer_fbdev[3]", FUN___mst_layer_fbdev3 },
    { "mst_layer_fbdev[4]", FUN___mst_layer_fbdev4 },
    { "mst_layer_fbdev[5]", FUN___mst_layer_fbdev5 },
    { "mst_debug_mma", FUN___mst_debug_mma},
    { "mst_oom_retry", FUN___mst_oom_retry },
    { "mst_use_system_memory_threshold", FUN___mst_use_system_memory_threshold },
    { "mst_GOP_AutoDetectBuf", FUN___mst_GOP_AutoDetectBuf },
    { "mst_call_disable_bootlogo", FUN___mst_call_disable_bootlogo },
    { "mst_GPU_window_compose", FUN___mst_GPU_window_compose },
    { "mst_debug_secure_mode", FUN___mst_debug_secure_mode },
    { "mst_debug_gles2", FUN___mst_debug_gles2},
    { "mst_layer_up_scaling", FUN___mst_layer_up_scaling},
    { "mst_layer_up_scaling_id", FUN___mst_layer_up_scaling_id},
    { "mst_layer_up_scaling_width", FUN___mst_layer_up_scaling_width},
    { "mst_layer_up_scaling_height", FUN___mst_layer_up_scaling_height},
    { "mst_debug_directory_access", FUN___mst_debug_directory_access},
    { "mst_debug_backtrace_dump", FUN___mst_debug_backtrace_dump},
    { "mst_debug_layer_setConfiguration_return", FUN___mst_debug_layer_setConfiguration_return},
    { "mst_debug_surface_clear_return", FUN___mst_debug_surface_clear_return},
    { "mst_debug_surface_fillrectangle_return", FUN___mst_debug_surface_fillrectangle_return},
    { "mst_GPU_AFBC", FUN___mst_GPU_AFBC},
    { "GPU_AFBC_EXT_SIZE", FUN___GPU_AFBC_EXT_SIZE},
    { "mst_input_vendor_id", FUN___mst_input_vendor_id},
    { "mst_input_product_id", FUN___mst_input_product_id},
    { "mst_gpio_key_code", FUN___mst_gpio_key_code},
    { "mst_debug_cursor", FUN___mst_debug_cursor},
    { "mst_cursor_gwin_width", FUN___mst_cursor_gwin_width},
    { "mst_cursor_gwin_height", FUN___mst_cursor_gwin_height},
    { "mst_hw_cursor", FUN___mst_hw_cursor},
};

void CreateConfigHashTable()
{
    ENTRY e, *ep;
    int i, size, ret;
    size = sizeof(ConfigTable)/sizeof(data_struct_t);

    //hcreate(size);

    memset( &hash, 0, sizeof(hash) );

    ret = hcreate_r(size, &hash);
    if(!ret)
    {
        if (errno == ENOMEM)
            printf("[DFB] hashtable NOMEM, %s, %d\n", __FUNCTION__, __LINE__);
        
        printf("[DFB] hashtable ERROR, %s, %d\n", __FUNCTION__, __LINE__);
    }


    for (i = 0; i < size; i++) 
    {
        e.key = ConfigTable[i].key_string;
        /* data is just an integer, instead of a
          pointer to something */
        e.data = (void*)ConfigTable[i].FuncPtr;

        //ep = hsearch(e, ENTER);

        ret = hsearch_r(e, ENTER, &ep, &hash);
        if(ret == 0)
            printf("[DFB] Hashtable is full %s, %d\n", __FUNCTION__, __LINE__);



        /* there should be no failures */
        if (ep == NULL)
        {
            printf("[DFB] ERROR %s, %d\n", __FUNCTION__, __LINE__);
            
            fprintf(stderr, "entry failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

bool SearchConfigHashTable( const char* name, const char* value)
{
    ENTRY e, *ep;
    /* print two entries from the table, and
    show that two are not in the table */
    e.key = name;
    //ep = hsearch(e, FIND);
    hsearch_r( e, FIND, &ep, &hash );

    //D_INFO("misc  %9.9s -> %9.9s:%d\n", e.key, ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);

    if (ep == NULL)
        return false;

    //data_struct_t *tableData;


    void (*FuncPtr)(char *value);
    
    FuncPtr = (void*)ep->data;
    FuncPtr(value);

    return true;

}

#endif
//end of hashtable
//////////////////////////////////////////



#ifdef CC_DFB_DEBUG_SUPPORT
static void
dump_main( const char *name, const char *value );
#endif

DFBSurfacePixelFormat
dfb_config_parse_pixelformat( const char *format )
{
     int    i;
     size_t length = strlen(format);

     for (i=0; dfb_pixelformat_names[i].format != DSPF_UNKNOWN; i++) {
          if (!direct_strcasecmp( format, dfb_pixelformat_names[i].name ))
               return dfb_pixelformat_names[i].format;
     }

     for (i=0; dfb_pixelformat_names[i].format != DSPF_UNKNOWN; i++) {
          if (!direct_strncasecmp( format, dfb_pixelformat_names[i].name, length ))
               return dfb_pixelformat_names[i].format;
     }

     return DSPF_UNKNOWN;
}

/**********************************************************************************************************************/

static void
print_config_usage( void )
{
     fprintf( stderr, "%s%s%s", config_usage, fusion_config_usage, direct_config_usage );
}

static DFBResult
parse_args( const char *args )
{
     int   len = strlen(args);
     char *buf, *tmp;

     tmp = D_MALLOC( len + 1 );
     if (!tmp)
          return D_OOM();

     buf = tmp;

     direct_memcpy( buf, args, len + 1 );

     while (buf && buf[0]) {
          DFBResult  ret;
          char      *value;
          char      *next;

          if ((next = strchr( buf, ',' )) != NULL)
               *next++ = '\0';

          if (strcmp (buf, "help") == 0) {
               print_config_usage();
               exit(1);
          }

          if (strcmp (buf, "memcpy=help") == 0) {
               direct_print_memcpy_routines();
               exit(1);
          }

          if ((value = strchr( buf, '=' )) != NULL)
               *value++ = '\0';

          ret = dfb_config_set( buf, value );
          switch (ret) {
               case DFB_OK:
                    break;
               case DFB_UNSUPPORTED:
                    D_ERROR( "DirectFB/Config: Unknown option '%s'!\n", buf );
                    break;
               default:
                    D_FREE( tmp );
                    return ret;
          }

          buf = next;
     }

     D_FREE( tmp );

     return DFB_OK;
}

static void config_values_parse( FusionVector *vector, const char *arg )
{
     char *values = D_STRDUP( arg );
     char *s      = values;
     char *r, *p  = NULL;

     if (!values) {
          D_OOM();
          return;
     }

     while ((r = direct_strtok_r( s, ",", &p ))) {
          direct_trim( &r );

          r = D_STRDUP( r );
          if (!r)
               D_OOM();
          else
               fusion_vector_add( vector, r );

          s = NULL;
     }

     D_FREE( values );
}

static void config_values_free( FusionVector *vector )
{
     char *value;
     int   i;

     fusion_vector_foreach (value, i, *vector)
          D_FREE( value );

     fusion_vector_destroy( vector );
     fusion_vector_init( vector, 2, NULL );
}

static int config_read_cmdline( char *cmdbuf, int size, FILE *f )
{
     int ret = 0;
     int len = 0;

     ret = fread( cmdbuf, 1, 1, f );

     /* empty dividing 0 */
     if( ret==1 && *cmdbuf==0 ) {
          ret = fread( cmdbuf, 1, 1, f );
     }

     while(ret==1 && len<(size-1)) {
          len++;
          ret = fread( ++cmdbuf, 1, 1, f );
          if( *cmdbuf == 0 )
               break;
     }

     if( len ) {
          cmdbuf[len]=0;
     }

     return  len != 0;
}

/*
 * The following function isn't used because the configuration should
 * only go away if the application is completely terminated. In that case
 * the memory is freed anyway.
 */

#if 0
static void config_cleanup( void )
{
     if (!dfb_config) {
          D_BUG("config_cleanup() called with no config allocated!");
          return;
     }

     if (dfb_config->fb_device)
          D_FREE( dfb_config->fb_device );

     if (dfb_config->layer_bg_filename)
          D_FREE( dfb_config->layer_bg_filename );

     D_FREE( dfb_config );
     dfb_config = NULL;
}
#endif

/*
 * allocates config and fills it with defaults
 */
static void config_allocate( void )
{
     int i;

     if (dfb_config)
          return;

     dfb_config = (DFBConfig*) calloc( 1, sizeof(DFBConfig) );

     dfb_config->ref = 1;

     for (i=0; i<D_ARRAY_SIZE(dfb_config->layers); i++) {
          dfb_config->layers[i].src_key_index          = -1;

          dfb_config->layers[i].background.color.a     = 0;
          dfb_config->layers[i].background.color.r     = 0;
          dfb_config->layers[i].background.color.g     = 0;
          dfb_config->layers[i].background.color.b     = 0;
          dfb_config->layers[i].background.color_index = -1;
          dfb_config->layers[i].background.mode        = DLBM_COLOR;
     }

     dfb_config->layers[0].init               = true;
     dfb_config->layers[0].background.color.a = 0x0;
     dfb_config->layers[0].background.color.r = 0x0;
     dfb_config->layers[0].background.color.g = 0x0;
     dfb_config->layers[0].background.color.b = 0x0;
     dfb_config->layers[0].stacking           = (1 << DWSC_UPPER)  |
                                                (1 << DWSC_MIDDLE) ;
                                                (1 << DWSC_LOWER);


     dfb_config->pci.bus                  = 1;
     dfb_config->pci.dev                  = 0;
     dfb_config->pci.func                 = 0;

     dfb_config->banner                   = true;
     dfb_config->deinit_check             = true;
     dfb_config->mmx                      = true;
     dfb_config->vt                       = true;
     dfb_config->vt_switch                = true;
     dfb_config->vt_num                   = -1;
     dfb_config->vt_switching             = true;
     dfb_config->kd_graphics              = true;
     dfb_config->translucent_windows      = true;
     dfb_config->font_premult             = true;
     dfb_config->mouse_motion_compression = false;
     dfb_config->mouse_gpm_source         = false;
     dfb_config->mouse_source             = D_STRDUP( DEV_NAME );
     dfb_config->linux_input_grab         = false;
     dfb_config->linux_input_force        = false;
     dfb_config->window_policy            = -1;
     dfb_config->buffer_mode              = -1;
     dfb_config->wm                       = D_STRDUP( "default" );
     dfb_config->decorations              = true;
     dfb_config->unichrome_revision       = -1;
     dfb_config->dma                      = false;
     dfb_config->agp                      = 0;
     dfb_config->matrox_tv_std            = DSETV_PAL;
     dfb_config->i8xx_overlay_pipe_b      = false;
#ifdef CC_PHX_CUST
	 dfb_config->surface_shmpool_size     = 300 * 1024 * 1024;
#else
     dfb_config->surface_shmpool_size     = 8 * 1024 * 1024;
#endif
     dfb_config->keep_accumulators        = 1024;
     dfb_config->font_format              = DSPF_A8;
     dfb_config->cursor_automation        = true;
     
     dfb_config->system_surface_align_base  = 64;
     dfb_config->system_surface_align_pitch = 64;

	 /* default to devmem */
     dfb_config->system = D_STRDUP( "DEVMEM" );

     dfb_config->mst_gfx_gop_index = 0;
     dfb_config->mst_gfx_width = 0;
     dfb_config->mst_gfx_height = 0;
     dfb_config->mst_gfx_v_offset = 0;
     dfb_config->mst_gfx_h_offset = 0;
     dfb_config->default_layer_opacity = -1;
     dfb_config->mst_lcd_width = 0;
     dfb_config->mst_lcd_height = 0;
     dfb_config->mst_ge_vq_phys_addr = 0;
     dfb_config->mst_ge_vq_phys_len = 0;
     dfb_config->mst_gop_regdmaphys_addr = 0;
     dfb_config->mst_gop_regdmaphys_len = 0;
     dfb_config->mst_gop_counts = 1;
     /*the dfb_config->mst_gop_available[i] value should be mutual exclusive*/
     dfb_config->mst_gop_available[0] = 0;//GOP0
     dfb_config->mst_gop_available[1] = 1;
     dfb_config->mst_gop_available[2] = 2;
     dfb_config->mst_gop_available[3] = 3;
     dfb_config->mst_gop_available[4] = 4;
     dfb_config->mst_gop_available[5] = 5;
     dfb_config->mst_gop_available_r[0] = 0x0f;//GOP0
     dfb_config->mst_gop_available_r[1] = 0x0f;
     dfb_config->mst_gop_available_r[2] = 0x0f;
     dfb_config->mst_gop_available_r[3] = 0x0f;
     dfb_config->mst_gop_available_r[4] = 0x0f;
     dfb_config->mst_gop_available_r[5] = 0x0f;
     dfb_config->mst_gop_dstPlane[0] = 2;//E_GOP_DST_OP0
     dfb_config->mst_gop_dstPlane[1] = 2;
     dfb_config->mst_gop_dstPlane[2] = 2;
     dfb_config->mst_gop_dstPlane[3] = 2;
     dfb_config->mst_gop_dstPlane[4] = 2;
     dfb_config->mst_gop_dstPlane[5] = 2;

     /* hw decode jpeg related buf */
     dfb_config->mst_jpeg_readbuff_addr       = 0x0;
     dfb_config->mst_jpeg_readbuff_length     = 0x0;
     dfb_config->mst_jpeg_interbuff_addr      = 0x0;
     dfb_config->mst_jpeg_interbuff_length    = 0x0;
     dfb_config->mst_jpeg_outbuff_addr        = 0x0;
     dfb_config->mst_jpeg_outbuff_length      = 0x0;
     dfb_config->mst_jpeg_hwdecode              = true;

     /* sw decode jpeg related info */
     dfb_config->mst_jpeg_hwdecode_option  = true;

     /* mstar hw dither configurations */
     dfb_config->mst_dither_enable  = false;

     /*mux-gop settings*/
     dfb_config->mst_mux_counts               = 0;
     dfb_config->mst_gop_mux[0]               = 0;
     dfb_config->mst_gop_mux[1]               = 0;
     dfb_config->mst_gop_mux[2]               = 0;
     dfb_config->mst_gop_mux[3]               = 0;
     dfb_config->mst_gop_mux[4]               = 0;
     dfb_config->mst_gop_mux[5]               = 0;

     dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU0_MASK;
 
     /*gop layer settings*/
     dfb_config->mst_goplayer_counts            = 0;
     dfb_config->mst_goplayer[0]                = 0;
     dfb_config->mst_goplayer[1]                = 0;
     dfb_config->mst_goplayer[2]                = 0;
     dfb_config->mst_goplayer[3]                = 0;
     dfb_config->mst_goplayer[4]                = 0;
     dfb_config->mst_goplayer[5]                = 0;



     /*miu settings*/
     dfb_config->mst_miu0_hal_offset            = 0;
     dfb_config->mst_miu0_cpu_offset            = 0;
     dfb_config->mst_miu0_hal_length            = 0x8000000;

     dfb_config->mst_miu1_hal_offset            = 0;
     dfb_config->mst_miu1_cpu_offset            = 0;
     dfb_config->mst_miu1_hal_length            = 0x8000000;


     dfb_config->mst_miu2_hal_offset    = 0;
     dfb_config->mst_miu2_cpu_offset    = 0;
     dfb_config->mst_miu2_hal_length    = 0;

     dfb_config->mst_disable_hwclip = false;
     dfb_config->mst_dfb_register_app_manager = false;


     dfb_config->video_length_secondary       = 0;
     dfb_config->video_phys_secondary_cpu         = 0;
     dfb_config->video_phys_secondary_hal         = 0;
     dfb_config->mst_gop_miu_setting          = 0;
     dfb_config->mst_GOP_HMirror = -1;
     dfb_config->mst_GOP_VMirror = -1;
     dfb_config->mst_layer_bTiled[0]          = false;
     dfb_config->mst_layer_bTiled[1]          = false;
     dfb_config->mst_layer_bTiled[2]          = false;
     dfb_config->mst_layer_bTiled[3]          = false;
     dfb_config->mst_osd_to_ve                  = false;
     dfb_config->mst_xc_to_ve_mux               = 0;
     dfb_config->mst_GOP_Set_YUV           = false;
     dfb_config->do_stretchdown_patch = true;
     dfb_config->line_stretchblit_patch = false;
     dfb_config->static_stretchdown_buf = false;
     dfb_config->mst_png_hwdecode = true;
     dfb_config->mst_forcewrite_DFB = 0x0;
     dfb_config->tvos_mode = false;
     dfb_config->enable_devmem_dump = false;
     dfb_config->do_yuvtorgb_sw_patch = false;
     dfb_config->mst_ir_max_keycode = 0x7FFF;
     dfb_config->bUsingHWTLB = false;
     dfb_config->bGOPUsingHWTLB = false;
     dfb_config->TLBAlignmentSize = 0x8000; //32k
     dfb_config->bPrealloc_map_tlb = false;
     dfb_config->mbootGOPIndex = 0;
     dfb_config->mst_enable_gevq = true;
     dfb_config->mst_disable_layer_init = -1;
     dfb_config->mst_debug_layer = false;
     dfb_config->mst_debug_surface= false;
     dfb_config->mst_debug_input= DFB_DBG_LEVEL_DISABLE;
     dfb_config->mst_debug_ion= false;
     dfb_config->mst_layer_default_width=1280;
     dfb_config->mst_layer_default_height=720;
     dfb_config->full_update_numerator = 9;
     dfb_config->full_update_denominator = 10;
     dfb_config->mst_surface_memory_type = 3;//-1;
     dfb_config->mst_disable_decode_small_jpeg_by_sw = false;
     dfb_config->mst_enable_GOP_Vmirror_Patch = false;     
     dfb_config->do_GE_Vmirror = false;     
     dfb_config->stretchdown_patch_ratio = 1;
     dfb_config->null_driver = false;
     dfb_config->sw_render = SWRF_NONE;
     dfb_config->mst_enable_jpeg_quality = false;
     dfb_config->mst_enable_dip = false;
     dfb_config->mst_dip_mload_addr = 0x0;
     dfb_config->mst_dip_mload_length = 0x0;
     dfb_config->mst_dip_select = 0;
     dfb_config->mst_disable_master_pri_high = false;
     dfb_config->small_font_patch = false;
     dfb_config->freeze_image_by_dfb = false;
     dfb_config->do_i8toargb4444_sw_patch = false;     
     dfb_config->mst_enable_ve_init = false;
     dfb_config->mst_register_gpd = true;
     dfb_config->window_double_buffer = false;
     dfb_config->do_hw_jpg_limit_patch = false;     
     dfb_config->mst_GE_performanceTest = false;
     dfb_config->mst_MIU_protect = false;
     dfb_config->mst_MIU_protect_BlockID = 3;
     //dfb_config->mst_disable_msos = false;
     dfb_config->mst_disable_dfbinfo = false;
     //dfb_config->mst_framebuffer_cma = false;
     dfb_config->mst_cma_heap_id = 26;  //set heap id to 26 default
     dfb_config->mst_sec_cma_heap_id = 26;  //set heap id to 26 default
     dfb_config->mst_mapmem_mode = E_DFB_MEMMAP_MSOS_POOL;   // default msos,  0: devmem  1:msos_mpool  2: cma_pool
     dfb_config->src_color_key_index_patch = 1;
     dfb_config->mst_ir_repeat_time = 250;
     dfb_config->mst_keypad_repeat_time = 1250;
     dfb_config->mst_usbir_repeat_time = 230;
     dfb_config->mst_handle_wm_key = false;
     dfb_config->disable_cjk_string_break = false;
     dfb_config->disable_quick_press = false;
     dfb_config->mst_ir_first_time_out = 0;
     dfb_config->mst_disable_window_scale_patch = false;
     dfb_config->mst_force_flip_wait_tagid = false;
     dfb_config->enable_cursor_mouse_optimize = false;
     dfb_config->layer_support_palette = false;
     dfb_config->stretchblit_with_rotate = true;
     dfb_config->mst_layer_gwin_level = 1;
     dfb_config->mst_enable_gop_gwin_partial_update = false;
     dfb_config->mst_new_alphamode = -1;
     dfb_config->mst_newalphamode_on_layer = 0; //default: none layer will be effect
     /* layer full update  settings*/
     dfb_config->mst_layer_bfullupdate[0]                = false;
     dfb_config->mst_layer_bfullupdate[1]                = false;
     dfb_config->mst_layer_bfullupdate[2]                = false;
     dfb_config->mst_layer_bfullupdate[3]                = false;
     dfb_config->mst_layer_bfullupdate[4]                = false;
     dfb_config->mst_layer_bfullupdate[5]                = false;
     dfb_config->mst_gop_miu2_setting_extend = 0;
     dfb_config->mst_AFBC_layer_enable = 0; // Disable layer 0 ~ layer 5
     dfb_config->mst_AFBC_mode = 0x101; // E_GOP_FB_AFBC_NONSPLT_YUVTRS_ARGB8888=0x101

     dfb_config->mst_enhance_stretchblit_precision = false;


     dfb_config->mst_enable_GLES2 = false;
     dfb_config->mst_mem_peak_usage = 0;
     dfb_config->mst_enable_gwin_multialpha = false;
     dfb_config->mst_measure_png_performance = false;
     dfb_config->mst_rgb2yuv_mode = 1; //default : GFX_YUV_RGB2YUV_255
     dfb_config->mst_bt601_formula = 0;
     dfb_config->mst_debug_cma= false;

     dfb_config->mst_margine_left = 0;
     dfb_config->mst_margine_wright = 0;
     dfb_config->mst_margine_top = 0;
     dfb_config->mst_margine_bottom = 0;

     dfb_config->mst_modify_symbol_by_keymap = true;
     dfb_config->mst_memory_use_cma= false;


     dfb_config->mst_gfxdriver = D_STRDUP(MSTAR_GFX_LIB);

     dfb_config->mst_font_use_video_mem = false;
     dfb_config->dfbinfo_dir = D_STRDUP( DFBINFO_DEFAULT_PATH );
     dfb_config->bus_address_check = true;
     dfb_config->mst_layer_flip_blit = false;
     dfb_config->window_single_buffer = false;
     dfb_config->mst_new_ir = false;
     dfb_config->mst_new_ir_repeat_time = 230;
     dfb_config->mst_new_ir_first_repeat_time = 250;
     dfb_config->mst_t_stretch_mode = DISPLAYER_TRANSPCOLOR_STRCH_DUPLICATE;
     dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_LINEAR;
     dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_4TAPE;//"4TAPE" is the high quality stretch mode of GOP, so default is enabled.
     dfb_config->mst_blink_frame_rate = 0;

     dfb_config->mst_cursor_swap_mode = false;
     dfb_config->mst_inputevent_layer = DLID_PRIMARY;
     dfb_config->mst_flip_dump_path = NULL;
     dfb_config->mst_flip_dump_type = NULL;
     dfb_config->mst_flip_dump_area = (DFBRectangle){.x = 0, .y = 0, .w = 0, .h = 0};
     dfb_config->mst_fix_string_break = true;
     dfb_config->mst_argb1555_display=false;
     dfb_config->mst_font_dsblit_src_premultiply = false;
     dfb_config->mst_gles2_sdr2hdr = false;
     dfb_config->mst_ge_hw_limit = GE_HW_LIMIT;

     dfb_config->mst_cmdq_phys_addr = 0;
     dfb_config->mst_cmdq_phys_len = 0;
     dfb_config->mst_cmdq_miusel = 0xff;

     dfb_config->mst_gop_interlace_adjust = false;

     dfb_config->mst_CFD_monitor_path = NULL;

     dfb_config->mst_layer_buffer_mode = DLBM_UNKNOWN;
     dfb_config->mst_layer_pixelformat = DSPF_UNKNOWN;

     dfb_config->mst_do_xc_ip1_patch = false;

     dfb_config->mst_dump_jpeg_buffer= NULL;

     dfb_config->mst_clip_stretchblit_width_high_nonzero_patch = true;

     dfb_config->mst_bank_force_write = true;

     dfb_config->mst_font_bold_enable = false;

     dfb_config->mst_fixed_mem_leak_patch_enable = false;

     dfb_config->mst_show_fps = false;

     dfb_config->mst_enable_layer_autoscaledown = false;

     dfb_config->mst_use_dlopen_dlsym = false;

     dfb_config->mst_mem_small_size = 0;
     dfb_config->mst_mem_medium_size = 0;

     dfb_config->mst_forbid_fragment_merge_to_api_locked_surface = false;

     dfb_config->mst_null_display_driver = false;

     dfb_config->mst_call_setkeypadcfg_in_device = false;

     dfb_config->mst_call_setdfbrccfg_disable = false;

     dfb_config->mst_gwin_disable = false;

     dfb_config->mst_using_mi_system = false;

     dfb_config->mst_force_wait_vsync = false;

     dfb_config->mst_new_mstar_linux_input = true;

     dfb_config->mst_mma_pool_enable = true;

     dfb_config->mst_call_gop_t3d = -1;

     dfb_config->mst_gopc_check_video_info = true;

     dfb_config->mst_preinit_fusion_world = false;

     dfb_config->mst_font_outline_enable = false;

     dfb_config->mst_PTK_customized_input_driver_enable = false;

     dfb_config->mst_layer_fbdev[0] = NULL;
     dfb_config->mst_layer_fbdev[1] = NULL;
     dfb_config->mst_layer_fbdev[2] = NULL;
     dfb_config->mst_layer_fbdev[3] = NULL;
     dfb_config->mst_layer_fbdev[4] = NULL;
     dfb_config->mst_layer_fbdev[5] = NULL;

     dfb_config->mst_debug_mma = false;
     dfb_config->mst_oom_retry = 2;
     dfb_config->mst_use_system_memory_threshold = 0;
     dfb_config->mst_GOP_AutoDetectBuf = true;
     dfb_config->mst_call_disable_bootlogo = (DFB_SUPPORT_AN == 1) ? false : true;
     dfb_config->mst_GPU_window_compose = false;
     dfb_config->mst_debug_secure_mode = false;
     dfb_config->mst_debug_gles2 = false;
     dfb_config->mst_layer_up_scaling = false;
     dfb_config->mst_layer_up_scaling_id = -1;
     dfb_config->mst_layer_up_scaling_width = 0;
     dfb_config->mst_layer_up_scaling_height = 0;
     dfb_config->mst_GPU_AFBC = false;
#define AFBC_EXT_SIZE 100
     dfb_config->GPU_AFBC_EXT_SIZE = AFBC_EXT_SIZE;
     dfb_config->mst_input_vendor_id = 0;
     dfb_config->mst_input_product_id = 0;
     dfb_config->mst_gpio_key_code = 0x0e;
     dfb_config->video_phys_hal = 0;
     dfb_config->video_length = 0;
     dfb_config->mst_debug_directory_access= D_STRDUP( "/data/dfbdbg" );
     dfb_config->mst_debug_backtrace_dump = false;
     dfb_config->mst_debug_layer_setConfiguration_return = false;
     dfb_config->mst_debug_surface_clear_return = false;
     dfb_config->mst_debug_surface_fillrectangle_return = false;
     for (i= 0; i < MAX_LINUX_INPUT_DEVICES; i++)
     {
        dfb_config->mst_inputdevice[i].name = NULL;
        dfb_config->mst_inputdevice[i].filename = NULL;
     }
     dfb_config->mst_inputdevice[0].name = D_STRDUP( "Hisense Consumer Control" );
     dfb_config->mst_inputdevice[0].filename = D_STRDUP( "/hidata/common/keymap/mufirkeymap_bluetooth" );

     /* hw cursor setting
        mst_hw_cursor : If set > 0, show hardware cursor, usually set to 2
        mst_debug_cursor : hw cursor log
     */

     dfb_config->mst_debug_cursor = false;
     dfb_config->mst_cursor_gwin_width = HW_CURSOR_WIDTH;
     dfb_config->mst_cursor_gwin_height = HW_CURSOR_HEIGHT;
     dfb_config->mst_hw_cursor = -1;

     /*reassignment to mst_probe_window_flip as the env PROBE_WIN_FLIP is set :
        if win_flip=1 : means measure performance
        others means dump files
     */
     char *win_flip = NULL;
     win_flip = getenv("PROBE_WIN_FLIP");
     if (win_flip)
     {
          if (strcmp( win_flip, "1" ) == 0)
               dfb_config->mst_flip_dump_path = D_STRDUP( "measure" );
          else
               dfb_config->mst_flip_dump_path = D_STRDUP( win_flip );
     }

     /* default to no-vt-switch if we don't have root privileges */
     if (direct_geteuid())
          dfb_config->vt_switch = false;

     fusion_vector_init( &dfb_config->linux_input_devices, 2, NULL );
     fusion_vector_init( &dfb_config->tslib_devices, 2, NULL );


     dfb_config->max_font_rows      = 10;//99;
     dfb_config->max_font_row_width = 2048;

     dfb_config->core_sighandler    = true;

	 fusion_config_init();
}

const char *dfb_config_usage( void )
{
     return config_usage;
}

void dfb_config_destroy(void)
{
     int i = 0;
     if (dfb_config == NULL)
        return;

     fusion_config_destroy();

     D_SAFE_FREE(dfb_config->mouse_source);
     D_SAFE_FREE(dfb_config->system);
     D_SAFE_FREE(dfb_config->wm);
     D_SAFE_FREE(dfb_config->fb_device);
     D_SAFE_FREE(dfb_config->mst_gfxdriver);
     D_SAFE_FREE(dfb_config->dfbinfo_dir);
     D_SAFE_FREE(dfb_config->screenshot_dir);
     D_SAFE_FREE(dfb_config->remote.host );
     D_SAFE_FREE(dfb_config->mouse_protocol);
     D_SAFE_FREE(dfb_config->h3600_device);
     D_SAFE_FREE(dfb_config->mut_device);
     D_SAFE_FREE(dfb_config->zytronic_device);
     D_SAFE_FREE(dfb_config->elo_device);
     D_SAFE_FREE(dfb_config->penmount_device);
     D_SAFE_FREE(dfb_config->mst_flip_dump_path);
     D_SAFE_FREE(dfb_config->mst_flip_dump_type);
     D_SAFE_FREE(dfb_config->mst_CFD_monitor_path);
     D_SAFE_FREE(dfb_config->mst_dump_jpeg_buffer);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[0]);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[1]);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[2]);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[3]);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[4]);
     D_SAFE_FREE(dfb_config->mst_layer_fbdev[5]);
     D_SAFE_FREE(dfb_config->mst_debug_directory_access);
     for (i = 0; i < MAX_LINUX_INPUT_DEVICES ; i++)
     {
         if (dfb_config->mst_inputdevice[i].name && dfb_config->mst_inputdevice[i].filename)
         {
            D_SAFE_FREE(dfb_config->mst_inputdevice[i].name);
            D_SAFE_FREE(dfb_config->mst_inputdevice[i].filename);
         }
     }
     if (dfb_config->mst_GPU_window_compose) {
         direct_waitqueue_deinit(&dfb_config->GPU_compose_cond);
         direct_mutex_deinit(&dfb_config->GPU_compose_mutex);
     }
}

int dfb_config_get_layer_stacking(int i4_layer)
{
    int i4_cfg =0x0;
    
    if(i4_layer>= MAX_LAYERS)
    {
         return -1;
    }
    
    if(dfb_config->layers[i4_layer].stacking & (1 << DWSC_LOWER))
    {
        i4_cfg |= DWSC_LOWER;
    }
    
    if(dfb_config->layers[i4_layer].stacking & (1 << DWSC_UPPER))
    {
        i4_cfg |= DWSC_UPPER;
    }
    
    if(dfb_config->layers[i4_layer].stacking & (1 << DWSC_MIDDLE))
    {
        i4_cfg |= DWSC_MIDDLE;
    }
        
    return i4_cfg;
}



int query_miu(const hal_phy addr, hal_phy *offset)
{
    hal_phy miu_offset;
    int miu_select;

    if ( dfb_config->mst_miu2_hal_offset != 0 && addr >= dfb_config->mst_miu2_hal_offset) 
    {
        miu_offset = dfb_config->mst_miu2_hal_offset;
        miu_select = 2;
    }
    else if (dfb_config->mst_miu1_hal_offset != 0 && addr >= dfb_config->mst_miu1_hal_offset) 
    {
        miu_offset = dfb_config->mst_miu1_hal_offset;
        miu_select = 1;
    }    
    else {
        miu_offset = dfb_config->mst_miu0_hal_offset;
        miu_select = 0;
    }
                
    *offset = (addr - miu_offset);   

    return miu_select;
}

void
inputdevice_name_filter_by_bracket( char **s )
{
     int i;
     int len = strlen( *s );

     for (i = len-1; i >= 0; i--){
          if ((*s)[i] == ']'){
               (*s)[i] = 0;
               break;
          }
     }

     while (**s){
          if (**s != '[')
               (*s)++;
          else{
               (*s)++;
               return;
         }
     }
}

void
record_surface_memory_usage(CoreDFBShared *mem_shared,int alloc_size, CoreSurface *surface)
{
    // record max peak usage memory of the system
    mem_shared->currentMem += alloc_size;

    if (mem_shared->currentMem > mem_shared->maxPeakMem)
    {
        mem_shared->maxPeakMem = mem_shared->currentMem;
    }

    D_INFO("\33[0;33;44m [DFB mem] maxPeakMem = %d k \33[0m\n", mem_shared->maxPeakMem >> SHIFT10 );

    // record  memory usage of every memory size
    if ( dfb_config->mst_mem_small_size != 0 && dfb_config->mst_mem_medium_size != 0)
    {
        int PID = surface->object.PID;

        if ((alloc_size >> SHIFT10) <= dfb_config->mst_mem_small_size)
        {
            mem_shared->smallMemCount ++;
            mem_shared->smallMemTotal += alloc_size;
        }
        else if ((alloc_size >> SHIFT10) > dfb_config->mst_mem_small_size && (alloc_size >> SHIFT10) <= dfb_config->mst_mem_medium_size)
        {
            mem_shared->mediumMemCount ++;
            mem_shared->mediumMemTotal += alloc_size;
        }
        else
        {
            mem_shared->largeMemCount ++;
            mem_shared->largeMemTotal += alloc_size;
        }

        D_INFO("\33[0;33;44m [DFB mem][PID: %d ] currentMem = %d k \33[0m\n", PID, mem_shared->currentMem >> SHIFT10);
        D_INFO("\33[0;33;44m [DFB mem][PID: %d ] smallMemCount = %d (0 ~ %d k size) total: %d k\33[0m\n", PID, mem_shared->smallMemCount, dfb_config->mst_mem_small_size, mem_shared->smallMemTotal >> SHIFT10);
        D_INFO("\33[0;33;44m [DFB mem][PID: %d ] mediumMemCount = %d (%d k ~ %d k size) total: %d k\33[0m\n", PID, mem_shared->mediumMemCount, dfb_config->mst_mem_small_size, dfb_config->mst_mem_medium_size, mem_shared->mediumMemTotal >> SHIFT10);
        D_INFO("\33[0;33;44m [DFB mem][PID: %d ] largeMemCount = %d  ( bigger than %d k size ) total: %d k\33[0m\n", PID, mem_shared->largeMemCount, dfb_config->mst_mem_medium_size, mem_shared->largeMemTotal >> SHIFT10);
    }

    // record max peak memory usage of every process
    if ( dfb_config->mst_mem_peak_usage > 0 )
    {
        int PID = surface->object.PID;

        if ( PID < dfb_config->mst_mem_peak_usage )
        {
            if (CSTF_LAYER & surface->type)
            {
                D_INFO("\33[0;33;44m[DFB] layerSurfaceMem[%d] = %dk + %dk\33[0m\n", PID, mem_shared->memInfo[PID].layerSurfaceMem >> SHIFT10, alloc_size >> SHIFT10);

                mem_shared->memInfo[PID].layerSurfaceMem += alloc_size;

                mem_shared->memInfo[PID].bUse = true;
                mem_shared->memInfo[PID].PPID = getppid();

                D_INFO("\33[0;33;44m[DFB] layerSurfaceMem[%d] = %dk \33[0m\n", PID, mem_shared->memInfo[PID].layerSurfaceMem >> SHIFT10);
            }
            else
            {

                D_INFO("\33[0;33;44m[DFB] currentMem[%d] = %dk + %dk\33[0m\n", PID, mem_shared->memInfo[PID].currentMem >> SHIFT10, alloc_size >> SHIFT10);

                mem_shared->memInfo[PID].currentMem += alloc_size;
                mem_shared->memInfo[PID].bUse = true;
                mem_shared->memInfo[PID].PPID = surface->object.PPID;

                if (mem_shared->memInfo[PID].currentMem > mem_shared->memInfo[PID].maxPeakMem)
                    mem_shared->memInfo[PID].maxPeakMem = mem_shared->memInfo[PID].currentMem;

                D_INFO("\33[0;33;44m[DFB] maxPeakMem[%d] = %dk \33[0m\n", PID, mem_shared->memInfo[PID].maxPeakMem >> SHIFT10);

            }
        }
    }
}

//utopia miu hal physical addr up to 64bits
hal_phy _BusAddrToHalAddr(cpu_phy cpuPhysicalAddr)
{
    if ( dfb_config->mst_miu2_cpu_offset != 0 && cpuPhysicalAddr >= dfb_config->mst_miu2_cpu_offset )
        return (cpuPhysicalAddr - dfb_config->mst_miu2_cpu_offset + dfb_config->mst_miu2_hal_offset);
    else if( dfb_config->mst_miu1_cpu_offset != 0 && cpuPhysicalAddr >= dfb_config->mst_miu1_cpu_offset)
        return (cpuPhysicalAddr - dfb_config->mst_miu1_cpu_offset + dfb_config->mst_miu1_hal_offset);
    else
        return (cpuPhysicalAddr - dfb_config->mst_miu0_cpu_offset + dfb_config->mst_miu0_hal_offset);
}

//utopia miu hal physical addr up to 64bits
cpu_phy _HalAddrToBusAddr(hal_phy halPhysicalAddr)
{
    if ( dfb_config->mst_miu2_hal_offset != 0 && halPhysicalAddr >= dfb_config->mst_miu2_hal_offset)
        return (dfb_config->mst_miu2_cpu_offset + halPhysicalAddr - dfb_config->mst_miu2_hal_offset);
    else if( dfb_config->mst_miu1_hal_offset != 0 && halPhysicalAddr >= dfb_config->mst_miu1_hal_offset)
        return dfb_config->mst_miu1_cpu_offset + halPhysicalAddr - dfb_config->mst_miu1_hal_offset;
    else
        return dfb_config->mst_miu0_cpu_offset + halPhysicalAddr - dfb_config->mst_miu0_hal_offset;
}

u32 _mstarGFXAddr(u32 cpuPhysicalAddr)
 {
     if (dfb_config->mst_miu2_cpu_offset != 0 && cpuPhysicalAddr >= dfb_config->mst_miu2_cpu_offset )
         return (cpuPhysicalAddr - dfb_config->mst_miu2_cpu_offset + dfb_config->mst_miu2_hal_offset);
   else if (dfb_config->mst_miu1_cpu_offset != 0 && cpuPhysicalAddr >= dfb_config->mst_miu1_cpu_offset )
        return (cpuPhysicalAddr - dfb_config->mst_miu1_cpu_offset + dfb_config->mst_miu1_hal_offset);
     else
        return (cpuPhysicalAddr - dfb_config->mst_miu0_cpu_offset + dfb_config->mst_miu0_hal_offset);
}

u32 _mstarCPUPhyAddr(u32 halPhysicalAddr)
{
    if (dfb_config->mst_miu2_hal_offset != 0 && halPhysicalAddr >= dfb_config->mst_miu2_hal_offset)
        return (dfb_config->mst_miu2_cpu_offset + halPhysicalAddr - dfb_config->mst_miu2_hal_offset);
    else if (dfb_config->mst_miu1_hal_offset != 0 && halPhysicalAddr >= dfb_config->mst_miu1_hal_offset)
        return (dfb_config->mst_miu1_cpu_offset + halPhysicalAddr - dfb_config->mst_miu1_hal_offset);
    else
        return (dfb_config->mst_miu0_cpu_offset + halPhysicalAddr - dfb_config->mst_miu0_hal_offset);
}

static void update_videomem_addr()
{
     cpu_phy  miu1_cpu_hal_diff = dfb_config->mst_miu1_cpu_offset - dfb_config->mst_miu1_hal_offset;
     cpu_phy  miu2_cpu_hal_diff = dfb_config->mst_miu2_cpu_offset - dfb_config->mst_miu2_hal_offset;

     if (dfb_config->mst_miu2_hal_offset != 0 && dfb_config->video_phys_hal >= dfb_config->mst_miu2_hal_offset)
     {
         dfb_config->video_phys = dfb_config->video_phys_hal + miu2_cpu_hal_diff;
     }
     else if (dfb_config->mst_miu1_hal_offset != 0 && dfb_config->video_phys_hal >= dfb_config->mst_miu1_hal_offset)
     {
         dfb_config->video_phys = dfb_config->video_phys_hal + miu1_cpu_hal_diff;
     }
     else
     {
         dfb_config->video_phys = dfb_config->video_phys_hal + dfb_config->mst_miu0_cpu_offset;
     }

     if (dfb_config->mst_miu2_hal_offset != 0 &&  dfb_config->video_phys_secondary_hal >= dfb_config->mst_miu2_hal_offset)
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + miu2_cpu_hal_diff;
     else if (dfb_config->mst_miu1_hal_offset != 0 && dfb_config->video_phys_secondary_hal >= dfb_config->mst_miu1_hal_offset)
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + miu1_cpu_hal_diff;
     else
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + dfb_config->mst_miu0_cpu_offset;

}

bool check_bus_address_error(cpu_phy u32BA0, cpu_phy u32BA1,  cpu_phy u32BA2)
{
    bool bcheck_bus_error = false;
    if (dfb_config->mst_miu0_cpu_offset != u32BA0)
    {
        bcheck_bus_error = true;
        D_INFO("[DFB] mst_miu0_cpu_offset:0x%llx,  mst_miu0_hal_offset:0x%llx, MsOS_PA2BA::0x%llx \n",dfb_config->mst_miu0_cpu_offset, dfb_config->mst_miu0_hal_offset, u32BA0);
    }
    if (dfb_config->mst_miu1_cpu_offset != 0 && dfb_config->mst_miu1_hal_offset != 0 && (dfb_config->mst_miu1_cpu_offset != u32BA1))
    {
        bcheck_bus_error = true;
        D_INFO("[DFB] mst_miu1_cpu_offset:0x%llx,  mst_miu1_hal_offset:0x%llx, MsOS_PA2BA::0x%llx \n",dfb_config->mst_miu1_cpu_offset, dfb_config->mst_miu1_hal_offset, u32BA1);
    }
    if (dfb_config->mst_miu2_cpu_offset != 0 && dfb_config->mst_miu2_hal_offset != 0 && (dfb_config->mst_miu2_cpu_offset != u32BA2))
    {
        bcheck_bus_error = true;
        D_INFO("[DFB] mst_miu2_cpu_offset:0x%llx,  mst_miu2_hal_offset:0x%llx, MsOS_PA2BA::0x%llx \n",dfb_config->mst_miu2_cpu_offset, dfb_config->mst_miu2_hal_offset, u32BA2);
    }

    if (bcheck_bus_error)
    {
        D_INFO("\n\n[DFB] ================================================================================= \n");
        D_INFO("     <<< FATAL ERROR: BUS ADDRESS SETTING ERROR!!! >>>\n");
        D_INFO("     There is a conflict between the DFB setting and utopia setting(MsOS_PA2BA)\n");
        D_INFO("     Please DOUBLE CHECK which setting is INCORRECT\n");
        D_INFO("     You can consult KERNEL TEAM for the correct bus address offset of miu 0, 1 ,2\n\n");
        D_INFO("     If DFB setting is INCORRECT, you can modify the below DFB setting.\n");
        D_INFO("\n");
        D_INFO("     For SN, please check /config/dfbrc.ini \n");
        D_INFO("     DFBRC_MST_MIU0_CPU_OFFSET : 0x%llx\n", dfb_config->mst_miu0_cpu_offset);
        D_INFO("     DFBRC_MST_MIU1_CPU_OFFSET : 0x%llx\n", dfb_config->mst_miu1_cpu_offset);
        D_INFO("     DFBRC_MST_MIU2_CPU_OFFSET : 0x%llx\n\n", dfb_config->mst_miu2_cpu_offset);
        D_INFO("\n");
        D_INFO("     For DDI or others, please check /config/directfbrc\n");
        D_INFO("     mst_miu0_cpu_offset : 0x%llx\n", dfb_config->mst_miu0_cpu_offset);
        D_INFO("     mst_miu1_cpu_offset : 0x%llx\n", dfb_config->mst_miu1_cpu_offset);
        D_INFO("     mst_miu2_cpu_offset : 0x%llx\n\n", dfb_config->mst_miu2_cpu_offset);
        D_INFO("\n");
        D_INFO("     If utopia setting is INCORRECT, please find an expert of utopia\n");
        D_INFO("     In Utopia setting MsOS_PA2BA\n");
        D_INFO("     [MsOS_PA2BA][MIU0]: PA(0x%llx) => BA(0x%llx)\n", dfb_config->mst_miu0_hal_offset, u32BA0);
        D_INFO("     [MsOS_PA2BA][MIU1]: PA(0x%llx) => BA(0x%llx)\n", dfb_config->mst_miu1_hal_offset, u32BA1);
        D_INFO("     [MsOS_PA2BA][MIU2]: PA(0x%llx) => BA(0x%llx)\n", dfb_config->mst_miu2_hal_offset, u32BA2);
        D_INFO("\n");
        D_INFO("     NOTE: If you want to IGNORE this error check, please append this command into directfbrc of the platform. (e.g. /config/directfbrc)\n");
        D_INFO("     disable_bus_address_check\n");
        D_INFO("[DFB] ================================================================================= \n\n");
    }
    return bcheck_bus_error;
}


static FILE * dfb_config_get_dump_settings_file_handle ()
{
    static bool is_init = false;
    static FILE *fp = NULL;

    if(is_init == false)
    {
        /**
              To avoid invoking "SetExtDFBRcCfg" of dfbinfo, the predumped directfbrc can be
              used to workaround that. There are two steps to introduce how to "Create and
              Apply" the predumped directfbrc.

              Step 1: Create the predumped directfbrc (e.g. /tmp/.directfbrc_PID)
              For example,
              export DFB_DUMP_SETTINGS_PATH=/tmp

              Step 2: Apply the predumped directfbrc
              a. rename .directfbrc_PID to .directfbrc
              b. move to the location you want
              c. export DFBHOME to direct that location
              d. reboot system

              For example,
              mv /tmp/.directfbrc_1234 /application
              export DFBHOME=/application
              reboot

        */
        char*output_path = getenv( "DFB_DUMP_SETTINGS_PATH" );
        char buf[512] = {0};

        snprintf(buf, sizeof(buf), "%s/.directfbrc_%d", output_path, getpid());

        if(output_path && fp == NULL)
            fp = fopen(buf, "w");

        if(fp)
        {
            fputs("mst_call_setdfbrccfg_disable\n", fp);
        }

        is_init = true;
    }

    return fp;

}

void dfb_config_close_dump_settings_file(void)
{
    FILE *fp = dfb_config_get_dump_settings_file_handle();

    if(fp)
        fclose(fp);
}

static void dfb_config_dump_settings(const char* name, const char* value)
{
    FILE *fp = dfb_config_get_dump_settings_file_handle();

    if(fp)
    {
        char buf[128] = {0};
        if(value)
            snprintf(buf, sizeof(buf), "%s=%s\n", name, value);
        else
            snprintf(buf, sizeof(buf), "%s\n", name);

        fputs(buf, fp);
    }
}


#if USE_HASH_TABLE_SREACH

DFBResult dfb_config_set( const char *name, const char *value )
{    
    bool bCheck = true;

    dfb_config_dump_settings(name, value);
    
    bCheck = SearchConfigHashTable(name, value);

    D_INFO("\33[0;33;44m[DFB] config name = %s, value =%s \33[0m\n", name, value);

    if (bCheck == false)
    {
        if (fusion_config_set( name, value ))
            return DFB_UNSUPPORTED;
    }
    
    return DFB_OK;

}

#else
DFBResult dfb_config_set( const char *name, const char *value )
{
#ifdef CC_DFB_DEBUG_SUPPORT
     if(strncmp (name, "dfbdump:", strlen("dfbdump:") ) == 0)
     {
          dump_main(name, value);
     }
     else
#endif      
     if (strcmp (name, "system" ) == 0) {
          if (value) {
               if (dfb_config->system)
                    D_FREE( dfb_config->system );
               dfb_config->system = D_STRDUP( value );
          }
          else {
               D_ERROR("DirectFB/Config 'system': No system specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "wm" ) == 0) {
          if (value) {
               if (dfb_config->wm)
                    D_FREE( dfb_config->wm );
               dfb_config->wm = D_STRDUP( value );
          }
          else {
               D_ERROR("DirectFB/Config 'wm': No window manager specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "fbdev" ) == 0) {
          if (value) {
               if (dfb_config->fb_device)
                    D_FREE( dfb_config->fb_device );
               dfb_config->fb_device = D_STRDUP( value );
          }
          else {
               D_ERROR("DirectFB/Config 'fbdev': No device name specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "busid") == 0 || strcmp (name, "pci-id") == 0) {
          if (value) {
               int bus, dev, func;

               if (direct_sscanf( value, "%d:%d:%d", &bus, &dev, &func ) != 3) {
                    D_ERROR( "DirectFB/Config 'busid': Could not parse busid!\n");
                    return DFB_INVARG;
               }

               dfb_config->pci.bus  = bus;
               dfb_config->pci.dev  = dev;
               dfb_config->pci.func = func;
          }
     } else
     if (strcmp (name, "screenshot-dir" ) == 0) {
          if (value) {
               if (dfb_config->screenshot_dir)
                    D_FREE( dfb_config->screenshot_dir );
               dfb_config->screenshot_dir = D_STRDUP( value );
          }
          else {
               D_ERROR("DirectFB/Config 'screenshot-dir': No directory name specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "scaled" ) == 0) {
          if (value) {
               int width, height;

               if (direct_sscanf( value, "%dx%d", &width, &height ) < 2) {
                    D_ERROR("DirectFB/Config 'scaled': Could not parse size!\n");
                    return DFB_INVARG;
               }

               dfb_config->scaled.width  = width;
               dfb_config->scaled.height = height;
          }
          else {
               D_ERROR("DirectFB/Config 'scaled': No size specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "primary-layer" ) == 0) {
          if (value) {
               int id;

               if (direct_sscanf( value, "%d", &id ) < 1) {
                    D_ERROR("DirectFB/Config 'primary-layer': Could not parse id!\n");
                    return DFB_INVARG;
               }

               dfb_config->primary_layer = id;
          }
          else {
               D_ERROR("DirectFB/Config 'primary-layer': No id specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "primary-only" ) == 0) {
          dfb_config->primary_only = true;
     } else
     if (strcmp (name, "resource-id" ) == 0) {
          if (value) {
               unsigned long long resource_id;

               if (direct_sscanf( value, "%llu", &resource_id ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse id!\n", name);
                    return DFB_INVARG;
               }

               dfb_config->resource_id = resource_id;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No id specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "font-format" ) == 0) {
          if (value) {
               DFBSurfacePixelFormat format;

               format = dfb_config_parse_pixelformat( value );
               if (format == DSPF_UNKNOWN) {
                    D_ERROR("DirectFB/Config 'font-format': Could not parse format!\n");
                    return DFB_INVARG;
               }

               dfb_config->font_format = format;
          }
          else {
               D_ERROR("DirectFB/Config 'font-format': No format specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "font-premult" ) == 0) {
          dfb_config->font_premult = true;
     } else
     if (strcmp (name, "no-font-premult" ) == 0) {
          dfb_config->font_premult = false;
     } else
     if (!strcmp( name, "surface-shmpool-size" )) {
          if (value) {
               int size_kb;

               if (direct_sscanf( value, "%d", &size_kb ) < 1) {
                    D_ERROR( "DirectFB/Config '%s': Could not parse value!\n", name);
                    return DFB_INVARG;
               }

               dfb_config->surface_shmpool_size = size_kb * 1024;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "session" ) == 0) {
          if (value) {
               int session;

               if (direct_sscanf( value, "%d", &session ) < 1) {
                    D_ERROR("DirectFB/Config 'session': Could not parse value!\n");
                    return DFB_INVARG;
               }

               dfb_config->session = session;
          }
          else {
               D_ERROR("DirectFB/Config 'session': No value specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "remote" ) == 0) {
          if (value) {
               char *colon;

               colon = strchr( value, ':' );
               if (colon) {
                    int len  = (long) colon - (long) value;
                    int port = 0;

                    if (direct_sscanf( colon + 1, "%d", &port ) < 1) {
                         D_ERROR("DirectFB/Config 'remote': "
                                 "Could not parse value (format is <host>[:<port>])!\n");
                         return DFB_INVARG;
                    }

                    if (dfb_config->remote.host)
                         D_FREE( dfb_config->remote.host );

                    dfb_config->remote.host = D_MALLOC( len+1 );
                    dfb_config->remote.port = port;

                    direct_snputs( dfb_config->remote.host, value, len+1 );
               }
               else {
                    if (dfb_config->remote.host)
                         D_FREE( dfb_config->remote.host );

                    dfb_config->remote.host = D_STRDUP( value );
                    dfb_config->remote.port = 0;
               }
          }
          else {
               if (dfb_config->remote.host)
                    D_FREE( dfb_config->remote.host );

               dfb_config->remote.host = D_STRDUP( "" );
               dfb_config->remote.port = 0;
          }
     } else
     if (strcmp (name, "videoram-limit" ) == 0) {
          if (value) {
               int limit;

               if (direct_sscanf( value, "%d", &limit ) < 1) {
                    D_ERROR("DirectFB/Config 'videoram-limit': Could not parse value!\n");
                    return DFB_INVARG;
               }

               if (limit < 0)
                    limit = 0;

               dfb_config->videoram_limit = limit << 10;
          }
          else {
               D_ERROR("DirectFB/Config 'videoram-limit': No value specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "keep-accumulators" ) == 0) {
          if (value) {
               int limit;

               if (direct_sscanf( value, "%d", &limit ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse value!\n", name);
                    return DFB_INVARG;
               }

               dfb_config->keep_accumulators = limit;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No value specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "banner" ) == 0) {
          dfb_config->banner = true;
     } else
     if (strcmp (name, "no-banner" ) == 0) {
          dfb_config->banner = false;
     } else
     if (strcmp (name, "surface-sentinel" ) == 0) {
          dfb_config->surface_sentinel = true;
     } else
     if (strcmp (name, "no-surface-sentinel" ) == 0) {
          dfb_config->surface_sentinel = false;
     } else
     if (strcmp (name, "force-windowed" ) == 0) {
          dfb_config->force_windowed = true;
     } else
     if (strcmp (name, "force-desktop" ) == 0) {
          dfb_config->force_desktop = true;
     } else
     if (strcmp (name, "hardware" ) == 0) {
          dfb_config->software_only = false;
     } else
     if (strcmp (name, "no-hardware" ) == 0) {
          dfb_config->software_only = true;
     } else
     if (strcmp (name, "software" ) == 0) {
          dfb_config->hardware_only = false;
     } else
     if (strcmp (name, "no-software" ) == 0) {
          dfb_config->hardware_only = true;
     } else
     if (strcmp (name, "software-warn" ) == 0) {
          dfb_config->software_warn = true;
     } else
     if (strcmp (name, "no-software-warn" ) == 0) {
          dfb_config->software_warn = false;
     } else
     if (strcmp (name, "software-trace" ) == 0) {
          dfb_config->software_trace = true;
     } else
     if (strcmp (name, "no-software-trace" ) == 0) {
          dfb_config->software_trace = false;
     } else
     if (strcmp (name,"small_font_patch") == 0) {
          dfb_config->small_font_patch = true;
     } else
     if (strcmp (name, "warn" ) == 0 || strcmp (name, "no-warn" ) == 0) {
          /* Enable/disable all at once by default. */
          DFBConfigWarnFlags flags = DMT_ALL;

          /* Find out the specific message type being configured. */
          if (value) {
               char *opt = strchr( value, ':' );

               if (opt)
                    opt++;

               if (!strncmp( value, "create-surface", 14 )) {
                    flags = DCWF_CREATE_SURFACE;

                    if (opt)
                         direct_sscanf( opt, "%dx%d",
                                 &dfb_config->warn.create_surface.min_size.w,
                                 &dfb_config->warn.create_surface.min_size.h );
               } else
               if (!strncmp( value, "create-window", 13 )) {
                    flags = DCWF_CREATE_WINDOW;
               } else
               if (!strncmp( value, "allocate-buffer", 15 )) {
                    flags = DCWF_ALLOCATE_BUFFER;

                    if (opt)
                         direct_sscanf( opt, "%dx%d",
                                 &dfb_config->warn.allocate_buffer.min_size.w,
                                 &dfb_config->warn.allocate_buffer.min_size.h );
               }
               else {
                    D_ERROR( "DirectFB/Config '%s': Unknown warning type '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }

          /* Set/clear the corresponding flag in the configuration. */
          if (name[0] == 'w')
               dfb_config->warn.flags |= flags;
          else
               dfb_config->warn.flags &= ~flags;
     } else
     if (strcmp (name, "dma" ) == 0) {
          dfb_config->dma = true;
     } else
     if (strcmp (name, "no-dma" ) == 0) {
          dfb_config->dma = false;
     } else
     if (strcmp (name, "mmx" ) == 0) {
          dfb_config->mmx = true;
     } else
     if (strcmp (name, "no-mmx" ) == 0) {
          dfb_config->mmx = false;
     } else
     if (strcmp (name, "agp" ) == 0) {
          if (value) {
               int mode;

               if (direct_sscanf( value, "%d", &mode ) < 1 || mode < 0 || mode > 8) {
                    D_ERROR( "DirectFB/Config 'agp': "
                             "invalid agp mode '%s'!\n", value );
                    return DFB_INVARG;
               }

               dfb_config->agp = mode;
          }
          else {
               dfb_config->agp = 8; /* maximum possible */
          }
     } else
     if (strcmp (name, "thrifty-surface-buffers" ) == 0) {
          dfb_config->thrifty_surface_buffers = true;
     } else
     if (strcmp (name, "no-thrifty-surface-buffers" ) == 0) {
          dfb_config->thrifty_surface_buffers = false;
     } else
     if (strcmp (name, "no-agp" ) == 0) {
          dfb_config->agp = 0;
     } else
     if (strcmp (name, "agpmem-limit" ) == 0) {
          if (value) {
               int limit;

               if (direct_sscanf( value, "%d", &limit ) < 1) {
                    D_ERROR( "DirectFB/Config 'agpmem-limit': "
                             "Could not parse value!\n" );
                    return DFB_INVARG;
               }

               if (limit < 0)
                    limit = 0;

               dfb_config->agpmem_limit = limit << 10;
          }
          else {
               D_ERROR("DirectFB/Config 'agpmem-limit': No value specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "vt" ) == 0) {
          dfb_config->vt = true;
     } else
     if (strcmp (name, "no-vt" ) == 0) {
          dfb_config->vt = false;
     } else
     if (strcmp (name, "block-all-signals" ) == 0) {
          dfb_config->block_all_signals = true;
     } else
     if (strcmp (name, "core-sighandler" ) == 0) {
          dfb_config->core_sighandler = true;
     } else
     if (strcmp (name, "no-core-sighandler" ) == 0) {
          dfb_config->core_sighandler = false;
     } else
     if (strcmp (name, "deinit-check" ) == 0) {
          dfb_config->deinit_check = true;
     } else
     if (strcmp (name, "no-deinit-check" ) == 0) {
          dfb_config->deinit_check = false;
     } else
     if (strcmp (name, "cursor" ) == 0) {
          dfb_config->no_cursor = false;
          D_INFO("[DFB][%s %d] set cursor pid=%d\n", __FUNCTION__, __LINE__, getpid());
     } else
     if (strcmp (name, "no-cursor" ) == 0) {
          dfb_config->no_cursor = true;
     } else
     if (strcmp (name, "cursor-automation" ) == 0) {
          dfb_config->cursor_automation = true;
     } else
     if (strcmp (name, "no-cursor-automation" ) == 0) {
          dfb_config->cursor_automation = false;
     } else
     if (strcmp (name, "cursor-updates" ) == 0) {
          dfb_config->no_cursor_updates = false;
     } else
     if (strcmp (name, "no-cursor-updates" ) == 0) {
          dfb_config->no_cursor_updates = true;
     } else
     if (strcmp (name, "linux-input-ir-only" ) == 0) {
          dfb_config->linux_input_ir_only = true;
     } else
     if (strcmp (name, "linux-input-grab" ) == 0) {
          dfb_config->linux_input_grab = true;
     } else
     if (strcmp (name, "no-linux-input-grab" ) == 0) {
          dfb_config->linux_input_grab = false;
     } else
     if (strcmp (name, "linux-input-force" ) == 0) {
          dfb_config->linux_input_force = true;
     } else
     if (strcmp (name, "no-linux-input-force" ) == 0) {
          dfb_config->linux_input_force = false;
     } else
     if (strcmp (name, "motion-compression" ) == 0) {
          dfb_config->mouse_motion_compression = true;
     } else
     if (strcmp (name, "no-motion-compression" ) == 0) {
          dfb_config->mouse_motion_compression = false;
     } else
     if (strcmp (name, "mouse-protocol" ) == 0) {
          if (value) {
               dfb_config->mouse_protocol = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No mouse protocol specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mouse-source" ) == 0) {
          if (value) {
               D_FREE( dfb_config->mouse_source );
               dfb_config->mouse_source = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No mouse source specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mouse-gpm-source" ) == 0) {
          dfb_config->mouse_gpm_source = true;
         D_FREE( dfb_config->mouse_source );
         dfb_config->mouse_source = D_STRDUP( DEV_NAME_GPM );
     } else
     if (strcmp (name, "no-mouse-gpm-source" ) == 0) {
          dfb_config->mouse_gpm_source = false;
          D_FREE( dfb_config->mouse_source );
          dfb_config->mouse_source = D_STRDUP( DEV_NAME );
     } else
     if (strcmp (name, "smooth-upscale" ) == 0) {
          dfb_config->render_options |= DSRO_SMOOTH_UPSCALE;
     } else
     if (strcmp (name, "no-smooth-upscale" ) == 0) {
          dfb_config->render_options &= ~DSRO_SMOOTH_UPSCALE;
     } else
     if (strcmp (name, "smooth-downscale" ) == 0) {
          dfb_config->render_options |= DSRO_SMOOTH_DOWNSCALE;
     } else
     if (strcmp (name, "no-smooth-downscale" ) == 0) {
          dfb_config->render_options &= ~DSRO_SMOOTH_DOWNSCALE;
     } else
     if (strcmp (name, "translucent-windows" ) == 0) {
          dfb_config->translucent_windows = true;
     } else
     if (strcmp (name, "no-translucent-windows" ) == 0) {
          dfb_config->translucent_windows = false;
     } else
     if (strcmp (name, "decorations" ) == 0) {
          dfb_config->decorations = true;
     } else
     if (strcmp (name, "no-decorations" ) == 0) {
          dfb_config->decorations = false;
     } else
     if (strcmp (name, "startstop" ) == 0) {
          dfb_config->startstop = true;
     } else
     if (strcmp (name, "no-startstop" ) == 0) {
          dfb_config->startstop = false;
     } else
     if (strcmp (name, "autoflip-window" ) == 0) {
          dfb_config->autoflip_window = true;
     } else
     if (strcmp (name, "no-autoflip-window" ) == 0) {
          dfb_config->autoflip_window = false;
     } else
     if (strcmp (name, "vsync-none" ) == 0) {
          dfb_config->pollvsync_none = true;
     } else
     if (strcmp (name, "vsync-after" ) == 0) {
          dfb_config->pollvsync_after = true;
     } else
     if (strcmp (name, "vt-switch" ) == 0) {
          dfb_config->vt_switch = true;
     } else
     if (strcmp (name, "no-vt-switch" ) == 0) {
          dfb_config->vt_switch = false;
     } else
     if (strcmp (name, "vt-num" ) == 0) {
          if (value) {
               int vt_num;

               if (direct_sscanf( value, "%d", &vt_num ) < 1) {
                    D_ERROR("DirectFB/Config 'vt-num': Could not parse value!\n");
                    return DFB_INVARG;
               }

               dfb_config->vt_num = vt_num;
          }
          else {
               D_ERROR("DirectFB/Config 'vt-num': No value specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "vt-switching" ) == 0) {
          dfb_config->vt_switching = true;
     } else
     if (strcmp (name, "no-vt-switching" ) == 0) {
          dfb_config->vt_switching = false;
     } else
     if (strcmp (name, "graphics-vt" ) == 0) {
          dfb_config->kd_graphics = true;
     } else
     if (strcmp (name, "no-graphics-vt" ) == 0) {
          dfb_config->kd_graphics = false;
     } else
#if !DIRECTFB_BUILD_PURE_VOODOO
     if (strcmp (name, "window-surface-policy" ) == 0) {
          if (value) {
               if (strcmp( value, "auto" ) == 0) {
                    dfb_config->window_policy = -1;
               } else
               if (strcmp( value, "videohigh" ) == 0) {
                    dfb_config->window_policy = CSP_VIDEOHIGH;
               } else
               if (strcmp( value, "videolow" ) == 0) {
                    dfb_config->window_policy = CSP_VIDEOLOW;
               } else
               if (strcmp( value, "systemonly" ) == 0) {
                    dfb_config->window_policy = CSP_SYSTEMONLY;
               } else
               if (strcmp( value, "videoonly" ) == 0) {
                    dfb_config->window_policy = CSP_VIDEOONLY;
               }
               else {
                    D_ERROR( "DirectFB/Config: "
                             "Unknown window surface policy `%s'!\n", value );
                    return DFB_INVARG;
               }
          }
          else {
               D_ERROR( "DirectFB/Config: "
                        "No window surface policy specified!\n" );
               return DFB_INVARG;
          }
     } else
#endif
     if (strcmp (name, "init-layer" ) == 0) {
          if (value) {
               int id;

               if (direct_sscanf( value, "%d", &id ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse id!\n", name);
                    return DFB_INVARG;
               }

               if (id < 0 || id >= D_ARRAY_SIZE(dfb_config->layers)) {
                    D_ERROR("DirectFB/Config '%s': ID %d out of bounds!\n", name, id);
                    return DFB_INVARG;
               }

               dfb_config->layers[id].init = true;

               dfb_config->config_layer = &dfb_config->layers[id];
          }
          else {
               D_ERROR("DirectFB/Config '%s': No id specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "no-init-layer" ) == 0) {
          if (value) {
               int id;

               if (direct_sscanf( value, "%d", &id ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse id!\n", name);
                    return DFB_INVARG;
               }

               if (id < 0 || id >= D_ARRAY_SIZE(dfb_config->layers)) {
                    D_ERROR("DirectFB/Config '%s': ID %d out of bounds!\n", name, id);
                    return DFB_INVARG;
               }

               dfb_config->layers[id].init = false;

               dfb_config->config_layer = &dfb_config->layers[id];
          }
          else
               dfb_config->layers[0].init = false;
     } else
     if (strcmp (name, "mode" ) == 0 || strcmp (name, "layer-size" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               int width, height;

               if (direct_sscanf( value, "%dx%d", &width, &height ) < 2) {
                    D_ERROR("DirectFB/Config '%s': Could not parse width and height!\n", name);
                    return DFB_INVARG;
               }

               if (conf == &dfb_config->layers[0]) {
                    dfb_config->mode.width  = width;
                    dfb_config->mode.height = height;
               }

               conf->config.width  = width;
               conf->config.height = height;

               conf->config.flags |= DLCONF_WIDTH | DLCONF_HEIGHT;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No width and height specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "depth" ) == 0 || strcmp (name, "layer-depth" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               int depth;

               if (direct_sscanf( value, "%d", &depth ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse value!\n", name);
                    return DFB_INVARG;
               }

               if (conf == &dfb_config->layers[0]) {
                    dfb_config->mode.depth = depth;
               }

               conf->config.pixelformat = dfb_pixelformat_for_depth( depth );
               conf->config.flags      |= DLCONF_PIXELFORMAT;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No value specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "pixelformat" ) == 0 || strcmp (name, "layer-format" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               DFBSurfacePixelFormat format;

               format = dfb_config_parse_pixelformat( value );
               if (format == DSPF_UNKNOWN) {
                    D_ERROR("DirectFB/Config '%s': Could not parse format!\n", name);
                    return DFB_INVARG;
               }

               if (conf == &dfb_config->layers[0])
                    dfb_config->mode.format = format;

               conf->config.pixelformat = format;
               conf->config.flags      |= DLCONF_PIXELFORMAT;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No format specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "desktop-buffer-mode" ) == 0 || strcmp (name, "layer-buffer-mode" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               if (strcmp( value, "auto" ) == 0) {
                    conf->config.flags &= ~DLCONF_BUFFERMODE;
               } else
               if (strcmp( value, "triple" ) == 0) {
                    conf->config.buffermode = DLBM_TRIPLE;
                    conf->config.flags     |= DLCONF_BUFFERMODE;
               } else
               if (strcmp( value, "backvideo" ) == 0) {
                    conf->config.buffermode = DLBM_BACKVIDEO;
                    conf->config.flags     |= DLCONF_BUFFERMODE;
               } else
               if (strcmp( value, "backsystem" ) == 0) {
                    conf->config.buffermode = DLBM_BACKSYSTEM;
                    conf->config.flags     |= DLCONF_BUFFERMODE;
               } else
               if (strcmp( value, "frontonly" ) == 0) {
                    conf->config.buffermode = DLBM_FRONTONLY;
                    conf->config.flags     |= DLCONF_BUFFERMODE;
               } else
               if (strcmp( value, "windows" ) == 0) {
                    conf->config.buffermode = DLBM_WINDOWS;
                    conf->config.flags     |= DLCONF_BUFFERMODE;
               } else {
                    D_ERROR( "DirectFB/Config '%s': Unknown mode '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No buffer mode specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "layer-src-key" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               char *error;
               u32   argb;

               argb = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in color '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               conf->src_key.b = argb & 0xFF;
               argb >>= 8;
               conf->src_key.g = argb & 0xFF;
               argb >>= 8;
               conf->src_key.r = argb & 0xFF;
               argb >>= 8;
               conf->src_key.a = argb & 0xFF;

               conf->config.options |= DLOP_SRC_COLORKEY;
               conf->config.flags   |= DLCONF_OPTIONS;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No color specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "layer-src-key-index" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               char *error;
               u32   index;

               index = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in index '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               conf->src_key_index = index;
               conf->config.options |= DLOP_SRC_COLORKEY;
               conf->config.flags   |= DLCONF_OPTIONS;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No index specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "bg-none" ) == 0 || strcmp (name, "layer-bg-none" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          conf->background.mode = DLBM_DONTCARE;
     } else
     if (strcmp (name, "bg-image" ) == 0 || strcmp (name, "bg-tile" ) == 0 ||
         strcmp (name, "layer-bg-image" ) == 0 || strcmp (name, "layer-bg-tile" ) == 0)
     {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               if (conf->background.filename)
                    D_FREE( conf->background.filename );

               conf->background.filename = D_STRDUP( value );
               conf->background.mode     = strcmp (name, "bg-tile" ) ? DLBM_IMAGE : DLBM_TILE;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No filename specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "bg-color" ) == 0 || strcmp (name, "layer-bg-color" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               char *error;
               u32   argb;

               argb = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in color '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               conf->background.color.b = argb & 0xFF;
               argb >>= 8;
               conf->background.color.g = argb & 0xFF;
               argb >>= 8;
               conf->background.color.r = argb & 0xFF;
               argb >>= 8;
               conf->background.color.a = argb & 0xFF;

               conf->background.color_index = -1;
               conf->background.mode        = DLBM_COLOR;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No color specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "layer-bg-color-index" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               char *error;
               u32   index;

               index = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in index '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               conf->background.color_index = index;
               conf->background.mode        = DLBM_COLOR;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No index specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "layer-stacking" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               char *stackings = D_STRDUP( value );
               char *p = NULL, *r, *s = stackings;

               conf->stacking = 0;

               while ((r = direct_strtok_r( s, ",", &p ))) {
                    direct_trim( &r );

                    if (!strcmp( r, "lower" ))
                         conf->stacking |= (1 << DWSC_LOWER);
                    else if (!strcmp( r, "middle" ))
                         conf->stacking |= (1 << DWSC_MIDDLE);
                    else if (!strcmp( r, "upper" ))
                         conf->stacking |= (1 << DWSC_UPPER);
                    else {
                         D_ERROR( "DirectFB/Config '%s': Unknown class '%s'!\n", name, r );
                         D_FREE( stackings );
                         return DFB_INVARG;
                    }

                    s = NULL;
               }

               D_FREE( stackings );
          }
          else {
               D_ERROR( "DirectFB/Config '%s': Missing value!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strncmp (name, "layer-palette-", 14 ) == 0) {
          int             index;
          char           *error;
          DFBConfigLayer *conf = dfb_config->config_layer;

          index = strtoul( name + 14, &error, 10 );

          if (*error) {
               D_ERROR( "DirectFB/Config '%s': Error in index '%s'!\n", name, error );
               return DFB_INVARG;
          }

          if (index < 0 || index > 255) {
               D_ERROR("DirectFB/Config '%s': Index %d out of bounds!\n", name, index);
               return DFB_INVARG;
          }

          if (value) {
               char *error;
               u32   argb;

               argb = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in color '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               if (!conf->palette) {
                    conf->palette = D_CALLOC( 256, sizeof(DFBColor) );
                    if (!conf->palette)
                         return D_OOM();
               }

               conf->palette[index].a = (argb & 0xFF000000) >> 24;
               conf->palette[index].r = (argb & 0xFF0000) >> 16;
               conf->palette[index].g = (argb & 0xFF00) >> 8;
               conf->palette[index].b = (argb & 0xFF);

               conf->palette_set = true;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No color specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "layer-rotate" ) == 0) {
          DFBConfigLayer *conf = dfb_config->config_layer;

          if (value) {
               int rotate;

               if (direct_sscanf( value, "%d", &rotate ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse value!\n", name);
                    return DFB_INVARG;
               }

               if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
                    D_ERROR("DirectFB/Config '%s': Only 0, 90, 180 or 270 supported!\n", name);
                    return DFB_UNSUPPORTED;
               }

               conf->rotate = rotate;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No value specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "video-phys" ) == 0) {
          if (value) {
               char *error;
               u64    phys;

               phys = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in hex value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->video_phys_hal = phys;
			   update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "video-length" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->video_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "video-phys-secondary" ) == 0) {
          if (value) {
               char *error;
               u64 phys;

               phys = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in hex value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->video_phys_secondary_hal= phys;
               update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "video-length-secondary" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->video_length_secondary = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mmio-phys" ) == 0) {
          if (value) {
               char *error;
               u64 phys;

               phys = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in hex value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mmio_phys = phys;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mmio-length" ) == 0) {
          if (value) {
               char *error;
               unsigned long length;

               length = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mmio_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "accelerator" ) == 0) {
          if (value) {
               char *error;
               unsigned long accel;

               accel = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->accelerator = accel;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "max-font-rows" ) == 0) {
          if (value) {
               char *error;
               unsigned long num;

               num = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->max_font_rows = num;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "max-font-row-width" ) == 0) {
          if (value) {
               char *error;
               unsigned long width;

               width = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->max_font_row_width = width;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "matrox-tv-standard" ) == 0) {
          if (value) {
               if (strcmp( value, "pal-60" ) == 0) {
                    dfb_config->matrox_tv_std = DSETV_PAL_60;
               } else
               if (strcmp( value, "pal" ) == 0) {
                    dfb_config->matrox_tv_std = DSETV_PAL;
               } else
               if (strcmp( value, "ntsc" ) == 0) {
                    dfb_config->matrox_tv_std = DSETV_NTSC;
               } else {
                    D_ERROR( "DirectFB/Config: Unknown TV standard "
                             "'%s'!\n", value );
                    return DFB_INVARG;
               }
          }
          else {
               D_ERROR( "DirectFB/Config: "
                        "No TV standard specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "matrox-cable-type" ) == 0) {
          if (value) {
               if (strcmp( value, "composite" ) == 0) {
                    dfb_config->matrox_cable = 0;
               } else
               if (strcmp( value, "scart-rgb" ) == 0) {
                    dfb_config->matrox_cable = 1;
               } else
               if (strcmp( value, "scart-composite" ) == 0) {
                    dfb_config->matrox_cable = 2;
               } else {
                    D_ERROR( "DirectFB/Config: Unknown cable type "
                             "'%s'!\n", value );
                    return DFB_INVARG;
               }
          }
          else {
               D_ERROR( "DirectFB/Config: "
                        "No cable type specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "x11-borderless" ) == 0) {
          dfb_config->x11_borderless = true;

          if (value && direct_sscanf( value, "%d.%d", &dfb_config->x11_position.x, &dfb_config->x11_position.y ) < 2) {
               D_ERROR("DirectFB/Config '%s': Could not parse x and y!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "matrox-sgram" ) == 0) {
          dfb_config->matrox_sgram = true;
     } else
     if (strcmp (name, "matrox-crtc2" ) == 0) {
          dfb_config->matrox_crtc2 = true;
     } else
     if (strcmp (name, "no-matrox-sgram" ) == 0) {
          dfb_config->matrox_sgram = false;
     } else
     if (strcmp (name, "sync" ) == 0) {
          dfb_config->sync = true;
     } else
     if (strcmp (name, "no-sync" ) == 0) {
          dfb_config->sync = false;
     } else
     if (strcmp (name, "lefty" ) == 0) {
          dfb_config->lefty = true;
     } else
     if (strcmp (name, "no-lefty" ) == 0) {
          dfb_config->lefty = false;
     } else
     if (strcmp (name, "capslock-meta" ) == 0) {
          dfb_config->capslock_meta = true;
     } else
     if (strcmp (name, "no-capslock-meta" ) == 0) {
          dfb_config->capslock_meta = false;
     } else
     if (strcmp (name, "h3600-device" ) == 0) {
          if (value) {
               if (dfb_config->h3600_device)
                    D_FREE( dfb_config->h3600_device );

               dfb_config->h3600_device = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No H3600 TS device specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mut-device" ) == 0) {
          if (value) {
               if (dfb_config->mut_device)
                    D_FREE( dfb_config->mut_device );

               dfb_config->mut_device = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No MuTouch device specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "zytronic-device" ) == 0) {
          if (value) {
               if (dfb_config->zytronic_device)
                    D_FREE( dfb_config->zytronic_device );

               dfb_config->zytronic_device = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No Zytronic device specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "elo-device" ) == 0) {
          if (value) {
               if (dfb_config->elo_device)
                    D_FREE( dfb_config->elo_device );

               dfb_config->elo_device = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No Elo device specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "penmount-device" ) == 0) {
          if (value) {
               if (dfb_config->penmount_device)
                    D_FREE( dfb_config->penmount_device );

               dfb_config->penmount_device = D_STRDUP( value );
          }
          else {
               D_ERROR( "DirectFB/Config: No PenMount device specified!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "linux-input-devices" ) == 0) {
          if (value) {
               config_values_free( &dfb_config->linux_input_devices );
               config_values_parse( &dfb_config->linux_input_devices, value );
          }
          else {
               D_ERROR( "DirectFB/Config: Missing value for linux-input-devices!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "tslib-devices" ) == 0) {
          if (value) {
               config_values_free( &dfb_config->tslib_devices );
               config_values_parse( &dfb_config->tslib_devices, value );
          }
          else {
               D_ERROR( "DirectFB/Config: Missing value for tslib-devices!\n" );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "unichrome-revision" ) == 0) {
          if (value) {
               int rev;

               if (direct_sscanf( value, "%d", &rev ) < 1) {
                    D_ERROR("DirectFB/Config 'unichrome-revision': Could not parse revision!\n");
                    return DFB_INVARG;
               }

               dfb_config->unichrome_revision = rev;
          }
          else {
               D_ERROR("DirectFB/Config 'unichrome-revision': No revision specified!\n");
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "i8xx_overlay_pipe_b") == 0) {
          dfb_config->i8xx_overlay_pipe_b = true;
     } else
     if (strcmp (name, "max-axis-rate" ) == 0) {
          if (value) {
               unsigned int rate;

               if (direct_sscanf( value, "%u", &rate ) < 1) {
                    D_ERROR("DirectFB/Config '%s': Could not parse value!\n", name);
                    return DFB_INVARG;
               }

               dfb_config->max_axis_rate = rate;
          }
          else {
               D_ERROR("DirectFB/Config '%s': No value specified!\n", name);
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "include") == 0) {
          if( value ) {
               DFBResult ret;
               ret = dfb_config_read( value );
               if( ret )
                    return ret;
          }
          else {
               D_ERROR("DirectFB/Config 'include': No include file specified!\n");
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_gfx_width") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gfx_width = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_gfx_height") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gfx_height = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_lcd_width") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_lcd_width = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_lcd_height") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_lcd_height = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_ge_vq_phys_addr") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_ge_vq_phys_addr = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_ge_vq_phys_len") == 0) {

          if (value) {
               char *error = NULL;
               unsigned long val = 0;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_ge_vq_phys_len = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_gop_regdma_phys_addr") == 0) {

          if (value) {
               char *error = NULL;
               u64 val = 0;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gop_regdmaphys_addr = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_gop_regdma_phys_len") == 0) {

          if (value) {
               char *error = NULL;
               unsigned long val = 0;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gop_regdmaphys_len = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu0_hal_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu0_hal_offset = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu0_hal_length") == 0) {

          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu0_hal_length = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu0_cpu_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu0_cpu_offset = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu1_hal_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu1_hal_offset = val;
               update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu1_hal_length") == 0) {

          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu1_hal_length = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
      if (strcmp (name, "mst_miu1_cpu_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }
               dfb_config->mst_miu1_cpu_offset = val;
               update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu2_hal_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu2_hal_offset = val;
               update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_miu2_hal_length") == 0) {

          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_miu2_hal_length = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
      if (strcmp (name, "mst_miu2_cpu_offset") == 0) {

          if (value) {
               char *error;
               u64 val;
               val = strtoull( value, &error, 16 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }
               dfb_config->mst_miu2_cpu_offset = val;
               update_videomem_addr();
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_gfx_h_offset") == 0) {

          if (value) {
               char *error;
              unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gfx_h_offset = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
     if (strcmp (name, "mst_gfx_v_offset") == 0) {
          if (value) {
               char *error;
              unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gfx_v_offset = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }else
      if (strcmp (name, "mst_gfx_gop_index") == 0) {
          if (value) {
               char *error;
              unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_gfx_gop_index = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }

     }
     else if (strcmp (name, "default_layer_opacity") == 0) {
          if (value) {
               char *error;
              unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->default_layer_opacity = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else
     if (strcmp (name,"mst_gop_counts") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_counts = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available[0]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[0] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
    }else
    if (strcmp (name,"mst_gop_available[1]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[1] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available[2]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[2] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available[3]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[3] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available[4]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[4] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available[5]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available[5] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
      if (strcmp (name,"mst_gop_available_r[0]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[0] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_available_r[1]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[1] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
      if (strcmp (name,"mst_gop_available_r[2]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[2] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
      if (strcmp (name,"mst_gop_available_r[3]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[3] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
      if (strcmp (name,"mst_gop_available_r[4]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[4] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
      if (strcmp (name,"mst_gop_available_r[5]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_available_r[5] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[0]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[0] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[1]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[1] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[2]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[2] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[3]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[3] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[4]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[4] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_dstPlane[5]") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_dstPlane[5] = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name, "mst_jpeg_readbuff_addr" ) == 0) {
          if (value) {
               char *error;
               u64 val;

               val = strtoull( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_readbuff_addr = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_jpeg_readbuff_length" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_readbuff_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_jpeg_interbuff_addr" ) == 0) {
          if (value) {
               char *error;
               u64 val;

               val = strtoull( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_interbuff_addr= val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_jpeg_interbuff_length" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_interbuff_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } else
     if (strcmp (name, "mst_jpeg_outbuff_addr" ) == 0) {
          if (value) {
               char *error;
               u64 val;

               val = strtoull( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_outbuff_addr = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
    } else
    if (strcmp (name, "mst_jpeg_outbuff_length" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_jpeg_outbuff_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
    } else
    if(strcmp(name,"mst_jpeg_hwdecode") == 0){
        dfb_config->mst_jpeg_hwdecode = true;
    } else
    if (strcmp (name,"muxCounts") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_mux_counts= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux0_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[0]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux1_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[1]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux2_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[2]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux3_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[3]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux4_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[4]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mux5_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_mux[5]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayerCount") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer_counts = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer0_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[0]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer1_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[1]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer2_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[2]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer3_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[3]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer4_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[4]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"goplayer5_gopIndex") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_goplayer[5]= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if (strcmp (name,"mst_gop_miu_setting") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_miu_setting = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }else
     if(strcmp (name,"mst_disable_hwclip") == 0)
     {
           dfb_config->mst_disable_hwclip = true;
     }
     else
     if(strcmp(name,"mst_reg_appm") == 0)
     {
             dfb_config->mst_dfb_register_app_manager = true;
     }
     else
     if(strcmp (name,"mst_enable_GOP_HMirror") == 0)
        dfb_config->mst_GOP_HMirror = 1;
     else
     if(strcmp (name,"mst_disable_GOP_HMirror") == 0)
        dfb_config->mst_GOP_HMirror = 0;
     else
     if(strcmp (name,"mst_enable_GOP_VMirror") == 0)
        dfb_config->mst_GOP_VMirror = 1;
     else
     if(strcmp (name,"mst_disable_GOP_VMirror") == 0)
        dfb_config->mst_GOP_VMirror = 0;
     else
     if(strcmp (name,"mst_osd_to_ve") == 0)
     {
        dfb_config->mst_osd_to_ve = true;
        if (value) {
             //char *error;
             unsigned long val = 0;
            if(strcmp(value,"VOP_SEL_OSD_BLEND0")==0)
            {
                dfb_config->mst_xc_to_ve_mux = 0;
            }
            else if(strcmp(value,"VOP_SEL_OSD_BLEND1")==0)
            {
                dfb_config->mst_xc_to_ve_mux = 1;
            }
            else if(strcmp(value,"VOP_SEL_OSD_BLEND2")==0)
            {
                 dfb_config->mst_xc_to_ve_mux = 2;
            }
            else if(strcmp(value,"VOP_SEL_MACE_RGB")==0)
            {
               dfb_config->mst_xc_to_ve_mux = 3;
            }
            else if(strcmp(value,"VOP_SEL_OSD_NONE")==0)
            {
                dfb_config->mst_xc_to_ve_mux = 4;
            }
            else
            {
                char *error;        
                val = strtoul( value, &error, 16);

                if (*error)        
                    printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );            

                dfb_config->mst_xc_to_ve_mux = val;

            }
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if(strcmp (name,"keypad_callback") == 0)
     {
        if (value)
        {
               char *error;
               ulong addr;

               addr = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->keypadCallback = (keypad_func)addr;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     
     //mstar patch start
     /*patch for mantis: 709338:[065B][ISDB][add keypad operation in app then coredump]
       this patch is  used by sn0401 branch only. The branch uses a set of independent keypad module without 
       use  input module  provided by DFB
    */
     else if(strcmp (name,"keypad_callback2") == 0)
     {
        if (value)
        {
               char *error;
               ulong addr;

               addr = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->keypadCallback2 = (keypad_func2)addr;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     //mstar patch end
     
     else if(strcmp (name,"keypad_internal") == 0)
     {
        if (value)
        {
               char *error;
               ulong internal;

               internal = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->keyinternal= internal;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if(strcmp (name,"mst_GOP_Set_YUV") == 0)
     {
         dfb_config->mst_GOP_Set_YUV = true;
     }
     else if(strcmp (name,"stretchdown_patch") == 0)
     {
         dfb_config->do_stretchdown_patch = true;
     }
     else if(strcmp (name,"line_stretchblit_patch") == 0)
     {
         dfb_config->line_stretchblit_patch = true;
     }
     else if(strcmp (name,"stretchdown_enhance") == 0)
     {
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                    dfb_config->do_stretchdown_patch = true;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->do_stretchdown_patch = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else if(strcmp (name,"static_stretchdown_buf") == 0)
     {
        if (value)
        {
               char *error;
               ulong static_buf;

               static_buf = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->static_stretchdown_buf = static_buf;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if (strcmp (name, "mst_png_hwdecode") == 0)
     {
        dfb_config->mst_png_hwdecode = true;
     }
     else if (strcmp (name, "mst_png_disable_hwdecode") == 0)
     {
        dfb_config->mst_png_hwdecode = false;
     }
     else if(strcmp (name, "tvos_mode") == 0)
     {
        dfb_config->tvos_mode = true;
     }
     else if(strcmp (name, "devmem_dump") == 0)
     {
        dfb_config->enable_devmem_dump = true;
     }
     else if (strcmp(name,"dfb_using_forcewriteEnable")==0)
     {
        dfb_config->mst_forcewrite_DFB = DFB_FORECWRITE_USING|DFB_FORCEWRITE_ENABLE;

     }
     else if(strcmp(name,"dfb_using_forcewriteDisable")==0)
     {

       dfb_config->mst_forcewrite_DFB = DFB_FORECWRITE_USING|DFB_FORCEWRITE_DISABLE;
     }
     else if(strcmp(name,"dfb_ignore_forcewrite")==0)
     {

       dfb_config->mst_forcewrite_DFB = dfb_config->mst_forcewrite_DFB &~(DFB_FORECWRITE_USING|DFB_FORCEWRITE_DISABLE|DFB_FORCEWRITE_ENABLE);
     }
     else if(strcmp (name,"yuvtorgb_patch") == 0)
     {
         dfb_config->do_yuvtorgb_sw_patch = true;
     }
     else if(strcmp (name,"enable_sw_jpeg") == 0)
     {
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                    dfb_config->mst_jpeg_hwdecode_option = false;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->mst_jpeg_hwdecode_option = true;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else if(strcmp (name,"enable_dither") == 0)
     {
          dfb_config->mst_dither_enable = true;
          
          if (value)        
          {        
              if (strcmp( value, "enable" ) == 0)        
              {        
                  dfb_config->mst_dither_enable = true;        
              }        
              
              else if(strcmp( value, "disable" ) == 0)        
              {        
                  dfb_config->mst_dither_enable = false;        
              }        
              else        
              {        
                  printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
              }        
          }
     }
     else if(strcmp (name,"mst_ir_max_keycode") == 0)
     {
        if (value)
        {
               char *error;
               ulong ir_max_keycode;

               ir_max_keycode = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_ir_max_keycode = ir_max_keycode;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if(strcmp (name,"gop_using_tlb") == 0)
     {
         dfb_config->bGOPUsingHWTLB = true;
     }
     else if(strcmp (name,"prealloc_map_tlb") == 0)
     {
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                    dfb_config->bPrealloc_map_tlb = true;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->bPrealloc_map_tlb = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else if(strcmp(name,"tlb_alignment")==0)
     {
        if(value)
        {
            unsigned long val;
            char *error;
            val = strtoul( value, &error, 10);
            if(*error)
            {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                return DFB_INVARG;
            }
            dfb_config->TLBAlignmentSize = val;
        }
        else
        { 
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if(strcmp(name,"mboot_gop_index")==0)
     {
        if(value)
        {
            unsigned long val;
            char *error;
            val = strtoul( value, &error, 10);
            if(*error)
            {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                return DFB_INVARG;
            }
            dfb_config->mbootGOPIndex = val;
        }
        else
        { 
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if(strcmp (name,"mst_enable_gevq") == 0)
     {
        if (value)
        {
            if (strcmp( value, "yes" ) == 0)
            {
                dfb_config->mst_enable_gevq = true;
            }
            else if (strcmp( value, "no" ) == 0)
            {
                dfb_config->mst_enable_gevq = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                return DFB_INVARG;
            }
        }
        else
        {
            dfb_config->mst_enable_gevq = true;
        }
     }
     else if (strcmp (name,"mst_disable_layer_init") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_disable_layer_init = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name,"debug_layer") == 0) 
    {
        dfb_config->mst_debug_layer = true;
    }
    else if (strcmp (name,"debug_surface") == 0) 
    {
        dfb_config->mst_debug_surface = true;
    }
    else if (strcmp (name,"debug_input") == 0) 
    {
        if (value && strcmp( value, "tmpfile" ) == 0)
            dfb_config->mst_debug_input = DFB_DFB_LEVEL_TEMP_FILE;
        else
            dfb_config->mst_debug_input = DFB_DBG_LEVEL_NORMAL;

    }
    else if (strcmp (name,"window_double_buffer") == 0) 
    {
        dfb_config->window_double_buffer= true;
    }
    else if (strcmp (name,"debug_ion") == 0) 
    {
        dfb_config->mst_debug_ion = true;
    }
    else if (strcmp (name, "ion_heapmask_by_layer[0]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
    else if (strcmp (name, "ion_heapmask_by_layer[1]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     }
    else if (strcmp (name, "ion_heapmask_by_layer[2]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
    else if (strcmp (name, "ion_heapmask_by_layer[3]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
    else if (strcmp (name, "ion_heapmask_by_layer[4]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
    else if (strcmp (name, "ion_heapmask_by_layer[5]" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
    else if (strcmp (name, "ion_heapmask_by_surface" ) == 0) 
    {
        if (value) {
           if (strcmp( value, "system" ) == 0) {
               dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_SYSTEM_MASK;
           } 
           else if (strcmp( value, "miu0" ) == 0) {
               dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU0_MASK;
           } 
           else if (strcmp( value, "miu1" ) == 0) {
               dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU1_MASK;
           } 
           else if (strcmp( value, "miu2" ) == 0) {
               dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU2_MASK;

           }
           else {
                D_ERROR( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", name, value );
                return DFB_INVARG;
           }
        }
        else {
           D_ERROR( "DirectFB/Config '%s': No heap mask specified!\n", name );
           return DFB_INVARG;
        }
     } 
     else if (strcmp (name, "mst_layer_default_width") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_layer_default_width = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     } 
     else if (strcmp (name, "mst_layer_default_height") == 0) {
          if (value) {
               char *error;
               unsigned long val;
               val = strtoul( value, &error, 10 );

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_layer_default_height = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else if(strcmp (name,"full_update_num") == 0)
     {
        if (value)
        {
               char *error;
               ulong numerator;

               numerator = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->full_update_numerator = numerator;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }
     else if(strcmp (name,"full_update_den") == 0)
     {
        if (value)
        {
               char *error;
               ulong denominator;

               denominator = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->full_update_denominator = denominator;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     } 
     else if (strcmp (name,"surface_memory_type") == 0) 
     {
        if (value)
        {
               char *error;
               u8 type;

               type = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_surface_memory_type = type;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }      
     else if(strcmp (name,"disable_decode_small_jpeg_by_sw") == 0)
     {
         dfb_config->mst_disable_decode_small_jpeg_by_sw = true; 
     }
     else if(strcmp (name,"enable_GOP_Vmirror_Patch") == 0)
     {
         dfb_config->mst_enable_GOP_Vmirror_Patch = true; 
     }
     else if(strcmp (name,"disble_GOP_Vmirror_Patch") == 0)
     {
         dfb_config->mst_enable_GOP_Vmirror_Patch = false; 
     }
     else if (strcmp (name,"stretchdown_patch_ratio") == 0) 
     {
        if (value)
        {
               char *error;
               u8 type;

               type = strtoul( value, &error, 10);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->stretchdown_patch_ratio = type;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
            return DFB_INVARG;
        }
     }      
     else if (strcmp (name, "null_driver") == 0 )
     {
         dfb_config->null_driver = true;
     }
     else if (strcmp (name, "sw_render") == 0 )
     {
          // enable sw_render flag.
          dfb_config->sw_render = SWRF_ENABLE;

          if (value)
          {              
              dfb_config->sw_render = atoi(value);
               printf("[DFB] sw_render = %d\n", dfb_config->sw_render);
          }
     }
     else if(strcmp (name,"enable_jpeg_quality") == 0)
     {
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                    dfb_config->mst_enable_jpeg_quality = true;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->mst_enable_jpeg_quality = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }
     else if(strcmp (name,"mst_enable_dip") == 0)
     {
          printf("[DFB] mst_enable_dip\n");
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                     printf("[DFB] mst_enable_dip enable\n");
                    dfb_config->mst_enable_dip = true;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->mst_enable_dip = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
     }    
     else if (strcmp (name, "mst_dip_mload_addr" ) == 0) {
          if (value) {
               char *error;
               u64 val;

               val = strtoull( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_dip_mload_addr = val;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
    }
    else if (strcmp (name, "mst_dip_mload_length" ) == 0) {
          if (value) {
               char *error;
               ulong length;

               length = strtoul( value, &error, 16);

               if (*error) {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                    return DFB_INVARG;
               }

               dfb_config->mst_dip_mload_length = length;
          }
          else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
    }   
    else if (strcmp (name,"mst_dip_select") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_dip_select= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
    else if (strcmp (name, "mst_disable_master_pri_high" ) == 0) {
    dfb_config->mst_disable_master_pri_high = true;
    } 
     else if(strcmp (name,"show_freeze_image") == 0){
         dfb_config->freeze_image_by_dfb = true;
     }
     else if(strcmp (name,"i8toargb4444_patch") == 0)
     {
         dfb_config->do_i8toargb4444_sw_patch = true;
     } 
     else if(strcmp (name,"mst_enable_ve_init") == 0)
     {
         dfb_config->mst_enable_ve_init = true;
     }
     else if (strcmp (name, "mst_register_gpd") == 0)
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_register_gpd = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if(strcmp (name,"hw_jpg_limit_patch") == 0)
     {
         dfb_config->do_hw_jpg_limit_patch = true;
     } 
     else if (strcmp (name, "dump_backtrace") ==0)
     {
#if !defined(__UCLIBC__)
#ifndef DFB_AARCH64
            registerSigalstack();
#endif
#endif
     }
     else if (strcmp (name, "test_ge") ==0)
     {
           dfb_config->mst_GE_performanceTest = true;
     }
     else if (strcmp (name, "miu_protect") ==0)
     {
          dfb_config->mst_MIU_protect = true;

          if (value) {
               int id;

               if (sscanf( value, "%d", &id ) < 0) {
                    D_ERROR("DirectFB/Config '%s': Could not parse id!\n", name);
                    return DFB_INVARG;
               }

               if (id < 0 || id > 3) {
                    D_ERROR("DirectFB/Config '%s': ID %d out of bounds!\n", name, id);
                    return DFB_INVARG;
               }

               dfb_config->mst_MIU_protect_BlockID = id;

          }

     }  
     else if (strcmp (name,"mst_cma_heap_id") == 0)
     {
           if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_cma_heap_id = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name,"mst_sec_cma_heap_id") == 0)
     {
           if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_sec_cma_heap_id = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mapmem_mode") == 0)
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_mapmem_mode = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_disable_dfbinfo") ==0)
     {
           dfb_config->mst_disable_dfbinfo = true;
     }
     else if (strcmp (name, "src_color_key_index_patch") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->src_color_key_index_patch= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_ir_repeat_time") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_ir_repeat_time= val;
         printf("DFB Note: mst_ir_repeat_time be set to %d ms\n", dfb_config->mst_ir_repeat_time);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_new_ir_repeat_time") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_new_ir_repeat_time= val;
         printf("DFB Note: mst_new_ir_repeat_time be set to %d ms\n", dfb_config->mst_new_ir_repeat_time);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_new_ir_first_repeat_time") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_new_ir_first_repeat_time= val;
         printf("DFB Note: mst_new_ir_first_repeat_time be set to %d ms\n", dfb_config->mst_new_ir_first_repeat_time);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_keypad_repeat_time") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_keypad_repeat_time= val;
         printf("DFB Note: mst_keypad_repeat_time be set to %d ms\n", dfb_config->mst_keypad_repeat_time);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_usbir_repeat_time") == 0 )
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_usbir_repeat_time= val;
            printf("DFB Note: mst_usbir_repeat_time be set to %d ms\n", dfb_config->mst_usbir_repeat_time);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_ir_first_time_out") == 0 )
     {
         // if not repeat, the time out is repeat_time + first_time_out.
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_ir_first_time_out = val;
            printf("DFB Note: mst_ir_first_time_out be set to %d ms\n", dfb_config->mst_ir_first_time_out);
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_handle_wm_key") ==0)
     {
           dfb_config->mst_handle_wm_key = true;
     }
     else if(strcmp (name, "disable_cjk_string_break") ==0)
     {
        dfb_config->disable_cjk_string_break = true;
     }
     else if(strcmp (name, "disable_quick_press") ==0)
     {
        dfb_config->disable_quick_press= true;
     }
     else if(strcmp (name, "mst_disable_window_scale_patch") ==0)
     {
        dfb_config->mst_disable_window_scale_patch= true;
     }
     else if(strcmp (name, "mst_force_flip_wait_tagid") ==0)
     {
        dfb_config->mst_force_flip_wait_tagid= true;
     }
     else if(strcmp (name, "enable_cursor_mouse_optimize") ==0)
     {
         dfb_config->enable_cursor_mouse_optimize = true;
     }
     else if(strcmp (name, "layer_support_palette") ==0)
     {
         dfb_config->layer_support_palette= true;
     }
     else if(strcmp (name, "stretchblit_with_rotate") ==0)
     {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->stretchblit_with_rotate= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if(strcmp (name, "mst_enable_gop_gwin_partial_update") ==0)
     {
         dfb_config->mst_enable_gop_gwin_partial_update = false;
     }
     else if (strcmp (name,"enable_new_alphamode") == 0 || strcmp (name,"mst_enable_new_alphamode") == 0)
     {
         if (value) {
             char *error;
             unsigned int val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_new_alphamode = val;
         }
     }
     else if (strcmp (name,"new_alphamode_on_layerid") == 0 || strcmp (name,"mst_new_alphamode_on_layerid") == 0)
     {
         if (value) {
             char *error;
             unsigned int val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_newalphamode_on_layer = (u8)val;
         }
     }
     else if(strcmp (name, "mst_layer_gwin_level") ==0)
     {
         if (value) {
            char *error;
            int val;
 
            val = strtoul( value, &error, 10 );
 
            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_layer_gwin_level= val;
         }
         else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
         }
     }
     else if (strcmp (name,"enable_layer_fullupdate[0]") == 0) {
         dfb_config->mst_layer_bfullupdate[0] = true;
    }
     else if (strcmp (name,"enable_layer_fullupdate[1]") == 0) {
         dfb_config->mst_layer_bfullupdate[1] = true;
    }
     else if (strcmp (name,"enable_layer_fullupdate[2]") == 0) {
         dfb_config->mst_layer_bfullupdate[2] = true;
    }
     else if (strcmp (name,"enable_layer_fullupdate[3]") == 0) {
         dfb_config->mst_layer_bfullupdate[3] = true;
    }
     else if (strcmp (name,"enable_layer_fullupdate[4]") == 0) {
         dfb_config->mst_layer_bfullupdate[4] = true;
    }
     else if (strcmp (name,"enable_layer_fullupdate[5]") == 0) {
         dfb_config->mst_layer_bfullupdate[5] = true;
    }
     else if (strcmp (name, "mst_enhance_stretchblit_precision") == 0){
        dfb_config->mst_enhance_stretchblit_precision = true;
    }
     else if (strcmp (name,"mst_gop_miu2_setting_extend") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_gop_miu2_setting_extend = val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "mst_enable_GLES2") == 0){
        dfb_config->mst_enable_GLES2 = true;
     }
     else if (strcmp (name, "mst_argb1555_display") == 0){
        dfb_config->mst_argb1555_display=true;
     }
     else if (strcmp (name, "mst_font_dsblit_src_premultiply") == 0){
        dfb_config->mst_font_dsblit_src_premultiply = true;
     }
     else if(strcmp (name, "mst_mem_peak_usage") ==0)
     {
         if (value) {
            char *error;
            int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_mem_peak_usage= val;
         }
         else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
         }
     }
     else if (strcmp (name,"mst_enable_gwin_multialpha") == 0)
     {
         dfb_config->mst_enable_gwin_multialpha = true;
     }
     else if (strcmp(name, "mst_AFBC_layer_enable") == 0)
     {
         dfb_config->mst_AFBC_layer_enable = 0x3f; // Enable AFBC from layer 0 ~ layer 5

         if (value) {
                char *error;
                ulong val;

                val = strtoul( value, &error, 16);

                if (*error){
                     printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                     return DFB_INVARG;
                }

                dfb_config->mst_AFBC_layer_enable = val;
         }
         D_INFO("mst_AFBC_layer_enable = 0x%08x \n", dfb_config->mst_AFBC_layer_enable);

     }
     else if (strcmp(name, "mst_AFBC_mode") == 0)
     {
         if (value) {
             char *error;
             ulong val;

             val = strtoul( value, &error, 16);

             if (*error){
                  printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                  return DFB_INVARG;
             }

             dfb_config->mst_AFBC_mode = val;
         }
         D_INFO("mst_AFBC_mode = 0x%08x \n", dfb_config->mst_AFBC_mode);
     }
     else if (strcmp (name,"mst_measure_png_performance") == 0)
     {
         dfb_config->mst_measure_png_performance = true;
     }
     else if (strcmp (name,"mst_rgb2yuv_mode") == 0)
     {
         if (value) {
             char *error;
             ulong val;

             val = strtoul( value, &error, 16);

             if (*error){
                  printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                  return DFB_INVARG;
             }

             dfb_config->mst_rgb2yuv_mode = val;
         }
         D_INFO("mst_AFBC_mode = 0x%08x \n", dfb_config->mst_rgb2yuv_mode);
     }
    else if (strcmp (name,"debug_cma") == 0)
    {
        dfb_config->mst_debug_cma = true;
    }
    else if (strcmp (name,"margine_left") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_margine_left= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name,"margine_wright") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_margine_wright= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name,"margine_top") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_margine_top= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name, "dfbinfo-dir") == 0) {
          if (value) {
               if (dfb_config->dfbinfo_dir)
                    D_FREE( dfb_config->dfbinfo_dir );
               dfb_config->dfbinfo_dir = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'module-dir': No directory name specified!\n");
               return DR_INVARG;
          }
        }
     else if (strcmp (name,"margine_bottom") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 16 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DFB_INVARG;
            }
            dfb_config->mst_margine_bottom= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
         }
     }
     else if (strcmp (name,"CTV_patch") == 0)
     {
        dfb_config->mst_modify_symbol_by_keymap= true;
     }
     else if (strcmp (name,"disable_modify_symbol_by_keymap") == 0)
     {
        dfb_config->mst_modify_symbol_by_keymap= false;
     }
    else if (strcmp (name,"mst_memory_use_cma") == 0)
    {
        dfb_config->mst_memory_use_cma = true;
    }
    else if (strcmp (name,"mst_gfxdriver") == 0)
    {
        if (value) {
            if (dfb_config->mst_gfxdriver)
            {
                  D_FREE( dfb_config->mst_gfxdriver );
                  dfb_config->mst_gfxdriver = NULL;
            }

            if (strcmp(value, "auto") != 0)
                dfb_config->mst_gfxdriver = D_STRDUP( value );
        }
    }
    else if (strcmp (name,"mst_font_use_video_mem") == 0)
    {
        dfb_config->mst_font_use_video_mem = true;
    }
    else if (strcmp (name,"disable_bus_address_check") == 0)
    {
        dfb_config->bus_address_check = false;
    }
    else if(strcmp (name, "mst_layer_flip_blit") ==0)
    {
        dfb_config->mst_layer_flip_blit = true;
    }
    else if (strcmp (name,"window_single_buffer") == 0)
    {
        dfb_config->window_single_buffer= true;
    }
    else if (strcmp (name,"mst_new_ir") == 0)
    {
        dfb_config->mst_new_ir = true;
    }
    else if (strcmp (name,"mst_t_stretch_mode") == 0)
    {
        if(value)
        {
            // linear mode
            if (strcmp( value, "DUPLICATE" ) == 0)
                dfb_config->mst_t_stretch_mode = DISPLAYER_TRANSPCOLOR_STRCH_DUPLICATE;

            //  duplicate mode
            else if(strcmp( value, "ASNORMAL" ) == 0)
                dfb_config->mst_t_stretch_mode = DISPLAYER_TRANSPCOLOR_STRCH_ASNORMAL;

            else
            {
                if (value)
                {
                    char *error;
                    unsigned long val;
                    val = strtoul( value, &error, 16 );

                    if (*error)
                    {
                        printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                    }

                    dfb_config->mst_t_stretch_mode = val;
                }
            }
        }
        else
            printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

    }
    else if (strcmp (name,"mst_h_stretch_mode") == 0)
    {
        if(value)
        {
            // 6-tape (including nearest) mode
            if (strcmp( value, "6TAPE" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE;

            // duplicate mode.
            else if(strcmp( value, "DUPLICATE" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_DUPLICATE;

            // 6-tape (Linear mode)
            else if(strcmp( value, "6TAPE_LINEAR" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_LINEAR;

            // 6-tape (Nearest mode)
            else if(strcmp( value, "6TAPE_NEAREST" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_NEAREST;

            // 6-tape (Gain0)
            else if(strcmp( value, "6TAPE_GAIN0" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN0;

            // 6-tape (Gain1)
            else if(strcmp( value, "6TAPE_GAIN1" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN1;

            // 6-tape (Gain2)
            else if(strcmp( value, "6TAPE_GAIN2" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN2;

            // 6-tape (Gain3)
            else if(strcmp( value, "6TAPE_GAIN3" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN3;

            // 6-tape (Gain4)
            else if(strcmp( value, "6TAPE_GAIN4" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN4;

            // 6-tape (Gain5)
            else if(strcmp( value, "6TAPE_GAIN5" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN5;

            // 4-tap filer
            else if(strcmp( value, "4TAPE" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_4TAPE;

            // 2-tape
            else if(strcmp( value, "2TAPE" ) == 0)
                dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_2TAPE;

            else
            {
                if (value)
                {
                    char *error;
                    unsigned long val;
                    val = strtoul( value, &error, 16 );

                    if (*error)
                    {
                        printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                    }

                    dfb_config->mst_h_stretch_mode = val;
                }
            }
        }
        else
            printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

    }
    else if (strcmp (name,"mst_v_stretch_mode") == 0)
    {
        if(value)
        {
            // linear mode
            if (strcmp( value, "LINEAR" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR;

            //  duplicate mode
            else if(strcmp( value, "DUPLICATE" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_DUPLICATE;

            //  nearest mode
            else if(strcmp( value, "NEAREST" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_NEAREST;

            //  Linear GAIN0 mode
            else if(strcmp( value, "LINEAR_GAIN0" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN0;

            //  Linear GAIN1 mode
            else if(strcmp( value, "LINEAR_GAIN1" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN1;

            //  Linear GAIN2 mode
            else if(strcmp( value, "LINEAR_GAIN2" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN2;

            //  4-TAPE mode
            else if(strcmp( value, "4TAPE" ) == 0)
                dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_4TAPE;
            else
            {
                if (value)
                {
                    char *error;
                    unsigned long val;
                    val = strtoul( value, &error, 16 );

                    if (*error)
                    {
                        printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                    }

                    dfb_config->mst_v_stretch_mode = val;
                }
            }
        }
        else
            printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

    }
    else if (strcmp (name,"mst_blink_frame_rate") == 0)
    {
        if (value) {
        char *error;
        int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_blink_frame_rate = val;
        }
        else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
        }
    }
    else if(strcmp (name,"mst_cursor_swap_mode") == 0)
    {
        dfb_config->mst_cursor_swap_mode = true;

        D_INFO("[DFB] mst_cursor_swap_mode enabled!\n");
    }
    else if(strcmp (name,"mst_inputevent_layer") == 0)
    {
         if (value) {
             char *error;
             int val;

             val = strtoul( value, &error, 10 );

             if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                 return DFB_INVARG;
             }

             dfb_config->mst_inputevent_layer = val;
             D_INFO("[DFB] %s : input event listen on layer %d\n", __FUNCTION__, dfb_config->mst_inputevent_layer);
         }
         else {
             D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
             return DFB_INVARG;
         }
    }
    else if(strcmp (name,"mst_flip_dump_path") == 0)
    {
        if (dfb_config->mst_flip_dump_path)
            D_FREE( dfb_config->mst_flip_dump_path );

        if (value)
        {
            dfb_config->mst_flip_dump_path = D_STRDUP( value );
            D_INFO("[DFB] mst_flip_dump_path =%s\n", dfb_config->mst_flip_dump_path);
        }
        else
        {
            dfb_config->mst_flip_dump_path = D_STRDUP( "measure" );
            D_INFO("[DFB] mst_flip_dump_path measure performance\n");
        }
    }
    else if(strcmp (name,"mst_flip_dump_type") == 0)
    {
        if (dfb_config->mst_flip_dump_type)
            D_FREE( dfb_config->mst_flip_dump_type );

        if (value)
        {
            dfb_config->mst_flip_dump_type = D_STRDUP( value );
            D_INFO("[DFB] mst_flip_dump_type = %s\n", dfb_config->mst_flip_dump_type);
        }
        else
        {
            dfb_config->mst_flip_dump_type = D_STRDUP( "all" );
            D_INFO("[DFB] mst_flip_dump_type = %s\n", dfb_config->mst_flip_dump_type);
        }
    }
    else if(strcmp (name,"mst_flip_dump_area") == 0)
    {
        if (value)
        {
             int arg[ARG_SIZE] = {0};
             char * token = strtok(value, ",");
             int i = 0;
             // loop through the string to extract all other tokens
             while(token != NULL)
             {
                 arg[i] = atoi(token);
                 token = strtok(NULL, ",");
                 i++;
             }

             if(arg[DUMP_X] > 0)
                dfb_config->mst_flip_dump_area.x = arg[DUMP_X];
             if(arg[DUMP_Y] > 0)
                dfb_config->mst_flip_dump_area.y = arg[DUMP_Y];
             if(arg[DUMP_W] > 0)
                dfb_config->mst_flip_dump_area.w = arg[DUMP_W];
             if(arg[DUMP_H] > 0)
                dfb_config->mst_flip_dump_area.h = arg[DUMP_H];

             D_INFO("[DFB] mst_flip_dump_area = (%d, %d, %d, %d)\n",
                                   dfb_config->mst_flip_dump_area.x,
                                   dfb_config->mst_flip_dump_area.y,
                                   dfb_config->mst_flip_dump_area.w,
                                   dfb_config->mst_flip_dump_area.h);
        }
        else
        {
            dfb_config->mst_flip_dump_area.x = 0;
            dfb_config->mst_flip_dump_area.y = 0;
            dfb_config->mst_flip_dump_area.w = 0;
            dfb_config->mst_flip_dump_area.h = 0;

            D_INFO("[DFB] mst_flip_dump_area = (%d, %d, %d, %d)\n",
                                  dfb_config->mst_flip_dump_area.x,
                                  dfb_config->mst_flip_dump_area.y,
                                  dfb_config->mst_flip_dump_area.w,
                                  dfb_config->mst_flip_dump_area.h);
        }
    }
    else if(strcmp(name, "mst_debug_cursor") == 0)
    {
        dfb_config->mst_debug_cursor = true;
    }
    else if ( strcmp (name, "mst_cursor_gwin_width") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_cursor_gwin_width = val;
        }
    }
    else if ( strcmp (name, "mst_cursor_gwin_height") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_cursor_gwin_height = val;
        }
    }
    else if (strcmp (name, "hw_cursor") == 0) {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }
            if(val >= 0)
                dfb_config->mst_hw_cursor = val;
        }
    }
    else if (strcmp (name,"mst_fix_string_break") == 0) {
          dfb_config->mst_fix_string_break = true;
    }
    else if (strcmp (name,"mst_gles2_sdr2hdr") == 0) {
        dfb_config->mst_enable_GLES2 = true;
        dfb_config->mst_gles2_sdr2hdr = true;
    }
    else if (strcmp (name,"mst_ge_hw_limit") == 0) {
        if (value) {
        char *error;
        int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_ge_hw_limit = val;
        }
        else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
        }
    }
    else if (strcmp (name,"mst_cmdq_phys_addr") == 0) {
        if (value) {
        char *error;
        unsigned long val;

            val = strtoul( value, &error, 16 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_cmdq_phys_addr = val;
        }
        else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
        }
    }
    else if (strcmp (name,"mst_cmdq_phys_len") == 0) {
        if (value) {
        char *error;
        unsigned long val;

            val = strtoul( value, &error, 16 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_cmdq_phys_len = val;
        }
        else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
        }
    }
    else if (strcmp (name,"mst_cmdq_miusel") == 0) {
        if (value) {
        char *error;
        unsigned long val;

            val = strtoul( value, &error, 16 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_cmdq_miusel = val;
        }
        else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
        }
    }
    else if (strcmp (name,"mst_gop_interlace_adjust") == 0) {
        dfb_config->mst_gop_interlace_adjust = true;
    }
    else if (strcmp (name, "mst_CFD_monitor_path" ) == 0) {
          if (value) {
               if (dfb_config->mst_CFD_monitor_path)
                    D_FREE( dfb_config->mst_CFD_monitor_path );
               dfb_config->mst_CFD_monitor_path = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'mst_CFD_monitor_path': No directory name specified!\n");
               return DR_INVARG;
          }
     }
    else if (strcmp (name,"mst_layer_buffer_mode") == 0)
    {
        if (value)
        {
           if (strcmp( value, "triple" ) == 0)
                dfb_config->mst_layer_buffer_mode = DLBM_TRIPLE;

           else if  (strcmp( value, "backvideo" ) == 0)
                dfb_config->mst_layer_buffer_mode = DLBM_BACKVIDEO;

           else if (strcmp( value, "backsystem" ) == 0)
                dfb_config->mst_layer_buffer_mode = DLBM_BACKSYSTEM;

           else if (strcmp( value, "frontonly" ) == 0)
                dfb_config->mst_layer_buffer_mode = DLBM_FRONTONLY;

           else if (strcmp( value, "windows" ) == 0)
                dfb_config->mst_layer_buffer_mode = DLBM_WINDOWS;

           else
                printf( "DirectFB/Config '%s': Unknown mode '%s'!\n", __FUNCTION__, value );
        }
        else
        {
           printf( "DirectFB/Config '%s': No buffer mode specified!\n", __FUNCTION__ );
        }
    }
    else if (strcmp (name,"mst_layer_pixelformat") == 0)
    {
        if (value)
        {
            dfb_config->mst_layer_pixelformat = dfb_config_parse_pixelformat( value );

            if ( dfb_config->mst_layer_pixelformat == DSPF_UNKNOWN)
                printf("DirectFB/Config '%s': Could not parse format!\n", __FUNCTION__);
        }
        else
        {
            printf("DirectFB/Config '%s': No format specified!\n", __FUNCTION__);
        }
    }
    else if(strcmp (name,"mst_do_xc_ip1_patch") == 0)
    {
          if (value)
          {
               if (strcmp( value, "enable" ) == 0)
               {
                    printf("[DFB] mst_do_xc_ip1_patch enable\n");
                    dfb_config->mst_do_xc_ip1_patch = true;
               }
               else if (strcmp( value, "disable" ) == 0)
               {
                    dfb_config->mst_do_xc_ip1_patch = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, value );
                    return DFB_INVARG;
               }
          }
          else
          {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DFB_INVARG;
          }
    }
    else if (strcmp (name, "mst_dump_jpeg_buffer" ) == 0) {
          if (value) {
               if (dfb_config->mst_dump_jpeg_buffer)
                    D_FREE( dfb_config->mst_dump_jpeg_buffer );
               dfb_config->mst_dump_jpeg_buffer = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'mst_dump_jpeg_buffer': No directory name specified!\n");
               return DR_INVARG;
          }
     }
    else if (strcmp (name, "mst_clip_stretchblit_width_high_nonzero_patch") == 0) {
          if(value){
               if (strcmp( value, "disable" ) == 0)
               {
                    printf("[DFB] mst_clip_stretchblit_width_high_nonzero_patch disable\n");
                    dfb_config->mst_clip_stretchblit_width_high_nonzero_patch = false;
               }
               else if (strcmp( value, "enable" ) == 0)
               {
                    printf("[DFB] mst_clip_stretchblit_width_high_nonzero_patch enable\n");
                    dfb_config->mst_clip_stretchblit_width_high_nonzero_patch = true;
               }
               else{
                    D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                    return DR_INVARG;
               }
          }
    }
    else if (strcmp (name, "mst_bank_force_write" ) == 0)
    {
          if(value)
          {
               if (strcmp( value, "disable" ) == 0)
               {
                    D_INFO( "[DFB] force to not support bankwrite\n");
                    dfb_config->mst_bank_force_write = false;
               }
               else if (strcmp( value, "enable" ) == 0)
               {
                    D_INFO( "[DFB] force to support bankwrite\n");
                    dfb_config->mst_bank_force_write = true;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                    return DR_INVARG;
               }
          }
    }
    else if (strcmp (name, "mst_font_bold_enable" ) == 0)
    {
          dfb_config->mst_font_bold_enable= true;
    }
    else if(strcmp(name, "mst_fixed_mem_leak_patch_enable") == 0)
    {
          if(value)
          {
               if (strcmp( value, "true" ) == 0)
               {
                    dfb_config->mst_fixed_mem_leak_patch_enable = true;
               }
               else if (strcmp( value, "false" ) == 0)
               {
                    D_INFO( "[DFB] mst_fixed_mem_leak_patch_enable = false\n");
                    dfb_config->mst_fixed_mem_leak_patch_enable = false;
               }
               else
               {
                    D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                    return DR_INVARG;
               }
          }
    }
    else if (strcmp (name, "mst_nice" ) == 0)
    {
        int val = -20;

        if (value)
        {
            val = atoi(value);
        }

        nice(val);
    }
    else if (strcmp (name, "mst_show_fps" ) == 0)
    {
          dfb_config->mst_show_fps= true;
    }
    else if (strcmp (name, "mst_enable_layer_autoscaledown" ) == 0)
    {
          dfb_config->mst_enable_layer_autoscaledown= true;
    }
     else if(strcmp (name, "mst_mem_small_size") ==0)
     {
         if (value) {
            char *error;
            int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_mem_small_size= val;
         }
         else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
         }
     }
     else if(strcmp (name, "mst_mem_medium_size") ==0)
     {
         if (value) {
            char *error;
            int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_mem_medium_size= val;
         }
         else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
         }
     }
    else if(strcmp (name, "mst_forbid_fragment_merge_to_api_locked_surface") == 0)
    {
        dfb_config->mst_forbid_fragment_merge_to_api_locked_surface = true;
    }
    else if(strcmp(name, "mst_null_display_driver") == 0 || strcmp(name, "null_display_driver") == 0)
    {
        dfb_config->mst_null_display_driver = true;
    }
    else if(strcmp (name, "mst_call_setkeypadcfg_in_device") == 0)
    {
        dfb_config->mst_call_setkeypadcfg_in_device = true;
    }
    else if(strcmp (name, "mst_call_setdfbrccfg_disable") == 0)
    {
        dfb_config->mst_call_setdfbrccfg_disable = true;
    }
    else if(strcmp(name, "mst_gwin_disable") == 0)
    {
        dfb_config->mst_gwin_disable = true;
    }
    else if(strcmp(name, "mst_using_mi_system") == 0)
    {
        dfb_config->mst_using_mi_system = true;
    }
    else if(strcmp(name, "mst_force_wait_vsync") == 0)
    {
        dfb_config->mst_force_wait_vsync = true;
    }
    else if(strcmp(name, "mst_new_mstar_linux_input") == 0)
    {
        if(value)
        {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_new_mstar_linux_input = true;
            }
            else if (strcmp( value, "false" ) == 0)
            {
                D_INFO( "[DFB] mst_new_mstar_linux_input = false\n");
                dfb_config->mst_new_mstar_linux_input = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if(strcmp(name, "mst_mma_pool_enable") == 0)
    {
        dfb_config->mst_mma_pool_enable = true;
    }
    else if(strcmp(name, "mst_call_gop_t3d") == 0)
     {
         if (value) {
            char *error;
            int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            dfb_config->mst_call_gop_t3d= val;
         }
         else {
            D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
            return DFB_INVARG;
         }
     }
    else if(strcmp(name, "mst_gopc_check_video_info") == 0)
    {
        if(value)
        {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_gopc_check_video_info = true;
            }
            else if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_gopc_check_video_info = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if(strcmp(name, "mst_preinit_fusion_world") == 0)
    {
        dfb_config->mst_preinit_fusion_world = true;
    }
    else if(strcmp(name, "mst_font_outline_enable") == 0)
    {
        dfb_config->mst_font_outline_enable = true;
    }
    else if(strcmp(name, "mst_bt601_formula") == 0)
    {
        if (value) {
            char *error;
            int val;

            val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DR_INVARG;
            }

            dfb_config->mst_bt601_formula = val;
        }
        else {
           D_ERROR( "[DFB] DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
           return DR_INVARG;
        }
    }
    else if(strcmp(name, "mst_PTK_customized_input_driver_enable") == 0)
    {
        dfb_config->mst_PTK_customized_input_driver_enable = true;
    } 
	else if (strcmp (name, "mst_layer_fbdev[0]" ) == 0) {
	if (value) {
            if (dfb_config->mst_layer_fbdev[0])
                D_FREE( dfb_config->mst_layer_fbdev[0] );
            dfb_config->mst_layer_fbdev[0] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[0]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_layer_fbdev[1]" ) == 0) {
        if (value) {
            if (dfb_config->mst_layer_fbdev[1])
                D_FREE( dfb_config->mst_layer_fbdev[1] );
            dfb_config->mst_layer_fbdev[1] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[1]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_layer_fbdev[2]" ) == 0) {
        if (value) {
            if (dfb_config->mst_layer_fbdev[2])
                D_FREE( dfb_config->mst_layer_fbdev[2] );
            dfb_config->mst_layer_fbdev[2] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[2]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_layer_fbdev[3]" ) == 0) {
        if (value) {
            if (dfb_config->mst_layer_fbdev[3])
                D_FREE( dfb_config->mst_layer_fbdev[3] );
            dfb_config->mst_layer_fbdev[3] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[3]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_layer_fbdev[4]" ) == 0) {
        if (value) {
            if (dfb_config->mst_layer_fbdev[4])
                D_FREE( dfb_config->mst_layer_fbdev[4] );
            dfb_config->mst_layer_fbdev[4] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[4]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_layer_fbdev[5]" ) == 0) {
        if (value) {
            if (dfb_config->mst_layer_fbdev[5])
                D_FREE( dfb_config->mst_layer_fbdev[5] );
            dfb_config->mst_layer_fbdev[5] = D_STRDUP( value );
        }
        else {
            D_ERROR("DirectFB/Config 'mst_layer_fbdev[5]': No device name specified!\n");
            return DFB_INVARG;
        }
    }
    else if (strcmp (name, "mst_debug_mma" ) == 0)
    {
        dfb_config->mst_debug_mma = true;
    }
    else if ( strcmp (name, "mst_oom_retry") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_oom_retry = val;
        }
    }
    else if ( strcmp (name, "mst_use_system_memory_threshold") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_use_system_memory_threshold = val;
        }
    }
    else if (strcmp (name, "mst_GOP_AutoDetectBuf" ) == 0)
    {
        if(value)
        {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_GOP_AutoDetectBuf= true;
            }
            else if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_GOP_AutoDetectBuf= false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
        else
            dfb_config->mst_GOP_AutoDetectBuf= true;
    }
    else if ( strcmp (name, "mst_call_disable_bootlogo") == 0 )
    {
        if (value) {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_call_disable_bootlogo = true;
            }
            else if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_call_disable_bootlogo = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if ( strcmp (name, "mst_GPU_window_compose") == 0 )
    {
        if (value) {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_GPU_window_compose = true;
                dfb_config->mst_enable_GLES2 = true;
                direct_mutex_init( &dfb_config->GPU_compose_mutex );
                direct_waitqueue_init( &dfb_config->GPU_compose_cond );
            }
            else if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_GPU_window_compose = false;
                dfb_config->mst_enable_GLES2 = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if ( strcmp (name, "mst_debug_secure_mode") == 0 )
    {
        dfb_config->mst_debug_secure_mode = true;
    }
    else if (strcmp (name, "mst_debug_gles2" ) == 0)
    {
        dfb_config->mst_debug_gles2 = true;
    }
    else if (strcmp (name, "mst_layer_up_scaling" ) == 0)
    {
        dfb_config->mst_enable_GLES2 = true;

        dfb_config->mst_layer_up_scaling = true;
        dfb_config->mst_layer_up_scaling_width = 1920;
        dfb_config->mst_layer_up_scaling_height = 1080;

        if (value)
        {
            if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_enable_GLES2 = false;

                dfb_config->mst_layer_up_scaling = false;
                dfb_config->mst_layer_up_scaling_width = 0;
                dfb_config->mst_layer_up_scaling_height = 0;
            }
        }
    }
    else if ( strcmp (name, "mst_layer_up_scaling_id") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val >= 0)
                dfb_config->mst_layer_up_scaling_id = val;
        }
    }
    else if ( strcmp (name, "mst_layer_up_scaling_width") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_layer_up_scaling_width = val;
        }
    }
    else if ( strcmp (name, "mst_layer_up_scaling_height") == 0 )
    {
        if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, 10 );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_layer_up_scaling_height = val;
        }
    }
    else if (strcmp (name, "mst_debug_directory_access" ) == 0) {
          if (value) {
               if (dfb_config->mst_debug_directory_access)
                    D_FREE( dfb_config->mst_debug_directory_access );
               dfb_config->mst_debug_directory_access = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'mst_debug_directory_access': No directory name specified!\n");
               return DR_INVARG;
          }
    }
    else if (strcmp (name, "mst_debug_backtrace_dump" ) == 0)
    {
        dfb_config->mst_debug_backtrace_dump = true;
    }
    else if (strcmp (name, "mst_debug_layer_setConfiguration_return" ) == 0)
    {
        dfb_config->mst_debug_layer_setConfiguration_return = true;
    }
    else if (strcmp (name, "mst_debug_surface_clear_return" ) == 0)
    {
        dfb_config->mst_debug_surface_clear_return = true;
    }
    else if (strcmp (name, "mst_debug_surface_fillrectangle_return" ) == 0)
    {
        dfb_config->mst_debug_surface_fillrectangle_return = true;
    }
    else if ( strcmp (name, "mst_GPU_AFBC") == 0 )
    {
        if (value) {
            if (strcmp( value, "true" ) == 0)
            {
                dfb_config->mst_GPU_window_compose = true;
                dfb_config->mst_enable_GLES2 = true;
                dfb_config->mst_GPU_AFBC = true;
                direct_mutex_init( &dfb_config->GPU_compose_mutex );
                direct_waitqueue_init( &dfb_config->GPU_compose_cond );
            }
            else if (strcmp( value, "false" ) == 0)
            {
                dfb_config->mst_GPU_window_compose = false;
                dfb_config->mst_enable_GLES2 = false;
                dfb_config->mst_GPU_AFBC = false;
            }
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if ( strcmp (name, "GPU_AFBC_EXT_SIZE") == 0 )
    {
        if (value) {
            char *error = NULL;
#define BASE 10
          int val = strtoul( value, &error, BASE );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->GPU_AFBC_EXT_SIZE = val;
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }            
    }
    else if ( strcmp (name, "mst_input_vendor_id") == 0 )
    {
        if (value) {
            char *error = NULL;
#define BASE 10
          int val = strtoul( value, &error, BASE );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_input_vendor_id = val;
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }            
    }
    else if ( strcmp (name, "mst_input_product_id") == 0 )
    {
        if (value) {
            char *error = NULL;
#define BASE 10
          int val = strtoul( value, &error, BASE );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_input_product_id = val;
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if ( strcmp (name, "mst_gpio_key_code") == 0 )
    {
        if (value) {
            char *error = NULL;
#define BASE 10
          int val = strtoul( value, &error, BASE );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }

            if(val > 0)
                dfb_config->mst_gpio_key_code = val;
            else
            {
                D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
                return DR_INVARG;
            }
        }
    }
    else if (fusion_config_set( name, value ) && direct_config_set( name, value ))
    {
        if (strstr( name, "mst_InputDevice"))
        {
            int i = 0;

            inputdevice_name_filter_by_bracket( &name );

            for (i =0; i < MAX_LINUX_INPUT_DEVICES; i++)
            {
                if ( (dfb_config->mst_inputdevice[i].name == NULL) && (dfb_config->mst_inputdevice[i].filename == NULL))
                {
                    dfb_config->mst_inputdevice[i].name = D_STRDUP( name);
                    dfb_config->mst_inputdevice[i].filename = D_STRDUP(value);

                    break;
                }
                else
                {
                    if (strcmp(dfb_config->mst_inputdevice[i].name, name) ==0)
                        break;
                }
             }
        }
        else
            return DFB_UNSUPPORTED;
    }
    else
    if (
#if DIRECTFB_BUILD_VOODOO
         voodoo_config_set( name, value ) &&
#endif
         fusion_config_set( name, value ) && direct_config_set( name, value ))
          return DFB_UNSUPPORTED;

     return DFB_OK;
}
#endif

DFBResult dfb_dump_surface(void *pt_surface, int from, char *prefix, int num)
{
     char buf[60];
     CoreSurface* pt_this = (CoreSurface*)pt_surface;
     
     if(!pt_this || !prefix)
     {
        return DFB_FAILURE;
     }
     
     if(!strstr(directory, "/mnt/usb/"))
     {
          strcpy(directory, "/mnt/usb/sda1");
     }    
     
     snprintf( buf, sizeof(buf), "%d_%s[%p][%p_%p][%dx%d]", num, prefix, 
        pt_this,pt_this->type,pt_this->config.caps,pt_this->config.size.w,pt_this->config.size.h);
     
     dfb_surface_dump_buffer( pt_this, (CoreSurfaceBufferRole)from, directory, buf );
 
     return DFB_OK;
}

#ifdef CC_DFB_DEBUG_SUPPORT

#include <unistd.h>

#include <directfb_strings.h>

#include <direct/clock.h>
#include <direct/debug.h>

#include <fusion/build.h>
#include <fusion/fusion.h>
#include <fusion/object.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/shm.h>
#include <fusion/shm/shm_internal.h>

#include <core/core.h>
#include <core/layer_control.h>
#include <core/layer_context.h>
#include <core/layers.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/windows.h>
#include <core/windowstack.h>
#include <core/windows_internal.h>
#include <core/wm.h>


static DirectFBPixelFormatNames( format_names );

/**********************************************************************************************************************/

typedef struct {
     int video;
     int system;
     int presys;
} MemoryUsage;

/**********************************************************************************************************************/

static MemoryUsage mem;

static bool show_shm = false;
static bool show_pools = false;
static bool show_allocs = false;
static bool dump_surcs = false;
static int  dump_layer = 0;       /* ref or -1 (all) or 0 (none) */
static int  dump_surface = 0;     /* ref or -1 (all) or 0 (none) */
static unsigned int dump_flags = 0;


/**********************************************************************************************************************/

static DFBBoolean parse_command_line( int argc, char *argv[] );

/**********************************************************************************************************************/

static int 
str_to_int(const char* string)
{
    int i;
    int value = 0;
    int length = strlen(string);

    if ((string == NULL) || (length == 0))
    {
        return 0;
    }

    if((length > 2) && (string[0] == '0') && (string[1] == 'x'))
    {
        length = (length > 10) ? 10 : length;

        for (i = 2; i < length; i++)
        {
             if ((string[i] >= '0') && (string[i] <= '9'))
             {
                 value = value << 4;
                 value += (int)(char)(string[i] - '0');
             }
             else if ((string[i] >= 'A') && (string[i] <= 'F'))
             {
                 value = value << 4;
                 value += (int)(char)(string[i] - 'A' ) + 10;
             }
             else if ((string[i] >= 'a') && (string[i] <= 'f'))
             {
                 value = value << 4;
                 value += (int)(char)(string[i] - 'a') + 10;
             }
        }

        return value;
    }

    return atoi(string);
}


static inline int
buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, bool video )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

     fusion_vector_foreach (allocation, i, buffer->allocs) {
          int size = allocation->size;
          if (allocation->flags & CSALF_ONEFORALL)
               size /= surface->num_buffers;
          if (video) {
               if (allocation->access[CSAID_GPU])
                    mem += size;
          }
          else if (!allocation->access[CSAID_GPU])
               mem += size;
     }

     return mem;
}

static int
buffer_sizes( CoreSurface *surface, bool video )
{
     int i, mem = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          mem += buffer_size( surface, buffer, video );
     }

     return mem;
}

static int
buffer_locks( CoreSurface *surface, bool video )
{
     int i, locks = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          locks += buffer->locked;
     }

     return locks;
}

static bool
surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult ret;
     int          i;
     int          refs;
     CoreSurface *surface = (CoreSurface*) object;
     MemoryUsage *mem     = ctx;
     int          vmem;
     int          smem;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

#if 0
     if (dump_surface && ((dump_surface < 0 && surface->type & CSTF_SHARED) ||
                          (dump_surface == object->ref.multi.id)) && surface->num_buffers)
     {
          char buf[32];

          snprintf( buf, sizeof(buf), "dfb_surface_0x%08x", object->ref.multi.id );

          dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
     }
#else
     if(dump_surface && surface->num_buffers)
     {
          char buf[32];

          snprintf( buf, sizeof(buf), "dfb_surface_0x%08x_%p_", object->ref.multi.id, surface ); 
          
          if((dump_surface < 0 && (surface->type & CSTF_SHARED || getpid() == surface->pid))
            || (dump_surface == object->ref.multi.id && (surface->type & CSTF_SHARED || getpid() == surface->pid)))
          {
                dfb_surface_dump_buffer( surface, CSBR_FRONT, directory, buf );
          }
     }


#endif

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d[%p]   ", refs, surface );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     vmem = buffer_sizes( surface, true );
     smem = buffer_sizes( surface, false );

     mem->video += vmem;

     /* FIXME: assumes all buffers have this flag (or none) */
     /*if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
          mem->presys += smem;
     else*/
          mem->system += smem;

     if (vmem && vmem < 1024)
          vmem = 1024;

     if (smem && smem < 1024)
          smem = 1024;

     printf( "%5dk%c  ", vmem >> 10, buffer_locks( surface, true ) ? '*' : ' ' );
     printf( "%5dk%c  ", smem >> 10, buffer_locks( surface, false ) ? '*' : ' ' );

     /* FIXME: assumes all buffers have this flag (or none) */
//     if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
//          printf( "preallocated " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied" );

     printf("%d", surface->pid);

     printf( "\n" );

     if(dump_surcs && (getpid() == surface->pid))
     {
          for(i = 0; i < surface->stack_size; i++)
          {
               printf("%d: %s \n", i, surface->stack_string[i]);
          }
     }

     return true;
}

static void
dump_surfaces( void )
{
     printf( "\n"
             "-----------------------------[ Surfaces ]-------------------------------------\n" );
     printf( "Reference   FID  . Refs[surface]  Width Height  Format     Video   System  Capabilities pid\n" );
     printf( "------------------------------------------------------------------------------\n" );

     dfb_core_enum_surfaces( NULL, surface_callback, &mem );

     printf( "                                                ------   ------\n" );
     printf( "                                               %6dk  %6dk   -> %dk total\n",
             mem.video >> 10, (mem.system + mem.presys) >> 10,
             (mem.video + mem.system + mem.presys) >> 10);
}

/**********************************************************************************************************************/

static DFBEnumerationResult
alloc_callback( CoreSurfaceAllocation *alloc,
                void                  *ctx )
{
     int                i, index;
     CoreSurface       *surface;
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( alloc, CoreSurfaceAllocation );

     buffer  = alloc->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     printf( "%9lu %8d  ", alloc->offset, alloc->size );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     index = dfb_surface_buffer_index( alloc->buffer );

     printf( " %-5s ",
             (dfb_surface_get_buffer( surface, CSBR_FRONT ) == buffer) ? "front" :
             (dfb_surface_get_buffer( surface, CSBR_BACK  ) == buffer) ? "back"  :
             (dfb_surface_get_buffer( surface, CSBR_IDLE  ) == buffer) ? "idle"  : "" );

     printf( direct_serial_check(&alloc->serial, &buffer->serial) ? " * " : "   " );

     printf( "%d  %2lu  ", fusion_vector_size( &buffer->allocs ), surface->resource_id );

     if (surface->type & CSTF_SHARED)
          printf( "SHARED  " );
     else
          printf( "PRIVATE " );

     if (surface->type & CSTF_LAYER)
          printf( "LAYER " );

     if (surface->type & CSTF_WINDOW)
          printf( "WINDOW " );

     if (surface->type & CSTF_CURSOR)
          printf( "CURSOR " );

     if (surface->type & CSTF_FONT)
          printf( "FONT " );

     printf( " " );

     if (surface->type & CSTF_INTERNAL)
          printf( "INTERNAL " );

     if (surface->type & CSTF_EXTERNAL)
          printf( "EXTERNAL " );

     printf( " " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied" );

     printf( "\n" );

     return DFENUM_OK;
}

static DFBEnumerationResult
surface_pool_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int length;

     printf( "\n" );
     printf( "--------------------[ Surface Buffer Allocations in %s ]-------------------%n\n", pool->desc.name, &length );
     printf( "Offset    Length   Width Height     Format  Role  Up nA ID  Usage   Type / Storage / Caps\n" );

     while (length--)
          putc( '-', stdout );

     printf( "\n" );

     dfb_surface_pool_enumerate( pool, alloc_callback, NULL );

     return DFENUM_OK;
}

static void
dump_surface_pools( void )
{
     dfb_surface_pools_enumerate( surface_pool_callback, NULL );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
surface_pool_info_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int                    i;
     unsigned long          total = 0;
     CoreSurfaceAllocation *alloc;

     fusion_vector_foreach (alloc, i, pool->allocs)
          total += alloc->size;

     printf( "%-20s ", pool->desc.name );

     switch (pool->desc.priority) {
          case CSPP_DEFAULT:
               printf( "DEFAULT  " );
               break;

          case CSPP_PREFERED:
               printf( "PREFERED " );
               break;

          case CSPP_ULTIMATE:
               printf( "ULTIMATE " );
               break;

          default:
               printf( "unknown  " );
               break;
     }

     printf( "%6lu/%6luk  ", total / 1024, pool->desc.size / 1024 );

     if (pool->desc.types & CSTF_SHARED)
          printf( "* " );
     else
          printf( "  " );


     if (pool->desc.types & CSTF_INTERNAL)
          printf( "INT " );

     if (pool->desc.types & CSTF_EXTERNAL)
          printf( "EXT " );

     if (!(pool->desc.types & (CSTF_INTERNAL | CSTF_EXTERNAL)))
          printf( "    " );


     if (pool->desc.types & CSTF_LAYER)
          printf( "LAYER " );
     else
          printf( "      " );

     if (pool->desc.types & CSTF_WINDOW)
          printf( "WINDOW " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_CURSOR)
          printf( "CURSOR " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_FONT)
          printf( "FONT " );
     else
          printf( "     " );


     for (i=CSAID_CPU; i<=CSAID_GPU; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     for (i=CSAID_LAYER0; i<=CSAID_LAYER15; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     printf( "\n" );

     return DFENUM_OK;
}

static void
dump_surface_pool_info( void )
{
     printf( "\n" );
     printf( "-------------------------------------[ Surface Buffer Pools ]------------------------------------\n" );
     printf( "Name                 Priority   Used/Capacity S I/E Resource Type Support     CPU GPU Layer 0 - 2\n" );
     printf( "-------------------------------------------------------------------------------------------------\n" );

     dfb_surface_pools_enumerate( surface_pool_info_callback, NULL );
}

/**********************************************************************************************************************/

static bool
context_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult       ret;
     int                i;
     int                refs;
     int                level;
     CoreLayer         *layer   = (CoreLayer*) ctx;
     CoreLayerContext  *context = (CoreLayerContext*) object;
     CoreLayerRegion   *region  = NULL;
     CoreSurface       *surface = NULL;

     if (object->state != FOS_ACTIVE)
          return true;

     if (context->layer_id != dfb_layer_id( layer ))
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

     if (dump_layer && (dump_layer < 0 || dump_layer == object->ref.multi.id)) {
          if (dfb_layer_context_get_primary_region( context, false, &region ) == DFB_OK) {
               if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK) {
                    if (surface->num_buffers) {
                         char buf[32];
                         
                         snprintf( buf, sizeof(buf), "dfb_layer_context_0x%08x", object->ref.multi.id );

                         dfb_surface_dump_buffer( surface, CSBR_FRONT, directory, buf );
                    }

                    dfb_surface_unref( surface );
               }
          }
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d  ", context->config.width, context->config.height );

     for (i=0; format_names[i].format; i++) {
          if (context->config.pixelformat == format_names[i].format) {
               printf( "%-8s ", format_names[i].name );
               break;
          }
     }

     if (!format_names[i].format)
          printf( "unknown  " );

     printf( "%.1f, %.1f -> %.1f, %.1f   ",
             context->screen.location.x,  context->screen.location.y,
             context->screen.location.x + context->screen.location.w,
             context->screen.location.y + context->screen.location.h );

     printf( "%2d     ", fusion_vector_size( &context->regions ) );

     printf( context->active ? "(*)    " : "       " );

     if (context == layer->shared->contexts.primary)
          printf( "SHARED   " );
     else
          printf( "PRIVATE  " );

     if (context->rotation)
          printf( "ROTATED %d ", context->rotation);

     if (dfb_layer_get_level( layer, &level ))
          printf( "N/A" );
     else
          printf( "%3d", level );

     printf( "\n" );

     return true;
}

static void
dump_contexts( CoreLayer *layer )
{
     if (fusion_vector_size( &layer->shared->contexts.stack ) == 0)
          return;

     printf( "\n"
             "----------------------------------[ Contexts of Layer %d ]----------------------------------------\n", dfb_layer_id( layer ));
     printf( "Reference   FID  . Refs  Width Height Format   Location on screen  Regions  Active  Info    Level\n" );
     printf( "-------------------------------------------------------------------------------------------------\n" );

     dfb_core_enum_layer_contexts( NULL, context_callback, layer );
}

static DFBEnumerationResult
window_callback( CoreWindow *window,
                 void       *ctx )
{
     DirectResult      ret;
     int               refs;
     CoreWindowConfig *config = &window->config;
     DFBRectangle     *bounds = &config->bounds;

     ret = fusion_ref_stat( &window->object.ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return DFENUM_OK;
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", window->object.ref.multi.id, window->object.ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d, %4d   ", bounds->x, bounds->y );

     printf( "%4d x %4d    ", bounds->w, bounds->h );

     printf( "0x%02x ", config->opacity );

     printf( "%5d  ", window->id );

     switch (config->stacking) {
          case DWSC_UPPER:
               printf( "^  " );
               break;
          case DWSC_MIDDLE:
               printf( "-  " );
               break;
          case DWSC_LOWER:
               printf( "v  " );
               break;
          default:
               printf( "?  " );
               break;
     }

     if (window->caps & DWCAPS_ALPHACHANNEL)
          printf( "alphachannel   " );

     if (window->caps & DWCAPS_INPUTONLY)
          printf( "input only     " );

     if (window->caps & DWCAPS_DOUBLEBUFFER)
          printf( "double buffer  " );

     if (config->options & DWOP_GHOST)
          printf( "GHOST          " );

     if (DFB_WINDOW_FOCUSED( window ))
          printf( "FOCUSED        " );

     if (DFB_WINDOW_DESTROYED( window ))
          printf( "DESTROYED      " );

     if (window->config.rotation)
          printf( "ROTATED %d     ", window->config.rotation);

     printf( "\n" );

     return DFENUM_OK;
}

static void
dump_windows( CoreLayer *layer )
{
     DFBResult         ret;
     CoreLayerShared  *shared;
     CoreLayerContext *context;
     CoreWindowStack  *stack;

     shared = layer->shared;

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "DirectFB/Dump: Could not lock the shared layer data!\n" );
          return;
     }

     context = layer->shared->contexts.primary;
     if (!context) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     stack = dfb_layer_context_windowstack( context );
     if (!stack) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     dfb_windowstack_lock( stack );

     if (stack->num) {
          printf( "\n"
                  "-----------------------------------[ Windows of Layer %d ]-----------------------------------------\n", dfb_layer_id( layer ) );
          printf( "Reference   FID  . Refs     X     Y   Width Height Opacity   ID     Capabilities   State & Options\n" );
          printf( "--------------------------------------------------------------------------------------------------\n" );

          dfb_wm_enum_windows( stack, window_callback, NULL );
     }

     dfb_windowstack_unlock( stack );

     fusion_skirmish_dismiss( &shared->lock );
}

static DFBEnumerationResult
layer_callback( CoreLayer *layer,
                void      *ctx)
{
     dump_windows( layer );
     dump_contexts( layer );

     return DFENUM_OK;
}

static void
dump_layers( void )
{
     dfb_layers_enumerate( layer_callback, NULL );
}

/**********************************************************************************************************************/

#if FUSION_BUILD_MULTI
static DirectEnumerationResult
dump_shmpool( FusionSHMPool *pool,
              void          *ctx )
{
     DFBResult     ret;
     SHMemDesc    *desc;
     unsigned int  total = 0;
     int           length;
     FusionSHMPoolShared *shared = pool->shared;

     printf( "\n" );
     printf( "----------------------------[ Shared Memory in %s ]----------------------------%n\n", shared->name, &length );
     printf( "      Size          Address      Offset      Function                     FusionID\n" );

     while (length--)
          putc( '-', stdout );

     putc( '\n', stdout );

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "Could not lock shared memory pool!\n" );
          return DFENUM_OK;
     }

     if (shared->allocs) {
          direct_list_foreach (desc, shared->allocs) {
               printf( " %9zu bytes at %p [%8lu] in %-30s [%3lx] (%s: %u)\n",
                       desc->bytes, desc->mem, (ulong)desc->mem - (ulong)shared->heap,
                       desc->func, desc->fid, desc->file, desc->line );

               total += desc->bytes;
          }

          printf( "   -------\n  %7dk total\n", total >> 10 );
     }

     printf( "\nShared memory file size: %dk\n", shared->heap->size >> 10 );

     fusion_skirmish_dismiss( &shared->lock );

     return DFENUM_OK;
}

static void
dump_shmpools( void )
{
     fusion_shm_enum_pools( dfb_core_world(NULL), dump_shmpool, NULL );
}
#endif


static void
dump_main( const char *name, const char *value )
{
     if (!dfb_config)
          return;
     
     if (strcmp (name, "dfbdump:ds" ) == 0)
     {
          if(value)
               dump_surface = str_to_int(value);
          else
               dump_surface = -1;
     }
     else if (strcmp (name, "dfbdump:dl" ) == 0)
     {
          if(value)
               dump_layer = str_to_int(value);
          else
               dump_layer = -1;  
     }
     else if (strcmp (name, "dfbdump:shm" ) == 0)
     {
          if(value)
               show_shm = (bool) str_to_int(value);
          else
               show_shm = true;      
     }
     else if (strcmp (name, "dfbdump:pools" ) == 0)
     {
          if(value)
               show_pools = (bool) str_to_int(value);
          else
               show_pools = true;        
     }
     else if (strcmp (name, "dfbdump:allocs" ) == 0)
     {
          if(value)
               show_allocs = (bool) str_to_int(value);
          else
               show_allocs = true;
     }
     else if (strcmp (name, "dfbdump:surcs" ) == 0)
     {
          if(value)
               dump_surcs = (bool) str_to_int(value);
          else
               dump_surcs = true;    
     }
     else if(strcmp(name, "dfbdump:directory") == 0)
     {
          if(value)
               strcpy(directory, value);
          else
               strcpy(directory, "/mnt/usb/sda1");
     }
     else if(strcmp(name, "dfbdump:flags") == 0)
     {
          if(value)
          {
               if(strcmp(value, "blit1") == 0) 
               {
                   dump_flags |= (1 << 0);
               }
               else if(strcmp(value, "blit0") == 0) 
               {
                   dump_flags &= ~(1 << 0);
               }else if(strcmp(value, "stretchblit1") == 0) 
               {
                   dump_flags |= (1 << 1);
               }
               else if(strcmp(value, "stretchblit0") == 0) 
               {
                   dump_flags &= ~(1 << 1);
               }
               else if(strcmp(value, "batchblit1") == 0) 
               {
                   dump_flags |= (1 << 2);
               }
               else if(strcmp(value, "batchblit0") == 0) 
               {
                   dump_flags &= ~(1 << 2);
               }
               else if(strcmp(value, "fillrect1") == 0) 
               {
                   dump_flags |= (1 << 3);
               }
               else if(strcmp(value, "fillrect0") == 0) 
               {
                   dump_flags &= ~(1 << 3);
               }
               else if(strcmp(value, "drawrect1") == 0) 
               {
                   dump_flags |= (1 << 4);
               }
               else if(strcmp(value, "drawrect0") == 0) 
               {
                   dump_flags &= ~(1 << 4);
               }
               else if(strcmp(value, "performance1") == 0) 
               {
                   dump_flags |= (1 << 30);
               }               
               else if(strcmp(value, "performance0") == 0) 
               {
                   dump_flags &= ~(1 << 30);
               }               
          }
          else
          {
               dump_flags = 0xFFFFFFFF;
          }

          printf("dump_flags=0x%x \n", dump_flags);
          
     }
     else if(0 == strcmp (name, "dfbdump:action"))
     {
          if(!strstr(directory, "/mnt/usb/"))
          {
               strcpy(directory, "/mnt/usb/sda1");
          }
     
          dump_surfaces();
          fflush( stdout );

          dump_layers();
          fflush( stdout );

#if FUSION_BUILD_MULTI
          if (show_shm) {
               printf( "\n" );
               dump_shmpools();
               fflush( stdout );
          }
#endif

          if (show_pools) {
               printf( "\n" );
               dump_surface_pool_info();
               fflush( stdout );
          }

          if (show_allocs) {
               printf( "\n" );
               dump_surface_pools();
               fflush( stdout );
          }

          printf( "\n" );
     }
}


/**********************************************************************************************************************/

bool dfb_enable_dump(unsigned int flags)
{
     return (bool) ( dump_flags & (1 << flags) );
}





DFBResult dfb_config_cmdlnset( int argc, char *argv[] )
{
     int i;

     for(i = 0; i < argc; i++)
     {
         if (strncmp (argv[i], "dfblog:", 7) == 0) 
         {
              parse_args( (argv[i] + 7) );
         }
         else
         {
              parse_args( (argv[i]) );
         }
     }

     return DFB_OK;
}
#endif


DFBResult dfb_config_init( int *argc, char *(*argv[]) )
{
     DFBResult ret;
     int i;
#ifndef WIN32
     char *home = direct_getenv( "HOME" );
#endif
     char *prog = NULL;
     char *session;
     char *dfbargs;
     char *config_path = getenv("CONFIG_PATH");
#ifndef WIN32
     char  cmdbuf[1024];
#endif

#if USE_HASH_TABLE_SREACH
		 static bool bcreate_misc_htable_once = false;
		 if(!bcreate_misc_htable_once)
		 {
			 CreateConfigHashTable();
			 bcreate_misc_htable_once = true;
		 }
#endif

     if (dfb_config)
          return DFB_OK;

     config_allocate();

     /* Read system settings. */
     ret = dfb_config_read( SYSCONFDIR"/directfbrc" );
     if (ret  &&  ret != DFB_IO)
          return ret;


      /* Read new path settings. */
     if (config_path) {
          int  len = strlen(config_path) + strlen("/directfbrc") + 1;
          char buf[len];
          snprintf( buf, len, "%s/directfbrc", config_path );
          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }


#ifndef WIN32
     /* Read user settings. */
     if (home) {
          int  len = strlen(home) + strlen("/.directfbrc") + 1;
          char buf[len];

          direct_snprintf( buf, len, "%s/.directfbrc", home );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }
#endif


     dfb_config_read("/3rd/directfbrc");

     /* Get application name. */
     if (argc && *argc && argv && *argv) {
          prog = strrchr( (*argv)[0], '/' );

          if (prog)
               prog++;
          else
               prog = (*argv)[0];
     }
#ifndef WIN32
     else {
          /* if we didn't receive argc/argv we try the proc system */
          FILE *f;
          int   len;

          f = fopen( "/proc/self/cmdline", "r" );
          if (f) {
               len = fread( cmdbuf, 1, 1023, f );
               if (len) {
                    cmdbuf[len] = 0; /* in case of no arguments, or long program name */
                    prog = strrchr( cmdbuf, '/' );
                    if (prog)
                         prog++;
                    else
                         prog = cmdbuf;
               }
               fprintf(stderr,"commandline read: %s\n", prog );
               fclose( f );
          }
     }
#endif

     /* Strip lt- prefix. */
     if (prog) {
          if (prog[0] == 'l' && prog[1] == 't' && prog[2] == '-')
            prog += 3;
     }

#ifndef WIN32
     /* Read global application settings. */
     if (prog && prog[0]) {
          int  len = strlen( SYSCONFDIR"/directfbrc." ) + strlen(prog) + 1;
          char buf[len];

          direct_snprintf( buf, len, SYSCONFDIR"/directfbrc.%s", prog );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }

     /* Read user application settings. */
     if (home && prog && prog[0]) {
          int  len = strlen(home) + strlen("/.directfbrc.") + strlen(prog) + 1;
          char buf[len];

          direct_snprintf( buf, len, "%s/.directfbrc.%s", home, prog );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }
#endif

     /* Read settings from environment variable. */
     dfbargs = direct_getenv( "DFBARGS" );
     if (dfbargs) {
          ret = parse_args( dfbargs );
          if (ret)
               return ret;
     }

     /* Active session is used if present, only command line can override. */
     session = direct_getenv( "DIRECTFB_SESSION" );
     if (session)
          dfb_config_set( "session", session );

     /* Read settings from command line. */
     if (argc && argv) {
          for (i = 1; i < *argc; i++) {

               if (strcmp ((*argv)[i], "--dfb-help") == 0) {
                    print_config_usage();
                    exit(1);
               }

               if (strncmp ((*argv)[i], "--dfb:", 6) == 0) {
                    ret = parse_args( (*argv)[i] + 6 );
                    if (ret)
                         return ret;

                    (*argv)[i] = NULL;
               }
          }

          for (i = 1; i < *argc; i++) {
               int k;

               for (k = i; k < *argc; k++)
                    if ((*argv)[k] != NULL)
                         break;

               if (k > i) {
                    int j;

                    k -= i;

                    for (j = i + k; j < *argc; j++)
                         (*argv)[j-k] = (*argv)[j];

                    *argc -= k;
               }
          }
     }
#ifndef WIN32
     else if (prog) {
          /* we have prog, so we try again the proc filesystem */
          FILE *f;
          int   len;

          len = strlen( cmdbuf );
          f = fopen( "/proc/self/cmdline", "r" );
          if (f) {
               len = fread( cmdbuf, 1, len, f ); /* skip arg 0 */
               while( config_read_cmdline( cmdbuf, 1024, f ) ) {
                    fprintf(stderr,"commandline read: %s\n", cmdbuf );
                    if (strcmp (cmdbuf, "--dfb-help") == 0) {
                         print_config_usage();
                         exit(1);
                    }

                    if (strncmp (cmdbuf, "--dfb:", 6) == 0) {
                         ret = parse_args( cmdbuf + 6 );
                         if (ret) {
                              fclose( f );
                              return ret;
                         }
                    }
               }
               fclose( f );
          }
     }
#endif

     if (!dfb_config->vt_switch)
          dfb_config->kd_graphics = true;

     return DFB_OK;
}

DFBResult dfb_config_read( const char *filename )
{
     DFBResult ret = DFB_OK;
     char line[400];
     FILE *f;

     char *slash = 0;
     char *cwd   = 0;

     config_allocate();

     dfb_config->config_layer = &dfb_config->layers[0];

     f = fopen( filename, "r" );
     if (!f) {
          D_DEBUG_AT( DirectFB_Config, "Unable to open config file `%s'!\n", filename );
          return DFB_IO;
     } else {
          D_DEBUG_AT( DirectFB_Config, "Parsing config file '%s'.\n", filename );
     }

#ifndef WIN32
     /* store/restore the cwd (needed for the "include" command */
     slash = strrchr( filename, '/' );
     if( slash ) {
          cwd = malloc( 4096 );
          if (!cwd) {
               fclose( f );
               return D_OOM();
          }

          if (!getcwd( cwd, 4096 )) {
               ret = errno2result( errno );
               free( cwd );
               fclose( f );
               return ret;
          }

          /* must copy filename for path, due to const'ness */
          char nwd[strlen(filename) + 1];
          strncpy( nwd, filename, sizeof(nwd));
          nwd[slash-filename] = 0;
          chdir( nwd );

          D_DEBUG_AT( DirectFB_Config, "changing configuration lookup directory to '%s'.\n", nwd );
     }
#endif

     while (fgets( line, 400, f )) {
          char *name = line;
          char *comment = strchr( line, '#');
          char *value;

          if (comment) {
               *comment = 0;
          }

          value = strchr( line, '=' );

          if (value) {
               *value++ = 0;
               direct_trim( &value );
          }

          direct_trim( &name );

          if (!*name  ||  *name == '#')
               continue;

          ret = dfb_config_set( name, value );
          if (ret) {
               if (ret == DFB_UNSUPPORTED) {
                    D_ERROR( "DirectFB/Config: *********** In config file `%s': "
                             "Invalid option `%s'! ***********\n", filename, name );
                    ret = DFB_OK;
                    continue;
               }
               break;
          }
     }

     fclose( f );

#ifndef WIN32
     /* restore original cwd */
     if( cwd ) {
          chdir( cwd );
          free( cwd );
     }
#endif

     return ret;
}

#if !defined(__UCLIBC__)
#ifndef DFB_AARCH64
#include <execinfo.h>
#define TRACE_SIZE 256
static void sigalHandler(int sig, siginfo_t* sigInfo, void* context)
{
    void* trace[TRACE_SIZE] = {NULL};
    int n = backtrace(trace, sizeof(trace) / sizeof(trace[0]));
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("!!!!!!!!!!!!!!! [DFB] SIGNAL = %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", sig);
    printf("!!!!!!!!!!!!!!! [DFB] backtrace returned %d addresses!!!!!!!!!!!!!!!\n", n);
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    backtrace_symbols_fd(trace, n, fileno(stderr));
    exit(EXIT_FAILURE);
}

static void registerSigalstack()
{
    stack_t newStack;
    struct sigaction newSigaction;

    newStack.ss_sp = malloc(SIGSTKSZ); // 8192 byte
    if (!newStack.ss_sp)
        printf("newStack.ss_sp is NULL, malloc fail\n");

    newStack.ss_size = SIGSTKSZ; // 8192 byte
    newStack.ss_flags = 0;

    if (sigaltstack(&newStack, 0) == -1)
        printf("sigaltstack == -1, that is fail\n");
        
    newSigaction.sa_sigaction = sigalHandler;
    newSigaction.sa_flags = 0;
    if (sigemptyset(&newSigaction.sa_mask) < 0)
        printf("set new action mask error\n");

    if (sigaction(SIGSEGV, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");

    if (sigaction(SIGFPE, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");

    if (sigaction(SIGBUS, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");       

}
#endif
#endif


#if USE_HASH_TABLE_SREACH

void FUN___system(char *value)
{
    if (value)
    {
        if (dfb_config->system)
            D_FREE( dfb_config->system );
        dfb_config->system = D_STRDUP( value );
    }
    else
    {
        printf("DirectFB/Config 'system': No system specified!\n");
    }
}

void FUN___wm(char *value)
{
    if (value)
    {
        if (dfb_config->wm)
            D_FREE( dfb_config->wm );
        dfb_config->wm = D_STRDUP( value );
    }
    else
    {
        printf("DirectFB/Config 'wm': No window manager specified!\n");

    }
}

void FUN___fbdev(char *value)
{
    if (value)
    {
        if (dfb_config->fb_device)
            D_FREE( dfb_config->fb_device );
        dfb_config->fb_device = D_STRDUP( value );
    }
    else
    {
        printf("DirectFB/Config 'fbdev': No device __FUNCTION__ specified!\n");

    }
}

void FUN___busid(char *value)
{
    if (value)
    {
        int bus, dev, func;

        if (sscanf( value, "%d:%d:%d", &bus, &dev, &func ) != 3)
        {
            printf( "DirectFB/Config 'busid': Could not parse busid!\n");

        }

        dfb_config->pci.bus  = bus;
        dfb_config->pci.dev  = dev;
        dfb_config->pci.func = func;
    }
}

void FUN___pci_id(char *value)
{
    if (value)
    {
        int bus, dev, func;

        if (sscanf( value, "%d:%d:%d", &bus, &dev, &func ) != 3)
        {
            printf( "DirectFB/Config 'busid': Could not parse busid!\n");

        }

        dfb_config->pci.bus  = bus;
        dfb_config->pci.dev  = dev;
        dfb_config->pci.func = func;
    }
}

void FUN___screenshot_dir(char *value)
{
    if (value)
    {
        if (dfb_config->screenshot_dir)
            D_FREE( dfb_config->screenshot_dir );
        dfb_config->screenshot_dir = D_STRDUP( value );
    }
    else
        printf("DirectFB/Config 'screenshot-dir': No directory name specified!\n");
}

void FUN___scaled(char *value)
{
    if (value)
    {
        int width, height;

        if (sscanf( value, "%dx%d", &width, &height ) < 2)
            printf("DirectFB/Config 'scaled': Could not parse size!\n");

        dfb_config->scaled.width  = width;
        dfb_config->scaled.height = height;
    }
    else
        printf("DirectFB/Config 'scaled': No size specified!\n");

}

void FUN___primary_layer(char *value)
{
    if (value)
    {
        int id;

        if (sscanf( value, "%d", &id ) < 1)
            printf("DirectFB/Config 'primary-layer': Could not parse id!\n");

        dfb_config->primary_layer = id;
    }
    else
        printf("DirectFB/Config 'primary-layer': No id specified!\n");
}

void FUN___primary_only(char *value)
{
    dfb_config->primary_only = true;
}

void FUN___font_format(char *value)
{
    if (value)
    {
        DFBSurfacePixelFormat format;

        format = dfb_config_parse_pixelformat( value );
        if (format == DSPF_UNKNOWN)
            printf("DirectFB/Config 'font-format': Could not parse format!\n");

        dfb_config->font_format = format;
    }
    else
        printf("DirectFB/Config 'font-format': No format specified!\n");
}

void FUN___font_premult(char *value) {        
    dfb_config->font_premult = true;        
}        
         
void FUN___no_font_premult(char *value) {        
    dfb_config->font_premult = false;        
}         
        
void FUN___surface_shmpool_size(char *value) {        
    if (value) {        
           int size_kb;        
        
           if (sscanf( value, "%d", &size_kb ) < 1){        
                printf( "DirectFB/Config '%s': Could not parse value!\n", __FUNCTION__);        
                        
         }        
        
    dfb_config->surface_shmpool_size = size_kb * 1024;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        

void FUN___session(char *value) {        
    if (value) {        
           int session;        
        
           if (sscanf( value, "%d", &session ) < 1){        
                printf("DirectFB/Config 'session': Could not parse value!\n");        
                        
         }        
        
    dfb_config->session = session;        
    }        
    else {        
           printf("DirectFB/Config 'session': No value specified!\n");        
                   
    }        
}        
         
void FUN___remote(char *value) {        
    if (value) {        
           char host[128];        
           int  session = 0;        
        
           if (sscanf( value, "%127s:%d", host, &session ) < 1){        
                printf("DirectFB/Config 'remote': "        
                        "Could not parse value (format is <host>[:<session>])!\n");        
                        
         }        
        
           if (dfb_config->remote.host)        
                D_FREE( dfb_config->remote.host );        
        
    dfb_config->remote.host    = D_STRDUP( host );        
    dfb_config->remote.session = session;        
    }        
    else {        
           printf("DirectFB/Config 'remote': No value specified!\n");        
                   
    }        
}        
         
void FUN___videoram_limit(char *value) {        
    if (value) {        
           int limit;        
        
           if (sscanf( value, "%d", &limit ) < 1){        
                printf("DirectFB/Config 'videoram-limit': Could not parse value!\n");        
                        
         }        
        
           if (limit < 0)        
                limit = 0;        
        
    dfb_config->videoram_limit = limit << 10;        
    }        
    else {        
           printf("DirectFB/Config 'videoram-limit': No value specified!\n");        
                   
    }        
}        
         
void FUN___keep_accumulators(char *value) {        
    if (value) {        
           int limit;        
        
           if (sscanf( value, "%d", &limit ) < 1){        
                printf("DirectFB/Config '%s': Could not parse value!\n", __FUNCTION__);        
                        
         }        
        
    dfb_config->keep_accumulators = limit;        
    }        
    else {        
           printf("DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___banner(char *value) {        
    dfb_config->banner = true;        
}        
         
void FUN___no_banner(char *value) {        
    dfb_config->banner = false;        
}        
         
void FUN___surface_sentinel(char *value) {        
    dfb_config->surface_sentinel = true;        
}        
         
void FUN___no_surface_sentinel(char *value) {        
    dfb_config->surface_sentinel = false;        
}        
         
void FUN___force_windowed(char *value) {        
    dfb_config->force_windowed = true;        
}        
         
void FUN___force_desktop(char *value) {        
    dfb_config->force_desktop = true;        
}        
         
void FUN___hardware(char *value) {        
    dfb_config->software_only = false;        
}        
         
void FUN___no_hardware(char *value) {        
    dfb_config->software_only = true;        
}        
         
void FUN___software(char *value) {        
    dfb_config->hardware_only = false;        
}        
         
void FUN___no_software(char *value) {        
    dfb_config->hardware_only = true;        
}        
         
void FUN___software_warn(char *value) {        
    dfb_config->software_warn = true;        
}        
         
void FUN___no_software_warn(char *value) {        
    dfb_config->software_warn = false;        
}        
         
void FUN___software_trace(char *value) {        
    dfb_config->software_trace = true;        
}        
         
void FUN___no_software_trace(char *value) {        
    dfb_config->software_trace = false;        
}        
         
void FUN___small_font_patch(char *value) {        
    dfb_config->small_font_patch = true;        
}        
         
void FUN___warn(char *value) {        
      /* Enable/disable all at once by default. */        
      DFBConfigWarnFlags flags = DMT_ALL;        
        
      /* Find out the specific message type being configured. */        
    if (value) {        
           char *opt = strchr( value, ':' );        
        
           if (opt)        
                opt++;        
        
           if (!strncmp( value, "create-surface", 14 )){        
                flags = DCWF_CREATE_SURFACE;        
        
                if (opt)        
                     sscanf( opt, "%dx%d",        
                             &dfb_config->warn.create_surface.min_size.w,        
                             &dfb_config->warn.create_surface.min_size.h );        
    }else        
           if (!strncmp( value, "create-window", 13 )){        
                flags = DCWF_CREATE_WINDOW;        
    }else        
           if (!strncmp( value, "allocate-buffer", 15 )){        
                flags = DCWF_ALLOCATE_BUFFER;        
        
                if (opt)        
                     sscanf( opt, "%dx%d",        
                             &dfb_config->warn.allocate_buffer.min_size.w,        
                             &dfb_config->warn.allocate_buffer.min_size.h );        
         }        
         else {        
                printf( "DirectFB/Config '%s': Unknown warning type '%s'!\n", __FUNCTION__, value );        
                        
         }        
    }        
        
      /* Set/clear the corresponding flag in the configuration. */        
    dfb_config->warn.flags |= flags;        
        
}        
        
void FUN___no_warn(char *value) {        
      /* Enable/disable all at once by default. */        
      DFBConfigWarnFlags flags = DMT_ALL;        
        
      /* Find out the specific message type being configured. */        
    if (value) {        
           char *opt = strchr( value, ':' );        
        
           if (opt)        
                opt++;        
        
           if (!strncmp( value, "create-surface", 14 )){        
                flags = DCWF_CREATE_SURFACE;        
        
                if (opt)        
                     sscanf( opt, "%dx%d",        
                             &dfb_config->warn.create_surface.min_size.w,        
                             &dfb_config->warn.create_surface.min_size.h );        
    }else        
           if (!strncmp( value, "create-window", 13 )){        
                flags = DCWF_CREATE_WINDOW;        
    }else        
           if (!strncmp( value, "allocate-buffer", 15 )){        
                flags = DCWF_ALLOCATE_BUFFER;        
        
                if (opt)        
                     sscanf( opt, "%dx%d",        
                             &dfb_config->warn.allocate_buffer.min_size.w,        
                             &dfb_config->warn.allocate_buffer.min_size.h );        
         }        
         else {        
                printf( "DirectFB/Config '%s': Unknown warning type '%s'!\n", __FUNCTION__, value );        
                        
         }        
    }        
        
      /* Set/clear the corresponding flag in the configuration. */        
        
    dfb_config->warn.flags &= ~flags;        
}        
         
void FUN___dma(char *value) {        
    dfb_config->dma = true;        
}        
         
void FUN___no_dma(char *value) {        
    dfb_config->dma = false;        
}        
         
void FUN___mmx(char *value) {        
    dfb_config->mmx = true;        
}        
         
void FUN___no_mmx(char *value) {        
    dfb_config->mmx = false;        
}        
         
void FUN___agp(char *value) {        
    if (value) {        
           int mode;        
        
           if (sscanf( value, "%d", &mode ) < 1 || mode < 0 || mode > 8){        
                printf( "DirectFB/Config 'agp': "        
                         "invalid agp mode '%s'!\n", value );        
                        
         }        
        
    dfb_config->agp = mode;        
    }        
    else {        
    dfb_config->agp = 8; /* maximum possible */        
    }        
}        
         
void FUN___thrifty_surface_buffers(char *value) {        
    dfb_config->thrifty_surface_buffers = true;        
}        
         
void FUN___no_thrifty_surface_buffers(char *value) {        
    dfb_config->thrifty_surface_buffers = false;        
}        
         
void FUN___no_agp(char *value) {        
    dfb_config->agp = 0;        
}        
         
void FUN___agpmem_limit(char *value) {        
    if (value) {        
           int limit;        
        
           if (sscanf( value, "%d", &limit ) < 1){        
                printf( "DirectFB/Config 'agpmem-limit': "        
                         "Could not parse value!\n" );        
                        
         }        
        
           if (limit < 0)        
                limit = 0;        
        
    dfb_config->agpmem_limit = limit << 10;        
    }        
    else {        
           printf("DirectFB/Config 'agpmem-limit': No value specified!\n");        
                   
    }        
}        
         
void FUN___vt(char *value) {        
    dfb_config->vt = true;        
}        
         
void FUN___no_vt(char *value) {        
    dfb_config->vt = false;        
}        
         
void FUN___block_all_signals(char *value) {        
    dfb_config->block_all_signals = true;        
}        
         
void FUN___deinit_check(char *value) {        
    dfb_config->deinit_check = true;        
}        
         
void FUN___no_deinit_check(char *value) {        
    dfb_config->deinit_check = false;        
}        
         
void FUN___cursor(char *value) {        
    dfb_config->no_cursor = false;        
}        
         
void FUN___no_cursor(char *value) {        
    dfb_config->no_cursor = true;        
}        
         
void FUN___cursor_updates(char *value) {        
    dfb_config->no_cursor_updates = false;        
}        
         
void FUN___no_cursor_updates(char *value) {        
    dfb_config->no_cursor_updates = true;        
}        
         
void FUN___linux_input_ir_only(char *value) {        
    dfb_config->linux_input_ir_only = true;        
}        
         
void FUN___linux_input_grab(char *value) {        
    dfb_config->linux_input_grab = true;        
}        
         
void FUN___no_linux_input_grab(char *value) {        
    dfb_config->linux_input_grab = false;        
}        
         
void FUN___motion_compression(char *value) {        
    dfb_config->mouse_motion_compression = true;        
}        
         
void FUN___no_motion_compression(char *value) {        
    dfb_config->mouse_motion_compression = false;        
}        
         
void FUN___mouse_protocol(char *value) {        
    if (value) {        
        dfb_config->mouse_protocol = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No mouse protocol specified!\n" );        
                   
    }        
}        
         
void FUN___mouse_source(char *value) {        
    if (value) {        
        D_FREE( dfb_config->mouse_source );        
        dfb_config->mouse_source = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No mouse source specified!\n" );        
                   
    }        
}        
         
void FUN___mouse_gpm_source(char *value) {        
    dfb_config->mouse_gpm_source = true;        
     D_FREE( dfb_config->mouse_source );        
     dfb_config->mouse_source = D_STRDUP( DEV_NAME_GPM );        
}        
         
void FUN___no_mouse_gpm_source(char *value) {        
    dfb_config->mouse_gpm_source = false;        
      D_FREE( dfb_config->mouse_source );        
    dfb_config->mouse_source = D_STRDUP( DEV_NAME );        
}        
         
void FUN___smooth_upscale(char *value) {        
    dfb_config->render_options |= DSRO_SMOOTH_UPSCALE;        
}        
         
void FUN___no_smooth_upscale(char *value) {        
    dfb_config->render_options &= ~DSRO_SMOOTH_UPSCALE;        
}        
         
void FUN___smooth_downscale(char *value) {        
    dfb_config->render_options |= DSRO_SMOOTH_DOWNSCALE;        
}        
         
void FUN___no_smooth_downscale(char *value) {        
    dfb_config->render_options &= ~DSRO_SMOOTH_DOWNSCALE;        
}        
         
void FUN___translucent_windows(char *value) {        
    dfb_config->translucent_windows = true;        
}        
         
void FUN___no_translucent_windows(char *value) {        
    dfb_config->translucent_windows = false;        
}        
         
void FUN___decorations(char *value) {        
    dfb_config->decorations = true;        
}        
         
void FUN___no_decorations(char *value) {        
    dfb_config->decorations = false;        
}        
         
void FUN___startstop(char *value) {        
    dfb_config->startstop = true;        
}        
         
void FUN___no_startstop(char *value) {        
    dfb_config->startstop = false;        
}        
         
void FUN___autoflip_window(char *value) {        
    dfb_config->autoflip_window = true;        
}        
         
void FUN___no_autoflip_window(char *value) {        
    dfb_config->autoflip_window = false;        
}        
         
void FUN___vsync_none(char *value) {        
    dfb_config->pollvsync_none = true;        
}        
         
void FUN___vsync_after(char *value) {        
    dfb_config->pollvsync_after = true;        
}        
         
void FUN___vt_switch(char *value) {        
    dfb_config->vt_switch = true;        
}        
         
void FUN___no_vt_switch(char *value) {        
    dfb_config->vt_switch = false;        
}        
         
void FUN___vt_num(char *value) {        
    if (value) {        
           int vt_num;        
        
           if (sscanf( value, "%d", &vt_num ) < 1){        
                printf("DirectFB/Config 'vt-num': Could not parse value!\n");        
                        
         }        
        
    dfb_config->vt_num = vt_num;        
    }        
    else {        
           printf("DirectFB/Config 'vt-num': No value specified!\n");        
                   
    }        
}        
         
void FUN___vt_switching(char *value) {        
    dfb_config->vt_switching = true;        
}        
         
void FUN___no_vt_switching(char *value) {        
    dfb_config->vt_switching = false;        
}        
         
void FUN___graphics_vt(char *value) {        
    dfb_config->kd_graphics = true;        
}        
         
void FUN___no_graphics_vt(char *value) {        
    dfb_config->kd_graphics = false;        
}        
         
void FUN___window_surface_policy(char *value) {        
    if (value) {        
       if (strcmp( value, "auto" ) == 0) {        
            dfb_config->window_policy = -1;        
       } else        
       if (strcmp( value, "videohigh" ) == 0) {        
            dfb_config->window_policy = CSP_VIDEOHIGH;        
       } else        
       if (strcmp( value, "videolow" ) == 0) {        
            dfb_config->window_policy = CSP_VIDEOLOW;        
       } else        
       if (strcmp( value, "systemonly" ) == 0) {        
            dfb_config->window_policy = CSP_SYSTEMONLY;        
       } else        
       if (strcmp( value, "videoonly" ) == 0) {        
            dfb_config->window_policy = CSP_VIDEOONLY;        
       }        
       else {        
            printf( "DirectFB/Config: "        
                     "Unknown window surface policy `%s'!\n", value );        
                    
       }        
    }        
    else {        
       printf( "DirectFB/Config: "        
                "No window surface policy specified!\n" );        
               
    }        
}        
         
void FUN___init_layer(char *value) {        
    if (value) {        
           int id;        
        
           if (sscanf( value, "%d", &id ) < 1){        
                printf("DirectFB/Config '%s': Could not parse id!\n", __FUNCTION__);        
                        
         }        
        
           if (id < 0 || id >= D_ARRAY_SIZE(dfb_config->layers)){                      //Coverity-03132013        
                printf("DirectFB/Config '%s': ID %d out of bounds!\n", __FUNCTION__, id);        
                        
         }        
        
    dfb_config->layers[id].init = true;        
        
    dfb_config->config_layer = &dfb_config->layers[id];        
    }        
    else {        
           printf("DirectFB/Config '%s': No id specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___no_init_layer(char *value) {        
    if (value) {        
           int id;        
        
           if (sscanf( value, "%d", &id ) < 1){        
                printf("DirectFB/Config '%s': Could not parse id!\n", __FUNCTION__);        
                        
         }        
        
           if (id < 0 || id > D_ARRAY_SIZE(dfb_config->layers)){        
                printf("DirectFB/Config '%s': ID %d out of bounds!\n", __FUNCTION__, id);        
                        
         }        
        
    dfb_config->layers[id].init = false;        
        
    dfb_config->config_layer = &dfb_config->layers[id];        
    }        
      else        
    dfb_config->layers[0].init = false;        
}        
         
void FUN___mode(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           int width, height;        
        
           if (sscanf( value, "%dx%d", &width, &height ) < 2){        
                printf("DirectFB/Config '%s': Could not parse width and height!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0]){        
       dfb_config->mode.width  = width;        
       dfb_config->mode.height = height;        
         }        
        
           conf->config.width  = width;        
           conf->config.height = height;        
        
           conf->config.flags |= DLCONF_WIDTH | DLCONF_HEIGHT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No width and height specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___layer_size(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           int width, height;        
        
           if (sscanf( value, "%dx%d", &width, &height ) < 2){        
                printf("DirectFB/Config '%s': Could not parse width and height!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0]){        
       dfb_config->mode.width  = width;        
       dfb_config->mode.height = height;        
         }        
        
           conf->config.width  = width;        
           conf->config.height = height;        
        
           conf->config.flags |= DLCONF_WIDTH | DLCONF_HEIGHT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No width and height specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___depth(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           int depth;        
        
           if (sscanf( value, "%d", &depth ) < 1){        
                printf("DirectFB/Config '%s': Could not parse value!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0]){        
       dfb_config->mode.depth = depth;        
         }        
        
           conf->config.pixelformat = dfb_pixelformat_for_depth( depth );        
           conf->config.flags      |= DLCONF_PIXELFORMAT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___layer_depth(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           int depth;        
        
           if (sscanf( value, "%d", &depth ) < 1){        
                printf("DirectFB/Config '%s': Could not parse value!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0]){        
       dfb_config->mode.depth = depth;        
         }        
        
           conf->config.pixelformat = dfb_pixelformat_for_depth( depth );        
           conf->config.flags      |= DLCONF_PIXELFORMAT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___pixelformat(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           DFBSurfacePixelFormat format;        
        
           format = dfb_config_parse_pixelformat( value );        
           if (format == DSPF_UNKNOWN){        
                printf("DirectFB/Config '%s': Could not parse format!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0])        
       dfb_config->mode.format = format;        
        
           conf->config.pixelformat = format;        
           conf->config.flags      |= DLCONF_PIXELFORMAT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No format specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___layer_format(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           DFBSurfacePixelFormat format;        
        
           format = dfb_config_parse_pixelformat( value );        
           if (format == DSPF_UNKNOWN){        
                printf("DirectFB/Config '%s': Could not parse format!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0])        
       dfb_config->mode.format = format;        
        
           conf->config.pixelformat = format;        
           conf->config.flags      |= DLCONF_PIXELFORMAT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No format specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___desktop_buffer_mode(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
       if (strcmp( value, "auto" ) == 0) {        
            conf->config.flags &= ~DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "triple" ) == 0) {        
            conf->config.buffermode = DLBM_TRIPLE;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "backvideo" ) == 0) {        
            conf->config.buffermode = DLBM_BACKVIDEO;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "backsystem" ) == 0) {        
            conf->config.buffermode = DLBM_BACKSYSTEM;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "frontonly" ) == 0) {        
            conf->config.buffermode = DLBM_FRONTONLY;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "windows" ) == 0) {        
            conf->config.buffermode = DLBM_WINDOWS;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else {        
            printf( "DirectFB/Config '%s': Unknown mode '%s'!\n", __FUNCTION__, value );        
                    
       }        
    }        
    else {        
       printf( "DirectFB/Config '%s': No buffer mode specified!\n", __FUNCTION__ );        
               
    }        
        
        
         
}        
        
void FUN___layer_buffer_mode(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
       if (strcmp( value, "auto" ) == 0) {        
            conf->config.flags &= ~DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "triple" ) == 0) {        
            conf->config.buffermode = DLBM_TRIPLE;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "backvideo" ) == 0) {        
            conf->config.buffermode = DLBM_BACKVIDEO;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "backsystem" ) == 0) {        
            conf->config.buffermode = DLBM_BACKSYSTEM;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "frontonly" ) == 0) {        
            conf->config.buffermode = DLBM_FRONTONLY;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else        
       if (strcmp( value, "windows" ) == 0) {        
            conf->config.buffermode = DLBM_WINDOWS;        
            conf->config.flags     |= DLCONF_BUFFERMODE;        
       } else {        
            printf( "DirectFB/Config '%s': Unknown mode '%s'!\n", __FUNCTION__, value );        
                    
       }        
    }        
    else {        
       printf( "DirectFB/Config '%s': No buffer mode specified!\n", __FUNCTION__ );        
               
    }        
}        
        
         
void FUN___layer_src_key(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *error;        
           u32   argb;        
        
           argb = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in color '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
           conf->src_key.b = argb & 0xFF;        
           argb >>= 8;        
           conf->src_key.g = argb & 0xFF;        
           argb >>= 8;        
           conf->src_key.r = argb & 0xFF;        
           argb >>= 8;        
           conf->src_key.a = argb & 0xFF;        
        
           conf->config.options |= DLOP_SRC_COLORKEY;        
           conf->config.flags   |= DLCONF_OPTIONS;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No color specified!\n", __FUNCTION__ );        
                   
    }        
}        
         
void FUN___layer_src_key_index(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *error;        
           u32   index;        
        
           index = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in index '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
           conf->src_key_index = index;        
           conf->config.options |= DLOP_SRC_COLORKEY;        
           conf->config.flags   |= DLCONF_OPTIONS;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No index specified!\n", __FUNCTION__ );        
                   
    }        
}        
         
void FUN___bg_none(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
      conf->background.mode = DLBM_DONTCARE;        
}        
        
void FUN___layer_bg_none(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
      conf->background.mode = DLBM_DONTCARE;        
}        
        
void FUN___bg_image(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           if (conf->background.filename)        
                D_FREE( conf->background.filename );        
        
           conf->background.filename = D_STRDUP( value );        
           conf->background.mode     = DLBM_IMAGE;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No filename specified!\n", __FUNCTION__ );        
                   
    }        
}        
        
void FUN___bg_tile(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           if (conf->background.filename)        
                D_FREE( conf->background.filename );        
        
           conf->background.filename = D_STRDUP( value );        
           conf->background.mode     = DLBM_TILE;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No filename specified!\n", __FUNCTION__ );        
                   
    }        
}        
        
void FUN___layer_bg_image(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           if (conf->background.filename)        
                D_FREE( conf->background.filename );        
        
           conf->background.filename = D_STRDUP( value );        
           conf->background.mode     = DLBM_IMAGE;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No filename specified!\n", __FUNCTION__ );        
                   
    }        
}        
        
void FUN___layer_bg_tile(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           if (conf->background.filename)        
                D_FREE( conf->background.filename );        
        
           conf->background.filename = D_STRDUP( value );        
           conf->background.mode     = DLBM_TILE;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No filename specified!\n", __FUNCTION__ );        
                   
    }        
}        
         
         
         
void FUN___bg_color(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *error;        
           u32   argb;        
        
           argb = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in color '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
           conf->background.color.b = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.g = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.r = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.a = argb & 0xFF;        
        
           conf->background.color_index = -1;        
           conf->background.mode        = DLBM_COLOR;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No color specified!\n", __FUNCTION__ );        
                   
    }        
}        
        
void FUN___layer_bg_color(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *error;        
           u32   argb;        
        
           argb = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in color '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
           conf->background.color.b = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.g = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.r = argb & 0xFF;        
           argb >>= 8;        
           conf->background.color.a = argb & 0xFF;        
        
           conf->background.color_index = -1;        
           conf->background.mode        = DLBM_COLOR;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No color specified!\n", __FUNCTION__ );        
                   
    }        
}        
        
void FUN___layer_bg_color_index(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *error;        
           u32   index;        
        
           index = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in index '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
           conf->background.color_index = index;        
           conf->background.mode        = DLBM_COLOR;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No index specified!\n", __FUNCTION__ );        
                   
    }        
}        
         
void FUN___layer_stacking(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           char *stackings = D_STRDUP( value );        
           char *p = NULL, *r, *s = stackings;        
        
           conf->stacking = 0;        
        
           while ((r = strtok_r( s, ",", &p ))){        
                direct_trim( &r );        
        
                if (!strcmp( r, "lower" ))        
                     conf->stacking |= (1 << DWSC_LOWER);        
                       
                else if(!strcmp( r, "middle" ))        
                     conf->stacking |= (1 << DWSC_MIDDLE);        
                       
                else if(!strcmp( r, "upper" ))        
                     conf->stacking |= (1 << DWSC_UPPER);        
              else {        
                     printf( "DirectFB/Config '%s': Unknown class '%s'!\n", __FUNCTION__, r );        
                     D_FREE( stackings );        
                             
              }        
        
                s = NULL;        
         }        
        
           D_FREE( stackings );        
    }        
    else {        
           printf( "DirectFB/Config '%s': Missing value!\n", __FUNCTION__ );        
                   
    }        
}        

void FUN___layer_palette(char *value)
{
    int             index = 0;
    u32             color;

    DFBConfigLayer *conf = dfb_config->config_layer;

    if (value)
    {

        if (sscanf( value, "%d-%x", &index, &color ) < 2)
            printf("DirectFB/Config '%s': Could not parse index and color!\n", __FUNCTION__);

        if (index < 0 || index > 255)
        {
            D_ERROR("DirectFB/Config '%s': Index %d out of bounds!\n", __FUNCTION__, index);
            return ;
        }

        u32   argb = color;

        //argb = strtoul( color, &error, 16 );

        if (!conf->palette)
        {
            conf->palette = D_CALLOC( 256, sizeof(DFBColor) );

            if (!conf->palette)
                return ;
        }

        conf->palette[index].a = (argb & 0xFF000000) >> 24;
        conf->palette[index].r = (argb & 0xFF0000) >> 16;
        conf->palette[index].g = (argb & 0xFF00) >> 8;
        conf->palette[index].b = (argb & 0xFF);

        conf->palette_set = true;

    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No color specified!\n", __FUNCTION__ );
        return ;
    }
}


void FUN___layer_rotate(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           int rotate;        
        
           if (sscanf( value, "%d", &rotate ) < 1){        
                printf("DirectFB/Config '%s': Could not parse value!\n", __FUNCTION__);        
                        
         }        
        
           if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270){        
                printf("DirectFB/Config '%s': Only 0, 90, 180 or 270 supported!\n", __FUNCTION__);        
                        
         }        
        
           conf->rotate = rotate;        
    }        
    else {        
           printf("DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_phys(char *value) {        
    if (value) {        
           char *error;        
           u64 phys = 0;
        
           phys = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in hex value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_phys_hal = phys;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_phys_secondary(char *value) {        
    if (value) {        
           char *error;        
           u64 phys = 0;
        
           phys = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in hex value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_phys_secondary_hal= phys;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_length_secondary(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_length_secondary = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mmio_phys(char *value) {        
    if (value) {        
           char *error;        
           u64 phys = 0;
        
           phys = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in hex value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mmio_phys = phys;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mmio_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mmio_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___accelerator(char *value) {        
    if (value) {        
           char *error;        
           ulong accel;        
        
           accel = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->accelerator = accel;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___matrox_tv_standard(char *value) {        
    if (value) {        
       if (strcmp( value, "pal-60" ) == 0) {        
            dfb_config->matrox_tv_std = DSETV_PAL_60;        
       } else        
       if (strcmp( value, "pal" ) == 0) {        
            dfb_config->matrox_tv_std = DSETV_PAL;        
       } else        
       if (strcmp( value, "ntsc" ) == 0) {        
            dfb_config->matrox_tv_std = DSETV_NTSC;        
       } else {        
            printf( "DirectFB/Config: Unknown TV standard "        
                     "'%s'!\n", value );        
                    
       }        
    }        
    else {        
       printf( "DirectFB/Config: "        
                "No TV standard specified!\n" );        
               
    }        
}        
         
void FUN___matrox_cable_type(char *value) {        
    if (value) {        
       if (strcmp( value, "composite" ) == 0) {        
            dfb_config->matrox_cable = 0;        
       } else        
       if (strcmp( value, "scart-rgb" ) == 0) {        
            dfb_config->matrox_cable = 1;        
       } else        
       if (strcmp( value, "scart-composite" ) == 0) {        
            dfb_config->matrox_cable = 2;        
       } else {        
            printf( "DirectFB/Config: Unknown cable type "        
                     "'%s'!\n", value );        
                    
       }        
    }        
    else {        
       printf( "DirectFB/Config: "        
                "No cable type specified!\n" );        
               
    }        
}        
         
void FUN___matrox_sgram(char *value) {        
    dfb_config->matrox_sgram = true;        
}        
         
void FUN___matrox_crtc2(char *value) {        
    dfb_config->matrox_crtc2 = true;        
}        
         
void FUN___no_matrox_sgram(char *value) {        
    dfb_config->matrox_sgram = false;        
}        
         
void FUN___sync(char *value) {        
    dfb_config->sync = true;        
}        
         
void FUN___no_sync(char *value) {        
    dfb_config->sync = false;        
}        
         
void FUN___lefty(char *value) {        
    dfb_config->lefty = true;        
}        
         
void FUN___no_lefty(char *value) {        
    dfb_config->lefty = false;        
}        
         
void FUN___capslock_meta(char *value) {        
    dfb_config->capslock_meta = true;        
}        
         
void FUN___no_capslock_meta(char *value) {        
    dfb_config->capslock_meta = false;        
}        
         
void FUN___h3600_device(char *value) {        
    if (value) {        
           if (dfb_config->h3600_device)        
                D_FREE( dfb_config->h3600_device );        
        
    dfb_config->h3600_device = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No H3600 TS device specified!\n" );        
                   
    }        
}        
         
void FUN___mut_device(char *value) {        
    if (value) {        
           if (dfb_config->mut_device)        
                D_FREE( dfb_config->mut_device );        
        
    dfb_config->mut_device = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No MuTouch device specified!\n" );        
                   
    }        
}        
         
void FUN___zytronic_device(char *value) {        
    if (value) {        
           if (dfb_config->zytronic_device)        
                D_FREE( dfb_config->zytronic_device );        
        
    dfb_config->zytronic_device = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No Zytronic device specified!\n" );        
                   
    }        
}        
         
void FUN___elo_device(char *value) {        
    if (value) {        
           if (dfb_config->elo_device)        
                D_FREE( dfb_config->elo_device );        
        
    dfb_config->elo_device = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No Elo device specified!\n" );        
                   
    }        
}        
         
void FUN___penmount_device(char *value) {        
    if (value) {        
           if (dfb_config->penmount_device)        
                D_FREE( dfb_config->penmount_device );        
        
    dfb_config->penmount_device = D_STRDUP( value );        
    }        
    else {        
           printf( "DirectFB/Config: No PenMount device specified!\n" );        
                   
    }        
}        
         
void FUN___linux_input_devices(char *value) {        
    if (value) {        
           config_values_free( &dfb_config->linux_input_devices );        
           config_values_parse( &dfb_config->linux_input_devices, value );        
    }        
    else {        
           printf( "DirectFB/Config: Missing value for linux-input-devices!\n" );        
                   
    }        
}        
         
void FUN___tslib_devices(char *value) {        
    if (value) {        
           config_values_free( &dfb_config->tslib_devices );        
           config_values_parse( &dfb_config->tslib_devices, value );        
    }        
    else {        
           printf( "DirectFB/Config: Missing value for tslib-devices!\n" );        
                   
    }        
}        
         
void FUN___unichrome_revision(char *value) {        
    if (value) {        
           int rev;        
        
           if (sscanf( value, "%d", &rev ) < 1){        
                printf("DirectFB/Config 'unichrome-revision': Could not parse revision!\n");        
                        
         }        
        
    dfb_config->unichrome_revision = rev;        
    }        
    else {        
           printf("DirectFB/Config 'unichrome-revision': No revision specified!\n");        
                   
    }        
}        
         
void FUN___i8xx_overlay_pipe_b(char *value) {        
    dfb_config->i8xx_overlay_pipe_b = true;        
}        
         
void FUN___include(char *value) {        
      if( value ){        
           DFBResult ret;        
           ret = dfb_config_read( value );        

    }        
    else {        
           printf("DirectFB/Config 'include': No include file specified!\n");        
                   
    }        
}        
         
void FUN___mst_gfx_width(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_width = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mst_gfx_height(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_height = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mst_lcd_width(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_lcd_width = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_lcd_height(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_lcd_height = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_ge_vq_phys_addr(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_ge_vq_phys_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_ge_vq_phys_len(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_ge_vq_phys_len = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gop_regdma_phys_addr(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gop_regdmaphys_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gop_regdma_phys_len(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gop_regdmaphys_len = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_hal_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_hal_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_hal_length(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_hal_length = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_cpu_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_cpu_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_hal_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu1_hal_offset = val;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_hal_length(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu1_hal_length = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_cpu_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
    dfb_config->mst_miu1_cpu_offset = val;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu2_hal_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu2_hal_offset = val;        
         update_videomem_addr();
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu2_hal_length(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu2_hal_length = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu2_cpu_offset(char *value) {        
        
    if (value) {        
           char *error;        
           u64 val = 0;
           val = strtoull( value, &error, 16 );
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
    dfb_config->mst_miu2_cpu_offset = val;        
         update_videomem_addr();
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gfx_h_offset(char *value) {        
        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_h_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gfx_v_offset(char *value) {        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_v_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gfx_gop_index(char *value) {        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_gop_index = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
        
}        
        
void FUN___default_layer_opacity(char *value) {        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->default_layer_opacity = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gop_counts(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                     
      }        
      dfb_config->mst_gop_counts = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available0(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[0] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available1(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[1] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available2(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[2] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available3(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[3] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available4(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[4] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available5(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[5] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r0(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[0] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r1(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[1] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r2(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[2] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r3(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[3] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r4(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[4] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available_r5(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available_r[5] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane0(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[0] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane1(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[1] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane2(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[2] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane3(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[3] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane4(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[4] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane5(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[5] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_jpeg_readbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           u64 val = 0;
        
           val = strtoull( value, &error, 16);
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_readbuff_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_readbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_readbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_interbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           u64 val = 0;
        
           val = strtoull( value, &error, 16);
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_interbuff_addr= val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_interbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_interbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_outbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           u64 val = 0;
        
           val = strtoull( value, &error, 16);
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_outbuff_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}         
        
void FUN___mst_jpeg_outbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_outbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
        
void FUN___mst_jpeg_hwdecode(char *value) {        
    dfb_config->mst_jpeg_hwdecode = true;        
}         
        
void FUN___muxCounts(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_mux_counts= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux0_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[0]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux1_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[1]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux2_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[2]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux3_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[3]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux4_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[4]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux5_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[5]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayerCount(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer_counts = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer0_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[0]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer1_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[1]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer2_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[2]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer3_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[3]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer4_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[4]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___goplayer5_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_goplayer[5]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_miu_setting(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 16 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_miu_setting = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_disable_hwclip(char *value) {        
     dfb_config->mst_disable_hwclip = true;        
}        
        
void FUN___mst_reg_appm(char *value) {        
         dfb_config->mst_dfb_register_app_manager = true;
}        
        
void FUN___mst_enable_GOP_HMirror(char *value) {        
    dfb_config->mst_GOP_HMirror = 1;        
}        
void FUN___mst_disable_GOP_HMirror(char *value) {        
    dfb_config->mst_GOP_HMirror = 0;        
}

void FUN___mst_enable_GOP_VMirror(char *value) {        
    dfb_config->mst_GOP_VMirror = 1;        
}    

void FUN___mst_disable_GOP_VMirror(char *value) {        
    dfb_config->mst_GOP_VMirror = 0;        
}

void FUN___mst_osd_to_ve(char *value) {        
    dfb_config->mst_osd_to_ve = true;        
    if (value) {        
         //char *error;        
         unsigned long val = 0;        
        if(strcmp(value,"VOP_SEL_OSD_BLEND0")==0)        
        {        
            dfb_config->mst_xc_to_ve_mux = 0;        
        }        
        else if(strcmp(value,"VOP_SEL_OSD_BLEND1")==0)        
        {        
            dfb_config->mst_xc_to_ve_mux = 1;        
        }        
        else if(strcmp(value,"VOP_SEL_OSD_BLEND2")==0)        
        {        
             dfb_config->mst_xc_to_ve_mux = 2;        
        }        
        else if(strcmp(value,"VOP_SEL_MACE_RGB")==0)        
        {        
           dfb_config->mst_xc_to_ve_mux = 3;        
        }        
        else if(strcmp(value,"VOP_SEL_OSD_NONE")==0)        
        {        
            dfb_config->mst_xc_to_ve_mux = 4;        
        }
        else
        {
            char *error;        
            val = strtoul( value, &error, 16);

            if (*error)        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );            

            dfb_config->mst_xc_to_ve_mux = val;

        }        
     }        
     else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
     }        
        
}        
        
void FUN___keypad_callback(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong addr;        
        
           addr = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->keypadCallback = (keypad_func)addr;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
         
 //mstar patch start        
 /*patch for mantis: 709338: [065B][ISDB][add keypad operation in app then coredump]
   this patch is  used by sn0401 branch only. The branch uses a set of independent keypad module without         
   use  input module  provided by DFB        
*/        
        
void FUN___keypad_callback2(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong addr;        
        
           addr = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->keypadCallback2 = (keypad_func2)addr;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
 //mstar patch end        
         
        
void FUN___keypad_internal(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong internal;        
        
           internal = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->keyinternal= internal;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___mst_GOP_Set_YUV(char *value) {        
     dfb_config->mst_GOP_Set_YUV = true;        
}        
        
void FUN___stretchdown_patch(char *value) {        
     dfb_config->do_stretchdown_patch = true;        
}        
        
void FUN___line_stretchblit_patch(char *value) {        
     dfb_config->line_stretchblit_patch = true;        
}        
        
void FUN___stretchdown_enhance(char *value) {        
    if (value)        
    {        
        if (strcmp( value, "enable" ) == 0)            
            dfb_config->do_stretchdown_patch = true;        
        else if(strcmp( value, "disable" ) == 0)            
            dfb_config->do_stretchdown_patch = false;            
        else        
        {        
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
        }        
    }        
    else        
    {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
        
void FUN___static_stretchdown_buf(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong static_buf;        
        
           static_buf = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->static_stretchdown_buf = static_buf;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___mst_png_hwdecode(char *value) {        
    dfb_config->mst_png_hwdecode = true;        
}        
        
void FUN___mst_png_disable_hwdecode(char *value) {        
    dfb_config->mst_png_hwdecode = false;        
}        
        
void FUN___tvos_mode(char *value) {        
    dfb_config->tvos_mode = true;        
}        
        
void FUN___devmem_dump(char *value) {        
    dfb_config->enable_devmem_dump = true;        
}        
        
void FUN___dfb_using_forcewriteEnable(char *value) {        
    dfb_config->mst_forcewrite_DFB = DFB_FORECWRITE_USING|DFB_FORCEWRITE_ENABLE;        
        
}        
        
void FUN___dfb_using_forcewriteDisable(char *value) {        
        
   dfb_config->mst_forcewrite_DFB = DFB_FORECWRITE_USING|DFB_FORCEWRITE_DISABLE;        
}        
        
void FUN___dfb_ignore_forcewrite(char *value) {        
        
   dfb_config->mst_forcewrite_DFB = dfb_config->mst_forcewrite_DFB &~(DFB_FORECWRITE_USING|DFB_FORCEWRITE_DISABLE|DFB_FORCEWRITE_ENABLE);        
}        
        
void FUN___yuvtorgb_patch(char *value) {        
     dfb_config->do_yuvtorgb_sw_patch = true;        
}        
        
void FUN___enable_sw_jpeg(char *value) {        
      if (value)        
     {        
           if (strcmp( value, "enable" ) == 0)        
          {        
       dfb_config->mst_jpeg_hwdecode_option = false;        
         }        
                  
    else if(strcmp( value, "disable" ) == 0)        
          {        
       dfb_config->mst_jpeg_hwdecode_option = true;        
         }        
           else        
          {        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
         }        
    }        
      else        
     {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
        
void FUN___enable_dither(char *value) {

    dfb_config->mst_dither_enable = true;
    
    if (value)        
    {        
        if (strcmp( value, "enable" ) == 0)        
        {        
            dfb_config->mst_dither_enable = true;        
        }        
        
        else if(strcmp( value, "disable" ) == 0)        
        {        
            dfb_config->mst_dither_enable = false;        
        }        
        else        
        {        
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
        }        
    }        
    
}        
        
void FUN___mst_ir_max_keycode(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong ir_max_keycode;        
        
           ir_max_keycode = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_ir_max_keycode = ir_max_keycode;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___gop_using_tlb(char *value) {        
     dfb_config->bGOPUsingHWTLB = true;        
}        
        
void FUN___prealloc_map_tlb(char *value) {        
      if (value)        
     {        
           if (strcmp( value, "enable" ) == 0)        
          {        
       dfb_config->bPrealloc_map_tlb = true;        
         }        
                  
    else if(strcmp( value, "disable" ) == 0)        
          {        
       dfb_config->bPrealloc_map_tlb = false;        
         }        
           else        
          {        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
         }        
    }        
      else        
     {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
        
void FUN___tlb_alignment(char *value) {        
    if(value)        
   {        
        unsigned long val;        
        char *error;        
        val = strtoul( value, &error, 10);        
        if(*error)        
       {        
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->TLBAlignmentSize = val;        
  }        
    else        
   {         
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___mboot_gop_index(char *value) {        
    if(value)        
   {        
        unsigned long val;        
        char *error;        
        val = strtoul( value, &error, 10);        
        if(*error)        
       {        
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mbootGOPIndex = val;        
  }        
    else        
   {         
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___mst_enable_gevq(char *value) {
    if (value)
    {
        if (strcmp( value, "yes" ) == 0)
        {
            dfb_config->mst_enable_gevq = true;
        }
        else if (strcmp( value, "no" ) == 0)
        {
            dfb_config->mst_enable_gevq = false;
        }
        else
        {
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );
            return ;
        }
    }
    else
    {
        dfb_config->mst_enable_gevq = true;
    }
}        
        
void FUN___mst_disable_layer_init(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_disable_layer_init = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___debug_layer(char *value) {         
    dfb_config->mst_debug_layer = true;        
}        
void FUN___debug_surface(char *value) {         
    dfb_config->mst_debug_surface = true;        
}        
void FUN___debug_input(char *value) {

    if (value && strcmp( value, "tmpfile" ) == 0)
        dfb_config->mst_debug_input = DFB_DFB_LEVEL_TEMP_FILE;
    else
        dfb_config->mst_debug_input = DFB_DBG_LEVEL_NORMAL;

}        
void FUN___window_double_buffer(char *value) {         
    dfb_config->window_double_buffer= true;        
}        
void FUN___debug_ion(char *value) {             
    dfb_config->mst_debug_ion = true;        
}        
void FUN___ion_heapmask_by_layer0(char *value) {         

    if (value) {
       if (strcmp( value, "system" ) == 0) {
           dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_SYSTEM_MASK;
       } 
       else if (strcmp( value, "miu0" ) == 0) {
           dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU0_MASK;
       } 
       else if (strcmp( value, "miu1" ) == 0) {
           dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU1_MASK;
       } 
       else if (strcmp( value, "miu2" ) == 0) {
           dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU2_MASK;

       }
       else {
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );
       }
    }
    else {
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );
    }



}         
        
void FUN___ion_heapmask_by_layer1(char *value) {         
    if (value){        
       if (strcmp( value, "system" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_SYSTEM_MASK;        
    }        
              
    else if(strcmp( value, "miu0" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU0_MASK;        
    }        
              
    else if(strcmp( value, "miu1" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU1_MASK;        
    }        
              
    else if(strcmp( value, "miu2" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU2_MASK;        
        
     }        
     else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
     }        
  }        
    else{        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
  }        
}        
        
void FUN___ion_heapmask_by_layer2(char *value) {         
    if (value) {        
        if (strcmp( value, "system" ) == 0)        
          dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_SYSTEM_MASK;        
            else if(strcmp( value, "miu0" ) == 0)        
          dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU0_MASK;        
            else if(strcmp( value, "miu1" ) == 0)        
          dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU1_MASK;        
            else if(strcmp( value, "miu2" ) == 0)        
          dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU2_MASK;        
            
        else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
        }        
    }        
    else {        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
    }        
}         
        
void FUN___ion_heapmask_by_layer3(char *value) {         
    if (value){        
       if (strcmp( value, "system" ) == 0) {    
    dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_SYSTEM_MASK;        
}        
              
    else if(strcmp( value, "miu0" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU0_MASK;        
}        
              
    else if(strcmp( value, "miu1" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU1_MASK;        
}        
              
    else if(strcmp( value, "miu2" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU2_MASK;        
        
     }        
     else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
     }        
  }        
    else{        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
  }        
}         
        
void FUN___ion_heapmask_by_layer4(char *value) {         
    if (value){        
       if (strcmp( value, "system" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_SYSTEM_MASK;        
    }        
              
    else if(strcmp( value, "miu0" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU0_MASK;        
    }        
              
    else if(strcmp( value, "miu1" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU1_MASK;        
    }        
              
    else if(strcmp( value, "miu2" ) == 0) {        
    dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU2_MASK;        
        
     }        
     else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
     }        
  }        
    else{        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
  }        
}         
        
void FUN___ion_heapmask_by_layer5(char *value) {         
    if (value){        
       if (strcmp( value, "system" ) == 0)        
    dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_SYSTEM_MASK;        
        
              
    else if(strcmp( value, "miu0" ) == 0)        
    dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU0_MASK;        
        
              
    else if(strcmp( value, "miu1" ) == 0)        
    dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU1_MASK;        
        
              
    else if(strcmp( value, "miu2" ) == 0)        
    dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU2_MASK;        
        
             
     else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
     }        
  }        
    else{        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
  }        
}         
        
void FUN___ion_heapmask_by_surface(char *value) {         
    if (value){        
       if (strcmp( value, "system" ) == 0)        
    dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_SYSTEM_MASK;        
        
              
    else if(strcmp( value, "miu0" ) == 0)        
    dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU0_MASK;        
        
              
    else if(strcmp( value, "miu1" ) == 0)        
    dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU1_MASK;        
        
              
    else if(strcmp( value, "miu2" ) == 0)        
    dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU2_MASK;        
        
             
     else {        
            printf( "DirectFB/Config '%s': Unknown heap mask '%s'!\n", __FUNCTION__, value );        
     }        
  }        
    else{        
       printf( "DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );        
  }        
}         
        
void FUN___mst_layer_default_width(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );            
         }        
        
    dfb_config->mst_layer_default_width = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);                
    }        
}         
        
void FUN___mst_layer_default_height(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );            
         }        
        
    dfb_config->mst_layer_default_height = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);            
    }        
}        

void FUN___full_update_num(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong numerator;        
        
           numerator = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );            
         }        
        
    dfb_config->full_update_numerator = numerator;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);            
  }        
}        
    
void FUN___full_update_den(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong denominator;        
        
           denominator = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
         }        
        
    dfb_config->full_update_denominator = denominator;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
  }        
}         
        
void FUN___surface_memory_type(char *value) {         
    if (value)        
   {        
           char *error;        
           u8 type;        
        
           type = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
        
         }        
        
    dfb_config->mst_surface_memory_type = type;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
  }        
}              
    
void FUN___disable_decode_small_jpeg_by_sw(char *value) {        
     dfb_config->mst_disable_decode_small_jpeg_by_sw = true;         
}        
        
void FUN___enable_GOP_Vmirror_Patch(char *value) {        
     dfb_config->mst_enable_GOP_Vmirror_Patch = true;         
}        
        
void FUN___disble_GOP_Vmirror_Patch(char *value) {        
     dfb_config->mst_enable_GOP_Vmirror_Patch = false;         
}        
        
void FUN___stretchdown_patch_ratio(char *value) {         
        
    if (value)        
   {        
           char *error;        
           u8 type;        
        
           type = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
         }        
        
    dfb_config->stretchdown_patch_ratio = type;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
  }        
}              
        
void FUN___null_driver(char *value) {        
     dfb_config->null_driver = true;        
}        
        
void FUN___sw_render(char *value) {        
      // enable sw_render flag.        
    dfb_config->sw_render = SWRF_ENABLE;        
        
      if (value)        
     {                  
          dfb_config->sw_render = atoi(value);    
           printf("[DFB] sw_render = %d\n", dfb_config->sw_render);        
    }        
}        
        
void FUN___enable_jpeg_quality(char *value) {        
      if (value)        
     {        
           if (strcmp( value, "enable" ) == 0)        
          {        
       dfb_config->mst_enable_jpeg_quality = true;        
         }        
                  
    else if(strcmp( value, "disable" ) == 0)        
          {        
       dfb_config->mst_enable_jpeg_quality = false;        
         }        
           else        
          {        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
    
         }        
    }        
      else        
     {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
    }        
}        
        
void FUN___mst_enable_dip(char *value) {        
      printf("[DFB] mst_enable_dip\n");        
      if (value)        
     {        
           if (strcmp( value, "enable" ) == 0)        
          {        
                 printf("[DFB] mst_enable_dip enable\n");        
       dfb_config->mst_enable_dip = true;        
         }        
                  
    else if(strcmp( value, "disable" ) == 0)        
          {        
       dfb_config->mst_enable_dip = false;        
         }        
           else        
          {        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );        
    
         }        
    }        
      else        
     {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
        
    }        
}            

void FUN___mst_dip_mload_addr(char *value) {        
    if (value) {        
           char *error;        
           u64 val = 0;
        
           val = strtoull( value, &error, 16);
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
         }        
        
    dfb_config->mst_dip_mload_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
        
    }        
}        
void FUN___mst_dip_mload_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_dip_mload_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
              
    }        
}           
void FUN___mst_dip_select(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_dip_select= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        
void FUN___mst_disable_master_pri_high(char *value) {        
dfb_config->mst_disable_master_pri_high = true;        
}         
        
void FUN___show_freeze_image(char *value) {        
     dfb_config->freeze_image_by_dfb = true;        
}        
        
void FUN___i8toargb4444_patch(char *value) {        
     dfb_config->do_i8toargb4444_sw_patch = true;        
}         
        
void FUN___mst_enable_ve_init(char *value) {        
     dfb_config->mst_enable_ve_init = true;        
}        
        
void FUN___mst_register_gpd(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
        
      }        
      dfb_config->mst_register_gpd = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        
        
void FUN___hw_jpg_limit_patch(char *value) {        
     dfb_config->do_hw_jpg_limit_patch = true;        
}         
        
void FUN___dump_backtrace(char *value) {

#if !defined(__UCLIBC__)
    registerSigalstack();
#endif  
}        
        
void FUN___test_ge(char *value) {        
     dfb_config->mst_GE_performanceTest = true;        
}        
        
void FUN___miu_protect(char *value) {        
    dfb_config->mst_MIU_protect = true;        
        
    if (value) {        
           int id;        
        
           if (sscanf( value, "%d", &id ) < 0){        
                printf("DirectFB/Config '%s': Could not parse id!\n", __FUNCTION__);        
    
         }        
        
           if (id < 0 || id > 3){        
                printf("DirectFB/Config '%s': ID %d out of bounds!\n", __FUNCTION__, id);        
        
         }        
        
    dfb_config->mst_MIU_protect_BlockID = id;        
        
    }        
        
}          
        
void FUN___mst_cma_heap_id(char *value) {        
     if (value) {        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_cma_heap_id = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        
        
void FUN___mst_sec_cma_heap_id(char *value) {        
     if (value) {        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_sec_cma_heap_id = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        
        
void FUN___mapmem_mode(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_mapmem_mode = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        
        
void FUN___mst_disable_dfbinfo(char *value) {        
     dfb_config->mst_disable_dfbinfo = true;        
}        
        
void FUN___src_color_key_index_patch(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->src_color_key_index_patch= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
        
   }        
}        
        
void FUN___mst_ir_repeat_time(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_ir_repeat_time= val;        
     printf("DFB Note: mst_ir_repeat_time be set to %d ms\n", dfb_config->mst_ir_repeat_time);        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        

void FUN___mst_keypad_repeat_time(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_keypad_repeat_time= val;        
     printf("DFB Note: mst_keypad_repeat_time be set to %d ms\n", dfb_config->mst_keypad_repeat_time);        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }        
}        

void FUN___mst_ir_first_time_out(char *value) {
     // if not repeat, the time out is repeat_time + first_time_out.
     if (value){
         char *error;
         unsigned long val;
         val = strtoul( value, &error, 10 );

        if (*error){
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
        }
        dfb_config->mst_ir_first_time_out = val;
        printf("DFB Note: mst_ir_first_time_out be set to %d ms\n", dfb_config->mst_ir_first_time_out);
     }
     else{
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
     }
}

void FUN___mst_usbir_repeat_time(char *value) {
     if (value){
         char *error;
         unsigned long val;
         val = strtoul( value, &error, 10 );

        if (*error){
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

      }
      dfb_config->mst_usbir_repeat_time= val;
     printf("DFB Note: mst_usbir_repeat_time be set to %d ms\n", dfb_config->mst_usbir_repeat_time);
   }
     else{
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);

   }
}

void FUN___mst_handle_wm_key(char *value) {        
     dfb_config->mst_handle_wm_key = true;        
}        
        
void FUN___disable_cjk_string_break(char *value) {        
    dfb_config->disable_cjk_string_break = true;        
}
        
void FUN___disable_quick_press(char *value) {        
    dfb_config->disable_quick_press= true;        
}        
        
void FUN___mst_disable_window_scale_patch(char *value) {        
    dfb_config->mst_disable_window_scale_patch= true;        
}        
        
void FUN___mst_force_flip_wait_tagid(char *value) {        
    dfb_config->mst_force_flip_wait_tagid= true;        
}    

void FUN___enable_cursor_mouse_optimize(char *value) {
    dfb_config->enable_cursor_mouse_optimize = true;
}

void FUN___layer_support_palette(char *value) {
    dfb_config->layer_support_palette= true;
}

void FUN___stretchblit_with_rotate(char * value) {

    if (value) {
        char *error;
        unsigned long val;
        
        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }
        
        dfb_config->stretchblit_with_rotate= val;
    }
    else {
       D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    } 
}

void FUN___mst_layer_gwin_level(char *value) {

    if (value) {
        char *error;
        int val;
        
        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_layer_gwin_level= val;
    }
    else {
       D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_enable_gop_gwin_partial_update(char *value) {
    D_INFO("[DFB] mst_enable_gop_gwin_partial_update!\n");
    dfb_config->mst_enable_gop_gwin_partial_update = true;
}

void FUN___mst_enable_new_alphamode(char *value)
{
    if (value) {
             char *error;
             unsigned int val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 return ;
            }
            dfb_config->mst_new_alphamode = val;
     }
}

void FUN___mst_new_alphamode_on_layerid(char *value)
{
    if (value) {
             char *error;
             unsigned int val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 return ;
            }
            dfb_config->mst_newalphamode_on_layer = (u8)val;
     }
}

void FUN___enable_layer_fullupdate0(char *value) {
    dfb_config->mst_layer_bfullupdate[0] = true;
}

void FUN___enable_layer_fullupdate1(char *value) {
    dfb_config->mst_layer_bfullupdate[1] = true;
}

void FUN___enable_layer_fullupdate2(char *value) {
    dfb_config->mst_layer_bfullupdate[2] = true;
}

void FUN___enable_layer_fullupdate3(char *value) {
    dfb_config->mst_layer_bfullupdate[3] = true;
}

void FUN___enable_layer_fullupdate4(char *value) {
    dfb_config->mst_layer_bfullupdate[4] = true;
}

void FUN___enable_layer_fullupdate5(char *value) {
    dfb_config->mst_layer_bfullupdate[5] = true;
}

void FUN___mst_AFBC_enable(char *value){

    dfb_config->mst_AFBC_layer_enable = 0x3f; // Enable AFBC from layer 0 ~ layer 5

    if (value) {
           char *error;
           ulong val;

           val = strtoul( value, &error, 16);

           if (*error){
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return;
           }

           dfb_config->mst_AFBC_layer_enable = val;
    }
    D_INFO("mst_AFBC_layer_enable = 0x%08x \n", dfb_config->mst_AFBC_layer_enable);
}

void FUN___mst_AFBC_mode(char *value){

    if (value) {
        char *error;
        ulong val;

        val = strtoul( value, &error, 16);

        if (*error){
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
             return;
        }

        dfb_config->mst_AFBC_mode = val;
    }
    D_INFO("mst_AFBC_mode = 0x%08x \n", dfb_config->mst_AFBC_mode);
}

void FUN___mst_enhance_stretchblit_precision(char *value)
{
    D_INFO("[DFB] mst_enhance_stretchblit_precision = true\n");
    dfb_config->mst_enhance_stretchblit_precision = true;
}

void FUN___mst_gop_miu2_setting_extend(char *value) {
     if (value){
         char *error;
         unsigned long val;
         val = strtoul( value, &error, 16 );

        if (*error){
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
      }
      dfb_config->mst_gop_miu2_setting_extend = val;
   }
     else{
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
   }
}

void FUN__mst_enable_GLES2(char *value)
{
    D_INFO("[DFB] mst_enable_GLES2 = true\n");
    dfb_config->mst_enable_GLES2 = true;
}

void FUN___mst_argb1555_display(char *value)
{
    D_INFO("[DFB] mst_argb1555_display = true\n");
    dfb_config->mst_argb1555_display=true;
}

void FUN___mst_font_dsblit_src_premultiply(char *value)
{
    D_INFO("[DFB] mst_font_dsblit_src_premultiply = true\n");
    dfb_config->mst_font_dsblit_src_premultiply = true;
}

void FUN___mst_mem_peak_usage(char *value) {

    if (value) {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_mem_peak_usage = val;
    }
    else {
       D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_enable_gwin_multialpha(char *value)
{
    D_INFO("[DFB] mst_enable_gwin_multialpha = true\n");
    dfb_config->mst_enable_gwin_multialpha = true;
}


void FUN___mst_measure_png_performance(char *value)
{
    D_INFO("[DFB] mst_measure_png_performance = true\n");
    dfb_config->mst_measure_png_performance = true;
}

void FUN___mst_rgb2yuv_mode(char *value)
{
    if (value)
    {
        char *error;
        unsigned int val;
        val = strtoul( value, &error, 10 );

        if (*error)
            return ;

        dfb_config->mst_rgb2yuv_mode = val;
    }
}

void FUN___debug_cma(char *value)
{
    dfb_config->mst_debug_cma = true;
}

void FUN___mst_margine_left(char *value) {
    if (value)
    {
        char *error;
        unsigned long val;
        val = strtoul( value, &error, 16 );

        if (*error)
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

        dfb_config->mst_margine_left= val;
    }
    else
    {
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
    }
}


void FUN___mst_margine_wright(char *value)
{
    if (value)
    {
        char *error;
        unsigned long val;
        val = strtoul( value, &error, 16 );

        if (*error)
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

        dfb_config->mst_margine_wright= val;
    }
    else
    {
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
    }
}

void FUN___mst_margine_top(char *value)
{
    if (value)
    {
        char *error;
        unsigned long val;
        val = strtoul( value, &error, 16 );

        if (*error)
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

        dfb_config->mst_margine_top= val;
    }
    else{
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
    }
}

void FUN___mst_margine_bottom(char *value)
{
    if (value)
    {
        char *error;
        unsigned long val;
        val = strtoul( value, &error, 16 );

        if (*error)
        {
            printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
        }
        dfb_config->mst_margine_bottom= val;
    }
    else
    {
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);
    }
}

void FUN___mst_modify_symbol_by_keymap(char *value)
{
    dfb_config->mst_modify_symbol_by_keymap = true;
}
void FUN___mst_disable_modify_symbol_by_keymap(char *value)
{
    dfb_config->mst_modify_symbol_by_keymap = false;
}

void FUN___mst_memory_use_cma(char *value)
{
    dfb_config->mst_memory_use_cma = true;
}

void FUN___mst_gfxdriver(char *value)
{
    if (value)
    {
         if (dfb_config->mst_gfxdriver)
         {
               D_FREE( dfb_config->mst_gfxdriver );
               dfb_config->mst_gfxdriver = NULL;
         }

         if (strcmp(value, "auto") != 0)
             dfb_config->mst_gfxdriver = D_STRDUP( value );
    }
}

void FUN___mst_font_use_video_mem(char *value)
{
    dfb_config->mst_font_use_video_mem = true;
}

void FUN___dfbinfo_dir(char *value)
{
    if (value)
    {
        if (dfb_config->dfbinfo_dir)
            D_FREE( dfb_config->dfbinfo_dir );
        dfb_config->dfbinfo_dir = D_STRDUP( value );
    }
    else
    {
        printf("DirectFB/Config 'dfbinfo-dir': No directory name specified!\n");
    }
}

void FUN___disable_bus_address_check(char *value)
{
    dfb_config->bus_address_check = false;
}

void FUN___mst_layer_flip_blit(char *value)
{
    dfb_config->mst_layer_flip_blit = true;
}

void FUN___window_single_buffer(char *value) {
    dfb_config->window_single_buffer= true;
}

void FUN___mst_new_ir(char *value) {
    dfb_config->mst_new_ir = true;
}

void FUN___mst_new_ir_repeat_time(char *value) {
        if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_new_ir_repeat_time= val;        
     printf("DFB Note: mst_new_ir_repeat_time be set to %d ms\n", dfb_config->mst_new_ir_repeat_time);        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }
}

void FUN___mst_new_ir_first_repeat_time(char *value) {
        if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
    
      }        
      dfb_config->mst_new_ir_first_repeat_time= val;        
     printf("DFB Note: mst_new_ir_first_repeat_time be set to %d ms\n", dfb_config->mst_new_ir_first_repeat_time);        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
   }
}

void FUN___mst_t_stretch_mode( char *value)
{
    if(value)
    {
        // linear mode
        if (strcmp( value, "DUPLICATE" ) == 0)
            dfb_config->mst_t_stretch_mode = DISPLAYER_TRANSPCOLOR_STRCH_DUPLICATE;

        //  duplicate mode
        else if(strcmp( value, "ASNORMAL" ) == 0)
            dfb_config->mst_t_stretch_mode = DISPLAYER_TRANSPCOLOR_STRCH_ASNORMAL;

        else
        {
            if (value)
            {
                char *error;
                unsigned long val;
                val = strtoul( value, &error, 16 );

                if (*error)
                {
                    printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                }

                dfb_config->mst_t_stretch_mode = val;
            }
        }


    }
    else
        printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

}

void FUN___mst_h_stretch_mode( char *value)
{
    if(value)
    {
        // 6-tape (including nearest) mode
        if (strcmp( value, "6TAPE" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE;

        // duplicate mode.
        else if(strcmp( value, "DUPLICATE" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_DUPLICATE;

        // 6-tape (Linear mode)
        else if(strcmp( value, "6TAPE_LINEAR" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_LINEAR;

        // 6-tape (Nearest mode)
        else if(strcmp( value, "6TAPE_NEAREST" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_NEAREST;

        // 6-tape (Gain0)
        else if(strcmp( value, "6TAPE_GAIN0" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN0;

        // 6-tape (Gain1)
        else if(strcmp( value, "6TAPE_GAIN1" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN1;

        // 6-tape (Gain2)
        else if(strcmp( value, "6TAPE_GAIN2" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN2;

        // 6-tape (Gain3)
        else if(strcmp( value, "6TAPE_GAIN3" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN3;

        // 6-tape (Gain4)
        else if(strcmp( value, "6TAPE_GAIN4" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN4;

        // 6-tape (Gain5)
        else if(strcmp( value, "6TAPE_GAIN5" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_6TAPE_GAIN5;

        // 4-tap filer
        else if(strcmp( value, "4TAPE" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_4TAPE;

        // 2-tape
        else if(strcmp( value, "2TAPE" ) == 0)
            dfb_config->mst_h_stretch_mode = DISPLAYER_HSTRCH_2TAPE;

        else
        {
            if (value)
            {
                char *error;
                unsigned long val;
                val = strtoul( value, &error, 16 );

                if (*error)
                    printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

                dfb_config->mst_h_stretch_mode = val;
            }
        }

    }
    else
        printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

}


void FUN___mst_v_stretch_mode( char *value)
{
    if(value)
    {
        // linear mode
        if (strcmp( value, "LINEAR" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR;

        //  duplicate mode
        else if(strcmp( value, "DUPLICATE" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_DUPLICATE;

        //  nearest mode
        else if(strcmp( value, "NEAREST" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_NEAREST;

        //  Linear GAIN0 mode
        else if(strcmp( value, "LINEAR_GAIN0" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN0;

        //  Linear GAIN1 mode
        else if(strcmp( value, "LINEAR_GAIN1" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN1;

        //  Linear GAIN2 mode
        else if(strcmp( value, "LINEAR_GAIN2" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_LINEAR_GAIN2;

        //  4-TAPE mode
        else if(strcmp( value, "4TAPE" ) == 0)
            dfb_config->mst_v_stretch_mode = DISPLAYER_VSTRCH_4TAPE;
        else
        {
            if (value)
            {
                char *error;
                unsigned long val;
                val = strtoul( value, &error, 16 );

                if (*error)
                {
                    printf( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                }

                dfb_config->mst_v_stretch_mode = val;
            }
        }
    }
    else
        printf( "[DFB] DirectFB/Config '%s': No heap mask specified!\n", __FUNCTION__ );

}

void FUN___mst_cursor_swap_mode(char *value)
{
    dfb_config->mst_cursor_swap_mode = true;

    D_INFO("[DFB] mst_cursor_swap_mode enabled!\n");
}

void FUN___mst_inputevent_layer(char *value)
{
     if (value) {
         char *error;
         int val;

         val = strtoul( value, &error, 10 );

         if (*error) {
             D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
             return;
         }

         dfb_config->mst_inputevent_layer = val;
         D_INFO("[DFB] %s : input event listen on layer %d\n", __FUNCTION__, dfb_config->mst_inputevent_layer);
     }
     else {
         D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
         return;
     }
}

void FUN___mst_blink_frame_rate(char *value)
{
    if (value)
    {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error)
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_blink_frame_rate = val;
    }
    else
    {
       D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_flip_dump_path(char *value)
{
    if (dfb_config->mst_flip_dump_path)
        D_FREE( dfb_config->mst_flip_dump_path );

    if (value)
    {
        dfb_config->mst_flip_dump_path = D_STRDUP( value );
        D_INFO("[DFB] mst_flip_dump_path =%s\n", dfb_config->mst_flip_dump_path);
    }
    else
    {
        dfb_config->mst_flip_dump_path = D_STRDUP( "measure" );
        D_INFO("[DFB] mst_flip_dump_path measure performance\n");
    }
}

void FUN___mst_flip_dump_type(char *value)
{
    if (dfb_config->mst_flip_dump_type)
            D_FREE( dfb_config->mst_flip_dump_type );

    if (value)
    {
        dfb_config->mst_flip_dump_type = D_STRDUP( value );
        D_INFO("[DFB] mst_flip_dump_type = %s\n", dfb_config->mst_flip_dump_type);
    }
    else
    {
        dfb_config->mst_flip_dump_type = D_STRDUP( "all" );
        D_INFO("[DFB] mst_flip_dump_type = %s\n", dfb_config->mst_flip_dump_type);
    }
}

void FUN___mst_flip_dump_area(char *value)
{
    if (value)
    {
        int arg[ARG_SIZE] = {0};
        char * token = strtok(value, ",");
        int i = 0;
        // loop through the string to extract all other tokens
        while(token != NULL)
        {
            arg[i] = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }

        if(arg[DUMP_X] > 0)
            dfb_config->mst_flip_dump_area.x = arg[DUMP_X];
        if(arg[DUMP_Y] > 0)
            dfb_config->mst_flip_dump_area.y = arg[DUMP_Y];
        if(arg[DUMP_W] > 0)
            dfb_config->mst_flip_dump_area.w = arg[DUMP_W];
        if(arg[DUMP_H] > 0)
            dfb_config->mst_flip_dump_area.h = arg[DUMP_H];

        D_INFO("[DFB] mst_flip_dump_area = (%d, %d, %d, %d)\n",
                              dfb_config->mst_flip_dump_area.x,
                              dfb_config->mst_flip_dump_area.y,
                              dfb_config->mst_flip_dump_area.w,
                              dfb_config->mst_flip_dump_area.h);
    }
    else
    {
        dfb_config->mst_flip_dump_area.x = 0;
        dfb_config->mst_flip_dump_area.y = 0;
        dfb_config->mst_flip_dump_area.w = 0;
        dfb_config->mst_flip_dump_area.h = 0;

        D_INFO("[DFB] mst_flip_dump_area = (%d, %d, %d, %d)\n",
                              dfb_config->mst_flip_dump_area.x,
                              dfb_config->mst_flip_dump_area.y,
                              dfb_config->mst_flip_dump_area.w,
                              dfb_config->mst_flip_dump_area.h);
    }
}
void FUN___mst_debug_cursor( char *value)
{
    dfb_config->mst_debug_cursor = true;
}


void FUN___mst_cursor_gwin_width(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, DEC );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_cursor_gwin_width = val;
    }
}

void FUN___mst_cursor_gwin_height(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, DEC );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_cursor_gwin_height = val;
    }
}

void FUN___mst_hw_cursor(char *value)
{
    if (value) {
            char *error = NULL;
            int val = strtoul( value, &error, DEC );

            if (*error) {
                D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
                return DFB_INVARG;
            }
            if(val >= 0)
                dfb_config->mst_hw_cursor = val;
    }
}

void FUN___mst_fix_string_break(char *value)
{
    dfb_config->mst_fix_string_break = true;
}
void FUN___mst_gles2_sdr2hdr(char *value)
{
    dfb_config->mst_enable_GLES2 = true;
    dfb_config->mst_gles2_sdr2hdr = true;
}
void FUN___mst_ge_hw_limit(char *value)
{
    if (value)
    {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error)
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_ge_hw_limit = val;
    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
        return ;
    }
}

void FUN___mst_cmdq_phys_addr(char * value)
{
    if (value)
    {
        char *error;
        unsigned long val;

        val = strtoul( value, &error, 16 );

        if (*error)
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_cmdq_phys_addr = val;
    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
        return ;
    }
}
void FUN___mst_cmdq_phys_len(char * value)
{
    if (value)
    {
        char *error;
        unsigned long val;

        val = strtoul( value, &error, 16 );

        if (*error)
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_cmdq_phys_len = val;
    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
        return ;
    }
}
void FUN___mst_cmdq_miusel(char * value)
{
    if (value)
    {
        char *error;
        unsigned long val;

        val = strtoul( value, &error, 16);

        if (*error)
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_cmdq_miusel = val;
    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
        return ;
    }
}

void FUN___mst_gop_interlace_adjust(char *value)
{
    dfb_config->mst_gop_interlace_adjust = true;
}

void FUN___mst_CFD_monitor_path(char *value)
{
    if (value) {
        if (dfb_config->mst_CFD_monitor_path)
            D_FREE( dfb_config->mst_CFD_monitor_path );
        dfb_config->mst_CFD_monitor_path = D_STRDUP( value );
    }
    else {
         D_ERROR("Direct/Config 'mst_CFD_monitor_path': No directory name specified!\n");
         return ;
    }
}

void FUN___mst_layer_buffer_mode(char *value)
{
    if (value)
    {
       if (strcmp( value, "triple" ) == 0)
            dfb_config->mst_layer_buffer_mode = DLBM_TRIPLE;

       else if  (strcmp( value, "backvideo" ) == 0)
            dfb_config->mst_layer_buffer_mode = DLBM_BACKVIDEO;

       else if (strcmp( value, "backsystem" ) == 0)
            dfb_config->mst_layer_buffer_mode = DLBM_BACKSYSTEM;

       else if (strcmp( value, "frontonly" ) == 0)
            dfb_config->mst_layer_buffer_mode = DLBM_FRONTONLY;

       else if (strcmp( value, "windows" ) == 0)
            dfb_config->mst_layer_buffer_mode = DLBM_WINDOWS;

       else
            printf( "DirectFB/Config '%s': Unknown mode '%s'!\n", __FUNCTION__, value );
    }
    else
    {
       printf( "DirectFB/Config '%s': No buffer mode specified!\n", __FUNCTION__ );
    }
}

void FUN___mst_layer_pixelformat(char *value)
{
    if (value)
    {
        dfb_config->mst_layer_pixelformat = dfb_config_parse_pixelformat( value );

        if ( dfb_config->mst_layer_pixelformat == DSPF_UNKNOWN)
            printf("DirectFB/Config '%s': Could not parse format!\n", __FUNCTION__);
    }
    else
    {
        printf("DirectFB/Config '%s': No format specified!\n", __FUNCTION__);
    }
}

void FUN___mst_do_xc_ip1_patch(char *value)
{
    if (value)
    {
        if (strcmp( value, "enable" ) == 0)
        {
            printf("[DFB] mst_do_xc_ip1_patch enable\n");
            dfb_config->mst_do_xc_ip1_patch = true;
        }
        else if (strcmp( value, "disable" ) == 0)
        {
            dfb_config->mst_do_xc_ip1_patch = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, value );
            return ;
        }
    }
    else
    {
        D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
        return ;
    }
}


void FUN___mst_dump_jpeg_buffer(char *value)
{
    if (value)
    {
        if (dfb_config->mst_dump_jpeg_buffer)
            D_FREE( dfb_config->mst_dump_jpeg_buffer);

        dfb_config->mst_dump_jpeg_buffer = D_STRDUP( value );
    }
    else
    {
         D_ERROR("Direct/Config 'mst_dump_jpeg_buffer': No directory name specified!\n");
         return ;
    }
}

void FUN___mst_clip_stretchblit_width_high_nonzero_patch(char *value)
{
    if(value){
          if (strcmp( value, "disable" ) == 0)
          {
               D_INFO("[DFB] mst_clip_stretchblit_width_high_nonzero_patch disable\n");
               dfb_config->mst_clip_stretchblit_width_high_nonzero_patch = false;
          }
          else if (strcmp( value, "enable" ) == 0)
          {
               D_INFO("[DFB] mst_clip_stretchblit_width_high_nonzero_patch enable\n");
               dfb_config->mst_clip_stretchblit_width_high_nonzero_patch = true;
          }
          else{
               D_ERROR( "[DFB] DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
          }
    }
}

void FUN___mst_bank_force_write(char *value)
{
    if(value)
    {
          if (strcmp( value, "disable" ) == 0)
          {
               D_INFO("[DFB] force to not support bankwrite\n");

               dfb_config->mst_bank_force_write = false;
          }
          else if (strcmp( value, "enable" ) == 0)
          {
               D_INFO("[DFB] force to support bankwrite\n");

               dfb_config->mst_bank_force_write = true;
          }
          else
          {
               D_ERROR( "[DFB] DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
          }
    }
}

void FUN___mst_font_bold_enable(char *value)
{
    dfb_config->mst_font_bold_enable = true;
}

void FUN__mst_fixed_mem_leak_patch_enable(char *value)
{
    if(value)
    {
         if (strcmp( value, "true" ) == 0)
         {
              D_INFO( "[DFB] mst_fixed_mem_leak_patch_enable = true\n");
              dfb_config->mst_fixed_mem_leak_patch_enable = true;
         }
         else if (strcmp( value, "false" ) == 0)
         {
              D_INFO( "[DFB] mst_fixed_mem_leak_patch_enable = false\n");
              dfb_config->mst_fixed_mem_leak_patch_enable = false;
         }
         else
         {
              D_ERROR( "[DFB] DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
         }
    }
}

void FUN___mst_nice(char *value)
{
    int val = -20;

    if (value)
    {
        val = atoi(value);
    }

    nice(val);
}


void FUN___mst_show_fps(char *value)
{
    dfb_config->mst_show_fps = true;
}

void FUN___mst_enable_layer_autoscaledown(char *value)
{
    dfb_config->mst_enable_layer_autoscaledown = true;
}

void FUN___mst_use_dlopen_dlsym(char *value)
{
    dfb_config->mst_use_dlopen_dlsym = true;
}

void FUN___mst_mem_small_size(char *value) {

    if (value) {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_mem_small_size = val;
    }
    else {
       D_ERROR( "[DFB] DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_mem_medium_size(char *value) {

    if (value) {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_mem_medium_size = val;
    }
    else {
       D_ERROR( "[DFB] DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_forbid_fragment_merge_to_api_locked_surface( char *value)
{
    dfb_config->mst_forbid_fragment_merge_to_api_locked_surface = true;
}

void FUN___mst_null_display_driver( char *value)
{
    dfb_config->mst_null_display_driver = true;
}

void FUN___mst_call_setkeypadcfg_in_device( char *value)
{
    dfb_config->mst_call_setkeypadcfg_in_device = true;
}

void FUN___mst_call_setdfbrccfg_disable( char *value)
{
    dfb_config->mst_call_setdfbrccfg_disable = true;
}

void FUN___mst_gwin_disable( char *value)
{
    dfb_config->mst_gwin_disable = true;
}

void FUN___mst_using_mi_system( char *value)
{
    dfb_config->mst_using_mi_system = true;
}

void FUN___mst_force_wait_vsync( char *value)
{
    dfb_config->mst_force_wait_vsync = true;
}

void FUN___mst_new_mstar_linux_input( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_new_mstar_linux_input = true;
        }
        else if (strcmp( value, "false" ) == 0)
        {
            D_INFO( "[DFB] mst_new_mstar_linux_input = false\n");
            dfb_config->mst_new_mstar_linux_input = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_mma_pool_enable( char *value)
{
    dfb_config->mst_mma_pool_enable = true;
}

void FUN___mst_call_gop_t3d( char *value)
{
    if (value) {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_call_gop_t3d = val;
    }
    else {
       D_ERROR( "[DFB] DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_gopc_check_video_info( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_gopc_check_video_info = true;
        }
        else if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_gopc_check_video_info = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_preinit_fusion_world(char *value)
{
    dfb_config->mst_preinit_fusion_world = true;
}

void FUN___mst_font_outline_enable( char *value)
{
    dfb_config->mst_font_outline_enable = true;
}

void FUN___mst_bt601_formula(char *value)
{
    if (value) {
        char *error;
        int val;

        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return ;
        }

        dfb_config->mst_bt601_formula = val;
    }
    else {
       D_ERROR( "[DFB] DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return ;
    }
}

void FUN___mst_PTK_customized_input_driver_enable( char *value)
{
    dfb_config->mst_PTK_customized_input_driver_enable = true;
}
void FUN___mst_layer_fbdev0( char *value)
{
    if (value) {
        if (dfb_config->mst_layer_fbdev[0])
            D_FREE( dfb_config->mst_layer_fbdev[0] );
        dfb_config->mst_layer_fbdev[0] = D_STRDUP( value );
    }
    else {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[0]': No device name specified!\n");
    }
}

void FUN___mst_layer_fbdev1( char *value)
{
    if (value) {
        if (dfb_config->mst_layer_fbdev[1])
            D_FREE( dfb_config->mst_layer_fbdev[1] );
        dfb_config->mst_layer_fbdev[1] = D_STRDUP( value );
    }
    else {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[1]': No device name specified!\n");
    }
}

void FUN___mst_layer_fbdev2( char *value)
{
    if (value) {
        if (dfb_config->mst_layer_fbdev[2])
            D_FREE( dfb_config->mst_layer_fbdev[2] );
        dfb_config->mst_layer_fbdev[2] = D_STRDUP( value );
    }
    else {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[2]': No device name specified!\n");
    }
}

void FUN___mst_layer_fbdev3( char *value)
{
    if (value) {
        if (dfb_config->mst_layer_fbdev[3])
            D_FREE( dfb_config->mst_layer_fbdev[3] );
        dfb_config->mst_layer_fbdev[3] = D_STRDUP( value );
    }
    else {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[3]': No device name specified!\n");
    }
}

void FUN___mst_layer_fbdev4( char *value)
{
    if (value) {
        if (dfb_config->mst_layer_fbdev[4])
            D_FREE( dfb_config->mst_layer_fbdev[4] );
        dfb_config->mst_layer_fbdev[4] = D_STRDUP( value );
    }
    else {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[4]': No device name specified!\n");
    }
}

void FUN___mst_layer_fbdev5( char *value)
{
    if (value)
    {
        if (dfb_config->mst_layer_fbdev[5])
            D_FREE( dfb_config->mst_layer_fbdev[5] );
        dfb_config->mst_layer_fbdev[5] = D_STRDUP( value );
    }
    else
    {
        D_ERROR("DirectFB/Config 'mst_layer_fbdev[5]': No device name specified!\n");
    }
}

void FUN___mst_debug_mma( char *value)
{
    dfb_config->mst_debug_mma = true;
}

void FUN___mst_oom_retry(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_oom_retry = val;
    }
}

void FUN___mst_use_system_memory_threshold(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_use_system_memory_threshold = val;
    }
}

void FUN___mst_GOP_AutoDetectBuf( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_GOP_AutoDetectBuf = true;
        }
        else if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_GOP_AutoDetectBuf = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_call_disable_bootlogo( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_call_disable_bootlogo = true;
        }
        else if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_call_disable_bootlogo = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_GPU_window_compose( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_GPU_window_compose= true;
            dfb_config->mst_enable_GLES2 = true;
            direct_mutex_init( &dfb_config->GPU_compose_mutex );
            direct_waitqueue_init( &dfb_config->GPU_compose_cond );
        }
        else if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_GPU_window_compose = false;
            dfb_config->mst_enable_GLES2 = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_debug_secure_mode( char *value)
{
    dfb_config->mst_debug_secure_mode = true;
}


void FUN___mst_debug_gles2( char *value)
{
    dfb_config->mst_debug_gles2 = true;
}

void FUN___mst_layer_up_scaling( char *value)
{
    dfb_config->mst_enable_GLES2 = true;

    dfb_config->mst_layer_up_scaling = true;
    dfb_config->mst_layer_up_scaling_width = 3840;
    dfb_config->mst_layer_up_scaling_height = 2160;

    if (value)
    {
        if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_enable_GLES2 = false;

            dfb_config->mst_layer_up_scaling = false;
            dfb_config->mst_layer_up_scaling_width = 0;
            dfb_config->mst_layer_up_scaling_height = 0;
        }
    }
}

void FUN___mst_layer_up_scaling_id(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val >= 0)
            dfb_config->mst_layer_up_scaling_id = val;
    }
}

void FUN___mst_layer_up_scaling_width(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_layer_up_scaling_width = val;
    }
}

void FUN___mst_layer_up_scaling_height(char *value)
{
    if (value)
    {
        char *error = NULL;
        int val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return;
        }

        if(val > 0)
            dfb_config->mst_layer_up_scaling_height = val;
    }
}
void FUN___mst_debug_directory_access(char *value)
{
    if (value)
    {
        if (dfb_config->mst_debug_directory_access)
            D_FREE( dfb_config->mst_debug_directory_access);

        dfb_config->mst_debug_directory_access = D_STRDUP( value );
    }
    else
    {
         D_ERROR("Direct/Config 'mst_debug_directory_access': No directory name specified!\n");
         return ;
    }
}

void FUN___mst_debug_backtrace_dump( char *value)
{
    dfb_config->mst_debug_backtrace_dump = true;
}

void FUN___mst_debug_layer_setConfiguration_return( char *value)
{
    dfb_config->mst_debug_layer_setConfiguration_return = true;
}

void FUN___mst_debug_surface_clear_return( char *value)
{
    dfb_config->mst_debug_surface_clear_return = true;
}

void FUN___mst_debug_surface_fillrectangle_return( char *value)
{
    dfb_config->mst_debug_surface_fillrectangle_return = true;
}
/*
 *  Define Combo Config Setting from here.
*/
DFBResult FUN__mst_InputDevice(const char *name, const char *value)
{
    int i = 0;

    inputdevice_name_filter_by_bracket( &name );

    for (i =0; i < MAX_LINUX_INPUT_DEVICES; i++)
    {
        if ( (dfb_config->mst_inputdevice[i].name == NULL) && (dfb_config->mst_inputdevice[i].filename == NULL))
        {
            dfb_config->mst_inputdevice[i].name = D_STRDUP( name);
            dfb_config->mst_inputdevice[i].filename = D_STRDUP(value);

            break;
        }
        else
        {
            /*If duplicate device name has found, break and no set */
            if (strcmp(dfb_config->mst_inputdevice[i].name, name) ==0)
                break;
        }
    }
    return DFB_OK;

}


void FUN___mst_GPU_AFBC( char *value)
{
    if(value)
    {
        if (strcmp( value, "true" ) == 0)
        {
            dfb_config->mst_GPU_window_compose= true;
            dfb_config->mst_enable_GLES2 = true;
            dfb_config->mst_GPU_AFBC = true;
            dfb_config->GPU_compose_mutex = PTHREAD_MUTEX_INITIALIZER;
            dfb_config->GPU_compose_cond = PTHREAD_COND_INITIALIZER;
        }
        else if (strcmp( value, "false" ) == 0)
        {
            dfb_config->mst_GPU_window_compose = false;
            dfb_config->mst_enable_GLES2 = false;
            dfb_config->mst_GPU_AFBC = false;
        }
        else
        {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___GPU_AFBC_EXT_SIZE( char *value)
{
    if(value)
    {
        char *error = NULL;
#define BASE = 10;
        int val = strtoul( value, &error, BASE );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );             
        }

        if(val > 0)
            dfb_config->GPU_AFBC_EXT_SIZE = val;
        else {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_input_vendor_id( char *value)
{
    if(value)
    {
        char *error = NULL;
#define BASE = 10;
        int val = strtoul( value, &error, BASE );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );             
        }

        if(val > 0)
            dfb_config->mst_input_vendor_id = val;
        else {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_input_product_id( char *value)
{
    if(value)
    {
        char *error = NULL;
#define BASE = 10;
        int val = strtoul( value, &error, BASE );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );             
        }

        if(val > 0)
            dfb_config->mst_input_product_id = val;
        else {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

void FUN___mst_gpio_key_code( char *value)
{
    if(value)
    {
        char *error = NULL;
#define BASE = 10;
        int val = strtoul( value, &error, BASE );

        if (*error) {
            D_ERROR( "[DFB] DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
        }

        if(val > 0)
            dfb_config->mst_gpio_key_code = val;
        else {
            D_ERROR( "DirectFB/Config '%s': Error in value\n", __FUNCTION__ );
        }
    }
}

#endif


