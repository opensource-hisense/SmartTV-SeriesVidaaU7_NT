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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include <direct/conf.h>
#include <direct/direct.h>
#include <direct/interface.h>
#include <direct/log.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <directfb.h>
#include <directfb_version.h>

#include <misc/conf.h>

#if !DIRECTFB_BUILD_PURE_VOODOO
#include <unistd.h>

#include <direct/thread.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layers.h>
#include <core/state.h>
#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/windows.h>
#include <core/windowstack.h>
#include <core/wm.h>

#include <gfx/convert.h>

#include <display/idirectfbsurface.h>
#endif

#include <idirectfb.h>
#ifdef CC_DFB_DEBUG_SUPPORT
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <pthread.h>
#endif


#ifndef DIRECTFB_VERSION_VENDOR
#define DIRECTFB_VERSION_VENDOR
#endif


//directfbrc path
#define DIRECTFBRC_PATH   "/tvconfig/config"

pthread_mutex_t  g_dfb_lock_counter_mutex= PTHREAD_MUTEX_INITIALIZER;
unsigned volatile int g_dfb_lock_flag = 0;

static DFBResult CreateRemote( const char *host, int session, IDirectFB **ret_interface );

/*
 * Version checking
 */
const unsigned int directfb_major_version = DIRECTFB_MAJOR_VERSION;
const unsigned int directfb_minor_version = DIRECTFB_MINOR_VERSION;
const unsigned int directfb_micro_version = DIRECTFB_MICRO_VERSION;
const unsigned int directfb_binary_age    = DIRECTFB_BINARY_AGE;
const unsigned int directfb_interface_age = DIRECTFB_INTERFACE_AGE;

const char *
DirectFBCheckVersion( unsigned int required_major,
                      unsigned int required_minor,
                      unsigned int required_micro )
{
     if (required_major > DIRECTFB_MAJOR_VERSION)
          return "DirectFB version too old (major mismatch)";
     if (required_major < DIRECTFB_MAJOR_VERSION)
          return "DirectFB version too new (major mismatch)";
     if (required_minor > DIRECTFB_MINOR_VERSION)
          return "DirectFB version too old (minor mismatch)";
     if (required_minor < DIRECTFB_MINOR_VERSION)
          return "DirectFB version too new (minor mismatch)";
     if (required_micro < DIRECTFB_MICRO_VERSION - DIRECTFB_BINARY_AGE)
          return "DirectFB version too new (micro mismatch)";
     if (required_micro > DIRECTFB_MICRO_VERSION)
          return "DirectFB version too old (micro mismatch)";

     return NULL;
}

const char *
DirectFBUsageString( void )
{
     return dfb_config_usage();
}

#ifdef CC_DFB_DEBUG_SUPPORT

static DFBResult
DirectFBAllocArgs(int argc, char **retArgv)
{
	int i = 0;
	
	for(i = 0; i < argc; i++)
	{
        retArgv[i] = (char *) malloc(60);
		memset(retArgv[i], 0, 60);
	}

	return DR_OK;
}


static DFBResult
DirectFBFreeArgs(int argc, char *argv[])
{
    int i = 0;

	for(i = 0; i < argc; i++)
	{
        free(argv[i]);                 	
	}

	return DR_OK;	
}


static DFBResult
DirectFBParseArgs(const char *strings, int n, char *retArgv[], int *retArgc)
{
    int index = 0;
    char *temp = (char *) strings;
	char *head = temp;
	
	do
	{
	     temp = strchr(head, ' ');

		 if(temp)
		 {
			  *temp = '\0';
		 }
		 
		 strcpy(retArgv[index], head);
		 D_INFO("%s: %s \n", __FUNCTION__, retArgv[index]);
		 
		 index++;
		 if(n < index)
		 {
		      break;
		 }

		 if(temp)
		 {
		      head = temp + 1;
		      while(*head == ' ')
		 	     head++;	
		 }
	}
	while(temp);

	*retArgc = index;
	
	return DR_OK;
}


