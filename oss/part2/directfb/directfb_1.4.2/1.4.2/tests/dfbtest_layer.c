/*
   (c) Copyright 2008  Denis Oliver Kropp

   All rights reserved.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <direct/messages.h>

#include <directfb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#define API_TRACE(X)\
    do{\
        printf("\33[0;33;44m%s, %d, "#X" \33[0m\n", __FUNCTION__, __LINE__);\
        ret = X;\
        if ( ret != DFB_OK ) {\
            int result = fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
            if (result < 0) \
            { \
                printf("%s <%d>:\n\t", __FUNCTION__, __LINE__); \
            } \
            DirectFBErrorFatal( #X, ret );                             \
        }\
    }while(0)


typedef DFBResult (*TestFunc)( IDirectFBDisplayLayer *layer, void *arg );

typedef struct {
     const char *name;
     TestFunc    func;

} Test;

typedef enum
{
    CMD_UNKNOWN = 0,
    CMD_TEST_ITEM,
    CMD_FLIP_MODE,
    CMD_BUFFER_MODE,
    CMD_PIXEL_FORMAT,
    CMD_LAYER_SIZE,
    CMD_LAYER_ID,
    CMD_WINDOW_NUM,
}E_COMMAND_TYPE;




static int test_item = 0;
static DFBSurfaceFlipFlags flip_mode = DSFLIP_NONE;
static DFBDisplayLayerBufferMode layer_buff_mode = DLBM_FRONTONLY;
static DFBSurfacePixelFormat surf_pix_format = DSPF_ARGB;
static bool bDump = false;

static int layer_width = 1280;
static int layer_height = 720;

static int layer_id = 0;

static int window_num = 2;
const char *semName = "DFBSem";
char pitch_size[10];
char dmabuf_id_str[10];
static int pitch;
static u32 dmabuf_id;

static DFBResult
RunTest( TestFunc               func,
         const char            *test_name,
         IDirectFB             *dfb,
         void                  *arg )
{
     DFBResult ret;

     /* Run the actual test... */
     ret = func( dfb, arg );
     if (ret)
          D_DERROR( ret, "RunTest: '%s' failed!\n", test_name );

     return ret;
}

static DFBResult simple_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;
    int w, h, i;

    for(i=0; i< 3; i++) {
        ret = dfb->GetScreen( dfb, i, &screen );
        if (ret) {
              D_DERROR( ret, "Tools/Screen: IDirectFB::GetScreen() failed!\n" );
        }
        screen->GetSize(screen, &w, &h);
        printf("%s, screen %d, size (%d, %d)\n", __FUNCTION__, i, w, h);
    }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = layer_width;
    layer_config.height = layer_height;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 0, 640, 360);

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 360, 640, 360);
    printf("%s, surface flip!\n", __FUNCTION__);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "test");

    sleep(3);
    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

