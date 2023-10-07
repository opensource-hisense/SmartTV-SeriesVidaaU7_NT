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

//#ifdef MSTAR_DEBUG_DRIVER
#define DIRECT_ENABLE_DEBUG
//#endif

#include <stdio.h>

#undef HAVE_STDLIB_H

#include <config.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>

#include <fcntl.h>
#include <dlfcn.h>

#include <asm/types.h>

#include <directfb.h>
#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/system.h>
#include <direct/mem.h>
#include <direct/memcpy.h>

#include <fusion/shmalloc.h>

#include <misc/conf.h>

#include <core/core.h>
#include <core/gfxcard.h>
#include <core/layers.h>
#include <core/screens.h>
#include <core/system.h>
#include <core/graphics_driver.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/state.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/palette.h>
#include <core/layers_internal.h>

#include <gfx/convert.h>

#define MI_OSD_INVALID_ID 0xFFFF

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_os.h"
#include "mi_osd.h"
#include "mi_disp.h"

#include "mstar_types.h"
#include "mstar.h"
#include "mstar_gles2.h"


#define USE_FBDEV 1


/* define global variables */
static bool (*GLESInitFuncs)(GLESFuncs *funcs) = NULL;
static GLESFuncs gGlesFuncs;


DFB_GRAPHICS_DRIVER( mstar_mi )


D_DEBUG_DOMAIN( MSTAR_MI_Driver, "MSTAR_MI/Driver", "MSTAR MI Driver" );

#pragma weak  MI_OSD_DrawLine_EX
MI_RESULT MI_OSD_DrawLine_EX(MI_HANDLE hSurface, const MI_OSD_LineAttr_t *pstLineAttr, MI_OSD_RenderJob_t *pstRenderJob, const MI_OSD_BlitOpt_t *pstBlitOpt);


static void _check_GLES2(void)
{
    static void* pHandle_gles2 = NULL;

    if (pHandle_gles2 == NULL)
    {
        const char *path = MSTAR_GLES2_SUBMODULE_PATH;
        char  buf[1024];

        snprintf(buf, sizeof(buf), "%s/%s",  direct_config->module_dir, path);
        printf("[DFB] %s, %d, load %s !\n", __FUNCTION__, __LINE__, buf);
        pHandle_gles2 = dlopen (buf, RTLD_LAZY);

        if (pHandle_gles2)
        {
            SAFE_GET_SYMBOL(pHandle_gles2, "GLES_Func_init", GLESInitFuncs);
            bool ret = GLESInitFuncs(&gGlesFuncs);

            if (ret == false)
                printf("[DFB] EGL/GL Init Failed!\n");
        }
        else
        {
            printf("[DFB] %s, %d, can't find %s !error: %s\n", __FUNCTION__, __LINE__, buf, dlerror());
        }
    }

}

static unsigned long
mstar_build_color( u32      pixel,
                   u8       a_st,
                   u8       a_bits,
                   u8       r_st,
                   u8       r_bits,
                   u8       g_st,
                   u8       g_bits,
                   u8       b_st,
                   u8       b_bits )
{
    u32 a = 0;
    u32 r = 0;
    u32 g = 0;
    u32 b = 0;

    if(a_bits)
        a = PIXEL_CHN_NORMAL(pixel, a_st, a_bits);

    if(r_bits)
        r = PIXEL_CHN_NORMAL(pixel, r_st, r_bits);

    if(g_bits)
        g = PIXEL_CHN_NORMAL(pixel, g_st, g_bits);

    if(b_bits)
        b = PIXEL_CHN_NORMAL(pixel, b_st, b_bits);

    if(a_bits)
    {
        while(a_bits < 8)
        {
            a = a | a >> a_bits;
            a_bits <<= 1;
        }
    }

    if(r_bits)
    {
        while(r_bits < 8)
        {
            r = r | r >> r_bits;
            r_bits <<= 1;
        }
    }

    if(g_bits)
    {
        while(g_bits < 8)
        {
            g = g | g >> g_bits;
            g_bits <<= 1;
        }
    }

    if(b_bits)
    {
        while(b_bits < 8)
        {
            b = b | b >> b_bits;
            b_bits <<= 1;
        }
    }

    return a << 24 | r << 16 | g << 8 | b;

}


static unsigned long mstar_pixel_to_color( DFBSurfacePixelFormat     format,
                      unsigned long             pixel,
                      CorePalette              *palette )
{
    u32 y = 0, cb = 0, cr = 0, r = 0, g = 0, b = 0;

    switch (format)
    {
        case DSPF_BLINK12355:
                return 0;
                //return mstar_build_color(pixel, 16, 0, 0, 5, 0, 5, 0, 5);
                //return mstar_build_color(pixel, 12, 4, 8, 4, 4, 4, 0, 4);

        case DSPF_BLINK2266:
                return 0;
                //return mstar_build_color(pixel, 16, 0, 0, 5, 0, 5, 0, 5);

        case DSPF_ARGB1555:
                return mstar_build_color(pixel, 15, 1, 10, 5, 5, 5, 0, 5);

        case DSPF_RGB16:
                return mstar_build_color(pixel, 16, 0, 11, 5, 5, 6, 0, 5);

        case DSPF_RGB555:
                return mstar_build_color(pixel, 15, 0, 10, 5, 5, 5, 0, 5);

        case DSPF_BGR555:
                return mstar_build_color(pixel, 15, 0, 0, 5, 5, 5, 10, 5);

        case DSPF_ARGB2554:
                return mstar_build_color(pixel, 14, 2, 9, 5, 4, 5, 0, 4);

        case DSPF_ARGB4444:
                return mstar_build_color(pixel, 12, 4, 8, 4, 4, 4, 0, 4);

        case DSPF_RGB444:
                return mstar_build_color(pixel, 12, 0, 8, 4, 4, 4, 0, 4);

        case DSPF_RGB24:
                return mstar_build_color(pixel, 24, 0, 16, 8, 8, 8, 0, 8);

        case DSPF_RGB32:
                return mstar_build_color(pixel, 24, 0, 16, 8, 8, 8, 0, 8);

        case DSPF_ARGB:
                return mstar_build_color(pixel, 24, 8, 16, 8, 8, 8, 0, 8);

        case DSPF_ABGR:
                return mstar_build_color(pixel, 24, 8, 0, 8, 8, 8, 16, 8);

        case DSPF_LUT8:
        case DSPF_LUT4:
        case DSPF_LUT2:
        case DSPF_LUT1:
                if( palette )
                {
                    if( pixel >= 0 &&
                        pixel <= (1UL << DFB_BITS_PER_PIXEL(format)) - 1)
                        return  ((unsigned long)palette->entries[pixel].a) << 24 |
                                ((unsigned long)palette->entries[pixel].r) << 16 |
                                ((unsigned long)palette->entries[pixel].g) << 8  |
                                                palette->entries[pixel].b;
                }

                break;

        case DSPF_YUY2:
                cb = (pixel >> 24) & 0xFF;
                y  = (pixel >> 16) & 0xFF;
                cr = (pixel >>  8) & 0xFF;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );

                return 0xFF << 24 | r << 16 | g << 8 | b;

        case DSPF_YVYU:
                cr = (pixel >> 24) & 0xFF;
                y  = (pixel >> 16) & 0xFF;
                cb = (pixel >> 8 ) & 0xFF;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );

                return 0xFF << 24 | r << 16 | g << 8 | b;

        case DSPF_UYVY:
                y   = (pixel >> 24) & 0xFF;
                cb  = (pixel >> 16) & 0xFF;
                cr  = (pixel      ) & 0xFF;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );
                return 0xFF << 24 | r << 16 | g << 8 | b;

        default:
                D_WARN( "[DFB] unknown format : %s\n", dfb_pixelformat_name(format) );
                break;
    }

    return 0x55555555;
}


static unsigned long mstar_pixel_to_colorkey( DFBSurfacePixelFormat      src_format,
                         DFBSurfacePixelFormat      dst_format,
                         unsigned long              pixel,
                         CorePalette               *palette )
{
    switch (dst_format)
    {
        case DSPF_LUT8:
        case DSPF_LUT4:
        case DSPF_LUT2:
        case DSPF_LUT1:
                if( pixel >= 0 && pixel<=(1UL<<DFB_BITS_PER_PIXEL(dst_format))-1)
                    return pixel << 24  |
                           pixel << 16  |
                           pixel <<  8  |
                           pixel;

                return 0;
        default:
                return mstar_pixel_to_color(src_format, pixel, palette);
    }
}


