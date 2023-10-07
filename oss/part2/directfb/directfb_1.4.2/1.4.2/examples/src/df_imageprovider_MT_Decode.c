
#include <directfb.h>
#include <direct/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>


/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                    \
     {                                                                    \
          err = x;                                                        \
          if (err != DFB_OK) {                                            \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
               DirectFBErrorFatal( #x, err );                             \
          }                                                               \
     }

#define MAX_ITEM 4

/* DirectFB interfaces needed by df_andi */
static IDirectFB               *dfb;
static IDirectFBDisplayLayer   *layer_0;
static IDirectFBWindow         *window;
static IDirectFBSurface        *window_surface;

static char string[8] = {0};
static int id = 0;

static pthread_t tid[8];
static pthread_mutex_t lock1;
static pthread_mutex_t lock2;


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
        
        
        
        dfb->GetDisplayLayer(dfb, 0, &layer_0);
        layer_0->SetCooperativeLevel(layer_0, DLSCL_ADMINISTRATIVE);    
        layer_0->GetConfiguration(layer_0, &layer_cfg);
        layer_0->SetBackgroundColor(layer_0, 0xff, 0xff, 0x00, 0xff);
        
        
        desc_wnd.flags = DWDESC_CAPS | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY;
        desc_wnd.caps = /*DWCAPS_DOUBLEBUFFER |*/ DWCAPS_ALPHACHANNEL;
        desc_wnd.posx = 0;
        desc_wnd.posy = 0;
        desc_wnd.width = layer_cfg.width;
        desc_wnd.height = layer_cfg.height;
        
        layer_0->CreateWindow(layer_0, &desc_wnd, &window);
        window->GetSurface(window, &window_surface);
        window->SetOpacity(window, 0xff);    
        
#if 0     
        /* Fill Background Color */
        window_surface->SetColor(window_surface, 0xff, 0x0, 0x0, 0xff);
        window_surface->FillRectangle(window_surface, 0, 0, layer_cfg.width, layer_cfg.height);
#endif                 
}

/*
 * Decode image
 */
