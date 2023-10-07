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
#define USECLUT 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct/debug.h>
#include <direct/util.h>

#include <directfb.h>

/**************************************************************************************************/

typedef struct {
     int                  magic;

     IDirectFBWindow     *window;
     IDirectFBSurface    *surface;
#ifdef USECLUT
     IDirectFBPalette    *palette;
#endif

     DFBWindowID          window_id;
} DemoWindow;

/**************************************************************************************************/

static IDirectFB             *m_dfb    = NULL;
static IDirectFBDisplayLayer *m_layer  = NULL;
static IDirectFBEventBuffer  *m_events = NULL;

/**************************************************************************************************/

static DemoWindow   main_window;
static DemoWindow   attached_window;

/**************************************************************************************************/

static DFBResult
init_directfb( int *pargc, char ***pargv )
{
     DFBResult ret;

     D_ASSERT( m_dfb == NULL );
     D_ASSERT( m_layer == NULL );
     D_ASSERT( m_events == NULL );

     ret = DirectFBInit( pargc, pargv );
     if (ret) {
          DirectFBError( "DirectFBInit() failed", ret );
          return ret;
     }

     ret = DirectFBCreate( &m_dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate() failed", ret );
          return ret;
     }

     ret = m_dfb->GetDisplayLayer( m_dfb, DLID_PRIMARY, &m_layer );
     if (ret) {
          DirectFBError( "IDirectFB::GetDisplayLayer() failed", ret );
          goto error;
     }

     ret = m_dfb->CreateEventBuffer( m_dfb, &m_events );
     if (ret) {
          DirectFBError( "IDirectFB::CreateEventBuffer() failed", ret );
          goto error;
     }

     return DFB_OK;


error:
     if (m_layer) {
          m_layer->Release( m_layer );
          m_layer = NULL;
     }

     if (m_dfb) {
          m_dfb->Release( m_dfb );
          m_dfb = NULL;
     }

     return ret;
}

static void
destroy_directfb()
{
     D_ASSERT( m_events != NULL );
     D_ASSERT( m_layer != NULL );
     D_ASSERT( m_dfb != NULL );

     m_events->Release( m_events );
     m_layer->Release( m_layer );
     m_dfb->Release( m_dfb );

     m_events = NULL;
     m_layer  = NULL;
     m_dfb    = NULL;
}

#ifdef USECLUTs
static void install_palette(IDirectFBPalette *dfbpalette)
{
  DFBColor  colors[256];
  int i = 0;

//0xff, 0x00, 0x00,   0xff
//0xff, 0xff, 0xff,   0xff
//0xff, 0x00, 0xff,   0xff
//0xff, 0xff, 0x00,   0xff
//0x00, 0x04, 0x00,   0x00


  colors[i].a = 0xff;
  colors[i].r = 0xff;
  colors[i].g = 0x00;
  colors[i++].b = 0x00;
  colors[i].a = 0xff;
  colors[i].r = 0xff;
  colors[i].g = 0xff;
  colors[i++].b = 0xff;
  colors[i].a = 0xff;
  colors[i].r = 0xff;
  colors[i].g = 0x00;
  colors[i++].b = 0xff;
  colors[i].a = 0xff;
  colors[i].r = 0xff;
  colors[i].g = 0xff;
  colors[i++].b = 0x00;
  colors[i].a = 0x00;
  colors[i].r = 0x00;
  colors[i].g = 0x04;
  colors[i++].b = 0x00;

  dfbpalette->SetEntries( dfbpalette, colors, 256, 0 );
}
#endif

/**************************************************************************************************/

