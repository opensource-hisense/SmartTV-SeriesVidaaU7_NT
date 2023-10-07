//#ifdef MSTAR_DEBUG_DRIVER
#define DIRECT_ENABLE_DEBUG
//#endif

#include <stdio.h>

#undef HAVE_STDLIB_H

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <fcntl.h>

#include <asm/types.h>

#include <directfb.h>
#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/system.h>

#include <misc/conf.h>

#include <core/core.h>
#include <core/gfxcard.h>
#include <core/layers.h>
#include <core/screens.h>
#include <core/system.h>

#include <core/graphics_driver.h>

#include <direct/mem.h>
#include <direct/memcpy.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/state.h>
#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>

#include  <core/palette.h>
#include <gfx/convert.h>

#include <MsCommon.h>
#include <drvSEM.h>
#include  <apiGFX.h>
#include  <apiGOP.h>
#include  <drvMMIO.h>

#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <drvMVOP.h>
#include <apiXC.h>
#include <apiPNL.h>
#if DFB_BUILD_FOR_PRJ_OBAMA
#include "madp.h"
#include "dfb_mmap_ref.h"
#endif
#include "mstar_layer.h"
#include "mstar_screen.h"
#include "mstar_ablmaptbl.h"

#define MSTAR_DFB_A8_SUPPORT 0
#define MSTAR_DFB_BLT_MIRROR_PATCH  1

#if MSTAR_DFB_BLT_MIRROR_PATCH
#define SET_BLT_MIRROR_SRCX(sdrv, rectinfo)    \
    if (sdrv->ge_blit_xmirror) \
    { \
        rectinfo.srcblk.x += rectinfo.srcblk.width - 1; \
    }

#define SET_BLT_MIRROR_SRCY(sdrv, rectinfo)    \
    if (sdrv->ge_blit_ymirror) \
    { \
        rectinfo.srcblk.y += rectinfo.srcblk.height - 1; \
    }
#else
#define SET_BLT_MIRROR_X(sdrv, rectinfo)
#define SET_BLT_MIRROR_Y(sdrv, rectinfo)
#endif

DFB_GRAPHICS_DRIVER( mstar)


D_DEBUG_DOMAIN( MSTAR_Driver, "MSTAR/Driver", "MSTAR Driver" );

void AddGraphicsDriverModule()
{
}

typedef enum
{
   MSTAR_FILL_RECT = 0x00000001,
   MSTAR_DRAW_RECT = 0x00000002,
   MSTAR_DRAW_LINE = 0x00000004,
   MSTAR_TRAPEZOID_FILL = 0x00000008,
   MSTAR_DRAW_OVAL = 0x00000010,
   MSTAR_BIT_BLIT = 0x00010000,
   MSTAR_STRETCH = 0x00020000,
   MSTAR_TRAPEZOID_BLIT = 0x00040000,
   MSTAR_OPT_MAX = 0xffffffff
}MSTAR_OP_TYPE;

#define DEFAULT_MAX_GFX_WIDTH   1366
#define DEFAULT_MAX_GFX_HEIGHT  768

#define IS_MSTAR_BLIT_OP(op)  ((op) & 0xffff0000)

#define MSTAR_GFX_RENDER_OP_NONE          0x0
#define MSTAR_GFX_RENDER_OP_DRAW          0x1
#define MSTAR_GFX_RENDER_OP_BLIT          0x2

static inline GFX_Buffer_Format adjustcolorformat(MSTARDriverData * sdrv)
{
   if(!sdrv)
       return -1;

   if((sdrv->src_ge_format ==GFX_FMT_I8 || sdrv->src_ge_format == GFX_FMT_I4 || sdrv->src_ge_format == GFX_FMT_I2 || sdrv->src_ge_format == GFX_FMT_I1 ) && sdrv->dst_ge_format != GFX_FMT_I8)
       return GFX_FMT_ARGB8888;
   else
       return (GFX_Buffer_Format)sdrv->src_ge_format;

}

unsigned long
    mstar_pixel_to_colorkey( DFBSurfacePixelFormat  format,
                      unsigned long pixel, CorePalette *palette);
unsigned long
    mstar_pixel_to_color( DFBSurfacePixelFormat  format,
                      unsigned long pixel, CorePalette *palette);


static void dumpFormat( unsigned int format ){
    switch(format){
    case DSPF_UNKNOWN:
        printf( "DSPF_UNKNOWN\n" ) ;
        break ;
    case DSPF_ARGB1555:
        printf( "DSPF_ARGB1555\n" ) ;
        break;
    case DSPF_RGB16:
        printf( "DSPF_RGB16\n" ) ;
        break ;
    case DSPF_RGB24:
        printf( "DSPF_RGB24\n" ) ;
        break ;
    case DSPF_RGB32:
        printf( "DSPF_RGB32\n" ) ;
        break ;
    case DSPF_ARGB:
        printf( "DSPF_ARGB\n" ) ;
        break ;
    case DSPF_A8:
        printf( "DSPF_A8\n" ) ;
        break ;
    case DSPF_YUY2:
        printf( "DSPF_YUY2\n" ) ;
        break ;
    case DSPF_YVYU:
        printf( "DSPF_YVYU\n" ) ;
        break ;
    case DSPF_RGB332:
        printf( "DSPF_RGB332\n" ) ;
        break ;
    case DSPF_UYVY:
        printf( "DSPF_UYVY\n" ) ;
        break ;
    case DSPF_I420:
        printf( "DSPF_I420\n" ) ;
        break ;
    case DSPF_YV12:
        printf( "DSPF_YV12\n" ) ;
        break ;
    case DSPF_LUT8:
        printf( "DSPF_LUT8\n" ) ;
        break ;
    case DSPF_ALUT44:
        printf( "DSPF_ALUT44\n" ) ;
        break ;
     /* 32 bit  ARGB (4 byte, inv. alpha 8@24, red 8@16, green 8@8, blue 8@0) */
    case DSPF_AiRGB:
        printf( "DSPF_AiRGB\n" ) ;
        break ;
     /*  1 bit alpha (1 byte/ 8 pixel, most significant bit used first) */
    case DSPF_A1:
        printf( "DSPF_A1\n" ) ;
        break ;
     /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cb|Cr [7:0|7:0] plane) */
    case DSPF_NV12:
        printf( "DSPF_NV12\n" ) ;
        break ;
     /* 16 bit   YUV (8 bit Y plane followed by one 16 bit half width Cb|Cr [7:0|7:0] plane) */
    case DSPF_NV16:
        printf( "DSPF_NV16\n" ) ;
        break ;
     /* 16 bit  ARGB (2 byte, alpha 2@14, red 5@9, green 5@4, blue 4@0) */
    case DSPF_ARGB2554:
        printf( "DSPF_ARGB2554\n" ) ;
        break ;
     /* 16 bit  ARGB (2 byte, alpha 4@12, red 4@8, green 4@4, blue 4@0) */
    case DSPF_ARGB4444:
        printf( "DSPF_ARGB4444\n" ) ;
        break ;
     /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cr|Cb [7:0|7:0] plane) */
    case DSPF_NV21:
        printf( "DSPF_NV21\n" ) ;
        break ;
     /* 32 bit  AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0) */
    case DSPF_AYUV:
        printf( "DSPF_AYUV\n" ) ;
        break ;
     /*  4 bit alpha (1 byte/ 2 pixel, more significant nibble used first) */
    case DSPF_A4:
        printf( "DSPF_A4\n" ) ;
        break ;
     /*  1 bit alpha (3 byte/  alpha 1@18, red 6@16, green 6@6, blue 6@0) */
    case DSPF_ARGB1666:
        printf( "DSPF_ARGB1666\n" ) ;
        break ;
     /*  6 bit alpha (3 byte/  alpha 6@18, red 6@16, green 6@6, blue 6@0) */
    case DSPF_ARGB6666:
        printf( "DSPF_ARGB6666\n" ) ;
        break ;
     /*  6 bit   RGB (3 byte/   red 6@16, green 6@6, blue 6@0) */
    case DSPF_RGB18:
        printf( "DSPF_RGB18\n" ) ;
        break ;
     /*  2 bit   LUT (1 byte/ 4 pixel, 2 bit color and alpha lookup from palette) */
    case DSPF_LUT2:
        printf( "DSPF_LUT2\n" ) ;
        break ;
     /*  1 bit   LUT (1 byte/ 8 pixel, 1 bit color and alpha lookup from palette) */
    case DSPF_LUT1:
        printf( "DSPF_LUT1\n" ) ;
        break ;
     /*  4 bit   LUT (1 byte/ 2 pixel, 4 bit color and alpha lookup from palette) */
    case DSPF_LUT4:
        printf( "DSPF_LUT4\n" ) ;
        break ;
     /* 16 bit   RGB (2 byte, nothing @12, red 4@8, green 4@4, blue 4@0) */
    case DSPF_RGB444:
        printf( "DSPF_RGB444\n" ) ;
        break ;
     /* 16 bit   RGB (2 byte, nothing @15, red 5@10, green 5@5, blue 5@0) */
    case DSPF_RGB555:
        printf( "DSPF_RGB555\n" ) ;
        break ;
     /* 16 bit   BGR (2 byte, nothing @15, blue 5@10, green 5@5, red 5@0) */
    case DSPF_BGR555:
        printf( "DSPF_BGR555\n" ) ;
        break ;
    }
}


u32 mstarGFXAddr(u32 cpuPhysicalAddr, u32 mst_miu1_cpu_offset, u32 mst_miu1_hal_offset)
{
   if(cpuPhysicalAddr >= mst_miu1_cpu_offset)
      return (cpuPhysicalAddr-mst_miu1_cpu_offset+mst_miu1_hal_offset);
    else
        return (cpuPhysicalAddr-dfb_config->mst_miu0_cpu_offset+dfb_config->mst_miu0_hal_offset);

   return cpuPhysicalAddr;
}

