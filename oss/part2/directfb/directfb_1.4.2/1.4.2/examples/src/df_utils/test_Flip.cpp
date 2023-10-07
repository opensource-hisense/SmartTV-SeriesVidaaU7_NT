#include <unistd.h>
#include "dfb_util_testbench.h"
#include "dfb_util.h"
#include "util.h"


#include <pthread.h>



#define MSOS_TYPE_LINUX
#include "MsOS.h"
#include <apiGFX.h>
#include <apiGOP.h>

#define USE_FLIP_ONCE 0


#define NONE "\033[m"
#define RED "\033[0;32;31m"

#define MS_TO_US(T) (T * 1000)

class TestFlip : public CTestCase
{
public:

#if 0  // In FPS
    int main( int argc, char *argv[] )
    {
        const int FPS = 500; // FPS: 60
        //const int FPS = 250; // FPS: 60
        //const int FPS = 125; // FPS: 40 ~ 60
        //const int FPS = 72; // FPS: 30 ~ 40
        //const int FPS = 61; // FPS: 30
        //const int FPS = 60; // FPS: 30
        //const int FPS = 50; // FPS:
        //const int FPS = 40; // FPS:
        //const int FPS = 30; // FPS: 

        unsigned int time_us = 1000000 / FPS;

        FPSData fps;

        DFBLayer layer (1);

        DFBImageProvider provider ("./rec/pp.jpg");
        provider.RenderTo(layer, 0);

        fps_init( &fps );

        while(1)
        {

            usleep(time_us);

            layer.Flip(0, (DFBSurfaceFlipFlags)0);

            fps_count( &fps, 2000 );

            printf("[FPS] %s\n", fps.fps_string);
        }


        return 0;
    }
#else

