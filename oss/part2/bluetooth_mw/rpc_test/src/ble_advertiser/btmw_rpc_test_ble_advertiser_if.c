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

/* FILE NAME:  btmw_rpc_test_ble_advertiser_if.c
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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "list.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <ctype.h>
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_ble_advertiser_if.h"
#include "mtk_bt_service_ble_advertiser_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define BTMW_RPC_TEST_BLE_ADV_XML_NAME_LEN    (128)

#define BTMW_RPC_TEST_BLE_ADV_SETS_NAME "adv-sets"
#define BTMW_RPC_TEST_BLE_ADV_SET_NAME "adv-set"
#define BTMW_RPC_TEST_BLE_ADV_PARAM_NAME "adv-param"
#define BTMW_RPC_TEST_BLE_ADV_DATA_NAME "adv-data"
#define BTMW_RPC_TEST_BLE_ADV_SCAN_RSP_NAME "adv-scan-rsp"
#define BTMW_RPC_TEST_BLE_ADV_PERI_PARAM_NAME "adv-periodic-param"
#define BTMW_RPC_TEST_BLE_ADV_PERI_DATA_NAME "adv-periodic-data"
#define BTMW_RPC_TEST_BLE_ADV_DURATION_NAME "adv-duration"
#define BTMW_RPC_TEST_BLE_ADV_MAX_EVENT_NAME "adv-max-event"


/* MACRO FUNCTION DECLARATIONS
 */
#define BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(_const,str) case _const: return str

#define BTMW_RPC_TEST_BLE_ADV_LOCK(lock) \
    do { \
        pthread_mutex_lock(&(lock)); \
    }while(0)


#define BTMW_RPC_TEST_BLE_ADV_UNLOCK(lock) \
        do { \
            pthread_mutex_unlock(&(lock));\
        }while(0)

#define BTMW_RPC_TEST_BLE_ADV_SIGNAL(lock, signal, cond) \
    do { \
        pthread_mutex_lock(&(lock));\
        (cond);\
        pthread_cond_signal(&(signal));\
        pthread_mutex_unlock(&(lock));\
    }while(0)


#define BTMW_RPC_TEST_BLE_ADV_WAIT(lock, signal, cond) \
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
    UINT8 adv_id;
    CHAR adv_name[BT_GATT_MAX_NAME_LEN];
    CHAR addr[MAX_BDADDR_LEN];
    BOOL enable;
    BOOL periodic_enable;
    INT8 tx_power;

    BT_BLE_ADV_START_SET_PARAM params;

    struct dl_list node;

    /* for ut */
    pthread_mutex_t lock;
    pthread_cond_t signal;
    UINT32 event_mask;
} BTMW_RPC_TEST_BLE_ADV_SET;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int btmw_rpc_test_ble_adv_load_adv_sets(int argc, char **argv);
static int btmw_rpc_test_ble_adv_unload_adv_sets(int argc, char **argv);

static int btmw_rpc_test_ble_adv_start_adv_sets(int argc, char **argv);
static int btmw_rpc_test_ble_adv_stop_adv_sets(int argc, char **argv);
static int btmw_rpc_test_ble_adv_set_param(int argc, char **argv);
static int btmw_rpc_test_ble_adv_set_data(int argc, char **argv);
static int btmw_rpc_test_ble_adv_set_scan_rsp(int argc, char **argv);
static int btmw_rpc_test_ble_adv_enable(int argc, char **argv);
static int btmw_rpc_test_ble_adv_disable(int argc, char **argv);
static int btmw_rpc_test_ble_adv_set_periodic_param(int argc, char **argv);
static int btmw_rpc_test_ble_adv_set_periodic_data(int argc, char **argv);
static int btmw_rpc_test_ble_adv_periodic_enable(int argc, char **argv);
static int btmw_rpc_test_ble_adv_periodic_disable(int argc, char **argv);
static int btmw_rpc_test_ble_adv_get_addr(int argc, char **argv);
static int btmw_rpc_test_ble_adv_show_adv_sets(int argc, char **argv);

static INT32 btmw_rpc_test_ble_adv_parse_value(CHAR *value_str);
static INT32 btmw_rpc_test_ble_adv_parse_array_value(CHAR *p_ascii, UINT8 *p_hex, INT32 len);
static INT32 btmw_rpc_test_ble_adv_load_adv_param(
    BT_BLE_ADV_PARAM *adv_param, xmlNode *param_node);
static INT32 btmw_rpc_test_ble_adv_load_periodic_adv_param(
    BT_BLE_ADV_PERIODIC_PARAM *adv_param, xmlNode *param_node);
static INT32 btmw_rpc_test_ble_adv_load_adv_data(
    BT_BLE_ADV_DATA_PARAM *adv_data, xmlNode *data_node);
static INT32 btmw_rpc_test_ble_adv_load_adv_set_from_xml(xmlNode *adv_set_node);
static int btmw_rpc_test_ble_adv_load_adv_sets_from_xml(CHAR *adv_sets_xml_path);
static VOID btmw_rpc_test_ble_adv_show_adv_set_value(BTMW_RPC_TEST_BLE_ADV_SET *adv_set);

static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_alloc_adv_set(VOID);
static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_find_adv_set_by_name(char *adv_name);
static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_find_adv_set_by_id(UINT8 adv_id);
#if 0
static VOID btmw_rpc_test_ble_adv_free_adv_set_by_id(INT32 adv_id);
#endif
static CHAR* btmw_rpc_test_ble_adv_get_event_str(BT_BLE_ADV_EVENT event);

/* STATIC VARIABLE DECLARATIONS
 */
static BTMW_RPC_TEST_CLI btmw_rpc_test_ble_adv_cli_commands[] =
{
    {(const char *)"load_adv_set",      btmw_rpc_test_ble_adv_load_adv_sets,      (const char *)" = load_adv_set xxx.xml"},
    {(const char *)"unload_adv_set",    btmw_rpc_test_ble_adv_unload_adv_sets,    (const char *)" = unload_adv_set"},
    {(const char *)"start",             btmw_rpc_test_ble_adv_start_adv_sets,     (const char *)" = start <adv_name>"},
    {(const char *)"stop",              btmw_rpc_test_ble_adv_stop_adv_sets,      (const char *)" = stop <adv_name>"},
    {(const char *)"set_param",         btmw_rpc_test_ble_adv_set_param,          (const char *)" = set_param <adv_name> [<new_adv_name>]"},
    {(const char *)"set_data",          btmw_rpc_test_ble_adv_set_data,           (const char *)" = set_data <adv_name> [<new_adv_name>]"},
    {(const char *)"set_scan_rsp",      btmw_rpc_test_ble_adv_set_scan_rsp,       (const char *)" = set_scan_rsp <adv_name> [<new_adv_name>]"},
    {(const char *)"enable",            btmw_rpc_test_ble_adv_enable,             (const char *)" = enable <adv_name>"},
    {(const char *)"disable",           btmw_rpc_test_ble_adv_disable,            (const char *)" = disable <adv_name>"},
    {(const char *)"set_periodic_param",btmw_rpc_test_ble_adv_set_periodic_param, (const char *)" = set_periodic_param <adv_name> [<new_adv_name>]"},
    {(const char *)"set_periodic_data", btmw_rpc_test_ble_adv_set_periodic_data,  (const char *)" = set_periodic_data <adv_name> [<new_adv_name>]"},
    {(const char *)"periodic_enable",   btmw_rpc_test_ble_adv_periodic_enable,    (const char *)" = periodic_enable <adv_name>"},
    {(const char *)"periodic_disable",  btmw_rpc_test_ble_adv_periodic_disable,   (const char *)" = periodic_disable <adv_name>"},
    {(const char *)"get_own_addr",      btmw_rpc_test_ble_adv_get_addr,           (const char *)" = get_own_addr <adv_name>"},
    {(const char *)"show",              btmw_rpc_test_ble_adv_show_adv_sets,      (const char *)" = show"},
    {NULL, NULL, NULL},
};

static struct dl_list btmw_rpc_test_ble_adv_set_list =
    {&btmw_rpc_test_ble_adv_set_list, &btmw_rpc_test_ble_adv_set_list};

static pthread_mutex_t btmw_rpc_test_ble_adv_lock;

/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */


static VOID btmw_rpc_test_ble_adv_handle_start_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_START_SET_DATA *start_set_data,
    UINT8 adv_id)
{
    if (NULL == adv_set || NULL == start_set_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s(%s), status:%d, adv_id:%d",
        adv_set->adv_name, start_set_data->adv_name,
        start_set_data->status, adv_id);
    adv_set->adv_id = adv_id;

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_set_param_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_SET_PARAM_DATA *set_param_data)
{
    if (NULL == adv_set || NULL == set_param_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d, tx_power:%d",
        adv_set->adv_name, set_param_data->status, adv_set->adv_id,
        set_param_data->tx_power);
    adv_set->tx_power = set_param_data->tx_power;

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_set_data_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_SET_ADV_DATA *set_data)
{
    if (NULL == adv_set || NULL == set_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d",
        adv_set->adv_name, set_data->status, adv_set->adv_id);

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_set_scan_rsp_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA *scan_rsp_data)
{
    if (NULL == adv_set || NULL == scan_rsp_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d",
        adv_set->adv_name, scan_rsp_data->status, adv_set->adv_id);

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_set_periodic_param_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA *periodic_param_data)
{
    if (NULL == adv_set || NULL == periodic_param_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d",
        adv_set->adv_name, periodic_param_data->status, adv_set->adv_id);

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_set_periodic_data_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_SET_PERIODIC_DATA *periodic_data)
{
    if (NULL == adv_set || NULL == periodic_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d",
        adv_set->adv_name, periodic_data->status, adv_set->adv_id);

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_enable_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_ENABLE_DATA *enable_data)
{
    if (NULL == adv_set || NULL == enable_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d enable:%d",
        adv_set->adv_name, enable_data->status, adv_set->adv_id, enable_data->enable);
    adv_set->enable = enable_data->enable;

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_periodic_enable_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA *enable_data)
{
    if (NULL == adv_set || NULL == enable_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, status:%d, adv_id:%d enable:%d",
        adv_set->adv_name, enable_data->status, adv_set->adv_id, enable_data->enable);
    adv_set->periodic_enable = enable_data->enable;

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_get_own_addr_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    BT_BLE_ADV_EVENT_GET_ADDR_DATA *get_addr_data)
{
    if (NULL == adv_set || NULL == get_addr_data)
    {
        return;
    }

    BTMW_RPC_TEST_Logd("adv name:%s, adv_id:%d, addr type:%d, addr:%s",
        adv_set->adv_name, adv_set->adv_id, get_addr_data->address_type, get_addr_data->addr);
    strncpy(adv_set->addr, get_addr_data->addr, MAX_BDADDR_LEN - 1);
    adv_set->addr[MAX_BDADDR_LEN - 1] = '\0';

    return;
}

static VOID btmw_rpc_test_ble_adv_handle_bt_off_event(
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set,
    UINT8 adv_id)
{
    if (NULL == adv_set)
    {
        return;
    }

    adv_set->adv_id = BT_BLE_ADV_INVALID_ADV_ID;

    return;
}


static VOID btmw_rpc_test_ble_adv_event_handle(BT_BLE_ADV_EVENT_PARAM *param, VOID *pv_tag)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;

    if (NULL != pv_tag)
    {
        adv_set = (BTMW_RPC_TEST_BLE_ADV_SET *)pv_tag;
    }

    if (NULL == adv_set)
    {
        if (param->event == BT_BLE_ADV_EVENT_START_ADV_SET)
        {
            adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(param->data.start_set_data.adv_name);
        }
        else
        {
            adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_id(param->adv_id);
        }
        if (adv_set == NULL)
        {
            BTMW_RPC_TEST_Logd("no adv_set %d \n", param->adv_id);
            return;
        }
    }

    BTMW_RPC_TEST_Logd("adv_id:%d(%s), event: %s, size:%d\n",
        param->adv_id, adv_set->adv_name,
        btmw_rpc_test_ble_adv_get_event_str(param->event),
        sizeof(BT_BLE_ADV_EVENT_PARAM));

    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    switch(param->event)
    {
        case BT_BLE_ADV_EVENT_START_ADV_SET:
            btmw_rpc_test_ble_adv_handle_start_event(adv_set,
                &param->data.start_set_data, param->adv_id);
            break;
        case BT_BLE_ADV_EVENT_SET_PARAM:
            btmw_rpc_test_ble_adv_handle_set_param_event(adv_set,
                &param->data.set_param_data);
            break;
        case BT_BLE_ADV_EVENT_SET_DATA:
            btmw_rpc_test_ble_adv_handle_set_data_event(adv_set,
                &param->data.set_adv_data);
            break;
        case BT_BLE_ADV_EVENT_SET_SCAN_RSP:
            btmw_rpc_test_ble_adv_handle_set_scan_rsp_event(adv_set,
                &param->data.set_scan_rsp_data);
            break;
        case BT_BLE_ADV_EVENT_SET_PERI_PARAM:
            btmw_rpc_test_ble_adv_handle_set_periodic_param_event(adv_set,
                &param->data.set_periodic_param_data);
            break;
        case BT_BLE_ADV_EVENT_SET_PERI_DATA:
            btmw_rpc_test_ble_adv_handle_set_periodic_data_event(adv_set,
                &param->data.set_periodic_data);
            break;
        case BT_BLE_ADV_EVENT_ENABLE:
            btmw_rpc_test_ble_adv_handle_enable_event(adv_set,
                &param->data.enable_data);
            break;
        case BT_BLE_ADV_EVENT_ENABLE_PERI:
            btmw_rpc_test_ble_adv_handle_periodic_enable_event(adv_set,
                &param->data.enable_periodic_data);
            break;
        case BT_BLE_ADV_EVENT_GET_ADDR:
            btmw_rpc_test_ble_adv_handle_get_own_addr_event(adv_set,
                &param->data.get_addr_data);
            break;
        case BT_BLE_ADV_EVENT_MAX:
            btmw_rpc_test_ble_adv_handle_bt_off_event(adv_set, param->adv_id);
            break;
        default:
            break;
    }
    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
    if (NULL != adv_set)
    {
        BTMW_RPC_TEST_BLE_ADV_SIGNAL(adv_set->lock,
            adv_set->signal,
            adv_set->event_mask |= (1 << param->event));
    }

    return;
}

static int btmw_rpc_test_ble_adv_load_adv_sets(int argc, char **argv)
{
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV load_adv_set <service_xml_path>\n");
        return -1;
    }

    if (0 > btmw_rpc_test_ble_adv_load_adv_sets_from_xml(argv[0]))
    {
        BTMW_RPC_TEST_Loge("load adv set from xml %s fail", argv[0]);
        return 0;
    }

    return 0;
}

static int btmw_rpc_test_ble_adv_unload_adv_sets(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);
    while (!dl_list_empty(&btmw_rpc_test_ble_adv_set_list))
    {
        adv_set = dl_list_first(&btmw_rpc_test_ble_adv_set_list, BTMW_RPC_TEST_BLE_ADV_SET, node);
        if (NULL != adv_set)
        {
            dl_list_del(&adv_set->node);
            free(adv_set);
            adv_set = NULL;
        }
    }
    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_start_adv_sets(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV start <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    BTMW_RPC_TEST_Loge("start %s size %d", argv[0], sizeof(adv_set->params));

    strncpy(adv_set->params.adv_name, argv[0], BT_GATT_MAX_NAME_LEN - 1);
    adv_set->params.adv_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';

    if (0 != a_mtkapi_bt_ble_adv_start_set(&adv_set->params,
        btmw_rpc_test_ble_adv_event_handle, adv_set))
    {
        BTMW_RPC_TEST_Loge("start %s fail", adv_set->params.adv_name);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_stop_adv_sets(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    BT_BLE_ADV_STOP_SET_PARAM stop_adv_set_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV stop <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (BT_BLE_ADV_INVALID_ADV_ID == adv_set->adv_id)
    {
        BTMW_RPC_TEST_Loge("adv set %s adv_id %d invalid", argv[0], adv_set->adv_id);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    stop_adv_set_param.adv_id = adv_set->adv_id;
    a_mtkapi_bt_ble_adv_stop_set(&stop_adv_set_param);

    BTMW_RPC_TEST_Loge("stop adv set %s adv_id %d success", argv[0], adv_set->adv_id);
    adv_set->adv_id = BT_BLE_ADV_INVALID_ADV_ID;
    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}


static int btmw_rpc_test_ble_adv_set_param(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV set_param <adv_name> [<new_adv_name>]\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (argc >= 2)
    {
        BTMW_RPC_TEST_BLE_ADV_SET *new_adv_set = NULL;
        new_adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[1]);
        if (NULL == new_adv_set)
        {
            BTMW_RPC_TEST_Loge("find adv set %s fail", argv[1]);
        }
        else
        {
            new_adv_set->params.adv_param.adv_id = adv_set->adv_id;

            a_mtkapi_bt_ble_adv_set_param(&new_adv_set->params.adv_param);
        }
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    adv_set->params.adv_param.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_set_param(&adv_set->params.adv_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s set param fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_set_data(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV set_data <adv_name> [<new_adv_name>]\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (argc >= 2)
    {
        BTMW_RPC_TEST_BLE_ADV_SET *new_adv_set = NULL;
        new_adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[1]);
        if (NULL == new_adv_set)
        {
            BTMW_RPC_TEST_Loge("find adv set %s fail", argv[1]);
        }
        else
        {

            new_adv_set->params.adv_data.adv_id = adv_set->adv_id;

            a_mtkapi_bt_ble_adv_set_data(&new_adv_set->params.adv_data);
        }
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    adv_set->params.adv_data.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_set_data(&adv_set->params.adv_data);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s set adv data fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_set_scan_rsp(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV set_scan_rsp <adv_name> [<new_adv_name>]\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (argc >= 2)
    {
        BTMW_RPC_TEST_BLE_ADV_SET *new_adv_set = NULL;
        new_adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[1]);
        if (NULL == new_adv_set)
        {
            BTMW_RPC_TEST_Loge("find adv set %s fail", argv[1]);
        }
        else
        {
            new_adv_set->params.scan_rsp.adv_id = adv_set->adv_id;

            a_mtkapi_bt_ble_adv_set_scan_rsp(&new_adv_set->params.scan_rsp);
        }

        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_set->params.scan_rsp.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_set_scan_rsp(&adv_set->params.scan_rsp);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s set scan rsp fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_enable(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    BT_BLE_ADV_ENABLE_PARAM adv_enable_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV enable <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_enable_param.adv_id = adv_set->adv_id;
    adv_enable_param.enable = TRUE;
    adv_enable_param.duration = adv_set->params.duration;
    adv_enable_param.max_ext_adv_events = adv_set->params.max_ext_adv_events;

    ret = a_mtkapi_bt_ble_adv_enable(&adv_enable_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s enable fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_disable(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    BT_BLE_ADV_ENABLE_PARAM adv_enable_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV enable <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_enable_param.adv_id = adv_set->adv_id;
    adv_enable_param.enable = FALSE;
    adv_enable_param.duration = 0;
    adv_enable_param.max_ext_adv_events = 0;

    ret = a_mtkapi_bt_ble_adv_enable(&adv_enable_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s disable fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_set_periodic_param(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV set_periodic_param <adv_name> [<new_adv_name>]\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (argc >= 2)
    {
        BTMW_RPC_TEST_BLE_ADV_SET *new_adv_set = NULL;
        new_adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[1]);
        if (NULL == new_adv_set)
        {
            BTMW_RPC_TEST_Loge("find adv set %s fail", argv[1]);
        }
        else
        {
            new_adv_set->params.peri_param.adv_id = adv_set->adv_id;

            a_mtkapi_bt_ble_adv_set_periodic_param(&new_adv_set->params.peri_param);
        }
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_set->params.peri_param.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_set_periodic_param(&adv_set->params.peri_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s set periodic param fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_set_periodic_data(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV set_periodic_data <adv_name> [<new_adv_name>]\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    if (argc >= 2)
    {
        BTMW_RPC_TEST_BLE_ADV_SET *new_adv_set = NULL;
        new_adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[1]);
        if (NULL == new_adv_set)
        {
            BTMW_RPC_TEST_Loge("find adv set %s fail", argv[1]);
        }
        else
        {
            new_adv_set->params.peri_data.adv_id = adv_set->adv_id;

            a_mtkapi_bt_ble_adv_set_periodic_data(&new_adv_set->params.peri_data);
        }
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_set->params.peri_data.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_set_periodic_data(&adv_set->params.peri_data);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s set periodic data fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_periodic_enable(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    BT_BLE_ADV_PERIODIC_ENABLE_PARAM adv_enable_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV periodic_enable <adv_name> \n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_enable_param.adv_id = adv_set->adv_id;
    adv_enable_param.enable = TRUE;

    ret = a_mtkapi_bt_ble_adv_periodic_enable(&adv_enable_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s enable periodic fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_periodic_disable(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    BT_BLE_ADV_PERIODIC_ENABLE_PARAM adv_enable_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV periodic_disable <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    adv_enable_param.adv_id = adv_set->adv_id;
    adv_enable_param.enable = FALSE;

    ret = a_mtkapi_bt_ble_adv_periodic_enable(&adv_enable_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s disable periodic fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static int btmw_rpc_test_ble_adv_get_addr(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    INT32 ret = 0;
    BT_BLE_ADV_GET_ADDR_PARAM get_adv_addr_param;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  MW_RPC_BLE_ADV get_addr <adv_name>\n");
        return -1;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }
    get_adv_addr_param.adv_id = adv_set->adv_id;

    ret = a_mtkapi_bt_ble_adv_get_own_address(&get_adv_addr_param);
    if (ret != BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logw("set %s get own addr fail %d", argv[0], ret);
    }

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

static void btmw_rpc_test_ble_adv_show_all_adv_sets(void)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    dl_list_for_each(adv_set, &btmw_rpc_test_ble_adv_set_list,
        BTMW_RPC_TEST_BLE_ADV_SET, node)
    {
        btmw_rpc_test_ble_adv_show_adv_set_value(adv_set);
    }
}

static int btmw_rpc_test_ble_adv_show_adv_sets(int argc, char **argv)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    if (argc < 1)
    {
        btmw_rpc_test_ble_adv_show_all_adv_sets();
        return 0;
    }
    BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);

    adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name(argv[0]);

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Loge("find adv set %s fail", argv[0]);
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        return 0;
    }

    btmw_rpc_test_ble_adv_show_adv_set_value(adv_set);

    BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);

    return 0;
}

int btmw_rpc_test_ble_adv_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_ble_adv_cli_commands;

    //BTMW_RPC_TEST_Logi("[BLE_Adv] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logi("[BLE_Adv] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BLE_ADVERTISER,
            btmw_rpc_test_ble_adv_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}


int btmw_rpc_test_ble_adv_init(void)
{
    BTMW_RPC_TEST_Logi("[BLE_Adv] btmw_rpc_test_ble_adv_init");
    int ret = 0;
    BTMW_RPC_TEST_MOD ble_adv_mod = {0};

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&btmw_rpc_test_ble_adv_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    // Register command to CLI
    ble_adv_mod.mod_id = BTMW_RPC_TEST_MOD_BLE_ADV;
    strncpy(ble_adv_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_BLE_ADVERTISER, sizeof(ble_adv_mod.cmd_key));
    ble_adv_mod.cmd_handler = btmw_rpc_test_ble_adv_cmd_handler;
    ble_adv_mod.cmd_tbl = btmw_rpc_test_ble_adv_cli_commands;

    ret = btmw_rpc_test_register_mod(&ble_adv_mod);
    BTMW_RPC_TEST_Logi("[BLE_SCANNER] btmw_rpc_test_register_mod() returns: %d\n", ret);
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] g_cli_pts_mode: %d\n", g_cli_pts_mode);

    return ret;
}

int btmw_rpc_test_ble_adv_deinit(void)
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}

static INT32 btmw_rpc_test_ble_adv_parse_value(CHAR *value_str)
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

static INT32 btmw_rpc_test_ble_adv_parse_array_value(CHAR *p_ascii, UINT8 *p_hex, INT32 len)
{
    INT32     x;
    UINT8     c;
    if (NULL == p_ascii || NULL == p_hex)
    {
        return 0;
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

static INT32 btmw_rpc_test_ble_adv_load_adv_param(
    BT_BLE_ADV_PARAM *adv_param,
    xmlNode *param_node)
{
    UINT32 attr_cnt = 0;
    if (NULL == adv_param || NULL == param_node)
    {
        BTMW_RPC_TEST_Logw("null parameter");
        return 0;
    }

    xmlNode *child_node = param_node->xmlChildrenNode;
    while (NULL != child_node)
    {
        if (child_node->type == XML_ELEMENT_NODE)
        {
            if (!xmlStrncmp(child_node->name, BAD_CAST "connectable", strlen("connectable")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->props.connectable = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "scannable", strlen("scannable")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->props.scannable = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "legacy", strlen("legacy")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->props.legacy = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "anonymous", strlen("anonymous")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->props.anonymous = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "include-tx-power", strlen("include-tx-power")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->props.include_tx_power = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "min-interval", strlen("min-interval")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->min_interval = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "max-interval", strlen("max-interval")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->max_interval = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "tx-power", strlen("tx-power")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->tx_power = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "primary-phy", strlen("primary-phy")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->primary_adv_phy = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "secondary-phy", strlen("secondary-phy")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->secondary_adv_phy = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
        }

        child_node = child_node->next;
    }

    return attr_cnt;
}


static INT32 btmw_rpc_test_ble_adv_load_periodic_adv_param(
    BT_BLE_ADV_PERIODIC_PARAM *adv_param,
    xmlNode *param_node)
{
    UINT32 attr_cnt = 0;
    if (NULL == adv_param || NULL == param_node)
    {
        BTMW_RPC_TEST_Logw("null parameter");
        return 0;
    }
    adv_param->enable = TRUE;

    xmlNode *child_node = param_node->xmlChildrenNode;
    while (NULL != child_node)
    {
        if (child_node->type == XML_ELEMENT_NODE)
        {
            if (!xmlStrncmp(child_node->name, BAD_CAST "include-tx-power", strlen("include-tx-power")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->include_tx_power = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "min-interval", strlen("min-interval")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->min_interval = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "max-interval", strlen("max-interval")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_param->max_interval = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
        }

        child_node = child_node->next;
    }

    return attr_cnt;
}


static INT32 btmw_rpc_test_ble_adv_load_adv_data(
    BT_BLE_ADV_DATA_PARAM *adv_data,
    xmlNode *data_node)
{
    UINT32 attr_cnt = 0;
    CHAR service_data_uuid[BT_GATT_MAX_UUID_LEN] = {'\0'};
    UINT8 service_manu_data[BT_BLE_ADV_MAX_ADV_DATA_LEN] = {0};
    UINT16 company_id = 0;
    if (NULL == adv_data || NULL == data_node)
    {
        BTMW_RPC_TEST_Logw("null parameter");
        return 0;
    }

    xmlNode *child_node = data_node->xmlChildrenNode;
    while (NULL != child_node)
    {
        if (child_node->type == XML_ELEMENT_NODE)
        {
            if (!xmlStrncmp(child_node->name, BAD_CAST "local-name", strlen("local-name")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                a_mtkapi_bt_ble_adv_build_name(adv_data, (CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "service-uuid", strlen("service-uuid")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                a_mtkapi_bt_ble_adv_build_service_uuid(adv_data, (CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "tx-power", strlen("tx-power")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                a_mtkapi_bt_ble_adv_build_tx_power(adv_data, atoi((CHAR*)desc_value));
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "service-data-uuid", strlen("service-data-uuid")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                INT32 str_len = strlen((CHAR*)desc_value) >= BT_GATT_MAX_UUID_LEN ?
                    strlen((CHAR*)desc_value) : BT_GATT_MAX_UUID_LEN - 1;
                strncpy(service_data_uuid, (CHAR*)desc_value, str_len);
                service_data_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "service-data-data", strlen("service-data-data")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                INT32 service_data_len = 0;
                service_data_len = btmw_rpc_test_ble_adv_parse_array_value((CHAR*)desc_value,
                    service_manu_data, BT_BLE_ADV_MAX_ADV_DATA_LEN);
                a_mtkapi_bt_ble_adv_build_service_data(adv_data, service_data_uuid, service_manu_data, service_data_len);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "manu-data-companyid", strlen("manu-data-companyid")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                company_id = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "manu-data-data", strlen("manu-data-data")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                INT32 manu_data_len = 0;
                manu_data_len = btmw_rpc_test_ble_adv_parse_array_value((CHAR*)desc_value,
                    service_manu_data, BT_BLE_ADV_MAX_ADV_DATA_LEN);
                a_mtkapi_bt_ble_adv_build_manu_data(adv_data, company_id, service_manu_data, manu_data_len);
                xmlFree(desc_value);
            }
            else if (!xmlStrncmp(child_node->name, BAD_CAST "raw-data", strlen("raw-data")))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                INT32 raw_data_len = 0;
                INT32 left_data_len = 0;
                raw_data_len = btmw_rpc_test_ble_adv_parse_array_value((CHAR*)desc_value,
                    service_manu_data, BT_BLE_ADV_MAX_ADV_DATA_LEN);

                if (adv_data->len < BT_BLE_ADV_MAX_ADV_DATA_LEN)
                {
                    left_data_len = BT_BLE_ADV_MAX_ADV_DATA_LEN - adv_data->len;
                    left_data_len = left_data_len < raw_data_len ? left_data_len : raw_data_len;
                    memcpy(&adv_data->data[adv_data->len], service_manu_data, left_data_len);
                    adv_data->len += left_data_len;
                }
                xmlFree(desc_value);
            }
        }

        child_node = child_node->next;
    }

    return attr_cnt;
}


static INT32 btmw_rpc_test_ble_adv_load_adv_set_from_xml(xmlNode *adv_set_node)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;

    if (NULL == adv_set_node)
    {
        BTMW_RPC_TEST_Logw("adv_set_node is NULL");
        return -1;
    }

    if (0 != xmlStrncmp(adv_set_node->name,
        (xmlChar *)BTMW_RPC_TEST_BLE_ADV_SET_NAME,
        strlen(BTMW_RPC_TEST_BLE_ADV_SET_NAME)))
    {
        BTMW_RPC_TEST_Logw("adv_set_node is not "BTMW_RPC_TEST_BLE_ADV_SET_NAME);
        return -1;
    }

    xmlAttrPtr attr = adv_set_node->properties;
    while (NULL != attr)
    {
        if (!xmlStrncmp(attr->name, BAD_CAST "name", 4))
        {
            xmlChar *attr_value = xmlGetProp(adv_set_node, BAD_CAST "name");

            adv_set = btmw_rpc_test_ble_adv_find_adv_set_by_name((CHAR*)attr_value);
            if (NULL == adv_set)
            {
                adv_set = btmw_rpc_test_ble_adv_alloc_adv_set();
                if (NULL == adv_set)
                {
                    BTMW_RPC_TEST_Logw("alloc adv set fail");
                    return -1;
                }
                strncpy(adv_set->adv_name,
                    (CHAR*)attr_value, BT_GATT_MAX_NAME_LEN - 1);
                adv_set->adv_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';
            }

            strncpy(adv_set->params.adv_name,
                (CHAR*)attr_value, BT_GATT_MAX_NAME_LEN - 1);
            adv_set->params.adv_name[BT_GATT_MAX_NAME_LEN - 1] = '\0';

            xmlFree(attr_value);
            break;
        }

        attr = attr->next;
    }

    if (NULL == adv_set)
    {
        BTMW_RPC_TEST_Logw("adv set no name");
        return -1;
    }

    memset((VOID*)&adv_set->params, 0, sizeof(adv_set->params));

    xmlNode *child_node = adv_set_node->xmlChildrenNode;
    while (NULL != child_node)
    {
        if (child_node->type == XML_ELEMENT_NODE)
        {
            /* adv param */
            if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_PARAM_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_PARAM_NAME)))
            {
                btmw_rpc_test_ble_adv_load_adv_param(&adv_set->params.adv_param, child_node);
            }
            /* adv adata */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_DATA_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_DATA_NAME)))
            {
                btmw_rpc_test_ble_adv_load_adv_data(&adv_set->params.adv_data, child_node);
            }
            /* scan rsp */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_SCAN_RSP_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_SCAN_RSP_NAME)))
            {
                btmw_rpc_test_ble_adv_load_adv_data(&adv_set->params.scan_rsp, child_node);
            }
            /* periodic param */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_PERI_PARAM_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_PERI_PARAM_NAME)))
            {
                btmw_rpc_test_ble_adv_load_periodic_adv_param(&adv_set->params.peri_param, child_node);
            }
            /* periodic data */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_PERI_DATA_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_PERI_DATA_NAME)))
            {
                btmw_rpc_test_ble_adv_load_adv_data(&adv_set->params.peri_data, child_node);
            }
            /* duration */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_DURATION_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_DURATION_NAME)))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_set->params.duration = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
            /* max extended adv event */
            else if (!xmlStrncmp(child_node->name,
                (xmlChar *)BTMW_RPC_TEST_BLE_ADV_MAX_EVENT_NAME,
                strlen(BTMW_RPC_TEST_BLE_ADV_MAX_EVENT_NAME)))
            {
                xmlChar *desc_value = xmlNodeGetContent(child_node);
                adv_set->params.max_ext_adv_events = btmw_rpc_test_ble_adv_parse_value((CHAR*)desc_value);
                xmlFree(desc_value);
            }
        }

        child_node = child_node->next;
    }


    return 0;
}


static int btmw_rpc_test_ble_adv_load_adv_sets_from_xml(CHAR *adv_sets_xml_path)
{
    xmlDoc *doc = NULL;
    xmlNode *xml_root = NULL;
    INT32 ret = -1;
    LIBXML_TEST_VERSION;

    if (NULL == adv_sets_xml_path)
    {
        BTMW_RPC_TEST_Logw("xml file name NULL");
        goto __ERROR;
    }

    if (strlen(adv_sets_xml_path) >= BTMW_RPC_TEST_BLE_ADV_XML_NAME_LEN - 1)
    {
        BTMW_RPC_TEST_Logw("xml file name (%s) too long", adv_sets_xml_path);
        goto __ERROR;
    }
    doc = xmlReadFile(adv_sets_xml_path, NULL, 0);
    if (adv_sets_xml_path == NULL)
    {
        BTMW_RPC_TEST_Logw("xmlReadFile %s fail", adv_sets_xml_path);
        goto __ERROR;
    }

    xml_root = xmlDocGetRootElement(doc);
    if (NULL == xml_root)
    {
        BTMW_RPC_TEST_Logw("xml root read fail");
        goto __ERROR;
    }

    if (0 == xmlStrncmp(xml_root->name,
        (xmlChar *)BTMW_RPC_TEST_BLE_ADV_SETS_NAME,
        strlen(BTMW_RPC_TEST_BLE_ADV_SETS_NAME)))
    {
        xmlNode *adv_set_node = xml_root->xmlChildrenNode;
        BTMW_RPC_TEST_BLE_ADV_LOCK(btmw_rpc_test_ble_adv_lock);
        while(NULL != adv_set_node)
        {
            if (adv_set_node->type == XML_ELEMENT_NODE)
            {
                btmw_rpc_test_ble_adv_load_adv_set_from_xml(adv_set_node);
            }
            adv_set_node = adv_set_node->next;
        }
        BTMW_RPC_TEST_BLE_ADV_UNLOCK(btmw_rpc_test_ble_adv_lock);
        BTMW_RPC_TEST_Logi("adv sets parse finish ");
    }
    else
    {
        BTMW_RPC_TEST_Logw("xml root is not "BTMW_RPC_TEST_BLE_ADV_SETS_NAME);
        goto __ERROR;
    }

    ret = 0;

__ERROR:
    if (NULL != doc)
    {
        xmlFreeDoc(doc);
    }
    xmlCleanupParser();
    return ret;
}

static VOID btmw_rpc_test_ble_adv_show_adv_data(CHAR *title, BT_BLE_ADV_DATA_PARAM *adv_data)
{
    UINT32 i = 0;
    UINT8 ad_type = 0;
    UINT8 ad_data_len = 0;
    if (NULL == adv_data || title == NULL) return;
    BTMW_RPC_TEST_Logd("---------------------%s(%d)-----------------------",
        title, adv_data->len);
    while(i < adv_data->len)
    {
        ad_data_len = adv_data->data[i++];
        ad_type = adv_data->data[i++];
        if (ad_type == BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_16_BIT_SERVICE_UUIDS)
        {
            BTMW_RPC_TEST_Logd("16bit service UUID: %02x%02x",
                adv_data->data[i+1],
                adv_data->data[i]);
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_32_BIT_SERVICE_UUIDS)
        {
            BTMW_RPC_TEST_Logd("32bit service UUID: %02x%02x%02x%02x",
                adv_data->data[i+3],
                adv_data->data[i+2],
                adv_data->data[i+1],
                adv_data->data[i]);
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_COMPLETE_LIST_128_BIT_SERVICE_UUIDS)
        {
            BTMW_RPC_TEST_Logd("128bit service UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x"
               "-%02x%02x-%02x%02x%02x%02x%02x%02x",
                adv_data->data[i+15], adv_data->data[i+14], adv_data->data[i+13],
                adv_data->data[i+12], adv_data->data[i+11], adv_data->data[i+10],
                adv_data->data[i+9], adv_data->data[i+8], adv_data->data[i+7],
                adv_data->data[i+6], adv_data->data[i+5], adv_data->data[i+4],
                adv_data->data[i+3], adv_data->data[i+2], adv_data->data[i+1],
                adv_data->data[i]);
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_SHORTENED_LOCAL_NAME
            || ad_type == BT_BLE_ADV_AD_TYPE_COMPLETE_LOCAL_NAME)
        {
            BTMW_RPC_TEST_Logd("local name: %s", (CHAR*)&adv_data->data[i]);
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_TX_POWER_LEVEL)
        {
            BTMW_RPC_TEST_Logd("tx power: %d",
                (INT8)adv_data->data[i]);
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_SERVICE_DATA_16_BIT_UUID)
        {
            BTMW_RPC_TEST_Logd("service data 16bit UUID: %02x%02x",
                adv_data->data[i+1],
                adv_data->data[i]);
            INT32 data_index = 0;
            printf("service data(%d): ", ad_data_len - 3);
            for (data_index = 0; data_index < ad_data_len - 3; data_index++)
            {
                printf("%02X ", adv_data->data[i+2+data_index]);
            }
            printf("\n");
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_SERVICE_DATA_32_BIT_UUID)
        {
            BTMW_RPC_TEST_Logd("service data 32bit UUID: %02x%02x%02x%02x",
                adv_data->data[i+3],
                adv_data->data[i+2],
                adv_data->data[i+1],
                adv_data->data[i]);
            INT32 data_index = 0;
            printf("service data(%d): ", ad_data_len - 5);
            for (data_index = 0; data_index < ad_data_len - 5; data_index++)
            {
                printf("%02X ", adv_data->data[i+4+data_index]);
            }
            printf("\n");
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_SERVICE_DATA_128_BIT_UUID)
        {
            BTMW_RPC_TEST_Logd("service data 32bit UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x"
               "-%02x%02x-%02x%02x%02x%02x%02x%02x",
                adv_data->data[i+15], adv_data->data[i+14], adv_data->data[i+13],
                adv_data->data[i+12], adv_data->data[i+11], adv_data->data[i+10],
                adv_data->data[i+9], adv_data->data[i+8], adv_data->data[i+7],
                adv_data->data[i+6], adv_data->data[i+5], adv_data->data[i+4],
                adv_data->data[i+3], adv_data->data[i+2], adv_data->data[i+1],
                adv_data->data[i]);
            INT32 data_index = 0;
            printf("service data(%d): ", ad_data_len - 17);
            for (data_index = 0; data_index < ad_data_len - 17; data_index++)
            {
                printf("%02X ", adv_data->data[i+16+data_index]);
            }
            printf("\n");
        }
        else if (ad_type == BT_BLE_ADV_AD_TYPE_MANUFACTURER_SPECIFIC_DATA)
        {
            UINT16 company_id = 0;
            company_id = (adv_data->data[i+1] << 8) + adv_data->data[i];

            BTMW_RPC_TEST_Logd("company id: %d", company_id);
            INT32 data_index = 0;
            printf("company data(%d): ", ad_data_len - 3);
            for (data_index = 0; data_index < ad_data_len - 3; data_index++)
            {
                printf("%02X ", adv_data->data[i+2+data_index]);
            }
            printf("\n");
        }

        i += (ad_data_len - 1);
    }
}

static VOID btmw_rpc_test_ble_adv_show_adv_set_value(BTMW_RPC_TEST_BLE_ADV_SET *adv_set)
{
    if (NULL == adv_set) return;
    BTMW_RPC_TEST_Logd("-------------------------------------------");
    BTMW_RPC_TEST_Logd("adv name: %s", adv_set->adv_name);
    BTMW_RPC_TEST_Logd("adv_id: %d", adv_set->adv_id);
    BTMW_RPC_TEST_Logd("addr: %s", adv_set->addr);
    BTMW_RPC_TEST_Logd("enable: %d", adv_set->enable);
    BTMW_RPC_TEST_Logd("periodic_enable: %d", adv_set->periodic_enable);
    BTMW_RPC_TEST_Logd("tx_power: %d", adv_set->tx_power);
    BTMW_RPC_TEST_Logd("---------------------param-----------------------");
    BTMW_RPC_TEST_Logd("connectable: %d", adv_set->params.adv_param.props.connectable);
    BTMW_RPC_TEST_Logd("scannable: %d", adv_set->params.adv_param.props.scannable);
    BTMW_RPC_TEST_Logd("legacy: %d", adv_set->params.adv_param.props.legacy);
    BTMW_RPC_TEST_Logd("anonymous: %d", adv_set->params.adv_param.props.anonymous);
    BTMW_RPC_TEST_Logd("include_tx_power: %d", adv_set->params.adv_param.props.include_tx_power);
    BTMW_RPC_TEST_Logd("min_interval: %d", adv_set->params.adv_param.min_interval);
    BTMW_RPC_TEST_Logd("max_interval: %d", adv_set->params.adv_param.max_interval);
    BTMW_RPC_TEST_Logd("tx_power: %d", adv_set->params.adv_param.tx_power);
    BTMW_RPC_TEST_Logd("primary_adv_phy: %d", adv_set->params.adv_param.primary_adv_phy);
    BTMW_RPC_TEST_Logd("secondary_adv_phy: %d", adv_set->params.adv_param.secondary_adv_phy);

    btmw_rpc_test_ble_adv_show_adv_data("adv data", &adv_set->params.adv_data);
    btmw_rpc_test_ble_adv_show_adv_data("scan rsp", &adv_set->params.scan_rsp);
    BTMW_RPC_TEST_Logd("---------------------periodic param-----------------------");
    BTMW_RPC_TEST_Logd("include_tx_power: %d", adv_set->params.peri_param.include_tx_power);
    BTMW_RPC_TEST_Logd("max_interval: %d", adv_set->params.peri_param.max_interval);
    BTMW_RPC_TEST_Logd("min_interval: %d", adv_set->params.peri_param.min_interval);
    btmw_rpc_test_ble_adv_show_adv_data("periodic data", &adv_set->params.peri_data);
    BTMW_RPC_TEST_Logd("max_ext_adv_events: %d", adv_set->params.max_ext_adv_events);
    BTMW_RPC_TEST_Logd("duration: %d", adv_set->params.duration);
    BTMW_RPC_TEST_Logd("----------------------------------------------------------");
}


static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_alloc_adv_set(VOID)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set =
        (BTMW_RPC_TEST_BLE_ADV_SET *)calloc(1, sizeof(BTMW_RPC_TEST_BLE_ADV_SET));
    if (NULL == adv_set) return NULL;

    adv_set->adv_id = BT_BLE_ADV_INVALID_ADV_ID;

    dl_list_init(&adv_set->node);
    dl_list_add_tail(&btmw_rpc_test_ble_adv_set_list, &adv_set->node);

    return adv_set;
}

static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_find_adv_set_by_name(char *adv_name)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    dl_list_for_each(adv_set, &btmw_rpc_test_ble_adv_set_list,
        BTMW_RPC_TEST_BLE_ADV_SET, node)
    {
        if (!strncmp(adv_set->adv_name, adv_name, BT_GATT_MAX_UUID_LEN - 1))
        {
            return adv_set;
        }
    }

    return NULL;
}

static BTMW_RPC_TEST_BLE_ADV_SET* btmw_rpc_test_ble_adv_find_adv_set_by_id(UINT8 adv_id)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    dl_list_for_each(adv_set, &btmw_rpc_test_ble_adv_set_list,
        BTMW_RPC_TEST_BLE_ADV_SET, node)
    {
        if (adv_set->adv_id == adv_id)
        {
            return adv_set;
        }
    }

    return NULL;
}

#if 0
static VOID btmw_rpc_test_ble_adv_free_adv_set_by_id(INT32 adv_id)
{
    BTMW_RPC_TEST_BLE_ADV_SET *adv_set = NULL;
    BTMW_RPC_TEST_BLE_ADV_SET *temp = NULL;
    dl_list_for_each_safe(adv_set, temp, &btmw_rpc_test_ble_adv_set_list,
        BTMW_RPC_TEST_BLE_ADV_SET, node)
    {
        if (adv_set->adv_id == adv_id)
        {
            dl_list_del(&adv_set->node);
            free(adv_set);
            return;
        }
    }
    return;
}
#endif


static CHAR* btmw_rpc_test_ble_adv_get_event_str(BT_BLE_ADV_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_START_ADV_SET, "start_adv_set");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_SET_PARAM, "set_param");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_SET_DATA, "set_data");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_SET_SCAN_RSP, "set_scan_rsp");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_SET_PERI_PARAM, "set_periodic_param");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_SET_PERI_DATA, "set_periodic_data");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_ENABLE, "adv_enable");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_ENABLE_PERI, "periodic_adv_enable");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_GET_ADDR, "get_addr");
        BTMW_RPC_TEST_BLE_ADV_CASE_RETURN_STR(BT_BLE_ADV_EVENT_MAX, "bt_off");
        default: return "UNKNOWN_EVENT";
   }
}

