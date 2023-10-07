/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/surface.h>
#include <core/gfxcard.h>
#include <core/layers.h>
#include <core/layers_internal.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/state.h>
#include <core/windows.h>
#include <core/windows_internal.h>
#include <core/windowstack.h>
#include <core/wm.h>

#include <core/CoreDFB.h>
#include <core/CoreLayer.h>
#include <core/CoreLayerContext.h>
#include <core/CoreLayerRegion.h>
#include <core/CoreWindowStack.h>

#include <windows/idirectfbwindow.h>

#include <gfx/convert.h>

#include <direct/debug.h>
#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <display/idirectfbdisplaylayer.h>
#include <display/idirectfbscreen.h>
#include <display/idirectfbsurface.h>
#include <display/idirectfbsurface_layer.h>


D_DEBUG_DOMAIN( Layer, "IDirectFBDisplayLayer", "Display Layer Interface" );

/*
 * private data struct of IDirectFB
 */
typedef struct {
     int                              ref;              /* reference counter */
     DFBDisplayLayerDescription       desc;             /* description of the layer's caps */
     DFBDisplayLayerCooperativeLevel  level;            /* current cooperative level */
     CoreScreen                      *screen;           /* layer's screen */
     CoreLayer                       *layer;            /* core layer data */
     CoreLayerContext                *context;          /* shared or exclusive context */
     CoreLayerRegion                 *region;           /* primary region of the context */
     CoreWindowStack                 *stack;            /* stack of shared context */
     DFBBoolean                       switch_exclusive; /* switch to exclusive context after creation? */
     CoreDFB                         *core;
     IDirectFB                       *idirectfb;
} IDirectFBDisplayLayer_data;



static void
IDirectFBDisplayLayer_Destruct( IDirectFBDisplayLayer *thiz )
{
     IDirectFBDisplayLayer_data *data = (IDirectFBDisplayLayer_data*)thiz->priv;

     D_DEBUG_AT( Layer, "IDirectFBDisplayLayer_Destruct()\n" );

     D_DEBUG_AT( Layer, "  -> unref region...\n" );

     dfb_layer_region_unref( data->region );

     D_DEBUG_AT( Layer, "  -> unref context...\n" );

     dfb_layer_context_unref( data->context );

     DIRECT_DEALLOCATE_INTERFACE( thiz );

     D_DEBUG_AT( Layer, "  -> done.\n" );
}

static DirectResult
IDirectFBDisplayLayer_AddRef( IDirectFBDisplayLayer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBDisplayLayer_Release( IDirectFBDisplayLayer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (--data->ref == 0)
          IDirectFBDisplayLayer_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetID( IDirectFBDisplayLayer *thiz,
                             DFBDisplayLayerID     *id )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!id)
          return DFB_INVARG;

     *id = dfb_layer_id_translated( data->layer );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetDescription( IDirectFBDisplayLayer      *thiz,
                                      DFBDisplayLayerDescription *desc )
{

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!desc)
          return DFB_INVARG;

     *desc = data->desc;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetSurface( IDirectFBDisplayLayer  *thiz,
                                  IDirectFBSurface      **interface )
{
     DFBResult         ret;
     CoreLayerRegion  *region;
     IDirectFBSurface *surface;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!interface)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED) {
          D_WARN( "letting unprivileged IDirectFBDisplayLayer::GetSurface() "
                   "call pass until cooperative level handling is finished" );
     }

     ret = CoreLayerContext_GetPrimaryRegion( data->context, true, &region );
     if (ret)
          return ret;

     DIRECT_ALLOCATE_INTERFACE( surface, IDirectFBSurface );

     ret = IDirectFBSurface_Layer_Construct( surface, NULL, NULL, NULL,
                                             region, DSCAPS_NONE, data->core );

     // Fix to only perform single buffered clearing using a background when 
     // configured to do so AND the display layer region is frozen.  Also 
     // added support for this behavior when the cooperative level is 
     // DLSCL_ADMINISTRATIVE.
     if (region->config.buffermode == DLBM_FRONTONLY && 
         data->level != DLSCL_SHARED && 
         D_FLAGS_IS_SET( region->state, CLRSF_FROZEN )) {
          // If a window stack is available, give it the opportunity to 
          // render the background (optionally based on configuration) and 
          // flip the display layer so it is visible.  Otherwise, just 
          // directly flip the display layer and make it visible.
          D_ASSERT( region->context );
          if (region->context->stack) {
               CoreWindowStack_RepaintAll( region->context->stack );
          }
          else {
               CoreLayerRegion_FlipUpdate( region, NULL, DSFLIP_NONE );
          }
     }

     *interface = ret ? NULL : surface;

     dfb_layer_region_unref( region );

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_GetScreen( IDirectFBDisplayLayer  *thiz,
                                 IDirectFBScreen       **interface )
{
     DFBResult        ret;
     IDirectFBScreen *screen;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!interface)
          return DFB_INVARG;

     DIRECT_ALLOCATE_INTERFACE( screen, IDirectFBScreen );

     ret = IDirectFBScreen_Construct( screen, data->screen );

     *interface = ret ? NULL : screen;

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_SetCooperativeLevel( IDirectFBDisplayLayer           *thiz,
                                           DFBDisplayLayerCooperativeLevel  level )
{
     DFBResult         ret;
     CoreLayerContext *context;
     CoreLayerRegion  *region;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == level)
          return DFB_OK;

     switch (level) {
          case DLSCL_SHARED:
          case DLSCL_ADMINISTRATIVE:
               if (data->level == DLSCL_EXCLUSIVE) {
                    ret = CoreLayer_GetPrimaryContext( data->layer, false, &context );
                    if (ret)
                         return ret;

                    ret = CoreLayerContext_GetPrimaryRegion( context, true, &region );
                    if (ret) {
                         dfb_layer_context_unref( context );
                         return ret;
                    }

                    dfb_layer_region_unref( data->region );
                    dfb_layer_context_unref( data->context );

                    data->context = context;
                    data->region  = region;
                    data->stack   = dfb_layer_context_windowstack( data->context );
               }

               break;

          case DLSCL_EXCLUSIVE:
               ret = CoreLayer_CreateContext( data->layer, &context );
               if (ret)
                    return ret;

               if (data->switch_exclusive) {
                    ret = dfb_layer_activate_context( data->layer, context );
                    if (ret) {
                         dfb_layer_context_unref( context );
                         return ret;
                    }
               }

               ret = CoreLayerContext_GetPrimaryRegion( context, true, &region );
               if (ret) {
                    dfb_layer_context_unref( context );
                    return ret;
               }

               dfb_layer_region_unref( data->region );
               dfb_layer_context_unref( data->context );

               data->context = context;
               data->region  = region;
               data->stack   = dfb_layer_context_windowstack( data->context );

               break;

          default:
               return DFB_INVARG;
     }

     data->level = level;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetStretchMode(IDirectFBDisplayLayer *thiz, DFBDisplayLayerStretchSetting stretch_setting)
{

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_stretchsetting( data->context, stretch_setting );
}

static DFBResult
_IDirectFBDisplayLayer_SetStretchMode(IDirectFBDisplayLayer *thiz, DFBDisplayLayerStretchSetting stretch_setting)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetStretchMode(thiz,stretch_setting);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}


static DFBResult
IDirectFBDisplayLayer_SetOpacity( IDirectFBDisplayLayer *thiz,
                                  u8                     opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_opacity( data->context, opacity );
}

static DFBResult
IDirectFBDisplayLayer_GetCurrentOutputField( IDirectFBDisplayLayer *thiz, int *field )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     return dfb_layer_get_current_output_field( data->layer, field );
}

static DFBResult
IDirectFBDisplayLayer_SetFieldParity( IDirectFBDisplayLayer *thiz, int field )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level != DLSCL_EXCLUSIVE)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_field_parity( data->context, field );
}

static DFBResult
IDirectFBDisplayLayer_SetClipRegions( IDirectFBDisplayLayer *thiz,
                                      const DFBRegion       *regions,
                                      int                    num_regions,
                                      DFBBoolean             positive )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!regions || num_regions < 1)
          return DFB_INVARG;

     if (num_regions > data->desc.clip_regions)
          return DFB_UNSUPPORTED;

     if (data->level != DLSCL_EXCLUSIVE)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_clip_regions( data->context, regions, num_regions, positive );
}

static DFBResult
IDirectFBDisplayLayer_SetSourceRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
     DFBRectangle source = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (x < 0 || y < 0 || width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_sourcerectangle( data->context, &source );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenLocation( IDirectFBDisplayLayer *thiz,
                                         float                  x,
                                         float                  y,
                                         float                  width,
                                         float                  height )
{
     DFBLocation location = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_LOCATION ))
          return DFB_UNSUPPORTED;

     if (width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenlocation( data->context, &location );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenPosition( IDirectFBDisplayLayer *thiz,
                                         int                    x,
                                         int                    y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_POSITION ))
          return DFB_UNSUPPORTED;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenposition( data->context, x, y );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
     DFBRectangle rect = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_LOCATION ))
          return DFB_UNSUPPORTED;

     if (width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenrectangle( data->context, &rect );
}

static DFBResult
IDirectFBDisplayLayer_SetSrcColorKey( IDirectFBDisplayLayer *thiz,
                                      u8                     r,
                                      u8                     g,
                                      u8                     b )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_src_colorkey( data->context, r, g, b, -1 );
}

static DFBResult
IDirectFBDisplayLayer_SetDstColorKey( IDirectFBDisplayLayer *thiz,
                                      u8                     r,
                                      u8                     g,
                                      u8                     b )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_dst_colorkey( data->context, r, g, b, -1 );
}

static DFBResult
IDirectFBDisplayLayer_GetLevel( IDirectFBDisplayLayer *thiz,
                                int                   *level )
{
     DFBResult ret;
     int       lvl;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!level)
          return DFB_INVARG;

     ret = dfb_layer_get_level( data->layer, &lvl );
     if (ret)
          return ret;

     *level = lvl;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetLevel( IDirectFBDisplayLayer *thiz,
                                int                    level )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_LEVELS ))
          return DFB_UNSUPPORTED;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_set_level( data->layer, level );
}

static DFBResult
IDirectFBDisplayLayer_GetConfiguration( IDirectFBDisplayLayer *thiz,
                                        DFBDisplayLayerConfig *config )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!config)
          return DFB_INVARG;

     return dfb_layer_context_get_configuration( data->context, config );
}

static DFBResult
IDirectFBDisplayLayer_TestConfiguration( IDirectFBDisplayLayer       *thiz,
                                         const DFBDisplayLayerConfig *config,
                                         DFBDisplayLayerConfigFlags  *failed )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!config)
          return DFB_INVARG;

     if (((config->flags & DLCONF_WIDTH) && (config->width < 0)) ||
         ((config->flags & DLCONF_HEIGHT) && (config->height < 0)))
          return DFB_INVARG;

     return dfb_layer_context_test_configuration( data->context, config, failed );
}