static void *
DirectFBDebugThread()
{
    int ret;
	int listenfd = -1, recvfd = -1, maxfd = -1;
	int len;
	bool running = true;
	char recv_buf[1020];
	static fd_set fdset;
	struct sockaddr_un srv_addr;
    struct sockaddr_un clt_addr;

	listenfd  = socket(PF_UNIX, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		 D_INFO("[DFB DBG] cannot create communication socket \n");
		 return;
	}
	
	srv_addr.sun_family = AF_UNIX;
	sprintf(srv_addr.sun_path, "/tmp/DFB_%d.dbg", getpid());
	unlink(srv_addr.sun_path);
	
	ret = bind(listenfd, (struct sockaddr *)& srv_addr, sizeof(struct sockaddr));
	if(ret < 0)
	{
		 D_INFO("[DFB DBG] cannot bind communication socket \n");
		 close(listenfd );
		 return;
	}
	
	ret = listen(listenfd, 1);
	if(ret < 0)
	{
		 D_INFO("[DFB DBG] cannot listen communication socket");
		 close(listenfd );
		 return;
	}	

    while(running)
	{
	     FD_ZERO(&fdset);
         FD_SET(listenfd, &fdset);
         ( recvfd >= 0 )? (FD_SET(recvfd, &fdset)): 0;
		 maxfd = (recvfd > listenfd) ? recvfd: listenfd;
		 
		 switch(select(maxfd + 1, &fdset, 0, 0, 0))
		 {
		      case -1:
		      case 0:
		           D_INFO("[DFB DBG] select error \n");
	          break;

			  default:
			       if(FD_ISSET(listenfd, &fdset))
			       {
			            len = sizeof(clt_addr);
						recvfd = accept(listenfd, (struct sockaddr*)&clt_addr,(socklen_t *) &len);  
			       }
				   else if(( recvfd >= 0 ) && ( FD_ISSET(recvfd, &fdset) ))
				   {
				        int num;
						memset(recv_buf, 0, 1020);
						if((num = read(recvfd, recv_buf,sizeof(recv_buf)) > 0))
						{
						     D_INFO("[DFB DBG] recvbuffer [%s] \n", recv_buf);
							 
							 if(strstr(recv_buf, "quit"))
							 {
                                  running = false;
							 }

							 int argc = 20;
							 char *argv[20];

							 DirectFBAllocArgs(argc, argv);

							 DirectFBParseArgs(recv_buf, argc, argv, &argc);

							 dfb_config_cmdlnset(argc, argv);

							 DirectFBFreeArgs(argc, argv);

							 close(recvfd);
							 recvfd = -1;							 
						}						
				   }
			  break;
		 }
		  
	}

	close(listenfd);

	return NULL;
}

#endif

#include <sys/inotify.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 16 * ( EVENT_SIZE + 16 ) )

#if DFB_SUPPORT_AN
#define DIRECTFBRC_DEBUG_PATH "/mnt/vendor/tmp/directfbrc_debug"
#else
#define DIRECTFBRC_DEBUG_PATH "/tmp/directfbrc_debug"
#endif

#define MAX_STRING_LENGTH  512

static int parsing_debug_pid()
{
    int ret_pid = -1;
    FILE *fp = fopen(DIRECTFBRC_DEBUG_PATH, "r");
    if (fp == NULL){
        D_ERROR("[DFB] %s, %d : open %s failed!\n",__FUNCTION__,__LINE__,DIRECTFBRC_DEBUG_PATH);
        return ret_pid;
    }
    char line[MAX_STRING_LENGTH]={0};
    static char *prog_name=NULL;

    if (fp == NULL) {
        printf("[DFB] %s, can't open directfbrc_debug\n", __FUNCTION__);
        return ret_pid;
    }

    if(fgets( line, MAX_STRING_LENGTH, fp ) == NULL)
        D_ERROR("[DFB] %s, %d : fgets from %s error!\n",__FUNCTION__,__LINE__,DIRECTFBRC_DEBUG_PATH);
    char *name = line;
    char *value = strchr( line, '=' );

    if (value) {
         *value++ = 0;
         direct_trim( &value );
    }
    else {
        if(!fclose(fp))
            D_ERROR("[DFB] %s, %d : close %s failed\n",__FUNCTION__,__LINE__,DIRECTFBRC_DEBUG_PATH);
        return ret_pid;
    }

    direct_trim( &name );

    if (strcmp(name, "#dbg_active_process_pid" ) == 0) {
        direct_sscanf(value, "%d", &ret_pid);
    }
    else if (strcmp(name, "#dbg_active_process_name" ) == 0) {
        if (prog_name == NULL) {
            size_t len = 0;
            char file[MAX_STRING_LENGTH] = {0}, cmdline[MAX_STRING_LENGTH] = {0};

            int ret = snprintf(file, MAX_STRING_LENGTH, "/proc/self/exe");
            if(ret < 0)
                D_ERROR("[DFB] %s, %d : snprintf failed\n",__FUNCTION__, __LINE__);
            len = readlink(file, cmdline, sizeof(cmdline) / sizeof(*cmdline) - 1);

            if( len > 0 && len <= (MAX_STRING_LENGTH-1) )
            {
                cmdline[len] = 0;
                prog_name = strrchr( cmdline , '/');

                D_INFO("[DFB] %s, value=%s, arg=%s, pid=%d\n", __FUNCTION__, value, prog_name, getpid());

                if (prog_name)
                    prog_name++;
                else
                    prog_name = cmdline;
            }
        }

        // compare prog name, and return current pid.
        if (prog_name != NULL && strcmp(value, prog_name) == 0) {
            ret_pid = getpid();
        }
        else
            ret_pid = 0;
    }
    if(!fclose(fp))
        D_ERROR("[DFB] %s, %d : close %s failed\n",__FUNCTION__,__LINE__,DIRECTFBRC_DEBUG_PATH);
    return ret_pid;
}

