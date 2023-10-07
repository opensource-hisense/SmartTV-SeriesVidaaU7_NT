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
#include <direct/hash.h>
#include <core/core.h>
#include <core/surface_pool.h>

#include <gfx/convert.h>
#include <linux/unistd.h>

#include <misc/conf.h>

#include "msos_mma_system.h"

#include "mtk_secure.h"

#define SHIFT32 32
#define SHIFT4 4
#define OFFSET_MASK 0xFFFFFFFF
#define CPU_PHYS_MASK 0x00000000FFFFFFFF


D_DEBUG_DOMAIN( DFBMMA_Surfaces, "DFBMMA/Surfaces", "DFBMMA Surface Pool" );

static CoreDFBShared          *mem_shared;

static u32 pipeline_id = 0;

/**********************************************************************************************************************/

DFBResult dfb_mma_alloc(mma_alloc_data *alloc_data)
{
    u32 size = alloc_data->length;
    cpu_phy bus_addr = 0x0;
    hal_phy phy_addr = 0x0;
    bool ret = false;
    int  miu_index = 0;
    bool secure = alloc_data->secure;

    D_DEBUG_MMA( DFBMMA_Surfaces, " mma_alloc size=0x%x %s(%d)\n",size,__FUNCTION__,__LINE__);

    ret = dfb_MMA_Alloc(alloc_data->u8bufTag, size, &phy_addr, &alloc_data->handle);
    if(ret == false)
    {
        printf("[DFB][MMA] Failed to mma_alloc  %s(%d)\n", __FUNCTION__, __LINE__);
        return DFB_FAILURE;
    }
    D_DEBUG_MMA( DFBMMA_Surfaces, "[DFB] MsOS_MMA_Alloc phy_addr: %llx \n ", phy_addr);

#if USE_MTK_STI
    bus_addr = phy_addr;
#else
    bus_addr = _HalAddrToBusAddr(phy_addr);
#endif

    if(bus_addr == 0)
    {
        printf("[DFB][MMA] get cpu phy address fail\n");
        return DFB_FAILURE;
    }
    D_DEBUG_MMA( DFBMMA_Surfaces, "[DFB] MsOS_MMA_Alloc bus_addr: %llx \n ", bus_addr);

    ret = dfb_MMA_Get_MIU_Index(phy_addr,  &miu_index);
    if(ret == false)
    {
        printf("[DFB][MMA]Failed to dfb_MMA_Get_MIU_Index  %s(%d)\n", __FUNCTION__, __LINE__);
        return DFB_FAILURE;
    }
    D_DEBUG_MMA( DFBMMA_Surfaces, "miu_index : %d \n", miu_index);

    if(alloc_data->initzero)
    {
        void *cpu_ptr = NULL;
        /* do mapping and PA2VA to get cpu_ptr(va) for the following memset */
        ret = dfb_MPool_Mapping(miu_index, phy_addr, size, 1);
        if(ret == false)
        {
            printf("[DFB][MMA]Failed to dfb_MPool_Mapping  %s(%d)\n", __FUNCTION__, __LINE__);
            return DFB_FAILURE;
        }

        cpu_ptr = (void *)dfb_MPool_PA2VANonCached(phy_addr);

        if ( 0 == cpu_ptr )
        {
            printf("[DFB][MMA] dfb_MPool_PA2VANonCached( %llx ) failed  ,%s(%d)\n",phy_addr ,__FUNCTION__,__LINE__);
            return DFB_FAILURE;
        }


        D_DEBUG_MMA( DFBMMA_Surfaces, "dfb_MPool_PA2VANonCached Virtual address : %x\n",  cpu_ptr);

        /* to clean buffer */
        memset(cpu_ptr, 0, size);

        /* after mapping, we need to do unmapping */
        ret = dfb_MPool_UnMapping(cpu_ptr, size);

        cpu_ptr = NULL;

        if(ret == false)
        {
            printf("[DFB][MMA]Failed to dfb_MPool_UnMapping  %s(%d)\n", __FUNCTION__, __LINE__);
            return DFB_FAILURE;
        }

    }

     if(secure)
     {
         /*============ get pipe line id ============*/
         
         if (!dfb_MMA_Secure_getPipeLineId(&pipeline_id)) {
             printf("[DFB][%s] get pipeline id failed!\n", __FUNCTION__);
             return DFB_FAILURE;
         }
         /*============ Lock Buffer============*/
         if (!dfb_MMA_Secure_LockBuf(alloc_data->u8bufTag, phy_addr, size, pipeline_id))
         {
             printf("[DFB][%s] dfb_MMA_Secure_LockBuf failed!\n", __FUNCTION__);
         }
         else
         {
             DBG_SECURE_MSG("[DFB][%s] dfb_MMA_Secure_LockBuf success!\n", __FUNCTION__);
         }
    }

    alloc_data->cpu_phys = bus_addr;
    alloc_data->hal_phys  = phy_addr;
    alloc_data->pid = getpid();

    return DFB_OK;

}


DFBResult dfb_mma_dealloc(mma_alloc_data *alloc_data)
{

    D_DEBUG_MMA( DFBMMA_Surfaces, "dfb_MMA_Free 0x%llx, size 0x%x\n", alloc_data->hal_phys, alloc_data->length );

    bool secure = alloc_data->secure;

    if(secure)
    {
        /*============1. Tee Unlock Buffer============*/
        if (!dfb_MMA_Secure_UnlockBuf(alloc_data->u8bufTag, alloc_data->hal_phys))
        {
            D_ERROR("[DFB][%s] dfb_MMA_Secure_UnlockBuf failed!\n", __FUNCTION__);
        }
        else
        {
            D_INFO("[DFB][%s] dfb_MMA_Secure_UnlockBuf success!\n", __FUNCTION__);
        }

    }

    bool ret = dfb_MMA_Free(alloc_data->hal_phys, alloc_data->length);
    if(ret == false)
    {
        D_DEBUG_MMA( DFBMMA_Surfaces, "dfb_MMA_Free failed!\n" );
        return DFB_FAILURE;
    }

    return DFB_OK;
}

DFBResult _dfb_mma_alloc(DFBMMAPoolLocalData *data, mma_alloc_data * mmaAlloc)
{
     mma_alloc_data *parameter;
     FusionSHMPoolShared *shm_pool;
     int ret;

     shm_pool = dfb_core_shmpool_data(data->systemData->core);
     D_ASSERT(shm_pool);

     parameter = SHCALLOC(shm_pool, 1, sizeof(mma_alloc_data));
     if (!parameter) {
          return D_OOSHM();
     }

     memcpy(parameter,mmaAlloc,sizeof(mma_alloc_data));
     if(fusion_call_execute(&(data->sharedData->call), FCEF_NONE, DPC_MMA_ALLOCATE, parameter, &ret ))
     {
         SHFREE(shm_pool, parameter);
         return DFB_FUSION;
     }
     memcpy(mmaAlloc,parameter,sizeof(mma_alloc_data));
     SHFREE(shm_pool, parameter);

     return ret;
}


DFBResult _dfb_mma_dealloc(DFBMMAPoolLocalData *data, mma_alloc_data * mmaAlloc)
{
    mma_alloc_data *parameter;
    FusionSHMPoolShared *shm_pool;
    int ret;

    shm_pool = dfb_core_shmpool_data(data->systemData->core);
    D_ASSERT(shm_pool);

    parameter = SHCALLOC(shm_pool, 1, sizeof(mma_alloc_data));
    if (!parameter) {
       return D_OOSHM();
    }

    memcpy(parameter,mmaAlloc,sizeof(mma_alloc_data));
    if(fusion_call_execute(&(data->sharedData->call), FCEF_NONE, DPC_MMA_DEALLOCATE, parameter, &ret ))
    {
      SHFREE(shm_pool, parameter);
      return DFB_FUSION;
    }
    SHFREE(shm_pool, parameter);

    return ret;
}

/**********************************************************************************************************************/

 static FusionCallHandlerResult
_DFBMMA_call_handler( int           caller,   /* fusion id of the caller */
                           int           call_arg, /* optional call parameter */
                           void         *call_ptr, /* optional call parameter */
                           void         *ctx,      /* optional handler context */
                           unsigned int  serial,
                           int          *ret_val )
{
    DFBMMACommand  command = call_arg;
    DFBMMAPoolLocalData          *local = ctx;
    DFBResult ret = DFB_FAILURE;

    D_MAGIC_ASSERT( local, DFBMMAPoolLocalData );
    switch (command)
    {
        case  DPC_MMA_ALLOCATE:
        {
            mma_alloc_data *mmaAlloc =(mma_alloc_data *) call_ptr;
            ret = dfb_mma_alloc(mmaAlloc);

            break;
        }

        case  DPC_MMA_DEALLOCATE:
        {
            mma_alloc_data *mmaAlloc =(mma_alloc_data *) call_ptr;

            ret = dfb_mma_dealloc(mmaAlloc);

            break;
        }

        default :
            *ret_val = DFB_BUG;

    }

    D_DEBUG_MMA( DFBMMA_Surfaces, "%s(), command = %d, ret = %d\n",
              __FUNCTION__ , command, ret);

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
DFBMMAPoolDataSize( void )
{
     return sizeof(DFBMMAPoolSharedData);
}

static int
DFBMMAPoolLocalDataSize( void )
{
     return sizeof(DFBMMAPoolLocalData);
}

static int
DFBMMAAllocationDataSize( void )
{
     return sizeof(DFBMMAAllocationData);
}


static DFBResult
DFBMMAInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DFBMMAPoolSharedData      *sharedData   = pool_data;
     DFBMMAPoolLocalData *local  = pool_local;

     char tmp_buf[32];
     D_DEBUG_MMA( DFBMMA_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( sharedData != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( ret_desc != NULL );

     mem_shared = core->shared;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL | CSTF_MIU0 |CSTF_MIU1 |CSTF_MIU2;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              = 0;

     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     sprintf(tmp_buf, "/dev/mma");

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, tmp_buf);

     D_MAGIC_SET( sharedData, DFBMMAPoolSharedData );
     D_MAGIC_SET( local, DFBMMAPoolLocalData );

     local->core = core;
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;

     fusion_call_init( &sharedData->call, _DFBMMA_call_handler, local, dfb_core_world(core) );

     fusion_skirmish_init( &sharedData->lock, "MMA POOL", dfb_core_world(core) );

     return DFB_OK;
}

static DFBResult
DFBMMAJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DFBMMAPoolSharedData      *sharedData   = pool_data;
     DFBMMAPoolLocalData *local  = pool_local;

     mem_shared = core->shared;

     D_DEBUG_MMA( DFBMMA_Surfaces, "%s(), pid = %d\n", __FUNCTION__, getpid());

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBMMAPoolSharedData );
     D_ASSERT( local != NULL );

     local->core = core;
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;

     D_MAGIC_SET( local, DFBMMAPoolLocalData );

    return DFB_OK;
}

static DFBResult
DFBMMADestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DFBMMAPoolSharedData      *sharedData  = pool_data;
     DFBMMAPoolLocalData *local = pool_local;

     D_DEBUG_MMA( DFBMMA_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBMMAPoolSharedData );
     D_MAGIC_ASSERT( local, DFBMMAPoolLocalData );

     dfb_MMA_Exit();

     fusion_call_destroy( &sharedData->call );

     fusion_skirmish_destroy( &sharedData->lock );

     D_MAGIC_CLEAR( sharedData );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DFBMMALeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DFBMMAPoolSharedData      *sharedData  = pool_data;
     DFBMMAPoolLocalData *local = pool_local;

     D_DEBUG_MMA( DFBMMA_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBMMAPoolSharedData );
     D_MAGIC_ASSERT( local, DFBMMAPoolLocalData );

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DFBMMATestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     D_DEBUG_MMA( DFBMMA_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     return DFB_OK;
}


static DFBResult
DFBMMAAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
    CoreSurface          *surface;
    DFBMMAPoolSharedData       *sharedData  = pool_data;
    DFBMMAPoolLocalData  *local = pool_local;
    DFBMMAAllocationData *alloc = alloc_data;
    DFBMMAData          *DFBMMA = local->systemData;
    DFBMMADataShared    *DFBMMAShared = DFBMMA->shared;
    CoreGraphicsDevice *device;
    int pitch = 0, length =0;
    mma_alloc_data mma_alloc;
    char *frame0 = "disp_directfb";
    char *frame1 = "directfb_frame1";
    DFBResult ret = DFB_FAILURE;

    D_DEBUG_MMA( DFBMMA_Surfaces, "%s \n", __FUNCTION__);

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( sharedData, DFBMMAPoolSharedData );
    D_MAGIC_ASSERT( local, DFBMMAPoolLocalData );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    surface = buffer->surface;

    /* FIXME: Only one global device at the moment. */
    device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
    D_ASSERT( device != NULL );

    dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length, DSCAPS_VIDEOONLY );

    if (dfb_config->mst_GPU_AFBC && (surface->type & (CSTF_LAYER | CSTF_AFBC)) ) {
        D_INFO("[DFB] %s, surface->type is Layer, origin length=0x%x\n", __FUNCTION__, length);
        length += (dfb_config->GPU_AFBC_EXT_SIZE * pitch);
        D_INFO("new length = 0x%x\n", length);
    }

    mma_alloc.length = length;

    D_DEBUG_MMA( DFBMMA_Surfaces, "mma_alloc: length=%d\n", length);

    if(surface->type & (CSTF_LAYER|CSTF_WINDOW)) {
        mma_alloc.initzero = true;
    } else {
        mma_alloc.initzero = false;
    }

    memset(&mma_alloc.u8bufTag, 0, sizeof(mma_alloc.u8bufTag));

    if(surface->type & (CSTF_MIU1|CSTF_MIU2)) {
        if(strlen(frame1) >= sizeof(mma_alloc.u8bufTag)){
            D_ERROR("[DFB] %s, %d, The size of frame1 is longer than mma_alloc bufTag\n", __FUNCTION__, __LINE__);
            return DFB_FAILURE;
        }
        strncpy(mma_alloc.u8bufTag, frame1, strlen(frame1));
    } else {
        if(strlen(frame0) >= sizeof(mma_alloc.u8bufTag)){
            D_ERROR("[DFB] %s, %d, The size of frame0 is longer than mma_alloc bufTag\n", __FUNCTION__, __LINE__);
            return DFB_FAILURE;
        }
        strncpy(mma_alloc.u8bufTag, frame0, strlen(frame0));
    }

    if(surface->config.caps & DSCAPS_SECURE_MODE)
    {
        mma_alloc.secure= true;
    }
    else
    {
        mma_alloc.secure= false;
    }

    fusion_skirmish_prevail( &sharedData->lock );

    //call mma driver to allocate buffer
    ret = _dfb_mma_alloc(local, &mma_alloc);

    fusion_skirmish_dismiss( &sharedData->lock );

    if (ret)
    {
        printf("[DFB] _dfb_mma_alloc  fail !!!!!\n");
        return DFB_NOVIDEOMEMORY;
    }

    alloc->pitch = pitch;
    alloc->size  = length;

    alloc->mma_hnd = mma_alloc.handle;
    alloc->pid = mma_alloc.pid;  //master pid
    alloc->cpu_phys = mma_alloc.cpu_phys;
    alloc->hal_phys = mma_alloc.hal_phys;

    allocation->size = alloc->size;

    D_DEBUG_MMA( DFBMMA_Surfaces, "mma_alloc: hal_phys=%llx\n", alloc->hal_phys);

    D_MAGIC_SET( alloc, DFBMMAAllocationData );

    /* 1. Record max peak usage memory of the system
       2. Record memory usage of every memory size (small, medium, large)
       3. Record max peak memory usage of every process
    */
    record_surface_memory_usage(mem_shared, alloc->size, surface);

    return DFB_OK;
}

static DFBResult
DFBMMADeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DFBMMAPoolSharedData       *sharedData  = pool_data;
     DFBMMAAllocationData *alloc = alloc_data;
     DFBMMAPoolLocalData  *local = pool_local;
     mma_alloc_data mma_alloc;
     DFBResult             ret;
     CoreSurface            *surface;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBMMAPoolSharedData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DFBMMAAllocationData );

     D_DEBUG_MMA( DFBMMA_Surfaces, "%s, pid %d,\n", __FUNCTION__, getpid() );

     mma_alloc.handle = alloc->mma_hnd;
     mma_alloc.pid = alloc->pid;
     mma_alloc.length = alloc->size;

     mma_alloc.cpu_phys = alloc->cpu_phys;
     mma_alloc.hal_phys = alloc->hal_phys;


     fusion_skirmish_prevail( &sharedData->lock );

     surface = buffer->surface;

     if(surface->config.caps & DSCAPS_SECURE_MODE)
     {
         mma_alloc.secure= true;
     }
     else
     {
         mma_alloc.secure= false;
     }

     _dfb_mma_dealloc(local, &mma_alloc);

     fusion_skirmish_dismiss( &sharedData->lock );

     D_MAGIC_CLEAR( alloc );
     return DFB_OK;
}


static DFBResult
DFBMMALock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
    DFBMMAPoolSharedData       *sharedData  = pool_data;
    DFBMMAPoolLocalData  *local = pool_local;
    DFBMMAAllocationData *alloc = alloc_data;
    DFBMMAData          *DFBMMA = local->systemData;

    int current_pid =0;
    void *cpu_addr = 0;
    bool ret = false;
    int miu_index = 0;
#if USE_MTK_STI
    u64 cpu_phys = ((alloc->cpu_phys >> SHIFT32) << SHIFT4);
#else
    u64 cpu_phys = alloc->cpu_phys;
#endif

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DFBMMAAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );
    lock->pitch = alloc->pitch;
#if USE_MTK_STI
    /* in iova, offset is not used. using offset to keep dmabuf_id.*/
    lock->offset = (alloc->cpu_phys & OFFSET_MASK);
#else
    lock->offset = 0;
#endif

    fusion_skirmish_prevail( &sharedData->lock );

    D_DEBUG_MMA(DFBMMA_Surfaces, "lock accessor %d, alloc->hal_phys: %llx\n", lock->accessor, alloc->hal_phys);

    switch (lock->accessor) {
        case CSAID_CPU:
            current_pid = getpid();

            dfb_MMA_Get_MIU_Index(alloc->hal_phys,  &miu_index);

            ret = dfb_MPool_Mapping(miu_index, alloc->hal_phys, alloc->size, 1);

            if(ret == false)
            {
                printf("[DFB][MMA]Failed to dfb_MPool_Mapping  %s(%d)\n", __FUNCTION__, __LINE__);
                fusion_skirmish_dismiss( &sharedData->lock );
                return DFB_FAILURE;
            }


            cpu_addr = (void *)dfb_MPool_PA2VANonCached(alloc->hal_phys);

            if ( 0 == cpu_addr )
            {
                printf("[DFB][MMA] dfb_MPool_PA2VANonCached( %llx ) failed, %s(%d)\n", alloc->hal_phys, __FUNCTION__, __LINE__);
                fusion_skirmish_dismiss( &sharedData->lock );
                return DFB_FAILURE;
            }


            lock->addr = cpu_addr;
            D_DEBUG_MMA( DFBMMA_Surfaces, "[DFB] DFBMMALock alloc->hal_phys: %llx, cpu_addr: %x miu_index: %d pid=%d\n", alloc->hal_phys, cpu_addr, miu_index, current_pid);


        /* fall through */

        case CSAID_LAYER0:
        case CSAID_LAYER1:
        case CSAID_LAYER2:
        case CSAID_LAYER3:
        case CSAID_LAYER4:
        case CSAID_LAYER5:
        case CSAID_LAYER6:
        case CSAID_LAYER7:
        case CSAID_GPU:

            lock->phys = (u32)(cpu_phys & CPU_PHYS_MASK);
            lock->phys_h = (u32)(cpu_phys >> SHIFT32);

            D_DEBUG_MMA( DFBMMA_Surfaces, " phys 0x%08lx, offset %d\n",  lock->phys, lock->offset);

         break;

        default:
            D_DEBUG_MMA(DFBMMA_Surfaces, "unsupported accessor %d", lock->accessor);
            fusion_skirmish_dismiss( &sharedData->lock );
            return DFB_BUG;
        }

    fusion_skirmish_dismiss( &sharedData->lock );
    return DFB_OK;
}

static DFBResult
DFBMMAUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
    DFBMMAPoolSharedData       *sharedData  = pool_data;
    DFBMMAPoolLocalData  *local = pool_local;
    DFBMMAAllocationData *alloc = alloc_data;
    int current_pid =0;
    bool ret = false;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DFBMMAAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_MMA(DFBMMA_Surfaces, "unlock accessor %d, alloc->hal_phys: %llx\n", lock->accessor, alloc->hal_phys);

    fusion_skirmish_prevail( &sharedData->lock );

    if(lock->addr && lock->accessor == CSAID_CPU) {
        current_pid = getpid();
        D_DEBUG_MMA( DFBMMA_Surfaces, "UnMapping %x, size:%d pid=%d\n", lock->addr , alloc->size, current_pid);
        ret = dfb_MPool_UnMapping(lock->addr, alloc->size);

        lock->addr = NULL;

        if(ret == false)
        {
            printf("[DFB][MMA]Failed to dfb_MPool_UnMapping  %s(%d)\n", __FUNCTION__, __LINE__);
            fusion_skirmish_dismiss( &sharedData->lock );
            return DFB_FAILURE;
        }

    }

    fusion_skirmish_dismiss( &sharedData->lock );

    return DFB_OK;
}



const SurfacePoolFuncs DFBMMASurfacePoolFuncs = {
     .PoolDataSize       = DFBMMAPoolDataSize,
     .PoolLocalDataSize  = DFBMMAPoolLocalDataSize,
     .AllocationDataSize = DFBMMAAllocationDataSize,

     .InitPool           = DFBMMAInitPool,
     .JoinPool           = DFBMMAJoinPool,
     .DestroyPool        = DFBMMADestroyPool,
     .LeavePool          = DFBMMALeavePool,

     .TestConfig         = DFBMMATestConfig,
     .AllocateBuffer     = DFBMMAAllocateBuffer,
     .DeallocateBuffer   = DFBMMADeallocateBuffer,

     .Lock               = DFBMMALock,
     .Unlock             = DFBMMAUnlock,
};
