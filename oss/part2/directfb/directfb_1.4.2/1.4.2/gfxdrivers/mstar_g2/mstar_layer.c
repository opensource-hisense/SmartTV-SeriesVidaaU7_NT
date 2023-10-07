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

#include <MsCommon.h>
#include  <apiGFX.h>
#include  <apiGOP.h>
//#include <apiGOP_v2.h>

#include <assert.h>

#include <drvXC_IOPort.h>
#include <drvTVEncoder.h>
#include <apiXC.h>
#include <apiPNL.h>
#include "mstar.h"
#include "mstar_types.h"
#include "mstar_layer.h"
#include "mstar_screen.h"
#include <drvMVOP.h>
#include <drvMIU.h>


//#define MS_LAYER_DEFAULT_WIDTH 1360     //replace with dfb_config->mst_layer_default_width
//#define MS_LAYER_DEFAULT_HEIGHT 768      //replace with dfb_config->mst_layer_default_height

D_DEBUG_DOMAIN(MSTAR_Layer, "MSTAR/Layer", "MSTAR Layers");


#define GOP_MIU_MASK 0x80000000

#define FRAME_PACKAGE_WIDTH_1920  1920
#define FRAME_PACKAGE_WIDTH_1280  1280
#define FRAME_PACKAGE_HEIGHT_1080 1080
#define FRAME_PACKAGE_HEIGHT_720  720

#undef ALIGN
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)


#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
/**********************************************************************************************************************/

#pragma weak  MDrv_VE_SetIOMapBase
#pragma weak  MApi_XC_SetOSD2VEMode
#pragma weak  MDrv_VE_GetStatus
#pragma weak  MApi_GOP_VE_SetOutputTiming
#pragma weak  MApi_VE_GetDstInfo
#pragma weak  MDrv_VE_Get_Output_Video_Std
E_GOP_API_Result MApi_GOP_GWIN_EnableMultiAlpha (MS_U32 u32GopIdx, MS_BOOL bEnable) __attribute__((weak));


static int _get_mutex_enum(int index)
{
   switch(index)
   {
     case 0:
          return 0;//EN_GOP_MUX0;
     case 1:
          return 1;//EN_GOP_MUX1;
     case 2:
          return 2;//EN_GOP_MUX2;
     case 3:
          return 3;//EN_GOP_MUX3;
     case 4:
          return 18;//EN_GOP_MUX4;
     default:
          assert(0);
   }

   return 0 /*EN_GOP_MUX0*/;
}

static MS_U32 mstar_dfb_OSD_RESOURCE_SetFBFmt(MS_U16 pitch, MS_U32  addr, MS_U16 fmt )
{
   printf("[DFB] set osd resource 0x%08x, 0x%08x, 0x%04x\n", addr, pitch, fmt);
   return 0x1;
}

static MS_BOOL mstar_sc_is_interlace(void)
{
    //use MApi_XC_GetStatus for IPC issue.
    XC_ApiStatus xcStatus;
    if (MApi_XC_GetStatus(&xcStatus, MAIN_WINDOW))
    {
        //printf("XC interlace=%d \n",xcStatus.bInterlace);
        return xcStatus.bInterlace;
    }
    else
    {
        printf("[DFB] Error! Cannot get XC Status! \n");
        assert(0);
    }

  //MS_XC_DST_DispInfo dstDispInfo;
  //MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo),E_GOP_XCDST_IP1_MAIN);
  //return dstDispInfo.bInterlaceMode;

  //return MDrv_MVOP_GetIsInterlace();

  return false;
}

static MS_U16 mstar_sc_get_h_cap_start(void)
{
    MS_PNL_DST_DispInfo dstDispInfo;
    MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
    return dstDispInfo.DEHST;
}

static MS_U16 mstar_sc_get_ip0_h_cap_start(void)
{
  MS_XC_DST_DispInfo dstDispInfo;
  MApi_XC_GetDstInfo(&dstDispInfo,sizeof(MS_XC_DST_DispInfo), E_GOP_XCDST_IP1_MAIN);
  return dstDispInfo.DEHST;
}

static void mstar_XC_Sys_PQ_ReduceBW_ForOSD( MS_U8 PqWin, MS_BOOL bOSD_On )
{
}

void mstar_init_gop_device(CoreDFB *core)
{
    //Init GOP:
    int i = 0;
    GOP_MuxConfig muxConfig;
    int status = 0;
    u8  curGopIndex = 0;
    bool bBankForceWrite = dfb_get_BankForceWrite();

    printf("\n[DFB] USE THE GFX : MSTAR_G2\n");

    if(dfb_config->mst_goplayer_counts > 0)
    {
        GOP_LayerConfig stLayerSetting;
        MS_U32 u32size = sizeof(GOP_LayerConfig);
        E_GOP_API_Result enResult = GOP_API_FAIL;
        stLayerSetting.u32LayerCounts = dfb_config->mst_goplayer_counts;
        for(i = 0; i < dfb_config->mst_goplayer_counts; i++)
        {
            stLayerSetting.stGopLayer[i].u32GopIndex = dfb_config->mst_goplayer[i];
            stLayerSetting.stGopLayer[i].u32LayerIndex = i;
        }

        /**
        Since API MApi_GOP_GWIN_SetMux is not clearly to describe the GOP order.
        We use  API MApi_GOP_GWIN_SetLayer instead of it.
        LayerIndex is not DFB layer.
        LayerIndex 0 is the bottomest GOP layer.
        LayerIndex 3 is the topest GOP layer.
        */
        DFB_UTOPIA_TRACE(enResult = MApi_GOP_GWIN_SetLayer(&stLayerSetting, u32size));
        if (enResult == GOP_API_SUCCESS)
        {
            DBG_LAYER_MSG("[DFB] MApi_GOP_GWIN_SetLayer  success %s(%d)\n",__FUNCTION__,__LINE__);
        }
        else
        {
            DBG_LAYER_MSG("[DFB] MApi_GOP_GWIN_SetLayer fail, return %u!!! %s(%d)\n", enResult,__FUNCTION__,__LINE__);
        }
    }
    else  if(dfb_config->mst_mux_counts > 0)
    {
        memset(&muxConfig,0,sizeof(muxConfig));
        for(i=0; i<dfb_config->mst_mux_counts; i++)
        {
            muxConfig.GopMux[i].u8MuxIndex = _get_mutex_enum(i); //patch for mux4
            muxConfig.GopMux[i].u8GopIndex = dfb_config->mst_gop_mux[i];
            DBG_LAYER_MSG("[DFB] mux[%d]:gop[%d]",i,muxConfig.GopMux[i].u8GopIndex);
        }
        muxConfig.u8MuxCounts = dfb_config->mst_mux_counts;
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetMux(&muxConfig,sizeof(muxConfig)));
    }

    //printf("[DFB] dfb_config->mst_gop_counts: %d\n", dfb_config->mst_gop_counts);

    for(i = 0; i < dfb_config->mst_gop_counts; i++)
    {
        if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
        {
              DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(dfb_config->mst_gop_available[i], true));
        }

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(dfb_config->mst_gop_available[i]));

        if (dfb_config->mst_disable_layer_init == i)
        {
            printf("[DFB] Ignore layer:%d, GOP:%d\n", dfb_config->mst_disable_layer_init, dfb_config->mst_gop_available[i]);

            if(core && core->shared)
                core->shared->bootlogo_layer_id = i; /* set the index of boot logo layer */
        }
        else
        {
            printf("[DFB] SetGOPDst layer:%d, GOP:%d\n", i, dfb_config->mst_gop_available[i]);
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetFieldInver(TRUE));
            DFB_UTOPIA_TRACE(status =MApi_GOP_GWIN_SetGOPDst(dfb_config->mst_gop_available[i], (EN_GOP_DST_TYPE)dfb_config->mst_gop_dstPlane[i]));
        }

        //Set Stretch Mode to Linear as GE:
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_HStretchMode(E_GOP_HSTRCH_6TAPE_LINEAR));

        if(bBankForceWrite && true == dfb_config->mst_bank_force_write)
        {
             DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(dfb_config->mst_gop_available[i], false));
        }

        // printf("\n[DFB] I = %d Set GOPDst status:%d; the gop number is %d;the gop dst is%d\n", i, status,dfb_config->mst_gop_available[i],dfb_config->mst_gop_dstPlane[i]);
        if(dfb_config->mst_gop_available_r[i]!= 0xf)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(dfb_config->mst_gop_available_r[i]));

            if (dfb_config->mst_disable_layer_init == i)
            {
                printf("[DFB] Ignore layer:%d, GOP:%d\n", dfb_config->mst_disable_layer_init, dfb_config->mst_gop_available_r[i]);
            }
            else
            {
                printf("[DFB] SetGOPDst layer:%d, GOP:%d\n", i, dfb_config->mst_gop_available_r[i]);
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetFieldInver(TRUE));
                DFB_UTOPIA_TRACE(status =MApi_GOP_GWIN_SetGOPDst(dfb_config->mst_gop_available_r[i], (EN_GOP_DST_TYPE)dfb_config->mst_gop_dstPlane[i]));
            }
            //Set Stretch Mode to Linear as GE:
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_HStretchMode(E_GOP_HSTRCH_6TAPE_LINEAR));

            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(dfb_config->mst_gop_available[i]));
        }

        if ( (EN_GOP_DST_TYPE)dfb_config->mst_gop_dstPlane[i] == E_GOP_DST_VE)
        {
            //For K3 Set the gopVETimingType
            VE_DrvStatus DrvStatus;
            if (MDrv_VE_GetStatus != NULL)
                DFB_UTOPIA_TRACE(MDrv_VE_GetStatus(&DrvStatus));
            GOP_VE_TIMINGTYPE gopVETimingType;
            if(DrvStatus.VideoSystem <= 2)
            {
                gopVETimingType = GOP_VE_NTSC;
            }
            else
            {
                gopVETimingType = GOP_VE_PAL;
            }
            printf("[DFB] MApi_GOP_VE_SetOutputTiming : %d\n", gopVETimingType);

            if (MApi_GOP_VE_SetOutputTiming != NULL)
                DFB_UTOPIA_TRACE(MApi_GOP_VE_SetOutputTiming(gopVETimingType));

        }
    }

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
          DFB_UTOPIA_TRACE(curGopIndex = MApi_GOP_GWIN_GetCurrentGOP());
          DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(curGopIndex, true));
    }

    if(dfb_config->mst_GOP_HMirror > 0)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetHMirror(TRUE));
    }
    else if(dfb_config->mst_GOP_HMirror == 0)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetHMirror(FALSE));
    }

    if(dfb_config->mst_GOP_VMirror > 0)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetVMirror(TRUE));
    }
    else if(dfb_config->mst_GOP_VMirror == 0)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetVMirror(FALSE));
    }

    if(dfb_config->mst_call_gop_t3d == 0)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableT3DMode(false));
    }
    else if(dfb_config->mst_call_gop_t3d == 1)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableT3DMode(true));
    }

    //Disable TransClr:
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE));

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(curGopIndex, false));
    }

#ifdef OSD2VE_SUPPORT
    //Enable OP to VEMode for U4
    if(dfb_config->mst_osd_to_ve)
    {
        // MApi_XC_SetOSD2VEMode(E_VOP_SEL_OSD_BLEND0); // Using in GOP0/1 set to OP

        if (MApi_XC_SetOSD2VEMode != NULL)
             DFB_UTOPIA_TRACE(MApi_XC_SetOSD2VEMode((EN_VOP_SEL_OSD_XC2VE_MUX)(dfb_config->mst_xc_to_ve_mux)));
    }
#endif

}


void mstar_init_gop_driver( CoreGraphicsDevice  *device, void  *driver_data )
{
    MSTARDriverData *sdrv = driver_data;

    GOP_InitInfo gopInitInfo;
    GOP_MuxConfig muxConfig;
    int gopCount = 4;
    u16 pnlW=0;
    u16 pnlH=0;
    hal_phy u32GOP_Regdma_addr;
    u32 u32GOP_Regdma_size;
    u32 u32GOP_Regdma_aligned;
    u32 u32GOP_Regdma_miu;
    int i = 0;
    int status = 0;
    static bool bGetChipCaps = false;
    //Get pnlW, pnlH
    pnlW = dfb_config->mst_lcd_width;
    pnlH = dfb_config->mst_lcd_height;


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
        CoreScreen *screen_mode = NULL;

        switch (dfb_config->mst_gop_dstPlane[i])
        {

            case E_GOP_DST_OP0 :
                screen_mode = sdrv->op_screen;
                break;

            case E_GOP_DST_IP0 :
                screen_mode = sdrv->ip0_screen;
                break;

            case E_GOP_DST_MIXER2VE :
                screen_mode = sdrv->ve_screen;
                break;

            case E_GOP_DST_MIXER2OP :
                screen_mode = sdrv->op_screen;
                break;

            case E_GOP_DST_FRC :
                screen_mode = sdrv->oc_screen;
                break;

            case E_GOP_DST_VE :
                screen_mode = sdrv->ve_screen;
                break;

            case E_GOP_DST_IP1 :
                screen_mode = sdrv->ip0_screen;
                break;

            case E_GOP_DST_BYPASS :
                screen_mode = sdrv->op_screen;
                break;

            //Fix me for 4K@120
            case DFB_E_GOP_DST_OP_DUAL_RATE :
                screen_mode = sdrv->op_screen;
                break;

            default:
                printf("\n[DFB] --------unsupport----------\n");
                break;
        }

        if ( screen_mode != NULL )
            sdrv->layers[i] = dfb_layers_register( screen_mode,
                                                   driver_data,
                                                   &mstarLayerFuncs );
    }
    /* for screen and layer initialize end */

    if (dfb_config->mst_null_display_driver)
        return;

    //Prepare GOP Init Params:
    gopInitInfo.u16PanelWidth = pnlW;
    gopInitInfo.u16PanelHeight= pnlH;
    gopInitInfo.u16PanelHStr = mstar_sc_get_h_cap_start();
    gopInitInfo.u32GOPRBAdr = 0;
    gopInitInfo.u32GOPRBLen = 0;
    gopInitInfo.bEnableVsyncIntFlip = TRUE;
    u32GOP_Regdma_addr = dfb_config->mst_gop_regdmaphys_addr;
    u32GOP_Regdma_size = dfb_config->mst_gop_regdmaphys_len;

    gopInitInfo.u32GOPRegdmaAdr = u32GOP_Regdma_addr;
    gopInitInfo.u32GOPRegdmaLen = u32GOP_Regdma_size;

    if( dfb_config->layer_support_palette && dfb_config->mst_gop_regdmaphys_addr != 0 && dfb_config->mst_gop_regdmaphys_len  != 0 )
    {
        void *pvirtual_address = NULL;
        unsigned long miu_offset;
        unsigned long offset;
        int miu_select = 0;

        D_INFO("[DFB] dfb_config->mst_gop_regdmaphys_addr:%x, dfb_config->mst_gop_regdmaphys_len:%x \n",
        dfb_config->mst_gop_regdmaphys_addr,
        dfb_config->mst_gop_regdmaphys_len);


        DFB_UTOPIA_TRACE(pvirtual_address = (void *)MsOS_MPool_PA2KSEG1( dfb_config->mst_gop_regdmaphys_addr ));

        if(NULL == pvirtual_address)
        {
            if ( dfb_config->mst_gop_regdmaphys_addr >= dfb_config->mst_miu2_hal_offset &&  dfb_config->mst_miu2_hal_offset != 0)
            {
                miu_offset = dfb_config->mst_miu2_hal_offset;
                miu_select = 2;
            }
            else if ( dfb_config->mst_gop_regdmaphys_addr >= dfb_config->mst_miu1_hal_offset &&  dfb_config->mst_miu1_hal_offset != 0)
            {
                miu_offset = dfb_config->mst_miu1_hal_offset;
                miu_select = 1;
            }
            else
            {
                miu_offset = dfb_config->mst_miu0_hal_offset;
                miu_select = 0;
            }

            offset = (dfb_config->mst_gop_regdmaphys_addr - miu_offset);

            D_INFO("[DFB] MsOS_MPool_Mapping offset:%x, dfb_config->mst_gop_regdmaphys_len:%x \n",
            offset,
            dfb_config->mst_gop_regdmaphys_len);

            MS_BOOL ret = 0;
            DFB_UTOPIA_TRACE(ret = MsOS_MPool_Mapping( miu_select, offset, dfb_config->mst_gop_regdmaphys_len, 1));

            if( !ret )
            {
                printf("[DFB] Waring! REGDMA map buf error! \n");
            }

        }
    }

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_CALLBACK, DF_MEASURE_START, DF_BOOT_LV7);

    //Register callback functions:
    DFB_UTOPIA_TRACE(MApi_GOP_RegisterFBFmtCB(mstar_dfb_OSD_RESOURCE_SetFBFmt));
    DFB_UTOPIA_TRACE(MApi_GOP_RegisterXCIsInterlaceCB(mstar_sc_is_interlace));
    DFB_UTOPIA_TRACE(MApi_GOP_RegisterXCGetCapHStartCB(mstar_sc_get_ip0_h_cap_start));
    DFB_UTOPIA_TRACE(MApi_GOP_RegisterXCReduceBWForOSDCB(mstar_XC_Sys_PQ_ReduceBW_ForOSD));

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_CALLBACK, DF_MEASURE_END, DF_BOOT_LV7);

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_START, DF_BOOT_LV7);
    DFB_UTOPIA_TRACE(gopCount = MApi_GOP_GWIN_GetMaxGOPNum());

    D_ASSERT( gopCount > 0 );

    //Init GOP:
    for(i = 0; i < dfb_config->mst_gop_counts; i++)
    {
        EN_GOP_IGNOREINIT eIgnor;
        bool bCheckBankForceWrite = false;

        if (dfb_get_BankForceWrite() && true == dfb_config->mst_bank_force_write)
        {
              DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(dfb_config->mst_gop_available[i], true));
              bCheckBankForceWrite = true;
        }

        if(i >= gopCount)
        {
            printf("\n[DFB] the gop %d is not existed  total have %d gop\n",i,gopCount);
            assert(0);
        }

        if (dfb_config->mst_disable_layer_init == i)
        {
            eIgnor = E_GOP_IGNORE_ALL;
            printf("[DFB] 1 Ignore layer:%d, GOP:%d\n",
            dfb_config->mst_disable_layer_init,
            dfb_config->mst_gop_available[i]);
        }
        else
        {
            eIgnor = (E_GOP_IGNORE_MUX | E_GOP_BOOTLOGO_IGNORE_VEOSDEN);
        }

        DFB_UTOPIA_TRACE(MApi_GOP_SetConfig(E_GOP_IGNOREINIT,&eIgnor));
        DFB_UTOPIA_TRACE(MApi_GOP_InitByGOP(&gopInitInfo,  dfb_config->mst_gop_available[i]));

        D_INFO("\33[1;31m[DFB][%s] MApi_GOP_InitByGOP layer_idx: %d  gop_idx: %d\33[0m\n",
        __FUNCTION__,  i, dfb_config->mst_gop_available[i]);

        if (bGetChipCaps == false)
        {
            MS_U32 ret_support_bnkforcewrite = 0;

            DFB_UTOPIA_TRACE(MApi_GOP_GetChipCaps( E_GOP_CAP_PIXELMODE_SUPPORT,
                                      &ret_support_bnkforcewrite,
                                      sizeof(MS_U32)));

            if (ret_support_bnkforcewrite)
                dfb_set_BankForceWrite(true);
            else
                dfb_set_BankForceWrite(false);

            D_INFO("\33[1;31m[DFB][%s] Return support BankForceWrite: %d \n", __FUNCTION__, ret_support_bnkforcewrite);
            bGetChipCaps = true;
        }

        if(dfb_config->mst_gop_available_r[i] != 0xf)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_InitByGOP(&gopInitInfo,  dfb_config->mst_gop_available_r[i]));
        }

        eIgnor = 0;
        DFB_UTOPIA_TRACE(MApi_GOP_SetConfig(E_GOP_IGNOREINIT, &eIgnor));

        if(bCheckBankForceWrite && true == dfb_config->mst_bank_force_write)
        {
             DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(dfb_config->mst_gop_available[i], false));
        }
    }

    int curgopIndex = -1;
    DFB_UTOPIA_TRACE( curgopIndex = MApi_GOP_GWIN_GetCurrentGOP());

    bool bBankForceWrite = dfb_get_BankForceWrite();

    if(bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(curgopIndex, true));
    }

    if(dfb_config->mst_GOP_HMirror > 0)
    {
        //DFB_CHECK_POINT("init driver : MApi_GOP_GWIN_SetHMirror(TRUE)");
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetHMirror(TRUE));
    }
    else if(dfb_config->mst_GOP_HMirror == 0)
    {
        //DFB_CHECK_POINT("init driver : MApi_GOP_GWIN_SetHMirror(FALSE)");
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetHMirror(FALSE));
    }

    if(dfb_config->mst_GOP_VMirror > 0)
    {
        //DFB_CHECK_POINT("init driver : MApi_GOP_GWIN_SetVMirror(TRUE)");
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetVMirror(TRUE));
    }
    else if(dfb_config->mst_GOP_VMirror == 0)
    {
        //DFB_CHECK_POINT("init driver : MApi_GOP_GWIN_SetVMirror(FALSE)");
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetVMirror(FALSE));
    }

    if(bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(curgopIndex, false));
    }

    DFB_BOOT_GETTIME( DF_BOOT_DRIVER_GOP_INIT, DF_MEASURE_END, DF_BOOT_LV7);

}


static void _mstar_layer_CoreLayerRegionConfigFlags_err2string (CoreLayerRegionConfigFlags fail)
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



static MS_ColorFormat
_mstarDFBFmt2MSFmt( DFBSurfacePixelFormat format )
{
    switch (format) {
        case DSPF_ARGB1555:
                return E_MS_FMT_ARGB1555;

        case DSPF_ARGB:
                return E_MS_FMT_ARGB8888;

        case DSPF_LUT8:
                return E_MS_FMT_I8;

        case DSPF_ARGB4444:
                return E_MS_FMT_ARGB4444;

        case DSPF_RGB16:
                return E_MS_FMT_RGB565;

        case DSPF_AYUV:
                return E_MS_FMT_AYUV8888;

        case DSPF_YVYU:
                return E_MS_FMT_YUV422;

        case DSPF_UYVY:
                return E_MS_FMT_YUV422;

        case DSPF_YUY2:
                return E_MS_FMT_YUV422;

        case DSPF_BLINK12355:
                return E_MS_FMT_1ABFgBg12355;

        case DSPF_BLINK2266:
                return E_MS_FMT_FaBaFgBg2266;
        default:
                return E_MS_FMT_GENERIC;
    }
}


static u8
_mstarGetMiuNumByAddr( unsigned long phy_addr )
{

    if( dfb_config->mst_miu2_hal_offset != 0 &&
        phy_addr > dfb_config->mst_miu2_hal_offset )
        //return (u8)E_GOP_SEL_MIU2;//miu2
        return (u8)2; //miu2, Fix me hardcode. (K2 not define E_GOP_SEL_MIU2)

    else if(phy_addr > dfb_config->mst_miu1_hal_offset)
        return (u8)E_GOP_SEL_MIU1; //miu1

    else
        return (u8)E_GOP_SEL_MIU0; //miu0
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


void
mstarAllSurfInfo(MSTARDriverData *sdrv)
{
    int i;

    for(i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyFB( (u8)sdrv->mstarLayerBuffer[i].u16GOPFBID));
            _mstarSetSurfInfoSlotFree(sdrv, i);
        }
    }

}


static int
_mstarGetFreeSurfInfoSlot(MSTARDriverData *sdrv)
{
    int i;

    for( i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        if(sdrv->mstarLayerBuffer[i].u16SlotUsed == 0)
            return i;
    }

    return -1;
}


/*
#define _MApi_GOP_GWIN_CreateFBFrom3rdSurf(width, height,   fbFmt, u32FbAddr, pitch, ret_FBId)  \
({   \
    printf("################# _MApi_GOP_GWIN_CreateFBFrom3rdSurf, %s, %d ,pid=%d \n",__FUNCTION__,__LINE__,getpid());   \
    MApi_GOP_GWIN_CreateFBFrom3rdSurf(width, height,   fbFmt, u32FbAddr, pitch, ret_FBId);\
})


#define _MApi_GOP_GWIN_DestroyFB(fbId)  \
({   \
    printf("################# _MApi_GOP_GWIN_DestroyFB, %s, %d ,fbid = %d \n",__FUNCTION__,__LINE__,fbId);   \
    MApi_GOP_GWIN_DestroyFB(fbId);\
})
*/

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


static void
_mstarSetSlotGWinID( MSTARDriverData    *sdrv,
                     int                 slotID,
                     u8                  u8GWinID )
{

    sdrv->mstarLayerBuffer[slotID].u16SlotUsed = 1;
    sdrv->mstarLayerBuffer[slotID].u8GWinID = u8GWinID;
}


static void
_mstarSetSurfInfoSlot( MSTARDriverData   *sdrv,
                       int                slotID,
                       u16                u16FBID,
                       CoreSurface       *surf,
                       CoreSurfaceBuffer *buffer,
                       hal_phy               physAddr,
                       unsigned long      pitch )
{

    D_ASSERT(sdrv && slotID >= 0 && slotID < MSTARGFX_MAX_LAYER_BUFFER);

    sdrv->mstarLayerBuffer[slotID].u16GOPFBID           = u16FBID;
    sdrv->mstarLayerBuffer[slotID].pCoDFBCoreSurface    = surf;
    sdrv->mstarLayerBuffer[slotID].pCoDFBBuffer         = buffer;
    sdrv->mstarLayerBuffer[slotID].physAddr             = physAddr;
    sdrv->mstarLayerBuffer[slotID].format               = surf->config.format;
    sdrv->mstarLayerBuffer[slotID].u16SlotUsed          = 1;
    sdrv->mstarLayerBuffer[slotID].pitch                = pitch;

    //printf("neohe debug set surf slot %d physical to %08x, surface %08x, buffer%08x\n", slotID, physAddr, surf, buffer);
}

static int
_mstarGetSurfInfoSlot( MSTARDriverData    *sdrv,
                       MSTARLayerData     *slay,
                       CoreSurface        *surf,
                       CoreSurfaceBuffer  *buffer,
                       hal_phy               phys,
                       unsigned long       pitch,
                       u16                *pFBId,
                       bool               *bNew )
{

    u8 u8GOP_Ret, u8FBId;
    int i = 0;

    //check whether the buffer has been assigned the bufferslot and fbid
    for(i = 0; i < MSTARGFX_MAX_LAYER_BUFFER; i++)
    {
        if( sdrv->mstarLayerBuffer[i].u16SlotUsed                   &&
            sdrv->mstarLayerBuffer[i].pCoDFBBuffer == buffer        &&
            sdrv->mstarLayerBuffer[i].pCoDFBCoreSurface == surf     &&
            sdrv->mstarLayerBuffer[i].format == surf->config.format &&
            sdrv->mstarLayerBuffer[i].pitch == pitch                &&
            sdrv->mstarLayerBuffer[i].physAddr == phys )
        {

            if(pFBId)
            {
                *pFBId = sdrv->mstarLayerBuffer[i].u16GOPFBID;
            }

            *bNew = false;
            return i;
        }

    }

    u8 curGOPMiuSel = 0;
    DFB_UTOPIA_TRACE( curGOPMiuSel = MApi_GOP_GetMIUSel(slay->gop_index));

    if(curGOPMiuSel != _mstarGetMiuNumByAddr(phys))
        DFB_UTOPIA_TRACE(MApi_GOP_MIUSel(slay->gop_index, (EN_GOP_SEL_TYPE)_mstarGetMiuNumByAddr(phys)));

    //create the new fbid and assign the new bufferslot
    i = _mstarGetFreeSurfInfoSlot(sdrv);

    D_ASSERT((i >= 0) && (i < MSTARGFX_MAX_LAYER_BUFFER));

    if (dfb_config->mst_AFBC_layer_enable & (1 << slay->layer_index))
    {
        DFB_UTOPIA_TRACE(u8FBId = MApi_GOP_GWIN_GetFreeFBID()); //MApi_GOP_GWIN_GetFree32FBID();

        DFB_UTOPIA_TRACE(u8GOP_Ret = MApi_GOP_GWIN_Create32FBbyStaticAddr2( u8FBId,
                                                           0,
                                                           0,
                                                           surf->config.size.w,
                                                           surf->config.size.h,
                                                           _mstarDFBFmt2MSFmt(surf->config.format),
                                                           phys,
                                                           dfb_config->mst_AFBC_mode));

        if(GWIN_OK != u8GOP_Ret)
        {
            printf("[DFB] Serious warning, create FB for allocation Failed(%d), free slot %d!!!\n",
                    u8GOP_Ret, i);

            _mstarSetSurfInfoSlotFree(sdrv, i);
            return -1;
        }

        DBG_LAYER_MSG("[DFB] MApi_GOP_GWIN_Create32FBbyStaticAddr2: layer_index=%d, width=%d, height=%d, fbid =%d, phys=0x%x, AFBC_mode=0x%08x\n ",
                       slay->layer_index,
                       surf->config.size.w,
                       surf->config.size.h,
                       u8FBId,
                       phys,
                       dfb_config->mst_AFBC_mode);

    }
    else
    {
        DFB_UTOPIA_TRACE(u8GOP_Ret = MApi_GOP_GWIN_CreateFBFrom3rdSurf( surf->config.size.w,
                                                       surf->config.size.h,
                                                       _mstarDFBFmt2MSFmt(surf->config.format),
                                                       phys,
                                                       pitch,
                                                       &u8FBId ));

        if(GWIN_OK != u8GOP_Ret)
        {
            printf("[DFB] Serious warning, create FB for allocation Failed(%d), free slot %d!!!\n",
                    u8GOP_Ret, i);

            _mstarSetSurfInfoSlotFree(sdrv, i);
            return -1;
        }

        DBG_LAYER_MSG("[DFB] MApi_GOP_GWIN_CreateFBFrom3rdSurf: width=%d, height=%d, pitch=%d, phys=0x%x, fbid =%d\n ",
                       surf->config.size.w, surf->config.size.h,pitch ,phys, u8FBId );

     }

     _mstarSetSurfInfoSlot(sdrv, i,  u8FBId, surf, buffer, phys,pitch);

    if(pFBId)
    {
        *pFBId = sdrv->mstarLayerBuffer[i].u16GOPFBID;
    }

    *bNew = true;

    return i;

}


static int
gfx_GOPStretch( MS_U8 gop,
                MS_U8 gop_dest,
                MS_U16 srcWidth,
                MS_U16 srcHeight,
                MS_U16 dstWidth,
                MS_U16 dstHeight,
                MS_U16 start_x,
                MS_U16 start_y )
{

    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(gop));

    /* GOP stretch setting */
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_HSCALE(TRUE, srcWidth, dstWidth));
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_VSCALE(TRUE, srcHeight, dstHeight));
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_STRETCHWIN(gop, gop_dest, start_x, start_y, srcWidth, srcHeight));

    DBG_LAYER_MSG("[DFB] %s(%d):srcWidth=%d  srcHeight=%d, dstWidth=%d  dstHeight=%d, start_x=%d  start_y=%d, gop:%d \n", __FUNCTION__, __LINE__,
        srcWidth, srcHeight, dstWidth, dstHeight, start_x, start_y, gop);

    if (gop_dest == E_GOP_DST_OP0               ||
        gop_dest == DFB_E_GOP_DST_OP_DUAL_RATE) //Fix me for 4K@120
        DFB_UTOPIA_TRACE(MApi_GOP_SetGOPHStart(gop, mstar_sc_get_h_cap_start()));

    return 0;
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

static u8
getLayerByGOPindex( u8 gopindex )
{
    u8 i ;
    for (i = 0; i < dfb_config->mst_goplayer_counts; i ++)
        if(dfb_config->mst_goplayer[i] == gopindex)
            return i;

    return 0xff;  //Invalid
}


