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
#include <fcntl.h>
#include <sys/mman.h>
#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/hash.h>
#include <core/core.h>
#include <core/surface_pool.h>

#include <gfx/convert.h>
#include <linux/unistd.h>

#include <misc/conf.h>
#include  <apiGFX.h>


#include "libion.h"
#include "ion_system.h"



D_DEBUG_DOMAIN( DFBION_Surfaces, "DFBION/Surfaces", "DFBION Surface Pool" );

#define DEV_ION     "/dev/ion"

#define ALIGNSIZE 4096


/**********************************************************************************************************************/



int dfb_ion_alloc(int fd,     ion_alloc_data *alloc_data)
{
    ion_handle_t ion_hnd;
    int          shared_fd;
    int size = alloc_data->length;
    static int serial = 0;
    void *cpu_ptr;
    int ret;
    unsigned long bus_addr = 0x0;
    
    D_DEBUG_ION( DFBION_Surfaces, " ion_cust_alloc size=0x%x %s(%d)\n",size,__FUNCTION__,__LINE__);
    
    ret = ion_cust_alloc(fd, size,ALIGNSIZE,alloc_data->heap_mask,alloc_data->flag,&ion_hnd );
    if(ret)
    {
        D_DEBUG_ION(DFBION_Surfaces ,"Failed to ion_alloc from ion_client:%d   %s(%d)\n", fd,__FUNCTION__,__LINE__);
        return -1;
    }

    bus_addr = ion_get_user_data(fd, ion_hnd);
    if(bus_addr == 0)
    {
        D_DEBUG_ION(DFBION_Surfaces ,"get phy address fail\n");
        return -1;
    } else {
        D_DEBUG_ION(DFBION_Surfaces ,"the contiguous memory bus addr is 0x%lx  %s(%d)\n",(unsigned long)bus_addr,__FUNCTION__,__LINE__);
    }
    alloc_data->phys = bus_addr;
    ret = ion_map(fd , ion_hnd, &shared_fd);
        if(ret !=0) {
            return -1;
    }
    cpu_ptr = (unsigned char*)mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0 );
    D_DEBUG_ION(DFBION_Surfaces ,"the memory cpu addr is %p , %s(%d)\n", cpu_ptr,__FUNCTION__,__LINE__);
    if ( MAP_FAILED == cpu_ptr )
    {
        D_DEBUG_ION( DFBION_Surfaces, "ion_map( %d ) failed  ,\%s(%d)n", fd ,__FUNCTION__,__LINE__);
        if ( 0 != ion_free( fd, ion_hnd ) ) 
            D_DEBUG_ION(DFBION_Surfaces, "ion_free( %d ) failed, %s(%d)\n", fd,__FUNCTION__,__LINE__ );
        close( shared_fd );
        return -1;
    }

    
    if(alloc_data->initzero)
    {
        memset(cpu_ptr, 0, size);
    }
    
    alloc_data->handle = ion_hnd;
    alloc_data->shared_fd = shared_fd;
    alloc_data->pid = getpid();
    alloc_data->serial = serial++;
    alloc_data->cpu_addr = cpu_ptr;

    return ret;
}


int dfb_ion_dealloc(int fd, ion_alloc_data *alloc_data)
{
    D_DEBUG_ION( DFBION_Surfaces,"close shard_fd =%d, pid = %d\n", alloc_data->shared_fd, alloc_data->pid );

    if(alloc_data->cpu_addr) {
        munmap(alloc_data->cpu_addr, alloc_data->length);
    }

    if(alloc_data->shared_fd > 0)
         close(alloc_data->shared_fd);
    
    int ret = ion_free(fd, alloc_data->handle);
    if(ret < 0)
    {
        D_DEBUG_ION( DFBION_Surfaces, "ion_free failed!\n" );
        return -1;
    }
    return ret;
}

DFBResult _dfb_ion_alloc(DFBIONPoolLocalData *data, ion_alloc_data * ionAlloc)
{
     ion_alloc_data *parameter;
     FusionSHMPoolShared *shm_pool;
     int ret;
 
     shm_pool = dfb_core_shmpool_data(data->systemData->core);
     D_ASSERT(shm_pool);
     
     parameter = SHCALLOC(shm_pool, 1, sizeof(ion_alloc_data));
     if (!parameter) {
          return D_OOSHM();
     }
 
     memcpy(parameter,ionAlloc,sizeof(ion_alloc_data));
     if(fusion_call_execute(&(data->sharedData->call), FCEF_NONE, DPC_ION_ALLOCATE, parameter, &ret ))
     {
         SHFREE(shm_pool, parameter);
         return DFB_FUSION;
     }
     memcpy(ionAlloc,parameter,sizeof(ion_alloc_data));
     SHFREE(shm_pool, parameter);
 
     return ret;
}


DFBResult _dfb_ion_dealloc(DFBIONPoolLocalData *data, ion_alloc_data * ionAlloc)
{
    ion_alloc_data *parameter;
    FusionSHMPoolShared *shm_pool;
    int ret;

    shm_pool = dfb_core_shmpool_data(data->systemData->core);
    D_ASSERT(shm_pool);

    parameter = SHCALLOC(shm_pool, 1, sizeof(ion_alloc_data));
    if (!parameter) {
       return D_OOSHM();
    }

    memcpy(parameter,ionAlloc,sizeof(ion_alloc_data));
    if(fusion_call_execute(&(data->sharedData->call), FCEF_NONE, DPC_ION_DEALLOCATE, parameter, &ret ))
    {
      SHFREE(shm_pool, parameter);
      return DFB_FUSION;
    }
    SHFREE(shm_pool, parameter);

    return ret;
}

/**********************************************************************************************************************/

 static FusionCallHandlerResult
_DFBION_call_handler( int           caller,   /* fusion id of the caller */
                           int           call_arg, /* optional call parameter */
                           void         *call_ptr, /* optional call parameter */
                           void         *ctx,      /* optional handler context */
                           unsigned int  serial,
                           int          *ret_val )
{
    DFBIONCommand  command = call_arg;
    DFBIONPoolLocalData          *local = ctx;
    DFBResult ret = DFB_FAILURE;

    D_MAGIC_ASSERT( local, DFBIONPoolLocalData );
    switch (command)
    {
        case  DPC_ION_ALLOCATE:
        {
            ion_alloc_data *ionAlloc =(ion_alloc_data *) call_ptr;
            ret = dfb_ion_alloc(local->systemData->fd, ionAlloc);

            break;
        }
        
        case  DPC_ION_DEALLOCATE:
        {
            DFBIONMsgType evt;
            ion_alloc_data *ionAlloc =(ion_alloc_data *) call_ptr;

            //dispatch reactor to client, no need reactor to master, because master didn't have hash
            evt.shared_fd = ionAlloc->shared_fd;
            evt.serial = ionAlloc->serial;
            
            D_DEBUG_ION( DFBION_Surfaces, "fusion_reactor_dispatch, hashkey =%d, serial = %d\n", evt.shared_fd,  evt.serial);
            ret = fusion_reactor_dispatch(local->sharedData->reactor, &evt, false, NULL);
            if (ret) {
                 D_DEBUG_ION( DFBION_Surfaces, "fusion_reactor_dispatch fail!\n" );
            }
            ret = dfb_ion_dealloc(local->systemData->fd, ionAlloc);


            break;
        }

        default :
            *ret_val = DFB_BUG;

    }
    
    D_DEBUG_ION( DFBION_Surfaces, "%s(),fd = %d, command = %d, ret = %d\n", 
              __FUNCTION__ , local->systemData->fd, command, ret);
    if(ret == DFB_OK)
    {
        *ret_val = DFB_OK ;
    }
    else
    {
        *ret_val = DFB_BUG ;
    }
    return FCHR_RETURN;
}


static ReactionResult ionhash_reaction_callback( const void *msg_data,  void *ctx )
{
    DFBIONMsgType *evt  = msg_data;
    DFBIONPoolLocalData *local = ctx;
    DFBIONLockData *node;

    DFBMatrix matrix;
    
    matrix.xx = 0x10000;
    matrix.xy = 0x00000;
    matrix.yx = 0x00000;
    matrix.yy = 0x10000;

    node = direct_hash_lookup( local->hash, evt->shared_fd );
    if(node) {
        D_DEBUG_ION( DFBION_Surfaces, "direct_hash_removed \n");
        if(node->cpu_addr) {
            D_DEBUG_ION( DFBION_Surfaces, "munmap addr %x\n", node->cpu_addr);
            munmap(node->cpu_addr, node->size );
        }
        close(node->shared_fd);
        ion_free(local->systemData->fd, node->ion_handle);

        direct_hash_remove(local->hash, evt->shared_fd, &matrix, evt->serial);
        D_FREE( node );
    }
    return RS_OK;

}



