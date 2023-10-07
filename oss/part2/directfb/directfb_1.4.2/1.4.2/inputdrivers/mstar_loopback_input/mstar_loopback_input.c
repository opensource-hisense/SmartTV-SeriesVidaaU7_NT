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


#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
typedef unsigned long kernel_ulong_t;
#define BITS_PER_LONG    (sizeof(long)*8)
#endif

#include <linux/input.h>

#ifndef KEY_OK
/* Linux kernel 2.5.42+ defines additional keys in linux/input.h */
#include "input_fake.h"
#endif

#ifndef EV_CNT
#define EV_CNT (EV_MAX+1)
#define KEY_CNT (KEY_MAX+1)
#define REL_CNT (REL_MAX+1)
#define ABS_CNT (ABS_MAX+1)
#define LED_CNT (LED_MAX+1)
#endif

/* compat defines for older kernel like 2.4.x */
#ifndef EV_SYN
#define EV_SYN            0x00
#define SYN_REPORT              0
#define SYN_CONFIG              1
#define ABS_TOOL_WIDTH        0x1c
#define BTN_TOOL_DOUBLETAP    0x14d
#define BTN_TOOL_TRIPLETAP    0x14e
#endif

#ifndef EVIOCGLED
#define EVIOCGLED(len) _IOC(_IOC_READ, 'E', 0x19, len)
#endif

#ifndef EVIOCGRAB
#define EVIOCGRAB _IOW('E', 0x90, int)
#endif

#include <linux/keyboard.h>


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

#define DFB_INPUTDRIVER_HAS_AXIS_INFO

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

#ifdef LINUX_INPUT_USE_FBDEV
#include <fbdev/fbdev.h>
#endif

#include <core/input_driver.h>
#include <core/windows.h>
#include <core/windows_internal.h>


DFB_INPUT_DRIVER( loopback_input )

D_DEBUG_DOMAIN( Debug_LinuxInput, "Input/Linux", "Linux input driver" );

#ifndef BITS_PER_LONG
#define BITS_PER_LONG        (sizeof(long) * 8)
#endif
#define NBITS(x)             ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)               ((x)%BITS_PER_LONG)
#define BIT(x)               (1UL<<OFF(x))
#define LONG(x)              ((x)/BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

/* compat for 2.4.x kernel - just a compile fix */
#ifndef HAVE_INPUT_ABSINFO
struct input_absinfo {
        s32 value;
         s32 minimum;
        s32 maximum;
        s32 fuzz;
        s32 flat;
};
#endif



#define FIXUP_KEY_FIELDS     (DIEF_MODIFIERS | DIEF_LOCKS | \
                              DIEF_KEYCODE | DIEF_KEYID | DIEF_KEYSYMBOL)


#if 0
#define DBG_INFO printf
#else
#define DBG_INFO(msg ...)               do {} while(0)
#endif

/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice         *device;
     DirectThread            *thread;

     int                      fd;
     int                      quitpipe[2];

     bool                     has_leds;
     unsigned long            led_state[NBITS(LED_CNT)];
     DFBInputDeviceLockState  locks;

     int                      vt_fd;

     int                      dx;
     int                      dy;

     bool                     touchpad;
     int                       sensitive_numerator;
     int                       sensitive_denominator;
     int                       rel_axis_x_motion_reminder;
     int                       rel_axis_y_motion_reminder;
} LinuxInputData;


#define MAX_LINUX_INPUT_DEVICES 16



static DFBInputDeviceID  FakedeviceType = DIDTF_NONE;

