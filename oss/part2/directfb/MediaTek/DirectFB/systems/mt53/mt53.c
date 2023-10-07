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

#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <asm/types.h>

#include <directfb.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/system.h>
#include <direct/util.h>

#include <fusion/arena.h>
#include <fusion/fusion.h>
#include <fusion/reactor.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/layers.h>
#include <core/gfxcard.h>
#include <core/screens.h>
#include <core/surface_pool.h>

#include <misc/conf.h>
#include <misc/util.h>

#include "mt53.h"
#include <direct/debug.h>
#include <direct/filesystem.h>
#include <direct/util.h>


#include <core/core_system.h>

DFB_CORE_SYSTEM( mt53sys )
D_DEBUG_DOMAIN(MT53_System, "MT53/System", "MT53 System Interface");


MT53 *dfb_mt53 = NULL;

#ifdef CC_OSD_M4U_IOVA_SUPPORT
static MT53AllocationData allocm4u;
#endif

int dfb_fb = 0;

#ifdef CC_B2R44K2K_SUPPORT
int _B2RBufNum = 0;
unsigned int _B2RBufPhyAddr[10];
unsigned int _B2RBufVirAddr[10];
unsigned int _B2RBufCPhyAddr[10];
#endif
/**********************************************************************************************************************/


void system_mmap_dfb_addr(void)
{
    D_DEBUG_AT(MT53_System, "   %s,%d\n", __func__, __LINE__);
    
    if(ioctl( dfb_mt53->fd, FBIOGET_FSCREENINFO, &dfb_mt53->fix ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIOGET_FSCREENINFO failed!\n" );
        return ;
    }
    dfb_mt53->mem = mmap( (void*)NULL,
                                dfb_mt53->fix.smem_len,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED ,
                                dfb_mt53->fd,
                                0x0);

    if(dfb_mt53->mem == MAP_FAILED)
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the DFB  buffer!\n");
    }
    else
    {
        //memset(dfb_mt53->mem,0x0,dfb_mt53->fix.smem_len);
        D_INFO( "DirectFB/MT53: DFB[vir=0x%x,phy=0x%x,size=0x%x]\n",
            (int)dfb_mt53->mem,dfb_mt53->fix.smem_start,dfb_mt53->fix.smem_len);
    }

    return;
}

void system_mmap_crsr_addr(void)
{
    D_DEBUG_AT(MT53_System, "   %s,%d   -> Phy addr: 0x%x\n", __func__, __LINE__, dfb_mt53->t_crsr.u4PhyAddr);

    if(ioctl( dfb_mt53->fd, FBIO_CRSR_INFO, &dfb_mt53->t_crsr ) < 0)
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
        return;
    }
    dfb_mt53->t_crsr.u4VirtAddr = (unsigned int)mmap(
                         (void*)NULL,
                         dfb_mt53->t_crsr.u4Size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED,
                         dfb_mt53->fd,
                         dfb_mt53->t_crsr.u4PhyAddr);

    if(dfb_mt53->t_crsr.u4VirtAddr == (unsigned int)MAP_FAILED)
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the t_crsr  buffer!\n");
    }
    else
    {
        //memset(dfb_mt53->t_crsr.u4VirtAddr,0x0,dfb_mt53->t_crsr.u4Size);
        D_INFO( "DirectFB/MT53: t_crsr[vir=0x%x,phy=0x%x,size=0x%x]\n",
        dfb_mt53->t_crsr.u4VirtAddr,dfb_mt53->t_crsr.u4PhyAddr,dfb_mt53->t_crsr.u4Size);
    }

    return ;
}

void system_munmap_imagebuf_addr(void)
{
    int ret=0;

    D_DEBUG_AT(MT53_System, "%s     -> Virt addr: 0x%x\n",__FUNCTION__,dfb_mt53->imagebuf.u4VirtAddr);
    
    if(dfb_mt53->imagebuf.u4VirtAddr)
    {
        if(dfb_mt53->shared->manager_fbm->avail >= dfb_mt53->shared->manager_fbm->length)
        {
            ret = munmap((void *)dfb_mt53->imagebuf.u4VirtAddr,dfb_mt53->imagebuf.u4Size);
            
            D_INFO( "DirectFB/MT53: munmap imagebuf[pid=%d][vir=0x%x,phy=0x%x,size=0x%x][%d]\n",getpid(),
            dfb_mt53->imagebuf.u4VirtAddr,dfb_mt53->imagebuf.u4PhyAddr,dfb_mt53->imagebuf.u4Size,ret);

            dfb_mt53->imagebuf.u4VirtAddr =0x0;
        }
    }

    return;
}

void system_mmap_imagebuf_addr(void)
{
    D_DEBUG_AT(MT53_System, "   %s,%d   -> Phy addr: 0x%x\n", __func__, __LINE__, dfb_mt53->imagebuf.u4PhyAddr);
    
    if (ioctl( dfb_mt53->fd, FBIO_GETIMAGEBUFFER, &dfb_mt53->imagebuf ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
       return ;
    }
    
    dfb_mt53->imagebuf.u4VirtAddr = (unsigned int)mmap( 
                         NULL, 
                         dfb_mt53->imagebuf.u4Size, 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED, 
                         dfb_mt53->fd, 
                         dfb_mt53->imagebuf.u4PhyAddr
                         );
 
    if(dfb_mt53->imagebuf.u4VirtAddr == (unsigned int)MAP_FAILED) 
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the imagebuf  buffer!\n");
    }
    else
    {
        //memset(dfb_mt53->imagebuf.u4VirtAddr,0x00,dfb_mt53->imagebuf.u4Size);
        D_INFO( "DirectFB/MT53: imagebuf[vir=0x%x,phy=0x%x,size=0x%x]\n",
        dfb_mt53->imagebuf.u4VirtAddr,dfb_mt53->imagebuf.u4PhyAddr,dfb_mt53->imagebuf.u4Size);
    }

    return ;
}

void system_munmap_vdpbuf_addr(void)
{
    int ret=0;

    D_DEBUG_AT(MT53_System, "%s[%d][0x%x]\n",__FUNCTION__,getpid(),dfb_mt53->vdpbuf.u4VirtAddr);
    
    if(dfb_mt53->vdpbuf.u4VirtAddr)
    {
        if(dfb_mt53->shared->manager_vdp->avail >= dfb_mt53->shared->manager_vdp->length)
        {
            ret = munmap((void *)dfb_mt53->vdpbuf.u4VirtAddr,dfb_mt53->vdpbuf.u4Size);

            if(ret)
            {
                D_ERROR( "DirectFB/MT53 ERROR: munmap vdpbuf[pid=%d][vir=0x%x,phy=0x%x,size=0x%x] ret[%d]\n",getpid(),
                dfb_mt53->vdpbuf.u4VirtAddr,dfb_mt53->vdpbuf.u4PhyAddr,dfb_mt53->vdpbuf.u4Size,ret);
            }
            else
            {
                D_INFO( "DirectFB/MT53: munmap vdpbuf[pid=%d][vir=0x%x,phy=0x%x,size=0x%x] ret[%d]\n",getpid(),
                dfb_mt53->vdpbuf.u4VirtAddr,dfb_mt53->vdpbuf.u4PhyAddr,dfb_mt53->vdpbuf.u4Size,ret);
            }

            dfb_mt53->vdpbuf.u4VirtAddr =0x0;
        }
    }

    return;
}


