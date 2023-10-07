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

#include <direct/debug.h>
#include <direct/mem.h>
#include <core/core.h>
#include <core/surface_pool.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include "devmem.h"
#include "surfacemanager.h"

D_DEBUG_DOMAIN( DevMem_Surfaces, "DevMem/Surfaces", "DevMem Framebuffer Surface Pool" );
D_DEBUG_DOMAIN( DevMem_SurfLock, "DevMem/SurfLock", "DevMem Framebuffer Surface Pool Locks" );
#define VIDEOMEMORY_FRAGMENT_MERGE 1
/**********************************************************************************************************************/


static CoreDFBShared          *mem_shared;

/**********************************************************************************************************************/

static int
devmemPoolDataSize( void )
{
     return sizeof(DevMemPoolData);
}

static int
devmemPoolLocalDataSize( void )
{
     return sizeof(DevMemPoolLocalData);
}

static int
devmemAllocationDataSize( void )
{
     return sizeof(DevMemAllocationData);
}


static DFBResult
devmemInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( data != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );
     D_ASSERT( ret_desc != NULL );

     mem_shared = core->shared;
     ret = dfb_surfacemanager_create( core, dfb_config->video_length, &data->manager );
     if (ret)
         return ret;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL | CSTF_DEDICATED;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              = dfb_config->video_length;


     if ( dfb_config->mst_miu2_cpu_offset != 0 && dfb_config->video_phys >= dfb_config->mst_miu2_cpu_offset)
         ret_desc->types |= CSTF_MIU2;
     else if ( dfb_config->mst_miu1_cpu_offset != 0 && dfb_config->video_phys >= dfb_config->mst_miu1_cpu_offset )
         ret_desc->types |= CSTF_MIU1;
     else
         ret_desc->types |= CSTF_MIU0;
      ret_desc->types |= CSTF_DECLARABLE;


     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "/dev/mem" );

     local->core = core;
     local->mem  = devmem->mem;

     D_MAGIC_SET( data, DevMemPoolData );
     D_MAGIC_SET( local, DevMemPoolLocalData );

     devmem->shared->manager     = data->manager;


     return DFB_OK;
}

static DFBResult
devmemJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );
     mem_shared = core->shared;

     (void) data;

     local->core = core;
     local->mem  = devmem->mem;

     D_MAGIC_SET( local, DevMemPoolLocalData );

     return DFB_OK;
}

static DFBResult
devmemDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     dfb_surfacemanager_destroy( data->manager );

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
devmemLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     (void) data;

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
devmemTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     DFBResult           ret;
     CoreSurface        *surface;
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if (surface->type & CSTF_LAYER)
          return DFB_OK;

     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );
#if VIDEOMEMORY_FRAGMENT_MERGE
     if(ret == DFB_NOVIDEOMEMORY)
     {
        bool needretry = false;
        int   retrycount = 2;
RETRY:
       ret = dfb_surfacemanager_fragmentMerge(local->core,data->manager, local, &needretry);

       if(!ret)
            ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );
       else
            ret = DFB_NOVIDEOMEMORY;

      if((ret == DFB_NOVIDEOMEMORY) && (retrycount-- > 0) && needretry)
        {
        /* To reduce CPU loading when HRT is enabled, extend the sleep time from 1 ms to 5 ms.
            In HRT case, sleep 1ms causes a large scale CPU overhead.
        */
        usleep(5*1000);
        goto RETRY;
        }
     }
#endif
     D_DEBUG_AT( DevMem_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );
     if(ret == DFB_TEMPUNAVAIL)
     {
        //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
        ret = DFB_NOVIDEOMEMORY;
     }
     return ret;
}

