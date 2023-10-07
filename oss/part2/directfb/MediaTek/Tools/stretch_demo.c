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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <gfx/convert.h>


#define IMAGEDIR "images"

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                               \
               err = x;                                                              \
               if (err != DFB_OK) {                                                  \
                    D_DERROR( err, "%s failed! (%s:%d)\n", #x, __FILE__, __LINE__ ); \
                    goto out;                                                        \
               }


typedef void (*StretchFunc)( void       *dst,
                             int         dpitch,
                             const void *src,
                             int         spitch,
                             int         width,
                             int         height,
                             int         dst_width,
                             int         dst_height );

typedef void (*StretchIndexedFunc)( void       *dst,
                                    int         dpitch,
                                    const void *src,
                                    int         spitch,
                                    int         width,
                                    int         height,
                                    int         dst_width,
                                    int         dst_height,
                                    const void *palette );

typedef struct {
     const char         *name;
     const char         *description;

     StretchFunc         func_rgb16;
     StretchFunc         func_argb4444;

     /* FIXME: Only RGB16 supported for conversion right now. */
     StretchIndexedFunc  func_rgb16_indexed;
     StretchFunc         func_rgb16_from32;
} StretchAlgo;

typedef enum {
     ZOOM_NONE,
     ZOOM_FULL,
     ZOOM_640x360,
     ZOOM_426x240,

     ZOOM_FIRST = ZOOM_NONE,
     ZOOM_LAST  = ZOOM_426x240
} ZoomMode;

typedef enum {
     RES_NATIVE,
     RES_1280x720,
     RES_1024x768,
     RES_1024x576,
     RES_852x480,
     RES_640x480,

     _RES_NUM
} Resolution;

/**********************************************************************************************************************/

static IDirectFB *m_dfb;
static int        m_width;
static int        m_height;
static int        m_orig_width;
static int        m_orig_height;
static int        m_diff;
static bool       m_load32;

/**********************************************************************************************************************/

static inline long myclock( void )
{
     struct timeval tv;

     gettimeofday (&tv, NULL);

     return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void print_usage( void )
{
     printf ("StretchBlit Demo\n\n");
     printf ("Usage: stretch_demo [options]\n\n");
     printf ("Options:\n\n");
     printf ("  --duration <milliseconds>    Duration of each benchmark.\n");
     printf ("  --help                       Print usage information.\n");
     printf ("  --dfb-help                   Output DirectFB usage information.\n\n");
}

/**********************************************************************************************************************/

static void stretch_simple( void        *dst,
                            int          dpitch,
                            const void  *src,
                            int          spitch,
                            int          width,
                            int          height,
                            int          dst_width,
                            int          dst_height )
{
    int x, y;
    int w2     = dst_width / 2;
    int hfraq  = (width  << 20) / dst_width;
    int vfraq  = (height << 20) / dst_height;
    int line   = 0;
    int hfraq2 = hfraq << 1;

    for (y=0; y<dst_height; y++) {
         int point = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              dst32[x] = src16[point>>20] | (src16[(point+hfraq)>>20] << 16);

              point += hfraq2;
         }

         line += vfraq;
    }
}

static void stretch_simple_rgb16_indexed( void        *dst,
                                          int          dpitch,
                                          const void  *src,
                                          int          spitch,
                                          int          width,
                                          int          height,
                                          int          dst_width,
                                          int          dst_height,
                                          const void  *palette )
{
    int x, y;
    int w2     = dst_width / 2;
    int hfraq  = (width  << 20) / dst_width;
    int vfraq  = (height << 20) / dst_height;
    int line   = 0;
    int hfraq2 = hfraq << 1;

    const __u16 *lookup = palette;

    for (y=0; y<dst_height; y++) {
         int point = 0;

         __u32      *dst32 = dst + dpitch * y;
         const __u8 *src8  = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              dst32[x] = lookup[ src8[point>>20] ] | (lookup[ src8[(point+hfraq)>>20] ] << 16);

              point += hfraq2;
         }

         line += vfraq;
    }
}

#define PIXEL_RGB32TO16(p)      ((((p) >> 8) & 0xf800) | (((p) >> 5) & 0x07e0) | (((p) >> 3) & 0x001f))

static void stretch_simple_rgb16_from32( void        *dst,
                                         int          dpitch,
                                         const void  *src,
                                         int          spitch,
                                         int          width,
                                         int          height,
                                         int          dst_width,
                                         int          dst_height )
{
    int x, y;
    int w2     = dst_width / 2;
    int hfraq  = (width  << 20) / dst_width;
    int vfraq  = (height << 20) / dst_height;
    int line   = 0;
    int hfraq2 = hfraq << 1;

    for (y=0; y<dst_height; y++) {
         int point = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u32 *src32 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              dst32[x] = PIXEL_RGB32TO16(src32[point>>20]) | (PIXEL_RGB32TO16(src32[(point+hfraq)>>20]) << 16);

              point += hfraq2;
         }

         line += vfraq;
    }
}

static void stretch_down1_rgb16( void        *dst,
                                 int          dpitch,
                                 const void  *src,
                                 int          spitch,
                                 int          width,
                                 int          height,
                                 int          dst_width,
                                 int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);

     __u32 linecache[dst_width/2];

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u32 dp;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * (line >> 20);


          for (x=0; x<w2; x++) {
               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];

               point += hfraq;

               dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                    ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);


               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];

               point += hfraq;

               dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                      ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));


               if (y == dst_height - 1)
                    dst32[x] = dp;
               else {
                    dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
               }

               linecache[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down1_rgb16_from32( void        *dst,
                                        int          dpitch,
                                        const void  *src,
                                        int          spitch,
                                        int          width,
                                        int          height,
                                        int          dst_width,
                                        int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);

     __u32 linecache[dst_width/2];

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u32 s1;
          __u32 s2;
          __u32 d1;
          __u32 d2;
          __u32 dp;

          __u32       *dst32 = dst + dpitch * y;
          const __u32 *src32 = src + spitch * (line >> 20);


          for (x=0; x<w2; x++) {
               s1 = src32[point>>20];
               s2 = src32[(point>>20) + 1];

               point += hfraq;

               d1 = ((s2 & 0xfcfcfc) + (s1 & 0xfcfcfc)) >> 1;


               s1 = src32[point>>20];
               s2 = src32[(point>>20) + 1];

               dp = PIXEL_RGB32TO16(d1);


               point += hfraq;

               d2 = ((s2 & 0xfcfcfc) + (s1 & 0xfcfcfc)) >> 1;

               dp |= PIXEL_RGB32TO16(d2) << 16;



               if (y == dst_height - 1)
                    dst32[x] = dp;
               else {
                    dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
               }

               linecache[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down1_rgb16_indexed( void        *dst,
                                         int          dpitch,
                                         const void  *src,
                                         int          spitch,
                                         int          width,
                                         int          height,
                                         int          dst_width,
                                         int          dst_height,
                                         const void  *palette )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);

     const __u16 *lookup = palette;

     __u32 linecache[dst_width/2];

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u32 dp;

          __u32      *dst32 = dst + dpitch * y;
          const __u8 *src8  = src + spitch * (line >> 20);


          for (x=0; x<w2; x++) {
               s1 = lookup[ src8[point>>20] ];
               s2 = lookup[ src8[(point>>20) + 1] ];

               point += hfraq;

               dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                    ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);


               s1 = lookup[ src8[point>>20] ];
               s2 = lookup[ src8[(point>>20) + 1] ];

               point += hfraq;

               dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                      ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));


               if (y == dst_height - 1)
                    dst32[x] = dp;
               else
                    dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));

               linecache[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down1_argb4444( void        *dst,
                                    int          dpitch,
                                    const void  *src,
                                    int          spitch,
                                    int          width,
                                    int          height,
                                    int          dst_width,
                                    int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);

     __u32 linecache[dst_width/2];

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u32 dp;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * (line >> 20);


          for (x=0; x<w2; x++) {
               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];

               point += hfraq;

               dp = ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0) |
                    ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f);


               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];

               point += hfraq;

               dp |= (((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000) |
                      ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000));


               if (y == dst_height - 1)
                    dst32[x] = dp;
               else
                    dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 1) & 0x0f0f0f0f) |
                                (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 3) & 0xf0f0f0f0));

               linecache[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down2_rgb16( void        *dst,
                                 int          dpitch,
                                 const void  *src,
                                 int          spitch,
                                 int          width,
                                 int          height,
                                 int          dst_width,
                                 int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);
     int sp2   = spitch / 2;

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;
          __u32 g;

          int sy = (line >> 20);

          if (sy > height - 2)
               sy = height - 2;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * sy;


          for (x=0; x<w2; x++) {
               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];
               s3 = src16[(point>>20) + sp2];
               s4 = src16[(point>>20) + sp2 + 1];

               point += hfraq;

               g = ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) >> 2) & 0x07e0);
               if (g == 0x20)
                    g = 0x40;

               dp = g | ((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) >> 2) & 0xf81f);



               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];
               s3 = src16[(point>>20) + sp2];
               s4 = src16[(point>>20) + sp2 + 1];

               point += hfraq;

               g = ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) << 14) & 0x07e00000);
               if (g == 0x200000)
                    g = 0x400000;

               dp |= g | ((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) << 14) & 0xf81f0000);


               dst32[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down2_rgb16_from32( void        *dst,
                                        int          dpitch,
                                        const void  *src,
                                        int          spitch,
                                        int          width,
                                        int          height,
                                        int          dst_width,
                                        int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);
     int sp4   = spitch / 4;

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u32 s1;
          __u32 s2;
          __u32 s3;
          __u32 s4;
          __u32 d1;
          __u32 d2;

          int sy = (line >> 20);

          if (sy > height - 2)
               sy = height - 2;

          __u32       *dst32 = dst + dpitch * y;
          const __u32 *src32 = src + spitch * sy;


          for (x=0; x<w2; x++) {
               s1 = src32[point>>20];
               s2 = src32[(point>>20) + 1];
               s3 = src32[(point>>20) + sp4];
               s4 = src32[(point>>20) + sp4 + 1];

               point += hfraq;

               d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s3 & 0xfcfcfc) + (s4 & 0xfcfcfc)) >> 2;


               s1 = src32[point>>20];
               s2 = src32[(point>>20) + 1];
               s3 = src32[(point>>20) + sp4];
               s4 = src32[(point>>20) + sp4 + 1];

               point += hfraq;

               d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s3 & 0xfcfcfc) + (s4 & 0xfcfcfc)) >> 2;


               dst32[x] = PIXEL_RGB32TO16(d1) | (PIXEL_RGB32TO16(d2) << 16);
          }

          line -= vfraq;
     }
}

static void stretch_down2_rgb16_indexed( void        *dst,
                                         int          dpitch,
                                         const void  *src,
                                         int          spitch,
                                         int          width,
                                         int          height,
                                         int          dst_width,
                                         int          dst_height,
                                         const void  *palette )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);

     const __u16 *lookup = palette;

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;

          int sy = (line >> 20);

          if (sy > height - 2)
               sy = height - 2;

          __u32      *dst32 = dst + dpitch * y;
          const __u8 *src8  = src + spitch * sy;


          for (x=0; x<w2; x++) {
               s1 = lookup[ src8[point>>20] ];
               s2 = lookup[ src8[(point>>20) + 1] ];
               s3 = lookup[ src8[(point>>20) + spitch] ];
               s4 = lookup[ src8[(point>>20) + spitch + 1] ];

               point += hfraq;

               dp = ((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) >> 2) & 0xf81f) |
                    ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) >> 2) & 0x07e0);


               s1 = lookup[ src8[point>>20] ];
               s2 = lookup[ src8[(point>>20) + 1] ];
               s3 = lookup[ src8[(point>>20) + spitch] ];
               s4 = lookup[ src8[(point>>20) + spitch + 1] ];

               point += hfraq;

               dp |= (((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) << 14) & 0xf81f0000) |
                      ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) << 14) & 0x07e00000));


               dst32[x] = dp;
          }

          line -= vfraq;
     }
}

static void stretch_down2_argb4444( void        *dst,
                                    int          dpitch,
                                    const void  *src,
                                    int          spitch,
                                    int          width,
                                    int          height,
                                    int          dst_width,
                                    int          dst_height )
{
     int x, y;
     int w2 = dst_width / 2;
     int hfraq = (width  << 20) / dst_width;
     int vfraq = (height << 20) / dst_height;
     int line  = vfraq * (dst_height - 1);
     int sp2   = spitch / 2;

     for (y=dst_height-1; y>=0; y--) {
          int point = 0;

          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;

          int sy = (line >> 20);

          if (sy > height - 2)
               sy = height - 2;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * sy;


          for (x=0; x<w2; x++) {
               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];
               s3 = src16[(point>>20) + sp2];
               s4 = src16[(point>>20) + sp2 + 1];

               point += hfraq;

               dp = ((((s1 & 0xf0f0) + (s2 & 0xf0f0) + (s3 & 0xf0f0) + (s4 & 0xf0f0)) >> 2) & 0xf0f0) |
                    ((((s1 & 0x0f0f) + (s2 & 0x0f0f) + (s3 & 0x0f0f) + (s4 & 0x0f0f)) >> 2) & 0x0f0f);


               s1 = src16[point>>20];
               s2 = src16[(point>>20) + 1];
               s3 = src16[(point>>20) + sp2];
               s4 = src16[(point>>20) + sp2 + 1];

               point += hfraq;

               dp |= (((((s1 & 0xf0f0) + (s2 & 0xf0f0) + (s3 & 0xf0f0) + (s4 & 0xf0f0)) << 14) & 0xf0f00000) |
                      ((((s1 & 0x0f0f) + (s2 & 0x0f0f) + (s3 & 0x0f0f) + (s4 & 0x0f0f)) << 14) & 0x0f0f0000));


               dst32[x] = dp;
          }

          line -= vfraq;
     }
}