/*
    return true if blending enabled; return false if blending disabled;
*/
static inline bool
mstarMapGFXBlitOption( DFBSurfaceBlittingFlags      blittingflags,
                       DFBSurfaceBlendFunction      src_blend,
                       DFBSurfaceBlendFunction      dst_blend,
                       MI_OSD_DfbBlendMode_e     *pgfx_src_bld_op,
                       MI_OSD_DfbBlendMode_e     *pgfx_dst_bld_op,
                       u16                         *pu16_bld_flags )
{
// kunic need to do
    if(DSBLIT_NOFX == blittingflags)
        return false;

    //Set Blend Flag:
    *pu16_bld_flags = 0x0;
    if(blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_ALPHA_CHANNEL;

    if(blittingflags & DSBLIT_BLEND_COLORALPHA)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_COLOR_ALPHA;

    if(blittingflags & DSBLIT_COLORIZE)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_COLORIZE;

    if(blittingflags & DSBLIT_SRC_PREMULTIPLY)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_SRC_PREMUL;
    if(blittingflags & DSBLIT_DST_PREMULTIPLY)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_DST_PREMUL;

    if(blittingflags & DSBLIT_DEMULTIPLY)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_DEMULTIPLY;

    if(blittingflags & DSBLIT_SRC_PREMULTCOLOR)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_SRC_PREMULCOL;

    if(blittingflags & DSBLIT_XOR)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_XOR;

    if(blittingflags & DSBLIT_SRC_MASK_ALPHA)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_SRC_ALPHA_MASK;

    if(blittingflags & DSBLIT_SRC_MASK_COLOR)
        *pu16_bld_flags |= E_MI_OSD_DFB_BLEND_FLAG_SRC_COLOR_MASK;

    //No blending!
    if(0x0 == *pu16_bld_flags)
        return false;

    if(DSBF_UNKNOWN == src_blend)
        *pgfx_src_bld_op = E_MI_OSD_DFB_BLEND_ONE;
    else
        *pgfx_src_bld_op = (MI_OSD_DfbBlendMode_e)(src_blend-1);

    if(DSBF_UNKNOWN == dst_blend)
        *pgfx_dst_bld_op = E_MI_OSD_DFB_BLEND_ONE;
    else
        *pgfx_dst_bld_op = (MI_OSD_DfbBlendMode_e)(dst_blend-1);

    if(!(blittingflags & (DSBLIT_BLEND_ALPHACHANNEL|DSBLIT_BLEND_COLORALPHA)))
    {
        //patch HW, current GE only do blend operations with alpha blend on
        *pgfx_src_bld_op = E_MI_OSD_DFB_BLEND_ONE;
        *pgfx_dst_bld_op = E_MI_OSD_DFB_BLEND_ZERO;
    }

    return true;
}

/*
   return true for hw support this blit options
*/
static inline bool
mstarCheckBlitOption( DFBSurfaceBlittingFlags       blittingflags,
                      DFBSurfaceBlittingFlags       supported_blittingflags )
{
    if(blittingflags & ~(supported_blittingflags))
        return false;

    return true;
}

static void
_mstarCheckStateInternal( CardState                     *state,
                          DFBAccelerationMask            accel,
                          DFBAccelerationMask           *pAccelRdy,
                          DFBSurfaceBlittingFlags        supported_blittingflags )
{
    bool bcheck_destination_rgb_format = false;

    if(state->render_options & DSRO_MATRIX)
    {
        if( state->matrix[0] != (1 << 16)       ||
            state->matrix[1] != 0               ||
            state->matrix[2] != 0               ||
            state->matrix[3] != 0               ||
            state->matrix[4] != (1 << 16)       ||
            state->matrix[5] != 0               ||
            state->matrix[6] != 0               ||
            state->matrix[7] != 0               ||
            state->matrix[8] != (1<<16)             )
            {
              printf("[DFB] matrix not supported yet!!\n");
            }
    }

    if( state->destination->config.size.w >= dfb_config->mst_ge_hw_limit    ||
        state->destination->config.size.h >= dfb_config->mst_ge_hw_limit )
    {
        printf("[DFB] Against the GE hardware limitation! HW (GE) not support return to SW render ! width: %d(limit:%d), height:%d(limit:%d)\n",
                state->destination->config.size.w,
                dfb_config->mst_ge_hw_limit - 1,
                state->destination->config.size.h,
                dfb_config->mst_ge_hw_limit -1);

        return;
    }

    //here check whether the dstbuffer foramt is supported by the  GE
    switch(state->destination->config.format)
    {
        case DSPF_ARGB4444:
        case DSPF_ARGB1555:
        case DSPF_ARGB:
        case DSPF_ABGR:
        case DSPF_RGB16:
        case DSPF_RGB32:
                if(dfb_config->do_yuvtorgb_sw_patch)
                     bcheck_destination_rgb_format = true;
        case DSPF_YVYU:
        case DSPF_YUY2:
        case DSPF_UYVY:
        case DSPF_LUT8:
        case DSPF_BLINK12355:
        case DSPF_BLINK2266:
                break;

        case DSPF_A8:
                if(DFB_BLITTING_FUNCTION(accel)&&(state->source->config.format == DSPF_A8))
                    break;
                else
                    return;
        default:
                printf( "[DFB] error format : %s (%s, %s)\n", dfb_pixelformat_name(state->destination->config.format), __FILE__, __FUNCTION__ );
                return;
    }

    if(DFB_BLITTING_FUNCTION(accel))
    {
        if( state->source->config.size.w  >=  dfb_config->mst_ge_hw_limit  ||
            state->source->config.size.h  >=  dfb_config->mst_ge_hw_limit)
        {
            printf("[DFB] Against the hardware limitation! HW (GE) not support return to SW render ! width: %d(limit:%d), height:%d(limit:%d)\n",
                    state->source->config.size.w,
                    dfb_config->mst_ge_hw_limit - 1,
                    state->source->config.size.h,
                    dfb_config->mst_ge_hw_limit -1);

            return;
        }

        /*
             HW GE can not support more than 4096 size of the rotate
        */
        if (state->blittingflags & (DSBLIT_ROTATE90|DSBLIT_ROTATE180|DSBLIT_ROTATE270))
        {
            if((state->source->config.size.w  >= DEFAULT_MAX_GFX_ROTATE_LIMIT)           ||
               (state->source->config.size.h  >= DEFAULT_MAX_GFX_ROTATE_LIMIT)           ||
               (state->destination->config.size.h  >= DEFAULT_MAX_GFX_ROTATE_LIMIT)      ||
               (state->destination->config.size.w  >= DEFAULT_MAX_GFX_ROTATE_LIMIT))
            {
                printf("[DFB] Against the hardware limitation! HW (GE) not support rotate return to SW render !\n");
                printf("[DFB] source(%d, %d) to dest(%d, %d) (GE rotate limit:%d)\n",
                    state->source->config.size.w,
                    state->source->config.size.h,
                    state->destination->config.size.w,
                    state->destination->config.size.h,
                    DEFAULT_MAX_GFX_ROTATE_LIMIT - 1);

                return;
            }
        }

        if(mstarCheckBlitOption(state->blittingflags, supported_blittingflags))
        {
            switch(state->source->config.format)
            {
                case DSPF_LUT8:
                        if (dfb_config->do_i8toargb4444_sw_patch) // For K5S U01 HW bug. U02 have fix it.
                        {
                            D_INFO("[DFB] HW BUG to patch it with SW (i8 to other pixelfomat)\n");
                            return;
                        }
                        else
                        {
                            *pAccelRdy |= accel&(DFXL_BLIT|DFXL_STRETCHBLIT|DFXL_BLIT2);
                            break;
                        }

                case DSPF_YUY2:
                case DSPF_YVYU:
                case DSPF_UYVY:
                        if (bcheck_destination_rgb_format && dfb_config->do_yuvtorgb_sw_patch)
                        {
                            D_INFO("[DFB] HW BUG to patch it with SW\n");
                            return;
                        }

                case DSPF_LUT4:
                case DSPF_LUT2:
                case DSPF_LUT1:
                case DSPF_ARGB1555:
                case DSPF_ARGB:
                case DSPF_ABGR:
                case DSPF_ARGB4444:
                case DSPF_RGB16:
                case DSPF_RGB32:
                case DSPF_BLINK12355:
                case DSPF_BLINK2266:
                case DSPF_A8:
                        *pAccelRdy |= accel&(DFXL_BLIT|DFXL_STRETCHBLIT|DFXL_BLIT2);
                        break;

                default:
                    break;
            }

            /*
            stretchblit_with_rotate, this flag default setting is enable.
            maybe some chip or some problem need to diable stretchblit with rotate.
            */

            //do not support stretchblit with rotation
            if( (!dfb_config->stretchblit_with_rotate)  &&
                (accel & DFXL_STRETCHBLIT)              &&
                (state->blittingflags & (DSBLIT_ROTATE90|DSBLIT_ROTATE180|DSBLIT_ROTATE270)))
            {
                //printf("do not support stretchblit with rotation NOW \n");
                *pAccelRdy ^= DFXL_STRETCHBLIT;
            }


        }
    }

    if(DFB_DRAWING_FUNCTION(accel))
    {
        *pAccelRdy |= accel & ( DFXL_FILLRECTANGLE      |
                                DFXL_DRAWLINE           |
                                DFXL_DRAWRECTANGLE      |
                                DFXL_FILLTRAPEZOID      |
                                DFXL_FILLQUADRANGLE );
    }
}

void
mstarCheckState( void                       *drv,
                 void                       *dev,
                 CardState                  *state,
                 DFBAccelerationMask         accel )
{
     MSTARDriverData *sdrv = drv;

     return _mstarCheckStateInternal( state,
                                      accel,
                                      &state->accel,
                                      sdrv->gfx_supported_bld_flags );
}

void
mstarCheckState_null( void                      *drv,
                      void                      *dev,
                      CardState                 *state,
                      DFBAccelerationMask        accel )
{

}

static void
mstarSetEngineBlitState( StateModificationFlags      modified,
                         MSTARDriverData            *sdrv,
                         MSTARDeviceData            *sdev )
{
   ////////////////////////////////////////////////////////////
    //Set Src Buffer, Blt ops Needed:
    if(modified & SMF_SOURCE)
    {
        MI_OSD_SurfaceInfo_t stSurfInfo;
        //MI_U32     u32VirtualAddr = 0;
        MI_RESULT Ret = MI_ERR_FAILED;
        memset(&stSurfInfo,0x00, sizeof(MI_OSD_SurfaceInfo_t));

        stSurfInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        stSurfInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;

       // if(MI_OS_PA2KSEG1(sdrv->src_phys, &u32VirtualAddr) != MI_OK)
       // {
       //     printf("SMF_SOURCE Fail to call MI_OS_PA2KSEG1()");
       // }

	 //printf("[DFB] SMF_SOURCE u32VirtualAddr:%x \n", u32VirtualAddr);
        //stSurfInfo.u32VAddr  = (MI_U32)u32VirtualAddr;

        stSurfInfo.phyAddr  = (MI_PHY)sdrv->src_phys;
        stSurfInfo.eColorFormat  = sdrv->src_ge_format;
        stSurfInfo.u32Width   = sdrv->src_w;
        stSurfInfo.u32Height  = sdrv->src_h;
        stSurfInfo.u32Pitch     = sdrv->src_pitch;

	 if(sdrv->hSrcSurface != MI_OSD_INVALID_ID)
	 {
	     Ret = MI_OSD_SurfaceDestroy(sdrv->hSrcSurface);
            if (Ret != MI_OK)
            {
                printf("[DFB] MI_OSD_SurfaceDestroy hSrcSurface! ret = %d\n", Ret);
            }
	 }

        Ret = MI_OSD_SurfaceCreate(&stSurfInfo, &sdrv->hSrcSurface);
        if (Ret != MI_OK)
        {
            printf("[DFB] MI_OSD_SurfaceCreate Create fail! ret = %d\n", Ret);
        }
    }

    //Set ColorKey:
    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY ))
    {
        MI_OSD_ColorKeyInfo_t stCKeyInfo;
        memset(&stCKeyInfo, 0x00, sizeof(MI_OSD_ColorKeyInfo_t));

        if (sdrv->ge_src_colorkey_enabled)
            MI_OSD_SurfaceEnableColorKey(sdrv->hSrcSurface);
        else
            MI_OSD_SurfaceDisableColorKey(sdrv->hSrcSurface);

        stCKeyInfo.eColorKeyOperation = E_MI_OSD_COLOR_KEY_OPERATION_RGB_EQUAL;
        stCKeyInfo.stColorKey.stMinRgb.u8Alpha = stCKeyInfo.stColorKey.stMaxRgb.u8Alpha = (sdrv->src_ge_clr_key >>24);
        stCKeyInfo.stColorKey.stMinRgb.u8Red = stCKeyInfo.stColorKey.stMaxRgb.u8Red = (sdrv->src_ge_clr_key >>16)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Green = stCKeyInfo.stColorKey.stMaxRgb.u8Green = (sdrv->src_ge_clr_key >>8)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Blue = stCKeyInfo.stColorKey.stMaxRgb.u8Blue = (sdrv->src_ge_clr_key)&0xFF;

        MI_OSD_SurfaceSetColorKey(sdrv->hSrcSurface, &stCKeyInfo);
    }

    if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY ))
    {
        MI_OSD_ColorKeyInfo_t stCKeyInfo;
        memset(&stCKeyInfo, 0x00, sizeof(MI_OSD_ColorKeyInfo_t));

        if (sdrv->ge_blit_dst_colorkey_enabled)
            MI_OSD_SurfaceEnableColorKey(sdrv->hDstSurface);
        else
            MI_OSD_SurfaceDisableColorKey(sdrv->hDstSurface);

        stCKeyInfo.eColorKeyOperation = E_MI_OSD_COLOR_KEY_OPERATION_RGB_NOT_EQUAL;
        stCKeyInfo.stColorKey.stMinRgb.u8Alpha = stCKeyInfo.stColorKey.stMaxRgb.u8Alpha = (sdrv->dst_ge_clr_key >>24);
        stCKeyInfo.stColorKey.stMinRgb.u8Red = stCKeyInfo.stColorKey.stMaxRgb.u8Red = (sdrv->dst_ge_clr_key >>16)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Green = stCKeyInfo.stColorKey.stMaxRgb.u8Green = (sdrv->dst_ge_clr_key >>8)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Blue = stCKeyInfo.stColorKey.stMaxRgb.u8Blue = (sdrv->dst_ge_clr_key)&0xFF;

        MI_OSD_SurfaceSetColorKey(sdrv->hDstSurface, &stCKeyInfo);
    }

    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR))//Set Alpha Blend
    {
        if(sdrv->ge_blit_alpha_blend_enabled)
        {
            switch(sdrv->state->src_blend)
            {

                /*
                If use MApi_GFX_SetAlpha, no need to call MApi_GFX_EnableAlphaBlending(TRUE).
                because MApi_GFX_SetAlpha first argument is used for controlling alpha blending enable or disable.
                */
                case DSBF_CSRC_ASRCxACONST_EXT:
                        sdrv->stBlitOpt.bIsDfbBlend = false;
                        sdrv->stBlitOpt.eBlendMode = E_MI_OSD_BLEND_ONE;
                        sdrv->stBlitOpt.eAlphaFrom = E_MI_OSD_ALPHA_FROM_SRC_CONST;
                        sdrv->stBlitOpt.eConstAlphaFrom = E_MI_OSD_CONST_ALPHA_FROM_DST;
                        MI_OSD_SurfaceSetConstAlpha(sdrv->hDstSurface, sdrv->color.a);
                        break;

                default :
                {
                        sdrv->stBlitOpt.bIsDfbBlend = true;
                        sdrv->stBlitOpt.eSrcDfbBlendMode = sdrv->ge_src_blend;
                        sdrv->stBlitOpt.eDstDfbBlendMode = sdrv->ge_dst_blend;
                        sdrv->stBlitOpt.eDfbBlendFlag = sdrv->ge_blit_bld_flags;

                        sdrv->stBlitOpt.stConstColor.u8Alpha = sdrv->color.a;
                        sdrv->stBlitOpt.stConstColor.u8Red = sdrv->color.r;
                        sdrv->stBlitOpt.stConstColor.u8Green = sdrv->color.g;
                        sdrv->stBlitOpt.stConstColor.u8Blue = sdrv->color.b;

                        break;
                }
            }
        }
        else
        {
            sdrv->stBlitOpt.bIsDfbBlend = false;
            sdrv->stBlitOpt.eBlendMode = E_MI_OSD_BLEND_NONE;
            sdrv->stBlitOpt.eAlphaFrom = E_MI_OSD_ALPHA_FROM_SRC;
        }
    }

    //Set BLIT rotation
    if(modified & SMF_BLITTING_FLAGS)
    {
        if(sdrv->bflags & DSBLIT_ROTATE90)
            sdrv->stBlitOpt.eRotate = E_MI_OSD_ROTATE_270;

        else if(sdrv->bflags & DSBLIT_ROTATE180)
            sdrv->stBlitOpt.eRotate = E_MI_OSD_ROTATE_180;

        else if(sdrv->bflags & DSBLIT_ROTATE270)
            sdrv->stBlitOpt.eRotate = E_MI_OSD_ROTATE_90;
        else
            sdrv->stBlitOpt.eRotate = E_MI_OSD_ROTATE_NONE;
    }

    //Set Blit Nearest Mode:
    if(modified & SMF_SRC_CONVOLUTION)
    {
        if (sdrv->ge_blit_nearestmode_enabled)
            sdrv->stBlitOpt.eStretchBlitType  = E_MI_OSD_STRETCH_BLIT_TYPE_NEAREST;
	else
            sdrv->stBlitOpt.eStretchBlitType  = E_MI_OSD_STRETCH_BLIT_TYPE_BILINEAR;
    }

#if 0
//kunic need to do
    if( modified & SMF_BLINK                            &&
        sdrv->src_ge_format == GFX_FMT_I1               &&
        sdrv->dst_ge_format == GFX_FMT_1ABFGBG12355 )
    {
        GFX_BlinkData blinkData;

        blinkData.foreground = sdrv->color.r;
        blinkData.background = sdrv->color.g;

        blinkData.Bits.Blink = 0;
        blinkData.Bits.blink_en = 1;
        blinkData.Bits.Alpha = 0;

        MApi_GFX_SetIntensity(0, sdrv->dst_ge_format, (MS_U32 *)&blinkData);

        blinkData.foreground = sdrv->color.b;
        blinkData.background = sdrv->color.a;


        if (sdrv->bflags & (DSBLIT_SOURCE2 & DSBLIT_BLINK_FOREGROUND))
            blinkData.Bits.Blink = GEBLINK_BOTH;

        else if (sdrv->bflags & DSBLIT_SOURCE2)
            blinkData.Bits.Blink = GEBLINK_BACKGROUND;

        else if  (sdrv->bflags & DSBLIT_BLINK_FOREGROUND)
            blinkData.Bits.Blink = GEBLINK_FOREGROUND;

        else
            blinkData.Bits.Blink = GEBLINK_NONE;


        blinkData.Bits.blink_en = 1;

        MApi_GFX_SetIntensity(1, sdrv->dst_ge_format, (MS_U32 *)&blinkData);

    }


    if( modified & SMF_BLINK                            &&
        sdrv->src_ge_format == GFX_FMT_I1               &&
        sdrv->dst_ge_format == GFX_FMT_FABAFGBG2266 )
    {
        GFX_BlinkData blinkData;
        // set blink data and intensity
        blinkData.Bits3.Fa = 0x00;
        blinkData.Bits3.Ba = 0x00;
        blinkData.Bits3.reserved = 0;
        blinkData.background = sdrv->color.r;
        blinkData.foreground = sdrv->color.g;

        if (sdrv->bflags & DSBLIT_FLIP_HORIZONTAL )
            blinkData.Bits3.Fa |= 0x00;

        else if (sdrv->bflags & DSBLIT_FLIP_VERTICAL)
            blinkData.Bits3.Fa |= 0x01;

        else if (sdrv->bflags & DSBLIT_ROP)
            blinkData.Bits3.Fa |= 0x10;

        else if (sdrv->bflags & DSBLIT_SRC_COLORMATRIX)
            blinkData.Bits3.Fa |= 0x11;


        if (sdrv->bflags & DSBLIT_SRC_CONVOLUTION )
            blinkData.Bits3.Ba |= 0x00;

        else if (sdrv->bflags & DSBLIT_BA_NOUSE)
            blinkData.Bits3.Ba |= 0x01;

        else if (sdrv->bflags & DSBLIT_BA_TRANSLUCENT)
            blinkData.Bits3.Ba |= 0x10;

        else if (sdrv->bflags & DSBLIT_BA_TRANSPARENT)
            blinkData.Bits3.Ba |= 0x11;

        MApi_GFX_SetIntensity(0, GFX_FMT_FABAFGBG2266, (MS_U32*)&(blinkData));


        // set blink data and intensity
        blinkData.Bits3.Fa = 0;
        blinkData.Bits3.Ba = 0;
        blinkData.Bits3.reserved = 0;
        blinkData.background = sdrv->color.b;
        blinkData.foreground = sdrv->color.a;


        if (sdrv->bflags & DSBLIT_FLIP_HORIZONTAL )
            blinkData.Bits3.Fa |= 0x00;

        else if (sdrv->bflags & DSBLIT_FLIP_VERTICAL)
            blinkData.Bits3.Fa |= 0x01;

        else if (sdrv->bflags & DSBLIT_ROP)
            blinkData.Bits3.Fa |= 0x10;

        else if (sdrv->bflags & DSBLIT_SRC_COLORMATRIX)
            blinkData.Bits3.Fa |= 0x11;


        if (sdrv->bflags & DSBLIT_SRC_CONVOLUTION )
            blinkData.Bits3.Ba |= 0x00;

        else if (sdrv->bflags & DSBLIT_BA_NOUSE)
            blinkData.Bits3.Ba |= 0x01;

        else if (sdrv->bflags & DSBLIT_BA_TRANSLUCENT)
            blinkData.Bits3.Ba |= 0x10;

        else if (sdrv->bflags & DSBLIT_BA_TRANSPARENT)
            blinkData.Bits3.Ba |= 0x11;

        MApi_GFX_SetIntensity(1, GFX_FMT_FABAFGBG2266, (MS_U32*)&(blinkData));
    }

