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

#include <directfb.h>

#include <direct/util.h>

#include "test_frame8.h"


/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                     \
               err = x;                                                    \
               if (err != DFB_OK) {                                        \
                    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
                    DirectFBErrorFatal( #x, err );                         \
               }

typedef int (*BenchFunc)( IDirectFBSurface *source,
                          IDirectFBSurface *dest,
                          int               num );

typedef void (*StretchFunc16)( __u16       *dst,
                               int          dpitch,
                               const __u16 *src,
                               int          spitch,
                               int          width,
                               int          height );

/**********************************************************************************************************************/

static int m_width;
static int m_height;
static int m_duration = 4000;

/**********************************************************************************************************************/

static inline long myclock( void )
{
     struct timeval tv;

     gettimeofday (&tv, NULL);

     return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void print_usage( void )
{
     printf ("StretchBlit Benchmark\n\n");
     printf ("Usage: stretch_bench [options]\n\n");
     printf ("Options:\n\n");
     printf ("  --duration <milliseconds>    Duration of each benchmark.\n");
     printf ("  --help                       Print usage information.\n");
     printf ("  --dfb-help                   Output DirectFB usage information.\n\n");
}

/**********************************************************************************************************************/

static int stretch_blit( IDirectFBSurface *source, IDirectFBSurface *dest, int num )
{
    int          loops = 0;
    long long    stamp = myclock();
    DFBRectangle drect = { 0, 0, m_width, m_height };

    do {
         dest->StretchBlit( dest, source, NULL, &drect );

         loops++;
    } while (myclock() < stamp + m_duration);

//    dest->Dump( dest, "/tmp", "StretchBlit" );

    return loops;
}

/**********************************************************************************************************************/

static void stretch_16_h1( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         for (x=0; x<w2; x++) {
              __u32 dp;

              __u32 s1 = src[point>>20];
              __u32 s2 = src[(point>>20) + 1];
              int   ra = (point >> 17) & 0x7;

              point += hfraq;

              switch (ra) {
                   case 0:
                        dp = s1;
                        break;
                   case 3:
                   case 4:
                        dp = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 7:
                        dp = s2;
                        break;
                   default: {
                        __u32 RB = ((s2 & 0x0f0f) - (s1 & 0x0f0f)) * (ra+1) + ((s1 & 0x0f0f) << 3);
                        __u32 AG = ((s2 & 0xf0f0) - (s1 & 0xf0f0)) * (ra+1) + ((s1 & 0xf0f0) << 3);

                        dp = ((RB >> 3) & 0x0f0f) | ((AG >> 3) & 0xf0f0);
                   }
              }

              __u32 s3 = src[point>>20];
              __u32 s4 = src[(point>>20) + 1];
              int   rb = (point >> 17) & 0x7;

              point += hfraq;

              switch (rb) {
                   case 0:
                        dp |= s3 << 16;
                        break;
                   case 3:
                   case 4:
                        dp |= ((((s4 & 0x0f0f) + (s3 & 0x0f0f)) << 15) & 0x0f0f0000) |
                              ((((s4 & 0xf0f0) + (s3 & 0xf0f0)) << 15) & 0xf0f00000);
                        break;
                   case 7:
                        dp |= s4 << 16;
                        break;
                   default: {
                        __u32 RB = ((s4 & 0x0f0f) - (s3 & 0x0f0f)) * (rb+1) + ((s3 & 0x0f0f) << 3);
                        __u32 AG = ((s4 & 0xf0f0) - (s3 & 0xf0f0)) * (rb+1) + ((s3 & 0xf0f0) << 3);

                        dp |= ((RB << 13) & 0x0f0f0000) | ((AG << 13) & 0xf0f00000);
                   }
              }

              dst32[x] = dp;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h2( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int   point = 0;
         int   ratio;
         __u32 dp;
         __u32 tmp;
         __u32 s1rb;
         __u32 s2rb;

         __u32 s1ag;
         __u32 s2ag;

         for (x=0; x<w2; x++) {
              tmp = src[point >> 20] | (src[(point >> 20)+1] << 16);

              s1rb = tmp & 0x0f0f;
              s2rb = (tmp >> 16) & 0x0f0f;

              s1ag = tmp & 0xf0f0;
              s2ag = (tmp >> 16) & 0xf0f0;

              ratio = (point >> 17) & 0x7;
              point += hfraq;

              switch (ratio) {
                   case 0:
                        dp = tmp & 0xffff;
                        break;
                   case 3:
                   case 4:
                        dp = (((s2rb + s1rb) >> 1) & 0x0f0f) |
                             (((s2ag + s1ag) >> 1) & 0xf0f0);
                        break;
                   case 7:
                        dp = tmp >> 16;
                        break;
                   default: {
                        __u32 RB = ((s2rb - s1rb) * (ratio+1)) + (s1rb << 3);
                        __u32 AG = ((s2ag - s1ag) * (ratio+1)) + (s1ag << 3);

                        dp = ((RB >> 3) & 0x0f0f) | ((AG >> 3) & 0xf0f0);
                   }
              }


              tmp = src[point >> 20] | (src[(point >> 20)+1] << 16);

              s1rb = tmp & 0x0f0f;
              s2rb = (tmp >> 16) & 0x0f0f;

              s1ag = tmp & 0xf0f0;
              s2ag = (tmp >> 16) & 0xf0f0;

              ratio = (point >> 17) & 0x7;
              point += hfraq;

              switch (ratio) {
                   case 0:
                        dp |= tmp << 16;
                        break;
                   case 3:
                   case 4:
                        dp |= (((s2rb + s1rb) << 15) & 0x0f0f0000) |
                              (((s2ag + s1ag) << 15) & 0xf0f00000);
                        break;
                   case 7:
                        dp |= tmp & 0xffff0000;
                        break;
                   default: {
                        __u32 RB = ((s2rb - s1rb) * (ratio+1)) + (s1rb << 3);
                        __u32 AG = ((s2ag - s1ag) * (ratio+1)) + (s1ag << 3);

                        dp |= ((RB << 13) & 0x0f0f0000) | ((AG << 13) & 0xf0f00000);
                   }
              }



              dst32[x] = dp;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h3( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         for (x=0; x<w2; x++) {
              __u32 s1 = src[point>>20];
              __u32 s2 = src[(point>>20) + 1];
              int   ra = ((point >> 17) & 0x7) + 1;

              point += hfraq;

              __u32 s3 = src[point>>20];
              __u32 s4 = src[(point>>20) + 1];
              int   rb = ((point >> 17) & 0x7) + 1;

              point += hfraq;


              dst32[x] = ((((((s2 & 0x0f0f) - (s1 & 0x0f0f)) * ra) >> 3) + (s1 & 0x0f0f)) & 0x0f0f) |
                         ((((((s2 & 0xf0f0) - (s1 & 0xf0f0)) * ra) >> 3) + (s1 & 0xf0f0)) & 0xf0f0) |
                        (((((((s4 & 0x0f0f) - (s3 & 0x0f0f)) * rb) >> 3) + (s3 & 0x0f0f)) & 0x0f0f) |
                         ((((((s4 & 0xf0f0) - (s3 & 0xf0f0)) * rb) >> 3) + (s3 & 0xf0f0)) & 0xf0f0)) << 16;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h4( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         for (x=0; x<w2; x++) {
              __u32 s1 = src[point>>20];
              __u32 s2 = src[(point>>20) + 1];
              int   ra = ((point >> 17) & 0x7) + 1;

              point += hfraq;

              switch (ra) {
                   case 1:
                        break;
                   case 4:
                   case 5:
                        s1 = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 8:
                        s1 = s2;
                        break;
                   default: {
                        s1 = ((((((s2 & 0x0f0f) - (s1 & 0x0f0f)) * ra) >> 3) + (s1 & 0x0f0f)) & 0x0f0f) |
                             ((((((s2 & 0xf0f0) - (s1 & 0xf0f0)) * ra) >> 3) + (s1 & 0xf0f0)) & 0xf0f0);
                   }
              }

              __u32 s3 = src[point>>20];
              __u32 s4 = src[(point>>20) + 1];
              int   rb = ((point >> 17) & 0x7) + 1;

              point += hfraq;

              switch (rb) {
                   case 1:
                        dst32[x] = s1 | (s3 << 16);
                        break;
                   case 4:
                   case 5:
                        dst32[x] = s1 | (((((s4 & 0x0f0f) + (s3 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                         ((((s4 & 0xf0f0) + (s3 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 8:
                        dst32[x] = s1 | (s4 << 16);
                        break;
                   default: {
                        dst32[x] = s1 | ((((((((s2 & 0x0f0f) - (s1 & 0x0f0f)) * ra) >> 3) + (s1 & 0x0f0f)) & 0x0f0f) |
                                          ((((((s2 & 0xf0f0) - (s1 & 0xf0f0)) * ra) >> 3) + (s1 & 0xf0f0)) & 0xf0f0)) << 16);
                   }
              }
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h5( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;
         int ratio;

         __u16 s1;
         __u16 s2;
         __u16 lo;

         for (x=0; x<w2; x++) {
              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = ((point >> 17) & 0x7) + 1;

              point += hfraq;

              switch (ratio) {
                   case 1:
                        lo = s1;
                        break;
                   case 4:
                   case 5:
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 8:
                        lo = s2;
                        break;
                   default: {
                        lo = ((((((s2 & 0x0f0f) - (s1 & 0x0f0f)) * ratio) >> 3) + (s1 & 0x0f0f)) & 0x0f0f) |
                             ((((((s2 & 0xf0f0) - (s1 & 0xf0f0)) * ratio) >> 3) + (s1 & 0xf0f0)) & 0xf0f0);
                   }
              }


              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = ((point >> 17) & 0x7) + 1;

              point += hfraq;

              switch (ratio) {
                   case 1:
                        dst32[x] = lo | (s1 << 16);
                        break;
                   case 4:
                   case 5:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 8:
                        dst32[x] = lo | (s2 << 16);
                        break;
                   default: {
                        dst32[x] = lo | ((((((((s2 & 0x0f0f) - (lo & 0x0f0f)) * ratio) >> 3) + (lo & 0x0f0f)) & 0x0f0f) |
                                          ((((((s2 & 0xf0f0) - (lo & 0xf0f0)) * ratio) >> 3) + (lo & 0xf0f0)) & 0xf0f0)) << 16);
                   }
              }
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h6( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;
         int ratio;

         __u16 s1;
         __u16 s2;
         __u16 lo = 0;

         for (x=0; x<w2; x++) {
              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = (point >> 18) & 0x3;

              point += hfraq;

              switch (ratio) {
                   case 0:
                        lo = s1;
                        break;
                   case 1:
                   case 2:
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 3:
                        lo = s2;
                        break;
              }


              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = (point >> 18) & 0x3;

              point += hfraq;

              switch (ratio) {
                   case 0:
                        dst32[x] = lo | (s1 << 16);
                        break;
                   case 1:
                   case 2:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 3:
                        dst32[x] = lo | (s2 << 16);
                        break;
              }
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h7( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;
         int ratio;

         __u16 s1;
         __u16 s2;
         __u16 lo;

         for (x=0; x<w2; x++) {
              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = (point >> 18) & 0x3;

              point += hfraq;

              if (ratio) {
                   if (ratio == 3)
                        lo = s2;
                   else
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
              }
              else
                   lo = s1;


              s1    = src[point>>20];
              s2    = src[(point>>20) + 1];
              ratio = (point >> 18) & 0x3;

              point += hfraq;

              if (ratio) {
                   if (ratio == 3)
                        dst32[x] = lo | (s2 << 16);
                   else
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
              }
              else
                   dst32[x] = lo | (s1 << 16);
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h8( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u16 lo = 0;

         for (x=0; x<w2; x++) {
              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        lo = s1;
                        break;
                   case 1:
                   case 2:
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                        break;
                   case 3:
                        lo = s2;
                        break;
              }

              point += hfraq;


              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dst32[x] = lo | (s1 << 16);
                        break;
                   case 1:
                   case 2:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000));
                        break;
                   case 3:
                        dst32[x] = lo | (s2 << 16);
                        break;
              }

              point += hfraq;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_h9( __u16       *dst,
                           int          dpitch,
                           const __u16 *src,
                           int          spitch,
                           int          width,
                           int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u16 lo = 0;

         for (x=0; x<w2; x++) {
              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        lo = s1;
                        break;
                   case 1:
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 2:
                        lo = ((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 3:
                        lo = s2;
                        break;
              }

              point += hfraq;


              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        dst32[x] = lo | (s1 << 16);
                        break;
                   case 1:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 2:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 3:
                        dst32[x] = lo | (s2 << 16);
                        break;
              }

              point += hfraq;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static const StretchFunc16 stretch_16_h[] = {
     stretch_16_h1,
     stretch_16_h2,
     stretch_16_h3,
     stretch_16_h4,
     stretch_16_h5,
     stretch_16_h6,
     stretch_16_h7,
     stretch_16_h8,
     stretch_16_h9
};

static int my_stretch_h( IDirectFBSurface *source, IDirectFBSurface *dest, int num )
{
    DFBResult              err;
    int                    loops = 0;
    long long              stamp = myclock();
    void                  *src;
    int                    spitch;
    void                  *dst;
    int                    dpitch;
    DFBSurfacePixelFormat  format;
    int                    width;
    int                    height;
    char                   buf[16];

    source->GetPixelFormat( source, &format );
    source->GetSize( source, &width, &height );

    DFBCHECK(source->Lock( source, DSLF_READ, &src, &spitch ));
    DFBCHECK(dest->Lock( dest, DSLF_WRITE, &dst, &dpitch ));

    switch (format) {
         case DSPF_ARGB4444:
              do {
                   stretch_16_h[num]( dst, dpitch, src, spitch, width, height );

                   loops++;
              } while (myclock() < stamp + m_duration);

              snprintf( buf, sizeof(buf), "stretch_16_h%d", num+1 );

//              dest->Dump( dest, "/tmp", buf );
              break;

         default:
              D_WARN( "unsupported format" );
              sleep(3);
              return 1;
    }

    source->Unlock( source );
    dest->Unlock( dest );

    return loops;
}

static void stretch_16_hv1( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = 0;

    __u32 *dst32 = (void*)dst;

    for (y=0; y<m_height; y++) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u16 sb;
         __u16 lo = 0;

         const __u16 *src2 = src;

         if (y < m_height - 1)
              src2 += spitch / 2;

         for (x=0; x<w2; x++) {
              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        switch (line >> 18) {
                             case 0:
                             case 1:
                                  lo = s1;
                                  break;
                             case 2:
                             case 3:
                                  sb = src2[point>>20];
                                  lo = ((((sb & 0x0f0f) + (s1 & 0x0f0f)) >> 1) & 0x0f0f) |
                                       ((((sb & 0xf0f0) + (s1 & 0xf0f0)) >> 1) & 0xf0f0);
                                  break;
//                                  lo = src2[point>>20];
                        }
                        break;
                   case 1:
                        lo = ((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 2:
                        lo = ((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) >> 2) & 0x0f0f) |
                             ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) >> 2) & 0xf0f0);
                        break;
                   case 3:
                        switch (line >> 18) {
                             case 0:
                             case 1:
                                  lo = s2;
                                  break;
                             case 2:
                             case 3:
                                  sb = src2[(point>>20) + 1];
                                  lo = ((((sb & 0x0f0f) + (s2 & 0x0f0f)) >> 1) & 0x0f0f) |
                                       ((((sb & 0xf0f0) + (s2 & 0xf0f0)) >> 1) & 0xf0f0);
                                  break;
//                                  lo = src2[(point>>20) + 1];
                        }
                        break;
              }

              point += hfraq;


              s1 = src[point>>20];
              s2 = src[(point>>20) + 1];

              switch ((point >> 18) & 0x3) {
                   case 0:
                        switch (line >> 18) {
                             case 0:
                             case 1:
                                  dst32[x] = lo | (s1 << 16);
                                  break;
                             case 2:
                             case 3:
                                  sb = src2[point>>20];
                                  dst32[x] = lo | ((((sb & 0x0f0f) + (s1 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                                  ((((sb & 0xf0f0) + (s1 & 0xf0f0)) << 15) & 0xf0f00000);
                                  break;
//                                  dst32[x] = lo | (src2[point>>20] << 16);
                        }
                        break;
                   case 1:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 2:
                        dst32[x] = lo | (((((s2 & 0x0f0f) + (s2 & 0x0f0f) + (s2 & 0x0f0f) + (s1 & 0x0f0f)) << 14) & 0x0f0f0000) |
                                         ((((s2 & 0xf0f0) + (s2 & 0xf0f0) + (s2 & 0xf0f0) + (s1 & 0xf0f0)) << 14) & 0xf0f00000));
                        break;
                   case 3:
                        switch (line >> 18) {
                             case 0:
                             case 1:
                                  dst32[x] = lo | (s2 << 16);
                                  break;
                             case 2:
                             case 3:
                                  sb = src2[(point>>20) + 1];
                                  dst32[x] = lo | ((((sb & 0x0f0f) + (s2 & 0x0f0f)) << 15) & 0x0f0f0000) |
                                                  ((((sb & 0xf0f0) + (s2 & 0xf0f0)) << 15) & 0xf0f00000);
                                  break;
//                                  dst32[x] = lo | (src2[(point>>20) + 1] << 16);
                        }
                        break;
              }

              point += hfraq;
         }

         dst32 += dpitch/4;

         line += vfraq;

         src += (line >> 20) * spitch/2;

         line &= 0xfffff;
    }
}

static void stretch_16_hv2( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


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


              if (y == m_height - 1)
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
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 3:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_16_hv3( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


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


              if (y == m_height - 1)
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

static void stretch_16_hv4( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


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


              if (y == m_height - 1)
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

static void stretch_16_hv5( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp = 0;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


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


              if (y == m_height - 1)
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
                             dst32[x] = (((((linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (linecache[x] & 0x0f0f0f0f) + (dp & 0x0f0f0f0f)) >> 2) & 0x0f0f0f0f) |
                                         (((((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((linecache[x] >> 4) & 0x0f0f0f0f) + ((dp >> 4) & 0x0f0f0f0f)) << 2) & 0xf0f0f0f0));
                             break;
                        case 3:
                             dst32[x] = linecache[x];
                             break;
                   }
              }

              linecache[x] = dp;
         }

         line -= vfraq;
    }
}

static void stretch_16_hv6( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
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


              if (y == m_height - 1)
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

static void stretch_16_hv7( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2    = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;

#define SLICE 64

    int offset = 0;

    __u32 linecache[SLICE];

    while (offset < w2) {
         int line = vfraq * (m_height - 1);

         int len = w2 - offset;

         if (len > SLICE)
              len = SLICE;

         for (y=m_height-1; y>=0; y--) {
              int point = offset*2 * hfraq;

              __u16 s1;
              __u16 s2;
              __u32 dp = 0;

              __u32       *dst32 = ((void*) dst) + dpitch * y + offset * 4;
              const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


              for (x=0; x<len; x++) {
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


                   if (y == m_height - 1)
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

         offset += len;
    }
}

#if 0
static void stretch_16_hv8( __u16       *dst,
                            int          dpitch,
                            const __u16 *src,
                            int          spitch,
                            int          width,
                            int          height )
{
    int x, y;
    int w2 = m_width / 2;
    int hfraq = (width  << 20) / m_width;
    int vfraq = (height << 20) / m_height;
    int line  = vfraq * (m_height - 1);

    __u32 linecache[m_width/2];

    for (y=m_height-1; y>=0; y--) {
         int point = 0;

         __u16 s1;
         __u16 s2;
         __u32 dp;

         __u32       *dst32 = ((void*) dst) + dpitch * y;
         const __u16 *src16 = ((void*) src) + spitch * (line >> 20);


         for (x=0; x<w2; x++) {
              s1 = src16[point>>20];
              s2 = src16[(point>>20) + 1];

              printf("%3d %3d\n",point>>20,(point>>20)+1);

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

              printf("%3d %3d\n",point>>20,(point>>20)+1);

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


              if (y == m_height - 1)
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
#endif

static const StretchFunc16 stretch_16_hv[] = {
     stretch_16_hv1,
     stretch_16_hv2,
     stretch_16_hv3,
     stretch_16_hv4,
     stretch_16_hv5,
     stretch_16_hv6,
     stretch_16_hv7
};

static int my_stretch_hv( IDirectFBSurface *source, IDirectFBSurface *dest, int num )
{
    DFBResult              err;
    int                    loops = 0;
    long long              stamp = myclock();
    void                  *src;
    int                    spitch;
    void                  *dst;
    int                    dpitch;
    DFBSurfacePixelFormat  format;
    int                    width;
    int                    height;
    char                   buf[16];

    source->GetPixelFormat( source, &format );
    source->GetSize( source, &width, &height );

    DFBCHECK(source->Lock( source, DSLF_READ, &src, &spitch ));
    DFBCHECK(dest->Lock( dest, DSLF_WRITE, &dst, &dpitch ));

    switch (format) {
         case DSPF_ARGB4444:
              do {
                   stretch_16_hv[num]( dst, dpitch, src, spitch, width, height );

                   loops++;
              } while (myclock() < stamp + m_duration);

              snprintf( buf, sizeof(buf), "stretch_16_hv%d", num+1 );

//              dest->Dump( dest, "/tmp", buf );
              break;

         default:
              D_WARN( "unsupported format" );
              sleep(3);
              return 1;
    }

    source->Unlock( source );
    dest->Unlock( dest );

    return loops;
}

/**********************************************************************************************************************/

int main( int argc, char *argv[] )
{
     DFBResult err;
     DFBSurfaceDescription dsc;
     IDirectFB *dfb;
     IDirectFBImageProvider *provider;
     IDirectFBSurface *primary;
     int i, n, m;

     DFBCHECK(DirectFBInit( &argc, &argv ));

     /* parse command line */
     for (n = 1; n < argc; n++) {
          if (strncmp (argv[n], "--", 2) == 0) {
               if (strcmp (argv[n] + 2, "help") == 0) {
                    print_usage();
                    return 0;
               }
               else
               if (strcmp (argv[n] + 2, "duration") == 0 &&
                   ++n < argc &&
                   sscanf (argv[n], "%d", &m_duration) == 1) {
                    continue;
               }
          }

          print_usage();
          return 1;
     }

     DirectFBSetOption ("bg-none", NULL);

     /* create the super interface */
     DFBCHECK(DirectFBCreate( &dfb ));

     /* Set the cooperative level to DFSCL_FULLSCREEN for exclusive access to the primary layer. */
     err = dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
     if (err)
          DirectFBError( "Failed to get exclusive access", err );

     /* Get the primary surface, i.e. the surface of the primary layer. */
     dsc.flags = DSDESC_CAPS | DSDESC_PIXELFORMAT;
     dsc.caps = DSCAPS_PRIMARY;
     dsc.pixelformat = DSPF_ARGB4444;

     DFBCHECK(dfb->CreateSurface( dfb, &dsc, &primary ));
     primary->Clear( primary, 0, 0, 0, 0 );
     primary->GetSize( primary, &m_width, &m_height );

//     m_height = 480;

     printf( "\nBenchmarking %dx%d -> %dx%d\n", 852, 480, m_width, m_height );

     BenchFunc         methods[] = { my_stretch_hv,
                                     my_stretch_h,
                                     stretch_blit };

     int               method_nums[] = { D_ARRAY_SIZE(stretch_16_hv),
                                         D_ARRAY_SIZE(stretch_16_h),
                                         1 };

     const char       *method_names[] = { "My Stretch HV",
                                          "My Stretch H",
                                          "StretchBlit" };

     IDirectFBSurface *sources[1];
     const char       *source_names[1] = { "16 bit ARGB"/*,
                                           " 8 bit indexed"*/ };

     /* create a surface and render an image to it */
     DFBCHECK(dfb->CreateImageProvider( dfb, "data/test_frame.png", &provider ));
     DFBCHECK(provider->GetSurfaceDescription( provider, &dsc ));
     DFBCHECK(dfb->CreateSurface( dfb, &dsc, &sources[0] ));
     DFBCHECK(provider->RenderTo( provider, sources[0], NULL ));
     provider->Release( provider );

     /* create a surface from the description */
//     DFBCHECK(dfb->CreateSurface( dfb, &test_frame8_png_desc, &sources[1] ));

/*     m_duration = 2000;
     while (true) {
          my_stretch_hv( sources[0], primary, 3 );
          my_stretch_hv( sources[0], primary, 6 );
     }
*/
     for (n = 0; n < D_ARRAY_SIZE(methods); n++) {
          printf( "\n" );
          printf( "------ %s ------\n", method_names[n] );

          for (i = 0; i < D_ARRAY_SIZE(sources); i++) {
               for (m = 0; m < method_nums[n]; m++) {
                    long t, dt, result;
                    unsigned long long pixels;

                    sync();
                    dfb->WaitIdle( dfb );
                    t = myclock();

                    pixels = methods[n]( sources[i], primary, m ) * m_width * m_height;

                    dfb->WaitIdle( dfb );
                    dt = myclock() - t;

                    result = pixels / (unsigned long long)dt;

                    printf( "(%2d) %-16s  %3ld.%.3ld MPixel/sec   -> %4ld ms per frame\n",
                            m+1, source_names[i], result / 1000, result % 1000, m_width*m_height / result );
               }
          }
     }

     printf( "\n" );

     /* release our interfaces to shutdown DirectFB */
     for (i=0; i<D_ARRAY_SIZE(sources); i++)
         sources[i]->Release( sources[i] );

     primary->Release( primary );

     dfb->Release( dfb );

     return 0;
}

