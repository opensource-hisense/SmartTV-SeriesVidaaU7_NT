// <MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
// <MStar Software>

//#ifdef MSTAR_DEBUG_LAYER
#define DIRECT_ENABLE_DEBUG
//#endif


#include <config.h>

#include <stdio.h>
#include <dlfcn.h>
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


#include <assert.h>

#include "mstar.h"

#include "mstar_screen.h"

#include "mstar_layer.h"

#include "drm_fourcc.h"

#include "mtk_tv_drm.h"

// for MI_RENDER
#define USE_MI_RENDER
#ifdef USE_MI_RENDER
#include "mi_common.h"
#include "mi_render.h"
#endif

// for PQ
#define USE_GRAPHIC_PQ
#ifdef USE_GRAPHIC_PQ
#include "mapi_pq_output_format_if.h"
#include "mapi_pq_cfd_gop_if.h"
#include "pqmap_utility_loader.h"

enum ENUM_COLOR_INDEX {
    COLOR_INDEX_R,
    COLOR_INDEX_G,
    COLOR_INDEX_B
};
#endif


D_DEBUG_DOMAIN(MSTAR_Layer, "MSTAR/Layer", "MSTAR Layers");



#define FRAME_PACKAGE_WIDTH_1920  1920
#define FRAME_PACKAGE_WIDTH_1280  1280
#define FRAME_PACKAGE_HEIGHT_1080 1080
#define FRAME_PACKAGE_HEIGHT_720  720

#undef ALIGN
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)


#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#ifndef USE_MI_RENDER

#define DRM_ATOMIC_COMMIT( mode )  \
{ \
    int cnt=0; \
    while (drmSetMaster(drmdata->drm_fd) ) {\
        printf( "[DFB] drmSetMaster fail (%d): %m\n", errno);\
        if (++cnt > 10) break; \
        /*return errno;*/\
    }	\
\
    ret = drmModeAtomicCommit(drmdata->drm_fd, req, mode, NULL);\
    drmModeAtomicFree(req);\
    if ( ret ) {\
        printf( "[DFB] drmModeAtomicCommit fail!! errno=%d\n", errno);\
        /*return errno;*/\
    }\
\
    if (drmDropMaster(drmdata->drm_fd)) {\
        printf( "[DFB] drmDropMaster fail (%d): %m\n", errno);\
        /*return errno;*/\
    }\
}

#endif

#define SHIFT4 4
#define SHIFT5 5
#define SHIFT6 6
#define SHIFT8 8
#define SHIFT16 16
#define SHIFT32 32
#define WAIT_TIME 1000
#define MI_RENDER_DEBUG_LEVEL 0xF0
#define DEVICE_NAME_SIZE 255
#define BUF_SIZE 24
#define CONTRAST_INIT 50
#define MAX_ALPHA 0xFF
#define NEW_ZPOS (drmdata->plane_num - 2 - slay->gop_index)
#define PIXEL_ALIGNMENT 4
#define TRIPLE_BUFFER 3
#define DOUBLE_BUFFER 2
#define SINGLE_BUFFER 1
#define ALLOC_IDLE 0
#define ALLOC_BACK 1
#define ALLOC_FRONT 2
#define REALLOC_FRONT 0
#define REALLOC_BACK  1
#define REALLOC_IDLE  2
#define HANDLE_NUM 4
#define TIMEOUT 100
#define TWO_FRAME 2
#define NSEC 1000000

/**********************************************************************************************************************/



static uint32_t get_property_id(int fd, drmModeObjectProperties *props, const char *name)
{
	drmModePropertyPtr property;
	uint32_t i, id = 0;

	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		if (!strcmp(property->name, name))
			id = property->prop_id;
		drmModeFreeProperty(property);

		if (id)
			break;
	}

	return id;
}

static uint64_t get_property_value_byName(int fd, drmModeObjectProperties *props, const char *name)
{
    uint64_t value = 0;
    drmModePropertyPtr prop;
    int i;

    for (i=0; i<props->count_props; i++)
    {
        prop = drmModeGetProperty(fd, props->props[i]);
        if (prop != NULL)
        {
            if (strcmp(prop->name, name) == 0)
                value = props->prop_values[i];

            drmModeFreeProperty(prop);

            if (value)
                break;
        }
    }

    return value;
}

/******************************************************************************/

#ifdef USE_GRAPHIC_PQ


#define ISPARAMSSAME(a, b) \
    ({ \
        bool val = true; \
        if ((a) != (b)) { \
            (a) = (b); \
            val = false; \
        } \
        val; \
    })


#define DEBUG_WCG_HDR (0)

#define WCG_HDR_CFD_PARAMS_HARDCODE (1)
#define WCG_HDR_PANEL_COLOR_METRY_HARDCODE (1)
#define WCG_HDR_COLOR_INDEX_R (0)
#define WCG_HDR_COLOR_INDEX_G (1)
#define WCG_HDR_COLOR_INDEX_B (2)
#define WCG_HDR_CFD_ML_CMD_SIZE (6)
#define WCG_HDR_SCALING_RATIO_SHIFT (32)

#if WCG_HDR_CFD_PARAMS_HARDCODE
#define CFD_HAEDCODE_UI_PARAM_HUE (50)
#define CFD_HAEDCODE_UI_PARAM_SATURATION (128)
#define CFD_HAEDCODE_UI_PARAM_CONTRAST (1024)
#define CFD_HAEDCODE_UI_PARAM_BRIGHTNESS (1024)
#define CFD_HAEDCODE_UI_PARAM_RGBGAIN    (1024)

#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_R (0x7D00)
#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_G (0x3A98)
#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_B (0x1D4C)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_R (0x4047)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_G (0x7530)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_B (0x0BB8)
#define CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_X (0x3D13)
#define CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_Y (0x4042)
#else
#if WCG_HDR_PANEL_COLOR_METRY_HARDCODE
#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_R (0x7D00)
#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_G (0x3A98)
#define CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_B (0x1D4C)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_R (0x4047)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_G (0x7530)
#define CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_B (0x0BB8)
#define CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_X (0x3D13)
#define CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_Y (0x4042)
#endif
#endif

enum CFD_INPUT_STRUCT_PARAMS_MEMBER {
    CFD_UI_PARAM,
    CFD_TMO_PARAM,
    CFD_BRIGHTNESS_GAIN,
    CFD_FMT_PARAMS,
    CFD_XVYCC_PARAMS,
    CFD_XVYCC_COLORMETRY_SOURCE,
    CFD_XVYCC_COLORMETRY_TARGET,
    CFD_XVYCC_EOTF_PARAMS,
    CFD_XVYCC_OETF_PARAMS,
    CFD_OFFSET_PARAMS,
    CFD_CLIPPING_PARMS,
    CFD_PARAMS_MAX
};

struct drm_mtk_tv_graphic_pq_setting *PqCfdSetting = NULL;
struct ST_PQ_CFD_GOP_PARAMS *CfdGopProcessIn = NULL;
struct ST_PQ_CFD_GOP_PARAMS *LastCfdGopProcessIn = NULL;
ST_PQ_CFD_GOP_SETTNG_TABLE *CfdGopsettingTable = NULL;

stPqmapTrigParams *PqmapTrigParam = NULL;
stPqmapTrigParams *PqmapLastTrigParam = NULL;
graphic_presetdb_info PresetdbInfos = {0};
uint32_t CfdParamsSize[CFD_PARAMS_MAX+1] = {0};

#define PANEL_LUMINACNCE 500

uint64_t panelLuminance = PANEL_LUMINACNCE;



