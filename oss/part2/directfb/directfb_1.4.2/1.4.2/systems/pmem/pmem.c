/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrj盲l盲 <syrjala@sci.fi> and
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

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>
#include <fusion/arena.h>
#include <fusion/property.h>

#include <core/core.h>
#include <core/surface_pool.h>

#include <misc/conf.h>

#include "pmem.h"
#include "surfacemanager.h"


#include <core/core_system.h>
#include <linux/android_pmem.h>
#include <MsCommon.h>
DFB_CORE_SYSTEM( devpmem )

#define DEV_MEM     "/dev/mem"
/**********************************************************************************************************************/

DevPMemData *g_pmem_data;    /* FIXME: Fix Core System API to pass data in all functions. */

/**********************************************************************************************************************/

bool PMEMGetSpaceInfo(int pool_idx, unsigned *free_size, unsigned *max_block_size)
{
     const char *pmem;
     int fd;
    struct pmem_mem_avail mem_avail;

     if(pool_idx==VIDEO_POOLS_INDEX1)
         pmem = DEV_PMEM0;
     else
        pmem = DEV_PMEM1;

     fd = open( pmem, O_RDWR | O_SYNC );

     if(fd >= 0)
     {
          if( ioctl(fd, PMEM_GET_MEMINFO, &mem_avail) < 0)
          {
                printf("\nwarning :PMEM_GET_MEMINFO from %s failed\n",pmem);
                close(fd);
                return false;
          }
          close(fd);
          if(free_size)
             *free_size = mem_avail.free_size;
          if(max_block_size)
             *max_block_size = mem_avail.free_max_blocksize;
          return true;
     }
     return false;
}

bool PMEMGetSpaceInfobyDevice(const char *pmem, unsigned *total_size, unsigned *free_size, unsigned *max_block_size)
{

     int fd;
    struct pmem_mem_avail mem_avail;

     fd = open( pmem, O_RDWR | O_SYNC );

     if(fd >= 0)
     {
          if( ioctl(fd, PMEM_GET_MEMINFO, &mem_avail) < 0)
          {
                printf("\nwarning :PMEM_GET_MEMINFO from %s failed\n",pmem);
                close(fd);
                return false;
          }

          close(fd);
          if(free_size)
             *free_size = mem_avail.free_size;
          if(max_block_size)
             *max_block_size = mem_avail.free_max_blocksize;
          if(total_size)
            *total_size = mem_avail.totoal_size;
          return true;
     }

     return false;
}


const char *PMemGetPoolPMemMIUInfo(int pool_idx, int *pMIU)
{
     const char *pmem;
     int fd;
      struct pmem_region region;


     if(pool_idx==VIDEO_POOLS_INDEX1)
         pmem = DEV_PMEM0;
     else
        pmem = DEV_PMEM1;

     if(!pMIU)
          return pmem;

     fd = open( pmem, O_RDWR | O_SYNC );

     if(fd < 0)
        return NULL;
    if( ioctl(fd, PMEM_ALLOCATE, 0x1000) < 0)
    {
        printf("\nwarning :allocate len:0x%x  from %s failed\n",0x1000, pmem);
        close(fd);
        return NULL;
    }
    if ( ioctl(fd, PMEM_GET_PHYS, &region) < 0||region.len<=0)//获取物理地址
    {
        printf("\nwarning:PMEM_GET_PHYS failed\n");
        close(fd);
        return NULL;
    }
    *pMIU = ((region.offset>=dfb_config->mst_miu1_cpu_offset)?1:0);
    close(fd);
    D_INFO("PMemGetPoolPMemMIUInfo[%08x] %s on %d\n", region.offset, pmem, *pMIU);

    if((pmem == DEV_PMEM0 && *pMIU>0) ||(pmem == DEV_PMEM1 && *pMIU==0))
         D_WARN("PMEM %s is on incorrect MIU\n", pmem);


    return pmem;
}


void PMemDumpPMemInfo(void)
{
     int fd;
    struct pmem_mem_avail mem_avail;

     printf("\n\n\n**************PMEM status**********************\n");
     fd = open( DEV_PMEM0, O_RDWR | O_SYNC );

     if(fd >= 0)
     {
          if( ioctl(fd, PMEM_GET_MEMINFO, &mem_avail) < 0)
          {
                printf("\nwarning :PMEM_GET_MEMINFO from %s failed\n", DEV_PMEM0);
                close(fd);
                return;
          }
          close(fd);
          printf("%s total memory is 0x%08x, free memory is %08x, max block size is %08x\n", DEV_PMEM0 ,
                               mem_avail.totoal_size, mem_avail.free_size,  mem_avail.free_max_blocksize);
     }
     fd = open( DEV_PMEM1, O_RDWR | O_SYNC );
     if(fd >= 0)
     {
          if( ioctl(fd, PMEM_GET_MEMINFO, &mem_avail) < 0)
          {
                printf("\nwarning :PMEM_GET_MEMINFO from %s failed\n", DEV_PMEM0);
                close(fd);
                return ;
          }
          close(fd);
          printf("%s total memory is 0x%08x, free memory is %08x, max block size is %08x\n", DEV_PMEM1 ,
                               mem_avail.totoal_size, mem_avail.free_size,  mem_avail.free_max_blocksize);
     }
    printf("\n\n\n");

}




DFBResult
PMemAllocMemBlock( DevPMemData    *data,
              int  pool_index, int block_idx,
              unsigned long  *mem_phys,
              unsigned int   mem_length, unsigned int *allocated_len )
{
     int pmem_fd;
     struct pmem_region region;
     const char *pmem;
     DevPMemDataShared *shared;
    // struct pmem_mem_avail meminfo;

    D_MAGIC_ASSERT(data, DevPMemData);
    D_ASSERT(dfb_core_is_master(data->core));

     shared = data->shared;
     D_MAGIC_ASSERT(shared, DevPMemDataShared);

     D_ASSERT(!shared->pmemBlocks[pool_index][block_idx].bAllocated);
     D_ASSERT(IS_POWER2(mem_length));

     pmem = PMemGetPoolPMemMIUInfo(pool_index, NULL);

     pmem_fd = open( pmem, O_RDWR | O_SYNC );


     if (pmem_fd < 0) {
        printf("\ncan  not find dev/pmem\n");
          D_PERROR( "System/pmem: Opening '%s' failed!\n", pool_index==VIDEO_POOLS_INDEX0?DEV_PMEM1:DEV_PMEM0);
          return DFB_FILENOTFOUND;
     }



    if( ioctl(pmem_fd, PMEM_ALLOCATE, mem_length) < 0)
    {
        printf("\nwarning :allocate len:0x%x failed\n",mem_length);
        close(pmem_fd);
        return DFB_NOVIDEOMEMORY;
    }


    if ( ioctl(pmem_fd, PMEM_GET_PHYS, &region) < 0)//获取物理地址
    {
        printf("\nwarning:PMEM_GET_PHYS failed\n");
        close(pmem_fd);
        return DFB_IO;
    }

    if (region.len<mem_length)//获取物理地址
    {
        printf("\nwarning:Allocate memory failed, run out of pmem on %s failed\n", pmem);
        close(pmem_fd);
        return DFB_IO;
    }
    D_ASSERT( region.offset > 0);

     *allocated_len = region.len;
    *mem_phys = region.offset;
     shared->pmemBlocks[pool_index][block_idx].bAllocated = true;
     shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys = region.offset ;
     shared->pmemBlocks[pool_index][block_idx].pmem_block_length = region.len;
     shared->pmemBlocks[pool_index][block_idx].bAllocated = true;
     data->pmemBlockData[pool_index][block_idx].pmem_fd = pmem_fd;

     int val = fcntl(pmem_fd,F_GETFD);
     val |= FD_CLOEXEC;
     fcntl(pmem_fd,F_SETFD,val);


     return DFB_OK;
}

void PMemForkCallback(FusionForkAction action, FusionForkState state)
{

   if(action==FFA_FORK && state==FFS_CHILD)
   {
      int i, j;

      for( i=0; i<VIDEO_POOL_NUM; i++)
        for(j=0; j<POOL_BLOCKS_MAX;j++)
            if(g_pmem_data->pmemBlockData[i][j].pmem_fd>0)
            {

                 close(g_pmem_data->pmemBlockData[i][j].pmem_fd);
                 g_pmem_data->pmemBlockData[i][j].pmem_fd = 0;
            }
   }
}


