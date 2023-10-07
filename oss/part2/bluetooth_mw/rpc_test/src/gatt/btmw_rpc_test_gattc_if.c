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
 *//* FILE NAME:  btmw_rpc_test_gattc_if.c
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include "list.h"

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_gattc_if.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatt_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define BTMW_RPC_TEST_GATTC_TEST_APP_CNT (BT_GATT_MAX_APP_CNT+10)

#define BTMW_RPC_TEST_GATTC_MAX_WRITTEN_TIME_SECOND 30

/* MACRO FUNCTION DECLARATIONS
 */
#define BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(_const,str) case _const: return str

#define BTMW_RPC_TEST_GATTC_CHECK(expr, value) \
    do { \
    if(!(expr)){ \
        printf("[\033[31m fail \033[0m] %s@%d " #expr ", "#value"=%d\n", __FUNCTION__, __LINE__, value);\
    }\
    else printf("[\033[32m pass \033[0m] %s@%d " #expr "\n", __FUNCTION__, __LINE__);\
    }while(0)

#define BTMW_RPC_TEST_GATTC_LOCK(lock) \
    do { \
        pthread_mutex_lock(&(lock)); \
    }while(0)


#define BTMW_RPC_TEST_GATTC_UNLOCK(lock) \
        do { \
            pthread_mutex_unlock(&(lock));\
        }while(0)

#define BTMW_RPC_TEST_GATTC_SIGNAL(lock, signal, cond) \
    do { \
        pthread_mutex_lock(&(lock));\
        (cond);\
        pthread_cond_signal(&(signal));\
        pthread_mutex_unlock(&(lock));\
    }while(0)


#define BTMW_RPC_TEST_GATTC_WAIT(lock, signal, cond) \
        do { \
            pthread_mutex_lock(&(lock));\
            while(!(cond))\
            {\
                pthread_cond_wait(&(signal), &(lock));\
            }\
            pthread_mutex_unlock(&(lock));\
        }while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    UINT8 perm;
    UINT8 key_size;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 len;
    struct dl_list node; /* link to char */
} BTMW_RPC_TEST_GATTC_DESC;

typedef struct
{
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    UINT8 prop;
    UINT8 perm;
    UINT8 key_size;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 len;

    UINT8 reg_notify; /* 0: not register, 1: registered */

    /* for throughput test */
    pthread_mutex_t lock;
    pthread_cond_t signal;
    UINT32 test_length;
    UINT32 pkt_cnt;

    struct dl_list node; /* link to service */
    struct dl_list desc_list;
} BTMW_RPC_TEST_GATTC_CHAR;

typedef struct
{
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    struct dl_list node; /* link to service */
} BTMW_RPC_TEST_GATTC_INCLUDE;

typedef struct
{
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    GATT_ATTR_TYPE type; /* attr type, pri, sec, include, ...*/
    UINT16 handle;
    struct dl_list node; /* link to client */
    struct dl_list include_list;
    struct dl_list char_list;
} BTMW_RPC_TEST_GATTC_SERVICE;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];      /* server addr */
    struct dl_list node;            /* link to server_list */
    struct dl_list service_list;
} BTMW_RPC_TEST_GATTC_SERVER;

typedef struct
{
    struct dl_list node; /* link to client */
    CHAR addr[MAX_BDADDR_LEN];
    INT32 connected;
    INT32 mtu;
    INT32 tx_phy;
    INT32 rx_phy;

    INT32 interval;
    INT32 latency;
    INT32 timeout;

    INT32 rssi;
} BTMW_RPC_TEST_GATTC_CONNECTION;

typedef struct
{
    INT32 client_if;
    BT_GATT_STATUS status;
    CHAR client_name[BT_GATT_MAX_NAME_LEN];

    struct dl_list server_list;     /* remote server list */
    struct dl_list connection_list; /* connected device list */

    /* for ut */
    pthread_t handle;
    pthread_mutex_t lock;
    pthread_cond_t signal;
    UINT32 event_mask;

    /* for tp test no log */
    INT32 tp_test;
} BTMW_RPC_TEST_GATTC_CLIENT;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if 0
static INT32 btmw_rpc_test_gattc_cmp_uuid(CHAR *uuid1, CHAR *uuid2);
#endif
static INT32 btmw_rpc_test_gattc_parse_auth_req(CHAR *auth_req_str);
static INT32 btmw_rpc_test_gattc_parse_connect_mode(CHAR *connect_mode);
static INT32 btmw_rpc_test_gattc_parse_transport(CHAR *transport_str);
static INT32 btmw_rpc_test_gattc_parse_value(CHAR *value_str);
static INT32 btmw_rpc_test_gattc_parse_excute_mode(CHAR *excute_mode_str);
static INT32 btmw_rpc_test_gattc_parse_register_mode(CHAR *register_mode_str);
static VOID btmw_rpc_test_gattc_show_value(CHAR *title, UINT8 *value, UINT32 value_len);

static VOID btmw_rpc_test_gattc_handle_register_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_REGISTER_DATA *register_data,
    INT32 client_if);

static VOID btmw_rpc_test_gattc_handle_connection_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_CONNECTION_DATA *connection_data);

static VOID btmw_rpc_test_gattc_handle_notif_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_NOTIF_DATA *notif_data);

static VOID btmw_rpc_test_gattc_handle_read_char_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_CHAR_RSP_DATA *read_char_rsp_data);

static VOID btmw_rpc_test_gattc_handle_write_char_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA *write_char_rsp_data);

static VOID btmw_rpc_test_gattc_handle_exec_write_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA *exec_write_rsp_data);

static VOID btmw_rpc_test_gattc_handle_read_desc_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_DESC_RSP_DATA *read_desc_rsp_data);

static VOID btmw_rpc_test_gattc_handle_write_desc_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_WRITE_DESC_RSP_DATA *write_char_rsp_data);

static VOID btmw_rpc_test_gattc_handle_read_rssi_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_RSSI_DATA *read_rssi_data);

static VOID btmw_rpc_test_gattc_handle_mtu_change_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_MTU_CHG_DATA *mtu_chg_data);

static VOID btmw_rpc_test_gattc_handle_conn_update_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_CONN_UPDATE_DATA *conn_update_data);

static VOID btmw_rpc_test_gattc_handle_bt_off_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client);

static VOID btmw_rpc_test_gattc_handle_phy_update_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_PHY_UPDATE_DATA *phy_update_data);

static VOID btmw_rpc_test_gattc_handle_phy_read_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_PHY_DATA *phy_read_data);

static VOID btmw_rpc_test_gattc_free_server(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    CHAR *addr);

static VOID btmw_rpc_test_gattc_add_db_in_server(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_GET_GATT_DB_DATA *get_db_data);

static VOID btmw_rpc_test_gattc_handle_get_db_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_GET_GATT_DB_DATA *get_db_data);

static VOID btmw_rpc_test_gattc_event_handle(BT_GATTC_EVENT_PARAM *param, VOID *pv_tag);
static int btmw_rpc_test_gattc_register_client_by_name(char *client_name);
static int btmw_rpc_test_gattc_register_client(int argc, char **argv);
static int btmw_rpc_test_gattc_unregister_client_by_name(char *client_name);
static int btmw_rpc_test_gattc_unregister_client(int argc, char **argv);
static int btmw_rpc_test_gattc_connect(int argc, char **argv);
static int btmw_rpc_test_gattc_disconnect(int argc, char **argv);
static int btmw_rpc_test_gattc_refresh(int argc, char **argv);
static int btmw_rpc_test_gattc_read_char(int argc, char **argv);
static int btmw_rpc_test_gattc_read_char_by_uuid(int argc, char **argv);
static int btmw_rpc_test_gattc_read_desc(int argc, char **argv);
static int btmw_rpc_test_gattc_cmd_write_char(int argc, char **argv);
static int btmw_rpc_test_gattc_reliable_write_char(int argc, char **argv);
static int btmw_rpc_test_gattc_exec_write_char(int argc, char **argv);
static int btmw_rpc_test_gattc_write_char(int argc, char **argv);
static int btmw_rpc_test_gattc_write_desc(int argc, char **argv);
static int btmw_rpc_test_gattc_reg_char_notify(int argc, char **argv);
static int btmw_rpc_test_gattc_read_rssi(int argc, char **argv);
static int btmw_rpc_test_gattc_get_dev_type(int argc, char **argv);
static int btmw_rpc_test_gattc_change_mtu(int argc, char **argv);
static int btmw_rpc_test_gattc_conn_update(int argc, char **argv);
static int btmw_rpc_test_gattc_read_phy(int argc, char **argv);
static int btmw_rpc_test_gattc_set_pref_phy(int argc, char **argv);
static int btmw_rpc_test_gattc_get_db(int argc, char **argv);
static int btmw_rpc_test_gattc_show_db(int argc, char **argv);
static int btmw_rpc_test_gattc_show_char(int argc, char **argv);
static int btmw_rpc_test_gattc_show_desc(int argc, char **argv);
static int btmw_rpc_test_gattc_throughput_test(int argc, char **argv);

static int btmw_rpc_test_gattc_cmd_handler(int argc, char **argv);

static BTMW_RPC_TEST_GATTC_CONNECTION *btmw_rpc_test_gattc_find_connection(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr);

static BTMW_RPC_TEST_GATTC_CHAR *btmw_rpc_test_gattc_find_char_by_handle(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr, UINT32 handle);

static BTMW_RPC_TEST_GATTC_DESC * btmw_rpc_test_gattc_find_desc_by_handle(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr, UINT32 handle);


static VOID btmw_rpc_test_gattc_show_server_db(
    BTMW_RPC_TEST_GATTC_CLIENT *client,
    CHAR *addr);

static INT32 btmw_rpc_test_gattc_alloc_client(void);
static INT32 btmw_rpc_test_gattc_find_client_by_name(char *client_name);
static BTMW_RPC_TEST_GATTC_CLIENT* btmw_rpc_test_gattc_find_client_by_if(INT32 client_if);
VOID btmw_rpc_test_gattc_free_client_by_if(INT32 client_if);
VOID btmw_rpc_test_gattc_free_client_by_index(INT32 index);
static CHAR* btmw_rpc_test_gattc_get_event_str(BT_GATTC_EVENT event);
static CHAR* btmw_rpc_test_gattc_get_status_str(BT_GATT_STATUS status);
static CHAR* btmw_rpc_test_gattc_get_attr_type_str(GATT_ATTR_TYPE type);
static CHAR* btmw_rpc_test_gattc_get_char_prop_str(UINT8 properties, CHAR *prop_str);
/* STATIC VARIABLE DECLARATIONS
 */
