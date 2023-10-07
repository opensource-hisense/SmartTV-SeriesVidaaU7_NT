/*
   (c) Copyright 2001-2010  MStar semiconductor.
   All rights reserved.
   Written by Roger Tsai <roger.tsai@mstarsemi.com.tw>
*/

#include <config.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <dfb_types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef HAVE_ANDROID_OS
#include <linux/kd.h>
#else
#include <sys/kd.h>
#endif
#include <stdlib.h>

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

#include <core/input_driver.h>

#include "MsTypes.h"
#include "MsCommon.h"
#include "drvMMIO.h"
#include "drvSAR.h"

//#define TIME_DEBUG
#ifdef  TIME_DEBUG
#include "time.h"
#endif

#define ENABLE_TSB_TOUCH_SENSOR  0  // for Toshiba Touch sensor

#if ENABLE_TSB_TOUCH_SENSOR
#include "drvMBX.h"
#endif

DFB_INPUT_DRIVER( keypad_input )

#define NULL_KEYVALUE 0xFF
#define REPEAT_TIME 250

static MS_BOOL  keypad_enable = FALSE;         /* default: disable */
static unsigned int keypad_input_interval = 10000; /* default: 10 msec */
#define KEYCODE_CNT  DIKS_CUSTOM99 - DIKS_CUSTOM80 + 1

/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice         *device;
     DirectThread            *thread;

    //@FIXME: revise follow.
    int                      value;
    DFBInputDeviceLockState  locks;
    bool                     dispatch_enable; 
    int           repeat_time;
    //int                       sensitive_numerator;
    //int                       sensitive_denominator;
    //int                       rel_axis_x_motion_reminder;
    //int                       rel_axis_y_motion_reminder;
} keypadInputData;

typedef struct {
    const char *pKeyDefine;
    unsigned char u8UtopiaKeycode;
    DFBInputDeviceKeySymbol DfbKeycode;
} stKeycodeInfo;

//============================================
//  Sysinfo Keycode,  Utopia Keycode,  DFB Keycode
//============================================
stKeycodeInfo KeycodeMappingTable[KEYCODE_CNT] = {
    {"KEYPAD_POWER", 0x46, DIKS_POWER},
    {"KEYPAD_UP", 0xA1, DIKS_CUSTOM81},
    {"KEYPAD_DOWN", 0xA2, DIKS_CUSTOM82},
    {"KEYPAD_LEFT", 0xA3, DIKS_CUSTOM83},
    {"KEYPAD_RIGHT", 0xA4, DIKS_CUSTOM84},
    {"KEYPAD_MENU", 0xA5, DIKS_CUSTOM85},
    {"KEYPAD_SELECT", 0xA6, DIKS_CUSTOM86},
    {"KEYPAD_EXIT", 0xA7, DIKS_CUSTOM87},
    {"KEYPAD_SOURCE", 0xA8, DIKS_SOURCE},
    {"KEYPAD_CUSTOMER89", 0xA9, DIKS_CUSTOM89},
    {"KEYPAD_CUSTOMER90", 0xAA, DIKS_CUSTOM90},
    {"KEYPAD_CUSTOMER91", 0xAB, DIKS_CUSTOM91},
    {"KEYPAD_CUSTOMER92", 0xAC, DIKS_CUSTOM92},
    {"KEYPAD_CUSTOMER93", 0xAD, DIKS_CUSTOM93},
    {"KEYPAD_CUSTOMER94", 0xAE, DIKS_CUSTOM94},
    {"KEYPAD_CUSTOMER95", 0xAF, DIKS_CUSTOM95},
    {"KEYPAD_CUSTOMER96", 0xB0, DIKS_CUSTOM96},
    {"KEYPAD_CUSTOMER97", 0xB1, DIKS_CUSTOM97},
    {"KEYPAD_CUSTOMER98", 0xB2, DIKS_CUSTOM98},
    {"KEYPAD_CUSTOMER99", 0xB3, DIKS_CUSTOM99},
};

//Translates a keypad keycode into DirectFB key id.
static DFBInputDeviceKeyIdentifier keyboard_get_identifier( int code )
{
     return DIKI_UNKNOWN;
}

static DFBInputDeviceKeySymbol keypad_get_symbol_default( int code )
{
     return DIKS_NULL;
}

static DFBInputDeviceKeySymbol GetDFBKeycodeByUtopiaKeycode(int iInput)
{
    int i = 0;
    for(i = 0; i < KEYCODE_CNT; i++)
    {
        if (iInput == KeycodeMappingTable[i].u8UtopiaKeycode)
            return KeycodeMappingTable[i].DfbKeycode;
    }
    /* if the keycode doesn't match, return NULL. */
    return DIKS_NULL;
}




static DFBInputDeviceKeySymbol (*keypad_get_symbol)( int keycode )=keypad_get_symbol_default;

extern void driver_set_keypad_conf(DFBInputDeviceKeySymbol (*func)( int keycode), unsigned long usec);

void driver_set_keypad_conf(DFBInputDeviceKeySymbol (*func)( int keycode ), unsigned long usec)
{
    if( func == NULL)
    {
        keypad_get_symbol = keypad_get_symbol_default;
        keypad_enable = FALSE;
    }
    else
    {
        keypad_get_symbol = func;
        keypad_enable = TRUE;
    }

    if(dfb_config->tvos_mode)
        keypad_enable = FALSE;

    keypad_input_interval = usec;
}


/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void*
keypad_EventThread(DirectThread *thread, void *driver_data)
{
#ifdef TIME_DEBUG
    int iNow;
    int iTimestamp = -1;
    int iTCount = 0;
#endif

#if ENABLE_TSB_TOUCH_SENSOR
    MBX_Msg stMsg;
#endif

#if !defined(ARCH_X86) && !defined(ARCH_X86_64)
        if (dfb_config && dfb_config->mst_disable_dfbinfo == false && dfb_config->mst_call_setkeypadcfg_in_device == true)
        {
             if(!dfb_dodfbinfo_setkeypadcfg())
             {
                printf("Note: DFBCreate: DirectFBDoDFBInfo_SetKEYPADCfg fail!  If you do not use SN, Please Ignore it\n");
             }
        }
#endif

    long long start = 0;
    bool thefirst = true;

    keypadInputData *pKeypadInputData = (keypadInputData*)driver_data;

    SAR_KpdResult ret;
    MS_U8 u8Keycode, u8Repeat;

    DFBInputEvent devt;
    memset(&devt, 0, sizeof(DFBInputEvent));

    while(1)
    {
        ret = E_SAR_KPD_FAIL;
        u8Keycode = NULL_KEYVALUE;
        u8Repeat = 0;
        /* test cancel */
        direct_thread_testcancel(thread);
        
        if(devt.flags & (DIEF_REPEAT | DIEF_KEYCODE))
        {
            if(thefirst)
            {
                if(direct_clock_get_millis() - start > pKeypadInputData->repeat_time)
                {
                    thefirst = false;
                    goto sendrelease;
                }
            }
            else if(direct_clock_get_millis() - start > REPEAT_TIME)
            {
                goto sendrelease;
            }
        }else
        {
    sendrelease:
            if(!thefirst && devt.type != DIET_UNKNOWN)
            {

                D_INFO("~!~ DIET_KEYRELEASE type = 0x%x, flags = 0x%x , code = 0x%x \n",devt.type,devt.flags,devt.key_code);
                devt.type       = DIET_KEYRELEASE;
                devt.flags &= ~DIEF_REPEAT;
                dfb_input_dispatch(pKeypadInputData->device, &devt);
                devt.type = DIET_UNKNOWN;
                devt.flags = 0;
            }
        }

        /* sleep an interval time */
        usleep(keypad_input_interval);
    #if ENABLE_TSB_TOUCH_SENSOR

        if(MDrv_MBX_RecvMsg(E_MBX_CLASS_PM_NOWAIT, &stMsg, 0, MBX_CHECK_NORMAL_MSG) == E_MBX_SUCCESS)
        {
            //printf("MDrv_MBX_RecvMsg,Index=%x\n",stMsg.u8Index);
            printf("MDrv_MBX_RecvMsg,u8Parameters[0]=0x%x\n",stMsg.u8Parameters[0]);
            u8Keycode = stMsg.u8Parameters[0];
        }
        else
        {
            //printf(".");
            continue;
        }
    #else
        if(pKeypadInputData->dispatch_enable)
        {
            ret = MDrv_SAR_Kpd_GetKeyCode(&u8Keycode, &u8Repeat);
        }

    #ifdef TIME_DEBUG
        iNow = time(NULL);
        if(iTimestamp == iNow)
            iTCount++;
        else
        {
            printf("Timestamp[%d] : %d (per sec)\n", iTimestamp, iTCount);
            iTimestamp = iNow;
            iTCount = 0;
        }
    #endif

        /* check the get keycode successfully */
        if(ret != E_SAR_KPD_OK)
            continue;

        /* check the keypad is vaild? */
        if(u8Keycode == DIKS_NULL || u8Keycode == NULL_KEYVALUE)
            continue;

        /* check the keycode is for keypad using? */
        if((*keypad_get_symbol)(u8Keycode) == DIKS_NULL)
            continue;
    #endif

        /* fill event struct. */
        devt.type = DIET_KEYPRESS;
        devt.flags = DIEF_KEYCODE;
        devt.key_code = u8Keycode;
        thefirst = true;
        if(u8Repeat)
            devt.flags |= DIEF_REPEAT;

        /* input the evnt to dispatch function */
        if(pKeypadInputData->dispatch_enable )
           dfb_input_dispatch(pKeypadInputData->device, &devt);
        
        start = direct_clock_get_millis();
    }

    D_PERROR ("keypad keypad thread died\n");
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
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "keypad Keypad Driver" );
     snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semiconductor." );

     info->version.major = 0;
     info->version.minor = 69;
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
    keypadInputData  *data = NULL;

     /* set device name */
    snprintf(info->desc.name,
                DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "keypadKeypad");

    /* set device vendor */
    snprintf(info->desc.vendor,
                DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "MStar");

    /* claim to be the primary keyboard */
    info->prefered_id = DIDID_KEYPAD;

    /* classify as a keyboard able to produce key events */
    /* set type flags */
    info->desc.type   = DIDTF_KEYBOARD;

    /* set capabilities */
    info->desc.caps   = DICAPS_KEYS;

    /* FIXME: the keycode range must be re-defined*/
    info->desc.min_keycode = 0x0;
    info->desc.max_keycode = 0xff;

    /* allocate and fill private data */
    data = D_CALLOC( 1, sizeof(keypadInputData) );
    if(!data)
    {
        return D_OOM();
    }

    /* fill device input private data field. */
    data->device = device;
    data->repeat_time= dfb_config->mst_keypad_repeat_time;

    //unsigned int ss;
    //MDrv_MMIO_GetBASE((MS_U32 *)&sar_mapped_base, (MS_U32 *)&ss, MS_MODULE_PM);

    //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>MMIO PM : %x %x\n", sar_mapped_base, ss);
    //WRITE_WORD(SARCONFIG, 0x0220);