static DFBResult
init_window( DemoWindow *dw,
             int width, int height, u32 key,
             DFBWindowStackingClass stacking,
             DemoWindow *parent )
{
     DFBResult             ret;
     DFBWindowDescription  desc;
     IDirectFBWindow       *window;
//     IDirectFBSurface      *surface;
#ifdef USECLUT
//     IDirectFBPalette      *wpalette;
#endif

     D_ASSERT( dw != NULL );
     D_ASSERT( width > 0 );
     D_ASSERT( height > 0 );
     D_MAGIC_ASSERT_IF( parent, DemoWindow );

     /* Fill window description. */
     desc.flags     = DWDESC_WIDTH    | DWDESC_HEIGHT | DWDESC_CAPS    |
                      DWDESC_STACKING | DWDESC_PARENT | DWDESC_OPTIONS | DWDESC_PIXELFORMAT;
     desc.width     = width;
     desc.height    = height;
     desc.caps      = DWCAPS_INPUTONLY;
     desc.stacking  = stacking;
     desc.parent_id = parent ? parent->window_id : 0;
#ifdef USECLUT
     desc.options   = DWOP_NONE;
     desc.pixelformat = DSPF_LUT8;
#else
     desc.options   = key    ? DWOP_COLORKEYING  : DWOP_NONE;
     desc.pixelformat = DSPF_RGB16;
#endif

     /* Create the window. */
     ret = m_layer->CreateWindow( m_layer, &desc, &window );
     if (ret) {
          DirectFBError( "IDirectFBDisplayLayer::CreateWindow() failed", ret );
          return ret;
     }
#if 0
     ret = window->GetSurface( window, &surface );
     if (ret) {
          DirectFBError( "IDirectFBWindow::GetSurface() failed", ret );
          window->Release( window );
          return ret;
     }

#ifdef USECLUT
     surface->GetPalette( surface, &wpalette );
     install_palette(wpalette);
     surface->Clear( surface, (key >> 24) & 0xff, (key >> 16) & 0xff, key & 0xff, 0xff ); // ERROR introduced by purpose
//     surface->Clear( surface, (key >> 24) & 0xff, (key >> 16) & 0xff, key & 0xff, 0x00 );
#else
     window->SetColorKey( window, (key >> 24) & 0xff, (key >> 16) & 0xff, key & 0xff );
     surface->Clear( surface, (key >> 24) & 0xff, (key >> 16) & 0xff, key & 0xff, 0xff );
#endif
#endif


     window->AttachEventBuffer( window, m_events );

     window->GetID( window, &dw->window_id );

     /* Return new interfaces. */
     dw->window  = window;
//     dw->surface = surface;
#ifdef USECLUT
//     dw->palette = wpalette;
#endif

     D_MAGIC_SET( dw, DemoWindow );

     return DFB_OK;
}

static void
show_window( DemoWindow *dw,
             bool        visible )
{
     IDirectFBWindow *window;

     D_MAGIC_ASSERT( dw, DemoWindow );

     window = dw->window;
     D_ASSERT( window != NULL );

     window->SetOpacity( window, visible ? 0xff : 0x00 );
}

static void
destroy_window( DemoWindow *dw )
{
     IDirectFBSurface *surface;
     IDirectFBWindow  *window;
#ifdef USECLUT
     IDirectFBPalette  *wpalette;
#endif

     D_MAGIC_ASSERT( dw, DemoWindow );

     surface = dw->surface;
     D_ASSERT( surface != NULL );

     window = dw->window;
     D_ASSERT( window != NULL );

#ifdef USECLUT
     wpalette = dw->palette;
     D_ASSERT( wpalette != NULL );
#endif


     window->DetachEventBuffer( window, m_events );

#ifdef USECLUT
     wpalette->Release( wpalette );
#endif
     surface->Release( surface );
     window->Release( window );

#ifdef USECLUT
     dw->palette = NULL;
#endif
     dw->surface = NULL;
     dw->window  = NULL;

     D_MAGIC_CLEAR( dw );
}

/**************************************************************************************************/

static void
dispatch_key( DFBWindowEvent *event )
{
/*     static const DFBRectangle rects[] = {
          { 0, 0, 10,  1 },
          { 0, 1,  1,  9 },

          { 290, 0, 10, 1 },
          { 299, 1,  1, 9 },

          { 290, 299, 10, 1 },
          { 299, 290,  1, 9 },

          { 0, 299, 10, 1 },
          { 0, 290,  1, 9 },


          { 100,   0, 100,   1 },
          { 100, 299, 100,   1 },
          {   0, 100,   1, 100 },
          { 299, 100,   1, 100 },

          { 150,   0,   1,  70 },
          { 150, 229,   1,  70 },
          {   0, 150,  70,   1 },
          { 229, 150,  70,   1 },
     };*/
     DFBWindowGeometry src;
     DFBWindowGeometry dst;

     if (event->type != DWET_KEYDOWN)
          return;

     switch (event->key_symbol) {
          case DIKS_CURSOR_UP:
               if (attached_window.window) {
                    src.mode = DWGM_LOCATION;
                    src.location.x = 0.0f;
                    src.location.y = 0.0f;
                    src.location.w = 1.0f;
                    src.location.h = 1.0f;
                    main_window.window->SetSrcGeometry( main_window.window, &src );                   
                    printf("UP\n");
               }
               break;

          case DIKS_CURSOR_DOWN:
               if (attached_window.window) {
                    src.mode = DWGM_LOCATION;
                    src.location.x = 0.4f;
                    src.location.y = 0.0f;
                    src.location.w = 0.2f;
                    src.location.h = 1.0f;
                    main_window.window->SetSrcGeometry( main_window.window, &src );   
                    printf("DOWN\n");

               }
               break;

          case DIKS_CURSOR_LEFT:
               if (attached_window.window) {
                    dst.mode = DWGM_LOCATION;
                    dst.location.x = 0.0f;
                    dst.location.y = 0.0f;
                    dst.location.w = 1.0f;
                    dst.location.h = 1.0f;
                    main_window.window->SetDstGeometry( main_window.window, &dst );                   
                    printf("LEFT\n");
               }
               break;

          case DIKS_CURSOR_RIGHT:
               if (attached_window.window) {
                    dst.mode = DWGM_LOCATION;
                    dst.location.x = 0.4f;
                    dst.location.y = 0.0f;
                    dst.location.w = 0.2f;
                    dst.location.h = 1.0f;
                    main_window.window->SetDstGeometry( main_window.window, &dst );   
                    printf("RIGHT\n");

               }
               break;

          case DIKS_OK:
               if (attached_window.window) {
                    destroy_window( &attached_window );
               }
               else {

//                    init_window( &attached_window, 300, 300, 0x000400, DWSC_MIDDLE, &main_window );
                    init_window( &attached_window, 300, 300, 0x000000, DWSC_MIDDLE, &main_window );

                    dst.mode = DWGM_FOLLOW;
                    attached_window.window->SetDstGeometry( attached_window.window, &dst );
//                    src.mode = DWGM_FOLLOW;
//                    attached_window.window->SetSrcGeometry( attached_window.window, &src );

/*                    attached_window.surface->SetColor( attached_window.surface,
                                                       0xff, 0x00, 0x00, 0xff );

                    attached_window.surface->FillRectangle( attached_window.surface,
                                                            100, 100, 100, 100 );

                    attached_window.surface->SetColor( attached_window.surface,
                                                       0xff, 0xff, 0xff, 0xff );

                    attached_window.surface->FillRectangles( attached_window.surface,
                                                             rects, D_ARRAY_SIZE(rects) );

                    attached_window.surface->Flip( attached_window.surface, NULL, DSFLIP_NONE );
*/
                    show_window( &attached_window, true );
               }
               break;

          default:
               break;
     }
}