static BTMW_RPC_TEST_CLI btmw_rpc_test_gattc_cli_commands[] =
{
    {(const char *)"register_client",     btmw_rpc_test_gattc_register_client,     (const char *)" = register_client <client_name>"},
    {(const char *)"unregister_client",   btmw_rpc_test_gattc_unregister_client,   (const char *)" = unregister_client <name>"},
    {(const char *)"connect",             btmw_rpc_test_gattc_connect,             (const char *)" = connect <client_name> <addr> [<bg|direct> [<auto|bredr|ble> [<initor_phy:0xHH> [<opportunitistic>]]]]"},
    {(const char *)"disconnect",          btmw_rpc_test_gattc_disconnect,          (const char *)" = disconnect <client_name> <addr>"},
    {(const char *)"refresh",             btmw_rpc_test_gattc_refresh,             (const char *)" = refresh <client_name> <addr>"},
    {(const char *)"read_char",           btmw_rpc_test_gattc_read_char,           (const char *)" = read_char <client_name> <addr> <handle> <default:none|no-mitm|mitm>"},
    {(const char *)"read_char_by_uuid",   btmw_rpc_test_gattc_read_char_by_uuid,   (const char *)" = read_char_by_uuid <client_name> <addr> <char_uuid> <start_handle> <end_handle> <default:none|no-mitm|mitm>"},
    {(const char *)"read_desc",           btmw_rpc_test_gattc_read_desc,           (const char *)" = read_desc <client_name> <addr> <handle> <default:none|no-mitm|mitm>"},
    {(const char *)"cmd_write_char",      btmw_rpc_test_gattc_cmd_write_char,      (const char *)" = cmd_write_char <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>"},
    {(const char *)"reliable_write_char", btmw_rpc_test_gattc_reliable_write_char, (const char *)" = reliable_write_char <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>"},
    {(const char *)"exec_write_char",     btmw_rpc_test_gattc_exec_write_char,     (const char *)" = exec_write_char <client_name> <addr> <0:cancel, 1:exec>"},
    {(const char *)"write_char",          btmw_rpc_test_gattc_write_char,          (const char *)" = write_char <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>"},
    {(const char *)"write_desc",          btmw_rpc_test_gattc_write_desc,          (const char *)" = write_desc <client_name> <addr> <handle> <desc_uuid> <default:none|no-mitm|mitm> <value>"},
    {(const char *)"reg_char_notify",     btmw_rpc_test_gattc_reg_char_notify,     (const char *)" = reg_char_notify <client_name> <addr> <reg_char_notify_param.handle> <0:unregister, 1:register>"},
    {(const char *)"read_rssi",           btmw_rpc_test_gattc_read_rssi,           (const char *)" = read_rssi <client_name> <addr>"},
    {(const char *)"get_dev_type",        btmw_rpc_test_gattc_get_dev_type,        (const char *)" = get_dev_type <client_name> <addr>"},
    {(const char *)"change_mtu",          btmw_rpc_test_gattc_change_mtu,          (const char *)" = change_mtu <client_name> <addr> <mtu>"},
    {(const char *)"conn_update",         btmw_rpc_test_gattc_conn_update,         (const char *)" = conn_update <client_name> <addr> <min_int> <max_int> <latency> <timeout> <min_ce> <max_ce>"},
    {(const char *)"read_phy",            btmw_rpc_test_gattc_read_phy,            (const char *)" = read_phy <client_name> <addr>"},
    {(const char *)"set_pref_phy",        btmw_rpc_test_gattc_set_pref_phy,        (const char *)" = set_pref_phy <client_name> <addr> <tx_phy> <rx_phy> <phy_option>"},
    {(const char *)"get_db",              btmw_rpc_test_gattc_get_db,              (const char *)" = get_db <client_name> <addr>"},
    {(const char *)"show_db",             btmw_rpc_test_gattc_show_db,             (const char *)" = show_db <client_name> <addr>"},
    {(const char *)"show_char",           btmw_rpc_test_gattc_show_char,           (const char *)" = show_char <client_name> <addr> <handle> "},
    {(const char *)"show_desc",           btmw_rpc_test_gattc_show_desc,           (const char *)" = show_desc <client_name> <addr> <handle>"},
    {(const char *)"throughput_test",     btmw_rpc_test_gattc_throughput_test,     (const char *)" = throughput_test <client_name> <addr> <char_handle> <length, unit:k> [<pkt_length, unit:byte> [write_req]]"},
    {NULL, NULL, NULL},
};
static pthread_mutex_t btmw_rpc_test_gattc_lock;

static BTMW_RPC_TEST_GATTC_CLIENT s_btmw_rpc_gattc_clients[BTMW_RPC_TEST_GATTC_TEST_APP_CNT];
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */
#if 0

static INT32 btmw_rpc_test_gattc_cmp_uuid(CHAR *uuid1, CHAR *uuid2)
{
    UINT8 uuid_128[2][16];
    UINT32 i = 0;
    UINT8 base_uuid[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
                0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};

    if (NULL == uuid1 || NULL == uuid2)
    {
        return -1;
    }

    if ((strlen(uuid1) >= BT_GATT_MAX_UUID_LEN)
        || (strlen(uuid2) >= BT_GATT_MAX_UUID_LEN))
    {
        BTMW_RPC_TEST_Logi("uuid1 %s or uuid2 %s too long", uuid1, uuid2);
        return -1;
    }

    for (i = 0; i < 2; i++)
    {
        CHAR *uuid = NULL;
        UINT8 *ptr_uuid_128 = uuid_128[i];
        memcpy((void*)ptr_uuid_128, base_uuid, sizeof(base_uuid));
        if (i == 0) uuid = uuid1;
        else uuid = uuid2;

        if (strlen(uuid) == BT_GATT_MAX_UUID_LEN - 1) {
            int c;
            int rc = sscanf(uuid,
              "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx"
              "-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%n",
              &ptr_uuid_128[0], &ptr_uuid_128[1], &ptr_uuid_128[2], &ptr_uuid_128[3],
              &ptr_uuid_128[4], &ptr_uuid_128[5], &ptr_uuid_128[6], &ptr_uuid_128[7],
              &ptr_uuid_128[8], &ptr_uuid_128[9], &ptr_uuid_128[10], &ptr_uuid_128[11],
              &ptr_uuid_128[12], &ptr_uuid_128[13], &ptr_uuid_128[14], &ptr_uuid_128[15], &c);
          } else if (strlen(uuid) == 8) {
            int c;
            int rc = sscanf(uuid,
              "%02hhx%02hhx%02hhx%02hhx%n",
              &ptr_uuid_128[0], &ptr_uuid_128[1], &ptr_uuid_128[2], &ptr_uuid_128[3], &c);
          } else if (strlen(uuid) == 4) {
            int c;
            int rc = sscanf(uuid,
              "%02hhx%02hhx%n", &ptr_uuid_128[2], &ptr_uuid_128[3], &c);
          }
          else
          {
            return -1;
          }
    }

    return memcmp(&uuid_128[0], &uuid_128[1], 16);
}
#endif

static INT32 btmw_rpc_test_gattc_parse_auth_req(CHAR *auth_req_str)
{
    if (NULL == auth_req_str)
    {
        return BT_GATTC_AUTH_REQ_NONE;
    }

    if (!strncasecmp(auth_req_str, "mitm", 4))
    {
        return BT_GATTC_AUTH_REQ_MITM;
    }
    else if (!strncasecmp(auth_req_str, "no-mitm", 7))
    {
        return BT_GATTC_AUTH_REQ_NO_MITM;
    }

    return BT_GATTC_AUTH_REQ_NONE;
}

static INT32 btmw_rpc_test_gattc_parse_connect_mode(CHAR *connect_mode)
{
    if (NULL == connect_mode)
    {
        return 0;
    }

    if (!strncasecmp(connect_mode, "direct", 6))
    {
        return 1;
    }

    return 0;
}

static INT32 btmw_rpc_test_gattc_parse_transport(CHAR *transport_str)
{
    if (NULL == transport_str)
    {
        return GATT_TRANSPORT_TYPE_AUTO;
    }

    if (!strncasecmp(transport_str, "auto", 4))
    {
        return GATT_TRANSPORT_TYPE_AUTO;
    }
    else if (!strncasecmp(transport_str, "bredr", 5))
    {
        return GATT_TRANSPORT_TYPE_BREDR;
    }
    else if (!strncasecmp(transport_str, "ble", 5))
    {
        return GATT_TRANSPORT_TYPE_LE;
    }

    return GATT_TRANSPORT_TYPE_AUTO;
}

static INT32 btmw_rpc_test_gattc_parse_value(CHAR *value_str)
{
    INT32 tmp = 0;
    if (NULL == value_str)
    {
        return 0;
    }

    if (!strncmp("0x", value_str, 2))
    {
        if (sscanf(value_str, "0x%x", &tmp))
        {
            return tmp;
        }
        else
        {
            return 0;
        }
    }

    if (!strncmp("0X", value_str, 2))
    {
        if (sscanf(value_str, "0X%x", &tmp))
        {
            return tmp;
        }
        else
        {
            return 0;
        }
    }

    if (!strncasecmp("true", value_str, 4))
    {
        return 1;
    }
    else if (!strncasecmp("false", value_str, 5))
    {
        return 0;
    }

    return atoi(value_str);
}

static INT32 btmw_rpc_test_gattc_parse_excute_mode(CHAR *excute_mode_str)
{
    if (NULL == excute_mode_str)
    {
        return 0;
    }

    if (!strncasecmp("exec", excute_mode_str, 4))
    {
        return 1;
    }
    else if (!strncasecmp("cancel", excute_mode_str, 6))
    {
        return 0;
    }

    return 0;
}

static INT32 btmw_rpc_test_gattc_parse_register_mode(CHAR *register_mode_str)
{
    if (NULL == register_mode_str)
    {
        return 0;
    }

    if (!strncasecmp("register", register_mode_str, strlen("register")))
    {
        return 1;
    }
    else if (!strncasecmp("unregister", register_mode_str, strlen("unregister")))
    {
        return 0;
    }

    return 0;
}

