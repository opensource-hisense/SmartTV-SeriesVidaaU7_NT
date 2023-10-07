#include <directfb.h>
//#include "dfb_util_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



int main( int argc, char *argv[] )
{
    DirectFBInit(&argc, &argv);

    IDirectFB *dfb;
    DirectFBCreate(&dfb);

    IDirectFBDisplayLayer  *layer;
    IDirectFBDisplayLayer  *layer1;

    IDirectFBSurface *layer_surface = NULL;
    IDirectFBSurface *layer_surface1 = NULL;

    void *data;
    int   pitch;

    dfb->GetDisplayLayer( dfb, 0, &layer ); // DFB Layer UI
    dfb->GetDisplayLayer( dfb, 1, &layer1 ); // DFB Layer dummy

    DFBDisplayLayerConfig conf;
    
    layer->SetCooperativeLevel(layer,DLSCL_ADMINISTRATIVE);
    layer1->SetCooperativeLevel(layer1,DLSCL_ADMINISTRATIVE);

    while(1)
    {

        layer1->SetLevel(layer, 1);

        layer->GetSurface( layer, &layer_surface );
        layer1->GetSurface( layer1, &layer_surface1 );
        
        /* 
            copy layer UI to layer dummy
        */
        layer_surface1->StretchBlit(layer_surface1, layer_surface, 0, 0);
        layer_surface1->Flip(layer_surface1, 0, DSFLIP_NONE);

        /* 
            disalbe layer UI
        */
        layer->SetLevel(layer, 0);

        conf.flags = (DFBDisplayLayerConfigFlags)(DLCONF_WIDTH | DLCONF_HEIGHT );

        int type = 1;

        printf(" 1: 960_540 \n 2: 1280_720 \n 3: 1920_1080 \n 4: 720_576 \n 5: 640_480 \n");
        scanf("%d", &type);


        switch (type)
        {
            case 1 :
                conf.width = 960;
                conf.height = 540;
                break;

            case 2 :
                conf.width = 1280;
                conf.height = 720;
                break;

            case 3 :
                conf.width = 1920;
                conf.height = 1080;
                break;            

            case 4 :
                conf.width = 720;
                conf.height = 576;
                break;

            case 5 :
                conf.width = 640;
                conf.height = 480;
                break;

            default :
                conf.width = 1280;
                conf.height = 720;
                break;

        }
        
        printf("\33[1;31m change to %d_%d\33[0m\n", conf.width, conf.height);
        layer->SetConfiguration(layer,&conf);

        layer_surface->Lock(layer_surface, DSLF_WRITE, &data, &pitch );
        layer_surface->Unlock(layer_surface);
        
        layer_surface1->Release(layer_surface1);  
    }

    return 0;
}

