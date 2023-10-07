/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/* FILE NAME:  bt_mw_common.c
 * PURPOSE:
 *      It provides bluetooth common structure to MW.
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//#include <assert.h>
#include <time.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include "bt_mw_common.h"
#include "bt_mw_a2dp_common.h"
#include "bt_mw_a2dp_snk.h"
#include "bt_mw_avrcp.h"
#include "bt_mw_gatts.h"


/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */

typedef VOID (*BT_MW_DBG_FUNC)(void *cntx, char *cmd);
typedef int (*BT_MW_DBG_INIT_FUNC)(void);

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
BT_MW_DBG_FUNC s_bt_mw_dbg_func = NULL;
BT_MW_DBG_INIT_FUNC s_bt_mw_dbg_init = NULL;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FILE NAME:  bt_mw_common.c
 * AUTHOR: Changlu Yi
 * PURPOSE:
 * NOTES:
 */
VOID bluetooth_uuid_stos(CHAR *uuid_s,  CHAR *uuid)
{
    UINT32 i = 0,j = 0;
    INT32 size = 0;
    CHAR  temp[3];
    UINT8 temp_uuid[BT_GATT_UUID_ARRAY_SIZE] = {0};
    temp[2] = '\0';

    if ((NULL == uuid_s) || (NULL == uuid) || (0 == strlen(uuid_s)))
    {
        return;
    }

    size = strlen(uuid_s);
    while (i < size)
    {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0')
        {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        temp_uuid[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }
    if (size <= 8)   // 16bits uuid or 32bits uuid
    {
        if (size == 4)
        {
            temp_uuid[2] = temp_uuid[0];
            temp_uuid[3] = temp_uuid[1];
            temp_uuid[0] = 0;
            temp_uuid[1] = 0;
        }
        temp_uuid[4] = 0x00;
        temp_uuid[5] = 0x00;
        temp_uuid[6] = 0x10;
        temp_uuid[7] = 0x00;

        temp_uuid[8] = 0x80;
        temp_uuid[9] = 0x00;
        temp_uuid[10] = 0x00;
        temp_uuid[11] = 0x80;

        temp_uuid[12] = 0x5F;
        temp_uuid[13] = 0x9B;
        temp_uuid[14] = 0x34;
        temp_uuid[15] = 0xFB;
    }


    CHAR *ptr;
    ptr = uuid;
    for (i = 0 ; i < BT_GATT_UUID_ARRAY_SIZE ; i++)
    {
        (void)sprintf(ptr, "%02X", temp_uuid[i]);
        ptr+=2;
        if (i == 3 || i == 5 || i == 7 || i == 9)
        {
            *ptr = '-';
            ptr++;
        }
    }
    *ptr = '\0';
    return;
}

VOID bluetooth_uuid_stoh(CHAR *uuid_s,  CHAR *uuid)
{
    UINT32 i = 0,j = 0;
    INT32 size = 0;
    CHAR  temp[3];
    temp[2] = '\0';
    UINT8 temp_uuid[BT_GATT_UUID_ARRAY_SIZE] = {0};
    if (NULL == uuid_s || NULL == uuid)
    {
        return;
    }

    size = strlen(uuid_s);
    while (i < size)
    {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0')
        {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        temp_uuid[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }

    if (size <= 8)   // 16bits uuid or 32bits uuid
    {
        if (size == 4)
        {
            temp_uuid[2] = temp_uuid[0];
            temp_uuid[3] = temp_uuid[1];
            temp_uuid[0] = 0;
            temp_uuid[1] = 0;
        }
        temp_uuid[4] = 0x00;
        temp_uuid[5] = 0x00;
        temp_uuid[6] = 0x10;
        temp_uuid[7] = 0x00;

        temp_uuid[8] = 0x80;
        temp_uuid[9] = 0x00;
        temp_uuid[10] = 0x00;
        temp_uuid[11] = 0x80;

        temp_uuid[12] = 0x5F;
        temp_uuid[13] = 0x9B;
        temp_uuid[14] = 0x34;
        temp_uuid[15] = 0xFB;
    }

    for (i = 0; i < BT_GATT_UUID_ARRAY_SIZE; i++)
    {
        uuid[BT_GATT_UUID_ARRAY_SIZE - 1 - i] = temp_uuid[i];
    }
    return;
}

#if 0

VOID bt_mw_dbg_load_dbg_func(void)
{
    void *dlhandle = NULL;

    dlhandle = dlopen("libbtmw-test.so", RTLD_LAZY);
    if (!dlhandle)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "libbtmw-test.so open fail(%s)", dlerror());
        return;
    }
    s_bt_mw_dbg_func = dlsym(dlhandle, "btmw_test_edit_cmd_cb");
    if (dlerror() != NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "libbtmw-test.so get symbol fail(%s)", dlerror());
        dlclose(dlhandle);
        s_bt_mw_dbg_func = NULL;
        return;
    }

    s_bt_mw_dbg_init = dlsym(dlhandle, "btmw_test_cli_init");
    if (dlerror() != NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "libbtmw-test.so get symbol fail(%s)", dlerror());
        dlclose(dlhandle);
        s_bt_mw_dbg_func = NULL;
        s_bt_mw_dbg_init = NULL;
        return;
    }

    s_bt_mw_dbg_init();
    dlclose(dlhandle);

    return;
}

VOID bt_mw_dbg_handle_cmd(char *cmd_str)
{
    BT_CHECK_POINTER_RETURN(BT_DEBUG_COMM, cmd_str);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "cmd_str=%s", cmd_str);

    if (0 == strncmp(cmd_str, "dump", 4))
    {
        if (strstr(cmd_str, "a2dp") != NULL)
        {
            bt_mw_a2dp_dump_dev_info();
            bt_mw_avrcp_dump_info();
        }

        if (strstr(cmd_str, "gatt") != NULL)
        {
            bt_mw_gatt_dump_info();
        }

        if (strstr(cmd_str, "pcm") != NULL)
        {
            char *ptr = NULL;
            if ((ptr = strstr(cmd_str, "start")) != NULL)
            {
                bt_mw_a2dp_sink_dump_pcm_start(ptr+6);
            }
            else if (strstr(cmd_str, "stop") != NULL)
            {
                bt_mw_a2dp_sink_dump_pcm_stop();
            }
        }

        if (strstr(cmd_str, "player") != NULL)
        {
            char *ptr = NULL;
            if ((ptr = strstr(cmd_str, "pcm_start")) != NULL)
            {
                //bt_mw_a2dp_sink_dump(BT_A2DP_SINK_DUMP_PLAYER_PCM_START);
            }
            else if (strstr(cmd_str, "pcm_stop") != NULL)
            {
                //bt_mw_a2dp_sink_dump(BT_A2DP_SINK_DUMP_PLAYER_PCM_STOP);
            }
            else if (strstr(cmd_str, "stat") != NULL)
            {
                //bt_mw_a2dp_sink_dump(BT_A2DP_SINK_DUMP_PLAYER_STAT);
            }
            else if (strstr(cmd_str, "stat2file") != NULL)
            {
                //bt_mw_a2dp_sink_dump(BT_A2DP_SINK_DUMP_PLAYER_STAT_2_FILE);
            }
        }
    }
    else if(0 == strncmp(cmd_str, "dbg", 3))
    {
        if (s_bt_mw_dbg_func != NULL && strlen(&cmd_str[4]) > 0)
        {
            BT_DBG_NORMAL(BT_DEBUG_COMM, "dbg_str=%s\n", &cmd_str[4]);
            s_bt_mw_dbg_func(NULL, &cmd_str[4]);
        }
        else
        {
            BT_DBG_NORMAL(BT_DEBUG_COMM, "dbg_str is empty\n");
        }
    }

    return;
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */






