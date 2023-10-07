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
#include <sys/time.h>

#define API_TRACE(X)\
    do{\
        printf("\33[0;33;44m%s, %d, "#X" \33[0m\n", __FUNCTION__, __LINE__);\
        ret = X;\
        if ( ret != DFB_OK ) {\
            fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
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

/* 1. just render picture in window */
static DFBResult show_picture( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerConfig   layer_config;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );
    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );



    for(i = 0; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );

    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

/* 2. multi view sample case 1 */
static DFBResult multi_view_sample( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerConfig   layer_config;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );
    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );



    for(i = 0; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );

    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }

    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 2, layer_config.height / 2);
    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, 0 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], 0, layer_config.height / 2 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 4, layer_config.height);
    for(i = 0; i < 4; i++) {
        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 4, layer_config.height );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 4, layer_config.height );
            window[i]->MoveTo( window[i], layer_config.width / 4, 0 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 4, layer_config.height );
            window[i]->MoveTo( window[i], (layer_config.width / 4) * 2, 0 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 4, layer_config.height );
            window[i]->MoveTo( window[i], (layer_config.width / 4) * 3, 0 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

/* 3. multi view sample case 2 */
static DFBResult multi_view_sample2( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerConfig   layer_config;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );
    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );



    for(i = 0; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );

    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }

    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 2, layer_config.height / 2);
    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, 0 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], 0, layer_config.height / 2 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);
    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, 480, 270);
    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], 480, 270 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], 480, 270 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], 480, 270 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], 480, 270 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, 400, 800);
    for(i = 0; i < 4; i++) {
        if(i == 0) {
            window[i]->Resize( window[i], 400, 800 );
            window[i]->MoveTo( window[i], 30, 40 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], 400, 800 );
            window[i]->MoveTo( window[i], 660, 40 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], 400, 800 );
            window[i]->MoveTo( window[i], 1290, 40 );
        }
        else if(i == 3) {
            window[i]->SetOpacity( window[i], 0 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

/* 4. multi view sample case 3 */
static DFBResult multi_view_sample3( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerConfig   layer_config;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;
    DFBDimension     size;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );
    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );



    for(i = 0; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );

    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }

    window[0]->GetSize( window[0], &size.w, &size.h );
    DFBDimension sizes[] = { { size.w - 96, size.h - 54},
                                         { size.w -96 * 2, size.h - 54 * 2 },
                                         { size.w -96 * 3, size.h -54 * 3 },
                                         { size.w -96 * 4, size.h -54 * 4 },
                                         { size.w -96 * 5, size.h -54 * 5 },
                                         { size.w -96 * 6, size.h -54 * 6 },
                                         { size.w -96 * 7, size.h -54 * 7 },
                                         { size.w -96 * 8, size.h -54 * 8 },
                                         { size.w -96 * 9, size.h -54 * 9 },
                                         { size.w -96 * 10, size.h -54 * 10 },
                                         { size.w -96 * 11, size.h -54 * 11 },
                                         { size.w -96 * 12, size.h -54 * 12 },
                                         { size.w -96 * 13, size.h -54 * 13 },
                                         { size.w -96 * 14, size.h -54 * 14 },
                                         { size.w -96 * 15, size.h -54 * 15 }
          };


    int count = 0;
    while(count < D_ARRAY_SIZE(sizes)) {
        printf("[%s %d] resize to %dx%d\n", __FUNCTION__, __LINE__, sizes[count].w, sizes[count].h);

        for(i = 0; i < 4; i++) {

            if(i == 0) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
            }
            else if(i == 1) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 96 * (count + 1), 0 );
            }
            else if(i == 2) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 0, 54 * (count + 1) );
            }
            else if(i == 3) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 96 * (count + 1), 54 * (count + 1) );
            }
        }

        window_surface[0]->Flip( window_surface[0], NULL, 0 );
        count++;
    }

    sleep(3);

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );

    return ret;
}

