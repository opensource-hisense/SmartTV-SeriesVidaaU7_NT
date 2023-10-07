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

int main( int argc, char *argv[] )
{
    //initialization
    IDirectFB *dfb = NULL;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow *window = NULL;
    IDirectFBSurface *window_surface = NULL;    
    IDirectFBImageProvider *prov = NULL;    
    DFBWindowDescription window_dsc;
    DFBSurfaceDescription surface_dsc;

    DFBCHECK (DirectFBInit (&argc, &argv));  
    DFBCHECK (DirectFBCreate (&dfb));  
    DFBCHECK (dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));
    
    
    //initialization for window
    window_dsc.flags = ( DWDESC_POSX | DWDESC_POSY |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
    
    window_dsc.caps   = (DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER);                    
    window_dsc.width  = 1280;
    window_dsc.height = 720;
    window_dsc.posx   = 0;
    window_dsc.posy   = 0;
    DFBCHECK( layer->CreateWindow( layer, &window_dsc, &window ) );
    window->GetSurface( window, &window_surface );                    
    
    // show window
    window->SetOpacity( window, 0xFF );


    //read png image into image providor
    IDirectFBSurface *sur;
    dfb->CreateImageProvider(dfb, "/applications/rc/rc/img/mainmenu/MAINMENU_IMG_AP.png", &prov);
    prov->GetSurfaceDescription (prov, &surface_dsc);
    surface_dsc.flags = (DFBSurfaceDescriptionFlags)( surface_dsc.flags | DSDESC_CAPS  );
    surface_dsc.caps = DSCAPS_VIDEOONLY;
    dfb->CreateSurface (dfb, &surface_dsc, &sur);

    
    // decode image
    prov->RenderTo(prov, sur, NULL);


    //set the color key to be red
    sur->SetSrcColorKey(sur, 255, 0, 0); // R, G, B
    
    //enable color key
    window_surface->SetBlittingFlags(window_surface, (DFBSurfaceBlittingFlags)( DSBLIT_SRC_COLORKEY));

    window_surface->Blit(window_surface, sur, NULL, 0, 0);
    window_surface->ReleaseSource(window_surface);
    window_surface->Flip(window_surface, NULL, DSFLIP_BLIT);
    

    // block program
    sleep (1000);
    
    
    //finalize; release resources
    prov->Release(prov);
    sur->Release(sur);    
    window_surface->Release(window_surface);  
    window->Release(window);
    layer->Release(layer);
    dfb->Release(dfb);    
                    
    return 0;
}
