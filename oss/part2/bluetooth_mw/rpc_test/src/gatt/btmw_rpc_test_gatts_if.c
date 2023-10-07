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

/* FILE NAME:  btmw_rpc_test_gatts_if.c
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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/time.h>

#include "list.h"
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_gatts_if.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatt_wrapper.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define BTMW_RPC_TEST_GATTS_XML_NAME_LEN    (128)

#define BTMW_RPC_TEST_GATTS_TEST_APP_CNT (BT_GATT_MAX_APP_CNT+10)

#define BTMW_RPC_TEST_GATTS_XML_SERVICES   "gatt-services"
#define BTMW_RPC_TEST_GATTS_XML_SERVICE    "gatt-service"
#define BTMW_RPC_TEST_GATTS_XML_INCLUDE    "gatt-include"
#define BTMW_RPC_TEST_GATTS_XML_CHAR       "gatt-char"
#define BTMW_RPC_TEST_GATTS_XML_CHAR_VAL   "gatt-char-value"
#define BTMW_RPC_TEST_GATTS_XML_DESC       "gatt-desc"

/* MACRO FUNCTION DECLARATIONS
 */
#define BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(_const,str) case _const: return str

#define BTMW_RPC_TEST_GATTS_CHECK(expr, value) \
    do { \
    if(!(expr)){ \
        printf("[\033[31m fail \033[0m] %s@%d " #expr ", "#value"=%d\n", __FUNCTION__, __LINE__, value);\
    }\
    else printf("[\033[32m pass \033[0m] %s@%d " #expr "\n", __FUNCTION__, __LINE__);\
    }while(0)

#define BTMW_RPC_TEST_GATTS_LOCK(lock) \
    do { \
        pthread_mutex_lock(&(lock)); \
    }while(0)


#define BTMW_RPC_TEST_GATTS_UNLOCK(lock) \
        do { \
            pthread_mutex_unlock(&(lock));\
        }while(0)

#define BTMW_RPC_TEST_GATTS_SIGNAL(lock, signal, cond) \
    do { \
        pthread_mutex_lock(&(lock));\
        (cond);\
        pthread_cond_signal(&(signal));\
        pthread_mutex_unlock(&(lock));\
    }while(0)


#define BTMW_RPC_TEST_GATTS_WAIT(lock, signal, cond) \
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
    xmlNode *xml_node;

    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    UINT8 perm;
    UINT8 key_size;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 len;
    struct dl_list node; /* link to char */
} BTMW_RPC_TEST_GATTS_DESC;

typedef struct
{
    xmlNode *xml_node; /* it will porint to char value node */

    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    UINT8 prop;
    UINT8 perm;
    UINT8 key_size;
    UINT8 value[BT_GATT_MAX_VALUE_LEN];
    UINT16 len;
    struct dl_list node; /* link to service */
    struct dl_list desc_list;
} BTMW_RPC_TEST_GATTS_CHAR;

typedef struct
{
    xmlNode *xml_node; /* it will porint to char value node */

    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    UINT16 handle;
    struct dl_list node; /* link to service */
} BTMW_RPC_TEST_GATTS_INCLUDE;

typedef struct
{
    xmlNode *xml_node;

    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    GATT_ATTR_TYPE type; /* attr type, pri, sec, include, ...*/
    UINT16 handle;
    INT32 started;  /* 0-not started, 1-started */
    struct dl_list node; /* link to services */
    struct dl_list include_list;
    struct dl_list char_list;
} BTMW_RPC_TEST_GATTS_SERVICE;

typedef struct
{
    INT32 server_if;
    CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
    xmlDoc *doc;
    xmlNode *xml_root;
    struct dl_list node; /* link to server */
    struct dl_list service_list;
} BTMW_RPC_TEST_GATTS_SERVICES;

typedef struct
{
    struct dl_list node; /* link to server */
    CHAR addr[MAX_BDADDR_LEN];
    INT32 mtu;
    INT32 tx_phy;
    INT32 rx_phy;

    INT32 interval;
    INT32 latency;
    INT32 timeout;
} BTMW_RPC_TEST_GATTS_CONNECTION;

typedef struct
{
    struct dl_list node; /* link to server */
    BT_GATTS_EVENT_WRITE_REQ_DATA data;
} BTMW_RPC_TEST_GATTS_WRITE_REQ;


typedef struct
{
    INT32 server_if;
    BT_GATT_STATUS status;
    CHAR server_name[BT_GATT_MAX_NAME_LEN];
    INT32 started;

    struct dl_list services_list; /* different xml different services */
    struct dl_list connection_list; /* connected device list */
    struct dl_list write_req_list; /* write request list */

    /* for ut */
    pthread_t handle;
    pthread_mutex_t lock;
    pthread_cond_t signal;
    UINT32 event_mask;
    UINT16 service_handle;

    /* for tp test */
    INT32 tp_test;
    struct timeval tp_rx_start;
    struct timeval tp_rx_last;
    UINT32 rx_pkt;
    UINT32 rx_length;
} BTMW_RPC_TEST_GATTS_SERVER;


/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static INT32 btmw_rpc_test_gatts_cmp_uuid(CHAR *uuid1, CHAR *uuid2);
static VOID btmw_rpc_test_gatts_handle_conn_update_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_CONN_UPDATE_DATA *conn_update_data);
static VOID btmw_rpc_test_gatts_handle_phy_update_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_PHY_UPDATE_DATA *phy_update_data);
static VOID btmw_rpc_test_gatts_handle_phy_read_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_READ_PHY_DATA *phy_read_data);
static VOID btmw_rpc_test_gatts_handle_ind_sent_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_IND_SENT_DATA *notif_send_data);
static VOID btmw_rpc_test_gatts_handle_mtu_change_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_MTU_CHG_DATA *mtu_chg_data);
static VOID btmw_rpc_test_gatts_handle_read_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_READ_REQ_DATA *read_req_data);
static VOID btmw_rpc_test_gatts_handle_free_write_req(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    CHAR *addr);
static VOID btmw_rpc_test_gatts_handle_write_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_WRITE_REQ_DATA *write_req_data);
static VOID btmw_rpc_test_gatts_handle_write_exec_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_WRITE_EXE_REQ_DATA *write_exec_req_data);
static VOID btmw_rpc_test_gatts_handle_register_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_REGISTER_DATA *register_data,
    INT32 server_if);
static INT32 btmw_rpc_test_gatts_save_service_handle(
    BTMW_RPC_TEST_GATTS_SERVICE *service,
    BT_GATTS_EVENT_SERVICE_ADD_DATA *add_service_data);
static VOID btmw_rpc_test_gatts_parse_service_handle(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_SERVICE_ADD_DATA *add_service_data);

static VOID btmw_rpc_test_gatts_handle_connection_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_CONNECTION_DATA *connection_data);

static VOID btmw_rpc_test_gatts_event_handle(BT_GATTS_EVENT_PARAM *param, VOID *pv_tag);
static int btmw_rpc_test_gatts_register_server_by_name(char *server_name);
static int btmw_rpc_test_gatts_register_server(int argc, char **argv);

static int btmw_rpc_test_gatts_unregister_server_by_name(char *server_name);
static int btmw_rpc_test_gatts_unregister_server(int argc, char **argv);
static int btmw_rpc_test_gatts_connect(int argc, char **argv);
static int btmw_rpc_test_gatts_disconnect(int argc, char **argv);
static VOID btmw_rpc_test_gatts_free_service(BTMW_RPC_TEST_GATTS_SERVICE *service);
static INT32 btmw_rpc_test_gatts_free_service_by_uuid(
    BTMW_RPC_TEST_GATTS_SERVER *server,
    CHAR *service_uuid);
static INT32 btmw_rpc_test_gatts_free_services(BTMW_RPC_TEST_GATTS_SERVICES *services);
static INT32 btmw_rpc_test_gatts_load_desc_from_xml(
    BTMW_RPC_TEST_GATTS_DESC *gatt_desc, xmlNode *desc_node);
static INT32 btmw_rpc_test_gatts_free_server_services(INT32 server_if);
static UINT16 btmw_rpc_test_gatts_change_char_value(INT32 server_if,
    CHAR *service_uuid, CHAR *char_uuid, CHAR *value);
static INT32 btmw_rpc_test_gatts_load_char_value_desc_from_xml(
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char, xmlNode *char_value_desc_node);
static INT32 btmw_rpc_test_gatts_load_char_from_xml(
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char, xmlNode *char_node);
static INT32 btmw_rpc_test_gatts_load_include_char_from_xml(
    BTMW_RPC_TEST_GATTS_SERVICE *service, xmlNode *include_char_node);
static UINT16 btmw_rpc_test_gatts_get_include_handle_by_uuid(CHAR *uuid);
static INT32 btmw_rpc_test_gatts_register_service_from_xml(
    INT32 server_if, BTMW_RPC_TEST_GATTS_SERVICE *service, UINT32 attr_cnt);
static INT32 btmw_rpc_test_gatts_load_service_from_xml(
    BTMW_RPC_TEST_GATTS_SERVICES *services, xmlNode *service_node);
static int btmw_rpc_test_gatts_load_services_from_xml(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server, CHAR *service_xml_path);

static int btmw_rpc_test_gatts_add_service (int argc, char **argv);
static int btmw_rpc_test_gatts_delete_service (int argc, char **argv);
static int btmw_rpc_test_gatts_delete_services (int argc, char **argv);
static int btmw_rpc_test_gatts_send_indication (int argc, char **argv);
static int btmw_rpc_test_gatts_send_notification (int argc, char **argv);
static int btmw_rpc_test_gatts_read_phy(int argc, char **argv);
static int btmw_rpc_test_gatts_set_pref_phy(int argc, char **argv);
static int btmw_rpc_test_gatts_throughput_test(int argc, char **argv);

static int btmw_rpc_test_gatts_ut_func000(void);
static int btmw_rpc_test_gatts_ut_func001(void);
static int btmw_rpc_test_gatts_ut_func002(void);
static int btmw_rpc_test_gatts_ut_func003(void);
#if 0
static int btmw_rpc_test_gatts_ut_func004(void);
#endif
static int btmw_rpc_test_gatts_ut(int argc, char **argv);

static int btmw_rpc_test_gatts_cmd_handler(int argc, char **argv);

static INT32 btmw_rpc_test_gatts_alloc_server(void);
static INT32 btmw_rpc_test_gatts_find_server_by_name(char *server_name);
static BTMW_RPC_TEST_GATTS_SERVER* btmw_rpc_test_gatts_find_server_by_if(INT32 server_if);
VOID btmw_rpc_test_gatts_free_server_by_if(INT32 server_if);
VOID btmw_rpc_test_gatts_free_server_by_index(INT32 index);
static CHAR* btmw_rpc_test_gatts_get_event_str(BT_GATTS_EVENT event);
/* STATIC VARIABLE DECLARATIONS
 */
static BTMW_RPC_TEST_CLI btmw_rpc_test_gatts_cli_commands[] =
{
    {(const char *)"register_server",     btmw_rpc_test_gatts_register_server,     (const char *)" = register_server <name>"},
    {(const char *)"unregister_server",   btmw_rpc_test_gatts_unregister_server,   (const char *)" = unregister_server <name>"},
    {(const char *)"connect",             btmw_rpc_test_gatts_connect,             (const char *)" = connect <server_name> <addr> [isDirect <true|false> [<auto|bredr|ble>]]"},
    {(const char *)"disconnect",          btmw_rpc_test_gatts_disconnect,          (const char *)" = disconnect <server_name> <addr>"},
    {(const char *)"add_service",         btmw_rpc_test_gatts_add_service,         (const char *)" = add_service <server_name> <service_xml_path>"},
    {(const char *)"delete_service",      btmw_rpc_test_gatts_delete_service,      (const char *)" = delete_service <server_name> <service_uuid>"},
    {(const char *)"delete_services",     btmw_rpc_test_gatts_delete_services,     (const char *)" = delete_services <server_name> <service_xml_path>"},
    {(const char *)"send_indication",     btmw_rpc_test_gatts_send_indication,     (const char *)" = send_indication <server_name> <service_uuid> <char_uuid> <addr> <value>"},
    {(const char *)"send_notify",         btmw_rpc_test_gatts_send_notification,   (const char *)" = send_notify <server_name> <service_uuid> <char_uuid> <addr> <value>"},
    {(const char *)"read_phy",            btmw_rpc_test_gatts_read_phy,            (const char *)" = read_phy <server_name> <addr>"},
    {(const char *)"set_pref_phy",        btmw_rpc_test_gatts_set_pref_phy,        (const char *)" = set_pref_phy <server_name> <addr>  <tx_phy> <rx_phy> <phy_option>"},
    {(const char *)"throughput_test",     btmw_rpc_test_gatts_throughput_test,     (const char *)" = throughput_test <server_name>  <start|stop>"},
    {(const char *)"ut",                  btmw_rpc_test_gatts_ut,                  (const char *)" = ut"},
    {NULL, NULL, NULL},
};

BTMW_RPC_TEST_GATTS_SERVER s_btmw_rpc_gatts_servers[BTMW_RPC_TEST_GATTS_TEST_APP_CNT];

static pthread_mutex_t btmw_rpc_test_gatts_lock;

/* EXPORTED SUBPROGRAM BODIES
 */

