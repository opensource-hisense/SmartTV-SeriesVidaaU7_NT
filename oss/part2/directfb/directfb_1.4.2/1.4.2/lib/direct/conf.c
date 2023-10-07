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

#include <config.h>
#include <search.h>

#include <direct/conf.h>
#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/util.h>


static DirectConfig config;


DirectConfig *direct_config       = &config;
const char   *direct_config_usage =
     "libdirect options:\n"
     "  memcpy=<method>                Skip memcpy() probing (help = show list)\n"
     "  [no-]quiet                     Disable text output except debug messages or direct logs\n"
     "  [no-]quiet=<type>              Only quiet certain types (cumulative with 'quiet')\n"
     "                                 [ info | warning | error | once | unimplemented ]\n"
     "  [no-]debug=<domain>            Configure debug domain (if no domain, sets default for unconfigured domains)\n"
     "  debug-all                      Enable all debug output (regardless of domain configuration)\n"
     "  debug-none                     Disable all debug output (regardless of all other debug options)\n"
     "  [no-]debugmem                  Enable memory allocation tracking\n"
     "  [no-]trace                     Enable stack trace support\n"
     "  log-file=<name>                Write all messages to a file\n"
     "  log-udp=<host>:<port>          Send all messages via UDP to host:port\n"
     "  fatal-level=<level>            Abort on NONE, ASSERT (default) or ASSUME (incl. assert)\n"
     "  [no-]fatal-break               Abort on BREAK (default)\n"
     "  dont-catch=<num>[[,<num>]...]  Don't catch these signals\n"
     "  [no-]sighandler                Enable signal handler\n"
     "  [no-]thread-block-signals      Block all signals in new threads?\n"
     "  disable-module=<module_name>   suppress loading this module\n"
     "  module-dir=<directory>         Override default module search directory (default = $libdir/directfb-x.y-z)\n"
     "  thread-priority-scale=<100th>  Apply scaling factor on thread type based priorities\n"
     "\n";

/**********************************************************************************************************************/

void
__D_conf_init()
{
     direct_config->log_level             = DIRECT_LOG_DEBUG_0;
     direct_config->trace                 = true;
     direct_config->sighandler            = true;

     direct_config->fatal                 = DCFL_ASSERT;
     direct_config->fatal_break           = true;
     direct_config->thread_block_signals  = true;
     direct_config->thread_priority_scale = 100;
     direct_config->bdestroy              = false;
     direct_config->debug_file            = false;
#if DFB_SUPPORT_AN
     direct_config->debug_file_dir        = D_STRDUP( "/mnt/vendor/tmp/dfb_file_log.txt" );
#else
     direct_config->debug_file_dir        = D_STRDUP( "/tmp/dfb_file_log.txt" );
#endif
}

void
__D_conf_deinit()
{
}

/**********************************************************************************************************************/
#if USE_HASH_TABLE_SREACH

////////////////////////////////////////
// start of hashtable

typedef struct data_struct_s
{
	const char *key_string;
	void (*FuncPtr)(char *value);

} data_struct_t;

struct hsearch_data hash_direct;

void FUN___module_dir ( char *value);
void FUN___debug ( char *value);
void FUN___quiet ( char *value);
void FUN___no_sighandler ( char *value);

#if !USE_SIZE_OPTIMIZATION

void FUN___disable_module ( char *value);
void FUN___memcpy ( char *value);
void FUN___no_quiet ( char *value);
void FUN___no_debug ( char *value);
void FUN___debugmem ( char *value);
void FUN___no_debugmem ( char *value);
void FUN___trace ( char *value);
void FUN___no_trace ( char *value);
void FUN___log_file ( char *value);
void FUN___log_udp ( char *value);
void FUN___fatal_level ( char *value);
void FUN___fatal_break ( char *value);
void FUN___no_fatal_break ( char *value);
void FUN___sighandler ( char *value);
void FUN___dont_catch ( char *value);
void FUN___thread_block_signals ( char *value);
void FUN___no_thread_block_signals ( char *value);
void FUN___thread_priority_scale ( char *value);
void FUN___thread_priority ( char *value);
void FUN___thread_scheduler ( char *value);
void FUN___thread_stacksize ( char *value);

