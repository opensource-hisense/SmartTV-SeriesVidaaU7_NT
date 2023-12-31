/*
   (c) Copyright 2001-2010  The world wide DirectFB Open Source Community (directfb.org)
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

#include <directfb.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/gfxcard.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/layers_internal.h>
#include <core/surface.h>

#include <core/CoreLayerRegion.h>

#include <gfx/util.h>
#include "mt53_fb.h"


D_DEBUG_DOMAIN( Core_Layers, "Core/Layers", "DirectFB Display Layer Core" );


static DFBResult region_buffer_unlock( CoreLayerRegion *region,
                                       bool unlockSurface );

static DFBResult region_buffer_lock( CoreLayerRegion       *region,
                                     CoreSurface           *surface,
                                     CoreSurfaceBufferRole  role );

static DFBResult set_region      ( CoreLayerRegion            *region,
                                   CoreLayerRegionConfig      *config,
                                   CoreLayerRegionConfigFlags  flags,
                                   CoreSurface                *surface );

static DFBResult realize_region  ( CoreLayerRegion            *region );

static DFBResult unrealize_region( CoreLayerRegion            *region );

/******************************************************************************/

static void
region_destructor( FusionObject *object, bool zombie, void *ctx )
{
     CoreLayerRegion  *region  = (CoreLayerRegion*) object;
     CoreLayerContext *context = region->context;
     CoreLayer        *layer   = dfb_layer_at( context->layer_id );
     CoreLayerShared  *shared  = layer->shared;

     D_DEBUG_AT( Core_Layers, "destroying region %p (%s, %dx%d, "
                 "%s, %s, %s, %s%s)\n", region, shared->description.name,
                 region->config.width, region->config.height,
                 D_FLAGS_IS_SET( region->state,
                                 CLRSF_CONFIGURED ) ? "configured" : "unconfigured",
                 D_FLAGS_IS_SET( region->state,
                                 CLRSF_ENABLED ) ? "enabled" : "disabled",
                 D_FLAGS_IS_SET( region->state,
                                 CLRSF_ACTIVE ) ? "active" : "inactive",
                 D_FLAGS_IS_SET( region->state,
                                 CLRSF_REALIZED ) ? "realized" : "not realized",
                 zombie ? " - ZOMBIE" : "" );

     /* Hide region etc. */
     if (D_FLAGS_IS_SET( region->state, CLRSF_ENABLED ))
     {
        dfb_layer_region_disable( region );
     }

     /* Remove the region from the context. */
     dfb_layer_context_remove_region( region->context, region );

     /* Throw away its surface. */
     if (region->surface) {
          /* Detach the global listener. */
          dfb_surface_detach_global( region->surface,
                                     &region->surface_reaction );

          /* Unlink from structure. */
          dfb_surface_unlink( &region->surface );
     }

     /* Unlink the context from the structure. */
     dfb_layer_context_unlink( &region->context );

     /* Free driver's region data. */
     if (region->region_data)
          SHFREE( shared->shmpool, region->region_data );

     CoreLayerRegion_Deinit_Dispatch(&region->call);
     
     /* Deinitialize the lock. */
     fusion_skirmish_destroy( &region->lock );

     /* Destroy the object. */
     fusion_object_destroy( object );
}

/******************************************************************************/

FusionObjectPool *
dfb_layer_region_pool_create( const FusionWorld *world )
{
     return fusion_object_pool_create( "Layer Region Pool",
                                       sizeof(CoreLayerRegion),
                                       sizeof(CoreLayerRegionNotification),
                                       region_destructor, NULL, world );
}

/******************************************************************************/

DFBResult
dfb_layer_region_create( CoreLayerContext  *context,
                         CoreLayerRegion  **ret_region )
{
     CoreLayer       *layer;
     CoreLayerShared *shared;
     CoreLayerRegion *region;

     D_ASSERT( context != NULL );
     D_ASSERT( ret_region != NULL );

     layer = dfb_layer_at( context->layer_id );

     shared = layer->shared;
     D_ASSERT( shared != NULL );

     /* Create the object. */
     region = dfb_core_create_layer_region( layer->core );
     if (!region)
          return DFB_FUSION;

     /* Link the context into the structure. */
     if (dfb_layer_context_link( &region->context, context )) {
          fusion_object_destroy( &region->object );
          return DFB_FUSION;
     }

     /* Initialize the lock. */
     if (fusion_skirmish_init( &region->lock, "Layer Region", dfb_core_world(layer->core) )) {
          dfb_layer_context_unlink( &region->context );
          fusion_object_destroy( &region->object );
          return DFB_FUSION;
     }

     /* Change global reaction lock. */
     fusion_object_set_lock( &region->object, &region->lock );

     region->state = CLRSF_FROZEN;

     if (shared->description.surface_accessor)
          region->surface_accessor = shared->description.surface_accessor;
     else
          region->surface_accessor = CSAID_LAYER0 + context->layer_id;

     if (dfb_config->osd_iommu)
          region->surface_accessor = CSAID_OSD;

     CoreLayerRegion_Init_Dispatch( layer->core, region, &region->call );

     /* Activate the object. */
     fusion_object_activate( &region->object );

     /* Add the region to the context. */
     dfb_layer_context_add_region( context, region );

     /* Return the new region. */
     *ret_region = region;

     return DFB_OK;
}

DFBResult
dfb_layer_region_activate( CoreLayerRegion *region )
{
     DFBResult ret;

     D_ASSERT( region != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     D_ASSUME( ! D_FLAGS_IS_SET( region->state, CLRSF_ACTIVE ) );

     if (D_FLAGS_IS_SET( region->state, CLRSF_ACTIVE )) {
          dfb_layer_region_unlock( region );
          return DFB_OK;
     }

     /* Realize the region if it's enabled. */
     // direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
     if (D_FLAGS_IS_SET( region->state, CLRSF_ENABLED )) {
          ret = realize_region( region );
          // direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
          if (ret) {
               dfb_layer_region_unlock( region );
               return ret;
          }
     }
     // direct_log_printf(NULL, "[%s,%d]\n",__FUNCTION__,__LINE__);

     /* Update the region's state. */
     D_FLAGS_SET( region->state, CLRSF_ACTIVE );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_deactivate( CoreLayerRegion *region )
{
     DFBResult ret;

     D_ASSERT( region != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     D_ASSUME( D_FLAGS_IS_SET( region->state, CLRSF_ACTIVE ) );

     if (! D_FLAGS_IS_SET( region->state, CLRSF_ACTIVE )) {
          dfb_layer_region_unlock( region );
          return DFB_OK;
     }

     /* Unrealize the region? */
     if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
          ret = unrealize_region( region );
          if (ret)
               return ret;
     }

     /* Update the region's state. */
     D_FLAGS_CLEAR( region->state, CLRSF_ACTIVE );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_enable( CoreLayerRegion *region )
{
     DFBResult ret;

     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     D_ASSUME( ! D_FLAGS_IS_SET( region->state, CLRSF_ENABLED ) );

     if (D_FLAGS_IS_SET( region->state, CLRSF_ENABLED )) {
          dfb_layer_region_unlock( region );
          return DFB_OK;
     }

     /* Realize the region if it's active. */
     // direct_log_printf(NULL,"%s[%d][%d][0x%x]\n",__FUNCTION__,__LINE__,region->context->layer_id,region->state);
     if (D_FLAGS_IS_SET( region->state, CLRSF_ACTIVE )) {
          ret = realize_region( region );
          // direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
          if (ret) {
               dfb_layer_region_unlock( region );
               return ret;
          }
     }

     /* Update the region's state. */
     D_FLAGS_SET( region->state, CLRSF_ENABLED );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_disable( CoreLayerRegion *region )
{
     DFBResult ret;

     D_ASSERT( region != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     D_ASSUME( D_FLAGS_IS_SET( region->state, CLRSF_ENABLED ) );

     if (! D_FLAGS_IS_SET( region->state, CLRSF_ENABLED )) {
          dfb_layer_region_unlock( region );
          return DFB_OK;
     }

     /* Unrealize the region? */
     if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
          ret = unrealize_region( region );
          if (ret)
               return ret;
     }

     /* Update the region's state. */
     D_FLAGS_CLEAR( region->state, CLRSF_ENABLED );
     D_FLAGS_SET( region->state, CLRSF_FROZEN );
     
     // direct_log_printf(NULL,"%s[%d]pid[%d][0x%x][%d]\n",__FUNCTION__,__LINE__,getpid(),region->state,region->context->layer_id);

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_set_surface( CoreLayerRegion *region,
                              CoreSurface     *surface )
{
     DFBResult ret;

     D_ASSERT( region != NULL );
     D_ASSERT( surface != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;
#ifdef CC_B2R44K2K_SUPPORT
	 if(region->context->layer_id == MT53_LAYER_VDP1)
	 {	      
		  D_FLAGS_SET( region->state, CLRSF_ENABLED );
		  D_FLAGS_SET( region->state, CLRSF_ACTIVE );
		  D_FLAGS_SET( region->state, CLRSF_REALIZED );
		  D_FLAGS_CLEAR( region->state, CLRSF_FROZEN );
	 }	 
#endif
     if (region->surface != surface) {
          /* Setup hardware for the new surface if the region is realized. */
          if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
               ret = set_region( region, &region->config, CLRCF_SURFACE | CLRCF_PALETTE, surface );
               if (ret) {
                    dfb_layer_region_unlock( region );
                    return ret;
               }
          }

          /* Throw away the old surface. */
          if (region->surface&&(!(region->surface->config.caps&DSCAPS_FROM_MPEG))) {
               /* Detach the global listener. */
               dfb_surface_detach_global( region->surface,
                                          &region->surface_reaction );

               /* Unlink surface from structure. */
               dfb_surface_unlink( &region->surface );
          }

          /* Take the new surface. */
          if (surface&&(!(surface->config.caps&DSCAPS_FROM_MPEG))) {
               /* Link surface into structure. */
               if (dfb_surface_link( &region->surface, surface )) {
                    D_WARN( "region lost it's surface" );
                    dfb_layer_region_unlock( region );
                    return DFB_FUSION;
               }

               /* Attach the global listener. */
               dfb_surface_attach_global( region->surface,
                                          DFB_LAYER_REGION_SURFACE_LISTENER,
                                          region, &region->surface_reaction );
          }

     }

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_get_surface( CoreLayerRegion  *region,
                              CoreSurface     **ret_surface )
{
     D_ASSERT( region != NULL );
     D_ASSERT( ret_surface != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     D_ASSUME( region->surface != NULL );

     /* Check for NULL surface. */
     if (!region->surface) {
          dfb_layer_region_unlock( region );
          return DFB_UNSUPPORTED;
     }

     /* Increase the surface's reference counter. */
     if (dfb_surface_ref( region->surface )) {
          dfb_layer_region_unlock( region );
          return DFB_FUSION;
     }

     /* Return the surface. */
     *ret_surface = region->surface;

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

#ifdef CC_HW_WINDOW_SUPPORT

DFBResult
dfb_layer_region_update( CoreLayerRegion *region )
{
     DFBResult ret = DFB_OK;
    /* Lock the region. */
     if (dfb_layer_region_lock( region ))
         return DFB_FUSION;

     if(D_FLAGS_IS_SET( region->state, CLRSF_REALIZED))
     {
          ret = set_region( region, &region->config, (CLRCF_ALL &(~(CLRCF_DEST|CLRCF_CUR_DEST))), region->surface );
     }
     
     dfb_layer_region_unlock( region );

     return ret;
}

DFBResult 
dfb_layer_region_ready(CoreLayerRegion *region)
{
    DFBResult                ret;
    CoreLayer               *layer;
    CoreLayerShared         *shared;
    const DisplayLayerFuncs *funcs;
    
    if (dfb_layer_region_lock( region ))
        return DFB_FUSION;

    D_FLAGS_CLEAR( region->state, CLRSF_FROZEN );
    
    // direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
    
    D_FLAGS_SET( region->state, CLRSF_ENABLED );

    D_FLAGS_SET( region->state, CLRSF_ACTIVE );

    D_DEBUG_AT( Core_Layers, "%s( %p )\n", __FUNCTION__, region );
    
    D_ASSERT( region != NULL );
    
    DFB_CORE_LAYER_REGION_CONFIG_DEBUG_AT( Core_Layers, &region->config );
    
    D_DEBUG_AT( Core_Layers, "  -> state    0x%08x\n", region->state );
    
    D_ASSERT( region->context != NULL );
    D_ASSERT( D_FLAGS_IS_SET( region->state, CLRSF_CONFIGURED ) );
    D_ASSERT( ! D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ) );
    
    layer = dfb_layer_at( region->context->layer_id );
    
    D_ASSERT( layer != NULL );
    D_ASSERT( layer->shared != NULL );
    D_ASSERT( layer->funcs != NULL );

    D_ASSERT( region->surface != NULL );
    
    shared = layer->shared;
    funcs  = layer->funcs;
    
    D_ASSERT( ! fusion_vector_contains( &shared->added_regions, region ) );
    
    /* Allocate the driver's region data. */
    if (funcs->RegionDataSize) {
         int size = funcs->RegionDataSize();
    
         if (size > 0) {
              region->region_data = SHCALLOC( shared->shmpool, 1, size );
              if (!region->region_data)
                   return D_OOSHM();
         }
    }
    
    D_DEBUG_AT( Core_Layers, "  => adding region to '%s'\n", shared->description.name );
    
    /* Add the region to the driver. */
    if (funcs->AddRegion) {
         ret = funcs->AddRegion( layer,
                                 layer->driver_data, layer->layer_data,
                                 region->region_data, &region->config );
         if (ret) {
              D_DERROR( ret, "Core/Layers: Could not add region!\n" );
    
              if (region->region_data) {
                   SHFREE( shared->shmpool, region->region_data );
                   region->region_data = NULL;
              }
    
              return ret;
         }
    }
    
    /* Add the region to the 'added' list. */
    fusion_vector_add( &shared->added_regions, region );
    
    /* Update the region's state. */
    D_FLAGS_SET( region->state, CLRSF_REALIZED );
    // direct_log_printf(NULL,"%s\n",__FUNCTION__);
    /* Initially setup hardware. */
    region_buffer_lock( region, region->surface, CSBR_FRONT );
    region_buffer_unlock(region, true);

    region_buffer_lock( region, region->surface, CSBR_BACK );
    region_buffer_unlock(region, true); 

    dfb_layer_region_unlock( region );

    return ret;
}

#endif

DFBResult
dfb_layer_region_flip_update( CoreLayerRegion     *region,
                              const DFBRegion     *update,
                              DFBSurfaceFlipFlags  flags )
{
     DFBResult                ret = DFB_OK;
     DFBRegion                unrotated;
     DFBRegion                rotated;
     CoreLayer               *layer;
     CoreLayerContext        *context;
     CoreSurface             *surface;
     const DisplayLayerFuncs *funcs;

     if (update)
          D_DEBUG_AT( Core_Layers,
                      "dfb_layer_region_flip_update( %p, %p, 0x%08x ) <- [%d, %d - %dx%d]\n",
                      region, update, flags, DFB_RECTANGLE_VALS_FROM_REGION( update ) );
     else
          D_DEBUG_AT( Core_Layers,
                      "dfb_layer_region_flip_update( %p, %p, 0x%08x )\n", region, update, flags );


     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     /* Check for stereo region */
     if (region->config.options & DLOP_STEREO) {
          ret = dfb_layer_region_flip_update_stereo( region, update, update, flags );
          dfb_layer_region_unlock( region );
          return ret;
     }

     D_ASSUME( region->surface != NULL );

     /* Check for NULL surface. */
     if (!region->surface) {
          D_DEBUG_AT( Core_Layers, "  -> No surface => no update!\n" );
          dfb_layer_region_unlock( region );
          return DFB_UNSUPPORTED;
     }

     context = region->context;
     surface = region->surface;
     layer   = dfb_layer_at( context->layer_id );

     D_ASSERT( layer->funcs != NULL );

     funcs = layer->funcs;

    /* Unfreeze region? */
    if( context &&(MT53_LAYER_VDP1 == context->layer_id ) && (!(D_FLAGS_IS_SET(region->state,CLRSF_ACTIVE ))) )
    {
        D_DEBUG_AT(Core_Layers, "[%s,%d,%d]\n",__FUNCTION__,__LINE__,context);
        dfb_layer_activate_context( layer,context);
    }
      
     if (D_FLAGS_IS_SET( region->state, CLRSF_FROZEN )) {
          D_FLAGS_CLEAR( region->state, CLRSF_FROZEN );
          if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
               ret = set_region( region, &region->config, CLRCF_ALL, surface );
               if (ret)
                    D_DERROR( ret, "Core/LayerRegion: set_region() in dfb_layer_region_flip_update() failed!\n" );
          }
          else if (D_FLAGS_ARE_SET( region->state, CLRSF_CONFIGURED | CLRSF_ACTIVE )) 
          {
               if(D_FLAGS_IS_SET( region->state, CLRSF_ENABLED ))
                {
                    ret = realize_region(region);
                }
               else
                {
                    ret = dfb_layer_region_enable( region );
                }
               D_DEBUG_AT(Core_Layers, "%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
               if (ret)
                    D_DERROR( ret, "Core/LayerRegion: realize_region() in dfb_layer_region_flip_update() failed!\n" );
          }

          if (ret) {
               dfb_layer_region_unlock( region );
               return ret;
          }
     }

     /* Depending on the buffer mode... */
     switch (region->config.buffermode) {
          case DLBM_TRIPLE:
          case DLBM_BACKVIDEO:
          case DLBM_WINDOWS:
               /* Check if simply swapping the buffers is possible... */
               if (!(flags & DSFLIP_BLIT) && !surface->rotation &&
                   (!update || (update->x1 == 0 &&
                                update->y1 == 0 &&
                                update->x2 == surface->config.size.w - 1 &&
                                update->y2 == surface->config.size.h - 1)))
               {
                    D_DEBUG_AT( Core_Layers, "  -> Going to swap buffers...\n" );

                    /* Use the driver's routine if the region is realized. */
#ifdef CC_HW_WINDOW_SUPPORT
                     if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )
                            && (region->context->cur_region == region))
#else
                     if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ))  
#endif
                     {
                         D_ASSUME( funcs->FlipRegion != NULL );

                         ret = region_buffer_lock( region, surface, CSBR_BACK );
                         if (ret) {
                              dfb_layer_region_unlock( region );
                              return ret;
                         }

                         D_DEBUG_AT( Core_Layers, "  -> Flipping region using driver...\n" );

                         if (funcs->FlipRegion)
                              ret = funcs->FlipRegion( layer,
                                                       layer->driver_data,
                                                       layer->layer_data,
                                                       region->region_data,
                                                       surface, flags, 
                                                       &region->left_buffer_lock,
                                                       NULL );

                         /* Unlock region buffer since the lock is no longer needed. */
                         region_buffer_unlock(region, true);
                    }
                    else {
                         D_DEBUG_AT( Core_Layers, "  -> Flipping region not using driver...\n" );

                         /* Just do the hardware independent work. */
                         dfb_surface_lock( surface );
                         dfb_surface_flip( surface, false );
                         dfb_surface_unlock( surface );
                    }
                    break;
               }

               /* fall through */

          case DLBM_BACKSYSTEM:
               D_DEBUG_AT( Core_Layers, "  -> Going to copy portion...\n" );

#ifdef CC_HW_WINDOW_SUPPORT
              if(0)
#else
              if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC)
#endif
              {
                    D_DEBUG_AT( Core_Layers, "  -> Waiting for VSync...\n" );

                    dfb_layer_wait_vsync( layer );
              }

               D_DEBUG_AT( Core_Layers, "  -> Copying content from back to front buffer...\n" );

               /* ...or copy updated contents from back to front buffer. */
               dfb_back_to_front_copy_rotation( surface, update, surface->rotation );

#ifdef CC_HW_WINDOW_SUPPORT
              if(0)
#else
              if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAIT)
#endif
               {
                    D_DEBUG_AT( Core_Layers, "  -> Waiting for VSync...\n" );

                    dfb_layer_wait_vsync( layer );
               }

               /* fall through */

          case DLBM_FRONTONLY:
               /* Tell the driver about the update if the region is realized. */
#ifdef CC_HW_WINDOW_SUPPORT
              if (funcs->UpdateRegion && D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )
               && (region->context->cur_region == region))
#else
              if (funcs->UpdateRegion && D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ))
#endif
              {
                 /* Lock region buffer before it is used. */
                 region_buffer_lock( region, surface, CSBR_FRONT );

                    if (surface) {
                         CoreSurfaceAllocation *allocation;

                         allocation = region->left_buffer_lock.allocation;
                         D_ASSERT( allocation != NULL );

                         /* If hardware has written or is writing... */
                         if (allocation->accessed[CSAID_GPU] & CSAF_WRITE) {
                              D_DEBUG_AT( Core_Layers, "  -> Waiting for pending writes...\n" );

                              /* ...wait for the operation to finish. */
                              if (!(flags & DSFLIP_PIPELINE))
                                   dfb_gfxcard_sync(); /* TODO: wait for serial instead */

                              // allocation->accessed[CSAID_GPU] &= ~CSAF_WRITE;
                         }

                         dfb_surface_lock( surface );
                         dfb_surface_allocation_update( allocation, CSAF_READ );
                         dfb_surface_unlock( surface );
                    }

                    D_DEBUG_AT( Core_Layers, "  -> Notifying driver about updated content...\n" );
                    
                    if( !update ) {
                         unrotated = DFB_REGION_INIT_FROM_RECTANGLE_VALS( 0, 0,
                                        region->config.width, region->config.height );
                         update    = &unrotated;
                    }
                    dfb_region_from_rotated( &rotated, update, &surface->config.size, surface->rotation );

                    ret = funcs->UpdateRegion( layer,
                                               layer->driver_data,
                                               layer->layer_data,
                                               region->region_data,
                                               surface,
                                               &rotated, &region->left_buffer_lock,
                                               NULL, NULL );

                    /* Unlock region buffer since the lock is no longer needed. */
                    region_buffer_unlock(region, true);
               }
               break;

          default:
               D_BUG("unknown buffer mode");
               ret = DFB_BUG;
     }

     D_DEBUG_AT( Core_Layers, "  -> done.\n" );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return ret;
}

