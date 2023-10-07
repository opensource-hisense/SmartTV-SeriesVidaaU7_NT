#include <unistd.h>
#include "directfb.h"
#include "test_OSD_Blending.h"

/* macro for a safe call to release functions */
#define SAFE_RELEASE(PTR)       \
    if(PTR)                     \
    {                           \
        PTR->Release(PTR);      \
    }                           \
    PTR = NULL;                 


/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                  \
{                                                                       \
    DFBResult err = x;                                                  \
    if (err != DFB_OK)                                                  \
    {                                                                   \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );          \
        DirectFBErrorFatal( #x, err );                                  \
    }                                                                   \
}

class OSD_LayerBlending
{

public:
    
    DFBResult    Init(int U_ID, int M_ID, int L_ID, int Out_ID);
    DFBResult    Render();

    static OSD_LayerBlending *GetInstance()
    {
        /* Initialization on first use */
        if (!m_pInstance)
            m_pInstance = new OSD_LayerBlending;
        
        return m_pInstance;
    }

    static void Release()
    {
        if (m_pInstance)
            delete m_pInstance;

        m_pInstance = NULL;
    }

private:   
    
    OSD_LayerBlending();
   ~OSD_LayerBlending();
    
    static OSD_LayerBlending       *m_pInstance;    // singleton instance    

    IDirectFB                      *dfb;

    IDirectFBDisplayLayer          *layer_Upper;
    IDirectFBDisplayLayer          *layer_Middle;
    IDirectFBDisplayLayer          *layer_Lower;
    IDirectFBDisplayLayer          *layer_Output;

    IDirectFBSurface               *surface_Upper;
    IDirectFBSurface               *surface_Middle;
    IDirectFBSurface               *surface_Lower;
    IDirectFBSurface               *surface_Output;
    
    int                             UpperLayerID;
    int                             MiddleLayerID;
    int                             LowerLayerID;
    int                             OutputLayerID;

};

OSD_LayerBlending* OSD_LayerBlending::m_pInstance = 0;  // Default initialization

OSD_LayerBlending::OSD_LayerBlending()
{
    dfb             = NULL;
    
    layer_Upper     = NULL;
    layer_Middle    = NULL;
    layer_Lower     = NULL;
    layer_Output    = NULL;

    surface_Upper   = NULL;
    surface_Middle  = NULL;
    surface_Lower   = NULL;
    surface_Output  = NULL;

    UpperLayerID    = -1;
    MiddleLayerID   = -1;
    LowerLayerID    = -1;
    OutputLayerID   = -1;
}

OSD_LayerBlending::~OSD_LayerBlending()
{
    SAFE_RELEASE(dfb);
}

DFBResult OSD_LayerBlending::Init(int U_ID, int M_ID, int L_ID, int Out_ID)
{
    DFBResult ret;
    
    UpperLayerID    = U_ID;
    MiddleLayerID   = M_ID;
    LowerLayerID    = L_ID;
    OutputLayerID   = Out_ID;

    if ( U_ID < 0 || M_ID < 0 || L_ID < 0 || Out_ID < 0 )
    {
        printf("\n%s, line=%d, Init Layer ID Fail \n", __FUNCTION__, __LINE__);
        return ret = DFB_FAILURE;
    }

    /* Initialize DirectFB */
    DFBCHECK(DirectFBInit(NULL, NULL));
    
    /* Create the super interface */
    DFBCHECK(DirectFBCreate( &dfb ));

}

DFBResult OSD_LayerBlending::Render()
{
    DFBResult ret;

    /* Retrieve an interface to the Output layer */
    ret = dfb->GetDisplayLayer( dfb, OutputLayerID, &layer_Output);

    if (DFB_OK == ret)
    {
        /* Acquire exclusive access to the layer. */
        DFBCHECK(layer_Output->SetCooperativeLevel(layer_Output, DLSCL_ADMINISTRATIVE)); 

        /* Get the surface of the Output layer */
        DFBCHECK(layer_Output->GetSurface( layer_Output, &surface_Output));

        /* disable blending */
        surface_Output->SetBlittingFlags(surface_Output, DSBLIT_NOFX);
    }
    else
    {
        D_INFO("\n%s, line=%d, can't layer ID:Upper \n", __FUNCTION__, __LINE__);
    }

  
    /* Retrieve an interface to the Lower layer */
    ret = dfb->GetDisplayLayer( dfb, LowerLayerID, &layer_Lower);
    
    if (DFB_OK == ret)
    {
        /* Acquire exclusive access to the layer. */
        DFBCHECK(layer_Lower->SetCooperativeLevel(layer_Lower, DLSCL_ADMINISTRATIVE)); 

        /* Get the surface of the Lower layer */
        DFBCHECK(layer_Lower->GetSurface( layer_Lower, &surface_Lower));

        /* Blit an area scaled from the Lower layer to Output layer*/
        surface_Output->StretchBlit(surface_Output, surface_Lower, 0, 0);     

        /* enables blending and uses alphachannel */
        surface_Output->SetBlittingFlags(surface_Output, DSBLIT_BLEND_ALPHACHANNEL);
    }
    else
    {
        D_INFO("\n%s, line=%d, can't layer ID:Lower \n", __FUNCTION__, __LINE__);
    }
    
    /* Retrieve an interface to the Middle layer */
    ret = dfb->GetDisplayLayer( dfb, MiddleLayerID, &layer_Middle);
    
    if (DFB_OK == ret)
    {
        /* Acquire exclusive access to the layer. */
        DFBCHECK(layer_Middle->SetCooperativeLevel(layer_Middle, DLSCL_ADMINISTRATIVE)); 

        /* Get the surface of the Middle layer */
        DFBCHECK(layer_Middle->GetSurface( layer_Middle, &surface_Middle));

        /* Blit an area scaled from the Middle layer to Output layer */
        surface_Output->StretchBlit(surface_Output, surface_Middle, 0, 0);

        /* enables blending and uses alphachannel */
        surface_Output->SetBlittingFlags(surface_Output, DSBLIT_BLEND_ALPHACHANNEL);
    }
    else
    {
        D_INFO("\n%s, line=%d, can't layer ID:Middle \n", __FUNCTION__, __LINE__);
    }

    /* Retrieve an interface to the Upper layer */
    ret = dfb->GetDisplayLayer( dfb, UpperLayerID, &layer_Upper);
    
    if (DFB_OK == ret)
    {
        /* Acquire exclusive access to the layer. */
        DFBCHECK(layer_Upper->SetCooperativeLevel(layer_Upper, DLSCL_ADMINISTRATIVE)); 

        /* Get the surface of the Upper layer */
        DFBCHECK(layer_Upper->GetSurface( layer_Upper, &surface_Upper));

        /* Blit an area scaled from the Upper layer to Output layer*/
        surface_Output->StretchBlit(surface_Output, surface_Upper, 0, 0);

        /* enables blending and uses alphachannel */
        surface_Output->SetBlittingFlags(surface_Output, DSBLIT_BLEND_ALPHACHANNEL);
    }
    else
    {
        D_INFO("\n%s, line=%d, can't layer ID:Upper \n", __FUNCTION__, __LINE__);
    }

    if (surface_Output != NULL)
    {
        DFBCHECK(surface_Output->Flip(surface_Output, 0, DSFLIP_NONE));
    }


    SAFE_RELEASE(layer_Upper);
    SAFE_RELEASE(surface_Upper);

    SAFE_RELEASE(layer_Middle);
    SAFE_RELEASE(surface_Middle);

    SAFE_RELEASE(layer_Lower);
    SAFE_RELEASE(surface_Lower);

    SAFE_RELEASE(layer_Output);
    SAFE_RELEASE(surface_Output);

    return DFB_OK;

}

////////////////////////    API interface   ////////////////////////////////////
void OSD_LayerBlending_Init(int UpperLayerID, int MiddleLayerID, int LowerLayerID, int OutputLayerID)
{
    DFBResult ret;

    if( OSD_LayerBlending * instant = OSD_LayerBlending::GetInstance() )
        ret = instant->Init(UpperLayerID, MiddleLayerID, LowerLayerID, OutputLayerID);

    if(ret)
       D_INFO("\n%s, line=%d, Init Fail \n", __FUNCTION__, __LINE__); 

}

void OSD_LayerBlending_Render()
{
    DFBResult ret;
    
    if( OSD_LayerBlending * instant = OSD_LayerBlending::GetInstance() )
        ret = instant->Render();

    if(ret)
       D_INFO("\n%s, line=%d, Render Fail \n", __FUNCTION__, __LINE__);     
}

void OSD_LayerBlending_Release()
{
    
    if( OSD_LayerBlending * instant = OSD_LayerBlending::GetInstance() )
    	instant->Release();
    
}
////////////////////////    API interface End   ////////////////////////////////////


int main( int argc, char *argv[] )
{

    int UpperLayerID;
    int MiddleLayerID;
    int LowerLayerID;
    int OutputLayerID;

    // set layer ID
    UpperLayerID    = 0;
    MiddleLayerID   = 1;
    LowerLayerID    = 2;
    OutputLayerID   = 3;
    
    OSD_LayerBlending_Init(UpperLayerID, MiddleLayerID, LowerLayerID, OutputLayerID);

    while(1)
    {
        OSD_LayerBlending_Render();
        usleep(333);
    }

    OSD_LayerBlending_Release();

    return 1;
}

