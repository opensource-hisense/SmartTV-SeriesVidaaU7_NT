/*
   (c) Copyright 2001-2010  The world wide DirectFB Open Source Community (directfb.org)
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>
#include <string.h>
#include <stdarg.h>

#include <directfb.h>

#include <display/idirectfbsurface.h>

#include <media/idirectfbimageprovider.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>

#include <core/CoreSurface.h>

#include <misc/gfx_util.h>
#include <misc/util.h>

#include <gfx/clip.h>
#include <gfx/convert.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include "config.h"

#if PNG_LIBPNG_VER < 10400 
#define trans_color  trans_values
#define trans_alpha  trans
#endif 

static DFBResult
Probe( IDirectFBImageProvider_ProbeContext *ctx );

static DFBResult
Construct( IDirectFBImageProvider *thiz,
           ... );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBImageProvider, PNG )


enum {
     STAGE_ABORT = -2,
     STAGE_ERROR = -1,
     STAGE_START =  0,
     STAGE_INFO,
     STAGE_IMAGE,
     STAGE_END
};

/*
 * private data struct of IDirectFBImageProvider_PNG
 */
typedef struct {
     IDirectFBImageProvider_data base;

     int                  stage;
     int                  rows;

     png_structp          png_ptr;
     png_infop            info_ptr;

     png_uint_32          width;
     png_uint_32          height;
     int                  bpp;
     int                  color_type;
     png_uint_32          color_key;
     bool                 color_keyed;

     void                *image;
     int                  pitch;
     u32                  palette[256];
     DFBColor             colors[256];

} IDirectFBImageProvider_PNG_data;


static DirectResult
IDirectFBImageProvider_PNG_AddRef  ( IDirectFBImageProvider *thiz );

static DirectResult
IDirectFBImageProvider_PNG_Release ( IDirectFBImageProvider *thiz );

static DFBResult
IDirectFBImageProvider_PNG_RenderTo( IDirectFBImageProvider *thiz,
                                     IDirectFBSurface       *destination,
                                     const DFBRectangle     *destination_rect );

static DFBResult
IDirectFBImageProvider_PNG_SetRenderCallback( IDirectFBImageProvider *thiz,
                                              DIRenderCallback        callback,
                                              void                   *context );

static DFBResult
IDirectFBImageProvider_PNG_GetSurfaceDescription( IDirectFBImageProvider *thiz,
                                                  DFBSurfaceDescription  *dsc );

static DFBResult
IDirectFBImageProvider_PNG_GetImageDescription( IDirectFBImageProvider *thiz,
                                                DFBImageDescription    *dsc );

/* Called at the start of the progressive load, once we have image info */
static void
png_info_callback (png_structp png_read_ptr,
                   png_infop   png_info_ptr);

/* Called for each row; note that you will get duplicate row numbers
   for interlaced PNGs */
static void
png_row_callback  (png_structp png_read_ptr,
                   png_bytep   new_row,
                   png_uint_32 row_num,
                   int         pass_num);

/* Called after reading the entire image */
static void
png_end_callback  (png_structp png_read_ptr,
                   png_infop   png_info_ptr);

/* Pipes data into libpng until stage is different from the one specified. */
static DFBResult
push_data_until_stage (IDirectFBImageProvider_PNG_data *data,
                       int                              stage,
                       int                              buffer_size);

/**********************************************************************************************************************/

static void
IDirectFBImageProvider_PNG_Destruct( IDirectFBImageProvider *thiz )
{
     IDirectFBImageProvider_PNG_data *data =
                              (IDirectFBImageProvider_PNG_data*)thiz->priv;

     png_destroy_read_struct( &data->png_ptr, &data->info_ptr, NULL );

     /* Decrease the data buffer reference counter. */
     data->base.buffer->Release( data->base.buffer );

     /* Deallocate image data. */
     if (data->image)
          D_FREE( data->image );
}

/**********************************************************************************************************************/

static DFBResult
Probe( IDirectFBImageProvider_ProbeContext *ctx )
{
     if (!png_sig_cmp( ctx->header, 0, 8 ))
          return DFB_OK;

     return DFB_UNSUPPORTED;
}



/*
Wrap the IDirectFBImageProvider_PNG_xxx to _IDirectFBImageProvider_PNG_xxx for safe call
*/

