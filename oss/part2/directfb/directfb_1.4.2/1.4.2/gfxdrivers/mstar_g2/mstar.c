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
#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/palette.h>
#include <core/layers_internal.h>

#include <gfx/convert.h>

#include <MsCommon.h>

#include <drvSEM.h>
#include <drvMMIO.h>
#include <drvMIU.h>
#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <drvMVOP.h>
#include <drvTVEncoder.h>

#include <apiGFX.h>
#include <apiGOP.h>
#include <apiXC.h>
#include <apiPNL.h>
#include <drvCMDQ.h>
#include <drvSYS.h>

#include "mstar_layer.h"
#include "mstar_screen.h"

// define dynamic check gles lib
#include "mstar_gles2.h"

#define MSTAR_DFB_BLT_MIRROR_PATCH              1
#define LINE_WIDTH                              1
#define DEFAULT_MAX_GFX_ROTATE_LIMIT            4096

#define MSTAR_GFX_RENDER_OP_NONE                0x0
#define MSTAR_GFX_RENDER_OP_DRAW                0x1
#define MSTAR_GFX_RENDER_OP_BLIT                0x2

#define OSD2VE_SUPPORT        //RC@20100212, Support the OP direct output to VE display


#define UTOPIA_PACKED                   1
#define UTOPIA_UNPACKED               0

#ifdef CONFIG_UTOPIA_FRAMEWORK_KERNEL_DRIVER_STRUCT_PACKED
#define UTOPIA_PACKED_STATE      UTOPIA_PACKED
#else
#define UTOPIA_PACKED_STATE      UTOPIA_UNPACKED
#endif


#pragma weak  MApi_GFX_TriFill
#pragma weak  MApi_GFX_SpanFill

#pragma weak  MDrv_CMDQ_Init
#pragma weak  MDrv_CMDQ_Get_Memory_Size
#pragma weak  MDrv_SYS_GlobalInit


typedef enum
{
   MSTAR_FILL_RECT          = 0x00000001,
   MSTAR_DRAW_RECT          = 0x00000002,
   MSTAR_DRAW_LINE          = 0x00000004,
   MSTAR_TRAPEZOID_FILL     = 0x00000008,
   MSTAR_DRAW_OVAL          = 0x00000010,
   MSTAR_BIT_BLIT           = 0x00010000,
   MSTAR_STRETCH            = 0x00020000,
   MSTAR_TRAPEZOID_BLIT     = 0x00040000,
   MSTAR_OPT_MAX            = 0xffffffff
} MSTAR_OP_TYPE;

bool mstar_GFX_Aconst_Multiply_Asrc_Ex( MSTARDriverData *sdrv);
bool mstar_GFXtoDFB_BlendFunction( MSTARDriverData *sdrv);

#define IS_MSTAR_BLIT_OP(op)  ((op) & 0xffff0000)

static void* pHandle_gles2 = NULL;

bool (*GLESInitFuncs)(GLESFuncs *funcs) = NULL;

static GLESFuncs glesFuncs;


static IDirectFBInternalMemBackend backend;


#if MSTAR_DFB_BLT_MIRROR_PATCH
#define SET_BLT_MIRROR_SRCX(sdrv, rectinfo)                 \
    if (sdrv->ge_blit_xmirror)                              \
    {                                                       \
        rectinfo.srcblk.x += rectinfo.srcblk.width - 1;     \
    }

#define SET_BLT_MIRROR_SRCY(sdrv,rectinfo)                      \
    if (dfb_config->do_GE_Vmirror == true)                      \
    {                                                           \
        if (!sdrv->ge_blit_ymirror)                             \
        {                                                       \
            rectinfo.srcblk.y += rectinfo.srcblk.height - 1;    \
        }                                                       \
    }                                                           \
    else                                                        \
    {                                                           \
        if (sdrv->ge_blit_ymirror)                              \
        {                                                       \
            rectinfo.srcblk.y += rectinfo.srcblk.height - 1;    \
        }                                                       \
    }  
#else

#define SET_BLT_MIRROR_X(sdrv, rectinfo) do{ } while(0)
#define SET_BLT_MIRROR_Y(sdrv, rectinfo) do{ } while(0)

#endif


DFB_GRAPHICS_DRIVER( mstar_g2 )


D_DEBUG_DOMAIN( MSTAR_G2_Driver, "MSTAR_G2/Driver", "MSTAR G2 Driver" );


#define PIXEL_CHN_MASK(st, bits)                                \
                        (((1UL << bits) - 1) << st )

#define PIXEL_CHN_VAL(pixel, st, bits)                          \
                        ((((u32)pixel) & PIXEL_CHN_MASK(st, bits)) >> st )


#define PIXEL_CHN_NORMAL(pixel, st, bits)                       \
                       (PIXEL_CHN_VAL(pixel, st, bits) << (8 - bits))



void
Global_Init()
{
#ifndef DFB_AARCH64
    if (dfb_config->mst_use_dlopen_dlsym)
    {
        typedef void  (*MDRV_SYS_GLOBALINIT) (void);
        typedef int (*MDRV_SYS_PACKMODE) (void); //[TODO:  Need to confirm the final function prototype with Kernel Team

        MDRV_SYS_GLOBALINIT MDrv_SYS_GlobalInit = NULL;
        MDRV_SYS_PACKMODE MDrv_Sys_PackMode = NULL;

        void *handle = NULL;
        int dfb_packed_state = UTOPIA_PACKED_STATE;

        handle = dlopen ("liblinux.so", RTLD_LAZY);

        if (!handle)
        {
            printf("[DFB][%s (%s)] dlopen liblinux.so failed  error:%s \n", __FUNCTION__, __FILE__,dlerror());
            return;
        }

        SAFE_GET_SYMBOL(handle, "MDrv_SYS_GlobalInit", MDrv_SYS_GlobalInit);

        MDrv_SYS_GlobalInit();
        dlclose(handle);
    }
    else
    {
        if (MDrv_SYS_GlobalInit != NULL)
        {
            D_INFO("[DFB][%s (%s) %d] Get fun MDrv_SYS_GlobalInit\n", __FUNCTION__, __FILE__, __LINE__);
            MDrv_SYS_GlobalInit();
        }
        else
        {
            printf("[DFB][%s (%s) %d] Get fun MDrv_SYS_GlobalInit failed\n", __FUNCTION__, __FILE__, __LINE__);
            return;
        }
    }

#if 0 // We disable for now just  because Utopia is not ready for Version Check Mechanism.
    /* Check Utopia packed/unpacked version

        Here are the rules:
        A. Check the returned value of "MDrv_Sys_PackMode" which is that =>  0: UNPACKED, 1: PACKED
        B. If there is no function symbol "MDrv_Sys_PackMode" defined in "liblinux.so", it means Utopia is "UNPACKED".

    */
    MDrv_Sys_PackMode = dlsym (handle, "MDrv_Sys_PackMode");

    if (MDrv_Sys_PackMode)
    {
        /* Check rule "A" */

        int ret = MDrv_Sys_PackMode ();

        if (ret != dfb_packed_state)
        {
            D_ERROR("[DFB] ERROR!!! DFB & Utopia is MISMATCH: DFB (%s), Utopia (%s)\n",
                dfb_packed_state == 1 ? "PACKED" : "UNPACKED",
                ret == 1 ? "PACKED" : "UNPACKED");
        }
    }
    else
    {
        /* Check rule "B" */

        if (dfb_packed_state != UTOPIA_UNPACKED)
        {
            D_ERROR("[DFB] ERROR!!! DFB & Utopia is MISMATCH: DFB (PACKED), Utopia (UNPACKED)\n");
        }
    }
#endif

#else
    MDrv_SYS_GlobalInit();
#endif
}

static void
check_GLES2()
{
    if (pHandle_gles2 == NULL)
    {
        char            buf[64] = {0};
        const char     *path = MSTAR_GLES2_SUBMODULE_PATH;

        sprintf(buf, "%s/%s",  direct_config->module_dir, path);
        printf("[DFB] %s, %d, load %s !\n", __FUNCTION__, __LINE__, buf);
        pHandle_gles2 = dlopen (buf, RTLD_LAZY);

        if (pHandle_gles2)
        {
            SAFE_GET_SYMBOL(pHandle_gles2, "GLES_Func_init", GLESInitFuncs);
            bool ret = GLESInitFuncs(&glesFuncs);

            if (ret == false)
                printf("[DFB] EGL/GL Init Failed!\n");
        }
        else
        {
            printf("[DFB] %s, %d, can't find %s !error: %s\n", __FUNCTION__, __LINE__, buf, dlerror());
        }
    }

}


static void
MIUProtectThread(void)
{
    printf("\33[0;33;44m[DFB]\33[0m MIU Protect Thread ready!!! \n");

    while(1)
    {
        sleep(1);

        MIU_PortectInfo protectInfo;

        int miuID = 0;

        for ( miuID = 0; miuID < 2; miuID++ )
        {
            MDrv_MIU_GetProtectInfo(miuID, &protectInfo);

            if (protectInfo.bHit)
            {
                printf("\33[0;33;44m[DFB] MIU%d Hit!\33[0m block = %d, group = %d, clientID = %d \n", 
                        miuID,
                        protectInfo.u8Block,
                        protectInfo.u8Group,
                        protectInfo.u8ClientID );
            }
        }
    }
}


static inline GFX_Buffer_Format
adjustcolorformat( MSTARDriverData * sdrv )
{
    if(!sdrv)
        return -1;

    if((sdrv->src_ge_format ==GFX_FMT_I8     ||
        sdrv->src_ge_format == GFX_FMT_I4    ||
        sdrv->src_ge_format == GFX_FMT_I2    ||
        sdrv->src_ge_format == GFX_FMT_I1)   &&
        sdrv->dst_ge_format != GFX_FMT_I8)
        return GFX_FMT_ARGB8888;
    else
        return (GFX_Buffer_Format)sdrv->src_ge_format;

}


unsigned long
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


unsigned long
mstar_pixel_to_color( DFBSurfacePixelFormat     format,
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
unsigned long
mstar_pixel_to_colorkey( DFBSurfacePixelFormat      src_format,
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
                       GFX_DFBBldOP                *pgfx_src_bld_op,
                       GFX_DFBBldOP                *pgfx_dst_bld_op,
                       u16                         *pu16_bld_flags )
{
    if(DSBLIT_NOFX == blittingflags)
        return false;

    //Set Blend Flag:
    *pu16_bld_flags = 0x0;
    if(blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_ALPHACHANNEL;

    if(blittingflags & DSBLIT_BLEND_COLORALPHA)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_COLORALPHA;

    if(blittingflags & DSBLIT_COLORIZE)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_COLORIZE;

    if(blittingflags & DSBLIT_SRC_PREMULTIPLY)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_SRCPREMUL;
    if(blittingflags & DSBLIT_DST_PREMULTIPLY)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_DSTPREMUL;

    if(blittingflags & DSBLIT_DEMULTIPLY)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_DEMULTIPLY;

    if(blittingflags & DSBLIT_SRC_PREMULTCOLOR)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_SRCPREMULCOL;

    if(blittingflags & DSBLIT_XOR)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_XOR;

    if(blittingflags & DSBLIT_SRC_MASK_ALPHA)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_SRCALPHAMASK;

    if(blittingflags & DSBLIT_SRC_MASK_COLOR)
        *pu16_bld_flags |= GFX_DFB_BLD_FLAG_SRCCOLORMASK;

    //No blending!
    if(0x0 == *pu16_bld_flags)
        return false;

    if(DSBF_UNKNOWN == src_blend)
        *pgfx_src_bld_op = GFX_DFB_BLD_OP_ONE;
    else
        *pgfx_src_bld_op = (GFX_DFBBldOP)(src_blend-1);

    if(DSBF_UNKNOWN == dst_blend)
        *pgfx_dst_bld_op = GFX_DFB_BLD_OP_ONE;
    else
        *pgfx_dst_bld_op = (GFX_DFBBldOP)(dst_blend-1);

    if(!(blittingflags & (DSBLIT_BLEND_ALPHACHANNEL|DSBLIT_BLEND_COLORALPHA)))
    {
        //patch HW, current GE only do blend operations with alpha blend on
        *pgfx_src_bld_op = GFX_DFB_BLD_OP_ONE;
        *pgfx_dst_bld_op = GFX_DFB_BLD_OP_ZERO;
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
    //Set ColorKey:
    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY ))
    {
        /* 
            default   case  : src_color_key_index_patch = 1
            original  case  : src_color_key_index_patch = 0
        */
        if (dfb_config->src_color_key_index_patch == 1)   
            MApi_GFX_SetSrcColorKey( sdrv->ge_src_colorkey_enabled,
                                     CK_OP_EQUAL,
                                     (GFX_Buffer_Format)sdrv->dst_ge_format,
                                     &sdrv->src_ge_clr_key,
                                     &sdrv->src_ge_clr_key );
        else 
            MApi_GFX_SetSrcColorKey( sdrv->ge_src_colorkey_enabled,
                                     CK_OP_EQUAL,
                                     adjustcolorformat(sdrv),
                                     &sdrv->src_ge_clr_key,
                                     &sdrv->src_ge_clr_key );
    }
    
    if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY ))
    {
        MApi_GFX_SetDstColorKey( sdrv->ge_blit_dst_colorkey_enabled,
                                 CK_OP_NOT_EQUAL,
                                 (GFX_Buffer_Format)sdrv->dst_ge_format,
                                 &sdrv->dst_ge_clr_key,
                                 &sdrv->dst_ge_clr_key);
    }


    /* Patch to support RGB32 in GE by using ARGB8888 with constant alpha 0xff instead
       When APP use RGB32 format, it use sw engine instead of GE ( GE don't support)
       Implement following step to support RGB32 in GE :
       1. Regard RGB32 as ARGB8888 in GE.
       2. Disable alpha blending, and use const alpha (0xff) to be the pixel alpha of destination while CSC.
    */
    if(sdrv->ge_src_rgb32_blit_enabled || sdrv->ge_dest_rgb32_blit_enabled)
    {
        if((modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR)) && sdrv->ge_blit_alpha_blend_enabled)
        {
            GFX_RgbColor rgbColor;

            MApi_GFX_SetDFBBldFlags((MS_U16)sdrv->ge_blit_bld_flags);
            MApi_GFX_SetDFBBldOP(sdrv->ge_src_blend, sdrv->ge_dst_blend);

            rgbColor.a = sdrv->color.a;
            rgbColor.r = sdrv->color.r;
            rgbColor.g = sdrv->color.g;
            rgbColor.b = sdrv->color.b;

            MApi_GFX_SetDFBBldConstColor(rgbColor);
            MApi_GFX_EnableDFBBlending(TRUE);
            MApi_GFX_EnableAlphaBlending(TRUE);
            printf("[DFB] BlitState RGB32 blending patch enable: sdrv->ge_src_blend:%d, sdrv->ge_dst_blend:%d\n", sdrv->ge_src_blend, sdrv->ge_dst_blend);
        }
        else
        {
            MApi_GFX_EnableDFBBlending(FALSE);
            MApi_GFX_EnableAlphaBlending(FALSE);

            MApi_GFX_SetAlpha(TRUE, COEF_ONE, ABL_FROM_CONST, 0xff);

            D_INFO("[DFB] BlitState RGB32 patch enable\n");
        }
    }
    else if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR))//Set Alpha Blend
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
                        MApi_GFX_EnableDFBBlending(FALSE);
                        MApi_GFX_SetAlpha(true, COEF_ONE, ABL_FROM_ROP8_SRC, sdrv->color.a);

                        break;

                default :
                {
                        GFX_RgbColor rgbColor;
                        MApi_GFX_SetDFBBldFlags((MS_U16)sdrv->ge_blit_bld_flags);
                        MApi_GFX_SetDFBBldOP(sdrv->ge_src_blend, sdrv->ge_dst_blend);

                        rgbColor.a = sdrv->color.a;
                        rgbColor.r = sdrv->color.r;
                        rgbColor.g = sdrv->color.g;
                        rgbColor.b = sdrv->color.b;

                        MApi_GFX_SetDFBBldConstColor(rgbColor);
                        MApi_GFX_EnableDFBBlending(TRUE);
                        MApi_GFX_EnableAlphaBlending(TRUE);

                        break;
                }
            }
        }
        else
        {
            MApi_GFX_EnableDFBBlending(FALSE);
            MApi_GFX_EnableAlphaBlending(FALSE);
            MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }
    }

    ////////////////////////////////////////////////////////////
    //Set Src Buffer, Blt ops Needed:
    if(modified & SMF_SOURCE)
    {
        GFX_BufferInfo buf;

        buf.u32Addr = sdrv->src_phys;
        buf.u32ColorFmt = sdrv->src_ge_format;
        buf.u32Width = sdrv->src_w;
        buf.u32Height = sdrv->src_h;
        buf.u32Pitch = sdrv->src_pitch;

        MApi_GFX_SetSrcBufferInfo(&buf, 0);
    }

    //Set BLIT rotation
    if(modified & SMF_BLITTING_FLAGS)
    {
        if(sdrv->bflags & DSBLIT_ROTATE90)
            MApi_GFX_SetRotate(GEROTATE_270);

        else if(sdrv->bflags & DSBLIT_ROTATE180)
            MApi_GFX_SetRotate(GEROTATE_180);

        else if(sdrv->bflags & DSBLIT_ROTATE270)
            MApi_GFX_SetRotate(GEROTATE_90);
        else
            MApi_GFX_SetRotate(GEROTATE_0);
    }

    //Set Blit Nearest Mode:
    if(modified & SMF_SRC_CONVOLUTION)
        MApi_GFX_SetNearestMode((MS_BOOL)sdrv->ge_blit_nearestmode_enabled);

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

     
    //GOP mirror do GE mirror for mantis id:0523773
    if (dfb_config->do_GE_Vmirror == true)
    {
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {
            MApi_GFX_SetMirror((MS_BOOL) sdrv->ge_blit_xmirror,
                               (MS_BOOL)!sdrv->ge_blit_ymirror);
        }

        MApi_GFX_SetDstMirror((MS_BOOL)false, (MS_BOOL)TRUE);
    }
    else
    {
        //Set Blit Mirror:
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
        {
            MApi_GFX_SetMirror((MS_BOOL)sdrv->ge_blit_xmirror,
                               (MS_BOOL)sdrv->ge_blit_ymirror);
        }

        MApi_GFX_SetDstMirror((MS_BOOL)false, (MS_BOOL)false);
    }

    //set ROP2 option Code:
    if(modified & SMF_WRITE_MASK_BITS)
        MApi_GFX_SetROP2(sdrv->ge_rop_enable, sdrv->ge_rop_op);

}

static void
mstarSetEngineDrawState( StateModificationFlags      modified,
                         MSTARDriverData            *sdrv,
                         MSTARDeviceData            *sdev)
{
    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_COLORKEY))
        MApi_GFX_SetSrcColorKey( FALSE,
                                 CK_OP_EQUAL,
                                 adjustcolorformat(sdrv),
                                 &sdrv->src_ge_clr_key,
                                 &sdrv->src_ge_clr_key);

    if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY ))
        MApi_GFX_SetDstColorKey( sdrv->ge_draw_dst_colorkey_enabled,
                                 CK_OP_NOT_EQUAL,
                                 (GFX_Buffer_Format)sdrv->dst_ge_format,
                                 &sdrv->dst_ge_clr_key,
                                 &sdrv->dst_ge_clr_key);

    /* Patch to support RGB32 in GE by using ARGB8888 with constant alpha 0xff instead
       When APP use RGB32 format, it use sw engine instead of GE ( GE don't support)
       Implement following step to support RGB32 in GE :
       1. Regard RGB32 as ARGB8888 in GE.
       2. Disable alpha blending, and use const alpha (0xff) to be the pixel alpha of destination while CSC.
    */
    if(sdrv->ge_dest_rgb32_blit_enabled)
    {
        if((modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR)) && sdrv->ge_blit_alpha_blend_enabled)
        {
            GFX_RgbColor rgbColor;

            MApi_GFX_SetDFBBldFlags((MS_U16)sdrv->ge_draw_bld_flags);
            MApi_GFX_SetDFBBldOP(sdrv->ge_src_blend, sdrv->ge_dst_blend);

            rgbColor.a = sdrv->color.a;
            rgbColor.r = sdrv->color.r;
            rgbColor.g = sdrv->color.g;
            rgbColor.b = sdrv->color.b;

            MApi_GFX_SetDFBBldConstColor(rgbColor);
            MApi_GFX_EnableDFBBlending(TRUE);
            MApi_GFX_EnableAlphaBlending(TRUE);
            printf("[DFB] DrawState RGB32 blending patch enable: sdrv->ge_src_blend:%d,sdrv->ge_dst_blend:%d\n", sdrv->ge_src_blend, sdrv->ge_dst_blend);
        }
        else
        {
            MApi_GFX_EnableDFBBlending(FALSE);
            MApi_GFX_EnableAlphaBlending(FALSE);

            MApi_GFX_SetAlpha(TRUE, COEF_ONE, ABL_FROM_CONST, 0xff);

            D_INFO("[DFB] DrawState RGB32 patch enable\n");
        }
    }
    else if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND | SMF_COLOR))
    {
        if(sdrv->ge_draw_alpha_blend_enabled)
        {
            GFX_RgbColor rgbColor;

            MApi_GFX_SetDFBBldFlags((MS_U16)sdrv->ge_draw_bld_flags);
            MApi_GFX_SetDFBBldOP(sdrv->ge_src_blend, sdrv->ge_dst_blend);

            rgbColor.a = sdrv->color.a;
            rgbColor.r = sdrv->color.r;
            rgbColor.g = sdrv->color.g;
            rgbColor.b = sdrv->color.b;

            MApi_GFX_SetDFBBldConstColor(rgbColor);
            MApi_GFX_EnableDFBBlending(TRUE);
            MApi_GFX_EnableAlphaBlending(TRUE);

            //if alpha_compare_mode can work when DFB HW blend ,this patch should be removed
            if(sdrv->ge_render_alpha_cmp_enabled)
            {
                MApi_GFX_EnableDFBBlending(FALSE);
                //(GFX_BlendCoef)0x2: Csrc * Asrc + Cdst * (1 - Asrc)
                MApi_GFX_SetAlphaBlending((GFX_BlendCoef)0x2, sdrv->color.a);
                MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
            }
        }
        else
        {
            MApi_GFX_EnableDFBBlending(FALSE);
            MApi_GFX_EnableAlphaBlending(FALSE);
            MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }
    }
}

static void
mstarSetEngineShareState( StateModificationFlags     modified,
                          DFBAccelerationMask        support_accel,
                          MSTARDriverData           *sdrv,
                          MSTARDeviceData           *sdev )
{
    GFX_Point v0, v1;
    GFX_BufferInfo buf;

    ////////////////////////////////////////////////////////////
    //Set Dst Buffer State, All ops Needed:
    if(modified & SMF_DESTINATION)
    {
        buf.u32Addr         = sdrv->dst_phys;
        buf.u32ColorFmt     = sdrv->dst_ge_format;
        buf.u32Width        = sdrv->dst_w;
        buf.u32Height       = sdrv->dst_h;
        buf.u32Pitch        = sdrv->dst_pitch;

        MApi_GFX_SetDstBufferInfo(&buf, 0);

        if(!sdev->b_hwclip)
        {
            v0.x = 0;
            v0.y = 0;
            v1.x = buf.u32Width - 1;
            v1.y = buf.u32Height - 1;

            MApi_GFX_SetClip(&v0, &v1);
        }
    }

    if((modified & SMF_CLIP) && sdev->b_hwclip)
    {
            v0.x = sdrv->clip.x1;
            v0.y = sdrv->clip.y1;
            v1.x = sdrv->clip.x2;
            v1.y = sdrv->clip.y2;

            MApi_GFX_SetClip(&v0, &v1);
    }

    //Set Alpha Compare:
    if(modified & SMF_SOURCE2)
        MApi_GFX_SetAlphaCmp((MS_BOOL)sdrv->ge_render_alpha_cmp_enabled,
                             sdrv->ge_render_alpha_cmp );

    //SetYUV Format:
    if(modified & (SMF_DESTINATION | SMF_SOURCE))
        MApi_GFX_SetDC_CSC_FMT( (GFX_YUV_Rgb2Yuv)dfb_config->mst_rgb2yuv_mode,
                                GFX_YUV_OUT_255, GFX_YUV_IN_255,
                                sdrv->src_ge_yuv_fmt,
                                sdrv->dst_ge_yuv_fmt);

    //Set Blit Palette:
    if(sdrv->ge_palette_enabled && sdev->num_entries)
        MApi_GFX_SetPaletteOpt( sdev->palette_tbl,
                                0,
                                (sdev->num_entries - 1));

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
}

#ifdef DFB_ION
#ifdef DFB_ION_TLB
static void
mstarSetGeTlb(CardState           *state)
{
    EN_GFX_TLB_Mode TLBmode;
    MS_PHYADDR TLBSrcMiuAddr,TLBDstMiuAddr;
    static CoreSurfaceBufferPhyType dst_phys_type = 0;
    static CoreSurfaceBufferPhyType src_phys_type = 0;

    if( (state->src.phys_type == src_phys_type)     &&
        (state->dst.phys_type == dst_phys_type) )
        return;

    if(state->src.phys_type == CSBPT_TLB)
    {
        if(state->dst.phys_type == CSBPT_TLB)
        {
            TLBmode         = E_GFX_TLB_SRC_DST;
            TLBSrcMiuAddr   = state->src.tlb_table_addr;
            TLBDstMiuAddr   = state->dst.tlb_table_addr;
        }
        else
        {
            TLBmode         = E_GFX_TLB_SRC;
            TLBSrcMiuAddr   = state->src.tlb_table_addr;
            TLBDstMiuAddr   = 0;
        }
    }
    else
    {
        if(state->dst.phys_type == CSBPT_TLB)
        {
            TLBmode         = E_GFX_TLB_DST;
            TLBSrcMiuAddr   = 0;
            TLBDstMiuAddr   = state->dst.tlb_table_addr;
        }
        else
        {
            TLBmode         = E_GFX_TLB_NONE;
            TLBSrcMiuAddr   = 0;
            TLBDstMiuAddr   = 0;
        }
    }
        
    MApi_GFX_SetTLBMode(TLBmode);
    MApi_GFX_SetTLBBaseADDR(TLBSrcMiuAddr, TLBDstMiuAddr);

    src_phys_type = state->src.phys_type;
    dst_phys_type = state->dst.phys_type;
}
#endif
#endif

bool
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


void
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
                    sdrv->dst_ge_format     = GFX_FMT_ARGB1555_DST;
                    break;

            case DSPF_ARGB:
                    sdrv->dst_ge_format     = GFX_FMT_ARGB8888;
                    break;

            case DSPF_ABGR:
                    sdrv->dst_ge_format     = GFX_FMT_ABGR8888;
                    break;

            case DSPF_YVYU:
                    sdrv->dst_ge_yuv_fmt    = GFX_YUV_YVYU;
                    sdrv->dst_ge_format     = GFX_FMT_YUV422;
                    break;

            case DSPF_YUY2:
                    sdrv->dst_ge_yuv_fmt    = GFX_YUV_YUYV;
                    sdrv->dst_ge_format     = GFX_FMT_YUV422;
                    break;

            case DSPF_UYVY:
                    sdrv->dst_ge_yuv_fmt    = GFX_YUV_UYVY;
                    sdrv->dst_ge_format     = GFX_FMT_YUV422;
                    break;

            case DSPF_LUT8:
                    sdrv->dst_ge_format     = GFX_FMT_I8;
                    palette                 = dsurface->palette;
                    break;

            case DSPF_ARGB4444:
                    sdrv->dst_ge_format     = GFX_FMT_ARGB4444;
                    break;

            case DSPF_RGB16:
                    sdrv->dst_ge_format     = GFX_FMT_RGB565;
                    break;

            case DSPF_A8:
                    sdrv->dst_ge_format     = GFX_FMT_I8;
                    break;

            case DSPF_RGB32:

                    sdrv->dst_ge_format     = GFX_FMT_ARGB8888;
                    sdrv->ge_dest_rgb32_blit_enabled = 1;
                    break;

            case DSPF_BLINK12355:
                    sdrv->dst_ge_format     = GFX_FMT_1ABFGBG12355;
                    break;

            case DSPF_BLINK2266:
                    sdrv->dst_ge_format     = GFX_FMT_FABAFGBG2266;
                    break;

            default:
                printf( "[DFB] error format : %s (%s, %s)\n",
                    dfb_pixelformat_name(state->dst.buffer->format), __FILE__, __FUNCTION__ );

                return;
        }

        if((dfb_config->bUsingHWTLB)
#ifdef DFB_ION
        || (state->dst.phys_type == CSBPT_TLB)
#endif
        )
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

        if(GFX_FMT_I8 == sdrv->dst_ge_format)
            sdrv->color.b = state->color_index;
    }

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

            if((dfb_config->bUsingHWTLB)
#ifdef DFB_ION
            || (state->dst.phys_type == CSBPT_TLB)
#endif
            )
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
                        sdrv->src_ge_format = GFX_FMT_ARGB1555;
                        break;

                case DSPF_ARGB:
                        sdrv->src_ge_format = GFX_FMT_ARGB8888;
                        break;

                case DSPF_ABGR:
                        sdrv->src_ge_format = GFX_FMT_ABGR8888;
                        break;

                case DSPF_YVYU:
                        sdrv->src_ge_yuv_fmt = GFX_YUV_YVYU;
                        sdrv->src_ge_format  = GFX_FMT_YUV422;
                        break;

                case DSPF_YUY2:
                        sdrv->src_ge_yuv_fmt = GFX_YUV_YUYV;
                        sdrv->src_ge_format  = GFX_FMT_YUV422;
                        break;

                case DSPF_UYVY:
                        sdrv->src_ge_yuv_fmt = GFX_YUV_UYVY;
                        sdrv->src_ge_format  = GFX_FMT_YUV422;
                        break;

                case DSPF_LUT8:
                        sdrv->src_ge_format = GFX_FMT_I8;

                        if( NULL == palette )
                            palette = ssurface->palette;

                        break;

                case DSPF_LUT4:
                        palette_intensity_format    = DSPF_LUT4;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I4_CNT;
                        sdrv->src_ge_format         = GFX_FMT_I4;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT2:
                        palette_intensity_format    = DSPF_LUT2;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I2_CNT;
                        sdrv->src_ge_format         = GFX_FMT_I2;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_LUT1:
                        palette_intensity_format    = DSPF_LUT1;
                        max_palette_intensity_cnt   = MAX_PALETTE_INTENSITY_I1_CNT;
                        sdrv->src_ge_format         = GFX_FMT_I1;
                        palette_intensity           = ssurface->palette;
                        break;

                case DSPF_ARGB4444:
                        sdrv->src_ge_format = GFX_FMT_ARGB4444;
                        break;

                case DSPF_RGB16:
                        sdrv->src_ge_format = GFX_FMT_RGB565;
                        break;

                case DSPF_A8:
                        sdrv->src_ge_format = GFX_FMT_I8;
                        break;

                case DSPF_RGB32:
                        sdrv->ge_src_rgb32_blit_enabled = 1;
                        sdrv->src_ge_format = GFX_FMT_ARGB8888;
                        break;

            case DSPF_BLINK12355:
                        sdrv->src_ge_format = GFX_FMT_1ABFGBG12355;
                        break;

            case DSPF_BLINK2266:
                        sdrv->src_ge_format = GFX_FMT_FABAFGBG2266;
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

            memset(sdev->palette_tbl, 0xFF, (sizeof(GFX_PaletteEntry)*MAX_PALETTE_ENTRY_CNT));

            for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
            {
                sdev->palette_tbl[i].RGB.u8A = i;
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
                    sdrv->ge_rop_op = ROP2_OP_ZERO;
                    break;

                case DFB_ROP2_NOT_PS_OR_PD:
                    sdrv->ge_rop_op = ROP2_OP_NOT_PS_OR_PD;
                    break;

                case DFB_ROP2_NS_AND_PD:
                    sdrv->ge_rop_op = ROP2_OP_PS_AND_PD;
                    break;

                case DFB_ROP2_NS:
                    sdrv->ge_rop_op = ROP2_OP_NS;
                    break;

                case DFB_ROP2_PS_AND_ND:
                    sdrv->ge_rop_op = ROP2_OP_PS_AND_ND;
                    break;

                case DFB_ROP2_ND:
                    sdrv->ge_rop_op = ROP2_OP_ND;
                    break;

                case DFB_ROP2_PS_XOR_PD:
                    sdrv->ge_rop_op = ROP2_OP_PS_XOR_PD;
                    break;

                case DFB_ROP2_NOT_PS_AND_PD:
                    sdrv->ge_rop_op = ROP2_OP_NOT_PS_AND_PD;
                    break;

                case DFB_ROP2_NOT_PS_XOR_PD:
                    sdrv->ge_rop_op = ROP2_OP_NOT_PS_XOR_PD;
                    break;

                case DFB_ROP2_PD:
                    sdrv->ge_rop_op = ROP2_OP_PD;
                    break;

                case DFB_ROP2_NS_OR_PD:
                    sdrv->ge_rop_op = ROP2_OP_NS_OR_PD;
                    break;

                case DFB_ROP2_PS:
                    sdrv->ge_rop_op = ROP2_OP_PS;
                    break;

                case DFB_ROP2_PS_OR_ND:
                    sdrv->ge_rop_op = ROP2_OP_PS_OR_ND;
                    break;

                case DFB_ROP2_PD_OR_PS:
                    sdrv->ge_rop_op = ROP2_OP_PD_OR_PS;
                    break;

                case DFB_ROP2_ONE:
                    sdrv->ge_rop_op = ROP2_OP_ONE;
                    break;

                case DFB_ROP2_NONE:
                    //disable ROP option
                    //gfx enum no DFB_ROP2_NONE option
                    sdrv->ge_rop_op = ROP2_OP_PD;
                    sdrv->ge_rop_enable = FALSE;
                    break;

                default:
                    sdrv->ge_rop_op = ROP2_OP_PD;
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
            sdev->palette_tbl[i].RGB.u8A = palette->entries[i].a;
            sdev->palette_tbl[i].RGB.u8R = palette->entries[i].r;
            sdev->palette_tbl[i].RGB.u8G = palette->entries[i].g;
            sdev->palette_tbl[i].RGB.u8B = palette->entries[i].b;
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

            color_original.a = sdev->palette_tbl[clr_key_index].RGB.u8A;
            color_original.r = sdev->palette_tbl[clr_key_index].RGB.u8R;
            color_original.g = sdev->palette_tbl[clr_key_index].RGB.u8G;
            color_original.b = sdev->palette_tbl[clr_key_index].RGB.u8B;       

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
            sdev->palette_tbl[clr_key_index].RGB.u8A = color_dummy.a;
            sdev->palette_tbl[clr_key_index].RGB.u8R = color_dummy.r;
            sdev->palette_tbl[clr_key_index].RGB.u8G = color_dummy.g;
            sdev->palette_tbl[clr_key_index].RGB.u8B = color_dummy.b;

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
        num_entries = (palette_intensity->num_entries > max_palette_intensity_cnt ? max_palette_intensity_cnt : palette_intensity->num_entries);

        for( i = 0; i < num_entries; ++i )
        {
            sdev->palette_intensity[i] = mstar_pixel_to_color( palette_intensity_format,
                                                               i,
                                                               palette_intensity);
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

    MApi_GFX_BeginDraw();

#ifdef DFB_ION
#ifdef DFB_ION_TLB
    mstarSetGeTlb(state);
#endif
#endif

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
        MApi_GFX_SetDither(true);
    }
    else
    {
        MApi_GFX_SetDither(false);
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
            (sdrv->src_ge_format != GFX_FMT_YUV422)     &&
            (sdrv->dst_ge_format == GFX_FMT_YUV422))
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

    MApi_GFX_EndDraw();

}


