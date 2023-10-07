/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <asm/types.h>
#include <sys/ioctl.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/surface_pool.h>
#include <core/core.h>

#include <gfx/convert.h>

#include "mt53.h"
#include "surfacemanager.h"
#ifdef CC_B2R44K2K_SUPPORT
#include <fusion/shmalloc.h>
#endif


D_DEBUG_DOMAIN( MT53_Surfaces, "MT53/Surfaces", "MT53 Framebuffer Surface Pool" );

extern void dfb_print_trace(void);

/**********************************************************************************************************************/

typedef struct 
{
    int            magic;
    SurfaceManager *manager;
    SurfaceManager *manager_fbm;
    SurfaceManager *manager_3dmm;
    SurfaceManager *manager_vdp;
    SurfaceManager *manager_crsr;
} MT53PoolData;

typedef struct {
     int             magic;

     CoreDFB        *core;
} MT53PoolLocalData;

typedef enum 
{
     VIDO_MEM_DFB   = 0x1,
     VIDO_MEM_FBM   = 0x2,
     VIDO_MEM_3DMM  = 0x4,
     VIDO_MEM_VDP   = 0x8,
     VIDO_MEM_CRSR  = 0x10,
#ifdef CC_B2R44K2K_SUPPORT     
     VIDO_MEM_B2R44K2K = 0x20,
#endif          
     VIDO_MEM_MAX   = 0xFF,
}E_VIDO_MEM;


/**********************************************************************************************************************/

static int
mt53PoolDataSize()
{
     return sizeof(MT53PoolData);
}

static int
mt53PoolLocalDataSize()
{
     return sizeof(MT53PoolLocalData);
}

static int
mt53AllocationDataSize()
{
     return sizeof(MT53AllocationData);
}

static DFBResult
mt53InitPool( CoreDFB                    *core,
              CoreSurfacePool            *pool,
              void                       *pool_data,
              void                       *pool_local,
              void                       *system_data,
              CoreSurfacePoolDescription *ret_desc )
{
     DFBResult           ret;
     MT53PoolData      *data  = pool_data;
     MT53PoolLocalData *local = pool_local;
     int               i4_acces = 0x0;
#ifdef CC_B2R44K2K_SUPPORT
     int i;
#endif     
     D_DEBUG_AT( MT53_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( data != NULL );
     D_ASSERT( local != NULL );
     D_ASSERT( ret_desc != NULL );

     ret = dfb_surfacemanager_create( core, dfb_mt53->fix.smem_len, &data->manager );
     if (ret)
          return ret;
     
     ret = dfb_surfacemanager_create( core, dfb_mt53->t_crsr.u4Size, &data->manager_crsr);
     if (ret)
          return ret;

#ifdef CC_OSD_USE_FBM
     ret = dfb_surfacemanager_create( core, dfb_mt53->imagebuf.u4Size, &data->manager_fbm );
     if (ret)
          return ret;
#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB  
     ret = dfb_surfacemanager_create( core, dfb_mt53->imagebuf2.u4Size, &data->manager_3dmm );
     if (ret)
          return ret;     
#endif
#ifdef CC_DFB_SUPPORT_VDP_LAYER
    ret = dfb_surfacemanager_vdp_create( core, dfb_mt53->vdpbuf.u4Size, &data->manager_vdp);
    if (ret)
    {
        return ret; 
    }
#endif
#endif

    /* MT53 surface pool don't provide IOVA address */
    ret_desc->caps = CSPCAPS_ALL &(~CSPCAPS_IOVA);

    for(i4_acces = CSAID_NONE;i4_acces<_CSAID_NUM;i4_acces++)
    {
        ret_desc->access[i4_acces] = CSAF_ALL;
    }

    if(DFB_SYSTEMONLY)
    {
        ret_desc->scaps    = DSCAPS_ALL&(~(DSCAPS_SYSTEMONLY|DSCAPS_IOMMU));
        ret_desc->types    = CSTF_ALL&(~(CSTF_FONT));
        ret_desc->priority = CSPP_ULTIMATE;
    }
    else
    {
        ret_desc->scaps   = DSCAPS_ALL;
        ret_desc->types    = CSTF_ALL;
        ret_desc->priority =  CSPP_DEFAULT;
    }

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "mt53fb Memory" );

     local->core = core;

     D_MAGIC_SET( data, MT53PoolData );
     D_MAGIC_SET( local, MT53PoolLocalData );


     D_ASSERT( dfb_mt53 != NULL );
     D_ASSERT( dfb_mt53->shared != NULL );

     dfb_mt53->shared->manager      = data->manager;
     dfb_mt53->shared->manager_crsr = data->manager_crsr;
     
    #ifdef CC_OSD_USE_FBM
     dfb_mt53->shared->manager_fbm = data->manager_fbm;
    #ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB  
     dfb_mt53->shared->manager_3dmm = data->manager_3dmm;
    #endif
    #ifdef CC_DFB_SUPPORT_VDP_LAYER
    dfb_mt53->shared->manager_vdp = data->manager_vdp;
    #endif
    #endif

#ifdef CC_B2R44K2K_SUPPORT
     dfb_mt53->shared->m_B2R44K2KFlags = 0x00;
     dfb_mt53->shared->m_B2R44K2KFlags = B2R_BUFFER_MASK;
#endif  

     return DFB_OK;
}