/*
   return true for hw support this blit options
*/
static inline bool mstarCheckBldOption(DFBSurfaceBlittingFlags  blittingflags,
                                        DFBSurfaceBlendFunction  src_blend,
                                        DFBSurfaceBlendFunction  dst_blend,
                                        u32 *pu32HWBlendOp,
                                        u32 *pu32AlphaSrcFrom,
                                        u8 *pu8constalpha)
{
    u32 u32MapFlag = 0;
    u32 u32AblMapEntryIdx, u32AblMapFlagLo, u32AblMapFlagHi;

    if(blittingflags & ~(DSBLIT_SRC_COLORKEY
                            | DSBLIT_DST_COLORKEY
                            | DSBLIT_DEMULTIPLY
                            | DSBLIT_BLEND_COLORALPHA
                            | DSBLIT_BLEND_ALPHACHANNEL
                            | DSBLIT_SRC_PREMULTIPLY
                            | DSBLIT_SRC_PREMULTCOLOR
                            | DSBLIT_DST_PREMULTIPLY
                            | (DSBLIT_ROTATE90| DSBLIT_ROTATE180|DSBLIT_ROTATE270)
                            #if MSTAR_DFB_A8_SUPPORT
                            | DSBLIT_COLORIZE
                            #endif
                            ))
        return false;

    blittingflags &= (DSBLIT_DEMULTIPLY | DSBLIT_BLEND_COLORALPHA | DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_SRC_PREMULTIPLY | DSBLIT_SRC_PREMULTCOLOR | DSBLIT_DST_PREMULTIPLY);

    if(blittingflags & DSBLIT_DEMULTIPLY)
        u32MapFlag |= ABLMAP_FLAG_DEMULTIPLY;

    if(blittingflags & DSBLIT_BLEND_COLORALPHA)
        u32MapFlag |= ABLMAP_FLAG_BLEND_COLORALPHA;

    if(blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
        u32MapFlag |= ABLMAP_FLAG_BLEND_ALPHACHANNEL;

    if(blittingflags & DSBLIT_SRC_PREMULTIPLY)
        u32MapFlag |= ABLMAP_FLAG_SRC_PREMULTIPLY;

    if(blittingflags & DSBLIT_SRC_PREMULTCOLOR)
        u32MapFlag |= ABLMAP_FLAG_SRC_PREMULTCOLOR;

    if(blittingflags & DSBLIT_DST_PREMULTIPLY)
        u32MapFlag |= ABLMAP_FLAG_DST_PREMULTIPLY;

    if(u32MapFlag & (ABLMAP_FLAG_BLEND_COLORALPHA | ABLMAP_FLAG_BLEND_ALPHACHANNEL))
    {
        u32AblMapFlagLo = MSTARAblMapFlagTbl[u32MapFlag].u32FlagLo & MSTARAblMapBldopTbl[src_blend][dst_blend].u32FlagLo;
        u32AblMapFlagHi = MSTARAblMapFlagTbl[u32MapFlag].u32FlagHi & MSTARAblMapBldopTbl[src_blend][dst_blend].u32FlagHi;
    }
    else
    {
        u32AblMapFlagLo = MSTARAblMapFlagTbl[u32MapFlag].u32FlagLo;
        u32AblMapFlagHi = MSTARAblMapFlagTbl[u32MapFlag].u32FlagHi;
    }

    u32AblMapEntryIdx = 0;
    if(u32AblMapFlagLo != 0)
    {
        while(!(u32AblMapFlagLo & (0x1<<u32AblMapEntryIdx)))
            u32AblMapEntryIdx++;

        *pu32HWBlendOp = MSTARAblMapTbl[u32AblMapEntryIdx].u32HWBlendOp;
        *pu32AlphaSrcFrom = MSTARAblMapTbl[u32AblMapEntryIdx].u32AlphaSrcFrom;
        if(MSTARAblMapTbl[u32AblMapEntryIdx].bTblConstAlpha)
            *pu8constalpha = MSTARAblMapTbl[u32AblMapEntryIdx].u8ConstAlpha;

        return true;
    }

    u32AblMapEntryIdx = 0;
    if(u32AblMapFlagHi != 0)
    {
        while(!(u32AblMapFlagHi & (0x1<<u32AblMapEntryIdx)))
            u32AblMapEntryIdx++;

        u32AblMapEntryIdx += 0x20;

//coverity fix
///////////////////
        if(u32AblMapEntryIdx>0x20)
            return false;
///////////////////////////////
        *pu32HWBlendOp = MSTARAblMapTbl[u32AblMapEntryIdx].u32HWBlendOp;
        *pu32AlphaSrcFrom = MSTARAblMapTbl[u32AblMapEntryIdx].u32AlphaSrcFrom;
        if(MSTARAblMapTbl[u32AblMapEntryIdx].bTblConstAlpha)
            *pu8constalpha = MSTARAblMapTbl[u32AblMapEntryIdx].u8ConstAlpha;

        return true;
    }

    *pu32HWBlendOp = 0xffffffff;
    return false;
}

static inline bool mstarCheckBlitOption(CardState *state,
                                        u32 *pu32HWBlendOp,
                                        u32 *pu32AlphaSrcFrom,
                                        u8 *pu8constalpha)
{
    DFBSurfaceBlittingFlags  blittingflags = state->blittingflags;
    DFBSurfaceBlendFunction  src_blend = state->src_blend;
    DFBSurfaceBlendFunction  dst_blend = state->dst_blend;
    DFBSurfacePixelFormat    dst_format = state->destination->config.format;

#ifdef PATCH_MSTAR_GE_BLEND_OPT
    if(IS_MSTAR_GE_BLEND_PARAM(state->mstar_patchBlendParam))
    {
        if(state->blittingflags & (DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA))
        {
               *pu32HWBlendOp = (u32)GE_MSTAR_GE_BLEND_COEF(state->mstar_patchBlendParam);
               *pu32AlphaSrcFrom = (u32)GE_MSTAR_GE_ALPHA_SRC(state->mstar_patchBlendParam);
        }
        else
       {
               *pu32HWBlendOp = 0xffffffff;
       }

        return true;
    }
    else
#endif
    if((((DSBLIT_BLEND_COLORALPHA | DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_SRC_PREMULTCOLOR) == blittingflags))
        && (DSBF_ONE == src_blend) && (DSBF_INVSRCALPHA == dst_blend) && (DSPF_ARGB == dst_format))
    {
        if(state->cmp_mode != DSBF_ACMP_OP_NONE)
            return false;

        //The requested Src Over Patch Path:
        *pu32HWBlendOp = 0x80000000;
        *pu32AlphaSrcFrom = 0x80000000;

        return true;
    }

#if MSTAR_DFB_A8_SUPPORT
    if( (blittingflags & DSBLIT_COLORIZE) &&  state->source->config.format != DSPF_A8 )
    {
        //printf("COLORIZE is only supported for DSPF_A8\n");
        return false;
    }
#endif

    return mstarCheckBldOption(blittingflags, src_blend, dst_blend, pu32HWBlendOp, pu32AlphaSrcFrom, pu8constalpha);
}

static inline bool mstarCheckDrawOption(CardState *state,
                                        u32 *pu32HWBlendOp,
                                        u32 *pu32AlphaSrcFrom,
                                        u8 *pu8constalpha)
{
    DFBSurfaceBlittingFlags  blittingflags = DSBLIT_NOFX;
    DFBSurfaceDrawingFlags   drawingflags = state->drawingflags;
    DFBSurfaceBlendFunction  src_blend = state->src_blend;
    DFBSurfaceBlendFunction  dst_blend = state->dst_blend;
    DFBSurfacePixelFormat    format = state->destination->config.format;

    if(drawingflags & DSDRAW_DST_PREMULTIPLY)
        blittingflags |= DSBLIT_DST_PREMULTIPLY;

    if(drawingflags & DSDRAW_DEMULTIPLY)
        blittingflags |= DSBLIT_DEMULTIPLY;

    if(drawingflags & DSDRAW_XOR)
        blittingflags |= DSBLIT_XOR;

    if(drawingflags & DSDRAW_DST_COLORKEY)
        blittingflags |= DSBLIT_DST_COLORKEY;

    if(drawingflags & DSDRAW_BLEND)
        blittingflags |= DSBLIT_BLEND_ALPHACHANNEL;

    if((DSBLIT_BLEND_ALPHACHANNEL == blittingflags)
        && (DSBF_ONE == src_blend) && (DSBF_INVSRCALPHA == dst_blend) && (DSPF_ARGB == format))
    {
        if(state->cmp_mode != DSBF_ACMP_OP_NONE)
            return false;

        //The requested Src Over Patch Path:
        *pu32HWBlendOp = 0x80000000;
        *pu32AlphaSrcFrom = 0x80000000;
        return true;
    }

    return mstarCheckBldOption(blittingflags, src_blend, dst_blend, pu32HWBlendOp, pu32AlphaSrcFrom, pu8constalpha);
}

static void _mstarCheckStateInternal(MSTARDriverData *sdrv, CardState *state, DFBAccelerationMask accel,  DFBAccelerationMask*pAccelRdy)
{
    u32 u32HWBlendOp;
    u32 u32HWAlphaSrcFrom;
    u8  u8HWAlphaConst;

    if(state->render_options & DSRO_MATRIX)
    {
        if(state->matrix[0] !=(1<<16) || state->matrix[1] !=0 || state->matrix[2] !=0 ||
            state->matrix[3] !=0 ||state->matrix[4] !=(1<<16)  || state->matrix[5] !=0 || state->matrix[6] !=0||
            state->matrix[7] !=0|| state->matrix[8] !=(1<<16) )
            {
              printf(" matrix not supported yet!!\n");
            }
    }

    //here check whether the dstbuffer foramt is supported by the  GE
    switch( state->destination->config.format ){
    case DSPF_ARGB1555:
    case DSPF_ARGB:
    case DSPF_YVYU:
    case DSPF_UYVY:
    case DSPF_YUY2:
    case DSPF_LUT8:
    case DSPF_ARGB4444:
    case DSPF_RGB16:
        break;

#if    MSTAR_DFB_A8_SUPPORT
    case DSPF_A8:
        if(DFB_BLITTING_FUNCTION(accel)&&(state->source->config.format == DSPF_A8))
            break;

#endif

    default:
        //printf( "error format %s:%d, 0x%08X\n",__FILE__,__LINE__,state->dst.buffer->format ) ;
        //dumpFormat(state->dst.buffer->format) ;
        return;
    }



    if(DFB_BLITTING_FUNCTION(accel))
    {


        if(
#ifdef PATCH_MSTAR_GE_BLEND_OPT
            IS_MSTAR_GE_BLEND_PARAM(state->mstar_patchBlendParam) ||
#endif
            mstarCheckBlitOption(state, &u32HWBlendOp, &u32HWAlphaSrcFrom, &u8HWAlphaConst))
        {
            switch(state->source->config.format){
            case DSPF_ARGB1555:
            case DSPF_ARGB:
            case DSPF_YUY2:
            case DSPF_YVYU:
            case DSPF_UYVY:
            case DSPF_LUT8:
            case DSPF_LUT4:
            case DSPF_LUT2:
            case DSPF_LUT1:
            case DSPF_ARGB4444:
            case DSPF_RGB16:
#if MSTAR_DFB_A8_SUPPORT
            case DSPF_A8:
#endif
                *pAccelRdy |= accel&(DFXL_BLIT|DFXL_STRETCHBLIT|DFXL_BLIT2);
                break ;
            default:
                break;
                }
        }
        else
        {
            //printf("reject alpha blend %08x, %08x, %08x\n", state->blittingflags, state->src_blend, state->dst_blend);
        }
    }

    if(DFB_DRAWING_FUNCTION(accel))
    {
        if(mstarCheckDrawOption(state, &u32HWBlendOp, &u32HWAlphaSrcFrom, &u8HWAlphaConst))
        {
            *pAccelRdy |= accel&(DFXL_FILLRECTANGLE|DFXL_DRAWLINE|DFXL_DRAWRECTANGLE|DFXL_FILLTRAPEZOID|DFXL_FILLQUADRANGLE) ;
        }
        else
        {
            //printf("reject drawing alpha blend %08x, %08x\n", state->src_blend, state->dst_blend);
        }
    }




}

void mstarCheckState( void                *drv,
                  void                *dev,
                  CardState           *state,
                  DFBAccelerationMask  accel )
{
     MSTARDriverData *sdrv = drv;
     return _mstarCheckStateInternal(sdrv, state, accel, &state->accel) ;
}

static void mstarSetEngineBlitState(StateModificationFlags   modified, MSTARDriverData *sdrv, MSTARDeviceData *sdev)
{
    //Set ColorKey:
    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY ))
    MApi_GFX_SetSrcColorKey(sdrv->ge_src_colorkey_enabled,
        CK_OP_EQUAL, adjustcolorformat(sdrv), &sdrv->src_ge_clr_key, &sdrv->src_ge_clr_key);

    if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY ))
        MApi_GFX_SetDstColorKey(sdrv->ge_blit_dst_colorkey_enabled,
            CK_OP_NOT_EQUAL, (GFX_Buffer_Format)sdrv->dst_ge_format, &sdrv->dst_ge_clr_key, &sdrv->dst_ge_clr_key);

    //Set Alphablending:
    if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
    {
        MApi_GFX_SetAlphaBlending((GFX_BlendCoef)sdrv->ge_blit_render_coef, sdrv->ge_blit_render_alpha_const);
        MApi_GFX_SetAlphaSrcFrom((GFX_AlphaSrcFrom)sdrv->ge_blit_render_alpha_from);
        MApi_GFX_EnableAlphaBlending((MS_BOOL)sdrv->ge_blit_alpha_blend_enabled);
        if(sdrv->ge_render_alpha_cmp_enabled)
        {

          MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }
    }
}

static void mstarSetEngineDrawState(StateModificationFlags   modified, MSTARDriverData *sdrv, MSTARDeviceData *sdev)
{
    //Set ColorKey:
    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_COLORKEY ))
        MApi_GFX_SetSrcColorKey(FALSE,
            CK_OP_EQUAL, adjustcolorformat(sdrv), &sdrv->src_ge_clr_key, &sdrv->src_ge_clr_key);

    if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY ))
        MApi_GFX_SetDstColorKey(sdrv->ge_draw_dst_colorkey_enabled,
            CK_OP_NOT_EQUAL, (GFX_Buffer_Format)sdrv->dst_ge_format, &sdrv->dst_ge_clr_key, &sdrv->dst_ge_clr_key);

    //Set Alphablending:
    if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
    {
        MApi_GFX_SetAlphaBlending((GFX_BlendCoef)sdrv->ge_draw_render_coef, sdrv->ge_draw_render_alpha_const);
        MApi_GFX_SetAlphaSrcFrom((GFX_AlphaSrcFrom)sdrv->ge_draw_render_alpha_from);
        MApi_GFX_EnableAlphaBlending((MS_BOOL)sdrv->ge_draw_alpha_blend_enabled);
        if(sdrv->ge_render_alpha_cmp_enabled)
        {

          MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ASRC);
        }

    }
}

