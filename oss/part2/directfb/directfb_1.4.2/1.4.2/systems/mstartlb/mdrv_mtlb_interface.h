////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2012 MStar Semiconductor, Inc.
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
/// @file   mdrv_mtlb_interface.h
/// @brief  MIU TLB  Driver IO Interface
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_MTLB_INTERFACE_H_
#define _MDRV_MTLB_INTERFACE_H_

/* Use 'T' as magic number */
#define MTLB_IOC_MAGIC                'T'

#define MTLB_IOC_GET_RANGE              _IOR(MTLB_IOC_MAGIC, 0x00, MemRange)
#define MTLB_IOC_ALLOC_BUF              _IOWR(MTLB_IOC_MAGIC, 0x01, BufferInfo)
#define MTLB_IOC_FREE_BUF               _IOWR(MTLB_IOC_MAGIC, 0x02, AllocationHandle)
#define MTLB_IOC_MOVE_BUF               _IOWR(MTLB_IOC_MAGIC, 0x03, MoveInfo)
#define MTLB_IOC_MTLB_ENABLE            _IO(MTLB_IOC_MAGIC, 0x04)
#endif

