#include "dfb_util_testbench.h"
#include "dfb_util.h"

#include <directfb.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>




#include <direct/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>







/* DirectFB interfaces needed by df_andi */
static IDirectFB               *dfb;
static IDirectFBDisplayLayer   *layer_0;
static IDirectFBWindow         *window;
static IDirectFBSurface        *window_surface;


static pthread_t tid[8];
static pthread_mutex_t lock1;
static pthread_mutex_t lock2;


// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestSetClip : public CTestCase
{
public:

/*
 * set up DirectFB and load resources
 */
static void init_resources( int argc, char *argv[] )
{
		DFBResult err;
		//DFBSurfaceDescription dsc;
		DFBWindowDescription  desc_wnd;
		DFBDisplayLayerConfig layer_cfg;
		
		DFBCHECK(DirectFBInit( &argc, &argv ));
		
		DirectFBSetOption ("bg-none", NULL);
		DirectFBSetOption ("no-init-layer", NULL);
		
		/* create the super interface */
		DFBCHECK(DirectFBCreate( &dfb ));
		
		
		
		dfb->GetDisplayLayer(dfb, 1, &layer_0);  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		layer_0->SetCooperativeLevel(layer_0, DLSCL_ADMINISTRATIVE);	
		layer_0->GetConfiguration(layer_0, &layer_cfg);
		layer_0->SetBackgroundColor(layer_0, 0x00, 0x00, 0x00, 0xff);  //black screen
		
		
		desc_wnd.flags = (DFBWindowDescriptionFlags)(DWDESC_CAPS | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY);
		desc_wnd.caps = /*DWCAPS_DOUBLEBUFFER |*/ DWCAPS_ALPHACHANNEL;
		desc_wnd.posx = 0;
		desc_wnd.posy = 0;
		desc_wnd.width = layer_cfg.width;
		desc_wnd.height = layer_cfg.height;
		printf("layer (%d x %d)\n", layer_cfg.width, layer_cfg.height);
		
		layer_0->CreateWindow(layer_0, &desc_wnd, &window);
		window->GetSurface(window, &window_surface);
		//window->SetOpacity(window, 0xff);	
		window->SetOpacity(window, 0x80);	//////////////////////////////////////////////////////////////////
		
#if 0     
		/* Fill Background Color */
		window_surface->SetColor(window_surface, 0xff, 0x0, 0x0, 0xff);
		window_surface->FillRectangle(window_surface, 0, 0, layer_cfg.width, layer_cfg.height);
#endif		 		
}

/*
 * Decode image
 */
static void decode_image(int index, int X, int Y, int shift_X, int shift_Y, float ratio, bool BeErased, bool smooth)
{
		DFBResult err;
		DFBSurfaceDescription dsc[8];
		DFBRectangle rect[8] = {NULL};
		DFBRectangle rect_scale[8] = {NULL};
    IDirectFBImageProvider  *provider[8] = {NULL};
    IDirectFBSurface *image_surface[8] = {NULL};
    DFBRegion region[8] = {NULL};
    
    if (index == 0){
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/111x111.jpg", &(provider[index]) )); }  //111x111
    else if (index == 1) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/50x50.png", &(provider[index]) )); }  //322x480
    else if (index == 2) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/250x250.jpg", &(provider[index]) )); }  //450x
    else if (index == 3)	{
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/250x250.jpg", &(provider[index]) )); }  //400x
    else if (index == 4) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/Wang5.gif", &(provider[index]) )); }  //300x219
    else if (index == 5) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/Wang3.png", &(provider[index]) )); }  //400x441
    else if (index == 6) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/Wang8_small.jpg", &(provider[index]) )); }  //450x
    else if (index == 7) {
    	  DFBCHECK(dfb->CreateImageProvider( dfb, "pic/Wang7_small.bmp", &(provider[index]) )); }  //400x
		
		DFBCHECK (provider[index]->GetSurfaceDescription ( provider[index], &(dsc[index]) ));
#if 1
dsc[index].flags = (DFBSurfaceDescriptionFlags)(dsc[index].flags | DSDESC_CAPS );
dsc[index].caps = DSCAPS_VIDEOONLY;
#endif

		if (BeErased)  //clear
		{
		    rect[index].x = X + shift_X;
		    rect[index].y = Y + shift_Y;
		    rect[index].w = (int)((float)dsc[index].width * (float)ratio);
		    rect[index].h = (int)((float)dsc[index].height * (float)ratio);
		    
        window_surface->SetColor(window_surface, 0xff, /*0xff*/ 0x00, 0x0, 0xff);
        window_surface->FillRectangle(window_surface, rect[index].x, rect[index].y, rect[index].w, rect[index].h);
	      
	      region[index].x1 = rect[index].x;
		    region[index].y1 = rect[index].y;
		    region[index].x2 = rect[index].x + rect[index].w - 1;
		    region[index].y2 = rect[index].y + rect[index].h - 1;
		    window_surface->Flip(window_surface, &(region[index]), DSFLIP_NONE);
		}
		else  //draw
		{
		    dfb->CreateSurface( dfb, &(dsc[index]), &(image_surface[index]) );  //to store image-buf

		    rect[index].x = 0;
		    rect[index].y = 0;
		    rect[index].w = dsc[index].width;
		    rect[index].h = dsc[index].height;     

		    DFBCHECK(provider[index]->RenderTo( provider[index], image_surface[index], &(rect[index]) ));    

#if 1
window_surface->SetSrcBlendFunction(window_surface,DSBF_SRCALPHA);
window_surface->SetDstBlendFunction(window_surface,DSBF_INVSRCALPHA);
window_surface->SetBlittingFlags(window_surface, DFBSurfaceBlittingFlags( DSBLIT_BLEND_ALPHACHANNEL ));
window_surface->SetAlphaCmpMode(window_surface, DSBF_ACMP_OP_MAX);
#endif		    
		    
		    rect_scale[index].x = X + shift_X;
		    rect_scale[index].y = Y + shift_Y;
		    rect_scale[index].w = (int)((float)rect[index].w * (float)ratio);
		    rect_scale[index].h = (int)((float)rect[index].h * (float)ratio);
  
        
        #if 0
        window_surface->StretchBlit( window_surface, image_surface[index], &(rect[index]), &(rect_scale[index]) );///////////////////////////////
        #elif 0  //this one
        window_surface->Blit( window_surface, image_surface[index], &(rect[index]), 0, 0 );///////////////////////////////
        #else
DFBRegion rg;
rg.x1 = 0;
rg.y1 = 0;
rg.x2 = 55;
rg.y2 = 55;
window_surface->SetClip(window_surface, &rg);
DFBRectangle rc;
rc.x = 0;
rc.y = 0;
rc.w = 111;
rc.h = 111;
window_surface->StretchBlit(window_surface, image_surface[index], NULL, &rc);

rg.x1 = 56;
rg.y1 = 0;
rg.x2 = 111;
rg.y2 = 55;
window_surface->SetClip(window_surface, &rg);
rc.x = 0;
rc.y = 0;
rc.w = 111;
rc.h = 111;
window_surface->StretchBlit(window_surface, image_surface[index], NULL, &rc);

rg.x1 = 0;
rg.y1 = 56;
rg.x2 = 111;
rg.y2 = 111;
window_surface->SetClip(window_surface, &rg);
rc.x = 0;
rc.y = 0;
rc.w = 111;
rc.h = 111;
window_surface->StretchBlit(window_surface, image_surface[index], NULL, &rc);

window_surface->Flip(window_surface, NULL, DSFLIP_BLIT);
        #endif

		    
		    //draw
        //window_surface->Flip(window_surface, 0, DSFLIP_NONE);
		    #if 0
		    region[index].x1 = rect_scale[index].x;
		    region[index].y1 = rect_scale[index].y;
		    region[index].x2 = rect_scale[index].x + rect_scale[index].w - 1;
		    region[index].y2 = rect_scale[index].y + rect_scale[index].h - 1;
		    window_surface->Flip(window_surface, &(region[index]), DSFLIP_NONE);
		    
		    #elif 0  //this one  /////////////////////////////////////
		    region[index].x1 = rect_scale[index].x;
		    region[index].y1 = rect_scale[index].y;
		    region[index].x2 = rect_scale[index].x + rect_scale[index].w - 1;
		    region[index].y2 = rect_scale[index].y + rect_scale[index].h - 1;
		    window_surface->Flip(window_surface, &(region[index]), DSFLIP_NONE);
		    #endif

		    
		    if (provider[index])  provider[index]->Release( provider[index] );
		    if(image_surface[index])  image_surface[index]->Release( image_surface[index] );
		}
}

/*
 * deinitializes resources and DirectFB
 */
static void deinit_resources()
{
		if(window_surface) window_surface->Release( window_surface );
		if(dfb) dfb->Release( dfb );
}

void thread_decode1()
{
    decode_image(0, 0, 0, 0, 0, (float)(1.0), 0, 0);  //draw  1920x1080
}

int main( int argc, char *argv[] )
{
	  init_resources( argc, argv );

	  int i = 0;
	  int err;
	  int thread_join_count = 0;

    thread_decode1();
   

	  while(1);
	  
	  deinit_resources();
	  
	  return 0;
}








};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestSetClip);