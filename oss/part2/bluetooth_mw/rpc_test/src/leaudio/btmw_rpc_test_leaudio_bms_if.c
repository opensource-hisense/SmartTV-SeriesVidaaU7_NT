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
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "list.h"
#include "util.h"
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_leaudio_bms_if.h"

#include "mtk_bt_service_leaudio_bms_wrapper.h"
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#include <dlfcn.h>
#include "aud_leaudio_uploader.h"
#include "u_rpcipc_types.h"
#include "x_aud_dec.h"
#include "mtauddec.h"
#include "drv_common.h"
#include "u_common.h"
#endif

#define BTMW_RPC_BMS_CASE_RETURN_STR(const) case const: return #const;
#define BTMW_RPC_TEST_BMS_XML_NAME_LEN    (128)
#define BTMW_RPC_TEST_BMS_ANNOUNCEMENT                  "basic-audio-announcement"
#define BTMW_RPC_TEST_BMS_BIG                           "big"
#define BTMW_RPC_TEST_BMS_SUBGROUP                      "subgroup"
#define BTMW_RPC_TEST_BMS_BIS                           "bis"
#define BTMW_RPC_TEST_BMS_ATTR_BAS_CONFIG_NAME          "bas-config-name"
#define BTMW_RPC_TEST_BMS_ATTR_CODEC_CONFIG_NAME        "codec-config-name"
#define BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION "audio-channel-allocation"
#define BTMW_RPC_TEST_BMS_ATTR_METADATA_LEN             "metadata_len"
#define BTMW_RPC_TEST_BMS_ATTR_METADATA                 "metadata"
#define BTMW_RPC_TEST_BMS_ATTR_SUBGROUP_NUM             "subgroup-num"
#define BTMW_RPC_TEST_BMS_ATTR_BIS_NUM                  "bis-num"
#define BMS_RECONFIG_AUTO_TEST TRUE

typedef struct
{
    CHAR xml_name[BTMW_RPC_TEST_BMS_XML_NAME_LEN];
    xmlDoc *doc;
    xmlNode *xml_root;
    struct dl_list big_param_list;
} BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML;

typedef enum {
    BT_BMS_CMD_NONE = 0,
    BT_BMS_PAUSE = 1,
    BT_BMS_STOP = 2,
    BT_BMS_STOP_ALL = 3,
    BT_BMS_RECONFIG = 4,
} BTMW_RPC_TEST_BMS_PENDING_CMD;

typedef enum {
    BT_BMS_2BIS_MONO = 0,
    BT_BMS_4BIS_MONO = 1,
    BT_BMS_1BIS_STEREO = 2,
    BT_BMS_2BIS_STEREO = 3,
    BT_BMS_BROADCAST_XML = 4,
} BTMW_RPC_TEST_BMS_BROADCAST_INFO;

struct socketidx_map_uploaderhdl_t smu_t[BT_BROADCAST_SOCKET_INDEX_NUM] = {0};
extern struct a2dp_leaudio_uploader_config_t auc;
BT_BMS_EVENT_SOCKET_INDEX_DATA _gbms_socket_t = {
      .instance_id=0,
      .socket_index_num=0,
      .socket_index_list={{
        .socket_index=0,
        .channel_num=0,
        .subgroup_num=0,
        .subgroup_ids={0},
        .iso_status=FALSE,
      }},
};

static UINT8 g_instance_id = 0xFF;
static BOOL g_flag_auto = FALSE;
static BT_BMS_BROADCAST_STATE g_current_broadcast_state = BT_BMS_STOPPED;
static BTMW_RPC_TEST_BMS_PENDING_CMD g_pending_cmd = BT_BMS_CMD_NONE;
BTMW_RPC_TEST_BMS_BROADCAST_INFO g_current_broadcast_info = BT_BMS_2BIS_MONO;
static CHAR g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN] = {0};

#if defined(BMS_RECONFIG_AUTO_TEST) && (BMS_RECONFIG_AUTO_TEST == TRUE)
static BOOL g_reconfig_stress_test = FALSE;
static UINT32 g_reconfig_num = 1;
#endif

//dlopen 3 shared lib for update conn state to app {
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
#define MAX_NUM_HANDLES 4096

static void *fHandle1 = NULL;
static void *fHandle2 = NULL;
static void *app_handle = NULL;
static INT32 bms_binit = 0;

INT32 (*bms_c_rpc_init_client)(VOID);
INT32 (*bms_c_rpc_start_client)(VOID);
INT32 (*bms_os_init)(const VOID *pv_addr, UINT32 z_size);
INT32 (*bms_handle_init) (UINT16   ui2_num_handles,
                             VOID**   ppv_mem_addr,
                             UINT32*  pz_mem_size);
INT32 (*bms_x_rtos_init) (GEN_CONFIG_T*  pt_config);

typedef INT32 (*dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state_func)(BOOL b_connected);
dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state_func dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state = NULL;
#endif
//}

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
extern INT32 start_a2dp_uploader(UINT32 sample_rate, UINT32 channel_num);
long start_leaudio_uploader(UINT8 socket_idx, UINT8 channel_num);
int stop_leaudio_uploader();
#endif

void btmw_rpc_test_bms_clear();

static int btmw_rpc_test_bms_create_broadcast(int argc, char **argv);
static int btmw_rpc_test_bms_create_broadcast_ext(int argc, char **argv);
static int btmw_rpc_test_bms_update_base_announcement(int argc, char **argv);
static int btmw_rpc_test_bms_update_subgroup_metadata(int argc, char **argv);
static int btmw_rpc_test_bms_start_broadcast(int argc, char **argv);
static int btmw_rpc_test_bms_start_broadcast_multi_thread(int argc, char **argv);
static int btmw_rpc_test_bms_pause_broadcast(int argc, char **argv);
static int btmw_rpc_test_bms_stop_broadcast(int argc, char **argv);
static int btmw_rpc_test_bms_get_own_address(int argc, char **argv);
static int btmw_rpc_test_bms_get_all_broadcasts_states(int argc, char **argv);
static int btmw_rpc_test_bms_stop_all_broadcast(int argc, char **argv);
static int btmw_rpc_test_bms_get_broadcast_info(int argc, char **argv);

static int btmw_rpc_test_bms_broadcast_1bis_mono(int argc, char **argv);
static int btmw_rpc_test_bms_broadcast_2bis_mono(int argc, char **argv);
static int btmw_rpc_test_bms_broadcast_4bis_mono(int argc, char **argv);
static int btmw_rpc_test_bms_broadcast_1bis_stereo(int argc, char **argv);
static int btmw_rpc_test_bms_broadcast_2bis_stereo(int argc, char **argv);

static int btmw_rpc_test_bms_check_announcement_xml(CHAR *announcement_xml_path);
static int btmw_rpc_test_bms_load_announcement_from_xml(
    BT_BMS_ANNOUNCEMENT *announcement, CHAR *announcement_xml_path);
static int btmw_rpc_test_bms_free_announcement_xml(
    BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML *announcement_xml);
static BOOL btmw_rpc_test_bms_parse_big_param(BT_BMS_ANNOUNCEMENT *announcement, xmlNode *node);
static BOOL btmw_rpc_test_bms_parse_subgroup_param(BT_BMS_ANNOUNCEMENT_SUBGROUP *subgroup, xmlNode *node);
static BOOL btmw_rpc_test_bms_parse_bis_param(BT_BMS_ANNOUNCEMENT_BIS *bis, xmlNode *node);
static int btmw_rpc_test_bms_metadata_acsii_2_hex(UINT8 *metadata_len, UINT8* metadata_hex, char* metadata_acsii);
static int btmw_rpc_test_bms_broadcastcode_acsii_2_hex(UINT8 *code_len, UINT8* code_hex, char* code_acsii);


INT32 ascii_2_hex(CHAR *p_ascii, INT32 len, UINT8 *p_hex)
{
    INT32     x;
    UINT8     c;
    if (NULL == p_ascii || NULL == p_hex)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (x = 0; (x < len) && (*p_ascii); x++)
    {
        if (isdigit (*p_ascii))
            c = (*p_ascii - '0') << 4;
        else
            c = (toupper(*p_ascii) - 'A' + 10) << 4;
        p_ascii++;
        if (*p_ascii)
        {
            if (isdigit (*p_ascii))
                c |= (*p_ascii - '0');
            else
                c |= (toupper(*p_ascii) - 'A' + 10);
            p_ascii++;
        }
        *p_hex++ = c;
    }
    return x;
}

static CHAR* btmw_rpc_test_bms_broadcast_state(BT_BMS_BROADCAST_STATE state)
{
    switch((int)state)
    {
        case BT_BMS_STOPPED:
          return "STOPPED";
        case BT_BMS_PAUSED:
          return "PAUSED";
        case BT_BMS_STREAMING:
          return "STREAMING";
        default:
          return "UNKNOWN_BROADCAST_STATE";
   }
}