static DFBResult
mt53JoinPool( CoreDFB         *core,
              CoreSurfacePool *pool,
              void            *pool_data,
              void            *pool_local,
              void            *system_data )
{
     MT53PoolData      *data  = pool_data;
     MT53PoolLocalData *local = pool_local;

     D_DEBUG_AT( MT53_Surfaces, "%s()\n", __FUNCTION__ );

     D_ASSERT( core != NULL );
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, MT53PoolData );
     D_ASSERT( local != NULL );

     local->core = core;

     D_ASSERT( dfb_mt53 != NULL );
     D_ASSERT( dfb_mt53->shared != NULL );
     D_ASSERT( dfb_mt53->shared->manager != NULL );

     data->manager = dfb_mt53->shared->manager;
     
     dfb_mt53->shared->manager_crsr = data->manager_crsr;

#ifdef CC_OSD_USE_FBM
     dfb_mt53->shared->manager_fbm = data->manager_fbm;
#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB  
     dfb_mt53->shared->manager_3dmm = data->manager_3dmm;
#endif

#ifdef CC_DFB_SUPPORT_VDP_LAYER
    dfb_mt53->shared->manager_vdp = data->manager_vdp;
#endif

#endif

     D_MAGIC_SET( local, MT53PoolLocalData );

     return DFB_OK;
}

