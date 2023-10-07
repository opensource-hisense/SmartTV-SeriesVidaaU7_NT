/*
   Copyright (C) 2006 Claudio Ciccani <klan@users.sf.net>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>

#include <display/idirectfbsurface.h>

#include <media/idirectfbimageprovider.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/surface.h>

#include <gfx/convert.h>

#include <misc/gfx_util.h>
#include <misc/util.h>

#include <direct/types.h>
#include <direct/messages.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/interface.h>


static DFBResult Probe( IDirectFBImageProvider_ProbeContext *ctx );

static DFBResult Construct( IDirectFBImageProvider *thiz,
                            IDirectFBDataBuffer    *buffer );

#include <direct/interface_implementation.h>

#define BMP_HEADER_SIZE   64


DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBImageProvider, BMP )


typedef enum {
     BMPIC_NONE      = 0,
     BMPIC_RLE8      = 1,
     BMPIC_RLE4      = 2,
     BMPIC_BITFIELDS = 3
} BMPImageCompression;

typedef struct {
    u32 redBits;
    u32 greenBits;
    u32 blueBits;
} bmp_color_bits;

typedef struct {
    int redShift;
    int greenShift;
    int blueShift;
} bmp_color_shift_bits;

typedef struct {
     IDirectFBImageProvider_data base;

     int                    width;
     int                    height;
     int                    depth;
     bool                   indexed;
     bool                   invert;
     bool                   rledecode;
     bmp_color_shift_bits   right_shift;
     bmp_color_shift_bits   left_shift;
     bmp_color_bits         color_bits;

     BMPImageCompression    compression;
     unsigned int           img_offset;
     unsigned long long     num_colors;

     DFBColor              *palette;

     u32                 *image;

     DIRenderCallback       render_callback;
     void                  *render_callback_ctx;
} IDirectFBImageProvider_BMP_data;

/*****************************************************************************/

static DFBResult
fetch_data( IDirectFBDataBuffer *buffer, void *ptr, int len )
{
     DFBResult ret;

     while (len > 0) {
          unsigned int read = 0;

          ret = buffer->WaitForData( buffer, len );
          if (ret == DFB_OK)
               ret = buffer->GetData( buffer, len, ptr, &read );

          if (ret)
               return ret;

          ptr += read;
          len -= read;
     }

     return DFB_OK;
}

static int
CalcShiftRight(u32 mask)
{
     int ret = 0;
     while(mask != 0 && !(mask & 1))
     {
          mask >>= 1;
          ret++;
     }
     return ret;
}

static int
CalcShiftLeft(u32 mask)
{
      int ret = 0;
      while(mask != 0 && !(mask & 1))
      {
           mask >>= 1;
      }
      while (mask != 0 && !(mask & 0x80))
      {
           mask <<= 1;
           ret++;
      }
      return ret;
}

static DFBResult
bmp_decode_header( IDirectFBImageProvider_BMP_data *data )
{
     DFBResult ret = 0;
     u8        buf[54];
     u32       tmp;
     u32       bihsize;

     memset( buf, 0, sizeof(buf) );

     ret = fetch_data( data->base.buffer, buf, sizeof(buf) );
     if (ret)
          return ret;

     /* 2 bytes: Magic */
     if (buf[0] != 'B' && buf[1] != 'M') {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Invalid magic (%c%c)!\n", buf[0], buf[1] );
          return DFB_UNSUPPORTED;
     }

     /* 4 bytes: FileSize */

     /* 4 bytes: Reserved */

     /* 4 bytes: DataOffset */
     data->img_offset = buf[10] | (buf[11]<<8) | (buf[12]<<16) | (buf[13]<<24);
     if (data->img_offset < 54) {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Invalid offset %08x!\n", data->img_offset );
          return DFB_UNSUPPORTED;
     }

     /* 4 bytes: HeaderSize */
     bihsize = buf[14] | (buf[15]<<8) | (buf[16]<<16) | (buf[17]<<24);
     if (bihsize < 40) {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Invalid image header size %d!\n", bihsize );
          return DFB_UNSUPPORTED;
     }

     /* 4 bytes: Width */
     data->width = buf[18] | (buf[19]<<8) | (buf[20]<<16) | (buf[21]<<24);
     if (data->width < 1 || data->width > 0xffff) {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Invalid width %d!\n", data->width );
          return DFB_UNSUPPORTED;
     }

     /* 4 bytes: Height */
     data->height = buf[22] | (buf[23]<<8) | (buf[24]<<16) | (buf[25]<<24);
     if (data->height > 0xffff) {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Invalid height %d!\n", data->height );
          return DFB_UNSUPPORTED;
     }
     if(data->height < 1)
     {
          data->height = -data->height;
          data->invert = true;
          printf( "IDirectFBImageProvider_BMP: "
                   "Negative height %d!\n", data->height );
     }

     /* 2 bytes: Planes */
     tmp = buf[26] | (buf[27]<<8);
     if (tmp != 1) {
          D_ERROR( "IDirectFBImageProvider_BMP: "
                   "Unsupported number of planes %d!\n", tmp );
          return DFB_UNSUPPORTED;
     }

     /* 2 bytes: Depth */
     data->depth = buf[28] | (buf[29]<<8);
     switch (data->depth) {
          case 1:
          case 4:
          case 8:
               data->indexed = true;
          case 16:
          case 24:
          case 32:
               break;
          default:
               D_ERROR( "IDirectFBImageProvider_BMP: "
                        "Unsupported depth %d!\n", data->depth );
               return DFB_UNSUPPORTED;
     }

     /* 4 bytes: Compression */
     data->compression = buf[30] | (buf[31]<<8) | (buf[32]<<16) | (buf[33]<<24);
     switch (data->compression) {
          case BMPIC_NONE:
          //case BMPIC_RLE8:
          //case BMPIC_RLE4:
          //case BMPIC_BITFIELDS:
               break;
          default:

               printf( "IDirectFBImageProvider_BMP: "
                        "Bmp compression %d!\n", data->compression );
               if(data->compression == 1 || data->compression ==2)
                   data->rledecode = true;
     }

     /* 4 bytes: CompressedSize */

     /* 4 bytes: HorizontalResolution */

     /* 4 bytes: VerticalResolution */

     /* 4 bytes: UsedColors */
     data->num_colors = buf[46] | (buf[47]<<8) | (buf[48]<<16) | ((unsigned long long)buf[49]<<24);
     if (!data->num_colors)
          data->num_colors = (unsigned long long)1 << data->depth;

     /* 4 bytes: ImportantColors */

     /* Skip remaining bytes */

     if (bihsize == BMP_HEADER_SIZE) {
          bihsize -= 40;       //skip 24 bytes
          while (bihsize--) {
               u8 b;
               ret = fetch_data( data->base.buffer, &b, 1 );
               if (ret)
                    return ret;
          }
     }

     /* Palette */
     if (data->indexed) {
          void *src;
          int   i,j;

          data->palette = src = D_MALLOC( 256 *4 );
          memset(src, 0, 256*4);
          if (!data->palette)
               return D_OOM();

          ret = fetch_data( data->base.buffer, src, data->num_colors*4 );
          if (ret)
               return ret;

          for (i = 0; i < data->num_colors; i++) {
               DFBColor c;

               c.a = 0xff;
               c.r = ((u8*)src)[i*4+2];
               c.g = ((u8*)src)[i*4+1];
               c.b = ((u8*)src)[i*4+0];
               data->palette[i] = c;

           switch (data->num_colors) {
                    case 2:
                         for (j = 0; j < 8; j++)
                              data->palette[i << j] = c;
                         break;
                    case 4:
                         data->palette[i] = c;
                         data->palette[i << 4] = c;
                         break;
               }
          }
     }

     if(data->compression ==3)
     {
          u32 color[3];
          memset(color, 0, sizeof(color));
          fetch_data( data->base.buffer, color, sizeof(color));

          data->color_bits.redBits = color[0] & 0xffff;
          data->color_bits.greenBits = color[1] & 0xffff;
          data->color_bits.blueBits = color[2] & 0xffff;
          data->right_shift.redShift = CalcShiftRight(data->color_bits.redBits);
          data->right_shift.greenShift = CalcShiftRight(data->color_bits.greenBits);
          data->right_shift.blueShift = CalcShiftRight(data->color_bits.blueBits);
          data->left_shift.redShift = CalcShiftLeft(data->color_bits.redBits);
          data->left_shift.greenShift = CalcShiftLeft(data->color_bits.greenBits);
          data->left_shift.blueShift = CalcShiftLeft(data->color_bits.blueBits);
      }

     return DFB_OK;
}

static void
PutPixel(int x, int y, IDirectFBImageProvider_BMP_data *data, u8 col)
{
     u32    *dst;
     dst  = data->image;

     if(data->invert)
           y = data->height- (y + 1);

     int base = (y * data->width) + x;
     DFBColor c   = data->palette[col];
     dst[base] = c.b | (c.g << 8) | (c.r << 16) | (c.a << 24);
}