static DFBResult change_layer_size( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 0, 640, 360);

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 360, 640, 360);
    printf("%s, flip surface!\n", __FUNCTION__);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "change_size_1");

    sleep(3);

    // change layer size.
    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

    // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1920;
    layer_config.height = 1080;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));
    printf("%s, %d, change size to 1920x1080!\n", __FUNCTION__, __LINE__);
    layer->GetSurface(layer, &primary);

    //layer->SetHVScale(layer, 1920, 1080);
    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 360, 640, 360);
    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 0, 640, 360);
    printf("%s, flip surface!\n", __FUNCTION__);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "change_size_2");

    sleep(5);
    sleep(5);
    // release.
    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult multi_layers( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;

    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBDisplayLayer *layer1 = NULL;
    IDirectFBDisplayLayer *layer2 = NULL;

    IDirectFBSurface     *primary = NULL;
    IDirectFBSurface     *layer1_surf = NULL;
    IDirectFBSurface     *layer2_surf = NULL;

    DFBDisplayLayerConfig   layer_config;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    // set layer cap.
    layer_config.flags = ( DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = DSPF_ARGB;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    printf("%s, %d, fill Rect, flip!\n", __FUNCTION__, __LINE__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0x7f );
    primary->FillRectangle( primary, 0, 0, 640, 360);
    printf("%s, %d, flip!\n", __FUNCTION__, __LINE__);
    primary->Flip( primary, NULL, flip_mode );
    if (bDump)
        primary->Dump( primary, "./", "mulit_layer_0");

    sleep(1);

    printf("%s, %d, render layer 1\n", __FUNCTION__, __LINE__);
    /* get the second layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 1, &layer1 );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }
    layer1->SetCooperativeLevel( layer1, DLSCL_ADMINISTRATIVE );

    // set layer cap.
    layer_config.flags = ( DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_OPTIONS);

    layer_config.pixelformat = DSPF_ARGB;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;
    layer_config.options = DLOP_ALPHACHANNEL;

    API_TRACE(layer1->SetConfiguration( layer1, &layer_config ));

    layer1->GetSurface(layer1, &layer1_surf);

    printf("%s, %d, fill Rect, flip!\n", __FUNCTION__, __LINE__);
    layer1_surf->SetColor( layer1_surf, 0xff, 0x0, 0xff, 0x7f );
    layer1_surf->FillRectangle( layer1_surf, 320, 180, 640, 360);
    printf("%s, %d, flip!\n", __FUNCTION__, __LINE__);
    layer1_surf->Flip( layer1_surf, NULL, flip_mode );
    if (bDump)
        layer1_surf->Dump( layer1_surf, "./", "mulit_layer_1");

    sleep(1);

    /* get the second layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 2, &layer2 );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer2->SetCooperativeLevel( layer2, DLSCL_ADMINISTRATIVE ) );

    // set layer cap.
    layer_config.flags = ( DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT  | DLCONF_OPTIONS);

    layer_config.pixelformat = DSPF_ARGB;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;
    layer_config.options = DLOP_ALPHACHANNEL;

    API_TRACE(layer2->SetConfiguration( layer2, &layer_config ));

    layer2->GetSurface(layer2, &layer2_surf);

    printf("%s, fill Rect, flip!\n", __FUNCTION__);
    layer2_surf->SetColor( layer2_surf, 0x0, 0xff, 0x0, 0x7f );
    layer2_surf->FillRectangle( layer2_surf, 640, 360, 640, 360);
    printf("%s, %d, flip!\n", __FUNCTION__, __LINE__);
    layer2_surf->Flip( layer2_surf, NULL, flip_mode );
    if (bDump)
        layer2_surf->Dump( layer2_surf, "./", "mulit_layer_2");

    sleep(30);

    // release.
    layer2_surf->Release( layer2_surf );
    layer1_surf->Release( layer1_surf );
    primary->Release( primary );

    /* Release the layer. */
    layer2->Release( layer2 );
    layer1->Release( layer1 );
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult buffer_mode_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;
    int i = 0;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_TRIPLE; //DLBM_FRONTONLY; //DLBM_BACKVIDEO, DLBM_BACKSYSTEM, DLBM_TRIPLE
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    for( i=0; i<3; i++) {
        printf("%s, flip 0!\n", __FUNCTION__);
        primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
        primary->Flip( primary, NULL, flip_mode );
        sleep(1);

        printf("%s, flip 1!\n", __FUNCTION__);
        primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
        primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
        primary->FillRectangle( primary, 0, 0, 640, 360);
        primary->Flip( primary, NULL, flip_mode );
        sleep(1);

        printf("%s, flip 2!\n", __FUNCTION__);
        primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
        primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
        primary->FillRectangle( primary, 640, 360, 640, 360);
        primary->Flip( primary, NULL, flip_mode );
        sleep(1);

        printf("%s, flip 3!\n", __FUNCTION__);
        primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
        primary->SetColor( primary, 0x0, 0x0, 0xff, 0xff );
        primary->FillRectangle( primary, 320, 180, 640, 360);
        primary->Flip( primary, NULL, flip_mode );
    }

    if (bDump)
        primary->Dump( primary, "./", "test");

    sleep(3);
    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult surface_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBSurface     *primary = NULL;
    DFBSurfaceDescription desc;

    printf("%s, start!\n", __FUNCTION__);

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS);
    desc.width  = 1280;
    desc.height = 720;
    desc.caps   = DSCAPS_VIDEOONLY | DSCAPS_PRIMARY;

    dfb->CreateSurface( dfb, &desc, &primary);

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, fill Rect, flip!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 0, 640, 360);

    printf("%s, fill Rect, flip!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 360, 640, 360);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "test");

    sleep(10);
    primary->Release(primary);

    /* Release the layer. */
    //layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult blit_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;
    DFBSurfaceDescription desc;

    IDirectFBSurface     *surf = NULL;
    int i;
    int num = 10;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_FRONTONLY;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);


    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_WIDTH | DSDESC_HEIGHT);
    desc.width  = 500;
    desc.height = 500;

    API_TRACE( dfb->CreateSurface( dfb, &desc, &surf) );


    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    for(i=0; i< num; i++) {
        surf->SetColor( surf, (25 * i), 0x0, 0x0, 0xff );
        surf->FillRectangle( surf, 0, 0, 500, 500);

        printf("%s, blit, flip, i= %d!\n", __FUNCTION__, i);
        primary->Blit( primary, surf, NULL, 20*i, 20*i);

        printf("%s, blit, flip end\n", __FUNCTION__);
        sleep(1);
    }
    if (bDump)
        primary->Dump( primary, "./", "blit_test");

    sleep(10);
    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult HVScale_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;
    int w, h;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    /* keep original screen size. */
    layer->GetScreen(layer, &screen);
    screen->GetSize(screen, &w, &h);
    printf("%s, get screen size (%d, %d)\n", __FUNCTION__, w, h);

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 0, 640, 360);

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 360, 640, 360);

    printf("%s, surface flip!\n", __FUNCTION__);
    primary->Flip( primary, NULL, flip_mode );

    sleep(3);
    printf("%s, set HV Scale to (1280, 720)\n", __FUNCTION__);
    layer->SetHVScale(layer, 1280, 720);
    //primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );
    //primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    //primary->FillRectangle( primary, 320, 180, 640, 360);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "test");

    sleep(3);

    // reset to dst size.
    printf("%s, restore back to original screen size (%d, %d)\n", __FUNCTION__, w, h);
    layer->SetHVScale(layer, w, h);

    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult TTX_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;
    int w, h, i;

    for(i=0; i< 3; i++) {
        ret = dfb->GetScreen( dfb, i, &screen );
        if (ret) {
              D_DERROR( ret, "Tools/Screen: IDirectFB::GetScreen() failed!\n" );
        }
        screen->GetSize(screen, &w, &h);
        printf("%s, screen %d, size (%d, %d)\n", __FUNCTION__, i, w, h);
    }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    API_TRACE( layer->GetConfiguration( layer, &layer_config ) );

    // Set CC layer stretch mode to nearest, avoiding color mix
    DFBDisplayLayerStretchSetting stretchSetting;
    memset(&stretchSetting, 0, sizeof(DFBDisplayLayerStretchSetting));
    stretchSetting.flag = DLSM_V_MODE;
    stretchSetting.v_mode = DISPLAYER_VSTRCH_NEAREST;
    API_TRACE(layer->SetStretchMode( layer, stretchSetting ) );

     // set layer cap.
    layer_config.flags = ( DLCONF_OPTIONS | DLCONF_WIDTH | DLCONF_HEIGHT );

    layer_config.width = layer_width;
    layer_config.height = layer_height;
    layer_config.options =  (DFBDisplayLayerOptions)(DLOP_ALPHACHANNEL);

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0xff, 0x0, 0x0, 0xff );
    primary->FillRectangle( primary, 0, 0, 640, 360);

    printf("%s, fill Rect!\n", __FUNCTION__);
    primary->SetColor( primary, 0x0, 0xff, 0x0, 0xff );
    primary->FillRectangle( primary, 640, 360, 640, 360);
    printf("%s, surface flip!\n", __FUNCTION__);
    primary->Flip( primary, NULL, flip_mode );

    if (bDump)
        primary->Dump( primary, "./", "test");

    sleep(3);
    //primary->Clear( primary, 0, 0, 0, 0 );

    primary->Release(primary);

    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

static DFBResult layer_fps_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;

    IDirectFBDisplayLayer *layer = NULL;

    IDirectFBSurface     *primary = NULL;

    DFBDisplayLayerConfig   layer_config;

    int w = 0, h = 0, x = 0, y = 0;
    int       frames = 0;
    float     fps = 0;
    long long fps_time = 0;
    long long diff = 0;
    long long now = 0;
    int count = 20;
    unsigned char r = 0, g = 0, b = 0, a = 0;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, layer_id, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    // set layer cap.
    layer_config.flags = ( DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_OPTIONS);

    layer_config.pixelformat = DSPF_ARGB;
    layer_config.buffermode = DLBM_TRIPLE;//DLBM_BACKVIDEO;
    layer_config.width = 1280;
    layer_config.height = 720;
    layer_config.options = DLOP_ALPHACHANNEL;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);
    //printf("%s, call SetHVMirrorEnable\n", __FUNCTION__);
    //layer->SetHVMirrorEnable(layer, false, true);
    if (layer_id == 0) {
        r = 0xff; g = 0; b = 0; a = 0x7f;
        x = 0; y = 0; w = 640; h = 360;
    }
    else {
        r = 0xff; g = 0; b = 0xff; a = 0x7f;
        x = 320; y = 180; w = 640; h = 360;
    }

    fps_time = direct_clock_get_millis();

    while(count > 0) {
        primary->SetColor( primary, r, g, b, a );
        primary->FillRectangle( primary, x, y, w, h );
        primary->Flip( primary, NULL, flip_mode );

        now = direct_clock_get_millis();
        frames++;
        diff = now - fps_time;
        if (diff >= 1000) {
            fps = frames * 1000 / (float) diff;

            printf( "layer %d, fps : %.1f\n", layer_id, fps );

            fps_time = now;
            frames   = 0;
            count--;
        }
    }

    // release.
    primary->Release( primary );

    /* Release the layer. */
    layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

