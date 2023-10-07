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

//#define DIRECT_ENABLE_DEBUG

#include <config.h>

#include <asm/types.h>    /* Needs to be included before dfb_types.h */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#if defined(HAVE_SYSIO)
# include <sys/io.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/kd.h>

#include <pthread.h>

#ifdef USE_SYSFS
# include <sysfs/libsysfs.h>
#endif

#include <fusion/arena.h>
#include <fusion/fusion.h>
#include <fusion/reactor.h>
#include <fusion/shmalloc.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/layer_control.h>
#include <core/layers.h>
#include <core/gfxcard.h>
#include <core/palette.h>
#include <core/screen.h>
#include <core/screens.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/state.h>
#include <core/windows.h>

#include <gfx/convert.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/signals.h>
#include <direct/system.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#include "mstar_types.h"

#include "fbdev.h"
#include "mstarFb.h"

#include "mstar_screen.h"

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_os.h"
#include "mi_osd.h"
#include "mi_disp.h"


#include <core/core_system.h>


D_DEBUG_DOMAIN( FBDev_Mode, "FBDev/Mode", "FBDev System Module Mode Switching" );
D_DEBUG_DOMAIN( FBDev_Primary, "FBDev/Primary", "FBDev Primary Layer" );

/******************************************************************************/

typedef enum {
    CLC_ADD_REGION,
    CLC_TEST_REGION,
    CLC_SET_REGION,
    CLC_REMOVE_REGION,
    CLC_SET_LAYERLEVEL,
    CLC_SET_COLORADJUSTMENT,
    CLC_FORCEWRITE_ENABLE,
    CLC_FORCEWRITE_DISABLE,
    CLC_CONFIG_DISPLAYMODE,
    CLC_SET_MIRRORMODE,
    CLC_SET_LBCOUPLEMODE,
    CLC_SET_GOPBYPASSMODE,
    CLC_SET_GOPSCALE,
    CLC_SET_GOPDST,
    CLC_SET_FORCEWRITE,
    CLC_SET_GOPBLOGO,
    CLC_UPDATE_REGION,
    CLC_SET_GOPAUTODETECTBUF,
} CoreLayerCommand;

typedef struct {
     CoreLayerCommand               type;
} CoreLayerEvent;

typedef struct {
    MSTARRegionData                     *reg_data;  // shared
    CoreSurface                         *surface;   // shared
    CoreSurfaceBufferLock               *lock;      // shared
    CoreLayerRegionConfig                config;
    CoreLayerRegionConfigFlags           flags;
    DFBRegion                            update;
    int                                  level;      // layer level
    DFBColorAdjustment                   coloradjustment;
    DFBDisplayLayerDeskTopDisplayMode    display_mode;
    bool                                 HMirrorEnable;
    bool                                 VMirrorEnable;
    bool                                 LBCoupleEnable;
    bool                                 ByPassEnable;
    int                                  HScale;
    int                                  VScale;
    int                      GopDst;
    bool                                 ForceWrite;
    int                                  miusel;
    bool                                 GOPAutoDetectEnable;
} CoreLayerCallParameter;

typedef struct {
    int fb_id;
    void *arg;
}FbdevCallParameter;

extern const SurfacePoolFuncs fbdevSurfacePoolFuncs;

static FusionCallHandlerResult
fbdev_ioctl_call_handler( int           caller,
                          int           call_arg,
                          void         *call_ptr,
                          void         *ctx,
                          unsigned int  serial,
                          int          *ret_val );

static int fbdev_ioctl( int request, void *arg, int arg_size );

#define FBDEV_IOCTL(request,arg)   fbdev_ioctl( request, arg, sizeof(*(arg)) )

#define RETRY_COUNT 10
#define SLEEP_TIME  50*1000

FBDev *dfb_all_fbdev[MSTAR_MAX_OUTPUT_LAYER_COUNT] = {NULL};

FBDev *dfb_fbdev = NULL;

static MI_HANDLE hDisp = MI_HANDLE_NULL;
/******************************************************************************/

static int       primaryLayerDataSize ( void );

static int       primaryRegionDataSize( void );
static bool      check_DFB_Reserved( int fd );
static DFBResult primaryInitLayer     ( CoreLayer                  *layer,
                                        void                       *driver_data,
                                        void                       *layer_data,
                                        DFBDisplayLayerDescription *description,
                                        DFBDisplayLayerConfig      *config,
                                        DFBColorAdjustment         *adjustment );

static DFBResult primarySetColorAdjustment( CoreLayer              *layer,
                                            void                   *driver_data,
                                            void                   *layer_data,
                                            DFBColorAdjustment     *adjustment );

static DFBResult primaryTestRegion    ( CoreLayer                  *layer,
                                        void                       *driver_data,
                                        void                       *layer_data,
                                        CoreLayerRegionConfig      *config,
                                        CoreLayerRegionConfigFlags *failed );


static DFBResult primarySetRegion     ( CoreLayer                  *layer,
                                        void                       *driver_data,
                                        void                       *layer_data,
                                        void                       *region_data,
                                        CoreLayerRegionConfig      *config,
                                        CoreLayerRegionConfigFlags  updated,
                                        CoreSurface                *surface,
                                        CorePalette                *palette,
										CoreSurfaceBufferLock	   *left_lock, 
										CoreSurfaceBufferLock	   *right_lock );



static DFBResult primaryFlipRegion    ( CoreLayer                  *layer,
                                        void                       *driver_data,
                                        void                       *layer_data,
                                        void                       *region_data,
                                        CoreSurface                *surface,
                                        DFBSurfaceFlipFlags         flags,
										CoreSurfaceBufferLock		*left_lock,
										CoreSurfaceBufferLock		*right_lock);


static DFBResult
mstarAddRegion( CoreLayer                   *layer,
                void                        *driver_data,
                void                        *layer_data,
                void                        *region_data,
                CoreLayerRegionConfig       *config );

static DFBResult
mstarRemoveRegion( CoreLayer    *layer,
                   void         *driver_data,
                   void         *layer_data,
                   void         *region_data );

static DFBResult mstar_primaryAllocateSurface( CoreLayer                 *layer,
                              void                      *driver_data,
                              void                      *layer_data,
                              void                      *region_data,
                              CoreLayerRegionConfig     *config,
                              CoreSurface              **ret_surface );

static DFBResult mstar_primaryReallocateSurface( CoreLayer                   *layer,
                                void                        *driver_data,
                                void                        *layer_data,
                                void                        *region_data,
                                CoreLayerRegionConfig       *config,
                                CoreSurface                 *surface );

static DFBResult mstar_primaryDeallocateSurface( CoreLayer              *layer,
                                void                   *driver_data,
                                void                   *layer_data,
                                void                   *region_data,
                                CoreSurface            *surface);


DFBResult mstar_GetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                    *level );

DFBResult mstar_SetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level );

DFBResult mstar_ConfigShadowLayer( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         ShadowLayerConfig      *shadowLayer_info);

DFBResult mstar_ConfigDisplayMode( CoreLayer                          *layer,
                         void                               *driver_data,
                         void                               *layer_data,
                         CoreSurface                        *surface,
                         CoreSurfaceBufferLock              *lock,
                         DFBDisplayLayerDeskTopDisplayMode   display_mode);

DFBResult mstar_SetHVMirrorEnable( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         bool                    HEnable,
                         bool                    VEnable);

DFBResult mstar_SetLBCoupleEnable( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         bool                    LBCoupleEnable);


DFBResult mstar_SetGOPDstByPassEnable( CoreLayer              *layer,
                             void                   *driver_data,
                             void                   *layer_data,
                             bool                    ByPassEnable);

DFBResult mstar_SetHVScale( CoreLayer              *layer,
                  void                   *driver_data,
                  void                   *layer_data,
                  void                   *region_data,
                  int                     HScale,
                  int                     VScale);

DFBResult mstar_SetGOPDst( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 DFBDisplayLayerGOPDST           GopDst);

DFBResult mstar_SetForceWrite( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     bool                    force_write);

DFBResult mstar_GetGOPDst( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 DFBDisplayLayerGOPDST      *GopDst);

DFBResult mstar_SetBootLogoPatch( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        int                     miusel);

DFBResult mstarUpdateRegion( CoreLayer                *layer,
                    void                    *driver_data,
                    void                    *layer_data,
                    void                    *region_data,
                    CoreSurface             *surface,
	                const DFBRegion 		   *left_update,
					CoreSurfaceBufferLock	   *left_lock, 
					const DFBRegion 		   *right_update,
					CoreSurfaceBufferLock	   *right_lock );


DFBResult mstarGetScreen( CoreLayer               *layer,
                CoreScreen             **ret_screen );


DFBResult mstar_SetGOPAutoDetectBuf( CoreLayer              *layer,
                                     void                   *driver_data,
                                     void                   *layer_data,
                                     bool                   bEnable );

DFBResult mstar_ScaleSourceRectangle( CoreLayer              *layer,
                            void                   *layer_data,
                            int x, int y, int w, int h);

DFBResult mstar_SetGOPOutputRatio( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        bool                    bEnable);

static DisplayLayerFuncs primaryLayerFuncs = {
     .LayerDataSize      = primaryLayerDataSize,
     .RegionDataSize     = primaryRegionDataSize,
     .InitLayer          = primaryInitLayer,

     .SetColorAdjustment = primarySetColorAdjustment,

     .TestRegion         = primaryTestRegion,
     .AddRegion          = mstarAddRegion,
     .SetRegion          = primarySetRegion,
     .RemoveRegion       = mstarRemoveRegion,
     .FlipRegion         = primaryFlipRegion,

     .AllocateSurface    =  mstar_primaryAllocateSurface,
     .ReallocateSurface  =  mstar_primaryReallocateSurface,
     .DeallocateSurface  =  mstar_primaryDeallocateSurface,
     .GetLevel                =   mstar_GetLayerlevel,
     .SetLevel                =   mstar_SetLayerlevel,
     .ConfigShadowLayer       =   mstar_ConfigShadowLayer,
     .ConfigDisplayMode       =   mstar_ConfigDisplayMode,
     .SetHVMirrorEnable       =   mstar_SetHVMirrorEnable,
     .SetLBCoupleEnable       =   mstar_SetLBCoupleEnable,
     .SetGOPDstByPassEnable   =   mstar_SetGOPDstByPassEnable,
     .SetHVScale              =   mstar_SetHVScale,
     .SetGOPDst               =   mstar_SetGOPDst,
     .SetForceWrite           =   mstar_SetForceWrite,
     .GetGOPDst               =   mstar_GetGOPDst,
     .SetBootLogoPatch        =   mstar_SetBootLogoPatch,
     .UpdateRegion            =   mstarUpdateRegion,
     .GetScreen               =   mstarGetScreen,
     .SetGOPAutoDetectBuf     =   mstar_SetGOPAutoDetectBuf,
     .ScaleSourceRectangle    =   mstar_ScaleSourceRectangle,
     .SetGOPOutputRatio       =   mstar_SetGOPOutputRatio,
};

/******************************************************************************/

static DFBResult primaryInitScreen  ( CoreScreen           *screen,
                                      CoreGraphicsDevice   *device,
                                      void                 *driver_data,
                                      void                 *screen_data,
                                      DFBScreenDescription *description );

static DFBResult primarySetPowerMode( CoreScreen           *screen,
                                      void                 *driver_data,
                                      void                 *screen_data,
                                      DFBScreenPowerMode    mode );

static DFBResult primaryWaitVSync   ( CoreScreen           *screen,
                                      void                 *driver_data,
                                      void                 *layer_data );

static DFBResult primaryGetScreenSize( CoreScreen           *screen,
                                       void                 *driver_data,
                                       void                 *screen_data,
                                       int                  *ret_width,
                                       int                  *ret_height );

static ScreenFuncs primaryScreenFuncs = {
     .InitScreen    = primaryInitScreen,
     .SetPowerMode  = primarySetPowerMode,
     .WaitVSync     = primaryWaitVSync,
     .GetScreenSize = primaryGetScreenSize,
};

FusionCallHandlerResult
mstar_layer_funcs_call_handler(int           caller,   /* fusion id of the caller */
                               int           call_arg, /* optional call parameter */
                               void         *call_ptr, /* optional call parameter */
                               void         *ctx,      /* optional handler context */
                               unsigned int  serial,
                               int          *ret_val);


/******************************************************************************/

//static DFBResult dfb_fbdev_read_modes( void );
static DFBResult dfb_fbdev_set_gamma_ramp( DFBSurfacePixelFormat format );
static DFBResult dfb_fbdev_set_palette( CorePalette *palette );
static DFBResult dfb_fbdev_set_rgb332_palette( void );
static DFBResult dfb_fbdev_pan( int xoffset, int yoffset, bool onsync );
static DFBResult dfb_fbdev_blank( int level );
static void      dfb_fbdev_var_to_mode( const struct fb_var_screeninfo *var,
                                        VideoMode                      *mode );
static const VideoMode *dfb_fbdev_find_mode( int                          width,
                                             int                          height );
static DFBResult dfb_fbdev_test_mode       ( const VideoMode             *mode,
                                             const CoreLayerRegionConfig *config );
static DFBResult dfb_fbdev_test_mode_simple( const VideoMode             *mode );

static DFBResult dfb_fbdev_set_mode        ( const VideoMode             *mode,
                                             CoreSurface                 *surface,
                                             unsigned int                 xoffset,
                                             unsigned int                 yoffset );

static void dfb_fbdev_changeSecureMode(bool bSecureStatus);
static DFBResult dfb_fbdev_register_output_timing(void);
static DFBResult dfb_fbdev_unregister_output_timing(void);

/******************************************************************************/

static inline
void waitretrace (void)
{
#if defined(HAVE_INB_OUTB_IOPL)
     if (iopl(3))
          return;

     if (!(inb (0x3cc) & 1)) {
          while ((inb (0x3ba) & 0x8))
               ;

          while (!(inb (0x3ba) & 0x8))
               ;
     }
     else {
          while ((inb (0x3da) & 0x8))
               ;

          while (!(inb (0x3da) & 0x8))
               ;
     }
#endif
}

/******************************************************************************/

