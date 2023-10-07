/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrj√§l√§ <syrjala@sci.fi> and
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

#include <fusion/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>

#include <fusion/arena.h>
#include <fusion/shmalloc.h>
#include <fusion/lock.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/surface.h>
//#include <core/surface_pool.h>
#include <core/system.h>
#include <gfx/convert.h>
#include <misc/conf.h>
#include <direct/messages.h>
#include <core/core_system.h>

#include <MsCommon.h>
#include <drvSEM.h>
#include  <apiGOP.h>
#include <apiXC.h>

#include "libion.h"
#include "ion_system.h"
//#include "devmem.h"

D_DEBUG_DOMAIN( DFBION_System, "DFBION/System", "DFBION System" );

DFB_CORE_SYSTEM( ION )

/**********************************************************************************************************************/


/**********************************************************************************************************************/

//TODO
//static DFBIONData *m_data;    /* FIXME: Fix Core System API to pass data in all functions. */
//callbacks
static MS_U32 mstar_dfb_OSD_RESOURCE_SetFBFmt(MS_U16 pitch,MS_U32 addr , MS_U16 fmt )
{
   printf("set osd resource %08x, %d, %04x\n", addr, pitch, fmt);
   return 0x1;
}

//TODO
/*
static DFBResult
MapMemAndReg( DFBIONData    *data,
              unsigned long  mem_phys,
              unsigned int   mem_length,
              unsigned long  reg_phys,
              unsigned int   reg_length )
{
     int fd;

     fd = open( DEV_MEM, O_RDWR | O_SYNC );
     if (fd < 0) {
          D_PERROR( "System/DevMem: Opening '%s' failed!\n", DEV_MEM );
          return DFB_INIT;
     }

     data->mem = mmap( NULL, mem_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mem_phys );


     if (data->mem == MAP_FAILED) {
          D_PERROR( "System/DevMem: Mapping %d bytes at 0x%08lx via '%s' failed!\n", mem_length, mem_phys, DEV_MEM );
          close( fd );
          return DFB_INIT;
     }

     if (reg_phys && reg_length) {
          data->reg = mmap( NULL, reg_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, reg_phys );
          if (data->reg == MAP_FAILED) {
               D_PERROR( "System/DevMem: Mapping %d bytes at 0x%08lx via '%s' failed!\n", reg_length, reg_phys, DEV_MEM );
               munmap( data->mem, mem_length );
               close( fd );
               return DFB_INIT;
          }
     }

     close( fd );

     return DFB_OK;
}

static void
UnmapMemAndReg( DFBIONData   *data,
                unsigned int  mem_length,
                unsigned int  reg_length )
{
     munmap( data->mem, mem_length );

     if (reg_length)
          munmap( (void*) data->reg, reg_length );
}

*/

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


static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_ION;
     info->caps = CSCAPS_ACCELERATION;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "ION" );
}