static void mstarSetEngineShareState(StateModificationFlags   modified, DFBAccelerationMask support_accel, MSTARDriverData *sdrv, MSTARDeviceData *sdev)
{
    GFX_Point v0, v1;
    GFX_BufferInfo buf;

    ////////////////////////////////////////////////////////////
    //Set Dst Buffer State, All ops Needed:
    if(modified & SMF_DESTINATION)
    {
        buf.u32Addr = (MS_U32)sdrv->dst_phys;
        buf.u32ColorFmt = sdrv->dst_ge_format;
        buf.u32Width = sdrv->dst_w;
        buf.u32Height = sdrv->dst_h;
        buf.u32Pitch = sdrv->dst_pitch;
        MApi_GFX_SetDstBufferInfo(&buf, 0);

        if(!sdev->b_hwclip)
        {
              v0.x = 0;
              v0.y = 0;
              v1.x = buf.u32Width-1;
              v1.y = buf.u32Height-1;
              MApi_GFX_SetClip(&v0, &v1);
        }
     }
    if((modified & SMF_CLIP) && sdev->b_hwclip )
    {
              v0.x = sdrv->clip.x1;
              v0.y = sdrv->clip.y1;
              v1.x = sdrv->clip.x2;
              v1.y = sdrv->clip.y2;
              MApi_GFX_SetClip(&v0, &v1);
    }

    //Set Alpha Compare:
    if(modified & SMF_ALPHA_CMP)
        MApi_GFX_SetAlphaCmp((MS_BOOL)sdrv->ge_render_alpha_cmp_enabled, sdrv->ge_render_alpha_cmp );

    if(modified & (SMF_DESTINATION | SMF_SOURCE)) //SetYUV Format:
        MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_255, GFX_YUV_OUT_255, GFX_YUV_IN_255, sdrv->src_ge_yuv_fmt,  sdrv->dst_ge_yuv_fmt);

    //Set Blit Only State which won't affect Draw Actions:
    if( DFB_BLITTING_FUNCTION(support_accel) )
    {
        ////////////////////////////////////////////////////////////
        //Set Src Buffer, Blt ops Needed:
        if(modified & SMF_SOURCE)
        {
            buf.u32Addr = (MS_U32)sdrv->src_phys;
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
                MApi_GFX_SetRotate(GEROTATE_90);
            else if(sdrv->bflags & DSBLIT_ROTATE180)
                MApi_GFX_SetRotate(GEROTATE_180);
            else if(sdrv->bflags & DSBLIT_ROTATE270)
                MApi_GFX_SetRotate(GEROTATE_270);
            else
                MApi_GFX_SetRotate(GEROTATE_0);
        }

        //Set Blit Nearest Mode:
        if(modified & SMF_SRC_CONVOLUTION)
            MApi_GFX_SetNearestMode((MS_BOOL)sdrv->ge_blit_nearestmode_enabled);
        //Set Blit Mirror:
        if(modified & (SMF_BLIT_XMIRROR | SMF_BLIT_YMIRROR))
            MApi_GFX_SetMirror((MS_BOOL)sdrv->ge_blit_xmirror, (MS_BOOL)sdrv->ge_blit_ymirror);
    }

    //Set Palette:
    if(sdrv->ge_palette_enabled && sdev->num_entries)
        MApi_GFX_SetPaletteOpt(sdev->palette_tbl, 0, (sdev->num_entries-1));

    //Set Intensity Palette:
    if(sdrv->ge_palette_intensity_enabled)
    {
        int i;
        for(i=0; i<sdev->num_intensity; i++)
        {
            MApi_GFX_SetIntensity(i, GFX_FMT_ARGB8888, (MS_U32 *)&sdev->palette_intensity[i]);
        }
    }
}

void
mstarSetState( void                *drv,
                void                *dev,
                GraphicsDeviceFuncs *funcs,
                CardState           *state,
                DFBAccelerationMask  accel )
{
    MSTARDeviceData *sdev = dev ;
    MSTARDriverData *sdrv = drv;
    CoreSurface       *dsurface = state->destination ;
    DFBAccelerationMask      support_accel = state->accel & accel ;
    CorePalette *palette = NULL;
    CorePalette *palette_intensity = NULL;
    DFBSurfacePixelFormat  palette_intensity_format = DSPF_LUT4;
    u32 max_palette_intensity_cnt = MAX_PALETTE_INTENSITY_I4_CNT;
    StateModificationFlags   modified = state->mod_hw;


    state->set = DFXL_NONE;

    if(support_accel == DFXL_NONE)
    {
        printf("Serious warning, rejet mstarSetState\n");
        return;
    }

    if(DFB_BLITTING_FUNCTION(support_accel) && (sdrv->ge_last_render_op != MSTAR_GFX_RENDER_OP_BLIT))
    {
        modified |= (SMF_SOURCE | SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY | SMF_DST_COLORKEY | SMF_SRC_BLEND | SMF_DST_BLEND);
    }else
    if(DFB_DRAWING_FUNCTION(support_accel) && (sdrv->ge_last_render_op != MSTAR_GFX_RENDER_OP_DRAW))
    {
        modified |= (SMF_DRAWING_FLAGS | SMF_SRC_COLORKEY | SMF_DST_COLORKEY | SMF_SRC_BLEND | SMF_DST_BLEND);
    }
    if( (modified & SMF_CLIP) && sdev->b_hwclip )
    {
        sdrv->clip = state->clip;
    }

    if(modified & SMF_DESTINATION)
    {
        switch( state->dst.buffer->format ){
        case DSPF_ARGB1555:
            sdrv->dst_ge_format = GFX_FMT_ARGB1555 ;
            break ;
        case DSPF_ARGB:
            sdrv->dst_ge_format = GFX_FMT_ARGB8888 ;
            break ;
        case DSPF_YVYU:
            sdrv->dst_ge_yuv_fmt = GFX_YUV_YVYU;
            sdrv->dst_ge_format = GFX_FMT_YUV422;
            break;
        case DSPF_YUY2:
            sdrv->dst_ge_yuv_fmt = GFX_YUV_YUYV;
            sdrv->dst_ge_format = GFX_FMT_YUV422;
            break ;

        case DSPF_UYVY:
            sdrv->dst_ge_yuv_fmt = GFX_YUV_UYVY;
            sdrv->dst_ge_format = GFX_FMT_YUV422;
            break ;

        case DSPF_LUT8:
            sdrv->dst_ge_format = GFX_FMT_I8 ;
            palette = dsurface->palette;
            //printf( "I8 not support yet %s:%d\n",__FILE__,__LINE__ ) ;
            break ;
        case DSPF_ARGB4444:
            sdrv->dst_ge_format = GFX_FMT_ARGB4444;
            break;
        case DSPF_RGB16:
            sdrv->dst_ge_format = GFX_FMT_RGB565;
            break;
#if  MSTAR_DFB_A8_SUPPORT
        case DSPF_A8:
            sdrv->dst_ge_format = GFX_FMT_I8 ;
            break;
#endif
        default:
            //printf( "error format %s:%d, 0x%08X\n",__FILE__,__LINE__,state->dst.buffer->format ) ;
            dumpFormat(state->dst.buffer->format) ;
            return;
        }

        sdrv->dst_phys = mstarGFXAddr(state->dst.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset);
        sdrv->dst_w = dsurface->config.size.w;
        sdrv->dst_h = dsurface->config.size.h;
        sdrv->dst_bpp    = DFB_BYTES_PER_PIXEL( state->dst.buffer->format );
        sdrv->dst_pitch  = state->dst.pitch;
    }

    if(modified & SMF_COLOR)
    {
        sdrv->color = state->color ;
        sdrv->color2 = state->color2;

        if(sdrv->dst_ge_format == GFX_FMT_I8)
            sdrv->color.b = state->color_index;
    }

    if(modified & SMF_SRC_CONVOLUTION)
        sdrv->ge_blit_nearestmode_enabled = state->blit_nearestmode_enabled;
    if(modified & SMF_BLIT_XMIRROR)
        sdrv->ge_blit_xmirror = state->blit_xmirror_enabled;
    if(modified & SMF_BLIT_YMIRROR)
        sdrv->ge_blit_ymirror = state->blit_ymirror_enabled;

    if(modified & SMF_ALPHA_CMP)
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

    if(DFB_DRAWING_FUNCTION(support_accel))
    {
        if(modified & SMF_DRAWING_FLAGS)
            sdrv->dflags = state->drawingflags;

        if(modified & (SMF_DRAWING_FLAGS | SMF_DST_COLORKEY))
        {
            sdrv->ge_draw_dst_colorkey_enabled = ((state->drawingflags& DSDRAW_DST_COLORKEY)?1:0);
                sdrv->dst_ge_clr_key = mstar_pixel_to_colorkey(state->dst.buffer->format, state->dst_colorkey, palette) ;
        }

        if(modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
        {
            sdrv->ge_draw_alpha_blend_enabled = 0;
            sdrv->ge_draw_src_over_patch = 0;
            sdrv->ge_draw_render_coef = 0xffffffff;
            sdrv->ge_draw_render_alpha_const = sdrv->color.a;
            sdrv->ge_draw_render_alpha_from = (u32)ABL_FROM_ASRC;

            if(state->drawingflags & DSDRAW_SRC_PREMULTIPLY)
            {
                u16 ca = sdrv->color.a;
                sdrv->color.r = (u8)((sdrv->color.r*ca)>>8);
                sdrv->color.g = (u8)((sdrv->color.r*ca)>>8);
                sdrv->color.b = (u8)((sdrv->color.r*ca)>>8);
                ca = sdrv->color2.a;
                sdrv->color2.r = (u8)((sdrv->color2.r*ca)>>8);
                sdrv->color2.g = (u8)((sdrv->color2.r*ca)>>8);
                sdrv->color2.b = (u8)((sdrv->color2.r*ca)>>8);
            }

            mstarCheckDrawOption(state, &sdrv->ge_draw_render_coef, &sdrv->ge_draw_render_alpha_from, &sdrv->ge_draw_render_alpha_const);

            if(sdrv->ge_draw_render_coef == 0xffffffff)
            {
                sdrv->ge_draw_render_alpha_from = (u32)ABL_FROM_ASRC;
            }
            else
            if(sdrv->ge_draw_render_coef == 0x80000000)
            {
                sdrv->ge_draw_src_over_patch = 1;
                sdrv->ge_draw_alpha_blend_enabled = 1;
                sdrv->ge_draw_render_coef = (u32)COEF_ONE;
                sdrv->ge_draw_render_alpha_from = (u32)ABL_FROM_ASRC;
            }
            else
            {
                if(sdrv->ge_draw_render_coef!=(u32)COEF_ONE || sdrv->ge_draw_render_alpha_from!=(u32)ABL_FROM_ASRC)
                    sdrv->ge_draw_alpha_blend_enabled = 1;
                else
                    sdrv->ge_draw_alpha_blend_enabled = 0;
            }
        }
    }

    if(DFB_BLITTING_FUNCTION(support_accel))
    {
        CoreSurface *ssurface = state->source;

        if(modified & SMF_BLITTING_FLAGS)
            sdrv->bflags = state->blittingflags;

        if(modified & (SMF_BLITTING_FLAGS | SMF_DST_COLORKEY))
        {
            sdrv->ge_blit_dst_colorkey_enabled = ((state->blittingflags& DSBLIT_DST_COLORKEY)?1:0);
                sdrv->dst_ge_clr_key = mstar_pixel_to_colorkey(state->dst.buffer->format, state->dst_colorkey, palette) ;
        }

        if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND))
        {
            sdrv->ge_blit_alpha_blend_enabled = 0;
            sdrv->ge_blit_src_over_patch = 0;
            sdrv->ge_blit_render_coef = 0xffffffff;
            sdrv->ge_blit_render_alpha_const = sdrv->color.a;
            sdrv->ge_blit_render_alpha_from = (u32)ABL_FROM_ASRC;

            mstarCheckBlitOption(state, &sdrv->ge_blit_render_coef, &sdrv->ge_blit_render_alpha_from, &sdrv->ge_blit_render_alpha_const);

            if(sdrv->ge_blit_render_coef == 0xffffffff)
            {
                sdrv->ge_blit_alpha_blend_enabled = 0;
                sdrv->ge_blit_render_alpha_from = (u32)ABL_FROM_ASRC;
            }
            else
            if(sdrv->ge_blit_render_coef == 0x80000000)
            {
                sdrv->ge_blit_src_over_patch = 1;
                sdrv->ge_blit_alpha_blend_enabled = 1;
                sdrv->ge_blit_render_coef = (u32)COEF_ONE;
                sdrv->ge_blit_render_alpha_from = (u32)ABL_FROM_ASRC;
            }
            else
            {
                if(sdrv->ge_blit_render_coef!=(u32)COEF_ONE || sdrv->ge_blit_render_alpha_from!=(u32)ABL_FROM_ASRC)
                    sdrv->ge_blit_alpha_blend_enabled = 1;
                else
                    sdrv->ge_blit_alpha_blend_enabled = 0;
            }
        }

    #if MSTAR_DFB_A8_SUPPORT
        if(DSPF_A8 == state->src.buffer->format)
        {

            if( NULL != dsurface->palette )
            {
                printf( "error format %s:%d, 0x%08X\n",__FILE__,__LINE__,state->src.buffer->format ) ;
                return ;
            }


            if( (sdev->a8_palette_color.r != sdrv->color.r)
                 || (sdev->a8_palette_color.g != sdrv->color.g)
                 || (sdev->a8_palette_color.b != sdrv->color.b)
                 || (!sdev->a8_palette) )

            {
                sdev->palette_tbl[0].RGB.u8A = 0;
                if (state->blittingflags & DSBLIT_COLORIZE)
                {
                    sdev->palette_tbl[0].RGB.u8R = sdrv->color.r;
                    sdev->palette_tbl[0].RGB.u8G = sdrv->color.g;
                    sdev->palette_tbl[0].RGB.u8B = sdrv->color.b;
                }
                else
                {
                    sdev->palette_tbl[0].RGB.u8R = 0xFF;
                    sdev->palette_tbl[0].RGB.u8G = 0xFF;
                    sdev->palette_tbl[0].RGB.u8B = 0xFF;
                }

                for(i=1; i < MAX_PALETTE_ENTRY_CNT; ++i)
                {
                    sdev->palette_tbl[i] = sdev->palette_tbl[0];
                    sdev->palette_tbl[i].RGB.u8A = i;
                }

                sdev->a8_palette_color = sdrv->color;
                sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
                sdrv->ge_palette_enabled = 1;
                sdev->a8_palette = true;
            }
        }
    #endif

        if(modified & SMF_SOURCE)
        {
            sdrv->src_w = ssurface->config.size.w ;
            sdrv->src_h = ssurface->config.size.h ;

            sdrv->src_phys = mstarGFXAddr(state->src.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset);

            switch( state->src.buffer->format )
            {
                case DSPF_ARGB1555:
                    sdrv->src_ge_format = GFX_FMT_ARGB1555 ;
                    break ;
                case DSPF_ARGB:
                    sdrv->src_ge_format = GFX_FMT_ARGB8888 ;
                    break ;
                case DSPF_YVYU:
                    sdrv->src_ge_yuv_fmt = GFX_YUV_YVYU;
                    sdrv->src_ge_format =  GFX_FMT_YUV422 ;
                    break ;
                case DSPF_YUY2:
                    sdrv->src_ge_yuv_fmt = GFX_YUV_YUYV;
                    sdrv->src_ge_format =  GFX_FMT_YUV422;
                    break ;

                case DSPF_UYVY:
                    sdrv->src_ge_yuv_fmt = GFX_YUV_UYVY;
                    sdrv->src_ge_format = GFX_FMT_YUV422;
                    break ;
                case DSPF_LUT8:
                    sdrv->src_ge_format = GFX_FMT_I8 ;
                    if( NULL == palette )
                    {
                        palette = ssurface->palette;
                    }
                    break ;
                case DSPF_LUT4:
                    palette_intensity_format = DSPF_LUT4;
                    max_palette_intensity_cnt = MAX_PALETTE_INTENSITY_I4_CNT;
                    sdrv->src_ge_format = GFX_FMT_I4 ;
                    palette_intensity = ssurface->palette;
                    break ;
                case DSPF_LUT2:
                    palette_intensity_format = DSPF_LUT2;
                    max_palette_intensity_cnt = MAX_PALETTE_INTENSITY_I2_CNT;
                    sdrv->src_ge_format = GFX_FMT_I2 ;
                    palette_intensity = ssurface->palette;
                    break ;
                case DSPF_LUT1:
                    palette_intensity_format = DSPF_LUT1;
                    max_palette_intensity_cnt = MAX_PALETTE_INTENSITY_I1_CNT;
                    sdrv->src_ge_format = GFX_FMT_I1 ;
                    palette_intensity = ssurface->palette;
                    break ;
                 case DSPF_ARGB4444:
                    sdrv->src_ge_format = GFX_FMT_ARGB4444 ;
                    break ;
                 case DSPF_RGB16:
                    sdrv->src_ge_format = GFX_FMT_RGB565 ;
                    break ;
#if MSTAR_DFB_A8_SUPPORT
                case DSPF_A8:
                    sdrv->src_ge_format = GFX_FMT_I8 ;
                    break ;
#endif
                default:
                    printf( "error format %s:%d, 0x%08X\n",__FILE__,__LINE__,state->src.buffer->format ) ;

                    dumpFormat(state->src.buffer->format) ;
                    return ;
            }

            sdrv->src_bpp    = DFB_BYTES_PER_PIXEL( state->src.buffer->format );
            sdrv->src_pitch  = state->src.pitch;
        }

        if(modified & (SMF_BLITTING_FLAGS | SMF_SRC_COLORKEY))
        {
            sdrv->ge_src_colorkey_enabled = ((state->blittingflags& DSBLIT_SRC_COLORKEY)?1:0);
            sdrv->src_ge_clr_key = mstar_pixel_to_colorkey(state->src.buffer->format, state->src_colorkey, ssurface->palette) ;
        }
    }

    if( palette )
    {
        int i ;
        int num_entries ;
        num_entries =( palette->num_entries > MAX_PALETTE_ENTRY_CNT ? MAX_PALETTE_ENTRY_CNT : palette->num_entries) ;

        for( i=0; i < num_entries; ++i )
        {
            sdev->palette_tbl[i].RGB.u8A = palette->entries[i].a ;
            sdev->palette_tbl[i].RGB.u8R = palette->entries[i].r ;
            sdev->palette_tbl[i].RGB.u8G = palette->entries[i].g ;
            sdev->palette_tbl[i].RGB.u8B = palette->entries[i].b ;
        }

        sdev->num_entries =  num_entries ;
        sdrv->ge_palette_enabled = 1 ;
        sdev->a8_palette = false ;
    }

    if( palette_intensity )
    {
        int i ;
        int num_entries ;

        num_entries =( palette_intensity->num_entries > max_palette_intensity_cnt ? max_palette_intensity_cnt : palette_intensity->num_entries) ;

        for( i=0; i < num_entries; ++i )
        {
            sdev->palette_intensity[i] = mstar_pixel_to_color(palette_intensity_format, i, palette_intensity) ;
        }

        sdev->num_intensity =  num_entries ;
        sdrv->ge_palette_intensity_enabled = 1 ;
    }

    state->mod_hw = 0;
    state->set = support_accel & ((DFXL_FILLRECTANGLE|DFXL_DRAWLINE|DFXL_DRAWRECTANGLE|DFXL_FILLTRAPEZOID|DFXL_FILLQUADRANGLE)
                                  |(DFXL_BLIT|DFXL_STRETCHBLIT|DFXL_BLIT2));

    MApi_GFX_BeginDraw();
    mstarSetEngineShareState(modified, support_accel, sdrv, sdev);

    if(DFB_BLITTING_FUNCTION(support_accel))
    {
        mstarSetEngineBlitState(modified, sdrv, sdev);
        sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_BLIT;
    }else
    if(DFB_DRAWING_FUNCTION(support_accel))
    {
        mstarSetEngineDrawState(modified, sdrv, sdev);
        sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_DRAW;
    }

    MApi_GFX_EndDraw();

    sdrv->state = state;
}