/** public **/
#define CHECK_IOCTLRET(X)  \
    { \
        if( X < 0 )  \
        { \
            printf("\33[0;33;44m%s, %d, "#X" ==> failed! \33[0m\n", __FUNCTION__, __LINE__);\
            fprintf(stderr," error msg is :%s\n",strerror(errno));\
            return false;\
        } \
    }

#define CHECK_RET_IOCTL(X, ret)  \
    { \
        ret = X;\
        if( ret < 0 )  \
        { \
            printf("\33[0;33;44m%s, %d, "#X" ==> failed! \33[0m\n", __FUNCTION__, __LINE__);\
            fprintf(stderr," error msg is :%s\n",strerror(errno));\
        } \
    }

#define REGION_WITDH_720 720
#define REGION_WITDH_1280 1280
#define REGION_WITDH_1366 1366
#define REGION_WITDH_1920 1920
#define REGION_WITDH_3840 3840
#define REGION_WITDH_4096 4096

#define REGION_HEIGHT_480 480
#define REGION_HEIGHT_576 576
#define REGION_HEIGHT_720 720
#define REGION_HEIGHT_768 768
#define REGION_HEIGHT_1080 1080
#define REGION_HEIGHT_2160 2160

void _MI_OutputTiming_CallBack(MI_HANDLE hDisp, MI_U32 u32EventFlags, void *pEventParam, void *pUserParam)
{
     if(u32EventFlags == E_MI_DISP_CALLBACK_EVENT_TIMING_CHANGE) {

          MI_DISP_GetControllerParams_t stGetControllerParams;
          memset(&stGetControllerParams, 0, sizeof(MI_DISP_GetControllerParams_t));
          MI_HANDLE hDispController = MI_HANDLE_NULL;
          MI_DISP_Timing_e eOutputTiming = E_MI_DISP_TIMING_1920X1080_50I;

          MI_RESULT ret = MI_DISP_GetController(&stGetControllerParams, &hDispController);
          if(ret != MI_OK) {
               printf( "[DFB][%s %d] MI_DISP_GetController fail ret=%d (pid = %d)\n", __FUNCTION__, __LINE__, ret, getpid());
               return;
          }

          ret = MI_DISP_GetOutputTiming(hDispController,&eOutputTiming);
          if(ret != MI_OK) {
               printf( "[DFB][%s %d] MI_DISP_GetOutputTiming fail ret=%d (pid = %d)\n", __FUNCTION__, __LINE__, ret, getpid());
               return;
          }

          MI_FB_Rectangle_t dispRegion;
          DBG_LAYER_MSG( "[DFB][%s %d] eOutputTiming=%d (pid = %d)\n", __FUNCTION__, __LINE__, eOutputTiming, getpid());


          dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );

          //FBIOGET_SCREEN_LOCATION
          if (FBDEV_IOCTL( FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
               printf("  => FAILED to FBIOGET_SCREEN_LOCATION! errno=%d\n", errno );
          }

          dispRegion.u16Xpos = 0;
          dispRegion.u16Ypos = 0;
          int tempWidth = dispRegion.u16Width;
          int tempHeight = dispRegion.u16Height;

          switch(eOutputTiming)
          {
               case E_MI_DISP_TIMING_720X480_60I:
               case E_MI_DISP_TIMING_720X480_60P:
                    dispRegion.u16Width = REGION_WITDH_720;
                    dispRegion.u16Height = REGION_HEIGHT_480;
                    break;

               case E_MI_DISP_TIMING_720X576_50I:
               case E_MI_DISP_TIMING_720X576_50P:
                    dispRegion.u16Width = REGION_WITDH_720;
                    dispRegion.u16Height = REGION_HEIGHT_576;
                    break;
               case E_MI_DISP_TIMING_1280X720_50P:
               case E_MI_DISP_TIMING_1280X720_60P:
                    dispRegion.u16Width = REGION_WITDH_1280;
                    dispRegion.u16Height = REGION_HEIGHT_720;
                    break;
               case E_MI_DISP_TIMING_1366X768_50I:
               case E_MI_DISP_TIMING_1366X768_50P:
               case E_MI_DISP_TIMING_1366X768_60I:
               case E_MI_DISP_TIMING_1366X768_60P:
                    dispRegion.u16Width = REGION_WITDH_1366;
                    dispRegion.u16Height = REGION_HEIGHT_768;
                    break;
               case E_MI_DISP_TIMING_1920X1080_50I:
               case E_MI_DISP_TIMING_1920X1080_60I:
               case E_MI_DISP_TIMING_1920X1080_24P:
               case E_MI_DISP_TIMING_1920X1080_25P:
               case E_MI_DISP_TIMING_1920X1080_30P:
               case E_MI_DISP_TIMING_1920X1080_50P:
               case E_MI_DISP_TIMING_1920X1080_60P:
                    dispRegion.u16Width = REGION_WITDH_1920;
                    dispRegion.u16Height = REGION_HEIGHT_1080;
                    break;
               case E_MI_DISP_TIMING_3840X2160_24P:
               case E_MI_DISP_TIMING_3840X2160_25P:
               case E_MI_DISP_TIMING_3840X2160_30P:
               case E_MI_DISP_TIMING_3840X2160_50P:
               case E_MI_DISP_TIMING_3840X2160_60P:
                    dispRegion.u16Width = REGION_WITDH_3840;
                    dispRegion.u16Height = REGION_HEIGHT_2160;
                    break;
               case E_MI_DISP_TIMING_3840X1080_50P:
               case E_MI_DISP_TIMING_3840X1080_60P:
                    dispRegion.u16Width = REGION_WITDH_3840;
                    dispRegion.u16Height = REGION_HEIGHT_1080;
                    break;
               case E_MI_DISP_TIMING_4096X2160_24P:
               case E_MI_DISP_TIMING_4096X2160_25P:
               case E_MI_DISP_TIMING_4096X2160_30P:
               case E_MI_DISP_TIMING_4096X2160_50P:
               case E_MI_DISP_TIMING_4096X2160_60P:
                    dispRegion.u16Width = REGION_WITDH_4096;
                    dispRegion.u16Height = REGION_HEIGHT_2160;
                    break;
               default:
                    dispRegion.u16Width = tempWidth;
                    dispRegion.u16Height = tempHeight;
                    break;
         }

         DBG_LAYER_MSG("[DFB][%s %d] %dx%d (pid = %d)\n", __FUNCTION__, __LINE__, dispRegion.u16Width, dispRegion.u16Height, getpid());

         if(tempWidth == dispRegion.u16Width && tempHeight == dispRegion.u16Height) {
              DBG_LAYER_MSG("[DFB][%s %d] the same %dx%d, don't need call FBIOSET_SCREEN_LOCATION (pid = %d)\n", __FUNCTION__, __LINE__, dispRegion.u16Width, dispRegion.u16Height, getpid());
         } else {
              int i = 0, retIoctl = 0;
              for( i = 0; i < dfb_config->mst_gop_counts; i++) {
                  if (dfb_all_fbdev[i]->fd > 0) {
                      retIoctl = ioctl( dfb_all_fbdev[i]->fd, FBIOSET_SCREEN_LOCATION, &dispRegion );
                      DBG_LAYER_MSG("[DFB][%s %d] fbdev_ioctl, FBIOSET_SCREEN_LOCATION retIoctl = %d errno=%d\n", __FUNCTION__, __LINE__, retIoctl, errno);
                      retIoctl = ioctl( dfb_all_fbdev[i]->fd, FBIOPAN_DISPLAY, &(dfb_all_fbdev[i]->shared->current_var) );
                      DBG_LAYER_MSG("[DFB][%s %d] fbdev_ioctl, FBIOPAN_DISPLAY retIoctl = %d errno=%d\n", __FUNCTION__, __LINE__, retIoctl, errno);
                      DBG_LAYER_MSG("[DFB] %s, output timing set by callback => (x = %d, y = %d, w = %d, h = %d)\n", __FUNCTION__, dispRegion.u16Xpos, dispRegion.u16Ypos, dispRegion.u16Width, dispRegion.u16Height);
                      DBG_LAYER_MSG("[DFB] %s, initial output timing => (w = %d, h = %d)\n", __FUNCTION__, dfb_all_fbdev[i]->shared->screen_width, dfb_all_fbdev[i]->shared->screen_height);
                      if ( (dispRegion.u16Width == dfb_all_fbdev[i]->shared->screen_width) && 
                           (dispRegion.u16Height == dfb_all_fbdev[i]->shared->screen_height) ) {
                          printf("[DFB] DLG_MODE is off!\n");
                          dfb_all_fbdev[i]->shared->DLG_MODE = false;
                      } else {
                          printf("[DFB] DLG_MODE is on!\n");
                          dfb_all_fbdev[i]->shared->DLG_MODE = true;
                      }
                  }
              }

         }

         dfb_gfxcard_unlock();
    }

}

static int dfb_fbdev_open(int _id)
{
    const int DEV_NAME_LENGTH = 20;
    const int DEV_MAX = 5;
    int fd = -1;
    char dev_name1[DEV_NAME_LENGTH], dev_name2[DEV_NAME_LENGTH];

    printf("[DFB] %s, open fbdev %d\n", __FUNCTION__, _id);
    if ( _id < 0 || _id > DEV_MAX)
        return fd;

    if ( dfb_config->mst_layer_fbdev[_id] )
    {
        printf("[DFB] %s, open fbdev : %s\n", __FUNCTION__, dfb_config->mst_layer_fbdev[_id]);
        fd = open( dfb_config->mst_layer_fbdev[_id], O_RDWR );
        if ( fd < 0 )
        {
            D_PERROR( "DirectFB/FBDev: Error opening '%s'! pid = %d\n", dfb_config->mst_layer_fbdev[_id], getpid());
            if( access( dfb_config->mst_layer_fbdev[_id], F_OK ) != 0 )
            {
              D_ERROR( "DirectFB/FBDev: framebuffer device : %d not exists!\n", _id );
            }
            else
            {
              D_ERROR( "DirectFB/FBDev: framebuffer device : %d exists! But no permission to open!\n", _id );
            }
       }

       int retry = 0;
       while(fd < 0 && retry < RETRY_COUNT) {
            fd = open( dfb_config->mst_layer_fbdev[_id], O_RDWR);
            printf("[DFB][%s %d] fd=%d errno=%d retry=%d\n", __FUNCTION__, __LINE__, fd, errno, retry);
            usleep(SLEEP_TIME);
            retry++;
       }

    }
    else
    {
        if(DFB_SUPPORT_AN == 1) //[AOSP] fbdev device node path
        {
            printf("[DFB] %s, open fbdev : /dev/graphics/fb%d\n", __FUNCTION__, _id);
            snprintf(dev_name1, sizeof(dev_name1), "/dev/graphics/fb%d", _id);
            snprintf(dev_name2, sizeof(dev_name2), "/dev/graphics/fb/%d", _id);
        }
        else //[Linux] fbdev device node path
        {
            printf("[DFB] %s, open fbdev : /dev/fb%d\n", __FUNCTION__, _id);
            snprintf(dev_name1, sizeof(dev_name1), "/dev/fb%d", _id);
            snprintf(dev_name2, sizeof(dev_name2), "/dev/fb/%d", _id);
        }

        fd = direct_try_open( dev_name1, dev_name2, O_RDWR, true );
        if ( fd < 0 )
        {
            D_ERROR( "DirectFB/FBDev: Error opening framebuffer device : %d pid = %d\n", _id, getpid());
            if (access( dev_name1, F_OK ) != 0 || access( dev_name2, F_OK ) != 0)
            {
                D_ERROR( "DirectFB/FBDev: framebuffer device : %d not exists!\n", _id );
            }
            else
            {
                D_ERROR( "DirectFB/FBDev: framebuffer device : %d exists! But no permission to open!\n", _id );
            }
        }

       int retry = 0;
       while(fd < 0 && retry < RETRY_COUNT) {
            fd = direct_try_open( dev_name1, dev_name2, O_RDWR, true );
            printf("[DFB][%s %d] fd=%d errno=%d retry=%d\n", __FUNCTION__, __LINE__, fd, errno, retry);
            usleep(SLEEP_TIME);
            retry++;
       }

    }
    return fd;
}



static bool check_DFB_Reserved( int fd )
{
    u8 dfb_reserved;

    /* check DFB_RESERVED */
    if ( ioctl( fd, FBIOGET_DFB_RESERVED, &dfb_reserved) < 0) {
        DBG_LAYER_MSG( "DirectFB/FBDev: fb %d can not get FBIOGET_DFB_RESERVED!\n", fd );
        close( fd );
        return false;
    }

    DBG_LAYER_MSG("%s, %d, dfb_reserved = %d\n", __FUNCTION__, __LINE__, dfb_reserved);
    if (dfb_reserved == 0) {
        DBG_LAYER_MSG( "DirectFB/FBDev: fb %d is not DFB Reserved!dfb_reserved = %d\n", fd, dfb_reserved );
        close( fd );
        return false;
    }

    return true;
}
/******************************************************************************/

DFBResult
mstar_fbdev_initialize( void *driver_data, void **data )
{
    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_START, DF_BOOT_LV6);

     DFBResult            ret = DFB_OK;
     CoreScreen          *screen;
     long                 page_size;
     FBDevShared         *shared = NULL;
     FusionSHMPoolShared *pool;
     FusionSHMPoolShared *pool_data;

     MSTARDriverData *sdrv = driver_data;
     CoreDFB *core = sdrv->core;
     int i=0, layer_idx=0;
     char buf[16];

     u8 bootlogo_used;
     bool b_miosd_init = false;
     int bIommuUsed = 0;

     pool      = dfb_core_shmpool( core );
     pool_data = dfb_core_shmpool_data( core );

     sdrv->op_screen  = dfb_screens_register( NULL, driver_data, &mstarOPScreenFuncs );
     sdrv->ip0_screen = dfb_screens_register( NULL, driver_data, &mstarIP0ScreenFuncs );
     sdrv->ve_screen  = dfb_screens_register( NULL, driver_data, &mstarVEScreenFuncs );
     sdrv->oc_screen  = dfb_screens_register( NULL, driver_data, &mstarOCScreenFuncs );

     for( i = 0; i < dfb_config->mst_gop_counts; i++)
     {
         int fd = dfb_fbdev_open(i);
         if (fd < 0) {
             printf("[DFB](%s line %d) fb%d open fail!!!\n", __FUNCTION__, __LINE__, i);
             break;
         }
         D_ASSERT( dfb_fbdev == NULL );

         /* check DFB_RESERVED */
         if ( !check_DFB_Reserved( fd ) )
            continue;

         /* check bootlogo used */
         if ( ioctl( fd, FBIOGET_BOOTLOGO_USED, &bootlogo_used) < 0) {
             DBG_LAYER_MSG( "DirectFB/FBDev: fb %d can not get FBIOGET_BOOTLOGO_USED!\n", i );
             close( fd );
             continue;
         }

         DBG_LAYER_MSG("%s, %d, bootlogo_used = %d\n", __FUNCTION__, __LINE__, bootlogo_used);
         if (bootlogo_used) {
             if ( ioctl( fd, FBIO_DISABLE_BOOTLOGO) < 0) {
                 DBG_LAYER_MSG( "DirectFB/FBDev: fb %d can not set FBIO_DISABLE_BOOTLOGO!\n", i );
                 close( fd );
                 continue;
             }
             b_miosd_init = true;
         }

         MI_FB_MemInfo_t fb_meminfo;
         if (ioctl( fd, FBIOGET_MEM_INFO, &fb_meminfo ) < 0) {
             printf( "DirectFB/FBDev: Could not get fbdev mem info!\n" );
             close( fd );
             continue;
         }
         DBG_LAYER_MSG("%s, get mem info, phyAddr : 0x%llx, length : 0x%x\n", __FUNCTION__, fb_meminfo.phyAddr, fb_meminfo.length);
         // assign dfb mem for fbdev.
         if (fb_meminfo.phyAddr <= 0 || fb_meminfo.length <= 0) {
             /*Get iommu used design for DFB*/
             if (ioctl( fd, FBIOGET_IOMMU_USED, &bIommuUsed) < 0) {
                 printf( "DirectFB/FBDev: Could not get fbdev iommu used!\n" );
                 close( fd );
                 continue;
             }
             printf("bIommuUsed is %d  \n", bIommuUsed);
             if (bIommuUsed) {
                 /*Allocate IOMMU MemLength: need allocate memory length*/
                 int width = dfb_config->mst_layer_default_width;
                 int height = dfb_config->mst_layer_default_height;
                 const int buf_num = 2;
                 if (dfb_config->mst_GPU_AFBC && (i == 1))
                     height += dfb_config->GPU_AFBC_EXT_SIZE;

                 long MemLength = DFB_BYTES_PER_LINE( DSPF_ARGB, width ) * height * buf_num;
                 
                 printf("[DFB] MemLength = 0x%x, height = %d\n", MemLength, height);
                 if (ioctl( fd, FBIO_AllOCATE_IOMMU,&MemLength) < 0) {
                     printf( "DirectFB/FBDev: Could not allocate iommu mem for fbdev!\n" );
                     close( fd );
                     continue;
                 }
             }
             else {
                 // TODO : preallocate surface to set mem for fbdev.
                 fb_meminfo.phyAddr = dfb_config->mst_gop_regdmaphys_addr;
                 fb_meminfo.length = dfb_config->mst_gop_regdmaphys_len;

                 if (ioctl( fd, FBIOSET_MEM_INFO, &fb_meminfo ) < 0) {
                     printf( "DirectFB/FBDev: Could not set dfb mem for fbdev!\n" );
                     close( fd );
                     continue;
                 }
             }
             // FBIOSET_OSD_INFO
             MI_FB_OsdInfo_t fb_osdinfo;
             fb_osdinfo.u32Width = dfb_config->mst_layer_default_width;
             fb_osdinfo.u32Height = dfb_config->mst_layer_default_height;
             fb_osdinfo.u32Pitch = DFB_BYTES_PER_LINE( DSPF_ARGB, fb_osdinfo.u32Width );
             fb_osdinfo.eColorFmt = E_MI_FB_COLOR_FMT_ARGB8888;
             if (ioctl( fd, FBIOSET_OSD_INFO, &fb_osdinfo) < 0) {
                 printf( "DirectFB/FBDev: Could not set FBIOSET_OSD_INFO!\n" );
                 close( fd );
                 continue;
             }
             //printf("%s, set OSD INFO finish!\n", __FUNCTION__);
             b_miosd_init = true;
         }
         else {
             // check dfb and fbdev mmaping is override or not.
             if ( ( !dfb_MMA_IsMMAEnabled() ) &&
                  (fb_meminfo.phyAddr == dfb_config->video_phys_hal) ||
                  (fb_meminfo.phyAddr < dfb_config->video_phys_hal && fb_meminfo.phyAddr + fb_meminfo.length > dfb_config->video_phys_hal) ||
                  (fb_meminfo.phyAddr > dfb_config->video_phys_hal && fb_meminfo.phyAddr < dfb_config->video_phys_hal + dfb_config->video_length) ) {
                 printf("\033[1;31mDirectFB/FBDev: Error, Fbdev phy addr override DFB phy addr !\033[0m\n");
                 break;
             }
         }

         if (b_miosd_init) {
             // FBIO_MIOSD_INIT
             if (ioctl( fd, FBIO_MIOSD_INIT ) < 0) {
                 printf( "DirectFB/FBDev: Could not set FBIO_MIOSD_INIT!\n" );
                 close( fd );
                 continue;
             }
             b_miosd_init = false;
             //printf("%s, set MIOSD INIT finish!\n", __FUNCTION__);
         }

         dfb_fbdev = D_CALLOC( 1, sizeof(FBDev) );
         if (!dfb_fbdev) {
             ret = D_OOM();
             continue;
         }

         dfb_fbdev->fd = fd;

         shared = (FBDevShared*) SHCALLOC( pool, 1, sizeof(FBDevShared) );
         if (!shared) {
             ret = D_OOSHM();
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;
             continue;
         }

         shared->shmpool      = pool;
         shared->shmpool_data = pool_data;

         shared->iommu = bIommuUsed;
         snprintf( buf, sizeof(buf), "fbdev %d", i );
         fusion_arena_add_shared_field( dfb_core_arena( core ), buf, shared );

         dfb_fbdev->core   = core;
         dfb_fbdev->shared = shared;

         page_size = 0x2000;
         //page_size = direct_pagesize();

         shared->page_mask = page_size < 0 ? 0 : (page_size - 1);

         /* Retrieve fixed informations like video ram size */
         if (ioctl( dfb_fbdev->fd, FBIOGET_FSCREENINFO, &shared->fix ) < 0) {
             D_PERROR( "DirectFB/FBDev: "
                    "Could not get fixed screen information!\n" );
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;
             continue;
         }

         DBG_LAYER_MSG( "DirectFB/FBDev: Found '%s' (ID %d) with frame buffer at 0x%08lx, %dk (MMIO 0x%08lx, %dk)\n",
                       shared->fix.id, shared->fix.accel,
                       shared->fix.smem_start, shared->fix.smem_len >> 10,
                       shared->fix.mmio_start, shared->fix.mmio_len >> 10 );

         if (( shared->iommu == 0 && shared->fix.smem_start <= 0 ) || shared->fix.smem_len <= 0)
            continue;


         if (ioctl( dfb_fbdev->fd, FBIOGET_VSCREENINFO, &shared->orig_var ) < 0) {
             D_PERROR( "DirectFB/FBDev: "
                    "Could not get variable screen information!\n" );
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;
             continue;
         }

         shared->current_var = shared->orig_var;
         shared->current_var.accel_flags = 0;

         if (ioctl( dfb_fbdev->fd, FBIOPUT_VSCREENINFO, &shared->current_var ) < 0) {
             D_PERROR( "DirectFB/FBDev: "
                    "Could not disable console acceleration!\n" );
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;
             continue;
         }

         DBG_LAYER_MSG("%s, current_mode bpp:%d, virtual (%d, %d)\n", __FUNCTION__, shared->current_var.bits_per_pixel,
            shared->current_var.xres_virtual, shared->current_var.yres_virtual);

         dfb_fbdev_var_to_mode( &shared->current_var, &shared->current_mode );

         shared->orig_cmap_memory = SHMALLOC( pool_data, 256 * 2 * 4 );
         if (!shared->orig_cmap_memory) {
             ret = D_OOSHM();
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;

             continue;
         }

         shared->orig_cmap.start  = 0;
         shared->orig_cmap.len    = 256;
         shared->orig_cmap.red    = shared->orig_cmap_memory + 256 * 2 * 0;
         shared->orig_cmap.green  = shared->orig_cmap_memory + 256 * 2 * 1;
         shared->orig_cmap.blue   = shared->orig_cmap_memory + 256 * 2 * 2;
         shared->orig_cmap.transp = shared->orig_cmap_memory + 256 * 2 * 3;

         if (ioctl( dfb_fbdev->fd, FBIOGETCMAP, &shared->orig_cmap ) < 0) {
             D_DEBUG( "DirectFB/FBDev: "
                   "Could not retrieve palette for backup!\n" );

             memset( &shared->orig_cmap, 0, sizeof(shared->orig_cmap) );

             SHFREE( pool_data, shared->orig_cmap_memory );
             shared->orig_cmap_memory = NULL;
         }

         shared->temp_cmap_memory = SHMALLOC( pool_data, 256 * 2 * 4 );
         if (!shared->temp_cmap_memory) {
             ret = D_OOSHM();
             close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;

             continue;
         }

         shared->temp_cmap.start  = 0;
         shared->temp_cmap.len    = 256;
         shared->temp_cmap.red    = shared->temp_cmap_memory + 256 * 2 * 0;
         shared->temp_cmap.green  = shared->temp_cmap_memory + 256 * 2 * 1;
         shared->temp_cmap.blue   = shared->temp_cmap_memory + 256 * 2 * 2;
         shared->temp_cmap.transp = shared->temp_cmap_memory + 256 * 2 * 3;

         shared->current_cmap_memory = SHMALLOC( pool_data, 256 * 2 * 4 );
         if (!shared->current_cmap_memory) {
              ret = D_OOSHM();
              close( dfb_fbdev->fd );
             D_FREE( dfb_fbdev );
             dfb_fbdev = NULL;

             continue;
         }

         shared->current_cmap.start  = 0;
         shared->current_cmap.len    = 256;
         shared->current_cmap.red    = shared->current_cmap_memory + 256 * 2 * 0;
         shared->current_cmap.green  = shared->current_cmap_memory + 256 * 2 * 1;
         shared->current_cmap.blue   = shared->current_cmap_memory + 256 * 2 * 2;
         shared->current_cmap.transp = shared->current_cmap_memory + 256 * 2 * 3;

         DBG_LAYER_MSG("%s, %d, dfb_fbdev = %p\n", __FUNCTION__, __LINE__, dfb_fbdev);

         fusion_call_init( &shared->fbdev_ioctl,
                           fbdev_ioctl_call_handler, NULL, dfb_core_world(core) );

         if (layer_idx == 0) {
             dfb_surface_pool_initialize( core, &fbdevSurfacePoolFuncs, &dfb_fbdev->shared->pool );
         }
         else {
             dfb_fbdev->shared->pool = dfb_all_fbdev[0]->shared->pool;
         }
         DBG_LAYER_MSG("%s, %d, shared->pool : %p\n", __FUNCTION__, __LINE__, dfb_fbdev->shared->pool);

         /* Register primary screen functions */
         screen = dfb_screens_register( NULL, driver_data, &primaryScreenFuncs );

         /* Register primary layer functions */
		 sdrv->layers[layer_idx] = dfb_layers_register( screen, driver_data, &primaryLayerFuncs );

         /* Initial DLG_mode to false */
         dfb_fbdev->shared->DLG_MODE = false;
         /* keep in global */
		 dfb_all_fbdev[layer_idx] = dfb_fbdev;
		 layer_idx++;

     }

     *data = dfb_all_fbdev[0];

     ret = dfb_fbdev_register_output_timing();
     if(ret) {
        printf("[DFB][%s %d] ret=0x%x\n", __FUNCTION__, __LINE__, ret);
        return ret;
     }

     DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_END, DF_BOOT_LV6);
     return ret;
}