static DirectResult
_IDirectFBImageProvider_PNG_AddRef( IDirectFBImageProvider *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_AddRef(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DirectResult
_IDirectFBImageProvider_PNG_Release( IDirectFBImageProvider *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBImageProvider_PNG_RenderTo( IDirectFBImageProvider *thiz,
                                     IDirectFBSurface       *destination,
                                     const DFBRectangle     *dest_rect )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_RenderTo(thiz, destination, dest_rect);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}


static DFBResult
_IDirectFBImageProvider_PNG_SetRenderCallback( IDirectFBImageProvider *thiz,
                                              DIRenderCallback        callback,
                                              void                   *context )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_SetRenderCallback(thiz,callback,context);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBImageProvider_PNG_GetSurfaceDescription( IDirectFBImageProvider *thiz,
                                                  DFBSurfaceDescription *dsc )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_GetSurfaceDescription(thiz,dsc);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBImageProvider_PNG_GetImageDescription( IDirectFBImageProvider *thiz,
                                                DFBImageDescription    *dsc )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBImageProvider_PNG_GetImageDescription(thiz,dsc);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}


static DFBResult
Construct( IDirectFBImageProvider *thiz,
           ... )
{
     DFBResult ret = DFB_FAILURE;

     IDirectFBDataBuffer *buffer;
     CoreDFB             *core;
     va_list              tag;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBImageProvider_PNG)

     va_start( tag, thiz );
     buffer = va_arg( tag, IDirectFBDataBuffer * );
     core = va_arg( tag, CoreDFB * );
     va_end( tag );

     data->base.ref    = 1;
     data->base.buffer = buffer;
     data->base.core   = core;

     buffer->AddRef( buffer );

     /* Create the PNG read handle. */
     data->png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
                                             NULL, NULL, NULL );
     if (!data->png_ptr)
          goto error;

     if (setjmp( data->png_ptr->jmpbuf )) {
          D_ERROR( "ImageProvider/PNG: Error reading header!\n" );
          goto error;
     }

     /* Create the PNG info handle. */
     data->info_ptr = png_create_info_struct( data->png_ptr );
     if (!data->info_ptr)
          goto error;

     /* Setup progressive image loading. */
     png_set_progressive_read_fn( data->png_ptr, data,
                                  png_info_callback,
                                  png_row_callback,
                                  png_end_callback );


     /* Read until info callback is called. */
     ret = push_data_until_stage( data, STAGE_INFO, 64 );
     if (ret)
          goto error;

     data->base.Destruct = IDirectFBImageProvider_PNG_Destruct;

     thiz->AddRef = _IDirectFBImageProvider_PNG_AddRef;
     thiz->Release = _IDirectFBImageProvider_PNG_Release;
     thiz->RenderTo              = _IDirectFBImageProvider_PNG_RenderTo;
     thiz->SetRenderCallback     = _IDirectFBImageProvider_PNG_SetRenderCallback;
     thiz->GetImageDescription   = _IDirectFBImageProvider_PNG_GetImageDescription;
     thiz->GetSurfaceDescription = _IDirectFBImageProvider_PNG_GetSurfaceDescription;

     return DFB_OK;

error:
     if (data->png_ptr)
          png_destroy_read_struct( &data->png_ptr, &data->info_ptr, NULL );

     buffer->Release( buffer );

     if (data->image)
          D_FREE( data->image );

     DIRECT_DEALLOCATE_INTERFACE(thiz);

     return ret;
}

/**********************************************************************************************************************/

static DFBResult
IDirectFBImageProvider_PNG_RenderTo( IDirectFBImageProvider *thiz,
                                     IDirectFBSurface       *destination,
                                     const DFBRectangle     *dest_rect )
{
     DFBResult              ret = DFB_OK;
     IDirectFBSurface_data *dst_data;
     CoreSurface           *dst_surface, *dst_surface_org;
     DFBRegion              clip;
     DFBRectangle           srect,rect;
     png_infop              info;
     int                    x, y;
     DFBRectangle           clipped;
     DFBSurfaceCapabilities caps;

     CoreSurface *surface_tmp = NULL;
     CoreSurfaceConfig config_tmp;
     int width_tmp, height_tmp;

     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA (IDirectFBImageProvider_PNG)

     info = data->info_ptr;

     dst_data = (IDirectFBSurface_data*) destination->priv;
     if (!dst_data)
          return DFB_DEAD;

     dst_surface_org = dst_data->surface;
     if (!dst_surface_org)
          return DFB_DESTROYED;

     destination->GetCapabilities( destination, &caps );
     if (caps & DSCAPS_PREMULTIPLIED)
     {
          //create tmp Surface
          destination->GetSize(destination, &width_tmp, &height_tmp);

          config_tmp.flags    = CSCONF_SIZE | CSCONF_FORMAT |  CSCONF_CAPS;
          config_tmp.size.w   = width_tmp;
          config_tmp.size.h   = height_tmp;
          config_tmp.format   = DSPF_ARGB;
          config_tmp.caps     = DSCAPS_VIDEOHIGH;
          ret = dfb_surface_create(data->base.core, &config_tmp, CSTF_EXTERNAL, 0, NULL, &surface_tmp);
          if (ret) {
              printf("[DFB]%s:%04d, :CreateSurface Failed!!\n", __FUNCTION__, __LINE__);
              return ret;
          }

          dst_surface = surface_tmp;  //set
     }
     else
     {
          dst_surface = dst_surface_org;  //set
     }


     dfb_region_from_rectangle( &clip, &dst_data->area.current );

     if (dest_rect) {
          if (dest_rect->w < 1 || dest_rect->h < 1)
          {
               if (surface_tmp)  dfb_surface_unref( surface_tmp );
               return DFB_INVARG;
          }
          rect = *dest_rect;
          rect.x += dst_data->area.wanted.x;
          rect.y += dst_data->area.wanted.y;
     }
     else {
          rect = dst_data->area.wanted;
     }

     if (setjmp( data->png_ptr->jmpbuf )) {
          D_ERROR( "ImageProvider/PNG: Error during decoding!\n" );

          if (data->stage < STAGE_IMAGE)
          {
               if (surface_tmp)  dfb_surface_unref( surface_tmp );
               return DFB_FAILURE;
          }

          data->stage = STAGE_ERROR;
     }

     /* Read until image is completely decoded. */
     if (data->stage != STAGE_ERROR) {
          ret = push_data_until_stage( data, STAGE_END, 16384 );
          if (ret)
          {
               if (surface_tmp)  dfb_surface_unref( surface_tmp );
               return ret;
          }
     }

     clipped = rect;

     if (!dfb_rectangle_intersect_by_region( &clipped, &clip ))
     {
          if (surface_tmp)  dfb_surface_unref( surface_tmp );
          return DFB_INVAREA;
     }

     /* actual rendering */
     if (0    &&   // FIXME
           rect.w == data->width && rect.h == data->height &&
         (data->color_type == PNG_COLOR_TYPE_RGB || data->color_type == PNG_COLOR_TYPE_RGBA) &&
         (dst_surface->config.format == DSPF_RGB32 || dst_surface->config.format == DSPF_ARGB) &&
         !(dst_surface->config.caps & DSCAPS_PREMULTIPLIED))
     {
          //ret = dfb_surface_write_buffer( dst_surface, CSBR_BACK,
          //                                data->image +
          //                                   (clipped.x - rect.x) * 4 +
          //                                   (clipped.y - rect.y) * data->width * 4,
          //                                data->width * 4, &clipped );
     }
     else {
          CoreSurfaceBufferLock lock;

          ret = dfb_surface_lock_buffer( dst_surface, CSBR_BACK, CSAID_CPU, CSAF_WRITE, &lock );
          if (ret)
           {
               if (surface_tmp)  dfb_surface_unref( surface_tmp );
               return ret;
           }

          switch (data->color_type) {
               case PNG_COLOR_TYPE_PALETTE:
                    if (dst_surface->config.format == DSPF_LUT8 && data->info_ptr->bit_depth == 8) {
                         /*
                          * Special indexed PNG to LUT8 loading.
                          */

                         /* FIXME: Limitation for LUT8 is to load complete surface only. */
                         dfb_clip_rectangle( &clip, &rect );
                         if (rect.x == 0 && rect.y == 0 &&
                             rect.w == dst_surface->config.size.w  &&
                             rect.h == dst_surface->config.size.h &&
                             rect.w == data->width         &&
                             rect.h == data->height)
                         {
                              for (y=0; y<data->height; y++)
                                   direct_memcpy( lock.addr + lock.pitch * y,
                                                  data->image + data->pitch * y,
                                                  data->width );

                              break;
                         }
                    }
                    /* fall through */

               case PNG_COLOR_TYPE_GRAY: {
                    /*
                     * Convert to ARGB and use generic loading code.
                     */

                    // FIXME: allocates four additional bytes because the scaling functions
                    //        in src/misc/gfx_util.c have an off-by-one bug which causes
                    //        segfaults on darwin/osx (not on linux)                
                    int size = data->width * data->height * 4 + 4;

                    /* allocate image data */
                    void *image_argb = D_MALLOC( size );

                    if (!image_argb) {
                         D_ERROR( "DirectFB/ImageProvider_PNG: Could not "
                                  "allocate %d bytes of system memory!\n", size );
                         ret = DFB_NOSYSTEMMEMORY;
                    }
                    else {
                         if (data->color_type == PNG_COLOR_TYPE_GRAY) {
                              int num = 1 << data->info_ptr->bit_depth;

                              for (x=0; x<num; x++) {
                                   int value = x * 255 / (num - 1);

                                   if(data->color_keyed)
                                   {
                                        /* apply color key for palette format */
                                        if(value == (data->color_key & 0xff))
                                            data->palette[x] = 0;
                                        else
                                            data->palette[x] = 0xff000000 | (value << 16) | (value << 8) | value;
                                   }
                                   else
                                     data->palette[x] = 0xff000000 | (value << 16) | (value << 8) | value;
                              }
                         }

                         switch (data->info_ptr->bit_depth) {
                              case 8:
                                   for (y=0; y<data->height; y++) {
                                        u8  *S = data->image + data->pitch * y;
                                        u32 *D = image_argb  + data->width * y * 4;

                                        for (x=0; x<data->width; x++)
                                             D[x] = data->palette[ S[x] ];
                                   }
                                   break;

                              case 4:
                                   for (y=0; y<data->height; y++) {
                                        u8  *S = data->image + data->pitch * y;
                                        u32 *D = image_argb  + data->width * y * 4;

                                        for (x=0; x<data->width; x++) {
                                             if (x & 1)
                                                  D[x] = data->palette[ S[x>>1] & 0xf ];
                                             else
                                                  D[x] = data->palette[ S[x>>1] >> 4 ];
                                        }
                                   }
                                   break;

                              case 2:
                                   for (y=0; y<data->height; y++) {
                                        int  n = 6;
                                        u8  *S = data->image + data->pitch * y;
                                        u32 *D = image_argb  + data->width * y * 4;

                                        for (x=0; x<data->width; x++) {
                                             D[x] = data->palette[ (S[x>>2] >> n) & 3 ];

                                             n = (n ? n - 2 : 6);
                                        }
                                   }
                                   break;

                              case 1:
                                   for (y=0; y<data->height; y++) {
                                        int  n = 7;
                                        u8  *S = data->image + data->pitch * y;
                                        u32 *D = image_argb  + data->width * y * 4;

                                        for (x=0; x<data->width; x++) {
                                             D[x] = data->palette[ (S[x>>3] >> n) & 1 ];

                                             n = (n ? n - 1 : 7);
                                        }
                                   }
                                   break;

                              default:
                                   D_ERROR( "ImageProvider/PNG: Unsupported indexed bit depth %d!\n",
                                            data->info_ptr->bit_depth );
                         }

                         dfb_scale_linear_32( image_argb, data->width, data->height,
                                              lock.addr, lock.pitch, &rect, dst_surface, &clip );

                         D_FREE( image_argb );
                    }
                    break;
               }
               default:
                    /*
                     * Generic loading code.
                     */
                    dfb_scale_linear_32( data->image, data->width, data->height,
                                         lock.addr, lock.pitch, &rect, dst_surface, &clip );
                    break;
          }

          dfb_surface_unlock_buffer( dst_surface, &lock );
     }

     if (data->stage != STAGE_END)
          ret = DFB_INCOMPLETE;


     //stretch blt to destsurface
     if (caps & DSCAPS_PREMULTIPLIED)
     {
          DFBSurfaceBlittingFlags  old_blittingflags = dst_data->state.blittingflags;
          destination->SetBlittingFlags( destination, DSBLIT_SRC_PREMULTIPLY );

          dfb_state_set_source( &dst_data->state, dst_surface );
          //set srcrect
          srect.x = 0;
          srect.y = 0;
          srect.w = data->width;
          srect.h = data->height;

          dfb_gfxcard_stretchblit( &srect, &rect, &dst_data->state );

          dfb_state_set_source( &dst_data->state, NULL );
          destination->SetBlittingFlags( destination, old_blittingflags );
     }

     if (surface_tmp)  dfb_surface_unref( surface_tmp );



     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_IMAGEPROVIDER_PNG_RENDERTO);

     return ret;
}