bool _mstarSWProcessSrvOver(
        void *driver_data,
        void *device_data,
        MS_U16 u16X, MS_U16 u16Y, MS_U16 u16Width, MS_U16 u16Height )
{
//    MSTARDeviceData *sdev = device_data;
    MSTARDriverData *sdrv = driver_data;
    CoreSurface *dsurface = sdrv->state->destination ;
    CoreSurfaceBufferLock ret_lock;
    DFBResult ret;
    MS_U32 u32IdxX, u32IdxY;
    MS_U8 *pu8Addr;
    MS_U32 u32Val;

    ret = dfb_surface_lock_buffer(dsurface, CSBR_BACK, CSAID_CPU, CSAF_WRITE, &ret_lock);
    if(DFB_OK != ret)
        return false;

    pu8Addr = (MS_U8*)ret_lock.addr;
    pu8Addr += u16Y * ret_lock.pitch;
    pu8Addr += u16X*4;

    {
        for(u32IdxY=0; u32IdxY<u16Height; u32IdxY++)
        {
            for(u32IdxX=0; u32IdxX<u16Width; u32IdxX++)
            {
                u32Val = (*(((MS_U32*)(void *)pu8Addr)+u32IdxX));
                if(u32Val & 0x80000000)
                    u32Val |= 0xFF000000;
                if(u32Val & 0x00800000)
                    u32Val |= 0x00FF0000;
                if(u32Val & 0x00008000)
                    u32Val |= 0x0000FF00;
                if(u32Val & 0x00000080)
                    u32Val |= 0x000000FF;
                *(((MS_U32*)(void *)pu8Addr)+u32IdxX) = u32Val << 1;
            }

            pu8Addr += ret_lock.pitch;
        }
    }

    return true;
}

bool _mstarFillSrcOver(
        void *driver_data,
        void *device_data,
        GFX_RectFillInfo *rectInfo )
{
    //MSTARDriverData *sdrv = driver_data;
    GFX_RectFillInfo fillInfo = *rectInfo;
    MS_U16 u16TagID;

    fillInfo.colorRange.color_s.r = fillInfo.colorRange.color_s.g = fillInfo.colorRange.color_s.b = 0x0;
    // 1.  0x0 * Asrc + Cdst * (1 - Asrc), Asrc * 0xFF - Adst * Asrc * 0xFF + Adst
    MApi_GFX_BeginDraw();
    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_ASRC, 0xFF);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ROP8_OVER);

    MApi_GFX_RectFill( &fillInfo ) ;
    MApi_GFX_EndDraw();

    // 2. Csrc * Aconst + Cdst * (1 - Aconst), Asrc*Aconst * Adst, Aconst = 0x80; Asrc = 0xFF;
    fillInfo.colorRange = rectInfo->colorRange;
    fillInfo.colorRange.color_s.a = 0xFF;
    MApi_GFX_BeginDraw();
    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_CONST, 0x80);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ROP8_IN);

    MApi_GFX_RectFill( &fillInfo ) ;

    u16TagID = MApi_GFX_SetNextTAGID();
    MApi_GFX_EndDraw();
    MApi_GFX_WaitForTAGID(u16TagID);

    // 3. SW Process:
    return _mstarSWProcessSrvOver(driver_data, device_data, rectInfo->dstBlock.x, rectInfo->dstBlock.y, rectInfo->dstBlock.width, rectInfo->dstBlock.height);
}

