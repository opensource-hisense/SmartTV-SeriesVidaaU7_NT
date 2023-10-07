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

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sched.h>

#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include "c_mw_config.h"
// Local header files.
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_tools_if.h"
#include "edit.h"
#include "eloop.h"
#include "util.h"

// Module header files
#include "btmw_rpc_test_gap_if.h"
#include "btmw_rpc_test_a2dp_sink_if.h"
#include "btmw_rpc_test_a2dp_src_if.h"
#include "btmw_rpc_test_hid_if.h"
#include "btmw_rpc_test_gatts_if.h"
#include "btmw_rpc_test_gattc_if.h"
#include "btmw_rpc_test_avrcp_if.h"
#include "btmw_rpc_test_avrcp_tg_if.h"
#include "btmw_rpc_test_hfclient_if.h"
#include "btmw_rpc_test_spp_if.h"
#include "btmw_rpc_test_mesh_if.h"
#include "btmw_rpc_test_leaudio_bms_if.h"
#include "btmw_rpc_test_leaudio_bass_if.h"
#include "btmw_rpc_test_leaudiohw_src_if.h"
#include "btmw_rpc_test_leaudiohw_uploader_if.h"
#include "btmw_rpc_test_ble_scanner_if.h"
#include "btmw_rpc_test_ble_advertiser_if.h"

#include "rw_init_mtk_bt_service.h"
#include "c_mw_config.h"



#define BT_RPC_TEST_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

// Macro definition
#define BTMW_RPC_TEST_CMD_KEY_CNF    "MW_CONF"

// Data structure
typedef struct _btmw_rpc_test_cntx_t
{
    // History file path
    char hst_path[BTMW_RPC_TEST_MAX_PATH_LEN];
    // Registered modules
    unsigned int mods_cnt;
    BTMW_RPC_TEST_MOD mods[BTMW_RPC_TEST_MAX_MODULES];
} BTMW_RPC_TEST_CNTX;

enum
{
    OPT_CLI_DEBUG = 1,
    OPT_CLI_PTS = 2,
    OPT_CLI_FLAGS,
};

// The socket addr for being filled when receving message.
static BTMW_RPC_TEST_CNTX g_btmw_rpc_test_cntx;
int g_cli_pts_mode = 0;

static int btmw_rpc_test_print_help(void);
static int btmw_rpc_test_conf_init(void);
static int btmw_rpc_test_conf_cmd_handler(int argc, char **argv);
static int btmw_rpc_test_get_version_handler(int argc, char *argv[]);
static int btmw_rpc_test_set_btcli_handler(int argc, char *argv[]);
static int btmw_rpc_test_edit_filter_hst_cb(void *cntx, const char *cmd);

static BTMW_RPC_TEST_CLI btmw_rpc_test_conf_cli_commands[] =
{
    { "set_btcli", btmw_rpc_test_set_btcli_handler,
      " = set btcli log level <bitmap>"},
    { "get_version", btmw_rpc_test_get_version_handler,
      " = get mw version"},
    { NULL, NULL, NULL }
};

int btmw_rpc_test_init(BOOL cli_multi);
int btmw_rpc_test_run();
int btmw_rpc_test_deinit();

static BTMW_RPC_TEST_MOD *btmw_rpc_test_find_mod_by_id(int mod_id);

#if 0
static void btmw_rpc_test_signal_handler(int sig)
{
    (void)btmw_rpc_test_deinit();
    exit(0);
}
#endif

static void btmw_rpc_test_sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void btmw_rpc_test_cmd_quit()
{
    struct itimerval itv;
#if 0
    (void)signal(SIGALRM, btmw_rpc_test_signal_handler);
#endif
    // Start a timer to disable BT in 3 seconds.
    itv.it_value.tv_sec = 3;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &itv, NULL);
}

static void btmw_rpc_test_edit_cmd_cb(void *cntx, char *cmd)
{
    BTMW_RPC_TEST_MOD *mod;
    char cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN] = {0};
    char *argv[BTMW_RPC_TEST_MAX_ARGS] = {NULL};
    unsigned int i, found = 0;
    int argc = 0;

    strncpy(cmd_buf, cmd, BTMW_RPC_TEST_MAX_PATH_LEN - 1);
    cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN - 1] = '\0';
    argc = util_tokenize_cmd(cmd_buf, argv, BTMW_RPC_TEST_MAX_ARGS);

    if (argc > 0)
    {
        if (!strncmp(argv[0], "quit", 4) ||
            !strncmp(argv[0], "exit", 4) ||
            !strncmp(argv[0], "q", 1))
        {
            btmw_rpc_test_cmd_quit();
            return;
        }

        if (!strncmp(argv[0], "help", 4) )
        {
            btmw_rpc_test_print_help();

            return;
        }

        for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
        {
            mod = &g_btmw_rpc_test_cntx.mods[i];
            if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
            {
                mod->cmd_handler( argc - 1, &argv[1]);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            BTMW_RPC_TEST_Logw("[BTMW_RPC_TEST] Invalid cmd: %s\n", argv[0]);
            btmw_rpc_test_print_help();
        }
    }
}

static void btmw_rpc_test_edit_eof_cb(void *cntx)
{
    eloop_terminate();
}

static char **btmw_rpc_test_build_1st_cmds(void)
{
    BTMW_RPC_TEST_MOD *mod;
    char **res;
    unsigned int i, count;

    count = g_btmw_rpc_test_cntx.mods_cnt + 1;
    res = (char **)util_zalloc(count * sizeof(char *));
    if (res == NULL)
    {
        return NULL;
    }

    for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
    {
        mod = &g_btmw_rpc_test_cntx.mods[i];

        res[i] = strdup(mod->cmd_key);
    }
    res[i] = NULL;

    return res;
}

static char **btmw_rpc_test_build_2nd_cmds(BTMW_RPC_TEST_MOD *mod)
{
    BTMW_RPC_TEST_CLI *tbl;
    char **res;
    int i, count;

    tbl = mod->cmd_tbl;

    count = 0;
    for (i = 0; tbl[i].cmd; i++)
    {
        count++;
    }
    count++;

    res = (char **)util_zalloc(count * sizeof(char *));
    if (res == NULL)
        return res;

    for (i = 0; tbl[i].cmd; i++)
    {
        res[i] = strdup(tbl[i].cmd);
        if (res[i] == NULL)
            break;
    }

    return res;
}

static char **btmw_rpc_test_edit_cpl_cb(void *cntx, const char *cmd, int pos)
{

    BTMW_RPC_TEST_MOD *mod;
    char cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN] = {0};
    char *argv[BTMW_RPC_TEST_MAX_ARGS] = {0};
    const char *end;
    unsigned int i;
    int argc = 0;

    strncpy(cmd_buf, cmd, BTMW_RPC_TEST_MAX_PATH_LEN - 1);
    cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN - 1] = '\0';
    argc = util_tokenize_cmd(cmd_buf, argv, BTMW_RPC_TEST_MAX_ARGS);

    if (argc < 2)
    {
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            return btmw_rpc_test_build_1st_cmds();
        }
    }
    else
    {
        end = strchr(cmd, ' ');
        if (end == NULL)
            return NULL;
        cmd = end + 1;
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
            {
                mod = &g_btmw_rpc_test_cntx.mods[i];
                if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
                {
                   return btmw_rpc_test_build_2nd_cmds(mod);
                }
            }
        }
    }

    return NULL;
}

static int btmw_rpc_test_edit_filter_hst_cb(void *cntx, const char *cmd)
{
    return 0;
}
static BTMW_RPC_TEST_MOD *btmw_rpc_test_find_mod_by_id(int mod_id)
{
    BTMW_RPC_TEST_MOD *mod = NULL;
    unsigned int i;

    for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
    {
        if (g_btmw_rpc_test_cntx.mods[i].mod_id == mod_id)
        {
            mod = &g_btmw_rpc_test_cntx.mods[i];
            break;
        }
    }

    return mod;
}

int btmw_rpc_test_register_mod(BTMW_RPC_TEST_MOD *mod)
{
    unsigned int i = 0;

    if (mod == NULL)
    {
        BTMW_RPC_TEST_Loge("[BTMW_RPC_TEST] mod is NULL\n");
        return -1;
    }

    if (mod->cmd_handler == NULL)
    {
        BTMW_RPC_TEST_Loge("[BTMW_RPC_TEST] cmd_handler: %x\n", mod->cmd_handler);
        return -1;
    }

    if (g_btmw_rpc_test_cntx.mods_cnt >= BTMW_RPC_TEST_MAX_MODULES - 1)
    {
        BTMW_RPC_TEST_Logw("[BTMW_RPC_TEST] module table is full. mods_cnts: %d\n", g_btmw_rpc_test_cntx.mods_cnt);
        return -1;
    }

    if (btmw_rpc_test_find_mod_by_id(mod->mod_id) != NULL)
    {
        BTMW_RPC_TEST_Logw("[BTMW_RPC_TEST] duplicated registration for mod_id: %d\n", mod->mod_id);
        return -1;
    }

    i = g_btmw_rpc_test_cntx.mods_cnt;
    if (i < BTMW_RPC_TEST_MAX_MODULES) {
        g_btmw_rpc_test_cntx.mods[i].mod_id = mod->mod_id;
        strncpy(g_btmw_rpc_test_cntx.mods[i].cmd_key, mod->cmd_key, BTMW_RPC_TEST_MAX_KEY_LEN);
        g_btmw_rpc_test_cntx.mods[i].cmd_handler = mod->cmd_handler;
        g_btmw_rpc_test_cntx.mods[i].cmd_tbl = mod->cmd_tbl;

        g_btmw_rpc_test_cntx.mods_cnt++;
    }

    return 0;
}

static int btmw_rpc_test_conf_init(void)
{
    int ret = 0;
    BTMW_RPC_TEST_MOD btmw_rpc_test_conf_mod = {0};

    // Register to btmw_rpc_test_cli.
    btmw_rpc_test_conf_mod.mod_id = BTMW_RPC_TEST_MOD_CONF;
    strncpy(btmw_rpc_test_conf_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_CNF, sizeof(btmw_rpc_test_conf_mod.cmd_key));
    btmw_rpc_test_conf_mod.cmd_handler = btmw_rpc_test_conf_cmd_handler;
    btmw_rpc_test_conf_mod.cmd_tbl = btmw_rpc_test_conf_cli_commands;

    ret = btmw_rpc_test_register_mod(&btmw_rpc_test_conf_mod);
    BTMW_RPC_TEST_Logi("[GAP] btmw_rpc_test_register_mod() returns: %d\n", ret);

    return ret;
}

static int btmw_rpc_test_conf_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_conf_cli_commands;

    BTMW_RPC_TEST_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logi("Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_CNF, btmw_rpc_test_conf_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static int btmw_rpc_test_get_version_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[GAP] %s()\n", __func__);
    //GetVersion();
    return 0;
}

static int btmw_rpc_test_set_btcli_handler(int argc, char *argv[])
{
    int flag;

    BTMW_RPC_TEST_Logi("[GAP] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_CONF set_btcli <bitmap>\n");
        BTMW_RPC_TEST_Logi("     Bit 0 - BTMW_RPC_TEST_LOG_LV_VBS\n");
        BTMW_RPC_TEST_Logi("     Bit 1 - BTMW_RPC_TEST_LOG_LV_INF\n");
        BTMW_RPC_TEST_Logi("     Bit 2 - BTMW_RPC_TEST_LOG_LV_DBG\n");
        BTMW_RPC_TEST_Logi("     Bit 3 - BTMW_RPC_TEST_LOG_LV_WRN\n");
        BTMW_RPC_TEST_Logi("     Bit 4 - BTMW_RPC_TEST_LOG_LV_ERR\n");
        BTMW_RPC_TEST_Logi("\n");
        BTMW_RPC_TEST_Logi("     Bit 8 - BTMW_RPC_TEST_LOG_FLAG_COLOR\n");
        BTMW_RPC_TEST_Logi("     Bit 9 - BTMW_RPC_TEST_LOG_FLAG_TIMESTAMP\n");

        return -1;
    }

    flag = strtol(argv[0], NULL, 16);
    if ((flag == LONG_MIN) || (flag == LONG_MAX))
        return -1;

    BTMW_RPC_TEST_Log_SetFlag((unsigned short)flag);

    BTMW_RPC_TEST_Logi("[GAP] lv = %x\n", flag);

    return 0;
}

int btmw_rpc_test_init(BOOL cli_multi)
{
    char cwd[BTMW_RPC_TEST_MAX_PATH_LEN - 32] = {0};

    memset(&g_btmw_rpc_test_cntx, 0, sizeof(BTMW_RPC_TEST_CNTX));
    getcwd(cwd, sizeof(cwd));

    (void)snprintf(g_btmw_rpc_test_cntx.hst_path, BTMW_RPC_TEST_MAX_PATH_LEN, "%s/%s", cwd, BTMW_RPC_TEST_HISTORY_FILE);
    BTMW_RPC_TEST_Logv("History file path: %s\n", g_btmw_rpc_test_cntx.hst_path);

    if (eloop_init())
    {
        BTMW_RPC_TEST_Loge("Failed to init eloop.\n");
        return -1;
    }
#if 0
    (void)signal(SIGINT, btmw_rpc_test_signal_handler);
    (void)signal(SIGTERM, btmw_rpc_test_signal_handler);
#endif
    (void)signal(SIGCHLD, btmw_rpc_test_sigchld_handler);

    btmw_rpc_test_tools_init();
    btmw_rpc_test_conf_init();
    btmw_rpc_test_gap_init();

#if ENABLE_HID_PROFILE_H
    btmw_rpc_test_hid_init();
#endif

#if ENABLE_A2DP_SRC
    btmw_rpc_test_a2dp_src_init();
    btmw_rpc_test_rc_tg_init();
#endif

#if ENABLE_A2DP_SINK
    btmw_rpc_test_a2dp_sink_init();
    btmw_rpc_test_rc_init();
#endif

#if MTK_LINUX_HFP
    btmw_rpc_test_hfclient_init();
#endif

#if ENABLE_SPP_PROFILE
    btmw_rpc_test_spp_init();
#endif

#if ENABLE_BLE_MESH
    btmw_rpc_test_mesh_init();
#endif

#if ENABLE_GATT_PROFILE
    btmw_rpc_test_ble_scanner_init();
    btmw_rpc_test_ble_adv_init();
    btmw_rpc_test_gattc_init();
    btmw_rpc_test_gatts_init();
#endif

#if MTK_LEAUDIO_BMS
    btmw_rpc_test_bms_init();
    btmw_rpc_test_bass_init();
    btmw_rpc_test_leaudiohw_src_init();
#if DTV_NO_APP_FOR_UPLOADER_AND_PLAYBACK
    btmw_rpc_test_leaudiohw_uploader_init();
#endif
#endif
#if MTK_LEAUDIO_BMR
    btmw_rpc_test_bmr_init();
#endif

    BTMW_RPC_TEST_Logi("[BTMW_RPC_TEST] init ok %s\n", g_cli_pts_mode?"PTS Mode":"Normal Mode");
    return 0;
}

int btmw_rpc_test_run()
{
    BTMW_RPC_TEST_Logi("[BTMW_RPC_TEST] running.\n");

    edit_init(
        btmw_rpc_test_edit_cmd_cb,
        btmw_rpc_test_edit_eof_cb,
        btmw_rpc_test_edit_cpl_cb,
        NULL, g_btmw_rpc_test_cntx.hst_path);

    eloop_run();

    return 0;
}

int btmw_rpc_test_deinit()
{
    BTMW_RPC_TEST_Logi("[BTMW_RPC_TEST] deinit. Register mods: %d\n", g_btmw_rpc_test_cntx.mods_cnt);

    edit_set_finish(1);
    edit_deinit(g_btmw_rpc_test_cntx.hst_path, btmw_rpc_test_edit_filter_hst_cb);

    eloop_terminate();
    eloop_destroy();

    return 0;
}

static int btmw_rpc_test_print_help(void)
{
    BTMW_RPC_TEST_MOD *mod;
    unsigned int i;

    for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
    {
        mod = &g_btmw_rpc_test_cntx.mods[i];

        btmw_rpc_test_print_cmd_help(mod->cmd_key, mod->cmd_tbl);
    }

    return 0;
}

void btmw_rpc_test_print_cmd_help(const char* prefix, BTMW_RPC_TEST_CLI *tbl)
{
    int i;
    char c;

    printf("=============================== %s ===============================\n", prefix);

    while(tbl->cmd)
    {
        if(prefix)
        {
            printf("  %s %s", prefix, tbl->cmd);
        }
        else
        {
            printf("  %s", tbl->cmd);
        }


        for (i = 0; (c = tbl->usage[i]); i++)
        {
            printf("%c", c);
            if (c == '\n')
                printf("%s", prefix);
        }
        printf("\n");

        tbl++;
    }
}

static void usage(void)
{
    printf(
"Usage: btcli [OPTION]...\n"
"\n"
"-h, --help              help\n"
"-c, --cli=#             choose cli mode. 0=disable, 1=enable\n"
"    --cli_debug=#       choose cli debug bitmap. #=2 byte hex number\n"
"    --cli_multi=#       choose multi mode. #=0 mode=disble, #=1 mode=enable\n"
        );
}

#if defined(MTK_BT_SYS_LOG)
__attribute__((visibility("default")))UINT32 ui4_enable_all_log = 0;
#endif

#define BTMW_RPC_DBG_FIFO "/tmp/btmw_rpc_cmd_fifo"
static pthread_t  btmw_rpc_dbg_thread;

static void btmw_rpc_test_cmd_handle(char *cmd)
{
    BTMW_RPC_TEST_MOD *mod;
    char cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN] = {0};
    char *argv[BTMW_RPC_TEST_MAX_ARGS] = {NULL};
    unsigned int i, found = 0;
    int argc = 0;

    strncpy(cmd_buf, cmd, BTMW_RPC_TEST_MAX_PATH_LEN - 1);
    cmd_buf[BTMW_RPC_TEST_MAX_PATH_LEN - 1] = '\0';
    argc = util_tokenize_cmd(cmd_buf, argv, BTMW_RPC_TEST_MAX_ARGS);

    if (argc > 0)
    {
        if (!strncmp(argv[0], "quit", 4) ||
            !strncmp(argv[0], "exit", 4) ||
            !strncmp(argv[0], "q", 1))
        {
            btmw_rpc_test_cmd_quit();
            return;
        }

        if (!strncmp(argv[0], "help", 4) )
        {
            btmw_rpc_test_print_help();
            return;
        }

        for (i = 0; i < g_btmw_rpc_test_cntx.mods_cnt; i++)
        {
            mod = &g_btmw_rpc_test_cntx.mods[i];
            if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
            {
                mod->cmd_handler(argc - 1, &argv[1]);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            BTMW_RPC_TEST_Logw("[BTMW_RPC_TEST] Invalid cmd: %s\n", argv[0]);
            btmw_rpc_test_print_help();
        }
    }
}

static VOID* btwm_rpc_test_cmd_recv_thread(VOID * args)
{
    INT32 ret = 0;
    INT32 i4_ret = 0;
    INT32 fifoFd = -1;
    char cmd_str[257] = {0};
    char fifo_name[64] = BTMW_RPC_DBG_FIFO;

    if (NULL != args)
    {
        strncpy(fifo_name, (char*)args, 63);
        fifo_name[63] = '\0';
    }

    if (remove(fifo_name)) {
        printf( "%s can't remove", fifo_name);
    } else {
        printf("%s removed", fifo_name);
    }

    i4_ret = mkfifo(fifo_name, 0777);
    if (i4_ret < 0) {
        printf("mkfifo %s fail:%d\n", fifo_name, i4_ret);
        return NULL;
    }
    printf("mkfifo success\n");

    prctl(PR_SET_NAME, "btwm_rpc_msg_recv_thread", 0, 0, 0);
    printf("Enter %s fifo_name: %s\n", __FUNCTION__, fifo_name);

    while(1)
    {
        memset(cmd_str, 0, sizeof(cmd_str));
        if (-1 == fifoFd)
        {
            fifoFd = open(fifo_name, O_RDONLY);
            if (fifoFd < 0)
            {
                printf("%s fifo open fail:%s\n", fifo_name, strerror(errno));
                return NULL;
            }
        }

        ret = read(fifoFd, cmd_str, 256);
        cmd_str[256] = '\0';
        if (ret <= 0) {
            close(fifoFd);
            fifoFd = -1;
            continue;
        }

        if (cmd_str[ret-1] <= 0x20)cmd_str[ret-1] = '\0';
        else cmd_str[ret] = '\0';

        printf("cmd_str=%s\n", cmd_str);
        btmw_rpc_test_cmd_handle(cmd_str);
    }

    return NULL;
}

static int btmw_rpc_test_loop_run_init(char *fifo_name)
{
    INT32 i4_ret = 0;
    printf("Enter %s", __FUNCTION__);

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 1;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        printf( "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }

    i4_ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (0 != i4_ret)
    {
        printf("pthread_attr_setschedpolicy i4_ret:%d", i4_ret);
    }

    i4_ret = pthread_attr_setschedparam(&attr, &param);
    if (0 != i4_ret)
    {
        printf("pthread_attr_setschedparam i4_ret:%d", i4_ret);
    }

    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&btmw_rpc_dbg_thread,
                                          &attr,
                                          btwm_rpc_test_cmd_recv_thread,
                                          fifo_name)))
        {
            printf("pthread_create i4_ret:%ld", (long)i4_ret);
        }
    }
    else
    {
        printf("pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    pthread_attr_destroy(&attr);

    return 0;
}

void btmw_rpc_test_stack_exit_cb(void *pv_tag)
{
    stack_exit_cb func = btmw_rpc_test_stack_exit_cb;
    BOOL cli_multi = pv_tag;

    BT_RPC_TEST_LOG("cli_multi=%d\n", cli_multi);
//    btmw_rpc_test_deinit();
    BT_RPC_TEST_LOG("1111 \n");
    a_mtk_bt_service_terminate();
    BT_RPC_TEST_LOG("2222 \n");
    sleep(1);
    a_mtk_bt_service_init();
    BT_RPC_TEST_LOG("3333 \n");
    if (a_mtk_bt_register_stack_exit_cb(func, NULL) < 0)
    {
        BT_RPC_TEST_LOG("Reg stack exit_cb failed");
    }
    BT_RPC_TEST_LOG("4444 \n");
//    btmw_rpc_test_init(cli_multi);
}

int main(int argc, char **argv)
{
    int option_index;
    static const char short_options[] = "hc:";
    static const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"cli", 1, 0, 'c'},
        {"cli_debug", 1, 0, OPT_CLI_DEBUG},
        {"pts", 1, 0, OPT_CLI_PTS},
        {"cli_multi", 1, 0, 'm'},
        {"fifo", 1, 0, 'f'},
        {0, 0, 0, 0}
    };
    int c;
    int cli_mode = 1;
    int cli_debug = 0;
    char *cli_debug_bitmap;
    int opt_argc;
    BOOL cli_multi = FALSE;
    char *opt_argv[2];
    struct sched_param param;
    int i4_ret = 0;
    char fifo_name[64] = BTMW_RPC_DBG_FIFO;
    stack_exit_cb func = btmw_rpc_test_stack_exit_cb;

    param.sched_priority = 95;
    printf("increase the priority of btmw_rpc_test.");

    i4_ret = sched_setscheduler(0, SCHED_RR, &param);
    printf("i4_ret:%d @ %s\n", i4_ret, __FUNCTION__);
    if (-1 == i4_ret)
    {
        printf("btmw_rpc_test sched_setscheduler error\n");
    }
    else
    {
        printf("set btmw_rpc_test priority done\n");
    }

#if defined(MTK_BT_SYS_LOG)
    /*init output log type*/
    if (0 == access("/data/log_all", 0))
    {
        printf("enable all ouput in btmw_rpc_test!!\n");
        ui4_enable_all_log = 1;
    }
#endif

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'h':
            usage();
            return 0;
        case 'c':
            cli_mode = strtol(optarg, NULL, 0);
            if ((cli_mode == LONG_MIN) || (cli_mode == LONG_MAX)) {
                (void)fprintf(stderr, "parse cli_mode failed.\n");
                return -1;
            }
            break;
        case OPT_CLI_DEBUG:
            cli_debug = 1;
            cli_debug_bitmap = optarg;
            break;
        case OPT_CLI_PTS:
            g_cli_pts_mode = strtol(optarg, NULL, 0);
            if ((g_cli_pts_mode == LONG_MIN) || (g_cli_pts_mode == LONG_MAX)) {
                (void)fprintf(stderr, "parse g_cli_pts_mode failed.\n");
                return -1;
            }
            break;
        case 'm':
            cli_mode = strtol(optarg, NULL, 0);
            if ((cli_mode == LONG_MIN) || (cli_mode == LONG_MAX)) {
                (void)fprintf(stderr, "parse cli_mode_multi failed.\n");
                return -1;
            }
            cli_multi = TRUE;
            break;
        case 'f':
            strncpy(fifo_name, optarg, 31);
            fifo_name[31] = '\0';
            break;
        default:
            (void)fprintf(stderr, "Try --help' for more information.\n");
            return 1;
        }
    }

    if (cli_debug)
    {
        opt_argc = 2;
        opt_argv[0] = cli_debug_bitmap;
        opt_argv[1] = "-1";

        btmw_rpc_test_set_btcli_handler(opt_argc, &opt_argv[0]);
    }

    BT_RPC_TEST_LOG("IPC/RPC initialize");
    a_mtk_bt_service_init();
    if (a_mtk_bt_register_stack_exit_cb(func, cli_multi) < 0)
    {
        BT_RPC_TEST_LOG("Reg stack exit_cb failed");
        a_mtk_bt_service_terminate();
        return -1;
    }

    sleep(3);
    BT_RPC_TEST_LOG("initialize");

    if (btmw_rpc_test_init(cli_multi))
    {
        return -1;
    }

    if (cli_mode)
    {
        btmw_rpc_test_run();
        btmw_rpc_test_deinit();
    }
    else
    {
        btmw_rpc_test_loop_run_init(fifo_name);
        pause();
    }

    BTMW_RPC_TEST_Logw("[BTMW_RPC_TEST] exit\n");
    return 0;
}