#if !defined(ARCH_X86) && !defined(ARCH_X86_64)
    if (dfb_config->mst_disable_dfbinfo == false && dfb_config->mst_call_setdfbrccfg_disable == true)
    {
    dfb_config->keypadCallback = GetDFBKeycodeByUtopiaKeycode;
    }
#endif

    driver_set_keypad_conf(dfb_config->keypadCallback,dfb_config->keyinternal);
    /* start input thread */
    if(keypad_enable == TRUE)
        data->thread = direct_thread_create( DTT_INPUT, keypad_EventThread, data, "keypad Input" );
    else
        data->thread = NULL;

    if(data->thread == NULL)
    {
        printf("\n*** Open Keypad Device not opened\n");
    }
    else
    {
        printf("\n*** Open Keypad Device: %s, Input Interval: %u \n", (keypad_enable)?"Enable":"Disable", keypad_input_interval);
    #if ENABLE_TSB_TOUCH_SENSOR
        if(MDrv_MBX_RegisterMSG(E_MBX_CLASS_PM_NOWAIT, 4) != E_MBX_SUCCESS)
        {
            printf("MDrv_MBX_RegisterMSG() failed\n");
        }
        else
        {
            printf("MDrv_MBX_RegisterMSG() success\n");
            MDrv_MBX_Enable(TRUE);
        }
    #endif

    }
      data->dispatch_enable = true;
     //data->sensitive_numerator = 1;
     //data->sensitive_denominator = 1;
     //data->rel_axis_x_motion_reminder = 0;
     //data->rel_axis_y_motion_reminder = 0;

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
     int code = entry->code;

     entry->locks &= ~DILS_CAPS;
     entry->locks &= !DILS_NUM;

     /* get the identifier for basic mapping */
     entry->identifier = keyboard_get_identifier( code );
     /* write base level symbol to entry */
     entry->symbols[DIKSI_BASE] = (*keypad_get_symbol)( code );

     /* write shifted base level symbol to entry */
     //entry->symbols[DIKSI_BASE_SHIFT] = keyboard_get_symbol( code );
     /* write alternative level symbol to entry */
     //entry->symbols[DIKSI_ALT] = keyboard_get_symbol( code );
     /* write shifted alternative level symbol to entry */
     //entry->symbols[DIKSI_ALT_SHIFT] = keyboard_get_symbol( code );
     return DFB_OK;
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
     keypadInputData *data = (keypadInputData*) driver_data;

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
     keypadInputData *data = (keypadInputData*) driver_data;

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     switch(param->request)
     {
      case DFB_DEV_IOC_SET_KEYPAD_ENABLE:
          if(((int)param->param[0])<0)
              return DFB_INVARG;
          data->dispatch_enable = ((int)param->param[0]);
          break;
       default:
          return DFB_UNSUPPORTED;
     }

     return DFB_OK;
}
