#include <stdlib.h>
#include <directfb.h>
#include "util.h"
#include "mtal.h"
#include "mtosd.h"
#include "mtpmx.h"
#include "mtgfx.h"

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                     \
               err = x;                                                    \
               if (err != DFB_OK) {                                        \
                    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
                    DirectFBErrorFatal (#x, err);                          \
               }

static IDirectFB*             dfb;
static IDirectFBSurface*      primary;

static int initUIResource(IDirectFBSurface** surface, DFBRectangle* rect)
{
    DFBSurfaceDescription desc;
    desc.flags          = (DFBSurfaceDescriptionFlags)( DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PIXELFORMAT | DSDESC_CAPS );
    desc.width          = rect->w;
    desc.height         = rect->h;
    desc.pixelformat    = DSPF_ARGB;
    desc.caps           = DSCAPS_FOUCE_VIDEOONLY;

    dfb->CreateSurface( dfb, &desc, surface );

    return DR_OK;
}

int main( int argc, char* argv[] )
{
    IDirectFBDisplayLayer* layer;
    DFBDisplayLayerConfig  config;
    IDirectFBImageProvider* provider;
    IDirectFBSurface*      surface_01;
    void*                  ps1;
    void*                  ps2;
    IDirectFBSurface*      surface_02;
    IDirectFBSurface*      logo;
    DFBRectangle           rect;
    DFBSurfaceDescription  desc;
    DFBResult              err;
    int                    select;

    if (argc < 2)
    {
        printf( " Usuage: <%s> <1/2/3>, 1:FHD, 2:HD, 3:FHD/HD\n", argv[0] );
        return -1;
    } else {
        select = atoi(argv[1]);
    }

    DFBCHECK (DirectFBInit( &argc, &argv ));
    DFBCHECK (DirectFBCreate( &dfb ));

    dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );


    // layer config
    dfb->GetDisplayLayer( dfb, 1, &layer );
    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

    config.flags      = DLCONF_BUFFERMODE;
    config.buffermode = DLBM_FRONTONLY;

    layer->SetConfiguration( layer, &config );

    int pitch1, pitch2;
    u32 u4_region, u4_region_list;
    u32 u4_region2, u4_region_list2;
    rect.x = rect.y = 0;
    rect.w = 1920;
    rect.h = 1080;
    MTAL_Init();
    MTOSD_Init();
    initUIResource( &surface_01, &rect );

    rect.w = 960;
    rect.h = 540;
    initUIResource( &surface_02, &rect );

    //decode image to logo surface
    dfb->CreateImageProvider( dfb, DATADIR"/biglogo.png", &provider );
    provider->GetSurfaceDescription( provider, &desc );
    printf( " Image info: w/h: %dx%d\n", desc.width, desc.height );
    DFBCHECK(dfb->CreateSurface( dfb, &desc, &logo ));
    provider->RenderTo( provider, logo, NULL );
    provider->Release( provider );

    //blit logo surface to display window
    surface_01->StretchBlit( surface_01, logo, NULL, NULL );
    surface_01->Flip( surface_01, NULL, DSFLIP_NONE );
    surface_02->StretchBlit( surface_02, logo, NULL, NULL );
    surface_02->Flip( surface_02, NULL, DSFLIP_NONE );

    surface_01->Lock( surface_01, DSLF_PHY, &ps1, &pitch1 );
    MTOSD_RGN_Create( &u4_region, 1920, 1080, ps1, 14, pitch1, 0, 0, 1920, 1080 );
    MTOSD_RGN_LIST_Create( &u4_region_list );
    MTOSD_RGN_Insert( u4_region, u4_region_list );
    surface_02->Lock( surface_02, DSLF_PHY, &ps2, &pitch2 );
    MTOSD_RGN_Create( &u4_region2, 960, 540, ps2, 14, pitch2, 0, 0, 960, 540 );
    MTOSD_RGN_LIST_Create( &u4_region_list2 );
    MTOSD_RGN_Insert( u4_region2, u4_region_list2 );

    printf(" PHY1: %p, PITCH1: %d,  PHY2: %p, PITCH2: 0x%x\n", ps1, pitch1, ps2, pitch2);

    int i = 0;
    u32 panel_width, panel_height;

    MTPMX_PANEL_GetResolution( &panel_width, &panel_height );

    for(i = 0; ; i++)
    {
        switch(select)
        {
        case 1:
            MTOSD_SC_Scale( 2, 1, 1920, 1080, panel_width, panel_height );
            MTOSD_PLA_FlipTo( 2, u4_region_list );
            MTOSD_PLA_Enable( 2, 1 );
            break;
        case 2:
            MTOSD_SC_Scale( 2, 1, 960, 540, panel_width, panel_height );
            MTOSD_PLA_FlipTo( 2, u4_region_list2 );
            MTOSD_PLA_Enable( 2, 1 );
            break;
        case 3:
            if (! (i % 2))
            {
                MTOSD_SC_Scale( 2, 1, 1920, 1080, panel_width, panel_height );
                MTOSD_PLA_FlipTo( 2, u4_region_list );
                MTOSD_PLA_Enable( 2, 1 );
            } else {
                MTOSD_SC_Scale( 2, 1, 960, 540, panel_width, panel_height );
                MTOSD_PLA_FlipTo( 2, u4_region_list2 );
                MTOSD_PLA_Enable( 2, 1 );
            }
            break;
        default:
            printf(" !ERROR");
            break;
        }
        MTOSD_BASE_Wait_Vsync(1);
    }
    return 0;
}
