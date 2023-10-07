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

#include <directfb.h>

#include <direct/mem.h>

#include <fusion/arena.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/surface_pool.h>

#include <misc/conf.h>


#include "devmem.h"
#include "surfacemanager.h"
#include "msos_mma_system.h"


#include <core/core_system.h>


DFB_CORE_SYSTEM( devmem )


/**********************************************************************************************************************/

static DevMemData *m_data;    /* FIXME: Fix Core System API to pass data in all functions. */

/**********************************************************************************************************************/

static void*
system_MPoolPA2VANonCached(cpu_phy pAddrPhys)
{
    return dfb_MPool_PA2VANonCached(pAddrPhys);
}

static bool
system_MPoolMapping(u8 u8MiuSel, cpu_phy Offset, u32 u32MapSize, bool  bNonCache)
{
    return dfb_MPool_Mapping(u8MiuSel, Offset, u32MapSize, bNonCache);
}

static bool
system_MPoolUnMapping(void* ptrVirtStart, u32  u32MapSize)
{
    return dfb_MPool_UnMapping(ptrVirtStart, u32MapSize);
}

static DFBResult
MapMemByMmap( DevMemData    *data,
              cpu_phy  mem_phys,
              unsigned int   mem_length,
              hal_phy  reg_phys,
              unsigned int   reg_length,
              bool secondary)
{
    int fd;
    void *tmem;
    
    fd = open( DEV_MEM, O_RDWR | O_SYNC );

    if (fd < 0) {
        D_PERROR( "System/DevMem: Opening '%s' failed!\n", DEV_MEM );
        return DFB_INIT;
    }

    tmem = mmap( NULL, mem_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mem_phys );
    
    if (tmem == MAP_FAILED) {
        D_PERROR( "System/DevMem: Mapping %d bytes at 0x%08llx via '%s' failed!\n", mem_length, mem_phys, DEV_MEM );
        close( fd );
        return DFB_INIT;
    }
    
    if (!secondary)
        data->mem = tmem;
    else
        data->mem_secondary = tmem;

    if (!secondary && reg_phys && reg_length) {
        data->reg = mmap( NULL, reg_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, reg_phys );
        if (data->reg == MAP_FAILED) {
            D_PERROR( "System/DevMem: Mapping %d bytes at 0x%08llx via '%s' failed!\n", reg_length, reg_phys, DEV_MEM );
            munmap( data->mem, mem_length );
            close( fd );
            return DFB_INIT;
        }
    }
    
    close( fd );
    return DFB_OK;
}


static DFBResult
MapMemByMsos( DevMemData    *data,
              cpu_phy  mem_phys,
              unsigned int   mem_length,
              hal_phy  reg_phys,
              unsigned int   reg_length,
              bool secondary )
{
    void *pvirtual_address = NULL;
    hal_phy miu_offset;
    cpu_phy cpu_offset;
    int miu_select = 0;
    bool bmpool_init = false;
    bmpool_init = dfb_MPool_Init();
    if ( !bmpool_init)
    {
        printf("%s, %d dfb_MPool_Init fail\n", __FUNCTION__, __LINE__);
        return DFB_INIT;
    }

    if (dfb_config->mst_miu2_cpu_offset != 0 && mem_phys >= dfb_config->mst_miu2_cpu_offset ) {
        miu_offset = dfb_config->mst_miu2_hal_offset;
        cpu_offset = dfb_config->mst_miu2_cpu_offset;
        miu_select = 2;
    }
    else if (dfb_config->mst_miu1_cpu_offset != 0 && mem_phys >= dfb_config->mst_miu1_cpu_offset ) {
        miu_offset = dfb_config->mst_miu1_hal_offset;
        cpu_offset = dfb_config->mst_miu1_cpu_offset;
        miu_select = 1;
    }
    else {
        miu_offset = dfb_config->mst_miu0_hal_offset;
        cpu_offset = dfb_config->mst_miu0_cpu_offset;
        miu_select = 0;
    }
    pvirtual_address = (void *)dfb_MPool_PA2VANonCached(mem_phys - cpu_offset +  miu_offset);
    if( !pvirtual_address )
    {
        //require MMAP 4k alignment(offset , len), otherwise may be cause some error
        if( !system_MPoolMapping(miu_select, (mem_phys - cpu_offset), mem_length, 1 ) )
        {
            D_PERROR("DFB Error,msos mapping failed %s(%d) \n",__FUNCTION__,__LINE__);
            return DFB_FAILURE;
        }
        pvirtual_address = (void *)dfb_MPool_PA2VANonCached(mem_phys - cpu_offset +  miu_offset);
        if( !pvirtual_address )
        {
            D_PERROR("Error, MsOS_MPool_PA2KSEG1 failed %s(%d) \n",__FUNCTION__,__LINE__);
            return DFB_FAILURE;
        }
    }
    if ( !secondary )
        data->mem = pvirtual_address;
    else
        data->mem_secondary = pvirtual_address;
    
    return DFB_OK;
}

#if USE_MSTAR_CMA

static DFBResult
MapMemByCMA( DevMemData    *data,
              unsigned long  mem_phys,
              unsigned int   mem_length,
              unsigned long  reg_phys,
              unsigned int   reg_length,
              bool secondary )
{
     dfb_MPool_Init();

     DFBResult ret;

     ret = dfb_CMA_MapMem(data, mem_phys, mem_length, reg_phys, reg_length, secondary);

     return ret;
}
#endif


static DFBResult
MapMemAndReg( DevMemData    *data,
              cpu_phy  mem_phys,
              unsigned int   mem_length,
              hal_phy  reg_phys,
              unsigned int   reg_length,
              bool           master)
{
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_MSOS_POOL)
        return MapMemByMsos(data, mem_phys, mem_length, reg_phys, reg_length, false);

#if USE_MSTAR_CMA    
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL)
    {
        //if the proccess is master , will get mem by cma,  otherwise(slave) by msos_mpool
        if(master)
        return MapMemByCMA(data, mem_phys, mem_length, reg_phys, reg_length, false);
    else
        return MapMemByMsos(data, mem_phys, mem_length, reg_phys, reg_length, false);
    }
#endif

    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_DEVMEM)
        return MapMemByMmap(data,mem_phys, mem_length, reg_phys, reg_length, false);

    return DFB_FAILURE;
}



static void
UnmapMemAndReg( DevMemData   *data,
                unsigned int  mem_length,
                unsigned int  reg_length ,
                bool master)
{
    D_ASSERT( data != NULL );
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_DEVMEM)
    {
        munmap( data->mem, mem_length );
        if (reg_length)
            munmap( (void*) data->reg, reg_length );
    }
#ifdef ARCH_ARM
    else if (dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_MSOS_POOL)
    {
        dfb_MPool_UnMapping(data->mem, mem_length);
    }
#endif
#if USE_MSTAR_CMA
    else
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL && master)
    {
        dfb_CMA_PutMem(data, mem_length);

        D_INFO("\033[31m %s, %d, call MApi_CMA_Pool_PutMem!!\033[m\n",__PRETTY_FUNCTION__, __LINE__);
    }
    else if (dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL) //slave process
    {
        dfb_MPool_UnMapping(data->mem, mem_length);
    }
#endif
}


///-------------------------------------------------------------
///Map the secondary video memory
///@param data \b OUT:data->secondary record the virtual address:
///@param mem_phys \b IN:physical address
///@param mem_length \b IN: size in byte
///@return DFBResult
///---------------------------------------------------------------


static DFBResult
_MapMemSecondary(DevMemData    *data,
              cpu_phy  mem_phys,
              unsigned int   mem_length, 
              bool master )
{
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_MSOS_POOL)
        return MapMemByMsos(data, mem_phys, mem_length, 0, 0, true);

#if USE_MSTAR_CMA
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL)
    {
        //if the proccess is master , will get mem by cma,  otherwise(slave) by msos_mpool
        if(master)
        return MapMemByCMA(data, mem_phys, mem_length, 0, 0, true);
    else
        return MapMemByMsos(data,mem_phys, mem_length, 0, 0, true);
    }
#endif

    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_DEVMEM)
        return MapMemByMmap(data, mem_phys, mem_length, 0, 0, true);

    return DFB_OK;
}


///----------------------------------------------------------------
///unmap the secondary video memory
///@param data \b IN data->mem_secondary record the virtual address
///@param mem_length \b IN  Size in byte
///@return void
///-----------------------------------------------------------------
static void
_UnmapMemSecondary( DevMemData   *data,
                unsigned int  mem_length,
                bool master )
{
    D_ASSERT( data != NULL );

    if( dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_DEVMEM)
        munmap(data->mem_secondary, mem_length );
#ifdef ARCH_ARM
    else if (dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_MSOS_POOL)
        dfb_MPool_UnMapping(data->mem_secondary, mem_length);
#endif
#if USE_MSTAR_CMA
    else
    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL && master)
    {
        dfb_CMA_PutMem(data->shared->sec_pool_handle_id, data->shared->sec_offset_in_pool, mem_length);
    }
    else if (dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL) //slave process
    {
        dfb_MPool_UnMapping(data->mem_secondary, mem_length);
    }
#endif
}


/**********************************************************************************************************************/

static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_DEVMEM;
     info->caps = CSCAPS_ACCELERATION;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "DevMem" );
}

static DFBResult
devmem_system_initialize( CoreDFB *core, void **ret_data )
{
     DFBResult            ret;
     DevMemData          *data;
     DevMemDataShared    *shared;
     FusionSHMPoolShared *pool;

     D_ASSERT( m_data == NULL );

     if (dfb_config->bus_address_check == true)
     {
         cpu_phy u32BA0 =0, u32BA1=0, u32BA2=0;

         u32BA0 = dfb_MsOSPA2BA(dfb_config->mst_miu0_hal_offset);
         u32BA1 = dfb_MsOSPA2BA(dfb_config->mst_miu1_hal_offset);
         u32BA2 = dfb_MsOSPA2BA(dfb_config->mst_miu2_hal_offset);

         check_bus_address_error(u32BA0, u32BA1,  u32BA2);
     }

     if (!dfb_config->video_phys || !dfb_config->video_length) {
          D_ERROR( "System/DevMem: Please supply 'video-phys = 0xXXXXXXXX' and 'video-length = XXXX' options!\n" );
          return DFB_INVARG;
     }

     if (dfb_config->video_phys_secondary_cpu && !dfb_config->video_length_secondary) {
        if(dfb_config->video_phys_secondary_cpu == dfb_config->mst_miu0_cpu_offset)
        {
            //0x40000000 case
        }
        else
        {
          D_ERROR( "System/DevMem: Please supply 'video-phys_secondary = 0xXXXXXXXX' and 'video-length_secondary = XXXX' options!\n" );
          return DFB_INVARG;
        }
     }

     if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
          D_ERROR( "System/DevMem: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
          return DFB_INVARG;
     }

     data = D_CALLOC( 1, sizeof(DevMemData) );
     if (!data)
          return D_OOM();

     pool = dfb_core_shmpool( core );

     shared = SHCALLOC( pool, 1, sizeof(DevMemDataShared) );
     if (!shared) {
          D_FREE( data );
          return D_OOSHM();
     }

     shared->shmpool = pool;

     data->shared = shared;

     ret = MapMemAndReg( data,
                         dfb_config->video_phys, dfb_config->video_length,
                         dfb_config->mmio_phys,  dfb_config->mmio_length, true );
     if (ret) {
          SHFREE( pool, shared );
          D_FREE( data );
          return ret;
     }

     *ret_data = m_data = data;
     dfb_surface_pool_initialize( core, &devmemSurfacePoolFuncs, &shared->pool );


     if(dfb_config->video_phys_secondary_cpu &&dfb_config->video_length_secondary)
     {
         if(((dfb_config->video_phys >= dfb_config->mst_miu2_cpu_offset) && (dfb_config->video_phys_secondary_cpu >= dfb_config->mst_miu2_cpu_offset))
             ||((dfb_config->video_phys >=dfb_config->mst_miu1_cpu_offset)&&(dfb_config->video_phys_secondary_cpu >=dfb_config->mst_miu1_cpu_offset))
             ||((dfb_config->video_phys <dfb_config->mst_miu1_cpu_offset)&&(dfb_config->video_phys_secondary_cpu <dfb_config->mst_miu1_cpu_offset)))
         {
             printf("\n==========the two video memory pool in the same miu=================\n");
         }

         ret = _MapMemSecondary(data, dfb_config->video_phys_secondary_cpu, dfb_config->video_length_secondary, true);
         if (ret) {
               
             SHFREE( pool, shared );
             D_FREE( data );
             return ret;
         }

         dfb_surface_pool_initialize(core, & devmemSurfacePoolSecondaryFuncs, &shared->pool_secondary);
     }
          
     dfb_surface_pool_initialize(core,&preallocInVidSurfacePoolFuncs,&shared->preAllocInVideoSurfacePool);
     fusion_arena_add_shared_field( dfb_core_arena( core ), "devmem", shared );

     return DFB_OK;
}

static DFBResult
msosmma_system_initialize( CoreDFB *core, void **ret_data )
{
    DFBMMAData       *mma;
    DFBMMADataShared *shared;
    FusionSHMPoolShared *pool;
    int ret = 0;

    mma = D_CALLOC( 1, sizeof(DFBMMAData) );
    if (!mma)
        return D_OOM();

    D_MAGIC_SET(mma, DFBMMAData);
    mma->core = core;

    pool = dfb_core_shmpool( core );

    shared = SHCALLOC( pool, 1, sizeof(DFBMMADataShared) );
    if (!shared) {
        D_FREE( mma );
        return D_OOSHM();
    }

    D_MAGIC_SET(shared, DFBMMADataShared);
    shared->shmpool = pool;

    mma->shared = shared;
    /*
    * Must be set before initializing the pools!
    */
     *ret_data = mma;

    /*
    * Master init
    */
    dfb_surface_pool_initialize( core, &DFBMMASurfacePoolFuncs, &shared->pool);
    dfb_surface_pool_initialize(core,&preallocInVidSurfacePoolFuncs,&shared->preallocpool);

    fusion_arena_add_shared_field( dfb_core_arena( core ), "MSOS_MMA", shared );

    return DFB_OK;
}

static DFBResult
system_initialize( CoreDFB *core, void **ret_data )
{
    if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
    {
        if( dfb_MMA_IsMMAEnabled() )
            return msosmma_system_initialize(core, ret_data);
    }

    return devmem_system_initialize(core, ret_data);
}

static DFBResult
devmem_system_join( CoreDFB *core, void **ret_data )
{
     DFBResult         ret;
     void             *tmp;
     DevMemData       *data;
     DevMemDataShared *shared;
     
     D_ASSERT( m_data == NULL );

     if (!dfb_config->video_phys || !dfb_config->video_length) {
          D_ERROR( "System/DevMem: Please supply 'video-phys = 0xXXXXXXXX' and 'video-length = XXXX' options!\n" );
          return DFB_INVARG;
     }

     if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
          D_ERROR( "System/DevMem: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
          return DFB_INVARG;
     }

     data = D_CALLOC( 1, sizeof(DevMemData) );
     if (!data)
          return D_OOM();

     ret = fusion_arena_get_shared_field( dfb_core_arena( core ), "devmem", &tmp );
     if (ret) {
          D_FREE( data );
          return ret;
     }

     data->shared = shared = tmp;

     ret = MapMemAndReg( data,
                         dfb_config->video_phys, dfb_config->video_length,
                         dfb_config->mmio_phys,  dfb_config->mmio_length, false );
     if (ret) {
          D_FREE( data );
          return ret;
     }

     *ret_data = m_data = data;
     dfb_surface_pool_join(core, shared->pool, &devmemSurfacePoolFuncs );

     if(dfb_config->video_phys_secondary_cpu && dfb_config->video_length_secondary)
     {
         ret = _MapMemSecondary(data, dfb_config->video_phys_secondary_cpu, dfb_config->video_length_secondary, false);
         if (ret)
         {
             D_FREE( data );
             return ret;
         }
         dfb_surface_pool_join(core, shared->pool_secondary, &devmemSurfacePoolSecondaryFuncs );
     }

     dfb_surface_pool_join(core,shared->preAllocInVideoSurfacePool,&preallocInVidSurfacePoolFuncs);

     return DFB_OK;
}

static DFBResult
msosmma_system_join( CoreDFB *core, void **data )
{
     DFBResult     ret;
     void         *ptr;
     DFBMMAData *mma;
     DFBMMADataShared *shared;

     mma = D_CALLOC( 1, sizeof(DFBMMAData) );
     if (!mma)
          return D_OOM();

     D_MAGIC_SET(mma, DFBMMAData);
     mma->core = core;

     fusion_arena_get_shared_field( dfb_core_arena( core ), "MSOS_MMA", &ptr );

     mma->shared = shared = ptr;

     /*
      * Must be set before joining the pools!
      */

     *data = mma;

     /*
      * Slave init
      */
     dfb_surface_pool_join( core, shared->pool, &DFBMMASurfacePoolFuncs );
     dfb_surface_pool_join( core, shared->preallocpool, &preallocInVidSurfacePoolFuncs);

     return DFB_OK;
}

static DFBResult
system_join( CoreDFB *core, void **ret_data )
{
    if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
    {
        if( dfb_MMA_IsMMAEnabled() )
            return msosmma_system_join(core, ret_data);
    }

    return devmem_system_join(core, ret_data);
}

static DFBResult
devmem_system_shutdown( bool emergency )
{
     DevMemDataShared *shared;

     D_ASSERT( m_data != NULL );

     shared = m_data->shared;
     D_ASSERT( shared != NULL );
     D_INFO("\033[31mWaring: dfb master will be exit!!!  %s(%d) \033[m\n",__FUNCTION__,__LINE__);
     dfb_surface_pool_destroy( shared->pool );
     UnmapMemAndReg( m_data, dfb_config->video_length, dfb_config->mmio_length, true );

     if(shared->pool_secondary)
     {
         dfb_surface_pool_destroy(shared->pool_secondary);
         _UnmapMemSecondary(m_data,dfb_config->video_length_secondary, true);     
     }
     
     dfb_surface_pool_destroy(shared->preAllocInVideoSurfacePool);
     SHFREE( shared->shmpool, shared );
     D_FREE( m_data );
     m_data = NULL;

     return DFB_OK;
}

static DFBResult
msosmma_system_shutdown( bool emergency )
{
    DFBMMADataShared *shared;

    DFBMMAData *mma = dfb_system_data();

    D_ASSERT( mma != NULL );

    shared = mma->shared;

    D_ASSERT( shared != NULL );

    if (shared->pool)
      dfb_surface_pool_destroy( shared->pool);

    if (shared->preallocpool)
     dfb_surface_pool_destroy(shared->preallocpool);

    SHFREE( shared->shmpool, shared );

    D_FREE( mma );
    mma = NULL;

    return DFB_OK;
}

static DFBResult
system_shutdown( bool emergency )
{
    if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
    {
        if( dfb_MMA_IsMMAEnabled() )
            return msosmma_system_shutdown(emergency);
    }

    return devmem_system_shutdown(emergency);
}

static DFBResult
devmem_system_leave( bool emergency )
{
     DevMemDataShared *shared;
     D_INFO("\033[31mDFB system leave %s(%d) \033[m\n",__FUNCTION__,__LINE__);
     D_ASSERT( m_data != NULL );

     shared = m_data->shared;
     D_ASSERT( shared != NULL );

     dfb_surface_pool_leave( shared->pool );

     UnmapMemAndReg( m_data, dfb_config->video_length, dfb_config->mmio_length, false);

     if (shared->pool_secondary)
     {
          dfb_surface_pool_leave( shared->pool_secondary );
          _UnmapMemSecondary(m_data,dfb_config->video_length_secondary,  false);
     }

     dfb_surface_pool_leave(shared->preAllocInVideoSurfacePool);
     D_FREE( m_data );
     m_data = NULL;

     return DFB_OK;
}

static DFBResult
msosmma_system_leave( bool emergency )
{
     DFBMMADataShared *shared;

     DFBMMAData *mma = dfb_system_data();

     D_ASSERT( mma != NULL );

     shared = mma->shared;
     D_ASSERT( shared != NULL );

     dfb_surface_pool_leave( shared->pool );

     dfb_surface_pool_leave(shared->preallocpool);

     D_FREE( mma );
     mma = NULL;

     return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
    if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
    {
        if( dfb_MMA_IsMMAEnabled() )
            return msosmma_system_leave(emergency);
    }

    return devmem_system_leave(emergency);
}

static DFBResult
system_suspend( void )
{
     D_ASSERT( m_data != NULL );

#if USE_MSTAR_CMA
    D_INFO("\033[31m %s(%d) \033[m\n",__FUNCTION__,__LINE__);

    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL)
    {
        DevMemDataShared *shared;

        shared = m_data->shared;
        D_ASSERT( shared != NULL );
        D_INFO("\033[31mWaring: dfb Suspend !!!  %s(%d) pool_handle_id = 0x%x, offset_in_pool = 0x%08x\033[m\n",__FUNCTION__,__LINE__, shared->pool_handle_id, shared->offset_in_pool);

        dfb_CMA_PutMem(shared->pool_handle_id, shared->offset_in_pool, dfb_config->video_length);

        if(shared->pool_secondary)
        {
            dfb_CMA_PutMem(shared->sec_pool_handle_id, shared->sec_offset_in_pool, dfb_config->video_length_secondary);
        }
        D_INFO("\033[31m %s, %d, call MApi_CMA_Pool_PutMem!!\033[m\n",__PRETTY_FUNCTION__, __LINE__);
    }
#endif
     return DFB_OK;
}

static DFBResult
system_resume( void )
{
     D_ASSERT( m_data != NULL );

#if USE_MSTAR_CMA
    D_INFO("\033[31m %s(%d) \033[m\n",__FUNCTION__,__LINE__);

    if(dfb_config->mst_mapmem_mode == E_DFB_MEMMAP_CMA_POOL)
    {
         bool ret;

         DevMemDataShared *shared;
         void *pvirtual_address = NULL;
         shared = m_data->shared;
         D_ASSERT( shared != NULL );

         pvirtual_address = dfb_CMA_GetMem(shared->pool_handle_id, shared->offset_in_pool, dfb_config->video_length);
         if(pvirtual_address = NULL)
         {
             D_PERROR("\033[35mFunction = %s, Line = %d, CMA_POOL_Get ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
         }
         else
         {
             D_INFO("\033[35mFunction = %s, Line = %d, get virtul_addr is 0x%x\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Alloc_PARAM.virt_addr);
             m_data->mem = pvirtual_address;
         }

         pvirtual_address = NULL;
         if(shared->pool_secondary)
         {
             pvirtual_address = dfb_CMA_GetMem(shared->sec_pool_handle_id, shared->sec_offset_in_pool, dfb_config->video_length_secondary);
             if(pvirtual_address == NULL)
             {
                 D_PERROR("\033[35mFunction = %s, Line = %d, CMA_POOL_Get ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
             }
             else
             {
                 D_INFO("\033[35mFunction = %s, Line = %d, get virtul_addr is 0x%x\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Alloc_PARAM.virt_addr);
                 m_data->mem_secondary = pvirtual_address;
             }
         }
    }
#endif
     return DFB_OK;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
     if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
     {
         if( dfb_MMA_IsMMAEnabled() )
            return NULL;
     }

     D_ASSERT( m_data != NULL );

     return m_data->reg + offset;
}

static void
system_unmap_mmio( volatile void  *addr,
                   int             length )
{
}

static int
system_get_accelerator( void )
{
     return dfb_config->accelerator;
}

static VideoMode *
system_get_modes( void )
{
     return NULL;
}

static VideoMode *
system_get_current_mode( void )
{
     return NULL;
}

static DFBResult
system_thread_init( void )
{
     return DFB_OK;
}

static bool
system_input_filter( CoreInputDevice *device,
                     DFBInputEvent   *event )
{
     return false;
}

static unsigned long
system_video_memory_physical( unsigned int offset )
{
     return dfb_config->video_phys + offset;
}

static void *
system_video_memory_virtual( unsigned int offset )
{
     if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
     {
         if( dfb_MMA_IsMMAEnabled() )
           return NULL;
     }

     D_ASSERT( m_data != NULL );

     return m_data->mem + offset;
}

static unsigned int
system_videoram_length( void )
{
     return dfb_config->video_length;
}

static unsigned long
system_aux_memory_physical( unsigned int offset )
{
     return 0;
}

static void *
system_aux_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_auxram_length( void )
{
     return 0;
}

static void
system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
     return;
}

static int
system_surface_data_size( void )
{
     /* Return zero because shared surface data is unneeded. */
     return 0;
}

static void
system_surface_data_init( CoreSurface *surface, void *data )
{
     /* Ignore since unneeded. */
     return;
}

static void
system_surface_data_destroy( CoreSurface *surface, void *data )
{
     /* Ignore since unneeded. */
     return;
}


static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
     return;
}
static unsigned int
system_video_memory_available(void * arg)
{

    unsigned int mem_available = 0;;

    if ( dfb_config->mst_mma_pool_enable == true ) //check the platform if support MSOS MMA
    {
        if( dfb_MMA_IsMMAEnabled() )
            return mem_available;
    }

     //get the free memory in the pool
     if(m_data&&m_data->shared)
     {

         if(m_data->shared->pool)
         {
             DevMemPoolData *data = m_data->shared->pool->data;
             if(data)
             {
                mem_available +=data->manager->avail;
             }
         }

         if(m_data->shared->pool_secondary)
         {
            DevMemPoolData *data =m_data->shared->pool_secondary->data;
            if(data)
            {
               mem_available +=data->manager->avail;
            }
         }

     }


    return mem_available;


}