static DFBResult
system_initialize( CoreDFB *core, void **data )
{
    DFBIONData       *ion;
    DFBIONDataShared *shared;
    FusionSHMPoolShared *pool;
    int fd, ret;
    
#ifdef DFB_ION_TLB    
    mtlb_hardware_info hw_info;
    mtlb_tlbclient_enable tlb_enable_data;
    int gob_used_by_dfb = 0;
    int i;
#endif

//TODO
/*
    if (!dfb_config->video_phys_cpu || !dfb_config->video_length) {
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
*/
    D_DEBUG_ION( DFBION_System, "%s()\n", __FUNCTION__ );

    ion = D_CALLOC( 1, sizeof(DFBIONData) );
    if (!ion)
        return D_OOM();

    D_MAGIC_SET(ion, DFBIONData);
    ion->core = core;

    pool = dfb_core_shmpool( core );

    shared = SHCALLOC( pool, 1, sizeof(DFBIONDataShared) );
    if (!shared) {
        D_FREE( ion );
        return D_OOSHM();
    }

    D_MAGIC_SET(shared, DFBIONDataShared);   
    shared->shmpool = pool;

//TODO
/*    ret = MapMemAndReg( ion,
                         dfb_config->video_phys_cpu, dfb_config->video_length,
                         dfb_config->mmio_phys,  dfb_config->mmio_length );
     if (ret) {
          SHFREE( pool, shared );
          D_FREE( data );
          return ret;
    }
    */
    fd = ion_open();
    if(fd < 0)
    {
         D_FREE( ion );
         SHFREE( pool, shared );
         return DFB_INIT;
    }
    ion->fd = fd;
    D_DEBUG_ION( DFBION_System, "ion->fd =%d  %s(%d)\n", fd ,__FUNCTION__,__LINE__);
    
#ifdef DFB_ION_TLB
    //ion cust init
    ion_cust_tlb_init(fd);

    //check mstar tlb hw configration
    ret = ion_cust_get_tlbinfo(fd, &hw_info);
    if(ret < 0)
    {
        D_FREE( ion );
        SHFREE( pool, shared );
        ion_close(fd);
        return DFB_INIT;
    }

    //whether GE/GOP°°support TLB
    shared->GE_support_tlb = hw_info.MIU_Has_TLB || hw_info.GE_Has_TLB; //ge support tlb

    if(hw_info.MIU_Has_TLB ) {
        shared->GOP_support_tlb = 0xff; //all gop support tlb
    } else if (hw_info.GOP_Has_TLB) {
        shared->GOP_support_tlb = hw_info.TLB_GOP_IDS; //some gop support tlb
    } else {
        shared->GOP_support_tlb = 0; //gop can't support tlb
    }
    
    D_DEBUG_ION( DFBION_System, "GE_support_tlb=%d, GOP_support_tlb=%x\n", shared->GE_support_tlb, shared->GOP_support_tlb );

    //check gop used by dfb
    for(i=0; i<dfb_config->mst_gop_counts; i++)
    {
        gob_used_by_dfb |= 1<<dfb_config->mst_gop_available[i];
    }

    //check which gop can enble tlb 
    shared->GOP_enabled_tlb = shared->GOP_support_tlb & gob_used_by_dfb;
    D_DEBUG_ION( DFBION_System, "gob_used_by_dfb=%d, GOP_enabled_tlb=%x\n", gob_used_by_dfb, shared->GOP_enabled_tlb );

    if(shared->GE_support_tlb || shared->GOP_enabled_tlb)
    {
        //reset gop which initianlize in mboot
        if(1<<dfb_config->mbootGOPIndex & shared->GOP_enabled_tlb)
            mstar_reset_gop(dfb_config->mbootGOPIndex);

        tlb_enable_data.clients= E_MTLB_GE_Src | E_MTLB_GE_Dest | E_MTLB_GOP;
        tlb_enable_data.gopidmask = shared->GOP_enabled_tlb;
        tlb_enable_data.enable = true;
        ret = ion_cust_tlb_enable(fd, &tlb_enable_data);

        //enabe gop tlb 
        if(shared->GOP_enabled_tlb)
        {
           
        }
    }

    //get tlb table addr in miu0/1/2
    shared->tlb_table_addr_miu0 = hw_info.tlb_table_addr_miu0 + dfb_config->mst_miu0_hal_offset;
    shared->tlb_table_addr_miu1 = hw_info.tlb_table_addr_miu1 + dfb_config->mst_miu1_hal_offset;
    shared->tlb_table_addr_miu2 = hw_info.tlb_table_addr_miu2 + dfb_config->mst_miu2_hal_offset;
#endif

    ion->shared = shared;
    /*
    * Must be set before initializing the pools!
    */
    *data = ion;
    //TODO
    //m_data = ion;

    /*
    * Master init
    */
    
    dfb_surface_pool_initialize( core, &DFBIONSurfacePoolFuncs, &shared->pool);
    //TODO
    //dfb_surface_pool_initialize( core, &devmemSurfacePoolFuncs, &shared->pool_devmem );
    dfb_surface_pool_initialize(core,&IonPreallocSurfacePoolFuncs,&shared->preallocpool);

    fusion_arena_add_shared_field( dfb_core_arena( core ), "ion", shared );

    return DFB_OK;
}