#if DEBUG_WCG_HDR
void dumpCfdParams(uint32_t idx) {
    DBG_LAYER_MSG( "----------MApi_PQ_CFD_GOP_Process CfdGopProcessIn[%d] info-------------\n", idx);
    DBG_LAYER_MSG( "pst_ui_params->u16Hue = 0x%x\n", CfdGopProcessIn[idx].pst_ui_params->u16Hue);
    DBG_LAYER_MSG( "pst_ui_params->u16Saturation = 0x%x\n", CfdGopProcessIn[idx].pst_ui_params->u16Saturation);
    DBG_LAYER_MSG( "pst_ui_params->u16Contrast = 0x%x\n", CfdGopProcessIn[idx].pst_ui_params->u16Contrast);
    DBG_LAYER_MSG( "pst_ui_params->bIsColorInversionEnable = %d\n", CfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable);
    DBG_LAYER_MSG( "pst_ui_params->bIsColorCorrectionEnable = %d\n", CfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable);
    DBG_LAYER_MSG( "pst_ui_params->u8CorrectionMode = %d\n", CfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode);

    DBG_LAYER_MSG( "pst_tmo_params->u8HDRMode = %d\n", CfdGopProcessIn[idx].pst_tmo_params->u8HDRMode);
    DBG_LAYER_MSG( "pst_tmo_params->u8TMOMode = %d\n", CfdGopProcessIn[idx].pst_tmo_params->u8TMOMode);
    DBG_LAYER_MSG( "pst_tmo_params->u16PanelMax = 0x%x\n", CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);
    DBG_LAYER_MSG( "pst_tmo_params->u16Samplesize = %d\n", CfdGopProcessIn[idx].pst_tmo_params->u16Samplesize);
    DBG_LAYER_MSG( "pst_tmo_params->pu32SrcNits_Address = %p\n", CfdGopProcessIn[idx].pst_tmo_params->pu32SrcNits_Address);
    DBG_LAYER_MSG( "pst_tmo_params->pu32TgtNits_Address = %p\n", CfdGopProcessIn[idx].pst_tmo_params->pu32TgtNits_Address);

    DBG_LAYER_MSG( "pst_brightness_gain_params->u16User_gain_sdr = %d\n", CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_sdr);
    DBG_LAYER_MSG( "pst_brightness_gain_params->u16User_gain_pq = %d\n", CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_pq);
    DBG_LAYER_MSG( "pst_brightness_gain_params->u16User_gain_hlg = 0x%x\n", CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_hlg);

    DBG_LAYER_MSG( "pst_color_fmt_params->u8HDRMode = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8InputRange = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8InputColorSpace = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8InputDataFormat = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8TrasferTC = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8TrasferTC);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8OutputDataFormat = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8OutputRange = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange);
    DBG_LAYER_MSG( "pst_color_fmt_params->u8OutputColorSpace = %d\n", CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace);

    DBG_LAYER_MSG( "pst_xvycc_info_params->u83x3Mode = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode);
    DBG_LAYER_MSG( "pst_xvycc_info_params->b3x3Enable = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->b3x3Enable);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_source_colormetry = %p\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_source_colormetry);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[0] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[1] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[2] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[0] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[1] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[2] = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B]);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16White_point_x = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_target_colormetry->u16White_point_y = 0x%x\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pu163x3_addr = %p\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pu163x3_addr);
    DBG_LAYER_MSG( "pst_xvycc_info_params->bXvycc_curve_Manual_Mode = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode);
    DBG_LAYER_MSG( "pst_xvycc_info_params->bEotf_ExtendMode = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->bEotf_ExtendMode);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_eotf_params->u16LUT_Size = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_eotf_params->u16LUT_Size);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_eotf_params->pu32EotfLut_Address = %p\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_eotf_params->pu32EotfLut_Address);
    DBG_LAYER_MSG( "pst_xvycc_info_params->bOetf_ExtendMode = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->bOetf_ExtendMode);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_oetf_params->u16LUT_Size = %d\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_oetf_params->u16LUT_Size);
    DBG_LAYER_MSG( "pst_xvycc_info_params->pst_oetf_params->pu32OetfLut_Address = %p\n", CfdGopProcessIn[idx].pst_xvycc_info_params->pst_oetf_params->pu32OetfLut_Address);

    DBG_LAYER_MSG( "pst_rgb_offset_params->bRGBOffsetEnable = %d\n", CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable);
    DBG_LAYER_MSG( "pst_rgb_offset_params->bRGBOffsetManualMode = %d\n", CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode);
    DBG_LAYER_MSG( "pst_rgb_offset_params->pu16RGBOffset_addr = %p\n", CfdGopProcessIn[idx].pst_rgb_offset_params->pu16RGBOffset_addr);

    DBG_LAYER_MSG( "pst_rgb_clipping_params->bRGBClippingManualMode = %d\n", CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode);
    DBG_LAYER_MSG( "pst_rgb_clipping_params->bRGBClippingEnable = %d\n", CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable);
    DBG_LAYER_MSG( "pst_rgb_clipping_params->pu16RGB_min_limit_addr = %p\n", CfdGopProcessIn[idx].pst_rgb_clipping_params->pu16RGB_min_limit_addr);
    DBG_LAYER_MSG( "pst_rgb_clipping_params->pu16RGB_max_limit_addr = %p\n", CfdGopProcessIn[idx].pst_rgb_clipping_params->pu16RGB_max_limit_addr);
    DBG_LAYER_MSG( "---------------------------------------------------------------------------\n");
}

#define INDEX_BYTE_CNT (4)
#define VALUE_BYTE_CNT (2)
#define BIT_PER_BYTE (8)

void dumpPqCfdSettings(uint32_t Id) {

    DBG_LAYER_MSG( "----------------------- PqCfdSetting[%d] info--------------------------\n", Id);
    DBG_LAYER_MSG( "-----------------------pq ml settings dump start--------------------------\n");

    char *scalingCbwriteAddr = (char *)(PqCfdSetting[Id].u64buf_pq_ml_addr);
    DBG_LAYER_MSG( "Id=%d, PqCfdSetting.u64buf_pq_ml_addr=0x%lx\n", Id, PqCfdSetting[Id].u64buf_pq_ml_addr);
    for (int i = 0; i < (PqCfdSetting[Id].u32buf_pq_ml_size / WCG_HDR_CFD_ML_CMD_SIZE); i++) {
       uint32_t idx = 0;
       uint16_t value = 0;
       for (int j = 0; j < INDEX_BYTE_CNT; j++) {
           idx = idx | (((uint32_t)(scalingCbwriteAddr[WCG_HDR_CFD_ML_CMD_SIZE * i + j])) << (BIT_PER_BYTE * j));
       }
       for (int j = 0; j < VALUE_BYTE_CNT; j++) {
           value = value | (((uint32_t)(scalingCbwriteAddr[WCG_HDR_CFD_ML_CMD_SIZE * i + j + INDEX_BYTE_CNT])) << (BIT_PER_BYTE * j));
       }
       DBG_LAYER_MSG( "reg[%d].idx = 0x%x, .value = 0x%x  ", i, idx, value);
    }
    DBG_LAYER_MSG( "\n-----------------------pq ml settings dump end--------------------------\n\n");

    DBG_LAYER_MSG( "-----------------------cfd ml settings dump start--------------------------\n PqCfdSetting[idx].u32buf_cfd_ml_size=%d,\n", PqCfdSetting[Id].u32buf_cfd_ml_size);
    scalingCbwriteAddr = (char *)(PqCfdSetting[Id].u64buf_cfd_ml_addr);
    DBG_LAYER_MSG( "Id=%d, PqCfdSetting.u64buf_cfd_ml_addr=0x%lx\n", Id, PqCfdSetting[Id].u64buf_cfd_ml_addr);
    for (int i = 0; i < (PqCfdSetting[Id].u32buf_cfd_ml_size / WCG_HDR_CFD_ML_CMD_SIZE); i++) {
       uint32_t idx = 0;
       uint16_t value = 0;
       for (int j = 0; j < INDEX_BYTE_CNT; j++) {
           idx = idx | (((uint32_t)(scalingCbwriteAddr[WCG_HDR_CFD_ML_CMD_SIZE * i + j])) << (BIT_PER_BYTE * j));
       }
       for (int j = 0; j < VALUE_BYTE_CNT; j++) {
           value = value | (((uint32_t)(scalingCbwriteAddr[WCG_HDR_CFD_ML_CMD_SIZE * i + j + INDEX_BYTE_CNT])) << (BIT_PER_BYTE * j));
       }
       DBG_LAYER_MSG( "reg[%d].idx = 0x%x, .value = 0x%x  ", i, idx, value);
    }
    DBG_LAYER_MSG( "\n-----------------------cfd ml settings dump end--------------------------\n\n");

    DBG_LAYER_MSG( "-----------------------cfd adl settings dump start--------------------------\nPqCfdSetting[%d].u32buf_cfd_adl_size=%d\n", Id, PqCfdSetting[Id].u32buf_cfd_adl_size);
    scalingCbwriteAddr = (char *)(PqCfdSetting[Id].u64buf_cfd_adl_addr);
    DBG_LAYER_MSG( "ID=%d\n", Id);
    for (int i = 0; i < PqCfdSetting[Id].u32buf_cfd_adl_size; i++)
       DBG_LAYER_MSG( "value[%d] = 0x%x\n", i, scalingCbwriteAddr[i]);
    DBG_LAYER_MSG( "\n-----------------------cfd adl settings dump end--------------------------\n");
    DBG_LAYER_MSG( "---------------------------------------------------------------------------\n");

}
#endif


void getPanelLuminance(MTKDRMData *drmdata)
{
    DBG_LAYER_MSG("[DFB][%s %d] drmdata->plane_idx=%d (pid = %d)\n", __FUNCTION__, __LINE__, drmdata->plane_idx , getpid());
    drmModePlaneRes *plane_res = drmdata->plane_resources;
    uint32_t plane_id =  plane_res->planes[drmdata->plane_idx];
    // get Panel Luminance.
    drmModeObjectProperties *props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);


    if (!props) {
        printf("No properties: %s.\n", strerror(errno));
        return;
    }

    panelLuminance = get_property_value_byName(drmdata->drm_fd, props, "PNL_CURLUM");
    DBG_LAYER_MSG("[DFB] getPanelLuminance = %ld\n", panelLuminance);
    drmModeFreeObjectProperties(props);

}

void initCfdInputParam(struct ST_PQ_CFD_GOP_PARAMS **targetAddr)
{
    char *tmpAddr = NULL;
    uint32_t tmpOffset = 0;
    uint32_t mallocSize = 0;
    u8 gopCount = dfb_config->mst_gop_counts;

    mallocSize = gopCount * (sizeof(struct ST_PQ_CFD_GOP_PARAMS) + CfdParamsSize[CFD_PARAMS_MAX]);
    tmpAddr = (char *)(malloc(mallocSize));
    if (tmpAddr == NULL) {
        D_OOM();
        return;
    }
    memset(tmpAddr, 0, mallocSize);
    *targetAddr =  (struct ST_PQ_CFD_GOP_PARAMS *)(tmpAddr);
    tmpOffset += gopCount * sizeof(struct ST_PQ_CFD_GOP_PARAMS);

    for (int i = 0; i < gopCount; i++) {
        (*targetAddr)[i].pst_ui_params =  (ST_PQ_CFD_GOP_UI_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_UI_PARAM];
        (*targetAddr)[i].pst_tmo_params = (ST_PQ_CFD_TMO_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_TMO_PARAM];
        (*targetAddr)[i].pst_brightness_gain_params = (ST_PQ_CFD_GOP_BRIGHTNESS_GAIN *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_BRIGHTNESS_GAIN];
        (*targetAddr)[i].pst_color_fmt_params = (ST_PQ_CFD_GOP_FMT_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_FMT_PARAMS];
        (*targetAddr)[i].pst_xvycc_info_params = (ST_PQ_CFD_SET_XVYCC_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_XVYCC_PARAMS];
        (*targetAddr)[i].pst_xvycc_info_params->pst_source_colormetry = (ST_PQ_CFD_COLORMETRY_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_XVYCC_COLORMETRY_SOURCE];
        (*targetAddr)[i].pst_xvycc_info_params->pst_target_colormetry = (ST_PQ_CFD_COLORMETRY_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_XVYCC_COLORMETRY_TARGET];
        (*targetAddr)[i].pst_xvycc_info_params->pst_eotf_params = (ST_PQ_CFD_SET_EOTF_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_XVYCC_EOTF_PARAMS];
        (*targetAddr)[i].pst_xvycc_info_params->pst_oetf_params = (ST_PQ_CFD_SET_OETF_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_XVYCC_OETF_PARAMS];
        (*targetAddr)[i].pst_rgb_offset_params = (ST_PQ_CFD_GOP_OFFSET_PARAMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_OFFSET_PARAMS];
        (*targetAddr)[i].pst_rgb_clipping_params = (ST_PQ_CFD_GOP_CLIPPING_PARMS *)(tmpAddr + tmpOffset);
        tmpOffset += CfdParamsSize[CFD_CLIPPING_PARMS];
    }
}

void initPQ()
{

    u8 gopCount = dfb_config->mst_gop_counts;

    if (PqCfdSetting != NULL) {
        printf("\033[31m [DFB] initPQ, had inited, return\033[m\n");
        return;
    }

    DBG_LAYER_MSG("[DFB] initPQ, call pqMapUtilityLoaderInit(), pid=%d\n", getpid());
    if (pqMapUtilityLoaderInit() == false) {
        printf("\033[31m [DFB] initPQ, pq map utility loader init failed!\033[m\n");
        return;
    }
    DBG_LAYER_MSG("[DFB] initPQ, call pqMapUtilityLoaderGetPresetDB()\n");
    if (pqMapUtilityLoaderGetPresetDB(&PresetdbInfos) == false) {
        printf("\033[31m [DFB] initPQ, get preset db info failed!\033[m\n");
        return;
    }

    PqCfdSetting = (struct drm_mtk_tv_graphic_pq_setting *)(malloc(gopCount * sizeof(struct drm_mtk_tv_graphic_pq_setting)));
    if (PqCfdSetting == NULL) {
        D_OOM();
        return;
    }
    CfdGopsettingTable = (ST_PQ_CFD_GOP_SETTNG_TABLE *)(malloc(gopCount * sizeof(ST_PQ_CFD_GOP_SETTNG_TABLE)));
    if (CfdGopsettingTable == NULL) {
        D_OOM();
        return;
    }
    PqmapTrigParam = (stPqmapTrigParams *)(malloc(gopCount * sizeof(stPqmapTrigParams)));
    if (PqmapTrigParam == NULL) {
        D_OOM();
        return;
    }
    PqmapLastTrigParam = (stPqmapTrigParams *)(malloc(gopCount * sizeof(stPqmapTrigParams)));
    if (PqmapLastTrigParam == NULL) {
        D_OOM();
        return;
    }

    memset(PqCfdSetting, 0, gopCount * sizeof(struct drm_mtk_tv_graphic_pq_setting));
    memset(CfdGopsettingTable, 0, gopCount * sizeof(ST_PQ_CFD_GOP_SETTNG_TABLE));
    memset(PqmapTrigParam, 0, gopCount * sizeof(stPqmapTrigParams));
    memset(PqmapLastTrigParam, 0, gopCount * sizeof(stPqmapTrigParams));

    CfdParamsSize[CFD_UI_PARAM] = sizeof(ST_PQ_CFD_GOP_UI_PARAMS);
    CfdParamsSize[CFD_TMO_PARAM] = sizeof(ST_PQ_CFD_TMO_PARAMS);;
    CfdParamsSize[CFD_BRIGHTNESS_GAIN] = sizeof(ST_PQ_CFD_GOP_BRIGHTNESS_GAIN);
    CfdParamsSize[CFD_FMT_PARAMS] = sizeof(ST_PQ_CFD_GOP_FMT_PARAMS);
    CfdParamsSize[CFD_XVYCC_PARAMS] = sizeof(ST_PQ_CFD_SET_XVYCC_PARAMS);
    CfdParamsSize[CFD_XVYCC_COLORMETRY_SOURCE] = sizeof(ST_PQ_CFD_COLORMETRY_PARAMS);
    CfdParamsSize[CFD_XVYCC_COLORMETRY_TARGET] = sizeof(ST_PQ_CFD_COLORMETRY_PARAMS);
    CfdParamsSize[CFD_XVYCC_EOTF_PARAMS] = sizeof(ST_PQ_CFD_SET_EOTF_PARAMS);
    CfdParamsSize[CFD_XVYCC_OETF_PARAMS] = sizeof(ST_PQ_CFD_SET_OETF_PARAMS);
    CfdParamsSize[CFD_OFFSET_PARAMS] = sizeof(ST_PQ_CFD_GOP_OFFSET_PARAMS);
    CfdParamsSize[CFD_CLIPPING_PARMS] = sizeof(ST_PQ_CFD_GOP_CLIPPING_PARMS);
    for (int i = 0; i < (CFD_PARAMS_MAX); i++) {
        CfdParamsSize[CFD_PARAMS_MAX] += CfdParamsSize[i];
    }

    initCfdInputParam(&CfdGopProcessIn);
    initCfdInputParam(&LastCfdGopProcessIn);

    for (int i = 0; i < gopCount; i++) {
        PqCfdSetting[i].u64buf_pq_ml_addr = (uintptr_t)(malloc(GOP_SETTING_PQ_ML_MAX_SIZE));
        if (PqCfdSetting[i].u64buf_pq_ml_addr == NULL) {
            D_OOM();
            return;
        }
        memset((uintptr_t)(PqCfdSetting[i].u64buf_pq_ml_addr), 0, GOP_SETTING_PQ_ML_MAX_SIZE);

        PqCfdSetting[i].u64buf_cfd_ml_addr = (uintptr_t)(malloc(GOP_SETTING_CFD_ML_MAX_SIZE));
        if (PqCfdSetting[i].u64buf_cfd_ml_addr == NULL) {
            D_OOM();
            return;
        }
        memset((uintptr_t)(PqCfdSetting[i].u64buf_cfd_ml_addr), 0, GOP_SETTING_CFD_ML_MAX_SIZE);

        PqCfdSetting[i].u64buf_cfd_adl_addr = (uintptr_t)(malloc(GOP_SETTING_CFD_ADL_MAX_SIZE));
        if (PqCfdSetting[i].u64buf_cfd_adl_addr == NULL) {
            D_OOM();
            return;
        }
        memset((uintptr_t)(PqCfdSetting[i].u64buf_cfd_adl_addr), 0, GOP_SETTING_CFD_ADL_MAX_SIZE);

        CfdGopsettingTable[i].pu64Ml_addr = (uintptr_t)(PqCfdSetting[i].u64buf_cfd_ml_addr);
        CfdGopsettingTable[i].pu64Adl_addr = (uintptr_t)(PqCfdSetting[i].u64buf_cfd_adl_addr);
        CfdGopsettingTable[i].u16Length = sizeof(ST_PQ_CFD_GOP_SETTNG_TABLE);
    }
    DBG_LAYER_MSG("[DFB] initPQ, done!\n");
}

void deInitPQ() {
    for (int i = 0; i < dfb_config->mst_gop_counts; i++) {
        free((uintptr_t)(PqCfdSetting[i].u64buf_pq_ml_addr));
        free((uintptr_t)(PqCfdSetting[i].u64buf_cfd_ml_addr));
        free((uintptr_t)(PqCfdSetting[i].u64buf_cfd_adl_addr));
    }
    free((void *)(CfdGopProcessIn));
    free((void *)(LastCfdGopProcessIn));
    free((void *)(CfdGopsettingTable));
    free((void *)(PqCfdSetting));
    free((void *)(PqmapTrigParam));
    free((void *)(PqmapLastTrigParam));

    pqMapUtilityLoaderDeInit();
}


bool OSDPropertyChanged(uint32_t idx, int srcW, int srcH, int destW, int destH, DFBSurfacePixelFormat format )
{
    bool ret = true;

    u8 hdrMode = QMAP_HDR_TYPE_SDR;
    presetdb_hdr_mode *presetDbHdrModeInfo = &(PresetdbInfos.sdr);
    if(format == DSPF_ARGB2101010 || format == DSPF_ABGR2101010) {
            hdrMode = QMAP_HDR_TYPE_HDR10;
            presetDbHdrModeInfo = &(PresetdbInfos.hdr10);
    }
    /*if(layer->isHdrLayer()) {
        if(layer->getLayerDataspace() == HAL_DATASPACE_BT2020_PQ) {
            hdrMode = QMAP_HDR_TYPE_HDR10;
            presetDbHdrModeInfo = &(PresetdbInfos.hdr10);
        }
        else if (layer->getLayerDataspace() == HAL_DATASPACE_BT2020) {
            hdrMode = QMAP_HDR_TYPE_HLG;
            presetDbHdrModeInfo = &(PresetdbInfos.hlg);
        }
        else
            printf("unsupport HDR mode!");
    }*/

    if (srcW <= 0 || srcH <= 0 || destW <= 0 || destH <=0)
        return false;

    uint64_t dstW = destW;
    uint64_t dstH= destH;
    uint64_t displayScaleRatio_w = (dstW << WCG_HDR_SCALING_RATIO_SHIFT) / srcW;
    uint64_t displayScaleRatio_h = (dstH << WCG_HDR_SCALING_RATIO_SHIFT) / srcH;

    DBG_LAYER_MSG("[DFB] OSDPropertyChanged,  start!dstW=%lld, dstH=%lld, displayScaleRatio_w=%lld, displayScaleRatio_h=%lld, \n",
                  dstW, dstH, displayScaleRatio_w, displayScaleRatio_h);
    PqmapTrigParam[idx].width = srcW;
    PqmapTrigParam[idx].height = srcH;
    PqmapTrigParam[idx].vScalingratio = displayScaleRatio_h;
    PqmapTrigParam[idx].hScalingratio = displayScaleRatio_w;
    PqmapTrigParam[idx].allUsedGopMaxHsize = srcW;
    PqmapTrigParam[idx].eotfEnable = presetDbHdrModeInfo->eotf_enable;
    PqmapTrigParam[idx].eotfExtMode = presetDbHdrModeInfo->eotf_ext_mode;
    PqmapTrigParam[idx].gamutEnable = presetDbHdrModeInfo->gamut_enable;
    PqmapTrigParam[idx].gamutManualMode = presetDbHdrModeInfo->gamut_manual_mode;
    PqmapTrigParam[idx].gopLayer = dfb_config->mst_gop_available[idx];
    PqmapTrigParam[idx].hdrMode = hdrMode;
    PqmapTrigParam[idx].isYuv = false;
    PqmapTrigParam[idx].oetfEnable = presetDbHdrModeInfo->oetf_enable;
    PqmapTrigParam[idx].oetfExtMode = presetDbHdrModeInfo->oetf_ext_mode;
    PqmapTrigParam[idx].pureRgbMode = true;
    PqmapTrigParam[idx].rgbClippingEnable = PresetdbInfos.rgb_clipping.enable;
    PqmapTrigParam[idx].rgbClippingManualMode = PresetdbInfos.rgb_clipping.manual_mode;
    PqmapTrigParam[idx].rgbOffsetEnable = PresetdbInfos.rgb_offset.enable;
    PqmapTrigParam[idx].rgbOffsetManualMode = PresetdbInfos.rgb_offset.manual_mode;
    PqmapTrigParam[idx].subSample = 0;
    PqmapTrigParam[idx].targetHsize = destH;
    PqmapTrigParam[idx].xvyccManualMode = presetDbHdrModeInfo->xvycc_manual_mode;

    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].width, PqmapTrigParam[idx].width);
    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].height, PqmapTrigParam[idx].height);
    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].vScalingratio, PqmapTrigParam[idx].vScalingratio);
    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].hScalingratio, PqmapTrigParam[idx].hScalingratio);
    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].hdrMode, PqmapTrigParam[idx].hdrMode);
    ret = ret & ISPARAMSSAME(PqmapLastTrigParam[idx].allUsedGopMaxHsize, PqmapTrigParam[idx].allUsedGopMaxHsize);

    DBG_LAYER_MSG("[DFB] OSDPropertyChanged,  end! ret=%d\n", ret);
    return !ret;
}

bool cfdParamsChanged( uint32_t idx, DFBSurfacePixelFormat format) {
    bool ret = true;
    u8 hdrMode = E_CFD_SDR_MODE;
    presetdb_hdr_mode *presetDbHdrModeInfo = &(PresetdbInfos.sdr);

    if(format == DSPF_ARGB2101010 || format == DSPF_ABGR2101010) {
        hdrMode = E_CFD_HDR10_MODE;
        presetDbHdrModeInfo = &(PresetdbInfos.hdr10);
    }
    /*auto layer = layers.at(0);
    if(layer->isHdrLayer()) {
        if(layer->getLayerDataspace() == HAL_DATASPACE_BT2020_PQ) {
            hdrMode = E_CFD_HDR10_MODE;
            presetDbHdrModeInfo = &(PresetdbInfos.hdr10);
        }
        else if (layer->getLayerDataspace() == HAL_DATASPACE_BT2020) {
            hdrMode = E_CFD_HLG_MODE;
            presetDbHdrModeInfo = &(PresetdbInfos.hlg);
        }
        else
            printf("unsupport HDR mode!");
    }*/

    DBG_LAYER_MSG("[DFB] cfdParamsChanged,  start!gop id=%d\n", idx);
    CfdGopProcessIn[idx].u8Path = dfb_config->mst_gop_available[idx];
    CfdGopProcessIn[idx].bIsGPUFmt = FALSE; //gop fmt
    CfdGopProcessIn[idx].u16Length = sizeof(struct ST_PQ_CFD_GOP_PARAMS);
    CfdGopProcessIn[idx].pst_ui_params->u16Length = sizeof(ST_PQ_CFD_GOP_UI_PARAMS);
#if WCG_HDR_CFD_PARAMS_HARDCODE
    CfdGopProcessIn[idx].pst_ui_params->u16Hue = CFD_HAEDCODE_UI_PARAM_HUE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Hue, CfdGopProcessIn[idx].pst_ui_params->u16Hue);

    CfdGopProcessIn[idx].pst_ui_params->u16Saturation = CFD_HAEDCODE_UI_PARAM_SATURATION;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Saturation, CfdGopProcessIn[idx].pst_ui_params->u16Saturation);

    CfdGopProcessIn[idx].pst_ui_params->u16Contrast = CFD_HAEDCODE_UI_PARAM_CONTRAST;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Contrast, CfdGopProcessIn[idx].pst_ui_params->u16Contrast);

    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R]);
    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G]);
    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R]);
    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G]);
    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable = TRUE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable, CfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable);

    CfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable = TRUE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable, CfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable);

    CfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode = 1;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode, CfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode);

    CfdGopProcessIn[idx].pst_tmo_params->u16Length = sizeof(ST_PQ_CFD_TMO_PARAMS);

    CfdGopProcessIn[idx].pst_tmo_params->u8HDRMode = hdrMode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u8HDRMode, CfdGopProcessIn[idx].pst_tmo_params->u8HDRMode);

    CfdGopProcessIn[idx].pst_tmo_params->u8TMOMode = E_CFD_TMO_MODE_ATUO;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u8TMOMode, CfdGopProcessIn[idx].pst_tmo_params->u8TMOMode);

    CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax  = panelLuminance;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u16PanelMax, CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode = hdrMode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode, CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat = E_CFD_RGB;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange = E_CFD_FULL;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace = E_CFD_COLORSPACE_BT601;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat = E_CFD_RGB;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange = E_CFD_FULL;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace = E_CFD_COLORSPACE_BT601;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace);

    CfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode = E_CFD_AUTO_TARGET;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode, CfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_R;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_G;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_B;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_R;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_G;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_B;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x = CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_X;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x, CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y = CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_Y;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y, CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y);

    CfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode = FALSE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode, CfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode);

    CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode = FALSE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode, CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode);

    CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable =FALSE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable, CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable);

    CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode = FALSE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode, CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode);

    CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable = FALSE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable, CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable);
#else
    CfdGopProcessIn[idx].pst_ui_params->u16Hue = PresetdbInfos.hue;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Hue, CfdGopProcessIn[idx].pst_ui_params->u16Hue);

    CfdGopProcessIn[idx].pst_ui_params->u16Saturation = PresetdbInfos.saturation;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Saturation, CfdGopProcessIn[idx].pst_ui_params->u16Saturation);

    CfdGopProcessIn[idx].pst_ui_params->u16Contrast = PresetdbInfos.contrast;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Contrast, CfdGopProcessIn[idx].pst_ui_params->u16Contrast);

    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_R]);
    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_G]);
    CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B] = CFD_HAEDCODE_UI_PARAM_BRIGHTNESS;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B], CfdGopProcessIn[idx].pst_ui_params->u16Brightness[COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_R]);
    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_G]);
    CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B] = CFD_HAEDCODE_UI_PARAM_RGBGAIN;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B], CfdGopProcessIn[idx].pst_ui_params->u16RGBGain[COLOR_INDEX_B]);

    /*TODO: get from UI*/
    CfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable = TRUE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable, CfdGopProcessIn[idx].pst_ui_params->bIsColorInversionEnable);

    /*TODO: get from UI*/
    CfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable = TRUE;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable, CfdGopProcessIn[idx].pst_ui_params->bIsColorCorrectionEnable);

    /*TODO: get from UI*/
    CfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode = 1;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode, CfdGopProcessIn[idx].pst_ui_params->u8CorrectionMode);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode = hdrMode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode, CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode);

    CfdGopProcessIn[idx].pst_tmo_params->u16Length = sizeof(ST_PQ_CFD_TMO_PARAMS);

    CfdGopProcessIn[idx].pst_tmo_params->u8HDRMode = hdrMode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u8HDRMode, CfdGopProcessIn[idx].pst_tmo_params->u8HDRMode);

    CfdGopProcessIn[idx].pst_tmo_params->u8TMOMode = E_CFD_TMO_MODE_ATUO;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u8TMOMode, CfdGopProcessIn[idx].pst_tmo_params->u8TMOMode);

    CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax  = (panelLuminance);
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u16PanelMax, CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);

    CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_sdr  = PresetdbInfos.sdr.user_gain;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u16PanelMax, CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);

    CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_pq  = PresetdbInfos.hdr10.user_gain;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u16PanelMax, CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);

    CfdGopProcessIn[idx].pst_brightness_gain_params->u16User_gain_hlg  = PresetdbInfos.hlg.user_gain;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_tmo_params->u16PanelMax, CfdGopProcessIn[idx].pst_tmo_params->u16PanelMax);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode = hdrMode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode, CfdGopProcessIn[idx].pst_color_fmt_params->u8HDRMode);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat = E_CFD_RGB; //hwc only receive RGB format
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputDataFormat);


    if (PresetdbInfos.input.range == RANGE_AUTO) {
        /* TODO: get input range and transfer tc from dataspace*/
    } else {
        if (PresetdbInfos.input.range == RANGE_LIMIT) {
            CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange = E_CFD_LIMIT;
        } else if (PresetdbInfos.input.range == RANGE_FULL) {
            CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange = E_CFD_FULL;
        }
    }
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputRange);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace = E_CFD_COLORSPACE_BT601;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace, CfdGopProcessIn[idx].pst_color_fmt_params->u8InputColorSpace);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat = E_CFD_RGB;//PresetdbInfos.output.format;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputDataFormat);

    if (PresetdbInfos.output.range == RANGE_LIMIT) {
        CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange = E_CFD_LIMIT;
    } else if (PresetdbInfos.output.range == RANGE_FULL) {
        CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange = E_CFD_FULL;
    }
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputRange);

    CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace = E_CFD_COLORSPACE_BT601;//PresetdbInfos.output.colorspace;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace, CfdGopProcessIn[idx].pst_color_fmt_params->u8OutputColorSpace);

    CfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode = presetDbHdrModeInfo->gamut_manual_mode ? E_CFD_GAMUT_MANUAL_MODE : E_CFD_AUTO_TARGET;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode, CfdGopProcessIn[idx].pst_xvycc_info_params->u83x3Mode);

    CfdGopProcessIn[idx].pst_xvycc_info_params->b3x3Enable = presetDbHdrModeInfo->gamut_enable;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->b3x3Enable, CfdGopProcessIn[idx].pst_xvycc_info_params->b3x3Enable);

#if WCG_HDR_PANEL_COLOR_METRY_HARDCODE
    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_R;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_R]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_G;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_G]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B] = CFD_HAEDCODE_XVYCC_PARAM_X_COORDINATES_GAMUT_B;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_x[WCG_HDR_COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_R;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_R]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_G;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_G]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B] = CFD_HAEDCODE_XVYCC_PARAM_Y_COORDINATES_GAMUT_B;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B], CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16DisplayPrimaries_y[WCG_HDR_COLOR_INDEX_B]);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x = CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_X;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x, CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_x);

    CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y = CFD_HAEDCODE_XVYCC_PARAM_WHITE_POINT_Y;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y, CfdGopProcessIn[idx].pst_xvycc_info_params->pst_target_colormetry->u16White_point_y);
#else
    /*TODO: get target colormetry from panel*/
#endif

    CfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode = presetDbHdrModeInfo->xvycc_manual_mode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode, CfdGopProcessIn[idx].pst_xvycc_info_params->bXvycc_curve_Manual_Mode);

    CfdGopProcessIn[idx].pst_xvycc_info_params->bEotf_ExtendMode = presetDbHdrModeInfo->eotf_ext_mode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->bEotf_ExtendMode, CfdGopProcessIn[idx].pst_xvycc_info_params->bEotf_ExtendMode);

    CfdGopProcessIn[idx].pst_xvycc_info_params->bOetf_ExtendMode = presetDbHdrModeInfo->oetf_ext_mode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_xvycc_info_params->bOetf_ExtendMode, CfdGopProcessIn[idx].pst_xvycc_info_params->bOetf_ExtendMode);

    CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode = PresetdbInfos.rgb_offset.manual_mode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode, CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetManualMode);

    CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable = PresetdbInfos.rgb_offset.enable;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable, CfdGopProcessIn[idx].pst_rgb_offset_params->bRGBOffsetEnable);

    CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode = PresetdbInfos.rgb_clipping.manual_mode;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode, CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingManualMode);

    CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable = PresetdbInfos.rgb_clipping.enable;
    ret = ret & ISPARAMSSAME(LastCfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable, CfdGopProcessIn[idx].pst_rgb_clipping_params->bRGBClippingEnable);


#endif

    DBG_LAYER_MSG("[DFB] cfdParamsChanged,  end! ret=%d\n", ret);
    return !ret;
}

void getPqCfdData( uint32_t idx, int srcW, int srcH, int destW, int destH, DFBSurfacePixelFormat format)
{
    struct ST_PQ_CFD_GOP_INFO gop_output;
    DBG_LAYER_MSG("[DFB] getPqCfdData, start! idx=%d, srcW=%d, srcH=%d, destW=%d, destH=%d, pid=%d\n",
		idx, srcW, srcH, destW, destH, getpid());

    if (OSDPropertyChanged( idx, srcW, srcH, destW, destH, format)) {
        stPqCfdSettings pqCfdSetting;
        memset(&pqCfdSetting, 0, sizeof(stPqCfdSettings));

        pqCfdSetting.scalingCbwriteAddr = (uintptr_t)(PqCfdSetting[idx].u64buf_pq_ml_addr);
        pqCfdSetting.scalingCbTotalMlCmdCnt = &(PqCfdSetting[idx].u32buf_pq_ml_size);
        pqCfdSetting.eotfLutAddr = &(CfdGopProcessIn[idx].pst_xvycc_info_params->pst_eotf_params->pu32EotfLut_Address);
        pqCfdSetting.eotfLutLen = &(CfdGopProcessIn[idx].pst_xvycc_info_params->pst_eotf_params->u16LUT_Size);
        pqCfdSetting.oetfLutAddr = &(CfdGopProcessIn[idx].pst_xvycc_info_params->pst_oetf_params->pu32OetfLut_Address);
        pqCfdSetting.oetfLutLen = &(CfdGopProcessIn[idx].pst_xvycc_info_params->pst_oetf_params->u16LUT_Size);
        pqCfdSetting.colorMatrixAddr = &(CfdGopProcessIn[idx].pst_xvycc_info_params->pu163x3_addr);
        pqCfdSetting.RGBOffsetAddr = &(CfdGopProcessIn[idx].pst_rgb_offset_params->pu16RGBOffset_addr);
        pqCfdSetting.clippingMaxAddr = &(CfdGopProcessIn[idx].pst_rgb_clipping_params->pu16RGB_max_limit_addr);
        pqCfdSetting.clippingMinAddr = &(CfdGopProcessIn[idx].pst_rgb_clipping_params->pu16RGB_min_limit_addr);

        DBG_LAYER_MSG("[DFB] getPqCfdData, call pqMapUtilityLoaderTrigger start!&(PqCfdSetting[idx].u32buf_pq_ml_size)=%p, pqCfdSetting.scalingCbTotalMlCmdCnt=%p, %d\n",
            &(PqCfdSetting[idx].u32buf_pq_ml_size), pqCfdSetting.scalingCbTotalMlCmdCnt, (*pqCfdSetting.scalingCbTotalMlCmdCnt));

        PqmapTrigParam[idx].pqCfdSetting = &pqCfdSetting;
        pqMapUtilityLoaderTrigger(&PqmapTrigParam[idx]);

        PqCfdSetting[idx].u32buf_pq_ml_size = PqCfdSetting[idx].u32buf_pq_ml_size * WCG_HDR_CFD_ML_CMD_SIZE;

        DBG_LAYER_MSG("[DFB] PqCfdSetting[idx].u64buf_pq_ml_addr = 0x%llx, PqCfdSetting[idx].u32buf_pq_ml_size = 0x%x\n",
        PqCfdSetting[idx].u64buf_pq_ml_addr, PqCfdSetting[idx].u32buf_pq_ml_size);
    }
    else {
        PqCfdSetting[idx].u32buf_pq_ml_size = 0;
        PqCfdSetting[idx].u64buf_pq_ml_addr = NULL;
    }

    memset(&gop_output, 0, sizeof(struct ST_PQ_CFD_GOP_INFO));
    gop_output.pst_gop_setting_table = &CfdGopsettingTable[idx];
    gop_output.u16Length = sizeof(struct ST_PQ_CFD_GOP_INFO);
    gop_output.pst_gop_setting_table->u16Length = sizeof(ST_PQ_CFD_GOP_SETTNG_TABLE);

    if (cfdParamsChanged(idx, format)) {
        enum EN_PQAPI_RESULT_CODES ret;
        gop_output.pst_gop_setting_table->u32Ml_used_size = 0;
        gop_output.pst_gop_setting_table->u32Adl_used_sized = 0;
#if DEBUG_WCG_HDR
        dumpCfdParams(idx);
#endif

        DBG_LAYER_MSG("[DFB] getPqCfdData,  call MApi_PQ_CFD_GOP_Process start! pst_gop_setting_table : %p, pid=%d\n", gop_output.pst_gop_setting_table, getpid());

        ret = MApi_PQ_CFD_GOP_Process(NULL, &CfdGopProcessIn[idx], &gop_output);
        if (ret != E_PQAPI_RC_SUCCESS)
            printf("[%s %d] MApi_PQ_CFD_GOP_Process failed %d\n", __FUNCTION__, __LINE__, ret);

        PqCfdSetting[idx].u32buf_cfd_adl_size = CfdGopsettingTable[idx].u32Adl_used_sized;
        PqCfdSetting[idx].u32buf_cfd_ml_size = CfdGopsettingTable[idx].u32Ml_used_size * WCG_HDR_CFD_ML_CMD_SIZE;

#if DEBUG_WCG_HDR
        dumpPqCfdSettings(idx);
#endif

        DBG_LAYER_MSG("[DFB] getPqCfdData,  call MApi_PQ_CFD_GOP_Process end! cfd_adl_size=%d, u32Adl_used_sized=%d, cfd_ml_size=%d, u32Ml_used_size=%d\n",
                      PqCfdSetting[idx].u32buf_cfd_adl_size, CfdGopsettingTable[idx].u32Adl_used_sized,
                      PqCfdSetting[idx].u32buf_cfd_ml_size, CfdGopsettingTable[idx].u32Ml_used_size);
    }
    else {
        PqCfdSetting[idx].u32buf_cfd_adl_size = 0;
        PqCfdSetting[idx].u64buf_cfd_adl_addr = NULL;
        PqCfdSetting[idx].u32buf_cfd_ml_size = 0;
        PqCfdSetting[idx].u64buf_cfd_ml_addr = NULL;
    }

    DBG_LAYER_MSG("[DFB] getPqCfdData, u64buf_pq_ml_addr=0x%llx, u32buf_pq_ml_size=0x%x \n u64buf_cfd_ml_addr=0x%llx, u32buf_cfd_ml_size=0x%x, pid=%d\n",
                  PqCfdSetting[idx].u64buf_pq_ml_addr, PqCfdSetting[idx].u32buf_pq_ml_size,
                  PqCfdSetting[idx].u64buf_cfd_ml_addr, PqCfdSetting[idx].u32buf_cfd_ml_size, getpid());

    DBG_LAYER_MSG("[DFB] getPqCfdData, u64buf_cfd_adl_addr=0x%llx, u32buf_cfd_adl_size=0x%x, pid=%d\n",
                  PqCfdSetting[idx].u64buf_cfd_adl_addr, PqCfdSetting[idx].u32buf_cfd_adl_size, getpid());

    DBG_LAYER_MSG("[DFB] getPqCfdData, end!\n");
}

#endif

/******************************************************************************/


int mtk_set_connector_crtc( MTKDRMData *drmdata )
{
    drmModeRes *res = drmdata->resources;
    drmModeConnector *conn;
    drmModeObjectProperties *props;
    drmModeAtomicReq *req;
    uint32_t conn_id, crtc_id;
    uint32_t property_crtc_id, property_active, property_mode_id, blob_id;
    int ret;


    drmdata->crtc_id = crtc_id = res->crtcs[0];
    conn_id = res->connectors[0];


    conn = drmModeGetConnector(drmdata->drm_fd, conn_id);
    if (conn == NULL) {
        printf("[DFB] drmModeGetConnector failed!\n");
        return -1;
    }

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);

    /* get connector properties */
    props = drmModeObjectGetProperties(drmdata->drm_fd, conn_id, DRM_MODE_OBJECT_CONNECTOR);
    property_crtc_id = get_property_id(drmdata->drm_fd, props, "CRTC_ID");
    drmModeFreeObjectProperties(props);

    /* get crtc properties */
    props = drmModeObjectGetProperties(drmdata->drm_fd, crtc_id, DRM_MODE_OBJECT_CRTC);
    property_active = get_property_id(drmdata->drm_fd, props, "ACTIVE");
    property_mode_id = get_property_id(drmdata->drm_fd, props, "MODE_ID");

    drmdata->property_out_fence_id = get_property_id(drmdata->drm_fd, props, "OUT_FENCE_PTR");

    drmModeCreatePropertyBlob(drmdata->drm_fd, &conn->modes[0], sizeof(conn->modes[0]), &blob_id);
    drmModeFreeObjectProperties(props);

#ifndef USE_MI_RENDER
    req = drmModeAtomicAlloc();
    drmModeAtomicAddProperty(req, crtc_id, property_active, 1);
    drmModeAtomicAddProperty(req, crtc_id, property_mode_id, blob_id);
    drmModeAtomicAddProperty(req, conn_id, property_crtc_id, crtc_id);

    DRM_ATOMIC_COMMIT( DRM_MODE_ATOMIC_ALLOW_MODESET );
#endif

    //drmdata->mode = conn->modes;
    memcpy(&drmdata->mode, conn->modes, sizeof(*conn->modes));

    return 0;
}

#ifndef USE_MI_RENDER
static int mtk_atomic_close_plane( MTKDRMData *drmdata )
{
    drmModePlaneRes *plane_res = drmdata->plane_resources;
    drmModeAtomicReq *req;
    int ret, i;

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    req = drmModeAtomicAlloc();
		
    for (i=0; i < drmdata->plane_num; i++) {
        uint32_t plane_id = plane_res->planes[drmdata->plane_idx + i];

        drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_x_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_y_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_w_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_h_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_x_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_y_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, 0);
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, 0);

    }

    DRM_ATOMIC_COMMIT( 0 );

    return 0;
}
#endif

void mtk_drm_get_standard_propertyId( MTKDRMData *drmdata )
{
    drmModeObjectProperties *props;
    drmModePlaneRes *plane_res;
    uint32_t plane_id;

    /* get plane resources */
    drmSetClientCap( drmdata->drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1 );
    drmdata->plane_resources = drmModeGetPlaneResources( drmdata->drm_fd );
    plane_res = drmdata->plane_resources;
    plane_id = plane_res->planes[0];
    printf("[DFB] %s, plane idx = %d\n", __FUNCTION__, drmdata->plane_idx);

    props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);

    drmdata->property_crtc_id = get_property_id(drmdata->drm_fd, props, "CRTC_ID");
    drmdata->property_fb_id = get_property_id(drmdata->drm_fd, props, "FB_ID");
    drmdata->property_crtc_x_id = get_property_id(drmdata->drm_fd, props, "CRTC_X");
    drmdata->property_crtc_y_id = get_property_id(drmdata->drm_fd, props, "CRTC_Y");
    drmdata->property_crtc_w_id = get_property_id(drmdata->drm_fd, props, "CRTC_W");
    drmdata->property_crtc_h_id = get_property_id(drmdata->drm_fd, props, "CRTC_H");
    drmdata->property_src_x_id = get_property_id(drmdata->drm_fd, props, "SRC_X");
    drmdata->property_src_y_id = get_property_id(drmdata->drm_fd, props, "SRC_Y");
    drmdata->property_src_w_id = get_property_id(drmdata->drm_fd, props, "SRC_W");
    drmdata->property_src_h_id = get_property_id(drmdata->drm_fd, props, "SRC_H");

    drmModeFreeObjectProperties(props);

}


static int mtk_drm_get_graphic_plane_num_start_index( MTKDRMData *drmdata )
{
    drmModePlaneRes *plane_res;
    uint32_t plane_num = 0;
    int plane_index_start = -1;
    int i;
    uint32_t plane_id;
    drmModeObjectProperties *props;
    uint64_t plane_type_prop_value;
    uint64_t afbc_prop_value;

    plane_res = drmdata->plane_resources;

    drmdata->afbc_idx = 0;
    for(i = 0; i < plane_res->count_planes; i++)
    {
        plane_id = plane_res->planes[i];
        props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
        plane_type_prop_value = get_property_value_byName(drmdata->drm_fd, props, "Plane_type");
        afbc_prop_value = get_property_value_byName(drmdata->drm_fd, props, "AFBC-feature");
        drmModeFreeObjectProperties(props);

        if (plane_type_prop_value == MTK_DRM_PLANE_TYPE_GRAPHIC)
        {
            if (plane_index_start < 0)
                plane_index_start = i;

            if ( afbc_prop_value ) {
		uint32_t idx = i - plane_index_start;
                drmdata->afbc_idx |= (1 << idx);
		printf( "[DFB]%s, GOP %d support AFBC!\n", __FUNCTION__, idx);
            }
				
            D_INFO( "[%s, %d] plane %d : GRAPHIC TYPE\n", __FUNCTION__, __LINE__, i);
            ++plane_num;
        }
    }

    drmdata->plane_idx = plane_index_start;
    drmdata->plane_num = plane_num;
    D_INFO( "[%s, %d] graphics plane idx = %d, plane num = %d\n", __FUNCTION__, __LINE__, drmdata->plane_idx, drmdata->plane_num );

    return 0;
}



