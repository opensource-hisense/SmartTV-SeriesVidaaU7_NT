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


#include "mstar.h"

#include "mstar_layer.h"

#include "mstar_gles2.h"

#include "mtk-v4l2-g2d.h"

#define BYTE_SIZE 8
#define SHIFT8 8
#define SHIFT16 16
#define SHIFT24  24
#define BUF_SIZE 1024

#define DSPF_ARGB1555_FORMAT 15,1,10,5,5,5,0,5
#define DSPF_RGB16_FORMAT    16,0,11,5,5,6,0,5
#define DSPF_RGB555_FORMAT   15,0,10,5,5,5,0,5
#define DSPF_BGR555_FORMAT   15,0,0,5,5,5,10,5
#define DSPF_ARGB2554_FORMAT 14,2,9,5,4,5,0,4
#define DSPF_ARGB4444_FORMAT 12,4,8,4,4,4,0,4
#define DSPF_RGB444_FORMAT   12,0,8,4,4,4,0,4
#define DSPF_RGB24_FORMAT    24,0,16,8,8,8,0,8
#define DSPF_RGB32_FORMAT    24,0,16,8,8,8,0,8
#define DSPF_ARGB_FORMAT     24,8,16,8,8,8,0,8
#define DSPF_ABGR_FORMAT     24,8,0,8,8,8,16,8

#define BYTE_MASK 0xFF
#define DEFAULT_COLOR 0x55555555
#define ROTATE90_VALUE  270
#define ROTATE180_VALUE 180
#define ROTATE270_VALUE 90
#define SHIFT32 32
#define CLEAR3 ~3
#define BYTEOFFSET_ALIGNMENT 4096
#define BYTEPITCH_ALIGNMENT 64
#define HEIGHT_ALIGNMENT 16
#define PATH_SIZE 100
#define CUBESIZE_INIT 6
#define CUBESIZE_BOUNDARY 9
#define COLOR_RANGE 256
#define HALF_CUBEEDGE (cubeEdge/2)
#define index0  0
#define index1  1
#define index2  2
#define index3  3
#define index4  4
#define index5  5
#define index6  6
#define index7  7
#define index8  8

/* define global variables */
static bool (*GLESInitFuncs)(GLESFuncs *funcs) = NULL;
static GLESFuncs gGlesFuncs;


DFB_GRAPHICS_DRIVER( mtk_sti )


D_DEBUG_DOMAIN( MTK_STI_Driver, "MTK_STI/Driver", "MTK STI Driver" );


