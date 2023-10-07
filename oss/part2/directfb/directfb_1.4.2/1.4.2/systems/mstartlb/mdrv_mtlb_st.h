////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2013 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_mtlb_st.h
/// @brief  MIU TLB  Driver struct define
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MTLB_ST_H_
#define _MDRV_MTLB_ST_H_

typedef unsigned int AllocationHandle ;
#if 0
enum EALLOC_TYPE {
    E_INVALID = 0,
    E_IN_KPAGE,
    E_IN_DPAGE
};
#endif

#define ALLOC_KPAGE 1
#define ALLOC_DPAGE 2

enum ECLIENTIP {    
    E_C_GE,
    E_C_GOP,
    CLIENT_NUM
};

typedef struct _MemRange {
    unsigned int start;
    unsigned int size;
} MemRange;

typedef struct _BufferInfo {
    unsigned int tlb_start;
    unsigned int tlb_size;
    unsigned int location;
    int miu_type;
    AllocationHandle handle;
    unsigned long phy_addr;
} BufferInfo;

typedef struct {
    AllocationHandle handle;
    unsigned long dest;
    unsigned int result;
} MoveInfo;
 
typedef struct _LockInfo {
    AllocationHandle handle;
    void * vAddr;
    unsigned int phyAddr;
    unsigned int tlbAddr;
} LockInfo;

#endif
