/*
   (c) Copyright 2001-2011  The world wide DirectFB Open Source Community (directfb.org)
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

#include "CoreLayerRegion.h"

extern "C" {
#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <core/core.h>
}

D_DEBUG_DOMAIN( DirectFB_CoreLayerRegion, "DirectFB/CoreLayerRegion", "DirectFB CoreLayerRegion" );

extern "C"
{
extern DFBResult mdfb_wait_for_sync(void);
}

/*********************************************************************************************************************/

namespace DirectFB {



DFBResult
ILayerRegion_Real::FlipUpdate(
                    const DFBRegion                           *update,
                    DFBSurfaceFlipFlags                        flags
)
{
    DFBResult ret;
    D_DEBUG_AT( DirectFB_CoreLayerRegion, "ILayerRegion_Requestor::%s()\n", __FUNCTION__ );

#ifdef CC_DFB_DEBUG_SUPPORT	  
	if(dfb_enable_dump(30))
	{
	    D_INFO("[%s][%d] flags[0x%x] \n", __FUNCTION__, __LINE__, flags);
		if(update)
		{
		     D_INFO("[%s][%d] repaint[%d, %d, %d, %d]\n", __FUNCTION__, __LINE__, update->x1, update->x2, update->y1, update->y2);
		}
		else
		{
		     D_INFO("[%s][%d] repaint[FULL]\n", __FUNCTION__, __LINE__);
		}
	    struct timeval _time;
		static struct timeval _backup;
		static int _init = 0;
		gettimeofday(&_time, 0);
		if(_init)
		{
			   D_INFO("%s, frame rate: %d \n",__FUNCTION__,	
					 (_time.tv_sec - _backup.tv_sec) * 1000000 
					 + _time.tv_usec - _backup.tv_usec);
		}
		_backup.tv_sec = _time.tv_sec;
		_backup.tv_usec = _time.tv_usec;
		_init=1;
	} 
#endif		  
	

#ifdef CC_HW_WINDOW_SUPPORT
    if(obj->context->cur_region == obj)
    {    
         if(flags & DSFLIP_ONSYNC)
         {
#ifdef CC_DFB_DEBUG_SUPPORT						
	          struct timeval _s_vsync;
	          struct timeval _e_vsync;
			  if(dfb_enable_dump(30))
			  {
			      gettimeofday(&_s_vsync, 0);
			  }
#endif         
	          mdfb_wait_for_sync();

#ifdef CC_DFB_DEBUG_SUPPORT
			  if(dfb_enable_dump(30))
			  {
				   gettimeofday(&_e_vsync, 0);
	 
				   D_INFO("%s, DSFLIP_ONSYNC: %d \n",__FUNCTION__,	 
						  (_e_vsync.tv_sec - _s_vsync.tv_sec) * 1000000 
						   + _e_vsync.tv_usec - _s_vsync.tv_usec);
			  }
#endif	
         }		 
	 
    }
#endif

#ifdef CC_DFB_DEBUG_SUPPORT						
	 struct timeval _s_update;
	 struct timeval _e_update;
	 if(dfb_enable_dump(30))
	 {
		 gettimeofday(&_s_update, 0);
	 }
#endif

    ret = dfb_layer_region_flip_update( obj, update, flags );

#ifdef CC_DFB_DEBUG_SUPPORT
	 if(dfb_enable_dump(30))
	 {
		  gettimeofday(&_e_update, 0);

		  D_INFO("%s, flip_update: %d \n",__FUNCTION__,	
				 (_e_update.tv_sec - _s_update.tv_sec) * 1000000 
				  + _e_update.tv_usec - _s_update.tv_usec);
	 }
#endif	


#ifdef CC_HW_WINDOW_SUPPORT
    if(obj->context->cur_region == obj)
	{
		if(flags & DSFLIP_WAIT)
		{
#ifdef CC_DFB_DEBUG_SUPPORT						
	          struct timeval _s_wait;
	          struct timeval _e_wait;
			  if(dfb_enable_dump(30))
			  {
			      gettimeofday(&_s_wait, 0);
			  }
#endif 		
		      mdfb_wait_for_sync();

#ifdef CC_DFB_DEBUG_SUPPORT
			  if(dfb_enable_dump(30))
			  {
				   gettimeofday(&_e_wait, 0);
	 
				   D_INFO("%s, DSFLIP_WAIT: %d \n",__FUNCTION__,	 
						  (_e_wait.tv_sec - _s_wait.tv_sec) * 1000000 
						   + _e_wait.tv_usec - _s_wait.tv_usec);
			  }
#endif	

		}
	}
#endif

	return ret;
}


DFBResult
ILayerRegion_Real::GetSurface(
                    CoreSurface                              **ret_surface
)
{
     D_DEBUG_AT( DirectFB_CoreLayerRegion, "ILayerRegion_Requestor::%s()\n", __FUNCTION__ );

     return dfb_layer_region_get_surface( obj, ret_surface );
}


}