static const
int basic_keycodes [] = {
     DIKI_UNKNOWN, DIKI_ESCAPE, DIKI_1, DIKI_2, DIKI_3, DIKI_4, DIKI_5,
     DIKI_6, DIKI_7, DIKI_8, DIKI_9, DIKI_0, DIKI_MINUS_SIGN,
     DIKI_EQUALS_SIGN, DIKI_BACKSPACE,

     DIKI_TAB, DIKI_Q, DIKI_W, DIKI_E, DIKI_R, DIKI_T, DIKI_Y, DIKI_U,
     DIKI_I, DIKI_O, DIKI_P, DIKI_BRACKET_LEFT, DIKI_BRACKET_RIGHT,
     DIKI_ENTER,

     DIKI_CONTROL_L, DIKI_A, DIKI_S, DIKI_D, DIKI_F, DIKI_G, DIKI_H, DIKI_J,
     DIKI_K, DIKI_L, DIKI_SEMICOLON, DIKI_QUOTE_RIGHT, DIKI_QUOTE_LEFT,

     DIKI_SHIFT_L, DIKI_BACKSLASH, DIKI_Z, DIKI_X, DIKI_C, DIKI_V, DIKI_B,
     DIKI_N, DIKI_M, DIKI_COMMA, DIKI_PERIOD, DIKI_SLASH, DIKI_SHIFT_R,
     DIKI_KP_MULT, DIKI_ALT_L, DIKI_SPACE, DIKI_CAPS_LOCK,

     DIKI_F1, DIKI_F2, DIKI_F3, DIKI_F4, DIKI_F5, DIKI_F6, DIKI_F7, DIKI_F8,
     DIKI_F9, DIKI_F10, DIKI_NUM_LOCK, DIKI_SCROLL_LOCK,

     DIKI_KP_7, DIKI_KP_8, DIKI_KP_9, DIKI_KP_MINUS,
     DIKI_KP_4, DIKI_KP_5, DIKI_KP_6, DIKI_KP_PLUS,
     DIKI_KP_1, DIKI_KP_2, DIKI_KP_3, DIKI_KP_0, DIKI_KP_DECIMAL,

     /*KEY_103RD,*/ DIKI_BACKSLASH,
     /*KEY_F13,*/ DFB_FUNCTION_KEY(13),
     /*KEY_102ND*/ DIKI_LESS_SIGN,

     DIKI_F11, DIKI_F12, DFB_FUNCTION_KEY(14), DFB_FUNCTION_KEY(15),
     DFB_FUNCTION_KEY(16), DFB_FUNCTION_KEY(17), DFB_FUNCTION_KEY(18),
     DFB_FUNCTION_KEY(19), DFB_FUNCTION_KEY(20),

     DIKI_KP_ENTER, DIKI_CONTROL_R, DIKI_KP_DIV, DIKI_PRINT, DIKS_ALTGR,

     /*KEY_LINEFEED*/ DIKI_UNKNOWN,

     DIKI_HOME, DIKI_UP, DIKI_PAGE_UP, DIKI_LEFT, DIKI_RIGHT,
     DIKI_END, DIKI_DOWN, DIKI_PAGE_DOWN, DIKI_INSERT, DIKI_DELETE,

     /*KEY_MACRO,*/ DIKI_UNKNOWN,

     DIKS_MUTE, DIKS_VOLUME_DOWN, DIKS_VOLUME_UP, DIKS_POWER, DIKI_KP_EQUAL,

     /*KEY_KPPLUSMINUS,*/ DIKI_UNKNOWN,

     DIKS_PAUSE, DFB_FUNCTION_KEY(21), DFB_FUNCTION_KEY(22),
     DFB_FUNCTION_KEY(23), DFB_FUNCTION_KEY(24),

     DIKI_KP_SEPARATOR, DIKI_META_L, DIKI_META_R, DIKI_SUPER_L,

     DIKS_STOP,

     /*DIKS_AGAIN, DIKS_PROPS, DIKS_UNDO, DIKS_FRONT, DIKS_COPY,
     DIKS_OPEN, DIKS_PASTE, DIKS_FIND, DIKS_CUT,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_HELP, DIKS_MENU, DIKS_CALCULATOR, DIKS_SETUP,

     /*KEY_SLEEP, KEY_WAKEUP, KEY_FILE, KEY_SENDFILE, KEY_DELETEFILE,
     KEY_XFER,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,
     DIKI_UNKNOWN,

     /*KEY_PROG1, KEY_PROG2,*/
     DIKS_CUSTOM1, DIKS_CUSTOM2,

     DIKS_INTERNET,

     /*KEY_MSDOS, KEY_COFFEE, KEY_DIRECTION, KEY_CYCLEWINDOWS,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_MAIL,

     /*KEY_BOOKMARKS, KEY_COMPUTER, */
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_BACK, DIKS_FORWARD,

     /*KEY_CLOSECD, KEY_EJECTCD, KEY_EJECTCLOSECD,*/
     DIKS_EJECT, DIKS_EJECT, DIKS_EJECT,

     DIKS_NEXT, DIKS_PLAYPAUSE, DIKS_PREVIOUS, DIKS_STOP, DIKS_RECORD,
     DIKS_REWIND, DIKS_PHONE,

     /*KEY_ISO,*/ DIKI_UNKNOWN,
     /*KEY_CONFIG,*/ DIKS_SETUP,
     /*KEY_HOMEPAGE, KEY_REFRESH,*/ DIKI_UNKNOWN, DIKS_SHUFFLE,

     DIKS_EXIT, /*KEY_MOVE,*/ DIKI_UNKNOWN, DIKS_EDITOR,

     /*KEY_SCROLLUP,*/ DIKS_PAGE_UP,
     /*KEY_SCROLLDOWN,*/ DIKS_PAGE_DOWN,
     /*KEY_KPLEFTPAREN,*/ DIKI_UNKNOWN,
     /*KEY_KPRIGHTPAREN,*/ DIKI_UNKNOWN,

     /* unused codes 181-182: */
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     DFB_FUNCTION_KEY(13), DFB_FUNCTION_KEY(14), DFB_FUNCTION_KEY(15),
     DFB_FUNCTION_KEY(16), DFB_FUNCTION_KEY(17), DFB_FUNCTION_KEY(18),
     DFB_FUNCTION_KEY(19), DFB_FUNCTION_KEY(20), DFB_FUNCTION_KEY(21),
     DFB_FUNCTION_KEY(22), DFB_FUNCTION_KEY(23), DFB_FUNCTION_KEY(24),

     /* unused codes 195-199: */
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     /* KEY_PLAYCD, KEY_PAUSECD */
     DIKS_PLAY, DIKS_PAUSE,

     /*KEY_PROG3, KEY_PROG4,*/
     DIKS_CUSTOM3, DIKS_CUSTOM4,

     DIKI_UNKNOWN,

     /*KEY_SUSPEND, KEY_CLOSE*/
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     /* KEY_PLAY */
     DIKS_PLAY,

     /* KEY_FASTFORWARD */
     DIKS_FASTFORWARD,

     /* KEY_BASSBOOST */
     DIKI_UNKNOWN,

     /* KEY_PRINT */
     DIKS_PRINT,

     /* KEY_HP             */  DIKI_UNKNOWN,
     /* KEY_CAMERA         */  DIKI_UNKNOWN,
     /* KEY_SOUND          */  DIKS_AUDIO,
     /* KEY_QUESTION       */  DIKS_HELP,
     /* KEY_EMAIL          */  DIKS_MAIL,
     /* KEY_CHAT           */  DIKI_UNKNOWN,
     /* KEY_SEARCH         */  DIKI_UNKNOWN,
     /* KEY_CONNECT        */  DIKI_UNKNOWN,
     /* KEY_FINANCE        */  DIKI_UNKNOWN,
     /* KEY_SPORT          */  DIKI_UNKNOWN,
     /* KEY_SHOP           */  DIKI_UNKNOWN,
     /* KEY_ALTERASE       */  DIKI_UNKNOWN,
     /* KEY_CANCEL         */  DIKS_CANCEL,
     /* KEY_BRIGHTNESSDOWN */  DIKI_UNKNOWN,
     /* KEY_BRIGHTNESSUP   */  DIKI_UNKNOWN,
     /* KEY_MEDIA          */  DIKI_UNKNOWN,
};