static u32 mtk_create_buffer_handle(
	MTKDRMData *drmdata,
	hal_phy phy_addr,
	u32 pitch,
	int width,
	int height,
	u32 format,
	bool bAFBC)
{
    
    struct drm_mtk_tv_gem_create creq;
    int ret;        
    u32 bo_handles[HANDLE_NUM] = {0}, pitches[HANDLE_NUM] = {0}, offsets[HANDLE_NUM] = {0};
    u32 fb_id;

    memset(&creq, 0, sizeof(creq));
    creq.u32version = 0x0;
    creq.u32length = sizeof(creq);
    creq.drm_dumb.size = pitch * height;
    creq.drm_dumb.flags = 0x0;
    creq.drm_dumb.handle = 0x0;
    creq.u64gem_dma_addr = phy_addr;

    ret = drmIoctl(drmdata->drm_fd, DRM_IOCTL_MTK_TV_GEM_CREATE, &creq);
    if (ret) {
        D_WARN( "cannot create dumb buffer (%d): %m\n", errno);
    }

    bo_handles[0] = creq.drm_dumb.handle;
    pitches[0] = pitch;

    if (bAFBC) {
#define ASIZE 4      
        uint64_t afbc_modifier[ASIZE];
        memset(afbc_modifier, 0x0, sizeof(afbc_modifier));
        afbc_modifier[0] = DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_32x8);	
        ret = drmModeAddFB2WithModifiers(drmdata->drm_fd, width, height, format,
                  bo_handles, pitches, offsets, afbc_modifier, &fb_id, DRM_MODE_FB_MODIFIERS);
        DBG_LAYER_MSG("[DFB] %s, create fb_id = %d for AFBC!\n", __FUNCTION__, fb_id);
    }
    else {
        // add fb to gen fb_id
        ret = drmModeAddFB2(drmdata->drm_fd, width, height, format, bo_handles, pitches, offsets, &fb_id, 0);
    }
    if (ret) {
        D_WARN("cannot bind the dumb-buffer to an FB object, ret: %d\n", ret);
        return 0;
    }

    DBG_LAYER_MSG("[DFB] %s, phy_addr=%llx, pitch=%d, w=%d, h=%d, fmt=%x, fb_id = %d\n", __FUNCTION__,
		phy_addr, pitch, width, height, format, fb_id);
    return fb_id;
}

#ifdef USE_MI_RENDER

static inline const char* getMiErrorName(MI_RESULT ret)
{
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


#define CHECK_MI_RET(RET, API, HANDLE)  \
    {   \
        MI_RESULT ret = RET; \
        if (ret != MI_OK) {   \
            printf("%s failed, handle: %x, Error[0x%x]: %s !!!, line=[%d].", API, HANDLE, ret, (getMiErrorName(ret)),  __LINE__);  \
        }   \
    }

#define MI_RENDER_COMMIT( ) \
{ \
    MI_HANDLE hController = MI_HANDLE_NULL;\
    MI_RENDER_OpenControllerParams_t stOpenControllerParams;\
    memset(&stOpenControllerParams, 0, sizeof(MI_RENDER_OpenControllerParams_t));\
    if (MI_RENDER_OpenController(&stOpenControllerParams, &hController) != MI_OK) {\
        printf("get MI Render Controller failed!!!!!!");\
    }\
\
    MI_RESULT ret = MI_RENDER_ApplyDeviceProperty(hController);\
    if (ret != MI_OK) {\
        printf("MI_RENDER_ApplyDeviceProperty failed error: %s!!!", getMiErrorName(ret));\
    }\
}

bool openMiRenderplane(int planeIdx, MI_HANDLE* phandle)
{
    if (phandle == NULL) {
        printf("invalid parameters phandle is NULL");
        return false;
    }

    MI_HANDLE renderhandle = MI_HANDLE_NULL;
    MI_RENDER_OpenParams_t stOpenParams;
    MI_U8 Name[] = "dummy";
    stOpenParams.eWinType = E_MI_RENDER_WIN_TYPE_GRAPHIC;
    stOpenParams.pszName = Name;
    stOpenParams.u32PqId = 0;
    stOpenParams.u8LayerId = (MI_U8)planeIdx;

    MI_RESULT ret = MI_RENDER_Open(&stOpenParams, &renderhandle);
    if (ret != MI_OK) {
        printf("MI_RENDER_Open failed error: %s!!!", getMiErrorName(ret));
        *phandle = MI_HANDLE_NULL;
        return false;
    }
	
    *phandle = renderhandle;
    return true;
}

static bool bFlipDone = false;

static MI_RESULT flipdone_callback(MI_HANDLE hnd, MI_U32 u32Event, void *pEventParams, void *pUserParams)
{

    if (u32Event == E_MI_RENDER_CALLBACK_EVENT_FLIP_FRMAE_DONE) {
        D_INFO("[DFB] %s, Non-blocking and flip event flag success\n", __FUNCTION__);
        bFlipDone = true;
    }

    return MI_OK;
}

#endif


#define USE_DRM_FENCE 0

#if !USE_DRM_FENCE

void mtk_test_event_handler(int fd, uint32_t frame, uint32_t sec, uint32_t usec, void *data)
{
    DBG_LAYER_MSG("[DFB] %s, Non-blocking and flip event flag success\n", __FUNCTION__);
}

static int mtk_atomic_set_plane_flip(
	MTKDRMData *drmdata,
	int fb_id,
	int gop_idx, int x, int y, int width, int height)
{
#ifdef USE_MI_RENDER

    MI_HANDLE planehandle = MI_HANDLE_NULL;
    openMiRenderplane(gop_idx, &planehandle);

    DBG_LAYER_MSG("[DFB] %s, layer id=%d, w=%d, h=%d, fb_id=%d, pid=%d\n", __FUNCTION__,
		gop_idx, width, height, fb_id, getpid());

    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_FBID, fb_id), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_X, x), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_Y, y), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_W, width << SHIFT16), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_H, height << SHIFT16), \
                "MI_RENDER_AddDeviceProperty", planehandle);

    // renderAtomicCommit
    MI_RENDER_COMMIT();

    int timeout = 0;
    struct timespec delay;
    delay.tv_nsec = NSEC;
    delay.tv_sec = 0;
    while(!bFlipDone) {
        nanosleep( &delay, NULL );// wait 1ms
        if (++timeout > TIMEOUT) {
            printf("[DFB] %s, drmHandleEvent timeout, break!!\n", __FUNCTION__);
            break;
        }
    }
    bFlipDone = false;

    D_INFO("[DFB] %s, done!\n", __FUNCTION__);
#else
    
    drmModePlaneRes *plane_res;
    drmModeAtomicReq *req;
    uint32_t plane_id;
    int ret;

    plane_res = drmdata->plane_resources;
    plane_id = plane_res->planes[drmdata->plane_idx + gop_idx];

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    req = drmModeAtomicAlloc();

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_fb_id, fb_id);

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_x_id, x);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_y_id, y);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, width << SHIFT16);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, height << SHIFT16);

    DBG_LAYER_MSG("[DFB] %s, layer id=%d, w=%d, h=%d, fb_id=%d, pid=%d\n", __FUNCTION__,
		gop_idx, width, height, fb_id, getpid());


    DRM_ATOMIC_COMMIT( DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT );

    drmEventContext event = {};
    event.version = DRM_EVENT_CONTEXT_VERSION;
    event.page_flip_handler = mtk_test_event_handler;
	
    if(drmHandleEvent(drmdata->drm_fd, &event))
    {
        D_WARN("drmHandleEvent fail!!\n");
        return errno;
    }
#endif
    return 0;
}


#else

#include <poll.h>

int sync_wait(int fd, int timeout)
{
    struct pollfd fds;
    int ret;

    if (fd < 0) {
        errno = EINVAL;
        return -1;
    }

    fds.fd = fd;
    fds.events = POLLIN;

    do {
        ret = poll(&fds, 1, timeout);
        if (ret > 0) {
            if (fds.revents & (POLLERR | POLLNVAL)) {
                errno = EINVAL;
                return -1;
            }
            return 0;
        } else if (ret == 0) {
            errno = ETIME;
            return -1;
        }
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    return ret;
}

static int mtk_atomic_set_plane_flip(
	MTKDRMData *drmdata,
	int fb_id,
	int layer_idx, int x, int y, int width, int height)
{	
    drmModePlaneRes *plane_res;
    drmModeAtomicReq *req;
    drmModeObjectProperties *props;
    uint32_t plane_id;
    //static uint32_t property_out_fence_id = 0;
    s32 fence_fd = 0;
    int ret;

    plane_res = drmdata->plane_resources;
    plane_id = plane_res->planes[drmdata->plane_idx + layer_idx];

    /*if (property_out_fence_id == 0)
    {
        props = drmModeObjectGetProperties(drmdata->drm_fd, drmdata->crtc_id, DRM_MODE_OBJECT_CRTC);
        property_out_fence_id = get_property_id(drmdata->drm_fd, props, "OUT_FENCE_PTR");	
printf("[DFB] %s, get property_out_fence_id = %d\n", __FUNCTION__, property_out_fence_id);
printf("[DFB] p_crtc_id=%d, p_fb_id=%d, src_x_id=%d, src_y_id=%d, src_w_id=%d, src_h_id=%d\n",
	drmdata->property_crtc_id, drmdata->property_fb_id, drmdata->property_src_x_id,
	drmdata->property_src_y_id, drmdata->property_src_w_id, drmdata->property_src_h_id);
    }*/

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    req = drmModeAtomicAlloc();

    drmModeAtomicAddProperty(req, drmdata->crtc_id, drmdata->property_out_fence_id, &fence_fd);
printf("[DFB] %s, %d, fence_fd = %p\n", __FUNCTION__, __LINE__, &fence_fd);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_fb_id, fb_id);

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_x_id, x);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_y_id, y);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, width << SHIFT16);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, height << SHIFT16);

    DBG_LAYER_MSG("[DFB] %s, layer id=%d, w=%d, h=%d, fb_id=%d, pid=%d\n", __FUNCTION__,
		layer_idx, width, height, fb_id, getpid());


    DRM_ATOMIC_COMMIT( DRM_MODE_ATOMIC_NONBLOCK );

    /*if atomic ioctl success, then fence fd is stored in variable "fence_fd" */
    /* call sync_wait to wait fence signal*/
printf("[DFB] %s, %d, fence_fd = %d\n", __FUNCTION__, __LINE__, fence_fd);
    ret = sync_wait(fence_fd, WAIT_TIME);
    if (ret <= 0)
	D_WARN("[DFB] %s, sync_wait failed, ret=%d\n", __FUNCTION__, ret);
//long long t4 = direct_clock_get_micros();

//printf("[DFB] %s, t1=%lld, t2=%lld, t3=%lld, t4=%lld\n", __FUNCTION__, t1, t2, t3, t4);
    return 0;
}


#endif


// disable bootlogo on drm

int mtk_disable_bootlogo(int fd)
{
    struct drm_mtk_tv_bootlogo_ctrl creq;
    int ret;

    memset(&creq, 0, sizeof(struct drm_mtk_tv_bootlogo_ctrl));
    creq.u8CmdType = MTK_CTRL_BOOTLOGO_CMD_GETINFO;

    ret = drmIoctl(fd, DRM_IOCTL_MTK_TV_CTRL_BOOTLOGO, &creq);
    if (ret) {
        D_WARN("can not get bootlogo GOP number\n");
        return errno;
    }

    D_INFO("[STI][GOP]Bootlogo GOP number is %d, and disable it\n",creq.u8GopNum);

    creq.u8CmdType = MTK_CTRL_BOOTLOGO_CMD_DISABLE;
    ret = drmIoctl(fd, DRM_IOCTL_MTK_TV_CTRL_BOOTLOGO, &creq);
    if (ret) {
        D_WARN("can not disable bootlogo GOP\n");
        return errno;
    }

    return 0;
}

/*****************************************************************************/

static DFBResult
InitDrm( MTKDRMData *drmdata)
{
    DFBResult ret;

#ifdef USE_MI_RENDER
    if(dfb_config->mst_debug_layer)
        MI_RENDER_SetDebugLevel(MI_RENDER_DEBUG_LEVEL);

    MI_RESULT errCode = MI_ERR_FAILED;
    errCode = MI_SYS_Init(NULL);

    if(errCode != MI_OK) {
        printf("\033[31m [DFB] Error 0x%x: failed to MI_SYS_Init()!\033[m\n", errCode);
    }

    MI_RENDER_InitParams_t stInitParams_t;
    memset(&stInitParams_t, 0, sizeof(MI_RENDER_InitParams_t));
    CHECK_MI_RET(MI_RENDER_Init(&stInitParams_t), "MI_RENDER_Init", MI_HANDLE_NULL);

#endif

    /* open drm module */
    drmdata->drm_fd = open( drmdata->device_name, O_RDWR | O_CLOEXEC );
    if (drmdata->drm_fd < 0) {
         printf( "DirectFB/DRM: Failed to open '%s'!errno:%d, %m\n", drmdata->device_name, errno );
         return DFB_INIT;
    }

    drmdata->resources = drmModeGetResources( drmdata->drm_fd );
    if (drmdata->resources == NULL)
    {
        printf( "drmModeGetResources failed!drm_fd = %d\n", drmdata->drm_fd);
        return DFB_INIT;
    }

    /* set crtc */
    ret = mtk_set_connector_crtc( drmdata );
    if (ret != 0)
    {
        printf("[STI][DRM] Modeset for crtc and connector failed!!\n");
        return DFB_INIT;
    }

    /*Get standard property ID first*/
    mtk_drm_get_standard_propertyId( drmdata );

    /*Get plane num & start index*/
    mtk_drm_get_graphic_plane_num_start_index( drmdata );

    return DFB_OK;
}

/* init_device  for master process, init drm */
void mstar_init_gop_device( CoreGraphicsDevice  *device, void  *driver_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData           *sdev = sdrv->dev;
    MTKDRMData *drmdata = &sdrv->DrmData;
    
    printf("\n[DFB] USE THE GFX : MTK_Drm\n");

#ifdef USE_GRAPHIC_PQ

    getPanelLuminance(drmdata);

    initPQ();

#endif

}

/* init_driver, init dfb screen and layer handle. */
void mstar_init_gop_driver( CoreGraphicsDevice  *device, void  *driver_data )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData           *sdev = sdrv->dev;
    int gopCount, gopStartIdx;
    hal_phy u32GOP_Regdma_addr;
    u32 u32GOP_Regdma_size;
    u32 u32GOP_Regdma_aligned;
    u32 u32GOP_Regdma_miu;
     int i = 0;
    int status = 0;
    static bool bGetChipCaps = false;

    int ret = 0;
    CoreDFB *core = sdrv->core;
    MTKDRMData *drmdata = &sdrv->DrmData;

    DBG_LAYER_MSG("[DFB] %s, driver_data(%p), device_data(%p), drmdata(%p)\n", __FUNCTION__, sdrv, sdev, drmdata);

    //if (dfb_core_is_master( core ))
    {
        // todo : set drm device name from config.
        direct_snputs( drmdata->device_name, "/dev/dri/card0", DEVICE_NAME_SIZE );

    
        InitDrm(drmdata);
    }

    DBG_LAYER_MSG("[DFB] %s, drmdata->drm_fd=%d, drmdata->mode.hdisplay=%d, drmdata->mode.vdisplay=%d\n", __FUNCTION__,
		drmdata->drm_fd, drmdata->mode.hdisplay, drmdata->mode.vdisplay);
 
    /* for screen and layer initialize start */
    /* Register primary screen. */
    sdrv->op_screen  = dfb_screens_register( device, driver_data, &mstarOPScreenFuncs );
    sdrv->ip0_screen = dfb_screens_register( device, driver_data, &mstarIP0ScreenFuncs);
    sdrv->ve_screen  = dfb_screens_register( device, driver_data, &mstarVEScreenFuncs );
    sdrv->oc_screen  = dfb_screens_register( device, driver_data, &mstarOCScreenFuncs );


    /* Register input system layers. */
    memset(sdrv->layers, NULL, sizeof(sdrv->layers));

    for( i = 0; i < dfb_config->mst_gop_counts; i++)
    {
        CoreScreen *screen_mode = sdrv->op_screen;

        if ( screen_mode != NULL )
            sdrv->layers[i] = dfb_layers_register( screen_mode,
                                                   driver_data,
                                                   &mstarLayerFuncs );
    }

    DBG_LAYER_MSG("[DFB] %s() end!\n", __FUNCTION__);
}

void ResetAllSurfInfo(MSTARDriverData *sdrv, MSTARDeviceData *sdev)
{

    MTKDRMData *drmdata = &sdrv->DrmData;
    int i;
    	
    for ( i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {  
        if (sdev->mstarLayerBuffer[i].u16SlotUsed == 1)
        {
            DBG_LAYER_MSG("[DFB] %s, mstarLayerBuffer[%d].u16SlitUsed = %d, pid=%d\n", __FUNCTION__, i, sdev->mstarLayerBuffer[i].u16SlotUsed, getpid());
            sdev->mstarLayerBuffer[i].u16SlotUsed = 0;
            sdev->mstarLayerBuffer[i].physAddr = 0;

            // remove fb from drm.
            DBG_LAYER_MSG("[DFB] %s, remove buffer fb_id = %d\n", __FUNCTION__, sdev->mstarLayerBuffer[i].fb_id);
            if (drmModeRmFB(drmdata->drm_fd, sdev->mstarLayerBuffer[i].fb_id)) {
                D_WARN("[DFB] Could not remove buffer fb_id = %d!! errno=%d, %m\n", sdev->mstarLayerBuffer[i].fb_id, errno);
            }
        }
    }
}

void mstar_close_driver( void  *driver_data  )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MTKDRMData *drmdata = &sdrv->DrmData;
    CoreDFB *core = sdrv->core;

    DBG_LAYER_MSG("[DFB] %s()\n", __FUNCTION__);
    
    if (dfb_core_is_master( core ))
    {
         // remove fb_id.
         ResetAllSurfInfo(sdrv, sdev);
         
        // TODO : close all plane ?
#ifndef USE_MI_RENDER
        mtk_atomic_close_plane(drmdata);
#endif

        if (drmdata->plane_resources)
            drmModeFreePlaneResources(drmdata->plane_resources);

        if (drmdata->resources)
            drmModeFreeResources( drmdata->resources );

#ifdef USE_GRAPHIC_PQ

        if (PqCfdSetting != NULL)
            deInitPQ();

        //enablePanelLuminanceControl(drmdata->drm_fd, false);

#endif
    }

    if (drmdata->drm_fd) {
        close (drmdata->drm_fd);
        printf("[DFB] %s, close drm_fd(%d)\n", __FUNCTION__, drmdata->drm_fd);
    }

    return DFB_OK;
}



