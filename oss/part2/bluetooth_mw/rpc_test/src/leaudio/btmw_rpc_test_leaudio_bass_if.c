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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_leaudio_bass_if.h"
#include "mtk_bt_service_leaudio_bass_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_ble_scanner_wrapper.h"
#include "util_dlist.h"
#include "util.h"

#define BTMW_RPC_BASS_CASE_RETURN_STR(const) case const: return #const;
#define BTMW_RPC_BASS_SHORTENED_LOCAL_NAME_TYPE (0x08)
#define BTMW_RPC_BASS_COMPLETE_LOCAL_NAME_TYPE  (0x09)
#define BTMW_RPC_BASS_EMPTY_ADDR                "00:00:00:00:00:00"

static VOID btmw_rpc_test_bass_show_counter_info(VOID);
static int btmw_rpc_test_bass_scan(int argc, char **argv);
static int btmw_rpc_test_bass_connect(int argc, char **argv);
static int btmw_rpc_test_bass_connect_by_name(int argc, char **argv);
static int btmw_rpc_test_bass_get_address_by_name(int argc, char **argv);
static int btmw_rpc_test_bass_disconnect(int argc, char **argv);
static int btmw_rpc_test_bass_set_builtin(int argc, char **argv);
static int btmw_rpc_test_bass_set_scan(int argc, char **argv);
static int btmw_rpc_test_bass_stop_observing(int argc, char **argv);
static int btmw_rpc_test_bass_get_receiver_state(int argc, char **argv);
static int btmw_rpc_test_bass_set_code(int argc, char **argv);
static int btmw_rpc_test_bass_set_source(int argc, char **argv);
static int btmw_rpc_test_bass_set_source_with_code(int argc, char **argv);
static int btmw_rpc_test_bass_modify_source(int argc, char **argv);
static int btmw_rpc_test_bass_remove_source(int argc, char **argv);
static int btmw_rpc_test_bass_dump(int argc, char **argv);
static int btmw_rpc_test_bass_reset_counter(int argc, char **argv);

typedef enum
{
    BT_BASS_COUNTER_TYPE_CONNECT_FAILED = 0,
    BT_BASS_COUNTER_TYPE_SYNC_PA_FAILED,
    BT_BASS_COUNTER_TYPE_SYNCING_MISSING,
} BT_BASS_COUNTER_TYPE;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    CHAR name[MAX_NAME_LEN];
} BT_BASS_SCAN_DEVICE_CTX;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BT_BASS_CONN_STATE state;
    BOOL is_w4bond;
    BOOL is_builtin;
    UINT32 builtin_sync_bis;
} BT_BASS_CONN_DEVICE_CTX;

typedef struct
{
    INT32 receiver_id;
    CHAR bt_addr[MAX_BDADDR_LEN];
    UINT8 local_name_len;
    CHAR local_name[MAX_NAME_LEN];
    BOOL encryption;
    BT_BASS_BROADCAST_ID broadcast_id;
    BT_BASS_ANNOUNCE_DATA announce_data;
} BT_BASS_ANNOUNCE_CTX;

typedef struct
{
    UINT32 connected_num;
    UINT32 disconnected_num;
    UINT32 enable_builtin_num;
    UINT32 syncing_num;
    UINT32 synced_to_pa_num;
    UINT32 sync_pa_failed_num;
    UINT32 recv_broadcast_num;
    UINT32 offset;
    _timestamp ts;
    BT_BASS_COUNTER_TYPE type;
} BT_BASS_COUNTER_CTX;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    BOOL enable;
    UINT8 code[BT_BROADCAST_BROADCAST_CODE_SIZE];
} BT_BASS_AURO_SET_CODE;

static BT_BASS_AURO_SET_CODE g_bass_auto_set_code;
static UINT8 g_bass_scanner_id = 0;
static VOID *g_bass_scan_device_list = NULL;
static VOID *g_bass_conn_device_list = NULL;
static VOID *g_bass_announce_list = NULL;
static VOID *g_bass_brs_list = NULL;
static VOID *g_bass_exception_counter_list = NULL;
static UINT8 g_bass_service_uuid[] = {0x4F, 0x18};
static UINT8 g_bass_service_uuid_mask[] = {0x4F, 0x18};
static BOOL g_bass_scan_started = FALSE;
static BT_BASS_COUNTER_CTX g_bass_counter;
static struct timeval g_bass_connect_start;
static struct timeval g_bass_connect_done;
static BTMW_RPC_TEST_CLI btmw_rpc_test_bass_cli_commands[] =
{
    {(const char *)"scan",                 btmw_rpc_test_bass_scan,                 (const char *)" = scan <enable>"},
    {(const char *)"connect",              btmw_rpc_test_bass_connect,              (const char *)" = connect <addr>"},
    {(const char *)"connect_by_name",      btmw_rpc_test_bass_connect_by_name,      (const char *)" = connect_by_name <name>"},
    {(const char *)"get_address_by_name",  btmw_rpc_test_bass_get_address_by_name,  (const char *)" = get_address_by_name <name>"},
    {(const char *)"disconnect",           btmw_rpc_test_bass_disconnect,           (const char *)" = disconnect <addr>"},
    {(const char *)"set_builtin",          btmw_rpc_test_bass_set_builtin,          (const char *)" = set_builtin <addr> <enable> [<sync_bis>]"},
    {(const char *)"set_scan",             btmw_rpc_test_bass_set_scan,             (const char *)" = set_scan <addr> <scan> [<duration_sec>]"},
    {(const char *)"stop_observing",       btmw_rpc_test_bass_stop_observing,       (const char *)" = stop_observing"},
    {(const char *)"get_receiver_state",   btmw_rpc_test_bass_get_receiver_state,   (const char *)" = get_receiver_state <addr> <receiver_id>"},
    {(const char *)"set_code",             btmw_rpc_test_bass_set_code,             (const char *)" = set_code <addr> <receiver_id> <code>, code sample 000102030405060708090A0B0C0D0E0F"},
    {(const char *)"set_source",           btmw_rpc_test_bass_set_source,           (const char *)" = set_source <addr> <adv_addr> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]"},
    {(const char *)"set_source_with_code", btmw_rpc_test_bass_set_source_with_code, (const char *)" = set_source_with_code <addr> <adv_addr> <code> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]"},
    {(const char *)"modify_source",        btmw_rpc_test_bass_modify_source,        (const char *)" = modify_source <addr> <receiver_id> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]"},
    {(const char *)"remove_source",        btmw_rpc_test_bass_remove_source,        (const char *)" = remove_source <addr> <receiver_id>"},
    {(const char *)"dump",                 btmw_rpc_test_bass_dump,                 (const char *)" = dump"},
    {(const char *)"reset_counter",        btmw_rpc_test_bass_reset_counter,        (const char *)" = reset_counter"},
    {NULL, NULL, NULL},
};

static int btmw_rpc_test_bass_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd = NULL;
    BTMW_RPC_TEST_CLI *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_RPC_TEST_Logi("[BASS] argc: %d, argv[0]: %s\n", argc, argv[0]);
    cmd = btmw_rpc_test_bass_cli_commands;
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logi("[BASS] Unknown command\n");
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BASS, btmw_rpc_test_bass_cli_commands);
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
        BTMW_RPC_TEST_Logi("[BASS] Unknown command '%s'\n", argv[0]);
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BASS, btmw_rpc_test_bass_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static BT_BASS_CONN_DEVICE_CTX* btmw_rpc_test_bass_get_conn_device(CHAR* bt_addr, BOOL add)
{
    BT_BASS_CONN_DEVICE_CTX *entry = (BT_BASS_CONN_DEVICE_CTX *)util_dlist_first(g_bass_conn_device_list);
    while (entry != NULL)
    {
        if (0 == strcasecmp(entry->bt_addr, bt_addr))
        {
            break;
        }
        entry = (BT_BASS_CONN_DEVICE_CTX *)util_dlist_next(g_bass_conn_device_list, entry);
    }
    if (entry)
    {
        return entry;
    }
    if (FALSE == add)
    {
        return NULL;
    }
    entry = (BT_BASS_CONN_DEVICE_CTX *)util_dlist_entry_alloc(sizeof(BT_BASS_CONN_DEVICE_CTX));
    if (NULL == entry)
    {
        BTMW_RPC_TEST_Loge("util_dlist_entry_alloc(%d) fail\n", sizeof(BT_BASS_CONN_DEVICE_CTX));
        return entry;
    }
    memcpy(entry->bt_addr, bt_addr, MAX_BDADDR_LEN);
    entry->state = BT_BASS_CONN_STATE_DISCONNECTED;
    entry->is_w4bond = FALSE;
    entry->is_builtin = FALSE;
    entry->builtin_sync_bis = 0xFFFFFFFF;
    util_dlist_insert(g_bass_conn_device_list, entry);
    return entry;
}