#endif


data_struct_t DirectConfigTable[] =
{

    { "module-dir", FUN___module_dir },
    { "quiet", FUN___quiet },
    { "debug", FUN___debug },
    { "no-sighandler", FUN___no_sighandler },

#if !USE_SIZE_OPTIMIZATION

    { "memcpy", FUN___memcpy },
    { "disable-module", FUN___disable_module },
    { "no-quiet", FUN___no_quiet },
    { "no-debug", FUN___no_debug },
    { "debugmem", FUN___debugmem },
    { "no-debugmem", FUN___no_debugmem },
    { "trace", FUN___trace },
    { "no-trace", FUN___no_trace },
    { "log-file", FUN___log_file },
    { "log-udp", FUN___log_udp },
    { "fatal-level", FUN___fatal_level },
    { "fatal-break", FUN___fatal_break },
    { "no-fatal-break", FUN___no_fatal_break },
    { "sighandler", FUN___sighandler },

    { "dont-catch", FUN___dont_catch },
    { "thread_block_signals", FUN___thread_block_signals },
    { "no-thread_block_signals", FUN___no_thread_block_signals },
    { "thread-priority-scale", FUN___thread_priority_scale },
    { "thread-priority", FUN___thread_priority },
    { "thread-scheduler", FUN___thread_scheduler },
    { "thread-stacksize", FUN___thread_stacksize },
    { "debug_file", FUN___debug_file },
    { "debug_file_dir", FUN___debug_file_dir },
#endif

};

static bool bCreateDirectTable = false;