/* 5. change window z-order */
static DFBResult multi_view_z_order( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerConfig   layer_config;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );
    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );



    for(i = 0; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );

    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }

    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, (layer_config.width / 3) * 2, (layer_config.height / 3) * 2);
    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 5, layer_config.height / 5 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], (layer_config.width / 5) * 2, layer_config.height / 5 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 5, (layer_config.height / 5) * 2 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], (layer_config.width / 5) * 2, (layer_config.height / 5) * 2 );
        }

    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);


    window[0]->RaiseToTop( window[0] );
    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    window[2]->RaiseToTop( window[2] );
    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);

    window[1]->RaiseToTop( window[1] );
    window_surface[0]->Flip( window_surface[0], NULL, 0 );
    sleep(3);


    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );

    return ret;
}


/* 6. two window in layer 0, two window in layer 1 */
static DFBResult multi_view_multi_layer( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBDisplayLayer  *layer2 = NULL;
    DFBDisplayLayerConfig   layer_config, layer_config2;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;
    DFBDimension     size;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 1, &layer2 );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );

    API_TRACE(layer2->SetCooperativeLevel( layer2, DLSCL_ADMINISTRATIVE ) );
    layer2->GetConfiguration( layer2, &layer_config2 );

    layer_config2.flags = ( DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config2.width = 1920;
    layer_config2.height = 1080;
    API_TRACE(layer2->SetConfiguration( layer2, &layer_config2 ));


    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );


    /* two window in layer 0 */
    for(i = 0; i < 2; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }

    /* two window in layer 1 */
    for(i = 2; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config2.width;
        desc[i].height = layer_config2.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer2->CreateWindow( layer2, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }

    window[0]->GetSize( window[0], &size.w, &size.h );
    DFBDimension sizes[] = { { size.w - 96, size.h - 54},
                                         { size.w -96 * 2, size.h - 54 * 2 },
                                         { size.w -96 * 3, size.h -54 * 3 },
                                         { size.w -96 * 4, size.h -54 * 4 },
                                         { size.w -96 * 5, size.h -54 * 5 },
                                         { size.w -96 * 6, size.h -54 * 6 },
                                         { size.w -96 * 7, size.h -54 * 7 },
                                         { size.w -96 * 8, size.h -54 * 8 },
                                         { size.w -96 * 9, size.h -54 * 9 },
                                         { size.w -96 * 10, size.h -54 * 10 },
                                         { size.w -96 * 11, size.h -54 * 11 },
                                         { size.w -96 * 12, size.h -54 * 12 },
                                         { size.w -96 * 13, size.h -54 * 13 },
                                         { size.w -96 * 14, size.h -54 * 14 },
                                         { size.w -96 * 15, size.h -54 * 15 }
          };



    int count = 0;
    while(count < D_ARRAY_SIZE(sizes)) {
        printf("[%s %d] resize to %dx%d\n", __FUNCTION__, __LINE__, sizes[count].w, sizes[count].h);
        for(i = 0; i < 4; i++) {

            if(i == 0) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
            }
            else if(i == 1) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 96 * (count + 1), 0 );
            }
            else if(i == 2) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 0, 54 * (count + 1) );
            }
            else if(i == 3) {
                window[i]->Resize( window[i], sizes[count].w, sizes[count].h );
                window[i]->MoveTo( window[i], 96 * (count + 1), 54 * (count + 1) );
            }
        }

        window_surface[0]->Flip( window_surface[0], NULL, 0 );
        window_surface[2]->Flip( window_surface[2], NULL, 0 );

        count++;
    }

    sleep(3);

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );
    layer2->Release( layer2 );
    return ret;
}

