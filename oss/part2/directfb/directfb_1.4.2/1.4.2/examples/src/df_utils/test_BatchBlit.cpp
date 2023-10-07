#include "dfb_util_testbench.h"
#include "dfb_util.h"

#include <directfb.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>





// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestBatchBlit : public CTestCase
{
public:

    virtual void initResource(){} // Invoke before the "main" function (optional)
    virtual void destroyResource(){} // Invoke after the "main" function (optional)

    // Must to override
    int main( int argc, char *argv[] )
{
    IDirectFB              *dfb;
    IDirectFBDisplayLayer  *layer0;
    
//    IDirectFBWindow        *window[4];
//    DFBWindowDescription   desc_win[4];
//    IDirectFBSurface       *surface_win[4];
    
    
    DFBSurfaceDescription    desc_surface_s[2];
    IDirectFBSurface         *surface_s[2];
    
    DFBSurfaceDescription    desc_surface_d;
    IDirectFBSurface         *surface_d;

    IDirectFBWindow          *window_d;
    DFBWindowDescription     desc_win_d;

    DFBRectangle    source_rects_0[720];  
    DFBPoint        dest_points_0[720];  
    DFBRectangle    source_rects_1[1280];  
    DFBPoint        dest_points_1[1280];  
    
    unsigned char * buf;
    int             pitch;
    
    int i;


//    #if 0
//    int resolution_new_width  = RESOLUTION_WIDTH_SMALL;    //set here
//    int resolution_new_height = RESOLUTION_HEIGHT_SMALL;   //set here
//    #elif 0
//    int resolution_new_width  = RESOLUTION_WIDTH_MIDDLE;   //set here
//    int resolution_new_height = RESOLUTION_HEIGHT_MIDDLE;  //set here
//    #else
//    int resolution_new_width  = RESOLUTION_WIDTH_BIG;      //set here
//    int resolution_new_height = RESOLUTION_HEIGHT_BIG;     //set here
//    #endif
//    int pos_X=100, pos_Y=100;  //window position, set here
//    int width_win, height_win;  //window size, set here
    
    int err;


    
    
    DirectFBInit( NULL, NULL );
    DirectFBCreate( &dfb );


    /***** generate layer0 *****/
    dfb->GetDisplayLayer( dfb, 0, &layer0 );  //layer0

    /***** set cooperative level *****/
    layer0->SetCooperativeLevel( layer0, DLSCL_ADMINISTRATIVE );
    
    
//    /***** get width x height on layer0 *****/
//    DFBDisplayLayerConfig layer0_cfg;
//    int width_layer, height_layer;
//    layer0->GetConfiguration(layer0, &layer0_cfg);
//    width_layer  = layer0_cfg.width;
//    height_layer = layer0_cfg.height;
//    printf("Layer width(%d) height(%d) before\n",width_layer, height_layer);
//    
//    //window size
//    width_win  = RESOLUTION_WIDTH_MIDDLE - pos_X*2;   //width_layer - pos_X*2;
//    height_win = RESOLUTION_HEIGHT_MIDDLE - pos_Y*2;  //height_layer - pos_Y*2;
//    
//    /***** change resolution *****/
//    layer0_cfg.width  = resolution_new_width;   //new width
//    layer0_cfg.height = resolution_new_height;  //new height
//    layer0->SetConfiguration ( layer0, &layer0_cfg ); 
//    
//    /***** get width x height on layer0 *****/
//    int width, height;
//    layer0->GetConfiguration(layer0, &layer0_cfg);
//    width  = layer0_cfg.width;
//    height = layer0_cfg.height;
//    printf("Layer width(%d) height(%d) after\n",width, height);
    
      ///***** draw color on layer0 & layer1 *****/
		  //layer0->SetBackgroundColor(layer0, 0x00, 0xff, 0x00, 0x80);  //把layer0塗成green
    
      desc_surface_s[0].flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
      desc_surface_s[0].width  = 1280;
      desc_surface_s[0].height = 1;
      desc_surface_s[0].pixelformat = DSPF_ARGB;
      dfb->CreateSurface( dfb, &(desc_surface_s[0]), &(surface_s[0]) ); 
      
      desc_surface_s[1].flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
      desc_surface_s[1].width  = 1;
      desc_surface_s[1].height = 720;
      desc_surface_s[1].pixelformat = DSPF_ARGB;
      dfb->CreateSurface( dfb, &(desc_surface_s[1]), &(surface_s[1]) ); 
      
      #if 0
      desc_surface_d.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
      desc_surface_d.width  = 1280;
      desc_surface_d.height = 720;
      desc_surface_d.pixelformat = DSPF_ARGB;
      dfb->CreateSurface( dfb, &(desc_surface_d), &(surface_d) ); 
      #else
      memset(&desc_win_d, 0, sizeof(DFBWindowDescription));
      desc_win_d.flags   = (DFBWindowDescriptionFlags)( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_PIXELFORMAT);
      desc_win_d.posx    = 0;
      desc_win_d.posy    = 0;
      desc_win_d.width   = 1280;
      desc_win_d.height  = 720;
      desc_win_d.pixelformat  = DSPF_ARGB;
     
      layer0->CreateWindow( layer0, &desc_win_d, &window_d );
      window_d->SetOpacity( window_d, 0xFF );
      window_d->GetSurface( window_d, &surface_d );
                 
      //surface_d->SetColor( surface_d,
      //                     0xFF, 0x00, 0x00, 0xFF );
     
      //surface_d->FillRectangle( surface_d,
      //                          0, 0,
      //                          width_win, height_win );
     
      //surface_d->Flip( surface_d, NULL, DSFLIP_NONE );       
      #endif
      
      surface_s[0]->Clear( surface_s[0], 0, 0, 0, 0);  //r, g, b, a
      surface_s[1]->Clear( surface_s[1], 0, 0, 0, 0);  //r, g, b, a
      surface_d->Clear( surface_d, 0, 0, 0, 0);        //r, g, b, a


      surface_s[0]->Lock( surface_s[0], (DFBSurfaceLockFlags)DSLF_WRITE, (void**)&buf, &pitch ); 
      printf("FIXJASON buf(0x%x)\n", buf);
      
      for (i=0; i<1280; i++)
      {
          *(buf+i*4+0) = /*0*/ 200;        //b
          *(buf+i*4+1) = /*0*/ 200;        //g
          *(buf+i*4+2) = /*0*/ 200;        //r
          
          if (i<28)
              *(buf+i*4+3) = 255;  //a
          else if (i<414)
              *(buf+i*4+3) = 0;    //a
          else
              *(buf+i*4+3) = 255;  //a
      }
      surface_s[0]->Flip( surface_s[0], NULL, DSFLIP_NONE );  ////////////////

      
      surface_s[1]->Lock( surface_s[1], (DFBSurfaceLockFlags)DSLF_WRITE, (void**)&buf, &pitch ); 
      printf("FIXJASON buf(0x%x) pitch(%d)\n", buf, pitch);
      
      for (i=0; i<720; i++)
      {
          *(buf+i*pitch+0) = /*0*/ 200;        //b
          *(buf+i*pitch+1) = /*0*/ 200;        //g
          *(buf+i*pitch+2) = /*0*/ 200;        //r
          
          if (i<200)
              *(buf+i*pitch+3) = 255;  //a
          else if (i<420)
              *(buf+i*pitch+3) = 0;    //a
          else
              *(buf+i*pitch+3) = 255;  //a
      }
      surface_s[1]->Flip( surface_s[1], NULL, DSFLIP_NONE );  ////////////////

      
      
      surface_s[0]->SetBlittingFlags ( surface_s[0], (DFBSurfaceBlittingFlags)DSBLIT_BLEND_ALPHACHANNEL ); 
      surface_s[1]->SetBlittingFlags ( surface_s[1], (DFBSurfaceBlittingFlags)DSBLIT_BLEND_ALPHACHANNEL ); 
      surface_d->SetBlittingFlags ( surface_d, (DFBSurfaceBlittingFlags)DSBLIT_BLEND_ALPHACHANNEL ); 
      
      
      
      
      
            
      
      
      
      
      for (i=0; i<720; i++)
      {
          source_rects_0[i].x = 0;
          source_rects_0[i].y = 0;
          source_rects_0[i].w = 1280;
          source_rects_0[i].h = 1;
      }
      for (i=0; i<720; i++)
      {
          dest_points_0[i].x = 0/*100*/;
          dest_points_0[i].y = i/*+100*/;
      }
      
      for (i=0; i<1280; i++)
      {
          source_rects_1[i].x = 0;
          source_rects_1[i].y = 0;
          source_rects_1[i].w = 1;
          source_rects_1[i].h = 720;
      }
      for (i=0; i<1280; i++)
      {
          dest_points_1[i].x = i/*100*/;
          dest_points_1[i].y = 0/*+100*/;
      }
    
      
      
      float fps = 0;
      PProbe probe;

      probe.setRepeatInterval(50);
      
      while(1)
      {
      
      probe.start();
      surface_d->BatchBlit ( surface_d, (surface_s[0]), source_rects_0, dest_points_0, 720 );  //v
      surface_d->BatchBlit ( surface_d, (surface_s[1]), source_rects_1, dest_points_1, 1280 ); //h
      probe.stop();
      
      ////printf("\n%s\n", getStringFPS() );
      if ((fps = probe.getFPS()) > 0.f)
            printf("FPS = %f\n", fps);
      
      }
   
   
   
   
      surface_d->Flip( surface_d, NULL, DSFLIP_NONE );  ////////////////
      
      //surface_s[0]->Dump(surface_s[0], "/usb/sda1/", "FIXJASON_s0");
      //surface_s[1]->Dump(surface_s[1], "/usb/sda1/", "FIXJASON_s1");
      //surface_d->Dump(surface_d, "/usb/sda1/", "FIXJASON_d");
      
      while(1);
   
//    memset(&desc_win[0], 0, sizeof(DFBWindowDescription));
//    desc_win[0].flags   = (DFBWindowDescriptionFlags)( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_OPTIONS );
//    desc_win[0].posx    = pos_X;
//    desc_win[0].posy    = pos_Y;
//    desc_win[0].width   = width_win;
//    desc_win[0].height  = height_win;
//    desc_win[0].options = DWOP_SCALE;
//   
//    layer0->CreateWindow( layer0, &desc_win[0], &window[0] );
//    window[0]->SetOpacity( window[0], 0xFF );
//    window[0]->GetSurface( window[0], &surface_win[0] );
//               
//    surface_win[0]->SetColor( surface_win[0],
//                              0xFF, 0x00, 0x00, 0xFF );
//   
//    surface_win[0]->FillRectangle( surface_win[0],
//                                   0, 0,
//                                   width_win, height_win );
//   
//    //surface_win[0]->Flip( surface_win[0], NULL, DSFLIP_NONE ); 
//    sleep(5);////////////////////////////////////////////////////////////////
   
   
   
//   //window[0]->Resize( window[0], resolution_new_width-pos_X*2, resolution_new_height-pos_Y*2);
//   window[0]->Resize( window[0], width_win * resolution_new_width /width_layer, height_win * resolution_new_height / height_layer );
//   window[0]->MoveTo(window[0], desc_win[0].posx * resolution_new_width /width_layer, desc_win[0].posy * resolution_new_height / height_layer );
//   printf("(x, y, w, h) = (%d, %d, %d, %d)\n", desc_win[0].posx * resolution_new_width /width_layer, desc_win[0].posy * resolution_new_height / height_layer, width_win * resolution_new_width /width_layer, height_win * resolution_new_height / height_layer );
//   
//   surface_win[0]->Flip( surface_win[0], NULL, DSFLIP_NONE ); 
   
   
   
   
    ////Final window Surface
    //surface_win[3]->Clear( surface_win[3], 0, 0, 0, 0);
    // 
   //
   //
    ///***** copy surface(on layer0) to temp surface *****/
    //surface_win[3]->SetBlittingFlags ( surface_win[3], DSBLIT_BLEND_ALPHACHANNEL);  //有做顏色的重疊的設定
    //surface_win[3]->Blit( surface_win[3], surface_win[0], NULL, 150, 450);
    //
    //
    //surface_win[3]->Flip( surface_win[3], NULL, DSFLIP_NONE ); 
    
    
    
    

   while(1);
   
   
   
   
   
   
   
#if 0   
    /***** create a surface on layer0 *****/
    layer0->GetSurface(layer0, &surface_layer0); 
    /***** set yellow color *****/
    surface_layer0->SetColor( surface_layer0,
                              0xFF, 0xFF, 0x00, 0x80 );
    /***** fill the surface with yellow color *****/
    surface_layer0->FillRectangle( surface_layer0,
                                   0, 0,
                                   width, height );
    surface_layer0->Flip(surface_layer0, NULL, DSFLIP_NONE);

    /***** create a surface on layer1 to simulate OSD *****/
    layer1->GetSurface(layer1, &surface_layer1); 
    /***** set black color *****/
    surface_layer1->SetColor( surface_layer1,
                              0x00, 0x00, 0x00, 0x80 );
    /***** fill the surface with black color *****/
    surface_layer1->FillRectangle( surface_layer1,
                                   0, 0,
                                   width, height );     
    surface_layer1->Flip(surface_layer1, NULL, DSFLIP_NONE);

    

//2.	如何在DRAM上create一塊temp buffer
    /***** create a temp surface *****/
    dsc_surface.flags        = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    dsc_surface.width        = width;
    dsc_surface.height       = height;
    dsc_surface.pixelformat  = DSPF_ARGB;  //format=argb8888
    dfb->CreateSurface( dfb, &dsc_surface, &tmp_surface );
    tmp_surface->Clear( tmp_surface, 0, 0, 0, 0);  //清空tmp_surface


//3.	如何將OSD (e.g. GOP1) bit blit到那塊temp buffer上並disable Alpha
    /***** copy surface(on layer0) to temp surface *****/
    tmp_surface->SetBlittingFlags ( tmp_surface, DSBLIT_BLEND_ALPHACHANNEL);  //有做顏色的重疊的設定
    tmp_surface->Blit( tmp_surface, surface_layer0, NULL, 0, 0);   //把surface_layer0的內容整個搬到tmp_surface
    /***** copy OSD(on layer1) to temp surface *****/
    tmp_surface->SetBlittingFlags ( tmp_surface, DSBLIT_NOFX);  //沒做顏色的重疊的設定，如字幕
    DFBRectangle source_rect;
    source_rect.x = 100;
    source_rect.y = 400;
    source_rect.w = 300;
    source_rect.h = 100;
    tmp_surface->Blit( tmp_surface, surface_layer1, &source_rect, 150, 450);  //把surface_layer1的內容裡的(x=100,y=400,w=300,h=100)這個range，搬到tmp_surface(x=150,y=450)的位置


//4. 最終的結果，這不重要，只是確認tmp_surface的內容正確與否。
    /***** dump temp surface to check the data *****/
    tmp_surface->Dump(tmp_surface, "/usb/sda1/", "tmp_surface_123");
#endif



////5. Release resource........    
//    sleep(10);
//    
//    /***** Release *****/
//    surface_win[0]->Release( surface_win[0] );
//    window[0]->Release( window[0] );
    
    
    
    
    
    
    
    
    layer0->Release( layer0 );
    
    dfb->Release( dfb );
     
    return 42;
}



};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestBatchBlit);