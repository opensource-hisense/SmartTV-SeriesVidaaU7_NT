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
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coretypes.h>

#include <core/fonts.h>
#include <core/gfxcard.h>
#include <core/layer_context.h>
#include <core/layer_region.h>
#include <core/state.h>
#include <core/surface.h>
#include <core/windows.h>
#include <core/windows_internal.h> /* FIXME */
#include <core/wm.h>

#include <core/CoreLayerRegion.h>
#include <core/CoreSurface.h>
#include <core/CoreWindow.h>

#include <display/idirectfbsurface.h>
#include <display/idirectfbsurface_window.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/thread.h>

#include <misc/util.h>

#include <gfx/util.h>


D_DEBUG_DOMAIN( Surface, "IDirectFBSurfaceW", "IDirectFBSurface_Window Interface" );

/**********************************************************************************************************************/

/*
 * private data struct of IDirectFBSurface_Window
 */
typedef struct {
     IDirectFBSurface_data base;   /* base Surface implementation */

     CoreWindow           *window; /* pointer to core data */

     DirectThread         *flip_thread; /* thread for non-flipping primary
                                           surfaces, to make changes visible */

//     CoreGraphicsSerial    serial;
} IDirectFBSurface_Window_data;

/**********************************************************************************************************************/

static void *Flipping_Thread( DirectThread *thread,
                              void         *arg );

/**********************************************************************************************************************/

static void
IDirectFBSurface_Window_Destruct( IDirectFBSurface *thiz )
{
     IDirectFBSurface_Window_data *data;

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     D_ASSERT( thiz != NULL );

     data = thiz->priv;

     if (data->flip_thread) {
          direct_thread_cancel( data->flip_thread );
          direct_thread_join( data->flip_thread );
          direct_thread_destroy( data->flip_thread );
     }

     dfb_window_unref( data->window );

     IDirectFBSurface_Destruct( thiz );
}

static DirectResult
IDirectFBSurface_Window_Release( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Window)

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     if (--data->base.ref == 0)
          IDirectFBSurface_Window_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Window_Flip( IDirectFBSurface    *thiz,
                              const DFBRegion     *region,
                              DFBSurfaceFlipFlags  flags )
{
     DFBRegion reg;
     static int reset = 0; //record continuous dump is terminated or not;

     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Window)

     D_DEBUG_AT( Surface, "%s( %p, %p, 0x%08x )\n", __FUNCTION__, thiz, region, flags );

     if (!data->base.surface) {
          printf("[DFB][%s %d] return DFB_DESTROYED pid=%d\n", __FUNCTION__, __LINE__, getpid());
          return DFB_DESTROYED;
     }

     if (data->base.surface->config.caps & DWCAPS_STEREO) {
          printf("[DFB][%s %d] return DFB_UNSUPPORTED pid=%d\n", __FUNCTION__, __LINE__, getpid());
          return DFB_UNSUPPORTED;
     }

     if (data->base.locked) {
          printf("[DFB][%s %d] return DFB_LOCKED pid=%d\n", __FUNCTION__, __LINE__, getpid());
          return DFB_LOCKED;
     }

     if (!data->base.area.current.w || !data->base.area.current.h ||
         (region && (region->x1 > region->x2 || region->y1 > region->y2))) {
          printf("[DFB][%s %d] w=%d h=%d pid=%d\n", __FUNCTION__, __LINE__, data->base.area.current.w, data->base.area.current.h, getpid());
          if (region) {
               printf("[DFB][%s %d] x1=%d x2=%d y1=%d y2=%d pid=%d\n", __FUNCTION__, __LINE__, region->x1, region->x2, region->y1, region->y2, getpid());
          }
          return DFB_INVAREA;
     }

     //[C_Skardin_Nagra_DVB_DDI_K2_TW] A/V sync 0718859: [display] OSD broken after change HDMI resolution
     if((data->window->config.options & DWOP_SCALE) && (data->window->config.options & DWOP_SCALE_WINDOW_AUTOFIT_LAYER))
      dfb_window_resize( data->window, data->window->stack->width,  data->window->stack->height );

     IDirectFBSurface_StopAll( &data->base );

     if (data->base.parent) {
          IDirectFBSurface_data *parent_data;

          DIRECT_INTERFACE_GET_DATA_FROM( data->base.parent, parent_data, IDirectFBSurface );

          /* Signal end of sequence of operations. */
          dfb_state_lock( &parent_data->state );
          dfb_state_stop_drawing( &parent_data->state );
          dfb_state_unlock( &parent_data->state );
     }



     dfb_region_from_rectangle( &reg, &data->base.area.current );

     if (region) {
          DFBRegion clip = DFB_REGION_INIT_TRANSLATED( region,
                                                       data->base.area.wanted.x,
                                                       data->base.area.wanted.y );

          if (!dfb_region_region_intersect( &reg, &clip )) {
               printf("[DFB][%s %d] reg:%4d,%4d-%4dx%4d clip:%4d,%4d-%4dx%4d pid=%d\n",
                __FUNCTION__, __LINE__, DFB_RECTANGLE_VALS_FROM_REGION( &reg ), DFB_RECTANGLE_VALS_FROM_REGION( &clip), getpid());
               return DFB_INVAREA;
          }
     }

     D_INFO(  "  %s -> FLIP %4d,%4d-%4dx%4d [pid=%d][tid=%d]\n", __FUNCTION__, DFB_RECTANGLE_VALS_FROM_REGION( &reg ), getpid(), syscall(SYS_gettid));


     if (flags & DSFLIP_PIPELINE) {
          dfb_gfxcard_wait_serial( &data->window->serial2 );

          data->window->serial2 = data->window->serial1;

          dfb_state_get_serial( &data->base.state, &data->window->serial1 );
     }

#ifdef CC_HW_WINDOW_SUPPORT
     /* Lock the context. */
     if (dfb_layer_context_lock( data->window->stack->context ))
          return DFB_FUSION;

     if (data->window->region && (data->window->region->context->config.buffermode == DLBM_WINDOWS))
#else
     if (data->window->region)
#endif      
     {
          CoreLayerRegion_FlipUpdate( data->window->region, &reg, flags );
#ifdef CC_HW_WINDOW_SUPPORT
          dfb_layer_context_unlock( data->window->stack->context );
#endif
     }
     else {
          if (data->base.surface->config.caps & DSCAPS_FLIPPING) {
               if (!(flags & DSFLIP_BLIT) && reg.x1 == 0 && reg.y1 == 0 &&
                   reg.x2 == data->base.surface->config.size.w  - 1 &&
                   reg.y2 == data->base.surface->config.size.h - 1)
               {
                    CoreSurface_Flip( data->base.surface, false );
               }
               else
                    dfb_back_to_front_copy( data->base.surface, &reg );
          }

#ifdef CC_HW_WINDOW_SUPPORT   
          if(data->window->stack->context->config.buffermode != DLBM_WINDOWS)
          {
                CoreWindow_Repaint( data->window, &reg, &reg, flags );
          }

          dfb_layer_context_unlock( data->window->stack->context );
#else
          CoreWindow_Repaint( data->window, &reg, &reg, flags );
#endif

     }

     if (!data->window->config.opacity && data->base.caps & DSCAPS_PRIMARY) {
          CoreWindowConfig config = { .opacity = 0xff };

          CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_OPACITY );
     }

     if(dfb_config->mst_flip_dump_path != NULL)
     {
         if((strcmp(dfb_config->mst_flip_dump_type,"all") == 0) && (access(dfb_config->mst_debug_directory_access, R_OK) == 0)){
             /* 'm' means measure flip time */
             if ('m' == dfb_config->mst_flip_dump_path)
             {
                 printf("\33[0;33;44m[DFB]\33[0m %s( %p, 0x%08x ) ,pid=%d \n",  __FUNCTION__, data->window->region, flags, getpid());
             }
             else
             {
                 char buf[64];
                 char *dir = dfb_config->mst_flip_dump_path;

                 snprintf( buf, sizeof(buf), "surface_win_front_pid%d", getpid());
                 dfb_surface_parallel_dump_buffer( data->window->surface, CSBR_FRONT, dir, buf, reset);
                 reset = 0; //record continuous dump is terminated or not;
             }
         }
         else
            reset = 1; //record continuous dump is terminated or not;
     }
     else
         reset = 1; //record continuous dump is terminated or not;
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_WINDOW_FLIP);
     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Window_FlipStereo( IDirectFBSurface    *thiz,
                                    const DFBRegion     *left_region,
                                    const DFBRegion     *right_region,
                                    DFBSurfaceFlipFlags  flags )
{
     DFBResult ret;
     DFBRegion l_reg, r_reg;
     DFBSurfaceStereoEye eye;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Window)

     D_DEBUG_AT( Surface, "%s( %p, %p, %p, 0x%08x )\n", __FUNCTION__, thiz, left_region, right_region, flags );

     if (!data->base.surface)
          return DFB_DESTROYED;

     if (!(data->base.surface->config.caps & DSCAPS_STEREO)) 
          return DFB_UNSUPPORTED;

     if (data->base.locked)
          return DFB_LOCKED;

     if (!data->base.area.current.w || !data->base.area.current.h ||
         (left_region && (left_region->x1 > left_region->x2 || left_region->y1 > left_region->y2)) ||
         (right_region && (right_region->x1 > right_region->x2 || right_region->y1 > right_region->y2)))
          return DFB_INVAREA;


     IDirectFBSurface_StopAll( &data->base );

     if (data->base.parent) {
          IDirectFBSurface_data *parent_data;

          DIRECT_INTERFACE_GET_DATA_FROM( data->base.parent, parent_data, IDirectFBSurface );

          /* Signal end of sequence of operations. */
          dfb_state_lock( &parent_data->state );
          dfb_state_stop_drawing( &parent_data->state );
          dfb_state_unlock( &parent_data->state );
     }

     dfb_region_from_rectangle( &l_reg, &data->base.area.current );
     dfb_region_from_rectangle( &r_reg, &data->base.area.current );

     if (left_region) {
          DFBRegion clip = DFB_REGION_INIT_TRANSLATED( left_region,
                                                       data->base.area.wanted.x,
                                                       data->base.area.wanted.y );

          if (!dfb_region_region_intersect( &l_reg, &clip ))
               return DFB_INVAREA;
     }
     if (right_region) {
          DFBRegion clip = DFB_REGION_INIT_TRANSLATED( right_region,
                                                       data->base.area.wanted.x,
                                                       data->base.area.wanted.y );

          if (!dfb_region_region_intersect( &r_reg, &clip ))
               return DFB_INVAREA;
     }

     D_DEBUG_AT( Surface, "  -> FLIP Left: %4d,%4d-%4dx%4d Right: %4d,%4d-%4dx%4d\n", 
          DFB_RECTANGLE_VALS_FROM_REGION( &l_reg ), DFB_RECTANGLE_VALS_FROM_REGION( &r_reg ) );


     if (flags & DSFLIP_PIPELINE) {
          dfb_gfxcard_wait_serial( &data->window->serial2 );

          data->window->serial2 = data->window->serial1;

          dfb_state_get_serial( &data->base.state, &data->window->serial1 );
     }

     if (data->window->region) {
          /* TODO STEREO: Add support for hardware windows. */
          /*dfb_layer_region_flip_update( data->window->region, &reg, flags );*/
          return DFB_UNSUPPORTED;
     }
     else {
          if (data->base.surface->config.caps & DSCAPS_FLIPPING) {
               if (!(flags & DSFLIP_BLIT)) {
                    if (l_reg.x1 == 0 && l_reg.y1 == 0 &&
                        l_reg.x2 == data->base.surface->config.size.w  - 1 &&
                        l_reg.y2 == data->base.surface->config.size.h - 1 &&
                        r_reg.x1 == 0 && r_reg.y1 == 0 &&
                        r_reg.x2 == data->base.surface->config.size.w  - 1 &&
                        r_reg.y2 == data->base.surface->config.size.h - 1)
                    {
                         ret = dfb_surface_lock( data->base.surface );
                         if (ret)
                              return ret;
     
                         dfb_surface_flip( data->base.surface, false );
     
                         dfb_surface_unlock( data->base.surface );
                    }
                    else {
                         /* Remember current stereo eye. */
                         eye = dfb_surface_get_stereo_eye(data->base.surface);

                         dfb_surface_set_stereo_eye(data->base.surface, DSSE_LEFT);
                         dfb_back_to_front_copy( data->base.surface, &l_reg );
                         dfb_surface_set_stereo_eye(data->base.surface, DSSE_RIGHT);
                         dfb_back_to_front_copy( data->base.surface, &r_reg );

                         /* Restore current stereo focus. */
                         dfb_surface_set_stereo_eye(data->base.surface, eye);

                    }
               }
          }
          dfb_window_repaint( data->window, &l_reg, &r_reg, flags );
     }

     if (!data->window->config.opacity && data->base.caps & DSCAPS_PRIMARY)
          dfb_window_set_opacity( data->window, 0xff );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Window_GetSubSurface( IDirectFBSurface    *thiz,
                                       const DFBRectangle  *rect,
                                       IDirectFBSurface   **surface )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Window)

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     /* Check arguments */
     if (!data->base.surface || !data->window || !data->window->surface)
          return DFB_DESTROYED;

     if (!surface)
          return DFB_INVARG;
          
     /* Allocate interface */
     DIRECT_ALLOCATE_INTERFACE( *surface, IDirectFBSurface );

     if (rect || data->base.limit_set) {
          DFBRectangle wanted, granted;
          
          /* Compute wanted rectangle */
          if (rect) {
               wanted = *rect;

               wanted.x += data->base.area.wanted.x;
               wanted.y += data->base.area.wanted.y;

               if (wanted.w <= 0 || wanted.h <= 0) {
                    wanted.w = 0;
                    wanted.h = 0;
               }
          }
          else {
               wanted = data->base.area.wanted;
          }
          
          /* Compute granted rectangle */
          granted = wanted;

          dfb_rectangle_intersect( &granted, &data->base.area.granted );
          
          /* Construct */
          ret = IDirectFBSurface_Window_Construct( *surface, thiz, &wanted, &granted,
                                                   data->window, data->base.caps |
                                                   DSCAPS_SUBSURFACE, data->base.core );
     }
     else {
          /* Construct */
          ret = IDirectFBSurface_Window_Construct( *surface, thiz, NULL, NULL,
                                                   data->window, data->base.caps |
                                                   DSCAPS_SUBSURFACE, data->base.core );
     }
     
     return ret;
}