static CHAR* btmw_rpc_test_bass_get_state_str(BT_BASS_CONN_STATE state)
{
    switch (state)
    {
    case BT_BASS_CONN_STATE_DISCONNECTED:
        return "DISCONNECTED";

    case BT_BASS_CONN_STATE_CONNECTING:
        return "CONNECTING";

    case BT_BASS_CONN_STATE_CONNECTED:
        return "CONNECTED";

    case BT_BASS_CONN_STATE_DISCONNECTING:
        return "DISCONNECTING";

    default:
        return "UNKNOWN";
    }
}

static CHAR* btmw_rpc_test_bass_get_counter_type_str(BT_BASS_COUNTER_TYPE type)
{
    switch (type)
    {
    case BT_BASS_COUNTER_TYPE_CONNECT_FAILED:
        return "CONNECT_FAILED";

    case BT_BASS_COUNTER_TYPE_SYNC_PA_FAILED:
        return "SYNC_PA_FAILED";

    case BT_BASS_COUNTER_TYPE_SYNCING_MISSING:
        return "SYNCING_MISSING";

    default:
        return "UNKNOWN";
    }
}

static VOID btmw_rpc_test_bass_add_exception_counter(BT_BASS_COUNTER_CTX *counter)
{
    BT_BASS_COUNTER_CTX *entry = NULL;

    if ((NULL == counter) || (NULL == g_bass_exception_counter_list))
    {
        return;
    }
    entry = (BT_BASS_COUNTER_CTX *)util_dlist_entry_alloc(sizeof(BT_BASS_COUNTER_CTX));
    if (NULL == entry)
    {
        BTMW_RPC_TEST_Loge("util_dlist_entry_alloc(%d) fail\n", sizeof(BT_BASS_COUNTER_CTX));
        return;
    }
    memcpy(entry, counter, sizeof(BT_BASS_COUNTER_CTX));
    BTMW_RPC_TEST_GetTimestamp(&entry->ts);
    util_dlist_insert(g_bass_exception_counter_list, entry);
}

static VOID btmw_rpc_test_bass_show_meta_data(UINT8 len, UINT8* data)
{
    char data_str[2 * BT_BROADCAST_METADATA_SIZE + 1] = {0};
    char *ptr = data_str;

    if (len > BT_BROADCAST_METADATA_SIZE)
    {
        len = BT_BROADCAST_METADATA_SIZE;
    }
    for (int i = 0; i < len; i++)
    {
        ptr += sprintf(ptr, "%02x", data[i]);
    }
    *ptr = '\0';
    BTMW_RPC_TEST_Logd("meta_data_len=%d, meta_data=%s\n", len, data_str);
}

static VOID btmw_rpc_test_bass_show_code(UINT8* data)
{
    char data_str[2 * BT_BROADCAST_BROADCAST_CODE_SIZE + 1] = {0};
    char *ptr = data_str;

    for (int i = 0; i < BT_BROADCAST_BROADCAST_CODE_SIZE; i++)
    {
        ptr += sprintf(ptr, "%02x", data[i]);
    }
    *ptr = '\0';
    BTMW_RPC_TEST_Logd("code=%s\n", data_str);
}

static VOID btmw_rpc_test_bass_show_conn_state(BT_BASS_EVENT_CONN_STATE_DATA *conn_state)
{
    UINT32 time_use = 0;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    BTMW_RPC_TEST_Logd("bt_addr=%s\n", conn_state->bt_addr);
    switch (conn_state->state)
    {
    case BT_BASS_CONN_STATE_DISCONNECTED:
        g_bass_counter.disconnected_num++;
        BTMW_RPC_TEST_Logd("BASS DISCONNECTED\n");
        break;

    case BT_BASS_CONN_STATE_CONNECTING:
        BTMW_RPC_TEST_Logd("BASS CONNECTING\n");
        break;

    case BT_BASS_CONN_STATE_CONNECTED:
        gettimeofday(&g_bass_connect_done, NULL);
        time_use = (g_bass_connect_done.tv_sec - g_bass_connect_start.tv_sec) * 1000000 + (g_bass_connect_done.tv_usec - g_bass_connect_start.tv_usec);
        time_use /= 1000;
        g_bass_counter.connected_num++;
        BTMW_RPC_TEST_Logd("BASS CONNECTED, time(ms) %u\n", time_use);
        break;

    case BT_BASS_CONN_STATE_DISCONNECTING:
        BTMW_RPC_TEST_Logd("BASS DISCONNECTING\n");
        break;

    default:
        break;
    }

    device = btmw_rpc_test_bass_get_conn_device(conn_state->bt_addr, TRUE);
    if (device)
    {
        if (((BT_BASS_CONN_STATE_DISCONNECTED == device->state) || (BT_BASS_CONN_STATE_CONNECTING == device->state)) &&
            (BT_BASS_CONN_STATE_DISCONNECTED == conn_state->state))
        {
            g_bass_counter.type = BT_BASS_COUNTER_TYPE_CONNECT_FAILED;
            btmw_rpc_test_bass_add_exception_counter(&g_bass_counter);
            btmw_rpc_test_bass_show_counter_info();
        }
        device->state = conn_state->state;
        if (BT_BASS_CONN_STATE_DISCONNECTED == device->state)
        {
            util_dlist_delete(g_bass_conn_device_list, device);
        }
    }
}

static VOID btmw_rpc_test_bass_show_dev_avail(BT_BASS_EVENT_DEV_AVAIL_DATA *dev_avail)
{
    BTMW_RPC_TEST_Logd("bt_addr=%s, num_receivers=%d\n", dev_avail->bt_addr, dev_avail->num_receivers);
}

static VOID btmw_rpc_test_bass_show_scanning_state(BT_BASS_EVENT_SCANNING_STATE_DATA *scanning_state)
{
    BTMW_RPC_TEST_Logd("bt_addr=%s, scan=%d\n", scanning_state->bt_addr, scanning_state->scan);
}

static VOID btmw_rpc_test_bass_show_lc3(BT_BASS_LC3_CODEC_CONFIG *lc3)
{
    BTMW_RPC_TEST_Logd("    ==============LC3 Start==============\n");
    BTMW_RPC_TEST_Logd("    sampling_frequency=0x%02x\n", lc3->sampling_frequency);
    BTMW_RPC_TEST_Logd("    frame_duration=0x%02x\n", lc3->frame_duration);
    BTMW_RPC_TEST_Logd("    octets_per_codec_frame=0x%04x\n", lc3->octets_per_codec_frame);
    BTMW_RPC_TEST_Logd("    codec_frame_blocks_per_sdu=0x%02x\n", lc3->codec_frame_blocks_per_sdu);
    BTMW_RPC_TEST_Logd("    audio_channel_allocation=0x%08x\n", lc3->audio_channel_allocation);
    BTMW_RPC_TEST_Logd("    ==============LC3 End================\n");
}

static VOID btmw_rpc_test_bass_show_codec(BT_BASS_ANNOUNCE_CODEC_CONFIG *codec)
{
    BTMW_RPC_TEST_Logd("================CODEC_CONFIG Start================\n");
    BTMW_RPC_TEST_Logd("codec_id=0x%02x\n", codec->codec_id);
    BTMW_RPC_TEST_Logd("vendor_company_id=0x%04x\n", codec->vendor_company_id);
    BTMW_RPC_TEST_Logd("vendor_codec_id=0x%04x\n", codec->vendor_codec_id);
    btmw_rpc_test_bass_show_lc3(&codec->codec_param);
    BTMW_RPC_TEST_Logd("================CODEC_CONFIG End==================\n");
}

static VOID btmw_rpc_test_bass_show_bis(int idx, BT_BASS_ANNOUNCE_BIS_CONFIG *bis)
{
    BTMW_RPC_TEST_Logd("==============BIS_CONFIG[%d] Start==============\n", idx);
    BTMW_RPC_TEST_Logd("bis_index=0x%02x\n", bis->bis_index);
    btmw_rpc_test_bass_show_lc3(&bis->codec_param);
    BTMW_RPC_TEST_Logd("==============BIS_CONFIG[%d] End================\n", idx);
}

static VOID btmw_rpc_test_bass_show_subgroup(BOOL show_all, int idx, BT_BASS_ANNOUNCE_SUBGROUP *subgroup)
{
    BTMW_RPC_TEST_Logd("======================SUBGROUP[%d] Start======================\n", idx);
    btmw_rpc_test_bass_show_meta_data(subgroup->meta_data_len, subgroup->meta_data);
    if (TRUE == show_all)
    {
        btmw_rpc_test_bass_show_codec(&subgroup->codec_config);
        BTMW_RPC_TEST_Logd("bis_size=0x%02x\n", subgroup->bis_size);
        for (int i = 0; i < subgroup->bis_size; i++)
        {
            btmw_rpc_test_bass_show_bis(i, &subgroup->bis_configs[i]);
        }
    }
    BTMW_RPC_TEST_Logd("======================SUBGROUP[%d] End========================\n", idx);
}

