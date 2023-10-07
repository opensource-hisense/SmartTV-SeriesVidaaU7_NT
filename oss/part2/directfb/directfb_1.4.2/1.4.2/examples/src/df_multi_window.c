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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <directfb.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

static IDirectFB *dfb;
static IDirectFBDisplayLayer *layer;
static IDirectFBWindow * window;
static IDirectFBSurface *window_surface;
static IDirectFBEventBuffer *buffer;

int main( int argc, char *argv[] )
{
//    DFBResult                    ret;
    int err;
    DFBGraphicsDeviceDescription gdesc;
    DFBDisplayLayerConfig   layer_config;
    DFBWindowDescription  desc;
    u8 u8R, u8G, u8B, u8A;
    u32 u32Idx;

    srand((int)time(0));

    DFBCHECK(DirectFBInit( &argc, &argv ));

    /* create the super interface */
    DFBCHECK(DirectFBCreate( &dfb ));

    //Set Co-operative mode:
    dfb->GetDeviceDescription(dfb, &gdesc);
    DFBCHECK(dfb->GetDisplayLayer(dfb, DLID_PRIMARY, &layer));

    layer->SetCooperativeLevel( layer, DLSCL_SHARED );//DLSCL_ADMINISTRATIVE );

    layer->GetConfiguration(layer, &layer_config );

    layer->EnableCursor( layer, 1 );

    desc.flags = ( DWDESC_POSX | DWDESC_POSY |
                    DWDESC_WIDTH | DWDESC_HEIGHT );

    desc.posx   = (int)(1266.0*rand()/(RAND_MAX+1.0));
    desc.posy   = (int)(668.0*rand()/(RAND_MAX+1.0));
    desc.width  = 0x100;
    desc.height = 0x100;

    DFBCHECK( layer->CreateWindow( layer, &desc, &window ) );
    window->SetOpacity( window, 0xFF );
    window->CreateEventBuffer( window, &buffer );

    window->GetSurface(window, &window_surface);

    u8R = 0x0;
    u8G = 0x0;
    u8B = 0x0;
    u8A = 0xFF;

    for(u32Idx=0; ; u32Idx++)
    {
        u8R = (int)(255.0*rand()/(RAND_MAX+1.0));
        u8G = (int)(255.0*rand()/(RAND_MAX+1.0));
        u8B = (int)(255.0*rand()/(RAND_MAX+1.0));

        window_surface->SetColor(window_surface, u8R, u8G, u8B, u8A);
        window_surface->FillRectangle(window_surface, 0x0, 0x0, desc.width, desc.height);
        window_surface->Flip(window_surface, NULL, 0);
    }

    window_surface->Release(window_surface);
    window->Release(window);
    layer->Release(layer);
    dfb->Release(dfb);

    return 0;
}
