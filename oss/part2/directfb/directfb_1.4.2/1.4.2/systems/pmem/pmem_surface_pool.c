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

#include <misc/conf.h>

#include "pmem.h"
#include "surfacemanager.h"
#include <linux/android_pmem.h>
#include "pmem_utility.h"


D_DEBUG_DOMAIN( DevPMem_Surfaces, "DevPMem/Surfaces", "DevPMem Framebuffer Surface Pool" );
D_DEBUG_DOMAIN( DevPMem_SurfLock, "DevPMem/SurfLock", "DevPMem Framebuffer Surface Pool Locks" );

#define DEV_MEM     "/dev/mem"


/**********************************************************************************************************************/

static int _ceil_up_2_power(int x)
{
   int ret = 1;
   if(IS_POWER2(x))
       return x;

   while(ret <x)
      ret<<=1;
   return ret;



}
static DevPMemPoolLocalData *pool_local[VIDEO_POOL_NUM];
static int  _FreeBlockSlot(int pool_index,DevPMemData *systemData)
{
    DevPMemData          *DevPMem = systemData;
    DevPMemDataShared    *shared = DevPMem->shared;
    int i = 0;
    D_ASSERT( DevPMem != NULL );
    D_ASSERT( DevPMem->shared != NULL );
    D_ASSERT(pool_index>=0 && pool_index<VIDEO_POOL_NUM);


    for(i=0;i<POOL_BLOCKS_MAX;i++)
    {
         if(!shared->pmemBlocks[pool_index][i].bAllocated)
        {
             return i;
        }
    }
    return -1;
}
static int  _GetPoolIdx(DevPMemData  *DevPMem, CoreSurfacePool            *pool)
{
    DevPMemDataShared    *shared = DevPMem->shared;
    int i = 0;

    for(i=0;i<VIDEO_POOL_NUM;i++)
    {
        if(shared->pool[i]== pool)
        {
             return i;
        }
    }

    return -1;
}


static ReactionResult _DevPMem_React(const void *msg_data,
                                                      void *ctx)

{
    const DFBPMemEvent *evt = msg_data;
    unsigned size = evt->block_size;
    int pool_index = evt->pool_index;
    int block_index = -1;

    DevPMemPoolLocalData *local = ctx;
    DevPMemData          *DevPMem = local->systemData;
    CoreSurfacePool *pool = local->pool ;

    D_MAGIC_ASSERT( local, DevPMemPoolLocalData );
    D_MAGIC_ASSERT( DevPMem, DevPMemData );
    D_MAGIC_ASSERT( pool, CoreSurfacePool );


    switch(evt->type)
    {

        case DPET_DEL_MEMBLOCK :
        {
            fusion_skirmish_prevail( &pool->lock );
            block_index = evt->block_index;
            PMemReleaseMemoryBlock(DevPMem,  pool_index, block_index);
            fusion_skirmish_dismiss( &pool->lock );
            break;
        }

        default:
            break;

    }

    return RS_OK;

}


static DFBResult _DevPMemPoolDispatch(DFBPMemEvent *evt,
                                               DevPMemPoolSharedData       *pool_shared_data,  int pool_idx)
{
   // fusion_skirmish_prevail(&pool_shared_data->lock);
    evt->pool_index = pool_idx;
    fusion_reactor_dispatch( pool_shared_data->reactor, evt, true, NULL);
   // fusion_skirmish_dismiss(&pool_shared_data->lock);


    return DFB_OK;



}



 static FusionCallHandlerResult