static VOID btmw_rpc_test_gattc_handle_register_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_REGISTER_DATA *register_data,
    INT32 client_if)
{
    if (NULL == register_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("client_name:%s, status:%d, client_if:%d",
        register_data->client_name, register_data->status, client_if);
    gattc_client->status = register_data->status;
    if (NULL != gattc_client)
    {
        if (register_data->status == BT_GATT_STATUS_OK)
        {
            gattc_client->client_if = client_if;
            gattc_client->status = register_data->status;
        }
        else
        {
            btmw_rpc_test_gattc_free_client_by_index(gattc_client - s_btmw_rpc_gattc_clients);
            BTMW_RPC_TEST_Loge("server_name:%s, status:%d register fail, free it",
                register_data->client_name,
                register_data->status);
        }
    }

    return;
}

static VOID btmw_rpc_test_gattc_handle_connection_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_CONNECTION_DATA *connection_data)
{
    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    if (NULL == connection_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, status:%d, connected:%d",
        connection_data->addr, connection_data->status,
        connection_data->connected);

    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, connection_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->connected = connection_data->connected;
            if (connection_data->connected)
            {
                BTMW_RPC_TEST_Logd("client_if %d addr:%s reconnected",
                    gattc_client->client_if, conn->addr);
            }
            else
            {
                BTMW_RPC_TEST_Logd("client_if %d addr:%s disconnected",
                    gattc_client->client_if,
                    connection_data->addr);
            }
            return;
        }
    }

    if (connection_data->connected)
    {
        conn = calloc(1, sizeof(BTMW_RPC_TEST_GATTC_CONNECTION));
        if (NULL == conn) return;

        strncpy(conn->addr, connection_data->addr, MAX_BDADDR_LEN - 1);
        conn->addr[MAX_BDADDR_LEN - 1] = '\0';
        conn->mtu = BT_GATT_DEFAULT_MTU_SIZE;
        dl_list_init(&conn->node);
        dl_list_add_tail(&gattc_client->connection_list, &conn->node);

        BTMW_RPC_TEST_Logd("client_if %d addr:%s connected",
            gattc_client->client_if, conn->addr);
    }
    else
    {
        BTMW_RPC_TEST_Logd("client_if %d addr:%s connect fail",
            gattc_client->client_if,
            connection_data->addr);
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_notif_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_NOTIF_DATA *notif_data)
{
    if (NULL == notif_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, handle:%d is_notify:%d",
        notif_data->addr, notif_data->handle, notif_data->is_notify);

    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (!strncasecmp(server->addr, notif_data->addr, MAX_BDADDR_LEN - 1))
        {
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            dl_list_for_each(service, &server->service_list,
                BTMW_RPC_TEST_GATTC_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
                dl_list_for_each(gattc_char, &service->char_list,
                    BTMW_RPC_TEST_GATTC_CHAR, node)
                {
                    if (gattc_char->handle == notif_data->handle)
                    {
                        BTMW_RPC_TEST_Logd("char:%s value_len:%d",
                            gattc_char->uuid, notif_data->value_len);
                        if (notif_data->value_len > BT_GATT_MAX_VALUE_LEN)
                        {
                            notif_data->value_len = BT_GATT_MAX_VALUE_LEN;
                        }
                        memset(gattc_char->value, 0, sizeof(gattc_char->value));
                        memcpy(gattc_char->value, notif_data->value, notif_data->value_len);
                        gattc_char->len = notif_data->value_len;

                        btmw_rpc_test_gattc_show_value("ind char value",
                            gattc_char->value, gattc_char->len);
                        return;
                    }
                }
            }
        }
    }

    BTMW_RPC_TEST_Logd("addr:%s, handle:%d, value_len:%d not found",
        notif_data->addr, notif_data->handle,
        notif_data->value_len);
    if (0 < notif_data->value_len)
    {
        btmw_rpc_test_gattc_show_value("ind char value", notif_data->value,
            notif_data->value_len);
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_read_char_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_CHAR_RSP_DATA *read_char_rsp_data)
{
    if (NULL == read_char_rsp_data || NULL == gattc_client)
    {
        return;
    }


    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (!strncasecmp(server->addr, read_char_rsp_data->addr, MAX_BDADDR_LEN - 1))
        {
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            dl_list_for_each(service, &server->service_list,
                BTMW_RPC_TEST_GATTC_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
                dl_list_for_each(gattc_char, &service->char_list,
                    BTMW_RPC_TEST_GATTC_CHAR, node)
                {
                    if (gattc_char->handle == read_char_rsp_data->handle)
                    {
                        BTMW_RPC_TEST_Logd("addr:%s, handle:%d, char:%s value_len:%d, status:%s",
                            read_char_rsp_data->addr, read_char_rsp_data->handle,
                            gattc_char->uuid, read_char_rsp_data->value_len,
                            btmw_rpc_test_gattc_get_status_str(read_char_rsp_data->status));
                        if (read_char_rsp_data->status == BT_GATT_STATUS_OK)
                        {
                            if (read_char_rsp_data->value_len > BT_GATT_MAX_VALUE_LEN)
                            {
                                read_char_rsp_data->value_len = BT_GATT_MAX_VALUE_LEN;
                            }
                            memset(gattc_char->value, 0, sizeof(gattc_char->value));
                            memcpy(gattc_char->value, read_char_rsp_data->value,
                                read_char_rsp_data->value_len);
                            gattc_char->len = read_char_rsp_data->value_len;

                            btmw_rpc_test_gattc_show_value("char value",
                                gattc_char->value, gattc_char->len);
                        }
                        return;
                    }
                }
            }
        }
    }
    BTMW_RPC_TEST_Logd("addr:%s, handle:%d, value_len:%d status:%s, not found",
        read_char_rsp_data->addr, read_char_rsp_data->handle,
        read_char_rsp_data->value_len,
        btmw_rpc_test_gattc_get_status_str(read_char_rsp_data->status));
    if (0 < read_char_rsp_data->value_len)
    {
        btmw_rpc_test_gattc_show_value("char value", read_char_rsp_data->value,
            read_char_rsp_data->value_len);
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_write_char_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_WRITE_CHAR_RSP_DATA *write_char_rsp_data)
{
    if (NULL == write_char_rsp_data || NULL == gattc_client)
    {
        return;
    }


    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
        dl_list_for_each(server, &gattc_client->server_list,
            BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (!strncasecmp(server->addr, write_char_rsp_data->addr, MAX_BDADDR_LEN - 1))
        {
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            dl_list_for_each(service, &server->service_list,
                BTMW_RPC_TEST_GATTC_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
                dl_list_for_each(gattc_char, &service->char_list,
                    BTMW_RPC_TEST_GATTC_CHAR, node)
                {
                    if (gattc_char->handle == write_char_rsp_data->handle)
                    {
                        pthread_mutex_lock(&gattc_char->lock);

                        if (gattc_char->pkt_cnt == 0)
                        {
                            BTMW_RPC_TEST_Logi("throughput write 1st pkt rsp");
                            BTMW_RPC_TEST_Logd("addr:%s, handle:%d, char:%s write result %s",
                                write_char_rsp_data->addr, write_char_rsp_data->handle,
                                gattc_char->uuid,
                                btmw_rpc_test_gattc_get_status_str(write_char_rsp_data->status));
                        }
                        pthread_cond_signal(&gattc_char->signal);
                        pthread_mutex_unlock(&gattc_char->lock);
                        return;
                    }
                }
            }
        }
    }
    BTMW_RPC_TEST_Logd("addr:%s, handle:%d, status:%s, not found",
        write_char_rsp_data->addr, write_char_rsp_data->handle,
         btmw_rpc_test_gattc_get_status_str(write_char_rsp_data->status));

    return;
}

static VOID btmw_rpc_test_gattc_handle_exec_write_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_EXEC_WRITE_RSP_DATA *exec_write_rsp_data)
{
    if (NULL == exec_write_rsp_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, status:%d",
        exec_write_rsp_data->addr, exec_write_rsp_data->status);

    if (exec_write_rsp_data->status != BT_GATT_STATUS_OK)
    {
        BTMW_RPC_TEST_Logd("exec write char fail, %s",
            btmw_rpc_test_gattc_get_status_str(exec_write_rsp_data->status));
        return;
    }

    return;
}

static VOID btmw_rpc_test_gattc_handle_read_desc_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_DESC_RSP_DATA *read_desc_rsp_data)
{
    if (NULL == read_desc_rsp_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (!strncasecmp(server->addr, read_desc_rsp_data->addr, MAX_BDADDR_LEN - 1))
        {
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            dl_list_for_each(service, &server->service_list,
                BTMW_RPC_TEST_GATTC_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
                dl_list_for_each(gattc_char, &service->char_list,
                    BTMW_RPC_TEST_GATTC_CHAR, node)
                {
                    BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
                    dl_list_for_each(gattc_desc, &gattc_char->desc_list,
                        BTMW_RPC_TEST_GATTC_DESC, node)
                    {
                        if (gattc_desc->handle == read_desc_rsp_data->handle)
                        {
                            BTMW_RPC_TEST_Logd("addr:%s, handle:%d, char %s desc:%s value_len:%d",
                                read_desc_rsp_data->addr, read_desc_rsp_data->handle,
                                gattc_char->uuid, gattc_desc->uuid,
                                read_desc_rsp_data->value_len);
                            if (read_desc_rsp_data->status == BT_GATT_STATUS_OK)
                            {
                                if (read_desc_rsp_data->value_len > BT_GATT_MAX_VALUE_LEN)
                                {
                                    read_desc_rsp_data->value_len = BT_GATT_MAX_VALUE_LEN;
                                }
                                memset(gattc_desc->value, 0, sizeof(gattc_desc->value));
                                memcpy(gattc_desc->value, read_desc_rsp_data->value,
                                    read_desc_rsp_data->value_len);
                                gattc_desc->len = read_desc_rsp_data->value_len;

                                btmw_rpc_test_gattc_show_value("desc value",
                                    gattc_desc->value, gattc_desc->len);
                            }
                            return;
                        }
                    }
                }
            }
        }
    }

    BTMW_RPC_TEST_Logd("addr:%s, handle:%d, value_len: %d status:%s, not found",
        read_desc_rsp_data->addr, read_desc_rsp_data->handle,
        read_desc_rsp_data->value_len,
        btmw_rpc_test_gattc_get_status_str(read_desc_rsp_data->status));
    if (0 < read_desc_rsp_data->value_len)
    {
        btmw_rpc_test_gattc_show_value("char value", read_desc_rsp_data->value,
            read_desc_rsp_data->value_len);
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_write_desc_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_WRITE_DESC_RSP_DATA *write_desc_rsp_data)
{
    if (NULL == write_desc_rsp_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (!strncasecmp(server->addr, write_desc_rsp_data->addr, MAX_BDADDR_LEN - 1))
        {
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            dl_list_for_each(service, &server->service_list,
                BTMW_RPC_TEST_GATTC_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
                dl_list_for_each(gattc_char, &service->char_list,
                    BTMW_RPC_TEST_GATTC_CHAR, node)
                {
                    BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
                    dl_list_for_each(gattc_desc, &gattc_char->desc_list,
                        BTMW_RPC_TEST_GATTC_DESC, node)
                    {
                        if (gattc_desc->handle == write_desc_rsp_data->handle)
                        {
                            BTMW_RPC_TEST_Logd("addr:%s, handle:%d, char %s desc:%s write result:%s",
                                write_desc_rsp_data->addr, write_desc_rsp_data->handle,
                                gattc_char->uuid, gattc_desc->uuid,
                                btmw_rpc_test_gattc_get_status_str(write_desc_rsp_data->status));
                            return;
                        }
                    }
                }
            }
        }
    }
    BTMW_RPC_TEST_Logd("addr:%s, handle:%d, status:%s, not found",
        write_desc_rsp_data->addr, write_desc_rsp_data->handle,
         btmw_rpc_test_gattc_get_status_str(write_desc_rsp_data->status));

    return;
}

static VOID btmw_rpc_test_gattc_handle_read_rssi_rsp_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_RSSI_DATA *read_rssi_data)
{
    if (NULL == read_rssi_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, rssi:%d, status:%s",
        read_rssi_data->addr, read_rssi_data->rssi,
        btmw_rpc_test_gattc_get_status_str(read_rssi_data->status));

    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, read_rssi_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->rssi = read_rssi_data->rssi;
            return;
        }
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_mtu_change_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_MTU_CHG_DATA *mtu_chg_data)
{
    if (NULL == mtu_chg_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, mtu:%d, status:%s",
        mtu_chg_data->addr, mtu_chg_data->mtu,
        btmw_rpc_test_gattc_get_status_str(mtu_chg_data->status));

    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, mtu_chg_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->mtu = mtu_chg_data->mtu;
            return;
        }
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_conn_update_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_CONN_UPDATE_DATA *conn_update_data)
{
    if (NULL == conn_update_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, interval:%d*1.25ms, latency:%d, timeout:%dms",
        conn_update_data->status, conn_update_data->addr,
        conn_update_data->interval, conn_update_data->latency,
        conn_update_data->timeout*10);
    gattc_client->status = conn_update_data->status;

    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, conn_update_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->interval = conn_update_data->interval;
            conn->latency = conn_update_data->latency;
            conn->timeout = conn_update_data->timeout;
            return;
        }
    }
    return;
}