int btmw_rpc_test_gatts_init(void)
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    int ret = 0;
    BTMW_RPC_TEST_MOD gatts_mod = {0};
    int i = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&btmw_rpc_test_gatts_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    // Register command to CLI
    gatts_mod.mod_id = BTMW_RPC_TEST_MOD_GATT_SERVER;
    strncpy(gatts_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GATTS, sizeof(gatts_mod.cmd_key));
    gatts_mod.cmd_handler = btmw_rpc_test_gatts_cmd_handler;
    gatts_mod.cmd_tbl = btmw_rpc_test_gatts_cli_commands;
    ret = btmw_rpc_test_register_mod(&gatts_mod);
    BTMW_RPC_TEST_Logd("[GATTS] btmw_rpc_test_register_mod() returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        a_mtkapi_bt_gatt_profile_init();
    }

    for(i=0;i<BTMW_RPC_TEST_GATTS_TEST_APP_CNT;i++)
    {
        memset((void*)&s_btmw_rpc_gatts_servers[i], 0,
            sizeof(s_btmw_rpc_gatts_servers[i]));
        dl_list_init(&s_btmw_rpc_gatts_servers[i].services_list);
        dl_list_init(&s_btmw_rpc_gatts_servers[i].connection_list);
        dl_list_init(&s_btmw_rpc_gatts_servers[i].write_req_list);
        pthread_cond_init(&s_btmw_rpc_gatts_servers[i].signal, NULL);
        pthread_mutex_init(&s_btmw_rpc_gatts_servers[i].lock, NULL);
    }
    return ret;
}

int btmw_rpc_test_gatts_deinit(void)
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */
static INT32 btmw_rpc_test_gatts_parse_value(CHAR *value_str)
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

static INT32 btmw_rpc_test_gatts_cmp_uuid(CHAR *uuid1, CHAR *uuid2)
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
            if (rc != 16) return -1;
            if (c != (BT_GATT_MAX_UUID_LEN - 1)) return -1;
          } else if (strlen(uuid) == 8) {
            int c;
            int rc = sscanf(uuid,
              "%02hhx%02hhx%02hhx%02hhx%n",
              &ptr_uuid_128[0], &ptr_uuid_128[1], &ptr_uuid_128[2], &ptr_uuid_128[3], &c);
            if (rc != 4) return -1;
            if (c != 8) return -1;
          } else if (strlen(uuid) == 4) {
            int c;
            int rc = sscanf(uuid,
              "%02hhx%02hhx%n", &ptr_uuid_128[2], &ptr_uuid_128[3], &c);
            if (rc != 2) return -1;
            if (c != 4) return -1;
          }
          else
          {
            return -1;
          }
    }

    return memcmp(&uuid_128[0], &uuid_128[1], 16);
}

static VOID btmw_rpc_test_gatts_handle_conn_update_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_CONN_UPDATE_DATA *conn_update_data)
{
    if (NULL == conn_update_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, interval:%d*1.25ms, latency:%d, timeout:%dms",
        conn_update_data->status, conn_update_data->addr,
        conn_update_data->interval, conn_update_data->latency,
        conn_update_data->timeout*10);
    gatts_server->status = conn_update_data->status;

    BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gatts_server->connection_list,
        BTMW_RPC_TEST_GATTS_CONNECTION, node)
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

static VOID btmw_rpc_test_gatts_handle_phy_update_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_PHY_UPDATE_DATA *phy_update_data)
{
    if (NULL == phy_update_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, rx_phy:%d, tx_phy:%d",
        phy_update_data->status, phy_update_data->addr,
        phy_update_data->rx_phy, phy_update_data->tx_phy);
    gatts_server->status = phy_update_data->status;

    BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gatts_server->connection_list,
        BTMW_RPC_TEST_GATTS_CONNECTION, node)
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

static VOID btmw_rpc_test_gatts_handle_phy_read_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_READ_PHY_DATA *phy_read_data)
{
    if (NULL == phy_read_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, addr:%s, rx_phy:%d, tx_phy:%d",
        phy_read_data->status, phy_read_data->addr,
        phy_read_data->rx_phy, phy_read_data->tx_phy);
    gatts_server->status = phy_read_data->status;

    BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gatts_server->connection_list,
        BTMW_RPC_TEST_GATTS_CONNECTION, node)
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

static VOID btmw_rpc_test_gatts_handle_ind_sent_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_IND_SENT_DATA *notif_send_data)
{
    if (NULL == notif_send_data || NULL == gatts_server)
    {
        return;
    }
    gatts_server->status = notif_send_data->status;

    BTMW_RPC_TEST_Logd("addr:%s, status:%d",
        notif_send_data->addr, notif_send_data->status);

    return;
}


static VOID btmw_rpc_test_gatts_handle_mtu_change_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_MTU_CHG_DATA *mtu_chg_data)
{
    if (NULL == mtu_chg_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("addr:%s, mtu:%d",
        mtu_chg_data->addr, mtu_chg_data->mtu);

    BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
    dl_list_for_each(conn, &gatts_server->connection_list,
        BTMW_RPC_TEST_GATTS_CONNECTION, node)
    {
        if (!strncasecmp(conn->addr, mtu_chg_data->addr, MAX_BDADDR_LEN - 1))
        {
            conn->mtu = mtu_chg_data->mtu;
            return;
        }
    }
    return;
}

static VOID btmw_rpc_test_gatts_handle_read_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_READ_REQ_DATA *read_req_data)
{
    BT_GATTS_RESPONSE_PARAM resp_param;
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    if (NULL == read_req_data || NULL == gatts_server)
    {
        BTMW_RPC_TEST_Loge("NULL pointer");
        return;
    }
    memset((void*)&resp_param, 0, sizeof(resp_param));

    memcpy(resp_param.addr, read_req_data->addr, sizeof(resp_param.addr));
    resp_param.server_if = gatts_server->server_if;
    resp_param.handle = read_req_data->handle;
    resp_param.trans_id = read_req_data->trans_id;
    resp_param.offset = read_req_data->offset;

    if (0 == gatts_server->tp_test)
    {
        BTMW_RPC_TEST_Logd("server_if:%d addr:%s, handle:%d, offset:%d, trans_id:%d",
            gatts_server->server_if, read_req_data->addr,
            read_req_data->handle, read_req_data->offset,
            read_req_data->trans_id);
    }

    dl_list_for_each(services, &gatts_server->services_list,
        BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
        dl_list_for_each(service, &services->service_list,
            BTMW_RPC_TEST_GATTS_SERVICE, node)
        {
            BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
            dl_list_for_each(gatt_char, &service->char_list,
                BTMW_RPC_TEST_GATTS_CHAR, node)
            {
                if (gatt_char->handle == read_req_data->handle)
                {
                    if (read_req_data->offset >= gatt_char->len)
                    {
                        resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                        resp_param.len = 0;
                    }
                    else
                    {
                        resp_param.status = BT_GATT_STATUS_OK;
                        resp_param.len = gatt_char->len - read_req_data->offset;
                        if (BT_GATT_MAX_VALUE_LEN < resp_param.len)
                        {
                            resp_param.len = BT_GATT_MAX_VALUE_LEN;
                        }
                        memcpy((void*)resp_param.value,
                            &gatt_char->value[read_req_data->offset],
                            resp_param.len);
                    }
                    BTMW_RPC_TEST_Logi("gatt_char %s response len %d",
                        gatt_char->uuid, resp_param.len);
                    a_mtkapi_bt_gatts_send_response(&resp_param);
                    return;
                }

                BTMW_RPC_TEST_GATTS_DESC *gatt_desc = NULL;
                dl_list_for_each(gatt_desc, &gatt_char->desc_list,
                    BTMW_RPC_TEST_GATTS_DESC, node)
                {
                    if (gatt_desc->handle == read_req_data->handle)
                    {
                        if (read_req_data->offset >= gatt_desc->len)
                        {
                            resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                            resp_param.len = 0;
                        }
                        else
                        {
                            resp_param.status = BT_GATT_STATUS_OK;
                            resp_param.len = gatt_desc->len - read_req_data->offset;
                            if (BT_GATT_MAX_VALUE_LEN < resp_param.len)
                            {
                                resp_param.len = BT_GATT_MAX_VALUE_LEN;
                            }
                            memcpy((void*)resp_param.value,
                                &gatt_desc->value[read_req_data->offset],
                                resp_param.len);
                        }
                        BTMW_RPC_TEST_Logi("gatt_desc %s response len %d, offset %d",
                            gatt_desc->uuid, resp_param.len, read_req_data->offset);
                        a_mtkapi_bt_gatts_send_response(&resp_param);
                        return;
                    }
                }
            }
        }
    }

    resp_param.status = BT_GATT_STATUS_INVALID_HANDLE;
    resp_param.len = 0;
    BTMW_RPC_TEST_Logi("error response");
    a_mtkapi_bt_gatts_send_response(&resp_param);

    return;
}

static VOID btmw_rpc_test_gatts_handle_free_write_req(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    CHAR *addr)
{
    BTMW_RPC_TEST_GATTS_WRITE_REQ *write_req = NULL;
    BTMW_RPC_TEST_GATTS_WRITE_REQ *temp = NULL;
    if (NULL == addr || NULL == gatts_server)
    {
        BTMW_RPC_TEST_Loge("NULL pointer");
        return;
    }

    dl_list_for_each_safe(write_req, temp, &gatts_server->write_req_list,
        BTMW_RPC_TEST_GATTS_WRITE_REQ, node)
    {
        if (!strncasecmp(write_req->data.addr, addr, MAX_BDADDR_LEN - 1))
        {
            dl_list_del(&write_req->node);
            free(write_req);
            write_req = NULL;
        }
    }
    return;
}

static VOID btmw_rpc_test_gatts_handle_write_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_WRITE_REQ_DATA *write_req_data)
{
    BT_GATTS_RESPONSE_PARAM resp_param;
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    if (NULL == write_req_data || NULL == gatts_server)
    {
        BTMW_RPC_TEST_Loge("NULL pointer");
        return;
    }
    memset((void*)&resp_param, 0, sizeof(resp_param));

    memcpy(resp_param.addr, write_req_data->addr, sizeof(resp_param.addr));
    resp_param.server_if = gatts_server->server_if;
    resp_param.handle = write_req_data->handle;
    resp_param.trans_id = write_req_data->trans_id;
    resp_param.offset = write_req_data->offset;
    resp_param.len = write_req_data->value_len;
    if (BT_GATT_MAX_VALUE_LEN < resp_param.len)
    {
        resp_param.len = BT_GATT_MAX_VALUE_LEN;
    }
    memcpy((void*)resp_param.value, write_req_data->value, resp_param.len);

    resp_param.status = BT_GATT_STATUS_OK;
    if (0 == gatts_server->tp_test)
    {
        BTMW_RPC_TEST_Logd("server_if:%d addr:%s, handle:%d, offset:%d, len:%d, is_prep:%d, need_rsp:%d, trans_id:%d",
            gatts_server->server_if, write_req_data->addr,
            write_req_data->handle, write_req_data->offset,
            write_req_data->value_len,
            write_req_data->is_prep, write_req_data->need_rsp,
            write_req_data->trans_id);
    }

    if (write_req_data->is_prep)
    {
        BTMW_RPC_TEST_GATTS_WRITE_REQ *write_req =
            calloc(1, sizeof(BTMW_RPC_TEST_GATTS_WRITE_REQ));
        if (NULL == write_req)
        {
            BTMW_RPC_TEST_Loge("alloc write_req fail");
            return;
        }

        memcpy((void*)&write_req->data, write_req_data,
            sizeof(BT_GATTS_EVENT_WRITE_REQ_DATA));
        dl_list_add_tail(&gatts_server->write_req_list, &write_req->node);
        a_mtkapi_bt_gatts_send_response(&resp_param);
        return;
    }

    dl_list_for_each(services, &gatts_server->services_list,
        BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
        dl_list_for_each(service, &services->service_list,
            BTMW_RPC_TEST_GATTS_SERVICE, node)
        {
            BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
            dl_list_for_each(gatt_char, &service->char_list,
                BTMW_RPC_TEST_GATTS_CHAR, node)
            {
                if (gatt_char->handle == write_req_data->handle)
                {
                    if (write_req_data->offset + write_req_data->value_len > BT_GATT_MAX_VALUE_LEN)
                    {
                        resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                        resp_param.len = 0;
                    }
                    else
                    {
                        if (0 == write_req_data->offset)
                        {
                            memset((void*)&gatt_char->value, 0, sizeof(gatt_char->value));
                        }
                        if (write_req_data->offset < BT_GATT_MAX_VALUE_LEN)
                        {
                            memcpy((void*)&gatt_char->value[write_req_data->offset],
                                write_req_data->value, write_req_data->value_len);
                        }
                        gatt_char->len = write_req_data->offset + write_req_data->value_len;

                        if (0 == gatts_server->tp_test)
                        {
                            BTMW_RPC_TEST_Logd("gatt_char %s offset:%d, value_len:%d",
                                gatt_char->uuid, write_req_data->offset, write_req_data->value_len);

                            /* save to xml node */
                            xmlNodeSetContentLen(gatt_char->xml_node,
                                (xmlChar*)gatt_char->value, gatt_char->len);
                            {
                                CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
                                strncpy(xml_name, services->xml_name, BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
                                xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';

                                xmlSaveFormatFile(strncat(xml_name, ".dat",
                                    BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1), services->doc, 1);
                            }
                        }
                        else
                        {
                            if (gatts_server->rx_pkt == 0)
                            {
                                gettimeofday(&gatts_server->tp_rx_start, NULL);
                                gettimeofday(&gatts_server->tp_rx_last, NULL);
                            }
                            else
                            {
                                gettimeofday(&gatts_server->tp_rx_last, NULL);
                            }
                            gatts_server->rx_pkt++;
                            gatts_server->rx_length += write_req_data->value_len;
                        }
                    }
                    if (write_req_data->need_rsp)
                    {
                        a_mtkapi_bt_gatts_send_response(&resp_param);
                    }
                    return;
                }

                BTMW_RPC_TEST_GATTS_DESC *gatt_desc = NULL;
                dl_list_for_each(gatt_desc, &gatt_char->desc_list,
                    BTMW_RPC_TEST_GATTS_DESC, node)
                {
                    if (gatt_desc->handle == write_req_data->handle)
                    {
                        if (write_req_data->offset + write_req_data->value_len > BT_GATT_MAX_VALUE_LEN)
                        {
                            resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                            resp_param.len = 0;
                        }
                        else
                        {
                            if (0 == write_req_data->offset)
                            {
                                memset((void*)&gatt_desc->value, 0, sizeof(gatt_desc->value));
                            }

                            if (write_req_data->offset < BT_GATT_MAX_VALUE_LEN)
                            {
                                memcpy((void*)&gatt_desc->value[write_req_data->offset],
                                    write_req_data->value, write_req_data->value_len);
                            }
                            gatt_desc->len = write_req_data->offset + write_req_data->value_len;
                            if (0 == gatts_server->tp_test)
                            {
                                BTMW_RPC_TEST_Logd("gatt_desc %s offset:%s, value_len:%d",
                                    gatt_desc->uuid, write_req_data->offset, write_req_data->value_len);

                                /* save to xml node */
                                xmlNodeSetContentLen(gatt_desc->xml_node,
                                    (xmlChar*)gatt_desc->value, gatt_desc->len );

                                {
                                    CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
                                    strncpy(xml_name, services->xml_name, BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
                                    xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';

                                    xmlSaveFormatFile(strncat(xml_name, ".dat",
                                        BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1), services->doc, 1);
                                }
                            }
                            else
                            {
                                if (gatts_server->rx_pkt == 0)
                                {
                                    gettimeofday(&gatts_server->tp_rx_start, NULL);
                                    gettimeofday(&gatts_server->tp_rx_last, NULL);
                                }
                                else
                                {
                                    gettimeofday(&gatts_server->tp_rx_last, NULL);
                                }
                                gatts_server->rx_pkt++;
                                gatts_server->rx_length += write_req_data->value_len;
                            }
                        }

                        if (write_req_data->need_rsp)
                        {
                            a_mtkapi_bt_gatts_send_response(&resp_param);
                        }
                        return;
                    }
                }
            }
        }
    }

    resp_param.status = BT_GATT_STATUS_INVALID_HANDLE;
    resp_param.len = 0;
    a_mtkapi_bt_gatts_send_response(&resp_param);

    return;
}

static VOID btmw_rpc_test_gatts_handle_write_exec_req_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_WRITE_EXE_REQ_DATA *write_exec_req_data)
{
    BT_GATTS_RESPONSE_PARAM resp_param;
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    BTMW_RPC_TEST_GATTS_WRITE_REQ *write_req = NULL;
    INT32 encounter_error = 0;
    if (NULL == write_exec_req_data || NULL == gatts_server)
    {
        BTMW_RPC_TEST_Loge("NULL pointer");
        return;
    }
    memset((void*)&resp_param, 0, sizeof(resp_param));

    memcpy(resp_param.addr, write_exec_req_data->addr, sizeof(resp_param.addr));
    resp_param.server_if = gatts_server->server_if;
    resp_param.trans_id = write_exec_req_data->trans_id;
    resp_param.status = BT_GATT_STATUS_OK;

    BTMW_RPC_TEST_Logd("server_if:%d addr:%s, exec_write:%d, trans_id:%d",
        gatts_server->server_if, write_exec_req_data->addr,
        write_exec_req_data->exec_write, write_exec_req_data->trans_id);

    dl_list_for_each(write_req, &gatts_server->write_req_list,
        BTMW_RPC_TEST_GATTS_WRITE_REQ, node)
    {
        BT_GATTS_EVENT_WRITE_REQ_DATA *write_req_data = &write_req->data;
        if (strncasecmp(write_req_data->addr,
            write_exec_req_data->addr, MAX_BDADDR_LEN - 1))
        {
            continue;
        }

        if ((0 == write_exec_req_data->exec_write)
            || (0 != encounter_error))
        {
            BTMW_RPC_TEST_Logd("skip write request addr:%s, trans_id:%d",
                write_req_data->addr, write_req_data->trans_id);
            goto __NEXT;
        }

        BTMW_RPC_TEST_Logd("addr:%s, trans_id:%d",
            write_req_data->addr, write_req_data->trans_id);

        dl_list_for_each(services, &gatts_server->services_list,
            BTMW_RPC_TEST_GATTS_SERVICES, node)
        {
            BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
            dl_list_for_each(service, &services->service_list,
                BTMW_RPC_TEST_GATTS_SERVICE, node)
            {
                BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
                dl_list_for_each(gatt_char, &service->char_list,
                    BTMW_RPC_TEST_GATTS_CHAR, node)
                {
                    if (gatt_char->handle == write_req_data->handle)
                    {
                        if (write_req_data->offset + write_req_data->value_len > BT_GATT_MAX_VALUE_LEN)
                        {
                            resp_param.status = BT_GATT_STATUS_ERROR;
                            resp_param.len = 0;

                            a_mtkapi_bt_gatts_send_response(&resp_param);

                            encounter_error = 1;
                        }
                        else
                        {
                            if (0 == write_req_data->offset)
                            {
                                memset((void*)&gatt_char->value, 0, sizeof(gatt_char->value));
                            }

                            if (write_req_data->offset < BT_GATT_MAX_VALUE_LEN)
                            {
                                memcpy((void*)&gatt_char->value[write_req_data->offset],
                                    write_req_data->value, write_req_data->value_len);
                            }
                            gatt_char->len = write_req_data->offset + write_req_data->value_len;

                            BTMW_RPC_TEST_Logd("gatt_char %s offset:%d, value_len:%d",
                                gatt_char->uuid, write_req_data->offset, write_req_data->value_len);

                            /* save to xml node */
                            xmlNodeSetContentLen(gatt_char->xml_node,
                                (xmlChar*)gatt_char->value, gatt_char->len);
                            {
                                CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
                                strncpy(xml_name, services->xml_name, BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
                                xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';

                                xmlSaveFormatFile(strncat(xml_name, ".dat",
                                    BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1), services->doc, 1);
                            }

                        }
                        goto __NEXT;
                    }

                    BTMW_RPC_TEST_GATTS_DESC *gatt_desc = NULL;
                    dl_list_for_each(gatt_desc, &gatt_char->desc_list,
                        BTMW_RPC_TEST_GATTS_DESC, node)
                    {
                        if (gatt_desc->handle == write_req_data->handle)
                        {
                            if (write_req_data->offset + write_req_data->value_len > BT_GATT_MAX_VALUE_LEN)
                            {
                                resp_param.status = BT_GATT_STATUS_ERROR;
                                resp_param.len = 0;
                                a_mtkapi_bt_gatts_send_response(&resp_param);

                                encounter_error = 1;
                            }
                            else
                            {
                                if (0 == write_req_data->offset)
                                {
                                    memset((void*)&gatt_desc->value, 0, sizeof(gatt_desc->value));
                                }

                                if (write_req_data->offset < BT_GATT_MAX_VALUE_LEN)
                                {
                                    memcpy((void*)&gatt_desc->value[write_req_data->offset],
                                        write_req_data->value, write_req_data->value_len);
                                }
                                gatt_desc->len = write_req_data->offset + write_req_data->value_len;

                                BTMW_RPC_TEST_Logd("gatt_desc %s offset:%d, value_len:%d",
                                    gatt_desc->uuid, write_req_data->offset, write_req_data->value_len);

                                /* save to xml node */
                                xmlNodeSetContent(gatt_desc->xml_node, (xmlChar*)gatt_desc->value);
                                {
                                    CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
                                    strncpy(xml_name, services->xml_name, BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
                                    xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';

                                    xmlSaveFormatFile(strncat(xml_name, ".dat",
                                        BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1), services->doc, 1);
                                }

                            }
                            goto __NEXT;
                        }
                    }
                }
            }
        }
__NEXT:
        ;
    }
    btmw_rpc_test_gatts_handle_free_write_req(gatts_server,
        write_exec_req_data->addr);

    if (0 == encounter_error
        || 0 == write_exec_req_data->exec_write)
    {
        resp_param.len = 0;
        a_mtkapi_bt_gatts_send_response(&resp_param);
    }
    return;
}


static VOID btmw_rpc_test_gatts_handle_register_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_REGISTER_DATA *register_data,
    INT32 server_if)
{
    if (NULL == register_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("server_name:%s, status:%d, server_if:%d",
        register_data->server_name, register_data->status, server_if);
    gatts_server->status = register_data->status;
    if (NULL != gatts_server)
    {
        if (register_data->status == BT_GATT_STATUS_OK)
        {
            gatts_server->server_if = server_if;
            gatts_server->status = register_data->status;
        }
        else
        {
            btmw_rpc_test_gatts_free_server_by_index(gatts_server - s_btmw_rpc_gatts_servers);
            BTMW_RPC_TEST_Loge("server_name:%s, status:%d register fail, free it",
                register_data->server_name,
                register_data->status);
        }
    }

    return;
}

static VOID btmw_rpc_test_gatts_handle_connection_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_CONNECTION_DATA *connection_data)
{
    if (NULL == connection_data || NULL == gatts_server)
    {
        return;
    }

    if (connection_data->connected)
    {
        BTMW_RPC_TEST_GATTS_CONNECTION *conn =
            calloc(1, sizeof(BTMW_RPC_TEST_GATTS_CONNECTION));
        if (NULL == conn) return;

        strncpy(conn->addr, connection_data->addr, MAX_BDADDR_LEN - 1);
        conn->addr[MAX_BDADDR_LEN - 1] = '\0';

        dl_list_add(&gatts_server->connection_list, &conn->node);

        BTMW_RPC_TEST_Logd("server %d addr:%s connected",
            gatts_server->server_if,
            conn->addr);
    }
    else
    {
        BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
        dl_list_for_each(conn, &gatts_server->connection_list,
            BTMW_RPC_TEST_GATTS_CONNECTION, node)
        {
            if (!strncasecmp(conn->addr, connection_data->addr, MAX_BDADDR_LEN - 1))
            {
                BTMW_RPC_TEST_Logd("server %d addr:%s disconnected",
                    gatts_server->server_if,
                    connection_data->addr);
                dl_list_del(&conn->node);
                free(conn);
                //conn = NULL; //coverity check don't allow to assign but no use

                btmw_rpc_test_gatts_handle_free_write_req(gatts_server,
                    connection_data->addr);
                return;
            }
        }
    }
}


static VOID btmw_rpc_test_gatts_handle_service_added_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_SERVICE_ADD_DATA *service_added_data)
{
    if (NULL == service_added_data || NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("status:%d, service:%s, attr_cnt:%d, handle=0x%x",
        service_added_data->status,
        service_added_data->service_uuid,
        service_added_data->attr_cnt,
        service_added_data->attrs[0].handle);
    gatts_server->status = service_added_data->status;
    if (BT_GATT_STATUS_OK == service_added_data->status)
    {
        /* for ut */
        if (service_added_data->attrs[0].type == GATT_ATTR_TYPE_PRIMARY_SERVICE)
        {
            gatts_server->service_handle = service_added_data->attrs[0].handle;
        }

        btmw_rpc_test_gatts_parse_service_handle(gatts_server,
            service_added_data);
    }
    else
    {
        /* for ut */
        if (service_added_data->attrs[0].type == GATT_ATTR_TYPE_PRIMARY_SERVICE)
        {
            gatts_server->service_handle = 0;
        }

        btmw_rpc_test_gatts_free_service_by_uuid(gatts_server,
            service_added_data->service_uuid);
    }

    return;
}


static INT32 btmw_rpc_test_gatts_save_service_handle(
    BTMW_RPC_TEST_GATTS_SERVICE *service,
    BT_GATTS_EVENT_SERVICE_ADD_DATA *add_service_data)
{
    UINT32 attr_index = 0;
    UINT32 is_finish = 0;
    BTMW_RPC_TEST_GATTS_INCLUDE *gatt_inc = NULL;
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
    BTMW_RPC_TEST_GATTS_DESC *gatt_desc = NULL;

    if (NULL == service || NULL == add_service_data)
    {
        BTMW_RPC_TEST_Loge("NULL pointer\n");
        return 0;
    }

    if (add_service_data->attr_cnt == 0)
    {
        BTMW_RPC_TEST_Loge("attr_cnt is 0\n");
        return  0;
    }

    if (add_service_data->attr_cnt > BT_GATT_MAX_ATTR_CNT)
    {
        BTMW_RPC_TEST_Loge("attr_cnt is %d\n", add_service_data->attr_cnt);
        add_service_data->attr_cnt = BT_GATT_MAX_ATTR_CNT;
    }

    if ((0 == service->handle)
        && (add_service_data->attrs[attr_index].type
                == GATT_ATTR_TYPE_PRIMARY_SERVICE
            || add_service_data->attrs[attr_index].type
                == GATT_ATTR_TYPE_SECONDARY_SERVICE))
    {
        service->handle =
            add_service_data->attrs[attr_index++].handle;
        BTMW_RPC_TEST_Loge("service %s handle %d\n", service->uuid, service->handle);
    }

    if (attr_index >= add_service_data->attr_cnt) goto __SAVE_END;

    dl_list_for_each(gatt_inc, &service->include_list,
                        BTMW_RPC_TEST_GATTS_INCLUDE, node)
    {
        if (attr_index >= add_service_data->attr_cnt) goto __SAVE_END;

        if (!btmw_rpc_test_gatts_cmp_uuid(gatt_inc->uuid,
            add_service_data->attrs[attr_index].uuid) && 0 != gatt_inc->handle)
        {
            gatt_inc->handle =
                add_service_data->attrs[attr_index++].handle;
            BTMW_RPC_TEST_Loge("include service %s handle %d\n", gatt_inc->uuid, gatt_inc->handle);
        }
        else
        {
            BTMW_RPC_TEST_Loge("include uuid %s not match attr uuid %s\n",
                gatt_inc->uuid,
                add_service_data->attrs[attr_index].uuid);
        }
    }

    dl_list_for_each(gatt_char, &service->char_list,
                        BTMW_RPC_TEST_GATTS_CHAR, node)
    {
        if (attr_index >= add_service_data->attr_cnt) goto __SAVE_END;

        if (0 == gatt_char->handle)
        {
            if (!btmw_rpc_test_gatts_cmp_uuid(gatt_char->uuid,
                add_service_data->attrs[attr_index].uuid))
            {
                gatt_char->handle =
                    add_service_data->attrs[attr_index++].handle;
                BTMW_RPC_TEST_Loge("char %s handle %d\n", gatt_char->uuid, gatt_char->handle);
            }
            else
            {
                BTMW_RPC_TEST_Loge("char uuid %s not match attr uuid %s\n",
                    gatt_char->uuid,
                    add_service_data->attrs[attr_index].uuid);
            }
        }

        dl_list_for_each(gatt_desc, &gatt_char->desc_list,
                            BTMW_RPC_TEST_GATTS_DESC, node)
        {
            if (attr_index >= add_service_data->attr_cnt) goto __SAVE_END;

            if (0 == gatt_desc->handle)
            {
                if (!btmw_rpc_test_gatts_cmp_uuid(gatt_desc->uuid,
                    add_service_data->attrs[attr_index].uuid))
                {
                    gatt_desc->handle =
                        add_service_data->attrs[attr_index++].handle;
                    BTMW_RPC_TEST_Loge("char %s handle %d\n", gatt_desc->uuid, gatt_desc->handle);
                }
                else
                {
                    BTMW_RPC_TEST_Loge("desc uuid %s not match %s\n",
                        gatt_desc->uuid,
                        add_service_data->attrs[attr_index].uuid);
                }
            }
        }
    }

    is_finish = 1;

__SAVE_END:
    if (add_service_data->attr_cnt != attr_index || is_finish == 0)
    {
        BTMW_RPC_TEST_Loge("service handle count %d not match attr cnt %d\n",
            attr_index, add_service_data->attr_cnt);
    }
    else
    {
        service->started = 1;
    }

    return attr_index;
}

static VOID btmw_rpc_test_gatts_parse_service_handle(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    BT_GATTS_EVENT_SERVICE_ADD_DATA *add_service_data)
{
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    if ((NULL == add_service_data) || (NULL == gatts_server))
    {
        BTMW_RPC_TEST_Loge("NULL pointer\n");
        return;
    }

    BTMW_RPC_TEST_Logd("server:%d , attr_cnt:%d, added_service:%s\n",
        gatts_server->server_if, add_service_data->attr_cnt,
        add_service_data->service_uuid);

    dl_list_for_each(services, &gatts_server->services_list,
        BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        BTMW_RPC_TEST_Logd("server:%d, xml_name:%s\n",
            gatts_server->server_if, services->xml_name);
        BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
        dl_list_for_each(service, &services->service_list,
            BTMW_RPC_TEST_GATTS_SERVICE, node)
        {
            BTMW_RPC_TEST_Logd("service:%s,  started=%d\n",
                service->uuid, service->started);
            /* find service from services list */
            if ((!btmw_rpc_test_gatts_cmp_uuid(service->uuid,
                add_service_data->service_uuid)) && (0 == service->started))
            {
                if (0 < btmw_rpc_test_gatts_save_service_handle(service, add_service_data))
                {
                    return;
                }
            }
        }
    }
}

static VOID btmw_rpc_test_gatts_handle_bt_off_event(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server)
{
    if (NULL == gatts_server)
    {
        return;
    }

    BTMW_RPC_TEST_Logd(" server_if:%d, name:%s",
        gatts_server->server_if, gatts_server->server_name);

    btmw_rpc_test_gatts_free_server_by_if(gatts_server->server_if);

    return;
}

static VOID btmw_rpc_test_gatts_event_handle(BT_GATTS_EVENT_PARAM *param, VOID *pv_tag)
{
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server = NULL;

    if (NULL != pv_tag)
    {
        gatts_server = (BTMW_RPC_TEST_GATTS_SERVER *)pv_tag;
    }

    if (NULL == gatts_server)
    {
        INT32 index = 0;
        if (param->event == BT_GATTS_EVENT_REGISTER)
        {
            index = btmw_rpc_test_gatts_find_server_by_name(param->data.register_data.server_name);
            if (index < 0)
            {
                BTMW_RPC_TEST_Logd("no server %d \n", param->server_if);
                return;
            }
            gatts_server = &s_btmw_rpc_gatts_servers[index];
        }
        else
        {
            gatts_server = btmw_rpc_test_gatts_find_server_by_if(param->server_if);
        }

        if (gatts_server == NULL)
        {
            BTMW_RPC_TEST_Logd("no server %d \n", param->server_if);
            return;
        }
    }

    if (0 == gatts_server->tp_test)
    {
        BTMW_RPC_TEST_Logd("server_if:%d(%s), event: %s, size:%d\n",
            param->server_if, gatts_server->server_name,
            btmw_rpc_test_gatts_get_event_str(param->event),
            sizeof(BT_GATTS_EVENT_PARAM));
    }

    BTMW_RPC_TEST_GATTS_LOCK(btmw_rpc_test_gatts_lock);

    switch(param->event)
    {
        case BT_GATTS_EVENT_REGISTER:
            btmw_rpc_test_gatts_handle_register_event(gatts_server,
                &param->data.register_data, param->server_if);
            break;
        case BT_GATTS_EVENT_CONNECTION:
            btmw_rpc_test_gatts_handle_connection_event(gatts_server,
                &param->data.connection_data);
            break;
        case BT_GATTS_EVENT_SERVICE_ADD:
            btmw_rpc_test_gatts_handle_service_added_event(gatts_server,
                &param->data.add_service_data);
            break;
        case BT_GATTS_EVENT_READ_REQ:
            btmw_rpc_test_gatts_handle_read_req_event(gatts_server,
                &param->data.read_req_data);
            break;
        case BT_GATTS_EVENT_WRITE_REQ:
            btmw_rpc_test_gatts_handle_write_req_event(gatts_server,
                &param->data.write_req_data);
            break;
        case BT_GATTS_EVENT_WRITE_EXE_REQ:
            btmw_rpc_test_gatts_handle_write_exec_req_event(gatts_server,
                &param->data.write_exe_req_data);
            break;
        case BT_GATTS_EVENT_IND_SENT:
            btmw_rpc_test_gatts_handle_ind_sent_event(gatts_server,
                &param->data.ind_sent_data);
            break;
        case BT_GATTS_EVENT_MTU_CHANGE:
            btmw_rpc_test_gatts_handle_mtu_change_event(gatts_server,
                &param->data.mtu_chg_data);
            break;
        case BT_GATTS_EVENT_PHY_READ:
            btmw_rpc_test_gatts_handle_phy_read_event(gatts_server,
                &param->data.read_phy_data);
            break;
        case BT_GATTS_EVENT_PHY_UPDATE:
            btmw_rpc_test_gatts_handle_phy_update_event(gatts_server,
                &param->data.phy_upd_data);
            break;
        case BT_GATTS_EVENT_CONN_UPDATE:
            btmw_rpc_test_gatts_handle_conn_update_event(gatts_server,
                &param->data.conn_upd_data);
            break;
        case BT_GATTS_EVENT_MAX:
            btmw_rpc_test_gatts_handle_bt_off_event(gatts_server);
            break;
        default:
            break;
    }
    BTMW_RPC_TEST_GATTS_UNLOCK(btmw_rpc_test_gatts_lock);
    if (NULL != gatts_server)
    {
        BTMW_RPC_TEST_GATTS_SIGNAL(gatts_server->lock,
            gatts_server->signal,
            gatts_server->event_mask |= (1 << param->event));
    }
    return;
}

static int btmw_rpc_test_gatts_register_server_by_name(char *server_name)
{
    INT32 index = 0;
    INT32 ret = 0;
    if (NULL == server_name)
    {
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(server_name);
    if (index >= 0)
    {
        BTMW_RPC_TEST_Logi("[GATTS] server %s registered, server_if:%d\n",
            server_name, s_btmw_rpc_gatts_servers[index].server_if);
        return -1;
    }

    index = btmw_rpc_test_gatts_alloc_server();
    if (index < 0)
    {
        BTMW_RPC_TEST_Logi("[GATTS] alloc server %s fail\n", server_name);
        return -1;
    }
    BTMW_RPC_TEST_Logi("server_names: %s", server_name);

    strncpy(s_btmw_rpc_gatts_servers[index].server_name,
        server_name, BT_GATT_MAX_UUID_LEN - 1);
    s_btmw_rpc_gatts_servers[index].server_name[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    s_btmw_rpc_gatts_servers[index].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

    if (BT_SUCCESS != (ret = a_mtkapi_bt_gatts_register(
        s_btmw_rpc_gatts_servers[index].server_name,
        btmw_rpc_test_gatts_event_handle,
        &s_btmw_rpc_gatts_servers[index])))
    {
        BTMW_RPC_TEST_Logd("register server fail %d, free it\n", ret);
        btmw_rpc_test_gatts_free_server_by_index(index);

        return -1;
    }

    return index;
}



static int btmw_rpc_test_gatts_register_server(int argc, char **argv)
{
    return btmw_rpc_test_gatts_register_server_by_name(argv[0]);
}

static int btmw_rpc_test_gatts_unregister_server_by_name(char *server_name)
{
    INT32 index = 0, ret = 0;
    index = btmw_rpc_test_gatts_find_server_by_name(server_name);

    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", server_name);
        return 0;
    }

    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", server_name);
        return 0;
    }

    BTMW_RPC_TEST_Logd("unregister server: %s, server_if: %d\n",
        server_name, s_btmw_rpc_gatts_servers[index].server_if);

    btmw_rpc_test_gatts_free_server_services(s_btmw_rpc_gatts_servers[index].server_if);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index].server_if);
    if (BT_SUCCESS != ret)
    {
        BTMW_RPC_TEST_Logw("server %s unregister fail(%d)", server_name, ret);
    }

    btmw_rpc_test_gatts_free_server_by_index(index);

    return 0;
}

static int btmw_rpc_test_gatts_unregister_server(int argc, char **argv)
{
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS unregister_server <name>\n");
        return -1;
    }

    return btmw_rpc_test_gatts_unregister_server_by_name(argv[0]);
}

static int btmw_rpc_test_gatts_connect(int argc, char **argv)
{
    BT_GATTS_CONNECT_PARAM conn_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS connect <server_name> <addr> [isDirect <true|false> [<auto|bredr|ble>]]\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }
    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&conn_param, 0, sizeof(conn_param));
    conn_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
    strncpy(conn_param.addr, argv[1], MAX_BDADDR_LEN);
    conn_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    if (argc >= 3)
    {
        conn_param.is_direct = btmw_rpc_test_gatts_parse_value(argv[2]);
    }
    if (argc >= 4)
    {
        // set transport, opt.
        if (0 == strncmp(argv[3], "auto", 4))
        {
            conn_param.transport = GATT_TRANSPORT_TYPE_AUTO;
        }
        else if (0 == strncmp(argv[3], "bredr", 5))
        {
            conn_param.transport = GATT_TRANSPORT_TYPE_BREDR;
        }
        else if (0 == strncmp(argv[3], "ble", 3))
        {
            conn_param.transport = GATT_TRANSPORT_TYPE_LE;
        }
    }
    ret = a_mtkapi_bt_gatts_connect(&conn_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("server %s connect %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}

static int btmw_rpc_test_gatts_disconnect(int argc, char **argv)
{
    BT_GATTS_DISCONNECT_PARAM disc_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS disconnect <server_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&disc_param, 0, sizeof(disc_param));
    disc_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
    strncpy(disc_param.addr, argv[1], MAX_BDADDR_LEN);
    disc_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gatts_disconnect(&disc_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("server %s connect %s fail %d", argv[0], argv[1], ret);
    }
    return ret;
}


static VOID btmw_rpc_test_gatts_free_service(BTMW_RPC_TEST_GATTS_SERVICE *service)
{
    /* free include */
    BTMW_RPC_TEST_GATTS_INCLUDE *incl = NULL;
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;

    if (NULL == service) return;

    BTMW_RPC_TEST_Logi("free service %s ", service->uuid);

    while (!dl_list_empty(&service->include_list))
    {
        incl = dl_list_first(&service->include_list, BTMW_RPC_TEST_GATTS_INCLUDE, node);
        if (NULL != incl)
        {
            dl_list_del(&incl->node);
            free(incl);
            incl = NULL;
        }
    }

    while (!dl_list_empty(&service->char_list))
    {
        gatt_char = dl_list_first(&service->char_list, BTMW_RPC_TEST_GATTS_CHAR, node);
        if (NULL != gatt_char)
        {
            BTMW_RPC_TEST_GATTS_DESC *gatt_desc = NULL;
            while (!dl_list_empty(&gatt_char->desc_list))
            {
                gatt_desc = dl_list_first(&gatt_char->desc_list, BTMW_RPC_TEST_GATTS_DESC, node);
                if (NULL != gatt_desc)
                {
                    dl_list_del(&gatt_desc->node);
                    free(gatt_desc);
                    gatt_desc = NULL;
                }
            }
            dl_list_del(&gatt_char->node);
            free(gatt_char);
            gatt_char = NULL;
        }
    }

    return;
}

static INT32 btmw_rpc_test_gatts_free_service_by_uuid(
    BTMW_RPC_TEST_GATTS_SERVER *server,
    CHAR *service_uuid)
{
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;

    if (NULL == server)
    {
        BTMW_RPC_TEST_Logi("no server");
        return 0;
    }

    dl_list_for_each(services, &server->services_list,
        BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
        dl_list_for_each(service, &services->service_list,
            BTMW_RPC_TEST_GATTS_SERVICE, node)
        {
            if (!strncasecmp(service->uuid, service_uuid,
                BT_GATT_MAX_UUID_LEN - 1))
            {
                dl_list_del(&service->node);

                BTMW_RPC_TEST_Logi("server %d delete service %s handle=%d",
                    services->server_if, service->uuid, service->handle);
                if (0 != service->handle)
                {
                    BT_GATTS_SERVICE_DEL_PARAM del_param;
                    del_param.server_if = services->server_if;
                    del_param.service_handle = service->handle;
                    INT32 ret = a_mtkapi_bt_gatts_del_service(&del_param);
                    if (ret != BT_SUCCESS)
                    {
                        BTMW_RPC_TEST_Logw("server %s delete %s fail %d",
                            server->server_name, service_uuid, ret);
                    }
                }

                btmw_rpc_test_gatts_free_service(service);
                free(service);
                //service = NULL; //coverity check don't allow to assign but no use
                return 0;
            }
        }
    }

    BTMW_RPC_TEST_Logi("server %d delete service %s not found",
        services->server_if, service_uuid);

    return 0;
}

static INT32 btmw_rpc_test_gatts_free_services(BTMW_RPC_TEST_GATTS_SERVICES *services)
{
    /* free include */
    BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;

    if (NULL == services) return 0;

    BTMW_RPC_TEST_Logi("free services xml %s ", services->xml_name);

    while (!dl_list_empty(&services->service_list))
    {
        service = dl_list_first(&services->service_list, BTMW_RPC_TEST_GATTS_SERVICE, node);
        if (NULL != service)
        {
            BTMW_RPC_TEST_Logi("free service %s ", service->uuid);
            dl_list_del(&service->node);
            if (0 != service->handle)
            {
                BT_GATTS_SERVICE_DEL_PARAM del_param;
                del_param.server_if = services->server_if;
                del_param.service_handle = service->handle;

                BTMW_RPC_TEST_Logi("delete service %s ", service->uuid);
                {
                    INT32 ret = a_mtkapi_bt_gatts_del_service(&del_param);
                    if (ret != BT_SUCCESS)
                    {
                        BTMW_RPC_TEST_Logw("delete %s fail %d", service->uuid, ret);
                    }
                }
            }

            btmw_rpc_test_gatts_free_service(service);
            free(service);
            service = NULL;
        }
    }
    xmlFreeDoc(services->doc);
    services->doc = NULL;

    return 0;
}


static INT32 btmw_rpc_test_gatts_free_server_services(INT32 server_if)
{
    BTMW_RPC_TEST_GATTS_SERVER *server =
        btmw_rpc_test_gatts_find_server_by_if(server_if);
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    BTMW_RPC_TEST_GATTS_CONNECTION *conn = NULL;
    BTMW_RPC_TEST_GATTS_WRITE_REQ *write_req = NULL;

    if (NULL == server)
    {
        BTMW_RPC_TEST_Logi("no server %d ", server_if);
        return 0;
    }

    BTMW_RPC_TEST_Logi("free server %d", server_if);

    while (!dl_list_empty(&server->services_list))
    {
        services = dl_list_first(&server->services_list, BTMW_RPC_TEST_GATTS_SERVICES, node);
        if (NULL != services)
        {
            dl_list_del(&services->node);
            btmw_rpc_test_gatts_free_services(services);
            free(services);
            services = NULL;
        }
    }

    while (!dl_list_empty(&server->connection_list))
    {
        conn = dl_list_first(&server->connection_list, BTMW_RPC_TEST_GATTS_CONNECTION, node);
        if (NULL != conn)
        {
            dl_list_del(&conn->node);
            free(conn);
            conn = NULL;
        }
    }

    while (!dl_list_empty(&server->write_req_list))
    {
        write_req = dl_list_first(&server->write_req_list, BTMW_RPC_TEST_GATTS_WRITE_REQ, node);
        if (NULL != write_req)
        {
            dl_list_del(&write_req->node);
            free(write_req);
            write_req = NULL;
        }
    }

    return 0;
}

static UINT16 btmw_rpc_test_gatts_change_char_value(INT32 server_if,
    CHAR *service_uuid, CHAR *char_uuid, CHAR *value)
{
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    BTMW_RPC_TEST_GATTS_SERVER *server =
        btmw_rpc_test_gatts_find_server_by_if(server_if);

    if (NULL == server) return 0;

    dl_list_for_each(services, &server->services_list,
        BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
        dl_list_for_each(service, &services->service_list,
            BTMW_RPC_TEST_GATTS_SERVICE, node)
        {
            if (!strncasecmp(service->uuid, service_uuid,
                BT_GATT_MAX_UUID_LEN - 1))
            {
                BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
                dl_list_for_each(gatt_char, &service->char_list,
                    BTMW_RPC_TEST_GATTS_CHAR, node)
                {
                    if (!strncasecmp(gatt_char->uuid, char_uuid,
                        BT_GATT_MAX_UUID_LEN - 1))
                    {
                        strncpy((CHAR*)gatt_char->value, value, BT_GATT_MAX_VALUE_LEN - 1);
                        gatt_char->value[BT_GATT_MAX_VALUE_LEN - 1] = '\0';
                        gatt_char->len = strlen((CHAR*)gatt_char->value);

                        xmlNodeSetContentLen(gatt_char->xml_node,
                            (xmlChar*)gatt_char->value, gatt_char->len);
                        {
                            CHAR xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN];
                            strncpy(xml_name, services->xml_name, BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
                            xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';

                            xmlSaveFormatFile(strncat(xml_name, ".dat",
                                BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1), services->doc, 1);
                        }

                        return gatt_char->handle;
                    }
                }
            }
        }
    }

    return 0;
}



static INT32 btmw_rpc_test_gatts_load_desc_from_xml(
    BTMW_RPC_TEST_GATTS_DESC *gatt_desc,
    xmlNode *desc_node)
{
    UINT32 attr_cnt = 0;
    if (NULL == gatt_desc || NULL == desc_node)
    {
        BTMW_RPC_TEST_Logw("null parameter");
        return 0;
    }
    /* parse char parameter */
    xmlAttrPtr attr = desc_node->properties;
    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST "uuid", 4))
        {
            xmlChar *attr_value = xmlGetProp(desc_node, BAD_CAST "uuid");

            strncpy(gatt_desc->uuid, (CHAR*)attr_value, BT_GATT_MAX_UUID_LEN - 1);
            gatt_desc->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

            BTMW_RPC_TEST_Logw("desc uuid: %s", attr_value);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "perm", 4))
        {
            xmlChar *attr_value = xmlGetProp(desc_node, BAD_CAST "perm");
            gatt_desc->perm = btmw_rpc_test_gatts_parse_value((CHAR*)attr_value);;

            BTMW_RPC_TEST_Logw("desc perm: 0x%x", gatt_desc->perm);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "key_size", 8))
        {
            xmlChar *attr_value = xmlGetProp(desc_node, BAD_CAST "key_size");
            gatt_desc->key_size = btmw_rpc_test_gatts_parse_value((CHAR*)attr_value);;

            BTMW_RPC_TEST_Logw("desc key_size: %u", gatt_desc->key_size);

            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    /* desc value */
    {
        xmlChar *desc_value = xmlNodeGetContent(desc_node);
        memset((void*)&gatt_desc->value, 0, sizeof(gatt_desc->value));
        strncpy((CHAR*)gatt_desc->value, (CHAR*)desc_value, BT_GATT_MAX_VALUE_LEN - 1);
        gatt_desc->value[BT_GATT_MAX_VALUE_LEN - 1] = '\0';
        gatt_desc->len = strlen((CHAR*)desc_value);

        BTMW_RPC_TEST_Logw("desc value: %s", desc_value);

        xmlFree(desc_value);
        attr_cnt++;
    }

    return attr_cnt;
}

static INT32 btmw_rpc_test_gatts_load_char_value_desc_from_xml(
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char,
    xmlNode *char_value_desc_node)
{
    BTMW_RPC_TEST_GATTS_DESC *desc = NULL;
    UINT32 attr_cnt = 0;
    if (NULL == char_value_desc_node)
    {
        BTMW_RPC_TEST_Logw("char_value_desc_node is NULL");
        return 0;
    }
    if (NULL == gatt_char)
    {
        BTMW_RPC_TEST_Logw("gatt_char is NULL");
        return 0;
    }

    do
    {
        if (char_value_desc_node->type == XML_ELEMENT_NODE)
        {
            /* char value */
            if (!xmlStrncmp(char_value_desc_node->name,
                (xmlChar *)BTMW_RPC_TEST_GATTS_XML_CHAR_VAL,
                strlen(BTMW_RPC_TEST_GATTS_XML_CHAR_VAL)))
            {
                xmlChar *char_value = xmlNodeGetContent(char_value_desc_node);
                memset((void*)&gatt_char->value, 0, sizeof(gatt_char->value));
                strncpy((CHAR*)gatt_char->value, (CHAR*)char_value, BT_GATT_MAX_VALUE_LEN - 1);
                gatt_char->value[BT_GATT_MAX_VALUE_LEN - 1] = '\0';
                gatt_char->len = strlen((CHAR*)gatt_char->value);

                gatt_char->xml_node = char_value_desc_node;

                BTMW_RPC_TEST_Logw("char %s value: %s", gatt_char->uuid, char_value);

                xmlFree(char_value);
            }

            /*  desc */
            if (!xmlStrncmp(char_value_desc_node->name,
                (xmlChar *)BTMW_RPC_TEST_GATTS_XML_DESC,
                strlen(BTMW_RPC_TEST_GATTS_XML_DESC)))
            {
                desc = (BTMW_RPC_TEST_GATTS_DESC *)
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTS_DESC));
                if (NULL == desc)
                {
                    BTMW_RPC_TEST_Logw("alloc char service fail");
                    return -1;
                }
                memset((void*)desc, 0,
                    sizeof(BTMW_RPC_TEST_GATTS_DESC));
                desc->xml_node = char_value_desc_node;
                attr_cnt++;

                btmw_rpc_test_gatts_load_desc_from_xml(desc, char_value_desc_node);

                dl_list_add_tail(&gatt_char->desc_list, &desc->node);
            }
        }
        char_value_desc_node = char_value_desc_node->next;
    } while (char_value_desc_node != NULL);

    return attr_cnt;
}


static INT32 btmw_rpc_test_gatts_load_char_from_xml(
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char,
    xmlNode *char_node)
{
    INT32 attr_cnt = 0;
    if (NULL == gatt_char || NULL == char_node)
    {
        BTMW_RPC_TEST_Logw("null parameter");
        return 0;
    }
    /* parse char parameter */
    xmlAttrPtr attr = char_node->properties;
    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST "uuid", 4))
        {
            xmlChar *attr_value = xmlGetProp(char_node, BAD_CAST "uuid");

            strncpy(gatt_char->uuid, (CHAR*)attr_value, BT_GATT_MAX_UUID_LEN - 1);
            gatt_char->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
            BTMW_RPC_TEST_Logw("char uuid: %s", attr_value);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "perm", 4))
        {
            xmlChar *attr_value = xmlGetProp(char_node, BAD_CAST "perm");
            gatt_char->perm = btmw_rpc_test_gatts_parse_value((CHAR*)attr_value);;;
            BTMW_RPC_TEST_Logw("char perm: 0x%x", gatt_char->perm);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "key_size", 8))
        {
            xmlChar *attr_value = xmlGetProp(char_node, BAD_CAST "key_size");
            gatt_char->key_size = btmw_rpc_test_gatts_parse_value((CHAR*)attr_value);;
            BTMW_RPC_TEST_Logw("char key_size: %u", gatt_char->key_size);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "prop", 4))
        {
            xmlChar *attr_value = xmlGetProp(char_node, BAD_CAST "prop");
            gatt_char->prop = btmw_rpc_test_gatts_parse_value((CHAR*)attr_value);
            BTMW_RPC_TEST_Logw("char prop: 0x%x", gatt_char->prop);

            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    xmlNode *child_node = char_node->xmlChildrenNode;
    if (NULL != child_node)
    {
        attr_cnt = btmw_rpc_test_gatts_load_char_value_desc_from_xml(gatt_char, child_node);
    }

    return attr_cnt;
}

static INT32 btmw_rpc_test_gatts_load_include_char_from_xml(
    BTMW_RPC_TEST_GATTS_SERVICE *service,
    xmlNode *include_char_node)
{
    INT32 attr_cnt = 0;
    BTMW_RPC_TEST_GATTS_INCLUDE *include_service = NULL;
    BTMW_RPC_TEST_GATTS_CHAR *gatt_char = NULL;
    if (NULL == include_char_node)
    {
        BTMW_RPC_TEST_Logw("service_node is NULL");
        return 0;
    }
    if (NULL == service)
    {
        BTMW_RPC_TEST_Logw("service is NULL");
        return 0;
    }

    do
    {
        if (include_char_node->type == XML_ELEMENT_NODE)
        {
            /* include service */
            if (!xmlStrncmp(include_char_node->name,
                (xmlChar *)BTMW_RPC_TEST_GATTS_XML_INCLUDE,
                strlen(BTMW_RPC_TEST_GATTS_XML_INCLUDE)))
            {
                include_service = (BTMW_RPC_TEST_GATTS_INCLUDE *)
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTS_INCLUDE));
                if (NULL == include_service)
                {
                    BTMW_RPC_TEST_Logw("alloc include service fail");
                    return -1;
                }
                memset((void*)include_service, 0,
                    sizeof(BTMW_RPC_TEST_GATTS_INCLUDE));

                dl_list_add_tail(&service->include_list, &include_service->node);
                include_service->xml_node = include_char_node;
                if (xmlHasProp(include_char_node, BAD_CAST "uuid"))
                {
                    xmlChar *attr_value = xmlGetProp(include_char_node, BAD_CAST "uuid");

                    strncpy(include_service->uuid, (CHAR*)attr_value, BT_GATT_MAX_UUID_LEN - 1);
                    include_service->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

                    include_service->handle =
                        btmw_rpc_test_gatts_get_include_handle_by_uuid(include_service->uuid);
                    BTMW_RPC_TEST_Logw("include uuid: %s, handle: %d",
                        attr_value, include_service->handle);

                    xmlFree(attr_value);
                    attr_cnt++;
                    if (0 == include_service->handle)
                    {
                        BTMW_RPC_TEST_Logw("include uuid: %s, handle: %d not registered",
                            include_service->uuid, include_service->handle);
                        return -1;
                    }
                }
                else
                {
                    BTMW_RPC_TEST_Logw("include service don't have uuid");
                }
            }

            /*  char */
            if (!xmlStrncmp(include_char_node->name,
                (xmlChar *)BTMW_RPC_TEST_GATTS_XML_CHAR,
                strlen(BTMW_RPC_TEST_GATTS_XML_CHAR)))
            {
                INT32 tmp_cnt = 0;
                gatt_char = (BTMW_RPC_TEST_GATTS_CHAR *)
                    calloc(1, sizeof(BTMW_RPC_TEST_GATTS_CHAR));
                if (NULL == gatt_char)
                {
                    BTMW_RPC_TEST_Logw("alloc char service fail");
                    return -1;
                }
                memset((void*)gatt_char, 0,
                    sizeof(BTMW_RPC_TEST_GATTS_CHAR));
                dl_list_init(&gatt_char->desc_list);
                dl_list_init(&gatt_char->node);
                gatt_char->xml_node = include_char_node;
                attr_cnt++;

                tmp_cnt = btmw_rpc_test_gatts_load_char_from_xml(gatt_char, include_char_node);
                dl_list_add_tail(&service->char_list, &gatt_char->node);

                if (0 > tmp_cnt) return -1;

                attr_cnt += tmp_cnt;
            }
        }
        include_char_node = include_char_node->next;
    } while (include_char_node != NULL);

    return attr_cnt;
}

static UINT16 btmw_rpc_test_gatts_get_include_handle_by_uuid(CHAR *uuid)
{
    UINT32 i = 0;

    for (i = 0;i < BTMW_RPC_TEST_GATTS_TEST_APP_CNT;i++)
    {
        BTMW_RPC_TEST_GATTS_SERVER *server = &s_btmw_rpc_gatts_servers[i];
        BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;

        dl_list_for_each(services, &server->services_list,
            BTMW_RPC_TEST_GATTS_SERVICES, node)
        {
            BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
            dl_list_for_each(service, &services->service_list,
                BTMW_RPC_TEST_GATTS_SERVICE, node)
            {
                BTMW_RPC_TEST_Logi("check server %d service %s",
                    server->server_if, service->uuid);
                if (!strncasecmp(service->uuid, uuid, BT_GATT_MAX_UUID_LEN - 1))
                {
                    return service->handle;
                }
            }
        }
    }

    return 0;
}




static INT32 btmw_rpc_test_gatts_register_service_from_xml(
    INT32 server_if,
    BTMW_RPC_TEST_GATTS_SERVICE *service,
    UINT32 attr_cnt)
{
    BT_GATTS_SERVICE_ADD_PARAM service_add_param;
    /* add char */
    BTMW_RPC_TEST_GATTS_CHAR *chr = NULL;
    /* add include */
    BTMW_RPC_TEST_GATTS_INCLUDE *incl = NULL;
    UINT32 attr_index = 0;

    if (NULL == service || 0 == attr_cnt)
    {
        BTMW_RPC_TEST_Loge("service NULL, or attr cnt(%d) invalid", attr_cnt);
        return -1;
    }

    memset((void*)&service_add_param, 0, sizeof(service_add_param));
    service_add_param.server_if = server_if;

    service_add_param.attr_cnt = attr_cnt;
    strncpy(service_add_param.service_uuid, service->uuid,
        BT_GATT_MAX_UUID_LEN - 1);
    service_add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    /* fill attribute */
    service_add_param.attrs = (BT_GATT_ATTR *)calloc(1, sizeof(BT_GATT_ATTR)*service_add_param.attr_cnt);
    if (NULL == service_add_param.attrs)
    {
        BTMW_RPC_TEST_Loge("alloc attrs fail");
        return -1;
    }

    /* add service */
    strncpy(service_add_param.attrs[attr_index].uuid, service->uuid,
        BT_GATT_MAX_UUID_LEN - 1);
    service_add_param.attrs[attr_index].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    service_add_param.attrs[attr_index].type = service->type;
    attr_index++;

    dl_list_for_each(incl, &service->include_list, BTMW_RPC_TEST_GATTS_INCLUDE, node)
    {
        strncpy(service_add_param.attrs[attr_index].uuid, incl->uuid,
            BT_GATT_MAX_UUID_LEN - 1);
        service_add_param.attrs[attr_index].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
        service_add_param.attrs[attr_index].type = GATT_ATTR_TYPE_INCLUDED_SERVICE;
        service_add_param.attrs[attr_index].handle = incl->handle;
        if (0 == service_add_param.attrs[attr_index].handle)
        {
            BTMW_RPC_TEST_Loge("include service %s not registered",
                service_add_param.attrs[attr_index].uuid);
            goto __ERROR;
        }
        attr_index++;
    }

    dl_list_for_each(chr, &service->char_list, BTMW_RPC_TEST_GATTS_CHAR, node)
    {
        strncpy(service_add_param.attrs[attr_index].uuid, chr->uuid,
            BT_GATT_MAX_UUID_LEN - 1);
        service_add_param.attrs[attr_index].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
        service_add_param.attrs[attr_index].type = GATT_ATTR_TYPE_CHARACTERISTIC;
        service_add_param.attrs[attr_index].properties = chr->prop;
        service_add_param.attrs[attr_index].permissions = chr->perm;
        service_add_param.attrs[attr_index].key_size = chr->key_size;
        attr_index++;

        /* add desc */
        BTMW_RPC_TEST_GATTS_DESC *desc = NULL;
        dl_list_for_each(desc, &chr->desc_list, BTMW_RPC_TEST_GATTS_DESC, node)
        {
            strncpy(service_add_param.attrs[attr_index].uuid, desc->uuid,
                BT_GATT_MAX_UUID_LEN - 1);
            service_add_param.attrs[attr_index].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
            service_add_param.attrs[attr_index].type = GATT_ATTR_TYPE_DESCRIPTOR;
            service_add_param.attrs[attr_index].permissions = desc->perm;
            service_add_param.attrs[attr_index].key_size = desc->key_size;
            attr_index++;
        }
    }

    BTMW_RPC_TEST_Loge("server_if: %d, attr_index=%d, attr_cnt=%d", server_if, attr_index, attr_cnt);
    if (BT_SUCCESS != a_mtkapi_bt_gatts_add_service(&service_add_param))
    {
        BTMW_RPC_TEST_Loge("server %d add service %s fail", server_if, service->uuid);
        goto __ERROR;
    }

    if (NULL != service_add_param.attrs)
    {
        free(service_add_param.attrs);
        //service_add_param.attrs = NULL; //coverity check don't allow to assign but no use
    }

    return 0;

__ERROR:
    if (NULL != service_add_param.attrs)
    {
        free(service_add_param.attrs);
        //service_add_param.attrs = NULL; //coverity check don't allow to assign but no use
    }

    return -1;
}


static INT32 btmw_rpc_test_gatts_load_service_from_xml(
    BTMW_RPC_TEST_GATTS_SERVICES *services,
    xmlNode *service_node)
{
    BTMW_RPC_TEST_GATTS_SERVICE *service = NULL;
    UINT32 attr_cnt = 0;

    if (NULL == service_node)
    {
        BTMW_RPC_TEST_Logw("service_node is NULL");
        return -1;
    }
    if (NULL == services)
    {
        BTMW_RPC_TEST_Logw("services is NULL");
        return -1;
    }

    if (0 != xmlStrncmp(service_node->name,
        (xmlChar *)BTMW_RPC_TEST_GATTS_XML_SERVICE,
        strlen(BTMW_RPC_TEST_GATTS_XML_SERVICE)))
    {
        BTMW_RPC_TEST_Logw("service_node is not "BTMW_RPC_TEST_GATTS_XML_SERVICE);
        return -1;
    }

    service = calloc(1, sizeof(BTMW_RPC_TEST_GATTS_SERVICE));
    if (NULL == service)
    {
        BTMW_RPC_TEST_Logw("alloc service fail");
        return -1;
    }

    memset((void*)service, 0, sizeof(BTMW_RPC_TEST_GATTS_SERVICE));
    dl_list_init(&service->include_list);
    dl_list_init(&service->char_list);
    dl_list_init(&service->node);
    service->xml_node = service_node;

    /* defautl primary service */
    service->type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    xmlAttrPtr attr = service_node->properties;
    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST "uuid", 4))
        {
            xmlChar *attr_value = xmlGetProp(service_node, BAD_CAST "uuid");

            strncpy(service->uuid, (CHAR*)attr_value, BT_GATT_MAX_UUID_LEN - 1);
            service->uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

            BTMW_RPC_TEST_Logw("service uuid: %s", attr_value);

            xmlFree(attr_value);
        }

        if (!xmlStrncmp(attr->name, BAD_CAST "type", 4))
        {
            xmlChar *attr_value = xmlGetProp(service_node, BAD_CAST "type");

            if (!xmlStrncmp(BAD_CAST(attr_value), BAD_CAST "primary", strlen("primary")))
            {
                service->type = GATT_ATTR_TYPE_PRIMARY_SERVICE;
            }
            else
            {
                service->type = GATT_ATTR_TYPE_SECONDARY_SERVICE;
            }
            BTMW_RPC_TEST_Logw("service type: %s", attr_value);

            xmlFree(attr_value);
        }

        attr = attr->next;
    }

    attr_cnt = 1; /* this is for service */
    xmlNode *child_node = service_node->xmlChildrenNode;
    if (NULL != child_node)
    {
        INT32 tmp_cnt = btmw_rpc_test_gatts_load_include_char_from_xml(service, child_node);
        if (0 > tmp_cnt) goto __ERROR;

        attr_cnt += tmp_cnt;
    }

    dl_list_add_tail(&services->service_list, &service->node);

    BTMW_RPC_TEST_Logw("load service %s, attr_count=%d", service->uuid, attr_cnt);

    if (0 > btmw_rpc_test_gatts_register_service_from_xml(services->server_if,
        service, attr_cnt))
    {
        goto __ERROR;
    }

    return 0;

__ERROR:
    BTMW_RPC_TEST_Logw("service %s load and register fail, free it", service->uuid);
    dl_list_del(&service->node);
    btmw_rpc_test_gatts_free_service(service);
    if (NULL != service)
    {
        free(service);
        //service = NULL; //coverity check don't allow to assign but no use
    }
    return -1;
}

static int btmw_rpc_test_gatts_load_services_from_xml(
    BTMW_RPC_TEST_GATTS_SERVER *gatts_server,
    CHAR *service_xml_path)
{
    BTMW_RPC_TEST_GATTS_SERVICES *services =
        calloc(1, sizeof(BTMW_RPC_TEST_GATTS_SERVICES));
    if (NULL == services)
    {
        BTMW_RPC_TEST_Logw("alloc services fail");
        goto __ERROR;
    }

    memset((void*)services, 0, sizeof(BTMW_RPC_TEST_GATTS_SERVICES));
    dl_list_init(&services->service_list);
    services->server_if = gatts_server->server_if;

    LIBXML_TEST_VERSION;

    if (strlen(service_xml_path) >= BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1)
    {
        BTMW_RPC_TEST_Logw("xml file name (%s) too long", service_xml_path);
        goto __ERROR;
    }
    strncpy(services->xml_name, service_xml_path,
        BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1);
    services->xml_name[BTMW_RPC_TEST_GATTS_XML_NAME_LEN - 1] = '\0';
    services->doc = xmlReadFile(service_xml_path, NULL, 0);
    if (services->doc == NULL)
    {
        BTMW_RPC_TEST_Logw("xmlReadFile %s fail", service_xml_path);
        goto __ERROR;
    }

    services->xml_root = xmlDocGetRootElement(services->doc);
    if (NULL == services->xml_root)
    {
        BTMW_RPC_TEST_Logw("xml root read fail");
        goto __ERROR;
    }

    if (0 == xmlStrncmp(services->xml_root->name,
        (xmlChar *)BTMW_RPC_TEST_GATTS_XML_SERVICES,
        strlen(BTMW_RPC_TEST_GATTS_XML_SERVICES)))
    {
        xmlNode *service_node = services->xml_root->xmlChildrenNode;
        BTMW_RPC_TEST_GATTS_LOCK(btmw_rpc_test_gatts_lock);
        dl_list_add_tail(&gatts_server->services_list, &services->node);
        while(NULL != service_node)
        {
            if (service_node->type == XML_ELEMENT_NODE)
            {
                btmw_rpc_test_gatts_load_service_from_xml(services,
                    service_node);
            }
            service_node = service_node->next;
        }
        BTMW_RPC_TEST_GATTS_UNLOCK(btmw_rpc_test_gatts_lock);
        BTMW_RPC_TEST_Logi("gatt service parse finish ");
    }
    else
    {
        BTMW_RPC_TEST_Logw("xml root is not "BTMW_RPC_TEST_GATTS_XML_SERVICES);
        goto __ERROR;
    }


    xmlCleanupParser();

    return 0;

__ERROR:
    if (NULL != services)
    {
        btmw_rpc_test_gatts_free_services(services);

        if (NULL != services->xml_root)
        {
            xmlFreeNode(services->xml_root);
            services->xml_root = NULL;
        }
        if (NULL != services->doc)
        {
            xmlFreeDoc(services->doc);
            services->doc = NULL;
        }

        free(services);
        //services = NULL; //coverity check don't allow to assign but no use
    }
    xmlCleanupParser();
    return -1;
}

