/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef _UAPI_MSTAR_FB_GRAPHIC_H
#define _UAPI_MSTAR_FB_GRAPHIC_H

#include <linux/ioctl.h>
#include <linux/types.h>

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    //E_MI_OSD_COLOR_FORMAT_RGB565
    E_MI_FB_COLOR_FMT_RGB565 = 4,
	//E_MI_OSD_COLOR_FORMAT_ARGB1555
    E_MI_FB_COLOR_FMT_ARGB1555 = 5,
    //E_MI_OSD_COLOR_FORMAT_ARGB4444
    E_MI_FB_COLOR_FMT_ARGB4444 = 6,
    //E_MI_OSD_COLOR_FORMAT_ARGB8888
    E_MI_FB_COLOR_FMT_ARGB8888 = 8,
    //E_MI_OSD_COLOR_FORMAT_YUV422_YUYV
    E_MI_FB_COLOR_FMT_YUV422 = 9,
    //E_MI_OSD_COLOR_FORMAT_GENERIC
    E_MI_FB_COLOR_FMT_INVALID  = 11,
}MI_FB_ColorFmt_e;

typedef enum
{
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS = 0x1,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE = 0x2,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE = 0x4,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE = 0x8,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_PREMUL = 0x10,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB = 0x20,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_OUTPUT_COLORSPACE = 0x40,
    E_MI_FB_DISPLAYLAYER_ATTR_MASK_DST_DISP = 0x80,
}MI_FB_DisplayLayerAttrMaskbit_e;

typedef struct MI_FB_GlobalAlpha_s
{
    __u8 bAlphaEnable;	/* alpha enable flag */
    __u8 bAlphaChannel;	/* alpha channel enable flag */
    __u8 u8Alpha0; /*alpha0 value*/
    __u8 u8Alpha1; /*alpha1 value*/
    __u8 u8GlobalAlpha;	/* global alpha value */
    __u8 u8Reserved;   /* reserved*/
}MI_FB_GlobalAlpha_t;

typedef struct MI_FB_ColorKey_s
{
    __u8 bKeyEnable;
    __u8 u8Red;
    __u8 u8Green;
    __u8 u8Blue;
}MI_FB_ColorKey_t;

typedef struct MI_FB_Rectangle_s
{
    __u16 u16Xpos;
    __u16 u16Ypos;
    __u16 u16Width;
    __u16 u16Height;
}MI_FB_Rectangle_t;

typedef struct MI_FB_AutoDectBufInfo_s
{
    __u8 bEnable;
    __u8 u8AlphaThreshold;
    __u8 bLargeThanThreshold;
}MI_FB_AutoDectBufInfo_t;

typedef enum
{
    //DRV_FB_GOPOUT_RGB
    E_MI_FB_OUTPUT_RGB = 0,
    //DRV_FB_GOPOUT_YUV
    E_MI_FB_OUTPUT_YUV = 1,
}MI_FB_OutputColorSpace_e;

typedef enum
{
    //E_DRV_FB_GOP_DST_IP0
    E_MI_FB_DST_IP0 = 0,
    //E_DRV_FB_GOP_DST_IP0_SUB
    E_MI_FB_DST_IP0_SUB = 1,
    //E_DRV_FB_GOP_DST_MIXER2VE
    E_MI_FB_DST_MIXER2VE = 2,
    //E_DRV_FB_GOP_DST_OP0
    E_MI_FB_DST_OP0 = 3,
    //E_DRV_FB_GOP_DST_VOP
    E_MI_FB_DST_VOP = 4,
    //E_DRV_FB_GOP_DST_IP1
    E_MI_FB_DST_IP1 = 5,
    //E_DRV_FB_GOP_DST_IP1_SUB
    E_MI_FB_DST_IP1_SUB = 6,
    //E_DRV_FB_GOP_DST_MIXER2OP
    E_MI_FB_DST_MIXER2OP = 7,
    //E_DRV_FB_GOP_DST_VOP_SUB
    E_MI_FB_DST_VOP_SUB = 8,
    //E_DRV_FB_GOP_DST_FRC
    E_MI_FB_DST_FRC = 9,
    //E_DRV_FB_GOP_DST_VE
    E_MI_FB_DST_VE = 10,
    //E_DRV_FB_GOP_DST_BYPASS
    E_MI_FB_DST_BYPASS = 11,
    //E_DRV_FB_GOP_DST_OP1
    E_MI_FB_DST_OP1 = 12,
    //E_DRV_FB_GOP_DST_MIXER2OP1
    E_MI_FB_DST_MIXER2OP1 = 13,
    //E_DRV_FB_GOP_DST_DIP
    E_MI_FB_DST_DIP = 14,
    //E_DRV_FB_GOP_DST_GOPScaling
    E_MI_FB_DST_GOPScaling  = 15,
    //E_DRV_FB_GOP_DST_OP_DUAL_RATE
    E_MI_FB_DST_OP_DUAL_RATE = 16,
    //E_DRV_FB_GOP_DST_INVALID
    E_MI_FB_DST_INVALID = 17,
}MI_FB_DstDisplayplane_e;

