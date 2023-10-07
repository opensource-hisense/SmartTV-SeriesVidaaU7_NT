////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2008 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (��MStar Confidential Information��) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <directfb.h>
#include <directfb_keynames.h>

#include "directfb.h"

#include <core/coretypes.h>

#include <core/input.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <core/input_driver.h>

#include <fcntl.h>
#include <signal.h>
#ifdef HAVE_ANDROID_OS
#include <linux/sem.h>
#else
#include <sys/sem.h>
#endif
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <misc/conf.h>
#if (!USE_MSTAR_MI && !USE_MTK_STI)
#include "MsTypes.h"
#include "drvIR.h"
#endif
#include <linux/input.h>
#define EV_CNT (EV_MAX+1)
#define KEY_CNT (KEY_MAX+1)
#define REL_CNT (REL_MAX+1)
#define ABS_CNT (ABS_MAX+1)
#define LED_CNT (LED_MAX+1)

#ifndef BITS_PER_LONG
#define BITS_PER_LONG        (sizeof(long) * 8)
#endif
#define NBITS(x)             ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)               ((x)%BITS_PER_LONG)
#define BIT(x)               (1UL<<OFF(x))
#define LONG(x)              ((x)/BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)
#define MAX_LINUX_INPUT_DEVICES 16

#define MSTARIR_KEYRELEASE_PATCH

#define DEBUG_IR(x) do{(x);}while(0)

DFB_INPUT_DRIVER( mstarir )
#define DEVICE "/dev/ir"

//static DirectFBKeySymbolNames(keynames);


union semun_dfb {
 int val;
 struct semid_ds *buf;
 unsigned short *array;
} arg;

static int
driver_get_available()
{




    if (dfb_config->mst_new_ir)
        return 1;

    /* Check if we are able to read from device */
    if (access( DEVICE, R_OK ))
        return 0;

    return 1;
}

static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     int ret = snprintf( info->name,
               DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "MSTARIR Driver" );
     if(ret < 0)
         D_ERROR("[DFB] %s, %d : snprintf failed",__FUNCTION__,__LINE__);

     ret = snprintf( info->vendor,
               DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semi" );
     if(ret < 0)
         D_ERROR("[DFB] %s, %d : snprintf failed",__FUNCTION__,__LINE__);

     info->version.major = 0;
     info->version.minor = 2;
}


typedef struct {
     CoreInputDevice  *device;
     DirectThread *thread;
     int           fd;
     int           repeat_time;
     int           new_ir_repeat_time; 
     int           new_ir_first_repeat_time; 
} MStarIrData;

#if (!USE_MSTAR_MI && !USE_MTK_STI)
static bool _CheckProcessExist(pid_t pid)
{
    int r;

    if (pid <= 0)
    {
        //printf("1 pid %d doesn't exist!\n", pid);
        return false;
    }
    else
    {
        r = kill(pid, 0);
        if (r == 0)
        {
            //printf("1. pid %d exist!\n", pid);
            return true;
        }
        else if (r == -1)
        {
            //printf("errno: %d\n", errno);
            //printf("strerror: %s\n", strerror(errno));

            if (errno == ESRCH)
            {
                //printf("2 pid %d doesn't exist!\n", pid);
                return false;
            }
        }
    }
    //printf("2. pid %d exist!\n", pid);
    return true;
}
#endif //#if !USE_MSTAR_MI

#ifdef IR_CUSTOMIZE
typedef enum
{
    _IR_SHOT_P = 0x01,   /// 2'b01: only pshot edge detect for counter
    _IR_SHOT_N = 0x02,   /// 2'b10: only nshot edge detect for counter
    _IR_SHOT_PN = 0x03,  /// 2'b11/2'b00: both pshot/nshot edge detect for counter
    _IR_SHOT_INVLD,      /// Invalid for value greater than 2'b11
} IR_Shot_Sel;
void ExeKeyCode()
{
    printf("[DFB] This is user keycode\n");
}
static BOOL (*UserKeyCode)(unsigned char* pu8Key,unsigned char* pu8Repeat)=0;
extern  void SetUserSoftwareDecode(bool (*func)(unsigned char* pu8Key,unsigned char* pu8Repeat));
void SetUserSoftwareDecode(bool (*func)(unsigned char* pu8Key,unsigned char* pu8Repeat))
{
    UserKeyCode = func;
}
extern DFBInputDeviceKeymapEntry*  Get_keymap_entry( CoreInputDevice *device, int code );
#endif