    // In ms
    int main( int argc, char *argv[] )
    {
        // Layer Flip
        
        // VSYNC x4
        unsigned int time_ms = 64; // FPS:
        //unsigned int time_ms = 60; // FPS:
        //unsigned int time_ms = 56; // FPS:
        //unsigned int time_ms = 52; // FPS:
        
        // VSYNC x3
        //unsigned int time_ms = 48; // FPS:
        //unsigned int time_ms = 44; // FPS:
        //unsigned int time_ms = 40; // FPS:
        //unsigned int time_ms = 36; // FPS: 20
        
        // VSYNC x2
        //unsigned int time_ms = 32; // FPS: 20
        //unsigned int time_ms = 28; // FPS: 30
        //unsigned int time_ms = 24; // FPS: 30
        //unsigned int time_ms = 20; // FPS: 30

        // VSYNC x1
        //unsigned int time_ms = 16; // FPS: 30       
        //unsigned int time_ms = 12; // FPS: 60
        //unsigned int time_ms = 8; // FPS: 60
        //unsigned int time_ms = 4; // FPS: 60
        //unsigned int time_ms = 1; // FPS: 60


        // Window Flip
        //unsigned int time_ms = 8; // FPS: 30
        //unsigned int time_ms = 7; // FPS: 40
        //unsigned int time_ms = 6; // FPS: 50
        //unsigned int time_ms = 5; // FPS: 53 ~ 58
        //unsigned int time_ms = 4; // FPS: 60 
         


        //pthread_t  tid = pthread_self();
        //printf("[Andy] 0x%x\n", tid);
        //while(1);

        

        DFBLayer layer (1);
        DFBDisplayLayerConfig conf;
        layer.SetCooperativeLevel(DLSCL_ADMINISTRATIVE);
        conf.flags = (DFBDisplayLayerConfigFlags)(DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_BUFFERMODE);
        conf.width = 1920;
        conf.height = 1080;
        conf.buffermode = DLBM_TRIPLE /*DLBM_BACKVIDEO*/;
        layer.SetConfiguration(&conf);
        
        DFBImageProvider provider ("./rec/pp.jpg");

        
        provider.RenderTo(layer, 0);


/*
        DFBSurface surface_1, surface_2;
        DFBSurfaceDescription sur_desc;
        sur_desc.flags = (DFBSurfaceDescriptionFlags) (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT);
        sur_desc.width = 1920;
        sur_desc.height = 1080;
        sur_desc.caps = DSCAPS_VIDEOONLY;
        sur_desc.pixelformat = DSPF_ARGB;
        surface_1.SetSurfaceDescription(sur_desc);
        surface_2.SetSurfaceDescription(sur_desc);
        provider.RenderTo(surface_1, 0);
*/

/*
        DFBWindowDescription win_desc;
        win_desc.flags = (DFBWindowDescriptionFlags) (DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
        win_desc.width = 1920;
        win_desc.height = 1080;
        win_desc.caps = DWCAPS_DOUBLEBUFFER;
        DFBWindow win (layer);
        win.SetWindowDescription(win_desc);
        win.SetOpacity(0xff);
        provider.RenderTo(win, 0);
*/


        unsigned int input_fps = 1000 / time_ms;
#if 1
        FPSData fps;

        unsigned char * addr = 0;
        int pitch;

        // Init Counter
        fps_init( &fps );

#if USE_FLIP_ONCE
        if(1)
#else
        while(1)
#endif
        {
            usleep(MS_TO_US(time_ms));

/*
            surface_2.Blit(surface_1, 0, 0, 0);
            surface_2.GetSurface()->Lock(surface_2.GetSurface(), DSLF_READ, (void**)&addr, &pitch);
            surface_2.GetSurface()->Unlock(surface_2.GetSurface());
*/

/*
#if USE_FLIP_ONCE
            printf(RED"[Andy] Begin Flip +++++++++++++++++ \n"NONE);
            win.Flip(0, DSFLIP_NONE);
            printf(RED"[Andy] Begin End ------------------ \n"NONE);
#else
            win.Flip(0, DSFLIP_NONE);
#endif
*/

            layer.Flip(0, DSFLIP_NONE);

            fps_count( &fps, 1000 );

            printf("[ time = %d ms (%d fps) FPS] %s\n", time_ms, input_fps, fps.fps_string);
            //printf("[FPS] %s\n", fps.fps_string);
        }

        while(1);

#else
        float fps = 0;
        PProbe probe;

        // Assign an interval which you want to use to average the result. (more large, more precise)
        probe.setRepeatInterval(60);

        while(1)
        {
            // Start counting
            probe.start();

            usleep(MS_TO_US(time_ms));

            layer.Flip(0, DSFLIP_NONE);

            // Stop counting
            probe.stop();

            //if ((fps = probe.getFPS()) > 0.f)
            //{
            //    printf("[ time = %d ms (%d fps) FPS] %.1f\n", time_ms, input_fps, fps);
            //    //printf("[FPS] %f\n", fps);
            //}

            printf("[ time = %d ms (%d fps) FPS] %s\n", time_ms, input_fps, probe.getStringFPS());
        }
#endif
        return 0;
    }

#endif

};


class TestGFlip : public CTestCase
{
public:


