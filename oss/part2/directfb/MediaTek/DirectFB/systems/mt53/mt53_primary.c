/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
#include <config.h>

#include <pthread.h>
#include <assert.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <asm/types.h>
#include <execinfo.h>

#include <directfb.h>

#include <direct/messages.h>
#include <direct/debug.h>

#include <core/palette.h>
#include <core/surface.h>
#include <core/layers_internal.h>

#include <gfx/convert.h>
#include <gfx_if.h>

#include "mt53.h"

D_DEBUG_DOMAIN( MT53_PRIMARY, "MT53/Primary", "MT53 primary layer" );

typedef enum
{
    OSD_BM_NONE,
    OSD_BM_PIXEL,
    OSD_BM_REGION,
    OSD_BM_PLANE
} OSD_BLEND_MODE_T;

void dfb_print_trace (void);


static void primary_set_region_mmu(GFX_MMU_T *pt_mmu)
{
    {
        pt_mmu->u4_init           = 0x1;
        pt_mmu->u4_enable         = 0x0;
        pt_mmu->u4_op_md          = 0x0;
        pt_mmu->u4_src_rw_mmu     = 0x0;
        pt_mmu->u4_dst_rw_mmu     = 0x0;
        pt_mmu->u4_vgfx_ord       = 0x0;
        pt_mmu->u4_vgfx_slva      = 0x0;
        pt_mmu->u4_pgt            = 0x0;
    }

    return;
}

static unsigned int _mapColorFormat(DFBSurfacePixelFormat format) 
{
    unsigned int mt_format = CM_Reserved0;
    switch(format)
    {
        case DSPF_UNKNOWN:
        case DSPF_ARGB1555:         
            break;
        case DSPF_RGB16:  
            mt_format = CM_RGB565_DIRECT16;
            break;          
        case DSPF_RGB24:
        case DSPF_RGB32:
            break;
        case DSPF_ARGB:
            mt_format = CM_ARGB8888_DIRECT32;
            break;
        case DSPF_A8:
        case DSPF_YUY2:
            mt_format = CM_YCbYCr422_DIRECT16;          
            break;          
        case DSPF_RGB332:
            break;
        case DSPF_UYVY:
            mt_format = CM_CbYCrY422_DIRECT16;          
            break;
        case DSPF_I420:
        case DSPF_YV12:
            break;     
        case DSPF_LUT8:
            mt_format = CM_RGB_CLUT8;           
            break;
        case DSPF_ALUT44:
        case DSPF_AiRGB:
        case DSPF_A1:
        case DSPF_NV12:
        case DSPF_NV16:
        case DSPF_ARGB2554:
            break;
        case DSPF_ARGB4444:
            mt_format = CM_ARGB4444_DIRECT16;
            break;
        case DSPF_NV21:
            break;
        case DSPF_AYUV:
            mt_format = CM_AYCbCr8888_DIRECT32;
            break;          
        case DSPF_A4:
        case DSPF_ARGB1666:
        case DSPF_ARGB6666:
        case DSPF_RGB18:
        case DSPF_LUT2:
        case DSPF_RGB444:
        case DSPF_RGB555:
            break;
      }
    return mt_format;
}

/**********************************************************************************************************************/

static DFBResult
primaryInitScreen( CoreScreen           *screen,
                   CoreGraphicsDevice   *device,
                   void                 *driver_data,
                   void                 *screen_data,
                   DFBScreenDescription *description )
{
     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf( description->name,
               DFB_SCREEN_DESC_NAME_LENGTH, "MediaTek 53xx Primary Screen" );

     return DFB_OK;
}

static DFBResult
primaryGetScreenSize( CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height )
{
    unsigned int data[2];
    struct mt53fb_get get;
    get.get_type = MT53FB_GET_PANEL_SIZE;
    get.get_size = 8;
    get.get_data = data;
    if (ioctl( dfb_mt53->fd, FBIO_GET, &get ) < 0) {
       D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] sizeof(mt53fb_get)=0x%x\n",  __func__, __LINE__, sizeof(get) );
       return DFB_IO;
    }

    D_DEBUG_AT(MT53_PRIMARY, "%s,%d: [Width: %d], [Height: %d]\n", __func__, __LINE__, data[0], data[1]);
    
    *ret_width  = data[0];
    *ret_height = data[1];
      
     return DFB_OK;
}