#endif
    //GOP mirror do GE mirror for mantis id:0523773
    if (dfb_config->do_GE_Vmirror == true)
    {
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {
            if ( sdrv->ge_blit_xmirror == true && sdrv->ge_blit_ymirror == true)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_HORIZONTAL;
            else if ( sdrv->ge_blit_xmirror == true && sdrv->ge_blit_ymirror == false)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL;
            else if ( sdrv->ge_blit_xmirror == false && sdrv->ge_blit_ymirror == true)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_NONE;
            else if ( sdrv->ge_blit_xmirror == false && sdrv->ge_blit_ymirror == false)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_VERTICAL;
        }
        sdrv->stBlitOpt.eDstMirror = E_MI_OSD_MIRROR_VERTICAL;
    }
    else
    {
        //Set Blit Mirror:
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {
            if ( sdrv->ge_blit_xmirror == true && sdrv->ge_blit_ymirror == true)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL;
            else if ( sdrv->ge_blit_xmirror == true && sdrv->ge_blit_ymirror == false)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_HORIZONTAL;
            else if ( sdrv->ge_blit_xmirror == false && sdrv->ge_blit_ymirror == true)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_VERTICAL;
            else if ( sdrv->ge_blit_xmirror == false && sdrv->ge_blit_ymirror == false)
                sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_NONE;
        }
        sdrv->stBlitOpt.eDstMirror = E_MI_OSD_MIRROR_NONE;
    }

    //set ROP2 option Code:
    if(modified & SMF_WRITE_MASK_BITS)
        sdrv->stBlitOpt.eRopMode = sdrv->ge_rop_op;

    //Set Blit Palette:
    if(sdrv->ge_palette_enabled && sdev->num_entries)
    {
        MI_OSD_SurfaceSetPalette(sdrv->hSrcSurface, 0,  sdev->num_entries, sdrv->dst_ge_format, sdev->palette_tbl);
    }

    //Set Intensity Palette:
    if(sdrv->ge_palette_intensity_enabled       &&
       dfb_config->mst_blink_frame_rate == 0)
    {
        MI_OSD_SurfaceSetPalette(sdrv->hSrcSurface, 0,  sdev->num_intensity, E_MI_OSD_COLOR_FORMAT_ARGB8888, sdev->palette_intensity);
    }
}

static void
mstarSetEngineDrawState( StateModificationFlags      modified,
                         MSTARDriverData            *sdrv,
                         MSTARDeviceData            *sdev)
{

    if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY ))
    {
        MI_OSD_ColorKeyInfo_t stCKeyInfo;
        memset(&stCKeyInfo, 0x00, sizeof(MI_OSD_ColorKeyInfo_t));

        if (sdrv->ge_draw_dst_colorkey_enabled)
            MI_OSD_SurfaceEnableColorKey(sdrv->hDstSurface);
        else
            MI_OSD_SurfaceDisableColorKey(sdrv->hDstSurface);

        stCKeyInfo.eColorKeyOperation = E_MI_OSD_COLOR_KEY_OPERATION_RGB_NOT_EQUAL;
        stCKeyInfo.stColorKey.stMinRgb.u8Alpha = stCKeyInfo.stColorKey.stMaxRgb.u8Alpha = (sdrv->dst_ge_clr_key >>24);
        stCKeyInfo.stColorKey.stMinRgb.u8Red = stCKeyInfo.stColorKey.stMaxRgb.u8Red = (sdrv->dst_ge_clr_key >>16)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Green = stCKeyInfo.stColorKey.stMaxRgb.u8Green = (sdrv->dst_ge_clr_key >>8)&0xFF;
        stCKeyInfo.stColorKey.stMinRgb.u8Blue = stCKeyInfo.stColorKey.stMaxRgb.u8Blue = (sdrv->dst_ge_clr_key)&0xFF;

        MI_OSD_SurfaceSetColorKey(sdrv->hDstSurface, &stCKeyInfo);
    }

    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR))
    {
        if(sdrv->ge_draw_alpha_blend_enabled)
        {
            sdrv->stBlitOpt.bIsDfbBlend = true;
            sdrv->stBlitOpt.eSrcDfbBlendMode = sdrv->ge_src_blend;
            sdrv->stBlitOpt.eDstDfbBlendMode = sdrv->ge_dst_blend;
            sdrv->stBlitOpt.eDfbBlendFlag = sdrv->ge_draw_bld_flags;

            sdrv->stBlitOpt.stConstColor.u8Alpha = sdrv->color.a;
            sdrv->stBlitOpt.stConstColor.u8Red = sdrv->color.r;
            sdrv->stBlitOpt.stConstColor.u8Green = sdrv->color.g;
            sdrv->stBlitOpt.stConstColor.u8Blue = sdrv->color.b;

            //if alpha_compare_mode can work when DFB HW blend ,this patch should be removed
            if(sdrv->ge_render_alpha_cmp_enabled)
            {
                sdrv->stBlitOpt.bIsDfbBlend = false;
                sdrv->stBlitOpt.eBlendMode = E_MI_OSD_BLEND_ASRC;
                sdrv->stBlitOpt.eAlphaFrom = E_MI_OSD_ALPHA_FROM_SRC;
                sdrv->stBlitOpt.eConstAlphaFrom = E_MI_OSD_CONST_ALPHA_FROM_DST;
                MI_OSD_SurfaceSetConstAlpha(sdrv->hDstSurface, sdrv->color.a);
            }
        }
        else
        {
            sdrv->stBlitOpt.bIsDfbBlend = false;
            sdrv->stBlitOpt.eBlendMode = E_MI_OSD_BLEND_NONE;
            sdrv->stBlitOpt.eAlphaFrom = E_MI_OSD_ALPHA_FROM_SRC;
        }
    }
}

static void
mstarSetEngineShareState( StateModificationFlags     modified,
                          DFBAccelerationMask        support_accel,
                          MSTARDriverData           *sdrv,
                          MSTARDeviceData           *sdev )
{
    ////////////////////////////////////////////////////////////
    //Set Dst Buffer State, All ops Needed:
    if(modified & SMF_DESTINATION)
    {

        MI_OSD_SurfaceInfo_t stSurfInfo;
        MI_RESULT Ret = MI_ERR_FAILED;

        memset(&stSurfInfo,0x00, sizeof(MI_OSD_SurfaceInfo_t));

        stSurfInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        stSurfInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;

        stSurfInfo.phyAddr  = (MI_PHY)sdrv->dst_phys;
        stSurfInfo.eColorFormat  = sdrv->dst_ge_format;
        stSurfInfo.u32Width   = sdrv->dst_w;
        stSurfInfo.u32Height  = sdrv->dst_h;
        stSurfInfo.u32Pitch     = sdrv->dst_pitch;

        if(sdrv->hDstSurface != MI_OSD_INVALID_ID)
        {
            Ret = MI_OSD_SurfaceDestroy(sdrv->hDstSurface);
            if (Ret != MI_OK)
            {
                printf("[DFB] MI_OSD_SurfaceDestroy hDstSurface! ret = %d\n", Ret);
            }
        }

        Ret = MI_OSD_SurfaceCreate(&stSurfInfo, &sdrv->hDstSurface);
        if (Ret != MI_OK)
        {
            printf("[DFB] MI_OSD_SurfaceCreate Create fail! ret = %d\n", Ret);
        }

        if(!sdev->b_hwclip)
        {
            sdrv->stBlitOpt.stClipRect.u32X = 0;
            sdrv->stBlitOpt.stClipRect.u32Y = 0;
			sdrv->stBlitOpt.stClipRect.u32Width = sdrv->dst_w;//fix Hw clip error of the MI backend
			sdrv->stBlitOpt.stClipRect.u32Height = sdrv->dst_h;
        }
    }

    if((modified & SMF_CLIP) && sdev->b_hwclip)
    {
        sdrv->stBlitOpt.stClipRect.u32X = sdrv->clip.x1;
        sdrv->stBlitOpt.stClipRect.u32Y = sdrv->clip.y1;
        sdrv->stBlitOpt.stClipRect.u32Width = sdrv->clip.x2 - sdrv->clip.x1 + 1;
        sdrv->stBlitOpt.stClipRect.u32Height = sdrv->clip.y2 - sdrv->clip.y1 + 1;
    }

    //SetYUV Format:
    if(modified & (SMF_DESTINATION | SMF_SOURCE))
    {
        sdrv->stBlitOpt.eSrcYuvFormat = sdrv->src_ge_yuv_fmt;
        sdrv->stBlitOpt.eDstYuvFormat = sdrv->dst_ge_yuv_fmt;

    }

    if(sdrv->ge_dither_enable)
        sdrv->stBlitOpt.bDither = true;
    else
        sdrv->stBlitOpt.bDither = false;

/*  need to do

    //Set Alpha Compare:
    if(modified & SMF_SOURCE2)
        MApi_GFX_SetAlphaCmp((MS_BOOL)sdrv->ge_render_alpha_cmp_enabled,
                             sdrv->ge_render_alpha_cmp );
    //SetYUV Format:
    MApi_GFX_SetDC_CSC_FMT( (GFX_YUV_Rgb2Yuv)dfb_config->mst_rgb2yuv_mode,
                                GFX_YUV_OUT_255, GFX_YUV_IN_255,
                                sdrv->src_ge_yuv_fmt,
                                sdrv->dst_ge_yuv_fmt);

    //Set Intensity Palette:
    if(sdrv->ge_palette_intensity_enabled       &&
       dfb_config->mst_blink_frame_rate == 0)
    {
        int i;
        for(i = 0; i<sdev->num_intensity; i++)
        {
            MApi_GFX_SetIntensity( i,
                                   GFX_FMT_ARGB8888,
                                   (MS_U32*)&sdev->palette_intensity[i]);
        }
    }
 */
}

static bool
FindDummyColor( CorePalette         *palette,
                DFBColor            *dummy_entry,
                DFBColor             ori_color,
                int                  cubeSize )
{
    /*
    * search reversely to prevent from the overkill issue during StretchBlit
      (hw limitation: just has 4 bit precision)
    * Fix the bug 0873569: Invalid conversion to clear color in StretchBlit from LUT8 to ARGB
    */

    #define  UNITCUBESIZE  8   // the dimension of cubic is 8x8x8

    const int num_entries = ( palette->num_entries > MAX_PALETTE_ENTRY_CNT ? MAX_PALETTE_ENTRY_CNT : palette->num_entries);

    const DFBColor *palette_color = palette->entries;

    D_ASSERT(cubeSize > 0);

    // in pixel
    const int cubeEdge = (256/cubeSize);

    bool bCube[UNITCUBESIZE][UNITCUBESIZE][UNITCUBESIZE] = {false};

    bool bCheck = false;

    int r = 0, g = 0, b = 0, i = 0;

    // set a default color to the output
    dummy_entry->a = 255;
    dummy_entry->r = 0;
    dummy_entry->g = 0;
    dummy_entry->b = 0;


    // calculate the current pixel belongs to which "Cubic (grid)"
    for ( i = 0; i < num_entries; i++ )
    {
        int index_R, index_G, index_B;

        index_R = (int)(palette_color[i].r / cubeEdge);
        index_G = (int)(palette_color[i].g / cubeEdge);
        index_B = (int)(palette_color[i].b / cubeEdge);

        bCube[index_R][index_G][index_B] = true;
    }

    // try to find a un-used cubic and lets its center to be the dummy color
    for ( r = 0; r < cubeSize; r++)
    {
        if(bCheck)
            break;

        for ( g = 0; g < cubeSize; g++)
        {
            if(bCheck)
                break;

           for ( b = 0; b < cubeSize; b++)
           {
               if(bCube[r][g][b] == false)
               {
                   // calculate the cubic center
                   dummy_entry->r = r*cubeEdge + cubeEdge/2;
                   dummy_entry->g = g*cubeEdge + cubeEdge/2;
                   dummy_entry->b = b*cubeEdge + cubeEdge/2;

                   // set the terminated flag
                   bCheck = true;
                   break;
                }
            }
        }
    }

    D_INFO("[DFB][FindDummyColor] dummy_color : (A,R,G,B) = (%d,%d,%d,%d), cubeSize = %d\n",
            dummy_entry->a, dummy_entry->r, dummy_entry->g, dummy_entry->b, cubeSize);


    D_INFO("[DFB][FindDummyColor] color key : (A,R,G,B) = (%d,%d,%d,%d)\n",
            ori_color.a, ori_color.r, ori_color.g, ori_color.b);

    return bCheck;

}