static DFBResult
devmemAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult             ret;
     Chunk                *chunk = NULL; //Fix converity END
     CoreSurface          *surface;
     DevMemPoolData       *data  = pool_data;
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, allocation, &chunk );

     if (ret)
     {
        if(ret == DFB_TEMPUNAVAIL)
        {
             //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
             ret = DFB_NOVIDEOMEMORY;
         }
          return ret;
      }

     D_MAGIC_ASSERT( chunk, Chunk );

     alloc->offset = chunk->offset;
     alloc->pitch  = chunk->pitch;
     alloc->size   = chunk->length;

     alloc->chunk  = chunk;

     D_DEBUG_AT( DevMem_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );
     D_INFO(" Allocate -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size);

     /* 1. Record max peak usage memory of the system
        2. Record memory usage of every memory size (small, medium, large)
        3. Record max peak memory usage of every process
     */
     record_surface_memory_usage(mem_shared, alloc->size, surface);

     allocation->size   = alloc->size;
     allocation->offset = alloc->offset;


     if(surface->type & (CSTF_LAYER|CSTF_WINDOW))
        memset(local->mem+alloc->offset, 0, alloc->size);

     D_MAGIC_SET( alloc, DevMemAllocationData );

     return DFB_OK;
}

static DFBResult
devmemDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DevMemPoolData       *data  = pool_data;
     DevMemAllocationData *alloc = alloc_data;
     CoreSurface          *surface;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );


     surface = buffer->surface;

     if (alloc->chunk)
     {
         D_INFO(" Deallocate -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size);

         //printf("\33[0;33;44m------- currentMem = %dk  -  %dk \33[0m\n", mem_shared->currentMem >> 10, alloc->size >> 10);

         mem_shared->currentMem -= alloc->size;

         // record  memory usage of every memory size
         if ( dfb_config->mst_mem_small_size != 0 && dfb_config->mst_mem_medium_size != 0)
         {
             int PID = surface->object.PID;

             if ((alloc->size >> 10) <= dfb_config->mst_mem_small_size)
             {
                 mem_shared->smallMemCount --;
                 mem_shared->smallMemTotal -= alloc->size;
             }
             else if ((alloc->size >> 10) > dfb_config->mst_mem_small_size && (alloc->size >> 10) <= dfb_config->mst_mem_medium_size)
             {
                 mem_shared->mediumMemCount --;
                 mem_shared->mediumMemTotal -= alloc->size;
             }
             else
             {
                 mem_shared->largeMemCount --;
                 mem_shared->largeMemTotal -= alloc->size;
             }

             D_INFO("\33[0;33;44m [DFB mem][PID: %d ] currentMem = %dk \33[0m\n", PID, mem_shared->currentMem >> 10);
             D_INFO("\33[0;33;44m [DFB mem][PID: %d ] smallMemCount = %d (0 ~ %d k size) total: %d k\33[0m\n", PID, mem_shared->smallMemCount, dfb_config->mst_mem_small_size, mem_shared->smallMemTotal >> 10);
             D_INFO("\33[0;33;44m [DFB mem][PID: %d ] mediumMemCount = %d (%d k ~ %d k size) total: %d k\33[0m\n", PID, mem_shared->mediumMemCount, dfb_config->mst_mem_small_size, dfb_config->mst_mem_medium_size, mem_shared->mediumMemTotal >> 10);
             D_INFO("\33[0;33;44m [DFB mem][PID: %d ] largeMemCount = %d  ( bigger than %d k size ) total: %d k\33[0m\n", PID, mem_shared->largeMemCount, dfb_config->mst_mem_medium_size, mem_shared->largeMemTotal >> 10);
         }

         if ( dfb_config->mst_mem_peak_usage > 0 )
         {
            int PID = surface->object.PID;
            
            if ( PID < dfb_config->mst_mem_peak_usage )
            {
                //printf("\33[0;33;44m------- currentMem[%d] = %dk  -  %dk \33[0m\n", PID, mem_shared->memInfo[PID].currentMem >> 10, alloc->size >> 10);
                if (CSTF_LAYER & buffer->surface->type)
                {

                    D_INFO("\33[0;33;44m[DFB]------- layerSurfaceMem[%d] = %dk - %dk\33[0m\n", PID, mem_shared->memInfo[PID].layerSurfaceMem >> 10, alloc->size >> 10);

                    if (alloc->size > mem_shared->memInfo[PID].layerSurfaceMem)
                        mem_shared->memInfo[PID].layerSurfaceMem = 0;
                    else
                        mem_shared->memInfo[PID].layerSurfaceMem -= alloc->size;


                    D_INFO("\33[0;33;44m[DFB]------- layerSurfaceMem[%d] = %dk \33[0m\n", PID, mem_shared->memInfo[PID].layerSurfaceMem >> 10);

                }
                else if (mem_shared->memInfo[PID].currentMem > 0)
                {
                    if (alloc->size > mem_shared->memInfo[PID].currentMem)
                        mem_shared->memInfo[PID].currentMem = 0;
                    else
                        mem_shared->memInfo[PID].currentMem -= alloc->size;

                }
                
                //printf("\33[0;33;44m------- currentMem[%d] = %dk \33[0m\n", PID, mem_shared->memInfo[PID].currentMem >> 10);
            }
         }
         
         dfb_surfacemanager_deallocate( data->manager, alloc->chunk );
     }

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}