void system_mmap_vdpbuf_addr(void)
{
    D_DEBUG_AT(MT53_System, "   %s,%d   -> Phy addr: 0x%x\n", __func__, __LINE__, dfb_mt53->vdpbuf.u4PhyAddr);
    
    if(ioctl( dfb_mt53->fd, FBIO_VDP_GETBUFFER, &dfb_mt53->vdpbuf ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
        return ;
    }
        
    dfb_mt53->vdpbuf.u4VirtAddr = (unsigned int)mmap( 
                         NULL, 
                         dfb_mt53->vdpbuf.u4Size, 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED, 
                         dfb_mt53->fd, 
                         dfb_mt53->vdpbuf.u4PhyAddr);

    if(dfb_mt53->vdpbuf.u4VirtAddr == (unsigned int)MAP_FAILED) 
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the vdpbuf  buffer!\n");
    }
    else
    {
        memset((void *)(dfb_mt53->vdpbuf.u4VirtAddr),0x0,dfb_mt53->vdpbuf.u4Size);
        D_INFO( "DirectFB/MT53: vdpbuf[pid=%d][vir=0x%x,phy=0x%x,size=0x%x]\n",getpid(),
        dfb_mt53->vdpbuf.u4VirtAddr,dfb_mt53->vdpbuf.u4PhyAddr,dfb_mt53->vdpbuf.u4Size);
    }

    return ;
}


void system_munmap_imagebuf2_addr(void)
{
    D_DEBUG_AT(MT53_System, "%s     -> VirtAddr: 0x%x\n",__FUNCTION__,dfb_mt53->imagebuf2.u4VirtAddr);
    
    if(dfb_mt53->imagebuf2.u4VirtAddr)
    {
        if(dfb_mt53->shared->manager_3dmm->avail >= dfb_mt53->shared->manager_3dmm->length)
        {
            munmap((void *)dfb_mt53->imagebuf2.u4VirtAddr,dfb_mt53->imagebuf2.u4Size);
            D_INFO( "DirectFB/MT53: munmap imagebuf2[vir=0x%x,phy=0x%x,size=0x%x]\n",
            dfb_mt53->imagebuf2.u4VirtAddr,dfb_mt53->imagebuf2.u4PhyAddr,dfb_mt53->imagebuf2.u4Size);

            dfb_mt53->imagebuf2.u4VirtAddr =0x0;
        }
    }

    return ;
}

void system_mmap_imagebuf2_addr(void)
{
    D_DEBUG_AT(MT53_System, "%s     -> PhyAddr: 0x%x\n",__FUNCTION__,dfb_mt53->imagebuf2.u4PhyAddr);

    if (ioctl( dfb_mt53->fd, FBIO_GETIMAGEBUFFER2, &dfb_mt53->imagebuf2 ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
       return ;
    }

    dfb_mt53->imagebuf2.u4VirtAddr = (unsigned int)mmap( 
                         NULL, 
                         dfb_mt53->imagebuf2.u4Size, 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED, 
                         dfb_mt53->fd, 
                         dfb_mt53->imagebuf2.u4PhyAddr
                         );
 
    if(dfb_mt53->imagebuf2.u4VirtAddr == (unsigned int)MAP_FAILED) 
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the imagebuf2  buffer!\n");
    }
    else
    {
        //memset(dfb_mt53->imagebuf2.u4VirtAddr,0x00,dfb_mt53->imagebuf2.u4Size);
        D_INFO( "DirectFB/MT53: imagebuf2[vir=0x%x,phy=0x%x,size=0x%x]\n",
        dfb_mt53->imagebuf2.u4VirtAddr,dfb_mt53->imagebuf2.u4PhyAddr,dfb_mt53->imagebuf2.u4Size);
    }

    return ;
}


#ifdef CC_B2R44K2K_SUPPORT
void system_mmap_b2r44K2Kbuf_addr(void)
{
    int n;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);
    
    if (ioctl( dfb_mt53->fd, FBIO_B2R44K2K_GETBUFFER, dfb_mt53->m_B2RSysBuf) < 0) {
         D_PERROR( "DirectFB/MT53: FBIO_VDP_GETBUFFER failed!\n" );
         return;
    }

    for(n = 0; n < B2R_BUFFER_NUM; n++)
    {
        dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr = dfb_mt53->m_B2RSysBuf[n].m_YAddr;
        dfb_mt53->m_B2RDfbBuf[n].u4Size = dfb_mt53->m_B2RSysBuf[n].m_Size;
    }

    for(n = 0; n < B2R_BUFFER_NUM; n++)
    {
        dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr = (unsigned int)mmap( 
                  NULL,
                  dfb_mt53->m_B2RDfbBuf[n].u4Size, 
                  PROT_READ | PROT_WRITE, MAP_SHARED, 
                  dfb_mt53->fd, 
                  dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr
                  );
 
        if (dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr == (unsigned int)MAP_FAILED) {
                   D_PERROR( "DirectFB/MT53: Could not mmap the MPEG2[%d] OSD buffer For 4k2k!\n", n);
                   dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr = 0;
        }
        else
        {
            //memset((void *)(dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr),0x0,dfb_mt53->m_B2RDfbBuf[n].u4Size);
        }
        _B2RBufVirAddr[n] = dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr;
        _B2RBufPhyAddr[n] = dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr;
        _B2RBufCPhyAddr[n] = dfb_mt53->m_B2RSysBuf[n].m_CAddr;

        D_INFO("##########dfb_mt53->mpegbuf[%d] [0x%x,0x%x] [%d] \n", n,dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr, dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr, dfb_mt53->m_B2RDfbBuf[n].u4Size);
        
    }   

    _B2RBufNum = B2R_BUFFER_NUM;
}


void system_munmap_b2r44K2Kbuf_addr(void)
{
    int ret=0;
    int n;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);

    if(!((dfb_mt53->shared->m_B2R44K2KFlags&0xf)==0xf))
    {
        return;
    }
    
    for(n = 0; n < B2R_BUFFER_NUM; n++)
    {
        if(dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr)
        {
            ret=munmap((void *)dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr,dfb_mt53->m_B2RDfbBuf[n].u4Size);
            if(ret)
            {
                D_INFO( "DirectFB/MT53: munmap b2r44K2Kbuf fail[pid=%d,ret=%d][vir=0x%x,phy=0x%x,size=0x%x]\n",getpid(),ret,
                        dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr,
                        dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr,dfb_mt53->m_B2RDfbBuf[n].u4Size);
            }
            else
            {
                D_INFO( "DirectFB/MT53: munmap b2r44K2Kbuf ok[pid=%d,ret=%d][vir=0x%x,phy=0x%x,size=0x%x]\n",getpid(),ret,
                        dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr,
                        dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr,dfb_mt53->m_B2RDfbBuf[n].u4Size);

                dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr =0x0;
                _B2RBufNum = 0x0;
            }
        }
    }  
}

#endif