static int
DFBIONPoolDataSize( void )
{
     return sizeof(DFBIONPoolSharedData);
}

static int
DFBIONPoolLocalDataSize( void )
{
     return sizeof(DFBIONPoolLocalData);
}

static int
DFBIONAllocationDataSize( void )
{
     return sizeof(DFBIONAllocationData);
}


static DFBResult
DFBIONInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     DFBResult            ret;
     DFBIONPoolSharedData      *sharedData   = pool_data;
     DFBIONPoolLocalData *local  = pool_local;

     char tmp_buf[32];
     D_DEBUG_ION( DFBION_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( sharedData != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( ret_desc != NULL );


     ret_desc->caps              = CSPCAPS_PHYSICAL | CSPCAPS_VIRTUAL;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE | CSAF_SHARED;
     ret_desc->types             = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority          = CSPP_DEFAULT;
     ret_desc->size              = 0;

     /* For hardware layers */
     ret_desc->access[CSAID_LAYER0] = CSAF_READ;
     ret_desc->access[CSAID_LAYER1] = CSAF_READ;
     ret_desc->access[CSAID_LAYER2] = CSAF_READ;
     ret_desc->access[CSAID_LAYER3] = CSAF_READ;
     ret_desc->access[CSAID_LAYER4] = CSAF_READ;
     ret_desc->access[CSAID_LAYER5] = CSAF_READ;
     ret_desc->access[CSAID_LAYER6] = CSAF_READ;
     ret_desc->access[CSAID_LAYER7] = CSAF_READ;

     sprintf(tmp_buf, "/dev/ion");

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, tmp_buf);

     D_MAGIC_SET( sharedData, DFBIONPoolSharedData );
     D_MAGIC_SET( local, DFBIONPoolLocalData );
     
     local->core = core;
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;


     sharedData->reactor = fusion_reactor_new( sizeof(DFBIONMsgType), "ION_POOL", dfb_core_world(core) );

     D_DEBUG_ION( DFBION_Surfaces, "reactor = %x\n", sharedData->reactor );

     fusion_call_init( &sharedData->call, _DFBION_call_handler, local, dfb_core_world(core) );

     fusion_skirmish_init( &sharedData->lock, "ION POOL", dfb_core_world(core) );

     return DFB_OK;
}

static DFBResult
DFBIONJoinPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *pool_local,
               void                       *system_data )
{
     DFBIONPoolSharedData      *sharedData   = pool_data;
     DFBIONPoolLocalData *local  = pool_local;

     D_DEBUG_ION( DFBION_Surfaces, "%s(), pid = %d\n", __FUNCTION__, getpid());

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBIONPoolSharedData );
     D_ASSERT( local != NULL );

     local->core = core;
     local->sharedData= sharedData;
     local->pool = pool;
     local->systemData = system_data;

     int ret = direct_hash_create( 43, &local->hash );
     if (ret) {
          D_DEBUG_ION( DFBION_Surfaces, "direct_hash_create fail!\n" );
          return ret;
     }
     D_DEBUG_ION( DFBION_Surfaces, "reactor = %x\n", sharedData->reactor );
     ret = fusion_reactor_attach(sharedData->reactor, ionhash_reaction_callback, local, &local->reaction);
     if (ret) {
          D_DEBUG_ION( DFBION_Surfaces,"fusion_reactor_attach fail!\n" );
          return ret;
     }

     D_MAGIC_SET( local, DFBIONPoolLocalData );

    return DFB_OK;
}

