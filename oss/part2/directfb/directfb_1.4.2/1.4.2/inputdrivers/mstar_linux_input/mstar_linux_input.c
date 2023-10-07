/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjala <syrjala@sci.fi> and
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
#define _GNU_SOURCE
#include <config.h>


#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
typedef unsigned long kernel_ulong_t;
#define BITS_PER_LONG    (sizeof(long)*8)
#endif

#include <linux/input.h>
#include <poll.h>

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

#define DEV_LOGIC_MOUSE "/dev/input/logic_mouse_xxxxxx"
#define DEV_LOGIC_KEYBOARD "/dev/input/logic_keyboard_xxxxxx"
#define DEV_LOGIC_JOYSTICK "/dev/input/logic_joystick_xxxxxx"
#define DEV_LOGIC_USBIR "/dev/input/logic_usbir_xxxxxx"

#define DEVICE_NAME_MAX_LENGTH  50


#if 0
#define DBG_INFO printf
#else
#define DBG_INFO(msg ...)               do {} while(0)
#endif

#if 0
#define THREAD_DBG_INFO printf
#else
#define THREAD_DBG_INFO(msg ...)               do {} while(0)
#endif


#if 0
#define DBG_FORCE_PRINT printf
#else
#define DBG_FORCE_PRINT(msg ...)               do {} while(0)
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

#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>

#ifdef LINUX_INPUT_USE_FBDEV
#include <fbdev/fbdev.h>
#endif

/* Exclude hot-plug stub functionality from this input provider.
   This define MUST before include input_driver.h.                    */
#define DISABLE_INPUT_HOTPLUG_FUNCTION_STUB

/* Exclude get subdevice info from this input provider.
   This define MUST before include input_driver.h.                   */
#define OVERRIDE_DRIVER_GET_SUBDEVICE_NAME_FUNCTION_STUB
#include <core/input_driver.h>


DFB_INPUT_DRIVER( mstar_linux_input )

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

/*When dfb initialize, use polling to scan every device node*/
#define POLLING_COUNT 20

/* When continus plug-in, the great amount of data pour in the socket buffer at the same time.
    It may cause DFB can't create the thread to monitor the device  becasuse the message drop.
    To avoid device hase no response, we increase the socket buffer size and recv buffer size to fix this problem */
#define MAX_LENGTH_OF_EVENT_STRING 1024 * 16


#define CHECK_SYSCALL_ERROR(X)\
do{\
    int _retval = X;\
    if(_retval == -1){\
        int err_no = errno;\
        DBG_INPUT_MSG("[DFB]  %s : "#X" fail!(%s)\n", __FUNCTION__, strerror(err_no));\
    }\
}while(0)


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

#ifdef HAVE_ANDROID_OS
#define SLEEP_TIME      500*1000
#else
#define SLEEP_TIME      1000*1000
#endif

/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice         *device;
     DirectThread            *thread;

     int                      fd;

     bool                     has_leds;
     unsigned long            led_state[NBITS(LED_CNT)];
     DFBInputDeviceLockState  locks;

     int                      vt_fd;

     int                      dx;
     int                      dy;

     bool                     touchpad;
     int                      index;
     char*                pDeviceName;
     int                       sensitive_numerator;
     int                       sensitive_denominator;
     int                       rel_axis_x_motion_reminder;
     int                       rel_axis_y_motion_reminder;
} LinuxInputData;


#define MAX_LINUX_INPUT_DEVICES 16

#define WRITE_PIPE  1
#define READ_PIPE 0

static int num_devices = 0;
static char *device_names[MAX_LINUX_INPUT_DEVICES];
/* The entries with the same index in device_names and device_nums are the same
 * are used in two different forms (one is "/dev/input/eventX", the other is
 * X).
 */
static int               device_nums[MAX_LINUX_INPUT_DEVICES] = { 0 };
/* Socket file descriptor for getting udev events. */
static int               socket_fd = 0;
/* Pipe file descriptor for terminating the hotplug thread. */
static int               hotplug_quitpipe[2];
/* The hot-plug thread that is launched by the launch_hotplug() function. */
static DirectThread     *hotplug_thread = NULL;
static DirectThread     *hotplug_thread_polling = NULL;
static DirectThread     *hotplug_thread_usbir_repeat = NULL;
/* The driver suspended lock mutex. */
static pthread_mutex_t   driver_suspended_lock;
/* The device lock mutex */
static pthread_mutex_t   keymap_repository_lock;
static pthread_mutex_t   keyboard_driver_lock;
static pthread_mutex_t   usbir_driver_lock;
static pthread_mutex_t   usbir_repeat_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t         usbir_cond = PTHREAD_COND_INITIALIZER;

/* Flag that indicates if the driver is suspended when true. */
static bool              driver_suspended = false;
static bool              usbir_exist = false;
/* For device polling thread */
static DirectThread *device_polling_thread[MAX_LINUX_INPUT_DEVICES];
static int device_polling_quitpipe[MAX_LINUX_INPUT_DEVICES][2];

static int device_type[MAX_LINUX_INPUT_DEVICES];
static int device_position[MAX_LINUX_INPUT_DEVICES];


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

     DIKI_KP_ENTER, DIKI_CONTROL_R, DIKI_KP_DIV, DIKI_PRINT, DIKI_ALT_R,

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
     /*KEY_HOMEPAGE, KEY_REFRESH,*/ DIKS_HOMEPAGE, DIKS_SHUFFLE,

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
     /* KEY_SEARCH         */  DIKS_SEARCH,
     /* KEY_CONNECT        */  DIKI_UNKNOWN,
     /* KEY_FINANCE        */  DIKI_UNKNOWN,
     /* KEY_SPORT          */  DIKI_UNKNOWN,
     /* KEY_SHOP           */  DIKI_UNKNOWN,
     /* KEY_ALTERASE       */  DIKI_UNKNOWN,
     /* KEY_CANCEL         */  DIKS_CANCEL,
     /* KEY_BRIGHTNESSDOWN */  DIKI_UNKNOWN,
     /* KEY_BRIGHTNESSUP   */  DIKI_UNKNOWN,
     /* KEY_MEDIA          */  DIKS_MIDEA,
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
     DIKS_PREVIOUS, DIKS_NEXT, DIKS_DIGITS, DIKS_TEEN, DIKS_TWEN, DIKS_BREAK,
     DIKS_ZOOMIN,DIKS_ZOOMOUT
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




/**********************************************************************************************************************/

typedef struct {
     int                 magic;

     int                 num;
     InputDeviceShared  *devices[MAX_INPUTDEVICES];
} DFBInputCoreShared;

struct __DFB_DFBInputCore {
     int                 magic;

     CoreDFB            *core;

     DFBInputCoreShared *shared;

     DirectLink         *drivers;
     DirectLink         *devices;
};

typedef struct {
    struct input_event levt[64];
    DFBInputDeviceKeymapEntry *entry[64];
    int length;
} InputDevicePipeData;

typedef struct S_LOGIC_MOUSE {
     int mousePipe[2];
      int mousefd[MAX_LINUX_INPUT_DEVICES];
      char device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
      char physical_device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
      bool isPhysicalAttached[MAX_LINUX_INPUT_DEVICES];
}LOGIC_MOUSE_T;

typedef struct S_LOGIC_KEYBOARD {
     int keyboardPipe[2];
      int keyboardfd[MAX_LINUX_INPUT_DEVICES];
      char device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
      char physical_device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH]; //the real device name

       int keyboardfd2[MAX_LINUX_INPUT_DEVICES];
      char device_name2[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];

      bool isPhysicalAttached[MAX_LINUX_INPUT_DEVICES];
}LOGIC_KEYBOARD_T;


typedef struct S_LOGIC_JOYSTICK {
    int joystickPipe[2];
    int joystickfd[MAX_LINUX_INPUT_DEVICES];
    char device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
    char physical_device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];

    int joystickfd2[MAX_LINUX_INPUT_DEVICES];
    char device_name2[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];

    bool isPhysicalAttached[MAX_LINUX_INPUT_DEVICES];
}LOGIC_JOYSTICK_T;

typedef struct S_LOGIC_USBIR {
    int usbirPipe[2];
    int usbirfd[MAX_LINUX_INPUT_DEVICES];
    char device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
    char physical_device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH]; //the real device name

    int usbirfd2[MAX_LINUX_INPUT_DEVICES];
    char device_name2[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];

    bool isPhysicalAttached[MAX_LINUX_INPUT_DEVICES];
}LOGIC_USBIR_T;

typedef enum
{
    LOGIC_MOUSE = 0,
    LOGIC_KEYBOARD,
    LOGIC_JOYSTICK,
    LOGIC_USBIR,
    LOGIC_DEVICE_NUMBER,
}E_LOGIC_DEVICE;

typedef struct _KEYMAP_REPOSITORY {
    char physical_device_name[MAX_LINUX_INPUT_DEVICES][DEVICE_NAME_MAX_LENGTH];
    char *filename[MAX_LINUX_INPUT_DEVICES];
    int count;
}KEYMAP_REPOSITORY;

static LOGIC_MOUSE_T g_logic_mouse;
static LOGIC_KEYBOARD_T g_logic_keyboard;
static LOGIC_JOYSTICK_T g_logic_joystick;
static LOGIC_USBIR_T g_logic_usbir;
static bool isHotplugExit = false;
static KEYMAP_REPOSITORY keymap_repository;

static char *logic_device_names[LOGIC_DEVICE_NUMBER];
static bool isPress = false;
static int  recordKeyCode = 0;
static bool isFollow = false;
static bool keyUp = false;
typedef struct _LogicDeviceFuncs
{
    DFBResult  (*OpenLogicDevice) (CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data);
    void  (*CloseLogicDevice)(void);
}LogicDeviceFuncs;


typedef struct _BTRCDevice
{
     CoreInputDevice         *device;
      DFBInputEvent            devt;
}BTRCDevice;


static LogicDeviceFuncs gLogicDeviceFuncs[LOGIC_DEVICE_NUMBER];
static BTRCDevice tempBTRCDevice;
static DFBResult  linux_open_logic_mouse(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data);
static DFBResult  linux_open_logic_keyboard(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data);
static DFBResult  linux_open_logic_usbir(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data);
static DFBResult  linux_open_logic_joystick(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data);
static void  linux_close_logic_mouse(void);
static void  linux_close_logic_keyboard(void);
static void  linux_close_logic_joystick(void);
static void  linux_close_logic_usbir(void);
static int find_register_physical_device_keymap(char *name);


static bool
linux_attache_physicalDevice(DFBInputCoreShared   *core_input, char * dev,  int *deviceType, int *devicePosition);

static void*
linux_input_hotplug_thread(DirectThread *thread, void *arg);

static void
touchpad_fsm_init( struct touchpad_fsm_state *state );
static int
touchpad_fsm( struct touchpad_fsm_state *state,
              const struct input_event  *levt,
              DFBInputEvent             *devt );

static bool
timeout_passed( const struct timeval *timeout, const struct timeval *current );
static bool
timeout_is_set( const struct timeval *timeout );
static void
timeout_sub( struct timeval *timeout, const struct timeval *sub );




u32 linux_GetSystemTime(void)
{
    struct timespec         ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec* 1000+ ts.tv_nsec/1000000;
}
u32 linux_DiffTimeFromNow(u32  ti)
{
    return linux_GetSystemTime()-ti;
}

#define CONVERSION 1000
#define CONVERSION_TWO 1000 * 1000
#define CONVERSION_THREE 1000 * 1000 * 1000
#define WAIT_TIME (dfb_config->mst_new_ir_first_repeat_time - dfb_config->mst_usbir_repeat_time)
static void*
linux_input_hotplug_thread_usbir_repeat(DirectThread *thread, void *arg)
{
    int count = 0;
    int event_num , dev_idx = 0;

    struct timespec ts = {0};
    int ret = 0;
    DBG_INPUT_MSG("[DFB][%s %d][Input]----------------> thread  start\n", __FUNCTION__, __LINE__);

    while (usbir_exist) {
        keyUp = false;
        usleep(WAIT_TIME * CONVERSION);
        /* When isPress is false means key release
             recordKeyCode and tempBTRCDevice.devt.key_code to avoid previous key in while loop
         */
        while ((usbir_exist) && (isPress) && (recordKeyCode == (tempBTRCDevice.devt.key_code)) ) {
            if (keyUp == true)
                break;

            usleep (dfb_config->mst_usbir_repeat_time * CONVERSION);

            if ((isPress) && (recordKeyCode == (tempBTRCDevice.devt.key_code)) ) {
                if (keyUp == true)
                    break;
        DBG_INPUT_MSG("[DFB][%s %d] ~~~~~~~~~~ keyUp=%d\n", __FUNCTION__, __LINE__, keyUp);

                struct timeval now;
                gettimeofday( &now, NULL );
                BTRCDevice repeatDevice;
                repeatDevice = tempBTRCDevice;
                repeatDevice.devt.timestamp.tv_sec = now.tv_sec;
                repeatDevice.devt.timestamp.tv_usec = now.tv_usec;
                repeatDevice.devt.flags |= DIEF_REPEAT;
                /* devt.type 1 means key press */
                if (tempBTRCDevice.devt.type == 1) {
                    if (keyUp == true || isPress == false) {
                        break;
                    }

                    dfb_input_dispatch( (repeatDevice.device), &(repeatDevice.devt) );

                }

            } else {
                break;
            }

        }
    }

    DBG_INPUT_MSG("[DFB][%s %d][Input]----------------> thread  exit\n", __FUNCTION__, __LINE__);

    return NULL;
}

/*
 * Translates a Linux input keycode into a DirectFB keycode.
 */
static int
translate_key( unsigned short code )
{

     if (code < D_ARRAY_SIZE( basic_keycodes ))
          return basic_keycodes[code];

     if (code >= KEY_OK){
          if (code - KEY_OK < D_ARRAY_SIZE( ext_keycodes ))
               return ext_keycodes[code-KEY_OK];
          else if( code <= KEY_MAX)
               return DIKI_KEY_UNMAPPING;
     }
     return DIKI_UNKNOWN;
}

static DFBInputDeviceKeySymbol
keyboard_get_symbol( int                             code,
                     unsigned short                  value,
                     DFBInputDeviceKeymapSymbolIndex level )
{
     unsigned char type  = KTYP(value);
     unsigned char index = KVAL(value);
     int           base  = (level == DIKSI_BASE);

     switch (type) {
          case KT_FN:
               if (index < 20)
               {
                 /*
                  refactory..
                  translate keycode 114 into symbol supposed to be KEY_VOLUMEDOWN
                  which derived from linux tranlate table, but it translate into functional key.
                  DFB use plain map and with KTYP, KVAL to obtain default keymap symbol.
                  keycode 114 is defined ad functional key during the translations.
                  Therefore, in keymap.entry[114], the default symbol is defined as functional key.
                  After that, the symbol obtain from linux tranlate table could be replace with default keymap.
                  We modify the keymap.entry[114] symbol as DIKS_VOLUME_DOWN to resolve this problem.
                 */
                    if (index == 12) //keycode 113
                        return DIKS_MUTE;
                    else if (index == 13) //keycode 114
                        return DIKS_VOLUME_DOWN;
                    else
                        return DFB_FUNCTION_KEY( index + 1 );
               }
               break;
          case KT_LETTER:
          case KT_LATIN:
               switch (index) {
                    case 0x1c:
                         return DIKS_PRINT;
                    case 0x7f:
                         return DIKS_BACKSPACE;
                    case 0xa4:
                         return 0x20ac; /* euro currency sign */
                    default:
                         return index;
               }
               break;
          case KT_DEAD:
               switch (value) {
                    case K_DGRAVE:
                         return DIKS_DEAD_GRAVE;

                    case K_DACUTE:
                         return DIKS_DEAD_ACUTE;

                    case K_DCIRCM:
                         return DIKS_DEAD_CIRCUMFLEX;

                    case K_DTILDE:
                         return DIKS_DEAD_TILDE;

                    case K_DDIERE:
                         return DIKS_DEAD_DIAERESIS;

                    case K_DCEDIL:
                         return DIKS_DEAD_CEDILLA;

                    default:
                         break;
               }
               break;
          case KT_PAD:
               if (index <= 9 && level != DIKSI_BASE)
                    return DIKS_0 + index;
               break;
          case 0xe: /* special IPAQ H3600 case - AH */
               switch (index) {
                    case 0x20:     return DIKS_CALENDAR;
                    case 0x1a:     return DIKS_BACK;
                    case 0x1c:     return DIKS_MEMO;
                    case 0x21:     return DIKS_POWER;
               }
               break;
          case 0xd: /* another special IPAQ H3600 case - AH */
               switch (index) {
                    case 0x2:     return DIKS_DIRECTORY;
                    case 0x1:     return DIKS_MAIL;  /* Q on older iPaqs */
               }
               break;
     }

     switch (value) {
          case K_LEFT:    return DIKS_CURSOR_LEFT;
          case K_RIGHT:   return DIKS_CURSOR_RIGHT;
          case K_UP:      return DIKS_CURSOR_UP;
          case K_DOWN:    return DIKS_CURSOR_DOWN;
          case K_ENTER:   return DIKS_ENTER;
          case K_CTRL:    return DIKS_CONTROL;
          case K_SHIFT:   return DIKS_SHIFT;
          case K_ALT:     return DIKS_ALT;
          case K_ALTGR:   return DIKS_ALTGR;
          case K_INSERT:  return DIKS_INSERT;
          case K_REMOVE:  return DIKS_DELETE;
          case K_FIND:    return DIKS_HOME;
          case K_SELECT:  return DIKS_END;
          case K_PGUP:    return DIKS_PAGE_UP;
          case K_PGDN:    return DIKS_PAGE_DOWN;
          case K_NUM:     return DIKS_NUM_LOCK;
          case K_HOLD:    return DIKS_SCROLL_LOCK;
          case K_PAUSE:   return DIKS_PAUSE;
          case K_BREAK:   return DIKS_BREAK;
          case K_CAPS:    return DIKS_CAPS_LOCK;

          case K_P0:      return DIKS_INSERT;
          case K_P1:      return DIKS_END;
          case K_P2:      return DIKS_CURSOR_DOWN;
          case K_P3:      return DIKS_PAGE_DOWN;
          case K_P4:      return DIKS_CURSOR_LEFT;
          case K_P5:      return DIKS_BEGIN;
          case K_P6:      return DIKS_CURSOR_RIGHT;
          case K_P7:      return DIKS_HOME;
          case K_P8:      return DIKS_CURSOR_UP;
          case K_P9:      return DIKS_PAGE_UP;
          case K_PPLUS:   return DIKS_PLUS_SIGN;
          case K_PMINUS:  return DIKS_MINUS_SIGN;
          case K_PSTAR:   return DIKS_ASTERISK;
          case K_PSLASH:  return DIKS_SLASH;
          case K_PENTER:  return DIKS_ENTER;
          case K_PCOMMA:  return base ? DIKS_DELETE : DIKS_COMMA;
          case K_PDOT:    return base ? DIKS_DELETE : DIKS_PERIOD;
          case K_PPARENL: return DIKS_PARENTHESIS_LEFT;
          case K_PPARENR: return DIKS_PARENTHESIS_RIGHT;
     }

     /* special keys not in the map, hack? */
     if (code == 99)
          return DIKS_PRINT;

     if (code == 124)         /* keypad equal key */
          return DIKS_EQUALS_SIGN;

     if (code == 125)         /* left windows key */
          return DIKS_META;

     if (code == 126)         /* right windows key */
          return DIKS_META;

     if (code == 127)         /* context menu key */
          return DIKS_SUPER;

     return DIKS_NULL;
}

