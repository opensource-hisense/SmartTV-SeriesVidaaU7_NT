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

//#include "surfacemanager.h"
#include <fusion/shmalloc.h>
#include <direct/hash.h>
#include <core/surface_pool.h>
#include "mdrv_mtlb_st.h"
#include "libion.h"

//#include <fcntl.h>
//#include <sys/mman.h>

//extern const SurfacePoolFuncs devmemSurfacePoolFuncs;
extern const SurfacePoolFuncs DFBIONSurfacePoolFuncs;
extern const SurfacePoolFuncs IonPreallocSurfacePoolFuncs;


typedef struct {
    int       magic;
    FusionSHMPoolShared *shmpool;
    CoreSurfacePool     *pool_devmem;
    CoreSurfacePool     *pool_secondary;
    CoreSurfacePool     *pool;
    CoreSurfacePool     *preallocpool;
    //TODO
    //SurfaceManager      *manager;

#ifdef DFB_ION_TLB
    //mstar customer data
    bool GE_support_tlb; //GE support TLB
    int  GOP_support_tlb; //each bit stand for a gop, ex, 111111B stand fo 0~5 gop support tlb
    int  GOP_enabled_tlb;  //which gop enable tlb, each bit stand for a gop
    unsigned long tlb_table_addr_miu0;  //the physical addr of tlb  table in MIU0
    unsigned long tlb_table_addr_miu1;  //the physical addr of tlb  table in MIU1
    unsigned long tlb_table_addr_miu2;  //the physical addr of tlb  table in MIU2    
#endif
} DFBIONDataShared;

typedef struct {
    int       magic;
    CoreDFB  *core;
    DFBIONDataShared    *shared;
    int fd;
    //devmem
    void                *mem;
    void                *mem_secondary;
    volatile void       *reg;
} DFBIONData;


typedef struct {
    int             magic;
    FusionCall      call;          /* fusion call */
    FusionReactor   *reactor;
    FusionSkirmish       lock;
}DFBIONPoolSharedData;

typedef struct {
    int             magic;

    CoreDFB        *core;
    DFBIONData   *systemData;
    DFBIONPoolSharedData *sharedData;
    CoreSurfacePool *pool;
    DirectHash      *hash;
    Reaction         reaction;
} DFBIONPoolLocalData;

typedef struct {
    int shared_fd;  //shared_fd in currrent process 
    int ion_handle; //ion_handle in currrent process 
    void *cpu_addr; //mmap addr in current process
    int size;
}DFBIONLockData;

typedef struct {
    int   magic;
    int   pitch;
    int   size;
    int   miuType;
    unsigned long   phys;
    int shared_fd;
    int pid;
    int ion_hnd;
    int serial; //serial number
    void *cpu_addr;
    EN_MTLB_AddressType addressType;
} DFBIONAllocationData;

typedef enum {
     DPC_ION_ALLOCATE = 1,
     DPC_ION_DEALLOCATE= 2,
     DPC_ION_COMMAND_MAX,
}DFBIONCommand;

typedef struct
{
    int type;
    int size;
}DFBIONCMDInfo;

typedef struct
{
    int shared_fd;
    int serial;
}DFBIONMsgType;
#endif
