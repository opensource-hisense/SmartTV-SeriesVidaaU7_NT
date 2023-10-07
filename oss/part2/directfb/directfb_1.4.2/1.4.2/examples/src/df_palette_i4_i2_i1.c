////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <directfb.h>

/* the super interface */
static IDirectFB            *dfb;

/* the primary surface (surface of primary layer) */
static IDirectFBSurface     *primary;
static IDirectFBFont           *font;

/* the src palette  surface */
static IDirectFBSurface     *surface_i4;
static IDirectFBSurface     *surface_i2;
static IDirectFBSurface     *surface_i1;

static IDirectFBPalette     *palette_i4;
static IDirectFBPalette     *palette_i2;
static IDirectFBPalette     *palette_i1;

/* Input interfaces: device and its buffer */
static IDirectFBEventBuffer *events;

static int screen_width, screen_height;
static int fontheight;

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
generate_palette(IDirectFBPalette *palette, u32 num_palette)
{
     int       i;
     DFBResult err;
     DFBColor  colors[256];

     for (i=0; i<256; i++) {
          colors[i].a = 0xff;
          colors[i].r = i + 85;
          colors[i].g = i;
          colors[i].b = i + 171;
     }

     DFBCHECK(palette->SetEntries( palette, colors, num_palette, 0 ));
}

static void
rotate_palette(IDirectFBPalette *palette, u32 num_palette)
{
     DFBResult err;
     DFBColor  colors[256];

     DFBCHECK(palette->GetEntries( palette, colors, num_palette, 0 ));

     DFBCHECK(palette->SetEntries( palette, colors + 1, (num_palette-1), 0 ));

     colors[0].r += 17;
     colors[0].g += 31;
     colors[0].b += 29;
     DFBCHECK(palette->SetEntries( palette, colors, 1, (num_palette-1) ));
}

static void
fill_surface( IDirectFBSurface *surface, u32 num_palette )
{
     DFBResult  err;
     int        x;
     int        y;
     void      *ptr;
     int        pitch;
     u8        *dst;

     DFBCHECK(surface->Lock( surface, DSLF_WRITE, &ptr, &pitch ));

     for (y=0; y<screen_height; y++) {
          dst = ptr + y * pitch;

          for (x=0; x<pitch; x++)
               dst[x] = ((x*x + y) / (y+1))%num_palette;
     }

     DFBCHECK(surface->Unlock( surface ));
}

/*
 * set up DirectFB and load resources
 */
static void init_resources( int argc, char *argv[] )
{
     DFBResult              err;
     DFBSurfaceDescription  sdsc;

     DFBCHECK(DirectFBInit( &argc, &argv ));

     DirectFBSetOption ("bg-none", NULL);
     DirectFBSetOption ("no-init-layer", NULL);

     /* create the super interface */
     DFBCHECK(DirectFBCreate( &dfb ));

     /* create an event buffer for all devices */
     DFBCHECK(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL,
                                           DFB_FALSE, &events ));

     /* set our cooperative level to DFSCL_FULLSCREEN for exclusive access to
        the primary layer */
     err = dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
     if (err)
       DirectFBError( "Failed to get exclusive access", err );

     /* get the primary surface, i.e. the surface of the primary layer we have
        exclusive access to */
     sdsc.flags = DSDESC_CAPS;
     sdsc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

     err = dfb->CreateSurface( dfb, &sdsc, &primary );

     primary->Clear( primary, 0, 0, 0, 0 );
     DFBCHECK(primary->GetSize( primary, &screen_width, &screen_height ));

     /* load font */
     {
          DFBFontDescription desc;

          desc.flags = DFDESC_HEIGHT;
          desc.height = 24;

          DFBCHECK(dfb->CreateFont( dfb, FONT, &desc, &font ));
          DFBCHECK(font->GetHeight( font, &fontheight ));
          DFBCHECK(primary->SetFont( primary, font ));
     }

     /* create & prepare the I4 palette surface */
     sdsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
     sdsc.width = screen_width;
     sdsc.height = screen_height;
     sdsc.caps = DSCAPS_VIDEOONLY;
     sdsc.pixelformat = DSPF_LUT4;
     DFBCHECK(dfb->CreateSurface( dfb, &sdsc, &surface_i4 ));

     DFBCHECK(surface_i4->GetPalette( surface_i4, &palette_i4 ));
     generate_palette(palette_i4, 16);
     fill_surface( surface_i4, 16 );

     /* create & prepare the I2 palette surface */
     sdsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
     sdsc.width = screen_width;
     sdsc.height = screen_height;
     sdsc.caps = DSCAPS_VIDEOONLY;
     sdsc.pixelformat = DSPF_LUT2;
     DFBCHECK(dfb->CreateSurface( dfb, &sdsc, &surface_i2 ));

     DFBCHECK(surface_i2->GetPalette( surface_i2, &palette_i2 ));
     generate_palette(palette_i2, 4);
     fill_surface( surface_i2, 4 );

     /* create & prepare the I1 palette surface */
     sdsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
     sdsc.width = screen_width;
     sdsc.height = screen_height;
     sdsc.caps = DSCAPS_VIDEOONLY;
     sdsc.pixelformat = DSPF_LUT1;
     DFBCHECK(dfb->CreateSurface( dfb, &sdsc, &surface_i1 ));

     DFBCHECK(surface_i1->GetPalette( surface_i1, &palette_i1 ));
     generate_palette(palette_i1, 2);
     fill_surface( surface_i1, 2 );
}