static void*
MstarIREventThread_NEW( DirectThread *thread, void *driver_data )
{
     MStarIrData *data = (MStarIrData*) driver_data;

     int  fdmax;
     fd_set   set;
     int  status,readlen;
     unsigned int  i;
     struct input_event levt[64];
     fdmax = data->fd ;

     while(1){

          DFBInputEvent devt = { .type = DIET_UNKNOWN };
          FD_ZERO( &set );
          FD_SET( data->fd, &set );
          struct timeval timeout={1,0};
          status = select( fdmax + 1, &set, NULL, NULL, &timeout );

          if (status < 0 && errno != EINTR)
          {
               printf(" Mstar IR EventThread  break line:%d errno=%d\n", __LINE__, errno);
               break;
          }

          if (status <= 0){
               continue;}

          direct_thread_testcancel( thread );

          readlen = read( data->fd, levt, sizeof(levt) );
          if (readlen < 0 && errno != EINTR)
          {
               printf(" Mstar IR  EventThread break line:%d errno=%d\n", __LINE__, errno);
               break;
          }

          direct_thread_testcancel( thread );
          if (readlen <= 0)
               continue;

         for (i=0; i<readlen / sizeof(levt[0]); i++) {
                 DBG_INPUT_MSG("[IR] levt[%d].type=%x ", i, levt[i].type);
                 switch(levt[i].type){
                     case EV_MSC:
                         if( ((levt[i].value >> 28)&0xf) == 0xf){
                             /* SKY format : bypass 8 bit cmd + 16 bit data 
                                scancode =( (0x0fUL << 28) |  (u8Addr<<16) |(u8AddrInv<<8)|u8Cmd;*/
                             DFBInputEvent evt;
                             u8 u8KeyCode;
                             u16 u16KeyData;
                             u8KeyCode = ( levt[i].value >> 16) & 0xFF;
                             u16KeyData = levt[i].value & 0xFFFF;
                             evt.clazz = DFEC_INPUT;
                             evt.flags = DIEF_KEYCODE|DIEF_MIN;
                             evt.key_id = DIKI_UNKNOWN;
                             evt.key_symbol = DIKS_NULL;
                             if(((levt[i].value >> 24)&0x01) == 0x01)
                                 evt.type = DIET_KEYPRESS;
                             else
                                 evt.type = DIET_KEYRELEASE;
                             evt.key_code = u8KeyCode;
                             evt.min = u16KeyData;
                             DBG_INPUT_MSG("[DFB]  evt.key_code =%x  evt.min =%x \n",evt.key_code,evt.min);
                             dfb_input_dispatch( data->device, &evt);
                             continue;
                         }
                         else if( ((levt[i].value >> 30)&0x1) == 0x1) {
                             /* LGE format : bypass 5 bit cmd + 24 bit data 
                                scancode =(0x1UL << 30)| (u8Addr<<24) |(u8AddrInv<<16)|(u8Cmd<<8) | u8CmdInv;*/
                             DFBInputEvent evt;
                             int KeyCode;
                             int KeyData;
                             KeyCode = ( levt[i].value >> 24) & 0x1F;
                             KeyData = levt[i].value & 0xFFFFFF;
                             evt.clazz = DFEC_INPUT;
                             evt.flags = DIEF_KEYCODE|DIEF_MIN;
                             evt.key_id = DIKI_UNKNOWN;
                             evt.key_symbol = DIKS_NULL;
                             if(((levt[i].value >> 29)&0x01) == 0x01)
                                 evt.type = DIET_KEYPRESS;
                             else
                                 evt.type = DIET_KEYRELEASE;
                             evt.key_code = KeyCode;
                             evt.min = KeyData;
                             DBG_INPUT_MSG("[DFB]  evt.key_code =%x  evt.min =%x \n",evt.key_code,evt.min);
                             dfb_input_dispatch( data->device, &evt);
                             continue;
                         }
                         break;
                     case EV_KEY:
                         devt.flags = DIEF_TIMESTAMP;
                         devt.timestamp = levt[i].time;
                         devt.key_code = levt[i].code;
                         devt.type = levt[i].value ? DIET_KEYPRESS : DIET_KEYRELEASE;
                         if (levt[i].value == 2)
                            devt.flags |= DIEF_REPEAT;
                         devt.flags |= DIEF_KEYCODE;
                         devt.key_id    = DIKI_UNKNOWN;
                         devt.key_symbol = DIKS_NULL;
        
                         //printf("[kmjdbg]%s key_code=0x%x key_symbol=0x%x\n",__FUNCTION__,devt.key_code,devt.key_symbol);
                         if(DFB_SUPPORT_AN == 0)
                             dfb_input_dispatch( data->device, &devt );
                         break;
                     default : continue;
                     
                 }
           }
       }

        if (status <= 0) {
             printf ("[%s %d] mstar IR thread died errno=%d pid=%d\n", __FUNCTION__, __LINE__, errno, getpid());
        }

     return NULL;
}


#if (!USE_MSTAR_MI && !USE_MTK_STI)

