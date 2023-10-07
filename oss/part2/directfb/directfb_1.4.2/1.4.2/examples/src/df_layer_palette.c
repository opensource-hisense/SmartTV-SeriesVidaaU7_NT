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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>
#include <unistd.h>

/* the super interface */
static IDirectFB            *dfb;
static IDirectFBDisplayLayer  *layer;

/* the primary surface (surface of primary layer) */
static IDirectFBSurface     *primary;
static IDirectFBPalette     *palette;

/* Input interfaces: device and its buffer */
static IDirectFBEventBuffer *events;

static int screen_width, screen_height;

DFBColor  colors[256];

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
        {                                                                      \
           err = x;                                                            \
           if (err != DFB_OK) {                                                \
              fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );           \
              DirectFBErrorFatal( #x, err );                                   \
           }                                                                   \
        }

static void
generate_palette()
{
     int       i;
     DFBResult err;

     for (i=0; i<256; i++)
     {
        if (i % 3 == 0)
        {
          colors[i].a = 255;
          colors[i].r = 255;
          colors[i].g = 100;
          colors[i].b = 100;
        }
        if (i % 3 == 1)
        {
          colors[i].a = 255;
          colors[i].r = 100;
          colors[i].g = 255;
          colors[i].b = 100;
        }
        if (i % 3 == 2)
        {
          colors[i].a = 255;
          colors[i].r = 100;
          colors[i].g = 100;
          colors[i].b = 255;
        }
     }

}

static void
fill_surface( IDirectFBSurface *surface )
{
     DFBResult  err;
     int        x;
     int        y;
     void      *ptr;
     int        pitch;
     u8        *dst;



     DFBCHECK(surface->Lock( surface, DSLF_WRITE, &ptr, &pitch ));

    for (y=0; y<screen_height; y++)
    {
        dst = ptr + y * pitch;

        for (x=0; x<screen_width; x++)
        {
            int i = 0, area = 16;
            for ( i = area ; i > 0 ; i--)
                if ( x < screen_width/area*i)
                    dst[x] = i;
        }
    }

     DFBCHECK(surface->Unlock( surface ));
}

int
main( int argc, char *argv[] )
{
    int                    quit = 0;
    DFBResult              err;
    DFBSurfaceDescription  sdsc, ddsc;

    IDirectFBSurface       *surfaceI8;

    DFBCHECK(DirectFBInit( &argc, &argv ));

    /* create the super interface */
    DFBCHECK(DirectFBCreate( &dfb ));
    DFBCHECK(dfb->GetDisplayLayer( dfb, 0, &layer ));

    generate_palette();

    DFBPaletteDescription paletteDesc;

    paletteDesc.flags = DPDESC_ENTRIES | DPDESC_SIZE | DPDESC_CAPS;
    paletteDesc.size = 256;
    paletteDesc.caps = DPCAPS_NONE;
    paletteDesc.entries = colors;

    dfb->CreatePalette(dfb, &paletteDesc, &palette);

    DFBCHECK(palette->SetEntries( palette, colors, 256, 0 ));

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );


    DFBDisplayLayerConfig         dlc;
    dlc.flags        = DLCONF_WIDTH | DLCONF_HEIGHT |
                       DLCONF_PIXELFORMAT | DLCONF_SURFACE_CAPS;

    dlc.width        = 1920;
    dlc.height       = 1080;
    dlc.pixelformat  = DSPF_LUT8;

    dlc.surface_caps = DSCAPS_PRIMARY | DSCAPS_VIDEOONLY;

    DFBCHECK(layer->SetConfiguration( layer, &dlc ));

    DFBCHECK(layer->GetSurface( layer, &primary ));

    primary->GetSize( primary, &screen_width, &screen_height );

    fill_surface( primary );

    DFBCHECK(primary->SetPalette( primary, palette ));


    primary->Flip(primary, NULL, DSFLIP_BLIT);

    while(1);

    /* release our interfaces to shutdown DirectFB */
    palette->Release( palette );
    primary->Release( primary );
    dfb->Release( dfb );

    return 0;
}
