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

/* FILE NAME:  bt_mw_a2dp_common.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
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
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <strings.h>

#include "bt_mw_a2dp_common.h"
#include "c_mw_config.h"

#if ENABLE_A2DP_SINK || ENABLE_A2DP_SRC
#include "linuxbt_a2dp_common_if.h"
#endif

#if ENABLE_A2DP_SINK
#include "linuxbt_a2dp_sink_if.h"
#include "linuxbt_avrcp_ct_if.h"
#include "bt_mw_a2dp_snk.h"
#endif
#if ENABLE_A2DP_SRC
#include "linuxbt_a2dp_src_if.h"
#include "linuxbt_avrcp_tg_if.h"
#include "bt_mw_a2dp_src.h"
#endif

#include "dev_list.h"

#include "bt_mw_message_queue.h"

#include "bt_mw_gap.h"

/* NAMING CONSTANT DECLARATIONS
 */

//#define BT_MW_A2DP_MAX_DEVICES  (4) /* 3 INT + 1 ACP */

#define BT_MW_A2DP_MAX_DEVICE_LIST  (10)

typedef enum
{
    BT_MW_A2DP_CONNECT_STATUS_DISCONNECTED,
    BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING,
    BT_MW_A2DP_CONNECT_STATUS_CONNECTING,
    BT_MW_A2DP_CONNECT_STATUS_CONNECTED,
    BT_MW_A2DP_CONNECT_STATUS_MAX
} BT_MW_A2DP_CONNECT_STATUS;

typedef enum
{
    BT_MW_A2DP_PENDING_SRC_DISABLE,
    BT_MW_A2DP_PENDING_SINK_DISABLE,
    BT_MW_A2DP_PENDING_SRC_ENABLE,
    BT_MW_A2DP_PENDING_SINK_ENABLE,
    BT_MW_A2DP_PENDING_POWER_OFF,
    BT_MW_A2DP_PENDING_DEINIT,
} BT_MW_A2DP_PENDING_FLAG;

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_A2DP_IS_VALID_ADDR(addr) do{                                          \
        if ((NULL == addr) || ((MAX_BDADDR_LEN - 1) != strlen(addr)))               \
        {                                                                           \
            BT_DBG_ERROR(BT_DEBUG_A2DP, "invalid addr(%s)", addr==NULL?"NULL":addr);\
            return BT_ERR_STATUS_PARM_INVALID;                                      \
        }                                                                           \
    }while(0)

#define BT_MW_A2DP_IS_VALID_ROLE(role) do {                             \
        if (role >= BT_MW_A2DP_ROLE_MAX || role < BT_MW_A2DP_ROLE_SRC)                              \
        {                                                               \
            BT_DBG_ERROR(BT_DEBUG_A2DP, "invalid role(%d)", (role));    \
            return BT_ERR_STATUS_PARM_INVALID;                          \
        }                                                               \
    }while(0)

#define BT_MW_A2DP_LOCK() do{                           \
    if(g_bt_mw_a2dp_cb.inited)                          \
        pthread_mutex_lock(&g_bt_mw_a2dp_cb.lock);      \
    } while(0)

#define BT_MW_A2DP_UNLOCK() do{                         \
    if(g_bt_mw_a2dp_cb.inited)                          \
        pthread_mutex_unlock(&g_bt_mw_a2dp_cb.lock);    \
    } while(0)

#if 0
#define BT_MW_A2DP_WAIT_TIMEOUT(sig, timeout_ms)   do {                                    \
    struct timespec wait_time;                                                             \
    clock_gettime(CLOCK_MONOTONIC, &wait_time);                                            \
    wait_time.tv_nsec = (wait_time.tv_nsec + timeout_ms * 1000000L) % 1000000000L;         \
    wait_time.tv_sec += ((wait_time.tv_nsec + timeout_ms * 1000000L) / 1000000000L);       \
    pthread_cond_timedwait((sig), &g_bt_mw_a2dp_cb.lock, &wait_time);                      \
    } while(0)
#endif


#define BT_MW_A2DP_REPORT_EVENT(param) bt_mw_a2dp_notify_app(&param)


#define BT_MW_A2DP_CHECK_INITED(ret)    do {                \
        if (FALSE == g_bt_mw_a2dp_cb.inited)                \
        {                                                   \
            BT_DBG_ERROR(BT_DEBUG_A2DP, "a2dp not init");   \
            return ret;                                     \
        }                                                   \
    }while(0)


/* DATA TYPE DECLARATIONS
 */

typedef struct list_t device_list_t;

typedef struct
{
    UINT8 codec_type;
    INT32 sample_rate;
    INT32 channel_mode;
} BT_MW_A2DP_CODEC_CONFIG;

typedef struct
{
    UINT16 delay;
} BT_MW_A2DP_DELAY;

typedef struct
{
    CHAR   addr[MAX_BDADDR_LEN];
    UINT16 value;
    BOOL   in_use;
} BT_MW_A2DP_SINK_DELAY;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_MW_A2DP_STREAM_STATUS stream_status;
    BT_MW_A2DP_CONNECT_STATUS conn_status;
    BT_MW_A2DP_ROLE local_role;
    BT_MW_A2DP_CODEC_CONFIG codec_config;   // for sink
    BT_A2DP_CODEC_CONFIG curr_codec_info; // save cuur select codec info for src
    BT_MW_A2DP_DELAY delay;
    BT_A2DP_CONF config;
    BOOL in_use;
    BOOL is_coming; /* TRUE: connect from remote */
    BOOL is_silence_mode;
} BT_MW_A2DP_DEV;

typedef struct _BT_MW_A2DP_DEV_LIST_NODE
{
    struct _BT_MW_A2DP_DEV_LIST_NODE *next;
    BT_A2DP_CONNECTED_DEVICE dev;
} BT_MW_A2DP_DEV_LIST_NODE;

typedef struct
{
    BOOL role_enable[BT_MW_A2DP_ROLE_MAX];
    INT32 link_num[BT_MW_A2DP_ROLE_MAX];
    BT_A2DP_SRC_INIT_CONFIG src_pending_init_config;
} BT_MW_A2DP_CONF;

typedef struct
{
    BOOL power_on;
    BOOL is_snklp;
    UINT32 pending_flag; /* something is pending */
    BT_MW_A2DP_DEV_LIST_NODE *dev_list[BT_MW_A2DP_ROLE_MAX];
    INT32 dev_cnt[BT_MW_A2DP_ROLE_MAX];
} BT_MW_A2DP_DATA;

typedef struct
{
    BT_MW_A2DP_DEV devices[BT_MW_A2DP_MAX_DEVICES];
    BT_MW_A2DP_CONF config;
    BT_MW_A2DP_DATA data;

    /*Note: index 0 is for addr=NULL; others(1,2 ... MAX) is for addr != NULL */
    BT_MW_A2DP_SINK_DELAY sink_delay[BT_MW_A2DP_MAX_DEVICES];
    CHAR active_dev_addr[MAX_BDADDR_LEN];

    BOOL inited;
    pthread_mutex_t lock;
} BT_MW_A2DP_CB;

extern bool g_mw_start_record;


/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static INT32 bt_mw_a2dp_init(VOID);

static INT32 bt_mw_a2dp_deinit(VOID);

static VOID bt_mw_a2dp_init_profile(VOID);

static VOID bt_mw_a2dp_deinit_profile(VOID);

static VOID bt_mw_a2dp_notify(BTMW_GAP_EVT *gap_state);

static VOID bt_mw_a2dp_factory_reset(VOID);

static VOID bt_mw_a2dp_rm_dev(CHAR *addr);

static BOOL bt_mw_a2dp_has_connection(BT_MW_A2DP_ROLE local_role);

static INT32 bt_mw_a2dp_disconnect_all(BT_MW_A2DP_ROLE local_role);

static INT32 bt_mw_a2dp_load_dev_list(BT_MW_A2DP_ROLE local_role);

static INT32 bt_mw_a2dp_save_dev_list(BT_MW_A2DP_DEV_LIST_NODE *dev_list,
    BT_MW_A2DP_ROLE local_role);

static INT32 bt_mw_a2dp_release_dev_list(void);

static INT32 bt_mw_a2dp_get_dev_index(CHAR *addr);

static INT32 bt_mw_a2dp_alloc_dev_index(CHAR *addr, BT_MW_A2DP_ROLE local_role);

static INT32 bt_mw_a2dp_free_dev(INT32 index);

static INT32 bt_mw_a2dp_sink_delay_get_index(CHAR *addr);

static INT32 bt_mw_a2dp_sink_delay_free_dev(CHAR *addr);

static INT32 bt_mw_a2dp_sink_delay_alloc_index(CHAR *addr, UINT16 value);

static VOID bt_mw_a2dp_dump_dev_list(BT_MW_A2DP_DEV_LIST_NODE *dev_list);

static VOID bt_mw_a2dp_msg_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_a2dp_notify_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_a2dp_add_dev(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_get_devices(BT_A2DP_DEVICE_LIST *dev_list,
    BT_MW_A2DP_DEV_LIST_NODE *devices);

static INT32 bt_mw_a2dp_get_connected_devices(BT_A2DP_CONNECT_DEV_INFO_LIST *dev_list,
    BT_MW_A2DP_DEV *devices);

static INT32 bt_mw_a2dp_move_devices(BT_MW_A2DP_DEV_LIST_NODE **dev_list,
    CHAR *addr);
static INT32 bt_mw_a2dp_remove_devices(BT_MW_A2DP_DEV_LIST_NODE **dev_list);

static INT32 bt_mw_a2dp_check_pending(VOID);

static INT32 bt_mw_a2dp_connected_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_start_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_suspend_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_sink_config_cb(CHAR *addr, BT_MW_A2DP_ROLE role,
    UINT32 freq, UINT8 channel, UINT8 codec_type);

static INT32 bt_mw_a2dp_src_config_cb(CHAR *addr, BT_MW_A2DP_ROLE role,
        BT_A2DP_EVENT_AUDIO_CONFIG_DATA *p_event_audio_config_data);

static INT32 bt_mw_a2dp_disconnected_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_disconnecting_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static INT32 bt_mw_a2dp_connecting_cb(CHAR *addr, BT_MW_A2DP_ROLE role);

static VOID bt_mw_a2dp_codec_config_cb(CHAR *addr, BT_A2DP_CONF *current_config);

static VOID bt_mw_a2dp_delay_cb(CHAR *addr, BT_MW_A2DP_DELAY *delay);

static INT32 bt_mw_a2dp_enable(BOOL power_on);

static INT32 bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE local_role, BOOL enable,
        BT_A2DP_SRC_INIT_CONFIG* p_src_init_config);

static INT32 bt_mw_a2dp_player_report_event_cb(CHAR *addr, BT_A2DP_PLAYER_EVENT player_event);

#if ENABLE_A2DP_SRC
static INT32 bt_mw_a2dp_enable_src_stack(BOOL src_enable,
                           BT_A2DP_SRC_INIT_CONFIG *p_src_init_config);
#endif

#if ENABLE_A2DP_SINK
static INT32 bt_mw_a2dp_enable_sink_stack(BOOL sink_enable, INT32 sink_num);
#endif

#if ENABLE_A2DP_SRC || ENABLE_A2DP_SINK
static INT32 bt_mw_a2dp_enable_common_stack(BOOL power_on);
#endif

static VOID bt_mw_a2dp_notify_app(BT_A2DP_EVENT_PARAM *param);

static CHAR* bt_mw_a2dp_get_role_str(BT_MW_A2DP_ROLE role);
static CHAR* bt_mw_a2dp_get_event_str(UINT32 event);
static CHAR* bt_mw_a2dp_get_player_event_str(BT_A2DP_PLAYER_EVENT player_event);
static CHAR* bt_mw_a2dp_get_codec_str(UINT8 codec_tpye);
static CHAR* bt_mw_a2dp_get_stream_state_str(BT_MW_A2DP_STREAM_STATUS state);
static CHAR* bt_mw_a2dp_get_conn_state_str(BT_MW_A2DP_CONNECT_STATUS state);

/* STATIC VARIABLE DECLARATIONS
 */

static BT_MW_A2DP_CB g_bt_mw_a2dp_cb;

static BT_A2DP_EVENT_HANDLE_CB g_bt_mw_a2dp_event_handle_cb = NULL;

static CHAR *g_bt_mw_a2dp_files[BT_MW_A2DP_ROLE_MAX] = {
    BLUETOOTH_MW_LOCAL_FOLDER"/a2dp_src_list.txt",
    BLUETOOTH_MW_LOCAL_FOLDER"/a2dp_sink_list.txt"};

/* EXPORTED SUBPROGRAM BODIES
 */

INT32 bt_mw_a2dp_connect(CHAR *addr, BT_MW_A2DP_ROLE local_role)
{
    INT32 ret = BT_SUCCESS;
    INT32 idx = 0;
    BOOL is_connecting = FALSE;

    BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_IS_VALID_ROLE(local_role);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=(%s)%d", addr,
        bt_mw_a2dp_get_role_str(local_role), local_role);
    UINT32 role = (UINT32)local_role;

    BT_MW_A2DP_LOCK();
    is_connecting = bt_mw_a2dp_is_connecting();

    if ((FALSE == g_bt_mw_a2dp_cb.inited) ||
        (FALSE == g_bt_mw_a2dp_cb.data.power_on) ||
        (FALSE == g_bt_mw_a2dp_cb.config.role_enable[role]) ||
        (0 != g_bt_mw_a2dp_cb.data.pending_flag) ||
        (is_connecting == TRUE))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP,
            "a2dp is not ready, init=%d, power=%d, role_enable[%d]=%d, pending_flag=0x%x, connecting=%d",
            g_bt_mw_a2dp_cb.inited,
            g_bt_mw_a2dp_cb.data.power_on,
            role,
            g_bt_mw_a2dp_cb.config.role_enable[role],
            g_bt_mw_a2dp_cb.data.pending_flag,
            is_connecting);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    idx = bt_mw_a2dp_get_dev_index(addr);
    if ((idx >= 0) && (g_bt_mw_a2dp_cb.devices[idx].conn_status
        != BT_MW_A2DP_CONNECT_STATUS_DISCONNECTED))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "dev[%d] is not disconnected(%d)", idx,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_BUSY;
    }

    idx = bt_mw_a2dp_alloc_dev_index(addr, (BT_MW_A2DP_ROLE)role);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device resource to connect %s", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_FAIL;
    }
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "allocate dev %d", idx);
    g_bt_mw_a2dp_cb.devices[idx].conn_status =
        BT_MW_A2DP_CONNECT_STATUS_CONNECTING;

    g_bt_mw_a2dp_cb.devices[idx].local_role = (BT_MW_A2DP_ROLE)role;
    g_bt_mw_a2dp_cb.devices[idx].is_coming = FALSE;

    BT_DBG_INFO(BT_DEBUG_A2DP, "role is %s(%d)",
        bt_mw_a2dp_get_role_str(local_role), local_role);

#if ENABLE_A2DP_SINK
    if (BT_MW_A2DP_ROLE_SINK == role)
    {
        ret = linuxbt_a2dp_sink_connect(addr);
    }
#endif

#if ENABLE_A2DP_SRC
    if (BT_MW_A2DP_ROLE_SRC == role)
    {
        ret = linuxbt_a2dp_src_connect(addr);
    }
#endif

    if (BT_SUCCESS != ret)
    {
        bt_mw_a2dp_free_dev(idx);
        bt_mw_a2dp_sink_delay_free_dev(addr);
    }
    BT_MW_A2DP_UNLOCK();

    return ret;
}

INT32 bt_mw_a2dp_disconnect(CHAR *addr)
{
    INT32 idx = 0;
    BT_A2DP_ROLE local_role = 0;
    INT32 ret = BT_ERR_STATUS_FAIL;

    BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_SUCCESS;
    }

    if ((BT_MW_A2DP_CONNECT_STATUS_CONNECTING
        == g_bt_mw_a2dp_cb.devices[idx].conn_status) ||
        (BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING
        == g_bt_mw_a2dp_cb.devices[idx].conn_status))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is connecting/disconnecting(%d), please wait", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_BUSY;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_DISCONNECTED
        == g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_SUCCESS;
    }

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "%s conn state(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);

    local_role = g_bt_mw_a2dp_cb.devices[idx].local_role;
    g_bt_mw_a2dp_cb.devices[idx].conn_status =
        BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING;

    BT_DBG_INFO(BT_DEBUG_A2DP, "local_role is %s(%d)",
        bt_mw_a2dp_get_role_str(local_role), local_role);

#if ENABLE_A2DP_SINK
    if (BT_A2DP_ROLE_SINK == local_role)
    {
        ret = linuxbt_a2dp_sink_disconnect(addr);
    }
#endif

#if ENABLE_A2DP_SRC
    if (BT_A2DP_ROLE_SRC == local_role)
    {
        ret = linuxbt_a2dp_src_disconnect(addr);
    }
#endif
    BT_MW_A2DP_UNLOCK();
    if (BT_SUCCESS != ret)
    {
        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        BT_DBG_WARNING(BT_DEBUG_A2DP, "disconnect %s fail(%d), reset it", addr, ret);

        memset((void*)&btmw_msg, 0, sizeof(btmw_msg));
        strncpy(btmw_msg.data.avrcp_msg.addr, addr, MAX_BDADDR_LEN - 1);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_MAX;
        btmw_msg.hdr.event = BTMW_AVRCP_DISCONNECTED;
        linuxbt_send_msg(&btmw_msg);

        memset((void*)&btmw_msg, 0, sizeof(btmw_msg));
        strncpy(btmw_msg.data.a2dp_msg.addr, addr, MAX_BDADDR_LEN - 1);
        btmw_msg.data.a2dp_msg.role = local_role;
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTED;
        linuxbt_send_msg(&btmw_msg);
    }

    return ret;

} /*bt_mw_a2dp_disconnect*/

INT32 bt_mw_a2dp_src_set_channel_allocation_for_lrmode(CHAR *addr, INT32 channel)
{
    return linuxbt_a2dp_src_set_channel_allocation_for_lrmode(addr, channel);
}

INT32 bt_mw_a2dp_src_set_audiomode(INT32 AudioMode)
{
    return linuxbt_a2dp_src_set_audiomode(AudioMode);
}

INT32 bt_mw_a2dp_register_callback(BT_A2DP_EVENT_HANDLE_CB a2dp_handle)
{
    INT32 ret = BT_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "a2dp_handle=%p", a2dp_handle);
    if (NULL != a2dp_handle)
    {
        ret = bt_mw_a2dp_init();
    }
    else
    {
        ret = bt_mw_a2dp_deinit();
    }
    g_bt_mw_a2dp_event_handle_cb = a2dp_handle;
    return ret;
}

INT32 bt_mw_a2dp_change_thread_priority(INT32 priority)
{
    return linuxbt_a2dp_change_thread_priority_handler(priority);
}

INT32 bt_mw_a2dp_codec_enable(BT_A2DP_CODEC_TYPE codec_type, BOOL enable)
{
    INT32 ret = BT_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "codec_type=%d, enable=%d", codec_type, enable);
    if(g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SRC])
    {
        ret = linuxbt_a2dp_src_codec_enable_handler(codec_type, enable);
    }
    else if(g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SINK])
    {
        ret = linuxbt_a2dp_sink_codec_enable_handler(codec_type, enable);
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "src and sink both not enable");
        ret = BT_ERR_STATUS_NOT_READY;
    }

    return ret;
}

INT32 bt_mw_a2dp_sink_enable(BOOL enable)
{
    INT32 ret = BT_SUCCESS;
#if ENABLE_A2DP_SINK
    ret = bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SINK, enable, NULL);
#endif

    return ret;
}

INT32 bt_mw_a2dp_src_enable(BOOL enable, BT_A2DP_SRC_INIT_CONFIG* p_src_init_config)
{
    INT32 ret = BT_SUCCESS;

#if ENABLE_A2DP_SRC
    if (p_src_init_config == NULL ||
        ((p_src_init_config != NULL) && (p_src_init_config->codec_config_list.codec_config_num == 0)))
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "p_src_init_config is NULL or codec num is 0, set defaul config");
        BT_A2DP_SRC_INIT_CONFIG init_config;
        memset(&init_config, 0, sizeof(BT_A2DP_SRC_INIT_CONFIG));
        init_config.max_connected_audio_devices = 1;
        init_config.codec_config_list.codec_config_num =1;
        init_config.codec_config_list.codec_config_list[0].bits_per_sample =
        BT_A2DP_CODEC_BITS_PER_SAMPLE_16;
        init_config.codec_config_list.codec_config_list[0].channel_mode =
        BT_A2DP_CODEC_CHANNEL_MODE_STEREO;
        init_config.codec_config_list.codec_config_list[0].channel_num = 1;
        init_config.codec_config_list.codec_config_list[0].sample_rate =
        BT_A2DP_CODEC_SAMPLE_RATE_48000;
        init_config.codec_config_list.codec_config_list[0].codec_priority =
        BT_A2DP_CODEC_PRIORITY_DEFAULT;
        init_config.codec_config_list.codec_config_list[0].codec_type =
        BT_A2DP_CODEC_TYPE_SBC;
        init_config.max_connected_audio_devices = 1;
        g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SRC] = 1;

        ret = bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SRC, enable, &init_config);
        return ret;
    }
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "src_max_connect=%d", p_src_init_config->max_connected_audio_devices);
    if(p_src_init_config->max_connected_audio_devices < 1)
    {
        p_src_init_config->max_connected_audio_devices = 1;
    }
    g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SRC] =
                p_src_init_config->max_connected_audio_devices;

    ret = bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SRC, enable, p_src_init_config);
#endif
    return ret;
}

INT32 bt_mw_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    INT32 ret = BT_SUCCESS;
#if ENABLE_A2DP_SRC
    BT_MW_A2DP_LOCK();
    ret = bt_mw_a2dp_get_devices(dev_list, g_bt_mw_a2dp_cb.data.dev_list[BT_MW_A2DP_ROLE_SRC]);
    BT_MW_A2DP_UNLOCK();
#endif
    return ret;
}

INT32 bt_mw_a2dp_get_connected_dev_list(BT_A2DP_CONNECT_DEV_INFO_LIST *dev_list)
{
    INT32 ret = BT_SUCCESS;
#if ENABLE_A2DP_SRC
    BT_MW_A2DP_LOCK();
    ret = bt_mw_a2dp_get_connected_devices(dev_list, g_bt_mw_a2dp_cb.devices);
    BT_MW_A2DP_UNLOCK();
#endif
    return ret;
}


INT32 bt_mw_a2dp_sink_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    INT32 ret = BT_SUCCESS;
#if ENABLE_A2DP_SINK
    BT_MW_A2DP_LOCK();
    ret = bt_mw_a2dp_get_devices(dev_list, g_bt_mw_a2dp_cb.data.dev_list[BT_MW_A2DP_ROLE_SINK]);
    BT_MW_A2DP_UNLOCK();
#endif
    return ret;
}


INT32 bt_mw_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag, BT_A2DP_DBG_PARAM *param)
{
    switch(flag)
    {
        case BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV:
#if ENABLE_A2DP_SRC
            bt_mw_a2dp_src_set_audio_log_lv(param->hw_audio_log_lv);
#endif
            break;
        case BT_A2DP_DBG_SHOW_INFO:
            bt_mw_a2dp_dump_dev_info();
            bt_mw_avrcp_dump_info();
            break;
        case BT_A2DP_DBG_SET_PLAYER_WAIT_TIME_MS:
            //bt_mw_a2dp_sink_player_set(0, param->wait_time_ms);
            break;
        case BT_A2DP_DBG_SET_PLAYER_ZERO_TIME_MS:
            //bt_mw_a2dp_sink_player_set(1, param->zero_time_ms);
            break;
        case BT_A2DP_DBG_SET_PLAYER_START_TIME_MS:
            //bt_mw_a2dp_sink_player_set(2, param->zero_time_ms);
            break;
        default:
            break;
    }
    return BT_SUCCESS;
}

INT32 bt_mw_a2dp_sink_active_src(CHAR *addr)
{
    INT32 ret = BT_SUCCESS;
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};

#if ENABLE_A2DP_SINK
    BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (memcmp(addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1) == 0)
    {
        ret = linuxbt_a2dp_sink_active_src(addr);
        if (ret == BT_SUCCESS)
        {
            strncpy(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1);
            g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
            // notify to app
        }
        memset(&param, 0, sizeof(param));
        param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SINK;
        strncpy(param.addr, addr, MAX_BDADDR_LEN-1);
        param.addr[MAX_BDADDR_LEN-1] = '\0';

        param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
        BT_MW_A2DP_REPORT_EVENT(param);

        return 0;
    }

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED
        != g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not connected,status=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_ROLE_SINK
        != g_bt_mw_a2dp_cb.devices[idx].local_role)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not src, local_role(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    //bt_mw_a2dp_sink_select_player(addr, g_bt_mw_a2dp_cb.devices[idx].config.codec_type);

    ret = linuxbt_a2dp_sink_active_src(addr);
    if (ret == BT_SUCCESS)
    {
        strncpy(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1);
        g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
        // notify to app
    }
    if (g_bt_mw_a2dp_cb.devices[idx].local_role == BT_MW_A2DP_ROLE_SINK &&
        g_bt_mw_a2dp_cb.devices[idx].stream_status == BT_MW_A2DP_STREAM_STATUS_PLAYING)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s need set focus(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        linuxbt_a2dp_sink_set_audio_focus_state(1);
    }

    memset(&param, 0, sizeof(param));
    param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SINK;
    strncpy(param.addr, addr, MAX_BDADDR_LEN-1);
    param.addr[MAX_BDADDR_LEN-1] = '\0';

    param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
    BT_MW_A2DP_REPORT_EVENT(param);

    BT_MW_A2DP_UNLOCK();
#endif
    return ret;
}