/*
    Wrap the IDirectFBSurface_Window_xxxx to IDirectFBSurface_Window_xxxx for safe call
*/


static DirectResult
_IDirectFBSurface_Window_Release( IDirectFBSurface *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Window_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBSurface_Window_Flip( IDirectFBSurface    *thiz,
                              const DFBRegion     *region,
                              DFBSurfaceFlipFlags  flags )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Window_Flip(thiz,region,flags);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBSurface_Window_GetSubSurface( IDirectFBSurface    *thiz,
                                       const DFBRectangle  *rect,
                                       IDirectFBSurface   **surface )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Window_GetSubSurface(thiz,rect,surface);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}



DFBResult
IDirectFBSurface_Window_Construct( IDirectFBSurface       *thiz,
                                   IDirectFBSurface       *parent,
                                   DFBRectangle           *wanted,
                                   DFBRectangle           *granted,
                                   CoreWindow             *window,
                                   DFBSurfaceCapabilities  caps,
                                   CoreDFB                *core )
{
     DFBResult        ret;
     DFBInsets        insets;
     CoreWindowStack *stack;
     CoreSurface     *surface;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBSurface_Window)

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     stack = window->stack;
     D_MAGIC_ASSERT( stack, CoreWindowStack );

     dfb_layer_context_lock( stack->context );

     dfb_wm_get_insets( stack, window, &insets );
     
     dfb_layer_context_unlock( stack->context );

     ret = CoreWindow_GetSurface( window, &surface );
     if (ret)
          return ret;

     ret = IDirectFBSurface_Construct( thiz, parent, wanted, granted, &insets, surface, caps, core );

     dfb_surface_unref( surface );

     if (ret)
          return ret;

     if (dfb_window_ref( window )) {
          IDirectFBSurface_Destruct( thiz );
          return DFB_FAILURE;
     }

     data->window = window;

     /*
      * Create an auto flipping thread if the application
      * requested a (primary) surface that doesn't need to be flipped.
      * Window surfaces even need to be flipped when they are single buffered.
      */
     if (!(caps & DSCAPS_FLIPPING) && !(caps & DSCAPS_SUBSURFACE)) {
          if (dfb_config->autoflip_window)
               data->flip_thread = direct_thread_create( DTT_DEFAULT, Flipping_Thread, thiz, "Flipping" );
          else
               D_WARN( "Non-flipping window surface and no 'autoflip-window' option used" );
     }

     thiz->Release       = _IDirectFBSurface_Window_Release;
     thiz->Flip          = _IDirectFBSurface_Window_Flip;
     thiz->FlipStereo    =IDirectFBSurface_Window_FlipStereo;
     thiz->GetSubSurface = _IDirectFBSurface_Window_GetSubSurface;
     
     return DFB_OK;
}


/* file internal */

static void *
Flipping_Thread( DirectThread *thread,
                 void         *arg )
{
     IDirectFBSurface             *thiz = (IDirectFBSurface*) arg;
     IDirectFBSurface_Window_data *data = (IDirectFBSurface_Window_data*) thiz->priv;

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     while (data->base.surface && data->window->surface) {
          direct_thread_testcancel( thread );

          /*
           * OPTIMIZE: only call if surface has been touched in the meantime
           */
          thiz->Flip( thiz, NULL, 0 );

          direct_thread_sleep( 40000 );
     }

     return NULL;
}

