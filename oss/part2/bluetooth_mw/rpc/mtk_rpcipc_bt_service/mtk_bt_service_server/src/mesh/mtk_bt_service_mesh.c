/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/*-----------------------------------------------------------------------------
                            include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "mtk_bt_service_mesh.h"
#include "c_bt_mw_mesh.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[setup.c]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

void *g_mesh_bt_evt_pvtag = NULL;

mtkrpcapi_BtAppMeshBtEventCbk mtkrpcapi_BtMeshBtEventCbk = NULL;
BtAppMeshAccessMsgCbk mtkrpcapi_BtMeshAccessMsgCbk = NULL;
BtAppMeshHealthClientEvtCbk mtkrpcapi_BtMeshHealthClientEventCbk = NULL;

VOID MWBtAppMeshBtEventCbk(BT_MESH_BT_EVENT_T *event)
{
    if (NULL == event)
    {
        BT_RC_LOG(("MWBtAppMeshBtEventCbk: event data is null!\n"));
        return;
    }
    if (mtkrpcapi_BtMeshBtEventCbk)
    {
        mtkrpcapi_BtMeshBtEventCbk(event, g_mesh_bt_evt_pvtag);
    }
}
VOID MWBtAppMeshAccessMsgCbk(BT_MESH_CBK_ACCESS_MSG_T *msg)
{
    if (NULL == msg)
    {
        BT_RC_LOG(("MWBtAppMeshAccessMsgCbk: event data is null!\n"));
        return;
    }

    if (mtkrpcapi_BtMeshAccessMsgCbk)
    {
        mtkrpcapi_BtMeshAccessMsgCbk(msg);
    }
}
VOID MWBtAppMeshHealthClientEventCbk(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event)
{
    if (NULL == event)
    {
        BT_RC_LOG(("MWBtAppMeshHealthClientEventCbk: event data is null!\n"));
        return;
    }
    if (mtkrpcapi_BtMeshHealthClientEventCbk)
    {
        mtkrpcapi_BtMeshHealthClientEventCbk(event);
    }
}

/*register APP callback function for BT EVENT*/
INT32 mtkrpcapi_btm_mesh_register_bt_evt_cbk_fct(MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func,void *pv_tag)
{
    INT32 i4_ret = 0;

    g_mesh_bt_evt_pvtag = pv_tag;
    if(NULL == func)
    {
        BT_RC_LOG(("callback func is null!\n"));
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_mesh_bt_event_cb)
    {
        mtkrpcapi_BtMeshBtEventCbk = func->bt_mesh_bt_event_cb;
    }
    else
    {
        BT_RC_LOG(("event callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }
    return i4_ret;
}

/*These callback function registered here is a entry for APP callback functions*/
INT32 mtkrpcapi_btm_mesh_register_internal_cbk_entry(MTKRPCAPI_BT_MESH_CB_FUNC_T *func)
{
    INT32 i4_ret = 0;

    if(NULL == func)
    {
        BT_RC_LOG(("register_access_msg_cbk_fct callback func is null!\n"));
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_mesh_access_msg_cb)
    {
        mtkrpcapi_BtMeshAccessMsgCbk = func->bt_mesh_access_msg_cb;
    }
    else
    {
        BT_RC_LOG(("mtkrpcapi_BtMeshAccessMsgCbk is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_mesh_health_client_evt_cb)
    {
        mtkrpcapi_BtMeshHealthClientEventCbk = func->bt_mesh_health_client_evt_cb;
    }
    else
    {
        BT_RC_LOG(("mtkrpcapi_BtMeshHealthClientEventCbk is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    return i4_ret;
}

INT32 mtkrpcapi_bt_mesh_init(MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func)
{
    BT_APP_MESH_CB_FUNC_T mesh_app_func;
    memset(&mesh_app_func, 0, sizeof(BT_APP_MESH_CB_FUNC_T));

    mesh_app_func.bt_mesh_bt_event_cb = MWBtAppMeshBtEventCbk;
    mesh_app_func.bt_mesh_acces_msg_cb = MWBtAppMeshAccessMsgCbk;
    mesh_app_func.bt_mesh_health_client_evt_cb = MWBtAppMeshHealthClientEventCbk;

    //register mw entry callback and then init
    return c_btm_bt_mesh_init(&mesh_app_func);
}

INT32 x_mtkapi_bt_mesh_init  (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func, void *pv_tag)
{
    INT32 i4_ret = 0;
    i4_ret = mtkrpcapi_btm_mesh_register_bt_evt_cbk_fct(func, pv_tag);
    if(0 != i4_ret)
    {
        BT_RC_LOG(("mtkrpcapi_btm_mesh_register_bt_evt_cbk_fct Error.\n"));
        return i4_ret;
    }

    return mtkrpcapi_bt_mesh_init(func);
}

INT32 x_mtkapi_bt_mesh_deinit (VOID)
{
    return c_btm_bt_mesh_deinit();
}

VOID x_mtkapi_bt_mesh_light_model_get_element_index(UINT32 model_id, UINT16 *element_idx)
{
    c_btm_bt_mesh_light_model_get_element_index(model_id, element_idx);
}

UINT16 x_mtkapi_bt_mesh_get_model_handle_by_elementIdx_and_modeId(UINT32 model_id, UINT16 element_idx)
{
    UINT16 model_handle = 0xFFFF;

    model_handle = c_btm_bt_mesh_get_model_handle_by_elementIdx_and_modeId(model_id, element_idx);

    return model_handle;
}

INT32 x_mtkapi_bt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md)
{
    return c_btm_bt_mesh_set_model_data(md);
}

INT32 x_mtkapi_bt_mesh_enable (const BT_MESH_INIT_PARAMS_T *init_params)
{
    return c_btm_bt_mesh_enable(init_params);
}

INT32 x_mtkapi_bt_mesh_disable (VOID)
{
    return c_btm_bt_mesh_disable();
}

INT32 x_mtkapi_bt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey)
{
    return c_btm_bt_mesh_set_netkey(netkey);
}

INT32 x_mtkapi_bt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey)
{
    return c_btm_bt_mesh_set_appkey(appkey);
}

VOID x_mtkapi_bt_mesh_unprov_dev_scan(BOOL start, UINT32 duration)
{
    return c_btm_bt_mesh_unprov_dev_scan(start, duration);
}

INT32 x_mtkapi_bt_mesh_invite_provisioning(const UINT8 *target_uuid, const BT_MESH_PROV_INVITE_T *invite)
{
    return c_btm_bt_mesh_invite_provisioning(target_uuid, invite);
}

INT32 x_mtkapi_bt_mesh_start_provisioning(const BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode)
{
    return c_btm_bt_mesh_start_provisioning(params, mode);
}

INT32 x_mtkapi_bt_mesh_set_prov_factor(const BT_MESH_PROV_FACTOR_T *factor)
{
    return c_btm_bt_mesh_set_prov_factor(factor);
}

INT32 x_mtkapi_bt_mesh_model_cc_msg_tx(const BT_MESH_CONFIGURATION_MSG_TX_T *msg)
{
    return c_btm_bt_mesh_model_cc_msg_tx(msg);
}

INT32 x_mtkapi_bt_mesh_send_packet(const BT_MESH_TX_PARAMS_T *params)
{
    return c_btm_bt_mesh_send_packet(params);
}

INT32 x_mtkapi_bt_mesh_send_packet_ex(const BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op)
{
    return c_btm_bt_mesh_send_packet_ex(params, model_op);
}

INT32 x_mtkapi_bt_mesh_data_reset(UINT32 record)
{
    return c_btm_bt_mesh_data_reset(record);
}

INT32 x_mtkapi_bt_mesh_data_save(VOID)
{
    return c_btm_bt_mesh_data_save();
}

INT32 x_mtkapi_bt_mesh_data_set(BT_MESH_RECORD_T *mesh_data)
{
    return c_btm_bt_mesh_data_set(mesh_data);
}

INT32 x_mtkapi_bt_mesh_version(CHAR *buf)
{
    return c_btm_bt_mesh_version(buf);
}

VOID x_mtkapi_bt_mesh_dump(BT_MESH_DUMP_TYPE_T type)
{
    c_btm_bt_mesh_dump(type);
}

UINT16 x_mtkapi_bt_mesh_get_element_address(UINT16 element_index)
{
    return c_btm_bt_mesh_get_element_address(element_index);
}

VOID x_mtkapi_bt_mesh_set_default_ttl(UINT8 def_ttl)
{
    c_btm_bt_mesh_set_default_ttl(def_ttl);
}

UINT8 x_mtkapi_bt_mesh_get_default_ttl(VOID)
{
    return c_btm_bt_mesh_get_default_ttl();
}

INT32 x_mtkapi_bt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index)
{
    return c_btm_bt_mesh_model_app_bind(model_handle, appkey_index);
}

INT32 x_mtkapi_bt_mesh_access_model_reply(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_ACCESS_MESSAGE_TX_T *reply)
{
    return c_btm_bt_mesh_access_model_reply(model_handle, msg, reply);
}

INT32 x_mtkapi_bt_mesh_bearer_adv_set_params(const BT_MESH_BEARER_ADV_PARAMS_T *adv_params, const BT_MESH_BEARER_SCAN_PARAMS_T *scan_params)
{
    return c_btm_bt_mesh_bearer_adv_set_params(adv_params, scan_params);
}

VOID x_mtkapi_bt_mesh_switch_adv(BOOL enable)
{
    c_btm_bt_mesh_switch_adv(enable);
}

VOID x_mtkapi_bt_mesh_log_setlevel(UINT32 level)
{
    c_btm_bt_mesh_log_setlevel(level);
}

INT32 x_mtkapi_bt_mesh_generate_uuid(UINT8 *uuid_buf)
{
    return c_btm_bt_mesh_generate_uuid(uuid_buf);
}

INT32 x_mtkapi_bt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info)
{
    return c_btm_bt_mesh_set_device_info(info);
}

INT32 x_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_T mode)
{
    return c_btm_bt_mesh_set_mesh_mode(mode);
}

INT32 x_mtkapi_bt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout)
{
    return c_btm_bt_mesh_set_heartbeat_period(num, hb_timeout);
}

INT32 x_mtkapi_bt_mesh_ota_initiator_operation(BT_MESH_OTA_OPERATION_PARAMS_T *params)
{
    return c_btm_bt_mesh_ota_initiator_operation(params);
}

INT32 x_mtkapi_bt_mesh_ota_get_client_model_handle(UINT16 *dist_client, UINT16 *update_client)
{
    return c_btm_bt_mesh_ota_get_client_model_handle(dist_client, update_client);
}

INT32 x_mtkapi_bt_mesh_set_special_pkt_params(BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params)
{
    return c_btm_bt_mesh_set_special_pkt_params(pkt_params);
}

#ifdef MTK_GATT_BEARER_SUPPORT
INT32 x_mtkapi_bt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type)
{
    return c_btm_bt_mesh_gatt_connect(addr, type);
}

INT32 x_mtkapi_bt_mesh_gatt_disconnect(VOID)
{
    return c_btm_bt_mesh_gatt_disconnect();
}
#endif