_PMem_call_handler( int           caller,   /* fusion id of the caller */
                           int           call_arg, /* optional call parameter */
                           void         *call_ptr, /* optional call parameter */
                           void         *ctx,      /* optional handler context */
                           unsigned int  serial,
                           int          *ret_val )
{
     DFBPMemCommand  command = call_arg;
     DevPMemPoolLocalData *localPoolData  = ctx;
     DevPMemPoolSharedData *sharedPoolData = NULL;
     CoreSurfacePool *pool = localPoolData->pool;
     DevPMemData          *DevPMem = localPoolData->systemData;

    D_MAGIC_ASSERT( localPoolData, DevPMemPoolLocalData );
    D_MAGIC_ASSERT( DevPMem, DevPMemData );
    D_MAGIC_ASSERT( pool, CoreSurfacePool );

     sharedPoolData= localPoolData->sharedData;

     D_MAGIC_ASSERT( sharedPoolData, DevPMemPoolSharedData);

     switch (command)
     {
      case  DPC_REQ_MEMBLOCK:
            {
                DFBResult ret = DFB_FAILURE;
                int pool_index = -1;
                int block_index = -1;
                 unsigned long mem_phy;

                int size = 0;
                DFBPMemCMDInfo *cmdParamter = NULL;

                if(call_ptr == NULL)
                {
                    *ret_val = DFB_INVARG;
                    break;
                }
              //  fusion_skirmish_prevail(&pool->lock);
                cmdParamter = (DFBPMemCMDInfo *)call_ptr;
                pool_index = cmdParamter->pool_index;
                block_index = _FreeBlockSlot( pool_index,DevPMem);
                if(block_index <0)
                {
                      printf("warning , run out of slot, please contract dfb owner to enlarge block array size!!\n");
                     // fusion_skirmish_dismiss(&pool->lock);
                      return DFB_BUG;
                }
                if(block_index == 0)
                    size = DevPMem->shared->pool_init_size[pool_index];
                else
                    size = DevPMem->shared->pool_step_size[pool_index];

                size = MAX(cmdParamter->min_size, size);
                size = _ceil_up_2_power(size);


                do{
                      ret = PMemAllocMemBlock(DevPMem, pool_index, block_index, &mem_phy, size, &size);
                      if(ret == DFB_OK)
                           break;
                      size >>=1;
                 }while(size >= cmdParamter->min_size);

                if(!ret)
                {
                    ret = dfb_surfacemanager_appendMemblock(block_index, mem_phy, size, sharedPoolData->manager,localPoolData);
                }
                D_INFO("allocated block %d for pool %d, minsize%08x\n", block_index, pool_index, cmdParamter->min_size);
               // PMemDumpPMemInfo();
                //pmem_dump_surface_buffers(pool_index, -1);
                *ret_val = ret ;
             //  fusion_skirmish_dismiss(&pool->lock);
                break;
            }
      case  DPC_DEL_MEMBLOCK:
             {
                DFBResult ret = DFB_FAILURE;
                int pool_index = -1;
                int block_index = -1;
                DFBPMemCMDInfo *cmdParamter = NULL;

                if(call_ptr == NULL)
                {
                    *ret_val = DFB_INVARG;
                    break;
                }
              //  fusion_skirmish_prevail(&pool->lock);
                cmdParamter = (DFBPMemCMDInfo *)call_ptr;
                pool_index = cmdParamter->pool_index;
                block_index = cmdParamter->block_index;

                dfb_surfacemanager_deleteMemBlock(block_index, sharedPoolData->manager, localPoolData);
                PMemReleaseMemoryBlock(DevPMem, pool_index, block_index);
                D_INFO("freed block %d for pool %d\n", block_index, pool_index);
                //PMemDumpPMemInfo();
               // pmem_dump_surface_buffers(pool_index, -1);
                *ret_val = DFB_OK ;
             //  fusion_skirmish_dismiss(&pool->lock);
                break;
            }
      default :
         *ret_val = DFB_BUG;

     }


     return FCHR_RETURN;


}

static int
DevPMemPoolDataSize( void )
{
     return sizeof(DevPMemPoolSharedData);
}

static int
DevPMemPoolLocalDataSize( void )
{
     return sizeof(DevPMemPoolLocalData);
}

static int
DevPMemAllocationDataSize( void )
{
     return sizeof(DevPMemAllocationData);
}


static DFBResult
DevPMemInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DevPMemPoolSharedData      *sharedData   = pool_data;
     DevPMemPoolLocalData *local  = pool_local;
     DevPMemData          *DevPMem = system_data;
     DevPMemDataShared    *shared = DevPMem->shared;
     int i=0;
     char tmp_buf[32];
     int pool_idx = DevPMem->privatedUsage;

     D_DEBUG_AT( DevPMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( sharedData != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( DevPMem != NULL );
     D_ASSERT( DevPMem->shared != NULL );
     D_ASSERT( ret_desc != NULL );

     D_ASSERT(pool_idx>=0 && pool_idx<VIDEO_POOL_NUM && shared->pool[pool_idx] == NULL);

     ret = dfb_surfacemanager_create( core,  &sharedData->manager );
     if (ret)
         return ret;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              =  shared->pmemBlocks[pool_idx][0].pmem_block_length;


     if (shared->pool_miu[pool_idx])
         ret_desc->types |= CSTF_MIU1;
     else
         ret_desc->types |= CSTF_MIU0;

     if(pool_idx == VIDEO_POOLS_DECLARABLE)
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

     sprintf(tmp_buf, "/dev/pmem_pool%d\n", pool_idx);

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, tmp_buf);

     local->core = core;

     local->systemData = system_data;
     sharedData->chunk_head[0] = sharedData->manager->chunks;

     D_MAGIC_SET( sharedData, DevPMemPoolSharedData );
     D_MAGIC_SET( local, DevPMemPoolLocalData );
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;
  //   sprintf(tmp_buf, "pmem_lock%d\n", pool_idx);
    // fusion_skirmish_init( &sharedData->lock, tmp_buf, dfb_core_world(core) );
     sprintf(tmp_buf, "pmem_reactor%d\n", pool_idx);
     sharedData->reactor = fusion_reactor_new( sizeof(DFBPMemEvent), tmp_buf, dfb_core_world(core) );
     fusion_reactor_attach( sharedData->reactor, _DevPMem_React, local, &local->reaction);
     fusion_call_init( &sharedData->call, _PMem_call_handler, local, dfb_core_world(core) );



    // DevPMem->shared->manager     = sharedData->manager;


     return DFB_OK;
}