bool mstarFillRectangle( void *drv, void *dev, DFBRectangle *rect ){
    MSTARDriverData *sdrv = drv ;
    GFX_RectFillInfo rectInfo ;
    u32 y, cb, cr ;

    rectInfo.dstBlock.x = rect->x ;
    rectInfo.dstBlock.y = rect->y ;
    rectInfo.dstBlock.width = rect->w ;
    rectInfo.dstBlock.height = rect->h ;
    rectInfo.fmt = sdrv->dst_ge_format ;

    switch(rectInfo.fmt)
    {
        case GFX_FMT_YUV422:
            RGB_TO_YCBCR( sdrv->color.r, sdrv->color.g, sdrv->color.b, y, cb, cr ) ;
            rectInfo.colorRange.color_s.a = 0xFF ;
            rectInfo.colorRange.color_s.r = cr ;
            rectInfo.colorRange.color_s.g = y ;
            rectInfo.colorRange.color_s.b = cb ;
            break;
        default:
            rectInfo.colorRange.color_s.a = sdrv->color.a;
            rectInfo.colorRange.color_s.r = sdrv->color.r;
            rectInfo.colorRange.color_s.g = sdrv->color.g;
            rectInfo.colorRange.color_s.b = sdrv->color.b;
            break;
    }
   if(sdrv->dflags &(DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
   {
      switch(rectInfo.fmt)
      {
            case GFX_FMT_YUV422:
                   RGB_TO_YCBCR( sdrv->color2.r, sdrv->color2.g, sdrv->color2.b, y, cb, cr ) ;
                   rectInfo.colorRange.color_e.a = 0xFF ;
                   rectInfo.colorRange.color_e.r = cr ;
                   rectInfo.colorRange.color_e.g = y ;
                   rectInfo.colorRange.color_e.b = cb ;
                   break;
            default:
                   rectInfo.colorRange.color_e.a = sdrv->color2.a;
                   rectInfo.colorRange.color_e.r = sdrv->color2.r;
                   rectInfo.colorRange.color_e.g = sdrv->color2.g;
                   rectInfo.colorRange.color_e.b = sdrv->color2.b;
                   break;
          }
    if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_X)
            rectInfo.flag = GFXRECT_FLAG_COLOR_GRADIENT_X;
    else
         rectInfo.flag = 0;
    if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            rectInfo.flag |= GFXRECT_FLAG_COLOR_GRADIENT_Y;
   }
   else
       rectInfo.flag = 0;

    if(sdrv->ge_draw_src_over_patch)
    {
        _mstarFillSrcOver(drv, dev, &rectInfo);
    }else
    {
        MApi_GFX_BeginDraw();
        MApi_GFX_RectFill( &rectInfo ) ;
        MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
        MApi_GFX_EndDraw();
    }

    return true ;
}

bool mstarDrawRectangle(
        void *driver_data,
        void *device_data,
        DFBRectangle *rect ){
    MSTARDriverData *sdrv = driver_data ;

    GFX_DrawLineInfo l ;

    l.fmt = sdrv->dst_ge_format ;
    l.colorRange.color_s.a = sdrv->color.a ;
    l.colorRange.color_s.r = sdrv->color.r ;
    l.colorRange.color_s.g = sdrv->color.g ;
    l.colorRange.color_s.b = sdrv->color.b ;
    l.colorRange.color_e = l.colorRange.color_s ;
    l.width = 1 ;
    l.flag = GFXLINE_FLAG_COLOR_CONSTANT ;

    l.x1 = rect->x ;
    l.y1 = rect->y ;
    l.x2 = rect->x+rect->w-1 ;
    l.y2 = rect->y ;
    MApi_GFX_BeginDraw();
    MApi_GFX_DrawLine( &l ) ;

    l.x2 = rect->x ;
    l.y2 = rect->y+rect->h-1 ;
    MApi_GFX_DrawLine( &l ) ;

    l.y1 = rect->y+rect->h-1 ;
    l.x2 = rect->x+rect->w-1 ;
    MApi_GFX_DrawLine( &l ) ;

    l.x1 = rect->x+rect->w-1 ;
    l.y1 = rect->y ;
    MApi_GFX_DrawLine( &l ) ;
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true ;
}

bool mstarDrawOval(
        void *driver_data,
        void *device_data,
        DFBOval *oval ){
    MSTARDriverData *sdrv = driver_data ;
    GFX_OvalFillInfo ovalInfo ;

    ovalInfo.dstBlock.x = oval->oval_rect.x ;
    ovalInfo.dstBlock.y = oval->oval_rect.y ;
    ovalInfo.dstBlock.width = oval->oval_rect.w ;
    ovalInfo.dstBlock.height = oval->oval_rect.h ;

    ovalInfo.fmt = sdrv->dst_ge_format ;
    ovalInfo.u32LineWidth = (u32)oval->line_width ;

    MApi_GFX_BeginDraw() ;

    if(oval->line_width > 0)
    {
        if(!DFB_COLOR_EQUAL(sdrv->color, oval->line_color))
        {
            ovalInfo.u32LineWidth = 0 ;
            ovalInfo.color.a = oval->line_color.a ;
            ovalInfo.color.r = oval->line_color.r ;
            ovalInfo.color.g = oval->line_color.g ;
            ovalInfo.color.b = oval->line_color.b ;
            MApi_GFX_DrawOval( &ovalInfo ) ;
            ovalInfo.u32LineWidth = (u32)oval->line_width ;
        }
        else
        {
            ovalInfo.u32LineWidth = 0 ;
        }
    }

    ovalInfo.color.a = sdrv->color.a ;
    ovalInfo.color.r = sdrv->color.r ;
    ovalInfo.color.g = sdrv->color.g ;
    ovalInfo.color.b = sdrv->color.b ;
    MApi_GFX_DrawOval( &ovalInfo ) ;

    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true ;
}


bool mstarDrawLine(
        void *driver_data,
        void *device_data,
        DFBRegion *line ){
    MSTARDriverData *sdrv = driver_data ;

    GFX_DrawLineInfo l ;

    l.x1 = line->x1 ;
    l.y1 = line->y1 ;
    l.x2 = line->x2 ;
    l.y2 = line->y2 ;
    l.fmt = sdrv->dst_ge_format ;
    l.colorRange.color_s.a = sdrv->color.a ;
    l.colorRange.color_s.r = sdrv->color.r ;
    l.colorRange.color_s.g = sdrv->color.g ;
    l.colorRange.color_s.b = sdrv->color.b ;
    l.colorRange.color_e = l.colorRange.color_s ;
    l.width = 1 ;
    l.flag = GFXLINE_FLAG_COLOR_CONSTANT ;

    MApi_GFX_BeginDraw();
    MApi_GFX_DrawLine( &l ) ;
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true ;
}

bool _mstarBlitSrcOver(
        void *driver_data,
        void *device_data,
        GFX_DrawRect *rectInfo,
        u32 u32BlitFlag )
{
    //resultC=srcC*colorA+dstC*(1-srcA*colorA); resultA=srcA*colorA+dstA*(1-srcA*colorA);
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    CoreSurface tmp_surface;
    CoreSurface *ptmp_surface = &tmp_surface;
    CoreSurfaceConfig conf;
    DFBResult ret;
    GFX_BufferInfo buf;
    CoreSurfaceBufferLock ret_lock;
    GFX_Point v0, v1;
    GFX_DrawRect rect_info;
    MS_PHYADDR u32TmpBufPhyAddr;
    MS_U16 u16TagID;
    GFX_RectFillInfo fillInfo;

    // 0. Create Tmp buffer which the same as Src:
    conf.flags  = CSCONF_SIZE | CSCONF_FORMAT | CSCONF_CAPS;
    conf.caps   = DSCAPS_VIDEOONLY;
    conf.size.w = sdrv->src_w;
    conf.size.h = sdrv->src_h;
    switch(sdrv->src_ge_format)
    {
        case GFX_FMT_ARGB1555:
            conf.format = DSPF_ARGB1555;
            break;
        case GFX_FMT_YUV422:
            conf.format = DSPF_YUY2;
            break;
        case GFX_FMT_ARGB4444:
            conf.format = DSPF_ARGB4444;
            break;
        case GFX_FMT_RGB565:
            conf.format = DSPF_RGB16;
            break;
        default:
        case GFX_FMT_ARGB8888:
            conf.format = DSPF_ARGB;
            break;
    }
    ret = dfb_surface_create(sdrv->core, &conf, CSTF_EXTERNAL, (unsigned long )0, NULL, &ptmp_surface);
    if(DFB_OK != ret)
        return false;

    ret = dfb_surface_lock_buffer(ptmp_surface, CSBR_FRONT, CSAID_GPU, CSAF_ALL, &ret_lock);
    if(DFB_OK != ret)
        return false;

    u32TmpBufPhyAddr = ret_lock.phys;
    dfb_surface_unlock_buffer(ptmp_surface, &ret_lock);

    // 1. Reset HW State:
    MApi_GFX_BeginDraw();
    MApi_GFX_SetSrcColorKey(FALSE,
            CK_OP_EQUAL, adjustcolorformat(sdrv), &sdrv->src_ge_clr_key, &sdrv->src_ge_clr_key);
    MApi_GFX_SetDstColorKey(FALSE,
         CK_OP_NOT_EQUAL, (GFX_Buffer_Format)sdrv->dst_ge_format, &sdrv->dst_ge_clr_key, &sdrv->dst_ge_clr_key);
    MApi_GFX_EnableAlphaBlending(FALSE);
    MApi_GFX_SetAlphaCmp(FALSE, sdrv->ge_render_alpha_cmp );

    // 2. Fill Tmp to color 0:
    buf.u32Addr = mstarGFXAddr(u32TmpBufPhyAddr, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset);
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetDstBufferInfo(&buf, 0);
    //Set Dst clip window, All ops Needed:
    v0.x = 0;
    v0.y = 0;
    v1.x = buf.u32Width-1;
    v1.y = buf.u32Height-1;
    MApi_GFX_SetClip(&v0, &v1);

    fillInfo.dstBlock.x = 0 ;
    fillInfo.dstBlock.y = 0 ;
    fillInfo.dstBlock.width = sdrv->src_w ;
    fillInfo.dstBlock.height = sdrv->src_h ;
    fillInfo.fmt = sdrv->src_ge_format ;
    fillInfo.colorRange.color_s.a = 0x0;
    fillInfo.colorRange.color_s.r = 0x0;
    fillInfo.colorRange.color_s.g = 0x0;
    fillInfo.colorRange.color_s.b = 0x0;
    fillInfo.flag = 0 ;

    MApi_GFX_RectFill( &fillInfo ) ;
    MApi_GFX_EndDraw();

    // 3.Src->Tmp: COEF_ZERO, ABL_FROM_ROP8_SRC, constAlpha=colorA; Tmp[0, srcA*colorA]
    MApi_GFX_BeginDraw();
    buf.u32Addr = (MS_U32)sdrv->src_phys;
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetSrcBufferInfo(&buf, 0);

    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_ZERO, sdrv->color.a);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ROP8_SRC);

    rect_info.srcblk = rect_info.dstblk = rectInfo->srcblk;
    SET_BLT_MIRROR_SRCX(sdrv, rect_info);
    SET_BLT_MIRROR_SRCY(sdrv, rect_info);
    MApi_GFX_BitBlt(&rect_info, GFXDRAW_FLAG_DEFAULT);
    MApi_GFX_EndDraw();

    // 4.Tmp->Dst: COEF_ASRC, ABL_FROM_ROP8_OVER, constAlpha=0xFF;DST[dstC*(1-srcA*colorA), srcA*colorA+dstA*(1-srcA*colorA)]
    MApi_GFX_BeginDraw();
    buf.u32Addr = u32TmpBufPhyAddr;
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetSrcBufferInfo(&buf, 0);

    buf.u32Addr = (MS_U32)sdrv->dst_phys;
    buf.u32ColorFmt = sdrv->dst_ge_format;
    buf.u32Width = sdrv->dst_w;
    buf.u32Height = sdrv->dst_h;
    buf.u32Pitch = sdrv->dst_pitch;
    MApi_GFX_SetDstBufferInfo(&buf, 0);

    v0.x = 0;
    v0.y = 0;
    v1.x = buf.u32Width-1;
    v1.y = buf.u32Height-1;
    MApi_GFX_SetClip(&v0, &v1);

    MApi_GFX_SetSrcColorKey(sdrv->ge_src_colorkey_enabled,
            CK_OP_EQUAL, adjustcolorformat(sdrv), &sdrv->src_ge_clr_key, &sdrv->src_ge_clr_key);
    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_ASRC, 0xFF);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ROP8_OVER);

    rect_info.srcblk = rectInfo->srcblk;
    rect_info.dstblk = rectInfo->dstblk;
    SET_BLT_MIRROR_SRCX(sdrv, rect_info);
    SET_BLT_MIRROR_SRCY(sdrv, rect_info);
    MApi_GFX_BitBlt(&rect_info, u32BlitFlag);
    MApi_GFX_EndDraw();

    // 5.Src->Tmp: COEF_CONST_SRC, ABL_FROM_ADST, constAlpha=colorA;TMP[srcC*colorA, srcA*colorA]
    MApi_GFX_BeginDraw();
    buf.u32Addr = (MS_U32)sdrv->src_phys;
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetSrcBufferInfo(&buf, 0);

    buf.u32Addr = u32TmpBufPhyAddr;
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetDstBufferInfo(&buf, 0);
    //Set Dst clip window, All ops Needed:
    v0.x = 0;
    v0.y = 0;
    v1.x = buf.u32Width-1;
    v1.y = buf.u32Height-1;
    MApi_GFX_SetClip(&v0, &v1);

    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_CONST_SRC, sdrv->color.a);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ADST);

    rect_info.srcblk = rect_info.dstblk = rectInfo->srcblk;
    SET_BLT_MIRROR_SRCX(sdrv, rect_info);
    SET_BLT_MIRROR_SRCY(sdrv, rect_info);
    MApi_GFX_BitBlt(&rect_info, GFXDRAW_FLAG_DEFAULT);
    MApi_GFX_EndDraw();

    // 6. Src->Tmp: COEF_ZERO, ABL_FROM_CONST, constAlpha=0xFF;
    MApi_GFX_BeginDraw();
    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_ZERO, 0xFF);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_CONST);

    rect_info.srcblk = rect_info.dstblk = rectInfo->srcblk;
    SET_BLT_MIRROR_SRCX(sdrv, rect_info);
    SET_BLT_MIRROR_SRCY(sdrv, rect_info);
    MApi_GFX_BitBlt(&rect_info, GFXDRAW_FLAG_DEFAULT);
    MApi_GFX_EndDraw();

    // 7. Tmp->Dst: COEF_CONST, ABL_FROM_ROP8_IN,constAlpha=0x80
    MApi_GFX_BeginDraw();
    buf.u32Addr = u32TmpBufPhyAddr;
    buf.u32ColorFmt = sdrv->src_ge_format;
    buf.u32Width = sdrv->src_w;
    buf.u32Height = sdrv->src_h;
    buf.u32Pitch = sdrv->src_pitch;
    MApi_GFX_SetSrcBufferInfo(&buf, 0);

    buf.u32Addr = (MS_U32)sdrv->dst_phys;
    buf.u32ColorFmt = sdrv->dst_ge_format;
    buf.u32Width = sdrv->dst_w;
    buf.u32Height = sdrv->dst_h;
    buf.u32Pitch = sdrv->dst_pitch;
    MApi_GFX_SetDstBufferInfo(&buf, 0);

    v0.x = 0;
    v0.y = 0;
    v1.x = buf.u32Width-1;
    v1.y = buf.u32Height-1;
    MApi_GFX_SetClip(&v0, &v1);

    MApi_GFX_EnableAlphaBlending(TRUE);
    MApi_GFX_SetAlphaBlending(COEF_CONST, 0x80);
    MApi_GFX_SetAlphaSrcFrom(ABL_FROM_ROP8_IN);

    rect_info.srcblk = rectInfo->srcblk;
    rect_info.dstblk = rectInfo->dstblk;
    SET_BLT_MIRROR_SRCX(sdrv, rect_info);
    SET_BLT_MIRROR_SRCY(sdrv, rect_info);
    MApi_GFX_BitBlt(&rect_info, u32BlitFlag);
    u16TagID = MApi_GFX_SetNextTAGID();
    MApi_GFX_EndDraw();
    MApi_GFX_WaitForTAGID(u16TagID);

    // 7. Release Tmp Buffer
    dfb_surface_destroy_buffers(ptmp_surface);

    // 8. Software handling shift:
    return _mstarSWProcessSrvOver(driver_data, device_data, rectInfo->dstblk.x, rectInfo->dstblk.y, rectInfo->dstblk.width, rectInfo->dstblk.height);
}

