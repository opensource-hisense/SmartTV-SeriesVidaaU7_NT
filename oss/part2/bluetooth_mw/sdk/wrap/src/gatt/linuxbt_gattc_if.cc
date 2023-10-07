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

/* FILE NAME:  linuxbt_gattc_if.cc
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
#include <list>
#include <base/bind.h>
#include <base/callback.h>
#include <mutex>
#include "bluetooth.h"
#include "bt_gatt_client.h"
#include "bt_mw_common.h"
#include "linuxbt_gattc_if.h"
#include "linuxbt_common.h"
#include "bt_mw_gatt.h"
#include "raw_address.h"
#include "bt_mw_gattc.h"

using LockGuard = std::lock_guard<std::mutex>;
/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_GATTC_CHECK_ADDR(addr, ret) do{                                         \
        if ((NULL == (addr)) || ((MAX_BDADDR_LEN - 1) != strlen(addr)))                 \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_GATT, "invalid addr(%s)", addr==NULL?"NULL":(addr));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)


#define LINUXBT_GATTC_CHECK_NAME(name, ret) do{                                         \
        if ((NULL == (name)) || (0 == strlen(name)) || (BT_GATT_MAX_NAME_LEN - 1 < strlen(name)))   \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_GATT, "invalid name(%s)", name==NULL?"NULL":(name));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)

#define LINUXBT_GATTC_CHECK_INITED(ret)    do {                     \
        if (NULL == linuxbt_gattc_interface)                        \
        {                                                           \
            BT_DBG_ERROR(BT_DEBUG_GATT, "gatt client not init");    \
            return ret;                                             \
        }                                                           \
    }while(0)


#define LINUXBT_GATTC_SET_MSG_LEN(msg) do{  \
    msg.hdr.len = sizeof(BT_GATTC_EVENT_PARAM);      \
    }while(0)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    RawAddress addr;
    int connected;
    int conn_id;
} LINUXBT_GATTC_DEV_CB;

typedef struct
{
    int in_use;
    int client_if;
    char client_name[BT_GATT_MAX_NAME_LEN];
    bluetooth::Uuid uuid;
    int congested;

    std::vector<btgatt_db_element_t> service;
    std::list<LINUXBT_GATTC_DEV_CB> dev_list;
    std::list<BT_GATTC_EVENT_PARAM> msg_list;
} LINUXBT_GATTC_SERVER_CB;

typedef struct
{
    BOOL inited;
    LINUXBT_GATTC_SERVER_CB client_cb[BT_GATT_MAX_APP_CNT];
} LINUXBT_GATTC_CB;
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
// Callback functions
static void linuxbt_gattc_read_phy_callback(uint8_t client_if, RawAddress bda, uint8_t tx_phy,
                            uint8_t rx_phy, uint8_t status);

static void linuxbt_gattc_register_client_callback(int status, int client_if,
                                                            const bluetooth::Uuid& app_uuid);


static void linuxbt_gattc_connect_callback(int conn_id, int status,
                                                    int client_if, const RawAddress& bda);

static void linuxbt_gattc_disconnect_callback(int conn_id, int status,
                                                        int client_if, const RawAddress& bda);

static void linuxbt_gattc_search_complete_callback(int conn_id, int status);

static void linuxbt_gattc_register_for_notification_callback(int client_if,
                                                                        int registered,
                                                                        int status,
                                                                        uint16_t handle);

static void linuxbt_gattc_notify_callback(int conn_id,
                                const btgatt_notify_params_t& p_data);

static void linuxbt_gattc_read_characteristic_callback(int conn_id,
                                                                  int status,
                                                                  btgatt_read_params_t *p_data);

static void linuxbt_gattc_write_characteristic_callback(int conn_id,
                                                                   int status,
                                                                   uint16_t handle);

static void linuxbt_gattc_read_descriptor_callback(int conn_id, int status,
                                         const btgatt_read_params_t& p_data);

static void linuxbt_gattc_write_descriptor_callback(int conn_id, int status,
                                          uint16_t handle);

static void linuxbt_gattc_execute_write_callback(int conn_id, int status);

static void linuxbt_gattc_read_remote_rssi_callback(int client_if,
                                                                const RawAddress& bda,
                                                                int rssi,
                                                                int status);

static void linuxbt_gattc_configure_mtu_callback(int conn_id,
                                                            int status,
                                                            int mtu);

static void linuxbt_gattc_congestion_callback(int conn_id, bool congested);

static void linuxbt_gattc_get_gatt_db_callback(int conn_id, const btgatt_db_element_t* db,
                                     int count);

static void linuxbt_gattc_phy_update_callback(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status);

static void linuxbt_gattc_conn_update_callback(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                      uint8_t status);

static int linuxbt_gattc_alloc_client(CHAR *client_name, const bluetooth::Uuid &app_uuid);
static int linuxbt_gattc_find_client_by_uuid(const bluetooth::Uuid &app_uuid);
static int linuxbt_gattc_find_client_by_name(const char *client_name);
static int linuxbt_gattc_find_client_by_if(const int client_if);
static int linuxbt_gattc_free_client_by_if(const int client_if);
static void linuxbt_gattc_free_client_by_uuid(const bluetooth::Uuid &app_uuid);
static std::list<LINUXBT_GATTC_DEV_CB>::iterator
linuxbt_gattc_find_dev_by_addr(std::list<LINUXBT_GATTC_DEV_CB> *dev_list, const RawAddress &addr);
static std::list<LINUXBT_GATTC_DEV_CB>::iterator
linuxbt_gattc_find_dev_by_conn_id(std::list<LINUXBT_GATTC_DEV_CB> *dev_list, const int conn_id);

/* STATIC VARIABLE DECLARATIONS
 */
static const btgatt_client_interface_t *linuxbt_gattc_interface = NULL;

static std::mutex s_linuxbt_gattc_mutex;

static LINUXBT_GATTC_CB s_linuxbt_gattc_cb;


btgatt_client_callbacks_t linuxbt_gattc_callbacks =
{
    linuxbt_gattc_register_client_callback,
    linuxbt_gattc_connect_callback,
    linuxbt_gattc_disconnect_callback,
    linuxbt_gattc_search_complete_callback,
    linuxbt_gattc_register_for_notification_callback,
    linuxbt_gattc_notify_callback,
    linuxbt_gattc_read_characteristic_callback,
    linuxbt_gattc_write_characteristic_callback,
    linuxbt_gattc_read_descriptor_callback,
    linuxbt_gattc_write_descriptor_callback,
    linuxbt_gattc_execute_write_callback,
    linuxbt_gattc_read_remote_rssi_callback,
    linuxbt_gattc_configure_mtu_callback,
    linuxbt_gattc_congestion_callback,
    linuxbt_gattc_get_gatt_db_callback,
    NULL, //services_removed_cb
    NULL, //services_added_cb
    linuxbt_gattc_phy_update_callback,
    linuxbt_gattc_conn_update_callback,
    NULL,
};

/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */






int linuxbt_gattc_init(const btgatt_client_interface_t *pt_interface)
{
    UINT32 index = 0;
    LockGuard lock(s_linuxbt_gattc_mutex);
    if (TRUE == s_linuxbt_gattc_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "");
    linuxbt_gattc_interface = pt_interface;

    for (index = 0; index < BT_GATT_MAX_APP_CNT; index++)
    {
        s_linuxbt_gattc_cb.client_cb[index].in_use = 0;
        s_linuxbt_gattc_cb.client_cb[index].client_if = 0;
        s_linuxbt_gattc_cb.client_cb[index].client_name[0] = 0;
        s_linuxbt_gattc_cb.client_cb[index].uuid = bluetooth::Uuid::kEmpty;
        s_linuxbt_gattc_cb.client_cb[index].congested = 0;

        s_linuxbt_gattc_cb.client_cb[index].service.clear();
        s_linuxbt_gattc_cb.client_cb[index].dev_list.clear();
        s_linuxbt_gattc_cb.client_cb[index].msg_list.clear();
    }
    s_linuxbt_gattc_cb.inited = TRUE;
    return BT_SUCCESS;
}

int linuxbt_gattc_deinit(void)
{
    UINT32 index = 0;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LockGuard lock(s_linuxbt_gattc_mutex);
    if (TRUE != s_linuxbt_gattc_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"not inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "");
    linuxbt_gattc_interface = NULL;

    for (index = 0; index < BT_GATT_MAX_APP_CNT; index++)
    {
        s_linuxbt_gattc_cb.client_cb[index].in_use = 0;
        s_linuxbt_gattc_cb.client_cb[index].client_if = 0;
        s_linuxbt_gattc_cb.client_cb[index].client_name[0] = 0;
        s_linuxbt_gattc_cb.client_cb[index].uuid = bluetooth::Uuid::kEmpty;
        s_linuxbt_gattc_cb.client_cb[index].congested = 0;

        s_linuxbt_gattc_cb.client_cb[index].service.clear();
        s_linuxbt_gattc_cb.client_cb[index].dev_list.clear();
        s_linuxbt_gattc_cb.client_cb[index].msg_list.clear();
    }

    s_linuxbt_gattc_cb.inited = FALSE;

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_MAX;
    bt_mw_gattc_notify_app(&gattc_msg);

    return BT_SUCCESS;
}

int linuxbt_gattc_register(CHAR *client_name)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = 0;
    LINUXBT_GATTC_CHECK_NAME(client_name, BT_ERR_STATUS_PARM_INVALID);
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    bluetooth::Uuid app_uuid = bluetooth::Uuid::GetRandom();
    LockGuard lock(s_linuxbt_gattc_mutex);

    index = linuxbt_gattc_find_client_by_name(client_name);
    if (0 <= index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_name %s exists", client_name);
        return BT_ERR_STATUS_DONE;
    }

    index = linuxbt_gattc_find_client_by_uuid(app_uuid);
    if (0 <= index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "uuid %s exists", app_uuid.ToString().c_str());
        return BT_ERR_STATUS_DONE;
    }

    index = linuxbt_gattc_alloc_client(client_name, app_uuid);
    if (0 > index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "alloc cb for %s fail", client_name);
        return BT_ERR_STATUS_NOMEM;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_name=%s, app_uuid = %s",
        client_name, app_uuid.ToString().c_str());

    ret = linuxbt_gattc_interface->register_client(app_uuid);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_unregister(int client_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d", client_if);

    if (0 == client_if) return BT_ERR_STATUS_PARM_INVALID;

    index = linuxbt_gattc_find_client_by_if(client_if);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "not client %d", client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = linuxbt_gattc_interface->unregister_client(client_if);

    linuxbt_gattc_free_client_by_if(client_if);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_connect(BT_GATTC_CONNECT_PARAM *conn_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, conn_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(conn_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc connected to %s", conn_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(conn_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            conn_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(conn_param->addr, bd_addr);

    ret = linuxbt_gattc_interface->connect(conn_param->client_if,
        bd_addr, 0 == conn_param->is_direct ? false : true,
        conn_param->transport, conn_param->opportunistic,
        conn_param->initiating_phys);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_disconnect(BT_GATTC_DISCONNECT_PARAM *disc_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, disc_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(disc_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc %d disconnected to %s",
        disc_param->client_if, disc_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(disc_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            disc_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(disc_param->addr, bd_addr);
    int conn_id = 0;

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        conn_id = 0;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->disconnect(disc_param->client_if,
        bd_addr, conn_id);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_refresh(BT_GATTC_REFRESH_PARAM *refresh_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, refresh_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(refresh_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc refresh to %s", refresh_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(refresh_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            refresh_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(refresh_param->addr, bd_addr);

    ret = linuxbt_gattc_interface->refresh(refresh_param->client_if, bd_addr);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_discover_by_uuid(BT_GATTC_DISCOVER_BY_UUID_PARAM *discover_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    bool is_valid = true;
    std::string uuid_str;
    bluetooth::Uuid service_uuid;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, discover_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(discover_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "discover %s uuid=%s",
        discover_param->addr, discover_param->service_uuid);
    if (0 > (index = linuxbt_gattc_find_client_by_if(discover_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            discover_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(discover_param->addr, bd_addr);

    uuid_str = discover_param->service_uuid;
    service_uuid = bluetooth::Uuid::FromString(uuid_str, &is_valid);
    if (false == is_valid)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "invalid service uuid %s", discover_param->service_uuid);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", discover_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    linuxbt_gattc_interface->btif_gattc_discover_service_by_uuid(conn_id, service_uuid);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_char(BT_GATTC_READ_CHAR_PARAM *read_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, read_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(read_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc read char to %s, handle=%d, auth_req=%d",
        read_param->addr, read_param->handle, read_param->auth_req);
    if (0 > (index = linuxbt_gattc_find_client_by_if(read_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            read_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(read_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", read_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->read_characteristic(conn_id,
        read_param->handle, read_param->auth_req);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_char_by_uuid(BT_GATTC_READ_BY_UUID_PARAM *read_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    bool is_valid = true;
    int conn_id = 0;
    std::string uuid_str;
    bluetooth::Uuid char_uuid;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, read_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(read_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc read char to %s, uuid=%s, s_handle=%d, e_handle=%d, auth_req=%d",
        read_param->addr, read_param->char_uuid, read_param->s_handle,
        read_param->e_handle, read_param->auth_req);
    if (0 > (index = linuxbt_gattc_find_client_by_if(read_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            read_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(read_param->addr, bd_addr);

    uuid_str = read_param->char_uuid;
    char_uuid = bluetooth::Uuid::FromString(uuid_str, &is_valid);
    if (false == is_valid)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "invalid char uuid %s", read_param->char_uuid);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", read_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->read_using_characteristic_uuid(conn_id,
        char_uuid, read_param->s_handle, read_param->e_handle,
        read_param->auth_req);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_desc(BT_GATTC_READ_DESC_PARAM *read_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, read_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(read_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc read char to %s, handle=%d",
        read_param->addr, read_param->handle);
    if (0 > (index = linuxbt_gattc_find_client_by_if(read_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            read_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(read_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", read_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->read_descriptor(conn_id,
        read_param->handle, read_param->auth_req);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_write_char(BT_GATTC_WRITE_CHAR_PARAM *write_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, write_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(write_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc write char to %s, handle=%d, type=%d, auth_req=%d, len=%d",
        write_param->addr, write_param->handle, write_param->write_type,
        write_param->auth_req, write_param->value_len);
    if (0 > (index = linuxbt_gattc_find_client_by_if(write_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            write_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(write_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", write_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    std::vector<uint8_t> value(write_param->value, write_param->value+write_param->value_len);

    ret = linuxbt_gattc_interface->write_characteristic(conn_id,
        write_param->handle, write_param->write_type, write_param->auth_req, value);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_write_desc(BT_GATTC_WRITE_DESC_PARAM *write_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, write_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(write_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc write char to %s, handle=%d",
        write_param->addr, write_param->handle);
    if (0 > (index = linuxbt_gattc_find_client_by_if(write_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            write_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(write_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", write_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    std::vector<uint8_t> value(write_param->value, write_param->value+write_param->value_len);

    ret = linuxbt_gattc_interface->write_descriptor(conn_id,
        write_param->handle, write_param->auth_req, value);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_exec_write(BT_GATTC_EXEC_WRITE_PARAM *exec_write_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, exec_write_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(exec_write_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc exec write to %s, execute=%d",
        exec_write_param->addr, exec_write_param->execute);
    if (0 > (index = linuxbt_gattc_find_client_by_if(exec_write_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            exec_write_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(exec_write_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", exec_write_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->execute_write(conn_id, exec_write_param->execute);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_reg_notification(BT_GATTC_REG_NOTIF_PARAM *reg_notif_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, reg_notif_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(reg_notif_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s, handle=%d",
        reg_notif_param->addr, reg_notif_param->handle);

    if (0 > (index = linuxbt_gattc_find_client_by_if(reg_notif_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            reg_notif_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(reg_notif_param->addr, bd_addr);

    if (reg_notif_param->registered)
    {
        ret = linuxbt_gattc_interface->register_for_notification(reg_notif_param->client_if,
            bd_addr, reg_notif_param->handle);
    }
    else
    {
        ret = linuxbt_gattc_interface->deregister_for_notification(reg_notif_param->client_if,
            bd_addr, reg_notif_param->handle);
    }
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_rssi(BT_GATTC_READ_RSSI_PARAM *read_rssi_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, read_rssi_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(read_rssi_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s",
        read_rssi_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(read_rssi_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            read_rssi_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(read_rssi_param->addr, bd_addr);

    ret = linuxbt_gattc_interface->read_remote_rssi(read_rssi_param->client_if,
        bd_addr);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_conn_update(BT_GATTC_CONN_UPDATE_PARAM *conn_update_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, conn_update_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(conn_update_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s, min_int=%d, max_int=%d,"
        "latency=%d, timeout=%d, min_ce_len=%d, max_ce_len=%d",
        conn_update_param->addr, conn_update_param->min_interval,
        conn_update_param->max_interval, conn_update_param->latency,
        conn_update_param->timeout, conn_update_param->min_ce_len,
        conn_update_param->max_ce_len);
    if (0 > (index = linuxbt_gattc_find_client_by_if(conn_update_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            conn_update_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(conn_update_param->addr, bd_addr);

    ret = linuxbt_gattc_interface->conn_parameter_update(bd_addr,
        conn_update_param->min_interval,
        conn_update_param->max_interval, conn_update_param->latency,
        conn_update_param->timeout, conn_update_param->min_ce_len,
        conn_update_param->max_ce_len);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_get_dev_type(BT_GATTC_GET_DEV_TYPE_PARAM *get_dev_type_param)
{
    INT32 ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, get_dev_type_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(get_dev_type_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s",
        get_dev_type_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(get_dev_type_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            get_dev_type_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(get_dev_type_param->addr, bd_addr);

    ret = linuxbt_gattc_interface->get_device_type(bd_addr);

    return ret;
}

int linuxbt_gattc_change_mtu(BT_GATTC_CHG_MTU_PARAM *chg_mtu_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, chg_mtu_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(chg_mtu_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s, mtu=%d",
        chg_mtu_param->addr, chg_mtu_param->mtu);

    if ((BT_GATT_MIN_MTU_SIZE > chg_mtu_param->mtu)
        || (BT_GATT_MAX_MTU_SIZE < chg_mtu_param->mtu))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s, mtu=%d must between %d ~ %d",
            chg_mtu_param->addr, chg_mtu_param->mtu,
            BT_GATT_MIN_MTU_SIZE, BT_GATT_MAX_MTU_SIZE);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (0 > (index = linuxbt_gattc_find_client_by_if(chg_mtu_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            chg_mtu_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(chg_mtu_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", chg_mtu_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->configure_mtu(conn_id, chg_mtu_param->mtu);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_set_prefer_phy(BT_GATTC_PHY_SET_PARAM *phy_set_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, phy_set_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(phy_set_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s, tx_phy=0x%x, rx_phy=0x%x, phy_option=0x%x",
        phy_set_param->addr, phy_set_param->tx_phy,
        phy_set_param->rx_phy, phy_set_param->phy_options);

    if (phy_set_param->phy_options > GATT_PHY_OPT_CODED_S8)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"phy_options: 0x%x invalid",
            phy_set_param->phy_options);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    phy_set_param->tx_phy &= 0x07;
    phy_set_param->rx_phy &= 0x07;

    if (0 > (index = linuxbt_gattc_find_client_by_if(phy_set_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            phy_set_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(phy_set_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", phy_set_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = linuxbt_gattc_interface->set_preferred_phy(bd_addr,
        phy_set_param->tx_phy,
        phy_set_param->rx_phy, phy_set_param->phy_options);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_phy(BT_GATTC_PHY_READ_PARAM *read_phy_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, read_phy_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(read_phy_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "addr %s", read_phy_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(read_phy_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            read_phy_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(read_phy_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", read_phy_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = linuxbt_gattc_interface->read_phy(bd_addr,
        base::Bind(&linuxbt_gattc_read_phy_callback,
        read_phy_param->client_if, bd_addr));

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_get_gatt_db(BT_GATTC_GET_GATT_DB_PARAM *get_gatt_db_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    int conn_id = 0;
    LINUXBT_GATTC_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, get_gatt_db_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(get_gatt_db_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gattc %d addr %s",
        get_gatt_db_param->client_if, get_gatt_db_param->addr);
    if (0 > (index = linuxbt_gattc_find_client_by_if(get_gatt_db_param->client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            get_gatt_db_param->client_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(get_gatt_db_param->addr, bd_addr);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "addr %s no connection", get_gatt_db_param->addr);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gattc_interface->get_gatt_db(conn_id);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}


static void linuxbt_gattc_read_phy_callback(uint8_t client_if, RawAddress bda, uint8_t tx_phy,
                            uint8_t rx_phy, uint8_t status) {
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if = %d, bda=%s, tx_phy= 0x%x, rx_phy=0x%x, status=%d",
        client_if, bda.ToString().c_str(), tx_phy, rx_phy, status);

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d", client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bda);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %s",
            client_if, bda.ToString().c_str());
        return;
    }

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_PHY_READ;
    gattc_msg.client_if = client_if;
    gattc_msg.data.phy_read_data.rx_phy = rx_phy;
    gattc_msg.data.phy_read_data.tx_phy = tx_phy;
    strncpy(gattc_msg.data.phy_read_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.phy_read_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.phy_read_data.status = status;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_register_client_callback(int status, int client_if,
                                                const bluetooth::Uuid& app_uuid)
{
    BT_GATTC_EVENT_PARAM gattc_msg;
    BT_DBG_NORMAL(BT_DEBUG_GATT,"client_if:%d, status:%d",
                 client_if, status);

    int index = linuxbt_gattc_find_client_by_uuid(app_uuid);
    if (0 > index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "find client cb fail, uuid=%s, unregister",
            app_uuid.ToString().c_str());

        linuxbt_gattc_interface->unregister_client(client_if);
        return;
    }

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_REGISTER;
    gattc_msg.client_if = client_if;
    gattc_msg.data.register_data.status = (BT_GATT_STATUS)status;
    memcpy(gattc_msg.data.register_data.client_name,
        s_linuxbt_gattc_cb.client_cb[index].client_name,
        sizeof(s_linuxbt_gattc_cb.client_cb[index].client_name));

    if (BT_STATUS_SUCCESS == status)
    {
        s_linuxbt_gattc_cb.client_cb[index].client_if = client_if;
    }
    else if (BT_GATT_STATUS_DUP_REG == status)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT, "duplicate register %s",
            s_linuxbt_gattc_cb.client_cb[index].client_name);
    }
    else
    {
        linuxbt_gattc_free_client_by_uuid(app_uuid);
    }

    bt_mw_gattc_notify_app(&gattc_msg);
}


static void linuxbt_gattc_connect_callback(int conn_id, int status,
                                        int client_if, const RawAddress& bda)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d %s connected status=%d",
        client_if, bda.ToString().c_str(), status);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bda);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        if (BT_GATT_STATUS_OK == status)
        {
            LINUXBT_GATTC_DEV_CB &dev =
                *(s_linuxbt_gattc_cb.client_cb[index].dev_list.emplace(it));

            dev.addr = bda;
            dev.connected = 1;
            dev.conn_id = conn_id;
        }
    }
    else
    {
        if (BT_GATT_STATUS_OK != status)
        {
            s_linuxbt_gattc_cb.client_cb[index].dev_list.erase(it);
        }
    }

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_CONNECTION;
    gattc_msg.client_if = client_if;
    gattc_msg.data.connection_data.status = (BT_GATT_STATUS)status;
    gattc_msg.data.connection_data.connected = (BT_GATT_STATUS_OK == status) ? 1 : 0;
    strncpy(gattc_msg.data.connection_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.connection_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_disconnect_callback(int conn_id, int status,
                                                        int client_if, const RawAddress& bda)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d %s disconnected status=%d",
        client_if, bda.ToString().c_str(), status);

    auto it = linuxbt_gattc_find_dev_by_addr(&s_linuxbt_gattc_cb.client_cb[index].dev_list, bda);
    if (it != s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        s_linuxbt_gattc_cb.client_cb[index].dev_list.erase(it);
    }

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_CONNECTION;
    gattc_msg.client_if = client_if;
    gattc_msg.data.connection_data.status = (BT_GATT_STATUS)status;
    gattc_msg.data.connection_data.connected = 0;
    strncpy(gattc_msg.data.connection_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.connection_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_search_complete_callback(int conn_id, int status)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT,"conn_id=%d, search complete status = %d", conn_id, status);
    if (NULL != linuxbt_gattc_interface)
    {
        linuxbt_gattc_interface->get_gatt_db(conn_id);
    }
}

static void linuxbt_gattc_register_for_notification_callback(int conn_id, int registered,
                                                   int status, uint16_t handle)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "conn_id=%d, status=%d, handle=%d, registered=%d",
        conn_id, status, handle, registered);

    return;
}

static void linuxbt_gattc_notify_callback(int conn_id,
                                const btgatt_notify_params_t& p_data)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, conn_id=%d, len=%d, handle=%d, is_notify=%d",
        client_if, it->addr.ToString().c_str(), conn_id,
        p_data.len, p_data.handle, p_data.is_notify);


    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_NOTIFY;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.notif_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.notif_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.notif_data.handle = p_data.handle;
    gattc_msg.data.notif_data.is_notify = p_data.is_notify;

    if (p_data.len > BT_GATT_MAX_VALUE_LEN)
    {
        gattc_msg.data.notif_data.value_len = BT_GATT_MAX_VALUE_LEN;
    }
    else
    {
        gattc_msg.data.notif_data.value_len = p_data.len;
    }
    memcpy((void*)&gattc_msg.data.notif_data.value, p_data.value,
        gattc_msg.data.notif_data.value_len);

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_read_characteristic_callback(int conn_id, int status,
                                             btgatt_read_params_t* p_data)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d, addr=%s status=%d, conn_id=%d, len=%d, handle=%d, value_type=%d",
        client_if, it->addr.ToString().c_str(), status, conn_id,
        p_data->value.len, p_data->handle, p_data->value_type);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_READ_CHAR_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.read_char_rsp_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.read_char_rsp_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.read_char_rsp_data.handle = p_data->handle;
    gattc_msg.data.read_char_rsp_data.status = (BT_GATT_STATUS)p_data->status;

    if (p_data->value.len > BT_GATT_MAX_VALUE_LEN)
    {
        gattc_msg.data.read_char_rsp_data.value_len = BT_GATT_MAX_VALUE_LEN;
    }
    else
    {
        gattc_msg.data.read_char_rsp_data.value_len = p_data->value.len;
    }
    memcpy((void*)&gattc_msg.data.read_char_rsp_data.value, p_data->value.value,
        gattc_msg.data.read_char_rsp_data.value_len);

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_write_characteristic_callback(int conn_id, int status,
                                              uint16_t handle)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, conn_id=%d, handle=%d,",
        client_if, it->addr.ToString().c_str(), status, conn_id, handle);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_WRITE_CHAR_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.write_char_rsp_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.write_char_rsp_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.write_char_rsp_data.handle = handle;
    gattc_msg.data.write_char_rsp_data.status = (BT_GATT_STATUS)status;


    /* if congested, buffer the event */
    if (s_linuxbt_gattc_cb.client_cb[index].congested)
    {
        if (gattc_msg.data.write_char_rsp_data.status == BT_GATT_STATUS_CONGESTED)
        {
            gattc_msg.data.write_char_rsp_data.status = BT_GATT_STATUS_OK;
        }
        s_linuxbt_gattc_cb.client_cb[index].msg_list.push_back(gattc_msg);
    }
    else
    {
        bt_mw_gattc_notify_app(&gattc_msg);
    }

    return;
}

static void linuxbt_gattc_read_descriptor_callback(int conn_id, int status,
                                         const btgatt_read_params_t& p_data)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d, addr=%s status=%d, conn_id=%d, len=%d, handle=%d, value_type=%d",
        client_if, it->addr.ToString().c_str(), status, conn_id,
        p_data.value.len, p_data.handle, p_data.value_type);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_READ_DESC_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.read_desc_rsp_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.read_desc_rsp_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.read_desc_rsp_data.handle = p_data.handle;
    gattc_msg.data.read_desc_rsp_data.status = (BT_GATT_STATUS)p_data.status;

    if (p_data.value.len > BT_GATT_MAX_VALUE_LEN)
    {
        gattc_msg.data.read_desc_rsp_data.value_len = BT_GATT_MAX_VALUE_LEN;
    }
    else
    {
        gattc_msg.data.read_desc_rsp_data.value_len = p_data.value.len;
    }
    memcpy((void*)&gattc_msg.data.read_desc_rsp_data.value, p_data.value.value,
        gattc_msg.data.read_desc_rsp_data.value_len);

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_write_descriptor_callback(int conn_id, int status,
                                          uint16_t handle)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, conn_id=%d, handle=%d,",
        client_if, it->addr.ToString().c_str(), status, conn_id, handle);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_WRITE_DESC_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.write_desc_rsp_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.write_desc_rsp_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.write_desc_rsp_data.handle = handle;
    gattc_msg.data.write_desc_rsp_data.status = (BT_GATT_STATUS)status;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_execute_write_callback(int conn_id, int status)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, conn_id=%d",
        client_if, it->addr.ToString().c_str(), status, conn_id);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_EXEC_WRITE_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.exec_write_rsp_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.exec_write_rsp_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.exec_write_rsp_data.status = (BT_GATT_STATUS)status;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_read_remote_rssi_callback(int client_if,
                                                                const RawAddress& bda,
                                                                int rssi,
                                                                int status)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, rssi=%d",
        client_if, bda.ToString().c_str(), status, rssi);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_READ_RSSI_RSP;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.read_rssi_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.read_rssi_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.read_rssi_data.status = (BT_GATT_STATUS)status;
    gattc_msg.data.read_rssi_data.rssi = rssi;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_configure_mtu_callback(int conn_id,
                                                            int status,
                                                            int mtu)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);
    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d",
            client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, mtu=%d",
        client_if, it->addr.ToString().c_str(), status, mtu);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_MTU_CHANGE;
    gattc_msg.client_if = client_if;
    strncpy(gattc_msg.data.mtu_chg_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.mtu_chg_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.mtu_chg_data.status = (BT_GATT_STATUS)status;
    gattc_msg.data.mtu_chg_data.mtu = mtu;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_congestion_callback(int conn_id, bool congested)
{
    int index = -1;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d", client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if = %d, bda=%s, congested=%d",
        client_if, it->addr.ToString().c_str(), congested);

    s_linuxbt_gattc_cb.client_cb[index].congested = congested;

    if (false == congested && !s_linuxbt_gattc_cb.client_cb[index].msg_list.empty())
    {
        auto msg_it = s_linuxbt_gattc_cb.client_cb[index].msg_list.begin();
        auto end_it = s_linuxbt_gattc_cb.client_cb[index].msg_list.end();
        for (; msg_it != end_it; msg_it++)
        {
            BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if=%d addr=%s, status=%d, handle=%d,",
                msg_it->client_if, msg_it->data.write_char_rsp_data.addr,
                msg_it->data.write_char_rsp_data.status,
                msg_it->data.write_char_rsp_data.handle );

            bt_mw_gattc_notify_app(&(*msg_it));
        }
        s_linuxbt_gattc_cb.client_cb[index].msg_list.clear();
    }

    return;
}


static void linuxbt_gattc_get_gatt_db_callback(int conn_id, const btgatt_db_element_t* db,
                                     int count)
{
    int index = -1;
    BT_GATTC_EVENT_PARAM gattc_msg;
    int attr_index = 0;
    int client_if = 0;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;
    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d", client_if);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if = %d, cnt=%d, gattc_msg size=%d",
        client_if, count, sizeof(gattc_msg));

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_GET_GATT_DB;
    gattc_msg.client_if = client_if;

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }
    strncpy(gattc_msg.data.get_gatt_db_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.get_gatt_db_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    do
    {
        UINT32 max_cnt = ((count - attr_index) > BT_GATT_MAX_ATTR_CNT)
                        ? BT_GATT_MAX_ATTR_CNT : count - attr_index;
        UINT32 i = 0;
        for (i = 0;i < max_cnt;i++)
        {
            strncpy(gattc_msg.data.get_gatt_db_data.attrs[i].uuid,
                db[attr_index + i].uuid.ToString().c_str(),
                BT_GATT_MAX_UUID_LEN - 1);
            gattc_msg.data.get_gatt_db_data.attrs[i].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
            if (db[attr_index + i].type == BTGATT_DB_PRIMARY_SERVICE
                 || db[attr_index + i].type == BTGATT_DB_SECONDARY_SERVICE)
            {
                gattc_msg.data.get_gatt_db_data.attrs[i].handle
                    = db[attr_index + i].start_handle;
            }
            else
            {
                gattc_msg.data.get_gatt_db_data.attrs[i].handle
                    = db[attr_index + i].attribute_handle;
            }

            gattc_msg.data.get_gatt_db_data.attrs[i].type
                = (GATT_ATTR_TYPE)db[attr_index + i].type;
            gattc_msg.data.get_gatt_db_data.attrs[i].properties
                = (GATT_ATTR_TYPE)db[attr_index + i].properties;
            gattc_msg.data.get_gatt_db_data.attrs[i].permissions
                = (GATT_ATTR_TYPE)db[attr_index + i].permissions;

            BT_DBG_NORMAL(BT_DEBUG_GATT, "db[%d] type = %d, uuid=%s, handle=%d",
                attr_index + i,
                db[attr_index + i].type,
                db[attr_index + i].uuid.ToString().c_str(),
                gattc_msg.data.get_gatt_db_data.attrs[i].handle);
        }
        gattc_msg.data.get_gatt_db_data.attr_cnt = max_cnt;
        if (0 == attr_index)
        {
            gattc_msg.data.get_gatt_db_data.is_first = 1;
        }
        else
        {
            gattc_msg.data.get_gatt_db_data.is_first = 0;
        }

        //BT_DBG_NORMAL(BT_DEBUG_GATT, "attr_cnt: %d, gattc_msg size=%d",
        //    max_cnt, sizeof(gattc_msg));
        bt_mw_gattc_notify_app(&gattc_msg);

        attr_index += max_cnt;
    }while(attr_index < count);

    return;
}

static void linuxbt_gattc_phy_update_callback(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status)
{
    int index = -1;
    int client_if = 0;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d", client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "client_if = %d, bda=%s, tx_phy= 0x%x, rx_phy=0x%x, status=%d",
        client_if, it->addr.ToString().c_str(), tx_phy, rx_phy, status);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_PHY_UPDATE;
    gattc_msg.client_if = client_if;
    gattc_msg.data.phy_upd_data.tx_phy = tx_phy;
    gattc_msg.data.phy_upd_data.rx_phy = rx_phy;
    strncpy(gattc_msg.data.phy_upd_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.phy_upd_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.phy_upd_data.status = (BT_GATT_STATUS)status;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static void linuxbt_gattc_conn_update_callback(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                      uint8_t status)
{
    int index = -1;
    int client_if = 0;
    BT_GATTC_EVENT_PARAM gattc_msg;
    LINUXBT_GATTC_CHECK_INITED();
    LockGuard lock(s_linuxbt_gattc_mutex);

    client_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gattc_find_client_by_if(client_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find client_if: %d", client_if);
        return;
    }

    auto it = linuxbt_gattc_find_dev_by_conn_id(&s_linuxbt_gattc_cb.client_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gattc_cb.client_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"client_if=%d no connection: %d ",
            client_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "clientIf = %d, bda=%s, interval= %u, latency=%u, timeout=%u, status=%d",
        client_if, it->addr.ToString().c_str(), interval, latency, timeout, status);

    memset((void*)&gattc_msg, 0, sizeof(gattc_msg));
    gattc_msg.event = BT_GATTC_EVENT_CONN_UPDATE;
    gattc_msg.client_if = client_if;
    gattc_msg.data.conn_upd_data.interval = interval;
    gattc_msg.data.conn_upd_data.latency = latency;
    gattc_msg.data.conn_upd_data.timeout = timeout;
    strncpy(gattc_msg.data.conn_upd_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gattc_msg.data.conn_upd_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gattc_msg.data.conn_upd_data.status = (BT_GATT_STATUS)status;

    bt_mw_gattc_notify_app(&gattc_msg);

    return;
}

static int linuxbt_gattc_alloc_client(CHAR *client_name, const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if (s_linuxbt_gattc_cb.client_cb[i].in_use == 0)
        {
            s_linuxbt_gattc_cb.client_cb[i].in_use = 1;
            s_linuxbt_gattc_cb.client_cb[i].uuid = app_uuid;
            s_linuxbt_gattc_cb.client_cb[i].congested = 0;
            strncpy(s_linuxbt_gattc_cb.client_cb[i].client_name,
                client_name, BT_GATT_MAX_NAME_LEN - 1);
            s_linuxbt_gattc_cb.client_cb[i].client_name[BT_GATT_MAX_NAME_LEN-1] = '\0';
            return i;
        }
    }

    return -1;
}

static int linuxbt_gattc_find_client_by_uuid(const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gattc_cb.client_cb[i].in_use != 0)
            && (s_linuxbt_gattc_cb.client_cb[i].uuid == app_uuid))
        {
            return i;
        }
    }
    return -1;
}

static int linuxbt_gattc_find_client_by_name(const char *client_name)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gattc_cb.client_cb[i].in_use != 0)
            && (0 == strncasecmp(s_linuxbt_gattc_cb.client_cb[i].client_name,
                                client_name, BT_GATT_MAX_NAME_LEN - 1)))
        {
            return i;
        }
    }
    return -1;
}


static int linuxbt_gattc_find_client_by_if(const int client_if)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gattc_cb.client_cb[i].in_use != 0)
            && (s_linuxbt_gattc_cb.client_cb[i].client_if == client_if))
        {
            return i;
        }
    }
    return -1;
}


static int linuxbt_gattc_free_client_by_if(const int client_if)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gattc_cb.client_cb[i].in_use != 0)
            && (s_linuxbt_gattc_cb.client_cb[i].client_if == client_if))
        {
            s_linuxbt_gattc_cb.client_cb[i].service.clear();
            s_linuxbt_gattc_cb.client_cb[i].dev_list.clear();
            s_linuxbt_gattc_cb.client_cb[i].msg_list.clear();

            s_linuxbt_gattc_cb.client_cb[i].in_use = 0;
            s_linuxbt_gattc_cb.client_cb[i].client_if = 0;
            s_linuxbt_gattc_cb.client_cb[i].client_name[0] = 0;
            s_linuxbt_gattc_cb.client_cb[i].uuid = bluetooth::Uuid::kEmpty;
            s_linuxbt_gattc_cb.client_cb[i].congested = 0;

            BT_DBG_NORMAL(BT_DEBUG_GATT,"free %d", client_if);
            return i;
        }
    }

    return -1;
}

static void linuxbt_gattc_free_client_by_uuid(const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gattc_cb.client_cb[i].in_use != 0)
            && (s_linuxbt_gattc_cb.client_cb[i].uuid == app_uuid))
        {
            s_linuxbt_gattc_cb.client_cb[i].service.clear();
            s_linuxbt_gattc_cb.client_cb[i].dev_list.clear();
            s_linuxbt_gattc_cb.client_cb[i].msg_list.clear();

            s_linuxbt_gattc_cb.client_cb[i].in_use = 0;
            s_linuxbt_gattc_cb.client_cb[i].client_if = 0;
            s_linuxbt_gattc_cb.client_cb[i].client_name[0] = 0;
            s_linuxbt_gattc_cb.client_cb[i].uuid = bluetooth::Uuid::kEmpty;
            s_linuxbt_gattc_cb.client_cb[i].congested = 0;
            BT_DBG_NORMAL(BT_DEBUG_GATT,"free uuid-%s", app_uuid.ToString().c_str());
            return;
        }
    }
    return;
}


static std::list<LINUXBT_GATTC_DEV_CB>::iterator linuxbt_gattc_find_dev_by_addr(std::list<LINUXBT_GATTC_DEV_CB> *dev_list, const RawAddress &addr)
{
    auto end_it = dev_list->end();
    auto it = dev_list->begin();
    for (; it != end_it; it++) {
      if (it->addr == addr) {
        return it;
      }
    }

    return it;
}


static std::list<LINUXBT_GATTC_DEV_CB>::iterator linuxbt_gattc_find_dev_by_conn_id(std::list<LINUXBT_GATTC_DEV_CB> *dev_list, const int conn_id)
{
    auto end_it = dev_list->end();
    auto it = dev_list->begin();
    for (; it != end_it; it++) {
      if (it->conn_id == conn_id) {
        return it;
      }
    }

    return it;
}