INT32 bt_mw_a2dp_sink_set_delay_value(CHAR *addr, UINT16 value)
{
    INT32 ret = BT_SUCCESS;

#if ENABLE_A2DP_SINK
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    if ((addr == NULL) || ((MAX_BDADDR_LEN - 1) != strlen(addr)))
    {
        /*Note: index 0 is for addr=NULL; others(1,2 ... MAX) is for addr != NULL */
        g_bt_mw_a2dp_cb.sink_delay[0].in_use = TRUE;
        memset(g_bt_mw_a2dp_cb.sink_delay[0].addr, 0, MAX_BDADDR_LEN - 1);
        g_bt_mw_a2dp_cb.sink_delay[0].value = value;
        BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "invalid addr=%s, value=%d.",
                                addr==NULL?"NULL":addr, value);
    }
    else
    {
        bt_mw_a2dp_sink_delay_alloc_index(addr, value);
        BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, value=%d.", addr, value);
    }

    ret = linuxbt_a2dp_sink_set_delay_value(addr, value);
#endif
    return ret;
}

INT32 bt_mw_a2dp_sink_lowpower_enable(BOOL enable)
{
    INT32 ret = BT_SUCCESS;
    linuxbt_a2dp_sink_lowpower_enable_handler(enable);
    g_bt_mw_a2dp_cb.data.is_snklp = enable;
    return ret;
}

INT32 bt_mw_a2dp_src_active_sink(CHAR *addr)
{
    INT32 ret = BT_SUCCESS;
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};

#if ENABLE_A2DP_SRC
    //BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED
        != g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not connected, status=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_ROLE_SRC
        != g_bt_mw_a2dp_cb.devices[idx].local_role)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not src, local_role(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }
    if (addr != NULL)
    {
        ret = linuxbt_a2dp_src_active_sink(addr);
        if (ret == BT_SUCCESS)
        {
            strncpy(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1);
            g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
            // notify to app
        }
        memset(&param, 0, sizeof(param));
        param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SRC;
        strncpy(param.addr, addr, MAX_BDADDR_LEN-1);
        param.addr[MAX_BDADDR_LEN-1] = '\0';

        param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
        BT_MW_A2DP_REPORT_EVENT(param);

        BT_MW_A2DP_UNLOCK();
        // if set a dev as  active dev, if in silence mode,need exit silence mode
        if(g_bt_mw_a2dp_cb.devices[idx].is_silence_mode)
        {
            bt_mw_a2dp_src_set_silence_device(addr, FALSE);
        }
    }
    else
    {
        if (memcmp(g_bt_mw_a2dp_cb.active_dev_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1) != 0)
        {
            ret = linuxbt_a2dp_src_active_sink("00:00:00:00:00:00");
            if (ret == BT_SUCCESS)
            {
                strncpy(g_bt_mw_a2dp_cb.active_dev_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1);
                g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
                // notify to app
            }
            memset(&param, 0, sizeof(param));
            strncpy(param.addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1);
            param.addr[MAX_BDADDR_LEN-1] = '\0';
            param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SRC;
            param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
            BT_MW_A2DP_REPORT_EVENT(param);

            BT_MW_A2DP_UNLOCK();
        }
    }

#endif
    return ret;
}

INT32 bt_mw_a2dp_get_active_device(CHAR *addr)
{
    INT32 ret = BT_SUCCESS;
    strncpy(addr, g_bt_mw_a2dp_cb.active_dev_addr, MAX_BDADDR_LEN-1);
    addr[MAX_BDADDR_LEN-1] = '\0';
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "addr=%s", addr);
    return ret;
}

INT32 bt_mw_a2dp_src_set_silence_device(CHAR *addr, BOOL enable)
{
    INT32 ret = BT_SUCCESS;
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};

#if ENABLE_A2DP_SRC
    //BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED
        != g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not connected,status=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_ROLE_SRC
        != g_bt_mw_a2dp_cb.devices[idx].local_role)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not src, local_role(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    // if curr device is not acitive device
    if (memcmp(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1) != 0)
    {
        if (enable)
        {
            ret = linuxbt_a2dp_src_set_silence_device(addr, enable);
            if (ret == BT_SUCCESS)
            {
                g_bt_mw_a2dp_cb.devices[idx].is_silence_mode = TRUE;
            }
        }
        else
        {
            ret = linuxbt_a2dp_src_set_silence_device(addr, enable);
            if (ret == BT_SUCCESS)
            {
                g_bt_mw_a2dp_cb.devices[idx].is_silence_mode = FALSE;
            }
            if (memcmp(g_bt_mw_a2dp_cb.active_dev_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1) == 0)
            {
                // set curr device as active device
                BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] set active device %s", addr);
                ret = linuxbt_a2dp_src_active_sink(addr);
                if (ret == BT_SUCCESS)
                {
                    strncpy(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1);
                    g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
                    // notify to app
                    memset(&param, 0, sizeof(param));
                    param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SRC;
                    strncpy(param.addr, addr, MAX_BDADDR_LEN-1);
                    param.addr[MAX_BDADDR_LEN-1] = '\0';

                    param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
                    BT_MW_A2DP_REPORT_EVENT(param);

                    BT_MW_A2DP_UNLOCK();
                    return ret;
                }
            }
        }
    }
    // if curr device is active device
    else
    {
        if (enable)
        {
            ret = linuxbt_a2dp_src_set_silence_device(addr, enable);
            if (ret == BT_SUCCESS)
            {
                g_bt_mw_a2dp_cb.devices[idx].is_silence_mode = TRUE;
            }
            ret = linuxbt_a2dp_src_active_sink("00:00:00:00:00:00");
            if (ret == BT_SUCCESS)
            {
                strncpy(g_bt_mw_a2dp_cb.active_dev_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1);
                g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
                // notify to app
                memset(&param, 0, sizeof(param));
                strncpy(param.addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1);
                param.addr[MAX_BDADDR_LEN-1] = '\0';
                param.data.active_changed.local_role = BT_MW_A2DP_ROLE_SRC;
                param.event = BT_A2DP_EVENT_ACTIVE_CHANGED;
                BT_MW_A2DP_REPORT_EVENT(param);

                BT_MW_A2DP_UNLOCK();
                return ret;
            }
        }
        else
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is active device, cannot exit silence", addr);
        }
    }
    BT_MW_A2DP_UNLOCK();
#endif
    return ret;
}

BOOL bt_mw_a2dp_src_is_in_silence_mode(CHAR *addr)
{
    INT32 idx = 0;
    BOOL is_silence_mode = FALSE;

    #if ENABLE_A2DP_SRC
    //BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return FALSE;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED
        != g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not connected status=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return FALSE;
    }

    if (BT_MW_A2DP_ROLE_SRC
        != g_bt_mw_a2dp_cb.devices[idx].local_role)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not src, local_role(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        BT_MW_A2DP_UNLOCK();
        return FALSE;
    }
    is_silence_mode = g_bt_mw_a2dp_cb.devices[idx].is_silence_mode;
    BT_MW_A2DP_UNLOCK();
  #endif
    return is_silence_mode;
}

INT32 bt_mw_a2dp_src_config_codec_info(CHAR *addr, BT_A2DP_SET_CODEC_CONFIG *p_src_set_codec_config)
{
    INT32 ret = BT_SUCCESS;
    INT32 idx = 0;
    BT_A2DP_CODEC_CONFIG codec_config;

    memset(&codec_config, 0, sizeof(BT_A2DP_CODEC_CONFIG));

#if ENABLE_A2DP_SRC
    //BT_MW_A2DP_IS_VALID_ADDR(addr);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is disconnected", addr);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED
        != g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not connected,status=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    if (BT_MW_A2DP_ROLE_SRC
        != g_bt_mw_a2dp_cb.devices[idx].local_role)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s is not src, local_role(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role);
        BT_MW_A2DP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }

    memcpy(&codec_config, &g_bt_mw_a2dp_cb.devices[idx].curr_codec_info, sizeof(BT_A2DP_CODEC_CONFIG));
    BT_DBG_ERROR(BT_DEBUG_A2DP, "save codec_config.codec_type(%d)", codec_config.codec_type);

    if(p_src_set_codec_config->bit_mask & BT_A2DP_CODEC_CONFIG_CODEC_TYPE)
    {
        codec_config.codec_type = p_src_set_codec_config->codec_type;
        BT_DBG_ERROR(BT_DEBUG_A2DP, "new set codec_config.codec_type(%d)", codec_config.codec_type);
    }
    if (p_src_set_codec_config->bit_mask & BT_A2DP_CODEC_CONFIG_CODEC_PRIORITY)
    {
        codec_config.codec_priority = p_src_set_codec_config->codec_priority;
    }
    if(p_src_set_codec_config->bit_mask & BT_A2DP_CODEC_CONFIG_CH_MODE)
    {
        codec_config.channel_mode = p_src_set_codec_config->ch_mode;
        BT_DBG_ERROR(BT_DEBUG_A2DP, "new set codec_config.channel_mode(%d)", codec_config.channel_mode);
    }
    if(p_src_set_codec_config->bit_mask & BT_A2DP_CODEC_CONFIG_SAMPLE_RATE)
    {
        codec_config.sample_rate= p_src_set_codec_config->sample_rate;
    }
    if(p_src_set_codec_config->bit_mask & BT_A2DP_CODEC_CONFIG_BITS_PER_SAMPLE)
    {
        codec_config.bits_per_sample = p_src_set_codec_config->bits_per_sample;
    }

    ret = linuxbt_a2dp_src_config_codec_info(addr, &codec_config);

    BT_MW_A2DP_UNLOCK();
#endif
    return ret;

}


INT32 bt_mw_a2dp_set_link_num(INT32 sink_num)
{
    INT32 ret = BT_SUCCESS;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "sink_link_num=%d", sink_num);

    if (sink_num > BT_MW_A2DP_MAX_DEVICES)
    {
        sink_num = BT_MW_A2DP_MAX_DEVICES;
    }

    BT_MW_A2DP_LOCK();
    g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SINK] = sink_num;
    BT_MW_A2DP_UNLOCK();
    return ret;
}

INT32 bt_mw_a2dp_dump_dev_info(VOID)
{
    INT32 dev_idx = 0;
    INT32 sink_delay_idx = 0;
    BT_MW_A2DP_DEV *ptr_dev = NULL;
    BT_MW_A2DP_SINK_DELAY *ptr_delay_dev = NULL;
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    bt_mw_dump_info_begin("bt_a2dp_dump.log");

    BT_MW_A2DP_LOCK();
    BT_DBG_DUMP(BT_DEBUG_A2DP, "==========config==========");

    BT_DBG_DUMP(BT_DEBUG_A2DP, "src %s",
        g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SRC]?"enable":"disable");
    BT_DBG_DUMP(BT_DEBUG_A2DP, "sink %s",
        g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SINK]?"enable":"disable");
    BT_DBG_DUMP(BT_DEBUG_A2DP, "src_num %d",
        g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SRC]);
    BT_DBG_DUMP(BT_DEBUG_A2DP, "sink_num %d",
        g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SINK]);

    for(dev_idx=0;dev_idx<BT_MW_A2DP_MAX_DEVICES;dev_idx++)
    {
        ptr_dev = &g_bt_mw_a2dp_cb.devices[dev_idx];
        if (TRUE == ptr_dev->in_use)
        {
            BT_DBG_DUMP(BT_DEBUG_A2DP, "==========device[%d]==========", dev_idx);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "addr:%s", ptr_dev->addr);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "stream_status: %s",
                bt_mw_a2dp_get_stream_state_str(ptr_dev->stream_status));
            BT_DBG_DUMP(BT_DEBUG_A2DP, "conn_status: %s",
                bt_mw_a2dp_get_conn_state_str(ptr_dev->conn_status));
            BT_DBG_DUMP(BT_DEBUG_A2DP, "local_role: %s",
                bt_mw_a2dp_get_role_str(ptr_dev->local_role));
            BT_DBG_DUMP(BT_DEBUG_A2DP, "sample_rate: %d",
                ptr_dev->codec_config.sample_rate);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "channel: %d",
                ptr_dev->codec_config.channel_mode);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "bitdepth: %d",
                ptr_dev->config.codec_conf.vendor_conf.conf.lhdc_conf.bit_depth==0?16:
                ptr_dev->config.codec_conf.vendor_conf.conf.lhdc_conf.bit_depth);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "codec: %s(%d)",
                bt_mw_a2dp_get_codec_str(ptr_dev->config.codec_type),
                ptr_dev->config.codec_type);
            BT_DBG_DUMP(BT_DEBUG_A2DP, "codec: %s(%d)",
                bt_mw_a2dp_get_codec_str(ptr_dev->config.codec_type),
                ptr_dev->config.codec_type);

            if (ptr_dev->local_role == BT_MW_A2DP_ROLE_SRC)
                BT_DBG_DUMP(BT_DEBUG_A2DP, "remote delay(0.1 ms): %d", ptr_dev->delay.delay);

            if (ptr_dev->local_role == BT_MW_A2DP_ROLE_SINK)
            {
                sink_delay_idx = bt_mw_a2dp_sink_delay_get_index(ptr_dev->addr);
                if (sink_delay_idx < 0)
                {
                    BT_DBG_DUMP(BT_DEBUG_A2DP, "local delay(0.1 ms): %d",
                    g_bt_mw_a2dp_cb.sink_delay[0].value);
                }
                else
                {
                    BT_DBG_DUMP(BT_DEBUG_A2DP, "local delay(0.1 ms): %d",
                    g_bt_mw_a2dp_cb.sink_delay[sink_delay_idx].value);
                }
            }
        }
    }

    for(dev_idx=0; dev_idx<BT_MW_A2DP_MAX_DEVICES; dev_idx++)
    {
        ptr_delay_dev = &g_bt_mw_a2dp_cb.sink_delay[dev_idx];

        if ((dev_idx == 0) && (TRUE == ptr_delay_dev->in_use))
        {
            BT_DBG_MINOR(BT_DEBUG_A2DP, "==========sink delay device[%d]==========", dev_idx);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "addr NULL");
            BT_DBG_MINOR(BT_DEBUG_A2DP, "delay value(0.1 ms): %d", ptr_delay_dev->value);
            continue;
        }

        if (TRUE == ptr_delay_dev->in_use)
        {
            BT_DBG_MINOR(BT_DEBUG_A2DP, "==========sink delay device[%d]==========", dev_idx);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "addr:%s", ptr_delay_dev->addr);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "delay value(0.1 ms): %d", ptr_delay_dev->value);
        }
    }

    BT_MW_A2DP_UNLOCK();
    bt_mw_dump_info_end();

    return BT_SUCCESS;
}