static void *
ReloadDirectfbrcThread( DirectThread *thread, void *arg )
{
    int fd = 0;
    int wd = -1;
    int ret = -1;
    char buffer[EVENT_BUF_LEN];
    FILE *fp = fopen(DIRECTFBRC_DEBUG_PATH, "r");

    if (fp == NULL) {
        fp = fopen(DIRECTFBRC_DEBUG_PATH, "w");
        if (fp == NULL) {
            printf("[DFB] %s, can't create directfbrc_debug\n", __FUNCTION__);
            return NULL;
        }
        ret = fprintf(fp, "#dbg_active_process_pid=-1\n");
        if(ret < 0)
            D_ERROR("[DFB] %s, %d : print in %s failed\n",__FUNCTION__, __LINE__, DIRECTFBRC_PATH);
        //fprintf(fp, "#dbg_active_process_name=dtv_svc\n");
        fclose(fp);
        D_INFO("[DFB] %s, create %s, pid=%d\n", __FUNCTION__, DIRECTFBRC_PATH, getpid());
    }
    else
        fclose(fp);

    fd = inotify_init();

    wd = inotify_add_watch (fd, DIRECTFBRC_DEBUG_PATH, IN_MODIFY);
    
    D_INFO("[DFB] %s, inotify_init fd=%d, wd=%d, pid=%d\n", __FUNCTION__, fd, wd, getpid());

    while(1)
    {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        if ( (select (fd+1, &rfds, NULL, NULL, NULL)) > 0)
        {
            int len = read(fd, buffer, EVENT_BUF_LEN);
            if (len < 0) {
                continue;
            }

            // check dbg_active_pid
            int pid = parsing_debug_pid();
            // to active debug, get pid >= 0 and as same as current.
            if ( pid >= 0 && pid != getpid()) {
                continue;
            }

            struct inotify_event* event = (struct inotify_event *)&buffer[0];

            D_INFO("[DFB] %s, len=%d, event->mask=0x%x, event->len=%d\n", __FUNCTION__, len, event->mask, event->len);

            if (event->mask & IN_MODIFY || event->mask & IN_IGNORED) {
                dfb_config->mst_debug_layer = false;
                dfb_config->mst_debug_input = 0;

                if (event->mask & IN_IGNORED) {
                    wd = inotify_add_watch (fd, DIRECTFBRC_DEBUG_PATH, IN_MODIFY);
                }

                dfb_config_read( DIRECTFBRC_DEBUG_PATH );
                D_INFO("[DFB] %s, after call dfb_config_init, debug_layer = %d, debug_input = %d, pid=%d\n", __FUNCTION__, dfb_config->mst_debug_layer, dfb_config->mst_debug_input, getpid());
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

DFBResult
DirectFBInit( int *argc, char *(*argv[]) )
{
#ifdef DFB_MEASURE_BOOT_TIME
     dfb_boot_initENV();
#endif

     DFBResult ret = DFB_OK;
#ifdef CC_DFB_DEBUG_SUPPORT
     static bool _init = false;
     pthread_t thread;
#endif

    static DirectThread *thread = NULL;

    static bool first = false;
    if (first == false)
    {
        first = true;
#ifdef DIRECTFBRC_PATH
        if ( getenv("CONFIG_PATH") == NULL )
            setenv( "CONFIG_PATH", DIRECTFBRC_PATH, true );
#endif
     }

     DIRECT_INTERFACE_DBG_DELTA_START();

     DFB_BOOT_GETTIME( DF_BOOT_DIRECTFBINIT, DF_MEASURE_START, DF_BOOT_LV1);


     printf("==========================================\n");
     printf("DFB library build @ ");
     printf(BUILDTIME);
     printf("\n"DIRECTFB_CHANGE_ID);
     printf("\nPID=%d\n", getpid());
     printf("\n");
     printf("===========================================\n");


     DFB_BOOT_GETTIME( DF_BOOT_CONFIG_INIT, DF_MEASURE_START, DF_BOOT_LV2);


     if(dfb_config == NULL)
         ret = dfb_config_init( argc, argv );
     else
     {
         dfb_config->ref++;

         return DFB_OK;
     }


     DFB_BOOT_GETTIME( DF_BOOT_CONFIG_INIT, DF_MEASURE_END, DF_BOOT_LV2);

     if (ret){
       return ret;
     }


#ifdef CC_DFB_DEBUG_SUPPORT

     if(false == _init)
     {
          pthread_create(&thread, NULL, DirectFBDebugThread, NULL);
     }

     _init = true;

#endif

     if (!thread) {
          thread = direct_thread_create(DTT_INPUT, ReloadDirectfbrcThread, NULL, "Reload directfbrc");
     }

     DFB_BOOT_GETTIME( DF_BOOT_DIRECTFBINIT, DF_MEASURE_END, DF_BOOT_LV1);

     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DFB_INIT);

     return DFB_OK;
}

DFBResult
DirectFBSetOption( const char *name, const char *value )
{
     DFBResult ret;

     if (dfb_config == NULL) {
          D_ERROR( "DirectFBSetOption: DirectFBInit has to be "
                   "called before DirectFBSetOption!\n" );
          return DFB_INIT;
     }

     if (!name)
          return DFB_INVARG;

     ret = dfb_config_set( name, value );
     if (ret)
          return ret;

     return DFB_OK;
}

/*
Afford one interface for the AP to get the miu cpu offset
*/

DFBResult
DirectFBGetMIUCPUOffset(MIUIdentifier identifier, unsigned long * puloffset)
{

     if (dfb_config == NULL)
     {
          D_ERROR( "DirectFBSetOption: DirectFBInit has to be "
                   "called before DirectFBSetOption!\n" );
          return DFB_INIT;
     }

    if((identifier >= MIUMAX)||(!puloffset))
    {
        return DFB_INVARG;
    }

    if(MIU0 == identifier)
    {
       *puloffset = dfb_config->mst_miu0_cpu_offset;
    }
    else if(MIU1 == identifier)
    {
       *puloffset = dfb_config->mst_miu1_cpu_offset;
    }
    else
    {
         return DFB_INVARG;
    }

    return DFB_OK;
}

/*
Afford one interface for the AP to get the miu hal offset
*/
DFBResult
DirectFBGetMIUHALOffset(MIUIdentifier identifier, unsigned long * puloffset)
{

     if (dfb_config == NULL)
     {
          D_ERROR( "DirectFBSetOption: DirectFBInit has to be "
                   "called before DirectFBSetOption!\n" );
          return DFB_INIT;
     }

    if((identifier >= MIUMAX)||(!puloffset))
    {
        return DFB_INVARG;
    }

    if(MIU0 == identifier)
    {
       *puloffset = dfb_config->mst_miu0_hal_offset;
    }
    else if(MIU1 == identifier)
    {
       *puloffset = dfb_config->mst_miu1_hal_offset;
    }
    else
    {
        return DFB_INVARG;
    }

    return DFB_OK;
}

DFBResult
DirectFBDisableBootLogo()
{
    return dfb_layer_Set_Disable_Boot_Logo();
}

DFBResult
DirectFBEnableZoommode(int layer_id, int x, int y ,int w, int h)
{
    return dfb_layer_Set_Enable_Zoom_Mode(layer_id, x, y, w, h);
}

u64
DirectFBHalAddrToBusAddr(u64 halPhysicalAddr)
{
    return _HalAddrToBusAddr(halPhysicalAddr);
}

/*
 * Programs have to call this to get the super interface
 * which is needed to access other functions
 */
DFBResult
DirectFBCreate( IDirectFB **interface_ptr )
{
    DFBResult  ret;

    DFB_BOOT_GETTIME( DF_BOOT_DIRECTFBCREATE, DF_MEASURE_START, DF_BOOT_LV1);

#if !DIRECTFB_BUILD_PURE_VOODOO

     IDirectFB *dfb;
     CoreDFB   *core_dfb;
#endif
     DIRECT_INTERFACE_DBG_DELTA_START();

     if (!dfb_config) {
          /*  don't use D_ERROR() here, it uses dfb_config  */
          direct_log_printf( NULL, "(!) DirectFBCreate: DirectFBInit "
                             "has to be called before DirectFBCreate!\n" );
          return DFB_INIT;
     }

     if (!interface_ptr)
          return DFB_INVARG;

     static DirectMutex lock = DIRECT_MUTEX_INITIALIZER(lock);

     direct_mutex_lock( &lock );

     if (!dfb_config->no_singleton && idirectfb_singleton) {
          idirectfb_singleton->AddRef( idirectfb_singleton );

          *interface_ptr = idirectfb_singleton;
          direct_mutex_unlock( &lock );
          return DFB_OK;
     }

direct_log_printf( NULL,"[ted] 1004 [%s %d] (pid = %d)\n", __FUNCTION__, __LINE__, getpid());
printf("[ted] 1004 [%s %d]\n", __FUNCTION__, __LINE__);

     DFB_BOOT_GETTIME( DF_BOOT_DIRECT_INIT, DF_MEASURE_START, DF_BOOT_LV2);
     direct_initialize();
     DFB_BOOT_GETTIME( DF_BOOT_DIRECT_INIT, DF_MEASURE_END, DF_BOOT_LV2);

     if ( !(direct_config->quiet & DMT_BANNER) && dfb_config->banner) {
          direct_log_printf( NULL,
                             "\n"
                             "   ~~~~~~~~~~~~~~~~~~~~~~~~~~| DirectFB " DIRECTFB_VERSION DIRECTFB_VERSION_VENDOR " |~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
                             "        (c) 2001-2010  The world wide DirectFB Open Source Community\n"
                             "        (c) 2000-2004  Convergence (integrated media) GmbH\n"
                             "      ----------------------------------------------------------------\n"
                             "\n" );
     }

#if !DIRECTFB_BUILD_PURE_VOODOO
     if (dfb_config->remote.host) {
          ret = CreateRemote( dfb_config->remote.host, dfb_config->remote.port, interface_ptr );
          direct_mutex_unlock( &lock );
          return ret;
     }


     DFB_BOOT_GETTIME( DF_BOOT_CORE_CREATE, DF_MEASURE_START, DF_BOOT_LV2);
     ret = dfb_core_create( &core_dfb );
     DFB_BOOT_GETTIME( DF_BOOT_CORE_CREATE, DF_MEASURE_END, DF_BOOT_LV2);
     if (ret) {
          direct_mutex_unlock( &lock );
          return ret;
     }

     DIRECT_ALLOCATE_INTERFACE( dfb, IDirectFB );

     DFB_BOOT_GETTIME( DF_BOOT_IDIRECTFB_CONSTRUCT, DF_MEASURE_START, DF_BOOT_LV2);
     ret = IDirectFB_Construct( dfb, core_dfb );
     DFB_BOOT_GETTIME( DF_BOOT_IDIRECTFB_CONSTRUCT, DF_MEASURE_END, DF_BOOT_LV2);

     if (ret) {
          dfb_core_destroy( core_dfb, false );
          direct_mutex_unlock( &lock );
          return ret;
     }

     if (dfb_core_is_master( core_dfb )) {
          if (!dfb_core_active( core_dfb )) {
               DFB_BOOT_GETTIME( DF_BOOT_INITLAYERS, DF_MEASURE_START, DF_BOOT_LV2);
               ret = IDirectFB_InitLayers( dfb );
               DFB_BOOT_GETTIME( DF_BOOT_INITLAYERS, DF_MEASURE_END, DF_BOOT_LV2);
               if (ret) {
                    dfb->Release( dfb );
                    direct_mutex_unlock( &lock );
                    return ret;
               }

               /* not fatal */
               ret = dfb_wm_post_init( core_dfb );
               if (ret)
                    D_DERROR( ret, "DirectFBCreate: Post initialization of WM failed!\n" );

               dfb_core_activate( core_dfb );
          }
     }

     *interface_ptr = dfb;

     if (!dfb_config->no_singleton)
          idirectfb_singleton = dfb;

     direct_mutex_unlock( &lock );

     DFB_BOOT_GETTIME( DF_BOOT_DIRECTFBCREATE, DF_MEASURE_END, DF_BOOT_LV1);
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DFB_CREATE);

#if DFB_MEASURE_BOOT_TIME
     dfb_boot_printTimeInfo();
#endif

     return DFB_OK;
#else
     ret = CreateRemote( dfb_config->remote.host ? dfb_config->remote.host : "", dfb_config->remote.port, interface_ptr );
     DIRECT_INTERFACE_DBG_DELTA_END(DDDT_DFB_CREATE);
     return ret;
#endif
}

DFBResult
DirectFBError( const char *msg, DFBResult error )
{
     if (msg)
          direct_log_printf( NULL, "(#) DirectFBError [%s]: %s\n", msg,
                             DirectFBErrorString( error ) );
     else
          direct_log_printf( NULL, "(#) DirectFBError: %s\n",
                             DirectFBErrorString( error ) );

     return error;
}

const char *
DirectFBErrorString( DFBResult error )
{
     return DirectResultString( error );
}

DFBResult
DirectFBErrorFatal( const char *msg, DFBResult error )
{
     DirectFBError( msg, error );

     //if (idirectfb_singleton)
          //IDirectFB_Destruct( idirectfb_singleton );

     exit( error );
}

/**************************************************************************************************/

static DFBResult
CreateRemote( const char *host, int port, IDirectFB **ret_interface )
{
     DFBResult             ret;
     DirectInterfaceFuncs *funcs;
     void                 *interface_ptr;

     D_ASSERT( host != NULL );
     D_ASSERT( ret_interface != NULL );

     ret = DirectGetInterface( &funcs, "IDirectFB", "Requestor", NULL, NULL );
     if (ret)
          return ret;

     ret = funcs->Allocate( &interface_ptr );
     if (ret)
          return ret;

     ret = funcs->Construct( interface_ptr, host, port );
     if (ret)
          return ret;

     *ret_interface = interface_ptr;

     return DFB_OK;
}


DFBResult
DirectFBSetGOPDst(DFBDisplayLayerID         id, DFBDisplayLayerGOPDST gop_dst )
{
    return dfb_layer_Set_GopDst(id, gop_dst);
}

DFBDisplayLayerGOPDST
DirectFBGetGOPDst(DFBDisplayLayerID         id)
{
    return dfb_layer_Get_GopDst(id);
}


/**********************************************************************************************************************/

typedef struct {
     int video;
     int system;
     int presys;
     int from_fbm;
} MemoryUsage;

/**********************************************************************************************************************/



static MemoryUsage mem = { 0, 0 };

static bool show_shm;
static bool show_pools;
static bool show_allocs;

/**********************************************************************************************************************/

static inline int dd_buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, bool video )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

     fusion_vector_foreach (allocation, i, buffer->allocs) 
     {
          if(video) 
          {
               if((buffer->surface->type&SURFACE_TYPE_VIDEO) || 
                  DSCAPS_FROM_VIDEO(buffer->surface->config.caps))
                {
                    mem += allocation->size;
                }
          }
          else
          {
               mem += allocation->size;
          }
     }

     return mem;
}