//#ifdef CC_HW_WINDOW_SUPPORT
DFBResult mdfb_wait_for_sync(void)
{
    DIRECT_INTERFACE_DBG_DELTA_START();
    
    if(ioctl( dfb_mt53->fd, FBIO_WAIT_VSYNC) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_SET MT53FB_SET_MASK_WAIT_VSYNC failed!\n" );
        return DFB_IO;
    }
    
    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PRMRY_WAIT);

    return DFB_OK;
}
//#endif

static DFBResult primaryWaitVSync( CoreScreen            *screen,
                                       void                  *driver_data,
                                       void                  *screen_data )
{
//#ifdef CC_HW_WINDOW_SUPPORT
    mdfb_wait_for_sync();
//#endif
    return DFB_OK;
}


DFBResult mdfb_check_wait_vsync(CoreLayer* pt_this)
{
    struct mt53fb_set set;
    MT53LayerData  *pt_layer_data = (MT53LayerData  *)pt_this->layer_data;
    DIRECT_INTERFACE_DBG_DELTA_START();
    
    memset(&set,0x0,sizeof(struct mt53fb_set));
    
    set.mask                    = MT53FB_SET_MASK_WAIT_VSYNC;
    set.rSetting.u4OsdPlaneID   = pt_layer_data->layer;
    
    if(ioctl( dfb_mt53->fd, FBIO_SET, &set ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_SET MT53FB_SET_MASK_WAIT_VSYNC failed!\n" );
        return DFB_IO;
    }
    
    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PRMRY_CHECK);

    return DFB_OK;     
}


DFBResult mdfb_check_fb_display(unsigned int u4Addr, unsigned int u4Size )
{
	struct mt53fb_imagebuffer rbuffer;

    memset(&rbuffer,0x0,sizeof(struct mt53fb_imagebuffer));

	rbuffer.u4Size = u4Size;
	rbuffer.u4PhyAddr = u4Addr;
    
    if(ioctl( dfb_mt53->fd, MT53FB_CHECK_DFB_DISPLAY_BUF, &rbuffer ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_SET mdfb_check_fb_display failed!\n" );
        return DFB_IO;
    }

    return DFB_OK;     
}

/**********************************************************************************************************************/

/*const*/ ScreenFuncs mt53PrimaryScreenFuncs = {
     InitScreen:    primaryInitScreen,
     GetScreenSize: primaryGetScreenSize,
     WaitVSync:     primaryWaitVSync
};

/**********************************************************************************************************************/
/**********************************************************************************************************************/

static int
primaryLayerDataSize()
{
     return sizeof(MT53LayerData);
}

static int
primaryRegionDataSize()
{
#ifdef CC_HW_WINDOW_SUPPORT
     return sizeof(MT53RegionData);
#else
     return 0;
#endif
}

static DFBResult
primaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{

     MT53DriverData *sdrv = driver_data;
     MT53LayerData  *data = layer_data;

    data->layer = (MT53LayerID)sdrv->num_plane;
    sdrv->num_plane++;

     /* set the layer surface accessor */
     description->surface_accessor = (CSAID_LAYER0 + data->layer) | CSAID_OSD;

     /* set capabilities and type */
#ifdef CC_HW_WINDOW_SUPPORT
      description->caps = DLCAPS_SURFACE | DLCAPS_SCREEN_POSITION | DLCAPS_SCREEN_SIZE | DLCAPS_LEVELS | DLCAPS_OPACITY | DLCAPS_ALPHACHANNEL | DLCAPS_WINDOWS;
#else
      description->caps = DLCAPS_SURFACE | DLCAPS_SCREEN_POSITION | DLCAPS_SCREEN_SIZE | DLCAPS_LEVELS | DLCAPS_OPACITY | DLCAPS_ALPHACHANNEL;
#endif
     description->type = DLTF_GRAPHICS;

     /* set name */
     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "MediaTek 53xx Primary Layer" );

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_OPTIONS;
     config->width       = MT53_PRIMARY_DEFAULT_WIDTH;
     config->height      = MT53_PRIMARY_DEFAULT_HEIGHT;
     config->pixelformat = DSPF_ARGB;
     config->buffermode  = DLBM_FRONTONLY;
     config->options     = DLOP_NONE;
     
     return DFB_OK;
}