void system_fb_mmap(void)
{
    int n=0x0;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);

	#ifdef CC_OSD_M4U_IOVA_SUPPORT
	system_mmap_dfb_m4u_addr();
	system_mmap_crsr_m4u_addr();
	#else
    system_mmap_dfb_addr();
    system_mmap_crsr_addr();
	#endif

    if (ioctl( dfb_mt53->fd, FBIO_GETIMAGEBUFFER, &dfb_mt53->imagebuf ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
       return ;
    }
    dfb_mt53->imagebuf.u4VirtAddr   = 0x0;

    if(ioctl( dfb_mt53->fd, FBIO_VDP_GETBUFFER, &dfb_mt53->vdpbuf ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
        return ;
    }
    dfb_mt53->vdpbuf.u4VirtAddr     = 0x0;


    if (ioctl( dfb_mt53->fd, FBIO_GETIMAGEBUFFER2, &dfb_mt53->imagebuf2 ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_CRSR_INFO failed!\n" );
       return ;
    }
    dfb_mt53->imagebuf2.u4VirtAddr= 0x0;
    
    #ifdef CC_B2R44K2K_SUPPORT
    if (ioctl( dfb_mt53->fd, FBIO_B2R44K2K_GETBUFFER, dfb_mt53->m_B2RSysBuf) < 0) 
    {
         D_PERROR( "DirectFB/MT53: FBIO_VDP_GETBUFFER failed!\n" );
         return;
    }
    for(n = 0; n < B2R_BUFFER_NUM; n++)
    {
        dfb_mt53->m_B2RDfbBuf[n].u4PhyAddr = dfb_mt53->m_B2RSysBuf[n].m_YAddr;
        dfb_mt53->m_B2RDfbBuf[n].u4Size    = dfb_mt53->m_B2RSysBuf[n].m_Size;
        dfb_mt53->m_B2RDfbBuf[n].u4VirtAddr=0x0;
    }
    #endif
    
    return;
}

#ifdef CC_OSD_M4U_IOVA_SUPPORT
void * alignment_malloc(unsigned int size, unsigned int aligned,void ** ptraddr)
{
	unsigned int newsize = 0;
	unsigned int iaddr = 0;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);
	
	newsize = size + aligned;
	*ptraddr = malloc(newsize);
	if(!(*ptraddr))
	{
		return NULL;
	}

	for(iaddr = (unsigned int)(*ptraddr); iaddr < ((unsigned int)(*ptraddr) + newsize); iaddr++)
	{
		if(0 == (iaddr % aligned))
			break;
	}

	
	return (void *)iaddr;
}

void * alignment_free(void * ptraddr)
{
	free(ptraddr);
}

void system_osd_m4u_init(void)
{
	unsigned int u4DfbBufSize = 0, u4CacheLineSize=0;
	void * pVirmem = NULL;
	void * pAlignVirmem = NULL;

	D_DEBUG_AT(MT53_System, "%s\n", __func__);

    if (ioctl( dfb_mt53->fd, FBIO_MMU_GET_DFBBUFFERSIZE, &u4DfbBufSize ) < 0) {
	      D_PERROR( "DirectFB/MT53: FBIO_MMU_GET_DFBBUFFERSIZE failed!\n" );
	      //goto error;
	 }

	if (ioctl( dfb_mt53->fd, FBIO_MMU_GET_CACHELINESIZE, &u4CacheLineSize ) < 0) {
	      D_PERROR( "DirectFB/MT53: FBIO_MMU_GET_CACHELINESIZE failed!\n" );
	      //goto error;
	 }

	 if(u4DfbBufSize)
	 {
	 	D_INFO("u4DfbBufSize = 0x%x u4CacheLineSize = 0x%x\n",u4DfbBufSize,u4CacheLineSize);
	 	pAlignVirmem = alignment_malloc(u4DfbBufSize,u4CacheLineSize,&pVirmem);
		if(!pVirmem)
		{
			D_INFO("pVirmem malloc fail!\n");
			//goto error;
		}
		D_INFO("pVirmem:%p ,pAlignVirmem:%p \n",pVirmem,pAlignVirmem);
			
		allocm4u.u4_alignaddr = pAlignVirmem;
		allocm4u.u4_addr = pVirmem;
		allocm4u.size = u4DfbBufSize;
		
		allocm4u.pin_range[E_DSSP_ALLOC].start = (unsigned long)(allocm4u.u4_alignaddr);
		allocm4u.pin_range[E_DSSP_ALLOC].size = allocm4u.size;
		system_mmu_pin_mem_osd(&allocm4u.pin_range[E_DSSP_ALLOC]); //E_DSSP_ALLOC : osd

		D_INFO("osd iova =0x%x\n",allocm4u.pin_range[E_DSSP_ALLOC].iova);
	 }
}

void system_gfx_m4u_init(void)
{
	struct fb_iovaswitch fbiova;
	
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
	fbiova.siova = allocm4u.pin_range[E_DSSP_ALLOC].iova;
	fbiova.ssize = allocm4u.pin_range[E_DSSP_ALLOC].size;
	
	if (ioctl( dfb_mt53->fd, FBIO_MMU_CHANGE_DIFF_DOMAIN_IOVA, &fbiova ) < 0) {
	      D_PERROR( "DirectFB/MT53: FBIO_MMU_GET_DFBBUFFERSIZE failed!\n" );
	      //goto error;
	 }

	allocm4u.pin_range[E_DSSP_SH].iova = fbiova.diova; //E_DSSP_SH : gfx
	D_INFO("gfx iova =0x%x\n",allocm4u.pin_range[E_DSSP_SH].iova);
}

void system_iova_fb_probe(void)
{
	struct fb_iovaswitch fbiova;
	
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
	fbiova.siova = allocm4u.pin_range[E_DSSP_ALLOC].iova; //E_DSSP_ALLOC:osd
	fbiova.ssize = allocm4u.pin_range[E_DSSP_ALLOC].size;
	fbiova.diova = allocm4u.pin_range[E_DSSP_SH].iova;   //E_DSSP_SH:gfx
	
	if (ioctl( dfb_mt53->fd, FBIO_MMU_IOVA_FB_PROBE, &fbiova ) < 0) {
	      D_PERROR( "DirectFB/MT53: FBIO_MMU_IOVA_FB_PROBE failed!\n" );
	      //goto error;
	 }
}

void system_gfx_m4u_uninit(void)
{
	struct fb_iovaswitch fbiova;
	
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
	fbiova.diova = allocm4u.pin_range[E_DSSP_SH].iova;
	fbiova.ssize = allocm4u.pin_range[E_DSSP_ALLOC].size;
	
	if(allocm4u.pin_range[E_DSSP_SH].iova)
	{
		if (ioctl( dfb_mt53->fd, FBIO_MMU_FREE_NEW_DOMAIN_IOVA, &fbiova ) < 0) {
		      D_PERROR( "DirectFB/MT53: FBIO_MMU_FREE_NEW_DOMAIN_IOVA failed!\n" );
		      //goto error;
		 }
	}
}

void system_osd_m4u_uninit(void)
{
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
	
	if(allocm4u.pin_range[E_DSSP_ALLOC].iova)
		system_mmu_unpin_mem_osd(&allocm4u.pin_range[E_DSSP_ALLOC]); //E_DSSP_ALLOC : osd
	
	if(allocm4u.u4_addr)
		alignment_free(allocm4u.u4_addr);
	
	memset(&allocm4u,0x0,sizeof(MT53AllocationData));
}

void system_mmap_dfb_m4u_addr(void)
{
	struct mt53fb_imagebuffer sIova;
		
	D_DEBUG_AT(MT53_System, "%s\n", __func__);

	//get dfb_mt53->fix 
	if(ioctl( dfb_mt53->fd, FBIOGET_FSCREENINFO, &dfb_mt53->fix ) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIOGET_FSCREENINFO failed!\n" );
        return ;
    }
	memcpy(&dfb_mt53->m4u_fix, &(dfb_mt53->fix), sizeof(struct fb_fix_screeninfo));
	
	//update fix->smem_start(dfb kernel space virtual),fix->smem_len(dfb buffer len)
	//fix->mmio_start(dfb iova address), fix->mmio_len(dfb buffer len)
    if(ioctl( dfb_mt53->fd, FBIO_MMU_GET_DFB_IOVA_ADDR, &sIova) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_MMU_GET_DFB_IOVA_ADDR failed!\n" );
        return ;
    }
	dfb_mt53->m4u_fix.mmio_len = sIova.u4Size;
	dfb_mt53->m4u_fix.smem_len = sIova.u4Size;
	dfb_mt53->m4u_fix.mmio_start =sIova.u4PhyAddr;
	dfb_mt53->m4u_fix.smem_start = sIova.u4VirtAddr;
	
	
	printf("dfb_mt53->m4u_fix.mmio_start:0x%lx,size:0x%x,dfb_mt53->m4u_fix.smem_start:0x%lx\n",dfb_mt53->m4u_fix.mmio_start,dfb_mt53->m4u_fix.mmio_len,dfb_mt53->m4u_fix.smem_start);
    dfb_mt53->mem = mmap( (void*)NULL,
                                dfb_mt53->m4u_fix.mmio_len,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED ,
                                dfb_mt53->fd,
                                dfb_mt53->m4u_fix.mmio_start);

    if(dfb_mt53->mem == MAP_FAILED)
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the m4u buffer!\n");
    }
    else
    {
        D_INFO( "DirectFB/MT53: DFB[vir=0x%x,iova=0x%x,size=0x%x]\n",
            (int)dfb_mt53->mem,dfb_mt53->m4u_fix.mmio_start,dfb_mt53->m4u_fix.mmio_len);
    }

    return;
}

void system_mmap_crsr_m4u_addr(void)
{
	struct mt53fb_imagebuffer sIova;
		
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
    if(ioctl( dfb_mt53->fd, FBIO_MMU_GET_CRSR_IOVA_ADDR, &sIova) < 0) 
    {
        D_PERROR( "DirectFB/MT53: FBIO_MMU_GET_CRSR_IOVA_ADDR failed!\n" );
        return ;
    }
	dfb_mt53->t_crsr.u4Size = sIova.u4Size;
	dfb_mt53->t_crsr.u4PhyAddr =sIova.u4PhyAddr;
	
	printf("dfb_mt53->t_crsr.u4IOVAddr:0x%lx,size:0x%x\n",dfb_mt53->t_crsr.u4PhyAddr,dfb_mt53->t_crsr.u4Size);
    dfb_mt53->t_crsr.u4VirtAddr = (unsigned int)mmap(
                         (void*)NULL,
                         dfb_mt53->t_crsr.u4Size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED,
                         dfb_mt53->fd,
                         dfb_mt53->t_crsr.u4PhyAddr);

    if(dfb_mt53->t_crsr.u4VirtAddr == (unsigned int)MAP_FAILED)
    {
        D_PERROR( "DirectFB/MT53: Could not mmap the t_crsr  buffer!\n");
    }
    else
    {
        //memset(dfb_mt53->t_crsr.u4VirtAddr,0x0,dfb_mt53->t_crsr.u4Size);
        D_INFO( "DirectFB/MT53: t_crsr[vir=0x%x,phy=0x%x,size=0x%x]\n",
        dfb_mt53->t_crsr.u4VirtAddr,dfb_mt53->t_crsr.u4PhyAddr,dfb_mt53->t_crsr.u4Size);
    }

    return;
}

UINT32 system_gfx_switch_osd_iova(UINT32 u4Siova)
{
	struct fb_iovaswitch fbiova;
	
	D_DEBUG_AT(MT53_System, "%s\n", __func__);
	fbiova.siova = u4Siova;
	
	if (ioctl( dfb_mt53->fd, FBIO_MMU_GFX_SWITCH_OSD_IOVA, &fbiova ) < 0) {
	      D_PERROR( "DirectFB/MT53: FBIO_MMU_GFX_SWITCH_OSD_IOVA failed!\n" );
	      //goto error;
	 }

	return fbiova.diova;
}

#endif

u32 system_mmu_transform_gfx2osd_iova( u32 u4_src, size_t u4_size)
{
    struct fb_iovaswitch fbiova;
    u32    iova;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);

    memset(&fbiova, 0x0, sizeof(fbiova));

    fbiova.siova = u4_src;
    fbiova.ssize = u4_size;

    if (ioctl( dfb_mt53->fd, FBIO_MMU_TRANSFORM_GFX2OSD_IOVA, &fbiova) < 0)
    {
        D_PERROR("DirectFB/MT53: FBIO_MMU_TRANSFORM_GFX2OSD_IOVA failed!\n");
    } else {
        iova = fbiova.diova;
    }

    return iova;
}

int system_mmu_free_osd_iova(u32 u4_src, size_t u4_size)
{
    struct fb_iovaswitch fbiova;
    u32    iova;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);

    memset(&fbiova, 0x0, sizeof(fbiova));

    fbiova.siova = u4_src;
    fbiova.ssize = u4_size;

    if (ioctl( dfb_mt53->fd, FBIO_MMU_FREE_OSD_IOVA, &fbiova) < 0)
    {
        D_PERROR("DirectFB/MT53: FBIO_MMU_TRANSFORM_GFX2OSD_IOVA failed!\n");
        return -1;
    }

    return 0;
}

#ifdef DFB_INPUT_FILER_EN

static FusionCallHandlerResult
call_handler( int           caller,
              int           call_arg,
              void         *call_ptr,
              void         *ctx,
              unsigned int  serial,
              int          *ret_val )
{
     bool ret = false;
     MT53Shared *shared;
     
     D_DEBUG_AT( MT53_System, "%s,%d     -> pid=%d; recieve event\n", __func__, __LINE__, getpid());

     D_ASSERT((dfb_mt53 != NULL));
     D_ASSERT((dfb_mt53->shared != NULL));

     shared = dfb_mt53->shared;
     
     if(dfb_mt53->input_filter != NULL)
     {
         ret = dfb_mt53->input_filter((const DFBInputEvent *)&shared->input_event);
     }

     D_DEBUG_AT( MT53_System, "event [%d][%d] \n", shared->input_event.key_symbol, shared->input_event.type);
     
     *ret_val = (int) ret;

     return FCHR_RETURN;
}