#if 0
static void stretch_down3_rgb16( void        *dst,
                                 int          dpitch,
                                 const void  *src,
                                 int          spitch,
                                 int          width,
                                 int          height,
                                 int          dst_width,
                                 int          dst_height )
{
     int x, sx, y;
     int w2  = dst_width / 2;
     int sp2 = spitch / 2;

     /* prevent segfaults */
     if (width < dst_width * 2 || height < dst_height * 2)
          return;

     for (y=0; y<dst_height; y++) {
          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * y * 2;

          for (x=0, sx=0; x<w2; x++, sx+=4) {
               s1 = src16[sx];
               s2 = src16[sx + 1];
               s3 = src16[sx + sp2];
               s4 = src16[sx + sp2 + 1];

               dp = ((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) >> 2) & 0xf81f) |
                    ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) >> 2) & 0x07e0);


               s1 = src16[sx + 2];
               s2 = src16[sx + 3];
               s3 = src16[sx + sp2 + 2];
               s4 = src16[sx + sp2 + 3];

               dp |= (((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) << 14) & 0xf81f0000) |
                      ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) << 14) & 0x07e00000));


               dst32[x] = dp;
          }
     }
}

static void stretch_down3_rgb16_from32( void        *dst,
                                        int          dpitch,
                                        const void  *src,
                                        int          spitch,
                                        int          width,
                                        int          height,
                                        int          dst_width,
                                        int          dst_height )
{
     int x, sx, y;
     int w2  = dst_width / 2;
     int sp4 = spitch / 4;

     /* prevent segfaults */
     if (width < dst_width * 2 || height < dst_height * 2)
          return;

     for (y=0; y<dst_height; y++) {
          __u32 s1;
          __u32 s2;
          __u32 s3;
          __u32 s4;
          __u32 d1;
          __u32 d2;

          __u32       *dst32 = dst + dpitch * y;
          const __u32 *src32 = src + spitch * y * 2;

          for (x=0, sx=0; x<w2; x++, sx+=4) {
               s1 = src32[sx];
               s2 = src32[sx + 1];
               s3 = src32[sx + sp4];
               s4 = src32[sx + sp4 + 1];

               d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s3 & 0xfcfcfc) + (s4 & 0xfcfcfc)) >> 2;


               s1 = src32[sx + 2];
               s2 = src32[sx + 3];
               s3 = src32[sx + sp4 + 2];
               s4 = src32[sx + sp4 + 3];

               d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s3 & 0xfcfcfc) + (s4 & 0xfcfcfc)) >> 2;


               dst32[x] = PIXEL_RGB32TO16(d1) | (PIXEL_RGB32TO16(d2) << 16);
          }
     }
}

static void stretch_down3_rgb16_indexed( void        *dst,
                                         int          dpitch,
                                         const void  *src,
                                         int          spitch,
                                         int          width,
                                         int          height,
                                         int          dst_width,
                                         int          dst_height,
                                         const void  *palette )
{
     int x, sx, y;
     int w2 = dst_width / 2;

     const __u16 *lookup = palette;

     /* prevent segfaults */
     if (width < dst_width * 2 || height < dst_height * 2)
          return;

     for (y=0; y<dst_height; y++) {
          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;

          __u32      *dst32 = dst + dpitch * y;
          const __u8 *src8  = src + spitch * y * 2;


          for (x=0, sx=0; x<w2; x++, sx+=4) {
               s1 = lookup[ src8[sx] ];
               s2 = lookup[ src8[sx + 1] ];
               s3 = lookup[ src8[sx + spitch] ];
               s4 = lookup[ src8[sx + spitch + 1] ];

               dp = ((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) >> 2) & 0xf81f) |
                    ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) >> 2) & 0x07e0);


               s1 = lookup[ src8[sx + 2] ];
               s2 = lookup[ src8[sx + 3] ];
               s3 = lookup[ src8[sx + spitch + 2] ];
               s4 = lookup[ src8[sx + spitch + 3] ];

               dp |= (((((s1 & 0xf81f) + (s2 & 0xf81f) + (s3 & 0xf81f) + (s4 & 0xf81f)) << 14) & 0xf81f0000) |
                      ((((s1 & 0x07e0) + (s2 & 0x07e0) + (s3 & 0x07e0) + (s4 & 0x07e0)) << 14) & 0x07e00000));


               dst32[x] = dp;
          }
     }
}

static void stretch_down3_argb4444( void        *dst,
                                    int          dpitch,
                                    const void  *src,
                                    int          spitch,
                                    int          width,
                                    int          height,
                                    int          dst_width,
                                    int          dst_height )
{
     int x, sx, y;
     int w2  = dst_width / 2;
     int sp2 = spitch / 2;

     /* prevent segfaults */
     if (width < dst_width * 2 || height < dst_height * 2)
          return;

     for (y=0; y<dst_height; y++) {
          __u16 s1;
          __u16 s2;
          __u16 s3;
          __u16 s4;
          __u32 dp;

          __u32       *dst32 = dst + dpitch * y;
          const __u16 *src16 = src + spitch * y * 2;


          for (x=0, sx=0; x<w2; x++, sx+=4) {
               s1 = src16[sx];
               s2 = src16[sx + 1];
               s3 = src16[sx + sp2];
               s4 = src16[sx + sp2 + 1];

               dp = ((((s1 & 0xf0f0) + (s2 & 0xf0f0) + (s3 & 0xf0f0) + (s4 & 0xf0f0)) >> 2) & 0xf0f0) |
                    ((((s1 & 0x0f0f) + (s2 & 0x0f0f) + (s3 & 0x0f0f) + (s4 & 0x0f0f)) >> 2) & 0x0f0f);


               s1 = src16[sx + 2];
               s2 = src16[sx + 3];
               s3 = src16[sx + sp2 + 2];
               s4 = src16[sx + sp2 + 3];

               dp |= (((((s1 & 0xf0f0) + (s2 & 0xf0f0) + (s3 & 0xf0f0) + (s4 & 0xf0f0)) << 14) & 0xf0f00000) |
                      ((((s1 & 0x0f0f) + (s2 & 0x0f0f) + (s3 & 0x0f0f) + (s4 & 0x0f0f)) << 14) & 0x0f0f0000));


               dst32[x] = dp;
          }
     }
}
#endif

