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

/* FILE NAME:  linuxbt_gatts_if.cc
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
#include "bt_gatt_server.h"
#include "u_bt_mw_common.h"
#include "bt_mw_common.h"
#include "u_bt_mw_gatt.h"
#include "bt_mw_log.h"
#include "linuxbt_gatts_if.h"
#include "linuxbt_common.h"
#include "bt_mw_gatt.h"
#include "raw_address.h"

using LockGuard = std::lock_guard<std::mutex>;

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_GATTS_CHECK_ADDR(addr, ret) do{                                         \
        if ((NULL == (addr)) || ((MAX_BDADDR_LEN - 1) != strlen(addr)))                 \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_GATT, "invalid addr(%s)", addr==NULL?"NULL":(addr));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)


#define LINUXBT_GATTS_CHECK_NAME(name, ret) do{                                         \
        if ((NULL == (name)) || (0 == strlen(name)) || (BT_GATT_MAX_NAME_LEN - 1 < strlen(name)))   \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_GATT, "invalid name(%s)", name==NULL?"NULL":(name));  \
            return (ret);                                                               \
        }                                                                               \
    }while(0)

#define LINUXBT_GATTS_CHECK_INITED(ret)    do {                     \
        if (NULL == linuxbt_gatts_interface)                        \
        {                                                           \
            BT_DBG_ERROR(BT_DEBUG_GATT, "gatt server not init");    \
            return ret;                                             \
        }                                                           \
    }while(0)


#define LINUXBT_GATTS_SET_MSG_LEN(msg) do{  \
            msg.hdr.len = sizeof(BT_GATTS_EVENT_PARAM);      \
            }while(0)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    RawAddress addr;
    int connected;
    int conn_id;
} LINUXBT_GATTS_DEV_CB;

typedef struct
{
    int in_use;
    int server_if;
    char server_name[BT_GATT_MAX_NAME_LEN];
    bluetooth::Uuid uuid;
    int congested;

    std::vector<btgatt_db_element_t> service;
    std::list<LINUXBT_GATTS_DEV_CB> dev_list;
    std::list<uint16_t> srvc_list;
    std::list<BT_GATTS_EVENT_PARAM> msg_list;
} LINUXBT_GATTS_SERVER_CB;

typedef struct
{
    BOOL inited;
    LINUXBT_GATTS_SERVER_CB server_cb[BT_GATT_MAX_APP_CNT];
} LINUXBT_GATTS_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void linuxbt_gatts_delete_services(LINUXBT_GATTS_SERVER_CB *server_cb);

static void linuxbt_gatts_register_server_callback(int status, int server_if,
                                         const bluetooth::Uuid& app_uuid);

static void linuxbt_gatts_connection_callback(int conn_id, int server_if,
                                                int connected,
                                                const RawAddress& bda);

static void linuxbt_gatts_service_added_callback(
    int status, int server_if, std::vector<btgatt_db_element_t> service);

static void linuxbt_gatts_service_stopped_callback(int status, int server_if,
                                         int srvc_handle);

static void linuxbt_gatts_service_deleted_callback(int status, int server_if,
                                         int srvc_handle);

static void linuxbt_gatts_request_read_callback(int conn_id, int trans_id,
                                      const RawAddress& bda, int attr_handle,
                                      int offset, bool is_long);

static void linuxbt_gatts_request_write_callback(int conn_id, int trans_id,
                                       const RawAddress& bda, int attr_handle,
                                       int offset, bool need_rsp, bool is_prep,
                                       std::vector<uint8_t> value);

static void linuxbt_gatts_request_exec_write_callback(int conn_id, int trans_id,
                                            const RawAddress& bda,
                                            int exec_write);

static void linuxbt_gatts_response_confirmation_callback(int status,
                                                                       int handle);

static void linuxbt_gatts_indication_sent_callback(int conn_id, int status);

static void linuxbt_gatts_mtu_changed_callback(int conn_id, int mtu);

static void linuxbt_gatts_congestion_callback(int conn_id, bool congested);

static void linuxbt_gatts_phy_update_callback(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status);

static void linuxbt_gatts_conn_update_callback(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                      uint8_t status);
static void linuxbt_gatts_read_phy_callback(uint8_t serverIf, RawAddress bda, uint8_t tx_phy,
                            uint8_t rx_phy, uint8_t status);

static int linuxbt_gatts_alloc_server(CHAR *server_name,
    const bluetooth::Uuid &app_uuid);

static int linuxbt_gatts_find_server_by_uuid(const bluetooth::Uuid &app_uuid);
static int linuxbt_gatts_find_server_by_name(const char *server_name);
static int linuxbt_gatts_find_server_by_if(const int server_if);
static int linuxbt_gatts_free_server_by_if(const int server_if);
static void linuxbt_gatts_free_server_by_uuid(const bluetooth::Uuid &app_uuid);
static std::list<LINUXBT_GATTS_DEV_CB>::iterator
    linuxbt_gatts_find_dev_by_addr(std::list<LINUXBT_GATTS_DEV_CB> *dev_list, const RawAddress &addr);
static std::list<LINUXBT_GATTS_DEV_CB>::iterator
    linuxbt_gatts_find_dev_by_conn_id(std::list<LINUXBT_GATTS_DEV_CB> *dev_list, const int conn_id);

/* STATIC VARIABLE DECLARATIONS
 */