static void
mstarSetState( void                         *drv,
               void                         *dev,
               GraphicsDeviceFuncs          *funcs,
               CardState                    *state,
               DFBAccelerationMask           accel )
{
    MSTARDeviceData         *sdev = dev;
    MSTARDriverData         *sdrv = drv;

    CoreSurface             *dsurface = state->destination;
    DFBAccelerationMask      support_accel = state->accel & accel;

    CorePalette             *palette = NULL;
    CorePalette             *palette_intensity = NULL;
    DFBSurfacePixelFormat    palette_intensity_format = DSPF_LUT4;

    StateModificationFlags   modified = state->mod_hw;

    u32 max_palette_intensity_cnt = MAX_PALETTE_INTENSITY_I4_CNT;

    sdrv->state = state;

    state->set = DFXL_NONE;

    if(support_accel == DFXL_NONE)
    {
        printf("[DFB] Serious warning, rejet mstarSetState\n");
        return;
    }

    if( DFB_BLITTING_FUNCTION(support_accel)                    &&
        (sdrv->ge_last_render_op != MSTAR_GFX_RENDER_OP_BLIT) )
    {
        modified |= ( SMF_SOURCE            |
                      SMF_BLITTING_FLAGS    |
                      SMF_SRC_COLORKEY      |
                      SMF_DST_COLORKEY      |
                      SMF_SRC_BLEND         |
                      SMF_DST_BLEND         |
                      SMF_BLIT_XMIRROR      |
                      SMF_BLIT_YMIRROR      |
                      SMF_BLIT_NEARESTMODE  |
                      SMF_SRC_CONVOLUTION  |
                      SMF_BLINK             |
                      SMF_WRITE_MASK_BITS );
    }
    else if( DFB_DRAWING_FUNCTION(support_accel)                &&
             (sdrv->ge_last_render_op != MSTAR_GFX_RENDER_OP_DRAW) )
    {
        modified |= ( SMF_DRAWING_FLAGS     |
                      SMF_SRC_COLORKEY      |
                      SMF_DST_COLORKEY      |
                      SMF_SRC_BLEND         |
                      SMF_DST_BLEND );
    }

    if((modified & SMF_CLIP) && sdev->b_hwclip)
    {
       sdrv->clip = state->clip;
    }

    if(modified & SMF_DESTINATION)
    {
        sdrv->ge_dest_rgb32_blit_enabled = 0;

        switch( state->dst.buffer->format )
        {
            case DSPF_ARGB1555:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_ARGB1555;
                    break;

            case DSPF_ARGB:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                    break;

            case DSPF_ABGR:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_ABGR8888;
                    break;

            case DSPF_YVYU:
                    sdrv->dst_ge_yuv_fmt    = E_MI_OSD_YUV422_YVYU;
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_YUV422_YVYU;
                    break;

            case DSPF_YUY2:
                    sdrv->dst_ge_yuv_fmt    = E_MI_OSD_YUV422_YUYV;
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_YUV422_YUYV;
                    break;

            case DSPF_UYVY:
                    sdrv->dst_ge_yuv_fmt    = E_MI_OSD_YUV422_UYVY;
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_YUV422_UYVY;
                    break;

            case DSPF_LUT8:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_I8;
                    palette                 = dsurface->palette;
                    break;

            case DSPF_ARGB4444:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_ARGB4444;
                    break;

            case DSPF_RGB16:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_RGB565;
                    break;

            case DSPF_A8:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_I8;
                    break;

            case DSPF_RGB32:

                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                    sdrv->ge_dest_rgb32_blit_enabled = 1;
                    break;

            case DSPF_BLINK12355:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_1ABFGBG12355;
                    break;

            case DSPF_BLINK2266:
                    sdrv->dst_ge_format     = E_MI_OSD_COLOR_FORMAT_FABAFGBG2266;
                    break;

            default:
                printf( "[DFB] error format : %s (%s, %s)\n",
                    dfb_pixelformat_name(state->dst.buffer->format), __FILE__, __FUNCTION__ );

                return;
        }

        if(dfb_config->bUsingHWTLB)
        {
            sdrv->dst_phys = state->dst.phys;
        }
        else
        {
            sdrv->dst_phys = _BusAddrToHalAddr(((u64)state->dst.phys_h << 32) | state->dst.phys);
        }

        sdrv->dst_w     = dsurface->config.size.w;
        sdrv->dst_h     = dsurface->config.size.h;
        sdrv->dst_bpp   = DFB_BYTES_PER_PIXEL( state->dst.buffer->format );
        sdrv->dst_pitch = state->dst.pitch;
    }

    if(modified & SMF_COLOR)
    {
        sdrv->color = state->color;
        sdrv->color2 = state->color2;

        if(E_MI_OSD_COLOR_FORMAT_I8 == sdrv->dst_ge_format)
            sdrv->color.b = state->color_index;
    }

    if(modified & SMF_BLIT_NEARESTMODE)
        sdrv->ge_blit_nearestmode_enabled = state->blit_nearestmode_enabled;

    if(modified & SMF_SRC_CONVOLUTION)
        sdrv->ge_blit_nearestmode_enabled = state->blit_nearestmode_enabled;

    if(modified & SMF_BLIT_XMIRROR)
        sdrv->ge_blit_xmirror = state->blit_xmirror_enabled;

    if(modified & SMF_BLIT_YMIRROR)
        sdrv->ge_blit_ymirror = state->blit_ymirror_enabled;

    if(modified & SMF_SOURCE2)
    {
        switch(state->cmp_mode)
        {
            case DSBF_ACMP_OP_MAX:
            case DSBF_ACMP_OP_MIN:
                    sdrv->ge_render_alpha_cmp = (u32)(state->cmp_mode-1);
                    sdrv->ge_render_alpha_cmp_enabled = 1;
                    break;

            default:
                    sdrv->ge_render_alpha_cmp_enabled = 0;
                    break;
        }
    }

    sdrv->ge_palette_enabled = 0;

    if( DFB_DRAWING_FUNCTION(support_accel) )
    {
        DFBSurfaceBlittingFlags blittingflags = DSBLIT_NOFX;

        if(modified & SMF_DRAWING_FLAGS)
            sdrv->dflags = state->drawingflags;

        if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY))
        {
            sdrv->ge_draw_dst_colorkey_enabled =
                    ((state->drawingflags & DSDRAW_DST_COLORKEY) ? 1 : 0);

            sdrv->dst_ge_clr_key = mstar_pixel_to_colorkey( state->dst.buffer->format,
                                                            state->dst.buffer->format,
                                                            state->dst_colorkey,
                                                            palette );
        }

        if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
        {
            if(state->drawingflags & DSDRAW_BLEND)
                blittingflags |= DSBLIT_BLEND_ALPHACHANNEL;

            if(state->drawingflags & DSDRAW_SRC_PREMULTIPLY)
                blittingflags |= DSBLIT_SRC_PREMULTIPLY;

            if(state->drawingflags & DSDRAW_DST_PREMULTIPLY)
                blittingflags |= DSBLIT_DST_PREMULTIPLY;

            if(state->drawingflags & DSDRAW_DEMULTIPLY)
                blittingflags |= DSBLIT_DEMULTIPLY;

            if(state->drawingflags & DSDRAW_XOR)
                blittingflags |= DSBLIT_XOR;

            sdrv->ge_draw_alpha_blend_enabled =
                            mstarMapGFXBlitOption( blittingflags,
                                                   state->src_blend,
                                                   state->dst_blend,
                                                   &sdrv->ge_src_blend,
                                                   &sdrv->ge_dst_blend,
                                                   &sdrv->ge_draw_bld_flags);
        }
    }

    if( DFB_BLITTING_FUNCTION(support_accel) )
    {
        CoreSurface *ssurface = state->source;

        if(modified & SMF_BLITTING_FLAGS)
            sdrv->bflags = state->blittingflags;

        if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY))
        {
            sdrv->ge_blit_dst_colorkey_enabled =
                        ((state->blittingflags & DSBLIT_DST_COLORKEY) ? 1 : 0);

            sdrv->dst_ge_clr_key = mstar_pixel_to_colorkey( state->dst.buffer->format,
                                                            state->dst.buffer->format,
                                                            state->dst_colorkey,
                                                            palette);
        }

        if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
        {
            sdrv->ge_blit_alpha_blend_enabled =
                        mstarMapGFXBlitOption( state->blittingflags,
                                               state->src_blend,
                                               state->dst_blend,
                                               &sdrv->ge_src_blend,
                                               &sdrv->ge_dst_blend,
                                               &sdrv->ge_blit_bld_flags );
        }

        if(modified & SMF_SOURCE)
        {
            sdrv->src_w = ssurface->config.size.w;
            sdrv->src_h = ssurface->config.size.h;

            if(dfb_config->bUsingHWTLB)
            {
                sdrv->src_phys = state->src.phys;
            }
            else
            {
                sdrv->src_phys = _BusAddrToHalAddr(((u64)state->src.phys_h << 32) | state->src.phys);
            }

            sdrv->ge_src_rgb32_blit_enabled = 0;

            switch( state->src.buffer->format )
            {
                case DSPF_ARGB1555:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_ARGB1555;
                        break;

                case DSPF_ARGB:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                        break;

                case DSPF_ABGR:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_ABGR8888;
                        break;

                case DSPF_YVYU:
                        sdrv->src_ge_yuv_fmt = E_MI_OSD_YUV422_YVYU;
                        sdrv->src_ge_format  = E_MI_OSD_COLOR_FORMAT_YUV422_YVYU;
                        sdrv->stBlitOpt.eRgb2YuvMode = (ssurface->config.rgb2yuvMode == 0) ? E_MI_OSD_RGB_TO_YUV_LIMITED_RANGE : E_MI_OSD_RGB_TO_YUV_FULL_RANGE;
                        break;

                case DSPF_YUY2:
                        sdrv->src_ge_yuv_fmt = E_MI_OSD_YUV422_YUYV;
                        sdrv->src_ge_format  = E_MI_OSD_COLOR_FORMAT_YUV422_YUYV;
                        sdrv->stBlitOpt.eRgb2YuvMode = (ssurface->config.rgb2yuvMode == 0) ? E_MI_OSD_RGB_TO_YUV_LIMITED_RANGE : E_MI_OSD_RGB_TO_YUV_FULL_RANGE;
                        break;

                case DSPF_UYVY:
                        sdrv->src_ge_yuv_fmt = E_MI_OSD_YUV422_UYVY;
                        sdrv->src_ge_format  = E_MI_OSD_COLOR_FORMAT_YUV422_UYVY;
                        sdrv->stBlitOpt.eRgb2YuvMode = (ssurface->config.rgb2yuvMode == 0) ? E_MI_OSD_RGB_TO_YUV_LIMITED_RANGE : E_MI_OSD_RGB_TO_YUV_FULL_RANGE;
                        break;

                case DSPF_LUT8:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_I8;

                        if( NULL == palette )
                            palette = ssurface->palette;

                        break;

                case DSPF_LUT4:
                        palette_intensity_format    = DSPF_LUT4;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I4_CNT;
                        sdrv->src_ge_format         = E_MI_OSD_COLOR_FORMAT_I4;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT2:
                        palette_intensity_format    = DSPF_LUT2;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I2_CNT;
                        sdrv->src_ge_format         = E_MI_OSD_COLOR_FORMAT_I2;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT1:
                        palette_intensity_format    = DSPF_LUT1;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I1_CNT;
                        sdrv->src_ge_format         = E_MI_OSD_COLOR_FORMAT_I1;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_ARGB4444:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_ARGB4444;
                        break;

                case DSPF_RGB16:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_RGB565;
                        break;

                case DSPF_A8:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_I8;
                        break;

                case DSPF_RGB32:
                        sdrv->ge_src_rgb32_blit_enabled = 1;
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                        break;

            case DSPF_BLINK12355:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_1ABFGBG12355;
                        break;

            case DSPF_BLINK2266:
                        sdrv->src_ge_format = E_MI_OSD_COLOR_FORMAT_FABAFGBG2266;
                        break;
                default:
                        printf( "[DFB] error format : %s (%s, %s)\n",
                                 dfb_pixelformat_name(state->src.buffer->format),
                                __FILE__,
                                __FUNCTION__ );

                        return;
            }

            sdrv->src_bpp    = DFB_BYTES_PER_PIXEL(state->src.buffer->format);
            sdrv->src_pitch  = state->src.pitch;
        }

        // For set the ge_palette_enabled when src_format=DSPF_A8, Roy modified
        //if(DSPF_A8 == state->src.buffer->format && (!sdev->a8_palette))
        if(DSPF_A8 == state->src.buffer->format)
        {
            int i = 0;

            if( NULL != dsurface->palette )
            {
                printf( "[DFB] error format %s:%d, 0x%08X\n",
                            __FILE__, __LINE__, state->src.buffer->format );
                return;
            }

            memset(sdev->palette_tbl, 0xFF, (sizeof(MI_OSD_RgbColor_t)*MAX_PALETTE_ENTRY_CNT));

            for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
            {
                sdev->palette_tbl[i].u8Alpha = i;
            }

            sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
            sdrv->ge_palette_enabled = 1;
            sdev->a8_palette = true;
        }

        if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY))
        {
            sdrv->ge_src_colorkey_enabled = ((state->blittingflags & DSBLIT_SRC_COLORKEY) ? 1 : 0);

            /*
                default case    : src_color_key_index_patch = 1
                original  case  : src_color_key_index_patch = 0
            */

            if (dfb_config->src_color_key_index_patch == 1)
                sdrv->src_ge_clr_key = mstar_pixel_to_colorkey( state->src.buffer->format,
                                                                state->dst.buffer->format,
                                                                state->src_colorkey,
                                                                ssurface->palette );
            else
                sdrv->src_ge_clr_key = mstar_pixel_to_colorkey( state->src.buffer->format,
                                                                state->src.buffer->format,
                                                                state->src_colorkey,
                                                                ssurface->palette);
        }

        if(modified & SMF_WRITE_MASK_BITS)
        {
            sdrv->ge_rop_enable = TRUE;
            switch(state->rop_op)
            {
                case DFB_ROP2_OP_ZERO:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_ZERO;
                    break;

                case DFB_ROP2_NOT_PS_OR_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NOT_PS_OR_PD;
                    break;

                case DFB_ROP2_NS_AND_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NS_AND_PD;
                    break;

                case DFB_ROP2_NS:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NS;
                    break;

                case DFB_ROP2_PS_AND_ND:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PS_AND_ND;
                    break;

                case DFB_ROP2_ND:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_ND;
                    break;

                case DFB_ROP2_PS_XOR_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PS_XOR_PD;
                    break;

                case DFB_ROP2_NOT_PS_AND_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NOT_PS_AND_PD;
                    break;

                case DFB_ROP2_PS_AND_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PS_AND_PD;
                    break;

                case DFB_ROP2_NOT_PS_XOR_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NOT_PS_XOR_PD;
                    break;

                case DFB_ROP2_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PD;
                    break;

                case DFB_ROP2_NS_OR_PD:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NS_OR_PD;
                    break;

                case DFB_ROP2_PS:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PS;
                    break;

                case DFB_ROP2_PS_OR_ND:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PS_OR_ND;
                    break;

                case DFB_ROP2_PD_OR_PS:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_PD_OR_PS;
                    break;

                case DFB_ROP2_ONE:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_ONE;
                    break;

                case DFB_ROP2_NONE:
                    //disable ROP option
                    //gfx enum no DFB_ROP2_NONE option
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NONE;
                    sdrv->ge_rop_enable = FALSE;
                    break;

                default:
                    sdrv->ge_rop_op = E_MI_OSD_ROP_NONE;
                    sdrv->ge_rop_enable = FALSE;
                    return;
            }
        }
    }

    if(palette)
    {
        int i = 0;
        int num_entries = 0;
        num_entries = (palette->num_entries > MAX_PALETTE_ENTRY_CNT ? MAX_PALETTE_ENTRY_CNT : palette->num_entries);

        for( i = 0; i < num_entries; ++i )
        {
            sdev->palette_tbl[i].u8Alpha = palette->entries[i].a;
            sdev->palette_tbl[i].u8Red = palette->entries[i].r;
            sdev->palette_tbl[i].u8Green = palette->entries[i].g;
            sdev->palette_tbl[i].u8Blue = palette->entries[i].b;
        }

        sdev->num_entries        = num_entries;
        sdrv->ge_palette_enabled = 1;
        sdev->a8_palette         = false;

        bool bCheckFormat = false;
        if ( NULL == state->src.buffer || NULL == state->dst.buffer)
        {
            bCheckFormat = false;
        }
        else
        {
            bCheckFormat =
                DFB_PIXELFORMAT_IS_INDEXED(state->src.buffer->format) != 0 &&
                DFB_PIXELFORMAT_IS_INDEXED(state->dst.buffer->format) == 0;
        }

        if ( dfb_config->src_color_key_index_patch == 1     &&
             sdrv->ge_src_colorkey_enabled                  &&
             bCheckFormat )
        {
            DFBColor color_dummy, color_original;
            u32 clr_key_index = state->src_colorkey;

            color_original.a = sdev->palette_tbl[clr_key_index].u8Alpha;
            color_original.r = sdev->palette_tbl[clr_key_index].u8Red;
            color_original.g = sdev->palette_tbl[clr_key_index].u8Green;
            color_original.b = sdev->palette_tbl[clr_key_index].u8Blue;

            // Find a dummy color
            bool ret = false;
            int cubeSize = 6;

            for ( cubeSize = 6; ret == false && cubeSize < 9; cubeSize++)
            {
                ret = FindDummyColor( palette,
                                      &color_dummy,
                                      color_original,
                                      cubeSize );
            }

            // Modify Palette Table & assign a dummy color key
            sdev->palette_tbl[clr_key_index].u8Alpha = color_dummy.a;
            sdev->palette_tbl[clr_key_index].u8Red = color_dummy.r;
            sdev->palette_tbl[clr_key_index].u8Green = color_dummy.g;
            sdev->palette_tbl[clr_key_index].u8Blue = color_dummy.b;

            sdrv->src_ge_clr_key = color_dummy.a << 24 |
                                   color_dummy.r << 16 |
                                   color_dummy.g <<  8 |
                                   color_dummy.b;
        }
    }

    if(palette_intensity)
    {
        int i = 0;
        int num_entries = 0;
        u32 color = 0;
        num_entries = (palette_intensity->num_entries > max_palette_intensity_cnt ? max_palette_intensity_cnt : palette_intensity->num_entries);

        for( i = 0; i < num_entries; ++i )
        {
            color = mstar_pixel_to_color( palette_intensity_format,
                                                               i,
                                                               palette_intensity);

            sdev->palette_intensity[i].u8Alpha = (color >>24);
            sdev->palette_intensity[i].u8Red = (color >>16)&0xFF;
            sdev->palette_intensity[i].u8Green = (color >>8)&0xFF;
            sdev->palette_intensity[i].u8Blue = (color)&0xFF;
        }

        sdev->num_intensity = num_entries;
        sdrv->ge_palette_intensity_enabled = 1;
    }

    state->mod_hw = 0;
    state->set = support_accel & ( DFXL_FILLRECTANGLE   |
                                   DFXL_DRAWLINE        |
                                   DFXL_DRAWRECTANGLE   |
                                   DFXL_FILLTRAPEZOID   |
                                   DFXL_FILLQUADRANGLE        |
                                   DFXL_BLIT            |
                                   DFXL_STRETCHBLIT     |
                                   DFXL_BLIT2 );

    MI_OSD_BeginDraw();

    bool bCheckFormat = false;
    if ( NULL == state->src.buffer || NULL == state->dst.buffer)
        bCheckFormat = false;
    else
        bCheckFormat = true;

    if ( bCheckFormat                                                       &&
         dfb_config->mst_dither_enable                                      &&
         (DFB_BITS_PER_PIXEL(state->src.buffer->format)     >
            DFB_BITS_PER_PIXEL(state->dst.buffer->format))                  &&
         (DFB_PIXELFORMAT_IS_INDEXED(state->src.buffer->format) == false)   &&
         (DFB_PIXELFORMAT_IS_INDEXED(state->dst.buffer->format) == false) )
    {
        D_INFO("[DFB] enable dither! \n");
        sdrv->ge_dither_enable = true;
    }
    else
    {
        sdrv->ge_dither_enable = false;
    }

    mstarSetEngineShareState(modified, support_accel, sdrv, sdev);

    if(DFB_BLITTING_FUNCTION(support_accel))
    {
        mstarSetEngineBlitState(modified, sdrv, sdev);
        sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_BLIT;

        /* Prevention mantis:1178283 */
        /* Whether src pixel format is not YUV, dest pixel format is YUV */
        if ((modified & SMF_DESTINATION)                &&
            (modified & SMF_SOURCE)                     &&
            (sdrv->src_ge_format != E_MI_OSD_COLOR_FORMAT_YUV422_YVYU &&
             sdrv->src_ge_format != E_MI_OSD_COLOR_FORMAT_YUV422_YUYV &&
             sdrv->src_ge_format != E_MI_OSD_COLOR_FORMAT_YUV422_UYVY) &&
            (sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YVYU ||
             sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YUYV ||
             sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_UYVY))
        {
            D_INFO("\33[0;33;44m[DFB][WARNING!!]\33[0m %s to YUV, it might cause precision "\
                    "loss in image quality: src(%d ,%d) -> dest(%d,%d) (pid=%d)",
                    dfb_pixelformat_name(state->src.buffer->format), sdrv->src_w, sdrv->src_h,
                    sdrv->dst_w, sdrv->dst_h, sdrv->state->destination->object.PID);
        }
    }
    else if(DFB_DRAWING_FUNCTION(support_accel))
    {
        mstarSetEngineDrawState(modified, sdrv, sdev);
        sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_DRAW;
    }

    MI_OSD_EndDraw();

}


