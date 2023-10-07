#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "dfb_util_testbench.h"
#include "dfb_util.h"



// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestMultiProcess : public CTestCase
{
public:

    virtual void initResource(){} // Invoke before the "main" function (optional)
    virtual void destroyResource(){} // Invoke after the "main" function (optional)


    int main( int argc, char *argv[] )
    {
        pid_t child_pid;
        child_pid = fork ();

        if(child_pid > 0)
        {
            printf("\n[Master] pid = %d, child_pid = %d\n", (int) getpid(), child_pid);
            run_master(argc, argv);
        }
        else if (child_pid <0)
        {
            printf("Error\n");
        }
        else
        {
            run_slave(argc, argv);
        }

        return 0;
    }


    void run_master(int argc, char *argv[])
    {
        printf("[Master] %d\n", __LINE__);

        DirectFBInit(&argc, &argv);

        IDirectFB *dfb;
        DirectFBCreate(&dfb);

        IDirectFBDisplayLayer  *layer0, *layer1;

        // DFB Layer 0
        dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer0 ); 

    #if 0
        // DFB Layer 1
        dfb->GetDisplayLayer( dfb, 1, &layer1 ); 
    #endif

        printf("[Master] %d\n", __LINE__);

        wait(NULL);

        printf("[Master] %d\n", __LINE__);

        while(1) {
            sleep(10);
        }
            

        printf("[Master] %d\n", __LINE__);
    }


    static const int LAYER_ID = 1;

    void run_slave(int argc, char *argv[])
    {
        sleep(3);
        printf("[Slave] pid = %d\n", (int) getpid());

        DirectFBInit(&argc, &argv);


        IDirectFB *dfb;
        DirectFBCreate(&dfb);


        IDirectFBDisplayLayer  *layer;
        IDirectFBWindow        *window1;
        IDirectFBSurface       *window_surface1;


        dfb->GetDisplayLayer( dfb, LAYER_ID, &layer );
        layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );   


        DFBWindowDescription win_desc;
        win_desc.flags = (DFBWindowDescriptionFlags) (DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_POSX | DWDESC_POSY);
        win_desc.width = 1280;
        win_desc.height = 720;
        win_desc.posx = 0;
        win_desc.posy = 0;
        win_desc.caps = DWCAPS_ALPHACHANNEL;


        layer->CreateWindow( layer, &win_desc, &window1 );
        window1->GetSurface( window1, &window_surface1 );

        window_surface1->Clear(window_surface1, 0xff, 0xff, 0, 0xff);
        window_surface1->Flip(window_surface1, NULL, (DFBSurfaceFlipFlags)0 );
        window1->SetOpacity(window1, 0xff);

        sleep(5);
        printf("[Slave] pid = %d : Release begin!\n", (int) getpid());

        window_surface1->Release(window_surface1);
        window1->Release(window1);
        layer->Release(layer);
        dfb->Release(dfb);


        printf("[Slave] pid = %d : Release done!\n", (int) getpid());
    }


};

// Step 2: Add it into the Execution List
//ADD_TESTCASE(TestMultiProcess);