static DFBResult
primaryTestRegion( CoreLayer                  *layer,
                   void                       *driver_data,
                   void                       *layer_data,
                   CoreLayerRegionConfig      *config,
                   CoreLayerRegionConfigFlags *failed )
{
     CoreLayerRegionConfigFlags fail = CLRCF_NONE;

     /* check options */
     if (config->options & ~MT53_PRIMARY_SUPPORTED_OPTIONS)
     {
		 D_WARN("@@@@@@ %s	 options [0x%x] [0x%x] \n", __FUNCTION__, config->options, MT53_PRIMARY_SUPPORTED_OPTIONS);
		 config->options &= MT53_PRIMARY_SUPPORTED_OPTIONS;
		 //fail |= CLRCF_OPTIONS;

     }
     
     /* check format */
     switch (config->format) {
          case DSPF_LUT8:
          case DSPF_ARGB4444:
          case DSPF_ARGB:
          case DSPF_RGB16:              
          case DSPF_AYUV:           
          case DSPF_YUY2:
          case DSPF_UYVY:
#ifdef CC_B2R44K2K_SUPPORT	  
          case DSPF_NV16:
#endif		  	
               break;

          default:
               fail |= CLRCF_FORMAT;
               break;
     }

#ifndef CC_B2R44K2K_SUPPORT
     /* check width */
     if (config->width > 1920 || config->width < 2)
          fail |= CLRCF_WIDTH;

     /* check height */
     if (config->height > 1080 || config->height < 2)
          fail |= CLRCF_HEIGHT;
#endif

     /* write back failing fields */
     if (failed)
          *failed = fail;

     /* return failure if any field failed */
     if (fail)
          return DFB_UNSUPPORTED;

     return DFB_OK;
}

