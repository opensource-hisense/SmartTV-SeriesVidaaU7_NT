//#ifdef MSTAR_DEBUG_LAYER
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
#include <core/surface_buffer.h>
#include <core/system.h>
#include <core/layers_internal.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>

#include <MsCommon.h>
#include  <apiGFX.h>
#include  <apiGOP.h>
#include <assert.h>

#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <apiXC.h>
#include <apiPNL.h>
#include "mstar.h"
#include "mstar_types.h"
#include "mstar_layer.h"


D_DEBUG_DOMAIN( MSTAR_Layer, "MSTAR/Layer", "MSTAR Layers" );


#define GOP_MIU_MASK 0x80000000
/**********************************************************************************************************************/

static MS_ColorFormat _mstarDFBFmt2MSFmt(DFBSurfacePixelFormat format)
{

     switch ( format )
     {
        case DSPF_ARGB1555:
               return E_MS_FMT_ARGB1555;
        case DSPF_ARGB :
               return E_MS_FMT_ARGB8888;
        case DSPF_LUT8 :
               return E_MS_FMT_I8;
        case DSPF_ARGB4444 :
               return E_MS_FMT_ARGB4444;
        case DSPF_RGB16:
               return E_MS_FMT_RGB565;
        default:
                 return E_MS_FMT_GENERIC;
       }
 }

static void  _mstarSetSurfInfoSlotFree(MSTARDriverData *sdrv, int slotID)
{
     D_ASSERT(sdrv && slotID>=0 && slotID<MSTARGFX_MAX_LAYER_BUFFER);
     sdrv->mstarLayerBuffer[slotID].u16SlotUsed  = 0;
     //printf("clear surf slot %d \n", slotID);
}
static void  _mstarFreeRelatedSurfInfo(MSTARDriverData *sdrv, CoreSurface *surf)
{
    int i;
    for( i=0; i<MSTARGFX_MAX_LAYER_BUFFER; i++)
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed && sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface==surf)
        {
            MApi_GOP_GWIN_DestroyFB( (u8)sdrv->mstarLayerBuffer[i].u16GOPFBID);

            _mstarSetSurfInfoSlotFree(sdrv, i);
        }
}

void  mstarAllSurfInfo(MSTARDriverData *sdrv)
{
    int i;
    for( i=0; i<MSTARGFX_MAX_LAYER_BUFFER; i++)
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed)
        {
             MApi_GOP_GWIN_DestroyFB( (u8)sdrv->mstarLayerBuffer[i].u16GOPFBID);
            _mstarSetSurfInfoSlotFree(sdrv, i);
        }
}

static int _mstarGetFreeSurfInfoSlot(MSTARDriverData *sdrv)
{
    int i;
    for( i=0; i<MSTARGFX_MAX_LAYER_BUFFER; i++)
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed == 0)
            return i;

    mstarAllSurfInfo(sdrv);

    return 0;
}

static void _mstarSetSurfInfoSlot(MSTARDriverData *sdrv, int slotID,  u16 u16FBID,
                                                                               CoreSurface *surf, CoreSurfaceBuffer *buffer, unsigned long physAddr)
{
    D_ASSERT(sdrv && slotID>=0 && slotID<MSTARGFX_MAX_LAYER_BUFFER);
    sdrv->mstarLayerBuffer[slotID].u16GOPFBID  = u16FBID;
    sdrv->mstarLayerBuffer[slotID].pCoDFBCoreSurface  = surf;
    sdrv->mstarLayerBuffer[slotID].pCoDFBBuffer  = buffer;
    sdrv->mstarLayerBuffer[slotID].physAddr = physAddr;
    sdrv->mstarLayerBuffer[slotID].u16SlotUsed  = 1;
    //printf("set surf slot %d physical to %08x, surface %08x, buffer%08x\n", slotID, physAddr, surf, buffer);
}

static int _mstarFindBufSlot(MSTARDriverData *sdrv, CoreSurface *surf, CoreSurfaceBuffer *buffer,
                                                            unsigned long  phys, unsigned long pitch, u16 *pFBId)
{
    int i;
    u8 u8GOP_Ret, u8FBId;
    for( i=0; i<MSTARGFX_MAX_LAYER_BUFFER; i++)
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed && sdrv->mstarLayerBuffer[i].pCoDFBBuffer==buffer)
        {
        //printf("\nslot:%x savedSurface:%x saved buffer:%x required surface:%x required buffer :%x\n",i,sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface,sdrv->mstarLayerBuffer[i].pCoDFBBuffer,surf,buffer);
        //assert(sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface == surf);
             //if this case is hit,means the pCoDFBCoreSurface has been released by the dfb, but the bufslot info was not free
             if(sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface != surf)
             {
                MApi_GOP_GWIN_DestroyFB( (u8)sdrv->mstarLayerBuffer[i].u16GOPFBID);
                _mstarSetSurfInfoSlotFree(sdrv, i);
             }

             if(phys != sdrv->mstarLayerBuffer[i].physAddr)
             {
                  printf("buffer allocation changed, re-create GOP FB, w=%d, h=%d, pitch=%d!!!\n", surf->config.size.w, surf->config.size.h, (int)pitch);
                  MApi_GOP_GWIN_DestroyFB( (u8)sdrv->mstarLayerBuffer[i].u16GOPFBID);
                  u8GOP_Ret = MApi_GOP_GWIN_CreateFBFrom3rdSurf(surf->config.size.w, surf->config.size.h,
                                            _mstarDFBFmt2MSFmt(surf->config.format),    phys, pitch, &u8FBId);
                  if(GWIN_OK != u8GOP_Ret)
                  {
                        printf("    Serious warning, re-create FB for allocation Failed(%d), free slot %d!!!\n", u8GOP_Ret, i);
                        _mstarSetSurfInfoSlotFree(sdrv, i);
                        return -1;
                  }
                  _mstarSetSurfInfoSlot(sdrv, i,  u8FBId, surf, buffer,  phys);
             }

             if(pFBId)
                   *pFBId = sdrv->mstarLayerBuffer[i].u16GOPFBID;
             return i;
        }
    return -1;
}


static int
mstarLayerDataSize()
{
     D_DEBUG_AT(MSTAR_Layer, "%s\n", __FUNCTION__);
     return sizeof(MSTARLayerData);
}

static int
mstarRegionDataSize()
{
     D_DEBUG_AT(MSTAR_Layer, "%s\n", __FUNCTION__);
     return sizeof(MSTARRegionData);
}