static DirectResult
IDirectFBImageProvider_PNG_AddRef( IDirectFBImageProvider *thiz )
{
     DIRECT_INTERFACE_GET_DATA (IDirectFBImageProvider_PNG)

     data->base.ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBImageProvider_PNG_Release( IDirectFBImageProvider *thiz )
{
     DIRECT_INTERFACE_GET_DATA (IDirectFBImageProvider_PNG)

     if (--data->base.ref == 0) {
          IDirectFBImageProvider_PNG_Destruct( thiz );
     }

     return DFB_OK;
}



static DFBResult
IDirectFBImageProvider_PNG_SetRenderCallback( IDirectFBImageProvider *thiz,
                                              DIRenderCallback        callback,
                                              void                   *context )
{
     DIRECT_INTERFACE_GET_DATA (IDirectFBImageProvider_PNG)

     data->base.render_callback         = callback;
     data->base.render_callback_context = context;

     return DFB_OK;
}


static DFBResult
IDirectFBImageProvider_PNG_GetSurfaceDescription( IDirectFBImageProvider *thiz,
                                                  DFBSurfaceDescription *dsc )
{
     DFBSurfacePixelFormat primary_format = dfb_primary_layer_pixelformat();

     DIRECT_INTERFACE_GET_DATA (IDirectFBImageProvider_PNG)

     dsc->flags  = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
     dsc->width  = data->width;
     dsc->height = data->height;

#ifdef CC_HW_WINDOW_SUPPORT
     if (data->color_type & PNG_COLOR_MASK_ALPHA)
     {
          if(PNG_COLOR_TYPE_RGBA == (data->color_type & PNG_COLOR_TYPE_RGBA))
          {
              if(data->bpp == 32)
              {
                   dsc->pixelformat = DSPF_ARGB;
              }
			  else if(data->bpp == 16)
			  {
			       dsc->pixelformat = DSPF_ARGB4444;
			  }
			  else
			  {
			       dsc->pixelformat = DFB_PIXELFORMAT_HAS_ALPHA(primary_format) ? primary_format : DSPF_ARGB;
			  }
          }
		  else
		  {
	           dsc->pixelformat = DFB_PIXELFORMAT_HAS_ALPHA(primary_format) ? primary_format : DSPF_ARGB;
		  }
     }
     else
     {
          if(PNG_COLOR_TYPE_RGB == (data->color_type & PNG_COLOR_TYPE_RGB))
          {
              if(data->bpp == 24)
              {
                   dsc->pixelformat = DSPF_ARGB;
              }
			  else if(data->bpp == 12)
			  {
			       dsc->pixelformat = DSPF_ARGB4444;
			  }
			  else
			  {
			       dsc->pixelformat = primary_format;
			  }          
          }
		  else
		  {
	           dsc->pixelformat = primary_format;
		  }
     }
#else
     if (data->color_type & PNG_COLOR_MASK_ALPHA)
          dsc->pixelformat = DFB_PIXELFORMAT_HAS_ALPHA(primary_format) ? primary_format : DSPF_ARGB;
     else
          dsc->pixelformat = primary_format;
#endif

     if (data->color_type == PNG_COLOR_TYPE_PALETTE) {
          dsc->flags  |= DSDESC_PALETTE;

          dsc->palette.entries = data->colors;  /* FIXME */
          dsc->palette.size    = 256;
     }

     return DFB_OK;
}

static DFBResult
IDirectFBImageProvider_PNG_GetImageDescription( IDirectFBImageProvider *thiz,
                                                DFBImageDescription    *dsc )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBImageProvider_PNG)

     if (!dsc)
          return DFB_INVARG;

     dsc->caps = DICAPS_NONE;

     if (data->color_type & PNG_COLOR_MASK_ALPHA)
          dsc->caps |= DICAPS_ALPHACHANNEL;

     if (data->color_keyed) {
          dsc->caps |= DICAPS_COLORKEY;

          dsc->colorkey_r = (data->color_key & 0xff0000) >> 16;
          dsc->colorkey_g = (data->color_key & 0x00ff00) >>  8;
          dsc->colorkey_b = (data->color_key & 0x0000ff);
     }

     return DFB_OK;
}