DFBResult PMemAssureMappedMemory(DevPMemData    *data,
              int pool_index,
              int block_idx )
{
     int fd;
     struct pmem_region region;
     DevPMemDataShared *shared = data->shared;
     unsigned long hal_offset= 0;

     D_ASSERT(pool_index>=0 && pool_index<VIDEO_POOL_NUM);
     D_ASSERT(block_idx>=0 && block_idx<POOL_BLOCKS_MAX);
     D_ASSERT(shared->pmemBlocks[pool_index][block_idx].bAllocated
        && shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys && shared->pmemBlocks[pool_index][block_idx].pmem_block_length);

     if(data->pmemBlockData[pool_index][block_idx].last_cpu_phys == shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys
        &&data->pmemBlockData[pool_index][block_idx].last_mem_len==shared->pmemBlocks[pool_index][block_idx].pmem_block_length)
     {
          D_ASSERT(data->pmemBlockData[pool_index][block_idx].pmem_vbase);
          return DFB_OK;
     }

     if(data->pmemBlockData[pool_index][block_idx].pmem_vbase && data->pmemBlockData[pool_index][block_idx].bPA2KSEG1==false)
     {
          munmap((void *)(data->pmemBlockData[pool_index][block_idx].pmem_vbase),data->pmemBlockData[pool_index][block_idx].last_mem_len);
     }

     data->pmemBlockData[pool_index][block_idx].pmem_vbase = NULL;
     data->pmemBlockData[pool_index][block_idx].last_cpu_phys = 0;
     data->pmemBlockData[pool_index][block_idx].last_mem_len = 0;


     hal_offset=_mstarGFXAddr(shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys);
     if(NULL!=MsOS_MPool_PA2KSEG1(hal_offset))
     {
        data->pmemBlockData[pool_index][block_idx].pmem_vbase =  MsOS_MPool_PA2KSEG1(hal_offset);
        data->pmemBlockData[pool_index][block_idx].bPA2KSEG1 = true;

        data->pmemBlockData[pool_index][block_idx].last_cpu_phys = shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys;
        data->pmemBlockData[pool_index][block_idx].last_mem_len = shared->pmemBlocks[pool_index][block_idx].pmem_block_length;
        return DFB_OK;
     }


     data->pmemBlockData[pool_index][block_idx].bPA2KSEG1 = false;

     unsigned long mem_phys =shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys;
     unsigned int mem_length = shared->pmemBlocks[pool_index][block_idx].pmem_block_length;


     fd = open( DEV_MEM, O_RDWR | O_SYNC );
     if (fd < 0) {
          printf("\ncan  not find dev/mem\n");
          D_PERROR( "System/mem: Opening '%s' failed!\n", DEV_MEM );
          return DFB_IO;
     }


     data->pmemBlockData[pool_index][block_idx].pmem_vbase = mmap( NULL, mem_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mem_phys );



     if (data->pmemBlockData[pool_index][block_idx].pmem_vbase == MAP_FAILED) {
          printf("\n!!!!memmap failed====================================\n");
          printf("\nmaybe not enough vitural adress space\n");
          D_PERROR( "System/pmem: Mapping %d bytes at 0x%08lx for pool %dfailed!\n", mem_length, mem_phys,  pool_index);
          data->pmemBlockData[pool_index][block_idx].pmem_vbase = NULL;
          data->pmemBlockData[pool_index][block_idx].last_mem_len = 0;
          close( fd );
          return DFB_IO;
     }
     close( fd);

     D_ASSERT(data->pmemBlockData[pool_index][block_idx].pmem_vbase);
     data->pmemBlockData[pool_index][block_idx].last_cpu_phys = mem_phys;
     data->pmemBlockData[pool_index][block_idx].last_mem_len = mem_length;



     return DFB_OK;

}

void PMemUnMapMemoryBlock(DevPMemData   *data, int pool_index,int block_idx)
{
    DevPMemDataShared *shared = data->shared;

    if(!data->pmemBlockData[pool_index][block_idx].bPA2KSEG1)
   {
          if(data->pmemBlockData[pool_index][block_idx].pmem_vbase)
          {
               munmap((void *)(data->pmemBlockData[pool_index][block_idx].pmem_vbase),data->pmemBlockData[pool_index][block_idx].last_mem_len);
               data->pmemBlockData[pool_index][block_idx].pmem_vbase = NULL;
               data->pmemBlockData[pool_index][block_idx].last_cpu_phys = 0;
               data->pmemBlockData[pool_index][block_idx].last_mem_len = 0;
          }
   }
}

void PMemReleaseMemoryBlock(DevPMemData   *data, int pool_index,int block_idx)
{
     DevPMemDataShared *shared = data->shared;

     D_ASSERT(dfb_core_is_master(data->core)) ;

     if(shared->pmemBlocks[pool_index][block_idx].bAllocated)
    {
          int pmem_fd = data->pmemBlockData[pool_index][block_idx].pmem_fd;
          int ret = -1;

          D_ASSERT(pmem_fd > 0);
          D_INFO("\nrelesae the pool index :%d  block_index:%d blocksize :%x\n",pool_index,block_idx,shared->pmemBlocks[pool_index][block_idx].pmem_block_length);

          ret = close(pmem_fd);
          if(0!=ret)
          {
              printf("\nclose the pmem block failed ,the return value %d\n",ret);
          }
          data->pmemBlockData[pool_index][block_idx].pmem_fd = 0;
          shared->pmemBlocks[pool_index][block_idx].bAllocated= false;
          shared->pmemBlocks[pool_index][block_idx].pmem_cpu_phys = 0;
          shared->pmemBlocks[pool_index][block_idx].pmem_block_length = 0;
    }

}

static DFBResult _MapAllMemBlock(DevPMemData   *data)
{
    DevPMemDataShared *shared = data->shared;
    DFBResult ret = DFB_OK;
    int i, j;

    for(i=0; i<VIDEO_POOL_NUM; i++)
    {
        for(j =0; j<POOL_BLOCKS_MAX; j++)
        {
           if(shared->pmemBlocks[i][j].bAllocated)
           {
               ret = PMemAssureMappedMemory(data, i, j);
               if(ret != DFB_OK)
                   break;
           }
        }
        if(ret != DFB_OK)
             break;
    }

    if(ret == DFB_OK)
        return DFB_OK;

    for(i=0; i<VIDEO_POOL_NUM; i++)
    {
        for(j =0; j<POOL_BLOCKS_MAX; j++)
        {
           PMemReleaseMemoryBlock(data, i, j);
        }
    }

    return DFB_IO;


}


static DFBResult MapReg(DevPMemData    *data,unsigned long  reg_phys,unsigned int   reg_length)
{
     int fd;

     if(!reg_phys  || !reg_length)
     {
          data->reg = NULL;
          return DFB_OK;
     }
     fd = open( DEV_MEM, O_RDWR | O_SYNC );
     if (fd < 0) {
        printf("\ncan  not find dev/mem\n");
          D_PERROR( "System/mem: Opening '%s' failed!\n", DEV_MEM );
          return DFB_INIT;
     }
     else
     {
        D_INFO("\nfind the dev/mem\n");
     }

     data->reg = mmap( NULL, reg_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, reg_phys );
     if (data->reg == MAP_FAILED) {
               D_PERROR( "System/mem: Mapping %d bytes at 0x%08lx via '%s' failed!\n", reg_length, reg_phys, DEV_MEM );
               close( fd );
               data->reg  = NULL;
               return DFB_INIT;
     }

     close( fd );

     return DFB_OK;

}
static void
UnmapReg( DevPMemData   *data,
                unsigned int  reg_length )
{


     if (reg_length && data->reg)
          munmap( (void*) data->reg, reg_length );
}



/**********************************************************************************************************************/

static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_PMEM;
     info->caps = CSCAPS_ACCELERATION;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "PMem" );
}

static DFBResult
system_initialize( CoreDFB *core, void **ret_data )
{
     DFBResult            ret;
     DevPMemData          *data = NULL; //Fix converity END
     DevPMemDataShared    *shared;
     FusionSHMPoolShared *pool;
     int i=0;

     D_ASSERT( g_pmem_data == NULL );

     printf("\nthe pmem initialize ...............................\n");
     //MsOS_MPool_Init();
     MsOS_MPool_Get(0,0,0, true);

     if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
          D_ERROR( "System/pmem: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
          return DFB_INVARG;
     }

     data = D_CALLOC( 1, sizeof(DevPMemData) );
     if (!data)
          return D_OOM();
     D_MAGIC_SET(data, DevPMemData);
     data->core = core;

     pool = dfb_core_shmpool( core );

     shared = SHCALLOC( pool, 1, sizeof(DevPMemDataShared) );
     if (!shared) {
          D_FREE( data );
          return D_OOSHM();
     }

     D_MAGIC_SET(shared, DevPMemDataShared);

    // shared->reactor = fusion_reactor_new( sizeof(DFBPMemEvent), "pmem_reactor", dfb_core_world(core) );

     shared->shmpool = pool;
     data->shared = shared;

     ret = MapReg(data,dfb_config->mmio_phys,  dfb_config->mmio_length );

     if (ret) {
          SHFREE( pool, shared );
          D_FREE( data );
          return ret;
     }

     shared->pool_init_size[VIDEO_POOLS_INDEX0] = 0x1000000;//mainly used by  dfb misc
     shared->pool_step_size[VIDEO_POOLS_INDEX0]=0x800000;
     shared->pool_init_size[VIDEO_POOLS_INDEX1] = 0x400000;//mainly used by zuffer
     shared->pool_step_size[VIDEO_POOLS_INDEX1]=0x200000;
     shared->pool_init_size[VIDEO_POOLS_DECLARABLE] = 0x1000000;//by reusable textures
     shared->pool_step_size[VIDEO_POOLS_DECLARABLE]=0x1000000;


     *ret_data = g_pmem_data = data;

    for(i=0; i<VIDEO_POOL_NUM; i++)
    {
        int miu;
        if(!PMemGetPoolPMemMIUInfo(i, &shared->pool_miu[i]))
             continue;
      printf("start init pool %d\n", i);
      data->privatedUsage = i;
        dfb_surface_pool_initialize( core, &DevPMemSurfacePoolFuncs, &shared->pool[i] );

     }
    printf("system_initialize2\n");


     if(DFB_OK != _MapAllMemBlock(data))
     {
         goto ERROR;
     }





     fusion_arena_add_shared_field( dfb_core_arena( core ), "devpmem", shared );
     fusion_world_set_fork_callback(dfb_core_world(core), PMemForkCallback);
     return DFB_OK;
ERROR:

     for(--i; i>=0; i--)
    {
           if(!shared->pool[i])
              continue;
           PMemReleaseMemoryBlock(data, i, 0);
           dfb_surface_pool_destroy( shared->pool[i] );
    }
    SHFREE( pool, shared );
    UnmapReg(data,dfb_config->mmio_length );
    D_FREE( data );

    return DFB_INIT;
}

static DFBResult
system_join( CoreDFB *core, void **ret_data )
{
     DFBResult         ret;
     void             *tmp;
     DevPMemData       *data;
     DevPMemDataShared *shared;
     int i=0, j;

     D_ASSERT( g_pmem_data == NULL );
     //MsOS_MPool_Init();
     MsOS_MPool_Get(0,0,0, true);
     if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
          D_ERROR( "System/pmem: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
          return DFB_INVARG;
     }

     data = D_CALLOC( 1, sizeof(DevPMemData) );
     if (!data)
          return D_OOM();

     ret = fusion_arena_get_shared_field( dfb_core_arena( core ), "devpmem", &tmp );
     if (ret) {
          D_FREE( data );
          return ret;
     }

     D_MAGIC_SET(data, DevPMemData);

     data->shared = shared = tmp;
     data->core = core;


     ret = MapReg(data,dfb_config->mmio_phys,  dfb_config->mmio_length );

     if (ret) {
          D_FREE( data );
          return ret;
     }


     *ret_data = g_pmem_data = data;
     for(i=0; i<VIDEO_POOL_NUM; i++)
     {
          if(shared->pool[i])
                  dfb_surface_pool_join(core, shared->pool[i], &DevPMemSurfacePoolFuncs );
     }
     return DFB_OK;
     
/*     
ERROR:

     for(--i; i>=0; i--)
    {
        if(!shared->pool[i])
              continue;

        dfb_surface_pool_leave(shared->pool[i]);
    }
    UnmapReg(data,dfb_config->mmio_length );
    D_FREE( data );

    return DFB_INIT;

    */
    
}

static DFBResult
system_shutdown( bool emergency )
{
     DevPMemDataShared *shared;
     int i;
     int j;
     D_ASSERT( g_pmem_data != NULL );
     D_MAGIC_ASSERT(g_pmem_data, DevMemData);

     shared = g_pmem_data->shared;

     D_MAGIC_ASSERT(shared, DevMemDataShared);

     for(i=0; i<VIDEO_POOL_NUM; i++)
    {
          if(!shared->pool[i])
              continue;
          for( j=0; j<POOL_BLOCKS_MAX; j++)
         {
                 PMemUnMapMemoryBlock(g_pmem_data, i, j);
                 PMemReleaseMemoryBlock(g_pmem_data, i, j);
          }
          dfb_surface_pool_destroy(shared->pool[i] );
    }
    D_MAGIC_CLEAR(shared);
    SHFREE( shared->shmpool, (void*)shared );
    UnmapReg(g_pmem_data,dfb_config->mmio_length );
    D_MAGIC_CLEAR(g_pmem_data);
    D_FREE( g_pmem_data );
    g_pmem_data = NULL;

     return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
     DevPMemDataShared *shared;
     int i;
     int j;
     D_ASSERT( g_pmem_data != NULL );

     shared = g_pmem_data->shared;
     D_ASSERT( shared != NULL );

     for(i=0; i<VIDEO_POOL_NUM; i++)
    {
        DFBResult ret;

        if(!shared->pool[i])
              continue;
        ret = fusion_skirmish_prevail( &shared->pool[i]->lock );

          for( j=0; j<POOL_BLOCKS_MAX; j++)
         {
                 PMemUnMapMemoryBlock(g_pmem_data, i, j);
          }
          if(ret == DFB_OK)
          {
                 fusion_skirmish_dismiss( &shared->pool[i]->lock );
          }
          dfb_surface_pool_leave(shared->pool[i] );
    }
    UnmapReg(g_pmem_data,dfb_config->mmio_length );
    D_MAGIC_CLEAR( g_pmem_data);
    D_FREE( g_pmem_data );
    g_pmem_data = NULL;

     return DFB_OK;
}

static DFBResult
system_suspend( void )
{
     D_ASSERT( g_pmem_data != NULL );

     return DFB_OK;
}

static DFBResult
system_resume( void )
{
     D_ASSERT( g_pmem_data != NULL );

     return DFB_OK;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
     D_ASSERT( g_pmem_data != NULL );

     return g_pmem_data->reg + offset;
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
     return offset;
}

static void *
system_video_memory_virtual( unsigned int offset )
{
     DevPMemDataShared *shared;
     int i;
     int j;
     D_ASSERT( g_pmem_data != NULL );

     shared = g_pmem_data->shared;

     for(i=0; i<VIDEO_POOL_NUM; i++)
    {
          if(!shared->pool[i])
              continue;
          for( j=0; j<POOL_BLOCKS_MAX; j++)
         {
                 if(shared->pmemBlocks[i][j].bAllocated
                      && shared->pmemBlocks[i][j].pmem_cpu_phys<=offset &&
                      shared->pmemBlocks[i][j].pmem_cpu_phys+shared->pmemBlocks[i][j].pmem_block_length>offset)
                    {
                         if(PMemAssureMappedMemory(g_pmem_data,i, j)!=DFB_OK)
                             return NULL;
                         return (void*)((char*)g_pmem_data->pmemBlockData[i][j].pmem_vbase+offset-shared->pmemBlocks[i][j].pmem_cpu_phys);
                    }
            }
        }
        return NULL;
}

static unsigned int
system_videoram_length( void )
{
    unsigned int videoram_length = 0;
    int device_index=0;
    int device_num =2;
    int pool_index;
    const char * device_name[2]={DEV_PMEM0,DEV_PMEM1};


//get the total videoram size
    for(device_index=0;device_index<device_num;device_index++)
    {
         unsigned total_size = 0;

         if(PMEMGetSpaceInfobyDevice(device_name[device_index], &total_size, NULL,NULL))
         {
            videoram_length += total_size;

         }
    }


     return videoram_length;
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

static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
     return;
}
static unsigned int
system_video_memory_available(void *arg)
{
    unsigned int mem_available = 0;
    int device_index=0;
    int device_num =2;
    int pool_index;
    const char * device_name[2]={DEV_PMEM0,DEV_PMEM1};


//get the free memory size in the pmem
    for(device_index=0;device_index<device_num;device_index++)
    {
         unsigned free_size = 0;

         if(PMEMGetSpaceInfobyDevice(device_name[device_index], NULL, &free_size,NULL))
         {
            mem_available += free_size;

         }
    }

     //get the free memory in the pool
     if(g_pmem_data&&g_pmem_data->shared)
     {
           for(pool_index=0;pool_index<VIDEO_POOL_NUM;pool_index++)
          {
             if(g_pmem_data->shared->pool[pool_index])
             {
                 DevPMemPoolSharedData *data = g_pmem_data->shared->pool[pool_index]->data;
                 if(data)
                 {
                    mem_available +=data->manager->avail;
                 }
             }
          }
     }
    return mem_available;
}