static void _layer_CoreLayerRegionConfigFlags_err2string (CoreLayerRegionConfigFlags fail)
{
#define PRINT_CLRCF_ERROR(err, option)\
do{\
    if(err & option)\
        DBG_LAYER_MSG("[DFB] "#option"\n");\
}while(0)


    DBG_LAYER_MSG("[DFB] ==== fail = 0x%08x (CoreLayerRegionConfigFlags) ====\n", fail);

    PRINT_CLRCF_ERROR(fail, CLRCF_WIDTH);
    PRINT_CLRCF_ERROR(fail, CLRCF_HEIGHT);
    PRINT_CLRCF_ERROR(fail, CLRCF_FORMAT);
    PRINT_CLRCF_ERROR(fail, CLRCF_SURFACE_CAPS);
    PRINT_CLRCF_ERROR(fail, CLRCF_BUFFERMODE);
    PRINT_CLRCF_ERROR(fail, CLRCF_OPTIONS);
    PRINT_CLRCF_ERROR(fail, CLRCF_SOURCE_ID);
    PRINT_CLRCF_ERROR(fail, CLRCF_COLORSPACE);
    PRINT_CLRCF_ERROR(fail, CLRCF_SOURCE);
    PRINT_CLRCF_ERROR(fail, CLRCF_DEST);
    PRINT_CLRCF_ERROR(fail, CLRCF_CLIPS);
    PRINT_CLRCF_ERROR(fail, CLRCF_HSTRETCH);
    PRINT_CLRCF_ERROR(fail, CLRCF_OPACITY);
    PRINT_CLRCF_ERROR(fail, CLRCF_ALPHA_RAMP);
    PRINT_CLRCF_ERROR(fail, CLRCF_VSTRETCH);
    PRINT_CLRCF_ERROR(fail, CLRCF_TSTRETCH);
    PRINT_CLRCF_ERROR(fail, CLRCF_SRCKEY);
    PRINT_CLRCF_ERROR(fail, CLRCF_DSTKEY);
    PRINT_CLRCF_ERROR(fail, CLRCF_PARITY);
    PRINT_CLRCF_ERROR(fail, CLRCF_SURFACE);
    PRINT_CLRCF_ERROR(fail, CLRCF_PALETTE);
    PRINT_CLRCF_ERROR(fail, CLRCF_FREEZE);

    DBG_LAYER_MSG("[DFB] ==== fail = 0x%08x (CoreLayerRegionConfigFlags) ====\n", fail);

}



static u32
_DFBFmt2DrmFmt( DFBSurfacePixelFormat format )
{
    switch (format) {
        case DSPF_ARGB1555:
                return DRM_FORMAT_ARGB1555;

        case DSPF_ARGB:
                return DRM_FORMAT_ARGB8888;

        case DSPF_ABGR:
                return DRM_FORMAT_ABGR8888;

        //case DSPF_LUT8:
                //return E_MS_FMT_I8;

        case DSPF_ARGB4444:
                return DRM_FORMAT_ARGB4444;

        case DSPF_RGB16:
                return DRM_FORMAT_RGB565;

        case DSPF_AYUV:
                return DRM_FORMAT_AYUV;

        case DSPF_YVYU:
                return DRM_FORMAT_YUYV;

        case DSPF_UYVY:
                return DRM_FORMAT_UYVY;

        case DSPF_YUY2:
                return DRM_FORMAT_YUV422;

        case DSPF_ARGB2101010:
                return DRM_FORMAT_ARGB2101010;

        case DSPF_ABGR2101010:
                return DRM_FORMAT_ABGR2101010;
        //case DSPF_BLINK12355:
                //return E_MS_FMT_1ABFgBg12355;

        //case DSPF_BLINK2266:
                //return E_MS_FMT_FaBaFgBg2266;
        default:
                return DRM_FORMAT_INVALID;
    }
}



static int
_FindFBIdbyPhyAddr( MTKDRMData *drmdata,
            MSTARDeviceData *sdev,
            CoreSurface       *surf,
            CoreSurfaceBuffer *buffer,
            hal_phy            physAddr,
            u32                pitch,
            u32                size )
{

    int i;

    for(i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        if( sdev->mstarLayerBuffer[i].u16SlotUsed &&
            sdev->mstarLayerBuffer[i].physAddr == physAddr )
        {
            DBG_LAYER_MSG("[DFB] %s, idx = %d, slot uesd, physAddr = %llx, pitch=%x, pid=%d\n", __FUNCTION__, i, physAddr, pitch, getpid());

            if (sdev->mstarLayerBuffer[i].pCoDFBCoreSurface != surf ||
		sdev->mstarLayerBuffer[i].pCoDFBBuffer != buffer ||
		sdev->mstarLayerBuffer[i].pitch != pitch ||
		sdev->mstarLayerBuffer[i].size != size )
	    {
                sdev->mstarLayerBuffer[i].u16SlotUsed = 0;
                sdev->mstarLayerBuffer[i].physAddr = 0;

                // remove fb from drm.
                DBG_LAYER_MSG("[DFB] %s, drm fd=%d, remove buffer fb_id = %d, pid=%d\n", __FUNCTION__,
                	drmdata->drm_fd, sdev->mstarLayerBuffer[i].fb_id, getpid());

                /*if (drmModeRmFB(drmdata->drm_fd, sdev->mstarLayerBuffer[i].fb_id)) {
                    D_WARN("[DFB] Could not remove buffer fb_id = %d!! errno=%d, %m\n", sdev->mstarLayerBuffer[i].fb_id, errno);
                }*/

                sdev->mstarLayerBuffer[i].fb_id = 0;
            }
	    else
                return i;
        }
    }

    return -1;
}

static void
_SetSurfInfoSlot(  MTKDRMData *drmdata,
            MSTARDeviceData *sdev,
            u8                 layer_index,
            CoreSurface       *surf,
            CoreSurfaceBuffer *buffer,
            hal_phy            physAddr,
            unsigned long      pitch,
            u32                size,
            u32                fb_id )
{
    int i, slotID = -1;

    for( i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        DBG_LAYER_MSG("[DFB] %s, i=%d, slotUsed=%d\n", __FUNCTION__, i, sdev->mstarLayerBuffer[i].u16SlotUsed);
        if(sdev->mstarLayerBuffer[i].u16SlotUsed == 0) {
            slotID = i;
            break;
        }
    }

    DBG_LAYER_MSG("[DFB] %s, get free slot : %d\n", __FUNCTION__, slotID);

    if (slotID == -1) {
        // can't find free slot, remove first slot.
        // remove fb from drm.
        slotID = 0;
        DBG_LAYER_MSG("[DFB] %s, remove buffer fb_id = %d\n", __FUNCTION__, sdev->mstarLayerBuffer[slotID].fb_id);
        if (drmModeRmFB(drmdata->drm_fd, sdev->mstarLayerBuffer[slotID].fb_id)) {
            D_WARN("[DFB] Could not remove buffer fb_id = %d!! errno=%d, %m\n", sdev->mstarLayerBuffer[slotID].fb_id, errno);
        }
    }

    sdev->mstarLayerBuffer[slotID].pCoDFBCoreSurface    = surf;
    sdev->mstarLayerBuffer[slotID].pCoDFBBuffer         = buffer;
    sdev->mstarLayerBuffer[slotID].physAddr             = physAddr;
    sdev->mstarLayerBuffer[slotID].format               = surf->config.format;
    sdev->mstarLayerBuffer[slotID].u16SlotUsed          = 1;
    sdev->mstarLayerBuffer[slotID].pitch                = pitch;
    sdev->mstarLayerBuffer[slotID].size                  =  size;
    sdev->mstarLayerBuffer[slotID].layer_index       = layer_index;
    sdev->mstarLayerBuffer[slotID].fb_id                = fb_id;    

    DBG_LAYER_MSG("[DFB] %s, mstarLayerBuffer: %p, set surf slot %d, physAddr = %llx, surface %p, buffer %p, pitch=%x, size=%x, fb_id=%d, pid=%d\n",
		__FUNCTION__, sdev->mstarLayerBuffer, slotID, physAddr, surf, buffer, pitch, size, fb_id, getpid());
}

static u32 _FindDrmFbid(MSTARDriverData            *sdrv,
                       MSTARDeviceData            *sdev,
                       u8                layer_index,
                       u8                gop_index,
                       CoreSurface       *surf,
                       CoreSurfaceBuffer *buffer,
                       hal_phy               physAddr,
                       unsigned long      pitch,
                       u32                size)
{
    MTKDRMData *drmdata = &sdrv->DrmData;
    u32 fb_id = 0;
    bool bAFBC = false;

    DBG_LAYER_MSG("[DFB] %s, \n", __FUNCTION__);
    // check if phys addr is record or not.
    int slotId = _FindFBIdbyPhyAddr( drmdata, sdev,  surf, buffer, physAddr, pitch, size);
    if ( slotId == -1 ) {
        u32 drm_fmt = _DFBFmt2DrmFmt(surf->config.format);
	    int width = surf->config.size.w;
	    int height = surf->config.size.h;

        if ( dfb_config->mst_GPU_AFBC  && (( drmdata->afbc_idx >> gop_index ) & 0x1) ) {
            bAFBC = true;
        }

        fb_id = mtk_create_buffer_handle( drmdata, physAddr, pitch, width, height, drm_fmt, bAFBC);

        if (fb_id > 0) {
            _SetSurfInfoSlot( drmdata, sdev, layer_index, surf, buffer, physAddr, pitch, size, fb_id);
        }
	else
            printf("[DFB] %s, can not get drm fb_id\n", __FUNCTION__);
    }
    else
        fb_id = sdev->mstarLayerBuffer[slotId].fb_id;

    return fb_id;
}


static inline void
_RecordLayerCurPhyAddr( MSTARDriverData            *sdrv,
                             MSTARDeviceData            *sdev,
                             CoreSurfaceBufferLock      *lock,
                             u8                          layer_index)
{
    hal_phy  halPhys = 0;

    if(NULL==sdrv || NULL==sdev || NULL == lock)
    {
        D_WARN("\n recored the CurPhyAddr Failed\n");
        return ;
    }

    u16 u16TagID = 0;
  
    if(lock)
   {
       halPhys = _BusAddrToHalAddr(((u64)lock->phys_h << SHIFT32) | lock->phys);

       u16TagID = (lock->allocation != NULL) ? (u16) lock->allocation->gfxSerial.serial : 0;
    }

    sdrv->layerFlipInfo[layer_index].CurPhysAddr =  halPhys;
    sdrv->layerFlipInfo[layer_index].tagID = u16TagID;

    DBG_LAYER_MSG("[DFB] %s, layer = %d, tagID = %d\n", __FUNCTION__, layer_index, u16TagID);
    return;
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
mstarInitLayer( CoreLayer                       *layer,
                void                            *driver_data,
                void                            *layer_data,
                DFBDisplayLayerDescription      *description,
                DFBDisplayLayerConfig           *config,
                DFBColorAdjustment              *adjustment )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *data = layer_data;
    DFBResult ret = DFB_OK;
    u8 i;
    u8  curGopIndex;
    CoreDFB *core = layer->core;

    MTKDRMData *drmdata = &sdrv->DrmData;


    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    /* initialize layer data */

    for( i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {

        if(layer == sdrv->layers[i])
        {
            char  buf[BUF_SIZE];

            data->layer_index = i;

            //the gopindex should be required form the dfbconfig
            data->gop_index                 = dfb_config->mst_gop_available[i];
            data->gop_index_r               = dfb_config->mst_gop_available_r[i];
            data->gop_dst                   = dfb_config->mst_gop_dstPlane[i];
            data->layer_displayMode         = DLDM_NORMAL;
            data->GopDstMode                = dfb_config->mst_gop_dstPlane[i];

            sdev->layer_zorder[i]           = dfb_config->mst_layer_gwin_level;

            sdev->layer_active[i]           = false;
 
            //sdev->GOP_support_palette[i]    = false;

            /*init fusion & reactor */
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

            break;
        }

    }

    D_ASSERT( i < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    D_DEBUG_AT(MSTAR_Layer, "%s: data->layer: %d\n", __FUNCTION__, data->layer_index);

    /*for( i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        memset(&sdev->mstarLayerBuffer[i],0,sizeof(sdev->mstarLayerBuffer[i]));
	printf("[DFB] %s, i=%d, slotUsed=%d\n", __FUNCTION__, i, sdev->mstarLayerBuffer[i].u16SlotUsed);
    }*/

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
    snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Input %d", sdrv->num_inputs );


    /* fill out the default configuration */
    config->flags       = DLCONF_WIDTH          |
                          DLCONF_HEIGHT         |
                          DLCONF_PIXELFORMAT    |
                          DLCONF_BUFFERMODE     |
                          DLCONF_OPTIONS;

    config->pixelformat = DSPF_ARGB;
    config->buffermode  = DLBM_BACKVIDEO;
    config->options     = DLOP_ALPHACHANNEL;
    
    /* TODO: switch(data->gop_dst), ...
       get panel size for config->width, conifg->height */

    config->width       = drmdata->mode.hdisplay;
    config->height      = drmdata->mode.vdisplay;

    int default_w, default_h;
    default_w = dfb_config->mst_layer_default_width;
    default_h = dfb_config->mst_layer_default_height;

    if( config->width > default_w && default_w > 0 )
        config->width = default_w;

    if( config->height > default_h && default_h > 0)
        config->height = default_h;


    adjustment->flags = DCAF_BRIGHTNESS | DCAF_CONTRAST;
    adjustment->contrast=CONTRAST_INIT;   //50 means no change for contrast,just for initialization


    DFB_CHECK_POINT("mstarInitLayer done");

    return ret;

}



static DFBResult
_mstarTestRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags *failed )
{
    MSTARDriverData           *sdrv = driver_data;
    MSTARDeviceData           *sdev = sdrv->dev;
    MSTARLayerData            *slay = layer_data;
    CoreLayerRegionConfigFlags fail = 0;
    u8  curGopIndex;

    DBG_LAYER_MSG("[DFB] %s --> %d, GOPINDEX:%d GOPDST:%d\n", __FUNCTION__, __LINE__ , slay->gop_index, slay->gop_dst);

    MTKDRMData *drmdata = &sdrv->DrmData;

    // get width, height from driver, current is drm.
    int width       = drmdata->mode.hdisplay;
    int height      = drmdata->mode.vdisplay;

    slay->screen_size.width = sdev->layer_screen_size[slay->layer_index].width = width;
    slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = height;

    if( config->width > slay->screen_size.width ) {
        printf("\33[0;33;44m[DFB]\33[0m warning! config->width=%d > screen_size.width=%d %s(%d)\n", config->width, slay->screen_size.width, __FUNCTION__, __LINE__);
        config->width = slay->screen_size.width;
    }

    if( config->height > slay->screen_size.height ) {
        printf("\33[0;33;44m[DFB]\33[0m warning! config->height=%d > screen_size.height=%d %s(%d)\n", config->height, slay->screen_size.height, __FUNCTION__, __LINE__);
        config->height = slay->screen_size.height;
    }


    if (config->options & ~MSTAR_LAYER_SUPPORTED_OPTIONS)
    {
        fail |= CLRCF_OPTIONS;
        printf("\n[DFB] %s --> %d\n", __FUNCTION__, __LINE__);
    }

     // Currently, we only implement I8, ARGB8888 and ARGB1555
    switch (config->format)
    {
        case DSPF_LUT8:
        case DSPF_ARGB:
	    case DSPF_ABGR:
        case DSPF_ARGB1555:
        case DSPF_ARGB4444:
        case DSPF_RGB16:

        case DSPF_YVYU:
        case DSPF_AYUV:
        case DSPF_UYVY:
        case DSPF_YUY2:
        case DSPF_BLINK12355:
        case DSPF_BLINK2266:
        case DSPF_ARGB2101010:
        case DSPF_ABGR2101010:
                break;
        default:
                fail |= CLRCF_FORMAT;
    }

    DBG_LAYER_MSG("[DFB] slayer->gop_dst:0x%x\n",slay->gop_dst);
    DBG_LAYER_MSG("[DFB] %s (%d) config->width=%d config->height=%d\n", __FUNCTION__, __LINE__, config->width, config->height);
    DBG_LAYER_MSG("[DFB] %s (%d) screen_size.width=%d screen_size.height=%d\n", __FUNCTION__, __LINE__, slay->screen_size.width, slay->screen_size.height );

    DBG_LAYER_MSG("[DFB] %s (%d) config->dest.x: %d config->dest.y=%d\n", __FUNCTION__, __LINE__, config->dest.x, config->dest.y);
    DBG_LAYER_MSG("[DFB] %s (%d) config->dest.w: %d config->dest.h=%d\n", __FUNCTION__, __LINE__, config->dest.w, config->dest.h);



    if( config->options & (DLOP_DST_COLORKEY|DLOP_FLICKER_FILTERING) )
        fail |= CLRCF_OPTIONS;

    if( (config->options & (DLOP_ALPHACHANNEL | DLOP_OPACITY)) == (DLOP_ALPHACHANNEL | DLOP_OPACITY) )
    {
        //we can't support both of them.
        fail |= CLRCF_OPTIONS;
    }

    if( config->options & (DLOP_DEINTERLACING | DLOP_FIELD_PARITY) )
        fail |= CLRCF_OPTIONS;

    if( config->num_clips > 0 )
        fail |= CLRCF_CLIPS;

    if ( failed )
        *failed = fail;

    if ( fail )
    {
        DBG_LAYER_MSG("[DFB] %s (%d) fail: 0x%x\n", __FUNCTION__, __LINE__, fail);

        _layer_CoreLayerRegionConfigFlags_err2string(fail);

        return DFB_UNSUPPORTED;
    }

    DBG_LAYER_MSG( "[DFB] %s  (%d)\n", __FUNCTION__, __LINE__);
    return DFB_OK;
}


static DFBResult
mstarTestRegion( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 CoreLayerRegionConfig          *config,
                 CoreLayerRegionConfigFlags     *failed )
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

    if( !parameter )
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->config, config, sizeof(*config));

    if( failed )
        memcpy(&parameter->flags, failed, sizeof(*failed));

    if ( fusion_call_execute(&slay->call, FCEF_NONE, CLC_TEST_REGION, parameter, &ret) )
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(config, &parameter->config, sizeof(*config));

    if( failed )
        memcpy(failed, &parameter->flags, sizeof(*failed));

    SHFREE(shm_pool, parameter);

    return ret;
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

    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    D_MAGIC_SET( sreg, MSTARRegionData );

    sreg->config = *config;
    sreg->config_dirtyFlag = (CLRCF_ALL & ~CLRCF_COLORSPACE);
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

static inline CoreLayerRegionConfigFlags
mstarBuildUpdate( MSTARRegionData               *sreg,
                  CoreLayerRegionConfig         *config,
                  CoreLayerRegionConfigFlags     updated )
{

    CoreLayerRegionConfigFlags retUpdated = CLRCF_NONE;

    if( (updated & CLRCF_WIDTH)                 &&
        sreg->config.width != config->width )
    {
        retUpdated |= CLRCF_WIDTH;
        sreg->config.width = config->width;
    }

    if( (updated & CLRCF_HEIGHT)                &&
        sreg->config.height != config->height )
    {
        retUpdated |= CLRCF_HEIGHT;
        sreg->config.height = config->height;
    }

    if( (updated & CLRCF_FORMAT)                &&
        sreg->config.format != config->format )
    {
        retUpdated |= CLRCF_FORMAT;
        sreg->config.format = config->format;
    }

    if( (updated & CLRCF_SURFACE_CAPS)          &&
        sreg->config.surface_caps != config->surface_caps)
    {
        retUpdated |= CLRCF_SURFACE_CAPS;
        sreg->config.surface_caps = config->surface_caps;
    }

    if( (updated & CLRCF_BUFFERMODE)            &&
        sreg->config.buffermode != config->buffermode)
    {
        retUpdated |= CLRCF_BUFFERMODE;
        sreg->config.buffermode = config->buffermode;
    }

    if( (updated & CLRCF_OPTIONS)               &&
        sreg->config.options != config->options)
    {
        retUpdated |= CLRCF_OPTIONS;
        sreg->config.options = config->options;
    }

    if( (updated & CLRCF_SOURCE_ID)             &&
        sreg->config.source_id != config->source_id)
    {
        retUpdated |= CLRCF_SOURCE_ID;
        sreg->config.source_id = config->source_id;
    }

    if( (updated & CLRCF_SOURCE)                &&
        memcmp(&sreg->config.source, &config->source, sizeof(sreg->config.source)) )
    {
        retUpdated |= CLRCF_SOURCE;
        memcpy(&sreg->config.source, &config->source, sizeof(sreg->config.source));
    }

    if( (updated & CLRCF_DEST)                  &&
        memcmp(&sreg->config.dest, &config->dest, sizeof(sreg->config.dest)))
    {
        retUpdated |= CLRCF_DEST;
        memcpy(&sreg->config.dest, &config->dest, sizeof(sreg->config.dest));
    }

    //U4 special ,because the U4 layer0 layer1 used the same GOP
    if( dfb_config->mst_gop_available[0] == dfb_config->mst_gop_available[1] )
    {
        if( updated & CLRCF_DEST )
        {
            retUpdated |= CLRCF_DEST;
            memcpy(&sreg->config.dest, &config->dest, sizeof(sreg->config.dest));
        }
    }

    if( updated & CLRCF_CLIPS && config->num_clips > 0 )
    {
        retUpdated |= CLRCF_CLIPS;
        printf("warning: mstar t2 gfx driver doesn't support HW clip chains yet!\n");
    }

    if( (updated & CLRCF_OPACITY)       &&
        sreg->config.opacity != config->opacity )
    {
        retUpdated |= CLRCF_OPACITY;
        sreg->config.opacity = config->opacity;
    }

    if( (updated & CLRCF_ALPHA_RAMP)    &&
        memcmp(sreg->config.alpha_ramp, config->alpha_ramp, sizeof(sreg->config.alpha_ramp)) )
    {
        retUpdated |= CLRCF_ALPHA_RAMP;
        memcpy(sreg->config.alpha_ramp, config->alpha_ramp, sizeof(sreg->config.alpha_ramp));
    }

    if( (updated & CLRCF_SRCKEY)        &&
        memcmp(&sreg->config.src_key, &config->src_key, sizeof(sreg->config.src_key)) )
    {
        retUpdated |= CLRCF_SRCKEY;
        memcpy(&sreg->config.src_key, &config->src_key, sizeof(sreg->config.src_key));
    }

    if( (updated & CLRCF_DSTKEY)        &&
        memcmp(&sreg->config.dst_key, &config->dst_key, sizeof(sreg->config.dst_key)) )
    {
        retUpdated |= CLRCF_DSTKEY;
        memcpy(&sreg->config.dst_key, &config->dst_key, sizeof(sreg->config.dst_key));
        printf("warning: mstar t2 gfx driver doesn't support destionation color yet!\n");
    }

    if( (updated & CLRCF_PARITY)        &&
        sreg->config.positive != config->positive )
    {
        retUpdated |= CLRCF_PARITY;
        sreg->config.positive = config->positive;
    }

    if( (updated & CLRCF_COLORSPACE) )
    {
        retUpdated |= CLRCF_COLORSPACE;
    }

    if( (updated & CLRCF_HSTRETCH)      &&
        sreg->config.hstretchmode != config->hstretchmode )
    {
        retUpdated |= CLRCF_HSTRETCH;
        sreg->config.hstretchmode = config->hstretchmode;
    }

    if( (updated & CLRCF_VSTRETCH)      &&
        sreg->config.vstretchmode != config->vstretchmode )
    {
        retUpdated |= CLRCF_VSTRETCH;
        sreg->config.vstretchmode = config->vstretchmode;
    }

    if( (updated & CLRCF_TSTRETCH)      &&
        sreg->config.tstretchmode != config->tstretchmode )
    {
        retUpdated |= CLRCF_TSTRETCH;
        sreg->config.tstretchmode = config->tstretchmode;
    }

    if( updated & CLRCF_FREEZE )
    {
        retUpdated |= CLRCF_FREEZE;
    }

    return retUpdated;

}

static DFBResult
_mstarSetRegion( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 void                           *region_data,
                 CoreLayerRegionConfig          *config,
                 CoreLayerRegionConfigFlags      updated,
                 CoreSurface                    *surface,
                 CorePalette                    *palette,
                 CoreSurfaceBufferLock          *lock )
{

    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARRegionData *sreg = region_data;
    MSTARLayerData  *slay = layer_data;

    MTKDRMData *drmdata = &sdrv->DrmData;
    hal_phy u64Phys = 0;
    u8  constAlpha = 0;

    //bool bGWinEnable = true;

    bool bnewFB = false;
    bool bvideoModeChanged = false;
    bool bGOPModeChanged = false;
    int bpp = DFB_BYTES_PER_PIXEL(sreg->config.format);
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    // drm init start.
    int ret;

    drmModeAtomicReq *req;
    drmModeObjectProperties *props;
   
    
    // skirmish lock.
    fusion_skirmish_prevail( &sdev->beu_lock );


    sreg->config_dirtyFlag |= mstarBuildUpdate(sreg, config, updated);

    if( (dfb_config->mst_margine_top        ||     \
         dfb_config->mst_margine_bottom     ||
         dfb_config->mst_margine_left       ||    \
         dfb_config->mst_margine_wright) )
    {
        sreg->config.dest.x += dfb_config->mst_margine_left;
        sreg->config.dest.w -= dfb_config->mst_margine_wright;
        sreg->config.dest.y += dfb_config->mst_margine_top;
        sreg->config.dest.h -= dfb_config->mst_margine_bottom;
    }


    DBG_LAYER_MSG ("[DFB] %s (%d)\n", __FUNCTION__, __LINE__ );
    DBG_LAYER_MSG ("[DFB] surface->num_buffers = %d, updated = %08x\n", surface->num_buffers,updated );
    DBG_LAYER_MSG ("[DFB] config_flage = 0x%x, config_dirtyFlag = %x\n", updated, sreg->config_dirtyFlag);
    D_ASSERT( slay->layer_index >= 0 );
    D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    D_DEBUG_AT( MSTAR_Layer, "\n[DFB] dfb_gfxcard_memory_physical( NULL, lock->offset )--> 0x%x\n", dfb_gfxcard_memory_physical( NULL, lock->offset ));
    D_DEBUG_AT( MSTAR_Layer, "\n[DFB] dfb_gfxcard_memory_virtual( NULL, lock->offset )--> 0x%x\n", dfb_gfxcard_memory_virtual( NULL, lock->offset ));

    DBG_LAYER_MSG( "[DFB] the gop is slay->gop_index:%d\n",slay->gop_index);
    DBG_LAYER_MSG( "[DFB] config->width: %d  config->height: %d\n", config->width, config->height);
    DBG_LAYER_MSG( "[DFB] config->buffermode: 0x%x\n", config->buffermode);
    DBG_LAYER_MSG( "[DFB] config->source_id: %d\n", config->source_id);
    DBG_LAYER_MSG( "[DFB] config->source.x:%d config->source.y:%d config->source.w:%d config->source.h:%d\n", config->source.x, config->source.y, config->source.w, config->source.h);
    DBG_LAYER_MSG( "[DFB] config->dest.x:%d config->dest.y:%d config->dest.w:%d config->dest.h:%d\n", config->dest.x, config->dest.y, config->dest.w, config->dest.h);
    DBG_LAYER_MSG( "[DFB] config->opacity: %d\n", config->opacity);

    if( (sreg->config_dirtyFlag & (CLRCF_WIDTH | CLRCF_HEIGHT | CLRCF_FORMAT)) &&
        (slay->ShadowFlags & (SLF_SHADOW_LAYER_BOOLEAN | SLF_SHADOW_LAYER_INDEXALL)) )
    {
        D_WARN("\nthe layer is configure as shadow layer or this layer has other shadowlayer,refuse setRegion\n");
        fusion_skirmish_dismiss( &sdev->beu_lock );
        
        return DFB_FAILURE;
    }



    if( sreg->config_dirtyFlag & (CLRCF_SURFACE|CLRCF_WIDTH|CLRCF_HEIGHT|CLRCF_FORMAT|CLRCF_PALETTE)||
        bvideoModeChanged   ||
        bGOPModeChanged     ||
        config->buffermode == DLBM_FRONTONLY )
    {


        switch( config->format )
        {
            case DSPF_UNKNOWN:
            case DSPF_RGB24:
            case DSPF_RGB32:
            case DSPF_A8:
            case DSPF_RGB332:
            case DSPF_I420:
            case DSPF_YV12:
            case DSPF_ALUT44:
            case DSPF_AiRGB:
            case DSPF_A1:
            case DSPF_NV12:
            case DSPF_NV16:
            case DSPF_ARGB2554:
            case DSPF_NV21:
            case DSPF_A4:
            case DSPF_ARGB1666:
            case DSPF_ARGB6666:
            case DSPF_RGB18:
            case DSPF_LUT2:
            case DSPF_LUT1:
            case DSPF_LUT4:
            case DSPF_RGB444:
            case DSPF_RGB555:
            case DSPF_BGR555:
            case DSPF_RGBA4444:
            case DSPF_RGBA5551:
            case DSPF_ARGB2101010:
            case DSPF_ABGR2101010:
                    DBG_LAYER_MSG("[DFB] format:%s\n", dfb_pixelformat_name(config->format));
            break;

            case DSPF_LUT8:
                    DBG_LAYER_MSG( "[DFB] format: DSPF_LUT8\n");


            /*fall through*/

            case DSPF_YVYU:
            case DSPF_UYVY:
            case DSPF_YUY2:
            case DSPF_AYUV:
            case DSPF_ARGB1555:
            case DSPF_RGB16:
            case DSPF_ARGB:
	        case DSPF_ABGR:
            case DSPF_ARGB4444:
            case DSPF_BLINK12355:
            case DSPF_BLINK2266:
                DBG_LAYER_MSG("[DFB] format:%s\n", dfb_pixelformat_name(config->format));

                {
                    //gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width,sreg->config.height, sreg->config.dest.w,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
                    
                }

                //sreg->config_dirtyFlag &= ~CLRCF_DEST;

                //When change resolution or format or buffer, reset the dimension of source.
                sreg->config.source.x = 0;
                sreg->config.source.y = 0;
                sreg->config.source.w = sreg->config.width;
                sreg->config.source.h = sreg->config.height;
                DBG_LAYER_MSG("[DFB] sreg->config.source.w:%d, sreg->config.source.h:%d \n" , sreg->config.source.w, sreg->config.source.h);

             break;

        }   // end of switch
    }
    else
    {
        // TODO : replace by drm call.
        /*GOP_GwinInfo info;
        u32 u32tmpPhys = 0;
        memset(&info, 0, sizeof(GOP_GwinInfo));

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info));
        // For source rectangle implement.
        x1 = ALIGN(sreg->config.source.x, PIXEL_ALIGNMENT);
        y1 = ALIGN(sreg->config.source.y, PIXEL_ALIGNMENT);
        u32tmpPhys = u32Phys + (y1 * lock->pitch + x1 * bpp);
        info.u32DRAMRBlkStart = u32tmpPhys;
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info));*/
    }


    if( sreg->config_dirtyFlag & (CLRCF_SRCKEY|CLRCF_OPTIONS) )
    {
        u32 tcol;
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
                    /*if(dfb_config->mst_argb1555_display)
			   // TODO : replace by drm call.
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], TRUE, 0));
                    else
                    {
                            sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> 5);

                            sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> 5);

                            sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> 5);
                    }*/
                }
                    break;

                case DSPF_RGB16:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> SHIFT5);

                    sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> SHIFT6);

                    sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> SHIFT5);
                }
                    break;

                case DSPF_ARGB:
                    break;

                case DSPF_ARGB4444:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> SHIFT4);

                    sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> SHIFT4);

                    sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> SHIFT4);
                }
                    break;

                default:
                    break;
            }

            if(!(dfb_config->mst_argb1555_display && (config->format & DSPF_ARGB1555)))
            {
                 tcol = sreg->config.src_key.r << SHIFT16 |
                       sreg->config.src_key.g << SHIFT8  |
                       sreg->config.src_key.b;

                // TODO : replace by drm call.
                 //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetTransClr_8888(tcol, 0));
                 //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, TRUE));
            }

            /*if( slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
            {
                // TODO : replace by drm call.
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetTransClr_8888(tcol, 0));
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, TRUE));
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }*/
        }
        else
        {
            //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE));

            /*if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                // TODO : replace by drm call.
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE));
                //DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }*/
            
        }
    }
    

    

    //is this need set for individual gop ?
    // TODO : replace by drm ?
    /*if(GOP_API_SUCCESS == MApi_GOP_GetChipCaps(E_GOP_CAP_CONSALPHA_VALIDBITS, &enConstAlpha_bits, sizeof(EN_GOP_CONSALPHA_BITS)))
    {
        DFB_UTOPIA_TRACE(MApi_GOP_SetConfig(confugType, (MS_U32*)(&enConstAlpha_bits)));
    }*/

    constAlpha = sreg->config.opacity;

    // TODO : replace by drm ?
    /*if( enConstAlpha_bits == E_GOP_VALID_6BITS )
    {
        constAlpha = constAlpha >>2;
    }
    else if( enConstAlpha_bits == E_GOP_VALID_8BITS )
    {
        constAlpha = constAlpha ;
    }*/

    if( sreg->config_dirtyFlag & (CLRCF_OPACITY|CLRCF_OPTIONS) )
    {
        if(sreg->config.options & DLOP_OPACITY)
        {

            if( 0x0 == sreg->config.opacity )
            {
                //bGWinEnable = false;
            }

        }
        else if (sreg->config.options & DLOP_ALPHACHANNEL)
        {
            
        }
        else
        {
            u8 cstAlpha = MAX_ALPHA;

            /*if(enConstAlpha_bits == E_GOP_VALID_6BITS)
            {
                cstAlpha = cstAlpha >> 2;
            }
            else if(enConstAlpha_bits == E_GOP_VALID_8BITS)
            {
                cstAlpha = cstAlpha;
            }*/

            if(!(dfb_config->mst_argb1555_display && (config->format & DSPF_ARGB1555)))
            {
               
            }

        }
    }

    // PQ set stretch mode. 
    /*if( (sreg->config_dirtyFlag & CLRCF_HSTRETCH) || (sreg->config_dirtyFlag & CLRCF_VSTRETCH) )
    {
        
        if ( sreg->config_dirtyFlag & CLRCF_HSTRETCH ) {
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_hstretch_id, sreg->config.hstretchmode);
            DBG_LAYER_MSG ("[DFB] Set_HStretchMode: hstretchmode = 0x%x\n", sreg->config.hstretchmode);
        }

        if ( sreg->config_dirtyFlag & CLRCF_VSTRETCH ) {
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_vstretch_id, sreg->config.vstretchmode);
            DBG_LAYER_MSG ("[DFB] Set_VStretchMode: vstretchmode = 0x%x\n", sreg->config.vstretchmode);
        }
    }*/

    // TODO : refine the drm driver usage