static DFBResult
devmemMuckOut( CoreSurfacePool   *pool,
               void              *pool_data,
               void              *pool_local,
               CoreSurfaceBuffer *buffer )
{
     CoreSurface           *surface;
     DevMemPoolData        *data  = pool_data;
     DevMemPoolLocalData   *local = pool_local;
     int          pitch  = 0;
     int        length = 0;
     CoreGraphicsDevice *device;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );
     dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length, DSCAPS_VIDEOONLY);


     return dfb_surfacemanager_displace( local->core, data->manager, buffer );

}

static DFBResult
devmemLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;
     u64 busAddr = 0;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     lock->pitch  = alloc->pitch;
     lock->offset = alloc->offset;
     lock->addr   = local->mem + alloc->offset;

     busAddr = dfb_config->video_phys + alloc->offset;
     lock->phys = (u32)(busAddr & 0x00000000FFFFFFFF);
     lock->phys_h = (u32)(busAddr >> 32);

     D_DEBUG_AT( DevMem_SurfLock, "  -> offset %lu, pitch %d, addr %p, phys_h 0x%08lx, phys 0x%08lx\n",
                 lock->offset, lock->pitch, lock->addr, lock->phys_h,  lock->phys );

     return DFB_OK;
}

static DFBResult
devmemUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     DevMemAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     (void) alloc;

     return DFB_OK;
}

const SurfacePoolFuncs devmemSurfacePoolFuncs = {
     .PoolDataSize       = devmemPoolDataSize,
     .PoolLocalDataSize  = devmemPoolLocalDataSize,
     .AllocationDataSize = devmemAllocationDataSize,

     .InitPool           = devmemInitPool,
     .JoinPool           = devmemJoinPool,
     .DestroyPool        = devmemDestroyPool,
     .LeavePool          = devmemLeavePool,

     .TestConfig         = devmemTestConfig,
     .AllocateBuffer     = devmemAllocateBuffer,
     .DeallocateBuffer   = devmemDeallocateBuffer,

     .MuckOut            = devmemMuckOut,

     .Lock               = devmemLock,
     .Unlock             = devmemUnlock,
};


///---------------------------------------------------------------------
///init the secondary video memory pool
///@param core \b IN: dfb core
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b OUT: pool local data
///@param system_data \b IN: devmem data
///@param ret_desc \b OUT: description of surface pool
///@return DFBResult
///--------------------------------------------------------------------

static DFBResult
_devmemInitPoolSecondary( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;


     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( data != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );
     D_ASSERT( ret_desc != NULL );

     ret = dfb_surfacemanager_create( core, dfb_config->video_length_secondary,&data->manager );
     if (ret)
          return ret;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              = dfb_config->video_length_secondary;

     if ( dfb_config->mst_miu2_cpu_offset != 0 && dfb_config->video_phys_secondary_cpu >= dfb_config->mst_miu2_cpu_offset)
         ret_desc->types |= CSTF_MIU2;
     else if ( dfb_config->mst_miu1_cpu_offset != 0 && dfb_config->video_phys_secondary_cpu >= dfb_config->mst_miu1_cpu_offset)
         ret_desc->types |= CSTF_MIU1;
     else
         ret_desc->types |= CSTF_MIU0;

     ret_desc->types |= CSTF_DECLARABLE;

     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "video memory secondary pool" );

     local->core = core;
     local->mem  = devmem->mem_secondary;

     D_MAGIC_SET( data, DevMemPoolData );
     D_MAGIC_SET( local, DevMemPoolLocalData );

     devmem->shared->manager     = data->manager;
     return DFB_OK;
}