#ifdef MSTARIR_KEYRELEASE_PATCH
//    #define REPEAT_TIME 250      // replace with dfb_config->mst_ir_repeat_time

    static void*
    MstarIREventThread( DirectThread *thread, void *driver_data )
    {
         MStarIrData *data = (MStarIrData*) driver_data;
         /* Read keyboard data */
     long long start = 0;
     DFBInputEvent lastEvent = {0};

    //printf("##################### MstarIREventThread $$$$$$$$$$$$$$$$$$$$$$$$$$  \n");

     while (1)
     {
            struct timeval tv =
            {
                .tv_sec = 0,
                .tv_usec = 50*1000
            };

            if(direct_thread_is_canceled(thread))
            {
                printf("[DFB] thread: %s      %d   exit !\n",__FUNCTION__,__LINE__);
                direct_thread_testcancel( thread );
            }

            // process long press, and check when to dispatch release event.             
            if ( lastEvent.flags & (DIEF_REPEAT | DIEF_KEYCODE) )
            {
                bool repeat = (lastEvent.flags & DIEF_REPEAT);
                long long ir_time_diff = direct_clock_get_millis() - start;

                if( ( !repeat && ( ir_time_diff > data->repeat_time + dfb_config->mst_ir_first_time_out) ) ||
                     ( repeat && ( ir_time_diff> data->repeat_time) ) )
                {
                    D_INFO("[DFB] time out!!!,end=%lld, start = %lld, diff = %lld  \n",direct_clock_get_millis(),start, ir_time_diff);
                    lastEvent.type       = DIET_KEYRELEASE;
                    lastEvent.flags &= ~DIEF_REPEAT;
                    D_INFO("[DFB] ~!~ DIET_KEYRELEASE type = 0x%x, flags = 0x%x , code = 0x%x \n",lastEvent.type,lastEvent.flags,lastEvent.key_code);
                    dfb_input_dispatch( data->device, &lastEvent);
                    lastEvent.type = DIET_UNKNOWN;
                    lastEvent.flags = 0;                    
                }        
            }
            
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(data->fd, &fdset);
            signed int ret;

            ret = select(data->fd+1, &fdset, NULL, NULL, &tv);

            if (ret > 0 && FD_ISSET(data->fd, &fdset))
            {
                unsigned int u32Data;
                unsigned char u8FantasyClass;
                read(data->fd, &u32Data, sizeof(signed int));
                u8FantasyClass=((u32Data>>28)&0xF);


                DBG_INPUT_MSG("[DFB] %s (%d):read_data = 0x%x\n", __FUNCTION__, __LINE__, u32Data);
                switch(u8FantasyClass)
                {
                    case 0x0f:
                    case 0x8:
                    case 0x4:
                    case 0x1: //legacy IR keypad data
                        {
#ifndef IR_CUSTOMIZE

                            DFBInputEvent evt;
                            u8 u8KeyCode;


                            u8KeyCode = (u32Data >> 8) & 0xFF;

                            evt.clazz = DFEC_INPUT;
                            evt.flags      = DIEF_KEYCODE;
                            evt.type       = DIET_KEYPRESS;
                            evt.key_id    = DIKI_UNKNOWN;
                            evt.key_symbol = DIKS_NULL;
                            if(u8FantasyClass == 0x4)
                            {
                                evt.key_code = u8KeyCode|(0x1<<8);
                            }
                            else if(u8FantasyClass == 0x8)
                            {
                                evt.key_code = u8KeyCode|(0x1<<9);
                            }
                            else if(u8FantasyClass == 0x1)
                            {
                                evt.key_code = u8KeyCode;
                            }
                            else if(u8FantasyClass == 0xf)
                            {
                                evt.key_code = (u32Data >> 8) & 0x0000ffff;
                            }

                            if((u32Data & 0xFF) > 0x0)
                            {
                                evt.flags |= DIEF_REPEAT;
                            }
                            else if (!dfb_config->disable_quick_press)
                            {
                                if (lastEvent.flags != 0) // if the second press is not REPEAT. this is for quick press same ir key.
                                {
                                    DFBInputEvent secEvent;
                                    
                                    secEvent.clazz          = DFEC_INPUT;
                                    secEvent.flags          = DIEF_KEYCODE;
                                    secEvent.type           = DIET_KEYRELEASE;
                                    secEvent.key_id        = DIKI_UNKNOWN;
                                    secEvent.key_symbol = DIKS_NULL;
                                    secEvent.key_code    = lastEvent.key_code;
                                    D_INFO("[DFB] ~!~ before the second press, DIET_KEYRELEASE type = 0x%x, flags = 0x%x , code = 0x%x \n", secEvent.type,secEvent.flags,secEvent.key_code);
                                    dfb_input_dispatch( data->device, &secEvent);
                                }
                            }
                            
                            memcpy(&lastEvent,&evt,sizeof(evt));
                            D_INFO("[DFB] ~!~ DIET_KEYPRESS type = 0x%x, flags = 0x%x , code = 0x%x \n",lastEvent.type,lastEvent.flags,lastEvent.key_code);
                            dfb_input_dispatch( data->device, &evt);
                            start = direct_clock_get_millis();
/* 
                                                        evt.type       = DIET_KEYRELEASE;
                                                        evt.flags &= ~DIEF_REPEAT;
                                                        dfb_input_dispatch( data->device, &evt);
*/
#endif
#ifdef IR_CUSTOMIZE
                            DFBInputEvent evt;
                            EN_KEY u8KeyCode;

                            //u8KeyCode = (u32Data >> 8) & 0xFF;
                            evt.clazz = DFEC_INPUT;
                            evt.flags      = DIEF_KEYCODE;
                            evt.type       = DIET_KEYPRESS;
                            //evt.key_code = u8KeyCode;

                            U8 u8ShotMode=0;
                            u8ShotMode = (u32Data>>24)&0x0F;

                          //  printf("shot mode = %d\n",u8ShotMode);
                            if(u8ShotMode==0)//HW decode mode
                            {
                                printf("[DFB] This is HW decode mode \n");

                                u8KeyCode = (u32Data >> 8) & 0xFF;
                                if(u8FantasyClass == 0x4)
                                {
                                    evt.key_code = u8KeyCode|(0x1<<8);
                                }
                                else if(u8FantasyClass == 0x8)
                                {
                                    evt.key_code = u8KeyCode|(0x1<<9);
                                }
                                else if(u8FantasyClass == 0x1)
                                {
                                    evt.key_code = u8KeyCode;
                                }
                                D_INFO("========================================\n");
                                D_INFO("[DFB] ir key code is: %d\n",u8KeyCode);
                                D_INFO("========================================\n");
                                if((u32Data & 0xFF) > 0x0)
                                {
                                    evt.flags |= DIEF_REPEAT;
                                }
                                dfb_input_dispatch( data->device, &evt);

                            }
                            else if(u8ShotMode==_IR_SHOT_N)
                            {
                                printf("[DFB] This is sw decode mode \n");

                                unsigned char uRepeat=0;
                                unsigned char u8SWKeyCode=0;
                                if(UserKeyCode)
                                {
                                    printf("[DFB] UserKeyCode is invoked \n");

                                    UserKeyCode(&u8SWKeyCode,&uRepeat);
                                }
                                else
                                {
                                    printf("[DFB] UserKeyCode is not invoked \n");
                                    u8SWKeyCode = (u32Data >> 8) & 0xFF;
                                }
                                printf("[DFB] repeat is %d\n",uRepeat);
                                if(uRepeat==1)
                                {
                                    DFBInputDeviceKeymapEntry *entry=NULL;
                                    if(u8FantasyClass == 0x4)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<8);
                                    }
                                    else if(u8FantasyClass == 0x8)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<9);
                                    }
                                    else if(u8FantasyClass == 0x1)
                                    {
                                        evt.key_code = u8KeyCode;
                                    }
                                    D_INFO("========================================\n");
                                    D_INFO("[DFB] 1. evt.key_code is: %d\n",evt.key_code);
                                    D_INFO("========================================\n");
                                    entry = Get_keymap_entry( data->device, evt.key_code );
                                    if(entry)
                                    {
                                        printf("[DFB] Rept=%d\n",entry->nRept);
                                        if((u32Data & 0xFF) > 0x0)
                                        {
                                            evt.flags |= DIEF_REPEAT;
                                        }
                                        if(evt.flags&&entry->nRept==1)
                                        {
                                            dfb_input_dispatch( data->device, &evt);

                                        }
                                    }
                                    else
                                    {
                                        printf("[DFB] entry is null\n");
                                    }
                                }
                                else
                                {
                                    if(u8FantasyClass == 0x4)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<8);
                                    }
                                    else if(u8FantasyClass == 0x8)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<9);
                                    }
                                    else if(u8FantasyClass == 0x1)
                                    {
                                        evt.key_code = u8KeyCode;
                                    }
                                    D_INFO("========================================\n");
                                    D_INFO("[DFB] 2. evt.key_code is: %d\n",evt.key_code);
                                    D_INFO("========================================\n");
                                    dfb_input_dispatch( data->device, &evt);

                                }
                            }
#endif
                        }
                        break;
                    case    0x2: //relative X,Y ( 0~10 bit for amount, 11 bit for direction)
                        break;
                    case    0x3: //absolute X,Y ( 0~11 bit for amount)
                        break;
                    case   0x6: //legacy IR keypad data for  another header coder(HEAD CODE  TCL)
                  {
#ifndef IR_CUSTOMIZE
                      DFBInputEvent evt;


                      evt.key_code  = (u32Data >> 8) & 0xFFFF;

                      evt.clazz = DFEC_INPUT;
                      evt.flags     = DIEF_KEYCODE;
                      evt.type      = DIET_KEYPRESS;
                      evt.key_id    = DIKI_UNKNOWN;
                      evt.key_symbol = DIKS_NULL;
        

                      //printf("06_Fir_dfb.key_code is %d\n",evt.key_code);

                      if((u32Data & 0xFF) > 0x0)
                      {
                          evt.flags |= DIEF_REPEAT;
                      }

                      memcpy(&lastEvent,&evt,sizeof(evt));
                      dfb_input_dispatch( data->device, &evt);
                      start = direct_clock_get_millis();


#endif
                  }
                    break;

                    default:
                        break;
                }

            }

        }
         return NULL;
    }


    #else
    static void*
    MstarIREventThread( DirectThread *thread, void *driver_data )
    {
         MStarIrData *data = (MStarIrData*) driver_data;
         /* Read keyboard data */
    
         while (1)
         {
            struct timeval tv =
            {
                .tv_sec = 0,
                .tv_usec = 100*1000
            };

            if(direct_thread_is_canceled(thread))
            {
                printf("[DFB] thread: %s      %d   exit !\n",__FUNCTION__,__LINE__);
                direct_thread_testcancel( thread );            
            }

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(data->fd, &fdset);
            signed int ret;

            ret = select(data->fd+1, &fdset, NULL, NULL, &tv);

            if (ret > 0 && FD_ISSET(data->fd, &fdset))
            {
                unsigned int u32Data;
                unsigned char u8FantasyClass;
                read(data->fd, &u32Data, sizeof(signed int));
                u8FantasyClass=((u32Data>>28)&0xF);
                unsigned int nRept;

                switch(u8FantasyClass)
                {
                    case 0x0f:
                    case 0x8:
                    case 0x4:
                    case 0x1: //legacy IR keypad data
                        {
#ifndef IR_CUSTOMIZE
                            DFBInputEvent evt;
                            u8 u8KeyCode;
                            u16 u16KeyCode;

                            u8KeyCode = (u32Data >> 8) & 0xFF;

                            evt.clazz = DFEC_INPUT;
                            evt.flags      = DIEF_KEYCODE;
                            evt.type       = DIET_KEYPRESS;
                            evt.key_id    = DIKI_UNKNOWN;
                            evt.key_symbol = DIKS_NULL;
                            if(u8FantasyClass == 0x4)
                            {
                                evt.key_code = u8KeyCode|(0x1<<8);
                            }
                            else if(u8FantasyClass == 0x8)
                            {
                                evt.key_code = u8KeyCode|(0x1<<9);
                            }
                            else if(u8FantasyClass == 0x1)
                            {
                                evt.key_code = u8KeyCode;
                            }
                            else if(u8FantasyClass == 0xf)
                            {
                                evt.key_code = (u32Data >> 8) & 0x0000ffff;
                            }

                            if((u32Data & 0xFF) > 0x0)
                            {
                                evt.flags |= DIEF_REPEAT;
                            }

                            dfb_input_dispatch( data->device, &evt);

                             evt.type       = DIET_KEYRELEASE;
                             evt.flags &= ~DIEF_REPEAT;
                             dfb_input_dispatch( data->device, &evt);
#endif
#ifdef IR_CUSTOMIZE
                            DFBInputEvent evt;
                            EN_KEY u8KeyCode;
                            //u8KeyCode = (u32Data >> 8) & 0xFF;
                            evt.clazz = DFEC_INPUT;
                            evt.flags      = DIEF_KEYCODE;
                            evt.type       = DIET_KEYPRESS;
                            //evt.key_code = u8KeyCode;

                            U8 u8ShotMode=0;
                            u8ShotMode = (u32Data>>24)&0x0F;

                          //  printf("shot mode = %d\n",u8ShotMode);
                            if(u8ShotMode==0)//HW decode mode
                            {
                                printf("[DFB] This is HW decode mode \n");

                                u8KeyCode = (u32Data >> 8) & 0xFF;
                                if(u8FantasyClass == 0x4)
                                {
                                    evt.key_code = u8KeyCode|(0x1<<8);
                                }
                                else if(u8FantasyClass == 0x8)
                                {
                                    evt.key_code = u8KeyCode|(0x1<<9);
                                }
                                else if(u8FantasyClass == 0x1)
                                {
                                    evt.key_code = u8KeyCode;
                                }
                                printf("========================================\n");
                                printf("[DFB] ir key code is: %d\n",u8KeyCode);
                                printf("========================================\n");
                                if((u32Data & 0xFF) > 0x0)
                                {
                                    evt.flags |= DIEF_REPEAT;
                                }
                                dfb_input_dispatch( data->device, &evt);
                                evt.type       = DIET_KEYRELEASE;
                                dfb_input_dispatch( data->device, &evt);
                            }
                            else if(u8ShotMode==_IR_SHOT_N)
                            {
                                printf("This is sw decode mode \n");

                                unsigned char uRepeat=0;
                                unsigned char u8SWKeyCode=0;
                                if(UserKeyCode)
                                {
                                    printf("[DFB] UserKeyCode is invoked \n");

                                    UserKeyCode(&u8SWKeyCode,&uRepeat);
                                }
                                else
                                {
                                    printf("[DFB] UserKeyCode is not invoked \n");
                                    u8SWKeyCode = (u32Data >> 8) & 0xFF;
                                }
                                printf("[DFB] repeat is %d\n",uRepeat);
                                if(uRepeat==1)
                                {
                                    DFBInputDeviceKeymapEntry *entry=NULL;
                                    if(u8FantasyClass == 0x4)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<8);
                                    }
                                    else if(u8FantasyClass == 0x8)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<9);
                                    }
                                    else if(u8FantasyClass == 0x1)
                                    {
                                        evt.key_code = u8KeyCode;
                                    }
                                    printf("========================================\n");
                                    printf("[DFB] 1. evt.key_code is: %d\n",evt.key_code);
                                    printf("========================================\n");
                                    entry = Get_keymap_entry( data->device, evt.key_code );
                                    if(entry)
                                    {
                                        printf("Rept=%d\n",entry->nRept);
                                        if((u32Data & 0xFF) > 0x0)
                                        {
                                            evt.flags |= DIEF_REPEAT;
                                        }
                                        if(evt.flags&&entry->nRept==1)
                                        {
                                            dfb_input_dispatch( data->device, &evt);
                                            evt.type       = DIET_KEYRELEASE;
                                            dfb_input_dispatch( data->device, &evt);
                                        }
                                    }
                                    else
                                    {
                                        printf("[DFB] entry is null\n");
                                    }
                                }
                                else
                                {
                                    if(u8FantasyClass == 0x4)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<8);
                                    }
                                    else if(u8FantasyClass == 0x8)
                                    {
                                        evt.key_code = u8KeyCode|(0x1<<9);
                                    }
                                    else if(u8FantasyClass == 0x1)
                                    {
                                        evt.key_code = u8KeyCode;
                                    }
                                    printf("========================================\n");
                                    printf("[DFB] 2. evt.key_code is: %d\n",evt.key_code);
                                    printf("========================================\n");
                                    dfb_input_dispatch( data->device, &evt);
                                    evt.type       = DIET_KEYRELEASE;
                                    dfb_input_dispatch( data->device, &evt);
                                }
                            }
#endif
                        }
                        break;
                    case    0x2: //relative X,Y ( 0~10 bit for amount, 11 bit for direction)
                        break;
                    case    0x3: //absolute X,Y ( 0~11 bit for amount)
                        break;
                    case   0x6: //legacy IR keypad data for  another header coder(HEAD CODE  TCL)
                  {
#ifndef IR_CUSTOMIZE
                      DFBInputEvent evt;
                      u8 u8KeyCode;

                      evt.key_code  = (u32Data >> 8) & 0xFFFF;

                      evt.clazz = DFEC_INPUT;
                      evt.flags     = DIEF_KEYCODE;
                      evt.type      = DIET_KEYPRESS;
                      evt.key_id    = DIKI_UNKNOWN;
                      evt.key_symbol = DIKS_NULL;
                      //evt.key_code = u8KeyCode|0x100;
                      //evt.key_code = evt.key_code & 0x00FF;

                      printf("[DFB] 06_11Fir_dfb.key_code is 0x%x\n",evt.key_code);

                      if((u32Data & 0xFF) > 0x0)
                        {
                             evt.flags |= DIEF_REPEAT;
                        }

                      dfb_input_dispatch( data->device, &evt);

                      printf("[DFB] 06_11Fir_dfb.key_symbol is 0x%x\n",evt.key_symbol);

                      evt.type      = DIET_KEYRELEASE;
                      evt.flags &= ~DIEF_REPEAT;
                      dfb_input_dispatch( data->device, &evt);
#endif
                  }
                    break;
                    default:
                        break;
                }

            }

        }
         return NULL;
    }
