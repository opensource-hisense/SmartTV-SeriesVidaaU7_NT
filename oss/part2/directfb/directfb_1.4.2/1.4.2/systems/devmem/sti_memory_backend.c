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
#include <config.h>
#include <misc/conf.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/list.h>

#include "mtk_iommu.h"
#include "dma-heap.h"
#include "mtk_iommu_dtv_api.h"

#define BUF_TAG_UNCACHEDD_HEAP_DEV "/dev/dma_heap/mtk_disp_directfb_uncached"

#define IOVA_ADDRESS_START                     0x200000000
#define IOVA_ADDRESS_END                       0x280000000

#define MERGE_IOVA_DMABUFID(iova, dmabuf_id) ((_HalAddrToBusAddr(iova) << 28) |dmabuf_id)

#define _IOVA( PHY ) (_BusAddrToHalAddr(PHY >> 28) &0x2FFFFFFFF)

#define QUERY_CNT 32
#define MASK 0xFFFFFFFF
#define BUF_TAG (14 << 24)
#define OFFSET_MASK 0xFFFFFFFF


static int ion_fd = -1;
static int dma_fd = -1;
static int fd_dd = -1;
static int iommu_heap_id = -1;

static bool bUseDmaHeap = false;

//for save fd and vaddr relationship.
typedef struct {
     DirectLink             link;

     u32	fd;
     u64     iova;
     u32     vaddr;
     u32     dmabuf_id;
} STIIOMMUEntry;

static DirectLink * sti_iommu_entry = NULL;

static DirectMutex   sti_iommu_lock;


static __inline__ STIIOMMUEntry *
lookup_iova( u64 iova, bool bva )
{
     STIIOMMUEntry *entry = NULL;

     direct_list_foreach (entry, sti_iommu_entry) {
          if ( entry->iova == iova ) {
              if ((bva && entry->vaddr !=NULL) || (!bva && entry->vaddr == NULL)) {
		  D_INFO("[DFB] %s, entry(%p) iova=%llx\n", __FUNCTION__, entry, iova);
                  return entry;
              }
          }
     }

     return NULL;
}

static __inline__ STIIOMMUEntry *
lookup_va( u32 va )
{
    STIIOMMUEntry *entry = NULL;

    if (va == NULL)
        return NULL;

    direct_list_foreach (entry, sti_iommu_entry) {
        if ( entry->vaddr == va ) {
             D_INFO("[DFB] %s, entry(%p) va=%x\n", __FUNCTION__, entry, va);
             return entry;
        }
    }

    return NULL;
}

void
add_iommu( u32 fd, u32 dmabuf_id, u64 iova, u32 va )
{
     STIIOMMUEntry *entry = NULL;

     if (direct_mutex_lock( &sti_iommu_lock ))
          return;

     entry = lookup_iova( iova, (va)? true : false );
     if (!entry) {
          direct_mutex_unlock( &sti_iommu_lock );

          entry = (STIIOMMUEntry*) direct_calloc( 1, sizeof(STIIOMMUEntry) );
          if (!entry) {
               D_WARN( "out of memory" );
               return;
          }

          entry->fd = fd;
          entry->dmabuf_id = dmabuf_id;
          entry->iova = iova; 
          entry->vaddr = va;

          D_INFO("[DFB] %s, new entry(%p), fd=%d, dmabuf_id=%d, iova=%llx, va=%x\n", __FUNCTION__, entry, fd, dmabuf_id, iova, va);
          if (direct_mutex_lock( &sti_iommu_lock )) {
               printf("[DFB] %s, sti_iommu_lock failed!\n", __FUNCTION__);
               direct_free( entry );
               return;
          }

          direct_list_prepend( &sti_iommu_entry, &entry->link );
     }

     direct_mutex_unlock( &sti_iommu_lock );
}