static DFBResult
DFBIONDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data,
                  void            *pool_local )
{
     DFBIONPoolSharedData      *sharedData  = pool_data;
     DFBIONPoolLocalData *local = pool_local;
     
     D_DEBUG_ION( DFBION_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBIONPoolSharedData );
     D_MAGIC_ASSERT( local, DFBIONPoolLocalData );

     fusion_call_destroy( &sharedData->call );
     
     fusion_reactor_free( sharedData->reactor );

     fusion_skirmish_destroy( &sharedData->lock );

     D_MAGIC_CLEAR( sharedData );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DFBIONLeavePool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     DFBIONPoolSharedData      *sharedData  = pool_data;
     DFBIONPoolLocalData *local = pool_local;

     D_DEBUG_ION( DFBION_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBIONPoolSharedData );
     D_MAGIC_ASSERT( local, DFBIONPoolLocalData );

     fusion_reactor_detach( sharedData->reactor, &local->reaction );
     direct_hash_destroy( local->hash );

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
DFBIONTestConfig( CoreSurfacePool         *pool,
                 void                    *pool_data,
                 void                    *pool_local,
                 CoreSurfaceBuffer       *buffer,
                 const CoreSurfaceConfig *config )
{
     D_DEBUG_ION( DFBION_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );
    
      return DFB_OK;
}

void DFBIONHeapNegotiate(DFBIONPoolLocalData  *local ,CoreSurfaceBuffer     *buffer, unsigned int   *pheap_mask, int *pflag )
{
    unsigned int heap_mask;
    int flag;
    CoreSurface          *surface;
    int layerid;
    DFBIONDataShared    *DFBIONShared = local->systemData->shared;

    surface = buffer->surface;

    D_DEBUG_ION( DFBION_Surfaces, "%s: layerid= %d\n", __FUNCTION__, layerid);
    /*for balance bandwidth, ion_heap_mask is determined by dfb config  */
    if (surface->type & CSTF_LAYER) 
    {
        layerid = surface->resource_id;
        switch(dfb_config->ion_heapmask_by_layer[layerid])
        {
            case CONF_ION_HEAP_SYSTEM_MASK:
                heap_mask = ION_HEAP_SYSTEM_MASK;
                break;

            case CONF_ION_HEAP_MIU0_MASK:
                heap_mask = 1<<ION_MALI_MIUO_HEAP_ID;
                break; 
            case CONF_ION_HEAP_MIU1_MASK:
                heap_mask = 1<<ION_MALI_MIU1_HEAP_ID;
                break; 

            case CONF_ION_HEAP_MIU2_MASK:
                heap_mask = 1<<ION_MALI_MIU2_HEAP_ID;
                break; 
        }

#ifdef DFB_ION_TLB
        //check gop support tlb
        int gop_index = dfb_config->mst_gop_available[surface->resource_id]; 
        if(DFBIONShared->GOP_enabled_tlb & 1<<gop_index) {
            flag = ION_FLAG_DISCRETE;  //gop support tlb
        } 
        else
#endif
        {
            flag = ION_FLAG_CONTIGUOUS;  //gop dont't support gop, usb continuous mem
        }
    } else {
        switch(dfb_config->ion_heapmask_by_surface)
        {
            case CONF_ION_HEAP_SYSTEM_MASK:
                heap_mask = ION_HEAP_SYSTEM_MASK;
                break;

            case CONF_ION_HEAP_MIU0_MASK:
                heap_mask = 1<<ION_MALI_MIUO_HEAP_ID;
                break; 
            case CONF_ION_HEAP_MIU1_MASK:
                heap_mask = 1<<ION_MALI_MIU1_HEAP_ID;
                break; 

            case CONF_ION_HEAP_MIU2_MASK:
                heap_mask = 1<<ION_MALI_MIU2_HEAP_ID;
                break; 
        }
#ifdef DFB_ION_TLB
        if(DFBIONShared->GE_support_tlb) 
        {
            flag = ION_FLAG_DISCRETE;  //support tlb
        } 
        else 
#endif
        {
            flag = ION_FLAG_CONTIGUOUS;  //can't support tlb 
        }
    }

    //temporary, not use discrect memory
    flag = ION_FLAG_CONTIGUOUS;

    *pheap_mask = heap_mask;
    *pflag = flag;
}


static DFBResult
DFBIONAllocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
    CoreSurface          *surface;
    DFBIONPoolSharedData       *sharedData  = pool_data;
    DFBIONPoolLocalData  *local = pool_local;
    DFBIONAllocationData *alloc = alloc_data;
    DFBIONData          *DFBION = local->systemData;
    DFBIONDataShared    *DFBIONShared = DFBION->shared;
    CoreGraphicsDevice *device;
    int pitch, length;
    unsigned int heap_mask;
    int flag;
    ion_alloc_data ion_alloc;
    int ret;
    
    D_DEBUG_ION( DFBION_Surfaces, "%s \n", __FUNCTION__);

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( sharedData, DFBIONPoolSharedData );
    D_MAGIC_ASSERT( local, DFBIONPoolLocalData );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    surface = buffer->surface;

    /* FIXME: Only one global device at the moment. */
    device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
    D_ASSERT( device != NULL );

    dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

    //negotiate the ion heap_mask and flag
    DFBIONHeapNegotiate(local, buffer, &heap_mask, &flag);

    ion_alloc.length = length;
    ion_alloc.heap_mask = heap_mask;
    ion_alloc.flag = flag;

    D_DEBUG_ION( DFBION_Surfaces, "ion_alloc: length=%d, heap_mask = %x, flag =%x\n", length, heap_mask, flag);
    if(surface->type & (CSTF_LAYER|CSTF_WINDOW)) {
        ion_alloc.initzero = true;
    } else {
        ion_alloc.initzero = false;
    }
    fusion_skirmish_prevail( &sharedData->lock );

    //call ion driver to allocate buffer
    _dfb_ion_alloc(local,&ion_alloc);

    
    fusion_skirmish_dismiss( &sharedData->lock );
    alloc->pitch = pitch;
    alloc->size  = length;

    /*because ion alloc/dealloc is transfered to master, so the ion_alloc represent the ion buffer information 
    in master process, other process can produce its own ion buffer information through these master information .*/
    alloc->ion_hnd = ion_alloc.handle;   
    alloc->shared_fd = ion_alloc.shared_fd;
    alloc->pid = ion_alloc.pid;  //master pid
    alloc->serial = ion_alloc.serial;
    alloc->phys = ion_alloc.phys;
    alloc->cpu_addr = ion_alloc.cpu_addr; //master cpu addr
    if(flag == ION_FLAG_DISCRETE) {
        alloc->addressType = E_ADDR_MTLB;
    } else {
        alloc->addressType = E_ADDR_PHY;
    }

    allocation->size = alloc->size;

    D_DEBUG_ION( DFBION_Surfaces, "  -> pitch %d, size %d£¨shared_fd %d, pid %d, serail = %d\n", pitch, length, alloc->shared_fd, alloc->pid, alloc->serial);

    D_MAGIC_SET( alloc, DFBIONAllocationData );

    return DFB_OK;
}