#endif

#endif //#if !USE_MSTAR_MI

static void
get_device_info( int  fd, InputDeviceInfo *info)
{
     unsigned int  num_keys     = 0;
     unsigned int  num_ext_keys = 0;
     unsigned int  num_buttons  = 0;
     unsigned int  num_rels     = 0;
     unsigned int  num_abs      = 0;
     int ret = 0;

     unsigned long evbit[NBITS(EV_CNT)];
     unsigned long keybit[NBITS(KEY_CNT)];
     unsigned long relbit[NBITS(REL_CNT)];
     unsigned long absbit[NBITS(ABS_CNT)];

     /* get device name */
     ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), info->desc.name );

     /* set device vendor */
     ret = snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Linux" );
     if(ret < 0){
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         return DFB_FAILURE;
     }

     /* get event type bits */
     ioctl( fd, EVIOCGBIT(0, sizeof(evbit)), evbit );

     if (test_bit( EV_KEY, evbit )) {
          int i;

          /* get keyboard bits */
          ioctl( fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit );

      /**  count typical keyboard keys only */
          for (i=KEY_Q; i<=KEY_M; i++)
               if (test_bit( i, keybit ))
                    num_keys++;

          for (i=KEY_OK; i<KEY_CNT; i++)
               if (test_bit( i, keybit ))
                    num_ext_keys++;

/*
          for (i=BTN_MOUSE; i<BTN_JOYSTICK; i++)
               if (test_bit( i, keybit ))
                    num_buttons++;
*/
          for (i=BTN_MOUSE; i<BTN_GAMEPAD; i++)
               if (test_bit( i, keybit ))
                    num_buttons++;

          for (i=BTN_GAMEPAD; i<BTN_DIGI; i++)
               if (test_bit( i, keybit ))
                    num_buttons++;

     }

     if (test_bit( EV_REL, evbit )) {
          int i;

          /* get bits for relative axes */
          ioctl( fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit );

          for (i=0; i<REL_CNT; i++)
               if (test_bit( i, relbit ))
                    num_rels++;
     }

     if (test_bit( EV_ABS, evbit )) {
          int i;

          /* get bits for absolute axes */
          ioctl( fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit );

          for (i=0; i<ABS_PRESSURE; i++)
               if (test_bit( i, absbit ))
                    num_abs++;
     }
#if 0
     /* Touchpad? */
     /* FIXME: can we rely on BTN_TOUCH? xorg synaptics driver doesn't use it. */
     if (test_bit( EV_KEY, evbit ) &&
         test_bit( BTN_TOUCH, keybit ) &&
         test_bit( BTN_TOOL_FINGER, keybit) &&
         test_bit( EV_ABS, evbit ) &&
         test_bit( ABS_X, absbit ) &&
         test_bit( ABS_Y, absbit ) &&
         test_bit( ABS_PRESSURE, absbit ))
          *touchpad = true;
     else
          *touchpad = false;