static void
system_participant_init(MT53Shared *shared)
{
    int i;
    D_ASSERT((shared != NULL));

    for(i = 0; i < MT53_MAX_PARTICIPANTS; i++)
    {
        shared->participants[i].mark = false;
    }

    shared->participant_number = 1;
}

static void
system_participant_attend(MT53Shared *shared)
{
    int i;
    DirectResult ret;
    mt53_participant_t *participant = NULL; 
    D_ASSERT((shared != NULL));

    //get idle participant
    for(i = 0; i < MT53_MAX_PARTICIPANTS; i++)
    {
        if(shared->participants[i].mark == false)
        {
            participant = &shared->participants[i];
        }
    }

    if(participant != NULL)
    {
        ret = fusion_call_init(&participant->call, call_handler, shared, dfb_core_world(dfb_mt53->core));
        if(ret < 0)
        {
            return;
        }
        
        participant->mark = true;
        participant->inputfilter = false;
        participant->pid = (int)getpid();
        shared->participant_number++;
    }
}

static void
system_participant_leave(MT53Shared *shared)
{
    int i, pid;
    mt53_participant_t *participant = NULL; 
    D_ASSERT((shared != NULL));

    pid = getpid();
    //get remove participant
    for(i = 0; i < MT53_MAX_PARTICIPANTS; i++)
    {
        if(shared->participants[i].mark == true && shared->participants[i].pid == pid)
        {
            participant = &shared->participants[i];
        }
    }

    if(participant != NULL)
    {
        fusion_call_destroy(&participant->call);
        participant->mark = false;
        participant->inputfilter = false;
        participant->pid = -1;
        shared->participant_number--;
    }
}

static bool
system_participant_dispatch_inputevent(MT53Shared *shared, DFBInputEvent *event)
{
    int i, mark = 0, retcall = 0;
    D_ASSERT((shared != NULL));
    D_ASSERT((event != NULL));

    if(dfb_mt53->input_filter != NULL)
    {
        mark |= (int) dfb_mt53->input_filter((const DFBInputEvent *)event);
    }

    if(shared->participant_number <= 1)
    {
        return ((bool) mark);
    }

    shared->input_event = *event;
    
    for(i = 0; i < MT53_MAX_PARTICIPANTS; i++)
    {
        if(shared->participants[i].mark == true  && shared->participants[i].inputfilter == true)
        {
            fusion_call_execute(&shared->participants[i].call, FCEF_NONE, 0, 0, &retcall);
            mark |= retcall;
            //D_INFO("return retcall = %d \n", retcall);
        }
    }

    //D_INFO("return mark = %d \n", mark);

    return ((bool) mark);
}
#endif

static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_ANY;
     info->caps = CSCAPS_ACCELERATION;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "MediaTek 53xx" );
}

static DFBResult
system_initialize( CoreDFB *core, void **data )
{
     DFBResult                 ret = DFB_INIT;
     struct fb_var_screeninfo  var;
     FusionSHMPoolShared      *pool;
     CoreScreen               *screen;
     MT53Shared               *shared = NULL;
#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB    
     char str[50];
     char size[50];
#endif
     (void)screen;


     DIRECT_INTERFACE_DBG_DELTA_START();

     D_ASSERT( dfb_mt53 == NULL );

     pool = dfb_core_shmpool( core );

     dfb_mt53 = (MT53 *)D_CALLOC( 1, sizeof(MT53) );
     if (!dfb_mt53)
          return D_OOM();

     dfb_mt53->fd = -1;

     shared = (MT53Shared*) SHCALLOC( pool, 1, sizeof(MT53Shared) );
     if (!shared) {
          ret = D_OOSHM();
          goto error;
     }

     shared->shmpool = pool;

     fusion_arena_add_shared_field( dfb_core_arena( core ), "mt53", shared );

     dfb_mt53->core   = core;
     dfb_mt53->shared = shared;
#ifdef CC_ANDROID_TWO_WORLDS
     dfb_mt53->fd = direct_try_open( "/dev/graphics/fb1", "/dev/graphics/fb/1", O_RDWR, true );
#else
     dfb_mt53->fd = direct_try_open( "/dev/fb0", "/dev/fb/0", O_RDWR, true );
#endif
     if (dfb_mt53->fd < 0)
          goto error;
     
          dfb_fb = dfb_mt53->fd;

     ret = DFB_INIT;

     /* Initialization of mt53fb */
     var.xres           = MT53_PRIMARY_DEFAULT_WIDTH;
     var.yres           = MT53_PRIMARY_DEFAULT_HEIGHT;
     var.xres_virtual   = MT53_PRIMARY_DEFAULT_WIDTH;
     var.yres_virtual   = MT53_PRIMARY_DEFAULT_HEIGHT * 4;
     var.xoffset        = 0;
     var.yoffset        = 0;
     var.bits_per_pixel = 32;
     var.vmode = CM_ARGB8888_DIRECT32;

     if (ioctl( dfb_mt53->fd, FBIO_WAITLOGO, &var ) < 0) {
          D_PERROR( "DirectFB/MT53: FBIO_WAITLOGO failed!\n" );
          goto error;
     }

     if (ioctl( dfb_mt53->fd, FBIO_INIT, &var ) < 0) {
          D_PERROR( "DirectFB/MT53: FBIO_INIT failed!\n" );
          goto error;
     }

     /* Map the framebuffer */
     system_fb_mmap();  
     
     dfb_surface_pool_initialize( core, &mt53SurfacePoolFuncs, &dfb_mt53->shared->pool );

     *data = dfb_mt53;
#ifdef CC_FBM_TWO_FBP_SHARED_WITH_DFB 
     sprintf(str,"%d",dfb_mt53->imagebuf2.u4VirtAddr);
     sprintf(size,"%d",dfb_mt53->imagebuf2.u4Size);  
     D_INFO("======DFB 3DMM u4VirtAddr : 0x%x, and DFB 3DMM u4Size : 0x%x======\n", dfb_mt53->imagebuf2.u4VirtAddr,dfb_mt53->imagebuf2.u4Size);

     if (setenv("DFB_3DMM_VIRTUAL_SIZE", size, 1) == -1)
     {
         D_PERROR( "DirectFB/MT53: Set DFB_3DMM_VIRTUAL_SIZE env error!\n");
         putenv("DFB_3DMM_VIRTUAL_SIZE=0");
     }
     
     if (setenv("DFB_3DMM_VIRTUAL_ADDR", str, 1) == -1)
     {
         D_PERROR( "DirectFB/MT53: Set DFB_3DMM_VIRTUAL_ADDR env error!\n");
         putenv("DFB_3DMM_VIRTUAL_ADDR=0");
     }
#endif
#ifdef DFB_INPUT_FILER_EN

     system_participant_init(shared);
     dfb_mt53->input_filter = NULL;
#endif
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_SYS_INIT);
     return DFB_OK;


error:
     if (shared)
          SHFREE( pool, shared );

     if (dfb_mt53->mem)
          munmap( dfb_mt53->mem, dfb_mt53->fix.smem_len );

     if (dfb_mt53->fd >= 0)
          close( dfb_mt53->fd );

     D_FREE( dfb_mt53 );
     dfb_mt53 = NULL;

     return ret;
}