static int btmw_rpc_test_gatts_add_service(int argc, char **argv)
{
    INT32 index = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS add_service <server_name> <service_xml_path>\n");
        return -1;
    }

    BTMW_RPC_TEST_Logi("from %s\n", argv[1]);

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (0 > btmw_rpc_test_gatts_load_services_from_xml(
        &s_btmw_rpc_gatts_servers[index], argv[1]))
    {
        BTMW_RPC_TEST_Loge("load service from xml %s fail", argv[1]);
        return 0;
    }

    return 0;
}

static int btmw_rpc_test_gatts_delete_service (int argc, char **argv)
{
    INT32 index = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS delete_service <server_name> <service_uuid>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    return btmw_rpc_test_gatts_free_service_by_uuid(
        &s_btmw_rpc_gatts_servers[index], argv[1]);

}

static int btmw_rpc_test_gatts_delete_services (int argc, char **argv)
{
    INT32 index = 0;
    BTMW_RPC_TEST_GATTS_SERVER *server = NULL;
    BTMW_RPC_TEST_GATTS_SERVICES *services = NULL;
    BTMW_RPC_TEST_GATTS_SERVICES *temp = NULL;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS delele_services <server_name> <service_xml_path>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    server = &s_btmw_rpc_gatts_servers[index];
    dl_list_for_each_safe(services, temp, &server->services_list, BTMW_RPC_TEST_GATTS_SERVICES, node)
    {
        if (!strncmp(services->xml_name, argv[1], BTMW_RPC_TEST_GATTS_XML_NAME_LEN))
        {
            dl_list_del(&services->node);
            btmw_rpc_test_gatts_free_services(services);
            free(services);
            //services = NULL; //coverity check don't allow to assign but no use
            return 0;
        }
    }

    BTMW_RPC_TEST_Logw("no found servers %s", argv[1]);

    return 0;
}

static int btmw_rpc_test_gatts_send_indication(int argc, char **argv)
{
    INT32 index = 0;
    UINT16 char_handle = 0;

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS send_indication <server_name> <service_uuid> <char_uuid> <addr> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    char_handle = btmw_rpc_test_gatts_change_char_value(
        s_btmw_rpc_gatts_servers[index].server_if,
        argv[1], argv[2], argv[4]);
    if (0 != char_handle)
    {
        BT_GATTS_IND_PARAM ind_param;
        memset((void*)&ind_param, 0, sizeof(ind_param));
        ind_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
        strncpy(ind_param.addr, argv[3], MAX_BDADDR_LEN);
        ind_param.addr[MAX_BDADDR_LEN - 1] = '\0';
        ind_param.char_handle = char_handle;
        ind_param.need_confirm = 1;
        ind_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
        memcpy((void*)ind_param.value, (void*)argv[4], ind_param.value_len);
        INT32 ret = a_mtkapi_bt_gatts_send_indication(&ind_param);
        if (ret != BT_SUCCESS)
        {
            BTMW_RPC_TEST_Logw("server %s send ind %s fail %d", argv[0], argv[1], ret);
        }
    }

    return 0;
}

static int btmw_rpc_test_gatts_send_notification (int argc, char **argv)
{
    INT32 index = 0;
    UINT16 char_handle = 0;

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS send_notify <server_name> <service_uuid> <char_uuid> <addr> <value>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    char_handle = btmw_rpc_test_gatts_change_char_value(
        s_btmw_rpc_gatts_servers[index].server_if,
        argv[1], argv[2],  argv[4]);
    if (0 != char_handle)
    {
        BT_GATTS_IND_PARAM ind_param;
        memset((void*)&ind_param, 0, sizeof(ind_param));
        ind_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
        strncpy(ind_param.addr, argv[3], MAX_BDADDR_LEN);
        ind_param.addr[MAX_BDADDR_LEN - 1] = '\0';
        ind_param.char_handle = char_handle;
        ind_param.need_confirm = 0;
        ind_param.value_len = BT_GATT_MAX_VALUE_LEN > strlen(argv[4]) ? strlen(argv[4]) : BT_GATT_MAX_VALUE_LEN;
        memcpy((void*)ind_param.value, (void*)argv[4], ind_param.value_len);
        INT32 ret = a_mtkapi_bt_gatts_send_indication(&ind_param);
        if (ret != BT_SUCCESS)
        {
            BTMW_RPC_TEST_Logw("server %s send notify %s fail %d", argv[0], argv[1], ret);
        }
    }

    return 0;
}



static int btmw_rpc_test_gatts_read_phy(int argc, char **argv)
{
    BT_GATTS_PHY_READ_PARAM phy_read_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS read_phy <server_name> <addr>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&phy_read_param, 0, sizeof(phy_read_param));
    phy_read_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
    strncpy(phy_read_param.addr, argv[1], MAX_BDADDR_LEN);
    phy_read_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    ret = a_mtkapi_bt_gatts_read_phy(&phy_read_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("server %s read phy %s fail %d", argv[0], argv[1], ret);
    }
    return 0;
}

static int btmw_rpc_test_gatts_set_pref_phy(int argc, char **argv)
{
    BT_GATTS_PHY_SET_PARAM phy_set_param;
    INT32 index = 0;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS set_pref_phy <server_name> <addr> <tx_phy> <rx_phy> <phy_option>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    memset((void*)&phy_set_param, 0, sizeof(phy_set_param));
    phy_set_param.server_if = s_btmw_rpc_gatts_servers[index].server_if;
    strncpy(phy_set_param.addr, argv[1], MAX_BDADDR_LEN);
    phy_set_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    phy_set_param.tx_phy = atoi(argv[2]);
    phy_set_param.rx_phy = atoi(argv[3]);
    phy_set_param.phy_options = atoi(argv[4]);

    ret = a_mtkapi_bt_gatts_set_prefer_phy(&phy_set_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("server %s set phy %s fail %d", argv[0], argv[1], ret);
    }
    return 0;
}


static int btmw_rpc_test_gatts_throughput_test(int argc, char **argv)
{
    INT32 index = 0;
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS throughput_test <server_name> <start|stop>\n");
        return -1;
    }

    index = btmw_rpc_test_gatts_find_server_by_name(argv[0]);
    if (index < 0)
    {
        BTMW_RPC_TEST_Logw("no found server %s", argv[0]);
        return 0;
    }

    if (s_btmw_rpc_gatts_servers[index].server_if <= 0)
    {
        BTMW_RPC_TEST_Logw("server %s has not registered", argv[0]);
        return 0;
    }

    if (0 == strncmp(argv[1], "start", 5))
    {
        s_btmw_rpc_gatts_servers[index].tp_test = 1;
        s_btmw_rpc_gatts_servers[index].rx_pkt = 0;
        s_btmw_rpc_gatts_servers[index].rx_length = 0;
    }
    else
    {
        float time_use = 0;
        float tp = 0;

        s_btmw_rpc_gatts_servers[index].tp_test = 0;

        time_use = (s_btmw_rpc_gatts_servers[index].tp_rx_last.tv_sec -
            s_btmw_rpc_gatts_servers[index].tp_rx_start.tv_sec) * 1000000
            + (s_btmw_rpc_gatts_servers[index].tp_rx_last.tv_usec
                - s_btmw_rpc_gatts_servers[index].tp_rx_start.tv_usec);
        time_use /= 1000000;
        if (time_use > 0.000001 || time_use < -0.000001)
        {
            tp = s_btmw_rpc_gatts_servers[index].rx_length / 1024 / time_use * 8;
        }
        else
        {
            tp = 0;
        }
        BTMW_RPC_TEST_Logi("Data_len: %d Kbytes, %d pkt, time: %f sec, throughput: %f Kbps",
            s_btmw_rpc_gatts_servers[index].rx_length / 1024,
            s_btmw_rpc_gatts_servers[index].rx_pkt,
            time_use, tp);
    }

    return 0;
}


static int btmw_rpc_test_gatts_ut_func000(void)
{
    INT32 ret = 0;
    CHAR server_name[BT_GATT_MAX_NAME_LEN+1];

    ret = a_mtkapi_bt_gatts_register("", btmw_rpc_test_gatts_event_handle, NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    memset((void*)&server_name, 'a', BT_GATT_MAX_NAME_LEN);
    server_name[BT_GATT_MAX_NAME_LEN] = '\0';

    ret = a_mtkapi_bt_gatts_register(server_name, btmw_rpc_test_gatts_event_handle, NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    ret = a_mtkapi_bt_gatts_register("abc", NULL, NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_unregister(0);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    ret = a_mtkapi_bt_gatts_unregister(BTMW_RPC_TEST_GATTS_TEST_APP_CNT);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    ret = a_mtkapi_bt_gatts_unregister(100);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    ret = a_mtkapi_bt_gatts_connect(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_disconnect(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_add_service(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_del_service(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_send_indication(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_send_response(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_read_phy(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_set_prefer_phy(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    ret = a_mtkapi_bt_gatts_set_prefer_phy(NULL);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_NULL_POINTER, ret);

    return 0;
}

static int btmw_rpc_test_gatts_ut_func001(void)
{
    INT32 index = btmw_rpc_test_gatts_register_server_by_name("func001");
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTS_CHECK(index >= 0, index);
    if (index < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index].lock,
        s_btmw_rpc_gatts_servers[index].signal,
        s_btmw_rpc_gatts_servers[index].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index].server_if > 0,
        s_btmw_rpc_gatts_servers[index].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index].status);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index);

    s_btmw_rpc_gatts_servers[index].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

    return 0;
}

static int btmw_rpc_test_gatts_ut_func002(void)
{
    INT32 index = btmw_rpc_test_gatts_register_server_by_name("func002");
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTS_CHECK(index >= 0, index);
    if (index < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index].lock,
        s_btmw_rpc_gatts_servers[index].signal,
        s_btmw_rpc_gatts_servers[index].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index].server_if > 0,
        s_btmw_rpc_gatts_servers[index].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index].status);

    INT32 index2 = a_mtkapi_bt_gatts_register("func002", btmw_rpc_test_gatts_event_handle, NULL);
    BTMW_RPC_TEST_GATTS_CHECK(index2 == BT_ERR_STATUS_DONE, index2);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index);

    s_btmw_rpc_gatts_servers[index].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

    return 0;
}

static int btmw_rpc_test_gatts_ut_func003(void)
{
    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func003-1");
    INT32 ret = 0;
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
        s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    INT32 index2 = btmw_rpc_test_gatts_register_server_by_name("func003-2");
    BTMW_RPC_TEST_GATTS_CHECK(index2 >= 0, index2);
    if (index2 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index2].lock,
        s_btmw_rpc_gatts_servers[index2].signal,
        s_btmw_rpc_gatts_servers[index2].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index2].server_if > 0,
        s_btmw_rpc_gatts_servers[index2].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index2].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index2].status);


    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index2].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index2);


    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index2].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

    return 0;
}

#if 0
static int btmw_rpc_test_gatts_ut_func004(void)
{
    pid_t fpid=fork();
    if (fpid < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
    }
    else if (fpid == 0)
    {
        BTMW_RPC_TEST_Logi("child");

        INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func004-001");
        BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
        if (index1 < 0)
        {
            BTMW_RPC_TEST_Loge(" fail");
            return 0;
        }

        BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
            s_btmw_rpc_gatts_servers[index1].signal,
            s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

        BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
            s_btmw_rpc_gatts_servers[index1].server_if);
        BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
            s_btmw_rpc_gatts_servers[index1].status);

        INT32 index = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
        BTMW_RPC_TEST_GATTS_CHECK(index == BT_SUCCESS, index);

        s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

        exit(0);
    }
    else
    {
        BTMW_RPC_TEST_Logi("parent");
        sleep(5);
        INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func004-002");
        BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
        if (index1 < 0)
        {
            BTMW_RPC_TEST_Loge(" fail");
            return 0;
        }


        BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
            s_btmw_rpc_gatts_servers[index1].signal,
            s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

        BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
            s_btmw_rpc_gatts_servers[index1].server_if);
        BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
            s_btmw_rpc_gatts_servers[index1].status);

        INT32 index = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
        BTMW_RPC_TEST_GATTS_CHECK(index == BT_SUCCESS, index);

        s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    }
}
#endif

static int btmw_rpc_test_gatts_ut_func005(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    INT32 ret = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));

    add_param.server_if = 100;

    strncpy(add_param.service_uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "3001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_CHARACTERISTIC;
    add_param.attr_cnt = 2;

    /* check server if */
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func006(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    INT32 ret = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));

    add_param.server_if = 0;

    strncpy(add_param.service_uuid, "0600", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "0600", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "0601", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_CHARACTERISTIC;
    add_param.attrs[1].permissions = 0x3f;
    add_param.attrs[1].properties = 11;
    add_param.attr_cnt = 2;

    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func006");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        free(add_param.attrs);
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    /* add service check */
    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    add_param.attr_cnt = 2;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func007(void)
{
    CHAR server_name[BT_GATT_MAX_NAME_LEN] = "func007";
    UINT32 reg_cnt = 0;
    INT32 result = 0;

    for (reg_cnt = 0; reg_cnt < 61; reg_cnt++)
    {
        result = snprintf(server_name, BT_GATT_MAX_NAME_LEN - 1, "%s-%03u", "func007", reg_cnt);
        BTMW_RPC_TEST_GATTS_CHECK(result >= 0, result);
        server_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';

        INT32 index1 = btmw_rpc_test_gatts_register_server_by_name(server_name);
        if (reg_cnt < 60)
        {
            BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
            if (index1 < 0)
            {
                BTMW_RPC_TEST_Loge("server_name %s reg fail", server_name);
                break;
            }
        }
        else
        {
            BTMW_RPC_TEST_GATTS_CHECK(index1 < 0, index1);
            break;
        }
        BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
            s_btmw_rpc_gatts_servers[index1].signal,
            s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

        if (reg_cnt < 60)
        {
            BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
                s_btmw_rpc_gatts_servers[index1].status);
            BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
                s_btmw_rpc_gatts_servers[index1].server_if);
        }
    }
    for (reg_cnt = 0; reg_cnt < 60; reg_cnt++)
    {
        INT32 ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[reg_cnt].server_if);
        BTMW_RPC_TEST_Loge("unreg %d ", s_btmw_rpc_gatts_servers[reg_cnt].server_if);
        BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
        btmw_rpc_test_gatts_free_server_by_index(reg_cnt);

        s_btmw_rpc_gatts_servers[reg_cnt].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    }
    return 0;
}


static int btmw_rpc_test_gatts_ut_func012(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    INT32 ret = 0;
    INT32 index1 = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));

    add_param.server_if = 100;

    strncpy(add_param.service_uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "30001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "3001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_CHARACTERISTIC;
    add_param.attrs[1].permissions = 0x3f;
    add_param.attrs[1].properties = 11;
    add_param.attr_cnt = 2;

    /* check server if */
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);


    index1 = btmw_rpc_test_gatts_register_server_by_name("func012");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        free(add_param.attrs);
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    /* add service check */
    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;

    /* check attr_cnt */
    add_param.attr_cnt = 0;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    add_param.attr_cnt = 2;

    /* check type */
    add_param.attrs[0].type = GATT_ATTR_TYPE_DESCRIPTOR + 1;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);

    /* check uuid */
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_ERR_STATUS_PARM_INVALID, ret);


    add_param.attr_cnt = 2;
    strncpy(add_param.attrs[0].uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func015(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    BT_GATTS_SERVICE_DEL_PARAM del_param;
    //UINT16 handle = 0;
    INT32 ret = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));
    memset((void*)&del_param, 0, sizeof(BT_GATTS_SERVICE_DEL_PARAM));

    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func015");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        free(add_param.attrs);
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
        s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    /* add include service check */
    strncpy(add_param.service_uuid, "3001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "3001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    add_param.attr_cnt = 1;

    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle >= 40,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    /* save include service handle */
    //handle = s_btmw_rpc_gatts_servers[index1].service_handle;

    /* add service check */
    strncpy(add_param.service_uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "3000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "3001", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_INCLUDED_SERVICE;
    add_param.attr_cnt = 1;

    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle >= 40,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func016(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    INT32 ret = 0;
    INT32 index1 = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));

    strncpy(add_param.service_uuid, "1600", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "1600", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "1601", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_INCLUDED_SERVICE;
    add_param.attrs[1].handle = 100;
    add_param.attr_cnt = 2;

    index1 = btmw_rpc_test_gatts_register_server_by_name("func016");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        free(add_param.attrs);
        return 0;
    }
    //BTMW_RPC_TEST_Logi("index1=%d, event_mask=0x%x, server=%p",
    //    index1, s_btmw_rpc_gatts_servers[index1].event_mask,
    //    &s_btmw_rpc_gatts_servers[index1]);
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(
        s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_ERROR,
        s_btmw_rpc_gatts_servers[index1].status);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func019(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    BT_GATTS_SERVICE_DEL_PARAM del_param;
    UINT16 handle = 0;
    INT32 ret = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));
    memset((void*)&del_param, 0, sizeof(BT_GATTS_SERVICE_DEL_PARAM));

    strncpy(add_param.service_uuid, "1900", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }
    strncpy(add_param.attrs[0].uuid, "1900", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    strncpy(add_param.attrs[1].uuid, "1901", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].type = GATT_ATTR_TYPE_CHARACTERISTIC;
    add_param.attrs[1].permissions = 0x3f;
    add_param.attrs[1].properties = 11;
    add_param.attr_cnt = 2;

    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func019");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        free(add_param.attrs);
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
        s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    /* add service check */
    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    add_param.attr_cnt = 2;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle >= 40,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    handle = s_btmw_rpc_gatts_servers[index1].service_handle;

    del_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    del_param.service_handle = s_btmw_rpc_gatts_servers[index1].service_handle;
    ret = a_mtkapi_bt_gatts_del_service(&del_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle == handle,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}

static int btmw_rpc_test_gatts_ut_func020(void)
{
    BT_GATTS_SERVICE_DEL_PARAM del_param;
    INT32 ret = 0;

    memset((void*)&del_param, 0, sizeof(BT_GATTS_SERVICE_DEL_PARAM));

    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func020");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    del_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    del_param.service_handle = 1000;
    ret = a_mtkapi_bt_gatts_del_service(&del_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);

    return 0;
}


static int btmw_rpc_test_gatts_ut_func021(void)
{
    BT_GATTS_SERVICE_ADD_PARAM add_param;
    BT_GATTS_SERVICE_DEL_PARAM del_param;
    UINT16 handle = 0;
    INT32 ret = 0;

    memset((void*)&add_param, 0, sizeof(BT_GATTS_SERVICE_ADD_PARAM));
    memset((void*)&del_param, 0, sizeof(BT_GATTS_SERVICE_DEL_PARAM));

    INT32 index1 = btmw_rpc_test_gatts_register_server_by_name("func021");
    BTMW_RPC_TEST_GATTS_CHECK(index1 >= 0, index1);
    if (index1 < 0)
    {
        BTMW_RPC_TEST_Loge(" fail");
        return 0;
    }
    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_REGISTER));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].server_if > 0,
        s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);

    /* add include service check */
    strncpy(add_param.service_uuid, "2000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    add_param.attrs = (BT_GATT_ATTR*)calloc(1, sizeof(BT_GATT_ATTR)*2);
    if (NULL == add_param.attrs)
    {
        BTMW_RPC_TEST_GATTS_CHECK(add_param.attrs != NULL, 0);
        return 0;
    }

    strncpy(add_param.attrs[0].uuid, "2000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    add_param.attr_cnt = 1;

    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle >= 40,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    /* save include service handle */
    handle = s_btmw_rpc_gatts_servers[index1].service_handle;

    /* add service with include service check */
    strncpy(add_param.service_uuid, "2002", BT_GATT_MAX_UUID_LEN - 1);
    add_param.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    strncpy(add_param.attrs[0].uuid, "2002", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[0].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;

    add_param.attrs[1].type = GATT_ATTR_TYPE_INCLUDED_SERVICE;
    strncpy(add_param.attrs[1].uuid, "2000", BT_GATT_MAX_UUID_LEN - 1);
    add_param.attrs[1].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
    add_param.attrs[1].handle = handle;
    add_param.attr_cnt = 2;
    BTMW_RPC_TEST_Logi("include handle=0x%x", handle);

    add_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    ret = a_mtkapi_bt_gatts_add_service(&add_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    BTMW_RPC_TEST_GATTS_WAIT(s_btmw_rpc_gatts_servers[index1].lock,
        s_btmw_rpc_gatts_servers[index1].signal,
        s_btmw_rpc_gatts_servers[index1].event_mask & (1 << BT_GATTS_EVENT_SERVICE_ADD));

    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].status == BT_GATT_STATUS_OK,
        s_btmw_rpc_gatts_servers[index1].status);
    BTMW_RPC_TEST_GATTS_CHECK(s_btmw_rpc_gatts_servers[index1].service_handle >= 40,
        s_btmw_rpc_gatts_servers[index1].service_handle);

    /* del include service */
    del_param.server_if = s_btmw_rpc_gatts_servers[index1].server_if;
    del_param.service_handle = handle;
    ret = a_mtkapi_bt_gatts_del_service(&del_param);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret);

    ret = a_mtkapi_bt_gatts_unregister(s_btmw_rpc_gatts_servers[index1].server_if);
    BTMW_RPC_TEST_GATTS_CHECK(ret == BT_SUCCESS, ret );
    btmw_rpc_test_gatts_free_server_by_index(index1);

    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_REGISTER);
    s_btmw_rpc_gatts_servers[index1].event_mask &= ~(1 << BT_GATTS_EVENT_SERVICE_ADD);

    if (NULL != add_param.attrs)
    {
        free(add_param.attrs);
    }
    return 0;
}


static int btmw_rpc_test_gatts_ut(int argc, char **argv)
{
    btmw_rpc_test_gatts_ut_func000();
    btmw_rpc_test_gatts_ut_func001();
    btmw_rpc_test_gatts_ut_func002();
    btmw_rpc_test_gatts_ut_func003();
    //btmw_rpc_test_gatts_ut_func004();
    btmw_rpc_test_gatts_ut_func005();
    btmw_rpc_test_gatts_ut_func006();
    btmw_rpc_test_gatts_ut_func007();
    btmw_rpc_test_gatts_ut_func012();
    btmw_rpc_test_gatts_ut_func015();
    btmw_rpc_test_gatts_ut_func016();
    btmw_rpc_test_gatts_ut_func019();
    btmw_rpc_test_gatts_ut_func020();
    btmw_rpc_test_gatts_ut_func021();

    return 0;
}


// For handling incoming commands from CLI.
static int btmw_rpc_test_gatts_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;
    count = 0;
    cmd = btmw_rpc_test_gatts_cli_commands;

    //BTMW_RPC_TEST_Logd("[GATTS] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logd("[GATTS] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GATTS, btmw_rpc_test_gatts_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }
    return ret;
}

static INT32 btmw_rpc_test_gatts_alloc_server(void)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTS_TEST_APP_CNT; i ++)
    {
        if (0 == s_btmw_rpc_gatts_servers[i].server_if)
        {
            dl_list_init(&s_btmw_rpc_gatts_servers[i].services_list);
            dl_list_init(&s_btmw_rpc_gatts_servers[i].connection_list);
            dl_list_init(&s_btmw_rpc_gatts_servers[i].write_req_list);
            return i;
        }
    }

    return -1;
}

static INT32 btmw_rpc_test_gatts_find_server_by_name(char *server_name)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTS_TEST_APP_CNT; i ++)
    {
        if (!strncasecmp(s_btmw_rpc_gatts_servers[i].server_name,
            server_name, BT_GATT_MAX_NAME_LEN - 1))
        {
            return i;
        }
    }

    return -1;
}

static BTMW_RPC_TEST_GATTS_SERVER* btmw_rpc_test_gatts_find_server_by_if(INT32 server_if)
{
    INT32 i = 0;

    if (0 == server_if) return NULL;

    for (i = 0; i < BTMW_RPC_TEST_GATTS_TEST_APP_CNT; i ++)
    {
        if (server_if == s_btmw_rpc_gatts_servers[i].server_if)
        {
            return &s_btmw_rpc_gatts_servers[i];
        }
    }

    return NULL;
}

VOID btmw_rpc_test_gatts_free_server_by_if(INT32 server_if)
{
    INT32 i = 0;
    for (i = 0; i < BTMW_RPC_TEST_GATTS_TEST_APP_CNT; i ++)
    {
        if (server_if == s_btmw_rpc_gatts_servers[i].server_if)
        {
            s_btmw_rpc_gatts_servers[i].event_mask = 0;
            s_btmw_rpc_gatts_servers[i].status = 0;
            s_btmw_rpc_gatts_servers[i].server_if = 0;
            s_btmw_rpc_gatts_servers[i].server_name[0] = '\0';
            dl_list_init(&s_btmw_rpc_gatts_servers[i].services_list);
            dl_list_init(&s_btmw_rpc_gatts_servers[i].connection_list);
            dl_list_init(&s_btmw_rpc_gatts_servers[i].write_req_list);

            return;
        }
    }

    return;
}

VOID btmw_rpc_test_gatts_free_server_by_index(INT32 index)
{
    if (index < 0 || index >= BTMW_RPC_TEST_GATTS_TEST_APP_CNT) return;

    s_btmw_rpc_gatts_servers[index].event_mask = 0;
    s_btmw_rpc_gatts_servers[index].status = 0;
    s_btmw_rpc_gatts_servers[index].server_if = 0;
    s_btmw_rpc_gatts_servers[index].server_name[0] = '\0';
    s_btmw_rpc_gatts_servers[index].started = 0;
    dl_list_init(&s_btmw_rpc_gatts_servers[index].services_list);
    dl_list_init(&s_btmw_rpc_gatts_servers[index].connection_list);
    dl_list_init(&s_btmw_rpc_gatts_servers[index].write_req_list);

    return;
}

static CHAR* btmw_rpc_test_gatts_get_event_str(BT_GATTS_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_REGISTER, "register");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_CONNECTION, "connection");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_SERVICE_ADD, "service_add");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_READ_REQ, "read_req");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_WRITE_REQ, "write_req");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_WRITE_EXE_REQ, "write_exe_req");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_IND_SENT, "ind_sent");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_MTU_CHANGE, "mtu_change");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_PHY_READ, "phy_read");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_PHY_UPDATE, "phy_update");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_CONN_UPDATE, "conn_update");
        BTMW_RPC_TEST_GATTS_CASE_RETURN_STR(BT_GATTS_EVENT_MAX, "bt_off");
        default: return "UNKNOWN_EVENT";
   }
}