static DFBResult
primarySetRegion( CoreLayer                  *layer,
                    void                       *driver_data,
                    void                       *layer_data,
                    void                       *region_data,
                    CoreLayerRegionConfig      *config,
                    CoreLayerRegionConfigFlags  updated,
                    CoreSurface                *surface,
                    CorePalette                *palette,
                    CoreSurfaceBufferLock      *left_lock, 
                    CoreSurfaceBufferLock      *right_lock 
                    )
{
    struct mt53fb_set set;

    MT53LayerData  *data = layer_data;
    CoreSurfaceBufferLock* lock=left_lock;
    assert(data->layer < MT53_MAX_OSD_PLANE);
#ifdef CC_HW_WINDOW_SUPPORT
    data->curregion = region_data;
#endif

    D_MAGIC_ASSERT(surface, CoreSurface);

    memset(&set,0x0,sizeof(struct mt53fb_set));

    if (updated & ~CLRCF_PALETTE) {
        set.rSetting.u4OsdPlaneID = data->layer;

        set.mask = MT53FB_SET_MASK_BASE | MT53FB_SET_MASK_VISIBLE;
		#ifdef CC_OSD_M4U_IOVA_SUPPORT
		set.rSetting.u4Base = system_gfx_switch_osd_iova(lock->phys);
		#else
        set.rSetting.u4Base = lock->phys;
		#endif
        set.rSetting.fgVisible = 1;
        set.rSetting.u4VirtAddr = (u32)lock->addr;

        if (surface->config.caps & DSCAPS_IOMMU)
        {
            set.mask                 |= MT53FB_SET_MASK_OSD_IOMMU;
            set.rSetting.u4OsdIommuEn = true;
            set.rSetting.u4_iova      = lock->iova;
        }
        else
        {
            set.mask                 |= MT53FB_SET_MASK_OSD_IOMMU;
            set.rSetting.u4OsdIommuEn = false;
        }

        if (updated & CLRCF_WIDTH)
        {
            set.mask |= MT53FB_SET_MASK_W; 
            set.rSetting.u2W = config->width;
        }

        if (updated & CLRCF_HEIGHT)
        {
            set.mask |= MT53FB_SET_MASK_H; 
            set.rSetting.u2H = config->height;
        }

        if (updated & CLRCF_FORMAT)
        {
            set.mask |= MT53FB_SET_MASK_CM; 
            set.rSetting.u1CM = _mapColorFormat(config->format);
        }

        if ((updated & CLRCF_WIDTH) || (updated & CLRCF_FORMAT))
        {
            set.mask |= MT53FB_SET_MASK_PITCH; 
            set.rSetting.u4Pitch = lock->pitch;
        }

        if (updated & CLRCF_BUFFERMODE)
        {
            set.mask |= MT53FB_SET_MASK_BUFFERMODE; 
            set.rSetting.u4BufferMode = config->buffermode;	
        }

        if (updated & CLRCF_SOURCE)
        {
            set.mask |= MT53FB_SET_MASK_SOURCE; 
            set.rSetting.source.x = config->source.x;
            set.rSetting.source.y = config->source.y;
            set.rSetting.source.w = config->source.w;
            set.rSetting.source.h = config->source.h;
        }

        if (updated & CLRCF_DEST || updated & CLRCF_CUR_DEST)
        {
            set.mask |= MT53FB_SET_MASK_DEST; 
            set.rSetting.dest.x = config->dest.x;
            set.rSetting.dest.y = config->dest.y;
            set.rSetting.dest.w = config->dest.w;
            set.rSetting.dest.h = config->dest.h;
        }

        if((config->options & DLOP_OPACITY) && (updated & CLRCF_OPACITY))
        {
            set.mask |= MT53FB_SET_MASK_OPACITY;			   
            set.rSetting.u4Opacity = config->opacity;
        }

        if((config->options & DLOP_SRC_COLORKEY) && (updated & CLRCF_SRCKEY))
        {
            unsigned int color_key = 0;
            switch(config->format)
            {
                case DSPF_ARGB4444:
                    // 				   color_key = PIXEL_ARGB4444(0xff, config->src_key.r, config->src_key.g, config->src_key.b);			   
                    color_key = PIXEL_ARGB4444(/*0xff*/0, config->src_key.r, config->src_key.g, config->src_key.b);
                    set.mask |= (MT53FB_SET_MASK_COLORKEY | MT53FB_SET_MASK_COLORKEYEN); 				   
                    break;

                case DSPF_ARGB:
                    color_key = PIXEL_ARGB(0xff, config->src_key.r, config->src_key.g, config->src_key.b);
                    set.mask |= (MT53FB_SET_MASK_COLORKEY | MT53FB_SET_MASK_COLORKEYEN); 									   
                    break;			 
                default:
                    break;
            }
            set.rSetting.u4ColorKeyEn = 1;								  
            set.rSetting.u4ColorKey = color_key; 											  
        }

        primary_set_region_mmu(&set.rSetting.mmu);
        D_DEBUG_AT( MT53_PRIMARY, "primarySetRegion[%d,0x%x,0x%x,%d]\n",set.rSetting.u4OsdPlaneID,set.rSetting.u4Base,set.rSetting.u4BufferMode,set.rSetting.u4Opacity );
        if (dfb_config_get_dump_stack(DDDS_LAYER_SET)) {
            D_INFO( "primarySetRegion[%d,0x%x,0x%x,%d]\n",set.rSetting.u4OsdPlaneID,set.rSetting.u4Base,set.rSetting.u4BufferMode,set.rSetting.u4Opacity );
            dfb_print_trace();
        }
        if (ioctl( dfb_mt53->fd, FBIO_SET, &set ) < 0) {
            D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__);
            return DFB_IO;
        }
    }

    if ((updated & CLRCF_PALETTE) && palette) {
        int i;
        struct mt53fb_palette pale;

        pale.plane_id = data->layer;

        for (i=0; i<256; i++) {
            pale.palette[i] = PIXEL_ARGB( palette->entries[i].a,
                    palette->entries[i].r,
                    palette->entries[i].g,
                    palette->entries[i].b );
        }

        ioctl( dfb_mt53->fd, FBIO_PALETTE, &pale );
    }

    return DFB_OK;
}

