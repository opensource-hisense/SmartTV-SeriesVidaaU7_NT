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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <directfb.h>

#include <direct/debug.h>

#include <core/coretypes.h>
#include <core/windows_internal.h>
#include <core/wm.h>
#include <core/layers_internal.h>
#include <misc/conf.h>

#include "mi_common.h"
#include "mi_osd.h"
#include "hwcursor.h"

#define MAX_BUFFER 20
#define INI_PARSER_SIZE             256
#define SHIFT32 32
#define MI_OSD_DEBUG_LEVEL 0xF0

/* global MI handle */
static MI_HANDLE glayerHnd = MI_HANDLE_NULL;
static MI_HANDLE gwinHnd = MI_HANDLE_NULL;

/* Amplification from DFB layer to MI_OSD layer */
static int width_scale = 1;
static int height_scale = 1;

static inline const char* getMiErrorName(MI_RESULT ret) {
        switch (ret) {
            case MI_OK: return "MI_OK";
            case MI_CONTINUE: return "MI_CONTINUE";
            case MI_HAS_INITED: return "MI_HAS_INITED";
            case MI_ERR_FAILED: return "MI_ERR_FAILED";
            case MI_ERR_NOT_INITED: return "MI_ERR_NOT_INITED";
            case MI_ERR_NOT_SUPPORT: return "MI_ERR_NOT_SUPPORT";
            case MI_ERR_NOT_IMPLEMENT: return "MI_ERR_NOT_IMPLEMENT";
            case MI_ERR_INVALID_HANDLE: return "MI_ERR_INVALID_HANDLE";
            case MI_ERR_INVALID_PARAMETER: return "MI_ERR_INVALID_PARAMETER";
            case MI_ERR_RESOURCES: return "MI_ERR_RESOURCES";
            case MI_ERR_MEMORY_ALLOCATE: return "MI_ERR_MEMORY_ALLOCATE";
            case MI_ERR_CHAOS: return "MI_ERR_CHAOS";
            case MI_ERR_DATA_ERROR: return "MI_ERR_DATA_ERROR";
            case MI_ERR_TIMEOUT: return "MI_ERR_TIMEOUT";
            case MI_ERR_LIMITION: return "MI_ERR_LIMITION";
            case MI_ERR_BUSY: return "MI_ERR_BUSY";
            default: return "UnDefined";
        }
}



#define DFB_MI_TRACE(RET, API, HANDLE)                                                  \
do {                                                                            \
    MI_RESULT miret=RET;    \
    if(miret != MI_OK) {   \
        DBG_CURSOR_MSG("\33[0;33;44m[DFB]%s failed\33[0m ,handle: %d, Error: %s !!!, line=[%d], pid=%d\n", API, HANDLE, (getMiErrorName(miret)),  __LINE__,getpid());  \
    }   \
} while(0)

DFBResult dfb_wm_destroy_hwcursor(int gop_id)
{

    MI_HANDLE glayerHnd = MI_HANDLE_NULL;
    MI_RESULT mi_ret = MI_OK;

    DBG_CURSOR_MSG("[DFB] %s %d, go to hw cursor destroy flow, query MI layer and window handle, gop id=%d,pid=%d\n",__FUNCTION__,__LINE__, gop_id, getpid());
    MI_OSD_GetLayerHandleParams_t  params = {gop_id};
    mi_ret = MI_OSD_GetLayerHandle (&params  , &glayerHnd);
    if (mi_ret){
        DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }

    if (glayerHnd != MI_HANDLE_NULL){
        DBG_CURSOR_MSG("[DFB] %s %d, cursor_ref is 0, destroy MI layer, layer=%d,pid=%d\n",__FUNCTION__,__LINE__,glayerHnd,getpid());

        mi_ret = MI_OSD_LayerDestroy(glayerHnd);
        if (mi_ret) {
            DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
            glayerHnd = MI_HANDLE_NULL;
        }

  }
  return DFB_OK;
}

static inline void
transform_stack_to_dest( CoreWindowStack *stack,
                         const DFBRegion *region,
                         DFBRegion       *ret_dest )
{
     DFBDimension size = { stack->width, stack->height };

     DFB_REGION_ASSERT( region );

     dfb_region_from_rotated( ret_dest, region, &size, stack->rotation );
}

static DFBResult createMiOsdSurface(const MI_OSD_SurfaceInfo_t *stSurfaceInfo, MI_HANDLE *phSurface) {
    MI_HANDLE surfaceHandle = MI_HANDLE_NULL;
    MI_OSD_BeginDraw();
    MI_RESULT mi_ret = MI_OSD_SurfaceCreate(stSurfaceInfo, &surfaceHandle);
    MI_OSD_EndDraw();

    if (mi_ret != MI_OK) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        phSurface = MI_HANDLE_NULL;
        return DFB_FAILURE;
    }
    *phSurface = surfaceHandle;
    return DFB_OK;
}

static DFBResult doMiOsdWindowFlip(MI_HANDLE hWindow, MI_HANDLE hSurface) {
    if (hWindow == MI_HANDLE_NULL || hSurface == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, window handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }
    DBG_CURSOR_MSG("[DFB] %s %d, dst handle=%d\n",__FUNCTION__,__LINE__,hSurface);
    MI_RESULT mi_ret =MI_OSD_WindowFlipByExternSurface(hWindow, hSurface);
    if (mi_ret != MI_OK)
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
    return DFB_OK;
}