int system_get_fb(void)
{
    D_ASSERT( dfb_mt53 == NULL );
    
    return dfb_mt53->fd;
}

static DFBResult
system_join( CoreDFB *core, void **data )
{
     DFBResult   ret = DFB_INIT;
     struct fb_var_screeninfo  var;
     CoreScreen *screen;
     void       *shared;
     
     DIRECT_INTERFACE_DBG_DELTA_START();
     
     D_ASSERT( dfb_mt53 == NULL );
     
     dfb_mt53 = D_CALLOC( 1, sizeof(MT53) );
     if (!dfb_mt53)
          return D_OOM();

     fusion_arena_get_shared_field( dfb_core_arena( core ), "mt53", &shared );

     dfb_mt53->core   = core;
     dfb_mt53->shared = shared;

#ifdef CC_ANDROID_TWO_WORLDS
     dfb_mt53->fd = direct_try_open( "/dev/graphics/fb1", "/dev/graphics/fb/1", O_RDWR, true );
#else
     dfb_mt53->fd = direct_try_open( "/dev/fb0", "/dev/fb/0", O_RDWR, true );
#endif   
     if (dfb_mt53->fd < 0)
          goto error;
     
#ifdef CC_SUPPORT_DYNAMIC_ENABLE_DFB_INPUTEVENT
          dfb_fb = dfb_mt53->fd;
#endif

     if (ioctl( dfb_mt53->fd, FBIO_WAITLOGO, &var ) < 0) {
          D_PERROR( "DirectFB/MT53: FBIO_WAITLOGO failed!\n" );
          goto error;
     }

     /* Map the framebuffer */
     system_fb_mmap();

     dfb_surface_pool_join( core, dfb_mt53->shared->pool, &mt53SurfacePoolFuncs );

     (void)screen;

     *data = dfb_mt53;
#ifdef DFB_INPUT_FILER_EN
     system_participant_attend(shared);
     dfb_mt53->input_filter = NULL;
#endif
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_SYS_INIT);
     return DFB_OK;
error:
     if (dfb_mt53->mem)
          munmap( dfb_mt53->mem, dfb_mt53->fix.smem_len );

     if (dfb_mt53->fd >= 0)
          close( dfb_mt53->fd );

     D_FREE( dfb_mt53 );
     dfb_mt53 = NULL;

     return ret;
}

static DFBResult
system_shutdown( bool emergency )
{
     MT53Shared          *shared;
     FusionSHMPoolShared *pool;

     D_ASSERT( dfb_mt53 != NULL );

     shared = dfb_mt53->shared;
     D_ASSERT( shared != NULL );

     pool = shared->shmpool;
     D_ASSERT( pool != NULL );

     dfb_surface_pool_destroy( shared->pool );

     SHFREE( pool, shared );


     munmap( dfb_mt53->mem, dfb_mt53->fix.smem_len );

     close( dfb_mt53->fd );

     D_FREE( dfb_mt53 );
     dfb_mt53 = NULL;

     return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
     D_ASSERT( dfb_mt53 != NULL );
#ifdef DFB_INPUT_FILER_EN
     system_participant_leave(dfb_mt53->shared);
#endif
     munmap( dfb_mt53->mem, dfb_mt53->fix.smem_len );

     close( dfb_mt53->fd );

     D_FREE( dfb_mt53 );
     dfb_mt53 = NULL;

     return DFB_OK;
}

static DFBResult
system_suspend()
{
     return DFB_OK;
}

static DFBResult
system_resume()
{
     return DFB_OK;
}

/******************************************************************************/

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

int system_get_accelerator()
{
     return FB_ACCEL_MEDIATEK_53XX;
}

static VideoMode *
system_get_modes()
{
     return NULL;
}

static VideoMode *
system_get_current_mode()
{
     return NULL;
}

static DFBResult
system_thread_init()
{
     return DFB_OK;
}

static bool
system_input_filter( CoreInputDevice *device,
                     DFBInputEvent   *event )
{
#ifdef DFB_INPUT_FILER_EN

     return system_participant_dispatch_inputevent(dfb_mt53->shared, event);
#else
    return false;
#endif
}

unsigned long system_video_memory_physical( unsigned int offset )
{
	 #ifdef CC_OSD_M4U_IOVA_SUPPORT
	 return dfb_mt53->m4u_fix.mmio_start + offset;
	 #else
     return dfb_mt53->fix.smem_start + offset;
	 #endif
}

void* system_video_memory_virtual( unsigned int offset )
{
     return dfb_mt53->mem + offset;
}

unsigned int system_videoram_length()
{
     return dfb_mt53->fix.smem_len;
}

unsigned long system_aux_memory_physical( unsigned int offset )
{
#ifdef CC_OSD_USE_FBM
     return dfb_mt53->imagebuf.u4PhyAddr;
#else
     return NULL;
#endif
}

void *system_aux_memory_virtual( unsigned int offset )
{
#ifdef CC_OSD_USE_FBM
     return (void*)dfb_mt53->imagebuf.u4VirtAddr;
#else
     return NULL;
#endif
}

unsigned int system_auxram_length()
{
#ifdef CC_OSD_USE_FBM
     return dfb_mt53->imagebuf.u4Size;
#else
     return 0;
#endif
}

void
system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
}

void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
}

int system_surface_data_size( void )
{
    return 0x0;
}

void system_surface_data_init( CoreSurface *surface, void *data )
{
    return;
}

void system_surface_data_destroy( CoreSurface *surface, void *data )
{
    return;
}
int system_mmu_pin_mem( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    unsigned int va = 0;
#endif
    int ret=0;
    int i=0;
    
    DIRECT_INTERFACE_DBG_DELTA_START();
    
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->devIdx=0; //gfx
    pt_page->direction=DFB_MTIOMMU_DMA_BIDIRECTION;
    va=(pt_page->start&0xfff);
    direct_log_printf(NULL, "%s ", __FUNCTION__);
#endif

    for(i = 0; i < 2; i++)
    {
        ret = ioctl(dfb_mt53->fd, FBIO_MMU_PINMEM, pt_page);
        if( ret < 0 )
        {
            memset((void *)(unsigned long)(pt_page->start),0xff,pt_page->size);//force kernel to do pte update
            D_DEBUG_AT(MT53_System, "pinmem_try_again\n");
            continue;
        } else {
            break;
        }
    }

    if(ret<0)
    {
        D_ERROR( "mmu_pin_mem fail! [0x%x,0x%x]\n",pt_page->start,pt_page->size);
        return -1;
    }

#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->iova|=va;
#endif

    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PIN_MEM);
        
    return 0x0;
}

