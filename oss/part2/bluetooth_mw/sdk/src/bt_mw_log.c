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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <signal.h>

//#include "osi/include/config.h" //need refactor
#include "osi/include/log.h"
//#include "c_mw_config.h"
//#include "bt_mw_common.h"
#include "bt_mw_log.h"
//#include "bluetooth_sync.h"

typedef UINT8 tBTTRC_MW_LAYER_ID;

typedef UINT8 (tBTTRC_SET_MW_TRACE_LEVEL)(UINT8);

typedef struct {
    const tBTTRC_MW_LAYER_ID         layer_id_start;
    const tBTTRC_MW_LAYER_ID         layer_id_end;
    tBTTRC_SET_MW_TRACE_LEVEL        *p_f;
    const char                       *trc_name;
    UINT8                            trace_level;
}tBTTRC_MW_FUNC_MAP;


#ifndef MW_DEFAULT_CONF_TRACE_LEVEL
#define MW_DEFAULT_CONF_TRACE_LEVEL BT_MW_TRACE_LEVEL_WARNING
#endif

#if 0 //#if defined(MTK_LINUX_MW_STACK_LOG2FILE) && (MTK_LINUX_MW_STACK_LOG2FILE == TRUE) //need refactor
const char *BT_LOG_PATH_KEY = "BtStackFileName";
const char *BT_LOG2FILE_TURNED_ON_KEY = "BtStackLog2File";
const char *BT_LOG2FILE_ONLY_ON_KEY = "BtStackLogOnly2File";
const char *BT_SHOULD_SAVE_LAST_KEY = "BtStackSaveLog";
const char *BT_LOG_LEVEL_KEY = "BtStackLogLevel";
const char *BT_A2DP_LOG_LEVEL_KEY = "BtStackA2dpLogLevel";
const char *BT_PICUS_LOG_PARAM_KEY = "BtPicusParam";
const char *BT_LOG_MODE_KEY = "BtStackLogSlice";
const char *BT_MAX_LOG_FILE_SIZE_KEY = "BtStackLogFileSize";
const char *BT_MAX_FILE_NUM_KEY = "BtStackLogMaxFileNum";
const char *BT_LOG_BUF_SIZE_KEY = "BtStackLogBufSize";
const char *BT_LOG_TIMEOUT_KEY = "BtStackLogTimeout";
#endif
const char *MW_TRACE_CONFIG_ENABLED_KEY = "MWTraceConf";

extern UINT8 audio_set_trace_level(UINT8 new_level);

EXPORT_SYMBOL BT_MW_TRC_MAP btmw_trc_map[BT_DEBUG_MAX] =
{
    {BT_DBG_COMM, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_GAP, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_A2DP, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_AVRCP, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_HID, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_SPP, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_GATT, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_HFP, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_PB, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_UPL, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_MESH, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_BLE_SCANNER, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_BASS, BT_MW_TRACE_LEVEL_VERBOSE},
    {BT_DBG_BMS, BT_MW_TRACE_LEVEL_VERBOSE},
    {BT_DBG_ADV, BT_MW_TRACE_LEVEL_API},
    {BT_DBG_BMR, BT_MW_TRACE_LEVEL_VERBOSE},
};

#if 0
static bt_error_str_t errorTbl[] =
{
    {BT_ERR_SUCCESS, "Success"},
    {BT_ERR_NULL_POINTER, "Null Pointer"},
    {BT_ERR_OUT_OF_RANGE, "Out of Range"},
    {BT_ERR_UNKNOWN_CMD, "Unknown Command"},
    {BT_ERR_INVALID_PARAM, "Invalid input paramter"},
    {BT_ERR_INVALID_PARAM_NUMS, "Invalid input paramter number"},
    {BT_ERR_NOT_SUPPORT, "Not supported"},
    {BT_ERR_FAILED, "Failed"},
};
#endif

static UINT8 BTMW_Set_Comm_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_COMM].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_COMM].trace_level;
}

static UINT8 BTMW_Set_Gap_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_GAP].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_GAP].trace_level;
}

static UINT8 BTMW_Set_A2dp_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_A2DP].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_A2DP].trace_level;
}

static UINT8 BTMW_Set_Avrcp_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_AVRCP].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_AVRCP].trace_level;
}

static UINT8 BTMW_Set_Hid_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_HID].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_HID].trace_level;
}

static UINT8 BTMW_Set_Spp_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_SPP].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_SPP].trace_level;
}

static UINT8 BTMW_Set_Gatt_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_GATT].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_GATT].trace_level;
}