/* 7. one window in layer 0, three window in layer 1 */
static DFBResult multi_view_multi_layer2( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBDisplayLayer  *layer2 = NULL;
    DFBDisplayLayerConfig   layer_config, layer_config2;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;
    DFBDimension     size;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 1, &layer2 );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );

     // set layer cap.
    layer_config.flags = DLCONF_BUFFERMODE;
    layer_config.buffermode = DLBM_TRIPLE;

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));


    API_TRACE(layer2->SetCooperativeLevel( layer2, DLSCL_ADMINISTRATIVE ) );
    layer2->GetConfiguration( layer2, &layer_config2 );

    layer_config2.flags = ( DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config2.width = 1920;
    layer_config2.height = 1080;
    API_TRACE(layer2->SetConfiguration( layer2, &layer_config2 ));


    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );


    /* one window in layer 0 */
    for(i = 0; i < 1; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }

    /* three window in layer 1 */
    for(i = 1; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config2.width;
        desc[i].height = layer_config2.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer2->CreateWindow( layer2, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }

    for(i = 0; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 0; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }


    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 2, layer_config.height / 2);

    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, 0 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], 0, layer_config.height / 2 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );

    window_surface[2]->Flip( window_surface[2], NULL, 0 );

    sleep(3);

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );
    layer2->Release( layer2 );
    return ret;
}

/* 8. resize two csp in layer 0, two window in layer 1 */
static DFBResult multi_view_csp_test( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBDisplayLayer  *layer2 = NULL;
    DFBDisplayLayerConfig   layer_config, layer_config2;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;
    DFBDimension     size;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, 1, &layer2 );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }

    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );

     // set layer cap.
    layer_config.flags = DLCONF_BUFFERMODE;
    layer_config.buffermode = DLBM_TRIPLE; //DLBM_FRONTONLY; //DLBM_BACKVIDEO, DLBM_BACKSYSTEM, DLBM_TRIPLE

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));


    API_TRACE(layer2->SetCooperativeLevel( layer2, DLSCL_ADMINISTRATIVE ) );
    layer2->GetConfiguration( layer2, &layer_config2 );

    layer_config2.flags = ( DLCONF_WIDTH | DLCONF_HEIGHT);
    layer_config2.width = 1920;
    layer_config2.height = 1080;
    API_TRACE(layer2->SetConfiguration( layer2, &layer_config2 ));


    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );
    dfb->CreateImageProvider( dfb, "Wang3.png", &(provider[1]) );
    dfb->CreateImageProvider( dfb, "Wang4_small.jpg", &(provider[2]) );
    dfb->CreateImageProvider( dfb, "Wang8_small.jpg", &(provider[3]) );

    /* two window in layer 0 */
    /* id 2 is nfx splash */
    ret = layer->GetWindow(layer, 2, &window[0]);
    if (ret) {
        printf("GetWindow() id 2 failed!\n" );
        return ret;
    }
    window[0]->GetSurface( window[0], &window_surface[0] );
    window[0]->SetOpacity( window[0], 0xFF );

    /* id 3 is sraf */
    ret = layer->GetWindow(layer, 3, &window[1]);
    if (ret) {
        printf("GetWindow() id 3 failed!\n" );
        return ret;
    }
    window[1]->GetSurface( window[1], &window_surface[1] );
    window[1]->SetOpacity( window[1], 0xFF );

    /* two window in layer 1 */
    for(i = 2; i < 4; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer2->CreateWindow( layer2, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }


    for(i = 2; i < 4; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    /* double buffer, source need to render twice */
    for(i = 2; i < 4; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(1);
    }


    for(i = 0; i < 4; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }


    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 2, layer_config.height / 2);

    for(i = 0; i < 4; i++) {

        if(i == 0) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
        else if(i == 1) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, 0 );
        }
        else if(i == 2) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], 0, layer_config.height / 2 );
        }
        else if(i == 3) {
            window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
            window[i]->MoveTo( window[i], layer_config.width / 2, layer_config.height / 2 );
        }
    }

    window_surface[0]->Flip( window_surface[0], NULL, 0 );

    window_surface[2]->Flip( window_surface[2], NULL, 0 );

    sleep(5);

    for(i = 0; i < 4; i++) {
        window[i]->MoveTo( window[i], 0, 0 );
        window[i]->Resize( window[i], layer_config.width, layer_config.height );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_MULTI_VIEW) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_SCALE_WINDOW_AUTOFIT_LAYER);
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
    }

    for(i=0; i< 4; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }


    /* Release the layer. */
    layer->Release( layer );
    layer2->Release( layer2 );
    return ret;
}

