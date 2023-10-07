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
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_BT_SERVICE_MESH_WRAPPER_H_
#define _MTK_BT_SERVICE_MESH_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_mesh.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppMeshBtEventCbk)(BT_MESH_BT_EVENT_T *event, VOID *pv_tag);

//for APP use, the callback function is set by APP
typedef struct
{
    mtkrpcapi_BtAppMeshBtEventCbk bt_mesh_bt_event_cb;
    //...
}MTKRPCAPI_BT_APP_MESH_CB_FUNC_T;

//For MESH RPC use, The callback function is not set by APP
typedef struct
{
    BtAppMeshAccessMsgCbk bt_mesh_access_msg_cb;
    BtAppMeshHealthClientEvtCbk bt_mesh_health_client_evt_cb;
    //...
}MTKRPCAPI_BT_MESH_CB_FUNC_T;

extern INT32 a_mtkapi_bt_mesh_init (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func, VOID *pv_tag);
extern INT32 a_mtkapi_bt_mesh_deinit (VOID);
extern INT32 a_mtkapi_bt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md);
extern INT32 a_mtkapi_bt_mesh_enable (const BT_MESH_INIT_PARAMS_T *init_params);
extern INT32 a_mtkapi_bt_mesh_disable (VOID);
extern INT32 a_mtkapi_bt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey);
extern INT32 a_mtkapi_bt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey);
extern VOID a_mtkapi_bt_mesh_unprov_dev_scan(BOOL start, UINT32 duration);
extern INT32 a_mtkapi_bt_mesh_invite_provisioning(const UINT8 *target_uuid, const BT_MESH_PROV_INVITE_T *invite);
extern INT32 a_mtkapi_bt_mesh_start_provisioning(const BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode);
extern INT32 a_mtkapi_bt_mesh_set_prov_factor(const BT_MESH_PROV_FACTOR_T *factor);
extern INT32 a_mtkapi_bt_mesh_model_cc_msg_tx(const BT_MESH_CONFIGURATION_MSG_TX_T *msg);
extern INT32 a_mtkapi_bt_mesh_send_packet(const BT_MESH_TX_PARAMS_T *params);
extern INT32 a_mtkapi_bt_mesh_send_packet_ex(const BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op);
extern INT32 a_mtkapi_bt_mesh_data_reset(UINT32 record);
extern INT32 a_mtkapi_bt_mesh_data_save(VOID);
extern INT32 a_mtkapi_bt_mesh_data_set(BT_MESH_RECORD_T *mesh_data);
extern INT32 a_mtkapi_bt_mesh_version(CHAR *buf);
extern INT32 a_mtkapi_bt_mesh_dump(BT_MESH_DUMP_TYPE_T type);
extern UINT16 a_mtkapi_bt_mesh_get_element_address(UINT16 element_index);
extern INT32 a_mtkapi_bt_mesh_set_default_ttl(UINT8 def_ttl);
extern UINT8 a_mtkapi_bt_mesh_get_default_ttl(VOID);
extern INT32 a_mtkapi_bt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index);
extern INT32 a_mtkapi_bt_mesh_access_model_reply(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_ACCESS_MESSAGE_TX_T *reply);
extern INT32 a_mtkapi_bt_mesh_bearer_adv_set_params(const BT_MESH_BEARER_ADV_PARAMS_T *adv_params, const BT_MESH_BEARER_SCAN_PARAMS_T *scan_params);
extern INT32 a_mtkapi_bt_mesh_switch_adv(BOOL enable);
extern VOID a_mtkapi_bt_mesh_log_setlevel(UINT32 level);
extern INT32 a_mtkapi_bt_mesh_generate_uuid(UINT8 *uuid_buf);
extern INT32 a_mtkapi_bt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info);
extern INT32 a_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_T mode);
extern INT32 a_mtkapi_bt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout);
extern INT32 a_mtkapi_bt_mesh_ota_initiator_operation(const BT_MESH_OTA_OPERATION_PARAMS_T *params);
extern INT32 a_mtkapi_bt_mesh_set_special_pkt_params(const BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params);

#ifdef MTK_GATT_BEARER_SUPPORT
extern INT32 a_mtkapi_bt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type);
extern INT32 a_mtkapi_bt_mesh_gatt_disconnect(VOID);
#endif


extern INT32 c_rpc_reg_mtk_bt_service_mesh_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