static const btgatt_server_interface_t *linuxbt_gatts_interface = NULL;

static LINUXBT_GATTS_CB s_linuxbt_gatts_cb;

btgatt_server_callbacks_t linuxbt_gatts_callbacks =
{
    linuxbt_gatts_register_server_callback,
    linuxbt_gatts_connection_callback,
    linuxbt_gatts_service_added_callback,
    linuxbt_gatts_service_stopped_callback,
    linuxbt_gatts_service_deleted_callback,
    linuxbt_gatts_request_read_callback,
    linuxbt_gatts_request_read_callback,
    linuxbt_gatts_request_write_callback,
    linuxbt_gatts_request_write_callback,
    linuxbt_gatts_request_exec_write_callback,
    linuxbt_gatts_response_confirmation_callback,
    linuxbt_gatts_indication_sent_callback,
    linuxbt_gatts_congestion_callback,
    linuxbt_gatts_mtu_changed_callback,
    linuxbt_gatts_phy_update_callback,
    linuxbt_gatts_conn_update_callback,
};

static std::mutex s_linuxbt_gatts_mutex;

/* EXPORTED SUBPROGRAM BODIES
 */


int linuxbt_gatts_init(const btgatt_server_interface_t *pt_interface)
{
    UINT32 index = 0;
    LockGuard lock(s_linuxbt_gatts_mutex);
    if (TRUE == s_linuxbt_gatts_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "");
    linuxbt_gatts_interface = pt_interface;

    for (index = 0; index < BT_GATT_MAX_APP_CNT; index++)
    {
        s_linuxbt_gatts_cb.server_cb[index].in_use = 0;
        s_linuxbt_gatts_cb.server_cb[index].server_if = 0;
        s_linuxbt_gatts_cb.server_cb[index].server_name[0] = 0;
        s_linuxbt_gatts_cb.server_cb[index].uuid = bluetooth::Uuid::kEmpty;
        s_linuxbt_gatts_cb.server_cb[index].congested = 0;

        s_linuxbt_gatts_cb.server_cb[index].service.clear();
        s_linuxbt_gatts_cb.server_cb[index].dev_list.clear();
        s_linuxbt_gatts_cb.server_cb[index].srvc_list.clear();
        s_linuxbt_gatts_cb.server_cb[index].msg_list.clear();
    }
    s_linuxbt_gatts_cb.inited = TRUE;

    return BT_SUCCESS;
}

int linuxbt_gatts_deinit(void)
{
    UINT32 index = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LockGuard lock(s_linuxbt_gatts_mutex);
    if (TRUE != s_linuxbt_gatts_cb.inited)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"not inited");
        return BT_SUCCESS;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, "");
    linuxbt_gatts_interface = NULL;

    for (index = 0; index < BT_GATT_MAX_APP_CNT; index++)
    {
        s_linuxbt_gatts_cb.server_cb[index].in_use = 0;
        s_linuxbt_gatts_cb.server_cb[index].server_if = 0;
        s_linuxbt_gatts_cb.server_cb[index].server_name[0] = 0;
        s_linuxbt_gatts_cb.server_cb[index].uuid = bluetooth::Uuid::kEmpty;
        s_linuxbt_gatts_cb.server_cb[index].congested = 0;

        s_linuxbt_gatts_cb.server_cb[index].service.clear();
        s_linuxbt_gatts_cb.server_cb[index].dev_list.clear();
        s_linuxbt_gatts_cb.server_cb[index].srvc_list.clear();
        s_linuxbt_gatts_cb.server_cb[index].msg_list.clear();
    }

    s_linuxbt_gatts_cb.inited = FALSE;

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_MAX;
    bt_mw_gatts_notify_app(&gatts_msg);

    return BT_SUCCESS;
}