static DFBResult
mstarInitLayer( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 DFBDisplayLayerDescription *description,
                 DFBDisplayLayerConfig      *config,
                 DFBColorAdjustment         *adjustment )
{
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     MSTARLayerData  *data = layer_data;
     u8 i;
     u8  curGopIndex;

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );
     D_DEBUG_AT(MSTAR_Layer, "%s\n", __FUNCTION__);

     /* initialize layer data */
     for( i=0; i<MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
     {
          if(layer == sdrv->layers[i])
          {
                 data->layer_index = i;
#if MSTAR_MULTI_GOP_SUPPORT
                 //the gopindex should be required form the dfbconfig
                 data->gop_index   = dfb_config->mst_gop_available[i];
                 data->gop_dst     = dfb_config->mst_gop_dstPlane[i];
                 sdev->layer_gop_index[i] = dfb_config->mst_gop_available[i];
#else
                 data->gop_index   = 0;
                 data->gop_dst     = E_GOP_DST_OP0;
                 sdev->layer_gop_index[i] = 0;
#endif
                 sdev->layer_zorder[i]  = 1;
                 sdev->layer_active[i] = false;
                 sdev->layer_gwin_id[i] = INVALID_WIN_ID;
                 break;
          }

     }

     D_ASSERT(i<MSTAR_MAX_OUTPUT_LAYER_COUNT);

     D_DEBUG_AT(MSTAR_Layer, "%s: data->layer: %d\n", __FUNCTION__, data->layer_index);

     /* set capabilities and type */
     description->caps = DLCAPS_SURFACE | DLCAPS_ALPHACHANNEL | DLCAPS_OPACITY  | DLCAPS_SRC_COLORKEY|
                                                 DLCAPS_LEVELS|DLCAPS_SCREEN_LOCATION;

     description->type = DLTF_STILL_PICTURE | DLTF_GRAPHICS | DLTF_VIDEO;

     /* set name */
     snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Input %d", sdrv->num_inputs );

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_OPTIONS;
     config->width       = sdrv->dev->gfx_max_width;//MSTAR MAX GFX WIDTH;
     config->height      = sdrv->dev->gfx_max_height;//MSTAR MAX GFX HEIGHT;
     config->pixelformat = DSPF_ARGB;
     config->buffermode  = DLBM_BACKVIDEO;
     config->options     = DLOP_ALPHACHANNEL;
     config->btile_mode  = false;
     curGopIndex = MApi_GOP_GWIN_GetCurrentGOP();
     if(curGopIndex != data->gop_index)
        MApi_GOP_GWIN_SwitchGOP(data->gop_index);

     if((data->gop_dst == E_GOP_DST_OP0)||(data->gop_dst == E_GOP_DST_MIXER2OP))
     {
           MS_PNL_DST_DispInfo dstDispInfo;
           config->width       = sdev->gfx_max_width;//MSTAR MAX GFX WIDTH;
           config->height      = sdev->gfx_max_height;//MSTAR MAX GFX HEIGHT;
           data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width = config->width;
           data->screen_size.height = sdev->layer_screen_size[data->layer_index].height= config->height;
           data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart= 0;
           data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart= 0;

           MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
           if(dstDispInfo.bYUVOutput)
           {
                MApi_GOP_GWIN_OutputColor(GOPOUT_YUV);
           }
           else
           {
                MApi_GOP_GWIN_OutputColor(GOPOUT_RGB);
           }
      /*
           printf("\nthe dst is OP\n");
           printf("\nthe dstDispInfo.HDTOT: %d\n",dstDispInfo.HDTOT);
           printf("\nthe dstDispInfo.VDTOT:%d\n",dstDispInfo.VDTOT);
           printf("\nthe dstDispInfo.DEHST:%d\n",dstDispInfo.DEHST);
           printf("\nthe dstDispInfo.DEHEND:%d\n",dstDispInfo.DEHEND);
           printf("\nthe dstDispInfo.DEVST:%d\n",dstDispInfo.DEVST);
           printf("\nthe dstDispInfo.DEVEND:%d\n",dstDispInfo.DEVEND);
           printf("\nthe dstDispInfo.bYUVOutput:%d\n",dstDispInfo.bYUVOutput);
      */

     }
     else if(data->gop_dst == E_GOP_DST_MIXER2VE)
     {

       //printf("\nbefore the MApi_VE_GetDstInfo\n");
       MS_VE_DST_DispInfo dstDispInfo;
       MApi_VE_GetDstInfo(&dstDispInfo,sizeof(MS_VE_DST_DispInfo));

       config->width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
       config->height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;
       data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
       data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
       data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = dstDispInfo.DEHST;
       data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = dstDispInfo.DEVST;

/*
       printf("\nthe layer data is%x\n",data);
       printf("\nafter MApi_VE_GetDstInfo\n");
       printf("\nthe dstDispInfo.bInterfaceMode is %d \n",dstDispInfo.bInterlaceMode);
       printf("\nthe dstDispInfo.HDTOT: %d\n",dstDispInfo.HDTOT);
       printf("\nthe dstDispInfo.VDTOT:%d\n",dstDispInfo.VDTOT);
       printf("\nthe dstDispInfo.DEHST:%d\n",dstDispInfo.DEHST);
       printf("\nthe dstDispInfo.DEHEND:%d\n",dstDispInfo.DEHEND);
       printf("\nthe dstDispInfo.DEVST:%d\n",dstDispInfo.DEVST);
       printf("\nthe dstDispInfo.DEVEND:%d\n",dstDispInfo.DEVEND);
*/

     }
     else if(data->gop_dst == E_GOP_DST_IP0)
     {

        //Default use the the 640*480 resolution
       config->options     = DLOP_ALPHACHANNEL;
       config->width       = 64;
       config->height      = 64;
       data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
       data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
       data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
       data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;
     }

     return DFB_OK;
}