static void
DoRLEDecode(IDirectFBImageProvider_BMP_data *data)
{
       static const u8 RLE_ESCAPE = 0;
       static const u8 RLE_EOL = 0;
       static const u8 RLE_EOF = 1;
       static const u8 RLE_DELTA = 2;
       int x = 0;
       int y = data->height- 1;
       int pos=0;
       int len = ((((data->width*data->depth + 7) >> 3) + 3) & ~3) * (data->height);
       u8  *buf = NULL;
       buf = (u8 *)D_MALLOC(len*sizeof(u8));
       memset( buf, 0, len*sizeof(u8));
       fetch_data( data->base.buffer, buf, len );

       while (pos< len - 1)
       {
              u8 cmd = buf[pos++];
              if (cmd != RLE_ESCAPE)
              {
                    u8 pixels = buf[pos++];
                    int num = 0;
                    int j=0;
                    u8 col = pixels;
                    DFBColor c;
                    while (cmd-- && x < data->width)
                    {
                           if(data->depth == 4)
                           {
                                if(num & 1)
                                     col = pixels & 0xf;
                                else
                                     col = pixels >> 4;
                           }
                          PutPixel(x++, y, data, col);
                          num++;
                    }
               }
               else
               {
                    cmd = buf[pos++];
                    if (cmd == RLE_EOF)
                    {
                       D_FREE(buf);
                       buf = NULL;
                       return;
                    }
                    else if (cmd == RLE_EOL)
                    {
                         x = 0;
                         y--;
                         if (y < 0)
                         {
                             D_FREE(buf);
                             buf = NULL;
                             return;
                         }
                    }
                    else if (cmd == RLE_DELTA)
                    {
                          if (pos< len - 1)
                          {
                                u8 dx = buf[pos++];
                                u8 dy = buf[pos++];
                                x += dx;
                                if (x > data->width)
                                {
                                      x = data->width;
                                }
                                y -= dy;
                                if (y < 0)
                                {
                                   D_FREE(buf);
                                   buf = NULL;
                                   return;
                                }
                           }
                     }
                    else
                    {
                          int num = 0;
                          int bytesRead = 0;
                          u8 val = 0;
                          while (cmd-- && pos< len)
                          {
                                if (data->depth == 8 || !(num & 1))
                                {
                                      val = buf[pos++];
                                      bytesRead++;
                                }
                                u8 col = val;
                                if (data->depth == 4)
                                {
                                      if (num & 1)
                                           col = col & 0xf;
                                      else
                                           col >>= 4;
                                 }
                                 if (x < data->width)
                                       PutPixel(x++, y, data, col);
                                 num++;
                            }
                            if((bytesRead & 1) && pos< len)
                            {
                                  buf[pos++];
                            }
                       }
               }
          }
          D_FREE(buf);
          buf = NULL;
    }

static DFBResult
bmp_decode_rgb_row( IDirectFBImageProvider_BMP_data *data, int row )
{
     DFBResult  ret;
     int        pitch = (((data->width*data->depth + 7) >> 3) + 3) & ~3;
     u8         buf[pitch];
     u32       *dst;
     int        i;

     //Coverity-03132013
     memset( buf, 0, pitch );

     ret = fetch_data( data->base.buffer, buf, pitch );
     if (ret)
          return ret;

     dst = data->image + row * data->width;
     u8 currVal =0;
     u8 col=0;

     switch (data->depth) {
          case 1:
               for (i = 0; i < data->width; i++) {
                    unsigned idx = buf[i>>3] & (0x80 >> (i&7));
                    DFBColor c   = data->palette[idx];
                    dst[i] = c.b | (c.g << 8) | (c.r << 16) | (c.a << 24);
               }
               break;
          case 4:
               for (i = 0; i < data->width; i++) {
                    if((i%2)==0)
                    {
                         currVal = buf[i>>1];
                         col = currVal>>4;
                    }
                    else
                    {
                         col = currVal & 0xf;
                    }
                    DFBColor c   = data->palette[col];
                    dst[i] = c.b | (c.g << 8) | (c.r << 16) | (c.a << 24);
               }
               break;
          case 8:
               for (i = 0; i < data->width; i++) {
                    DFBColor c = data->palette[buf[i]];
                    dst[i] = c.b | (c.g << 8) | (c.r << 16) | (c.a << 24);
               }
               break;
          case 16:
               for (i = 0; i < data->width; i++) {
                    u32 r, g, b;
                    u16 c;

                    if(data->compression !=3)
                    {
                          c = buf[i*2+0] | (buf[i*2+1]<<8);
                          r = (c >> 10) & 0x1f;
                          g = (c >>  5) & 0x1f;
                          b = (c      ) & 0x1f;
                          r = (r << 3) | (r >> 2);
                          g = (g << 3) | (g >> 2);
                          b = (b << 3) | (b >> 2);
                          dst[i] = b | (g<<8) | (r<<16) | 0xff000000;
                     }
                     else
                     {
                          u16 val = buf[i*2+0] | (buf[i*2+1]<<8);
                          r= ((val & data->color_bits.redBits) >> data->right_shift.redShift) << data->left_shift.redShift;
                          g = ((val & data->color_bits.greenBits) >> data->right_shift.greenShift) << data->left_shift.greenShift;
                          b = ((val & data->color_bits.blueBits) >> data->right_shift.blueShift) << data->left_shift.blueShift;
                          dst[i] = b | (g<<8) | (r<<16) | 0xff000000;
                      }
               }
               break;
          case 24:
               for (i = 0; i < data->width; i++) {
                    dst[i] = (buf[i*3+0]    ) |
                             (buf[i*3+1]<< 8) |
                             (buf[i*3+2]<<16) |
                             0xff000000;
               }
               break;
          case 32:
               for (i = 0; i < data->width; i++) {
                    dst[i] = (buf[i*4+0]    ) |
                             (buf[i*4+1]<< 8) |
                             (buf[i*4+2]<<16) |
                             0xff000000;
               }
               break;
          default:
               break;
     }

     return DFB_OK;
}

