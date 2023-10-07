#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <direct/messages.h>

#include <directfb.h>
#include <directfb_util.h>

#define WIDTH 1360
#define HEIGHT 768
/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          if (x != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, x );                         \
          }                                                           \
     }

/* the super interface */
static IDirectFB *dfb;

/* the primary surface (surface of primary layer) */
static IDirectFBSurface *primary;
static IDirectFBSurface *logo;
static IDirectFBFont           *font;

int main( int argc, char *argv[] )
{
    IDirectFBDisplayLayer  *layer;
    DFBSurfaceDescription desc;
    IDirectFBImageProvider *provider;
    
    DFBResult              ret;
    
    const char               *url      = NULL;
    
    /* Parse arguments. */
     url = argv[1];
    
    printf("the image name is %s\n", url);


    DFBCHECK(DirectFBInit( &argc, &argv ));
    DFBCHECK(DirectFBCreate( &dfb ));
    
    /* Request fullscreen mode. */
    dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

    /* Fill the surface description. */
    desc.flags         = DSDESC_CAPS;
    desc.caps         = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
    
    /* Create the primary surface. */
    DFBCHECK(dfb->CreateSurface( dfb, &desc, &primary ));

    if(url!=NULL)
    {
        DFBCHECK(dfb->CreateImageProvider( dfb, url, &provider ));
        
        DFBCHECK(provider->GetSurfaceDescription( provider, &desc ));

        desc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
        desc.width = WIDTH;
        desc.height = HEIGHT;
        desc.caps = DSCAPS_VIDEOONLY;
        desc.pixelformat = DSPF_ARGB;

        DFBCHECK(dfb->CreateSurface( dfb, &desc, &logo ));
        
        DFBCHECK(provider->RenderTo( provider, logo, NULL ));
        
        provider->Release( provider );
    }
    else
    {
        desc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS| DSDESC_PIXELFORMAT;
        desc.width = WIDTH;
        desc.height = HEIGHT;
        desc.caps = DSCAPS_VIDEOONLY ;
        desc.pixelformat = DSPF_ARGB4444;

        DFBCHECK(dfb->CreateSurface( dfb, &desc, &logo ));

        logo->Clear( logo, 0 , 0x88, 0, 0xff );
    }

    {
              DFBFontDescription desc;
    
              desc.flags = DFDESC_HEIGHT;
              desc.height = 50;
    
              DFBCHECK(dfb->CreateFont( dfb, FONT, &desc, &font ));
    }
    logo->SetDrawingFlags( logo, DSDRAW_BLEND );

     DFBCHECK(logo->SetFont( logo, font ));
          //logo->SetPorterDuff( logo, DSPD_SRC_OVER );
          logo->SetColor( logo, 0xCF, 0xCF, 0xFF, 0xFF );
          logo->DrawString( logo,
                                 "Move the mouse over a window to activate it.",
                                 -1, 0, 0, DSTF_LEFT | DSTF_TOP );



    
    primary->SetBlittingFlags( primary, DSBLIT_BLEND_ALPHACHANNEL );
    primary->Blit( primary, logo, NULL, 0, 0 );


    primary->Flip( primary, NULL, 0);

   //sleep( 25 );
   while(1)
       {

       }

   logo->Release( logo );
   primary->Release( primary );
   dfb->Release(dfb);
}

