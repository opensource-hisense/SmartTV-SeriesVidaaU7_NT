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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <direct/messages.h>

#define CHK(x) \
     do {                                              \
          if ((x) < 0) {                               \
               D_PERROR( "Failure at '%s'!\n", #x );   \
               return -1;                              \
          }                                            \
     } while (0)

#define OUTER_SIZE   (1024*1024)
#define INNER_SIZE   ( 512*1024)
#define INNER_OFFSET ( 256*1024)

int
main( int argc, char *argv[] )
{
     int  i;
     int  ofd;
//     int  ifd;
     u8  *map;
//     u8  *tmp;

     CHK( ofd = open( "outer.tst", O_CREAT | O_RDWR, 0644 ) );

     map = mmap( NULL, OUTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ofd, 0 );
     if (map == MAP_FAILED) {
          D_PERROR( "mmap() on outer file failed!\n" );
          return -1;
     }

     CHK( ftruncate( ofd, 1024 * 1024 ) );


/*     CHK( ifd = open( "inner.tst", O_CREAT | O_RDWR, 0644 ) );
     CHK( ftruncate( ifd,  512 * 1024 ) );

     munmap( map + INNER_OFFSET, INNER_SIZE );

     tmp = mmap( map + INNER_OFFSET, INNER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ifd, 0 );
     if (tmp == MAP_FAILED) {
          D_PERROR( "mmap() on inner file failed!\n" );
          return -1;
     }

     close( ifd );
*/

     for (i=0; i<OUTER_SIZE; i++)
          map[i] = 0xff;

//     madvise( map + INNER_OFFSET, INNER_SIZE, MADV_DONTNEED );
//     munlock( map + INNER_OFFSET, INNER_SIZE );
     madvise( map + INNER_OFFSET, INNER_SIZE, 9 );

//     CHK( munmap( tmp, INNER_SIZE ) );
     CHK( munmap( map, OUTER_SIZE ) );


     close( ofd );

     return 0;
}
