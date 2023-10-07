////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include <directfb.h>

#include <direct/util.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "util.h"

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                    \
     {                                                                    \
          err = x;                                                        \
          if (err != DFB_OK) {                                            \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
               DirectFBErrorFatal( #x, err );                             \
          }                                                               \
     }

/* DirectFB interfaces needed by df_andi */
static IDirectFB               *dfb;
static IDirectFBSurface        *primary;
static IDirectFBEventBuffer    *keybuffer;
static IDirectFBImageProvider  *provider;
static IDirectFBFont           *font;

/* DirectFB surfaces used by df_andi */
static IDirectFBSurface *srcimage;
static IDirectFBSurface *background;

/* values on which placement of penguins and text depends */
static int xres;
static int yres;
static int fontheight;

static bool triple;

/*
 * set up DirectFB and load resources
 */
static void init_resources( int argc, char *argv[] )
{
     DFBResult err;
     DFBSurfaceDescription dsc;

     srand((long)time(0));

     DFBCHECK(DirectFBInit( &argc, &argv ));

     DirectFBSetOption ("bg-none", NULL);
     DirectFBSetOption ("no-init-layer", NULL);

     /* create the super interface */
     DFBCHECK(DirectFBCreate( &dfb ));

     /* create an input buffer for key events */
     DFBCHECK(dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS,
                                           DFB_FALSE, &keybuffer ));

     /* set our cooperative level to DFSCL_FULLSCREEN for exclusive access to
        the primary layer */
     err = dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
     if (err)
       DirectFBError( "Failed to get exclusive access", err );

     /* get the primary surface, i.e. the surface of the primary layer we have
        exclusive access to */
     dsc.flags = DSDESC_CAPS;
     dsc.caps = DSCAPS_PRIMARY | DSCAPS_TRIPLE;

     err = dfb->CreateSurface( dfb, &dsc, &primary );
     if (err) {
          dsc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

          DFBCHECK(dfb->CreateSurface( dfb, &dsc, &primary ));
     }
     else
          triple = true;

     DFBCHECK(primary->GetSize( primary, &xres, &yres ));

     /* load font */
     {
          DFBFontDescription desc;

          desc.flags = DFDESC_HEIGHT;
          desc.height = 24;

          DFBCHECK(dfb->CreateFont( dfb, FONT, &desc, &font ));
          DFBCHECK(font->GetHeight( font, &fontheight ));
          DFBCHECK(primary->SetFont( primary, font ));
     }

     /* load the background image */
     DFBCHECK(dfb->CreateImageProvider( dfb, DATADIR"/background.jpg",
                                        &provider ));

     DFBCHECK (provider->GetSurfaceDescription (provider, &dsc));
     dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
     dsc.width = xres;
     dsc.height = yres;
     DFBCHECK(dfb->CreateSurface( dfb, &dsc, &background ));

     DFBCHECK(provider->RenderTo( provider, background, NULL ));
     provider->Release( provider );

     /* load penguin animation */
     DFBCHECK(dfb->CreateImageProvider( dfb, DATADIR"/pngtest.png",
                                        &provider ));

     DFBCHECK (provider->GetSurfaceDescription (provider, &dsc));
     DFBCHECK(dfb->CreateSurface( dfb, &dsc, &srcimage ));

     DFBCHECK(provider->RenderTo( provider, srcimage, NULL ));

     DFBCHECK(srcimage->GetSize( srcimage, &xres, &yres ));

     provider->Release( provider );
}

/*
 * deinitializes resources and DirectFB
 */
static void deinit_resources()
{
     srcimage->Release( srcimage );
     background->Release( background );
     primary->Release( primary );
     keybuffer->Release( keybuffer );
     font->Release( font );
     dfb->Release( dfb );
}