#if MSTAR_MULTI_GOP_SUPPORT
static DFBResult mstarReorderGWIN(MSTARDriverData           *sdrv)
{
#if 0
     MSTARDeviceData           *sdev = sdrv->dev;
     u8  gwinId[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     int gwinzlevel[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     u8  gopIndex[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     u8  gwinCnt = 0;
     GOP_MuxConfig gopMuxConfig;
     u8 i, j, k;
      for( i=0; i<MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
     {
          if(false == sdev->layer_active[i])
                continue;
          for(j=0; j<gwinCnt; j++)
          {
              if(gwinzlevel[j]<sdev->layer_zorder[i])
                break;
          }
          k=gwinCnt;
          while(k>j)
          {
               gwinId[k]     = gwinId[k-1];
               gwinzlevel[k] = gwinzlevel[k-1];
               gopIndex[k]   = gopIndex[k-1];
               k--;
          }
          gwinzlevel[j] = sdev->layer_zorder[i];
          gwinId[j]     = sdev->layer_gwin_id[i];
          gopIndex[j]   = sdev->layer_gop_index[i];
          gwinCnt++;
     }
     gopMuxConfig.u8MuxCounts = gwinCnt;

     for(i=0; i<gwinCnt; i++)
     {
         gopMuxConfig.GopMux[i].u8GopIndex = gopIndex[i];
         gopMuxConfig.GopMux[i].u8MuxIndex = gwinzlevel[i];
     }
     //MApi_GOP_GWIN_SetOutputLayers(&gopMuxConfig);
     if (gwinCnt > 1)
     {
         GOP_GwinPri gwinPri;
         gwinPri.u8GwinNum = 1;
         for ( i=0; i<gwinCnt; i++)
         {
            gwinPri.u8GwinPri[0] = gwinId[i];
            MApi_GOP_GWIN_SetRelativeWinPrio(gopIndex[i], &gwinPri);
         }

     }
#endif
     return DFB_OK;
}
#else
static DFBResult mstarReorderGWIN(MSTARDriverData           *sdrv)
{
     MSTARDeviceData           *sdev = sdrv->dev;
     u8  gwinId[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     int  gwinzlevel[MSTAR_MAX_OUTPUT_LAYER_COUNT];
     u8  gwinCnt = 0;
     u8 i, j, k;
      for( i=0; i<MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
     {
          if(false == sdev->layer_active[i])
                continue;
          for(j=0; j<gwinCnt; j++)
          {
              if(gwinzlevel[j]<sdev->layer_zorder[i])
                break;
          }
          k=gwinCnt;
          while(k>j)
          {
               gwinId[k] = gwinId[k-1];
               gwinzlevel[k] = gwinzlevel[k-1];
               k--;
          }
          gwinzlevel[j] = sdev->layer_zorder[i];
          gwinId[j] = sdev->layer_gwin_id[i];
          gwinCnt++;
     }
     if(gwinCnt > 1)
     {
         GOP_GwinPri gwinPri;
         gwinPri.u8GwinNum = gwinCnt;
         for( i=0; i<gwinCnt; i++)
            gwinPri.u8GwinPri[i] = gwinId[i];
         MApi_GOP_GWIN_SetRelativeWinPrio(sdev->gfx_gop_index, &gwinPri);

     }
     return DFB_OK;
}
#endif
static DFBResult
mstarTestRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags *failed )
{
     MSTARDriverData           *sdrv = driver_data;
     MSTARDeviceData           *sdev = sdrv->dev;
     MSTARLayerData            *slay = layer_data;
     CoreLayerRegionConfigFlags  fail = 0;


     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );
     D_DEBUG_AT(MSTAR_Layer, "%s --> %d\n", __FUNCTION__, __LINE__);


   // Ip path special ,because OP ve path should can get the valid value in mstarInitlayer
     if(slay->gop_dst == E_GOP_DST_IP0)
     {
         MS_XC_DST_DispInfo dstDispInfo;
         u32 width=0;
         u32 height=0;
         u8  curGopIndex;

         curGopIndex = MApi_GOP_GWIN_GetCurrentGOP();
         if(curGopIndex != slay->gop_index)
            MApi_GOP_GWIN_SwitchGOP(slay->gop_index);


        if(MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN))
        {
           width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
           height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

           if((width>=64) && (width<2048) && (height>=64) && (height<2048))
           {
               slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width  = width;
               slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = height;
               slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart = dstDispInfo.DEHST;
               slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart = dstDispInfo.DEVST;
               if(dstDispInfo.bYUVInput)
               {
                    MApi_GOP_GWIN_OutputColor(GOPOUT_YUV);
               }
               else
               {
                    MApi_GOP_GWIN_OutputColor(GOPOUT_RGB);
               }
           }
        }
        if(curGopIndex != slay->gop_index)
            MApi_GOP_GWIN_SwitchGOP(curGopIndex);
     }

#if MSTAR_MULTI_GOP_SUPPORT
     //sdev->gfx_max_height= slay->screen_size.height;
     //sdev->gfx_max_width = slay->screen_size.width ;
#endif

     if (config->options & ~MSTAR_LAYER_SUPPORTED_OPTIONS){
          fail |= CLRCF_OPTIONS;
          printf("\nDean %s --> %d\n", __FUNCTION__, __LINE__);
     }

     // TODO: Currently, we only implement I8, ARGB8888 and ARGB1555
     switch (config->format) {
          case DSPF_LUT8:
          case DSPF_ARGB:
          case DSPF_ARGB1555:
          case DSPF_ARGB4444:
          case DSPF_RGB16:
               break;
          default:
               fail |= CLRCF_FORMAT;
     }
    D_DEBUG_AT(MSTAR_Layer,"slayer->gop_dst:%x\n",slay->gop_dst);
    D_DEBUG_AT(MSTAR_Layer, "%s --> %d config->width=%d config->height=%d\n", __FUNCTION__, __LINE__, config->width, config->height);
    D_DEBUG_AT(MSTAR_Layer, "%s --> %d sdev->lcd_width: %d, sdev->lcd_height: %d\n", __FUNCTION__, __LINE__, sdev->lcd_width, sdev->lcd_height);
     if (config->width  < 32 || config->width  > slay->screen_size.width)
          fail |= CLRCF_WIDTH;

     if (config->height < 32 || config->height > slay->screen_size.height)
          fail |= CLRCF_HEIGHT;



      if(slay->gop_dst == E_GOP_DST_OP0)
     {
          if (config->dest.x >= sdev->lcd_width || config->dest.y >= sdev->lcd_height)
               fail |= CLRCF_DEST;
          if(config->dest.x+config->dest.w >sdev->lcd_width||  config->dest.y+config->dest.h > sdev->lcd_height)
               fail |= CLRCF_DEST;
      }
     else
     {
          if (config->dest.x >= slay->screen_size.width || config->dest.y >= slay->screen_size.height)
               fail |= CLRCF_DEST;
          if(config->dest.x+config->dest.w >slay->screen_size.width||  config->dest.y+config->dest.h > slay->screen_size.height)
               fail |= CLRCF_DEST;
     }

     if(config->options & (DLOP_DST_COLORKEY|DLOP_FLICKER_FILTERING))
        fail |= CLRCF_OPTIONS;

     if((config->options&(DLOP_ALPHACHANNEL|DLOP_OPACITY)) == (DLOP_ALPHACHANNEL|DLOP_OPACITY))
     {
        //we can't support both of them.
        fail |= CLRCF_OPTIONS;
     }
     if(config->options&(DLOP_DEINTERLACING|DLOP_FIELD_PARITY))
        fail |= CLRCF_OPTIONS;

     if(config->num_clips > 0)
        fail |= CLRCF_CLIPS;

    D_DEBUG_AT(MSTAR_Layer, "%s --> %d config->dest.x: %d config->dest.y=%d\n", __FUNCTION__, __LINE__, config->dest.x, config->dest.y);
    D_DEBUG_AT(MSTAR_Layer, "%s --> %d config->dest.w: %d config->dest.h=%d\n", __FUNCTION__, __LINE__, config->dest.w, config->dest.h);

     if (failed)
          *failed = fail;

     D_DEBUG_AT(MSTAR_Layer, "%s --> %d fail: 0x%x\n", __FUNCTION__, __LINE__, fail);
     if (fail)
          return DFB_UNSUPPORTED;

     D_DEBUG_AT(MSTAR_Layer, "%s --> %d\n", __FUNCTION__, __LINE__);
     return DFB_OK;
}

static DFBResult
mstarAddRegion( CoreLayer             *layer,
                 void                  *driver_data,
                 void                  *layer_data,
                 void                  *region_data,
                 CoreLayerRegionConfig *config )
{
     MSTARRegionData *sreg = region_data;
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     MSTARLayerData  *slay = layer_data;

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

     D_MAGIC_SET( sreg, MSTARRegionData );
     sreg->config = *config;
     sreg->config_dirtyFlag = CLRCF_ALL;
     sdev->layer_refcnt[slay->layer_index]++;

     return DFB_OK;
}

static inline CoreLayerRegionConfigFlags mstarBuildUpdate( MSTARRegionData *sreg,    CoreLayerRegionConfig      *config,
                                                                                        CoreLayerRegionConfigFlags  updated)
{
     CoreLayerRegionConfigFlags retUpdated = CLRCF_NONE;


    if((updated&CLRCF_WIDTH) && sreg->config.width != config->width)
    {
        retUpdated |= CLRCF_WIDTH;
       sreg->config.width = config->width;
    }
    if((updated&CLRCF_HEIGHT) && sreg->config.height != config->height)
    {
        retUpdated |= CLRCF_HEIGHT;
       sreg->config.height = config->height;
    }
    if((updated&CLRCF_FORMAT) && sreg->config.format != config->format)
    {
        retUpdated |= CLRCF_FORMAT;
       sreg->config.format = config->format;
    }
    if((updated&CLRCF_SURFACE_CAPS) && sreg->config.surface_caps != config->surface_caps)
    {
        retUpdated |= CLRCF_SURFACE_CAPS;
       sreg->config.surface_caps = config->surface_caps;
    }
    if((updated&CLRCF_BUFFERMODE) && sreg->config.buffermode != config->buffermode)
    {
        retUpdated |= CLRCF_BUFFERMODE;
       sreg->config.buffermode = config->buffermode;
    }
    if((updated&CLRCF_OPTIONS) && sreg->config.options != config->options)
    {
        retUpdated |= CLRCF_OPTIONS;
       sreg->config.options = config->options;
    }


    if((updated&CLRCF_SOURCE_ID) && sreg->config.source_id != config->source_id)
    {
        retUpdated |= CLRCF_SOURCE_ID;
       sreg->config.source_id = config->source_id;


    }


    if((updated&CLRCF_SOURCE) && memcmp(&sreg->config.source ,  &config->source, sizeof(sreg->config.source)))
    {
        retUpdated |= CLRCF_SOURCE;
       memcpy(&sreg->config.source, &config->source, sizeof(sreg->config.source));
    }



    if((updated&CLRCF_DEST) && memcmp(&sreg->config.dest, &config->dest, sizeof(sreg->config.dest)))
    {
        retUpdated |= CLRCF_DEST;
       memcpy(&sreg->config.dest, &config->dest, sizeof(sreg->config.dest));
    }

    if(updated&CLRCF_CLIPS && config->num_clips>0)
    {
        retUpdated |= CLRCF_CLIPS;
        printf("warning: mstar t2 gfx driver doesn't support HW clip chains yet!\n");
    }


    if((updated&CLRCF_OPACITY) && sreg->config.opacity != config->opacity)
    {
        retUpdated |= CLRCF_OPACITY;
       sreg->config.opacity = config->opacity;
    }

    if((updated&CLRCF_ALPHA_RAMP) && memcmp(sreg->config.alpha_ramp, config->alpha_ramp, sizeof(sreg->config.alpha_ramp)))
    {
        retUpdated |= CLRCF_ALPHA_RAMP;
       memcpy(sreg->config.alpha_ramp , config->alpha_ramp, sizeof(sreg->config.alpha_ramp));
    }


    if((updated&CLRCF_SRCKEY) && memcmp(&sreg->config.src_key , &config->src_key, sizeof(sreg->config.src_key)))
    {
        retUpdated |= CLRCF_SRCKEY;
        memcpy(&sreg->config.src_key , &config->src_key, sizeof(sreg->config.src_key));
    }


    if((updated&CLRCF_DSTKEY) && memcmp(&sreg->config.dst_key , &config->dst_key, sizeof(sreg->config.dst_key)))
    {
        retUpdated |= CLRCF_DSTKEY;
        memcpy(&sreg->config.dst_key , &config->dst_key, sizeof(sreg->config.dst_key));
        printf("warning: mstar t2 gfx driver doesn't support destionation color yet!\n");
    }


     if((updated&CLRCF_PARITY) && sreg->config.positive != config->positive)
    {
        retUpdated |= CLRCF_PARITY;
        sreg->config.positive = config->positive;
    }

    if((updated&CLRCF_COLORSPACE) && sreg->config.btile_mode != config->btile_mode)
    {
        retUpdated |= CLRCF_COLORSPACE;
        sreg->config.btile_mode = config->btile_mode;

    }

    if((updated&CLRCF_HSTRETCH) && sreg->config.hstretchmode != config->hstretchmode)
    {
        retUpdated |= CLRCF_HSTRETCH;
        sreg->config.hstretchmode = config->hstretchmode;
    }

    if((updated&CLRCF_VSTRETCH) && sreg->config.vstretchmode != config->vstretchmode)
    {
        retUpdated |= CLRCF_VSTRETCH;
        sreg->config.vstretchmode = config->vstretchmode;
    }

    if((updated&CLRCF_TSTRETCH) && sreg->config.tstretchmode != config->tstretchmode)
    {
        retUpdated |= CLRCF_TSTRETCH;
        sreg->config.tstretchmode = config->tstretchmode;
    }

    if(updated&CLRCF_PALETTE)
    {
        if(config->format==DSPF_LUT8)
        {
            retUpdated |= CLRCF_PALETTE;
        }
    }

    if(updated&CLRCF_FREEZE)
    {
        retUpdated |= CLRCF_FREEZE;
    }


    return retUpdated;
}

static DFBResult
mstarSetRegion( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 void                       *region_data,
                 CoreLayerRegionConfig      *config,
                 CoreLayerRegionConfigFlags  updated,
                 CoreSurface                *surface,
                 CorePalette                *palette,
                 CoreSurfaceBufferLock      *lock )
{
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     MSTARRegionData *sreg = region_data;
     MSTARLayerData  *slay = layer_data;
     u8  curGopIndex;
     u16 u16FBId;
     u8 u8FBId;
     int freeSurfInfoSlot;
     bool bGWinEnable = true;

#if MSTAR_MULTI_GOP_SUPPORT
     fusion_skirmish_prevail( &sdev->beu_lock );
     //sdev->gfx_gop_index = slay->gop_index;
     //sdev->gfx_max_height= slay->screen_size.height;
     //sdev->gfx_max_width = slay->screen_size.width ;
     curGopIndex = MApi_GOP_GWIN_GetCurrentGOP();
     if(curGopIndex!=slay->gop_index)
        MApi_GOP_GWIN_SwitchGOP(slay->gop_index);

#endif

     sreg->config_dirtyFlag |= mstarBuildUpdate(sreg, config, updated);


     if(_mstarFindBufSlot(sdrv, surface, lock->buffer,  mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), lock->pitch, &u16FBId) < 0)
     {
          //If Apps GetPrimarySurface which create by other Apps instead of create it, just create Buf Slot and GOP FB here:
          freeSurfInfoSlot = _mstarGetFreeSurfInfoSlot(sdrv);
          if(freeSurfInfoSlot < 0)
          {
               D_DEBUG_AT( MSTAR_Layer, "\nmstarGetFreeSurfInfoSlot failed-->%d\n", freeSurfInfoSlot);
               D_ASSERT(0);
               return DFB_FAILURE;
          }

          if(GWIN_OK != MApi_GOP_GWIN_CreateFBFrom3rdSurf(surface->config.size.w, surface->config.size.h,
                            _mstarDFBFmt2MSFmt(config->format), mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), lock->pitch, &u8FBId))
          {
               D_DEBUG_AT( MSTAR_Layer, "\n MApi_GOP_GWIN_CreateFBFrom3rdSurf failed\n");
               D_ASSERT(0);
               return DFB_FAILURE;
          }

          _mstarSetSurfInfoSlot(sdrv, freeSurfInfoSlot,  (u16)u8FBId, surface, lock->buffer,  mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset));
          u16FBId = u8FBId;
     }

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );
     D_DEBUG_AT( MSTAR_Layer, "%s\n", __FUNCTION__);
     D_DEBUG_AT( MSTAR_Layer, "surface->num_buffers: %d\n", surface->num_buffers);
     //D_MAGIC_ASSERT( sreg, MSTARRegionData );
     D_ASSERT( slay->layer_index >= 0 );
     D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

     D_DEBUG_AT( MSTAR_Layer, "updated: %08x\n", updated);


     D_DEBUG_AT( MSTAR_Layer, "\ndfb_gfxcard_memory_physical( NULL, lock->offset )--> 0x%x\n", dfb_gfxcard_memory_physical( NULL, lock->offset ));
     D_DEBUG_AT( MSTAR_Layer, "\ndfb_gfxcard_memory_virtual( NULL, lock->offset )--> 0x%x\n", dfb_gfxcard_memory_virtual( NULL, lock->offset ));

    //memset(dfb_gfxcard_memory_virtual( NULL, 0), 0x00, 0xc00000);


     // Do some HW setting
     // create GWIN

     D_DEBUG_AT( MSTAR_Layer, "\n\nconfig->width: %d  config->height: %d\n", config->width, config->height);
     D_DEBUG_AT( MSTAR_Layer, "config->buffermode: 0x%x\n", config->buffermode);
     D_DEBUG_AT( MSTAR_Layer, "config->source_id: %d\n", config->source_id);
     D_DEBUG_AT( MSTAR_Layer, "config->source.x:%d config->source.y:%d config->source.w:%d config->source.h:%d\n", config->source.x, config->source.y, config->source.w, config->source.h);
     D_DEBUG_AT( MSTAR_Layer, "config->dest.x:%d config->dest.y:%d config->dest.w:%d config->dest.h:%d\n", config->dest.x, config->dest.y, config->dest.w, config->dest.h);
     D_DEBUG_AT( MSTAR_Layer, "config->opacity: %d\n", config->opacity);


    if(sreg->config_dirtyFlag & (CLRCF_SURFACE|CLRCF_WIDTH|CLRCF_HEIGHT|CLRCF_FORMAT|CLRCF_PALETTE))
    {
       if(INVALID_WIN_ID != sdev->layer_gwin_id[slay->layer_index])
        {
              MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id[slay->layer_index]);
              sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
        }
        switch(config->format)
        {
        case DSPF_UNKNOWN:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_UNKNOWN\n");
            break;

        case DSPF_ARGB1555:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ARGB1555\n");
            sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0);
            MApi_GOP_GWIN_SetFieldInver(TRUE);
            //printf("\nsdev->gfx_width:%d,sdev->gfx_height:%d\n",sdev->gfx_max_width, sdev->gfx_max_height);
            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
            MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index],0, 0);
            sreg->config_dirtyFlag &= ~CLRCF_DEST;
            //MAdp_GOP_GWIN_Blending_Set(sdev->gfx_gop_index,0,TRUE,0);
            break;

        case DSPF_RGB16:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB16\n");
            sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0);
            MApi_GOP_GWIN_SetFieldInver(TRUE);
            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
            MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index],0, 0);
            sreg->config_dirtyFlag &= ~CLRCF_DEST;
            break;

        case DSPF_RGB24:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB24\n");
            break;

        case DSPF_RGB32:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB32\n");
            break;

        case DSPF_ARGB:
            sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0);

            //printf("\nsreg->config.dest.x :%d,sreg->config.dest.y:%d\n",sreg->config.dest.x,sreg->config.dest.y);
            //printf("\nthe sdrv->dev->gop_stretch_enabled is %d\n",sdrv->dev->gop_stretch_enabled);
            MApi_GOP_GWIN_SetFieldInver(TRUE);

            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
            MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index],0, 0);
            sreg->config_dirtyFlag &= ~CLRCF_DEST;
            break;

        case DSPF_A8:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_A8\n");
            break;

        case DSPF_YUY2:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_YUY2\n");
            break;

        case DSPF_YVYU:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_YVYU\n");
            break;

        case DSPF_RGB332:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB332\n");
            break;

        case DSPF_UYVY:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_UYVY\n");
            break;

        case DSPF_I420:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_I420\n");
            break;

        case DSPF_YV12:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_YV12\n");
            break;

        case DSPF_LUT8:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_LUT8\n");

            // set palette table
            if(sreg->config_dirtyFlag & CLRCF_PALETTE){
                if(palette){
                    D_DEBUG_AT( MSTAR_Layer, "\n================================================\n");
                    D_DEBUG_AT( MSTAR_Layer, "palette->num_entries: %d\n", palette->num_entries);
                    D_DEBUG_AT( MSTAR_Layer, "palette->hash_attached: %d\n", palette->hash_attached);
                    D_DEBUG_AT( MSTAR_Layer, "\n================================================\n");

                    //                getchar();
                    GOP_PaletteEntry GOPPaletteEntry8888[256];
                    if(palette->entries){
                        int i;
                        for(i=0; i < palette->num_entries; ++i){
                            GOPPaletteEntry8888[i].RGB.u8A = palette->entries[i].a;
                            GOPPaletteEntry8888[i].RGB.u8R = palette->entries[i].r;
                            GOPPaletteEntry8888[i].RGB.u8G = palette->entries[i].g;
                            GOPPaletteEntry8888[i].RGB.u8B = palette->entries[i].b;
                        }
                        MApi_GOP_GWIN_SetPaletteOpt(GOPPaletteEntry8888, 0, (palette->num_entries-1), E_GOP_PAL_ARGB8888);
                    }
                }
            }

            sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0);
            //printf("\nsdev->gfx_width:%d,sdev->gfx_height:%d\n",sdev->gfx_max_width, sdev->gfx_max_height);

            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
            MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index],0, 0);
            sreg->config_dirtyFlag &= ~CLRCF_DEST;
            break;

        case DSPF_ALUT44:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ALUT44\n");
            break;

        case DSPF_AiRGB:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_AiRGB\n");
            break;

        case DSPF_A1:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_A1\n");
            break;

        case DSPF_NV12:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_NV12\n");
            break;

        case DSPF_NV16:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_NV16\n");
            break;

        case DSPF_ARGB2554:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ARGB2554\n");
            break;

        case DSPF_ARGB4444:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ARGB4444\n");

            sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0);
            MApi_GOP_GWIN_SetFieldInver(TRUE);
            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
            MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index],0, 0);
            sreg->config_dirtyFlag &= ~CLRCF_DEST;

            break;

        case DSPF_NV21:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_NV21\n");
            break;

        case DSPF_AYUV:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_AYUV\n");
            break;

        case DSPF_A4:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_A4\n");
            break;

        case DSPF_ARGB1666:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ARGB1666\n");
            break;

        case DSPF_ARGB6666:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_ARGB6666\n");
            break;

        case DSPF_RGB18:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB18\n");
            break;

        case DSPF_LUT2:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_LUT2\n");
            break;

        case DSPF_LUT1:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_LUT1\n");
            break;

        case DSPF_LUT4:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_LUT4\n");
            break;

        case DSPF_RGB444:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB444\n");
            break;

        case DSPF_RGB555:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGB555\n");
            break;

        case DSPF_BGR555:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_BGR555\n");
            break;
        case DSPF_RGBA4444:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGBA4444\n");
            break;
        case DSPF_RGBA5551:
            D_DEBUG_AT( MSTAR_Layer, "format: DSPF_RGBA5551\n");
            break;
        }
   }
    if(INVALID_WIN_ID==sdev->layer_gwin_id[slay->layer_index] ||  GWIN_FAIL == sdev->layer_gwin_id[slay->layer_index])
    {
          sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
          return DFB_FAILURE;
    }
    if(sreg->config_dirtyFlag & (CLRCF_SRCKEY|CLRCF_OPTIONS))
    {
        MS_U32 tcol;
        if(sreg->config.options & DLOP_SRC_COLORKEY)
        {
        //to make the layer colorkey interface is the same as surface colorkey,if the format is not ARGB32.
        //the AP need to do bit shift to complete the format convert.
        //for example:ARGB1555:ARRRRRGGGGGBBBBB---->Rchannel :RRRRR000   Gchannel:GGGGG000    Bchannel:BBBBB000
        //In this function of the DFB ,we will do the second format convert as following. the blank LSB BIT will be filled with
        //the MSB

            switch(config->format)
            {
                case DSPF_ARGB1555:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)|(sreg->config.src_key.r>>5);
                    sreg->config.src_key.g = (sreg->config.src_key.g)|(sreg->config.src_key.g>>5);
                    sreg->config.src_key.b = (sreg->config.src_key.b)|(sreg->config.src_key.b>>5);
                }
                break;

                case DSPF_RGB16:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)|(sreg->config.src_key.r>>5);
                    sreg->config.src_key.g = (sreg->config.src_key.g)|(sreg->config.src_key.g>>6);
                    sreg->config.src_key.b = (sreg->config.src_key.b)|(sreg->config.src_key.b>>5);
                }
                break;

                case DSPF_ARGB:
                break;

                case DSPF_ARGB4444:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)|(sreg->config.src_key.r>>4);
                    sreg->config.src_key.g = (sreg->config.src_key.g)|(sreg->config.src_key.g>>4);
                    sreg->config.src_key.b = (sreg->config.src_key.b)|(sreg->config.src_key.b>>4);
                }
                break;

                default:
                break;
            }

            tcol = sreg->config.src_key.r<<16|sreg->config.src_key.g<<8|sreg->config.src_key.b;
            MApi_GOP_GWIN_SetTransClr_8888(tcol, 0);
            MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, TRUE);
        }
        else
        {
            MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE);
        }
   }
   if(sreg->config_dirtyFlag & (CLRCF_OPACITY|CLRCF_OPTIONS))
   {
       if(sreg->config.options & DLOP_OPACITY)
       {
            MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], FALSE, sreg->config.opacity>>2);
            if(0x0 == sreg->config.opacity)
            {
                bGWinEnable = false;
            }
       }
       else if (sreg->config.options & DLOP_ALPHACHANNEL)
       {
            MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], TRUE, sreg->config.opacity>>2);
       }
       else
       {
            MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], FALSE, 0xFF>>2);
       }
   }

   if(sreg->config_dirtyFlag & CLRCF_HSTRETCH)
   {
           MApi_GOP_GWIN_Set_HStretchMode(sreg->config.hstretchmode);
   }

   if(sreg->config_dirtyFlag & CLRCF_VSTRETCH)
   {
           MApi_GOP_GWIN_Set_VStretchMode(sreg->config.vstretchmode);
   }

   if(sreg->config_dirtyFlag & CLRCF_TSTRETCH)
   {
           MApi_GOP_GWIN_Set_TranspColorStretchMode(sreg->config.tstretchmode);
   }



   if(sreg->config_dirtyFlag & (CLRCF_DEST|CLRCF_WIDTH|CLRCF_HEIGHT))
   {

        //MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index], sreg->config.dest.x, sreg->config.dest.y);
        /*
        printf("\nsdrv->dev->gop_stretch_enabled:%d\n",sdrv->dev->gop_stretch_enabled);
        printf("\nsreg->config.dest.w:%d\n", sreg->config.dest.w);
        printf("\nsreg->config.dest.h:%d\n",sreg->config.dest.h);
        printf("\nsreg->config.width:%d\n",sreg->config.width);
        printf("\n\sreg->config.height:%d\n",sreg->config.height);
        printf("\nsreg->config.dest.x:%d\n",sreg->config.dest.x);
        printf("\nsreg->config.dest.y:%d\n",sreg->config.dest.y);
        */
        if(sreg->config.dest.w && sreg->config.dest.h)
        {
            MApi_GOP_GWIN_Set_STRETCHWIN(slay->gop_index,slay->gop_dst, sreg->config.dest.x, sreg->config.dest.y,sreg->config.width, sreg->config.height);
            MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, sreg->config.dest.w);
            MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, sreg->config.dest.h);
        }


   }


   if(sreg->config_dirtyFlag & CLRCF_COLORSPACE)
   {
        EN_GOP_TILE_DATA_TYPE tiletype = 0xff;
        switch(DFB_BITS_PER_PIXEL(config->format))
        {
            case 16:
              tiletype = E_GOP_TILE_DATA_16BPP;
              break;
            case 32:
              tiletype = E_GOP_TILE_DATA_32BPP;
              break;
            default:
              break;

        }
        MApi_GOP_GWIN_EnableTileMode(sdev->layer_gwin_id[slay->layer_index], sreg->config.btile_mode, tiletype );
       /*
        printf("\n sreg->config.btile_mode :%d\n",sreg->config.btile_mode);
        printf("\n the tiletype is %d\n",tiletype);
       */
   }



    MApi_GOP_GWIN_Enable(sdev->layer_gwin_id[slay->layer_index], (MS_BOOL)bGWinEnable) ;
    MApi_GOP_GWIN_SetGWinShared(sdev->layer_gwin_id[slay->layer_index], TRUE) ;
    MApi_GOP_GWIN_SetGWinSharedCnt(sdev->layer_gwin_id[slay->layer_index], sdev->layer_refcnt[slay->layer_index]);
    sdev->layer_active[slay->layer_index] = true;
    mstarReorderGWIN(sdrv);
    fusion_skirmish_dismiss( &sdev->beu_lock );
    sreg->config_dirtyFlag = CLRCF_NONE;