static DFBInputDeviceKeyIdentifier
keyboard_get_identifier( int code, unsigned short value )
{
     unsigned char type  = KTYP(value);
     unsigned char index = KVAL(value);

     if (type == KT_PAD) {
          if (index <= 9)
               return DIKI_KP_0 + index;

          switch (value) {
               case K_PSLASH: return DIKI_KP_DIV;
               case K_PSTAR:  return DIKI_KP_MULT;
               case K_PMINUS: return DIKI_KP_MINUS;
               case K_PPLUS:  return DIKI_KP_PLUS;
               case K_PENTER: return DIKI_KP_ENTER;
               case K_PCOMMA:
               case K_PDOT:   return DIKI_KP_DECIMAL;
          }
     }

     /* Looks like a hack, but don't know a better way yet. */
     switch (code) {
          case 12: return DIKI_MINUS_SIGN;
          case 13: return DIKI_EQUALS_SIGN;
          case 26: return DIKI_BRACKET_LEFT;
          case 27: return DIKI_BRACKET_RIGHT;
          case 39: return DIKI_SEMICOLON;
          case 40: return DIKI_QUOTE_RIGHT;
          case 41: return DIKI_QUOTE_LEFT;
          case 43: return DIKI_BACKSLASH;
          case 51: return DIKI_COMMA;
          case 52: return DIKI_PERIOD;
          case 53: return DIKI_SLASH;
          case 54: return DIKI_SHIFT_R;
          case 97: return DIKI_CONTROL_R;
          case 100: return DIKI_ALT_R;
          default:
               ;
     }

     /* special keys not in the map, hack? */
     if (code == 124)         /* keypad equal key */
          return DIKI_KP_EQUAL;

     if (code == 125)         /* left windows key */
          return DIKI_META_L;

     if (code == 126)         /* right windows key */
          return DIKI_META_R;

     if (code == 127)         /* context menu key */
          return DIKI_SUPER_R;

     return DIKI_UNKNOWN;
}

#include "defkeymap.hxx"
static unsigned short
keyboard_read_value( const LinuxInputData *data,
                     unsigned char table, unsigned char index )
{

     if( data->vt_fd >= 0)
    {
            struct kbentry entry;

            entry.kb_table = table;
            entry.kb_index = index;
            entry.kb_value = 0;
            if (ioctl( data->vt_fd, KDGKBENT, &entry )) {
                          D_PERROR("DirectFB/keyboard: KDGKBENT (table: %d, index: %d) "
                                     "failed!\n", table, index);
                       return 0;
            }

            return entry.kb_value;
     }
     else
     {
              u_short *key_map, val;
              //read kernel default keymap
             key_map = key_maps[table];
             if (key_map) {
                   val = U(key_map[index]);
        //    if ( KTYP(val) >= NR_TYPES)
        //    val = K_HOLE;
             } else
                   val = (index ? K_HOLE : K_NOSUCHMAP);
        return val;
     }
}

/*
 * Translates key and button events.
 */
static bool
key_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     int code = levt->code;


     /* map touchscreen and smartpad events to button mouse */
     if (code == BTN_TOUCH || code == BTN_TOOL_FINGER)
          code = BTN_MOUSE;

     if ((code >= BTN_MOUSE && code < BTN_JOYSTICK) || code == BTN_TOUCH) {
          /* ignore repeat events for buttons */
          if (levt->value == 2)
               return false;

          devt->type   = levt->value ? DIET_BUTTONPRESS : DIET_BUTTONRELEASE;
          /* don't set DIEF_BUTTONS, it will be set by the input core */
          devt->button = DIBI_FIRST + code - BTN_MOUSE;
     }
     else if((code >= BTN_JOYSTICK)&&(code < BTN_GAMEPAD)) {
                  /* ignore repeat events for buttons */
          if (levt->value == 2)
               return false;

          devt->type   = levt->value ? DIET_BUTTONPRESS : DIET_BUTTONRELEASE;
          /* don't set DIEF_BUTTONS, it will be set by the input core */
          devt->button = DIBI_FIRST + code - BTN_JOYSTICK;
     }
     else if((code >= BTN_GAMEPAD)&&(code < BTN_DIGI)) {
                  /* ignore repeat events for buttons */
          if (levt->value == 2)
               return false;

          devt->type   = levt->value ? DIET_BUTTONPRESS : DIET_BUTTONRELEASE;
          /* don't set DIEF_BUTTONS, it will be set by the input core */
          devt->button = DIBI_FIRST + code - BTN_GAMEPAD;
     }
     else {
          int key = translate_key( code );

          if (key == DIKI_UNKNOWN)
               return false;

          devt->type = levt->value ? DIET_KEYPRESS : DIET_KEYRELEASE;

          if (DFB_KEY_TYPE(key) == DIKT_IDENTIFIER) {
               devt->key_id = key;
               devt->flags |= DIEF_KEYID;
          }
          else {
               devt->key_symbol = key;
               devt->flags |= DIEF_KEYSYMBOL;
               if ((DFB_KEY_TYPE(key) == DIKT_SPECIAL) && dfb_config->mst_modify_symbol_by_keymap)
                    devt->flags &= ~DIEF_KEYSYMBOL;
          }

          devt->flags |= DIEF_KEYCODE;
          devt->key_code = code;
     }

     if (levt->value == 2)
          devt->flags |= DIEF_REPEAT;

     return true;
}

/*
 * Translates relative axis events.
 */
static bool
rel_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     switch (levt->code) {
          case REL_X:
               devt->axis = DIAI_X;
               devt->axisrel = levt->value;
               break;

          case REL_Y:
               devt->axis = DIAI_Y;
               devt->axisrel = levt->value;
               break;

          case REL_Z:
          case REL_WHEEL: /* vertical scroll wheels */
               devt->axis = DIAI_Z;
               devt->axisrel = -levt->value;
               break;

          case REL_HWHEEL: /* horizontal scroll wheels */
               devt->axis = DIAI_H;
               devt->axisrel = -levt->value;
               break;

          default:
               if (levt->code > REL_MAX || levt->code > DIAI_LAST)
                    return false;
               devt->axis = levt->code;
               devt->axisrel = levt->value;
     }

     devt->type    = DIET_AXISMOTION;
     devt->flags  |= DIEF_AXISREL;

     return true;
}

/*
 * Translates absolute axis events.
 */
static bool
abs_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     switch (levt->code) {
          case ABS_X:
               devt->axis = DIAI_X;
               break;

          case ABS_Y:
               devt->axis = DIAI_Y;
               break;

          case ABS_Z:
          case ABS_WHEEL:
               devt->axis = DIAI_Z;
               break;

          default:
               if (levt->code >= ABS_PRESSURE || levt->code > DIAI_LAST)
                    return false;
               devt->axis = levt->code;
     }

     devt->type    = DIET_AXISMOTION;
     devt->flags  |= DIEF_AXISABS;
     devt->axisabs = levt->value;

     return true;
}

/*
 * Translates a Linux input event into a DirectFB input event.
 */
static bool
translate_event( const struct input_event *levt,
                 DFBInputEvent            *devt )
{
     devt->flags     = DIEF_TIMESTAMP;
     devt->timestamp = levt->time;


     switch (levt->type) {
          case EV_KEY:
               return key_event( levt, devt );

          case EV_REL:
               return rel_event( levt, devt );

          case EV_ABS:
               return abs_event( levt, devt );

          default:
               ;
     }

     return false;
}

static void
set_led( const LinuxInputData *data, int led, int state )
{
     struct input_event levt = {0};

     levt.type = EV_LED;
     levt.code = led;
     levt.value = !!state;

     CHECK_SYSCALL_ERROR(write( data->fd, &levt, sizeof(levt) ));
}

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
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void*
linux_input_EventThread( DirectThread *thread, void *driver_data )
{
    DBG_INFO("[directFB:input]-------->start linux_input_EventThread-\n");
     LinuxInputData    *data = (LinuxInputData*) driver_data;
     int                readlen, status;
     unsigned int       i;
     int                fdmax;
     fd_set             set;
     struct touchpad_fsm_state fsm_state;
     InputDevicePipeData pipedata;
     memset(&pipedata, 0, sizeof(InputDevicePipeData));
     char device_name[DEVICE_NAME_MAX_LENGTH] = {0};
     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

     fdmax = data->fd ;
       DBG_INPUT_MSG("[directFB:input]--------> linux_input_EventThread-- data->fd=%d,  fdmax=%d\n", data->fd,  fdmax);
     /* Query min/max coordinates. */
     if (data->touchpad) {
          struct input_absinfo absinfo;

          touchpad_fsm_init( &fsm_state );

          ioctl( data->fd, EVIOCGABS(ABS_X), &absinfo );
          fsm_state.x.min = absinfo.minimum;
          fsm_state.x.max = absinfo.maximum;

          ioctl( data->fd, EVIOCGABS(ABS_Y), &absinfo );
          fsm_state.y.min = absinfo.minimum;
          fsm_state.y.max = absinfo.maximum;
     }
      DBG_INPUT_MSG("--- linux_input_EventThread------data->fd:%d,  data->pDeviceName:%s\n", data->fd, data->pDeviceName);

     while (1) {
          #if 0
          DBG_INFO("-- linux_input_EventThread    polling....\n");
          #endif


          DFBInputEvent devt = { .type = DIET_UNKNOWN };

          FD_ZERO( &set );
          FD_SET( data->fd, &set );

          if (data->touchpad && timeout_is_set( &fsm_state.timeout )) {
               struct timeval time;
               gettimeofday( &time, NULL );

               if (!timeout_passed( &fsm_state.timeout, &time )) {
                    struct timeval timeout = fsm_state.timeout;
                    timeout_sub( &timeout, &time );
                    status = select( fdmax + 1, &set, NULL, NULL, &timeout );
               } else {
                    status = 0;
               }
          }
          else {
               struct timeval timeout={1,0};
               status = select( fdmax + 1, &set, NULL, NULL, &timeout );
          }

          if (status < 0 && errno != EINTR)
          {
               printf(" linux_input_EventThread  break line:%d errno=%d\n", __LINE__, errno);
               break;
          }

          /*
          if (FD_ISSET( data->quitpipe[0], &set ))
          {
              printf(" linux_input_EventThread  break line:%d\n", __LINE__);
               break;
          }
          */

          direct_thread_testcancel( thread );

          if (status < 0)
               continue;

          /* timeout? */
          if (status == 0) {
               if (data->touchpad && touchpad_fsm( &fsm_state, NULL, &devt ) > 0)
                    dfb_input_dispatch( data->device, &devt );

               continue;
          }
          DBG_INPUT_MSG("-----------------[%s]-----------------------before read\n", __FUNCTION__);
          memset(&pipedata, 0, sizeof(InputDevicePipeData));
          readlen = read( data->fd, &pipedata, sizeof(pipedata));
          DBG_INPUT_MSG("-----------------[%s]-----------------------readlen:%d\n", __FUNCTION__, readlen);
          if (readlen < 0 && errno != EINTR)
          {
               printf(" linux_input_EventThread  break line:%d errno=%d\n", __LINE__, errno);
               break;
          }

          if (strcmp(thread->name, "Linux Input Keyboard") == 0) {
               int readRet = read( data->fd, &device_name, sizeof(device_name));
               if (readRet <= 0) {
                   DBG_INPUT_MSG("[%s %d] read fail errno=%d\n", __FUNCTION__, __LINE__, errno);
                   break;
               }
          }

          direct_thread_testcancel( thread );
          if (readlen <= 0)
               continue;
          for (i=0; i< pipedata.length; i++) {
                DBG_INPUT_MSG("--- linux_input_EventThread------data->fd:%d,  data->pDeviceName:%s i:%d\n", data->fd, data->pDeviceName, i);
                DFBInputEvent temp = { .type = DIET_UNKNOWN };
                /* mute mic device is regarded as keyboard type in get_device_info */
                if ((strcmp(thread->name, "Linux Input Keyboard") == 0) && (strcmp(device_name, "mic_mute") == 0)) {
                    if ((pipedata.entry[i] != NULL) && (pipedata.entry[i]->code == 0) && (pipedata.levt[i].code == dfb_config->mst_gpio_key_code)) {
                        DBG_INPUT_MSG("[DFB][%s %d] mute mic GPIO flow pid=%d\n", __FUNCTION__, __LINE__, getpid());
                        temp.type = pipedata.levt[i].value ? DIET_KEYPRESS : DIET_KEYRELEASE;
                        temp.key_code = pipedata.entry[i]->code;
                        temp.flags     = DIEF_TIMESTAMP | DIEF_KEYCODE | DIEF_KEYSYMBOL | DIEF_KEYID;
                        temp.timestamp = pipedata.levt[i].time;
                        temp.key_id = pipedata.entry[i]->identifier;
                        /* CL: 6982883 get dfb_input_dispatch event */
                        dfb_input_dispatch( data->device, &temp );
                        break;
                    }
                }

               if ( (pipedata.entry[i] != NULL) && (pipedata.entry[i]->code != 0))
               {


                    InputDeviceShared  *shared  = data->device->shared;
                    /* If physical keymap has value, replace key_symbol and key_id*/
                    if (pipedata.levt[i].type != EV_KEY)
                        continue;
                    temp.type = pipedata.levt[i].value ? DIET_KEYPRESS : DIET_KEYRELEASE;
                    temp.key_code = pipedata.entry[i]->code;
                    temp.flags     = DIEF_TIMESTAMP | DIEF_KEYCODE | DIEF_KEYSYMBOL | DIEF_KEYID;
                    temp.timestamp = pipedata.levt[i].time;
                    temp.key_id = pipedata.entry[i]->identifier;
                    temp.modifiers = shared->modifiers_l | shared->modifiers_r;
                    temp.locks = shared->locks;

                    DFBInputDeviceKeymapSymbolIndex index =(temp.modifiers & DIMM_ALTGR) ? DIKSI_ALT : DIKSI_BASE;

                    if (!(temp.modifiers & DIMM_SHIFT) ^ !(pipedata.entry[i]->locks & temp.locks))
                         index++;

                    /* don't modify modifiers */
                    if (DFB_KEY_TYPE( pipedata.entry[i]->symbols[DIKSI_BASE] ) == DIKT_MODIFIER)
                        temp.key_symbol = pipedata.entry[i]->symbols[DIKSI_BASE];
                    else
                        temp.key_symbol = pipedata.entry[i]->symbols[index];

                    if (pipedata.levt[i].value == 2)
                        temp.flags |= DIEF_REPEAT;

               }
               else
               {

                    if (data->touchpad) {
                        status = touchpad_fsm( &fsm_state, &pipedata.levt[i], &temp );
                        if (status < 0) {
                             /* Not handled. Try the direct approach. */
                             if (!translate_event( &pipedata.levt[i], &temp ))
                                  continue;
                        }
                        else if (status == 0) {
                             /* Handled but no further processing is necessary. */
                             continue;
                        }
                    }
                    else {

                        if (!translate_event( &pipedata.levt[i], &temp ))
                             continue;
                    }
               }
               /* Flush previous event with DIEF_FOLLOW? */
               if (devt.type != DIET_UNKNOWN) {
                    flush_xy( data, false );

                    /* Signal immediately following event. */
                    devt.flags |= DIEF_FOLLOW;
                    dfb_input_dispatch( data->device, &devt );
                    isFollow = true;
                    if (data->has_leds && (devt.locks != data->locks)) {
                         set_led( data, LED_SCROLLL, devt.locks & DILS_SCROLL );
                         set_led( data, LED_NUML, devt.locks & DILS_NUM );
                         set_led( data, LED_CAPSL, devt.locks & DILS_CAPS );
                         data->locks = devt.locks;
                    }
                    devt.type  = DIET_UNKNOWN;
                    devt.flags = DIEF_NONE;
               }
               devt = temp;
               if(D_FLAGS_IS_SET( devt.flags, DIEF_AXISREL ) && devt.type == DIET_AXISMOTION)
               {
                  switch (devt.axis)
                  {
                         case DIAI_X:
                               data->rel_axis_x_motion_reminder  += devt.axisrel*data->sensitive_numerator;
                              devt.axisrel = data->rel_axis_x_motion_reminder/data->sensitive_denominator;
                              data->rel_axis_x_motion_reminder %=data->sensitive_denominator;
                              break;

                         case DIAI_Y:
                              data->rel_axis_y_motion_reminder  += devt.axisrel*data->sensitive_numerator;
                              devt.axisrel = data->rel_axis_y_motion_reminder/data->sensitive_denominator;
                              data->rel_axis_y_motion_reminder %=data->sensitive_denominator;
                              break;
                          default:
                             break;
                   }
                }



               if (D_FLAGS_IS_SET( devt.flags, DIEF_AXISREL ) && devt.type == DIET_AXISMOTION &&
                   dfb_config->mouse_motion_compression)
               {
                    switch (devt.axis) {
                         case DIAI_X:
                              data->dx += devt.axisrel;
                              continue;

                         case DIAI_Y:
                              data->dy += devt.axisrel;
                              continue;

                         default:
                              break;
                    }
               }
               /* Event is dispatched in next round of loop. */
          }
          /* Flush last event without DIEF_FOLLOW. */
          if (devt.type != DIET_UNKNOWN) {

               flush_xy( data, false );

               if (strcmp(thread->name, "Linux Input usbir") == 0) {
                   /* devt.type 2 means key up, when key up, block BTRC repeat flow */
                   if (devt.type == 2) {
                         isFollow = false;
                         recordKeyCode = -1;
                         isPress = false;
                         keyUp = true;
                   }
               }

               dfb_input_dispatch( data->device, &devt );

               if (strcmp(thread->name, "Linux Input usbir") == 0) {

                   tempBTRCDevice.device = data->device;
                   tempBTRCDevice.devt = devt;
                   /* devt.type 1 means key press */
                   if (devt.type == 1 && (isFollow == false)) {
                        if((isPress == false)) {
                             recordKeyCode = devt.key_code;
                             isPress = true;
                        }

                   }
               }

               if (data->has_leds && (devt.locks != data->locks)) {
                    set_led( data, LED_SCROLLL, devt.locks & DILS_SCROLL );
                    set_led( data, LED_NUML, devt.locks & DILS_NUM );
                    set_led( data, LED_CAPSL, devt.locks & DILS_CAPS );
                    data->locks = devt.locks;
               }
          }
          else
               flush_xy( data, true );
     }

     if (status <= 0)
          D_PERROR ("linux_input thread died\n");

     return NULL;
}