static DFBResult
mt53DestroyPool( CoreSurfacePool *pool,
                 void            *pool_data,
                 void            *pool_local )
{
     MT53PoolData      *data  = pool_data;
     MT53PoolLocalData *local = pool_local;

     D_DEBUG_AT( MT53_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, MT53PoolData );
     D_MAGIC_ASSERT( local, MT53PoolLocalData );
     
     mt53EngineSync(NULL,NULL);
     
     dfb_surfacemanager_destroy( data->manager );
     dfb_surfacemanager_destroy( data->manager_crsr );

#ifdef CC_OSD_USE_FBM
     dfb_surfacemanager_destroy( data->manager_fbm );
#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB  
     dfb_surfacemanager_destroy( data->manager_3dmm );
#endif

#ifdef CC_DFB_SUPPORT_VDP_LAYER
    dfb_surfacemanager_destroy( data->manager_vdp );
#endif

#endif

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
mt53LeavePool( CoreSurfacePool *pool,
               void            *pool_data,
               void            *pool_local )
{
     MT53PoolData      *data  = pool_data;
     MT53PoolLocalData *local = pool_local;

     D_DEBUG_AT( MT53_Surfaces, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, MT53PoolData );
     D_MAGIC_ASSERT( local, MT53PoolLocalData );

     mt53EngineSync(NULL,NULL);
      
     (void) data;

     D_MAGIC_CLEAR( local );

     return DFB_OK;
}

static DFBResult
mt53TestConfig( CoreSurfacePool         *pool,
                void                    *pool_data,
                void                    *pool_local,
                CoreSurfaceBuffer       *buffer,
                const CoreSurfaceConfig *config )
{
    DFBResult           ret;
    CoreSurface        *surface;
    MT53PoolData      *data  = pool_data;
    MT53PoolLocalData *local = pool_local;

    D_DEBUG_AT( MT53_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( data, MT53PoolData );
    D_MAGIC_ASSERT( local, MT53PoolLocalData );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    surface = buffer->surface;
    D_MAGIC_ASSERT( surface, CoreSurface );

#ifdef CC_B2R44K2K_SUPPORT
    if(surface->config.caps & DSCAPS_FROM_MPEG)
    {
        D_INFO("mt53TestConfig DSCAPS_FROM_MPEG \n");
        MT53Shared  *shared = dfb_mt53->shared;

        if(shared->m_B2R44K2KFlags & (B2R_BUFFER_MASK))
        {
            D_INFO("mt53TestConfig DSCAPS_FROM_MPEG DR_OK \n");
            ret = DR_OK;
        }
        else
        {
            D_INFO("mt53TestConfig DSCAPS_FROM_MPEG  DFB_NOVIDEOMEMORY\n");
            ret = DFB_NOVIDEOMEMORY;
        }
         
        return ret;
    }
#endif  

    if(surface->config.caps & DSCAPS_FROM_FBM) // From FBM
    {
        ret = dfb_surfacemanager_allocate( local->core, data->manager_fbm, buffer, NULL,(unsigned int)DSCAPS_FROM_FBM); 
    } 
    else if(surface->config.caps & DSCAPS_FROM_JPEG_VDP) // From VDP
    {
        ret = dfb_surfacemanager_vdp_allocate( local->core, data->manager_vdp, buffer, NULL);
    }   
    else if(surface->config.caps & DSCAPS_FROM_3DMM) // From 3DMM
    {
        ret = dfb_surfacemanager_allocate( local->core, data->manager_3dmm, buffer, NULL, (unsigned int)DSCAPS_FROM_3DMM);
    }
    else if(surface->type & CSTF_CURSOR)
    {
        ret = dfb_surfacemanager_allocate( local->core, data->manager_crsr, buffer, NULL, (unsigned int)DSCAPS_FROM_CRSR);
    }
    else
    {
        ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, NULL,(unsigned int)DSCAPS_VIDEOONLY);  
    }

    D_DEBUG_AT( MT53_Surfaces, "  -> %s\n", DirectFBErrorString(ret) );

    return ret;
}

#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB
extern int DirectFBDumpPrintOnly(void);
#endif

void mt53_fill_rect(unsigned int u4_addr,unsigned int u4_pitch,unsigned int u4_h, int fgPhyEn)
{
    struct mt53fb_fillrect rRect;

    D_DEBUG_AT(MT53_Surfaces, "%s,%d Fill rect to clear\n", __func__, __LINE__);
    
    mt53_gfx_set_mmu(&rRect.mmu, fgPhyEn);

    rRect.u2DstX     = 0;
    rRect.u2DstY     = 0; 
    rRect.i4Bpp      = CM_ARGB8888_DIRECT32;      
    rRect.i4Color    = 0x0;
    rRect.u2Width    = (u4_pitch/4);
    rRect.u2Height   = u4_h;
    rRect.u2DstPitch = u4_pitch;
    rRect.pu1DstBaseAddr = (unsigned char *)u4_addr;
    
    if(ioctl( dfb_mt53->fd, FBIO_RECTFILL, &rRect ) < 0)
    {
        D_ERROR("%s[pid=%d][0x%x,0x%x,0x%x]\n",
                            __FUNCTION__,getpid(),u4_addr,u4_pitch,u4_h);
    }   

    return;
}

static int mt53_check_mmu_pin_mem(MT53AllocationData *alloc)
{
    E_DFB_SSP e_sh = E_DSSP_SH;
    
    if(!alloc || (0x0 == alloc->pin_range[E_DSSP_ALLOC].start))
    {
        D_ERROR("%s[pid = %d] Fail\n",__FUNCTION__,getpid());
        return -1;
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
        return -1;
    }

    alloc->pid[e_sh] = getpid();
	#ifdef CC_OSD_M4U_IOVA_SUPPORT
    alloc->pin_range[e_sh].start = (unsigned long)(dfb_mt53->mem + (alloc->u4_phy - dfb_mt53->m4u_fix.mmio_start));
	#else
    alloc->pin_range[e_sh].start = (unsigned long)(dfb_mt53->mem + (alloc->u4_phy - dfb_mt53->fix.smem_start));
    #endif
    alloc->pin_range[e_sh].size  = alloc->pin_range[E_DSSP_ALLOC].size;

    if ( mlock((void*)(UPTR)alloc->pin_range[e_sh].start,alloc->pin_range[e_sh].size) != 0 )
    {
        D_PERROR( "%s:mlock failed! [pid = %d][%llx, %lx]\n", __func__, getpid(), alloc->pin_range[e_sh].start,         alloc->pin_range[e_sh].start );
        return -1;
    }
    
    if ( system_mmu_pin_iomem(&alloc->pin_range[e_sh]) != 0 )
    {
        D_PERROR( "%s:system_mmu_pin_iomem failed! [pid = %d][%llx, %lx]\n", __func__, getpid(),
                   alloc->pin_range[e_sh] .start, alloc->pin_range[e_sh].start );
        return -1;
    }

    return 0;
}

static void mt53_check_mmu_unpin_mem(MT53AllocationData *alloc)
{
    E_DFB_SSP e_sh = E_DSSP_SH;
    
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
    //for iomap memory, need not to unpin memory
    alloc->pin_range[e_sh].start    = 0x0;
    alloc->pin_range[e_sh].size     = 0x0;
    alloc->pid[e_sh]                = 0x0;
    
    return ;
}


static DFBResult
mt53AllocateBuffer( CoreSurfacePool       *pool,
                    void                  *pool_data,
                    void                  *pool_local,
                    CoreSurfaceBuffer     *buffer,
                    CoreSurfaceAllocation *allocation,
                    void                  *alloc_data )
{
     DFBResult           ret;
     Chunk              *chunk;
     CoreSurface        *surface;
     MT53PoolData       *data  = pool_data;
     MT53PoolLocalData  *local = pool_local;
     MT53AllocationData *alloc = alloc_data;
#ifdef CC_B2R44K2K_SUPPORT   
     int i;
#endif
     D_DEBUG_AT( MT53_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( data, MT53PoolData );
     D_MAGIC_ASSERT( local, MT53PoolLocalData );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );
     
#ifdef CC_B2R44K2K_SUPPORT
     if(surface->config.caps & DSCAPS_FROM_MPEG)
     {
          MT53Shared    *shared = dfb_mt53->shared;

          alloc->u4_vdo_mem_type = DSCAPS_FROM_MPEG;  

          if(!dfb_mt53->m_B2RDfbBuf[0].u4VirtAddr)
          {
              system_mmap_b2r44K2Kbuf_addr();
          }
          
          ret = DFB_NOVIDEOMEMORY;

          for(i = 0; i < B2R_BUFFER_NUM; i++)
          {
               if(1 == (shared->m_B2R44K2KFlags >> i))
               {
                    D_DEBUG_AT(MT53_Surfaces, "mt53AllocateBuffer[pid=%d][0x%x] [%d]1\n",getpid(),shared->m_B2R44K2KFlags,chunk->m_Index);
                    shared->m_B2R44K2KFlags &= ~ (1 << i); 
                    CoreGraphicsDevice *device = dfb_core_get_part( local->core, DFCP_GRAPHICS );
                    D_ASSERT( device != NULL );                                   
                    chunk = (Chunk*) SHCALLOC( dfb_core_shmpool( local->core ), 1, sizeof(Chunk) );
                    D_MAGIC_SET( chunk, Chunk );
                    chunk->offset = 0;
                    dfb_gfxcard_calc_buffer_size( device, buffer, &chunk->pitch, &chunk->length,DSCAPS_FROM_MPEG );
                    chunk->buffer = buffer;
                    chunk->m_Index = i;
                    ret = DR_OK;
                    D_DEBUG_AT(MT53_Surfaces, "mt53AllocateBuffer[pid=%d][0x%x] [%d]\n",getpid(),shared->m_B2R44K2KFlags,chunk->m_Index);
                    
                    break;
               }
           }
     }
     else
#endif   
    if(surface->config.caps & DSCAPS_FROM_FBM) // From FBM
    {
        alloc->u4_vdo_mem_type = DSCAPS_FROM_FBM;  
        if(!dfb_mt53->imagebuf.u4VirtAddr)
        {
           system_mmap_imagebuf_addr();
        }
        
        ret = dfb_surfacemanager_allocate( local->core, data->manager_fbm, buffer, &chunk,alloc->u4_vdo_mem_type);
    }
    else if(surface->config.caps & DSCAPS_FROM_3DMM) // From FBM 3DMM
    {
        alloc->u4_vdo_mem_type = DSCAPS_FROM_3DMM;
        if(!dfb_mt53->imagebuf2.u4VirtAddr)
        {
           system_mmap_imagebuf2_addr();
        }
        
        ret = dfb_surfacemanager_allocate( local->core, data->manager_3dmm, buffer, &chunk,alloc->u4_vdo_mem_type);
    }
    else if(surface->type & CSTF_CURSOR) 
    {
        alloc->u4_vdo_mem_type = DSCAPS_FROM_CRSR;
        ret = dfb_surfacemanager_allocate( local->core, data->manager_crsr, buffer, &chunk,alloc->u4_vdo_mem_type);
    }
    else if(surface->config.caps & DSCAPS_FROM_JPEG_VDP) // From VDP
    {
        alloc->u4_vdo_mem_type = DSCAPS_FROM_JPEG_VDP;
        if(!dfb_mt53->vdpbuf.u4VirtAddr)
        {
            system_mmap_vdpbuf_addr();
        }
        
        ret = dfb_surfacemanager_vdp_allocate( local->core, data->manager_vdp, buffer, &chunk);
    }  
    else
    {
        alloc->u4_vdo_mem_type = DSCAPS_VIDEOONLY;
        ret = dfb_surfacemanager_allocate( local->core, data->manager, buffer, &chunk, alloc->u4_vdo_mem_type); 
    }

    if (dfb_config_get_dump_stack(DDDS_SURFACE_ALLOC))
    {
        D_INFO("%s,%d [pid = %d] [buffer: 0x%x] [caps: 0x%x] [type: 0x%x] [size: %d]\n",
                            __FUNCTION__,
                            __LINE__,
                            getpid(),
                            (int)buffer,
                            surface->config.caps,
                            surface->type,
                            chunk->length);
        dfb_print_trace();
    }

    if(ret)
    {
        direct_log_printf(NULL,"%s[pid = %d][0x%x][0x%x][0x%x] ret[0x%x]\n",
                            __FUNCTION__,
                            getpid(),
                            (int)buffer,
                            surface->config.caps,
                            surface->type,
                            ret);
        dfb_print_trace();
        return ret;
    }

    D_MAGIC_ASSERT( chunk, Chunk );
    alloc->offset = chunk->offset;
    alloc->pitch  = chunk->pitch;
    alloc->size   = chunk->length;
    alloc->chunk  = chunk;

    D_DEBUG_AT( MT53_Surfaces, "  -> offset %d, pitch %d, size %d\n", alloc->offset, alloc->pitch, alloc->size );

    allocation->size   = alloc->size;
    allocation->offset = alloc->offset;

    if(alloc->u4_vdo_mem_type&DSCAPS_FROM_FBM) 
    {
       alloc->u4_addr   = (void*)dfb_mt53->imagebuf.u4VirtAddr + alloc->offset;
       alloc->u4_phy    = dfb_mt53->imagebuf.u4PhyAddr + alloc->offset;    
    }
    else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_3DMM) 
    {
       alloc->u4_addr   = (void*)dfb_mt53->imagebuf2.u4VirtAddr + alloc->offset;
       alloc->u4_phy    = dfb_mt53->imagebuf2.u4PhyAddr + alloc->offset;    
    }
    else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_CRSR) 
    {
       alloc->u4_addr   = (void*)dfb_mt53->t_crsr.u4VirtAddr + alloc->offset;
       alloc->u4_phy    = dfb_mt53->t_crsr.u4PhyAddr + alloc->offset;    
    }
    else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_JPEG_VDP)
    {
       alloc->u4_addr   = (void*)dfb_mt53->vdpbuf.u4VirtAddr + alloc->offset;
       alloc->u4_phy    = dfb_mt53->vdpbuf.u4PhyAddr + alloc->offset;     
    }
    else
    {
        alloc->u4_addr   = dfb_mt53->mem + alloc->offset;
		#ifdef CC_OSD_M4U_IOVA_SUPPORT
		alloc->u4_phy    = dfb_mt53->m4u_fix.mmio_start + alloc->offset;
		#else
        alloc->u4_phy    = dfb_mt53->fix.smem_start + alloc->offset;
		#endif
    }

    D_DEBUG_AT( MT53_Surfaces, "%s[pid = %d][0x%x][0x%x,0x%x][0x%x][0x%x][0x%x,0x%x][0x%x]\n",
                                __FUNCTION__,
                                getpid(),
                                (int)allocation,
                                surface->config.caps,
                                alloc->u4_vdo_mem_type,
                                surface->type,
                                alloc->offset,
                                alloc->u4_addr,
                                alloc->u4_phy,
                                alloc->size );
    