///---------------------------------------------------------------------
///attach the secondary video memory pool
///@param core \b IN: dfb core
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b OUT: pool local data
///@param system_data \b IN: devmem data
///@return DFBResult
///--------------------------------------------------------------------


static DFBResult
_devmemJoinPoolSecondary( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DevMemPoolData      *data   = pool_data;
     DevMemPoolLocalData *local  = pool_local;
     DevMemData          *devmem = system_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_ASSERT( local != NULL );
     D_ASSERT( devmem != NULL );
     D_ASSERT( devmem->shared != NULL );

     (void) data;

     local->core = core;
     local->mem  = devmem->mem_secondary;

     D_MAGIC_SET( local, DevMemPoolLocalData );

     return DFB_OK;
}



///---------------------------------------------------------------------
///join the secondary video memory pool
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@return DFBResult
///--------------------------------------------------------------------

static DFBResult
_devmemDestroyPoolSecondary( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     dfb_surfacemanager_destroy( data->manager );

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}


///---------------------------------------------------------------------
///deatch the secondary video memory pool
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@return DFBResult
///--------------------------------------------------------------------

static DFBResult
_devmemLeavePoolSecondary( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );

     (void) data;

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}



///---------------------------------------------------------------------
///test whether the buffer can be allocated in the secondary video memory pool
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param buffer \b IN: the buffer need to be allocated
///@param config \b IN: no use currently
///@return DFBResult
///--------------------------------------------------------------------
static DFBResult
_devmemTestConfigSecondary( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     DFBResult           ret;
     CoreSurface        *surface;
     DevMemPoolData      *data  = pool_data;
     DevMemPoolLocalData *local = pool_local;


     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if (surface->type & CSTF_LAYER)
          return DFB_OK;


     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );
#if VIDEOMEMORY_FRAGMENT_MERGE
     if(ret == DFB_NOVIDEOMEMORY)
     {
        bool needretry = false;
        int   retrycount = 2;
RETRY:
       ret = dfb_surfacemanager_fragmentMerge(local->core,data->manager, local,&needretry);

       if(!ret)
            ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );
       else
            ret = DFB_NOVIDEOMEMORY;

      if((ret == DFB_NOVIDEOMEMORY) && (retrycount-- > 0) && needretry)
      {
          /* To reduce CPU loading when HRT is enabled, extend the sleep time from 1 ms to 5 ms.
              In HRT case, sleep 1 ms causes a large scale CPU overhead.
          */
          usleep(5*1000);
        goto RETRY;
      }
     }
#endif

     D_DEBUG_AT( DevMem_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );
     if(ret == DFB_TEMPUNAVAIL)
     {
        //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
        ret = DFB_NOVIDEOMEMORY;
     }
     return ret;
}

///---------------------------------------------------------------------
///allocate the buffer in the secondary video memory pool
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param buffer \b IN: the buffer need to be allocated
///@param allocation \b IN:allocation belong to the buffer
///@param allocc_data \b OUT: allocation data
///@return DFBResult
///--------------------------------------------------------------------

static DFBResult
_devmemAllocateBufferSecondary( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult             ret;
     Chunk                *chunk = NULL; //Fix converity END
     CoreSurface          *surface;
     DevMemPoolData       *data  = pool_data;
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;


     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );


     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, allocation, &chunk );


     if (ret)
          return ret;

     D_MAGIC_ASSERT( chunk, Chunk );

     alloc->offset = chunk->offset;
     alloc->pitch  = chunk->pitch;
     alloc->size   = chunk->length;
     alloc->chunk  = chunk;

     D_DEBUG_AT( DevMem_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );

     allocation->size   = alloc->size;
     allocation->offset = alloc->offset;

     if(surface->type & (CSTF_LAYER|CSTF_WINDOW))
        memset(local->mem+alloc->offset, 0, alloc->size);


     D_MAGIC_SET( alloc, DevMemAllocationData );

     return DFB_OK;
}



