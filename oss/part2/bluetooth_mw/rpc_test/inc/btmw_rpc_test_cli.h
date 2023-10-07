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

#ifndef __BTMW_RPC_TEST_CLI_H__
#define __BTMW_RPC_TEST_CLI_H__
#include "c_mw_config.h"

#define BTMW_RPC_TEST_SUCCESS 0
#define BTMW_RPC_TEST_FAILED (-1)

#define BTMW_RPC_TEST_MAX_ARGS 25
#define BTMW_RPC_TEST_MAX_KEY_LEN    32

#define BTMW_RPC_TEST_MAX_PATH_LEN   256
#define BTMW_RPC_TEST_HISTORY_FILE   ".btmw_rpc_test_history"

#define BTMW_RPC_TEST_MAX_MODULES    40

typedef struct btmw_rpc_test_cli_t
{
    const char *cmd;
    int (*handler)(int argc, char *argv[]);
    const char *usage;
} BTMW_RPC_TEST_CLI;

typedef int (*BTMW_RPC_TEST_CMD_HANDLER)(int argc, char **argv);

enum
{
    BTMW_RPC_TEST_MOD_GAP = 0,
    BTMW_RPC_TEST_MOD_HFCLIENT,
    BTMW_RPC_TEST_MOD_A2DP_SINK,
    BTMW_RPC_TEST_MOD_A2DP_SRC,
    BTMW_RPC_TEST_MOD_GATT_CLIENT,
    BTMW_RPC_TEST_MOD_GATT_SERVER,
    BTMW_RPC_TEST_MOD_BLE_SCANNER,
    BTMW_RPC_TEST_MOD_BLE_ADV,
    BTMW_RPC_TEST_MOD_HID,
    BTMW_RPC_TEST_MOD_AVRCP_CT,
    BTMW_RPC_TEST_MOD_AVRCP_TG,
    BTMW_RPC_TEST_MOD_SPP,
    BTMW_RPC_TEST_MOD_CONF,
    BTMW_RPC_TEST_MOD_TOOLS,
    BTMW_RPC_TEST_MOD_MESH,
    BTMW_RPC_TEST_MOD_LEAUDIO_BASS,
    BTMW_RPC_TEST_MOD_LEAUDIO_BMS,
    BTMW_RPC_TEST_MOD_LEAUDIO_SRC,
    BTMW_RPC_TEST_MOD_LEAUDIO_UPLOADER,
    BTMW_RPC_TEST_MOD_LEAUDIO_BMR,
    BTMW_RPC_TEST_MOD_NUM,
};

typedef struct btmw_rpc_test_mod_t
{
    int mod_id;
    char cmd_key[BTMW_RPC_TEST_MAX_KEY_LEN];
    BTMW_RPC_TEST_CMD_HANDLER cmd_handler;
    BTMW_RPC_TEST_CLI *cmd_tbl;
} BTMW_RPC_TEST_MOD;

#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_leaudio_bms_wrapper.h"
#include "u_rpcipc_types.h"

typedef struct a2dp_leaudio_uploader_config_t {
    UINT32 sample_rate;//for src/sink
    UINT32 channel_num;//for src/sink
    BT_BMS_BROADCAST_STATE bst;
    BT_A2DP_EVENT src_st;
    BT_A2DP_EVENT snk_st;
    BOOL leaudio_uploader_in_use;
    BOOL a2dpsrc_uploader_in_use;
    BOOL a2dpsnk_playback_in_use;
} a2dp_leaudio_uploader_config_t;

typedef struct socketidx_map_uploaderhdl_t {
    int socketidx;
    long hUploaderhandle;
    BOOL in_use;
} socketidx_map_uploaderhdl_t;

extern void btmw_rpc_test_print_cmd_help(const char* prefix, BTMW_RPC_TEST_CLI *tbl);
extern int btmw_rpc_test_register_mod(BTMW_RPC_TEST_MOD *mod);
extern int g_cli_pts_mode;

#endif