static VOID btmw_rpc_test_bass_show_scan_device_list_info(VOID)
{
    BT_BASS_SCAN_DEVICE_CTX *entry = NULL;
    int idx = 0;
    UINT32 count = 0;

    if (NULL == g_bass_scan_device_list)
    {
        return;
    }
    count = util_dlist_count(g_bass_scan_device_list);
    if (0 == count)
    {
        return;
    }
    BTMW_RPC_TEST_Logd("======================SCAN_DEVICE_LIST(%d) Start======================\n", count);
    entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_first(g_bass_scan_device_list);
    while (entry != NULL)
    {
        BTMW_RPC_TEST_Logd("SCAN_DEVICE[%d] addr=%s, name=%s\n", idx, entry->bt_addr, entry->name);
        entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_next(g_bass_scan_device_list, entry);
        idx++;
    }
    BTMW_RPC_TEST_Logd("======================SCAN_DEVICE_LIST(%d) End========================\n", count);
}

static VOID btmw_rpc_test_bass_show_conn_device_list_info(VOID)
{
    BT_BASS_CONN_DEVICE_CTX *entry = NULL;
    int idx = 0;
    UINT32 count = 0;

    if (NULL == g_bass_conn_device_list)
    {
        return;
    }
    count = util_dlist_count(g_bass_conn_device_list);
    if (0 == count)
    {
        return;
    }
    BTMW_RPC_TEST_Logd("=======================================CONN_DEVICE_LIST(%d) Start=======================================\n", count);
    entry = (BT_BASS_CONN_DEVICE_CTX *)util_dlist_first(g_bass_conn_device_list);
    while (entry != NULL)
    {
        BTMW_RPC_TEST_Logd("CONN_DEVICE[%d] addr=%s, state=%s, is_w4bond=%s, is_builtin=%s, builtin_sync_bis=0x%08X\n",
            idx, entry->bt_addr, btmw_rpc_test_bass_get_state_str(entry->state),
            (TRUE == entry->is_w4bond) ? "YES" : "NO", (TRUE == entry->is_builtin) ? "YES" : "NO", entry->builtin_sync_bis);
        entry = (BT_BASS_CONN_DEVICE_CTX *)util_dlist_next(g_bass_conn_device_list, entry);
        idx++;
    }
    BTMW_RPC_TEST_Logd("=======================================CONN_DEVICE_LIST(%d) End=========================================\n", count);
}

static VOID btmw_rpc_test_bass_show_announce_list_info(VOID)
{
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    int idx = 0;
    UINT32 count = 0;

    if (NULL == g_bass_announce_list)
    {
        return;
    }
    count = util_dlist_count(g_bass_announce_list);
    if (0 == count)
    {
        BTMW_RPC_TEST_Logd("g_bass_announce_list is empty\n");
        return;
    }
    BTMW_RPC_TEST_Logd("======================ANNOUNCE_LIST(%d) Start======================\n", count);
    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
    while (entry != NULL)
    {
        BTMW_RPC_TEST_Logd("ANNOUNCE[%d] adv_addr=%s, addr_type=%d, adv_sid=%d, name=%s, encryption=%d\n",
                            idx, entry->broadcast_id.bt_addr, entry->broadcast_id.addr_type,
                            entry->broadcast_id.adv_sid, entry->local_name, entry->encryption);
        BTMW_RPC_TEST_Logd("ANNOUNCE[%d] bsd addr=%s, receiver_id=%d\n", idx, entry->bt_addr, entry->receiver_id);
        BTMW_RPC_TEST_Logd("ANNOUNCE[%d] presentation_delay=%d, subgroup_size=%d\n",
            idx, entry->announce_data.presentation_delay, entry->announce_data.subgroup_size);
        for (int i =0; i < entry->announce_data.subgroup_size; i++)
        {
            btmw_rpc_test_bass_show_subgroup(TRUE, i, &entry->announce_data.subgroup_configs[i]);
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
        idx++;
    }
    BTMW_RPC_TEST_Logd("======================ANNOUNCE_LIST(%d) End========================\n", count);
}

static VOID btmw_rpc_test_bass_show_brs_list_info(VOID)
{
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    int idx = 0;
    UINT32 count = 0;

    if (NULL == g_bass_brs_list)
    {
        return;
    }
    count = util_dlist_count(g_bass_brs_list);
    if (0 == count)
    {
        BTMW_RPC_TEST_Logd("g_bass_brs_list is empty\n");
        return;
    }
    BTMW_RPC_TEST_Logd("======================BRS_LIST(%d) Start======================\n", count);
    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_brs_list);
    while (entry != NULL)
    {
        BTMW_RPC_TEST_Logd("BRS[%d] adv_addr=%s, addr_type=%d, adv_sid=%d, name=%s, encryption=%d\n",
                            idx, entry->broadcast_id.bt_addr, entry->broadcast_id.addr_type,
                            entry->broadcast_id.adv_sid, entry->local_name, entry->encryption);
        BTMW_RPC_TEST_Logd("BRS[%d] bsd addr=%s, receiver_id=%d\n", idx, entry->bt_addr, entry->receiver_id);
        BTMW_RPC_TEST_Logd("BRS[%d] presentation_delay=%d, subgroup_size=%d\n",
            idx, entry->announce_data.presentation_delay, entry->announce_data.subgroup_size);
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_brs_list, entry);
        idx++;
    }
    BTMW_RPC_TEST_Logd("======================BRS_LIST(%d) End========================\n", count);
}

static VOID btmw_rpc_test_bass_show_counter_info(VOID)
{
    BT_BASS_COUNTER_CTX *entry = NULL;
    UINT32 count = 0;
    UINT32 connect_failed_num = 0;
    UINT32 sync_pa_failed_num = 0;
    UINT32 syncing_missing_num = 0;
    char buffer[] = "00:00:00.000";
    int idx = 0;

    BTMW_RPC_TEST_Logd("[COUNTER] connected=%u, disconnected=%u, enable_builtin=%u, syncing=%u, synced_to_pa=%u, sync_pa_failed=%u, recv_broadcast=%u\n",
        g_bass_counter.connected_num, g_bass_counter.disconnected_num, g_bass_counter.enable_builtin_num,
        g_bass_counter.syncing_num, g_bass_counter.synced_to_pa_num, g_bass_counter.sync_pa_failed_num,
        g_bass_counter.recv_broadcast_num);

    if (NULL == g_bass_exception_counter_list)
    {
        return;
    }
    count = util_dlist_count(g_bass_exception_counter_list);
    if (0 == count)
    {
        return;
    }
    idx = count - 1;
    BTMW_RPC_TEST_Logd("=================================EXCEPTION_LIST(%d) Start=================================\n", count);
    entry = (BT_BASS_COUNTER_CTX *)util_dlist_first(g_bass_exception_counter_list);
    while (entry != NULL)
    {
        switch (entry->type)
        {
        case BT_BASS_COUNTER_TYPE_CONNECT_FAILED:
            connect_failed_num++;
            break;
        case BT_BASS_COUNTER_TYPE_SYNC_PA_FAILED:
            sync_pa_failed_num++;
            break;
        case BT_BASS_COUNTER_TYPE_SYNCING_MISSING:
            syncing_missing_num++;
            break;
        default:
            break;
        }
        (void)snprintf(&buffer[0], sizeof(buffer), "%02d:%02d:%02d.%03d", entry->ts.hour, entry->ts.min, entry->ts.sec, entry->ts.msec);
        BTMW_RPC_TEST_Logd("EXCEPTION[%d] type=%s, ts=%s, offset=%u, connected=%u, disconnected=%u\n",
            idx, btmw_rpc_test_bass_get_counter_type_str(entry->type), buffer,
            entry->offset, entry->connected_num, entry->disconnected_num);
        BTMW_RPC_TEST_Logd("EXCEPTION[%d] enable_builtin=%u, syncing=%u, synced_to_pa=%u, sync_pa_failed=%u\n",
            idx, entry->enable_builtin_num, entry->syncing_num, entry->synced_to_pa_num, entry->sync_pa_failed_num);
        entry = (BT_BASS_COUNTER_CTX *)util_dlist_next(g_bass_exception_counter_list, entry);
        if (idx >= 1)
        {
            idx--;
        }
        else
        {
            break;
        }
    }
    BTMW_RPC_TEST_Logd("TOTAL connect_failed=%u, sync_pa_failed=%u, syncing_missing=%u\n",
                connect_failed_num, sync_pa_failed_num, syncing_missing_num);
    BTMW_RPC_TEST_Logd("=================================EXCEPTION_LIST(%d) End===================================\n", count);
}

static void btmw_rpc_test_bass_get_addr_by_name(char* name, char* addr)
{
    BT_BASS_SCAN_DEVICE_CTX* entry = NULL;

    if (NULL == g_bass_scan_device_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_conn_device_list is NULL\n");
        return -1;
    }

    entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_first(g_bass_scan_device_list);
    while (entry != NULL)
    {
        if (!strcasecmp(name, entry->name))
        {
            break;
        }
        entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_next(g_bass_scan_device_list, entry);
    }

    if (entry)
    {
        strncpy(addr, entry->bt_addr, MAX_BDADDR_LEN);
        addr[MAX_BDADDR_LEN - 1] = '\0';
    }
}

