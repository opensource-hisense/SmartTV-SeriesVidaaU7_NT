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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "u_bt_mw_types.h"
#include "u_bt_mw_avrcp.h"
#include "u_bt_mw_a2dp.h"
#include "btmw_test_avrcp_if.h"
#include "c_bt_mw_avrcp.h"

static BTMW_TEST_CLI btmw_test_rc_ct_cli_commands[];
#define BTMW_TEST_CMD_KEY_AVRCP_CT     "MW_AVRCP_CT"

CHAR g_avrcp_addr_test[18];
UINT32 g_avrcp_reg_event = 0;

#define BTMW_AVRCP_CASE_RETURN_STR(const) case const: return #const;


int btmw_test_rc_ct_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_TEST_Logi("[AVRCP] CT argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = btmw_test_rc_ct_cli_commands;
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
        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_AVRCP_CT, btmw_test_rc_ct_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static int btmw_test_rc_send_play_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PLAY, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_test_rc_send_pause_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_PAUSE, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_test_rc_send_stop_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_STOP, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_test_rc_send_fwd_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_bwd_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_BWD, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_BWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_ffwd_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FFWD, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_FFWD, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_rwd_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_RWD, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_RWD, BT_AVRCP_KEY_STATE_RELEASE);

    return 0;
}

static int btmw_test_rc_send_volumeup_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_UP, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_UP, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_volumedown_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_DOWN, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_passthrough_cmd(g_avrcp_addr_test, BT_AVRCP_CMD_TYPE_VOL_DOWN, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_next_group_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    c_btm_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 0, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 0, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}

static int btmw_test_rc_send_prev_group_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);
    c_btm_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 1, BT_AVRCP_KEY_STATE_PRESS);
    sleep(1);
    c_btm_avrcp_send_vendor_unique_cmd(g_avrcp_addr_test, 1, BT_AVRCP_KEY_STATE_RELEASE);
    return 0;
}


static int btmw_test_rc_set_volume_cmd_handler(int argc, char *argv[])
{
    UINT8 u1volume = 0;
    u1volume = atoi(argv[0]);
    BTMW_TEST_Logi("[AVRCP] %s() volume=%d\n", __func__, u1volume);
    c_btm_avrcp_change_volume(g_avrcp_addr_test, u1volume);
    return 0;
}

#if 0
static int btmw_test_rc_player_setting_cmd_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[AVRCP] %s() \n", __func__);

    BT_AVRCP_PLAYER_SETTING player_setting;
    memset(&player_setting, 0x0, sizeof(BT_AVRCP_PLAYER_SETTING));

    if (argc >= 1)
    {
        strcpy(g_avrcp_addr_test, argv[0]);
    }

    if (argc >= 2)
    {
        player_setting.num_attr = argc - 1;
        for (int ii = 1; ii < argc; ii++)
            {
               char *p_temp = strtok(argv[ii], "=");
               BTMW_TEST_Logi("p_temp=%s", p_temp);
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
                  BTMW_TEST_Logi("attribute id is empty\n");
                  continue;
                }
               char *p_value = strtok(NULL, "=");
               BTMW_TEST_Logi("p_value=%s", p_value);
               if(p_value)
                {
                  player_setting.attr_values[ii-1] = atoi(p_value);
                }
               else
                {
                  BTMW_TEST_Logi("attribute value is empty\n");
                  continue;
                }
            }

        BTMW_TEST_Logi("argc:%ld  num_attr:%d\n", argc, player_setting.num_attr);
    }
    else
    {
        BTMW_TEST_Logi("player_setting attribute is empty!\n");
        return 0;
    }
    c_btm_avrcp_change_player_app_setting(g_avrcp_addr_test, &player_setting);
    return 0;
}
#endif