bool mstarBlit(
        void *driver_data,
        void *device_data,
        DFBRectangle *rect,
        int dx, int dy )
{
    MSTARDriverData *sdrv = driver_data ;

    GFX_DrawRect rectInfo;

    rectInfo.srcblk.x = rect->x;
    rectInfo.srcblk.y = rect->y;
    rectInfo.dstblk.x = dx;
    rectInfo.dstblk.y = dy;
    rectInfo.srcblk.width = rectInfo.dstblk.width = rect->w;
    rectInfo.srcblk.height = rectInfo.dstblk.height = rect->h;

    if(sdrv->ge_blit_src_over_patch)
        _mstarBlitSrcOver(driver_data, device_data, &rectInfo, GFXDRAW_FLAG_DEFAULT);
    else
    {
        MApi_GFX_BeginDraw();
        SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
        SET_BLT_MIRROR_SRCY(sdrv, rectInfo);
        MApi_GFX_BitBlt(&rectInfo, GFXDRAW_FLAG_DEFAULT);
        MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
        MApi_GFX_EndDraw();
    }

    return true ;
}

bool mstarStretchBlit(
        void *driver_data,
        void *device_data,
        DFBRectangle *srect,
        DFBRectangle *drect ){
    MSTARDriverData *sdrv = driver_data;
    GFX_DrawRect rectInfo;
    GFX_Result ret;

    rectInfo.srcblk.x = srect->x, rectInfo.srcblk.y = srect->y;
    rectInfo.srcblk.width = srect->w, rectInfo.srcblk.height = srect->h;
    rectInfo.dstblk.x = drect->x, rectInfo.dstblk.y = drect->y ;
    rectInfo.dstblk.width = drect->w, rectInfo.dstblk.height = drect->h;

    if(sdrv->ge_blit_src_over_patch)
    {
        return _mstarBlitSrcOver(driver_data, device_data, &rectInfo, GFXDRAW_FLAG_SCALE);
    }
    else
    {
        MApi_GFX_BeginDraw();
        SET_BLT_MIRROR_SRCX(sdrv, rectInfo);
        SET_BLT_MIRROR_SRCY(sdrv, rectInfo);
        ret = MApi_GFX_BitBlt(&rectInfo, GFXDRAW_FLAG_SCALE);
        if(GFX_SUCCESS != ret)
        {
          D_INFO("GE downscaling blit ratio can not be supported by HW(32:1)!, src height %d, dst height%d\n",
            rectInfo.srcblk.height, rectInfo.dstblk.height);
        }else
            MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
        MApi_GFX_EndDraw();

        return (GFX_SUCCESS==ret) ;
    }
}


bool mstarStretchBlitEx(
        void *driver_data,
        void *device_data,
        DFBRectangle *srect,
        DFBRectangle *drect,
        const DFBGFX_ScaleInfo *scale_info)
{
    MSTARDriverData *sdrv = driver_data;
    GFX_DrawRect rectInfo;
    GFX_Result ret;
    GFX_ScaleInfo ScaleInfo;


    rectInfo.srcblk.x = srect->x, rectInfo.srcblk.y = srect->y;
    rectInfo.srcblk.width = srect->w, rectInfo.srcblk.height = srect->h;
    rectInfo.dstblk.x = drect->x, rectInfo.dstblk.y = drect->y ;
    rectInfo.dstblk.width = drect->w, rectInfo.dstblk.height = drect->h;
    ScaleInfo.u32DeltaX     = scale_info->u32DeltaX;
    ScaleInfo.u32DeltaY     = scale_info->u32DeltaY;
    ScaleInfo.u32InitDelatX = scale_info->u32InitDelatX;
    ScaleInfo.u32InitDelatY = scale_info->u32InitDelatY;

    if(sdrv->ge_blit_src_over_patch)
    {
        return _mstarBlitSrcOver(driver_data, device_data, &rectInfo, GFXDRAW_FLAG_SCALE);
    }
    else
    {
        MApi_GFX_BeginDraw();
        ret = MApi_GFX_BitBltEx(&rectInfo, (MS_U32)(scale_info->u32GFXDrawFlag), &ScaleInfo);
        if(GFX_SUCCESS != ret)
        {
          D_INFO("GE downscaling blit ratio can not be supported by HW(32:1)!, src height %d, dst height%d\n",
            rectInfo.srcblk.height, rectInfo.dstblk.height);
        }else
            MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
        MApi_GFX_EndDraw();

        return (GFX_SUCCESS==ret) ;
    }

}

bool mstarBlitTrapezoid(
        void *driver_data,
        void *device_data,
        DFBRectangle *srect,
        DFBTrapezoid *dtrapezoid ){
    //MSTARDriverData *sdrv = driver_data;

    GFX_DrawRect rectInfo;
    GFX_Result ret;

    rectInfo.srcblk.x = srect->x, rectInfo.srcblk.y = srect->y;
    rectInfo.srcblk.width = srect->w, rectInfo.srcblk.height = srect->h;
    rectInfo.dsttrapeblk.u16X0= dtrapezoid->x0;
    rectInfo.dsttrapeblk.u16Y0= dtrapezoid->y0;
    rectInfo.dsttrapeblk.u16X1= dtrapezoid->x1;
    rectInfo.dsttrapeblk.u16Y1= dtrapezoid->y1;
    rectInfo.dsttrapeblk.u16DeltaTop = dtrapezoid->top_edge_width;
    rectInfo.dsttrapeblk.u16DeltaBottom= dtrapezoid->bottom_edge_width;

    MApi_GFX_BeginDraw();

    if(false == dtrapezoid->horiz_direct)
        ret = MApi_GFX_BitBlt(&rectInfo, GFXRECT_FLAG_TRAPE_DIRECTION_Y|GFXDRAW_FLAG_SCALE );
    else
        ret = MApi_GFX_BitBlt(&rectInfo, GFXRECT_FLAG_TRAPE_DIRECTION_X|GFXDRAW_FLAG_SCALE );

    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();
    return true ;
}

bool mstarFillTrapezoid(
        void *driver_data,
        void *device_data,
        DFBTrapezoid *dtrapezoid ){
    MSTARDriverData *sdrv = driver_data ;
    GFX_RectFillInfo rectInfo ;
    u32 y, cb, cr ;

    rectInfo.dstTrapezoidBlk.u16X0= dtrapezoid->x0;
    rectInfo.dstTrapezoidBlk.u16Y0= dtrapezoid->y0;
    rectInfo.dstTrapezoidBlk.u16X1= dtrapezoid->x1;
    rectInfo.dstTrapezoidBlk.u16Y1= dtrapezoid->y1;
    rectInfo.dstTrapezoidBlk.u16DeltaTop = dtrapezoid->top_edge_width;
    rectInfo.dstTrapezoidBlk.u16DeltaBottom = dtrapezoid->bottom_edge_width;

    rectInfo.fmt = sdrv->dst_ge_format ;
   if(sdrv->dflags &(DSDRAW_COLOR_GRADIENT_X|DSDRAW_COLOR_GRADIENT_Y))
   {
      switch(rectInfo.fmt)
      {
            case GFX_FMT_YUV422:
                   RGB_TO_YCBCR( sdrv->color2.r, sdrv->color2.g, sdrv->color2.b, y, cb, cr ) ;
                   rectInfo.colorRange.color_e.a = 0xFF ;
                   rectInfo.colorRange.color_e.r = cr ;
                   rectInfo.colorRange.color_e.g = y ;
                   rectInfo.colorRange.color_e.b = cb ;
                   break;
            default:
                   rectInfo.colorRange.color_e.a = sdrv->color2.a;
                   rectInfo.colorRange.color_e.r = sdrv->color2.r;
                   rectInfo.colorRange.color_e.g = sdrv->color2.g;
                   rectInfo.colorRange.color_e.b = sdrv->color2.b;
                   break;
          }
    if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_X)
            rectInfo.flag = GFXRECT_FLAG_COLOR_GRADIENT_X;
    else
         rectInfo.flag = 0;
    if(sdrv->dflags &DSDRAW_COLOR_GRADIENT_Y)
            rectInfo.flag |= GFXRECT_FLAG_COLOR_GRADIENT_Y;
   }
   else
       rectInfo.flag = 0;

    if(false == dtrapezoid->horiz_direct)
        rectInfo.flag |= GFXDRAW_FLAG_TRAPEZOID_Y ;
    else
        rectInfo.flag |= GFXDRAW_FLAG_TRAPEZOID_X ;

    MApi_GFX_BeginDraw();
    MApi_GFX_TrapezoidFill( &rectInfo) ;
    MApi_GFX_SetTAGID(MApi_GFX_GetNextTAGID(TRUE));
    MApi_GFX_EndDraw();

    return true;
}

/**********************************************************************************************************************/

static int
driver_probe( CoreGraphicsDevice *device )
{
     D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
     //printf("\nDean %s\n", __FUNCTION__);

     //return dfb_gfxcard_get_accelerator( device ) == 0x2D47;
     return 1;
}

static void
driver_get_info( CoreGraphicsDevice *device,
                 GraphicsDriverInfo *info )
{
     D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
     //printf("\nDean %s\n", __FUNCTION__);

     /* fill driver info structure */
     snprintf( info->name,
               DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
               "Mstar Driver" );

     snprintf( info->vendor,
               DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
               "Mstar corp." );

     info->version.major = 0;
     info->version.minor = 1;

     info->driver_data_size = sizeof(MSTARDriverData);
     info->device_data_size = sizeof(MSTARDeviceData);
}

void mstarGetSerial( void *driver_data, void *device_data, CoreGraphicsSerial *serial )
{
     serial->generation = 0;
     serial->serial = (unsigned int)MApi_GFX_GetNextTAGID(FALSE);
}
DFBResult mstarWaitSerial( void *driver_data, void *device_data, const CoreGraphicsSerial *serial )
{
     u16 tagID;

     tagID = (u16)serial->serial;
     MApi_GFX_WaitForTAGID(tagID);

     return DFB_OK;
}