/*****************************************************************************/

static void
IDirectFBImageProvider_BMP_Destruct( IDirectFBImageProvider *thiz )
{
     IDirectFBImageProvider_BMP_data *data = thiz->priv;

     if (data->base.buffer)
          data->base.buffer->Release( data->base.buffer );

     if (data->image)
          D_FREE( data->image );

     if (data->palette)
          D_FREE( data->palette );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DFBResult
IDirectFBImageProvider_BMP_AddRef( IDirectFBImageProvider *thiz )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     data->base.ref++;

     return DFB_OK;
}

static DFBResult
IDirectFBImageProvider_BMP_Release( IDirectFBImageProvider *thiz )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     if (--data->base.ref == 0)
           IDirectFBImageProvider_BMP_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBImageProvider_BMP_RenderTo( IDirectFBImageProvider *thiz,
                                     IDirectFBSurface       *destination,
                                     const DFBRectangle     *dest_rect )
{
     IDirectFBSurface_data  *dst_data;
     CoreSurface            *dst_surface;
     CoreSurfaceBufferLock   lock;
     DFBRectangle            rect;
     DFBRegion               clip;
     DIRenderCallbackResult  cb_result = DIRCR_OK;
     DFBResult               ret       = DFB_OK;

     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     if (!destination)
          return DFB_INVARG;

     dst_data = destination->priv;
     if (!dst_data || !dst_data->surface)
          return DFB_DESTROYED;

     dst_surface = dst_data->surface;

     if (dest_rect) {
          if (dest_rect->w < 1 || dest_rect->h < 1)
               return DFB_INVARG;
          rect = *dest_rect;
          rect.x += dst_data->area.wanted.x;
          rect.y += dst_data->area.wanted.y;
     }
     else {
          rect = dst_data->area.wanted;
     }

     dfb_region_from_rectangle( &clip, &dst_data->area.current );
     if (!dfb_rectangle_region_intersects( &rect, &clip ))
          return DFB_OK;

     ret = dfb_surface_lock_buffer( dst_surface, CSBR_BACK, CSAID_CPU, CSAF_WRITE, &lock );
     if (ret)
          return ret;

     if (!data->image) {
          bool direct = (rect.w == data->width  &&
                         rect.h == data->height &&
                         data->render_callback);
          int  y;

          if (data->indexed && dst_surface->config.format == DSPF_LUT8) {
               IDirectFBPalette *palette;

               ret = destination->GetPalette( destination, &palette );
               if (ret) {
                    dfb_surface_unlock_buffer( dst_surface, &lock );
                    return ret;
               }

               palette->SetEntries( palette, data->palette, data->num_colors, 0 );
               palette->Release( palette );
          }

          data->image = D_MALLOC( data->width*data->height*4 );
          if (!data->image) {
               dfb_surface_unlock_buffer( dst_surface, &lock );
               return D_OOM();
          }

          data->base.buffer->SeekTo( data->base.buffer, data->img_offset );

          if(!data->rledecode)
          {
               for(y = data->height-1; y >= 0 && cb_result == DIRCR_OK; y--) {
                     if(data->invert)
                          ret = bmp_decode_rgb_row( data, data->height-1-y);
                     else
                          ret = bmp_decode_rgb_row( data, y);
                     if (ret)
                          break;

                     if (direct) {
                           DFBRectangle r = { rect.x, rect.y+y, data->width, 1 };

                    dfb_copy_buffer_32( data->image+y*data->width,
                                        lock.addr, lock.pitch, &r, dst_surface, &clip );

                    if (data->render_callback) {
                         r = (DFBRectangle) { 0, y, data->width, 1 };
                         cb_result = data->render_callback( &r,
                                             data->render_callback_ctx );
                    }
               }
            }
        }
        else
        {
              DoRLEDecode(data);
              if (direct) {
                   DFBRectangle r = { rect.x, rect.y, data->width, data->height };

                   dfb_copy_buffer_32( data->image,
                                      lock.addr, lock.pitch, &r, dst_surface, &clip );

                   if (data->render_callback) {
                         r = (DFBRectangle) { 0, 0, data->width, data->height };
                         cb_result = data->render_callback( &r,
                                            data->render_callback_ctx );
                   }
               }
            }

          if (!direct) {
               dfb_scale_linear_32( data->image, data->width, data->height,
                                    lock.addr, lock.pitch, &rect, dst_surface, &clip );

               if (data->render_callback) {
                    DFBRectangle r = { 0, 0, data->width, data->height };
                    data->render_callback( &r, data->render_callback_ctx );
               }
          }

          if (cb_result == DIRCR_OK) {
               data->base.buffer->Release( data->base.buffer );
               data->base.buffer = NULL;
          }
          else {
               D_FREE( data->image );
               data->image = NULL;
               ret = DFB_INTERRUPTED;
          }
     }
     else {
          dfb_scale_linear_32( data->image, data->width, data->height,
                               lock.addr, lock.pitch, &rect, dst_surface, &clip );

          if (data->render_callback) {
               DFBRectangle r = {0, 0, data->width, data->height};
               data->render_callback( &r, data->render_callback_ctx );
          }
     }

     dfb_surface_unlock_buffer( dst_surface, &lock );

     return ret;
}