/* 9. render picture to window and layer */
static DFBResult multi_view_window_and_layer( IDirectFB *dfb, void *arg )
{
    DFBResult ret = DFB_OK;
    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBDisplayLayer  *layer2 = NULL;
    DFBDisplayLayerConfig   layer_config, layer_config2;
    IDirectFBWindow     *window[4];
    IDirectFBSurface     *window_surface[4];
    IDirectFBSurface     *layer_surface;
    DFBWindowDescription  desc[4];
    DFBWindowOptions opts;
    IDirectFBImageProvider  *provider[4] = {NULL};
    DFBRectangle rect[4] = {NULL};
    int w, h, i;
    DFBDimension     size;

    /* Get the primary layer interface. */
    ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
    if (ret) {
        D_DERROR( ret, "IDirectFB::GetDisplayLayer() failed!\n" );
        return ret;
    }


    API_TRACE(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ) );
    layer->GetConfiguration( layer, &layer_config );

     // set layer cap.
    layer_config.flags = DLCONF_BUFFERMODE;
    layer_config.buffermode = DLBM_TRIPLE; //DLBM_FRONTONLY; //DLBM_BACKVIDEO, DLBM_BACKSYSTEM, DLBM_TRIPLE

    API_TRACE(layer->SetConfiguration( layer, &layer_config ));

    /* get layer 0 surface */
    layer->GetSurface( layer, &layer_surface );

    dfb->CreateImageProvider( dfb, "Wang2.png", &(provider[0]) );

    for(i = 0; i < 1; i++) {
        memset(&desc[i], 0, sizeof(DFBWindowDescription));
        desc[i].flags = ( DWDESC_POSX | DWDESC_POSY |
                       DWDESC_WIDTH | DWDESC_HEIGHT |DWDESC_OPTIONS |DWDESC_CAPS |DWDESC_OPTIONS);
        desc[i].posx   = 0;
        desc[i].posy   = 0;
        desc[i].width  = layer_config.width;
        desc[i].height = layer_config.height;
        desc[i].caps = DWCAPS_ALPHACHANNEL;
        desc[i].options = DWOP_SCALE |DWOP_SCALE_WINDOW_AUTOFIT_LAYER;
        layer->CreateWindow( layer, &desc[i], &window[i] );
        window[i]->GetSurface( window[i], &window_surface[i] );
        window[i]->SetOpacity( window[i], 0xFF );
    }


    for(i = 0; i < 1; i++) {
        rect[i].x = 0;
        rect[i].y = 0;
        rect[i].w = layer_config.width;
        rect[i].h = layer_config.height;
    }

    printf("%s, %d, layer flip\n", __FUNCTION__, __LINE__);
    for(i = 0; i < 1; i++) {
        provider[i]->RenderTo( provider[i], layer_surface, &(rect[i]) );
        layer_surface->Flip( layer_surface, NULL, 0 );
        sleep(3);
    }

    printf("%s, %d, window flip\n", __FUNCTION__, __LINE__);
    for(i = 0; i < 1; i++) {
        provider[i]->RenderTo( provider[i], window_surface[i], &(rect[i]) );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
        sleep(3);
    }

    printf("%s, %d, layer clear\n", __FUNCTION__, __LINE__);
    layer_surface->Clear(layer_surface, 0,0,0,0);
    sleep(3);

    for(i = 0; i < 1; i++) {
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_SCALE_WINDOW_AUTOFIT_LAYER) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_MULTI_VIEW);
    }


    printf("%s, %d, resize to %dx%d\n", __FUNCTION__, __LINE__, layer_config.width / 2, layer_config.height / 2);

    for(i = 0; i < 1; i++) {
        window[i]->Resize( window[i], layer_config.width / 2, layer_config.height / 2 );
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
    }

    sleep(5);

    for(i = 0; i < 1; i++) {
        window[i]->MoveTo( window[i], 0, 0 );
        window[i]->Resize( window[i], layer_config.width, layer_config.height );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts & ~( DWOP_MULTI_VIEW) );
        window[i]->GetOptions( window[i], &opts );
        window[i]->SetOptions( window[i],  opts | DWOP_SCALE_WINDOW_AUTOFIT_LAYER);
        window_surface[i]->Flip( window_surface[i], NULL, 0 );
    }

    sleep(5);
    printf("[%s %d]  layer flip\n", __FUNCTION__, __LINE__);
    for(i = 0; i < 1; i++) {
        provider[i]->RenderTo( provider[i], layer_surface, &(rect[i]) );
        layer_surface->Flip( layer_surface, NULL, 0 );
        sleep(5);
    }

    printf("[%s %d]  window Clear\n", __FUNCTION__, __LINE__);
    window_surface[0]->Clear( window_surface[0], 0, 0, 0, 0 );

    sleep(5);
    printf("[%s %d]  layer Clear\n", __FUNCTION__, __LINE__);
    layer_surface->Clear(layer_surface, 0,0,0,0);
    layer_surface->Flip( layer_surface, NULL, 0 );

    sleep(5);

    for(i=0; i< 1; i++)
    {
        if (provider[i])  provider[i]->Release( provider[i] );
        window_surface[i]->Release( window_surface[i] );
        window[i]->Release( window[i] );
    }

    layer_surface->Release(layer_surface);
    /* Release the layer. */
    layer->Release( layer );
    return ret;
}