static int
dd_buffer_sizes( CoreSurface *surface, bool video )
{
     int i, mem = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          mem += dd_buffer_size( surface, buffer, video );
     }

     return mem;
}


static bool
dd_surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult ret;
     int          i;
     int          refs;
     CoreSurface *surface = (CoreSurface*) object;
     MemoryUsage *mem     = ctx;
     int          vmem;
     int          smem;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

     vmem = dd_buffer_sizes( surface, true );
     smem = dd_buffer_sizes( surface, false );

     if(DSCAPS_FROM_VIDEO(surface->config.caps))
     {
         mem->from_fbm += vmem;
     }
     else
     {
         mem->video += vmem;     
     }
     
      mem->system += smem;

     return true;
}

static void dd_dump_surfaces()
{
     dfb_core_enum_surfaces( NULL, dd_surface_callback, &mem );
}

/**********************************************************************************************************************/

int DirectFBDump(int *video, int *system, int *fbm)
{
     DFBResult ret;
     long long millis;
     long int  seconds, minutes, hours, days;
     IDirectFB *dfb = NULL;

     MemoryUsage mem_ret = { 0, 0 };

     memset((void*)&mem, 0, sizeof(MemoryUsage));

     /* Initialize DirectFB. */    
     if(dfb == NULL)
     {     
         ret = DirectFBInit( 0, NULL );
         if (ret) {
              DirectFBError( "DirectFBInit", ret );
              return -1;
         }
    
         /* Create the super interface. */
         ret = DirectFBCreate( &dfb );
         if (ret) {
              DirectFBError( "DirectFBCreate", ret );
              return -3;
         }
     }
     
     dd_dump_surfaces();

     if(video != NULL)
     {
         *video = mem.video;
     }
     if(system != NULL)
     {
         *system = mem.system + mem.presys;
     }
     if(fbm != NULL)
     {
         *fbm = mem.from_fbm;
     }

     /* DirectFB deinitialization. */
     if (dfb)
          dfb->Release( dfb );
     
     
     return ret;
}
  