void
mstarSetState_null( void                        *drv,
                    void                        *dev,
                    GraphicsDeviceFuncs         *funcs,
                    CardState                   *state,
                    DFBAccelerationMask          accel )
{

}


bool
mstarFillRectangle_null( void                   *drv,
                         void                   *dev,
                         DFBRectangle           *rect )
{
    return true;
}


bool
mstarFillRectangle( void                        *drv,
                    void                        *dev,
                    DFBRectangle                *rect )
{
    MSTARDeviceData *sdev = dev;
    MSTARDriverData *sdrv = drv;
    GFX_RectFillInfo rectInfo = {0};

    u32 y = 0, cb = 0, cr = 0;

    rectInfo.dstBlock.x         = rect->x;
    rectInfo.dstBlock.y         = rect->y;
    rectInfo.dstBlock.width     = rect->w;
    rectInfo.dstBlock.height    = rect->h;
    rectInfo.fmt                = sdrv->dst_ge_format;

    switch(rectInfo.fmt)
    {
        case GFX_FMT_YUV422:
                RGB_TO_YCBCR( sdrv->color.r,
                              sdrv->color.g,
                              sdrv->color.b,
                              y,
                              cb,
                              cr );

                rectInfo.colorRange.color_s.a = 0xFF;
                rectInfo.colorRange.color_s.r = cr;
                rectInfo.colorRange.color_s.g = y;
                rectInfo.colorRange.color_s.b = cb;
                break;

        default:
                rectInfo.colorRange.color_s.a = sdrv->color.a;
                rectInfo.colorRange.color_s.r = sdrv->color.r;
                rectInfo.colorRange.color_s.g = sdrv->color.g;
                rectInfo.colorRange.color_s.b = sdrv->color.b;
                break;
    }

    if(sdrv->dflags & (DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(rectInfo.fmt)
        {
            case GFX_FMT_YUV422:
                    RGB_TO_YCBCR( sdrv->color2.r,
                                  sdrv->color2.g,
                                  sdrv->color2.b,
                                  y,
                                  cb,
                                  cr );

                    rectInfo.colorRange.color_e.a = 0xFF;
                    rectInfo.colorRange.color_e.r = cr;
                    rectInfo.colorRange.color_e.g = y;
                    rectInfo.colorRange.color_e.b = cb;
                    break;

            default:
                    rectInfo.colorRange.color_e.a = sdrv->color2.a;
                    rectInfo.colorRange.color_e.r = sdrv->color2.r;
                    rectInfo.colorRange.color_e.g = sdrv->color2.g;
                    rectInfo.colorRange.color_e.b = sdrv->color2.b;
                    break;
        }

        if(sdrv->dflags & DSDRAW_COLOR_GRADIENT_X)
            rectInfo.flag = GFXRECT_FLAG_COLOR_GRADIENT_X;
        else
            rectInfo.flag = 0;

        if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            rectInfo.flag |= GFXRECT_FLAG_COLOR_GRADIENT_Y;
    }
    else
        rectInfo.flag = 0;

    MApi_GFX_BeginDraw();
    MApi_GFX_RectFill(&rectInfo);
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true;
}


bool
mstarFillRectangle_sw( void                     *drv,
                       void                     *dev,
                       DFBRectangle             *rect )
{


    if (dfb_config->sw_render & SWRF_FILLRECT)
        return false;
    else
        return mstarFillRectangle(drv, dev, rect);
}


#if defined(USE_GFX_EXTENSION)
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


bool
mstarDrawRectangle_null( void               *driver_data,
                         void               *device_data,
                         DFBRectangle       *rect )
{
    return true;
}


bool
mstarDrawRectangle( void                    *driver_data,
                    void                    *device_data,
                    DFBRectangle            *rect )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    GFX_DrawLineInfo l;

    l.fmt = sdrv->dst_ge_format;
    l.colorRange.color_s.a  = sdrv->color.a;
    l.colorRange.color_s.r  = sdrv->color.r;
    l.colorRange.color_s.g  = sdrv->color.g;
    l.colorRange.color_s.b  = sdrv->color.b;
    l.colorRange.color_e    = l.colorRange.color_s;
    l.width                 = LINE_WIDTH;
    l.flag                  = GFXLINE_FLAG_COLOR_CONSTANT;

    l.x1 = rect->x;
    l.y1 = rect->y;
    l.x2 = rect->x+rect->w - 1;
    l.y2 = rect->y;

    MApi_GFX_BeginDraw();
    MApi_GFX_DrawLine(&l);

    l.x2 = rect->x;
    l.y2 = rect->y + rect->h - 1;
    MApi_GFX_DrawLine(&l);

    l.y1 = rect->y + rect->h - 1;
    l.x2 = rect->x + rect->w - 1;
    MApi_GFX_DrawLine(&l);

    l.x1 = rect->x + rect->w - 1;
    l.y1 = rect->y;
    
    l.x2 = rect->x + rect->w - 1;
    l.y2 = rect->y + rect->h - 1 + l.width + 1;

    MApi_GFX_DrawLine(&l);
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));

    MApi_GFX_EndDraw();

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
    GFX_OvalFillInfo ovalInfo;

    ovalInfo.dstBlock.x         = oval->oval_rect.x;
    ovalInfo.dstBlock.y         = oval->oval_rect.y;
    ovalInfo.dstBlock.width     = oval->oval_rect.w;
    ovalInfo.dstBlock.height    = oval->oval_rect.h;

    ovalInfo.fmt                = sdrv->dst_ge_format;
    ovalInfo.u32LineWidth       = (u32)oval->line_width;

    MApi_GFX_BeginDraw();

    if(oval->line_width > 0)
    {
        if(!DFB_COLOR_EQUAL(sdrv->color, oval->line_color))
        {
            ovalInfo.u32LineWidth = 0;
            ovalInfo.color.a = oval->line_color.a;
            ovalInfo.color.r = oval->line_color.r;
            ovalInfo.color.g = oval->line_color.g;
            ovalInfo.color.b = oval->line_color.b;

            MApi_GFX_DrawOval(&ovalInfo);
            ovalInfo.u32LineWidth = (u32)oval->line_width;
        }
        else
        {
            ovalInfo.u32LineWidth = 0;
        }
    }

    ovalInfo.color.a = sdrv->color.a;
    ovalInfo.color.r = sdrv->color.r;
    ovalInfo.color.g = sdrv->color.g;
    ovalInfo.color.b = sdrv->color.b;
    MApi_GFX_DrawOval( &ovalInfo );

    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

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

    GFX_DrawLineInfo l;
    u32 y=0, cb=0, cr=0;

    l.x1 = line->x1;
    l.y1 = line->y1;
    l.x2 = line->x2;
    l.y2 = line->y2;

    l.fmt = sdrv->dst_ge_format;

    l.colorRange.color_s.a  = sdrv->color.a;
    l.colorRange.color_s.r  = sdrv->color.r;
    l.colorRange.color_s.g  = sdrv->color.g;
    l.colorRange.color_s.b  = sdrv->color.b;
    l.colorRange.color_e    = l.colorRange.color_s;

    l.width = width;
    l.flag = GFXLINE_FLAG_COLOR_CONSTANT;

    switch(l.fmt)
    {
        case GFX_FMT_YUV422:
            RGB_TO_YCBCR(sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr);
            l.colorRange.color_s.a = 0xFF;
            l.colorRange.color_s.r = cr;
            l.colorRange.color_s.g = y;
            l.colorRange.color_s.b = cb;
            break;

        default:
            l.colorRange.color_s.a = sdrv->color.a;
            l.colorRange.color_s.r = sdrv->color.r;
            l.colorRange.color_s.g = sdrv->color.g;
            l.colorRange.color_s.b = sdrv->color.b;
            break;
    }

    l.flag = GFXLINE_FLAG_COLOR_CONSTANT;

    if(sdrv->dflags &(DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
    {
        switch(l.fmt)
        {
            case GFX_FMT_YUV422:
                    RGB_TO_YCBCR( sdrv->color2.r,
                                  sdrv->color2.g,
                                  sdrv->color2.b,
                                  y,
                                  cb,
                                  cr );

                    l.colorRange.color_e.a = 0xFF;
                    l.colorRange.color_e.r = cr;
                    l.colorRange.color_e.g = y;
                    l.colorRange.color_e.b = cb;
                    break;

            default:
                    l.colorRange.color_e.a = sdrv->color2.a;
                    l.colorRange.color_e.r = sdrv->color2.r;
                    l.colorRange.color_e.g = sdrv->color2.g;
                    l.colorRange.color_e.b = sdrv->color2.b;
                    break;
        }

    }

    MApi_GFX_BeginDraw();
    MApi_GFX_DrawLine(&l);
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

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

    GFX_DrawRect rectInfo;

    rectInfo.srcblk.x = rect->x;
    rectInfo.srcblk.y = rect->y;
    rectInfo.dstblk.x = dx;
    rectInfo.dstblk.y = dy;

    rectInfo.srcblk.width   = rectInfo.dstblk.width     = rect->w;
    rectInfo.srcblk.height  = rectInfo.dstblk.height    = rect->h;

    MApi_GFX_BeginDraw();
      
    SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
    SET_BLT_MIRROR_SRCY(sdrv, rectInfo);

    MApi_GFX_BitBlt(&rectInfo, GFXDRAW_FLAG_DEFAULT);


    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

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
                 DFBRectangle               *rect,
                 int                         dx,
                 int                         dy )
{
    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;

    bool ret = false;

    DFBSurfacePixelFormat src_format = state->source->config.format;
    DFBSurfacePixelFormat dst_format = state->destination->config.format;

    int src_w = state->source->config.size.w;
    int src_h = state->source->config.size.h;

    if ( GLESInitFuncs  == NULL         ||
         src_format     != DSPF_ARGB    ||
         dst_format     != DSPF_ARGB )
    {
        printf("[DFB][%s] GL Init Failed!!\n", __FUNCTION__);
        ret = false;
    }
    else
    {
        GLESBlitInfo blitInfo;
        SDR2HDRParameter *param = (SDR2HDRParameter *)sdev->sdr2hdr_param;

        blitInfo.dst_x = dx;
        blitInfo.dst_y = dy;

        blitInfo.rect.x = rect->x;
        blitInfo.rect.y = rect->y;
        blitInfo.rect.w = rect->w;
        blitInfo.rect.h = rect->h;

        blitInfo.eglImageInfo.src.width  = src_w;
        blitInfo.eglImageInfo.src.height = src_h;
        blitInfo.eglImageInfo.src.pitch  = state->src.pitch;
        blitInfo.eglImageInfo.src.phys   = state->src.phys;

        blitInfo.eglImageInfo.dst.width  = state->destination->config.size.w;
        blitInfo.eglImageInfo.dst.height = state->destination->config.size.h;
        blitInfo.eglImageInfo.dst.pitch  = state->dst.pitch;
        blitInfo.eglImageInfo.dst.phys   = state->dst.phys;

        // sdr to hdr coeff.
        SDR2HDR_MUTEX_LOCK_SLAVE(&sdrv->core->shared->lock_sw_sdr2hdr);
        memcpy(&blitInfo.coeff, &param->coeff, sizeof(SDR2HDRCoeff));
        SDR2HDR_MUTEX_UNLOCK(&sdrv->core->shared->lock_sw_sdr2hdr);

        blitInfo.coeff.en_sdr2hdr = (dfb_config->mst_gles2_sdr2hdr | blitInfo.coeff.en_sdr2hdr);
        // gles render.
        ret = glesFuncs.GLES2Blit( &blitInfo);
    }

    return ret;
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
    GFX_DrawRect rectInfo;
    GFX_Result ret;

    rectInfo.srcblk.x       = srect->x;
    rectInfo.srcblk.y       = srect->y;
    rectInfo.srcblk.width   = srect->w;
    rectInfo.srcblk.height  = srect->h;

    rectInfo.dstblk.x       = drect->x;
    rectInfo.dstblk.y       = drect->y;
    rectInfo.dstblk.width   = drect->w;
    rectInfo.dstblk.height  = drect->h;


    if(sdrv->src_ge_format == GFX_FMT_YUV422)
    {
        rectInfo.srcblk.width = rectInfo.srcblk.width & ~1;
    }

    if(sdrv->dst_ge_format == GFX_FMT_YUV422)
    {
        rectInfo.dstblk.width = (rectInfo.dstblk.width + 1) & ~1;
    }

    MApi_GFX_BeginDraw();
    SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
    SET_BLT_MIRROR_SRCY(sdrv, rectInfo);

    ret = MApi_GFX_BitBlt(&rectInfo, GFXDRAW_FLAG_SCALE);

    if(GFX_SUCCESS != ret)
    {
        D_INFO("[DFB] GE downscaling blit ratio can not be supported by HW(32:1)!, src height %d, dst height%d , src width :%d dst width %d\n",
                    rectInfo.srcblk.height,
                    rectInfo.dstblk.height,
                    rectInfo.srcblk.width,
                    rectInfo.dstblk.width );
    }
    else
        MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));


    MApi_GFX_EndDraw();

    return (GFX_SUCCESS == ret);
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

    GFX_DrawRect     rectInfo;
    GFX_Result       ret;
    GFX_ScaleInfo    ScaleInfo;


    rectInfo.srcblk.x       = srect->x;
    rectInfo.srcblk.y       = srect->y;
    rectInfo.srcblk.width   = srect->w;
    rectInfo.srcblk.height  = srect->h;

    rectInfo.dstblk.x       = drect->x;
    rectInfo.dstblk.y       = drect->y;
    rectInfo.dstblk.width   = drect->w;
    rectInfo.dstblk.height  = drect->h;

    ScaleInfo.u32DeltaX     = scale_info->u32DeltaX;
    ScaleInfo.u32DeltaY     = scale_info->u32DeltaY;
    ScaleInfo.u32InitDelatX = scale_info->u32InitDelatX;
    ScaleInfo.u32InitDelatY = scale_info->u32InitDelatY;

    if(sdrv->src_ge_format == GFX_FMT_YUV422)
    {
        rectInfo.srcblk.width = rectInfo.srcblk.width & ~1;
    }

    if(sdrv->dst_ge_format == GFX_FMT_YUV422)
    {
        rectInfo.dstblk.width = (rectInfo.dstblk.width + 1)& ~1;
    }

    MApi_GFX_BeginDraw();
    SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
    SET_BLT_MIRROR_SRCY(sdrv, rectInfo);

    ret = MApi_GFX_BitBltEx(&rectInfo, scale_info->u32GFXDrawFlag, &ScaleInfo);

    if(GFX_SUCCESS != ret)
    {
        D_INFO("[DFB] GE downscaling blit ratio can not be supported by HW(32:1)!, src height %d, dst height%d , src width :%d dst width %d\n",
                    rectInfo.srcblk.height,
                    rectInfo.dstblk.height,
                    rectInfo.srcblk.width,
                    rectInfo.dstblk.width );


    }
    else
        MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));

    MApi_GFX_EndDraw();

    return (GFX_SUCCESS == ret);
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

    GFX_DrawRect rectInfo;
    GFX_Result ret;

    rectInfo.srcblk.x       = srect->x;
    rectInfo.srcblk.y       = srect->y;
    rectInfo.srcblk.width   = srect->w;
    rectInfo.srcblk.height  = srect->h;

    rectInfo.dsttrapeblk.u16X0 = dtrapezoid->x0;
    rectInfo.dsttrapeblk.u16Y0 = dtrapezoid->y0;
    rectInfo.dsttrapeblk.u16X1 = dtrapezoid->x1;
    rectInfo.dsttrapeblk.u16Y1 = dtrapezoid->y1;

    rectInfo.dsttrapeblk.u16DeltaTop    = dtrapezoid->top_edge_width;
    rectInfo.dsttrapeblk.u16DeltaBottom = dtrapezoid->bottom_edge_width;

    MApi_GFX_BeginDraw();
    SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
    SET_BLT_MIRROR_SRCY(sdrv, rectInfo);

    if(false == dtrapezoid->horiz_direct)
        ret = MApi_GFX_BitBlt(&rectInfo, GFXRECT_FLAG_TRAPE_DIRECTION_Y|GFXDRAW_FLAG_SCALE);
    else
        ret = MApi_GFX_BitBlt(&rectInfo, GFXRECT_FLAG_TRAPE_DIRECTION_X|GFXDRAW_FLAG_SCALE);

    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();
    return true;
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
    GFX_RectFillInfo rectInfo;

    rectInfo.dstTrapezoidBlk.u16X0 = dtrapezoid->x0;
    rectInfo.dstTrapezoidBlk.u16Y0 = dtrapezoid->y0;
    rectInfo.dstTrapezoidBlk.u16X1 = dtrapezoid->x1;
    rectInfo.dstTrapezoidBlk.u16Y1 = dtrapezoid->y1;

    rectInfo.dstTrapezoidBlk.u16DeltaTop    = dtrapezoid->top_edge_width;
    rectInfo.dstTrapezoidBlk.u16DeltaBottom = dtrapezoid->bottom_edge_width;

    rectInfo.fmt = sdrv->dst_ge_format;

    rectInfo.colorRange.color_s.a = sdrv->color.a;
    rectInfo.colorRange.color_s.r = sdrv->color.r;
    rectInfo.colorRange.color_s.g = sdrv->color.g;
    rectInfo.colorRange.color_s.b = sdrv->color.b;

    if(false == dtrapezoid->horiz_direct)
        rectInfo.flag = GFXDRAW_FLAG_TRAPEZOID_Y;
    else
        rectInfo.flag = GFXDRAW_FLAG_TRAPEZOID_X;

    MApi_GFX_BeginDraw();
    MApi_GFX_TrapezoidFill(&rectInfo);
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();
    
    return true;
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
    D_DEBUG_AT( MSTAR_G2_Driver, "[DFB] %s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);

    //return dfb_gfxcard_get_accelerator( device ) == 0x2D47;
    return 1;
}


