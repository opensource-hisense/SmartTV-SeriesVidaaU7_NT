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

#include <config.h>
#include <search.h>


#include <stddef.h>
#include <string.h>

#include <direct/conf.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#if FUSION_BUILD_MULTI
#include <grp.h>
#endif


static FusionConfig config;

FusionConfig *fusion_config       = &config;
const char   *fusion_config_usage =
     "libfusion options:\n"
     "  force-slave                    Always enter as a slave, waiting for the master, if not there\n"
     "  tmpfs=<directory>              Location of shared memory file\n"
#if FUSION_BUILD_MULTI
     "  shmfile-group=<groupname>      Group that owns shared memory files\n"
#endif
     "  [no-]debugshm                  Enable shared memory allocation tracking\n"
     "  [no-]madv-remove               Enable usage of MADV_REMOVE (default = auto)\n"
     "  [no-]secure-fusion             Use secure fusion, e.g. read-only shm (default=no)\n"
     "\n";

/**********************************************************************************************************************/

void
__Fusion_conf_init()
{
     fusion_config->shmfile_gid = -1;
     fusion_config->mst_shm_main_pool_size = 3;
     fusion_config->mst_directfb_main_pool_size = 16;
     fusion_config->mst_directfb_data_pool_size = 16;
}

void
__Fusion_conf_deinit()
{
}

/**********************************************************************************************************************/

void fusion_config_init(void)
{
    if(fusion_config->fusion_dir == NULL)
        fusion_config->fusion_dir = D_STRDUP( "/dev");

    if(fusion_config->fusion_shm_dir == NULL)
        fusion_config->fusion_shm_dir = D_STRDUP("/dev/shm");

	if(DFB_SUPPORT_AN == 0)
		fusion_config->tmpfs=D_STRDUP( "/tmp");
	else if(DFB_SUPPORT_AN == 1) //for android two world
	 	fusion_config->tmpfs=D_STRDUP( "/data/vendor/tmp");
	else
	{
		printf("fusion_config->tmpfs set to default!\n");
		fusion_config->tmpfs=D_STRDUP( "/tmp");
	}
}

#if USE_HASH_TABLE_SREACH

////////////////////////////////////////
// start of hashtable

typedef struct data_struct_s
{
	const char *key_string;
	void (*FuncPtr)(char *value);

} data_struct_t;

struct hsearch_data hash_fusion;


void FUN___debugshm ( char *value);

#if !USE_SIZE_OPTIMIZATION

void FUN___tmpfs ( char *value);
void FUN___shmfile_group ( char *value);
void FUN___force_slave ( char *value);
void FUN___no_force_slave ( char *value);
void FUN___debugshm ( char *value);
void FUN___no_debugshm ( char *value);
void FUN___madv_remove ( char *value);
void FUN___no_madv_remove ( char *value);
void FUN___mst_shm_main_pool_size ( char *value);
void FUN___mst_directfb_main_pool_size ( char *value);
void FUN___mst_directfb_data_pool_size ( char *value);
void FUN___fusion_world_virtual_addr ( char *value);
void FUN___fusion_dir ( char *value);
void FUN___fusion_shm_dir ( char *value);
void FUN___mst_fusion_fix_addr_enable ( char *value);
void FUN___mst_fusion_shm_access_mode( char *value);

#endif

data_struct_t FusionConfigTable[] =
{
    { "debugshm", FUN___debugshm },

#if !USE_SIZE_OPTIMIZATION
    { "tmpfs", FUN___tmpfs },
    { "shmfile-group", FUN___shmfile_group },
    { "force-slave", FUN___force_slave },
    { "no-force-slave", FUN___no_force_slave },
    { "no-debugshm", FUN___no_debugshm },
    { "madv-remove", FUN___madv_remove },
    { "no-madv-remove", FUN___no_madv_remove },
    { "mst_shm_main_pool_size", FUN___mst_shm_main_pool_size },
    { "mst_directfb_main_pool_size", FUN___mst_directfb_main_pool_size },
    { "mst_directfb_data_pool_size", FUN___mst_directfb_data_pool_size },
    { "fusion_world_virtual_addr", FUN___fusion_world_virtual_addr },
    { "fusion-dir", FUN___fusion_dir },
    { "fusion-shm-dir", FUN___fusion_shm_dir },
    { "mst_fusion_fix_addr_enable", FUN___mst_fusion_fix_addr_enable },
    { "mst_fusion_shm_access_mode", FUN___mst_fusion_shm_access_mode },
#endif

};

