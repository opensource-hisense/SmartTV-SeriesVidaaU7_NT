#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/clock.h>
#include <direct/debug.h>

#include <fusion/build.h>
#include <fusion/fusion.h>
#include <fusion/object.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/shm.h>
#include <fusion/shm/shm_internal.h>

#include <core/core.h>
#include <core/layer_control.h>
#include <core/layer_context.h>
#include <core/layers.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/windows.h>
#include <core/windowstack.h>
#include <core/windows_internal.h>
#include <core/wm.h>
#include "pmem_utility.h"
#include "pmem.h"


extern DevPMemData *g_pmem_data;    /* FIXME: Fix Core System API to pass data in all functions. */


static int  pmem_pool_idx(DevPMemData  *DevPMem, CoreSurfacePool*pool)
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


static inline void
buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, PMEM_Dump_Info* info, bool print )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

    D_ASSERT(surface);
    D_ASSERT(buffer);

     fusion_vector_foreach (allocation, i, buffer->allocs) {
          int size = allocation->size;

          if(!allocation->pool)
              continue;
          if(pmem_pool_idx(g_pmem_data, allocation->pool)<0)
              continue;
           if(info->offsetstart && !(info->offsetstart<=allocation->offset &&info->offsetend>allocation->offset))
               continue;
           if(print)
              printf("           buffer[0x%08x-%08x] fix_pos[%c  %dx%d]\n", allocation->offset, size,
              (surface->config.caps & DSCAPS_STATIC_POS)?'y':'n', surface->config.size.w, surface->config.size.h);

           mem += size;
     }

     info->total_used_mem += mem;
}

static void
dump_pmem_buffers( CoreSurface *surface, PMEM_Dump_Info* info, bool print)
{
     int i;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];
          if(buffer)
                 buffer_size( surface, buffer, info, print );
     }
}

static bool
pmem_surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult ret;
     int          i;
     int          refs;
     CoreSurface *surface = (CoreSurface*) object;
     PMEM_Dump_Info *info     = ctx;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }
     if(surface->state & CSSF_DESTROYED)
     {
        printf( "surface is under destorying\n");
          return false;
     }
#if 0

#if FUSION_BUILD_MULTI
     printf( "\n0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "\nN/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );


     /* FIXME: assumes all buffers have this flag (or none) */
//     if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
//          printf( "preallocated " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

     if (surface->config.caps & DSCAPS_VIDEOHIGH)
          printf( "video preferred   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied     " );

     if(surface->config.caps & DSCAPS_STATIC_ALLOC)
         printf("static_alloc");

     printf( "\n" );
#endif
     dump_pmem_buffers( surface, info , info->print);
    //  printf("\n");

     return true;
}

void
pmem_dump_surface_buffers( int pool_idx, int block_idx )
{
    PMEM_Dump_Info info;
    DevPMemDataShared *shared = g_pmem_data->shared;
    int pool_s, pool_e;
    int block_s, block_e;
    int i,j;
    int mem_total=0;
    int mem_used=0;


    info.total_block_size = 0;
    info.total_used_mem = 0;
    info.print = true;

    if(pool_idx<0)
         pool_s = 0, pool_e = VIDEO_POOL_NUM-1;
    else
        pool_s = pool_e = pool_idx;


    if(block_idx<0)
         block_s = 0, block_e = POOL_BLOCKS_MAX-1;
    else
        block_s = block_e = pool_idx;

    for( i=pool_s; i<=pool_e; i++)
    {
           for(j=block_s;j<=block_e;j++)
           {
                if(!shared->pmemBlocks[i][j].bAllocated)
                      continue;
                info.total_block_size = 0;
                info.total_used_mem = 0;
                info.offsetstart = shared->pmemBlocks[i][j].pmem_cpu_phys;
                info.offsetend = shared->pmemBlocks[i][j].pmem_cpu_phys+shared->pmemBlocks[i][j].pmem_block_length;
                mem_total += shared->pmemBlocks[i][j].pmem_block_length;
                printf("################dump block %d of pool %d [range0x%08x:0x%08x]\n\n", j, i,  shared->pmemBlocks[i][j].pmem_cpu_phys, shared->pmemBlocks[i][j].pmem_block_length);
                dfb_core_enum_surfaces( NULL, pmem_surface_callback, &info);
                mem_used += info.total_used_mem;
                printf("   &&blocked total used size=0x%08x\n", info.total_used_mem);
           }
           printf("pmem pool %d total memory 0x%08x, used memory 0x%08x\n", i, mem_total, mem_used);
    }


}