/**********************************************************************************************************************/

#define MAXCOLORMAPSIZE 256

static int SortColors (const void *a, const void *b)
{
     return (*((const u8 *) a) - *((const u8 *) b));
}

/*  looks for a color that is not in the colormap and ideally not
    even close to the colors used in the colormap  */
static u32 FindColorKey( int n_colors, u8 *cmap )
{
     u32   color = 0xFF000000;
     u8    csort[n_colors];
     int   i, j, index, d;

     if (n_colors < 1)
          return color;

     for (i = 0; i < 3; i++) {
          direct_memcpy( csort, cmap + (n_colors * i), n_colors );
          qsort( csort, n_colors, 1, SortColors );

          for (j = 1, index = 0, d = 0; j < n_colors; j++) {
               if (csort[j] - csort[j-1] > d) {
                    d = csort[j] - csort[j-1];
                    index = j;
               }
          }
          if ((csort[0] - 0x0) > d) {
               d = csort[0] - 0x0;
               index = n_colors;
          }
          if (0xFF - (csort[n_colors - 1]) > d) {
               index = n_colors + 1;
          }

          if (index < n_colors)
               csort[0] = csort[index] - (d/2);
          else if (index == n_colors)
               csort[0] = 0x0;
          else
               csort[0] = 0xFF;

          color |= (csort[0] << (8 * (2 - i)));
     }

     return color;
}

