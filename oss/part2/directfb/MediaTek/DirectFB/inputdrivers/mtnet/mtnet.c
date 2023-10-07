/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
 #define CC_DFB_INPUT_MTNET_SUPPORT
#ifdef CC_DFB_INPUT_MTNET_SUPPORT

#include <config.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <linux/version.h>

#include <dfb_types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#ifdef CC_ANDROID_TWO_WORLDS
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include <directfb.h>
#include <directfb_keyboard.h>

#include <core/coredefs.h>
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

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 
 
#include <core/input_driver.h>


#ifdef CC_SUPPORT_DYNAMIC_ENABLE_DFB_INPUTEVENT
#define MTKIO_SET_DFBDISCARDKEY       _IOW('M', 0x41, unsigned int)
#define MTKIO_SET_DFBDISCARDKEY_EXCEPTION    _IOW('M', 0x44, unsigned int)
#define MTKIO_UNSET_DFBDISCARDKEY_EXCEPTION  _IOW('M', 0x45, unsigned int)
#endif
#define MT53FB_GET_DFB_LAYER0_SIZE 2
#define FBIO_GET 0x4645


DFB_INPUT_DRIVER( netevent )

#define MAX_INPUT_DEV_INUSE 1  /* create number of thread */
#define MAX_netevent_DEVICES 1


#define TAGNAME  "DFBNetEvt-R1"
struct netevnet_packet
{
	char tag[16]; /* should be TAGNAME*/
	DFBInputEvent evt;
};

/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice         *device;
     DirectThread            *thread;
     int                      fd;
     int                      number;
     struct netevnet_packet	pkt;
} NetEventData;


static int sock=-1;

static struct sockaddr_in mtnet_addr;  /* Internet socket name */

static int driver_open=1;

#ifndef CC_MTNET_ACCEPT_ALL
#define CC_MTNET_ACCEPT_ALL  0
#endif

extern int dfb_fb;
/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
#ifdef CC_ANDROID_TWO_WORLDS

#include <directfb_ex.h>

struct mt53fb_get
{
    unsigned int get_type;
    unsigned int get_size;
    union {
    unsigned int *get_data;
	unsigned int compat_value;
	unsigned long long u8padding;
	};
};


#endif

static void*
netevent_EventThread( DirectThread *thread, void *driver_data )
{
     NetEventData    *data = (NetEventData*) driver_data;
	 
#ifndef CC_ANDROID_TWO_WORLDS
	int cnt;
	struct netevnet_packet *pkt=&data->pkt;
     DFBInputEvent devt={0};

     D_INFO("[NETEVT]thread-%d\n", data->number);
     
     if(data->number==0)
     {
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	       D_INFO("[NETEVT]network socket fail=%d\n", errno);
	    }
	    else
	    {
		mtnet_addr.sin_family = AF_INET;  /* Internet address */
		mtnet_addr.sin_port = htons(3579)/*0xdfb*/;  /* System will assign port #  */
#if CC_MTNET_ACCEPT_ALL
		mtnet_addr.sin_addr.s_addr = INADDR_ANY;  /* "Wildcard" */
#else
		mtnet_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  /* "loopback" 0x0100007f */
#endif
		if ( bind(sock, &mtnet_addr, sizeof(mtnet_addr) ) < 0 ) {
			D_INFO("[NETEVT]network socket bind socket fail=%d\n", errno);
			close(sock);
			sock=-1;
		}
   		if ( listen ( sock, 5 ) < 0 ) {
			D_INFO("[NETEVT]network socket listen socket fail=%d\n", errno);
			close(sock);
			sock=-1;
   		}
	    }	 
     }
     
     while (driver_open) { 
     	if(sock<0)
	{
		usleep(1000);
		continue;
	}

      if ( ( data->fd = accept ( sock, 0, 0 ) ) < 0 ) {
		D_INFO("[NETEVT]network socket accept socket fail=%d\n", errno);
		close(sock);
		sock=-1;
		continue;
      }
         do {      
            memset (pkt,0,sizeof(struct netevnet_packet));  
            if (( cnt = read (data->fd, pkt, sizeof(struct netevnet_packet))) < 0 ) {
	       D_INFO("[NETEVT]read socket fail=%d\n", errno);
               close(data->fd);
            }
            else if (cnt == 0) {
               D_INFO("[NETEVT]msg len=0,closing...\n");
               close (data->fd);
            }
            else {
		if(strncmp(pkt->tag,TAGNAME,16)==0)
		{		
			//D_INFO("[NETEVT]dispatch event\n");
			dfb_input_dispatch( data->device, &pkt->evt );
		}
		else	D_INFO("[NETEVT]got wrong event tag\n");

            } 
 
         }  while (cnt > 0); 
     }
     D_INFO("[EVT_TH]thread %d leaving\n",data->number); 
     return NULL;

