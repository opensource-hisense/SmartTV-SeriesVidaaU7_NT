/*
   (c) Copyright 2001-2010  The world wide DirectFB Open Source Community (directfb.org)
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

#ifndef __INPUT_H__
#define __INPUT_H__

#include <pthread.h>
#include <directfb.h>

#include <direct/modules.h>

#include <fusion/reactor.h>

#include <core/coretypes.h>
#include <fusion/ref.h>



DECLARE_MODULE_DIRECTORY( dfb_input_modules );


/*
 * Increase this number when changes result in binary incompatibility!
 */
#define DFB_INPUT_DRIVER_ABI_VERSION         7

#define DFB_INPUT_DRIVER_INFO_NAME_LENGTH   48
#define DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH 64
#define INVALID_DEVICE_INDEX -1
#define MAX_LINUX_INPUT_DEVICES 16


typedef struct {
     int          major;              /* major version */
     int          minor;              /* minor version */
} InputDriverVersion;                 /* major.minor, e.g. 0.1 */

typedef struct {
     InputDriverVersion version;

     char               name[DFB_INPUT_DRIVER_INFO_NAME_LENGTH];
                                      /* Name of driver,
                                         e.g. 'Serial Mouse Driver' */

     char               vendor[DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH];
                                      /* Vendor (or author) of the driver,
                                         e.g. 'directfb.org' or 'Sven Neumann' */
} InputDriverInfo;

typedef struct {
     unsigned int       prefered_id;  /* Prefered predefined input device id,
                                         e.g. DIDID_MOUSE */

     DFBInputDeviceDescription desc;  /* Capabilities, type, etc. */
} InputDeviceInfo;

typedef struct {

    int index; /* the index of current linux input device */
    char *name[MAX_LINUX_INPUT_DEVICES]; /* the maximum number of device is 16 */
}InputSubDeviceInfos;

/*
 *  Input provider capability flags.
 */
typedef enum {
     IDC_NONE      = 0x00,            /* None */
     IDC_HOTPLUG   = 0x01,            /* Input devices support hot-plug */

     IDC_ALL       = 0x01             /* All flags supported */
} InputDriverCapability;

typedef struct {
     CoreDFB          *core;
     void             *driver;
} HotplugThreadData;

typedef struct {
     int       (*GetAvailable)   (void);
     void      (*GetDriverInfo)  (InputDriverInfo              *driver_info);
     DFBResult (*OpenDevice)     (CoreInputDevice              *device,
                                  unsigned int                  number,
                                  InputDeviceInfo              *device_info,
                                  void                        **driver_data);
     DFBResult (*GetKeymapEntry) (CoreInputDevice              *device,
                                  void                         *driver_data,
                                  DFBInputDeviceKeymapEntry    *entry);
     void      (*CloseDevice)    (void                         *driver_data);
     DFBResult (*Suspend)        (void);
     DFBResult (*Resume)         (void);
     DFBResult (*IsCreated)      (int                        index,
                                  void                      *data);
     InputDriverCapability
               (*GetCapability)  (void);
     DFBResult (*LaunchHotplug)  (CoreDFB                  *core,
                                  void                     *input_driver);
     DFBResult (*StopHotplug)    (void);

     DFBResult (*GetAxisInfo)    (CoreInputDevice              *device,
                                  void                         *driver_data,
                                  DFBInputDeviceAxisIdentifier  axis,
                                  DFBInputDeviceAxisInfo       *ret_info);
     DFBResult (*IOCtl)          (CoreInputDevice              *device,
                                  void                         *driver_data,
                                  InputDeviceIoctlData *param);
     DFBResult (*GetSubDeviceInfos) (CoreInputDevice *device,
                                     InputSubDeviceInfos *param);
} InputDriverFuncs;


typedef enum {
     CIDC_RELOAD_KEYMAP,
     CIDC_DEVICE_IOCTL
} CoreInputDeviceCommand;


typedef struct {
     DirectLink               link;

     int                      magic;

     DirectModuleEntry       *module;

     const InputDriverFuncs  *funcs;

     InputDriverInfo          info;

     int                      nr_devices;
} InputDriver;

typedef struct {
     int                          min_keycode;
     int                          max_keycode;
     int                          num_entries;
     DFBInputDeviceKeymapEntry   *entries;
} InputDeviceKeymap;

typedef struct {
     int                          magic;

     DFBInputDeviceID             id;            /* unique device id */

     int                          num;

     InputDeviceInfo              device_info;

     InputDeviceKeymap            keymap;

     DFBInputDeviceModifierMask   modifiers_l;
     DFBInputDeviceModifierMask   modifiers_r;
     DFBInputDeviceLockState      locks;
     DFBInputDeviceButtonMask     buttons;

     DFBInputDeviceKeyIdentifier  last_key;      /* last key pressed */
     DFBInputDeviceKeySymbol      last_symbol;   /* last symbol pressed */
     bool                         first_press;   /* first press of key */

     FusionReactor               *reactor;       /* event dispatcher */
     FusionSkirmish               lock;

     FusionCall                   call;          /* driver call via master */

     unsigned int                 axis_num;
     DFBInputDeviceAxisInfo      *axis_info;
     FusionRef                    ref; /* Ref between shared device & local device */

     InputDeviceKeymap            physical_keymap[MAX_LINUX_INPUT_DEVICES];/* Every physical device has own keymap */
} InputDeviceShared;

struct __DFB_CoreInputDevice {
     DirectLink          link;

     int                 magic;

     InputDeviceShared  *shared;

     InputDriver        *driver;
     void               *driver_data;

     CoreDFB            *core;
};

/**********************************************************************************************************************/


typedef enum 
{
     DFB_KEY_RST_LAN_START,
     DFB_KEY_RST_LAN_TYPE,
     DFB_KEY_RST_LAN_SMB, 
     DFB_KEY_RST_LAN_SMB_CNT, 
     DFB_KEY_RST_LAN_MODI,   
     DFB_KEY_RST_LAN_MAP,
     DFB_KEY_RST_LAN_END,
     DFB_KEY_RST_LAN_INITED,
     DFB_KEY_RST_LAN_MAX
} DFB_KEY_LAN_RST;

typedef enum 
{
    DIKI_MODI_SHIFT_L=0x0,
    DIKI_MODI_SHIFT_R,
    DIKI_MODI_CONTROL_L,
    DIKI_MODI_CONTROL_R,
    DIKI_MODI_ALT_L,
    DIKI_MODI_ALT_R,
    DIKI_MODI_META_L,
    DIKI_MODI_META_R,
    DIKI_MODI_SUPER_L,
    DIKI_MODI_SUPER_R,
    DIKI_MODI_HYPER_L,
    DIKI_MODI_HYPER_R,
    DIKI_MODI_CAPS_LOCK,
    DIKI_MODI_NUM_LOCK,
    DIKI_MODI_SCROLL_LOCK,
    DIKI_MODI_MAX
}DFB_KEY_LAN_MODI;

typedef struct _DFBKeyModiName 
{
     DFB_KEY_LAN_MODI   symbol;
     const char         *name;
}DFBKeyModiName;

typedef struct  _DFB_KEY_LAN_OBJ
{
    DFB_KEY_LAN_RST                     e_sate; 
    FusionSHMPoolShared                 *pool;
    DFBInputDeviceKeymapSymbolIndex     ec_smb[DIKSI_MAX];
    unsigned int                        au4_modi[DIKI_MODI_MAX];
    unsigned int                        au4_modi_mask[DIKI_MODI_MAX];
    DFBKeyModiName                      *pa_modi_name;
    InputDeviceKeymap                   t_map;
} DFB_KEY_LAN_OBJ;



typedef DFBEnumerationResult (*InputDeviceCallback) (CoreInputDevice *device,
                                                     void            *ctx);

void dfb_input_enumerate_devices( InputDeviceCallback         callback,
                                  void                       *ctx,
                                  DFBInputDeviceCapabilities  caps );


DirectResult dfb_input_attach       ( CoreInputDevice *device,
                                      ReactionFunc     func,
                                      void            *ctx,
                                      Reaction        *reaction );

DirectResult dfb_input_detach       ( CoreInputDevice *device,
                                      Reaction        *reaction );

DirectResult dfb_input_attach_global( CoreInputDevice *device,
                                      int              index,
                                      void            *ctx,
                                      GlobalReaction  *reaction );

DirectResult dfb_input_detach_global( CoreInputDevice *device,
                                      GlobalReaction  *reaction );


DFBResult    dfb_input_add_global   ( ReactionFunc     func,
                                      int             *ret_index );

DFBResult    dfb_input_set_global   ( ReactionFunc     func,
                                      int              index );


void         dfb_input_dispatch     ( CoreInputDevice *device,
                                      DFBInputEvent   *event );



void              dfb_input_device_description( const CoreInputDevice     *device,
                                                DFBInputDeviceDescription *desc );

DFBInputDeviceID  dfb_input_device_id         ( const CoreInputDevice     *device );

CoreInputDevice  *dfb_input_device_at         ( DFBInputDeviceID           id );



DFBInputDeviceCapabilities dfb_input_device_caps( const CoreInputDevice *device );



DFBResult         dfb_input_device_get_keymap_entry( CoreInputDevice           *device,
                                                     int                        keycode,
                                                     DFBInputDeviceKeymapEntry *entry );

DFBResult         dfb_input_device_set_keymap_entry( CoreInputDevice           *device,
                                                     int                        keycode,
                                                     DFBInputDeviceKeymapEntry *entry );

DFBResult         dfb_input_device_load_keymap   ( CoreInputDevice           *device,
                                                   char                      *filename );

DFBResult        dfb_input_device_ioctl( CoreInputDevice           *device, InputDeviceIoctlData *param);
DFBResult         dfb_input_device_reload_keymap   ( CoreInputDevice           *device );




void              containers_attach_device( CoreInputDevice *device );

void              containers_detach_device( CoreInputDevice *device );

void              stack_containers_attach_device( CoreInputDevice *device );

void              stack_containers_detach_device( CoreInputDevice *device );

DFBResult         dfb_input_create_device( int      device_index,
                                           CoreDFB *core_in,
                                           void    *driver_in );

DFBResult         dfb_input_remove_device( int   device_index,
                                           void *driver_in );

DFBResult         dfb_input_allocate_physical_device_keymap(DFBInputDeviceID id,
                                                            int deviceposition,
                                                            char *filename );
DFBResult         dfb_input_deallocate_physical_device_keymap(DFBInputDeviceID id,
                                                        int deviceposition);

DFBResult         dfb_input_load_physical_device_keymap(CoreInputDevice *device,
                                                        char *filename,
                                                        int deviceposition);
DFBInputDeviceKeymapEntry * dfb_input_get_entry_from_sub_device_keymap(DFBInputDeviceID id,
                                                                 int deviceposition,
                                                                 int keycode);
/* global reactions */

typedef enum {
     DFB_WINDOWSTACK_INPUTDEVICE_LISTENER
} DFB_INPUT_GLOBALS;


DFBResult dfb_input_insert_local_device_handle(int id, IDirectFBInputDevice * interface);
DFBResult dfb_input_find_local_device_handle(int id, IDirectFBInputDevice ** interface);
DFBResult dfb_input_remove_local_device_handle(int id);
DFBResult dfb_input_remove_all_local_device_handles(void);
DFBResult dfb_input_device_ioctl( CoreInputDevice           *device,
                                  InputDeviceIoctlData      *param);

#endif