static DFBResult
mstarInitLayer_fake( CoreLayer                       *layer,
                void                            *driver_data,
                void                            *layer_data,
                DFBDisplayLayerDescription      *description,
                DFBDisplayLayerConfig           *config,
                DFBColorAdjustment              *adjustment )
{
        MSTARDriverData *sdrv = driver_data;
        MSTARDeviceData *sdev = sdrv->dev;
        MSTARLayerData  *data = layer_data;
        u8 i;
        u8  curGopIndex;
        CoreDFB *core = layer->core;

        D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

        /* initialize layer data */

        for( i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
        {
            char  buf[24];

            if(layer == sdrv->layers[i])
            {
                data->layer_index = i;


                //the gopindex should be required form the dfbconfig
                data->gop_index                 = dfb_config->mst_gop_available[i];
                data->gop_index_r               = dfb_config->mst_gop_available_r[i];
                data->gop_dst                   = dfb_config->mst_gop_dstPlane[i];
                data->layer_displayMode         = DLDM_NORMAL;
                data->GopDstMode                = dfb_config->mst_gop_dstPlane[i];

                sdev->layer_zorder[i]           = dfb_config->mst_layer_gwin_level;

                sdev->layer_active[i]           = false;
                sdev->layer_gwin_id[i]          = INVALID_WIN_ID;
                sdev->layer_gwin_id_r[i]        = INVALID_WIN_ID;

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

                break;
            }

        }

        D_ASSERT( i < MSTAR_MAX_OUTPUT_LAYER_COUNT );

        D_DEBUG_AT(MSTAR_Layer, "%s: data->layer: %d\n", __FUNCTION__, data->layer_index);

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
        snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Input %d", sdrv->num_inputs );


        /* fill out the default configuration */
        config->flags       = DLCONF_WIDTH          |
                              DLCONF_HEIGHT         |
                              DLCONF_PIXELFORMAT    |
                              DLCONF_BUFFERMODE     |
                              DLCONF_OPTIONS;

    /*
        config->width       = MS_LAYER_DEFAULT_WIDTH;//MSTAR MAX GFX WIDTH;
        config->height      = MS_LAYER_DEFAULT_HEIGHT;//MSTAR MAX GFX HEIGHT;
    */
        config->pixelformat = DSPF_ARGB;
        config->buffermode  = DLBM_BACKVIDEO;
        config->options     = DLOP_ALPHACHANNEL;
        config->btile_mode  = false;



        //Default use the the 64*64 resolution
        config->options     = DLOP_ALPHACHANNEL;
        config->width       = 64;
        config->height      = 64;

        data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
        data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
        data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
        data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;



        int default_w, default_h;
        default_w = dfb_config->mst_layer_default_width;
        default_h = dfb_config->mst_layer_default_height;

        if( config->width > default_w && default_w > 0 )
            config->width = default_w;

        if( config->height > default_h && default_h > 0)
            config->height = default_h;


        adjustment->flags = DCAF_BRIGHTNESS | DCAF_CONTRAST;
        adjustment->contrast=50;   //50 means no change for contrast,just for initialization

        DFB_CHECK_POINT("mstarInitLayer done");

        return DFB_OK;
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
    u8 i;
    u8  curGopIndex;
    CoreDFB *core = layer->core;

    if(dfb_config->mst_null_display_driver)
        return mstarInitLayer_fake(layer, driver_data, layer_data, description, config, adjustment);


    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    /* initialize layer data */

    for( i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
    {
        char  buf[24];

        if(layer == sdrv->layers[i])
        {
            data->layer_index = i;


            //the gopindex should be required form the dfbconfig
            data->gop_index                 = dfb_config->mst_gop_available[i];
            data->gop_index_r               = dfb_config->mst_gop_available_r[i];
            data->gop_dst                   = dfb_config->mst_gop_dstPlane[i];
            data->layer_displayMode         = DLDM_NORMAL;
            data->GopDstMode                = dfb_config->mst_gop_dstPlane[i];

            sdev->layer_zorder[i]           = dfb_config->mst_layer_gwin_level;

            sdev->layer_active[i]           = false;
            sdev->layer_gwin_id[i]          = INVALID_WIN_ID;
            sdev->layer_gwin_id_r[i]        = INVALID_WIN_ID;

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

            break;
        }

    }

    D_ASSERT( i < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    D_DEBUG_AT(MSTAR_Layer, "%s: data->layer: %d\n", __FUNCTION__, data->layer_index);

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
    snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Input %d", sdrv->num_inputs );


    /* fill out the default configuration */
    config->flags       = DLCONF_WIDTH          |
                          DLCONF_HEIGHT         |
                          DLCONF_PIXELFORMAT    |
                          DLCONF_BUFFERMODE     |
                          DLCONF_OPTIONS;

/*
    config->width       = MS_LAYER_DEFAULT_WIDTH;//MSTAR MAX GFX WIDTH;
    config->height      = MS_LAYER_DEFAULT_HEIGHT;//MSTAR MAX GFX HEIGHT;
*/
    config->pixelformat = DSPF_ARGB;
    config->buffermode  = DLBM_BACKVIDEO;
    config->options     = DLOP_ALPHACHANNEL;
    config->btile_mode  = false;

    DFB_UTOPIA_TRACE(curGopIndex = MApi_GOP_GWIN_GetCurrentGOP());

    if(curGopIndex != data->gop_index)
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(data->gop_index));


    switch (data->gop_dst)
    {
        case E_GOP_DST_OP0:
        case E_GOP_DST_MIXER2OP:
        case E_GOP_DST_BYPASS:
        case DFB_E_GOP_DST_OP_DUAL_RATE:  //Fix me for 4K@120
            {
                MS_PNL_DST_DispInfo dstDispInfo;
                DFB_UTOPIA_TRACE(MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo)));

                if(dstDispInfo.bYUVOutput || dfb_config->mst_GOP_Set_YUV)
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_YUV));
                }
                else
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_RGB));
                }

                if(data->gop_index_r!=0xf && data->gop_dst == E_GOP_DST_BYPASS) //switch gop_r
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(data->gop_index_r));

                    if(dstDispInfo.bYUVOutput || dfb_config->mst_GOP_Set_YUV)
                    {
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_YUV));
                    }
                    else
                    {
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_RGB));
                    }

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(data->gop_index));
                }

                config->width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                config->height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
                data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
                data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
                data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;

                if(sdev->gfx_max_width <= config->width)
                {
                    config->width = sdev->gfx_max_width;
                }

                if(sdev->gfx_max_height <= config->height)
                {
                    config->height = sdev->gfx_max_height;
                }
            }
                break;

        case E_GOP_DST_MIXER2VE:
        case E_GOP_DST_VE:
            {
               //printf("\nbefore the MApi_VE_GetDstInfo\n");
               MS_VE_DST_DispInfo dstDispInfo;

               if (MApi_VE_GetDstInfo != NULL)
                    DFB_UTOPIA_TRACE(MApi_VE_GetDstInfo(&dstDispInfo,sizeof(MS_VE_DST_DispInfo)));

               config->width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
               config->height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

               data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
               data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
               data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = dstDispInfo.DEHST;
               data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = dstDispInfo.DEVST;
            }
                break;


        case E_GOP_DST_IP0 :
            {
                //Default use the the 64*64 resolution
                config->options     = DLOP_ALPHACHANNEL;
                config->width       = 64;
                config->height      = 64;

                data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
                data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
                data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
                data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;
            }
                break;

        case E_GOP_DST_FRC :
            {
                MS_OSDC_DST_DispInfo dstDispInfo;
                memset(&dstDispInfo, 0, sizeof(MS_OSDC_DST_DispInfo));

                dstDispInfo.ODSC_DISPInfo_Version = ODSC_DISPINFO_VERSIN;
                DFB_UTOPIA_TRACE(MApi_XC_OSDC_GetDstInfo(&dstDispInfo, sizeof(MS_OSDC_DST_DispInfo)));

                config->width   = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                config->height  = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
                data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
                data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
                data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;
            }
                break;

        case E_GOP_DST_IP1 :
            {
                //Default use the the 64*64 resolution
                config->options     = DLOP_ALPHACHANNEL;
                config->width       = 64;
                config->height      = 64;

                data->screen_size.width  = sdev->layer_screen_size[data->layer_index].width  = config->width;
                data->screen_size.height = sdev->layer_screen_size[data->layer_index].height = config->height;
                data->screen_size.Hstart = sdev->layer_screen_size[data->layer_index].Hstart = 0;
                data->screen_size.Vstart = sdev->layer_screen_size[data->layer_index].Vstart = 0;
            }
                break;

        default :
            printf("[DFB][Error]GOP destination displayplane type don't support!!!\n");
                break;
    }

    int default_w, default_h;
    default_w = dfb_config->mst_layer_default_width;
    default_h = dfb_config->mst_layer_default_height;

    if( config->width > default_w && default_w > 0 )
        config->width = default_w;

    if( config->height > default_h && default_h > 0)
        config->height = default_h;


    adjustment->flags = DCAF_BRIGHTNESS | DCAF_CONTRAST;
    adjustment->contrast=50;   //50 means no change for contrast,just for initialization

    DFB_CHECK_POINT("mstarInitLayer done");

    return DFB_OK;

}


static DFBResult
mstarReorderGWIN(MSTARDriverData    *sdrv)
{
     return DFB_OK;
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
    bool bBankForceWrite = dfb_get_BankForceWrite();
    DBG_LAYER_MSG("[DFB] %s --> %d, GOPINDEX:%d GOPDST:%d\n", __FUNCTION__, __LINE__ , slay->gop_index, slay->gop_dst);

    // Ip path special ,because OP ve path should can get the valid value in mstarInitlayer

    DFB_UTOPIA_TRACE(curGopIndex = MApi_GOP_GWIN_GetCurrentGOP());

    if(curGopIndex != slay->gop_index)
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DBG_LAYER_MSG("[DFB] %s(%d): MApi_GOP_GWIN_SetBnkForceWrite enabled!!!\n", __FUNCTION__, __LINE__);
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(slay->gop_index, true));
    }

    switch(slay->gop_dst)
    {
        case E_GOP_DST_IP0:
        case E_GOP_DST_IP1:
            {
                MS_XC_DST_DispInfo dstDispInfo;
                u32 width = 0;
                u32 height = 0;
                MS_BOOL bRet = FALSE;

                DFB_UTOPIA_TRACE(bRet = MApi_XC_GetDstInfo(&dstDispInfo, sizeof(MS_XC_DST_DispInfo), E_GOP_XCDST_IP1_MAIN));

                if (dfb_config->mst_do_xc_ip1_patch == true) //XC IP1 is not be support in STB, Enable patch to fix it.
                {
                    if (MApi_XC_GetDynamicScalingStatus()) // DS Enable, use original case
                    {
                        if(bRet)
                        {
                            width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                            height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                            slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width  = width;
                            slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = height;
                            slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart = dstDispInfo.DEHST;
                            slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart = dstDispInfo.DEVST;
                        }
                    }
                    else // DS disable, use MApi_XC_GetStatusEx to get info
                    {
                        XC_ApiStatusEx stXCStatusEx;
                        memset(&stXCStatusEx, 0, sizeof(XC_ApiStatusEx));
                        stXCStatusEx.u16ApiStatusEX_Length = sizeof(XC_ApiStatusEx);
                        stXCStatusEx.u32ApiStatusEx_Version = API_STATUS_EX_VERSION;
                        if(MApi_XC_GetStatusEx(&stXCStatusEx, MAIN_WINDOW) == sizeof(XC_ApiStatusEx))
                        {
                            slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width  = stXCStatusEx.stCapWin.width;
                            slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = stXCStatusEx.stCapWin.height;
                            slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart = stXCStatusEx.stCapWin.x;
                            slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart = stXCStatusEx.stCapWin.y;
                            printf("[DFB] %s : MApi_XC_GetStatusEx: x = %d, y = %d width = %d, height = %d\n", __FUNCTION__,
                                   stXCStatusEx.stCapWin.x,  stXCStatusEx.stCapWin.y, stXCStatusEx.stCapWin.width,  stXCStatusEx.stCapWin.height );
                        }
                    }
                }
                else
                {
                    if(bRet)
                    {
                        width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                        height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                        slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width  = width;
                        slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = height;
                        slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart = dstDispInfo.DEHST;
                        slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart = dstDispInfo.DEVST;
                    }
                }

                if(bRet)
                {
                    if(dstDispInfo.bYUVInput)
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_YUV));
                    else
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_RGB));
                }
            }

            break;

        case E_GOP_DST_MIXER2VE:
        case E_GOP_DST_VE:
            {

                //printf("\nbefore the MApi_VE_GetDstInfo\n");
                MS_VE_DST_DispInfo dstDispInfo;
                u32 width = 0;
                u32 height = 0;

               if (MApi_VE_GetDstInfo != NULL)
                    DFB_UTOPIA_TRACE(MApi_VE_GetDstInfo(&dstDispInfo,sizeof(MS_VE_DST_DispInfo)));

                width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width  = width;
                slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height = height;
                slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart = dstDispInfo.DEHST;
                slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart = dstDispInfo.DEVST;

                if( config->width > slay->screen_size.width )
                {
                    printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->width=%d > screen_size.width=%d %s(%d)\n", slay->gop_dst, config->width, slay->screen_size.width, __FUNCTION__, __LINE__);
                    printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_VE_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                    config->width = slay->screen_size.width;
                }

                if( config->height > slay->screen_size.height )
                {
                    printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->height=%d > screen_size.height=%d %s(%d)\n", slay->gop_dst, config->height, slay->screen_size.height, __FUNCTION__, __LINE__);
                    printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_VE_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                    config->height = slay->screen_size.height;
                }
            }

            break;

        case E_GOP_DST_OP0:
        case E_GOP_DST_BYPASS:
        case DFB_E_GOP_DST_OP_DUAL_RATE:      //Fix me for 4K@120
            {
                MS_PNL_DST_DispInfo dstDispInfo;
                DFB_UTOPIA_TRACE(MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo)));

                int width = 0;
                int height = 0;

                width  = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                height = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                if (dfb_config->mst_gop_interlace_adjust && dstDispInfo.bInterlaceMode)
                {
                    height <<= 1;  //for mantis:1226868
                    DBG_LAYER_MSG("[DFB] %s : MApi_PNL_GetDstInfo width = %d, height = %d\n", __FUNCTION__, width, height);
                }

                slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width = width;
                slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height=height;
                slay->screen_size.Hstart = sdev->layer_screen_size[slay->layer_index].Hstart= dstDispInfo.DEHST;
                slay->screen_size.Vstart = sdev->layer_screen_size[slay->layer_index].Vstart= dstDispInfo.DEVST;

                if (dfb_config->mst_enable_layer_autoscaledown == false)
                {
                    if( config->width > slay->screen_size.width )
                    {
                        printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->width=%d > screen_size.width=%d %s(%d)\n", slay->gop_dst, config->width, slay->screen_size.width, __FUNCTION__, __LINE__);
                        printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_PNL_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                        config->width = slay->screen_size.width;
                    }

                    if( config->height > slay->screen_size.height )
                    {
                        printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->height=%d > screen_size.height=%d %s(%d)\n", slay->gop_dst, config->height, slay->screen_size.height, __FUNCTION__, __LINE__);
                        printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_PNL_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                        config->height = slay->screen_size.height;
                    }
                }

                if(dstDispInfo.bYUVOutput || dfb_config->mst_GOP_Set_YUV)
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_YUV));
                else
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_RGB));

                if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));

                    if(dstDispInfo.bYUVOutput || dfb_config->mst_GOP_Set_YUV)
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_YUV));
                    else
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_OutputColor(GOPOUT_RGB));


                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
                }
            }
            break;

        case E_GOP_DST_FRC:
            {
                int width = 0;
                int height = 0;

                MS_OSDC_DST_DispInfo dstDispInfo;
                memset(&dstDispInfo,0,sizeof(MS_OSDC_DST_DispInfo));

                dstDispInfo.ODSC_DISPInfo_Version = ODSC_DISPINFO_VERSIN;
                DFB_UTOPIA_TRACE(MApi_XC_OSDC_GetDstInfo(&dstDispInfo,sizeof(MS_OSDC_DST_DispInfo)));

                width   = dstDispInfo.DEHEND - dstDispInfo.DEHST + 1;
                height  = dstDispInfo.DEVEND - dstDispInfo.DEVST + 1;

                slay->screen_size.width  = sdev->layer_screen_size[slay->layer_index].width = width;
                slay->screen_size.height = sdev->layer_screen_size[slay->layer_index].height=height;

                if( config->width > slay->screen_size.width )
                {
                    printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->width=%d > screen_size.width=%d %s(%d)\n", slay->gop_dst, config->width, slay->screen_size.width, __FUNCTION__, __LINE__);
                    printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_XC_OSDC_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                    config->width = slay->screen_size.width;
                }

                if( config->height > slay->screen_size.height )
                {
                    printf("\33[0;33;44m[DFB]\33[0m warning! slay->gop_dst=0x%x, config->height=%d > screen_size.height=%d %s(%d)\n", slay->gop_dst, config->height, slay->screen_size.height, __FUNCTION__, __LINE__);
                    printf("\33[0;33;44m[DFB]\33[0m warning! got screen_size by MApi_XC_OSDC_GetDstInfo %s(%d)\n", __FUNCTION__, __LINE__);
                    config->height = slay->screen_size.height;
                }

            }
            break;

        default:
            printf("[DFB][Error]GOP destination displayplane type don't support!!!\n");
            break;
    }

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(slay->gop_index, false));
    }

    if(curGopIndex != slay->gop_index)
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(curGopIndex));


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
        case DSPF_ARGB1555:
        case DSPF_ARGB4444:
        case DSPF_RGB16:

        case DSPF_YVYU:
        case DSPF_AYUV:
        case DSPF_UYVY:
        case DSPF_YUY2:
        case DSPF_BLINK12355:
        case DSPF_BLINK2266:
                break;
        default:
                fail |= CLRCF_FORMAT;
    }

    DBG_LAYER_MSG("[DFB] slayer->gop_dst:0x%x\n",slay->gop_dst);
    DBG_LAYER_MSG("[DFB] %s (%d) config->width=%d config->height=%d\n", __FUNCTION__, __LINE__, config->width, config->height);
    DBG_LAYER_MSG("[DFB] %s (%d) screen_size.width=%d screen_size.height=%d\n", __FUNCTION__, __LINE__, slay->screen_size.width, slay->screen_size.height );

    DBG_LAYER_MSG("[DFB] %s (%d) config->dest.x: %d config->dest.y=%d\n", __FUNCTION__, __LINE__, config->dest.x, config->dest.y);
    DBG_LAYER_MSG("[DFB] %s (%d) config->dest.w: %d config->dest.h=%d\n", __FUNCTION__, __LINE__, config->dest.w, config->dest.h);


    if (dfb_config->mst_enable_layer_autoscaledown == false)
    {
        if ( config->width < 32     ||
             config->width > slay->screen_size.width )
              fail |= CLRCF_WIDTH;

        if ( config->height < 32    ||
             config->height > slay->screen_size.height )
            fail |= CLRCF_HEIGHT;

        if( slay->gop_dst == E_GOP_DST_OP0  ||
            slay->gop_dst == DFB_E_GOP_DST_OP_DUAL_RATE) //Fix me for 4K@120
        {
            if (config->dest.x >= slay->screen_size.width || config->dest.y >= slay->screen_size.height ||
                config->dest.x+config->dest.w > slay->screen_size.width || config->dest.y+config->dest.h > slay->screen_size.height)
            {
                DBG_LAYER_MSG("[DFB] Warning the dimension maybe can't support by GOP, Please check it \n");
            }
        }
        else
        {
#define CHECK_FAIL(COND, OPT, FLAG)\
        do{\
            if(COND)\
            {\
                DBG_LAYER_MSG("[DFB] "#COND"\n");\
                FLAG |= OPT;\
            }\
        }while(0)

            CHECK_FAIL(config->dest.x >= slay->screen_size.width || config->dest.y >= slay->screen_size.height, CLRCF_DEST, fail);

            CHECK_FAIL(config->dest.x+config->dest.w > slay->screen_size.width || config->dest.y+config->dest.h > slay->screen_size.height, CLRCF_DEST, fail);

            DBG_LAYER_MSG("[DFB] %s (%d) sdev->screen_size.w: %d, sdev->screen_size.h: %d\n", __FUNCTION__, __LINE__,slay->screen_size.width, slay->screen_size.height);
        }
    }

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

        _mstar_layer_CoreLayerRegionConfigFlags_err2string(fail);

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

    if( (updated & CLRCF_COLORSPACE)      &&
        sreg->config.btile_mode != config->btile_mode )
    {
        retUpdated |= CLRCF_COLORSPACE;
        sreg->config.btile_mode = config->btile_mode;
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

    if( updated & CLRCF_PALETTE )
    {
        if( config->format == DSPF_LUT8         ||
            config->format == DSPF_BLINK12355   ||
            config->format == DSPF_BLINK2266

            )
        {
            retUpdated |= CLRCF_PALETTE;
        }
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

    u8  u8FBId = 0;
    u8  curGopIndex = 0;
    u16 u16FBId = 0;  //Coverity-03132013
    hal_phy u32Phys = 0;
    u8  constAlpha = 0;

    int freeSurfInfoSlot = -1;
    bool bGWinEnable = true;

    EN_GOP_CONSALPHA_BITS enConstAlpha_bits = E_GOP_VALID_6BITS;
    EN_GOP_CONFIG_TYPE confugType = E_GOP_CONSALPHA_BITS ;

    bool bnewFB = false;
    bool bvideoModeChanged = false;
    bool bGOPModeChanged = false;
    int bpp = DFB_BYTES_PER_PIXEL(sreg->config.format);
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    fusion_skirmish_prevail( &sdev->beu_lock );


    DFB_UTOPIA_TRACE(curGopIndex = MApi_GOP_GWIN_GetCurrentGOP());

    if( curGopIndex != slay->gop_index )
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));


    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnce(TRUE));
    //MApi_GOP_GWIN_Enable(sdev->layer_gwin_id[slay->layer_index], (MS_BOOL)FALSE) ;
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
    //MApi_GOP_GWIN_UpdateRegOnceEx(FALSE, FALSE);
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnce(TRUE));

    sreg->config_dirtyFlag |= mstarBuildUpdate(sreg, config, updated);

    if( (slay->gop_dst == E_GOP_DST_OP0)    &&
        (dfb_config->mst_margine_top        ||     \
         dfb_config->mst_margine_bottom     ||
         dfb_config->mst_margine_left       ||    \
         dfb_config->mst_margine_wright) )
    {
        sreg->config.dest.x += dfb_config->mst_margine_left;
        sreg->config.dest.w -= dfb_config->mst_margine_wright;
        sreg->config.dest.y += dfb_config->mst_margine_top;
        sreg->config.dest.h -= dfb_config->mst_margine_bottom;
    }

    if(dfb_config->bGOPUsingHWTLB)
        u32Phys = lock->phys;
    else
        u32Phys = _BusAddrToHalAddr(((u64)lock->phys_h << 32) | lock->phys);