DFBResult
dfb_layer_region_flip_update_stereo( CoreLayerRegion     *region,
                                     const DFBRegion     *left_update,
                                     const DFBRegion     *right_update,
                                     DFBSurfaceFlipFlags  flags )
{
     DFBResult                ret = DFB_OK;
     DFBRegion                unrotated;
     DFBRegion                left_rotated, right_rotated;
     CoreLayer               *layer;
     CoreLayerContext        *context;
     CoreSurface             *surface;
     const DisplayLayerFuncs *funcs;
     DFBSurfaceStereoEye      eye;

     D_DEBUG_AT( Core_Layers, "%s( %p, %p, %p, 0x%08x )\n", __FUNCTION__, region, left_update, right_update, flags );
     if (left_update)
          D_DEBUG_AT( Core_Layers, "Left: [%d, %d - %dx%d]\n", DFB_RECTANGLE_VALS_FROM_REGION( left_update ) );
     if (right_update)
          D_DEBUG_AT( Core_Layers, "Right: [%d, %d - %dx%d]\n", DFB_RECTANGLE_VALS_FROM_REGION( right_update ) );


     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     /* Check for stereo region */
     if (!(region->config.options & DLOP_STEREO)) {
          D_DEBUG_AT( Core_Layers, "  -> Not a stereo region!\n" );
          dfb_layer_region_unlock( region );
          return DFB_UNSUPPORTED;
     }

     D_ASSUME( region->surface != NULL );

     /* Check for NULL surface. */
     if (!region->surface) {
          D_DEBUG_AT( Core_Layers, "  -> No surface => no update!\n" );
          dfb_layer_region_unlock( region );
          return DFB_UNSUPPORTED;
     }

     context = region->context;
     surface = region->surface;
     layer   = dfb_layer_at( context->layer_id );

     D_ASSERT( layer->funcs != NULL );

     funcs = layer->funcs;

     /* Unfreeze region? */
     if (D_FLAGS_IS_SET( region->state, CLRSF_FROZEN )) {
          D_FLAGS_CLEAR( region->state, CLRSF_FROZEN );
          //direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
          if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
               ret = set_region( region, &region->config, CLRCF_ALL, surface );
               if (ret)
                    D_DERROR( ret, "Core/LayerRegion: set_region() in dfb_layer_region_flip_update() failed!\n" );
          }
          else if (D_FLAGS_ARE_SET( region->state, CLRSF_CONFIGURED | CLRSF_ACTIVE )) {
               ret = realize_region( region );
               //direct_log_printf(NULL,"%s[%d][0x%x]\n",__FUNCTION__,__LINE__,region->state);
               if (ret)
                    D_DERROR( ret, "Core/LayerRegion: realize_region() in dfb_layer_region_flip_update() failed!\n" );
          }

          if (ret) {
               dfb_layer_region_unlock( region );
               return ret;
          }
     }

     /* Depending on the buffer mode... */
     switch (region->config.buffermode) {
          case DLBM_TRIPLE:
          case DLBM_BACKVIDEO:
          case DLBM_WINDOWS:
               /* Check if simply swapping the buffers is possible... */
               if (!(flags & DSFLIP_BLIT) && !surface->rotation &&
                   ((!left_update && !right_update) || 
                    ((left_update->x1 == 0 &&
                      left_update->y1 == 0 &&
                      left_update->x2 == surface->config.size.w - 1 &&
                      left_update->y2 == surface->config.size.h - 1) &&
                     (right_update->x1 == 0 &&
                      right_update->y1 == 0 &&
                      right_update->x2 == surface->config.size.w - 1 &&
                      right_update->y2 == surface->config.size.h - 1))))
               {
                    D_DEBUG_AT( Core_Layers, "  -> Going to swap buffers...\n" );

                    /* Use the driver's routine if the region is realized. */
                    if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
                         D_ASSUME( funcs->FlipRegion != NULL );

                         ret = region_buffer_lock( region, surface, CSBR_BACK );
                         if (ret) {
                              dfb_layer_region_unlock( region );
                              return ret;
                         }

                         D_DEBUG_AT( Core_Layers, "  -> Flipping region using driver...\n" );

                         if (funcs->FlipRegion)
                              ret = funcs->FlipRegion( layer,
                                                       layer->driver_data,
                                                       layer->layer_data,
                                                       region->region_data,
                                                       surface, flags, 
                                                       &region->left_buffer_lock,
                                                       &region->right_buffer_lock);

                         /* Unlock region buffer since the lock is no longer needed. */
                         region_buffer_unlock(region, true);
                    }
                    else {
                         D_DEBUG_AT( Core_Layers, "  -> Flipping region not using driver...\n" );

                         /* Just do the hardware independent work. */
                         dfb_surface_lock( surface );
                         dfb_surface_flip( surface, false );
                         dfb_surface_unlock( surface );
                    }
                    break;
               }

               /* fall through */

          case DLBM_BACKSYSTEM:
               D_DEBUG_AT( Core_Layers, "  -> Going to copy portion...\n" );

               if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC) {
                    D_DEBUG_AT( Core_Layers, "  -> Waiting for VSync...\n" );

                    dfb_layer_wait_vsync( layer );
               }

               D_DEBUG_AT( Core_Layers, "  -> Copying content from back to front buffer...\n" );

               /* ...or copy updated contents from back to front buffer. */
               eye = dfb_surface_get_stereo_eye(surface);
               dfb_surface_set_stereo_eye(surface, DSSE_LEFT);
               dfb_back_to_front_copy_rotation( surface, left_update, surface->rotation );
               dfb_surface_set_stereo_eye(surface, DSSE_RIGHT);
               dfb_back_to_front_copy_rotation( surface, right_update, surface->rotation );
               dfb_surface_set_stereo_eye(surface, eye);

               if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAIT) {
                    D_DEBUG_AT( Core_Layers, "  -> Waiting for VSync...\n" );

                    dfb_layer_wait_vsync( layer );
               }

               /* fall through */

          case DLBM_FRONTONLY:
               /* Tell the driver about the update if the region is realized. */
               if (funcs->UpdateRegion && D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {

                    /* Lock region buffer before it is used. */
                    region_buffer_lock( region, surface, CSBR_FRONT );

                    if (surface) {
                         CoreSurfaceAllocation *left_allocation, *right_allocation;

                         left_allocation = region->left_buffer_lock.allocation;
                         right_allocation = region->right_buffer_lock.allocation;
                         D_ASSERT( left_allocation != NULL );
                         D_ASSERT( right_allocation != NULL );

                         /* If hardware has written or is writing... */
                         if ((left_allocation->accessed[CSAID_GPU] & CSAF_WRITE) ||
                             (right_allocation->accessed[CSAID_GPU] & CSAF_WRITE)) {
                              D_DEBUG_AT( Core_Layers, "  -> Waiting for pending writes...\n" );

                              /* ...wait for the operation to finish. */
                              if (!(flags & DSFLIP_PIPELINE))
                                   dfb_gfxcard_sync(); /* TODO: wait for serial instead */

                              //left_allocation->accessed[CSAID_GPU] &= ~CSAF_WRITE;
                              //right_allocation->accessed[CSAID_GPU] &= ~CSAF_WRITE;
                         }

                         dfb_surface_lock( surface );
                         dfb_surface_allocation_update( left_allocation, CSAF_READ );
                         dfb_surface_allocation_update( right_allocation, CSAF_READ );
                         dfb_surface_unlock( surface );
                    }

                    D_DEBUG_AT( Core_Layers, "  -> Notifying driver about updated content...\n" );
                    
                    if( !left_update ) {
                         unrotated      = DFB_REGION_INIT_FROM_RECTANGLE_VALS( 0, 0,
                                             region->config.width, region->config.height );
                         left_update    = &unrotated;
                    }
                    if( !right_update ) {
                         unrotated      = DFB_REGION_INIT_FROM_RECTANGLE_VALS( 0, 0,
                                             region->config.width, region->config.height );
                         right_update    = &unrotated;
                    }
                    dfb_region_from_rotated( &left_rotated, left_update, &surface->config.size, surface->rotation );
                    dfb_region_from_rotated( &right_rotated, right_update, &surface->config.size, surface->rotation );

                    ret = funcs->UpdateRegion( layer,
                                               layer->driver_data,
                                               layer->layer_data,
                                               region->region_data,
                                               surface, 
                                               &left_rotated, &region->left_buffer_lock,
                                               &right_rotated, &region->right_buffer_lock );

                    /* Unlock region buffer since the lock is no longer needed. */
                    region_buffer_unlock(region, true);
               }
               break;

          default:
               D_BUG("unknown buffer mode");
               ret = DFB_BUG;
     }

     D_DEBUG_AT( Core_Layers, "  -> done.\n" );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return ret;
}