#ifdef USE_MI_RENDER
    MI_HANDLE planehandle = MI_HANDLE_NULL;
    openMiRenderplane(slay->gop_index, &planehandle);

#else
    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);

    uint32_t plane_id = drmdata->plane_resources->planes[drmdata->plane_idx + slay->gop_index];

    req = drmModeAtomicAlloc();

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);
#endif

    if(sreg->config_dirtyFlag & (CLRCF_DEST|CLRCF_WIDTH|CLRCF_HEIGHT))
    {
    
        if(sreg->config.dest.w && sreg->config.dest.h)
        {
#ifdef USE_MI_RENDER
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_X, sreg->config.dest.x), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_Y, sreg->config.dest.y), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_W, sreg->config.dest.w), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_H, sreg->config.dest.h), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
#else
            // drm atomic add for output.
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_x_id, sreg->config.dest.x);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_y_id, sreg->config.dest.y);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_w_id, sreg->config.dest.w);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_h_id, sreg->config.dest.h);
#endif

            DBG_LAYER_MSG( "[DFB] %s, %d: plane %d set crtc dest, x:%d, y:%d, w:%d, h:%d\n",
		__FUNCTION__, __LINE__, (drmdata->plane_idx + slay->gop_index),
              sreg->config.dest.x,  sreg->config.dest.y, sreg->config.dest.w, sreg->config.dest.h);
        }
    }

    if(sreg->config_dirtyFlag & CLRCF_SOURCE)
    {
        if(sreg->config.source.w &&
           sreg->config.source.h &&
           sreg->config.source.w <= sreg->config.dest.w &&
           sreg->config.source.h <= sreg->config.dest.h)
        {
#ifdef USE_MI_RENDER
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_X, 0), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_Y, 0), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_W, sreg->config.width << SHIFT16), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_H, sreg->config.height << SHIFT16), \
                    "MI_RENDER_AddDeviceProperty", planehandle);

#else
            // drm atomic add for source.
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_x_id, 0);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_y_id, 0);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, sreg->config.width << SHIFT16);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, sreg->config.height << SHIFT16);
#endif
            DBG_LAYER_MSG( "[DFB] %s, %d: plane %d set src, w:%d, h:%d\n",
		__FUNCTION__, __LINE__, (drmdata->plane_idx + slay->gop_index),
               sreg->config.width, sreg->config.height);
        }
    }