static CHAR* btmw_rpc_test_bms_app_event(BT_BMS_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_CREATED)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_DESTORYED)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_BROADCAST_STATE)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_SOCKET_INDEX_NOTIFY)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_ISO_STATUS)
        BTMW_RPC_BMS_CASE_RETURN_STR(BT_BMS_EVENT_OWN_ADDRESS)
        default: return "UNKNOWN_BMS_EVENT";
   }
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_bms_cli_commands[] =
{
    {(const char *)"create_broadcast",              btmw_rpc_test_bms_create_broadcast,             (const char *)" = create_broadcast <localname> <metadata_len> [metadata] <profile> [broadcast_code]"},
    {(const char *)"create_broadcast_ext",          btmw_rpc_test_bms_create_broadcast_ext,         (const char *)" = create_broadcast_ext <localname> <announcement_xml> [broadcast_code]"},
    {(const char *)"update_base_announcement",      btmw_rpc_test_bms_update_base_announcement,     (const char *)" = update_base_announcement [instance_id] <announcement_xml>"},
    {(const char *)"update_subgroup_metadata",      btmw_rpc_test_bms_update_subgroup_metadata,     (const char *)" = update_subgroup_metadata [instance_id] <subgroup_id> <metadata>"},
    {(const char *)"start_broadcast",               btmw_rpc_test_bms_start_broadcast,              (const char *)" = start_broadcast [instance_id]"},
    {(const char *)"start_broadcast_multi_thread",  btmw_rpc_test_bms_start_broadcast_multi_thread, (const char *)" = start_broadcast_multi_thread [instance_id]"},
    {(const char *)"pause_broadcast",               btmw_rpc_test_bms_pause_broadcast,              (const char *)" = pause_broadcast [instance_id]"},
    {(const char *)"stop_broadcast",                btmw_rpc_test_bms_stop_broadcast,               (const char *)" = stop_broadcast [instance_id]"},
    {(const char *)"stop_all_broadcast",            btmw_rpc_test_bms_stop_all_broadcast,           (const char *)" = stop_all_broadcast"},
    {(const char *)"get_own_address",               btmw_rpc_test_bms_get_own_address,              (const char *)" = get_own_address [instance_id]"},
    {(const char *)"get_all_broadcasts_states",     btmw_rpc_test_bms_get_all_broadcasts_states,    (const char *)" = get_all_broadcasts_states"},
    {(const char *)"get_broadcast_info",            btmw_rpc_test_bms_get_broadcast_info,           (const char *)" = get_broadcast_info"},

    {(const char *)"broadcast_1bis_mono",           btmw_rpc_test_bms_broadcast_1bis_mono,          (const char *)" = broadcast_1bis_mono <localname> [broadcast_code]"},
    {(const char *)"broadcast_2bis_mono",           btmw_rpc_test_bms_broadcast_2bis_mono,          (const char *)" = broadcast_2bis_mono <localname> [broadcast_code]"},
    {(const char *)"broadcast_4bis_mono",           btmw_rpc_test_bms_broadcast_4bis_mono,          (const char *)" = broadcast_4bis_mono <localname> [broadcast_code]"},
    {(const char *)"broadcast_1bis_stereo",         btmw_rpc_test_bms_broadcast_1bis_stereo,        (const char *)" = broadcast_1bis_stereo <localname> [broadcast_code]"},
    {(const char *)"broadcast_2bis_stereo",         btmw_rpc_test_bms_broadcast_2bis_stereo,        (const char *)" = broadcast_2bis_stereo <localname> [broadcast_code]"},
    {NULL, NULL, NULL},
};

static int btmw_rpc_test_bms_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd = NULL;
    BTMW_RPC_TEST_CLI *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_RPC_TEST_Logi("[BMS] argc: %d, argv[0]: %s\n", argc, argv[0]);
    cmd = btmw_rpc_test_bms_cli_commands;
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logi("[BMS] Unknown command\n");
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BMS, btmw_rpc_test_bms_cli_commands);
        return -1;
    }

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
        BTMW_RPC_TEST_Logi("[BMS] Unknown command '%s'\n", argv[0]);
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BMS, btmw_rpc_test_bms_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static void btmw_rpc_test_bms_reset_var()
{
    BTMW_RPC_TEST_Logi("[BMS] reset global variables\n");
    g_instance_id = 0xFF;
    g_flag_auto = FALSE;
    g_current_broadcast_state = BT_BMS_STOPPED;
    g_pending_cmd = BT_BMS_CMD_NONE;
    g_current_broadcast_info = BT_BMS_2BIS_MONO;
    memset(g_announcement_xml_path, 0, BTMW_RPC_TEST_BMS_XML_NAME_LEN);

#if defined(BMS_RECONFIG_AUTO_TEST) && (BMS_RECONFIG_AUTO_TEST == TRUE)
    g_reconfig_stress_test = FALSE;
    g_reconfig_num = 1;
#endif
}

void btmw_rpc_test_bms_clear()
{
    BTMW_RPC_TEST_Logi("[BMS] clear\n");
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (g_current_broadcast_state == BT_BMS_STREAMING)
    {
        BTMW_RPC_TEST_Logi("[BMS] stop_leaudio_uploader\n");
        stop_leaudio_uploader();
    }
#endif
    btmw_rpc_test_bms_reset_var();
}

static VOID btmw_rpc_test_bms_show_broadcast_created(BT_BMS_EVENT_BROADCAST_CREATED_DATA *broadcast_created)
{
    if (broadcast_created->success)
    {
        BTMW_RPC_TEST_Logd("[BMS] instance_id=%d broadcast created success\n", broadcast_created->instance_id);
        g_instance_id = broadcast_created->instance_id;
        if (g_flag_auto) {
            BTMW_RPC_TEST_Logi("[BMS] auto start broadcast, instance_id=%d\n", g_instance_id);
            BT_BMS_START_BROADCAST_PARAM param;
            memset(&param, 0, sizeof(param));
            param.instance_id = g_instance_id;
            a_mtkapi_bt_bms_start_broadcast(&param);
            return;
        }
    }
    else
    {
        BTMW_RPC_TEST_Loge("[BMS] instance_id=%d broadcast created fail\n", broadcast_created->instance_id);
    }
}

static VOID btmw_rpc_test_bms_show_broadcast_destroyed(BT_BMS_EVENT_BROADCAST_DESTORYED_DATA *broadcast_destroyed)
{
    BTMW_RPC_TEST_Logd("[BMS] instance_id=%d\n", broadcast_destroyed->instance_id);

    btmw_rpc_test_bms_reset_var();

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    LEaudio_uploader_deinit();
#endif
}

/*#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
//refer bt_mw_a2dp_sink_report_player_event
static VOID btmw_rpc_test_bms_a2dp_sink_report_player_event(BT_A2DP_PLAYER_EVENT event)
{
    if (BT_A2DP_ALSA_PB_EVENT_START == event)
    {
        BTMW_RPC_TEST_Logd("A2DP SINK  BT_A2DP_ALSA_PB_EVENT_START\n");
    }
    else if (BT_A2DP_ALSA_PB_EVENT_STOP == event)
    {
        BTMW_RPC_TEST_Logd("A2DP SINK  BT_A2DP_ALSA_PB_EVENT_STOP\n");
    }
}
#endif*/

