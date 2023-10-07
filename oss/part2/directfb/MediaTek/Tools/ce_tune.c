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
#include <config.h>

#include <stdlib.h>
#include <unistd.h>

#include <direct/log.h>
#include <direct/messages.h>

#include <fusiondale.h>

#include <misc/conf.h>

#include "ce_tuner.h"


int
main( int argc, char *argv[] )
{
     DFBResult        ret;
     int                 result;
     int                 frequency = 2342;
     void               *ptr;
     IFusionDale        *dale;
     IComa              *coma;
     IComaComponent     *tuner;

     dfb_config_init( &argc, &argv );

     ret = FusionDaleInit( &argc, &argv );
     if (ret) {
          D_DERROR( ret, "FusionDaleInit() failed!\n" );
          return -1;
     }

     ret = FusionDaleCreate( &dale );
     if (ret) {
          D_DERROR( ret, "FusionDaleCreate() failed!\n" );
          return -2;
     }

     ret = dale->EnterComa( dale, "AV Platform", &coma );
     if (ret) {
          D_DERROR( ret, "IFusionDale::EnterComa('AV Platform') failed!\n" );
          return -1;
     }


     if (argc > 1) {
          char *end;

          frequency = strtoul( argv[1], &end, 10 );
     }


     ret = coma->GetComponent( coma, "Tuner", 7000, &tuner );
     if (ret) {
          D_DERROR( ret, "IComa::GetComponent('Tuner') failed!\n" );
          return -2;
     }

     /* 
      * Test SetFrequency
      */
     direct_log_printf( NULL, "AV/Tune: Calling SetFrequency( %d )...\n", frequency );

     ret = tuner->Call( tuner, AV_TUNER_SETFREQUENCY, (void*)(long) frequency, &result );
     if (ret)
          D_DERROR( ret, "IComaComponent::Call( TUNER_SETFREQUENCY, %d ) failed!\n", frequency );
     else
          direct_log_printf( NULL, "AV/Tune: ...SetFrequency( %d ) returned %d.\n", frequency, result );


     sleep( 2 );

     /* 
      * Test SetGains
      */
     direct_log_printf( NULL, "AV/Tune: Allocating %zu bytes...\n", sizeof(AVTunerSetGainsCtx) );

     ret = coma->GetLocal( coma, sizeof(AVTunerSetGainsCtx), &ptr );
     if (ret) {
          D_DERROR( ret, "IComa::GetLocal( %zu ) failed!\n", sizeof(AVTunerSetGainsCtx) );
     }
     else {
          AVTunerSetGainsCtx *gains = ptr;

          gains->num      = 3;
          gains->gains[0] = 23;
          gains->gains[1] = 24;
          gains->gains[2] = 25;

          direct_log_printf( NULL, "AV/Tune: Calling SetGains( %p )...\n", gains );

          ret = tuner->Call( tuner, AV_TUNER_SETGAINS, gains, &result );
          if (ret)
               D_DERROR( ret, "IComaComponent::Call( TUNER_SETGAINS, %p ) failed!\n", gains );
          else
               direct_log_printf( NULL, "AV/Tune: ...SetGains( %p ) returned %d.\n", gains, result );

          direct_log_printf( NULL, "AV/Tune: Deallocating %zu bytes...\n", sizeof(AVTunerSetGainsCtx) );

          coma->FreeLocal( coma );
     }

     direct_log_printf( NULL, "AV/Tune: Exiting...\n" );

     tuner->Release( tuner );
     coma->Release( coma );
     dale->Release( dale );

     return 0;
}

