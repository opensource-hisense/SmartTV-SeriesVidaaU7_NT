#include "dfb_util_testbench.h"
#include "dfb_util.h"


// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestDrawPic : public CTestCase
{
public:

    // Must to override
    int main( int argc, char *argv[] )
    {
        IDirectFB *dfb = NULL;
        IDirectFBSurface *primary = NULL;
        int screen_width  = 0;
        int screen_height = 0;
    
        int i;
        DFBSurfaceDescription dsc;
        IDirectFBImageProvider *provider;
        IDirectFBSurface *logo = NULL;
        
        dfb = DFBEnvBase::GetDFB();
        DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
        dsc.flags = DSDESC_CAPS;
        dsc.caps  = /*DSCAPS_PRIMARY |*/ DSCAPS_FLIPPING;
        DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
        DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
        DFBCHECK (dfb->CreateImageProvider (dfb, DATADIR"/dfblogo.png", &provider));

        DFBCHECK (provider->GetSurfaceDescription (provider, &dsc));
        DFBCHECK (dfb->CreateSurface( dfb, &dsc, &logo ));
        DFBCHECK (provider->RenderTo (provider, logo, NULL));

        provider->Release(provider);
                
        for (i = -dsc.width; i< screen_width; i--)
        {
            DFBCHECK (primary->FillRectangle (primary, 0, 0, screen_width, screen_height));
            DFBCHECK (primary->Blit (primary, logo, NULL, i, (screen_height - dsc.height) / 2));
            DFBCHECK (primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC));
        }
        
        logo->Release (logo);
        primary->Release (primary);        
                	
        return 0;
    }
};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestDrawPic);
