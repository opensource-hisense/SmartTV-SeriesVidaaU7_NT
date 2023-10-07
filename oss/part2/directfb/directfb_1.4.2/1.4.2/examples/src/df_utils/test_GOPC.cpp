#include "dfb_util_testbench.h"
#include "dfb_util.h"


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

#if 0
/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }
#endif

void PlayToCallback(void *ctx)
{
    IDirectFBSurface *flipSurface = (IDirectFBSurface *)ctx;

    flipSurface->Flip(flipSurface, NULL, (DFBSurfaceFlipFlags)(0) );
}



// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestGOPC : public CTestCase
{
public:

    virtual void initResource(){} // Invoke before the "main" function (optional)
    virtual void destroyResource(){} // Invoke after the "main" function (optional)





// Must to override
int main( int argc, char *argv[] )
{
     DFBDisplayLayerConfig  layer_cfg;
     int loop_cnt = 10;

     DFBCHECK(DirectFBInit( &argc, &argv ));

     DFBCHECK(DirectFBCreate( &dfb ));

     DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));
     layer->GetConfiguration(layer, &layer_cfg);

printf("layer   : w(%d) h(%d)\n", layer_cfg.width, layer_cfg.height );     
printf("capture : w(%d) h(%d)\n", atoi(argv[1]), atoi(argv[2]) );

     {
          DFBSurfaceDescription sdsc;
          DFBWindowDescription desc;

          DFBCHECK(dfb->CreateImageProvider( dfb, "./df_gopc.gopc",
                                             &imageprovider ));
          imageprovider->GetSurfaceDescription( imageprovider, &sdsc );

printf("panel   : w(%d) h(%d)\n", sdsc.width, sdsc.height );

          desc.flags   = (DFBWindowDescriptionFlags)(DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_OPTIONS);
          desc.posx    = 0;
          desc.posy    = 0;
          desc.width   = sdsc.width;   //panel size
          desc.height  = sdsc.height;  //panel size
          desc.options = DWOP_SCALE;
 
          DFBCHECK(layer->CreateWindow( layer, &desc, &imagewindow ) );
          DFBCHECK(imagewindow->GetSurface( imagewindow, &imagesurface ) );


          imagewindow->SetOpacity( imagewindow, 0xFF );
          DFBGOPCSetting_t gopc_setting = {0,0,atoi(argv[1]),atoi(argv[2]),0x0f}; 
          imageprovider->SetHWDecoderParameter(imageprovider,&gopc_setting);
          
          //imagesurface->Clear(imagesurface, 0, 0, 0, 0);  //(r, g, b, a)
          
          DFBCHECK(imageprovider->RenderTo( imageprovider, imagesurface,
                                          NULL ) );

          imagesurface->Dump(imagesurface, "/usb/sda1/", "dwin_capture");

          /***** Show *****/
          imagewindow->Resize(imagewindow, layer_cfg.width, layer_cfg.height);
          imagesurface->Flip(imagesurface, NULL, (DFBSurfaceFlipFlags)(0) );
     }
#if 0
     {
          DFBSurfaceDescription sdsc;
          DFBWindowDescription desc;

          DFBCHECK(dfb->CreateVideoProvider( dfb, DATADIR"/df_gopc.gopc",
                                             &videoprovider ));
          videoprovider->GetSurfaceDescription( videoprovider, &sdsc );

          desc.flags = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT;
          desc.posx = desc.width+10;
          desc.posy = 0;
          desc.width = sdsc.width;
          desc.height = sdsc.height;

          DFBCHECK(layer->CreateWindow( layer, &desc, &videowindow ) );
          DFBCHECK(videowindow->GetSurface( videowindow, &videosurface ) );


          videowindow->SetOpacity( videowindow, 0xFF );

          DFBCHECK(videoprovider->PlayTo( videoprovider, videosurface,
                                          NULL, PlayToCallback, (void*)videosurface ));
     }
#endif

     while (loop_cnt--) {
        sleep(1);
     }

     //videoprovider->Stop(videoprovider);
     //videoprovider->Release( videoprovider );
     //videosurface->Release(videosurface);
     //videowindow->Release( videowindow );
     imagesurface->Release( imagesurface );
     imagewindow->Release( imagewindow );
     layer->Release( layer );
     dfb->Release( dfb );

     return 42;
}



};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestGOPC);