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
    IDirectFB *dfb = NULL;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow *window = NULL;
    IDirectFBSurface *window_surface = NULL;	

    IDirectFBImageProvider  *provider;
    DFBWindowDescription window_dsc;
    DFBCHECK (DirectFBInit (&argc, &argv));  
    DFBResult ret = DirectFBCreate (&dfb);  
    DFBCHECK (dfb->GetDisplayLayer( dfb, 0, &layer ));
    DFBCHECK (layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE));
    window_dsc.flags =(DFBWindowDescriptionFlags)( DWDESC_POSX | DWDESC_POSY |
	      DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS |DWDESC_OPTIONS);

    window_dsc.caps   = (DFBWindowCapabilities)(DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER);                    
    window_dsc.width  = 1280;
    window_dsc.height = 720;
    window_dsc.posx   = 0;
    window_dsc.posy   = 0;
    
    int type = 2;
    printf(" 1: create a window with flag DWOP_SCALE \n 2: create a window with flag DWOP_SCALE and DWOP_SCALE_WINDOW_AUTOFIT_LAYER \n");
    scanf("%d", &type);
    //test if code modification changes the orignal DWOP_SCALE's behavior
    if(type == 1)
        window_dsc.options = (DFBWindowOptions)(DWOP_SCALE);
    else if(type == 2)
        window_dsc.options = (DFBWindowOptions)(DWOP_SCALE_WINDOW_AUTOFIT_LAYER | DWOP_SCALE);
         
    DFBCHECK( layer->CreateWindow( layer, &window_dsc, &window ) );
    window->SetStackingClass(window,DWSC_LOWER);
    window->GetSurface( window, &window_surface );		  		  
    window->SetOpacity( window, 0xFF );
    dfb->CreateImageProvider( dfb, "./pp.jpg", &provider );
    provider->RenderTo (provider, window_surface, 0);

    //create another transparent fullscreen window
    IDirectFBWindow *anotherwin = NULL;
    DFBDisplayLayerConfig	layerconfig;
    layer->GetConfiguration(layer,&layerconfig);
    window_dsc.width  = layerconfig.width;
    window_dsc.height = layerconfig.height;
    window_dsc.posx   = 0;
    window_dsc.posy   = 0;
    window_dsc.caps = (DFBWindowCapabilities)DWOP_ALPHACHANNEL;
    window_dsc.flags = (DFBWindowDescriptionFlags)( DWDESC_POSX | DWDESC_POSY |
  	                                            DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);

    layer->CreateWindow( layer, &window_dsc, &anotherwin );
    IDirectFBSurface* anotherwin_surface = 0;
    anotherwin->GetSurface(anotherwin,&anotherwin_surface);
    anotherwin_surface->Clear(anotherwin_surface,0xff, 0x0, 0x0, 0x30);
    anotherwin->SetOpacity(anotherwin,0x30);

    while(1) {
      window_surface->Flip (window_surface, 0, (DFBSurfaceFlipFlags)0);
      anotherwin_surface->Flip(anotherwin_surface,0,(DFBSurfaceFlipFlags)0);
    }
    anotherwin_surface->Release(anotherwin_surface);
    anotherwin->Release(anotherwin);
    window_surface->Release(window_surface);  
    window->Release(window);
    layer->Release(layer);
    dfb->Release(dfb);    
    return 0;
}