#if !USE_SIZE_OPTIMIZATION
    if ( true == dfb_config->mst_GE_performanceTest )
    {
        int width = 256;
        int height = 256;
        int pixelformat = 4;
        hal_phy u32SrcAddr = u32Phys;
        hal_phy u32DstAddr = u32Phys + width*height*pixelformat;

        printf("\33[0;33m Test GE Blit \33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, true, false, 0);

        printf("\33[0;33m Test GE Blit with rotate 90\33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, true, false, GEROTATE_90);

        printf("\33[0;33m Test GE Blit with rotate 180\33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, true, false, GEROTATE_180);

        printf("\33[0;33m Test GE Blit with rotate 270\33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, true, false, GEROTATE_270);

        printf("\33[0;33m Test GE Blit (Blend) \33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, true, true, 0);

        printf("\33[0;33m Test GE Fill Rectangle \33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, false, false, 0);

        printf("\33[0;33m Test GE Fill Rectangle (blend) \33[0m \n");
        GE_API_Performance(u32SrcAddr, u32DstAddr, width, height, width, height, false, true, 0);

        printf("\33[0;33m Test Done. Please remove test_ge in the directfbrc and restart \33[0m \n");

        while(true);
    }
#endif

    if( slay->gop_dst == E_GOP_DST_IP0  ||
        slay->gop_dst == E_GOP_DST_IP1 )
    {
        E_VIDEO_FIELD_MODE video_mode = E_VIDEO_FIELD_PROGRESSIVE;

        if( mstar_sc_is_interlace() )
        {
            video_mode = E_VIDEO_FIELD_INTERLACE ;
        }

        if(video_mode!=slay->VideoFiledMode)
        {
            slay->VideoFiledMode = video_mode ;
            bvideoModeChanged = true ;
        }

    }
    else if ( slay->gop_dst == E_GOP_DST_BYPASS     ||
              slay->gop_dst == E_GOP_DST_OP0        ||
              slay->gop_dst == DFB_E_GOP_DST_OP_DUAL_RATE) //Fix me for 4K@120
    {
        if( slay->gop_dst != slay->GopDstMode )
        {
            bGOPModeChanged = true ;
            slay->GopDstMode = slay->gop_dst ;
            sreg->config_dirtyFlag |= CLRCF_OPACITY; //set GOP Alpha
        }

        //fixbug in mantis: 627551: movie: subtitle display abnormally
        if ( slay->gop_dst ==  E_GOP_DST_OP0        ||
             slay->gop_dst ==  DFB_E_GOP_DST_OP_DUAL_RATE) //Fix me for 4K@120
        {
            MS_PNL_DST_DispInfo dstDispInfo;
            MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));
            if (dfb_config->mst_gop_interlace_adjust && dstDispInfo.bInterlaceMode)
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableProgressive(FALSE));//for mantis:1226868
                DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_EnableProgressive: FALSE\n");
            }
            else
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableProgressive(TRUE));
                DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_EnableProgressive: TRUE\n");
            }
        }
    }
    else if ( dfb_config->mst_enable_ve_init )
    {
        if ( slay->gop_dst == E_GOP_DST_VE )  //For K3s Set the gopVETimingType
        {
            GOP_VE_TIMINGTYPE gopVETimingType;

            MS_VE_VIDEOSYS enVesys;
            if (MDrv_VE_Get_Output_Video_Std != NULL)
                enVesys = MDrv_VE_Get_Output_Video_Std();

            if(enVesys == MS_VE_NTSC)
            {
                gopVETimingType = GOP_VE_NTSC;
            }
            else
            {
                gopVETimingType = GOP_VE_PAL;
            }
            if (MApi_GOP_VE_SetOutputTiming != NULL)
                 DFB_UTOPIA_TRACE(MApi_GOP_VE_SetOutputTiming(gopVETimingType));

        }
        else if ( slay->gop_dst == E_GOP_DST_MIXER2VE )  //For K3s Set the gopMIXER2VETimingType
        {
            GOP_MixerTiming stTiming;

            MS_VE_VIDEOSYS enVesys;
            if (MDrv_VE_Get_Output_Video_Std != NULL)
                enVesys =  MDrv_VE_Get_Output_Video_Std();

            if( enVesys == MS_VE_NTSC )
            {
                DFB_UTOPIA_TRACE(MApi_GOP_MIXER_SetOutputTiming(GOP_NTSC, &stTiming));
            }
            else
            {
                DFB_UTOPIA_TRACE(MApi_GOP_MIXER_SetOutputTiming(GOP_PAL, &stTiming));
            }
        }
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
        printf("\nthe layer is configure as shadow layer or this layer has other shadowlayer,refuse setRegion\n");
        fusion_skirmish_dismiss( &sdev->beu_lock );
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnce(FALSE));

        return DFB_FAILURE;
    }

    if( sreg->config_dirtyFlag & (CLRCF_SURFACE|CLRCF_WIDTH|CLRCF_HEIGHT|CLRCF_FORMAT|CLRCF_PALETTE)||
        bvideoModeChanged   ||
        bGOPModeChanged     ||
        config->buffermode == DLBM_FRONTONLY   ||
        sdev->layer_gwin_id[slay->layer_index] == INVALID_WIN_ID )
    {

        if( INVALID_WIN_ID != sdev->layer_gwin_id[slay->layer_index] )
        {
            u16 FBId,slotID;
            DBG_LAYER_MSG ("[DFB]  MApi_GOP_GWIN_DestroyWin: sdev->layer_gwin_id[slay->layer_index]= %d\n", sdev->layer_gwin_id[slay->layer_index]);
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id[slay->layer_index]));

            if( slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
            {
                if(INVALID_WIN_ID != sdev->layer_gwin_id_r[slay->layer_index])
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id_r[slay->layer_index]));
            }

            do
            {
                FBId = _mstarFindFBIdbyGWinID( sdrv,
                                               sdev->layer_gwin_id[slay->layer_index],
                                               &slotID,
                                               &surface);

                if( FBId != 0xff )
                {
                    DBG_LAYER_MSG ("[DFB]  MApi_GOP_GWIN_DestroyFB: FBId= %d\n", FBId);
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyFB(FBId));
                    _mstarSetSurfInfoSlotFree(sdrv,slotID);
                }
            
            } while( FBId != 0xff );


            sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
        }

        freeSurfInfoSlot = _mstarGetSurfInfoSlot( sdrv,
                                                  slay,
                                                  surface,
                                                  lock->buffer,
                                                  u32Phys,
                                                  lock->pitch,
                                                  &u16FBId,
                                                  &bnewFB);

        if (freeSurfInfoSlot < 0)
        {
            printf("\n%s %s (%d) : Cannot find valid freeSurfInfoSlot (%d)\n", __FILE__, __FUNCTION__, __LINE__, freeSurfInfoSlot);
            fusion_skirmish_dismiss( &sdev->beu_lock );
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnce(FALSE));
            return DFB_FAILURE;
        }

        DFB_UTOPIA_TRACE(MApi_GOP_SetUVSwap(slay->gop_index, FALSE));
        DFB_UTOPIA_TRACE(MApi_GOP_SetYCSwap(slay->gop_index, FALSE));

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
            case DSPF_ARGB4444:
            case DSPF_BLINK12355:
            case DSPF_BLINK2266:

                if(config->format == DSPF_YUY2)
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_SetUVSwap(slay->gop_index, TRUE));
                }
                else if (config->format == DSPF_UYVY)
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_SetYCSwap(slay->gop_index, TRUE));
                }

                DFB_UTOPIA_TRACE(sdev->layer_gwin_id[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index, (u8)u16FBId, 0, 0));

                DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: sdev->layer_gwin_id[slay->layer_index(%d)] =  %d\n", slay->layer_index, sdev->layer_gwin_id[slay->layer_index]);
                DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: slay->gop_index = %d\n", slay->gop_index);
                DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: u16FBId = %d\n", u16FBId);

                _mstarSetSlotGWinID( sdrv,
                                     freeSurfInfoSlot,
                                     sdev->layer_gwin_id[slay->layer_index]);

                if( slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
                {
                    GOP_GwinInfo info;

                   //gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width/2,sreg->config.height, sreg->config.dest.w/2,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
                    gfx_GOPStretch( slay->gop_index,
                                    slay->gop_dst, sreg->config.width/2,
                                    sreg->config.height,
                                    sreg->config.dest.w/2,
                                    sreg->config.dest.h,
                                    0,
                                    0 );

                    memset(&info, 0, sizeof(GOP_GwinInfo));

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

                    info.u16DispHPixelEnd = sreg->config.width/2;
                    info.u16RBlkHPixSize = info.u16DispHPixelEnd - info.u16DispHPixelStart;

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

                }
                else
                {
                    gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width,sreg->config.height, sreg->config.dest.w,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
                }

                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index], 0, 0));
                DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id[slay->layer_index], FALSE, GOP_PINPON_G3D));

                if( slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
                {
                    GOP_GwinInfo info;

                    DFB_UTOPIA_TRACE(sdev->layer_gwin_id_r[slay->layer_index] = MApi_GOP_GWIN_CreateWin_Assign_FB(slay->gop_index_r, (u8)u16FBId, 0, 0));

                    DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: sdev->layer_gwin_id_r[slay->layer_index(%d)] =  %d\n", slay->layer_index, sdev->layer_gwin_id_r[slay->layer_index]);
                    DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: slay->gop_index_r = %d\n", slay->gop_index_r);
                    DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_CreateWin_Assign_FB: u16FBId = %d\n", u16FBId);


                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                     
                    //gfx_GOPStretch(slay->gop_index_r,slay->gop_dst, sreg->config.width/2,sreg->config.height, sreg->config.dest.w/2,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
                    gfx_GOPStretch( slay->gop_index_r,
                                    slay->gop_dst,
                                    sreg->config.width/2,
                                    sreg->config.height,
                                    sreg->config.dest.w/2,
                                    sreg->config.dest.h,
                                    0,
                                    0);

                    memset(&info, 0, sizeof(GOP_GwinInfo));

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id_r[slay->layer_index],&info));

                    info.u16DispHPixelEnd = sreg->config.width/2;
                    info.u16RBlkHPixSize = info.u16DispHPixelEnd - info.u16DispHPixelStart;

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id_r[slay->layer_index],&info));

                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id_r[slay->layer_index], 0, 0));
                    DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], FALSE, GOP_PINPON_G3D));
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
                }

                sreg->config_dirtyFlag &= ~CLRCF_DEST;

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
        GOP_GwinInfo info;
        u32 u32tmpPhys = 0;
        memset(&info, 0, sizeof(GOP_GwinInfo));

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info));
        // For source rectangle implement.
        x1 = ALIGN(sreg->config.source.x, 4);
        y1 = ALIGN(sreg->config.source.y, 4);
        u32tmpPhys = u32Phys + (y1 * lock->pitch + x1 * bpp);
        info.u32DRAMRBlkStart = u32tmpPhys;
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info));
    }

    if( INVALID_WIN_ID == sdev->layer_gwin_id[slay->layer_index]    ||
        GWIN_FAIL == sdev->layer_gwin_id[slay->layer_index] )
    {
          sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
          fusion_skirmish_dismiss( &sdev->beu_lock );
          DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
          DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnce(FALSE));
          
          return DFB_FAILURE;
    }

    if( sreg->config_dirtyFlag & (CLRCF_SRCKEY|CLRCF_OPTIONS) )
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
                    if(dfb_config->mst_argb1555_display)
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], TRUE, 0));
                    else
                    {
                            sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> 5);

                            sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> 5);

                            sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> 5);
                    }
                }
                    break;

                case DSPF_RGB16:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> 5);

                    sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> 6);

                    sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> 5);
                }
                    break;

                case DSPF_ARGB:
                    break;

                case DSPF_ARGB4444:
                {
                    sreg->config.src_key.r = (sreg->config.src_key.r)       |
                                             (sreg->config.src_key.r >> 4);

                    sreg->config.src_key.g = (sreg->config.src_key.g)       |
                                             (sreg->config.src_key.g >> 4);

                    sreg->config.src_key.b = (sreg->config.src_key.b)       |
                                             (sreg->config.src_key.b >> 4);
                }
                    break;

                default:
                    break;
            }

            if(!(dfb_config->mst_argb1555_display && (config->format & DSPF_ARGB1555)))
            {
                 tcol = sreg->config.src_key.r << 16 |
                       sreg->config.src_key.g << 8  |
                       sreg->config.src_key.b;

                 DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetTransClr_8888(tcol, 0));
                 DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, TRUE));
            }

            if( slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetTransClr_8888(tcol, 0));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, TRUE));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }
        }
        else
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE));

            if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTransClr(GOPTRANSCLR_FMT0, FALSE));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }
            
        }
    }
    if (dfb_config->mst_blink_frame_rate > 0)
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlink(true, dfb_config->mst_blink_frame_rate));

    if( dfb_config->layer_support_palette           &&
        (sreg->config_dirtyFlag & CLRCF_PALETTE) )
    {
        if( palette )
        {
            printf( "\n================================================\n");
            printf(  "[DFB] palette->num_entries: %d\n", palette->num_entries);
            printf(  "[DFB] slay->gop_index: %d\n", slay->gop_index);
            printf( "\n================================================\n");

            GOP_PaletteEntry GOPPaletteEntry8888[256];

            if(palette->entries)
            {
                int i;

                for(i = 0; i < palette->num_entries; ++i)
                {
                    GOPPaletteEntry8888[i].RGB.u8A = (u8)palette->entries[i].a;
                    GOPPaletteEntry8888[i].RGB.u8R = (u8)palette->entries[i].r;
                    GOPPaletteEntry8888[i].RGB.u8G = (u8)palette->entries[i].g;
                    GOPPaletteEntry8888[i].RGB.u8B = (u8)palette->entries[i].b;
                }

                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));

                GOP_CAP_PAL_TYPE pRet;
                DFB_UTOPIA_TRACE(MApi_GOP_GetChipCaps( E_GOP_CAP_PALETTE,
                                      &pRet,
                                      sizeof(GOP_CAP_PAL_TYPE)));

                if ( pRet.GOP_PalTbl[slay->gop_index] != 0 )
                {
                    sdev->GOP_support_palette[slay->gop_index] = true;
                    printf("\n\33[0;33;44mGOP%d support palette mode \33[0m \n", slay->gop_index);

                    if (CSTF_LAYER & surface->type)
                    {
                        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetPaletteOpt((GOP_PaletteEntry *)GOPPaletteEntry8888,
                                                    0,
                                                    (palette->num_entries-1),
                                                    E_GOP_PAL_ARGB8888));
                    }
                }
                else
                {
                    sdev->GOP_support_palette[slay->gop_index] = false;
                    printf("\n\33[0;33;44mGOP%d can't support palette mode \33[0m \n", slay->gop_index);
                }
            }
        }

    }

    //is this need set for individual gop ?
    if(GOP_API_SUCCESS == MApi_GOP_GetChipCaps(E_GOP_CAP_CONSALPHA_VALIDBITS, &enConstAlpha_bits, sizeof(EN_GOP_CONSALPHA_BITS)))
    {
        DFB_UTOPIA_TRACE(MApi_GOP_SetConfig(confugType, (MS_U32*)(&enConstAlpha_bits)));
    }

    constAlpha = sreg->config.opacity;

    if( enConstAlpha_bits == E_GOP_VALID_6BITS )
    {
        constAlpha = constAlpha >>2;
    }
    else if( enConstAlpha_bits == E_GOP_VALID_8BITS )
    {
        constAlpha = constAlpha ;
    }

    if( sreg->config_dirtyFlag & (CLRCF_OPACITY|CLRCF_OPTIONS) )
    {
        if(sreg->config.options & DLOP_OPACITY)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending( sdev->layer_gwin_id[slay->layer_index],
                                       FALSE,
                                       constAlpha));

            if( slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS )
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));

                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending( sdev->layer_gwin_id_r[slay->layer_index],
                                           FALSE,
                                           constAlpha));

                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }

            if( 0x0 == sreg->config.opacity )
            {
                bGWinEnable = false;
            }

        }
        else if (sreg->config.options & DLOP_ALPHACHANNEL)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], TRUE, constAlpha));

            if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id_r[slay->layer_index], TRUE, constAlpha));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }
        }
        else
        {
            u8 cstAlpha = 0xFF;

            if(enConstAlpha_bits == E_GOP_VALID_6BITS)
            {
                cstAlpha = cstAlpha >> 2;
            }
            else if(enConstAlpha_bits == E_GOP_VALID_8BITS)
            {
                cstAlpha = cstAlpha;
            }

            if(!(dfb_config->mst_argb1555_display && (config->format & DSPF_ARGB1555)))
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id[slay->layer_index], FALSE, cstAlpha));
            }

            if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBlending(sdev->layer_gwin_id_r[slay->layer_index], FALSE, constAlpha));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }
        }
    }
    if(sreg->config_dirtyFlag & CLRCF_HSTRETCH)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_HStretchMode(sreg->config.hstretchmode));
        DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_Set_HStretchMode: hstretchmode = 0x%x\n", sreg->config.hstretchmode);
        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_HStretchMode(sreg->config.hstretchmode));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
        }

    }

    if(sreg->config_dirtyFlag & CLRCF_VSTRETCH)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_VStretchMode(sreg->config.vstretchmode));
        DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_Set_VStretchMode: vstretchmode = 0x%x\n", sreg->config.vstretchmode);
        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_VStretchMode(sreg->config.vstretchmode));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
        }
    }

    if(sreg->config_dirtyFlag & CLRCF_TSTRETCH)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_TranspColorStretchMode(sreg->config.tstretchmode));
        DBG_LAYER_MSG ("[DFB] MApi_GOP_GWIN_Set_TranspColorStretchMode: tstretchmode = 0x%x\n", sreg->config.tstretchmode);
        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Set_TranspColorStretchMode(sreg->config.tstretchmode));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
        }
    }

    if(sreg->config_dirtyFlag & (CLRCF_DEST|CLRCF_WIDTH|CLRCF_HEIGHT))
    {
        //MApi_GOP_GWIN_SetWinPosition(sdev->layer_gwin_id[slay->layer_index], sreg->config.dest.x, sreg->config.dest.y);

        if(sreg->config.dest.w && sreg->config.dest.h)
        {
            if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width/2,sreg->config.height, sreg->config.dest.w/2,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);             
            }
            else
            {
                gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width,sreg->config.height, sreg->config.dest.w,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
            }

            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetStretchWinPosition(slay->gop_index, sreg->config.dest.x, sreg->config.dest.y));
            DBG_LAYER_MSG( "[DFB] MApi_GOP_GWIN_SetStretchWinPosition the gop is slay->gop_index:%d, x:%d, y:%d\n",
                                slay->gop_index, sreg->config.dest.x,  sreg->config.dest.y);
            if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index_r));
                gfx_GOPStretch(slay->gop_index_r,slay->gop_dst, sreg->config.width/2,sreg->config.height, sreg->config.dest.w/2,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetStretchWinPosition(slay->gop_index_r, sreg->config.dest.x, sreg->config.dest.y));
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
            }
        }
    }

    if(sreg->config_dirtyFlag & CLRCF_SOURCE)
    {
        if(sreg->config.source.w &&
           sreg->config.source.h &&
           sreg->config.source.w <= sreg->config.dest.w &&
           sreg->config.source.h <= sreg->config.dest.h)
        {

            // get gwininfo, and modify it using new gwininfo.
            GOP_GwinInfo info;

            memset(&info, 0, sizeof(GOP_GwinInfo));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

            //x1,y1,x2,y2 need 4 pixel alignment.
            x1 = ALIGN(sreg->config.source.x, 4);
            y1 = ALIGN(sreg->config.source.y, 4);
            x2 = ALIGN((sreg->config.source.x + sreg->config.source.w ), 4);
            y2 = ALIGN((sreg->config.source.y + sreg->config.source.h ), 4);

            info.u32DRAMRBlkStart = u32Phys + (y1 * lock->pitch + x1 * bpp);
            info.u16DispHPixelStart = 0;
            info.u16DispHPixelEnd   = x2 -x1;
            info.u16RBlkHPixSize    = sreg->config.width; //pixel buffer width
            info.u16DispVPixelStart = 0;
            info.u16DispVPixelEnd   = y2 - y1;
            info.u16RBlkVPixSize    = sreg->config.height; //pixel buffer Height

            DBG_LAYER_MSG("[DFB] info.u16RBlkHPixSize:%d, info.u16RBlkVPixSize:%d \n" ,  info.u16RBlkHPixSize,  info.u16RBlkVPixSize);
            DBG_LAYER_MSG("[DFB] u32Phys:0x%x, info.u32DRAMRBlkStart:0x%x \n" , u32Phys, info.u32DRAMRBlkStart);
            DBG_LAYER_MSG("[DFB] set GwinInfo (%d, %d, %d, %d)\n",info.u16DispHPixelStart, info.u16DispVPixelStart,info.u16DispHPixelEnd, info.u16DispVPixelEnd);

            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

            if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                //don't support two gop
                DBG_LAYER_MSG("[DFB] Don't support two GOP for CLRCF_SOURCE\n");
            }
            else
            {
                gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.source.w,sreg->config.source.h, sreg->config.dest.w,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);
            }

            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetStretchWinPosition(slay->gop_index, sreg->config.dest.x, sreg->config.dest.y));
        }
    }
    //close pinon to make setting works
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
        DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON( sdev->layer_gwin_id[slay->layer_index],
                            FALSE,
                            GOP_PINPON_G3D));


        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTileMode( sdev->layer_gwin_id[slay->layer_index],
                                      sreg->config.btile_mode,
                                      tiletype ));

        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], FALSE, GOP_PINPON_G3D));


            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableTileMode( sdev->layer_gwin_id_r[slay->layer_index],
                                          sreg->config.btile_mode,
                                          tiletype ));
        }

        /*
        printf("\n sreg->config.btile_mode :%d\n",sreg->config.btile_mode);
        printf("\n the tiletype is %d\n",tiletype);
        */

    }

    if (dfb_config->mst_enable_gwin_multialpha)
    {
        if (dfb_config->mst_use_dlopen_dlsym)
        {
            static bool bdlopenGOP = false;
            static void* pHandle;

            if (bdlopenGOP == false)
            {
                bdlopenGOP = true;
                pHandle = dlopen ("libapiGOP.so", RTLD_LAZY);
            }

            if(pHandle)
            {
                E_GOP_API_Result  (*MApi_GOP_GWIN_EnableMultiAlpha)(MS_U32, MS_BOOL);
                MApi_GOP_GWIN_EnableMultiAlpha = dlsym(pHandle, "MApi_GOP_GWIN_EnableMultiAlpha");

                if(!MApi_GOP_GWIN_EnableMultiAlpha)
                    DBG_LAYER_MSG("[DFB] Dlsym FAIL (MApi_GOP_GWIN_EnableMultiAlpha)!!!! error: %s \n",dlerror());
                else
                {
                    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableMultiAlpha(slay->gop_index, true));
                    DBG_LAYER_MSG("[DFB] MApi_GOP_GWIN_EnableMultiAlpha set true\n");
                }
            }
            else
                DBG_LAYER_MSG("[DFB] dlopen libapiGOP.so fail  error:%s ",dlerror());
        }
        else
        {
            if (MApi_GOP_GWIN_EnableMultiAlpha != NULL)
            {
                DBG_LAYER_MSG("[DFB][%s (%s) %d] 1Get fun MApi_GOP_GWIN_EnableMultiAlpha\n", __FUNCTION__, __FILE__, __LINE__);
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_EnableMultiAlpha(slay->gop_index, true));
            }
            else
                printf("[DFB][%s (%s) %d] Get fun MApi_GOP_GWIN_EnableMultiAlpha failed\n", __FUNCTION__, __FILE__, __LINE__);
        }

    }

    u8 goplayer = getLayerByGOPindex(slay->gop_index);

    if( (slay->gop_dst == E_GOP_DST_OP0                 ||
         slay->gop_dst == DFB_E_GOP_DST_OP_DUAL_RATE  ) && //Fix me for 4K@120
         dfb_config->mst_new_alphamode == 1             && //E_XC_OSD_BlENDING_MODE1
        (dfb_config->mst_newalphamode_on_layer & (u8)(1 << slay->layer_index)) )
    //applay on mst_newalphamode_on_layer(by bit)
    {
        DBG_LAYER_MSG("[DFB] DFB new alphamode,layerIndex=%d newalphaonlayer=%d %s(%d)\n",
                       slay->layer_index, dfb_config->mst_newalphamode_on_layer,
                       __FUNCTION__, __LINE__);


        MApi_XC_SetOSDBlendingFormula( (E_XC_OSD_INDEX)goplayer,
                                        E_XC_OSD_BlENDING_MODE1,
                                        MAIN_WINDOW);

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetNewAlphaMode(sdev->layer_gwin_id[slay->layer_index], true));
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetAlphaInverse(FALSE));
    }

    if(dfb_config->mst_gwin_disable || sdev->layer_zorder[slay->layer_index] < 1)
        bGWinEnable = FALSE;


    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable(sdev->layer_gwin_id[slay->layer_index], (MS_BOOL)bGWinEnable));

    if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable(sdev->layer_gwin_id_r[slay->layer_index], (MS_BOOL)bGWinEnable));
    }

    if( sreg->config.btile_mode )
    {
        //this code should be only used when HW FLIP is enable
        DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id[slay->layer_index], TRUE, GOP_PINPON_G3D));

        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], TRUE, GOP_PINPON_G3D));
        }

        sdev->layer_pinpon_mode[slay->layer_index] = true;
    }
    else
    {
        sdev->layer_pinpon_mode[slay->layer_index] = false;
    }

    sdev->layer_active[slay->layer_index] = true;
    mstarReorderGWIN(sdrv);
    sreg->config_dirtyFlag = CLRCF_NONE;

    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_UpdateRegOnceEx(FALSE,TRUE));


    if(curGopIndex != slay->gop_index)
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(curGopIndex));


    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}