#else
    int ret;
    int listenfd = -1, recvfd = -1, maxfd = -1;
    int len;
    bool running = true;
    static fd_set fdset;
    struct sockaddr_un srv_addr;
    struct sockaddr_un clt_addr;
    
    memset(&srv_addr, 0, sizeof(srv_addr));

    listenfd  = socket(PF_UNIX, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
	    D_INFO("[DFB DBG] cannot create communication socket \n");
	    return NULL;
    }

    srv_addr.sun_family = AF_UNIX;
   
    strcpy(srv_addr.sun_path, LINUX_TMP_PATH"/DFBInput");
    unlink(srv_addr.sun_path);

    ret = bind(listenfd, (struct sockaddr*)& srv_addr, sizeof(struct sockaddr_un));
    if(ret < 0)
    {
	    D_INFO("[DFB DBG] cannot bind communication socket \n");
	    close(listenfd );
	    return NULL;
    }

    ret = listen(listenfd, 1);
    if(ret < 0)
    {
	    D_INFO("[DFB DBG] cannot listen communication socket");
	    close(listenfd );
	    return NULL;
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
			{
			   if(FD_ISSET(listenfd, &fdset))
			   {
			       printf("[socket_key] recv new client \n");
			   }
			   
			   if(( recvfd >= 0 ) && ( FD_ISSET(recvfd, &fdset) ))
			   {
					int num;
					DFBSocketEvent inputdata;
					memset(&inputdata, 0, sizeof(DFBSocketEvent));
					if((num = read(recvfd, &inputdata,sizeof(DFBSocketEvent)) > 0))
					{
                         DFBInputEvent event;
						 if(inputdata.m_Type == 0)//KEY 
						 {
						     if(inputdata.u.Key.m_Type == 0) //PRESS
						     {
						         printf("[socket_key] key press \n");
						         event.type = DIET_KEYPRESS;

                     event.flags =  (DFBInputEventFlags)(DIEF_KEYSYMBOL | DIEF_KEYCODE);
                     event.key_symbol = inputdata.u.Key.m_KeySymbol;
                     event.key_code = inputdata.u.Key.m_KeyCode;
                     printf("[socket_key] key symbol[0x%x] \n", event.key_symbol);

                     dfb_input_dispatch( data->device, &event );
						     }
							 else if(inputdata.u.Key.m_Type == 1)//RELEASE
							 {
							     printf("[socket_key] key release \n");
							     event.type = DIET_KEYRELEASE;

                   event.flags =  (DFBInputEventFlags)(DIEF_KEYSYMBOL | DIEF_KEYCODE);
                   event.key_symbol = inputdata.u.Key.m_KeySymbol;
                   event.key_code = inputdata.u.Key.m_KeyCode;
                   printf("[socket_key] key symbol[0x%x] \n", event.key_symbol);

                   dfb_input_dispatch( data->device, &event );
							 }
               else if(inputdata.u.Key.m_Type == 2) {
                   printf("[socket_key] key press/release \n");
							     event.type = DIET_KEYPRESS;

                   event.flags =  (DFBInputEventFlags)(DIEF_KEYSYMBOL | DIEF_KEYCODE | DIEF_NOREPEAT);
                   event.key_symbol = inputdata.u.Key.m_KeySymbol;
                   event.key_code = inputdata.u.Key.m_KeyCode;
                   printf("[socket_key] key symbol[0x%x] \n", event.key_symbol);

                   dfb_input_dispatch( data->device, &event );

                   event.type = DIET_KEYRELEASE;

                   event.flags =  (DFBInputEventFlags)(DIEF_KEYSYMBOL | DIEF_KEYCODE | DIEF_NOREPEAT);
                   event.key_symbol = inputdata.u.Key.m_KeySymbol;
                   event.key_code = inputdata.u.Key.m_KeyCode;
                   printf("[socket_key] key symbol[0x%x] \n", event.key_symbol);

                   dfb_input_dispatch( data->device, &event );
               }//PRESS+RELEASE try to fix socket delay issue
						 }
						 else if(inputdata.m_Type == 1)//mouse rel motion
						 {
						     event.type = DIET_AXISMOTION;
						     if(inputdata.u.MouseRelMotion.m_XRef != 0)
						     {						         
								 if(inputdata.u.MouseRelMotion.m_YRef != 0)
								 {
								     event.flags = (DFBInputEventFlags) (DIEF_AXISREL | DIEF_FOLLOW);
								 }
								 else
								 {
								     event.flags = (DFBInputEventFlags) (DIEF_AXISREL);
								 }
								 
								 event.axis = DIAI_X;
								 event.axisrel = inputdata.u.MouseRelMotion.m_XRef;

								 dfb_input_dispatch( data->device, &event );
						     }

						     if(inputdata.u.MouseRelMotion.m_YRef != 0)
						     {
						         event.flags = (DFBInputEventFlags) (DIEF_AXISREL);								 
								 event.axis = DIAI_Y;
								 event.axisrel = inputdata.u.MouseRelMotion.m_YRef;

								 dfb_input_dispatch( data->device, &event );								 
						     }							 
						 }
						 else if(inputdata.m_Type == 2)//mouse abs motion
						 {
                             unsigned int get_data[2];
                             struct mt53fb_get get;
                             get.get_type = MT53FB_GET_DFB_LAYER0_SIZE;
                             get.get_size = sizeof(get_data);
                             get.get_data = get_data;
                             ioctl(dfb_fb, FBIO_GET, &get);

						     event.type = DIET_AXISMOTION;
							 event.flags = (DFBInputEventFlags) (DIEF_AXISABS | DIEF_FOLLOW);
							 event.axis = DIAI_X;
							 event.axisabs = inputdata.u.MouseABSMotion.m_XPos * get_data[0] / inputdata.u.MouseABSMotion.m_XMax;
						     dfb_input_dispatch( data->device, &event );

						     event.type = DIET_AXISMOTION;
							 event.flags = (DFBInputEventFlags) (DIEF_AXISABS);
							 event.axis = DIAI_Y;
							 event.axisabs = inputdata.u.MouseABSMotion.m_YPos * get_data[1] / inputdata.u.MouseABSMotion.m_YMax;
						     dfb_input_dispatch( data->device, &event );						 
						 }
						 else if(inputdata.m_Type == 3)//mouse button
						 {
						     if(inputdata.u.Button.m_Type == 0) //PRESS
						     {
						         printf("button press \n");
						         event.type = DIET_BUTTONPRESS;
						     }
							 else if(inputdata.u.Button.m_Type == 1)//RELEASE
							 {
							     printf("button release \n");
							     event.type = DIET_BUTTONRELEASE;
							 }
							 
						     event.flags = 	(DFBInputEventFlags)(DIEF_BUTTONS);	
							 event.button = (DFBInputDeviceButtonIdentifier) inputdata.u.Button.m_Status;

							 dfb_input_dispatch( data->device, &event );								 
						 }
						 else if(inputdata.m_Type == 4)//set key enable/disable
						 {
#ifdef CC_SUPPORT_DYNAMIC_ENABLE_DFB_INPUTEVENT
                             ioctl(dfb_fb, MTKIO_SET_DFBDISCARDKEY, &inputdata.u.KeyState.m_Discard);
#endif
						 }
						 else if(inputdata.m_Type == 5)//set discard key exception
                                                 {
#ifdef CC_SUPPORT_DYNAMIC_ENABLE_DFB_INPUTEVENT
                             ioctl(dfb_fb, MTKIO_SET_DFBDISCARDKEY_EXCEPTION, &inputdata.u.KeyException.m_KeySymbol);
#endif
                                                 }
						 else if(inputdata.m_Type == 6)//unset discard key exception
                                                 {
#ifdef CC_SUPPORT_DYNAMIC_ENABLE_DFB_INPUTEVENT
                             ioctl(dfb_fb, MTKIO_UNSET_DFBDISCARDKEY_EXCEPTION, &inputdata.u.KeyException.m_KeySymbol);
#endif
                                                 }
						 
						 close(recvfd);
						 
						 recvfd = -1;							 
					}						
			   }

			   if(FD_ISSET(listenfd, &fdset))
			   {
			        if(recvfd >= 0)
			        {
			            printf("[socket_key] close original client %d \n", recvfd);
			            close(recvfd);
			        }
					
					len = sizeof(clt_addr);
					recvfd = accept(listenfd, (struct sockaddr*)&clt_addr,(socklen_t *) &len);	
					printf("[socket_key] new recv client %d \n", recvfd);
			   }
		    }
		    break;
	    }
	  
    }

    close(listenfd);

    return NULL;

#endif
}

/*
 * Fill device information.
 * Queries the input device and tries to classify it.
 */
static void
get_device_info(InputDeviceInfo *info)
{
     #define  num_buttons  3
     #define  num_rels     3
     #define  num_abs      0

     /* get device name */
     snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "DFB NetEvent Device" );
     /* set device vendor */
     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Mediatek" );

     info->desc.type |= DIDTF_MOUSE|DIDTF_JOYSTICK|DIDTF_KEYBOARD|DIDTF_REMOTE;

     /* A Keyboard, do we have at least some letters? */
     info->desc.caps |= DICAPS_KEYS|DICAPS_BUTTONS|DICAPS_AXES;

     info->desc.min_keycode =-1;
     info->desc.max_keycode =-1;//KEY_OK+ D_ARRAY_SIZE( ext_keycodes ) -1;;

     /* Buttons */
     info->desc.max_button  = DIBI_FIRST + num_buttons - 1;

     /* Axes */
     info->desc.max_axis    = DIAI_FIRST + MAX(num_rels, num_abs) - 1;

     /* Decide which primary input device to be. */
     if (info->desc.type & DIDTF_KEYBOARD)
          info->prefered_id = DIDID_KEYBOARD;
     else if (info->desc.type & DIDTF_REMOTE)
          info->prefered_id = DIDID_REMOTE;
     else if (info->desc.type & DIDTF_JOYSTICK)
          info->prefered_id = DIDID_JOYSTICK;
     else if (info->desc.type & DIDTF_MOUSE)
          info->prefered_id = DIDID_MOUSE;
     else
          info->prefered_id = DIDID_ANY;
}

/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available()
{
     return MAX_INPUT_DEV_INUSE;
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
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "DFB NetEvent Driver" );
     snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "Mediatek" );
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
     int              fd, ret;
     NetEventData  *data;

     D_INFO("[NETEVT]open net evnet %d\n",number);
     /* open device */
     driver_open=1;

     /* fill device info structure */
     get_device_info(info);

     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(NetEventData) );

     data->fd     = -1;
     data->device = device;
     data->number=number;
     D_INFO("[NETEVT]create thread %d\n",number);
     /* start input thread */
     data->thread = direct_thread_create( DTT_INPUT, netevent_EventThread, data, "net event" );

     /* set private data pointer */
     *driver_data = data;

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
     NetEventData *data = (NetEventData*) driver_data;
     D_INFO("[NETEVT]closing net event dev\n");
     driver_open=0;
	if(data->fd>=0)
	{
     /* stop input thread */
     direct_thread_cancel( data->thread );
     direct_thread_join( data->thread );
     direct_thread_destroy( data->thread );

     /* release device */
     close( data->fd );
	}
     /* free private data */
     D_FREE( data );
}

#endif