static DFBResult updateMiWindowRect(MI_HANDLE hLayer,MI_HANDLE hWindow,const MI_OSD_Rect_t *pstRect) {
    if (hWindow == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, window handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }


    MI_OSD_Rect_t Rect ={pstRect->u32X, pstRect->u32Y, pstRect->u32Width, pstRect->u32Height};
    /*
        we have set the MI window width and height while calling createCursoLayer function.
        The cursor shape size(w & h) can't greater than MI window size. Therefore, we need to make decisions in our code.
    */
    if (Rect.u32Width > dfb_config->mst_cursor_gwin_width || Rect.u32Height > dfb_config->mst_cursor_gwin_height )
    {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, src rect (width ,height) =(%d,%d) is greater than mst_cursor_gwin_width=%d, mst_cursor_gwin_height=%d , pid=%d\n",
            __FUNCTION__,__LINE__,pstRect->u32Width, pstRect->u32Height,  dfb_config->mst_cursor_gwin_width, dfb_config->mst_cursor_gwin_height,getpid());
        Rect.u32Width = dfb_config->mst_cursor_gwin_width;
        Rect.u32Height = dfb_config->mst_cursor_gwin_height;
    }

    DBG_CURSOR_MSG("[DFB]%s %d, Move cursor to x=%d, y=%d, w=%d, h=%d, gwinHnd=%d, pid=%d\n",
            __FUNCTION__,__LINE__,Rect.u32X, Rect.u32Y, Rect.u32Width, Rect.u32Height,gwinHnd,getpid());

    MI_RESULT mi_ret = MI_OK;

    mi_ret = MI_OSD_LayerBeginConfig(hLayer);
    if (mi_ret != MI_OK) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }
  
    /* multiplied by a magnification to fit panel size */
    MI_OSD_Rect_t Rect_output ={Rect.u32X * width_scale, pstRect->u32Y * height_scale, pstRect->u32Width * width_scale, pstRect->u32Height * height_scale};
    mi_ret = MI_OSD_WindowSetRect(hWindow, &Rect_output);
    /*
        If the Rect x, y,height and width =0, this setting to MI_OSD_WindowSetRect would return fail. Ignore this error!
        Why we set this settings? You can search "set window rect to position(0,0) and know the root cause"
    */
    if (mi_ret != MI_OK        &&
         Rect_output.u32X > 0         &&
         Rect_output.u32Y > 0         &&
         Rect_output.u32Height > 0    &&
         Rect_output.u32Width > 0 ) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }
    mi_ret = MI_OSD_LayerApplyConfig(hLayer);
    if (mi_ret != MI_OK) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }

    return DFB_OK;
}

static DFBResult createMiOsdLayer(const MI_OSD_LayerInfo_t pstLayerInfo, MI_HANDLE *phLayer, const int gop_id){
    MI_RESULT mi_ret = MI_OK;
    MI_HANDLE layerHandle = MI_HANDLE_NULL;
    MI_OSD_GetLayerHandleParams_t  params = {gop_id};
    MI_OSD_GetLayerHandle (&params  , &layerHandle);
    if(layerHandle != MI_HANDLE_NULL)
    {
        /* Forced to destroy old MI Layer, maybe it's not created from DFB*/
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d \33[0m,Handle has been created, destroy old layer, gop id=%d!\n",__FUNCTION__,__LINE__,gop_id);
        //DFB_MI_TRACE(MI_OSD_LayerDestroy(layerHandle),"MI_OSD_LayerDestroy",layerHandle);
        mi_ret = MI_OSD_LayerDestroy(layerHandle);
        if (mi_ret != MI_OK)
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail \33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);

         layerHandle = MI_HANDLE_NULL;
    }

    mi_ret = MI_OSD_LayerCreate(&pstLayerInfo, &layerHandle);
    if (mi_ret != MI_OK) {
        *phLayer = MI_HANDLE_NULL;
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_INVARG;
    }
    *phLayer = layerHandle;
    return DFB_OK;
}

