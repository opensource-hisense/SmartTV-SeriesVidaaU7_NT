/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
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

#ifndef __DIRECT__MEM_H__
#define __DIRECT__MEM_H__

#include <direct/os/mem.h>

/**********************************************************************************************************************/

void DIRECT_API direct_print_memleaks( void );

/**********************************************************************************************************************/

void DIRECT_API *direct_dbg_malloc ( const char *file, int line,
                                     const char *func, size_t bytes );

void DIRECT_API *direct_dbg_calloc ( const char *file, int line,
                                     const char *func, size_t count, size_t bytes);

void DIRECT_API *direct_dbg_realloc( const char *file, int line,
                                     const char *func, const char *what, void *mem,
                                     size_t bytes );

char DIRECT_API *direct_dbg_strdup ( const char *file, int line,
                                     const char *func, const char *str );

void DIRECT_API  direct_dbg_free   ( const char *file, int line,
                                     const char *func, const char *what, void *mem );

/**********************************************************************************************************************/

#if 0

#if !DIRECT_BUILD_DEBUGS
#warning Building with debug, but library headers suggest that debug is not supported.
#endif


#define D_MALLOC(bytes)       direct_dbg_malloc( __FILE__, __LINE__, __FUNCTION__, bytes )
#define D_CALLOC(count,bytes) direct_dbg_calloc( __FILE__, __LINE__, __FUNCTION__, count, bytes )
#define D_REALLOC(mem,bytes)  direct_dbg_realloc( __FILE__, __LINE__, __FUNCTION__, #mem, mem, bytes )
#define D_STRDUP(str)         direct_dbg_strdup( __FILE__, __LINE__, __FUNCTION__, str )
#define D_FREE(mem)           direct_dbg_free( __FILE__, __LINE__, __FUNCTION__, #mem, mem )

#else

/**********************************************************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define D_MEMALIGN memalign
#define D_MEMSET   memset
#define D_MALLOC   direct_malloc
#define D_CALLOC   direct_calloc
#define D_REALLOC  direct_realloc
#define D_STRDUP   direct_strdup
#define D_FREE     direct_free

#endif


void __D_mem_init( void );
void __D_mem_deinit( void );


#define D_SAFE_FREE(ptr)     if(ptr){ D_FREE(ptr); ptr = NULL;}


#include <direct/types.h>
#include <direct/types.h>

bool  dfb_MPool_Init(void);
bool  dfb_MPool_Get(void ** pAddrVirt, u32 * pu32AddrPhys, u32 * pu32Size, bool bNonCache);
bool  dfb_MPool_Mapping(u8 u8MiuSel, hal_phy Offset, u32 u32MapSize, bool  bNonCache);
void* dfb_MPool_PA2VANonCached(hal_phy pAddrPhys);
bool  dfb_MPool_UnMapping(void* ptrVirtStart, u32  u32MapSize);
bool  dfb_MMA_Alloc(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle);
bool  dfb_MMA_Init(void);
bool  dfb_MMA_Free(hal_phy phy_addr, u32 u32size);
bool  dfb_MMA_Exit(void);
bool  dfb_MMA_IsMMAEnabled(void);
bool  dfb_MMA_Get_MIU_Index(hal_phy Phy_addr, int *ret_miu_index);
bool  dfb_MMA_Get_Buf_Fd(hal_phy Phy_addr, int *pfd);
bool  dfb_MMA_Get_IOVA(u32 Phy_addr, u64 *iova);
bool  dfb_MMA_Secure_LockBuf(char *buftag, hal_phy u64Addr, u32 size, u32 u32PipelineId);
bool  dfb_MMA_Secure_UnlockBuf(char *buftag, hal_phy u64Addr);
bool  dfb_MMA_Secure_getPipeLineId(u32 *u32PipelineId);
bool  dfb_MMA_IsIOVA_Address(u64 busAddr);
DirectResult dfb_CMA_MapMem(void *input_data, unsigned long mem_phys,
                            unsigned int mem_length, unsigned long  reg_phys,
                            unsigned int reg_length, bool secondary);
void* dfb_CMA_GetMem(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  mem_length);
bool  dfb_CMA_PutMem(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  mem_length);

unsigned long long dfb_MsOSPA2BA(hal_phy pAddrPhys);

typedef struct _IDirectFBInternalMemBackend IDirectFBInternalMemBackend;

struct _IDirectFBInternalMemBackend
{
    bool  (*Init)(void);
    bool  (*Get)(void ** pAddrVirt, u32 * pu32AddrPhys, u32 * pu32Size, bool bNonCache);
    bool  (*Mapping)(u8 u8MiuSel, hal_phy Offset, u32 u32MapSize, bool  bNonCache);
    void* (*PA2VANonCached)(hal_phy pAddrPhys);
    bool  (*UnMapping)(void* ptrVirtStart, u32  u32MapSize);
    bool  (* MMA_Alloc)(u8* u8bufTag, u32 u32size, hal_phy* pPhy, void** ppBufHandle);
    bool  (*MMA_Init)(void);
    bool  (*MMA_Free)(hal_phy Phy_addr, u32 u32size);
    bool  (*MMA_Exit)(void);
    bool  (*MMA_IsMMAEnabled)(void);
    bool  (*MMA_GetMIUIndex)(hal_phy Phy_addr, int *pIndex);
    bool  (*MMA_GetBufFd)(hal_phy Phy_addr, int *pfd);
    bool  (*MMA_GetIOVA)(u32 Phy_addr, u64 *iova);
    bool  (*MMA_IsIOVA_Address)(u64 busAddr);
    DirectResult (*CMA_MapMem)(void *input_data, unsigned long  mem_phys, unsigned int   mem_length, unsigned long  reg_phys,
                               unsigned int reg_length, bool secondary);
    void* (*CMA_GetMem)(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  mem_length);
    bool  (*CMA_PutMem)(unsigned long pool_handle_id, unsigned long long offset_in_pool, unsigned int  mem_length);
    unsigned long long (*MsOSPA2BA)(hal_phy pAddrPhys);
    bool  (*MMA_Secure_LockBuf)(char *buftag, hal_phy u64Addr, u32 size, u32 u32PipelineId);
    bool  (*MMA_Secure_UnlockBuf)(char *buftag, hal_phy u64Addr);
    bool (*MMA_Secure_GetPipeLineId)(u32 *u32PipelineId);

};


void dfb_SetMemBackend(IDirectFBInternalMemBackend * backend);


#endif