static DFBResult
IDirectFBDisplayLayer_SetConfiguration( IDirectFBDisplayLayer       *thiz,
                                        const DFBDisplayLayerConfig *config )
{
     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if((access(dfb_config->mst_debug_directory_access, R_OK)) == 0){
         if(dfb_config->mst_debug_layer_setConfiguration_return) {
             D_INFO("[DFB] return from %s pid:%d\n",__FUNCTION__,getpid());
             return DFB_OK;
         }
     }

     if (!config)
          return DFB_INVARG;

     if (((config->flags & DLCONF_WIDTH) && (config->width <= 0)) ||
         ((config->flags & DLCONF_HEIGHT) && (config->height <= 0)))
          return DFB_INVARG;

     switch (data->level) {
          case DLSCL_EXCLUSIVE:
          case DLSCL_ADMINISTRATIVE:
          {

                D_INFO("[DFB] %s: width=%d, height=%d, buffermode=%d (pid=%d)\n", __FUNCTION__,
                config->flags & DLCONF_WIDTH ? config->width : -1,
                config->flags & DLCONF_HEIGHT ? config->height : -1,
                config->flags & DLCONF_BUFFERMODE ? config->buffermode : -1, getpid());

                if (dfb_config->mst_debug_backtrace_dump)
                {
                    void* trace[20] = {NULL};
                    int n = backtrace(trace, 20);
                    char **strings;
                    strings = backtrace_symbols(trace, n);
                    printf("backtrace n:%d\n", n);
                    if(n > 0 && strings != NULL)
                    {
                        int i;
                        for(i = 0; i < n; i++)
                            printf("backtrace:%d, %s\n", i, strings[i]);
                    }
                }


               return CoreLayerContext_SetConfiguration( data->context, config );
          }
          default:
               break;
     }
     
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DISPLAYLAYER_SETCONFIGURATION);
     
     return DFB_ACCESSDENIED;
}

static DFBResult
IDirectFBDisplayLayer_SetBackgroundMode( IDirectFBDisplayLayer         *thiz,
                                         DFBDisplayLayerBackgroundMode  background_mode )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     switch (background_mode) {
          case DLBM_DONTCARE:
          case DLBM_COLOR:
          case DLBM_IMAGE:
          case DLBM_TILE:
               break;

          default:
               return DFB_INVARG;
     }

     return CoreWindowStack_BackgroundSetMode( data->stack, background_mode );
}

static DFBResult
IDirectFBDisplayLayer_SetHVScale( IDirectFBDisplayLayer *thiz,
                                int                    HScale,
                                int                    VScale)
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     if (HScale <= 0 || VScale<=0)
        return DFB_INVARG;

     return dfb_layer_set_hv_scale( data->layer, HScale, VScale);
}

static DFBResult
IDirectFBDisplayLayer_ForceFullUpdateWindowStack( IDirectFBDisplayLayer         *thiz,
                                                       bool  full_update )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;
     
     return dfb_windowstack_set_force_fullupdate(data->stack,full_update);      
}

static DFBResult
_IDirectFBDisplayLayer_SetHVScale( IDirectFBDisplayLayer *thiz,
                                int                    HScale,
                                int                    VScale)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetHVScale(thiz, HScale, VScale);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}
static DFBResult
_IDirectFBDisplayLayer_ForceFullUpdateWindowStack( IDirectFBDisplayLayer         *thiz,
                                                        bool  full_update)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_ForceFullUpdateWindowStack(thiz,full_update);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}


static DFBResult
IDirectFBDisplayLayer_SetBackgroundImage( IDirectFBDisplayLayer *thiz,
                                          IDirectFBSurface      *surface )
{
     IDirectFBSurface_data *surface_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     /*
     layer->SetBackgroundImage( layer, NULL );
     if SetBackgroundImage null means we want to vanish background image,
     force background image ref count minus 1, and set background image null
     */
     if (!surface)
     {
          CoreWindowStack *stack = data->stack;

          if (!stack)
               return DFB_FAILURE;

          /* Lock the window stack. */
          if (dfb_windowstack_lock( stack ))
               return DFB_FUSION;

          if (stack->bg.image)
          {
               stack->bg.bRelease_image = true;
               dfb_surface_unlink( &stack->bg.image );
          }
          else
          {
               /* Unlock the window stack. */
               dfb_windowstack_unlock( stack );

               return DFB_INVARG;
          }
          /* Unlock the window stack. */
          dfb_windowstack_unlock( stack );

          return DFB_OK;
     }

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     surface_data = (IDirectFBSurface_data*)surface->priv;
     if (!surface_data)
          return DFB_DEAD;

     if (!surface_data->surface)
          return DFB_DESTROYED;

     return CoreWindowStack_BackgroundSetImage( data->stack,
                                                surface_data->surface );
}