static DFBResult enableTransClrEX(MI_HANDLE hLayer, bool bEnable) {

    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("[DFB] %s %d, layer handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }
    if (bEnable)
        DFB_MI_TRACE(MI_OSD_LayerEnableTransparentColor(hLayer),"MI_OSD_LayerEnableTransparentColor",hLayer);
    else
        DFB_MI_TRACE(MI_OSD_LayerDisableTransparentColor(hLayer),"MI_OSD_LayerDisableTransparentColor",hLayer);

    return DFB_OK;
}

// bVsyncMode: true     blocked in MiOsdApplyconfig util gop vsync arrived
// bVsyncMode: fasle    beginconfig & applyconfig will return directly.need to wait vsync use another api
static DFBResult SetMiOsdVsyncMode(MI_HANDLE hLayer, bool  bVsyncMode) {
    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("[DFB] %s %d, layer handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    MI_BOOL attrParams;
    if (bVsyncMode) {
        attrParams = TRUE;
    } else {
        attrParams = FALSE;
    }

    DFB_MI_TRACE(MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_CONFIG_WAIT_SYNC, &attrParams),"MI_OSD_LayerSetAttr",hLayer);

    return DFB_OK;
}

static DFBResult enableMultiAlpha(MI_HANDLE hLayer, MI_BOOL bEnable) {
    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("[DFB] %s %d, layer handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }
    if (bEnable)
       DFB_MI_TRACE(MI_OSD_LayerEnableGlobalAlpha(hLayer),"MI_OSD_LayerEnableGlobalAlpha",hLayer);
    else
        DFB_MI_TRACE(MI_OSD_LayerDisableGlobalAlpha(hLayer),"MI_OSD_LayerDisableGlobalAlpha",hLayer);

    return DFB_OK;
}

static DFBResult createCursoLayer(const int OsdWidth, const int OsdHeight, const int PanelWidth, const int PanelHeight, const int cursorSurWidth, const int cursorSurHeight, const int gop_id)
{
    MI_OSD_LayerInfo_t stLayerInfo;
    DFBResult ret = DFB_OK;
    MI_RESULT mi_ret = MI_OK;
    memset(&stLayerInfo, 0, sizeof(MI_OSD_LayerInfo_t));
    stLayerInfo.eLayerId = gop_id;
    stLayerInfo.eLayerSize = E_MI_OSD_LAYER_SIZE_CUSTOMIZE;
    stLayerInfo.stLayerCustomSize.u32X = 0;
    stLayerInfo.stLayerCustomSize.u32Y = 0;
    
    /* GOP2 not support magnification 
       create a panel size MI_OSD layer */
    stLayerInfo.stLayerCustomSize.u32LayerWidth = PanelWidth;
    stLayerInfo.stLayerCustomSize.u32LayerHeight = PanelHeight;
    stLayerInfo.stLayerCustomSize.u32DstWidth = PanelWidth;
    stLayerInfo.stLayerCustomSize.u32DstHeight = PanelHeight;

    /* Amplification from DFB layer to MI_OSD layer */
    width_scale = PanelWidth/OsdWidth;
    height_scale = PanelHeight/OsdHeight;

    ret = createMiOsdLayer(stLayerInfo, &glayerHnd, gop_id);
    if (ret){
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, OSD(w,h)=(%d,%d), PNL(w,h)=(%d,%d), ret=%s\n",
            __FUNCTION__,__LINE__, OsdWidth, OsdHeight, PanelWidth, PanelHeight,DirectResultString( ret ));
        return ret;
    }

    MI_OSD_WindowInfo_t stWindowInfo;
    memset(&stWindowInfo, 0, sizeof(MI_OSD_WindowInfo_t));
    stWindowInfo.hLayer = glayerHnd;
  
    /* GOP2 not support magnification 
       create an panel size MI_OSD window */
    stWindowInfo.u32SurfaceWidth = dfb_config->mst_cursor_gwin_width * width_scale;
    stWindowInfo.u32SurfaceHeight = dfb_config->mst_cursor_gwin_height * height_scale;
    stWindowInfo.eBufType = E_MI_OSD_WINDOW_BUFFER_EXTERNAL;
    stWindowInfo.bPixelAlpha = TRUE;
    stWindowInfo.eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB8888;
    stWindowInfo.stRect.u32X = 0;
    stWindowInfo.stRect.u32Y = 0;
    stWindowInfo.stRect.u32Width = PanelWidth;
    stWindowInfo.stRect.u32Height = PanelHeight;
    mi_ret = MI_OSD_WindowCreate(&stWindowInfo, &gwinHnd);
    if (mi_ret){
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }
    DBG_CURSOR_MSG("[DFB] %s, %d, createCursoLayer OK!, layer handle=%d, OSD(w,h)=(%d,%d), PNL(w,h)=(%d,%d), windle handle=%d\n",
        __FUNCTION__,__LINE__,glayerHnd, OsdWidth, OsdHeight, PanelWidth, PanelHeight, gwinHnd);

    //using MI_OSD_LayerConfigUpdated to get vsync
    SetMiOsdVsyncMode(glayerHnd, FALSE);
    // Disable HWCursor layer TransparentColor
    enableTransClrEX(glayerHnd, FALSE);
    // Disable HWCursor layer global alpha
    enableMultiAlpha(glayerHnd, FALSE);

    return DFB_OK;
}

static DFBResult getCursoLayerHandle(const int OsdWidth, const int OsdHeight, const int PanelWidth,const int PanelHeight, const int gop_id)
{
    MI_U32 u32Width, u32Height =0;
    MI_RESULT mi_ret = MI_OK;
    DBG_CURSOR_MSG("[DFB] %s %d, query MI layer and window handle, gop id=%d\n",__FUNCTION__,__LINE__, gop_id);
    MI_OSD_GetLayerHandleParams_t  params = {gop_id};
    mi_ret = MI_OSD_GetLayerHandle (&params  , &glayerHnd);
    if (mi_ret){
        DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }

    mi_ret = MI_OSD_LayerGetSize(glayerHnd, &u32Width, &u32Height);
    if (mi_ret){
        DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }

    if (u32Width != PanelWidth || u32Height != PanelWidth)
    {
        MI_OSD_LayerCustomSize_t stCustomSize;
        stCustomSize.u32X = 0;
        stCustomSize.u32Y = 0;
      
        /* GOP2 not support magnification 
          create an panel size MI_OSD layer */
        stCustomSize.u32LayerWidth = PanelWidth;
        stCustomSize.u32LayerHeight = PanelHeight;
        stCustomSize.u32DstWidth =  PanelWidth;
        stCustomSize.u32DstHeight = PanelHeight;
        DBG_CURSOR_MSG("[DFB] %s %d, change layer size, glayerHnd=%d, original OSD size(w,h)=(%d,%d), new OSD size(w,h)=(%d,%d)\n",
        __FUNCTION__,__LINE__, glayerHnd, u32Width, u32Height,PanelWidth,PanelHeight );

        mi_ret = MI_OSD_LayerSetCustomSize(glayerHnd, &stCustomSize);
        if (mi_ret){
            DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
            return DFB_FAILURE;
        }
    }

    mi_ret = MI_OSD_LayerQueryWindowHandle(glayerHnd, &gwinHnd);
    if (mi_ret){
        DBG_CURSOR_MSG("[DFB] %s %d fail, ret=0x%x\n",__FUNCTION__,__LINE__,mi_ret);
        return DFB_FAILURE;
    }

    /* Amplification from DFB layer to MI_OSD layer */
    width_scale = PanelWidth/OsdWidth;
    height_scale = PanelHeight/OsdHeight;

    DBG_CURSOR_MSG("[DFB] %s %d, glayerHnd=%d, OSD(w,h)=(%d,%d), PNL(w,h)=(%d,%d), windle handle=%d\n",
        __FUNCTION__,__LINE__, glayerHnd,OsdWidth,OsdHeight,PanelWidth,PanelHeight, gwinHnd );

    return DFB_OK;
}

static DFBResult destroyMiOsdSurface(MI_HANDLE hSurface) {
    if (hSurface == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("[DFB] %s %d, window handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    DFB_MI_TRACE(MI_OSD_SurfaceDestroy(hSurface),"MI_OSD_SurfaceDestroy",hSurface);
    hSurface = MI_HANDLE_NULL;
    return DFB_OK;
}

static DFBResult cursorMiStretchBlit(MI_HANDLE hSrcSurface, const MI_OSD_Rect_t *pstSrcRect,
                                MI_HANDLE hDstSurface, const MI_OSD_Rect_t *pstDstRect,
                                MI_BOOL pbAlphaBlending, MI_U8 planeAlpha) {
    if (hSrcSurface == MI_HANDLE_NULL || hDstSurface == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m \n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    DBG_CURSOR_MSG("[DFB]SrcRect (X,Y,W,H)=(%d,%d,%d,%d), DstRect (X,Y,W,H)=(%d,%d,%d,%d), dfb config cursor gwin(W,H)=(%d,%d), alpha=%x\n",
        pstSrcRect->u32X, pstSrcRect->u32Y, pstSrcRect->u32Width, pstSrcRect->u32Height,
        pstDstRect->u32X, pstDstRect->u32Y, pstDstRect->u32Width, pstDstRect->u32Height,
        dfb_config->mst_cursor_gwin_width, dfb_config->mst_cursor_gwin_height, planeAlpha);

    MI_RESULT mi_ret = MI_OK;
    MI_OSD_RenderJob_t stRenderJob;
    memset(&stRenderJob, 0, sizeof(MI_OSD_RenderJob_t));
    MI_OSD_BeginDraw();
    MI_OSD_SurfaceClear(hDstSurface, NULL, &stRenderJob);
    MI_OSD_EndDraw();
    MI_OSD_WaitRenderDone(&stRenderJob);

    MI_OSD_BlitOpt_t stBlitOpt;
    MI_OSD_GetDefaultBlitOpt(&stBlitOpt);
    stBlitOpt.eBlendMode = (pbAlphaBlending) ? E_MI_OSD_BLEND_ONE : stBlitOpt.eBlendMode;
    stBlitOpt.stConstColor.u8Blue = 0xFF;
    stBlitOpt.stConstColor.u8Green = 0xFF;
    stBlitOpt.stConstColor.u8Red = 0xFF;
    stBlitOpt.stConstColor.u8Alpha = planeAlpha;
    stBlitOpt.eSrcDfbBlendMode = E_MI_OSD_DFB_BLEND_SRC_ALPHA;
    stBlitOpt.eDstDfbBlendMode = E_MI_OSD_DFB_BLEND_ZERO;
    stBlitOpt.eDfbBlendFlag = (MI_OSD_DfbBlendFlag_e)(E_MI_OSD_DFB_BLEND_FLAG_COLOR_ALPHA|E_MI_OSD_DFB_BLEND_FLAG_ALPHA_CHANNEL);
    stBlitOpt.bIsDfbBlend = TRUE;

    memset(&stRenderJob, 0, sizeof(MI_OSD_RenderJob_t));

    MI_OSD_BeginDraw();
    mi_ret =MI_OSD_Bitblit(hSrcSurface, pstSrcRect, hDstSurface, pstDstRect, &stBlitOpt, &stRenderJob);
    MI_OSD_EndDraw();

    return DFB_OK;
}

static DFBResult setMiOsdConfig(MI_HANDLE hLayer, MI_BOOL beginConfig) {
    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, window handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    if (beginConfig)
        DFB_MI_TRACE(MI_OSD_LayerBeginConfig(hLayer),"MI_OSD_LayerBeginConfig",hLayer);
    else
        DFB_MI_TRACE(MI_OSD_LayerApplyConfig(hLayer),"MI_OSD_LayerApplyConfig",hLayer);

    return DFB_OK;
}

//MI_OSD_LayerConfigUpdated: it will  blocked in function internally util the gop vsync arrived
static DFBResult GetMiConfigUpdated(MI_HANDLE hLayer) {
    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, layer handle is NULL\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    MI_RESULT mi_ret = MI_OSD_LayerConfigUpdated(hLayer);

    if (mi_ret != MI_OK)
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m, GetMiConfigUpdated error or timeout, ret=0x%x\n",__FUNCTION__,__LINE__, mi_ret);


    return DFB_OK;
}

static DFBResult enableMiOsdLayerDisplay(MI_HANDLE hLayer, MI_BOOL enableShow) {
    if (hLayer == MI_HANDLE_NULL) {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d, layer handle is NULL\33[0m\n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
    }

    if (enableShow) {
        DBG_CURSOR_MSG("[DFB]%s %d, Show the layer, pid=%d\n", __FUNCTION__,__LINE__,getpid());
        DFB_MI_TRACE(MI_OSD_LayerShow(hLayer),"MI_OSD_LayerShow",hLayer);
    } else {
        DBG_CURSOR_MSG("[DFB]%s %d, Hide the layer, pid=%d\n", __FUNCTION__,__LINE__,getpid());
        DFB_MI_TRACE(MI_OSD_LayerHide(hLayer), "MI_OSD_LayerHide", hLayer);
    }
    return DFB_OK;
}

static MI_OSD_ColorFormat_e changeDFBPixelFormatToMIColorFormat(DFBSurfacePixelFormat    format){
    MI_OSD_ColorFormat_e eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB8888;
    switch( format )
        {
                case DSPF_ARGB1555:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB1555;
                        break;

                case DSPF_ARGB:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                        break;

                case DSPF_ABGR:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_ABGR8888;
                        break;

                case DSPF_YVYU:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_YUV422_YVYU;
                        break;

                case DSPF_YUY2:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_YUV422_YUYV;
                        break;

                case DSPF_UYVY:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_YUV422_UYVY;
                        break;

                case DSPF_LUT8:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_I8;

                        break;

                case DSPF_LUT4:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_I4;
                        break;

                case DSPF_LUT2:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_I2;
                        break;

                case DSPF_LUT1:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_I1;
                        break;

                case DSPF_ARGB4444:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB4444;
                        break;

                case DSPF_RGB16:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_RGB565;
                        break;

                case DSPF_A8:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_I8;
                        break;

                case DSPF_RGB32:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB8888;
                        break;

            case DSPF_BLINK12355:
                        eColorFormat = E_MI_OSD_COLOR_FORMAT_1ABFGBG12355;
                        break;

            case DSPF_BLINK2266:
                        eColorFormat= E_MI_OSD_COLOR_FORMAT_FABAFGBG2266;
                        break;
                default:
                        DBG_CURSOR_MSG( "[DFB] error format : %s (%s, %s)\n",
                                 dfb_pixelformat_name(format),
                                __FILE__,
                                __FUNCTION__ );

        }
         return eColorFormat;
}

static DFBResult hwcursorInit( CoreDFB *core,CoreWindowStack *stack, WindowSharedData *shared )
{
        CoreLayerContext *context =stack->context ;
        CoreLayerRegionConfig  config = context->primary.config;
        unsigned int layer_id = stack->context->layer_id;
        DFBResult ret = DFB_OK;


        /*
            If you have calledl fusion_ref_init, the ref.multi.id definitely gonna be larger than 0 and ref.multi.shared is not NULL.
            When you see ref.multi.id=0 and ref.multi.shared is NULL means nobody uses cursor to this date.
            Therefore, we will do fusion_ref_init, fusion_ref_watch and create MI layer.
            It is noteworthy that fusion_ref_init and fusion_ref_watch must be used in the same process,
            otherwise, kernel fusion would return can't access while calling fusion_ref_watch
        */
        if (shared->cursor_ref.multi.id ==0 && shared->cursor_ref.multi.shared == NULL)
        {
            ret = fusion_ref_init( &shared->cursor_ref, "cursor shared ", dfb_core_world(core) );
            if (ret){
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                return ret;
            }

            ret = fusion_ref_up( &shared->cursor_ref, false );
            if (ret){
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                return ret;
            }

            ret = fusion_ref_watch( &shared->cursor_ref, &shared->call, getpid());
            if (ret){
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fai\33[0ml, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                return ret;
            }

            DBG_CURSOR_MSG("[DFB] %s %d, it's first time to initialize the cursor_ref and  cursorMiInit, gop id=%d, pid=%d\n", __FUNCTION__, __LINE__, shared->cusor_gop_id,getpid());
            ret = createCursoLayer( stack->width,stack->height,config.dest.w,config.dest.h, stack->cursor.size.w, stack->cursor.size.h, shared->cusor_gop_id );
            shared->hw_cursor_shape[layer_id] = NULL;

            if(ret){
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));

                return ret;
            }
        }
        else
        {
            /*
                In this flow, it's mean someone has used curor, we only need to check refs count.
                If refs=0 means somebody has destroyed the layer and disable the cursor, recreate the MI layer & window layer.
                If refs is greater than 0 means someone already create MI layer & window, you just get  MI layer & window layer.
            */
            int          refs = -1;
            ret =  fusion_ref_stat(&shared->cursor_ref, &refs);
            if (ret == DFB_OK)
            {
                if (refs == 0)
                {
                    DBG_CURSOR_MSG("[DFB] %s %d, ref is 0, createCursoLayer, gop id=%d, pid=%d,\n", __FUNCTION__, __LINE__, shared->cusor_gop_id, getpid());
                    ret = createCursoLayer(stack->width,stack->height,config.dest.w,config.dest.h, stack->cursor.size.w, stack->cursor.size.h, shared->cusor_gop_id );

                    shared->hw_cursor_shape[layer_id] = NULL;

                    if(ret){
                        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                        return ret;
                    }
                    ret = fusion_ref_up( &shared->cursor_ref, false );

                    if (ret){
                        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                        return ret;
                    }
                }
                else{
                        DBG_CURSOR_MSG("[DFB] %s %d, ref is greater than 0, get MI layer & window handle, gop id=%d, refs=%d, pid=%d\n", __FUNCTION__, __LINE__, shared->cusor_gop_id, refs, getpid());
                        ret = getCursoLayerHandle(stack->width,stack->height, config.dest.w,config.dest.h, shared->cusor_gop_id);

                        if(ret){
                            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                            return ret;
                        }
                        fusion_ref_up(&shared->cursor_ref, false);
                }
            }
            else{
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
                return ret;
            }
        }
        return DFB_OK;
}

static DFBResult hwcursorDeInit( CoreDFB *core,CoreWindowStack *stack, WindowSharedData *shared, MI_HANDLE *hSrcSurface, MI_HANDLE *hDstSurface )
{
        unsigned int layer_id = stack->context->layer_id;
        CoreSurface *hw_cursor_surface = NULL;
        hw_cursor_surface = shared->hw_cursor_shape[layer_id];

        fusion_ref_down(&shared->cursor_ref, false);
        DBG_CURSOR_MSG("[DFB] %s,%d, disable hw cursor, layer id=%d, pid=%d\n",__FUNCTION__,__LINE__,layer_id, getpid());

        if (hSrcSurface != NULL && *hSrcSurface != MI_HANDLE_NULL){
            DBG_CURSOR_MSG("[DFB] %s,%d,destroy src MI surface handle, layer id=%d, pid=%d\n",__FUNCTION__,__LINE__,layer_id, getpid());
            destroyMiOsdSurface(*hSrcSurface);
            *hSrcSurface = MI_HANDLE_NULL;
        }

        /*
           when shared->cursor_ref is zero, DFB master will call MI_OSD_LayerDestroy. This MI API would help us destroy dst surface handle
           which is used in MI_OSD_WindowFlipByExternSurface. Thus, we don't need to call destroyMiOsdSurface when calling MI_OSD_LayerDestroy.
           Only need to destroy dst MI surface handle by ourselves when shared->cursor_ref is greater than 0.
        */
        int          refs = -1;
        DFBResult ret =  fusion_ref_stat(&shared->cursor_ref, &refs);
        if (ret == DFB_OK){
            if (hDstSurface != NULL && *hDstSurface != MI_HANDLE_NULL){
                if ( refs > 0 ) {
                    DBG_CURSOR_MSG("[DFB] %s,%d,destroy dst MI surface handle,refs=%d, layer id=%d, pid=%d\n",__FUNCTION__,__LINE__,refs, layer_id, getpid());
                    enableMiOsdLayerDisplay(glayerHnd, FALSE);
                    destroyMiOsdSurface(*hDstSurface);
                    *hDstSurface = MI_HANDLE_NULL;
                } else {
                    if (shared->hw_cursor_shape[layer_id] != NULL) {
                        dfb_surface_unref(shared->hw_cursor_shape[layer_id]);
                        shared->hw_cursor_shape[layer_id] = NULL;
                    }
                    fusion_ref_destroy(&shared->cursor_ref);
                    shared->cursor_ref.multi.id =0;
                    shared->cursor_ref.multi.shared = NULL;
                }
            }
        }


        if (ret)
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
        else
            DBG_CURSOR_MSG("[DFB] %s %d, deinit cursor flow end, refs=%d, layer id=%d, pid=%d\n",__FUNCTION__,__LINE__,refs,layer_id, getpid());




        return DFB_OK;
}

static DFBResult hwcursorUpdateShape( CoreDFB *core,CoreWindowStack *stack, WindowSharedData *shared, DFBRegion *cursor_region, MI_HANDLE *ret_hSrcSurface, MI_HANDLE *ret_hDstSurface )
{
        CoreSurfaceBufferLock  dst_lock;
        CoreSurfaceBufferLock  src_lock;
        MI_HANDLE hSrcSurface = *ret_hSrcSurface;
        MI_HANDLE hDstSurface = *ret_hDstSurface;
        hal_phy dst_phys, src_phys = 0;
        bool brecreateMIsurface = false;
        unsigned int layer_id = stack->context->layer_id;
        CoreSurface *hw_cursor_surface = NULL;
        hw_cursor_surface = shared->hw_cursor_shape[layer_id];
        DFBResult ret = DFB_OK;

        if (!stack->cursor.surface){
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail, because of NULL cursor surface,layer_id=%d,pid=%d\33[0m ,\n",__FUNCTION__,__LINE__,layer_id,getpid());
            return DFB_INVARG;
        }

        /* prepare the parameter for creating MI surface*/
        MI_OSD_SurfaceInfo_t stCursorSurfInfo;
        memset(&stCursorSurfInfo,0x00, sizeof(MI_OSD_SurfaceInfo_t));
        stCursorSurfInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;
        stCursorSurfInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        stCursorSurfInfo.bReArrange = FALSE;
        stCursorSurfInfo.eColorFormat = E_MI_OSD_COLOR_FORMAT_ARGB8888;

        if (hw_cursor_surface == NULL)
        {
            DBG_CURSOR_MSG("[DFB] %s %d, create flipping surface, layer_id=%d, pid=%d\n",__FUNCTION__,__LINE__,layer_id, getpid());
            /* create surface for flipping*/
            DFBSurfaceCapabilities  surface_caps = DSCAPS_PREMULTIPLIED | DSCAPS_VIDEOONLY;
            /* create an enlarged cursor surface in DFB */
            ret = dfb_surface_create_simple( core, dfb_config->mst_cursor_gwin_width * width_scale , dfb_config->mst_cursor_gwin_height * height_scale,
                                   DSPF_ARGB, DSCS_RGB,
                                   surface_caps, CSTF_SHARED | CSTF_CURSOR,
                                   0, NULL, &hw_cursor_surface);

            if (ret)
            {
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m , layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
                return ret;
            }
            shared->hw_cursor_shape[layer_id] = hw_cursor_surface;
            if(hDstSurface != MI_HANDLE_NULL) {
                destroyMiOsdSurface(hDstSurface);
                hDstSurface = MI_HANDLE_NULL;
            }
        }
        else{
            DBG_CURSOR_MSG("[DFB] %s %d, already create shared->hw_cursor_shape[%d]=%p \n",__FUNCTION__,__LINE__, layer_id, hw_cursor_surface);
        }


        if(hDstSurface == MI_HANDLE_NULL)
        {

            ret = dfb_surface_lock_buffer( hw_cursor_surface, CSBR_FRONT, CSAID_GPU, CSAF_WRITE, &dst_lock );
            if (ret)
            {
                DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
                return ret;
            }
          
            /* Create an enlarged cursor surface which buffer is from DFB */
            dst_phys = _BusAddrToHalAddr(((u64)dst_lock.phys_h << SHIFT32) | dst_lock.phys);
            stCursorSurfInfo.phyAddr = (MI_PHY)dst_phys;
            stCursorSurfInfo.u32Pitch = dst_lock.pitch;
            stCursorSurfInfo.u32Width = dfb_config->mst_cursor_gwin_width * width_scale;
            stCursorSurfInfo.u32Height = dfb_config->mst_cursor_gwin_height * height_scale;
            createMiOsdSurface(&stCursorSurfInfo, &hDstSurface);
            dfb_surface_unlock_buffer( hw_cursor_surface, &dst_lock );
            DBG_CURSOR_MSG("[DFB] %s %d, dst handle=%d, dst addr=0x%08llx, layer_id=%d, pid=%d\n",
                __FUNCTION__,__LINE__, hDstSurface,dst_phys, layer_id,getpid());
        }


        if (hSrcSurface != MI_HANDLE_NULL)
        {
            destroyMiOsdSurface(hSrcSurface);
            hSrcSurface = MI_HANDLE_NULL;
        }

        ret = dfb_surface_lock_buffer( stack->cursor.surface, CSBR_FRONT, CSAID_GPU, CSAF_READ, &src_lock );
        if (ret)
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            return ret;
        }
        ret = dfb_surface_lock_buffer( hw_cursor_surface, CSBR_FRONT, CSAID_GPU, CSAF_READ, &dst_lock );
        if (ret)
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            return ret;
        }

        src_phys =  _BusAddrToHalAddr(((u64)src_lock.phys_h << SHIFT32) | src_lock.phys);

        DBG_CURSOR_MSG("[DFB] %s %d, src handle=%d, pixel format=%s, layer id=%u, pid=%d\n",
            __FUNCTION__,__LINE__, hSrcSurface, dfb_pixelformat_name(stack->cursor.surface->config.format),layer_id, getpid());

        stCursorSurfInfo.eColorFormat = changeDFBPixelFormatToMIColorFormat(stack->cursor.surface->config.format);
        stCursorSurfInfo.phyAddr = (MI_PHY)src_phys;
        stCursorSurfInfo.u32Pitch = src_lock.pitch;
        stCursorSurfInfo.u32Width = stack->cursor.size.w;
        stCursorSurfInfo.u32Height = stack->cursor.size.h;
        createMiOsdSurface(&stCursorSurfInfo, &hSrcSurface);

        DBG_CURSOR_MSG("[DFB] %s %d, src handle=%d, dst handle=%d, layer id=%u, pid=%d\n",__FUNCTION__,__LINE__, hSrcSurface,hDstSurface, layer_id,getpid());

        MI_OSD_Rect_t stSrcRect;
        MI_OSD_Rect_t stDstRect;

        stSrcRect.u32X = 0;
        stSrcRect.u32Y = 0;
        stSrcRect.u32Width = stack->cursor.size.w;
        stSrcRect.u32Height = stack->cursor.size.h;
        if (stSrcRect.u32Width > dfb_config->mst_cursor_gwin_width  || stSrcRect.u32Height > dfb_config->mst_cursor_gwin_height )
        {
            stDstRect.u32X =0;
            stDstRect.u32Y = 0;
            stDstRect.u32Width = dfb_config->mst_cursor_gwin_width;
            stDstRect.u32Height = dfb_config->mst_cursor_gwin_height;
        }
        else
            stDstRect = stSrcRect;

        /* stretch blit to an enlarged cursor surface */
        stDstRect.u32Width *= width_scale;
        stDstRect.u32Height *= height_scale;

        DBG_CURSOR_MSG("[DFB] %s %d, src addr=0x%08llx, pitch=%d ,dst addr=0x%08llx, pitch=%d, opacity=%x, pid=%d\n",
            __FUNCTION__,__LINE__,src_phys, src_lock.pitch, _BusAddrToHalAddr(((u64)dst_lock.phys_h << SHIFT32) | dst_lock.phys), dst_lock.pitch, stack->cursor.opacity, getpid());

        cursorMiStretchBlit( hSrcSurface, &stSrcRect,
                            hDstSurface, &stDstRect,
                            TRUE, (MI_U8)stack->cursor.opacity);

        /*
            set window rect to position(0,0)
            when calling doMiOsdWindowFlip, MI OSD will check if window rect is on the position(0,0) or not.
            If it's not on the origin(0,0) , MI OSD will caculate the offset of the flipping surface.
        */
        DFBRegion               dest;
        transform_stack_to_dest( stack, cursor_region, &dest );
        MI_OSD_Rect_t stRect = {0};
        stRect.u32X = 0;
        stRect.u32Y =0;
        stRect.u32Width  = 0;
        stRect.u32Height  =0;

        updateMiWindowRect(glayerHnd, gwinHnd, &stRect);
        doMiOsdWindowFlip(gwinHnd, hDstSurface);

        /* move cursor to spefic position */
        stRect.u32X = dest.x1;
        stRect.u32Y = dest.y1;
        stRect.u32Width  = dest.x2 - dest.x1 +1;
        stRect.u32Height  = dest.y2 - dest.y1 +1;

        updateMiWindowRect(glayerHnd, gwinHnd, &stRect);

        dfb_surface_unlock_buffer( stack->cursor.surface, &src_lock );
        dfb_surface_unlock_buffer( hw_cursor_surface, &dst_lock );

        *ret_hSrcSurface = hSrcSurface;
        *ret_hDstSurface = hDstSurface;
        return DFB_OK;
}

static DFBResult hwcursorSetOpacity( CoreWindowStack *stack, WindowSharedData *shared, DFBRegion *cursor_region, MI_HANDLE hSrcSurface, MI_HANDLE hDstSurface )
{

        if (hSrcSurface == MI_HANDLE_NULL || hDstSurface == MI_HANDLE_NULL )
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d\33[0m hSrcSurface or hDstSurface us NULL, SetOpacity fail,pid=%d\n",__FUNCTION__,__LINE__, getpid());
            return DR_INVARG;
        }

        DFBResult ret = DFB_OK;
        CoreSurfaceBufferLock  src_lock,dst_lock ;
        unsigned int layer_id = stack->context->layer_id;
        CoreSurface *hw_cursor_surface = NULL;
        hw_cursor_surface = shared->hw_cursor_shape[layer_id];
        ret = dfb_surface_lock_buffer( stack->cursor.surface, CSBR_FRONT, CSAID_GPU, CSAF_READ, &src_lock );
        if (ret)
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            return ret;
        }
        ret = dfb_surface_lock_buffer( hw_cursor_surface, CSBR_FRONT, CSAID_GPU, CSAF_WRITE, &dst_lock );
        if (ret)
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            return ret;
        }
        MI_OSD_Rect_t stSrcRect;
        MI_OSD_Rect_t stDstRect;

        stSrcRect.u32X = 0;
        stSrcRect.u32Y = 0;
        stSrcRect.u32Width = stack->cursor.size.w;
        stSrcRect.u32Height = stack->cursor.size.h;
        stDstRect = stSrcRect;

        /* stretch blit to an enlarged cursor surface */
        stDstRect.u32Width *= width_scale;
        stDstRect.u32Height *= height_scale;

        cursorMiStretchBlit( hSrcSurface, &stSrcRect,
                            hDstSurface, &stDstRect,
                            TRUE, (MI_U8)stack->cursor.opacity);

        /*
            set window rect to position(0,0)
            when calling doMiOsdWindowFlip, MI OSD will check if window rect is on the position(0,0) or not.
            If it's not on the origin(0,0) , MI OSD will caculate the offset of the flipping surface.
        */
        DFBRegion               dest;
        transform_stack_to_dest( stack, cursor_region, &dest );
        MI_OSD_Rect_t stRect = {0};
        stRect.u32X = 0;
        stRect.u32Y =0;
        stRect.u32Width  = 0;
        stRect.u32Height  = 0;

        updateMiWindowRect(glayerHnd, gwinHnd, &stRect);
        doMiOsdWindowFlip(gwinHnd, hDstSurface);

        /* move cursor to spefic position */
        stRect.u32X = dest.x1;
        stRect.u32Y = dest.y1;
        stRect.u32Width  = dest.x2 - dest.x1 +1;
        stRect.u32Height  = dest.y2 - dest.y1 +1;
        DBG_CURSOR_MSG("[DFB]%s %d, Move cursor to x=%d, y=%d, w=%d, h=%d, gwinHnd=%d, pid=%d\n",
            __FUNCTION__,__LINE__,stRect.u32X, stRect.u32Y, stRect.u32Width, stRect.u32Height,gwinHnd,getpid());
        updateMiWindowRect(glayerHnd, gwinHnd, &stRect);


        ret = dfb_surface_unlock_buffer( stack->cursor.surface, &src_lock );
        if (ret)
        {

            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            dfb_surface_unlock_buffer( hw_cursor_surface, &dst_lock );
            return ret;
        }

        ret = dfb_surface_unlock_buffer( hw_cursor_surface, &dst_lock );
        if (ret)
        {
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d fail\33[0m ,layer_id=%d,ret=%s\n",__FUNCTION__,__LINE__, layer_id,DirectResultString( ret ));
            return ret;
        }

        return DFB_OK;

}

DFBResult dfb_wm_update_hwcursor( CoreDFB *core,CoreWindowStack *stack, CoreCursorUpdateFlags  flags, DFBRegion *cursor_region, void *shared_data )
{
    WindowSharedData *shared = shared_data;
    static bool isdebug =false;
    static bool bLayerShow_ori = false;

    /*
    If we call MI_OSD_SetDebugLevel means we can see more MI OSD log on the console,
    By the way, If you don't see the MI OSD log on the console, try to input the following command on the console(runtime input)
    =======================================
    echo dbg MI_OSD 0xf0 > /sys/kernel/mik/MI_UTIL
    echo 7 > /proc/sys/kernel/printk
    =======================================
    */
    if (!isdebug){
        MI_OSD_SetDebugLevel (MI_OSD_DEBUG_LEVEL );
        isdebug =true;
    }

    /* set MI handle*/
    static MI_HANDLE hCursorSrcSurfHnd = MI_HANDLE_NULL;
    static MI_HANDLE hCursorDstSurfHnd = MI_HANDLE_NULL;
    MI_BOOL bLayerShow = TRUE;
    CoreCursorUpdateFlags new_flags = flags;


    if (glayerHnd  == MI_HANDLE_NULL &&  gwinHnd  == MI_HANDLE_NULL )
        new_flags |= CCUF_ENABLE;

    DFBResult ret =DFB_OK;

    ret= fusion_skirmish_prevail( &shared->cursor_lock );
    if(ret){
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
        return ret;
    }

    DBG_CURSOR_MSG("[DFB]%s %d, original flag=0x%08x, new flag=0x%08x,pid=%d\n",__FUNCTION__,__LINE__,flags,new_flags,getpid());

    if ( new_flags & CCUF_ENABLE) {
        /*
         DESCRIPTION:
         CCUF_ENABLE means enable cursor,
         DFB will create new layer or get the layer if it has been created.1
        */
        ret = hwcursorInit( core, stack, shared );
        if(ret){

            fusion_skirmish_dismiss( &shared->cursor_lock );
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
            return ret;
        }

        new_flags |= CCUF_SHAPE;
    }

    DBG_CURSOR_MSG("[DFB] %s %d, flag=0x%08x, pid=%d\n",__FUNCTION__,__LINE__,new_flags,getpid());

    if (new_flags & CCUF_DISABLE) {
        /*
         DESCRIPTION:
         CCUF_DISABLE means disable cursor,
         DFB would use fusion_ref_stat to check if somebody still use cursor or not.
         If the output refs is 0, destroy all the resources.
        */
        ret = hwcursorDeInit( core, stack, shared, &hCursorSrcSurfHnd, &hCursorDstSurfHnd );
        if(ret)
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));

        glayerHnd = MI_HANDLE_NULL;
        gwinHnd = MI_HANDLE_NULL;

        fusion_skirmish_dismiss( &shared->cursor_lock );

        /*
            If the process calls disable cursor which means this process doesn't have any right to control cursor,
            DFB directly returns the function.
        */
        return ret;
    }


    if (new_flags & (CCUF_SHAPE | CCUF_SIZE)) {
        /*
         DESCRIPTION:
         DFB copys cursor surface to flipping surface and the displays the cursor.
        */
        ret =hwcursorUpdateShape( core, stack, shared, cursor_region, &hCursorSrcSurfHnd, &hCursorDstSurfHnd );
        if(ret){

            fusion_skirmish_dismiss( &shared->cursor_lock );
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
            return ret;
        }
    }


    if (new_flags & CCUF_OPACITY) {
        ret =hwcursorSetOpacity( stack, shared, cursor_region, hCursorSrcSurfHnd, hCursorDstSurfHnd );
        if(ret){

            fusion_skirmish_dismiss( &shared->cursor_lock );
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
            return ret;
        }
    }

    if (new_flags & CCUF_POSITION) {

        /*
         DESCRIPTION:
         CCUF_POSITION means move cursor to spefic position,
         Input the cursor position and the range, GOP would reference the parameters and move the gwin.
        */
        DFBRegion               dest;
        transform_stack_to_dest( stack, cursor_region, &dest );
        MI_OSD_Rect_t stRect = {0};
        stRect.u32X = dest.x1;
        stRect.u32Y = dest.y1;
        stRect.u32Width  = dest.x2 - dest.x1 +1;
        stRect.u32Height  = dest.y2 - dest.y1 +1;
        DBG_CURSOR_MSG("[DFB]%s %d, Move cursor to x=%d, y=%d, w=%d, h=%d,gwinHnd=%d,pid=%d\n",
            __FUNCTION__,__LINE__,stRect.u32X, stRect.u32Y, stRect.u32Width, stRect.u32Height,gwinHnd,getpid());
        updateMiWindowRect(glayerHnd, gwinHnd, &stRect);
    }

    if (stack->cursor.opacity == 0x00) {
        DBG_CURSOR_MSG("[DFB]%s %d, Hide the layer, pid=%d\n", __FUNCTION__,__LINE__,getpid());
        bLayerShow = false;
    } else {
        DBG_CURSOR_MSG("[DFB]%s %d, Show the layer, pid=%d\n", __FUNCTION__,__LINE__,getpid());
        bLayerShow = true;
    }

    if (bLayerShow_ori != bLayerShow) {
        enableMiOsdLayerDisplay(glayerHnd, bLayerShow);
        bLayerShow_ori = bLayerShow;
    }

    /* In mixed mode, MI_OSD_LayerConfigUpdated is not needed, the registers are updated automatically */
    //GetMiConfigUpdated(glayerHnd);

    ret = fusion_skirmish_dismiss( &shared->cursor_lock );
    if(ret){
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s %d fail\33[0m, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
        return ret;
    }

    return DFB_OK;
}