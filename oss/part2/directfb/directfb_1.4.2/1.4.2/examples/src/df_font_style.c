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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                    \
     {                                                                    \
          err = x;                                                        \
          if (err != DFB_OK) {                                            \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
               DirectFBErrorFatal( #x, err );                             \
          }                                                               \
     }
/******************************************************************************/

static IDirectFB            *dfb     = NULL;
static IDirectFBSurface     *primary = NULL;
static IDirectFBEventBuffer *events  = NULL;

/******************************************************************************/

static void init_application( int *argc, char **argv[] );
static void exit_application( int status );

/******************************************************************************/

static inline long myclock()
{
     struct timeval tv;

     gettimeofday (&tv, NULL);

     return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void drawMultiLine()
{
     char txt[] =
     {
        0xe4, 0xba, 0x94, 0xe6, 0x98, 0x9f,
        0xe7, 0xba, 0xa2, 0xe6, 0x97, 0x97,
        0xe8, 0xbf, 0x8e, 0xe9, 0xa3, 0x8e,
        0xe9, 0xa3, 0x98, 0xe6, 0x89, 0xac, 0x0};

     DFBFontDescription desc;
     IDirectFBFont* dfb_font;
    desc.flags = (DFBFontDescriptionFlags)(DFDESC_HEIGHT|DFDESC_OUTLINE_WIDTH | DFDESC_ATTRIBUTES | DFDESC_OUTLINE_OPACITY | DFDESC_BOLD_STRENGTH);
    desc.height = 24;
    desc.outline_width = (3 << 16);;
    desc.attributes = DFFA_OUTLINED;
    desc.outline_opacity = 255;
    desc.bold_strength = 8;

     printf("%s, %d, create font,font file=%s\n", __FUNCTION__, __LINE__,FONT);
    dfb->CreateFont(dfb, FONT, &desc, &dfb_font);

    if (!dfb_font)
        printf(" no font handle\n");

    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff);
    primary->SetFont ( primary, dfb_font);


    printf("%s, %d, create font\n", __FUNCTION__, __LINE__);
    DFBColorID colorID[3] = {DCID_PRIMARY, DCID_OUTLINE, DCID_BACKGROUND};
    DFBColor colors[3];

    /* font primary*/
    colors[0].a = 0xff;
    colors[0].r = 0xff;
    colors[0].g = 0;
    colors[0].b = 0;

    /* font outline*/
    colors[1].a = 0xff;
    colors[1].r = 0xff;
    colors[1].g = 0xff;
    colors[1].b = 0xff;

    /*front background*/
    colors[2].a = 0xff;
    colors[2].r = 0x00;
    colors[2].g = 0xff;
    colors[2].b = 0x00;

    /*Italics setting*/
    float lean = 0.5f;
    DFBMatrix matrix;
    matrix.xx = 0x10000L;
    matrix.xy = lean * 0x10000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L;

    /*show Italics and background*/
    primary->SetColors(primary, &colorID, &colors, 3);
    primary->DrawString2( primary, txt,-1, 100, 100, DSTF_CENTER | DSTF_BACKGROUND,matrix );

    /*show outline and background*/
    primary->DrawString( primary, txt,-1, 100, 200, DSTF_LEFT |DSTF_OUTLINE | DSTF_BACKGROUND );

    /*show bold and background*/
    primary->SetColor ( primary, 0xff, 0x00, 0x00, 0xff);
    primary->DrawString( primary, txt,-1, 100, 300, DSTF_LEFT |DSTF_BOLD | DSTF_BACKGROUND );


    /* font primary*/
    colors[0].a = 0xff;
    colors[0].r = 0;
    colors[0].g = 0;
    colors[0].b = 0xff;

    /* font outline*/
    colors[1].a = 0xff;
    colors[1].r = 0;
    colors[1].g = 0;
    colors[1].b = 0;

    /* font background*/
    colors[2].a = 0xff;
    colors[2].r = 0x00;
    colors[2].g = 0xff;
    colors[2].b = 0xff;

    /*show Italics and outline*/
    primary->SetColors(primary, &colorID, &colors, 3);
    primary->DrawString2( primary, txt,-1, 100, 400, DSTF_LEFT |  DSTF_OUTLINE, matrix );

    /*show bold and outline*/
    primary->DrawString( primary, txt,-1, 100, 500, DSTF_LEFT | DSTF_BOLD | DSTF_OUTLINE  );

    /*show Italics and outline and background */
    primary->DrawString2( primary, txt,-1, 100, 600, DSTF_LEFT | DSTF_OUTLINE | DSTF_BACKGROUND, matrix  );
    primary->Flip( primary, NULL, 0);

    while(1){
      sleep(1);
    }
}

int
main( int argc, char *argv[] )
{
     long t1, t2;
     unsigned int frame = 0, fps = 0;

     /* Initialize application. */
     init_application( &argc, &argv );

     drawMultiLine();

     return 0;
}

/******************************************************************************/

static void
init_application( int *argc, char **argv[] )
{
     DFBResult             ret;
     DFBSurfaceDescription desc;

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

     /* Request fullscreen mode. */
     dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

     /* Fill the surface description. */
     desc.flags       = DSDESC_CAPS;
     desc.caps        = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

     /* Create the primary surface. */
     ret = dfb->CreateSurface( dfb, &desc, &primary );
     if (ret) {
          DirectFBError( "IDirectFB::CreateSurface() failed", ret );
          exit_application( 3 );
     }

     /* Create an event buffer with key capable devices attached. */
     ret = dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS, DFB_FALSE, &events );
     if (ret) {
          DirectFBError( "IDirectFB::CreateEventBuffer() failed", ret );
          exit_application( 4 );
     }
}

static void
exit_application( int status )
{
     /* Release the event buffer. */
     if (events)
          events->Release( events );

     /* Release the primary surface. */
     if (primary)
          primary->Release( primary );

     /* Release the super interface. */
     if (dfb)
          dfb->Release( dfb );

     /* Terminate application. */
     exit( status );
}