static void _check_GLES2(void)
{
    static void* pHandle_gles2 = NULL;

    if (pHandle_gles2 == NULL)
    {
        const char *path = MSTAR_GLES2_SUBMODULE_PATH;
        char  buf[BUF_SIZE];

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
        while(a_bits < BYTE_SIZE)
        {
            a = a | a >> a_bits;
            a_bits <<= 1;
        }
    }

    if(r_bits)
    {
        while(r_bits < BYTE_SIZE)
        {
            r = r | r >> r_bits;
            r_bits <<= 1;
        }
    }

    if(g_bits)
    {
        while(g_bits < BYTE_SIZE)
        {
            g = g | g >> g_bits;
            g_bits <<= 1;
        }
    }

    if(b_bits)
    {
        while(b_bits < BYTE_SIZE)
        {
            b = b | b >> b_bits;
            b_bits <<= 1;
        }
    }

    return a << SHIFT24 | r << SHIFT16 | g << SHIFT8 | b;

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
                return mstar_build_color(pixel, DSPF_ARGB1555_FORMAT);

        case DSPF_RGB16:
                return mstar_build_color(pixel, DSPF_RGB16_FORMAT);

        case DSPF_RGB555:
                return mstar_build_color(pixel, DSPF_RGB555_FORMAT);

        case DSPF_BGR555:
                return mstar_build_color(pixel, DSPF_BGR555_FORMAT);

        case DSPF_ARGB2554:
                return mstar_build_color(pixel, DSPF_ARGB2554_FORMAT);

        case DSPF_ARGB4444:
                return mstar_build_color(pixel, DSPF_ARGB4444_FORMAT);

        case DSPF_RGB444:
                return mstar_build_color(pixel, DSPF_RGB444_FORMAT);

        case DSPF_RGB24:
                return mstar_build_color(pixel, DSPF_RGB24_FORMAT);

        case DSPF_RGB32:
                return mstar_build_color(pixel, DSPF_RGB32_FORMAT);

        case DSPF_ARGB:
                return mstar_build_color(pixel, DSPF_ARGB_FORMAT);

        case DSPF_ABGR:
                return mstar_build_color(pixel, DSPF_ABGR_FORMAT);

        case DSPF_LUT8:
        case DSPF_LUT4:
        case DSPF_LUT2:
        case DSPF_LUT1:
                if( palette )
                {
                    if( pixel >= 0 &&
                        pixel <= (1UL << DFB_BITS_PER_PIXEL(format)) - 1)
                        return  ((unsigned long)palette->entries[pixel].a) << SHIFT24 |
                                ((unsigned long)palette->entries[pixel].r) << SHIFT16 |
                                ((unsigned long)palette->entries[pixel].g) << SHIFT8  |
                                                palette->entries[pixel].b;
                }

                break;

        case DSPF_YUY2:
                cb = (pixel >> SHIFT24) & BYTE_MASK;
                y  = (pixel >> SHIFT16) & BYTE_MASK;
                cr = (pixel >> SHIFT8) & BYTE_MASK;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );

                return BYTE_MASK << SHIFT24 | r << SHIFT16 | g << SHIFT8 | b;

        case DSPF_YVYU:
                cr = (pixel >> SHIFT24) & BYTE_MASK;
                y  = (pixel >> SHIFT16) & BYTE_MASK;
                cb = (pixel >> SHIFT8 ) & BYTE_MASK;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );

                return BYTE_MASK << SHIFT24 | r << SHIFT16 | g << SHIFT8 | b;

        case DSPF_VYUY:
                y   = (pixel >> SHIFT24) & BYTE_MASK;
                cr  = (pixel >> SHIFT16) & BYTE_MASK;
                cb  = (pixel      ) & BYTE_MASK;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );
                return BYTE_MASK << SHIFT24 | r << SHIFT16 | g << SHIFT8 | b;

        case DSPF_UYVY:
                y   = (pixel >> SHIFT24) & BYTE_MASK;
                cb  = (pixel >> SHIFT16) & BYTE_MASK;
                cr  = (pixel      ) & BYTE_MASK;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );
                return BYTE_MASK << SHIFT24 | r << SHIFT16 | g << SHIFT8 | b;

        default:
                D_WARN( "[DFB] unknown format : %s\n", dfb_pixelformat_name(format) );
                break;
    }

    return DEFAULT_COLOR;
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
                    return pixel << SHIFT24  |
                           pixel << SHIFT16  |
                           pixel << SHIFT8  |
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
                       enum v4l2_g2d_dfbbld_op     *pgfx_src_bld_op,
                       enum v4l2_g2d_dfbbld_op     *pgfx_dst_bld_op,
                       u16                         *pu16_bld_flags )
{

    if(DSBLIT_NOFX == blittingflags)
        return false;

    //Set Blend Flag:
    *pu16_bld_flags = 0x0;
    if(blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_ALPHACHANNEL;

    if(blittingflags & DSBLIT_BLEND_COLORALPHA)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_COLORALPHA;

    if(blittingflags & DSBLIT_COLORIZE)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_COLORIZE;

    if(blittingflags & DSBLIT_SRC_PREMULTIPLY)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_SRCPREMUL;
    if(blittingflags & DSBLIT_DST_PREMULTIPLY)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_DSTPREMUL;

    if(blittingflags & DSBLIT_DEMULTIPLY)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_DEMULTIPLY;

    if(blittingflags & DSBLIT_SRC_PREMULTCOLOR)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_SRCPREMULCOL;

    if(blittingflags & DSBLIT_XOR)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_XOR;

    if(blittingflags & DSBLIT_SRC_MASK_ALPHA)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_SRCALPHAMASK;

    if(blittingflags & DSBLIT_SRC_MASK_COLOR)
        *pu16_bld_flags |= G2D_DFB_BLD_FLAG_SRCCOLORMASK;

    //No blending!
    if(0x0 == *pu16_bld_flags)
        return false;

    if(DSBF_UNKNOWN == src_blend)
        *pgfx_src_bld_op = G2D_DFB_BLD_OP_ONE;
    else
        *pgfx_src_bld_op = (enum v4l2_g2d_dfbbld_op)(src_blend-1);

    if(DSBF_UNKNOWN == dst_blend)
        *pgfx_dst_bld_op = G2D_DFB_BLD_OP_ONE;
    else
        *pgfx_dst_bld_op = (enum v4l2_g2d_dfbbld_op)(dst_blend-1);

    if(!(blittingflags & (DSBLIT_BLEND_ALPHACHANNEL|DSBLIT_BLEND_COLORALPHA)))
    {
        //patch HW, current GE only do blend operations with alpha blend on
        *pgfx_src_bld_op = G2D_DFB_BLD_OP_ONE;
        *pgfx_dst_bld_op = G2D_DFB_BLD_OP_ZERO;
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
        if( state->matrix[index0] != (1 << SHIFT16)  ||
            state->matrix[index1] != 0               ||
            state->matrix[index2] != 0               ||
            state->matrix[index3] != 0               ||
            state->matrix[index4] != (1 << SHIFT16)  ||
            state->matrix[index5] != 0               ||
            state->matrix[index6] != 0               ||
            state->matrix[index7] != 0               ||
            state->matrix[index8] != (1 << SHIFT16)             )
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
        case DSPF_VYUY:
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
                case DSPF_VYUY:
                case DSPF_UYVY:
                        if (bcheck_destination_rgb_format && dfb_config->do_yuvtorgb_sw_patch)
                        {
                            D_INFO("[DFB] HW BUG to patch it with SW\n");
                            break;
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

#if USE_SIZE_OPTIMIZATION
                case DSPF_A8:
#endif
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
    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&config, 0, sizeof(config));
    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
    ext_controls.controls->ptr = (void *)&config;

    D_INFO("[DFB] %s(%d) src_colorkey_enabled=%d, dst_colorkey_enabled=%d, ge_blit_xmirror=%d, ge_blit_ymirror=%d, pid=%d \n",
        __FUNCTION__,__LINE__,sdrv->ge_src_colorkey_enabled, sdrv->ge_blit_dst_colorkey_enabled,sdrv->ge_blit_xmirror,sdrv->ge_blit_ymirror,getpid());

    //Set ColorKey:
    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY ))
    {
        config.type = V4L2_G2D_COLORKEY;
        config.colorkey.type = G2D_COLORKEY_SRC;
        config.colorkey.mode = G2D_CK_OP_EQUAL;
        config.colorkey.range.color_s.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_s.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_s.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_s.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.range.color_e.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_e.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_e.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_e.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.fmt = sdrv->dst_ge_format;
        config.colorkey.enable =  sdrv->ge_src_colorkey_enabled;

        D_INFO("yuhan YAYA,enable=%d\n",config.colorkey.enable);
        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls)<0)
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }
  



    if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY ))
    {

        config.type = V4L2_G2D_COLORKEY;
        config.colorkey.type = G2D_COLORKEY_DST;
        config.colorkey.mode = G2D_CK_OP_NOT_EQUAL;
        config.colorkey.range.color_s.a = (sdrv->dst_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_s.r = (sdrv->dst_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_s.g = (sdrv->dst_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_s.b = (sdrv->dst_ge_clr_key)&BYTE_MASK;
        config.colorkey.range.color_e.a = (sdrv->dst_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_e.r = (sdrv->dst_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_e.g = (sdrv->dst_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_e.b = (sdrv->dst_ge_clr_key)&BYTE_MASK;
        config.colorkey.fmt = sdrv->dst_ge_format;
        config.colorkey.enable =  sdrv->ge_blit_dst_colorkey_enabled;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
        
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
                        /* discuss with GE owner, only set enable false means MApi_GFX_EnableDFBBlending(FALSE) */
                        config.type = V4L2_G2D_DFB;
                        config.abl.enable = false;

                        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
                        {
                            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
                        }

                        /*  
                             MApi_GFX_SetAlpha = 
                             MApi_GFX_EnableAlphaBlending((MS_BOOL)abl.enable);
                             MApi_GFX_SetAlphaBlending((GFX_BlendCoef)abl.eBlendCoef,abl.constAlpha);
                             MApi_GFX_SetAlphaSrcFrom((GFX_AlphaSrcFrom)abl.eAlphaSrc);
                        */
                        config.type = V4L2_G2D_ABL;
                        config.abl.enable = true;
                        config.abl.eBlendCoef = G2D_COEF_ONE;
                        config.abl.constAlpha = sdrv->color.a;
                        config.abl.eAlphaSrc = G2D_ABL_FROM_CONST;

                        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
                        {
                            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
                        }

                        
                        break;

                default :
                {

                        config.type = V4L2_G2D_DFB;
                        config.dfb.enable = true;
                        config.dfb.flags = sdrv->ge_blit_bld_flags;
                        config.dfb.eSrcDFBBldOP = sdrv->ge_src_blend;
                        config.dfb.eDstDFBBldOP = sdrv->ge_dst_blend;
                        config.dfb.color.a = sdrv->color.a;
                        config.dfb.color.r = sdrv->color.r;
                        config.dfb.color.g = sdrv->color.g;
                        config.dfb.color.b = sdrv->color.b; 

                        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
                        {
                            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
                        }

                        break;
                }
            }
        }
        else
        {
            /* discuss with GE owner, only set enable false means MApi_GFX_EnableDFBBlending(FALSE) */
            config.type = V4L2_G2D_DFB;
            config.abl.enable = false;
            
            if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }
        

            config.type = V4L2_G2D_ABL;
            config.abl.enable = false;
            config.abl.eBlendCoef = G2D_COEF_ONE;
            config.abl.eAlphaSrc = G2D_ABL_FROM_ASRC;

            if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }
        }
    }

    //Set BLIT rotation
    if(modified & SMF_BLITTING_FLAGS)
    {
        struct v4l2_control control;
        control.id = V4L2_CID_ROTATE;

        if(sdrv->bflags & DSBLIT_ROTATE90)
            control.value = ROTATE90_VALUE;

        else if(sdrv->bflags & DSBLIT_ROTATE180)
            control.value = ROTATE180_VALUE;

        else if(sdrv->bflags & DSBLIT_ROTATE270)
            control.value = ROTATE270_VALUE;
        else
            control.value = 0;


        if(ioctl(sdrv->fd, VIDIOC_S_CTRL, &control) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }


    }

    //Set Blit Nearest Mode:
    if(modified & SMF_SRC_CONVOLUTION)
    {

        config.type = V4L2_G2D_NEARESTMODE;
        config.enable = sdrv->ge_blit_nearestmode_enabled;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }


    //GOP mirror do GE mirror for mantis id:0523773
    if (dfb_config->do_GE_Vmirror == true)
    {       
    
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {

            config.type = V4L2_G2D_MIRROR;
            config.mirror.HMirror = sdrv->ge_blit_xmirror;
            config.mirror.VMirror = sdrv->ge_blit_ymirror;

            if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }

        }

        config.type = V4L2_G2D_DSTMIRROR;
        config.mirror.HMirror = false;
        config.mirror.VMirror = true;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }


    }
    else
    {
        //Set Blit Mirror:
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {
            config.type = V4L2_G2D_MIRROR;
            config.mirror.HMirror = sdrv->ge_blit_xmirror;
            config.mirror.VMirror = sdrv->ge_blit_ymirror;

            if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }

        }

        config.type = V4L2_G2D_DSTMIRROR;
        config.mirror.HMirror = false;
        config.mirror.VMirror = false;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }

    }


    //set ROP2 option Code:
    if(modified & SMF_WRITE_MASK_BITS){
        config.type = V4L2_G2D_ROP2;
        config.rop2.enable = sdrv->ge_rop_enable;
        config.rop2.eRopMode = sdrv->ge_rop_op;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }

}

