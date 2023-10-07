/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrj?l? <syrjala@sci.fi> and
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

#include <string.h>
#include <unistd.h>

#include <direct/debug.h>

#include <fusion/conf.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/pool.h>

#include <core/core.h>
#include <core/surface_pool.h>

#include <misc/conf.h>
#include "x_typedef.h"
#ifdef DFB_IOMMU_PRECHECK
#include <stdio.h>
#include <fcntl.h>
#endif
#include "directfb.h"

/**********************************************************************************************************************/

typedef struct {
     FusionSHMPoolShared *shmpool;
} SharedPoolData;

typedef struct {
     CoreDFB     *core;
     FusionWorld *world;
} SharedPoolLocalData;

D_DEBUG_DOMAIN( Share_Surfaces, "Share/Surfaces", "Share Surface Pool" );

/**********************************************************************************************************************/

static void shared_check_mmu_pin_mem(SharedAllocationData *alloc)
{
    int ret=0;
    E_DFB_SSP e_sh = E_DSSP_SH;

    D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

    if(!alloc || (0x0 == alloc->pin_range[E_DSSP_ALLOC].start))
    {
        D_ERROR("%s[pid = %d] Fail\n",__FUNCTION__,getpid());
        return ;
    }

    if(alloc->pin_range[E_DSSP_SH].start)
    {
        e_sh = E_DSSP_SH2;
    }
    else if(alloc->pin_range[E_DSSP_SH2].start)
    {
        e_sh = E_DSSP_SH3;
    }
    else if(alloc->pin_range[E_DSSP_SH3].start)
    {
        D_ERROR("%s [pid = %d][%d,%d,%d] Fail\n",__FUNCTION__,getpid(),alloc->pid[E_DSSP_SH],alloc->pid[E_DSSP_SH2],alloc->pid[E_DSSP_SH3]);
        return ;
    }

    alloc->pid[e_sh] = getpid();
    alloc->pin_range[e_sh].start = alloc->pin_range[E_DSSP_ALLOC].start;
    alloc->pin_range[e_sh].size  = alloc->pin_range[E_DSSP_ALLOC].size;

    ret=mlock((void*)(UPTR)alloc->pin_range[e_sh].start,alloc->pin_range[e_sh].size);
    if(ret != 0)
    {
        D_ERROR( "sharedAllocateBuffer:mlock fail[0x%p, 0x%p], errno=%d.\n", alloc->addr,alloc->size,errno );
    }
    system_mmu_pin_mem(&alloc->pin_range[e_sh]);

    return ;
}

static void shared_check_mmu_unpin_mem(SharedAllocationData *alloc)
{
    E_DFB_SSP e_sh = E_DSSP_SH;

    D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

    if(!alloc)
    {
        D_ERROR("%s[pid = %d] Fail\n",__FUNCTION__,getpid());
        return ;
    }

    if(alloc->pid[E_DSSP_SH] == getpid())
    {
        e_sh = E_DSSP_SH;
        if(0x0 == alloc->pin_range[E_DSSP_SH].start)
        {
            D_ERROR("%s E_DSSP_SH[pid = %d][%d,%d,%d] Fail\n",__FUNCTION__,getpid(),alloc->pid[E_DSSP_SH],alloc->pid[E_DSSP_SH2],alloc->pid[E_DSSP_SH3]);
            return ;
        }
    }
    else if (alloc->pid[E_DSSP_SH2] == getpid())
    {
        e_sh = E_DSSP_SH2;
        D_ERROR("%s E_DSSP_SH2 [pid = %d] \n",__FUNCTION__,getpid());
        if(0x0 == alloc->pin_range[E_DSSP_SH2].start)
        {
            D_ERROR("%s E_DSSP_SH2[pid = %d][%d,%d,%d] Fail\n",__FUNCTION__,getpid(),alloc->pid[E_DSSP_SH],alloc->pid[E_DSSP_SH2],alloc->pid[E_DSSP_SH3]);
            return ;
        }
    }
    else if (alloc->pid[E_DSSP_SH3] == getpid())
    {
        e_sh = E_DSSP_SH3;
        D_ERROR("%s E_DSSP_SH2 [pid = %d] \n",__FUNCTION__,getpid());
        if(0x0 == alloc->pin_range[E_DSSP_SH3].start)
        {
            D_ERROR("%s E_DSSP_SH2[pid = %d][%d,%d,%d] Fail\n",__FUNCTION__,getpid(),alloc->pid[E_DSSP_SH],alloc->pid[E_DSSP_SH2],alloc->pid[E_DSSP_SH3]);
            return ;
        }
    }
    else
    {
        D_ERROR("%s[pid = %d][%d,%d,%d] Fail\n",__FUNCTION__,getpid(),alloc->pid[E_DSSP_SH],alloc->pid[E_DSSP_SH2],alloc->pid[E_DSSP_SH3]);
        return ;
    }

    mt53EngineSync(NULL,NULL);

    system_mmu_unpin_mem(&alloc->pin_range[e_sh]);
    alloc->pin_range[e_sh].start    = 0x0;
    alloc->pin_range[e_sh].size     = 0x0;
    alloc->pid[e_sh]                = 0x0;

    return ;
}


static int
sharedPoolDataSize( void )
{
     return sizeof(SharedPoolData);
}

static int
sharedPoolLocalDataSize( void )
{
     return sizeof(SharedPoolLocalData);
}

static int
sharedAllocationDataSize( void )
{
     return sizeof(SharedAllocationData);
}

static DFBResult
sharedInitPool( CoreDFB                    *core,
                CoreSurfacePool            *pool,
                void                       *pool_data,
                void                       *pool_local,
                void                       *system_data,
                CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     SharedPoolData      *data  = pool_data;
     SharedPoolLocalData *local = pool_local;
     int i4_acces;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( ret_desc != NULL );

     D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

     local->core  = core;
     local->world = dfb_core_world( core );

     ret = fusion_shm_pool_create( local->world, "Surface Memory Pool", dfb_config->surface_shmpool_size,
                                   fusion_config->debugshm, &data->shmpool );
     if (ret)
          return ret;

     if(DFB_SYSTEMONLY)
     {
        ret_desc->caps   = CSPCAPS_ALL;
        ret_desc->scaps  = DSCAPS_ALL&(~(DSCAPS_VIDEO));

        if (dfb_config->osd_iommu)
            ret_desc->types = CSTF_ALL;
        else
            ret_desc->types = CSTF_ALL&(~(SURFACE_TYPE_VIDEO));

        ret_desc->priority          = CSPP_ULTIMATE;
        for(i4_acces = CSAID_NONE;i4_acces<_CSAID_NUM;i4_acces++)
        {
            ret_desc->access[i4_acces] = CSAF_ALL;
        }
     }
     else
     {
        ret_desc->caps              = CSPCAPS_NONE;
        ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
        ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_INTERNAL;
        ret_desc->priority          = CSPP_DEFAULT;
     }

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "Shared Memory" );

     return DFB_OK;
}

static DFBResult
sharedDestroyPool( CoreSurfacePool *pool,
                   void            *pool_data,
                   void            *pool_local )
{
     SharedPoolData      *data  = pool_data;
     SharedPoolLocalData *local = pool_local;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );

     D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

     mt53EngineSync(NULL,NULL);

     fusion_shm_pool_destroy( local->world, data->shmpool );

     return DFB_OK;
}