static const
int ext_keycodes [] = {
     DIKS_OK, DIKS_SELECT, DIKS_GOTO, DIKS_CLEAR, DIKS_POWER2, DIKS_OPTION,
     DIKS_INFO, DIKS_TIME, DIKS_VENDOR, DIKS_ARCHIVE, DIKS_PROGRAM,
     DIKS_CHANNEL, DIKS_FAVORITES, DIKS_EPG, DIKS_PVR, DIKS_MHP,
     DIKS_LANGUAGE, DIKS_TITLE, DIKS_SUBTITLE, DIKS_ANGLE, DIKS_ZOOM,
     DIKS_MODE, DIKS_KEYBOARD, DIKS_SCREEN, DIKS_PC, DIKS_TV, DIKS_TV2,
     DIKS_VCR, DIKS_VCR2, DIKS_SAT, DIKS_SAT2, DIKS_CD, DIKS_TAPE,
     DIKS_RADIO, DIKS_TUNER, DIKS_PLAYER, DIKS_TEXT, DIKS_DVD, DIKS_AUX,
     DIKS_MP3, DIKS_AUDIO, DIKS_VIDEO, DIKS_DIRECTORY, DIKS_LIST, DIKS_MEMO,
     DIKS_CALENDAR, DIKS_RED, DIKS_GREEN, DIKS_YELLOW, DIKS_BLUE,
     DIKS_CHANNEL_UP, DIKS_CHANNEL_DOWN, DIKS_FIRST, DIKS_LAST, DIKS_AB,
     DIKS_NEXT, DIKS_RESTART, DIKS_SLOW, DIKS_SHUFFLE, DIKS_FASTFORWARD,
     DIKS_PREVIOUS, DIKS_NEXT, DIKS_DIGITS, DIKS_TEEN, DIKS_TWEN, DIKS_BREAK
};

/*
 * Touchpads related stuff
 */
enum {
     TOUCHPAD_FSM_START,
     TOUCHPAD_FSM_MAIN,
     TOUCHPAD_FSM_DRAG_START,
     TOUCHPAD_FSM_DRAG_MAIN,
};
struct touchpad_axis {
     int old, min, max;
};
struct touchpad_fsm_state {
     int fsm_state;
     struct touchpad_axis x;
     struct touchpad_axis y;
     struct timeval timeout;
};


static void
flush_xy( LinuxInputData *data, bool last )
{
     DFBInputEvent evt = { .type = DIET_UNKNOWN };

     if (data->dx) {
          evt.type    = DIET_AXISMOTION;
          evt.flags   = DIEF_AXISREL;
          evt.axis    = DIAI_X;
          evt.axisrel = data->dx;

          /* Signal immediately following event. */
          if (!last || data->dy)
               evt.flags |= DIEF_FOLLOW;

          dfb_input_dispatch( data->device, &evt );

          data->dx = 0;
     }

     if (data->dy) {
          evt.type    = DIET_AXISMOTION;
          evt.flags   = DIEF_AXISREL;
          evt.axis    = DIAI_Y;
          evt.axisrel = data->dy;

          /* Signal immediately following event. */
          if (!last)
               evt.flags |= DIEF_FOLLOW;

          dfb_input_dispatch( data->device, &evt );

          data->dy = 0;
     }
}


/*
 * Fill device information.
 * Queries the input device and tries to classify it.
 */
static void
get_device_info( InputDeviceInfo *info)
{



     unsigned int  num_rels     = 2;
     unsigned int  num_abs      = 2;






     /* get device name */
     //ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), info->desc.name );
     snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "mstarloopback" );

     /* set device vendor */
     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Linux" );


      info->desc.type |= DIDTF_MOUSE;
      info->desc.type |= DIDTF_JOYSTICK;
      info->desc.type |= DIDTF_KEYBOARD;
      info->desc.caps |= DICAPS_KEYS;
      info->desc.min_keycode = 0;
      info->desc.max_keycode = 0xFFFF;
      info->desc.type |= DIDTF_REMOTE;
      info->desc.caps |= DICAPS_KEYS;
      info->desc.caps       |= DICAPS_BUTTONS;
      info->desc.max_button  = DIBI_FIRST + /*num_buttons*/ 3 - 1;
      info->desc.caps       |= DICAPS_AXES;
      info->desc.max_axis    = DIAI_FIRST + MAX(num_rels, num_abs) - 1;
      info->prefered_id = DIDID_VIRTUAL;
}


/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available( void )
{



#ifdef LINUX_INPUT_USE_FBDEV
     if (dfb_system_type() != CORE_FBDEV)
          return 0;
#endif

     return 1;//only one loopback device
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
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "MSTAR LOOPBACK Device" );
     snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semi" );

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



     LinuxInputData  *data;

     /* fill device info structure */
     get_device_info( info );

     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(LinuxInputData) );
     if (!data) {
          return D_OOM();
     }
      data->fd     = 0;
     data->device = device;
     data->touchpad = false;
     data->vt_fd    = -1;
     data->sensitive_numerator = 1;
     data->sensitive_denominator = 1;
     data->rel_axis_x_motion_reminder = 0;
     data->rel_axis_y_motion_reminder = 0;

     /* set private data pointer */
     *driver_data = data;

     return DFB_OK;

/*
driver_open_device_error:
     D_FREE( data );

     return DFB_INIT;
*/
}

/*
 * Obtain information about an axis (only absolute axis so far).
 */
static DFBResult
driver_get_axis_info( CoreInputDevice              *device,
                      void                         *driver_data,
                      DFBInputDeviceAxisIdentifier  axis,
                      DFBInputDeviceAxisInfo       *ret_info )
{


     //if (data->touchpad)
          return DFB_OK;

}

static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                      void                         *driver_data,
                     InputDeviceIoctlData *param)
{
     LinuxInputData *data = (LinuxInputData*) driver_data;

     DBG_INPUT_MSG("[directfb:input]mstarloopback enter driver_device_ioctl\n" );

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     DBG_INPUT_MSG("[directfb:input]mstarloopback to check request tag...\n" );

     switch(param->request)
     {
           case DFB_DEV_IOC_SEND_LOOPBACK_EVENT:
            {
                  DFBInputEvent devt = { .type = DIET_UNKNOWN };
                  memcpy(&devt, param->param, sizeof(DFBInputEvent));
                  devt.device_id= FakedeviceType;
                  DBG_INPUT_MSG("[directfb:input]mstarloopback  got event: devt.type:%d, devt.key_symbol:0x%x, devt.key_id:%d, devt.key_code:%d\n", devt.type, devt.key_symbol, devt.key_id, devt.key_code);
                  //post event to event queue
                  if (devt.type != DIET_UNKNOWN) {
                        DBG_INPUT_MSG("[directfb:input]mstarloopback Post event to directfb Event Queue\n" );
                        flush_xy( data, false );

                        if(devt.flags == 0)
                        {
                            printf("warning!!! You don't set flags. Set to DIEF_KEYCODE|DIEF_KEYID|DIEF_KEYSYMBOL as default.\n");
                            devt.flags = DIEF_KEYCODE|DIEF_KEYID|DIEF_KEYSYMBOL;
                        }
                    /* Signal immediately following event. */
                        dfb_input_dispatch( data->device, &devt );
                  }

           }
          break;

          case DFB_DEV_IOC_SET_FAKE_DEVICE_ID:
           {
                  FakedeviceType = param->param[0];
           }
          break;
       default:
          DBG_INPUT_MSG("[directfb:input]mstarloopback UNSUPPORTED this request\n" );
          return DFB_UNSUPPORTED;
     }

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
     LinuxInputData *data = (LinuxInputData*) driver_data;

     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

     /* free private data */
     D_FREE( data );

     D_DEBUG_AT( Debug_LinuxInput, "%s() closed\n", __FUNCTION__ );
}

