/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
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
#ifndef __MISC__UTIL_H__
#define __MISC__UTIL_H__

#include <directfb_util.h>
#include <core/core.h>


#define DFB_CHECK_POINT(name)                           \
    D_ONCE("\33[0;33;44mCheck => %s\33[0m", name);   


#define SAFE_GET_SYMBOL(HANDLE, SYMBOL, FPTR)                                       \
    FPTR = dlsym(HANDLE, SYMBOL);                                                   \
    if(!FPTR){                                                \
        printf("[DFB][%s (%s)] dlsym failed: " SYMBOL " : %s\n", __FUNCTION__, __FILE__, dlerror());  \
        dlclose(HANDLE);                                                            \
        return ;                                                         \
    }


typedef bool(*DFB_DUMPQUERYINFO)(int , void *);

typedef struct {
          const void* input;
                void* output;
} DumpInfoType;


typedef struct {

    // Specify the interval of PIDs which you want to dump surface within "Flip".

    int pid_start;
    int pid_end;

} DumpSurfaceProcessInfo;


typedef enum {
    DF_DUMP_MAXMEM = 0,
    DF_DUMP_CURRENT_MEM,
    DF_DUMP_GET_PROCESS_ID,
    DF_DUMP_GET_PARENT_PROCESS_ID,
    DF_DUMP_SET_DUMPSURFACE_BY_PROCESS_ID,
    DF_DUMP_GET_PROCESS_MEM_INFO,
    
} ENUM_DFBDUMP_QUERY_INFO;

#define DFB_MEASURE_BOOT_TIME  1 

#ifdef DFB_MEASURE_BOOT_TIME

#define DFB_BOOT_GETTIME(INTERVALS, FLAG, LEVEL)               \
        dfb_boot_getTime(#INTERVALS, INTERVALS, FLAG, LEVEL)   

typedef enum {
    DF_MEASURE_START = 0,
    DF_MEASURE_END       
} DFB_Boot_MeasureFlag;

typedef enum { 
    DF_BOOT_DIRECTFBINIT,               // lv1
    DF_BOOT_CONFIG_INIT,                // ------lv2
    DF_BOOT_SETKEYPADCFG,               // ------lv2

    DF_BOOT_DIRECTFBCREATE,             // lv1
    DF_BOOT_CONFIG_SET,                 // ------lv2
    DF_BOOT_DIRECT_INIT,                // ------lv2
    DF_BOOT_CORE_CREATE,                // ------lv2
    DF_BOOT_SYSTEM_LOOKUP,              // ---------lv3
    DF_BOOT_FUSION_ENTER,               // ---------lv3
    DF_BOOT_FUSION_ARENA_ENTER,         // ---------lv3
    DF_BOOT_FUSION_SHM_POOL_CREATE,     // ------------lv4
    DF_BOOT_CORE_INITIALIZE,            // ------------lv4
    DF_BOOT_CLIPBOARD_CORE_INIT,        // ---------------lv5
    DF_BOOT_COLORHASH_CORE_INIT,        // ---------------lv5
    DF_BOOT_SURFACE_CORE_INIT,          // ---------------lv5
    DF_BOOT_SYSTEM_CORE_INIT,           // ---------------lv5
    DF_BOOT_INPUT_CORE_INIT,            // ---------------lv5
    DF_BOOT_GRAPGICS_CORE_INIT,         // ---------------lv5       
    DF_BOOT_DRIVER_INIT_DRIVER,         // ---------------lv6
    DF_BOOT_DRIVER_GLOBAL_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_IOMAP_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_VE_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_XC_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_GOP_CALLBACK,         // ---------------lv7
    DF_BOOT_DRIVER_GOP_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_GE_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_DIP_INIT,         // ---------------lv7
    DF_BOOT_DRIVER_INIT_DEVICE,         // ---------------lv6
    DF_BOOT_DEVICE_GOP_INIT,         // ---------------lv7
    DF_BOOT_DEVICE_GFX_INIT,         // ---------------lv7
    DF_BOOT_SCREEN_CORE_INIT,           // ---------------lv5        
    DF_BOOT_LAYER_CORE_INIT,            // ---------------lv5            
    DF_BOOT_WM_CORE_INIT,               // ---------------lv5
    DF_BOOT_IDIRECTFB_CONSTRUCT,        // ------lv2
    DF_BOOT_LAYER_GET_PRIMARY_CONTEXT,  // ---------lv3
    DF_BOOT_LAYER_CONTEXT_WINDOWSTACK,  // ---------lv3
    DF_BOOT_INITLAYERS,                 // ---------lv3

    DF_BOOT_MSOS_INIT,                  // lv1
    
    DF_BOOT_ALL
    
} DFB_Boot_MeasureIntervals;

typedef enum {
    DF_BOOT_LV1,
    DF_BOOT_LV2,
    DF_BOOT_LV3,
    DF_BOOT_LV4,
    DF_BOOT_LV5,
    DF_BOOT_LV6,
    DF_BOOT_LV7
        
} DFB_Boot_PrintLevel;

typedef struct {
    char    *name;
    long     startTime;
    long     endTime;
    int      level;
    
} DFBBootLog;

typedef struct {
    long     startTime;
    long     endTime;
    long     diff;
    int      count;
    bool     bCheck;
    bool     bOpen;
    char*    fileName;
    char*    funName;
    int      line;

} DFBMeasureLog;
extern DFBBootLog    bootInfo[DF_BOOT_ALL];
extern char         *getENVFlag;


void dfb_boot_initENV(void);


void dfb_boot_getTime(  const char                  *name,  
                        DFB_Boot_MeasureIntervals    intervals, 
                        DFB_Boot_MeasureFlag         flag,
                        DFB_Boot_PrintLevel          level );

void dfb_boot_printTimeInfo(void);

void dfb_print_duration(DFB_Boot_MeasureFlag flag, const char* title);

#else
     
#define DFB_BOOT_GETTIME()

#endif  // end of #ifdef DFB_MEASURE_BOOT_TIME

bool dfb_dumpQueryInfo( int index, DumpInfoType* info);


bool dfb_get_BankForceWrite(void);
void dfb_set_BankForceWrite(bool bState);

void dfb_check_versionInfo(void);
void dfb_check_gop_info(void);

void dfb_fps_count(void);

long dfb_getTime(void);

#define MEASURE_TIME_INTERVAL 0

#if MEASURE_TIME_INTERVAL

extern DFBMeasureLog timeInfo[100];


#define DFB_MEASURE_TIME_INTERVAL(block_ID)    \
        dfb_measure_time_interval(block_ID, __FILE__, __FUNCTION__, __LINE__)


void dfb_measure_time_interval( int             block_ID,
                                const char     *file,
                                const char     *fun,
                                int             line );

void dfb_printf_measure_time_interval();
#endif // end of #ifdef MEASURE_TIME_INTERVAL

void dfb_get_backtrace(void);
bool dfb_dodfbinfo_setkeypadcfg(void);
bool dfb_dodfbinfo_setdfbrccfg(void);


bool dfb_CRC32_Init(void **handle);
bool dfb_CRC32_Deinit(void *handle);
bool dfb_CRC32_Update(void *handle, const unsigned char *pu8Data, unsigned long u32Size);
unsigned long dfb_CRC32_GetResult(void *handle);
DFBResult dfb_CRC32_Update_by_Surface(void *handle, IDirectFBSurface *surface);
DFBResult dfb_Surface_GetCRC32(IDirectFBSurface *surface, unsigned long * ret_checksum);
DFBResult dfb_CoreSurface_GetCRC32(CoreSurface *surface, unsigned long * ret_checksum);

#endif