DFBResult
dfb_layer_region_set_configuration( CoreLayerRegion            *region,
                                    CoreLayerRegionConfig      *config,
                                    CoreLayerRegionConfigFlags  flags )
{
     DFBResult                ret;
     CoreLayer               *layer;
     const DisplayLayerFuncs *funcs;
     CoreLayerRegionConfig    new_config;

     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );
     D_ASSERT( config != NULL );
     D_ASSERT( config->buffermode != DLBM_UNKNOWN );
     D_ASSERT( (flags != CLRCF_NONE) || (region->state & CLRSF_CONFIGURED) );

     D_ASSUME( flags != CLRCF_NONE );
     D_ASSUME( ! (flags & ~CLRCF_ALL) );

     layer = dfb_layer_at( region->context->layer_id );

     D_ASSERT( layer != NULL );
     D_ASSERT( layer->funcs != NULL );
     D_ASSERT( layer->funcs->TestRegion != NULL );

     funcs = layer->funcs;

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;
     
     

     /* Full configuration supplied? */
     if (flags == CLRCF_ALL) {
          new_config = *config;
     }
     else {
          /* Use the current configuration. */
          new_config = region->config;

          /* Update each modified entry. */
          if (flags & CLRCF_WIDTH)
          {
           //direct_log_printf(NULL,"%s[%d][0x%x][%d,%d]\n",__FUNCTION__,__LINE__,flags,config->width,config->height);
           new_config.width = config->width;
          }

          if (flags & CLRCF_HEIGHT)
               new_config.height = config->height;

          if (flags & CLRCF_FORMAT)
#if defined(CC_PHX_CUST) || (defined(CC_S_PLATFORM)&&defined(CC_LMP_PLATFORM))
		  {
               if(config->format == DSPF_ARGB4444)
                   new_config.format = DSPF_ARGB;
               else
                   new_config.format = config->format;			  	
          }
#else
		new_config.format = config->format;	
#endif
          if (flags & CLRCF_COLORSPACE)
               new_config.colorspace = config->colorspace;

          if (flags & CLRCF_SURFACE_CAPS)
               new_config.surface_caps = config->surface_caps;

          if (flags & CLRCF_BUFFERMODE)
               new_config.buffermode = config->buffermode;

          if (flags & CLRCF_OPTIONS)
               new_config.options = config->options;

          if (flags & CLRCF_SOURCE_ID)
               new_config.source_id = config->source_id;

          if (flags & CLRCF_SOURCE)
               new_config.source = config->source;

          if (flags & CLRCF_DEST || flags & CLRCF_CUR_DEST)
               new_config.dest = config->dest;

          if (flags & CLRCF_OPACITY)
               new_config.opacity = config->opacity;

          if (flags & CLRCF_ALPHA_RAMP) {
               new_config.alpha_ramp[0] = config->alpha_ramp[0];
               new_config.alpha_ramp[1] = config->alpha_ramp[1];
               new_config.alpha_ramp[2] = config->alpha_ramp[2];
               new_config.alpha_ramp[3] = config->alpha_ramp[3];
          }

          if (flags & CLRCF_SRCKEY)
               new_config.src_key = config->src_key;

          if (flags & CLRCF_DSTKEY)
               new_config.dst_key = config->dst_key;

          if (flags & CLRCF_PARITY)
               new_config.parity = config->parity;

          if (flags & CLRCF_CLIPS) {
               new_config.clips     = config->clips;
               new_config.num_clips = config->num_clips;
               new_config.positive  = config->positive;
          }

        #ifdef CC_DFB_SUPPORT_VDP_LAYER
        if (flags & CLRCF_3D_CROP_OFFSET) {
            new_config.crop_bottom_offset = config->crop_bottom_offset;
            new_config.crop_top_offset = config->crop_top_offset;
            new_config.crop_left_offset = config->crop_left_offset;
            new_config.crop_right_offset = config->crop_right_offset;          
        }
        #endif        
     }

     /* Check if the new configuration is supported. */
     ret = funcs->TestRegion( layer, layer->driver_data, layer->layer_data,
                              &new_config, NULL );
     if (ret) {
          dfb_layer_region_unlock( region );
          return ret;
     }

     /* Check if the region should be frozen, thus requiring to apply changes explicitly. */
     if (flags & CLRCF_FREEZE)
          region->state |= CLRSF_FROZEN;

     /* Propagate new configuration to the driver if the region is realized. */
     if (D_FLAGS_IS_SET( region->state, CLRSF_REALIZED )) {
          ret = set_region( region, &new_config, flags, region->surface );
          if (ret) {
               dfb_layer_region_unlock( region );
               return ret;
          }
     }

     // direct_log_printf(NULL,"%s[%d][%d][0x%x][0x%x]\n",__FUNCTION__,__LINE__,getpid(),flags,region->state);

     /* Update the region's current configuration. */
     region->config = new_config;

     /* Update the region's state. */
     D_FLAGS_SET( region->state, CLRSF_CONFIGURED );

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DFBResult
dfb_layer_region_get_configuration( CoreLayerRegion       *region,
                                    CoreLayerRegionConfig *config )
{
     D_ASSERT( region != NULL );
     D_ASSERT( config != NULL );

     D_ASSERT( D_FLAGS_IS_SET( region->state, CLRSF_CONFIGURED ) );

     /* Lock the region. */
     if (dfb_layer_region_lock( region ))
          return DFB_FUSION;

     /* Return the current configuration. */
     *config = region->config;

     /* Unlock the region. */
     dfb_layer_region_unlock( region );

     return DFB_OK;
}

DirectResult
dfb_layer_region_lock( CoreLayerRegion *region )
{
     D_ASSERT( region != NULL );

     return fusion_skirmish_prevail( &region->lock );
}

DirectResult
dfb_layer_region_unlock( CoreLayerRegion *region )
{
     D_ASSERT( region != NULL );

     return fusion_skirmish_dismiss( &region->lock );
}

/******************************************************************************/

/*
 * listen to the layer's surface
 */
ReactionResult
_dfb_layer_region_surface_listener( const void *msg_data, void *ctx )
{
     CoreSurfaceNotificationFlags   flags;
     CoreSurface                   *surface;
     CoreLayer                     *layer;
     CoreLayerShared               *shared;
     const DisplayLayerFuncs       *funcs;
     const CoreSurfaceNotification *notification = msg_data;
     CoreLayerRegion               *region       = ctx;

     D_ASSERT( notification != NULL );
     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );

     D_DEBUG_AT( Core_Layers, "_dfb_layer_region_surface_listener( %p, %p ) <- 0x%08x\n",
                 notification, region, notification->flags );

     D_ASSERT( notification->surface != NULL );

     D_ASSUME( notification->surface == region->surface );

     if (notification->surface != region->surface)
          return RS_OK;

     layer = dfb_layer_at( region->context->layer_id );

     D_ASSERT( layer != NULL );
     D_ASSERT( layer->funcs != NULL );
     D_ASSERT( layer->funcs->SetRegion != NULL );
     D_ASSERT( layer->shared != NULL );

     funcs   = layer->funcs;
     shared  = layer->shared;

     flags   = notification->flags;
     surface = notification->surface;

     if (flags & CSNF_DESTROY) {
          D_WARN( "layer region surface destroyed" );
          region->surface = NULL;
          return RS_REMOVE;
     }

     if (dfb_layer_region_lock( region ))
          return RS_OK;

     if (D_FLAGS_ARE_SET( region->state, CLRSF_REALIZED | CLRSF_CONFIGURED ) &&
         !D_FLAGS_IS_SET( region->state, CLRSF_FROZEN ))
     {
          if (D_FLAGS_IS_SET( flags, CSNF_PALETTE_CHANGE | CSNF_PALETTE_UPDATE )) {
               if (surface->palette) {
                    /* Lock region buffer before it is used. */
                    region_buffer_lock( region, surface, CSBR_BACK );
                    D_ASSERT(region->left_buffer_lock.buffer != NULL);

                    funcs->SetRegion( layer,
                                      layer->driver_data, layer->layer_data,
                                      region->region_data, &region->config,
                                      CLRCF_PALETTE, surface, surface->palette,
                                      &region->left_buffer_lock,
                                      &region->right_buffer_lock );

                    /* Unlock region buffer since the lock is no longer needed. */
                    region_buffer_unlock(region, true);
               }
          }

          if ((flags & CSNF_FIELD) && funcs->SetInputField)
               funcs->SetInputField( layer,
                                     layer->driver_data, layer->layer_data,
                                     region->region_data, surface->field );

          if ((flags & CSNF_ALPHA_RAMP) && (shared->description.caps & DLCAPS_ALPHA_RAMP)) {
               region->config.alpha_ramp[0] = surface->alpha_ramp[0];
               region->config.alpha_ramp[1] = surface->alpha_ramp[1];
               region->config.alpha_ramp[2] = surface->alpha_ramp[2];
               region->config.alpha_ramp[3] = surface->alpha_ramp[3];

               /* Lock region buffer before it is used. */
               region_buffer_lock( region, surface, CSBR_BACK );
               D_ASSERT(region->left_buffer_lock.buffer != NULL);

               funcs->SetRegion( layer,
                                 layer->driver_data, layer->layer_data,
                                 region->region_data, &region->config,
                                 CLRCF_ALPHA_RAMP, surface, surface->palette,
                                 &region->left_buffer_lock,
                                 &region->right_buffer_lock );

               /* Unlock region buffer since the lock is no longer needed. */
               region_buffer_unlock(region, true);
          }
     }

     dfb_layer_region_unlock( region );

     return RS_OK;
}