static VOID btmw_rpc_test_bass_show_announce_received(int receiver_id, BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA *announce_received)
{
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    BOOL show_all = (-1 == receiver_id) ? TRUE : FALSE;

    BTMW_RPC_TEST_Logd("bt_addr=%s\n", announce_received->bt_addr);
    BTMW_RPC_TEST_Logd("[broadcast_id] bt_addr=%s, addr_type=%d, adv_sid=%d\n",
        announce_received->broadcast_id.bt_addr, announce_received->broadcast_id.addr_type,
        announce_received->broadcast_id.adv_sid);
    if (TRUE == show_all)
    {
        BTMW_RPC_TEST_Logd("[announce_data] presentation_delay=%d, subgroup_size=%d\n",
            announce_received->announce_data.presentation_delay, announce_received->announce_data.subgroup_size);
    }
    for (int i =0; i < announce_received->announce_data.subgroup_size; i++)
    {
        btmw_rpc_test_bass_show_subgroup(show_all, i, &announce_received->announce_data.subgroup_configs[i]);
    }
    if (NULL == g_bass_announce_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return;
    }

    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
    while (entry != NULL)
    {
        if (0 == strcasecmp(entry->broadcast_id.bt_addr, announce_received->broadcast_id.bt_addr))
        {
            break;
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
    }

    if (NULL == entry)
    {
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_entry_alloc(sizeof(BT_BASS_ANNOUNCE_CTX));
        if (NULL == entry)
        {
            BTMW_RPC_TEST_Loge("util_dlist_entry_alloc(%d) fail\n", sizeof(BT_BASS_ANNOUNCE_CTX));
            return;
        }
        if (receiver_id == -1) {
          entry->receiver_id = -1;
          memcpy(entry->bt_addr, announce_received->bt_addr, MAX_BDADDR_LEN);
          memcpy(entry->local_name, announce_received->local_name, MAX_NAME_LEN);
          entry->encryption = announce_received->encryption;
          memcpy(&entry->broadcast_id, &announce_received->broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
          memcpy(&entry->announce_data, &announce_received->announce_data, sizeof(BT_BASS_ANNOUNCE_DATA));
          util_dlist_insert(g_bass_announce_list, entry);
        }
    }
    else if (TRUE == show_all)
    {
        memcpy(entry->bt_addr, announce_received->bt_addr, MAX_BDADDR_LEN);
        memcpy(entry->local_name, announce_received->local_name, MAX_NAME_LEN);
        entry->encryption = announce_received->encryption;
        memcpy(&entry->broadcast_id, &announce_received->broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
        memcpy(&entry->announce_data, &announce_received->announce_data, sizeof(BT_BASS_ANNOUNCE_DATA));
    }
    else
    {
        entry->receiver_id = receiver_id;
        memcpy(entry->bt_addr, announce_received->bt_addr, MAX_BDADDR_LEN);
        memcpy(entry->local_name, announce_received->local_name, MAX_NAME_LEN);
        entry->encryption = announce_received->encryption;
        memcpy(&entry->broadcast_id, &announce_received->broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
        entry->announce_data.subgroup_size = announce_received->announce_data.subgroup_size;
        for (int i = 0; i < entry->announce_data.subgroup_size; i++)
        {
            entry->announce_data.subgroup_configs[i].meta_data_len =
                announce_received->announce_data.subgroup_configs[i].meta_data_len;
            if (entry->announce_data.subgroup_configs[i].meta_data_len > BT_BROADCAST_METADATA_SIZE)
            {
                entry->announce_data.subgroup_configs[i].meta_data_len = BT_BROADCAST_METADATA_SIZE;
            }
            memcpy(entry->announce_data.subgroup_configs[i].meta_data,
                announce_received->announce_data.subgroup_configs[i].meta_data,
                entry->announce_data.subgroup_configs[i].meta_data_len);
        }
    }
    if (TRUE == show_all)
    {
        btmw_rpc_test_bass_show_announce_list_info();
        btmw_rpc_test_bass_show_brs_list_info();
    }
}

static VOID btmw_rpc_test_bass_show_receiver_state(BT_BASS_EVENT_RECV_STATE_DATA *recv_state)
{
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA data;

    BTMW_RPC_TEST_Logd("bt_addr=%s, receiver_id=%d\n", recv_state->bt_addr, recv_state->receiver_id);
    BTMW_RPC_TEST_Logd("[broadcast_id] bt_addr=%s, addr_type=%d, adv_sid=%d\n",
        recv_state->broadcast_id.bt_addr, recv_state->broadcast_id.addr_type,
        recv_state->broadcast_id.adv_sid);
    BTMW_RPC_TEST_Logd("num_subgroups=%d\n", recv_state->data.num_subgroups);
    for (int i = 0; i < recv_state->data.num_subgroups; i++)
    {
        BTMW_RPC_TEST_Logd("subgroup[%d] sync_bis=0x%08X, ", i, recv_state->data.subgroup[i].sync_bis);
        btmw_rpc_test_bass_show_meta_data(recv_state->data.subgroup[i].meta_data_len, recv_state->data.subgroup[i].meta_data);
    }
    switch (recv_state->state)
    {
    case BT_BASS_RECV_STATE_IDLE:
        BTMW_RPC_TEST_Logd("IDLE\n");
        break;

    case BT_BASS_RECV_STATE_SET_SOURCE_FAILED:
        BTMW_RPC_TEST_Logd("SET SOURCE FAILED\n");
        break;

    case BT_BASS_RECV_STATE_SYNCING:
        g_bass_counter.syncing_num++;
        BTMW_RPC_TEST_Logd("SYNCING\n");
        break;

    case BT_BASS_RECV_STATE_SYNC_PA_FAILED:
        g_bass_counter.sync_pa_failed_num++;
        g_bass_counter.type = BT_BASS_COUNTER_TYPE_SYNC_PA_FAILED;
        btmw_rpc_test_bass_add_exception_counter(&g_bass_counter);
        BTMW_RPC_TEST_Logd("SYNC PA FAILED\n");
        break;

    case BT_BASS_RECV_STATE_SYNC_BIS_FAILED:
        BTMW_RPC_TEST_Logd("SYNC BIS FAILED\n");
        break;

    case BT_BASS_RECV_STATE_SYNC_BIS_STOPPED:
        BTMW_RPC_TEST_Logd("SYNC BIS STOPPED\n");
        break;

    case BT_BASS_RECV_STATE_SYNCED_TO_PA:
        g_bass_counter.synced_to_pa_num++;
        BTMW_RPC_TEST_Logd("SYNCED TO PA\n");
        break;

    case BT_BASS_RECV_STATE_BROADCAST_CODE_REQUIRED:
        BTMW_RPC_TEST_Logd("BROADCAST CODE REQUIRED\n");
        if (g_bass_auto_set_code.enable)
        {
            BT_BASS_BROADCAST_CODE_PARAM param;
            memset(&param, 0, sizeof(param));
            strncpy(param.bt_addr, recv_state->bt_addr, MAX_BDADDR_LEN);
            param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
            param.receiver_id = recv_state->receiver_id;
            memcpy(param.code, g_bass_auto_set_code.code, BT_BROADCAST_BROADCAST_CODE_SIZE);
            a_mtkapi_bt_bass_set_broadcast_code(&param);
            memset(&g_bass_auto_set_code, 0, sizeof(g_bass_auto_set_code));
        }
        break;

    case BT_BASS_RECV_STATE_BAD_BROADCAST_CODE:
        BTMW_RPC_TEST_Logd("BAD BROADCAST CODE\n");
        break;

    case BT_BASS_RECV_STATE_RECEIVING_BROADCAST:
        g_bass_counter.recv_broadcast_num++;
        BTMW_RPC_TEST_Logd("RECEIVING BROADCAST\n");
        break;

    default:
        break;
    }

    if (NULL == g_bass_announce_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return;
    }

    //remove source
    if (0 == strcasecmp(recv_state->broadcast_id.bt_addr, BTMW_RPC_BASS_EMPTY_ADDR)
        && 0 == recv_state->broadcast_id.addr_type
        && 0 == recv_state->broadcast_id.adv_sid)
    {
        UINT32 count = 0;

        if (g_bass_brs_list != NULL){
          count = util_dlist_count(g_bass_brs_list);
          if (count != 0) {
            entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_brs_list);
            while (entry != NULL)
            {
                if ((0 == strcasecmp(entry->bt_addr, recv_state->bt_addr)) &&
                    entry->receiver_id == recv_state->receiver_id)
                {
                    break;
                }
                entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_brs_list, entry);
            }

            if (entry)
            {
                util_dlist_delete(g_bass_brs_list, entry);
            }
          }
        }

        if (g_bass_announce_list != NULL)
        {
          count = util_dlist_count(g_bass_announce_list);
          if (count != 0)
          {
            entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
            while (entry != NULL)
            {
                if (0 == strcasecmp(recv_state->bt_addr, entry->bt_addr)){
                  entry->receiver_id = -1;
                }
                entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
            }
          }
        }
        return;
    }
    memset(&data, 0, sizeof(BT_BASS_EVENT_ANNOUNCE_RECEIVED_DATA));
    memcpy(data.bt_addr, recv_state->bt_addr, sizeof(data.bt_addr));
    memcpy(&data.broadcast_id, &recv_state->broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
    data.announce_data.subgroup_size = recv_state->data.num_subgroups;
    for (int i = 0; i < recv_state->data.num_subgroups; i++)
    {
        data.announce_data.subgroup_configs[i].meta_data_len = recv_state->data.subgroup[i].meta_data_len;
        if (data.announce_data.subgroup_configs[i].meta_data_len > BT_BROADCAST_METADATA_SIZE)
        {
            data.announce_data.subgroup_configs[i].meta_data_len = BT_BROADCAST_METADATA_SIZE;
        }
        memcpy(data.announce_data.subgroup_configs[i].meta_data, recv_state->data.subgroup[i].meta_data,
            data.announce_data.subgroup_configs[i].meta_data_len);
    }

    if (g_bass_brs_list != NULL){
      entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_brs_list);
      while (entry != NULL)
      {
          if ((0 == strcasecmp(entry->bt_addr, recv_state->bt_addr)) &&
              entry->receiver_id == recv_state->receiver_id)
          {
              break;
          }
          entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_brs_list, entry);
      }

      if (entry == NULL)
      {
          entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_entry_alloc(sizeof(BT_BASS_ANNOUNCE_CTX));
          if (NULL == entry)
          {
              BTMW_RPC_TEST_Loge("util_dlist_entry_alloc(%d) fail\n", sizeof(BT_BASS_ANNOUNCE_CTX));
              return;
          }
          entry->receiver_id = recv_state->receiver_id;
          memcpy(entry->bt_addr, data.bt_addr, MAX_BDADDR_LEN);
          memcpy(entry->local_name, data.local_name, MAX_NAME_LEN);
          entry->encryption = data.encryption;
          memcpy(&entry->broadcast_id, &data.broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
          memcpy(&entry->announce_data, &data.announce_data, sizeof(BT_BASS_ANNOUNCE_DATA));
          util_dlist_insert(g_bass_brs_list, entry);
      }
      else
      {
        entry->receiver_id = recv_state->receiver_id;
        memcpy(entry->bt_addr, data.bt_addr, MAX_BDADDR_LEN);
        memcpy(entry->local_name, data.local_name, MAX_NAME_LEN);
        entry->encryption = data.encryption;
        memcpy(&entry->broadcast_id, &data.broadcast_id, sizeof(BT_BASS_BROADCAST_ID));
        memcpy(&entry->announce_data, &data.announce_data, sizeof(BT_BASS_ANNOUNCE_DATA));
      }
    }

    btmw_rpc_test_bass_show_announce_received(recv_state->receiver_id, &data);
}

static VOID btmw_rpc_test_bass_show_builtin_mode(BT_BASS_EVENT_BUILTIN_MODE_DATA *builtin_mode)
{
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    BTMW_RPC_TEST_Logd("bt_addr=%s, enable=%d, sync_bis=0x%08X\n", builtin_mode->bt_addr, builtin_mode->enable, builtin_mode->sync_bis);
    if (TRUE == builtin_mode->enable)
    {
        g_bass_counter.enable_builtin_num++;
        if ((g_bass_counter.syncing_num + g_bass_counter.offset + 1) < g_bass_counter.enable_builtin_num)
        {
            g_bass_counter.offset = g_bass_counter.enable_builtin_num - g_bass_counter.syncing_num - 1;
            g_bass_counter.type = BT_BASS_COUNTER_TYPE_SYNCING_MISSING;
            btmw_rpc_test_bass_add_exception_counter(&g_bass_counter);
            btmw_rpc_test_bass_show_counter_info();
        }
    }
    device = btmw_rpc_test_bass_get_conn_device(builtin_mode->bt_addr, TRUE);
    if (device)
    {
        device->is_builtin = builtin_mode->enable;
        device->builtin_sync_bis = builtin_mode->sync_bis;
    }
}

static VOID btmw_rpc_test_bass_show_sync_lost(BT_BASS_EVENT_SYNC_LOST_DATA *sync_lost)
{
    BT_BASS_ANNOUNCE_CTX *entry = NULL;

    BTMW_RPC_TEST_Logd("bt_addr=%s, addr_type=%d, adv_sid=%d\n",
                        sync_lost->broadcast_id.bt_addr, sync_lost->broadcast_id.addr_type,
                        sync_lost->broadcast_id.adv_sid);
    if (NULL == g_bass_announce_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return;
    }

    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
    while (entry != NULL)
    {
        if ((0 == strcasecmp(entry->broadcast_id.bt_addr, sync_lost->broadcast_id.bt_addr)) &&
            entry->broadcast_id.addr_type == sync_lost->broadcast_id.addr_type &&
            entry->broadcast_id.adv_sid == sync_lost->broadcast_id.adv_sid)
        {
            break;
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
    }

    if (entry)
    {
        BTMW_RPC_TEST_Logd("remove %s from g_bass_announce_list\n", sync_lost->broadcast_id.bt_addr);
        util_dlist_delete(g_bass_announce_list, entry);
    }

    btmw_rpc_test_bass_show_announce_list_info();
}

static CHAR* btmw_rpc_test_bass_app_event(BT_BASS_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_CONN_STATE)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_DEV_AVAIL)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_SCANNING_STATE)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_ANNOUNCE_RECEIVED)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_RECEIVER_STATE)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_BUILTIN_MODE)
        BTMW_RPC_BASS_CASE_RETURN_STR(BT_BASS_EVENT_SYNC_LOST)
        default: return "UNKNOWN_BASS_EVENT";
   }
}

static VOID btmw_rpc_test_bass_app_cbk(BT_BASS_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("event=%d %s\n", param->event,
                        btmw_rpc_test_bass_app_event(param->event));
    switch (param->event)
    {
    case BT_BASS_EVENT_CONN_STATE:
        btmw_rpc_test_bass_show_conn_state(&param->data.conn_state);
        break;

    case BT_BASS_EVENT_DEV_AVAIL:
        btmw_rpc_test_bass_show_dev_avail(&param->data.dev_avail);
        break;

    case BT_BASS_EVENT_SCANNING_STATE:
        btmw_rpc_test_bass_show_scanning_state(&param->data.scanning_state);
        break;

    case BT_BASS_EVENT_ANNOUNCE_RECEIVED:
        btmw_rpc_test_bass_show_announce_received(-1, &param->data.announce_received);
        break;

    case BT_BASS_EVENT_RECEIVER_STATE:
        btmw_rpc_test_bass_show_receiver_state(&param->data.recv_state);
        break;

    case BT_BASS_EVENT_BUILTIN_MODE:
        btmw_rpc_test_bass_show_builtin_mode(&param->data.builtin_mode);
        break;

    case BT_BASS_EVENT_SYNC_LOST:
        btmw_rpc_test_bass_show_sync_lost(&param->data.sync_lost);
        break;

    default:
        break;
    }
}

/**
 * This function returns a pointer inside the |ad| array of length |ad_len|
 * where a field of |type| is located, together with its length in |p_length|
 */
static UINT8* btmw_rpc_test_bass_get_field_by_type(UINT8* ad, size_t ad_len, UINT8 type, UINT8* p_length)
{
    size_t position = 0;

    while (position != ad_len)
    {
        UINT8 len = ad[position];
        if (len == 0)
        {
            break;
        }
        if (position + len >= ad_len)
        {
            break;
        }
        UINT8 adv_type = ad[position + 1];
        if (adv_type == type)
        {
            /* length doesn't include itself */
            *p_length = len - 1; /* minus the length of type */
            return ad + position + 2;
        }
        position += len + 1; /* skip the length of data */
    }

    *p_length = 0;
    return NULL;
}

static VOID btmw_rpc_test_bass_ble_scanner_cbk(BT_BLE_SCANNER_CALLBACK_PARAM *callback_param, VOID *pv_tag)
{
    if (NULL == callback_param)
    {
        return;
    }

    switch (callback_param->event)
    {
        case BT_BLE_SCANNER_EVT_REGISTER:
        {
            BTMW_RPC_TEST_Logd("BT_BLE_SCANNER_EVT_REGISTER status %d, scanner_id %d\n",
                callback_param->data.register_data.status, callback_param->scanner_id);
            if (0 == callback_param->data.register_data.status)
            {
                g_bass_scanner_id = callback_param->scanner_id;
            }
            break;
        }

        case BT_BLE_SCANNER_EVT_SCAN_RESULT:
        {
            if (NULL == g_bass_scan_device_list)
            {
                BTMW_RPC_TEST_Loge("g_bass_scan_device_list is NULL\n");
                break;
            }

            BT_BASS_SCAN_DEVICE_CTX *entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_first(g_bass_scan_device_list);
            while (entry != NULL)
            {
                if (0 == strcasecmp(entry->bt_addr, callback_param->data.scan_result_data.bd_addr))
                {
                    break;
                }
                entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_next(g_bass_scan_device_list, entry);
            }
            if (entry)
            {
                break;
            }
            UINT8 len = 0;
            UINT8 *p_name = btmw_rpc_test_bass_get_field_by_type(callback_param->data.scan_result_data.adv_data,
                callback_param->data.scan_result_data.adv_data_len,
                BTMW_RPC_BASS_COMPLETE_LOCAL_NAME_TYPE, &len);
            if (0 == len)
            {
                p_name = btmw_rpc_test_bass_get_field_by_type(callback_param->data.scan_result_data.adv_data,
                    callback_param->data.scan_result_data.adv_data_len,
                    BTMW_RPC_BASS_SHORTENED_LOCAL_NAME_TYPE, &len);
            }
            if (0 == len)
            {
                break;
            }
            entry = (BT_BASS_SCAN_DEVICE_CTX *)util_dlist_entry_alloc(sizeof(BT_BASS_SCAN_DEVICE_CTX));
            if (NULL == entry)
            {
                BTMW_RPC_TEST_Loge("util_dlist_entry_alloc(%d) fail\n", sizeof(BT_BASS_SCAN_DEVICE_CTX));
                return;
            }
            memcpy(entry->bt_addr, callback_param->data.scan_result_data.bd_addr, MAX_BDADDR_LEN);
            if (len >= MAX_NAME_LEN)
            {
                len = MAX_NAME_LEN - 1;
            }
            memcpy(entry->name, p_name, len);
            entry->name[len] = '\0';
            util_dlist_insert(g_bass_scan_device_list, entry);
            btmw_rpc_test_bass_show_scan_device_list_info();
            break;
        }

        default:
            break;
    }
}

static int btmw_rpc_test_bass_scan(int argc, char **argv)
{
    int enable = 0;
    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM start_param;
    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_param;
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_WITH_FILT_PARAM *p_filter = NULL;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS scan <enable>\n");
        return -1;
    }

    enable = atoi(argv[0]);
    BTMW_RPC_TEST_Logi("[BASS] %s() enable %d\n", __func__, enable);
    g_bass_scan_started = (BOOL)enable;
    if (enable)
    {
        if (g_bass_scan_device_list)
        {
            util_dlist_empty(g_bass_scan_device_list, TRUE);
        }
        memset(&start_param, 0, sizeof(start_param));
        p_filter = &start_param.scan_param.regular_scan_filt_param;
        start_param.scanner_id = g_bass_scanner_id;
        start_param.scan_type = REGULAR_SCAN_WITH_FILT;
        p_filter->regular_scan_param.scan_mode = SCAN_MODE_LOW_LATENCY;
        p_filter->scan_filt_param.dely_mode = FILT_DELY_MODE_IMMEDIATE;
        p_filter->scan_filt_param.filt_match_mode = MATCH_MODE_AGGRESSIVE;
        p_filter->scan_filt_param.num_of_matches = MATCH_NUM_MAX_ADVERTISEMENT;
        p_filter->scan_filt_num = 1;
        p_filter->scan_filt_data[0].srvc_data_len = sizeof(g_bass_service_uuid);
        memcpy(p_filter->scan_filt_data[0].srvc_data, g_bass_service_uuid, sizeof(g_bass_service_uuid));
        memcpy(p_filter->scan_filt_data[0].srvc_data_mask, g_bass_service_uuid_mask, sizeof(g_bass_service_uuid_mask));
        return a_mtkapi_bt_ble_scanner_start_simple_scan(&start_param);
    }
    else
    {
        stop_param.scanner_id = g_bass_scanner_id;
        return a_mtkapi_bt_ble_scanner_stop_scan(&stop_param);
    }
}

static int btmw_rpc_test_bass_connect(int argc, char **argv)
{
    BT_BASS_CONN_PARAM param;
    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_param;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS connect <addr>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s\n", __func__, param.bt_addr);
    if (TRUE == g_bass_scan_started)
    {
        g_bass_scan_started = FALSE;
        stop_param.scanner_id = g_bass_scanner_id;
        a_mtkapi_bt_ble_scanner_stop_scan(&stop_param);
    }
    device = btmw_rpc_test_bass_get_conn_device(param.bt_addr, FALSE);
    if (device && (BT_BASS_CONN_STATE_CONNECTED == device->state))
    {
        BTMW_RPC_TEST_Logd("[BASS] already CONNECTED %s, is_builtin=%s, builtin_sync_bis=0x%08X\n",
            device->bt_addr, (TRUE == device->is_builtin) ? "YES" : "NO", device->builtin_sync_bis);
        return 0;
    }
    if (BT_SUCCESS == a_mtkapi_bt_gap_get_bond_state(param.bt_addr))
    {
        gettimeofday(&g_bass_connect_start, NULL);
        return a_mtkapi_bt_bass_connect(&param);
    }
    else
    {
        device = btmw_rpc_test_bass_get_conn_device(param.bt_addr, TRUE);
        if (NULL == device)
        {
            BTMW_RPC_TEST_Logi("[BASS] %s() no memory for addr %s\n", __func__, param.bt_addr);
            return -1;
        }
        device->is_w4bond = TRUE;
        gettimeofday(&g_bass_connect_start, NULL);
        return a_mtkapi_bt_gap_pair(param.bt_addr, 2);
    }
}

static int btmw_rpc_test_bass_connect_by_name(int argc, char **argv)
{
    BT_BASS_CONN_PARAM param;
    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_param;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS connect_by_name <name>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));

    btmw_rpc_test_bass_get_addr_by_name(argv[0], param.bt_addr);

    if (strlen(param.bt_addr) != (MAX_BDADDR_LEN - 1))
    {
        BTMW_RPC_TEST_Logi("[BASS] not find addr by name %s\n", argv[0]);
        return -1;
    }

    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s\n", __func__, param.bt_addr);
    if (TRUE == g_bass_scan_started)
    {
        g_bass_scan_started = FALSE;
        stop_param.scanner_id = g_bass_scanner_id;
        a_mtkapi_bt_ble_scanner_stop_scan(&stop_param);
    }
    device = btmw_rpc_test_bass_get_conn_device(param.bt_addr, FALSE);
    if (device && (BT_BASS_CONN_STATE_CONNECTED == device->state))
    {
        BTMW_RPC_TEST_Logd("[BASS] already CONNECTED %s, is_builtin=%s, builtin_sync_bis=0x%08X\n",
            device->bt_addr, (TRUE == device->is_builtin) ? "YES" : "NO", device->builtin_sync_bis);
        return 0;
    }
    if (BT_SUCCESS == a_mtkapi_bt_gap_get_bond_state(param.bt_addr))
    {
        gettimeofday(&g_bass_connect_start, NULL);
        return a_mtkapi_bt_bass_connect(&param);
    }
    else
    {
        device = btmw_rpc_test_bass_get_conn_device(param.bt_addr, TRUE);
        if (NULL == device)
        {
            BTMW_RPC_TEST_Logi("[BASS] %s() no memory for addr %s\n", __func__, param.bt_addr);
            return -1;
        }
        device->is_w4bond = TRUE;
        gettimeofday(&g_bass_connect_start, NULL);
        return a_mtkapi_bt_gap_pair(param.bt_addr, 2);
    }
}

static int btmw_rpc_test_bass_get_address_by_name(int argc, char **argv)
{
    BT_BASS_CONN_PARAM param;
    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_param;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS get_address_by_name <name>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));

    btmw_rpc_test_bass_get_addr_by_name(argv[0], param.bt_addr);

    if (strlen(param.bt_addr) != (MAX_BDADDR_LEN - 1))
    {
        BTMW_RPC_TEST_Logi("[BASS] not find addr by name %s\n", argv[0]);
        return -1;
    }

    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s\n", __func__, param.bt_addr);
}

static int btmw_rpc_test_bass_disconnect(int argc, char **argv)
{
    BT_BASS_DISC_PARAM param;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS disconnect <addr>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s\n", __func__, param.bt_addr);
    return a_mtkapi_bt_bass_disconnect(&param);
}

static int btmw_rpc_test_bass_set_builtin(int argc, char **argv)
{
    BT_BASS_SET_BUILTIN_MODE_PARAM param;

    if ((argc != 2) && (argc != 3))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS set_builtin <addr> <enable> [<sync_bis>]\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.enable = (BOOL)atoi(argv[1]);
    if (3 == argc)
    {
        param.sync_bis = strtoul(argv[2], NULL, 16);
        if (param.sync_bis == 0)
        {
          BTMW_RPC_TEST_Loge("[BASS] %s() strtoul error\n", __func__);
          param.sync_bis = 0xFFFFFFFF;
        }
    }
    else
    {
        param.sync_bis = 0xFFFFFFFF;
    }
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, enable %d, sync_bis 0x%08X\n",
        __func__, param.bt_addr, param.enable, param.sync_bis);
    return a_mtkapi_bt_bass_set_builtin_mode(&param);
}

static int btmw_rpc_test_bass_set_scan(int argc, char **argv)
{
    BT_BASS_BROADCAST_SCAN_PARAM param;

    if ((argc != 2) && (argc != 3))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS set_scan <addr> <scan> [<duration_sec>]\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.scan = (BOOL)atoi(argv[1]);
    if (3 == argc)
    {
        param.duration = (UINT8)atoi(argv[2]);
    }
    else
    {
        param.duration = 30;
    }
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, scan %d, duration %d\n", __func__, param.bt_addr, param.scan, param.duration);
    return a_mtkapi_bt_bass_set_broadcast_scan(&param);
}

static int btmw_rpc_test_bass_stop_observing(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[BASS] %s()\n", __func__);
    return a_mtkapi_bt_bass_stop_broadcast_observing();
}

static int btmw_rpc_test_bass_get_receiver_state(int argc, char **argv)
{
    BT_BASS_BROADCAST_RECV_STATE_PARAM param;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS get_receiver_state <addr> <receiver_id>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.receiver_id = (INT32)atoi(argv[1]);
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, receiver_id %d\n", __func__, param.bt_addr, param.receiver_id);
    return a_mtkapi_bt_bass_get_broadcast_receiver_state(&param);
}

static int btmw_rpc_test_bass_set_code(int argc, char **argv)
{
    BT_BASS_BROADCAST_CODE_PARAM param;
    uint8_t hex_buf[200] = {0};
    int hex_bytes = 0;

    if (argc != 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS set_code <addr> <receiver_id> <code>, code sample 000102030405060708090A0B0C0D0E0F\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.receiver_id = (INT32)atoi(argv[1]);
    hex_bytes = util_ascii_2_hex(argv[2], (strlen(argv[2]) + 1) / 2, (char *)hex_buf);
    if (hex_bytes != BT_BROADCAST_BROADCAST_CODE_SIZE)
    {
        BTMW_RPC_TEST_Logi("  BASS <code> len need 16 bytes, such as 000102030405060708090A0B0C0D0E0F, current is %d\n", hex_bytes);
        return -1;
    }
    memcpy(param.code, hex_buf, BT_BROADCAST_BROADCAST_CODE_SIZE);
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, receiver_id %d\n", __func__, param.bt_addr, param.receiver_id);
    btmw_rpc_test_bass_show_code(param.code);
    return a_mtkapi_bt_bass_set_broadcast_code(&param);
}

static int btmw_rpc_test_bass_set_source(int argc, char **argv)
{
    BT_BASS_SET_BROADCAST_SRC_PARAM param;
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    int idx = 0;

    if ((argc < 3) || (argc > 7))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS set_source <addr> <adv_addr> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]\n");
        return -1;
    }

    if (NULL == g_bass_announce_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return -1;
    }

    memset(&g_bass_auto_set_code, 0, sizeof(g_bass_auto_set_code));

    memset(&param, 0, sizeof(param));
    strncpy(param.broadcast_src.bt_addr, argv[idx++], MAX_BDADDR_LEN);
    param.broadcast_src.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(param.broadcast_id.bt_addr, argv[idx++], MAX_BDADDR_LEN);
    param.broadcast_id.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.broadcast_src.sync_pa = (UINT8)atoi(argv[idx++]);

    btmw_rpc_test_bass_show_announce_list_info();
    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
    while (entry != NULL)
    {
        if (0 == strcasecmp(entry->broadcast_id.bt_addr, param.broadcast_id.bt_addr))
        {
            break;
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
    }

    if (NULL == entry)
    {
        BTMW_RPC_TEST_Logi("unknown adv_addr %s, please do set_scan firstly\n", param.broadcast_id.bt_addr);
        return -1;
    }
    else
    {
        param.broadcast_id.addr_type = entry->broadcast_id.addr_type;
        param.broadcast_id.adv_sid = entry->broadcast_id.adv_sid;
        param.broadcast_src.num_subgroups = entry->announce_data.subgroup_size;
        BTMW_RPC_TEST_Logi("num_subgroups=%d\n", param.broadcast_src.num_subgroups);
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].sync_bis = 0xFFFFFFFF;
        }
        switch (argc)
        {
        case 4:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 5:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 6:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 7:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[3].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        default:
            break;
        }
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].meta_data_len = entry->announce_data.subgroup_configs[i].meta_data_len;
            memcpy(param.broadcast_src.subgroup[i].meta_data, entry->announce_data.subgroup_configs[i].meta_data,
                BT_BROADCAST_METADATA_SIZE);
            if (0 == param.broadcast_src.subgroup[i].sync_bis)
            {
                BTMW_RPC_TEST_Logw("please confirm input vaild sync bis, sync_bis will be 0\n");
            }
            else
            {
                BTMW_RPC_TEST_Logi("sync_bis=0x%08X\n", param.broadcast_src.subgroup[i].sync_bis);
            }
            btmw_rpc_test_bass_show_meta_data(param.broadcast_src.subgroup[i].meta_data_len,
                param.broadcast_src.subgroup[i].meta_data);
        }
        BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, adv_addr %s, sync_pa %d\n",
            __func__, param.broadcast_src.bt_addr, param.broadcast_id.bt_addr, param.broadcast_src.sync_pa);
        return a_mtkapi_bt_bass_set_broadcast_source(&param);
    }
}

static int btmw_rpc_test_bass_set_source_with_code(int argc, char **argv)
{
    BT_BASS_SET_BROADCAST_SRC_PARAM param;
    BT_BASS_ANNOUNCE_CTX *entry = NULL;
    int idx = 0;
    uint8_t hex_buf[200] = {0};
    int hex_bytes = 0;

    if ((argc < 4) || (argc > 8))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS set_source <addr> <adv_addr> <code> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]\n");
        return -1;
    }

    if (NULL == g_bass_announce_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.broadcast_src.bt_addr, argv[idx++], MAX_BDADDR_LEN);
    param.broadcast_src.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(param.broadcast_id.bt_addr, argv[idx++], MAX_BDADDR_LEN);
    param.broadcast_id.bt_addr[MAX_BDADDR_LEN - 1] = '\0';

    memset(&g_bass_auto_set_code, 0, sizeof(g_bass_auto_set_code));
    hex_bytes = util_ascii_2_hex(argv[idx], (strlen(argv[idx]) + 1) / 2, (char *)hex_buf);
    if (hex_bytes != BT_BROADCAST_BROADCAST_CODE_SIZE)
    {
        BTMW_RPC_TEST_Logi("  BASS <code> len need 16 bytes, such as 000102030405060708090A0B0C0D0E0F, current is %d\n", hex_bytes);
        return -1;
    }
    memcpy(g_bass_auto_set_code.code, hex_buf, BT_BROADCAST_BROADCAST_CODE_SIZE);
    btmw_rpc_test_bass_show_code(g_bass_auto_set_code.code);
    g_bass_auto_set_code.enable = TRUE;
    idx++;

    param.broadcast_src.sync_pa = (UINT8)atoi(argv[idx++]);

    btmw_rpc_test_bass_show_announce_list_info();
    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_announce_list);
    while (entry != NULL)
    {
        if (0 == strcasecmp(entry->broadcast_id.bt_addr, param.broadcast_id.bt_addr))
        {
            break;
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_announce_list, entry);
    }

    if (NULL == entry)
    {
        BTMW_RPC_TEST_Logi("unknown adv_addr %s, please do set_scan firstly\n", param.broadcast_id.bt_addr);
        return -1;
    }
    else
    {
        param.broadcast_id.addr_type = entry->broadcast_id.addr_type;
        param.broadcast_id.adv_sid = entry->broadcast_id.adv_sid;
        param.broadcast_src.num_subgroups = entry->announce_data.subgroup_size;
        BTMW_RPC_TEST_Logi("num_subgroups=%d\n", param.broadcast_src.num_subgroups);
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].sync_bis = 0xFFFFFFFF;
        }
        switch (argc)
        {
        case 5:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 6:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 7:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 8:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[3].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        default:
            break;
        }
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].meta_data_len = entry->announce_data.subgroup_configs[i].meta_data_len;
            memcpy(param.broadcast_src.subgroup[i].meta_data, entry->announce_data.subgroup_configs[i].meta_data,
                BT_BROADCAST_METADATA_SIZE);
            if (0 == param.broadcast_src.subgroup[i].sync_bis)
            {
                BTMW_RPC_TEST_Logw("please confirm input vaild sync bis, sync_bis will be 0\n");
            }
            else
            {
                BTMW_RPC_TEST_Logi("sync_bis=0x%08X\n", param.broadcast_src.subgroup[i].sync_bis);
            }
            btmw_rpc_test_bass_show_meta_data(param.broadcast_src.subgroup[i].meta_data_len,
                param.broadcast_src.subgroup[i].meta_data);
        }
        BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, adv_addr %s, sync_pa %d\n",
            __func__, param.broadcast_src.bt_addr, param.broadcast_id.bt_addr, param.broadcast_src.sync_pa);
        return a_mtkapi_bt_bass_set_broadcast_source(&param);
    }
}


static int btmw_rpc_test_bass_modify_source(int argc, char **argv)
{
    BT_BASS_MODIFY_BROADCAST_SRC_PARAM param;
    BT_BASS_ANNOUNCE_CTX* entry = NULL;
    int idx = 0;

    if ((argc < 3) || (argc > 7))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS modify_source <addr> <receiver_id> <sync_pa> [<sync_bis> [<sync_bis> [<sync_bis> [<sync_bis>]]]]\n");
        return -1;
    }

    if (NULL == g_bass_brs_list)
    {
        BTMW_RPC_TEST_Loge("g_bass_announce_list is NULL\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.broadcast_src.bt_addr, argv[idx++], MAX_BDADDR_LEN);
    param.broadcast_src.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.receiver_id = (INT32)atoi(argv[idx++]);
    param.broadcast_src.sync_pa = (UINT8)atoi(argv[idx++]);

    btmw_rpc_test_bass_show_brs_list_info();
    entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_first(g_bass_brs_list);
    while (entry != NULL)
    {
        if (entry->receiver_id == param.receiver_id)
        {
            break;
        }
        entry = (BT_BASS_ANNOUNCE_CTX *)util_dlist_next(g_bass_brs_list, entry);
    }

    if (NULL == entry)
    {
        BTMW_RPC_TEST_Logi("unknown receiver_id %d, please do set_scan firstly\n", param.receiver_id);
        return -1;
    }
    else
    {
        param.broadcast_src.num_subgroups = entry->announce_data.subgroup_size;
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].sync_bis = 0xFFFFFFFF;
        }
        switch (argc)
        {
        case 4:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 5:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 6:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        case 7:
            param.broadcast_src.subgroup[0].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[1].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[2].sync_bis = strtoul(argv[idx++], NULL, 16);
            param.broadcast_src.subgroup[3].sync_bis = strtoul(argv[idx++], NULL, 16);
            break;

        default:
            break;
        }
        for (int i = 0; i < param.broadcast_src.num_subgroups; i++)
        {
            param.broadcast_src.subgroup[i].meta_data_len = entry->announce_data.subgroup_configs[i].meta_data_len;
            memcpy(param.broadcast_src.subgroup[i].meta_data, entry->announce_data.subgroup_configs[i].meta_data,
                BT_BROADCAST_METADATA_SIZE);
            if (0 == param.broadcast_src.subgroup[i].sync_bis)
            {
                BTMW_RPC_TEST_Logw("please confirm input vaild sync bis, sync_bis will be 0\n");
            }
            else
            {
                BTMW_RPC_TEST_Logi("sync_bis=0x%08X\n", param.broadcast_src.subgroup[i].sync_bis);
            }
            btmw_rpc_test_bass_show_meta_data(param.broadcast_src.subgroup[i].meta_data_len,
                param.broadcast_src.subgroup[i].meta_data);
        }

        BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, receiver_id %d, sync_pa %d\n",
            __func__, param.broadcast_src.bt_addr, param.receiver_id, param.broadcast_src.sync_pa);
        return a_mtkapi_bt_bass_modify_broadcast_source(&param);
    }
}

static int btmw_rpc_test_bass_remove_source(int argc, char **argv)
{
    BT_BASS_REMOVE_BROADCAST_SRC_PARAM param;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  BASS remove_source <addr> <receiver_id>\n");
        return -1;
    }

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, argv[0], MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    param.receiver_id = (INT32)atoi(argv[1]);
    BTMW_RPC_TEST_Logi("[BASS] %s() addr %s, receiver_id %d\n", __func__, param.bt_addr, param.receiver_id);
    return a_mtkapi_bt_bass_remove_broadcast_source(&param);
}

static int btmw_rpc_test_bass_dump(int argc, char **argv)
{
    btmw_rpc_test_bass_show_scan_device_list_info();
    btmw_rpc_test_bass_show_conn_device_list_info();
    btmw_rpc_test_bass_show_announce_list_info();
    btmw_rpc_test_bass_show_brs_list_info();
    btmw_rpc_test_bass_show_counter_info();
    return 0;
}

static int btmw_rpc_test_bass_reset_counter(int argc, char **argv)
{
    memset(&g_bass_counter, 0, sizeof(g_bass_counter));
    if (g_bass_exception_counter_list)
    {
        util_dlist_empty(g_bass_exception_counter_list, TRUE);
    }
    if (g_bass_announce_list)
    {
        util_dlist_empty(g_bass_announce_list, TRUE);
    }
    btmw_rpc_test_bass_show_counter_info();
    return 0;
}

int btmw_rpc_test_bass_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD bass_mod = {0};
    BT_BASS_CONNECTION_INFO conn_info;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    bass_mod.mod_id = BTMW_RPC_TEST_MOD_LEAUDIO_BASS;
    strncpy(bass_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_BASS, sizeof(bass_mod.cmd_key));
    bass_mod.cmd_handler = btmw_rpc_test_bass_cmd_handler;
    bass_mod.cmd_tbl = btmw_rpc_test_bass_cli_commands;

    ret = btmw_rpc_test_register_mod(&bass_mod);
    BTMW_RPC_TEST_Logd("[BASS] btmw_rpc_test_register_mod() returns: %d\n", ret);

    g_bass_scanner_id = 0;
    g_bass_scan_device_list = util_dlist_alloc();
    g_bass_conn_device_list = util_dlist_alloc();
    g_bass_announce_list = util_dlist_alloc();
    g_bass_brs_list = util_dlist_alloc();
    g_bass_exception_counter_list = util_dlist_alloc();
    memset(&g_bass_counter, 0, sizeof(g_bass_counter));
    if (!g_cli_pts_mode)
    {
        a_mtkapi_bt_ble_scanner_register((mtkrpcapi_BtAppBleScannerCbk *)btmw_rpc_test_bass_ble_scanner_cbk, NULL);
        a_mtkapi_bt_bass_register_callback(btmw_rpc_test_bass_app_cbk, NULL);
        a_mtkapi_bt_bass_get_connection_status(&conn_info);
        for (int i = 0; i < MAX_BASS_DEV_NUM; i++)
        {
            if (TRUE == conn_info.device[i].in_used)
            {
                device = btmw_rpc_test_bass_get_conn_device(conn_info.device[i].bt_addr, TRUE);
                if (device)
                {
                    device->state = BT_BASS_CONN_STATE_CONNECTED;
                    device->is_builtin = conn_info.device[i].is_builtin;
                    device->builtin_sync_bis = conn_info.device[i].builtin_sync_bis;
                    BTMW_RPC_TEST_Logd("[BASS] already CONNECTED %s, is_builtin=%s, builtin_sync_bis=0x%08X\n",
                        device->bt_addr, (TRUE == device->is_builtin) ? "YES" : "NO", device->builtin_sync_bis);
                }
            }
        }
    }
    BTMW_RPC_TEST_Logd("[BASS] btmw_rpc_test_bass_init done\n");
    return ret;
}

int btmw_rpc_test_bass_deinit()
{
    BT_BLE_SCANNER_UNREG_PARAM param;

    BTMW_RPC_TEST_Logi("%s", __func__);
    if (g_bass_scanner_id)
    {
        param.scanner_id = g_bass_scanner_id;
        a_mtkapi_bt_ble_scanner_unregister(&param);
    }
    if (g_bass_scan_device_list)
    {
        util_dlist_free(g_bass_scan_device_list);
    }
    if (g_bass_conn_device_list)
    {
        util_dlist_free(g_bass_conn_device_list);
    }
    if (g_bass_announce_list)
    {
        util_dlist_free(g_bass_announce_list);
    }
    if (g_bass_brs_list)
    {
        util_dlist_free(g_bass_brs_list);
    }
    if (g_bass_exception_counter_list)
    {
        util_dlist_free(g_bass_exception_counter_list);
    }
    return 0;
}

int btmw_rpc_test_bass_connect_bonded_device(CHAR* bt_addr)
{
    BT_BASS_CONN_PARAM param;
    BT_BASS_CONN_DEVICE_CTX *device = NULL;

    memset(&param, 0, sizeof(param));
    strncpy(param.bt_addr, bt_addr, MAX_BDADDR_LEN);
    param.bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    device = btmw_rpc_test_bass_get_conn_device(param.bt_addr, FALSE);
    if (device && (TRUE == device->is_w4bond))
    {
        device->is_w4bond = FALSE;
        return a_mtkapi_bt_bass_connect(&param);
    }
    return 0;
}
