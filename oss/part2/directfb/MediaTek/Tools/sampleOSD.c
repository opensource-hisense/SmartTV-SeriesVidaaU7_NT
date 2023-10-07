/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <neo@directfb.org>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <directfb.h>
#include <directfb_util.h>
#include <fusiondale.h>

#include <direct/clock.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**********************************************************************************************************************/

static IDirectFB             *dfb     = NULL;
static IDirectFBFont         *font    = NULL;
static IDirectFBDisplayLayer *layer   = NULL;
static IDirectFBWindow       *window  = NULL;
static IDirectFBSurface      *surface = NULL;
static IDirectFBEventBuffer  *events  = NULL;

/**********************************************************************************************************************/

static IFusionDale           *pDale      = NULL;
static IFusionDaleMessenger  *pMessenger = NULL;

static FDMessengerEventID     gTuningEventID;
static FDMessengerEventID     gVolumeEventID;
static FDMessengerEventID     gAppRunEventID;
static FDMessengerEventID     gAppKillEventID;

/**********************************************************************************************************************/

static void init_application( int *argc, char **argv[] );
static void exit_application( int status );

static void InitFusionDale();

static void update_cpu();
static void update_vol( int volume );
static void update_text( const char *text );

/**********************************************************************************************************************/

static long long msg_stamp;

int
main( int argc, char *argv[] )
{
     bool show_msg = false;

     /* Initialize application. */
     init_application( &argc, &argv );

     InitFusionDale();

     surface->Clear( surface, 0x10, 0x10, 0x10, 0xe0 );

     surface->SetColor( surface, 0x40, 0x60, 0xb0, 0xe0 );
     surface->DrawRectangle( surface, 0, 0, 64, 64 );

     surface->SetColor( surface, 0x50, 0x90, 0xd0, 0xe0 );
     surface->DrawRectangle( surface, 0, 0, 300, 100 );

     update_text( "Philips TV\nDemonstrator" );

     update_cpu();
     update_vol( 10 );

     msg_stamp = direct_clock_get_millis();

     /* Main loop. */
     while (1) {
          if (events->WaitForEventWithTimeout( events, 0, 300 ) == DFB_TIMEOUT) {
               long long diff = direct_clock_get_millis() - msg_stamp;
               /* hide? */
               if (diff > 5000) {
                    if (argc < 2 || strcmp(argv[1],"-s"))
                         window->SetOpacity( window, 0x00 );
               }
               else if (diff > 2000) {
                    if (show_msg) {
                         show_msg = false;
                         update_text( "Philips TV\nDemonstrator" );
                    }
               }
               else if (!show_msg) {
                    show_msg = true;
                    window->SetOpacity( window, 0xff );
               }
          }
          else {
               DFBEvent event;

               /* Check for new events. */
               while (events->GetEvent( events, DFB_EVENT(&event) ) == DFB_OK) {
                    switch (event.clazz) {
                         case DFEC_USER:
                              break;

                         default:
                              break;
                    }
               }
          }

          update_cpu();
     }

     /* Shouldn't reach this. */
     return 0;
}

/******************************************************************************/

static void
init_application( int *argc, char **argv[] )
{
     DFBResult             ret;
     DFBFontDescription    fdsc;
     DFBWindowDescription  desc;
     DFBDisplayLayerConfig config;

     /* Initialize DirectFB including command line parsing. */
     ret = DirectFBInit( argc, argv );
     if (ret) {
          DirectFBError( "DirectFBInit() failed", ret );
          exit_application( 1 );
     }

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate() failed", ret );
          exit_application( 2 );
     }

     /* Get the primary display layer. */
     ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
     if (ret) {
          DirectFBError( "IDirectFB::GetDisplayLayer() failed", ret );
          exit_application( 3 );
     }

     /* Get the screen size etc. */
     layer->GetConfiguration( layer, &config );

     /* Fill the window description. */
     desc.flags  = DWDESC_POSX | DWDESC_POSY | DWDESC_STACKING |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS;
     desc.posx   = 1000;
     desc.posy   = 100;
     desc.width  = 300;
     desc.height = 100;
     desc.stacking = DWSC_UPPER;
     desc.caps   = DWCAPS_ALPHACHANNEL | DWCAPS_NODECORATION;

     /* Create the window. */
     ret = layer->CreateWindow( layer, &desc, &window );
     if (ret) {
          DirectFBError( "IDirectFBDisplayLayer::CreateWindow() failed", ret );
          exit_application( 4 );
     }

     /* Get the window's surface. */
     ret = window->GetSurface( window, &surface );
     if (ret) {
          DirectFBError( "IDirectFBWindow::GetSurface() failed", ret );
          exit_application( 5 );
     }

     /* Create an event buffer for all keyboard events. */
     ret = window->CreateEventBuffer( window, &events );
     if (ret) {
          DirectFBError( "IDirectFBWindow::CreateEventBuffer() failed", ret );
          exit_application( 6 );
     }

     /* Add ghost option (behave like an overlay). */
     window->SetOptions( window, DWOP_ALPHACHANNEL | DWOP_GHOST );

     /* Move window to upper stacking class. */
     window->SetStackingClass( window, DWSC_UPPER );

     /* Make it the top most window. */
     window->RaiseToTop( window );


     /* Load the font. */
     fdsc.flags  = DFDESC_HEIGHT;
     fdsc.height = 33;

     ret = dfb->CreateFont( dfb, "data/decker.ttf", &fdsc, &font );
     if (ret) {
          DirectFBError( "IDirectFB::CreateFont() failed", ret );
          exit_application( 7 );
     }

     surface->SetFont( surface, font );
}