/******************************************************************************/
/*
 * This function was added to avoid locking the region buffer for the lifetime
 * of the display layer and it now only needs to be locked whenever needed.
 * This allows more than a single process to lock the region context and
 * prevents them from deadlocking.
 */
static DFBResult
region_buffer_unlock( CoreLayerRegion *region,
                      bool             unlockSurface)
{
     D_ASSERT(region != NULL);

     D_DEBUG_AT( Core_Layers, "%s(): region=%p, lock buffer=%p\n", __FUNCTION__, (void *)region, (void *)region->left_buffer_lock.buffer );
     /* Unlock any previously locked buffer. */
     if (region->left_buffer_lock.buffer) {
          D_MAGIC_ASSERT( region->left_buffer_lock.buffer, CoreSurfaceBuffer );

          dfb_surface_unlock_buffer( region->left_buffer_lock.buffer->surface, &region->left_buffer_lock );
     }

     if (region->right_buffer_lock.buffer) {
          D_MAGIC_ASSERT( region->right_buffer_lock.buffer, CoreSurfaceBuffer );

          dfb_surface_unlock_buffer( region->right_buffer_lock.buffer->surface, &region->right_buffer_lock );
     }

     /* Unlock the surface Fusion skirmish. */
     if (unlockSurface) {
         if (dfb_surface_unlock( region->surface ))
              return DFB_FUSION;
     }

     return DFB_OK;
}

