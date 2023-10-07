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

//#define DIRECT_ENABLE_DEBUG

#include <config.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/surface_pool.h>

#include <gfx/convert.h>

#include "fbdev.h"
#include "surfacemanager.h"
#include "mi_common.h"
#include "mstarFb.h"
#include <fusion/shmalloc.h>

extern FBDev *dfb_fbdev;
extern FBDev *dfb_all_fbdev[];

D_DEBUG_DOMAIN( FBDev_Surfaces, "FBDev/Surfaces", "FBDev Framebuffer Surface Pool" );
D_DEBUG_DOMAIN( FBDev_SurfLock, "FBDev/SurfLock", "FBDev Framebuffer Surface Pool Locks" );

#define TRIPLE_BUFFER 3
#define DOUBLE_BUFFER 2
#define SINGLE_BUFFER 1

/** public **/
#define CHECK_IOCTLRET(X)  \
    { \
        if( X < 0 )  \
        { \
            printf("\33[0;33;44m%s, %d, "#X" ==> failed! \33[0m\n", __FUNCTION__, __LINE__);\
            fprintf(stderr," error msg is :%s\n",strerror(errno));\
            return false;\
        } \
    }

typedef struct {
    int fb_id;
    void *arg;
}FbdevCallParameter;

/**********************************************************************************************************************/

typedef struct {
     int             magic;

     SurfaceManager *manager;
} FBDevPoolData;

typedef struct {
     int             magic;

     CoreDFB        *core;
     void           *mem;
} FBDevPoolLocalData;

typedef struct {
     int    magic;

     Chunk *chunk;
} FBDevAllocationData;

/**********************************************************************************************************************/

static int
fbdevPoolDataSize( void )
{
     return sizeof(FBDevPoolData);
}

static int
fbdevPoolLocalDataSize( void )
{
     return sizeof(FBDevPoolLocalData);
}

static int
fbdevAllocationDataSize( void )
{
     return sizeof(FBDevAllocationData);
}

int
fbdev_iommu_ioctl( int request, void *arg, int arg_size, int layer_id )
{
     int          ret = 0;
     int          erno = 0;
     void        *tmp_shm = NULL;
     FBDevShared *shared;
     FbdevCallParameter *param;

     shared = dfb_all_fbdev[layer_id]->shared;

     D_ASSERT( shared != NULL );

     param = SHMALLOC( shared->shmpool, sizeof(*param));
     if(!param)
       return DFB_NOSHAREDMEMORY;

     param->fb_id = shared->layer_id;

     if (arg) {
          if (!fusion_is_shared( dfb_core_world(dfb_all_fbdev[layer_id]->core), arg )) {
               tmp_shm = SHMALLOC( shared->shmpool, arg_size );
               if (!tmp_shm) {
                    D_ERROR("[DFB] %s, %d, SHMALLOC fail, errno = ENOMEM\n ", __FUNCTION__, __LINE__);
                    errno = ENOMEM;
                    return -1;
               }

               direct_memcpy( tmp_shm, arg, arg_size );
               param->arg = tmp_shm;
          }
          else
               param->arg = arg;
     }

     DBG_LAYER_MSG("[DFB] %s, %d, call fbdev_ioctl_call_handler, request: %x, arg : %p, pid=%d\n",
                   __FUNCTION__, __LINE__, request, arg, getpid() );

     ret = fusion_call_execute( &shared->fbdev_ioctl, FCEF_NONE,
                                request, param, &erno );

     if (tmp_shm) {
          direct_memcpy( arg, tmp_shm, arg_size );
          SHFREE( shared->shmpool, tmp_shm );
     }

     SHFREE(shared->shmpool, param);

     errno = erno;

     return errno ? -1 : 0;
}

static bool dfb_fbdev_iommu_allocate(CoreSurface *surface, long MemLength)
{
    FBDevShared *shared = dfb_all_fbdev[surface->resource_id]->shared;

    // fbdev iommu allocate mem.
    CHECK_IOCTLRET( FBDEV_IOMMU_IOCTL( FBIO_AllOCATE_IOMMU, &MemLength, surface->resource_id) );

    if ( dfb_core_is_master( dfb_all_fbdev[surface->resource_id]->core ) == false  ) {
         // get new buffer address from iommu allocate.
         CHECK_IOCTLRET( ioctl( dfb_all_fbdev[surface->resource_id]->fd, FBIOGET_FSCREENINFO, &shared->fix ) );
    }

    DBG_LAYER_MSG("[DFB] %s, layer %d, smem_start = 0x%lx, smem_len = %x, pitch = %d, pid=%d\n",
                      __FUNCTION__, surface->resource_id, shared->fix.smem_start, shared->fix.smem_len, shared->fix.line_length, getpid());

    return true;
}

static void dfb_fbdev_iommu_free(CoreSurface *surface, long MemLength)
{
    FBDevShared *shared = dfb_all_fbdev[surface->resource_id]->shared;

    DBG_LAYER_MSG("[DFB] %s, smem_start = 0x%lx, smem_len = %x, pid=%d\n",
        __FUNCTION__, shared->fix.smem_start, shared->fix.smem_len, getpid());

    if (FBDEV_IOMMU_IOCTL( FBIO_FREE_IOMMU, &MemLength, surface->resource_id) < 0) {
        D_ERROR( "[DFB] %s, Could not free iommu mem for fbdev!\n", __FUNCTION__);
        close( dfb_all_fbdev[surface->resource_id]->fd );
    }
    // reset fix
    dfb_fbdev->shared->fix.smem_start = 0;
    dfb_fbdev->shared->fix.smem_len = 0;
}

static DFBResult
fbdevInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult           ret;
     FBDevPoolData      *data  = pool_data;
     FBDevPoolLocalData *local = pool_local;

     D_DEBUG_AT( FBDev_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( data != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( ret_desc != NULL );

     DBG_LAYER_MSG("%s, %d, dfb_fbdev:%p, shared->fix.smem_start=%lx, shared->fix.smem_len=%x\n", __FUNCTION__, __LINE__, dfb_fbdev, dfb_fbdev->shared->fix.smem_start, dfb_fbdev->shared->fix.smem_len);
     ret = dfb_surfacemanager_create( core, dfb_fbdev->shared->fix.smem_len, &data->manager );
     if (ret)
          return ret;

     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_EXTERNAL;
     ret_desc->priority          = CSPP_PREFERED;//CSPP_DEFAULT;

     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "Frame Buffer Memory" );

     local->core = core;

     D_MAGIC_SET( data, FBDevPoolData );
     D_MAGIC_SET( local, FBDevPoolLocalData );


     D_ASSERT( dfb_fbdev != NULL );
     D_ASSERT( dfb_fbdev->shared != NULL );

     local->mem = dfb_fbdev->framebuffer_base;
     dfb_fbdev->shared->manager = data->manager;

     return DFB_OK;
}

static DFBResult
fbdevJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     FBDevPoolData      *data  = pool_data;
     FBDevPoolLocalData *local = pool_local;

     D_DEBUG_AT( FBDev_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_ASSERT( local != NULL );
     D_ASSERT( dfb_fbdev != NULL );

     (void) data;

     local->core = core;
     local->mem = dfb_fbdev->framebuffer_base;

     D_MAGIC_SET( local, FBDevPoolLocalData );

     return DFB_OK;
}

static DFBResult
fbdevDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     FBDevPoolData      *data  = pool_data;
     FBDevPoolLocalData *local = pool_local;

     D_DEBUG_AT( FBDev_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_MAGIC_ASSERT( local, FBDevPoolLocalData );

     dfb_surfacemanager_destroy( data->manager );

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
fbdevLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     FBDevPoolData      *data  = pool_data;
     FBDevPoolLocalData *local = pool_local;

     D_DEBUG_AT( FBDev_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_MAGIC_ASSERT( local, FBDevPoolLocalData );

     (void) data;

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
fbdevTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     DFBResult           ret;
     CoreSurface        *surface;
     FBDevPoolData      *data  = pool_data;
     FBDevPoolLocalData *local = pool_local;

     D_DEBUG_AT( FBDev_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_MAGIC_ASSERT( local, FBDevPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if ((surface->type & CSTF_LAYER) /*&& surface->resource_id == DLID_PRIMARY*/)
          return DFB_OK;
     else
          return DFB_NOVIDEOMEMORY;

     ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL, NULL );

     D_DEBUG_AT( FBDev_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );

     return ret;
}

void * fbdev_mpool_mapping(unsigned long fb_hal_addr , unsigned long fb_mem_len , int bIommuUsed)
{
     void *pvirtual_address = NULL;

     bool ret = 0;

     unsigned long miu_offset;
     unsigned long offset;
     int miu_select = 0;

     if (bIommuUsed)
     {
         hal_phy fb_addr = (0x200000000 | fb_hal_addr);

         DFB_UTOPIA_TRACE(ret = dfb_MPool_Mapping( miu_select, fb_addr, fb_mem_len, 1));

         if( !ret )
         {
             printf("[DFB] Waring! map fbdev buf error! \n");
             return NULL;
         }

         DFB_UTOPIA_TRACE(pvirtual_address = (void *)dfb_MPool_PA2VANonCached( fb_addr ));
         if( !pvirtual_address )
         {
             D_PERROR("Error, dfb_MPool_PA2VANonCached failed %s(%d) \n",__FUNCTION__,__LINE__);
             return NULL;
         }
     }
     else
     {
         DFB_UTOPIA_TRACE(pvirtual_address = (void *)dfb_MPool_PA2VANonCached( fb_hal_addr ));

         if(NULL == pvirtual_address)
         {
             if ( fb_hal_addr >= dfb_config->mst_miu2_cpu_offset &&  dfb_config->mst_miu2_cpu_offset != 0)
             {
                 miu_offset = dfb_config->mst_miu2_hal_offset;
                 miu_select = 2;
             }
             else if ( fb_hal_addr >= dfb_config->mst_miu1_hal_offset &&  dfb_config->mst_miu1_hal_offset != 0)
             {
                 miu_offset = dfb_config->mst_miu1_hal_offset;
                 miu_select = 1;
             }
             else
             {
                 miu_offset = dfb_config->mst_miu0_hal_offset;
                 miu_select = 0;
             }

             offset = (fb_hal_addr - miu_offset);

             DBG_LAYER_MSG("[DFB] dfb_MPool_Mapping offset:%lx, dfb_config->mst_gop_regdmaphys_len:%lx \n", offset, fb_mem_len);

             DFB_UTOPIA_TRACE(ret = dfb_MPool_Mapping( miu_select, offset, fb_mem_len, 1));

             if( !ret )
             {
                 printf("[DFB] Waring! map fbdev buf error! \n");
                 return NULL;
             }

             DFB_UTOPIA_TRACE(pvirtual_address = (void *)dfb_MPool_PA2VANonCached( fb_hal_addr ));
             if( !pvirtual_address )
             {
                D_PERROR("Error, dfb_MPool_PA2VANonCached failed %s(%d) \n",__FUNCTION__,__LINE__);
                return NULL;
             }
         }
     }

     return pvirtual_address;
}
static DFBResult
fbdevAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     DFBResult            ret = DFB_OK;
     CoreSurface         *surface;
     FBDevPoolData       *data  = pool_data;
     FBDevPoolLocalData  *local = pool_local;
     FBDevAllocationData *alloc = alloc_data;
     unsigned int need_mem;
     int buffers;

     D_DEBUG_AT( FBDev_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_MAGIC_ASSERT( local, FBDevPoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     FBDevShared *shared = dfb_all_fbdev[surface->resource_id]->shared;
     D_MAGIC_ASSERT( surface, CoreSurface );
     if (surface->type & CSTF_LAYER ) {
          DBG_LAYER_MSG("[DFB] %s (%d)  -> primary layer buffer (index %d)\n", __FUNCTION__, __LINE__, dfb_surface_buffer_index( buffer ) );

          dfb_surface_calc_buffer_size( surface, 8, 1, NULL, &allocation->size );

          if (shared->iommu) {
              if (surface->config.caps & DSCAPS_TRIPLE)
                  buffers = TRIPLE_BUFFER;
              else if (surface->config.caps & DSCAPS_DOUBLE)
                  buffers = DOUBLE_BUFFER;
              else
                  buffers = SINGLE_BUFFER;

              need_mem = allocation->size * buffers;
              /* Set for AFBC */
              if (dfb_config->mst_GPU_AFBC && (surface->type & CSTF_AFBC)) 
              {
                  const u32 MASK = 0xfff;
                  u32 extsize = (surface->config.size.h > dfb_config->GPU_AFBC_EXT_SIZE)? dfb_config->GPU_AFBC_EXT_SIZE : surface->config.size.h;
                  need_mem += (extsize * DFB_BYTES_PER_LINE( DSPF_ARGB, surface->config.size.w )  * buffers);
                  /* align mem to 4k size base */
                  need_mem = (((need_mem)+(MASK))&~(MASK));
              }
              DBG_LAYER_MSG("[DFB] %s, allocation->size=%x, need_mem = %x, fix.smem_len = %x\n", __FUNCTION__, allocation->size, need_mem, shared->fix.smem_len);

              if (shared->fix.smem_len != need_mem) {
                   /* memory allocate & secure lock*/
                  dfb_fbdev_iommu_free(surface, shared->fix.smem_len);
                  dfb_fbdev_iommu_allocate(surface, need_mem);
              } else if (shared->bSecure != shared->bSecure_current) {
                  /* Only secure lock/unlock */
                  dfb_fbdev_iommu_free(surface, 0);
                  dfb_fbdev_iommu_allocate(surface, 0);
              }
          }

     }
     else {
          DBG_LAYER_MSG("[DFB] %s, %d, not support non-layer buffer allocate!\n", __FUNCTION__, __LINE__);
		  ret = DFB_NOVIDEOMEMORY;
	 }

     D_MAGIC_SET( alloc, FBDevAllocationData );

     return ret;
}

static DFBResult
fbdevDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     FBDevPoolData       *data  = pool_data;
     FBDevAllocationData *alloc = alloc_data;

     D_DEBUG_AT( FBDev_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, FBDevPoolData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, FBDevAllocationData );

     if (alloc->chunk)
          dfb_surfacemanager_deallocate( data->manager, alloc->chunk );

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}

static DFBResult
fbdevLock( CoreSurfacePool       *pool,
           void                  *pool_data,
           void                  *pool_local,
           CoreSurfaceAllocation *allocation,
           void                  *alloc_data,
           CoreSurfaceBufferLock *lock )
{
     CoreSurface         *surface;
     FBDevAllocationData *alloc  = alloc_data;
     const int RETRY_LIMIT = 5;
     const int MSEC = 1000;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, FBDevAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( FBDev_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );

     surface = allocation->surface;
     FBDevShared *shared = dfb_all_fbdev[surface->resource_id]->shared;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if (surface->type & CSTF_LAYER /*&& surface->resource_id == DLID_PRIMARY*/) {
          int index  = dfb_surface_buffer_index( allocation->buffer );

          D_DEBUG_AT( FBDev_Surfaces, "  -> primary layer buffer (index %d)\n", index );

          /* Check fbdev buffer length is > 0 or retry */
          for (int i = 0; i < RETRY_LIMIT; i++) {
              if (shared->fix.smem_len != 0) {
                  break;
              };
              D_ERROR("memory length is invalid!Retry %d time!\n", i+1);
              usleep(MSEC);
          }
          if (shared->fix.smem_len == 0) {
              D_ERROR("Retry failed!Check memory allocation is correct..\n");
              return DFB_FAILURE;
          }
	  
          lock->pitch  = shared->fix.line_length;

          /* Set for AFBC using */
          if (dfb_config->mst_GPU_AFBC && (surface->type & CSTF_AFBC))          
          {
              u32 extsize = (surface->config.size.h > dfb_config->GPU_AFBC_EXT_SIZE)? dfb_config->GPU_AFBC_EXT_SIZE : surface->config.size.h;		  
              lock->offset = index * (surface->config.size.h + extsize) * lock->pitch;
          }
          else
              lock->offset = index * surface->config.size.h * lock->pitch;
     }
     else {
          D_MAGIC_ASSERT( alloc->chunk, Chunk );

          lock->pitch  = alloc->chunk->pitch;
          lock->offset = alloc->chunk->offset;
     }

     if  (lock->accessor == CSAID_CPU) {
         dfb_all_fbdev[surface->resource_id]->framebuffer_base = fbdev_mpool_mapping( shared->fix.smem_start, shared->fix.smem_len, shared->iommu );
         if(dfb_all_fbdev[surface->resource_id]->framebuffer_base == NULL) {
             return DFB_FAILURE;
         }
    }

     DBG_LAYER_MSG("[DFB] %s, surface->resource_id=%lu, framebuffer_base : %p, smem_start : %lx\n", __FUNCTION__, surface->resource_id,
                            dfb_all_fbdev[surface->resource_id]->framebuffer_base, shared->fix.smem_start);
     lock->addr = dfb_all_fbdev[surface->resource_id]->framebuffer_base + lock->offset;
     // lock->phys must be bus address.
     lock->phys = shared->fix.smem_start + lock->offset;
     lock->phys_h = (shared->iommu)? 0x2 : 0x0;

     cpu_phy bus_addr = _HalAddrToBusAddr(((hal_phy)lock->phys_h << 32) | lock->phys);

     lock->phys = (u32)(bus_addr & 0x00000000FFFFFFFF);
     lock->phys_h = (u32)(bus_addr >> 32);
     DBG_LAYER_MSG("[DFB] %s, -> offset %u, pitch %d, addr %p, phys_h 0x%lx, phys 0x%08x\n",
                  __FUNCTION__, lock->offset, lock->pitch, lock->addr, lock->phys_h, lock->phys );

     return DFB_OK;
}

static DFBResult
fbdevUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     FBDevAllocationData *alloc = alloc_data;
     CoreSurface         *surface;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, FBDevAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( FBDev_SurfLock, "%s( %p )\n", __FUNCTION__, lock->buffer );
     surface = allocation->surface;

     FBDevShared *shared = dfb_all_fbdev[surface->resource_id]->shared;

     if  (lock->accessor == CSAID_CPU) {
         /* No need to unmap when it is in MMAP mode */
         if(shared->iommu && dfb_all_fbdev[surface->resource_id]->framebuffer_base != NULL){
             dfb_MPool_UnMapping( dfb_all_fbdev[surface->resource_id]->framebuffer_base, shared->fix.smem_len );
             dfb_all_fbdev[surface->resource_id]->framebuffer_base = NULL;
         }
     }
     (void) alloc;

     return DFB_OK;
}

const SurfacePoolFuncs fbdevSurfacePoolFuncs = {
     .PoolDataSize       = fbdevPoolDataSize,
     .PoolLocalDataSize  = fbdevPoolLocalDataSize,
     .AllocationDataSize = fbdevAllocationDataSize,

     .InitPool           = fbdevInitPool,
     .JoinPool           = fbdevJoinPool,
     .DestroyPool        = fbdevDestroyPool,
     .LeavePool          = fbdevLeavePool,

     .TestConfig         = fbdevTestConfig,
     .AllocateBuffer     = fbdevAllocateBuffer,
     .DeallocateBuffer   = fbdevDeallocateBuffer,

     .Lock               = fbdevLock,
     .Unlock             = fbdevUnlock,
};