///---------------------------------------------------------------------
///deallocate the buffer in the secondary video memory pool
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param buffer \b IN: the buffer need to be allocated
///@param allocation \b IN:allocation belong to the buffer
///@param allocc_data \b IN: allocation data
///@return DFBResult
///--------------------------------------------------------------------
static DFBResult
_devmemDeallocateBufferSecondary( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DevMemPoolData       *data  = pool_data;
     DevMemAllocationData *alloc = alloc_data;

     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );

     if (alloc->chunk)
     {
        dfb_surfacemanager_deallocate( data->manager, alloc->chunk );
     }

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}




///---------------------------------------------------------------------
///get the alllocation virtual & physical address
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param allocation \b IN:allocation belong to the buffer
///@param allocc_data \b IN: allocation data
///@param lock \b OUT: record the allocation lock information
///@return DFBResult
///--------------------------------------------------------------------
static DFBResult
_devmemLockSecondary( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     DevMemPoolLocalData  *local = pool_local;
     DevMemAllocationData *alloc = alloc_data;
     u64 busAddr = 0;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     lock->pitch  = alloc->pitch;
     lock->offset = alloc->offset;
     lock->addr   = local->mem + alloc->offset;

     busAddr = dfb_config->video_phys_secondary_cpu + alloc->offset;
     lock->phys = (u32)(busAddr & 0x00000000FFFFFFFF);
     lock->phys_h = (u32)(busAddr >> 32);


     D_DEBUG_AT( DevMem_SurfLock, "  -> offset %lu, pitch %d, addr %p, phys_h 0x%08lx, phys 0x%08lx\n",
                 lock->offset, lock->pitch, lock->addr,  lock->phys_h , lock->phys );

     return DFB_OK;
}



///---------------------------------------------------------------------
///only to match the Lock operation
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param allocation \b IN:allocation belong to the buffer
///@param allocc_data \b IN: allocation data
///@param lock \b IN: record the allocation lock information
///@return DFBResult
///--------------------------------------------------------------------

static DFBResult
_devmemUnlockSecondary( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     DevMemAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     (void) alloc;

     return DFB_OK;
}



///---------------------------------------------------------------------
///mark whether buffer can be displace
///@param pool \b IN: surface pool
///@param pool_data \b IN: pool share data
///@param pool_local \b IN: pool local data
///@param buffer \b IN:surafce buffer
///@return DFBResult


static DFBResult
_devmemMuckOutSecondary( CoreSurfacePool   *pool,
               void              *pool_data,
               void              *pool_local,
               CoreSurfaceBuffer *buffer )
{
     CoreSurface           *surface;
     DevMemPoolData        *data  = pool_data;
     DevMemPoolLocalData   *local = pool_local;


     D_DEBUG_AT( DevMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMemPoolData );
     D_MAGIC_ASSERT( local, DevMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     return dfb_surfacemanager_displace( local->core, data->manager, buffer );
}




const SurfacePoolFuncs devmemSurfacePoolSecondaryFuncs = {
     .PoolDataSize       = devmemPoolDataSize,
     .PoolLocalDataSize  = devmemPoolLocalDataSize,
     .AllocationDataSize = devmemAllocationDataSize,

     .InitPool           = _devmemInitPoolSecondary,
     .JoinPool           = _devmemJoinPoolSecondary,
     .DestroyPool        = _devmemDestroyPoolSecondary,
     .LeavePool          = _devmemLeavePoolSecondary,

     .TestConfig         = _devmemTestConfigSecondary,
     .AllocateBuffer     = _devmemAllocateBufferSecondary,
     .DeallocateBuffer   = _devmemDeallocateBufferSecondary,

     .MuckOut            = _devmemMuckOutSecondary,

     .Lock               = _devmemLockSecondary,
     .Unlock             = _devmemUnlockSecondary,
};