DFBResult
mstar_fbdev_join(  void *driver_data, void **data )
{
     DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_START, DF_BOOT_LV6);
     DFBResult   ret = DFB_OK;
     CoreScreen *screen;
     void       *shared;

     MSTARDriverData *sdrv = driver_data;
     CoreDFB *core = sdrv->core;

     int i=0, layer_idx=0;
     char buf[16];

     sdrv->op_screen  = dfb_screens_register( NULL, driver_data, &mstarOPScreenFuncs );
     sdrv->ip0_screen = dfb_screens_register( NULL, driver_data, &mstarIP0ScreenFuncs );
     sdrv->ve_screen  = dfb_screens_register( NULL, driver_data, &mstarVEScreenFuncs );
     sdrv->oc_screen  = dfb_screens_register( NULL, driver_data, &mstarOCScreenFuncs );

     for( i = 0; i < dfb_config->mst_gop_counts; i++)
     {
         int fd = dfb_fbdev_open(i);
         if (fd < 0)
             break;

         /* check DFB_RESERVED */
         if ( !check_DFB_Reserved( fd ) )
            continue;

         D_ASSERT( dfb_fbdev == NULL );

         dfb_fbdev = D_CALLOC( 1, sizeof(FBDev) );
         if (!dfb_fbdev)
             return D_OOM();

         snprintf( buf, sizeof(buf), "fbdev %d", i );
         ret = fusion_arena_get_shared_field( dfb_core_arena( core ), buf, &shared );
         if(ret) {
             DBG_LAYER_MSG("%s, %d, ret=%s\n", __FUNCTION__, __LINE__, DirectFBErrorString( ret ));
             D_FREE(dfb_fbdev);
             return ret;
         }

         dfb_fbdev->core = core;
         dfb_fbdev->shared = shared;
         dfb_fbdev->fd = fd;
         DBG_LAYER_MSG("%s, fb %d, dfb_fbdev->fd=%d, dfb_fbdev->shared->iommu=%d\n", __FUNCTION__, i, dfb_fbdev->fd, dfb_fbdev->shared->iommu);

         if (i == 0) {
             dfb_surface_pool_join( core, dfb_fbdev->shared->pool, &fbdevSurfacePoolFuncs );
         }
         else {
             dfb_fbdev->shared->pool = dfb_all_fbdev[0]->shared->pool;
         }

         /* Register primary screen functions */
         screen = dfb_screens_register( NULL, driver_data, &primaryScreenFuncs );

         /* Register primary layer functions */
         // todo : what layer id used.
         sdrv->layers[layer_idx] = dfb_layers_register( screen, driver_data, &primaryLayerFuncs );

          /* keep in global */
         dfb_all_fbdev[layer_idx] = dfb_fbdev;
		 layer_idx++;
     }

     *data = dfb_all_fbdev[0];
     DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_END, DF_BOOT_LV6);
     return ret;
}

DFBResult
mstar_fbdev_shutdown( bool emergency )
{
     DFBResult            ret = DFB_OK;
     VideoMode           *m;
     FBDevShared         *shared;
     FusionSHMPoolShared *pool;
     int i;

     ret = dfb_fbdev_unregister_output_timing();
     if(ret)
        printf("[DFB][%s %d] ret=0x%x\n", __FUNCTION__, __LINE__, ret);

    for(i=0; i< dfb_config->mst_gop_counts; i++)
    {
         if (dfb_all_fbdev[i] == NULL)
            continue;

         shared = dfb_all_fbdev[i]->shared;

         D_ASSERT( shared != NULL );

         pool = shared->shmpool;

         D_ASSERT( pool != NULL );

         m = shared->modes;
         while (m) {
              VideoMode *next = m->next;
              SHFREE( pool, m );
              m = next;
         }

         if (dfb_all_fbdev[i]->fd <= 0) {
             D_PERROR( "DirectFB/FBDev: "
                        "dfb_all_fbdev[%d]->fd = %d!\n", i, dfb_all_fbdev[i]->fd );
             continue;
         }

         /* restore orig_var to fbdev. */
         if (ioctl( dfb_all_fbdev[i]->fd, FBIOPUT_VSCREENINFO, &shared->orig_var ) < 0) {
              D_PERROR( "DirectFB/FBDev: "
                        "Could not restore variable screen information!\n" );
         }

         if (shared->orig_cmap.len) {
              if (ioctl( dfb_all_fbdev[i]->fd, FBIOPUTCMAP, &shared->orig_cmap ) < 0)
                   D_DEBUG( "DirectFB/FBDev: "
                            "Could not restore palette!\n" );
         }

         if (shared->orig_cmap_memory)
              SHFREE( shared->shmpool_data, shared->orig_cmap_memory );

         if (shared->temp_cmap_memory)
              SHFREE( shared->shmpool_data, shared->temp_cmap_memory );

         if (shared->current_cmap_memory)
              SHFREE( shared->shmpool_data, shared->current_cmap_memory );

         if (i == 0)
             dfb_surface_pool_destroy( dfb_all_fbdev[i]->shared->pool );

         if (shared->iommu) {
             if (ioctl(dfb_all_fbdev[i]->fd, FBIO_FREE_IOMMU) < 0)
                 printf( "DirectFB/FBDev: Could not free fbdev iommu mem!\n" );
         }

         fusion_call_destroy( &shared->fbdev_ioctl );

         close( dfb_all_fbdev[i]->fd );

         SHFREE( pool, shared );
         D_FREE( dfb_all_fbdev[i] );
         dfb_all_fbdev[i] = NULL;
    }

     return ret;
}

DFBResult
mstar_fbdev_leave( bool emergency )
{
     DFBResult ret = DFB_OK;
     int i;

     for(i=0; i< dfb_config->mst_gop_counts; i++ )
     {
         if ( dfb_all_fbdev[i] == NULL )
            continue;

         if (i == 0)
             dfb_surface_pool_leave( dfb_all_fbdev[i]->shared->pool );

         close( dfb_all_fbdev[i]->fd );

         D_FREE( dfb_all_fbdev[i] );
         dfb_all_fbdev[i] = NULL;
     }

     return ret;
}


/******************************************************************************/

static DFBResult
init_modes( void )
{
     //dfb_fbdev_read_modes();

     if (!dfb_fbdev->shared->modes) {
          /* try to use current mode*/
          dfb_fbdev->shared->modes = (VideoMode*) SHCALLOC( dfb_fbdev->shared->shmpool,
                                                            1, sizeof(VideoMode) );
          if (!dfb_fbdev->shared->modes)
               return D_OOSHM();

          *dfb_fbdev->shared->modes = dfb_fbdev->shared->current_mode;

          if (dfb_fbdev_test_mode_simple(dfb_fbdev->shared->modes)) {
               D_ERROR("DirectFB/FBDev: "
                        "No supported modes found in /etc/fb.modes and "
                        "current mode not supported!\n");

               D_ERROR( "DirectFB/FBDev: Current mode's pixelformat: "
                         "rgba %d/%d, %d/%d, %d/%d, %d/%d (%dbit)\n",
                         dfb_fbdev->shared->orig_var.red.length,
                         dfb_fbdev->shared->orig_var.red.offset,
                         dfb_fbdev->shared->orig_var.green.length,
                         dfb_fbdev->shared->orig_var.green.offset,
                         dfb_fbdev->shared->orig_var.blue.length,
                         dfb_fbdev->shared->orig_var.blue.offset,
                         dfb_fbdev->shared->orig_var.transp.length,
                         dfb_fbdev->shared->orig_var.transp.offset,
                         dfb_fbdev->shared->orig_var.bits_per_pixel );

               return DFB_INIT;
          }

          DBG_LAYER_MSG( "DirectFB/FBDev: Current mode's pixelformat: "
                         "rgba %d/%d, %d/%d, %d/%d, %d/%d (%dbit)\n",
                         dfb_fbdev->shared->orig_var.red.length,
                         dfb_fbdev->shared->orig_var.red.offset,
                         dfb_fbdev->shared->orig_var.green.length,
                         dfb_fbdev->shared->orig_var.green.offset,
                         dfb_fbdev->shared->orig_var.blue.length,
                         dfb_fbdev->shared->orig_var.blue.offset,
                         dfb_fbdev->shared->orig_var.transp.length,
                         dfb_fbdev->shared->orig_var.transp.offset,
                         dfb_fbdev->shared->orig_var.bits_per_pixel );
     }

     return DFB_OK;
}

/******************************************************************************/

static DFBResult
primaryInitScreen( CoreScreen           *screen,
                   CoreGraphicsDevice   *device,
                   void                 *driver_data,
                   void                 *screen_data,
                   DFBScreenDescription *description )
{
     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf(description->name, DFB_SCREEN_DESC_NAME_LENGTH, "MSTAR Screen (FBDev)");

     return DFB_OK;
}

static DFBResult
primarySetPowerMode( CoreScreen         *screen,
                     void               *driver_data,
                     void               *screen_data,
                     DFBScreenPowerMode  mode )
{
     int level;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

     switch (mode) {
          case DSPM_OFF:
               level = 4;
               break;
          case DSPM_SUSPEND:
               level = 3;
               break;
          case DSPM_STANDBY:
               level = 2;
               break;
          case DSPM_ON:
               level = 0;
               break;
          default:
               return DFB_INVARG;
     }

     return dfb_fbdev_blank( level );
}

static DFBResult
primaryWaitVSync( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data )
{
     static const int zero = 0;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

     if (dfb_config->pollvsync_none)
          return DFB_OK;

     if (ioctl( dfb_fbdev->fd, FBIO_WAITFORVSYNC, &zero ))
          waitretrace();

     return DFB_OK;
}

static DFBResult
primaryGetScreenSize( CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height )
{
     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

     D_ASSERT( dfb_fbdev != NULL );
     D_ASSERT( dfb_fbdev->shared != NULL );

     MI_FB_Rectangle_t dispRegion;
     //FBIOGET_SCREEN_LOCATION
     *ret_width = dfb_config->mst_lcd_width;
     *ret_height = dfb_config->mst_lcd_height;

     if (FBDEV_IOCTL( FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
         int erno = errno;
         D_DEBUG_AT( FBDev_Mode, "  => FAILED to FBIOGET_SCREEN_LOCATION!\n" );
         return errno2result( erno );
     }

     *ret_width = dispRegion.u16Width;
     *ret_height = dispRegion.u16Height;

     DBG_LAYER_MSG("%s, (%d, %d)\n", __FUNCTION__, dispRegion.u16Width, dispRegion.u16Height);

     return DFB_OK;
}

/******************************************************************************/

static int
primaryLayerDataSize( void )
{
     D_DEBUG_AT(FBDev_Primary, "%s\n", __FUNCTION__);
     return sizeof(MSTARLayerData);
}

static int
primaryRegionDataSize( void )
{
     D_DEBUG_AT(FBDev_Primary, "%s\n", __FUNCTION__);
     return sizeof(MSTARRegionData);
}

static DFBResult
primaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{
     DFBResult  ret;
     VideoMode *default_mode;

     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     MSTARLayerData  *data = layer_data;
     u8 i;
     CoreDFB *core = layer->core;
     MI_FB_Rectangle_t dispRegion;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

     /* check output timing */
     if ( ioctl( dfb_all_fbdev[0]->fd, FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
         int erno = errno;
         D_DEBUG_AT( FBDev_Mode, "  => FAILED to FBIOGET_SCREEN_LOCATION!, %s\n", errno2result( erno ) );
         //return errno2result( erno );
         data->screen_size.width  = dfb_config->mst_lcd_width;
         data->screen_size.height = dfb_config->mst_lcd_height;
     }
     else {
         data->screen_size.width  = dispRegion.u16Width;
         data->screen_size.height = dispRegion.u16Height;
     }

     /* initialize layer data */

     for( i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
     {
        if(layer == sdrv->layers[i])
        {
            char  buf[24];

            /* get current fbdev info. */
            dfb_fbdev = dfb_all_fbdev[i];

            /* initialize mode table */
            ret = init_modes();
            if (ret)
                return ret;

            /* keep initial screen size */
            dfb_fbdev->shared->screen_width = data->screen_size.width;
            dfb_fbdev->shared->screen_height = data->screen_size.height;

            default_mode = dfb_fbdev->shared->modes;

            data->layer_index = i;

            //the gopindex should be required form the dfbconfig
            data->gop_index                 = dfb_config->mst_gop_available[i];
            data->gop_index_r               = dfb_config->mst_gop_available_r[i];
            data->gop_dst                   = dfb_config->mst_gop_dstPlane[i];
            data->layer_displayMode         = DLDM_NORMAL;
            data->GopDstMode                = dfb_config->mst_gop_dstPlane[i];

            sdev->layer_zorder[i]           = dfb_config->mst_layer_gwin_level;

            sdev->layer_active[i]           = false;
            sdev->layer_gwin_id[i]          = 0xff;
            sdev->layer_gwin_id_r[i]        = 0xff;

            sdev->GOP_support_palette[i]    = false;

            //init fusion & reactor
            snprintf( buf, sizeof(buf), "Core Layer %d", i );

            /* create reactor */
            data->reactor = fusion_reactor_new( sizeof(CoreLayerEvent),
                                                buf,
                                                dfb_core_world(core) );

            DBG_LAYER_MSG("\33[1;31m[%s] layer_idx: %d  gop_idx: %d\33[0m\n",
                    __FUNCTION__, i, dfb_config->mst_gop_available[i]);

            /* init call */
            fusion_call_init( &data->call,
                              mstar_layer_funcs_call_handler,
                              layer,
                              dfb_core_world(core) );
            {
                MI_FB_LayerAttr_t stLayerAttr;
                stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_H_STRETCH;
                stLayerAttr.unLayerAttrParam.eLayerHStrecthMode = dfb_config->mst_h_stretch_mode;
                if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                    printf("[DFB] %s, FBIOSET_LAYER_ATTR, set H STRETCH mode failed!\n", __FUNCTION__);
                }

                stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_V_STRETCH;
                stLayerAttr.unLayerAttrParam.eLayerVStretchMode = dfb_config->mst_v_stretch_mode;
                if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                    printf("[DFB] %s, FBIOSET_LAYER_ATTR, set V STRETCH mode failed!\n", __FUNCTION__);
                }

                /* set AFBC */
                if (dfb_config->mst_GPU_AFBC && (data->layer_index == 1)) {
                    printf("[DFB] %s, GOP %d set AFBC\n", __FUNCTION__, data->layer_index);
                    stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_AFBC_MODE;
                    stLayerAttr.unLayerAttrParam.bOsdAfbc = true;
                    if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                        printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
                    }
                    /* set AFBC Crop */
                    stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_AFBC_CROP;
                    stLayerAttr.unLayerAttrParam.stCropParams.bEnableCrop = true;
                    stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropX = 0;
                    stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropY = 0;
                    stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropWidth = dfb_config->mst_layer_default_width;
                    stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropHeight = dfb_config->mst_layer_default_height;
                    if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                        printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
                    }
                }
            }
            break;
        }

    }

    D_ASSERT( i < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    D_DEBUG_AT(FBDev_Primary, "%s: data->layer: %d\n", __FUNCTION__, data->layer_index);

    for( i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        memset(&sdrv->mstarLayerBuffer[i],0,sizeof(sdrv->mstarLayerBuffer[i]));
        sdrv->mstarLayerBuffer[i].u8GWinID = 0xee;
    }

     /* set capabilities and type */
    description->caps = DLCAPS_SURFACE          |
                        DLCAPS_ALPHACHANNEL     |
                        DLCAPS_OPACITY          |
                        DLCAPS_SRC_COLORKEY     |
                        DLCAPS_LEVELS           |
                        DLCAPS_SCREEN_LOCATION;

    description->type = DLTF_STILL_PICTURE  |
                        DLTF_GRAPHICS       |
                        DLTF_VIDEO;

     /* set name */
     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "FBDev Primary Layer" );

     /* fill out default color adjustment */
     adjustment->flags      = DCAF_BRIGHTNESS | DCAF_CONTRAST | DCAF_SATURATION;
     adjustment->brightness = 0x8000;
     adjustment->contrast   = 0x8000;
     adjustment->saturation = 0x8000;

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH          |
                          DLCONF_HEIGHT         |
                          DLCONF_PIXELFORMAT    |
                          DLCONF_BUFFERMODE     |
                          DLCONF_OPTIONS;

     //config->buffermode = DLBM_FRONTONLY;
     //config->width      = dfb_config->mode.width  ? dfb_config->mode.width  : default_mode->xres;
     //config->height     = dfb_config->mode.height ? dfb_config->mode.height : default_mode->yres;
     config->width      =  (data->screen_size.width > dfb_config->mst_layer_default_width )? dfb_config->mst_layer_default_width : data->screen_size.width;
     config->height     =  (data->screen_size.height > dfb_config->mst_layer_default_height)? dfb_config->mst_layer_default_height :  data->screen_size.height;
     DBG_LAYER_MSG("\33[1;31m[%s] screen_size ( %d, %d )  layer_default_size ( %d, %d )\33[0m\n",
                    __FUNCTION__, data->screen_size.width, data->screen_size.height, dfb_config->mst_layer_default_width, dfb_config->mst_layer_default_height);

     config->buffermode  = DLBM_BACKVIDEO;
     //config->options     = DLOP_ALPHACHANNEL;
     //config->btile_mode  = false;

     //config->pixelformat = DSPF_ARGB;
     if (dfb_config->mode.format)
          config->pixelformat = dfb_config->mode.format;
     else
          config->pixelformat = dfb_pixelformat_for_depth( default_mode->bpp );

     return DFB_OK;
}