/* LOCAL SUBPROGRAM BODIES
 */


static INT32 bt_mw_a2dp_init(VOID)
{
    profile_operator_t a2dp_op= {0};
    pthread_mutexattr_t attr;
    if (TRUE == g_bt_mw_a2dp_cb.inited)
    {
        return BT_SUCCESS;
    }

    memset((void*)&g_bt_mw_a2dp_cb, 0, sizeof(g_bt_mw_a2dp_cb));

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&g_bt_mw_a2dp_cb.lock, &attr);

    BT_MW_A2DP_LOCK();
#if ENABLE_A2DP_SINK
    bt_mw_a2dp_load_dev_list(BT_MW_A2DP_ROLE_SINK);
#endif
#if ENABLE_A2DP_SRC
    bt_mw_a2dp_load_dev_list(BT_MW_A2DP_ROLE_SRC);
#endif
    BT_MW_A2DP_UNLOCK();

    linuxbt_hdl_register(BTWM_ID_A2DP, bt_mw_a2dp_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_A2DP, bt_mw_a2dp_notify_handle);

    a2dp_op.init = bt_mw_a2dp_init_profile;
    a2dp_op.deinit = bt_mw_a2dp_deinit_profile;
    a2dp_op.notify_acl_state = bt_mw_a2dp_notify;
    a2dp_op.facotry_reset = bt_mw_a2dp_factory_reset;
    a2dp_op.rm_dev = bt_mw_a2dp_rm_dev;

    g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SRC] = 1;
    g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SINK] = 1;

    g_bt_mw_a2dp_cb.inited = TRUE;

    bt_mw_gap_register_profile(BTWM_ID_A2DP, &a2dp_op);

    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_deinit(VOID)
{
    profile_operator_t a2dp_op = {0};

    if (FALSE == g_bt_mw_a2dp_cb.inited)
    {
        return BT_SUCCESS;
    }

    BT_MW_A2DP_LOCK();
    if (BT_SUCCESS != bt_mw_a2dp_enable(FALSE))
    {
        g_bt_mw_a2dp_cb.data.pending_flag |=
            (1 << BT_MW_A2DP_PENDING_DEINIT);
        BT_MW_A2DP_UNLOCK();
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "there is connection, pending 0x%x",
            g_bt_mw_a2dp_cb.data.pending_flag);
        return BT_ERR_STATUS_BUSY;
    }
    bt_mw_a2dp_release_dev_list();

    BT_MW_A2DP_UNLOCK();

    pthread_mutex_destroy(&g_bt_mw_a2dp_cb.lock);

    memset((void*)&g_bt_mw_a2dp_cb, 0, sizeof(g_bt_mw_a2dp_cb));

    a2dp_op.init = NULL;
    a2dp_op.deinit = NULL;
    a2dp_op.notify_acl_state = NULL;
    a2dp_op.facotry_reset = NULL;

    bt_mw_gap_register_profile(BTWM_ID_A2DP, &a2dp_op);

    linuxbt_hdl_register(BTWM_ID_A2DP, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_A2DP, NULL);

    return BT_SUCCESS;
}

static VOID bt_mw_a2dp_init_profile(VOID)
{
    bt_mw_a2dp_enable(TRUE);
}

static VOID bt_mw_a2dp_deinit_profile(VOID)
{
    bt_mw_a2dp_enable(FALSE);
}

static VOID bt_mw_a2dp_notify(BTMW_GAP_EVT *gap_state)
{
    //tBTMW_MSG btmw_msg = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s state=%d", gap_state->bd_addr,
        gap_state->state);
#if 0
    if (GAP_STATE_ACL_DISCONNECTED == gap_state->state)
    {
        memset((void*)&btmw_msg, 0, sizeof(btmw_msg));
        strcpy(btmw_msg.data.avrcp_msg.addr, gap_state->bd_addr);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_MAX;
        btmw_msg.hdr.event = BTMW_AVRCP_DISCONNECTED;
        linuxbt_send_msg(&btmw_msg);

        memset((void*)&btmw_msg, 0, sizeof(btmw_msg));
        strcpy(btmw_msg.data.a2dp_msg.addr, gap_state->bd_addr);
        btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_MAX;
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTED;
        linuxbt_send_msg(&btmw_msg);
    }
#endif
}

static VOID bt_mw_a2dp_factory_reset(VOID)
{
    INT32 i = BT_MW_A2DP_ROLE_SRC;
    BT_MW_A2DP_LOCK();
    bt_mw_a2dp_release_dev_list();
    for (i=BT_MW_A2DP_ROLE_SRC;i<BT_MW_A2DP_ROLE_MAX;i++)
    {
        (void)unlink(g_bt_mw_a2dp_files[i]);
    }
    BT_MW_A2DP_UNLOCK();
}

static VOID bt_mw_a2dp_rm_dev(CHAR *addr)
{
    BT_MW_A2DP_DEV_LIST_NODE **dev_list = NULL;
    INT32 *dev_cnt = 0;
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    INT32 ret = BT_SUCCESS;
    BT_MW_A2DP_ROLE local_role = BT_MW_A2DP_ROLE_SRC;

    BT_MW_A2DP_LOCK();
    for (local_role = BT_MW_A2DP_ROLE_SRC;local_role<BT_MW_A2DP_ROLE_MAX;local_role++)
    {
        dev_list = &g_bt_mw_a2dp_cb.data.dev_list[local_role];
        dev_cnt = &g_bt_mw_a2dp_cb.data.dev_cnt[local_role];

        ret = bt_mw_a2dp_move_devices(dev_list, addr);
        if (ret == BT_ERR_STATUS_DONE || ret == BT_SUCCESS)
        {
            dev = *dev_list;
            if (dev != NULL)
            {
                *dev_list = dev->next;
                free(dev);
                *dev_cnt = *dev_cnt - 1;
                bt_mw_a2dp_save_dev_list(*dev_list, local_role);
            }

            bt_mw_a2dp_dump_dev_list(*dev_list);

            continue; /* maybe it's source and sink */
        }
        else
        {
            continue;
        }
    }
    BT_MW_A2DP_UNLOCK();

    return;
}


static BOOL bt_mw_a2dp_has_connection(BT_MW_A2DP_ROLE local_role)
{
    INT32 idx = 0;

    for (idx=0;idx < BT_MW_A2DP_MAX_DEVICES;idx++)
    {
        if (g_bt_mw_a2dp_cb.devices[idx].in_use == TRUE
            && ((local_role == g_bt_mw_a2dp_cb.devices[idx].local_role)
                || (BT_MW_A2DP_ROLE_MAX == local_role)))
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "devices[%d] is is use", idx);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL bt_mw_a2dp_is_connecting(void)
{
#if defined(MTK_A2DP_MULTI_AVRCP) && (MTK_A2DP_MULTI_AVRCP == TRUE)
    return FALSE;
#else
    INT32 idx = 0;

    for (idx=0;idx < BT_MW_A2DP_MAX_DEVICES;idx++)
    {
        if (g_bt_mw_a2dp_cb.devices[idx].in_use == TRUE
            && (BT_MW_A2DP_CONNECT_STATUS_CONNECTING
                == g_bt_mw_a2dp_cb.devices[idx].conn_status))
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "devices[%d] is connecting", idx);
            return TRUE;
        }
    }
    return FALSE;
#endif
}


static INT32 bt_mw_a2dp_disconnect_all(BT_MW_A2DP_ROLE local_role)
{
    INT32 idx = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "local_role=%s",
        bt_mw_a2dp_get_role_str(local_role));

    for (idx = 0;idx < BT_MW_A2DP_MAX_DEVICES;idx++)
    {
        if ((g_bt_mw_a2dp_cb.devices[idx].in_use == TRUE) &&
            (BT_MW_A2DP_CONNECT_STATUS_CONNECTED ==
                g_bt_mw_a2dp_cb.devices[idx].conn_status) &&
            ((local_role == g_bt_mw_a2dp_cb.devices[idx].local_role)
                || (BT_MW_A2DP_ROLE_MAX == local_role)))
        {
            g_bt_mw_a2dp_cb.devices[idx].conn_status =
                BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING;
#if ENABLE_A2DP_SINK
            if (BT_MW_A2DP_ROLE_SINK == g_bt_mw_a2dp_cb.devices[idx].local_role)
            {
                linuxbt_a2dp_sink_disconnect(g_bt_mw_a2dp_cb.devices[idx].addr);
            }
#endif

#if ENABLE_A2DP_SRC
            if (BT_MW_A2DP_ROLE_SRC == g_bt_mw_a2dp_cb.devices[idx].local_role)
            {
                linuxbt_a2dp_src_disconnect(g_bt_mw_a2dp_cb.devices[idx].addr);
            }
#endif
        }
    }
    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_load_dev_list(BT_MW_A2DP_ROLE local_role)
{
    FILE *fp_dev_list = NULL;
    int err = 0;
    UINT32 role = 0;
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    BT_MW_A2DP_DEV_LIST_NODE device;

    BT_MW_A2DP_DEV_LIST_NODE *last = NULL;
    BT_MW_A2DP_IS_VALID_ROLE(local_role);
    role = (UINT32)local_role;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "role=%s", bt_mw_a2dp_get_role_str(role));

    fp_dev_list = fopen(g_bt_mw_a2dp_files[role], "r");
    if (NULL == fp_dev_list)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "Failed to Read %s(%s)",
            g_bt_mw_a2dp_files[role], strerror(errno));
        err = remove(g_bt_mw_a2dp_files[role]);
        if (err < 0)
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "remove fail %s", strerror(errno));
        }
        goto __READ_FAIL;
    }

    last = g_bt_mw_a2dp_cb.data.dev_list[role];

    while(sizeof(BT_MW_A2DP_DEV_LIST_NODE) ==
        fread(&device, 1, sizeof(BT_MW_A2DP_DEV_LIST_NODE), fp_dev_list))
    {
        dev = malloc(sizeof(BT_MW_A2DP_DEV_LIST_NODE));
        if (NULL == dev)
        {
            (void)fclose(fp_dev_list);
            return BT_SUCCESS;
        }
        memcpy(dev, &device, sizeof(device));

        if (NULL == g_bt_mw_a2dp_cb.data.dev_list[role])
        {
            g_bt_mw_a2dp_cb.data.dev_list[role] = dev;
        }
        else
        {
            last->next = dev;
        }
        dev->next = NULL;
        last = dev;
        //BT_DBG_NORMAL(BT_DEBUG_A2DP, "[%d]dev(%s), role(%d), name(%s)",
        //    g_bt_mw_a2dp_cb.data.dev_cnt[role],
        //    device.dev.addr, device.dev.role, device.dev.name);
        g_bt_mw_a2dp_cb.data.dev_cnt[role]++;
    }

    (void)fclose(fp_dev_list);

    bt_mw_a2dp_dump_dev_list(g_bt_mw_a2dp_cb.data.dev_list[role]);
    return BT_SUCCESS;

__READ_FAIL:
    g_bt_mw_a2dp_cb.data.dev_list[role] = NULL;
    g_bt_mw_a2dp_cb.data.dev_cnt[role] = 0;
    return BT_ERR_STATUS_FAIL;
}

