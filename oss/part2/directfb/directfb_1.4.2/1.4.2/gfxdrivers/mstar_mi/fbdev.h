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

#ifndef __CORE__FBDEV_H__
#define __CORE__FBDEV_H__

#include <asm/types.h>    /* Needs to be included before dfb_types.h */

#include <core/coretypes.h>

#include <core/layers_internal.h>

#include <core/system.h>

#include <fusion/call.h>
#include <fusion/reactor.h>


#include "fb.h"
#include "surfacemanager.h"


#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC    _IOW('F', 0x20, u_int32_t)
#endif


typedef struct {
     /* fbdev fixed screeninfo, contains infos about memory and type of card */
     struct fb_fix_screeninfo fix;

     VideoMode                *modes;        /* linked list of valid
                                                video modes */
     VideoMode                 current_mode; /* current video mode */

     struct fb_var_screeninfo current_var;   /* fbdev variable screeninfo
                                                set by DirectFB */
     struct fb_var_screeninfo orig_var;      /* fbdev variable screeninfo
                                                before DirectFB was started */

     void                    *orig_cmap_memory;
     void                    *temp_cmap_memory;
     void                    *current_cmap_memory;

     struct fb_cmap           orig_cmap;     /* original palette */

     struct fb_cmap           current_cmap;  /* our copy of the cmap */

     struct fb_cmap           temp_cmap;     /* scratch */

     FusionCall               fbdev_ioctl;   /* ioctl rpc */

     unsigned long            page_mask;     /* PAGE_SIZE - 1 */

     CoreSurfacePool          *pool;

     struct {
          int                 bus;
          int                 dev;
          int                 func;
     } pci;                                  /* PCI Bus ID of graphics device */

     struct {
          unsigned short      vendor;        /* Graphics device vendor id */
          unsigned short      model;         /* Graphics device model id */
     } device;


     FusionSHMPoolShared     *shmpool;
     FusionSHMPoolShared     *shmpool_data;

     CoreLayerRegionConfig    config;

     SurfaceManager          *manager;

     int layer_id;
     int iommu;
     bool bSecure_current; /* Record current secure mode(normal/secure) */
     bool bSecure;
     bool DLG_MODE; // To idetify is in DLG mode or not
     unsigned int screen_width;
     unsigned int screen_height;
} FBDevShared;

typedef struct {
     FBDevShared             *shared;

     CoreDFB                 *core;

     /* virtual framebuffer address */
     void                    *framebuffer_base;

     int                      fd;            /* file descriptor for /dev/fb */

} FBDev;

int fbdev_iommu_ioctl( int request, void *arg, int arg_size, int layer_id );
#define FBDEV_IOMMU_IOCTL(request, arg, layer_id)  fbdev_iommu_ioctl( request, arg, sizeof(*(arg)), layer_id )
void * fbdev_mpool_mapping(unsigned long fb_hal_addr , unsigned long fb_mem_len , int bIommuUsed);

#endif