int
main( int argc, char *argv[] )
{
     int                    quit = 0;

     init_resources( argc, argv );

     while (!quit) {
          DFBInputEvent evt;
          char buf[64];

          while (events->GetEvent( events, DFB_EVENT(&evt) ) == DFB_OK) {
               switch (evt.type) {
                    case DIET_KEYPRESS:
                         switch (evt.key_symbol) {
                              case DIKS_ESCAPE:
                                   quit = 1;
                                   break;
                              default:
                                   ;
                         }
                    default:
                         ;
               }
          }

          /* blit i4 palette surface to primary surface and rotate i4 palette */
          sleep(1);
          primary->Clear( primary, 0, 0, 0, 0 );
          primary->SetBlittingFlags( primary, DSBLIT_NOFX );
          primary->Blit( primary, surface_i4, NULL, 0, 0 );

          snprintf( buf, sizeof(buf), "Blit I4-DSPF_LUT4 surface to primary" );
          primary->SetColor( primary, 180, 200, 255, 0xFF );
          primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

          primary->Flip( primary, NULL, DSFLIP_WAITFORSYNC );

          rotate_palette(palette_i4, 16);

          /* blit i2 palette surface to primary surface and rotate i2 palette */
          sleep(1);
          primary->Clear( primary, 0, 0, 0, 0 );
          primary->SetBlittingFlags( primary, DSBLIT_NOFX );
          primary->Blit( primary, surface_i2, NULL, 0, 0 );

          snprintf( buf, sizeof(buf), "Blit I2-DSPF_LUT2 surface to primary" );
          primary->SetColor( primary, 180, 200, 255, 0xFF );
          primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

          primary->Flip( primary, NULL, DSFLIP_WAITFORSYNC );

          rotate_palette(palette_i2, 4);


          /* blit i1 palette surface to primary surface and rotate i1 palette */
          sleep(1);
          primary->Clear( primary, 0, 0, 0, 0 );
          primary->SetBlittingFlags( primary, DSBLIT_NOFX );
          primary->Blit( primary, surface_i1, NULL, 0, 0 );

          snprintf( buf, sizeof(buf), "Blit I1-DSPF_LUT1 surface to primary" );
          primary->SetColor( primary, 180, 200, 255, 0xFF );
          primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

          primary->Flip( primary, NULL, DSFLIP_WAITFORSYNC );

          rotate_palette(palette_i1, 2);
     }

     /* release our interfaces to shutdown DirectFB */
     palette_i4->Release( palette_i4 );
     palette_i2->Release( palette_i2 );
     palette_i1->Release( palette_i1 );

     surface_i4->Release( surface_i4 );
     surface_i2->Release( surface_i2 );
     surface_i1->Release( surface_i1 );
     primary->Release( primary );
     events->Release( events );
     dfb->Release( dfb );

     return 0;
}