int linuxbt_gatts_register(CHAR *server_name)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = 0;
    LINUXBT_GATTS_CHECK_NAME(server_name, BT_ERR_STATUS_PARM_INVALID);
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    bluetooth::Uuid app_uuid = bluetooth::Uuid::GetRandom();
    LockGuard lock(s_linuxbt_gatts_mutex);

    index = linuxbt_gatts_find_server_by_name(server_name);
    if (0 <= index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_name %s exists", server_name);
        return BT_ERR_STATUS_DONE;
    }

    index = linuxbt_gatts_find_server_by_uuid(app_uuid);
    if (0 <= index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "uuid %s exists", app_uuid.ToString().c_str());
        return BT_ERR_STATUS_DONE;
    }

    index = linuxbt_gatts_alloc_server(server_name, app_uuid);
    if (0 > index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "alloc cb for %s fail", server_name);
        return BT_ERR_STATUS_NOMEM;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_name=%s, app_uuid = %s",
        server_name, app_uuid.ToString().c_str());

    ret = linuxbt_gatts_interface->register_server(app_uuid);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_unregister(int server_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    INT32 index = 0;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if=%d", server_if);

    if (0 == server_if) return BT_ERR_STATUS_PARM_INVALID;

    index = linuxbt_gatts_find_server_by_if(server_if);
    if (index < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "not server %d", server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_gatts_delete_services(&s_linuxbt_gatts_cb.server_cb[index]);

    ret = linuxbt_gatts_interface->unregister_server(server_if);

    linuxbt_gatts_free_server_by_if(server_if);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_connect(BT_GATTS_CONNECT_PARAM *conn_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, conn_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(conn_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gatts connected to %s", conn_param->addr);
    if (0 > (index = linuxbt_gatts_find_server_by_if(conn_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            conn_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(conn_param->addr, bd_addr);

    ret = linuxbt_gatts_interface->connect(conn_param->server_if,
        bd_addr, 0 == conn_param->is_direct ? false : true, conn_param->transport);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_disconnect(BT_GATTS_DISCONNECT_PARAM *disc_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, disc_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(disc_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "gatts disconnected to %s", disc_param->addr);
    if (0 > (index = linuxbt_gatts_find_server_by_if(disc_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            disc_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(disc_param->addr, bd_addr);
    int conn_id = 0;

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        conn_id = 0;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gatts_interface->disconnect(disc_param->server_if,
        bd_addr, conn_id);

    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_add_service(BT_GATTS_SERVICE_ADD_PARAM *add_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    UINT32 attr_index = 0;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, add_param);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if=%d attr_cnt=%d, last=%d",
        add_param->server_if, add_param->attr_cnt, add_param->is_last);
    if (0 > (index = linuxbt_gatts_find_server_by_if(add_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            add_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (0 == add_param->attr_cnt)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"no attrs, attr_cnt: %d",
            add_param->attr_cnt);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    for (attr_index = 0; attr_index < add_param->attr_cnt; attr_index++)
    {
        btgatt_db_element_t el;
        memset(&el, 0, sizeof(el));
        bool is_valid = true;

        if ((GATT_ATTR_TYPE_PRIMARY_SERVICE ==
                add_param->attrs[attr_index].type)
            || (GATT_ATTR_TYPE_SECONDARY_SERVICE ==
                    add_param->attrs[attr_index].type))
        {
            BT_DBG_NORMAL(BT_DEBUG_GATT, "attr_index:%d add attr %s type %d",
                attr_index,
                add_param->attrs[attr_index].uuid,
                add_param->attrs[attr_index].type);
            s_linuxbt_gatts_cb.server_cb[index].service.clear();
        }
        //BT_DBG_NORMAL(BT_DEBUG_GATT, "add attr %s type %d, perm 0x%x, prop 0x%x",
        //    add_param->attrs[attr_index].uuid,
        //    add_param->attrs[attr_index].type,
        //    add_param->attrs[attr_index].permissions,
        //    add_param->attrs[attr_index].properties);

        if (GATT_ATTR_TYPE_INCLUDED_SERVICE != add_param->attrs[attr_index].type)
        {
            std::string uuid_str = add_param->attrs[attr_index].uuid;
            el.uuid = bluetooth::Uuid::FromString(uuid_str, &is_valid);
        }
        else
        {
            el.attribute_handle = add_param->attrs[attr_index].handle;
        }
        el.type = (bt_gatt_db_attribute_type_t)add_param->attrs[attr_index].type;
        el.properties = add_param->attrs[attr_index].properties;
        if (add_param->attrs[attr_index].key_size)
        {
            el.permissions = add_param->attrs[attr_index].permissions |
                ((add_param->attrs[attr_index].key_size - 7) << 12);
        }
        else
        {
            el.permissions = add_param->attrs[attr_index].permissions;
        }

        //BT_DBG_NORMAL(BT_DEBUG_GATT, "add attr %s e1 type %d, perm 0x%x, prop 0x%x",
        //    add_param->attrs[attr_index].uuid,
        //    el.type, el.permissions, el.properties);

        if (true == is_valid)
        {
            s_linuxbt_gatts_cb.server_cb[index].service.push_back(el);
            //BT_DBG_NORMAL(BT_DEBUG_GATT,"service size:%d uuid %s",
            //    s_linuxbt_gatts_cb.server_cb[index].service.size(),
            //    el.uuid.ToString().c_str());
        }
        else
        {
            BT_DBG_ERROR(BT_DEBUG_GATT,"invalid uuid %s",
                add_param->attrs[attr_index].uuid);
            return BT_ERR_STATUS_PARM_INVALID;
        }
    }

    if (add_param->is_last)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT,"service size:%d",
            s_linuxbt_gatts_cb.server_cb[index].service.size());
        ret = linuxbt_gatts_interface->add_service(add_param->server_if,
            std::move(s_linuxbt_gatts_cb.server_cb[index].service));
        return linuxbt_return_value_convert(ret);
    }
    else
    {
        return BT_SUCCESS;
    }
}


int linuxbt_gatts_delete_service(BT_GATTS_SERVICE_DEL_PARAM *del_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, del_param);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if= %d", del_param->server_if);
    if (0 > (index = linuxbt_gatts_find_server_by_if(del_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            del_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = linuxbt_gatts_interface->delete_service(del_param->server_if,
        del_param->service_handle);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_send_indication(BT_GATTS_IND_PARAM *ind_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, ind_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(ind_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if= %d", ind_param->server_if);
    if (0 > (index = linuxbt_gatts_find_server_by_if(ind_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            ind_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(ind_param->addr, bd_addr);
    int conn_id = 0;
    std::vector<uint8_t> value(ind_param->value, ind_param->value+ind_param->value_len);

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"no connection: %d - %s",
            ind_param->server_if, ind_param->addr);
        return BT_ERR_STATUS_FAIL;
    }
    else
    {
        conn_id = it->conn_id;
    }

    ret = linuxbt_gatts_interface->send_indication(ind_param->server_if,
            ind_param->char_handle, conn_id, ind_param->need_confirm, value);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_send_response(BT_GATTS_RESPONSE_PARAM *resp_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, resp_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(resp_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if= %d", resp_param->server_if);
    if (0 > (index = linuxbt_gatts_find_server_by_if(resp_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            resp_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(resp_param->addr, bd_addr);
    int conn_id = 0;

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {

        conn_id = 0;
    }
    else
    {
        conn_id = it->conn_id;
    }
    btgatt_response_t response;
    memset((void*)&response, 0, sizeof(response));
    response.attr_value.offset = resp_param->offset;
    response.attr_value.handle = resp_param->handle;
    response.attr_value.auth_req = 0;
    response.attr_value.len = (resp_param->len > BT_GATT_MAX_VALUE_LEN)
        ? BT_GATT_MAX_VALUE_LEN : resp_param->len;
    if (0 != response.attr_value.len)
    {
        memcpy(response.attr_value.value, resp_param->value,
            response.attr_value.len);
    }

    ret = linuxbt_gatts_interface->send_response(conn_id,
        resp_param->trans_id, resp_param->status, response);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_read_phy(BT_GATTS_PHY_READ_PARAM *phy_read_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, phy_read_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(phy_read_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if= %d", phy_read_param->server_if);
    if (0 > (index = linuxbt_gatts_find_server_by_if(phy_read_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            phy_read_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    //RawAddress::FromString(phy_read_param->addr, bd_addr);

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %s",
            phy_read_param->server_if, phy_read_param->addr);
        return BT_ERR_STATUS_FAIL;
    }

    ret = linuxbt_gatts_interface->read_phy(bd_addr,
        base::Bind(&linuxbt_gatts_read_phy_callback,
        phy_read_param->server_if, bd_addr));
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_set_prefer_phy(BT_GATTS_PHY_SET_PARAM *phy_set_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    int index = -1;
    RawAddress bd_addr;
    LINUXBT_GATTS_CHECK_INITED(BT_ERR_STATUS_NOT_READY);
    BT_CHECK_POINTER(BT_DEBUG_GATT, phy_set_param);
    LINUXBT_CHECK_AND_CONVERT_ADDR(phy_set_param->addr, bd_addr);
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if= %d", phy_set_param->server_if);
    if (0 > (index = linuxbt_gatts_find_server_by_if(phy_set_param->server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            phy_set_param->server_if);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (phy_set_param->phy_options > GATT_PHY_OPT_CODED_S8)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"phy_options: 0x%x invalid",
            phy_set_param->phy_options);
        return BT_ERR_STATUS_PARM_INVALID;
    }

    phy_set_param->tx_phy &= 0x07;
    phy_set_param->rx_phy &= 0x07;

    //RawAddress::FromString(phy_set_param->addr, bd_addr);

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bd_addr);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %s",
            phy_set_param->server_if, phy_set_param->addr);
        return BT_ERR_STATUS_FAIL;
    }

    ret = linuxbt_gatts_interface->set_preferred_phy(bd_addr,
        phy_set_param->tx_phy, phy_set_param->rx_phy, phy_set_param->phy_options);
    return linuxbt_return_value_convert(ret);
}



/* LOCAL SUBPROGRAM BODIES
 */

static void linuxbt_gatts_delete_services(LINUXBT_GATTS_SERVER_CB *server_cb)
{
    BT_CHECK_POINTER_RETURN(BT_DEBUG_GATT, server_cb);
    auto it = server_cb->srvc_list.begin();

    for (; it != server_cb->srvc_list.end(); it++)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT, "service_handle=%d", *it);
        linuxbt_gatts_interface->delete_service(server_cb->server_if, *it);
    }

    return;
}

static void linuxbt_gatts_register_server_callback(int status, int server_if,
                                         const bluetooth::Uuid& app_uuid)
{
    BT_GATTS_EVENT_PARAM gatts_msg;
    BT_DBG_NORMAL(BT_DEBUG_GATT,"server_if:%d, status:%d",
                 server_if, status);

    int index = linuxbt_gatts_find_server_by_uuid(app_uuid);
    if (0 > index)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "find server cb fail, uuid=%s, unregister",
            app_uuid.ToString().c_str());

        linuxbt_gatts_interface->unregister_server(server_if);
        return;
    }

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_REGISTER;
    gatts_msg.server_if = server_if;
    gatts_msg.data.register_data.status = (BT_GATT_STATUS)status;
    memcpy(gatts_msg.data.register_data.server_name,
        s_linuxbt_gatts_cb.server_cb[index].server_name,
        sizeof(s_linuxbt_gatts_cb.server_cb[index].server_name));

    if (BT_STATUS_SUCCESS == status)
    {
        s_linuxbt_gatts_cb.server_cb[index].server_if = server_if;
    }
    else if (BT_GATT_STATUS_DUP_REG == status)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT, "duplicate register %s",
            s_linuxbt_gatts_cb.server_cb[index].server_name);
    }
    else
    {
        linuxbt_gatts_free_server_by_uuid(app_uuid);
    }

    bt_mw_gatts_notify_app(&gatts_msg);
}

static void linuxbt_gatts_connection_callback(int conn_id, int server_if,
                                                        int connected, const RawAddress& bda)
{
    int index = -1;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d",
            server_if);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT, " %s %s", bda.ToString().c_str(),
        connected==0?"disconnected":"connected");

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bda);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        if (connected)
        {
            LINUXBT_GATTS_DEV_CB &dev =
                *(s_linuxbt_gatts_cb.server_cb[index].dev_list.emplace(it));

            dev.addr = bda;
            dev.connected = connected;
            dev.conn_id = conn_id;
        }
    }
    else
    {
        if (0 == connected)
        {
            s_linuxbt_gatts_cb.server_cb[index].dev_list.erase(it);
        }
    }

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_CONNECTION;
    gatts_msg.server_if = server_if;
    gatts_msg.data.connection_data.connected = connected;
    strncpy(gatts_msg.data.connection_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.connection_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_service_added_callback(
    int status, int server_if, std::vector<btgatt_db_element_t> service)
{
    int index = -1;
    BT_GATTS_EVENT_PARAM gatts_msg;
    UINT32 attr_index = 0;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, cnt=%d, status=%d, gatts_msg size=%d",
        server_if, service.size(), status, sizeof(gatts_msg));

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_SERVICE_ADD;
    gatts_msg.server_if = server_if;
    strncpy(gatts_msg.data.add_service_data.service_uuid,
        service[0].uuid.ToString().c_str(),
        BT_GATT_MAX_UUID_LEN - 1);
    gatts_msg.data.add_service_data.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

    gatts_msg.data.add_service_data.status = status;
    if (status == BT_GATT_STATUS_OK)
    {
        s_linuxbt_gatts_cb.server_cb[index].srvc_list.push_back(service[0].attribute_handle);
        do
        {
            UINT32 max_cnt = ((service.size() - attr_index) > BT_GATT_MAX_ATTR_CNT)
                            ? BT_GATT_MAX_ATTR_CNT : service.size() - attr_index;
            UINT32 i = 0;
            for (i = 0;i < max_cnt;i++)
            {
                strncpy(gatts_msg.data.add_service_data.attrs[i].uuid,
                    service[attr_index + i].uuid.ToString().c_str(),
                    BT_GATT_MAX_UUID_LEN - 1);
                gatts_msg.data.add_service_data.attrs[i].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';

                gatts_msg.data.add_service_data.attrs[i].handle
                    = service[attr_index + i].attribute_handle;

                gatts_msg.data.add_service_data.attrs[i].type
                    = (GATT_ATTR_TYPE)service[attr_index + i].type;

                //BT_DBG_NORMAL(BT_DEBUG_GATT, "type = %d, uuid=%s, handle=%d",
                //    gatts_msg.data.add_service_data.attrs[i].type,
                //    gatts_msg.data.add_service_data.attrs[i].uuid,
                //    gatts_msg.data.add_service_data.attrs[i].handle);
            }
            gatts_msg.data.add_service_data.attr_cnt = max_cnt;

            //BT_DBG_NORMAL(BT_DEBUG_GATT, "attr_cnt: %d, gatts_msg size=%d",
            //    max_cnt, sizeof(gatts_msg));
            bt_mw_gatts_notify_app(&gatts_msg);

            attr_index += max_cnt;
        }while(attr_index < service.size());
    }
    else
    {
        bt_mw_gatts_notify_app(&gatts_msg);
    }

    return;
}

static void linuxbt_gatts_service_stopped_callback(int status, int server_if,
                                         int srvc_handle)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT,"service stopped handle = %d, status = %d",
                srvc_handle, status);
    return;
}

static void linuxbt_gatts_service_deleted_callback(int status, int server_if,
                                         int srvc_handle)
{
    int index = -1;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_GATT,"server_if=%d handle = %d, status = %d",
                server_if, srvc_handle, status);

    s_linuxbt_gatts_cb.server_cb[index].srvc_list.remove(srvc_handle);

    return;
}

static void linuxbt_gatts_request_read_callback(int conn_id, int trans_id,
                                      const RawAddress& bda, int attr_handle,
                                      int offset, bool is_long)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, trans_id=%d,"
        " attr_handle=%d, offset=%d, is_long=%d",
        server_if, it->addr.ToString().c_str(), trans_id, attr_handle,
        offset, is_long);

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_READ_REQ;
    gatts_msg.server_if = server_if;
    gatts_msg.data.read_req_data.trans_id = trans_id;
    gatts_msg.data.read_req_data.handle = attr_handle;
    gatts_msg.data.read_req_data.offset = offset;
    strncpy(gatts_msg.data.read_req_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.read_req_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_request_write_callback(int conn_id, int trans_id,
                                       const RawAddress& bda, int attr_handle,
                                       int offset, bool need_rsp, bool is_prep,
                                       std::vector<uint8_t> value)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, trans_id=%d,"
        " attr_handle=%d, offset=%d, need_rsp=%d, is_prep=%d, len=%d",
        server_if, it->addr.ToString().c_str(), trans_id, attr_handle,
        offset, need_rsp, is_prep, value.size());

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_WRITE_REQ;
    gatts_msg.server_if = server_if;
    gatts_msg.data.write_req_data.trans_id = trans_id;
    gatts_msg.data.write_req_data.handle = attr_handle;
    gatts_msg.data.write_req_data.offset = offset;
    gatts_msg.data.write_req_data.need_rsp = need_rsp;
    gatts_msg.data.write_req_data.is_prep = is_prep;
    strncpy(gatts_msg.data.write_req_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.write_req_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gatts_msg.data.write_req_data.is_prep = is_prep;
    if (value.size() > BT_GATT_MAX_VALUE_LEN)
    {
        gatts_msg.data.write_req_data.value_len = BT_GATT_MAX_VALUE_LEN;
    }
    else
    {
        gatts_msg.data.write_req_data.value_len = value.size();
    }

    memcpy((void*)&gatts_msg.data.write_req_data.value, value.data(),
        gatts_msg.data.write_req_data.value_len);

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_request_exec_write_callback(int conn_id, int trans_id,
                                            const RawAddress& bda,
                                            int exec_write)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, trans_id=%d, exec_write=%d",
        server_if, it->addr.ToString().c_str(), trans_id, exec_write);

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_WRITE_EXE_REQ;
    gatts_msg.server_if = server_if;
    gatts_msg.data.write_exe_req_data.trans_id = trans_id;
    gatts_msg.data.write_exe_req_data.exec_write = exec_write;
    strncpy(gatts_msg.data.write_exe_req_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.write_exe_req_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_response_confirmation_callback(int status,
                                                                       int handle)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "handle = %d,  status=%d", handle, status);

    return;
}

static void linuxbt_gatts_indication_sent_callback(int conn_id, int status)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, status=%d",
        server_if, it->addr.ToString().c_str(), status);

    gatts_msg.server_if = server_if;
    gatts_msg.event = BT_GATTS_EVENT_IND_SENT;
    strncpy(gatts_msg.data.ind_sent_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.ind_sent_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gatts_msg.data.ind_sent_data.status = status;

    if (s_linuxbt_gatts_cb.server_cb[index].congested)
    {
        s_linuxbt_gatts_cb.server_cb[index].msg_list.push_back(gatts_msg);
    }
    else
    {
        bt_mw_gatts_notify_app(&gatts_msg);
    }

    return;
}

static void linuxbt_gatts_mtu_changed_callback(int conn_id, int mtu)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, mtu=%d",
        server_if, it->addr.ToString().c_str(), mtu);

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_MTU_CHANGE;
    gatts_msg.server_if = server_if;
    gatts_msg.data.mtu_chg_data.mtu = mtu;
    strncpy(gatts_msg.data.mtu_chg_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.mtu_chg_data.addr[MAX_BDADDR_LEN - 1] = '\0';

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_congestion_callback(int conn_id, bool congested)
{
    int index = -1;
    int server_if = 0;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, congested=%d",
        server_if, it->addr.ToString().c_str(), congested);

    s_linuxbt_gatts_cb.server_cb[index].congested = congested;

    if (false == congested && !s_linuxbt_gatts_cb.server_cb[index].msg_list.empty())
    {
        auto msg_it = s_linuxbt_gatts_cb.server_cb[index].msg_list.begin();
        auto end_it = s_linuxbt_gatts_cb.server_cb[index].msg_list.end();
        for (; msg_it != end_it; msg_it++) {
          bt_mw_gatts_notify_app(&(*msg_it));
        }
        s_linuxbt_gatts_cb.server_cb[index].msg_list.clear();
    }

    return;
}

static void linuxbt_gatts_phy_update_callback(int conn_id, uint8_t tx_phy,
                                     uint8_t rx_phy, uint8_t status)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, tx_phy= 0x%x, rx_phy=0x%x, status=%d",
        server_if, it->addr.ToString().c_str(), tx_phy, rx_phy, status);

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_PHY_UPDATE;
    gatts_msg.server_if = server_if;
    gatts_msg.data.phy_upd_data.tx_phy = tx_phy;
    gatts_msg.data.phy_upd_data.rx_phy = rx_phy;
    strncpy(gatts_msg.data.phy_upd_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.phy_upd_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gatts_msg.data.phy_upd_data.status = status;

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_conn_update_callback(int conn_id, uint16_t interval,
                                      uint16_t latency, uint16_t timeout,
                                      uint8_t status)
{
    int index = -1;
    int server_if = 0;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    server_if = conn_id & 0xFF;

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_conn_id(&s_linuxbt_gatts_cb.server_cb[index].dev_list, conn_id);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %d ",
            server_if, conn_id);
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GATT, "serverIf = %d, bda=%s, interval= %u, latency=%u, timeout=%u, status=%d",
        server_if, it->addr.ToString().c_str(), interval, latency, timeout, status);

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_CONN_UPDATE;
    gatts_msg.server_if = server_if;
    gatts_msg.data.conn_upd_data.interval = interval;
    gatts_msg.data.conn_upd_data.latency = latency;
    gatts_msg.data.conn_upd_data.timeout = timeout;
    strncpy(gatts_msg.data.conn_upd_data.addr, it->addr.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.conn_upd_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gatts_msg.data.conn_upd_data.status = status;

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}

static void linuxbt_gatts_read_phy_callback(uint8_t server_if, RawAddress bda, uint8_t tx_phy,
                            uint8_t rx_phy, uint8_t status) {
    int index = -1;
    BT_GATTS_EVENT_PARAM gatts_msg;
    LINUXBT_GATTS_CHECK_INITED();
    LockGuard lock(s_linuxbt_gatts_mutex);

    BT_DBG_NORMAL(BT_DEBUG_GATT, "server_if = %d, bda=%s, tx_phy= 0x%x, rx_phy=0x%x, status=%d",
        server_if, bda.ToString().c_str(), tx_phy, rx_phy, status);

    if (0 > (index = linuxbt_gatts_find_server_by_if(server_if)))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"cannot find server_if: %d", server_if);
        return;
    }

    auto it = linuxbt_gatts_find_dev_by_addr(&s_linuxbt_gatts_cb.server_cb[index].dev_list, bda);
    if (it == s_linuxbt_gatts_cb.server_cb[index].dev_list.end())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"server_if=%d no connection: %s",
            server_if, bda.ToString().c_str());
        return;
    }

    memset((void*)&gatts_msg, 0, sizeof(gatts_msg));
    gatts_msg.event = BT_GATTS_EVENT_PHY_READ;
    gatts_msg.server_if = server_if;
    gatts_msg.data.read_phy_data.rx_phy = rx_phy;
    gatts_msg.data.read_phy_data.tx_phy = tx_phy;
    strncpy(gatts_msg.data.read_phy_data.addr, bda.ToString().c_str(),
        MAX_BDADDR_LEN - 1);
    gatts_msg.data.read_phy_data.addr[MAX_BDADDR_LEN - 1] = '\0';
    gatts_msg.data.read_phy_data.status = status;

    bt_mw_gatts_notify_app(&gatts_msg);

    return;
}


static int linuxbt_gatts_alloc_server(CHAR *server_name,
    const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if (s_linuxbt_gatts_cb.server_cb[i].in_use == 0)
        {
            s_linuxbt_gatts_cb.server_cb[i].in_use = 1;
            s_linuxbt_gatts_cb.server_cb[i].uuid = app_uuid;
            s_linuxbt_gatts_cb.server_cb[i].congested = 0;
            strncpy(s_linuxbt_gatts_cb.server_cb[i].server_name,
                server_name, BT_GATT_MAX_NAME_LEN - 1);
            s_linuxbt_gatts_cb.server_cb[i].server_name[BT_GATT_MAX_NAME_LEN-1] = '\0';
            return i;
        }
    }

    return -1;
}

static int linuxbt_gatts_find_server_by_uuid(const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gatts_cb.server_cb[i].in_use != 0)
            && (s_linuxbt_gatts_cb.server_cb[i].uuid == app_uuid))
        {
            return i;
        }
    }
    return -1;
}

static int linuxbt_gatts_find_server_by_name(const char *server_name)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gatts_cb.server_cb[i].in_use != 0)
            && (0 == strncasecmp(s_linuxbt_gatts_cb.server_cb[i].server_name,
                                server_name, BT_GATT_MAX_NAME_LEN - 1)))
        {
            return i;
        }
    }
    return -1;
}


static int linuxbt_gatts_find_server_by_if(const int server_if)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gatts_cb.server_cb[i].in_use != 0)
            && (s_linuxbt_gatts_cb.server_cb[i].server_if == server_if))
        {
            return i;
        }
    }
    return -1;
}


static int linuxbt_gatts_free_server_by_if(const int server_if)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gatts_cb.server_cb[i].in_use != 0)
            && (s_linuxbt_gatts_cb.server_cb[i].server_if == server_if))
        {
            s_linuxbt_gatts_cb.server_cb[i].service.clear();
            s_linuxbt_gatts_cb.server_cb[i].dev_list.clear();
            s_linuxbt_gatts_cb.server_cb[i].srvc_list.clear();
            s_linuxbt_gatts_cb.server_cb[i].msg_list.clear();

            s_linuxbt_gatts_cb.server_cb[i].in_use = 0;
            s_linuxbt_gatts_cb.server_cb[i].server_if = 0;
            s_linuxbt_gatts_cb.server_cb[i].server_name[0] = 0;
            s_linuxbt_gatts_cb.server_cb[i].uuid = bluetooth::Uuid::kEmpty;
            s_linuxbt_gatts_cb.server_cb[i].congested = 0;

            BT_DBG_NORMAL(BT_DEBUG_GATT,"free %d", server_if);
            return i;
        }
    }

    return -1;
}

static void linuxbt_gatts_free_server_by_uuid(const bluetooth::Uuid &app_uuid)
{
    int i = 0;

    for (i = 0; i < BT_GATT_MAX_APP_CNT; i++)
    {
        if ((s_linuxbt_gatts_cb.server_cb[i].in_use != 0)
            && (s_linuxbt_gatts_cb.server_cb[i].uuid == app_uuid))
        {
            s_linuxbt_gatts_cb.server_cb[i].service.clear();
            s_linuxbt_gatts_cb.server_cb[i].dev_list.clear();
            s_linuxbt_gatts_cb.server_cb[i].srvc_list.clear();
            s_linuxbt_gatts_cb.server_cb[i].msg_list.clear();

            s_linuxbt_gatts_cb.server_cb[i].in_use = 0;
            s_linuxbt_gatts_cb.server_cb[i].server_if = 0;
            s_linuxbt_gatts_cb.server_cb[i].server_name[0] = 0;
            s_linuxbt_gatts_cb.server_cb[i].uuid = bluetooth::Uuid::kEmpty;
            s_linuxbt_gatts_cb.server_cb[i].congested = 0;
            BT_DBG_NORMAL(BT_DEBUG_GATT,"free uuid-%s", app_uuid.ToString().c_str());
            return;
        }
    }
    return;
}


static std::list<LINUXBT_GATTS_DEV_CB>::iterator linuxbt_gatts_find_dev_by_addr(std::list<LINUXBT_GATTS_DEV_CB> *dev_list, const RawAddress &addr)
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


static std::list<LINUXBT_GATTS_DEV_CB>::iterator linuxbt_gatts_find_dev_by_conn_id(std::list<LINUXBT_GATTS_DEV_CB> *dev_list, const int conn_id)
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