static DFBResult
system_join( CoreDFB *core, void **data )
{
     DFBResult     ret;
     void         *ptr;
     DFBIONData *ion;
     DFBIONDataShared *shared;
     int fd;

     D_DEBUG_ION( DFBION_System, "%s()\n", __FUNCTION__ );

//TODO
/*
     D_ASSERT( m_data == NULL );
     if (!dfb_config->video_phys_cpu || !dfb_config->video_length) {
          D_ERROR( "System/DevMem: Please supply 'video-phys = 0xXXXXXXXX' and 'video-length = XXXX' options!\n" );
          return DFB_INVARG;
     }

     if (dfb_config->mmio_phys && !dfb_config->mmio_length) {
          D_ERROR( "System/DevMem: Please supply both 'mmio-phys = 0xXXXXXXXX' and 'mmio-length = XXXX' options or none!\n" );
          return DFB_INVARG;
     }
     */
     ion = D_CALLOC( 1, sizeof(DFBIONData) );
     if (!ion)
          return D_OOM();

     D_MAGIC_SET(ion, DFBIONData);
     ion->core = core;

     fd = ion_open();
     if(fd < 0)
     {
         D_FREE( ion );
         return DFB_INIT;
     }
     ion->fd = fd;
     D_DEBUG_ION( DFBION_System, "ion->fd =%d %s(%d)\n", fd ,__FUNCTION__,__LINE__);

     fusion_arena_get_shared_field( dfb_core_arena( core ), "ion", &ptr );

     ion->shared = shared = ptr;
//TODO
/*     ret = MapMemAndReg( ion,
                         dfb_config->video_phys_cpu, dfb_config->video_length,
                         dfb_config->mmio_phys,  dfb_config->mmio_length );
     if (ret) {
          D_FREE( ion );
          return ret;
     }
     */
     

     /*
      * Must be set before joining the pools!
      */
      
     //*data = m_data = ion;
     *data = ion;

     /*
      * Slave init
      */
     if (shared->pool)
        dfb_surface_pool_join( core, shared->pool, &DFBIONSurfacePoolFuncs );

     //TODO  
     /*
     if (shared->pool_devmem)
         dfb_surface_pool_join(core, shared->pool_devmem, &devmemSurfacePoolFuncs );
     */
     if (shared->preallocpool)
        dfb_surface_pool_join(core,shared->preallocpool,&IonPreallocSurfacePoolFuncs);

     return DFB_OK;
}

static DFBResult
system_shutdown( bool emergency )
{
    DFBIONData *ion    = dfb_system_data();
    DFBIONDataShared *shared = ion->shared;

    D_DEBUG_ION( DFBION_System, "%s()\n", __FUNCTION__ );

    /*
    * Master deinit
    */

    //TODO
    /*
    if (shared->pool_devmem)
    {
        dfb_surface_pool_destroy( shared->pool_devmem);
        UnmapMemAndReg( ion, dfb_config->video_length, dfb_config->mmio_length );
    }
    */
    if (shared->pool)
      dfb_surface_pool_destroy( shared->pool);

    ion_close(ion->fd);

    /*
    * Shared deinit (master only)
    */
    SHFREE( dfb_core_shmpool( ion->core ), shared );

    D_FREE( ion );

    return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
     DFBIONData *ion    = dfb_system_data();
     DFBIONDataShared *shared = ion->shared;

     D_DEBUG_ION( DFBION_System, "%s()\n", __FUNCTION__ );

     /*
      * Slave deinit
      */
     if (shared->pool)
          dfb_surface_pool_leave( shared->pool);
     /*
     if (shared->pool_devmem)
     {
         dfb_surface_pool_leave( shared->pool_devmem);
         UnmapMemAndReg( ion, dfb_config->video_length, dfb_config->mmio_length );
     }*/
     ion_close(ion->fd);
     //m_data = NULL;
     D_FREE( ion );

     return DFB_OK;
}

static DFBResult
system_suspend( void )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
system_resume( void )
{
     return DFB_UNIMPLEMENTED;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
     return NULL;
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
     return 0;
}

static void *
system_video_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_videoram_length( void )
{
     return 0;
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
}

static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
}

static unsigned int
system_video_memory_available(void * arg)
{
   /* unsigned int mem_available = 0;
     //get the free memory in the pool
     if(m_data&&m_data->shared)
     {

         if(m_data->shared->pool)
         {
             DevMemPoolData *data = m_data->shared->pool_devmem->data;
             if(data)
             {
                mem_available +=data->manager->avail;
             }
         }
     }

    return mem_available;*/
    return (unsigned int)0;
}
