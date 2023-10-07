#include "dfb_util_testbench.h"
#include "dfb_util.h"

// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestFading : public CTestCase
{
public:

    // Must to override
    int main( int argc, char *argv[] )
    {
        DFBLayer layer (1);
        DFBWindow win (layer);
        //DFBImageProvider provider ("./rec/pp.jpg");
        DFBImageProvider provider (RESOURCE("pp.jpg"));


        DFBWindowDescription win_desc;
        win_desc.flags = (DFBWindowDescriptionFlags) (DWDESC_WIDTH | DWDESC_HEIGHT);
        win_desc.width = 1280;
        win_desc.height = 720;
        win.SetWindowDescription(win_desc);
        win.SetOpacity(0xff);



        IDirectFBSurface * img_surface = 0;
        IDirectFBSurface * fading_surface = 0;
        DFBSurfaceDescription sur_desc;
        sur_desc.flags = (DFBSurfaceDescriptionFlags) (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT);
        sur_desc.width = 250;
        sur_desc.height = 250;
        sur_desc.caps = DSCAPS_VIDEOONLY;
        sur_desc.pixelformat = DSPF_ARGB;

        DFBEnvBase::GetDFB()->CreateSurface(DFBEnvBase::GetDFB(), &sur_desc, &img_surface);
        DFBEnvBase::GetDFB()->CreateSurface(DFBEnvBase::GetDFB(), &sur_desc, &fading_surface);

        // Image decode
        provider.RenderTo(img_surface, 0);


        unsigned char a = 0xff;

        fading_surface->SetBlittingFlags(fading_surface, DSBLIT_BLEND_ALPHACHANNEL);
        fading_surface->SetSrcBlendFunction(fading_surface, DSBF_DESTALPHA);
        fading_surface->SetDstBlendFunction(fading_surface, DSBF_ZERO);

        win.SetBlittingFlags(DSBLIT_BLEND_ALPHACHANNEL);

        while(1)
        {
            fading_surface->Clear(fading_surface, 0, 0, 0, a);
            fading_surface->Blit(fading_surface, img_surface, 0, 0, 0);

            // Output to Frame Buffer
            win.Clear(0xff, 0xff, 0, 0xff);
            win.Blit(fading_surface, 0, 0, 0);
            win.Flip(0, (DFBSurfaceFlipFlags)0);

            // Animating
            a++;
        }


        while(1);

        return 0;
    }
};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestFading);