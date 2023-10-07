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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_avrcp_if.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_leaudio_bms_wrapper.h"

static BTMW_RPC_TEST_CLI btmw_rpc_test_rc_ct_cli_commands[];
#define BTMW_RPC_TEST_CMD_KEY_AVRCP_CT     "MW_RPC_AVRCP_CT"

static CHAR g_avrcp_addr_test[18];
static UINT32 g_avrcp_reg_event = 0;

int stream_state = -1;
static INT32 atuo_avrcp_ct_test_cnt = 0;
static INT32 atuo_avrcp_ct_next_test_cnt = 0;
static INT32 atuo_avrcp_ct_connecet_thread_exit = 0;
static pthread_t g_a2dp_auto_avrcp_ct_test_thread = 0;
static INT32 cmd = 0;
static INT32 cmd_test_cnt = 0;

static INT32 track_change = -1;

#define BTMW_RPC_AVRCP_CASE_RETURN_STR(const) case const: return #const;


int btmw_rpc_test_rc_ct_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_RPC_TEST_Logi("[AVRCP] CT argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = btmw_rpc_test_rc_ct_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_AVRCP_CT, btmw_rpc_test_rc_ct_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static int btmw_rpc_test_rc_send_play_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);

    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_rpc_test_rc_auto_send_cmd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }
    cmd = atoi(argv[2]); // 1: pause ;2: stop 3. next
    if (cmd == 1 || cmd ==2)
    {
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);

        usleep(interval);
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);
    }
    else if (cmd ==3)
    {
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);

        usleep(interval);
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
    }

    return 0;
}


static void* gattc_a2dp_auto_avrcp_test_thread(void *arg)
{
    while (1)
    {
        usleep(3000*1000);
        if (TRUE == atuo_avrcp_ct_connecet_thread_exit)
        {
            BTMW_RPC_TEST_Loge("disable src exit thread!");
            break;
        }
        if (stream_state == AVRCP_PLAY_STATUS_PLAYING)
        {
            stream_state = -1;
            if (cmd == 1)
            {
                BTMW_RPC_TEST_Loge("send pause in thread!");
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);
                usleep(1000*10);
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);
            }
            else if (cmd ==2)
            {
                BTMW_RPC_TEST_Loge("send stop in thread!");
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_PRESS);
                usleep(1000*10);
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_RELEASE);
            }
            else if (cmd ==3 && track_change == 1)
            {
                usleep(4000*1000);
                track_change = -1;
                atuo_avrcp_ct_next_test_cnt++;
                if (atuo_avrcp_ct_next_test_cnt < 1000)
                {
                    BTMW_RPC_TEST_Loge("send NEXT in thread!");
                    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);
                        usleep(1000*10);
                    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                        BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
                }
                else
                {
                     BTMW_RPC_TEST_Loge("auto test connect success!");
                     break;
                }

            }
        }
        else if (stream_state == (int)AVRCP_PLAY_STATUS_STOPPED
            || stream_state == (int)AVRCP_PLAY_STATUS_PAUSED)
        {
            stream_state = -1;
            atuo_avrcp_ct_test_cnt++;
            if (atuo_avrcp_ct_test_cnt < 1000)
            {
                BTMW_RPC_TEST_Loge("send play in thread!");
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);
                usleep(1000*10);
                a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);
            }
            else
            {
                BTMW_RPC_TEST_Loge("auto test connect success!");
                break;
            }
        }
    };
    g_a2dp_auto_avrcp_ct_test_thread = -1;
    atuo_avrcp_ct_connecet_thread_exit = TRUE;
    return NULL;
}

static int btmw_rpc_test_rc_auto_send_play_handler(int argc, char *argv[])
{
    INT32 i4_ret = 0;
    INT32 interval = 0;

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }
    cmd = atoi(argv[2]); // 1: pause ;2: stop

    BTMW_RPC_TEST_Logd("%s()\n", __func__);
    atuo_avrcp_ct_connecet_thread_exit = 0;

    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BTMW_RPC_TEST_Loge("[avrcp] pthread_attr_init i4_ret:%d\n", i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&g_a2dp_auto_avrcp_ct_test_thread,
                                          &attr,
                                          gattc_a2dp_auto_avrcp_test_thread,
                                          NULL)))
        {
            //BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_create i4_ret:%ld", (long)i4_ret);
            BTMW_RPC_TEST_Logi("[GATTC] pthread_create i4_ret:%d\n", i4_ret);
        }
    }
    else
    {
        //BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
        BTMW_RPC_TEST_Logi("[GATTC] pthread_attr_setdetachstate i4_ret:%d\n", i4_ret);
    }

    pthread_attr_destroy(&attr);
    if (argc != 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
     if (cmd == 1 || cmd ==2)
     {
         BTMW_RPC_TEST_Loge("[USAGE] call send play");
         a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);
         usleep(1000*10);
         a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);
     }
     else if (cmd == 3)
     {
         BTMW_RPC_TEST_Loge("[USAGE] call send next");
         a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);
         usleep(1000*10);
         a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test,
                    BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
     }

    return 0;
}

static int btmw_rpc_test_rc_send_pause_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_rpc_test_rc_send_stop_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_rpc_test_rc_send_fwd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_bwd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_BWD, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_BWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_ffwd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FFWD, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FFWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_rwd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_RWD, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_RWD, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_rpc_test_rc_send_volumeup_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_UP, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_UP, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_volumedown_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_DOWN, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_DOWN, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_next_group_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 0, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 0, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_rpc_test_rc_send_prev_group_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 2)
    {
        interval = 1000 * atoi(argv[1]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 1, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 1, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}


static int btmw_rpc_test_rc_set_volume_cmd_handler(int argc, char *argv[])
{
    UINT8 u1volume = 0;
    u1volume = atoi(argv[0]);

    if (argc >= 2)
    {
        strncpy(g_avrcp_addr_test, argv[1], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    BTMW_RPC_TEST_Logi("[AVRCP] %s() volume=%d\n", __func__, u1volume);
    if (BT_ERR_STATUS_UNSUPPORTED ==
        a_mtkapi_avrcp_change_volume(g_avrcp_addr_test, u1volume))
    {
        BTMW_RPC_TEST_Loge("[AVRCP] not support absolute volume\n", __func__);
    }
    return 0;
}

static int btmw_rpc_test_rc_player_setting_cmd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);

    BT_AVRCP_PLAYER_SETTING player_setting;
    memset(&player_setting, 0x0, sizeof(BT_AVRCP_PLAYER_SETTING));

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }

    if (argc >= 2)
    {
        player_setting.num_attr = argc - 1;
        for (int ii = 1; ii < argc; ii++)
            {
               char *p_temp = strtok(argv[ii], "=");
               BTMW_RPC_TEST_Logi("p_temp=%s", p_temp);
               if (p_temp)
                {
                  if(strcmp(p_temp, "equalizer") == 0)
                    {
                      player_setting.attr_ids[ii-1] = 0x01;
                    }
                  else if(strcmp(p_temp, "repeat") == 0)
                    {
                      player_setting.attr_ids[ii-1] = 0x02;
                    }
                  else if(strcmp(p_temp, "shuffle") == 0)
                    {
                      player_setting.attr_ids[ii-1] = 0x03;
                    }
                  else if(strcmp(p_temp, "scan") == 0)
                    {
                      player_setting.attr_ids[ii-1] = 0x04;
                    }
                }
              else
                {
                  BTMW_RPC_TEST_Logi("attribute id is empty\n");
                  continue;
                }
               char *p_value = strtok(NULL, "=");
               BTMW_RPC_TEST_Logi("p_value=%s", p_value);
               if(p_value)
                {
                  player_setting.attr_values[ii-1] = atoi(p_value);
                }
               else
                {
                  BTMW_RPC_TEST_Logi("attribute value is empty\n");
                  continue;
                }
            }

        BTMW_RPC_TEST_Logi("argc:%ld  num_attr:%d\n", argc, player_setting.num_attr);
    }
    else
    {
        BTMW_RPC_TEST_Logi("player_setting attribute is empty!\n");
        return 0;
    }
    a_mtkapi_avrcp_change_player_app_setting(g_avrcp_addr_test, &player_setting);
    return 0;
}

static int btmw_rpc_test_rc_send_key_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT32 interval = 1000000;
    BT_AVRCP_CMD_TYPE cmd_type = BT_AVRCP_CMD_TYPE_PLAY;

    if (0 == strcmp("chn_up", argv[0]))
    {
        cmd_type = BT_AVRCP_CMD_TYPE_CHN_UP;
    }
    else if (0 == strcmp("chn_down", argv[0]))
    {
        cmd_type = BT_AVRCP_CMD_TYPE_CHN_DOWN;
    }
    else if (0 == strcmp("mute", argv[0]))
    {
        cmd_type = BT_AVRCP_CMD_TYPE_MUTE;
    }
    else if (0 == strcmp("power", argv[0]))
    {
        cmd_type = BT_AVRCP_CMD_TYPE_POWER;
    }
    else
    {
        BTMW_RPC_TEST_Logi("bad key %s\n", argv[0]);
        return 0;
    }

    if (argc >= 2)
    {
        strncpy(g_avrcp_addr_test, argv[1], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc >= 3)
    {
        interval = 1000 * atoi(argv[2]);
        BTMW_RPC_TEST_Logi("interval:%ld us\n", interval);
    }

    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, cmd_type, BT_AVRCP_KEY_STATE_PRESS);
    usleep(interval);
    a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, cmd_type, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_rpc_test_rc_playitem_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT8 count = 0;
    BT_AVRCP_PLAYITEM playitem;
    memset(&playitem, 0, sizeof(BT_AVRCP_PLAYITEM));

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    if (argc < 7)
    {
        BTMW_RPC_TEST_Loge("[USAGE] playitem <addr> uid <1-10> scope <0-3> uid_counter <1-0xffff>");
        return -1;
    }
    count++;
    while (count < argc - 1)
    {
        BTMW_RPC_TEST_Logi("%s parser cli\n", __func__);
        if (strcmp(argv[count], "uid") == 0)
        {
            count++;
            playitem.uid = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("%s uid =%d\n", __func__, playitem.uid);
            continue;
        }
        else if(strcmp(argv[count], "scope") == 0)
        {
            count++;
            playitem.scope = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("%s scope =%d\n", __func__, playitem.scope);
            continue;
        }
        else if(strcmp(argv[count], "uid_counter") == 0)
        {
            count++;
            playitem.uid_counter = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("%s uid_counter =%d\n", __func__, playitem.uid_counter);
            continue;
        }
    }
    a_mtkapi_avrcp_send_playitem(g_avrcp_addr_test, &playitem);

    return 0;
}

static int btmw_rpc_test_rc_get_playback_state_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    BT_AVRCP_PLAYITEM playitem;
    memset(&playitem, 0, sizeof(BT_AVRCP_PLAYITEM));

    if (argc >= 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_playback_state <addr>");
        return -1;
    }

    if (argc >= 1)
    {
        strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
        g_avrcp_addr_test[17] = '\0';
    }
    a_mtkapi_avrcp_get_playback_state(g_avrcp_addr_test);

    return 0;
}

static int btmw_rpc_test_rc_get_nowplaying_list_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    BT_AVRCP_LIST_RANGE list_range;
    memset(&list_range, 0, sizeof(BT_AVRCP_LIST_RANGE));

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_nowplaying_list <addr> start <uint8> end <uint8>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "start") == 0)
    {
        list_range.start = atoi(argv[2]);
    }
    if (strcmp(argv[3], "end") == 0)
    {
        list_range.end = atoi(argv[4]);
    }

    BTMW_RPC_TEST_Logi("%s start =%d, end=%d\n", __func__, list_range.start, list_range.end);
    a_mtkapi_avrcp_get_now_playing_list(g_avrcp_addr_test, &list_range);

    return 0;
}

static int btmw_rpc_test_rc_get_folder_list_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    BT_AVRCP_LIST_RANGE list_range;
    memset(&list_range, 0, sizeof(BT_AVRCP_LIST_RANGE));

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_playback_state <addr>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "start") == 0)
    {
        list_range.start = atoi(argv[2]);
    }
    if (strcmp(argv[3], "end") == 0)
    {
        list_range.end = atoi(argv[4]);
    }
    BTMW_RPC_TEST_Logi("%s start =%d, end=%d\n", __func__, list_range.start, list_range.end);
    a_mtkapi_avrcp_get_folder_list(g_avrcp_addr_test, &list_range);

    return 0;
}

static int btmw_rpc_test_rc_get_player_list_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    BT_AVRCP_LIST_RANGE list_range;
    memset(&list_range, 0, sizeof(BT_AVRCP_LIST_RANGE));

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_playback_state <addr>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "start") == 0)
    {
        list_range.start = atoi(argv[2]);
    }
    if (strcmp(argv[3], "end") == 0)
    {
        list_range.end = atoi(argv[4]);
    }
    BTMW_RPC_TEST_Logi("%s start =%d, end=%d\n", __func__, list_range.start, list_range.end);
    a_mtkapi_avrcp_get_player_list(g_avrcp_addr_test, &list_range);

    return 0;
}

static int btmw_rpc_test_rc_change_folder_path_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    BT_AVRCP_FOLDER_PATH folder_path;
    memset(&folder_path, 0, sizeof(BT_AVRCP_FOLDER_PATH));

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_playback_state <addr>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "direction") == 0)
    {
        folder_path.folder_direction = atoi(argv[2]);
    }
    if (strcmp(argv[3], "uid") == 0)
    {
        folder_path.uid = (UINT64)(atoi(argv[4]));
    }

    BTMW_RPC_TEST_Logi("%s start =%d, uid=%lld\n", __func__, folder_path.folder_direction, folder_path.uid);
    a_mtkapi_avrcp_change_folder_path(g_avrcp_addr_test, &folder_path);

    return 0;
}

static int btmw_rpc_test_rc_set_browsed_player_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT16 player_id = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_browsed_player <addr> <playerid:0-255>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "player_id") == 0)
    {
        player_id = atoi(argv[2]);
    }

    BTMW_RPC_TEST_Logi("%s player_id =%d\n", __func__, player_id);
    a_mtkapi_avrcp_set_browsed_player(g_avrcp_addr_test, player_id);
    return 0;
}

static int btmw_rpc_test_rc_set_addressed_player_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    UINT16 player_id = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] set_address_player <addr> <playerid:0-255>");
        return -1;
    }

    strncpy(g_avrcp_addr_test, argv[0], sizeof(g_avrcp_addr_test));
    g_avrcp_addr_test[17] = '\0';

    if (strcmp(argv[1], "player_id") == 0)
    {
        player_id = atoi(argv[2]);
    }

    BTMW_RPC_TEST_Logi("%s player_id =%d\n", __func__, player_id);
    a_mtkapi_avrcp_set_addressed_player(g_avrcp_addr_test, player_id);
    return 0;
}

static int btmw_rpc_test_rc_get_connected_dev_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[AVRCP] %s() \n", __func__);
    int i = 0;
    BT_AVRCP_CONNECTED_DEV_INFO_LIST dev_list;
    memset(&dev_list, 0, sizeof(BT_AVRCP_CONNECTED_DEV_INFO_LIST));

    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("[USAGE] get_connected_dev no param");
        return -1;
    }

    a_mtkapi_avrcp_get_connected_dev_list(&dev_list);
    BTMW_RPC_TEST_Logi("[AVRCP] %s() dev_num=%d\n", __func__, dev_list.dev_num);
    for (i = 0; i < dev_list.dev_num; i++)
    {
        BTMW_RPC_TEST_Logi("[AVRCP] dec addr=%s\n",
            dev_list.avrcp_connected_dev_list[i].addr);
        BTMW_RPC_TEST_Logi("[AVRCP] media artist=%s\n",
            dev_list.avrcp_connected_dev_list[i].element_attr.artist);
        BTMW_RPC_TEST_Logi("[AVRCP] media title=%s\n",
            dev_list.avrcp_connected_dev_list[i].element_attr.title);
        BTMW_RPC_TEST_Logi("[AVRCP] media title=0x%2x-0x%2x-0x%2x\n",
            dev_list.avrcp_connected_dev_list[i].element_attr.title[0],
            dev_list.avrcp_connected_dev_list[i].element_attr.title[1],
            dev_list.avrcp_connected_dev_list[i].element_attr.title[2]);
    }
    return 0;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_rc_ct_cli_commands[] =
{
    {"play",             btmw_rpc_test_rc_send_play_handler,             " play <addr> [interval(ms)]"},
    {"auto_play",        btmw_rpc_test_rc_auto_send_play_handler,        " auto_play <addr> [interval(ms)] <cmd:1-3>"},
    {"auto_cmd",         btmw_rpc_test_rc_auto_send_cmd_handler,        " auto_cmd <addr> [interval(ms)] <cmd:1-3>"},
    {"pause",            btmw_rpc_test_rc_send_pause_handler,            " pause <addr> [interval(ms)]"},
    {"stop",             btmw_rpc_test_rc_send_stop_handler,             " stop <addr> [interval(ms)]"},
    {"fwd",              btmw_rpc_test_rc_send_fwd_handler,              " fwd <addr> [interval(ms)]"},
    {"bwd",              btmw_rpc_test_rc_send_bwd_handler,              " bwd <addr> [interval(ms)]"},
    {"ffwd",             btmw_rpc_test_rc_send_ffwd_handler,             " ffwd <addr> [interval(ms)]"},
    {"rwd",              btmw_rpc_test_rc_send_rwd_handler,              " rwd <addr> [interval(ms)]"},
    {"volume_up",        btmw_rpc_test_rc_send_volumeup_handler,         " volume_up <addr> [interval(ms)]"},
    {"volume_down",      btmw_rpc_test_rc_send_volumedown_handler,       " volume_down <addr> [interval(ms)]"},
    {"next_group",       btmw_rpc_test_rc_send_next_group_handler,       " next_group <addr> [interval(ms)]"},
    {"prev_group",       btmw_rpc_test_rc_send_prev_group_handler,       " prev_group <addr> [interval(ms)]"},
    {"set_volume",       btmw_rpc_test_rc_set_volume_cmd_handler,        " set_volume <volume:0~100%> <addr>"},
    {"player_setting",   btmw_rpc_test_rc_player_setting_cmd_handler,    " player_setting <addr> equalizer=[1-2]/repeat=[1-4]/shuffle=[1-3]/scan=[1-3]"},
    {"key",              btmw_rpc_test_rc_send_key_handler,              " key <chn_up|chn_down|mute|power> <addr> [interval(ms)]"},
    {"playitem",         btmw_rpc_test_rc_playitem_handler,              " playitem <addr> uid <1-10> scope <0-3> uid_counter <1-0xffff>"},
    {"get_playback_state", btmw_rpc_test_rc_get_playback_state_handler,  " get_playback_state <addr>"},
    {"get_now_playing_list", btmw_rpc_test_rc_get_nowplaying_list_handler,  " get_now_playing_list <addr> <start> <end>"},
    {"get_folder_list", btmw_rpc_test_rc_get_folder_list_handler,         " get_folder_list <addr> <start> <end>"},
    {"get_player_list", btmw_rpc_test_rc_get_player_list_handler,         " get_player_list <addr> <start> <end>"},
    {"change_folder_path", btmw_rpc_test_rc_change_folder_path_handler,   " change_folder_path <addr> <direction:0-1> <uid:0-0xffffffff>"},
    {"set_browsed_player", btmw_rpc_test_rc_set_browsed_player_handler,   " set_browsed_player <addr> <playerid:0-255>"},
    {"set_addressed_player", btmw_rpc_test_rc_set_addressed_player_handler,   " set_addressed_player <addr> <playerid:0-255>"},
    {"get_connected_dev", btmw_rpc_test_rc_get_connected_dev_handler,   " get_connected_dev"},
    {NULL, NULL, NULL},
};

static CHAR* btmw_rpc_test_avrcp_app_event(BT_AVRCP_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CONNECTED)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_DISCONNECTED)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_TRACK_CHANGE)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAYER_SETTING_RSP)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_LIST_PLAYER_SETTING_RSP)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_POS_CHANGE)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAY_STATUS_CHANGE)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_VOLUME_CHANGE)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_SET_VOLUME_REQ)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ)
        BTMW_RPC_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_ABS_SUPPORT)
        default: return "UNKNOWN_AVRCP_EVENT";
   }
}

static VOID btmw_rpc_test_avrcp_show_track_change(BT_AVRCP_TRACK_CHANGE *track_change)
{
    if (cmd ==3)
    {
        cmd_test_cnt++;
        if (cmd_test_cnt > 1000)
        {
            BTMW_RPC_TEST_Logd("auto cmd success(%u)\n");
            return;
        }
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);

        usleep(10*1000);
        a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
    }
    BTMW_RPC_TEST_Logd("title(%s)\n",
        track_change->element_attr.title);
    BTMW_RPC_TEST_Logd("artist(%s)\n",
        track_change->element_attr.artist);
    BTMW_RPC_TEST_Logd("album(%s)\n",
        track_change->element_attr.album);
    BTMW_RPC_TEST_Logd("current_track_number(%u)\n",
        track_change->element_attr.current_track_number);
    BTMW_RPC_TEST_Logd("number_of_tracks(%u)\n",
        track_change->element_attr.number_of_tracks);
    BTMW_RPC_TEST_Logd("genre(%s)\n",
        track_change->element_attr.genre);
    BTMW_RPC_TEST_Logd("position(%u)\n",
        track_change->element_attr.position);
    return;
}

static VOID btmw_rpc_test_avrcp_show_player_setting_change(BT_AVRCP_PLAYERSETTING_CHANGE *player_setting_change)
{
    BTMW_RPC_TEST_Logd("num_attr(%d)\n",
        player_setting_change->player_setting.num_attr);

    for (int ii = 0; ii < player_setting_change->player_setting.num_attr; ii++)
     {
        BTMW_RPC_TEST_Logd("attr_id(%d), attr_values(%d)\n",
            player_setting_change->player_setting.attr_ids[ii], player_setting_change->player_setting.attr_values[ii]);
     }
}

static VOID btmw_rpc_test_avrcp_show_player_setting_rsp(BT_AVRCP_PLAYERSETTING_RSP *player_setting_rsp)
{
    BTMW_RPC_TEST_Logd("rsp accepted(%d)\n",
        player_setting_rsp->accepted);
}

static VOID btmw_rpc_test_avrcp_show_list_player_setting_rsp(BT_AVRCP_LIST_PLAYERSETTING_RSP *list_player_setting_rsp)
{
    BTMW_RPC_TEST_Logd("list playersetting rsp num_attr(%d) num_ext_attr(%d)\n",
        list_player_setting_rsp->num_attr, list_player_setting_rsp->num_ext_attr);

    for (int i = 0; i < list_player_setting_rsp->num_attr; i++)
    {
        BTMW_RPC_TEST_Logd("attr_id(%d) num_val(%d)\n",
        list_player_setting_rsp->player_app_attr.attr_id, list_player_setting_rsp->player_app_attr.num_val);
        for (int j = 0; j < list_player_setting_rsp->player_app_attr.num_val; j++)
        {
            BTMW_RPC_TEST_Logd("attr_val(%d)\n",
            list_player_setting_rsp->player_app_attr.attr_val[j]);
        }
    }

    for (int i = 0; i < list_player_setting_rsp->num_ext_attr; i++)
    {
        BTMW_RPC_TEST_Logd("ext_attr_id(%d) ext_num_val(%d) charset_id(%d)\n",
        list_player_setting_rsp->player_app_ext_attr.attr_id, list_player_setting_rsp->player_app_ext_attr.num_val,
        list_player_setting_rsp->player_app_ext_attr.charset_id);
        for (int j = 0; j < list_player_setting_rsp->player_app_ext_attr.num_val; j++)
        {
            BTMW_RPC_TEST_Logd("ext_attr_val(%d)\n",
            list_player_setting_rsp->player_app_ext_attr.ext_attr_val[j].val);
        }
    }

}

static VOID btmw_rpc_test_avrcp_show_pos_change(BT_AVRCP_POS_CHANGE *pos_change)
{
    BTMW_RPC_TEST_Logd("song_len(%u)\n", pos_change->song_len);
    BTMW_RPC_TEST_Logd("song_pos(%u)\n", pos_change->song_pos);
    return;
}

static VOID btmw_rpc_test_avrcp_show_play_status_change(
    BT_AVRCP_PLAY_STATUS_CHANGE *status_change)
{
    BTMW_RPC_TEST_Logd("play_status(%u)\n", status_change->play_status);
    stream_state = (int)status_change->play_status;
    if(status_change->play_status == AVRCP_PLAY_STATUS_PLAYING)
    {
        if (cmd == 1)
        {
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);

            usleep(1000*10);//10ms
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);
        }
        else if(cmd == 2)
        {
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_PRESS);

            usleep(1000*10);
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_RELEASE);
        }
    }
    else
    {
        if (cmd == 1 || cmd ==2)
        {
            cmd_test_cnt++;
            if (cmd_test_cnt > 1000)
            {
                BTMW_RPC_TEST_Logd("auto cmd success(%u)\n");
                return;
            }
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);

            usleep(1000*10);
            a_mtkapi_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);
        }
    }

    return;
}

static VOID btmw_rpc_test_avrcp_show_volume_change(
    BT_AVRCP_VOLUME_CHANGE *volume_change)
{
    BTMW_RPC_TEST_Logd("remote_volume(%u)\n", volume_change->abs_volume);
    return;
}

static VOID btmw_rpc_test_avrcp_handle_set_volume_req(
    BT_AVRCP_SET_VOL_REQ *set_vol_req)
{
    BTMW_RPC_TEST_Logd("volume(%u)\n", set_vol_req->abs_volume);
    a_mtkapi_avrcp_update_absolute_volume(set_vol_req->abs_volume);
    return;
}


static VOID btmw_rpc_test_avrcp_handle_passthrough_cmd_req(
    BT_AVRCP_PASSTHROUGH_CMD_REQ *passthrough_cmd_req)
{

    char *key_name[BT_AVRCP_CMD_TYPE_MAX] = {"play", "pause", "next", "previous",
        "fast forward", "reward", "stop", "volume up", "volume down"};
    char *action[BT_AVRCP_KEY_STATE_MAX] = {"press", "release", "auto"};
    UINT32 cmd_type = 0;
    UINT32 action_type = 0;
    if (passthrough_cmd_req->cmd_type < BT_AVRCP_CMD_TYPE_MAX &&
        passthrough_cmd_req->cmd_type >= BT_AVRCP_CMD_TYPE_PLAY)
    {
        cmd_type = (UINT32)passthrough_cmd_req->cmd_type;
        BTMW_RPC_TEST_Logd("cmd_type(%s)\n", key_name[cmd_type]);
    }
    if (passthrough_cmd_req->action < BT_AVRCP_KEY_STATE_MAX &&
        passthrough_cmd_req->action >= BT_AVRCP_KEY_STATE_PRESS)
    {
        action_type = (UINT32)passthrough_cmd_req->action;
        BTMW_RPC_TEST_Logd("action(%s)\n", action[action_type]);
    }
    return;
}

static VOID btmw_rpc_test_avrcp_handle_remote_feature_cb(
    BT_AVRCP_FEATURE *avrcp_features)
{
    BTMW_RPC_TEST_Logd("role(%s)\n", avrcp_features->role);
    BTMW_RPC_TEST_Logd("feature(%x)\n", avrcp_features->feature);
    return;
}

static VOID btmw_rpc_test_avrcp_handle_get_items_cb(
    BT_AVRCP_GET_FOLDER_ITEMS_CB_DATA *get_folder_items_data)
{
    BTMW_RPC_TEST_Logd("count =%d\n", get_folder_items_data->count);
    BTMW_RPC_TEST_Logd("status=%d\n", get_folder_items_data->status);

    BTMW_RPC_TEST_Logd("item_type =%d\n",
        (UINT8)(get_folder_items_data->folder_items[0].item_type));
    if (get_folder_items_data->folder_items[0].item_type == BT_AVRCP_ITEM_TYPE_PLAYER)
    {
        BTMW_RPC_TEST_Logd("player charset_id =%d\n",
           get_folder_items_data->folder_items[0].item.player.charset_id);
        BTMW_RPC_TEST_Logd("player features =%x\n",
           get_folder_items_data->folder_items[0].item.player.features);
        BTMW_RPC_TEST_Logd("player major_type =%d\n",
           get_folder_items_data->folder_items[0].item.player.major_type);
        BTMW_RPC_TEST_Logd("player name =%d\n",
           get_folder_items_data->folder_items[0].item.player.name);
        BTMW_RPC_TEST_Logd("player player_id =%d\n",
           get_folder_items_data->folder_items[0].item.player.player_id);
    }
    else if (get_folder_items_data->folder_items[0].item_type == BT_AVRCP_ITEM_TYPE_FOLDER)
    {
        BTMW_RPC_TEST_Logd("folder charset_id =%d\n",
           get_folder_items_data->folder_items[0].item.folder.charset_id);
        BTMW_RPC_TEST_Logd("folder name =%s\n",
           get_folder_items_data->folder_items[0].item.folder.name);
        BTMW_RPC_TEST_Logd("folder uid =%d\n",
           get_folder_items_data->folder_items[0].item.folder.uid);
        BTMW_RPC_TEST_Logd("folder type =%d\n",
           get_folder_items_data->folder_items[0].item.folder.type);
    }
    else if (get_folder_items_data->folder_items[0].item_type == BT_AVRCP_ITEM_TYPE_MEDIA)
    {
        BTMW_RPC_TEST_Logd("media charset_id =%d\n",
           get_folder_items_data->folder_items[0].item.media.charset_id);
        BTMW_RPC_TEST_Logd("media name =%d\n",
           get_folder_items_data->folder_items[0].item.media.name);
        BTMW_RPC_TEST_Logd("media num_attrs =%d\n",
           get_folder_items_data->folder_items[0].item.media.num_attrs);
        BTMW_RPC_TEST_Logd("media uid =%d\n",
           get_folder_items_data->folder_items[0].item.media.uid);
    }
    return;
}

static VOID btmw_rpc_test_avrcp_handle_change_folder_path_cb(
    BT_AVRCP_CHANGE_FOLDER_PATH_CB_DATA *change_folder_path_data)
{
    BTMW_RPC_TEST_Logd("count =%d\n", change_folder_path_data->count);
    return;
}

static VOID btmw_rpc_test_avrcp_handle_set_browsed_player_cb(
    BT_AVRCP_SET_BROWSED_PLAYER_CB_DATA *set_browsed_player_data)
{
    BTMW_RPC_TEST_Logd("num_items =%d\n", set_browsed_player_data->num_items);
    BTMW_RPC_TEST_Logd("depth =%d\n", set_browsed_player_data->depth);
    return;
}

static VOID btmw_rpc_test_avrcp_handle_set_addressed_player_cb(
    BT_AVRCP_SET_ADDRESSED_PLAYER_CB_DATA *set_addressed_player_data)
{
    BTMW_RPC_TEST_Logd("status =%d\n", set_addressed_player_data->status);
    return;
}

static VOID btmw_rpc_test_avrcp_handle_addressed_player_changed_cb(
    BT_AVRCP_ADDRESSED_PLAYER_CHANGED_CB_DATA *addressed_player_changed_data)
{
    BTMW_RPC_TEST_Logd("id =%d\n", addressed_player_changed_data->id);
    return;
}

