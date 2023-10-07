#include <unistd.h>

#include "dfb_util_testbench.h"
#include "dfb_util.h"


// Callback function for VideoProvider
void callback_flip (void *ctx)
{
	IDirectFBSurface *surface = (IDirectFBSurface*) ctx;
	surface->Flip (surface, 0, (DFBSurfaceFlipFlags) 0);
}

// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestGIFPlaying : public CTestCase
{
public:
	static IDirectFB               *dfb;
	static IDirectFBSurface *primary;

    virtual void initResource()
	{
		DFBSurfaceDescription dsc;

		DFBCHECK(DirectFBInit( 0, 0));
		DFBCHECK(DirectFBCreate( &dfb ));

		/* get the primary surface, i.e. the surface of the primary layer we have
		exclusive access to */
		dsc.flags = DSDESC_CAPS;
		dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY | DSCAPS_DOUBLE);
		dfb->CreateSurface( dfb, &dsc, &primary );
	} 

    virtual void destroyResource()
	{
		primary->Release( primary );
		dfb->Release( dfb );
	} 


	void play_gif_animation  ()
	{
		IDirectFBVideoProvider * provider = NULL;

		DFBCHECK(dfb->CreateVideoProvider(dfb, "./image.gif", &provider));

		if (provider)
		{
			provider->SetPlaybackFlags (provider, DVPLAY_LOOPING);
			provider->PlayTo (provider, primary, 0, callback_flip, (void*)primary);		
		}
	}

    // Must to override
    int main( int argc, char *argv[] )
    {
		play_gif_animation();

		while(1)
			sleep(10000);

        return 0;
    }
};

IDirectFB               * TestGIFPlaying::dfb = 0;
IDirectFBSurface * TestGIFPlaying::primary = 0;


// Step 2: Add it into the Execution List
//ADD_TESTCASE(TestGIFPlaying);