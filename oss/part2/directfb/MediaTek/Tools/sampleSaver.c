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

#include <direct/clock.h>
#include <direct/types.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#define CHECK(x)                                  \
     do {                                         \
          DFBResult ret = (x);                    \
          if (ret)                                \
               DirectFBErrorFatal(#x,ret);        \
     } while (0)

/**********************************************************************************************************************/

static IDirectFB             *dfb     = NULL;
static IDirectFBSurface      *surface = NULL;
static IDirectFBSurface      *logo    = NULL;
static IDirectFBEventBuffer  *events  = NULL;

/**********************************************************************************************************************/

static void
init_surface()
{
     DFBSurfaceDescription desc;

     if (surface)
          return;

     dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

     desc.flags = DSDESC_CAPS;
     desc.caps  = DSCAPS_TRIPLE | DSCAPS_PRIMARY;

     /* Get the primary surface. */
     CHECK( dfb->CreateSurface( dfb, &desc, &surface ) );
}

static void
deinit_surface()
{
     dfb->SetCooperativeLevel( dfb, DFSCL_NORMAL );

     if (surface) {
          surface->Release( surface );
          surface = NULL;
     }
}

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     /* Initialize application. */
     DFBSurfaceDescription   desc;
     IDirectFBImageProvider *provider;

     struct sched_param param;

     bool running = false;
     int  xpos    = -99999;
     int  xpos2   = -99999;
     int  xpos3   = -99999;
     int  ypos    = 200;

     long long stamp;


     /* Initialize DirectFB including command line parsing. */
     CHECK( DirectFBInit( &argc, &argv ) );

     /* Create the super interface. */
     CHECK( DirectFBCreate( &dfb ) );

     /* Create an event buffer for all keyboard events. */
     CHECK( dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS, DFB_TRUE, &events ) );

     CHECK( dfb->CreateImageProvider( dfb, "data/philips_cut_black.png", &provider ) );
     CHECK( provider->GetSurfaceDescription( provider, &desc ) );
     CHECK( dfb->CreateSurface( dfb, &desc, &logo ) );
     CHECK( provider->RenderTo( provider, logo, NULL ) );
     provider->Release( provider );


     param.sched_priority = 99;

     pthread_setschedparam( pthread_self(), SCHED_FIFO, &param );


     stamp = direct_clock_get_millis();

     /* Main loop. */
     while (1) {
          long long now  = direct_clock_get_millis();
          long long diff = now - stamp;

          stamp = now;

          if (running) {
               int nxpos;

               if (xpos < -desc.width) {
                    xpos = 1366;
                    ypos = rand() % (768 - desc.height);

                    surface->Clear( surface, 0, 0, 0, 0 );
                    surface->Flip( surface, NULL, DSFLIP_NONE );
                    surface->Clear( surface, 0, 0, 0, 0 );
                    surface->Flip( surface, NULL, DSFLIP_NONE );
                    surface->Clear( surface, 0, 0, 0, 0 );
               }

               nxpos = xpos - diff / 4;

               surface->FillRectangle( surface, nxpos + desc.width, ypos, xpos3 - nxpos, desc.height );
               surface->Blit( surface, logo, NULL, nxpos, ypos );
               surface->Flip( surface, NULL, DSFLIP_ONSYNC );
               usleep(1);

               xpos3 = xpos2;
               xpos2 = xpos;
               xpos = nxpos;

               if (events->HasEvent( events ) == DFB_OK) {
                    deinit_surface();
                    running = false;
               }
          }
          else if (events->WaitForEventWithTimeout( events, 10, 0 ) == DFB_TIMEOUT) {
               init_surface();
               running = true;
               xpos    = -99999;
               stamp   = direct_clock_get_millis();
          }

          events->Reset( events );
     }

     /* Shouldn't reach this. */
     return 0;
}

