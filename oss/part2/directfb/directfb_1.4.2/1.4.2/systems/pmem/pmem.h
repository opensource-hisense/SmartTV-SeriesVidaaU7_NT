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

#ifndef __PMEM_PMEM_H__
#define __PMEM_PMEM_H__

#include <fusion/shmalloc.h>

#include <core/surface_pool.h>

#include "surfacemanager.h"


#define DEV_PMEM0     "/dev/pmem0"
#define DEV_PMEM1    "/dev/pmem1"

#define IS_POWER2(x) (!((x)&((x)-1)))
#ifndef MIN
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif


#define POOL_BLOCKS_MAX 16

typedef enum
{
     VIDEO_POOLS_INDEX0 = 0,
     VIDEO_POOLS_INDEX1 = 1,
     VIDEO_POOLS_DECLARABLE  = 2,
     VIDEO_POOL_NUM = VIDEO_POOLS_DECLARABLE+1
}PMEM_POOL_INDEX;




extern const SurfacePoolFuncs DevPMemSurfacePoolFuncs;

typedef struct {
    unsigned long pmem_cpu_phys;
    unsigned long pmem_block_length;
    bool  bAllocated;
}PMemBlockSharedInfo;

typedef struct {
    void * pmem_vbase;
    unsigned long last_cpu_phys;//last mapped physical address, if not equal to current one, will need to remap
    unsigned long last_mem_len;
    int  pmem_fd;   //pmem_fd, used in master
    bool bPA2KSEG1;

}PMemBlockInfo;

typedef struct {
     int       magic;
     FusionSHMPoolShared *shmpool;

     CoreSurfacePool     *pool[VIDEO_POOL_NUM];
     unsigned                  pool_init_size[VIDEO_POOL_NUM];
     unsigned                  pool_step_size[VIDEO_POOL_NUM];
     int                           pool_miu[VIDEO_POOL_NUM];

    // CoreSurfacePool     *pool_secondary;
     //SurfaceManager      *manager;
     PMemBlockSharedInfo  pmemBlocks[VIDEO_POOL_NUM][POOL_BLOCKS_MAX];
     FusionSkirmish               lock;
} DevPMemDataShared;

typedef struct {
     int       magic;
     CoreDFB  *core;
     DevPMemDataShared    *shared;
     PMemBlockInfo        pmemBlockData[VIDEO_POOL_NUM][POOL_BLOCKS_MAX];
     volatile void       *reg;
     int privatedUsage;
} DevPMemData;


typedef struct {
     int             magic;
     SurfaceManager *manager;
     FusionReactor               *reactor;       /* event dispatcher */
   //  FusionSkirmish  lock;
     FusionCall                   call;          /* fusion call */
     Chunk          *chunk_head[POOL_BLOCKS_MAX];

} DevPMemPoolSharedData;

typedef struct {
     int             magic;

     CoreDFB        *core;
     DevPMemData   *systemData;
     DevPMemPoolSharedData *sharedData;
     CoreSurfacePool *pool;
     Reaction        reaction;
} DevPMemPoolLocalData;


typedef struct {
     int   magic;
     int   offset;
     int   pitch;
     int   size;
     Chunk *chunk;
} DevPMemAllocationData;


typedef enum
{
    DPET_REQMEMBLOCK = 1,
    DPET_DEL_MEMBLOCK = 2,
    DPET_MAXMUM ,

}DFBPMemEventType;


typedef struct
{
    unsigned int  block_size;
    int           pool_index;
    int           block_index;
    DFBPMemEventType type;
}DFBPMemEvent;

typedef enum {
     DPC_REQ_MEMBLOCK = 1,
     DPC_DEL_MEMBLOCK = 2,
     DPC_COMMAND_MAX,
}DFBPMemCommand;

typedef struct
{
    int min_size ;
    int pool_index;
    int block_index;
}DFBPMemCMDInfo;

DFBResult
PMemAllocMemBlock( DevPMemData    *data,
              int  pool_index, int block_idx,
              unsigned long  *mem_phys,
              unsigned int   mem_length, unsigned int *allocated_len );
DFBResult PMemAssureMappedMemory(DevPMemData    *data,
              int pool_index,   int block_idx );
void PMemReleaseMemoryBlock(DevPMemData   *data, int pool_index,int block_idx);

const char *PMemGetPoolPMemMIUInfo(int pool_idx, int *pMIU);
bool PMEMGetSpaceInfo(int pool_idx, unsigned *free_size, unsigned *max_block_size);
void PMemDumpPMemInfo(void);

#endif