static void
exit_application( int status )
{
     /* Release the event buffer. */
     if (events)
          events->Release( events );

     /* Release the window's surface. */
     if (surface)
          surface->Release( surface );

     /* Release the window. */
     if (window)
          window->Release( window );

     /* Release the layer. */
     if (layer)
          layer->Release( layer );

     /* Release the super interface. */
     if (dfb)
          dfb->Release( dfb );

     /* Terminate application. */
     exit( status );
}

/******************************************************************************/

#define SET_IF_DESIRED(x,y) do{  if(x) *(x) = (y); }while(0)
#define JT unsigned long
static void four_cpu_numbers(int *uret, int *nret, int *sret, int *iret)
{
     int       tmp_u, tmp_n, tmp_s, tmp_i;
     static JT old_u, old_n, old_s, old_i, old_wa, old_hi, old_si;
     JT        new_u, new_n, new_s, new_i, new_wa = 0, new_hi = 0, new_si = 0;
     JT        ticks_past; /* avoid div-by-0 by not calling too often :-( */
     char      dummy[16];
     FILE     *stat;

     stat = fopen ("/proc/stat", "r");
     if (!stat)
          return;

     if (fscanf (stat, "%s %lu %lu %lu %lu %lu %lu %lu", dummy,
                 &new_u, &new_n, &new_s, &new_i, &new_wa, &new_hi, &new_si) < 5)
     {
          fclose (stat);
          return;
     }

     fclose (stat);

     ticks_past = ((new_u + new_n + new_s + new_i + new_wa + new_hi + new_si) -
                   (old_u + old_n + old_s + old_i + old_wa + old_hi + old_si));
     if (ticks_past) {
          tmp_u = ((new_u - old_u) << 16) / ticks_past;
          tmp_n = ((new_n - old_n) << 16) / ticks_past;
          tmp_s = ((new_s - old_s) << 16) / ticks_past;
          tmp_i = ((new_i - old_i) << 16) / ticks_past;
     }
     else {
          tmp_u = 0;
          tmp_n = 0;
          tmp_s = 0;
          tmp_i = 0;
     }

     SET_IF_DESIRED(uret, tmp_u);
     SET_IF_DESIRED(nret, tmp_n);
     SET_IF_DESIRED(sret, tmp_s);
     SET_IF_DESIRED(iret, tmp_i);

     old_u  = new_u;
     old_n  = new_n;
     old_s  = new_s;
     old_i  = new_i;
     old_wa = new_wa;
     old_hi = new_hi;
     old_si = new_si;
}
#undef JT

static int
get_load()
{
     static int old_load = 0;

     int u = 0, n = 0, s = 0, i, load;

     four_cpu_numbers( &u, &n, &s, &i );

     load = u + n + s;

     old_load = (load + load + load + old_load) >> 2;

     return old_load >> 10;
}

static void
update_cpu()
{
     int          load = get_load();
     DFBRectangle rect = { 2, 1, 61, 62 };
     DFBRegion    region = { 1, 1, 62, 62 };

     surface->Blit( surface, surface, &rect, 1, 1 );

     switch (load) {
          case 0:
               surface->SetColor( surface, 0x30, 0x30, 0x30, 0xd0 );
               surface->FillRectangle( surface, 62, 1, 1, 62 );
               break;

          case 63:
          case 64:
               surface->SetColor( surface, 0x00, 0x70, 0xe0, 0xee );
               surface->FillRectangle( surface, 62, 1, 1, 62 );
               break;

          default:
               surface->SetColor( surface, 0x30, 0x30, 0x30, 0xd0 );
               surface->FillRectangle( surface, 62, 1, 1, 62 - load );

               surface->SetColor( surface, 0x00, 0x70, 0xe0, 0xee );
               surface->FillRectangle( surface, 62, 1 + 62 - load, 1, load );
               break;
     }

     surface->Flip( surface, &region, 0 );
}

