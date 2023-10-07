/*
   (c) Copyright 2001-2010  MStar semiconductor.
   All rights reserved.
   Written by Roger Tsai <roger.tsai@mstarsemi.com.tw>
*/

#include <config.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <dfb_types.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#include <directfb.h>
#include <directfb_keynames.h>
#include <directfb_keyboard.h>

#include <core/coretypes.h>
#include <core/input.h>
#include <core/system.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#include <core/input_driver.h>

#include "MsTypes.h"
#include "MsCommon.h"
#include "drvMMIO.h"


DFB_INPUT_DRIVER( networkir )

static MS_BOOL  networkir_enable = FALSE;         /* default: disable */
static MS_U32   networkir_input_interval = 10000; /* default: 10 msec */

static volatile unsigned char networkir_tv_state = E_DFB_IPRC_TV_STATE_BOOTING; /* default: TV_STATE_BOOTING */

/*
 * declaration of private data
 */
typedef struct {
    CoreInputDevice         *device;
    DirectThread            *thread;

    //@FIXME: revise follow.
    int                      value;
    DFBInputDeviceLockState  locks;
    //int                       sensitive_numerator;
    //int                       sensitive_denominator;
    //int                       rel_axis_x_motion_reminder;
    //int                       rel_axis_y_motion_reminder;
} NetworkIrData;


#define NULL_KEYVALUE 0xFF
#define IRKEY_DUMY  NULL_KEYVALUE

#define NETWORKIR_TV_STATE_FILE "/dev/shm/tv_state"
#define GET_STRING_MAX  8

//Translates a networkir keycode into DirectFB key id.


static int networkir_trans_keycode_default( int code )
{
    return NULL_KEYVALUE;
}

static int (*networkir_trans_keycode)( int keycode )=networkir_trans_keycode_default;
extern void driver_set_networkir_conf(int (*func)( int keycode ), unsigned long usec);

void driver_set_networkir_conf(int (*func)( int keycode ), unsigned long usec)
{
    if(func == NULL)
    {
        networkir_trans_keycode = networkir_trans_keycode_default;
        networkir_enable = FALSE;
    }
    else
    {
        networkir_trans_keycode = func;
        networkir_enable = TRUE;
    }

    networkir_input_interval = usec;
}

//#define SOCK_SERVER  "/tmp/socket_server"
#define SOCK_SERVER "/dev/shm/socket_server"
#define MSG_MAX  512

static int create_named_socket (const char *filename)
{
    struct sockaddr_un named;

    int sock;

    //printf("\033[1;33m %s %d : socket bind filename is %s\033[0m \n", __FUNCTION__, __LINE__,filename);

    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("\033[1;33m %s %d : socket fail \033[0m \n", __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }
    named.sun_family = AF_LOCAL;
    strncpy(named.sun_path, filename, sizeof(named.sun_path));
    //printf("\033[1;33m %s %d : socket bind filename is %s\033[0m \n", __FUNCTION__, __LINE__,named.sun_path);
    
    named.sun_path[sizeof(named.sun_path) - 1] = '\0';

    
    //printf("\033[1;33m %s %d : size is %d named socket is %d fail \033[0m \n", __FUNCTION__, __LINE__,size,sizeof(named));

    int ret = bind(sock, (struct sockaddr *)&named, sizeof(named));

    if(ret<0)
    {
        printf("\033[1;33m %s %d : bind fail error is %d \033[0m \n", __FUNCTION__, __LINE__,ret);
        strerror(errno);
        printf("strerror(errno)=%s\n", strerror(errno));
        printf("\033[1;33m %s %d : bind fail error is %d \033[0m \n", __FUNCTION__, __LINE__,ret);
        exit(EXIT_FAILURE);
    }
    return sock;
}

static int create_networkir_tv_state(unsigned char tv_state)
{
    FILE *pSetFile;

    pSetFile = fopen(NETWORKIR_TV_STATE_FILE, "wt");
    if(pSetFile == NULL)
    {
        printf("\033[0;36m %s %d: fopen failed. \033[0m \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    fprintf(pSetFile, "%x", tv_state);
    fflush(pSetFile);
    fclose(pSetFile);

    return TRUE;
}

static int get_networkir_tv_state(void)
{
    FILE *pGetFile;
    char pGetString[GET_STRING_MAX];
    unsigned int tv_state;

    pGetFile = fopen(NETWORKIR_TV_STATE_FILE, "r");
    if(pGetFile == NULL)
    {
        printf("\033[0;36m %s %d: fopen failed. \033[0m \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if( NULL == (fgets(pGetString, GET_STRING_MAX, pGetFile)))
    {
        printf("\033[0;36m %s %d: fgets failed. \033[0m \n", __FUNCTION__, __LINE__);
        fclose(pGetFile);
        return FALSE;
    }
    sscanf(pGetString, "%x", &tv_state);
    fclose(pGetFile);

    networkir_tv_state = tv_state;
    if(networkir_tv_state >= E_DFB_IPRC_TV_STATE_MAX)
    {
        printf("\033[0;36m %s %d: error networkir_tv_state. \033[0m \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    //printf("\033[1;32m *** networkir_tv_state = %x \033[0m \n", networkir_tv_state);

    return TRUE;
}

extern int driver_set_networkir_tv_state(unsigned char tv_state);
int driver_set_networkir_tv_state(unsigned char tv_state)
{
    FILE *pSetFile;

    pSetFile = fopen(NETWORKIR_TV_STATE_FILE, "wt+");
    if(pSetFile == NULL)
    {
        printf("\033[0;36m %s %d: fopen failed. \033[0m \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    fprintf(pSetFile, "%x\n", tv_state);
    fflush(pSetFile);
    fclose(pSetFile);

    return TRUE;
}

int main_server (DirectThread *thread, void *driver_data)
{
    int sock;
    char msg[MSG_MAX];
    struct sockaddr_un named;
    size_t size;
    int len_bytes;




    DFBInputEvent devt;
    NetworkIrData *pNetworkIrData = (NetworkIrData*)driver_data;

    unlink (SOCK_SERVER);

    create_networkir_tv_state(E_DFB_IPRC_TV_STATE_BOOTING); //set default to BOOTING state
    get_networkir_tv_state();

    sock = create_named_socket(SOCK_SERVER);

    memset(&devt, 0, sizeof(DFBInputEvent));

    while (1)
    {

        struct timeval tv =
        {
            .tv_sec = 0,
            .tv_usec = 100*1000
        };

        if(direct_thread_is_canceled(thread))
        {
            direct_thread_testcancel( thread );
        }
        /* sleep an interval time */
        usleep(networkir_input_interval);

        get_networkir_tv_state();

        if( E_DFB_IPRC_TV_STATE_ACTIVE_STANDBY == networkir_tv_state)
        {
            /* (sendto) response return value */
            // In MStar models, the return value shall not be returned because remote commands shall be ignored in Active Standby in MStar models
            continue;
        }

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        signed int ret;

        ret = select(sock + 1, &fdset, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(sock, &fdset))
        {
            //Get Network Key Code
            size = sizeof(named);
            len_bytes = recvfrom(sock, msg, MSG_MAX, 0,
                              (struct sockaddr *)&named, &size);
            if(len_bytes < 0)
            {
                printf("========= Get %d byte from Network IR ================\n",len_bytes);
            }
            else
            {
                if(strcmp(msg,"NetworkIRTest")==0)
                {
                    devt.key_symbol = DIKS_DIAL_NFLX;//DIKS_DIAL_NFLX
                    printf("********* Get Netflix start request fron DIAL ================\n");
                } 
                else if(strcmp(msg,"YoutobeIRTest")==0)
                {
                    devt.key_symbol = DIKS_DIAL_YTTV;
                    printf("********* Get Youtobe start request fron DIAL ================\n");
                }
                else if(strcmp(msg,"YoutobeIRTest_EXIT")==0)
                {
                    devt.key_symbol = 0xf062;//DIKS_EXIT
                    printf("********* Get Youtobe stop request fron DIAL ================\n");
                }
                else
                    printf("********* Get %d byte from Network IR, the data is %s ================\n",len_bytes,msg);                    
                    
            }
                

            #if 0
            if (len_bytes < 0)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_UNKNOW_FAILURE, sock, named, size);
                continue;
            }

            if( E_DFB_IPRC_TV_STATE_BOOTING == networkir_tv_state)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_FAIL_TV_BOOTING, sock, named, size);
                continue;
            }

            fprintf(stderr, "Server: got msg: %s\n", msg);

            p_str = strtok(msg, str_sep);
            if (p_str == NULL)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_UNKNOW_FAILURE, sock, named, size);
                continue;
            }

            p_str = strtok(NULL, str_sep);
            if (p_str == NULL)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_UNKNOW_FAILURE, sock, named, size);
                continue;
            }

            if( !IsHexString(p_str) )
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_FAIL_KEYCODE_FORMAT, sock, named, size);
                continue;
            }
            //printf("\033[1;33m %s %d : keycode_string = %s \033[0m \n", __FUNCTION__, __LINE__, p_str);
            sscanf(p_str,"%x",&keycode);
            //printf("\033[1;33m %s %d : keycode = 0x%X  \033[0m \n", __FUNCTION__, __LINE__, keycode);
            u8Keycode = (unsigned char)(keycode & 0x000000FF) ;
            //printf("\033[1;33m %s %d : u8Keycode = 0x%X  \033[0m \n", __FUNCTION__, __LINE__, u8Keycode);
            //Get Network Key Code

            /* check the keycode is vaild? */
            if(u8Keycode == NULL_KEYVALUE)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_FAIL_KEYCODE_NOT_EXIST, sock, named, size);
                continue;
            }

            u8Keycode = (*networkir_trans_keycode)(keycode);  //map network keycode to general ir keycode
            printf("\033[1;33m %s %d : u8Keycode = 0x%X  \033[0m \n", __FUNCTION__, __LINE__, u8Keycode);
            /* check the keycode is for keypad using? */
            if( u8Keycode == IRKEY_DUMY)
            {
                /* (sendto) response return value */
                response_ret_val(E_DFB_IPRC_RET_FAIL_KEYCODE_NOT_EXIST, sock, named, size);
                continue;
            }

            /* (sendto) response return value */
            response_ret_val(E_DFB_IPRC_RET_SUCCESSFUL, sock, named, size);

            #endif
            /* sleep an interval time */
            usleep(networkir_input_interval);   //delay for waiting sendto() finish

            /* fill event struct. */
            devt.type = DIET_KEYPRESS;
            devt.flags = DIEF_KEYSYMBOL|DIEF_KEYCODE|DIEF_KEYID;
            printf("\033[1;33m %s %d : devt.flags = 0x%X  0x%x \033[0m \n", __FUNCTION__, __LINE__, devt.flags,devt.key_symbol);

            /* input the evnt to dispatch function */
            dfb_input_dispatch(pNetworkIrData->device, &devt);

            devt.type       = DIET_KEYRELEASE;
            dfb_input_dispatch(pNetworkIrData->device, &devt);
        }
    }
}

