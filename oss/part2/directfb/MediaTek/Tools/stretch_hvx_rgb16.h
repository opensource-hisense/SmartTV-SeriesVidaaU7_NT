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
static void STRETCH_HVX_RGB16( void       *dst,
                               int         dpitch,
                               const void *src,
                               int         spitch,
                               int         width,
                               int         height,
                               int         dst_width,
                               int         dst_height/*,
                               DFBRegion  *clip */ )
{
     DFBRegion  _clip   = { 0, 0, dst_width - 1, dst_height - 1 };
     DFBRegion *clip    = &_clip;
     int        x, y, r = 0;
     int        head    = ((((unsigned long) dst) & 2) >> 1) ^ (clip->x1 & 1);
     int        cw      = clip->x2 - clip->x1 + 1;
     int        ch      = clip->y2 - clip->y1 + 1;
     int        tail    = (cw - head) & 1;
     int        w2      = (cw - head) / 2;
     u32        hfraq   = ((u32)width  << 18) / (u32)dst_width;
     u32        vfraq   = ((u32)height << 18) / (u32)dst_height;
     int        dp4     = dpitch / 4;
     u32        point0  = POINT_0 + clip->x1 * hfraq;
     u32        point   = point0;
     u32        line    = LINE_0 + clip->y1 * vfraq;
     u32       *dst32;
     u32        ratios[cw];

     u32        _lbT[w2+8];
     u32        _lbB[w2+8];

     u32       *lbX;
     u32       *lbT = (u32*)((((unsigned long)(&_lbT[0])) + 31) & ~31);
     u32       *lbB = (u32*)((((unsigned long)(&_lbB[0])) + 31) & ~31);

     int        lineT = -2000;

//     direct_log_printf( NULL, "     %p [%d] -> %p [%d]\n", src, spitch, dst, dpitch );

     for (x=0; x<cw; x++) {
          ratios[x] = POINT_TO_RATIO( point, hfraq );
         // printf("(%3d) %6x (%6x) <- %3d %3d\n",x,ratios[x], point, POINT_L( point, hfraq ), POINT_R( point, hfraq ));

          point += hfraq;
     }

     dst += clip->x1 * 2 + clip->y1 * dpitch;

     dst32 = dst;

     if (head) {
          u32 dpT, dpB, L, R;

          u16 *dst16 = dst;

          point = point0;

          for (y=0; y<ch; y++) {
               u32 X = LINE_TO_RATIO( line, vfraq );

               const u16 *srcT = src + spitch * LINE_T( line, vfraq );
               const u16 *srcB = src + spitch * LINE_B( line, vfraq );


               /*
                * Horizontal interpolation
                */

               L = srcT[POINT_L( point, hfraq )];
               R = srcT[POINT_R( point, hfraq )];

               dpT = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                      ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

               L = srcB[POINT_L( point, hfraq )];
               R = srcB[POINT_R( point, hfraq )];

               dpB = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                      ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

               /*
                * Vertical interpolation
                */

               dst16[0] = ((((((dpB & 0xf81f) - (dpT & 0xf81f))*X) >> 5) + (dpT & 0xf81f)) & 0xf81f) +
                          ((((((dpB>>5) & 0x3f) - ((dpT>>5) & 0x3f))*X) + (dpT & 0x07e0)) & 0x07e0);

               dst16 += dpitch / 2;
               line  += vfraq;
          }

          /* Adjust */
          point0 += hfraq;
          dst32   = dst + 2;

          /* Reset */
          line = LINE_0 + clip->y1 * vfraq;
     }

     /*
      * Scale line by line.
      */
     for (y=0; y<ch; y++) {
          int nlT = LINE_T( line, vfraq );

          /*
           * Fill line buffer(s) ?
           */
          if (nlT != lineT) {
               u32 L, R, dpT, dpB;
               const u16 *srcT = src + spitch * nlT;
               const u16 *srcB = src + spitch * (nlT + 1);
               int        diff = nlT - lineT;

               if (diff > 1) {
                    /*
                     * Two output pixels per step.
                     */
                    for (x=0, r=head, point=point0; x<w2; x++) {
                         /*
                          * Horizontal interpolation
                          */
                         L = srcT[POINT_L( point, hfraq )];
                         R = srcT[POINT_R( point, hfraq )];

                         dpT = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

                         L = srcB[POINT_L( point, hfraq )];
                         R = srcB[POINT_R( point, hfraq )];

                         dpB = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

                         point += hfraq;
                         r++;


                         L = srcT[POINT_L( point, hfraq )];
                         R = srcT[POINT_R( point, hfraq )];

                         dpT |= (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                 ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) << 10;

                         L = srcB[POINT_L( point, hfraq )];
                         R = srcB[POINT_R( point, hfraq )];

                         dpB |= (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                 ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) << 10;

                         point += hfraq;
                         r++;

                         /* Store */
                         lbT[x] = dpT;
                         lbB[x] = dpB;
                    }
               }
               else {
                    /* Swap */
                    lbX = lbT;
                    lbT = lbB;
                    lbB = lbX;

                    /*
                     * Two output pixels per step.
                     */
                    for (x=0, r=head, point=point0; x<w2; x++) {
                         /*
                          * Horizontal interpolation
                          */
                         L = srcB[POINT_L( point, hfraq )];
                         R = srcB[POINT_R( point, hfraq )];

                         dpB = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

                         point += hfraq;
                         r++;


                         L = srcB[POINT_L( point, hfraq )];
                         R = srcB[POINT_R( point, hfraq )];

                         dpB |= (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                                 ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) << 10;

                         point += hfraq;
                         r++;

                         /* Store */
                         lbB[x] = dpB;
                    }
               }

               lineT = nlT;
          }

          /*
           * Vertical interpolation
           */
          u32 X = LINE_TO_RATIO( line, vfraq );

          for (x=0; x<w2; x++) {
               dst32[x] = ((((((lbB[x] & 0x07e0f81f) - (lbT[x] & 0x07e0f81f))*X) >> 5) + (lbT[x] & 0x07e0f81f)) & 0x07e0f81f) +
                          ((((((lbB[x]>>5) & 0x07c0f83f) - ((lbT[x]>>5) & 0x07c0f83f))*X) + (lbT[x] & 0xf81f07e0)) & 0xf81f07e0);
          }

          dst32 += dp4;
          line  += vfraq;
     }

     if (tail) {
          u32 dpT, dpB, L, R;

          u16 *dst16 = dst + cw * 2 - 2;

          /* Reset */
          line = LINE_0 + clip->y1 * vfraq;

          for (y=0; y<ch; y++) {
               u32 X = LINE_TO_RATIO( line, vfraq );

               const u16 *srcT = src + spitch * LINE_T( line, vfraq );
               const u16 *srcB = src + spitch * LINE_B( line, vfraq );


               /*
                * Horizontal interpolation
                */

               L = srcT[POINT_L( point, hfraq )];
               R = srcT[POINT_R( point, hfraq )];

               dpT = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                      ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

               L = srcB[POINT_L( point, hfraq )];
               R = srcB[POINT_R( point, hfraq )];

               dpB = (((((R & 0xf81f)-(L & 0xf81f))*ratios[r] + ((L & 0xf81f)<<6)) & 0x003e07c0) + 
                      ((((R & 0x07e0)-(L & 0x07e0))*ratios[r] + ((L & 0x07e0)<<6)) & 0x0001f800)) >> 6;

               /*
                * Vertical interpolation
                */

               dst16[0] = ((((((dpB & 0xf81f) - (dpT & 0xf81f))*X) >> 5) + (dpT & 0xf81f)) & 0xf81f) +
                          ((((((dpB>>5) & 0x3f) - ((dpT>>5) & 0x3f))*X) + (dpT & 0x07e0)) & 0x07e0);

               dst16 += dpitch / 2;
               line  += vfraq;
          }
     }
}

