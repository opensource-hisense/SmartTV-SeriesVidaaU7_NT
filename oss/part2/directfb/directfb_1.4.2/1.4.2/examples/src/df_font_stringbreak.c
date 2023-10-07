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
    { 0xbe, 0xc2, 0xbf, 0xc3, 0x80, 0xc3, 0x81, 0xc3, 0x82, 0xc3,
      0x83, 0xc4, 0x80, 0xc4, 0x82, 0xc3, 0x84, 0xc3, 0x85, 0xc4,
      0x84, 0xe2, 0x80, 0x94, 0xc2, 0xb9, 0xc2, 0xae, 0xc2, 0xa9,
      0xe2, 0x84, 0xa2, 0xe2, 0x99, 0xaa, 0xc2, 0xac, 0xc2, 0xa6,
      0xe2, 0x85, 0x9b, 0xe2, 0x85, 0x9d, 0xe2, 0x85, 0x9e, 0xe2,
      0x84, 0xa6, 0xc3, 0x86, 0xc3, 0x90, 0xc2, 0xaa, 0xc4, 0xa6,
      0xc4, 0xb2, 0xc4, 0xbf, 0xc5, 0x81, 0xc3, 0x98, 0xc5, 0x92,
      0xc2, 0xba, 0xc3, 0x9e, 0xc5, 0xa6, 0xc5, 0x8a, 0xc5, 0x89,
      0xc4, 0xb8, 0xc3, 0xa6, 0xc4, 0x91, 0xc3, 0xb0, 0xc4, 0xa7,
      0xc4, 0xb1, 0xc4, 0xb3, 0xc5, 0x80, 0xc5, 0x82, 0xc3, 0xb8,
      0xc5, 0x93, 0xc3, 0x9f, 0xc3, 0xbe, 0xC2, 0xA5, 0xC3, 0x96,
      0xC3, 0x97, 0xC3, 0x91, 0xC5, 0x92, 0x0};

    int width = 444;
    int i = 0;
    int ret_str_length = 0;
    int ret_width = 0;
    const char * ret_next_line = NULL;

    DFBFontDescription desc;
    IDirectFBFont* dfb_font;
    DFBSurfaceDescription dsc;
    static IDirectFBImageProvider  *provider;
    static IDirectFBSurface *TmpGolden;
    DFBResult err;

    desc.flags = (DFBFontDescriptionFlags)(DFDESC_HEIGHT | DFDESC_BOLD_STRENGTH);
    desc.height = 24;
    desc.bold_strength = 1;
printf("%s, %d, create font\n", __FUNCTION__, __LINE__);
    dfb->CreateFont(dfb, "/applications/rc/rc/DejaVuSans.ttf", &desc, &dfb_font);

printf("%s, %d, get string break, width=%d,\n", __FUNCTION__, __LINE__, width);

    dfb_font->GetStringBreak(dfb_font,
                             txt,
                             -1,
                             width,
                             &ret_width,
                             &ret_str_length,
                             &ret_next_line);

printf("%s, %d, ret_width=%d, ret_str_length=%d,\n", __FUNCTION__, __LINE__, ret_width, ret_str_length);

    for (i=0; i<(ret_width>>3); i++)
    {
        printf("0x%x, ", txt[i]);
    }
    printf("\n");
    printf("ret_next_line:0x%x\n", ret_next_line[0]);

    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff);
    primary->SetFont ( primary, dfb_font);
    primary->SetColor ( primary, 0xff, 0x00, 0x00, 0xff);
    primary->DrawString( primary, txt, -1, 100, 100, DSTF_LEFT);
    primary->DrawString( primary, ret_next_line, -1, 100, 200, DSTF_LEFT);

    /* Load the golden image */
    DFBCHECK(dfb->CreateImageProvider( dfb, DATADIR"/tmpgolden.png",
                                       &provider ));

    DFBCHECK (provider->GetSurfaceDescription (provider, &dsc));
    dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
    dsc.width = 993;
    dsc.height = 178;
    DFBCHECK(dfb->CreateSurface( dfb, &dsc, &TmpGolden ));

    DFBCHECK(provider->RenderTo( provider, TmpGolden, NULL ));
    provider->Release( provider );

    /* Draw Rectangle */
    primary->SetColor( primary, 0, 0, 60, 0xa0 );
    primary->FillRectangle( primary, 80, 300, 1050, 400 );

    /* Blit TmpGolden to the promary */
    primary->Blit ( primary, TmpGolden, NULL, 100, 400);

    /* Draw Golden Font */
    primary->SetColor( primary, 180, 200, 255, 0xFF );
    primary->DrawString( primary, "GOLDEN",-1, 100, 370, DSTF_LEFT );
    primary->Flip( primary, NULL, 0);

    while(1);
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