static DFBResult
primaryRemoveRegion( CoreLayer             *layer,
                     void                  *driver_data,
                     void                  *layer_data,
                     void                  *region_data )
{
     struct mt53fb_set set;
     MT53LayerData  *data = layer_data;
     
     assert(data->layer < MT53_MAX_OSD_PLANE);

     memset(&set,0x0,sizeof(struct mt53fb_set));
     
#ifdef CC_HW_WINDOW_SUPPORT
     if(data->curregion != region_data)
     {
          return DFB_OK;
     }
#endif
     set.rSetting.fgVisible = 0;
     set.rSetting.u4OsdPlaneID = data->layer;
        
     set.mask = MT53FB_SET_MASK_VISIBLE | MT53FB_SET_MASK_PLANE_ID;


     if (ioctl( dfb_mt53->fd, FBIO_SET, &set ) < 0) {
          D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__ );
          return DFB_IO;
     }

     return DFB_OK;
}

void dfb_print_trace (void)
{
    void *array[20];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace (array, 20);

    strings = (char **)backtrace_symbols (array, size);
	
    direct_log_printf(NULL,"dfb_print_trace[pid=%d] [size=%d]\n", getpid(), size);
	
    for (i = 0; i < size; i++)
    {
        direct_log_printf(NULL,"%s\n", strings[i]);
    }

    free (strings);
}


static DFBResult
primaryFlipRegion( CoreLayer                  *layer,
                    void                       *driver_data,
                    void                       *layer_data,
                    void                       *region_data,
                    CoreSurface                *surface,
                    DFBSurfaceFlipFlags         flags,
                    CoreSurfaceBufferLock      *left_lock,
                    CoreSurfaceBufferLock      *right_lock)
{
     struct mt53fb_set     set;
     MT53LayerData*         data = layer_data;
     CoreSurfaceBufferLock* lock=left_lock;
     u32                    u4_value;
     DIRECT_INTERFACE_DBG_DELTA_START();

     assert(data->layer < MT53_MAX_OSD_PLANE);
     D_MAGIC_ASSERT(surface, CoreSurface);

     memset(&set,0x0,sizeof(struct mt53fb_set));
    // system_munmap_check();

#ifdef CC_HW_WINDOW_SUPPORT
     if(data->curregion != region_data)
     {
          goto _primaryFlipRegion_END;
     }
#endif
	 #ifdef CC_OSD_M4U_IOVA_SUPPORT
	 set.rSetting.u4Base = system_gfx_switch_osd_iova(lock->phys);
	 #else
     set.rSetting.u4Base = lock->phys;
	 #endif
     set.rSetting.u4VirtAddr = (u32)lock->addr;

     if (surface->config.caps & DSCAPS_IOMMU)
     {
         set.mask |= MT53FB_SET_MASK_OSD_IOMMU;
         set.rSetting.u4OsdIommuEn = true;
         set.rSetting.u4_iova      = lock->iova;
     }
     else
     {
         set.mask                 |= MT53FB_SET_MASK_OSD_IOMMU;
         set.rSetting.u4OsdIommuEn = false;
     }

     set.rSetting.u4OsdPlaneID = data->layer;

     D_DEBUG_AT( MT53_PRIMARY, "primaryFlipRegion[%d,0x%x]\n",set.rSetting.u4OsdPlaneID,set.rSetting.u4Base);

     /* dump call stack ? */
     if (dfb_config_get_dump_stack(DDDS_LAYER_FLIP)) {
         dfb_print_trace();
     }

     /* delay for debug ? */
     if ((u4_value = dfb_config_get_delay_time(DDDLT_LAYER_FLIP)) > 0) {
          usleep(1000 * u4_value);
     }

     set.mask = MT53FB_SET_MASK_BASE | MT53FB_SET_MASK_PLANE_ID;
     
     primary_set_region_mmu(&set.rSetting.mmu);

     if (ioctl( dfb_mt53->fd, FBIO_SET, &set ) < 0) {
          D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__ );
          return DFB_IO;
     }