static DFBResult
region_buffer_lock( CoreLayerRegion       *region,
                    CoreSurface           *surface,
                    CoreSurfaceBufferRole  role )
{
     DFBResult              ret;
     CoreSurfaceBuffer     *buffer;
     CoreSurfaceAllocation *allocation;
     CoreLayerContext      *context;
     bool                   stereo;
     DFBSurfaceStereoEye    eye;

     D_ASSERT( region != NULL );
     D_MAGIC_ASSERT( surface, CoreSurface );

     context = region->context;
     D_MAGIC_ASSERT( context, CoreLayerContext );

     stereo = surface->config.caps & DSCAPS_STEREO;
	 
     /* First unlock any previously locked buffer. */
     if (region->left_buffer_lock.buffer) {
          D_MAGIC_ASSERT( region->left_buffer_lock.buffer, CoreSurfaceBuffer );

          dfb_surface_unlock_buffer( region->left_buffer_lock.buffer->surface, &region->left_buffer_lock );
     }
     if (stereo) {
          if (region->right_buffer_lock.buffer) {
               D_MAGIC_ASSERT( region->right_buffer_lock.buffer, CoreSurfaceBuffer );
     
               dfb_surface_unlock_buffer( region->right_buffer_lock.buffer->surface, &region->right_buffer_lock );
          }
     }

     if (dfb_surface_lock( surface ))
          return DFB_FUSION;

     /* Save current buffer focus. */
     eye = dfb_surface_get_stereo_eye(surface);

     dfb_surface_set_stereo_eye(surface, DSSE_LEFT);

     buffer = dfb_surface_get_buffer( surface, role );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     /* Lock the surface buffer. */
     ret = dfb_surface_buffer_lock( buffer, region->surface_accessor, CSAF_READ, &region->left_buffer_lock );
     if (ret) {
          D_DERROR( ret, "Core/LayerRegion: Could not lock region surface for SetRegion()!\n" );
          dfb_surface_unlock( surface );
          return ret;
     }

     allocation = region->left_buffer_lock.allocation;
     D_ASSERT( allocation != NULL );

     /* If hardware has written or is writing... */
     if (allocation->accessed[CSAID_GPU] & CSAF_WRITE) {
          D_DEBUG_AT( Core_Layers, "  -> Waiting for pending writes...\n" );

          /* ...wait for the operation to finish. */
          dfb_gfxcard_sync(); /* TODO: wait for serial instead */

          // allocation->accessed[CSAID_GPU] &= ~CSAF_WRITE;
     }

     if (stereo) {
          dfb_surface_set_stereo_eye(surface, DSSE_RIGHT);
          buffer = dfb_surface_get_buffer( surface, role );
          D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     
          /* Lock the surface buffer. */
          ret = dfb_surface_buffer_lock( buffer, region->surface_accessor, CSAF_READ, &region->right_buffer_lock );
          if (ret) {
               D_DERROR( ret, "Core/LayerRegion: Could not lock region surface for SetRegion()!\n" );
               dfb_surface_unlock( surface );
               return ret;
          }
     
          allocation = region->right_buffer_lock.allocation;
          D_ASSERT( allocation != NULL );
     
          /* If hardware has written or is writing... */
          if (allocation->accessed[CSAID_GPU] & CSAF_WRITE) {
               D_DEBUG_AT( Core_Layers, "  -> Waiting for pending writes...\n" );
     
               /* ...wait for the operation to finish. */
               dfb_gfxcard_sync(); /* TODO: wait for serial instead */
     
               // allocation->accessed[CSAID_GPU] &= ~CSAF_WRITE;
          }
     }

     /* Restore current buffer focus. */
     dfb_surface_set_stereo_eye(surface, eye);

     /* surface is unlocked by caller */

     return DFB_OK;
}

/******************************************************************************/

static DFBResult
set_region( CoreLayerRegion            *region,
            CoreLayerRegionConfig      *config,
            CoreLayerRegionConfigFlags  flags,
            CoreSurface                *surface )
{
     DFBResult                ret;
     CoreLayer               *layer;
     CoreLayerShared         *shared;
     const DisplayLayerFuncs *funcs;
     bool                     stereo;

     D_DEBUG_AT( Core_Layers, "%s( %p, %p, 0x%08x, %p )\n", __FUNCTION__, region, config, flags, surface );

     DFB_CORE_LAYER_REGION_CONFIG_DEBUG_AT( Core_Layers, config );

     D_DEBUG_AT( Core_Layers, "  -> state    0x%08x\n", region->state );

     D_ASSERT( region != NULL );
     D_ASSERT( region->context != NULL );
     D_ASSERT( config != NULL );
     D_ASSERT( config->buffermode != DLBM_UNKNOWN );

     D_ASSERT( D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ) );

     layer = dfb_layer_at( region->context->layer_id );

     D_ASSERT( layer != NULL );
     D_ASSERT( layer->shared != NULL );
     D_ASSERT( layer->funcs != NULL );
     D_ASSERT( layer->funcs->SetRegion != NULL );

     stereo = !!(config->options & DLOP_STEREO);

     if (region->state & CLRSF_FROZEN) {
          D_DEBUG_AT( Core_Layers, "  -> FROZEN!\n" );
          return DFB_OK;
     }

     shared = layer->shared;
     funcs  = layer->funcs;

     if (surface) {
          if (flags & (CLRCF_SURFACE | CLRCF_WIDTH   | CLRCF_HEIGHT | CLRCF_FORMAT | CLRCF_SRCKEY |
                       CLRCF_DSTKEY  | CLRCF_OPACITY | CLRCF_SOURCE | CLRCF_DEST | CLRCF_CUR_DEST)) {
               ret = region_buffer_lock( region, surface, CSBR_FRONT );
               if (ret)
               {
                    return ret;
               }

               dfb_surface_unlock( surface );
          }

          /* The buffer is often NULL since the region is no longer kept locked. */
     }
     else {
          if (region->left_buffer_lock.buffer) {
               D_MAGIC_ASSERT( region->left_buffer_lock.buffer, CoreSurfaceBuffer );
               dfb_surface_unlock_buffer( region->left_buffer_lock.buffer->surface, &region->left_buffer_lock );
          }
          if (stereo && region->right_buffer_lock.buffer) {
               D_MAGIC_ASSERT( region->right_buffer_lock.buffer, CoreSurfaceBuffer );
               dfb_surface_unlock_buffer( region->right_buffer_lock.buffer->surface, &region->right_buffer_lock );
          }
     }

     D_DEBUG_AT( Core_Layers, "  => setting region of '%s'\n", shared->description.name );

     /* Setup hardware. */
     ret =  funcs->SetRegion( layer, layer->driver_data, layer->layer_data,
                              region->region_data, config, flags,
                              surface, surface ? surface->palette : NULL, 
                              &region->left_buffer_lock, &region->right_buffer_lock );

     /* Unlock the region buffer since the lock is no longer necessary. */
     region_buffer_unlock(region, false);