static DFBResult
primarySetColorAdjustment( CoreLayer          *layer,
                           void               *driver_data,
                           void               *layer_data,
                           DFBColorAdjustment *adjustment )
{
     struct fb_cmap *cmap       = &dfb_fbdev->shared->current_cmap;
     struct fb_cmap *temp       = &dfb_fbdev->shared->temp_cmap;
     int             contrast   = adjustment->contrast >> 8;
     int             brightness = (adjustment->brightness >> 8) - 128;
     int             saturation = adjustment->saturation >> 8;
     int             r, g, b, i;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );


     if (dfb_fbdev->shared->fix.visual != FB_VISUAL_DIRECTCOLOR)
          return DFB_UNIMPLEMENTED;

     /* Use gamma ramp to set color attributes */
     for (i = 0; i < (int)cmap->len; i++) {
          r = cmap->red[i];
          g = cmap->green[i];
          b = cmap->blue[i];
          r >>= 8;
          g >>= 8;
          b >>= 8;

          /*
        * Brightness Adjustment: Increase/Decrease each color channels
        * by a constant amount as specified by value of brightness.
        */
          if (adjustment->flags & DCAF_BRIGHTNESS) {
               r += brightness;
               g += brightness;
               b += brightness;

               r = CLAMP( r, 0, 255 );
               g = CLAMP( g, 0, 255 );
               b = CLAMP( b, 0, 255 );
          }

          /*
           * Contrast Adjustment:  We increase/decrease the "separation"
           * between colors in proportion to the value specified by the
           * contrast control. Decreasing the contrast has a side effect
           * of decreasing the brightness.
           */

          if (adjustment->flags & DCAF_CONTRAST) {
               /* Increase contrast */
               if (contrast > 128) {
                    int c = contrast - 128;

                    r = ((r + c/2)/c) * c;
                    g = ((g + c/2)/c) * c;
                    b = ((b + c/2)/c) * c;
               }
               /* Decrease contrast */
               else if (contrast < 127) {
                    r = (r * contrast) >> 7;
                    g = (g * contrast) >> 7;
                    b = (b * contrast) >> 7;
               }

               r = CLAMP( r, 0, 255 );
               g = CLAMP( g, 0, 255 );
               b = CLAMP( b, 0, 255 );
          }

          /*
           * Saturation Adjustment:  This is is a better implementation.
           * Saturation is implemented by "mixing" a proportion of medium
           * gray to the color value.  On the other side, "removing"
           * a proportion of medium gray oversaturates the color.
           */
          if (adjustment->flags & DCAF_SATURATION) {
               if (saturation > 128) {
                    int gray = saturation - 128;
                    int color = 128 - gray;

                    r = ((r - gray) << 7) / color;
                    g = ((g - gray) << 7) / color;
                    b = ((b - gray) << 7) / color;
               }
               else if (saturation < 128) {
                    int color = saturation;
                    int gray = 128 - color;

                    r = ((r * color) >> 7) + gray;
                    g = ((g * color) >> 7) + gray;
                    b = ((b * color) >> 7) + gray;
               }

               r = CLAMP( r, 0, 255 );
               g = CLAMP( g, 0, 255 );
               b = CLAMP( b, 0, 255 );
          }
          r |= r << 8;
          g |= g << 8;
          b |= b << 8;

          temp->red[i]   =  (unsigned short)r;
          temp->green[i] =  (unsigned short)g;
          temp->blue[i]  =  (unsigned short)b;
     }

     temp->len = cmap->len;
     temp->start = cmap->start;
     if (FBDEV_IOCTL( FBIOPUTCMAP, temp ) < 0) {
          D_PERROR( "DirectFB/FBDev: Could not set the palette!\n" );

          return errno2result(errno);
     }

     return DFB_OK;
}

const VideoMode *
dfb_fbdev_find_mode( int width, int height )
{
     FBDevShared     *shared    = dfb_fbdev->shared;
     const VideoMode *videomode = shared->modes;
     const VideoMode *highest   = NULL;

     D_DEBUG_AT( FBDev_Mode, "%s()\n", __FUNCTION__ );

     while (videomode) {
          if (videomode->xres == width && videomode->yres == height) {
               if (!highest || highest->priority < videomode->priority)
                    highest = videomode;
          }

          videomode = videomode->next;
     }

     if (!highest)
          D_ONCE( "no mode found for %dx%d", width, height );

     return highest;
}

static void adjust_destiniation_region( int layer_id, CoreLayerRegionConfig *config, int *width, int *height)
{
    DBG_LAYER_MSG("[DFB] %s, layer_id=%d, config->source.w=%d, config->source.h=%d\n",
                  __FUNCTION__, layer_id, config->source.w, config->source.h);
    if ( LAYER_UP_SCALING_CHECK( layer_id ) ) {
          *width = dfb_config->mst_layer_up_scaling_width;
          *height = dfb_config->mst_layer_up_scaling_height;
     }
     else {
          *width = config->source.w;
          *height = config->source.h;
     }
}

static DFBResult
primaryTestRegion( CoreLayer                  *layer,
                   void                       *driver_data,
                   void                       *layer_data,
                   CoreLayerRegionConfig      *config,
                   CoreLayerRegionConfigFlags *failed )
{
     CoreLayerRegionConfigFlags  fail   = CLRCF_NONE;
     VideoMode                   dummy;
     const VideoMode            *mode;
     MSTARLayerData          *slay      = layer_data;
     dfb_fbdev = dfb_all_fbdev[slay->layer_index];
     FBDevShared                *shared = dfb_fbdev->shared;
     int width, height;
     shared->layer_id = slay->layer_index;
     D_DEBUG_AT( FBDev_Primary, "%s( %dx%d, %s )\n", __FUNCTION__,
                 config->source.w, config->source.h, dfb_pixelformat_name(config->format) );
     
     if(shared->config.width == config->source.w && shared->config.height == config->source.h
        && shared->config.format == config->format && shared->config.buffermode == config->buffermode) {
         DBG_LAYER_MSG("[DFB] primaryTestRegion same config, pid:%d\n", getpid());
         return DFB_OK;
     }

     if (config->width > slay->screen_size.width || config->height> slay->screen_size.height) {
         DBG_LAYER_MSG("layer size (%d x %d) is bigger then panel size (%d x %d)\n", config->width, config->height, slay->screen_size.width, slay->screen_size.height);
         config->source.w = config->width = (slay->screen_size.width>>4)<<4;
         config->source.h = config->height = slay->screen_size.height;
     }

     adjust_destiniation_region(shared->layer_id, config, &width, &height);

     mode = dfb_fbdev_find_mode( width, height );
     if (!mode) {
          dummy = shared->current_mode;

          dummy.xres = width;
          dummy.yres = height;
          dummy.bpp  = DFB_BITS_PER_PIXEL(config->format);
          DBG_LAYER_MSG("%s, dummy xres = %d, yres = %d\n", __FUNCTION__, dummy.xres, dummy.yres);
          mode = &dummy;
     }

     if (dfb_fbdev_test_mode( mode, config ))
          fail |= CLRCF_WIDTH | CLRCF_HEIGHT | CLRCF_FORMAT | CLRCF_BUFFERMODE;

     if ( config->options & ~(DLOP_ALPHACHANNEL | DLOP_OPACITY | DLOP_SRC_COLORKEY) )
          fail |= CLRCF_OPTIONS;

     if ( config->options & (DLOP_DST_COLORKEY | DLOP_FLICKER_FILTERING) )
          fail |= CLRCF_OPTIONS;

     if( (config->options & (DLOP_ALPHACHANNEL | DLOP_OPACITY)) == (DLOP_ALPHACHANNEL | DLOP_OPACITY) )
          fail |= CLRCF_OPTIONS;

     if ( config->options & (DLOP_DEINTERLACING | DLOP_FIELD_PARITY) )
          fail |= CLRCF_OPTIONS;

     if ( config->num_clips > 0 )
          fail |= CLRCF_CLIPS;

     if ((config->source.x && !shared->fix.xpanstep) ||
         (config->source.y && !shared->fix.ypanstep && !shared->fix.ywrapstep))
          fail |= CLRCF_SOURCE;

     if (failed)
          *failed = fail;

     if (fail)
     {
          printf("[DFB] primaryTestRegion fail : %d\n", fail);
          return DFB_UNSUPPORTED;
     }

     return DFB_OK;
}

static DFBResult
primarySetRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  void                       *region_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags  updated,
                  CoreSurface                *surface,
                  CorePalette                *palette,
                  CoreSurfaceBufferLock      *left_lock, 
                  CoreSurfaceBufferLock      *right_lock )
{
     DFBResult    ret;
     MSTARLayerData    *slay   = layer_data;
     /* get layer fbdev info. */
     dfb_fbdev = dfb_all_fbdev[slay->layer_index];

     FBDevShared       *shared = dfb_fbdev->shared;
     shared->layer_id = slay->layer_index;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );
     DBG_LAYER_MSG("%s(), layer idx = %d\n", __FUNCTION__, shared->layer_id );

     if (updated & CLRCF_DEST) {
         MI_FB_Rectangle_t dispRegion;
         MI_FB_Rectangle_t dispRegionTemp;

         /* get screen location. */
         if (FBDEV_IOCTL( FBIOGET_SCREEN_LOCATION, &dispRegionTemp) < 0) {
             int erno = errno;
             printf("  => FAILED to FBIOgET_SCREEN_LOCATION!\n" );
             return errno2result( erno );
         }

         dispRegion.u16Xpos = config->dest.x;
         dispRegion.u16Ypos = config->dest.y;
         if (dfb_fbdev->shared->DLG_MODE) {
             dispRegion.u16Width = config->dest.w > dispRegionTemp.u16Width ? dispRegionTemp.u16Width : config->dest.w;
             dispRegion.u16Height = config->dest.h > dispRegionTemp.u16Height ? dispRegionTemp.u16Height : config->dest.h;
         } else {
             dispRegion.u16Width = config->dest.w;
             dispRegion.u16Height = config->dest.h;
         }

         /* set screen location. */
         if (FBDEV_IOCTL( FBIOSET_SCREEN_LOCATION, &dispRegion) < 0) {
             int erno = errno;
             D_ERROR("  => FAILED to FBIOSET_SCREEN_LOCATION!\n" );
             return errno2result( erno );
         }
     }

	 if(shared->config.width == config->source.w && shared->config.height == config->source.h
     && shared->config.format == config->format && shared->config.buffermode == config->buffermode) {
        DBG_LAYER_MSG("[DFB] primarySetRegion same config, pid:%d\n", getpid());
        return DFB_OK;
     }

     if (updated & (CLRCF_SURFACE | CLRCF_WIDTH   | CLRCF_HEIGHT | CLRCF_FORMAT  | CLRCF_BUFFERMODE) ) {
          DBG_LAYER_MSG("%s, %d, config (w=%d, h=%d), cur_var (xres=%d, yres=%d)\n",
            __FUNCTION__, __LINE__, config->source.w, config->source.h, shared->current_var.xres, shared->current_var.yres );

          /*if (config->source.w == shared->current_var.xres && config->source.h == shared->current_var.yres &&
               DFB_BITS_PER_PIXEL(config->format) == shared->current_var.bits_per_pixel ) {
               ret = dfb_fbdev_pan( config->source.x, left_lock->offset / left_lock->pitch + config->source.y, true );
               if (ret)
                    return ret;
          }
          else*/ {
               const VideoMode *mode;
               VideoMode        dummy;
               int width, height;

               DBG_LAYER_MSG( "FBDev/Mode: Setting %dx%d %s\n", config->source.w, config->source.h,
                       dfb_pixelformat_name( surface->config.format ) );

               adjust_destiniation_region(shared->layer_id, config, &width, &height);


               if (dfb_config->mst_GPU_AFBC && (shared->layer_id == 1)) {
                  MI_FB_LayerAttr_t stLayerAttr;
                  /* set AFBC*/
                  DBG_LAYER_MSG("[DFB] %s, set AFBC\n", __FUNCTION__ );
                  stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_AFBC_MODE;
                  stLayerAttr.unLayerAttrParam.bOsdAfbc = true;
                  if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                      printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
                  } 

                  /* set AFBC Crop */        
                  stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_AFBC_CROP;
                  stLayerAttr.unLayerAttrParam.stCropParams.bEnableCrop = true;
                  stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropX = 0;
                  stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropY = 0;
                  stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropWidth = width;
                  stLayerAttr.unLayerAttrParam.stCropParams.u16AfbcCropHeight = height;
                  DBG_LAYER_MSG("[DFB] %s, set layer attr AFBC crop (0, 0, %dx%d)\n", __FUNCTION__, width, height);
                  if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
                      printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
                  }
               }


               mode = dfb_fbdev_find_mode( width, height );
               if (!mode) {
                    dummy = shared->current_mode;

                    dummy.xres = width;
                    dummy.yres = height;
                    dummy.bpp  = DFB_BITS_PER_PIXEL(config->format);

                    mode = &dummy;
               }

               ret = dfb_fbdev_set_mode( mode, surface, config->source.x,
                                         left_lock->offset / left_lock->pitch + config->source.y );
               if (ret)
                    return ret;
          }
     }

     if ((updated & CLRCF_PALETTE) && palette)
          dfb_fbdev_set_palette( palette );

     /* remember configuration */
     shared->config = *config;

     return DFB_OK;
}


static DFBResult
primaryFlipRegion( CoreLayer             *layer,
                   void                  *driver_data,
                   void                  *layer_data,
                   void                  *region_data,
                   CoreSurface           *surface,
                   DFBSurfaceFlipFlags    flags,
				   CoreSurfaceBufferLock	   *left_lock,
	               CoreSurfaceBufferLock	   *right_lock)

{
     DFBResult ret;
     MSTARLayerData          *slay      = layer_data;
     MSTARDriverData         *sdrv = driver_data;
     MSTARDeviceData         *sdev = sdrv->dev;

     if (sdev->layer_zorder[slay->layer_index] < 0) {
         return DFB_OK;
     }

     /* get layer fbdev info. */
     dfb_fbdev = dfb_all_fbdev[slay->layer_index];
     dfb_fbdev->shared->layer_id = slay->layer_index;
     CoreLayerRegionConfig *config = &dfb_fbdev->shared->config;

     D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );
     DBG_LAYER_MSG("%s(), layer id=%d, left_lock->phys=%x, smem_start = %lx\n", __FUNCTION__, slay->layer_index, left_lock->phys, dfb_fbdev->shared->fix.smem_start);

     if (((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC) &&
         !dfb_config->pollvsync_after)
          dfb_screen_wait_vsync( dfb_screens_at(DSCID_PRIMARY) );

     ret = dfb_fbdev_pan( config->source.x,
                          left_lock->offset / left_lock->pitch + config->source.y,
                          (flags & DSFLIP_WAITFORSYNC) == DSFLIP_ONSYNC );
     if (ret) {
          printf("[%s %d] ret=%d errno=%d pid=%d\n", __FUNCTION__, __LINE__, ret, errno, getpid());
          return ret;
     }

     if ((flags & DSFLIP_WAIT) &&
         (dfb_config->pollvsync_after || !(flags & DSFLIP_ONSYNC)))
          dfb_screen_wait_vsync( dfb_screens_at(DSCID_PRIMARY) );

     dfb_surface_flip( surface, false );

     return DFB_OK;
}

/** fbdev internal **/

static void
dfb_fbdev_var_to_mode( const struct fb_var_screeninfo *var,
                       VideoMode                      *mode )
{
     mode->xres          = var->xres;
     mode->yres          = var->yres;
     mode->bpp           = var->bits_per_pixel;
     mode->hsync_len     = var->hsync_len;
     mode->vsync_len     = var->vsync_len;
     mode->left_margin   = var->left_margin;
     mode->right_margin  = var->right_margin;
     mode->upper_margin  = var->upper_margin;
     mode->lower_margin  = var->lower_margin;
     mode->pixclock      = var->pixclock;
     mode->hsync_high    = (var->sync & FB_SYNC_HOR_HIGH_ACT) ? 1 : 0;
     mode->vsync_high    = (var->sync & FB_SYNC_VERT_HIGH_ACT) ? 1 : 0;
     mode->csync_high    = (var->sync & FB_SYNC_COMP_HIGH_ACT) ? 1 : 0;
     mode->sync_on_green = (var->sync & FB_SYNC_ON_GREEN) ? 1 : 0;
     mode->external_sync = (var->sync & FB_SYNC_EXT) ? 1 : 0;
     mode->broadcast     = (var->sync & FB_SYNC_BROADCAST) ? 1 : 0;
     mode->laced         = (var->vmode & FB_VMODE_INTERLACED) ? 1 : 0;
     mode->doubled       = (var->vmode & FB_VMODE_DOUBLE) ? 1 : 0;
}

/*
 * pans display (flips buffer) using fbdev ioctl
 */
static DFBResult
dfb_fbdev_pan( int xoffset, int yoffset, bool onsync )
{
     struct fb_var_screeninfo *var;
     FBDevShared              *shared = dfb_fbdev->shared;

     D_DEBUG_AT( FBDev_Mode, "%s( xoffset: %d, yoffset: %d, onsync: %d )\n", __FUNCTION__, xoffset, yoffset, onsync );

     DBG_LAYER_MSG("%s( xoffset: %d, yoffset: %d, onsync: %d ) dfb_fbdev : %p\n", __FUNCTION__, xoffset, yoffset, onsync, dfb_fbdev );
     if (!shared->fix.xpanstep && !shared->fix.ypanstep && !shared->fix.ywrapstep)
          return DFB_OK;

     var = &shared->current_var;

     if (var->xres_virtual < xoffset + var->xres) {
          D_ERROR( "DirectFB/FBDev: xres %d, vxres %d, xoffset %d\n",
                    var->xres, var->xres_virtual, xoffset );
          D_BUG( "panning buffer out of range" );
          return DFB_BUG;
     }

     DBG_LAYER_MSG( "%s, %d, yres %d, vyres %d\n", __FUNCTION__, __LINE__, var->yres, var->yres_virtual );
     if (var->yres_virtual < yoffset + var->yres) {
          D_ERROR( "DirectFB/FBDev: yres %d, vyres %d, offset %d\n",
                    var->yres, var->yres_virtual, yoffset );
          D_BUG( "panning buffer out of range" );
          return DFB_BUG;
     }

     if (shared->fix.xpanstep)
          var->xoffset = xoffset - (xoffset % shared->fix.xpanstep);
     else
          var->xoffset = 0;

     if (shared->fix.ywrapstep) {
          var->yoffset = yoffset - (yoffset % shared->fix.ywrapstep);
          var->vmode |= FB_VMODE_YWRAP;
     }
     else if (shared->fix.ypanstep) {
          var->yoffset = yoffset - (yoffset % shared->fix.ypanstep);
          var->vmode &= ~FB_VMODE_YWRAP;
     }
     else {
          var->yoffset = 0;
     }

     var->activate = onsync ? FB_ACTIVATE_VBL : FB_ACTIVATE_NOW;

     DBG_LAYER_MSG("[DFB] %s, %d, call pan_display, var->bit_per_pixel = %d\n", __FUNCTION__, __LINE__, var->bits_per_pixel);

     if (FBDEV_IOCTL(FBIOPAN_DISPLAY, var) < 0) {
          D_ERROR("  => FAILED!errno=%d\n", errno );
          return errno2result( errno );
     }

     return DFB_OK;
}

/*
 * blanks display using fbdev ioctl
 */
static DFBResult
dfb_fbdev_blank( int level )
{
     if (ioctl( dfb_fbdev->fd, FBIOBLANK, level ) < 0) {
          D_PERROR( "DirectFB/FBDev: Display blanking failed!\n" );

          return errno2result( errno );
     }

     return DFB_OK;
}