static UINT8 BTMW_Set_Hfp_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_HFP].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_HFP].trace_level;
}

static UINT8 BTMW_Set_Pb_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_PB].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_PB].trace_level;
}

static UINT8 BTMW_Set_Upl_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_UPL].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_UPL].trace_level;
}

static UINT8 BTMW_Set_Adv_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_BLE_ADV].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_BLE_ADV].trace_level;
}

static UINT8 BTMW_Set_Ble_Scanner_TraceLevel(UINT8 new_level)
{
    if (new_level != 0xFF)
    {
        btmw_trc_map[BT_DEBUG_BLE_SCANNER].trace_level = new_level;
    }

    return btmw_trc_map[BT_DEBUG_BLE_SCANNER].trace_level;
}

/* make sure list is order by increasing layer id!!! */
static tBTTRC_MW_FUNC_MAP btmw_trc_set_level_map[] =
{
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Comm_TraceLevel, "TRC_MW_COMM", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Gap_TraceLevel, "TRC_MW_GAP", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_A2dp_TraceLevel, "TRC_MW_A2DP", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Avrcp_TraceLevel, "TRC_MW_AVRCP", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Hid_TraceLevel, "TRC_MW_HID", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Spp_TraceLevel, "TRC_MW_SPP", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Gatt_TraceLevel, "TRC_MW_GATT", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Hfp_TraceLevel, "TRC_MW_HFP", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Pb_TraceLevel, "TRC_MW_PB", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Upl_TraceLevel, "TRC_MW_UPL", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Adv_TraceLevel, "TRC_MW_ADV", MW_DEFAULT_CONF_TRACE_LEVEL},
    {BT_MW_TRC_ID_START, BT_MW_TRC_ID_END, BTMW_Set_Ble_Scanner_TraceLevel, "TRC_MW_BLE_SCANNER", MW_DEFAULT_CONF_TRACE_LEVEL},

    {0, 0, NULL, NULL, MW_DEFAULT_CONF_TRACE_LEVEL}
};

static int s_bt_mw_dump_info_fd = -1;

static void bt_mw_load_levels_from_config(void)
{
    tBTTRC_MW_FUNC_MAP *functions;

    if (g_log_interface == NULL) return;

    for (functions = &btmw_trc_set_level_map[0]; functions->trc_name; ++functions)
    {
        int value = -1;

        g_log_interface->log_config_get_int(functions->trc_name, -1, &value);
        if (value != -1)
            functions->trace_level = value;

        printf("BT_MW_InitTraceLevels -- %s(%d)\n", functions->trc_name, functions->trace_level);

        if (functions->p_f)
            functions->p_f(functions->trace_level);
    }
}

extern int ipcd_exec(char *cmd, char *priv);
#define BT_PICUS_LOG_PARAM_KEY "BtPicusParam"
static void get_btmw_picus_log_param(char *value)
{
  if (g_log_interface == NULL) return;

  g_log_interface->log_config_get_string((char *)BT_PICUS_LOG_PARAM_KEY, (char *)"", value);
}

#define PICUS_RUN "/basic/bin/picus"
static pid_t picus_pid = -1;
#define MAX_PARAM_NUM 32
static pid_t btmw_start_app(char *app, char *param)
{
    pid_t pid = fork();
    char *argc[MAX_PARAM_NUM + 1] = {0};
    int i, len;
    char *p_start, *p_end;

    switch (pid)
    {
    case -1:
        BT_DBG_NORMAL(BT_DEBUG_COMM, " fork failed\n");
        exit(1);
        break;
    case 0:
        /* child process */
        BT_DBG_NORMAL(BT_DEBUG_COMM, " start %s(%p) in child process\n", app, param);
        p_start = param;
        for (i = 0; i < MAX_PARAM_NUM; i++)
        {
            while (*p_start == ' ')
                p_start++;
            if (*p_start == '\0')
                break;

            p_end = strstr(p_start, " ");
            if (p_end == NULL)
                p_end = p_start + strlen(p_start);

            len = p_end - p_start + 1;
            argc[i] = (char *)malloc(len);
            if (argc[i] == NULL)
            {
                BT_DBG_NORMAL(BT_DEBUG_COMM, " alloc failed(%d:%d)\n", i, len);
                exit(1);
            }
            memcpy(argc[i], p_start, len);
            *(argc[i] + len) = '\0';
            BT_DBG_NORMAL(BT_DEBUG_COMM, " %d:%s\n", i, argc[i]);
            p_start = p_end;
        }
        argc[i] = (char *)malloc(1);
        if (argc[i] == NULL)
        {
            BT_DBG_NORMAL(BT_DEBUG_COMM, " alloc failed(%d:%d)\n", i, len);
            exit(1);
        }
        *argc[i] = 0;
        execvp(app, argc);
        break;
    default:
        BT_DBG_NORMAL(BT_DEBUG_COMM, " start %s OK. return child process pid(%d)\n", app, pid);
        break;
    }
    return pid;
}