#ifdef CC_HW_WINDOW_SUPPORT
     region->context->cur_region = region;
#endif

     return ret;
}

static DFBResult
realize_region( CoreLayerRegion *region )
{
     DFBResult                ret;
     CoreLayer               *layer;
     CoreLayerShared         *shared;
     const DisplayLayerFuncs *funcs;

     D_DEBUG_AT( Core_Layers, "%s( %p )\n", __FUNCTION__, region );

     D_ASSERT( region != NULL );

     DFB_CORE_LAYER_REGION_CONFIG_DEBUG_AT( Core_Layers, &region->config );

     D_DEBUG_AT( Core_Layers, "  -> state    0x%08x\n", region->state );

     D_ASSERT( region->context != NULL );
     D_ASSERT( D_FLAGS_IS_SET( region->state, CLRSF_CONFIGURED ) );
     D_ASSERT( ! D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ) );

     layer = dfb_layer_at( region->context->layer_id );

     D_ASSERT( layer != NULL );
     D_ASSERT( layer->shared != NULL );
     D_ASSERT( layer->funcs != NULL );

     shared = layer->shared;
     funcs  = layer->funcs;

     D_ASSERT( ! fusion_vector_contains( &shared->added_regions, region ) );

     if (region->state & CLRSF_FROZEN) {
          D_DEBUG_AT( Core_Layers, "  -> FROZEN!\n" );
          return DFB_OK;
     }

     /* Allocate the driver's region data. */
     if (funcs->RegionDataSize) {
          int size = funcs->RegionDataSize();

          if (size > 0) {
               region->region_data = SHCALLOC( shared->shmpool, 1, size );
               if (!region->region_data)
                    return D_OOSHM();
          }
     }

     D_DEBUG_AT( Core_Layers, "  => adding region to '%s'\n", shared->description.name );

     /* Add the region to the driver. */
     if (funcs->AddRegion) {
          ret = funcs->AddRegion( layer,
                                  layer->driver_data, layer->layer_data,
                                  region->region_data, &region->config );
          if (ret) {
               D_DERROR( ret, "Core/Layers: Could not add region!\n" );

               if (region->region_data) {
                    SHFREE( shared->shmpool, region->region_data );
                    region->region_data = NULL;
               }

               return ret;
          }
     }

     /* Add the region to the 'added' list. */
     fusion_vector_add( &shared->added_regions, region );

     /* Update the region's state. */
     D_FLAGS_SET( region->state, CLRSF_REALIZED );

     /* Initially setup hardware. */
     ret = set_region( region, &region->config, CLRCF_ALL, region->surface );
     if (ret) {
          unrealize_region( region );
          return ret;
     }

     return DFB_OK;
}

static DFBResult
unrealize_region( CoreLayerRegion *region )
{
     DFBResult                ret;
     int                      index;
     CoreLayer               *layer;
     CoreLayerShared         *shared;
     const DisplayLayerFuncs *funcs;
     bool                     stereo;

     D_DEBUG_AT( Core_Layers, "%s( %p )\n", __FUNCTION__, region );

     D_ASSERT( region != NULL );

     DFB_CORE_LAYER_REGION_CONFIG_DEBUG_AT( Core_Layers, &region->config );

     D_DEBUG_AT( Core_Layers, "  -> state    0x%08x\n", region->state );

     D_ASSERT( region->context != NULL );
     D_ASSERT( D_FLAGS_IS_SET( region->state, CLRSF_REALIZED ) );

     layer = dfb_layer_at( region->context->layer_id );

     D_ASSERT( layer != NULL );
     D_ASSERT( layer->shared != NULL );
     D_ASSERT( layer->funcs != NULL );

     shared = layer->shared;
     funcs  = layer->funcs;

     stereo = region->config.options & DLOP_STEREO;

     D_ASSERT( fusion_vector_contains( &shared->added_regions, region ) );

     index = fusion_vector_index_of( &shared->added_regions, region );

     D_DEBUG_AT( Core_Layers, "  => removing region from '%s'\n", shared->description.name );

     /* Remove the region from hardware and driver. */
     if (funcs->RemoveRegion) {
          ret = funcs->RemoveRegion( layer, layer->driver_data,
                                     layer->layer_data, region->region_data );
          if (ret) {
               D_DERROR( ret, "Core/Layers: Could not remove region!\n" );
               return ret;
          }
     }

     /* Remove the region from the 'added' list. */
     if (index >= 0 && index < shared->added_regions.count)
     {
         fusion_vector_remove( &shared->added_regions, index );
     }

     /* Deallocate the driver's region data. */
     if (region->region_data) {
          SHFREE( shared->shmpool, region->region_data );
          region->region_data = NULL;
     }

     /* Update the region's state. */
     D_FLAGS_CLEAR( region->state, CLRSF_REALIZED );
     D_FLAGS_SET( region->state, CLRSF_FROZEN );
     
     // direct_log_printf(NULL,"%s[%d][%d][0x%x]\n",__FUNCTION__,getpid(),region->context->layer_id,region->state);

     /* Unlock the region buffer if it is locked. */
     if (region->surface) { 
          if (region->left_buffer_lock.buffer) {
               dfb_surface_unlock_buffer( region->surface, &region->left_buffer_lock );

               if (!stereo)
                    dfb_surface_destroy_buffers( region->surface );
          }
          if (stereo) { 
               if (region->right_buffer_lock.buffer)
                    dfb_surface_unlock_buffer( region->surface, &region->right_buffer_lock );

               dfb_surface_destroy_buffers( region->surface );
          }
     }

     return DFB_OK;
}