void dump_raw(IDirectFBSurface *surf, const char *directory, const char *prefix )
{
     int                    len = (directory ? strlen(directory) : 0) + (prefix ? strlen(prefix) : 0) + 40;
     char                   filename[len];
     static int num = 0;
     void *data;
     int   pitch;

     if (snprintf( filename, len, "%s/%s_%04d.raw", directory, prefix, num++ ) < 0) {
         printf("%s, can't create filename, snprintf failed!\n", __FUNCTION__);
         return;
     }

     int fd_p = open( filename, O_EXCL | O_CREAT | O_WRONLY, 0644 );
     if (fd_p < 0) {
         printf("%s, open file (%s) failed\n", __FUNCTION__, filename);
         return;
     }

     if (surf->Lock(surf, DSLF_READ, &data, &pitch) !=DFB_OK) {
         printf("%s, lock surface failed!\n", __FUNCTION__);
         close(fd_p);
         return;
     }

    write(fd_p, (u8 *)data, layer_width * (layer_height+100) * 4);

    close(fd_p);

    surf->Unlock(surf);
}

static DFBResult window_flip( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow  *window0 = NULL;
    IDirectFBWindow  *window1 = NULL;
    IDirectFBWindow  *window2 = NULL;
    IDirectFBSurface *surface0 = NULL;
    IDirectFBSurface *surface1 = NULL;
    IDirectFBSurface *surface2 = NULL;
    IDirectFBSurface     *primary = NULL;

    DFBDisplayLayerConfig   layer_config;

    DFBWindowDescription m_desc_top = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS ,
         .posx          = 0,
         .posy          = 0,
         .width         = layer_width,
         .height        = layer_height,
         .caps          = DWCAPS_ALPHACHANNEL,
    };

    DFBWindowDescription m_desc_sub = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS ,
         .posx          = 0,
         .posy          = 0,
         .width         = layer_width,
         .height        = layer_height,
         .caps          = DWCAPS_ALPHACHANNEL,
    };

    int i = 0;

    int       frames = 0;
    float     fps = 0;
    long long start = 0;
    long long diff = 0;
    long long end = 0;
    int count = 3;

    /* Get the primary layer interface. */
    //layer_id = 1;
    ret = dfb->GetDisplayLayer( dfb, layer_id, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT  | DLCONF_OPTIONS);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = layer_width;
    layer_config.height = layer_height;
    layer_config.options = DLOP_ALPHACHANNEL;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));
    
    //if ( flip_mode != DSFLIP_BLIT )
        //layer->ForceFullUpdateWindowStack(layer, true);

    layer->GetSurface(layer, &primary);

    API_TRACE( layer->CreateWindow( layer, &m_desc_top, &window0 )) ;
    API_TRACE( window0->GetSurface( window0, &surface0 ));
    API_TRACE( window0->SetOpacity( window0, 0xFF ) );
    
    if (window_num > 1) {
        API_TRACE( layer->CreateWindow( layer, &m_desc_top, &window1 )) ;
        API_TRACE( window1->GetSurface( window1, &surface1 ));
        API_TRACE( window1->SetOpacity( window1, 0xFF ) );
        
    }

    if (window_num > 2) {
        API_TRACE( layer->CreateWindow( layer, &m_desc_top, &window2 )) ;
        API_TRACE( window2->GetSurface( window2, &surface2 ));
        API_TRACE( window2->SetOpacity( window2, 0xFF ) );
    }
    
    while(count > 0)
    {
        surface0->SetColor( surface0, 0xFF, (0xff/count), 0, 0xFF );
        surface0->FillRectangle( surface0, 0, 0, m_desc_top.width/2, m_desc_top.height/2 );
	surface0->Flip(surface0, NULL, flip_mode );
	printf("[%s] surface0 flip!, count=%d\n", __FUNCTION__, count);
        sleep(1);

        if (window_num > 1) {
	    surface1->SetColor( surface1,  0, 0xFF,  (0xff/count), 0xFF );
            surface1->FillRectangle( surface1, m_desc_top.width/4,  m_desc_top.height/4, m_desc_top.width/2, m_desc_top.height/2 );
            surface1->Flip(surface1, NULL, flip_mode );
  	    printf("[%s] surface1 flip!, count=%d\n", __FUNCTION__, count);
	    sleep(1);
        }

        if (window_num > 2) {
            surface2->SetColor( surface2,  (0xff/count),  0, 0xFF, 0xFF );
            surface2->FillRectangle( surface2, m_desc_top.width/2,  m_desc_top.height/2, m_desc_top.width/2, m_desc_top.height/2  );
            surface2->Flip(surface2, NULL, flip_mode );
 	    printf("[%s] surface2 flip!, count=%d\n", __FUNCTION__, count);
	    sleep(1);
        }

        count--;
    }
    
    if (bDump) {
        primary->Dump( primary, "./", "layer_surface");
        //dump_raw(primary, "./", "layer_surface");
	surface0->Dump( surface0, "./", "window0_surface");
	if (window_num > 2) {
	    surface2->Dump(surface2, "./", "window2_surface");
	}
	if (window_num > 1) {
	    surface1->Dump(surface1, "./", "window1_surface");
	}
    }

    
    if (window_num > 2){
        surface2->Release( surface2 );
        window2->Release( window2 );
    }

    if (window_num > 1) {
        surface1->Release( surface1);
        window1->Release( window1);
    }

    surface0->Release( surface0);
    window0->Release( window0);

    primary->Release ( primary );
    
    layer->Release( layer );

    return ret;
}