static bool bCreateFusionTable = false;

void CreateFusionConfigHashTable()
{
    ENTRY e, *ep;
    int i, size, ret;
    size = sizeof(FusionConfigTable)/sizeof(data_struct_t);     

    //hcreate(size);

    memset( &hash_fusion, 0, sizeof(hash_fusion) );

    ret = hcreate_r(size, &hash_fusion);
    if(!ret) {
#if DIRECT_BUILD_TEXT        
        if (errno == ENOMEM)
            printf("DFB hashtable NOMEM, %s, %d\n", __FUNCTION__, __LINE__);
#endif
        
        printf("DFB hashtable ERROR, %s, %d\n", __FUNCTION__, __LINE__);
    }

    for (i = 0; i < size; i++) 
    {
        e.key = FusionConfigTable[i].key_string;
        /* data is just an integer, instead of a
          pointer to something */
        e.data = (void*)FusionConfigTable[i].FuncPtr;

        //ep = hsearch(e, ENTER);

        ret = hsearch_r(e, ENTER, &ep, &hash_fusion);
        if(ret == 0) {
        printf("DFB Hashtable is full %s, %d\n", __FUNCTION__, __LINE__);
        }


        /* there should be no failures */
        if (ep == NULL) {
            printf("ERROR %s, %d\n", __FUNCTION__, __LINE__);
            
           fprintf(stderr, "entry failed\n");
           exit(EXIT_FAILURE);
        }
    }
}

bool SearchFusionConfigHashTable( const char* name, const char* value)
{
    if (bCreateFusionTable == false)
    {
        CreateFusionConfigHashTable();
        bCreateFusionTable = true;
    }

    ENTRY e, *ep;
    /* print two entries from the table, and
    show that two are not in the table */
    e.key = name;
    //ep = hsearch(e, FIND);
    hsearch_r( e, FIND, &ep, &hash_fusion );

    //D_INFO("fusion  %9.9s -> %9.9s:%d\n", e.key, ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);

    if (ep == NULL)
        return false;

    //data_struct_t *tableData;


    void (*FuncPtr)(char *value);
    
    FuncPtr = (void*)ep->data;
    FuncPtr(value);

    return true;

}

DirectResult fusion_config_set( const char *name, const char *value )
{
//    printf("\33[0;33;44m name = %s, value =%s \33[0m\n", name, value);

    bool bCheck = true;
    
    bCheck = SearchFusionConfigHashTable(name, value);

    if (bCheck == false)
    {
        if (direct_config_set( name, value ))
            return DR_UNSUPPORTED;
    }
    
    return DR_OK;

}
//end of hashtable
//////////////////////////////////////////

#else