static inline void
_mstarRecordLayerCurPhyAddr( MSTARDriverData            *sdrv,
                             MSTARDeviceData            *sdev,
                             CoreSurfaceBufferLock      *lock,
                             u8                          layer_index)
{
    hal_phy  halPhys = 0;

    if(NULL==sdrv || NULL==sdev || NULL == lock)
    {
        printf("\n recored the CurPhyAddr Failed\n");
        return ;
    }

  u16 u16TagID = 0;
  
  if(lock)
  {
    if(dfb_config->bGOPUsingHWTLB)
         halPhys = lock->phys;
    else
         halPhys = _BusAddrToHalAddr(((u64)lock->phys_h << 32) | lock->phys);
    
    u16TagID = (lock->allocation != NULL) ? (u16) lock->allocation->gfxSerial.serial : 0;
  }

    //printf("\nthe layer:%d tagID is %d\n",layer_index,u16TagID);
    sdrv->layerFlipInfo[layer_index].CurPhysAddr =  halPhys;
    sdrv->layerFlipInfo[layer_index].tagID = u16TagID;

    return;
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


    _mstarRecordLayerCurPhyAddr(sdrv, sdev, lock, slay->layer_index);

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


    if(dfb_config->mst_forcewrite_DFB & DFB_FORECWRITE_USING)
    {
        if(dfb_config->mst_forcewrite_DFB & DFB_FORCEWRITE_ENABLE)
        {
            if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_FORCEWRITE_ENABLE, parameter, &ret))
            {
                SHFREE(shm_pool, parameter);
                return DFB_FUSION;
            }

        }
        else if(dfb_config->mst_forcewrite_DFB & DFB_FORCEWRITE_DISABLE)
        {

            if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_FORCEWRITE_DISABLE, parameter, &ret ))
            {
                SHFREE(shm_pool, parameter);
                return DFB_FUSION;
            }
        }
    }

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

    D_ASSERT( sdev != NULL );
    D_ASSERT( slay != NULL );

    D_ASSERT( slay->layer_index >= 0 );
    D_ASSERT( slay->layer_index < MSTAR_MAX_OUTPUT_LAYER_COUNT );

    fusion_skirmish_prevail( &sdev->beu_lock );
    DBG_LAYER_MSG("[DFB]  %s(%d), slay->layer_index:%d\n", __FUNCTION__, __LINE__, slay->layer_index);

    sdev->layer_refcnt[slay->layer_index]--;

    if((sdev->layer_refcnt[slay->layer_index] <= 0) &&
       (INVALID_WIN_ID != sdev->layer_gwin_id[slay->layer_index]))
    {
        u16 FBId, slotID;
        CoreSurface *surface= NULL;

        //MAdp_GOP_GWIN_Enable( sdev->gfx_gop_index, 0, false) ;
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], FALSE));

        if( sdev->layer_zorder[slay->layer_index] > 0)
            sdev->layer_zorder[slay->layer_index] = 0;

        DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id[slay->layer_index], FALSE, GOP_PINPON_G3D));

        DBG_LAYER_MSG ("[DFB]  MApi_GOP_GWIN_DestroyWin: sdev->layer_gwin_id[slay->layer_index]= %d\n", sdev->layer_gwin_id[slay->layer_index]);

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id[slay->layer_index]));

        if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable( sdev->layer_gwin_id_r[slay->layer_index], FALSE));
            DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], FALSE, GOP_PINPON_G3D));
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyWin(sdev->layer_gwin_id_r[slay->layer_index]));
        }


        do
        {
            FBId = _mstarFindFBIdbyGWinID( sdrv,
                                           sdev->layer_gwin_id[slay->layer_index],
                                           &slotID,
                                           &surface);
            if(FBId != 0xff)
            {
                DBG_LAYER_MSG ("[DFB]  MApi_GOP_GWIN_DestroyFB: FBId= %d\n", FBId);
                DFB_UTOPIA_TRACE(MApi_GOP_GWIN_DestroyFB(FBId));
                _mstarSetSurfInfoSlotFree(sdrv,slotID);
            }
        }
        while(FBId!=0xff);

        sdev->layer_gwin_id[slay->layer_index] = INVALID_WIN_ID;
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

    GOP_GwinInfo info;
    hal_phy halPhys = 0;

    int bpp = DFB_BYTES_PER_PIXEL(sreg->config.format);
    hal_phy x1, y1, x2, y2;

    D_DEBUG_AT( MSTAR_Layer, "%s()\n", __FUNCTION__ );

    // if no need partial update.
    if ( ((dfb_config->mst_enable_gop_gwin_partial_update == false) ||
         (lock->phys == 0 && lock->pitch == 0)) )
        return DFB_OK;

    fusion_skirmish_prevail( &sdev->beu_lock );

    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SwitchGOP(slay->gop_index));

    DBG_LAYER_MSG("[DFB] %s, %d, update region (%d, %d, %d,%d) , config (%d, %d)\n",
            __FUNCTION__, __LINE__,
            update->x1, update->y1, update->x2, update->y2,
            sreg->config.width, sreg->config.height );

    if(dfb_config->bGOPUsingHWTLB)
        halPhys = lock->phys;
    else
        halPhys = _BusAddrToHalAddr(((u64)lock->phys_h << 32) | lock->phys);

    DBG_LAYER_MSG("[DFB] lock->phys = 0x%08x, lock->pitch = %d, halPhys = 0x%08llx,\n", lock->phys, lock->pitch, halPhys);

    x1 = (update->x1 < 0) ? 0 : ALIGN(update->x1, 4);
    y1 = (update->y1 < 0) ? 0 : ALIGN(update->y1, 4);
    x2 = (update->x2 >= (sreg->config.width - 1) ) ? (ALIGN(sreg->config.width - 1, 4)) : ALIGN(update->x2, 4);
    y2 = (update->y2 >= (sreg->config.height - 1)) ? (ALIGN(sreg->config.height - 1, 4)) : ALIGN(update->y2, 4);

    // get gwininfo, and modify it using new gwininfo.

    memset(&info, 0, sizeof(GOP_GwinInfo));
    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

    info.u32DRAMRBlkStart = halPhys + (y1 * lock->pitch + x1 * bpp);

    info.u16DispHPixelStart = x1;
    info.u16DispHPixelEnd   = x2;
    info.u16RBlkHPixSize    = info.u16DispHPixelEnd - info.u16DispHPixelStart;

    info.u16DispVPixelStart = y1;
    info.u16DispVPixelEnd   = y2;
    info.u16RBlkVPixSize    = info.u16DispVPixelEnd - info.u16DispVPixelStart;

    DBG_LAYER_MSG("[DFB] set GwinInfo (%d, %d, %d, %d)\n",
            info.u16DispHPixelStart, info.u16DispVPixelStart,
            info.u16DispHPixelEnd, info.u16DispVPixelEnd);

    DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index], &info));

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
mstarFlipRegion( CoreLayer *layer,
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
    int bpp = 0;
    int x1 = 0, y1 = 0;

    MSTARDriverData    *sdrv = driver_data;

    if(sdrv == NULL)
      return DFB_FAILURE;

    MSTARDeviceData    *sdev = sdrv->dev;
    MSTARLayerData     *slay = layer_data;
    MSTARRegionData    *sreg = region_data;

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

    // for gop gwin partial update,
    if (dfb_config->mst_enable_gop_gwin_partial_update)
    {
        const DFBRegion update = {0, 0, sreg->config.width - 1, sreg->config.height - 1};
        mstarUpdateRegion(layer, driver_data, layer_data, region_data, surface, &update, lock);
    }

    if(lock && slay)
      _mstarRecordLayerCurPhyAddr(sdrv, sdev, lock, slay->layer_index);

    //G3D mode, GWIN will been flipped directly by G3D HW
    if(sreg->config.btile_mode)
    {
        fusion_skirmish_prevail( &sdev->beu_lock );
        dfb_surface_flip( surface, false );
        fusion_skirmish_dismiss( &sdev->beu_lock );
        return DFB_OK;
    }


    bpp = DFB_BYTES_PER_PIXEL(sreg->config.format);

    if(dfb_config->bGOPUsingHWTLB)
        halPhys = lock->phys;
    else
        halPhys = _BusAddrToHalAddr(((u64)lock->phys_h << 32) | lock->phys);

    // For source rectangle implement. 4 pixel alignment
    x1 = ALIGN(sreg->config.source.x, 4);
    y1 = ALIGN(sreg->config.source.y, 4);
    halPhys = halPhys + (y1 * lock->pitch + x1 * bpp);

    tagID = (lock != NULL && lock->allocation != NULL) ? (u16) lock->allocation->gfxSerial.serial : 0;

    if (dfb_config->mst_force_wait_vsync)
    {
        targetQueueCnt = 1;
    }
    else
    {
        if(surface && surface->config.caps & DSCAPS_TRIPLE)
            targetQueueCnt = 2;
        else//  if(surface->config.caps & DSCAPS_DOUBLE)
            targetQueueCnt = 1;
    }

    DBG_LAYER_MSG("[DFB] mstarFlipRegion: lock->phys: 0x%08X, lock->pitch: %d (pid = %d)\n", lock->phys, lock->pitch, getpid());
    DBG_LAYER_MSG("[DFB] mstarFlipRegion: halPhys: 0x%08llX, slay->gop_index: %d, slay->gop_dst: %d (pid = %d)\n", halPhys, slay->gop_index, slay->gop_dst, getpid());

    dfb_print_duration(DF_MEASURE_START, "GFLIP SWAP TIME");

    do
    {
        u16QueueCnt = targetQueueCnt;

        if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            GOP_MultiFlipInfo stMultiFlipInfo;
            stMultiFlipInfo.u8InfoCnt                   = 2;
            stMultiFlipInfo.astGopInfo[0].gWinId        = sdev->layer_gwin_id[slay->layer_index];
            stMultiFlipInfo.astGopInfo[0].u32FlipAddr   = halPhys;
            stMultiFlipInfo.astGopInfo[0].u32SubAddr    = 0;
            stMultiFlipInfo.astGopInfo[0].u16WaitTagID  = tagID;
            stMultiFlipInfo.astGopInfo[0].pU16QueueCnt  = &u16QueueCnt;

            stMultiFlipInfo.astGopInfo[1].gWinId        = sdev->layer_gwin_id_r[slay->layer_index];
            stMultiFlipInfo.astGopInfo[1].u32FlipAddr   = (halPhys + lock->pitch/2);
            stMultiFlipInfo.astGopInfo[1].u32SubAddr    = 0;
            stMultiFlipInfo.astGopInfo[1].u16WaitTagID  = tagID;
            stMultiFlipInfo.astGopInfo[1].pU16QueueCnt  = &targetQueueCnt;

            if(MApi_GOP_Switch_Multi_GWIN_2_FB_BY_ADDR(stMultiFlipInfo))
            {
                break;
            }

        }
        else
        {
            if(MApi_GOP_Switch_GWIN_2_FB_BY_ADDR(sdev->layer_gwin_id[slay->layer_index], halPhys, tagID, &u16QueueCnt))
            {
                //printf("MApi_GOP_Switch_GWIN_2_FB_Fusion success , break \n");
                break;
            }
        }

        timeouttimes ++;

        //if hw flip failed,but the u16QueueCnt<targetQueueCnt ,it is strange. because  u16QueueCnt <targetQueueCnt imply flip successfully

        if ( u16QueueCnt <= (targetQueueCnt-1) )
        {
            printf("[DFB] %s: Serious Warning, Unknow Error!!\n", __FUNCTION__);
            return DFB_FAILURE;
        }

    }
    while(timeouttimes < timeThreshold);

    dfb_print_duration(DF_MEASURE_END, "GFLIP SWAP TIME");
    DBG_LAYER_MSG("[DFB] %s: timeouttimes: %d\n", __FUNCTION__, timeouttimes);

    if (timeouttimes >= timeThreshold)
    {
        GOP_GwinInfo info;
        int i = 0;

        memset(&info, 0, sizeof(GOP_GwinInfo));

        if (GOP_API_SUCCESS == MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info))
        {
            info.u32DRAMRBlkStart = halPhys;
            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info));
        }
        else
            printf("[DFB] mstarFlipRegion: MApi_GOP_GWIN_GetWinInfo fail \n");

        for(i = 0 ; i < timeouttimes ; i ++)
        {
            printf("[DFB] mstarFlipRegion: [%d]#################### layer=%d , gwin=%d \n",
                i, slay->layer_index, sdev->layer_gwin_id[slay->layer_index]);
        }
    }

    if(slay->ShadowFlags & SLF_SHADOW_LAYER_INDEXALL)
    {
        int i = 0;
        u32 layerIndexFlag = slay->ShadowFlags & SLF_SHADOW_LAYER_INDEXALL;

        for(i = 0; i < MSTAR_MAX_OUTPUT_LAYER_COUNT; i++)
        {
            if((layerIndexFlag >> i) & SLF_SHADOW_LAYER_INDEX0)
            {
                timeouttimes = 0;    

                do
                {
                    u16QueueCnt = 1;
                    //if(MApi_GOP_Switch_GWIN_2_FB(sdev->layer_gwin_id[slay->layer_index], u16FBId, tagID, &u16QueueCnt))

                    if(MApi_GOP_Switch_GWIN_2_FB_BY_ADDR(sdev->layer_gwin_id[i], halPhys, 0, &u16QueueCnt))
                    {
                        //printf("MApi_GOP_Switch_GWIN_2_FB_Fusion success , break \n");
                        break;
                    }

                    timeouttimes ++;

                }
                while(timeouttimes < timeThreshold);

                if(timeouttimes >= timeThreshold)
                {
                    int j;
                    for(j = 0 ; j< timeouttimes ; j ++)
                        printf("[DFB][%d]#################### shadow_layer_index=%d , gwin=%d \n",j,i,sdev->layer_gwin_id[i]);
                }
            }
        }
    }


    fusion_skirmish_prevail( &sdev->beu_lock );
    dfb_surface_flip( surface, false );
    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
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

    CoreSurface *surface;
    int i, number_buffers;

    CoreSurfaceBufferLock ret_lock;

    int freeSurfInfoSlot;
    u8 u8FBID, u8GOP_Ret;

    CoreSurfaceTypeFlags cstf_flag = 0;

    u32 u32Phys = 0;
    u32 u32_gop_index = 0;
    bool tryAgain = false;

    if(_mstarDFBFmt2MSFmt(config->format) == E_MS_FMT_GENERIC)
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

    cstf_flag = CSTF_LAYER;

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
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;

    DFBResult         ret;
    CoreSurfaceConfig conf;

    int i = 0, number_buffers = 0;

    CoreSurfaceBufferLock ret_lock;

    u32 u32Phys = 0;
    bool tryAgain = false;

    DBG_LAYER_MSG ("[DFB] %s (%d), w =%d, h =%d\n", __FUNCTION__, __LINE__ , config->width, config->height );

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
_mstarSetLayerlevel( CoreLayer              *layer,
                     void                   *driver_data,
                     void                   *layer_data,
                     int                     level )
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    DBG_LAYER_MSG("[DFB] %s(%d): level = %d\n", __FUNCTION__, __LINE__, level);

    fusion_skirmish_prevail( &sdev->beu_lock );

    if (dfb_config->mst_gwin_disable)
        level = -1;

    /**/
    if(sdev->layer_zorder[slay->layer_index] != level)
    {
        sdev->layer_zorder[slay->layer_index] = level;
        mstarReorderGWIN(sdrv);

        if(level > 0 && sdev->layer_pinpon_mode[slay->layer_index])
            DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id[slay->layer_index], TRUE, GOP_PINPON_G3D));

        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], level>0?TRUE:FALSE));

        if(level <= 0)
            DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id[slay->layer_index], FALSE, GOP_PINPON_G3D));

        if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
        {
            if((level > 0) && sdev->layer_pinpon_mode[slay->layer_index])
                DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], TRUE, GOP_PINPON_G3D));

            DFB_UTOPIA_TRACE(MApi_GOP_GWIN_Enable( sdev->layer_gwin_id_r[slay->layer_index], level>0?TRUE:FALSE));

            if(level <= 0)
                DFB_UTOPIA_TRACE(MApi_GOP_SetPINPON(sdev->layer_gwin_id_r[slay->layer_index], FALSE, GOP_PINPON_G3D));
        }
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
    bool bBankForceWrite = dfb_get_BankForceWrite();

    if((adjustment->flags & DCAF_HUE)            ||
       (adjustment->flags & DCAF_SATURATION))
    {
        printf("SetColorAdjustment don't support these adjustment yet! \n");
        return DFB_UNSUPPORTED;
    }

    fusion_skirmish_prevail( &sdev->beu_lock );

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
       DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(slay->gop_index, true));
    }

    if(adjustment->flags & DCAF_CONTRAST)
    {
        u16 u16ContrastValue = 0;
        u16 u16Ratio = 1; //default ratio:1
        u16 u16UsrMidValue = 50; //Middle value is 50 for user define
        u16 u16GOPContrastMideVaue = 0x10; //Middle value is 0x10 for Driver
        if(adjustment->contrast>100)
        {
             printf("warning !!! The value of contrast must be less than 100 \n");
             adjustment->contrast=100;
        }
        if(adjustment->contrast <= u16UsrMidValue)
        {
             u16Ratio = (u16) (u16UsrMidValue / u16GOPContrastMideVaue);
             u16ContrastValue = (u16) adjustment->contrast / u16Ratio;
        }
        if(adjustment->contrast > u16UsrMidValue)
        {
              u16ContrastValue = ((adjustment->contrast - u16UsrMidValue) + u16GOPContrastMideVaue);
              if(u16ContrastValue > 0x3f) //Contrast value will be 0x0-0x3f
                  u16ContrastValue = 0x3f;
        }
        if(GOP_API_SUCCESS!=MApi_GOP_SetGOPContrast(slay->gop_index,u16ContrastValue,u16ContrastValue,u16ContrastValue))
            ret=DFB_FAILURE;
    }
    else
    {
         printf("flag DCAF_CONTRAST not set! \n");
         ret = DFB_FAILURE;
    }

    if(adjustment->flags & DCAF_BRIGHTNESS)
    {
        if(0 == MApi_GOP_SetGOPBrightness(slay->gop_index, adjustment->brightness, adjustment->bMSB))
            ret = DFB_FAILURE;
    }
    else
    {
        printf("flag DCAF_BRIGHTNESS not set! \n");
        ret = DFB_FAILURE;
    }

    if (bBankForceWrite && true == dfb_config->mst_bank_force_write)
    {
        DFB_UTOPIA_TRACE(MApi_GOP_GWIN_SetBnkForceWrite(slay->gop_index, false));
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

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


DFBResult
_mstar_ForceWriteEnable(bool bEnable)
{
    DBG_LAYER_MSG("[DFB] %s(%d): MApi_GOP_GWIN_SetForceWrite was removed!!!\n", __FUNCTION__, __LINE__);
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
    MSTARLayerData  *slay = layer_data;
    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;


    u8      u8SubFbId = MAX_GWIN_FB_SUPPORT;
    u16     slotID = 0;
    u8      fbid = MAX_GWIN_FB_SUPPORT;
    int     pitch = lock->pitch;
    hal_phy  fbaddr;

    if(dfb_config->bGOPUsingHWTLB)
        fbaddr = lock->phys;
    else
        fbaddr = _BusAddrToHalAddr(((u64)lock->phys_h << 32) | lock->phys);

    //printf("\nthe fbaddr is %x\n",fbaddr);

    DBG_LAYER_MSG("[DFB] %s(%d): display_mode = %d\n", __FUNCTION__, __LINE__, display_mode);

    if(display_mode == slay->layer_displayMode)
        return DFB_OK;

    if(( (display_mode != DLDM_STEREO_FRAME_PACKING                 &&
          display_mode != DLDM_STEREO_FRAME_PACKING_FHD)            &&
         (slay->layer_displayMode != DLDM_STEREO_FRAME_PACKING      &&
          slay->layer_displayMode != DLDM_STEREO_FRAME_PACKING_FHD) ))
    {
        //no need to do anything
        slay->layer_displayMode = display_mode;
        return DFB_OK;
    }

    if( display_mode == DLDM_STEREO_FRAME_PACKING ||
        display_mode == DLDM_STEREO_FRAME_PACKING_FHD)
    {
        if(!(((surface->config.size.w == FRAME_PACKAGE_WIDTH_1280)    &&
              (surface->config.size.h == FRAME_PACKAGE_HEIGHT_720))   ||
             ((surface->config.size.w == FRAME_PACKAGE_WIDTH_1920)    &&
              (surface->config.size.h == FRAME_PACKAGE_HEIGHT_1080))))
        {
            //printf("\nthe layer size is width:%d height:%d\n",surface->config.size.w,surface->config.size.h);
            //printf("\ncan not set displaymode DLDM_STEREO_FRAME_PACKING  DLDM_STEREO_FRAME_PACKING mode need 1280x720 or 1920x1080 layer\n");
            return DFB_FAILURE;
        }
    }


    do
    {
        fbid = _mstarFindFBIdbyGWinID(sdrv, sdev->layer_gwin_id[slay->layer_index], &slotID, &surface);

        if(fbid != 0xff)
        {
            MApi_GOP_GWIN_DestroyFB(fbid);
            _mstarSetSurfInfoSlotFree(sdrv, slotID);
        }
    } while(fbid != 0xff);
        

    MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], FALSE);

    if( sdev->layer_zorder[slay->layer_index] > 0)
        sdev->layer_zorder[slay->layer_index] = 0;

    if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
    {
        MApi_GOP_GWIN_Enable( sdev->layer_gwin_id_r[slay->layer_index], FALSE);
    }

    MApi_GOP_GWIN_UpdateRegOnce(TRUE);    


    switch(display_mode)
    {
        case DLDM_STEREO_FRAME_PACKING:
        case DLDM_STEREO_FRAME_PACKING_FHD:
        {

            u8 fbid = MApi_GOP_GWIN_GetFreeFBID();

            MApi_GOP_GWIN_CreateFBbyStaticAddr( fbid,
                                                0,
                                                0,
                                                surface->config.size.w,
                                                surface->config.size.h,
                                                _mstarDFBFmt2MSFmt(surface->config.format),
                                                fbaddr);

            MApi_GOP_GWIN_MapFB2Win(fbid, sdev->layer_gwin_id[slay->layer_index]);

            MApi_GOP_Set3DOSDMode( sdev->layer_gwin_id[slay->layer_index],
                                   fbid,
                                   fbid,
                                   E_GOP_3D_FRAMEPACKING);

            if (display_mode == DLDM_STEREO_FRAME_PACKING)
            {
                gfx_GOPStretch( slay->gop_index,
                                slay->gop_dst,
                                surface->config.size.w,
                                surface->config.size.h,
                                surface->config.size.w,
                                surface->config.size.h,
                                0,
                                0 );
            }
            else // DLDM_STEREO_FRAME_PACKING_FHD
            {
                if ((slay->screen_size.width != FRAME_PACKAGE_WIDTH_1920)   ||
                    (slay->screen_size.height != FRAME_PACKAGE_HEIGHT_1080))
                    DBG_LAYER_MSG("\33[1;31m DLDM_STEREO_FRAME_PACKING_FHD -> slay->screen_size.width:%d  slay->screen_size.height:%d \33[0m\n",
                            slay->screen_size.width, slay->screen_size.height);

                    //for box setting the screen size is 1920x2205, patch it to 1920x1080.
                    gfx_GOPStretch( slay->gop_index,
                                    slay->gop_dst,
                                    surface->config.size.w,
                                    surface->config.size.h,
                                    FRAME_PACKAGE_WIDTH_1920,
                                    FRAME_PACKAGE_HEIGHT_1080,
                                    0,
                                    0 );
            }

            MApi_GOP_GWIN_MapFB2Win(fbid, sdev->layer_gwin_id[slay->layer_index]);

            slotID = _mstarGetFreeSurfInfoSlot(sdrv);

            if(slotID != -1)
            {
                _mstarSetSurfInfoSlot( sdrv,
                                       slotID,
                                       fbid,
                                       surface,
                                       lock->buffer,
                                       fbaddr,
                                       lock->pitch );
            }

            _mstarSetSlotGWinID(sdrv, slotID, sdev->layer_gwin_id[slay->layer_index]);

            //printf("\nDLDM_STERRO_FRAME_PACKING coming here======== the fbid is %d \n",fbid);

            break;
        }
        case DLDM_NORMAL:
        case DLDM_STEREO_LEFTRIGHT_3DUI:
        case DLDM_STEREO_TOPBOTTOM_3DUI:
        case DLDM_STEREO_LEFTRIGHT_TOPHALF_3DUI:
        case DLDM_STEREO_TOPHALF_3DUI:
        {
            u8 fbid= MApi_GOP_GWIN_GetFreeFBID();

            MApi_GOP_GWIN_CreateFBbyStaticAddr( fbid,
                                                0,
                                                0,
                                                surface->config.size.w,
                                                surface->config.size.h,
                                                _mstarDFBFmt2MSFmt(surface->config.format),
                                                fbaddr);

            MApi_GOP_GWIN_MapFB2Win(fbid, sdev->layer_gwin_id[slay->layer_index]);

            MApi_GOP_Set3DOSDMode( sdev->layer_gwin_id[slay->layer_index],
                                   fbid,
                                   MAX_GWIN_FB_SUPPORT,
                                   E_GOP_3D_DISABLE);


            if(slay->gop_index_r != 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                GOP_GwinInfo info;
                //gfx_GOPStretch(slay->gop_index,slay->gop_dst, sreg->config.width/2,sreg->config.height, sreg->config.dest.w/2,sreg->config.dest.h,sreg->config.dest.x, sreg->config.dest.y);

                gfx_GOPStretch( slay->gop_index,
                                slay->gop_dst,
                                surface->config.size.w/2,
                                surface->config.size.h,
                                slay->screen_size.width/2,
                                slay->screen_size.height,
                                0,
                                0);

                memset(&info, 0, sizeof(GOP_GwinInfo));

                MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id_r[slay->layer_index],&info);

                info.u16DispHPixelEnd = surface->config.size.w/2;
                info.u16RBlkHPixSize = info.u16DispHPixelEnd - info.u16DispHPixelStart;

                MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id[slay->layer_index],&info);
            }
            else
            {
                gfx_GOPStretch( slay->gop_index,
                                slay->gop_dst,
                                surface->config.size.w,
                                surface->config.size.h,
                                slay->screen_size.width,
                                slay->screen_size.height,
                                0,
                                0 );
            }
            //MApi_GOP_GWIN_MapFB2Win(fbid, sdev->layer_gwin_id[slay->layer_index]);

            if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
            {
                GOP_GwinInfo info;
                memset(&info, 0, sizeof(GOP_GwinInfo));

                MApi_GOP_GWIN_MapFB2Win(fbid, sdev->layer_gwin_id_r[slay->layer_index]);

                MApi_GOP_Set3DOSDMode( sdev->layer_gwin_id_r[slay->layer_index],
                                       fbid,
                                       MAX_GWIN_FB_SUPPORT,
                                       E_GOP_3D_DISABLE);

                gfx_GOPStretch( slay->gop_index_r,
                                slay->gop_dst,
                                surface->config.size.w,
                                surface->config.size.h,
                                slay->screen_size.width,
                                slay->screen_size.height,
                                0,
                                0);

                gfx_GOPStretch( slay->gop_index_r,
                                slay->gop_dst,
                                surface->config.size.w/2,
                                surface->config.size.h,
                                slay->screen_size.width/2,
                                slay->screen_size.height,
                                0,
                                0);

                MApi_GOP_GWIN_GetWinInfo(sdev->layer_gwin_id_r[slay->layer_index],&info);
                
                info.u16DispHPixelEnd = surface->config.size.w/2;
                info.u16RBlkHPixSize = info.u16DispHPixelEnd - info.u16DispHPixelStart;

                MApi_GOP_GWIN_SetWinInfo(sdev->layer_gwin_id_r[slay->layer_index],&info);

            }

            slotID = _mstarGetFreeSurfInfoSlot(sdrv);

            if(slotID != -1)
            {
                _mstarSetSurfInfoSlot( sdrv,
                                       slotID,
                                       fbid,
                                       surface,
                                       lock->buffer,
                                       fbaddr,
                                       lock->pitch );
            }
            else
            {
                D_DERROR(DFB_FAILURE,"T\nthere is no slot %s:%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
                D_ASSERT(slotID!=-1);
            }

            _mstarSetSlotGWinID(sdrv, slotID, sdev->layer_gwin_id[slay->layer_index]);

            break;
        }

    default:
            break;


    }   // end of switch

    MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], (dfb_config->mst_gwin_disable)? FALSE : TRUE);
    
    if( sdev->layer_zorder[slay->layer_index] >= 0)
        sdev->layer_zorder[slay->layer_index] = 1;

    if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
    {
        MApi_GOP_GWIN_Enable( sdev->layer_gwin_id_r[slay->layer_index], (dfb_config->mst_gwin_disable)? FALSE : TRUE);
    }

    MApi_GOP_GWIN_UpdateRegOnce(FALSE);

    // MApi_GOP_GWIN_CreateFBbyStaticAddr(u8NewFbid, 0, 0, surface->config.size.w,surface->config.size.h , _mstarDFBFmt2MSFmt(surface->config.format), fbaddr);

    slay->layer_displayMode = display_mode;

    return DFB_OK;
}



static DFBResult
mstar_ConfigDisplayMode( CoreLayer                          *layer,
                         void                               *driver_data,
                         void                               *layer_data,
                         CoreSurface                        *surface,
                         CoreSurfaceBufferLock              *lock,
                         DFBDisplayLayerDeskTopDisplayMode   display_mode)

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

    parameter->display_mode = display_mode;
    parameter->surface      = surface;
    parameter->lock         = lock;

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_CONFIG_DISPLAYMODE, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

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
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    fusion_skirmish_prevail( &sdev->beu_lock );
    /**/
    if(HEnable)
    {
        DBG_LAYER_MSG("[DFB]MApi_GOP_GWIN_SetHMirror(TRUE)\n");
        MApi_GOP_GWIN_SetHMirror(TRUE);
    }
    else
    {
        DBG_LAYER_MSG("[DFB]MApi_GOP_GWIN_SetHMirror(FALSE)\n");
        MApi_GOP_GWIN_SetHMirror(FALSE);
    }

    if(VEnable)
    {
        DBG_LAYER_MSG("[DFB]MApi_GOP_GWIN_SetVMirror(TRUE)\n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
    }
    else
    {
        DBG_LAYER_MSG("[DFB]MApi_GOP_GWIN_SetVMirror(FALSE)\n");
        MApi_GOP_GWIN_SetVMirror(FALSE);
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

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


/*
* Set the LBCouple Mode
*/
DFBResult
_mstar_SetLBCoupleEnable( CoreLayer              *layer,
                          void                   *driver_data,
                          void                   *layer_data,
                          bool                    LBCoupleEnable)
{
    MSTARDriverData *sdrv = driver_data;
    MSTARLayerData  *slay = layer_data;
    MSTARDeviceData *sdev = sdrv->dev;

    fusion_skirmish_prevail( &sdev->beu_lock );

    /**/
    if(LBCoupleEnable)
    {
        MApi_GOP_EnableLBCouple(slay->gop_index, TRUE);
    }
    else
    {
        MApi_GOP_EnableLBCouple(slay->gop_index, FALSE);
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

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

    fusion_skirmish_prevail( &sdev->beu_lock );
    /**/

    if(ByPassEnable)// change to Bypass mode.
    {
        MApi_GOP_GWIN_SetGOPDst(slay->gop_index, E_GOP_DST_BYPASS);
        slay->gop_dst = E_GOP_DST_BYPASS;
    }
    else // change to OP mode.
    {
        MApi_GOP_GWIN_SetGOPDst(slay->gop_index, E_GOP_DST_OP0);
        slay->gop_dst = E_GOP_DST_OP0;
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}
    
DFBResult
mstar_SetLBCoupleEnable( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         bool                    LBCoupleEnable)
{
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    DFBResult ret = DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->LBCoupleEnable, &LBCoupleEnable, sizeof(LBCoupleEnable));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_LBCOUPLEMODE, parameter, &ret ))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(&LBCoupleEnable, &parameter->LBCoupleEnable, sizeof(LBCoupleEnable));

    SHFREE(shm_pool, parameter);

    return ret;

}

DFBResult
mstar_SetGOPDstByPassEnable( CoreLayer              *layer,
                             void                   *driver_data,
                             void                   *layer_data,
                             bool                    ByPassEnable)
{
    MSTARLayerData          *slay = layer_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    DFBResult ret = DFB_OK;

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));

    if(!parameter)
       return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->ByPassEnable, &ByPassEnable, sizeof(ByPassEnable));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_GOPBYPASSMODE, parameter, &ret))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    memcpy(&ByPassEnable, &parameter->ByPassEnable, sizeof(ByPassEnable));

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
    MSTARLayerData  *slay = layer_data;
    MSTARRegionData *sreg = region_data;
    MSTARDeviceData *sdev = sdrv->dev;

    DBG_LAYER_MSG("[DFB] %s(%d): HScale = %d  VScale=%d, slay->gop_index:%d \n", __FUNCTION__, __LINE__,
        HScale, VScale, slay->gop_index);

    fusion_skirmish_prevail( &sdev->beu_lock );

    /**/
    MApi_GOP_GWIN_Set_STRETCHWIN( slay->gop_index,
                                  slay->gop_dst,
                                  sreg->config.dest.x,
                                  sreg->config.dest.y,
                                  sreg->config.width,
                                  sreg->config.height );

    MApi_GOP_GWIN_Set_HSCALE(TRUE, sreg->config.width, HScale);
    MApi_GOP_GWIN_Set_VSCALE(TRUE, sreg->config.height, VScale);

    if (slay->gop_dst == E_GOP_DST_OP0  ||
        slay->gop_dst == DFB_E_GOP_DST_OP_DUAL_RATE) //Fix me for 4K@120
        MApi_GOP_SetGOPHStart(slay->gop_index, mstar_sc_get_h_cap_start());

    fusion_skirmish_dismiss( &sdev->beu_lock );

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

