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

#include <config.h>

#include <string.h>

#include <direct/conf.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <voodoo/conf.h>


static VoodooConfig config = {
};

VoodooConfig *voodoo_config = &config;
const char   *voodoo_config_usage =
     "libvoodoo options:\n"
     "  player-name=<name>             Set player name\n"
     "  player-vendor=<name>           Set player vendor\n"
     "  player-model=<name>            Set player model\n"
     "  player-uuid=<name>             Set player uuid\n"
     "  proxy-memory-max=<kB>          Set maximum amount of memory per connection\n"
     "  proxy-surface-max=<kB>         Set maximum amount of memory per surface\n"
     "\n";

/**********************************************************************************************************************/

DirectResult
voodoo_config_set( const char *name, const char *value )
{
     if (strcmp (name, "player-name" ) == 0) {
          if (value) {
               direct_snputs( voodoo_config->play_info.name, value, VOODOO_PLAYER_NAME_LENGTH );
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "player-vendor" ) == 0) {
          if (value) {
               direct_snputs( voodoo_config->play_info.vendor, value, VOODOO_PLAYER_VENDOR_LENGTH );
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "player-model" ) == 0) {
          if (value) {
               direct_snputs( voodoo_config->play_info.model, value, VOODOO_PLAYER_MODEL_LENGTH );
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "player-uuid" ) == 0) {
          if (value) {
               sscanf( value, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                       (unsigned int*)&voodoo_config->play_info.uuid[0], (unsigned int*)&voodoo_config->play_info.uuid[1], (unsigned int*)&voodoo_config->play_info.uuid[2], (unsigned int*)&voodoo_config->play_info.uuid[3], (unsigned int*)&voodoo_config->play_info.uuid[4],
                       (unsigned int*)&voodoo_config->play_info.uuid[5], (unsigned int*)&voodoo_config->play_info.uuid[6], (unsigned int*)&voodoo_config->play_info.uuid[7], (unsigned int*)&voodoo_config->play_info.uuid[8], (unsigned int*)&voodoo_config->play_info.uuid[9],
                       (unsigned int*)&voodoo_config->play_info.uuid[10], (unsigned int*)&voodoo_config->play_info.uuid[11], (unsigned int*)&voodoo_config->play_info.uuid[12], (unsigned int*)&voodoo_config->play_info.uuid[13], (unsigned int*)&voodoo_config->play_info.uuid[14],
                       (unsigned int*)&voodoo_config->play_info.uuid[15] );
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "proxy-memory-max" ) == 0) {
          if (value) {
               unsigned int max;

               if (sscanf( value, "%u", &max ) != 1) {
                    D_ERROR( "Voodoo/Config '%s': Invalid value specified!\n", name );
                    return DR_INVARG;
               }

               voodoo_config->memory_max = max * 1024;
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "proxy-surface-max" ) == 0) {
          if (value) {
               unsigned int max;

               if (sscanf( value, "%u", &max ) != 1) {
                    D_ERROR( "Voodoo/Config '%s': Invalid value specified!\n", name );
                    return DR_INVARG;
               }

               voodoo_config->surface_max = max * 1024;
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "proxy-layer-mask" ) == 0) {
          if (value) {
               unsigned int mask;

               if (sscanf( value, "%u", &mask ) != 1) {
                    D_ERROR( "Voodoo/Config '%s': Invalid value specified!\n", name );
                    return DR_INVARG;
               }

               voodoo_config->layer_mask = mask;
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "proxy-stacking-mask" ) == 0) {
          if (value) {
               unsigned int mask;

               if (sscanf( value, "%u", &mask ) != 1) {
                    D_ERROR( "Voodoo/Config '%s': Invalid value specified!\n", name );
                    return DR_INVARG;
               }

               voodoo_config->stacking_mask = mask;
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (strcmp (name, "proxy-resource-id" ) == 0) {
          if (value) {
               unsigned int resource_id;

               if (sscanf( value, "%u", &resource_id ) != 1) {
                    D_ERROR( "Voodoo/Config '%s': Invalid value specified!\n", name );
                    return DR_INVARG;
               }

               voodoo_config->resource_id = resource_id;
          }
          else {
               D_ERROR( "Voodoo/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (direct_config_set( name, value ))
          return DR_UNSUPPORTED;

     return DR_OK;
}