/**************************************************************/

static Test m_tests[] = {
     { "show picture",           show_picture },
     { "multi view sample",     multi_view_sample },
     { "multi view sample 2",     multi_view_sample2 },
     { "multi view sample 3",      multi_view_sample3 },
     { "multi view z-order",          multi_view_z_order  },
     { "multi view multi layer",          multi_view_multi_layer  },
     { "multi view multi layer 2",          multi_view_multi_layer2  },
     { "multi view csp test",          multi_view_csp_test  },
     { "multi view window and layer",          multi_view_window_and_layer  },
};

/**************************************************************/

static void
print_usage (const char *prg_name)
{
     fprintf (stderr, "\nUsage: %s [options] ...\n\n", prg_name);
     fprintf (stderr, "Note: folder need include Wang2.png, Wang3.png, Wang4_small.jpg, Wang8_small.jpg\n\n");
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "   -h, --help, \tShow this help message.\n");
     fprintf (stderr, "   -test item, \n");
     fprintf (stderr, "         item = 1, show picture.\n");
     fprintf (stderr, "         item = 2, multi view sample.\n");
     fprintf (stderr, "         item = 3, multi view sample 2.\n");
     fprintf (stderr, "         item = 4, multi view sample 3.\n");
     fprintf (stderr, "         item = 5, multi view z-order.\n");
     fprintf (stderr, "         item = 6, multi view multi layer.\n");
     fprintf (stderr, "         item = 7, multi view multi layer 2.\n");
     fprintf (stderr, "         item = 8, multi view csp test.\n");
     fprintf (stderr, "         item = 9, multi view window and layer.\n");
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

            if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
                 ret = false;
                 break;
            }

            switch(cmd)
            {
                 case CMD_TEST_ITEM:
                     test_item = strtol(arg, NULL, 10);
                     if (test_item <= all_items && test_item > 0) {
                         test_item--;
                         ret = true;
                     }
                     else {
                         printf("test item %d is not exist! all items is %d\n", test_item, all_items);
                         ret = false;
                     }

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

#define KEYCODE_CNT  DIKS_CUSTOM99 - DIKS_CUSTOM80 + 1
int
main( int argc, char *argv[] )
{
     DFBResult  ret;
     IDirectFB *dfb = NULL;

     if(!parse_command_line( argc, argv ))
        return 0;

     /* Initialize DirectFB. */
     ret = DirectFBInit( NULL, NULL );
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