static INT32 bt_mw_a2dp_save_dev_list(BT_MW_A2DP_DEV_LIST_NODE *dev_list,
    BT_MW_A2DP_ROLE local_role)
{
    INT32 ret = 0;
    UINT32 role = 0;
    FILE *fp_dev_list = NULL;
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    BT_MW_A2DP_IS_VALID_ROLE(local_role);
    role = (UINT32)local_role;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "role=%d", role);

    fp_dev_list = fopen(g_bt_mw_a2dp_files[role], "w");
    if (NULL == fp_dev_list)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "Failed to write %s(%s)",
            g_bt_mw_a2dp_files[role], strerror(errno));
        goto __SAVE_FAIL;
    }

    dev = g_bt_mw_a2dp_cb.data.dev_list[role];

    while(dev != NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "save %s", dev->dev.addr);
        ret = fwrite(dev, 1, sizeof(BT_MW_A2DP_DEV_LIST_NODE), fp_dev_list);
        if (ret <= 0)
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "fwrite error(%s)", strerror(errno));
            (void)fclose(fp_dev_list);
            return BT_ERR_STATUS_FAIL;
        }
        (void)fflush(fp_dev_list);

        dev = dev->next;
    }
    fsync(fileno(fp_dev_list));

    (void)fclose(fp_dev_list);

    return BT_SUCCESS;

__SAVE_FAIL:
    return BT_ERR_STATUS_FAIL;
}



static INT32 bt_mw_a2dp_release_dev_list(void)
{
    BT_MW_A2DP_DEV_LIST_NODE *dev_list = NULL;
    BT_MW_A2DP_DEV_LIST_NODE *tmp = NULL;
    INT32 i = BT_MW_A2DP_ROLE_SRC;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    for (i=BT_MW_A2DP_ROLE_SRC;i<BT_MW_A2DP_ROLE_MAX;i++)
    {
        dev_list = g_bt_mw_a2dp_cb.data.dev_list[i];
        while(NULL != dev_list)
        {
            tmp = dev_list;
            dev_list = dev_list->next;
            free(tmp);
        }
        g_bt_mw_a2dp_cb.data.dev_list[i] = NULL;
    }

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_get_dev_index(CHAR *addr)
{
    int i = 0;
    BT_MW_A2DP_IS_VALID_ADDR(addr);

    for(i = 0;i < BT_MW_A2DP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_a2dp_cb.devices[i].addr, g_bt_mw_a2dp_cb.devices[i].in_use);
        if (TRUE == g_bt_mw_a2dp_cb.devices[i].in_use
            && 0 == strcasecmp(g_bt_mw_a2dp_cb.devices[i].addr, addr))
        {
            BT_DBG_INFO(BT_DEBUG_A2DP, "found at %d", i);
            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "%s not found", addr);
    return -1;
}

static BOOL bt_mw_a2dp_get_link_cnt_by_role(BT_MW_A2DP_ROLE local_role)
{
    INT32 i = 0, count = 0;

    for(i = 0;i < BT_MW_A2DP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "device[%d] addr=%s, in_use=%d, role=%s",
            i, g_bt_mw_a2dp_cb.devices[i].addr,
            g_bt_mw_a2dp_cb.devices[i].in_use,
            bt_mw_a2dp_get_role_str(g_bt_mw_a2dp_cb.devices[i].local_role));
        if ((TRUE == g_bt_mw_a2dp_cb.devices[i].in_use)
            && (local_role == g_bt_mw_a2dp_cb.devices[i].local_role))
        {
            count ++;
        }
    }

    return count;
}

static INT32 bt_mw_a2dp_alloc_dev_index(CHAR *addr, BT_MW_A2DP_ROLE local_role)
{
    INT32 i = 0, count = 0, other_count = 0;
    BT_MW_A2DP_ROLE other_role =
        (local_role==BT_MW_A2DP_ROLE_SRC)?BT_MW_A2DP_ROLE_SINK:BT_MW_A2DP_ROLE_SRC;

    count = bt_mw_a2dp_get_link_cnt_by_role(local_role);
    other_count = bt_mw_a2dp_get_link_cnt_by_role(other_role);
    if (count >= g_bt_mw_a2dp_cb.config.link_num[local_role] ||
        other_count > 0)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "%s conn is full(%d) or %s conn(%d) exist",
            bt_mw_a2dp_get_role_str(local_role), count,
            bt_mw_a2dp_get_role_str(other_role), other_count);

        return -1;
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "%s conn %d, other role conn(%d)",
        bt_mw_a2dp_get_role_str(local_role), count, other_count);

    for(i = 0;i < BT_MW_A2DP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_a2dp_cb.devices[i].addr, g_bt_mw_a2dp_cb.devices[i].in_use);
        if (FALSE == g_bt_mw_a2dp_cb.devices[i].in_use)
        {
            BT_DBG_INFO(BT_DEBUG_A2DP, "found free at %d", i);
            g_bt_mw_a2dp_cb.devices[i].in_use = TRUE;
            strncpy(g_bt_mw_a2dp_cb.devices[i].addr, addr, MAX_BDADDR_LEN - 1);
            g_bt_mw_a2dp_cb.devices[i].addr[MAX_BDADDR_LEN - 1] = '\0';

            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "not free found");
    return -1;
}

static INT32 bt_mw_a2dp_free_dev(INT32 index)
{
    if (index >= BT_MW_A2DP_MAX_DEVICES)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "invalid index %d", index);
        return -1;
    }

    memset((void*)&g_bt_mw_a2dp_cb.devices[index], 0, sizeof(BT_MW_A2DP_DEV));

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "free dev[%d]", index);
    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_sink_delay_alloc_index(CHAR *addr, UINT16 value)
{
    INT32 i = 1;
    BT_MW_A2DP_IS_VALID_ADDR(addr);

    for(i = 1; i < BT_MW_A2DP_MAX_DEVICES; i++)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_a2dp_cb.sink_delay[i].addr, g_bt_mw_a2dp_cb.sink_delay[i].in_use);
        if (FALSE == g_bt_mw_a2dp_cb.sink_delay[i].in_use)
        {
            BT_DBG_INFO(BT_DEBUG_A2DP, "found free at %d", i);
            g_bt_mw_a2dp_cb.sink_delay[i].in_use = TRUE;
            strncpy(g_bt_mw_a2dp_cb.sink_delay[i].addr, addr, MAX_BDADDR_LEN - 1);
            g_bt_mw_a2dp_cb.sink_delay[i].addr[MAX_BDADDR_LEN - 1] = '\0';

            g_bt_mw_a2dp_cb.sink_delay[i].value = value;

            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "not free found");
    return -1;
}

static INT32 bt_mw_a2dp_sink_delay_get_index(CHAR *addr)
{
    int i = 1;
    BT_MW_A2DP_IS_VALID_ADDR(addr);

    for(i = 1; i < BT_MW_A2DP_MAX_DEVICES; i++)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_a2dp_cb.sink_delay[i].addr, g_bt_mw_a2dp_cb.sink_delay[i].in_use);
        if (TRUE == g_bt_mw_a2dp_cb.sink_delay[i].in_use
            && 0 == strcasecmp(g_bt_mw_a2dp_cb.sink_delay[i].addr, addr))
        {
            BT_DBG_INFO(BT_DEBUG_A2DP, "found at %d", i);
            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "%s not found", addr);
    return -1;
}


static INT32 bt_mw_a2dp_sink_delay_free_dev(CHAR *addr)
{
    int index = 0;
    BT_MW_A2DP_IS_VALID_ADDR(addr);

    index = bt_mw_a2dp_sink_delay_get_index(addr);

    if (index == -1)
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "%s not found", addr);
        return -1;
    }
    if (index >= 0 && index < BT_MW_A2DP_MAX_DEVICES)
    {
        memset((void*)&g_bt_mw_a2dp_cb.sink_delay[index], 0, sizeof(BT_MW_A2DP_SINK_DELAY));
    }

    BT_DBG_INFO(BT_DEBUG_A2DP, "free dev[%d]", index);
    return BT_SUCCESS;
}

static VOID bt_mw_a2dp_msg_handle(tBTMW_MSG *p_msg)
{
    tBT_MW_A2DP_MSG *a2dp_msg = &p_msg->data.a2dp_msg;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "event:%s(%d), addr:%s",
        bt_mw_a2dp_get_event_str(p_msg->hdr.event),
        p_msg->hdr.event, a2dp_msg->addr);
    BT_MW_A2DP_CHECK_INITED();

    switch(p_msg->hdr.event)
    {
        case BTMW_A2DP_STREAM_START:
            bt_mw_a2dp_start_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_STREAM_SUSPEND:
            bt_mw_a2dp_suspend_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_STREAM_CONFIG:
            if (a2dp_msg->role == BT_MW_A2DP_ROLE_SRC)
            {
                bt_mw_a2dp_src_config_cb(a2dp_msg->addr, a2dp_msg->role, &a2dp_msg->data.src_codec_config);
            }
            else
            {
                bt_mw_a2dp_sink_config_cb(a2dp_msg->addr, a2dp_msg->role,
                a2dp_msg->data.sink_codec_config.sample_rate,
                a2dp_msg->data.sink_codec_config.channel_num,
                a2dp_msg->data.sink_codec_config.codec_type);
            }
            break;
        case BTMW_A2DP_CODEC_CONFIG:
            bt_mw_a2dp_codec_config_cb(a2dp_msg->addr,
                                       &(a2dp_msg->data.config));
            break;
        case BTMW_A2DP_DELAY:
            bt_mw_a2dp_delay_cb(a2dp_msg->addr,
                                (BT_MW_A2DP_DELAY *)(&(a2dp_msg->data.delay)));
            break;
        case BTMW_A2DP_CONNECTED:
            bt_mw_a2dp_connected_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_DISCONNECTING:
            bt_mw_a2dp_disconnecting_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_CONNECTING:
            bt_mw_a2dp_connecting_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_DISCONNECTED:
            bt_mw_a2dp_disconnected_cb(a2dp_msg->addr, a2dp_msg->role);
            break;
        case BTMW_A2DP_PLAYER_REPORT_EVENT:
            bt_mw_a2dp_player_report_event_cb(a2dp_msg->addr, a2dp_msg->data.player_event);
            break;
        default:
            break;
    }
}


static VOID bt_mw_a2dp_notify_handle(tBTMW_MSG *p_msg)
{
    BT_A2DP_EVENT_PARAM *a2dp_event = &p_msg->data.a2dp_event;
    BT_DBG_INFO(BT_DEBUG_A2DP, "event:%d, addr:%s", p_msg->hdr.event, p_msg->data.a2dp_msg.addr);
    BT_MW_A2DP_CHECK_INITED();

    switch(p_msg->hdr.event)
    {
        case BTMW_A2DP_NOTIFY_APP:
            if(g_bt_mw_a2dp_event_handle_cb)
            {
                BT_DBG_NORMAL(BT_DEBUG_A2DP, "report event:%d",
                    a2dp_event->event);
                g_bt_mw_a2dp_event_handle_cb(a2dp_event);
            }
            break;
        default:
            break;
    }
}

static INT32 bt_mw_a2dp_get_connected_devices(BT_A2DP_CONNECT_DEV_INFO_LIST *dev_list,
    BT_MW_A2DP_DEV *devices)
{
    UINT32 dev_num = 0;
    UINT32 i = 0;
    BT_CHECK_POINTER(BT_DEBUG_A2DP, dev_list);
    BT_CHECK_POINTER(BT_DEBUG_A2DP, devices);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    for (i = 0; i < BT_MW_A2DP_MAX_DEVICES; i++)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "i:%d devices[i].in_use=%d", i, devices[i].in_use);
        if(devices[i].in_use && (devices[i].conn_status == BT_MW_A2DP_CONNECT_STATUS_CONNECTED))
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "i:%d devices[i].addr=%s", i, devices[i].addr);
            dev_num++;
            memcpy(dev_list->a2dp_connected_dev_list[i].addr, devices[i].addr, MAX_BDADDR_LEN);
            dev_list->a2dp_connected_dev_list[i].stream_status
                = devices[i].stream_status;
            dev_list->a2dp_connected_dev_list[i].conn_status
                = devices[i].conn_status;
            dev_list->a2dp_connected_dev_list[i].local_role
                = devices[i].local_role;
            memcpy(&dev_list->a2dp_connected_dev_list[i].codec_config,
                &devices[i].codec_config, sizeof(BT_MW_A2DP_CODEC_CONFIG));
            memcpy(&dev_list->a2dp_connected_dev_list[i].curr_codec_info,
                &devices[i].curr_codec_info, sizeof(BT_MW_A2DP_CODEC_CONFIG));
            memcpy(&dev_list->a2dp_connected_dev_list[i].config,
                &devices[i].config, sizeof(BT_A2DP_CONF));
            dev_list->a2dp_connected_dev_list[i].delay.delay
                = devices[i].delay.delay;
            dev_list->a2dp_connected_dev_list[i].in_use
                = devices[i].in_use;
            dev_list->a2dp_connected_dev_list[i].is_coming
                = devices[i].is_coming;
            dev_list->a2dp_connected_dev_list[i].is_silence_mode
                = devices[i].is_silence_mode;
        }
    }
    dev_list->dev_num = dev_num;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "dev_list->dev_num=%d", dev_list->dev_num);
    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_get_devices(BT_A2DP_DEVICE_LIST *dev_list,
    BT_MW_A2DP_DEV_LIST_NODE *devices)
{
    BT_MW_A2DP_DEV_LIST_NODE *dev_node = NULL;
    BT_CHECK_POINTER(BT_DEBUG_A2DP, dev_list);
    BT_CHECK_POINTER(BT_DEBUG_A2DP, devices);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    dev_list->dev_num = 0;
    dev_list->is_snklp = g_bt_mw_a2dp_cb.data.is_snklp;
    dev_node = devices;

    while(NULL != dev_node)
    {
        memcpy(&dev_list->dev[dev_list->dev_num],
            &dev_node->dev, sizeof(BT_A2DP_CONNECTED_DEVICE));
        dev_list->dev_num ++;

        dev_node = dev_node->next;
    }
    return BT_SUCCESS;
}