static void
mstarSetEngineDrawState( StateModificationFlags      modified,
                         MSTARDriverData            *sdrv,
                         MSTARDeviceData            *sdev)
{
    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&config, 0, sizeof(config));
    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
    ext_controls.controls->ptr = (void *)&config;

    D_INFO("[DFB] %s(%d) src_colorkey_enabled=%d, dst_colorkey_enabled=%d, ge_blit_xmirror=%d, ge_blit_ymirror=%d, pid=%d \n",
        __FUNCTION__,__LINE__,sdrv->ge_src_colorkey_enabled, sdrv->ge_blit_dst_colorkey_enabled,sdrv->ge_blit_xmirror,sdrv->ge_blit_ymirror,getpid());

    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_COLORKEY))
    {
        config.type = V4L2_G2D_COLORKEY;
        config.colorkey.type = G2D_COLORKEY_SRC;
        config.colorkey.mode = G2D_CK_OP_EQUAL;
        config.colorkey.range.color_s.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_s.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_s.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_s.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.range.color_e.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_e.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_e.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_e.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.fmt = sdrv->dst_ge_format;
        config.colorkey.enable =  sdrv->ge_src_colorkey_enabled;

        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }

    if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY ))
    {

        config.type = V4L2_G2D_COLORKEY;
        config.colorkey.type = G2D_COLORKEY_DST;
        config.colorkey.mode = G2D_CK_OP_NOT_EQUAL;
        config.colorkey.range.color_s.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_s.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_s.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_s.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.range.color_e.a = (sdrv->src_ge_clr_key >>SHIFT24);
        config.colorkey.range.color_e.r = (sdrv->src_ge_clr_key >>SHIFT16)&BYTE_MASK;
        config.colorkey.range.color_e.g = (sdrv->src_ge_clr_key >>SHIFT8)&BYTE_MASK;
        config.colorkey.range.color_e.b = (sdrv->src_ge_clr_key)&BYTE_MASK;
        config.colorkey.fmt = sdrv->dst_ge_format;
        config.colorkey.enable =  sdrv->ge_src_colorkey_enabled;
        
        if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }

    }

    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR))
    {
        if(sdrv->ge_draw_alpha_blend_enabled)
        {
            config.type = V4L2_G2D_DFB;
            config.dfb.enable = true;
            config.dfb.flags = sdrv->ge_draw_bld_flags;
            config.dfb.eSrcDFBBldOP = sdrv->ge_src_blend;
            config.dfb.eDstDFBBldOP = sdrv->ge_dst_blend;
            config.dfb.color.a = sdrv->color.a;
            config.dfb.color.r = sdrv->color.r;
            config.dfb.color.g = sdrv->color.g;
            config.dfb.color.b = sdrv->color.b; 

            if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }


            //if alpha_compare_mode can work when DFB HW blend ,this patch should be removed
            if(sdrv->ge_render_alpha_cmp_enabled)
            {
                /* discuss with GE owner, only set enable false means MApi_GFX_EnableDFBBlending(FALSE) */
                config.type = V4L2_G2D_DFB;
                config.abl.enable = false;

                if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 )
                {
                    printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
                }

                /*  
                     MApi_GFX_SetAlpha = 
                     MApi_GFX_EnableAlphaBlending((MS_BOOL)abl.enable);
                     MApi_GFX_SetAlphaBlending((GFX_BlendCoef)abl.eBlendCoef,abl.constAlpha);
                     MApi_GFX_SetAlphaSrcFrom((GFX_AlphaSrcFrom)abl.eAlphaSrc);
                */
                config.type = V4L2_G2D_ABL;
                config.abl.enable = true;
                config.abl.eBlendCoef = G2D_COEF_ASRC;
                config.abl.constAlpha = sdrv->color.a;
                config.abl.eAlphaSrc = G2D_ABL_FROM_ASRC;
                
                if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) <0)
                {
                    printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
                }  
            }
        }
        else
        {
            /* discuss with GE owner, only set enable false means MApi_GFX_EnableDFBBlending(FALSE) */
            config.type = V4L2_G2D_DFB;
            config.abl.enable = false;

            if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls)<0)
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }  
        

            config.type = V4L2_G2D_ABL;
            config.abl.enable = false;
            config.abl.eBlendCoef = G2D_COEF_ONE;
            config.abl.eAlphaSrc = G2D_ABL_FROM_ASRC;

            if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }  
            
        }
    }
}