static DFBResult
DFBIONDeallocateBuffer( CoreSurfacePool       *pool,
                       void                  *pool_data,
                       void                  *pool_local,
                       CoreSurfaceBuffer     *buffer,
                       CoreSurfaceAllocation *allocation,
                       void                  *alloc_data )
{
     DFBIONPoolSharedData       *sharedData  = pool_data;
     DFBIONAllocationData *alloc = alloc_data;
     DFBIONPoolLocalData  *local = pool_local;
     ion_alloc_data ion_alloc;
     DFBResult             ret;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( sharedData, DFBIONPoolSharedData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, DFBIONAllocationData );

     D_DEBUG_ION( DFBION_Surfaces, "%s, pid %d,\n", __FUNCTION__, getpid() );

     ion_alloc.handle = alloc->ion_hnd;
     ion_alloc.shared_fd = alloc->shared_fd;
     ion_alloc.pid = alloc->pid;
     ion_alloc.serial = alloc->serial;
     ion_alloc.cpu_addr = alloc->cpu_addr;
     ion_alloc.length = alloc->size;

     ion_alloc.phys = alloc->phys;
     
     fusion_skirmish_prevail( &sharedData->lock );

     _dfb_ion_dealloc(local, &ion_alloc);

     fusion_skirmish_dismiss( &sharedData->lock );

     D_MAGIC_CLEAR( alloc );
     return DFB_OK;
}