static VOID btmw_rpc_test_bms_show_broadcast_state(BT_BMS_EVENT_BROADCAST_STATE_DATA *broadcast_state)
{
    BTMW_RPC_TEST_Logd("[BMS] instance_id=%d, broadcast state %d - %s\n",
                        broadcast_state->instance_id,
                        broadcast_state->state,
                        btmw_rpc_test_bms_broadcast_state(broadcast_state->state));

    g_current_broadcast_state = broadcast_state->state;
    auc.bst = broadcast_state->state;

    switch (broadcast_state->state)
    {
        case BT_BMS_STOPPED:
            BTMW_RPC_TEST_Logd("[BMS] STOPPED\n");
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            memset(&_gbms_socket_t, 0, sizeof(_gbms_socket_t));
            if (auc.src_st == BT_A2DP_EVENT_CONNECTED && !auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
            {
                BTMW_RPC_TEST_Logd("[BMS] STOPPED then start a2dp src uploader!\n");
                if (start_a2dp_uploader(auc.sample_rate, auc.channel_num) == 0)
                {
                    auc.a2dpsrc_uploader_in_use = TRUE;
                }
            }
            /*else if (auc.snk_st == BT_A2DP_EVENT_CONNECTED && !auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
            {
                BTMW_RPC_TEST_Logd("[BMS] STOPPED then start a2dp snk playback! %d-%d\n", auc.sample_rate, auc.channel_num);
                bt_a2dp_alsa_pb_init(btmw_rpc_test_bms_a2dp_sink_report_player_event);
                bt_a2dp_alsa_player_start(auc.sample_rate, auc.channel_num, 16);
                auc.a2dpsnk_playback_in_use = TRUE;
            }*/
            else
            {
                //BTMW_RPC_TEST_Logd("[BMS] do more check here!!!\n");
                if (dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state != NULL)
                {
                    dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state(FALSE);
                    BTMW_RPC_TEST_Logd("[BMS] update_conn_state done\n");
                }
            }
#endif
            break;

        case BT_BMS_PAUSED:
            BTMW_RPC_TEST_Logd("[BMS] PAUSED\n");
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            memset(&_gbms_socket_t, 0, sizeof(_gbms_socket_t));
            if (auc.src_st == BT_A2DP_EVENT_CONNECTED && !auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
            {
                BTMW_RPC_TEST_Logd("[BMS] PAUSED then start a2dp src uploader! %d-%d\n", auc.sample_rate, auc.channel_num);
                if (start_a2dp_uploader(auc.sample_rate, auc.channel_num) == 0)
                {
                    auc.a2dpsrc_uploader_in_use = TRUE;
                }
            }
            /*else if (auc.snk_st == BT_A2DP_EVENT_CONNECTED && !auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
            {
                BTMW_RPC_TEST_Logd("[BMS] PAUSED then start a2dp snk playback! %d-%d\n", auc.sample_rate, auc.channel_num);
                bt_a2dp_alsa_pb_init(btmw_rpc_test_bms_a2dp_sink_report_player_event);
                bt_a2dp_alsa_player_start(auc.sample_rate, auc.channel_num, 16);
                auc.a2dpsnk_playback_in_use = TRUE;
            }*/
            else
            {
                //BTMW_RPC_TEST_Logd("[BMS] do more check here!\n");
                if (dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state != NULL)
                {
                    dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state(FALSE);
                    BTMW_RPC_TEST_Logd("[BMS] update_conn_state done\n");
                }
            }
#endif
            break;

        case BT_BMS_STREAMING:
            BTMW_RPC_TEST_Logd("[BMS] STREAMING\n");
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
            if (dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state != NULL)
            {
                dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state(TRUE);
                BTMW_RPC_TEST_Logd("[BMS] update_conn_state done\n");
            }
#endif
            break;

        default:
            break;
    }
}

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
long start_leaudio_uploader(UINT8 socket_idx, UINT8 channel_num)//return hUploaderhandle
{
    BTMW_RPC_TEST_Logd("[BMS] start_leaudio_uploader Enter %d", socket_idx);
    long hUploaderhandle;
    LEaudio_uploader_init();
    LEaudio_uploader_create(&hUploaderhandle, (audio_io_handle_t)socket_idx, 0, channel_num);
    if (LEaudio_uploader_start(hUploaderhandle) != 0)
    {
        BTMW_RPC_TEST_Logi(" Le Audio Uploader start failed.\n");
        LEaudio_uploader_stop(hUploaderhandle);
        LEaudio_uploader_close(hUploaderhandle);
        //smu_t[i].in_use = FALSE;
        //LEaudio_uploader_deinit();
        return -1;
    }

    // channel num need event!
    //smu_t[i].hUploaderhandle = hUploaderhandle;
    //smu_t[i].in_use = TRUE;
    //auc.leaudio_uploader_in_use = TRUE;
    //BTMW_RPC_TEST_Logd("[BMS]checkhandle, %ld -- %ld, ", hUploaderhandle, LEaudio_uploader_get_handle(smu_t[i].socketidx));
    #if 0
    int i4_ret = 0;
    // set bluetooth volume 100
    DRV_COMP_ID_T t_comp_id = { .ui2_id = 0};
    AUD_DEC_VOLUME_INFO_EX_T t_aud_vol_info = {
        .u.ui1_level = 100,
        .e_vol_type = AUD_DEC_ALL_CH,
        .e_out_port = AUD_DEC_OUT_PORT_BLUETOOTH,
    };
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_VOLUME_EX, &t_aud_vol_info, sizeof(AUD_DEC_VOLUME_INFO_EX_T), &i4_ret);

    // 1st, get current out port type
    int ui4_drv_out_port_mask = 0;
    SIZE_T z_outportsize = sizeof(int);
    MTAUDDEC_AudGet(&t_comp_id, AUD_DEC_GET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, &z_outportsize, &i4_ret);

    // 2nd, add bluetooth type to out port type
    ui4_drv_out_port_mask |= AUD_DEC_OUT_PORT_FLAG_BLUETOOTH;
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, sizeof(int), &i4_ret);
    BTMW_RPC_TEST_Logd("[BMS] start_leaudio_uploader %ld", hUploaderhandle);
    #endif
    return hUploaderhandle;
}

int stop_leaudio_uploader()
{
    int ret = 0;
    int i4_ret = 0;
    // 1st, get current out port type
    DRV_COMP_ID_T t_comp_id = { .ui2_id = 0};
    int ui4_drv_out_port_mask = 0;
    SIZE_T z_outportsize = sizeof(int);
    MTAUDDEC_AudGet(&t_comp_id, AUD_DEC_GET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, &z_outportsize, &i4_ret);

    // 2nd, remove bluetooth type to out port type
    ui4_drv_out_port_mask &= ~(AUD_DEC_OUT_PORT_FLAG_BLUETOOTH);
    MTAUDDEC_AudSet(&t_comp_id, AUD_DEC_SET_TYPE_OUT_PORT, &ui4_drv_out_port_mask, sizeof(int), &i4_ret);
    for (int i=0; i<BT_BROADCAST_SOCKET_INDEX_NUM; i++) {
        BTMW_RPC_TEST_Logd("[BMS] uploader stop Line %d, i %d, in_use %d!!\n", __LINE__, i, smu_t[i].in_use);
        if (smu_t[i].in_use) {
            LEaudio_uploader_stop(smu_t[i].hUploaderhandle);
        }
    }
    for (int i=0; i<BT_BROADCAST_SOCKET_INDEX_NUM; i++) {
        BTMW_RPC_TEST_Logd("[BMS] uploader close Line %d, i %d, in_use %d!!\n", __LINE__, i, smu_t[i].in_use);
        if (smu_t[i].in_use) {
            LEaudio_uploader_close(smu_t[i].hUploaderhandle);
            smu_t[i].in_use = false;
        }
    }
    ret = LEaudio_uploader_deinit();
    auc.leaudio_uploader_in_use = false;
    return ret;
}
#endif

static VOID btmw_rpc_test_bms_show_socket_index(BT_BMS_EVENT_SOCKET_INDEX_DATA *socket_index)
{
    BTMW_RPC_TEST_Logd("[BMS] instance_id=%d socket_index_num=%d\n",
      socket_index->instance_id, socket_index->socket_index_num);

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    //LEaudio_uploader_init();
    _gbms_socket_t.instance_id = socket_index->instance_id;
    _gbms_socket_t.socket_index_num = socket_index->socket_index_num;
    memcpy(_gbms_socket_t.socket_index_list, socket_index->socket_index_list, sizeof(_gbms_socket_t.socket_index_list));

    for (UINT8 i = 0; i < socket_index->socket_index_num; i++)
    {
        BTMW_RPC_TEST_Logd("[BMS] i=%d: socket_index=%d, channel_num=%d, subgroup_num=%d", i,
          socket_index->socket_index_list[i].socket_index,
          socket_index->socket_index_list[i].channel_num,
          socket_index->socket_index_list[i].subgroup_num);

#if 0
        for (UINT8 j = 0; j < socket_index->socket_index_list[i].subgroup_num; j++)
        {
            BTMW_RPC_TEST_Logd("[BMS] subgroup[%d]=%d", j,
              socket_index->socket_index_list[i].subgroup_ids[j]);
        }
#endif

        smu_t[i].socketidx = (audio_io_handle_t)socket_index->socket_index_list[i].socket_index;
        if (!auc.a2dpsrc_uploader_in_use && !auc.a2dpsnk_playback_in_use)
        {
            long hUploaderhandle = start_leaudio_uploader(
              socket_index->socket_index_list[i].socket_index,
              socket_index->socket_index_list[i].channel_num);
            if (hUploaderhandle == -1) continue;
            smu_t[i].hUploaderhandle = hUploaderhandle;
            smu_t[i].in_use = TRUE;
            auc.leaudio_uploader_in_use = TRUE;
        }
    }
    if (!auc.leaudio_uploader_in_use)
    {
        LEaudio_uploader_deinit();//if cannot start any one uploader, so deinit!
    }
#endif
}

