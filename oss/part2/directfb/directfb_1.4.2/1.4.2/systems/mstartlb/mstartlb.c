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
#include <string.h>

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
//neo.he
#include "mstartlb.h"
#include "surfacemanager.h"


#include <core/core_system.h>
#include <MsCommon.h>
#include <drvSEM.h>
#include  <apiGOP.h>
#include <apiXC.h>


DFB_CORE_SYSTEM( mstartlb )

#define DEV_MEM     "/dev/mem"

/**********************************************************************************************************************/

DevMstarTLBData *g_mstartlb_data;    /* FIXME: Fix Core System API to pass data in all functions. */


//callbacks
static MS_U32 mstar_dfb_OSD_RESOURCE_SetFBFmt(MS_U16 pitch,MS_U32 addr , MS_U16 fmt )
{
   printf("set osd resource %08x, %d, %04x\n", addr, pitch, fmt);
   return 0x1;
}

static MS_BOOL mstar_sc_is_interlace(void)
{
    //use MApi_XC_GetStatus for IPC issue.
    XC_ApiStatus xcStatus;
    if (MApi_XC_GetStatus(&xcStatus, MAIN_WINDOW))
    {
        return xcStatus.bInterlace;
    }
    else
    {
        printf("Error! Cannot get XC Status! \n");
    }
}

static MS_U16 mstar_sc_get_ip0_h_cap_start(void)
{
  MS_XC_DST_DispInfo dstDispInfo;
  MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN);
  return dstDispInfo.DEHST;

}

static void mstar_XC_Sys_PQ_ReduceBW_ForOSD(MS_U8 PqWin, MS_BOOL bOSD_On)
{

}

static void mstar_reset_gop( u8 gop_id)
{
    XC_INITDATA XC_InitData;
    GOP_InitInfo gopInitInfo;
      //Init MsOS & SEM:
    //MsOS_Init();
    //MDrv_SEM_Init();
    MApi_XC_Init(&XC_InitData, 0);

    //Register callback functions:
    MApi_GOP_RegisterFBFmtCB(mstar_dfb_OSD_RESOURCE_SetFBFmt);
    MApi_GOP_RegisterXCIsInterlaceCB(mstar_sc_is_interlace);
    MApi_GOP_RegisterXCGetCapHStartCB(mstar_sc_get_ip0_h_cap_start);
    MApi_GOP_RegisterXCReduceBWForOSDCB(mstar_XC_Sys_PQ_ReduceBW_ForOSD);

    memset(&gopInitInfo, 0, sizeof(GOP_InitInfo));
    MApi_GOP_InitByGOP(&gopInitInfo,  gop_id);
}


static DFBResult MapReg(DevMstarTLBData    *data,unsigned long  reg_phys,unsigned int   reg_length)
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
UnmapReg( DevMstarTLBData   *data,
                unsigned int  reg_length )
{
     if (reg_length && data->reg)
          munmap( (void*) data->reg, reg_length );
}



/**********************************************************************************************************************/

static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_MSTARTLB;
     info->caps = CSCAPS_ACCELERATION;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "MSTARTLB" );
}