static DFBResult
dfb_fbdev_mode_to_var( const VideoMode           *mode,
                       DFBSurfacePixelFormat      pixelformat,
                       unsigned int               vxres,
                       unsigned int               vyres,
                       unsigned int               xoffset,
                       unsigned int               yoffset,
                       DFBDisplayLayerBufferMode  buffermode,
                       struct fb_var_screeninfo  *ret_var )
{
     struct fb_var_screeninfo  var;
     FBDevShared              *shared = dfb_fbdev->shared;

     D_DEBUG_AT( FBDev_Mode, "%s( mode: %p )\n", __FUNCTION__, mode );

     D_ASSERT( mode != NULL );
     D_ASSERT( ret_var != NULL );

     DBG_LAYER_MSG( "%s, framebuffer_base :%p, smem_start: %lx\n", __FUNCTION__, dfb_fbdev->framebuffer_base, shared->fix.smem_start);
     DBG_LAYER_MSG( "  -> resolution   %dx%d\n", mode->xres, mode->yres );
     DBG_LAYER_MSG( "  -> virtual      %dx%d\n", vxres, vyres );
     DBG_LAYER_MSG( "  -> pixelformat  %s\n", dfb_pixelformat_name(pixelformat) );
     DBG_LAYER_MSG( "  -> buffermode   %s\n",
                 buffermode == DLBM_FRONTONLY  ? "FRONTONLY"  :
                 buffermode == DLBM_BACKVIDEO  ? "BACKVIDEO"  :
                 buffermode == DLBM_BACKSYSTEM ? "BACKSYSTEM" :
                 buffermode == DLBM_TRIPLE     ? "TRIPLE"     : "invalid!" );
     DBG_LAYER_MSG( "  -> panstep   %dx%d\n", shared->fix.ypanstep, shared->fix.ywrapstep );
     /* Start from current information */
     var              = shared->current_var;
     var.activate     = FB_ACTIVATE_NOW;

     /* Set timings */
     var.pixclock     = mode->pixclock;
     var.left_margin  = mode->left_margin;
     var.right_margin = mode->right_margin;
     var.upper_margin = mode->upper_margin;
     var.lower_margin = mode->lower_margin;
     var.hsync_len    = mode->hsync_len;
     var.vsync_len    = mode->vsync_len;

     /* Set resolution */
     var.xres         = mode->xres;
     var.yres         = mode->yres;
     var.xres_virtual = vxres;
     var.yres_virtual = vyres;

     /* Set for AFBC */
     if (dfb_config->mst_GPU_AFBC && (shared->layer_id == 1)) {
#define _MIN_SIZE 32
         var.yres_virtual += ((var.yres > _MIN_SIZE)? dfb_config->GPU_AFBC_EXT_SIZE : 0);
     }

     if (shared->fix.xpanstep)
          var.xoffset = xoffset - (xoffset % shared->fix.xpanstep);
     else
          var.xoffset = 0;

     if (shared->fix.ywrapstep)
          var.yoffset = yoffset - (yoffset % shared->fix.ywrapstep);
     else if (shared->fix.ypanstep)
          var.yoffset = yoffset - (yoffset % shared->fix.ypanstep);
     else
          var.yoffset = 0;

     /* Set buffer mode */
     switch (buffermode) {
          case DLBM_TRIPLE:
               if (shared->fix.ypanstep == 0 && shared->fix.ywrapstep == 0)
                    return DFB_UNSUPPORTED;

               var.yres_virtual *= 3;
               break;

          case DLBM_BACKVIDEO:
               if (shared->fix.ypanstep == 0 && shared->fix.ywrapstep == 0)
                    return DFB_UNSUPPORTED;

               var.yres_virtual *= 2;
               break;

          case DLBM_BACKSYSTEM:
          case DLBM_FRONTONLY:
               break;

          default:
               return DFB_UNSUPPORTED;
     }

     DBG_LAYER_MSG("%s, %d, vxres = %d, vyres = %d\n", __FUNCTION__, __LINE__, var.xres_virtual, var.yres_virtual);

     /* Set pixel format */
     var.bits_per_pixel = DFB_BITS_PER_PIXEL(pixelformat);
     var.transp.length  = var.transp.offset = 0;

     switch (pixelformat) {
          case DSPF_ARGB1555:
               var.transp.length = 1;
               var.red.length    = 5;
               var.green.length  = 5;
               var.blue.length   = 5;
               var.transp.offset = 15;
               var.red.offset    = 10;
               var.green.offset  = 5;
               var.blue.offset   = 0;
               break;

          case DSPF_RGBA5551:
               var.transp.length = 1;
               var.red.length    = 5;
               var.green.length  = 5;
               var.blue.length   = 5;
               var.transp.offset = 1;
               var.red.offset    = 11;
               var.green.offset  = 6;
               var.blue.offset   = 1;
               break;

          case DSPF_RGB555:
               var.red.length    = 5;
               var.green.length  = 5;
               var.blue.length   = 5;
               var.red.offset    = 10;
               var.green.offset  = 5;
               var.blue.offset   = 0;
               break;

          case DSPF_BGR555:
               var.red.length    = 5;
               var.green.length  = 5;
               var.blue.length   = 5;
               var.red.offset    = 0;
               var.green.offset  = 5;
               var.blue.offset   = 10;
               break;

          case DSPF_ARGB4444:
               var.transp.length = 4;
               var.red.length    = 4;
               var.green.length  = 4;
               var.blue.length   = 4;
               var.transp.offset = 12;
               var.red.offset    = 8;
               var.green.offset  = 4;
               var.blue.offset   = 0;
               break;

          case DSPF_RGBA4444:
               var.transp.length = 4;
               var.red.length    = 4;
               var.green.length  = 4;
               var.blue.length   = 4;
               var.transp.offset = 0;
               var.red.offset    = 12;
               var.green.offset  = 8;
               var.blue.offset   = 4;
               break;

         case DSPF_RGB444:
               var.red.length    = 4;
               var.green.length  = 4;
               var.blue.length   = 4;
               var.red.offset    = 8;
               var.green.offset  = 4;
               var.blue.offset   = 0;
               break;

         case DSPF_RGB32:
               var.red.length    = 8;
               var.green.length  = 8;
               var.blue.length   = 8;
               var.red.offset    = 16;
               var.green.offset  = 8;
               var.blue.offset   = 0;
               break;

          case DSPF_RGB16:
               var.red.length    = 5;
               var.green.length  = 6;
               var.blue.length   = 5;
               var.red.offset    = 11;
               var.green.offset  = 5;
               var.blue.offset   = 0;
               break;

          case DSPF_ARGB:

          case DSPF_AiRGB:
               var.transp.length = 8;
               var.red.length    = 8;
               var.green.length  = 8;
               var.blue.length   = 8;
               var.transp.offset = 24;
               var.red.offset    = 16;
               var.green.offset  = 8;
               var.blue.offset   = 0;
               break;

          case DSPF_LUT8:
          case DSPF_RGB24:
          case DSPF_RGB332:
               break;

          case DSPF_ARGB1666:
               var.transp.length = 1;
               var.red.length    = 6;
               var.green.length  = 6;
               var.blue.length   = 6;
               var.transp.offset = 18;
               var.red.offset    = 12;
               var.green.offset  = 6;
               var.blue.offset   = 0;
               break;

          case DSPF_ARGB6666:
               var.transp.length = 6;
               var.red.length    = 6;
               var.green.length  = 6;
               var.blue.length   = 6;
               var.transp.offset = 18;
               var.red.offset    = 12;
               var.green.offset  = 6;
               var.blue.offset   = 0;
               break;

          case DSPF_RGB18:
               var.red.length    = 6;
               var.green.length  = 6;
               var.blue.length   = 6;
               var.red.offset    = 12;
               var.green.offset  = 6;
               var.blue.offset   = 0;
               break;

          default:
               return DFB_UNSUPPORTED;
     }

     /* Set sync options */
     var.sync = 0;
     if (mode->hsync_high)
          var.sync |= FB_SYNC_HOR_HIGH_ACT;
     if (mode->vsync_high)
          var.sync |= FB_SYNC_VERT_HIGH_ACT;
     if (mode->csync_high)
          var.sync |= FB_SYNC_COMP_HIGH_ACT;
     if (mode->sync_on_green)
          var.sync |= FB_SYNC_ON_GREEN;
     if (mode->external_sync)
          var.sync |= FB_SYNC_EXT;
     if (mode->broadcast)
          var.sync |= FB_SYNC_BROADCAST;

     /* Set interlace/linedouble */
     var.vmode = 0;
     if (mode->laced)
          var.vmode |= FB_VMODE_INTERLACED;
     if (mode->doubled)
          var.vmode |= FB_VMODE_DOUBLE;

     *ret_var = var;

     return DFB_OK;
}

static DFBResult
dfb_fbdev_test_mode( const VideoMode             *mode,
                     const CoreLayerRegionConfig *config )
{
     DFBResult                  ret;
     struct fb_var_screeninfo   var = {0};
     unsigned int               need_mem;
     FBDevShared               *shared = dfb_fbdev->shared;
     const DFBRectangle        *source = &config->source;
     int width, height;

     D_DEBUG_AT( FBDev_Mode, "%s( mode: %p, config: %p )\n", __FUNCTION__, mode, config );

     D_ASSERT( mode != NULL );
     D_ASSERT( config != NULL );

     /* Is panning supported? */
     if (source->w != mode->xres && shared->fix.xpanstep == 0)
          return DFB_UNSUPPORTED;
     if (source->h != mode->yres && shared->fix.ypanstep == 0 && shared->fix.ywrapstep == 0)
          return DFB_UNSUPPORTED;

     adjust_destiniation_region(shared->layer_id, config, &width, &height);

     ret = dfb_fbdev_mode_to_var( mode, config->format, width, height,
                                  0, 0, config->buffermode, &var );
     DBG_LAYER_MSG( "DirectFB/FBDev: var virt(%d, %d)\n", var.xres_virtual, var.yres_virtual);
     if (ret) {
          printf("dfb_fbdev_mode_to_var ret = %d\n", ret);
          return ret;
     }
     need_mem = DFB_BYTES_PER_LINE( config->format, var.xres_virtual ) *
                DFB_PLANE_MULTIPLY( config->format, var.yres_virtual );

     if(shared->iommu == 0) {
         if (shared->fix.smem_len < need_mem) {
             D_ERROR( "  => not enough framebuffer memory (need : %u > current : %u)!\n",
                          need_mem, shared->fix.smem_len );
             return DFB_LIMITEXCEEDED;
         }
     } else {
         /* later updated by master when allocating buffer(FBIO_AllOCATE_IOMMU) */
         if(shared->fix.smem_len != need_mem) {
             shared->current_var = var;
             return DFB_OK;
         }
     }

     DBG_LAYER_MSG( "DirectFB/FBDev: Current mode's pixelformat: "
                         "rgba %d/%d, %d/%d, %d/%d, %d/%d (%dbit)\n",
                         var.red.length, var.red.offset,
                         var.green.length, var.green.offset,
                         var.blue.length, var.blue.offset,
                         var.transp.length, var.transp.offset,
                         var.bits_per_pixel );

     /* Enable test mode */
     //var.activate = FB_ACTIVATE_TEST;
     var.activate = FB_ACTIVATE_FORCE;

     dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );

     if (FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &var ) < 0) {
          int erno = errno;
          dfb_gfxcard_unlock();
          D_ERROR("  => FAILED!\n" );
          return errno2result( erno );
     }

     FBDEV_IOCTL( FBIOGET_FSCREENINFO, &shared->fix );
     DBG_LAYER_MSG( "FBDev/Mode: %s, pitch %d\n", __FUNCTION__, shared->fix.line_length );

     dfb_gfxcard_unlock();

     D_DEBUG_AT( FBDev_Mode, "  => SUCCESS\n" );

     return DFB_OK;
}

static DFBResult
dfb_fbdev_test_mode_simple( const VideoMode *mode )
{
     DFBResult                ret;
     struct fb_var_screeninfo var;

     D_DEBUG_AT( FBDev_Mode, "%s( mode: %p )\n", __FUNCTION__, mode );

     D_ASSERT( mode != NULL );

     ret = dfb_fbdev_mode_to_var( mode, dfb_pixelformat_for_depth(mode->bpp), mode->xres, mode->yres,
                                  0, 0, /*DLBM_FRONTONLY*/DLBM_BACKVIDEO, &var );
     if (ret)
          return ret;

     /* Enable test mode */
     var.activate = FB_ACTIVATE_TEST;
     //var.activate = FB_ACTIVATE_FORCE;
     if (FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &var ) < 0) {
          D_ERROR("  => FAILED!errno=%d\n", errno );
          return errno2result( errno );
     }

     DBG_LAYER_MSG("  => SUCCESS\n" );

     return DFB_OK;
}

static inline int
num_video_buffers( CoreSurface *surface )
{
      int i;

      for (i = 0; i < surface->num_buffers; i++) {
           if (surface->buffers[i]->policy == CSP_SYSTEMONLY)
                break;
      }

      return i;
}

static DFBResult
dfb_fbdev_set_mode( const VideoMode         *mode,
                    CoreSurface             *surface,
                    unsigned int             xoffset,
                    unsigned int             yoffset )
{
     DFBResult                  ret;
     int                        bufs;
     struct fb_var_screeninfo   var;
     struct fb_var_screeninfo   var2;
     FBDevShared               *shared     = dfb_fbdev->shared;
     DFBDisplayLayerBufferMode  buffermode = DLBM_FRONTONLY;
     const CoreSurfaceConfig   *config     = &surface->config;
     int width, height;

     D_DEBUG_AT( FBDev_Mode, "%s( mode: %p, config: %p )\n", __FUNCTION__, mode, config );

     D_ASSERT( mode != NULL );
     D_ASSERT( config != NULL );

     bufs = num_video_buffers( surface );
     switch (bufs) {
          case 3:
               buffermode = DLBM_TRIPLE;
               break;
          case 2:
               buffermode = DLBM_BACKVIDEO;
               break;
          case 1:
               buffermode = DLBM_FRONTONLY;
               break;
          default:
               D_BUG( "dfb_fbdev_set_mode() called with %d video buffers!", bufs );
               return DFB_BUG;
     }

     if ( LAYER_UP_SCALING_CHECK( shared->layer_id )) {
         // setting up scaling size.
         width = dfb_config->mst_layer_up_scaling_width;
         height = dfb_config->mst_layer_up_scaling_height;
     }
     else {
         // using surface config size.
         width = config->size.w;
         height = config->size.h;
     }

     ret = dfb_fbdev_mode_to_var( mode, config->format, width, height,
                                  xoffset, yoffset, buffermode, &var );
     if (ret) {
          D_ERROR( "FBDev/Mode: Failed to switch to %dx%d %s (buffermode %d)\n",
                   config->size.w, config->size.h, dfb_pixelformat_name(config->format), buffermode );
          return ret;
     }


     dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );
     if (FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &var )  < 0) {
          D_ERROR("  => FBIOPUT_VSCREENINFO failed!\n" );

          ret = errno2result( errno );
          goto error;
     }

     if (FBDEV_IOCTL( FBIOGET_VSCREENINFO, &var2 )  < 0) {
          D_ERROR("  => FBIOGET_VSCREENINFO failed!\n" );

          ret = errno2result( errno );
          goto error;
     }

     if (var.xres != var2.xres || var.xres_virtual != var2.xres_virtual ||
         var.yres != var2.yres || var.yres_virtual != var2.yres_virtual)
     {
          D_ERROR( "  => read back mismatch! (%dx%d [%dx%d] should be %dx%d [%dx%d])\n",
                      var2.xres, var2.yres, var2.xres_virtual, var2.yres_virtual,
                      var.xres, var.yres, var.xres_virtual, var.yres_virtual );

          ret = DFB_IO;
          goto error;
     }


     D_DEBUG_AT( FBDev_Mode, "  => SUCCESS\n" );


     shared->current_var = var;
     dfb_fbdev_var_to_mode( &var, &shared->current_mode );

     /* To get the new pitch */
     FBDEV_IOCTL( FBIOGET_FSCREENINFO, &shared->fix );

     DBG_LAYER_MSG( "FBDev/Mode: Switched to %dx%d (virtual %dx%d) at %d bit (%s), pitch %d\n",
             var.xres, var.yres, var.xres_virtual, var.yres_virtual, var.bits_per_pixel,
             dfb_pixelformat_name(config->format), shared->fix.line_length );

     if (config->format == DSPF_RGB332)
          dfb_fbdev_set_rgb332_palette();
     else
          dfb_fbdev_set_gamma_ramp( config->format );

     /* invalidate original pan offset */
     shared->orig_var.xoffset = 0;
     shared->orig_var.yoffset = 0;
     
     dfb_gfxcard_unlock();

     return DFB_OK;


error:
     dfb_gfxcard_unlock();

     D_ERROR( "FBDev/Mode: Failed to switched to %dx%d (virtual %dx%d) at %d bit (%s)!\n",
              var.xres, var.yres, var.xres_virtual, var.yres_virtual, var.bits_per_pixel,
              dfb_pixelformat_name(config->format) );

     return ret;
}

static void dfb_fbdev_changeSecureMode(bool bSecureStatus)
{
    FBDevShared               *shared     = dfb_fbdev->shared;
    MI_FB_LayerAttr_t stLayerAttr;
    //static bool bOldSecureStatus = false;

    //if (bOldSecureStatus == bSecureStatus ) 
        //return;

    //printf("[DFB] %s, bOldSecureStatus = %d,  bSecureStatus = %d\n", __FUNCTION__, bOldSecureStatus, bSecureStatus);
    if (bSecureStatus) {
        DBG_SECURE_MSG("[DFB] %s, set GE & GOP to secure mode!\n", __FUNCTION__);
        // set GOP secure AID.
        stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_SWITCH_SECURE_AID;
        stLayerAttr.unLayerAttrParam.eGOP_AID_Type = MI_FB_GOP_AID_S;
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set GOP SWITCH_SECURE_AID failed!\n", __FUNCTION__);
            return;  
        }

        // set GE1 secure interface 
        stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_SWITCH_INTERFACE_GE;
        stLayerAttr.unLayerAttrParam.eGEInterface = E_MI_FB_INTERFACE_GE1;
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set GE SECURE INTERFACE failed!\n", __FUNCTION__);
            return;
        }

    }
    else {
        DBG_SECURE_MSG("[DFB] %s, set GE & GOP to non-secure mode!\n", __FUNCTION__);
        // set GOP non-secure AID
        stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_SWITCH_SECURE_AID;
        stLayerAttr.unLayerAttrParam.eGOP_AID_Type = MI_FB_GOP_AID_NS;
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set GOP non-SECURE AID failed!\n", __FUNCTION__);
            return;
        }

        // set GE0 normal interface
        stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_SWITCH_INTERFACE_GE;
        stLayerAttr.unLayerAttrParam.eGEInterface = E_MI_FB_INTERFACE_GE0;
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set GE non-SECURE INTERFACE failed!\n", __FUNCTION__);
            return;
        }

    }

    //bOldSecureStatus = bSecureStatus;
}
static DFBResult
dfb_fbdev_register_output_timing(void)
{
     MI_DISP_CallbackInputParams_t stInputDispParams = {0};
     MI_DISP_CallbackOutputParams_t stOutputDispParams = {0};
     MI_RESULT u32Ret = MI_OK;
     char szMainWinName[MI_MODULE_HANDLE_NAME_LENGTH_MAX] = {0};
     int ret = snprintf(szMainWinName,sizeof(szMainWinName), "MI_DISP_HD0");
     if (ret < 0) {
        printf("[DFB] %s, snprintf fail with errno : %d\n", __FUNCTION__, errno);
        return DFB_FAILURE;
     }

     u32Ret = MI_DISP_Init(NULL);
     if (u32Ret != MI_OK && u32Ret != MI_HAS_INITED) {
          printf("[DFB] %s, MI_DISP_Init failed, Error : 0x%x\n", __FUNCTION__, u32Ret);
          return DFB_FAILURE;
     }


     MI_DISP_QueryHandleParams_t stDispQueryParams;
     memset(&stDispQueryParams, 0, sizeof(stDispQueryParams));

     stDispQueryParams.pszName = (MI_U8*)szMainWinName;
     u32Ret = MI_DISP_GetHandle(&stDispQueryParams, &hDisp);
     if(u32Ret != MI_OK)
     {
          MI_DISP_OpenParams_t stDispOpenParams;
          memset(&stDispOpenParams, 0, sizeof(stDispOpenParams));
          stDispOpenParams.pszName = (MI_U8*)szMainWinName;
          stDispOpenParams.eVidWinType = E_MI_DISP_VIDWIN_HD;
          if(MI_OK != MI_DISP_Open(&stDispOpenParams, &hDisp))
          {
               printf("%s %d DISP open fail\n",__FUNCTION__,__LINE__);
               return DFB_FAILURE;
          }
     }

     stInputDispParams.u64CallbackId = 0;
     stInputDispParams.pUserParams = NULL;
     stInputDispParams.u32EventFlags = E_MI_DISP_CALLBACK_EVENT_TIMING_CHANGE;
     stInputDispParams.pfEventCallback = (MI_DISP_EventCallback)_MI_OutputTiming_CallBack;
     u32Ret = MI_DISP_RegisterCallback(hDisp, &stInputDispParams, &stOutputDispParams);
     if(u32Ret != MI_OK)
     {
          printf("MI_DISP_RegisterCallback fail!!! ret = 0x%x\n", u32Ret);
          return DFB_FAILURE;
     }

     DBG_LAYER_MSG("[DFB][%s %d] (pid = %d)\n", __FUNCTION__, __LINE__,  getpid());

     return DFB_OK;
}