#ifdef CC_GFX_MMU
    {
    alloc->pin_range[E_DSSP_ALLOC].start = (unsigned long)alloc->u4_addr;
    alloc->pin_range[E_DSSP_ALLOC].size  = alloc->size;
    alloc->pid[E_DSSP_ALLOC] = getpid();
    
    ret = mlock(alloc->u4_addr,alloc->size);
    if(ret != 0)
    {
        D_PERROR( "%s:mlock fail[0x%p, 0x%p]\n", __func__, alloc->u4_addr, alloc->size );
        return DFB_FAILURE;
    }
    ret = system_mmu_pin_iomem(&alloc->pin_range[E_DSSP_ALLOC]);
    if ( ret != 0 )
    {
        D_ERROR( "system_mmu_pin_mem failed!\n");
        return DFB_FAILURE;
    }
    }
#endif
#ifdef DFB_IOMMU_PRECHECK
    CalcPhysicalAddress((unsigned int)alloc->u4_addr, 0);
#endif
    if(surface->type  & (SURFACE_TYPE_VIDEO |CSTF_WINDOW) )
    {
#ifdef GFX_IOMMU_IOVA_SUPPORT
        memset((void *)alloc->u4_addr,0, alloc->pitch*surface->config.size.h);//fix me, use hw
        mt53FlushInvalidCache( (unsigned int)alloc->u4_addr, alloc->size );
#else

        if(DFB_SYSTEMONLY)
        {
            mt53_fill_rect((unsigned int)alloc->u4_addr,alloc->pitch,surface->config.size.h, 0);
        }
        else
        {
            mt53_fill_rect(alloc->u4_phy,alloc->pitch,surface->config.size.h, 1);
        }
#endif
    }
    else
    {
       mt53FlushInvalidCache( (unsigned int)alloc->u4_addr, alloc->size );
    }

    D_MAGIC_SET( alloc, MT53AllocationData );

    return DFB_OK;
}

