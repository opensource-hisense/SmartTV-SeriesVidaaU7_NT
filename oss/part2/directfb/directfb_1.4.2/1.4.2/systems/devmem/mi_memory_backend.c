// <MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
// <MStar Software>

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <misc/conf.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_os.h"
#include "mi_osd.h"
#include "mi_disp.h"

#include "mtk_secure.h"

#include "mtk_iommu_dtv_api.h"


#define IOVA_ADDRESS_START                     0x200000000
#define IOVA_ADDRESS_END                       0x280000000
#define ERR_BUF_SIZE 256

static bool _mi_Pool_Init(void)
{
    static bool binit = false;
    if (!binit)
    {
        if ( access("/dev/shm/ota_flow_flag", F_OK) == 0 ) {
            //Init MI_SYS:
            MI_RESULT errCode = MI_ERR_FAILED;
            MI_SYS_InitParams_t stSysInitParams;
            memset(&stSysInitParams, 0, sizeof(MI_SYS_InitParams_t));
            stSysInitParams.stBoard.u32BoardInfoCnt = 0;
            stSysInitParams.stBoard.pstBoardInfo = 0;
            stSysInitParams.stBoard.u32Version = MI_SYS_BOARD_INFO_PREFIX_VERSION;
            stSysInitParams.pszMmapFileName = (MI_U8 *)"/application/MMAP_MI.h";
            stSysInitParams.stMmap.u32Version = MI_SYS_MMAP_PREFIX_VERSION;

            errCode = MI_SYS_Init(&stSysInitParams);

            if(errCode != MI_OK)
            {
                printf("\033[31m [DFB] Error 0x%x: failed to MI_SYS_Init()!\033[m\n", errCode);
            }

            // Init MI_DISP:
            MI_DISP_InitParams_t stDispInitParams;
            memset(&stDispInitParams, 0, sizeof(stDispInitParams));
            stDispInitParams.eDispTiming = E_MI_DISP_TIMING_1920X1080_50I;
            stDispInitParams.eTvSystem = E_MI_DISP_TV_SYSTEM_PAL;
            stDispInitParams.eDeFlicker = E_MI_DISP_DE_FLICKER_DEFAULT;

            errCode = MI_DISP_Init(&stDispInitParams);
            if (errCode != MI_OK && errCode != MI_HAS_INITED) {
                printf("[DFB] %s, MI_DISP_Init failed, Error : 0x%x\n", __FUNCTION__, errCode);
            }
        }
        else {
            //Init MI_SYS:
            MI_RESULT errCode = MI_ERR_FAILED;
            errCode = MI_SYS_Init(NULL);

            if(errCode != MI_OK)
            {
                printf("\033[31m [DFB] Error 0x%x: failed to MI_SYS_Init()!\033[m\n", errCode);
                //return false;
            }
        }

        binit = true;
    }

    return true;
}

static bool _mi_Pool_Get(void ** pAddrVirt, u32 * pu32AddrPhys, u32 * pu32Size, bool bNonCache)
{

    return true;
}

static bool _mi_Pool_Mapping(u8 u8MiuSel, hal_phy Offset, u32 u32MapSize, bool  bNonCache)
{
    MI_SYS_MmapLayout_t mmp_layout;
    memset(&mmp_layout, 0x0, sizeof(MI_SYS_MmapLayout_t));

    mmp_layout.eMemType = (bNonCache)? E_MI_SYS_MEM_TYPE_UNCACHED : E_MI_SYS_MEM_TYPE_MIN;
    mmp_layout.u32MemLen = u32MapSize;
    mmp_layout.phyMemAddr = (MI_PHY)Offset;

    D_INFO("[DFB] %s, %d, call MI_SYS_MappingMmapLayout pAddr:0x%llx, Size:0x%x, pid=%d\n", __FUNCTION__, __LINE__, Offset, u32MapSize, getpid());
    if (MI_SYS_MappingMmapLayout(E_MI_SYS_MMAP_ID_GOP_GWIN_RB, &mmp_layout) != MI_OK) {
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        printf("[DFB] %s, %d, call MI_SYS_MappingMmapLayout pAddr:0x%llx, Size:0x%x Failed errno=%s\n", __FUNCTION__, __LINE__, Offset, u32MapSize, ret);
        return false;
    }

    return true;
}

static void* _mi_Pool_PA2VANonCached(hal_phy pAddrPhys)
{
    static MI_VIRT pu32VAddr = 0;
    MI_OS_Pa2NonCachedVa(pAddrPhys, &pu32VAddr);
    D_INFO("[DFB] %s [pid=%d], pAddrPhys =0x%llx, pu32VAddr = %llx\n", __FUNCTION__, getpid(), pAddrPhys, pu32VAddr);

    return (void *)pu32VAddr;
}

static bool _mi_Pool_UnMapping(void* ptrVirtStart, u32  u32MapSize)
{
    MI_VIRT pu32VAddr = (MI_VIRT *)ptrVirtStart;
    MI_SYS_MmapLayout_t mmp_layout;
    memset(&mmp_layout, 0x0, sizeof(MI_SYS_MmapLayout_t));

    D_INFO("[DFB] %s, %d, VA:%p, Size:0x%x, ", __FUNCTION__, __LINE__, ptrVirtStart, u32MapSize);
    MI_OS_Va2Pa(pu32VAddr, &mmp_layout.phyMemAddr);
    mmp_layout.eMemType = E_MI_SYS_MEM_TYPE_UNCACHED;
    mmp_layout.u32MemLen = u32MapSize;

    D_INFO(" call MI_SYS_UnMappingMmapLayout, pAddr:0x%llx, pid=%d\n", mmp_layout.phyMemAddr, getpid());
    if (MI_SYS_UnMappingMmapLayout(E_MI_SYS_MMAP_ID_GOP_GWIN_RB, &mmp_layout) != MI_OK) {
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        printf("[DFB] %s, %d, call MI_SYS_MappingMmapLayout pAddr:0x%llx, Size:0x%x Failed errno=%s\n", __FUNCTION__, __LINE__, mmp_layout.phyMemAddr, u32MapSize, ret);
        return false;
    }

    return true;
}

#ifdef MI_FUSION
static bool _mi_MMA_Alloc(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle)
{
    MI_SYS_MmapLayout_t stMmapLayout;
    MI_OS_IommuAllocate_t stIommuAllocate;

    memset(&stMmapLayout,0,sizeof(stMmapLayout));
    if (MI_SYS_GetMmapLayout( E_MI_SYS_MMAP_ID_GOP_GWIN_RB, &stMmapLayout) != MI_OK) {
        printf("[DFB] %s, %d,  call MI_SYS_GetMmapLayout failed!\n", __FUNCTION__, __LINE__);
        return false;
    }

    D_INFO("[DFB] %s, %d, stMmapLayout.au8TagName = %s, pid=%d\n", __FUNCTION__, __LINE__, stMmapLayout.au8TagName, getpid());

    memset(&stIommuAllocate, 0, sizeof(MI_OS_IommuAllocate_t));

    if(strlen(stMmapLayout.au8TagName) >= sizeof(stIommuAllocate.au8TagName)){
        D_ERROR("[DFB] %s, %d, The size of stMmapLayout tag name is longer than stIommuAllocate tag name\n", __FUNCTION__, __LINE__);
        return false;
    }
    strncpy(stIommuAllocate.au8TagName,  stMmapLayout.au8TagName, strlen(stMmapLayout.au8TagName));
    
    stIommuAllocate.u32Size = u32size;
    stIommuAllocate.eMmaMappingType = E_MI_OS_MAPPING_TYPE_NONE;

    if (MI_OS_AllocateIommuMemory( &stIommuAllocate ) != MI_OK ) {
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        printf("[DFB] %s, %d, call MI_OS_AllocateIommuMemory failed! errno=%s pid=%d\n", __FUNCTION__, __LINE__, ret, getpid());
        return false;
    }

    D_INFO("[DFB] %s, %d, phyAddr = 0x%llx, size=%d, pid=%d\n", __FUNCTION__, __LINE__, stIommuAllocate.phyAddr, u32size, getpid());
    *pPhy = stIommuAllocate.phyAddr;

    return true;
}
#else
static bool _mi_MMA_Alloc(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle)
{
    MI_OS_IommuAllocate_t stIommuAllocate;

    memset(&stIommuAllocate, 0, sizeof(MI_OS_IommuAllocate_t));

    strncpy(stIommuAllocate.au8TagName,  u8bufTag, strlen(u8bufTag));
    stIommuAllocate.u32Size = u32size;
    stIommuAllocate.eMmaMappingType = E_MI_OS_MAPPING_TYPE_NONCACHE;
    printf("[DFB] %s, %d, stIommuAllocate.au8TagName = %s, pid=%d\n", __FUNCTION__, __LINE__, stIommuAllocate.au8TagName, getpid());

    if (MI_OS_AllocateIommuMemory( &stIommuAllocate ) != MI_OK ) {
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        printf("[DFB] %s, %d, call MI_OS_AllocateIommuMemory failed! errno=%s pid=%d\n", __FUNCTION__, __LINE__, ret, getpid());
        return false;
    }

    printf("[DFB] %s, %d, phyAddr = 0x%llx, size=%d, pid=%d\n", __FUNCTION__, __LINE__, stIommuAllocate.phyAddr, u32size, getpid());
    *pPhy = stIommuAllocate.phyAddr;

    return true;
}
#endif

static bool _mi_MMA_Init(void)
{
    // default return true on MI.
    return true;
}

static bool _mi_MMA_Free(hal_phy Phy_addr, u32 u32size)
{
    MI_OS_IommuFree_t stIommuFree;

    memset(&stIommuFree, 0, sizeof(MI_OS_IommuFree_t));

    stIommuFree.phyAddr = Phy_addr;
    stIommuFree.u32Size = u32size;

    D_INFO("[DFB] %s, %d, call MI_OS_FreeIommuMemory, phyAddr = 0x%llx, size=%d, pid=%d\n", __FUNCTION__, __LINE__, stIommuFree.phyAddr, stIommuFree.u32Size, getpid());
    if ( MI_OS_FreeIommuMemory (&stIommuFree) != MI_OK ) {
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        printf("[DFB] %s, %d, call MI_OS_FreeIommuMemory failed! errno=%s pid=%d\n", __FUNCTION__, __LINE__, ret, getpid());
        return false;
    }
    D_INFO("[DFB] %s, %d, call MI_OS_FreeIommuMemory, done\n", __FUNCTION__, __LINE__ );
    return true;
}

static bool _mi_MMA_Exit(void)
{
    // default return true on MI.
    return true;
}


static bool _mi_MMA_IsMMAEnabled(void)
{
    static bool bEnabledInitial = false;

    if(bEnabledInitial == false) {

         // default return true on MI.
        _mi_Pool_Init();

        // check MI to query MMAP for GOP_GWIN_RB.
        MI_SYS_MmapLayout_t stMmapLayout;

        memset(&stMmapLayout,0,sizeof(stMmapLayout));
        if (MI_SYS_GetMmapLayout( E_MI_SYS_MMAP_ID_GOP_GWIN_RB, &stMmapLayout) != MI_OK) {

            printf("[DFB] %s, %d,  call MI_SYS_GetMmapLayout failed!\n", __FUNCTION__, __LINE__);

        } else {

            D_INFO("%s, E_MI_SYS_MMAP_ID_GOP_GWIN_RB, CoBufLayer : %x, addr=0x%llx, len=%x\n", __FUNCTION__,
stMmapLayout.eCoBufLayer, stMmapLayout.phyMemAddr, stMmapLayout.u32MemLen);

            dfb_config->video_phys_hal = stMmapLayout.phyMemAddr;
            dfb_config->video_length = stMmapLayout.u32MemLen;
        }


        bEnabledInitial = true;

    }

    // if MMAP layout GOP_GWIN_RB length, not use iommu.
    return (dfb_config->video_length == 0) ? true : false;


}

static bool _mi_MMA_GetMIUIndex(hal_phy Phy_addr, int *pIndex)
{
    //D_UNIMPLEMENTED();
    return true;
}

static bool _mi_MMA_GetBufFd(hal_phy Phy_addr, int *pfd)
{
    MI_OS_IommuBufFd_t stBufFdParams = {0};
    stBufFdParams.phyAddr = Phy_addr;
    if (MI_OS_IommuOperation(E_MI_OS_IOMMU_OPERATION_TYPE_GET_BUFFD, &stBufFdParams) == MI_OK) {
        *pfd = stBufFdParams.fd;
        D_INFO("%s *pfd: %d\n", __FUNCTION__,*pfd);
        return true;
    } else {
        *pfd = -1;
        char errbuf[ERR_BUF_SIZE] = {0};
        char *ret = strerror_r(errno, errbuf, sizeof(errbuf));
        D_INFO("%s [DFB] get dma buf fd fail! errno=%s pid=%d\n", __FUNCTION__, ret, getpid());
        return false;
    }

}

static bool _mi_MMA_IsIOVA_Address(u64 busAddr)
{
      if(busAddr >= IOVA_ADDRESS_START && busAddr <= IOVA_ADDRESS_END)
          return true;
      else
          return false;
}

#if USE_MSTAR_CMA
static DirectResult _mi_CMA_MapMem(void * input_data, unsigned long  mem_phys,
                                           unsigned int mem_length, unsigned long  reg_phys,
                                           unsigned int reg_length, bool secondary)
{
    D_UNIMPLEMENTED();
    return false;
}

static void* _mi_CMA_GetMem(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  length)
{
    D_UNIMPLEMENTED();
    return false;
}

static bool _mi_CMA_PutMem(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  length)
{
    D_UNIMPLEMENTED();
    return false;
}
#endif
static unsigned long long _mi_MsOSPA2BA(hal_phy pAddrPhys)
{
    MI_VIRT pphyBusAddr = 0;
    MI_OS_Pa2Ba(pAddrPhys, &pphyBusAddr);
    D_INFO("[DFB1] %s (%s) pAddrPhys =0x%llx, pphyBusAddr =0x%llx \n", __FUNCTION__, __FILE__, pAddrPhys, pphyBusAddr);
    return pphyBusAddr;
}

static bool _mi_MMA_Secure_LockBuf(char *buftag, hal_phy u64Addr, u32 size, u32 u32PipelineId)
{
    bool result = true;
#if 0
    MI_OSD_GetLayerHandleParams_t pstQueryParams;
    MI_HANDLE hLayer;
    MI_OSD_MMAAuthorizeParams_t AuthorizeParams;

    AuthorizeParams.phyAddr = u64Addr;
    AuthorizeParams.u32Size = size;
    AuthorizeParams.u32PipeID = u32PipelineId;
    memcpy( AuthorizeParams.au8BufferTag, buftag, sizeof(AuthorizeParams.au8BufferTag));

    D_INFO("[DFB] %s, phyAddr:0x%llx, Size:%d, PipeID:%x\n", __FUNCTION__, AuthorizeParams.phyAddr, AuthorizeParams.u32Size, AuthorizeParams.u32PipeID);

    pstQueryParams.eLayerId = E_MI_OSD_LAYER_0;
    if ( MI_OSD_GetLayerHandle( &pstQueryParams, &hLayer) != MI_OK ) {
        printf("[DFB] %s, MI_OSD_GetLayerHandle failed!\n", __FUNCTION__);
        return false;
    }

    if (MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MMA_Authorize, (void*)(&AuthorizeParams)) != MI_OK) {
        printf("[DFB] %s, MI_OSD_LayerSetAttr failed!\n", __FUNCTION__);
        return false;
    }
#else

#define BUF_TAG (14 << 24)

    int ret;
    MI_OS_IommuBufFd_t stBufFdParams = {0};

    struct mdiommu_ioc_data data;
    memset(&data, 0, sizeof(data));

    stBufFdParams.phyAddr = u64Addr;
    // get fd.
    if (MI_OS_IommuOperation(E_MI_OS_IOMMU_OPERATION_TYPE_GET_BUFFD, &stBufFdParams) == MI_OK) {
        data.fd = stBufFdParams.fd;
        DBG_SECURE_MSG("[DFB] %s, get fd: %d\n", __FUNCTION__, data.fd);
    }
    else {        
        D_ERROR("[DFB] %s, get dma buf fd fail!\n", __FUNCTION__);
    }

    int fd_dd = open(MDIOMMU_DEV_NAME, O_RDWR);

    //set buf_tag, directfb_frame0
    data.flag = BUF_TAG;
    data.len  = size;
    data.addr = u64Addr;
    data.pipelineid = u32PipelineId;

    ret = ioctl(fd_dd, MDIOMMU_IOC_AUTHORIZE, &data);
    if(ret < 0)
    {
        D_ERROR("[DFB][%s] MDIOMMU_IOC_AUTHORIZE failed, addr:0x%llx, ret: %d\n",__FUNCTION__, data.addr, ret);
        result = false;
    }

    // release fd.
    if (MI_OS_IommuOperation(E_MI_OS_IOMMU_OPERATION_TYPE_PUT_BUFFD, &stBufFdParams) != MI_OK) {
        D_ERROR("[DFB] %s, put fd: %d failed!\n", __FUNCTION__, stBufFdParams.fd);
    }

    close(fd_dd);

    _dfb_secure_buf_lock();
#endif
    DBG_SECURE_MSG("[DFB] %s, done\n", __FUNCTION__);
    return result;
}

static bool _mi_MMA_Secure_Unlock(char *buftag, hal_phy u64Addr)
{
#if 0
    MI_OSD_GetLayerHandleParams_t pstQueryParams;
    MI_HANDLE hLayer = NULL;
    MI_OSD_MMAAuthorizeParams_t AuthorizeParams;

    AuthorizeParams.phyAddr = u64Addr;
    AuthorizeParams.u32PipeID = 0;
    memcpy( AuthorizeParams.au8BufferTag, buftag, sizeof(AuthorizeParams.au8BufferTag));

    D_INFO("[%s]phyAddr:0x%llx, Size:%d, PipeID:%x\n", __FUNCTION__, AuthorizeParams.phyAddr, AuthorizeParams.u32Size, AuthorizeParams.u32PipeID);

    pstQueryParams.eLayerId = E_MI_OSD_LAYER_0;    
    if ( MI_OSD_GetLayerHandle( &pstQueryParams, &hLayer) != MI_OK ) {
        printf("[DFB] %s, MI_OSD_GetLayerHandle failed!\n", __FUNCTION__);
        return false;
    }

    if (MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MMA_Authorize, (void*)(&AuthorizeParams)) != MI_OK) {
        printf("[DFB] %s, MI_OSD_LayerSetAttr failed!\n", __FUNCTION__);
        return false;
    }
#else

// when free buffer, un-authorize at the same time.
/* 
    int ret;
    
    struct mdiommu_ioc_data data;
    memset(&data, 0, sizeof(data));

    int fd_dd = open(MDIOMMU_DEV_NAME, O_RDWR);

    _mi_MMA_GetBufFd(u64Addr, &data.fd);
	
    data.addr = u64Addr;

    ret = ioctl(fd_dd, MDIOMMU_IOC_UNAUTHORIZE, &data);
    if(ret < 0)
    {
        D_ERROR("[DFB][%s] MDIOMMU_IOC_UNAUTHORIZE Failed, addr:0x%llx, result = %d!\n", __FUNCTION__, data.addr, ret);
        close(fd_dd);
        return false;
    }

    close(fd_dd);*/

    _dfb_secure_buf_unlock();
#endif
    DBG_SECURE_MSG("[DFB] %s, done\n", __FUNCTION__);

    return true;
}

static bool _mi_MMA_Secure_GetPipeLineId(u32 *u32PipelineId)
{

    if (_dfb_secure_get_pipeline_id(u32PipelineId) != DFB_OK) {
        printf("[DFB] %s, get pipeline id failed!\n", __FUNCTION__);
        return false;
    }

    DBG_SECURE_MSG("[DFB] %s, get pipeline Id = %x, pid=%d\n", __FUNCTION__, *u32PipelineId, getpid());
    return true;
}

static IDirectFBInternalMemBackend mi_memory_backend =
{
    .Init = _mi_Pool_Init,
    .Get = _mi_Pool_Get,
    .Mapping = _mi_Pool_Mapping,
    .PA2VANonCached = _mi_Pool_PA2VANonCached,
    .UnMapping = _mi_Pool_UnMapping,
    .MMA_Alloc = _mi_MMA_Alloc,
    .MMA_Init = _mi_MMA_Init,
    .MMA_Free = _mi_MMA_Free,
    .MMA_Exit = _mi_MMA_Exit,
    .MMA_IsMMAEnabled = _mi_MMA_IsMMAEnabled,
    .MMA_GetMIUIndex = _mi_MMA_GetMIUIndex,
    .MMA_GetBufFd = _mi_MMA_GetBufFd,
    .MMA_IsIOVA_Address = _mi_MMA_IsIOVA_Address,
#if USE_MSTAR_CMA
    .CMA_MapMem = _mi_CMA_MapMem,
    .CMA_GetMem = _mi_CMA_GetMem,
    .CMA_PutMem = _mi_CMA_PutMem,
#endif
    .MsOSPA2BA = _mi_MsOSPA2BA,
    .MMA_Secure_LockBuf = _mi_MMA_Secure_LockBuf,
    .MMA_Secure_UnlockBuf = _mi_MMA_Secure_Unlock,    
    .MMA_Secure_GetPipeLineId = _mi_MMA_Secure_GetPipeLineId,
};


__attribute__((constructor)) static void registerMemoryBackend(void)
{
    D_INFO("[DFB] %s (%s)\n", __FUNCTION__, __FILE__);
    dfb_SetMemBackend(&mi_memory_backend);
}