static int btmw_stop_app(pid_t pid)
{
    return kill(pid, SIGKILL);
}

void mw_log_start_picus(void)
{
    char picus_param[255] = {0};

    get_btmw_picus_log_param(picus_param);

    if (strlen(picus_param) == 0) return;

    picus_param[254] = '\0';
    BT_DBG_NORMAL(BT_DEBUG_COMM, "picus param:(%s)", picus_param);
    picus_pid = btmw_start_app(PICUS_RUN, picus_param);
}

void mw_log_stop_picus(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_COMM, "");
    if (picus_pid >= 0)
        btmw_stop_app(picus_pid);
    picus_pid = -1;
}

const log_interface_t *g_log_interface = NULL;
void mw_log_init(void *stack_handle)
{
    const char *path = BLUETOOTH_STACK_CONFIG_FOLDER"/bt_stack.conf";

    /* In case of configuration files in /data/misc/bluedroid/ are deleted,
    * configuration files in /etc/bluetooth/ can be alternates
    */
    if((path == NULL) || access(path, R_OK))
    {
        if(!access("/etc/bluetooth/bt_stack.conf", R_OK))
        {
            path = "/etc/bluetooth/bt_stack.conf";
        }
    }

    assert(path != NULL);
    BT_DBG_WARNING(BT_DEBUG_COMM, "%s attempt to load mw conf from %s", __func__, path);

    assert(stack_handle != NULL);
    g_log_interface = (log_interface_t *)dlsym(stack_handle, "LogInterface");
    if (NULL == g_log_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get Log interface");
        return;
    }

    bt_mw_load_levels_from_config();
}

INT32 bt_get_dbg_level(BT_DEBUG_LAYER_NAME_T layer)
{
    UINT32 i = 0;
    if (BT_DEBUG_MAX <= layer || layer < 0)
    {
        for (i = 0; i < BT_DEBUG_MAX; i++)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "layer:%d level:%d", i, btmw_trc_map[i].trace_level);
        }
        return -1;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "layer:%d level:%d", layer, btmw_trc_map[layer].trace_level);
        return btmw_trc_map[layer].trace_level;
    }
}

VOID bt_set_dbg_level(BT_DEBUG_LAYER_NAME_T layer, INT32 level)
{
    UINT32 i = 0;
    if (BT_DEBUG_MAX <= layer || layer < 0)
    {
        for (i = 0; i < BT_DEBUG_MAX; i++)
        {
            btmw_trc_map[i].trace_level = level;
        }
    }
    else
    {
        btmw_trc_map[layer].trace_level = level;
    }
    BT_DBG_WARNING(BT_DEBUG_COMM, "layer:%d level:%d", layer, level);
}


EXPORT_SYMBOL VOID bt_mw_dump_info_begin(CHAR *dump_file_name)
{
    char log_path[64] = "/tmp/bt_dump/";

    if (mkdir(log_path, 0770) == -1)
    {
        if (errno != EEXIST)
        {
            printf("mkdir %s error! %s, use /tmp/\n", log_path, (char*)strerror(errno));
            strncpy(log_path, "/tmp/", strlen(log_path));
        }
    }

    strncat(log_path, dump_file_name, 63);
    log_path[63] = '\0';
    if (-1 == s_bt_mw_dump_info_fd)
    {
        s_bt_mw_dump_info_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (-1 == s_bt_mw_dump_info_fd)
        {
            printf("%s unable to open '%s': %s\n", __func__, log_path, strerror(errno));
        }
    }
}

EXPORT_SYMBOL VOID bt_mw_dump_info_write(const char *format, ...)
{
    va_list args;
    char msg[500] = {0};

    va_start(args, format);
    (void)vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    if (s_bt_mw_dump_info_fd != -1)
    {
        write(s_bt_mw_dump_info_fd, msg, strlen(msg));
    }
    return;
}

EXPORT_SYMBOL VOID bt_mw_dump_info_end(VOID)
{
    if (s_bt_mw_dump_info_fd != -1)
    {
        close(s_bt_mw_dump_info_fd);
        s_bt_mw_dump_info_fd = -1;
    }
}