static void
mstarSetState_null( void                        *drv,
                    void                        *dev,
                    GraphicsDeviceFuncs         *funcs,
                    CardState                   *state,
                    DFBAccelerationMask          accel )
{

}


static bool
mstarFillRectangle_null( void                   *drv,
                         void                   *dev,
                         DFBRectangle           *rect )
{
    return true;
}


static bool
mstarFillRectangle( void                        *drv,
                    void                        *dev,
                    DFBRectangle                *rect )
{
    MI_RESULT eRet = MI_OK;
    bool ret = true;
    MSTARDriverData *sdrv = drv;
    MI_OSD_RectAttr_t stRectAttr = {0};
    u32 y = 0, cb = 0, cr = 0;

    stRectAttr.stDstRect.u32X         = rect->x;
    stRectAttr.stDstRect.u32Y         = rect->y;
    stRectAttr.stDstRect.u32Width     = rect->w;
    stRectAttr.stDstRect.u32Height    = rect->h;

    switch(sdrv->dst_ge_format)
    {
        case E_MI_OSD_COLOR_FORMAT_YUV422_YVYU:
        case E_MI_OSD_COLOR_FORMAT_YUV422_YUYV:
        case E_MI_OSD_COLOR_FORMAT_YUV422_UYVY:
                RGB_TO_YCBCR( sdrv->color.r,
                              sdrv->color.g,
                              sdrv->color.b,
                              y,
                              cb,
                              cr );

                stRectAttr.stStartColor.u8Alpha = 0xFF;
                stRectAttr.stStartColor.u8Red = cr;
                stRectAttr.stStartColor.u8Green = y;
                stRectAttr.stStartColor.u8Blue = cb;
                break;

        default:
                stRectAttr.stStartColor.u8Alpha= sdrv->color.a;
                stRectAttr.stStartColor.u8Red = sdrv->color.r;
                stRectAttr.stStartColor.u8Green = sdrv->color.g;
                stRectAttr.stStartColor.u8Blue = sdrv->color.b;
                break;
    }

    if(sdrv->dflags & (DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(sdrv->dst_ge_format)
        {
            case E_MI_OSD_COLOR_FORMAT_YUV422_YVYU:
            case E_MI_OSD_COLOR_FORMAT_YUV422_YUYV:
            case E_MI_OSD_COLOR_FORMAT_YUV422_UYVY:
                    RGB_TO_YCBCR( sdrv->color2.r,
                                  sdrv->color2.g,
                                  sdrv->color2.b,
                                  y,
                                  cb,
                                  cr );

                    stRectAttr.stEndColor.u8Alpha = 0xFF;
                    stRectAttr.stEndColor.u8Red= cr;
                    stRectAttr.stEndColor.u8Green = y;
                    stRectAttr.stEndColor.u8Blue = cb;
                    break;

            default:
                    stRectAttr.stEndColor.u8Alpha = sdrv->color2.a;
                    stRectAttr.stEndColor.u8Red = sdrv->color2.r;
                    stRectAttr.stEndColor.u8Green = sdrv->color2.g;
                    stRectAttr.stEndColor.u8Blue = sdrv->color2.b;
                    break;
        }

        if( (sdrv->dflags & DSDRAW_COLOR_GRADIENT_X) &&  (sdrv->dflags & DSDRAW_COLOR_GRADIENT_Y))
            stRectAttr.eColorGradient = E_MI_OSD_COLOR_GRADIENT_HORIZONTAL_VERTICAL;
        else if( (sdrv->dflags & DSDRAW_COLOR_GRADIENT_X))
            stRectAttr.eColorGradient = E_MI_OSD_COLOR_GRADIENT_HORIZONTAL;
        else if( (sdrv->dflags & DSDRAW_COLOR_GRADIENT_Y))
            stRectAttr.eColorGradient = E_MI_OSD_COLOR_GRADIENT_VERTICAL;
        else
            stRectAttr.eColorGradient = E_MI_OSD_COLOR_GRADIENT_NONE;

    }
    else
       stRectAttr.eColorGradient = E_MI_OSD_COLOR_GRADIENT_NONE;

    MI_OSD_BeginDraw();
    eRet = MI_OSD_FillRect(sdrv->hDstSurface, &stRectAttr, &sdrv->stBlitOpt, &sdrv->stRenderJob);
    if(eRet != MI_OK)
    {
        ret = false;
        D_ERROR("[DFB][%s] MI_OSD_FillRect failed! eRet = %d\n", __FUNCTION__, eRet);
    }
    MI_OSD_EndDraw();

    return ret;
}


static bool
mstarFillRectangle_sw( void                     *drv,
                       void                     *dev,
                       DFBRectangle             *rect )
{


    if (dfb_config->sw_render & SWRF_FILLRECT)
        return false;
    else
        return mstarFillRectangle(drv, dev, rect);
}


//#if defined(USE_GFX_EXTENSION)
#if 0
bool
mstarFillTriangleEx( void                       *drv,
                     void                       *dev,
                     DFBTriangle                *tri,
                     DFBRegion                  *clip )
{
    MSTARDeviceData *sdev = dev;
    MSTARDriverData *sdrv = drv;

    GFX_TriFillInfo  triInfo;

    u32 y=0, cb=0, cr=0;

    triInfo.tri.x0 = tri->x1;
    triInfo.tri.y0 = tri->y1;
    triInfo.tri.x1 = tri->x2;
    triInfo.tri.y1 = tri->y2;
    triInfo.tri.x2 = tri->x3;
    triInfo.tri.y2 = tri->y3;

    triInfo.clip_box.x = clip->x1;
    triInfo.clip_box.y = clip->y1;

    triInfo.clip_box.width  = (clip->x2 > clip->x1) ?
                              (clip->x2 - clip->x1 + 1) : (clip->x1 - clip->x2 + 1);

    triInfo.clip_box.height = (clip->y2 > clip->y1) ?
                              (clip->y2 - clip->y1 + 1) : (clip->y1 - clip->y2 + 1);

    triInfo.fmt = sdrv->dst_ge_format;

    switch(triInfo.fmt)
    {
        case GFX_FMT_YUV422:
                RGB_TO_YCBCR( sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);

                triInfo.colorRange.color_s.a = 0xFF;
                triInfo.colorRange.color_s.r = cr;
                triInfo.colorRange.color_s.g = y;
                triInfo.colorRange.color_s.b = cb;

                break;

        default:
                triInfo.colorRange.color_s.a = sdrv->color.a;
                triInfo.colorRange.color_s.r = sdrv->color.r;
                triInfo.colorRange.color_s.g = sdrv->color.g;
                triInfo.colorRange.color_s.b = sdrv->color.b;
                break;
    }

    triInfo.flag = 0;

    if(sdrv->dflags & (DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(triInfo.fmt)
        {
            case GFX_FMT_YUV422:
                   RGB_TO_YCBCR( sdrv->color2.r,
                                 sdrv->color2.g,
                                 sdrv->color2.b,
                                 y,
                                 cb,
                                 cr );

                   triInfo.colorRange.color_e.a = 0xFF;
                   triInfo.colorRange.color_e.r = cr;
                   triInfo.colorRange.color_e.g = y;
                   triInfo.colorRange.color_e.b = cb;
                   break;

            default:
                   triInfo.colorRange.color_e.a = sdrv->color2.a;
                   triInfo.colorRange.color_e.r = sdrv->color2.r;
                   triInfo.colorRange.color_e.g = sdrv->color2.g;
                   triInfo.colorRange.color_e.b = sdrv->color2.b;
                   break;
        }

        if(sdrv->dflags & DSDRAW_COLOR_GRADIENT_X)
            triInfo.flag = GFXRECT_FLAG_COLOR_GRADIENT_X;

        if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            triInfo.flag |= GFXRECT_FLAG_COLOR_GRADIENT_Y;
    }


    MApi_GFX_BeginDraw();

    MApi_GFX_TriFill(&triInfo);
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));

    MApi_GFX_EndDraw();

    return true;
}