static DFBResult
mt53DeallocateBuffer( CoreSurfacePool       *pool,
                      void                  *pool_data,
                      void                  *pool_local,
                      CoreSurfaceBuffer     *buffer,
                      CoreSurfaceAllocation *allocation,
                      void                  *alloc_data )
{
    MT53PoolData       *data  = pool_data;
    MT53AllocationData *alloc = alloc_data;

    D_DEBUG_AT( MT53_Surfaces, "%s( %p )\n", __FUNCTION__, buffer );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( data, MT53PoolData );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
    D_MAGIC_ASSERT( alloc, MT53AllocationData );

    if(buffer->surface->type&SURFACE_TYPE_VIDEO)
    {
          mt53_fill_rect(alloc->u4_phy,alloc->pitch,buffer->surface->config.size.h, 1);
    }  
    
    if(alloc->chunk)
    {   
        #ifdef CC_B2R44K2K_SUPPORT
        if(alloc->u4_vdo_mem_type == DSCAPS_FROM_MPEG)
        {
            MT53Shared  *shared = dfb_mt53->shared;
            MT53PoolLocalData  *local = pool_local;

            D_DEBUG_AT(MT53_Surfaces, "%s[%d][0x%x][%d]1\n",__FUNCTION__,getpid(),shared->m_B2R44K2KFlags,alloc->chunk->m_Index);

            shared->m_B2R44K2KFlags |= (1 << alloc->chunk->m_Index);
            D_MAGIC_CLEAR( alloc->chunk );

            SHFREE( dfb_core_shmpool( local->core ), alloc->chunk );

            D_DEBUG_AT( MT53_Surfaces, "%s[%d][0x%x][%d]\n",__FUNCTION__,getpid(),shared->m_B2R44K2KFlags,alloc->chunk->m_Index);
                
            if((shared->m_B2R44K2KFlags&0xf)==0xf)
            {
                system_munmap_b2r44K2Kbuf_addr();
            }
        }
        else
        #endif
        if(alloc->u4_vdo_mem_type&DSCAPS_FROM_FBM) 
        {
            dfb_surfacemanager_deallocate( data->manager_fbm, alloc->chunk);

            if(data->manager_fbm->avail>= data->manager_fbm->length)
            {
                system_munmap_imagebuf_addr();
            }
        }
        else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_3DMM)
        {
            dfb_surfacemanager_deallocate( data->manager_3dmm, alloc->chunk);

            if(data->manager_3dmm->avail>= (data->manager_3dmm->length- (1024*1024)))
            {
                system_munmap_imagebuf2_addr();
            }
        }
        else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_JPEG_VDP)
        {
            dfb_surfacemanager_deallocate( data->manager_vdp, alloc->chunk);

            if(data->manager_vdp->avail >= (data->manager_vdp->length))
            {
                system_munmap_vdpbuf_addr();
            }
        }
        else if(alloc->u4_vdo_mem_type&DSCAPS_FROM_CRSR)
        {
            dfb_surfacemanager_deallocate( data->manager_crsr, alloc->chunk);
        }
        else
        {
            dfb_surfacemanager_deallocate( data->manager, alloc->chunk);
        }
    }

    D_DEBUG_AT( MT53_Surfaces, "%s [pid=%d][0x%x,0x%x,0x%x] \n",__FUNCTION__,getpid(),(int)allocation,allocation->size,(int)buffer );

    D_MAGIC_CLEAR( alloc );

    return DFB_OK;
}

static DFBResult
mt53Lock( CoreSurfacePool       *pool,
          void                  *pool_data,
          void                  *pool_local,
          CoreSurfaceAllocation *allocation,
          void                  *alloc_data,
          CoreSurfaceBufferLock *lock )
{
    MT53AllocationData *alloc = alloc_data;

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, MT53AllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_AT( MT53_Surfaces, "%s( %p )\n", __FUNCTION__, lock->buffer );

#ifdef CC_B2R44K2K_SUPPORT
    if(alloc->u4_vdo_mem_type == DSCAPS_FROM_MPEG)
    {
        lock->pitch  = alloc->pitch;
        lock->offset = alloc->offset;   
        lock->addr   = (void*)dfb_mt53->m_B2RDfbBuf[alloc->chunk->m_Index].u4VirtAddr + alloc->offset;
        lock->phys   = dfb_mt53->m_B2RDfbBuf[alloc->chunk->m_Index].u4PhyAddr + alloc->offset;

        return DFB_OK;
    }
#endif

    lock->pitch  = alloc->pitch;
    lock->offset = alloc->offset;
    lock->addr   = (void *)alloc->u4_addr;
    lock->phys   = alloc->u4_phy;
    lock->size   = alloc->size;

    if (lock->accessor == CSAID_DMA || lock->accessor == CSAID_GPU)
    {
        if (alloc->fd <= 0)
            alloc->fd = system_mem_get_dmafd(alloc->u4_phy, alloc->size);

        lock->dma_fd = alloc->fd;
    }


    D_DEBUG_AT( MT53_Surfaces, "%s,%d: [pid = %d][accessor = 0x%x][addr=0x%x, phy=0x%x, size=0x%x dma_fd = %d] \n",
                __FUNCTION__, __LINE__, getpid(), lock->accessor == CSAID_DMA, lock->addr, lock->phys, alloc->size, alloc->fd);
    #ifdef CC_GFX_MMU
     if(alloc->pid[E_DSSP_ALLOC] != getpid())
     {
     	#ifdef CC_OSD_M4U_IOVA_SUPPORT		
        lock->addr = (void *)dfb_mt53->mem + (alloc->u4_phy - dfb_mt53->m4u_fix.mmio_start);
		#else
        lock->addr = (void *)dfb_mt53->mem + (alloc->u4_phy - dfb_mt53->fix.smem_start);
		#endif
        
        if ( mt53_check_mmu_pin_mem(alloc) != 0)
            return DFB_FAILURE;
     }
    #endif

    return DFB_OK;
}