static VOID btmw_rpc_test_bms_show_iso_status(BT_BMS_EVENT_ISO_STATUS_DATA *iso_status)
{
    BOOL all_iso_removed = TRUE;

    for (UINT8 i = 0; i < _gbms_socket_t.socket_index_num; i++)
    {
        if (_gbms_socket_t.socket_index_list[i].socket_index == iso_status->socket_index)
        {
            _gbms_socket_t.socket_index_list[i].iso_status = iso_status->up;
            break;
        }
    }

    if (iso_status->up)
    {
        BTMW_RPC_TEST_Logd("[BMS] instance_id=%d socket_index %d's iso all setup, broadcaster streaming now\n",
          iso_status->instance_id, iso_status->socket_index);

#if defined(BMS_RECONFIG_AUTO_TEST) && (BMS_RECONFIG_AUTO_TEST == TRUE)
        if (g_reconfig_stress_test)
        {

            for (UINT8 i = 0; i < _gbms_socket_t.socket_index_num; i++)
            {
                if (!_gbms_socket_t.socket_index_list[i].iso_status)
                {
                    //if any one not setup done, return
                    return;
                }
            }

            if (g_reconfig_num%2!=0) //even
            {
                //reconfig 2bis
                BTMW_RPC_TEST_Logd("[BMS][Auto Test] ============= g_reconfig_num=%d, reconfig to 2bis\n", g_reconfig_num);
                CHAR announcement_xml_path_2bis[BTMW_RPC_TEST_BMS_XML_NAME_LEN] = "/data/bms_param_xml/test_leaudio_base_param_1big2sub1bis_total2bis_mono.xml";
                memcpy(g_announcement_xml_path, announcement_xml_path_2bis, BTMW_RPC_TEST_BMS_XML_NAME_LEN);
            }
            else //odd
            {
                //reconfig 4bis
                BTMW_RPC_TEST_Logd("[BMS][Auto Test] ============= g_reconfig_num=%d, reconfig to 4bis\n", g_reconfig_num);
                CHAR announcement_xml_path_4bis[BTMW_RPC_TEST_BMS_XML_NAME_LEN] = "/data/bms_param_xml/test_leaudio_base_param_1big4sub1bis_total4bis_mono.xml";
                memcpy(g_announcement_xml_path, announcement_xml_path_4bis, BTMW_RPC_TEST_BMS_XML_NAME_LEN);
            }

            g_reconfig_num++;

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
              if (g_current_broadcast_state == BT_BMS_STREAMING)
              {
                  if (stop_leaudio_uploader() == -1)
                  {
                      BTMW_RPC_TEST_Logw("[BMS][Auto Test] Uploader did not be inited, no need stop");
                      return;
                  }
                  g_pending_cmd = BT_BMS_RECONFIG;
                  return;
              }
#endif
        }
#endif
     } else {
        BTMW_RPC_TEST_Logd("[BMS] instance_id=%d socket_index %d's iso all removed, broadcaster not streaming now\n",
          iso_status->instance_id, iso_status->socket_index);

        for (UINT8 i = 0; i < _gbms_socket_t.socket_index_num; i++)
        {
            if (_gbms_socket_t.socket_index_list[i].iso_status)
            {
                all_iso_removed = FALSE;
                break;
            }
        }

        if (all_iso_removed)
        {
            //can do pause/stop/reconfig now
            switch (g_pending_cmd)
            {
              case BT_BMS_PAUSE:
                  BTMW_RPC_TEST_Logi("[BMS] pause broadcast\n");
                  BT_BMS_PAUSE_BROADCAST_PARAM pause_param;
                  memset(&pause_param, 0, sizeof(pause_param));
                  pause_param.instance_id = g_instance_id;
                  a_mtkapi_bt_bms_pause_broadcast(&pause_param);
                break;
              case BT_BMS_STOP:
                  BTMW_RPC_TEST_Logi("[BMS] stop broadcast\n");
                  BT_BMS_STOP_BROADCAST_PARAM stop_param;
                  memset(&stop_param, 0, sizeof(stop_param));
                  stop_param.instance_id = g_instance_id;
                  a_mtkapi_bt_bms_stop_broadcast(&stop_param);
                break;
              case BT_BMS_STOP_ALL:
                  BTMW_RPC_TEST_Logi("[BMS] stop all broadcast\n");
                  a_mtkapi_bt_bms_stop_all_broadcast();
                break;
              case BT_BMS_RECONFIG:
                  BTMW_RPC_TEST_Logi("[BMS] reconfig\n");
                  BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM reconfig_param;
                  memset(&reconfig_param, 0, sizeof(reconfig_param));
                  reconfig_param.instance_id = g_instance_id;
                  if (btmw_rpc_test_bms_load_announcement_from_xml(
                      &reconfig_param.announcement, g_announcement_xml_path) < 0)
                  {
                      BTMW_RPC_TEST_Loge("[BMS] load announcement from xml %s fail", g_announcement_xml_path);
                      return;
                  }
                  a_mtkapi_bt_bms_update_base_announcement(&reconfig_param);
                  break;
              default:
                  break;
            }
            g_pending_cmd = BT_BMS_CMD_NONE;
        }
    }
}

static VOID btmw_rpc_test_bms_show_own_address(BT_BMS_EVENT_OWN_ADDRESS_DATA *own_address)
{
    BTMW_RPC_TEST_Logd(" instance_id=%d addr_type=%d addr=%s\n", own_address->instance_id, own_address->addr_type, own_address->addr);
}

static VOID btmw_rpc_test_bms_app_cbk(BT_BMS_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("event=%d %s\n", param->event,
                        btmw_rpc_test_bms_app_event(param->event));
    switch (param->event)
    {
    case BT_BMS_EVENT_BROADCAST_CREATED:
        btmw_rpc_test_bms_show_broadcast_created(&param->data.broadcast_created);
        break;

    case BT_BMS_EVENT_BROADCAST_DESTORYED:
        btmw_rpc_test_bms_show_broadcast_destroyed(&param->data.broadcast_destroyed);
        break;

    case BT_BMS_EVENT_BROADCAST_STATE:
        btmw_rpc_test_bms_show_broadcast_state(&param->data.broadcast_state);
        break;

    case BT_BMS_EVENT_SOCKET_INDEX_NOTIFY:
        btmw_rpc_test_bms_show_socket_index(&param->data.socket_index);
        break;

    case BT_BMS_EVENT_ISO_STATUS:
        btmw_rpc_test_bms_show_iso_status(&param->data.iso_status);
        break;

    case BT_BMS_EVENT_OWN_ADDRESS:
        btmw_rpc_test_bms_show_own_address(&param->data.own_address);
        break;

    default:
        break;
    }
}