static void
mstarSetEngineShareState( StateModificationFlags     modified,
                          DFBAccelerationMask        support_accel,
                          MSTARDriverData           *sdrv,
                          MSTARDeviceData           *sdev )
{

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&config, 0, sizeof(config));
    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
    ext_controls.controls->ptr = (void *)&config;

    ////////////////////////////////////////////////////////////
    //Set Dst Buffer State, All ops Needed:
    if(modified & SMF_DESTINATION)
    {
        if(!sdev->b_hwclip)
        {

            struct v4l2_crop cr;
            cr.c.left = 0;
            cr.c.top = 0;
            cr.c.width = sdrv->dst_w - 1;
            cr.c.height = sdrv->dst_h - 1;
            if (ioctl(sdrv->fd, VIDIOC_S_CROP, &cr) < 0) {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }
            
        }
    }

    if((modified & SMF_CLIP) && sdev->b_hwclip)
    {

        struct v4l2_crop cr;
        cr.c.left = sdrv->clip.x1;
        cr.c.top = sdrv->clip.y1;
        cr.c.width = sdrv->clip.x2 - sdrv->clip.x1 + 1;
        cr.c.height = sdrv->clip.y2 - sdrv->clip.y1 + 1;

        if ( ioctl(sdrv->fd, VIDIOC_S_CROP, &cr) < 0 ) {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }

    //SetYUV Format:
    if(modified & (SMF_DESTINATION | SMF_SOURCE))
    {
        config.type = V4L2_G2D_DC_CSC_FMT;
        config.cscfmt.mode = (enum v4l2_g2d_rgb2yuv)dfb_config->mst_rgb2yuv_mode;
        config.cscfmt.outRange = G2D_YUV_OUT_255;
        config.cscfmt.inRange = G2D_YUV_IN_255;
        config.cscfmt.srcfmt = sdrv->src_ge_yuv_fmt;
        config.cscfmt.dstfmt = sdrv->dst_ge_yuv_fmt;

        if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }  
    }

    //Set Blit Palette:
    if(sdrv->ge_palette_enabled && sdev->num_entries)
    {
        config.type = V4L2_G2D_PALETTEOPT;
        config.palette.palStart = 0;
        config.palette.palEnd = sdev->num_entries  -1;
        config.palette.color = sdev->palette_tbl;

        if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }
    }

    //Set Intensity Palette:
    if(sdrv->ge_palette_intensity_enabled       &&
       dfb_config->mst_blink_frame_rate == 0)
    {

        int i;
        for(i = 0; i<sdev->num_intensity; i++)
        {
            config.type = V4L2_G2D_INTENSITY;
            config.intensity.fmt = V4L2_PIX_FMT_ARGB32;
            config.intensity.id = i;
            config.intensity.color = sdev->palette_tbl[i];

            if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
            {
                printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
            }
        }
    }

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
    const int cubeEdge = (COLOR_RANGE/cubeSize); 

    bool bCube[UNITCUBESIZE][UNITCUBESIZE][UNITCUBESIZE] = {false};

    bool bCheck = false;

    int r = 0, g = 0, b = 0, i = 0;

    // set a default color to the output
    dummy_entry->a = BYTE_MASK;
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
                   dummy_entry->r = (r*cubeEdge) + HALF_CUBEEDGE;
                   dummy_entry->g = (g*cubeEdge) + HALF_CUBEEDGE;
                   dummy_entry->b = (b*cubeEdge) + HALF_CUBEEDGE;

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

    enum v4l2_g2d_interface geInterface = G2D_INTERFACE_GE0;

    /* define the data member for v4l2 control */
    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
    ext_controls.controls->ptr = (void *)&config;

    D_INFO("[DFB] %s, state=%p,  flag=%x\n ", __FUNCTION__, state, state->blittingflags);

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

        D_INFO("[DFB] %s, dst format : %s\n", __FUNCTION__, dfb_pixelformat_name(state->dst.buffer->format) );
        switch( state->dst.buffer->format )
        {
            case DSPF_ARGB1555:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_ARGB555;
                    break;

            case DSPF_ARGB:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_ARGB32;
                    break;

            case DSPF_ABGR:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_ABGR32;
                    break;

            case DSPF_YVYU:
                    sdrv->dst_ge_yuv_fmt    = G2D_YUV_YVYU;
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_YUYV;
                    break;

            case DSPF_YUY2:
                    sdrv->dst_ge_yuv_fmt    = G2D_YUV_YUYV;
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_YUYV;
                    break;

            case DSPF_VYUY:
                    sdrv->src_ge_yuv_fmt    = G2D_YUV_VYUY;
                    sdrv->src_ge_format     = V4L2_PIX_FMT_YUYV;
                    break;

            case DSPF_UYVY:
                    sdrv->dst_ge_yuv_fmt    = G2D_YUV_UYVY;
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_YUYV;
                    break;

            case DSPF_LUT8:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_PAL8;
                    palette                 = dsurface->palette;
                    break;

            case DSPF_ARGB4444:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_ARGB444;
                    break;

            case DSPF_RGB16:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_RGB565;
                    break;

            case DSPF_A8:
                    sdrv->dst_ge_format     = V4L2_PIX_FMT_PAL8;
                    break;

            case DSPF_RGB32:

                    sdrv->dst_ge_format     = V4L2_PIX_FMT_ARGB32;
                    sdrv->ge_dest_rgb32_blit_enabled = 1;
                    break;

            case DSPF_BLINK12355:
                    printf("[DFB] %s(%d) GOP doesn't support BLINK since moore. Therefore, GE doesn't implement BLINK\n",__FUNCTION__,__LINE__);
                    break;

            case DSPF_BLINK2266:
                    printf("[DFB] %s(%d) GOP doesn't support BLINK since moore. Therefore, GE doesn't implement BLINK\n",__FUNCTION__,__LINE__);
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
            sdrv->dst_phys = _BusAddrToHalAddr(((u64)state->dst.phys_h << SHIFT32) | state->dst.phys);
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

        if(V4L2_PIX_FMT_PAL8 == sdrv->dst_ge_format)
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
                sdrv->src_phys = _BusAddrToHalAddr(((u64)state->src.phys_h << SHIFT32) | state->src.phys);
            }

            sdrv->ge_src_rgb32_blit_enabled = 0;

            D_INFO("[DFB] %s, src format : %s\n", __FUNCTION__, dfb_pixelformat_name(state->src.buffer->format) );
            switch( state->src.buffer->format )
            {
                case DSPF_ARGB1555:
                        sdrv->src_ge_format = V4L2_PIX_FMT_ARGB555;
                        break;

                case DSPF_ARGB:
                        sdrv->src_ge_format = V4L2_PIX_FMT_ARGB32;
                        break;

                case DSPF_ABGR:
                        sdrv->src_ge_format = V4L2_PIX_FMT_ABGR32;
                        break;

                case DSPF_YVYU:
                        sdrv->src_ge_yuv_fmt = G2D_YUV_YVYU;
                        sdrv->src_ge_format  = V4L2_PIX_FMT_YUYV;
                        break;

                case DSPF_YUY2:
                        sdrv->src_ge_yuv_fmt = G2D_YUV_YUYV;
                        sdrv->src_ge_format  = V4L2_PIX_FMT_YUYV;
                        break;

                case DSPF_VYUY:
                        sdrv->src_ge_yuv_fmt    = G2D_YUV_VYUY;
                        sdrv->src_ge_format     = V4L2_PIX_FMT_YUYV;
                        break;

                case DSPF_UYVY:
                        sdrv->src_ge_yuv_fmt = G2D_YUV_UYVY;
                        sdrv->src_ge_format  = V4L2_PIX_FMT_YUYV;
                        break;

                case DSPF_LUT8:
                        sdrv->src_ge_format = V4L2_PIX_FMT_PAL8;

                        if( NULL == palette )
                            palette = ssurface->palette;

                        break;

                case DSPF_LUT4:
                        palette_intensity_format    = DSPF_LUT4;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I4_CNT;
                        sdrv->src_ge_format         = V4L2_PIX_FMT_I4;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT2:
                        palette_intensity_format    = DSPF_LUT2;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I2_CNT;
                        sdrv->src_ge_format         = V4L2_PIX_FMT_I2;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT1:
                        palette_intensity_format    = DSPF_LUT1;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I1_CNT;
                        sdrv->src_ge_format         = V4L2_PIX_FMT_I1;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_ARGB4444:
                        sdrv->src_ge_format = V4L2_PIX_FMT_ARGB444;
                        break;

                case DSPF_RGB16:
                        sdrv->src_ge_format = V4L2_PIX_FMT_RGB565X; //V4L2_PIX_FMT_RGB565
                        break;

                case DSPF_A8:
                        sdrv->src_ge_format = V4L2_PIX_FMT_PAL8;
                        break;

                case DSPF_RGB32:
                        sdrv->ge_src_rgb32_blit_enabled = 1;
                        sdrv->src_ge_format = V4L2_PIX_FMT_ARGB32;
                        break;

            case DSPF_BLINK12355:
                        printf("[DFB] %s(%d) GOP doesn't support BLINK since moore. Therefore, GE doesn't implement BLINK\n",__FUNCTION__,__LINE__);
                        break;

            case DSPF_BLINK2266:
                        printf("[DFB] %s(%d) GOP doesn't support BLINK since moore. Therefore, GE doesn't implement BLINK\n",__FUNCTION__,__LINE__);
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


            memset(sdev->palette_tbl, BYTE_MASK, (sizeof(struct v4l2_argb_color)*MAX_PALETTE_ENTRY_CNT));

            for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
            {
                sdev->palette_tbl[i].a = i;
            }

            sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
            sdrv->ge_palette_enabled = 1;
            sdev->a8_palette = true;
        }

        if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY))
        {
            sdrv->ge_src_colorkey_enabled = ((state->blittingflags & DSBLIT_SRC_COLORKEY) ? 1 : 0);
    D_INFO("yuhan11 YAYA,enable=%d\n", sdrv->ge_src_colorkey_enabled );
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
            sdrv->ge_rop_enable = true;
            switch(state->rop_op)
            {
                case DFB_ROP2_OP_ZERO:
                    sdrv->ge_rop_op = G2D_ROP2_OP_ZERO;
                    break;

                case DFB_ROP2_NOT_PS_OR_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NOT_PS_OR_PD;
                    break;

                case DFB_ROP2_NS_AND_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NS_AND_PD;
                    break;

                case DFB_ROP2_NS:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NS;
                    break;

                case DFB_ROP2_PS_AND_ND:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PS_AND_ND;
                    break;

                case DFB_ROP2_ND:
                    sdrv->ge_rop_op = G2D_ROP2_OP_ND;
                    break;

                case DFB_ROP2_PS_XOR_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PS_XOR_PD;
                    break;

                case DFB_ROP2_NOT_PS_AND_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NOT_PS_AND_PD;
                    break;

                case DFB_ROP2_PS_AND_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PS_AND_PD;
                    break;

                case DFB_ROP2_NOT_PS_XOR_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NOT_PS_XOR_PD;
                    break;

                case DFB_ROP2_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PD;
                    break;

                case DFB_ROP2_NS_OR_PD:
                    sdrv->ge_rop_op = G2D_ROP2_OP_NOT_PS_XOR_PD;
                    break;

                case DFB_ROP2_PS:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PS;
                    break;

                case DFB_ROP2_PS_OR_ND:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PS_OR_ND;
                    break;

                case DFB_ROP2_PD_OR_PS:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PD_OR_PS;
                    break;

                case DFB_ROP2_ONE:
                    sdrv->ge_rop_op = G2D_ROP2_OP_ONE;
                    break;

                case DFB_ROP2_NONE:
                    //disable ROP option
                    //gfx enum no DFB_ROP2_NONE option
                    sdrv->ge_rop_op = G2D_ROP2_OP_PD;
                    sdrv->ge_rop_enable = false;
                    break;

                default:
                    sdrv->ge_rop_op = G2D_ROP2_OP_PD;
                    sdrv->ge_rop_enable = false;
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
            sdev->palette_tbl[i].a = palette->entries[i].a;
            sdev->palette_tbl[i].r = palette->entries[i].r;
            sdev->palette_tbl[i].g = palette->entries[i].g;
            sdev->palette_tbl[i].b = palette->entries[i].b;
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

            color_original.a = sdev->palette_tbl[clr_key_index].a;
            color_original.r = sdev->palette_tbl[clr_key_index].r;
            color_original.g = sdev->palette_tbl[clr_key_index].g;
            color_original.b = sdev->palette_tbl[clr_key_index].b;

            // Find a dummy color
            bool ret = false;
            int cubeSize = CUBESIZE_INIT;

            for ( cubeSize = CUBESIZE_INIT; ret == false && cubeSize < CUBESIZE_BOUNDARY; cubeSize++)
            {
                ret = FindDummyColor( palette,
                                      &color_dummy,
                                      color_original,
                                      cubeSize );
            }

            // Modify Palette Table & assign a dummy color key
            sdev->palette_tbl[clr_key_index].a = color_dummy.a;
            sdev->palette_tbl[clr_key_index].r = color_dummy.r;
            sdev->palette_tbl[clr_key_index].g = color_dummy.g;
            sdev->palette_tbl[clr_key_index].b = color_dummy.b;

            sdrv->src_ge_clr_key = color_dummy.a << SHIFT24 |
                                   color_dummy.r << SHIFT16 |
                                   color_dummy.g <<  SHIFT8 |
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

            sdev->palette_intensity[i].color.a = (color >>SHIFT24);
            sdev->palette_intensity[i].color.r = (color >>SHIFT16)&BYTE_MASK;
            sdev->palette_intensity[i].color.g = (color >>SHIFT8)&BYTE_MASK;
            sdev->palette_intensity[i].color.b = (color)&BYTE_MASK;
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



    // GE begin draw, 
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    if ( ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
         printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    if(state->destination->config.caps & DSCAPS_SECURE_MODE)
    {
        D_INFO("[DFB] %s Secure mode!\n",__FUNCTION__);
        geInterface = G2D_INTERFACE_GE1;
    }
    else
    {
        D_INFO("[DFB] %s Non-secure mode!\n",__FUNCTION__);
        geInterface = G2D_INTERFACE_GE0;
    }
    ext_controls.controls->id = V4L2_CID_EXT_G2D_SET_INTERFACE;
    ext_controls.controls->ptr = (void *)&geInterface;
    if(ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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
        ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
        config.type = V4L2_G2D_DITHER;
        config.enable = true;

        if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }

    }
    else
    {
        ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
        config.type = V4L2_G2D_DITHER;
        config.enable = false;

        if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
        {
            printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
        }           

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
            (sdrv->src_ge_format != V4L2_PIX_FMT_YVYU &&
             sdrv->src_ge_format != V4L2_PIX_FMT_YUYV &&
             sdrv->src_ge_format != V4L2_PIX_FMT_UYVY) &&
            (sdrv->dst_ge_format == V4L2_PIX_FMT_YVYU ||
             sdrv->dst_ge_format == V4L2_PIX_FMT_YUYV ||
             sdrv->dst_ge_format == V4L2_PIX_FMT_UYVY))
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

    // GE end draw
    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }         

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
    MSTARDeviceData *sdev = dev;
    MSTARDriverData *sdrv = drv;

    u32 y=0, cb=0, cr=0;

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&render_info;


    render_info.dst.addr = sdrv->dst_phys;
    render_info.dst.width = sdrv->dst_w;
    render_info.dst.height = sdrv->dst_h;
    render_info.dst.pitch = sdrv->dst_pitch;
    render_info.dst.fmt = sdrv->dst_ge_format;


    render_info.type = V4L2_G2D_RECTFILL;
    render_info.rect.dstBlock.x = rect->x;
    render_info.rect.dstBlock.y = rect->y;
    render_info.rect.dstBlock.width =rect->w;
    render_info.rect.dstBlock.height = rect->h;
    render_info.rect.colorRange.color_s.a = sdrv->color.a;
    render_info.rect.colorRange.color_s.r = sdrv->color.r;
    render_info.rect.colorRange.color_s.g = sdrv->color.g;
    render_info.rect.colorRange.color_s.b = sdrv->color.b;
    render_info.rect.colorRange.color_e.a = sdrv->color.a;
    render_info.rect.colorRange.color_e.r = sdrv->color.r;
    render_info.rect.colorRange.color_e.g = sdrv->color.g;
    render_info.rect.colorRange.color_e.b = sdrv->color.b;
    render_info.rect.drawflag = 0;

    switch(sdrv->dst_ge_format)
    {
        case V4L2_PIX_FMT_YVYU:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
            render_info.rect.colorRange.color_s.a= BYTE_MASK;
            render_info.rect.colorRange.color_s.r = cr;
            render_info.rect.colorRange.color_s.g = y;
            render_info.rect.colorRange.color_s.b = cb;
            break;

        default:
            render_info.rect.colorRange.color_s.a = sdrv->color.a;
            render_info.rect.colorRange.color_s.r = sdrv->color.r;
            render_info.rect.colorRange.color_s.g = sdrv->color.g;
            render_info.rect.colorRange.color_s.b= sdrv->color.b;
            break;
    }


    if(sdrv->dflags & (DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(sdrv->dst_ge_format)
        {
            case V4L2_PIX_FMT_YVYU:
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_UYVY:
                    RGB_TO_YCBCR( sdrv->color2.r,
                                  sdrv->color2.g,
                                  sdrv->color2.b,
                                  y,
                                  cb,
                                  cr );

                    render_info.rect.colorRange.color_e.a= BYTE_MASK;
                    render_info.rect.colorRange.color_e.r = cr;
                    render_info.rect.colorRange.color_e.g = y;
                    render_info.rect.colorRange.color_e.b = cb;
                    break;

            default:
                    render_info.rect.colorRange.color_e.a = sdrv->color2.a;
                    render_info.rect.colorRange.color_e.r = sdrv->color2.r;
                    render_info.rect.colorRange.color_e.g = sdrv->color2.g;
                    render_info.rect.colorRange.color_e.b = sdrv->color2.b;
                    break;
        }

        if(sdrv->dflags & DSDRAW_COLOR_GRADIENT_X)
            render_info.rect.drawflag = G2D_RECT_FLAG_COLOR_GRADIENT_X ;
        else
            render_info.rect.drawflag = 0;

        if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            render_info.rect.drawflag |= G2D_RECT_FLAG_COLOR_GRADIENT_Y;
    }
    else
        render_info.rect.drawflag = 0;



    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = render_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    return true;
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

                triInfo.colorRange.color_s.a = BYTE_MASK;
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

                   triInfo.colorRange.color_e.a = BYTE_MASK;
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
                spanInfo.colorRange.color_s.a = BYTE_MASK;
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

                    spanInfo.colorRange.color_e.a = BYTE_MASK;
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

    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    u32 y=0, cb=0, cr=0;

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }
 
    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&render_info;


    render_info.dst.addr = sdrv->dst_phys;
    render_info.dst.width = sdrv->dst_w;
    render_info.dst.height = sdrv->dst_h;
    render_info.dst.pitch = sdrv->dst_pitch;
    render_info.dst.fmt = sdrv->dst_ge_format;

    render_info.type = V4L2_G2D_DRAW_LINE;
    render_info.line.x1 = rect->x;
    render_info.line.y1 = rect->y;
    render_info.line.x2 = rect->x+rect->w - 1;
    render_info.line.y2 = rect->y;
    render_info.line.width= LINE_WIDTH;
    render_info.line.colorRange.color_s.a = sdrv->color.a;
    render_info.line.colorRange.color_s.r = sdrv->color.r;
    render_info.line.colorRange.color_s.g = sdrv->color.g;
    render_info.line.colorRange.color_s.b = sdrv->color.b;
    render_info.line.colorRange.color_e.a = sdrv->color.a;
    render_info.line.colorRange.color_e.r = sdrv->color.r;
    render_info.line.colorRange.color_e.g = sdrv->color.g;
    render_info.line.colorRange.color_e.b = sdrv->color.b;
    render_info.line.drawflag = 0;
    render_info.line.pattern.enable= 0;
    render_info.line.reset=0;

    // draw 4 line to construct the rectangle
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }


    render_info.line.x1 = rect->x + rect->w - 1;
    render_info.line.y2 = rect->y + rect->h - 1;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    
    render_info.line.y1 = rect->y + rect->h - 1;
    render_info.line.x2 = rect->x;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }


    render_info.line.x1 = rect->x;
    render_info.line.y2 = rect->y;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = render_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }


    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&render_info;


    render_info.dst.addr = sdrv->dst_phys;
    render_info.dst.width = sdrv->dst_w;
    render_info.dst.height = sdrv->dst_h;
    render_info.dst.pitch = sdrv->dst_pitch;
    render_info.dst.fmt = sdrv->dst_ge_format;


    render_info.type = V4L2_G2D_DRAW_OVAL;
    render_info.oval.lineWidth = oval->line_width;
    render_info.oval.color.a = sdrv->color.a;
    render_info.oval.color.r = sdrv->color.r;
    render_info.oval.color.g = sdrv->color.g;
    render_info.oval.color.b = sdrv->color.b;
    render_info.oval.dstBlock.width = oval->oval_rect.w;
    render_info.oval.dstBlock.height = oval->oval_rect.h;
    render_info.oval.dstBlock.x = oval->oval_rect.x;
    render_info.oval.dstBlock.y = oval->oval_rect.y;    

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = render_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0)
    {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&render_info;


    render_info.dst.addr = sdrv->dst_phys;
    render_info.dst.width = sdrv->dst_w;
    render_info.dst.height = sdrv->dst_h;
    render_info.dst.pitch = sdrv->dst_pitch;
    render_info.dst.fmt = sdrv->dst_ge_format;

    //draw line
    render_info.type = V4L2_G2D_DRAW_LINE;
    render_info.line.x1 = line->x1;
    render_info.line.y1 = line->y1;
    render_info.line.x2 = line->x2;
    render_info.line.y2 = line->y2;
    render_info.line.width= width;
    render_info.line.colorRange.color_s.a = sdrv->color.a;
    render_info.line.colorRange.color_s.r = sdrv->color.r;
    render_info.line.colorRange.color_s.g = sdrv->color.g;
    render_info.line.colorRange.color_s.b = sdrv->color.b;
    render_info.line.colorRange.color_e.a = sdrv->color.a;
    render_info.line.colorRange.color_e.r = sdrv->color.r;
    render_info.line.colorRange.color_e.g = sdrv->color.g;
    render_info.line.colorRange.color_e.b = sdrv->color.b;
    render_info.line.drawflag = 0;
    render_info.line.pattern.enable= 0;
    render_info.line.reset=0;
    
    switch(sdrv->dst_ge_format)
    {
        case V4L2_PIX_FMT_YVYU:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
            render_info.line.colorRange.color_s.a= BYTE_MASK;
            render_info.line.colorRange.color_s.r = cr;
            render_info.line.colorRange.color_s.g = y;
            render_info.line.colorRange.color_s.b = cb;
            break;

        default:
            render_info.line.colorRange.color_s.a = sdrv->color.a;
            render_info.line.colorRange.color_s.r = sdrv->color.r;
            render_info.line.colorRange.color_s.g = sdrv->color.g;
            render_info.line.colorRange.color_s.b= sdrv->color.b;
            break;
    }

    if(sdrv->dflags &(DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(sdrv->dst_ge_format)
        {
             case V4L2_PIX_FMT_YVYU:
             case V4L2_PIX_FMT_YUYV:
             case V4L2_PIX_FMT_UYVY:
                 RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
                 render_info.line.colorRange.color_e.a= BYTE_MASK;
                 render_info.line.colorRange.color_e.r = cr;
                 render_info.line.colorRange.color_e.g = y;
                 render_info.line.colorRange.color_e.b = cb;
                 break;

            default:
                 render_info.line.colorRange.color_e.a = sdrv->color.a;
                 render_info.line.colorRange.color_e.r = sdrv->color.r;
                 render_info.line.colorRange.color_e.g = sdrv->color.g;
                 render_info.line.colorRange.color_e.b= sdrv->color.b;
                 break;
        }

    }

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = render_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  blit_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&blit_info, 0, sizeof(blit_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    /* set src and dst, ex: addr, width and height... */
    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&blit_info;

    blit_info.src.addr = sdrv->src_phys;
    blit_info.src.width = sdrv->src_w;
    blit_info.src.height = sdrv->src_h;
    blit_info.src.pitch = sdrv->src_pitch;
    blit_info.src.fmt = sdrv->src_ge_format;

    blit_info.dst.addr = sdrv->dst_phys;
    blit_info.dst.width = sdrv->dst_w;
    blit_info.dst.height = sdrv->dst_h;
    blit_info.dst.pitch = sdrv->dst_pitch;
    blit_info.dst.fmt = sdrv->dst_ge_format;

#if 0
    D_INFO("[DFB] %s, blit_info, src(w=%d, h=%d, pitch=%d, fmt=%s), dst(w=%d, h=%d, pitch=%d, fmt=%s)\n",
		__FUNCTION__, blit_info.src.width, blit_info.src.height, blit_info.src.pitch, (blit_info.src.fmt == V4L2_PIX_FMT_ARGB32)? "ARGB32" : "not-ARGB32",
		blit_info.dst.width, blit_info.dst.height, blit_info.dst.pitch, (blit_info.dst.fmt == V4L2_PIX_FMT_ARGB32)? "ARGB32" : "not-ARGB32" );
#endif
    /* set blit info, ex:blit area*/
    blit_info.type = V4L2_G2D_BITBLT;

    blit_info.bitblt.srcblock.x = rect->x;
    blit_info.bitblt.srcblock.y = rect->y;
    blit_info.bitblt.srcblock.width = rect->w;
    blit_info.bitblt.srcblock.height = rect->h;
    blit_info.bitblt.dstblock.x = dx;
    blit_info.bitblt.dstblock.y = dy;
    blit_info.bitblt.dstblock.width =rect->w;
    blit_info.bitblt.dstblock.height = rect->h;
    blit_info.bitblt.drawflag = 0;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = blit_info.tagID;
    //D_INFO("yuhan  sdrv->tagid=%d\n",sdrv->tagid);

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }		

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

    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  blit_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&blit_info, 0, sizeof(blit_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    /* set src and dst, ex: addr, width and height... */
    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&blit_info;

    blit_info.src.addr = sdrv->src_phys;
    blit_info.src.width = sdrv->src_w;
    blit_info.src.height = sdrv->src_h;
    blit_info.src.pitch = sdrv->src_pitch;
    blit_info.src.fmt = sdrv->src_ge_format;

    blit_info.dst.addr = sdrv->dst_phys;
    blit_info.dst.width = sdrv->dst_w;
    blit_info.dst.height = sdrv->dst_h;
    blit_info.dst.pitch = sdrv->dst_pitch;
    blit_info.dst.fmt = sdrv->dst_ge_format;

    blit_info.type = V4L2_G2D_BITBLT;

    blit_info.bitblt.srcblock.x = srect->x;
    blit_info.bitblt.srcblock.y = srect->y;
    blit_info.bitblt.srcblock.width = srect->w;
    blit_info.bitblt.srcblock.height = srect->h;
    blit_info.bitblt.dstblock.x = drect->x;
    blit_info.bitblt.dstblock.y = drect->y;
    blit_info.bitblt.dstblock.width =drect->w;
    blit_info.bitblt.dstblock.height = drect->h;
    blit_info.bitblt.drawflag = 1;

    if(sdrv->src_ge_format == V4L2_PIX_FMT_YVYU || sdrv->src_ge_format == V4L2_PIX_FMT_YUYV || sdrv->src_ge_format == V4L2_PIX_FMT_UYVY)
    {
        blit_info.bitblt.srcblock.width = blit_info.bitblt.srcblock.width & ~1;
    }

    if(sdrv->dst_ge_format == V4L2_PIX_FMT_YVYU || sdrv->dst_ge_format == V4L2_PIX_FMT_YUYV || sdrv->dst_ge_format == V4L2_PIX_FMT_UYVY)
    {
        blit_info.bitblt.dstblock.width = (blit_info.bitblt.dstblock.width + 1) & ~1;
    }


    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = blit_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_GET_RESOURCE;
    ext_controls.controls->ptr = (void *)&config;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }


    ext_controls.controls->id = V4L2_CID_EXT_G2D_RENDER;
    ext_controls.controls->ptr = (void *)&render_info;

    render_info.src.addr = sdrv->src_phys;
    render_info.src.width = sdrv->src_w;
    render_info.src.height = sdrv->src_h;
    render_info.src.pitch = sdrv->src_pitch;
    render_info.src.fmt = sdrv->src_ge_format;

    render_info.dst.addr = sdrv->dst_phys;
    render_info.dst.width = sdrv->dst_w;
    render_info.dst.height = sdrv->dst_h;
    render_info.dst.pitch = sdrv->dst_pitch;
    render_info.dst.fmt = sdrv->dst_ge_format;

    render_info.type = V4L2_G2D_BITBLT;

    render_info.bitblt.srcblock.x = srect->x;
    render_info.bitblt.srcblock.y = srect->y;
    render_info.bitblt.srcblock.width = srect->w;
    render_info.bitblt.srcblock.height = srect->h;
    render_info.bitblt.dstblock.x = drect->x;
    render_info.bitblt.dstblock.y = drect->y;
    render_info.bitblt.dstblock.width =drect->w;
    render_info.bitblt.dstblock.height = drect->h;
    render_info.bitblt.drawflag = 1;

    if(sdrv->src_ge_format == V4L2_PIX_FMT_YVYU || sdrv->src_ge_format == V4L2_PIX_FMT_YUYV || sdrv->src_ge_format == V4L2_PIX_FMT_UYVY)
    {
        render_info.bitblt.srcblock.width = render_info.bitblt.srcblock.width & ~1;
    }

    if(sdrv->dst_ge_format == V4L2_PIX_FMT_YVYU || sdrv->dst_ge_format == V4L2_PIX_FMT_YUYV || sdrv->dst_ge_format == V4L2_PIX_FMT_UYVY)
    {
        render_info.bitblt.dstblock.width = (render_info.bitblt.dstblock.width + 1) & ~1;
    }


    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

    sdrv->tagid = render_info.tagID;

    ext_controls.controls->id = V4L2_CID_EXT_G2D_FREE_RESOURCE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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

    printf("[DFB] Do not support BlitTrapezoid\n");

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
    D_DEBUG_AT( MTK_STI_Driver, "[DFB] %s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);

    //return dfb_gfxcard_get_accelerator( device ) == 0x2D47;
    return 1;
}


static void
driver_get_info( CoreGraphicsDevice                 *device,
                 GraphicsDriverInfo                 *info )
{
    D_DEBUG_AT( MTK_STI_Driver, "[DFB] %s()\n", __FUNCTION__ );

    /* fill driver info structure */
    snprintf( info->name,
              DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
              "MTK STI Driver" );

    snprintf( info->vendor,
              DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
              "MTK corp." );

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
    serial->serial = sdrv->tagid;
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

    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    
    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_WAIT_DONE;
    ext_controls.controls->p_u16 = (u16 *)&serial->serial;

    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0 ) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

     return DFB_OK;
}


DFBResult
mstarWaitSerial_null( void                          *driver_data,
                      void                          *device_data,
                      const CoreGraphicsSerial      *serial )
{
     return DFB_OK;
}

int get_device_path(char *path)
{
    const char device[] = "/sys/devices/platform/1c482000.mtk-g2d/g2d_device_num";
    char buf[BUF_SIZE] = "";
    ssize_t len = 0;
    int fd = -1;
    int ret = 0;

    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("[DFB]: open %s fail\n", device);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    len = read(fd, buf, sizeof(buf)-1);
    close(fd);
    ret = snprintf(path, PATH_SIZE, "%s%s", "/dev/video", buf);
    if (ret < 0) {
        printf("[DFB] %s, snprintf fail with error : %s\n", __FUNCTION__, strerror(errno));
        return ret;
    }
    
    return 0;
}

static DFBResult
mstar_init_driver( CoreGraphicsDevice               *device,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;

    D_DEBUG_AT( MTK_STI_Driver, "%s()\n", __FUNCTION__ );
    DFB_CHECK_POINT("mstar_init_driver start");
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_START, DF_BOOT_LV5);
    //Init GOP:
    //Get GOP Idx:
    sdev->gfx_gop_index = dfb_config->mst_gfx_gop_index;

    // call sti_drm init device
    mstar_init_gop_driver(device, driver_data);



    /* 
        Open a file, start using device, GE owner says /dev/video0 has been occupied, 
        therefore DFB need to use /dev/video30 
    */
    char DEVICE_PATH[PATH_SIZE] = "";
    int ret = get_device_path(DEVICE_PATH);
    if (ret != 0 ) {
        printf("[DFB] %s, can't get device path\n", __FUNCTION__);
        return DFB_INIT;
    }

    int fd = open(DEVICE_PATH, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1) {
        int err = errno;
        printf("\33[0;33;44m[DFB] %s (%d), open %s error, please ask v4l2 owner to fix this problem!!! \33[0m errno=%d\n",
			__FUNCTION__,__LINE__, DEVICE_PATH, err);
        return DFB_INIT;
    }

    sdrv->fd = fd;
    D_INFO("[DFB] open %s, fd=%d, pid=%d\n", DEVICE_PATH, sdrv->fd, getpid());

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

    sdrv->ge_src_blend      = G2D_DFB_BLD_OP_ONE;
    sdrv->ge_dst_blend      = G2D_DFB_BLD_OP_ONE;
    sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_NONE;
    sdrv->src_ge_yuv_fmt    = G2D_YUV_YUYV;
    sdrv->dst_ge_yuv_fmt    = G2D_YUV_YUYV;

    //Set Rop2
    sdrv->ge_rop_op = G2D_ROP2_OP_PD;
    sdrv->ge_rop_enable = false;



    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_END, DF_BOOT_LV5);

    DFB_CHECK_POINT("mstar_init_driver done");

    return DFB_OK;
}