static INT32 btmw_rpc_test_gattc_free_service(BTMW_RPC_TEST_GATTC_SERVICE *service)
{
    /* free include */
    BTMW_RPC_TEST_GATTC_INCLUDE *incl = NULL;
    BTMW_RPC_TEST_GATTC_CHAR *gatt_char = NULL;

    if (NULL == service) return 0;

    BTMW_RPC_TEST_Logi("free service %s ", service->uuid);

    while (!dl_list_empty(&service->include_list))
    {
        incl = dl_list_first(&service->include_list, BTMW_RPC_TEST_GATTC_INCLUDE, node);
        if (NULL != incl)
        {
            dl_list_del(&incl->node);
            free(incl);
            incl = NULL;
        }
    }

    while (!dl_list_empty(&service->char_list))
    {
        gatt_char = dl_list_first(&service->char_list, BTMW_RPC_TEST_GATTC_CHAR, node);
        if (NULL != gatt_char)
        {
            BTMW_RPC_TEST_GATTC_DESC *gatt_desc = NULL;
            while (!dl_list_empty(&gatt_char->desc_list))
            {
                gatt_desc = dl_list_first(&gatt_char->desc_list, BTMW_RPC_TEST_GATTC_DESC, node);
                if (NULL != gatt_desc)
                {
                    dl_list_del(&gatt_desc->node);
                    free(gatt_desc);
                    gatt_desc = NULL;
                }
            }

            pthread_cond_destroy(&gatt_char->signal);
            pthread_mutex_destroy(&gatt_char->lock);
            dl_list_del(&gatt_char->node);
            free(gatt_char);
            gatt_char = NULL;
        }
    }

    return 0;
}

static VOID btmw_rpc_test_gattc_free_server(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    CHAR *addr)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;

    if (NULL == addr || NULL == gattc_client)
    {
        return;
    }
    BTMW_RPC_TEST_Logi("free server %s ", addr);

    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (strncasecmp(server->addr, addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }
        /* free include */
        BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
        while (!dl_list_empty(&server->service_list))
        {
            service = dl_list_first(&server->service_list, BTMW_RPC_TEST_GATTC_SERVICE, node);
            if (NULL != service)
            {
                BTMW_RPC_TEST_Logi("free service %s ", service->uuid);
                dl_list_del(&service->node);

                btmw_rpc_test_gattc_free_service(service);
                free(service);
                service = NULL;
            }
        }
        dl_list_del(&server->node);
        free(server);
        return;
    }
}

static VOID btmw_rpc_test_gattc_add_db_in_server(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_GET_GATT_DB_DATA *get_db_data)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    CHAR prop_str[128];

    if (NULL == get_db_data || NULL == gattc_client)
    {
        return;
    }
    BTMW_RPC_TEST_Logi("add server %s ", get_db_data->addr);

    dl_list_for_each(server, &gattc_client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (strncasecmp(server->addr, get_db_data->addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }
        UINT32 attr_index = 0;
        BTMW_RPC_TEST_Logi("found server %s ", get_db_data->addr);
        for (attr_index = 0; attr_index < get_db_data->attr_cnt; attr_index++)
        {
            BTMW_RPC_TEST_Logi("attr[%d] handle %d type %s ", attr_index,
                get_db_data->attrs[attr_index].handle,
                btmw_rpc_test_gattc_get_attr_type_str(get_db_data->attrs[attr_index].type));
            if ((GATT_ATTR_TYPE_PRIMARY_SERVICE ==
                get_db_data->attrs[attr_index].type)
                || (GATT_ATTR_TYPE_SECONDARY_SERVICE ==
                get_db_data->attrs[attr_index].type))
            {
                BTMW_RPC_TEST_GATTC_SERVICE *service =
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTC_SERVICE));
                if (NULL == service)
                {
                    BTMW_RPC_TEST_Logw("alloc service fail");
                    return;
                }
                dl_list_init(&service->char_list);
                dl_list_init(&service->include_list);
                dl_list_init(&service->node);
                service->handle = get_db_data->attrs[attr_index].handle;
                service->type = get_db_data->attrs[attr_index].type;
                strncpy(service->uuid, (CHAR*)get_db_data->attrs[attr_index].uuid,
                    BT_GATT_MAX_UUID_LEN - 1);
                service->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
                dl_list_add_tail(&server->service_list, &service->node);
                BTMW_RPC_TEST_Logi("add service %s handle %d", service->uuid, service->handle);
                continue;
            }

            if (GATT_ATTR_TYPE_INCLUDED_SERVICE ==
                get_db_data->attrs[attr_index].type)
            {
                BTMW_RPC_TEST_GATTC_INCLUDE *gatt_include =
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTC_INCLUDE));
                if (NULL == gatt_include)
                {
                    BTMW_RPC_TEST_Logw("alloc gatt_include fail");
                    return;
                }
                dl_list_init(&gatt_include->node);
                gatt_include->handle = get_db_data->attrs[attr_index].handle;
                strncpy(gatt_include->uuid, (CHAR*)get_db_data->attrs[attr_index].uuid,
                    BT_GATT_MAX_UUID_LEN - 1);
                gatt_include->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

                BTMW_RPC_TEST_GATTC_SERVICE *service =
                    dl_list_last(&server->service_list, BTMW_RPC_TEST_GATTC_SERVICE, node);

                if (service != NULL)
                {
                    dl_list_add_tail(&service->include_list, &gatt_include->node);
                    BTMW_RPC_TEST_Logi("add include %s handle %d", gatt_include->uuid, gatt_include->handle);
                }
                else
                {
                    free(gatt_include);
                }
                continue;
            }

            if (GATT_ATTR_TYPE_CHARACTERISTIC ==
                get_db_data->attrs[attr_index].type)
            {
                pthread_condattr_t condattr;
                BTMW_RPC_TEST_GATTC_CHAR *gatt_char =
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTC_CHAR));
                if (NULL == gatt_char)
                {
                    BTMW_RPC_TEST_Logw("alloc gatt_char fail");
                    return;
                }
                dl_list_init(&gatt_char->node);
                dl_list_init(&gatt_char->desc_list);

                if (pthread_condattr_init(&condattr) < 0)
                {
                    BTMW_RPC_TEST_Logw("init cond fail\n");
                }
                pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
                pthread_cond_init(&gatt_char->signal, &condattr);
                pthread_mutex_init(&gatt_char->lock, NULL);
                pthread_condattr_destroy(&condattr);

                gatt_char->handle = get_db_data->attrs[attr_index].handle;
                gatt_char->prop = get_db_data->attrs[attr_index].properties;
                gatt_char->perm = get_db_data->attrs[attr_index].permissions;
                gatt_char->key_size = get_db_data->attrs[attr_index].key_size;
                strncpy(gatt_char->uuid, (CHAR*)get_db_data->attrs[attr_index].uuid,
                    BT_GATT_MAX_UUID_LEN - 1);
                gatt_char->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

                BTMW_RPC_TEST_GATTC_SERVICE *service =
                    dl_list_last(&server->service_list, BTMW_RPC_TEST_GATTC_SERVICE, node);

                if (service != NULL)
                {
                    dl_list_add_tail(&service->char_list, &gatt_char->node);
                    BTMW_RPC_TEST_Logi("add char %s handle %d prop 0x%x(%s)",
                        gatt_char->uuid, gatt_char->handle, gatt_char->prop,
                        btmw_rpc_test_gattc_get_char_prop_str(gatt_char->prop, prop_str));
                }
                else
                {
                    pthread_cond_destroy(&gatt_char->signal);
                    pthread_mutex_destroy(&gatt_char->lock);
                    free(gatt_char);
                }
                continue;
            }

            if (GATT_ATTR_TYPE_DESCRIPTOR ==
                get_db_data->attrs[attr_index].type)
            {
                BTMW_RPC_TEST_GATTC_DESC *gatt_desc =
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTC_DESC));
                if (NULL == gatt_desc)
                {
                    BTMW_RPC_TEST_Logw("alloc gatt_desc fail");
                    return;
                }
                dl_list_init(&gatt_desc->node);
                gatt_desc->handle = get_db_data->attrs[attr_index].handle;
                gatt_desc->perm = get_db_data->attrs[attr_index].permissions;
                gatt_desc->key_size = get_db_data->attrs[attr_index].key_size;
                strncpy(gatt_desc->uuid, (CHAR*)get_db_data->attrs[attr_index].uuid,
                    BT_GATT_MAX_UUID_LEN - 1);
                gatt_desc->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

                BTMW_RPC_TEST_GATTC_SERVICE *service =
                    dl_list_last(&server->service_list, BTMW_RPC_TEST_GATTC_SERVICE, node);
                if (service != NULL)
                {
                    BTMW_RPC_TEST_GATTC_CHAR *gatt_char =
                        dl_list_last(&service->char_list, BTMW_RPC_TEST_GATTC_CHAR, node);
                    if (gatt_char != NULL)
                    {
                        dl_list_add_tail(&gatt_char->desc_list, &gatt_desc->node);
                        BTMW_RPC_TEST_Logi("add desc %s handle %d ",
                            gatt_desc->uuid, gatt_desc->handle);
                    }
                    else
                    {
                        free(gatt_desc);
                    }
                }
                else
                {
                    free(gatt_desc);
                }
                continue;
            }
        }
    }
}

static VOID btmw_rpc_test_gattc_handle_get_db_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_GET_GATT_DB_DATA *get_db_data)
{
    if (NULL == get_db_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd(" addr:%s, is_first:%d, attr_cnt:%d",
        get_db_data->addr,
        get_db_data->is_first, get_db_data->attr_cnt);

    if (get_db_data->is_first)
    {
        BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
        btmw_rpc_test_gattc_free_server(gattc_client, get_db_data->addr);
        server = calloc(1, sizeof(BTMW_RPC_TEST_GATTC_SERVER));
        if (NULL == server)
        {
            BTMW_RPC_TEST_Logw("alloc server fail");
            return;
        }
        dl_list_init(&server->service_list);
        dl_list_init(&server->node);
        strncpy(server->addr, get_db_data->addr, MAX_BDADDR_LEN - 1);
        server->addr[MAX_BDADDR_LEN - 1] = '\0';
        dl_list_add_tail(&gattc_client->server_list, &server->node);
    }

    btmw_rpc_test_gattc_add_db_in_server(gattc_client, get_db_data);

    return;
}

static VOID btmw_rpc_test_gattc_handle_bt_off_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client)
{
    if (NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd(" client_if:%d, name:%s",
        gattc_client->client_if, gattc_client->client_name);

    btmw_rpc_test_gattc_free_client_by_if(gattc_client->client_if);

    return;
}


static VOID btmw_rpc_test_gattc_handle_phy_update_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_PHY_UPDATE_DATA *phy_update_data)
{
    if (NULL == phy_update_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, rx_phy:%d, tx_phy:%d",
        phy_update_data->status, phy_update_data->addr,
        phy_update_data->rx_phy, phy_update_data->tx_phy);
    gattc_client->status = phy_update_data->status;

    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, phy_update_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->rx_phy = phy_update_data->rx_phy;
            conn->tx_phy = phy_update_data->tx_phy;
            return;
        }
    }
    return;
}