static int btmw_rpc_test_bms_broadcast_1bis_mono(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_mono <localname> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_mono broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[1] && (strlen(argv[1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_mono broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = TRUE;
    g_current_broadcast_info = BT_BMS_2BIS_MONO;

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    param.announcement.bas_config_name = BASE_CONFIG_48_2_2_HR;
    param.announcement.subgroup_num = 1;
    param.announcement.subgroup[0].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[0].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[0].metadata_len = 0;
    param.announcement.subgroup[0].bis_num = 1;
    param.announcement.subgroup[0].bis[0].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_FRONTLEFT;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[1]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_broadcast_2bis_mono(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_mono <localname> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_mono broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[1] && (strlen(argv[1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_mono broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = TRUE;
    g_current_broadcast_info = BT_BMS_2BIS_MONO;

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    param.announcement.bas_config_name = BASE_CONFIG_48_2_2_HR;
    param.announcement.subgroup_num = 1;
    param.announcement.subgroup[0].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[0].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[0].metadata_len = 0;
    param.announcement.subgroup[0].bis_num = 2;
    param.announcement.subgroup[0].bis[0].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_FRONTLEFT;
    param.announcement.subgroup[0].bis[1].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_FRONTRIGHT;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[1]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_broadcast_4bis_mono(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_4bis_mono <localname> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_4bis_mono broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[1] && (strlen(argv[1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_4bis_mono broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = TRUE;
    g_current_broadcast_info = BT_BMS_4BIS_MONO;

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    param.announcement.bas_config_name = BASE_CONFIG_48_2_2_HR;
    param.announcement.subgroup_num = 1;
    param.announcement.subgroup[0].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[0].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[0].metadata_len = 0;
    param.announcement.subgroup[0].bis_num = 4;
    param.announcement.subgroup[0].bis[0].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_FRONTLEFT;
    param.announcement.subgroup[0].bis[1].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_FRONTRIGHT;
    param.announcement.subgroup[0].bis[2].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_BACKLEFT;
    param.announcement.subgroup[0].bis[3].audio_channel_allocation = (UINT32) BT_LE_AUDIO_ALLOCATION_BACKRIGHT;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[1]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_broadcast_1bis_stereo(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_stereo <localname> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_stereo broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[1] && (strlen(argv[1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_1bis_stereo broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = TRUE;
    g_current_broadcast_info = BT_BMS_1BIS_STEREO;

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    param.announcement.bas_config_name = BASE_CONFIG_48_2_2_HR;
    param.announcement.subgroup_num = 1;
    param.announcement.subgroup[0].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[0].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[0].metadata_len = 0;
    param.announcement.subgroup[0].bis_num = 1;
    param.announcement.subgroup[0].bis[0].audio_channel_allocation = (UINT32) LE_AUDIO_ALLOCATION_STEREO_LR;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[1]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_broadcast_2bis_stereo(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_stereo <localname> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_stereo broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[1] && (strlen(argv[1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_2bis_stereo broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = TRUE;
    g_current_broadcast_info = BT_BMS_2BIS_STEREO;

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    param.announcement.bas_config_name = BASE_CONFIG_48_2_2_HR;
    param.announcement.subgroup_num = 2;
    param.announcement.subgroup[0].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[0].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[0].metadata_len = 0;
    param.announcement.subgroup[0].bis_num = 1;
    param.announcement.subgroup[0].bis[0].audio_channel_allocation = (UINT32) LE_AUDIO_ALLOCATION_STEREO_LR;
    param.announcement.subgroup[1].codec_config_name = CODEC_CONFIG_48_2;
    param.announcement.subgroup[1].audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
    param.announcement.subgroup[1].metadata_len = 0;
    param.announcement.subgroup[1].bis_num = 1;
    param.announcement.subgroup[1].bis[0].audio_channel_allocation = (UINT32) LE_AUDIO_ALLOCATION_STEREO_LR;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[1]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_create_broadcast(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_PARAM param;
    UINT8 index = 0;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast <localname> <metadata_len> [metadata] <profile> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast profile: 0 or 1\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    if ((atoi(argv[1]) != 0))
    {
      //medadata ACSII to Hex
      btmw_rpc_test_bms_metadata_acsii_2_hex(&param.metadata_len, param.metadata, argv[2]);
      index = 3;
    }
    else
    {
      param.metadata_len = 0;
      index = 2;
    }

    if ((atoi(argv[index]) != BT_BMS_SONIFICATION) && (atoi(argv[index]) != BT_BMS_MEDIA))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast <profile> should be %d or %d\n",
            BT_BMS_SONIFICATION, BT_BMS_MEDIA);
        return -1;
    }

    if (argv[index+1] && (strlen(argv[index+1]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[1]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    g_flag_auto = FALSE;
    g_current_broadcast_info = BT_BMS_2BIS_MONO;

    param.profile = (BT_BMS_PROFILE_NAME)atoi(argv[index]);

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[index+1]);

    return a_mtkapi_bt_bms_create_broadcast(&param);
}

static int btmw_rpc_test_bms_create_broadcast_ext(int argc, char **argv)
{
    BT_BMS_CREATE_BROADCAST_EXT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id != 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] already exist a broadcast, please stop it first\n");
        return -1;
    }

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_ext <localname> <announcement_xml> [broadcast_code]\n");
        BTMW_RPC_TEST_Logi("  BMS create_broadcast_ext broadcast_code: 16 octets or null\n");
        return -1;
    }

    if (strlen(argv[0]) > MAX_NAME_LEN)
    {
        BTMW_RPC_TEST_Logi("localname length should not larger than %d!\n", MAX_NAME_LEN);
        return -1;
    }

    if (argv[2] && (strlen(argv[2]) != BT_BROADCAST_BROADCAST_CODE_SIZE*2)) {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS btmw_rpc_test_bms_create_broadcast_ext broadcast_code length is %d, should be 0 or %d*2\n",
            strlen(argv[2]), BT_BROADCAST_BROADCAST_CODE_SIZE);
        return -1;
    }

    //localname
    param.localname_len = strlen(argv[0]);
    strncpy(param.localname, argv[0], param.localname_len);
    BTMW_RPC_TEST_Logd("localname is: %s\n", argv[0]);

    //xml
    strncpy(g_announcement_xml_path, argv[1], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
    g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';

    BTMW_RPC_TEST_Logi("[BMS] load announcement from %s\n", argv[1]);
    if (btmw_rpc_test_bms_load_announcement_from_xml(&param.announcement, g_announcement_xml_path) < 0)
    {
        BTMW_RPC_TEST_Loge("[BMS] load announcement from xml %s fail", g_announcement_xml_path);
        return -1;
    }

    g_flag_auto = FALSE;
    g_current_broadcast_info = BT_BMS_BROADCAST_XML;

    //broadcast code ACSII to Hex
    btmw_rpc_test_bms_broadcastcode_acsii_2_hex(&param.broadcast_code_len, param.broadcast_code, argv[2]);

    return a_mtkapi_bt_bms_create_broadcast_ext(&param);
}

static int btmw_rpc_test_bms_update_base_announcement(int argc, char **argv)
{
    BT_BMS_UPDATE_BASE_ANNOUNCEMENT_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS] btmw_rpc_test_bms_update_base_announcement\n");

    if (g_instance_id == 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, cannot update_base_announcement\n");
        return -1;
    }

    if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] update_base_announcement lastest instance_id %d\n", g_instance_id);
        strncpy(g_announcement_xml_path, argv[0], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
        g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';
        param.instance_id = g_instance_id;
    }
    else if ((argc == 2) && (strlen(argv[0]) == 1)) //cli with instance_id
    {
        BTMW_RPC_TEST_Logi("[BMS] update_base_announcement instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        strncpy(g_announcement_xml_path, argv[1], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
        g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';
        param.instance_id = (UINT8)atoi(argv[0]);
    }
#if defined(BMS_RECONFIG_AUTO_TEST) && (BMS_RECONFIG_AUTO_TEST == TRUE)
    else if ((argc == 2) && (strlen(argv[0]) != 1))
    {
        BTMW_RPC_TEST_Logi("[BMS][Auto Test] update_base_announcement lastest instance_id %d\n", g_instance_id);
        if (g_current_broadcast_state != BT_BMS_STREAMING)
        {
            BTMW_RPC_TEST_Logi("[BMS] please start broadcast first before auto test\n");
            return -1;
        }

        strncpy(g_announcement_xml_path, argv[0], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
        g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';
        param.instance_id = g_instance_id;

        if ((UINT8)atoi(argv[1]) == 1)
        {
            BTMW_RPC_TEST_Logi("[BMS][Auto Test] auto run reconfig stress test\n");
            g_reconfig_stress_test = TRUE;
        }
    }
    else if ((argc == 3) && (strlen(argv[0]) == 1)) //cli with instance_id
    {
        BTMW_RPC_TEST_Logi("[BMS][Auto Test] update_base_announcement instance_id %d\n", (UINT8)atoi(argv[0]));
        if (g_current_broadcast_state != BT_BMS_STREAMING)
        {
            BTMW_RPC_TEST_Logi("[BMS] please start broadcast first before auto test\n");
            return -1;
        }

        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS][Auto Test] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        strncpy(g_announcement_xml_path, argv[1], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
        g_announcement_xml_path[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';
        param.instance_id = (UINT8)atoi(argv[0]);

        if ((UINT8)atoi(argv[2]) == 1)
        {
            BTMW_RPC_TEST_Logi("[BMS][Auto Test] auto run reconfig stress test\n");
            g_reconfig_stress_test = TRUE;
        }
    }
#endif
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS update_base_announcement [instance_id] <announcement_xml>\n");
        return -1;
    }

    if (btmw_rpc_test_bms_check_announcement_xml(g_announcement_xml_path) != 0)
    {
#if defined(BMS_RECONFIG_AUTO_TEST) && (BMS_RECONFIG_AUTO_TEST == TRUE)
        g_reconfig_stress_test = FALSE;
#endif
        return -1;
    }

    g_current_broadcast_info = BT_BMS_BROADCAST_XML;

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (g_current_broadcast_state == BT_BMS_STREAMING)
    {
        if (stop_leaudio_uploader() == -1)
        {
            BTMW_RPC_TEST_Logw("[BMS] Uploader did not be inited, no need stop");
            return a_mtkapi_bt_bms_update_base_announcement(&param);
        }
        g_pending_cmd = BT_BMS_RECONFIG;
        return 0;
    }
#endif
    if (btmw_rpc_test_bms_load_announcement_from_xml(&param.announcement, g_announcement_xml_path) < 0)
    {
        BTMW_RPC_TEST_Loge("[BMS] load announcement from xml %s fail", g_announcement_xml_path);
        return -1;
    }
    return a_mtkapi_bt_bms_update_base_announcement(&param);
}

static int btmw_rpc_test_bms_update_subgroup_metadata(int argc, char **argv)
{
    BT_BMS_UPDATE_SUBGROUP_METADATA_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id == 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, cannot update_base_announcement\n");
        return -1;
    }

    if (argc == 2)
    {
        BTMW_RPC_TEST_Logi("[BMS] update_subgroup_metadata lastest instance_id %d\n", g_instance_id);
        strncpy(g_announcement_xml_path, argv[0], BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);

        param.instance_id = g_instance_id;
        param.subgroup_id = (UINT8)atoi(argv[0]);
        //medadata ACSII to Hex
        btmw_rpc_test_bms_metadata_acsii_2_hex(&param.metadata_len, param.metadata, argv[1]);

        if (strlen(argv[1]) > BT_BROADCAST_METADATA_SIZE*2) {
            BTMW_RPC_TEST_Logi("Usage :\n");
            BTMW_RPC_TEST_Logi("  BMS update_subgroup_metadata metadata size is %d, should be less than %d*2\n",
                strlen(argv[1]), BT_BROADCAST_METADATA_SIZE);
            return -1;
        }
    }
    else if (argc == 3)
    {
        BTMW_RPC_TEST_Logi("[BMS] update_subgroup_metadata instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize for multi big later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
        param.subgroup_id = (UINT8)atoi(argv[1]);
        //medadata ACSII to Hex
        btmw_rpc_test_bms_metadata_acsii_2_hex(&param.metadata_len, param.metadata, argv[2]);

        if (strlen(argv[2]) > BT_BROADCAST_METADATA_SIZE*2) {
            BTMW_RPC_TEST_Logi("Usage :\n");
            BTMW_RPC_TEST_Logi("  BMS update_subgroup_metadata metadata size is %d, should be less than %d*2\n",
                strlen(argv[2]), BT_BROADCAST_METADATA_SIZE);
            return -1;
        }
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS update_subgroup_metadata [instance_id] <subgroup_id> <metadata>\n");
        return -1;
    }

    return a_mtkapi_bt_bms_update_subgroup_metadata(&param);
}

static int btmw_rpc_test_bms_start_broadcast(int argc, char **argv)
{
    BT_BMS_START_BROADCAST_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_current_broadcast_state != BT_BMS_PAUSED)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast or already started, broadcast state is %s\n",
          btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));
        return -1;
    }

    if (argc == 0)
    {
        BTMW_RPC_TEST_Logi("[BMS] start_broadcast lastest instance_id %d\n", g_instance_id);
        param.instance_id = g_instance_id;
    }
    else if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] start_broadcast instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS start_broadcast [instance_id]\n");
        return -1;
    }

    BTMW_RPC_TEST_Logi("[BMS] instance_id=%d\n", param.instance_id);
    return a_mtkapi_bt_bms_start_broadcast(&param);
}

static int btmw_rpc_test_bms_start_broadcast_multi_thread(int argc, char **argv)
{
    BT_BMS_START_BROADCAST_MULTI_THREAD_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_current_broadcast_state != BT_BMS_PAUSED)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast or already started, broadcast state is %s\n",
          btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));
        return -1;
    }

    if (argc == 0)
    {
        BTMW_RPC_TEST_Logi("[BMS] start_broadcast_multi_thread lastest instance_id %d\n", g_instance_id);
        param.instance_id = g_instance_id;
    }
    else if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] start_broadcast_multi_thread instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS start_broadcast_multi_thread [instance_id]\n");
        return -1;
    }

    BTMW_RPC_TEST_Logi("[BMS] instance_id=%d\n", param.instance_id);
    return a_mtkapi_bt_bms_start_broadcast_multi_thread(&param);
}

static int btmw_rpc_test_bms_pause_broadcast(int argc, char **argv)
{
    BT_BMS_PAUSE_BROADCAST_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_current_broadcast_state != BT_BMS_STREAMING)
    {
        BTMW_RPC_TEST_Logi("[BMS] no streaming, no need pause, broadcast state is %s\n",
          btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));
        return -1;
    }

    if (argc == 0)
    {
        BTMW_RPC_TEST_Logi("[BMS] pause_broadcast lastest instance_id %d\n", g_instance_id);
        param.instance_id = g_instance_id;
    }
    else if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] pause_broadcast instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS pause_broadcast [instance_id]\n");
        return -1;
    }

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (g_current_broadcast_state == BT_BMS_STREAMING)
    {
        if (stop_leaudio_uploader() == -1)
        {
            BTMW_RPC_TEST_Logw("[BMS] Uploader did not be inited, no need stop");
            return a_mtkapi_bt_bms_pause_broadcast(&param);
        }
        g_pending_cmd = BT_BMS_PAUSE;
        return 0;
    }
    return a_mtkapi_bt_bms_pause_broadcast(&param);
#else
    return a_mtkapi_bt_bms_pause_broadcast(&param);
#endif
}

static int btmw_rpc_test_bms_stop_broadcast(int argc, char **argv)
{
    BT_BMS_STOP_BROADCAST_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_current_broadcast_state == BT_BMS_STOPPED)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, no need stop, broadcast state is %s\n",
          btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));
        return -1;
    }

    if (argc == 0)
    {
        BTMW_RPC_TEST_Logi("[BMS] stop_broadcast lastest instance_id %d\n", g_instance_id);
        param.instance_id = g_instance_id;
    }
    else if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] stop_broadcast instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS stop_broadcast [instance_id]\n");
        return -1;
    }

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (g_current_broadcast_state == BT_BMS_STREAMING)
    {
        if (stop_leaudio_uploader() == -1)
        {
            BTMW_RPC_TEST_Logw("[BMS] Uploader did not be inited, no need stop");
            return a_mtkapi_bt_bms_stop_broadcast(&param);
        }
        g_pending_cmd = BT_BMS_STOP;
        return 0;
    }
    return a_mtkapi_bt_bms_stop_broadcast(&param);
#else
    return a_mtkapi_bt_bms_stop_broadcast(&param);
#endif
}

static int btmw_rpc_test_bms_stop_all_broadcast(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_current_broadcast_state == BT_BMS_STOPPED)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, no need stop, broadcast state is %s\n",
          btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));
        return -1;
    }

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (g_current_broadcast_state == BT_BMS_STREAMING)
    {
        if (stop_leaudio_uploader() == -1)
        {
            BTMW_RPC_TEST_Logw("[BMS] Uploader did not be inited, no need stop");
            return a_mtkapi_bt_bms_stop_all_broadcast();
        }
        g_pending_cmd = BT_BMS_STOP_ALL;
        return 0;
    }
    return a_mtkapi_bt_bms_stop_all_broadcast();
#else
    return a_mtkapi_bt_bms_stop_all_broadcast();
#endif
}

static int btmw_rpc_test_bms_get_own_address(int argc, char **argv)
{
    BT_BMS_GET_OWN_ADDRESS_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id == 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, cannot get broadcast address\n");
        return -1;
    }

    if (argc == 0)
    {
        BTMW_RPC_TEST_Logi("[BMS] get_own_address lastest instance_id %d\n", g_instance_id);
        param.instance_id = g_instance_id;
    }
    else if (argc == 1)
    {
        BTMW_RPC_TEST_Logi("[BMS] get_own_address instance_id %d\n", (UINT8)atoi(argv[0]));
        if ((UINT8)atoi(argv[0]) != g_instance_id)
        {
            //not support multi big now, need optimize later
            BTMW_RPC_TEST_Loge("[BMS] wrong instance_id %d\n", (UINT8)atoi(argv[0]));
            return -1;
        }
        param.instance_id = (UINT8)atoi(argv[0]);
    }
    else
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BMS get_own_address [instance_id]\n");
        return -1;
    }

    return a_mtkapi_bt_bms_get_own_address(&param);
}

static int btmw_rpc_test_bms_get_all_broadcasts_states(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id == 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast, cannot get broadcast states\n");
        return -1;
    }

    return a_mtkapi_bt_bms_get_all_broadcasts_states();
}

static int btmw_rpc_test_bms_get_broadcast_info(int argc, char **argv)
{
    BT_BMS_STOP_BROADCAST_PARAM param;
    memset(&param, 0, sizeof(param));

    BTMW_RPC_TEST_Logi("[BMS]\n");

    if (g_instance_id == 0xFF)
    {
        BTMW_RPC_TEST_Logi("[BMS] no broadcast\n");
        return -1;
    }

    switch (g_current_broadcast_info)
    {
        case BT_BMS_2BIS_MONO:
          BTMW_RPC_TEST_Logi("[BMS] ============broadcast 2bis mono==========\n");
          break;
        case BT_BMS_4BIS_MONO:
          BTMW_RPC_TEST_Logi("[BMS] ============broadcast 4bis mono==========\n");
          break;
        case BT_BMS_1BIS_STEREO:
          BTMW_RPC_TEST_Logi("[BMS] ============broadcast 1bis stereo==========\n");
          break;
        case BT_BMS_2BIS_STEREO:
          BTMW_RPC_TEST_Logi("[BMS] ============broadcast 2bis stereo==========\n");
          break;
        case BT_BMS_BROADCAST_XML:
          BTMW_RPC_TEST_Logi("[BMS] ============broadcast xml %s==========\n", g_announcement_xml_path);
          break;
        default:
          BTMW_RPC_TEST_Loge("[BMS] no broadcast info\n");
          break;
    }

    BTMW_RPC_TEST_Logd("[BMS] ============instance_id %d==========\n", g_instance_id);
    BTMW_RPC_TEST_Logd("[BMS] ============status %s==========\n",
                        btmw_rpc_test_bms_broadcast_state(g_current_broadcast_state));

    return 0;
}

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
INT32 bms_src_init_dl_function(VOID)
{
    fHandle1 = dlopen("libhandle_app.so",RTLD_LAZY|RTLD_DEEPBIND);
    if (!fHandle1)
    {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("[BMS] load libhandle_app.so fail error:%s", err_str?err_str:"unknown");
        return -1;
    }
    bms_handle_init = dlsym(fHandle1, "handle_init");

    fHandle2 = dlopen("libdtv_osai.so",RTLD_LAZY|RTLD_DEEPBIND);
    if (!fHandle2)
    {
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("[BMS] load libdtv_osai.so fail error:%s", err_str?err_str:"unknown");
        (void)dlclose(fHandle1);
        return -1;
    }
    bms_c_rpc_start_client = dlsym(fHandle2, "c_rpc_start_client");
    bms_os_init = dlsym(fHandle2, "os_init");
    bms_x_rtos_init = dlsym(fHandle2, "x_rtos_init");

    app_handle = dlopen("libapp_if_rpc.so", RTLD_LAZY);
    if (!app_handle)
    {
        (void)dlclose(fHandle1);
        (void)dlclose(fHandle2);
        char const *err_str = dlerror();
        BTMW_RPC_TEST_Loge("[BMS] load libapp_if_rpc.so fail error:%s", err_str?err_str:"unknown");
        return -1;
    }
    bms_c_rpc_init_client = dlsym(app_handle, "c_rpc_init_client");
    dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state = dlsym(app_handle, "a_mtktvapi_bluetooth_agent_update_conn_state");
    if (dl_a_mtktvapi_bluetooth_agent_update_bms_conn_state == NULL)
    {
        BTMW_RPC_TEST_Loge("[BMS] Failed to get update conn state\n");
        return -1;
    }

    return 0;
}

static INT32 bms_rpc_env_init( VOID )
{
    INT32 i4_ret = 0;
    GEN_CONFIG_T t_rtos_config;
    VOID* pv_mem_addr = 0;
    UINT32 z_mem_size = 0xc00000;

    if (bms_binit == 1)
    {
        return 0;
    }
    BTMW_RPC_TEST_Loge("[BMS] bms_rpc_env_init ret=%d\n", i4_ret);
    memset( &t_rtos_config, 0, sizeof( GEN_CONFIG_T ) );
    i4_ret = bms_x_rtos_init ( &t_rtos_config );
    if (i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("[BMS] x_rtos_init ret=%d\n", i4_ret);
        return -1;
    }
    i4_ret = bms_handle_init(MAX_NUM_HANDLES, &pv_mem_addr, &z_mem_size);
    if (i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("[BMS] handle_init ret=%d\n", i4_ret);
        return -2;
    }
    i4_ret = bms_os_init(pv_mem_addr, z_mem_size);
    if (i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("[BMS] os_init ret=%d\n", i4_ret);
        return -3;
    }

    i4_ret = bms_c_rpc_init_client();
    if (i4_ret !=0)
    {
        BTMW_RPC_TEST_Loge("[BMS] c_rpc_init_client ret=%d\n", i4_ret);
        return -4;
    }
    i4_ret = bms_c_rpc_start_client();
    if (i4_ret <0)
    {
        BTMW_RPC_TEST_Loge("[BMS] c_rpc_start_client ret=%d\n", i4_ret);
        return -5;
    }

    bms_binit = 1;
    return 0;
}
#endif


int btmw_rpc_test_bms_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD bms_mod = {0};

    bms_mod.mod_id = BTMW_RPC_TEST_MOD_LEAUDIO_BMS;
    strncpy(bms_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_BMS, sizeof(bms_mod.cmd_key));
    bms_mod.cmd_handler = btmw_rpc_test_bms_cmd_handler;
    bms_mod.cmd_tbl = btmw_rpc_test_bms_cli_commands;

    ret = btmw_rpc_test_register_mod(&bms_mod);
    BTMW_RPC_TEST_Logd("[BMS] btmw_rpc_test_register_mod() returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        a_mtkapi_bt_bms_register_callback(btmw_rpc_test_bms_app_cbk, NULL);
    }

#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    ret = bms_src_init_dl_function();
    if (ret != 0)
    {
        return -1;
    }

    //init rpc
    ret = bms_rpc_env_init();
    if (ret != 0)
    {
        BTMW_RPC_TEST_Loge("[BMS] rpc_env_init fail ret=%d\n", ret);
        return -1;
    }
#endif

    return ret;
}

int btmw_rpc_test_bms_deinit()
{
    BTMW_RPC_TEST_Logi("[BMS]\n");
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    if (fHandle1 != NULL)
    {
        (void)dlclose(fHandle1);
    }
    if (fHandle2 != NULL)
    {
        (void)dlclose(fHandle2);
    }
    if (app_handle != NULL)
    {
        (void)dlclose(app_handle);
    }
#endif

    return 0;
}

static int btmw_rpc_test_bms_check_announcement_xml(CHAR *announcement_xml_path)
{
    BTMW_RPC_TEST_Logi("[BMS] xml is %s\n", announcement_xml_path);

    if (strlen(announcement_xml_path) >= BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1)
    {
        BTMW_RPC_TEST_Loge("[BMS] xml file name (%s) too long", announcement_xml_path);
        return -1;
    }

    if (xmlReadFile(announcement_xml_path, NULL, 0) == NULL)
    {
        BTMW_RPC_TEST_Loge("[BMS] cannot find xml file %s !!!", announcement_xml_path);
        return -1;
    }

    return 0;
}

static int btmw_rpc_test_bms_load_announcement_from_xml(
    BT_BMS_ANNOUNCEMENT *announcement, CHAR *announcement_xml_path)
{
    xmlNode *big_node = NULL;

    BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML *announcement_xml =
        malloc(sizeof(BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML));

    if (NULL == announcement_xml)
    {
        BTMW_RPC_TEST_Loge("[BMS] alloc services fail");
        goto __ERROR;
    }

    memset((void*)announcement_xml, 0, sizeof(BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML));

    LIBXML_TEST_VERSION;

    if (btmw_rpc_test_bms_check_announcement_xml(announcement_xml_path) != 0)
    {
        BTMW_RPC_TEST_Loge("[BMS] check xml file fail: %s", announcement_xml_path);
        goto __ERROR;
    }

    strncpy(announcement_xml->xml_name, announcement_xml_path,
        BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1);
    announcement_xml->xml_name[BTMW_RPC_TEST_BMS_XML_NAME_LEN - 1] = '\0';
#if 0
    BTMW_RPC_TEST_Logi("[BMS] xml_name %s\n", announcement_xml->xml_name);
#endif

    announcement_xml->doc = xmlReadFile(announcement_xml_path, NULL, 0);
    if (announcement_xml->doc == NULL)
    {
        BTMW_RPC_TEST_Loge("[BMS] xmlReadFile %s fail", announcement_xml_path);
        goto __ERROR;
    }

    //<basic-audio-announcement>
    announcement_xml->xml_root = xmlDocGetRootElement(announcement_xml->doc);
    if (NULL == announcement_xml->xml_root)
    {
        BTMW_RPC_TEST_Loge("[BMS] xml root read fail");
        goto __ERROR;
    }

#if 0
    BTMW_RPC_TEST_Logi("[BMS] announcement_xml->xml_root->name %s\n", announcement_xml->xml_root->name);
#endif
    if (0 != xmlStrncmp(announcement_xml->xml_root->name,
        BAD_CAST BTMW_RPC_TEST_BMS_ANNOUNCEMENT,
        strlen(BTMW_RPC_TEST_BMS_ANNOUNCEMENT)))
    {
        BTMW_RPC_TEST_Loge("[BMS] xml root is not "BTMW_RPC_TEST_BMS_ANNOUNCEMENT);
        goto __ERROR;
    }

    //<big>
    big_node = announcement_xml->xml_root->xmlChildrenNode;
    while(NULL != big_node)
    {
        if (big_node->type == XML_ELEMENT_NODE)
        {
            //BTMW_RPC_TEST_Logi("[BMS] big_node->name %s\n", big_node->name);

            //big params:
            if (!btmw_rpc_test_bms_parse_big_param(announcement, big_node))
            {
                BTMW_RPC_TEST_Loge("[BMS] big param parse fail");
                goto __ERROR;
            }

#if 0
            BTMW_RPC_TEST_Logi("[BMS] big: announcement->bas_config_name %d, subgroup num is %d\n",
              announcement->bas_config_name, announcement->subgroup_num);
#endif

            if(0 == xmlStrncmp(big_node->name, BAD_CAST BTMW_RPC_TEST_BMS_BIG,
                strlen(BTMW_RPC_TEST_BMS_BIG)))
            {
                UINT8 subgroup_id = 0;
                xmlNode *subgroup_node = big_node->xmlChildrenNode;
                while(NULL != subgroup_node)
                {
                    if (subgroup_node->type == XML_ELEMENT_NODE)
                    {
                        //BTMW_RPC_TEST_Logi("[BMS] subgroup_node->name %s\n", subgroup_node->name);
                        if(0 == xmlStrncmp(subgroup_node->name, BAD_CAST BTMW_RPC_TEST_BMS_SUBGROUP,
                            strlen(BTMW_RPC_TEST_BMS_SUBGROUP)))
                        {
                            //subgroup params:
                            if (!btmw_rpc_test_bms_parse_subgroup_param(&announcement->subgroup[subgroup_id], subgroup_node))
                            {
                                BTMW_RPC_TEST_Loge("[BMS] subgroup[%d] param parse fail", subgroup_id);
                                goto __ERROR;
                            }

#if 0
                            BTMW_RPC_TEST_Logi("[BMS] subgroup[%d]: codec_config_name %d, bis num %d\n",
                                subgroup_id, announcement->subgroup[subgroup_id].codec_config_name,
                                announcement->subgroup[subgroup_id].bis_num);
#endif

                            UINT8 bis_id = 0;
                            xmlNode *bis_node = subgroup_node->xmlChildrenNode;
                            while(NULL != bis_node)
                            {
                                if (bis_node->type == XML_ELEMENT_NODE)
                                {
#if 0
                                    BTMW_RPC_TEST_Logi("[BMS] bis_node->name %s\n", bis_node->name);
#endif
                                    if(0 == xmlStrncmp(bis_node->name, BAD_CAST BTMW_RPC_TEST_BMS_BIS,
                                        strlen(BTMW_RPC_TEST_BMS_BIS)))
                                    {
                                        //bis params:
                                        if (!btmw_rpc_test_bms_parse_bis_param(
                                            &announcement->subgroup[subgroup_id].bis[bis_id], bis_node))
                                        {
                                            BTMW_RPC_TEST_Loge("[BMS] bis[%d] param parse fail", bis_id);
                                            goto __ERROR;
                                        }
                                        bis_id++;
                                        if (bis_id == announcement->subgroup[subgroup_id].bis_num)
                                            break;
                                    }
                                }
                                bis_node = bis_node->next;
                            }
                            if (0 == bis_id)
                            {
                                BTMW_RPC_TEST_Loge("[BMS] sugroup should have at least 1 "BTMW_RPC_TEST_BMS_BIS);
                                goto __ERROR;
                            }

                            subgroup_id++;
                            if (subgroup_id == announcement->subgroup_num)
                                break;
                        }
                    }
                    subgroup_node = subgroup_node->next;
                }

                if (0 == subgroup_id)
                {
                    BTMW_RPC_TEST_Loge("[BMS] big should have at least 1 "BTMW_RPC_TEST_BMS_SUBGROUP);
                    goto __ERROR;
                }

#if 0
                BTMW_RPC_TEST_Logi("[BMS] have found 1 big, done");
#endif
                break;
            }
        }
        big_node = big_node->next;
    }

#if 0
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[0].metadata_len %d, announcement->subgroup[0].bis_num %d",
        announcement->subgroup[0].metadata_len, announcement->subgroup[0].bis_num);
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[1].metadata_len %d, announcement->subgroup[1].bis_num %d",
        announcement->subgroup[1].metadata_len, announcement->subgroup[1].bis_num);
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[0].bis[0].codec_config_name %d, announcement->subgroup[0].bis[0].audio_channel_allocation 0x%08x",
        announcement->subgroup[0].bis[0].codec_config_name, announcement->subgroup[0].bis[0].audio_channel_allocation);
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[0].bis[1].codec_config_name %d, announcement->subgroup[0].bis[1].audio_channel_allocation 0x%08x",
        announcement->subgroup[0].bis[1].codec_config_name, announcement->subgroup[0].bis[1].audio_channel_allocation);
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[1].bis[0].codec_config_name %d, announcement->subgroup[1].bis[0].audio_channel_allocation 0x%08x",
        announcement->subgroup[1].bis[0].codec_config_name, announcement->subgroup[1].bis[0].audio_channel_allocation);
    BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup[1].bis[1].codec_config_name %d, announcement->subgroup[1].bis[1].audio_channel_allocation 0x%08x",
        announcement->subgroup[1].bis[1].codec_config_name, announcement->subgroup[1].bis[1].audio_channel_allocation);