#ifdef USE_MI_RENDER

    if(dfb_config->mst_GOP_HMirror >= 0)
    {
        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_H_MIRROR, \
			(dfb_config->mst_GOP_VMirror > 0)? 1 : 0), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    }

    if (dfb_config->mst_GOP_VMirror >= 0)
    {
        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_V_MIRROR, \
			(dfb_config->mst_GOP_VMirror > 0)? 1 : 0), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    }

    /* big num is high zpos, so, base on all plane num and remove mouse plane.
	  zpos = (plan_num - 2 ) - slay->gop_index*/
    uint32_t new_zpos = NEW_ZPOS;

    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_ZORDER, new_zpos), \
                "MI_RENDER_AddDeviceProperty", planehandle);

    DBG_LAYER_MSG("[DFB] %s, set layer %d to zpos %d\n", __FUNCTION__, slay->layer_index,new_zpos);

#else
    // set drm property.
    props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);

    // H mirror not support.
    if(dfb_config->mst_GOP_HMirror >= 0)
    {
        // set GOP H & V Mirror.
        drmdata->property_H_Mirror_id = get_property_id(drmdata->drm_fd, props, "H-Mirror");
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_H_Mirror_id, (dfb_config->mst_GOP_HMirror > 0)? 1 : 0);
        DBG_LAYER_MSG("[DFB] %s, set GOP HMirror(id=%d) : %d\n", __FUNCTION__, drmdata->property_H_Mirror_id, dfb_config->mst_GOP_HMirror);
    }

    if (dfb_config->mst_GOP_VMirror >= 0)
    {
        drmdata->property_V_Mirror_id = get_property_id(drmdata->drm_fd, props, "V-Mirror");
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_V_Mirror_id, (dfb_config->mst_GOP_VMirror > 0)? 1 : 0);
	    DBG_LAYER_MSG("[DFB] %s, set GOP VMirror(id=%d) : %d\n", __FUNCTION__, drmdata->property_V_Mirror_id, dfb_config->mst_GOP_VMirror);
    }

    /* drm set zpos */
    // todo: how to set zpos.
    //uint32_t new_zpos = dfb_config->mst_gop_counts - 1 - slay->layer_index;
    /* big num is high zpos, so, base on all plane num and remove mouse plane.
	  zpos = (plan_num - 2 ) - slay->gop_index*/
    uint32_t new_zpos = NEW_ZPOS;

    uint32_t property_zpos_id = get_property_id(drmdata->drm_fd, props, "zpos");

    DBG_LAYER_MSG("[DFB] %s, set layer %d to zpos %d\n", __FUNCTION__, slay->layer_index,new_zpos);

    drmModeAtomicAddProperty(req, plane_id, property_zpos_id, new_zpos);

    drmModeFreeObjectProperties(props);
#endif

#ifdef USE_GRAPHIC_PQ

    printf("[DFB] %s, %d, layer_index=%d, gop_index=%d, PqCfdSetting = %p, dirtyFlag = 0x%x, pid=%d\n",
        __FUNCTION__, __LINE__, slay->layer_index, slay->gop_index, PqCfdSetting, sreg->config_dirtyFlag, getpid());

    if(PqCfdSetting != NULL && (sreg->config_dirtyFlag & (CLRCF_DEST|CLRCF_WIDTH|CLRCF_HEIGHT | CLRCF_SOURCE | CLRCF_FORMAT))) {
        DBG_LAYER_MSG("[DFB] %s, config_dirtyFlag = 0x%x\n", __FUNCTION__, sreg->config_dirtyFlag);

        getPqCfdData( slay->layer_index, sreg->config.source.w, sreg->config.source.h, sreg->config.dest.w, sreg->config.dest.h, config->format);


        struct drm_mtk_tv_graphic_pq_setting stGfxPQInfo;
        memset(&stGfxPQInfo, 0, sizeof(struct drm_mtk_tv_graphic_pq_setting));
        stGfxPQInfo.u32version = GOP_PQ_BUF_ST_IDX_VERSION;
        stGfxPQInfo.u32GopIdx = slay->gop_index;
        stGfxPQInfo.u32buf_cfd_ml_size = PqCfdSetting[slay->layer_index].u32buf_cfd_ml_size;
        stGfxPQInfo.u64buf_cfd_ml_addr = PqCfdSetting[slay->layer_index].u64buf_cfd_ml_addr;
        stGfxPQInfo.u32buf_cfd_adl_size = PqCfdSetting[slay->layer_index].u32buf_cfd_adl_size;
        stGfxPQInfo.u64buf_cfd_adl_addr = PqCfdSetting[slay->layer_index].u64buf_cfd_adl_addr;
        stGfxPQInfo.u32buf_pq_ml_size = PqCfdSetting[slay->layer_index].u32buf_pq_ml_size;
        stGfxPQInfo.u64buf_pq_ml_addr = PqCfdSetting[slay->layer_index].u64buf_pq_ml_addr;

        int ret = drmIoctl(drmdata->drm_fd, DRM_IOCTL_MTK_TV_SET_GRAPHIC_PQ_BUF, &stGfxPQInfo);
        if (ret)
            printf("[DFB] %s, %d, drmIoctl DRM_IOCTL_MTK_TV_SET_GRAPHIC_PQ_BUF failed, errno=%d\n", __FUNCTION__, __LINE__, errno);

    }
#endif

    u64Phys = _BusAddrToHalAddr(((u64)lock->phys_h << SHIFT32) | lock->phys);

    u32 fb_id = _FindDrmFbid( sdrv, sdev, slay->layer_index, slay->gop_index , surface, lock->buffer, u64Phys, lock->pitch, lock->size );

#ifdef USE_MI_RENDER
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_FBID, fb_id), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    if(surface->config.caps & DSCAPS_SECURE_MODE)
    {
        D_INFO("[DFB] %s Secure mode!\n",__FUNCTION__);
        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_GFX_AID_TYPE, MTK_DRM_AID_TYPE_S), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
    }
    else
    {
        D_INFO("[DFB] %s Non-secure mode!\n",__FUNCTION__);
        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_GFX_AID_TYPE, MTK_DRM_AID_TYPE_NS), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
    }
#else
    // set drm property_fb_id,
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_fb_id, fb_id);
#endif
    DBG_LAYER_MSG("[DFB] %s, fb_id = %d\n", __FUNCTION__, fb_id);
    
    /*if(dfb_config->mst_gwin_disable || sdev->layer_zorder[slay->layer_index] < 1)
        bGWinEnable = false;*/


    sdev->layer_active[slay->layer_index] = true;

    sreg->config_dirtyFlag = CLRCF_NONE;

#ifdef USE_MI_RENDER
    MI_RENDER_COMMIT();
#else
    DRM_ATOMIC_COMMIT( 0 );
#endif

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}

static DFBResult
mstarSetRegion( CoreLayer                       *layer,
                void                            *driver_data,
                void                            *layer_data,
                void                            *region_data,
                CoreLayerRegionConfig           *config,
                CoreLayerRegionConfigFlags       updated,
                CoreSurface                     *surface,
                CorePalette                     *palette,
                CoreSurfaceBufferLock           *lock )
{
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter = NULL;
    FusionSHMPoolShared     *shm_pool = NULL;
    MSTARDriverData         *sdrv = driver_data;
    MSTARDeviceData         *sdev = sdrv->dev;

    int ret;


    if(dfb_config->mst_null_display_driver)
        return DFB_OK;


    //_RecordLayerCurPhyAddr(sdrv, sdev, lock, slay->layer_index);

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    parameter->reg_data= region_data;
    parameter->lock = lock;

    memcpy(&parameter->config, config, sizeof(*config));
    memcpy(&parameter->flags, &updated, sizeof(updated));

    parameter->surface = surface;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_REGION, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(config,&parameter->config,sizeof(*config));
    memcpy(&updated,&parameter->flags,sizeof(updated));
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

    MTKDRMData *drmdata = &sdrv->DrmData;

    D_ASSERT( sdev != NULL );
    D_ASSERT( slay != NULL );

    D_ASSERT( slay->layer_index >= 0 );
    D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    sdev->layer_refcnt[slay->layer_index]--;

    if(sdev->layer_refcnt[slay->layer_index] <= 0)
    {
        for(int i=0; i<MSTARGFX_MAX_LAYER_BUFFER ; i++)
        {
            if (sdev->mstarLayerBuffer[i].layer_index == slay->layer_index &&
                sdev->mstarLayerBuffer[i].u16SlotUsed)
            {  
                sdev->mstarLayerBuffer[i].u16SlotUsed = 0;
                sdev->mstarLayerBuffer[i].physAddr = 0;

                // remove fb from drm.
                DBG_LAYER_MSG("[DFB] %s, layer %d, remove buffer fb_id = %d\n", __FUNCTION__,
                		slay->layer_index, sdev->mstarLayerBuffer[i].fb_id);
                if (drmModeRmFB(drmdata->drm_fd, sdev->mstarLayerBuffer[i].fb_id)) {
                    D_WARN("[DFB] Could not remove buffer fb_id = %d!! errno=%d, %m\n",
				sdev->mstarLayerBuffer[i].fb_id, errno);
                }
            }
    	}
    }

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


static DFBResult
_mstarUpdateRegion( CoreLayer                   *layer,
                    void                        *driver_data,
                    void                        *layer_data,
                    void                        *region_data,
                    CoreSurface                 *surface,
                    const DFBRegion             *update,
                    CoreSurfaceBufferLock       *lock )
{
    MSTARLayerData  *slay = layer_data;
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARRegionData *sreg = region_data;

    hal_phy halPhys = 0;

    int bpp = DFB_BYTES_PER_PIXEL(sreg->config.format);
    hal_phy x1, y1, x2, y2;

    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    // if no need partial update.
    if ( ((dfb_config->mst_enable_gop_gwin_partial_update == false) ||
         (lock->phys == 0 && lock->pitch == 0)) )
        return DFB_OK;

    fusion_skirmish_prevail( &sdev->beu_lock );


    DBG_LAYER_MSG("[DFB] %s, %d, update region (%d, %d, %d,%d) , config (%d, %d)\n",
            __FUNCTION__, __LINE__,
            update->x1, update->y1, update->x2, update->y2,
            sreg->config.width, sreg->config.height );

    if(dfb_config->bGOPUsingHWTLB)
        halPhys = lock->phys;
    else {
        halPhys = _BusAddrToHalAddr(((u64)lock->phys_h << SHIFT32) | lock->phys);
    }

    DBG_LAYER_MSG("[DFB] lock->phys = 0x%08x, lock->pitch = %d, halPhys = 0x%08llx,\n", lock->phys, lock->pitch, halPhys);

    x1 = (update->x1 < 0) ? 0 : ALIGN(update->x1, 4);
    y1 = (update->y1 < 0) ? 0 : ALIGN(update->y1, 4);
    x2 = (update->x2 >= (sreg->config.width - 1) ) ? (ALIGN(sreg->config.width - 1, 4)) : ALIGN(update->x2, 4);
    y2 = (update->y2 >= (sreg->config.height - 1)) ? (ALIGN(sreg->config.height - 1, 4)) : ALIGN(update->y2, 4);

    // get gwininfo, and modify it using new gwininfo.


    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}


static DFBResult
mstarUpdateRegion( CoreLayer                *layer,
                    void                    *driver_data,
                    void                    *layer_data,
                    void                    *region_data,
                    CoreSurface             *surface,
                    const DFBRegion         *update,
                    CoreSurfaceBufferLock   *lock )
{
    MSTARLayerData         *slay = layer_data;
    CoreLayerCallParameter *parameter;
    FusionSHMPoolShared    *shm_pool;

    int ret;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(region_data);
    D_ASSERT(surface);
    D_ASSERT(layer->core);

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if (!parameter)
     return DFB_NOSHAREDMEMORY;

    parameter->reg_data    = region_data;
    parameter->surface     = surface;
    parameter->update.x1   = update->x1;
    parameter->update.y1   = update->y1;
    parameter->update.x2   = update->x2;
    parameter->update.y2   = update->y2;
    parameter->lock        = lock;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_UPDATE_REGION, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;
}


// If you support double buffer or triple buffer, you
// should implement the feature in FlipRegion().

static DFBResult
_mstarFlipRegion( CoreLayer *layer,
                  void                      *driver_data,
                  void                      *layer_data,
                  void                      *region_data,
                  CoreSurface               *surface,
                  DFBSurfaceFlipFlags        flags,
                  CoreSurfaceBufferLock     *lock )
{
    const u8 timeThreshold = 30;
    u8 timeouttimes = 0;
    u16 tagID = 0;
    u16 u16QueueCnt = 0;
    u16 targetQueueCnt = 0;
    hal_phy halPhys = 0;
    //int bpp = 0;
    int x1 = 0, y1 = 0;
    int slotId;

    MSTARDriverData    *sdrv = driver_data;

    if(sdrv == NULL)
      return DFB_FAILURE;

    MSTARDeviceData    *sdev = sdrv->dev;
    MSTARLayerData     *slay = layer_data;
    MSTARRegionData    *sreg = region_data;

    MTKDRMData *drmdata = &sdrv->DrmData;

    if(dfb_config == NULL || sdev == NULL || slay == NULL || sreg == NULL || lock == NULL)
      return DFB_FAILURE;

    if(dfb_config && dfb_config->mst_null_display_driver)
        return DFB_OK;

    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    D_ASSERT( region_data != NULL );
    D_ASSERT( surface != NULL );
    D_ASSERT( lock && lock->allocation);
    D_ASSERT( slay != NULL );
    D_ASSERT( slay->layer_index >= 0 );
    D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );
    D_ASSERT( sdrv != NULL );
    D_ASSERT( sdev != NULL );


    if(slay->ShadowFlags & SLF_SHADOW_LAYER_BOOLEAN)
    {
        return DFB_OK;
    }


    if(lock && slay) // pass to master to flip on libdrm.
      _RecordLayerCurPhyAddr(sdrv, sdev, lock, slay->layer_index);

    halPhys = sdrv->layerFlipInfo[slay->layer_index].CurPhysAddr;

    // For source rectangle implement. 4 pixel alignment
    x1 = ALIGN(sreg->config.source.x, PIXEL_ALIGNMENT);
    y1 = ALIGN(sreg->config.source.y, PIXEL_ALIGNMENT);
    //halPhys = halPhys + (y1 * lock->pitch + x1 * bpp);

    //tagID = (lock != NULL && lock->allocation != NULL) ? (u16) lock->allocation->gfxSerial.serial : 0;

    // TODO : how to implement force_wait_vsync on drm?
    if (dfb_config->mst_force_wait_vsync)
    {
        targetQueueCnt = 1;
    }
    else
    {
        if(surface->config.caps & DSCAPS_TRIPLE)
            targetQueueCnt = TWO_FRAME;
        else//  if(surface->config.caps & DSCAPS_DOUBLE)
            targetQueueCnt = 1;
    }

    DBG_LAYER_MSG("[DFB] mstarFlipRegion: lock->phys: 0x%08X, lock->pitch: %d (pid = %d)\n", lock->phys, lock->pitch, getpid());
    DBG_LAYER_MSG("[DFB] mstarFlipRegion: halPhys: 0x%08llX, slay->gop_index: %d, slay->gop_dst: %d (pid = %d)\n", halPhys, slay->gop_index, slay->gop_dst, getpid());


    // get drm fb_id. 
    u32 fb_id = _FindDrmFbid( sdrv, sdev, slay->layer_index, slay->gop_index, surface, lock->buffer, halPhys, lock->pitch, lock->size );

    dfb_print_duration(DF_MEASURE_START, "GFLIP SWAP TIME");

    MI_HANDLE planehandle = MI_HANDLE_NULL;
    openMiRenderplane(slay->gop_index, &planehandle);

    MI_RENDER_CallbackInputParams_t stInputParams = {0, };
    //only register one callback can use random value
    stInputParams.u64CallbackId = 0;
    stInputParams.pfEventCallback = (MI_RENDER_EventCallback)flipdone_callback;
    stInputParams.u32EventFlags = E_MI_RENDER_CALLBACK_EVENT_FLIP_FRMAE_DONE;

    // set CallbackOutputParams_t
    MI_RENDER_CallbackOutputParams_t stOutputParams = {0, };

    CHECK_MI_RET( MI_RENDER_RegisterCallback(planehandle, &stInputParams, &stOutputParams),
		"MI_RENDER_RegisterCallback", planehandle);

    // TODO : call drm atomic commit fb_id for flip.
    mtk_atomic_set_plane_flip( drmdata, fb_id, slay->gop_index, x1, y1, surface->config.size.w, surface->config.size.h);

    CHECK_MI_RET(MI_RENDER_UnRegisterCallback(planehandle, &stInputParams),
		"MI_RENDER_UnRegisterCallback", planehandle);

    dfb_print_duration(DF_MEASURE_END, "GFLIP SWAP TIME");
    


    fusion_skirmish_prevail( &sdev->beu_lock );
    dfb_surface_flip( surface, false );
    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}


static DFBResult
mstarFlipRegion( CoreLayer *layer,
                  void                      *driver_data,
                  void                      *layer_data,
                  void                      *region_data,
                  CoreSurface               *surface,
                  DFBSurfaceFlipFlags        flags,
                  CoreSurfaceBufferLock     *lock )
{
    MSTARLayerData         *slay = layer_data;
    CoreLayerCallParameter *parameter;
    FusionSHMPoolShared    *shm_pool;

    int ret;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(region_data);
    D_ASSERT(surface);
    D_ASSERT(layer->core);

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if (!parameter)
     return DFB_NOSHAREDMEMORY;

    parameter->reg_data    = region_data;
    parameter->surface     = surface;
    parameter->flipflags   = flags;
    parameter->lock        = lock;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_FLIP_REGION, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;
}


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
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData     *slay = layer_data;
    MTKDRMData *drmdata = &sdrv->DrmData;

    CoreSurface *surface;
    int i, number_buffers;

    //CoreSurfaceBufferLock ret_lock;

    //u8 u8FBID, u8GOP_Ret;

    CoreSurfaceTypeFlags cstf_flag = 0;

    u32 u32Phys = 0;
    u32 u32_gop_index = 0;
    bool tryAgain = false;

    if(_DFBFmt2DrmFmt(config->format) == DRM_FORMAT_INVALID)
        return DFB_FAILURE;

    DBG_LAYER_MSG ("[DFB] %s (%d), w =%d, h =%d\n", __FUNCTION__, __LINE__ , config->width, config->height );


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

    if(config->surface_caps & DSCAPS_SECURE_MODE)
    {
        conf.caps |= DSCAPS_SECURE_MODE;
    }
    else
    {
        conf.caps &= ~DSCAPS_SECURE_MODE;
    }

    cstf_flag = CSTF_LAYER;

    if (dfb_config->mst_GPU_AFBC) {
        if (drmdata->afbc_idx & (1 << slay->gop_index))
            cstf_flag |= CSTF_AFBC;
    }

    ret = dfb_surface_create( layer->core,
                              &conf,
                              cstf_flag,
                              (unsigned long)layer->shared->layer_id,
                              NULL,
                              ret_surface );
    if(ret)
    {
        D_DEBUG_AT( MSTAR_Layer, "\nstar_primaryAllocateSurface failed-->%d\n", ret);
        return ret;
    }

    surface = *ret_surface;

    if(conf.caps & DSCAPS_TRIPLE)
    {
        number_buffers = TRIPLE_BUFFER;
    }
    else if(conf.caps & DSCAPS_DOUBLE)
    {
        number_buffers = DOUBLE_BUFFER;
    }
    else
    {
        number_buffers = SINGLE_BUFFER;
    }