/* Called at the start of the progressive load, once we have image info */
static void
png_info_callback( png_structp png_read_ptr,
                   png_infop   png_info_ptr )
{
     int                              i;
     IDirectFBImageProvider_PNG_data *data;

     u32 bpp1[2] = {0, 0xff};
     u32 bpp2[4] = {0, 0x55, 0xaa, 0xff};
     u32 bpp4[16] = {0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

     data = png_get_progressive_ptr( png_read_ptr );

     /* error stage? */
     if (data->stage < 0)
          return;

     /* set info stage */
     data->stage = STAGE_INFO;

     png_get_IHDR( data->png_ptr, data->info_ptr,
                   &data->width, &data->height, &data->bpp, &data->color_type,
                   NULL, NULL, NULL );

     if (png_get_valid( data->png_ptr, data->info_ptr, PNG_INFO_tRNS )) {
          data->color_keyed = true;

          /* generate color key based on palette... */
          if (data->color_type == PNG_COLOR_TYPE_PALETTE) {
               u32        key;
               png_colorp palette    = data->info_ptr->palette;
               png_bytep  trans      = data->info_ptr->trans_alpha;
               int        num_colors = MIN( MAXCOLORMAPSIZE,
                                            data->info_ptr->num_palette );
               u8         cmap[3][num_colors];

               for (i=0; i<num_colors; i++) {
                    cmap[0][i] = palette[i].red;
                    cmap[1][i] = palette[i].green;
                    cmap[2][i] = palette[i].blue;
               }

               key = FindColorKey( num_colors, &cmap[0][0] );

               for (i=0; i<data->info_ptr->num_trans; i++) {
                    if (!trans[i]) {
                         palette[i].red   = (key & 0xff0000) >> 16;
                         palette[i].green = (key & 0x00ff00) >>  8;
                         palette[i].blue  = (key & 0x0000ff);
                    }
               }

               data->color_key = key;
          }
          else if (data->color_type == PNG_COLOR_TYPE_GRAY) {
                   /* ...or based on trans gray value */
                   png_bytep     trans_alpha;
                   png_color_16p trans_color;
                   int           num_trans = 0;

                   if (png_get_tRNS( data->png_ptr, data->info_ptr,
                                     &trans_alpha, &num_trans, &trans_color )) {

                        switch (data->bpp) {
                             case 1:
                                  data->color_key = (((bpp1[trans_color[0].gray]) << 16) |
                                                     ((bpp1[trans_color[0].gray]) << 8) |
                                                     ((bpp1[trans_color[0].gray])));
                                  break;
                             case 2:
                                  data->color_key = (((bpp2[trans_color[0].gray]) << 16) |
                                                     ((bpp2[trans_color[0].gray]) << 8) |
                                                     ((bpp2[trans_color[0].gray])));
                                  break;
                             case 4:
                                  data->color_key = (((bpp4[trans_color[0].gray]) << 16) |
                                                     ((bpp4[trans_color[0].gray]) << 8) |
                                                     ((bpp4[trans_color[0].gray])));
                                  break;
                             case 8:
                                  data->color_key = (((trans_color[0].gray & 0x00ff) << 16) |
                                                     ((trans_color[0].gray & 0x00ff) << 8) |
                                                     ((trans_color[0].gray & 0x00ff)));
                                  break;
                             case 16:
                             default:
                                  data->color_key = (((trans_color[0].gray & 0xff00) << 8) |
                                                     ((trans_color[0].gray & 0xff00)) |
                                                     ((trans_color[0].gray & 0xff00) >> 8));
                                  break;
                        }

                        D_INFO("[DFB] data->color_key =0x%08x (%s, %d)\n", data->color_key,  __FUNCTION__, __LINE__);

                   }

          }
          else {
               /* ...or based on trans rgb value */
               png_bytep     trans_alpha;
               png_color_16p trans_color;
               int           num_trans = 0;


               if (png_get_tRNS( data->png_ptr, data->info_ptr,
                                 &trans_alpha, &num_trans, &trans_color )) {
                    switch(data->bpp) {
                         case 1:
                              data->color_key = (((bpp1[trans_color[0].red]) << 16) |
                                                 ((bpp1[trans_color[0].green]) << 8) |
                                                 ((bpp1[trans_color[0].blue])));
                              break;
                         case 2:
                              data->color_key = (((bpp2[trans_color[0].red]) << 16) |
                                                 ((bpp2[trans_color[0].green]) << 8) |
                                                 ((bpp2[trans_color[0].blue])));
                              break;
                         case 4:
                              data->color_key = (((bpp4[trans_color[0].red]) << 16) |
                                                 ((bpp4[trans_color[0].green]) << 8) |
                                                 ((bpp4[trans_color[0].blue])));
                              break;
                         case 8:
                              data->color_key = (((trans_color[0].red & 0x00ff) << 16) |
                                                 ((trans_color[0].green & 0x00ff) << 8) |
                                                 ((trans_color[0].blue & 0x00ff)));
                              break;
                         case 16:
                         default:
                              data->color_key = (((trans_color[0].red & 0xff00) << 8) |
                                                 ((trans_color[0].green & 0xff00)) |
                                                 ((trans_color[0].blue & 0xff00) >> 8));
                              break;
                        }

                    D_INFO("[DFB] data->color_key =0x%08x (%s, %d)\n", data->color_key,  __FUNCTION__, __LINE__);
               }
          }
     }

     switch (data->color_type) {
          case PNG_COLOR_TYPE_PALETTE: {
               png_colorp palette    = data->info_ptr->palette;
               png_bytep  trans      = data->info_ptr->trans_alpha;
               int        num_trans  = data->info_ptr->num_trans;
               int        num_colors = MIN( MAXCOLORMAPSIZE, data->info_ptr->num_palette );

               for (i=0; i<num_colors; i++) {
                    data->colors[i].a = (i < num_trans) ? trans[i] : 0xff;
                    data->colors[i].r = palette[i].red;
                    data->colors[i].g = palette[i].green;
                    data->colors[i].b = palette[i].blue;

                    data->palette[i] = PIXEL_ARGB( data->colors[i].a,
                                                   data->colors[i].r,
                                                   data->colors[i].g,
                                                   data->colors[i].b );
               }

               data->pitch = (data->width + 7) & ~7;
               break;
          }

          case PNG_COLOR_TYPE_GRAY:
               data->pitch = data->width;

               if (data->bpp == 16)
                    png_set_strip_16( data->png_ptr );

               break;

          case PNG_COLOR_TYPE_GRAY_ALPHA:
               png_set_gray_to_rgb( data->png_ptr );
               /* fall through */

          default:
               data->pitch = data->width * 4;

               if (data->bpp == 16)
                    png_set_strip_16( data->png_ptr );

#ifdef WORDS_BIGENDIAN
               if (!(data->color_type & PNG_COLOR_MASK_ALPHA))
                    png_set_filler( data->png_ptr, 0xFF, PNG_FILLER_BEFORE );

               png_set_swap_alpha( data->png_ptr );
#else
               if (!(data->color_type & PNG_COLOR_MASK_ALPHA))
                    png_set_filler( data->png_ptr, 0xFF, PNG_FILLER_AFTER );

               png_set_bgr( data->png_ptr );
#endif
               break;
     }

     png_set_interlace_handling( data->png_ptr );

     /* Update the info to reflect our transformations */
     png_read_update_info( data->png_ptr, data->info_ptr );
}

/* Called for each row; note that you will get duplicate row numbers
   for interlaced PNGs */
static void
png_row_callback( png_structp png_read_ptr,
                  png_bytep   new_row,
                  png_uint_32 row_num,
                  int         pass_num )
{
     IDirectFBImageProvider_PNG_data *data;

     data = png_get_progressive_ptr( png_read_ptr );

     /* error stage? */
     if (data->stage < 0)
          return;

     /* set image decoding stage */
     data->stage = STAGE_IMAGE;

     /* check image data pointer */
     if (!data->image) {
          // FIXME: allocates four additional bytes because the scaling functions
          //        in src/misc/gfx_util.c have an off-by-one bug which causes
          //        segfaults on darwin/osx (not on linux)                
          int size = data->pitch * data->height + 4;

          /* allocate image data */
          data->image = D_CALLOC( 1, size );
          if (!data->image) {
               D_ERROR("DirectFB/ImageProvider_PNG: Could not "
                        "allocate %d bytes of system memory!\n", size);

               /* set error stage */
               data->stage = STAGE_ERROR;

               return;
          }
     }

     /* write to image data */
     if (data->color_keyed && data->color_type == PNG_COLOR_TYPE_RGB) {

         png_progressive_combine_row( data->png_ptr, (png_bytep)((u8*)data->image + row_num * data->pitch), new_row );

         int i = 0;

         u32 *dst = (u8*)((u8*)data->image + row_num * data->pitch);

         unsigned int mask = 0;

         mask = 0xffffff;
         /* apply color key for RGB format */
         for(i = 0; i< data->width; i++)
         {
              if( (*dst & mask) == (data->color_key & mask))
                  *dst = 0;

              dst++;
         }
     }
     else
         png_progressive_combine_row( data->png_ptr, (png_bytep)((u8*)data->image + row_num * data->pitch), new_row );

     /* increase row counter, FIXME: interlaced? */
     data->rows++;

     if (data->base.render_callback) {
          DIRenderCallbackResult r;
          DFBRectangle rect = { 0, row_num, data->width, 1 };

          r = data->base.render_callback( &rect,
                                          data->base.render_callback_context );
          if (r != DIRCR_OK)
              data->stage = STAGE_ABORT;
     }
}

/* Called after reading the entire image */
static void
png_end_callback   (png_structp png_read_ptr,
                    png_infop   png_info_ptr)
{
     IDirectFBImageProvider_PNG_data *data;

     data = png_get_progressive_ptr( png_read_ptr );

     /* error stage? */
     if (data->stage < 0)
          return;

     /* set end stage */
     data->stage = STAGE_END;
}

/* Pipes data into libpng until stage is different from the one specified. */
static DFBResult
push_data_until_stage (IDirectFBImageProvider_PNG_data *data,
                       int                              stage,
                       int                              buffer_size)
{
     DFBResult            ret;
     IDirectFBDataBuffer *buffer = data->base.buffer;

     while (data->stage < stage) {
          unsigned int  len;
          unsigned char buf[buffer_size];

          if (data->stage < 0)
               return DFB_FAILURE;

          while (buffer->HasData( buffer ) == DFB_OK) {
               D_DEBUG( "ImageProvider/PNG: Retrieving data (up to %d bytes)...\n", buffer_size );

               ret = buffer->GetData( buffer, buffer_size, buf, &len );
               if (ret)
                    return ret;

               D_DEBUG( "ImageProvider/PNG: Got %d bytes...\n", len );

               png_process_data( data->png_ptr, data->info_ptr, buf, len );

               D_DEBUG( "ImageProvider/PNG: ...processed %d bytes.\n", len );

               /* are we there yet? */
               if (data->stage < 0 || data->stage >= stage) {
                   switch (data->stage) {
                        case STAGE_ABORT: return DFB_INTERRUPTED;
                        case STAGE_ERROR: return DFB_FAILURE;
                        default:          return DFB_OK;
                   }
               }
          }

          D_DEBUG( "ImageProvider/PNG: Waiting for data...\n" );

          if (buffer->WaitForData( buffer, 1 ) == DFB_EOF)
               return DFB_FAILURE;
     }

     return DFB_OK;
}