typedef enum
{
    E_MI_FB_DST_HD_PATH_OUTPUT = 0x0,
    E_MI_FB_DST_HD_PATH_INPUT,
    E_MI_FB_DST_HD_PATH_MAX,

    E_MI_FB_DST_SD_PATH_OUTPUT = 0x10,
    E_MI_FB_DST_SD_PATH_INPUT,
    E_MI_FB_DST_SD_PATH_MAX,
} MI_FB_LayerDst_e;

typedef struct MI_FB_CropParams_s
{
    MI_BOOL bEnableCrop;
    MI_U16 u16AfbcCropX;
    MI_U16 u16AfbcCropY;
    MI_U16 u16AfbcCropWidth;
    MI_U16 u16AfbcCropHeight;
}MI_FB_AfbcCropParams_t;

typedef struct MI_FB_DisplayLayerAttr_s
{
    __u32 u32Xpos;    /**the x pos of orign point in screen*/
    __u32 u32YPos;   /**the y pos of orign point in screen*/
    __u32 u32dstWidth; /**display buffer dest with in screen*/
    __u32 u32dstHeight; /**display buffer dest hight in screen*/
    __u32 u32DisplayWidth;  /**the width of display buf in fb */
    __u32 u32DisplayHeight;  /**the height of display buf in fb. */
    __u32 u32ScreenWidth;  /**the width of screen */
    __u32 u32ScreenHeight; /** the height of screen */
    __u8 bPreMul;  /**the data drawed in buffer whether is premultiply alpha or not*/
    MI_FB_ColorFmt_e eFbColorFmt; /**the color format of framebuffer*/
    MI_FB_OutputColorSpace_e  eFbOutputColorSpace;  /**output color space*/
    MI_FB_DstDisplayplane_e  eFbDestDisplayPlane;  /**destination displayplane*/
    __u32 u32SetAttrMask; /** display attribute modify mask*/
} MI_FB_DisplayLayerAttr_t;

typedef struct MI_FB_CursorImage_s
{
    __u32 u32Width; /**width, unit pixel*/
    __u32 u32Height; /**Height, unit pixel*/
    __u32 u32Pitch; /**Pitch, unit pixel*/
    MI_FB_ColorFmt_e eColorFmt; /**Color format*/
#ifndef __KERNEL__
    const char  *data; /**Image raw data*/
#else
    const char __user *data; /**Image raw data*/
#endif
}MI_FB_CursorImage_t;

typedef enum
{
    E_MI_FB_CURSOR_ATTR_MASK_ICON = 0x1,
    E_MI_FB_CURSOR_ATTR_MASK_POS = 0x2,
    E_MI_FB_CURSOR_ATTR_MASK_ALPHA = 0x4,
    E_MI_FB_CURSOR_ATTR_MASK_SHOW = 0x8,
    E_MI_FB_CURSOR_ATTR_MASK_HIDE = 0x10,
    E_MI_FB_CURSOR_ATTR_MASK_COLORKEY = 0x20,
    E_MI_FB_CURSOR_ATTR_MASK = 0x3F
}MI_FB_CursorAttrMaskbit_e;