#ifdef DFB_IOMMU_PRECHECK
static int fd=0;
unsigned int GetPhysicalValue(unsigned int phy_addr)
{

    unsigned int rdbuf=0xff;

    int i;

    if(fd==0)
    {
        fd=open("/dev/mem", O_RDONLY);
    }
    if(fd<0)
    {
        direct_log_printf(NULL, "%s error[fd:0x%x]\n", __FUNCTION__, fd);
    }
    lseek(fd, phy_addr, 0);
    read(fd, &rdbuf, 4);
    lseek(fd, 0, 0);
    D_DEBUG_AT( Share_Surfaces, "%s phy_addr=0x%x, rdbuf=0x%x\n", __FUNCTION__, phy_addr, rdbuf );

    return rdbuf;
}
extern unsigned int system_mmu_get_ttb(void);
#define BIT(DATA, M, N) ((DATA<<(((sizeof(UPTR)*8)-1)-M))>>(N+((sizeof(UPTR)*8)-1)-M))
#define BIT3(LEVEL_PA) (((LEVEL_PA>>3)&0x1)==0?0:1)

unsigned int CalcPhysicalAddress(unsigned int VA, unsigned int TTB)
{
	unsigned int LSB=0;
	unsigned int LEVEL1_PA=0;
	unsigned int LEVEL2_PA=0;
	unsigned int LEVEL3_PA=0;
	unsigned int LEVEL1=0;
	unsigned int LEVEL2=0;
	UPTR LEVEL3=0;
	unsigned int PA=0;
	TTB=system_mmu_get_ttb();
        
	D_DEBUG_AT( Share_Surfaces, "%s,%d: 32bit4k mode\n", __func__, __LINE__ );
	D_DEBUG_AT( Share_Surfaces, "[0]VA=0x%p, TTB=0x%p......\n", VA, TTB );
	LEVEL1_PA=(BIT(TTB, 31, 14)<<14)|(BIT(VA, 31, 20)<<2);
	LSB=GetPhysicalValue(LEVEL1_PA);
	LEVEL1 = LSB;
	D_DEBUG_AT( Share_Surfaces, "[1]0x%p,0x%p,LEVEL1_PA=0x%p,LSB=0x%p,LEVEL1=0x%p...\n", \
			    BIT(TTB, 31, 14), BIT(VA, 31, 20), LEVEL1_PA, LSB,LEVEL1 );
		
	LEVEL2_PA=(BIT(LEVEL1, 31, 10)<<10)|(BIT(VA, 19, 12)<<2);
	LSB=GetPhysicalValue(LEVEL2_PA);
	LEVEL2 = LSB;
	D_DEBUG_AT( Share_Surfaces, "[2]0x%p,0x%p,LEVEL2_PA=0x%p,LSB=0x%p,LEVEL2=0x%p...\n", \
			    BIT(LEVEL1, 31, 10), BIT(VA, 19, 12), LEVEL2_PA, LSB, LEVEL2 );

			
	PA=(BIT(LEVEL2, 31, 12)<<12)|(BIT(VA, 11, 0));
	LSB=GetPhysicalValue(PA);
		
	D_DEBUG_AT( Share_Surfaces, "[*]PA: 0x%p, real phy memory LSB=0x%p...\n", PA,  LSB );	

    if(((LEVEL1&0x3)!=0x1)||((LEVEL2&0x2)==0))
    {
        D_ERROR( "CalcPhysicalAddress error[VA:0x%p, TTB:0x%p, PA:0x%p, LEVEL1_PA:0x%p, LEVEL1:0x%p, LEVEL2_PA:0x%p, LEVEL2:0x%p]\n", \
            VA, TTB, PA, LEVEL1_PA, LEVEL1, LEVEL2_PA, LEVEL2 );
    }
        
	return PA;
}
#endif
static DFBResult
sharedAllocateBuffer( CoreSurfacePool       *pool,
                      void                  *pool_data,
                      void                  *pool_local,
                      CoreSurfaceBuffer     *buffer,
                      CoreSurfaceAllocation *allocation,
                      void                  *alloc_data )
{
    CoreSurface          *surface;
    SharedPoolData       *data  = pool_data;
    SharedAllocationData *alloc = alloc_data;
    int byte_align  =64;
    int ret=0;

    D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    /* 4K page align for dma_buf using */
    if (dfb_config->osd_iommu)
        byte_align = 4096;

    surface = buffer->surface;
    D_MAGIC_ASSERT( surface, CoreSurface );
    memset(alloc,0x0,sizeof(SharedAllocationData));
    dfb_surface_calc_buffer_size( surface,byte_align,0,&alloc->pitch, &alloc->size );
    alloc->size += byte_align;
    alloc->addr = SHMALLOC( data->shmpool, alloc->size);
    if ( !alloc->addr )
    return D_OOSHM();

    /* Calculate the aligned address. */

    unsigned long addr           = (unsigned long) alloc->addr;
    unsigned long aligned_offset = (byte_align - (addr % byte_align ));

    alloc->aligned_addr = (void*) (addr + aligned_offset);
    if(aligned_offset != byte_align)
    {
        alloc->aligned_addr = (void*) (addr + aligned_offset);
    }
    else
    {
      alloc->aligned_addr = (void*) (addr);
    }
    alloc->pin_range[E_DSSP_ALLOC].start = (unsigned long)alloc->addr;
    alloc->pin_range[E_DSSP_ALLOC].size  = alloc->size;
    alloc->pid[E_DSSP_ALLOC] = getpid();

    ret = mlock(alloc->addr,alloc->size);
    if(ret != 0)
    {
        D_PERROR( "%s:mlock fail[0x%p, 0x%p]\n", __func__, alloc->addr, alloc->size );
        return DFB_FAILURE;
    }
    ret = system_mmu_pin_mem(&alloc->pin_range[E_DSSP_ALLOC]);
    if (ret != 0)
    {
        D_ERROR( "system_mmu_pin_mem failed!\n");
        return DFB_FAILURE;
    }

    // save iova address, TODO: IMGRZ need to achieve
    alloc->iova[E_IDEV_GFX] = alloc->pin_range[E_DSSP_ALLOC].iova;

    D_DEBUG_AT(Share_Surfaces, "[%s(),%d]addr = 0x%x, align_addr = 0x%x, size = %d, align_offset = %d, iova = 0x%x\n",
            __func__, __LINE__, alloc->addr, alloc->aligned_addr, alloc->size, aligned_offset, alloc->iova[E_IDEV_GFX]);

    if (dfb_config->osd_iommu)
        alloc->iova[E_IDEV_OSD] = system_mmu_transform_gfx2osd_iova(alloc->iova[E_IDEV_GFX], alloc->size);

    allocation->flags = CSALF_VOLATILE;
    allocation->size  = alloc->size;

    if(surface->type  & CSTF_WINDOW )
    {
#ifdef GFX_IOMMU_IOVA_SUPPORT
        memset((void *)alloc->addr,0, alloc->pitch*surface->config.size.h);//fix me, use hw
        mt53FlushInvalidCache( (unsigned int)alloc->addr, alloc->size );
#else
        mt53_fill_rect((unsigned int)alloc->addr, alloc->pitch,surface->config.size.h, 0);
#endif

    }
    else
    {
       memset(alloc->addr,0x0,alloc->size);
       mt53FlushInvalidCache( (unsigned int)alloc->addr, alloc->size );
    }

    if (dfb_config_get_dump_stack(DDDS_SURFACE_ALLOC))
    {
        D_INFO("%s,%d: [pid = %d][caps: 0x%x][type: 0x%x][start: %p][size: 0x%x]\n",
                            __FUNCTION__,
                            __LINE__,
                            getpid(),
                            surface->config.caps,
                            surface->type,
                            alloc->addr,
                            alloc->size);
        dfb_print_trace();
    }

    D_DEBUG_AT( Share_Surfaces, "%s,%d: [pid = %d][caps: 0x%x][type: 0x%x][start: %p][size: 0x%x]\n",
                                __FUNCTION__,
                                __LINE__,
                                getpid(),
                                surface->config.caps,
                                surface->type,
                                alloc->addr,
                                alloc->size);

#ifdef DFB_IOMMU_PRECHECK
   CalcPhysicalAddress((unsigned int)alloc->addr, 0);
#endif
    return DFB_OK;
}

