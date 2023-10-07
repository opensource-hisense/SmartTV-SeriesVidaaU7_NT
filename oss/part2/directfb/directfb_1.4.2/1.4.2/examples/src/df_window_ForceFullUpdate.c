#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include <directfb.h>

#include <sys/time.h>

#define TRUE  1
#define FALSE 0
#define LOOPS 1000


/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

#define ABS(a) (a>=0?a:-a)


typedef enum
{
    Window_Moving = 1,
    ContentOfWindow_Moving = 2,
    Color_Change = 3
} eForceFullUpdateWindowStack_Mode;


int main( int argc, char *argv[] )
{
    IDirectFB              *dfb;
    IDirectFBDisplayLayer  *layer;
     
    IDirectFBWindow        *window[4];
    IDirectFBSurface       *window_surface[4];

    int err;
     
    int i, j;
    int pos_start_x[4], pos_start_y[4];
    int pos_cur_x[4], pos_cur_y[4];
    int shift_x[4], shift_y[4];
    bool going_Right[4];
    int count[4];

     
    printf("Plz Choose df_ForceFullUpdateWindowStack Mode as following:\n");
    printf("----------------------------\n");
    printf(" Window_Moving          = 1\n");
    printf(" ContentOfWindow_Moving = 2\n");
    printf(" Color_Change           = 3\n");
    printf("----------------------------\n"); 
     
    DFBCHECK(DirectFBInit( &argc, &argv ));
    DFBCHECK(DirectFBCreate( &dfb ));


    DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

    //New
    #if 1
    layer->ForceFullUpdateWindowStack(layer, true);

    //layer->SetDeskDisplayMode( layer, DLDM_STEREO_LEFTRIGHT_3DUI /*DLDM_STEREO_TOPBOTTOM_3DUI*/ /*DLDM_STEREO_FRAME_PACKING*/);
    #endif

     
    DFBWindowDescription  desc[4];
    /***** red window *****/
    memset(&desc[0], 0, sizeof(DFBWindowDescription));
    desc[0].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[0].posx   = 50;
    desc[0].posy   = 50;
    desc[0].width  = 150;
    desc[0].height = 300;
     
    DFBCHECK( layer->CreateWindow( layer, &desc[0], &window[0] ) );
    window[0]->GetSurface( window[0], &window_surface[0] );
     
    /***** green window *****/
    memset(&desc[1], 0, sizeof(DFBWindowDescription));
    desc[1].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[1].posx   = 950;
    desc[1].posy   = 250;
    desc[1].width  = 200;
    desc[1].height = 200;
     
    DFBCHECK( layer->CreateWindow( layer, &desc[1], &window[1] ) );
    window[1]->GetSurface( window[1], &window_surface[1] );
     
    /***** blue window *****/
    memset(&desc[2], 0, sizeof(DFBWindowDescription));
    desc[2].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT );
    desc[2].posx   = 1000;
    desc[2].posy   = 300;
    desc[2].width  = 200;
    desc[2].height = 200;
     
    DFBCHECK( layer->CreateWindow( layer, &desc[2], &window[2] ) );
    window[2]->GetSurface( window[2], &window_surface[2] );

    /***** yellow-pink region in window *****/
    memset(&desc[3], 0, sizeof(DFBWindowDescription));
    desc[3].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |
                       DWDESC_CAPS);
    desc[3].caps = DWCAPS_ALPHACHANNEL;
    desc[3].posx   = 50;
    desc[3].posy   = 400;
    desc[3].width  = 600;
    desc[3].height = 300;
     
    DFBCHECK( layer->CreateWindow( layer, &desc[3], &window[3] ) );
    window[3]->GetSurface( window[3], &window_surface[3] );
     

    pos_start_x[0] = 50;
    pos_start_y[0] = 50;
    pos_cur_x[0] = pos_start_x[0];
    pos_cur_y[0] = pos_start_y[0];
    shift_x[0] = 1;
    shift_y[0] = 0;
    going_Right[0] = TRUE;
    count[0] = 0;
     
    pos_start_x[1] = 950;
    pos_start_y[1] = 250;
    pos_cur_x[1] = pos_start_x[1];
    pos_cur_y[1] = pos_start_y[1];
    shift_x[1] = 20;
    shift_y[1] = 20;
    going_Right[1] = FALSE;
    count[1] = 0;
     
    pos_start_x[2] = 1000;
    pos_start_y[2] = 300;
    pos_cur_x[2] = pos_start_x[2];
    pos_cur_y[2] = pos_start_y[2];
    shift_x[2] = 20;
    shift_y[2] = 20;
    going_Right[2] = TRUE;
    count[2] = 0;
     
    pos_start_x[3] = 50;
    pos_start_y[3] = 400;
    pos_cur_x[3] = pos_start_x[3];
    pos_cur_y[3] = pos_start_y[3];
    shift_x[3] = 1;
    shift_y[3] = 0;
    going_Right[3] = TRUE;
    count[3] = 0;
     
     
    if ( atoi(argv[1]) == Window_Moving )    
        window[0]->SetOpacity( window[0], 0xFF );
    if ( atoi(argv[1]) == Color_Change )
        window[1]->SetOpacity( window[1], 0x80 );
    if ( atoi(argv[1]) == Color_Change )
        window[2]->SetOpacity( window[2], 0x80 );
    if (atoi(argv[1]) == ContentOfWindow_Moving )
        window[3]->SetOpacity( window[3], 0xFF );
     
     
    if ( atoi(argv[1]) == Window_Moving )
        window_surface[0]->SetColor( window_surface[0],
                                     0xFF, 0x00, 0x00, 0xFF );
    if ( atoi(argv[1]) == Color_Change )
        window_surface[1]->SetColor( window_surface[1],
                                     0x00, 0xFF, 0x00, 0xFF );
    if ( atoi(argv[1]) == Color_Change )
        window_surface[2]->SetColor( window_surface[2],
                                     0x00, 0x00, 0xFF, 0xFF );
    if (atoi(argv[1]) == ContentOfWindow_Moving )
        window_surface[3]->SetColor( window_surface[3],
                                     0xFF, 0xFF, 0x00, 0xFF );
                                                                                               
    
    if ( atoi(argv[1]) == Window_Moving )
        window_surface[0]->FillRectangle( window_surface[0],
                                          0, 0,
                                          150, 300 );     
    if ( atoi(argv[1]) == Color_Change )
        window_surface[1]->FillRectangle( window_surface[1],
                                          0, 0,
                                          200, 200 );      
    if ( atoi(argv[1]) == Color_Change )
        window_surface[2]->FillRectangle( window_surface[2],
                                          0, 0,
                                          200, 200 );     
    if (atoi(argv[1]) == ContentOfWindow_Moving )
        window_surface[3]->FillRectangle( window_surface[3],
                                          0, 0,
                                          600, 300 );     
     
    
    /**********************************************************/     
    /***** shift / scale / / color-change / show in LOOPS *****/
    /**********************************************************/
    for (i=0; i<LOOPS; i++)
    {
        /*************************/
        /***** Window_Moving *****/
        /*************************/
        if (count[0]/250 == 1)  //switch L or R direction, every 250 loops.
        {
            //inverse
            if (going_Right[0]  == TRUE)  going_Right[0] = FALSE;
            else                          going_Right[0] = TRUE;
            count[0] = 0;  //reset
        }
        count[0]++;
        //printf("count[0](%d) going_Right[0](%d)\n", count[0], going_Right[0]);
        
        //calculate X-coordinate
        if (count[0]!=1)
        {  
            if ( (int)(count[0]/50) <= 0)
                pos_cur_x[0] += 1*shift_x[0]*(going_Right[0]==TRUE? 1:-1);
            else if ( (int)(count[0]/50) <= 1)
                pos_cur_x[0] += 2*shift_x[0]*(going_Right[0]==TRUE? 1:-1);  //speed-up
            else if ( (int)(count[0]/50) <= 2)
                pos_cur_x[0] += 3*shift_x[0]*(going_Right[0]==TRUE? 1:-1);  //turbo speed-up
            else if ( (int)(count[0]/50) <= 3)
                pos_cur_x[0] += 2*shift_x[0]*(going_Right[0]==TRUE? 1:-1);  //speed-up
            else if ( (int)(count[0]/50) <= 4)
                pos_cur_x[0] += 1*shift_x[0]*(going_Right[0]==TRUE? 1:-1);
        }
    
    
        /************************/
        /***** Color_Change *****/
        /************************/
        if (count[1]/150 == 1)  //switch size/location, every 150 loops.
        {
            count[1] = 0;  //reset
          
            pos_cur_x[1] = pos_start_x[1];  //reset
            pos_cur_y[1] = pos_start_y[1];  //reset
        }
        count[1]++;
             
        if ( ((int)(count[1]/50) <= 0) && (count[1]%50 == 1) )  //every 50 rounds¡Azoom-in-out/shift
        {
            pos_cur_x[1] += 0;
            pos_cur_y[1] += 0;
           
        }
        else if ( ((int)(count[1]/50) == 1) && (count[1]%50 == 1) )  //every 50 rounds¡Azoom-in-out/shift
        {
            pos_cur_x[1] += shift_x[1]*(going_Right[1]==TRUE? 1:-1);
            pos_cur_y[1] += shift_y[1]*(going_Right[1]==TRUE? 1:-1);
        }
        else if ( ((int)(count[1]/50) == 2) && (count[1]%50 == 1) )  //every 50 rounds¡Azoom-in-out/shift
        {
            pos_cur_x[1] += shift_x[1]*(going_Right[1]==TRUE? 1:-1);
            pos_cur_y[1] += shift_y[1]*(going_Right[1]==TRUE? 1:-1);
        }
        //printf("count[1](%d) pos_cur_x[1](%d) pos_cur_y[1](%d) going_Right[1](%d)\n", count[1], pos_cur_x[1], pos_cur_y[1], going_Right[1]);
        
             
        if (count[2]/450 == 1)  //switch size, every 450 loops.
        {
            count[2] = 0;  //reset
        }
        count[2]++;

        
        /**********************************/
        /***** ContentOfWindow_Moving *****/
        /**********************************/
        if (count[3]/250 == 1)  //switch location, every 250 loops.
        {
            //inverse
            if (going_Right[3]  == TRUE)  going_Right[3] = FALSE;
            else                          going_Right[3] = TRUE;
            count[3] = 0;  //reset
        }
        count[3]++;
        //printf("count[3](%d) going_Right[3](%d)\n", count[3], going_Right[3]);
        
        if (count[3]!=1)
        {  
            if ( (int)(count[3]/50) <= 0)
                pos_cur_x[3] += 1*shift_x[3]*(going_Right[3]==TRUE? 1:-1);
            else if ( (int)(count[3]/50) <= 1)
                pos_cur_x[3] += 2*shift_x[3]*(going_Right[3]==TRUE? 1:-1);  //speed-up
            else if ( (int)(count[3]/50) <= 2)
                pos_cur_x[3] += 3*shift_x[3]*(going_Right[3]==TRUE? 1:-1);  //turbo speed-up
            else if ( (int)(count[3]/50) <= 3)
                pos_cur_x[3] += 2*shift_x[3]*(going_Right[3]==TRUE? 1:-1);  //speed-up
            else if ( (int)(count[3]/50) <= 4)
                pos_cur_x[3] += 1*shift_x[3]*(going_Right[3]==TRUE? 1:-1);
        }
             
        
        /************************************/
        /***** red window¡Agoing L or R *****/
        /************************************/
        if ( atoi(argv[1]) == Window_Moving )
            window[0]->MoveTo(window[0], pos_cur_x[0], pos_cur_y[0]);
    
        
        /******************************************************/
        /***** green window¡A£\-value/zoom-in-out/going LU *****/
        /******************************************************/
        if ( atoi(argv[1]) == Color_Change )
        {
            if ( ((int)(count[1]/50) <= 0) && (count[1]%50 == 1) )  // scale / shift /color-change every 50 rounds
            {
                window[1]->SetOpacity( window[1], 120 );
               
                window[1]->Resize( window[1], desc[1].width, desc[1].height);
               
                window[1]->MoveTo(window[1], pos_cur_x[1], pos_cur_y[1]);
               
                window_surface[1]->FillRectangle( window_surface[1],
                                                  0, 0,
                                                  desc[1].width, desc[1].height);      
            }
            else if ( ((int)(count[1]/50) <= 1) && (count[1]%50 == 1) )  // scale / shift /color-change every 50 rounds
            {
                window[1]->SetOpacity( window[1], 150 );
               
                window[1]->Resize( window[1], desc[1].width+shift_x[1], desc[1].height+shift_y[1]);
               
                window[1]->MoveTo(window[1], pos_cur_x[1], pos_cur_y[1]);
               
                window_surface[1]->FillRectangle( window_surface[1],
                                                  0, 0,
                                                  desc[1].width+shift_x[1], desc[1].height+shift_y[1]);      
            }
            else if ( ((int)(count[1]/50) <= 2) && (count[1]%50 == 1) )  // scale / shift /color-change every 50 rounds
            {
                window[1]->SetOpacity( window[1], 180 );
               
                window[1]->Resize( window[1], desc[1].width+shift_x[1]*2, desc[1].height+shift_y[1]*2);
               
                window[1]->MoveTo(window[1], pos_cur_x[1], pos_cur_y[1]);
               
                window_surface[1]->FillRectangle( window_surface[1],
                                                  0, 0,
                                                  desc[1].width+shift_x[1]*2, desc[1].height+shift_y[1]*2);      
            }
        
        }
        
        
        /*******************************************/
        /***** blue window¡A£\-value/zoom-in-out*****/
        /*******************************************/
        if ( atoi(argv[1]) == Color_Change )
        {
    
            if ( ((int)(count[2]/150) <= 0) && (count[2]%150 == 1) )  // scale /color-change every 150 rounds
            {
                window[2]->SetOpacity( window[2], 120 );
               
                window[2]->Resize( window[2], desc[2].width, desc[2].height);
            
                window_surface[2]->FillRectangle( window_surface[2],
                                                  0, 0,
                                                  desc[2].width, desc[2].height);      
            }
            else if ( ((int)(count[2]/150) <= 1) && (count[2]%150 == 1) )  // scale /color-change every 150 rounds
            {
                window[2]->SetOpacity( window[2], 150 );
               
                window[2]->Resize( window[2], desc[2].width+shift_x[2], desc[2].height+shift_y[2]);
            
                window_surface[2]->FillRectangle( window_surface[2],
                                                  0, 0,
                                                  desc[2].width+shift_x[2], desc[2].height+shift_y[2]);      
            }
            else if ( ((int)(count[2]/150) <= 2) && (count[2]%150 == 1) )  // scale /color-change every 150 rounds
            {
                window[2]->SetOpacity( window[2], 180 );
               
                window[2]->Resize( window[2], desc[2].width+shift_x[2]*2, desc[2].height+shift_y[2]*2);
            
                window_surface[2]->FillRectangle( window_surface[2],
                                                  0, 0,
                                                  desc[2].width+shift_x[2]*2, desc[2].height+shift_y[2]*2);      
            }  
        
        }
        
        
        /***************************************************/
        /***** yellow-pink region in window¡Agoing L&R *****/
        /***************************************************/
        if (atoi(argv[1]) == ContentOfWindow_Moving )
        {
            window[3]->SetOpacity( window[3], 240 );
        
            window_surface[3]->Clear( window_surface[3], 0, 0, 0, 0 );
         
            //yellow region
            window_surface[3]->SetColor( window_surface[3],
                                         0xFF, 0xFF, 0x00, 0xFF );
            window_surface[3]->FillRectangle( window_surface[3],
                                              pos_cur_x[3]-pos_start_x[3], 0,
                                              150, 300 );     
            //pink region 
            window_surface[3]->SetColor( window_surface[3],
                                         0xFF, 0x00, 0xFF, 0xFF );
            window_surface[3]->FillRectangle( window_surface[3],
                                              (desc[3].width-150)-(pos_cur_x[3]-pos_start_x[3]), 0,
                                              150, 300 );     
        }
        
        
        /****************/
        /***** Flip *****/
        /****************/                                    
        if ( atoi(argv[1]) == Window_Moving )
            window_surface[0]->Flip( window_surface[0], NULL, 0 ); 
        if ( atoi(argv[1]) == Color_Change )
            window_surface[1]->Flip( window_surface[1], NULL, 0 ); 
        if ( atoi(argv[1]) == Color_Change )
            window_surface[2]->Flip( window_surface[2], NULL, 0 );
        if (atoi(argv[1]) == ContentOfWindow_Moving )
            window_surface[3]->Flip( window_surface[3], NULL, 0 );
        
        //sleep(1);
        //usleep(1000);
   
    }  //for (i=0; i<LOOPS; i++)
    

    /***** Release *****/
    for(i=0; i<4; i++)
    {
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }

    layer->Release( layer );
    dfb->Release( dfb );
     
    return 42;
}