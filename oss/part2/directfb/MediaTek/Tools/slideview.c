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
#include <stdlib.h>
#include <string.h>

#include <directfb.h>

#include "slideview.h"

/**************************************************************************************************/

static IDirectFB             *dfb   = NULL;
static IDirectFBDisplayLayer *layer = NULL;

/**************************************************************************************************/

static IDirectFBWindow      *window  = NULL;
static IDirectFBSurface     *surface = NULL;
static IDirectFBEventBuffer *events  = NULL;

/**************************************************************************************************/

static DFBBoolean visible = DFB_FALSE;

/**************************************************************************************************/

static char **images        = NULL;
static int    num_images    = 0;
static int    current_image = -1;

/**************************************************************************************************/

static DFBResult
init_directfb()
{
     DFBResult ret;

     ret = DirectFBCreate( &dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate() failed", ret );
          return ret;
     }

     ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
     if (ret) {
          DirectFBError( "IDirectFB::GetDisplayLayer() failed", ret );
          dfb->Release( dfb );
          return ret;
     }

     return DFB_OK;
}

static void
deinit_directfb()
{
     layer->Release( layer );
     dfb->Release( dfb );
}

/**************************************************************************************************/

static DFBResult
create_window( int x, int y, int width, int height )
{
     DFBResult             ret;
     DFBDisplayLayerConfig conf;
     DFBWindowDescription  desc;

     layer->GetConfiguration( layer, &conf );

     desc.flags  = DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY;
     desc.width  = width;
     desc.height = height;
     desc.posx   = x;
     desc.posy   = y;

     ret = layer->CreateWindow( layer, &desc, &window );
     if (ret) {
          DirectFBError( "IDirectFBDisplayLayer::CreateWindow() failed", ret );
          return ret;
     }

/*     ret = window->GrabKey( window, DIKS_F10, DIMM_META );
     if (ret) {
          DirectFBError( "IDirectFBWindow::GrabKey() failed", ret );
          window->Release( window );
          return ret;
     }
*/
     ret = window->GetSurface( window, &surface );
     if (ret) {
          DirectFBError( "IDirectFBWindow::GetSurface() failed", ret );
          window->Release( window );
          return ret;
     }

//     window->SetOptions( window, DWOP_KEEP_POSITION | DWOP_KEEP_SIZE | DWOP_KEEP_STACKING );
//     window->SetStackingClass( window, DWSC_UPPER );

     window->CreateEventBuffer( window, &events );

     return DFB_OK;
}

static void
destroy_window()
{
     events->Release( events );
     surface->Release( surface );
     window->Release( window );
}

/**************************************************************************************************/

static void
fade_window( int from, int to, int duration )
{
/*
     int  opacity;
     long start, diff = 0;

     start = get_millis() - 100;
     duration += 100;

     do {
          opacity = from + (to - from) * (diff*diff) / (duration*duration);

          window->SetOpacity( window, opacity );

          diff = get_millis() - start;
     } while (diff < duration && opacity != to);

     window->SetOpacity( window, to );
*/
}

static void
show_window()
{
     fade_window( 0, 255, 500 );

     window->SetOpacity( window, 255 );

     window->RequestFocus( window );

     visible = DFB_TRUE;
}

static void
hide_window()
{
     fade_window( 255, 0, 500 );

     window->SetOpacity( window, 0 );

     visible = DFB_FALSE;
}

/**************************************************************************************************/

static void
load_image( const char *filename )
{
     DFBResult               ret;
     IDirectFBImageProvider *provider;

     /* Create an image provider for loading the file */
     ret = dfb->CreateImageProvider( dfb, filename, &provider );
     if (ret) {
          fprintf (stderr,
                   "load_image: CreateImageProvider for '%s': %s\n",
                   filename, DirectFBErrorString (ret));

          surface->Clear( surface, 0x40, 0, 0, 0xff );
     }
     else {
          /* Render the image to the created surface */
          ret = provider->RenderTo (provider, surface, NULL);
          if (ret) {
               fprintf (stderr,
                        "load_image: RenderTo for '%s': %s\n",
                        filename, DirectFBErrorString (ret));

               surface->Clear( surface, 0x40, 0, 0x40, 0xff );
          }

          /* Release the provider */
          provider->Release (provider);
     }

     surface->Flip( surface, NULL, 0 );
}

static void
load_previous()
{
     if (current_image > 0)
          load_image( images[--current_image] );
}

static void
load_next()
{
     if (current_image < num_images-1)
          load_image( images[++current_image] );
}

/**************************************************************************************************/

static void
dispatch_button( DFBWindowEvent *event )
{
}

static void
dispatch_key( DFBWindowEvent *event )
{
     if (event->type == DWET_KEYDOWN) {
          switch (event->key_symbol) {
               case DIKS_F10:
                    if (event->modifiers == DIMM_META) {
                         if (visible)
                              hide_window();
                         else
                              show_window();
                    }
                    break;

               case DIKS_CURSOR_LEFT:
                    load_previous();
                    break;

               case DIKS_CURSOR_RIGHT:
                    load_next();
                    break;

               default:
                    break;
          }
     }
}

static void
dispatch_motion( DFBWindowEvent *event )
{
}

static DFBBoolean
dispatch_events()
{
     DFBWindowEvent event;

     while (events->GetEvent( events, DFB_EVENT(&event) ) == DFB_OK) {
          switch (event.type) {
               case DWET_CLOSE:
               case DWET_DESTROYED:
                    return DFB_FALSE;

               case DWET_BUTTONUP:
               case DWET_BUTTONDOWN:
                    dispatch_button( &event );
                    break;

               case DWET_KEYUP:
                    if (event.key_symbol == DIKS_ESCAPE)
                         return DFB_FALSE;

               case DWET_KEYDOWN:
                    dispatch_key( &event );
                    break;

               case DWET_MOTION:
                    dispatch_motion( &event );
                    break;

               default:
                    break;
          }
     }

     return DFB_TRUE;
}

/**************************************************************************************************/

static DFBResult
parse_args( int argc, char *argv[] )
{
     int i;

     if (argc < 2) {
          fprintf( stderr, "\nUsage: slideview <file> [<file>]...\n\n" );
          return DFB_INVARG;
     }

     if (argc > 0x10000) {
          fprintf( stderr, "Too many arguments (%d)!\n", argc );
          return DFB_BUFFERTOOLARGE;
     }

     num_images = argc - 1;

     images = malloc( argc * sizeof(char*) );
     if (!images) {
          fprintf( stderr, "Out of memory!\n" );
          return DFB_NOSYSTEMMEMORY;
     }

     for (i=0; i<num_images; i++) {
          images[i] = strdup( argv[i+1] );
          if (!images[i]) {
               fprintf( stderr, "Out of memory!\n" );
               return DFB_NOSYSTEMMEMORY;
          }
     }

     return DFB_OK;
}

/**************************************************************************************************/

int
main( int argc, char *argv[] )
{
     DFBResult ret;

     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          DirectFBError( "DirectFBInit() failed", ret );
          return -1;
     }

     if (parse_args( argc, argv ))
          return -2;

     if (init_directfb())
          return -3;

     if (strcmp(argv[0], "slideview-big") ?
         create_window( 20, 80, 852, 480 ) : create_window( 0, 0, 1366, 768 ))
     {
          deinit_directfb();
          return -4;
     }

     load_next();

     show_window();

     while (dispatch_events()) {
          events->WaitForEvent( events );
     }

     destroy_window();

     deinit_directfb();

     return 0;
}