static VOID btmw_rpc_test_avrcp_app_cbk(BT_AVRCP_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_rpc_test_avrcp_app_event(param->event));
    switch (param->event)
    {
        case BT_AVRCP_EVENT_CONNECTED:
            BTMW_RPC_TEST_Logd("AVRCP connected %s\n", (param->data.connection_cb.role == BT_MW_AVRCP_ROLE_CT)?"AVRCP_CT":"AVRCP_TG");
            g_avrcp_reg_event = 0;
            strncpy(g_avrcp_addr_test, param->addr, sizeof(g_avrcp_addr_test));
            g_avrcp_addr_test[17] = '\0';
            BTMW_RPC_TEST_Logd("rc_connect\n", param->data.connection_cb.rc_connect);
            BTMW_RPC_TEST_Logd("bt_connect\n", param->data.connection_cb.bt_connect);
            break;
        case BT_AVRCP_EVENT_DISCONNECTED:
            BTMW_RPC_TEST_Logd("AVRCP disconnected\n");
            BTMW_RPC_TEST_Logd("rc_connect\n", param->data.connection_cb.rc_connect);
            BTMW_RPC_TEST_Logd("bt_connect\n", param->data.connection_cb.bt_connect);
            g_avrcp_reg_event = 0;
            g_avrcp_addr_test[0] = 0;
            break;
        case BT_AVRCP_EVENT_TRACK_CHANGE:
            track_change = 1;
            btmw_rpc_test_avrcp_show_track_change(&param->data.track_change);
            break;
        case BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE:
            btmw_rpc_test_avrcp_show_player_setting_change(&param->data.player_setting_change);
            break;
        case BT_AVRCP_EVENT_PLAYER_SETTING_RSP:
            btmw_rpc_test_avrcp_show_player_setting_rsp(&param->data.player_setting_rsp);
            break;
        case BT_AVRCP_EVENT_LIST_PLAYER_SETTING_RSP:
            btmw_rpc_test_avrcp_show_list_player_setting_rsp(&param->data.list_player_setting_rsp);
            break;
        case BT_AVRCP_EVENT_POS_CHANGE:
            btmw_rpc_test_avrcp_show_pos_change(&param->data.pos_change);
            break;
        case BT_AVRCP_EVENT_PLAY_STATUS_CHANGE:
            btmw_rpc_test_avrcp_show_play_status_change(&param->data.play_status_change);
            break;
        case BT_AVRCP_EVENT_VOLUME_CHANGE:
            btmw_rpc_test_avrcp_show_volume_change(&param->data.volume_change);
            break;
        case BT_AVRCP_EVENT_SET_VOLUME_REQ:
            btmw_rpc_test_avrcp_handle_set_volume_req(&param->data.set_vol_req);
            break;
        case BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ:
            btmw_rpc_test_avrcp_handle_passthrough_cmd_req(&param->data.passthrough_cmd_req);
            break;
        case BT_AVRCP_EVENT_FEATURE:
            btmw_rpc_test_avrcp_handle_remote_feature_cb(&param->data.feature);
            break;
        case BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB:
            BTMW_RPC_TEST_Logd("count =%d\n", param->data.get_folder_items.count);
            btmw_rpc_test_avrcp_handle_get_items_cb(&param->data.get_folder_items);
            break;
        case BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB:
            btmw_rpc_test_avrcp_handle_change_folder_path_cb(&param->data.change_folder_path);
            break;
        case BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB:
            btmw_rpc_test_avrcp_handle_set_browsed_player_cb(&param->data.set_browsed_player);
            break;
        case BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB:
            btmw_rpc_test_avrcp_handle_set_addressed_player_cb(&param->data.set_addressed_player);
            break;
        case BT_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB:
            btmw_rpc_test_avrcp_handle_addressed_player_changed_cb(&param->data.addressed_player_changed);
            break;
        case BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB:
            BTMW_RPC_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
                btmw_rpc_test_avrcp_app_event(param->event));
            break;
        case BT_AVRCP_EVENT_ABS_SUPPORT:
            BTMW_RPC_TEST_Logd("AVRCP abs supported\n");
            break;
        default:
            break;
    }
    return;
}

int btmw_rpc_test_rc_init(void)
{
    int ret = 0;
    BTMW_RPC_TEST_MOD avrcp_ct_mode = {0};

    avrcp_ct_mode.mod_id = BTMW_RPC_TEST_MOD_AVRCP_CT;
    strncpy(avrcp_ct_mode.cmd_key, BTMW_RPC_TEST_CMD_KEY_AVRCP_CT, sizeof(avrcp_ct_mode.cmd_key));
    avrcp_ct_mode.cmd_handler = btmw_rpc_test_rc_ct_cmd_handler;
    avrcp_ct_mode.cmd_tbl = btmw_rpc_test_rc_ct_cli_commands;

    ret = btmw_rpc_test_register_mod(&avrcp_ct_mode);
    BTMW_RPC_TEST_Logd("[AVRCP][CT] btmw_rpc_test_register_mod() for returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        a_mtkapi_avrcp_register_callback(btmw_rpc_test_avrcp_app_cbk, NULL);
    }

    return ret;
}

int btmw_rpc_test_rc_deinit(void)
{
    return 0;
}

