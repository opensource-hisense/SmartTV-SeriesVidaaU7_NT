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
#include <fcntl.h>
#include <sys/mman.h>
#include <direct/debug.h>
#include <direct/mem.h>
#include <core/core.h>
#include <core/surface_pool.h>

#include <gfx/convert.h>
#include <linux/unistd.h>

#include <misc/conf.h>

#include "mstartlb.h"
#include "surfacemanager.h"
//#include <linux/android_mstartlb.h>
#include "MsCommon.h"



D_DEBUG_DOMAIN( DevMstarTLB_Surfaces, "DevMstarTLB/Surfaces", "DevMstarTLB Framebuffer Surface Pool" );
D_DEBUG_DOMAIN( DevMstarTLB_SurfLock, "DevMstarTLB/SurfLock", "DevMstarTLB Framebuffer Surface Pool Locks" );

#define DEV_MEM     "/dev/mem"


 static FusionCallHandlerResult
_MstarTLB_call_handler( int           caller,   /* fusion id of the caller */
                           int           call_arg, /* optional call parameter */
                           void         *call_ptr, /* optional call parameter */
                           void         *ctx,      /* optional handler context */
                           unsigned int  serial,
                           int          *ret_val )
{
    DFBMstarTLBCommand  command = call_arg;
    DevMstarTLBData          *DevMstarTLB = ctx;
    DFBResult ret = DFB_FAILURE;

    D_MAGIC_ASSERT( DevMstarTLB, DevMstarTLBData );
    switch (command)
    {
        case  DPC_MEM_ALLOCATE:
        {
            BufferInfo *tlbAlloc =(BufferInfo *) call_ptr;
            ret = HWTLB_AllocateBuffer(DevMstarTLB, tlbAlloc);

            break;
        }
        
        case  DPC_MEM_DEALLOCATE:
        {
            unsigned int handle =* (unsigned int*) call_ptr;
            ret = HWTLB_DeallocteBuffer(DevMstarTLB, handle);

            break;
        }

        default :
            *ret_val = DFB_BUG;

    }
    if(ret == DFB_OK)
    {
        *ret_val = DFB_OK ;
    }
    else
    {
        *ret_val = DFB_BUG ;
    }
    return FCHR_RETURN;
}

static int
DevMstarTLBPoolDataSize( void )
{
     return sizeof(DevMstarTLBPoolSharedData);
}

static int
DevMstarTLBPoolLocalDataSize( void )
{
     return sizeof(DevMstarTLBPoolLocalData);
}

static int
DevMstarTLBAllocationDataSize( void )
{
     return sizeof(DevMstarTLBAllocationData);
}


static DFBResult
DevMstarTLBInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DevMstarTLBPoolSharedData      *sharedData   = pool_data;
     DevMstarTLBPoolLocalData *local  = pool_local;
     DevMstarTLBData          *DevMstarTLB = system_data;
     DevMstarTLBDataShared    *shared = DevMstarTLB->shared;
     TLBMemInfo tlbMemInfo;
     char tmp_buf[32];
     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( sharedData != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( DevMstarTLB != NULL );
     D_ASSERT( DevMstarTLB->shared != NULL );
     D_ASSERT( ret_desc != NULL );


     tlbMemInfo.msttlb_videoMem_length = shared->msttlb_videoMem_length;
     tlbMemInfo.msttlb_miu0_videoMem_length = shared->msttlb_miu0_videoMem_length;
     tlbMemInfo.msttlb_miu1_videoMem_length = shared->msttlb_miu1_videoMem_length;

     tlbMemInfo.msttlb_TLBAddr_start = shared->msttlb_TLBAddr_start;
     tlbMemInfo.msttlb_TLBAddr_length = shared->msttlb_TLBAddr_length;

     ret = dfb_surfacemanager_create( core,  &sharedData->manager,&tlbMemInfo );
     if (ret)
         return ret;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     if(dfb_config->bPrealloc_map_tlb)
        ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL | CSTF_PREALLOCATED_IN_VIDEO ;
     else
        ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              = shared->msttlb_videoMem_length;

     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     sprintf(tmp_buf, "/dev/mstartlb");

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, tmp_buf);

     local->core = core;

     local->systemData = system_data;


     D_MAGIC_SET( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_SET( local, DevMstarTLBPoolLocalData );
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;
     local->mem = DevMstarTLB->mem;  

     fusion_call_init( &shared->call, _MstarTLB_call_handler, system_data, dfb_core_world(core) );

     return DFB_OK;
}

