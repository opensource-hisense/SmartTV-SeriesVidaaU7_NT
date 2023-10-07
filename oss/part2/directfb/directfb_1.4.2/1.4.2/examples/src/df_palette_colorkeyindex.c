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

/* the primary surface (surface of primary layer) */
static IDirectFBSurface     *primary;
static IDirectFBPalette     *palette;

/* Input interfaces: device and its buffer */
static IDirectFBEventBuffer *events;

static int screen_width, screen_height;

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
     DFBColor  colors[256];

     for (i=0; i<256; i++) 
     {
        if (i % 3 == 0)
        {
          colors[i].a = 255;
          colors[i].r = 255;
          colors[i].g = 0;
          colors[i].b = 0;
        }
        if (i % 3 == 1)
        {
          colors[i].a = 255;
          colors[i].r = 0;
          colors[i].g = 255;
          colors[i].b = 0;
        }
        if (i % 3 == 2)
        {
          colors[i].a = 255;
          colors[i].r = 0;
          colors[i].g = 0;
          colors[i].b = 255;
        }
     }

     DFBCHECK(palette->SetEntries( palette, colors, 256, 0 ));
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
            int i = 0, area = 9;
            for ( i = 9 ; i > 0 ; i--)
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
    DFBSurfaceDescription  src_dsc, dst_dsc;

    IDirectFBSurface       *surfaceI8;
    IDirectFBSurface       *surfaceARGB;
    DFBColor               color_dummy;

    DFBCHECK(DirectFBInit( &argc, &argv ));

    /* create the super interface */
    DFBCHECK(DirectFBCreate( &dfb ));

    dst_dsc.flags       =   DSDESC_PIXELFORMAT   |
                            DSDESC_WIDTH         |
                            DSDESC_HEIGHT        |
                            DSDESC_CAPS;

    dst_dsc.width       = 1280;
    dst_dsc.height      = 720;
    dst_dsc.caps        = DSCAPS_PRIMARY | DSCAPS_VIDEOONLY;
    dst_dsc.pixelformat = DSPF_ARGB;

    DFBCHECK(dfb->CreateSurface( dfb, &dst_dsc, &primary ));
    primary->GetSize( primary, &screen_width, &screen_height );

    src_dsc.flags       =   DSDESC_WIDTH     |
                            DSDESC_HEIGHT    |
                            DSDESC_CAPS      |
                            DSDESC_PIXELFORMAT;

    src_dsc.width       = 1280;
    src_dsc.height      = 720;
    src_dsc.caps        = DSCAPS_VIDEOONLY;
    src_dsc.pixelformat = DSPF_LUT8;

    DFBCHECK(dfb->CreateSurface( dfb, &src_dsc, &surfaceI8 ));
    DFBCHECK(surfaceI8->GetPalette( surfaceI8, &palette ));

    generate_palette();
    fill_surface( surfaceI8 );

    int iNum = 0;

    while( iNum < 1 || iNum > 9 )
    {
        printf("\nchoose color key index 1~9 [1:G 2:B 3:R 4:G 5:B 6:R 7:G 8:B 9:R]\n");
        scanf("%d", &iNum);        
    }
           
    surfaceI8->SetSrcColorKeyIndex(surfaceI8, iNum);
    primary->SetBlittingFlags( primary, (DFBSurfaceBlittingFlags)( DSBLIT_SRC_COLORKEY ));

    primary->Blit(primary, surfaceI8, NULL, 0, 0);
    primary->Flip(primary, NULL, DSFLIP_BLIT);

    while(1);

    /* release our interfaces to shutdown DirectFB */
    palette->Release( palette );
    primary->Release( primary );
    dfb->Release( dfb );

    return 0;
}

