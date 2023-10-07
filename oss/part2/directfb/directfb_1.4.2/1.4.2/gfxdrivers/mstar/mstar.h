#ifndef __MSTAR__MSTAR_H__
#define __MSTAR__MSTAR_H__

#include <sys/ioctl.h>

/******************************************************************************
 * Platform specific values (FIXME: add runtime config)
 */

//#  define    MSTAR_LCD_WIDTH    1280
//#  define    MSTAR_LCD_HEIGHT    1024

/******************************************************************************
 * Register access
 */

//#define SH7722_TDG_REG_USE_IOCTLS
u32 mstarGFXAddr(u32 cpuPhysicalAddr, u32 mst_miu1_cpu_offset, u32 mst_miu1_hal_offset);

#endif
