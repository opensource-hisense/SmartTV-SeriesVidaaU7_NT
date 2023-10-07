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

#ifndef __MSTAR__LAYER_H__
#define __MTSAR__LAYER_H__

#include "mstar_types.h"

#define MSTAR_LAYER_SUPPORTED_OPTIONS  (DLOP_ALPHACHANNEL | DLOP_OPACITY | DLOP_SRC_COLORKEY)

/*
    utopia driver can't support this flag at some chip
    in order to avoid compiler error and code readability
    add this flag (4K@120) in the DFB
*/
#define DFB_E_GOP_DST_OP_DUAL_RATE 16

    
extern DisplayLayerFuncs mstarLayerFuncs;


typedef enum {
    CLC_ADD_REGION,
    CLC_TEST_REGION,
    CLC_SET_REGION,
    CLC_REMOVE_REGION,
    CLC_SET_LAYERLEVEL,
    CLC_SET_COLORADJUSTMENT,
    CLC_FORCEWRITE_ENABLE,
    CLC_FORCEWRITE_DISABLE,
    //CLC_CONFIG_DISPLAYMODE,
    CLC_SET_MIRRORMODE,
    CLC_SET_ZOOMMODE,
    CLC_SET_LBCOUPLEMODE,
    CLC_SET_GOPBYPASSMODE,
    CLC_SET_GOPSCALE,
    CLC_SET_GOPDST,
    CLC_SET_FORCEWRITE,
    CLC_SET_GOPBLOGO,
    CLC_UPDATE_REGION,
    CLC_FLIP_REGION,
    CLC_SET_GOPEXTADJUST,
    //CLC_SET_GOPAUTODETECTBUF,
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
    DFBSurfaceFlipFlags                  flipflags;
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

    bool                                 ForceWrite;
    int                                  miusel;
    int                                  x, y, w, h; /* used for zoom mode */

    // todo: remove in sti.
    int                      GopDst;    
    int                      GopExtAdjustType;
    int                      GopR2YMatrix;
    int                       GopCscParam;
    int                        GopSDR2HDR;
    bool                                 GOPAutoDetectEnable;
} CoreLayerCallParameter;

FusionCallHandlerResult
mstar_layer_funcs_call_handler(int           caller,   /* fusion id of the caller */
                               int           call_arg, /* optional call parameter */
                               void         *call_ptr, /* optional call parameter */
                               void         *ctx,      /* optional handler context */
                               unsigned int  serial,
                               int          *ret_val);

void mstar_init_gop_device( CoreGraphicsDevice  *device,
                            void  *driver_data );

void mstar_init_gop_driver( CoreGraphicsDevice  *device,
                            void                *driver_data );

void mstar_close_driver( void  *driver_data );


#endif