static void decode_image(int index, int X, int Y, int shift_X, int shift_Y, float ratio, bool BeErased)
{
        DFBResult err;
        DFBSurfaceDescription dsc[8];
        DFBSurfaceDescription dsc_3;
        DFBSurfaceDescription dsc_7;
        DFBRectangle rect[8] = {NULL};
        DFBRectangle rect_scale[8] = {NULL};
    IDirectFBImageProvider  *provider[8] = {NULL};
    IDirectFBSurface *image_surface[8] = {NULL};
    DFBRegion region[8] = {NULL};

    if (index == 0){
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang1.gif", &(provider[index]) )); }  //367x300
    else if (index == 1) {
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang2.png", &(provider[index]) )); }  //322x480
    else if (index == 2) {
          //DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang4.jpg", &(provider[index]) )); }  //512x504
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang4_small.jpg", &(provider[index]) )); }  //450x
    else if (index == 3)    {
          //DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang6.bmp", &(provider[index]) )); }  //600x400
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang8_small.jpg", &(provider[index]) )); }  //450x293
    else if (index == 4) {
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang5.gif", &(provider[index]) )); }  //300x219
    else if (index == 5) {
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang3.png", &(provider[index]) )); }  //400x441
    else if (index == 6) {
          //DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang8.jpg", &(provider[index]) )); }  //800x521
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang8_small.jpg", &(provider[index]) )); }  //450x
    else if (index == 7) {
          //DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang7.bmp", &(provider[index]) )); }  //1024x791
          DFBCHECK(dfb->CreateImageProvider( dfb, "../share/directfb-examples/Wang2.png", &(provider[index]) )); }  //322x480

        DFBCHECK (provider[index]->GetSurfaceDescription ( provider[index], &(dsc[index]) ));

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
            
//            provider[index]->Release( provider[index] );
            
            rect_scale[index].x = X + shift_X;
            rect_scale[index].y = Y + shift_Y;
            rect_scale[index].w = (int)((float)rect[index].w * (float)ratio);
            rect_scale[index].h = (int)((float)rect[index].h * (float)ratio);

//FIXJASON, del later
//pthread_mutex_lock(&lock1);

        /***** stretchblit() NG while ration is less than 0.5 on A5 *****/
        window_surface->StretchBlit( window_surface, image_surface[index], &(rect[index]), &(rect_scale[index]) );///////////////////////////////

//FIXJASON, del later
//pthread_mutex_unlock(&lock1);
        
//            if(image_surface[index]) image_surface[index]->Release( image_surface[index] );
            
            //draw
        //window_surface->Flip(window_surface, 0, DSFLIP_NONE);
            region[index].x1 = rect_scale[index].x;
            region[index].y1 = rect_scale[index].y;
            region[index].x2 = rect_scale[index].x + rect_scale[index].w - 1;
            region[index].y2 = rect_scale[index].y + rect_scale[index].h - 1;
            window_surface->Flip(window_surface, &(region[index]), DSFLIP_NONE);
            
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
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(0, 70, 10, 0*230, 0, (float)(230.f/376.f), 1);  //clear
       
        decode_image(0, 70, 10, 0*230, 0, (float)(230.f/376.f), 0);  //draw
    }
}
void thread_decode2()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(1, 170, 10, 1*230, 0, (float)(230.f/322.f), 1);  //clear
        
        decode_image(1, 170, 10, 1*230, 0, (float)(230.f/322.f), 0);  //draw
    }
}
void thread_decode3()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(2, 270, 10, 2*230, 0, (float)(230.f/450.f), 1);  //clear
        
        //decode_image(2, 270, 10, 2*230, 0, (float)(230.f/512.f), 0);  //draw, NG
        decode_image(2, 270, 10, 2*230, 0, (float)(230.f/450.f), 0);  //draw
    }
}
void thread_decode4()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(3, 370, 10, 3*230, 0, (float)(230.f/400.f), 1);  //clear
        
        decode_image(3, 370, 10, 3*230, 0, (float)(230.f/400.f), 0);  //draw
    }
}
void thread_decode5()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(4, 70, 380, 0*230, 0, (float)(230.f/300.f), 1);  //clear
        
        decode_image(4, 70, 380, 0*230, 0, (float)(230.f/300.f), 0);  //draw
    }
}
void thread_decode6()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(5, 170, 380, 1*230, 0, (float)(230.f/400.f), 1);  //clear
        
        decode_image(5, 170, 380, 1*230, 0, (float)(230.f/400.f), 0);  //draw
    }
}
void thread_decode7()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(6, 270, 380, 2*230, 0, (float)(230.f/450.f), 1);  //clear
        
        //decode_image(6, 270, 380, 2*230, 0, (float)(230.f/800.f), 0);  //draw, NG
        decode_image(6, 270, 380, 2*230, 0, (float)(230.f/450.f), 0);  //draw
    }
}
void thread_decode8()
{
    int i;

    for (i=0; i<30; i++)  //do 30 loops
    {
        decode_image(7, 370, 380, 3*230, 0, (float)(230.f/400.f), 1);  //clear
        
        decode_image(7, 370, 380, 3*230, 0, (float)(230.f/400.f), 0);  //draw
    }
}



int main( int argc, char *argv[] )
{
      init_resources( argc, argv );
      
      //syscall("ls -al");
      //pid_t child_pid_0;
    //child_pid_0 = fork();

      int i = 0;
      int err;
      int thread_join_count = 0;
      
      if (pthread_mutex_init(&lock1, NULL) != 0)
      {
              printf("Mutex init failed.\n");
              return;            
      }
      if (pthread_mutex_init(&lock2, NULL) != 0)
      {
              printf("Mutex init failed.\n");
              return;            
      }
          
      for (i=0; i<8; i++)
      {
            if (i == 0)
                err = pthread_create(&(tid[i]), NULL, &thread_decode1, NULL);  //gif
          if (i == 1)
                err = pthread_create(&(tid[i]), NULL, &thread_decode2, NULL);  //png
            if (i == 2)
                err = pthread_create(&(tid[i]), NULL, &thread_decode3, NULL);  //jpg
          if (i == 3)
                err = pthread_create(&(tid[i]), NULL, &thread_decode4, NULL);  //bmp
          if (i == 4)
                err = pthread_create(&(tid[i]), NULL, &thread_decode5, NULL);  //gif
          if (i == 5)
                err = pthread_create(&(tid[i]), NULL, &thread_decode6, NULL);  //png
          if (i == 6)
                err = pthread_create(&(tid[i]), NULL, &thread_decode7, NULL);  //jpg
            if (i == 7)
                err = pthread_create(&(tid[i]), NULL, &thread_decode8, NULL);  //bmp
      }
//    sleep(15);  //delay***************************************************************************************

    for (i=0; i<8; i++)
    {
        err = pthread_join(tid[i], NULL);
        
        if (err == 0)  thread_join_count++;
    }
    
    if (thread_join_count == 8)
      printf("< < < < < Images Decode Finished !!! > > > > >\n\n");

    sleep(2);  //delay

    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);

//      //while(1);
      deinit_resources();
      
      return 0;
}

