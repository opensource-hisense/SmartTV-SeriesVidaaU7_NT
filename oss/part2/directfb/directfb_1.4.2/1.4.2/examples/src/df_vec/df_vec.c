////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (��MStar Confidential Information��) by the recipient.
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

int i =0;
void PlayToCallback(void *ctx)
{

    IDirectFBSurface *flipSurface = (IDirectFBSurface *)ctx;
    i++;

    flipSurface->Flip(flipSurface, NULL, 0);
}

int main( int argc, char *argv[] )
{
     int loop_cnt = 50;

     DFBCHECK(DirectFBInit( &argc, &argv ));

     DFBCHECK(DirectFBCreate( &dfb ));

     DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));


     {
          DFBSurfaceDescription sdsc;
          DFBWindowDescription desc;

          DFBCHECK(dfb->CreateImageProvider( dfb, "/config/df_vec.vec",
                                             &imageprovider ));
          imageprovider->GetSurfaceDescription( imageprovider, &sdsc );

          desc.flags = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT;
          desc.posx = 0;
          desc.posy = 0;
          desc.width = sdsc.width;
          desc.height = sdsc.height;

          DFBCHECK(layer->CreateWindow( layer, &desc, &imagewindow ) );
          DFBCHECK(imagewindow->GetSurface( imagewindow, &imagesurface ) );


          imagewindow->SetOpacity( imagewindow, 0xFF );
          /*

          DFBCHECK(imageprovider->RenderTo( imageprovider, imagesurface,
                                          NULL ) );

          imagesurface->Flip(imagesurface, NULL, 0);
          */

     }


     {
          DFBSurfaceDescription sdsc;
          DFBWindowDescription desc;

          DFBCHECK(dfb->CreateVideoProvider( dfb, "/config/df_vec.vec",
                                             &videoprovider ));
          videoprovider->GetSurfaceDescription( videoprovider, &sdsc );

          desc.flags = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT;
          desc.posx = desc.width+10;
          desc.posy = 0;
          desc.width = 320;
          desc.height = 240;

          DFBCHECK(layer->CreateWindow( layer, &desc, &videowindow ) );
          DFBCHECK(videowindow->GetSurface( videowindow, &videosurface ) );


          videowindow->SetOpacity( videowindow, 0xFF );

          DFBCHECK(videoprovider->PlayTo( videoprovider, videosurface,
                                          NULL, PlayToCallback, (void*)videosurface ));
     }

     while (loop_cnt--) {
        sleep(1);
     }
     printf("\nthe count of i is %d\n",i);

     videoprovider->Stop(videoprovider);
     videoprovider->Release( videoprovider );
     videosurface->Release(videosurface);
     videowindow->Release( videowindow );
     layer->Release( layer );
     dfb->Release( dfb );

     return 42;
}