static DFBBoolean
dispatch_events()
{
     DFBWindowEvent event;

     while (m_events->GetEvent( m_events, DFB_EVENT(&event) ) == DFB_OK) {
          switch (event.type) {
               case DWET_CLOSE:
               case DWET_DESTROYED:
                    return DFB_FALSE;

               case DWET_KEYUP:
               case DWET_KEYDOWN:
                    dispatch_key( &event );
                    break;

               default:
                    break;
          }
     }

     return DFB_TRUE;
}

/**************************************************************************************************/

int
main( int argc, char *argv[] )
{
     DFBWindowGeometry dst;
     DFBWindowGeometry src;
/*     static const DFBRectangle rects[] = {
          { 0, 0, 10,  1 },
          { 0, 1,  1,  9 },

          { 290, 0, 10, 1 },
          { 299, 1,  1, 9 },

          { 290, 299, 10, 1 },
          { 299, 290,  1, 9 },

          { 0, 299, 10, 1 },
          { 0, 290,  1, 9 },


          { 100,   0, 100,   1 },
          { 100, 299, 100,   1 },
          {   0, 100,   1, 100 },
          { 299, 100,   1, 100 },

          { 150,   0,   1,  70 },
          { 150, 229,   1,  70 },
          {   0, 150,  70,   1 },
          { 229, 150,  70,   1 },
     };
*/

     if (init_directfb( &argc, &argv ))
          return -1;

     if (init_window( &main_window, 300, 300, 0x000400, DWSC_UPPER, NULL )) {
//     if (init_window( &main_window, 300, 300, 0x000400, DWSC_LOWER, NULL )) {
//     if (init_window( &main_window, 300, 300, 0x000400, DWSC_MIDDLE, NULL )) {
          destroy_directfb();
          return -2;
     }

     dst.mode = DWGM_LOCATION;
     dst.location.x = 0.2f;
     dst.location.y = 0.0f;
     dst.location.w = 0.6f;
     dst.location.h = 1.0f;
     main_window.window->SetDstGeometry( main_window.window, &dst );

     
     src.mode = DWGM_LOCATION;
     src.location.x = 0.2f;
     src.location.y = 0.0f;
     src.location.w = 0.6f;
     src.location.h = 1.0f;
     main_window.window->SetSrcGeometry( main_window.window, &src );


/*     main_window.surface->SetColor( main_window.surface,
                                                       0xff, 0x00, 0xff, 0xff );

     main_window.surface->FillRectangle( main_window.surface,
                                                            125, 125, 50, 50 );
     main_window.surface->SetColor( main_window.surface,
                                                       0xff, 0xff, 0x00, 0xff );

     main_window.surface->FillRectangles( main_window.surface,
                                                             rects, D_ARRAY_SIZE(rects) );


     main_window.surface->Flip( main_window.surface, NULL, DSFLIP_NONE );
*/

     show_window( &main_window, true );

     while (dispatch_events())
          m_events->WaitForEvent( m_events );

     destroy_window( &main_window );

     destroy_directfb();

     return 0;
}