/* FUNCTION NAME: bt_mw_a2dp_move_devices
 * PURPOSE:
 *      move the same device to the head
 * INPUT:
 *      dev_list: the device list
 *      addr    : the device will be moved if exists
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS            -- move success
 *      BT_ERR_STATUS_FAIL    -- not exists
 * NOTES:
 *
 */
static INT32 bt_mw_a2dp_move_devices(BT_MW_A2DP_DEV_LIST_NODE **dev_list,
    CHAR *addr)
{
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    BT_MW_A2DP_DEV_LIST_NODE *next = NULL;
    BLUETOOTH_DEVICE dev_info = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);
    BT_CHECK_POINTER(BT_DEBUG_A2DP, dev_list);
    BT_CHECK_POINTER(BT_DEBUG_A2DP, *dev_list);

    dev = *dev_list;
    next = dev->next;

    if (0 == strcasecmp(dev->dev.addr, addr))
    {
        INT32 ret = BT_ERR_STATUS_DONE;
        /* name may be changed */
        if (BT_SUCCESS == bt_mw_gap_get_device_info(&dev_info, addr))
        {
            /* if the name is valid, update it */
            if (dev_info.name[0] != '\0'
                && strncmp(dev->dev.name, dev_info.name, MAX_NAME_LEN-1))
            {
                strncpy(dev->dev.name, dev_info.name, MAX_NAME_LEN-1);
                dev->dev.name[MAX_NAME_LEN-1] = '\0';
                ret = BT_SUCCESS;
            }
        }
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "%s(%s) at head, don't move", addr, dev->dev.name);
        return ret;
    }

    while(NULL != next)
    {
        if (0 == strcasecmp(next->dev.addr, addr))
        {
            /* name may be changed */
            if (BT_SUCCESS == bt_mw_gap_get_device_info(&dev_info, addr))
            {
                /* if the name is valid, update it */
                if (dev_info.name[0] != '\0')
                {
                    strncpy(next->dev.name, dev_info.name, MAX_NAME_LEN-1);
                    next->dev.name[MAX_NAME_LEN-1] = '\0';
                }
            }
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "%s(%s) move to head", addr, next->dev.name);
            dev->next = next->next;
            next->next = *dev_list;
            *dev_list = next;
            return BT_SUCCESS;
        }

        dev = next;
        next = next->next;
    }
    return BT_ERR_STATUS_FAIL;
}

/* FUNCTION NAME: bt_mw_a2dp_remove_devices
 * PURPOSE:
 *      remove the last device node.
 * INPUT:
 *      dev_list: the device list
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS            -- remove success
 *      BT_ERR_STATUS_FAIL    -- remove fail
 * NOTES:
 *
 */
static INT32 bt_mw_a2dp_remove_devices(BT_MW_A2DP_DEV_LIST_NODE **dev_list)
{
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    BT_MW_A2DP_DEV_LIST_NODE *next = NULL;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "dev_list=%p", dev_list);

    if (NULL == dev_list || NULL == *dev_list)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "dev_list=%p", dev_list);
        return BT_ERR_STATUS_FAIL;
    }

    dev = *dev_list;
    next = dev->next;

    if (NULL == next)
    {
        *dev_list = NULL;
        free(dev);
        return BT_SUCCESS;
    }

    while(NULL != next->next)
    {
        dev = next;
        next = next->next;
    }
    dev->next = NULL;
    free(next);

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_insert_devices(BT_MW_A2DP_DEV_LIST_NODE **dev_list,
    BT_MW_A2DP_DEV_LIST_NODE *dev)
{
    dev->next = *dev_list;
    *dev_list = dev;
    return BT_SUCCESS;
}


static VOID bt_mw_a2dp_add_dev(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    BT_MW_A2DP_DEV_LIST_NODE **dev_list = NULL;
    INT32 *dev_cnt = NULL;
    BLUETOOTH_DEVICE dev_info = {0};
    BT_MW_A2DP_DEV_LIST_NODE *dev = NULL;
    INT32 ret = BT_SUCCESS;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    dev_list = &g_bt_mw_a2dp_cb.data.dev_list[role];
    dev_cnt = &g_bt_mw_a2dp_cb.data.dev_cnt[role];

    ret = bt_mw_a2dp_move_devices(dev_list, addr);
    if (ret == BT_ERR_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "addr:%s exist", addr);
        return;
    }
    else if (ret == BT_SUCCESS)
    {
        bt_mw_a2dp_save_dev_list(*dev_list, role);
        return;
    }

    dev = (BT_MW_A2DP_DEV_LIST_NODE *)malloc(sizeof(BT_MW_A2DP_DEV_LIST_NODE));
    if (NULL == dev)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "addr:%s alloc fail", addr);
        return;
    }

    strncpy(dev->dev.addr, addr, MAX_BDADDR_LEN-1);
    dev->dev.addr[MAX_BDADDR_LEN-1] = '\0';
    dev->dev.role = role;
    if (BT_SUCCESS == bt_mw_gap_get_device_info(&dev_info, addr))
    {
        strncpy(dev->dev.name, dev_info.name, MAX_NAME_LEN-1);
        dev->dev.name[MAX_NAME_LEN-1] = '\0';
    }
    else
    {
        dev->dev.name[0] = '\0';
    }

    bt_mw_a2dp_insert_devices(dev_list, dev);
    *dev_cnt = *dev_cnt + 1;

    if (*dev_cnt > BT_MW_A2DP_MAX_DEVICE_LIST)
    {
        bt_mw_a2dp_remove_devices(dev_list);
    }

    bt_mw_a2dp_dump_dev_list(*dev_list);

    bt_mw_a2dp_save_dev_list(*dev_list, role);

    return;
}

static VOID bt_mw_a2dp_dump_dev_list(BT_MW_A2DP_DEV_LIST_NODE *dev_list)
{
    UINT32 cnt = 0;
    while(NULL != dev_list)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[%d]dev(%s), role(%d), name(%s)",
            cnt, dev_list->dev.addr, dev_list->dev.role, dev_list->dev.name);
        dev_list = dev_list->next;
        cnt++;
    }

    return;
}

static INT32 bt_mw_a2dp_check_pending(VOID)
{
    BT_MW_A2DP_ROLE local_role = BT_MW_A2DP_ROLE_MAX;
    UINT32 pending_flag = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "pending_flag=0x%x",
        g_bt_mw_a2dp_cb.data.pending_flag);

    if (0 == g_bt_mw_a2dp_cb.data.pending_flag)
    {
        return BT_SUCCESS;
    }
    if ((g_bt_mw_a2dp_cb.data.pending_flag &
        (1 << BT_MW_A2DP_PENDING_POWER_OFF))
        || (g_bt_mw_a2dp_cb.data.pending_flag &
            (1 << BT_MW_A2DP_PENDING_DEINIT)))
    {
        local_role = BT_MW_A2DP_ROLE_MAX;
    }
    else
    {
        if (g_bt_mw_a2dp_cb.data.pending_flag &
            (1 << BT_MW_A2DP_PENDING_SRC_DISABLE))
        {
            local_role = BT_MW_A2DP_ROLE_SRC;
        }
        if (g_bt_mw_a2dp_cb.data.pending_flag &
            (1 << BT_MW_A2DP_PENDING_SINK_DISABLE))
        {
            if (BT_MW_A2DP_ROLE_SRC == local_role)
            {
                local_role = BT_MW_A2DP_ROLE_MAX;
            }
            else
            {
                local_role = BT_MW_A2DP_ROLE_SINK;
            }
        }
    }

    if (TRUE == bt_mw_a2dp_has_connection(local_role))
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "0x%x is pending",
                        g_bt_mw_a2dp_cb.data.pending_flag);
        bt_mw_a2dp_disconnect_all(local_role);
        return BT_ERR_STATUS_BUSY;
    }
    pending_flag = g_bt_mw_a2dp_cb.data.pending_flag;
    g_bt_mw_a2dp_cb.data.pending_flag = 0;

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_SRC_DISABLE))
    {
        bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SRC, FALSE, NULL);
    }

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_SINK_DISABLE))
    {
        bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SINK, FALSE, NULL);
    }

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_SRC_ENABLE))
    {
        bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SRC, TRUE, &g_bt_mw_a2dp_cb.config.src_pending_init_config);
    }

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_SINK_ENABLE))
    {
        bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE_SINK, TRUE, NULL);
    }

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_POWER_OFF))
    {
        bt_mw_a2dp_enable(FALSE);
    }

    if (pending_flag & (1 << BT_MW_A2DP_PENDING_DEINIT))
    {
        bt_mw_a2dp_deinit();
    }
    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_connected_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    INT32 freq = 48000;
    INT32 channel = 2;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx >= 0)
    {
        if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED ==
            g_bt_mw_a2dp_cb.devices[idx].conn_status)
        {
            BT_DBG_WARNING(BT_DEBUG_A2DP, "%s has connected", addr);
            BT_MW_A2DP_UNLOCK();
            return BT_SUCCESS;
        }

        if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
        {
            BT_DBG_WARNING(BT_DEBUG_A2DP,
                "%s role change, org role=%d, new role=%d", addr,
                g_bt_mw_a2dp_cb.devices[idx].local_role, role);
            g_bt_mw_a2dp_cb.devices[idx].local_role = role;
        }
    }
    else
    {
        idx = bt_mw_a2dp_alloc_dev_index(addr, role);
        if (idx < 0)
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "no device resource dev %s", addr);
            BT_MW_A2DP_UNLOCK();
            return BT_ERR_STATUS_FAIL;
        }
        g_bt_mw_a2dp_cb.devices[idx].local_role = role;
    }
    g_bt_mw_a2dp_cb.devices[idx].conn_status =
            BT_MW_A2DP_CONNECT_STATUS_CONNECTED;
    g_bt_mw_a2dp_cb.devices[idx].stream_status = BT_MW_A2DP_STREAM_STATUS_PAUSE;

    /* sample rate and channel mode only update for sink */
    if (role == BT_MW_A2DP_ROLE_SINK)
    {
        freq = g_bt_mw_a2dp_cb.devices[idx].codec_config.sample_rate;
        channel = g_bt_mw_a2dp_cb.devices[idx].codec_config.channel_mode;

        /* if only support 1 sink connection, select player here, or it should
         * select by app.
         */
        if (g_bt_mw_a2dp_cb.config.link_num[role] == 1)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "select player for codec %s",
                bt_mw_a2dp_get_codec_str(g_bt_mw_a2dp_cb.devices[idx].config.codec_type));
            //bt_mw_a2dp_sink_select_player(addr, g_bt_mw_a2dp_cb.devices[idx].config.codec_type);
        }
    }

    bt_mw_a2dp_add_dev(addr, role);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "freq=%d, channel=%d", freq, channel);

#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
    if (BT_MW_A2DP_ROLE_SRC == role)
    {
        bt_mw_a2dp_src_start_uploader(freq, channel);
    }
