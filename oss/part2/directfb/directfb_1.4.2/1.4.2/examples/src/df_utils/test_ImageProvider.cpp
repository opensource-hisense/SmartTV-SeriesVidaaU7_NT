#include <dlfcn.h>
#include "dfb_util_testbench.h"
#include "dfb_util.h"

//#include "drvSYS.h"

class TestImageProvider : public CTestCase
{
public:

    void Global_Init ()
    {
        typedef void  (*MDRV_SYS_GLOBALINIT) (void);
        MDRV_SYS_GLOBALINIT MDrv_SYS_GlobalInit;

        void *handle = NULL;
        handle = dlopen ("liblinux.so", RTLD_LAZY);
        if (!handle)
        {
            //fputs (dlerror(), stderr);
            //exit(1);
            printf("\n[Andy] %s!!!!\n\n\n", dlerror());        
        }

        MDrv_SYS_GlobalInit = (MDRV_SYS_GLOBALINIT)dlsym (handle, "MDrv_SYS_GlobalInit");
        if (const char* err = dlerror())
        {
            //fputs(err, stderr);
            //exit(1);
            printf("\n[Andy] %s!!!!\n\n\n", err);
        }

        MDrv_SYS_GlobalInit();
        dlclose(handle);
    }

    int main( int argc, char *argv[] )
    {
        //Global_Init ();

        DFBLayer layer (1);
        DFBWindow win (layer);
        //DFBImageProvider provider ("./rec/pp.jpg");
        DFBImageProvider provider (RESOURCE("pp.jpg"));


        //DFBDisplayLayerConfig layer_conf;
        //layer_conf.flags = (DFBDisplayLayerConfigFlags)( DLCONF_PIXELFORMAT);
        //layer_conf.pixelformat = /*DSPF_ARGB4444*/ /*DSPF_YUY2*/ DSPF_AYUV;
        //layer.SetConfiguration(&layer_conf);


        DFBWindowDescription win_desc;
        //win_desc.flags = DWDESC_PIXELFORMAT;
        //win_desc.pixelformat = DSPF_YUY2;
        win_desc.flags = (DFBWindowDescriptionFlags) (DWDESC_WIDTH | DWDESC_HEIGHT);
        win_desc.width = 1280;
        win_desc.height = 720;
        win.SetWindowDescription(win_desc);
        win.SetOpacity(0xff);

        DFBSurface surface;
        DFBSurfaceDescription sur_desc;
        sur_desc.flags = (DFBSurfaceDescriptionFlags) (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT);
        sur_desc.width = 250;
        sur_desc.height = 250;
        sur_desc.caps = DSCAPS_VIDEOONLY;
        sur_desc.pixelformat = DSPF_ARGB /*DSPF_AYUV*/ /*DSPF_YUY2*/ /*DSPF_NV16*/;
        surface.SetSurfaceDescription(sur_desc);

        //while(1)
        {
            provider.RenderTo(surface, 0);
        }

        IDirectFBSurface * sur = win.GetSurface();
        unsigned char * addr = 0;
        int pitch;
        unsigned long argb;

        sur->Lock(sur, DSLF_READ, (void**)&addr, &pitch);
        if (addr)
        {
            argb = *(unsigned long *) (addr + 124 * pitch + 124);
            printf("ARGB = 0x%08x\n", argb);
        }  
        sur->Unlock(sur);

        while(1);
        return 0;
    }
};

ADD_TESTCASE(TestImageProvider);