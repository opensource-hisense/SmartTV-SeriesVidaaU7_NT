#include <directfb.h>
#include "dfb_util_common.h"
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
    dfb->GetDisplayLayer( dfb, 0, &layer ); // DFB Layer 0

    DFBDisplayLayerConfig conf;
    layer->SetCooperativeLevel(layer,DLSCL_ADMINISTRATIVE);
    conf.flags = (DFBDisplayLayerConfigFlags)(DLCONF_WIDTH | DLCONF_HEIGHT );
    conf.width = 640;
    conf.height = 480;
    DFBResult ret = layer->SetConfiguration(layer,&conf);
    printf("\33[1;31m debra: change to %d_%d error: %d\33[0m\n", conf.width,conf.height,(int)ret);

    int type = 1;
again:
    printf(" 1: 960_540 \n 2: 1280_720 \n 3: 1920_1080 \n 4: 720_576 \n 5: 640_480 \n");
    scanf("%d", &type);
    if(type == 1) {
        conf.width = 960;
        conf.height = 540;
    }
    else if(type == 2) {
      conf.width = 1280;
      conf.height = 720;
    }
    else if(type == 3) {
      conf.width = 1920;
      conf.height = 1080;
    }
    else if(type == 4) {
      conf.width = 720;
      conf.height = 576;
    }
    else if(type == 5) {
      conf.width = 640;
      conf.height = 480;
    }
    printf("\33[1;31m debra: change to %d_%d\33[0m\n", conf.width,conf.height);
    layer->SetConfiguration(layer,&conf);
goto again;
    return 0;
}

