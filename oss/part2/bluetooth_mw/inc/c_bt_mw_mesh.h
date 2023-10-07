/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2013
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

#ifndef _C_BT_MW_MESH_H_
#define _C_BT_MW_MESH_H_

/* INCLUDE FILE DECLARATIONS
 */

#include "u_bt_mw_mesh.h"

extern INT32 c_btm_bt_mesh_init (BT_APP_MESH_CB_FUNC_T *func);
extern INT32 c_btm_bt_mesh_deinit (VOID);
extern VOID c_btm_bt_mesh_light_model_get_element_index(UINT32 model_id, UINT16 *element_idx);
extern UINT16 c_btm_bt_mesh_get_model_handle_by_elementIdx_and_modeId(UINT32 model_id, UINT16 element_idx);
extern INT32 c_btm_bt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md);
extern INT32 c_btm_bt_mesh_enable (const BT_MESH_INIT_PARAMS_T *init_params);
extern INT32 c_btm_bt_mesh_disable (VOID);
extern INT32 c_btm_bt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey);
extern INT32 c_btm_bt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey);
extern VOID c_btm_bt_mesh_unprov_dev_scan(BOOL start, UINT32 duration);
extern INT32 c_btm_bt_mesh_invite_provisioning(const UINT8 *target_uuid, const BT_MESH_PROV_INVITE_T *invite);
extern INT32 c_btm_bt_mesh_start_provisioning(const BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode);
extern INT32 c_btm_bt_mesh_set_prov_factor(const BT_MESH_PROV_FACTOR_T *factor);
extern INT32 c_btm_bt_mesh_model_cc_msg_tx(const BT_MESH_CONFIGURATION_MSG_TX_T *msg);
extern INT32 c_btm_bt_mesh_send_packet(const BT_MESH_TX_PARAMS_T *params);
extern INT32 c_btm_bt_mesh_send_packet_ex(const BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op);
extern INT32 c_btm_bt_mesh_data_reset(UINT32 record);
extern INT32 c_btm_bt_mesh_data_save(VOID);
extern INT32 c_btm_bt_mesh_data_set(BT_MESH_RECORD_T *mesh_data);
extern INT32 c_btm_bt_mesh_version(CHAR *buf);
extern VOID c_btm_bt_mesh_dump(BT_MESH_DUMP_TYPE_T type);
extern UINT16 c_btm_bt_mesh_get_element_address(UINT16 element_index);
extern VOID c_btm_bt_mesh_set_default_ttl(UINT8 def_ttl);
extern UINT8 c_btm_bt_mesh_get_default_ttl(VOID);
extern INT32 c_btm_bt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index);
extern INT32 c_btm_bt_mesh_access_model_reply(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_ACCESS_MESSAGE_TX_T *reply);
extern INT32 c_btm_bt_mesh_bearer_adv_set_params(const BT_MESH_BEARER_ADV_PARAMS_T *adv_params, const BT_MESH_BEARER_SCAN_PARAMS_T *scan_params);
extern VOID c_btm_bt_mesh_switch_adv(BOOL enable);
extern VOID c_btm_bt_mesh_log_setlevel(UINT32 level);
extern INT32 c_btm_bt_mesh_generate_uuid(UINT8 *uuid_buf);
extern INT32 c_btm_bt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info);
extern INT32 c_btm_bt_mesh_set_mesh_mode(BT_MESH_MODE_T mode);
extern INT32 c_btm_bt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout);
extern INT32 c_btm_bt_mesh_ota_initiator_operation(BT_MESH_OTA_OPERATION_PARAMS_T *params);
extern INT32 c_btm_bt_mesh_ota_get_client_model_handle(UINT16 *dist_client, UINT16 *update_client);
extern INT32 c_btm_bt_mesh_set_special_pkt_params(BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params);

#ifdef MTK_GATT_BEARER_SUPPORT
extern INT32 c_btm_bt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type);
extern INT32 c_btm_bt_mesh_gatt_disconnect(VOID);
#endif

#endif /*  _C_BT_MW_MESH_H_  */