typedef struct MI_FB_CursorAttr_s
{
    __u32 u32XPos;
    __u32 u32YPos;
    __u32 u32HotSpotX;
    __u32 u32HotSpotY;
    MI_FB_GlobalAlpha_t stAlpha;
    MI_FB_ColorKey_t stColorKey;
    __u8 bShown;
    MI_FB_CursorImage_t stCursorImageInfo;
    __u16 u16CursorAttrMask;
}MI_FB_CursorAttr_t;


typedef struct MI_FB_MemInfo_s
{
    __u64 phyAddr; /**physical address include miu_interval info*/
    __u32 length; /**Framebuffer memory length*/
}MI_FB_MemInfo_t;


//------------------------------------------------------------------------------------------------
//Resolution Info
//------------------------------------------------------------------------------------------------
typedef struct MI_FB_OsdInfo_s
{
    __u32 u32Width;
    __u32 u32Height;
    __u32 u32Pitch;
    MI_FB_ColorFmt_e eColorFmt;
}MI_FB_OsdInfo_t;


//------------------------------------------------------------------------------------------------
//layer contrast info
//------------------------------------------------------------------------------------------------
typedef struct MI_FB_ContrastInfo_s
{
    __u16 u16ContrastY;
    __u16 u16ContrastU;
    __u16 u16ContrastV;
}MI_FB_ContrastInfo_t;

typedef struct MI_FB_HVMirror_s
{
    __u8 u8HMirror;
    __u8 u8VMirror;
}MI_FB_HVMirror_t;

typedef struct MI_FB_MMAAuthorizeParams_s
{
    __u8 au8BufferTag[16];
    __u64 phyAddr;
    __u32 u32Size;
    __u32 u32PipeID;
} MI_FB_MMAAuthorizeParams_t;

typedef enum
{
    E_MI_FB_ATTR_TYPE_V_STRETCH = 0x0,
    E_MI_FB_ATTR_TYPE_H_STRETCH,
    E_MI_FB_ATTR_TYPE_3D_MODE,
    E_MI_FB_ATTR_TYPE_MIRROR,
    E_MI_FB_ATTR_TYPE_BLINK_RATE,
    E_MI_FB_ATTR_TYPE_WAIT_SYNC,
    E_MI_FB_ATTR_TYPE_DESTINATION,
    E_MI_FB_ATTR_TYPE_SCREEN_SIZE,
    E_MI_FB_ATTR_TYPE_AUTO_DETECT_BUFFER,
    E_MI_FB_ATTR_TYPE_SWITCH_SECURE_AID,
    E_MI_FB_ATTR_TYPE_SWITCH_INTERFACE_GE,
    E_MI_FB_ATTR_TYPE_MMA_Authorize, 
    E_MI_FB_ATTR_TYPE_AFBC_MODE,
    E_MI_FB_ATTR_TYPE_AFBC_CROP,
    E_MI_FB_ATTR_TYPE_OUTPUTFPSRATIO,
    E_MI_FB_ATTR_TYPE_MAX
} MI_FB_LayerAttrType_e;

typedef enum
{
    /// GOP V-Stretch Mode
    E_MI_FB_V_STRETCH_LINEAR  = 0x0,
    E_MI_FB_V_STRETCH_LINEAR_GAIN0,
    E_MI_FB_V_STRETCH_LINEAR_GAIN1,
    E_MI_FB_V_STRETCH_DUPLICATE,
    E_MI_FB_V_STRETCH_NEAREST,
    E_MI_FB_V_STRETCH_MAX

} MI_FB_VStretchMode_e;


typedef enum
{
    /// GOP H-Stretch Mode resverd
    E_MI_FB_H_STRETCH_MAX = 0x0,
} MI_FB_HStretchMode_e;


