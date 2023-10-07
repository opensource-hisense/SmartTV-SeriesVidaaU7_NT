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

#ifndef __FUSION__CONF_H__
#define __FUSION__CONF_H__


#include <fusion/types.h>

struct __Fusion_FusionConfig {
     char *tmpfs;             /* location of shm file */
     char *fusion_dir;
     char *fusion_shm_dir;
     bool  debugshm;
     bool  madv_remove;
     bool  madv_remove_force;
     bool  force_slave;
     u8    mst_shm_main_pool_size;/*set the fusion main pool memory size. default : 1  (MB)*/
     u8    mst_directfb_main_pool_size;/*set the directfb main pool memory size. default : 4  (MB)*/
     u8    mst_directfb_data_pool_size;/*set the directfb data pool memory size. default : 16  (MB)*/
     gid_t shmfile_gid;       /* group that owns shm file */     

     bool  secure_fusion;
	 unsigned long  mst_fusion_world_virtual_addr;
     bool   mst_fusion_fix_addr_enable; /* enable the mmap of fusion address to use MAP_FIXED */

     /*
     default :   0666 (oct-type)
     goal    :   set access mode for Shm heap file.
     */
     unsigned int mst_fusion_shm_access_mode;
};

extern FusionConfig FUSION_API *fusion_config;

extern const char   FUSION_API *fusion_config_usage;


DirectResult        FUSION_API  fusion_config_set( const char *name, const char *value );
void                fusion_config_destroy(void);
void                FUSION_API  fusion_config_init(void);


void __Fusion_conf_init( void );
void __Fusion_conf_deinit( void );

#endif

