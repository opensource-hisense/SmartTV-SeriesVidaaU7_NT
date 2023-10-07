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
#ifndef __MT53__MT53_H__
#define __MT53__MT53_H__

#include <linux/fb.h>

#include <directfb.h>

#include <fusion/shm/pool.h>

#include <core/layers.h>
#include <core/screens.h>
#include <core/surface_pool.h>

#include "mt53_fb.h"

#include "surfacemanager.h"

#include "../../gfxdrivers/gles/mstar_gles2.h"

#include <gfx_cmdque.h>
#include <gfx_if.h>

#ifndef FB_ACCEL_MEDIATEK_53XX
#define FB_ACCEL_MEDIATEK_53XX 5300
#endif

#define MT53_PRIMARY_SUPPORTED_OPTIONS (DLOP_ALPHACHANNEL | DLOP_OPACITY | DLOP_SRC_COLORKEY)

#ifndef DEFAULT_LAYER_WIDTH
#define DEFAULT_LAYER_WIDTH 960
#endif
#ifndef DEFAULT_LAYER_HEIGHT
#define DEFAULT_LAYER_HEIGHT 540
#endif
#define MT53_PRIMARY_DEFAULT_WIDTH DEFAULT_LAYER_WIDTH
#define MT53_PRIMARY_DEFAULT_HEIGHT DEFAULT_LAYER_HEIGHT

#define MT53_MAX_OSD_PLANE     (MT53_LAYER_MAX)
#define MT53_PMX_MAX_INPORT_NS 0x8

#if DIRECT_INTERFACE_DBG_DELTA
#define MT53_MAX_DBG_DELTA              (DDDT_MAX)
#else
#define MT53_MAX_DBG_DELTA              (0x1)
#endif
#define MT53_DBG_DELTA_ENABLE           (0x1)


typedef struct {
     long long   u8_dfb_delta_time;
     long long   u8_cnt;
} MT53_DBG_DELTA_T;
#ifdef DFB_INPUT_FILER_EN

#define MT53_MAX_PARTICIPANTS           (20)
typedef struct
{
    FusionCall  call;
    bool        mark;
	bool        inputfilter;
    int         pid;
}mt53_participant_t;
#endif
typedef struct {
     CoreSurfacePool           *pool;

     FusionSHMPoolShared       *shmpool;

     SurfaceManager            *manager;
     SurfaceManager            *manager_fbm;
     SurfaceManager            *manager_3dmm;
     SurfaceManager            *manager_vdp;
     SurfaceManager            *manager_crsr;
     #ifdef CC_B2R44K2K_SUPPORT
     unsigned int               m_B2R44K2KFlags;
     #endif
#ifdef DFB_INPUT_FILER_EN
	 mt53_participant_t         participants[MT53_MAX_PARTICIPANTS];
	 int                        participant_number;
	 DFBInputEvent              input_event;
#endif
     unsigned int               u4_dbg_lvl;
     MT53_DBG_DELTA_T           a_dbg_delta[MT53_MAX_DBG_DELTA];
} MT53Shared;

typedef struct 
{
    MT53Shared                 *shared;
    CoreDFB                    *core;
    int                        fd;
    struct fb_fix_screeninfo  fix;
    void                       *mem;
	struct fb_fix_screeninfo  m4u_fix;

    struct mt53fb_imagebuffer  imagebuf;
    struct mt53fb_imagebuffer  imagebuf2;
    struct mt53fb_imagebuffer  vdpbuf;
    struct mt53fb_imagebuffer  t_fbm[E_OAD_BUF_MAX];
    struct mt53fb_imagebuffer  t_crsr;
#ifdef CC_B2R44K2K_SUPPORT
    struct mt53fb_imagebuffer  m_B2RDfbBuf[B2R_BUFFER_NUM];
    struct B2R44K2K_Buffer     m_B2RSysBuf[B2R_BUFFER_NUM];
#endif  
#ifdef DFB_INPUT_FILER_EN

    DFBInputFilter input_filter;
#endif  
} MT53;

typedef struct {
     /* file descriptor from /dev/fb0 */
     int                    fd;

     /* mapping of shared command queue control structure and DMA region */
     volatile GFX_CMDQUE_T *shm_cmd_que;
     volatile UINT64       *cmd_que_buf;

     int num_plane;
     
     CoreScreen              *screen;     

     CoreLayer *layer1;
     CoreLayer *layer2;
     CoreLayer *layer3; 
     CoreLayer *layer_vdp;
     CoreLayer *cursor;
     CoreLayer *layer_vdp2;
#ifdef CC_B2R44K2K_SUPPORT
     CoreLayer *m_ContainerLayer;
#endif
     GLESFuncs  glesFunc;
} MT53DriverData;