#if MSTAR_MULTI_GOP_SUPPORT

    if(curGopIndex!=slay->gop_index)
        MApi_GOP_GWIN_SwitchGOP(curGopIndex);

#endif

     return DFB_OK;
}

static DFBResult
mstarRemoveRegion( CoreLayer *layer,
                    void      *driver_data,
                    void      *layer_data,
                    void      *region_data )
{
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     MSTARLayerData  *slay = layer_data;

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );
     D_DEBUG_AT( MSTAR_Layer, " %s\n", __FUNCTION__);

     D_ASSERT( sdev != NULL );
     D_ASSERT( slay != NULL );

     D_ASSERT( slay->layer_index >= 0 );
     D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

     fusion_skirmish_prevail( &sdev->beu_lock );
     sdev->layer_refcnt[slay->layer_index]--;
     MApi_GOP_GWIN_SetGWinSharedCnt(sdev->layer_gwin_id[slay->layer_index], sdev->layer_refcnt[slay->layer_index]);

     if((sdev->layer_refcnt[slay->layer_index]<=0) && (INVALID_WIN_ID != sdev->layer_gwin_id[slay->layer_index]))
     {
          //MAdp_GOP_GWIN_Enable( sdev->gfx_gop_index, 0, false) ;
          MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], FALSE) ;
          MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id[slay->layer_index]);
          sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
          sdev->layer_active[slay->layer_index] = false;

     }

     fusion_skirmish_dismiss( &sdev->beu_lock );

     return DFB_OK;
}

// If you support double buffer or triple buffer, you
// should implement the feature in FlipRegion().

static DFBResult
mstarFlipRegion( CoreLayer             *layer,
                  void                  *driver_data,
                  void                  *layer_data,
                  void                  *region_data,
                  CoreSurface           *surface,
                  DFBSurfaceFlipFlags    flags,
                  CoreSurfaceBufferLock *lock )
{

     u16 tagID;
     CoreSurfaceBuffer *buffer;
     MSTARDriverData  *sdrv = driver_data;
     MSTARDeviceData  *sdev = sdrv->dev;
     MSTARLayerData   *slay = layer_data;
     u16 u16FBId;
     u16 u16QueueCnt;
     u16 targetQueueCnt;

     u8  u8FBId;
     int freeSurfInfoSlot;

     D_ASSERT( slay->layer_index >= 0 );
     D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

     #if 0
     printf("\nDean %s\n", __FUNCTION__);
     printf("\nlock->addr = 0x%x\n", (int)lock->addr);
     printf("\nlock->phy = 0x%x\n", lock->phys);
     printf("\nlock->offset = 0x%x\n", lock->offset);

     printf("\nsurface->num_buffers: %d\n", surface->num_buffers);
     printf("\nsurface->resource_id: %d\n", surface->resource_id);
     printf("\nsurface->notifications: 0x%x\n", surface->notifications);
     printf("\nsurface->flips: %d\n", surface->flips);
     #endif

     D_ASSERT( surface != NULL );
     D_ASSERT( sdrv != NULL );
     D_ASSERT( sdev != NULL );
     D_ASSERT( slay != NULL );
     D_ASSERT( region_data != NULL );
     D_ASSERT(lock && lock->allocation);

     //printf( "Flip: 0x%08X, pitch%d, buffer%08x\n", lock->phys, lock->pitch, lock->buffer) ;
     if(_mstarFindBufSlot(sdrv, surface, lock->buffer, mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), lock->pitch, &u16FBId) < 0)
     {
          //If Apps GetPrimarySurface which create by other Apps instead of create it, just create Buf Slot and GOP FB here:
          freeSurfInfoSlot = _mstarGetFreeSurfInfoSlot(sdrv);
          if(freeSurfInfoSlot < 0)
          {
               D_DEBUG_AT( MSTAR_Layer, "\nmstarGetFreeSurfInfoSlot failed-->%d\n", freeSurfInfoSlot);
               D_ASSERT(0);
               return DFB_FAILURE;
          }

          if(GWIN_OK != MApi_GOP_GWIN_CreateFBFrom3rdSurf(surface->config.size.w, surface->config.size.h,
                            _mstarDFBFmt2MSFmt(surface->config.format), mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), lock->pitch, &u8FBId))
          {
               printf("local GOP FB full, recreate all!!\n");
               mstarAllSurfInfo(sdrv);
               freeSurfInfoSlot = 0;
               //try again
               if(GWIN_OK != MApi_GOP_GWIN_CreateFBFrom3rdSurf(surface->config.size.w, surface->config.size.h,
                            _mstarDFBFmt2MSFmt(surface->config.format), mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), lock->pitch, &u8FBId))
                {

                     D_DEBUG_AT( MSTAR_Layer, "\n MApi_GOP_GWIN_CreateFBFrom3rdSurf failed\n");
                     D_ASSERT(0);
                     return DFB_FAILURE;
               }
          }

          _mstarSetSurfInfoSlot(sdrv, freeSurfInfoSlot,  (u16)u8FBId, surface, lock->buffer,  mstarGFXAddr(lock->phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset));
          u16FBId = u8FBId;
     }

    tagID = (u16) lock->allocation->gfxSerial.serial;

    if(surface->config.caps & DSCAPS_TRIPLE)
         targetQueueCnt = 2;
    else//  if(surface->config.caps & DSCAPS_DOUBLE)
         targetQueueCnt = 1;


    do
    {
         u16QueueCnt = targetQueueCnt;
         if(MApi_GOP_Switch_GWIN_2_FB(sdev->layer_gwin_id[slay->layer_index], u16FBId, tagID, &u16QueueCnt))
              break;

         if(u16QueueCnt <= targetQueueCnt-1)
         {
              printf("Serious warning, unknow error!!\n");
              return DFB_FAILURE;
         }
    }while(1);


     buffer = lock->buffer;
     D_ASSERT( buffer != NULL );

     fusion_skirmish_prevail( &sdev->beu_lock );

     dfb_surface_flip( surface, false );

     fusion_skirmish_dismiss( &sdev->beu_lock );



     return DFB_OK;
}