static void Fill_Trapezoid_Test()
{
      char buf[32];
      DFBTrapezoid trapeZoidFill;

      /* draw the background image */
      primary->SetBlittingFlags( primary, DSBLIT_NOFX );
      primary->Blit( primary, background, NULL, 0, 0 );

      /* Test Fill Trapezoid!!! */
      primary->SetColor( primary, 0, 0, 60, 0xa0 );
      primary->FillRectangle( primary, 0, 0, 440, fontheight+5 );
      snprintf( buf, sizeof(buf), "Fill Trapezoid Example");
      primary->SetColor( primary, 180, 200, 255, 0xFF );
      primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

      //Fill horiz_direct examples
      primary->SetColor( primary, 0xFF, 0x0, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_TRUE;
      trapeZoidFill.x0 = 100;
      trapeZoidFill.y0 = 50;
      trapeZoidFill.x1 = 20;
      trapeZoidFill.y1 = 350;
      trapeZoidFill.top_edge_width = 100;
      trapeZoidFill.bottom_edge_width = 280;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_TRUE;
      trapeZoidFill.x0 = 320;
      trapeZoidFill.y0 = 50;
      trapeZoidFill.x1 = 400;
      trapeZoidFill.y1 = 350;
      trapeZoidFill.top_edge_width = 280;
      trapeZoidFill.bottom_edge_width = 100;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      trapeZoidFill.horiz_direct = DFB_TRUE;
      trapeZoidFill.x0 = 620;
      trapeZoidFill.y0 = 50;
      trapeZoidFill.x1 = 620;
      trapeZoidFill.y1 = 350;
      trapeZoidFill.top_edge_width = 100;
      trapeZoidFill.bottom_edge_width = 280;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0xFF, 0xFF, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_TRUE;
      trapeZoidFill.x0 = 920;
      trapeZoidFill.y0 = 50;
      trapeZoidFill.x1 = 1100;
      trapeZoidFill.y1 = 350;
      trapeZoidFill.top_edge_width = 280;
      trapeZoidFill.bottom_edge_width = 100;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      ////////////////////////////////////////////////////////////////////////
      //Fill non horiz_direct examples
      primary->SetColor( primary, 0xFF, 0xFF, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_FALSE;
      trapeZoidFill.x0 = 280;
      trapeZoidFill.y0 = 540;
      trapeZoidFill.x1 = 20;
      trapeZoidFill.y1 = 450;
      trapeZoidFill.top_edge_width = 100;
      trapeZoidFill.bottom_edge_width = 280;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      trapeZoidFill.horiz_direct = DFB_FALSE;
      trapeZoidFill.x0 = 580;
      trapeZoidFill.y0 = 450;
      trapeZoidFill.x1 = 320;
      trapeZoidFill.y1 = 540;
      trapeZoidFill.top_edge_width = 280;
      trapeZoidFill.bottom_edge_width = 100;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_FALSE;
      trapeZoidFill.x0 = 880;
      trapeZoidFill.y0 = 450;
      trapeZoidFill.x1 = 620;
      trapeZoidFill.y1 = 450;
      trapeZoidFill.top_edge_width = 100;
      trapeZoidFill.bottom_edge_width = 280;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      primary->SetColor( primary, 0xFF, 0x0, 0x0, 0xFF );
      trapeZoidFill.horiz_direct = DFB_FALSE;
      trapeZoidFill.x0 = 1180;
      trapeZoidFill.y0 = 450;
      trapeZoidFill.x1 = 920;
      trapeZoidFill.y1 = 630;
      trapeZoidFill.top_edge_width = 280;
      trapeZoidFill.bottom_edge_width = 100;
      primary->FillTrapezoid( primary, &trapeZoidFill );

      /* flip display */
      primary->Flip( primary, NULL, triple ? DSFLIP_ONSYNC : DSFLIP_WAITFORSYNC );
}

static void Blit_Trapezoid_Test()
{
      char buf[32];
      DFBTrapezoid trapeZoidBlit;
      DFBRectangle srcrect;

      //Prepare srcrect:
      srcrect.x = srcrect.y = 0;
      srcrect.w = xres;
      srcrect.h = yres;

      /* draw the background image */
      primary->SetBlittingFlags( primary, DSBLIT_NOFX );
      primary->Blit( primary, background, NULL, 0, 0 );

      /* Test Blit Trapezoid!!! */
      primary->SetColor( primary, 0, 0, 60, 0xa0 );
      primary->FillRectangle( primary, 0, 0, 440, fontheight+5 );
      snprintf( buf, sizeof(buf), "Blit Trapezoid Example");
      primary->SetColor( primary, 180, 200, 255, 0xFF );
      primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

      //Blit horiz_direct examples
      //primary->SetColor( primary, 0xFF, 0x0, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 100;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 20;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 320;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 400;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 620;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 620;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0xFF, 0xFF, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 920;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 1100;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      ////////////////////////////////////////////////////////////////////////
      //Fill non horiz_direct examples
      //primary->SetColor( primary, 0xFF, 0xFF, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 280;
      trapeZoidBlit.y0 = 540;
      trapeZoidBlit.x1 = 20;
      trapeZoidBlit.y1 = 450;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 580;
      trapeZoidBlit.y0 = 450;
      trapeZoidBlit.x1 = 320;
      trapeZoidBlit.y1 = 540;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 880;
      trapeZoidBlit.y0 = 450;
      trapeZoidBlit.x1 = 620;
      trapeZoidBlit.y1 = 450;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0xFF, 0x0, 0x0, 0xFF );
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 1180;
      trapeZoidBlit.y0 = 450;
      trapeZoidBlit.x1 = 920;
      trapeZoidBlit.y1 = 630;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      /* flip display */
      primary->Flip( primary, NULL, triple ? DSFLIP_ONSYNC : DSFLIP_WAITFORSYNC );
}

static void Blit_Trapezoid_Mirror_Test()
{
      char buf[48];
      DFBTrapezoid trapeZoidBlit;
      DFBRectangle srcrect;

      //Prepare srcrect:
      srcrect.x = srcrect.y = 0;
      srcrect.w = xres;
      srcrect.h = yres;

      /* draw the background image */
      primary->SetBlittingFlags( primary, DSBLIT_NOFX );
      primary->Blit( primary, background, NULL, 0, 0 );

      /* Test Blit Trapezoid!!! */
      primary->SetColor( primary, 0, 0, 60, 0xa0 );
      primary->FillRectangle( primary, 0, 0, 440, fontheight+5 );
      snprintf( buf, sizeof(buf), "Blit Trapezoid With Mirror Example");
      primary->SetColor( primary, 180, 200, 255, 0xFF );
      primary->DrawString( primary, buf, -1, 10, 0, DSTF_LEFT | DSTF_TOP );

      //Blit horiz_direct examples
      //primary->SetColor( primary, 0xFF, 0x0, 0x0, 0xFF );
      primary->SetBlitMirror(primary, true, false);
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 100;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 20;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      primary->SetBlitMirror(primary, false, true);
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 320;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 400;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      primary->SetBlitMirror(primary, true, true);
      trapeZoidBlit.horiz_direct = DFB_TRUE;
      trapeZoidBlit.x0 = 700;
      trapeZoidBlit.y0 = 50;
      trapeZoidBlit.x1 = 620;
      trapeZoidBlit.y1 = 350;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      ////////////////////////////////////////////////////////////////////////
      //Fill non horiz_direct examples
      //primary->SetColor( primary, 0xFF, 0xFF, 0x0, 0xFF );
      primary->SetBlitMirror(primary, true, false);
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 280;
      trapeZoidBlit.y0 = 540;
      trapeZoidBlit.x1 = 20;
      trapeZoidBlit.y1 = 450;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0x0, 0xFF, 0xFF );
      primary->SetBlitMirror(primary, false, true);
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 580;
      trapeZoidBlit.y0 = 450;
      trapeZoidBlit.x1 = 320;
      trapeZoidBlit.y1 = 540;
      trapeZoidBlit.top_edge_width = 280;
      trapeZoidBlit.bottom_edge_width = 100;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //primary->SetColor( primary, 0x0, 0xFF, 0x0, 0xFF );
      primary->SetBlitMirror(primary, true, true);
      trapeZoidBlit.horiz_direct = DFB_FALSE;
      trapeZoidBlit.x0 = 880;
      trapeZoidBlit.y0 = 540;
      trapeZoidBlit.x1 = 620;
      trapeZoidBlit.y1 = 450;
      trapeZoidBlit.top_edge_width = 100;
      trapeZoidBlit.bottom_edge_width = 280;
      primary->TrapezoidBlit( primary, srcimage, &srcrect, &trapeZoidBlit );

      //Restore Mirror State:
      primary->SetBlitMirror(primary, false, false);

      /* flip display */
      primary->Flip( primary, NULL, triple ? DSFLIP_ONSYNC : DSFLIP_WAITFORSYNC );
}

int main( int argc, char *argv[] )
{
     int     quit = 0;

     init_resources( argc, argv );

     primary->SetDrawingFlags( primary, DSDRAW_BLEND );

     /* main loop */
     while (!quit) {
          DFBInputEvent evt;

          Fill_Trapezoid_Test();

          sleep(5);

          Blit_Trapezoid_Test();

          sleep(5);

          Blit_Trapezoid_Mirror_Test();

          sleep(5);

          /* process keybuffer */
          while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
               if (evt.type == DIET_KEYPRESS) {
                    switch (DFB_LOWER_CASE(evt.key_symbol)) {
                         case DIKS_ESCAPE:
                         case DIKS_SMALL_Q:
                         case DIKS_BACK:
                         case DIKS_STOP:
                              /* quit main loop */
                              quit = 1;
                              break;
                         default:
                              break;
                    }
               }
          }
     }

     deinit_resources();
     return 42;
}
