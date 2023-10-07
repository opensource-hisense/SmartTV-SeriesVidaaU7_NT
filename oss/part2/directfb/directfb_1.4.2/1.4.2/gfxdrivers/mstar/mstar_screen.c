//#ifdef MSTAR_DEBUG_SCREEN
#define DIRECT_ENABLE_DEBUG
//#endif


#include <config.h>

#include <stdio.h>

#include <sys/mman.h>

#include <asm/types.h>

#include <directfb.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>


#include <MsCommon.h>
#include <drvSEM.h>
#include  <apiGFX.h>
#include  <apiGOP.h>
#include  <drvMMIO.h>

#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <drvMVOP.h>
#include <apiXC.h>
#include <apiPNL.h>

#include "mstar.h"
#include "mstar_types.h"
#include "mstar_screen.h"

D_DEBUG_DOMAIN( MSTAR_Screen, "MSTAR/Screen", "MSTAR Screen" );

/**********************************************************************************************************************/

static DFBResult
mstarInitScreen( CoreScreen           *screen,
               CoreGraphicsDevice       *device,
               void                 *driver_data,
               void                 *screen_data,
               DFBScreenDescription *description )
{
     D_DEBUG_AT( MSTAR_Screen, "%s()\n", __FUNCTION__ );

     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf( description->name, DFB_SCREEN_DESC_NAME_LENGTH, "MSTAR Screen" );

     return DFB_OK;
}

static DFBResult
mstarGetOPScreenSize( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data,
                  int        *ret_width,
                  int        *ret_height )
{
     //MSTARDriverData *sdrv = driver_data;
     MS_PNL_DST_DispInfo dstDispInfo;
     D_DEBUG_AT( MSTAR_Screen, "%s()\n", __FUNCTION__ );

     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );


    MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
    *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

     return DFB_OK;
}

static DFBResult
mstarGetIP0ScreenSize( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data,
                  int        *ret_width,
                  int        *ret_height )
{
     //MSTARDriverData *sdrv = driver_data;
     MS_XC_DST_DispInfo dstDispInfo;

     D_DEBUG_AT( MSTAR_Screen, "%s()\n", __FUNCTION__ );

     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );
     //printf("\nget ip0 screen size\n");
     MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN);
    *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

    if((*ret_width<64) || (*ret_height<64) || (*ret_width>=2048) || (*ret_height>=2048))
    {
      *ret_width  = 640;
      *ret_height = 480;
    }
     return DFB_OK;
}


static DFBResult
mstarGetVEScreenSize( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data,
                  int        *ret_width,
                  int        *ret_height )
{
     //MSTARDriverData *sdrv = driver_data;

     D_DEBUG_AT( MSTAR_Screen, "%s()\n", __FUNCTION__ );
     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );
     MS_VE_DST_DispInfo dstDispInfo;
     //printf("\nget ve screen size \n");

     MApi_VE_GetDstInfo(&dstDispInfo,sizeof(MS_VE_DST_DispInfo));

    *ret_width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
    *ret_height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
     return DFB_OK;
}


ScreenFuncs mstarOPScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetOPScreenSize
};

ScreenFuncs mstarIP0ScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetIP0ScreenSize
};

ScreenFuncs mstarVEScreenFuncs = {
     InitScreen:    mstarInitScreen,
     GetScreenSize: mstarGetVEScreenSize
};