int dfb_hash_get_lockdata(void          *pool_local, void *alloc_data, DFBIONLockData **lock_data)
{
    DFBIONPoolLocalData  *local = pool_local;
    DFBIONAllocationData *alloc = alloc_data;
    DFBIONData          *DFBION = local->systemData;
    DFBIONLockData *node;
    int newfd;
    ion_handle_t new_handle;
    void *cpu_addr;
    DFBMatrix matrix;
    
    matrix.xx = 0x10000;
    matrix.xy = 0x00000;
    matrix.yx = 0x00000;
    matrix.yy = 0x10000;
    node = direct_hash_lookup( local->hash, alloc->shared_fd, &matrix, alloc->serial);

    if(!node) {
        
      node = D_CALLOC( 1, sizeof(DFBIONLockData) );
      if (!node) {
           D_OOM();
           return -1;
      }
      /*in ion driver, every process has its own shared_fd and handle, these is no global data can be shared by 
      different processes. mstar provide a way to transfer shared_fd to another process which like android binder 
      does, so different processes can share ion buffer. ion_cust_import_fd function can achive this goal.*/

      //get the newfd in this process
       int ret = ion_cust_import_fd(DFBION->fd, alloc->pid, alloc->shared_fd, &newfd);
       if(ret < 0)
       {
           D_DEBUG_ION( DFBION_Surfaces, "%s( %d ): ion_cust_import_fd failed!\n", __FUNCTION__, __LINE__ );
           return -1;
       }
       D_DEBUG_ION( DFBION_Surfaces, "%s( %d ):  DFBION->fd = %d   newfd = %d \n", __FUNCTION__, __LINE__ ,DFBION->fd,newfd);
       //get the new_handle of this process
       ret = ion_import(DFBION->fd, newfd, &new_handle);
       if(ret < 0)
       {
           D_DEBUG_ION( DFBION_Surfaces, "%s( %d ): ion_import failed!\n", __FUNCTION__, __LINE__ );
           return -1;
       }

       //get the cpu_addr of this process
       cpu_addr = mmap(NULL, alloc->size, PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0);
        if (MAP_FAILED == cpu_addr)
        {
            D_DEBUG_ION( DFBION_Surfaces, " mmap fail , alloc->size = %d, shard_fd = %d\n", alloc->size, alloc->shared_fd);
            return -1;
        }
       node->shared_fd = newfd;
       node->ion_handle = new_handle;
       node->cpu_addr = cpu_addr;
       node->size = alloc->size;

        ret = direct_hash_insert(  local->hash, alloc->shared_fd,  node, &matrix, alloc->serial );
        if (ret) {
             D_FREE( node );
             return -1;
        }
    }

    *lock_data = node;
    
    return DFB_OK;
}