static DFBResult
IDirectFBDisplayLayer_SetBackgroundColor( IDirectFBDisplayLayer *thiz,
                                          u8 r, u8 g, u8 b, u8 a )
{
     DFBColor color = { a: a, r: r, g: g, b: b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return CoreWindowStack_BackgroundSetColor( data->stack, &color );
}

static DFBResult
IDirectFBDisplayLayer_CreateWindow( IDirectFBDisplayLayer       *thiz,
                                    const DFBWindowDescription  *desc,
                                    IDirectFBWindow            **window )
{
     CoreWindow           *w;
     DFBResult             ret;
     DFBWindowDescription  wd;
     CoreWindow           *parent   = NULL;
     CoreWindow           *toplevel = NULL;
     int                  layer_id=0x0;
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     memset( &wd, 0, sizeof(wd) );

     wd.flags = DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY |
                DWDESC_PIXELFORMAT | DWDESC_COLORSPACE | DWDESC_SURFACE_CAPS | 
                DWDESC_CAPS;

     wd.width  = (desc->flags & DWDESC_WIDTH)  ? desc->width  : 480;
     wd.height = (desc->flags & DWDESC_HEIGHT) ? desc->height : 300;
     wd.posx   = (desc->flags & DWDESC_POSX)   ? desc->posx   : 100;
     wd.posy   = (desc->flags & DWDESC_POSY)   ? desc->posy   : 100;
     
     D_DEBUG_AT( Layer, "CreateWindow() <- %d,%d - %dx%d )\n", wd.posx, wd.posy, wd.width, wd.height );

     if (wd.width < 1 || wd.width > dfb_config->mst_ge_hw_limit || wd.height < 1 || wd.height > dfb_config->mst_ge_hw_limit)
     {
         D_ERROR("[DFB] (%s line %d) wd.width = %d, wd.height = %d, mst_ge_hw_limit = %d",__FUNCTION__,__LINE__, wd.width, wd.height, dfb_config->mst_ge_hw_limit);
         return DFB_INVARG;
     }

     if (desc->flags & DWDESC_CAPS) {
          if ((desc->caps & ~DWCAPS_ALL) || !window)
               return DFB_INVARG;

          wd.caps = desc->caps;
     }

     if (desc->flags & DWDESC_PIXELFORMAT) {
          wd.pixelformat = desc->pixelformat;
          // GE Settging for YUV format (LIMITED_RANGE/FULL_RANGE)
          if (wd.pixelformat == DSPF_YUY2 ||
               wd.pixelformat == DSPF_YVYU ||
               wd.pixelformat == DSPF_UYVY ||
               wd.pixelformat == DSPF_VYUY) {
              wd.rgb2yuvMode = desc->rgb2yuvMode;
          }
     }

     if (desc->flags & DWDESC_COLORSPACE) 
          wd.colorspace = desc->colorspace;

     if (desc->flags & DWDESC_SURFACE_CAPS)
          wd.surface_caps = desc->surface_caps;

     if (desc->flags & DWDESC_PARENT) {
          ret = dfb_core_get_window( data->core, desc->parent_id, &parent );
          if (ret)
               goto out;

          wd.flags     |= DWDESC_PARENT;
          wd.parent_id  = desc->parent_id;
     }

     if (desc->flags & DWDESC_OPTIONS) {
          wd.flags   |= DWDESC_OPTIONS;
          wd.options  = desc->options;
     }

     if (desc->flags & DWDESC_STACKING) {
          wd.flags    |= DWDESC_STACKING;
          wd.stacking  = desc->stacking;
     }
     else
     {
         wd.flags       |= DWDESC_STACKING;
         layer_id       = (int)dfb_layer_id_translated( data->layer );
         wd.stacking    = (DFBWindowStackingClass)dfb_config_get_layer_stacking(layer_id);

         // direct_log_printf(NULL,"%s[pid = %d][%d,%d] \n",__FUNCTION__,getpid(),layer_id,wd.stacking);
     }

     if (desc->flags & DWDESC_RESOURCE_ID) {
          wd.flags       |= DWDESC_RESOURCE_ID;
          wd.resource_id  = desc->resource_id;
     }

     if (desc->flags & DWDESC_TOPLEVEL_ID) {
          ret = dfb_core_get_window( data->core, desc->toplevel_id, &toplevel );
          if (ret)
               goto out;

          wd.flags       |= DWDESC_TOPLEVEL_ID;
          wd.toplevel_id  = desc->toplevel_id;
     }


     ret = CoreLayerContext_CreateWindow( data->context, &wd, parent, toplevel, &w );
     if (ret)
          goto out;

     DIRECT_ALLOCATE_INTERFACE( *window, IDirectFBWindow );

     ret = IDirectFBWindow_Construct( *window, w, data->layer, data->core, data->idirectfb );

out:
     if (toplevel)
          dfb_window_unref( toplevel );

     if (parent)
          dfb_window_unref( parent );

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_GetWindow( IDirectFBDisplayLayer  *thiz,
                                 DFBWindowID             id,
                                 IDirectFBWindow       **window )
{
     CoreWindow *w;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!window)
          return DFB_INVARG;
   
      //remove for now
     //if (data->level == DLSCL_SHARED)
     //    return DFB_ACCESSDENIED;

     w = dfb_layer_context_find_window( data->context, id );
     if (!w)
          return DFB_IDNOTFOUND;

     DIRECT_ALLOCATE_INTERFACE( *window, IDirectFBWindow );

     return IDirectFBWindow_Construct( *window, w, data->layer, data->core, data->idirectfb );
}

static DFBResult
IDirectFBDisplayLayer_EnableCursor( IDirectFBDisplayLayer *thiz, int enable )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return CoreWindowStack_CursorEnable( data->stack, enable );
}

static DFBResult
IDirectFBDisplayLayer_GetCursorPosition( IDirectFBDisplayLayer *thiz,
                                         int *x, int *y )
{
     DFBResult ret;
     DFBPoint  point;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!x && !y)
          return DFB_INVARG;

     ret = CoreWindowStack_CursorGetPosition( data->stack, &point );
     if (ret)
          return ret;

     if (x)
          *x = point.x;

     if (y)
          *y = point.y;

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_WarpCursor( IDirectFBDisplayLayer *thiz, int x, int y )
{
     DFBPoint point = { x, y };
     DFBResult ret;
     
     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     ret = CoreWindowStack_CursorWarp( data->stack, &point );

     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DISPLAYLAYER_WARPCURSOR);

      return ret;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorAcceleration( IDirectFBDisplayLayer *thiz,
                                             int                    numerator,
                                             int                    denominator,
                                             int                    threshold )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (numerator < 0  ||  denominator < 1  ||  threshold < 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return CoreWindowStack_CursorSetAcceleration( data->stack, numerator,
                                                   denominator, threshold );
}

