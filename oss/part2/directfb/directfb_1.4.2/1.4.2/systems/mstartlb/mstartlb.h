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

#ifndef __MSTARTLB_H__
#define __MSTARTLB_H__

#include <fusion/shmalloc.h>

#include <core/surface_pool.h>

#include "surfacemanager.h"
#include "mdrv_mtlb_st.h"
#include "mdrv_mtlb_interface.h"

#define IS_POWER2(x) (!((x)&((x)-1)))
#ifndef MIN
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif


#define MIU_1  (1)
#define MIU_0  (0)
#define MIU_NONE  (2)
#define DEV_MSTARTLB "/dev/semtlb"

extern const SurfacePoolFuncs DevMstarTLBSurfacePoolFuncs;
extern const SurfacePoolFuncs TlbPreallocSurfacePoolFuncs;


typedef struct {
    int       magic;
    FusionSHMPoolShared *shmpool;
    CoreSurfacePool     *pool;
    CoreSurfacePool     *preallocpool;
    int      msttlb_TLBAddr_start;
    int      msttlb_TLBAddr_length;
    int      msttlb_videoMem_length;
    int      msttlb_miu0_videoMem_length;
    int      msttlb_miu1_videoMem_length;    
    FusionSkirmish               lock;
    FusionCall                   call;          /* fusion call */
} DevMstarTLBDataShared;

typedef struct {
    int       magic;
    CoreDFB  *core;
    DevMstarTLBDataShared    *shared;
    void *mem;
    void *reg;
    int privatedUsage;
    int fd;
} DevMstarTLBData;




typedef struct {
     int             magic;
     SurfaceManager *manager;
} DevMstarTLBPoolSharedData;

typedef struct {
    int             magic;

    CoreDFB        *core;
    DevMstarTLBData   *systemData;
    DevMstarTLBPoolSharedData *sharedData;
    CoreSurfacePool *pool;
    void *mem;
} DevMstarTLBPoolLocalData;


typedef struct {
    int   magic;
    int   offset;
    int   pitch;
    int   size;
    Chunk *chunk;
    int   miuType;
    int   phys;
    int   prealloc_offset;
    unsigned int handle;
} DevMstarTLBAllocationData;




typedef enum {
     DPC_MEM_ALLOCATE = 1,
     DPC_MEM_DEALLOCATE= 2,
     DPC_COMMAND_MAX,
}DFBMstarTLBCommand;

typedef struct
{
    int type;
    int size;
}DFBMstarTLBCMDInfo;


DFBResult OpenHWTLBDevice(DevMstarTLBData    *data);
void CloseHWTLBDevice(DevMstarTLBData    *data);
DFBResult MapHWTLBMem(DevMstarTLBData    *data);
void UnMapHWTLBMem(DevMstarTLBData    *data);
DFBResult HWTLB_TransferBuffer(DevMstarTLBData*data,MoveInfo *transfer);
DFBResult HWTLB_AllocateBuffer(DevMstarTLBData * data,BufferInfo * tlbAlloc);
DFBResult HWTLB_DeallocteBuffer(DevMstarTLBData * data,AllocationHandle handle);
DFBResult HWTLB_GetRange(DevMstarTLBData *data );

DFBResult HWTLB_Enable(DevMstarTLBData*data);

#endif