static void stretch_hv4_rgb16( void        *dst,
                               int          dpitch,
                               const void  *src,
                               int          spitch,
                               int          width,
                               int          height,
                               int          dst_width,
                               int          dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              register u32 s1;
              register u32 s2;

              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 2:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 3:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 3:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              register u32 dt = 0;

              if (y == dst_height - 1)
                   dt = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dt = dp;
                             break;
                        case 1:
                             dt = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                   (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dt = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                   (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dt = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                   (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              register u16 l = dt;
              register u16 h = dt >> 16;

              dst32[x] = (((h == 0x20) ? 0x40 : h) << 16) | ((l == 0x20) ? 0x40 : l);

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv4_rgb16_from32( void        *dst,
                                      int          dpitch,
                                      const void  *src,
                                      int          spitch,
                                      int          width,
                                      int          height,
                                      int          dst_width,
                                      int          dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u32 s1;
         __u32 s2;
         __u32 d1 = 0;
         __u32 d2 = 0;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u32 *src32 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        d1 = s1;
                        break;
                   case 1:
                        d1 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 2:
                        d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 3:
                        d1 = s2;
                        break;
              }

              point += hfraq;

              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              dp = PIXEL_RGB32TO16(d1);

              switch ((point >> 18) & 0x3) {
                   case 0:
                        d2 = s1;
                        break;
                   case 1:
                        d2 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 2:
                        d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 3:
                        d2 = s2;
                        break;
              }

              dp |= (PIXEL_RGB32TO16(d2) << 16);

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv4_rgb16_indexed( void        *dst,
                                       int          dpitch,
                                       const void  *src,
                                       int          spitch,
                                       int          width,
                                       int          height,
                                       int          dst_width,
                                       int          dst_height,
                                       const void  *palette )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    const __u16 *lookup = palette;

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32      *dst32 = dst + dpitch * y;
         const __u8 *src8  = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 2:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 3:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 3:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv4_argb4444( void       *dst,
                                  int         dpitch,
                                  const void *src,
                                  int         spitch,
                                  int         width,
                                  int         height,
                                  int         dst_width,
                                  int         dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 2:
                        dp = ((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 3:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 3:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 1) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 3) & 0xf0f0f0f0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

#define COLOR_KEY_PROTECT
/*
 * read source    - 60 ms     no longer valid
 * multiplication - 50 ms     no longer valid
 * vertical scale - 50 ms     no longer valid
 * line cache     - 20 ms
 * key protection - 10 ms
 * write dest     - 25 ms
 *                 215 ms     no longer valid
 *                 250 ms     no longer valid
 *
 *
 * 1st pixel - 80 ms
 * 2nd pixel - 82 ms
 */
static void stretch_hvx_rgb16( void       *dst,
                               int         dpitch,
                               const void *src,
                               int         spitch,
                               int         width,
                               int         height,
                               int         dst_width,
                               int         dst_height )
{
     int x;
     int h      = dst_height;
     int w2     = dst_width / 2;
     int hfraq  = (width  << 20) / dst_width;
     int vfraq  = (height << 20) / dst_height;
     int line   = (h-1) * vfraq;
     int dp4    = dpitch / 4;

     int        point = 0;
     int        last  = (line >> 20) + 1;
     const u16 *src16 = src + spitch * ((last > height - 1) ? height - 1 : last);
     u32       *dst32 = dst + (h-1) * dpitch;
     u32        linecache[w2];

     /* Prefill the line cache. */
     for (x=0; x<w2; x++) {
          u32 X, L, R, dp;

          /* Horizontal interpolation of 1st pixel */
          X = (point >> 14) & 0x3F;
          L = src16[point>>20];
          R = src16[(point>>20) + 1];

          dp = (((((R & 0xf81f)-(L & 0xf81f))*X + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                ((((R & 0x07e0)-(L & 0x07e0))*X + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

          point += hfraq;

          /* Horizontal interpolation of 2nd pixel */
          X = (point >> 14) & 0x3F;
          L = src16[point>>20];
          R = src16[(point>>20) + 1];

          dp |= (((((R & 0xf81f)-(L & 0xf81f))*X + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                 ((((R & 0x07e0)-(L & 0x07e0))*X + ((L & 0x07e0)<<6)) & 0x0001f800)) << 10;

          point += hfraq;

          /* Store pixels in line cache. */
          linecache[x] = dp;
     }

     /* Scale the image. */
     while (h--) {
          point = 0;
          src16 = src + spitch * (line >> 20);

          for (x=0; x<w2; x++) {
               u32 X, L, R, dp;

               /* Horizontal interpolation of 1st pixel */
               L = src16[point>>20];
               R = src16[(point>>20) + 1];
               X = (point >> 14) & 0x3F;

               dp = (((((R & 0xf81f)-(L & 0xf81f))*X + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                     ((((R & 0x07e0)-(L & 0x07e0))*X + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

               point += hfraq;

               /* Horizontal interpolation of 2nd pixel */
               L = src16[point>>20];
               R = src16[(point>>20) + 1];
               X = (point >> 14) & 0x3F;

               dp |= (((((R & 0xf81f)-(L & 0xf81f))*X + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                      ((((R & 0x07e0)-(L & 0x07e0))*X + ((L & 0x07e0)<<6)) & 0x0001f800)) << 10;

               point += hfraq;

               /* Vertical interpolation of both pixels */
               X = (line >> 15) & 0x1F;

#ifdef COLOR_KEY_PROTECT
               u32 dt = ((((((linecache[x] & 0x07e0f81f) - (dp & 0x07e0f81f))*X) >> 5) + (dp & 0x07e0f81f)) & 0x07e0f81f) +
                        ((((((linecache[x]>>5) & 0x07c0f83f) - ((dp>>5) & 0x07c0f83f))*X) + (dp & 0xf81f07e0)) & 0xf81f07e0);

               /* Get two new pixels. */
               u16 l = dt;
               u16 h = dt >> 16;

               /* Write to destination with color key protection */
               dst32[x] = (((h == 0x20) ? 0x40 : h) << 16) | ((l == 0x20) ? 0x40 : l);
#else
               /* Write to destination without color key protection */
               dst32[x] = ((((((linecache[x] & 0x07e0f81f) - (dp & 0x07e0f81f))*X) >> 5) + (dp & 0x07e0f81f)) & 0x07e0f81f) +
                          ((((((linecache[x]>>5) & 0x07c0f83f) - ((dp>>5) & 0x07c0f83f))*X) + (dp & 0xf81f07e0)) & 0xf81f07e0);
#endif

               /* Store pixels in line cache. */
               linecache[x] = dp;
          }

          dst32 -= dp4;



          /*
           * What a great optimization!
           */
          int next = line - vfraq;

          while ((next >> 20) == (line >> 20) && h) {
               h--;

#ifdef COLOR_KEY_PROTECT
               for (x=0; x<w2; x++) {
                    /* Get two new pixels. */
                    u16 l = linecache[x];
                    u16 h = linecache[x] >> 16;

                    /* Write to destination with color key protection */
                    dst32[x] = (((h == 0x20) ? 0x40 : h) << 16) | ((l == 0x20) ? 0x40 : l);
               }
#else
               memcpy( dst32, linecache, dst_width * 2 );
#endif

               dst32 -= dp4;
               next  -= vfraq;
          }

          line = next;
     }
}

static void stretch_hv6_rgb16( void       *dst,
                               int         dpitch,
                               const void *src,
                               int         spitch,
                               int         width,
                               int         height,
                               int         dst_width,
                               int         dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp = s1;
                        break;
                   case 2:
                   case 3:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 4:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);
                        break;
                   case 5:
                   case 6:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 7:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp |= (s1 << 16);
                        break;
                   case 2:
                   case 3:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 4:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));
                        break;
                   case 5:
                   case 6:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 7:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 17) & 0x7) {
                        case 0:
                        case 1:
                             dst32[x] = dp;
                             break;
                        case 2:
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 4:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 5:
                        case 6:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 7:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv6_rgb16_from32( void       *dst,
                                      int         dpitch,
                                      const void *src,
                                      int         spitch,
                                      int         width,
                                      int         height,
                                      int         dst_width,
                                      int         dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u32 s1;
         __u32 s2;
         __u32 d1 = 0;
         __u32 d2 = 0;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u32 *src32 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        d1 = s1;
                        break;
                   case 2:
                   case 3:
                        d1 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 4:
                        d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 1;
                        break;
                   case 5:
                   case 6:
                        d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 7:
                        d1 = s2;
                        break;
              }

              point += hfraq;


              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        d2 = s1;
                        break;
                   case 2:
                   case 3:
                        d2 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 4:
                        d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 1;
                        break;
                   case 5:
                   case 6:
                        d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 7:
                        d2 = s2;
                        break;
              }

              point += hfraq;


              dp = PIXEL_RGB32TO16(d1) | (PIXEL_RGB32TO16(d2) << 16);


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 17) & 0x7) {
                        case 0:
                        case 1:
                             dst32[x] = dp;
                             break;
                        case 2:
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 4:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 5:
                        case 6:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 7:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv6_rgb16_indexed( void       *dst,
                                       int         dpitch,
                                       const void *src,
                                       int         spitch,
                                       int         width,
                                       int         height,
                                       int         dst_width,
                                       int         dst_height,
                                       const void *palette )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    const __u16 *lookup = palette;

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32      *dst32 = dst + dpitch * y;
         const __u8 *src8  = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp = s1;
                        break;
                   case 2:
                   case 3:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 4:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);
                        break;
                   case 5:
                   case 6:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 7:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp |= (s1 << 16);
                        break;
                   case 2:
                   case 3:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 4:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));
                        break;
                   case 5:
                   case 6:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 7:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 17) & 0x7) {
                        case 0:
                        case 1:
                             dst32[x] = dp;
                             break;
                        case 2:
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 4:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 5:
                        case 6:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 7:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv6_argb4444( void       *dst,
                                  int         dpitch,
                                  const void *src,
                                  int         spitch,
                                  int         width,
                                  int         height,
                                  int         dst_width,
                                  int         dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp = s1;
                        break;
                   case 2:
                   case 3:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 4:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 5:
                   case 6:
                        dp = ((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 7:
                        dp = s2;
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 17) & 0x7) {
                   case 0:
                   case 1:
                        dp |= (s1 << 16);
                        break;
                   case 2:
                   case 3:
                        dp |= (((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 4:
                        dp |= (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 5:
                   case 6:
                        dp |= (((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 7:
                        dp |= (s2 << 16);
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 17) & 0x7) {
                        case 0:
                        case 1:
                             dst32[x] = dp;
                             break;
                        case 2:
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 4:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 1) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 3) & 0xf0f0f0f0));
                             break;
                        case 5:
                        case 6:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 7:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv3_rgb16( void        *dst,
                               int          dpitch,
                               const void  *src,
                               int          spitch,
                               int          width,
                               int          height,
                               int          dst_width,
                               int          dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 2:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);
                        break;
                   case 3:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));
                        break;
                   case 3:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv3_rgb16_from32( void        *dst,
                                      int          dpitch,
                                      const void  *src,
                                      int          spitch,
                                      int          width,
                                      int          height,
                                      int          dst_width,
                                      int          dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u32 s1;
         __u32 s2;
         __u32 d1 = 0;
         __u32 d2 = 0;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u32 *src32 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        d1 = s1;
                        break;
                   case 1:
                        d1 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 2:
                        d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 1;
                        break;
                   case 3:
                        d1 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
              }

              point += hfraq;


              s1 = src32[point>>20];
              s2 = src32[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        d2 = s1;
                        break;
                   case 1:
                        d2 = ((s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
                   case 2:
                        d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 1;
                        break;
                   case 3:
                        d2 = ((s1 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc) + (s2 & 0xfcfcfc)) >> 2;
                        break;
              }

              point += hfraq;


              dp = PIXEL_RGB32TO16(d1) | (PIXEL_RGB32TO16(d2) << 16);


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv3_rgb16_indexed( void        *dst,
                                       int          dpitch,
                                       const void  *src,
                                       int          spitch,
                                       int          width,
                                       int          height,
                                       int          dst_width,
                                       int          dst_height,
                                       const void  *palette )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    const __u16 *lookup = palette;

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32      *dst32 = dst + dpitch * y;
         const __u8 *src8  = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
                   case 2:
                        dp = ((((s2 & 0xf81f) + (s1 & 0xf81f)) >> 1) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s1 & 0x07e0)) >> 1) & 0x07e0);
                        break;
                   case 3:
                        dp = ((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) >> 2) & 0xf81f) |
                             ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) >> 2) & 0x07e0);
                        break;
              }

              point += hfraq;


              s1 = lookup[ src8[point>>20] ];
              s2 = lookup[ src8[(point>>20) + 1] ];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0xf81f) + (s1 & 0xf81f)) << 15) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s1 & 0x07e0)) << 15) & 0x07e00000));
                        break;
                   case 3:
                        dp |= (((((s2 & 0xf81f) + (s2 & 0xf81f) + (s2 & 0xf81f) + (s1 & 0xf81f)) << 14) & 0xf81f0000) |
                               ((((s2 & 0x07e0) + (s2 & 0x07e0) + (s2 & 0x07e0) + (s1 & 0x07e0)) << 14) & 0x07e00000));
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 1) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 3) & 0xf81f07e0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (linecache[x] & 0x07e0f81f) + (dp & 0x07e0f81f)) >> 2) & 0x07e0f81f) |
                                         (((((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((linecache[x] >> 4) & 0x0f81f07e) + ((dp >> 4) & 0x0f81f07e)) << 2) & 0xf81f07e0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_hv3_argb4444( void       *dst,
                                  int         dpitch,
                                  const void *src,
                                  int         spitch,
                                  int         width,
                                  int         height,
                                  int         dst_width,
                                  int         dst_height )
{
    int x, y;
    int w2 = dst_width / 2;
    int hfraq = (width  << 20) / dst_width;
    int vfraq = (height << 20) / dst_height;
    int line  = vfraq * (dst_height - 1);

    __u32 linecache[dst_width/2];

    for (y=dst_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = dst + dpitch * y;
         const __u16 *src16 = src + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp = s1;
                        break;
                   case 1:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 2:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 3:
                        dp = ((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
              }

              point += hfraq;


              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dp |= (s1 << 16);
                        break;
                   case 1:
                        dp |= (((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 2:
                        dp |= (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 3:
                        dp |= (((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                               ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
              }

              point += hfraq;


              if (y == dst_height - 1)
                   dst32[x] = dp;
              else {
                   switch ((line >> 18) & 0x3) {
                        case 0:
                             dst32[x] = dp;
                             break;
                        case 1:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 2:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 1) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 3) & 0xf0f0f0f0));
                             break;
                        case 3:
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}


#define POINT_0               hfraq
#define LINE_0                vfraq
#define POINT_TO_RATIO(p,ps)  ( (((((p)) & 0x3ffff) ? : 0x40000) << 6) / (ps) )
#define LINE_TO_RATIO(l,ls)   ( (((((l)) & 0x3ffff) ? : 0x40000) << 5) / (ls) )

#define POINT_L(p,ps)  ( (((p)-1) >> 18) - 1 )
#define POINT_R(p,ps)  ( (((p)-1) >> 18) )

#define LINE_T(l,ls)  ( (((l)-1) >> 18) - 1 )
#define LINE_B(l,ls)  ( (((l)-1) >> 18) )

#define STRETCH_HVX_RGB16     stretch_hvx_rgb16_down

#include "stretch_hvx_rgb16.h"

#undef POINT_0
#undef LINE_0
#undef POINT_TO_RATIO
#undef LINE_TO_RATIO
#undef POINT_L
#undef POINT_R
#undef LINE_T
#undef LINE_B
#undef STRETCH_HVX_RGB16


#define POINT_0               0
#define LINE_0                0
#define POINT_TO_RATIO(p,ps)  ( ((p) & 0x3ffff) >> 12 )
#define LINE_TO_RATIO(l,ls)   ( ((l) & 0x3ffff) >> 13 )

#define POINT_L(p,ps)  ( (((p)) >> 18) )
#define POINT_R(p,ps)  ( (((p)) >> 18) + 1 )

#define LINE_T(l,ls)  ( (((l)) >> 18) )
#define LINE_B(l,ls)  ( (((l)) >> 18) + 1 )

#define STRETCH_HVX_RGB16     stretch_hvx_rgb16_up

#include "stretch_hvx_rgb16.h"

#undef POINT_0
#undef LINE_0
#undef POINT_TO_RATIO
#undef LINE_TO_RATIO
#undef POINT_L
#undef POINT_R
#undef LINE_T
#undef LINE_B
#undef STRETCH_HVX_RGB16


/* FIXME: Only RGB16 supported for indexed conversion right now. */
static const StretchAlgo stretch_algos[] = {
     { "simple", "No interpolation",        stretch_simple,
                                            stretch_simple,
                                            stretch_simple_rgb16_indexed,
                                            stretch_simple_rgb16_from32 },

     { "hv4",    "H/V Interpolation",       stretch_hv4_rgb16,
                                            stretch_hv4_argb4444,
                                            stretch_hv4_rgb16_indexed,
                                            stretch_hv4_rgb16_from32 },

     { "hv3",    "H/V Interpolation 2nd",   stretch_hv3_rgb16,
                                            stretch_hv3_argb4444,
                                            stretch_hv3_rgb16_indexed,
                                            stretch_hv3_rgb16_from32 },

     { "hv6",    "H/V Interpolation 3rd",   stretch_hv6_rgb16,
                                            stretch_hv6_argb4444,
                                            stretch_hv6_rgb16_indexed,
                                            stretch_hv6_rgb16_from32 },

     { "hvx",    "H/V Interpolation 4th",   stretch_hvx_rgb16,
                                            stretch_hv3_argb4444,
                                            stretch_hv3_rgb16_indexed,
                                            stretch_hv3_rgb16_from32 },

     { "hvx-up", "H/V Interpolation 5th",   stretch_hvx_rgb16_up,
                                            stretch_hv3_argb4444,
                                            stretch_hv3_rgb16_indexed,
                                            stretch_hv3_rgb16_from32 },

     { "hvx-down", "H/V Interpolation 6th", stretch_hvx_rgb16_down,
                                            stretch_down2_argb4444,
                                            stretch_down2_rgb16_indexed,
                                            stretch_down2_rgb16_from32 },

     { "down2",  "2x2 Down Scaler",         stretch_down2_rgb16,
                                            stretch_down2_argb4444,
                                            stretch_down2_rgb16_indexed,
                                            stretch_down2_rgb16_from32 },

     { "down1",  "50/50 Down Scaler",       stretch_down1_rgb16,
                                            stretch_down1_argb4444,
                                            stretch_down1_rgb16_indexed,
                                            stretch_down1_rgb16_from32 },

/*
     { "down3",  "2x2 Down Scaler (fast)",  stretch_down3_rgb16,
                                            stretch_down3_argb4444,
                                            stretch_down3_rgb16_indexed,
                                            stretch_down3_rgb16_from32 },*/
};

static const int num_algos = D_ARRAY_SIZE( stretch_algos );

/**********************************************************************************************************************/

static struct dirent **dir_images;
static int             num_images;

static int
filter_entry( const struct dirent *entry )
{
     char                    buf[256];
     IDirectFBImageProvider *provider;

     snprintf( buf, sizeof(buf), "%s/%s", IMAGEDIR, entry->d_name );

     if (m_dfb->CreateImageProvider( m_dfb, buf, &provider ))
          return false;

     provider->Release( provider );

     return true;
}

static DFBResult
scan_for_images()
{
     long long stamp = myclock();

     num_images = scandir( IMAGEDIR, &dir_images, filter_entry, alphasort );

     if (num_images < 0) {
          D_PERROR( "Could not scan image directory '%s'!\n", IMAGEDIR );
          return DFB_FAILURE;
     }

     if (num_images == 0) {
          D_ERROR( "Could not find any image in directory '%s'!\n", IMAGEDIR );
          return DFB_ITEMNOTFOUND;
     }

     D_INFO( "Found %d images in '%s' scanned in %d ms.\n", num_images, IMAGEDIR, (int)(myclock() - stamp) );

     return DFB_OK;
}

/**********************************************************************************************************************/

static struct {
     int                    width;
     int                    height;
     DFBSurfacePixelFormat  format;

     IDirectFBSurface      *surface;

     __u16                  palette[256];
} m_image;

static DFBResult
load_image( int                   index,
            DFBSurfacePixelFormat format )
{
     int                     i;
     DFBResult               ret;
     DFBSurfaceDescription   desc;
     IDirectFBImageProvider *provider;
     IDirectFBSurface       *surface;
     long long               stamp = myclock();
     char                    buf[256];

     D_ASSERT( index >= 0 );
     D_ASSERT( index < num_images - 1 );

     snprintf( buf, sizeof(buf), "%s/%s", IMAGEDIR, dir_images[index]->d_name );

     D_INFO( "Loading '%s'...\n", buf );

     ret = m_dfb->CreateImageProvider( m_dfb, buf, &provider );
     if (ret) {
          D_DERROR( ret, "IDirectFB::CreateImageProvider() for '%s' failed!\n", buf );
          return ret;
     }

     ret = provider->GetSurfaceDescription( provider, &desc );
     if (ret) {
          D_DERROR( ret, "IDirectFBImageProvider::GetSurfaceDescription() for '%s' failed!\n", buf );
          return ret;
     }

     if (desc.flags & DSDESC_PALETTE) {
          for (i=0; i<256; i++) {
               /* FIXME: Only RGB16 supported right now. */
               m_image.palette[i] = PIXEL_RGB16( desc.palette.entries[i].r,
                                                 desc.palette.entries[i].g,
                                                 desc.palette.entries[i].b );
          }

          desc.pixelformat = DSPF_LUT8;
     }
     else
          desc.pixelformat = format;

     ret = m_dfb->CreateSurface( m_dfb, &desc, &surface );
     if (ret) {
          D_DERROR( ret, "IDirectFB::CreateSurface() at %dx%d %s failed!\n",
                    desc.width, desc.height, dfb_pixelformat_name( desc.pixelformat ) );
          provider->Release( provider );
          return ret;
     }

     ret = provider->RenderTo( provider, surface, NULL );
     if (ret) {
          D_DERROR( ret, "IDirectFBImageProvider::GetSurfaceDescription() for '%s' failed!\n", buf );
          provider->Release( provider );
          return ret;
     }

     if (m_image.surface)
          m_image.surface->Release( m_image.surface );

     m_image.width   = desc.width;
     m_image.height  = desc.height;
     m_image.format  = desc.pixelformat;
     m_image.surface = surface;
     
     provider->Release( provider );

     D_INFO( "...loaded %dx%d %s image in %d ms.\n", desc.width, desc.height,
             dfb_pixelformat_name( desc.pixelformat ), (int)(myclock() - stamp) );

     return DFB_OK;
}

/**********************************************************************************************************************/

static void
stretch_blit( IDirectFBSurface  *surface,
              IDirectFBSurface  *source,
              int                x,
              int                y,
              int                width,
              int                height,
              const StretchAlgo *algo )
{
     DFBResult              ret;
     void                  *src;
     int                    spitch;
     void                  *dst;
     int                    dpitch;
     DFBSurfacePixelFormat  src_format;
     DFBSurfacePixelFormat  dst_format;
     int                    src_width;
     int                    src_height;
     long long              stamp;

     x &= ~1;

     source->GetSize( source, &src_width, &src_height );

     source->GetPixelFormat( source, &src_format );
     surface->GetPixelFormat( surface, &dst_format );

     ret = source->Lock( source, DSLF_READ, &src, &spitch );
     if (ret) {
          D_DERROR( ret, "IDirectFBSurface::Lock() on source failed!\n" );
          return;
     }

     ret = surface->Lock( surface, DSLF_WRITE, &dst, &dpitch );
     if (ret) {
          D_DERROR( ret, "IDirectFBSurface::Lock() on destination failed!\n" );
          source->Unlock( source );
          return;
     }

     dst += DFB_BYTES_PER_LINE( dst_format, x ) + y * dpitch;

     stamp = myclock();

     switch (dst_format) {
          case DSPF_RGB16:
               switch (src_format) {
                   case DSPF_RGB16:
                       algo->func_rgb16( dst, dpitch, src, spitch, src_width, src_height, width, height );
                       break;

                   case DSPF_LUT8:
                       algo->func_rgb16_indexed( dst, dpitch, src, spitch, src_width, src_height, width, height, m_image.palette );
                       break;

                   case DSPF_RGB32:
                       algo->func_rgb16_from32( dst, dpitch, src, spitch, src_width, src_height, width, height );
                       break;

                   default:
                       D_BUG( "unsupported source format %s", dfb_pixelformat_name(src_format) );
               }
               break;

          case DSPF_ARGB4444:
               algo->func_argb4444( dst, dpitch, src, spitch, src_width, src_height, width, height );
               break;

          default:
               D_BUG( "unsupported format %s", dfb_pixelformat_name(dst_format) );
               break;
     }

     m_diff = myclock() - stamp;

     source->Unlock( source );
     surface->Unlock( surface );
}

/**********************************************************************************************************************/

static void
draw_text( IDirectFBSurface    *surface,
           const char          *text,
           int                  x,
           int                  y,
           DFBSurfaceTextFlags  flags )
{
     surface->SetColor( surface, 0xff, 0xff, 0xff, 0xff );
//     surface->SetColor( surface, 0x00, 0x00, 0x00, 0xff );
     surface->DrawString( surface, text, -1, x+2, y-2, flags );
     surface->DrawString( surface, text, -1, x-2, y+2, flags );
     surface->DrawString( surface, text, -1, x+2, y+2, flags );
     surface->DrawString( surface, text, -1, x-2, y-2, flags );

     surface->SetColor( surface, 0x00, 0x00, 0xa0, 0xff );
//     surface->SetColor( surface, 0xff, 0xff, 0xff, 0xff );
     surface->DrawString( surface, text, -1, x, y, flags );
}

static void
draw_image( IDirectFBSurface  *surface,
            ZoomMode           zoom_mode,
            const StretchAlgo *algo )
{
     char      buf[32];
     long long stamp;

     switch (zoom_mode) {
          case ZOOM_NONE:
               snprintf( buf, sizeof(buf), "%dx%d", m_image.width, m_image.height );

               m_dfb->WaitIdle( m_dfb );
               stamp = myclock();

               surface->Blit( surface, m_image.surface, NULL,
                              (m_width  - m_image.width) / 2,
                              (m_height - m_image.height) / 2 );

               m_dfb->WaitIdle( m_dfb );
               m_diff = myclock() - stamp;

               break;

          case ZOOM_FULL:
               snprintf( buf, sizeof(buf), "%dx%d", m_width, m_height );

               stretch_blit( surface, m_image.surface,
                             0, 0, m_width, m_height,
                             algo );
               break;

          case ZOOM_640x360:
               snprintf( buf, sizeof(buf), "640x360" );

               stretch_blit( surface, m_image.surface,
                             (m_width  - 640) / 2,
                             (m_height - 360) / 2,
                             640, 360,
                             algo );
               break;

          case ZOOM_426x240:
               snprintf( buf, sizeof(buf), "426x240" );

               stretch_blit( surface, m_image.surface,
                             (m_width  - 426) / 2,
                             (m_height - 240) / 2,
                             426, 240,
                             algo );
               break;
     }

     /* zoom, bottom right */
     draw_text( surface, buf, m_width - 10, m_height - 55, DSTF_BOTTOMRIGHT );


     snprintf( buf, sizeof(buf), "%dx%d", m_width, m_height );

     /* layer resolution, top right */
     draw_text( surface, buf, m_width - 10, 10, DSTF_TOPRIGHT );
}

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     int                    i;
     bool                   redraw = true;
     DFBResult              err;
     DFBFontDescription     fdsc;
     DFBSurfaceDescription  dsc;
     IDirectFBFont         *font       = NULL;
     IDirectFBEventBuffer  *buffer     = NULL;
     IDirectFBSurface      *primary    = NULL;
     int                    cur_algo   = 0;
     int                    cur_image  = 0;
     ZoomMode               zoom_mode  = ZOOM_FIRST;
     Resolution             resolution = RES_NATIVE;

     DFBCHECK(DirectFBInit( &argc, &argv ));

     /* parse command line */
     for (i = 1; i < argc; i++) {
          if (strncmp (argv[i], "--", 2) == 0) {
               if (strcmp (argv[i] + 2, "help") == 0) {
                    print_usage();
                    return 0;
               }
          }

          print_usage();
          return 1;
     }

     /* create the super interface */
     DFBCHECK(DirectFBCreate( &m_dfb ));

     /* create the input buffer */
     DFBCHECK(m_dfb->CreateInputEventBuffer( m_dfb, DICAPS_KEYS, DFB_FALSE, &buffer ));

     /* load the font */
     fdsc.flags  = DFDESC_HEIGHT;
     fdsc.height = 46;

     DFBCHECK(m_dfb->CreateFont( m_dfb, "data/decker.ttf", &fdsc, &font ));

     dsc.flags       = DSDESC_CAPS | DSDESC_PIXELFORMAT;
     dsc.caps        = DSCAPS_PRIMARY;
     dsc.pixelformat = DSPF_RGB16;

     /* Set the cooperative level to DFSCL_FULLSCREEN for exclusive access to the primary layer. */
     err = m_dfb->SetCooperativeLevel( m_dfb, DFSCL_FULLSCREEN );
     if (err)
          DirectFBError( "Failed to get exclusive access", err );
     else
          dsc.caps |= DSCAPS_DOUBLE;

     /* Get the primary surface, i.e. the surface of the primary layer. */
     DFBCHECK(m_dfb->CreateSurface( m_dfb, &dsc, &primary ));

     primary->Clear( primary, 0, 0, 0, 0 );
     primary->GetSize( primary, &m_width, &m_height );

     m_orig_width  = m_width;
     m_orig_height = m_height;

     primary->SetFont( primary, font );

     /* scan */
     err = scan_for_images();
     if (err)
          goto out;

     /* load */
     err = load_image( 0, dsc.pixelformat );
     if (err)
          goto out;

     while (1) {
          DFBInputEvent event;

          /*
           * Display
           */
          if (redraw) {
               char buf[16];

               if (zoom_mode != ZOOM_FULL)
                    primary->Clear( primary, 0, 0, 0, 0 );

               draw_image( primary, zoom_mode, &stretch_algos[cur_algo] );

               /* filename, top left */
               draw_text( primary, dir_images[cur_image]->d_name, 10, 10, DSTF_TOPLEFT );


               /* src format, bottom left */
               draw_text( primary, dfb_pixelformat_name(m_image.format), 10, m_height - 100, DSTF_BOTTOMLEFT );

               /* dst format, bottom left */
               draw_text( primary, dfb_pixelformat_name(dsc.pixelformat), 10, m_height - 55, DSTF_BOTTOMLEFT );


               /* algo name, bottom center */
               draw_text( primary, stretch_algos[cur_algo].name, m_width/2, m_height - 55, DSTF_BOTTOMCENTER );

               /* algo description, bottom center */
               draw_text( primary, stretch_algos[cur_algo].description, m_width/2, m_height - 10, DSTF_BOTTOMCENTER );


               /* scaling time, bottom right */
               snprintf( buf, sizeof(buf), "%d ms", m_diff );
               draw_text( primary, buf, m_width - 10, m_height - 100, DSTF_BOTTOMRIGHT );


               primary->Flip( primary, NULL, DSFLIP_NONE );

               /* for later fast message display */
               primary->Blit( primary, primary, NULL, 0, 0 );

               redraw = false;
          }

          /*
           * Handle events
           */

          buffer->WaitForEvent( buffer );

          while (buffer->GetEvent( buffer, DFB_EVENT(&event) ) == DFB_OK) {
               if (event.type != DIET_KEYPRESS)
                    continue;

               switch (event.key_symbol) {
                    case DIKS_CURSOR_DOWN:   /* next image */
                         if (cur_image != num_images - 1) {
                              draw_text( primary, "Loading...", 680, 400, DSTF_TOPCENTER );
                              primary->Flip( primary, NULL, DSFLIP_NONE );

                              load_image( ++cur_image, m_load32 ? DSPF_RGB32 : dsc.pixelformat );
                              redraw = true;
                         }
                         break;

                    case DIKS_CURSOR_UP:     /* prev image */
                         if (cur_image != 0) {
                              draw_text( primary, "Loading...", 680, 400, DSTF_TOPCENTER );
                              primary->Flip( primary, NULL, DSFLIP_NONE );

                              load_image( --cur_image, m_load32 ? DSPF_RGB32 : dsc.pixelformat );
                              redraw = true;
                         }
                         break;

                    case DIKS_CURSOR_RIGHT:  /* next algo */
                         if (cur_algo != num_algos - 1) {
                              if (zoom_mode != ZOOM_NONE) {
                                   draw_text( primary, "Scaling...", 680, 400, DSTF_TOPCENTER );
                                   primary->Flip( primary, NULL, DSFLIP_NONE );
                              }

                              cur_algo++;
                              redraw = true;
                         }
                         break;

                    case DIKS_CURSOR_LEFT:   /* prev algo */
                         if (cur_algo != 0) {
                              if (zoom_mode != ZOOM_NONE) {
                                   draw_text( primary, "Scaling...", 680, 400, DSTF_TOPCENTER );
                                   primary->Flip( primary, NULL, DSFLIP_NONE );
                              }

                              cur_algo--;
                              redraw = true;
                         }
                         break;

                    case DIKS_OK:            /* zoom */
                         if (zoom_mode == ZOOM_LAST)
                              zoom_mode = ZOOM_FIRST;
                         else
                              zoom_mode++;

                         if (zoom_mode != ZOOM_NONE) {
                              draw_text( primary, "Scaling...", 680, 400, DSTF_TOPCENTER );
                              primary->Flip( primary, NULL, DSFLIP_NONE );
                         }

                         redraw = true;
                         break;

                    case DIKS_TEXT:          /* switch layer format */
                         draw_text( primary, "Reloading...", 680, 400, DSTF_TOPCENTER );
                         primary->Flip( primary, NULL, DSFLIP_NONE );

                         dsc.pixelformat = (dsc.pixelformat == DSPF_RGB16) ? DSPF_ARGB4444 : DSPF_RGB16;

                         load_image( cur_image, m_load32 ? DSPF_RGB32 : dsc.pixelformat );


                         primary->Release( primary );

                         DFBCHECK(m_dfb->CreateSurface( m_dfb, &dsc, &primary ));

                         primary->SetFont( primary, font );


                         redraw = true;
                         break;

                    case DIKS_MUTE:          /* switch source format */
                         draw_text( primary, "Reloading...", 680, 400, DSTF_TOPCENTER );
                         primary->Flip( primary, NULL, DSFLIP_NONE );

                         m_load32 = !m_load32;

                         load_image( cur_image, m_load32 ? DSPF_RGB32 : DSPF_RGB16 );

                         redraw = true;
                         break;

                    case DIKS_CUSTOM1:       /* switch resolution */
                         if (++resolution == _RES_NUM)
                              resolution = 0;

                         dsc.flags |= DSDESC_WIDTH | DSDESC_HEIGHT;

                         switch (resolution) {
                              case RES_NATIVE:
                                   dsc.width  = m_orig_width;
                                   dsc.height = m_orig_height;
                                   break;
                              case RES_1280x720:
                                   dsc.width  = 1280;
                                   dsc.height =  720;
                                   break;
                              case RES_1024x768:
                                   dsc.width  = 1024;
                                   dsc.height =  768;
                                   break;
                              case RES_1024x576:
                                   dsc.width  = 1024;
                                   dsc.height =  576;
                                   break;
                              case RES_852x480:
                                   dsc.width  =  852;
                                   dsc.height =  480;
                                   break;
                              case RES_640x480:
                                   dsc.width  =  640;
                                   dsc.height =  480;
                                   break;
                              default:
                                   break;
                         }

                         primary->Release( primary );

                         DFBCHECK(m_dfb->CreateSurface( m_dfb, &dsc, &primary ));

                         primary->SetFont( primary, font );

                         primary->GetSize( primary, &m_width, &m_height );

                         redraw = true;
                         break;

                    case DIKS_MENU:
                         if (zoom_mode != ZOOM_NONE) {
                              draw_text( primary, "Scaling...", 680, 400, DSTF_TOPCENTER );
                              primary->Flip( primary, NULL, DSFLIP_NONE );
                         }

                         redraw = true;
                         break;

                    default:
                         break;
               }
          }
     }


out:
     /* release our interfaces to shutdown DirectFB */
     if (m_image.surface)
          m_image.surface->Release( m_image.surface );
     if (primary)
          primary->Release( primary );
     if (font)
          font->Release( font );
     if (buffer)
          buffer->Release( buffer );
     if (m_dfb)
          m_dfb->Release( m_dfb );

     return 0;
}