static DFBResult window_rotate( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow  *window = NULL;
    
    IDirectFBSurface *surface = NULL;
    
    IDirectFBSurface     *primary = NULL;

    DFBDisplayLayerConfig   layer_config;

    DFBWindowDescription m_desc_top = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT,
         .posx          = 100,
         .posy          = 100,
         .width         = 600,
         .height        = 400,
    };

    int i = 0;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, layer_id, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_BACKVIDEO;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    //if ( flip_mode != DSFLIP_BLIT )
        //layer->ForceFullUpdateWindowStack(layer, true);

    layer->GetSurface(layer, &primary);
    primary->Clear(primary, 0, 0, 0, 0);

    API_TRACE(layer->CreateWindow( layer, &m_desc_top, &window ));
    
    API_TRACE(window->GetSurface( window, &surface ));
    
    API_TRACE(window->SetOpacity( window, 0xff ));
    
    surface->SetColor( surface, 0, 0xF0, 0xc0, 0xFF );
    
    surface->FillRectangle( surface, 0, 0, m_desc_top.width, m_desc_top.height );
       
    surface->Flip( surface, NULL, flip_mode );
    
    sleep(3);

    API_TRACE(window->SetRotation( window, 90));
    surface->SetColor( surface, 0xFF, 0x0, 0x0, 0xFF );
    surface->FillRectangle( surface, 0, 0, m_desc_top.width, m_desc_top.height );
    surface->Flip( surface, NULL, flip_mode );
    
    sleep(3);

    surface->Release( surface );
    primary->Release ( primary );
    window->Release( window );
    layer->Release( layer );

    return ret;
}

static DFBResult window_auto_fit( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBWindow  *window = NULL;
    IDirectFBSurface *surface = NULL;
    DFBDisplayLayerConfig   layer_config, layer_origin_config;

    DFBWindowDescription m_desc_top = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT| DWDESC_OPTIONS,
         .posx          = 0,
         .posy          = 0,
         .width         = 1280,
         .height        = 720,
         .options       = DWOP_SCALE_WINDOW_AUTOFIT_LAYER| DWOP_SCALE
    };

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, layer_id, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    layer->GetConfiguration( layer, &layer_origin_config );
    printf("%s, get origin layer size (%d, %d)\n", __FUNCTION__, layer_origin_config.width, layer_origin_config.height);

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    printf("%s, set layer to (1280, 720)\n", __FUNCTION__);
     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_FRONTONLY;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    API_TRACE(layer->CreateWindow( layer, &m_desc_top, &window ));

    API_TRACE(window->GetSurface( window, &surface ));

    surface->Clear(surface, 0xff, 0xff, 0xff, 0xff);

    printf("%s, fill Rect!\n", __FUNCTION__);
    surface->SetColor( surface, 0xff, 0x0, 0x0, 0xff );
    surface->FillRectangle( surface, 0, 0, m_desc_top.width/2, m_desc_top.height/2 );

    printf("%s, fill Rect!\n", __FUNCTION__);
    surface->SetColor( surface, 0x0, 0xff, 0x0, 0xff );
    surface->FillRectangle( surface, m_desc_top.width/2, m_desc_top.height/2, m_desc_top.width/2, m_desc_top.height/2);

    printf("%s, surface flip!\n", __FUNCTION__);
    surface->Flip( surface, NULL, flip_mode );
    API_TRACE(window->SetOpacity( window, 0xff ));

    sleep(3);
    API_TRACE(window->SetOpacity( window, 0x00 ));

    printf("%s, set layer to (1920, 1080)\n", __FUNCTION__);
    layer_config.width = 1920;
    layer_config.height = 1080;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));
    API_TRACE(window->SetOpacity( window, 0xff ));

    sleep(3);
    API_TRACE(window->SetOpacity( window, 0x00 ));

    printf("%s, restore layer back to (%d,  %d)\n", __FUNCTION__,layer_origin_config.width, layer_origin_config.height);
    API_TRACE(layer->SetConfiguration( layer, &layer_origin_config ));

    surface->Release( surface );
    window->Release( window );
    layer->Release( layer );

    return ret;
}

static DFBResult screen_scale_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBSurface *surface = NULL;
    DFBDisplayLayerConfig   layer_config, layer_origin_config;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, layer_id, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    layer->GetConfiguration( layer, &layer_origin_config );
    printf("%s, get origin layer size (%d, %d)\n", __FUNCTION__, layer_origin_config.width, layer_origin_config.height);

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    printf("%s, set layer to (1280, 720)\n", __FUNCTION__);
     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_FRONTONLY;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    API_TRACE(layer->GetSurface( layer, &surface ));

    surface->Clear(surface, 0xff, 0xff, 0xff, 0xff);

    /* row 1 */

    surface->SetColor( surface, 0xff, 0x0, 0x0, 0xff );
    surface->FillRectangle( surface, 0, 0, layer_config.width/3, layer_config.height/3 );

    surface->SetColor( surface, 0x0, 0xff, 0x0, 0xff );
    surface->FillRectangle( surface, layer_config.width/3 + 1, 0, layer_config.width/3, layer_config.height/3);

    surface->SetColor( surface, 0x0, 0x0, 0xff, 0xff );
    surface->FillRectangle( surface, (layer_config.width/3)*2 + 1, 0, layer_config.width/3, layer_config.height/3);

    /* row 2 */

    surface->SetColor( surface, 0xff, 0xff, 0x0, 0xff );
    surface->FillRectangle( surface, 0, layer_config.height/3, layer_config.width/3, layer_config.height/3 );

    surface->SetColor( surface, 0x0, 0xff, 0xff, 0xff );
    surface->FillRectangle( surface, layer_config.width/3 + 1, layer_config.height/3 + 1, layer_config.width/3, layer_config.height/3);

    surface->SetColor( surface, 0xff, 0x0, 0xff, 0xff );
    surface->FillRectangle( surface, (layer_config.width/3)*2 + 1, layer_config.height/3 + 1, layer_config.width/3, layer_config.height/3);

    /* row 3 */

    surface->SetColor( surface, 0x11, 0x33, 0x55, 0xff );
    surface->FillRectangle( surface, 0, (layer_config.height/3)*2, layer_config.width/3, layer_config.height/3 );

    surface->SetColor( surface, 0x77, 0x66, 0x55, 0xff );
    surface->FillRectangle( surface, layer_config.width/3, (layer_config.height/3)*2 + 1, layer_config.width/3, layer_config.height/3);

    surface->SetColor( surface, 0x0, 0x0, 0x0, 0xff );
    surface->FillRectangle( surface, (layer_config.width/3)*2, (layer_config.height/3)*2 + 1, layer_config.width/3 +1, layer_config.height/3);

    API_TRACE(surface->Flip( surface, NULL, flip_mode ));
    sleep(3);

    API_TRACE(DirectFBEnableZoommode (0, 0, 0, 500, 500));

    API_TRACE(surface->Flip( surface, NULL, flip_mode ));
    sleep(3);

    API_TRACE(DirectFBEnableZoommode (0, 0, 0, layer_config.width, layer_config.height));

    API_TRACE(surface->Flip( surface, NULL, flip_mode ));
    sleep(3);

    printf("%s, restore layer back to (%d,  %d)\n", __FUNCTION__,layer_origin_config.width, layer_origin_config.height);
    API_TRACE(layer->SetConfiguration( layer, &layer_origin_config ));

    surface->Release( surface );
    layer->Release( layer );

    return ret;
}