void CreateDirectConfigHashTable (void)
{
    ENTRY e, *ep;
    int i, size, ret;
    size = sizeof(DirectConfigTable)/sizeof(data_struct_t);     

    //hcreate(size);

    memset( &hash_direct, 0, sizeof(hash_direct) );

    ret = hcreate_r(size, &hash_direct);
    if(!ret) {
        if (errno == ENOMEM)
            printf("DFB hashtable NOMEM, %s, %d\n", __FUNCTION__, __LINE__);
        
        printf("DFB hashtable ERROR, %s, %d\n", __FUNCTION__, __LINE__);
    }

    for (i = 0; i < size; i++) 
    {
        e.key = (char*) DirectConfigTable[i].key_string;
        /* data is just an integer, instead of a
          pointer to something */
        e.data = (void*)DirectConfigTable[i].FuncPtr;

        //ep = hsearch(e, ENTER);

        ret = hsearch_r(e, ENTER, &ep, &hash_direct);
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

bool SearchDirectConfigHashTable( const char* name, const char* value)
{
    if (bCreateDirectTable == false)
    {    
        CreateDirectConfigHashTable();
        bCreateDirectTable = true;
    }

    ENTRY e, *ep;
    /* print two entries from the table, and
    show that two are not in the table */
    e.key = (char*) name;
    //ep = hsearch(e, FIND);
    hsearch_r( e, FIND, &ep, &hash_direct );

    //D_INFO("fusion  %9.9s -> %9.9s:%d\n", e.key, ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);

    if (ep == NULL)
        return false;

    //data_struct_t *tableData;


    void (*FuncPtr)(char *value);
    
    FuncPtr = (void*)ep->data;
    FuncPtr((char*)value);

    return true;

}

DirectResult direct_config_set( const char *name, const char *value )
{
    bool bCheck = true;
    
    bCheck = SearchDirectConfigHashTable(name, value);

    if (bCheck == false)
    {
          return DR_UNSUPPORTED;
    }
    
    return DR_OK;

}

//end of hashtable
//////////////////////////////////////////

#else

DirectResult
direct_config_set( const char *name, const char *value )
{
     if (direct_strcmp (name, "disable-module" ) == 0) {
          if (value) {
               int n = 0;

               while (direct_config->disable_module &&
                      direct_config->disable_module[n])
                    n++;

               direct_config->disable_module = (char**) D_REALLOC( direct_config->disable_module,
                                                                   sizeof(char*) * (n + 2) );

               direct_config->disable_module[n] = D_STRDUP( value );
               direct_config->disable_module[n+1] = NULL;
          }
          else {
               D_ERROR("Direct/Config '%s': No module name specified!\n", name);
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "module-dir" ) == 0) {
          if (value) {
               if (direct_config->module_dir)
                    D_FREE( direct_config->module_dir );
               direct_config->module_dir = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'module-dir': No directory name specified!\n");
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "memcpy" ) == 0) {
          if (value) {
               if (direct_config->memcpy)
                    D_FREE( direct_config->memcpy );
               direct_config->memcpy = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config '%s': No method specified!\n", name);
               return DR_INVARG;
          }
     }
     else
          if (direct_strcmp (name, "quiet" ) == 0 || strcmp (name, "no-quiet" ) == 0) {
          /* Enable/disable all at once by default. */
          DirectMessageType type = DMT_ALL;

          /* Find out the specific message type being configured. */
          if (value) {
               if (!strcmp( value, "info" ))           type = DMT_INFO;              else
               if (!strcmp( value, "warning" ))        type = DMT_WARNING;           else
               if (!strcmp( value, "error" ))          type = DMT_ERROR;             else
               if (!strcmp( value, "once" ))           type = DMT_ONCE;              else
               if (!strcmp( value, "untested" ))       type = DMT_UNTESTED;          else
               if (!strcmp( value, "unimplemented" ))  type = DMT_UNIMPLEMENTED; 
               else {
                    D_ERROR( "DirectFB/Config '%s': Unknown message type '%s'!\n", name, value );
                    return DR_INVARG;
               }
          }

          /* Set/clear the corresponding flag in the configuration. */
          if (name[0] == 'q')
               D_FLAGS_SET( direct_config->quiet, type );
          else
               D_FLAGS_CLEAR( direct_config->quiet, type );
     }
     else
          if (direct_strcmp (name, "no-quiet" ) == 0) {
          direct_config->quiet = DMT_NONE;
     }
     else
          if (direct_strcmp (name, "debug" ) == 0) {
          if (value) {
               DirectLogDomainConfig config = {0};

               D_UNIMPLEMENTED();

               if (config.level < DIRECT_LOG_DEBUG)
                    config.level = DIRECT_LOG_DEBUG;

               direct_log_domain_configure( value, &config );
          }
          else if (direct_config->log_level < DIRECT_LOG_DEBUG)
               direct_config->log_level = DIRECT_LOG_DEBUG;
     }
     else
          if (direct_strcmp (name, "no-debug" ) == 0) {
          if (value) {
               DirectLogDomainConfig config = {0};

               D_UNIMPLEMENTED();

               if (config.level > DIRECT_LOG_DEBUG_0)
                    config.level = DIRECT_LOG_DEBUG_0;
                    
               direct_log_domain_configure( value, &config );
          }
          else if (direct_config->log_level > DIRECT_LOG_DEBUG_0)
               direct_config->log_level = DIRECT_LOG_DEBUG_0;
     }
     else
          if (direct_strcmp (name, "log-all" ) == 0) {
          direct_config->log_all = true;
     }
     else
          if (direct_strcmp (name, "log-none" ) == 0) {
          direct_config->log_none = true;
     }
     else
          if (direct_strcmp (name, "debugmem" ) == 0) {
          direct_config->debugmem = true;
     }
     else
          if (direct_strcmp (name, "no-debugmem" ) == 0) {
          direct_config->debugmem = false;
     }
     else
          if (direct_strcmp (name, "trace" ) == 0) {
          direct_config->trace = true;
     }
     else
          if (direct_strcmp (name, "no-trace" ) == 0) {
          direct_config->trace = false;
     }
     else
          if (direct_strcmp (name, "log-file" ) == 0 || strcmp (name, "log-udp" ) == 0) {
          if (value) {
               DirectResult  ret;
               DirectLog    *log;

               ret = direct_log_create( strcmp(name,"log-udp") ? DLT_FILE : DLT_UDP, value, &log );
               if (ret)
                    return ret;

               if (direct_config->log)
                    direct_log_destroy( direct_config->log );

               direct_config->log = log;

               direct_log_set_default( log );
          }
          else {
               if (direct_strcmp(name,"log-udp"))
                    D_ERROR("Direct/Config '%s': No file name specified!\n", name);
               else
                    D_ERROR("Direct/Config '%s': No host and port specified!\n", name);
               return DR_INVARG;
          }
     }
     else
          if (direct_strcmp (name, "fatal-level" ) == 0) {
          if (direct_strcasecmp (value, "none" ) == 0) {
               direct_config->fatal = DCFL_NONE;
          }
          else
               if (direct_strcasecmp (value, "assert" ) == 0) {
               direct_config->fatal = DCFL_ASSERT;
          }
          else
               if (direct_strcasecmp (value, "assume" ) == 0) {
               direct_config->fatal = DCFL_ASSUME;
          }
          else {
               D_ERROR("Direct/Config '%s': Unknown level specified (use 'none', 'assert', 'assume')!\n", name);
               return DR_INVARG;
          }
     }
     else
          if (direct_strcmp (name, "fatal-break" ) == 0) {
          direct_config->fatal_break = true;
     }
     else
          if (direct_strcmp (name, "no-fatal-break" ) == 0) {
          direct_config->fatal_break = false;
     }
     else
          if (direct_strcmp (name, "sighandler" ) == 0) {
          direct_config->sighandler = true;
     }
     else
          if (direct_strcmp (name, "no-sighandler" ) == 0) {
          direct_config->sighandler = false;
     }
     else
          if (direct_strcmp (name, "dont-catch" ) == 0) {
          if (value) {
               char *signals   = D_STRDUP( value );
               char *p = NULL, *r, *s = signals;

               while ((r = direct_strtok_r( s, ",", &p ))) {
                    char          *error;
                    unsigned long  signum;

                    direct_trim( &r );

                    signum = direct_strtoul( r, &error, 10 );

                    if (*error) {
                         D_ERROR( "Direct/Config '%s': Error in number at '%s'!\n", name, error );
                         D_FREE( signals );
                         return DR_INVARG;
                    }

                    sigaddset( &direct_config->dont_catch, signum );

                    s = NULL;
               }

               D_FREE( signals );
          }
          else {
               D_ERROR("Direct/Config '%s': No signals specified!\n", name);
               return DR_INVARG;
          }
     }
     else
          if (direct_strcmp (name, "thread_block_signals") == 0) {
          direct_config->thread_block_signals = true;
     }
     else
          if (direct_strcmp (name, "no-thread_block_signals") == 0) {
          direct_config->thread_block_signals = false;
     } else
     if (direct_strcmp (name, "thread-priority-scale" ) == 0) {
          if (value) {
               int scale;

               if (direct_sscanf( value, "%d", &scale ) < 1) {
                    D_ERROR("Direct/Config '%s': Could not parse value!\n", name);
                    return DR_INVARG;
               }

               direct_config->thread_priority_scale = scale;
          }
          else {
               D_ERROR("Direct/Config '%s': No value specified!\n", name);
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "thread-priority" ) == 0) {  /* Must be moved to lib/direct/conf.c in trunk! */
          if (value) {
               int priority;

               if (direct_sscanf( value, "%d", &priority ) < 1) {
                    D_ERROR("Direct/Config '%s': Could not parse value!\n", name);
                    return DR_INVARG;
               }

               direct_config->thread_priority = priority;
          }
          else {
               D_ERROR("Direct/Config '%s': No value specified!\n", name);
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "thread-scheduler" ) == 0) {  /* Must be moved to lib/direct/conf.c in trunk! */
          if (value) {
               if (direct_strcmp( value, "other" ) == 0) {
                    direct_config->thread_scheduler = DCTS_OTHER;
               } else
               if (direct_strcmp( value, "fifo" ) == 0) {
                    direct_config->thread_scheduler = DCTS_FIFO;
               } else
               if (direct_strcmp( value, "rr" ) == 0) {
                    direct_config->thread_scheduler = DCTS_RR;
               } else {
                    D_ERROR( "Direct/Config '%s': Unknown scheduler '%s'!\n", name, value );
                    return DR_INVARG;
               }
          }
          else {
               D_ERROR( "Direct/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "thread-stacksize" ) == 0) {  /* Must be moved to lib/direct/conf.c in trunk! */
          if (value) {
               int size;

               if (direct_sscanf( value, "%d", &size ) < 1) {
                    D_ERROR( "Direct/Config '%s': Could not parse value!\n", name );
                    return DR_INVARG;
               }

               direct_config->thread_stack_size = size;
          }
          else {
               D_ERROR( "Direct/Config '%s': No value specified!\n", name );
               return DR_INVARG;
          }
     } else
     if (direct_strcmp (name, "debug_file" ) == 0) {
          if (value) {
                if (strcmp( value, "true" ) == 0)
                {
                    direct_config->debug_file = true;
                }
                else if(strcmp( value, "false" ) == 0)
                {
                    direct_config->debug_file = false;
                }
                else
                {
                    D_ERROR( "Direct/Config '%s': No value specified!\n", name );
                    return DR_INVARG;
                }
          }
          else {
               direct_config->debug_file = true;
          }
     }  else
     if (direct_strcmp (name, "debug_file_dir" ) == 0) {
          if (value) {
               if (direct_config->debug_file_dir)
                    D_FREE( direct_config->debug_file_dir );
               direct_config->debug_file_dir = D_STRDUP( value );
          }
          else {
               D_ERROR("Direct/Config 'debug_file_dir': No directory name specified!\n");
               return DR_INVARG;
          }
     } 
     else
          return DR_UNSUPPORTED;

     return DR_OK;
}
#endif

void direct_config_destroy(void)
{
    D_SAFE_FREE(direct_config->module_dir );
    D_SAFE_FREE(direct_config->memcpy);
    D_SAFE_FREE(direct_config->debug_file_dir);

    int n = 0;
    while (direct_config->disable_module && direct_config->disable_module[n])
    {
          D_SAFE_FREE(direct_config->disable_module[n]);
          n++;
    }
    D_SAFE_FREE(direct_config->disable_module);

    if (direct_config->log)
        direct_log_destroy( direct_config->log );

}


#if USE_HASH_TABLE_SREACH
void FUN___module_dir(char *value)
{
  if (value) {
       if (direct_config->module_dir)
            D_FREE( direct_config->module_dir );
       direct_config->module_dir = D_STRDUP( value );
  }
  else {
       printf("Direct/Config 'module-dir': No directory name specified!\n");
       
  }
}

void FUN___quiet(char *value)
{
  /* Enable/disable all at once by default. */
  DirectMessageType type = DMT_ALL;

  /* Find out the specific message type being configured. */
  if (value) {
       if (!strcmp( value, "info" ))           type = DMT_INFO;              else
       if (!strcmp( value, "warning" ))        type = DMT_WARNING;           else
       if (!strcmp( value, "error" ))          type = DMT_ERROR;             else
       if (!strcmp( value, "once" ))           type = DMT_ONCE;              else
       if (!strcmp( value, "unimplemented" ))  type = DMT_UNIMPLEMENTED;
       else {
            printf( "DirectFB/Config '%s': Unknown message type '%s'!\n", __FUNCTION__, value );
            
       }
  }
  /* Set/clear the corresponding flag in the configuration. */
       direct_config->quiet |= type;
}

void FUN___debug(char *value)
{
  if (value)
       direct_debug_config_domain( value, true );
  else
       direct_config->debug = true;
}

void FUN___no_sighandler(char *value)
{
  direct_config->sighandler = false;
}

#if !USE_SIZE_OPTIMIZATION

void FUN___disable_module(char *value)
{
  if (value) {
       int n = 0;

       while (direct_config->disable_module &&
              direct_config->disable_module[n])
            n++;

       direct_config->disable_module = D_REALLOC( direct_config->disable_module,
                                                  sizeof(char*) * (n + 2) );

       direct_config->disable_module[n] = D_STRDUP( value );
       direct_config->disable_module[n+1] = NULL;
  }
  else {
       printf("Direct/Config '%s': No module name specified!\n", __FUNCTION__);

  }
}

void FUN___memcpy(char *value)
{
  if (value) {
       if (direct_config->memcpy)
            D_FREE( direct_config->memcpy );
       direct_config->memcpy = D_STRDUP( value );
  }
  else {
       printf("Direct/Config '%s': No method specified!\n", __FUNCTION__);

  }
}

void FUN___no_quiet(char *value)
{
  /* Enable/disable all at once by default. */
  DirectMessageType type = DMT_ALL;
  
  /* Find out the specific message type being configured. */
  if (value) {
       if (!strcmp( value, "info" ))           type = DMT_INFO;              else
       if (!strcmp( value, "warning" ))        type = DMT_WARNING;           else
       if (!strcmp( value, "error" ))          type = DMT_ERROR;             else
       if (!strcmp( value, "once" ))           type = DMT_ONCE;              else
       if (!strcmp( value, "unimplemented" ))  type = DMT_UNIMPLEMENTED;
       else {
            printf( "DirectFB/Config '%s': Unknown message type '%s'!\n", __FUNCTION__, value ); 
       }

       /* Set/clear the corresponding flag in the configuration. */
       direct_config->quiet &= ~type;
       
  }
  else {
       direct_config->quiet = false;
  }

}

void FUN___no_debug(char *value)
{
  if (value)
       direct_debug_config_domain( value, false );
  else
       direct_config->debug = false;
}

void FUN___debugmem(char *value)
{
  direct_config->debugmem = true;
}

void FUN___no_debugmem(char *value)
{
  direct_config->debugmem = false;
}

void FUN___trace(char *value)
{
  direct_config->trace = true;
}

void FUN___no_trace(char *value)
{
  direct_config->trace = false;
}

void FUN___log_udp(char *value)
{
  if (value) {
       DirectResult  ret;
       DirectLog    *log;

       ret = direct_log_create( DLT_UDP, value, &log );
       if (ret)
            return;

       if (direct_config->log)
            direct_log_destroy( direct_config->log );

       direct_config->log = log;

       direct_log_set_default( log );
  }
  else {
            printf("Direct/Config '%s': No host and port specified!\n", __FUNCTION__);
       
  }
}

void FUN___log_file(char *value)
{
  if (value) {
       DirectResult  ret;
       DirectLog    *log;

       ret = direct_log_create( DLT_FILE, value, &log );
       if (ret)
            return;

       if (direct_config->log)
            direct_log_destroy( direct_config->log );

       direct_config->log = log;

       direct_log_set_default( log );
  }
  else {
            printf("Direct/Config '%s': No file name specified!\n", __FUNCTION__);

       
  }
}

void FUN___fatal_level(char *value)
{
    if (strcasecmp (value, "none" ) == 0) {
       direct_config->fatal = DCFL_NONE;
    }
    else
       if (strcasecmp (value, "assert" ) == 0) {
       direct_config->fatal = DCFL_ASSERT;
    }
    else
       if (strcasecmp (value, "assume" ) == 0) {
       direct_config->fatal = DCFL_ASSUME;
    }
    else {
       printf("Direct/Config '%s': Unknown level specified (use 'none', 'assert', 'assume')!\n", __FUNCTION__);
       
    }
}

void FUN___fatal_break(char *value)
{
  direct_config->fatal_break = true;
}

void FUN___no_fatal_break(char *value)
{
  direct_config->fatal_break = false;
}

void FUN___sighandler(char *value)
{
  direct_config->sighandler = true;
}

void FUN___dont_catch(char *value)
{
  if (value) {
       char *signals   = D_STRDUP( value );
       char *p = NULL, *r, *s = signals;

       while ((r = strtok_r( s, ",", &p ))) {
            char          *error;
            unsigned long  signum;

            direct_trim( &r );

            signum = strtoul( r, &error, 10 );

            if (*error) {
                 printf( "Direct/Config '%s': Error in number at '%s'!\n", __FUNCTION__, error );
                 D_FREE( signals );
                 
            }

            sigaddset( &direct_config->dont_catch, signum );

            s = NULL;
       }

       D_FREE( signals );
  }
  else {
       printf("Direct/Config '%s': No signals specified!\n", __FUNCTION__);
       
  }
}

void FUN___thread_block_signals(char *value)
{
  direct_config->thread_block_signals = true;
}

void FUN___no_thread_block_signals(char *value)
{
  direct_config->thread_block_signals = false;
}

void FUN___thread_priority_scale(char *value)
{
  if (value) {
       int scale;

       if (sscanf( value, "%d", &scale ) < 1) {
            printf("Direct/Config '%s': Could not parse value!\n", __FUNCTION__);
            
       }

       direct_config->thread_priority_scale = scale;
  }
  else {
       printf("Direct/Config '%s': No value specified!\n", __FUNCTION__);
       
  }
}

void FUN___thread_priority(char *value)
{  
  if (value) {
       int priority;

       if (sscanf( value, "%d", &priority ) < 1) {
            printf("Direct/Config '%s': Could not parse value!\n", __FUNCTION__);
            
       }

       direct_config->thread_priority = priority;
  }
  else {
       printf("Direct/Config '%s': No value specified!\n", __FUNCTION__);
       
  }
}

void FUN___thread_scheduler(char *value)
{  
    if (value) {
       if (strcmp( value, "other" ) == 0) {
            direct_config->thread_scheduler = DCTS_OTHER;
       } else
       if (strcmp( value, "fifo" ) == 0) {
            direct_config->thread_scheduler = DCTS_FIFO;
       } else
       if (strcmp( value, "rr" ) == 0) {
            direct_config->thread_scheduler = DCTS_RR;
       } else {
            printf( "Direct/Config '%s': Unknown scheduler '%s'!\n", __FUNCTION__, value );
            
       }
    }
    else {
       printf( "Direct/Config '%s': No value specified!\n", __FUNCTION__ );
       
    }
}

void FUN___thread_stacksize(char *value)
{  
  if (value) {
       int size;

       if (sscanf( value, "%d", &size ) < 1) {
            printf( "Direct/Config '%s': Could not parse value!\n", __FUNCTION__ );
            
       }

       direct_config->thread_stack_size = size;
  }
  else {
       printf( "Direct/Config '%s': No value specified!\n", __FUNCTION__ );
       
  }
}

void FUN___debug_file(char *value)
{  
    if (value) {
       if (strcmp( value, "true" ) == 0) {
            direct_config->debug_file = true;
       } else
       if (strcmp( value, "false" ) == 0) {
            direct_config->debug_file = false;
       } else {
            printf( "Direct/Config '%s': Unknown scheduler '%s'!\n", __FUNCTION__, value );
            
       }
    } else {
        direct_config->debug_file = true;
    }
}

void FUN___debug_file_dir(char *value)
{
  if (value) {
       if (direct_config->debug_file_dir)
            D_FREE( direct_config->debug_file_dir );
       direct_config->debug_file_dir = D_STRDUP( value );
  }
  else {
       printf("Direct/Config 'module-dir': No directory name specified!\n");
       
  }
}
#endif  // end of USE_SIZE_OPTIMIZATION
#endif  // end of USE_HASH_TABLE_SREACH


