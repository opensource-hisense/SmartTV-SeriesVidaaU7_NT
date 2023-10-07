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
#include <directfb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "config.h"
#include <dlfcn.h>
#if !USE_MSTAR_MI
#include "apiXC.h"
#include "MsTypes.h"
#endif
#include "gfxdrivers/mstar_gles2/mstar_gles2.h"


#define FLIP_LAYER 0

#define MSTAR_GLES2_SUBMODULE_PATH   "libdirectfb_mstar_gles2.so"

static void* pHandle_gles2 = NULL;

void (*writeToShareData)(SDR2HDRCoeff *coeff) = NULL;

static IDirectFB            *dfb     = NULL;
static IDirectFBSurface  *primary = NULL;


static struct sigaction act;
static int quitpipe[2] = {0};


static void sighandler (int signum, siginfo_t *info, void *ptr)
{
    printf("[CFD_Monitor]: Wake up the main thread.\n");
    write( quitpipe[1], " ", 1 );
}

static void install_handler()
{
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sighandler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, NULL);
}

bool
check_GLES2()
{
    bool ret = true;

    if (pHandle_gles2 == NULL)
    {
        const char     *path = MSTAR_GLES2_SUBMODULE_PATH;

        printf("[DFB] %s, %d, load %s !\n", __FUNCTION__, __LINE__, path);
        pHandle_gles2 = dlopen (path, RTLD_LAZY);

        if (pHandle_gles2)
        {
            writeToShareData = dlsym(pHandle_gles2, "WriteToShareData");

            if (!writeToShareData )
            {
                printf("[DFB] Can't find WriteToShareData : %s\n", dlerror());
                ret = false;
            }

        }
        else
        {
            printf("[DFB] %s, %d, can't find %s !error: %s\n", __FUNCTION__, __LINE__, path, dlerror());
            ret = false;
        }
    }

    return ret;
}

void
exit_application( int status )
{
     /* Release the primary surface. */
     if (primary)
          primary->Release( primary );

     /* Release the super interface. */
     if (dfb)
          dfb->Release( dfb );

     /* Terminate application. */
     exit( status );
}

void
init_application( int *argc, char **argv[] )
{
     DFBResult             ret;
     IDirectFBDisplayLayer  *layer;
     DFBDisplayLayerConfig config;


     /* Initialize DirectFB including command line parsing. */
     ret = DirectFBInit( argc, argv );
     if (ret) {
          DirectFBError( "DirectFBInit() failed", ret );
          exit_application( 1 );
     }

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate() failed", ret );
          exit_application( 2 );
     }

     /* Request fullscreen mode. */
#if  FLIP_LAYER
     ret = dfb->GetDisplayLayer(dfb, DLID_PRIMARY, &layer);
     if (ret)
        DirectFBError( "Failed to get primary layer", ret );

     layer->SetCooperativeLevel(layer, DLSCL_SHARED);

     config.flags = DLCONF_BUFFERMODE;
     config.buffermode = DLBM_BACKVIDEO;
     layer->SetConfiguration(layer, &config);
     layer->GetSurface(layer, &primary);
#endif
}

#if BUILD_CFD_MONITOR

int _get_xc_cfd_osd_process_configs(XC_CFD_OSD_PROCESS_CONFIGS *stCfdOSDConf)
{
    int ret = 0;
    XC_CFD_CONTROL_INFO stXCCFDControlInfo = {0};

    stCfdOSDConf->u32Version = CFD_FIRE_VERSION;
    stCfdOSDConf->u16Length = sizeof(XC_CFD_OSD_PROCESS_CONFIGS);

    stXCCFDControlInfo.enCtrlType = E_XC_CFD_CTRL_GET_OSD_PROCESS_CONFIGS;
    stXCCFDControlInfo.pParam = stCfdOSDConf;
    stXCCFDControlInfo.u32ParamLen = stCfdOSDConf->u16Length;
    ret = MApi_XC_HDR_Control(E_XC_HDR_CTRL_CFD_CONTROL, &stXCCFDControlInfo);

    return ret;
}

void print_stu_cfdapi_osd_process_config(XC_CFD_OSD_PROCESS_CONFIGS *pt_alg)
{
    printf("u32Version = %u\n", pt_alg->u32Version);
    printf("u16Length = %u\n", pt_alg->u16Length);
    printf("u8OSD_SDR2HDR_en = %u, ", pt_alg->u8OSD_SDR2HDR_en);
    printf("u8OSD_IsFullRange = %u, ", pt_alg->u8OSD_IsFullRange);
    printf("u8OSD_Dataformat = %u, ", pt_alg->u8OSD_Dataformat);
    printf("u8OSD_HDRMode = %u, ", pt_alg->u8OSD_HDRMode);
    printf("u8OSD_colorprimary = %u, ", pt_alg->u8OSD_colorprimary);
    printf("u8OSD_transferf = %u, ", pt_alg->u8OSD_transferf);
    printf("u16OSD_MaxLumInNits = %u, ", pt_alg->u16OSD_MaxLumInNits);
    printf("u8Video_colorprimary = %u, ", pt_alg->u8Video_colorprimary);
    printf("u8Video_MatrixCoefficients = %u, ", pt_alg->u8Video_MatrixCoefficients);
    printf("u8Video_HDRMode = %u, ", pt_alg->u8Video_HDRMode);
    printf("u16Video_MaxLumInNits = %u, ", pt_alg->u16Video_MaxLumInNits);
    printf("u8Video_IsFullRange = %u, ", pt_alg->u8Video_IsFullRange);
    printf("u8Video_Dataformat = %u, ", pt_alg->u8Video_Dataformat);
    printf("u16AntiTMO_SourceInNits = %u\n", pt_alg->u16AntiTMO_SourceInNits);

}

#endif

void CFD_Monitor_Thread()
{
    SDR2HDRCoeff param = {0};
#if BUILD_CFD_MONITOR
    XC_CFD_OSD_PROCESS_CONFIGS curt_osd_config = {0};
#endif
    int prev_SDR2HDR_en = 0;
    float prev_nits = 0;
    int ret = 0;
    int sleep_time = 20 * 1000; /* 20 ms */

    fd_set set;
    int status;

    /* init pipe & install signal handler for terminating the program if parent sent SIGINT */
    pipe( quitpipe );
    install_handler();


    printf("[CFD_Monitor] init_application\n");
    init_application( NULL, NULL );

    if ( !check_GLES2() )
        exit_application(0);

    /* assign defaults */
    param.struct_size = sizeof(SDR2HDRCoeff); /* checked by server side to make sure the client & server uses the "same" type of parameters */

    param.en_gles2 = false;
    param.en_sdr2hdr = false;

    param.nits = 100.0f;

    // get ENV "CFDMonitor_Time"
    {
    char *time;
    if ( (time = getenv("CFDMonitor_Time")) )
        sleep_time = atoi(time);
    }

#if BUILD_CFD_MONITOR
    while(1)
    {
        ret = _get_xc_cfd_osd_process_configs( &curt_osd_config );
        if ( ret != 1) {
            printf("\033[1;31m[%s] Can not get CFD configs!\033[0m\n", __FILE__);
            break;
        }
        //print_stu_cfdapi_osd_process_config( &curt_osd_config );

        if ( curt_osd_config.u8OSD_SDR2HDR_en )
        {
            param.nits = curt_osd_config.u16AntiTMO_SourceInNits;

            param.en_gles2 = param.en_sdr2hdr = curt_osd_config.u8OSD_SDR2HDR_en;

            //param.en_gles2 = param.en_sdr2hdr = (param.nits > 0)? 1 : 0;

            if (param.en_gles2 != prev_SDR2HDR_en || param.nits != prev_nits)
            {
                printf("\033[1;31m[%s] en_gles2 = %d, en_sdr2hdr = %d, nits = %f\033[0m\n", __FILE__, param.en_gles2, param.en_sdr2hdr, param.nits);
                prev_SDR2HDR_en = param.en_gles2;
                prev_nits = param.nits;

                if ( writeToShareData )
                    writeToShareData( &param );

#if FLIP_LAYER
                /* update the display by the latest parameters */
                primary->Flip( primary, NULL, 0 );
#endif
            }
        }

        /* wait for an interval sleep_time */
        FD_ZERO( &set );
        FD_SET( quitpipe[0], &set );
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_sec = sleep_time;
        status = select( quitpipe[0] +1, &set, NULL, NULL, &timeout );

        if (status < 0 && errno != EINTR)
        {
            int err = errno;
            printf("[CFD_Monitor]: %s  errno: %d\n", __FUNCTION__, err);
            break;
        }

        /* break the while loop */
        if (status > 0 && FD_ISSET( quitpipe[0], &set ))
            break;

    }

#else

    printf("[CFD_Monitor]: Error!!! CFD monitor does not exist, please rebuild this program. (%s, %s)\n", __FILE__, __FUNCTION__);
    printf("[CFD_Monitor]: Please add --enable-CFDMonitor in build script to build with CFD function\n");
#endif

    /* close pipe */
    close( quitpipe[0] );
    close( quitpipe[1] );


    printf("[CFD Monitor] exit application!\n");
    exit_application(0);

}

int main( int argc, char *argv[] )
{
    pthread_t id = -1;

    printf("[CFD_Monitor]: Program Start\n");

    CFD_Monitor_Thread();

    printf("[CFD_Monitor]: Program Exit\n");

    return 0;
}