#ifdef CC_HW_WINDOW_SUPPORT
_primaryFlipRegion_END:
#endif

     if(!(flags & DSFLIP_BLIT))
     {
         dfb_surface_flip( surface, false );
     }

     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PRMRY_FLIP);

     return DFB_OK;
}

DFBResult primaryUpdateRegion( CoreLayer               *layer,
                            void                       *driver_data,
                            void                       *layer_data,
                            void                       *region_data,
                            CoreSurface                *surface,
                            const DFBRegion            *left_update,
                            CoreSurfaceBufferLock      *left_lock, 
                            const DFBRegion            *right_update,
                            CoreSurfaceBufferLock      *right_lock )
{
    return primaryFlipRegion(   layer,driver_data,layer_data,region_data,
                                surface,DSFLIP_BLIT,left_lock,right_lock);
}

static unsigned int primary_get_hw_layer_id(int dfb_layer_id)
{
    unsigned int hw_layer_id;
#ifdef CC_ANDROID_TWO_WORLDS
    switch(dfb_layer_id)
    {
#ifdef CC_ARIB_MODE_PRO
        case MT53_LAYER_OSD1:
            hw_layer_id = PMX_OSD1;
            break;
        case MT53_LAYER_OSD2:
            hw_layer_id = PMX_OSD2;
            break;
#else
        case MT53_LAYER_OSD1:
            hw_layer_id = PMX_OSD2;
            break;
        case MT53_LAYER_OSD2:
            hw_layer_id = PMX_OSD1;
            break;
#endif
        case MT53_LAYER_OSD3:
            hw_layer_id = PMX_OSD3;
            break;
        case MT53_LAYER_VDP1:
            hw_layer_id = PMX_PIP;
            break;
        case MT53_LAYER_VDP2:
            hw_layer_id = PMX_MAIN;
            break;
        default:
            hw_layer_id = PMX_MAX_INPORT_NS;
            break;
    }
#elif defined(CC_C4TV_SUPPORT) 
	switch(dfb_layer_id)
	{
		case MT53_LAYER_OSD1:
			hw_layer_id = PMX_OSD2;
			break;
		case MT53_LAYER_OSD2:
			hw_layer_id = PMX_OSD1;
			break;
		case MT53_LAYER_OSD3:
			hw_layer_id = PMX_OSD3;
			break;
		case MT53_LAYER_VDP1:
			hw_layer_id = PMX_PIP;
			break;
		case MT53_LAYER_VDP2:
			hw_layer_id = PMX_MAIN;
			break;
		default:
			hw_layer_id = PMX_MAX_INPORT_NS;
			break;
	}
#else
    switch(dfb_layer_id)
    {
        case MT53_LAYER_OSD1:
            hw_layer_id = PMX_OSD2;
            break;
        case MT53_LAYER_OSD2:
            hw_layer_id = PMX_OSD3;
            break;
        case MT53_LAYER_OSD3:
            hw_layer_id = PMX_OSD1;
            break;
        case MT53_LAYER_VDP1:
            hw_layer_id = PMX_PIP;
            break;
        case MT53_LAYER_VDP2:
            hw_layer_id = PMX_MAIN;
            break;
        default:
            hw_layer_id = PMX_MAX_INPORT_NS;
            break;
    }
#endif
    return hw_layer_id;
}


static DFBResult 
primaryGetLevel( CoreLayer              *layer,
                                         void                   *driver_data,
                                         void                   *layer_data,
                                         int                    *level )
{
     unsigned int au4Array[MT53_MAX_OSD_PLANE+1];
     unsigned int i;
     MT53LayerData* data = layer_data;

     assert(data->layer < MT53_MAX_OSD_PLANE);
     
     if (ioctl( dfb_mt53->fd, FBIO_GET_PLANE_ORDER_ARRAY, au4Array ) < 0) 
     {
          D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__ );
          return DFB_IO;
     }
     
     for(i = 0; i < MT53_MAX_OSD_PLANE; i++)
     {
         if(au4Array[i] == primary_get_hw_layer_id(data->layer))
         {
             *level = i;
         }
     }         


#ifdef CC_ANDROID_TWO_WORLDS
     *level = (*level) - 1;
#endif

     return DFB_OK;     
}




static DFBResult 
primarySetLevel             ( CoreLayer              *layer,
                                         void                   *driver_data,
                                         void                   *layer_data,
                                         int                     level )
{
    MT53LayerData  *data = layer_data;
    unsigned int i;
    unsigned int j;
    unsigned int au4Array[PMX_MAX_INPORT_NS+1];
    unsigned int hw_layer_id;

#ifdef CC_ANDROID_TWO_WORLDS
    level = level + 1;
#endif	

    assert(data->layer < MT53_MAX_OSD_PLANE);    

    if (ioctl( dfb_mt53->fd, FBIO_GET_PLANE_ORDER_ARRAY, au4Array ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__ );
        return DFB_IO;
    }   

    hw_layer_id = primary_get_hw_layer_id(data->layer);

    for(i = 0; i < PMX_MAX_INPORT_NS; i++)
    {
         if(au4Array[i] == hw_layer_id)
         {    
             break;
         }
    }  

    if(i < PMX_MAX_INPORT_NS)
    {
        if(level < i)
        {
            for(j = i; j > level; j --)            
            {
                au4Array[j] = au4Array[j - 1];
            }
        }     
        else if(level > i)
        {
            for(j = i; j < level; j ++)
            {
                au4Array[j] = au4Array[j + 1];
            }
        }
        au4Array[level] = hw_layer_id;
    }
    else
    {
        return DFB_FAILURE;
    }

    if (dfb_config_get_dump_stack(DDDS_LAYER_SER_ORDER)) {
        dfb_print_trace();
    }

    /* MHF Linux - Chun end */
    if (ioctl( dfb_mt53->fd, FBIO_SET_PLANE_ORDER_ARRAY, au4Array ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_SET failed! %s[line = %d] \n",  __func__, __LINE__ );
        return DFB_IO;
    }     

    return DFB_OK;
}



long long mdfb_clock_get_micros()
{   
    struct timeval tv;     
    gettimeofday( &tv, NULL );     
    return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}

void dfb_primary_enable_layer(unsigned int u4_layer,unsigned int u4_enable)
{
     struct mt53fb_set set;

     assert(u4_layer < MT53_MAX_OSD_PLANE);

     D_DEBUG_AT(MT53_PRIMARY, "%s,%d: [Layer: %d], [Enable: %d]\n", __func__, __LINE__, u4_layer, u4_enable);
     if (dfb_config_get_dump_stack(DDDS_LAYER_ENABLE)) {
         dfb_print_trace();
     }
     
     memset(&set,0x0,sizeof(struct mt53fb_set));

     set.rSetting.fgVisible     = u4_enable;
     set.rSetting.u4OsdPlaneID  = u4_layer;

     set.mask = MT53FB_SET_MASK_VISIBLE | MT53FB_SET_MASK_PLANE_ID;

     if (ioctl( dfb_mt53->fd, FBIO_SET, &set ) < 0)
     {
          D_PERROR( "DirectFB/MT53: dfb_primary_disable_layer failed!\n" );
     }

     return ;
}

/**********************************************************************************************************************/

/*const*/ DisplayLayerFuncs mt53PrimaryLayerFuncs = {
     LayerDataSize:      primaryLayerDataSize,
     RegionDataSize:     primaryRegionDataSize,
     InitLayer:          primaryInitLayer,
     GetLevel:           primaryGetLevel,
     SetLevel:           primarySetLevel,
     TestRegion:         primaryTestRegion,
     SetRegion:          primarySetRegion,
     RemoveRegion:       primaryRemoveRegion,
     FlipRegion:         primaryFlipRegion,
     UpdateRegion:       primaryUpdateRegion
};