static DFBResult
IDirectFBDisplayLayer_SetCursorShape( IDirectFBDisplayLayer *thiz,
                                      IDirectFBSurface      *shape,
                                      int                    hot_x,
                                      int                    hot_y )
{
     DFBPoint               hotspot = { hot_x, hot_y };
     IDirectFBSurface_data *shape_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!shape)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     shape_data = (IDirectFBSurface_data*)shape->priv;

     if (hot_x < 0  ||
         hot_y < 0  ||
         hot_x >= shape_data->surface->config.size.w  ||
         hot_y >= shape_data->surface->config.size.h)
          return DFB_INVARG;

     return CoreWindowStack_CursorSetShape( data->stack,
                                            shape_data->surface,
                                            &hotspot );
}

static DFBResult
IDirectFBDisplayLayer_SetCursorOpacity( IDirectFBDisplayLayer *thiz,
                                        u8                     opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return CoreWindowStack_CursorSetOpacity( data->stack, opacity );
}

static DFBResult
IDirectFBDisplayLayer_GetColorAdjustment( IDirectFBDisplayLayer *thiz,
                                          DFBColorAdjustment    *adj )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!adj)
          return DFB_INVARG;

     return dfb_layer_context_get_coloradjustment( data->context, adj );
}

static DFBResult
IDirectFBDisplayLayer_SetColorAdjustment( IDirectFBDisplayLayer    *thiz,
                                          const DFBColorAdjustment *adj )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!adj || (adj->flags & ~DCAF_ALL))
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     if (!adj->flags)
          return DFB_OK;

     return dfb_layer_context_set_coloradjustment( data->context, adj );
}

static DFBResult
IDirectFBDisplayLayer_SetStereoDepth( IDirectFBDisplayLayer    *thiz,
                                      bool                      follow_video,
                                      int                       z )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!follow_video && (z < -DLSO_FIXED_LIMIT || z > DLSO_FIXED_LIMIT))
          return DFB_INVARG;

     if (!(data->context->config.options & DLOP_LR_MONO) &&
         !(data->context->config.options & DLOP_STEREO)) {
          return DFB_INVARG;
     }

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_stereo_depth( data->context, follow_video, z );
}

static DFBResult
IDirectFBDisplayLayer_GetStereoDepth( IDirectFBDisplayLayer *thiz,
                                      bool                  *follow_video,
                                      int                   *z )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!(data->context->config.options & DLOP_LR_MONO) &&
         !(data->context->config.options & DLOP_STEREO)) {
          return DFB_INVARG;
     }

     if (!z || !follow_video)
          return DFB_INVARG;

     return dfb_layer_context_get_stereo_depth( data->context, follow_video, z );
}

static DFBResult
IDirectFBDisplayLayer_WaitForSync( IDirectFBDisplayLayer *thiz )
{
     DFBResult         ret = DFB_OK;
     
     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     ret = dfb_layer_wait_vsync( data->layer );

     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DISPLAYLAYER_WAITFORSYNC);

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_GetSourceDescriptions( IDirectFBDisplayLayer            *thiz,
                                             DFBDisplayLayerSourceDescription *ret_descriptions )
{
     int i;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!ret_descriptions)
          return DFB_INVARG;

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SOURCES ))
          return DFB_UNSUPPORTED;

     for (i=0; i<data->desc.sources; i++)
          dfb_layer_get_source_info( data->layer, i, &ret_descriptions[i] );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SwitchContext( IDirectFBDisplayLayer *thiz,
                                     DFBBoolean             exclusive )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!exclusive && data->level == DLSCL_EXCLUSIVE) {
          DFBResult         ret;
          CoreLayerContext *context;

          ret = CoreLayer_GetPrimaryContext( data->layer, false, &context );
          if (ret)
               return ret;

          dfb_layer_activate_context( data->layer, context );

          dfb_layer_context_unref( context );
     }
     else
         dfb_layer_activate_context( data->layer, data->context );

     data->switch_exclusive = exclusive;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetRotation( IDirectFBDisplayLayer *thiz,
                                   int                    rotation )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_rotation( data->context, rotation );
}

static DFBResult
IDirectFBDisplayLayer_GetRotation( IDirectFBDisplayLayer *thiz,
                                   int                   *ret_rotation )
{
     CoreLayerContext *context;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!ret_rotation)
          return DFB_INVARG;

     context = data->context;
     D_MAGIC_ASSERT( context, CoreLayerContext );

     /* Lock the context. */
     if (dfb_layer_context_lock( context ))
          return DFB_FUSION;

     *ret_rotation = context->rotation;

     /* Unlock the context. */
     dfb_layer_context_unlock( context );

     return DFB_OK;
}

typedef struct {
     unsigned long    resource_id;
     CoreWindow      *window;
} IDirectFBDisplayLayer_GetWindowByResourceID_Context;

static DFBEnumerationResult
IDirectFBDisplayLayer_GetWindowByResourceID_WindowCallback( CoreWindow *window,
                                                            void       *_ctx )
{
     IDirectFBDisplayLayer_GetWindowByResourceID_Context *ctx = _ctx;

     if (window->surface) {
          if (window->surface->resource_id == ctx->resource_id) {
               ctx->window = window;

               return DFENUM_CANCEL;
          }
     }

     return DFENUM_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetWindowByResourceID( IDirectFBDisplayLayer  *thiz,
                                             unsigned long           resource_id,
                                             IDirectFBWindow       **ret_window )
{
     DFBResult                                            ret;
     CoreLayerContext                                    *context;
     CoreWindowStack                                     *stack;
     IDirectFBDisplayLayer_GetWindowByResourceID_Context  ctx;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!ret_window)
          return DFB_INVARG;

     context = data->context;
     D_MAGIC_ASSERT( context, CoreLayerContext );

     stack = context->stack;
     D_ASSERT( stack != NULL );

     ctx.resource_id = resource_id;
     ctx.window      = NULL;

     ret = dfb_layer_context_lock( context );
     if (ret)
         return ret;

     ret = dfb_wm_enum_windows( stack, IDirectFBDisplayLayer_GetWindowByResourceID_WindowCallback, &ctx );
     if (ret == DFB_OK) {
          if (ctx.window) {
               IDirectFBWindow *window;

               ret = dfb_window_ref( ctx.window );
               if (ret == DFB_OK) {
                    DIRECT_ALLOCATE_INTERFACE( window, IDirectFBWindow );

                    ret = IDirectFBWindow_Construct( window, ctx.window, data->layer, data->core, data->idirectfb );
                    if (ret == DFB_OK)
                         *ret_window = window;
               }
          }
          else
               ret = DFB_IDNOTFOUND;
     }

     dfb_layer_context_unlock( context );

     return ret;
}


