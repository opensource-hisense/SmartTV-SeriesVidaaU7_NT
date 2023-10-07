
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

// layer size
#define LAYER_WIDTH     1360
#define LAYER_HEIGHT     768
// window size
#define WINDOW_WIDTH    1280
#define WINDOW_HEIGHT    720

// for slide show step
#define STRING_MOVE_STEP     10
#define IMAGE_MOVE_STEP        10

//
static IDirectFB              *dfb;
static IDirectFBDisplayLayer  *layer;
static IDirectFBWindow        *window;
static IDirectFBImageProvider *provider;
static IDirectFBFont          *font;
//
static IDirectFBSurface       *window_surface;
static IDirectFBSurface         *tuximage;
static IDirectFBEventBuffer    *keybuffer;
     
static DFBSurfaceDescription sdsc;
static DFBWindowDescription  desc;     
     
static DFBResult err;         

//
void init_setting( int argc, char *argv[])
{
    
    DFBDisplayLayerConfig         layer_config;
    DFBGraphicsDeviceDescription  gdesc;

    
    DFBCHECK(DirectFBInit( &argc, &argv ));
    DFBCHECK(DirectFBCreate( &dfb ));

    dfb->GetDeviceDescription( dfb, &gdesc );

    DFBCHECK(dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS, DFB_FALSE, &keybuffer ));
     
    DFBCHECK(dfb->GetDisplayLayer( dfb, 1/*DLID_PRIMARY*/, &layer ));

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );
    
    // set layer cap.
    layer_config.flags = (DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);
          
    layer_config.buffermode = DLBM_BACKVIDEO; //DLBM_FRONTONLY; //DLBM_BACKVIDEO, DLBM_BACKSYSTEM, DLBM_TRIPLE
    layer_config.width = LAYER_WIDTH;
    layer_config.height = LAYER_HEIGHT;

    layer->SetConfiguration( layer, &layer_config );
     
    // set window and surface.
    desc.flags = ( DWDESC_POSX | DWDESC_POSY |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
          
    desc.caps   = ( DWCAPS_DOUBLEBUFFER);                    
    desc.posx   = (LAYER_WIDTH - WINDOW_WIDTH)/2;
    desc.posy   = (LAYER_HEIGHT - WINDOW_HEIGHT)/2;
    desc.width  = WINDOW_WIDTH;
    desc.height = WINDOW_HEIGHT;    
    //printf("window pos(%d, %d), size(%d, %d)\n", desc.posx, desc.posy, desc.width, desc.height);
    DFBCHECK( layer->CreateWindow( layer, &desc, &window ) );
    window->GetSurface( window, &window_surface );        
    window->SetOpacity( window, 0xFF );    
    
    // perpare image.    
    DFBCHECK(dfb->CreateImageProvider( dfb, DATADIR"/line.png", &provider ));
    DFBCHECK (provider->GetSurfaceDescription (provider, &sdsc));
    DFBCHECK (dfb->CreateSurface( dfb, &sdsc, &tuximage ));
    DFBCHECK (provider->RenderTo( provider, tuximage, NULL ));
    provider->Release( provider );
     
}

int main( int argc, char *argv[] )
{    
    DFBRectangle srect;
    DFBRectangle drect;
     
    //int fontheight;
    //int err;
    int quit = 0;     
    int image_move = 0;
    int string_move = 0;

    // init dfb and resource.
    init_setting( argc, argv );
    
    // image blit rect size.
    srect.x = 0;
    srect.y = 0;
    srect.w = sdsc.width;
    srect.h = sdsc.height;

    drect.x = 404;
    drect.y = 95;
    drect.w = 92;
    drect.h = 125;

    
    while(1) {
     window_surface->Clear(window_surface, 0, 0, 0, 0);
        window_surface->StretchBlit( window_surface, tuximage, &srect, &drect);
        window_surface->Flip( window_surface, NULL, 0 );
    }
    /*while (!quit) {
        DFBInputEvent evt;          
          
        //window_surface->SetColor( window_surface,  0xff, 0xff, 0xff, 0xff );
        //window_surface->DrawRectangle( window_surface,  0, 0, desc.width, desc.height );
          
        //window_surface->SetColor( window_surface,  0xff, 0xa0, 0x00, 0xff );
        //window_surface->FillRectangle( window_surface,  1, 1,  desc.width-2, desc.height-2 );
              
        window_surface->StretchBlit( window_surface, tuximage, &srect, &drect);
          
        window_surface->Flip( window_surface, NULL, 0 );
          
        //string_move = (string_move + STRING_MOVE_STEP > (WINDOW_WIDTH - STRING_MOVE_STEP))? 0 : (string_move + STRING_MOVE_STEP);
        //image_move = (image_move + IMAGE_MOVE_STEP > (WINDOW_WIDTH - rect.w))? 0 : (image_move + IMAGE_MOVE_STEP);
                   
        while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
            if (evt.type == DIET_KEYPRESS) {
                quit = 1;
                break;
            }
        }
    }*/
     
    // release 
    tuximage->Release( tuximage);
     
    keybuffer->Release( keybuffer );
    font->Release( font );
    window_surface->Release( window_surface );     
    window->Release( window );
    layer->Release( layer );
     
    dfb->Release( dfb );
     
    return 1;
}
