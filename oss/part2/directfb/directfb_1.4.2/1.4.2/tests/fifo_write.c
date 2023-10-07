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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#define MYFIFO "/tmp/fifo_sdr2hdr"
#define PARAM SDR2HDRParameter

//#define FLIP_LAYER

typedef struct
{
    unsigned int struct_size;

    /* start from here to declare variables */

    bool en_gles2;
    bool en_sdr2hdr;

    int  rgb2yuv;
    float nits;
    float tmo_slope;
    float tmo_rolloff;
    float tmo_c1;
    float tmo_c2;
    float tmo_c3;


}SDR2HDRParameter;


static IDirectFB            *dfb     = NULL;
static IDirectFBSurface  *primary = NULL;

static void
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

static void
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
#ifdef  FLIP_LAYER
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

void FIFO_Thread()
{
    int fd;
    char * myfifo = MYFIFO;
    int count = 0;
    PARAM param = {0};

    printf("init_application\n");
    init_application( NULL, NULL );


    /* assign defaults */
    param.struct_size = sizeof(PARAM); /* checked by server side to make sure the client & server uses the "same" type of parameters */

    param.en_gles2 = false;
    param.en_sdr2hdr = false;

    param.rgb2yuv = 0;
    param.nits = 100.0f;
    param.tmo_slope = 3.0f;
    param.tmo_rolloff = 0.5f;
    param.tmo_c1 = 0.00010592f;
    param.tmo_c2 = 1.7192f;
    param.tmo_c3 = -5.5102f;


    /* create the FIFO (named pipe) */

    printf("[write]: Accessing FIFO\n");
    if(access(myfifo, F_OK) == -1)
    {
        printf("[write]: Create FIFO\n");
        if(mkfifo(myfifo, 0666))
        {
            perror("mkfifo");
            exit(1);
        }
    }

    printf("[write]: Opening FIFO\n");

    fd = open(myfifo, O_WRONLY);

    printf("[write]: Start sending data...\n");

    while(1)
    {
        printf("[write]: Wrote: %d, R2Y = %d\n", count, param.rgb2yuv);

        /* write "param" to the FIFO */
        param.en_gles2 = ((count/10)%3 != 0)? true : false;
        param.en_sdr2hdr = false;//((count/10)%3 == 2)? true : false;
        param.rgb2yuv = (param.rgb2yuv == 0)? 1 : 0; /* change some parameter if necessary */
        write(fd, &param, sizeof(PARAM));

#ifdef FLIP_LAYER
        /* update the display by the latest parameters */
        primary->Flip( primary, NULL, 0 );
#endif

        if(count++ < 100)
        {
            sleep(1);
        }
        else
        {
            count = 0;
            printf("[write]: Wait 10 sec for the next round start...\n\n");
            sleep(10);
            break;
        }
    }

    printf("exit_application!\n");

    exit_application(0);

    /* close FIFO */
    close(fd);

    /* remove the FIFO */
    unlink(myfifo);
}

int main( int argc, char *argv[] )
{
    pthread_t id = -1;

    printf("[write]: Program Start\n");

    if(pthread_create(&id, NULL, (void*)FIFO_Thread, NULL) != 0)
    {
        printf("[write] Create thread failed, exit program\n");
        exit(1);
    }

    printf("[write]: Program Wait\n");

    pthread_join(id, NULL);

    printf("[write]: Program Exit\n");

    return 0;
}