static BTMW_TEST_CLI btmw_test_rc_ct_cli_commands[] =
{
    {"play",             btmw_test_rc_send_play_handler,             " = send play command <addr>"},
    {"pause",            btmw_test_rc_send_pause_handler,            " = send pause command <addr>"},
    {"stop",             btmw_test_rc_send_stop_handler,             " = send stop command <addr>"},
    {"fwd",              btmw_test_rc_send_fwd_handler,              " = send fwd command <addr>"},
    {"bwd",              btmw_test_rc_send_bwd_handler,              " = send bwd command <addr>"},
    {"ffwd",             btmw_test_rc_send_ffwd_handler,             " = send ffwd command <addr>"},
    {"rwd",              btmw_test_rc_send_rwd_handler,              " = send rwd command <addr>"},
    {"volume_up",        btmw_test_rc_send_volumeup_handler,         " = send volume up command <addr>"},
    {"volume_down",      btmw_test_rc_send_volumedown_handler,       " = send volume down command <addr>"},
    {"next_group",       btmw_test_rc_send_next_group_handler,       " = send next group command <addr>"},
    {"prev_group",       btmw_test_rc_send_prev_group_handler,       " = send previous group command <addr>"},
    {"set_volume",       btmw_test_rc_set_volume_cmd_handler,        " = set_volume <volume:0~127> "},
//    {"player_setting",   btmw_test_rc_player_setting_cmd_handler,    " = player_setting <addr> <attribute=[1-4]> attribute=equalizer[1-2]/repeat[1-4]/shuffle[1-3]/scan[1-3]"},
    {NULL, NULL, NULL},
};

static CHAR* btmw_test_avrcp_app_event(BT_AVRCP_EVENT event)
{
    switch((int)event)
    {
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CONNECTED)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_DISCONNECTED)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_TRACK_CHANGE)
//       BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_POS_CHANGE)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAY_STATUS_CHANGE)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_GET_FOLDER_ITEMS_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_CHANGE_FOLDER_PTAH_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_BROWSED_PLAYER_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_SET_ADDRESSED_PLAYER_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_ADDRESSED_PLAYER_CHANGED_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CT_NOW_PLAYING_COTENTS_CHANGED_CB)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_VOLUME_CHANGE)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_SET_VOLUME_REQ)
        BTMW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ)
        default: return "UNKNOWN_AVRCP_EVENT";
   }
}

static VOID btmw_test_avrcp_show_track_change(BT_AVRCP_TRACK_CHANGE *track_change)
{
    BTMW_TEST_Logd("title(%s)\n",
        track_change->element_attr.title);
    BTMW_TEST_Logd("artist(%s)\n",
        track_change->element_attr.artist);
    BTMW_TEST_Logd("album(%s)\n",
        track_change->element_attr.album);
    BTMW_TEST_Logd("current_track_number(%u)\n",
        track_change->element_attr.current_track_number);
    BTMW_TEST_Logd("number_of_tracks(%u)\n",
        track_change->element_attr.number_of_tracks);
    BTMW_TEST_Logd("genre(%s)\n",
        track_change->element_attr.genre);
    BTMW_TEST_Logd("position(%u)\n",
        track_change->element_attr.position);
    return;
}

#if 0
static VOID btmw_test_avrcp_show_player_setting_change(BT_AVRCP_PLAYERSETTING_CHANGE *player_setting_change)
{
    BTMW_TEST_Logd("num_attr(%d)\n",
       player_setting_change->player_setting.num_attr);

    for (int ii = 0; ii < player_setting_change->player_setting.num_attr; ii++)
     {
        BTMW_TEST_Logd("attr_id(%d), attr_values(%d)\n",
            player_setting_change->player_setting.attr_ids, player_setting_change->player_setting.attr_values);
     }
}
#endif

static VOID btmw_test_avrcp_show_pos_change(BT_AVRCP_POS_CHANGE *pos_change)
{
    BTMW_TEST_Logd("song_len(%u)\n", pos_change->song_len);
    BTMW_TEST_Logd("song_pos(%u)\n", pos_change->song_pos);
    return;
}

static VOID btmw_test_avrcp_show_play_status_change(
    BT_AVRCP_PLAY_STATUS_CHANGE *status_change)
{
    BTMW_TEST_Logd("play_status(%u)\n", status_change->play_status);
    return;
}