static void
driver_get_info( CoreGraphicsDevice                 *device,
                 GraphicsDriverInfo                 *info )
{
    D_DEBUG_AT( MSTAR_G2_Driver, "[DFB] %s()\n", __FUNCTION__ );

    /* fill driver info structure */
    snprintf( info->name,
              DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
              "Mstar G2 Driver" );

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
     MSTARDeviceData *sdev = device_data;
     serial->generation = 0;
     serial->serial = (unsigned int)MApi_GFX_GetNextTAGID(FALSE);
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
     u16 tagID;

     tagID = (u16)serial->serial;
     MApi_GFX_WaitForTAGID(tagID);

     return DFB_OK;
}


DFBResult
mstarWaitSerial_null( void                          *driver_data,
                      void                          *device_data,
                      const CoreGraphicsSerial      *serial )
{
     return DFB_OK;
}


static DFBResult
mstar_init_driver( CoreGraphicsDevice               *device,
                    void                            *driver_data,
                    void                            *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;

    GFX_Config gfx_config;
    GFX_RgbColor gfxColor;

    hal_phy u32GE_VQ_addr = 0;
    u32 u32GE_VQ_size = 0;
    u32 u32GE_VQ_aligned;
    u32 u32GE_VQ_miu;


    u16 gfx_supported_bld_flags;
    XC_INITDATA XC_InitData;

    D_DEBUG_AT( MSTAR_G2_Driver, "%s()\n", __FUNCTION__ );
    DFB_CHECK_POINT("mstar_init_driver start");

    memset(&XC_InitData, 0, sizeof(XC_InitData));

    ////////////////////////////////////////////////////////////////////////////////

    //Init MsOS & SEM:
    //MsOS_Init();
    //MDrv_SEM_Init();
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GLOBAL_INIT, DF_MEASURE_START, DF_BOOT_LV7);
    Global_Init();
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GLOBAL_INIT, DF_MEASURE_END, DF_BOOT_LV7);
    DFB_CHECK_POINT("Global_Init done");

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_IOMAP_INIT, DF_MEASURE_START, DF_BOOT_LV7);
    DFB_UTOPIA_TRACE(MApi_PNL_IOMapBaseInit());
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_IOMAP_INIT, DF_MEASURE_END, DF_BOOT_LV7);

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_VE_INIT, DF_MEASURE_START, DF_BOOT_LV7);

    if (MDrv_VE_SetIOMapBase != NULL)
         DFB_UTOPIA_TRACE(MDrv_VE_SetIOMapBase());

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_VE_INIT, DF_MEASURE_END, DF_BOOT_LV7);

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_XC_INIT, DF_MEASURE_START, DF_BOOT_LV7);
    DFB_UTOPIA_TRACE(MApi_XC_Init(&XC_InitData, 0));
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_XC_INIT, DF_MEASURE_END, DF_BOOT_LV7);

    ////////////////////////////////////////////////////////////////////////////////


    //Init GOP:
    //Get GOP Idx:
    sdev->gfx_gop_index = dfb_config->mst_gfx_gop_index;


    mstar_init_gop_driver(device, driver_data);

    ////////////////////////////////////////////////////////////////////////////////

    //Init GE:
    //Get VQ Addr & Size:
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_START, DF_BOOT_LV7);

    D_INFO("[DFB] GE VQ Setting = %d, ge_vq_phys_addr = 0x%08x, ge_vq_phys_len = %d \n",
                dfb_config->mst_enable_gevq,
                dfb_config->mst_ge_vq_phys_addr,
                dfb_config->mst_ge_vq_phys_len);

    if(dfb_config->mst_enable_gevq      == true     &&
        dfb_config->mst_ge_vq_phys_addr != 0        &&
        dfb_config->mst_ge_vq_phys_len  != 0 )
    {
        u32GE_VQ_addr = dfb_config->mst_ge_vq_phys_addr;
        u32GE_VQ_size = dfb_config->mst_ge_vq_phys_len;
    }


    //Prepare GE Init Params:
    gfx_config.u8Dither = FALSE;
    gfx_config.bIsCompt = FALSE;
    gfx_config.bIsHK = TRUE;
    gfx_config.u8Miu = ((dfb_config->video_phys_cpu >= dfb_config->mst_miu1_cpu_offset) ? 1 : 0);
    gfx_config.u32VCmdQAddr = u32GE_VQ_addr;
    gfx_config.u32VCmdQSize =  u32GE_VQ_size;

    //Init GE:
    DFB_UTOPIA_TRACE(MApi_GFX_Init(&gfx_config));

    //Get Supported Bliting Flags Here:
    DFB_UTOPIA_TRACE(MApi_GFX_QueryDFBBldCaps(&gfx_supported_bld_flags));
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
                                      DSBLIT_BA_TRANSPARENT
                                        );



    sdrv->gfx_supported_draw_flags = DSDRAW_NOFX;

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_COLORALPHA)
        sdrv->gfx_supported_bld_flags |= DSBLIT_BLEND_COLORALPHA;

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_ALPHACHANNEL)
    {
        sdrv->gfx_supported_bld_flags |= DSBLIT_BLEND_ALPHACHANNEL;
        sdrv->gfx_supported_draw_flags|= DSDRAW_BLEND;
    }

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_COLORIZE)
        sdrv->gfx_supported_bld_flags |= DSBLIT_COLORIZE;

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_SRCPREMUL)
    {
        sdrv->gfx_supported_bld_flags |= DSBLIT_SRC_PREMULTIPLY;
        sdrv->gfx_supported_draw_flags|= DSDRAW_SRC_PREMULTIPLY;

    }

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_SRCPREMULCOL)
        sdrv->gfx_supported_bld_flags |= DSBLIT_SRC_PREMULTCOLOR;

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_DSTPREMUL)
    {
        sdrv->gfx_supported_bld_flags |= DSBLIT_DST_PREMULTIPLY;
        sdrv->gfx_supported_draw_flags|= DSDRAW_DST_PREMULTIPLY;
    }

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_XOR)
    {
        sdrv->gfx_supported_bld_flags |= DSBLIT_XOR;
        sdrv->gfx_supported_draw_flags|= DSDRAW_XOR;
    }
    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_DEMULTIPLY)
    {
        sdrv->gfx_supported_bld_flags |= DSBLIT_DEMULTIPLY;
        sdrv->gfx_supported_draw_flags|= DSDRAW_DEMULTIPLY;
    }

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_SRCALPHAMASK)
        sdrv->gfx_supported_bld_flags |= DSBLIT_SRC_MASK_ALPHA;

    if(gfx_supported_bld_flags & GFX_DFB_BLD_FLAG_SRCCOLORMASK)
        sdrv->gfx_supported_bld_flags |= DSBLIT_SRC_MASK_COLOR;

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

    sdrv->ge_src_blend      = GFX_DFB_BLD_OP_ONE;
    sdrv->ge_dst_blend      = GFX_DFB_BLD_OP_ONE;
    sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_NONE;
    sdrv->src_ge_yuv_fmt    = GFX_YUV_YUYV;
    sdrv->dst_ge_yuv_fmt    = GFX_YUV_YUYV;

    // 0767408: [Mxxx][ATSC] OSD red edge
    gfxColor.a = 0x00;
    gfxColor.r = 0x00;
    gfxColor.g = 0x00;
    gfxColor.b = 0x00;

    //Set Rop2
    sdrv->ge_rop_op = DFB_ROP2_PD;
    sdrv->ge_rop_enable = FALSE;

    DFB_UTOPIA_TRACE(MApi_GFX_SetStrBltSckType(GFX_NEAREST, &gfxColor));

    //init argb1555 alpha value to 0xff
    DFB_UTOPIA_TRACE(MApi_GFX_SetAlpha_ARGB1555(0xff));
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GE_INIT, DF_MEASURE_END, DF_BOOT_LV7);

    ////////////////////////////////////////////////////////////////////////////////
    //Init DIP:
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_DIP_INIT, DF_MEASURE_START, DF_BOOT_LV7);
    if( (dfb_config->mst_enable_dip         == true)    &&
        (dfb_config->mst_dip_mload_addr     != 0)       &&
        (dfb_config->mst_dip_mload_length   != 0) )
    {
        void *pvirtual_address = NULL;
        hal_phy miu_offset;
        hal_phy offset;
        int miu_select = 0;

        D_INFO("[DFB] dfb_config->mst_dip_mload_addr:%llx, dfb_config->mst_dip_mload_length:%x \n",
                dfb_config->mst_dip_mload_addr,
                dfb_config->mst_dip_mload_length);

        DFB_UTOPIA_TRACE(pvirtual_address = (void *)MsOS_MPool_PA2KSEG1( dfb_config->mst_dip_mload_addr ));

        if(NULL == pvirtual_address)
        {
            if ( dfb_config->mst_dip_mload_addr >= dfb_config->mst_miu2_hal_offset &&  dfb_config->mst_miu2_hal_offset != 0)
            {
                miu_offset = dfb_config->mst_miu2_hal_offset;
                miu_select = 2;
            }
            else if ( dfb_config->mst_dip_mload_addr >= dfb_config->mst_miu1_hal_offset &&  dfb_config->mst_miu1_hal_offset != 0)
            {
                miu_offset = dfb_config->mst_miu1_hal_offset;
                miu_select = 1;
            }
            else
            {
                miu_offset = dfb_config->mst_miu0_hal_offset;
                miu_select = 0;
            }

            offset = (dfb_config->mst_dip_mload_addr-miu_offset);

            D_INFO("[DFB] MsOS_MPool_Mapping offset:%llx, dfb_config->mst_dip_mload_length:%x \n",
                offset,
                dfb_config->mst_dip_mload_length);

            MS_BOOL ret=0;
            DFB_UTOPIA_TRACE(ret = MsOS_MPool_Mapping(miu_select, offset, dfb_config->mst_dip_mload_length, 1));

            if( !ret )
            {
                printf("[DFB] Waring! DIP map buf error! \n");
            }
            else
            {
                DFB_UTOPIA_TRACE(MApi_XC_MLoad_Init( dfb_config->mst_dip_mload_addr,
                                    dfb_config->mst_dip_mload_length));
                DFB_UTOPIA_TRACE(MApi_XC_MLoad_Enable(TRUE));
            }
        }
        else
        {
            DFB_UTOPIA_TRACE(MApi_XC_MLoad_Init( dfb_config->mst_dip_mload_addr,
                                dfb_config->mst_dip_mload_length));
            DFB_UTOPIA_TRACE(MApi_XC_MLoad_Enable(TRUE));
        }      
    }
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_DIP_INIT, DF_MEASURE_END, DF_BOOT_LV7);

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
    u32 u32GE_VQ_addr = 0;
    u32 u32GE_VQ_size = 0;
    u32 u32GE_VQ_aligned;
    u32 u32GE_VQ_miu;
    GFX_RgbColor gfxColor;

    DFB_CHECK_POINT("mstar_init_device start");
    DFB_BOOT_GETTIME( DF_BOOT_DEVICE_GOP_INIT, DF_MEASURE_START, DF_BOOT_LV7);

    //Init cmdq
    if(dfb_config->mst_cmdq_phys_len != 0)
    {
        void *pVirAddr;

        if(MDrv_CMDQ_Init != NULL)
            DFB_UTOPIA_TRACE(MDrv_CMDQ_Init(dfb_config->mst_cmdq_miusel));
        else
            D_WARN("[DFB] mstar_init_device, Not surpport CMDQ in this platform.\n");

        DFB_UTOPIA_TRACE(pVirAddr = (void *)MsOS_MPool_PA2KSEG1(dfb_config->mst_cmdq_phys_addr));
        if(NULL == pVirAddr)
        {
            MS_BOOL ret = 0;
            DFB_UTOPIA_TRACE(ret = MsOS_MPool_Mapping(dfb_config->mst_cmdq_miusel, dfb_config->mst_cmdq_phys_addr, dfb_config->mst_cmdq_phys_len,TRUE ));
            if(!ret)
            {
                D_WARN("[DFB] mapping cmdq_addr 0x%lx failed, some func will not work well\n",
                    dfb_config->mst_cmdq_phys_addr);
            }
        } else
        {
            D_WARN("[DFB] cmdq_addr 0x%lx have mapped in current process.\n",
                    dfb_config->mst_cmdq_phys_addr);
        }

        if(MDrv_CMDQ_Get_Memory_Size != NULL)
            DFB_UTOPIA_TRACE(MDrv_CMDQ_Get_Memory_Size(dfb_config->mst_cmdq_phys_addr,
                dfb_config->mst_cmdq_phys_addr + dfb_config->mst_cmdq_phys_len, dfb_config->mst_cmdq_miusel));
        else
            D_WARN("[DFB] mstar_init_device, Not surpport CMDQ in this platform.\n");
    }


    if(dfb_config->mst_null_display_driver == false)
    {
        /* Init GOP */
        CoreDFB   *core = sdrv ? sdrv->core : NULL;
        mstar_init_gop_device(core);
    }

    DFB_BOOT_GETTIME( DF_BOOT_DEVICE_GOP_INIT, DF_MEASURE_END, DF_BOOT_LV7);

    DFB_BOOT_GETTIME( DF_BOOT_DEVICE_GFX_INIT, DF_MEASURE_START, DF_BOOT_LV7);



    if(dfb_config->mst_enable_gevq)
    {
        //Get VQ Addr & Size:
        u32GE_VQ_addr = dfb_config->mst_ge_vq_phys_addr;
        u32GE_VQ_size = dfb_config->mst_ge_vq_phys_len;
    }
    else
    {
        //turn off GE VQ
        u32GE_VQ_size = 0;
    }


    //Set VCMDQ & Buf:
    if(u32GE_VQ_size>=4*1024)
    {
        GFX_VcmqBufSize vqSize;
        GFX_Result ret = GFX_FAIL;

        if(u32GE_VQ_size>=512*1024)
            vqSize = GFX_VCMD_512K;
        else if(u32GE_VQ_size>=256*1024)
            vqSize = GFX_VCMD_256K;
        else if(u32GE_VQ_size>=128*1024)
            vqSize = GFX_VCMD_128K;
        else if(u32GE_VQ_size>=64*1024)
            vqSize = GFX_VCMD_64K;
        else if(u32GE_VQ_size>=32*1024)
            vqSize = GFX_VCMD_32K;
        else if(u32GE_VQ_size>=16*1024)
            vqSize = GFX_VCMD_16K;
        else if(u32GE_VQ_size>=8*1024)
            vqSize = GFX_VCMD_8K;
        else
            vqSize = GFX_VCMD_4K;

        u32GE_VQ_addr = u32GE_VQ_addr;

        DFB_UTOPIA_TRACE(ret = MApi_GFX_SetVCmdBuffer(u32GE_VQ_addr, vqSize));
        D_INFO("[DFB] GE Enable: u32GE_VQ_addr: %x, vqSize:%d \n",u32GE_VQ_addr, vqSize);

        if(GFX_SUCCESS == ret)
            DFB_UTOPIA_TRACE(MApi_GFX_EnableVCmdQueue(TRUE));
        else
            DFB_UTOPIA_TRACE(MApi_GFX_EnableVCmdQueue(FALSE));
    }
    else 
    {
        DFB_UTOPIA_TRACE(MApi_GFX_EnableVCmdQueue(FALSE));
    }

    DFB_BOOT_GETTIME( DF_BOOT_DEVICE_GFX_INIT, DF_MEASURE_END, DF_BOOT_LV7);
    DFB_CHECK_POINT("mstar_init_device done");

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
    MsOS_FlushMemory();
}