#endif
    bt_mw_a2dp_check_pending();
    BT_MW_A2DP_UNLOCK();

    {
        INT32 count = 0, other_count = 0;
        BT_MW_A2DP_ROLE other_role =
            (role==BT_MW_A2DP_ROLE_SRC)?BT_MW_A2DP_ROLE_SINK:BT_MW_A2DP_ROLE_SRC;
        count = bt_mw_a2dp_get_link_cnt_by_role(role);
        other_count = bt_mw_a2dp_get_link_cnt_by_role(other_role);
        if (count > g_bt_mw_a2dp_cb.config.link_num[role] ||
            other_count > 0)
        {
            BT_DBG_ERROR(BT_DEBUG_A2DP, "%s conn over(%d) or other role conn(%d)",
                addr, count, other_count);
            bt_mw_a2dp_disconnect(addr);
        }
        else
        {
            memset(&param, 0, sizeof(param));
            param.data.connected_data.local_role = role;
            param.data.connected_data.sample_rate = freq;
            param.data.connected_data.channel_num = channel;
            param.data.connected_data.config = g_bt_mw_a2dp_cb.devices[idx].config;

            strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
            param.event = BT_A2DP_EVENT_CONNECTED;
            BT_MW_A2DP_REPORT_EVENT(param);
            bt_mw_a2dp_change_thread_priority(-19); // -19: high priority
        }
    }

    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_start_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    memset(&param, 0, sizeof(param));
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
            addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
        return BT_ERR_STATUS_FAIL;
    }
    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED !=
            g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "device %s bad state(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        return BT_ERR_STATUS_FAIL;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role == BT_MW_A2DP_ROLE_SINK)
    {
        g_mw_start_record = false;
    }

    if (BT_MW_A2DP_STREAM_STATUS_PLAYING ==
        g_bt_mw_a2dp_cb.devices[idx].stream_status)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "%s is playing", addr);
    }
    g_bt_mw_a2dp_cb.devices[idx].stream_status = BT_MW_A2DP_STREAM_STATUS_PLAYING;

    BT_MW_A2DP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.event = BT_A2DP_EVENT_STREAM_START;
    BT_MW_A2DP_REPORT_EVENT(param);

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_suspend_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    memset(&param, 0, sizeof(param));
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
        return BT_ERR_STATUS_FAIL;
    }
    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED !=
            g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "device %s bad state(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
        return BT_ERR_STATUS_FAIL;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }
    if (BT_MW_A2DP_STREAM_STATUS_PLAYING !=
        g_bt_mw_a2dp_cb.devices[idx].stream_status)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "%s is pause(%d)", addr,
            g_bt_mw_a2dp_cb.devices[idx].stream_status);
    }
    g_bt_mw_a2dp_cb.devices[idx].stream_status = BT_MW_A2DP_STREAM_STATUS_PAUSE;

    BT_MW_A2DP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.event = BT_A2DP_EVENT_STREAM_SUSPEND;
    BT_MW_A2DP_REPORT_EVENT(param);

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_sink_config_cb(CHAR *addr, BT_MW_A2DP_ROLE role,
    UINT32 freq, UINT8 channel, UINT8 codec_type)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s, freq=%d, channel=%d",
        addr, bt_mw_a2dp_get_role_str(role), freq, channel);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "not found  %s, for bt_mw_a2dp_sink_config_cb", addr);
        idx = bt_mw_a2dp_alloc_dev_index(addr, BT_MW_A2DP_ROLE_SINK);
        if (idx < 0)
          {
              BT_MW_A2DP_UNLOCK();
              BT_DBG_ERROR(BT_DEBUG_A2DP, "no alloc device for  %s", addr);
              return BT_ERR_STATUS_FAIL;
          }
        g_bt_mw_a2dp_cb.devices[idx].local_role = BT_MW_A2DP_ROLE_SINK;
        g_bt_mw_a2dp_cb.devices[idx].is_coming = TRUE;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "%s sample_rate=%d, channel_mode=%d codec_type=%d",
        addr, freq, channel, codec_type);

    g_bt_mw_a2dp_cb.devices[idx].codec_config.sample_rate = freq;
    g_bt_mw_a2dp_cb.devices[idx].codec_config.channel_mode = channel;
    g_bt_mw_a2dp_cb.devices[idx].codec_config.codec_type = codec_type;

    memset(&param, 0, sizeof(param));

    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.event = BT_A2DP_EVENT_SINK_AUDIO_CONFIG;
    param.data.sink_audio_config.sample_rate = freq;
    param.data.sink_audio_config.channel_num = channel;
    param.data.sink_audio_config.codec_type = codec_type;
    BT_MW_A2DP_REPORT_EVENT(param);

    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_src_config_cb(CHAR *addr, BT_MW_A2DP_ROLE role,
        BT_A2DP_EVENT_AUDIO_CONFIG_DATA *p_event_audio_config_data)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s", addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "not found  %s, for bt_mw_a2dp_src_config_cb", addr);
        idx = bt_mw_a2dp_alloc_dev_index(addr, BT_MW_A2DP_ROLE_SRC);
        if (idx < 0)
          {
              BT_MW_A2DP_UNLOCK();
              BT_DBG_ERROR(BT_DEBUG_A2DP, "no alloc device for  %s", addr);
              return BT_ERR_STATUS_FAIL;
          }
        g_bt_mw_a2dp_cb.devices[idx].local_role = BT_MW_A2DP_ROLE_SRC;
        g_bt_mw_a2dp_cb.devices[idx].is_coming = TRUE;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }
    // save curr codec config info
    memcpy(&g_bt_mw_a2dp_cb.devices[idx].curr_codec_info, &p_event_audio_config_data->audio_config, sizeof(BT_A2DP_CODEC_CONFIG));
    // notify event
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.codec_type =%d", g_bt_mw_a2dp_cb.devices[idx].curr_codec_info.codec_type);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.codec_priority =%d", g_bt_mw_a2dp_cb.devices[idx].curr_codec_info.codec_priority);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.sample_rate =%d", g_bt_mw_a2dp_cb.devices[idx].curr_codec_info.sample_rate);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.bits_per_sample =%d", g_bt_mw_a2dp_cb.devices[idx].curr_codec_info.bits_per_sample);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP]codec_config.channel_mode =%d", g_bt_mw_a2dp_cb.devices[idx].curr_codec_info.channel_mode);
    memset(&param, 0, sizeof(param));

    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.event = BT_A2DP_EVENT_SRC_AUDIO_CONFIG;

    memcpy(&param.data.audio_config, p_event_audio_config_data, sizeof(BT_A2DP_EVENT_AUDIO_CONFIG_DATA));
    BT_MW_A2DP_REPORT_EVENT(param);
    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_disconnected_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    BOOL connect_timeout = FALSE;
#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
    BT_MW_A2DP_ROLE local_role = BT_MW_A2DP_ROLE_MAX;
#endif

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
        return BT_SUCCESS;
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTING ==
            g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        if (FALSE == g_bt_mw_a2dp_cb.devices[idx].is_coming)
        {
            connect_timeout = TRUE;
        }
        goto __FREE_DEV;
    }

#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
    local_role = g_bt_mw_a2dp_cb.devices[idx].local_role;
    if (BT_MW_A2DP_ROLE_SRC == local_role)
    {
        bt_mw_a2dp_src_stop_uploader();
    }
#endif

__FREE_DEV:
    bt_mw_a2dp_free_dev(idx);
    bt_mw_a2dp_sink_delay_free_dev(addr);

    if (memcmp(g_bt_mw_a2dp_cb.active_dev_addr, addr, MAX_BDADDR_LEN-1) == 0)
    {
        //strncpy(g_bt_mw_a2dp_cb.active_dev_addr, "00:00:00:00:00:00", MAX_BDADDR_LEN-1);
        //g_bt_mw_a2dp_cb.active_dev_addr[MAX_BDADDR_LEN-1] = '\0';
        BT_DBG_WARNING(BT_DEBUG_A2DP, "active device disconnected %s", addr);
    }

    memset(&param, 0, sizeof(param));
    if (TRUE == connect_timeout)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "device[%d] %s connect timeout",
            idx, addr);
        strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
        param.event = BT_A2DP_EVENT_CONNECT_TIMEOUT;
        BT_MW_A2DP_REPORT_EVENT(param);
    }
    else
    {
        strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
        param.event = BT_A2DP_EVENT_DISCONNECTED;
        BT_MW_A2DP_REPORT_EVENT(param);
        bt_mw_a2dp_change_thread_priority(0);   // 0: low priority (default)
    }

    bt_mw_a2dp_check_pending();
    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_player_report_event_cb(CHAR *addr, BT_A2DP_PLAYER_EVENT player_event)
{
    BT_A2DP_EVENT_PARAM param = {0};

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, player_event=%s",
        addr, bt_mw_a2dp_get_player_event_str(player_event));
    memset(&param, 0, sizeof(param));
    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.event = BT_A2DP_EVENT_PLAYER_EVENT;
    param.data.player_event = player_event;

    BT_MW_A2DP_REPORT_EVENT(param);

    return BT_SUCCESS;
}



static INT32 bt_mw_a2dp_disconnecting_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
        return BT_ERR_STATUS_FAIL;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTED !=
        g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s bad status %d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
    }

    g_bt_mw_a2dp_cb.devices[idx].conn_status = BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING;


#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
    if (BT_MW_A2DP_ROLE_SRC == role)
    {
        bt_mw_a2dp_src_stop_uploader();
    }
#endif
    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}

static INT32 bt_mw_a2dp_enable(BOOL power_on)
{
#if ENABLE_A2DP_SRC
    BOOL src_enable = FALSE;
#endif
#if ENABLE_A2DP_SINK
    BOOL sink_enable = FALSE;
#endif
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "power_on=%d", power_on);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);
    BT_MW_A2DP_LOCK();

    if (power_on == g_bt_mw_a2dp_cb.data.power_on)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "power not change(%d)", power_on);
        g_bt_mw_a2dp_cb.data.pending_flag &=
            ~(1 << BT_MW_A2DP_PENDING_POWER_OFF);
        BT_MW_A2DP_UNLOCK();
        return BT_SUCCESS;
    }


#if ENABLE_A2DP_SRC
    src_enable = g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SRC];
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "src_enable=%d", src_enable);
#endif
#if ENABLE_A2DP_SINK
    sink_enable = g_bt_mw_a2dp_cb.config.role_enable[BT_MW_A2DP_ROLE_SINK];;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "sink_enable=%d", sink_enable);
#endif

    /* if power on and there is connection, pending it */
    if (FALSE == power_on)
    {
        if (g_bt_mw_a2dp_cb.data.pending_flag)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "0x%x is pending, pending power off",
                g_bt_mw_a2dp_cb.data.pending_flag);

            g_bt_mw_a2dp_cb.data.pending_flag |=
                (1 << BT_MW_A2DP_PENDING_POWER_OFF);
            BT_MW_A2DP_UNLOCK();
            return BT_ERR_STATUS_BUSY;
        }

        if (TRUE == bt_mw_a2dp_has_connection(BT_MW_A2DP_ROLE_MAX))
        {
            bt_mw_a2dp_disconnect_all(BT_MW_A2DP_ROLE_MAX);
            g_bt_mw_a2dp_cb.data.pending_flag |=
                (1 << BT_MW_A2DP_PENDING_POWER_OFF);
            BT_MW_A2DP_UNLOCK();
            return BT_ERR_STATUS_NOT_READY;
        }

        memset(&g_bt_mw_a2dp_cb.devices, 0, sizeof(g_bt_mw_a2dp_cb.devices));
    }
    g_bt_mw_a2dp_cb.data.power_on = power_on;

#if ENABLE_A2DP_SRC
    if (TRUE == src_enable)
    {
        bt_mw_a2dp_enable_src_stack(power_on,
            &g_bt_mw_a2dp_cb.config.src_pending_init_config);
    }
#endif

#if ENABLE_A2DP_SINK
    if (TRUE == sink_enable)
    {
        bt_mw_a2dp_enable_sink_stack(power_on,
            g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SINK]);
    }
#endif

#if ENABLE_A2DP_SRC || ENABLE_A2DP_SINK
    bt_mw_a2dp_enable_common_stack(power_on);