#endif

    BTMW_RPC_TEST_Logi("[BMS] bms announcement parse finish");

    btmw_rpc_test_bms_free_announcement_xml(announcement_xml);
    return 0;

__ERROR:
    if (NULL != announcement_xml)
    {
        btmw_rpc_test_bms_free_announcement_xml(announcement_xml);
    }
    return -1;
}

static int btmw_rpc_test_bms_free_announcement_xml(
    BTMW_RPC_TEST_BMS_ANNOUNCEMENT_XML *announcement_xml)
{
    if (NULL == announcement_xml)
    {
        return 0;
    }

    BTMW_RPC_TEST_Logi("[BMS] free xml %s", announcement_xml->xml_name);

    if (NULL != announcement_xml->doc)
    {
        xmlFreeDoc(announcement_xml->doc);
        announcement_xml->doc = NULL;
    }

    free(announcement_xml);

    xmlCleanupParser();

    return 0;
}

static BOOL btmw_rpc_test_bms_parse_big_param(BT_BMS_ANNOUNCEMENT *announcement, xmlNode *node)
{
    BOOL bas_config_name_found = FALSE;
    BOOL subgroup_num_found = FALSE;
    xmlAttrPtr attr = node->properties;

    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_BAS_CONFIG_NAME,
            strlen(BTMW_RPC_TEST_BMS_ATTR_BAS_CONFIG_NAME)))
        {
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_BAS_CONFIG_NAME);

            announcement->bas_config_name = (BT_BMS_BASE_CONFIG_NAME)atoi((const char *)attr_value);

            //BTMW_RPC_TEST_Logi("[BMS] announcement->bas_config_name: %d",
            //    announcement->bas_config_name);

            bas_config_name_found = TRUE;
            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_SUBGROUP_NUM,
            strlen(BTMW_RPC_TEST_BMS_ATTR_SUBGROUP_NUM)))
        {
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_SUBGROUP_NUM);

            announcement->subgroup_num = (UINT8)atoi((const char *)attr_value);

            //BTMW_RPC_TEST_Logi("[BMS] announcement->subgroup_num: %d", announcement->subgroup_num);

            if ((announcement->subgroup_num < 1) || (announcement->subgroup_num > BT_BROADCAST_MAX_SUBGROUP_NUM))
            {
                BTMW_RPC_TEST_Loge("[BMS] subgroup_num should be 1 - %d", BT_BROADCAST_MAX_SUBGROUP_NUM);
                xmlFree(attr_value);
                return FALSE;
            }
            subgroup_num_found = TRUE;
            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    //BTMW_RPC_TEST_Logi("[BMS] btmw_rpc_test_bms_parse_big_param finish");
    return bas_config_name_found && subgroup_num_found;
}

static BOOL btmw_rpc_test_bms_parse_subgroup_param(BT_BMS_ANNOUNCEMENT_SUBGROUP *subgroup, xmlNode *node)
{
    BOOL codec_config_name_found = FALSE;
    BOOL bis_num_found = FALSE;
    xmlAttrPtr attr = node->properties;

    subgroup->audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;

    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_CODEC_CONFIG_NAME,
            strlen(BTMW_RPC_TEST_BMS_ATTR_CODEC_CONFIG_NAME)))
        {
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_CODEC_CONFIG_NAME);

            subgroup->codec_config_name = (BT_BMS_CODEC_CONFIG_NAME)atoi((const char *)attr_value);

            //BTMW_RPC_TEST_Logi("[BMS] subgroup->codec_config_name: %d", subgroup->codec_config_name);

            codec_config_name_found = TRUE;
            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION,
            strlen(BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION)))
        {
            UINT32 tmp = 0;
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION);

            if (0 == strlen((const char *)attr_value))
            {
                //BTMW_RPC_TEST_Logi("[BMS] subgroup->audio_channel_allocation is LE_AUDIO_ALLOCATION_INVALID");
                subgroup->audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
            }
            else
            {
                if (!strncmp((const char *)attr_value, "0x", 2))
                {
                    if (!sscanf((const char *)attr_value+2, "%8x", &tmp))
                    {
                        BTMW_RPC_TEST_Loge("[BMS] sscanf error");
                    }
                }
                else
                {
                    tmp = atoi((const char *)attr_value);
                }
                subgroup->audio_channel_allocation = tmp;
            }

            //BTMW_RPC_TEST_Logi("[BMS] subgroup->audio_channel_allocation: 0x%08x", subgroup->audio_channel_allocation);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_METADATA,
            strlen(BTMW_RPC_TEST_BMS_ATTR_METADATA)))
        {
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_METADATA);

            btmw_rpc_test_bms_metadata_acsii_2_hex(
                &subgroup->metadata_len, subgroup->metadata, (CHAR*)attr_value);

            //BTMW_RPC_TEST_Logw("[BMS] subgroup->metadata_len: %d", subgroup->metadata_len);
#if 0
            for (UINT8 i = 0; i < subgroup->metadata_len; i++)
            {
                BTMW_RPC_TEST_Logi("[BMS] metadata[%d] 0x%02x\n", i, subgroup->metadata[i]);
            }
#endif
            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_BIS_NUM,
            strlen(BTMW_RPC_TEST_BMS_ATTR_BIS_NUM)))
        {
            xmlChar *attr_value = xmlGetProp(node, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_BIS_NUM);

            subgroup->bis_num = (UINT8)atoi((const char *)attr_value);

            //BTMW_RPC_TEST_Logw("[BMS] subgroup->bis_num: %d", subgroup->bis_num);

            if ((subgroup->bis_num < 1) || (subgroup->bis_num > BT_BROADCAST_MAX_BIS_NUM))
            {
                BTMW_RPC_TEST_Loge("[BMS] bis_num should be 1 - %d", BT_BROADCAST_MAX_BIS_NUM);
                xmlFree(attr_value);
                return FALSE;
            }
            bis_num_found = TRUE;
            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    return codec_config_name_found && bis_num_found;
}

static BOOL btmw_rpc_test_bms_parse_bis_param(BT_BMS_ANNOUNCEMENT_BIS *bis, xmlNode *node)
{
    xmlAttrPtr attr = node->properties;

    bis->audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;

    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION,
            strlen(BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION)))
        {
            UINT32 tmp = 0;
            xmlChar *attr_value = xmlGetProp(node,
                BAD_CAST BTMW_RPC_TEST_BMS_ATTR_AUDIO_CHANNEL_ALLOCATION);

            if (0 == strlen((const char *)attr_value))
            {
                BTMW_RPC_TEST_Logw("[BMS] bis->audio_channel_allocation is LE_AUDIO_ALLOCATION_INVALID");
                bis->audio_channel_allocation = LE_AUDIO_ALLOCATION_INVALID;
            }
            else
            {
                if (!strncmp((const char *)attr_value, "0x", 2))
                {
                  if (!sscanf((const char *)attr_value+2, "%8x", &tmp))
                  {
                      BTMW_RPC_TEST_Loge("[BMS] sscanf error");
                  }
                }
                else
                {
                    tmp = atoi((const char *)attr_value);
                }
                bis->audio_channel_allocation = tmp;
            }

#if 0
            BTMW_RPC_TEST_Logi("[BMS] bis->audio_channel_allocation: 0x%08x",
                bis->audio_channel_allocation);
#endif

            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    return TRUE;
}


static int btmw_rpc_test_bms_metadata_acsii_2_hex(UINT8 *metadata_len, UINT8* metadata_hex, char* metadata_acsii)
{
    UINT8 i = 0;
    CHAR p_argv_meta[BT_BROADCAST_METADATA_SIZE*2] = {0};

    //medadata ACSII to Hex
    if (metadata_acsii)
    {
        *metadata_len = (UINT8)(strlen(metadata_acsii) + 1) / 2;
#if 0
        BTMW_RPC_TEST_Logi("[BMS] metadata_len %d\n", *metadata_len);
#endif
        strncpy(p_argv_meta, metadata_acsii, strlen(metadata_acsii));
        util_ascii_2_hex(p_argv_meta, *metadata_len, (char*)metadata_hex);

#if 0
        for (i = 0; i < *metadata_len; i++)
        {
            BTMW_RPC_TEST_Logi("[BMS] metadata[%d] 0x%02x\n", i, metadata_hex[i]);
        }
#endif
    }
    else
    {
        *metadata_len = 0;
    }
    return 0;
}

static int btmw_rpc_test_bms_broadcastcode_acsii_2_hex(UINT8 *code_len, UINT8* code_hex, char* code_acsii)
{
    UINT8 i = 0;
    CHAR p_argv_code[BT_BROADCAST_BROADCAST_CODE_SIZE*2] = {0};

    //medadata ACSII to Hex
    if (code_acsii)
    {
        *code_len = (UINT8)(strlen(code_acsii) + 1) / 2;
        BTMW_RPC_TEST_Logi("[BMS] code_len %d\n", *code_len);
        strncpy(p_argv_code, code_acsii, strlen(code_acsii));
        util_ascii_2_hex(p_argv_code, *code_len, (char*)code_hex);
#if 0
        for (i = 0; i < *code_len; i++)
        {
            BTMW_RPC_TEST_Logi("[BMS] code[%d] 0x%02x\n", i, code_hex[i]);
        }
#endif
    }
    else
    {
        *code_len = 0;
    }
    return 0;
}