bool
mstarFillSpan( void                 *drv,
               void                 *dev,
               DFBSpan              *span,
               int                   num_spans,
               int                   y_offset,
               DFBRegion            *clip )
{
    MSTARDeviceData *sdev = dev;
    MSTARDriverData *sdrv = drv;
    GFX_SpanFillInfo spanInfo;

    u32 y=0, cb=0, cr=0;

    spanInfo.span.y         = y_offset;
    spanInfo.span.spans     = span;
    spanInfo.span.num_spans = num_spans;

    spanInfo.clip_box.x = clip->x1;
    spanInfo.clip_box.y = clip->y1;

    spanInfo.clip_box.width  = (clip->x2 > clip->x1) ?
                               (clip->x2 - clip->x1 + 1) : (clip->x1 - clip->x2 + 1);

    spanInfo.clip_box.height = (clip->y2 > clip->y1) ?
                               (clip->y2 - clip->y1 + 1) : (clip->y1 - clip->y2 + 1);

    spanInfo.fmt = sdrv->dst_ge_format;

    switch(spanInfo.fmt)
    {
        case GFX_FMT_YUV422:
                RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
                spanInfo.colorRange.color_s.a = 0xFF;
                spanInfo.colorRange.color_s.r = cr;
                spanInfo.colorRange.color_s.g = y;
                spanInfo.colorRange.color_s.b = cb;
                break;

        default:
                spanInfo.colorRange.color_s.a = sdrv->color.a;
                spanInfo.colorRange.color_s.r = sdrv->color.r;
                spanInfo.colorRange.color_s.g = sdrv->color.g;
                spanInfo.colorRange.color_s.b = sdrv->color.b;
                break;
    }

    spanInfo.flag = 0;

    if(sdrv->dflags &(DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(spanInfo.fmt)
        {
            case GFX_FMT_YUV422:
                    RGB_TO_YCBCR( sdrv->color2.r,
                                  sdrv->color2.g,
                                  sdrv->color2.b,
                                  y,
                                  cb,
                                  cr);

                    spanInfo.colorRange.color_e.a = 0xFF;
                    spanInfo.colorRange.color_e.r = cr;
                    spanInfo.colorRange.color_e.g = y;
                    spanInfo.colorRange.color_e.b = cb;
                    break;

            default:
                    spanInfo.colorRange.color_e.a = sdrv->color2.a;
                    spanInfo.colorRange.color_e.r = sdrv->color2.r;
                    spanInfo.colorRange.color_e.g = sdrv->color2.g;
                    spanInfo.colorRange.color_e.b = sdrv->color2.b;
                    break;
        }

        if(sdrv->dflags & DSDRAW_COLOR_GRADIENT_X)
            spanInfo.flag = GFXRECT_FLAG_COLOR_GRADIENT_X;

        if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            spanInfo.flag |= GFXRECT_FLAG_COLOR_GRADIENT_Y;
   }



    MApi_GFX_BeginDraw();
    MApi_GFX_SpanFill( &spanInfo );
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true;
}
#endif


static bool
mstarDrawRectangle_null( void               *driver_data,
                         void               *device_data,
                         DFBRectangle       *rect )
{
    return true;
}


static bool
mstarDrawRectangle( void                    *driver_data,
                    void                    *device_data,
                    DFBRectangle            *rect )
{
    MSTARDriverData *sdrv = driver_data;

    MI_OSD_LineAttr_t stLineAttr;

    memset(&stLineAttr,0x00,sizeof(MI_OSD_LineAttr_t));

    stLineAttr.stColor.u8Alpha  = sdrv->color.a;
    stLineAttr.stColor.u8Red  = sdrv->color.r;
    stLineAttr.stColor.u8Green = sdrv->color.g;
    stLineAttr.stColor.u8Blue = sdrv->color.b;

    stLineAttr.u32LineWidth = LINE_WIDTH;

    stLineAttr.u32X1 = rect->x;
    stLineAttr.u32Y1 = rect->y;
    stLineAttr.u32X2 = rect->x+rect->w - 1;
    stLineAttr.u32Y2 = rect->y;

    MI_OSD_BeginDraw();
    if(MI_OSD_DrawLine_EX)
        MI_OSD_DrawLine_EX(sdrv->hDstSurface, &stLineAttr, NULL, &sdrv->stBlitOpt);
    else
        MI_OSD_DrawLine(sdrv->hDstSurface, &stLineAttr, NULL);

    stLineAttr.u32X1 = rect->x + rect->w - 1;
    stLineAttr.u32Y2 = rect->y + rect->h - 1;

    if(MI_OSD_DrawLine_EX)
        MI_OSD_DrawLine_EX(sdrv->hDstSurface, &stLineAttr, NULL, &sdrv->stBlitOpt);
    else
        MI_OSD_DrawLine(sdrv->hDstSurface, &stLineAttr, NULL);

    stLineAttr.u32Y1 = rect->y + rect->h - 1;
    stLineAttr.u32X2 = rect->x;

    if(MI_OSD_DrawLine_EX)
        MI_OSD_DrawLine_EX(sdrv->hDstSurface, &stLineAttr, NULL, &sdrv->stBlitOpt);
    else
        MI_OSD_DrawLine(sdrv->hDstSurface, &stLineAttr, NULL);

    stLineAttr.u32X1 = rect->x;
    stLineAttr.u32Y2 = rect->y;

    if(MI_OSD_DrawLine_EX)
        MI_OSD_DrawLine_EX(sdrv->hDstSurface, &stLineAttr, &sdrv->stRenderJob, &sdrv->stBlitOpt);
    else
        MI_OSD_DrawLine(sdrv->hDstSurface, &stLineAttr, &sdrv->stRenderJob);

    MI_OSD_EndDraw();

    return true;
}


bool
mstarDrawRectangle_sw( void                 *driver_data,
                       void                 *device_data,
                       DFBRectangle         *rect )
{

    if (dfb_config->sw_render & SWRF_DRAWRECT)
        return false;
    else
        return mstarDrawRectangle(driver_data, device_data, rect);
}



bool
mstarDrawOval_null( void                *driver_data,
                    void                *device_data,
                    DFBOval             *oval )
{
    return true;
}


bool
mstarDrawOval( void                     *driver_data,
               void                     *device_data,
               DFBOval                  *oval )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;
    MI_OSD_OvalAttr_t ovalInfo;


    ovalInfo.stDstRect.u32X         = oval->oval_rect.x;
    ovalInfo.stDstRect.u32Y         = oval->oval_rect.y;
    ovalInfo.stDstRect.u32Width     = oval->oval_rect.w;
    ovalInfo.stDstRect.u32Height    = oval->oval_rect.h;

    //ovalInfo.u32LineWidth       = (u32)oval->line_width;

    MI_OSD_BeginDraw();

/*  ?? need to do ??
    if(oval->line_width > 0)
    {
        if(!DFB_COLOR_EQUAL(sdrv->color, oval->line_color))
        {
            //ovalInfo.u32LineWidth = 0;
            ovalInfo.stColor.u8Alpha = oval->line_color.a;
            ovalInfo.stColor.u8Red = oval->line_color.r;
            ovalInfo.stColor.u8Green = oval->line_color.g;
            ovalInfo.stColor.u8Blue = oval->line_color.b;

            MApi_GFX_DrawOval(&ovalInfo);
            ovalInfo.u32LineWidth = (u32)oval->line_width;
        }
        else
        {
            ovalInfo.u32LineWidth = 0;
        }
    }
*/
    ovalInfo.stColor.u8Alpha = sdrv->color.a;
    ovalInfo.stColor.u8Red = sdrv->color.r;
    ovalInfo.stColor.u8Green = sdrv->color.g;
    ovalInfo.stColor.u8Blue = sdrv->color.b;

    MI_OSD_DrawOval(sdrv->hDstSurface, &ovalInfo, &sdrv->stBlitOpt, &sdrv->stRenderJob);
    //MApi_GFX_DrawOval( &ovalInfo );
    //MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MI_OSD_EndDraw();

    return true;
}


bool
mstarDrawOval_sw( void                  *driver_data,
                  void                  *device_data,
                  DFBOval               *oval )
{
    if (dfb_config->sw_render & SWRF_DRAWOVAL)
        return false;
    else
        return mstarDrawOval(driver_data, device_data, oval);
}


bool
mstarDrawLine_null( void                *driver_data,
                    void                *device_data,
                    DFBRegion           *line,
                    unsigned int       width )
{
    return true;
}


bool
mstarDrawLine( void                     *driver_data,
               void                     *device_data,
               DFBRegion                *line,
               unsigned int           width )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    u32 y=0, cb=0, cr=0;
    MI_OSD_LineAttr_t stLineAttr;

    memset(&stLineAttr,0x00,sizeof(MI_OSD_LineAttr_t));

    stLineAttr.u32X1 = line->x1;
    stLineAttr.u32Y1 = line->y1;
    stLineAttr.u32X2 = line->x2;
    stLineAttr.u32Y2 = line->y2;

    stLineAttr.stColor.u8Alpha  = sdrv->color.a;
    stLineAttr.stColor.u8Red  = sdrv->color.r;
    stLineAttr.stColor.u8Green = sdrv->color.g;
    stLineAttr.stColor.u8Blue = sdrv->color.b;

    stLineAttr.u32LineWidth = width;

    switch(sdrv->dst_ge_format)
    {
        case E_MI_OSD_COLOR_FORMAT_YUV422_YVYU:
        case E_MI_OSD_COLOR_FORMAT_YUV422_YUYV:
        case E_MI_OSD_COLOR_FORMAT_YUV422_UYVY:
            RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
            stLineAttr.stColor.u8Alpha= 0xFF;
            stLineAttr.stColor.u8Red = cr;
            stLineAttr.stColor.u8Green = y;
            stLineAttr.stColor.u8Blue = cb;
            break;

        default:
            stLineAttr.stColor.u8Alpha = sdrv->color.a;
            stLineAttr.stColor.u8Red = sdrv->color.r;
            stLineAttr.stColor.u8Green = sdrv->color.g;
            stLineAttr.stColor.u8Blue= sdrv->color.b;
            break;
    }

    MI_OSD_BeginDraw();

    if(MI_OSD_DrawLine_EX)
        MI_OSD_DrawLine_EX(sdrv->hDstSurface, &stLineAttr, &sdrv->stRenderJob, &sdrv->stBlitOpt);
    else
        MI_OSD_DrawLine(sdrv->hDstSurface, &stLineAttr, &sdrv->stRenderJob);

    MI_OSD_EndDraw();

    return true;
}


bool
mstarDrawLine_sw( void                  *driver_data,
                  void                  *device_data,
                  DFBRegion             *line,
                  unsigned int         width )
{
    if (dfb_config->sw_render & SWRF_DRAWLINE)
        return false;
    else
        return mstarDrawLine(driver_data, device_data, line, width);
}


bool
mstarBlit_null( void                    *driver_data,
                void                    *device_data,
                DFBRectangle            *rect,
                int                      dx,
                int                      dy )
{
    return true;
}


bool
mstarBlit( void                         *driver_data,
           void                         *device_data,
           DFBRectangle                 *rect,
           int                           dx,
           int                           dy )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    MI_OSD_Rect_t stSrcRect;
    MI_OSD_Rect_t stDstRect;

    stSrcRect.u32X = rect->x;
    stSrcRect.u32Y = rect->y;
    stSrcRect.u32Width = rect->w;
    stSrcRect.u32Height = rect->h;

    stDstRect.u32X = dx;
    stDstRect.u32Y = dy;
    stDstRect.u32Width = rect->w;
    stDstRect.u32Height = rect->h;

    MI_OSD_BeginDraw();
    MI_OSD_Bitblit(sdrv->hSrcSurface, &stSrcRect, sdrv->hDstSurface, &stDstRect, &sdrv->stBlitOpt, &sdrv->stRenderJob);
    MI_OSD_EndDraw();

    return true;
}


bool
mstarBlit_sw( void                      *driver_data,
              void                      *device_data,
              DFBRectangle              *rect,
              int                        dx,
              int                        dy )
{
    if (dfb_config->sw_render & SWRF_BLIT)
        return false;
    else
        return mstarBlit(driver_data, device_data, rect, dx, dy);
}


bool
mstarBlit_GLES2( void                       *driver_data,
                 void                       *device_data,
                 CardState                  *state,
                 DFBRectangle               *srect,
                 DFBRectangle               *drect )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    bool ret = true;

    int src_w = state->source->config.size.w;
    int src_h = state->source->config.size.h;
    int dst_w = state->destination->config.size.w;
    int dst_h = state->destination->config.size.h;

    if ( GLESInitFuncs  == NULL )
    {
        printf("[DFB][%s] GL Init Failed!!\n", __FUNCTION__);
        ret = false;
    }
    else
    {
#define SHIFT_32 32      
        GLESBlitInfo blitInfo;
#ifdef SDR2HDR
        SDR2HDRParameter *param = (SDR2HDRParameter *)sdev->sdr2hdr_param;
#endif
        blitInfo.drect.x = drect->x;
        blitInfo.drect.y = drect->y;
        blitInfo.drect.w = drect->w;
        blitInfo.drect.h = drect->h;

        blitInfo.srect.x = srect->x;
        blitInfo.srect.y = srect->y;
        blitInfo.srect.w = srect->w;
        blitInfo.srect.h = srect->h;

        blitInfo.eglImageInfo.src.width  = src_w;
        blitInfo.eglImageInfo.src.height = src_h;
        blitInfo.eglImageInfo.src.pitch  = state->src.pitch;
        blitInfo.eglImageInfo.src.phys   = ((u64)state->src.phys_h << SHIFT_32) | state->src.phys;
        blitInfo.eglImageInfo.src.format = state->source->config.format;
        blitInfo.eglImageInfo.src.offset = state->src.offset;

        blitInfo.eglImageInfo.dst.width  = dst_w;
        blitInfo.eglImageInfo.dst.height = dst_h;
        blitInfo.eglImageInfo.dst.pitch  = state->dst.pitch;
        blitInfo.eglImageInfo.dst.phys   = ((u64)state->dst.phys_h << SHIFT_32) | state->dst.phys;
        blitInfo.eglImageInfo.dst.format = state->destination->config.format;
        blitInfo.eglImageInfo.dst.offset = state->dst.offset;

        blitInfo.eglImageInfo.enableAFBC = false;
        {
            if ( dfb_config->mst_GPU_AFBC == true && (state->destination->type & CSTF_AFBC)) {
                D_INFO("[DFB] %s, layer = %d, enable GPU AFBC!, pid=%d\n", __FUNCTION__, state->destination->resource_id, getpid());
                blitInfo.eglImageInfo.enableAFBC = true;
            }
        }

        // blending setting.
        blitInfo.dfb_src_blend = state->src_blend;
        blitInfo.dfb_dst_blend = state->dst_blend;

        // src color key.
        blitInfo.dfb_src_color_key = state->src_colorkey;

#ifdef SDR2HDR
        // sdr to hdr coeff.
        //SDR2HDR_MUTEX_LOCK_SLAVE(&sdrv->core->shared->lock_sw_sdr2hdr);
        //memcpy(&blitInfo.coeff, &param->coeff, sizeof(SDR2HDRCoeff));
        //SDR2HDR_MUTEX_UNLOCK(&sdrv->core->shared->lock_sw_sdr2hdr);
#endif

        blitInfo.coeff.en_sdr2hdr = dfb_config->mst_gles2_sdr2hdr;//(dfb_config->mst_gles2_sdr2hdr | blitInfo.coeff.en_sdr2hdr);
        // gles render.
        ret = gGlesFuncs.GLES2Blit( &blitInfo);
    }

    return ret;
}

void mstarBlit_GLES2_makeCurrent( void **shareContext)
{
    gGlesFuncs.GLES2MakeCurrent(shareContext);
}

void mstarBlit_GLES2_finish(void *shareContext)
{
    gGlesFuncs.GLES2Finish(shareContext);
}

bool
mstarStretchBlit_null( void                 *driver_data,
                       void                 *device_data,
                       DFBRectangle         *srect,
                       DFBRectangle         *drect )
{
    return true;
}


bool
mstarStretchBlit( void                      *driver_data,
                  void                      *device_data,
                  DFBRectangle              *srect,
                  DFBRectangle              *drect )
{
    MSTARDriverData *sdrv = driver_data;
    MI_OSD_Rect_t stSrcRect;
    MI_OSD_Rect_t stDstRect;

    stSrcRect.u32X = srect->x;
    stSrcRect.u32Y = srect->y;
    stSrcRect.u32Width = srect->w;
    stSrcRect.u32Height = srect->h;

    stDstRect.u32X = drect->x;
    stDstRect.u32Y = drect->y;
    stDstRect.u32Width = drect->w;
    stDstRect.u32Height = drect->h;

    if(sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YVYU || sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YUYV || sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_UYVY)
    {
        stSrcRect.u32Width = stSrcRect.u32Width & ~1;
    }

    if(sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YVYU || sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YUYV || sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_UYVY)
    {
        stDstRect.u32Width = (stDstRect.u32Width + 1) & ~1;
    }

    MI_OSD_BeginDraw();
    MI_OSD_Bitblit(sdrv->hSrcSurface, &stSrcRect, sdrv->hDstSurface, &stDstRect, &sdrv->stBlitOpt, &sdrv->stRenderJob);
    MI_OSD_EndDraw();

    return true;
}


bool
mstarStretchBlit_sw( void                   *driver_data,
                     void                   *device_data,
                     DFBRectangle           *srect,
                     DFBRectangle           *drect )
{
    if (dfb_config->sw_render & SWRF_STRETCHBLIT)
        return false;
    else
        return mstarStretchBlit(driver_data, device_data, srect, drect);
}