static DFBResult
system_initialize( CoreDFB *core, void **ret_data )
{
    DFBResult            ret;
    DevMstarTLBData          *data = NULL; //Fix converity END
    DevMstarTLBDataShared    *shared;
    FusionSHMPoolShared *pool;

    D_ASSERT( g_mstartlb_data == NULL );

    if (dfb_config->mmio_phys && !dfb_config->mmio_length) 
    {
      D_ERROR( "System/mstartlb: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
      return DFB_INVARG;
    }

    data = D_CALLOC( 1, sizeof(DevMstarTLBData) );
    if (!data)
        return D_OOM();
    
    D_MAGIC_SET(data, DevMstarTLBData);
    data->core = core;

    pool = dfb_core_shmpool( core );

    shared = SHCALLOC( pool, 1, sizeof(DevMstarTLBDataShared) );
    if (!shared) {
        D_FREE( data );
        return D_OOSHM();
    }

    D_MAGIC_SET(shared, DevMstarTLBDataShared);   

    shared->shmpool = pool;
    data->shared = shared;

    *ret_data = g_mstartlb_data = data;

    ret = MapReg(data,dfb_config->mmio_phys,  dfb_config->mmio_length );

    if (ret) 
    {
        SHFREE( pool, shared );
        D_FREE( data );
        return ret;
    }

    //system("mknod /dev/semtlb c 182 0");

    system("insmod /config/krl/usb/mtlb.ko");

    ret = OpenHWTLBDevice(data);
    if(ret)
    {
        goto ERROR;
    }

    ret = HWTLB_GetRange(data);
    if(ret)
    {
        goto ERROR;
    }

    ret = MapHWTLBMem(data);
    if (ret) 
    {
        goto ERROR;
    }

    //reset gop which initianlize in mboot
    mstar_reset_gop(dfb_config->mbootGOPIndex);
    
    ret = HWTLB_Enable(data);
    if (ret) 
    {
        goto ERROR;
    }

    dfb_surface_pool_initialize( core, &DevMstarTLBSurfacePoolFuncs, &shared->pool );
    if(!dfb_config->bPrealloc_map_tlb)
        dfb_surface_pool_initialize(core,&TlbPreallocSurfacePoolFuncs,&shared->preallocpool);
    fusion_arena_add_shared_field( dfb_core_arena( core ), "devmstartlb", shared );

    dfb_config->bUsingHWTLB = true;
    return DFB_OK;

ERROR:

    UnmapReg(data,dfb_config->mmio_length );
    SHFREE( pool, shared );
    D_FREE( data );
    return DFB_INIT;
}


static DFBResult
system_join( CoreDFB *core, void **ret_data )
{
    DFBResult         ret;
    void             *tmp;
    DevMstarTLBData       *data;
    DevMstarTLBDataShared *shared;
    int i=0, j;

    D_ASSERT( g_mstartlb_data == NULL );

    if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
        D_ERROR( "System/mstartlb: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
        return DFB_INVARG;
    }

    data = D_CALLOC( 1, sizeof(DevMstarTLBData) );
    if (!data)
        return D_OOM();

    ret = fusion_arena_get_shared_field( dfb_core_arena( core ), "devmstartlb", &tmp );
    if (ret) {
        D_FREE( data );
        return ret;
    }

    D_MAGIC_SET(data, DevMstarTLBData);

    data->shared = shared = tmp;
    data->core = core;
    *ret_data = g_mstartlb_data = data;

    ret = MapReg(data,dfb_config->mmio_phys,  dfb_config->mmio_length );

    if (ret) 
    {
        D_FREE( data );
        return ret;
    }

    ret = OpenHWTLBDevice(data);
    if(ret)
    {
        D_FREE( data );
        return ret;
    }

    ret = HWTLB_GetRange(data);
    if(ret)
    {
        goto ERROR;
    }

    ret = MapHWTLBMem(data);
    if (ret) 
    {
        D_FREE( data );
        return ret;
    }   

    dfb_surface_pool_join(core, shared->pool, &DevMstarTLBSurfacePoolFuncs );
    if(!dfb_config->bPrealloc_map_tlb)
        dfb_surface_pool_join(core,shared->preallocpool,&TlbPreallocSurfacePoolFuncs);
    dfb_config->bUsingHWTLB = true;
    return DFB_OK;

ERROR:

    UnmapReg(data,dfb_config->mmio_length );
    D_FREE( data );
    return DFB_INIT;
}

static DFBResult
system_shutdown( bool emergency )
{
    DevMstarTLBDataShared *shared;
    D_ASSERT( g_mstartlb_data != NULL );
    D_MAGIC_ASSERT(g_mstartlb_data, DevMemData);

    shared = g_mstartlb_data->shared;

    D_MAGIC_ASSERT(shared, DevMemDataShared);

    dfb_surface_pool_destroy(shared->pool );
    UnMapHWTLBMem(g_mstartlb_data);
    CloseHWTLBDevice(g_mstartlb_data);
    printf("%s:%d\n", __FUNCTION__,__LINE__); 

    D_MAGIC_CLEAR(shared);
    SHFREE( shared->shmpool, (void*)shared );
    UnmapReg(g_mstartlb_data,dfb_config->mmio_length );
    D_MAGIC_CLEAR(g_mstartlb_data);
    D_FREE( g_mstartlb_data );
    g_mstartlb_data = NULL;

    return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{

    DevMstarTLBDataShared *shared;
    D_ASSERT( g_mstartlb_data != NULL );

    shared = g_mstartlb_data->shared;
    D_ASSERT( shared != NULL );
    dfb_surface_pool_leave(shared->pool );

    UnMapHWTLBMem(g_mstartlb_data);
    CloseHWTLBDevice(g_mstartlb_data);

    printf("%s:%d\n", __FUNCTION__,__LINE__); 

    UnmapReg(g_mstartlb_data,dfb_config->mmio_length );
    D_MAGIC_CLEAR( g_mstartlb_data);
    D_FREE( g_mstartlb_data );
    g_mstartlb_data = NULL;

    return DFB_OK;
}

static DFBResult
system_suspend( void )
{
     D_ASSERT( g_mstartlb_data != NULL );

     return DFB_OK;
}

static DFBResult
system_resume( void )
{
     D_ASSERT( g_mstartlb_data != NULL );

     return DFB_OK;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
     D_ASSERT( g_mstartlb_data != NULL );

     return g_mstartlb_data->reg + offset;
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
     D_ASSERT( g_mstartlb_data != NULL );
     return g_mstartlb_data->mem+offset;
     
}

static unsigned int
system_videoram_length( void )
{
     D_ASSERT( g_mstartlb_data != NULL );
     return g_mstartlb_data->shared->msttlb_videoMem_length;
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
    unsigned int mem_available = 0;;

     //get the free memory in the pool
     if(g_mstartlb_data&&g_mstartlb_data->shared)
     {
         if(g_mstartlb_data->shared->pool)
         {
             DevMstarTLBPoolSharedData *data = g_mstartlb_data->shared->pool->data;
             if(data)
             {
                mem_available +=data->manager->avail;
             }
         }         
     }

    return mem_available;
}