static DFBResult
mt53Unlock( CoreSurfacePool       *pool,
            void                  *pool_data,
            void                  *pool_local,
            CoreSurfaceAllocation *allocation,
            void                  *alloc_data,
            CoreSurfaceBufferLock *lock )
{
     MT53AllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, MT53AllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_DEBUG_AT( MT53_Surfaces, "%s( %p )\n", __FUNCTION__, lock->buffer );
#ifdef CC_GFX_MMU
 if(alloc->pid[E_DSSP_ALLOC] != getpid())
    {
        mt53_check_mmu_unpin_mem(alloc);
    }
#endif

     (void) alloc;

     return DFB_OK;
}

/**********************************************************************************************************************/

const SurfacePoolFuncs mt53SurfacePoolFuncs = {
     PoolDataSize:       mt53PoolDataSize,
     PoolLocalDataSize:  mt53PoolLocalDataSize,
     AllocationDataSize: mt53AllocationDataSize,

     InitPool:           mt53InitPool,
     JoinPool:           mt53JoinPool,
     DestroyPool:        mt53DestroyPool,
     LeavePool:          mt53LeavePool,

     TestConfig:         mt53TestConfig,
     AllocateBuffer:     mt53AllocateBuffer,
     DeallocateBuffer:   mt53DeallocateBuffer,

     Lock:               mt53Lock,
     Unlock:             mt53Unlock
};