/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void*
networkir_EventThread(DirectThread *thread, void *driver_data)
{

    main_server(thread, driver_data);
    printf("\033[1;33m %s %d : networkir thread died \033[0m \n", __FUNCTION__, __LINE__);
    return NULL;
}

/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available( void )
{
    return 1;
}

/*
 * Fill out general information about this driver.
 * Called once during initialization of DirectFB.
 */
static void
driver_get_info( InputDriverInfo *info )
{
    /* fill driver info structure */
    snprintf ( info->name,
          DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "Network Ir Driver" );
    snprintf ( info->vendor,
          DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semiconductor." );

    info->version.major = 0;
    info->version.minor = 1;
}

/*
 * Open the device, fill out information about it,
 * allocate and fill private data, start input thread.
 * Called during initialization, resuming or taking over mastership.
 */
static DFBResult
driver_open_device( CoreInputDevice  *device,
                  unsigned int      number,
                  InputDeviceInfo  *info,
                  void            **driver_data )
{

    NetworkIrData  *data;

    /* set device name */
    snprintf(info->desc.name,
              DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "networkir");

    /* set device vendor */
    snprintf(info->desc.vendor,
              DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "MStar");

    /* claim to be the primary keyboard */
    info->prefered_id = DIDID_MSTAR_NETWORKIR;

    /* classify as a keyboard able to produce key events */
    /* set type flags */
    info->desc.type   = DIDTF_REMOTE;

    /* set capabilities */
    info->desc.caps   = DICAPS_KEYS;

    /* FIXME: the keycode range must be re-defined*/
    info->desc.min_keycode = 0x0;
    info->desc.max_keycode = 0x2FF;     //MCT note: map->max_keycode

    /* allocate and fill private data */
    data = D_CALLOC( 1, sizeof(NetworkIrData) );
    if(!data)
    {
        return D_OOM();
    }
    /* fill device input private data field. */
    data->device = device;

    /* start input thread */
    data->thread = direct_thread_create( DTT_INPUT, networkir_EventThread, data, "Net IR Input" );

    /* set private data pointer */
    *driver_data = data;

    printf("\033[1;33m %s %d : Open Network IR Device Success \033[0m \n", __FUNCTION__, __LINE__);
    return DFB_OK;
}

/*
 * Fetch one entry from the kernel keymap.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                       void                      *driver_data,
                       DFBInputDeviceKeymapEntry *entry )
{
    return DFB_UNSUPPORTED;
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
    NetworkIrData *data = (NetworkIrData*) driver_data;

    /* stop input thread */
    if(data->thread != NULL)
    {
        direct_thread_cancel( data->thread );
        direct_thread_join( data->thread );
        direct_thread_destroy( data->thread );
    }
    //@FIXME: close driver here.

    /* free private data */
    D_FREE( data );
}

static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                    void                         *driver_data,
                    InputDeviceIoctlData *param)
{


    if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
     return DFB_INVARG;

    switch(param->request)
    {
    default:
     return DFB_UNSUPPORTED;
    }
    return DFB_OK;
}