static EN_GOP_DST_TYPE
transformDfbDst2Utopia(DFBDisplayLayerGOPDST         GopDst)
{
    EN_GOP_DST_TYPE gop_dst;
    switch (GopDst)
    {
        case DLGD_IP0:
                gop_dst = E_GOP_DST_IP0;
                break;
            
        case DLGD_MIXER2VE:
                gop_dst = E_GOP_DST_MIXER2VE;
                break;
            
        case DLGD_OP0:
                gop_dst = E_GOP_DST_OP0;
                break;
            
        case DLGD_VOP:
                gop_dst = E_GOP_DST_VOP;
                break;
            
        case DLGD_IP1:
                gop_dst = E_GOP_DST_IP1;
                break;
            
        case DLGD_IP_MAIN:
                gop_dst = E_GOP_DST_IP_MAIN;
                break;
            
        case DLGD_IP_SUB:
                gop_dst = E_GOP_DST_IP_SUB;
                break;
            
        case DLGD_MIXER2OP:
                gop_dst = E_GOP_DST_MIXER2OP;
                break;
            
        case DLGD_VOP_SUB:
                gop_dst = E_GOP_DST_VOP_SUB;
                break;
            
        case DLGD_FRC:
                gop_dst = E_GOP_DST_FRC;
                break;
            
        case DLGD_VE:
                gop_dst = E_GOP_DST_VE;
                break;
            
        case DLGD_OP1:
                gop_dst = E_GOP_DST_OP1;
                break;
            
        case DLGD_MIXER2OP1:
                gop_dst = E_GOP_DST_MIXER2OP1;
                break;
            
        case DLGD_DIP:
                gop_dst = E_GOP_DST_DIP;
                break;
            
        case DLGD_GOPScaling:
                gop_dst = E_GOP_DST_GOPScaling;
                break;
            
        case DLGD_BYPASS:
                gop_dst = E_GOP_DST_BYPASS;
                break;

        //Fix me for 4K@120
        case DLGD_OP_DUAL_RATE:
                gop_dst = DFB_E_GOP_DST_OP_DUAL_RATE;
                break;

        default:
                gop_dst = E_GOP_DST_OP0;
                break;
    }

    return gop_dst;
}


static DFBDisplayLayerGOPDST
transformUtopia2DfbDst( EN_GOP_DST_TYPE         gop_dst )
{
    DFBDisplayLayerGOPDST GopDst;
    switch (gop_dst)
    {
        case E_GOP_DST_IP0:
                GopDst = DLGD_IP0;
                break;
            
        case E_GOP_DST_MIXER2VE:
                GopDst = DLGD_MIXER2VE;
                break;
            
        case E_GOP_DST_OP0:
                GopDst = DLGD_OP0;
                break;
            
        case E_GOP_DST_VOP:
                GopDst = DLGD_VOP;
                break;
            
        case E_GOP_DST_IP1:
                GopDst = DLGD_IP1;
                break;
            
        case E_GOP_DST_IP_MAIN:
                GopDst = DLGD_IP_MAIN;
                break;
            
        case E_GOP_DST_IP_SUB:
                GopDst = DLGD_IP_SUB;
                break;
            
        case E_GOP_DST_MIXER2OP:
                GopDst = DLGD_MIXER2OP;
                break;
            
        case E_GOP_DST_VOP_SUB:
                GopDst = DLGD_VOP_SUB;
                break;
            
        case E_GOP_DST_FRC:
                GopDst = DLGD_FRC;
                break;
            
        case E_GOP_DST_VE:
                GopDst = DLGD_VE;
                break;
            
        case E_GOP_DST_OP1:
                GopDst = DLGD_OP1;
                break;
            
        case E_GOP_DST_MIXER2OP1:
                GopDst = DLGD_MIXER2OP1;
                break;
            
        case E_GOP_DST_DIP:
                GopDst = DLGD_DIP;
                break;
            
        case E_GOP_DST_GOPScaling:
                GopDst = DLGD_GOPScaling;
                break;
            
        case E_GOP_DST_BYPASS:
                GopDst = DLGD_BYPASS;
                break;

        //Fix me for 4K@120
        case DFB_E_GOP_DST_OP_DUAL_RATE:
                GopDst = DLGD_OP_DUAL_RATE;
                break;
        default:
                GopDst = DLGD_UNKNOWN;
                break;
    }

    return GopDst;
}