static DFBResult
DevPMemJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DevPMemPoolSharedData      *sharedData   = pool_data;
     DevPMemPoolLocalData *local  = pool_local;
     DevPMemData          *DevPMem = system_data;
     DevPMemDataShared    *shared = DevPMem->shared;
     int i =0;
     int pool_idx = _GetPoolIdx(DevPMem, pool);

     D_DEBUG_AT( DevPMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_ASSERT( local != NULL );
     D_ASSERT( DevPMem != NULL );
     D_ASSERT( DevPMem->shared != NULL );
     D_ASSERT( pool_idx>=0 && pool_idx<VIDEO_POOL_NUM );
     D_ASSERT( pool == shared->pool[pool_idx]);

     local->core = core;


//  pool_local[VIDEO_POOLS_INDEX0] = local;
    local->sharedData= sharedData;
    local->pool = pool;
    local->systemData = system_data;
    fusion_reactor_attach( sharedData->reactor, _DevPMem_React, local, &local->reaction);

    D_MAGIC_SET( local, DevPMemPoolLocalData );

    return DFB_OK;
}

static DFBResult
DevPMemDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DevPMemPoolSharedData      *sharedData  = pool_data;
     DevPMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevPMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( local, DevPMemPoolLocalData );

     dfb_surfacemanager_destroy( sharedData->manager );
     fusion_call_destroy( &sharedData->call );
     fusion_reactor_detach( sharedData->reactor, &local->reaction );
     fusion_reactor_free( sharedData->reactor );
//     fusion_skirmish_destroy( &sharedData->lock );

     D_MAGIC_CLEAR( sharedData );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DevPMemLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DevPMemPoolSharedData      *sharedData  = pool_data;
     DevPMemPoolLocalData *local = pool_local;

     D_DEBUG_AT( DevPMem_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( local, DevPMemPoolLocalData );

     fusion_reactor_detach( sharedData->reactor, &local->reaction );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DevPMemTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     int           ret;
     CoreSurface        *surface;
     DevPMemPoolSharedData      *sharedData  = pool_data;
     DevPMemPoolLocalData *local = pool_local;
     DevPMemData          *DevPMem = local->systemData;
     int pool_idx = _GetPoolIdx(DevPMem, pool);

     D_DEBUG_AT( DevPMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( local, DevPMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_ASSERT(pool_idx >= 0);

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );     

    int type = (surface->type & (CSTF_MIU1|CSTF_MIU0));
    if(type == 0)
    {
     //default we will use  the pmem1 in the MIU1
       type = CSTF_MIU1;
    }
   


   //  if (surface->type & CSTF_LAYER)
       //   return DFB_OK;
    ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, NULL, NULL );

     if(ret == DFB_NOVIDEOMEMORY)
     {
       ret = dfb_surfacemanager_fragmentMerge(local->core, pool_idx, sharedData->manager, local);

       if(!ret)
            ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, NULL, NULL );
       else
           ret = DFB_NOVIDEOMEMORY;

     }

     if((ret == DFB_NOVIDEOMEMORY)||(ret == DFB_TEMPUNAVAIL))
     {

          int block_index = -1;

           int min_size;
           int pitch;
           CoreGraphicsDevice *device;
         // fusion_skirmish_prevail(&sharedData->lock);
         //try again :maybe other slave have apply for the the memblock
          if(ret == DFB_TEMPUNAVAIL)
                ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, NULL, NULL );
          if((ret == DFB_NOVIDEOMEMORY)||(ret == DFB_TEMPUNAVAIL))
          {
               unsigned max_block_size = 0; //Fix converity END
               int length;

               device = dfb_core_get_part( DevPMem->core, DFCP_GRAPHICS );
               D_ASSERT( device != NULL );

               dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

               if(PMEMGetSpaceInfo(pool_idx, NULL, &max_block_size) && max_block_size>=length)
                     ret = DFB_OK;
               else
                {

                    if (D_FLAGS_ARE_SET( pool->desc.types, type)) 
                       {           
                        printf("can not find enough pmem memory space :max_block_size=%08x, required len=%08x,  pool_index :%d\n", max_block_size, length,pool_idx);
                       }
                     ret = DFB_NOVIDEOMEMORY;
                }
           }
       // fusion_skirmish_dismiss(&sharedData->lock);
       }

      if(ret != DFB_OK)
      {
            D_DEBUG_AT( DevPMem_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );
            if(ret == DFB_TEMPUNAVAIL)
            {
                  //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
                   ret = DFB_NOVIDEOMEMORY;
             }
      }
   if((ret !=DFB_OK)&&D_FLAGS_ARE_SET( pool->desc.types, type))
   {
      printf("error !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
   }
      return ret;
}

static DFBResult
DevPMemAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult             ret;
     Chunk                *chunk;
     CoreSurface          *surface;
     DevPMemPoolSharedData       *sharedData  = pool_data;
     DevPMemPoolLocalData  *local = pool_local;
     DevPMemAllocationData *alloc = alloc_data;
     DevPMemData          *DevPMem = local->systemData;
     int pool_idx = _GetPoolIdx(DevPMem, pool);
     bool bFirstTry = true;

     D_DEBUG_AT( DevPMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( local, DevPMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_ASSERT(pool_idx>=0);

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

TRY_AGAIN:

     ret = dfb_surfacemanager_allocate( local->core, sharedData->manager, buffer, allocation, &chunk );

     if (ret)
     {
        if(ret == DFB_TEMPUNAVAIL)
        {
             //convert DFB_TEMPUNAVAILABLE to DFB_NOVIDEOMEMORY for buffers to have a change to been muck out
             ret = DFB_NOVIDEOMEMORY;
        }
        if(bFirstTry &&ret==DFB_NOVIDEOMEMORY)
        {
               CoreGraphicsDevice *device;
               int min_size;
               int pitch;
               int ret_val;
               FusionSHMPoolShared *shm_pool= NULL;
               DFBPMemCMDInfo * cmd_info = NULL;
               shm_pool = DevPMem->shared->shmpool;
              D_ASSERT(shm_pool);
              cmd_info = SHCALLOC(shm_pool,1,sizeof(DFBPMemCMDInfo));
              if(!cmd_info)
              {
                     return DFB_NOSHAREDMEMORY;
             }
              device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
              dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &min_size );

              cmd_info->min_size = min_size;
              cmd_info->pool_index = pool_idx;
              ret = DFB_OK;
              if (fusion_call_execute( &sharedData->call, FCEF_NONE, DPC_REQ_MEMBLOCK, cmd_info, &ret_val ))
              {
                     ret = DFB_FUSION;
              }
              SHFREE(shm_pool, cmd_info);
              bFirstTry = false;
              if(ret == DFB_OK)
                   goto TRY_AGAIN;
         }
          return ret;
      }

     D_MAGIC_ASSERT( chunk, Chunk );

     alloc->offset = chunk->offset;
     alloc->pitch  = chunk->pitch;
     alloc->size   = chunk->length;
     alloc->chunk  = chunk;

     D_DEBUG_AT( DevPMem_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );

     allocation->size   = alloc->size;
     allocation->offset = alloc->offset;

     ret = PMemAssureMappedMemory(DevPMem, pool_idx, chunk->block_index);

     D_ASSERT(ret == DFB_OK);

     if(surface->type & (CSTF_LAYER|CSTF_WINDOW))
     {
           unsigned char *addr = (unsigned char *)DevPMem->pmemBlockData[pool_idx][chunk->block_index].pmem_vbase
                +(alloc->offset-DevPMem->pmemBlockData[pool_idx][chunk->block_index].last_cpu_phys);
           memset(addr, 0, alloc->size);
      }

     D_MAGIC_SET( alloc, DevPMemAllocationData );

     return DFB_OK;
}

static DFBResult
DevPMemDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DevPMemPoolSharedData       *sharedData  = pool_data;
     DevPMemAllocationData *alloc = alloc_data;
     DevPMemPoolLocalData  *local = pool_local;
     int  block_index = -1;
     DevPMemData          *DevPMem = local->systemData;
     int pool_idx = _GetPoolIdx(DevPMem, pool);

     D_DEBUG_AT( DevPMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DevPMemAllocationData );
     D_ASSERT(pool_idx>=0);

     block_index = alloc->chunk->block_index ;

     D_ASSERT(block_index>=0 && block_index<POOL_BLOCKS_MAX);

     if (alloc->chunk)
     {
         dfb_surfacemanager_deallocate( sharedData->manager, alloc->chunk );
     }


     if(!dfb_surfacemanager_bMemBlockUsed(block_index  , sharedData->manager, local))
     {
         DFBPMemEvent evt;
         int ret;
         FusionSHMPoolShared *shm_pool = DevPMem->shared->shmpool;
         D_ASSERT(shm_pool);
         DFBPMemCMDInfo *cmd_info = SHCALLOC(shm_pool,1,sizeof(DFBPMemCMDInfo));

         if(!cmd_info)
         {
                D_OOM();
                return DFB_NOSHAREDMEMORY;
         }
         cmd_info->block_index = block_index;
         cmd_info->pool_index = pool_idx;

          if (fusion_call_execute( &sharedData->call, FCEF_NONE, DPC_DEL_MEMBLOCK, cmd_info, &ret ))
          {
                  D_BUG("IPC serious error");
           }
           SHFREE(shm_pool, cmd_info);
         //  evt.type = DPET_DEL_MEMBLOCK;
           //evt.block_index = block_index;
           //_DevPMemPoolDispatch(&evt, sharedData, pool_idx);

        }

     D_MAGIC_CLEAR( alloc );
     return DFB_OK;
}


static DFBResult
DevPMemMuckOut( CoreSurfacePool   *pool,
               void              *pool_data,
               void              *pool_local,
               CoreSurfaceBuffer *buffer )
{
     CoreSurface           *surface;
     DevPMemPoolSharedData        *data  = pool_data;
     DevPMemPoolLocalData   *local = pool_local;
     int          pitch  = 0;
     int        length = 0;
     CoreGraphicsDevice *device;

     D_DEBUG_AT( DevPMem_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, DevPMemPoolSharedData );
     D_MAGIC_ASSERT( local, DevPMemPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );
     dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

     return dfb_surfacemanager_displace( local->core, data->manager, buffer );

}

static DFBResult
DevPMemLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     DevPMemPoolLocalData  *local = pool_local;
     DevPMemAllocationData *alloc = alloc_data;
     DevPMemData          *DevPMem = local->systemData;
     int pool_idx = _GetPoolIdx(DevPMem, pool);
     DFBResult ret;
     unsigned char *addr;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevPMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );
     D_ASSERT(pool_idx >= 0);
     D_ASSERT(alloc->chunk);
     D_MAGIC_ASSERT(alloc->chunk, Chunk);

     D_DEBUG_AT( DevPMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

    // if(!IsPMemBlockMapped(DevPMem,  pool_idx,alloc->chunk->block_index))

     ret = fusion_skirmish_prevail( &pool->lock );
     if(ret == DFB_OK)
     {
              ret = PMemAssureMappedMemory(DevPMem, pool_idx, alloc->chunk->block_index);
              fusion_skirmish_dismiss( &pool->lock);
     }

     D_ASSERT(ret == DFB_OK);

     lock->pitch  = alloc->pitch;
     lock->offset = alloc->offset;

     addr = (unsigned char *)DevPMem->pmemBlockData[pool_idx][alloc->chunk->block_index].pmem_vbase
                +(alloc->offset-DevPMem->pmemBlockData[pool_idx][alloc->chunk->block_index].last_cpu_phys);

     lock->addr   = (void*)addr;
     lock->phys   =  alloc->offset;

     D_DEBUG_AT( DevPMem_SurfLock, "  -> offset %lu, pitch %d, addr %p, phys 0x%08lx\n",
                 lock->offset, lock->pitch, lock->addr, lock->phys );

     return DFB_OK;
}

static DFBResult
DevPMemUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     DevPMemAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, DevPMemAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( DevPMem_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     (void) alloc;

     return DFB_OK;
}



const SurfacePoolFuncs DevPMemSurfacePoolFuncs = {
     .PoolDataSize       = DevPMemPoolDataSize,
     .PoolLocalDataSize  = DevPMemPoolLocalDataSize,
     .AllocationDataSize = DevPMemAllocationDataSize,

     .InitPool           = DevPMemInitPool,
     .JoinPool           = DevPMemJoinPool,
     .DestroyPool        = DevPMemDestroyPool,
     .LeavePool          = DevPMemLeavePool,

     .TestConfig         = DevPMemTestConfig,
     .AllocateBuffer     = DevPMemAllocateBuffer,
     .DeallocateBuffer   = DevPMemDeallocateBuffer,

     .MuckOut            = DevPMemMuckOut,

     .Lock               = DevPMemLock,
     .Unlock             = DevPMemUnlock,
};