static void
update_vol( int volume )
{
     DFBRectangle rect   = { 2, 94, 296, 4 };
     DFBRegion    region = { 2, 94, 297, 197 };
     int          vw     = rect.w * volume / 100;

     surface->SetColor( surface, 0x30, 0xa0, 0xc0, 0xee );
     surface->FillRectangle( surface, rect.x, rect.y, vw, 4 );

     surface->SetColor( surface, 0x50, 0x50, 0x50, 0xd0 );
     surface->FillRectangle( surface, rect.x + vw, rect.y, rect.w - vw, 4 );

     surface->Flip( surface, &region, 0 );
}

static void
update_text( const char *text )
{
     const char *nl = strchr( text, '\n' );

     DFBRegion   region = { 72, 2, 297, 79 };


     surface->SetColor( surface, 0x10, 0x10, 0x10, 0xe0 );
     surface->FillRectangle( surface, DFB_RECTANGLE_VALS_FROM_REGION(&region) );


     surface->SetColor( surface, 0x60, 0xb0, 0xc0, 0xff );

     if (nl) {
          surface->DrawString( surface, text, (unsigned long)nl - (unsigned long)text, 74, 2, DSTF_TOPLEFT );
          surface->DrawString( surface, nl+1, -1, 74, 40, DSTF_TOPLEFT );
     }
     else
          surface->DrawString( surface, text, -1, 74, 2, DSTF_TOPLEFT );


     surface->Flip( surface, &region, 0 );
}

/**********************************************************************************************************************/

static void
TuningCallback( FDMessengerEventID  event_id,
                int                 param,
                void               *data,
                int                 data_size,
                void               *context )
{
     char buf[64];

     msg_stamp = direct_clock_get_millis();

     snprintf( buf, sizeof(buf), "APID   0x%x\nVPID   0x%x", param, param + 3 );

     update_text( buf );
}

static void
VolumeCallback( FDMessengerEventID  event_id,
                int                 param,
                void               *data,
                int                 data_size,
                void               *context )
{
     msg_stamp = direct_clock_get_millis();

     update_vol( param );
}

static void
AppRunCallback( FDMessengerEventID  event_id,
                int                 param,
                void               *data,
                int                 data_size,
                void               *context )
{
     char  buf[64];
     char *name = data;

     msg_stamp = direct_clock_get_millis();

     snprintf( buf, sizeof(buf), "Starting...\n%s (%d)", name, param );

     update_text( buf );
}

static void
AppKillCallback( FDMessengerEventID  event_id,
                 int                 param,
                 void               *data,
                 int                 data_size,
                 void               *context )
{
     char  buf[64];
     char *name = data;

     msg_stamp = direct_clock_get_millis();

     snprintf( buf, sizeof(buf), "Killing...\n%s (%d)", name, param );

     update_text( buf );
}

/**********************************************************************************************************************/

static void
InitFusionDale()
{
     DFBResult          ret;
     FDMessengerListenerID tuninglistener_id;
     FDMessengerListenerID volumelistener_id;
     FDMessengerListenerID apprunlistener_id;
     FDMessengerListenerID appkilllistener_id;

     /* Initialize FusionDale config etc. */
     ret = FusionDaleInit( NULL, NULL );
     if (ret)
          FusionDaleErrorFatal( "FusionDaleInit", ret );

     /* Create the super interface. */
     ret = FusionDaleCreate( &pDale );
     if (ret)
          FusionDaleErrorFatal( "FusionDaleCreate", ret );

     /* Create the event manager. */
     ret = pDale->GetMessenger( pDale, &pMessenger );
     if (ret)
          FusionDaleErrorFatal( "IFusionDale::CreateMessenger", ret );

     /* Register a new event for tuning notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Tuning Event", &gTuningEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );

     /* Register a new event for volume change notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Volume Event", &gVolumeEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );

     /* Register a new event for application starting notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Application Run", &gAppRunEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );

     /* Register a new event for application killing notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Application Kill", &gAppKillEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );


     ret = pMessenger->RegisterListener( pMessenger, gTuningEventID, TuningCallback, NULL, &tuninglistener_id );
     if (ret)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterListener", ret );

     ret = pMessenger->RegisterListener( pMessenger, gVolumeEventID, VolumeCallback, NULL, &volumelistener_id );
     if (ret)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterListener", ret );

     ret = pMessenger->RegisterListener( pMessenger, gAppRunEventID, AppRunCallback, NULL, &apprunlistener_id );
     if (ret)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterListener", ret );

     ret = pMessenger->RegisterListener( pMessenger, gAppKillEventID, AppKillCallback, NULL, &appkilllistener_id );
     if (ret)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterListener", ret );
}