DFBResult
mstar_SetGOPDst( CoreLayer                      *layer,
                 void                           *driver_data,
                 void                           *layer_data,
                 DFBDisplayLayerGOPDST           GopDst)
{
    MSTARLayerData          *slay = layer_data;
    MSTARDriverData         *sdrv = driver_data;
    CoreLayerCallParameter  *parameter;
    FusionSHMPoolShared     *shm_pool;

    EN_GOP_DST_TYPE gop_dst;
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

    gop_dst = transformDfbDst2Utopia(GopDst);
    
    memcpy(&parameter->GopDst, &gop_dst, sizeof(gop_dst));

    if (fusion_call_execute(&slay->call, FCEF_NONE, CLC_SET_GOPDST, parameter, &ret))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    slay->gop_dst = gop_dst;

    switch(gop_dst)
    {
        case E_GOP_DST_IP0:
            layer->screen = sdrv->ip0_screen;
            break;
            
        case E_GOP_DST_OP0:
            layer->screen = sdrv->op_screen;
            break;
            
        case E_GOP_DST_BYPASS:
            layer->screen = sdrv->op_screen;
            break;

        case E_GOP_DST_VE:
            layer->screen = sdrv->ve_screen;
            break;

        //Fix me for 4K@120
        case DFB_E_GOP_DST_OP_DUAL_RATE:
            layer->screen = sdrv->op_screen;
            break;

        default:
            break;
    }

    SHFREE(shm_pool, parameter);

    return ret;    
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

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(layer->core);

    shm_pool = dfb_core_shmpool_data(layer->core);
    D_ASSERT(shm_pool);

    parameter = SHMALLOC(shm_pool, sizeof(*parameter));
    if(!parameter)
        return DFB_NOSHAREDMEMORY;

    memcpy(&parameter->ForceWrite, &force_write, sizeof(force_write));

    if (fusion_call_execute( &slay->call, FCEF_NONE, CLC_SET_FORCEWRITE, parameter, &ret))
    {
        SHFREE(shm_pool, parameter);
        return DFB_FUSION;
    }

    SHFREE(shm_pool, parameter);

    return ret;    
}


DFBResult
_mstar_SetGOPDst( CoreLayer              *layer,
                  void                   *driver_data,
                  void                   *layer_data,
                  EN_GOP_DST_TYPE         GopDst)
{
    MSTARDriverData *sdrv   = driver_data;
    MSTARLayerData  *slay   = layer_data;
    MSTARDeviceData *sdev   = sdrv->dev;
    EN_GOP_DST_TYPE gop_dst = GopDst;

    DFBResult ret = DFB_OK;
    unsigned char gwin_enabled = 0;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(sdrv->dev);

    DBG_LAYER_MSG("[DFB] %s(%d): GopDst = %d  layer=%d  gwinid=%d \n", __FUNCTION__, __LINE__,
        gop_dst, slay->layer_index, sdev->layer_gwin_id[slay->layer_index]);

    fusion_skirmish_prevail( &sdev->beu_lock );

    if(sdev->layer_gwin_id[slay->layer_index] != INVALID_WIN_ID)
    {
        gwin_enabled = MApi_GOP_GWIN_IsGWINEnabled(sdev->layer_gwin_id[slay->layer_index]);
    }

    if(gwin_enabled)
        MApi_GOP_GWIN_Enable(sdev->layer_gwin_id[slay->layer_index], FALSE);

    slay->gop_dst = gop_dst;
    MApi_GOP_GWIN_SetGOPDst(slay->gop_index, gop_dst);

    if(slay->gop_index_r!= 0xf && slay->gop_dst == E_GOP_DST_BYPASS)
    {
        MApi_GOP_GWIN_SetGOPDst(slay->gop_index_r, gop_dst);
    }

    switch (gop_dst)
    {
        case E_GOP_DST_IP0:
        {
            layer->screen = sdrv->ip0_screen;

            MS_XC_DST_DispInfo dstDispInfo;

            if(MApi_XC_GetDstInfo(&dstDispInfo, sizeof(MS_XC_DST_DispInfo), E_GOP_XCDST_IP1_MAIN))
            {
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
            break;

        //Fix me for 4K@120
        case DFB_E_GOP_DST_OP_DUAL_RATE:
        case E_GOP_DST_OP0:
        {
            MS_PNL_DST_DispInfo dstDispInfo;

            layer->screen = sdrv->op_screen;

            MApi_PNL_GetDstInfo(&dstDispInfo, sizeof(MS_PNL_DST_DispInfo));

            if(dstDispInfo.bYUVOutput)
            {
                MApi_GOP_GWIN_OutputColor(GOPOUT_YUV);
            }
            else
            {
                if(dfb_config->mst_GOP_Set_YUV)
                {
                    MApi_GOP_GWIN_OutputColor(GOPOUT_YUV);
                }
                else
                {
                    MApi_GOP_GWIN_OutputColor(GOPOUT_RGB);
                }
            }
        }
            break;

        case E_GOP_DST_BYPASS:
        {
            layer->screen = sdrv->op_screen;
        }
            break;

        case E_GOP_DST_VE:
        {
            //For K3 Set the gopVETimingType
             VE_DrvStatus DrvStatus;
            layer->screen = sdrv->ve_screen;
            if (MDrv_VE_GetStatus != NULL)
                 MDrv_VE_GetStatus(&DrvStatus);
            
            GOP_VE_TIMINGTYPE gopVETimingType;

            if(DrvStatus.VideoSystem <= 2) // MS_VE_NTSC_J
            {
                gopVETimingType = GOP_VE_NTSC;
            }
            else
            {
                gopVETimingType = GOP_VE_PAL;
            }

            printf("[DFB] MApi_GOP_VE_SetOutputTiming : %d\n", gopVETimingType);

            if (MApi_GOP_VE_SetOutputTiming != NULL)
                 MApi_GOP_VE_SetOutputTiming(gopVETimingType);

        }
            break;

        default:
            break;
    }

    //printf("%s:%d, ret = 0x%x, pid =%d\n", __FUNCTION__, __LINE__, ret, getpid());
    if(gwin_enabled) {
        MApi_GOP_GWIN_Enable( sdev->layer_gwin_id[slay->layer_index], (dfb_config->mst_gwin_disable)? FALSE : TRUE);
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return DFB_OK;
}


DFBResult
_mstar_SetForceWrite( CoreLayer              *layer,
                      void                   *driver_data,
                      void                   *layer_data,
                      bool                    force_write )
{
    DBG_LAYER_MSG("[DFB] %s(%d): MApi_GOP_GWIN_SetForceWrite was removed!!!\n", __FUNCTION__, __LINE__);
    return DFB_OK;
}

DFBResult
mstar_GetGOPDst( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 DFBDisplayLayerGOPDST      *GopDst)
{
    MSTARDriverData     *sdrv = driver_data;
    MSTARDeviceData     *sdev = sdrv->dev;
    MSTARLayerData      *slay = layer_data;

    DFBDisplayLayerID layerid;

    if(GopDst)
    {
        *GopDst = transformUtopia2DfbDst(slay->gop_dst);
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

    GOP_InitInfo gopInitInfo;
    u8 curGopIndex;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(sdrv->dev);

    fusion_skirmish_prevail( &sdev->beu_lock );

    //Prepare GOP Init Params:
    gopInitInfo.u16PanelWidth        = dfb_config->mst_lcd_width;
    gopInitInfo.u16PanelHeight       = dfb_config->mst_lcd_height;
    gopInitInfo.u16PanelHStr         = mstar_sc_get_h_cap_start();
    gopInitInfo.u32GOPRBAdr          = 0;
    gopInitInfo.u32GOPRBLen          = 0;
    gopInitInfo.bEnableVsyncIntFlip  = TRUE;


    curGopIndex = MApi_GOP_GWIN_GetCurrentGOP();

    if(curGopIndex!=slay->gop_index)
        MApi_GOP_GWIN_SwitchGOP(slay->gop_index);


    MApi_GOP_GWIN_SetFieldInver(TRUE);


    if(curGopIndex!=slay->gop_index)
        MApi_GOP_GWIN_SwitchGOP(curGopIndex);

    if (sdev->layer_gwin_id[slay->layer_index] != INVALID_WIN_ID)// this layer already have Gwin id, the display will fail
    {
        printf("\n\n[DFB] ================================================================================= \n");
        printf("     <<< FATAL ERROR: The boot-logo delay function error !!! >>>\n");
        printf("    Please select another layer for boot-logo delay, the layer %d have already be used \n", slay->layer_index);
        printf("[DFB] ================================================================================= \n\n");
    }

    EN_GOP_IGNOREINIT eIgnor;
    eIgnor = E_GOP_IGNORE_MUX;

    //close  GOP IGNORE INIT
    MApi_GOP_SetConfig(E_GOP_IGNOREINIT,&eIgnor);

    //Initial boot logo GOP
    MApi_GOP_InitByGOP(&gopInitInfo,  slay->gop_index);

    printf("[DFB] _mstar_SetBootLogoPatch : MApi_GOP_InitByGOP gopindex: %d\n",slay->gop_index);

    //Set the GOP miu select to correct location.
    MApi_GOP_MIUSel(slay->gop_index, (EN_GOP_SEL_TYPE) miusel);
    //printf("[DFB] MApi_GOP_MIUSel  %d\n");


    if(sdrv && sdrv->core && sdrv->core->shared)
         sdrv->core->shared->bootlogo_layer_id = -1; /* reset the flag */

    fusion_skirmish_dismiss( &sdev->beu_lock );

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

            *ret_val = _mstarSetLayerlevel( layer,
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

        case CLC_FORCEWRITE_ENABLE:
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
            break;

        case CLC_CONFIG_DISPLAYMODE:
        {
            DFBDisplayLayerDeskTopDisplayMode display_mode;
            display_mode = parameter->display_mode;

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

        case CLC_SET_GOPDST:
        {
            EN_GOP_DST_TYPE *GopDst;
            GopDst = &parameter->GopDst;

            *ret_val =  _mstar_SetGOPDst( layer,
                                          layer->driver_data,
                                          layer->layer_data,
                                         *GopDst );
        }
            break;

        case CLC_SET_FORCEWRITE:
        {
            bool ForceWrite;
            ForceWrite = parameter->ForceWrite;

            *ret_val = _mstar_SetForceWrite( layer,
                                             layer->driver_data,
                                             layer->layer_data,
                                             ForceWrite );
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

        default:
            D_BUG( "[DFB] Error!!! Unknown  Command '%d'", command );
            *ret_val = DFB_BUG;
    }

    return FCHR_RETURN;
}


DFBResult
mstar_ConfigShadowLayer( CoreLayer              *layer,
                         void                   *driver_data,
                         void                   *layer_data,
                         ShadowLayerConfig      *shadowLayer_info)
{

    MSTARDriverData *sdrv = driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer_data;
    DFBResult ret = DFB_OK;

    D_ASSERT(layer);
    D_ASSERT(driver_data);
    D_ASSERT(layer_data);
    D_ASSERT(sdrv->dev);
    
    fusion_skirmish_prevail( &sdev->beu_lock );

    //this layer is set as shadowlayer
    if(shadowLayer_info->bShadowLayer)
    {
        if(shadowLayer_info->bindShadowLayer)
        {
            if(slay->ShadowFlags & SLF_SHADOW_LAYER_BOOLEAN)
            {
                printf("\n[DFB] this layer :%d has been bind as shadowlayer by other layers\n",slay->layer_index);
                fusion_skirmish_dismiss( &sdev->beu_lock );
                return DFB_FAILURE;
            }
            else if(slay->ShadowFlags & SLF_SHADOW_LAYER_INDEXALL)
            {
                printf("\n[DFB] this layer :%d has bind other shadowlayer ,so it can not set as shadowlayer\n", slay->layer_index);
                fusion_skirmish_dismiss(&sdev->beu_lock);
                return DFB_FAILURE;
            }
            else
            {
                slay->ShadowFlags |= SLF_SHADOW_LAYER_BOOLEAN;
                printf("\n[DFB] this layer :%d is set  as shadowlayer\n",slay->layer_index);
            }
       }
       else if(shadowLayer_info->unbindShadowLayer)
       {
            slay->ShadowFlags &= (~SLF_SHADOW_LAYER_BOOLEAN);
            printf("\n[DFB] shadow layer %d SLF_SHADOW_LAYER_BOOLEAN flag is cleared \n",slay->layer_index);
       }

    }
    else  //this layer  will bind /ubind other layers as shadow layer
    {
        hal_phy halPhys = 0;
        if(shadowLayer_info->bindShadowLayer)
        {

            slay->ShadowFlags |= (SLF_SHADOW_LAYER_INDEX0<<shadowLayer_info->shadowLayerIndex);
            halPhys = sdrv->layerFlipInfo[slay->layer_index].CurPhysAddr;
        }
        else if(shadowLayer_info->unbindShadowLayer)
        {
            slay->ShadowFlags &= (~(SLF_SHADOW_LAYER_INDEX0<<shadowLayer_info->shadowLayerIndex));
            halPhys = sdrv->layerFlipInfo[shadowLayer_info->shadowLayerIndex].CurPhysAddr;
        }

        u8 timeouttimes = 0;
        const u8 timeThreshold = 30;
        if(halPhys)
        {
            do
            {
                u16 targetQueueCnt = 1;
                u16 u16QueueCnt = targetQueueCnt;

                //if(MApi_GOP_Switch_GWIN_2_FB(sdev->layer_gwin_id[slay->layer_index], u16FBId, tagID, &u16QueueCnt))
                if(MApi_GOP_Switch_GWIN_2_FB_BY_ADDR(sdev->layer_gwin_id[shadowLayer_info->shadowLayerIndex], halPhys, 0, &u16QueueCnt))
                {
                //printf("MApi_GOP_Switch_GWIN_2_FB_Fusion success , break \n");
                    break;
                }

                timeouttimes++;

                if(u16QueueCnt <= targetQueueCnt-1)
                {
                    printf("[DFB] Serious warning, unknow error!!\n");
                    fusion_skirmish_dismiss( &sdev->beu_lock );
                    return DFB_FAILURE;
                }

            } while(timeouttimes < timeThreshold);

            if(timeouttimes >= timeThreshold)
            {
                int i;
                for(i = 0 ; i < timeouttimes ; i ++)
                    printf("[DFB] [%d]#################### layer=%d , gwin=%d \n",i,slay->layer_index,sdev->layer_gwin_id[slay->layer_index]);
            }
        }

    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

    return ret;
}


DFBResult
mstarGetScreen( CoreLayer               *layer,
                CoreScreen             **ret_screen )
{
    DFBResult ret = DFB_OK;

    MSTARDriverData *sdrv = (MSTARDriverData *)layer->driver_data;
    MSTARDeviceData *sdev = sdrv->dev;
    MSTARLayerData  *slay = layer->layer_data;

    fusion_skirmish_prevail( &sdev->beu_lock );

    switch(slay->gop_dst)
    {
        case E_GOP_DST_IP0:
            *ret_screen = sdrv->ip0_screen;
            break;

        case E_GOP_DST_OP0:
        case E_GOP_DST_BYPASS:
        case DFB_E_GOP_DST_OP_DUAL_RATE:        //Fix me for 4K@120
            *ret_screen = sdrv->op_screen;
            break;

        case E_GOP_DST_VE:
            *ret_screen = sdrv->ve_screen;
            break;

        default:
            break;
    }

    fusion_skirmish_dismiss( &sdev->beu_lock );

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
    ConfigShadowLayer       :   mstar_ConfigShadowLayer,
    ConfigDisplayMode       :   mstar_ConfigDisplayMode,
    SetHVMirrorEnable       :   mstar_SetHVMirrorEnable,
    SetLBCoupleEnable       :   mstar_SetLBCoupleEnable,
    SetGOPDstByPassEnable   :   mstar_SetGOPDstByPassEnable,
    SetHVScale              :   mstar_SetHVScale,
    SetGOPDst               :   mstar_SetGOPDst,
    SetForceWrite           :   mstar_SetForceWrite,
    GetGOPDst               :   mstar_GetGOPDst,
    SetBootLogoPatch        :   mstar_SetBootLogoPatch,
    UpdateRegion            :   mstarUpdateRegion,
    GetScreen               :   mstarGetScreen
};