static DFBResult
IDirectFBImageProvider_BMP_SetRenderCallback( IDirectFBImageProvider *thiz,
                                              DIRenderCallback        callback,
                                              void                   *ctx )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     data->render_callback     = callback;
     data->render_callback_ctx = ctx;

     return DFB_OK;
}

static DFBResult
IDirectFBImageProvider_BMP_GetSurfaceDescription( IDirectFBImageProvider *thiz,
                                                  DFBSurfaceDescription  *desc )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     if (!desc)
          return DFB_INVARG;

     desc->flags       = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
     desc->width       = data->width;
     desc->height      = data->height;
     desc->pixelformat = (data->indexed) ? DSPF_LUT8 : DSPF_RGB32;

     return DFB_OK;
}

static DFBResult
IDirectFBImageProvider_BMP_GetImageDescription( IDirectFBImageProvider *thiz,
                                                DFBImageDescription    *desc )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBImageProvider_BMP )

     if (!desc)
          return DFB_INVARG;

     desc->caps = DICAPS_NONE;

     return DFB_OK;
}

/* exported symbols */

static DFBResult
Probe( IDirectFBImageProvider_ProbeContext *ctx )
{
     if (ctx->header[0] == 'B' && ctx->header[1] == 'M')
          return DFB_OK;

     return DFB_UNSUPPORTED;
}

static DFBResult
Construct( IDirectFBImageProvider *thiz,
           IDirectFBDataBuffer    *buffer )
{
     DFBResult ret;

     DIRECT_ALLOCATE_INTERFACE_DATA( thiz, IDirectFBImageProvider_BMP )

     data->base.ref    = 1;
     data->base.buffer = buffer;

     buffer->AddRef( buffer );

     ret = bmp_decode_header( data );
     if (ret) {
          IDirectFBImageProvider_BMP_Destruct( thiz );
          return ret;
     }

     thiz->AddRef                = IDirectFBImageProvider_BMP_AddRef;
     thiz->Release               = IDirectFBImageProvider_BMP_Release;
     thiz->RenderTo              = IDirectFBImageProvider_BMP_RenderTo;
     thiz->SetRenderCallback     = IDirectFBImageProvider_BMP_SetRenderCallback;
     thiz->GetImageDescription   = IDirectFBImageProvider_BMP_GetImageDescription;
     thiz->GetSurfaceDescription = IDirectFBImageProvider_BMP_GetSurfaceDescription;

     return DFB_OK;
}