static DFBResult
dfb_fbdev_unregister_output_timing(void)
{
     MI_RESULT u32Ret = MI_OK;

     MI_DISP_CallbackInputParams_t stInputDispParams = {0};
     stInputDispParams.u64CallbackId = 0;
     stInputDispParams.pUserParams = NULL;
     stInputDispParams.u32EventFlags = E_MI_DISP_CALLBACK_EVENT_TIMING_CHANGE;
     stInputDispParams.pfEventCallback = (MI_DISP_EventCallback)_MI_OutputTiming_CallBack;
     u32Ret = MI_DISP_UnRegisterCallback(hDisp, &stInputDispParams);
     if(u32Ret != MI_OK)
     {
          printf("MI_DISP_UnRegisterCallback fail!!! ret = 0x%x\n", u32Ret);
          return DFB_FAILURE;
     }

     return DFB_OK;
}

#if 0
/*
 * parses video modes in /etc/fb.modes and stores them in dfb_fbdev->shared->modes
 * (to be replaced by DirectFB's own config system
 */
static DFBResult
dfb_fbdev_read_modes( void )
{
     FILE        *fp;
     char         line[80],label[32],value[16];
     int          geometry=0, timings=0;
     int          dummy;
     VideoMode    temp_mode;
     FBDevShared *shared = dfb_fbdev->shared;
     VideoMode   *prev   = shared->modes;

     D_DEBUG_AT( FBDev_Mode, "%s()\n", __FUNCTION__ );

     if (!(fp = fopen("/etc/fb.modes","r")))
          return errno2result( errno );

     while (fgets(line,79,fp)) {
          if (sscanf(line, "mode \"%31[^\"]\"",label) == 1) {
               memset( &temp_mode, 0, sizeof(VideoMode) );

               geometry = 0;
               timings = 0;

               while (fgets(line,79,fp) && !(strstr(line,"endmode"))) {
                    if (5 == sscanf(line," geometry %d %d %d %d %d", &temp_mode.xres, &temp_mode.yres, &dummy, &dummy, &temp_mode.bpp)) {
                         geometry = 1;
                    }
                    else if (7 == sscanf(line," timings %d %d %d %d %d %d %d", &temp_mode.pixclock, &temp_mode.left_margin,  &temp_mode.right_margin,
                                         &temp_mode.upper_margin, &temp_mode.lower_margin, &temp_mode.hsync_len,    &temp_mode.vsync_len)) {
                         timings = 1;
                    }
                    else if (1 == sscanf(line, " hsync %15s",value) && 0 == strcasecmp(value,"high")) {
                         temp_mode.hsync_high = 1;
                    }
                    else if (1 == sscanf(line, " vsync %15s",value) && 0 == strcasecmp(value,"high")) {
                         temp_mode.vsync_high = 1;
                    }
                    else if (1 == sscanf(line, " csync %15s",value) && 0 == strcasecmp(value,"high")) {
                         temp_mode.csync_high = 1;
                    }
                    else if (1 == sscanf(line, " laced %15s",value) && 0 == strcasecmp(value,"true")) {
                         temp_mode.laced = 1;
                    }
                    else if (1 == sscanf(line, " double %15s",value) && 0 == strcasecmp(value,"true")) {
                         temp_mode.doubled = 1;
                    }
                    else if (1 == sscanf(line, " gsync %15s",value) && 0 == strcasecmp(value,"true")) {
                         temp_mode.sync_on_green = 1;
                    }
                    else if (1 == sscanf(line, " extsync %15s",value) && 0 == strcasecmp(value,"true")) {
                         temp_mode.external_sync = 1;
                    }
                    else if (1 == sscanf(line, " bcast %15s",value) && 0 == strcasecmp(value,"true")) {
                         temp_mode.broadcast = 1;
                    }
               }

               if (geometry && timings && !dfb_fbdev_test_mode_simple(&temp_mode)) {
                    VideoMode *mode = SHCALLOC( shared->shmpool, 1, sizeof(VideoMode) );
                    if (!mode) {
                         D_OOSHM();
                         continue;
                    }

                    if (!prev)
                         shared->modes = mode;
                    else
                         prev->next = mode;

                    direct_memcpy (mode, &temp_mode, sizeof(VideoMode));

                    prev = mode;

                    D_DEBUG_AT( FBDev_Mode, " +-> %16s %4dx%4d  %s%s\n", label, temp_mode.xres, temp_mode.yres,
                                temp_mode.laced ? "interlaced " : "", temp_mode.doubled ? "doublescan" : "" );
               }
          }
     }

     fclose (fp);

     return DFB_OK;
}
#endif

/*
 * some fbdev drivers use the palette as gamma ramp in >8bpp modes, to have
 * correct colors, the gamme ramp has to be initialized.
 */

static u16
dfb_fbdev_calc_gamma(int n, int max)
{
     int ret = 65535 * n / max;
     return CLAMP( ret, 0, 65535 );
}

static DFBResult
dfb_fbdev_set_gamma_ramp( DFBSurfacePixelFormat format )
{
     int i;

     int red_size   = 0;
     int green_size = 0;
     int blue_size  = 0;
     int red_max    = 0;
     int green_max  = 0;
     int blue_max   = 0;

     struct fb_cmap *cmap;

     D_DEBUG_AT( FBDev_Mode, "%s()\n", __FUNCTION__ );

     if (!dfb_fbdev) {
          D_BUG( "dfb_fbdev_set_gamma_ramp() called while dfb_fbdev == NULL!" );

          return DFB_BUG;
     }

     switch (format) {
          case DSPF_ARGB1555:
          case DSPF_RGBA5551:
          case DSPF_RGB555:
          case DSPF_BGR555:
               red_size   = 32;
               green_size = 32;
               blue_size  = 32;
               break;
          case DSPF_ARGB4444:
          case DSPF_RGBA4444:
          case DSPF_RGB444:
               red_size   = 16;
               green_size = 16;
               blue_size  = 16;
               break;
          case DSPF_RGB16:
               red_size   = 32;
               green_size = 64;
               blue_size  = 32;
               break;
          case DSPF_RGB24:
          case DSPF_RGB32:
          case DSPF_ARGB:
               red_size   = 256;
               green_size = 256;
               blue_size  = 256;
               break;
          default:
               return DFB_OK;
     }

     /*
      * ++Tony: The gamma ramp must be set differently if in DirectColor,
      *         ie, to mimic TrueColor, index == color[index].
      */
     if (dfb_fbdev->shared->fix.visual == FB_VISUAL_DIRECTCOLOR) {
          red_max   = 65536 / (256/red_size);
          green_max = 65536 / (256/green_size);
          blue_max  = 65536 / (256/blue_size);
     }
     else {
          red_max   = red_size;
          green_max = green_size;
          blue_max  = blue_size;
     }

     cmap = &dfb_fbdev->shared->current_cmap;

     /* assume green to have most weight */
     cmap->len = green_size;

     for (i = 0; i < red_size; i++)
          cmap->red[i] = dfb_fbdev_calc_gamma( i, red_max );

     for (i = 0; i < green_size; i++)
          cmap->green[i] = dfb_fbdev_calc_gamma( i, green_max );

     for (i = 0; i < blue_size; i++)
          cmap->blue[i] = dfb_fbdev_calc_gamma( i, blue_max );

     /* ++Tony: Some drivers use the upper byte, some use the lower */
     if (dfb_fbdev->shared->fix.visual == FB_VISUAL_DIRECTCOLOR) {
          for (i = 0; i < red_size; i++)
               cmap->red[i] |= cmap->red[i] << 8;

          for (i = 0; i < green_size; i++)
               cmap->green[i] |= cmap->green[i] << 8;

          for (i = 0; i < blue_size; i++)
               cmap->blue[i] |= cmap->blue[i] << 8;
     }

     if (FBDEV_IOCTL( FBIOPUTCMAP, cmap ) < 0) {
          D_PERROR( "DirectFB/FBDev: "
                     "Could not set gamma ramp" );

          return errno2result(errno);
     }

     return DFB_OK;
}

static DFBResult
dfb_fbdev_set_palette( CorePalette *palette )
{
     int             i;
     struct fb_cmap *cmap = &dfb_fbdev->shared->current_cmap;

     D_ASSERT( palette != NULL );

     cmap->len = palette->num_entries <= 256 ? palette->num_entries : 256;

     for (i = 0; i < (int)cmap->len; i++) {
          cmap->red[i]     = palette->entries[i].r;
          cmap->green[i]   = palette->entries[i].g;
          cmap->blue[i]    = palette->entries[i].b;
          //cmap->transp[i]  = 0xff - palette->entries[i].a;
		  cmap->transp[i]  = palette->entries[i].a;

          cmap->red[i]    |= cmap->red[i] << 8;
          cmap->green[i]  |= cmap->green[i] << 8;
          cmap->blue[i]   |= cmap->blue[i] << 8;
          cmap->transp[i] |= cmap->transp[i] << 8;
     }

     if (FBDEV_IOCTL( FBIOPUTCMAP, cmap ) < 0) {
          D_PERROR( "DirectFB/FBDev: Could not set the palette!\n" );

          return errno2result(errno);
     }

     return DFB_OK;
}

static DFBResult
dfb_fbdev_set_rgb332_palette( void )
{
     DFBResult ret = DFB_OK;
     int red_val;
     int green_val;
     int blue_val;
     int i = 0;
     FusionSHMPoolShared *pool = dfb_fbdev->shared->shmpool_data;

     struct fb_cmap cmap;

     if (!dfb_fbdev) {
          D_BUG( "dfb_fbdev_set_rgb332_palette() called while dfb_fbdev == NULL!" );

          return DFB_BUG;
     }

     cmap.start  = 0;
     cmap.len    = 256;
     cmap.red    = (u16*)SHMALLOC( pool, 2 * 256 );
     if (!cmap.red) {
          return D_OOSHM();
     }
     cmap.green  = (u16*)SHMALLOC( pool, 2 * 256 );
     if (!cmap.green) {
          ret = D_OOSHM();
          goto free_red;
     }
     cmap.blue   = (u16*)SHMALLOC( pool, 2 * 256 );
     if (!cmap.blue) {
          ret = D_OOSHM();
          goto free_green;
     }
     cmap.transp = (u16*)SHMALLOC( pool, 2 * 256 );
     if (!cmap.transp) {
          ret = D_OOSHM();
          goto free_blue;
     }

     for (red_val = 0; red_val  < 8 ; red_val++) {
          for (green_val = 0; green_val  < 8 ; green_val++) {
               for (blue_val = 0; blue_val  < 4 ; blue_val++) {
                    cmap.red[i]    = dfb_fbdev_calc_gamma( red_val, 7 );
                    cmap.green[i]  = dfb_fbdev_calc_gamma( green_val, 7 );
                    cmap.blue[i]   = dfb_fbdev_calc_gamma( blue_val, 3 );
                    cmap.transp[i] = (i ? 0x2000 : 0xffff);
                    i++;
               }
          }
     }

     if (FBDEV_IOCTL( FBIOPUTCMAP, &cmap ) < 0) {
          D_PERROR( "DirectFB/FBDev: "
                     "Could not set rgb332 palette" );
          ret = errno2result(errno);
          goto free_transp;
     }

 free_transp:
     SHFREE( pool, cmap.transp );
 free_blue:
     SHFREE( pool, cmap.blue );
 free_green:
     SHFREE( pool, cmap.green );
 free_red:
     SHFREE( pool, cmap.red );

     return ret;
}

#define IOVA_ADDRESS_START                     0x200000000

static u32 pipeline_id = 0;

static FusionCallHandlerResult
fbdev_ioctl_call_handler( int           caller,
                          int           call_arg,
                          void         *call_ptr,
                          void         *ctx,
                          unsigned int  serial,
                          int          *ret_val )
{
     int        ret = 0;
     //const char cursoroff_str[] = "\033[?1;0;0c";
     //const char blankoff_str[] = "\033[9;0]";
     FbdevCallParameter *param = call_ptr;
     int layerid = param->fb_id;
     bool bSecure = dfb_all_fbdev[layerid]->shared->bSecure;
     bool bSecure_current = dfb_all_fbdev[layerid]->shared->bSecure_current;

     DBG_LAYER_MSG("[DFB] %s, layer id=%d, fd=%d, call_arg=0x%x, ret=%d, pid=%d\n", __FUNCTION__, layerid, dfb_all_fbdev[layerid]->fd, call_arg, ret, getpid());

     switch( call_arg )
     {
     case FBIO_FREE_IOMMU :
         if (bSecure_current) {
             u64 phys = (dfb_all_fbdev[layerid]->shared->iommu)? (IOVA_ADDRESS_START | dfb_all_fbdev[layerid]->shared->fix.smem_start) : dfb_all_fbdev[layerid]->shared->fix.smem_start;
             if (!dfb_MMA_Secure_UnlockBuf("disp_fbdev", phys)) {
                 D_ERROR("[DFB][%s] dfb_MMA_Secure_UnlockBuf failed!\n", __FUNCTION__);
             }
             else {         
                DBG_SECURE_MSG("[DFB][%s] dfb_MMA_Secure_UnlockBuf success!\n", __FUNCTION__);
             }
         }

         if (*(long*)param->arg > 0) {
             // in master process, unmap before free.
             DBG_LAYER_MSG("[DFB] %s, unMapping, framebuffer_base = %p, smem_len = %x\n", __FUNCTION__,
                           dfb_all_fbdev[layerid]->framebuffer_base, dfb_all_fbdev[layerid]->shared->fix.smem_len );

             CHECK_RET_IOCTL( ioctl( dfb_all_fbdev[layerid]->fd, call_arg, param->arg ), ret );

             // reset fix
             dfb_all_fbdev[layerid]->shared->fix.smem_start = 0;
             dfb_all_fbdev[layerid]->shared->fix.smem_len = 0;
         }

         break;


     case  FBIO_AllOCATE_IOMMU :
         if (*(long*)param->arg > 0) {
             // in master process, after allocate iommu mem, set new var, reinit osd, and mapping.
             CHECK_RET_IOCTL( ioctl( dfb_all_fbdev[layerid]->fd, call_arg, param->arg ), ret );

             // FBIO_MIOSD_INIT
             CHECK_RET_IOCTL( ioctl( dfb_all_fbdev[layerid]->fd, FBIO_MIOSD_REINIT ), ret );

             // set new var for new fbdev buffer
             CHECK_RET_IOCTL( ioctl( dfb_all_fbdev[layerid]->fd, FBIOPUT_VSCREENINFO, &dfb_all_fbdev[layerid]->shared->current_var), ret );

             // get new buffer address from iommu allocate.
             CHECK_RET_IOCTL( ioctl( dfb_all_fbdev[layerid]->fd, FBIOGET_FSCREENINFO, &dfb_all_fbdev[layerid]->shared->fix ), ret );

             // memset before secure lock because CPU cannot touch the locked buffer
             dfb_all_fbdev[layerid]->framebuffer_base = fbdev_mpool_mapping( dfb_all_fbdev[layerid]->shared->fix.smem_start, dfb_all_fbdev[layerid]->shared->fix.smem_len, dfb_all_fbdev[layerid]->shared->iommu );
             if (dfb_all_fbdev[layerid]->framebuffer_base != NULL) {
                 memset(dfb_all_fbdev[layerid]->framebuffer_base, 0, dfb_all_fbdev[layerid]->shared->fix.smem_len);
                 dfb_MPool_UnMapping( dfb_all_fbdev[layerid]->framebuffer_base, dfb_all_fbdev[layerid]->shared->fix.smem_len );
                 dfb_all_fbdev[layerid]->framebuffer_base = NULL;
             }
         }

         if (bSecure) {
             // get pipe line id.
             if (!dfb_MMA_Secure_getPipeLineId(&pipeline_id)) {
                 printf("[DFB] %s, get pipeline id failed\n", __FUNCTION__);
             }
             else {
                 u64 phys = (dfb_all_fbdev[layerid]->shared->iommu)? (IOVA_ADDRESS_START | dfb_all_fbdev[layerid]->shared->fix.smem_start) : dfb_all_fbdev[layerid]->shared->fix.smem_start;
                 printf("[DFB] %s, pipeline_id = %x, buf phys : %llx, len = %x\n", __FUNCTION__, pipeline_id, phys, dfb_all_fbdev[layerid]->shared->fix.smem_len);
                 // lock buffer
                 if (!dfb_MMA_Secure_LockBuf("disp_fbdev", phys, dfb_all_fbdev[layerid]->shared->fix.smem_len, pipeline_id)) {
                     printf("[DFB][%s] dfb_MMA_Secure_LockBuf failed!\n", __FUNCTION__);
                 }
                 else {
                     dfb_all_fbdev[layerid]->shared->bSecure_current = dfb_all_fbdev[layerid]->shared->bSecure;
                     DBG_SECURE_MSG("[DFB][%s] dfb_MMA_Secure_LockBuf success!\n", __FUNCTION__);
                 }
             }
         }

         dfb_fbdev_changeSecureMode(bSecure);
         
         break;
     }

     *ret_val = ret;

     return FCHR_RETURN;
}

