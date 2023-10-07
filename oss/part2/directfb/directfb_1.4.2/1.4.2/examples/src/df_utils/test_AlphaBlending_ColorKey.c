#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
        
#include <directfb.h>

#define DFBCHECK(x...)                                         \
{                                                              \
    DFBResult err = x;                                         \
    if (err != DFB_OK)                                         \
    {                                                          \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
    }                                                          \
}
// layer size
#define LAYER_WIDTH     1280
#define LAYER_HEIGHT     720
// window size
#define WINDOW_WIDTH    1280
#define WINDOW_HEIGHT    720

int main( int argc, char *argv[] )
{

    IDirectFB *dfb = NULL;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow *window = NULL;
    IDirectFBSurface *window_surface = NULL;    
    IDirectFBImageProvider *prov = NULL;

    bool quit = false;
    DFBWindowDescription window_dsc;
    DFBSurfaceDescription surface_dsc;
    DFBDisplayLayerConfig layer_config;
    DFBCHECK (DirectFBInit (&argc, &argv));  
    DFBCHECK (DirectFBCreate (&dfb));  
    
    DFBCHECK (dfb->GetDisplayLayer( dfb, 1, &layer ));
    DFBCHECK (layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE));
    layer_config.flags = (DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);          
    layer_config.buffermode = DLBM_TRIPLE;
    layer_config.width = LAYER_WIDTH;
    layer_config.height = LAYER_HEIGHT;
    layer->SetConfiguration( layer, &layer_config );
    layer->ForceFullUpdateWindowStack(layer,true);

    window_dsc.flags = ( DWDESC_POSX | DWDESC_POSY |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
              
    window_dsc.caps   = (DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER);                    
    window_dsc.width  = WINDOW_WIDTH;
    window_dsc.height = WINDOW_HEIGHT;
    window_dsc.posx   = (LAYER_WIDTH - WINDOW_WIDTH)/2;
    window_dsc.posy   = (LAYER_HEIGHT - WINDOW_HEIGHT)/2;
    layer->GetSurface(layer,&window_surface);

    DFBCHECK( layer->CreateWindow( layer, &window_dsc, &window ) );

    window->GetSurface( window, &window_surface );                    
    window->SetOpacity( window, 0xFF );


    window_surface->SetColor( window_surface,  0xff, 0x80, 0x0, 0xff );
    window_surface->FillRectangle( window_surface, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);        
 
    IDirectFBSurface *sur;
    dfb->CreateImageProvider(dfb, "/applications/rc/rc/img/mainmenu/MAINMENU_IMG_AP.png", &prov);
    prov->GetSurfaceDescription (prov, &surface_dsc);
    surface_dsc.flags = (DFBSurfaceDescriptionFlags)(surface_dsc.flags | DSDESC_CAPS | DSDESC_WIDTH |DSDESC_HEIGHT );

    surface_dsc.caps = DSCAPS_VIDEOONLY;
    surface_dsc.width=200;
    surface_dsc.height=200;
    dfb->CreateSurface (dfb, &surface_dsc, &sur);
    prov->RenderTo(prov, sur, NULL);

    unsigned int r, g,b;
    unsigned int cKey = 0xff0000;
    r = (cKey&0x00ff0000)>>16;
    g = (cKey&0x0000ff00)>>8;
    b = (cKey&0x000000ff);
    //set color key red
    sur->SetSrcColorKey(sur, 255, 0, 0);
    DFBRectangle rc;
    rc.x =0;
    rc.y =0;
    rc.w = 200;
    rc.h = 200;

    //http://directfb.org/docs/DirectFB_Reference_1_4/types.html#DFBSurfaceBlendFunction
    //http://en.wikipedia.org/wiki/Alpha_compositing
    //alpha blending formulas
    //(1)  C final= COLORFACTOR source * COLOR source + COLOR FACTOR dst * COLOR dst
    //(2)  ALPHA final = ALPHA FACTOR source + ALPHA FACTOR dst * ALPHA dst 
    window_surface->SetColor(window_surface, 0xff, 0xff, 0xff, 120);
    window_surface->SetBlittingFlags(window_surface, (DFBSurfaceBlittingFlags)( /*weird*/DSBLIT_BLEND_COLORALPHA /*| DSBLIT_BLEND_ALPHACHANNEL*/ | DSBLIT_SRC_COLORKEY));
    window_surface->SetSrcBlendFunction(window_surface, DSBF_SRCALPHA);
    window_surface->SetDstBlendFunction(window_surface, DSBF_INVSRCALPHA);
    window_surface->SetAlphaCmpMode(window_surface, DSBF_ACMP_OP_MAX);
    window_surface->StretchBlit(window_surface, sur, NULL, &rc);
    window_surface->ReleaseSource(window_surface);

    prov->Release(prov);
    sur->Release(sur);
    window_surface->Flip(window_surface, NULL, DSFLIP_BLIT);

    while (!quit)
      ;

    window_surface->Release(window_surface);  
    window->Release(window);
    layer->Release(layer);
    dfb->Release(dfb);    
                    
    return 0;
}