DirectResult
fusion_config_set( const char *name, const char *value )
{
     if (strcmp (name, "tmpfs" ) == 0) {
          if (value) {
               if (fusion_config->tmpfs)
                    D_FREE( fusion_config->tmpfs );
               fusion_config->tmpfs = D_STRDUP( value );
          }
          else {
               D_ERROR("Fusion/Config 'tmpfs': No directory specified!\n");
               return DR_INVARG;
          }
     } else
#if FUSION_BUILD_MULTI
     if (strcmp (name, "shmfile-group" ) == 0) {
          if (value) {
               struct group *group_info;
               
               group_info = getgrnam( value );
               if (group_info)
                    fusion_config->shmfile_gid = group_info->gr_gid;
               else
                    D_PERROR("Fusion/Config 'shmfile-group': Group '%s' not found!\n", value);
          }
          else {
               D_ERROR("Fusion/Config 'shmfile-group': No file group name specified!\n");
               return DR_INVARG;
          }
     } else
#endif
     if (strcmp (name, "force-slave" ) == 0) {
          fusion_config->force_slave = true;
     } else
     if (strcmp (name, "no-force-slave" ) == 0) {
          fusion_config->force_slave = false;
     } else
     if (strcmp (name, "debugshm" ) == 0) {
          fusion_config->debugshm = true;
     } else
     if (strcmp (name, "no-debugshm" ) == 0) {
          fusion_config->debugshm = false;
     } else
     if (strcmp (name, "madv-remove" ) == 0) {
          fusion_config->madv_remove       = true;
          fusion_config->madv_remove_force = true;
     } else
     if (strcmp (name, "no-madv-remove" ) == 0) {
          fusion_config->madv_remove       = false;
          fusion_config->madv_remove_force = true;
     } else
     if (strcmp (name,"mst_shm_main_pool_size") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DR_INVARG;
            }

            if (val < 1 || val >16){
                 // 1MB default value,  4MB netflix oom fix.
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!the range is (1~16) MB\n", name, error );
                 return DR_INVARG;
            }

            fusion_config->mst_shm_main_pool_size= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DR_INVARG;
         }
     }
        else
     if (strcmp (name,"mst_directfb_main_pool_size") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DR_INVARG;
            }

            if (val < 4 || val >16){
                 // 4MB default value,  8MB netflix oom fix.
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!the range is (4~16) MB\n", name, error );
                 return DR_INVARG;
            }

            fusion_config->mst_directfb_main_pool_size= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DR_INVARG;
               }
     }
     else
     if (strcmp (name,"mst_directfb_data_pool_size") == 0) {
         if (value) {
             char *error;
             unsigned long val;
             val = strtoul( value, &error, 10 );

            if (*error) {
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", name, error );
                 return DR_INVARG;
            }

            if (val < 1 || val >32){
                 // 16MB default value
                 D_ERROR( "DirectFB/Config '%s': Error in value '%s'!the range is (1~32) MB\n", name, error );
                 return DR_INVARG;
            }

            fusion_config->mst_directfb_data_pool_size= val;
         }
         else {
               D_ERROR( "DirectFB/Config '%s': No value specified!\n", name );
               return DR_INVARG;
         }
     }
     if (strcmp (name, "secure-fusion" ) == 0) {
          fusion_config->secure_fusion = true;
     } else
     if (strcmp (name, "no-secure-fusion" ) == 0) {
          fusion_config->secure_fusion = false;
     } else
     if (direct_config_set( name, value ))
          return DR_UNSUPPORTED;

     return DR_OK;
}
#endif


void fusion_config_destroy(void)
{
    direct_config_destroy();

    D_SAFE_FREE(fusion_config->tmpfs);
    D_SAFE_FREE(fusion_config->fusion_dir);
    D_SAFE_FREE(fusion_config->fusion_shm_dir);
}


#if USE_HASH_TABLE_SREACH

void FUN___debugshm( char *value)
{
    fusion_config->debugshm = true;
}

#if !USE_SIZE_OPTIMIZATION

void FUN___tmpfs( char *value)
{
    if (value) {
       if (fusion_config->tmpfs)
            D_FREE( fusion_config->tmpfs );
       fusion_config->tmpfs = D_STRDUP( value );
    }
}

void FUN___shmfile_group( char *value)
{
    if (value) {
       struct group *group_info;
       
       group_info = getgrnam( value );
       if (group_info)
            fusion_config->shmfile_gid = group_info->gr_gid;
       else
            D_PERROR("Fusion/Config 'shmfile-group': Group '%s' not found!\n", value);
    }
}

void FUN___force_slave( char *value)
{
    fusion_config->force_slave = true;
}

void FUN___no_force_slave( char *value)
{
    fusion_config->force_slave = false;
}

void FUN___no_debugshm( char *value)
{
    fusion_config->debugshm = false;
}

void FUN___madv_remove( char *value)
{
    fusion_config->madv_remove       = true;
    fusion_config->madv_remove_force = true;
}

void FUN___no_madv_remove( char *value)
{
    fusion_config->madv_remove       = false;
    fusion_config->madv_remove_force = true;
}

