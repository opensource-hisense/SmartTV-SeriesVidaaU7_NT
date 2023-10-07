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
#include <unistd.h>
#include <math.h>
#include <directfb.h>

static IDirectFB              *dfb;
static IDirectFBDisplayLayer  *layer;

static IDirectFBImageProvider *imageprovider;
static IDirectFBWindow        *imagewindow;
static IDirectFBSurface       *imagesurface;

static IDirectFBVideoProvider *videoprovider;
static IDirectFBWindow        *videowindow;
static IDirectFBSurface       *videosurface;

int err;

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

void PlayToCallback(void *ctx)
{
    IDirectFBSurface *flipSurface = (IDirectFBSurface *)ctx;
    flipSurface->Flip(flipSurface, NULL, 0);
}

int main( int argc, char *argv[] )
{
     DFBCHECK(DirectFBInit( &argc, &argv ));

     DFBCHECK(DirectFBCreate( &dfb ));

     DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));
     DFBSurfaceDescription sdsc;
     DFBWindowDescription desc;

     DFBCHECK(dfb->CreateVideoProvider( dfb, "gif_motion_test.gif", &videoprovider ));
     DFBCHECK(videoprovider->GetSurfaceDescription( videoprovider, &sdsc ));

     desc.flags = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT;
     desc.posx = 0;
     desc.posy = 0;
     desc.width = 1280;
     desc.height = 720;

     DFBCHECK(layer->CreateWindow( layer, &desc, &videowindow ) );
     DFBCHECK(videowindow->GetSurface( videowindow, &videosurface ) );


     DFBRectangle rect;
     rect.x = 0;
     rect.y = 0;
     rect.w = 1280;
     rect.h = 720;
     DFBCHECK(videowindow->SetOpacity( videowindow, 0xFF ));
     DFBCHECK(videoprovider->SetPlaybackFlags(videoprovider,DVPLAY_LOOPING));
     DFBCHECK(videoprovider->PlayTo( videoprovider, videosurface,
                                          &rect, PlayToCallback, (void*)videosurface ));

     while (1) ;


     videoprovider->Stop(videoprovider);
     videoprovider->Release( videoprovider );
     videosurface->Release(videosurface);
     videowindow->Release( videowindow );
     layer->Release( layer );
     dfb->Release( dfb );

     return 0;
}
