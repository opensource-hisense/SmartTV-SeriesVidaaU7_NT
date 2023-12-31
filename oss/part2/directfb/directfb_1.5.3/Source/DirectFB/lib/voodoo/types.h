/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
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

#ifndef __VOODOO__TYPES_H__
#define __VOODOO__TYPES_H__

#include <direct/types.h>


typedef u32 VoodooInstanceID;
typedef u32 VoodooMethodID;
//typedef u64 VoodooMessageSerial;
typedef u32 VoodooMessageSerial;

#define VOODOO_INSTANCE_NONE  ((VoodooInstanceID) 0)


typedef struct __V_VoodooMessageHeader   VoodooMessageHeader;
typedef struct __V_VoodooSuperMessage    VoodooSuperMessage;
typedef struct __V_VoodooRequestMessage  VoodooRequestMessage;
typedef struct __V_VoodooResponseMessage VoodooResponseMessage;


typedef struct __V_VoodooClient          VoodooClient;
typedef struct __V_VoodooConfig          VoodooConfig;
typedef struct __V_VoodooManager         VoodooManager;
typedef struct __V_VoodooPlayer          VoodooPlayer;
typedef struct __V_VoodooServer          VoodooServer;


typedef DirectResult (*VoodooSuperConstruct)( VoodooServer         *server,
                                              VoodooManager        *manager,
                                              const char           *name,
                                              void                 *ctx,
                                              VoodooInstanceID     *ret_instance );

typedef DirectResult (*VoodooDispatch)      ( void                 *dispatcher,
                                              void                 *real,
                                              VoodooManager        *manager,
                                              VoodooRequestMessage *msg );

#endif

