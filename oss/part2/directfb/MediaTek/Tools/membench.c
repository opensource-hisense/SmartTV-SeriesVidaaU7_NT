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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <asm/types.h>

typedef struct {
     const char *name;
     void       *(*copy) (void *dst, const void *src, size_t n);
} MemcpyMethod;


static void *
loop_copy (void *dst, const void *src, size_t n)
{
     int          i;
     __u32       *d = dst;
     const __u32 *s = src;

     n >>= 2;

     for (i = 0; i < n; i++)
          d[i] = s[i];

     return dst;
}

static void *
duff_copy (void *dst, const void *src, size_t n)
{
     __u32       *d = dst;
     const __u32 *s = src;

     n >>= 2;

     while (n) {
          int l = n & 7;

          switch (l) {
               default:
                    l = 8;
                    d[7] = s[7];
               case 7:
                    d[6] = s[6];
               case 6:
                    d[5] = s[5];
               case 5:
                    d[4] = s[4];
               case 4:
                    d[3] = s[3];
               case 3:
                    d[2] = s[2];
               case 2:
                    d[1] = s[1];
               case 1:
                    d[0] = s[0];
          }

          d += l;
          s += l;
          n -= l;
     }

     return dst;
}

static void *
memset_benchmark (void *dst, const void *src, size_t n)
{
     return memset( dst, 0, n );
}

static void *
memset_loop_benchmark (void *dst, const void *src, size_t n)
{
     int    i;
     __u32 *d = dst;

     n >>= 2;

     for (i = 0; i < n; i++)
          d[i] = 0;

     return dst;
}

static MemcpyMethod methods[] = {
     { "glibc memcpy", memcpy},
     { "loop_copy", loop_copy},
     { "duff_copy", duff_copy},
     { "glibc memset", memset_benchmark},
     { "loop memset", memset_loop_benchmark},
     { NULL, NULL}
};

int
main (int argc, char *argv[])
{
     int           verify = 0;
     MemcpyMethod *method = methods;


     if (argv[1] && !strcmp(argv[1],"-v"))
          verify = 1;

     while (method->name) {
          int            i;
          char           buffer1[0x100000];
          char           buffer2[0x100000];
          struct timeval tv1, tv2;

          memset( buffer1, 0, sizeof(buffer1) );
          memset( buffer2, 0, sizeof(buffer2) );


          printf ("Benchmarking '%s'...", method->name);

          method->copy (buffer1, buffer2, 0x100000);
          method->copy (buffer2, buffer1, 0x100000);


          gettimeofday (&tv1, NULL);

          for (i=0; i<640; i++) {
               method->copy (buffer1, buffer2, 0x100000);

               if (verify) {
                    int n;

                    for (n=0; n<sizeof(buffer1); n++)
                         if (buffer1[n]) {
                              printf( "BYTE ERROR: %02x at %d\n", buffer1[n], n );
                              buffer1[n] = 0;
                         }
               }
          }

          gettimeofday (&tv2, NULL);

          printf ("%5ld Mb/sec\n",
                  640000 / (
                          (tv2.tv_sec * 1000 + tv2.tv_usec / 1000) -
                          (tv1.tv_sec * 1000 + tv1.tv_usec / 1000)));

          method++;
     }

     return 0;
}