bool
mstarStretchBlitEx_null( void                       *driver_data,
                         void                       *device_data,
                         DFBRectangle               *srect,
                         DFBRectangle               *drect,
                         const DFBGFX_ScaleInfo     *scale_info )
{
    return true;
}


bool
mstarStretchBlitEx( void                            *driver_data,
                    void                            *device_data,
                    DFBRectangle                    *srect,
                    DFBRectangle                    *drect,
                    const DFBGFX_ScaleInfo          *scale_info )
{
    MSTARDriverData *sdrv = driver_data;
    MI_OSD_Rect_t stSrcRect;
    MI_OSD_Rect_t stDstRect;

    stSrcRect.u32X = srect->x;
    stSrcRect.u32Y = srect->y;
    stSrcRect.u32Width = srect->w;
    stSrcRect.u32Height = srect->h;

    stDstRect.u32X = drect->x;
    stDstRect.u32Y = drect->y;
    stDstRect.u32Width = drect->w;
    stDstRect.u32Height = drect->h;

    //MI do not support scale_info !!!!! need to discuss
    if(sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YVYU || sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YUYV || sdrv->src_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_UYVY)
    {
        stSrcRect.u32Width = stSrcRect.u32Width & ~1;
    }

    if(sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YVYU || sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_YUYV || sdrv->dst_ge_format == E_MI_OSD_COLOR_FORMAT_YUV422_UYVY)
    {
        stDstRect.u32Width = (stDstRect.u32Width + 1) & ~1;
    }

    MI_OSD_BeginDraw();
    MI_OSD_Bitblit(sdrv->hSrcSurface, &stSrcRect, sdrv->hDstSurface, &stDstRect, &sdrv->stBlitOpt, &sdrv->stRenderJob);
    MI_OSD_EndDraw();

    return true;
}


bool
mstarStretchBlitEx_sw( void                         *driver_data,
                       void                         *device_data,
                       DFBRectangle                 *srect,
                       DFBRectangle                 *drect,
                       const DFBGFX_ScaleInfo       *scale_info )
{
    if (dfb_config->sw_render & SWRF_STRETCHBLITEX)
        return false;
    else
        return mstarStretchBlitEx(driver_data, device_data, srect, drect, scale_info);
}

bool
mstarBlitTrapezoid_null( void                   *driver_data,
                         void                   *device_data,
                         DFBRectangle           *srect,
                         DFBTrapezoid           *dtrapezoid )
{
    return true;
}


bool
mstarBlitTrapezoid( void                        *driver_data,
                    void                        *device_data,
                    DFBRectangle                *srect,
                    DFBTrapezoid                *dtrapezoid )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    //MI do not support
    return false;
}


bool
mstarBlitTrapezoid_sw( void                     *driver_data,
                       void                     *device_data,
                       DFBRectangle             *srect,
                       DFBTrapezoid             *dtrapezoid )
{
    if (dfb_config->sw_render & SWRF_BLITTRAPEZOID)
        return false;
    else
        return mstarBlitTrapezoid(driver_data, device_data, srect, dtrapezoid);
}


bool
mstarFillTrapezoid_null( void                   *driver_data,
                         void                   *device_data,
                         DFBTrapezoid           *dtrapezoid )
{
    return true;
}


bool
mstarFillTrapezoid( void                        *driver_data,
                    void                        *device_data,
                    DFBTrapezoid                *dtrapezoid )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    //MI do not support
    return false;
}


bool
mstarFillTrapezoid_sw( void                     *driver_data,
                       void                     *device_data,
                       DFBTrapezoid             *dtrapezoid )
{
    if (dfb_config->sw_render & SWRF_FILLTRAPEZOID)
        return false;
    else
        return mstarFillTrapezoid(driver_data, device_data, dtrapezoid);
}


/**********************************************************************************************************************/


static int
driver_probe( CoreGraphicsDevice                *device )
{
    D_DEBUG_AT( MSTAR_MI_Driver, "[DFB] %s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);

    //return dfb_gfxcard_get_accelerator( device ) == 0x2D47;
    return 1;
}


static void
driver_get_info( CoreGraphicsDevice                 *device,
                 GraphicsDriverInfo                 *info )
{
    D_DEBUG_AT( MSTAR_MI_Driver, "[DFB] %s()\n", __FUNCTION__ );

    /* fill driver info structure */
    snprintf( info->name,
              DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
              "Mstar MI Driver" );

    snprintf( info->vendor,
              DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
              "Mstar corp." );

    info->version.major = 0;
    info->version.minor = 1;

    info->driver_data_size = sizeof(MSTARDriverData);
    info->device_data_size = sizeof(MSTARDeviceData);

    DFB_CHECK_POINT("driver_get_info done");
}


void
mstarGetSerial( void                                *driver_data,
                void                                *device_data,
                CoreGraphicsSerial                  *serial )
{
    MSTARDriverData *sdrv = driver_data;
    serial->generation = 0;
    serial->serial = (unsigned int)sdrv->stRenderJob.u32JobId;
}


void
mstarGetSerial_null( void                           *driver_data,
                     void                           *device_data,
                     CoreGraphicsSerial             *serial )
{

}


DFBResult
mstarWaitSerial( void                               *driver_data,
                 void                               *device_data,
                 const CoreGraphicsSerial           *serial )
{
     MI_OSD_RenderJob_t stRenderJob;

     stRenderJob.u32JobId = (u32)serial->serial;

     MI_OSD_WaitRenderDone(&stRenderJob);
     return DFB_OK;
}


DFBResult
mstarWaitSerial_null( void                          *driver_data,
                      void                          *device_data,
                      const CoreGraphicsSerial      *serial )
{
     return DFB_OK;
}

#if USE_FBDEV
static void                  *fbdev_data   = NULL;    /* FIXME */

extern DFBResult
mstar_fbdev_initialize( void *driver_data,
                    void **data );

extern DFBResult
mstar_fbdev_join(  void *driver_data,
                    void **data );
#endif

static DFBResult
mstar_init_driver( CoreGraphicsDevice               *device,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;
    MI_OSD_InitParams_t stInitPar;

    D_DEBUG_AT( MSTAR_MI_Driver, "%s()\n", __FUNCTION__ );
    DFB_CHECK_POINT("mstar_init_driver start");
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_START, DF_BOOT_LV5);
    //Init GOP:
    //Get GOP Idx:
    sdev->gfx_gop_index = dfb_config->mst_gfx_gop_index;

    //Init MI_OSD:
    stInitPar.bAutoFlip =  FALSE;
    stInitPar.bWaitSync =  FALSE;
    stInitPar.bWaitIdle =  FALSE;
    stInitPar.eCanvasBufMode =  E_MI_OSD_WINDOW_CANVAS_FIX;

    printf("\033[31m%s %d[ OSD Init Para ( CansMode, AutoFlip, WaitSync, WaitIdle) = (%d, %d, %d, %d) ]\033[m\n", __FUNCTION__, __LINE__,
        stInitPar.eCanvasBufMode,stInitPar.bAutoFlip,stInitPar.bWaitSync,stInitPar.bWaitIdle);

    stInitPar.u16SupportSurfaceCount = 500;

    MI_OSD_Init(&stInitPar);

#if USE_FBDEV
    if (dfb_core_is_master( sdrv->core )) {
        if (dfb_config->mst_call_disable_bootlogo)
        {
            MI_DISP_GetControllerParams_t stGetControllerParams;
            MI_HANDLE hDispController = MI_HANDLE_NULL;
            memset(&stGetControllerParams, 0x00, sizeof(MI_DISP_GetControllerParams_t));

            if (MI_DISP_GetController(&stGetControllerParams, &hDispController) != MI_OK)
            {
                MI_DISP_OpenControllerParams_t stOpenParams;
                memset(&stOpenParams, 0x00, sizeof(MI_DISP_OpenControllerParams_t));

                if (MI_DISP_OpenController(&stOpenParams, &hDispController) != MI_OK)
                    printf("[DFB] closeBootLogo get MI DISP Controller failed!!!!!!\n");
            }

            printf("[DFB] mstar_init_driver, call MI_OSD_DisableBootLogo\n");
            MI_OSD_DisableBootLogo();
            printf("[DFB] mstar_init_driver, call MI_DISP_DisableBootLogo\n");
            MI_DISP_DisableBootLogo(hDispController);
        }
        mstar_fbdev_initialize( driver_data, &fbdev_data );
    }
    else
        mstar_fbdev_join( driver_data, &fbdev_data );
#else
    mstar_init_gop_driver(device, driver_data);
#endif

    //MI_OSD_SetDebugLevel(MI_DBG_NONE);

    //Get Supported Bliting Flags Here:
    sdrv->gfx_supported_bld_flags = ( DSBLIT_SRC_COLORKEY       |
                                      DSBLIT_DST_COLORKEY       |
                                      DSBLIT_ROTATE90           |
                                      DSBLIT_ROTATE180          |
                                      DSBLIT_ROTATE270          |
                                      DSBLIT_SOURCE2   |
                                      DSBLIT_BLINK_FOREGROUND   |
                                      DSBLIT_FLIP_HORIZONTAL           |
                                      DSBLIT_FLIP_VERTICAL           |
                                      DSBLIT_ROP     |
                                      DSBLIT_SRC_COLORMATRIX     |
                                      DSBLIT_SRC_CONVOLUTION           |
                                      DSBLIT_BA_NOUSE           |
                                      DSBLIT_BA_TRANSLUCENT     |
                                      DSBLIT_BA_TRANSPARENT  |
                                      DSBLIT_BLEND_COLORALPHA |
                                      DSBLIT_BLEND_ALPHACHANNEL |
                                      DSBLIT_COLORIZE                    |
                                      DSBLIT_SRC_PREMULTIPLY    |
                                      DSBLIT_SRC_PREMULTCOLOR    |
                                      DSBLIT_DST_PREMULTIPLY        |
                                      DSBLIT_XOR        |
                                      DSBLIT_DEMULTIPLY        |
                                      DSBLIT_SRC_MASK_ALPHA        |
                                      DSBLIT_SRC_MASK_COLOR
                                        );

    sdrv->gfx_supported_draw_flags = DSDRAW_NOFX;
    sdrv->gfx_supported_draw_flags|= DSDRAW_BLEND;
    sdrv->gfx_supported_draw_flags|= DSDRAW_SRC_PREMULTIPLY;
    sdrv->gfx_supported_draw_flags|= DSDRAW_DST_PREMULTIPLY;
    sdrv->gfx_supported_draw_flags|= DSDRAW_XOR;
    sdrv->gfx_supported_draw_flags|= DSDRAW_DEMULTIPLY;

    sdrv->ge_blit_nearestmode_enabled   = 0;
    sdrv->ge_blit_xmirror               = 0;
    sdrv->ge_blit_ymirror               = 0;
    sdrv->ge_draw_dst_colorkey_enabled  = 0;
    sdrv->ge_blit_dst_colorkey_enabled  = 0;
    sdrv->ge_draw_alpha_blend_enabled   = 0;
    sdrv->ge_blit_alpha_blend_enabled   = 0;
    sdrv->ge_src_colorkey_enabled       = 0;
    sdrv->ge_palette_enabled            = 0;
    sdrv->ge_palette_intensity_enabled  = 0;
    sdrv->ge_render_alpha_cmp_enabled   = 0;
    sdrv->ge_dest_rgb32_blit_enabled    = 0;
    sdrv->ge_src_rgb32_blit_enabled     = 0;

    sdrv->ge_src_blend      = E_MI_OSD_DFB_BLEND_NONE;
    sdrv->ge_dst_blend      = E_MI_OSD_DFB_BLEND_NONE;
    sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_NONE;
    sdrv->src_ge_yuv_fmt    = E_MI_OSD_YUV422_YUYV;
    sdrv->dst_ge_yuv_fmt    = E_MI_OSD_YUV422_YUYV;

    //Set Rop2
    sdrv->ge_rop_op = E_MI_OSD_ROP_NONE;
    sdrv->ge_rop_enable = FALSE;
    sdrv->ge_dither_enable = FALSE;

    //Set MI init
    sdrv->hSrcSurface = MI_OSD_INVALID_ID;
    sdrv->hDstSurface = MI_OSD_INVALID_ID;
    sdrv->stBlitOpt.eMirror = E_MI_OSD_MIRROR_NONE;
    sdrv->stBlitOpt.eRotate = E_MI_OSD_ROTATE_NONE;
    sdrv->stBlitOpt.eBlendMode = E_MI_OSD_BLEND_NONE;
    sdrv->stBlitOpt.eSrcDfbBlendMode = E_MI_OSD_DFB_BLEND_NONE;
    sdrv->stBlitOpt.eDstDfbBlendMode = E_MI_OSD_DFB_BLEND_NONE;
    sdrv->stBlitOpt.eDfbBlendFlag = E_MI_OSD_DFB_BLEND_FLAG_NONE;
    sdrv->stBlitOpt.bIsDfbBlend = FALSE;
    sdrv->stBlitOpt.eRopMode = E_MI_OSD_ROP_NONE;
    sdrv->stBlitOpt.eAlphaFrom = E_MI_OSD_ALPHA_FROM_SRC;
    sdrv->stBlitOpt.eConstAlphaFrom = E_MI_OSD_CONST_ALPHA_FROM_DST;
    sdrv->stBlitOpt.eSrcYuvFormat = E_MI_OSD_YUV422_YUYV;
    sdrv->stBlitOpt.eDstYuvFormat = E_MI_OSD_YUV422_YUYV;
    sdrv->stBlitOpt.stClipRect.u32X = 0;
    sdrv->stBlitOpt.stClipRect.u32Y = 0;
    sdrv->stBlitOpt.stClipRect.u32Width = 0;
    sdrv->stBlitOpt.stClipRect.u32Height = 0;
    sdrv->stBlitOpt.stConstColor.u8Blue = 0x00;
    sdrv->stBlitOpt.stConstColor.u8Green = 0x00;
    sdrv->stBlitOpt.stConstColor.u8Red = 0x00;
    sdrv->stBlitOpt.stConstColor.u8Alpha = 0x00;
    sdrv->stBlitOpt.bWaitIdle = FALSE;
    sdrv->stBlitOpt.bDither = FALSE;
    sdrv->stBlitOpt.eRgb2YuvMode = E_MI_OSD_RGB_TO_YUV_FULL_RANGE;
    sdrv->stBlitOpt.eStretchBlitType = E_MI_OSD_STRETCH_BLIT_TYPE_BILINEAR;
    sdrv->stBlitOpt.eDstMirror = E_MI_OSD_MIRROR_NONE;

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_END, DF_BOOT_LV5);

    //DFB_CHECK_POINT("mstar_init_driver done");

    return DFB_OK;
}


static DFBResult
mstar_init_device ( CoreGraphicsDevice              *device,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;

    return DFB_OK;
}

void
mstar_FlushTextureCache_null( void                  *driver_data,
                              void                  *device_data )
{
}


void
mstar_FlushTextureCache( void                       *driver_data,
                         void                       *device_data )
{
    MI_OS_FlushMemory();
}


