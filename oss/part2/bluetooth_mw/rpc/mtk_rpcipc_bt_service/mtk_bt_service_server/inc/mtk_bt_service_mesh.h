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

#ifndef _MTK_BT_SERVICE_MESH_H_
#define _MTK_BT_SERVICE_MESH_H_

#include "mtk_bt_service_mesh_wrapper.h"

typedef struct {
    UINT16 model_handle;     //the model handle for this entry
    UINT8 cb_cnt;        //The callback count which share the same pv_tag, at most 25. special value 0 means that the model register only one handler for all operations.
    UINT16 opcode;           //opcode for the message with handler of pv_cb
    UINT16 company_id;       //company id for this operation
    UINT8 cb_index;         //the callback index in the apv_cb_addr_ex array in pvtag
    void *pv_cb;             //The address of the callback
    void *pv_tag;           //pv_tag which contains the pv_cb
}BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T;

INT32 mtkrpcapi_bt_mesh_init(MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func);
INT32 mtkrpcapi_btm_mesh_register_internal_cbk_entry(MTKRPCAPI_BT_MESH_CB_FUNC_T *func);

INT32 x_mtkapi_bt_mesh_init  (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func, void *pv_tag);
INT32 x_mtkapi_bt_mesh_deinit (VOID);
VOID x_mtkapi_bt_mesh_light_model_get_element_index(UINT32 model_id, UINT16 *element_idx);
UINT16 x_mtkapi_bt_mesh_get_model_handle_by_elementIdx_and_modeId(UINT32 model_id, UINT16 element_idx);
INT32 x_mtkapi_bt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md);
INT32 x_mtkapi_bt_mesh_enable (const BT_MESH_INIT_PARAMS_T *init_params);
INT32 x_mtkapi_bt_mesh_disable (VOID);
INT32 x_mtkapi_bt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey);
INT32 x_mtkapi_bt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey);
VOID x_mtkapi_bt_mesh_unprov_dev_scan(BOOL start, UINT32 duration);
INT32 x_mtkapi_bt_mesh_invite_provisioning(const UINT8 *target_uuid, const BT_MESH_PROV_INVITE_T *invite);
INT32 x_mtkapi_bt_mesh_start_provisioning(const BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode);
INT32 x_mtkapi_bt_mesh_set_prov_factor(const BT_MESH_PROV_FACTOR_T *factor);
INT32 x_mtkapi_bt_mesh_model_cc_msg_tx(const BT_MESH_CONFIGURATION_MSG_TX_T *msg);
INT32 x_mtkapi_bt_mesh_send_packet(const BT_MESH_TX_PARAMS_T *params);
INT32 x_mtkapi_bt_mesh_send_packet_ex(const BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op);
INT32 x_mtkapi_bt_mesh_data_reset(UINT32 record);
INT32 x_mtkapi_bt_mesh_data_save(VOID);
INT32 x_mtkapi_bt_mesh_data_set(BT_MESH_RECORD_T *mesh_data);
INT32 x_mtkapi_bt_mesh_version(CHAR *buf);
VOID x_mtkapi_bt_mesh_dump(BT_MESH_DUMP_TYPE_T type);
UINT16 x_mtkapi_bt_mesh_get_element_address(UINT16 element_index);
VOID x_mtkapi_bt_mesh_set_default_ttl(UINT8 def_ttl);
UINT8 x_mtkapi_bt_mesh_get_default_ttl(VOID);
INT32 x_mtkapi_bt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index);
INT32 x_mtkapi_bt_mesh_access_model_reply(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_ACCESS_MESSAGE_TX_T *reply);
INT32 x_mtkapi_bt_mesh_bearer_adv_set_params(const BT_MESH_BEARER_ADV_PARAMS_T *adv_params, const BT_MESH_BEARER_SCAN_PARAMS_T *scan_params);
VOID x_mtkapi_bt_mesh_switch_adv(BOOL enable);
VOID x_mtkapi_bt_mesh_log_setlevel(UINT32 level);
INT32 x_mtkapi_bt_mesh_generate_uuid(UINT8 *uuid_buf);
INT32 x_mtkapi_bt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info);
INT32 x_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_T mode);
INT32 x_mtkapi_bt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout);
INT32 x_mtkapi_bt_mesh_ota_initiator_operation(BT_MESH_OTA_OPERATION_PARAMS_T *params);
INT32 x_mtkapi_bt_mesh_ota_get_client_model_handle(UINT16 *dist_client, UINT16 *update_client);
INT32 x_mtkapi_bt_mesh_set_special_pkt_params(BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params);

#ifdef MTK_GATT_BEARER_SUPPORT
INT32 x_mtkapi_bt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type);
INT32 x_mtkapi_bt_mesh_gatt_disconnect(VOID);
#endif


#endif
