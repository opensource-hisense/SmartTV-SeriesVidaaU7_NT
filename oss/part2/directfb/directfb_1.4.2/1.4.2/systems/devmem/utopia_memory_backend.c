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
#include <misc/conf.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <MsCommon.h>
#include "devmem.h"
#include <drvCMAPool.h>
#include <MsOS.h>

static bool _utopia_MPool_Init(void)
{

#if USE_MSTAR_CMA
    MsOS_MPool_Init();
    MsOS_Init();
#else
    MsOS_MPool_Init();
#endif
    return true;
}

static bool _utopia_MPool_Get(void ** pAddrVirt, u32 * pu32AddrPhys, u32 * pu32Size, bool bNonCache)
{

    MsOS_MPool_Get(pAddrVirt, pu32AddrPhys, pu32Size, bNonCache);

    return true;
}

static bool _utopia_MPool_Mapping(u8 u8MiuSel, hal_phy Offset, u32 u32MapSize, bool  bNonCache)
{

#ifndef ARCH_ARM
#pragma weak  MsOS_MPool_Mapping_v2
        MS_BOOL MsOS_MPool_Mapping_v2(MS_U8 u8MiuSel, MS_U32 offset, MS_U32 tMapSize, MS_U8 u8MapMode);
#else
#pragma weak  MsOS_MPool_Mapping_v2
        MS_BOOL MsOS_MPool_Mapping_v2(MS_U8 u8MiuSel, MS_PHY offset, MS_SIZE tMapSize, MS_U8 u8MapMode);
#endif


    if (MsOS_MPool_Mapping_v2)
        return MsOS_MPool_Mapping_v2(u8MiuSel, Offset, u32MapSize, bNonCache);
    else
        return MsOS_MPool_Mapping(u8MiuSel, Offset, u32MapSize, bNonCache);
}

static void* _utopia_MPool_PA2VANonCached(hal_phy pAddrPhys)
{
    return MsOS_MPool_PA2KSEG1(pAddrPhys);
}

static bool _utopia_MPool_UnMapping(void* ptrVirtStart, u32  u32MapSize)
{
#ifdef ARCH_ARM
    return (ptrVirtStart != NULL && u32MapSize > 0) ? MsOS_MPool_UnMapping(ptrVirtStart, u32MapSize) : false;
#else
    return true;
#endif
}

static bool _utopia_MMA_Alloc(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle)
{
    D_UNIMPLEMENTED();
    return 0;
}

static bool _utopia_MMA_Init(void)
{
    D_UNIMPLEMENTED();
    return 0;
}

static bool _utopia_MMA_Free(hal_phy Phy_addr)
{
    D_UNIMPLEMENTED();
    return 0;
}

static bool _utopia_MMA_Exit(void)
{
    D_UNIMPLEMENTED();
    return 0;
}


static bool _utopia_MMA_IsMMAEnabled(void)
{
    bool is_mma_enabled = false;

#if 0
     if(MSOS_MMA_query_feature)
     {
         MSOS_MMA_FEATURE  feature;
         MSOS_MMA_query_feature(&feature);
         if (feature.u32support == 1)
             is_mma_enabled = true;
     }
#endif

    return is_mma_enabled;
}

static bool _utopia_MMA_GetMIUIndex(int *pIndex)
{
    D_UNIMPLEMENTED();
    return false;
}

#if USE_MSTAR_CMA
static DirectResult _utopia_CMA_MapMem(void * input_data, unsigned long  mem_phys,
                                                                       unsigned int mem_length, unsigned long  reg_phys,
                                                                       unsigned int reg_length, bool secondary)
{
    /* Type Casting */
    DevMemData *data = (DevMemData*)input_data;

    struct CMA_Pool_Init_Param CMA_Pool_Init_PARAM;    // for MApi_CMA_Pool_Init
    struct CMA_Pool_Alloc_Param CMA_Alloc_PARAM;        // for MApi_CMA_Pool_GetMem

    bool ret;
    CMA_Pool_Init_PARAM.heap_id = ( !secondary )? dfb_config->mst_cma_heap_id : dfb_config->mst_sec_cma_heap_id;
    CMA_Pool_Init_PARAM.flags = CMA_FLAG_MAP_VMA;
    CMA_Pool_Init_PARAM.mtlb_addrspace_offset = (unsigned long long)0;
    CMA_Pool_Init_PARAM.mtlb_addrspace_len = (unsigned long long)0;

    ret = MApi_CMA_Pool_Init(&CMA_Pool_Init_PARAM);
    if(ret == false)
    {
        D_PERROR("\033[35mFunction = %s, Line = %d, CMA_POOL_INIT ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
    }
    else
    {
        D_INFO("\033[35mFunction = %s, Line = %d, get pool_handle_id is %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM.pool_handle_id);
        D_INFO("\033[35mFunction = %s, Line = %d, get miu is %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM.miu);
        D_INFO("\033[35mFunction = %s, Line = %d, get heap_miu_start_offset is 0x%llX\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM.heap_miu_start_offset);
        D_INFO("\033[35mFunction = %s, Line = %d, get heap_length is 0x%X\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM.heap_length);

        if ( !secondary )
        {
            data->shared->pool_handle_id = CMA_Pool_Init_PARAM.pool_handle_id;
            if(CMA_Pool_Init_PARAM.heap_length < mem_length)
                return DR_INIT;
        }
        else
        {
            data->shared->sec_pool_handle_id = CMA_Pool_Init_PARAM.pool_handle_id;
            if(CMA_Pool_Init_PARAM.heap_length < mem_length)
                return DR_INIT;
        }
    }

    D_INFO("\033[35mFunction = %s, Line = %d,  miu0cpu_offset = 0x%X  hal_offset= 0x%X \033[m\n",__PRETTY_FUNCTION__, __LINE__,dfb_config->mst_miu0_cpu_offset,dfb_config->mst_miu0_hal_offset);
    D_INFO("\033[35mFunction = %s, Line = %d, miu1cpu_offset = 0x%X  hal_offset= 0x%X \033[m\n",__PRETTY_FUNCTION__, __LINE__,dfb_config->mst_miu1_cpu_offset,dfb_config->mst_miu1_hal_offset);
    D_INFO("\033[35mFunction = %s, Line = %d, video_phys_cpu = 0x%X, video_phys_hal = 0x%X \033[m\n",__PRETTY_FUNCTION__, __LINE__,dfb_config->video_phys_cpu, dfb_config->video_phys_hal );

    CMA_Alloc_PARAM.pool_handle_id = CMA_Pool_Init_PARAM.pool_handle_id;
    CMA_Alloc_PARAM.offset_in_pool = 0; //fix the coverity fail, initialize the value
    if( CMA_Pool_Init_PARAM.miu == 0)
        CMA_Alloc_PARAM.offset_in_pool = ( ( !secondary )? dfb_config->video_phys_hal : dfb_config->video_phys_secondary_hal ) - dfb_config->mst_miu0_hal_offset -CMA_Pool_Init_PARAM.heap_miu_start_offset;
    else if( CMA_Pool_Init_PARAM.miu == 1)
        CMA_Alloc_PARAM.offset_in_pool =  ( ( !secondary )? dfb_config->video_phys_hal : dfb_config->video_phys_secondary_hal ) - dfb_config->mst_miu1_hal_offset -CMA_Pool_Init_PARAM.heap_miu_start_offset;

    CMA_Alloc_PARAM.length = mem_length;
    CMA_Alloc_PARAM.flags = CMA_FLAG_VIRT_ADDR;

    D_INFO("\033[35mFunction = %s, Line = %d, CMA_POOL_Get offset_in_pool =0x%llX !!\033[m\n", __PRETTY_FUNCTION__, __LINE__,CMA_Alloc_PARAM.offset_in_pool);

    ret = MApi_CMA_Pool_GetMem(&CMA_Alloc_PARAM);
    if(ret == false)
    {
        D_PERROR("\033[35mFunction = %s, Line = %d, CMA_POOL_Get ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
    }
    else
    {
        D_INFO("\033[35mFunction = %s, Line = %d, get virtul_addr is 0x%x\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Alloc_PARAM.virt_addr);
        if ( !secondary )
        {
            data->mem = CMA_Alloc_PARAM.virt_addr;

            data->shared->offset_in_pool = CMA_Alloc_PARAM.offset_in_pool;
        }
        else
        {
            data->mem_secondary = CMA_Alloc_PARAM.virt_addr;

            data->shared->sec_offset_in_pool = CMA_Alloc_PARAM.offset_in_pool;
        }
    }

    return  DR_OK;
}


static void* _utopia_CMA_GetMem(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  length)
{
    struct CMA_Pool_Alloc_Param CMA_Alloc_PARAM;
    CMA_Alloc_PARAM.pool_handle_id = pool_handle_id;
    CMA_Alloc_PARAM.offset_in_pool = offset_in_pool;
    CMA_Alloc_PARAM.length = length;
    CMA_Alloc_PARAM.flags = CMA_FLAG_VIRT_ADDR;

    MApi_CMA_Pool_GetMem(&CMA_Alloc_PARAM);
    return CMA_Alloc_PARAM.virt_addr;
}

static bool _utopia_CMA_PutMem( unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  mem_length)
{
    struct CMA_Pool_Free_Param CMA_Free_PARAM;// for MApi_CMA_Pool_PutMem
    CMA_Free_PARAM.pool_handle_id = pool_handle_id;
    CMA_Free_PARAM.offset_in_pool = offset_in_pool;
    CMA_Free_PARAM.length = mem_length;

    return MApi_CMA_Pool_PutMem(&CMA_Free_PARAM);
}
#endif

static unsigned long long _utopia_MsOSPA2BA(hal_phy pAddrPhys)
{
    return MsOS_PA2BA(pAddrPhys);
}

static IDirectFBInternalMemBackend utopia_memory_backend =
{
    .Init = _utopia_MPool_Init,
    .Get = _utopia_MPool_Get,
    .Mapping = _utopia_MPool_Mapping,
    .PA2VANonCached = _utopia_MPool_PA2VANonCached,
    .UnMapping = _utopia_MPool_UnMapping,
    .MMA_Alloc = _utopia_MMA_Alloc,
    .MMA_Init = _utopia_MMA_Init,
    .MMA_Free = _utopia_MMA_Free,
    .MMA_Exit = _utopia_MMA_Exit,
    .MMA_IsMMAEnabled = _utopia_MMA_IsMMAEnabled,
    .MMA_GetMIUIndex = _utopia_MMA_GetMIUIndex,
#if USE_MSTAR_CMA
    .CMA_MapMem = _utopia_CMA_MapMem,
    .CMA_GetMem = _utopia_CMA_GetMem,
    .CMA_PutMem = _utopia_CMA_PutMem,
#endif
    .MsOSPA2BA = _utopia_MsOSPA2BA,
};


__attribute__((constructor)) static void registerMemoryBackend(void)
{
    D_INFO("[DFB] %s (%s)\n", __FUNCTION__, __FILE__);
    dfb_SetMemBackend(&utopia_memory_backend);
}