static MS_U32 mstar_dfb_OSD_RESOURCE_SetFBFmt(MS_U16 pitch,MS_U32 addr , MS_U16 fmt )
{
   printf("set osd resource %08x, %d, %04x\n", (u32)addr, pitch, fmt);

   return 0x1;
}
static MS_BOOL mstar_sc_is_interlace(void)
{
  //MS_XC_DST_DispInfo dstDispInfo;
  //MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN);
  return MDrv_MVOP_GetIsInterlace();
}
static MS_U16 mstar_sc_get_h_cap_start(void)
{
#if DFB_BUILD_FOR_PRJ_OBAMA
    return (MS_U16)MAdp_SYS_GetPanelHStart();
#else
    MS_PNL_DST_DispInfo dstDispInfo;
    MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
    return dstDispInfo.DEHST;
#endif
}

static MS_U16 mstar_sc_get_ip0_h_cap_start(void)
{
  MS_XC_DST_DispInfo dstDispInfo;
  MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN);
  return dstDispInfo.DEHST;

}


static void mstar_XC_Sys_PQ_ReduceBW_ForOSD(MS_U8 PqWin, MS_BOOL bOSD_On)
{
   // printf("mstar_XC_Sys_PQ_ReduceBW_ForOSD %d, %s\n", PqWin, bOSD_On?"on":"off");
#ifdef FIX_ME_OBAMA2
    //do Nothing currently. Remove drvPQ dependency first.
#endif
}
#if DFB_BUILD_FOR_PRJ_OBAMA
typedef MS_S8                       FONTHANDLE;      ///< Font handle
typedef MS_S16                  BMPHANDLE;       ///< Bitmap handle
typedef MS_S8                       DBHANDLE;
static MS_U32 mstar_OSD_RESOURCE_GetFontInfoGFX(FONTHANDLE handle, GFX_FontInfo* pinfo)
{
   printf("Should not arrive here!!!\n");
   assert(0);
}
static MS_U32 msAPI_OSD_RESOURCE_GetBitmapInfoGFX(BMPHANDLE handle, GFX_BitmapInfo* pinfo)
{
   printf("Should not arrive here!!!\n");
   assert(0);
}
#endif

static DFBResult
mstar_init_driver( CoreGraphicsDevice *device,
                    void               *driver_data,
                    void               *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;
    u16 pnlW;
    u16 pnlH;

    GOP_InitInfo gopInitInfo;
    u32 u32GOP_Regdma_addr;
    u32 u32GOP_Regdma_size;
    u32 u32GOP_Regdma_aligned;
    u32 u32GOP_Regdma_miu;

    GFX_Config gfx_config;
    GFX_RgbColor gfxColor;
    u32 u32GE_VQ_addr;
    u32 u32GE_VQ_size;
    u32 u32GE_VQ_aligned;
    u32 u32GE_VQ_miu;
    int i=0;
    int status = 0;

    GOP_MuxConfig muxConfig;

    D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
    printf("\n DFB USE THE GFX : MSTAR  WARNING PLEASE REMOVE libdirectfb_mstar.so in mslib\directfb-1.4-0\gfxdrivers \n");
    ////////////////////////////////////////////////////////////////////////////////
    //Init MsOS & SEM:
    MsOS_Init();
    MDrv_SEM_Init();
    MApi_PNL_IOMapBaseInit();
    MDrv_XC_SetIOMapBase();
    MDrv_VE_SetIOMapBase();


    ////////////////////////////////////////////////////////////////////////////////
    //Init GOP:
    //Get GOP Idx:
    sdev->gfx_gop_index=dfb_config->mst_gfx_gop_index;

    //Get pnlW, pnlH
    #if DFB_BUILD_FOR_PRJ_OBAMA
    if(0==dfb_config->mst_lcd_width || 0==dfb_config->mst_lcd_height)
        MAdp_SYS_GetPanelResolution(&pnlW,&pnlH);
    else
    #endif
    {
        pnlW = dfb_config->mst_lcd_width;
        pnlH = dfb_config->mst_lcd_height;
    }

    //Prepare GOP Init Params:
    gopInitInfo.u16PanelWidth = pnlW;
    gopInitInfo.u16PanelHeight= pnlH;
    gopInitInfo.u16PanelHStr = mstar_sc_get_h_cap_start();
    gopInitInfo.u32GOPRBAdr = 0;
    gopInitInfo.u32GOPRBLen = 0;
    gopInitInfo.bEnableVsyncIntFlip = TRUE;
    u32GOP_Regdma_addr = dfb_config->mst_gop_regdmaphys_addr;
    u32GOP_Regdma_size = dfb_config->mst_gop_regdmaphys_len;
    #if DFB_BUILD_FOR_PRJ_OBAMA
    if(0==u32GOP_Regdma_addr || 0==u32GOP_Regdma_size)
    {
        if(false==MAdp_SYS_GetMemoryInfo(MMAP_GOP_REGDMABASE, (unsigned long *)&u32GOP_Regdma_addr, (unsigned long *)&u32GOP_Regdma_size,
                    (unsigned long *)&u32GOP_Regdma_aligned, (unsigned long *)&u32GOP_Regdma_miu))
        {
            u32GOP_Regdma_addr = 0;
            u32GOP_Regdma_size = 0;
        }
    }
    #endif
    gopInitInfo.u32GOPRegdmaAdr = u32GOP_Regdma_addr;
    gopInitInfo.u32GOPRegdmaLen = u32GOP_Regdma_size;

    //Register callback functions:
    MApi_GOP_RegisterFBFmtCB(mstar_dfb_OSD_RESOURCE_SetFBFmt);
    MApi_GOP_RegisterXCIsInterlaceCB(mstar_sc_is_interlace);
    MApi_GOP_RegisterXCGetCapHStartCB(mstar_sc_get_ip0_h_cap_start);
    MApi_GOP_RegisterXCReduceBWForOSDCB(mstar_XC_Sys_PQ_ReduceBW_ForOSD);
    #if DFB_BUILD_FOR_PRJ_OBAMA
    MApi_GFX_RegisterGetFontCB(mstar_OSD_RESOURCE_GetFontInfoGFX);
    MApi_GFX_RegisterGetBMPCB(msAPI_OSD_RESOURCE_GetBitmapInfoGFX);
    #endif

    //Init GOP:

    #if MSTAR_MULTI_GOP_SUPPORT
    for(i=0; i<dfb_config->mst_gop_counts; i++)
    {
        MApi_GOP_InitByGOP(&gopInitInfo,  dfb_config->mst_gop_available[i]);
        //MApi_GOP_GWIN_SetForceWrite(true);
        MApi_GOP_GWIN_SwitchGOP(dfb_config->mst_gop_available[i]);

        status =MApi_GOP_GWIN_SetGOPDst(dfb_config->mst_gop_available[i], (EN_GOP_DST_TYPE)dfb_config->mst_gop_dstPlane[i]);
        //Set Stretch Mode to Linear as GE:
        MApi_GOP_GWIN_Set_HStretchMode(E_GOP_HSTRCH_6TAPE_LINEAR);
        //MApi_GOP_GWIN_SetForceWrite(false);
        //printf("\nSet GOPDst status:%d; the gop number is %d;the gop dst is%d\n",status,dfb_config->mst_gop_available[i],dfb_config->mst_gop_dstPlane[i]);
    }
    #else
    MApi_GOP_InitByGOP(&gopInitInfo,  sdev->gfx_gop_index);
    MApi_GOP_GWIN_SwitchGOP(sdev->gfx_gop_index);
    //Set Stretch Mode to Linear as GE:
    MApi_GOP_GWIN_Set_HStretchMode(E_GOP_HSTRCH_6TAPE_LINEAR);
    #endif
    if(dfb_config->mst_GOP_HMirror>0)
         MApi_GOP_GWIN_SetHMirror(TRUE);
    else if(dfb_config->mst_GOP_HMirror==0)
          MApi_GOP_GWIN_SetHMirror(FALSE);

    if(dfb_config->mst_GOP_VMirror>0)
         MApi_GOP_GWIN_SetVMirror(TRUE);
    else if(dfb_config->mst_GOP_VMirror==0)
          MApi_GOP_GWIN_SetVMirror(FALSE);

    if(dfb_config->mst_mux_counts > 0)
    {
        memset(&muxConfig,0,sizeof(muxConfig));
        for(i=0; i<dfb_config->mst_mux_counts; i++)
        {
            muxConfig.GopMux[i].u8MuxIndex = i;
            muxConfig.GopMux[i].u8GopIndex = dfb_config->mst_gop_mux[i];
           // printf("mux[%d]:gop[%d]",i,muxConfig.GopMux[i].u8GopIndex);
        }
        muxConfig.u8MuxCounts = dfb_config->mst_mux_counts;
        MApi_GOP_GWIN_SetMux(&muxConfig,sizeof(muxConfig));
    }


    //Disable TransClr:
    MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE);

    ////////////////////////////////////////////////////////////////////////////////
    //Init GE:
    //Get VQ Addr & Size:
    u32GE_VQ_addr = dfb_config->mst_ge_vq_phys_addr;
    u32GE_VQ_size = dfb_config->mst_ge_vq_phys_len;
    #if DFB_BUILD_FOR_PRJ_OBAMA
    if(0==u32GE_VQ_addr || 0==u32GE_VQ_size)
    {
        if(false==MAdp_SYS_GetMemoryInfo(MMAP_GE_VQ_BUFFER, (unsigned long *)&u32GE_VQ_addr, (unsigned long *)&u32GE_VQ_size,
                (unsigned long *)&u32GE_VQ_aligned, (unsigned long *)&u32GE_VQ_miu))
        {
            u32GE_VQ_addr = 0;
            u32GE_VQ_size = 0;
        }
    }
    #endif

    //Prepare GE Init Params:
    gfx_config.u8Dither = FALSE;
    gfx_config.bIsCompt = FALSE;
    gfx_config.bIsHK = TRUE;
    gfx_config.u8Miu = ((dfb_config->video_phys_cpu>= dfb_config->mst_miu1_cpu_offset)?1:0);
    gfx_config.u32VCmdQAddr = u32GE_VQ_addr;
    gfx_config.u32VCmdQSize =  u32GE_VQ_size;
    //Init GE:
    MApi_GFX_Init(&gfx_config);
    //Set VCMDQ & Buf:
    if(u32GE_VQ_size>=4*1024)
    {
        GFX_VcmqBufSize vqSize;

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
        if(GFX_SUCCESS == MApi_GFX_SetVCmdBuffer(u32GE_VQ_addr, vqSize))
            MApi_GFX_EnableVCmdQueue(TRUE);
        else
            MApi_GFX_EnableVCmdQueue(FALSE);
    }
    else
    {
        MApi_GFX_EnableVCmdQueue(FALSE);
    }

    gfxColor.a =
    gfxColor.r =
    gfxColor.g =
    gfxColor.b = 0x00;
    MApi_GFX_SetStrBltSckType(GFX_NEAREST, &gfxColor);

    sdrv->src_ge_yuv_fmt = sdrv->dst_ge_yuv_fmt = GFX_YUV_YUYV;
    sdrv->ge_blit_nearestmode_enabled =
    sdrv->ge_blit_xmirror =
    sdrv->ge_blit_ymirror =
    sdrv->ge_draw_dst_colorkey_enabled =
    sdrv->ge_blit_dst_colorkey_enabled =
    sdrv->ge_draw_alpha_blend_enabled =
    sdrv->ge_blit_alpha_blend_enabled =
    sdrv->ge_draw_src_over_patch =
    sdrv->ge_blit_src_over_patch =
    sdrv->ge_src_colorkey_enabled =
    sdrv->ge_palette_enabled =
    sdrv->ge_palette_intensity_enabled =
    sdrv->ge_render_alpha_cmp_enabled = 0;
    sdrv->ge_blit_render_coef = 0xffffffff;
    sdrv->ge_blit_render_alpha_from = (u32)ABL_FROM_ASRC;
    sdrv->ge_last_render_op = MSTAR_GFX_RENDER_OP_NONE;

    return DFB_OK;
}
void mstar_FlushTextureCache( void *driver_data, void *device_data )
{
    MsOS_FlushMemory();
}

void mstar_EngineSync(void *driver_data, void *device_data)
{
    MApi_GFX_FlushQueue();
}