TRY_AGAIN:
    for(i=0; i<number_buffers; i++)
    {
        CoreSurfaceBufferRole bufferRole;

        switch(i)
        {
            case ALLOC_IDLE:
                    bufferRole = CSBR_IDLE;
                    break;
            case ALLOC_BACK:
                    bufferRole = CSBR_BACK;
                    break;
            case ALLOC_FRONT:
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
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;

    DFBResult         ret;
    CoreSurfaceConfig conf;

    int i = 0, number_buffers = 0;

    CoreSurfaceBufferLock ret_lock;

    u32 u32Phys = 0;
    bool tryAgain = false;

    DBG_LAYER_MSG ("[DFB] %s (%d), w =%d, h =%d, sdev : %p\n", __FUNCTION__, __LINE__ , config->width, config->height, sdev );

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

    if(config->surface_caps & DSCAPS_SECURE_MODE)
    {
        conf.caps |= DSCAPS_SECURE_MODE;
    }
    else
    {
        conf.caps &= ~DSCAPS_SECURE_MODE;
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
        number_buffers = TRIPLE_BUFFER;
    }
    else if(conf.caps & DSCAPS_DOUBLE)
    {
        number_buffers = DOUBLE_BUFFER;
    }
    else
    {
        number_buffers = SINGLE_BUFFER;
    }

TRY_AGAIN:

    for(i = 0; i < number_buffers; i++)
    {
        CoreSurfaceBufferRole bufferRole;

        switch(i)
        {
            case REALLOC_FRONT:
                    bufferRole = CSBR_FRONT;
                    break;
            case REALLOC_BACK:
                    bufferRole = CSBR_BACK;
                    break;
            case REALLOC_IDLE:
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
    MSTARDriverData *sdrv = driver_data;

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


/*
* Move the layer below or on top of others (z position).
*/
DFBResult
_mstar_SetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    MTKDRMData *drmdata = &sdrv->DrmData;
    
	
    DBG_LAYER_MSG("[DFB] %s(%d): level = %d\n", __FUNCTION__, __LINE__, level);

    fusion_skirmish_prevail( &sdev->beu_lock );

    if (dfb_config->mst_gwin_disable)
        level = -1;

    /**/
    if(sdev->layer_zorder[slay->layer_index] != level)
    {
        sdev->layer_zorder[slay->layer_index] = level;

#ifdef USE_MI_RENDER
        MI_HANDLE planehandle = MI_HANDLE_NULL;
        openMiRenderplane(slay->gop_index, &planehandle);

        if (level <= 0) {
            DBG_LAYER_MSG("[DFB] %s, disable plane idx : %d, layer index : %d\n", __FUNCTION__, drmdata->plane_idx, slay->gop_index );

            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_FBID, 0), \
                "MI_RENDER_AddDeviceProperty", planehandle);
        }
        else {
            DBG_LAYER_MSG("[DFB] %s, plane idx : %d, layer index : %d, level = %d\n", __FUNCTION__, drmdata->plane_idx, slay->gop_index, level );

            CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_ZORDER, level), \
                "MI_RENDER_AddDeviceProperty", planehandle);
        }

        MI_RENDER_COMMIT();

#else
	drmModePlaneRes *plane_res = drmdata->plane_resources;
        drmModeAtomicReq *req;
        uint32_t plane_id = plane_res->planes[drmdata->plane_idx + slay->gop_index];
        int ret;

        // set drm layer level.
        drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);

        req = drmModeAtomicAlloc();

        // disable plane
        if (level <= 0)
        {
            DBG_LAYER_MSG("[DFB] %s, disable plane idx : %d, layer index : %d\n", __FUNCTION__, drmdata->plane_idx, slay->gop_index );

            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, 0);
            drmModeAtomicAddProperty(req, plane_id, drmdata->property_fb_id, 0);
        }
        else
        {
            drmModeObjectProperties *props;

            drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);
            // need fb_id or not ?
            //drmModeAtomicAddProperty(req, plane_id, drmdata->property_fb_id, fb_id);

            props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
            uint32_t property_zpos_id = get_property_id(drmdata->drm_fd, props, "zpos");

            DBG_LAYER_MSG("[DFB] set layer %d to zpos %d\n", slay->gop_index, level);

            drmModeAtomicAddProperty(req, plane_id, property_zpos_id, level);

            drmModeFreeObjectProperties(props);
        }


        DRM_ATOMIC_COMMIT( 0 );
#endif
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}


DFBResult
mstar_SetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level )
{
    DFBResult ret = DFB_OK;
    MSTARLayerData          *slay = layer_data;
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

DFBResult
_mstar_SetColorAdjustment( CoreLayer              *layer,
                           void                   *driver_data,
                           void                   *layer_data,
                           DFBColorAdjustment     *adjustment )
{
    DFBResult ret = DFB_OK;
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    if((adjustment->flags & DCAF_HUE)            ||
       (adjustment->flags & DCAF_SATURATION))
    {
        D_WARN("SetColorAdjustment don't support these adjustment yet! \n");
        return DFB_UNSUPPORTED;
    }

    /*fusion_skirmish_prevail( &sdev->beu_lock );


    if(adjustment->flags & DCAF_CONTRAST)
    {
         D_WARN("flag DCAF_CONTRAST not set! \n");
         ret = DFB_OK;
    }
    else
    {
         D_WARN("flag DCAF_CONTRAST not set! \n");
         ret = DFB_FAILURE;
    }

    if(adjustment->flags & DCAF_BRIGHTNESS)
    {
          ret = DFB_FAILURE;
    }
    else
    {
        D_WARN("flag DCAF_BRIGHTNESS not set! \n");
        ret = DFB_FAILURE;
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );*/

    return ret;
}

DFBResult
mstar_SetColorAdjustment( CoreLayer              *layer,
                          void                   *driver_data,
                          void                   *layer_data,
                          DFBColorAdjustment     *adjustment)
{
    DFBResult ret = DFB_OK;
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;


    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->coloradjustment, adjustment, sizeof(DFBColorAdjustment));

    if (fusion_call_execute(&slay->call, FCEF_NONE, CLC_SET_COLORADJUSTMENT, parameter, &ret))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(adjustment,&parameter->coloradjustment,sizeof(DFBColorAdjustment));
    SHFREE(shm_pool, parameter);

    return ret;
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
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData           *sdev = sdrv->dev;

    MTKDRMData *drmdata = &sdrv->DrmData;
    drmModePlaneRes *plane_res;
    drmModeAtomicReq *req;
    int ret =0;

    DBG_LAYER_MSG("[DFB] SetHMirror(%s), SetVMirror(%s)\n", (HEnable? "TRUE":"FALSE"), (VEnable? "TRUE":"FALSE"));

    // set GOP H & V Mirror.
    //ret = mtk_atomic_set_plane_mirror(drmdata, HEnable, VEnable);
#ifdef USE_MI_RENDER
    
    for (int i = 0; i < dfb_config->mst_gop_counts; i++)
    {
        MI_HANDLE planehandle = MI_HANDLE_NULL;
        openMiRenderplane( dfb_config->mst_gop_available[i], &planehandle);

        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_H_MIRROR, \
			(HEnable > 0)? 1 : 0), \
                "MI_RENDER_AddDeviceProperty", planehandle);

        CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_V_MIRROR, \
			(VEnable > 0)? 1 : 0), \
                "MI_RENDER_AddDeviceProperty", planehandle);
    }

    MI_RENDER_COMMIT();

#else

    plane_res = drmdata->plane_resources;

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    req = drmModeAtomicAlloc();

    for (int i = 0; i < dfb_config->mst_gop_counts; i++)
    {
        uint32_t plane_id = plane_res->planes[drmdata->plane_idx + dfb_config->mst_gop_available[i]];

        drmModeObjectProperties *props = drmModeObjectGetProperties(drmdata->drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
        // H mirror not support
        drmdata->property_H_Mirror_id = get_property_id(drmdata->drm_fd, props, "H-Mirror");
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_H_Mirror_id, (HEnable > 0)? 1 : 0);
        
        drmdata->property_V_Mirror_id = get_property_id(drmdata->drm_fd, props, "V-Mirror");        
        drmModeAtomicAddProperty(req, plane_id, drmdata->property_V_Mirror_id, (VEnable > 0)? 1 : 0);

        drmModeFreeObjectProperties(props);
    }


    DRM_ATOMIC_COMMIT( 0 );
#endif

    return DFB_OK;
}

DFBResult
mstar_SetHVMirrorEnable( CoreLayer              *layer,
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



DFBResult
_mstar_SetHVScale( CoreLayer              *layer,
                   void                   *driver_data,
                   void                   *layer_data,
                   void                   *region_data,
                   int                     HScale,
                   int                     VScale)
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;
    MSTARRegionData *sreg = region_data;

    DBG_LAYER_MSG("[DFB] %s(%d): HScale = %d  VScale=%d, slay->gop_index:%d \n", __FUNCTION__, __LINE__,
        HScale, VScale, slay->gop_index);

#ifdef USE_MI_RENDER

    MI_HANDLE planehandle = MI_HANDLE_NULL;
    openMiRenderplane(slay->gop_index, &planehandle);

    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_W, HScale), \
            "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_CONTROLLER_H, VScale), \
            "MI_RENDER_AddDeviceProperty", planehandle);

    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_W, sreg->config.width << SHIFT16), \
            "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_H, sreg->config.height << SHIFT16), \
            "MI_RENDER_AddDeviceProperty", planehandle);

    MI_RENDER_COMMIT();

#else

    MTKDRMData *drmdata = &sdrv->DrmData;
    drmModePlaneRes *plane_res = drmdata->plane_resources;
    drmModeAtomicReq *req;
    u8 plane_idx = drmdata->plane_idx + slay->gop_index;
    uint32_t plane_id = plane_res->planes[plane_idx];
    int ret;

    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    req = drmModeAtomicAlloc();

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_w_id, HScale);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_h_id, VScale);

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, sreg->config.width << SHIFT16);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, sreg->config.height << SHIFT16);


    DRM_ATOMIC_COMMIT( 0 );

#endif

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


// Patch for time expand of display the boot logo.
DFBResult
_mstar_SetBootLogoPatch( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         int                     miusel)
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    MTKDRMData *drmdata = &sdrv->DrmData;
    int ret = mtk_disable_bootlogo(drmdata->drm_fd);
    if ( ret ) {
        D_WARN("[DFB] %s, disable bootlogo failed! errno = %d\n", __FUNCTION__, ret);
        return DFB_FAILURE;
    }
    return DFB_OK;
}


DFBResult
mstar_SetBootLogoPatch( CoreLayer              *layer,
                        void                   *driver_data,
                        void                   *layer_data,
                        int                     miusel)
{
    MSTARLayerData          *slay = layer_data;
    MSTARDriverData         *sdrv = driver_data;
    CoreLayerCallParameter  *parameter = NULL;
    FusionSHMPoolShared     *shm_pool = NULL;
    DFBResult ret = DFB_OK;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(layer->core);

    if(dfb_config->mst_null_display_driver)
        return DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
        return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->miusel,&miusel,sizeof(miusel));
    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_GOPBLOGO, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }
      
    SHFREE(shm_pool, parameter);

    return ret;    
}

DFBResult
_mstar_ScaleSourceRectangle( CoreLayer              *layer,
                            void                   *layer_data,
                            void                   *driver_data,
                            int x, int y, int w, int h)
{
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;
    MTKDRMData *drmdata = &sdrv->DrmData;
    int rect_x = x, rect_y = y, rect_w = w, rect_h = h;

    drmModeAtomicReq *req;

    if ( rect_w <= 0 || rect_h <= 0 || rect_x < 0 || rect_y  < 0 )
    {
        DBG_LAYER_MSG("[DFB] %s line %d : invalid area argument!restore origin setting!\n",__FUNCTION__,__LINE__);
        rect_w = sdev->layer_screen_size[slay->layer_index].width;
        rect_h = sdev->layer_screen_size[slay->layer_index].height;
        rect_x = 0;
        rect_y = 0;
    }

#ifdef USE_MI_RENDER
    MI_HANDLE planehandle = MI_HANDLE_NULL;
    openMiRenderplane(slay->gop_index, &planehandle);

    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_X, rect_x), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_Y, rect_y), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_W, rect_w << SHIFT16), \
                    "MI_RENDER_AddDeviceProperty", planehandle);
    CHECK_MI_RET(MI_RENDER_AddDeviceProperty(planehandle, E_MI_RENDER_DEVICE_PROPERTY_SOURCE_H, rect_h << SHIFT16), \
                    "MI_RENDER_AddDeviceProperty", planehandle);

   MI_RENDER_COMMIT();

#else
    drmSetClientCap(drmdata->drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);

    uint32_t plane_id = drmdata->plane_resources->planes[drmdata->plane_idx + slay->gop_index];

    req = drmModeAtomicAlloc();

    drmModeAtomicAddProperty(req, plane_id, drmdata->property_crtc_id, drmdata->crtc_id);

    // drm atomic add for source.
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_x_id, rect_x);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_y_id, rect_y);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_w_id, rect_w << SHIFT16);
    drmModeAtomicAddProperty(req, plane_id, drmdata->property_src_h_id, rect_h << SHIFT16);

    DRM_ATOMIC_COMMIT( 0 );
#endif

    DBG_LAYER_MSG("[DFB] %s : inputRect  (x = %d, y = %d, w = %d, h = %d)\n",__FUNCTION__,rect_x, rect_y, rect_w, rect_h);

    return DFB_OK;
}

static DFBResult
mstar_ScaleSourceRectangle( CoreLayer *layer,
                            void *layer_data,
                            int x, int y, int w, int h )
{

    MSTARLayerData  *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;
    int ret;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if( !parameter )
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->x, &x, sizeof(int));
    memcpy(&parameter->y, &y, sizeof(int));
    memcpy(&parameter->w, &w, sizeof(int));
    memcpy(&parameter->h, &h, sizeof(int));

    if ( fusion_call_execute(&slay->call, FCEF_NONE, CLC_SET_ZOOMMODE, parameter, &ret) )
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;
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

        case CLC_TEST_REGION:
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
            break;

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

            *ret_val = _mstar_SetLayerlevel( layer,
                                            layer->driver_data,
                                            layer->layer_data,
                                            *level );
        }
            break;

        case CLC_SET_COLORADJUSTMENT:
        {
            DFBColorAdjustment *adjustment;
            adjustment = &parameter->coloradjustment;

            *ret_val = _mstar_SetColorAdjustment( layer,
                                                  layer->driver_data,
                                                  layer->layer_data,
                                                  adjustment);
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

        case CLC_SET_ZOOMMODE:
        {
                int *x, *y, *w, *h;
                x = &parameter->x;
                y = &parameter->y;
                w = &parameter->w;
                h = &parameter->h;

                *ret_val = _mstar_ScaleSourceRectangle( layer,
                                                        layer->layer_data,
                                                        layer->driver_data,
                                                        *x, *y, *w, *h );
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

        case CLC_SET_GOPBLOGO:
        {
            int *miusel;
            miusel = &parameter->miusel;

            *ret_val = _mstar_SetBootLogoPatch( layer,
                                                layer->driver_data,
                                                layer->layer_data,
                                               *miusel);
        }
            break;

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

        case CLC_FLIP_REGION:
        {
            CoreSurface *surface;

            surface = parameter->surface;

            *ret_val = _mstarFlipRegion( layer,
                                           layer->driver_data,
                                           layer->layer_data,
                                           parameter->reg_data,
                                           surface,
                                           parameter->flipflags,
                                           parameter->lock );
        }
            break;

        default:
            D_BUG( "[DFB] Error!!! Unknown  Command '%d'", command );
            *ret_val = DFB_BUG;
    }

    return FCHR_RETURN;
}


DFBResult
mstarGetScreen( CoreLayer               *layer,
                CoreScreen             **ret_screen )
{
    DFBResult ret = DFB_OK;

    MSTARDriverData *sdrv = (MSTARDriverData *)layer->driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer->layer_data;

    *ret_screen = sdrv->op_screen;

    return ret;
}


//- AddRegion() will be called when the window is first displayed
//- TestRegion() and SetRegion() are called when the configuration changes
//  eg. the window is moved or resized.
//- RemoveRegion() will be called when the window is destroyed.

DisplayLayerFuncs
mstarLayerFuncs = {
    LayerDataSize           :   mstarLayerDataSize,
    RegionDataSize          :   mstarRegionDataSize,
    InitLayer               :   mstarInitLayer,
    TestRegion              :   mstarTestRegion,
    AddRegion               :   mstarAddRegion,
    SetRegion               :   mstarSetRegion,
    RemoveRegion            :   mstarRemoveRegion,
    FlipRegion              :   mstarFlipRegion,
    AllocateSurface         :   mstar_primaryAllocateSurface,
    ReallocateSurface       :   mstar_primaryReallocateSurface,
    DeallocateSurface       :   mstar_primaryDeallocateSurface,
    GetLevel                :   mstar_GetLayerlevel,
    SetLevel                :   mstar_SetLayerlevel,
    SetColorAdjustment      :   mstar_SetColorAdjustment,
    SetHVMirrorEnable       :   mstar_SetHVMirrorEnable,
    SetHVScale              :   mstar_SetHVScale,
    SetBootLogoPatch        :   mstar_SetBootLogoPatch,
    UpdateRegion            :   mstarUpdateRegion,
    GetScreen               :   mstarGetScreen,
    ScaleSourceRectangle    :   mstar_ScaleSourceRectangle
};