typedef enum
{
    /// GOP 3D Mode
    E_MI_FB_3D_DISABLE  = 0x0,
    E_MI_FB_3D_TOP_BOTTOM,
    E_MI_FB_3D_LEFT_RIGHT,
    E_MI_FB_3D_MAX

} MI_FB_3dMode_e;

typedef enum
{
    E_MI_FB_MIRROR_NONE = 0,
    E_MI_FB_MIRROR_HORIZONTAL, //Left Right
    E_MI_FB_MIRROR_VERTICAL,   //Top Bottom
    E_MI_FB_MIRROR_HORIZONTAL_VERTICAL,    //Both
    E_MI_FB_MIRROR_MAX
} MI_FB_MirrorMode_e;

typedef enum
{
    MI_FB_GOP_AID_NS,
    MI_FB_GOP_AID_SDC,
    MI_FB_GOP_AID_S,
    MI_FB_GOP_AID_CSP,
    MI_FB_GOP_AID_MAX,
} MI_FB_GOP_AID_TYPE_e;

typedef enum
{
    E_MI_FB_INTERFACE_GE0,
    E_MI_FB_INTERFACE_GE1,
    E_MI_FB_INTERFACR_MAX,
}MI_FB_GE_Interface_e;

typedef union
{
    MI_FB_VStretchMode_e eLayerVStretchMode;
    MI_FB_HStretchMode_e eLayerHStrecthMode;
    MI_FB_3dMode_e  eLayer3dMode;
    MI_FB_MirrorMode_e eMirrorMode;
    MI_FB_GE_Interface_e eGEInterface;
    MI_FB_GOP_AID_TYPE_e eGOP_AID_Type;
    __u16 u16Blink_Rate;
    __u8 bWaitSync;
    MI_FB_LayerDst_e eLayerDst;
    MI_FB_Rectangle_t stScreenSize;
    MI_FB_AutoDectBufInfo_t stAutoDectBufInfo;
    MI_FB_MMAAuthorizeParams_t stOsdMMAAuthorizeParams;
    __u8 bOsdAfbc;
    MI_FB_AfbcCropParams_t stCropParams;
    __u8 u8OutputFpsRatio;
}MI_FB_LayerAttrParam_u;

typedef struct MI_FB_LayerAttr_s
{
    MI_FB_LayerAttrType_e  eLayerAttrType;
    MI_FB_LayerAttrParam_u unLayerAttrParam;
}MI_FB_LayerAttr_t;

typedef enum
{
    /// MI_OSD Flip mode
    E_MI_FB_FlipMode_Queue  = 0x0,
    E_MI_FB_FlipMode_WaitSync,
    E_MI_FB_FlipMode_MAX
} MI_FB_FlipMode_e;


typedef struct MI_FB_CropInfo_s
{
    MI_FB_Rectangle_t  inputRect;
    MI_FB_Rectangle_t  outputRect;
}MI_FB_CropInfo_t;


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define FB_IOC_MAGIC 'F'

#define FBIOGET_SCREEN_LOCATION _IOR(FB_IOC_MAGIC, 0x60, MI_FB_Rectangle_t)
#define FBIOSET_SCREEN_LOCATION _IOW(FB_IOC_MAGIC, 0x61, MI_FB_Rectangle_t)

#define FBIOGET_SHOW _IOR(FB_IOC_MAGIC, 0x62, __u8)
#define FBIOSET_SHOW _IOW(FB_IOC_MAGIC, 0x63, __u8)

#define FBIOGET_GLOBAL_ALPHA _IOR(FB_IOC_MAGIC, 0x64, MI_FB_GlobalAlpha_t)
#define FBIOSET_GLOBAL_ALPHA _IOW(FB_IOC_MAGIC, 0x65, MI_FB_GlobalAlpha_t)

#define FBIOGET_COLORKEY _IOR(FB_IOC_MAGIC, 0x66, MI_FB_ColorKey_t)
#define FBIOSET_COLORKEY _IOW(FB_IOC_MAGIC, 0x67, MI_FB_ColorKey_t)

