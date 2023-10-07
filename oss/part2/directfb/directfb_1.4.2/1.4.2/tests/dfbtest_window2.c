/*
   (c) Copyright 2001-2009  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include <directfb.h>

#include <sys/time.h>

#define TRUE  1
#define FALSE 0
#define LOOPS 1000


/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          int err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

#define ABS(a) (a>=0?a:-a)

static DFBSurfaceFlipFlags flip_mode = DSFLIP_NONE;

int main( int argc, char *argv[] )
{
    DFBResult ret = DFB_OK;
    IDirectFB              *dfb;
    IDirectFBDisplayLayer *layer = NULL;


    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    DFBWindowID       window_id;



    DirectFBInit( &argc, &argv );
    DirectFBCreate( &dfb );
    DFBDisplayLayerConfig   layer_config;

    printf("./dfbtest_window2 0 : normal\n./dfbtest_window2 1 : alphachannel\n");

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );

    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );

    }

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

    // set layer cap.
    layer_config.flags = ( DLCONF_PIXELFORMAT | DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config.pixelformat = DSPF_ARGB;
    layer_config.width = 1280;
    layer_config.height = 720;



    layer->SetConfiguration( layer, &layer_config );


    // red window
    memset(&desc[0], 0, sizeof(DFBWindowDescription));
    desc[0].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[0].posx   = 100;
    desc[0].posy   = 100;
    desc[0].width  = 200;
    desc[0].height = 200;

    if(atoi(argv[1]) == 1) {
        desc[0].caps = DWCAPS_ALPHACHANNEL;
        desc[0].flags |= DWDESC_CAPS;
    }

    DFBCHECK(layer->CreateWindow( layer, &desc[0], &window[0] ));
    window[0]->GetSurface( window[0], &window_surface[0] );
    window[0]->SetOpacity( window[0], 0xFF );

    if(atoi(argv[1]) == 1) {
        window[0]->SetOpacity( window[0], 0x88 );
    }

    window_surface[0]->SetColor( window_surface[0], 0xFF, 0x00, 0x00, 0xFF );
    window_surface[0]->FillRectangle( window_surface[0],
                                          0, 0,
                                          200, 200 );

    // green window
    memset(&desc[1], 0, sizeof(DFBWindowDescription));
    desc[1].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[1].posx   = 220;
    desc[1].posy   = 100;
    desc[1].width  = 200;
    desc[1].height = 200;

    if(atoi(argv[1]) == 1) {
        desc[1].caps = DWCAPS_ALPHACHANNEL;
        desc[1].flags |= DWDESC_CAPS;
    }

    DFBCHECK(layer->CreateWindow( layer, &desc[1], &window[1] ));
    window[1]->GetSurface( window[1], &window_surface[1] );
    window[1]->SetOpacity( window[1], 0xFF );

    if(atoi(argv[1]) == 1) {
        window[1]->SetOpacity( window[1], 0x88 );
    }

    window_surface[1]->SetColor( window_surface[1], 0x00, 0xFF, 0x00, 0xFF );
    window_surface[1]->FillRectangle( window_surface[1],
                                          0, 0,
                                          200, 200 );

    // white window
    memset(&desc[2], 0, sizeof(DFBWindowDescription));
    desc[2].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[2].posx   = 150;
    desc[2].posy   = 250;
    desc[2].width  = 200;
    desc[2].height = 200;

    if(atoi(argv[1]) == 1) {
        desc[2].caps = DWCAPS_ALPHACHANNEL;
        desc[2].flags |= DWDESC_CAPS;
    }

    DFBCHECK( layer->CreateWindow( layer, &desc[2], &window[2] ) );
    window[2]->GetSurface( window[2], &window_surface[2] );
    window[2]->SetOpacity( window[2], 0xFF );

    if(atoi(argv[1]) == 1) {
        window[2]->SetOpacity( window[2], 0x88 );
    }

    window_surface[2]->SetColor( window_surface[2], 0xFF, 0xFF, 0xFF, 0xFF );
    window_surface[2]->FillRectangle( window_surface[2],
                                          0, 0,
                                          200, 200 );

    // yellow window
    memset(&desc[3], 0, sizeof(DFBWindowDescription));
    desc[3].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[3].posx   = 450;
    desc[3].posy   = 300;
    desc[3].width  = 100;
    desc[3].height = 200;

    if(atoi(argv[1]) == 1) {
        desc[3].caps = DWCAPS_ALPHACHANNEL;
        desc[3].flags |= DWDESC_CAPS;
    }

    DFBCHECK( layer->CreateWindow( layer, &desc[3], &window[3] ) );
    window[3]->GetSurface( window[3], &window_surface[3] );
    window[3]->SetOpacity( window[3], 0xFF );

    if(atoi(argv[1]) == 1) {
        window[3]->SetOpacity( window[3], 0x88 );
    }

    window_surface[3]->SetColor( window_surface[3], 0xFF, 0xFF, 0x00, 0xFF );
    window_surface[3]->FillRectangle( window_surface[3],
                                          0, 0,
                                          100, 200 );
    sleep(3);

/*---------------window id---------------*/
    window[0]->GetID( window[0], &window_id );
    printf("window 0 id: %d\n", window_id);
    window[1]->GetID( window[1], &window_id );
    printf("window 1 id: %d\n", window_id);
    window[2]->GetID( window[2], &window_id );
    printf("window 2 id: %d\n", window_id);
    window[3]->GetID( window[3], &window_id );
    printf("window 3 id: %d\n", window_id);

/*---------------SetOpacity---------------*/
    window[2]->SetOpacity( window[2], 0x00 );
    sleep(2);
    window[2]->SetOpacity( window[2], 0xFF );
    sleep(2);
    window[2]->SetOpacity( window[2], 0x55 );
    sleep(2);
    window[2]->SetOpacity( window[2], 0xFF );

/*---------------window stack---------------*/
    int k;

    window[2]->Lower( window[2] );
    sleep(1);
    window[0]->Raise( window[0] );
    sleep(1);
    window[0]->Raise( window[0] );
    sleep(1);
    window[1]->Lower( window[1] );
    sleep(1);

    window[1]->Raise( window[1] );
    sleep(1);
    window[2]->SetStackingClass( window[2], DWSC_UPPER );
    window[2]->RaiseToTop( window[2] );
    sleep(1);


/*---------------window move---------------*/

     DFBPoint pos;
     window[2]->GetPosition( window[2], &pos.x, &pos.y );
     printf("window[2] position: x%d, y%d\n",pos.x, pos.y);
     DFBPoint poss[] = { { pos.x - 40, pos.y - 40 },
                              { pos.x + 40, pos.y - 40 },
                              { pos.x + 40, pos.y + 40 },
                              { pos.x - 40, pos.y + 40 },
                              { pos.x     , pos.y      } };

     for (k=0; k <D_ARRAY_SIZE(poss); k++) {
          window[2]->MoveTo( window[2], poss[k].x, poss[k].y );
          sleep(1);
     }

/*---------------window resize---------------*/
    window[0]->SetStackingClass( window[0], DWSC_UPPER );
    window[0]->RaiseToTop(  window[0] );
    DFBDimension     size;
    window[0]->GetSize( window[0], &size.w, &size.h );
    window[0]->GetOptions( window[0], &opts );
    window[0]->SetOptions( window[0], opts | DWOP_SCALE );
    DFBDimension sizes[] = { { size.w + 40, size.h      },
                                   { size.w + 80, size.h      },
                                   { size.w + 80, size.h + 40 },
                                   { size.w + 40, size.h + 40 },
                                   { size.w,      size.h      } };
    for(k = 0; k < D_ARRAY_SIZE(sizes) ;k ++) {
        window[0]->Resize( window[0], sizes[k].w, sizes[k].h );
        sleep(1);
    }

    sleep(3);

/*---------------Release the window---------------*/
    int i;
    for(i=0; i< 4; i++)
    {
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


/*---------------Release the layer---------------*/

    layer->Release( layer );


    dfb->Release( dfb );


    return 42;
}