static VOID btmw_rpc_test_gattc_handle_phy_read_event(
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client,
    BT_GATTC_EVENT_READ_PHY_DATA *phy_read_data)
{
    if (NULL == phy_read_data || NULL == gattc_client)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, rx_phy:%d, tx_phy:%d",
        phy_read_data->status, phy_read_data->addr,
        phy_read_data->rx_phy, phy_read_data->tx_phy);
    gattc_client->status = phy_read_data->status;

    BTMW_RPC_TEST_GATTC_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gattc_client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, phy_read_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->rx_phy = phy_read_data->rx_phy;
            conn->tx_phy = phy_read_data->tx_phy;
            return;
        }
    }
    return;
}

static VOID btmw_rpc_test_gattc_event_handle(BT_GATTC_EVENT_PARAM *param, VOID *pv_tag)
{
    BTMW_RPC_TEST_GATTC_CLIENT *gattc_client = NULL;

    if (NULL != pv_tag)
    {
        gattc_client = (BTMW_RPC_TEST_GATTC_CLIENT *)pv_tag;
    }

    if (NULL == gattc_client)
    {
        if (param->event == BT_GATTC_EVENT_REGISTER)
        {
            INT32 index = 0;
            index = btmw_rpc_test_gattc_find_client_by_name(param->data.register_data.client_name);
            if (index < 0)
            {
                BTMW_RPC_TEST_Logd("no client %d \n", param->client_if);
                return;
            }
            gattc_client = &s_btmw_rpc_gattc_clients[index];
        }
        else
        {
            gattc_client = btmw_rpc_test_gattc_find_client_by_if(param->client_if);
        }
        if (NULL == gattc_client)
        {
            BTMW_RPC_TEST_Logd("no client %d \n", param->client_if);
            return;
        }
    }

    if (0 == gattc_client->tp_test)
    {
        BTMW_RPC_TEST_Logd("client_if:%d(%s), event: %s, size:%d\n",
            param->client_if, gattc_client->client_name,
            btmw_rpc_test_gattc_get_event_str(param->event),
            sizeof(BT_GATTC_EVENT_PARAM));
    }

    BTMW_RPC_TEST_GATTC_LOCK(btmw_rpc_test_gattc_lock);

    switch(param->event)
    {
        case BT_GATTC_EVENT_REGISTER:
            btmw_rpc_test_gattc_handle_register_event(gattc_client,
                &param->data.register_data, param->client_if);
            break;
        case BT_GATTC_EVENT_CONNECTION:
            btmw_rpc_test_gattc_handle_connection_event(gattc_client,
                &param->data.connection_data);
            break;
        case BT_GATTC_EVENT_NOTIFY:
            btmw_rpc_test_gattc_handle_notif_event(gattc_client,
                &param->data.notif_data);
            break;
        case BT_GATTC_EVENT_READ_CHAR_RSP:
            btmw_rpc_test_gattc_handle_read_char_rsp_event(gattc_client,
                &param->data.read_char_rsp_data);
            break;
        case BT_GATTC_EVENT_WRITE_CHAR_RSP:
            btmw_rpc_test_gattc_handle_write_char_rsp_event(gattc_client,
                &param->data.write_char_rsp_data);
            break;
        case BT_GATTC_EVENT_EXEC_WRITE_RSP:
            btmw_rpc_test_gattc_handle_exec_write_rsp_event(gattc_client,
                &param->data.exec_write_rsp_data);
            break;
        case BT_GATTC_EVENT_READ_DESC_RSP:
            btmw_rpc_test_gattc_handle_read_desc_rsp_event(gattc_client,
                &param->data.read_desc_rsp_data);
            break;
        case BT_GATTC_EVENT_WRITE_DESC_RSP:
            btmw_rpc_test_gattc_handle_write_desc_rsp_event(gattc_client,
                &param->data.write_desc_rsp_data);
            break;
        case BT_GATTC_EVENT_READ_RSSI_RSP:
            btmw_rpc_test_gattc_handle_read_rssi_rsp_event(gattc_client,
                &param->data.read_rssi_data);
            break;
        case BT_GATTC_EVENT_MTU_CHANGE:
            btmw_rpc_test_gattc_handle_mtu_change_event(gattc_client,
                &param->data.mtu_chg_data);
            break;
        case BT_GATTC_EVENT_PHY_READ:
            btmw_rpc_test_gattc_handle_phy_read_event(gattc_client,
                &param->data.phy_read_data);
            break;
        case BT_GATTC_EVENT_PHY_UPDATE:
            btmw_rpc_test_gattc_handle_phy_update_event(gattc_client,
                &param->data.phy_upd_data);
            break;
        case BT_GATTC_EVENT_CONN_UPDATE:
            btmw_rpc_test_gattc_handle_conn_update_event(gattc_client,
                &param->data.conn_upd_data);
            break;
        case BT_GATTC_EVENT_GET_GATT_DB:
            btmw_rpc_test_gattc_handle_get_db_event(gattc_client,
                &param->data.get_gatt_db_data);
            break;
        case BT_GATTC_EVENT_MAX:
            btmw_rpc_test_gattc_handle_bt_off_event(gattc_client);
            break;
        default:
            break;
    }
    BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
    if (NULL != gattc_client)
    {
        BTMW_RPC_TEST_GATTC_SIGNAL(gattc_client->lock,
            gattc_client->signal,
            gattc_client->event_mask |= (1 << param->event));
    }

    return;
}

static int btmw_rpc_test_gattc_register_client_by_name(char *client_name)
{
    INT32 index = 0;
    INT32 ret = 0;
    if (NULL == client_name)
    {
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(client_name);
    if (index >= 0)
    {
        BTMW_RPC_TEST_Logi("[GATTC] cient %s registered, client_if:%d\n",
            client_name, s_btmw_rpc_gattc_clients[index].client_if);
        return -1;
    }

    index = btmw_rpc_test_gattc_alloc_client();
    if (index < 0)
    {
        BTMW_RPC_TEST_Logi("[GATTC] alloc cient %s fail\n", client_name);
        return -1;
    }
    BTMW_RPC_TEST_Logi("client_name: %s", client_name);

    strncpy(s_btmw_rpc_gattc_clients[index].client_name,
        client_name, BT_GATT_MAX_UUID_LEN - 1);
    s_btmw_rpc_gattc_clients[index].client_name[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    s_btmw_rpc_gattc_clients[index].event_mask &= ~(1 << BT_GATTC_EVENT_REGISTER);

    if (BT_SUCCESS != (ret = a_mtkapi_bt_gattc_register(
        s_btmw_rpc_gattc_clients[index].client_name,
        btmw_rpc_test_gattc_event_handle,
        &s_btmw_rpc_gattc_clients[index])))
    {
        BTMW_RPC_TEST_Logd("register client fail %d, free it\n", ret);
        btmw_rpc_test_gattc_free_client_by_index(index);

        return -1;
    }

    return index;
}

static int btmw_rpc_test_gattc_register_client(int argc, char **argv)
{
    return btmw_rpc_test_gattc_register_client_by_name(argv[0]);
}

static int btmw_rpc_test_gattc_unregister_client_by_name(char *client_name)
{
    INT32 index = 0, ret = 0;
    index = btmw_rpc_test_gattc_find_client_by_name(client_name);

    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", client_name);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", client_name);
        return 0;
    }

    BTMW_RPC_TEST_Logd("unregister client: %s, client_if: %d\n",
        client_name, s_btmw_rpc_gattc_clients[index].client_if);

    ret = a_mtkapi_bt_gattc_unregister(s_btmw_rpc_gattc_clients[index].client_if);
    if (BT_SUCCESS != ret)
    {
        BTMW_RPC_TEST_Logw("client %s unregister fail(%d)", client_name, ret);
    }

    BTMW_RPC_TEST_GATTC_LOCK(btmw_rpc_test_gattc_lock);
    btmw_rpc_test_gattc_free_client_by_index(index);
    BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);

    return 0;
}

static int btmw_rpc_test_gattc_unregister_client(int argc, char **argv)
{
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC unregister_client <name>\n");
        return -1;
    }

    return btmw_rpc_test_gattc_unregister_client_by_name(argv[0]);
}