#define PMX_MAIN    0 // VDP1
#define PMX_PIP     1 // VDP2
#define PMX_OSD1    2 // VDP3
#define PMX_OSD2    3 // VDP4
#define PMX_OSD3    4 // VDP5
#define PMX_MAX_INPORT_NS 5


#ifdef CC_HW_WINDOW_SUPPORT

typedef struct
{
    int magic;
}MT53RegionData;

#endif


typedef struct 
{
    MT53LayerID            layer;
    /*CC_DFB_SUPPORT_VDP_LAYER   */  
    int pitch;
    int height;
    unsigned char _3dmode;    
    unsigned int u4CropBottomOffset;
    unsigned int u4CropTopOffset;
    unsigned int u4CropLeftOffset;
    unsigned int u4CropRightOffset;    
#ifdef CC_HW_WINDOW_SUPPORT
    MT53RegionData *curregion;
#endif  
} MT53LayerData;

typedef struct 
{
     /* validation flags */
     int                    v_flags;

     /* cached/computed values */
     void                  *dst_addr;
     unsigned long          dst_phys;
     unsigned long          dst_pitch;
     DFBSurfacePixelFormat  dst_format;
     unsigned long          dst_bpp;

     void                  *src_addr;
     unsigned long          src_phys;
     unsigned long          src_pitch;
     DFBSurfacePixelFormat  src_format;
     unsigned long          src_bpp;

     DFBColor               color;
     unsigned long          color_pixel;

     u32                    bltopts;

     u32                porterduff;

     u32                       rotateopt;
     u32 src_cbcr_offset;
     u32 dst_cbcr_offset;       
     CoreSurfaceAccessFlags   src_access;
     CoreSurfaceAccessFlags   dst_access;
     GFX_MMU_T                mmu;
}MT53DeviceData;

//#ifdef GFX_IOMMU_IOVA_SUPPORT
typedef struct 
{
     int             magic;
     int             offset;
     int             pitch;
     int             size;
     void *          u4_addr;
	 void *          u4_alignaddr;
     unsigned int    u4_phy;
     Chunk          *chunk;
     unsigned int    u4_vdo_mem_type;
	 int   pid[E_DSSP_MAX];
     int             fd;        /* file descriptor access by dma_buf */
     DFB_IOMMU_PIN_RANGE_T pin_range[E_DSSP_MAX];
} MT53AllocationData;
//#endif

#define FBIOGET_DFB_DMA_BUF       0x461F
typedef enum
{
    FB_MEM_TYPE_NONE   = 0,
    FB_MEM_TYPE_DMABUF = 1,
    FB_MEM_TYPE_IOVA_DMABUF = 2
} fb_mem_type;
/* pay more attention to this strecture must match with FB driver */
struct dfb_mem_info {
	unsigned int fd;
	unsigned int flags;
	unsigned int phyaddr;
	unsigned int viraddr;
	unsigned int size;
	unsigned int type;
	unsigned int iova;
	unsigned int devidx;
	unsigned int reserved[3];
};

extern void mt53_gfx_set_mmu(GFX_MMU_T *pt_this, int fgPhyEn);

extern MT53 *dfb_mt53;

extern const SurfacePoolFuncs   mt53SurfacePoolFuncs;
extern /*const*/ DisplayLayerFuncs  mt53PrimaryLayerFuncs;
extern /*const*/ ScreenFuncs        mt53PrimaryScreenFuncs;
extern void system_mmap_crsr_addr(void);
extern void system_mmap_imagebuf_addr(void);
extern void system_munmap_imagebuf_addr(void);
extern void system_mmap_vdpbuf_addr(void);
extern void system_munmap_vdpbuf_addr(void);
extern void system_munmap_imagebuf2_addr(void);
extern void system_mmap_imagebuf2_addr(void);
extern void system_munmap_check(void);
extern void system_mmap_dfb_m4u_addr(void);
extern void system_mmap_crsr_m4u_addr(void);
extern void mt53_fill_rect(unsigned int u4_addr,unsigned int u4_pitch,unsigned int u4_h,int fgPhyEn);
extern int  system_mem_get_dmafd(unsigned int u4_phyaddr, unsigned int u4_size);
extern DFBResult dfb_surfacemanager_vdp_create( CoreDFB *core,unsigned int length,SurfaceManager **ret_manager );
extern DFBResult dfb_surfacemanager_vdp_allocate( CoreDFB *core,SurfaceManager *manager,CoreSurfaceBuffer *buffer,Chunk **ret_chunk );  
#ifdef CC_B2R44K2K_SUPPORT
extern void system_mmap_b2r44K2Kbuf_addr(void);
extern void system_munmap_b2r44K2Kbuf_addr(void);
#endif
extern int vdpGetCropRect(mt53fb_vdp_crop_rect * pt_vdp_crop_rect);
extern DFBResult mdfb_check_wait_vsync(CoreLayer* pt_this);
#endif