void
mstar_EngineSync_null( void                         *driver_data,
                       void                         *device_data)
{
}


void
mstar_EngineSync( void                              *driver_data,
                  void                              *device_data)
{

    MApi_GFX_FlushQueue();
}


void
mstar_BeginDraw_null( void                          *driver_data,
                      void                          *device_data,
                      CardState                     *state )
{
}


void
mstar_BeginDraw( void                               *driver_data,
                 void                               *device_data,
                 CardState                          *state )
{
    //printf("in %s  \n",__FUNCTION__);
    MApi_GFX_BeginDraw();
}


void
mstar_EndDraw_null( void                            *driver_data,
                    void                            *device_data,
                    CardState                       *state )
{
}


void
mstar_EndDraw( void                                 *driver_data,
               void                                 *device_data,
               CardState                            *state )
{
    //printf("in %s  !!!\n",__FUNCTION__);

    MApi_GFX_EndDraw();
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
    .BeginDraw          = mstar_BeginDraw_null,
    .EndDraw            = mstar_EndDraw_null,
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
    .BeginDraw          = mstar_BeginDraw,
    .EndDraw            = mstar_EndDraw,

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
    .BeginDraw            = mstar_BeginDraw,
    .EndDraw              = mstar_EndDraw,

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

    D_DEBUG_AT( MSTAR_G2_Driver, "%s()\n", __FUNCTION__ );
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DRIVER, DF_MEASURE_START, DF_BOOT_LV6);

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