static int btmw_rpc_test_gattc_connect(int argc, char **argv)
{
    BT_GATTC_CONNECT_PARAM conn_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC connect <client_name> <addr> [<bg|direct> [<auto|bredr|ble> [<initor_phy:0xHH> [<opportunitistic>]]]]\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }
    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&conn_param, 0, sizeof(conn_param));
    conn_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(conn_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    conn_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    if (argc >= 3)
    {
        conn_param.is_direct = btmw_rpc_test_gattc_parse_connect_mode(argv[2]);
    }
    else
    {
        conn_param.is_direct = TRUE;
    }

    if (argc >= 4)
    {
        conn_param.transport = btmw_rpc_test_gattc_parse_transport(argv[3]);
    }
    else
    {
        conn_param.transport = GATT_TRANSPORT_TYPE_LE;
    }

    if (argc >= 5)
    {
        conn_param.initiating_phys = btmw_rpc_test_gattc_parse_value(argv[4]);
    }
    else
    {
        conn_param.initiating_phys = 0x07;
    }

    if (argc >= 6)
    {
        conn_param.opportunistic = TRUE;
    }
    else
    {
        conn_param.opportunistic = FALSE;
    }

    ret = a_mtkapi_bt_gattc_connect(&conn_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s connect %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_disconnect(int argc, char **argv)
{
    BT_GATTC_DISCONNECT_PARAM disc_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC disconnect <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&disc_param, 0, sizeof(disc_param));
    disc_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(disc_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    disc_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_disconnect(&disc_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s disconnect %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_refresh(int argc, char **argv)
{
    BT_GATTC_REFRESH_PARAM refresh_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC refresh <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&refresh_param, 0, sizeof(refresh_param));
    refresh_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(refresh_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    refresh_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_refresh(&refresh_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s refresh %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

#if 0
static int btmw_rpc_test_gattc_discover_by_uuid(int argc, char **argv)
{
    BT_GATTC_DISCOVER_BY_UUID_PARAM disc_by_uuid_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC discover_by_uuid <client_name> <addr> <uuid>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&disc_by_uuid_param, 0, sizeof(disc_by_uuid_param));
    disc_by_uuid_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(disc_by_uuid_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    disc_by_uuid_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(disc_by_uuid_param.service_uuid, argv[2], BT_GATT_MAX_UUID_LEN - 1);
    disc_by_uuid_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_discover_by_uuid(&disc_by_uuid_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s discover %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}
#endif

static int btmw_rpc_test_gattc_read_char(int argc, char **argv)
{
    BT_GATTC_READ_CHAR_PARAM read_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_char_by_handle <client_name> <addr> <handle> <default:none|no-mitm|mitm>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&read_char_param, 0, sizeof(read_char_param));
    read_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(read_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    read_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    read_char_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    read_char_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);

    if (0 == read_char_param.handle)
    {
        BTMW_RPC_TEST_Logw("service %s char %s not found in %s services",
            argv[2], argv[3], argv[1]);
        return 0;
    }

    ret = a_mtkapi_bt_gattc_read_char(&read_char_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read char %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}


static int btmw_rpc_test_gattc_read_char_by_uuid(int argc, char **argv)
{
    BT_GATTC_READ_BY_UUID_PARAM read_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 6)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_char_by_uuid <client_name> <addr> <char_uuid> <start_handle> <end_handle> <default:none|no-mitm|mitm>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&read_char_param, 0, sizeof(read_char_param));
    read_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(read_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    read_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    strncpy(read_char_param.char_uuid, argv[2], BT_GATT_MAX_UUID_LEN - 1);
    read_char_param.char_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    read_char_param.s_handle = btmw_rpc_test_gattc_parse_value(argv[3]);
    read_char_param.e_handle = btmw_rpc_test_gattc_parse_value(argv[4]);
    read_char_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[5]);

    if (0 == read_char_param.s_handle || 0 == read_char_param.e_handle)
    {
        BTMW_RPC_TEST_Logw("invalid handle, s_handle=%d, e_handle=%d",
            read_char_param.s_handle, read_char_param.e_handle);
        return 0;
    }

    ret = a_mtkapi_bt_gattc_read_char_by_uuid(&read_char_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read char %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_read_desc(int argc, char **argv)
{
    BT_GATTC_READ_DESC_PARAM read_desc_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_desc_by_handle <client_name> <addr> <handle> <default:none|no-mitm|mitm>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&read_desc_param, 0, sizeof(read_desc_param));
    read_desc_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(read_desc_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    read_desc_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    read_desc_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    read_desc_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);

    if (0 == read_desc_param.handle)
    {
        BTMW_RPC_TEST_Logw("service %s char %s desc %s not found in %s services",
            argv[2], argv[3], argv[4], argv[1]);
        return 0;
    }

    ret = a_mtkapi_bt_gattc_read_desc(&read_desc_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read desc %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_cmd_write_char(int argc, char **argv)
{
    BT_GATTC_WRITE_CHAR_PARAM write_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC cmd_write_char_by_handle <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&write_char_param, 0, sizeof(write_char_param));
    write_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(write_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    write_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_char_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    write_char_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);

    if (0 == write_char_param.handle)
    {
        BTMW_RPC_TEST_Logw("service %s char %s not found in %s services",
            argv[2], argv[3],  argv[1]);
        return 0;
    }
    write_char_param.write_type = BT_GATTC_WRITE_TYPE_CMD;

    gattc_char = btmw_rpc_test_gattc_find_char_by_handle(
        &s_btmw_rpc_gattc_clients[index], argv[1], write_char_param.handle);

    write_char_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
    memcpy((void*)write_char_param.value, (void*)argv[4], write_char_param.value_len);
    if (NULL != gattc_char)
    {
        memcpy((void*)gattc_char->value, (void*)argv[4], write_char_param.value_len);
        gattc_char->len = write_char_param.value_len;
    }

    ret = a_mtkapi_bt_gattc_write_char(&write_char_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s write char %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_reliable_write_char(int argc, char **argv)
{
    BT_GATTC_WRITE_CHAR_PARAM write_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC reliable_write_char <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&write_char_param, 0, sizeof(write_char_param));
    write_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(write_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    write_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_char_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);

    write_char_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);

    if (0 == write_char_param.handle)
    {
        BTMW_RPC_TEST_Logw("service %s char %s not found in %s services",
            argv[2], argv[3], argv[1]);
        return 0;
    }
    write_char_param.write_type = BT_GATTC_WRITE_TYPE_PREPARE;

    gattc_char = btmw_rpc_test_gattc_find_char_by_handle(
        &s_btmw_rpc_gattc_clients[index], argv[1], write_char_param.handle);

    write_char_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
    memcpy((void*)write_char_param.value, (void*)argv[4], write_char_param.value_len);
    if (gattc_char != NULL)
    {
        memcpy((void*)gattc_char->value, (void*)argv[4], write_char_param.value_len);
        gattc_char->len = write_char_param.value_len;
    }

    ret = a_mtkapi_bt_gattc_write_char(&write_char_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s write char %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_exec_write_char(int argc, char **argv)
{
    BT_GATTC_EXEC_WRITE_PARAM exec_write_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC exec_write_char <client_name> <addr> <cancel|exec>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&exec_write_param, 0, sizeof(exec_write_param));
    exec_write_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(exec_write_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    exec_write_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    exec_write_param.execute = btmw_rpc_test_gattc_parse_excute_mode(argv[2]);

    ret = a_mtkapi_bt_gattc_exec_write(&exec_write_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s exec write %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}


static int btmw_rpc_test_gattc_write_char(int argc, char **argv)
{
    BT_GATTC_WRITE_CHAR_PARAM write_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC write_char <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&write_char_param, 0, sizeof(write_char_param));
    write_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(write_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    write_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_char_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    write_char_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);
    if (0 == write_char_param.handle)
    {
        BTMW_RPC_TEST_Logw("handle should be 0");
        return 0;
    }
    write_char_param.write_type = BT_GATTC_WRITE_TYPE_REQ;

    gattc_char = btmw_rpc_test_gattc_find_char_by_handle(
        &s_btmw_rpc_gattc_clients[index], argv[1], write_char_param.handle);

    write_char_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
    memcpy((void*)write_char_param.value, (void*)argv[4], write_char_param.value_len);
    if (gattc_char != NULL)
    {
        memcpy((void*)gattc_char->value, (void*)argv[4], write_char_param.value_len);
        gattc_char->len = write_char_param.value_len;
    }

    ret = a_mtkapi_bt_gattc_write_char(&write_char_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s write char %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_write_desc(int argc, char **argv)
{
    BT_GATTC_WRITE_DESC_PARAM write_desc_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC write_desc <client_name> <addr> <handle> <default:none|no-mitm|mitm> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&write_desc_param, 0, sizeof(write_desc_param));
    write_desc_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(write_desc_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    write_desc_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_desc_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    write_desc_param.auth_req = btmw_rpc_test_gattc_parse_auth_req(argv[3]);

    if (0 == write_desc_param.handle)
    {
        BTMW_RPC_TEST_Logw("handle should be 0");
        return 0;
    }

    gattc_desc = btmw_rpc_test_gattc_find_desc_by_handle(
        &s_btmw_rpc_gattc_clients[index], argv[1], write_desc_param.handle);

    write_desc_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
    memcpy((void*)write_desc_param.value, (void*)argv[4], write_desc_param.value_len);
    if (gattc_desc != NULL)
    {
        memcpy((void*)gattc_desc->value, (void*)argv[4], write_desc_param.value_len);
        gattc_desc->len = write_desc_param.value_len;
    }

    ret = a_mtkapi_bt_gattc_write_desc(&write_desc_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s write desc %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_reg_char_notify(int argc, char **argv)
{
    BT_GATTC_REG_NOTIF_PARAM reg_char_notify_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC reg_char_notify <client_name> <addr> <handle> <0:unregister, 1:register>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }


    memset((void*)&reg_char_notify_param, 0, sizeof(reg_char_notify_param));
    reg_char_notify_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(reg_char_notify_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    reg_char_notify_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    reg_char_notify_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    reg_char_notify_param.registered = btmw_rpc_test_gattc_parse_register_mode(argv[3]);

    gattc_char = btmw_rpc_test_gattc_find_char_by_handle(
        &s_btmw_rpc_gattc_clients[index], argv[1], reg_char_notify_param.handle);
    if (NULL != gattc_char)
    {
        gattc_char->reg_notify = reg_char_notify_param.registered;
    }

    ret = a_mtkapi_bt_gattc_reg_notification(&reg_char_notify_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s reg notify %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_read_rssi(int argc, char **argv)
{
    BT_GATTC_READ_RSSI_PARAM read_rssi_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_rssi <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&read_rssi_param, 0, sizeof(read_rssi_param));
    read_rssi_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(read_rssi_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    read_rssi_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_read_rssi(&read_rssi_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_get_dev_type(int argc, char **argv)
{
    BT_GATTC_GET_DEV_TYPE_PARAM get_dev_type_param;
    INT32 index = 0;
    INT32 dev_type = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_rssi <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&get_dev_type_param, 0, sizeof(get_dev_type_param));
    get_dev_type_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(get_dev_type_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    get_dev_type_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    dev_type = a_mtkapi_bt_gattc_get_dev_type(&get_dev_type_param);

    BTMW_RPC_TEST_Logi("device %s type = %d", argv[0], dev_type);
    return 0;
}

static int btmw_rpc_test_gattc_change_mtu(int argc, char **argv)
{
    BT_GATTC_CHG_MTU_PARAM chg_mtu_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC change_mtu <client_name> <addr> <mtu>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&chg_mtu_param, 0, sizeof(chg_mtu_param));
    chg_mtu_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(chg_mtu_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    chg_mtu_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    chg_mtu_param.mtu = btmw_rpc_test_gattc_parse_value(argv[2]);

    ret = a_mtkapi_bt_gattc_change_mtu(&chg_mtu_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_conn_update(int argc, char **argv)
{
    BT_GATTC_CONN_UPDATE_PARAM conn_upd_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 8)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC conn_update <client_name> <addr> <min_int> <max_int> <latency> <timeout> <min_ce> <max_ce>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&conn_upd_param, 0, sizeof(conn_upd_param));
    conn_upd_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(conn_upd_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    conn_upd_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_upd_param.min_interval = btmw_rpc_test_gattc_parse_value(argv[2]);
    conn_upd_param.max_interval = btmw_rpc_test_gattc_parse_value(argv[3]);
    conn_upd_param.latency = btmw_rpc_test_gattc_parse_value(argv[4]);
    conn_upd_param.timeout = btmw_rpc_test_gattc_parse_value(argv[5]);
    conn_upd_param.min_ce_len = btmw_rpc_test_gattc_parse_value(argv[6]);
    conn_upd_param.max_ce_len = btmw_rpc_test_gattc_parse_value(argv[7]);

    ret = a_mtkapi_bt_gattc_conn_update(&conn_upd_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_read_phy(int argc, char **argv)
{
    BT_GATTC_PHY_READ_PARAM phy_read_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_phy <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&phy_read_param, 0, sizeof(phy_read_param));
    phy_read_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(phy_read_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    phy_read_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_read_phy(&phy_read_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_set_pref_phy(int argc, char **argv)
{
    BT_GATTC_PHY_SET_PARAM phy_set_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC set_pref_phy <client_name> <addr> <tx_phy> <rx_phy> <phy_option>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&phy_set_param, 0, sizeof(phy_set_param));
    phy_set_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(phy_set_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    phy_set_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    phy_set_param.tx_phy = btmw_rpc_test_gattc_parse_value(argv[2]);
    phy_set_param.rx_phy = btmw_rpc_test_gattc_parse_value(argv[3]);
    phy_set_param.phy_options = btmw_rpc_test_gattc_parse_value(argv[4]);

    ret = a_mtkapi_bt_gattc_set_prefer_phy(&phy_set_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gattc_get_db(int argc, char **argv)
{
    BT_GATTC_GET_GATT_DB_PARAM get_db_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC get_db <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&get_db_param, 0, sizeof(get_db_param));
    get_db_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(get_db_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    get_db_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gattc_get_gatt_db(&get_db_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("client %s read rssi %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static VOID btmw_rpc_test_gattc_show_value(CHAR *title, UINT8 *value, UINT32 value_len)
{
    if (title == NULL || value == NULL || value_len == 0)
    {
        return;
    }
    BTMW_RPC_TEST_Logd("%s, length: %u", title, value_len);

    BTMW_RPC_TEST_Logd("-----------------------hex------------------");
    UINT32 i = 0;
    for (i = 0; i < value_len; i++)
    {
        printf("%02X ", value[i]);
        if (i%16 == 15 || i == value_len-1) printf("\n");
    }
    BTMW_RPC_TEST_Logd("---------------------string----------------");
    BTMW_RPC_TEST_Logd(": %s", value);
    BTMW_RPC_TEST_Logd("-------------------------------------------");

    return;
}

static int btmw_rpc_test_gattc_show_db(int argc, char **argv)
{
    INT32 index = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC get_db <client_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    btmw_rpc_test_gattc_show_server_db(&s_btmw_rpc_gattc_clients[index], argv[1]);
    return 0;
}

static int btmw_rpc_test_gattc_show_char(int argc, char **argv)
{
    INT32 index = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC show_char <client_name> <addr> <handle>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    gattc_char =
        btmw_rpc_test_gattc_find_char_by_handle(&s_btmw_rpc_gattc_clients[index],
            argv[1], atoi(argv[2]));
    if (NULL != gattc_char)
    {
        btmw_rpc_test_gattc_show_value("gatt_char", gattc_char->value, gattc_char->len);
    }
    else
    {
        BTMW_RPC_TEST_Logw("handle %s not found", argv[2]);
    }
    return 0;
}

static int btmw_rpc_test_gattc_show_desc(int argc, char **argv)
{
    INT32 index = 0;
    BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC show_desc <client_name> <addr> <handle>\n");
        return -1;
    }

    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        return 0;
    }

    gattc_desc =
        btmw_rpc_test_gattc_find_desc_by_handle(&s_btmw_rpc_gattc_clients[index],
            argv[1], atoi(argv[2]));
    if (NULL != gattc_desc)
    {
        btmw_rpc_test_gattc_show_value("gatt_desc", gattc_desc->value, gattc_desc->len);
    }
    else
    {
        BTMW_RPC_TEST_Logw("handle %s not found", argv[2]);
    }
    return 0;
}

static int btmw_rpc_test_gattc_throughput_test(int argc, char **argv)
{
    BT_GATTC_WRITE_CHAR_PARAM write_char_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
    BTMW_RPC_TEST_GATTC_CONNECTION *gattc_conn = NULL;
    struct timespec wait_time;
    INT32 wait_ret = 0;
    struct timeval write_char_start;
    struct timeval write_char_done;
    float time_use = 0;
    float tp = 0;
    INT32 mtu = 0, tx_len_per_pkt = 0;
    UINT32 test_length = 0;
    UINT32 percent_cnt = 0, tmp_percent = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC throughput_test <client_name> <addr> <char_handle> <length, unit:k> [<pkt_length, unit:byte> [write_req]]\n");
        return -1;
    }
    BTMW_RPC_TEST_GATTC_LOCK(btmw_rpc_test_gattc_lock);
    index = btmw_rpc_test_gattc_find_client_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found client %s", argv[0]);
        BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
        return 0;
    }

    if (s_btmw_rpc_gattc_clients[index].client_if <= 0)
    {
        BTMW_RPC_TEST_Logw("client %s has not registered", argv[0]);
        BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
        return 0;
    }

    memset((void*)&write_char_param, 0, sizeof(write_char_param));
    write_char_param.client_if = s_btmw_rpc_gattc_clients[index].client_if;
    strncpy(write_char_param.addr, argv[1], MAX_BDADDR_LEN - 1);
    write_char_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_char_param.handle = btmw_rpc_test_gattc_parse_value(argv[2]);
    write_char_param.auth_req = BT_GATTC_AUTH_REQ_NONE;
    if (0 == write_char_param.handle)
    {
        BTMW_RPC_TEST_Logw("service %s char handle %s not found in server %s",
            argv[2], argv[3],  argv[1]);
        BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
        return 0;
    }
    gattc_char = btmw_rpc_test_gattc_find_char_by_handle(
        &s_btmw_rpc_gattc_clients[index],
        write_char_param.addr,
        write_char_param.handle);
    if (NULL == gattc_char)
    {
        BTMW_RPC_TEST_Logw("server %s char handle %d not found",
            write_char_param.addr, write_char_param.handle);
        BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
        return 0;
    }
    gattc_conn = btmw_rpc_test_gattc_find_connection(&s_btmw_rpc_gattc_clients[index],
        write_char_param.addr);
    if (NULL == gattc_conn)
    {
        BTMW_RPC_TEST_Logw("server %s not connected", write_char_param.addr);
        BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);
        return 0;
    }
    mtu = gattc_conn->mtu;

    BTMW_RPC_TEST_GATTC_UNLOCK(btmw_rpc_test_gattc_lock);

    if (mtu == 0) mtu = BT_GATT_DEFAULT_MTU_SIZE;

    if (argc > 4)
    {
        tx_len_per_pkt = btmw_rpc_test_gattc_parse_value(argv[4]);
    }

    if (tx_len_per_pkt == 0)
    {
        tx_len_per_pkt = mtu - 3; /* GATT_HDR_SIZE */
    }

    if (argc > 5)
    {
        write_char_param.write_type = BT_GATTC_WRITE_TYPE_REQ;
    }
    else
    {
        write_char_param.write_type = BT_GATTC_WRITE_TYPE_CMD;
    }

    test_length = gattc_char->test_length =
        btmw_rpc_test_gattc_parse_value(argv[3]) * 1024;
    if (0 == gattc_char->test_length)
    {
        BTMW_RPC_TEST_Logw("test_length should be 0");
        return 0;
    }

    BTMW_RPC_TEST_Logi("start throughput: %d KBytes, MTU=%d, pkt_size=%d, write_type=%d",
        test_length / 1024, mtu, tx_len_per_pkt, write_char_param.write_type);

    s_btmw_rpc_gattc_clients[index].tp_test = 1;

    gettimeofday(&write_char_start, NULL);

    pthread_mutex_lock(&gattc_char->lock);
    gattc_char->pkt_cnt = 0;
    do
    {
        if (gattc_char->test_length > tx_len_per_pkt)
        {
            write_char_param.value_len = tx_len_per_pkt;
        }
        else
        {
            write_char_param.value_len = gattc_char->test_length;
        }

        memset((void*)write_char_param.value, 0xaa, write_char_param.value_len);

        if (gattc_char->pkt_cnt == 0)
        {
            BTMW_RPC_TEST_Logi("throughput write 1st pkt");
        }
        ret = a_mtkapi_bt_gattc_write_char(&write_char_param);
        if (ret != 0)
        {
            BTMW_RPC_TEST_Logi("throughput write fail(%d)", ret);
            break;
        }
        if (gattc_char->pkt_cnt == 0)
        {
            BTMW_RPC_TEST_Logi("throughput write 1st pkt done");
        }

        memset(&wait_time, 0, sizeof(wait_time));
        clock_gettime(CLOCK_MONOTONIC, &wait_time);
        wait_time.tv_sec += BTMW_RPC_TEST_GATTC_MAX_WRITTEN_TIME_SECOND;
        wait_ret = pthread_cond_timedwait(&gattc_char->signal,
            &gattc_char->lock, &wait_time);
        if (wait_ret == ETIMEDOUT)
        {
            BTMW_RPC_TEST_Logi("throughput test timeout!!!");
            break;
        }
        gattc_char->pkt_cnt++;
        gattc_char->test_length -= write_char_param.value_len;

        tmp_percent = (test_length - gattc_char->test_length) * 100 / test_length;

        if (tmp_percent != percent_cnt)
        {
            BTMW_RPC_TEST_Logi("handle %d progress: %2d%%",
                write_char_param.handle, tmp_percent);

            percent_cnt = tmp_percent;
        }
#if 0
        if (gattc_conn->connected == 0)
        {
            BTMW_RPC_TEST_Logi("throughput test connection disconnected");
            break;
        }
#endif

    } while (gattc_char->test_length > 0);
    pthread_mutex_unlock(&gattc_char->lock);

    if (wait_ret != ETIMEDOUT)
    {
        gettimeofday(&write_char_done, NULL);

        time_use = (write_char_done.tv_sec - write_char_start.tv_sec) * 1000000
            + (write_char_done.tv_usec - write_char_start.tv_usec);
        time_use /= 1000000;
        if (time_use > 0.000001 || time_use < -0.000001)
        {
            tp = test_length / 1024 / time_use * 8;
        }
        else
        {
            tp = 0;
        }
        BTMW_RPC_TEST_Logi("Data_len: %d Kbytes, %d pkt, time: %f sec, throughput: %f Kbps, MTU: %d, TX PHY: %d",
            test_length / 1024, gattc_char->pkt_cnt,
            time_use, tp, gattc_conn->mtu, gattc_conn->tx_phy);
    }
    s_btmw_rpc_gattc_clients[index].tp_test = 0;

    return 0;
}

static int btmw_rpc_test_gattc_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_gattc_cli_commands;

    //BTMW_RPC_TEST_Logi("[GATTC] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logi("[GATTC] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GATTC, btmw_rpc_test_gattc_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_gattc_init(void)
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    int ret = 0;
    BTMW_RPC_TEST_MOD gattc_mod = {0};
    int i = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&btmw_rpc_test_gattc_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    // Register command to CLI
    gattc_mod.mod_id = BTMW_RPC_TEST_MOD_GATT_CLIENT;
    strncpy(gattc_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GATTC, sizeof(gattc_mod.cmd_key));
    gattc_mod.cmd_handler = btmw_rpc_test_gattc_cmd_handler;
    gattc_mod.cmd_tbl = btmw_rpc_test_gattc_cli_commands;
    ret = btmw_rpc_test_register_mod(&gattc_mod);
    BTMW_RPC_TEST_Logd("[GATTC] btmw_rpc_test_register_mod() returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        a_mtkapi_bt_gatt_profile_init();
    }

    for(i=0;i<BTMW_RPC_TEST_GATTC_TEST_APP_CNT;i++)
    {
        memset((void*)&s_btmw_rpc_gattc_clients[i], 0,
            sizeof(s_btmw_rpc_gattc_clients[i]));
        dl_list_init(&s_btmw_rpc_gattc_clients[i].server_list);
        dl_list_init(&s_btmw_rpc_gattc_clients[i].connection_list);
        pthread_cond_init(&s_btmw_rpc_gattc_clients[i].signal, NULL);
        pthread_mutex_init(&s_btmw_rpc_gattc_clients[i].lock, NULL);
    }

    return ret;
}

int btmw_rpc_test_gattc_deinit(void)
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}


static BTMW_RPC_TEST_GATTC_CONNECTION *btmw_rpc_test_gattc_find_connection(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr)
{
    BTMW_RPC_TEST_GATTC_CONNECTION *gatt_conn = NULL;
    if (NULL == client || NULL == server_addr)
    {
        return NULL;
    }

    dl_list_for_each(gatt_conn, &client->connection_list,
        BTMW_RPC_TEST_GATTC_CONNECTION, node)
    {
        if (strncasecmp(gatt_conn->addr, server_addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }

        return gatt_conn;
    }
    return NULL;
}


static BTMW_RPC_TEST_GATTC_CHAR *btmw_rpc_test_gattc_find_char_by_handle(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr, UINT32 handle)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    if (NULL == client || NULL == server_addr)
    {
        return NULL;
    }

    dl_list_for_each(server, &client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (strncasecmp(server->addr, server_addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }

        BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
        dl_list_for_each(service, &server->service_list,
            BTMW_RPC_TEST_GATTC_SERVICE, node)
        {
            BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
            dl_list_for_each(gattc_char, &service->char_list,
                BTMW_RPC_TEST_GATTC_CHAR, node)
            {
                if(gattc_char->handle == handle)
                {
                    return gattc_char;
                }
            }
        }
        return NULL;
    }
    return NULL;
}

static BTMW_RPC_TEST_GATTC_DESC * btmw_rpc_test_gattc_find_desc_by_handle(
    BTMW_RPC_TEST_GATTC_CLIENT *client, CHAR *server_addr, UINT32 handle)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;

    if (NULL == client || NULL == server_addr)
    {
        return NULL;
    }

    dl_list_for_each(server, &client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        if (strncmp(server->addr, server_addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }

        BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
        dl_list_for_each(service, &server->service_list,
            BTMW_RPC_TEST_GATTC_SERVICE, node)
        {
            BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
            dl_list_for_each(gattc_char, &service->char_list,
                BTMW_RPC_TEST_GATTC_CHAR, node)
            {

                BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
                dl_list_for_each(gattc_desc, &gattc_char->desc_list,
                    BTMW_RPC_TEST_GATTC_DESC, node)
                {
                    if(gattc_desc->handle == handle)
                    {
                        return gattc_desc;
                    }
                }
            }
        }
    }
    return NULL;
}

static VOID btmw_rpc_test_gattc_show_server_db(
    BTMW_RPC_TEST_GATTC_CLIENT *client,
    CHAR *addr)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    CHAR prop_str[128];
    if (NULL == addr || NULL == client)
    {
        return;
    }

    BTMW_RPC_TEST_Logi("client:%d, addr:%s\n", client->client_if, addr);

    dl_list_for_each(server, &client->server_list,
        BTMW_RPC_TEST_GATTC_SERVER, node)
    {
        //BTMW_RPC_TEST_Logi("server:%s\n", server->addr);
        if (strncasecmp(server->addr, addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }

        BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
        dl_list_for_each(service, &server->service_list,
            BTMW_RPC_TEST_GATTC_SERVICE, node)
        {
            BTMW_RPC_TEST_Logi("[GATTC][Service] type=%d handle=%d uuid=%s\n",
                service->type, service->handle, service->uuid);
            BTMW_RPC_TEST_GATTC_INCLUDE *gattc_include = NULL;
            dl_list_for_each(gattc_include, &service->include_list,
                BTMW_RPC_TEST_GATTC_INCLUDE, node)
            {
                BTMW_RPC_TEST_Logi("  [GATTC][Include] handle=%d uuid=%s\n",
                    gattc_include->handle, gattc_include->uuid);
            }

            BTMW_RPC_TEST_GATTC_CHAR *gattc_char = NULL;
            dl_list_for_each(gattc_char, &service->char_list,
                BTMW_RPC_TEST_GATTC_CHAR, node)
            {
                BTMW_RPC_TEST_Logi("  [GATTC][Char] handle=%d uuid=%s prop=0x%02x(%s)\n",
                    gattc_char->handle, gattc_char->uuid, gattc_char->prop,
                    btmw_rpc_test_gattc_get_char_prop_str(gattc_char->prop, prop_str));
                BTMW_RPC_TEST_GATTC_DESC *gattc_desc = NULL;
                dl_list_for_each(gattc_desc, &gattc_char->desc_list,
                    BTMW_RPC_TEST_GATTC_DESC, node)
                {
                    BTMW_RPC_TEST_Logi("    [GATTC][Desc] handle=%d uuid=%s\n",
                        gattc_desc->handle, gattc_desc->uuid);
                }
            }
        }
        return;
    }
}

static INT32 btmw_rpc_test_gattc_alloc_client(void)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTC_TEST_APP_CNT; i ++)
    {
        if (0 == s_btmw_rpc_gattc_clients[i].client_if)
        {
            return i;
        }
    }

    return -1;
}

static INT32 btmw_rpc_test_gattc_find_client_by_name(char *client_name)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTC_TEST_APP_CNT; i ++)
    {
        if (!strncasecmp(s_btmw_rpc_gattc_clients[i].client_name,
            client_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            return i;
        }
    }

    return -1;
}

static BTMW_RPC_TEST_GATTC_CLIENT* btmw_rpc_test_gattc_find_client_by_if(INT32 client_if)
{
    INT32 i = 0;

    if (0 == client_if) return NULL;

    for (i = 0; i < BTMW_RPC_TEST_GATTC_TEST_APP_CNT; i ++)
    {
        if (client_if == s_btmw_rpc_gattc_clients[i].client_if)
        {
            return &s_btmw_rpc_gattc_clients[i];
        }
    }

    return NULL;
}

VOID btmw_rpc_test_gattc_free_client(BTMW_RPC_TEST_GATTC_CLIENT *gattc_client)
{
    BTMW_RPC_TEST_GATTC_SERVER *server = NULL;
    BTMW_RPC_TEST_GATTC_CONNECTION *gatt_conn = NULL;

    if (NULL == gattc_client)
    {
        return;
    }

    while (!dl_list_empty(&gattc_client->server_list))
    {
        server = dl_list_first(&gattc_client->server_list, BTMW_RPC_TEST_GATTC_SERVER, node);
        if (server != NULL)
        {
            /* free service */
            BTMW_RPC_TEST_GATTC_SERVICE *service = NULL;
            while (!dl_list_empty(&server->service_list))
            {
                service = dl_list_first(&server->service_list, BTMW_RPC_TEST_GATTC_SERVICE, node);
                if (NULL != service)
                {
                    BTMW_RPC_TEST_Logi("free service %s ", service->uuid);
                    dl_list_del(&service->node);

                    btmw_rpc_test_gattc_free_service(service);
                    free(service);
                    service = NULL;
                }
            }
            dl_list_del(&server->node);
            free(server);
            server = NULL;
        }
    }

    while (!dl_list_empty(&gattc_client->connection_list))
    {
        gatt_conn = dl_list_first(&gattc_client->connection_list, BTMW_RPC_TEST_GATTC_CONNECTION, node);
        if (gatt_conn != NULL)
        {
            dl_list_del(&gatt_conn->node);
            free(gatt_conn);
            gatt_conn = NULL;
        }
    }

    gattc_client->event_mask = 0;
    gattc_client->status = 0;
    gattc_client->client_if = 0;
    gattc_client->client_name[0] = '\0';
    gattc_client->tp_test = 0;

    return;
}

VOID btmw_rpc_test_gattc_free_client_by_if(INT32 client_if)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTC_TEST_APP_CNT; i ++)
    {
        if (client_if == s_btmw_rpc_gattc_clients[i].client_if)
        {
            btmw_rpc_test_gattc_free_client(&s_btmw_rpc_gattc_clients[i]);
            return;
        }
    }

    return;
}

VOID btmw_rpc_test_gattc_free_client_by_index(INT32 index)
{
    if (index < 0 || index >= BTMW_RPC_TEST_GATTC_TEST_APP_CNT) return;

    btmw_rpc_test_gattc_free_client(&s_btmw_rpc_gattc_clients[index]);

    return;
}

static CHAR* btmw_rpc_test_gattc_get_event_str(BT_GATTC_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_REGISTER, "register");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_CONNECTION, "connection");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_NOTIFY, "notify");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_READ_CHAR_RSP, "read_char_rsp");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_WRITE_CHAR_RSP, "write_char_rsp");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_EXEC_WRITE_RSP, "exec_write_rsp");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_READ_DESC_RSP, "read_desc_rsp");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_READ_RSSI_RSP, "read_rssi_rsp");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_MTU_CHANGE, "mtu_change");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_PHY_READ, "phy_read");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_GET_GATT_DB, "get_gatt_db");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_PHY_UPDATE, "phy_update");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_CONN_UPDATE, "conn_update");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_DISCOVER_COMPL, "discover_complete");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATTC_EVENT_MAX, "bt_off");
        default: return "UNKNOWN_EVENT";
   }
}