static int ion_ioctl(u32 buf_tag, u64 size, int* fd, u32 flag)
{
    int ret;
    struct ion_allocation_data data = {0};

    if (fd == NULL) {
        printf("[DFB] %s, fd is NULL\n", __FUNCTION__);
        return -1;
    }

    data.len = size;
    //iommu heap
    data.heap_id_mask = 1 << iommu_heap_id;
    data.flags = flag | buf_tag;

    ret = ioctl(ion_fd, ION_IOC_ALLOC, &data);
    if (ret < 0) {
        printf("[DFB] %s, Failed! ret = %d!\n", __FUNCTION__, ret);
        return ret;
    }

    *fd = data.fd;
    return 0;
}

bool init_ion(void)
{
    struct ion_heap_data *data = NULL;
    struct ion_heap_query query;
    int ret, i;

    ion_fd = open(ION_DEV_NAME, O_RDWR);
    if (ion_fd < 0) {
        printf("[DFB] %s, %s open fail! errno=%d\n", __FUNCTION__, ION_DEV_NAME, errno);
        return false;
    }
    D_INFO("[DFB] %s, open ion device,  ion_fd = %d\n", __FUNCTION__, ion_fd);

    memset(&query, 0, sizeof(struct ion_heap_query));
    query.cnt = QUERY_CNT;
    data = (struct ion_heap_data *)malloc(sizeof(struct ion_heap_data) * query.cnt);
    if (!data) {
        D_OOM();
        return false;
    }
    memset(data, 0, sizeof(struct ion_heap_data) * query.cnt);
    query.heaps = (uintptr_t)data;

    ret = ioctl(ion_fd, ION_IOC_HEAP_QUERY, &query);
    if(ret < 0){
        free(data);
        printf("[DFB] %s, Failed, result = %d!\n",__FUNCTION__, ret);
        return false;
    }
    for(i = 0;i < query.cnt; i++){
        /*printf("[DFB] %s, name=%s, type=%d, heapid=%d\n", __FUNCTION__,
                      data[i].name,data[i].type,data[i].heap_id);*/

        ret = strcmp(data[i].name, "ion_mtkdtv_iommu_heap");
        if(ret == 0){
            iommu_heap_id = data[i].heap_id;
            printf("[DFB] %s, iommu_heap_id = %d!\n",__FUNCTION__, iommu_heap_id);
            break;
        }
    }
    free(data);

    if(iommu_heap_id < 0){
        printf("[DFB] %s, Failed, iommu_heap_id = %d!\n",__FUNCTION__, iommu_heap_id);
        return false;
    }

    return true;
}

bool init_dma_heap(void)
{
    dma_fd = open(BUF_TAG_UNCACHEDD_HEAP_DEV, O_RDONLY | O_CLOEXEC);
    if (dma_fd < 0) {
        printf("[DFB] %s, %s open fail! errno=%d\n", __FUNCTION__, BUF_TAG_UNCACHEDD_HEAP_DEV, errno);
        return false;
    }
    D_INFO("[DFB] %s, open dma_heap device %s,  dma_fd = %d\n", __FUNCTION__, BUF_TAG_UNCACHEDD_HEAP_DEV, dma_fd);

    return true;
}

static bool _sti_Pool_Init(void)
{
    static bool binit = false;
    if (!binit)
    {
        bool ret = init_dma_heap();
        if (ret == false) {
            ret = init_ion();
            if (ret == false) {
                printf("[DFB] %s, no dma-heap or ion device!\n", __FUNCTION__);
                return false;
            }
            bUseDmaHeap = false;
        }
        else
             bUseDmaHeap = true;

        fd_dd = open(MDIOMMU_DEV_NAME, O_RDWR);
        if (fd_dd < 0){
            printf("[DFB] %s, iommuDD open fail! errno=%d\n", __FUNCTION__, errno);
            close(bUseDmaHeap? dma_fd : ion_fd);
            return false;
        }
        D_INFO("[DFB] %s, open iommu device,  fd_dd = %d\n", __FUNCTION__, fd_dd);


        binit = true;

        direct_mutex_init( &sti_iommu_lock );
        D_INFO("[DFB] %s, done!, pid=%d\n", __FUNCTION__, getpid());
    }

    return true;
}

static bool _sti_Pool_Get(void ** pAddrVirt, u32 * pu32AddrPhys, u32 * pu32Size, bool bNonCache)
{

    return true;
}


static bool _sti_Pool_Mapping(u8 u8MiuSel, hal_phy Offset, u32 u32MapSize, bool  bNonCache)
{    
    int fd = -1, ret = 0;
    u32 pu32VAddr;
    struct mdiommu_ioc_data data;

    D_INFO("[DFB] %s, offset = %llx, map size = 0x%x, pid=%d\n", __FUNCTION__, Offset, u32MapSize, getpid());

    memset(&data, 0, sizeof(data));
    data.addr = _IOVA(Offset) ;
    data.fd = -1;
    data.dmabuf_id = (u32)(Offset & OFFSET_MASK);

    D_INFO("[DFB] %s, iova = %llx, dmabuf_id = %d\n", __FUNCTION__, data.addr, data.dmabuf_id);

    ret = ioctl(fd_dd, MDIOMMU_IOC_GETFD, &data);
    if(ret < 0){
        printf("[DFB]%s, MDIOMMU_IOC_GETFD Failed, result = %d!\n", __FUNCTION__, ret);
        ret = -1;
    }	
    fd = data.fd;

    if ( u32MapSize > data.len ) {
        printf("[DFB] mapping size bigger then iommu buffer\n");
    }

    //get va for user
    pu32VAddr = mmap(NULL, u32MapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(MAP_FAILED == pu32VAddr){
        printf("[DFB]%s, mmap Failed\n",__FUNCTION__ );
        close(fd);
        return false;
    }

    // keep in link.
    add_iommu( fd, data.dmabuf_id, data.addr, pu32VAddr );

    D_INFO("[DFB] %s, done! fd=%d, dmabuf_id=%d, iova=0x%llx, va=0x%x, map size=0x%x, pid=%d\n",
		__FUNCTION__, fd, data.dmabuf_id, data.addr, pu32VAddr, u32MapSize, getpid());
    return true;
}

static void* _sti_Pool_PA2VANonCached(hal_phy pAddrPhys)
{
    STIIOMMUEntry *entry;
    u32 pu32VAddr;
    hal_phy iova = _IOVA(pAddrPhys);

    if (direct_mutex_lock( &sti_iommu_lock ))
         return NULL;

    entry = lookup_iova( iova, true );

    if (!entry) {
        direct_mutex_unlock( &sti_iommu_lock );
        return NULL;
    }

    direct_mutex_unlock( &sti_iommu_lock );

    pu32VAddr = entry->vaddr;

    D_INFO("[DFB] %s, pAddrPhys=0x%llx, pu32VAddr=0x%x, pid=%d\n",
		__FUNCTION__, iova, pu32VAddr, getpid());

    return (void *)pu32VAddr;
}

static bool _sti_Pool_UnMapping(void* ptrVirtStart, u32  u32MapSize)
{
    int fd;
    STIIOMMUEntry *entry;

    if (direct_mutex_lock( &sti_iommu_lock ))
         return NULL;

    entry = lookup_va( ptrVirtStart );
    if (!entry) {
        direct_mutex_unlock( &sti_iommu_lock );
        return NULL;
    }

    fd = entry->fd;
    direct_list_remove( &sti_iommu_entry, &entry->link );

    direct_mutex_unlock( &sti_iommu_lock );

    D_INFO("[DFB] %s, fd=%d, va=%p, size=%x!\n", __FUNCTION__, fd, ptrVirtStart, u32MapSize);
    //unmap va
    munmap( ptrVirtStart, u32MapSize );

    close( fd );

    D_INFO("[DFB] %s, done! pid=%d\n", __FUNCTION__, getpid());
    return true;
}

static bool _sti_IOMMU_Alloc(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle)
{
    int ret = 0;
    struct mdiommu_ioc_data data;

    memset(&data, 0, sizeof(data));

    if (bUseDmaHeap ) {
        struct dma_heap_allocation_data heap_data = {0};

        heap_data.len = u32size;
        heap_data.fd_flags = O_RDWR | O_CLOEXEC;

        //allocate buffer
        if(ioctl(dma_fd, DMA_HEAP_IOCTL_ALLOC, &heap_data) < 0){
            printf("[DFB]%s, dma-heap alloc Failed, fd=%d,  errno %s, pid=%d\n",__FUNCTION__, dma_fd,  strerror(errno), getpid() );
            return false;
        }
        D_INFO("[DFB] %s, dma-heap alloc size = 0x%x, fd=%d\n", __FUNCTION__, u32size, heap_data.fd);
        data.fd = heap_data.fd;
    }
    else {
        int fd = -1;
        u32 flag = 0;
        u32 buf_tag = 0;

        //set buf_tag, directfb_frame0
        buf_tag = BUF_TAG;

        //if buffer need cached, if not setting flag is non-cached.
        //flag = ION_FLAG_CACHED;

        //allocate buffer
        ret = ion_ioctl(buf_tag, u32size, &fd, flag);
        if(ret){
            printf("[DFB]%s, Failed\n",__FUNCTION__);
            return false;
        }

        data.fd = fd;
    }

    // get iova
    ret = ioctl(fd_dd, MDIOMMU_IOC_GETIOVA, &data);
    if(ret < 0){
        printf("[DFB]%s, MDIOMMU_IOC_GETIOVA Failed, result = %d!\n", __FUNCTION__, ret);
        return false;
    }
    D_INFO("[DFB]%s, Get iova = %llx\n", __FUNCTION__, data.addr);

    //set IOMMU SHARE
    ret = ioctl(fd_dd, MDIOMMU_IOC_SHARE, &data);
    if(ret < 0){
        printf("[DFB]%s, MDIOMMU_IOC_SHARE Failed, result = %d!\n", __FUNCTION__, ret);
        return false;
    }

    // keep in link.
    add_iommu( data.fd, data.dmabuf_id,	data.addr, NULL );

    // iova addr and dmabuf id
    *pPhy = MERGE_IOVA_DMABUFID(data.addr, data.dmabuf_id);
    
    D_INFO("[DFB] %s, done! fd=%d, dmabuf_id=%d, addr=%llx, pid=%d\n", __FUNCTION__, data.fd, data.dmabuf_id, data.addr, getpid());
    return true;
}

static bool _sti_IOMMU_Free(hal_phy Phy_addr, u32 u32size)
{
    int fd = -1, ret = 0;
    struct mdiommu_ioc_data data;
    STIIOMMUEntry *entry;
    hal_phy iova = _IOVA(Phy_addr);

    D_INFO("[DFB] %s, start! phy_addr=%llx, size=%x\n", __FUNCTION__, iova, u32size);

    memset(&data, 0, sizeof(data));

    if (direct_mutex_lock( &sti_iommu_lock ))
         return NULL;

    // find iova list and remove from list.
    entry = lookup_iova( iova, false );
    if (!entry) {
        printf("[DFB] %s, can't find iova(%llx) from list\n", __FUNCTION__, iova);
        direct_mutex_unlock( &sti_iommu_lock );
        return NULL;
    }

    data.dmabuf_id = entry->dmabuf_id;
    data.addr = entry->iova;

    //set IOMMU delete share
    ret = ioctl(fd_dd, MDIOMMU_IOC_DELETE, &data);
    if(ret < 0){
        printf("[DFB]%s, MDIOMMU_IOC_DELETE Failed, result = %d!\n", __FUNCTION__, ret);
        ret = -1;
    }
    
    fd = entry->fd;
    if (fd)
        close(fd);

    direct_list_remove( &sti_iommu_entry, &entry->link );

    direct_mutex_unlock( &sti_iommu_lock );

    D_INFO("[DFB] %s, free iommu buffer, done! fd=%d, addr=%llx, pid=%d\n", __FUNCTION__, fd, iova, getpid());
	
    return true;
}

static bool _sti_IOMMU_Init(void)
{
    // default return true on MI.
    return true;
}

static bool _sti_IOMMU_Exit(void)
{
    // close ion fd or dma-heap fd.
    if (bUseDmaHeap) {
        if (dma_fd >= 0 )
            close(dma_fd);
    }
    else {
        if (ion_fd >= 0 )
            close(ion_fd);
    }

    // close iommu fd
    if (fd_dd >= 0 )
        close(fd_dd);

    direct_mutex_deinit( &sti_iommu_entry );

    return true;
}


static bool _sti_IOMMU_IsMMAEnabled(void)
{
    static bool bEnabledInitial = false;

    if(bEnabledInitial == false) {

        _sti_Pool_Init();

        bEnabledInitial = true;

    }

    // if MMAP layout GOP_GWIN_RB length, not use iommu.
    return (dfb_config->video_length == 0) ? true : false;


}

static bool _sti_IOMMU_GetMIUIndex(hal_phy Phy_addr, int *pIndex)
{
    //D_UNIMPLEMENTED();
    return true;
}

static bool _sti_IOMMU_GetBufFd(hal_phy Phy_addr, int *pfd)
{
    int ret = 0;
    struct mdiommu_ioc_data data;

    memset(&data, 0, sizeof(data));
    data.dmabuf_id = (u32)(Phy_addr & MASK);
    data.addr = _IOVA(Phy_addr);
    data.fd = -1;

    D_INFO("[DFB] %s, phy_addr = 0x%llx, dmabuf_id = %d\n", __FUNCTION__, data.addr, data.dmabuf_id);
    // todo : IOCTL_IOMMU_GETFD(IOVA)
    ret = ioctl(fd_dd, MDIOMMU_IOC_GETFD, &data);
    if(ret < 0){
        printf("[DFB]%s, MDIOMMU_IOC_GETFD Failed, result = %d!\n", __FUNCTION__, ret);
        *pfd = -1;
        return false;
    }

    *pfd = data.fd;
    D_INFO("[DFB] %s, get fd = %d\n", __FUNCTION__, data.fd);
    return true;
}

static bool _sti_IOMMU_IsIOVA_Address(u64 busAddr)
{
      if(busAddr >= IOVA_ADDRESS_START && busAddr <= IOVA_ADDRESS_END)
          return true;
      else
          return false;
}


static unsigned long long _sti_MsOSPA2BA(hal_phy pAddrPhys)
{
    unsigned long long pphyBusAddr = pAddrPhys ;//+ 0x20000000;

    D_INFO("[DFB1] %s (%s) pAddrPhys =0x%llx, pphyBusAddr =0x%llx \n", __FUNCTION__, __FILE__, pAddrPhys, pphyBusAddr);

    return pphyBusAddr;
}

static bool _sti_IOMMU_GetIOVA(u32 Phy_addr, u64 *iova)
{
    int ret = 0;
    struct mdiommu_ioc_data data;

    memset(&data, 0, sizeof(data));
    data.fd = -1;
    data.dmabuf_id = Phy_addr & MASK;

    D_INFO("[DFB] %s, dmabuf_id = %d\n", __FUNCTION__, data.dmabuf_id);

    // get fd
    ret = ioctl(fd_dd, MDIOMMU_IOC_GETFD, &data);
    if(ret < 0){
        D_ERROR("[DFB]%s, MDIOMMU_IOC_GETFD Failed, result = %d!\n", __FUNCTION__, ret);
        return false;
    }
     // get iova
    ret = ioctl(fd_dd, MDIOMMU_IOC_GETIOVA, &data);
    if(ret < 0){
        D_ERROR("[DFB]%s, MDIOMMU_IOC_GETIOVA Failed, result = %d!\n", __FUNCTION__, ret);
        return false;
    }
    D_INFO("[DFB]%s, Get iova = 0x%llx\n", __FUNCTION__, data.addr);
    close(data.fd);
    *iova = data.addr;

    return true;
}

static bool _sti_IOMMU_Secure_Lock(hal_phy u64Addr, u32 size, u32 u32PipelineId)
{
    STIIOMMUEntry *entry;
    int ret;
    struct mdiommu_ioc_data data;
    memset(&data, 0, sizeof(data));
    hal_phy iova = _IOVA(u64Addr);

    if (direct_mutex_lock( &sti_iommu_lock ))
         return NULL;

    // find iova list and remove from list.
    entry = lookup_iova( iova, false );
    if (!entry) {
        printf("[DFB] %s, can't find iova(%llx) from list\n", __FUNCTION__, iova);
        direct_mutex_unlock( &sti_iommu_lock );
        return NULL;
    }

    data.fd = entry->fd;
    direct_mutex_unlock( &sti_iommu_lock );

    //set buf_tag, directfb_frame0
    data.flag = BUF_TAG;
    data.len  = size;
    data.addr = iova;
    data.pipelineid = u32PipelineId;

    ret = ioctl(fd_dd, MDIOMMU_IOC_AUTHORIZE, &data);
    if(ret < 0)
    {
        D_ERROR("[DFB][%s] MDIOMMU_IOC_AUTHORIZE failed, addr:0x%lx, ret: %d\n",__FUNCTION__, data.addr, ret);
        return false;
    }

    return true;
}

static bool _sti_IOMMU_Secure_Unlock(hal_phy u64Addr)
{
    STIIOMMUEntry *entry;
    int ret;
    struct mdiommu_ioc_data data;
    hal_phy iova = _IOVA(u64Addr);

    memset(&data, 0, sizeof(data));

    if (direct_mutex_lock( &sti_iommu_lock ))
         return NULL;

    // find iova list and remove from list.
    entry = lookup_iova( iova, false );
    if (!entry) {
        printf("[DFB] %s, can't find iova(%llx) from list !\n", __FUNCTION__, iova);
        direct_mutex_unlock( &sti_iommu_lock );
        return NULL;
    }

    data.fd = entry->fd;
    direct_mutex_unlock( &sti_iommu_lock );
  
    data.addr = iova;
    ret = ioctl(fd_dd, MDIOMMU_IOC_UNAUTHORIZE, &data);
    if(ret < 0)
    {
        D_ERROR("[DFB][%s] MDIOMMU_IOC_UNAUTHORIZE Failed, result = %d!\n", __FUNCTION__, ret);
        return false;
    }

    return true;
}

static bool _sti_IOMMU_Secure_GetPipeLineId(u32 *u32PipelineId)
{

    if (_dfb_secure_get_pipeline_id(u32PipelineId) != DFB_OK) {
        printf("[DFB] %s, get pipeline id failed!\n", __FUNCTION__);
        return false;
    }

    D_INFO("[DFB] %s, get pipeline Id = %x, pid=%d\n", __FUNCTION__, *u32PipelineId, getpid());
    return true;
}

static IDirectFBInternalMemBackend sti_memory_backend =
{
    .Init = _sti_Pool_Init,
    .Get = _sti_Pool_Get,
    .Mapping = _sti_Pool_Mapping,
    .PA2VANonCached = _sti_Pool_PA2VANonCached,
    .UnMapping = _sti_Pool_UnMapping,
    .MMA_Alloc = _sti_IOMMU_Alloc,
    .MMA_Init = _sti_IOMMU_Init,
    .MMA_Free = _sti_IOMMU_Free,
    .MMA_Exit = _sti_IOMMU_Exit,
    .MMA_IsMMAEnabled = _sti_IOMMU_IsMMAEnabled,
    .MMA_GetMIUIndex = _sti_IOMMU_GetMIUIndex,
    .MMA_GetBufFd = _sti_IOMMU_GetBufFd,
    .MMA_GetIOVA = _sti_IOMMU_GetIOVA,
    .MMA_IsIOVA_Address = _sti_IOMMU_IsIOVA_Address,
    .MsOSPA2BA = _sti_MsOSPA2BA,
    .MMA_Secure_LockBuf = _sti_IOMMU_Secure_Lock,
    .MMA_Secure_UnlockBuf = _sti_IOMMU_Secure_Unlock,
    .MMA_Secure_GetPipeLineId = _sti_IOMMU_Secure_GetPipeLineId,
};


__attribute__((constructor)) static void registerMemoryBackend(void)
{
    D_INFO("[DFB] %s (%s)\n", __FUNCTION__, __FILE__);
    dfb_SetMemBackend(&sti_memory_backend);
}
