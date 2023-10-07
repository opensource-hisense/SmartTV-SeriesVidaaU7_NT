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
#include <unistd.h>
#include <math.h>
#include <time.h>

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

int main( int argc, char *argv[] )
{
     IDirectFBFont          *font;
     IDirectFB              *dfb;
     IDirectFBDisplayLayer  *layer;
     IDirectFBSurface       *bgsurface;

     IDirectFBWindow        *window1;
     IDirectFBSurface       *window_surface1;


     DFBDisplayLayerConfig         layer_config;
     DFBGraphicsDeviceDescription  gdesc;
     int fontheight;

     int err;

     DFBCHECK(DirectFBInit( &argc, &argv ));
     DFBCHECK(DirectFBCreate( &dfb ));
     dfb->GetDeviceDescription( dfb, &gdesc );

     DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));

     layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

     if (!((gdesc.blitting_flags & DSBLIT_BLEND_ALPHACHANNEL) &&
           (gdesc.blitting_flags & DSBLIT_BLEND_COLORALPHA  )))
     {
          layer_config.flags = DLCONF_BUFFERMODE;
          layer_config.buffermode = DLBM_BACKSYSTEM;

          layer->SetConfiguration( layer, &layer_config );
     }

     layer->GetConfiguration( layer, &layer_config );
     layer->EnableCursor ( layer, 1 );

     {
          DFBFontDescription desc;

          desc.flags  = DFDESC_HEIGHT | DFDESC_WIDTH;
          desc.height = 15;
          desc.width  = 15;

          DFBCHECK(dfb->CreateFont( dfb, NULL, &desc, &font ));
          font->GetHeight( font, &fontheight );
     }
     {
          DFBWindowDescription wdesc;

          wdesc.flags  = ( DWDESC_POSX | DWDESC_POSY | DWDESC_SURFACE_CAPS |
                          DWDESC_WIDTH | DWDESC_HEIGHT );
          wdesc.posx   = 200;
          wdesc.posy   = 200;
          wdesc.width  = 1600;
          wdesc.height = 800;
          wdesc.surface_caps = DSCAPS_VIDEOONLY;
          DFBCHECK(layer->CreateWindow( layer, &wdesc, &window1 ) );
          window1->GetSurface( window1, &window_surface1 );
          window_surface1->Clear( window_surface1 , 0x20, 0x20, 0x80, 0xFF);
          window_surface1->SetColor( window_surface1, 0x70, 0xD0, 0xA0, 0xFF );
          DFBCHECK(window_surface1->SetFont( window_surface1, font ) );

          IDirectFBSurface       *surface1;

          DFBSurfaceDescription   desc;
          desc.flags  = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS;
          desc.width  = 200;
          desc.height = 200;
          desc.caps   = DSCAPS_VIDEOONLY;
          DFBCHECK(dfb->CreateSurface( dfb, &desc, &surface1 ) );

          surface1->Clear( surface1 , 0x70, 0x56 , 0x17, 0xFF);
          //set ROP different ROP option
          window_surface1->SetBlittingFlags(window_surface1, DSBLIT_NOFX);

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_OP_ZERO );
          window_surface1->Blit( window_surface1, surface1, 0, 0, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_OP_ZERO",
                                 -1, 0, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NOT_PS_OR_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 200, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NOT_PS_OR_PD",
                                 -1, 200, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NS_AND_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 400, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NS_AND_PD",
                                 -1, 400, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NS );
          window_surface1->Blit( window_surface1, surface1, 0, 600, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NS",
                                 -1, 600, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PS_AND_ND );
          window_surface1->Blit( window_surface1, surface1, 0, 800, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PS_AND_ND",
                                 -1, 800, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_ND );
          window_surface1->Blit( window_surface1, surface1, 0, 1000, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_ND",
                                 -1, 1000, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PS_XOR_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 1200, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PS_XOR_PD",
                                 -1, 1200, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NOT_PS_AND_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 1400, 0 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NOT_PS_AND_PD",
                                 -1, 1400, 0, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PS_AND_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 0, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PS_AND_PD",
                                 -1, 0, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NOT_PS_XOR_PD);
          window_surface1->Blit( window_surface1, surface1, 0, 200, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NOT_PS_XOR_PD",
                                 -1, 200, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 400, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PD",
                                 -1, 400, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NS_OR_PD );
          window_surface1->Blit( window_surface1, surface1, 0, 600, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NS_OR_PD",
                                 -1, 600, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PS );
          window_surface1->Blit( window_surface1, surface1, 0, 800, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PS",
                                 -1, 800, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PS_OR_ND );
          window_surface1->Blit( window_surface1, surface1, 0, 1000, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PS_OR_ND",
                                 -1, 1000, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PD_OR_PS );
          window_surface1->Blit( window_surface1, surface1, 0, 1200, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PD_OR_PS",
                                 -1, 1200, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_ONE );
          window_surface1->Blit( window_surface1, surface1, 0, 1400, 200 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_ONE",
                                 -1, 1400, 200, DSTF_LEFT | DSTF_TOP );

          window_surface1->SetROPFlags( window_surface1, DFB_ROP2_NONE );
          window_surface1->Blit( window_surface1, surface1, 0, 0, 400 );
          window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_NONE(disable rop)",
                                 -1, 0, 400, DSTF_LEFT | DSTF_TOP );

          window_surface1->Blit( window_surface1, surface1, 0, 200, 400 );
          window_surface1->DrawString( window_surface1,
                                 "src: R:0x70 G:0x56 B:0x17",
                                 -1, 200, 400, DSTF_LEFT | DSTF_TOP );

          window_surface1->DrawString( window_surface1,
                                 "dst: R:0x20, G:0x20, B:0x80",
                                 -1, 400, 400, DSTF_LEFT | DSTF_TOP );

          //test StretchBlit
          {
               DFBRectangle  ret;
               ret.h = 200;
               ret.w = 400;
               ret.x = 0;
               ret.y = 600;
               window_surface1->StretchBlit ( window_surface1, surface1, 0, &ret);
               window_surface1->DrawString( window_surface1,
                                 "StretchBlit",
                                 -1, 0, 600, DSTF_LEFT | DSTF_TOP );
               window_surface1->DrawString( window_surface1,
                                 "No Rop",
                                 -1, 0, 600 + fontheight, DSTF_LEFT | DSTF_TOP );

               ret.x = 400;
               ret.y = 600;
               window_surface1->SetROPFlags( window_surface1, DFB_ROP2_PD_OR_PS );
               window_surface1->StretchBlit ( window_surface1, surface1, 0, &ret);
               window_surface1->DrawString( window_surface1,
                                 "StretchBlit",
                                 -1, 400, 600, DSTF_LEFT | DSTF_TOP );
               window_surface1->DrawString( window_surface1,
                                 "DFB_ROP2_PD_OR_PS",
                                 -1, 400, 600 + fontheight, DSTF_LEFT | DSTF_TOP );
          }

          window1->SetOpacity( window1, 0xFF );

          window_surface1->Flip( window_surface1, NULL, 0 );
     }

     window1->RequestFocus( window1 );
     window1->RaiseToTop( window1 );
     while(1);

     window_surface1->Release( window_surface1 );
     window1->Release( window1 );
     layer->Release( layer );
     bgsurface->Release( bgsurface );
     dfb->Release( dfb );

     return 42;
}