static DFBResult preallocate_surface_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    IDirectFBSurface     *tmp_surf = NULL;
    IDirectFBSurface     *tmp_surf1 = NULL;
    DFBSurfaceDescription desc;
    void *data;
    u64 u64phy;
    int pitch;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }
    
    layer->GetSurface(layer, &primary);

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS);
    desc.pixelformat = DSPF_ARGB;
    desc.width  = 500;
    desc.height = 500;
    desc.caps   = DSCAPS_VIDEOONLY;

    API_TRACE(dfb->CreateSurface( dfb, &desc, &tmp_surf));

    tmp_surf->Clear( tmp_surf, 0xff, 0, 0, 0xff );

    API_TRACE(tmp_surf->Lock3(tmp_surf, DSLF_READ, &data, &u64phy, &pitch));
    tmp_surf->Unlock( tmp_surf);

    printf("%s, Lock3 return va: %p, pa: %llx, pitch: %d\n", __FUNCTION__, data, u64phy, pitch);

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED |
                        DSDESC_PREALLOCATED_IN_VIDEO | DSDESC_PREALLOCATED_IN_VIDEO_NO_VA | DSDESC_PREALLOCATED_IN_VIDEO_64BIT_ADDR );
    desc.pixelformat = DSPF_ARGB;
    desc.width  = 500;
    desc.height = 500;
    desc.preallocated[0].data = (u32)(u64phy  & 0x00000000FFFFFFFF);
    desc.preallocated[1].data = (u32)(u64phy >> 32);
    desc.preallocated[0].pitch = pitch;

    printf("preallocated[0].data : %x, preallocated[1].data : %x\n", desc.preallocated[0].data, desc.preallocated[1].data);
    API_TRACE(dfb->CreateSurface( dfb, &desc, &tmp_surf1));

    printf("%s, clear, !\n", __FUNCTION__);
    primary->Clear( primary, 0xff, 0xff, 0xff, 0xff );

    printf("%s, blit tmp_surf, !\n", __FUNCTION__);
    API_TRACE(primary->Blit( primary, tmp_surf, NULL, 50, 50));

    printf("%s, blit tmp_surf1, flip!\n", __FUNCTION__);
    API_TRACE(primary->Blit( primary, tmp_surf1, NULL, 600, 50));

    primary->Flip( primary, NULL, flip_mode );


    if (bDump) {		
	 tmp_surf->Dump( tmp_surf, "./", "tmp_surf");
        primary->Dump( primary, "./", "primary");
    }

    sleep(10);
    tmp_surf->Release(tmp_surf);
    tmp_surf1->Release(tmp_surf1);
    primary->Release(primary);

    /* Release the layer. */
    //layer->Release( layer );

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult preallocate_surface_create_by_dmabuf_id( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    IDirectFBSurface     *tmp_surf = NULL;
    DFBSurfaceDescription desc;
    DFBDisplayLayerConfig   layer_config;
    void *data;
    u64 u64phy;

    printf("%s, start!\n", __FUNCTION__);

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 0, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer_config.flags = (DLCONF_PIXELFORMAT |DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = DLBM_FRONTONLY;
    layer_config.width = 1280;
    layer_config.height = 720;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &primary);

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS);
    desc.pixelformat = DSPF_ARGB;
    desc.width  = 500;
    desc.height = 500;
    desc.caps   = DSCAPS_VIDEOONLY;

    API_TRACE(dfb->CreateSurface( dfb, &desc, &tmp_surf));
    API_TRACE(tmp_surf->Lock3(tmp_surf, DSLF_READ, &data, &u64phy, &pitch));
    tmp_surf->Unlock( tmp_surf);

    printf("%s : Please input dmabuf_id = ",__FUNCTION__);
    scanf("%d", &dmabuf_id);
    API_TRACE(tmp_surf->Clear( tmp_surf, 0xff, 0, 0, 0xff ));

    API_TRACE(primary->Clear( primary, 0xff, 0xff, 0xff, 0xff ));
    API_TRACE(primary->Blit( primary, tmp_surf, NULL, 50, 50));
    API_TRACE(primary->Flip( primary, NULL, flip_mode ));

    sleep(3);

    if(snprintf(pitch_size, sizeof(pitch_size), "%d", pitch) < 0)
    {
        printf("[DFB] %s(%d) snprintf failed with error [%s]\n",__FUNCTION__,__LINE__,strerror(errno));
        return DFB_FAILURE;
    }
    if(snprintf(dmabuf_id_str, sizeof(dmabuf_id_str), "%d", dmabuf_id) < 0)
    {
        printf("[DFB] %s(%d) snprintf failed with error [%s]\n",__FUNCTION__,__LINE__,strerror(errno));
        return DFB_FAILURE;
    }

#if DFB_SUPPORT_AN
    const char *argv[] = {"/mnt/vendor/linux_rootfs/lib/ld-linux.so.3", "./dfbtest_layer", "-test", "15", pitch_size, dmabuf_id_str, NULL};
#else
    const char *argv[] = {"./dfbtest_layer", "-test", "15", pitch_size, dmabuf_id_str, NULL};
#endif

    if(fork()==0){ /*child process*/
#if DFB_SUPPORT_AN
                ret = execvp("/mnt/vendor/linux_rootfs/lib/ld-linux.so.3", argv);
#else
                ret = execvp("./dfbtest_layer", argv);
#endif
                if(ret == -1)
                    perror("execl error");
    }
    else
    {
        sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 0);
        if (sem_id == SEM_FAILED)
        {
            perror("Parent  : [sem_open] Failed\n");
        }

        if (sem_wait(sem_id) < 0)
        {
            printf("Parent  : [sem_wait] Failed\n");
        }
        if (sem_close(sem_id) != 0)
        {
            perror("Parent  : [sem_close] Failed\n"); 
        }
        if (sem_unlink(semName) < 0){
            printf("Parent  : [sem_unlink] Failed\n");
        }

        tmp_surf->Release(tmp_surf);
        primary->Release(primary);
        layer->Release(layer);
    }

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult preallocate_surface_create_by_dmabuf_id_child( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBWindow  *window_normal = NULL;
    IDirectFBWindow  *window_secure = NULL;
    IDirectFBSurface   *surf_normal = NULL;
    IDirectFBSurface   *surf_secure = NULL;
    IDirectFBSurface     *tmp_surf1 = NULL;
    DFBSurfaceDescription desc;
    DFBDisplayLayerConfig   layer_config;

    DFBWindowDescription m_desc_normal = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS,
         .posx          = 0,
         .posy          = 0,
         .width         = 1280,
         .height        = 720,
         .caps          = DWCAPS_ALPHACHANNEL,
    };

    DFBWindowDescription m_desc_secure = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_SURFACE_CAPS,
         .posx          = 0,
         .posy          = 0,
         .width         = 1280,
         .height        = 720,
         .surface_caps = DSCAPS_SECURE_MODE,
         .caps          = DWCAPS_ALPHACHANNEL,
    };

    printf("%s, start!\n", __FUNCTION__);

     sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 0);
     if (sem_id == SEM_FAILED){
         perror("Child   : [sem_open] Failed\n");
         return DFB_FAILURE;
     }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 0, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }
    API_TRACE( layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED |
                        DSDESC_PREALLOCATED_IN_VIDEO | DSDESC_PREALLOCATED_IN_VIDEO_NO_VA | DSDESC_PREALLOCATED_IN_VIDEO_64BIT_ADDR );
    desc.pixelformat = DSPF_ARGB;
    desc.width  = 500;
    desc.height = 500;
    desc.preallocated[0].data = dmabuf_id;
    desc.preallocated[1].data = 0;
    desc.preallocated[0].pitch = pitch;
    printf("preallocated[0].data : %p, preallocated[1].data : %p\n", desc.preallocated[0].data, desc.preallocated[1].data);

    printf("[%s] Normal window..\n", __FUNCTION__);

    layer_config.flags = (DLCONF_SURFACE_CAPS | DLCONF_BUFFERMODE);
    layer_config.buffermode = DLBM_TRIPLE;
    layer_config.surface_caps = DSCAPS_SECURE_MODE;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->CreateWindow( layer, &m_desc_normal, &window_normal );
    window_normal->GetSurface( window_normal, &surf_normal );

    API_TRACE(dfb->CreateSurface( dfb, &desc, &tmp_surf1));
    API_TRACE(surf_normal->Clear( surf_normal, 0xaa, 0xbb, 0xcc, 0xff ));
    API_TRACE(surf_normal->Blit( surf_normal, tmp_surf1, NULL, 0, 0));
    API_TRACE(surf_normal->Flip( surf_normal, NULL, flip_mode ));
    API_TRACE(window_normal->SetOpacity( window_normal, 0xFF ) );
    sleep(3);

    printf("[%s] Secure window..\n", __FUNCTION__);

    layer->CreateWindow( layer, &m_desc_secure, &window_secure );
    window_secure->GetSurface( window_secure, &surf_secure );

    API_TRACE(surf_secure->Clear( surf_secure, 0xaa, 0xbb, 0xcc, 0xff ));
    API_TRACE(surf_secure->Blit( surf_secure, tmp_surf1, NULL, 600, 0));
    API_TRACE(surf_secure->Flip( surf_secure, NULL, flip_mode ));
    API_TRACE(window_secure->SetOpacity( window_secure, 0xFF ) );
    sleep(3);

    API_TRACE(layer->GetConfiguration( layer, &layer_config ));
    layer_config.flags = DLCONF_SURFACE_CAPS;
    layer_config.surface_caps &= ~DSCAPS_SECURE_MODE;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    tmp_surf1->Release(tmp_surf1);
    surf_normal->Release(surf_normal);
    surf_secure->Release(surf_secure);
    window_normal->Release(window_normal);
    window_secure->Release(window_secure);
    layer->Release(layer);

    if (sem_post(sem_id) < 0)
        printf("Child   : [sem_post] Failed \n");

    printf("%s, end!\n", __FUNCTION__);

    return ret;
}

static DFBResult cursor_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBWindow  *window = NULL;
    IDirectFBSurface     *primary = NULL;
    IDirectFBSurface     *cursor = NULL;
    IDirectFBEventBuffer *events;
    DFBDisplayLayerConfig   layer_config;
    DFBInputEvent evt;
    DFBSurfaceDescription desc;
    int x, y;
    bool exit = false;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = layer_width;
    layer_config.height = layer_height;

    layer->SetConfiguration( layer, &layer_config );

    DFBWindowDescription m_desc_top = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_PIXELFORMAT,
         .posx          = 0,
         .posy          = 0,
         .width         = layer_width,
         .height        = layer_height,
         .pixelformat = surf_pix_format,
    };

    layer->CreateWindow( layer, &m_desc_top, &window );
    window->SetOpacity( window, 0xff );
    window->GetSurface(window, &primary);
    primary->Clear(primary, 0, 0, 0, 0xff);
    primary->Flip( primary, NULL, flip_mode );

    desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS);
    desc.pixelformat = DSPF_ARGB;
    desc.width  = 50;
    desc.height = 50;
    desc.caps   = DSCAPS_VIDEOONLY;

    dfb->CreateSurface( dfb, &desc, &cursor);
    cursor->Clear( cursor, 0xff, 0, 0, 0xff );

    //DirectFBSetOption( "cursor", NULL );
    API_TRACE(layer->EnableCursor( layer, true ));
    API_TRACE(layer->SetCursorShape( layer, cursor, 0, 0 ));
    API_TRACE(layer->SetCursorOpacity( layer, 0xff ));

    /* create an event buffer for all devices */
    API_TRACE(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL, DFB_FALSE, &events ));

     while (!exit) {
          while (events->GetEvent( events, DFB_EVENT(&evt) ) == DFB_OK) {
              if (evt.type == DIET_AXISMOTION) {
                  layer->GetCursorPosition(layer, &x,&y );
                  printf("[%s] x= %d, y = %d\n", __FUNCTION__,x, y);
              }
              break;
         }
         events->WaitForEvent( events );
   }

    primary->Release(primary);
    cursor->Release(cursor);
    events->Release(events);
    window->Release(window);
    layer->Release( layer );

    return ret;
}


unsigned int map[1920*1080*4];
unsigned int map2[256*128*4];
const char abgr2101010[] = "abgr2101010_1920x1080.bin";
const char a2rgb101010[] = "Orange_Cat_256x128_a2rgb10.bin";