#endif

    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_role_enable(BT_MW_A2DP_ROLE local_role, BOOL enable,
    BT_A2DP_SRC_INIT_CONFIG* p_src_init_config)
{
    BOOL role_enable = FALSE;
    BOOL power_on = FALSE;
    BT_A2DP_EVENT_PARAM param = {0};
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "local role=%s, enable=%d",
        bt_mw_a2dp_get_role_str(local_role), enable);
    BT_MW_A2DP_IS_VALID_ROLE(local_role);
    BT_MW_A2DP_CHECK_INITED(BT_ERR_STATUS_FAIL);

    BT_MW_A2DP_LOCK();

    if (local_role == BT_MW_A2DP_ROLE_SRC)
    {
        if (p_src_init_config != NULL && enable)
        {
            memcpy(&g_bt_mw_a2dp_cb.config.src_pending_init_config, p_src_init_config,
                                                    sizeof(BT_A2DP_SRC_INIT_CONFIG));
        }
        else
        {
            memset(&g_bt_mw_a2dp_cb.config.src_pending_init_config, 0, sizeof(BT_A2DP_SRC_INIT_CONFIG));
        }
    }

    role_enable = g_bt_mw_a2dp_cb.config.role_enable[local_role];
    if (role_enable == enable)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "enable=%d no change", enable);
        if (TRUE == enable)
        {
            g_bt_mw_a2dp_cb.data.pending_flag &=
                ~(1 << (BT_MW_A2DP_PENDING_SRC_DISABLE + local_role));
        }
        else
        {
            g_bt_mw_a2dp_cb.data.pending_flag &=
                ~(1 << (BT_MW_A2DP_PENDING_SRC_ENABLE + local_role));
        }
        BT_MW_A2DP_UNLOCK();
        return BT_SUCCESS;
    }

    if (FALSE == enable)
    {
        if (g_bt_mw_a2dp_cb.data.pending_flag)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "0x%x is pending, pending %s disable",
                g_bt_mw_a2dp_cb.data.pending_flag,
                bt_mw_a2dp_get_role_str(local_role));

            g_bt_mw_a2dp_cb.data.pending_flag |=
                (1 << (BT_MW_A2DP_PENDING_SRC_DISABLE+local_role));
            BT_MW_A2DP_UNLOCK();

            return BT_ERR_STATUS_BUSY;
        }

        if (TRUE == bt_mw_a2dp_has_connection(local_role))
        {
            bt_mw_a2dp_disconnect_all(local_role);
            g_bt_mw_a2dp_cb.data.pending_flag |=
                (1 << (BT_MW_A2DP_PENDING_SRC_DISABLE + local_role));
            BT_MW_A2DP_UNLOCK();

            BT_DBG_WARNING(BT_DEBUG_A2DP, "has connection, pending %s",
                bt_mw_a2dp_get_role_str(local_role));
            return BT_ERR_STATUS_BUSY;
        }
    }
    else
    {
        if (g_bt_mw_a2dp_cb.data.pending_flag)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "0x%x is pending, pending %s enable",
                g_bt_mw_a2dp_cb.data.pending_flag,
                bt_mw_a2dp_get_role_str(local_role));

            g_bt_mw_a2dp_cb.data.pending_flag |=
                (1 << (BT_MW_A2DP_PENDING_SRC_ENABLE+local_role));
            BT_MW_A2DP_UNLOCK();

            return BT_ERR_STATUS_BUSY;
        }
    }
    g_bt_mw_a2dp_cb.config.role_enable[local_role] = enable;
    power_on = g_bt_mw_a2dp_cb.data.power_on;

    if (TRUE == power_on)
    {
        if (BT_MW_A2DP_ROLE_SINK == local_role)
        {
#if ENABLE_A2DP_SINK
            bt_mw_a2dp_enable_sink_stack(enable,
                g_bt_mw_a2dp_cb.config.link_num[BT_MW_A2DP_ROLE_SINK]);
#endif
        }

        if (BT_MW_A2DP_ROLE_SRC == local_role)
        {
#if ENABLE_A2DP_SRC
            bt_mw_a2dp_enable_src_stack(enable,
                &g_bt_mw_a2dp_cb.config.src_pending_init_config);
#endif
        }
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "power is off(%d)", power_on);
    }
    BT_MW_A2DP_UNLOCK();
    usleep(500000);
    if (TRUE == power_on)
    {
        param.event = BT_A2DP_EVENT_ROLE_CHANGED;
        param.data.role_change.role = local_role;
        param.data.role_change.enable = enable;
        BT_MW_A2DP_REPORT_EVENT(param);
    }
    return BT_SUCCESS;
}


static INT32 bt_mw_a2dp_connecting_cb(CHAR *addr, BT_MW_A2DP_ROLE role)
{
    INT32 idx = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, role=%s",
        addr, bt_mw_a2dp_get_role_str(role));

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "not found  %s, connection coming", addr);
        idx = bt_mw_a2dp_alloc_dev_index(addr, role);
        if (idx < 0)
        {
            BT_MW_A2DP_UNLOCK();
            BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
            return BT_ERR_STATUS_FAIL;
        }
        g_bt_mw_a2dp_cb.devices[idx].local_role = role;
        g_bt_mw_a2dp_cb.devices[idx].is_coming = TRUE;
    }

    if (g_bt_mw_a2dp_cb.devices[idx].local_role != role)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s role change, org role=%d, new role=%d", addr,
            g_bt_mw_a2dp_cb.devices[idx].local_role, role);
    }

    if (BT_MW_A2DP_CONNECT_STATUS_CONNECTING <
        g_bt_mw_a2dp_cb.devices[idx].conn_status)
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP,
            "%s bad status %d", addr,
            g_bt_mw_a2dp_cb.devices[idx].conn_status);
    }

    g_bt_mw_a2dp_cb.devices[idx].conn_status = BT_MW_A2DP_CONNECT_STATUS_CONNECTING;

    BT_MW_A2DP_UNLOCK();

    return BT_SUCCESS;
}

static VOID bt_mw_a2dp_delay_cb(CHAR *addr, BT_MW_A2DP_DELAY *delay)
{
    INT32 idx = 0;
    BT_A2DP_EVENT_PARAM param = {0};
    memset(&param, 0, sizeof(param));
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_A2DP, delay);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device %s", addr);
        return;
    }

    g_bt_mw_a2dp_cb.devices[idx].delay.delay = delay->delay;

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "current delay is:(%d)",
                  delay->delay);
    BT_MW_A2DP_UNLOCK();

    strncpy(param.addr, addr, MAX_BDADDR_LEN - 1);
    param.data.delay.delay = delay->delay;
    param.event = BT_A2DP_EVENT_DELAY;
    BT_MW_A2DP_REPORT_EVENT(param);
}

static VOID bt_mw_a2dp_codec_config_cb(CHAR *addr, BT_A2DP_CONF *current_config)
{
    INT32 idx = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_A2DP, current_config);

    BT_MW_A2DP_LOCK();
    idx = bt_mw_a2dp_get_dev_index(addr);
    if (idx < 0)
    {
        BT_MW_A2DP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_A2DP, "no device  %s", addr);
        return;
    }

    g_bt_mw_a2dp_cb.devices[idx].config = *current_config;

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "current codec is:%s(%d)\n",
        bt_mw_a2dp_get_codec_str(current_config->codec_type),
        current_config->codec_type);
    switch (current_config->codec_type)
    {
        case BT_A2DP_CODEC_TYPE_STE: //A2D_MEDIA_CT_STE
        {
            BT_A2DP_SBC_CONF *local_ste = &current_config->codec_conf.ste_conf;
            BT_DBG_MINOR(BT_DEBUG_A2DP, "min_bitpool:%d\n", local_ste->min_bitpool);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "max_bitpool:%d\n", local_ste->max_bitpool);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "block_len:%d\n", local_ste->block_len);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "num_subbands:%d\n", local_ste->num_subbands);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "alloc_mthd:%d\n", local_ste->alloc_mthd);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "samp_freq:%d\n", local_ste->samp_freq);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "ch_mode:%d\n", local_ste->ch_mode);

            break;
        }
        case BT_A2DP_CODEC_TYPE_SBC: //A2D_MEDIA_CT_SBC
        {
            BT_A2DP_SBC_CONF *local_sbc = &current_config->codec_conf.sbc_conf;
            BT_DBG_MINOR(BT_DEBUG_A2DP, "min_bitpool:%d\n", local_sbc->min_bitpool);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "max_bitpool:%d\n", local_sbc->max_bitpool);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "block_len:%d\n", local_sbc->block_len);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "num_subbands:%d\n", local_sbc->num_subbands);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "alloc_mthd:%d\n", local_sbc->alloc_mthd);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "samp_freq:%d\n", local_sbc->samp_freq);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "ch_mode:%d\n", local_sbc->ch_mode);

            break;
        }
        case BT_A2DP_CODEC_TYPE_AAC:
        {
            BT_A2DP_AAC_CONF *local_aac = &current_config->codec_conf.aac_conf;
            BT_DBG_MINOR(BT_DEBUG_A2DP, "object_type:%d\n", local_aac->object_type);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "sample_rate:%d\n", local_aac->samp_freq);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "channels:%d\n", local_aac->channels);
            BT_DBG_MINOR(BT_DEBUG_A2DP, "VBR supported? (%s)\n", local_aac->vbr ? "TRUE" : "FALSE");
            BT_DBG_MINOR(BT_DEBUG_A2DP, "bit_rate:%d\n", local_aac->bitrate);

            break;
        }
#if ENABLE_LDAC_CODEC
        case BT_A2DP_CODEC_TYPE_LDAC:
        {
            break;
        }
#endif
        case BT_A2DP_CODEC_TYPE_LHDC_LL:
        case BT_A2DP_CODEC_TYPE_LHDC:
        {
            BT_A2DP_LHDC_CONF *local_ldac = &current_config->codec_conf.vendor_conf.conf.lhdc_conf;
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "sample_rate:%d\n", local_ldac->samp_freq);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "channels:%d\n", local_ldac->channels);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "bitdepth:%d\n", local_ldac->bit_depth);
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "max_rate:%d\n", local_ldac->max_rate);
            break;
        }
        default:
            break;
    }
    BT_MW_A2DP_UNLOCK();
}

#if ENABLE_A2DP_SRC
static INT32 bt_mw_a2dp_enable_src_stack(BOOL src_enable,
                         BT_A2DP_SRC_INIT_CONFIG *p_src_init_config)
{
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "src_enable=%d", src_enable);
    if (TRUE == src_enable)
    {
        linuxbt_a2dp_src_init(p_src_init_config);
        usleep(30*1000);

#if ENABLE_AVRCP_PROFILE
        linuxbt_rc_tg_init();
#endif
    }
    else
    {
        linuxbt_a2dp_src_deinit();
        //usleep(500*1000);

#if ENABLE_AVRCP_PROFILE
        linuxbt_rc_tg_deinit();
#endif
    }

    return BT_SUCCESS;
}
#endif

#if ENABLE_A2DP_SINK
static INT32 bt_mw_a2dp_enable_sink_stack(BOOL sink_enable, INT32 sink_num)
{
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "sink_enable=%d, sink_num=%d", sink_enable, sink_num);
    if (TRUE == sink_enable)
    {
        bt_mw_a2dp_sink_init();

        linuxbt_a2dp_sink_init(sink_num);
        //usleep(500*1000);

#if ENABLE_AVRCP_PROFILE
        linuxbt_rc_init();
#endif
    }
    else
    {
        linuxbt_a2dp_sink_deinit();
        //usleep(500*1000);

#if ENABLE_AVRCP_PROFILE
        linuxbt_rc_deinit();
#endif
        bt_mw_a2dp_sink_deinit();
    }
    return BT_SUCCESS;
}
#endif

#if ENABLE_A2DP_SINK || ENABLE_A2DP_SRC
static INT32 bt_mw_a2dp_enable_common_stack(BOOL common_enable)
{
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "common_enable=%d", common_enable);
    if (TRUE == common_enable)
    {
        //linuxbt_a2dp_ext_init(); //need refactor
    }
    else
    {
        //linuxbt_a2dp_ext_deinit();
    }
    return BT_SUCCESS;
}
#endif

static VOID bt_mw_a2dp_notify_app(BT_A2DP_EVENT_PARAM *param)
{
    tBTMW_MSG msg;
    msg.hdr.event = BTMW_A2DP_NOTIFY_APP;
    msg.hdr.len = sizeof(*param);
    memcpy((void*)&msg.data.a2dp_event, param, sizeof(*param));
    bt_mw_nty_send_msg(&msg);
}


static CHAR* bt_mw_a2dp_get_role_str(BT_MW_A2DP_ROLE role)
{
    switch((int)role)
    {
        BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_ROLE_SRC, "src");
        BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_ROLE_SINK, "sink");
        BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_ROLE_MAX, "invalid");
        default: return "UNKNOWN_ROLE";
   }
}

static CHAR* bt_mw_a2dp_get_event_str(UINT32 event)
{
    switch((int)event)
    {
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_CONNECTED, "connected");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_STREAM_START, "stream_start");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_STREAM_SUSPEND, "stream_suspend");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_STREAM_CONFIG, "stream_config");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_CODEC_CONFIG, "codec_config");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_DISCONNECTED, "disconnected");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_CONNECTING, "connecting");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_DISCONNECTING, "disconnecting");
        BT_MW_A2DP_CASE_RETURN_STR(BTMW_A2DP_PLAYER_REPORT_EVENT, "report_player_event");
        default: return "UNKNOWN_EVENT";
   }
}


static CHAR* bt_mw_a2dp_get_player_event_str(BT_A2DP_PLAYER_EVENT player_event)
{
    switch((int)player_event)
    {
        BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP, "stop_ok");
        BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP_FAIL, "stop_fail");
        BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START, "start_ok");
        BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START_FAIL, "start_fail");
        BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_DATA_COME, "data_come");
        default: return "UNKNOWN_STATE";
   }
}

static CHAR* bt_mw_a2dp_get_codec_str(UINT8 codec_tpye)
{
     switch(codec_tpye)
     {
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_SBC, "sbc");
         //BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_MP3, "mp3");
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_AAC, "aac");
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_LDAC, "ldac");
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_STE, "stereo");
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_LHDC_LL, "lhdc-ll");
         BT_MW_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_LHDC, "lhdc");
         default: return "UNKNOWN_CODEC_TYPE";
    }
}

static CHAR* bt_mw_a2dp_get_stream_state_str(BT_MW_A2DP_STREAM_STATUS state)
{
     switch(state)
     {
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_STREAM_STATUS_PAUSE, "pause");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_STREAM_STATUS_PLAYING, "playing");
         default: return "UNKNOWN_STATE";
    }
}

static CHAR* bt_mw_a2dp_get_conn_state_str(BT_MW_A2DP_CONNECT_STATUS state)
{
     switch(state)
     {
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_CONNECT_STATUS_DISCONNECTED, "disconnected");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_CONNECT_STATUS_DISCONNECTING, "disconnecting");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_CONNECT_STATUS_CONNECTING, "connecting");
         BT_MW_A2DP_CASE_RETURN_STR(BT_MW_A2DP_CONNECT_STATUS_CONNECTED, "connected");
         default: return "UNKNOWN_STATE";
    }
}