DFBResult
IDirectFBDisplayLayer_SetSurface( IDirectFBDisplayLayer  *thiz,
                                        IDirectFBSurface                     *surface_interface)
{
    DFBResult                                            ret;
    CoreLayerContext                                    *context;
    CoreSurface                                         *surface;
    IDirectFBSurface_data                               *surface_data;

    DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer);

    surface_data = surface_interface->priv;

    
    context = data->context;
    D_MAGIC_ASSERT( context, CoreLayerContext );

    surface = surface_data->surface;

    if (!surface)
         return DFB_DESTROYED;

    
    return dfb_layer_context_set_surface(context, surface);
     
}


static DFBResult
IDirectFBDisplayLayer_SetHVMirrorEnable( IDirectFBDisplayLayer *thiz,
                                bool                    HEnable,
                                bool                    VEnable)
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_set_hv_mirror_enable( data->layer, HEnable, VEnable);
}


static DFBResult
IDirectFBDisplayLayer_SetGOPOutputRatio( IDirectFBDisplayLayer *thiz,
                                bool                    bEnable)
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     return dfb_layer_Set_GOP_Output_Ratio( data->layer, bEnable);
}


/*
Wrap the IDirectFBDisplayLayer_xxx to _IDirectFBDisplayLayer_xxx for safe call
*/

static DirectResult
_IDirectFBDisplayLayer_AddRef( IDirectFBDisplayLayer *thiz )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_AddRef(thiz);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DirectResult
_IDirectFBDisplayLayer_Release( IDirectFBDisplayLayer *thiz )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_Release(thiz);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetID( IDirectFBDisplayLayer *thiz,
                             DFBDisplayLayerID     *id )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetID(thiz,id);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_GetDescription( IDirectFBDisplayLayer      *thiz,
                                      DFBDisplayLayerDescription *desc )
{

  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetDescription(thiz,desc);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_GetSurface( IDirectFBDisplayLayer  *thiz,
                                  IDirectFBSurface      **interface )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetSurface(thiz,interface);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);

  if (ret == DFB_OK)
     DBG_LAYER_MSG("[DFB] [PID=%d] %s , Get surface success. \n", getpid(), __FUNCTION__);
  else
     DBG_LAYER_MSG("[DFB] [PID=%d] %s , Get surface fail. \n", getpid(), __FUNCTION__);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetScreen( IDirectFBDisplayLayer  *thiz,
                                 IDirectFBScreen       **interface )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetScreen(thiz,interface);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetCooperativeLevel( IDirectFBDisplayLayer           *thiz,
                                           DFBDisplayLayerCooperativeLevel  level )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetCooperativeLevel(thiz,level);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

#if 0
static DFBResult
_IDirectFBDisplayLayer_SetAlphaMode(IDirectFBDisplayLayer *thiz, DFBDisplayLayerOptions flag)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetAlphaMode(thiz,flag);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;


}
#endif

static DFBResult
_IDirectFBDisplayLayer_SetOpacity( IDirectFBDisplayLayer *thiz,
                                  u8                     opacity )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetOpacity(thiz,opacity);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_GetCurrentOutputField( IDirectFBDisplayLayer *thiz, int *field )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetCurrentOutputField(thiz,field);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetFieldParity( IDirectFBDisplayLayer *thiz, int field )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetFieldParity(thiz,field);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetClipRegions( IDirectFBDisplayLayer *thiz,
                                      const DFBRegion       *regions,
                                      int                    num_regions,
                                      DFBBoolean             positive )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetClipRegions(thiz,regions,num_regions,positive);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetSourceRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetSourceRectangle(thiz,x,y,width,height);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetScreenLocation( IDirectFBDisplayLayer *thiz,
                                         float                  x,
                                         float                  y,
                                         float                  width,
                                         float                  height )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetScreenLocation(thiz,x,y,width,height);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetScreenPosition( IDirectFBDisplayLayer *thiz,
                                         int                    x,
                                         int                    y )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetScreenPosition(thiz,x,y);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetScreenRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetScreenRectangle(thiz,x,y,width,height);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetSrcColorKey( IDirectFBDisplayLayer *thiz,
                                      u8                     r,
                                      u8                     g,
                                      u8                     b )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetSrcColorKey(thiz,r,g,b);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetDstColorKey( IDirectFBDisplayLayer *thiz,
                                      u8                     r,
                                      u8                     g,
                                      u8                     b )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetDstColorKey(thiz,r,g,b);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetLevel( IDirectFBDisplayLayer *thiz,
                                int                   *level )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetLevel(thiz,level);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetLevel( IDirectFBDisplayLayer *thiz,
                                int                    level )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetLevel(thiz,level);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

#if 0
static DFBResult
_IDirectFBDisplayLayer_SetForceWrite( IDirectFBDisplayLayer *thiz,
                                bool                    force_write )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetForceWrite(thiz,force_write);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}
#endif