#endif
     /* Mouse, Touchscreen or Smartpad ? */
     if ((test_bit( EV_KEY, evbit ) &&
          (test_bit( BTN_TOUCH, keybit ) || test_bit( BTN_TOOL_FINGER, keybit ))) ||
          ((num_rels >= 2 && num_buttons)  ||  (num_abs == 2 && (num_buttons == 1))))
          info->desc.type |= DIDTF_MOUSE;
     else if (num_abs && num_buttons) /* Or a Joystick? */
          info->desc.type |= DIDTF_JOYSTICK;

     /* A Keyboard, do we have at least some letters? */
     if (num_keys > 20) {
          info->desc.type |= DIDTF_KEYBOARD;
          info->desc.caps |= DICAPS_KEYS;

          info->desc.min_keycode = 0;
          info->desc.max_keycode = 127;
     }

     /* A Remote Control? */
     if (num_ext_keys) {
          info->desc.type |= DIDTF_REMOTE;
          info->desc.caps |= DICAPS_KEYS;
     }

     /* Buttons */
     if (num_buttons) {
          info->desc.caps       |= DICAPS_BUTTONS;
          info->desc.max_button  = DIBI_FIRST + num_buttons - 1;
     }

     /* Axes */
     if (num_rels || num_abs) {
          info->desc.caps       |= DICAPS_AXES;
          info->desc.max_axis    = DIAI_FIRST + MAX(num_rels, num_abs) - 1;
     }
     if ((info->desc.type&DIDTF_KEYBOARD)&&(info->desc.type&DIDTF_MOUSE)&&
              (info->desc.type&DIDTF_REMOTE)){
         char name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH];
         memset( name, 0, sizeof(name));
         /* get device name */
         ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), name );
         DBG_INPUT_MSG("[DFB] %s %s, Device name %s\n",__FILE__, __FUNCTION__, name);

         /* fusion : MStar Smart TV IR Receiver
            merak  : MTK PMU IR
            mixed  : MTK Smart TV IR Receiver
         */
         if ( strcmp(name, "MTK PMU IR")==0 || strcmp(name, "MStar Smart TV IR Receiver")==0 || strcmp(name, "MTK Smart TV IR Receiver")==0 )
         {
               info->desc.type = DIDTF_MSTARIR;
               info->prefered_id = DIDID_MSTARIR;
         }
      }
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

     D_INFO("[DFB] %s, %s, info->prefered_id = %u, info->desc.type = %x\n", __FILE__, __FUNCTION__, info->prefered_id, info->desc.type);
}

static DFBResult
driver_open_device( CoreInputDevice *device,
                    unsigned int    number,
                    InputDeviceInfo *info,
                    void            **driver_data )
{
   int ret = 0;
   if (dfb_config == NULL)
          return DFB_FAILURE;
#if (!USE_MSTAR_MI && !USE_MTK_STI)
   if( !dfb_config->mst_new_ir){
     MStarIrData *data = NULL;
     int fd = -1;
     data = D_CALLOC(1, sizeof(MStarIrData));
     if (data == NULL)
         return DFB_FAILURE;
     data->device = device;
     /* set private data pointer */
     *driver_data = data;

     DFB_UTOPIA_TRACE(fd = open(DEVICE, O_RDWR));

     if (fd >= 0)
     {
          pid_t masterIrPid = getpid();
          int flag = 1;
          ioctl(fd, MDRV_IR_GET_MASTER_PID, &masterIrPid);

            // if master ir doesn't exist, set this as the mater ir
            if (_CheckProcessExist(masterIrPid) == false)
            {
                pid_t masterIrPid = getpid();
                // init IR
                DFB_UTOPIA_TRACE(ioctl(fd, MDRV_IR_INIT));
                DFB_UTOPIA_TRACE(ioctl(fd, MDRV_IR_SET_MASTER_PID, &masterIrPid));
            }
            DFB_UTOPIA_TRACE(ioctl(fd, MDRV_IR_ENABLE_IR, &flag));
            data->fd = fd;
    }
   else
   {
         DEBUG_IR(printf("[DFB] ERROR!!! Fail to open IR Kernal Module (%s, %s), errno=%d\n", __FILE__, __FUNCTION__, errno));
         D_FREE(data);
         return DFB_FAILURE;
   }
     data->repeat_time= dfb_config->mst_ir_repeat_time;

     if (info == NULL){
          D_FREE(data);
          return DFB_FAILURE;
     }
     /* fill driver info structure */
     ret = snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "MSTARIR Device" );
     if(ret < 0){
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         D_FREE(data);
         return DFB_FAILURE;
     }

     ret = snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "MStar Semi" );
     if(ret < 0){
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         D_FREE(data);
         return DFB_FAILURE;
     }

     info->prefered_id = DIDID_MSTARIR;
     info->desc.type   = DIDTF_REMOTE;
     info->desc.caps   = DICAPS_KEYS;
     info->desc.min_keycode = 0;
     info->desc.max_keycode = dfb_config->mst_ir_max_keycode;  //0x7FFF;

     /* start input thread */

     data->thread = direct_thread_create( DTT_INPUT, MstarIREventThread, data, "MStar IR" );
   }
   else