static CHAR* btmw_rpc_test_gattc_get_status_str(BT_GATT_STATUS status)
{
    switch((int)status)
    {
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_OK, "ok");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INVALID_HANDLE, "invalid handle");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_READ_NOT_PERMIT, "read not permit");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_WRITE_NOT_PERMIT, "write not permit");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INVALID_PDU, "invalid pdu");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INSUF_AUTHENTICATION, "insuf authen");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_REQ_NOT_SUPPORTED, "req not support");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INVALID_OFFSET, "invalid offset");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INSUF_AUTHORIZATION, "insuf author");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_PREPARE_Q_FULL, "prepare q full");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_NOT_FOUND, "not found");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_NOT_LONG, "not long");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INSUF_KEY_SIZE, "insuf key size");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INVALID_ATTR_LEN, "invalid attr len");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_ERR_UNLIKELY, "unlikely");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INSUF_ENCRYPTION, "insuf encry");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_UNSUPPORT_GRP_TYPE, "unsupport grp type");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INSUF_RESOURCE, "insuf resource");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_ILLEGAL_PARAMETER, "illigeal param/pending");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_NO_RESOURCES, "no resource");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INTERNAL_ERROR, "internal err");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_WRONG_STATE, "wrong state");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_DB_FULL, "db full");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_BUSY, "busy");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_ERROR, "err");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_CMD_STARTED, "cmd started");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_AUTH_FAIL, "authen fail");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_MORE, "more");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_INVALID_CFG, "invalid cfg");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_SERVICE_STARTED, "service started");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_ENCRYPED_NO_MITM, "encrypt no mitm");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_NOT_ENCRYPTED, "no encrypt");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_CONGESTED, "congested");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_DUP_REG, "duplicate reg");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_ALREADY_OPEN, "already open");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_CANCEL, "cancel");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_CCC_CFG_ERR, "cfg err");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_PRC_IN_PROGRESS, "in progress");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(BT_GATT_STATUS_OUT_OF_RANGE, "out of range");
        default: return "UNKNOWN_STATUS";
   }
}