/*
 * Fill device information.
 * Queries the input device and tries to classify it.
 */
static void
get_device_info( int              fd,
                 InputDeviceInfo *info,
                 bool            *touchpad )
{
     unsigned int  num_keys     = 0;
     unsigned int  num_ext_keys = 0;
     unsigned int  num_buttons  = 0;
     unsigned int  num_rels     = 0;
     unsigned int  num_abs      = 0;

     unsigned long evbit[NBITS(EV_CNT)] = {0};
     unsigned long keybit[NBITS(KEY_CNT)] = {0};
     unsigned long relbit[NBITS(REL_CNT)] = {0};
     unsigned long absbit[NBITS(ABS_CNT)] = {0};

    /* get bus type for query the capability of blue tooth support*/
     struct input_id input_info;

     if (ioctl(fd, EVIOCGID, &input_info) < 0)
          DBG_INPUT_MSG("[DFB] IOCTL EVIOCGID fail\n");

     /* get device name */
     ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), info->desc.name );
     DBG_INPUT_MSG("[%s %d], Device name %s\n", __FUNCTION__, __LINE__, info->desc.name);

     if ((strcmp(info->desc.name, "mic_mute") == 0)) {
         info->desc.type = DIDTF_KEYBOARD;
         info->prefered_id = DIDID_KEYBOARD;
         return;
     }

     /* check if it's a keypad*/
     char     *pos = NULL;
     pos = strstr(info->desc.name, "KEYPAD");
     if (pos)
     {
        info->desc.type & DIDTF_NONE;
        info->prefered_id = DIDID_KEYPAD;
        return;
     }

     /* set device vendor */
     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Linux" );

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


     /* Mouse, Touchscreen or Smartpad ? */
     if ((test_bit( EV_KEY, evbit ) &&
          (test_bit( BTN_TOUCH, keybit ) || test_bit( BTN_TOOL_FINGER, keybit ))) ||
          ((num_rels >= 2 && num_buttons)  ||  (num_abs == 2 && (num_buttons == 1)))) {

          DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
          info->desc.type |= DIDTF_MOUSE;

     }else if (num_abs && num_buttons) /* Or a Joystick? */{

          DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
          info->desc.type |= DIDTF_JOYSTICK;

     }

     /* A Keyboard, do we have at least some letters? */
     if (num_keys > 20) {

          DBG_INPUT_MSG("[DFB] %s,%d num_keys=%d\n",__FUNCTION__,__LINE__, num_keys);
          info->desc.type |= DIDTF_KEYBOARD;
          info->desc.caps |= DICAPS_KEYS;

          info->desc.min_keycode = 0;
          info->desc.max_keycode = 127;
     }

     /* A Remote Control? */
     if (num_ext_keys) {
          DBG_INPUT_MSG("[DFB] %s,%d, num_ext_keys=%d\n",__FUNCTION__,__LINE__,num_ext_keys);
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

    if ((input_info.bustype == BUS_BLUETOOTH) && (info->desc.type & DIDTF_REMOTE))
    {
         DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
         if( info->desc.type & DIDTF_KEYBOARD){
            int key, miss_keys=0;

            /* To clarify if this input device is keyboard?
                 if this device has KeyA~KeyZ, CapsLock, TAB, Space and Alt, it means that's a keyboard.
            */
            for (key=KEY_Q; key<=KEY_P; key++)
            {
                if (!test_bit( key, keybit )){
                    DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                    miss_keys++;
                }
            }

            for (key=KEY_A; key<=KEY_L; key++)
            {
                if (!test_bit( key, keybit )){
                    DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                    miss_keys++;
                }
            }

            for (key=KEY_Z; key<=KEY_M; key++)
            {
                if (!test_bit( key, keybit )){
                    DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                    miss_keys++;
                }
            }

            if (!(test_bit( KEY_CAPSLOCK, keybit ))){
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                miss_keys++;
            }

            if (!(test_bit( KEY_TAB, keybit ))){
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                miss_keys++;
            }

            if (!(test_bit( KEY_SPACE, keybit ))){
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                miss_keys++;
            }

            if (!(test_bit( KEY_LEFTSHIFT, keybit ) |  test_bit( KEY_RIGHTSHIFT, keybit ))){
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                miss_keys++;
            }

            if (!(test_bit( KEY_LEFTALT, keybit ) |  test_bit( KEY_RIGHTALT, keybit ))){
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                miss_keys++;
            }

            if (miss_keys)
            {
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                info->desc.type = DIDTF_REMOTE;
            }
            else
            {
                DBG_INPUT_MSG("[DFB] %s,%d\n",__FUNCTION__,__LINE__);
                info->desc.type = DIDTF_KEYBOARD;
            }
       }
       else
       {
            /* If the input device is bluetooth ir , defined as DIDTF_REMOTE(usb-ir) */
            info->desc.type = DIDTF_REMOTE;
       }

       if ( (num_keys > 20 ? (num_ext_keys > 8) : num_ext_keys) &&
            (strstr(info->desc.name, "Keyboard") == NULL) &&
            (strcasestr(info->desc.name, "KB") == NULL)) {
            /* If the input device is bluetooth ir , defined as DIDTF_REMOTE(usb-ir) */
            info->desc.type = DIDTF_REMOTE;
            DBG_INPUT_MSG( "[%s %d] DIDTF_REMOTE\n", __FUNCTION__, __LINE__);
       } else {
            info->desc.type = DIDTF_KEYBOARD;
            DBG_INPUT_MSG( "[%s %d] DIDTF_KEYBOARD\n", __FUNCTION__, __LINE__);
       }
    }

    if ((strstr(info->desc.name, "Keyboard") != NULL) || (strcasestr(info->desc.name, "KB") != NULL)) {
           info->desc.type = DIDTF_KEYBOARD;
           DBG_INPUT_MSG( "[%s %d] DIDTF_KEYBOARD\n", __FUNCTION__, __LINE__);
    }

    if ((input_info.vendor == dfb_config->mst_input_vendor_id) && (input_info.product == dfb_config->mst_input_product_id)) {
          info->desc.type = DIDTF_REMOTE;
          DBG_INPUT_MSG( "[%s %d] DIDTF_REMOTE\n", __FUNCTION__, __LINE__);
    }

    if((info->desc.type & DIDTF_KEYBOARD) && (info->desc.type & DIDTF_REMOTE) && (info->desc.type & DIDTF_MOUSE))
    {
         /* Fix mantis 1784878
            Some device has been recognized as the wrong type(ex: mouse as MSTARIR type), but can not modify the device type.
            Therefore, we add the device name to improve the preciseness of device type,
            only the device name is MStar Smart TV IR Receiver has been recognized as the MSTARIR type(means Mstar own IR of public version),
            and the remaining is determinded in the following order: mouse, keyboard, joystick and remote
         */

         char name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH];
         memset( name, 0, sizeof(name));
         /* get device name */
         ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), name );
         DBG_INPUT_MSG("[DFB] %s, Device name %s\n", __FUNCTION__, name);

         /* fusion : MStar Smart TV IR Receiver
            merak  : MTK PMU IR
            mixed  : MTK Smart TV IR Receiver
         */
         if ( (strcmp(name, "MStar Smart TV IR Receiver")==0) || (strcmp(name, "MTK PMU IR")==0)  || (strcmp(name, "MTK Smart TV IR Receiver")==0) )
         {
              DBG_INPUT_MSG("[DFB] This is MSTAR IR\n");
              info->desc.type = DIDTF_MSTARIR;
         }
         else if (info->desc.type & DIDTF_MOUSE)
              info->desc.type = DIDTF_MOUSE;
         else if (info->desc.type & DIDTF_KEYBOARD)
              info->desc.type = DIDTF_KEYBOARD;
         else if (info->desc.type & DIDTF_JOYSTICK)
              info->desc.type = DIDTF_JOYSTICK;
         else
              info->desc.type = DIDTF_REMOTE;
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
}

static bool
check_device( const char *device )
{
     int  fd;
           #if 0
            if(linux_DiffTimeFromNow(debugtime) > 2000)
            {
                printf("hotplug polling  check  device......\n");
                 debugtime = linux_GetSystemTime();
            }
            #endif

     /* Check if we are able to open the device */
     fd = open( device, O_RDWR );
     if (fd < 0) {
          return false;
     }
     else {
        //printf("open device :%s  success\n",device);
          InputDeviceInfo info;
          bool touchpad;

          /* try to grab the device */
          if (dfb_config->linux_input_grab) {
               /* 2.4 kernels don't have EVIOCGRAB so ignore EINVAL */
               if (ioctl( fd, EVIOCGRAB, 1 ) && errno != EINVAL) {
                    close( fd );
                    return false;
               }
          }

          memset( &info, 0, sizeof(InputDeviceInfo) );

          get_device_info( fd, &info, &touchpad );


          if (dfb_config->linux_input_grab)
               ioctl( fd, EVIOCGRAB, 0 );
          close( fd );

          if (!dfb_config->linux_input_ir_only ||
              (info.desc.type & DIDTF_REMOTE))
               return true;
     }

     return false;
}


/*
 *new mstar_linux_input start!
*/

static void*
device_mouse_polling_thread(DirectThread *thread, void *arg)
{
    int *dev_idx_ptr = (int *)arg;
    int dev_idx;
    if(dev_idx_ptr != NULL)
        dev_idx = *dev_idx_ptr;
    else {
        printf("[%s %d] arg is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(dev_idx < 0 || dev_idx >= MAX_LINUX_INPUT_DEVICES) {
        DBG_INPUT_MSG("[DFB] %s, %d : index out of bound!\n",__FUNCTION__,__LINE__);
        return NULL;
    }

    int log_idx = device_position[dev_idx];
    /* dev_idx_ptr malloc in add_hotplug_device, need free it */
    D_FREE(dev_idx_ptr);
    dev_idx_ptr = NULL;

    fd_set set;
    unsigned int i = 0;

    InputDevicePipeData pipedata;
    memset(&pipedata, 0, sizeof(InputDevicePipeData));

    THREAD_DBG_INFO("%s,  idx=%d, start!\n", __FUNCTION__, log_idx);
    while (g_logic_mouse.isPhysicalAttached[log_idx])
    {
        FD_ZERO(&set);
        FD_SET(g_logic_mouse.mousefd[log_idx], &set);
        FD_SET(device_polling_quitpipe[dev_idx][READ_PIPE], &set);

        THREAD_DBG_INFO("%s, idx=%d, select!\n", __FUNCTION__, log_idx);
        int status = select( g_logic_mouse.mousefd[log_idx] + 1, &set, NULL, NULL, NULL );
        if(status <= 0 ||  !FD_ISSET(g_logic_mouse.mousefd[log_idx], &set))
        {
            continue;
        }

        if(direct_thread_is_canceled(thread))
        {
            direct_thread_testcancel( thread );
            printf("\nthread is not canceled ====%s:%d \n",__FUNCTION__,__LINE__);
        }

        DBG_INPUT_MSG("======================%s read..      device :%s================\n", __FUNCTION__, g_logic_mouse.device_name[log_idx]);

        int readlen = read(g_logic_mouse.mousefd[log_idx], pipedata.levt, sizeof(pipedata.levt) );
        DBG_INPUT_MSG("======================%s write readlen:%d================\n", __FUNCTION__, readlen);
        if(readlen > 0)
        {
              pipedata.length = readlen / sizeof(pipedata.levt[0]);
              for (i=0; i < readlen / sizeof(pipedata.levt[0]); i++)
                 pipedata.entry[i] = NULL;
             DBG_INPUT_MSG("======================%s write...================\n", __FUNCTION__);

             CHECK_SYSCALL_ERROR(write(g_logic_mouse.mousePipe[WRITE_PIPE], &pipedata, sizeof(pipedata)));

             DBG_INPUT_MSG("======================%s write end..================\n", __FUNCTION__);
        }
        else
        {
             DBG_INPUT_MSG("[%s]----------should remove device \n", __FUNCTION__);

             if (g_logic_mouse.mousefd[log_idx] > 0) {
                 DBG_FORCE_PRINT("[%s]-------->Detect  mouse device:%s  disconnected\n", __FUNCTION__, g_logic_mouse.device_name[log_idx]);
                close(g_logic_mouse.mousefd[log_idx]);
                g_logic_mouse.mousefd[log_idx]= -1;
             }
             g_logic_mouse.isPhysicalAttached[log_idx] = false;
             break;
        }
    }
    DBG_INPUT_MSG("%s, idx=%d, stop!\n", __FUNCTION__, log_idx);
    return NULL;
}

static void*
device_keyboard_polling_thread(DirectThread *thread, void *arg)
{
    int *dev_idx_ptr = (int *)arg;
    int dev_idx;
    if(dev_idx_ptr != NULL)
        dev_idx = *dev_idx_ptr;
    else {
        printf("[%s %d] arg is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(dev_idx < 0 || dev_idx >= MAX_LINUX_INPUT_DEVICES) {
        DBG_INPUT_MSG("[DFB] %s, %d : index out of bound!\n",__FUNCTION__,__LINE__);
        return NULL;
    }

    int log_idx = device_position[dev_idx];
    /* dev_idx_ptr malloc in add_hotplug_device, need free it */
    D_FREE(dev_idx_ptr);
    dev_idx_ptr = NULL;

    fd_set set;
    unsigned int i = 0 ;

    InputDevicePipeData pipedata;
    memset(&pipedata, 0, sizeof(InputDevicePipeData));

    DBG_INPUT_MSG("%s, idx=%d, start!\n", __FUNCTION__, log_idx);
    while(g_logic_keyboard.isPhysicalAttached[log_idx])
    {
        FD_ZERO(&set);
        FD_SET(g_logic_keyboard.keyboardfd[log_idx], &set);
        FD_SET(device_polling_quitpipe[dev_idx][READ_PIPE], &set);

        THREAD_DBG_INFO("%s, idx=%d, select!\n", __FUNCTION__, log_idx);
        int status = select( g_logic_keyboard.keyboardfd[log_idx] + 1, &set, NULL, NULL, NULL );
        if(status <= 0 ||  !FD_ISSET(g_logic_keyboard.keyboardfd[log_idx], &set))
        {
            continue;
        }

        if(direct_thread_is_canceled(thread))
        {
            direct_thread_testcancel( thread );
            printf("\nthread is not canceled ====%s:%d \n",__FUNCTION__,__LINE__);
        }

        DBG_INPUT_MSG("======================%s read..      device :%s================\n", __FUNCTION__, g_logic_keyboard.device_name[log_idx]);

        int readlen = read(g_logic_keyboard.keyboardfd[log_idx], pipedata.levt, sizeof(pipedata.levt) );
        DBG_INPUT_MSG("======================%s write readlen:%d================\n", __FUNCTION__, readlen);
        if(readlen > 0)
        {
            /*get input event from sub device keymap*/
            pipedata.length = readlen / sizeof(pipedata.levt[0]);
            for (i=0; i < readlen / sizeof(pipedata.levt[0]); i++)
            {
                pthread_mutex_lock(&keyboard_driver_lock);
                pipedata.entry[i] = dfb_input_get_entry_from_sub_device_keymap( DIDID_KEYBOARD, log_idx, pipedata.levt[i].code);
                pthread_mutex_unlock(&keyboard_driver_lock);

            }
            DBG_INPUT_MSG("======================%s write...================\n", __FUNCTION__);

            CHECK_SYSCALL_ERROR(write(g_logic_keyboard.keyboardPipe[WRITE_PIPE], &pipedata, sizeof(pipedata)));
            CHECK_SYSCALL_ERROR(write(g_logic_keyboard.keyboardPipe[WRITE_PIPE], &g_logic_keyboard.physical_device_name[log_idx], sizeof(g_logic_keyboard.physical_device_name[log_idx])));
            DBG_INPUT_MSG("======================%s write end..================\n", __FUNCTION__);
        }
        else
        {
            DBG_INPUT_MSG("[%s]----------should remove device \n", __FUNCTION__);

            if(g_logic_keyboard.keyboardfd[log_idx] > 0 )
            {
                DBG_FORCE_PRINT("[%s]------>Detect  keyboard device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_keyboard.device_name[log_idx], __LINE__);
                close(g_logic_keyboard.keyboardfd[log_idx]);
                g_logic_keyboard.keyboardfd[log_idx] = -1;
            }
            if(g_logic_keyboard.keyboardfd2[log_idx] > 0 )
            {
                DBG_FORCE_PRINT("[%s]------>Detect  keyboard device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_keyboard.device_name2[log_idx], __LINE__);
                close(g_logic_keyboard.keyboardfd2[log_idx]);
                 g_logic_keyboard.keyboardfd2[log_idx] = -1;
            }
            g_logic_keyboard.isPhysicalAttached[log_idx] = false;
            break;
        }
    }
    DBG_INPUT_MSG("%s, idx=%d, stop!\n", __FUNCTION__, log_idx);
    return NULL;
}

static void*
device_joystick_polling_thread(DirectThread *thread, void *arg)
{
    int *dev_idx_ptr = (int *)arg;
    int dev_idx;
    if(dev_idx_ptr != NULL)
        dev_idx = *dev_idx_ptr;
    else {
        printf("[%s %d] arg is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(dev_idx < 0 || dev_idx >= MAX_LINUX_INPUT_DEVICES) {
        DBG_INPUT_MSG("[DFB] %s, %d : index out of bound!\n",__FUNCTION__,__LINE__);
        return NULL;
    }

    int log_idx = device_position[dev_idx];
    /* dev_idx_ptr malloc in add_hotplug_device, need free it */
    D_FREE(dev_idx_ptr);
    dev_idx_ptr = NULL;


    fd_set set;
    unsigned int i = 0;

    InputDevicePipeData pipedata;
    memset(&pipedata, 0, sizeof(InputDevicePipeData));

    DBG_INPUT_MSG("%s, idx=%d, start!\n", __FUNCTION__, log_idx);
    while(g_logic_joystick.isPhysicalAttached[log_idx])
    {
        FD_ZERO(&set);
        FD_SET(g_logic_joystick.joystickfd[log_idx], &set);
        FD_SET(device_polling_quitpipe[dev_idx][READ_PIPE], &set);

        int status = select( g_logic_joystick.joystickfd[log_idx] + 1, &set, NULL, NULL, NULL );
        if(status <= 0 ||  !FD_ISSET(g_logic_joystick.joystickfd[log_idx], &set))
        {
            continue;
        }

        if(direct_thread_is_canceled(thread))
        {
            direct_thread_testcancel( thread );
            printf("\nthread is not canceled ====%s:%d \n",__FUNCTION__,__LINE__);
        }

        DBG_INPUT_MSG("======================%s read..      device :%s================\n", __FUNCTION__, g_logic_joystick.device_name[log_idx]);
        int readlen = read(g_logic_joystick.joystickfd[log_idx], pipedata.levt, sizeof(pipedata.levt) );
        DBG_INPUT_MSG("======================%s write readlen:%d================\n", __FUNCTION__, readlen);
        if(readlen > 0)
        {
              pipedata.length = readlen / sizeof(pipedata.levt[0]);
              for (i=0; i < readlen / sizeof(pipedata.levt[0]); i++)
                 pipedata.entry[i] = NULL;
             DBG_INPUT_MSG("======================%s write...================\n", __FUNCTION__);

              CHECK_SYSCALL_ERROR(write(g_logic_joystick.joystickPipe[WRITE_PIPE], &pipedata, sizeof(pipedata)));

              DBG_INPUT_MSG("======================%s write end..================\n", __FUNCTION__);
        }
        else
        {
            DBG_INPUT_MSG("[%s]----------should remove device \n", __FUNCTION__);
            if(g_logic_joystick.joystickfd[log_idx] > 0 )
            {
                DBG_FORCE_PRINT("[%s]------>Detect  joystick device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_joystick.device_name[log_idx], __LINE__);
                close(g_logic_joystick.joystickfd[log_idx]);
                g_logic_joystick.joystickfd[log_idx] = -1;
            }
            if(g_logic_joystick.joystickfd2[log_idx] > 0 )
            {
                DBG_FORCE_PRINT("[%s]------>Detect  joystick device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_joystick.device_name2[log_idx], __LINE__);
                close(g_logic_joystick.joystickfd2[log_idx]);
                g_logic_joystick.joystickfd2[log_idx] = -1;
            }
            g_logic_joystick.isPhysicalAttached[log_idx] = false;
            break;
        }
    }
    DBG_INPUT_MSG("%s, idx=%d, stop!\n", __FUNCTION__, log_idx);
    return NULL;
}

static void*
device_usbir_polling_thread(DirectThread *thread, void *arg)
{
    int *dev_idx_ptr = (int *)arg;
    int dev_idx;
    if(dev_idx_ptr != NULL)
        dev_idx = *dev_idx_ptr;
    else {
        printf("[%s %d] arg is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(dev_idx < 0 || dev_idx >= MAX_LINUX_INPUT_DEVICES) {
        DBG_INPUT_MSG("[DFB] %s, %d : index out of bound!\n",__FUNCTION__,__LINE__);
        return NULL;
    }

    int log_idx = device_position[dev_idx];
    /* dev_idx_ptr malloc in add_hotplug_device, need free it */
    D_FREE(dev_idx_ptr);
    dev_idx_ptr = NULL;



    fd_set set;
    unsigned int i = 0;

    InputDevicePipeData pipedata;
    memset(&pipedata, 0, sizeof(InputDevicePipeData));


    DBG_INPUT_MSG("[%s %d], idx=%d, start!\n", __FUNCTION__, __LINE__, log_idx);
    while(g_logic_usbir.isPhysicalAttached[log_idx])
    {
        FD_ZERO(&set);
        FD_SET(g_logic_usbir.usbirfd[log_idx], &set);
        FD_SET(device_polling_quitpipe[dev_idx][READ_PIPE], &set);

        int status = select( g_logic_usbir.usbirfd[log_idx] + 1, &set, NULL, NULL, NULL );
        if(status <= 0 ||  !FD_ISSET(g_logic_usbir.usbirfd[log_idx], &set))
        {
            continue;
        }

        if(direct_thread_is_canceled(thread))
        {
            direct_thread_testcancel( thread );
            printf("\nthread is not canceled ====%s:%d \n",__FUNCTION__,__LINE__);
        }

        DBG_INPUT_MSG("======================%s read..      device :%s================\n", __FUNCTION__, g_logic_usbir.device_name[log_idx]);




        int readlen = read(g_logic_usbir.usbirfd[log_idx], pipedata.levt, sizeof(pipedata.levt) );
        DBG_INPUT_MSG("======================%s write readlen:%d================ errno=%d\n", __FUNCTION__, readlen, errno);
        if(readlen > 0)
        {
             /*get input event from sub device keymap*/
             pipedata.length = readlen / sizeof(pipedata.levt[0]);
             for (i=0; i < readlen / sizeof(pipedata.levt[0]); i++)
             {
                    pthread_mutex_lock(&usbir_driver_lock);
                    pipedata.entry[i] = dfb_input_get_entry_from_sub_device_keymap( DIDID_REMOTE, log_idx, pipedata.levt[i].code);
                    pthread_mutex_unlock(&usbir_driver_lock);
                    /* pipedata.levt[i].value 0 means key up, when key up, block BTRC repeat flow */
                    if (i == 1 && pipedata.levt[i].value == 0) {
                         recordKeyCode = -1;
                         isPress = false;
                         keyUp = true;
                    }
             }

             DBG_INPUT_MSG("======================%s write...================\n", __FUNCTION__);

             CHECK_SYSCALL_ERROR(write(g_logic_usbir.usbirPipe[WRITE_PIPE], &pipedata, sizeof(pipedata)));

             DBG_INPUT_MSG("======================%s write end..================\n", __FUNCTION__);
        }
        else
        {
            DBG_INPUT_MSG("[%s]----------should remove device \n", __FUNCTION__);
            if(g_logic_usbir.usbirfd[log_idx] > 0 )
            {
                DBG_INPUT_MSG("[%s]------>Detect  usbir device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_usbir.device_name[log_idx], __LINE__);
                close(g_logic_usbir.usbirfd[log_idx]);
                g_logic_usbir.usbirfd[log_idx] = -1;
            }
            if(g_logic_usbir.usbirfd2[log_idx] > 0 )
            {
                DBG_INPUT_MSG("[%s]------>Detect  usbir device:%s  disconnected(%d)\n", __FUNCTION__, g_logic_usbir.device_name2[log_idx], __LINE__);
                close(g_logic_usbir.usbirfd2[log_idx]);
                g_logic_usbir.usbirfd2[log_idx] = -1;
            }
            g_logic_usbir.isPhysicalAttached[log_idx] = false;
            break;
        }
    }
    DBG_INPUT_MSG("[%s %d], idx=%d, stop!\n", __FUNCTION__, __LINE__, log_idx);
    return NULL;
}

static void
add_hotplug_device(int dev_num, void *arg )
{
    int   dev_idx, ret;

    char buf[32];
    DFBInputCoreShared *core_input = (DFBInputCoreShared *)arg;

    if(dev_num >= MAX_LINUX_INPUT_DEVICES)
        return;

    ret = snprintf( buf, sizeof(buf), "/dev/input/event%d", dev_num );
    if(ret < 0)
        D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);

    for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
    {
         if(device_nums[dev_idx]   == dev_num){

            DBG_FORCE_PRINT("[%s], device_names[%d] : %s is exist!!\n", __FUNCTION__, dev_idx, buf);
            return;
         }
    }
    int devicetype = -1;
    int deviceposition = -1;

    bool isAttachedSuccess = linux_attache_physicalDevice(core_input,  buf, &devicetype, &deviceposition);

    if ( isAttachedSuccess && devicetype != (int)DIDTF_MSTARIR )
    {
        DBG_INPUT_MSG("[%s %d]-----Detect new device:%s\n", __FUNCTION__, __LINE__ , buf);

        for (dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
        {
            if (device_nums[dev_idx] == MAX_LINUX_INPUT_DEVICES) {
                device_nums[dev_idx] = dev_num;
                num_devices++;
                break;
            }
        }

        if ( dev_idx < MAX_LINUX_INPUT_DEVICES )
        {
            if(device_names[dev_idx] == NULL)
            {

                    /* pass dev_idx_ptr to polling thread argument, allocate it first */
                    int *dev_idx_ptr = D_MALLOC( sizeof(int));

                    if (!dev_idx_ptr) {
                        DBG_INPUT_MSG( "[%s %d] Can not allocate memory!\n", __FUNCTION__ , __LINE__);
                        return;
                    }

                    /* assign dev_idx_ptr value */
                    *dev_idx_ptr = dev_idx;

                    /* create pipe for polling thread. */
                    ret = pipe(device_polling_quitpipe[dev_idx]);
                    if (ret < 0) {
                          DBG_INPUT_MSG( "DirectFB/linux_input: could not open quitpipe" );
                          D_FREE(dev_idx_ptr);
                          dev_idx_ptr = NULL;
                          return;
                    }

                    char * toAttacheDev= D_STRDUP( buf );
                    DFBResult res = DFB_OK;

                        device_names[dev_idx] = toAttacheDev;
                        device_type[dev_idx] = devicetype;
                        device_position[dev_idx] = deviceposition;

                        DBG_INPUT_MSG("[%s %d]-----Attache Physical device:%s  SUCCESS!\ndevice_names[%d] : %s\n", __FUNCTION__, __LINE__ ,
                            toAttacheDev, dev_idx, device_names[dev_idx]);

                        /* device thread to connect and transport to logic device. */
                        if (devicetype == (int)DIDTF_MOUSE) {
                            if (device_polling_thread[dev_idx] !=NULL)
                                printf("device_polling_thread[%d] is exist!\n", dev_idx);
                            device_polling_thread[dev_idx] = direct_thread_create( DTT_DEFAULT, device_mouse_polling_thread, dev_idx_ptr, "mouse polling" );

                            if(device_polling_thread[dev_idx] == NULL) {
                                   D_FREE(dev_idx_ptr);
                                   dev_idx_ptr = NULL;
                            }
                        }
                        else if (devicetype == (int)DIDTF_KEYBOARD) {
                            if (device_polling_thread[dev_idx]!=NULL)
                                printf("device_polling_thread[%d] is exist!\n", dev_idx);
                            device_polling_thread[dev_idx] = direct_thread_create( DTT_DEFAULT, device_keyboard_polling_thread, dev_idx_ptr, "keyboard polling" );

                            if(device_polling_thread[dev_idx] == NULL) {
                                   D_FREE(dev_idx_ptr);
                                   dev_idx_ptr = NULL;
                            }

                            pthread_mutex_lock(&keymap_repository_lock);
                            /* check if keymap has loaded before input device plug-in*/
                            int keymap_index = find_register_physical_device_keymap(g_logic_keyboard.physical_device_name[deviceposition]);
                            if (keymap_index != INVALID_DEVICE_INDEX)
                            {
                                DBG_INPUT_MSG("[DFB] device name :%s, keymap has loaded, index =%d, filename = %s\n",g_logic_keyboard.physical_device_name[deviceposition], keymap_index,keymap_repository.filename[keymap_index]);
                                res = dfb_input_allocate_physical_device_keymap(DIDID_KEYBOARD, deviceposition, keymap_repository.filename[keymap_index]);
                            }
                            else
                            {
                                DBG_INPUT_MSG("[DFB] device name :%s, keymap have not loaded yet..\n", g_logic_keyboard.physical_device_name[deviceposition]);
                                res = dfb_input_allocate_physical_device_keymap(DIDID_KEYBOARD, deviceposition, NULL);
                            }
                            pthread_mutex_unlock(&keymap_repository_lock);

                            if (res)
                                DBG_INPUT_MSG("[DFB] keymap allocate physical device keymap has error, ret = %d\n", res);
                        }
                        else if (devicetype == (int)DIDTF_JOYSTICK) {
                            if (device_polling_thread[dev_idx]!=NULL)
                                printf("device_polling_thread[%d] is exist!\n", dev_idx);
                            device_polling_thread[dev_idx] = direct_thread_create( DTT_DEFAULT, device_joystick_polling_thread, dev_idx_ptr, "joystick polling" );

                            if(device_polling_thread[dev_idx] == NULL) {
                                   D_FREE(dev_idx_ptr);
                                   dev_idx_ptr = NULL;
                            }

                        }
                        else if (devicetype == (int)DIDTF_REMOTE) {
                            if (device_polling_thread[dev_idx]!=NULL)
                                printf("device_polling_thread[%d] is exist!\n", dev_idx);
                            device_polling_thread[dev_idx] = direct_thread_create( DTT_DEFAULT, device_usbir_polling_thread, dev_idx_ptr, "usbir polling" );
                            pthread_condattr_t attr;
                            pthread_condattr_init(&attr);
                            pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
                            pthread_cond_init(&usbir_cond, &attr);
                            pthread_condattr_destroy(&attr);

                            if (hotplug_thread_usbir_repeat == NULL){
                                 usbir_exist = true;
                                 hotplug_thread_usbir_repeat = direct_thread_create( DTT_DEFAULT, linux_input_hotplug_thread_usbir_repeat, NULL, "usbir repeat" );
                            }

                            if(device_polling_thread[dev_idx] == NULL || hotplug_thread_usbir_repeat == NULL) {
                                   D_FREE(dev_idx_ptr);
                                   dev_idx_ptr = NULL;
                            }

                            pthread_mutex_lock(&keymap_repository_lock);
                            /* check if keymap has loaded before input device plug-in*/
                            int keymap_index = find_register_physical_device_keymap(g_logic_usbir.physical_device_name[deviceposition]);

                            if (keymap_index != INVALID_DEVICE_INDEX)
                            {
                                DBG_INPUT_MSG("device name :%s, keymap has loaded, index =%d, filename = %s\n",g_logic_usbir.physical_device_name[deviceposition], keymap_index,keymap_repository.filename[keymap_index]);
                                res = dfb_input_allocate_physical_device_keymap(DIDID_REMOTE, deviceposition, keymap_repository.filename[keymap_index]);
                            }
                            else
                            {
                                DBG_INPUT_MSG("device name :%s, keymap have not loaded yet..\n",g_logic_usbir.physical_device_name[deviceposition]);
                                res = dfb_input_allocate_physical_device_keymap(DIDID_REMOTE, deviceposition, NULL);
                            }
                            pthread_mutex_unlock(&keymap_repository_lock);

                            if (res)
                                DBG_INPUT_MSG("[DFB] keymap allocate physical device keymap has error, ret = %d\n", res);
                        }
                        else {
                                    /* if don't enter any polling thread, free dev_idx_ptr */
                                    D_FREE(dev_idx_ptr);
                                    dev_idx_ptr = NULL;
                        }
                }
                else
                {
                    /* If there is no valid devicetype, reset device_nums and device_names. */
                    if(device_names[dev_idx])
                    {
                         D_FREE(device_names[dev_idx]);
                         device_names[dev_idx] = NULL;
                    }

                    device_nums[dev_idx] = MAX_LINUX_INPUT_DEVICES;
                }
            }
    }
}

static void
remove_hotplug_device( int dev_num )
{
    if(dev_num >= MAX_LINUX_INPUT_DEVICES)
        return;

    int dev_idx, log_idx,ret = 0;
    char buf[32];
    bool bfind = false;

    for (dev_idx=0; dev_idx<MAX_LINUX_INPUT_DEVICES; dev_idx++) {
        if (device_nums[dev_idx] == dev_num) {
            device_nums[dev_idx] = MAX_LINUX_INPUT_DEVICES;
            num_devices--;
            DBG_INPUT_MSG("[%s %d] dev_num=%d dev_idx=%d \n", __FUNCTION__, __LINE__, dev_num, dev_idx);

            D_FREE(device_names[dev_idx]);
            device_names[dev_idx] = NULL;
            bfind = true;
            break;
        }
    }

    if (!bfind)
    {
        printf("[DFB] No device_nums was found\n");
        return;
    }


    ret = snprintf( buf, sizeof(buf), "/dev/input/event%d", dev_num );
    if(ret < 0){
        D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
        return;
    }

    log_idx = device_position[dev_idx];

    DBG_INPUT_MSG("[%s]----------dev : %s, dev_idx = %d, log_idx = %d \n", __FUNCTION__, buf, dev_idx, log_idx);

    if(device_polling_thread[dev_idx]) {
        DBG_INPUT_MSG("[%s %d] call direct_thread_cancel name=%s\n", __FUNCTION__, __LINE__, device_polling_thread[dev_idx]->name);
        direct_thread_cancel( device_polling_thread[dev_idx] );
    }else{
        printf("[DFB] device_polling_thread[%d] is NULL\n", dev_idx);
        return;
    }

    CHECK_SYSCALL_ERROR(write( device_polling_quitpipe[dev_idx][WRITE_PIPE], " ", 1 ));
    direct_thread_join( device_polling_thread[dev_idx] );
    direct_thread_destroy( device_polling_thread[dev_idx] );
    device_polling_thread[dev_idx] = NULL;
    close( device_polling_quitpipe[dev_idx][READ_PIPE] );
    close( device_polling_quitpipe[dev_idx][WRITE_PIPE] );

    switch(device_type[dev_idx])
    {
    case DIDTF_MOUSE:
        if(g_logic_mouse.mousefd[log_idx] > 0)
        {
            if(strcmp(g_logic_mouse.device_name[log_idx], buf) == 0)
            {
                DBG_INPUT_MSG("[%s]-------->Detect  mouse device:%s  disconnected\n", __FUNCTION__, g_logic_mouse.device_name[log_idx]);

                close(g_logic_mouse.mousefd[log_idx]);
                g_logic_mouse.mousefd[log_idx]= -1;
                g_logic_mouse.isPhysicalAttached[log_idx] = false;
                memset(&g_logic_mouse.physical_device_name[log_idx], 0, DEVICE_NAME_MAX_LENGTH);
            }
        }
        break;
    case DIDTF_KEYBOARD:
        if (g_logic_keyboard.keyboardfd[log_idx] > 0)
        {
            if (strcmp(g_logic_keyboard.device_name[log_idx], buf) == 0)
            {
                DBG_INPUT_MSG("[%s]-------->Detect  keyboard device:%s  disconnected\n",
                                 __FUNCTION__, g_logic_keyboard.device_name[log_idx]);

                close(g_logic_keyboard.keyboardfd[log_idx]);
                g_logic_keyboard.keyboardfd[log_idx]= -1;
                g_logic_keyboard.isPhysicalAttached[log_idx] = false;
                memset(&g_logic_keyboard.physical_device_name[log_idx], 0, DEVICE_NAME_MAX_LENGTH);
                if(g_logic_keyboard.keyboardfd2[log_idx] > 0 )
                {
                     DBG_INPUT_MSG("[%s]------>Detect  keyboard device:%s  disconnected(%d)\n",
                                     __FUNCTION__, g_logic_keyboard.device_name2[log_idx], __LINE__);

                     close(g_logic_keyboard.keyboardfd2[log_idx]);
                     g_logic_keyboard.keyboardfd2[log_idx] = -1;
                }
                g_logic_keyboard.isPhysicalAttached[log_idx] = false;
                pthread_mutex_lock(&keyboard_driver_lock);
                dfb_input_deallocate_physical_device_keymap(DIDID_KEYBOARD, log_idx);
                pthread_mutex_unlock(&keyboard_driver_lock);
            }
        }
        break;
    case DIDTF_JOYSTICK:
        if (g_logic_joystick.joystickfd[log_idx] > 0)
        {
            if (strcmp(g_logic_joystick.device_name[log_idx], buf) == 0)
            {
                DBG_INPUT_MSG("[%s]-------->Detect  joystick device:%s  disconnected\n", __FUNCTION__, g_logic_joystick.device_name[log_idx]);

                close(g_logic_joystick.joystickfd[log_idx]);
                g_logic_joystick.joystickfd[log_idx]= -1;
                g_logic_joystick.isPhysicalAttached[log_idx] = false;
                memset(&g_logic_joystick.physical_device_name[log_idx], 0, DEVICE_NAME_MAX_LENGTH);
            }
        }
        break;
    case DIDTF_REMOTE:
        if (g_logic_usbir.usbirfd[log_idx] > 0)
        {

            if (strcmp(g_logic_usbir.device_name[log_idx], buf) == 0)
            {
                DBG_INPUT_MSG("[%s]-------->Detect  usbir device:%s  disconnected\n", __FUNCTION__, g_logic_usbir.device_name[log_idx]);
                if(g_logic_usbir.usbirfd[log_idx] > 0 ) {
                    close(g_logic_usbir.usbirfd[log_idx]);
                    g_logic_usbir.usbirfd[log_idx]= -1;
                }
                g_logic_usbir.isPhysicalAttached[log_idx] = false;
                memset(&g_logic_usbir.physical_device_name[log_idx], 0, DEVICE_NAME_MAX_LENGTH);
            }

        }

        pthread_mutex_lock(&keymap_repository_lock);
        dfb_input_deallocate_physical_device_keymap(DIDID_REMOTE, log_idx);
        pthread_mutex_unlock(&keymap_repository_lock);


        if (hotplug_thread_usbir_repeat) {
            usbir_exist = false;
            direct_thread_join(hotplug_thread_usbir_repeat);
            direct_thread_destroy( hotplug_thread_usbir_repeat );
            hotplug_thread_usbir_repeat = NULL;
        }


        break;
    default:
        THREAD_DBG_INFO("[%s]----------not match device!!\n", __FUNCTION__);
        return;
    }

    THREAD_DBG_INFO("[%s]----------end!!\n", __FUNCTION__);
}

#define USLEEP_TIME 50000

static void*
linux_input_hotplug_thread_polling(DirectThread *thread, void *arg)
{
    int count = 0;
    int event_num , dev_idx = 0;

    /* if user input the device berfore dfb initialize, we need to scan every device node to make sure every device has reponse*/
    while( count <= POLLING_COUNT){
        usleep(USLEEP_TIME);

        for(event_num=0; event_num < MAX_LINUX_INPUT_DEVICES; event_num++)
        {
           char buf[32];
           int fd = 0;
           int ret = snprintf( buf, sizeof(buf), "/dev/input/event%d", event_num );
           if(ret < 0) {
               D_ERROR("[DFB] %s, %d : snprintf failed",__FUNCTION__,__LINE__);
               return NULL;
           }

           pthread_mutex_lock(&driver_suspended_lock);

           fd = open( buf, O_RDWR ) ;

           if ( fd < 0)
           {
                for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
                {
                   /*if open device node fail, check device is removed or doesn't has any device connect this node */
                    if (device_nums[dev_idx] == event_num)
                        remove_hotplug_device(event_num);
                }
           }
           else
           {
                close(fd);
                add_hotplug_device(event_num, arg);
            }

            pthread_mutex_unlock(&driver_suspended_lock);
        }

        count++;

    }

    DBG_INPUT_MSG("[DFB][Input]---------------->polling thread  exit\n");
    return NULL;

}

static void*
linux_input_hotplug_thread_new(DirectThread *thread, void *arg)
{

    /* interrupt mode*/
     struct sockaddr_nl addr;
     int                fdmax;

     /* When continus plug-in, the great amount of data pour in the socket buffer at the same time.
         It may cause DFB can't create the thread to monitor the device  becasuse the message drop.
         To avoid device hase no response, we increase the socket buffer size and recv buffer size to fix this problem */
     const int buffersize = 1024 * 16;

     /* Open and bind the socket /org/kernel/udev/monitor */

     socket_fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
     if (socket_fd == -1) {
          DBG_INPUT_MSG( "DirectFB/linux_input: socket() failed: %s\n",
                    strerror(errno) );
          goto errorExit;
     }

     fdmax = MAX( socket_fd, hotplug_quitpipe[0] );

     memset(&addr, 0, sizeof(addr));
     addr.nl_family = AF_NETLINK;
     addr.nl_pid = pthread_self() << 16 | getpid();
     addr.nl_groups = -1;


    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    if(bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        close(socket_fd);
        return NULL;
    }
     DBG_INFO("%s:%d\n", __FUNCTION__, __LINE__);

     //while(!isHotplugExit)
     while(1)
     {
          char      udev_event[MAX_LENGTH_OF_EVENT_STRING];
          char     *pos;
          char     *event_cont; //udev event content
          int       device_num, number_file, recv_len;

          fd_set    rset;

          /* get udev event */
          FD_ZERO(&rset);
          FD_SET(socket_fd, &rset);
          FD_SET(hotplug_quitpipe[0], &rset);

          number_file = select(fdmax+1, &rset, NULL, NULL, NULL);

          //DBG_INFO("%s:%d, %d\n", __FUNCTION__, __LINE__, number_file);

          if (number_file < 0 && errno != EINTR)
               break;

          if (FD_ISSET( hotplug_quitpipe[0], &rset ))
               break;

          /* check cancel thread */
          direct_thread_testcancel( thread );

          if (FD_ISSET(socket_fd, &rset)) {
               recv_len = recv(socket_fd, udev_event, sizeof(udev_event), 0);
               if (recv_len <= 0) {
                    printf( "[DFB] error receiving uevent message: %s\n", strerror(errno) );
                    continue;
               }
               /* check cancel thread */
               direct_thread_testcancel( thread );
          }
          /* analysize udev event */
          DBG_INFO("%s: %s\n", __FUNCTION__, udev_event);

          pos = strchr(udev_event, '@');
          if (pos == NULL)
               continue;

          /* replace '@' with '\0' to separate event type and event content */
          *pos = '\0';

          event_cont = pos + 1;

          pos = strstr(event_cont, "/event");
          if (pos == NULL)
               continue;

          /* get event device number */
          device_num = atoi(pos + 6);

          /* Attempt to lock the driver suspended mutex. */
          pthread_mutex_lock(&driver_suspended_lock);
          if (driver_suspended)
          {
               /* Release the lock and quit handling hotplug events. */
               D_DEBUG_AT( Debug_LinuxInput, "Driver is suspended\n" );
               pthread_mutex_unlock(&driver_suspended_lock);
               continue;
          }

          if (!strcmp(udev_event, "add")) {
               DBG_INPUT_MSG(  "Device node /dev/input/event%d is created by udev\n", device_num);

               add_hotplug_device(device_num, arg);
          }
          else if (!strcmp(udev_event, "remove")) {
               DBG_INPUT_MSG(   "Device node /dev/input/event%d is removed by udev\n", device_num );

               remove_hotplug_device(device_num);
          }

          /* Hotplug event handling is complete so release the lock. */
          pthread_mutex_unlock(&driver_suspended_lock);
     }

     DBG_INPUT_MSG(  "Finished hotplug detection thread within Linux Input provider.\n" );
     return NULL;

errorExit:
     DBG_INPUT_MSG( "Linux/Input: Fail to open udev socket, disable detecting "
             "hotplug with Linux Input provider\n" );

     if (socket_fd != -1) {
          close(socket_fd);
     }

     return NULL;
}

/*
 * new mstar_linux_input end.
*/

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
     int j = 0;

     /* Initialize device_names and device_nums array entries. */
     for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
     {
         device_names[j] = NULL;
         device_nums[j] = MAX_LINUX_INPUT_DEVICES;
         device_type[j] = 0;
         device_position[j] = MAX_LINUX_INPUT_DEVICES;
         device_polling_thread[j] = NULL;
     }


     /* logic mouse */
     memset(&g_logic_mouse, 0, sizeof(LOGIC_MOUSE_T));
     logic_device_names[LOGIC_MOUSE] = D_STRDUP(DEV_LOGIC_MOUSE);

     CHECK_SYSCALL_ERROR(pipe(g_logic_mouse.mousePipe));

     for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
     {
         g_logic_mouse.mousefd[j] = -1;
         g_logic_mouse.isPhysicalAttached[j] = false;
         memset(&g_logic_mouse.device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
         memset(&g_logic_mouse.physical_device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
     }
     gLogicDeviceFuncs[LOGIC_MOUSE].OpenLogicDevice = linux_open_logic_mouse;
     gLogicDeviceFuncs[LOGIC_MOUSE].CloseLogicDevice = linux_close_logic_mouse;


     /* logic keyboard */
     logic_device_names[LOGIC_KEYBOARD] = D_STRDUP(DEV_LOGIC_KEYBOARD);

     CHECK_SYSCALL_ERROR(pipe(g_logic_keyboard.keyboardPipe));

     for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
     {
         g_logic_keyboard.keyboardfd[j] = -1;
         memset(&g_logic_keyboard.device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
         memset(&g_logic_keyboard.physical_device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
         g_logic_keyboard.isPhysicalAttached[j] = false;
         g_logic_keyboard.keyboardfd2[j] = -1;
         memset(&g_logic_keyboard.device_name2[j], 0, DEVICE_NAME_MAX_LENGTH);
     }
     gLogicDeviceFuncs[LOGIC_KEYBOARD].OpenLogicDevice = linux_open_logic_keyboard;
     gLogicDeviceFuncs[LOGIC_KEYBOARD].CloseLogicDevice = linux_close_logic_keyboard;


    /* logic joystick */
    logic_device_names[LOGIC_JOYSTICK] = D_STRDUP(DEV_LOGIC_JOYSTICK);

    CHECK_SYSCALL_ERROR(pipe(g_logic_joystick.joystickPipe));

    for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
    {
        g_logic_joystick.joystickfd[j] = -1;
        memset(&g_logic_joystick.device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
        memset(&g_logic_joystick.physical_device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
        g_logic_joystick.isPhysicalAttached[j] = false;
        g_logic_joystick.joystickfd2[j] = -1;
        memset(&g_logic_joystick.device_name2[j], 0, DEVICE_NAME_MAX_LENGTH);
    }
    gLogicDeviceFuncs[LOGIC_JOYSTICK].OpenLogicDevice = linux_open_logic_joystick;
    gLogicDeviceFuncs[LOGIC_JOYSTICK].CloseLogicDevice = linux_close_logic_joystick;


    /* logic usb ir */
    logic_device_names[LOGIC_USBIR] = D_STRDUP(DEV_LOGIC_USBIR);

    CHECK_SYSCALL_ERROR(pipe(g_logic_usbir.usbirPipe));

    for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
    {
        g_logic_usbir.usbirfd[j] = -1;
        memset(&g_logic_usbir.device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
        memset(&g_logic_usbir.physical_device_name[j], 0, DEVICE_NAME_MAX_LENGTH);
        g_logic_usbir.isPhysicalAttached[j] = false;
        g_logic_usbir.usbirfd2[j] = -1;
        memset(&g_logic_usbir.device_name2[j], 0, DEVICE_NAME_MAX_LENGTH);
    }
    gLogicDeviceFuncs[LOGIC_USBIR].OpenLogicDevice = linux_open_logic_usbir;
    gLogicDeviceFuncs[LOGIC_USBIR].CloseLogicDevice = linux_close_logic_usbir;


    return LOGIC_DEVICE_NUMBER;
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
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "Linux Input Driver" );
     snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "directfb.org" );

     info->version.major = 0;
     info->version.minor = 1;
}

static void linux_detach_physicalDevice(int*  pdevicefd, char * device_name)
{
    if(*pdevicefd == -1)
        return;
      close(*pdevicefd);
      int i =0;

      for(i =0; i < MAX_LINUX_INPUT_DEVICES; i++)
      {
            if(device_names[i] == NULL)
            {
                continue;
            }

            if(strcmp(device_names[i], device_name) == 0)
            {
                THREAD_DBG_INFO("[directFB:input]-------->to remove device : %s\n", device_name);
                D_FREE(device_names[i]);
                device_names[i] = NULL;
                num_devices--;
            }
      }
      *pdevicefd= -1;
}
static bool linux_detect_device_connection(int  devicefd)
{
        if(devicefd == -1)
            return false;
          struct pollfd  pfd;
          memset(&pfd,0,sizeof(pfd));

          pfd.fd = devicefd;
          pfd.events = POLLIN;
          int nready = poll(&pfd, 1, 1);    // for hotplug issue     int poll(struct pollfd fds[], nfds_t nfds, int timeout)
          if((nready == 1) && pfd.revents == (POLLERR | POLLHUP))
          {
                  return false;
          }
          else
          {
                  return true;
          }
}
static void linux_hotplug_transport(int*  pdevicefd,char *device_name,  int outputPipe)
{
    if(*pdevicefd == -1)
    {
          return;
    }
    fd_set set;
    InputDevicePipeData pipedata;
    memset(&pipedata, 0, sizeof(InputDevicePipeData));
    struct timeval timeout={0,5};
    FD_ZERO(&set);
    FD_SET(*pdevicefd, &set);

    int status = select( *pdevicefd + 1, &set, NULL, NULL, &timeout );
    if(status <= 0 ||  !FD_ISSET(*pdevicefd, &set))
    {
        return;
    }
    THREAD_DBG_INFO("======================linux_hotplug_transport read..      device :%s================\n", device_name);
    int readlen = read(*pdevicefd, pipedata.levt, sizeof(pipedata.levt) );
    THREAD_DBG_INFO("======================linux_hotplug_transport write readlen:%d================\n", readlen);
    if(readlen > 0)
    {
         unsigned int i = 0;
         pipedata.length = readlen / sizeof(pipedata.levt[0]);
         for (i=0; i < readlen / sizeof(pipedata.levt[0]); i++)
              pipedata.entry[i] = NULL;

         THREAD_DBG_INFO("======================linux_hotplug_transport write...================\n");
         CHECK_SYSCALL_ERROR(write(outputPipe, &pipedata, sizeof(pipedata)));
         THREAD_DBG_INFO("======================linux_hotplug_transport write end..================\n");
    }
    else
    {
        THREAD_DBG_INFO("\n======================linux_hotplug_transport can not read data from device %d,  may be disconnected ,close it\n", *pdevicefd);
        linux_detach_physicalDevice(pdevicefd, device_name);
    }

}
static bool linux_attache_physicalDevice(DFBInputCoreShared   *core_input, char * dev,  int *deviceType, int *devicePosition)
{
    DBG_INPUT_MSG("[directFB:input]-------->Enter  %s   line:%d    dev:%s\n", __FUNCTION__, __LINE__, dev);

    /* When parent process do fork or execve to generate
       child process, child process will copy all file descriptor that
       were opened. The file descriptor will not be closed in child
       process, leading to un-balance open/close.
       Add flag O_CLOEXEC in open, and the fd will automatically
       close in child process */
    int fd = open( dev, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        D_PERROR( "DirectFB/linux_input: could not open device" );
        DBG_INPUT_MSG("[directFB:input]-------->%s   open: %s  fail,   line:%d\n", __FUNCTION__, dev, __LINE__);
        return false;
    }
    DBG_INPUT_MSG("[directFB:input]-------->%s   open: %s  success,  line:%d\n", __FUNCTION__, dev,  __LINE__);

    /* Get real device name */
    char physical_device_name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH];
    memset( physical_device_name, 0, sizeof(physical_device_name));
    CHECK_SYSCALL_ERROR(ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), physical_device_name ));

    InputDeviceInfo device_info;
    memset( &device_info, 0, sizeof(InputDeviceInfo) );
    bool  istouchpad = false;
    get_device_info( fd, &device_info, &istouchpad);

    DBG_INPUT_MSG("[directFB:input]-------->dev: %s   device_info.desc.type: %s\n", dev,
        (device_info.desc.type&DIDTF_MOUSE) ? "DIDTF_MOUSE" :
            (device_info.desc.type&DIDTF_KEYBOARD) ? "DIDTF_KEYBOARD" :
                (device_info.desc.type&DIDTF_REMOTE) ? "DIDTF_REMOTE" :
                    (device_info.desc.type&DIDTF_JOYSTICK) ? "DIDTF_JOYSTICK" : "Unknown");

    if (device_info.prefered_id = DIDID_KEYPAD && device_info.desc.type & DIDTF_NONE )
    {
        close (fd);
        return false;
    }

    if(device_info.desc.type & DIDTF_MSTARIR)
    {
        close (fd);
        *deviceType = (int)DIDTF_MSTARIR;
        return true;
    }
    else if(device_info.desc.type & DIDTF_MOUSE)
    {
        int k = 0;
        for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
        {
            if(g_logic_mouse.mousefd[k] ==  -1 )
            {
                break;
            }
        }
      if (k<MAX_LINUX_INPUT_DEVICES) {
        g_logic_mouse.mousefd[k] = fd;
        strncpy(g_logic_mouse.device_name[k], dev, DEVICE_NAME_MAX_LENGTH);
        g_logic_mouse.device_name[k][DEVICE_NAME_MAX_LENGTH - 1] = 0;
        strncpy(g_logic_mouse.physical_device_name[k], physical_device_name, DEVICE_NAME_MAX_LENGTH);
        g_logic_mouse.physical_device_name[k][DEVICE_NAME_MAX_LENGTH - 1] = 0;
        g_logic_mouse.isPhysicalAttached[k] = true;
        *deviceType = (int)DIDTF_MOUSE;
        *devicePosition = k;

        DBG_INPUT_MSG("[directFB:input]-------->g_logic_mouse.mousefd[%d]:%d,    g_logic_mouse.device_name[%d]:%s,    g_logic_mouse.device_node[%d]:%s          %s     line:%d\n",
            k, g_logic_mouse.mousefd[k], k, g_logic_mouse.physical_device_name[k], k, g_logic_mouse.device_name[k],   __FUNCTION__, __LINE__);

        return true;
      }
    }
    else if(device_info.desc.type & DIDTF_KEYBOARD)
    {
        int kindex = 0;
        for(kindex= 0; kindex < MAX_LINUX_INPUT_DEVICES; kindex++)
        {
            if(g_logic_keyboard.keyboardfd[kindex] ==  -1 )
            {
                break;
            }
        }
      if (kindex<MAX_LINUX_INPUT_DEVICES) {
        g_logic_keyboard.keyboardfd[kindex] = fd;
        strncpy(g_logic_keyboard.physical_device_name[kindex], physical_device_name, DEVICE_NAME_MAX_LENGTH);
        g_logic_keyboard.physical_device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;
        strncpy(g_logic_keyboard.device_name[kindex], dev, DEVICE_NAME_MAX_LENGTH);
        g_logic_keyboard.device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;
        g_logic_keyboard.isPhysicalAttached[kindex] = true;
        *deviceType = (int)DIDTF_KEYBOARD;
        *devicePosition = kindex;

        DBG_INPUT_MSG("[directFB:input]-------->g_logic_keyboard.keyboardfd[%d]:%d,    g_logic_keyboard.device_name[%d]:%s,    g_logic_keyboard.device_node[%d]:%s          %s     line:%d\n",
            kindex, g_logic_keyboard.keyboardfd[kindex], kindex, g_logic_keyboard.physical_device_name[kindex], kindex, g_logic_keyboard.device_name[kindex],   __FUNCTION__, __LINE__);

        return true;
      }
    }
    else if(device_info.desc.type & DIDTF_JOYSTICK)
    {
        int kindex = 0;
        for(kindex= 0; kindex < MAX_LINUX_INPUT_DEVICES; kindex++)
        {
            if(g_logic_joystick.joystickfd[kindex] ==  -1 )
            {
                break;
            }
        }
      if (kindex<MAX_LINUX_INPUT_DEVICES) {
        g_logic_joystick.joystickfd[kindex] = fd;
        strncpy(g_logic_joystick.physical_device_name[kindex], physical_device_name, DEVICE_NAME_MAX_LENGTH);
        g_logic_joystick.physical_device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;
        strncpy(g_logic_joystick.device_name[kindex], dev, DEVICE_NAME_MAX_LENGTH);
        g_logic_joystick.device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;
        g_logic_joystick.isPhysicalAttached[kindex] = true;
        *deviceType = (int)DIDTF_JOYSTICK;
        *devicePosition = kindex;

        DBG_INPUT_MSG("[directFB:input]-------->g_logic_joystick.joystickfd[%d]:%d,    g_logic_joystick.device_name[%d]:%s,    g_logic_joystick.device_node[%d]:%s          %s     line:%d\n",
            kindex, g_logic_joystick.joystickfd[kindex], kindex, g_logic_joystick.physical_device_name[kindex], kindex, g_logic_joystick.device_name[kindex],   __FUNCTION__, __LINE__);

        return true;
      }
    }
    else if(device_info.desc.type & DIDTF_REMOTE)
    {
        int kindex = 0;
        for(kindex= 0; kindex < MAX_LINUX_INPUT_DEVICES; kindex++)
        {
            if(g_logic_usbir.usbirfd[kindex] ==  -1 )
            {
                break;
            }
        }
      if (kindex<MAX_LINUX_INPUT_DEVICES) {
        g_logic_usbir.usbirfd[kindex] = fd;
        strncpy(g_logic_usbir.physical_device_name[kindex], physical_device_name, DEVICE_NAME_MAX_LENGTH);
        g_logic_usbir.physical_device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;
        strncpy(g_logic_usbir.device_name[kindex], dev, DEVICE_NAME_MAX_LENGTH);
        g_logic_usbir.device_name[kindex][DEVICE_NAME_MAX_LENGTH -1] = 0;

        g_logic_usbir.isPhysicalAttached[kindex] = true;
        *deviceType = (int)DIDTF_REMOTE;
        *devicePosition = kindex;

        DBG_INPUT_MSG("[directFB:input]-------->g_logic_usbir.usbirfd[%d]:%d,    g_logic_usbir.device_name[%d]:%s,    g_logic_usbir.device_node[%d]:%s          %s     line:%d\n",
            kindex, g_logic_usbir.usbirfd[kindex], kindex, g_logic_usbir.physical_device_name[kindex], kindex, g_logic_usbir.device_name[kindex],   __FUNCTION__, __LINE__);

        return true;
      }
    }

    DBG_INPUT_MSG("[directFB:input]-------->Not supported  hotplug device:  %s   \n", dev);
    // If the attach device does not match mouse, keyboard, joystick and usbir, then close fd
    close(fd);

    DBG_INPUT_MSG("[directFB:input]-------->end of  %s   line:%d\n", __FUNCTION__, __LINE__);
    return false;
}

int
driver_hotplug_polling2( void *arg)
{
    static u32 lasttime = 0;
    DFBInputCoreShared *core_input = (DFBInputCoreShared *)arg;
    #if 0
    if(linux_DiffTimeFromNow(debugtime) > 2000)
    {
        printf("hotplug polling ......\n");
         debugtime = linux_GetSystemTime();
    }
    #endif

    //Moving input data from the physical devices to logical device
    int k=0;
    for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
    {
            if(g_logic_mouse.isPhysicalAttached[k])
            {
                #if 0
                if(linux_DiffTimeFromNow(debugtime2) > 2000)
                {
                    THREAD_DBG_INFO("hotplug polling   g_logic_mouse.isPhysicalAttached[%d]:%d,  g_logic_mouse.device_name[%d]:%s, g_logic_mouse.mousefd[%d]:%d......\n", k, g_logic_mouse.isPhysicalAttached[k] , k ,  g_logic_mouse.device_name[k], k, g_logic_mouse.mousefd[k]);
                    THREAD_DBG_INFO("linux_detect_device_connection(g_logic_mouse.mousefd[%d]):%d\n", k,linux_detect_device_connection(g_logic_mouse.mousefd[k]));
                     debugtime2 = linux_GetSystemTime();
                }
                #endif
                if(linux_detect_device_connection(g_logic_mouse.mousefd[k]))
                {
                    linux_hotplug_transport(&g_logic_mouse.mousefd[k], g_logic_mouse.device_name[k],  g_logic_mouse.mousePipe[1]);
                }
                else
                {
                    THREAD_DBG_INFO("----------should remove device \n");
                    if(g_logic_mouse.mousefd[k] > 0)
                    {
                        DBG_FORCE_PRINT("[directFB:input]-------->Detect  mouse device:%s  disconnected(%d)\n", g_logic_mouse.device_name[k], __LINE__);
                        linux_detach_physicalDevice(&g_logic_mouse.mousefd[k], g_logic_mouse.device_name[k]);
                        g_logic_mouse.mousefd[k]= -1;
                    }
                     g_logic_mouse.isPhysicalAttached[k] = false;
                }
            }
    }

    for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
    {
         if(g_logic_keyboard.isPhysicalAttached[k])
         {
             if(linux_detect_device_connection(g_logic_keyboard.keyboardfd[k]))
             {
                   linux_hotplug_transport(&g_logic_keyboard.keyboardfd[k], g_logic_keyboard.device_name[k],  g_logic_keyboard.keyboardPipe[1]);
             }
             else
             {
                  if(g_logic_keyboard.keyboardfd[k] > 0 )
                  {
                       DBG_FORCE_PRINT("[directFB:input]------>Detect  keyboard device:%s  disconnected(%d)\n", g_logic_keyboard.device_name[k], __LINE__);
                        linux_detach_physicalDevice(&g_logic_keyboard.keyboardfd[k],  g_logic_keyboard.device_name[k]);
                        g_logic_keyboard.keyboardfd[k] = -1;
                  }
                  if(g_logic_keyboard.keyboardfd2[k] > 0 )
                  {
                       DBG_FORCE_PRINT("[directFB:input]------>Detect  keyboard device:%s  disconnected(%d)\n", g_logic_keyboard.device_name2[k], __LINE__);
                        linux_detach_physicalDevice(&g_logic_keyboard.keyboardfd2[k],  g_logic_keyboard.device_name2[k]);
                        g_logic_keyboard.keyboardfd2[k] = -1;
                  }
                  g_logic_keyboard.isPhysicalAttached[k] = false;
             }
         }
    }

    for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
    {
        if(g_logic_joystick.isPhysicalAttached[k])
        {
            if(linux_detect_device_connection(g_logic_joystick.joystickfd[k]))
            {
                linux_hotplug_transport(&g_logic_joystick.joystickfd[k], g_logic_joystick.device_name[k],  g_logic_joystick.joystickPipe[1]);
            }
            else
            {
                if(g_logic_joystick.joystickfd[k] > 0 )
                {
                    DBG_FORCE_PRINT("[directFB:input]------>Detect  joystick device:%s  disconnected(%d)\n", g_logic_joystick.device_name[k], __LINE__);
                    linux_detach_physicalDevice(&g_logic_joystick.joystickfd[k],  g_logic_joystick.device_name[k]);
                    g_logic_joystick.joystickfd[k] = -1;
                }
                if(g_logic_joystick.joystickfd2[k] > 0 )
                {
                    DBG_FORCE_PRINT("[directFB:input]------>Detect  joystick device:%s  disconnected(%d)\n", g_logic_joystick.device_name2[k], __LINE__);
                    linux_detach_physicalDevice(&g_logic_joystick.joystickfd2[k],  g_logic_joystick.device_name2[k]);
                    g_logic_joystick.joystickfd2[k] = -1;
                }
                g_logic_joystick.isPhysicalAttached[k] = false;
            }
        }
    }

    for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
    {
        if(g_logic_usbir.isPhysicalAttached[k])
        {
            if(linux_detect_device_connection(g_logic_usbir.usbirfd[k]))
            {
                linux_hotplug_transport(&g_logic_usbir.usbirfd[k], g_logic_usbir.device_name[k],  g_logic_usbir.usbirPipe[1]);
            }
            else
            {
                if(g_logic_usbir.usbirfd[k] > 0 )
                {
                    DBG_FORCE_PRINT("[directFB:input]------>Detect  usbir device:%s  disconnected(%d)\n", g_logic_usbir.device_name[k], __LINE__);
                    linux_detach_physicalDevice(&g_logic_usbir.usbirfd[k],  g_logic_usbir.device_name[k]);
                    g_logic_usbir.usbirfd[k] = -1;
                }
                if(g_logic_usbir.usbirfd2[k] > 0 )
                {
                    DBG_FORCE_PRINT("[directFB:input]------>Detect  usbir device:%s  disconnected(%d)\n", g_logic_usbir.device_name2[k], __LINE__);
                    linux_detach_physicalDevice(&g_logic_usbir.usbirfd2[k],  g_logic_usbir.device_name2[k]);
                    g_logic_usbir.usbirfd2[k] = -1;
                }
                g_logic_usbir.isPhysicalAttached[k] = false;
            }
        }
    }


    bool shouldSleep = true;
    for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
    {
         if(g_logic_mouse.isPhysicalAttached[k] || g_logic_keyboard.isPhysicalAttached[k] || g_logic_joystick.isPhysicalAttached[k]|| g_logic_usbir.isPhysicalAttached[k])
         {
              shouldSleep = false;
              break;
         }
    }
    if(shouldSleep)
   {
        usleep(SLEEP_TIME);
    }

    if(linux_DiffTimeFromNow(lasttime) < 1000)
        return -1;
    lasttime = linux_GetSystemTime();

     int   i;
     char *tsdev = NULL;
     bool opened = false;

     static u32 keyboardLastAttachedTime = 0;



    pthread_mutex_lock( &core_dfb_lock );
    tsdev = getenv( "TSLIB_TSDEVICE" );
    if(tsdev == NULL)
        D_ERROR("[DFB] %s, %d : getenv TSLIB_TSDEVICE failed\n",__FUNCTION__,__LINE__);
    pthread_mutex_unlock( &core_dfb_lock );

     /* No devices specified. Try to guess some. */
     for (i=0; i<MAX_LINUX_INPUT_DEVICES; i++)
    {
          char buf[32];

          int ret = snprintf( buf, sizeof(buf), "/dev/input/event%d", i );
          if(ret < 0)
              D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);

          /* Let tslib driver handle its device. */
          if (tsdev && !strcmp( tsdev, buf ))
               continue;
          opened = false;
          int j = 0;
          for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
          {
               if(device_names[j]  != NULL)
               {
                   if(strcmp(device_names[j], buf) == 0)//opened device
                   {
                        opened = true;
                   }
               }
          }
          if(opened)
          {
              continue;
          }
           #if 0
            if(linux_DiffTimeFromNow(debugtime) > 2000)
            {
                printf("hotplug polling  check  device......\n");
                 debugtime = linux_GetSystemTime();
            }
            #endif

          if (check_device( buf ))
          {
               //detect new device
               THREAD_DBG_INFO("[directFB:input]-----Detect new device:%s\n", buf);
               int j =0;
               for(j = 0; j < MAX_LINUX_INPUT_DEVICES; j++)
               {
                    if(device_names[j] == NULL)
                    {
                        char * toAttacheDev= D_STRDUP( buf );

                        static int lastAttachedDeviceposition = -1;

                        int devicetype = -1;
                        int deviceposition = -1;
                       bool isAttachedSuccess = linux_attache_physicalDevice(core_input,  toAttacheDev, &devicetype, &deviceposition);
                       if(isAttachedSuccess)
                       {
                             device_names[j] = toAttacheDev;
                             if(devicetype == (int)DIDTF_MOUSE)
                             {


                                 lastAttachedDeviceposition = deviceposition;
                             }
                             if(devicetype == (int)DIDTF_KEYBOARD)
                             {
                                 keyboardLastAttachedTime = linux_GetSystemTime();

                                 lastAttachedDeviceposition = deviceposition;
                             }
                             if(devicetype == (int)DIDTF_JOYSTICK)
                             {


                                 lastAttachedDeviceposition = deviceposition;
                             }
                             if(devicetype == (int)DIDTF_REMOTE)
                             {


                                 lastAttachedDeviceposition = deviceposition;
                             }
                             DBG_FORCE_PRINT("[directFB:input]-----Attache Physical device:%s  SUCCESS!\n", toAttacheDev);
                             return  j;
                       }
                       else
                       {
                              DBG_FORCE_PRINT("[directFB:input]-----Attache Physical device:%s  FAILED!  %d\n", toAttacheDev, linux_DiffTimeFromNow(keyboardLastAttachedTime));
                              /*
                              if(linux_DiffTimeFromNow(lastAttachedTime) < 1500)  --> This is a patch for the following reasons:
                              There will be two devices file can be used to open the keyboard, while the second device file can not be read correctly,
                              therefore, the time interval is used to identify the device file is from the same usb keyboard,
                              so pull out the keyboard which can be found when a physical device  be removed
                              */
                              if(linux_DiffTimeFromNow(keyboardLastAttachedTime) < 1500)
                              {
                                   THREAD_DBG_INFO("[directFB:input]-----keyboard hotplug Try temporary solution...\n");
                                    int fd = open( toAttacheDev, O_RDWR );
                                    if(fd > 0)
                                    {
                                        THREAD_DBG_INFO("[directFB:input]-----keyboard hotplug Try temporary solution success! %d\n", lastAttachedDeviceposition);
                                        if(lastAttachedDeviceposition > -1 && lastAttachedDeviceposition < MAX_LINUX_INPUT_DEVICES)
                                        {
                                             g_logic_keyboard.keyboardfd2[lastAttachedDeviceposition] = fd;
                                             strncpy(g_logic_keyboard.device_name2[lastAttachedDeviceposition], toAttacheDev, DEVICE_NAME_MAX_LENGTH);
                                             g_logic_keyboard.device_name2[lastAttachedDeviceposition][DEVICE_NAME_MAX_LENGTH - 1] =0;
                                             device_names[j] = toAttacheDev;
                                        }
                                        else
                                        {
                                             DBG_FORCE_PRINT("[directFB:input]---keyboard hotplug Try temporary solution fail! %d\n", lastAttachedDeviceposition );
                                        }
                                    }
                                    else
                                    {
                                        D_FREE(toAttacheDev);
                                    }
                              }
                              else
                              {
                                    D_FREE(toAttacheDev);
                              }
                              break;
                             //return -1;
                       }

                    }

               }
               THREAD_DBG_INFO("[directFB:input]-----No  device slot for use!!!!\n");

          }

     }

     return -1;
}

static void*
linux_input_hotplug_thread(DirectThread *thread, void *arg)
{
    while(!isHotplugExit)
    {
        direct_thread_testcancel(thread);
        driver_hotplug_polling2(arg);
        usleep(50*1000);
    }
    printf("[DFB][Input]---------------->hotplug thread  exit\n");
    return NULL;
}

#if defined(DISABLE_INPUT_HOTPLUG_FUNCTION_STUB)
static DFBResult
is_created( int event_num, void *data)
{
     D_UNUSED_P( event_num );
     D_UNUSED_P( data );

     return DFB_UNSUPPORTED;
}

static InputDriverCapability
get_capability( void )
{
     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

     InputDriverCapability   capabilities = IDC_NONE;

     capabilities |= IDC_HOTPLUG;

     return capabilities;
}

/*
 * Stop hotplug detection thread.
 */
static DFBResult
stop_hotplug( void )
{
     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

     /* Exit immediately if the hotplug thread is not created successfully in
      * launch_hotplug().
      */
     if ((!hotplug_thread) || (!hotplug_thread_polling))
          goto exit;

     /* Write to the hotplug quit pipe to cause the thread to terminate */
     CHECK_SYSCALL_ERROR(write( hotplug_quitpipe[WRITE_PIPE], " ", 1 ));

     /* Shutdown the hotplug detection thread. */
     direct_thread_join(hotplug_thread);
     direct_thread_destroy(hotplug_thread);
     direct_thread_join(hotplug_thread_polling);
     direct_thread_destroy(hotplug_thread_polling);
     close( hotplug_quitpipe[READ_PIPE] );
     close( hotplug_quitpipe[WRITE_PIPE] );

     hotplug_thread = NULL;
     hotplug_thread_polling = NULL;

     /* Destroy the suspended mutex. */
     pthread_mutex_destroy(&driver_suspended_lock);

     /* shutdown the connection of the socket */
     if (socket_fd > 0) {
          int rt = shutdown(socket_fd, SHUT_RDWR);
          if (rt < 0) {
               D_PERROR( "DirectFB/linux_input: Socket shutdown failed: %s\n",
                         strerror(errno) );
               return DFB_FAILURE;
          }
     }
     if (socket_fd > 0) {
          close(socket_fd);
          socket_fd = 0;
     }

exit:
     D_DEBUG_AT( Debug_LinuxInput, "%s() closed\n", __FUNCTION__ );
     return DFB_OK;
}

/*
 * Launch hotplug detection thread.
 */
static DFBResult
launch_hotplug(CoreDFB         *core,
               void            *input_driver)
{
     int ret;

     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

     DFBResult           result;

     D_ASSERT( core != NULL );
     D_ASSERT( input_driver != NULL );
     D_ASSERT( hotplug_thread == NULL );
     D_ASSERT( hotplug_thread_polling == NULL );

     /* open a pipe to awake the reader thread when we want to quit */
     ret = pipe( hotplug_quitpipe );
     if (ret < 0) {
          D_PERROR( "DirectFB/linux_input: could not open quitpipe for hotplug" );
          result = DFB_INIT;
          goto errorExit;
     }
     socket_fd = 0;


     /* Initialize a mutex used to communicate to the hotplug handling thread
      * when the driver is suspended.
      */
     pthread_mutex_init(&driver_suspended_lock, NULL);

     /* Create a thread to handle hotplug events. */
     D_INFO("[DFB][Input]---------------->start thread hotplug polling..\n");
     if (dfb_config->mst_new_mstar_linux_input) {
         hotplug_thread = direct_thread_create( DTT_DEFAULT, linux_input_hotplug_thread_new, NULL, "Linux hotplug" );
         hotplug_thread_polling = direct_thread_create( DTT_DEFAULT, linux_input_hotplug_thread_polling, NULL, "Linux polling" );
     } else
         hotplug_thread = direct_thread_create( DTT_DEFAULT, linux_input_hotplug_thread, NULL, "Linux hotplug" );
     D_INFO("[DFB][Input]----------------->start thread hotplug polling end..\n");

     if ((!hotplug_thread) || (!hotplug_thread_polling)) {
          pthread_mutex_destroy(&driver_suspended_lock);

          result = DFB_UNSUPPORTED;
     }
     else
          result = DFB_OK;

errorExit:
     return result;
}
#endif

static DFBResult  linux_open_logic_mouse(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data)
{

    if(strcmp(logic_device_names[number], DEV_LOGIC_MOUSE) != 0 ||  number != LOGIC_MOUSE || info == NULL || device == NULL)
    {
        return DFB_INIT;
    }

    LinuxInputData* data = D_CALLOC( 1, sizeof(LinuxInputData) );
    if (!data) {
        return D_OOM();
    }
    data->sensitive_numerator = 1;
    data->sensitive_denominator = 1;
    data->rel_axis_x_motion_reminder = 0;
    data->rel_axis_y_motion_reminder = 0;

    data->fd     = g_logic_mouse.mousePipe[READ_PIPE];
    data->device = device;
    data->touchpad = 0;
    data->vt_fd    = -1;
    data->pDeviceName = logic_device_names[number];
    data->index = number;

    info->desc.min_keycode = -1;
    info->desc.max_keycode = -1;
    info->desc.type |= DIDTF_MOUSE;
    info->desc.caps       |= DICAPS_BUTTONS;
    info->desc.caps       |= DICAPS_AXES;
    info->prefered_id = DIDID_MOUSE;
    strncpy(info->desc.name, logic_device_names[number], DFB_INPUT_DEVICE_DESC_NAME_LENGTH);
    info->desc.name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1] = 0;

    /* start input thread */
    DBG_INFO("========================start input thread  for logic mouse============================\n");
    data->thread = direct_thread_create( DTT_INPUT, linux_input_EventThread, data, "Linux Input Mouse" );
    /* set private data pointer */
    *driver_data = data;

    return DFB_OK;
}

static DFBResult  linux_open_logic_keyboard(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data)
{

    if(strcmp(logic_device_names[number], DEV_LOGIC_KEYBOARD) != 0 || number != LOGIC_KEYBOARD || info == NULL || device == NULL)
    {
        return DFB_INIT;
    }

    LinuxInputData* data = D_CALLOC( 1, sizeof(LinuxInputData) );
    if (!data) {
        return D_OOM();
    }
    data->sensitive_numerator = 1;
    data->sensitive_denominator = 1;
    data->rel_axis_x_motion_reminder = 0;
    data->rel_axis_y_motion_reminder = 0;

    data->fd     = g_logic_keyboard.keyboardPipe[READ_PIPE];
    data->device = device;
    data->touchpad = 0;
    data->vt_fd    = -1;
    data->pDeviceName = logic_device_names[number];
    data->index = number;

    info->desc.type |= DIDTF_KEYBOARD;
    info->desc.caps |= DICAPS_KEYS;

    info->desc.min_keycode = 0;
    //info->desc.max_keycode = 127;
    info->desc.max_keycode = 0x250;

    info->desc.caps       |= DICAPS_BUTTONS;
    info->desc.caps       |= DICAPS_AXES;
    info->prefered_id = DIDID_KEYBOARD;
    strncpy(info->desc.name, logic_device_names[number], DFB_INPUT_DEVICE_DESC_NAME_LENGTH);
    info->desc.name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1] = 0;

    /* start input thread */
    DBG_INFO("========================start input thread  for logic keyboard============================\n");
    data->thread = direct_thread_create( DTT_INPUT, linux_input_EventThread, data, "Linux Input Keyboard" );
    /* set private data pointer */
    *driver_data = data;

    return DFB_OK;
}

static DFBResult  linux_open_logic_usbir(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data)
{

    if(strcmp(logic_device_names[number], DEV_LOGIC_USBIR) != 0 || number != LOGIC_USBIR || info == NULL || device == NULL)
    {
        return DFB_INIT;
    }

    LinuxInputData* data = D_CALLOC( 1, sizeof(LinuxInputData) );

    if (!data) {
        return D_OOM();
    }

    data->sensitive_numerator = 1;
    data->sensitive_denominator = 1;
    data->rel_axis_x_motion_reminder = 0;
    data->rel_axis_y_motion_reminder = 0;

    data->fd     = g_logic_usbir.usbirPipe[READ_PIPE];
    data->device = device;
    data->touchpad = 0;
    data->vt_fd    = -1;
    data->pDeviceName = logic_device_names[number];
    data->index = number;

    info->desc.type |= DIDTF_REMOTE;
    info->desc.caps |= DICAPS_KEYS;

    info->desc.min_keycode = 0;
    //info->desc.max_keycode = 127;
    info->desc.max_keycode = dfb_config->mst_ir_max_keycode;

    info->desc.caps       |= DICAPS_BUTTONS;
    info->desc.caps       |= DICAPS_AXES;
    info->prefered_id = DIDID_REMOTE ;
    strncpy(info->desc.name, logic_device_names[number], DFB_INPUT_DEVICE_DESC_NAME_LENGTH);
    info->desc.name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1] = 0;

    /* start input thread */
    DBG_INFO("========================start input thread  for logic usbir============================\n");
    data->thread = direct_thread_create( DTT_INPUT, linux_input_EventThread, data, "Linux Input usbir" );
    /* set private data pointer */
    *driver_data = data;

    return DFB_OK;
}



static DFBResult  linux_open_logic_joystick(CoreInputDevice  *device, unsigned int   number,InputDeviceInfo  *info, void  **driver_data)
{

    if(strcmp(logic_device_names[number], DEV_LOGIC_JOYSTICK) != 0 || number != LOGIC_JOYSTICK || info == NULL || device == NULL)
    {
        return DFB_INIT;
    }

    LinuxInputData* data = D_CALLOC( 1, sizeof(LinuxInputData) );
    if (!data) {
        return D_OOM();
    }

    data->sensitive_numerator = 1;
    data->sensitive_denominator = 1;
    data->rel_axis_x_motion_reminder = 0;
    data->rel_axis_y_motion_reminder = 0;

    data->fd     = g_logic_joystick.joystickPipe[READ_PIPE];
    data->device = device;
    data->touchpad = 0;
    data->vt_fd    = -1;
    data->pDeviceName = logic_device_names[number];
    data->index = number;

    info->desc.type |= DIDTF_JOYSTICK;
    info->desc.caps |= DICAPS_BUTTONS;
    info->desc.caps |= DICAPS_AXES;
    info->prefered_id = DIDID_JOYSTICK;
    strncpy(info->desc.name, logic_device_names[number], DFB_INPUT_DEVICE_DESC_NAME_LENGTH);
    info->desc.name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1] = 0;

    /* start input thread */
    DBG_INFO("========================start input thread  for logic joystick============================\n");
    data->thread = direct_thread_create( DTT_INPUT, linux_input_EventThread, data, "Linux Input Joystick" );
    /* set private data pointer */
    *driver_data = data;

    return DFB_OK;
}

static void  linux_close_logic_mouse(void)
{
    int log_idx, dev_idx;

    DBG_INFO("%s\n", __FUNCTION__);
    for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
    {
        if ( device_type[dev_idx] == (int)DIDTF_MOUSE )
        {
            log_idx = device_position[dev_idx];
            if ( g_logic_mouse.mousefd[log_idx] > 0 )
            {
                direct_thread_cancel( device_polling_thread[dev_idx] );
                CHECK_SYSCALL_ERROR(write( device_polling_quitpipe[dev_idx][WRITE_PIPE], " ", 1 ));
                direct_thread_join( device_polling_thread[dev_idx] );
                direct_thread_destroy( device_polling_thread[dev_idx] );
                device_polling_thread[dev_idx] = NULL;
                close( device_polling_quitpipe[dev_idx][READ_PIPE] );
                close( device_polling_quitpipe[dev_idx][WRITE_PIPE] );

                close( g_logic_mouse.mousefd[log_idx] );
            }
        }
    }

    if(g_logic_mouse.mousePipe[READ_PIPE] >= 0)
        close(g_logic_mouse.mousePipe[READ_PIPE]);
    if(g_logic_mouse.mousePipe[WRITE_PIPE] >= 0)
        close(g_logic_mouse.mousePipe[WRITE_PIPE]);

    return;
}

static void  linux_close_logic_keyboard(void)
{
    int log_idx, dev_idx;

    DBG_INFO("%s\n", __FUNCTION__);
    for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
    {
        if ( device_type[dev_idx] == (int)DIDTF_KEYBOARD )
        {
            log_idx = device_position[dev_idx];
            if ( g_logic_keyboard.keyboardfd[log_idx] > 0 )
            {
                direct_thread_cancel( device_polling_thread[dev_idx] );
                CHECK_SYSCALL_ERROR(write( device_polling_quitpipe[dev_idx][WRITE_PIPE], " ", 1 ));
                direct_thread_join( device_polling_thread[dev_idx] );
                direct_thread_destroy( device_polling_thread[dev_idx] );
                device_polling_thread[dev_idx] = NULL;
                close( device_polling_quitpipe[dev_idx][READ_PIPE] );
                close( device_polling_quitpipe[dev_idx][WRITE_PIPE] );

                close(g_logic_keyboard.keyboardfd[log_idx]);
            }

            if ( g_logic_keyboard.keyboardfd2[log_idx] > 0 )
                close(g_logic_keyboard.keyboardfd2[log_idx]);
        }
    }
    if(g_logic_keyboard.keyboardPipe[READ_PIPE] >= 0)
        close(g_logic_keyboard.keyboardPipe[READ_PIPE]);
    if(g_logic_keyboard.keyboardPipe[WRITE_PIPE] >= 0)
        close(g_logic_keyboard.keyboardPipe[WRITE_PIPE]);

    return;
}

static void  linux_close_logic_joystick(void)
{
    int log_idx, dev_idx;
    DBG_INFO("%s\n", __FUNCTION__);
    for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
    {
        if ( device_type[dev_idx] == (int)DIDTF_JOYSTICK)
        {
            log_idx = device_position[dev_idx];
            if ( g_logic_joystick.joystickfd[log_idx] > 0 )
            {
                direct_thread_cancel( device_polling_thread[dev_idx] );
                CHECK_SYSCALL_ERROR(write( device_polling_quitpipe[dev_idx][WRITE_PIPE], " ", 1 ));
                direct_thread_join( device_polling_thread[dev_idx] );
                direct_thread_destroy( device_polling_thread[dev_idx] );
                device_polling_thread[dev_idx] = NULL;
                close( device_polling_quitpipe[dev_idx][READ_PIPE] );
                close( device_polling_quitpipe[dev_idx][WRITE_PIPE] );

                close(g_logic_joystick.joystickfd[log_idx]);
            }

            if ( g_logic_joystick.joystickfd2[log_idx] > 0 )
                close(g_logic_joystick.joystickfd2[log_idx]);
        }
    }
    if(g_logic_joystick.joystickPipe[READ_PIPE] >= 0)
        close(g_logic_joystick.joystickPipe[READ_PIPE]);
    if(g_logic_joystick.joystickPipe[WRITE_PIPE] >= 0)
        close(g_logic_joystick.joystickPipe[WRITE_PIPE]);

    return;
}

static void  linux_close_logic_usbir(void)
{
    int log_idx, dev_idx;

    DBG_INFO("%s\n", __FUNCTION__);
    for(dev_idx = 0; dev_idx < MAX_LINUX_INPUT_DEVICES; dev_idx++)
    {
        if ( device_type[dev_idx] == (int)DIDTF_REMOTE )
        {
            log_idx = device_position[dev_idx];
            if ( g_logic_usbir.usbirfd[log_idx] > 0 )
            {
                direct_thread_cancel( device_polling_thread[dev_idx] );
                CHECK_SYSCALL_ERROR(write( device_polling_quitpipe[dev_idx][WRITE_PIPE], " ", 1 ));
                direct_thread_join( device_polling_thread[dev_idx] );
                direct_thread_destroy( device_polling_thread[dev_idx] );
                device_polling_thread[dev_idx] = NULL;
                close( device_polling_quitpipe[dev_idx][READ_PIPE] );
                close( device_polling_quitpipe[dev_idx][WRITE_PIPE] );

                close(g_logic_usbir.usbirfd[log_idx]);
            }

            if ( g_logic_usbir.usbirfd2[log_idx] > 0 )
                close(g_logic_usbir.usbirfd2[log_idx]);
        }
    }
    if(g_logic_usbir.usbirPipe[READ_PIPE] >= 0)
        close(g_logic_usbir.usbirPipe[READ_PIPE]);
    if(g_logic_usbir.usbirPipe[WRITE_PIPE] >= 0)
        close(g_logic_usbir.usbirPipe[WRITE_PIPE]);

    return;
}


static DFBResult update_keymap_repository (char *filename, char *name)
{
    if (filename == NULL && name ==NULL)
        return DFB_FAILURE;

    pthread_mutex_lock(&keymap_repository_lock);

    /* find the keymap is already register or not */
    int kemap_index =  find_register_physical_device_keymap(name);

    if (filename != NULL && kemap_index == INVALID_DEVICE_INDEX && keymap_repository.count >= 0 && keymap_repository.count < MAX_LINUX_INPUT_DEVICES)
    {
        /* this input device has not loaded yet */
        D_ASSERT(keymap_repository.count >= 0);
        keymap_repository.filename[keymap_repository.count] = D_STRDUP(filename);

        strncpy(keymap_repository.physical_device_name[keymap_repository.count], name, DEVICE_NAME_MAX_LENGTH);
        keymap_repository.physical_device_name[keymap_repository.count][DEVICE_NAME_MAX_LENGTH - 1] = 0;

        DBG_INPUT_MSG("[DFB] keymap is new. device name = %s, filename = %s, count = %d\n",
            keymap_repository.physical_device_name[keymap_repository.count], keymap_repository.filename[keymap_repository.count], keymap_repository.count );
        keymap_repository.count++;
    }
    else if(filename != NULL && kemap_index >= 0 && kemap_index < MAX_LINUX_INPUT_DEVICES)
    {
         /* this input device has loaded before */
         if (keymap_repository.filename[kemap_index])
            D_FREE(keymap_repository.filename[kemap_index]);
        keymap_repository.filename[kemap_index] = D_STRDUP(filename);

        DBG_INPUT_MSG("[DFB] keymap is found. device name = %s, filename = %s, count = %d\n",
            keymap_repository.physical_device_name[kemap_index], keymap_repository.filename[kemap_index], keymap_repository.count );
    }

    pthread_mutex_unlock(&keymap_repository_lock);
    return DFB_OK;
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
    int i = 0;
    for( i = 0; i < MAX_LINUX_INPUT_DEVICES; i++)
    {
        if (dfb_config->mst_inputdevice[i].filename && dfb_config->mst_inputdevice[i].name)
            update_keymap_repository ( dfb_config->mst_inputdevice[i].filename, dfb_config->mst_inputdevice[i].name);
        else
            break;
    }

    if ( gLogicDeviceFuncs[number].OpenLogicDevice )
        return (gLogicDeviceFuncs[number].OpenLogicDevice)(device, number, info, driver_data);


     return DFB_INIT;
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
     LinuxInputData *data = (LinuxInputData*) driver_data;

     if (data->touchpad)
          return DFB_OK;

     if (axis <= ABS_PRESSURE && axis < DIAI_LAST) {
          unsigned long absbit[NBITS(ABS_CNT)];

          /* check if we have an absolute axes */
          CHECK_SYSCALL_ERROR(ioctl( data->fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit ));

          if (test_bit (axis, absbit)) {
               struct input_absinfo absinfo;

               int retval = ioctl( data->fd, EVIOCGABS(axis), &absinfo );

               if (retval == 0 &&
                   (absinfo.minimum || absinfo.maximum)) {
                    ret_info->flags  |= DIAIF_ABS_MIN | DIAIF_ABS_MAX;
                    ret_info->abs_min = absinfo.minimum;
                    ret_info->abs_max = absinfo.maximum;
               }
               else if(retval == -1)
                    CHECK_SYSCALL_ERROR(ioctl( data->fd, EVIOCGABS(axis), &absinfo ));
          }
     }

     return DFB_OK;
}


static int
get_physical_device_position(CoreInputDevice *device, char *name)
{
     if ( (!device) || (!name) )
        return INVALID_DEVICE_INDEX;

     int idx=0;
     switch(device->shared->device_info.prefered_id )
     {
        case DIDID_KEYBOARD:
            for (idx=0; idx < MAX_LINUX_INPUT_DEVICES; idx++)
            {
                if (strcmp(g_logic_keyboard.physical_device_name[idx], name) == 0)
                    return idx;
            }
            return INVALID_DEVICE_INDEX;
        case DIDID_REMOTE:
            for (idx=0; idx < MAX_LINUX_INPUT_DEVICES; idx++)
            {
                if (strcmp(g_logic_usbir.physical_device_name[idx], name) == 0)
                    return idx;
            }
            return INVALID_DEVICE_INDEX;
        default:
            return INVALID_DEVICE_INDEX;
     }

}
static int
find_register_physical_device_keymap(char *name)
{
    int index , count = 0;

    if (name == NULL)
        return INVALID_DEVICE_INDEX;


    for (index = 0; index < MAX_LINUX_INPUT_DEVICES; index++)
    {
        if (count > keymap_repository.count)
            return INVALID_DEVICE_INDEX;



        char *physical_device_name_temp = keymap_repository.physical_device_name[index];
        if ((physical_device_name_temp != NULL) && count < keymap_repository.count)
        {
            if (strcmp( name, keymap_repository.physical_device_name[index])== 0) {
                physical_device_name_temp = NULL;
                return index;
            }else
                count ++;
        }

        physical_device_name_temp = NULL;
    }
    return INVALID_DEVICE_INDEX;
}

static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                      void                         *driver_data,
                     InputDeviceIoctlData *param)
{
     LinuxInputData *data = (LinuxInputData*) driver_data;

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     switch(param->request)
     {
       case DFB_DEV_IOC_SET_REL_AXIS_SENSE_NUMERATOR:
          if(((int)param->param[0])<=0)
              return DFB_INVARG;
          data->sensitive_numerator = ((int)param->param[0]);
          data->rel_axis_x_motion_reminder = data->rel_axis_y_motion_reminder = 0;
          break;
       case DFB_DEV_IOC_SET_REL_AXIS_SENSE_DENOMINATOR:
          if(((int)param->param[0])<=0)
              return DFB_INVARG;
          data->sensitive_denominator = ((int)param->param[0]);
          data->rel_axis_x_motion_reminder = data->rel_axis_y_motion_reminder = 0;
          break;
       case DFB_DEV_IOC_GET_PHY_KEYBOARD_CNT:
           if(g_logic_keyboard.isPhysicalAttached)
           {
                memset(param->param, 0, sizeof(param->param));
                param->param[0] = 1;
           }
            else
            {
                memset(param->param, 0, sizeof(param->param));
                param->param[0] = 0;
           }
            break;
        case DFB_DEV_IOC_GET_PHY_MOUSE_CNT:
        {
              bool mouseDeviceIsAttached = false;
                int k = 0;
               for(k = 0; k < MAX_LINUX_INPUT_DEVICES; k++)
               {
                    if(g_logic_mouse.isPhysicalAttached[k])
                    {
                        mouseDeviceIsAttached = true;
                    }
               }
               if(mouseDeviceIsAttached)
               {
                    memset(param->param, 0, sizeof(param->param));
                    param->param[0] = 1;
               }
                else
                {
                    memset(param->param, 0, sizeof(param->param));
                    param->param[0] = 0;
               }
        }
            break;
        case DFB_DEV_IOC_SET_SUB_DEVICE_KEYMAP:
        {

               char *filename = (char *)param->param_ptr[0];
               char *device_name  = (char *)param->param_ptr[1];
               if (filename == NULL || device_name == NULL)
                    return DFB_INVARG;

               DBG_INPUT_MSG("[DFB] %s, Get data =>File path is: %s, device name is : %s\n", __FUNCTION__, filename, device_name);

               /* check the file is valid */
               if (access( filename, F_OK ) != 0)
               {
                    DBG_INPUT_MSG("[DFB] keymap is invalid ,errno = %d\n", errno);
                    return errno2result( errno );
               }

               update_keymap_repository ( filename, device_name);

               /* Load/reload the predefined keymap of customer if the device was existed */
               int ret_device_position = INVALID_DEVICE_INDEX;
               DFBResult ret;
               ret_device_position = get_physical_device_position(device, device_name);

               if (ret_device_position  != INVALID_DEVICE_INDEX)
               {

                    switch(device->shared->device_info.prefered_id )
                    {
                        case DIDID_KEYBOARD:
                            pthread_mutex_lock(&keyboard_driver_lock);
                            ret = dfb_input_load_physical_device_keymap(device, filename, ret_device_position);
                            pthread_mutex_unlock(&keyboard_driver_lock);
                            if (ret)
                                DBG_INPUT_MSG("[DFB] load keymap : %s to keyboard device: %s  has error. ret = %d\n", filename, device_name, ret);
                            break;
                        case DIDID_REMOTE:
                            pthread_mutex_lock(&usbir_driver_lock);
                            ret = dfb_input_load_physical_device_keymap(device, filename, ret_device_position);
                            pthread_mutex_unlock(&usbir_driver_lock);
                            if (ret)
                                DBG_INPUT_MSG("[DFB] load keymap : %s to usb-ir device: %s  has error. ret = %d\n", filename, device_name, ret);
                            break;
                        default:
                            return DFB_UNSUPPORTED;
                    }
               }
        }
            break;
        case DFB_DEV_IOC_SET_LINUX_INPUT_REPEAT_TIME:
        {
               int ret = 0, err = 0, fd = 0;
               int repeat_time[2];

               if (param->param[1] <=0  || param->param[2] <=0)
                    return DFB_INVARG;

               /* device name*/
               char *dev_name = (char*)param->param_ptr[0];

               /* first repeat time */
               repeat_time[0] = param->param[1];

               /* repeat time */
               repeat_time[1] = param->param[2];

               int ret_device_position = INVALID_DEVICE_INDEX;
               ret_device_position = get_physical_device_position(device, dev_name);

               if (ret_device_position != INVALID_DEVICE_INDEX)
               {
                    switch(device->shared->id )
                    {
                        case DIDID_KEYBOARD:
                            pthread_mutex_lock(&keyboard_driver_lock);
                            fd = g_logic_keyboard.keyboardfd[ret_device_position];
                            DBG_INPUT_MSG("[DFB]device name[%d] =%s, first repeat time=%d, repeat time=%d \n",
                                ret_device_position,g_logic_keyboard.physical_device_name[ret_device_position], repeat_time[0], repeat_time[1]);
                            pthread_mutex_unlock(&keyboard_driver_lock);
                            break;
                        case DIDID_REMOTE:
                            pthread_mutex_lock(&usbir_driver_lock);
                            fd =  g_logic_usbir.usbirfd[ret_device_position];
                            DBG_INPUT_MSG("[DFB] device name[%d] =%s, first repeat time=%d, repeat time=%d \n",
                                ret_device_position,g_logic_usbir.physical_device_name[ret_device_position], repeat_time[0], repeat_time[1]);
                            pthread_mutex_unlock(&usbir_driver_lock);
                            break;
                        default:
                            return DFB_UNSUPPORTED;
                    }
               }
               else
               {
                    DBG_INPUT_MSG("[DFB] can not find the device name %s\n", dev_name);
                    return DFB_INVARG;
               }

               ret = ioctl( fd , EVIOCSREP, repeat_time);
               if (-1 == ret)
               {
                    err = errno;
                    DBG_INPUT_MSG("[DFB] set linux input device repeat time fail, err=%d \n",err);
                    return DFB_FAILURE;
               }
               DBG_INPUT_MSG("[DFB] device name =%s, first repeat time=%d, repeat time=%d set OK\n", dev_name, param->param[1], param->param[2]);
        }
            break;
        default:
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
    // LinuxInputData             *data = (LinuxInputData*) driver_data;
     int                         code = entry->code;
     unsigned short              value;
     DFBInputDeviceKeyIdentifier identifier;

    // if (data->vt_fd < 0)
     //     return DFB_UNSUPPORTED;

     /* fetch the base level */
     value = code < NR_KEYS ? keyboard_read_value( driver_data, K_NORMTAB, code ) : K_NOSUCHMAP;

     /* get the identifier for basic mapping */
     identifier = keyboard_get_identifier( code, value );

     /* is CapsLock effective? */
     if (KTYP(value) == KT_LETTER)
          entry->locks |= DILS_CAPS;

     /* is NumLock effective? */
     if (identifier >= DIKI_KP_DECIMAL && identifier <= DIKI_KP_9)
          entry->locks |= DILS_NUM;

     /* write identifier to entry */
     entry->identifier = identifier;

     /* write base level symbol to entry */
     entry->symbols[DIKSI_BASE] = keyboard_get_symbol( code, value, DIKSI_BASE );


     /* fetch the shifted base level */
     value = code < NR_KEYS ? keyboard_read_value( driver_data, K_SHIFTTAB, entry->code ) : K_NOSUCHMAP;

     /* write shifted base level symbol to entry */
     entry->symbols[DIKSI_BASE_SHIFT] = keyboard_get_symbol( code, value,
                                                             DIKSI_BASE_SHIFT );


     /* fetch the alternative level */
     value = code < NR_KEYS ? keyboard_read_value( driver_data, K_ALTTAB, entry->code ) : K_NOSUCHMAP;

     /* write alternative level symbol to entry */
     entry->symbols[DIKSI_ALT] = keyboard_get_symbol( code, value, DIKSI_ALT );


     /* fetch the shifted alternative level */
     value = code < NR_KEYS ? keyboard_read_value( driver_data, K_ALTSHIFTTAB, entry->code ) : K_NOSUCHMAP;

     /* write shifted alternative level symbol to entry */
     entry->symbols[DIKSI_ALT_SHIFT] = keyboard_get_symbol( code, value,
                                                            DIKSI_ALT_SHIFT );

     return DFB_OK;
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
     LinuxInputData *data = (LinuxInputData*) driver_data;
     int idx = 0;

     D_DEBUG_AT( Debug_LinuxInput, "%s()\n", __FUNCTION__ );

    if ( dfb_config->mst_new_mstar_linux_input )
    {

        if((data->index > 0) && (gLogicDeviceFuncs[data->index].CloseLogicDevice))
            (gLogicDeviceFuncs[data->index].CloseLogicDevice)();

    }

     /* stop input thread */
     direct_thread_cancel( data->thread );
     direct_thread_join( data->thread );
     direct_thread_destroy( data->thread );

     if (data->has_leds) {
          /* restore LED state */
          set_led( data, LED_SCROLLL, test_bit( LED_SCROLLL, data->led_state ) );
          set_led( data, LED_NUML, test_bit( LED_NUML, data->led_state ) );
          set_led( data, LED_CAPSL, test_bit( LED_CAPSL, data->led_state ) );
     }

     /* release device */
     if (dfb_config->linux_input_grab)
          ioctl( data->fd, EVIOCGRAB, 0 );

     if (data->vt_fd >= 0)
          close( data->vt_fd );

     /* close file */
     close( data->fd );

     /* free private data */
     D_FREE( data );
     for (idx = 0; idx< keymap_repository.count; idx++)
     {
          if (keymap_repository.filename[idx]) {
               D_FREE(keymap_repository.filename[idx]);
               keymap_repository.filename[idx] = NULL;
          }
     }

     keymap_repository.count = 0;

     D_DEBUG_AT( Debug_LinuxInput, "%s() closed\n", __FUNCTION__ );
}

static bool
timeout_is_set( const struct timeval *timeout )
{
     return timeout->tv_sec || timeout->tv_usec;
}

static bool
timeout_passed( const struct timeval *timeout, const struct timeval *current )
{
     return !timeout_is_set( timeout ) ||
          current->tv_sec > timeout->tv_sec ||
          (current->tv_sec == timeout->tv_sec && current->tv_usec > timeout->tv_usec);
}

static void
timeout_clear( struct timeval *timeout )
{
     timeout->tv_sec  = 0;
     timeout->tv_usec = 0;
}

static void
timeout_add( struct timeval *timeout, const struct timeval *add )
{
     timeout->tv_sec += add->tv_sec;
     timeout->tv_usec += add->tv_usec;
     while (timeout->tv_usec >= 1000000) {
          timeout->tv_sec++;
          timeout->tv_usec -= 1000000;
     }
}

static void
timeout_sub( struct timeval *timeout, const struct timeval *sub )
{
     timeout->tv_sec -= sub->tv_sec;
     timeout->tv_usec -= sub->tv_usec;
     while (timeout->tv_usec < 0) {
          timeout->tv_sec--;
          timeout->tv_usec += 1000000;
     }
}

static void
touchpad_fsm_init( struct touchpad_fsm_state *state )
{
     state->x.old = -1;
     state->y.old = -1;
     state->fsm_state = TOUCHPAD_FSM_START;
     timeout_clear( &state->timeout );
}

static int
touchpad_normalize( const struct touchpad_axis *axis, int value )
{
     return ((value - axis->min) << 9) / (axis->max - axis->min);
}

static int
touchpad_translate( struct touchpad_fsm_state *state,
                    const struct input_event  *levt,
                    DFBInputEvent             *devt )
{
     struct touchpad_axis *axis = NULL;
     int abs, rel;

     devt->flags     = DIEF_TIMESTAMP | DIEF_AXISREL;
     devt->timestamp = levt->time;
     devt->type      = DIET_AXISMOTION;

     switch (levt->code) {
     case ABS_X:
          axis       = &state->x;
          devt->axis = DIAI_X;
          break;
     case ABS_Y:
          axis       = &state->y;
          devt->axis = DIAI_Y;
          break;
     default:
          return 0;
     }

     abs = touchpad_normalize( axis, levt->value );
     if (axis->old == -1)
          axis->old = abs;
     rel = abs - axis->old;

#define ACCEL_THRESHOLD 25
#define ACCEL_NUM       3
#define ACCEL_DENOM     1

     if (rel > ACCEL_THRESHOLD)
          rel += (rel - ACCEL_THRESHOLD) * ACCEL_NUM / ACCEL_DENOM;
     else if (rel < -ACCEL_THRESHOLD)
          rel += (rel + ACCEL_THRESHOLD) * ACCEL_NUM / ACCEL_DENOM;

     axis->old     = abs;
     devt->axisrel = rel;

     return 1;
}

static bool
touchpad_finger_landing( const struct input_event *levt )
{
     return levt->type == EV_KEY && levt->code == BTN_TOUCH && levt->value == 1;
}

static bool
touchpad_finger_leaving( const struct input_event *levt )
{
     return levt->type == EV_KEY && levt->code == BTN_TOUCH && levt->value == 0;
}

static bool
touchpad_finger_moving( const struct input_event *levt )
{
     return levt->type == EV_ABS && (levt->code == ABS_X || levt->code == ABS_Y);
}

/*
 * This FSM takes into accout finger landing on touchpad and leaving and
 * translates absolute DFBInputEvent into a relative one
 */
static int
touchpad_fsm( struct touchpad_fsm_state *state,
              const struct input_event  *levt,
              DFBInputEvent             *devt )
{
     struct timeval timeout = { 0, 125000 };

     /* select() timeout? */
     if (!levt) {
          /* Check if button release is due. */
          if (state->fsm_state == TOUCHPAD_FSM_DRAG_START) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of current time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }

          /* Already passed, clear it so select() won't return until there is something to do. */
          timeout_clear( &state->timeout );
          return 0;
     }

     /* More or less ignore these events for now */
     if ((levt->type == EV_SYN && levt->code == SYN_REPORT) ||
         (levt->type == EV_ABS && levt->code == ABS_PRESSURE) ||
         (levt->type == EV_ABS && levt->code == ABS_TOOL_WIDTH) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_FINGER) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_DOUBLETAP) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_TRIPLETAP)) {

          /* Check if button release is due. */
          if (state->fsm_state == TOUCHPAD_FSM_DRAG_START &&
              timeout_passed( &state->timeout, &levt->time )) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of levt->time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }

          return 0;
     }

     /* Use translate_event() for other events. */
     if (!(levt->type == EV_KEY && levt->code == BTN_TOUCH) &&
         !(levt->type == EV_ABS && (levt->code == ABS_X || levt->code == ABS_Y)))
          return -1;

     switch (state->fsm_state) {
     case TOUCHPAD_FSM_START:
          if (touchpad_finger_landing( levt )) {
               state->fsm_state = TOUCHPAD_FSM_MAIN;
               state->timeout = levt->time;
               timeout_add( &state->timeout, &timeout );
          }
          return 0;

     case TOUCHPAD_FSM_MAIN:
          if (touchpad_finger_moving( levt )) {
               if (1){//timeout_passed( &state->timeout, &levt->time )) {
                    //timeout_clear( &state->timeout );
                    return touchpad_translate( state, levt, devt );
               }
          }
          else if (touchpad_finger_leaving( levt )) {
               if (!timeout_passed( &state->timeout, &levt->time )) {
                    devt->flags     = DIEF_TIMESTAMP;
                    devt->timestamp = levt->time;
                    devt->type      = DIET_BUTTONPRESS;
                    devt->button    = DIBI_FIRST;

                    touchpad_fsm_init( state );
                    state->fsm_state = TOUCHPAD_FSM_DRAG_START;
                    state->timeout = levt->time;
                    timeout_add( &state->timeout, &timeout );
                    return 1;
               }
               else {
                    touchpad_fsm_init( state );
               }
          }
          return 0;

     case TOUCHPAD_FSM_DRAG_START:
          if (timeout_passed( &state->timeout, &levt->time )){
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of levt->time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init(state);
               return 1;
          }
          else {
               if (touchpad_finger_landing( levt )) {
                    state->fsm_state = TOUCHPAD_FSM_DRAG_MAIN;
                    state->timeout = levt->time;
                    timeout_add( &state->timeout, &timeout );
               }
          }
          return 0;

     case TOUCHPAD_FSM_DRAG_MAIN:
          if (touchpad_finger_moving( levt )) {
               if (1){//timeout_passed( &state->timeout, &levt->time )) {
                   //timeout_clear( &state->timeout );
                    return touchpad_translate( state, levt, devt );
               }
          }
          else if (touchpad_finger_leaving( levt )) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = levt->time;
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }
          return 0;

     default:
          return 0;
     }

     return 0;
}