static DFBResult HDR_test( IDirectFB *dfb, void *arg )
{

    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBSurface     *primary = NULL;
    DFBDisplayLayerConfig   layer_config;

    void *data = NULL;
    u64 u64phy;
    int pitch;
    FILE * fp = NULL;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        printf( "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

    printf("example 1: show 1920 x 1080 ABGR2101010 buffer\n");
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config.pixelformat = DSPF_ABGR2101010;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1920;
    layer_config.height = 1080;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));
    printf("%s, getsurface, !\n\n", __FUNCTION__);
    layer->GetSurface(layer, &primary);


    API_TRACE(primary->Lock3(primary, DSLF_READ, &data, &u64phy, &pitch));
    printf("%s, Lock3 return va: %p, pa: %llx, pitch: %d\n", __FUNCTION__, data, u64phy, pitch);


    unsigned int *temp = data;

    int n, ret2;
    int i, j;
    fp = fopen(abgr2101010, "rb");
    if (fp != NULL) {
        printf("open file %s PASS\n", abgr2101010);
        printf("reading A2BGR10 buffer...ing\n");
        ret2 = fread(map, 1920*1080*4, 1, fp);
        if(ret2 == 0) {
            ret2 = fclose(fp);
            if(ret2 != 0) {
                printf("fclose fail=%d\n", ret2);
                ret = DFB_FAILURE;
                return ret;
            }

            ret = DFB_FAILURE;
            return ret;
        }

        unsigned int *src8 = (map);

        int k = 0, n=0;
        for(i = 0; i < 1080 ; i++) {
            for(k = 0, j = 0; j < 1920; j++, k++) {
                temp[k] = (src8[j + n]);
            }

            n += 1920;
            temp += 1920;

        }

        ret2 = fclose(fp);
        if(ret2 != 0) {
            printf("fclose fail=%d\n", ret2);
            ret = DFB_FAILURE;
            return ret;
        }
        printf("reading abgr2101010 buffer done\n\n");
    } else {
        printf("open file failed \n");
    }

    primary->Unlock( primary);
    primary->Flip( primary, NULL, flip_mode );
    sleep(3);

    primary->Release(primary);

    sleep(3);

    printf("example 2: show 256 x 128 ARGB2101010 buffer\n");
    layer_config.pixelformat = DSPF_ARGB2101010;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = 1280;
    layer_config.height = 720;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));
    printf("%s, getsurface, !\n\n", __FUNCTION__);
    layer->GetSurface(layer, &primary);


    API_TRACE(primary->Lock3(primary, DSLF_READ, &data, &u64phy, &pitch));
    printf("%s, Lock3 return va: %p, pa: %llx, pitch: %d\n", __FUNCTION__, data, u64phy, pitch);

    temp = data;
    fp = fopen(a2rgb101010, "rb");
    if (fp != NULL) {
        printf("open file %s PASS\n", a2rgb101010);
        printf("reading A2RGB10 buffer...ing\n");
        ret2 = fread(map2, 128*256*4, 1, fp);
        if(ret2 == 0) {
            ret2 = fclose(fp);
            if(ret2 != 0) {
                printf("fclose fail=%d\n", ret2);
                ret = DFB_FAILURE;
                return ret;
            }

            ret = DFB_FAILURE;
            return ret;
        }

        unsigned int *src8 = (map2);

        int k = 0, n=0;
        for(i = 0; i < 128 ; i++) {
            for(k = 0, j = 0; j < 256; j++, k++) {
                temp[k] = (src8[j + n]);
            }

            n += 256;
            temp += 1280;

        }

        ret2 = fclose(fp);
        if(ret2 != 0) {
            printf("fclose fail=%d\n", ret2);
            ret = DFB_FAILURE;
            return ret;
        }
        printf("reading a2rgb101010 buffer done\n\n");
    } else {
        printf("open file failed \n");
    }

    primary->Unlock( primary);
    primary->Flip( primary, NULL, flip_mode );
    sleep(3);


    primary->Release(primary);

    sleep(3);

    /* Release the layer. */
    ret=layer->Release( layer );

    return ret;
}


static DFBResult secure_layer_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBWindow  *window = NULL;
    IDirectFBSurface     *surface = NULL;
	
    DFBDisplayLayerConfig   layer_config;
   
    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_SURFACE_CAPS);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = layer_width;
    layer_config.height = layer_height;
    layer_config.surface_caps = DSCAPS_SECURE_MODE;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    layer->GetSurface(layer, &surface);
    
    surface->SetColor( surface, 0, 0xF0, 0xc0, 0xFF );
    
    surface->FillRectangle( surface, 0, 0, layer_width, layer_height );
       
    surface->Flip( surface, NULL, flip_mode );


    if (bDump)
        surface->Dump( surface, "./", "test");

    sleep(3);

    /* reset to normal mode */
    API_TRACE(layer->GetConfiguration( layer, &layer_config ));
    layer_config.flags = DLCONF_SURFACE_CAPS;
    layer_config.surface_caps &= ~DSCAPS_SECURE_MODE;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

static DFBResult secure_window_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBScreen      *screen = NULL;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBWindow  *window = NULL;
    IDirectFBSurface     *surface = NULL;

    DFBDisplayLayerConfig   layer_config;

    DFBWindowDescription m_desc_top = {
         .flags         = DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_SURFACE_CAPS ,
         .posx          = 0,
         .posy          = 0,
         .width         = layer_width,
         .height        = layer_height,
         .surface_caps = DSCAPS_SECURE_MODE,
         .caps          = DWCAPS_ALPHACHANNEL,
    };

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );

     // set layer cap.
    layer_config.flags = (DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_WIDTH | DLCONF_HEIGHT | DLCONF_SURFACE_CAPS);

    layer_config.pixelformat = surf_pix_format;
    layer_config.buffermode = layer_buff_mode;
    layer_config.width = layer_width;
    layer_config.height = layer_height;
    layer_config.surface_caps = DSCAPS_SECURE_MODE;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    API_TRACE(layer->CreateWindow( layer, &m_desc_top, &window ));
    
    API_TRACE(window->GetSurface( window, &surface ));
    
    API_TRACE(window->SetOpacity( window, 0xff ));
    
    API_TRACE(surface->SetColor( surface, 0xf0, 0, 0xc0, 0xFF ));
    
    API_TRACE(surface->FillRectangle( surface, 0, 0, m_desc_top.width, m_desc_top.height ));
       
    API_TRACE(surface->Flip( surface, NULL, flip_mode ));


    if (bDump)
        surface->Dump( surface, "./", "test");

    sleep(3);

    /* reset to normal mode */
    API_TRACE(layer->GetConfiguration( layer, &layer_config ));
    layer_config.flags = DLCONF_SURFACE_CAPS;
    layer_config.surface_caps &= ~DSCAPS_SECURE_MODE;
    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    window->Release( window );
    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

static DFBResult SetGOPRatio_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret;
    IDirectFBDisplayLayer  *layer = NULL;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 1, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    ret = layer->SetGOPOutputRatio( layer, true );
    if (ret != DFB_OK) {
	printf("[%s] %s, SetGOPOutputRatio failed\n", __FILE__, __FUNCTION__);
    }

    return ret;
}

/**************************************************************/