static DFBResult
DevMstarTLBJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DevMstarTLBPoolSharedData      *sharedData   = pool_data;
     DevMstarTLBPoolLocalData *local  = pool_local;
     DevMstarTLBData          *DevMstarTLB = system_data;
     DevMstarTLBDataShared    *shared = DevMstarTLB->shared;

     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_ASSERT( local != NULL );
     D_ASSERT( DevMstarTLB != NULL );
     D_ASSERT( DevMstarTLB->shared != NULL );

     local->core = core;
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;
     local->mem  = DevMstarTLB->mem;  

     D_MAGIC_SET( local, DevMstarTLBPoolLocalData );

    return DFB_OK;
}

static DFBResult
DevMstarTLBDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DevMstarTLBPoolSharedData      *sharedData  = pool_data;
     DevMstarTLBPoolLocalData *local = pool_local;
     DevMstarTLBDataShared    *shared = local->systemData->shared;
     
     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( local, DevMstarTLBPoolLocalData );

     dfb_surfacemanager_destroy( sharedData->manager );
     fusion_call_destroy( &shared->call );


     D_MAGIC_CLEAR( sharedData );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DevMstarTLBLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DevMstarTLBPoolSharedData      *sharedData  = pool_data;
     DevMstarTLBPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( local, DevMstarTLBPoolLocalData );


     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DevMstarTLBTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     int           ret;
     CoreSurface        *surface;
     DevMstarTLBPoolSharedData      *sharedData  = pool_data;
     DevMstarTLBPoolLocalData *local = pool_local;
     DevMstarTLBData          *DevMstarTLB = local->systemData;
     int index, busaddr, phyaddr = 0;


     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( local, DevMstarTLBPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );


     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );
     if(surface->type == CSTF_PREALLOCATED_IN_VIDEO)
     {
         for (index=0; index<MAX_SURFACE_BUFFERS; index++) {
             if (surface->buffers[index] == buffer)
                 break;
         }
         busaddr = surface->config.preallocated[index].addr;
         phyaddr = MsOS_BA2PA(busaddr);    
     }


     ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, phyaddr, NULL, NULL);

     if(ret == DFB_NOVIDEOMEMORY)
     {
       ret = dfb_surfacemanager_fragmentMerge(local->core, sharedData->manager, local);

       if(!ret)
            ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, phyaddr, NULL, NULL);
       else
            ret = DFB_NOVIDEOMEMORY;

     }

    if(ret != DFB_OK)
    {
        D_DEBUG_AT( DevMstarTLB_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );
        if(ret == DFB_TEMPUNAVAIL)
        {
         //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
         ret = DFB_NOVIDEOMEMORY;
        }
    }
     
      return ret;
}

DFBResult _HWTLB_AllocateBuffer(DevMstarTLBData * data,BufferInfo * tlbAlloc)
{
     BufferInfo *parameter;
     FusionSHMPoolShared *shm_pool;
     int ret;
 
     shm_pool = dfb_core_shmpool_data(data->core);
     D_ASSERT(shm_pool);
     
     parameter = SHCALLOC(shm_pool, 1, sizeof(BufferInfo));
     if (!parameter) {
          return D_OOSHM();
     }
 
     memcpy(parameter,tlbAlloc,sizeof(BufferInfo));
     if(fusion_call_execute(&(data->shared->call), FCEF_NONE, DPC_MEM_ALLOCATE, parameter, &ret ))
     {
         SHFREE(shm_pool, parameter);
         return DFB_FUSION;
     }
     memcpy(tlbAlloc,parameter,sizeof(BufferInfo));
     SHFREE(shm_pool, parameter);
 
     return ret;
}