static DFBResult
driver_init_driver( CoreGraphicsDevice  *device,
                    GraphicsDeviceFuncs *funcs,
                    void                *driver_data,
                    void                *device_data,
                    CoreDFB             *core )
{
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = device_data;
     u8  i;

     D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
     //printf("\nDean %s\n", __FUNCTION__);

     /* Keep pointer to shared device data. */
     sdrv->dev = device_data;

     /* Keep core and device pointer. */
     sdrv->core   = core;
     sdrv->device = device;
     memset(sdrv->mstarLayerBuffer, 0, sizeof(sdrv->mstarLayerBuffer));
     MDrv_MMIO_Init(); // I/O remap for current process

    funcs->GetSerial        = mstarGetSerial;
    funcs->WaitSerial        = mstarWaitSerial;
    funcs->CheckState        = mstarCheckState;
    funcs->SetState          = mstarSetState;
    funcs->FillRectangle     = mstarFillRectangle ;
    funcs->DrawRectangle     = mstarDrawRectangle ;
    funcs->DrawOval          = mstarDrawOval ;
    funcs->DrawLine     = mstarDrawLine ;
    funcs->FillTrapezoid = mstarFillTrapezoid;
    funcs->Blit     = mstarBlit ;
    funcs->StretchBlit     = mstarStretchBlit ;
    funcs->BlitTrapezoid = mstarBlitTrapezoid ;
    funcs->StretchBlitEx = mstarStretchBlitEx;
    funcs->FlushTextureCache = mstar_FlushTextureCache;
    funcs->EngineSync = mstar_EngineSync;

     /* Get virtual address for the LCD buffer in slaves here,
        master does it in driver_init_device(). */
     if (!dfb_core_is_master( core ))
          sdrv->lcd_virt = dfb_gfxcard_memory_virtual( device, sdev->lcd_offset );


     /* Register primary screen. */
     sdrv->op_screen  = dfb_screens_register( device, driver_data, &mstarOPScreenFuncs );
     sdrv->ip0_screen = dfb_screens_register( device, driver_data, & mstarIP0ScreenFuncs);
     sdrv->ve_screen  = dfb_screens_register( device, driver_data, & mstarVEScreenFuncs );
     /* Register input system layers. */
     for( i=0; i<MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
     {

#if MSTAR_MULTI_GOP_SUPPORT
          if(i <dfb_config->mst_gop_counts)
          {
               if(dfb_config->mst_gop_dstPlane[i]== E_GOP_DST_OP0)
               {
                   sdrv->layers[i] = dfb_layers_register( sdrv->op_screen, driver_data, &mstarLayerFuncs );
               }
               else if(dfb_config->mst_gop_dstPlane[i]== E_GOP_DST_IP0)
               {
                   sdrv->layers[i] = dfb_layers_register( sdrv->ip0_screen, driver_data, &mstarLayerFuncs );
               }
               else if(dfb_config->mst_gop_dstPlane[i]== E_GOP_DST_MIXER2VE)
               {
                   sdrv->layers[i] = dfb_layers_register( sdrv->ve_screen, driver_data, &mstarLayerFuncs );
               }
               else if(dfb_config->mst_gop_dstPlane[i]== E_GOP_DST_MIXER2OP)
               {
                      sdrv->layers[i] = dfb_layers_register( sdrv->op_screen, driver_data, &mstarLayerFuncs);
               }
               else
               {
                   printf("\n--------unsupport----------\n");
               }
          }
          else
               sdrv->layers[i] = NULL;
#else
          sdrv->layers[i] = dfb_layers_register( sdrv->op_screen, driver_data, &mstarLayerFuncs);
#endif
      }

     /* Register multi window layer. */
     //sdrv->multi = dfb_layers_register( sdrv->screen, driver_data, &sh7722MultiLayerFuncs );
     return mstar_init_driver(device, driver_data, device_data);
}

static DFBResult
driver_init_device( CoreGraphicsDevice *device,
                    GraphicsDeviceInfo *device_info,
                    void               *driver_data,
                    void               *device_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = device_data;
    u16 pnlW;
    u16 pnlH;
    int i;

    //sdev->gfx_gop_index=dfb_config->mst_gfx_gop_index;
    D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
    //printf("\nDean %s\n", __FUNCTION__);
    //  printf("MAdp_SYS_GetPanelResolution-->%d, %d\n", pnlW, pnlH);

    for( i=0; i<MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {
        sdev->layer_active[i] = false;
        sdev->layer_gwin_id[i] = INVALID_WIN_ID;
        sdev->layer_refcnt[i] = 0x0;
    }


    //Get pnlW, pnlH
    #if DFB_BUILD_FOR_PRJ_OBAMA
    if(0==dfb_config->mst_lcd_width || 0==dfb_config->mst_lcd_height)
        MAdp_SYS_GetPanelResolution(&pnlW,&pnlH);
    else
    #endif
    {
        pnlW = dfb_config->mst_lcd_width;
        pnlH = dfb_config->mst_lcd_height;
    }

    if(dfb_config->mst_gfx_width==0)
    {
        sdev->gfx_max_width = pnlW;
    }
    else
    {
        sdev->gfx_max_width = dfb_config->mst_gfx_width;
        if(sdev->gfx_max_width > pnlW)
            sdev->gfx_max_width = pnlW;
    }

    if(dfb_config->mst_gfx_height==0)
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

    sdev->gfx_h_offset=dfb_config->mst_gfx_h_offset;
    sdev->gfx_v_offset=dfb_config->mst_gfx_v_offset;
    sdev->mst_miu1_hal_offset   = dfb_config->mst_miu1_hal_offset;
    sdev->mst_miu1_cpu_offset   = dfb_config->mst_miu1_cpu_offset;

    //Init A8 GE Palette default:
    sdev->a8_palette_color.r = sdev->a8_palette_color.g = sdev->a8_palette_color.b = 0xFF;
    sdev->palette_tbl[0].RGB.u8A = 0;
    sdev->palette_tbl[0].RGB.u8R = 0xFF;
    sdev->palette_tbl[0].RGB.u8G = 0xFF;
    sdev->palette_tbl[0].RGB.u8B = 0xFF;
    for(i=1; i < MAX_PALETTE_ENTRY_CNT; ++i)
    {
        sdev->palette_tbl[i] = sdev->palette_tbl[0];
        sdev->palette_tbl[i].RGB.u8A = i;
    }
    sdev->num_entries = MAX_PALETTE_ENTRY_CNT;
    sdev->a8_palette = true;
    MApi_GFX_SetPaletteOpt(sdev->palette_tbl, 0, (sdev->num_entries-1));

    /*
     * Initialize I1/I2/I4 related palette.
     */
    sdev->num_intensity = 0x0;

    /*
    * Initialize hardware.
    */
    device_info->caps.accel    = DFXL_FILLRECTANGLE|DFXL_DRAWRECTANGLE|DFXL_DRAWLINE|DFXL_BLIT2|DFXL_FILLTRAPEZOID|DFXL_BLIT|DFXL_STRETCHBLIT|DFXL_TRAPEZOID_BLIT;
    device_info->caps.blitting = DSBLIT_DEMULTIPLY | DSBLIT_BLEND_COLORALPHA | DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_SRC_PREMULTIPLY | DSBLIT_SRC_PREMULTCOLOR | DSBLIT_DST_PREMULTIPLY |DSBLIT_SRC_COLORKEY;
    device_info->caps.drawing  = DSDRAW_BLEND|DSDRAW_SRC_PREMULTIPLY|DSDRAW_DST_PREMULTIPLY|DSDRAW_DEMULTIPLY;
    /* Set device limitations. */
    device_info->limits.surface_byteoffset_alignment = 16;
    device_info->limits.surface_bytepitch_alignment  = 16;
    device_info->limits.surface_hegiht_alignment     = 16;

    //default the hwclip is enable
    device_info->caps.flags |= CCF_CLIPPING;
    device_info->caps.clip = DFXL_ALL;
    sdev->b_hwclip = true;

    if(dfb_config->mst_disable_hwclip)
    {
        device_info->caps.flags &= ~CCF_CLIPPING;
        device_info->caps.clip = 0;
        sdev->b_hwclip = false;
    }

    /* Initialize BEU lock. */
    fusion_skirmish_init( &sdev->beu_lock, "BEU", dfb_core_world(sdrv->core) );

    return DFB_OK;
}

static void
driver_close_device( CoreGraphicsDevice *device,
                     void               *driver_data,
                     void               *device_data )
{
     MSTARDeviceData *sdev = device_data;

     D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );
     //printf("\nDean %s\n", __FUNCTION__);

     /* Destroy BEU lock. */
     fusion_skirmish_destroy( &sdev->beu_lock );
}
void  mstarAllSurfInfo(MSTARDriverData *sdrv);
static void
driver_close_driver( CoreGraphicsDevice *device,
                     void               *driver_data )
{
     MSTARDriverData    *sdrv   = driver_data;

     D_DEBUG_AT( MSTAR_Driver, "%s()\n", __FUNCTION__ );

     mstarAllSurfInfo(sdrv);
}


#define PIXEL_CHN_MASK(st, bits) (((1UL<<(bits))-1)<<(st))

#define PIXEL_CHN_VAL(pixel, st, bits)  ((((u32)pixel)&PIXEL_CHN_MASK(st, bits))>>(st))
#define PIXEL_CHN_NORMAL(pixel, st, bits)  (PIXEL_CHN_VAL(pixel, st, bits)<<(8-(bits)))

unsigned long
mstar_build_color(u32 pixel,  u8 a_st, u8  a_bits, u8  r_st, u8  r_bits,
                               u8  g_st, u8  g_bits, u8  b_st, u8  b_bits)
{
     u32 a, r, g, b;

     if(a_bits)
        a = PIXEL_CHN_NORMAL(pixel, a_st, a_bits);
     else
        a = 0;
     if(r_bits)
         r = PIXEL_CHN_NORMAL(pixel, r_st, r_bits);
     else
      r = 0;
     if(g_bits)
         g = PIXEL_CHN_NORMAL(pixel, g_st, g_bits);
     else
      g = 0;
     if(b_bits)
         b = PIXEL_CHN_NORMAL(pixel, b_st, b_bits);
     else
      b = 0;

     if(a_bits)
     {
        while(a_bits<8)
        {
            a = a| a>>a_bits;
            a_bits <<= 1;
        }
     }
     if(r_bits)
     {
         while(r_bits<8)
         {
             r = r| r>>r_bits;
            r_bits <<= 1;
         }
     }
     if(g_bits)
     {
          while(g_bits<8)
         {
             g = g| g>>g_bits;
             g_bits <<= 1;
         }
     }
     if(b_bits)
     {
         while(b_bits<8)
        {
             b = b| b>>b_bits;
            b_bits <<= 1;
         }
     }
     return a<<24 | r<<16 | g<<8| b;

}
unsigned long
    mstar_pixel_to_color( DFBSurfacePixelFormat  format,
                      unsigned long pixel, CorePalette *palette)
{
     u32 y, cb, cr,r,g,b;

     switch (format) {

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

          case DSPF_LUT8:
          case DSPF_LUT4:
          case DSPF_LUT2:
          case DSPF_LUT1:
               if( palette )
               {
                    if(pixel>=0 && pixel<=(DFB_BITS_PER_PIXEL(format)<<1UL)-1)
                       return  ((unsigned long)palette->entries[pixel].a)<<24|
                                ((unsigned long)palette->entries[pixel].r)<<16|
                                ((unsigned long)palette->entries[pixel].g)<<8|
                                palette->entries[pixel].b ;
               }

               break;

          case DSPF_YUY2:
                cb = (pixel>>24) & 0xFF;
                y  = (pixel>>16) & 0xFF;
                cr = (pixel>>8) & 0xFF;

                YCBCR_TO_RGB( y, cb, cr, r, g, b );
                return 0xFF<<24|r<<16|g<<8|b;
          case DSPF_YVYU:
               cr = (pixel>>24) & 0xFF;
               y  = (pixel>>16) & 0xFF;
               cb = (pixel>>8) & 0xFF;

               YCBCR_TO_RGB( y, cb, cr, r, g, b );
               return 0xFF<<24|r<<16|g<<8|b;

          case DSPF_UYVY:
               y = (pixel>>24) & 0xFF;
               cb  = (pixel>>16) & 0xFF;
               cr = (pixel) & 0xFF;

               YCBCR_TO_RGB( y, cb, cr, r, g, b );
               return 0xFF<<24|r<<16|g<<8|b;
          default:
               D_WARN( "unknown format 0x%08x", format );
               break;
     }

     return 0x55555555;
}
unsigned long mstar_pixel_to_colorkey(DFBSurfacePixelFormat  format,
                      unsigned long pixel, CorePalette *palette)
{
     switch (format)
     {
          case DSPF_LUT8:
          case DSPF_LUT4:
          case DSPF_LUT2:
          case DSPF_LUT1:
            if(pixel>=0 && pixel<=(1UL<<DFB_BITS_PER_PIXEL(format))-1)
                return pixel<<24|pixel<<16|pixel<<8|pixel;
            return 0;
          default:
            return mstar_pixel_to_color(format, pixel, palette);
     }
}