/*
static DFBResult
mstarUpdateRegion( CoreLayer             *layer,
                    void                  *driver_data,
                    void                  *layer_data,
                    void                  *region_data,
                    CoreSurface           *surface,
                    const DFBRegion       *update,
                    CoreSurfaceBufferLock *lock )
{
     //MSTARDriverData *sdrv = driver_data;
     //MSTARDeviceData *sdev = sdrv->dev;

     D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );
     D_DEBUG_AT( MSTAR_Layer, " %s\n", __FUNCTION__);

     D_ASSERT( surface != NULL );
     //D_ASSERT( sdrv != NULL );
     //D_ASSERT( sdev != NULL );


     //BEU_Start( sdrv, sdev );

     //if (!(surface->config.caps & DSCAPS_FLIPPING))
         // BEU_Wait( sdrv, sdev );

     return DFB_OK;
}
*/
static DFBResult
mstar_primaryAllocateSurface( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        void                   *region_data,
                        CoreLayerRegionConfig  *config,
                        CoreSurface           **ret_surface )
{
     CoreSurfaceConfig conf;
     DFBResult ret;
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData  *sdev = sdrv->dev;
     CoreSurface *surface;
     int i, number_buffers;
     CoreSurfaceBufferLock ret_lock;
     int freeSurfInfoSlot;
     u8 u8FBID, u8GOP_Ret;
     CoreSurfaceTypeFlags cstf_flag = 0;
     u32 u32_gop_index = 0;
     bool tryAgain = false;
     if(_mstarDFBFmt2MSFmt(config->format) == E_MS_FMT_GENERIC)
        return DFB_FAILURE;

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


     cstf_flag = CSTF_LAYER;

#if 1
     if (dfb_config->mst_gop_miu_setting & GOP_MIU_MASK)
     {
        u32_gop_index = dfb_config->mst_gop_available[layer->shared->layer_id];
        if (dfb_config->mst_gop_miu_setting & (0x1L<<u32_gop_index))
        {
            cstf_flag |= CSTF_MIU1;
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

     ret = dfb_surface_create( layer->core, &conf, cstf_flag, (unsigned long )layer->shared->layer_id, NULL, ret_surface );
     if(ret)
     {
         D_DEBUG_AT( MSTAR_Layer, "\nstar_primaryAllocateSurface failed-->%d\n", ret);
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
                bufferRole = CSBR_IDLE; break;
            case 1:
               bufferRole = CSBR_BACK; break;
            case 2:
               bufferRole = CSBR_FRONT; break;
            default:
                D_ASSERT(0);
               goto FAILED_FREE_SURFACE;
          }
            freeSurfInfoSlot = _mstarGetFreeSurfInfoSlot(sdrv);
            if(freeSurfInfoSlot < 0)
            {
                   D_DEBUG_AT( MSTAR_Layer, "\nmstarGetFreeSurfInfoSlot failed-->%d\n", ret);
                   goto FAILED_FREE_SURFACE;
            }

            if(DFB_OK != (ret=dfb_surface_lock_buffer(surface, bufferRole, CSAID_GPU, CSAF_READ, &ret_lock)))
            {
                D_DEBUG_AT( MSTAR_Layer, "\ndfb_surface_lock_buffer failed-->%d\n", ret);
                goto FAILED_FREE_SURFACE;
            }

             u8GOP_Ret = MApi_GOP_GWIN_CreateFBFrom3rdSurf(surface->config.size.w, surface->config.size.h,
                   _mstarDFBFmt2MSFmt(config->format), mstarGFXAddr(ret_lock.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), ret_lock.pitch, &u8FBID);

               if(u8GOP_Ret != GWIN_OK)
              {
                     dfb_surface_unlock_buffer(surface, &ret_lock);
                     if(tryAgain)
                     {
                             D_DEBUG_AT( MSTAR_Layer, "\nApi_GOP_GWIN_CreateFBbyStaticAddr failed-->%d\n", ret);
                             goto FAILED_FREE_SURFACE;
                     }
                     mstarAllSurfInfo(sdrv);
                     tryAgain = true;
                     goto TRY_AGAIN;

              }
               _mstarSetSurfInfoSlot(sdrv, freeSurfInfoSlot,  (u16)u8FBID, surface, ret_lock.buffer,  mstarGFXAddr(ret_lock.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset));
                dfb_surface_unlock_buffer(surface, &ret_lock);
        }

     return DFB_OK;
FAILED_FREE_SURFACE:
        /* Unlink from structure. */

    _mstarFreeRelatedSurfInfo(sdrv, surface);
     dfb_surface_unlink( ret_surface );
    return DFB_FAILURE;

}

static DFBResult
mstar_primaryReallocateSurface( CoreLayer             *layer,
                          void                  *driver_data,
                          void                  *layer_data,
                          void                  *region_data,
                          CoreLayerRegionConfig *config,
                          CoreSurface           *surface )
{
     MSTARDriverData *sdrv = driver_data;
     MSTARDeviceData *sdev = sdrv->dev;
     DFBResult         ret;
     CoreSurfaceConfig conf;
     int i, number_buffers;
     int freeSurfInfoSlot;
     u8 u8FBID, u8GOP_Ret;
     CoreSurfaceBufferLock ret_lock;
     bool tryAgain = false;

     memset(&conf,0,sizeof(CoreSurfaceConfig));
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

     if(!memcmp(&conf,&surface->config,sizeof(CoreSurfaceConfig)))
     {
        return DFB_OK;
     }


     ret = dfb_surface_reconfig( surface, &conf );

     if (ret)
          return ret;

      _mstarFreeRelatedSurfInfo(sdrv, surface);

     if (DFB_PIXELFORMAT_IS_INDEXED(config->format) && !surface->palette) {

          DFBResult    ret;
          CorePalette *palette;

          ret = dfb_palette_create( NULL,    /* FIXME */
                                    1 << DFB_COLOR_BITS_PER_PIXEL( config->format ),
                                    &palette );
          if (ret)
               return ret;

          if (config->format == DSPF_LUT8)
               dfb_palette_generate_rgb332_map( palette );

          dfb_surface_set_palette( surface, palette );

          dfb_palette_unref( palette );
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
     for(i=0; i<number_buffers; i++)
     {
         CoreSurfaceBufferRole bufferRole;
         switch(i)
         {
            case 0:
                bufferRole = CSBR_FRONT; break;
            case 1:
               bufferRole = CSBR_BACK; break;
            case 2:
               bufferRole = CSBR_IDLE; break;
            default:
                D_ASSERT(0);
               goto FAILED_RETURN;
          }

            freeSurfInfoSlot = _mstarGetFreeSurfInfoSlot(sdrv);

            if(freeSurfInfoSlot < 0)
            {
                   D_DEBUG_AT( MSTAR_Layer, "\n_mstarGetFreeSurfInfoSlot failed-->%d\n", ret);
                   goto FAILED_RETURN;
            }

             if(DFB_OK != dfb_surface_lock_buffer(surface, bufferRole, CSAID_GPU, CSAF_READ, &ret_lock))
             {
                    D_DEBUG_AT( MSTAR_Layer, "\ndfb_surface_lock_buffer failed-->%d\n", ret);
                    goto FAILED_RETURN;
             }

             u8GOP_Ret = MApi_GOP_GWIN_CreateFBFrom3rdSurf(surface->config.size.w, surface->config.size.h,
                   _mstarDFBFmt2MSFmt(config->format), mstarGFXAddr(ret_lock.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset), ret_lock.pitch, &u8FBID);

               if(u8GOP_Ret != GWIN_OK)
              {
                     dfb_surface_unlock_buffer(surface, &ret_lock);
                     if(tryAgain)
                     {
                             D_DEBUG_AT( MSTAR_Layer, "\nApi_GOP_GWIN_CreateFBbyStaticAddr failed-->%d\n", ret);
                             goto FAILED_RETURN;
                     }
                     mstarAllSurfInfo(sdrv);
                     tryAgain = true;
                     goto TRY_AGAIN;

              }
               _mstarSetSurfInfoSlot(sdrv, freeSurfInfoSlot,  (u16)u8FBID, surface, ret_lock.buffer,  mstarGFXAddr(ret_lock.phys, sdev->mst_miu1_cpu_offset, sdev->mst_miu1_hal_offset));
                dfb_surface_unlock_buffer(surface, &ret_lock);

     }
     return DFB_OK;
FAILED_RETURN:
    _mstarFreeRelatedSurfInfo(sdrv, surface);
    return ret;
}

static DFBResult
mstar_primaryDeallocateSurface( CoreLayer              *layer,
                                     void                   *driver_data,
                                     void                   *layer_data,
                                     void                   *region_data,
                                     CoreSurface            *surface)
{
     MSTARDriverData *sdrv = driver_data;
     //MSTARDeviceData *sdev = sdrv->dev;
     _mstarFreeRelatedSurfInfo(sdrv, surface);
     return DFB_OK;
}

     /*
      * Return the z position of the layer.
      */
DFBResult mstar_GetLayerlevel( CoreLayer              *layer,
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

     /*
      * Move the layer below or on top of others (z position).
      */

DFBResult mstar_SetLayerlevel( CoreLayer              *layer,
                                         void                   *driver_data,
                                         void                   *layer_data,
                                         int                     level )
{
       MSTARDriverData *sdrv = driver_data;
       MSTARLayerData  *slay = layer_data;
        MSTARDeviceData *sdev = sdrv->dev;

       fusion_skirmish_prevail( &sdev->beu_lock );
/**/
       if(sdev->layer_zorder[slay->layer_index] != level)
       {
           sdev->layer_zorder[slay->layer_index] = level;
           mstarReorderGWIN(sdrv);
           MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], level) ;
       }

       fusion_skirmish_dismiss( &sdev->beu_lock );

       return DFB_OK;
}


//- AddRegion() will be called when the window is first displayed
//- TestRegion() and SetRegion() are called when the configuration changes
//  eg. the window is moved or resized.
//- RemoveRegion() will be called when the window is destroyed.


DisplayLayerFuncs mstarLayerFuncs = {
     LayerDataSize:      mstarLayerDataSize,
     RegionDataSize:     mstarRegionDataSize,
     InitLayer:          mstarInitLayer,

     TestRegion:         mstarTestRegion,
     AddRegion:          mstarAddRegion,
     SetRegion:          mstarSetRegion,
     RemoveRegion:       mstarRemoveRegion,
     FlipRegion:         mstarFlipRegion,
     AllocateSurface:  mstar_primaryAllocateSurface,
     ReallocateSurface:   mstar_primaryReallocateSurface,
     DeallocateSurface:   mstar_primaryDeallocateSurface,
     GetLevel:               mstar_GetLayerlevel,
     SetLevel:               mstar_SetLayerlevel
     //UpdateRegion:       mstarUpdateRegion
};