DFBResult _HWTLB_DeallocteBuffer(DevMstarTLBData * data,AllocationHandle handle)
{
    AllocationHandle *parameter;
    FusionSHMPoolShared *shm_pool;
    int ret;

    shm_pool = dfb_core_shmpool_data(data->core);
    D_ASSERT(shm_pool);

    parameter = SHCALLOC(shm_pool, 1, sizeof(AllocationHandle));
    if (!parameter) {
       return D_OOSHM();
    }

    memcpy(parameter,&handle,sizeof(AllocationHandle));
    if(fusion_call_execute(&(data->shared->call), FCEF_NONE, DPC_MEM_DEALLOCATE, parameter, &ret ))
    {
      SHFREE(shm_pool, parameter);
      return DFB_FUSION;
    }
    SHFREE(shm_pool, parameter);

    return ret;
}


static DFBResult
DevMstarTLBAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult             ret;
     Chunk                *chunk;
     CoreSurface          *surface;
     DevMstarTLBPoolSharedData       *sharedData  = pool_data;
     DevMstarTLBPoolLocalData  *local = pool_local;
     DevMstarTLBAllocationData *alloc = alloc_data;
     DevMstarTLBData          *DevMstarTLB = local->systemData;
     DevMstarTLBDataShared    *DevMstarTLBShared = DevMstarTLB->shared;
     int type = MIU_NONE;
     int index;
     int busaddr, phyaddr = 0;

     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( local, DevMstarTLBPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );


     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );
     if(surface->type & CSTF_PREALLOCATED_IN_VIDEO)
     {
         for (index=0; index<MAX_SURFACE_BUFFERS; index++) {
             if (surface->buffers[index] == buffer)
                 break;
         }
         busaddr = surface->config.preallocated[index].addr;
          //phyaddr =_mstarGFXAddr(busaddr);
         phyaddr = MsOS_BA2PA(busaddr);    
         printf("prealloc buf: busaddr = 0x%08x, phy_addr =0x%08x\n", busaddr, phyaddr);
     }

     ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, phyaddr, allocation, &chunk );     

     if(ret == DFB_OK)
     {
        BufferInfo tlbAlloc;
        int alignment = dfb_config->TLBAlignmentSize;
        tlbAlloc.tlb_start= chunk->offset ;
        tlbAlloc.tlb_size = chunk->length;
        tlbAlloc.miu_type = MIU_0;
        alloc->prealloc_offset = 0;

        if(surface->type & CSTF_PREALLOCATED_IN_VIDEO)
        {
            tlbAlloc.location = ALLOC_DPAGE;
            tlbAlloc.phy_addr = phyaddr/alignment*alignment;
            alloc->prealloc_offset = phyaddr%alignment;

            printf("DevMstarTLBAllocateBuffer: busaddr = 0x%08x, phy_addr = 0x%08x\n", busaddr, tlbAlloc.phy_addr);
        }
        else
        {
            tlbAlloc.location = ALLOC_KPAGE;
        }

        ret = _HWTLB_AllocateBuffer(DevMstarTLB,&tlbAlloc);

        if(ret ==DFB_OK)
        {
            //printf("HWTLB_AllocateBuffer alloc->handle = %d, tid = %d  \n", tlbAlloc.handle, syscall(__NR_gettid));
            alloc->phys = tlbAlloc.phy_addr;
            alloc->handle = tlbAlloc.handle;
            if(alloc->handle == 0)
                printf("HWTLB_AllocateBuffer alloc->handle = 0 !!!!!, tid = %d  \n", syscall(__NR_gettid));
        }
        else
        {
            printf("HWTLB_AllocateBuffer fail\n");
            return DFB_NOVIDEOMEMORY;
        }
     }

     D_MAGIC_ASSERT( chunk, Chunk );

     alloc->offset = chunk->offset;
     alloc->pitch  = chunk->pitch;
     alloc->size   = chunk->length;
     alloc->chunk  = chunk;
     if(surface->type & CSTF_PREALLOCATED_IN_VIDEO)
        alloc->pitch  = surface->config.preallocated[index].pitch;


     D_DEBUG_AT( DevMstarTLB_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );
    // printf("DevMstarTLBAllocateBuffer  -> offset 0x%x, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );
     allocation->size   = alloc->size;
     allocation->offset = alloc->offset;


     if(surface->type & (CSTF_LAYER|CSTF_WINDOW))
     {
        void *vaddr;

        vaddr = DevMstarTLB->mem + alloc->offset;
        memset(vaddr, 0, alloc->size);
    }

     D_MAGIC_SET( alloc, DevMstarTLBAllocationData );

     return DFB_OK;
}