int system_mmu_get_dmafd( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
#if 0 /*FBIO_MMU_EXPORTMEM located in dtv_driver*/
    struct fb_dmaexport rExport;

    if (!pt_page)
        return -1;

    memset(&rExport, 0x0, sizeof(struct fb_dmaexport));

    rExport.idev = 0;   /* 0 is gfx */
    rExport.iova = pt_page->iova;
    rExport.size = pt_page->size;
    rExport.fd   = -1;

    if (ioctl(dfb_mt53->fd, FBIO_MMU_EXPORTMEM, &rExport) < 0)
    {
        D_PERROR("error to ioctl FBIO_MMU_EXPORTMEM, iova = 0x%x, size = %d\n", rExport.iova, rExport.size);
        return -1;
    }

    D_DEBUG_AT(MT53_System, "[%s(),%d]: iova = 0x%x, size = %d, fd = %d\n",
            __func__, __LINE__, rExport.iova, rExport.size, rExport.fd);

    return rExport.fd;
#else /* FBIOGET_DFB_DMA_BUF in fbdev core */
    struct dfb_mem_info rMemInfo;

    memset(&rMemInfo, 0x0, sizeof(struct dfb_mem_info));
    rMemInfo.iova = pt_page->iova;
    rMemInfo.size = pt_page->size;
    rMemInfo.type = FB_MEM_TYPE_IOVA_DMABUF;

    if (ioctl(dfb_mt53->fd, FBIOGET_DFB_DMA_BUF, &rMemInfo) < 0)
    {
        D_PERROR("error to ioctl FBIOGET_DFB_DMA_BUF, start = 0x%x, size = %d\n", rMemInfo.phyaddr, rMemInfo.size);
        return -1;
    }

    D_DEBUG_AT(MT53_System, "[%s(),%d]: iova = 0x%x, size = %d, fd = %d\n",
            __func__, __LINE__, rMemInfo.iova, rMemInfo.size, rMemInfo.fd);

    return rMemInfo.fd;
#endif
}

int system_mem_get_dmafd(unsigned int u4_phyaddr, unsigned int u4_size)
{
    struct dfb_mem_info rMemInfo;

    memset(&rMemInfo, 0x0, sizeof(struct dfb_mem_info));

    rMemInfo.phyaddr = u4_phyaddr;
    rMemInfo.size    = u4_size;
    rMemInfo.type    = FB_MEM_TYPE_DMABUF;

    if (ioctl(dfb_mt53->fd, FBIOGET_DFB_DMA_BUF, &rMemInfo) < 0)
    {
        D_PERROR("error to ioctl FBIOGET_DFB_DMA_BUF, start = 0x%x, size = %d\n", rMemInfo.phyaddr, rMemInfo.size);
        return -1;
    }

    D_DEBUG_AT(MT53_System, "[%s(),%d]: start = 0x%x, size = %d, fd = %d\n",
            __func__, __LINE__, rMemInfo.phyaddr, rMemInfo.size, rMemInfo.fd);

    return rMemInfo.fd;
}

int system_mmu_pin_mem_osd( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    unsigned int va = 0;
#endif
    int ret=0;
    int i=0;
    
    DIRECT_INTERFACE_DBG_DELTA_START();
    
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->devIdx=5; //osd
    pt_page->direction=DFB_MTIOMMU_DMA_BIDIRECTION;
    va=(pt_page->start&0xfff);
    direct_log_printf(NULL, "%s ", __FUNCTION__);
#endif

    for(i=0;i<2;i++)
    {
        ret=ioctl(dfb_mt53->fd, FBIO_MMU_PINMEM, pt_page);
#ifdef GFX_IOMMU_IOVA_SUPPORT    
        break;
#endif
        if((ret<0) && (errno==EAGAIN))
        {
            memset((void *)(unsigned long)(pt_page->start),0xff,pt_page->size);//force kernel to do pte update
            D_DEBUG_AT(MT53_System, "pinmem_try_again\n");
            continue;
        }
        break;
    }
    if(ret<0)
    {
        D_ERROR( "mmu_pin_mem fail! [0x%x,0x%x]\n",pt_page->start,pt_page->size);
    }
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->iova|=va;
#endif

    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PIN_MEM);
        
    return 0x0;
}


#ifdef DFB_IOMMU_PRECHECK
unsigned int system_mmu_get_ttb(void)
{
    unsigned int TTB = 0;
    if(ioctl( dfb_mt53->fd, FBIO_MMU_GETPGD, &TTB ) < 0)
    {
        D_PERROR(NULL, "system_mmu_get_ttb Fail!\n");
    }
    return TTB;
}
#endif
int system_mmu_pin_iomem( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
    DIRECT_INTERFACE_DBG_DELTA_START();
    if(ioctl( dfb_mt53->fd, FBIO_MMU_IOMAP_PINMEM, pt_page ) < 0)
    {
        D_PERROR(NULL, "mmu_pin_iomem Fail![0x%x,0x%x], pid=0x%x\n",pt_page->start,pt_page->size, getpid());
        return -1;
    }
    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_PIN_MEM);
        
    return 0x0;
}

int system_mmu_unpin_mem( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
    DIRECT_INTERFACE_DBG_DELTA_START();
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->devIdx=0; //gfx
#endif
    if(ioctl( dfb_mt53->fd, FBIO_MMU_UNPINMEM, pt_page ) < 0)
    {
        D_PERROR(NULL, "mmu_unpin_mem Fail![0x%x,0x%x]\n",pt_page->start,pt_page->size);
    }
    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_UNPIN_MEM);
    return 0x0;
}

int system_mmu_unpin_mem_osd( DFB_IOMMU_PIN_RANGE_T *pt_page )
{
    DIRECT_INTERFACE_DBG_DELTA_START();
#ifdef GFX_IOMMU_IOVA_SUPPORT//need to be kept.
    pt_page->devIdx=5; //osd
#endif
    if(ioctl( dfb_mt53->fd, FBIO_MMU_UNPINMEM, pt_page ) < 0)
    {
        D_PERROR(NULL, "mmu_unpin_mem Fail![0x%x,0x%x]\n",pt_page->start,pt_page->size);
    }
    DIRECT_INTERFACE_DBG_DELTA_END(DDDT_UNPIN_MEM);
    return 0x0;
}

void system_get_fbm_buf(struct mt53fb_imagebuffer *pt_fbm)
{
    int i4_cnt =0x0;
    
    if(pt_fbm == NULL )
    {
         D_WARN("%s,%d     -> null pointer input\n", __func__, __LINE__);
         return ;
    }
    
    do
    {
        if( dfb_mt53 == NULL )
        {
             D_WARN("%s,%d     -> mt53 null pointer\n", __func__, __LINE__);
             break;
        }
        if (ioctl( dfb_mt53->fd, FBIO_FBM_INFO, dfb_mt53->t_fbm ) < 0)
        {
            D_PERROR( "DirectFB/MT53: FBIO_VDP_GETBUFFER failed!\n" );
            break;
        }


        dfb_mt53->t_fbm[0].u4VirtAddr = (unsigned int)mmap(
                             NULL,
                             dfb_mt53->t_fbm[0].u4Size,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             dfb_mt53->fd,
                             dfb_mt53->t_fbm[0].u4PhyAddr
                             );
        if (dfb_mt53->t_fbm[0].u4VirtAddr == (unsigned int)MAP_FAILED)
        {
            D_PERROR( "DirectFB/MT53: Could not mmap the fbm total buffer!\n");
            break;
        }
        else
        {
            D_INFO( "DirectFB/MT53: FBM[vir=0x%x,phy=0x%x,size=0x%x]\n",
            dfb_mt53->t_fbm[0].u4VirtAddr,dfb_mt53->t_fbm[0].u4PhyAddr,dfb_mt53->t_fbm[0].u4Size);
        }
        pt_fbm->u4VirtAddr  = dfb_mt53->t_fbm[0].u4VirtAddr;
        pt_fbm->u4PhyAddr   = dfb_mt53->t_fbm[0].u4PhyAddr;
        pt_fbm->u4Size      = dfb_mt53->t_fbm[0].u4Size;

    }while(0);

    return ;
}