static DFBResult
mstar_init_device ( CoreGraphicsDevice              *device,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;

    // sti drm init device.
    mstar_init_gop_device(device, driver_data);

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

    struct v4l2_ext_controls  ext_controls;
    struct v4l2_ext_control  ext_control;
    struct v4l2_ext_g2d_render_info  render_info;
    struct v4l2_ext_g2d_config  config;
    memset(&ext_controls, 0, sizeof(ext_controls));
    memset(&ext_control, 0, sizeof(ext_control));
    memset(&render_info, 0, sizeof(render_info));
    memset(&config, 0, sizeof(config));

    ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ext_controls.count = 1;
    ext_controls.controls = &ext_control;
    ext_controls.controls->id = V4L2_CID_EXT_G2D_CTRL;
    ext_controls.controls->ptr = (void *)&config;

    config.type = V4L2_G2D_FLUSHQUEUE;
    if (ioctl(sdrv->fd, VIDIOC_S_EXT_CTRLS, &ext_controls) < 0) {
        printf("\33[0;33;44m[DFB] %s (%d), call ioctl fail , please ask v4l2 owner to fix this problem!!! \33[0m errno=%d, (%m), pid=%d\n", __FUNCTION__,__LINE__, errno,getpid());
    }

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

    D_DEBUG_AT( MTK_STI_Driver, "%s()\n", __FUNCTION__ );
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DRIVER, DF_MEASURE_START, DF_BOOT_LV5);

    /* Keep pointer to shared device data. */
    sdrv->dev = device_data;

    /* Keep core and device pointer. */
    sdrv->core   = core;
    sdrv->device = device;


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
    D_DEBUG_AT( MTK_STI_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);
    //  printf("MAdp_SYS_GetPanelResolution-->%d, %d\n", pnlW, pnlH);
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DEVICE, DF_MEASURE_START, DF_BOOT_LV5);

    for(i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {
        sdev->layer_active[i]       = false;
        sdev->layer_pinpon_mode[i]  = false;
        sdev->layer_refcnt[i]       = 0x0;
    }

    memset(sdev->mstarLayerBuffer, 0, sizeof(sdev->mstarLayerBuffer));

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

    sdev->gfx_max_width  &= CLEAR3;

    sdev->gfx_h_offset          = dfb_config->mst_gfx_h_offset;
    sdev->gfx_v_offset          = dfb_config->mst_gfx_v_offset;

    //Init A8 GE Palette default:
    memset(sdev->palette_tbl, BYTE_MASK, (sizeof(struct v4l2_argb_color)*MAX_PALETTE_ENTRY_CNT));

    for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
    {
        sdev->palette_tbl[i].a = i;
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
        device_info->limits.surface_byteoffset_alignment = BYTEOFFSET_ALIGNMENT;
    }

    device_info->limits.surface_bytepitch_alignment  = BYTEPITCH_ALIGNMENT;
    device_info->limits.surface_hegiht_alignment     = HEIGHT_ALIGNMENT;

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

    D_DEBUG_AT( MTK_STI_Driver, "%s()\n", __FUNCTION__ );

    /* Destroy BEU lock. */
    fusion_skirmish_destroy( &sdev->beu_lock );
}


static void
driver_close_driver( CoreGraphicsDevice             *device,
                     void                           *driver_data )
{
     MSTARDriverData *sdrv = driver_data;

     D_DEBUG_AT( MTK_STI_Driver, "%s()\n", __FUNCTION__ );

     if (gGlesFuncs.GLES2Destroy)
         gGlesFuncs.GLES2Destroy();

     mstar_close_driver( driver_data );

}