    // In ms
    int main( int argc, char *argv[] )
    {
        //unsigned int time_ms = 60; // FPS: 13.3
        //unsigned int time_ms = 56; // FPS: 15
        //unsigned int time_ms = 52; // FPS: 15
        //unsigned int time_ms = 48; // FPS: 15 ~ 20
        //unsigned int time_ms = 44; // FPS: 19 ~ 20
        //unsigned int time_ms = 40; // FPS: 20
        //unsigned int time_ms = 36; // FPS: 20
        //unsigned int time_ms = 32; // FPS: 20 
        //unsigned int time_ms = 28; // FPS:  20
        //unsigned int time_ms = 24; // FPS: 20 ~ 30  
        //unsigned int time_ms = 20; // FPS:  30 
        //unsigned int time_ms = 16; // FPS: 30 ~ 33

        unsigned int time_ms = 8; // FPS:

        //unsigned int time_ms = 8; // FPS: 40 ~ 60
        //unsigned int time_ms = 4; // FPS:  40 ~ 60 
        //unsigned int time_ms = 1; // FPS:  

        unsigned int input_fps = 1000 / time_ms;




        DFBLayer layer (1);
        DFBWindow win (layer);

        DFBWindowDescription win_desc;
        win_desc.flags = (DFBWindowDescriptionFlags) (DWDESC_WIDTH | DWDESC_HEIGHT);
        win_desc.width = 1920;
        win_desc.height = 1080;
        win.SetWindowDescription(win_desc);
        win.SetOpacity(0xff);

        DFBImageProvider provider ("./rec/pp.jpg");


        DFBSurface surface_front, surface_back;
        DFBSurfaceDescription sur_desc;
        sur_desc.flags = (DFBSurfaceDescriptionFlags) (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT);
        sur_desc.width = 1920;
        sur_desc.height = 1080;
        sur_desc.caps = DSCAPS_VIDEOONLY;
        sur_desc.pixelformat = DSPF_ARGB;
        
        surface_front.SetSurfaceDescription(sur_desc);
        surface_back.SetSurfaceDescription(sur_desc);

        provider.RenderTo(surface_front, 0);


        win.Blit(surface_front, 0, 0, 0);



        DFBSurface surface_1, surface_2;
        DFBSurfaceDescription desc;
        desc.flags = (DFBSurfaceDescriptionFlags) (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT);
        desc.width = 1920;
        desc.height = 1080;
        desc.caps = DSCAPS_VIDEOONLY;
        desc.pixelformat = DSPF_ARGB;
        surface_1.SetSurfaceDescription(desc);
        surface_2.SetSurfaceDescription(desc);
        provider.RenderTo(surface_1, 0);

        //win.Flip(0, DSFLIP_NONE);
        //while(1);



        MS_U8 u16FBId = 0;
        MS_U16 u16QueueCnt = 1;
        MS_U16 tagID;
        MS_U8 gWinId = 0;

        //MS_U32 u32FlipAddr0 = 0x8b3eff00, u32FlipAddr1 = 0x0;
        //MS_U32 physicalAddr = 0x8b3eff00;


        MS_U32  physicalAddr = 0;
        void * addr = 0, * addr_phys = 0;
        int pitch = 0;
        IDirectFBSurface * dfb_sur = win.GetSurface();

        dfb_sur->Lock2(dfb_sur, DSLF_READ, &addr, &addr_phys, &pitch);
        physicalAddr = (MS_U32)addr_phys;
        dfb_sur->Unlock(dfb_sur);


        MApi_GOP_GWIN_CreateFBFrom3rdSurf(1920, 1080, E_MS_FMT_ARGB8888, physicalAddr, pitch, &u16FBId);

        gWinId = MApi_GOP_GWIN_CreateWin_Assign_FB(0 /* GOP Index */, u16FBId, 0, 0);

        tagID = MApi_GFX_GetNextTAGID(TRUE);

        //MApi_GOP_Switch_GWIN_2_FB_BY_ADDR(gWinId, physicalAddr, tagID, &u16QueueCnt);
        //while(1);

        
        
        FPSData fps;
        // Init Counter
        fps_init( &fps );

        int i = 0;

        while(1)
        {

            usleep(MS_TO_US(time_ms));


            dfb_sur->Blit(dfb_sur, surface_1.GetSurface(), 0, 0, 0);
            dfb_sur->Lock2(dfb_sur, DSLF_READ, &addr, &addr_phys, &pitch);
            physicalAddr = (MS_U32)addr_phys;
            dfb_sur->Unlock(dfb_sur);

            if (MApi_GOP_Switch_GWIN_2_FB_BY_ADDR(gWinId, physicalAddr, tagID, &u16QueueCnt))  
            {
                // swap physicalAddr here
                dfb_sur = i%2 == 0 ? surface_front.GetSurface() : win.GetSurface();
                //dfb_sur->Lock2(dfb_sur, DSLF_READ, &addr, &addr_phys, &pitch);
                //physicalAddr = (MS_U32)addr_phys;
                //dfb_sur->Unlock(dfb_sur);
            }
            else
            {
                printf(" flip error \n");
            }


            fps_count( &fps, 1000 );

            printf("[ time = %d ms (%d fps) FPS] %s\n", time_ms, input_fps, fps.fps_string);
            //printf("[FPS] %s\n", fps.fps_string);

            i++;
        }

        while(1);

        return 0;
    }


};

#if 1
ADD_TESTCASE(TestFlip);
#else
ADD_TESTCASE(TestGFlip);
#endif