void* system_get_crsr_addr(void)
{
    if( dfb_mt53 == NULL )
    {
         return 0x0;
    }
    
    return (void*)dfb_mt53->t_crsr.u4VirtAddr;
}


unsigned int system_get_crsr_phy(void)
{
    if( dfb_mt53 == NULL )
    {
         return 0x0;
    }
    
    return dfb_mt53->t_crsr.u4PhyAddr;
}

unsigned int system_get_crsr_size(void)
{
    if( dfb_mt53 == NULL )
    {
         return 0x0;
    }
    
    return dfb_mt53->t_crsr.u4Size;
}

void dfb_dbg_delta_clear(void)
{
    unsigned int u4_idx =0x0;
    
    if( dfb_mt53 == NULL )
    {
         return;
    }
    
    for(u4_idx =0x0;u4_idx < DDDT_MAX;u4_idx++)
    {
        dfb_mt53->shared->a_dbg_delta[u4_idx].u8_cnt            = 0x0;
        dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time = 0x0;
    }
}

void dfb_dbg_delta_print(void)
{
    unsigned int u4_idx         = 0x0;
    long long    u8_val         = 0x0;
    long long    u8_drv_val     = 0x0;
    long long    u8_surf_val    = 0x0;
    long long    u8_card_val    = 0x0;
    
    if( dfb_mt53 == NULL )
    {
         return;
    }
    
    for(u4_idx =0;u4_idx < DDDT_EMMT_CMD;u4_idx++)
    {
        u8_val += dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time;
        
        if((u4_idx>=DDDT_SURFACE_FLIP)&&(u4_idx<=DDDT_SURFACE_DRAWMONOGLYPHS))
        {
            u8_surf_val += dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time;
        }

        if((u4_idx>=DDDT_CARD_FILLRECT)&&(u4_idx<=DDDT_CARD_TEXTURE_TRIANGLES))
        {
            u8_card_val += dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time;
        }
    }
    direct_log_printf(NULL,"%s[pid = %d] cpu[%lld ms] s[%lld ms] c[%lld ms]",__FUNCTION__,getpid(),u8_val/1000,u8_surf_val/1000,u8_card_val/1000);

    for(u4_idx = DDDT_EMMT_CMD;u4_idx < DDDT_MAX;u4_idx++)
    {
        u8_drv_val += dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time;
    }

    direct_log_printf(NULL,"hw[%lld ms] \n",u8_drv_val/1000);

    for(u4_idx = 0;u4_idx < DDDT_MAX;u4_idx++)
    {
        if(!(u4_idx%2))
        {
            direct_log_printf(NULL,"\n");
        }
        direct_log_printf(NULL,"delta[%4d][%4lld][%8lld ms] ",u4_idx,dfb_mt53->shared->a_dbg_delta[u4_idx].u8_cnt,dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time/1000);
    }

    direct_log_printf(NULL,"\n");
}


u32 dfb_interface_dbg_delta_enable(void)
{
    if( dfb_mt53 == NULL )
    {
         return 0x0;
    }

    return (dfb_mt53->shared->u4_dbg_lvl&MT53_DBG_DELTA_ENABLE);
}

void dfb_interface_dbg_set_delta_enable(unsigned int u4_enable)
{
    if( dfb_mt53 == NULL )
    {
         return;
    }

    if(u4_enable)
    {
        dfb_mt53->shared->u4_dbg_lvl |= (MT53_DBG_DELTA_ENABLE);
    }
    else
    {
        dfb_mt53->shared->u4_dbg_lvl &= (~MT53_DBG_DELTA_ENABLE);
    }

    return;
}

void dfb_interface_dbg_set_delta_data(unsigned int u4_idx,long long u8_val)
{
    if( dfb_mt53 == NULL )
    {
         return;
    }
    
    if(u4_idx >= MT53_MAX_DBG_DELTA)
    {
        return;
    }

    if(u8_val>1000000)
    {
        direct_log_printf(NULL,"set_delta_data fail[%d][%lld ms] \n",u4_idx,u8_val/1000);
        return;
    }
    
    dfb_mt53->shared->a_dbg_delta[u4_idx].u8_cnt++;
    dfb_mt53->shared->a_dbg_delta[u4_idx].u8_dfb_delta_time += u8_val;
}


void system_get_tmp_buf(struct mt53fb_imagebuffer *pt_fbm)
{
    if(pt_fbm == NULL )
    {
         return ;
    }
    
    do
    {
        if( dfb_mt53 == NULL )
        {
             break;
        }
        
        if (ioctl( dfb_mt53->fd, FBIO_GET_TMP_BUF, pt_fbm ) < 0) 
        {
            D_PERROR( "DirectFB/MT53: FBIO_VDP_GETBUFFER failed!\n" );
            break;
        }
    }while(0);

    return ;
}

void system_lock_tmp_buf(void)
{
    do
    {
        if( dfb_mt53 == NULL )
        {
             break;
        }
        
        if (ioctl( dfb_mt53->fd, FBIO_LOCK_TMP_BUF) < 0) 
        {
            D_PERROR( "DirectFB/MT53: system_lock_tmp_buf failed!\n" );
            break;
        }
    }while(0);

    return ;
}

void system_unlock_tmp_buf(void)
{
    do
    {
        if( dfb_mt53 == NULL )
        {
             break;
        }
        
        if (ioctl( dfb_mt53->fd, FBIO_UNLOCK_TMP_BUF) < 0) 
        {
            D_PERROR( "DirectFB/MT53: system_unlock_tmp_buf failed!\n" );
            break;
        }
    }while(0);

    return ;
}

void system_munmap_check(void)
{
    system_munmap_imagebuf_addr();
    system_munmap_imagebuf2_addr();
    system_munmap_vdpbuf_addr();
    
    #ifdef CC_B2R44K2K_SUPPORT
    system_munmap_b2r44K2Kbuf_addr();
    #endif
}
#ifdef DFB_INPUT_FILER_EN

void dfb_register_inputfilter(DFBInputFilter filter)
{
    int i, pid;
    MT53Shared *shared;

    D_DEBUG_AT(MT53_System, "%s\n", __func__);

    if(dfb_mt53 != NULL)
    {
        dfb_mt53->input_filter = filter;
        if(dfb_core_is_master(dfb_mt53->core) == false)
        {
            pid = getpid();
            shared = dfb_mt53->shared;
            for(i = 0; i < MT53_MAX_PARTICIPANTS; i++)
            {
                if(shared->participants[i].mark == true && shared->participants[i].pid == pid)
                {
                    shared->participants[i].inputfilter = (bool) filter;
                }
            }
        }
    }
}
#endif