static VOID btmw_test_avrcp_show_volume_change(
    BT_AVRCP_VOLUME_CHANGE *volume_change)
{
    BTMW_TEST_Logd("remote_volume(%u)\n", volume_change->abs_volume);
    return;
}

static VOID btmw_test_avrcp_handle_set_volume_req(
    BT_AVRCP_SET_VOL_REQ *set_vol_req)
{
    BTMW_TEST_Logd("volume(%u)\n", set_vol_req->abs_volume);
    c_btm_avrcp_update_absolute_volume(set_vol_req->abs_volume);
    return;
}

static VOID btmw_test_avrcp_handle_passthrough_cmd_req(
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
        BTMW_TEST_Logd("cmd_type(%u)\n", key_name[cmd_type]);
    }
    if (passthrough_cmd_req->action < BT_AVRCP_KEY_STATE_MAX &&
        passthrough_cmd_req->action >= BT_AVRCP_KEY_STATE_PRESS)
    {
        action_type = (UINT32)passthrough_cmd_req->action;
        BTMW_TEST_Logd("action(%u)\n", action[action_type]);
    }
    return;
}


static VOID btmw_test_avrcp_app_cbk(BT_AVRCP_EVENT_PARAM *param)
{
    if (NULL == param)
    {
        BTMW_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_test_avrcp_app_event(param->event));
    switch (param->event)
    {
        case BT_AVRCP_EVENT_CONNECTED:
            BTMW_TEST_Logd("AVRCP connected\n");
            g_avrcp_reg_event = 0;
            strncpy(g_avrcp_addr_test, param->addr, sizeof(g_avrcp_addr_test));
            g_avrcp_addr_test[17] = '\0';
            break;
        case BT_AVRCP_EVENT_DISCONNECTED:
            BTMW_TEST_Logd("AVRCP disconnected\n");
            g_avrcp_reg_event = 0;
            g_avrcp_addr_test[0] = 0;
            break;
        case BT_AVRCP_EVENT_TRACK_CHANGE:
            btmw_test_avrcp_show_track_change(&param->data.track_change);
            break;
/*        case BT_AVRCP_EVENT_PLAYER_SETTING_CHANGE:
            btmw_test_avrcp_show_player_setting_change(&param->data.player_setting_change);
            break;*/
        case BT_AVRCP_EVENT_POS_CHANGE:
            btmw_test_avrcp_show_pos_change(&param->data.pos_change);
            break;
        case BT_AVRCP_EVENT_PLAY_STATUS_CHANGE:
            btmw_test_avrcp_show_play_status_change(&param->data.play_status_change);
            break;
        case BT_AVRCP_EVENT_VOLUME_CHANGE:
            btmw_test_avrcp_show_volume_change(&param->data.volume_change);
            break;
        case BT_AVRCP_EVENT_SET_VOLUME_REQ:
            btmw_test_avrcp_handle_set_volume_req(&param->data.set_vol_req);
            break;
        case BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ:
            btmw_test_avrcp_handle_passthrough_cmd_req(&param->data.passthrough_cmd_req);
            break;
        default:
            break;
    }
    return;
}

int btmw_test_rc_init(int reg_callback)
{
    int ret = 0;
    BTMW_TEST_MOD avrcp_ct_mode = {0};

    avrcp_ct_mode.mod_id = BTMW_TEST_MOD_AVRCP_CT;
    strncpy(avrcp_ct_mode.cmd_key, BTMW_TEST_CMD_KEY_AVRCP_CT, sizeof(avrcp_ct_mode.cmd_key));
    avrcp_ct_mode.cmd_handler = btmw_test_rc_ct_cmd_handler;
    avrcp_ct_mode.cmd_tbl = btmw_test_rc_ct_cli_commands;

    ret = btmw_test_register_mod(&avrcp_ct_mode);
    BTMW_TEST_Logd("[AVRCP][CT] btmw_test_register_mod() for returns: %d\n", ret);
    if (reg_callback)
    {
        c_btm_avrcp_register_callback(btmw_test_avrcp_app_cbk);
    }

    return ret;
}

int btmw_test_rc_deinit(void)
{
    return 0;
}