#if defined(USE_GFX_EXTENSION)
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
            check_GLES2();
            funcs->BlitGLES2 = (glesFuncs.GLES2Blit) ? mstarBlit_GLES2 : NULL;

            if ( funcs->BlitGLES2 == NULL )
                printf("[DFB] driver_init_driver, not define BlitGLES2!\n");
            else
            {
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

                if (glesFuncs.InitSDR2HDRParameter)
                    glesFuncs.InitSDR2HDRParameter(sdev->sdr2hdr_param);
                else
                    printf("\33[0;33;44m[DFB]\33[0m glesFuncs.InitSDR2HDRParameter error!\n");
            }
        }
    }


     /* Get virtual address for the LCD buffer in slaves here,
        master does it in driver_init_device(). */
     if (!dfb_core_is_master( core ))
          sdrv->lcd_virt = dfb_gfxcard_memory_virtual( device, sdev->lcd_offset );


     /* Register multi window layer. */
     //sdrv->multi = dfb_layers_register( sdrv->screen, driver_data, &sh7722MultiLayerFuncs );
     ret = mstar_init_driver(device, driver_data, device_data);
     DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DRIVER, DF_MEASURE_END, DF_BOOT_LV6);


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
    D_DEBUG_AT( MSTAR_G2_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);
    //  printf("MAdp_SYS_GetPanelResolution-->%d, %d\n", pnlW, pnlH);
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DEVICE, DF_MEASURE_START, DF_BOOT_LV6);

    for(i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {
        sdev->layer_active[i]       = false;
        sdev->layer_pinpon_mode[i]  = false;
        sdev->layer_gwin_id[i]      = INVALID_WIN_ID;
        sdev->layer_gwin_id_r[i]    = INVALID_WIN_ID;
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
    memset(sdev->palette_tbl, 0xFF, (sizeof(GFX_PaletteEntry)*MAX_PALETTE_ENTRY_CNT));

    for(i = 0; i < MAX_PALETTE_ENTRY_CNT; ++i)
    {
        sdev->palette_tbl[i].RGB.u8A = i;
    }

    sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
    sdev->a8_palette = true;
    DFB_UTOPIA_TRACE(MApi_GFX_SetPaletteOpt(sdev->palette_tbl, 0, (sdev->num_entries-1)));

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

    //default the hwclip is enable
    //device_info->caps.flags |= CCF_CLIPPING;
    //device_info->caps.clip = DFXL_ALL;

    device_info->caps.flags &= ~CCF_CLIPPING;
    device_info->caps.clip = DFXL_STRETCHBLIT;
    sdev->b_hwclip = true;

    if(dfb_config->mst_disable_hwclip)
    {
        device_info->caps.flags &= ~CCF_CLIPPING;
        device_info->caps.clip = 0;
        sdev->b_hwclip = false;
    }

    if ( true == dfb_config->mst_MIU_protect ) {
        /***
        about MIU Protect
        Protect some region to prevent all MIU clients 
        but some specific clients to write it.
        Hardware provide 4 blocks for MIU protection. (default=3)

        Associated code is referenced to drvMIU.h structure : eMIUClientID
        path : //DAILEO/Supernova/develop/include/utopia/chip/drvMIU.h
        
        ***/

        MS_U8 u8MIUProtectkernel_ID[16] = {0};

        u8MIUProtectkernel_ID[0] =  MIU_CLIENT_MIPS_RW;
        u8MIUProtectkernel_ID[1] =  MIU_CLIENT_GE_RW;
        u8MIUProtectkernel_ID[2] =  MIU_CLIENT_JPD_RW;
        u8MIUProtectkernel_ID[3] =  MIU_CLIENT_GPD_RW;
        u8MIUProtectkernel_ID[4] =  MIU_CLIENT_SC_DIPW_RW;


#ifdef ARCH_ARM
        u8MIUProtectkernel_ID[5] = MIU_CLIENT_SC_DWIN_W;
#else
        /*
        K2 drvMIU.h undefine MIU_CLIENT_SC_DWIN_W       
        */
        printf("\33[0;33;44m[DFB] MIPS K2 drvMIU.h undefine MIU_CLIENT_SC_DWIN_W \33[0m\n");
#endif


        int MIU_TEST_ADDR0 = dfb_config->video_phys_hal; //u32Phys; 
        int MIU_TEST_LEN = dfb_config->video_length; //config->width * config->height * config->format;

        bool bRet = 0;
        u32 pageShift = 1 << 13;  // alignment to 8kb
        
        int protectBlockID = dfb_config->mst_MIU_protect_BlockID;

        DFB_UTOPIA_TRACE(MDrv_MIU_ProtectAlign(&pageShift));

        DFB_UTOPIA_TRACE(bRet = MDrv_MIU_Protect(protectBlockID,
                                u8MIUProtectkernel_ID, 
                                ALIGN(MIU_TEST_ADDR0 + (1<<pageShift-1), pageShift), 
                                ALIGN(MIU_TEST_ADDR0 + MIU_TEST_LEN, pageShift), 
                                true));


        if (bRet == false) {
            printf("\33[0;33;44m[DFB]\33[0m MIU protect fail\n");
        }
        else {
            printf("\33[0;33;44m[DFB]\33[0m MIU protect OK\n");  

            pthread_t id;

            bRet = pthread_create(&id, NULL, (void *) MIUProtectThread, NULL);

            if( 0 != bRet)
            {
                printf("\33[0;33;44m[DFB]\33[0m Create MIU protect pthread error \n");
            } 
        }  
    }   // end of if ( true == dfb_config->mst_MIU_protect )


    mstar_init_device(device, driver_data, device_data);
    /* Initialize BEU lock. */
    fusion_skirmish_init( &sdev->beu_lock, "BEU", dfb_core_world(sdrv->core) );
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_INIT_DEVICE, DF_MEASURE_END, DF_BOOT_LV6);
    return DFB_OK;
}

