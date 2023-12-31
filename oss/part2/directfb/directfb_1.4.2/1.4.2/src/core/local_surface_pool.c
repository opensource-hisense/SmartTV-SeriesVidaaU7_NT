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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <config.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <fusion/fusion.h>

#include <core/core.h>
#include <core/surface_pool.h>
#include <core/system.h>


/**********************************************************************************************************************/

typedef struct {
} LocalPoolData;

typedef struct {
    FusionCall  call;
} LocalPoolLocalData;

typedef struct {
     int         magic;

     void       *addr;
     int         pitch;
     int         size;
     FusionCall  call;
     FusionID    fid;
} LocalAllocationData;

/**********************************************************************************************************************/
static FusionCallHandlerResult
local_surface_pool_call_handler( int           caller,
                                 int           call_arg,
                                 void         *call_ptr,
                                 void         *ctx,
                                 unsigned int  serial,
                                 int          *ret_val )
{
     // direct_log_printf(NULL,"%s[0x%x,0x%x] \n",__FUNCTION__,(unsigned int)call_ptr);
     D_FREE( call_ptr );

     *ret_val = 0;

     return FCHR_RETURN;
}

static int
localPoolDataSize( void )
{
     return sizeof(LocalPoolData);
}

static int
localPoolLocalDataSize( void )
{
     return sizeof(LocalPoolLocalData);
}

static int
localAllocationDataSize( void )
{
     return sizeof(LocalAllocationData);
}

static DFBResult
localInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     int i4_acces=0;
     LocalPoolLocalData *local = pool_local;
     
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( ret_desc != NULL );

     ret_desc->caps           = CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE;
     ret_desc->types          = CSTF_FONT | CSTF_INTERNAL;
     ret_desc->priority          = CSPP_PREFERED;

    
     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "System Memory" );
     
     fusion_call_init( &local->call, local_surface_pool_call_handler, local, dfb_core_world(core) );

     return DFB_OK;
}

static DFBResult
localJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     LocalPoolLocalData *local = pool_local;
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     
     return fusion_call_init( &local->call, local_surface_pool_call_handler, local, dfb_core_world(core) );
}

static DFBResult
localDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     CoreSurfaceAllocation *allocation;
     LocalPoolLocalData    *local = pool_local;
     LocalAllocationData   *data;
     FusionID               fid;

     DFBResult res;
     int       i;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( pool_local != NULL );

     res = fusion_call_destroy( &local->call );
     fid = fusion_id( dfb_core_world(NULL) );

     /* remove the local allocations */
     fusion_vector_foreach (allocation, i, pool->allocs) {
          data = allocation->data;
          if( data->fid == fid )
               D_FREE( data->addr );
     }

     return res;    

}

static DFBResult
localLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     CoreSurfaceAllocation *allocation;
     LocalPoolLocalData    *local = pool_local;
     LocalAllocationData   *data;
     FusionID               fid;
     
     DFBResult res;
     int       i;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( pool_local != NULL );

     res = fusion_call_destroy( &local->call );
     fid = fusion_id( dfb_core_world(NULL) );

     /* remove the local allocations */
     fusion_vector_foreach (allocation, i, pool->allocs) {
          data = allocation->data;
          if( data->fid == fid )
               D_FREE( data->addr );
     }

     return res;    

}


static DFBResult
localAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     CoreSurface         *surface;
     LocalAllocationData *alloc = alloc_data;
     int                 i4_address_alignment = 0x40;
     int                 i4_pitch_alignment   = 0x40;
     LocalPoolLocalData  *local = pool_local;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_ASSERT( alloc != NULL );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     dfb_surface_calc_buffer_size( surface, i4_pitch_alignment, 0, &alloc->pitch, &alloc->size );

     alloc->addr = (void *)D_MEMALIGN(i4_address_alignment,alloc->size);
     
     if(!alloc->addr)
     {
        return D_OOM();
     }

     D_MAGIC_SET( alloc, LocalAllocationData );
     
     allocation->pin_range.start = (unsigned long)alloc->addr;
     allocation->pin_range.size  = alloc->size;
    
     //system_mmu_pin_mem(&allocation->pin_range);
     mlock(alloc->addr,alloc->size);
     
     // D_MEMSET(alloc->addr,0x98,alloc->size);
     
     allocation->flags = CSALF_VOLATILE;
     allocation->size  = alloc->size;
     alloc->call = local->call;
     alloc->fid  = fusion_id( dfb_core_world(NULL) );

     // direct_log_printf(NULL,"%s[pid = %d][0x%x,%d] \n",__FUNCTION__,getpid(),alloc->addr,alloc->size);
     
     return DFB_OK;
}

static DFBResult
localDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DFBResult            ret;
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, LocalAllocationData );
     
     //system_mmu_unpin_mem(&allocation->pin_range);
     // direct_log_printf(NULL,"%s[0x%x,%d] \n",__FUNCTION__,alloc->addr,alloc->size);
     
     if(alloc->call.handler != NULL)
     {
         ret = fusion_call_execute( &alloc->call, FCEF_ONEWAY, 0, alloc->addr, NULL );
         if (ret)
         {
            D_DERROR( ret, "SurfPool/Local: Could not call buffer owner to free it there!\n" );
         }
     }

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}

static DFBResult
localLock( CoreSurfacePool       *pool,
           void                  *pool_data,
           void                  *pool_local,
           CoreSurfaceAllocation *allocation,
           void                  *alloc_data,
           CoreSurfaceBufferLock *lock )
{
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );
     D_MAGIC_ASSERT( alloc, LocalAllocationData );
     
     lock->addr     = alloc->addr;
     lock->phys     = (unsigned long)alloc->addr;
     lock->pitch    = alloc->pitch;
     lock->access   |= CSAF_MMU;

     return DFB_OK;
}

static DFBResult
localUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );
     D_MAGIC_ASSERT( alloc, LocalAllocationData );

     (void) alloc;

     return DFB_OK;
}

const SurfacePoolFuncs localSurfacePoolFuncs = {
     .PoolDataSize       = localPoolDataSize,
     .PoolLocalDataSize  = localPoolLocalDataSize,
     .AllocationDataSize = localAllocationDataSize,

     .InitPool           = localInitPool,
     .JoinPool           = localJoinPool,
     .DestroyPool        = localDestroyPool,
     .LeavePool          = localLeavePool,

     .AllocateBuffer     = localAllocateBuffer,
     .DeallocateBuffer   = localDeallocateBuffer,

     .Lock               = localLock,
     .Unlock             = localUnlock,
};

