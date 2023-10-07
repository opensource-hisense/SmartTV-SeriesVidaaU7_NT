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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>


#define MAX_BUF 1024
#define MYFIFO "/tmp/myfifo"
#define PARAM SDR2HDRParameter

typedef enum {

    HDR_PARAM_NONE = 0x00000000,

    HDR_PARAM_NITS = 0x00000001,
    HDR_PARAM_TMO_SLOPE = 0x00000002,
    HDR_PARAM_TMO_ROLLOFF = 0x00000004,
    HDR_PARAM_TMO_C1 = 0x00000008,
    HDR_PARAM_TMO_C2 = 0x00000010,
    HDR_PARAM_TMO_C3 = 0x00000020,

    HDR_PARAM_ALL = 0x0000003F

}SDR2HDRParamFlags;


typedef struct
{
    unsigned int struct_size;

    /* start from here to declare variables */

    SDR2HDRParamFlags flags;

    float nits;
    float tmo_slope;
    float tmo_rolloff;
    float tmo_c1;
    float tmo_c2;
    float tmo_c3;


}SDR2HDRParameter;



int main( int argc, char *argv[] )
{
    char * myfifo = MYFIFO;
    char buf[MAX_BUF];
    int fd;
    fd_set set;


    if(access(myfifo, F_OK) == -1)
    {
        printf("[write]: Create FIFO\n");
        if(mkfifo(myfifo, 0777))
        {
             perror("mkfifo");
             exit(1);
        }
    }

    /* open, read, and display the message from the FIFO */
    fd = open(myfifo, O_NONBLOCK | O_RDONLY);


    FD_ZERO(&set);
    FD_SET(fd, &set);


    while(1)
    {
        PARAM param = {0};

        select( fd +1, &set, NULL, NULL, NULL);

        if(FD_ISSET(fd, &set))
        {

            if( read(fd, &param, sizeof(PARAM)) > 0)
            {
                /* check input param valid */
                if (param.struct_size != sizeof(PARAM))
                {
                    printf("ERROR!!! API mis-match of param\n");
                    break;
                }

                if (param.flags & HDR_PARAM_NITS)
                    printf("Received: %f\n", param.nits);
            }
            else
                break;
        }

    }

    /* close FIFO */
    close(fd);

    printf("Received: Exit\n");

    return 0;
}