#define FBIOGET_DISPLAYLAYER_ATTRIBUTES _IOR(FB_IOC_MAGIC, 0x68, MI_FB_DisplayLayerAttr_t)
#define FBIOSET_DISPLAYLAYER_ATTRIBUTES _IOW(FB_IOC_MAGIC, 0x69, MI_FB_DisplayLayerAttr_t)

#define FBIOGET_CURSOR_ATTRIBUTE _IOR(FB_IOC_MAGIC, 0x70, MI_FB_CursorAttr_t)
#define FBIOSET_CURSOR_ATTRIBUTE _IOW(FB_IOC_MAGIC, 0x71, MI_FB_CursorAttr_t)

#define FBIOGET_MEM_INFO _IOR(FB_IOC_MAGIC, 0x72, MI_FB_MemInfo_t)
#define FBIOSET_MEM_INFO _IOW(FB_IOC_MAGIC, 0x73, MI_FB_MemInfo_t)

#define FBIOGET_OSD_INFO _IOR(FB_IOC_MAGIC, 0x74, MI_FB_OsdInfo_t)
#define FBIOSET_OSD_INFO _IOW(FB_IOC_MAGIC, 0x75, MI_FB_OsdInfo_t)

#define FBIOGET_DFB_RESERVED _IOR(FB_IOC_MAGIC, 0x76, __u8)
#define FBIOGET_BOOTLOGO_USED _IOR(FB_IOC_MAGIC, 0x77, __u8)

#define FBIO_DISABLE_BOOTLOGO _IO(FB_IOC_MAGIC, 0x78)

#define FBIO_BEGIN_TRANSACTION _IO(FB_IOC_MAGIC, 0x79)
#define FBIO_COMMIT_TRANSACTION _IO(FB_IOC_MAGIC, 0x80)

#define FBIO_MIOSD_INIT _IO(FB_IOC_MAGIC, 0x81)

#define FBIOSET_LAYER_BRIGHTNESS  _IOW(FB_IOC_MAGIC, 0x82, __u16)
#define FBIOSET_LAYER_CONTRAST    _IOW(FB_IOC_MAGIC, 0x83,MI_FB_ContrastInfo_t)
#define FBIOSET_LAYER_HVMIRROR    _IOW(FB_IOC_MAGIC, 0x84,MI_FB_HVMirror_t)

#define FBIOGET_DISP_TIMING       _IOR(FB_IOC_MAGIC, 0x85, MI_FB_Rectangle_t)
#define FBIOSET_LAYER_DST         _IOW(FB_IOC_MAGIC, 0x86,MI_FB_LayerDst_e)

#define  FBIOGET_LAYER_ATTR    _IOWR(FB_IOC_MAGIC, 0x87, MI_FB_LayerAttr_t)
#define  FBIOSET_LAYER_ATTR    _IOW(FB_IOC_MAGIC, 0x88, MI_FB_LayerAttr_t)

#define FBIOGET_FlipMode         _IOR(FB_IOC_MAGIC, 0x89, MI_FB_FlipMode_e)
#define FBIOSET_FlipMode         _IOW(FB_IOC_MAGIC, 0x90, MI_FB_FlipMode_e)

#define FBIOSET_GOP_AUTO_DETECT  _IOW(FB_IOC_MAGIC, 0x91, __u8)
#define FBIOGET_GOP_AUTO_DETECT  _IOR(FB_IOC_MAGIC, 0x92, __u8)


#define FBIOSET_CROP_INFO        _IOW(FB_IOC_MAGIC, 0x93, MI_FB_CropInfo_t)
#define FBIOGET_CROP_INFO        _IOR(FB_IOC_MAGIC, 0x94, MI_FB_CropInfo_t)


#define FBIOGET_IOMMU_USED       _IOR(FB_IOC_MAGIC, 0x95, __u8)
#define FBIO_AllOCATE_IOMMU      _IOW(FB_IOC_MAGIC, 0x96, __u32)
#define FBIO_FREE_IOMMU          _IO(FB_IOC_MAGIC, 0x97)
#define FBIO_MIOSD_REINIT        _IO(FB_IOC_MAGIC, 0x98)

#endif
