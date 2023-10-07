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
#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>

static int cuser, cnice, csystem, cidle, ctotal;
static int puser, pnice, psystem, pidle, ptotal;
static int duser, dnice, dsystem, didle, dtotal;

static int
read_stat()
{
     char  dummy[4];
     FILE *file;

     puser   = cuser;
     pnice   = cnice;
     psystem = csystem;
     pidle   = cidle;
     ptotal  = ctotal;

     file = fopen( "/proc/stat", "r" );
     if (!file) {
          perror( "Could not open '/proc/stat'" );
          return 0;
     }

     if (fscanf( file, "%3s %d %d %d %d", dummy, &cuser, &cnice, &csystem, &cidle ) < 4) {
          fprintf( stderr, "Parsing '/proc/stat' failed!\n" );
          return 0;
     }

     fclose( file );

     ctotal  = cuser + cnice + csystem + cidle;

     duser   = cuser - puser;
     dnice   = cnice - pnice;
     dsystem = csystem - psystem;
     didle   = cidle - pidle;
     dtotal  = ctotal - ptotal;

     return 1;
}

int
main( int argc, char *argv[] )
{
     unsigned int cycle      = 0;
     int          interval   = 1000000;
     char         symbols[4] = { '-', '\\', '|', '/' };

     if (argc > 1) {
          float val;

          if (sscanf( argv[1], "%f", &val ) != 1) {
               fprintf( stderr, "Usage: %s [interval]\n", argv[0] );
               return 0;
          }

          interval = 1000000 * val;
     }

     if (!read_stat())
          return 0;

     usleep( 200000 );

     while (read_stat()) {
          int u = duser * 100 / dtotal;
          int n = dnice * 100 / dtotal;
          int s = dsystem * 100 / dtotal;
          int i = didle * 100 / dtotal;

          printf( "    user %3d%% | nice %3d%% | system %3d%% | idle %3d%%   (%c)\r",
                  u, n, s, i, symbols[cycle++&3] );

          fflush( stdout );

          usleep( interval );
     }

     return 0;
}