static CHAR* btmw_rpc_test_gattc_get_attr_type_str(GATT_ATTR_TYPE type)
{
    switch((int)type)
    {
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(GATT_ATTR_TYPE_PRIMARY_SERVICE, "primary");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(GATT_ATTR_TYPE_SECONDARY_SERVICE, "secondary");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(GATT_ATTR_TYPE_INCLUDED_SERVICE, "include");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(GATT_ATTR_TYPE_CHARACTERISTIC, "char");
        BTMW_RPC_TEST_GATTC_CASE_RETURN_STR(GATT_ATTR_TYPE_DESCRIPTOR, "descriptor");
        default: return "UNKNOWN_TYPE";
   }
}

static CHAR* btmw_rpc_test_gattc_get_char_prop_str(UINT8 properties, CHAR *prop_str)
{
    if (NULL == prop_str) return "none";

    CHAR *property_str[8] =
    {
        "broadcast:", "read:", "write cmd:", "write:", "notify:", "indicate:",
            "signed write:", "extended prop:",
    };
    prop_str[0] = '\0';

    UINT8 i = 0;
    for (i = 0; i < 8; i++)
    {
        if (properties & (1 << i))
        {
            strncat(prop_str, property_str[i], 127 - strlen(prop_str));
        }
    }

    return prop_str;
}