static Test m_tests[] = {
     { "simple test",           simple_test },
     { "change layer size",     change_layer_size },
     { "multi layers test",     multi_layers },
     { "buffer mode test",      buffer_mode_test },
     { "surface test",          surface_test  },
     { "blit test",             blit_test },
     { "hvscale test",          HVScale_test},
     { "ttx test",              TTX_test},
     { "layer fps test",        layer_fps_test},
     { "window_flip_test",      window_flip },
     { "window_rotate_test",    window_rotate },
     { "preallocate_surface_test", preallocate_surface_test},
     { "window_auto_fit", window_auto_fit},
     { "screen_scale_test", screen_scale_test},
     { "preallocate_surface_create_by_dmabuf_id", preallocate_surface_create_by_dmabuf_id},
     { "preallocate_surface_create_by_dmabuf_id_child", preallocate_surface_create_by_dmabuf_id_child},
     { "HDR test",           HDR_test },
     { "secure layer test", secure_layer_test},
     { "secure window test", secure_window_test},
     { "cursor test", cursor_test},
     { "Set GOP Output Ratio", SetGOPRatio_test},
};

/**************************************************************/

static void
print_usage (const char *prg_name)
{
     int i;
     int all_items = sizeof(m_tests)/sizeof(Test);

     fprintf (stderr, "\nUsage: %s [options] ...\n\n", prg_name);
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "   -h, --help, \tShow this help message.\n");
     fprintf (stderr, "   -test item, \n");

     for (int i =0; i < all_items; i++) {
         fprintf (stderr, "         item = %d, %s.\n", i+1, m_tests[i].name);
     }
     
     fprintf (stderr, "   -buffer [num=1, 2, 3],           set layer buffer mode.\n");
     fprintf (stderr, "   -format [ARGB, RGB16, ARGB1555]  set surface color format.\n");
     fprintf (stderr, "   -flip [blit] ,                   using blit mode on flip. otherwise, default is flip mode\n");
     fprintf (stderr, "   -size [576P, 720P, 1080P] ,      set layer size.\n");
     fprintf (stderr, "\n");
}

bool parse_command_line( int argc, char *argv[] )
{
    bool ret = false;
    int all_items = sizeof(m_tests)/sizeof(Test);

    if(argc >1)
    {
        int n = 0;
        E_COMMAND_TYPE cmd = CMD_UNKNOWN;

        for (n = 1; n < argc; n++) {
            char *arg = argv[n];

            if (strcmp (arg, "-test") == 0 ) {
                 cmd = CMD_TEST_ITEM;
                 continue;
            }

            if (strcmp (arg, "-buffer") == 0 ) {
                cmd = CMD_BUFFER_MODE;
                continue;
            }

            if (strcmp (arg, "-format") == 0 ) {
                cmd = CMD_PIXEL_FORMAT;
                continue;
            }

            if (strcmp (arg, "-flip") == 0) {
                 cmd = CMD_FLIP_MODE;
                 continue;
            }

            if (strcmp (arg, "-dump") == 0) {
                bDump = true;
                continue;
            }

            if (strcmp (arg, "-size") == 0) {
                cmd = CMD_LAYER_SIZE;
                continue;
            }

	     if (strcmp (arg, "-layer") == 0) {
	         cmd = CMD_LAYER_ID;
	         continue;
	     }     

	     if (strcmp (arg, "-window") == 0) {
	         cmd = CMD_WINDOW_NUM;
	         continue;
	     }

            if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
                 print_usage (argv[0]);
                 ret = true;
                 break;
            }

            switch(cmd)
            {
                 case CMD_TEST_ITEM:
                     test_item = strtol(arg, NULL, 10);
                     if (test_item <= all_items && test_item > 0) {
                         test_item--;
                         ret = true;
                         if(test_item == 15)
                         {
                             pitch = strtol(argv[n+1], NULL, 10);
                             if (errno != 0)
                             {
                                 perror("strtol");
                                 ret = false;
                             }
                             dmabuf_id= strtol(argv[n+2], NULL, 10);
                             if (errno != 0)
                             {
                                 printf("[DFB](%s : %d) strtol failed!\n",__FUNCTION__,__LINE__);
                                 perror("strtol");
                                 ret = false;
                             }
                             return ret;
                         }
                     }
                     else {
                         printf("test item %d is not exist! all items is %d\n", test_item, all_items);
                         ret = false;
                     }

                     break;

                  case CMD_FLIP_MODE:
                      if ( strcmp( arg, "blit" ) == 0 )
                           flip_mode = DSFLIP_BLIT;
                      else
                           flip_mode = DSFLIP_NONE;
                      break;

                  case CMD_BUFFER_MODE:
                      switch( strtol(arg, NULL, 10) )
                      {
                      case 3:
                        layer_buff_mode = DLBM_TRIPLE;
                        break;
                      case 2:
                        layer_buff_mode = DLBM_BACKVIDEO;
                        break;
                      case 1:
                      default:
                        layer_buff_mode = DLBM_FRONTONLY;
                        break;
                      }
                      break;

                  case CMD_PIXEL_FORMAT:
                      if ( strcmp( arg, "ARGB") == 0 )
                          surf_pix_format = DSPF_ARGB;
                      else if ( strcmp( arg, "RGB16") == 0 )
                          surf_pix_format = DSPF_RGB16;
                      else if ( strcmp(arg, "ARGB1555") == 0 )
                          surf_pix_format = DSPF_ARGB1555;
                      break;

                  case CMD_LAYER_SIZE:
                      if ( strcmp( arg, "1080P") == 0 ) {
                          layer_width = 1920;
                          layer_height = 1080;
                      }
                      else if ( strcmp( arg, "576P") == 0 )  {
                          layer_width = 720;
                          layer_height = 576;
                      }
                      else { // default using 720P.
                          layer_width = 1280;
                          layer_height = 720;
                      }
                      break;

                  case CMD_LAYER_ID:
                      layer_id = strtol(arg, NULL, 10);
                      if (layer_id < 0 || layer_id > 3)
                          layer_id =0;
                      break;

  	           case CMD_WINDOW_NUM:
                      window_num = strtol(arg, NULL, 10);
                      if (window_num < 1 )
                          layer_id =1;
                      break;
                  default:
                      break;
             }

        }

    }

    if ( ret == false ) {
        print_usage (argv[0]);
    }

    return ret;

}


int
main( int argc, char *argv[] )
{
     DFBResult  ret;
     IDirectFB *dfb = NULL;

     if(!parse_command_line( argc, argv ))
        return 0;

     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          D_DERROR( ret, "DFBTest/Reinit: DirectFBInit() failed!\n" );
          return ret;
     }

	 printf("DirectFBInit ok!\n");

     /* Create super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          D_DERROR( ret, "DFBTest/Reinit: 1st DirectFBCreate() failed!\n" );
          return ret;
     }
	 printf("DirectFBCreate ok!\n");


     RunTest( m_tests[test_item].func, m_tests[test_item].name, dfb, NULL );

     printf("%s, dfb release!\n", __FILE__);
     /* Shutdown DirectFB. */
     dfb->Release( dfb );

     return ret;
}