void FUN___mst_shm_main_pool_size( char *value)
{
    if (value) {
     char *error;
     unsigned long val;
     val = strtoul( value, &error, 10 );

    if (*error) {
         printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

    }

    if (val < 1 || val >16){
         // 1MB default value,  4MB netflix oom fix.  
         printf( "DirectFB/Config '%s': Error in value '%s'!the range is (1~16) MB\n", __FUNCTION__, error );

    }
        
    fusion_config->mst_shm_main_pool_size= val;
    }
}    

void FUN___mst_directfb_main_pool_size( char *value)
{
    if (value) {
     char *error;
     unsigned long val;
     val = strtoul( value, &error, 10 );

    if (*error) {
         printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
    }

    if (val < 4 || val >16){
         // 1MB default value,  4MB netflix oom fix.
         printf( "DirectFB/Config '%s': Error in value '%s'!the range is (4~16) MB\n", __FUNCTION__, error );
    }

    fusion_config->mst_directfb_main_pool_size= val;
    }
}

void FUN___mst_directfb_data_pool_size( char *value)
{
    if (value) {
     char *error;
     unsigned long val;
     val = strtoul( value, &error, 10 );

    if (*error) {
         printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
    }

    if (val < 1 || val >32){
         // 16MB default value
         printf( "DirectFB/Config '%s': Error in value '%s'!the range is (1~32) MB\n", __FUNCTION__, error );
    }

    fusion_config->mst_directfb_data_pool_size= val;
    }
}

void FUN___fusion_world_virtual_addr ( char *value)
{
    if (value)
    {
        char *error;
        unsigned long val;
        val = strtoul( value, &error, 16 );

        if (*error)
        {
            D_ERROR( "DirectFB/Config fusion_world_virtual_addr error!\n");
            return ;
        }

        fusion_config->mst_fusion_world_virtual_addr = val;
    }
}


void FUN___fusion_dir( char *value)
{
    if (value)
    {
        char fusionPath[32];

        if (fusion_config->fusion_dir)
            D_FREE( fusion_config->fusion_dir );

        fusion_config->fusion_dir = D_STRDUP( value );

        snprintf( fusionPath, sizeof(fusionPath), "%s/fusion",
                  fusion_config->fusion_dir );

        D_INFO("fusion path=%s \n", fusionPath);

        // create new fusion folder
        if ( mkdir(fusionPath, 0775) < 0 && errno != EEXIST )
        {
            printf("Fusion/Config 'fusion-dir': Could not create directory : %s!\n", fusionPath);
            return ;
        }
        else
        {
            /*
            create fusion node ID 0~7
            this part is the same as mknod /fusion/ID 237 0
            major device ID = 237
            minor device ID = 0
            */
            mode_t mode = S_IFCHR | 0777;
            dev_t dev = makedev(237, 0);

            int i = 0;
            for ( i = 0 ; i < 8 ; i++)
            {
                char nodeID[32];
                snprintf( nodeID, sizeof(nodeID), "%s/fusion/%d",
                          fusion_config->fusion_dir, i);

                if ( mknod(nodeID, mode, dev) < 0 && errno != EEXIST )
                {
                    printf("Fusion/Config 'fusion-dir': mknod error!\n");
                    return ;
                }
            }
        }
    }
}

void FUN___fusion_shm_dir( char *value)
{
    if (value) {

        if (fusion_config->fusion_shm_dir)
            D_FREE( fusion_config->fusion_shm_dir );

        fusion_config->fusion_shm_dir = D_STRDUP( value );
    }
}

void FUN___mst_fusion_fix_addr_enable( char *value)
{
    fusion_config->mst_fusion_fix_addr_enable= true;
}

void FUN___mst_fusion_shm_access_mode(char * value)
{
    if (value) {
        char *error;
        unsigned int val;

        val = strtoul( value, &error, 8 );

        if (*error) {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
        }

        fusion_config->mst_fusion_shm_access_mode = val;
    }

}

#endif  // end of USE_SIZE_OPTIMIZATION
#endif  // end of USE_HASH_TABLE_SREACH