static DFBResult
DevMstarTLBDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DevMstarTLBPoolSharedData       *sharedData  = pool_data;
     DevMstarTLBAllocationData *alloc = alloc_data;
     DevMstarTLBPoolLocalData  *local = pool_local;
     DevMstarTLBData          *DevMstarTLB = local->systemData;
     DFBResult             ret;

     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DevMstarTLBAllocationData );


     if (alloc->chunk)
     {
         int type = alloc->miuType;
         dfb_surfacemanager_deallocate( sharedData->manager, alloc->chunk ,type);

        ret = _HWTLB_DeallocteBuffer(DevMstarTLB,alloc->handle);
        if(ret != DFB_OK )
        {
            printf("DevMstarTLBDeallocateBuffer fail!!!!!\n");
        }
     }

     D_MAGIC_CLEAR( alloc );
     return DFB_OK;
}


static DFBResult
DevMstarTLBMuckOut( CoreSurfacePool   *pool,
               void              *pool_data,
               void              *pool_local,
               CoreSurfaceBuffer *buffer )
{
     CoreSurface           *surface;
     DevMstarTLBPoolSharedData        *data  = pool_data;
     DevMstarTLBPoolLocalData   *local = pool_local;
     int          pitch  = 0;
     int        length = 0;
     CoreGraphicsDevice *device;

     D_DEBUG_AT( DevMstarTLB_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevMstarTLBPoolSharedData );
     D_MAGIC_ASSERT( local, DevMstarTLBPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );
     dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

     return dfb_surfacemanager_displace( local->core, data->manager, buffer );

}

static DFBResult
DevMstarTLBLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
    DevMstarTLBPoolLocalData  *local = pool_local;
    DevMstarTLBAllocationData *alloc = alloc_data;
    DevMstarTLBData          *DevMstarTLB = local->systemData;
    DevMstarTLBDataShared    *sharedData = DevMstarTLB->shared;

    DFBResult ret;
    unsigned char *addr;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DevMstarTLBAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_ASSERT(alloc->chunk);
    D_MAGIC_ASSERT(alloc->chunk, Chunk);

    D_DEBUG_AT( DevMstarTLB_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

    lock->pitch = alloc->pitch;
    lock->offset = alloc->offset;

    lock->addr = DevMstarTLB->mem + lock->offset + alloc->prealloc_offset;
    lock->phys = sharedData->msttlb_TLBAddr_start + lock->offset + alloc->prealloc_offset; //tlb addr

    //printf( "  -> offset %x, DevMstarTLB->mem %x, addr %p, phys 0x%08lx\n",
      //       lock->offset, DevMstarTLB->mem, lock->addr, lock->phys );

    D_DEBUG_AT( DevMstarTLB_SurfLock, "  -> offset 0x%x, DevMstarTLB->mem 0x%x, addr %p, phys 0x%08lx\n",
             lock->offset, DevMstarTLB->mem, lock->addr, lock->phys );

    return DFB_OK;
}

static DFBResult
DevMstarTLBUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
    DevMstarTLBPoolLocalData  *local = pool_local;
    DevMstarTLBAllocationData *alloc = alloc_data;
    DevMstarTLBData          *DevMstarTLB = local->systemData;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DevMstarTLBAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_AT( DevMstarTLB_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

    (void) alloc;
    return DFB_OK;
}



const SurfacePoolFuncs DevMstarTLBSurfacePoolFuncs = {
     .PoolDataSize       = DevMstarTLBPoolDataSize,
     .PoolLocalDataSize  = DevMstarTLBPoolLocalDataSize,
     .AllocationDataSize = DevMstarTLBAllocationDataSize,

     .InitPool           = DevMstarTLBInitPool,
     .JoinPool           = DevMstarTLBJoinPool,
     .DestroyPool        = DevMstarTLBDestroyPool,
     .LeavePool          = DevMstarTLBLeavePool,

     .TestConfig         = DevMstarTLBTestConfig,
     .AllocateBuffer     = DevMstarTLBAllocateBuffer,
     .DeallocateBuffer   = DevMstarTLBDeallocateBuffer,

     .MuckOut            = DevMstarTLBMuckOut,

     .Lock               = DevMstarTLBLock,
     .Unlock             = DevMstarTLBUnlock,
};