static DFBResult
_IDirectFBDisplayLayer_SetHVMirrorEnable( IDirectFBDisplayLayer *thiz,
                                bool                    HEnable,
                                bool                    VEnable)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetHVMirrorEnable(thiz, HEnable, VEnable);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

#if 0
static DFBResult
_IDirectFBDisplayLayer_SetLBCoupleEnable( IDirectFBDisplayLayer *thiz,
                                bool                    LBCoupleEnable)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetLBCoupleEnable(thiz, LBCoupleEnable);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetGOPDstByPassEnable( IDirectFBDisplayLayer *thiz,
                                bool                    ByPassEnable)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetGOPDstByPassEnable(thiz, ByPassEnable);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}
#endif


static DFBResult
_IDirectFBDisplayLayer_GetConfiguration( IDirectFBDisplayLayer *thiz,
                                        DFBDisplayLayerConfig *config )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetConfiguration(thiz,config);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_TestConfiguration( IDirectFBDisplayLayer       *thiz,
                                         const DFBDisplayLayerConfig *config,
                                         DFBDisplayLayerConfigFlags  *failed )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_TestConfiguration(thiz,config,failed);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetConfiguration( IDirectFBDisplayLayer       *thiz,
                                        const DFBDisplayLayerConfig *config )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetConfiguration(thiz,config);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetBackgroundMode( IDirectFBDisplayLayer         *thiz,
                                         DFBDisplayLayerBackgroundMode  background_mode )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetBackgroundMode(thiz,background_mode);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

#if 0
static DFBResult
_IDirectFBDisplayLayer_SetDeskDisplayMode( IDirectFBDisplayLayer         *thiz,
                                         DFBDisplayLayerDeskTopDisplayMode  disktop_mode )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetDeskDisplayMode(thiz,disktop_mode);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_QueryState( IDirectFBDisplayLayer         *thiz,
                                   DFBDisplayLayerQueryID        id,
                                   void                          *pstate)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_QueryState(thiz, id,pstate);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}
#endif

static DFBResult
_IDirectFBDisplayLayer_SetBackgroundImage( IDirectFBDisplayLayer *thiz,
                                          IDirectFBSurface      *surface )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetBackgroundImage(thiz,surface);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetBackgroundColor( IDirectFBDisplayLayer *thiz,
                                          u8 r, u8 g, u8 b, u8 a )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetBackgroundColor(thiz,r,g,b,a);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_CreateWindow( IDirectFBDisplayLayer       *thiz,
                                    const DFBWindowDescription  *desc,
                                    IDirectFBWindow            **window )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_CreateWindow(thiz,desc,window);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetWindow( IDirectFBDisplayLayer  *thiz,
                                 DFBWindowID             id,
                                 IDirectFBWindow       **window )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetWindow(thiz,id,window);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_EnableCursor( IDirectFBDisplayLayer *thiz, int enable )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_EnableCursor(thiz,enable);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_GetCursorPosition( IDirectFBDisplayLayer *thiz,
                                         int *x, int *y )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetCursorPosition(thiz,x,y);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

#if 0
static DFBResult
_IDirectFBDisplayLayer_MoveCursorTo( IDirectFBDisplayLayer *thiz,
                                         int x, int y )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_MoveCursorTo(thiz,x,y);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetActiveWindowNum( IDirectFBDisplayLayer *thiz,
                                         int *ret_num)
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetActiveWindowNum(thiz,ret_num);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}
#endif

static DFBResult
_IDirectFBDisplayLayer_WarpCursor( IDirectFBDisplayLayer *thiz, int x, int y )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_WarpCursor(thiz,x,y);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetCursorAcceleration( IDirectFBDisplayLayer *thiz,
                                             int                    numerator,
                                             int                    denominator,
                                             int                    threshold )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetCursorAcceleration(thiz,numerator,denominator,threshold);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;


}

static DFBResult
_IDirectFBDisplayLayer_SetCursorShape( IDirectFBDisplayLayer *thiz,
                                      IDirectFBSurface      *shape,
                                      int                    hot_x,
                                      int                    hot_y )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetCursorShape(thiz,shape,hot_x,hot_y);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetCursorOpacity( IDirectFBDisplayLayer *thiz,
                                        u8                     opacity )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetCursorOpacity(thiz,opacity);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetColorAdjustment( IDirectFBDisplayLayer *thiz,
                                          DFBColorAdjustment    *adj )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetColorAdjustment(thiz,adj);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetColorAdjustment( IDirectFBDisplayLayer    *thiz,
                                          const DFBColorAdjustment *adj )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetColorAdjustment(thiz,adj);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_WaitForSync( IDirectFBDisplayLayer *thiz )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_WaitForSync(thiz);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_GetSourceDescriptions( IDirectFBDisplayLayer            *thiz,
                                             DFBDisplayLayerSourceDescription *ret_descriptions )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetSourceDescriptions(thiz,ret_descriptions);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SwitchContext( IDirectFBDisplayLayer *thiz,
                                     DFBBoolean             exclusive )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SwitchContext(thiz,exclusive);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;
}

static DFBResult
_IDirectFBDisplayLayer_SetRotation( IDirectFBDisplayLayer *thiz,
                                   int                    rotation )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetRotation(thiz,rotation);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_GetRotation( IDirectFBDisplayLayer *thiz,
                                   int                   *ret_rotation )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_GetRotation(thiz,ret_rotation);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

static DFBResult
_IDirectFBDisplayLayer_SetGOPOutputRatio( IDirectFBDisplayLayer *thiz,
                                   bool                   bEnable )
{
  DFBResult ret = DFB_OK;
  DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
  ret = IDirectFBDisplayLayer_SetGOPOutputRatio(thiz, bEnable);
  DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
  return ret;

}

