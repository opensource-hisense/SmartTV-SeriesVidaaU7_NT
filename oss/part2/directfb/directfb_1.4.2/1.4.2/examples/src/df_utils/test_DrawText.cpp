#include "dfb_util_testbench.h"
#include "dfb_util.h"


// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestDrawText : public CTestCase
{
public:

    // Must to override
    int main( int argc, char *argv[] )
    {
        IDirectFB *dfb = NULL;
        IDirectFBSurface *primary = NULL;
        int screen_width  = 0;
        int screen_height = 0;
        IDirectFBFont *font = NULL;
        static char *text = "DirectFB rulez!";
    
        int i, width;
        DFBFontDescription font_dsc;
        DFBSurfaceDescription dsc;
        
        //DFBCHECK (DirectFBInit (&argc, &argv));
        //DFBCHECK (DirectFBCreate (&dfb));
        dfb = DFBEnvBase::GetDFB();
        DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
        dsc.flags = DSDESC_CAPS;
        dsc.caps  = /*DSCAPS_PRIMARY |*/ DSCAPS_FLIPPING;
        DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
        DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
        
        font_dsc.flags = DFDESC_HEIGHT;
        font_dsc.height = 48;
        DFBCHECK (dfb->CreateFont (dfb, FONT, &font_dsc, &font));
        DFBCHECK (primary->SetFont (primary, font));
        DFBCHECK (font->GetStringWidth (font, text, -1, &width));
        
        for (i = screen_width; i > -width; i--)
        {
            DFBCHECK (primary->SetColor (primary, 0x0, 0x0, 0x0, 0xFF));
            DFBCHECK (primary->FillRectangle (primary, 0, 0, screen_width, screen_height));
            DFBCHECK (primary->SetColor (primary, 0x80, 0x0, 0x20, 0xFF));
            DFBCHECK (primary->DrawString (primary, text, -1, i, screen_height / 2, DSTF_LEFT));
            DFBCHECK (primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC));    
        }
        
        font->Release (font);
        primary->Release (primary);
        //dfb->Release (dfb);
                	
        return 0;
    }
};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestDrawText);