static int
fbdev_ioctl( int request, void *arg, int arg_size )
{
     int          ret;
     int          erno;

     FBDevShared *shared;
     D_ASSERT( dfb_fbdev != NULL );
     shared = dfb_fbdev->shared;
     D_ASSERT( shared != NULL );

     DBG_LAYER_MSG("[DFB] fbdev_ioctl, %d, call fbdev ioctl, request: %x, arg : %p, pid=%d\n", __LINE__, request, arg, getpid());
     
     ret = ioctl( dfb_all_fbdev[shared->layer_id]->fd, request, arg );
     if (ret != 0) {
          printf("[DFB][%s %d] errno=%d\n",__FUNCTION__, __LINE__, errno);
          printf("[DFB][%s %d] request=%x arg=%p layer_id=%d pid=%d\n",__FUNCTION__, __LINE__, request, arg, shared->layer_id, getpid());
     }

     errno = ret;
     return errno ? -1 : 0;
}

static void
_mstarSetSurfInfoSlotFree( MSTARDriverData  *sdrv,
                           int               slotID )
{

     D_ASSERT(sdrv && slotID>=0 && slotID<MSTARGFX_MAX_LAYER_BUFFER);

     sdrv->mstarLayerBuffer[slotID].u16SlotUsed = 0;
     sdrv->mstarLayerBuffer[slotID].u8GWinID = 0xee;

     //printf("clear surf slot %d \n", slotID);
}

static u8
_mstarFindFBIdbyGWinID( MSTARDriverData    *sdrv,
                        u8                  u8GWindID,
                        u16                *u16SlotID,
                        CoreSurface       **pSurface)
{

    int i;

    for(i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        if( sdrv->mstarLayerBuffer[i].u16SlotUsed &&
            sdrv->mstarLayerBuffer[i].u8GWinID == u8GWindID )
        {
            //printf("~!~ find FBID %d by GWinID%d ,index [%d]\n",sdrv->mstarLayerBuffer[i].u16GOPFBID,u8GWindID,i);
            *u16SlotID = i;
            *pSurface = sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface;

            return sdrv->mstarLayerBuffer[i].u16GOPFBID;
        }
    }

    return 0xff;
}

static MI_FB_ColorFmt_e
_mstarDFBFmt2MSFmt( DFBSurfacePixelFormat format )
{
    switch (format) {
        case DSPF_ARGB1555:
                return E_MI_FB_COLOR_FMT_ARGB1555;

        case DSPF_ARGB:
                return E_MI_FB_COLOR_FMT_ARGB8888;

        /*case DSPF_LUT8:
                return E_MS_FMT_I8;*/

        case DSPF_ARGB4444:
                return E_MI_FB_COLOR_FMT_ARGB4444;

        case DSPF_RGB16:
                return E_MI_FB_COLOR_FMT_RGB565;

        /*case DSPF_AYUV:
                return E_MS_FMT_AYUV8888;*/

        case DSPF_YVYU:
                return E_MI_FB_COLOR_FMT_YUV422;

        case DSPF_UYVY:
                return E_MI_FB_COLOR_FMT_YUV422;

        case DSPF_YUY2:
                return E_MI_FB_COLOR_FMT_YUV422;

        /*case DSPF_BLINK12355:
                return E_MS_FMT_1ABFgBg12355;

        case DSPF_BLINK2266:
                return E_MS_FMT_FaBaFgBg2266;*/
        default:
                return E_MI_FB_COLOR_FMT_INVALID;
    }
}

static DFBResult
_mstarAddRegion( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 void                       *region_data,
                 CoreLayerRegionConfig      *config )
{

    MSTARRegionData *sreg = region_data;
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;

    D_DEBUG_AT( FBDev_Primary, "%s()\n", __FUNCTION__ );

    D_MAGIC_SET( sreg, MSTARRegionData );

    sreg->config = *config;
    sreg->config_dirtyFlag = (CLRCF_ALL & ~CLRCF_COLORSPACE); // new GOP not support tile moe.
    sdev->layer_refcnt[slay->layer_index]++;

    return DFB_OK;
}

static DFBResult
mstarAddRegion( CoreLayer                   *layer,
                void                        *driver_data,
                void                        *layer_data,
                void                        *region_data,
                CoreLayerRegionConfig       *config )
{

    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    int ret;

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);

    D_ASSERT(shm_pool);
    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if(!parameter)
        return DFB_NOSHAREDMEMORY;

    parameter->reg_data = region_data;
    memcpy(&parameter->config,config,sizeof(*config));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_ADD_REGION, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(config, &parameter->config, sizeof(*config));
    SHFREE(shm_pool, parameter);

    return ret;
}

static DFBResult
_mstarRemoveRegion( CoreLayer       *layer,
                    void            *driver_data,
                    void            *layer_data,
                    void            *region_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;

    D_ASSERT( sdev != NULL );
    D_ASSERT( slay != NULL );

    D_ASSERT( slay->layer_index >= 0 );
    D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    fusion_skirmish_prevail( &sdev->beu_lock );
    DBG_LAYER_MSG("[DFB]  %s(%d), slay->layer_index:%d\n", __FUNCTION__, __LINE__, slay->layer_index);

    sdev->layer_refcnt[slay->layer_index]--;

    if((sdev->layer_refcnt[slay->layer_index] <= 0) &&
       (0xff != sdev->layer_gwin_id[slay->layer_index]))
    {
        u16 FBId, slotID;
        CoreSurface *surface= NULL;

        dfb_fbdev = dfb_all_fbdev[slay->layer_index];
        //FBIOSET_SHOW, to disable gwin.
        unsigned char bShown = false;
        if (FBDEV_IOCTL( FBIOSET_SHOW, &bShown)<0) {
            D_PERROR( "DirectFB/FBDev: "
                    "Error: failed to FBIOSET_SHOW\n" );
        }

        if( sdev->layer_zorder[slay->layer_index] > 0)
            sdev->layer_zorder[slay->layer_index] = 0;

        do
        {
            FBId = _mstarFindFBIdbyGWinID( sdrv,
                                           sdev->layer_gwin_id[slay->layer_index],
                                           &slotID,
                                           &surface);
            if(FBId != 0xff)
            {
                _mstarSetSurfInfoSlotFree(sdrv,slotID);
            }
        }
        while(FBId!=0xff);

        sdev->layer_gwin_id[slay->layer_index] = 0xff;
        sdev->layer_active[slay->layer_index] = false;
    }


    fusion_skirmish_dismiss( &sdev->beu_lock );
    return DFB_OK;
}


static DFBResult
mstarRemoveRegion( CoreLayer    *layer,
                   void         *driver_data,
                   void         *layer_data,
                   void         *region_data )

{
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    int ret;

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if(!parameter)
        return DFB_NOSHAREDMEMORY;

    parameter->reg_data = region_data;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_REMOVE_REGION, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);

        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;
}

#define GOP_MIU_MASK 0x80000000

static DFBResult
mstar_primaryAllocateSurface( CoreLayer                 *layer,
                              void                      *driver_data,
                              void                      *layer_data,
                              void                      *region_data,
                              CoreLayerRegionConfig     *config,
                              CoreSurface              **ret_surface )
{
    CoreSurfaceConfig conf;
    DFBResult ret;

    MSTARDriverData *sdrv = driver_data;
    //MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];
    FBDevShared       *shared = dfb_fbdev->shared;

    CoreSurface *surface;
    int i, number_buffers;

    CoreSurfaceBufferLock ret_lock;

    int freeSurfInfoSlot;
    u8 u8FBID, u8GOP_Ret;

    CoreSurfaceTypeFlags cstf_flag = 0;

    u32 u32Phys = 0;
    u32 u32_gop_index = 0;
    bool tryAgain = false;

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;
    if(_mstarDFBFmt2MSFmt(config->format) == E_MI_FB_COLOR_FMT_INVALID)
        return DFB_FAILURE;

    DBG_LAYER_MSG ("[DFB] %s , w =%d, h =%d, format=%s, layer_id = %d\n",
        __FUNCTION__, config->width, config->height, dfb_pixelformat_name(config->format), slay->layer_index);

    memset(&conf, 0, sizeof(CoreSurfaceConfig));

    conf.flags  = CSCONF_SIZE | CSCONF_FORMAT | CSCONF_CAPS;
    conf.size.w = config->width;
    conf.size.h = config->height;
    conf.format = config->format;
    conf.caps   = DSCAPS_VIDEOONLY;

    if (config->buffermode != DLBM_FRONTONLY)
    {
        if(config->buffermode & DLBM_TRIPLE)
            conf.caps |= DSCAPS_TRIPLE;
        else
            conf.caps |= DSCAPS_DOUBLE;
    }

    D_INFO("[DFB] %s, surface_caps = %x\n", __FUNCTION__, config->surface_caps);

    shared->bSecure_current = shared->bSecure;
    /* for secure path */
    if (config->surface_caps & DSCAPS_SECURE_MODE)
    {
        conf.caps |= DSCAPS_SECURE_MODE;
        shared->bSecure = true;
    }
    else
    {
        conf.caps &= ~DSCAPS_SECURE_MODE;
        shared->bSecure = false;
    }

    D_INFO("[DFB] %s, current secure mode = %s\n", __FUNCTION__, shared->bSecure_current ? "Secure" : "Non-Secure");
    D_INFO("[DFB] %s, new secure mode = %s\n", __FUNCTION__, shared->bSecure ? "Secure" : "Non-Secure");

    cstf_flag = CSTF_LAYER;

    if (dfb_config->mst_GPU_AFBC) {
        if (layer->shared->layer_id == 1) /* check GOP width AFBC ? */
            cstf_flag |= CSTF_AFBC;
    }

    /* allocate the temp surface to other mem pool, not layer pool. */
    if ( LAYER_UP_SCALING_CHECK( layer->shared->layer_id ) &&
        (dfb_config->mst_layer_up_scaling_height != config->height) )
        cstf_flag |= CSTF_WINDOW;

#if 1
    if (dfb_config->mst_gop_miu_setting & GOP_MIU_MASK )
    {
        u32_gop_index= dfb_config->mst_gop_available[layer->shared->layer_id];
        if(dfb_config->mst_gop_miu_setting & (0x1L<<u32_gop_index))
        {
            if (dfb_config->mst_gop_miu2_setting_extend & GOP_MIU_MASK )
            {
                if(dfb_config->mst_gop_miu2_setting_extend & (0x1L<<u32_gop_index))
                    cstf_flag |= CSTF_MIU2;
                else
                    cstf_flag |= CSTF_MIU1;
            }
            else
            {
                cstf_flag |= CSTF_MIU1;
            }
        }
        else
        {
            cstf_flag |= CSTF_MIU0;
        }
    }
#else
    u32_gop_index = dfb_config->mst_gop_available[layer->shared->layer_id];
    if(0==MApi_GOP_GetMIUSel(u32_gop_index))
    {
    cstf_flag |= CSTF_MIU0;
    }
    else
    {
    cstf_flag |= CSTF_MIU1;
    }

#endif

    DBG_LAYER_MSG("%s, layer_id=%d\n", __FUNCTION__, layer->shared->layer_id);
    ret = dfb_surface_create( layer->core,
                              &conf,
                              cstf_flag,
                              (unsigned long)layer->shared->layer_id,
                              NULL,
                              ret_surface );
    if(ret)
    {
        D_ERROR("\n[DFB] star_primaryAllocateSurface failed-->%d\n", ret);
        return ret;
    }

    surface = *ret_surface;

    if(conf.caps & DSCAPS_TRIPLE)
    {
        number_buffers = 3;
    }
    else if(conf.caps & DSCAPS_DOUBLE)
    {
        number_buffers = 2;
    }
    else
    {
        number_buffers = 1;
    }

TRY_AGAIN:
    for(i=0; i<number_buffers; i++)
    {
        CoreSurfaceBufferRole bufferRole;

        switch(i)
        {
            case 0:
                    bufferRole = CSBR_IDLE;
                    break;
            case 1:
                    bufferRole = CSBR_BACK;
                    break;
            case 2:
                    bufferRole = CSBR_FRONT;
                    break;
            default:
                    D_ASSERT(0);
                    goto FAILED_FREE_SURFACE;
        }
    }

    return DFB_OK;
    FAILED_FREE_SURFACE:
    /* Unlink from structure. */

    dfb_surface_unlink( ret_surface );
    return DFB_FAILURE;

}

static DFBResult
mstar_primaryReallocateSurface( CoreLayer                   *layer,
                                void                        *driver_data,
                                void                        *layer_data,
                                void                        *region_data,
                                CoreLayerRegionConfig       *config,
                                CoreSurface                 *surface )
{
    //MSTARDriverData *sdrv = driver_data;
    //MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];
    FBDevShared       *shared = dfb_fbdev->shared;

    DFBResult         ret;
    CoreSurfaceConfig conf;

    int i = 0, number_buffers = 0;

    CoreSurfaceBufferLock ret_lock;

    u32 u32Phys = 0;
    bool tryAgain = false;

    DBG_LAYER_MSG ("[DFB] %s (%d), w =%d, h =%d, layer_id = %d\n", __FUNCTION__, __LINE__ , config->width, config->height, slay->layer_index );
if(dfb_config->mst_null_display_driver)
    return DFB_OK;
    memset(&conf, 0, sizeof(CoreSurfaceConfig));

    conf.flags  = CSCONF_SIZE | CSCONF_FORMAT | CSCONF_CAPS;
    conf.size.w = config->width;
    conf.size.h = config->height;
    conf.format = config->format;
    conf.caps   = DSCAPS_VIDEOONLY;

    if (config->buffermode != DLBM_FRONTONLY)
    {
        if(config->buffermode & DLBM_TRIPLE)
            conf.caps |= DSCAPS_TRIPLE;
        else
            conf.caps |= DSCAPS_DOUBLE;
    }

    D_INFO("[DFB] %s, surface_caps = %x\n", __FUNCTION__, config->surface_caps);
    /* for secure path */
    if (config->surface_caps & DSCAPS_SECURE_MODE)
    {
        conf.caps |= DSCAPS_SECURE_MODE;
        shared->bSecure = true;
    }
    else
    {
        conf.caps &= ~DSCAPS_SECURE_MODE;
        shared->bSecure = false;
    }

    if(!memcmp(&conf, &surface->config, sizeof(CoreSurfaceConfig)))
    {
        return DFB_OK;
    }

    ret = dfb_surface_reconfig(surface, &conf);

    if(ret)
        return ret;


    if (DFB_PIXELFORMAT_IS_INDEXED(config->format) && !surface->palette) {

        DFBResult    ret;
        CorePalette *palette;

        ret = dfb_palette_create( layer->core,
                                  1 << DFB_COLOR_BITS_PER_PIXEL( config->format ),
                                  &palette );

        if (ret)
           return ret;

        if (config->format == DSPF_LUT8)
           dfb_palette_generate_rgb332_map(palette);

        dfb_surface_set_palette(surface, palette);

        dfb_palette_unref(palette);
    }

    if(conf.caps & DSCAPS_TRIPLE)
    {
        number_buffers = 3;
    }
    else if(conf.caps & DSCAPS_DOUBLE)
    {
        number_buffers = 2;
    }
    else
    {
        number_buffers = 1;
    }

TRY_AGAIN:

    for(i = 0; i < number_buffers; i++)
    {
        CoreSurfaceBufferRole bufferRole;

        switch(i)
        {
            case 0:
                    bufferRole = CSBR_FRONT;
                    break;
            case 1:
                    bufferRole = CSBR_BACK;
                    break;
            case 2:
                    bufferRole = CSBR_IDLE;
                    break;
            default:
                    D_ASSERT(0);
                    goto FAILED_RETURN;
        }
    }

    return DFB_OK;

FAILED_RETURN:
    return ret;
}

static DFBResult
mstar_primaryDeallocateSurface( CoreLayer              *layer,
                                void                   *driver_data,
                                void                   *layer_data,
                                void                   *region_data,
                                CoreSurface            *surface)
{
    //MSTARDriverData *sdrv = driver_data;

    return DFB_OK;
}

/*
* Return the z position of the layer.
*/
DFBResult
mstar_GetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                    *level )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;

    if(level)
        *level =  sdev->layer_zorder[slay->layer_index];

    return DFB_OK;
}

DFBResult
_mstarSetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    fusion_skirmish_prevail( &sdev->beu_lock );

    if (dfb_config->mst_gwin_disable)
        level = -1;

    if(sdev->layer_zorder[slay->layer_index] != level)
    {
        sdev->layer_zorder[slay->layer_index] = level;
        dfb_fbdev = dfb_all_fbdev[slay->layer_index];
        //FBIOSET_SHOW, to disable gwin.
        unsigned char bShown  = (level > 0)? true : false;
        if (FBDEV_IOCTL( FBIOSET_SHOW, &bShown )<0) {
            D_PERROR( "DirectFB/FBDev: "
                    "Error: failed to FBIOSET_SHOW\n" );
        }
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}

DFBResult mstar_SetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level )
{
    DFBResult ret = DFB_OK;
    MSTARLayerData  *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;


    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->level, &level, sizeof(level));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_LAYERLEVEL, parameter, &ret))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(&level,&parameter->level, sizeof(level));
    SHFREE(shm_pool, parameter);

    return ret;
}

DFBResult mstar_ConfigShadowLayer( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         ShadowLayerConfig      *shadowLayer_info)
{
    return DFB_OK;
}

DFBResult
_mstar_ConfigDisplayMode( CoreLayer                             *layer,
                          void                                  *driver_data,
                          void                                  *layer_data,
                          CoreSurface                           *surface,
                          CoreSurfaceBufferLock                 *lock,
                          DFBDisplayLayerDeskTopDisplayMode      display_mode )
{
    return DFB_OK;
}

DFBResult mstar_ConfigDisplayMode( CoreLayer                          *layer,
                         void                               *driver_data,
                         void                               *layer_data,
                         CoreSurface                        *surface,
                         CoreSurfaceBufferLock              *lock,
                         DFBDisplayLayerDeskTopDisplayMode   display_mode)
{
    return DFB_OK;
}

/*
* Set the Mirror Mode
*/
DFBResult
_mstar_SetHVMirrorEnable( CoreLayer              *layer,
                          void                   *driver_data,
                          void                   *layer_data,
                          bool                    HEnable,
                          bool                    VEnable )
{
    MSTARLayerData  *slay = layer_data;
    MI_FB_HVMirror_t stMI_FB_HVMirror;

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];

    stMI_FB_HVMirror.u8HMirror = HEnable;
    stMI_FB_HVMirror.u8VMirror = VEnable;

    DBG_LAYER_MSG("%s, setHV mirror, set HScale enable %d, VScale enable %d)\n", __FUNCTION__,
                 HEnable, VEnable);

    if (FBDEV_IOCTL( FBIOSET_LAYER_HVMIRROR, &stMI_FB_HVMirror) < 0) {
        int erno = errno;
        D_ERROR("  => FAILED to FBIOSET_LAYER_HVMIRROR!\n" );
        return errno2result( erno );
    }

    return DFB_OK;
}

DFBResult mstar_SetHVMirrorEnable( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         bool                    HEnable,
                         bool                    VEnable)
{
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter = NULL;
    FusionSHMPoolShared     *shm_pool = NULL;

    DFBResult ret = DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->HMirrorEnable, &HEnable, sizeof(HEnable));
    memcpy(&parameter->VMirrorEnable, &VEnable, sizeof(VEnable));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_MIRRORMODE, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(&HEnable, &parameter->HMirrorEnable, sizeof(HEnable));
    memcpy(&VEnable, &parameter->VMirrorEnable, sizeof(VEnable));

    SHFREE(shm_pool, parameter);

    return ret;
}