DFBResult
IDirectFBDisplayLayer_Construct( IDirectFBDisplayLayer *thiz,
                                 CoreLayer             *layer,
                                 CoreDFB               *core,
                                 IDirectFB             *idirectfb )
{
     DFBResult         ret;
     CoreLayerContext *context;
     CoreLayerRegion  *region;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBDisplayLayer)

     ret = CoreLayer_GetPrimaryContext( layer, true, &context );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz )
          return ret;
     }

     ret = CoreLayerContext_GetPrimaryRegion( context, true, &region );
     if (ret) {
          dfb_layer_context_unref( context );
          DIRECT_DEALLOCATE_INTERFACE( thiz )
          return ret;
     }

     data->ref              = 1;
     data->core             = core;
     data->idirectfb        = idirectfb;
     data->screen           = dfb_layer_screen( layer );
     data->layer            = layer;
     data->context          = context;
     data->region           = region;
     data->stack            = dfb_layer_context_windowstack( context );
     data->switch_exclusive = DFB_TRUE;

     dfb_layer_get_description( data->layer, &data->desc );

     thiz->AddRef                = _IDirectFBDisplayLayer_AddRef;
     thiz->Release               = _IDirectFBDisplayLayer_Release;
     thiz->GetID                 = _IDirectFBDisplayLayer_GetID;
     thiz->GetDescription        = _IDirectFBDisplayLayer_GetDescription;
     thiz->GetSurface            = _IDirectFBDisplayLayer_GetSurface;
     thiz->GetScreen             = _IDirectFBDisplayLayer_GetScreen;
     thiz->SetCooperativeLevel   = _IDirectFBDisplayLayer_SetCooperativeLevel;
     thiz->SetStretchMode        = _IDirectFBDisplayLayer_SetStretchMode;
     thiz->SetOpacity            = _IDirectFBDisplayLayer_SetOpacity;
     thiz->GetCurrentOutputField = _IDirectFBDisplayLayer_GetCurrentOutputField;
     thiz->SetSourceRectangle    = _IDirectFBDisplayLayer_SetSourceRectangle;
     thiz->SetScreenLocation     = _IDirectFBDisplayLayer_SetScreenLocation;
     thiz->SetSrcColorKey        = _IDirectFBDisplayLayer_SetSrcColorKey;
     thiz->SetDstColorKey        = _IDirectFBDisplayLayer_SetDstColorKey;
     thiz->GetLevel              = _IDirectFBDisplayLayer_GetLevel;
     thiz->SetLevel              = _IDirectFBDisplayLayer_SetLevel;
     thiz->GetConfiguration      = _IDirectFBDisplayLayer_GetConfiguration;
     thiz->TestConfiguration     = _IDirectFBDisplayLayer_TestConfiguration;
     thiz->SetConfiguration      = _IDirectFBDisplayLayer_SetConfiguration;
     thiz->SetBackgroundMode     = _IDirectFBDisplayLayer_SetBackgroundMode;
     thiz->SetBackgroundColor    = _IDirectFBDisplayLayer_SetBackgroundColor;
     thiz->SetBackgroundImage    = _IDirectFBDisplayLayer_SetBackgroundImage;
     thiz->GetColorAdjustment    = _IDirectFBDisplayLayer_GetColorAdjustment;
     thiz->SetColorAdjustment    = _IDirectFBDisplayLayer_SetColorAdjustment;
     thiz->CreateWindow          = _IDirectFBDisplayLayer_CreateWindow;
     thiz->GetWindow             = _IDirectFBDisplayLayer_GetWindow;
     thiz->WarpCursor            = _IDirectFBDisplayLayer_WarpCursor;
     thiz->SetCursorAcceleration = _IDirectFBDisplayLayer_SetCursorAcceleration;
     thiz->EnableCursor          = _IDirectFBDisplayLayer_EnableCursor;
     thiz->GetCursorPosition     = _IDirectFBDisplayLayer_GetCursorPosition;
     thiz->SetCursorShape        = _IDirectFBDisplayLayer_SetCursorShape;
     thiz->SetCursorOpacity      = _IDirectFBDisplayLayer_SetCursorOpacity;
     thiz->SetFieldParity        = _IDirectFBDisplayLayer_SetFieldParity;
     thiz->SetClipRegions        = _IDirectFBDisplayLayer_SetClipRegions;
     thiz->WaitForSync           = _IDirectFBDisplayLayer_WaitForSync;
     thiz->GetSourceDescriptions = _IDirectFBDisplayLayer_GetSourceDescriptions;
     thiz->SetScreenPosition     = _IDirectFBDisplayLayer_SetScreenPosition;
     thiz->SetScreenRectangle    = _IDirectFBDisplayLayer_SetScreenRectangle;
     thiz->SwitchContext         = _IDirectFBDisplayLayer_SwitchContext;
     thiz->SetRotation           = _IDirectFBDisplayLayer_SetRotation;
     thiz->GetRotation           = _IDirectFBDisplayLayer_GetRotation;
     thiz->GetWindowByResourceID = IDirectFBDisplayLayer_GetWindowByResourceID;
     thiz->SetStereoDepth        = IDirectFBDisplayLayer_SetStereoDepth;
     thiz->GetStereoDepth        = IDirectFBDisplayLayer_GetStereoDepth;
     thiz->SetHVScale              = _IDirectFBDisplayLayer_SetHVScale;
     thiz->ForceFullUpdateWindowStack    = _IDirectFBDisplayLayer_ForceFullUpdateWindowStack;
     thiz->SetHVMirrorEnable              = _IDirectFBDisplayLayer_SetHVMirrorEnable;
     thiz->SetGOPOutputRatio                = _IDirectFBDisplayLayer_SetGOPOutputRatio;
     return DFB_OK;
}