#endif
   {
    int i;
    int repeat_time[2];
    int ret = 0;
    InputDeviceInfo device_info = {0};
    MStarIrData *data = NULL;
    data = D_CALLOC(1, sizeof(MStarIrData));
    if (data == NULL)
        return DFB_FAILURE;

    for (i=0; i<MAX_LINUX_INPUT_DEVICES; i++)
    {
        int fd = -1;
        char dev[32];
        ret = snprintf( dev, sizeof(dev), "/dev/input/event%d", i );
        if(ret < 0){
            D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
            D_FREE(data);
            return DFB_FAILURE;
        }
        fd = open( dev, O_RDWR );

        if (fd >= 0) {
           memset( &device_info, 0, sizeof(InputDeviceInfo) );
           get_device_info( fd, &device_info);
          if(device_info.desc.type & DIDTF_MSTARIR){
             data->fd = fd;
             break;
          }
          else
            close( fd );
        }
        else
            DEBUG_IR(printf("[DFB] ERROR!!! Fail to open New IR Device (%s, %s), errno=%d\n", __FILE__, __FUNCTION__, errno));
     }

     if((device_info.desc.type & DIDTF_MSTARIR) == 0){
           D_FREE(data);
           return DFB_FAILURE;
     }

      data->device = device;
      /* set private data pointer */
      *driver_data = data;

      if (info == NULL){
          D_FREE(data);
          return DFB_FAILURE;
      }
       /* fill driver info structure */
      ret = snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "MSTARIR Device" );
     if(ret < 0){
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         D_FREE(data);
         return DFB_FAILURE;
     }
      ret = snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "MStar Semi" );
      if(ret < 0){
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         D_FREE(data);
         return DFB_FAILURE;
     }

      info->prefered_id = DIDID_MSTARIR;
      info->desc.type   = DIDTF_MSTARIR;
      info->desc.caps   = DICAPS_KEYS;
      info->desc.min_keycode = 0;
      info->desc.max_keycode = dfb_config->mst_ir_max_keycode;  //0x7FFF;

      data->new_ir_repeat_time = dfb_config->mst_new_ir_repeat_time;
      data->new_ir_first_repeat_time = dfb_config->mst_new_ir_first_repeat_time;

	repeat_time[0] = dfb_config->mst_new_ir_first_repeat_time;
	repeat_time[1] = dfb_config->mst_new_ir_repeat_time;
	ret = ioctl( data->fd , EVIOCSREP, repeat_time);  
	if (-1 == ret)
		printf("set mstar new IR repeat time fail \n");

      data->thread = direct_thread_create( DTT_INPUT, MstarIREventThread_NEW, data, "MStar NEW IR" );

    }
     return DFB_OK;
}

/*
 * Fetch one entry from the device's keymap if supported.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
    return DFB_UNSUPPORTED;
}


static void
driver_close_device( void *driver_data )
{
     MStarIrData *data = (MStarIrData*) driver_data;

     direct_thread_cancel( data->thread );

     direct_thread_join( data->thread );

     direct_thread_destroy( data->thread );

     /* close socket */
     close( data->fd );


     /* free private data */
    D_FREE( data );
}


static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                      void                         *driver_data,
                     InputDeviceIoctlData *param)
{
     MStarIrData *data = (MStarIrData*) driver_data;
     int repeat_time[2];
     int ret = 0;

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     switch(param->request)
     {
       case DFB_DEV_IOC_SET_MSTARIR_REPEAT_TIME:
#if (!USE_MSTAR_MI && !USE_MTK_STI)
          if( !dfb_config->mst_new_ir){
               if(((int)param->param[0])<=0)
                  return DFB_INVARG;
               data->repeat_time= ((int)param->param[0]);
               printf("[DFB] Mstarir Set repeat time:%d (ms)\n", data->repeat_time);
               break;
           }
           else
#endif
           {

               if ((int)param->param[0]<=0)
                    data->new_ir_first_repeat_time = dfb_config->mst_new_ir_first_repeat_time;
               else
                    data->new_ir_first_repeat_time = ((int)param->param[0]);

               if ((int)param->param[1]<=0)
                    data->new_ir_repeat_time = dfb_config->mst_new_ir_repeat_time;
               else
                    data->new_ir_repeat_time = ((int)param->param[1]);

               repeat_time[0] = data->new_ir_first_repeat_time;
               repeat_time[1] = data->new_ir_repeat_time;
               ret = ioctl( data->fd , EVIOCSREP, repeat_time);
               if (-1 == ret)
                  printf("set mstar new IR repeat time fail \n");

               printf("[DFB] Mstar new ir Set first repeat time:%d (ms)\n", data->new_ir_first_repeat_time);
               printf("[DFB] Mstar new ir Set repeat time:%d (ms)\n", data->new_ir_repeat_time);
               break;
            }
       case DFB_DEV_IOC_GET_MSTARIR_REPEAT_TIME:
#if (!USE_MSTAR_MI && !USE_MTK_STI)
           if( !dfb_config->mst_new_ir){
                memset(param->param, 0, sizeof(param->param));
                param->param[0] = data->repeat_time;
                printf("[DFB] Mstarir Get repeat time:%d (ms)\n", data->repeat_time);
                break;
            }
            else
#endif
            {
                memset(param->param, 0, sizeof(param->param));
                ret = ioctl( data->fd , EVIOCGREP, repeat_time);
                if (-1 == ret)
                {
                    printf("Get mstar new IR repeat time fail \n");
                    break;
                }
                param->param[0] = repeat_time[0];
                param->param[1] = repeat_time[1];
                param->param[2] = dfb_config->mst_new_ir;
                D_INFO("[DFB] Mstar new ir Get first repeat time:%d (ms)\n", param->param[0]);
                D_INFO("[DFB] Mstar new ir Get repeat time:%d (ms)\n", param->param[1]);

                break;
            }
        
       default:
          printf("[DFB] WARNING!!! Invalid argument input.\n");
          return DFB_UNSUPPORTED;
     }

     return DFB_OK;
}
