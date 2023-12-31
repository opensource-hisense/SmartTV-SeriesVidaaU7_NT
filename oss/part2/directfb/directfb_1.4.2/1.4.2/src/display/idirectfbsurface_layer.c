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

#include <string.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coretypes.h>

#include <core/gfxcard.h>
#include <core/state.h>
#include <core/layers.h>
#include <core/layer_region.h>
#include <core/surface.h>
#include <core/system.h>

#include <core/CoreLayerRegion.h>

#include "idirectfbsurface.h"
#include "idirectfbsurface_layer.h"

#include <misc/util.h>

#include <direct/interface.h>
#include <direct/mem.h>

#include <gfx/util.h>


D_DEBUG_DOMAIN( Surface, "IDirectFBSurfaceL", "IDirectFBSurface_Layer Interface" );

/**********************************************************************************************************************/

static void
IDirectFBSurface_Layer_Destruct( IDirectFBSurface *thiz )
{
     IDirectFBSurface_Layer_data *data = (IDirectFBSurface_Layer_data*) thiz->priv;

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     dfb_layer_region_unref( data->region );
     IDirectFBSurface_Destruct( thiz );
}

static DirectResult
IDirectFBSurface_Layer_Release( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Layer)

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     if (--data->base.ref == 0)
          IDirectFBSurface_Layer_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Layer_Flip( IDirectFBSurface    *thiz,
                             const DFBRegion     *region,
                             DFBSurfaceFlipFlags  flags )
{
     DFBRegion reg;
     DFBResult ret =DR_OK;

     DIRECT_INTERFACE_DBG_DELTA_START();
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Layer)

     D_DEBUG_AT( Surface, "%s( %p, %p, 0x%08x )\n", __FUNCTION__, thiz, region, flags );

     if (!data->base.surface) {
          printf("[DFB][%s %d] return DFB_DESTROYED pid=%d\n", __FUNCTION__, __LINE__, getpid());
          return DFB_DESTROYED;
     }

     if (data->base.surface->config.caps & DSCAPS_STEREO) {
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

     D_DEBUG_AT( Surface, "  -> FLIP %4d,%4d-%4dx%4d\n", DFB_RECTANGLE_VALS_FROM_REGION( &reg ) );

     ret = CoreLayerRegion_FlipUpdate( data->region, &reg, flags );
     if (ret != 0) {
          printf("[DFB][%s %d] ret=%d pid=%d\n", __FUNCTION__, __LINE__, ret, getpid());
     }

     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_LAYER_FLIP);
     
     return ret;
}

static DFBResult
IDirectFBSurface_Layer_FlipStereo( IDirectFBSurface    *thiz,
                                   const DFBRegion     *left_region,
                                   const DFBRegion     *right_region,
                                   DFBSurfaceFlipFlags  flags )
{
     DFBRegion l_reg, r_reg;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Layer)

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

     D_DEBUG_AT( Surface, "  -> FLIPSTEREO %4d,%4d-%4dx%4d, %4d,%4d-%4dx%4d\n", 
          DFB_RECTANGLE_VALS_FROM_REGION( &l_reg ), DFB_RECTANGLE_VALS_FROM_REGION( &r_reg ) );

     return dfb_layer_region_flip_update_stereo( data->region, &l_reg, &r_reg, flags );
}

static DFBResult
IDirectFBSurface_Layer_GetSubSurface( IDirectFBSurface    *thiz,
                                      const DFBRectangle  *rect,
                                      IDirectFBSurface   **surface )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Layer)

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     /* Check arguments */
     if (!data->base.surface)
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
          ret = IDirectFBSurface_Layer_Construct( *surface, thiz, &wanted, &granted,
                                                  data->region, data->base.caps |
                                                  DSCAPS_SUBSURFACE, data->base.core );
     }
     else {
          /* Construct */
          ret = IDirectFBSurface_Layer_Construct( *surface, thiz, NULL, NULL,
                                                  data->region, data->base.caps |
                                                  DSCAPS_SUBSURFACE, data->base.core );
     }
     
     return ret;
}


/*
wrap the IDirectFBSurface_Layer_xxx to _IDirectFBSurface_Layer_xxx for safe call
*/



static DirectResult
_IDirectFBSurface_Layer_Release( IDirectFBSurface *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Layer_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;

}

static DFBResult
_IDirectFBSurface_Layer_Flip( IDirectFBSurface    *thiz,
                             const DFBRegion     *region,
                             DFBSurfaceFlipFlags  flags )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Layer_Flip(thiz,region,flags);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBSurface_Layer_GetSubSurface( IDirectFBSurface    *thiz,
                                      const DFBRectangle  *rect,
                                      IDirectFBSurface   **surface )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBSurface_Layer_GetSubSurface(thiz,rect,surface);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;

}



DFBResult
IDirectFBSurface_Layer_Construct( IDirectFBSurface       *thiz,
                                  IDirectFBSurface       *parent,
                                  DFBRectangle           *wanted,
                                  DFBRectangle           *granted,
                                  CoreLayerRegion        *region,
                                  DFBSurfaceCapabilities  caps,
                                  CoreDFB                *core )
{
     DFBResult    ret;
     CoreSurface *surface;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBSurface_Layer);

     D_DEBUG_AT( Surface, "%s( %p )\n", __FUNCTION__, thiz );

     if (dfb_layer_region_ref( region ))
          return DFB_FUSION;

     ret = CoreLayerRegion_GetSurface( region, &surface );
     if (ret) {
          dfb_layer_region_unref( region );
          DIRECT_DEALLOCATE_INTERFACE(thiz);
          return ret;
     }

     ret = IDirectFBSurface_Construct( thiz, parent, wanted, granted, NULL,
                                       surface, surface->config.caps | caps, core );

	if(DFB_SYSTEMONLY)
	{
	    surface->config.caps &= (~DSCAPS_SYSTEMONLY);
	    surface->config.caps |= (DSCAPS_VIDEOONLY|DSCAPS_PRIMARY);
	    surface->type        |= CSTF_LAYER;
	}
		
     if (ret) {
          dfb_surface_unref( surface );
          dfb_layer_region_unref( region );
          return ret;
     }

     dfb_surface_unref( surface );

     data->region = region;

     thiz->Release       = _IDirectFBSurface_Layer_Release;
     thiz->Flip          = _IDirectFBSurface_Layer_Flip;
     thiz->FlipStereo    = IDirectFBSurface_Layer_FlipStereo;
     thiz->GetSubSurface = _IDirectFBSurface_Layer_GetSubSurface;

     return DFB_OK;
}