static DFBResult
DFBIONLock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
    DFBIONPoolSharedData       *sharedData  = pool_data;
    DFBIONPoolLocalData  *local = pool_local;
    DFBIONAllocationData *alloc = alloc_data;
    DFBIONData          *DFBION = local->systemData;

    ion_lock_data  ion_lockdata;
    int current_pid, shared_fd;
    DFBIONLockData *lock_data; //current process lock data
    void *cpu_addr;
    int ret;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DFBIONAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );
    lock->pitch = alloc->pitch;
    lock->offset = 0;

    fusion_skirmish_prevail( &sharedData->lock );
    current_pid = getpid();
    if(current_pid == alloc->pid) {
        //local hanle and cpu_addr
        ion_lockdata.handle = alloc->ion_hnd;
        shared_fd = alloc->shared_fd; 
        cpu_addr = alloc->cpu_addr;
        
        D_DEBUG_ION( DFBION_Surfaces,"DFBIONLock from same process, pid=%d, ion_hnd = %x, shard_fd = %d\n", current_pid, ion_lockdata.handle, shared_fd );
        
     } else {
     
        D_DEBUG_ION( DFBION_Surfaces,"DFBIONLock from different process\n");
        ret = dfb_hash_get_lockdata(local, alloc, &lock_data);
        if(ret < 0){
            D_DEBUG_ION( DFBION_Surfaces, "dfb_hash_get_lockdata failed!\n");
            
            fusion_skirmish_dismiss( &sharedData->lock );
            return -1;
        }
        
        //local hanle and cpu_addr
        ion_lockdata.handle = lock_data->ion_handle;
        shared_fd = lock_data->shared_fd;
        cpu_addr = lock_data->cpu_addr;
        
        D_DEBUG_ION( DFBION_Surfaces,"pid=%d, ion_hnd = %x, shared_fd =%d\n", current_pid, ion_lockdata.handle, shared_fd );
    }

    switch (lock->accessor) {
        case CSAID_CPU:
            lock->addr = cpu_addr;
            D_DEBUG_ION( DFBION_Surfaces, " cpu addr %x\n", lock->addr);
        /* fall through */

        case CSAID_LAYER0:
        case CSAID_GPU:
            lock->phys = alloc->phys;

            
#ifdef DFB_ION_TLB            
            switch(lock->miu)
            {
                case 0:
                    lock->tlb_table_addr = DFBION->shared->tlb_table_addr_miu0;
                    break;
                case 1:
                    lock->tlb_table_addr = DFBION->shared->tlb_table_addr_miu1;
                    break;
                case 2:
                    lock->tlb_table_addr = DFBION->shared->tlb_table_addr_miu2;
                    break;
                default:
                    D_DEBUG_ION( DFBION_Surfaces, "%s( %d ): unkown miu id!\n", __FUNCTION__, __LINE__ );
                    break;
            }

            /*for monaco, shoule load tlb table to GE    */
            if(ion_lockdata.tlb_table_change)
            {
                MApi_GFX_SetTLBFlushTable(true);
            }
#endif
            D_DEBUG_ION( DFBION_Surfaces, " addr_type %d, phys 0x%08lx, offset %d\n", lock->phys_type,  lock->phys, lock->offset);
         break;
         
        default:
            D_DEBUG_ION(DFBION_Surfaces, "unsupported accessor %d", lock->accessor);
            fusion_skirmish_dismiss( &sharedData->lock );
            return DFB_BUG;
        }

    fusion_skirmish_dismiss( &sharedData->lock );
    return DFB_OK;
}

static DFBResult
DFBIONUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             void                  *pool_local,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
    DFBIONPoolSharedData       *sharedData  = pool_data;
    DFBIONPoolLocalData  *local = pool_local;
    DFBIONAllocationData *alloc = alloc_data;
    DFBIONData          *DFBION = local->systemData;
    ion_lock_data  ion_lockdata;
    int newfd, current_pid, shared_fd;
    ion_handle_t new_handle;
    DFBIONLockData *lock_data; //current process lock data
    int ret;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, DFBIONAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    fusion_skirmish_prevail( &sharedData->lock );

    current_pid = getpid();
    if(current_pid == alloc->pid) {
        ion_lockdata.handle = alloc->ion_hnd;
        
        D_DEBUG_ION( DFBION_Surfaces,"DFBIONUnlock from same process, pid=%d, ion_hnd = %x\n", current_pid, ion_lockdata.handle );
     } else {
        ret = dfb_hash_get_lockdata(local, alloc, &lock_data);
        if(ret < 0){
            D_DEBUG_ION( DFBION_Surfaces, "dfb_hash_get_lockdata failed!\n");
            fusion_skirmish_dismiss( &sharedData->lock );
            return -1;
        }
        
        //local hanle and cpu_addr
        ion_lockdata.handle = lock_data->ion_handle;
        shared_fd = lock_data->shared_fd;
        D_DEBUG_ION( DFBION_Surfaces,"DFBIONUnlock from different process, pid=%d, ion_hnd = %x, newfd =%d\n", current_pid, ion_lockdata.handle, shared_fd );
    }

        fusion_skirmish_dismiss( &sharedData->lock );
    return DFB_OK;
}



const SurfacePoolFuncs DFBIONSurfacePoolFuncs = {
     .PoolDataSize       = DFBIONPoolDataSize,
     .PoolLocalDataSize  = DFBIONPoolLocalDataSize,
     .AllocationDataSize = DFBIONAllocationDataSize,

     .InitPool           = DFBIONInitPool,
     .JoinPool           = DFBIONJoinPool,
     .DestroyPool        = DFBIONDestroyPool,
     .LeavePool          = DFBIONLeavePool,

     .TestConfig         = DFBIONTestConfig,
     .AllocateBuffer     = DFBIONAllocateBuffer,
     .DeallocateBuffer   = DFBIONDeallocateBuffer,

     .Lock               = DFBIONLock,
     .Unlock             = DFBIONUnlock,
};