static DFBResult
sharedDeallocateBuffer( CoreSurfacePool       *pool,
                        void                  *pool_data,
                        void                  *pool_local,
                        CoreSurfaceBuffer     *buffer,
                        CoreSurfaceAllocation *allocation,
                        void                  *alloc_data )
{
     SharedPoolData       *data  = pool_data;
     SharedAllocationData *alloc = alloc_data;

     D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     /* free allocated for other device iova */
     if (dfb_config->osd_iommu && alloc->iova[E_IDEV_OSD] > 0)
         system_mmu_free_osd_iova(alloc->iova[E_IDEV_OSD], alloc->size);

     system_mmu_unpin_mem(&alloc->pin_range[E_DSSP_ALLOC]);
     munlock(alloc->addr,alloc->size);

     SHFREE( data->shmpool, alloc->addr );

     return DFB_OK;
}

static DFBResult
sharedLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     SharedAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( Share_Surfaces, "%s,%d, accessor:0x%x\n", __func__, __LINE__, lock->accessor );

     if(alloc->pid[E_DSSP_ALLOC] != getpid())
     {
         int ret =0;
         ret = mlock(lock->addr,lock->size);
         if(ret != 0)
         {
           direct_log_printf(NULL, "sharedLock:mlock fail[0x%p, 0x%p], errno=%d.\n", lock->addr,lock->size,errno);
         }
         shared_check_mmu_pin_mem(alloc);
     }

     /* Lock for dma_buf export */
     if (lock->accessor == CSAID_DMA || lock->accessor == CSAID_GPU)
     {
         if (alloc->fd <= 0)
             alloc->fd = system_mmu_get_dmafd(&alloc->pin_range[E_DSSP_ALLOC]);


         lock->dma_fd = alloc->fd;
         lock->iova   = alloc->iova[E_IDEV_GFX];

     }
     else if (lock->accessor == CSAID_OSD)
     {
          lock->iova = alloc->iova[E_IDEV_OSD];
     }
     else
     {
          /* default to use GFX IOVA */
          lock->iova = alloc->iova[E_IDEV_GFX];
     }

     if (alloc->aligned_addr)
     {
          lock->addr = alloc->aligned_addr;
          lock->phys = (unsigned long)alloc->aligned_addr;
     }
     else
     {
          lock->addr = alloc->addr;
          lock->phys = (unsigned long)alloc->addr;
     }
     lock->pitch = alloc->pitch;
     lock->size  = alloc->size;

     D_DEBUG_AT( Share_Surfaces, "%s,%d: [pid = %d] [addr = 0x%x] [phys = 0x%x] [pitch = %d][fd = %d]\n",
                                 __func__,
                                 __LINE__,
                                 getpid(),
                                 lock->addr,
                                 lock->phys,
                                 lock->pitch,
                                 lock->dma_fd);

     return DFB_OK;
}

static DFBResult
sharedUnlock( CoreSurfacePool       *pool,
              void                  *pool_data,
              void                  *pool_local,
              CoreSurfaceAllocation *allocation,
              void                  *alloc_data,
              CoreSurfaceBufferLock *lock )
{
    SharedAllocationData *alloc = alloc_data;
    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_AT( Share_Surfaces, "%s,%d\n", __func__, __LINE__ );
    
    if(alloc->pid[E_DSSP_ALLOC] != getpid())
    {
        shared_check_mmu_unpin_mem(alloc);
    }

     return DFB_OK;
}

const SurfacePoolFuncs sharedSurfacePoolFuncs = {
     .PoolDataSize       = sharedPoolDataSize,
     .PoolLocalDataSize  = sharedPoolLocalDataSize,
     .AllocationDataSize = sharedAllocationDataSize,
     .InitPool           = sharedInitPool,
     .DestroyPool        = sharedDestroyPool,

     .AllocateBuffer     = sharedAllocateBuffer,
     .DeallocateBuffer   = sharedDeallocateBuffer,

     .Lock               = sharedLock,
     .Unlock             = sharedUnlock,
};