/*
* Set the LBCouple Mode
*/
DFBResult
_mstar_SetLBCoupleEnable( CoreLayer              *layer,
                          void                   *driver_data,
                          void                   *layer_data,
                          bool                    LBCoupleEnable)
{
    return DFB_OK;
}

DFBResult mstar_SetLBCoupleEnable( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         bool                    LBCoupleEnable)
{
    return DFB_OK;
}

/*
* Set the GOP DST ByPass Mode
*/
DFBResult
_mstar_SetGOPDstByPassEnable( CoreLayer              *layer,
                              void                   *driver_data,
                              void                   *layer_data,
                              bool                    ByPassEnable)
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    /*fusion_skirmish_prevail( &sdev->beu_lock );
    

    if(ByPassEnable)// change to Bypass mode.
    {
        //MApi_GOP_GWIN_SetGOPDst(slay->gop_index, E_GOP_DST_BYPASS);
        //slay->gop_dst = E_GOP_DST_BYPASS;
    }
    else // change to OP mode.
    {
        //MApi_GOP_GWIN_SetGOPDst(slay->gop_index, E_GOP_DST_OP0);
        //slay->gop_dst = E_GOP_DST_OP0;
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );*/

    return DFB_OK;
}

DFBResult mstar_SetGOPDstByPassEnable( CoreLayer              *layer,
                             void                   *driver_data,
                             void                   *layer_data,
                             bool                    ByPassEnable)
{
    return DFB_OK;
}


DFBResult _mstar_SetHVScale( CoreLayer              *layer,
                  void                   *driver_data,
                  void                   *layer_data,
                  void                   *region_data,
                  int                     HScale,
                  int                     VScale)
{
    MSTARLayerData  *slay = layer_data;
    MI_FB_Rectangle_t dispRegion;

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];

    dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );

    //FBIOGET_SCREEN_LOCATION
    if (FBDEV_IOCTL( FBIOGET_SCREEN_LOCATION, &dispRegion) < 0) {
        int erno = errno;
        dfb_gfxcard_unlock();
        D_DEBUG_AT( FBDev_Mode, "  => FAILED to FBIOGET_SCREEN_LOCATION!\n" );
        return errno2result( erno );
    }

    DBG_LAYER_MSG("%s, get screen location, (%d, %d, %dx%d), set HVScale (%d, %d)\n", __FUNCTION__,
                dispRegion.u16Xpos, dispRegion.u16Ypos, dispRegion.u16Width, dispRegion.u16Height, HScale, VScale);

    //FBIOSET_SCREEN_LOCATION
    dispRegion.u16Width = HScale;
    dispRegion.u16Height = VScale;

    if (FBDEV_IOCTL( FBIOSET_SCREEN_LOCATION, &dispRegion) < 0) {
        int erno = errno;
        dfb_gfxcard_unlock();
        D_ERROR("  => FAILED to FBIOSET_SCREEN_LOCATION!\n" );
        return errno2result( erno );
    }

    dfb_gfxcard_unlock();

    D_DEBUG_AT( FBDev_Mode, "  => SUCCESS\n" );

    return DFB_OK;
}

DFBResult
mstar_SetHVScale( CoreLayer              *layer,
                  void                   *driver_data,
                  void                   *layer_data,
                  void                   *region_data,
                  int                     HScale,
                  int                     VScale)
{
    MSTARLayerData  *slay = layer_data;
    CoreLayerCallParameter *parameter;
    FusionSHMPoolShared *shm_pool;
    DFBResult ret = DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if(!parameter)
        return DFB_NOSHAREDMEMORY;

    parameter->HScale = HScale;
    parameter->VScale = VScale;
    parameter->reg_data = region_data;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_GOPSCALE, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;

}

DFBResult _mstar_SetGOPDst( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 DFBDisplayLayerGOPDST           GopDst)
{
    return DFB_OK;
}

DFBResult mstar_SetGOPDst( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 DFBDisplayLayerGOPDST           GopDst)
{
    return DFB_OK;
}

DFBResult
_mstar_SetForceWrite( CoreLayer              *layer,
                      void                   *driver_data,
                      void                   *layer_data,
                      bool                    force_write )
{
    return DFB_OK;
}

DFBResult
mstar_SetForceWrite( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     bool                    force_write)
{
    MSTARLayerData          *slay = layer_data;
    MSTARDriverData         *sdrv = driver_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    DFBResult ret = DFB_OK;

    return ret;    
}

DFBResult mstar_GetGOPDst( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 DFBDisplayLayerGOPDST      *GopDst)
{
    return DFB_OK;
}

DFBResult mstar_SetBootLogoPatch( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        int                     miusel)
{
    MI_DISP_GetControllerParams_t stGetControllerParams;
    MI_HANDLE hDispController = MI_HANDLE_NULL;
    memset(&stGetControllerParams, 0x00, sizeof(MI_DISP_GetControllerParams_t));

    if (MI_DISP_GetController(&stGetControllerParams, &hDispController) != MI_OK)
    {
        MI_DISP_OpenControllerParams_t stOpenParams;
        memset(&stOpenParams, 0x00, sizeof(MI_DISP_OpenControllerParams_t));

        if (MI_DISP_OpenController(&stOpenParams, &hDispController) != MI_OK) {
            D_INFO("[DFB] closeBootLogo get MI DISP Controller failed!!!!!!\n");
            return DFB_FAILURE;
        }
    }

    D_INFO("[DFB] call MI_OSD_DisableBootLogo\n");
    MI_OSD_DisableBootLogo();
    D_INFO("[DFB] call MI_DISP_DisableBootLogo\n");
    MI_DISP_DisableBootLogo(hDispController);
    return DFB_OK;
}

DFBResult
_mstarUpdateRegion( CoreLayer                   *layer,
                    void                        *driver_data,
                    void                        *layer_data,
                    void                        *region_data,
                    CoreSurface                 *surface,
                    const DFBRegion             *update,
                    CoreSurfaceBufferLock       *lock )
{
    return DFB_OK;
}

DFBResult mstarUpdateRegion( CoreLayer               *layer,
                            void                       *driver_data,
                            void                       *layer_data,
                            void                       *region_data,
                            CoreSurface                *surface,
                            const DFBRegion            *left_update,
                            CoreSurfaceBufferLock      *left_lock,
                            const DFBRegion            *right_update,
                            CoreSurfaceBufferLock      *right_lock )
{
    return DFB_OK;
}


DFBResult
mstar_SetGOPAutoDetectBuf( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     bool                   bEnable )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    bool gop_auto_detect = bEnable;

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];

    DBG_LAYER_MSG("[DFB] %s, ST_GOP_AUTO_DETECT_BUF_INFO, bEnable=%d\n",
        __FUNCTION__, gop_auto_detect);

    if (FBDEV_IOCTL( FBIOSET_GOP_AUTO_DETECT, &gop_auto_detect) < 0) {
        int erno = errno;
        D_ERROR("  => FAILED to FBIOSET_LAYER_HVMIRROR!\n" );

        return errno2result( erno );
    }

    return DFB_OK;

}


DFBResult mstarGetScreen( CoreLayer               *layer,
                CoreScreen             **ret_screen )
{
    DFBResult ret = DFB_OK;

    MSTARDriverData *sdrv = (MSTARDriverData *)layer->driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer->layer_data;

    fusion_skirmish_prevail( &sdev->beu_lock );

    *ret_screen = sdrv->layers[slay->layer_index]->screen;

    fusion_skirmish_dismiss( &sdev->beu_lock );

    DBG_LAYER_MSG("%s, layer id =%d, screen : %p\n", __FUNCTION__, slay->layer_index, *ret_screen);

    return ret;
}

DFBResult
_mstar_ForceWriteEnable(bool bEnable)
{
    return DFB_OK;
}

FusionCallHandlerResult
mstar_layer_funcs_call_handler( int             caller,   /* fusion id of the caller */
                                int             call_arg, /* optional call parameter */
                                void           *call_ptr, /* optional call parameter */
                                void           *ctx,      /* optional handler context */
                                unsigned int    serial,
                                int            *ret_val )
{
    CoreLayerCommand        command = call_arg;
    CoreLayer              *layer = (CoreLayer *)ctx;
    CoreLayerCallParameter *parameter = call_ptr;


    //printf("\n the command is %d  pid is %d\n",command,getpid());
    D_ASSERT(layer);

    MSTARLayerData  *slay = layer->layer_data;

    switch (command)
    {
        case CLC_ADD_REGION:
        {
            MSTARRegionData *reg_data;
            reg_data = parameter->reg_data;

            *ret_val = _mstarAddRegion( layer,
                                        layer->driver_data,
                                        layer->layer_data,
                                        reg_data,
                                        &parameter->config );
        }
            break;

        /*case CLC_TEST_REGION:
        {
            CoreLayerRegionConfigFlags *flags;
            CoreLayerRegionConfig *config;

            flags = &parameter->flags;
            config = &parameter->config;

            *ret_val = _mstarTestRegion( layer,
                                         layer->driver_data,
                                         layer->layer_data,
                                         config,
                                         flags );
        }
            break;

        case CLC_SET_REGION:
        {
            CoreSurface *surface;

            surface = parameter->surface;

            *ret_val = _mstarSetRegion( layer,
                                        layer->driver_data,
                                        layer->layer_data,
                                        parameter->reg_data,
                                        &parameter->config,
                                        parameter->flags,
                                        surface,
                                        surface ? surface->palette : NULL,
                                        parameter->lock );
        }
            break;*/

        case CLC_REMOVE_REGION:
        {
            *ret_val = _mstarRemoveRegion( layer,
                                           layer->driver_data,
                                           layer->layer_data,
                                           parameter->reg_data );
        }
            break;

        case CLC_SET_LAYERLEVEL:
        {
            int *level;
            level = &parameter->level;

            *ret_val = _mstarSetLayerlevel( layer,
                                            layer->driver_data,
                                            layer->layer_data,
                                            *level );
        }
            break;

        /*case CLC_SET_COLORADJUSTMENT:
        {
            DFBColorAdjustment *adjustment;
            adjustment = &parameter->coloradjustment;

            *ret_val = _mstar_SetColorAdjustment( layer,
                                                  layer->driver_data,
                                                  layer->layer_data,
                                                  adjustment);
        }
            break;*/

        /*case CLC_FORCEWRITE_ENABLE:
        {
            _mstar_ForceWriteEnable(true);
            //printf("\n_mstar force write enable\n");
        }
            break;

        case CLC_FORCEWRITE_DISABLE:
        {
            _mstar_ForceWriteEnable(false);
            //printf("\n_mstar force write disable\n");
        }
            break;*/

        case CLC_CONFIG_DISPLAYMODE:
        {
            //DFBDisplayLayerDeskTopDisplayMode display_mode;
            //display_mode = parameter->display_mode;

            *ret_val = _mstar_ConfigDisplayMode(layer,
                                                layer->driver_data,
                                                layer->layer_data,
                                                parameter->surface,
                                                parameter->lock,
                                                parameter->display_mode);
            }
                break;

            case CLC_SET_MIRRORMODE:
            {
                bool *HMirrorEnable;
                bool *VMirrorEnable;
                HMirrorEnable = &parameter->HMirrorEnable;
                VMirrorEnable = &parameter->VMirrorEnable;

                *ret_val = _mstar_SetHVMirrorEnable( layer,
                                                     layer->driver_data,
                                                     layer->layer_data,
                                                    *HMirrorEnable,
                                                    *VMirrorEnable);
        }
            break;

        case CLC_SET_LBCOUPLEMODE:
        {
            bool *LBCoupleEnable;
            LBCoupleEnable = &parameter->LBCoupleEnable;

            *ret_val =  _mstar_SetLBCoupleEnable( layer,
                                                  layer->driver_data,
                                                  layer->layer_data,
                                                 *LBCoupleEnable);
        }
            break;

        case CLC_SET_GOPBYPASSMODE:
        {
            bool *ByPassEnable;
            ByPassEnable = &parameter->ByPassEnable;

            *ret_val = _mstar_SetGOPDstByPassEnable( layer,
                                                     layer->driver_data,
                                                     layer->layer_data,
                                                    *ByPassEnable);
        }
            break;

        case CLC_SET_GOPSCALE:
        {
            int *HScale;
            int *VScale;
            HScale = &parameter->HScale;
            VScale = &parameter->VScale;

            *ret_val = _mstar_SetHVScale( layer,
                                          layer->driver_data,
                                          layer->layer_data,
                                          parameter->reg_data,
                                         *HScale,
                                         *VScale);
        }
            break;

        /*case CLC_SET_GOPDST:
        {
            EN_GOP_DST_TYPE *GopDst;
            GopDst = &parameter->GopDst;

            *ret_val =  _mstar_SetGOPDst( layer,
                                          layer->driver_data,
                                          layer->layer_data,
                                         *GopDst );
        }
            break;*/

        /*case CLC_SET_FORCEWRITE:
        {
            bool ForceWrite;
            ForceWrite = parameter->ForceWrite;

            *ret_val = _mstar_SetForceWrite( layer,
                                             layer->driver_data,
                                             layer->layer_data,
                                             ForceWrite );
        }
            break;*/

        case CLC_UPDATE_REGION:
        {
            CoreSurface *surface;

            surface = parameter->surface;

            *ret_val = _mstarUpdateRegion( layer,
                                           layer->driver_data,
                                           layer->layer_data,
                                           parameter->reg_data,
                                           surface,
                                           &parameter->update,
                                           parameter->lock );
        }
            break;

        default:
            D_BUG( "[DFB] Error!!! Unknown  Command '%d'", command );
            *ret_val = DFB_BUG;
    }

    return FCHR_RETURN;
}

void
mstarAllSurfInfo(MSTARDriverData *sdrv)
{
    int i;
    for( i=0; i<MSTARGFX_MAX_LAYER_BUFFER; i++)
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed)
        {
            _mstarSetSurfInfoSlotFree(sdrv, i);
        }
}

DFBResult
mstar_ScaleSourceRectangle( CoreLayer              *layer,
                            void                   *layer_data,
                            int x, int y, int w, int h)
{
    MSTARLayerData  *slay = layer_data;
    MI_FB_CropInfo_t cropinfo = {0};

    dfb_fbdev = dfb_all_fbdev[slay->layer_index];

    dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );

    cropinfo.inputRect.u16Width = w;
    cropinfo.inputRect.u16Height = h;
    cropinfo.inputRect.u16Xpos = x;
    cropinfo.inputRect.u16Ypos = y;

    if ( w <= 0 || h <= 0 ||x < 0 || y < 0 )
    {
        DBG_LAYER_MSG("[DFB] %s line %d : invalid area argument! restore origin setting!\n",__FUNCTION__,__LINE__);
        cropinfo.inputRect.u16Width  = dfb_fbdev->shared->config.width;
        cropinfo.inputRect.u16Height = dfb_fbdev->shared->config.height;
        cropinfo.inputRect.u16Xpos    = 0;
        cropinfo.inputRect.u16Ypos    = 0;
    }

    cropinfo.outputRect.u16Width = dfb_fbdev->shared->current_var.xres;
    cropinfo.outputRect.u16Height = dfb_fbdev->shared->current_var.yres;
    cropinfo.outputRect.u16Xpos = 0;
    cropinfo.outputRect.u16Ypos = 0;

    DBG_LAYER_MSG("[DFB] %s : inputRect  (x = %d, y = %d, w = %d, h = %d)\n"
                 "outputRect(x = %d, y = %d, w = %d, h = %d)\n",__FUNCTION__,
                   cropinfo.inputRect.u16Xpos, cropinfo.inputRect.u16Ypos,
                   cropinfo.inputRect.u16Width, cropinfo.inputRect.u16Height,
                   cropinfo.outputRect.u16Xpos, cropinfo.outputRect.u16Ypos,
                   cropinfo.outputRect.u16Width, cropinfo.outputRect.u16Height);

    if (FBDEV_IOCTL( FBIOSET_CROP_INFO, &cropinfo) < 0)
    {
        int erno = errno;
        dfb_gfxcard_unlock();
        D_ERROR("  => FAILED to FBIOSET_CROP_INFO!\n" );
        return errno2result( erno );
    }

    dfb_gfxcard_unlock();

    D_DEBUG_AT( FBDev_Mode, "  => SUCCESS\n" );

    return DFB_OK;
}

#define MDRV_GFLIP_IOC_MAGIC       ('2')

typedef struct gflip_getvsync_data {
     int vsync_gop;              // Input
     signed long long vsync_ts;  // Output
} GFLIP_GETVSYNC_DATA, *PGFLIP_GETVSYNC_DATA;

#define MDRV_GFLIP_IOC_GETVSYNC_EX_NR  30 
#define MDRV_GFLIP_IOC_GETVSYNCEX     _IOWR(MDRV_GFLIP_IOC_MAGIC,MDRV_GFLIP_IOC_GETVSYNC_EX_NR,GFLIP_GETVSYNC_DATA)
#define VSYNC_TH 10000000
#define VSYNC_RATIO 2

DFBResult mstar_SetGOPOutputRatio( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        bool                    bEnable)
{
    MSTARLayerData  *slay = layer_data;
    dfb_fbdev = dfb_all_fbdev[slay->layer_index];

    MI_FB_LayerAttr_t stLayerAttr;
    stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_OUTPUTFPSRATIO;
    stLayerAttr.unLayerAttrParam.u8OutputFpsRatio = 1;

    if (bEnable) {
        u8 u8Ratio = 1;
        const char* gflip_path = "/dev/gflip";
        int fdSync = open(gflip_path, O_RDWR);
        if (fdSync < 0) {
            printf("[DFB] %s can't open file:%s, Errno[%d]\n", __FUNCTION__, gflip_path, errno);
            return DFB_FAILURE;
        }

        GFLIP_GETVSYNC_DATA data = {0}, data1 = {0};
        /* get first vsync time. */
        if (ioctl(fdSync, MDRV_GFLIP_IOC_GETVSYNCEX, &data) != 0) {
            printf("[DFB] %s ioctl gflip get vsync time failed\n", __FUNCTION__);
            close(fdSync);
            return DFB_FAILURE;
        }
        /* get second vsync time. */
        if (ioctl(fdSync, MDRV_GFLIP_IOC_GETVSYNCEX, &data1) != 0) {
            printf("[DFB] %s ioctl gflip get vsync time failed\n", __FUNCTION__);
            close(fdSync);
            return DFB_FAILURE;
        }

        close(fdSync);

        printf("[DFB] %s two vsync diff = %lld\n", __FUNCTION__, (data1.vsync_ts - data.vsync_ts) );
        /* calculate vsync diff is over 8ms or not. if over 8ms, ratio=1 otherwise ratio=2 */
        u8Ratio = ((data1.vsync_ts - data.vsync_ts) > VSYNC_TH)? 1 : VSYNC_RATIO;
        printf("[DFB] %s Set GOP output Ratio = %d\n", __FUNCTION__, u8Ratio);
        
        stLayerAttr.unLayerAttrParam.u8OutputFpsRatio = u8Ratio;
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
        }
    }
    else {
        /* GOP does not change. */
        printf("[%s] Reset GOP output Ratio to 1\n", __FUNCTION__);
        if (ioctl( dfb_fbdev->fd, FBIOSET_LAYER_ATTR, &stLayerAttr ) < 0) {
            printf("[DFB] %s, FBIOSET_LAYER_ATTR, set AFBC failed!\n", __FUNCTION__);
        }
    }

    return DFB_OK;
}