static void
driver_close_device( CoreGraphicsDevice             *device,
                     void                           *driver_data,
                     void                           *device_data )
{
    MSTARDeviceData *sdev = device_data;

    D_DEBUG_AT( MSTAR_G2_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);

#if USE_MSTAR_CMA     
    //Register callback functions:
    MApi_GOP_RegisterFBFmtCB(NULL);
    MApi_GOP_RegisterXCIsInterlaceCB(NULL);
    MApi_GOP_RegisterXCGetCapHStartCB(NULL);
    MApi_GOP_RegisterXCReduceBWForOSDCB(NULL);
#endif    

    /* Destroy BEU lock. */
    fusion_skirmish_destroy( &sdev->beu_lock );
}


void
mstarAllSurfInfo( MSTARDriverData                   *sdrv);


static void
driver_close_driver( CoreGraphicsDevice             *device,
                     void                           *driver_data )
{
     MSTARDriverData *sdrv = driver_data;

     D_DEBUG_AT( MSTAR_G2_Driver, "%s()\n", __FUNCTION__ );

#ifdef ARCH_ARM
    // Un-mapping VA of DIP
    if( (dfb_config->mst_enable_dip         == true)    &&
        (dfb_config->mst_dip_mload_addr     != 0)       &&
        (dfb_config->mst_dip_mload_length   != 0) )
    {
        void *pvirtual_address = NULL;

         pvirtual_address = (void*)MsOS_MPool_PA2KSEG1(dfb_config->mst_dip_mload_addr);

//       if(pvirtual_address)
//            MsOS_MPool_UnMapping(pvirtual_address, dfb_config->mst_dip_mload_length);
    }

    // Un-mapping VA of REGDMA
    if( dfb_config->layer_support_palette           &&
        dfb_config->mst_gop_regdmaphys_addr != 0    &&
        dfb_config->mst_gop_regdmaphys_len  != 0 )
    {
        void *pvirtual_address = NULL;

         pvirtual_address = (void*)MsOS_MPool_PA2KSEG1(dfb_config->mst_gop_regdmaphys_addr);

        if(pvirtual_address)
        {
              MsOS_MPool_UnMapping(pvirtual_address, dfb_config->mst_gop_regdmaphys_len);
        }
    }
#endif
     mstarAllSurfInfo(sdrv);
}




#if !USE_SIZE_OPTIMIZATION
MS_BOOL
GE_API_Performance( hal_phy                          u32SrcAddr,
                    hal_phy                          u32DstAddr,
                    MS_U32                          u32SrcWidth,
                    MS_U32                          u32SrcHeight,
                    MS_U32                          u32DstWidth,
                    MS_U32                          u32DstHeight,
                    MS_BOOL                         bBlit,
                    MS_BOOL                         bBlending,
                    MS_U32                          u32Rotate )
{

    int LOOP_CNT = 1000;

    MS_U32 u32GfxScale = GFXDRAW_FLAG_DEFAULT;

    GFX_DrawRect        stBitbltInfo;
    GFX_RectFillInfo    stFillBlockInfo;
    GFX_Point           stP0, stP1;
    GFX_BufferInfo      stSrcbuf;
    GFX_BufferInfo      stDstbuf;

    int i;
    float TotalPixel    = 0;
    float TimeStart     = 0;
    float TimeEnd       = 0;
    float MPPerSec      = 0;
        
    stSrcbuf.u32Addr        = u32SrcAddr;
    stSrcbuf.u32ColorFmt    = GFX_FMT_ARGB8888;
    stSrcbuf.u32Width       = u32SrcHeight;
    stSrcbuf.u32Pitch       = u32SrcHeight*4;
    stSrcbuf.u32Height      = u32SrcHeight;

    stDstbuf.u32Addr        = u32DstAddr;
    stDstbuf.u32ColorFmt    = GFX_FMT_ARGB8888;
    stDstbuf.u32Width       = u32DstWidth;
    stDstbuf.u32Pitch       = u32DstWidth*4;
    stDstbuf.u32Height      = u32DstHeight;

    TotalPixel = u32DstWidth*u32DstHeight*LOOP_CNT;
    TimeStart = MsOS_GetSystemTime();

    for(i = 0; i < LOOP_CNT; i++)
    {
        MApi_GFX_BeginDraw();
        
        if(bBlending)
        {
            MApi_GFX_EnableDFBBlending(TRUE);
            MApi_GFX_EnableAlphaBlending(TRUE);
            MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }
        else
        {
            MApi_GFX_EnableDFBBlending(FALSE);
            MApi_GFX_EnableAlphaBlending(FALSE);
            MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }
        
        if ((stSrcbuf.u32Width == stDstbuf.u32Width)        &&
            (stSrcbuf.u32Height == stDstbuf.u32Height))
        {
            u32GfxScale = GFXDRAW_FLAG_DEFAULT;
        }
        else
        {
            u32GfxScale = GFXDRAW_FLAG_SCALE;
        }

        stP0.x = 0;
        stP0.y = 0;
        stP1.x = stDstbuf.u32Width;
        stP1.y = stDstbuf.u32Height;
        MApi_GFX_SetClip(&stP0, &stP1);

        if (MApi_GFX_SetSrcBufferInfo(&stSrcbuf, 0) != GFX_SUCCESS)
        {
            printf("\33[0;33;44m[DFB]\33[0m[DipDataHandleThread]GFX set SrcBuffer failed\n");
            return FALSE;
        }

        if (MApi_GFX_SetDstBufferInfo(&stDstbuf, 0) != GFX_SUCCESS)
        {
            printf("\33[0;33;44m[DFB]\33[0m[DipDataHandleThread]GFX set DetBuffer failed\n");
            return FALSE;
        }

        if(bBlit)
        {
            stBitbltInfo.srcblk.x       = 0;
            stBitbltInfo.srcblk.y       = 0;
            stBitbltInfo.srcblk.width   = stSrcbuf.u32Width;
            stBitbltInfo.srcblk.height  = stSrcbuf.u32Height;

            stBitbltInfo.dstblk.x       = 0;
            stBitbltInfo.dstblk.y       = 0;
            stBitbltInfo.dstblk.width   = stDstbuf.u32Width;
            stBitbltInfo.dstblk.height  = stDstbuf.u32Height;

            switch (u32Rotate)
            {
                case 0:
                    break;

                case GEROTATE_90:
                    MApi_GFX_SetRotate(GEROTATE_90);
                    break;

                case GEROTATE_180:
                    MApi_GFX_SetRotate(GEROTATE_180);
                    break;

                case GEROTATE_270:
                    MApi_GFX_SetRotate(GEROTATE_270);
                    break;

                default:
                    break;
            }

            if (MApi_GFX_BitBlt(&stBitbltInfo, u32GfxScale) != GFX_SUCCESS)
            {
                printf("\33[0;33;44m[DFB]\33[0m[DipDataHandleThread]GFX BitBlt failed\n");
                return FALSE;
            }
        }
        else
        {
            stFillBlockInfo.dstBlock.x      = 0;
            stFillBlockInfo.dstBlock.y      = 0;
            stFillBlockInfo.dstBlock.width  = stDstbuf.u32Width;
            stFillBlockInfo.dstBlock.height = stDstbuf.u32Height;
            stFillBlockInfo.fmt             = GFX_FMT_ARGB8888;

            if (MApi_GFX_RectFill(&stFillBlockInfo) != GFX_SUCCESS)
            {
                printf("\33[0;33;44m[DFB]\33[0m[DipDataHandleThread]GFX BitBlt failed\n");
                return FALSE;
            }
        }
        MApi_GFX_EndDraw();
    }

    if (MApi_GFX_FlushQueue() != GFX_SUCCESS)
    {
        printf("[DFB][DipDataHandleThread]GFX FlushQueue failed\n");
        return FALSE;
    }

    TimeEnd = MsOS_GetSystemTime();
    MPPerSec = (float)(TotalPixel/1024/1024)/((TimeEnd - TimeStart)/1000);
    printf("\33[0;33;44m[DFB] Total MPixel = %02f, time = %02f Sec, MPixel/Sec = %02f \33[m\n\n",
            (float)(TotalPixel/1024/1024),
            (float)((TimeEnd - TimeStart)/1000),
            MPPerSec);


    return TRUE;
}

#endif