DFBResult
mstar_EngineSync_null( void                         *driver_data,
                       void                         *device_data)
{
    return DFB_OK;
}


DFBResult
mstar_EngineSync( void                              *driver_data,
                  void                              *device_data)
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    if (sdrv->hDstSurface != MI_OSD_INVALID_ID)
    {

        MI_OSD_SurfaceFlush(sdrv->hDstSurface);

    }
    //MApi_GFX_FlushQueue();

    return DFB_OK;
}


static GraphicsDeviceFuncs nullDriverFuncSetting =
{
    .GetSerial          = mstarGetSerial,
    .WaitSerial         = mstarWaitSerial_null,
    .CheckState         = mstarCheckState,
    .SetState           = mstarSetState,
    .FillRectangle      = mstarFillRectangle_null,
    .DrawRectangle      = mstarDrawRectangle_null,
    .DrawOval           = mstarDrawOval_null,
    .DrawLine           = mstarDrawLine_null,
    .Blit               = mstarBlit_null,
    .StretchBlit        = mstarStretchBlit_null,
    .BlitTrapezoid      = mstarBlitTrapezoid_null,
    .FillTrapezoid      = mstarFillTrapezoid_null,
    .StretchBlitEx      = mstarStretchBlitEx_null,
    .FlushTextureCache  = mstar_FlushTextureCache_null,
    .EngineSync         = mstar_EngineSync_null,
};


static GraphicsDeviceFuncs softwareRenderFuncSetting =
{
    .GetSerial          = mstarGetSerial,
    .WaitSerial         = mstarWaitSerial,
    .CheckState         = mstarCheckState,
    .SetState           = mstarSetState,
    .FillRectangle      = mstarFillRectangle_sw,
    .DrawRectangle      = mstarDrawRectangle_sw,
    .DrawOval           = mstarDrawOval_sw,
    .DrawLine           = mstarDrawLine_sw,
    .Blit               = mstarBlit_sw,
    .StretchBlit        = mstarStretchBlit_sw,
    .BlitTrapezoid      = mstarBlitTrapezoid_sw,
    .FillTrapezoid      = mstarFillTrapezoid_sw,
    .StretchBlitEx      = mstarStretchBlitEx_sw,
    .FlushTextureCache  = mstar_FlushTextureCache,
    .EngineSync         = mstar_EngineSync,

};

static GraphicsDeviceFuncs defaultFuncSetting =
{
    .GetSerial            = mstarGetSerial,
    .WaitSerial           = mstarWaitSerial,
    .CheckState           = mstarCheckState,
    .SetState             = mstarSetState,
    .FillRectangle        = mstarFillRectangle,
    .DrawRectangle        = mstarDrawRectangle,
    .DrawOval             = mstarDrawOval,
    .DrawLine             = mstarDrawLine,
    .Blit                 = mstarBlit,
    .StretchBlit          = mstarStretchBlit,
    .BlitTrapezoid        = mstarBlitTrapezoid,
    .FillTrapezoid        = mstarFillTrapezoid,
    .StretchBlitEx        = mstarStretchBlitEx,
    .FlushTextureCache    = mstar_FlushTextureCache,
    .EngineSync           = mstar_EngineSync,

};

static DFBResult
driver_init_driver( CoreGraphicsDevice              *device,
                    GraphicsDeviceFuncs             *funcs,
                    void                            *driver_data,
                    void                            *device_data,
                    CoreDFB                         *core )
{
    DFBResult         ret;
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;
    u8  i;

    D_DEBUG_AT( MSTAR_MI_Driver, "%s()\n", __FUNCTION__ );
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DRIVER, DF_MEASURE_START, DF_BOOT_LV5);

    /* Keep pointer to shared device data. */
    sdrv->dev = device_data;

    /* Keep core and device pointer. */
    sdrv->core   = core;
    sdrv->device = device;

    memset(sdrv->mstarLayerBuffer, 0, sizeof(sdrv->mstarLayerBuffer));

    /* I/O remap for current process   */
    //MDrv_MMIO_Init();


    if (dfb_config->null_driver)
    {
        /* null driver function setting */
        printf("[DFB] driver_init_driver, null driver\n");

        *funcs = nullDriverFuncSetting;
    }
    else if (dfb_config->sw_render & SWRF_ENABLE)
    {
        /* software render function setting */
        printf("[DFB] driver_init_driver, sw render\n");

        *funcs = softwareRenderFuncSetting;
    }
    else
    {
        /* default function setting */
        *funcs = defaultFuncSetting;

//#if defined(USE_GFX_EXTENSION)
#if 0
        if (dfb_config->mst_use_dlopen_dlsym)
        {
            static void* pHandle = NULL;
            GFX_Result (*GFXTriFill)(GFX_TriFillInfo *ptriblock);
            GFX_Result (*GFXSpanFill)(GFX_SpanFillInfo *pspanblock);
            pHandle = dlopen ("libapiGFX.so", RTLD_LAZY);

            if (pHandle)
            {
                GFXTriFill = dlsym(pHandle, "MApi_GFX_TriFill");
                GFXSpanFill = dlsym(pHandle, "MApi_GFX_SpanFill");
                funcs->FillTriangleEx = GFXTriFill ? mstarFillTriangleEx : NULL;
                funcs->FillSpan = GFXSpanFill ? mstarFillSpan : NULL;
            }
        }
        else
        {
            if (MApi_GFX_TriFill)
            {
                funcs->FillTriangleEx = mstarFillTriangleEx ;
                D_INFO("[DFB][%s (%s) %d] Get fun MApi_GFX_TriFill success\n", __FUNCTION__, __FILE__, __LINE__);
            }
            else
            {
                funcs->FillTriangleEx = NULL;
                printf("[DFB][%s (%s) %d] Get fun MApi_GFX_TriFill failed\n", __FUNCTION__, __FILE__, __LINE__);
            }

            if (MApi_GFX_SpanFill)
            {
                funcs->FillSpan = mstarFillSpan ;
                D_INFO("[DFB][%s (%s) %d] Get fun MApi_GFX_SpanFill success\n", __FUNCTION__, __FILE__, __LINE__);
            }
            else
            {
                funcs->FillSpan = NULL;
                printf("[DFB][%s (%s) %d] Get fun MApi_GFX_SpanFill failed\n", __FUNCTION__, __FILE__, __LINE__);
            }
        }
#endif

        /* blit at gles2 mode */
        if (dfb_config->mst_enable_GLES2)
        {
            _check_GLES2();

            if (gGlesFuncs.GLES2Blit)
            {
                funcs->BlitGLES2 =  mstarBlit_GLES2;
                funcs->FinishGLES2 = mstarBlit_GLES2_finish;
                funcs->MakeCurrent = mstarBlit_GLES2_makeCurrent;

                if (gGlesFuncs.GLES2Init)
                {
                    bool bgles2init = false;
                    bgles2init = gGlesFuncs.GLES2Init();

                    if (!bgles2init)
                        printf("[DFB] GLES2Init fail\n");
                }

#ifdef SDR2HDR
                printf("[DFB] %s, %d, sdev->sdr2hdr_param = %x\n",
                            __FUNCTION__, __LINE__, sdev->sdr2hdr_param);

                /* create shm mem to keep SDR2HDR parameter. */
                if (sdev->sdr2hdr_param == NULL)
                {
                    FusionSHMPoolShared *shm_pool;
                    SDR2HDRParameter    *param;

                    shm_pool = dfb_core_shmpool_data(sdrv->core);

                    param               = SHMALLOC(shm_pool, sizeof(SDR2HDRParameter));
                    param->lock         = &sdrv->core->shared->lock_sw_sdr2hdr;
                    param->ptr_en_gles2 = &sdrv->core->shared->enable_GLES2;

                    sdev->sdr2hdr_param = param;

                    printf("[DFB] %s, %d, allocate shm, sdev->sdr2hdr_param = %x\n",
                        __FUNCTION__, __LINE__, sdev->sdr2hdr_param);
                }

                if (gGlesFuncs.InitSDR2HDRParameter)
                    gGlesFuncs.InitSDR2HDRParameter(sdev->sdr2hdr_param);
                else
                    printf("\33[0;33;44m[DFB]\33[0m gGlesFuncs.InitSDR2HDRParameter error!\n");
#endif
            }
            else
                printf("[DFB] driver_init_driver, not define BlitGLES2!\n");
            
        }
    }


     /* Get virtual address for the LCD buffer in slaves here,
        master does it in driver_init_device(). */
     if (!dfb_core_is_master( core ))
          sdrv->lcd_virt = dfb_gfxcard_memory_virtual( device, sdev->lcd_offset );


     /* Register multi window layer. */
     //sdrv->multi = dfb_layers_register( sdrv->screen, driver_data, &sh7722MultiLayerFuncs );
     ret = mstar_init_driver(device, driver_data, device_data);
     DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DRIVER, DF_MEASURE_END, DF_BOOT_LV5);


     return ret;
}

static DFBResult
driver_init_device( CoreGraphicsDevice              *device,
                    GraphicsDeviceInfo              *device_info,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;
    u16 pnlW;
    u16 pnlH;
    int i;

    //sdev->gfx_gop_index=dfb_config->mst_gfx_gop_index;
    D_DEBUG_AT( MSTAR_MI_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);
    //  printf("MAdp_SYS_GetPanelResolution-->%d, %d\n", pnlW, pnlH);
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DEVICE, DF_MEASURE_START, DF_BOOT_LV5);

    for(i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {
        sdev->layer_active[i]       = false;
        sdev->layer_pinpon_mode[i]  = false;
        sdev->layer_gwin_id[i]      = 0xFF;
        sdev->layer_gwin_id_r[i]    = 0xFF;
        sdev->layer_refcnt[i]       = 0x0;
    }


    //Get pnlW, pnlH

    pnlW = dfb_config->mst_lcd_width;
    pnlH = dfb_config->mst_lcd_height;


    if(dfb_config->mst_gfx_width == 0)
    {
        sdev->gfx_max_width = pnlW;
    }
    else
    {
        sdev->gfx_max_width = dfb_config->mst_gfx_width;

        if(sdev->gfx_max_width > pnlW)
            sdev->gfx_max_width = pnlW;
    }

    if(dfb_config->mst_gfx_height == 0)
    {
        sdev->gfx_max_height = pnlH;
    }
    else
    {
        sdev->gfx_max_height = dfb_config->mst_gfx_height;

        if(sdev->gfx_max_height >pnlH)
            sdev->gfx_max_height  = pnlH;
    }

    sdev->lcd_width = pnlW;
    sdev->lcd_height= pnlH;

    sdev->gfx_max_width  &= ~3;

    sdev->gfx_h_offset          = dfb_config->mst_gfx_h_offset;
    sdev->gfx_v_offset          = dfb_config->mst_gfx_v_offset;

    //Init A8 GE Palette default:
    memset(sdev->palette_tbl, 0xFF, (sizeof(MI_OSD_RgbColor_t)*MAX_PALETTE_ENTRY_CNT));

    for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
    {
        sdev->palette_tbl[i].u8Alpha = i;
    }

    sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
    sdev->a8_palette = true;

    /*
     * Initialize I1/I2/I4 related palette.
     */
    sdev->num_intensity = 0x0;

    /*
    * Initialize hardware.
    */
    device_info->caps.accel = DFXL_FILLRECTANGLE    |
                              DFXL_DRAWRECTANGLE    |
                              DFXL_DRAWLINE         |
                              DFXL_FILLQUADRANGLE         |
                              DFXL_FILLTRAPEZOID    |
                              DFXL_BLIT             |
                              DFXL_STRETCHBLIT      |
                              DFXL_BLIT2;

    device_info->caps.blitting = sdrv->gfx_supported_bld_flags;
    device_info->caps.drawing  = sdrv->gfx_supported_draw_flags;

    /* Set device limitations. */
    if(dfb_config->bUsingHWTLB)
    {
        device_info->limits.surface_byteoffset_alignment = dfb_config->TLBAlignmentSize;
    }
    else
    {
        //DIP capture need 256 bytes alignment and IPA need 4096 alignment
        //so force create buffe 4096bytes alignment
        device_info->limits.surface_byteoffset_alignment = 4096;
    }

    device_info->limits.surface_bytepitch_alignment  = 64;
    device_info->limits.surface_hegiht_alignment     = 16;

    device_info->caps.flags &= ~CCF_CLIPPING;
    device_info->caps.clip = DFXL_STRETCHBLIT;
    sdev->b_hwclip = true;

    if(dfb_config->mst_disable_hwclip)
    {
        device_info->caps.flags &= ~CCF_CLIPPING;
        device_info->caps.clip = 0;
        sdev->b_hwclip = false;
    }

    mstar_init_device(device, driver_data, device_data);
    /* Initialize BEU lock. */
    fusion_skirmish_init( &sdev->beu_lock, "BEU", dfb_core_world(sdrv->core) );
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DEVICE, DF_MEASURE_END, DF_BOOT_LV5);
    return DFB_OK;
}

static void
driver_close_device( CoreGraphicsDevice             *device,
                     void                           *driver_data,
                     void                           *device_data )
{
    MSTARDeviceData *sdev = device_data;

    D_DEBUG_AT( MSTAR_MI_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);

    /* Destroy BEU lock. */
    fusion_skirmish_destroy( &sdev->beu_lock );
}

void
mstarAllSurfInfo( MSTARDriverData                   *sdrv);

#if USE_FBDEV
extern DFBResult mstar_fbdev_leave( bool emergency );
extern DFBResult mstar_fbdev_shutdown( bool emergency );
#endif

static void
driver_close_driver( CoreGraphicsDevice             *device,
                     void                           *driver_data )
{
     MSTARDriverData *sdrv = driver_data;
     MI_RESULT Ret = MI_ERR_FAILED;

     D_DEBUG_AT( MSTAR_MI_Driver, "%s()\n", __FUNCTION__ );

     if (gGlesFuncs.GLES2Destroy)
         gGlesFuncs.GLES2Destroy();

#if USE_FBDEV
     if (dfb_core_is_master( sdrv->core ))
          mstar_fbdev_shutdown(false);
     else
          mstar_fbdev_leave(false);
#endif

     mstarAllSurfInfo(sdrv);

    D_INFO("[DFB] %s, %d, hSrcSurface = %x, pid=%d\n", __FUNCTION__, __LINE__, sdrv->hSrcSurface, getpid());
    if(sdrv->hSrcSurface != MI_OSD_INVALID_ID)
    {
        Ret = MI_OSD_SurfaceDestroy(sdrv->hSrcSurface);
        if (Ret != MI_OK)
        {
            printf("[DFB] MI_OSD_SurfaceDestroy hSrcSurface! ret = %d\n", Ret);
        }
    }

    D_INFO("[DFB] %s, %d, hDstSurface = %x, pid=%d\n", __FUNCTION__, __LINE__, sdrv->hDstSurface, getpid());
    if(sdrv->hDstSurface != MI_OSD_INVALID_ID)
    {
        Ret = MI_OSD_SurfaceDestroy(sdrv->hDstSurface);
        if (Ret != MI_OK)
        {
            printf("[DFB] MI_OSD_SurfaceDestroy hDstSurface! ret = %d\n", Ret);
        }
    }

}
