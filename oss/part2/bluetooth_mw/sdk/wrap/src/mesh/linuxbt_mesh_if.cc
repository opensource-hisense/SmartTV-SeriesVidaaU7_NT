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

/* FILE NAME:  linuxbt_gatt_if.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATT common operation interface to MW.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/


#include <string.h>
#include <stdlib.h>
#include "bt_mw_common.h"
#include "linuxbt_mesh_if.h"
#include "linuxbt_common.h"
#include "u_bt_mw_mesh.h"
#include "bt_mw_mesh.h"
#include "c_mw_config.h"

#if ENABLE_BLE_MESH
#include "ble_mesh_interface.h"
#include "util_dlist.h"

static void *model_handler_list = NULL;
static bt_mesh_health_client_evt_handler linuxbt_health_client_evt_cbk = NULL;

#define ARG_CHECK_NULL_AND_RETURN_CODE(arg)     \
    do {    \
        if (NULL == arg)    \
        {   \
            BT_DBG_ERROR(BT_DEBUG_MESH,"%s, Error. arg is NULL!!! L %d",__func__,__LINE__); \
            return -1;  \
        }   \
    }while(0)

#define ARG_CHECK_NULL(arg)     \
            do {    \
                if (NULL == arg)    \
                {   \
                    BT_DBG_ERROR(BT_DEBUG_MESH,"%s ARG NULL and return, L %d", __func__, __LINE__);  \
                    return;  \
                }   \
            }while(0)

//structure map
static inline void _linuxbt_mesh_prov_capabilities_t_map(BT_MESH_PROV_CAPABILITIES_T *from, meshif_prov_capabilities_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->number_of_elements = to->number_of_elements;
        from->algorithms = to->algorithms;
        from->public_key_type = to->public_key_type;
        from->static_oob_type = to->static_oob_type;
        from->output_oob_size = to->output_oob_size;
        from->output_oob_action = to->output_oob_action;
        from->input_oob_size = to->input_oob_size;
        from->input_oob_action = to->input_oob_action;
    }
    else
    {
        to->number_of_elements = from->number_of_elements;
        to->algorithms = from->algorithms;
        to->public_key_type = from->public_key_type;
        to->static_oob_type = from->static_oob_type;
        to->output_oob_size = from->output_oob_size;
        to->output_oob_action = from->output_oob_action;
        to->input_oob_size = from->input_oob_size;
        to->input_oob_action = from->input_oob_action;
    }
}

static inline void _linuxbt_mesh_evt_prov_request_auth_t_map(BT_MESH_EVT_PROV_REQUEST_AUTH_T *from, meshif_evt_prov_request_auth_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->method = to->method;
        from->action = to->action;
        from->size = to->size;
    }
    else
    {
        to->method = from->method;
        to->action = from->action;
        to->size = from->size;
    }
}

static inline void _linuxbt_mesh_prov_unprovisioned_device_t_map(BT_MESH_PROV_UNPROVISIONED_DEVICE_T *from, meshif_prov_unprovisioned_device_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        memcpy(from->uuid, to->uuid, sizeof(from->uuid));
        from->oob_info = to->oob_info;
        from->uri_hash = to->uri_hash;
    }
    else
    {
        memcpy(to->uuid, from->uuid, sizeof(to->uuid));
        to->oob_info = from->oob_info;
        to->uri_hash = from->uri_hash;
    }
}

static inline void _linuxbt_mesh_evt_prov_done_t_map(BT_MESH_EVT_PROV_DONE_T *from, meshif_evt_prov_done_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->address = to->address;
        memcpy(from->device_key, to->device_key, sizeof(from->device_key));
        from->success = to->success;
        from->gatt_bearer = to->gatt_bearer;
        from->reason = (BT_MESH_PROV_ERROR_T)to->reason;
    }
    else
    {
        to->address = from->address;
        memcpy(to->device_key, from->device_key, sizeof(to->device_key));
        to->success = from->success;
        to->gatt_bearer = from->gatt_bearer;
        to->reason = (meshif_prov_error_t)from->reason;
    }
}

static inline void _linuxbt_mesh_address_t_map(BT_MESH_ADDRESS_T *from, meshif_address_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->type = (BT_MESH_ADDRESS_TYPE_T)to->type;
        from->value = to->value;
        from->virtual_uuid = to->virtual_uuid;
    }
    else
    {
        to->type = (meshif_address_type_t)from->type;
        to->value = from->value;
        to->virtual_uuid = from->virtual_uuid;
    }
}

static inline void _linuxbt_mesh_security_t_map(BT_MESH_SECURITY_T *from, meshif_security_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->netidx = to->netidx;
        from->appidx = to->appidx;
        from->device_key = to->device_key;
    }
    else
    {
        to->netidx = from->netidx;
        to->appidx = from->appidx;
        to->device_key = from->device_key;
    }
}

static inline void _linuxbt_mesh_tx_params_t_map(BT_MESH_TX_PARAMS_T *from, meshif_tx_params_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->src = to->src;
        from->ttl = to->ttl;
        from->data = to->data;
        from->data_len = to->data_len;
    }
    else
    {
        to->src = from->src;
        to->ttl = from->ttl;
        to->data = from->data;
        to->data_len = from->data_len;
    }
    _linuxbt_mesh_address_t_map(&from->dst, &to->dst, reverse);
    _linuxbt_mesh_security_t_map(&from->security, &to->security, reverse);
}

static inline void _linuxbt_mesh_access_opcode_t_map(BT_MESH_ACCESS_OPCODE_T *from, meshif_access_opcode_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->opcode = to->opcode;
        from->company_id = to->company_id;
    }
    else
    {
        to->opcode = from->opcode;
        to->company_id = from->company_id;
    }
}

static inline void _linuxbt_mesh_access_message_rx_meta_t_map(BT_MESH_ACCESS_MESSAGE_RX_META_T *from, meshif_access_message_rx_meta_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->src_addr = to->src_addr;
        from->dst_addr = to->dst_addr;
        from->appkey_index = to->appkey_index;
        from->netkey_index = to->netkey_index;
        from->rssi = to->rssi;
        from->ttl = to->ttl;
    }
    else
    {
        to->src_addr = from->src_addr;
        to->dst_addr = from->dst_addr;
        to->appkey_index = from->appkey_index;
        to->netkey_index = from->netkey_index;
        to->rssi = from->rssi;
        to->ttl = from->ttl;
    }
}

static inline void _linuxbt_mesh_access_message_rx_t_map(BT_MESH_ACCESS_MESSAGE_RX_T *from, meshif_access_message_rx_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->buf = to->buf;
        from->buf_len = to->buf_len;
    }
    else
    {
        to->buf = from->buf;
        to->buf_len = from->buf_len;
    }
    _linuxbt_mesh_access_opcode_t_map(&from->opcode, &to->opcode, reverse);
    _linuxbt_mesh_access_message_rx_meta_t_map(&from->meta_data, &to->meta_data, reverse);
}

static inline void _linuxbt_mesh_access_message_tx_t_map(BT_MESH_ACCESS_MESSAGE_TX_T *from, meshif_access_message_tx_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->opcode.opcode = to->opcode.opcode;
        from->opcode.company_id = to->opcode.company_id;
        from->p_buffer = to->p_buffer;
        from->length = to->length;
    }
    else
    {
        to->opcode.opcode = from->opcode.opcode;
        to->opcode.company_id = from->opcode.company_id;
        to->p_buffer = from->p_buffer;
        to->length = from->length;
    }
}

static inline void _linuxbt_mesh_config_init_params_t_map(BT_MESH_CONFIG_INIT_PARAMS_T *from, meshif_config_init_params_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        memcpy(from->device_uuid, to->device_uuid, sizeof(from->device_uuid));
        from->oob_info = to->oob_info;
        from->default_ttl = to->default_ttl;
        from->uri = to->uri;
    }
    else
    {
        memcpy(to->device_uuid, from->device_uuid, sizeof(to->device_uuid));
        to->oob_info = from->oob_info;
        to->default_ttl = from->default_ttl;
        to->uri = from->uri;
    }
}

static inline void _linuxbt_mesh_init_params_t_map(BT_MESH_INIT_PARAMS_T *from, meshif_init_params_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->role = (BT_MESH_ROLE_T)to->role;
        from->feature_mask = to->feature_mask;
        if ((from->friend_params != NULL) && (to->friend_params != NULL))
        {
            from->friend_params->lpn_number = to->friend_params->lpn_number;
            from->friend_params->queue_size = to->friend_params->queue_size;
            from->friend_params->subscription_list_size = to->friend_params->subscription_list_size;
        }
        if ((from->debug != NULL) && (to->debug != NULL))
        {
            from->debug->verbose_level = to->debug->verbose_level;
            from->debug->info_level = to->debug->info_level;
            from->debug->notify_level = to->debug->notify_level;
            from->debug->warning_level = to->debug->warning_level;
        }
        if ((from->customize_params != NULL) && (to->customize_params != NULL))
        {
            from->customize_params->max_remote_node_cnt = to->customize_params->max_remote_node_cnt;
            from->customize_params->save2flash = to->customize_params->save2flash;
        }
    }
    else
    {
        to->role = (meshif_role_t)from->role;
        to->feature_mask = from->feature_mask;
        if ((from->friend_params != NULL) && (to->friend_params != NULL))
        {
            to->friend_params->lpn_number = from->friend_params->lpn_number;
            to->friend_params->queue_size = from->friend_params->queue_size;
            to->friend_params->subscription_list_size = from->friend_params->subscription_list_size;
        }
        if ((from->debug != NULL) && (to->debug != NULL))
        {
            to->debug->verbose_level = from->debug->verbose_level;
            to->debug->info_level = from->debug->info_level;
            to->debug->notify_level = from->debug->notify_level;
            to->debug->warning_level = from->debug->warning_level;
        }
        if ((from->customize_params != NULL) && (to->customize_params != NULL))
        {
            to->customize_params->max_remote_node_cnt = from->customize_params->max_remote_node_cnt;
            to->customize_params->save2flash = from->customize_params->save2flash;
        }
    }

    _linuxbt_mesh_prov_capabilities_t_map(&from->provisionee->cap, &to->provisionee->cap, reverse);
    _linuxbt_mesh_config_init_params_t_map(from->config, to->config, reverse);
}

static inline void _linuxbt_mesh_prov_start_t_map(BT_MESH_PROV_START_T *from, meshif_prov_start_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->algorithm = to->algorithm;
        from->public_key = to->public_key;
        from->authentication_method = to->authentication_method;
        from->authentication_action = to->authentication_action;
        from->authentication_size = to->authentication_size;
    }
    else
    {
        to->algorithm = from->algorithm;
        to->public_key = from->public_key;
        to->authentication_method = from->authentication_method;
        to->authentication_action = from->authentication_action;
        to->authentication_size = from->authentication_size;
    }
}

static inline void _linuxbt_mesh_prov_data_t_map(BT_MESH_PROV_DATA_T *from, meshif_prov_data_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        memcpy(from->netkey, to->netkey, sizeof(from->netkey));
        from->netkey_index = to->netkey_index;
        from->iv_index = to->iv_index;
        from->address = to->address;
        from->flags = to->flags;
    }
    else
    {
        memcpy(to->netkey, from->netkey, sizeof(to->netkey));
        to->netkey_index = from->netkey_index;
        to->iv_index = from->iv_index;
        to->address = from->address;
        to->flags = from->flags;
    }
}

static inline void _linuxbt_mesh_model_publication_state_t_map(BT_MESH_MODEL_PUBLICATION_STATE_T *from, meshif_model_publication_state_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->element_address = to->element_address;
        from->appkey_index = to->appkey_index;
        from->friendship_credential_flag = to->friendship_credential_flag;
        from->publish_ttl = to->publish_ttl;
        from->publish_period = to->publish_period;
        from->retransmit_count = to->retransmit_count;
        from->retransmit_interval_steps = to->retransmit_interval_steps;
        from->model_id = to->model_id;
    }
    else
    {
        to->element_address = from->element_address;
        to->appkey_index = from->appkey_index;
        to->friendship_credential_flag = from->friendship_credential_flag;
        to->publish_ttl = from->publish_ttl;
        to->publish_period = from->publish_period;
        to->retransmit_count = from->retransmit_count;
        to->retransmit_interval_steps = from->retransmit_interval_steps;
        to->model_id = from->model_id;
    }
    _linuxbt_mesh_address_t_map(&from->publish_address, &to->publish_address, reverse);
}

static inline void _linuxbt_mesh_heartbeat_publication_t_map(BT_MESH_HEARTBEAT_PUBLICATION_T *from, meshif_heartbeat_publication_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->destination = to->destination;
        from->count_log = to->count_log;
        from->period_log = to->period_log;
        from->ttl = to->ttl;
        from->features = to->features;
        from->netkey_index = to->netkey_index;
    }
    else
    {
        to->destination = from->destination;
        to->count_log = from->count_log;
        to->period_log = from->period_log;
        to->ttl = from->ttl;
        to->features = from->features;
        to->netkey_index = from->netkey_index;
    }
}

static inline void _linuxbt_mesh_heartbeat_subscription_t_map(BT_MESH_HEARTBEAT_SUBSCRIPTION_T *from, meshif_heartbeat_subscription_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->source = to->source;
        from->destination = to->destination;
        from->period_log = to->period_log;
    }
    else
    {
        to->source = from->source;
        to->destination = from->destination;
        to->period_log = from->period_log;
    }
}

static inline void _linuxbt_mesh_config_meta_t_map(BT_MESH_CONFIG_META_T *from, config_meta_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->src_addr = to->src_addr;
        from->dst_addr = to->dst_addr;
        from->ttl = to->ttl;
        from->msg_netkey_index = to->msg_netkey_index;
    }
    else
    {
        to->src_addr = from->src_addr;
        to->dst_addr = from->dst_addr;
        to->ttl = from->ttl;
        to->msg_netkey_index = from->msg_netkey_index;
    }
}

static inline void _linuxbt_mesh_tx_model_operation_t_map(BT_MESH_MODEL_OPERATION_T *from,
                                                                          meshif_model_operation_t *to, bool reverse)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    if (true == reverse)
    {
        from->op_type = (BT_MESH_MODEL_OPERATION_TYPE_T)to->op_type;
        from->model_handle = to->model_handle;
        from->reliable = to->reliable;
    }
    else
    {
        to->op_type = (meshif_model_operation_type_t)from->op_type;
        to->model_handle = from->model_handle;
        to->reliable = from->reliable;
    }
}

static inline void _linuxbt_netkey_t_map(BT_MESH_NETKEY_FLASH_DATA_T *from, meshif_netkey_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->keyidx = from->keyidx;
    memcpy(to->key, from->key, sizeof(from->key));
    to->ivphase = from->ivphase == 0xFF ? MESHIF_IV_INDEX_UPDATE_NORMAL_STATE_STAGE_1 : from->ivphase;
    to->ivIndex = from->ivIndex == 0xFFFFFFFF ? 0 : from->ivIndex;
    to->phase = from->phase == 0xFF ? MESHIF_KEY_REFRESH_STATE_NONE : from->phase;
    to->node_identity = from->node_identity == 0xFF ? MESHIF_FEATURE_STATE_ENABLED : from->node_identity;
    memcpy(to->tmpkey, from->tmpkey, sizeof(from->tmpkey));
}

static inline void _linuxbt_friend_t_map(BT_MESH_FRIEND_FLASH_DATA_T *from, meshif_friend_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->netkeyIdx = from->netkeyIdx;
    memcpy(to->lpn_addr, from->lpn_addr, sizeof(from->lpn_addr));
    memcpy(to->friend_addr, from->friend_addr, sizeof(from->friend_addr));
    memcpy(to->lpn_counter, from->lpn_counter, sizeof(from->lpn_counter));
    memcpy(to->friend_counter, from->friend_counter, sizeof(from->friend_counter));
}

static inline void _linuxbt_appkey_t_map(BT_MESH_APPKEY_FLASH_DATA_T *from, meshif_appkey_flash_data_t *to)
{
    uint8_t keyidx[3];
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->appkeyIdx = from->appkeyIdx;
    memcpy(to->key, from->key, sizeof(from->key));
    to->netkeyIdx = from->netkeyIdx;
    to->phase = from->phase == 0xFF ? MESHIF_KEY_REFRESH_STATE_NONE : from->phase;
    memcpy(to->tmpkey, from->tmpkey, sizeof(from->tmpkey));
}

static inline void _linuxbt_model_t_map(BT_MESH_MODEL_FLASH_DATA_T *from, meshif_model_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->appkeyIdx = from->appkeyIdx;
    to->idLength = from->idLength == 0xFF ? 2 : from->idLength;
    to->model_id = from->model_id;
    to->unicast_addr = from->unicast_addr;
}

static inline void _linuxbt_publication_t_map(BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T *from, meshif_model_publication_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->model_id = from->model_id;
    to->unicast_addr = from->unicast_addr;
    to->model_publication.addr = from->model_publication.addr;
    to->model_publication.appkey_index = from->model_publication.appkey_index;
    to->model_publication.flag = from->model_publication.flag;
    to->model_publication.rfu = from->model_publication.rfu;
    to->model_publication.ttl = from->model_publication.ttl;
    to->model_publication.period = from->model_publication.period;
    to->model_publication.retransmit_count = from->model_publication.retransmit_count;
    to->model_publication.retransmit_interval_steps = from->model_publication.retransmit_interval_steps;
}

static inline void _linuxbt_subscription_t_map(BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T *from, meshif_model_subscription_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->model_id = from->model_id;
    to->unicast_addr = from->unicast_addr;
    to->subscriptionAddr = from->subscriptionAddr;
}

static inline void _linuxbt_device_info_t_map(BT_MESH_DEVICE_FLASH_DATA_T *from, meshif_device_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    memcpy(to->uuid, from->uuid, sizeof(from->uuid));
    memcpy(to->deviceKey, from->deviceKey, sizeof(from->deviceKey));
    to->unicast_addr = from->unicast_addr;
}

static inline void _linuxbt_seq_num_t_map(BT_MESH_SEQNUM_FLASH_DATA_T *from, meshif_seqnum_flash_data_t *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);

    to->isValidData = from->isValidData;
    to->seq_num = to->seq_num;
}

static inline void _linuxbt_mesh_record_t_map(BT_MESH_RECORD_T *from, meshif_record *to)
{
    ARG_CHECK_NULL(from);
    ARG_CHECK_NULL(to);
    int i = 0;
    for (i = 0; i < BT_MESH_NET_KEY_RECORD_NUMBER; i++)
    {
        if (from->netkey[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_netkey_t_map(&from->netkey[i], &to->netkey[i]);
        }
    }
    for (i = 0; i < BT_MESH_FRIEND_RECORD_NUMBER; i++)
    {
        if (from->friend_info[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_friend_t_map(&from->friend_info[i], &to->mesh_friend[i]);
        }
    }
    for (i = 0; i < BT_MESH_APP_KEY_RECORD_NUMBER; i++)
    {
        if (from->appkey[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_appkey_t_map(&from->appkey[i], &to->appkey[i]);
        }
    }
    for (i = 0; i < BT_MESH_MODEL_RECORD_NUMBER; i++)
    {
        if (from->model[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_model_t_map(&from->model[i], &to->model[i]);
        }
    }
    for (i = 0; i < BT_MESH_MODEL_PUBLICATION_RECORD_NUMBER; i++)
    {
        if (from->publication[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_publication_t_map(&from->publication[i], &to->publication[i]);
        }
    }
    for (i = 0; i < BT_MESH_NET_KEY_RECORD_NUMBER; i++)
    {
        if (from->subscription[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_subscription_t_map(&from->subscription[i], &to->subscription[i]);
        }
    }
    for (i = 0; i < BT_MESH_LOCAL_DEVICE_INFO_RECORD_NUMBER; i++)
    {
        if (from->local_deviceInfo[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_device_info_t_map(&from->local_deviceInfo[i], &to->local_deviceInfo[i]);
        }
    }
    for (i = 0; i < BT_MESH_SEQUENCE_NUMBER_RECORD_NUMBER; i++)
    {
        if (from->seq_info[i].isValidData == MESHIF_FLASH_VALID_DATA)
        {
            _linuxbt_seq_num_t_map(&from->seq_info[i], &to->seq_info[i]);
        }
    }
}

static void _linuxbt_mesh_health_client_evt_handler_wrapper(uint16_t model_handle, meshif_access_message_rx_t *msg, const meshif_health_client_evt_t *event)
{
    BT_MESH_ACCESS_MESSAGE_RX_T rx_msg;
    BT_MESH_HEALTH_CLIENT_EVT_T evt;

    switch (msg->opcode.opcode)
    {
        case MESHIF_ACCESS_MSG_HEALTH_CURRENT_STATUS:
        {
            evt.data.current_status.test_id = event->data.current_status.test_id;
            evt.data.current_status.company_id = event->data.current_status.company_id;
            evt.data.current_status.fault_array = event->data.current_status.fault_array;
            evt.data.current_status.fault_array_length = event->data.current_status.fault_array_length;
            break;
        }
        case MESHIF_ACCESS_MSG_HEALTH_FAULT_STATUS:
        {
            evt.data.fault_status.test_id = event->data.fault_status.test_id;
            evt.data.fault_status.company_id = event->data.fault_status.company_id;
            evt.data.fault_status.fault_array = event->data.fault_status.fault_array;
            evt.data.fault_status.fault_array_length = event->data.fault_status.fault_array_length;
            break;
        }
        case MESHIF_ACCESS_MSG_HEALTH_PERIOD_STATUS:
        {
            evt.data.period_status.fast_period_divisor = event->data.period_status.fast_period_divisor;
            break;
        }
        case MESHIF_ACCESS_MSG_HEALTH_ATTENTION_STATUS:
        {
            evt.data.attention_status.attention = event->data.attention_status.attention;
            break;
        }
        default:
            BT_DBG_ERROR(BT_DEBUG_MESH,"%s unknown opcode: %d",__func__, msg->opcode.opcode);
            return;
    }
    #if 0
    if (NULL != linuxbt_health_client_evt_cbk)
    {
        _linuxbt_mesh_access_message_rx_t_map(&bt_msg, msg, true);
        linuxbt_health_client_evt_cbk(model_handle, &bt_msg, &evt);
    }
    #endif
    {
        BT_MESH_CBK_HEALTH_CLIENT_EVT_T cbk_event;
        cbk_event.model_handle = model_handle;
        cbk_event.msg = &rx_msg;
        cbk_event.event = &evt;
        _linuxbt_mesh_access_message_rx_t_map(cbk_event.msg, msg, true);

        bluetooth_mesh_health_client_evt_cbk(&cbk_event);

    }
}

static void _linuxbt_mesh_add_model_handler_entry(BT_MESH_MODEL_HANDLER_ENTRY_T *new_entry)
{
    BT_MESH_MODEL_HANDLER_ENTRY_T *entry = NULL;

    entry = (BT_MESH_MODEL_HANDLER_ENTRY_T *)util_dlist_entry_alloc(sizeof(BT_MESH_MODEL_HANDLER_ENTRY_T));
    if (NULL == entry)
    {
        BT_DBG_ERROR(BT_DEBUG_MESH,"%s fail @ L %d",__func__,__LINE__);
        return;
    }
    entry->model_handle = new_entry->model_handle;
    entry->opcode_cnt = new_entry->opcode_cnt;
    entry->opcode = new_entry->opcode;
    entry->company_id = new_entry->company_id;
    entry->handler = new_entry->handler;

    BT_DBG_ERROR(BT_DEBUG_MESH,"add_model_handler_entry, model_handle =0x%x, opcode[0x%x][0x%x]",entry->model_handle,entry->opcode,entry->company_id);

    util_dlist_insert(model_handler_list, entry);
}

static void _linuxbt_mesh_model_access_msg_dispatch(uint16_t model_handle, meshif_access_message_rx_t *msg, void *arg)
{
    BT_MESH_CBK_ACCESS_MSG_T cbk_msg;
    BT_MESH_ACCESS_MESSAGE_RX_T rx_msg;

    cbk_msg.model_handle = model_handle;
    cbk_msg.msg = &rx_msg;
    cbk_msg.arg = arg;
    _linuxbt_mesh_access_message_rx_t_map(cbk_msg.msg, msg, true);

    bluetooth_mesh_access_msg_cbk(&cbk_msg);
}

static void _linuxbt_mesh_bt_event_handler(meshif_bt_evt_t *evt)
{
    BT_MESH_BT_EVENT_T event;

    if (NULL == evt)
    {
        BT_DBG_ERROR(BT_DEBUG_MESH,"%s, event is NULL",__func__);
        return;
    }
    memset(&event, 0, sizeof(BT_MESH_BT_EVENT_T));
    event.evt_id = evt->evt_id;

    switch (evt->evt_id)
    {
        case MESHIF_EVT_PROV_CAPABILITIES:
        {
            _linuxbt_mesh_prov_capabilities_t_map(&event.evt.mesh_evt.mesh.prov_cap.cap, &evt->evt.mesh_evt.mesh.prov_cap.cap, true);
            break;
        }
        case MESHIF_EVT_PROV_REQUEST_OOB_PUBLIC_KEY:
        {
            break;
        }
        case MESHIF_EVT_PROV_REQUEST_OOB_AUTH_VALUE:
        {
            _linuxbt_mesh_evt_prov_request_auth_t_map(&event.evt.mesh_evt.mesh.prov_request_auth, &evt->evt.mesh_evt.mesh.prov_request_auth, true);
            break;
        }
        case MESHIF_EVT_PROV_SHOW_OOB_PUBLIC_KEY:
        {
            event.evt.mesh_evt.mesh.prov_show_pk.pk = evt->evt.mesh_evt.mesh.prov_show_pk.pk;
            break;
        }
        case MESHIF_EVT_PROV_SHOW_OOB_AUTH_VALUE:
        {
            memcpy(event.evt.mesh_evt.mesh.prov_show_auth.auth, evt->evt.mesh_evt.mesh.prov_show_auth.auth, sizeof(evt->evt.mesh_evt.mesh.prov_show_auth.auth));
            break;
        }
        case MESHIF_EVT_PROV_DONE:
        {
            _linuxbt_mesh_evt_prov_done_t_map(&event.evt.mesh_evt.mesh.prov_done, &evt->evt.mesh_evt.mesh.prov_done, true);
            break;
        }
        case MESHIF_EVT_PROV_SCAN_UD_RESULT:
        {
            event.evt.mesh_evt.mesh.prov_scan_ud.total_count = evt->evt.mesh_evt.mesh.prov_scan_ud.total_count;
            event.evt.mesh_evt.mesh.prov_scan_ud.current_index = evt->evt.mesh_evt.mesh.prov_scan_ud.current_index;
            _linuxbt_mesh_prov_unprovisioned_device_t_map(&event.evt.mesh_evt.mesh.prov_scan_ud.ud, &evt->evt.mesh_evt.mesh.prov_scan_ud.ud, true);
            //callback to APP and then clear the ud_list, APP shall copy and store the ud info
            break;
        }
        case MESHIF_EVT_PROV_FACTOR:
        {
            event.evt.mesh_evt.mesh.prov_factor.type = (BT_MESH_PROV_FACTOR_TYPE_T)evt->evt.mesh_evt.mesh.prov_factor.type;
            event.evt.mesh_evt.mesh.prov_factor.buf = evt->evt.mesh_evt.mesh.prov_factor.buf;
            event.evt.mesh_evt.mesh.prov_factor.buf_len = evt->evt.mesh_evt.mesh.prov_factor.buf_len;
            break;
        }
        case MESHIF_EVT_ADV_REPORT:
        {
            event.evt.mesh_evt.mesh.adv_report.rssi = evt->evt.mesh_evt.mesh.adv_report.rssi;
            event.evt.mesh_evt.mesh.adv_report.type = (BT_MESH_REPORT_TYPE)evt->evt.mesh_evt.mesh.adv_report.type;
            event.evt.mesh_evt.mesh.adv_report.dlen = evt->evt.mesh_evt.mesh.adv_report.dlen;
            memcpy(event.evt.mesh_evt.mesh.adv_report.data, evt->evt.mesh_evt.mesh.adv_report.data, event.evt.mesh_evt.mesh.adv_report.dlen);
            event.evt.mesh_evt.mesh.adv_report.peer_addr.addr_type = (BT_MESH_BLE_ADDR_TYPE_T)evt->evt.mesh_evt.mesh.adv_report.peer_addr.addr_type;
            linuxbt_mesh_btaddr_htos(evt->evt.mesh_evt.mesh.adv_report.peer_addr.addr, event.evt.mesh_evt.mesh.adv_report.peer_addr.addr);
        }
            break;
        case MESHIF_EVT_KEY_REFRESH:
        {
            event.evt.mesh_evt.mesh.key_refresh.netkey_index = evt->evt.mesh_evt.mesh.key_refresh.netkey_index;
            event.evt.mesh_evt.mesh.key_refresh.phase = (BT_MESH_KEY_REFRESH_STATE_T)evt->evt.mesh_evt.mesh.key_refresh.phase;
            break;
        }
        case MESHIF_EVT_HEARTBEAT:
        {
            event.evt.mesh_evt.mesh.heartbeat.address = evt->evt.mesh_evt.mesh.heartbeat.address;
            event.evt.mesh_evt.mesh.heartbeat.active = evt->evt.mesh_evt.mesh.heartbeat.active;
            break;
        }
        case MESHIF_EVT_IV_UPDATE:
        {
            event.evt.mesh_evt.mesh.iv_update.iv_index = evt->evt.mesh_evt.mesh.iv_update.iv_index;
            event.evt.mesh_evt.mesh.iv_update.state = (BT_MESH_IV_UPDATE_STATE_T)evt->evt.mesh_evt.mesh.iv_update.state;
            event.evt.mesh_evt.mesh.iv_update.iv_phase = evt->evt.mesh_evt.mesh.iv_update.iv_phase;
            break;
        }
        case MESHIF_EVT_SEQ_CHANGE:
        {
            event.evt.mesh_evt.mesh.seq_change.seq_num = evt->evt.mesh_evt.mesh.seq_change.seq_num;
            break;
        }
        case MESHIF_EVT_OTA_EVENT:
        {
            event.evt.mesh_evt.mesh.ota_evt.event_id = (BT_MESH_OTA_EVENT_T)evt->evt.mesh_evt.mesh.ota_evt.event_id;
            event.evt.mesh_evt.mesh.ota_evt.error_code = (BT_MESH_OTA_ERROR_CODE_T)evt->evt.mesh_evt.mesh.ota_evt.error_code;
            event.evt.mesh_evt.mesh.ota_evt.serial_number = evt->evt.mesh_evt.mesh.ota_evt.serial_number;
            event.evt.mesh_evt.mesh.ota_evt.firmware_id = evt->evt.mesh_evt.mesh.ota_evt.firmware_id;
            event.evt.mesh_evt.mesh.ota_evt.time_escaped = evt->evt.mesh_evt.mesh.ota_evt.time_escaped;
            event.evt.mesh_evt.mesh.ota_evt.nodes_num = evt->evt.mesh_evt.mesh.ota_evt.nodes_num;
            event.evt.mesh_evt.mesh.ota_evt.curr_block = evt->evt.mesh_evt.mesh.ota_evt.curr_block;
            event.evt.mesh_evt.mesh.ota_evt.total_block = evt->evt.mesh_evt.mesh.ota_evt.total_block;
            event.evt.mesh_evt.mesh.ota_evt.curr_chunk = evt->evt.mesh_evt.mesh.ota_evt.curr_chunk;
            event.evt.mesh_evt.mesh.ota_evt.chunk_mask = evt->evt.mesh_evt.mesh.ota_evt.chunk_mask;
            event.evt.mesh_evt.mesh.ota_evt.nodes_status = (BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T *)evt->evt.mesh_evt.mesh.ota_evt.nodes_status;
            break;
        }
        case MESHIF_EVT_FRIENDSHIP_STATUS:
        {
            event.evt.mesh_evt.mesh.friendship_status.address = evt->evt.mesh_evt.mesh.friendship_status.address;
            event.evt.mesh_evt.mesh.friendship_status.status = (BT_MESH_FRIDSHIP_STATUS_T)evt->evt.mesh_evt.mesh.friendship_status.status;
            break;
        }
        case MESHIF_EVT_BEARER_GATT_STATUS:
        {
            event.evt.mesh_evt.mesh.bearer_gatt_status.handle = evt->evt.mesh_evt.mesh.bearer_gatt_status.handle;
            event.evt.mesh_evt.mesh.bearer_gatt_status.status = (BT_MESH_BEARER_GATT_STATUS_T)evt->evt.mesh_evt.mesh.bearer_gatt_status.status;
            break;
        }
        case MESHIF_EVT_ERROR_CODE:
        {
            event.evt.mesh_evt.mesh.error_code.type = (BT_MESH_ERROR_CODE_TYPE_T)evt->evt.mesh_evt.mesh.error_code.type;
            break;
        }
        case MESHIF_EVT_INIT_DONE:
        case MESHIF_EVT_CONFIG_RESET:
        case MESHIF_EVT_PROV_UD_RESULT_COMPLETE:
        default:
        {
            //We cannot return here. There is no data for these event, but they have to be notified to upper layer.
            break;
        }
    }

    bluetooth_mesh_bt_event_cbk(&event);
}
INT32 linuxbt_mesh_init(VOID)
{
    meshif_status_code_t ret = MESHIF_SUCCESS;

    ret = meshif_init(_linuxbt_mesh_bt_event_handler);
    if (MESHIF_SUCCESS == ret)
    {
        if (NULL == model_handler_list)
        {
            model_handler_list = (void *)util_dlist_alloc();
        }
    }
    return ret;
}

INT32 linuxbt_mesh_deinit(VOID)
{
    meshif_status_code_t ret = MESHIF_SUCCESS;

    ret = meshif_deinit();
    if (MESHIF_SUCCESS == ret)
    {
        if (NULL != model_handler_list)
        {
            util_dlist_free(model_handler_list);
            model_handler_list = NULL;
        }
    }
    return ret;
}

VOID linuxbt_mesh_light_model_get_element_index(UINT32 model_id, UINT16 *element_idx)
{
    meshif_light_model_get_element_index(model_id, element_idx);
}

UINT16 linuxbt_mesh_get_model_handle_by_elementIdx_and_modeId(UINT32 model_id, UINT16 element_idx)
{
    UINT16 model_handle = 0xFFFF;

    model_handle = meshif_get_model_handle_by_elementIdx_and_modeId(model_id, element_idx);

    return model_handle;
}

INT32 linuxbt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md)
{
    BOOL ret = true;
    meshif_model_data_t model_data;
    BT_MESH_MODEL_HANDLER_ENTRY_T entry;

    ARG_CHECK_NULL_AND_RETURN_CODE(md);

    memset(&model_data, 0, sizeof(meshif_model_data_t));
    memset(&entry, 0, sizeof(BT_MESH_MODEL_HANDLER_ENTRY_T));
    BT_DBG_NORMAL(BT_DEBUG_MESH,"OPCODE:0x%x", md->opcode);

    model_data.opcode = (meshif_model_data_opcode_t)md->opcode;

    switch (md->opcode)
    {
        case MESHIF_MODEL_DATA_OP_SET_COMPOSITION_DATA_HEADER:
            model_data.data.composition_data.buf = md->data.composition_data.buf;
            model_data.data.composition_data.buf_len = md->data.composition_data.buf_len;
            ret = meshif_set_model_data(&model_data);
            break;
        case MESHIF_MODEL_DATA_OP_ADD_ELEMENT:
            model_data.data.element.element_index = md->data.element.element_index;
            model_data.data.element.location = md->data.element.location;
            ret = meshif_set_model_data(&model_data);
            break;
        case MESHIF_MODEL_DATA_OP_SET_ELEMENT_ADDR:
            model_data.data.element_addr.unicast_addr = md->data.element_addr.unicast_addr;
            ret = meshif_set_model_data(&model_data);
            break;
        case MESHIF_MODEL_DATA_OP_ADD_CONFIGURATION_SERVER:
        {
            model_data.data.configuration_server.model_handle = md->data.configuration_server.model_handle;
            model_data.data.configuration_server.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.configuration_server.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.configuration_server.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_CONFIGURATION_CLIENT:
        {
            model_data.data.configuration_client.model_handle = md->data.configuration_client.model_handle;
            model_data.data.configuration_client.element_index = md->data.configuration_client.element_index;
            model_data.data.configuration_client.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.configuration_client.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.configuration_client.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_HEALTH_SERVER:
        {
            model_data.data.health_server.model_handle = md->data.health_server.model_handle;
            model_data.data.health_server.element_index = md->data.health_server.element_index;
            model_data.data.health_server.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.health_server.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.health_server.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_HEALTH_CLIENT:
        {
            linuxbt_health_client_evt_cbk = md->data.health_client.callback;
            model_data.data.health_client.model_handle = md->data.health_client.model_handle;
            model_data.data.health_client.element_index = md->data.health_client.element_index;
            model_data.data.health_client.callback = _linuxbt_mesh_health_client_evt_handler_wrapper;
            ret = meshif_set_model_data(&model_data);
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_GENERIC_ONOFF_SERVER:
        {
            model_data.data.generic_onoff_server.model_handle = md->data.generic_onoff_server.model_handle;
            model_data.data.generic_onoff_server.element_index = md->data.generic_onoff_server.element_index;
            model_data.data.generic_onoff_server.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.generic_onoff_server.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.generic_onoff_server.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_CTL_SETUP_SERVER:
        {
            model_data.data.ctl_setup_server.model_handle = md->data.ctl_setup_server.model_handle;
            model_data.data.ctl_setup_server.element_index = md->data.ctl_setup_server.element_index;
            model_data.data.ctl_setup_server.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);

            if (true == ret)
            {
                UINT16 ctl_temp_element_idx = 0;
                meshif_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER, &ctl_temp_element_idx);

                ble_mesh_extend_model_element_info_t extend_model_element_info[] = {
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SETUP_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                    {ctl_temp_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER},
                    {ctl_temp_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER},
                    {model_data.data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER},
                };

                if (NULL != md->data.ctl_setup_server.callback)
                {
                    for (int i = 0; i < (sizeof(extend_model_element_info) / sizeof(ble_mesh_extend_model_element_info_t)); i++)
                    {
                        entry.model_handle = meshif_get_model_handle_by_elementIdx_and_modeId(extend_model_element_info[i].model_id,
                                                                                              extend_model_element_info[i].element_idx);
                        entry.handler = md->data.ctl_setup_server.callback;
                        entry.opcode_cnt = 0;
                        _linuxbt_mesh_add_model_handler_entry(&entry);
                    }
                }
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_GENERIC_POWER_ONOFF_CLIENT:
        {
            model_data.data.generic_power_onoff_client.model_handle = md->data.generic_power_onoff_client.model_handle;
            model_data.data.generic_power_onoff_client.element_index = md->data.generic_power_onoff_client.element_index;
            model_data.data.generic_power_onoff_client.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.generic_power_onoff_client.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.generic_power_onoff_client.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_LIGHTNESS_CLIENT:
        {
            model_data.data.lightness_client.model_handle = md->data.lightness_client.model_handle;
            model_data.data.lightness_client.element_index = md->data.lightness_client.element_index;
            model_data.data.lightness_client.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.lightness_client.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.lightness_client.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_CTL_CLIENT:
        {
            model_data.data.ctl_client.model_handle = md->data.ctl_client.model_handle;
            model_data.data.ctl_client.element_index = md->data.ctl_client.element_index;
            model_data.data.ctl_client.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.ctl_client.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.ctl_client.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_HSL_SETUP_SERVER:
        {
            model_data.data.hsl_setup_server.model_handle = md->data.hsl_setup_server.model_handle;
            model_data.data.hsl_setup_server.element_index = md->data.hsl_setup_server.element_index;
            model_data.data.hsl_setup_server.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);

            if (true == ret)
            {
                UINT16 hsl_hue_element_idx = 0;
                UINT16 hsl_saturation_element_idx = 0;
                meshif_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_HSL_HUE_SERVER, &hsl_hue_element_idx);
                meshif_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SATURATION_SERVER, &hsl_saturation_element_idx);

                ble_mesh_extend_model_element_info_t extend_model_element_info[] = {
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SETUP_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                    {hsl_hue_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_HUE_SERVER},
                    {hsl_hue_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                    {hsl_saturation_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SATURATION_SERVER},
                    {hsl_saturation_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER},
                    {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER},
                };

                if (NULL != md->data.hsl_setup_server.callback)
                {
                    for (int i = 0; i < (sizeof(extend_model_element_info) / sizeof(ble_mesh_extend_model_element_info_t)); i++)
                    {
                        entry.model_handle = meshif_get_model_handle_by_elementIdx_and_modeId(extend_model_element_info[i].model_id,
                                                                                              extend_model_element_info[i].element_idx);
                        entry.handler = md->data.hsl_setup_server.callback;
                        entry.opcode_cnt = 0;
                        _linuxbt_mesh_add_model_handler_entry(&entry);
                    }
                }
            }
            break;
        }

        case MESHIF_MODEL_DATA_OP_ADD_HSL_CLIENT:
        {
            model_data.data.hsl_client.model_handle = md->data.hsl_client.model_handle;
            model_data.data.hsl_client.element_index = md->data.hsl_client.element_index;
            model_data.data.hsl_client.callback = _linuxbt_mesh_model_access_msg_dispatch;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                entry.model_handle = *(model_data.data.hsl_client.model_handle);
                entry.opcode_cnt = 0;
                entry.handler = md->data.hsl_client.callback;
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            break;
        }
        case MESHIF_MODEL_DATA_OP_ADD_MODEL:
        {
            UINT32 i;
            meshif_model_add_params_t add_params;
            meshif_access_opcode_handler_t temp_handlers[md->data.model_data.model_params->opcode_count];

            memset(&add_params, 0, sizeof(meshif_model_add_params_t));

            for (i = 0; i < md->data.model_data.model_params->opcode_count; i++)
            {
                temp_handlers[i].opcode.opcode = md->data.model_data.model_params->opcode_handlers[i].opcode.opcode;
                temp_handlers[i].opcode.company_id = md->data.model_data.model_params->opcode_handlers[i].opcode.company_id;
                temp_handlers[i].handler = _linuxbt_mesh_model_access_msg_dispatch;
            }

            add_params.model_id = md->data.model_data.model_params->model_id;
            add_params.element_index = md->data.model_data.model_params->element_index;
            add_params.opcode_handlers = temp_handlers;
            add_params.opcode_count = md->data.model_data.model_params->opcode_count;

            model_data.data.generic_model_data.model_handle = md->data.model_data.model_handle;
            model_data.data.generic_model_data.model_params = &add_params;
            ret = meshif_set_model_data(&model_data);
            if (true == ret)
            {
                for (i = 0; i < md->data.model_data.model_params->opcode_count; i++)
                {
                    BT_MESH_MODEL_HANDLER_ENTRY_T entry;
                    entry.model_handle = *(md->data.model_data.model_handle);
                    entry.opcode_cnt = md->data.model_data.model_params->opcode_count;
                    entry.opcode = md->data.model_data.model_params->opcode_handlers[i].opcode.opcode;
                    entry.company_id = md->data.model_data.model_params->opcode_handlers[i].opcode.company_id;
                    entry.handler = md->data.model_data.model_params->opcode_handlers[i].handler;

                    _linuxbt_mesh_add_model_handler_entry(&entry);
                }
                BT_DBG_NORMAL(BT_DEBUG_MESH,"%s, ADD_MODEL, model_handle=0x%x",entry.model_handle);
            }
            break;
        }
        default:
            BT_DBG_ERROR(BT_DEBUG_MESH,"invalid opcode!!!");
            return false;
    }

    return ret;
}

INT32 linuxbt_mesh_enable(BT_MESH_INIT_PARAMS_T *init_params)
{
    meshif_init_params_t params;
    meshif_prov_provisionee_params_t provisionee_params;
    meshif_config_init_params_t config_params;
    meshif_friend_init_params_t friend_params;
    meshif_debug_init_params_t debug;
    meshif_customize_para_t cust_para;

    ARG_CHECK_NULL_AND_RETURN_CODE(init_params);

    memset(&params, 0, sizeof(meshif_init_params_t));
    memset(&provisionee_params, 0, sizeof(meshif_prov_provisionee_params_t));
    memset(&config_params, 0, sizeof(meshif_config_init_params_t));
    memset(&friend_params, 0, sizeof(meshif_friend_init_params_t));
    memset(&debug, 0, sizeof(meshif_debug_init_params_t));

    friend_params.lpn_number = MESHIF_FRIEND_DEFAULT_LPN_NUMBER;
    friend_params.queue_size = MESHIF_FRIEND_DEFAULT_QUEUE_SIZE;
    friend_params.subscription_list_size = MESHIF_FRIEND_DEFAULT_SUBSCRIPTION_LIST_SIZE;

    params.provisionee = &provisionee_params;
    params.config = &config_params;
    params.friend_params = &friend_params;
    params.debug = &debug;
    params.customize_params = &cust_para;
    _linuxbt_mesh_init_params_t_map(init_params, &params, false);

    return meshif_enable(&params);
}

INT32 linuxbt_mesh_disable(VOID)
{
    return meshif_disable();
}

INT32 linuxbt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey)
{
    meshif_netkey_t key;

    ARG_CHECK_NULL_AND_RETURN_CODE(netkey);

    memset(&key, 0, sizeof(meshif_netkey_t));
    key.opcode = (meshif_key_opcode_t)netkey->opcode;
    key.network_key = netkey->network_key;
    key.key_index = netkey->key_index;

    return meshif_set_netkey(&key);
}

INT32 linuxbt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey)
{
    meshif_appkey_t key;

    ARG_CHECK_NULL_AND_RETURN_CODE(appkey);

    memset(&key, 0, sizeof(meshif_netkey_t));
    key.opcode = (meshif_key_opcode_t)appkey->opcode;
    key.application_key = appkey->application_key;
    key.appkey_index = appkey->appkey_index;
    key.netkey_index = appkey->netkey_index;

    return meshif_set_appkey(&key);
}

VOID linuxbt_mesh_unprov_dev_scan(BOOL start, UINT32 duration)
{
    meshif_unprov_dev_scan(start, duration);
}

INT32 linuxbt_mesh_invite_provisioning(UINT8 *target_uuid, BT_MESH_PROV_INVITE_T *invite)
{
    meshif_prov_invite_t inv;

    ARG_CHECK_NULL_AND_RETURN_CODE(invite);

    inv.attention_duration = invite->attention_duration;
    return meshif_invite_provisioning(target_uuid, &inv);
}

INT32 linuxbt_mesh_start_provisioning(BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode)
{
    meshif_prov_provisioner_params_t provisioner_params;

    ARG_CHECK_NULL_AND_RETURN_CODE(params);

    memset(&provisioner_params, 0, sizeof(meshif_prov_provisioner_params_t));
    _linuxbt_mesh_prov_start_t_map(&params->start, &provisioner_params.start, false);
    _linuxbt_mesh_prov_data_t_map(&params->data, &provisioner_params.data, false);

    return meshif_start_provisioning(&provisioner_params, mode);
}

INT32 linuxbt_mesh_set_prov_factor(BT_MESH_PROV_FACTOR_T *factor)
{
    meshif_prov_factor_t prov_factor;

    ARG_CHECK_NULL_AND_RETURN_CODE(factor);
    ARG_CHECK_NULL_AND_RETURN_CODE(factor->buf);

    prov_factor.type = (meshif_prov_factor_type_t)factor->type;
    prov_factor.buf = factor->buf;
    prov_factor.buf_len = factor->buf_len;

    return meshif_set_prov_factor(&prov_factor);
}

INT32 linuxbt_mesh_model_cc_msg_tx(BT_MESH_CONFIGURATION_MSG_TX_T *msg)
{
    meshif_configuration_msg_tx_t tx_msg;

    ARG_CHECK_NULL_AND_RETURN_CODE(msg);

    BT_DBG_NORMAL(BT_DEBUG_MESH,"%s CONFIG ACCESS_MSG OPCODE:0x%x", __func__, msg->opcode);

    memset(&tx_msg, 0, sizeof(meshif_configuration_msg_tx_t));
    tx_msg.opcode = msg->opcode;
    switch (msg->opcode)
    {
        case MESHIF_ACCESS_MSG_CONFIG_BEACON_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.beacon_get.meta, &tx_msg.data.beacon_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_BEACON_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.beacon_get.meta, &tx_msg.data.beacon_get.meta, false);
            tx_msg.data.beacon_set.beacon = msg->data.beacon_set.beacon;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.composition_data_get.meta, &tx_msg.data.composition_data_get.meta, false);
            tx_msg.data.composition_data_get.page = msg->data.composition_data_get.page;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.default_ttl_get.meta, &tx_msg.data.default_ttl_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.beacon_get.meta, &tx_msg.data.beacon_get.meta, false);
            tx_msg.data.default_ttl_set.TTL = msg->data.default_ttl_set.TTL;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_GATT_PROXY_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.gatt_proxy_get.meta, &tx_msg.data.gatt_proxy_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_GATT_PROXY_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.gatt_proxy_set.meta, &tx_msg.data.gatt_proxy_set.meta, false);
            tx_msg.data.gatt_proxy_set.gatt_proxy = msg->data.gatt_proxy_set.gatt_proxy;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.key_ref_pha_get.meta, &tx_msg.data.key_ref_pha_get.meta, false);
            tx_msg.data.key_ref_pha_get.netkey_index = msg->data.key_ref_pha_get.netkey_index;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.beacon_get.meta, &tx_msg.data.beacon_get.meta, false);
            tx_msg.data.key_ref_pha_set.netkey_index = msg->data.key_ref_pha_set.netkey_index;
            tx_msg.data.key_ref_pha_set.transition = msg->data.key_ref_pha_set.transition;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_FRIEND_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.friend_get.meta, &tx_msg.data.friend_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_FRIEND_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.beacon_get.meta, &tx_msg.data.beacon_get.meta, false);
            tx_msg.data.friend_set.mesh_friend = msg->data.friend_set.mesh_friend;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_RELAY_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.relay_get.meta, &tx_msg.data.relay_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_RELAY_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.relay_set.meta, &tx_msg.data.relay_set.meta, false);
            tx_msg.data.relay_set.relay = msg->data.relay_set.relay;
            tx_msg.data.relay_set.retransmit_count = msg->data.relay_set.retransmit_count;
            tx_msg.data.relay_set.retransmit_interval_steps = msg->data.relay_set.retransmit_interval_steps;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_pub_get.meta, &tx_msg.data.model_pub_get.meta, false);
            tx_msg.data.model_pub_get.element_addr = msg->data.model_pub_get.element_addr;
            tx_msg.data.model_pub_get.model_id = msg->data.model_pub_get.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET:
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET:
        {
            meshif_model_publication_state_t pub_state;
            tx_msg.data.model_pub_set.state = &pub_state;
            memset(&pub_state, 0, sizeof(meshif_model_publication_state_t));
            _linuxbt_mesh_model_publication_state_t_map(msg->data.model_pub_set.state, tx_msg.data.model_pub_set.state, false);
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_pub_set.meta, &tx_msg.data.model_pub_set.meta, false);
            break;
        }
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD:
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_sub_add.meta, &tx_msg.data.model_sub_add.meta, false);
            tx_msg.data.model_sub_add.element_addr = msg->data.model_sub_add.element_addr;
            tx_msg.data.model_sub_add.model_id = msg->data.model_sub_add.model_id;
            _linuxbt_mesh_address_t_map(&msg->data.model_sub_add.address, &tx_msg.data.model_sub_add.address, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE:
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_sub_del.meta, &tx_msg.data.model_sub_del.meta, false);
            tx_msg.data.model_sub_del.element_addr = msg->data.model_sub_del.element_addr;
            tx_msg.data.model_sub_del.model_id = msg->data.model_sub_del.model_id;
            _linuxbt_mesh_address_t_map(&msg->data.model_sub_del.address, &tx_msg.data.model_sub_del.address, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_sub_del_all.meta, &tx_msg.data.model_sub_del_all.meta, false);
            tx_msg.data.model_sub_del_all.element_addr = msg->data.model_sub_del_all.element_addr;
            tx_msg.data.model_sub_del_all.model_id = msg->data.model_sub_del_all.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_sub_ow.meta, &tx_msg.data.model_sub_ow.meta, false);
            tx_msg.data.model_sub_ow.element_addr = msg->data.model_sub_ow.element_addr;
            tx_msg.data.model_sub_ow.model_id = msg->data.model_sub_ow.model_id;
            _linuxbt_mesh_address_t_map(&msg->data.model_sub_ow.address, &tx_msg.data.model_sub_ow.address, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.net_trans_get.meta, &tx_msg.data.net_trans_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.net_trans_set.meta, &tx_msg.data.net_trans_set.meta, false);
            tx_msg.data.net_trans_set.count = msg->data.net_trans_set.count;
            tx_msg.data.net_trans_set.interval_steps = msg->data.net_trans_set.interval_steps;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.sig_model_sub_get.meta, &tx_msg.data.sig_model_sub_get.meta, false);
            tx_msg.data.sig_model_sub_get.element_addr = msg->data.sig_model_sub_get.element_addr;
            tx_msg.data.sig_model_sub_get.model_id = msg->data.sig_model_sub_get.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.vendor_model_sub_get.meta, &tx_msg.data.vendor_model_sub_get.meta, false);
            tx_msg.data.vendor_model_sub_get.element_addr = msg->data.vendor_model_sub_get.element_addr;
            tx_msg.data.vendor_model_sub_get.model_id = msg->data.vendor_model_sub_get.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETKEY_ADD:
            _linuxbt_mesh_config_meta_t_map(&msg->data.netkey_add.meta, &tx_msg.data.netkey_add.meta, false);
            tx_msg.data.netkey_add.netkey_index = msg->data.netkey_add.netkey_index;
            tx_msg.data.netkey_add.netkey = msg->data.netkey_add.netkey;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETKEY_DELETE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.netkey_del.meta, &tx_msg.data.netkey_del.meta, false);
            tx_msg.data.netkey_del.netkey_index = msg->data.netkey_del.netkey_index;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETKEY_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.netkey_get.meta, &tx_msg.data.netkey_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NETKEY_UPDATE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.netkey_update.meta, &tx_msg.data.netkey_update.meta, false);
            tx_msg.data.netkey_update.netkey_index = msg->data.netkey_update.netkey_index;
            tx_msg.data.netkey_update.netkey = msg->data.netkey_update.netkey;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_APPKEY_ADD:
            _linuxbt_mesh_config_meta_t_map(&msg->data.appkey_add.meta, &tx_msg.data.appkey_add.meta, false);
            tx_msg.data.appkey_add.appkey_index = msg->data.appkey_add.appkey_index;
            tx_msg.data.appkey_add.netkey_index = msg->data.appkey_add.netkey_index;
            tx_msg.data.appkey_add.appkey = msg->data.appkey_add.appkey;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_APPKEY_UPDATE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.appkey_update.meta, &tx_msg.data.appkey_update.meta, false);
            tx_msg.data.appkey_update.appkey_index = msg->data.appkey_update.appkey_index;
            tx_msg.data.appkey_update.netkey_index = msg->data.appkey_update.netkey_index;
            tx_msg.data.appkey_update.appkey = msg->data.appkey_update.appkey;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_APPKEY_DELETE:
            _linuxbt_mesh_config_meta_t_map(&msg->data.appkey_del.meta, &tx_msg.data.appkey_del.meta, false);
            tx_msg.data.appkey_del.appkey_index = msg->data.appkey_del.appkey_index;
            tx_msg.data.appkey_del.netkey_index = msg->data.appkey_del.netkey_index;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_APPKEY_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.appkey_get.meta, &tx_msg.data.appkey_get.meta, false);
            tx_msg.data.appkey_get.netkey_index = msg->data.appkey_get.netkey_index;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_APP_BIND:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_app_bind.meta, &tx_msg.data.model_app_bind.meta, false);
            tx_msg.data.model_app_bind.element_addr = msg->data.model_app_bind.element_addr;
            tx_msg.data.model_app_bind.appkey_index = msg->data.model_app_bind.appkey_index;
            tx_msg.data.model_app_bind.model_id = msg->data.model_app_bind.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND:
            _linuxbt_mesh_config_meta_t_map(&msg->data.model_app_unbind.meta, &tx_msg.data.model_app_unbind.meta, false);
            tx_msg.data.model_app_unbind.element_addr = msg->data.model_app_unbind.element_addr;
            tx_msg.data.model_app_unbind.appkey_index = msg->data.model_app_unbind.appkey_index;
            tx_msg.data.model_app_unbind.model_id = msg->data.model_app_unbind.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_SIG_MODEL_APP_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.sig_model_app_get.meta, &tx_msg.data.sig_model_app_get.meta, false);
            tx_msg.data.sig_model_app_get.element_addr = msg->data.sig_model_app_get.element_addr;
            tx_msg.data.sig_model_app_get.model_id = msg->data.sig_model_app_get.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.vendor_model_app_get.meta, &tx_msg.data.vendor_model_app_get.meta, false);
            tx_msg.data.vendor_model_app_get.element_addr = msg->data.vendor_model_app_get.element_addr;
            tx_msg.data.vendor_model_app_get.model_id = msg->data.vendor_model_app_get.model_id;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NODE_IDENTITY_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.node_identity_get.meta, &tx_msg.data.node_identity_get.meta, false);
            tx_msg.data.node_identity_get.netkey_index = msg->data.node_identity_get.netkey_index;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NODE_IDENTITY_SET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.node_identity_set.meta, &tx_msg.data.node_identity_set.meta, false);
            tx_msg.data.node_identity_set.netkey_index = msg->data.node_identity_set.netkey_index;
            tx_msg.data.node_identity_set.identity = msg->data.node_identity_set.identity;
            break;
        case MESHIF_ACCESS_MSG_CONFIG_NODE_RESET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.node_reset.meta, &tx_msg.data.node_reset.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.hb_pub_get.meta, &tx_msg.data.hb_pub_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET:
        {
            meshif_heartbeat_publication_t pub;
            tx_msg.data.hb_pub_set.publication = &pub;
            memset(&pub, 0, sizeof(meshif_heartbeat_publication_t));
            _linuxbt_mesh_heartbeat_publication_t_map(msg->data.hb_pub_set.publication, tx_msg.data.hb_pub_set.publication, false);
            _linuxbt_mesh_config_meta_t_map(&msg->data.hb_pub_set.meta, &tx_msg.data.hb_pub_set.meta, false);
            break;
        }
        case MESHIF_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_GET:
            _linuxbt_mesh_config_meta_t_map(&msg->data.hb_sub_get.meta, &tx_msg.data.hb_sub_get.meta, false);
            break;
        case MESHIF_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET:
        {
            meshif_heartbeat_subscription_t sub;
            tx_msg.data.hb_sub_set.subscription = &sub;
            memset(&sub, 0, sizeof(meshif_heartbeat_subscription_t));
            _linuxbt_mesh_heartbeat_subscription_t_map(msg->data.hb_sub_set.subscription, tx_msg.data.hb_sub_set.subscription, false);
            _linuxbt_mesh_config_meta_t_map(&msg->data.hb_sub_set.meta, &tx_msg.data.hb_sub_set.meta, false);
            break;
        }
        default:
            BT_DBG_ERROR(BT_DEBUG_MESH,"BLE_MESH_ACCESS_MSG UNKNOWN OPCODE:0x%x", msg->opcode);
            break;
    }

    return meshif_model_cc_msg_tx(&tx_msg);
}

INT32 linuxbt_mesh_send_packet(BT_MESH_TX_PARAMS_T *params)
{
    meshif_tx_params_t tx_params;

    ARG_CHECK_NULL_AND_RETURN_CODE(params);

    _linuxbt_mesh_tx_params_t_map(params, &tx_params, false);

    return meshif_send_packet(&tx_params);
}

INT32 linuxbt_mesh_send_packet_ex(BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op)
{
    meshif_tx_params_t tx_params;
    meshif_model_operation_t tx_model_op;

    ARG_CHECK_NULL_AND_RETURN_CODE(params);

    _linuxbt_mesh_tx_params_t_map(params, &tx_params, false);

    _linuxbt_mesh_tx_model_operation_t_map(model_op, &tx_model_op, false);
    return meshif_send_packet_ex(&tx_params, &tx_model_op);
}

INT32 linuxbt_mesh_data_reset(UINT32 record)
{
    if (meshif_data_reset(record))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

INT32 linuxbt_mesh_data_save(VOID)
{
    if (meshif_data_save())
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

INT32 linuxbt_mesh_data_set(BT_MESH_RECORD_T *mesh_data)
{
    meshif_record data;
    memset(&data, 0xFF, sizeof(meshif_record));

    ARG_CHECK_NULL_AND_RETURN_CODE(mesh_data);

    _linuxbt_mesh_record_t_map(mesh_data, &data);

    if (meshif_data_set(&data))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

INT32 linuxbt_mesh_version(CHAR *buf)
{
    ARG_CHECK_NULL_AND_RETURN_CODE(buf);
    if (NULL != meshif_version(buf))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

VOID linuxbt_mesh_dump(BT_MESH_DUMP_TYPE_T type)
{
    //meshif_dump((meshif_dump_type_t)type);
    return;
}

UINT16 linuxbt_mesh_get_element_address(UINT16 element_index)
{
    return meshif_get_element_address(element_index);
}

VOID linuxbt_mesh_set_default_ttl(UINT8 def_ttl)
{
    meshif_set_default_ttl(def_ttl);
}

UINT8 linuxbt_mesh_get_default_ttl(VOID)
{
    return meshif_get_default_ttl();
}

INT32 linuxbt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index)
{
    return meshif_model_app_bind(model_handle, appkey_index);
}

INT32 linuxbt_mesh_access_model_reply(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T *msg, BT_MESH_ACCESS_MESSAGE_TX_T *reply)
{
    meshif_access_message_rx_t rx_msg;
    meshif_access_message_tx_t tx_msg;

    _linuxbt_mesh_access_message_rx_t_map(msg, &rx_msg, false);
    _linuxbt_mesh_access_message_tx_t_map(reply, &tx_msg, false);

    return meshif_access_model_reply(model_handle, &rx_msg, &tx_msg);

}

INT32 linuxbt_mesh_bearer_adv_set_params(BT_MESH_BEARER_ADV_PARAMS_T *adv_params, BT_MESH_BEARER_SCAN_PARAMS_T *scan_params)
{
    meshif_bearer_adv_params_t adv_p;
    meshif_bearer_scan_params_t scan_p;

    if ((NULL == adv_params) && (NULL == scan_params))
    {
        BT_DBG_ERROR(BT_DEBUG_MESH,"%s, Error. arg is NULL!!! L %d",__func__,__LINE__);
        return -1;
    }

    if (NULL != adv_params)
    {
        adv_p.adv_period = adv_params->adv_period;
        adv_p.min_interval = adv_params->min_interval;
        adv_p.max_interval = adv_params->max_interval;
        adv_p.resend = adv_params->resend;
        if (NULL == scan_params)
        {
            return meshif_bearer_adv_set_params(&adv_p, NULL);
        }
    }

    if (NULL != scan_params)
    {
        scan_p.scan_period = scan_params->scan_period;
        scan_p.scan_interval = scan_params->scan_period;
        scan_p.scan_window = scan_params->scan_period;
        if (NULL == adv_params)
        {
            return meshif_bearer_adv_set_params(NULL, &scan_p);
        }
    }

    return meshif_bearer_adv_set_params(&adv_p, &scan_p);
}

VOID linuxbt_mesh_switch_adv(BOOL enable)
{
    meshif_switch_adv(enable);
}

VOID linuxbt_mesh_log_setlevel(UINT32 level)
{
    meshif_log_setlevel(level);
}

INT32 linuxbt_mesh_generate_uuid(UINT8 *uuid_buf)
{
    meshif_generate_uuid(uuid_buf);
}

INT32 linuxbt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info)
{
    meshif_access_msg_status_code_t ret = MESHIF_ACCESS_MSG_STATUS_SUCCESS;
    meshif_device_info_t dev_info;

    ARG_CHECK_NULL_AND_RETURN_CODE(info);

    memset(&dev_info, 0, sizeof(meshif_device_info_t));

    switch (info->opcode)
    {
        case BT_MESH_DEV_INFO_OP_ADD_DEVKEY:
        {
            dev_info.opcode = MESHIF_DEV_INFO_OP_ADD_DEVKEY;
            dev_info.data.devkey.unicast_addr = info->data.devkey.unicast_addr;
            memcpy(dev_info.data.devkey.deviceKey, info->data.devkey.deviceKey, MESHIF_DEVKEY_SIZE);
            memcpy(dev_info.data.devkey.uuid, info->data.devkey.uuid, MESHIF_UUID_SIZE);
            break;
        }
        case BT_MESH_DEV_INFO_OP_GET_DEVKEY:
        {
            dev_info.opcode = MESHIF_DEV_INFO_OP_GET_DEVKEY;
            dev_info.data.devkey.unicast_addr = info->data.devkey.unicast_addr;
            ret =  meshif_set_device_info(&dev_info);
            if (ret == MESHIF_ACCESS_MSG_STATUS_SUCCESS)
            {
                memcpy(info->data.devkey.deviceKey, dev_info.data.devkey.deviceKey, MESHIF_DEVKEY_SIZE);
                memcpy(info->data.devkey.uuid, dev_info.data.devkey.uuid, MESHIF_UUID_SIZE);
            }
            return ret;
        }
        case BT_MESH_DEV_INFO_OP_DELETE_DEVKEY:
        {
            dev_info.opcode = MESHIF_DEV_INFO_OP_DELETE_DEVKEY;
            dev_info.data.devkey.unicast_addr = info->data.devkey.unicast_addr;
            break;
        }
        default:
            BT_DBG_ERROR(BT_DEBUG_MESH,"UNKNOWN OPCODE:0x%x", info->opcode);
            break;
    }

    return meshif_set_device_info(&dev_info);
}

INT32 linuxbt_mesh_set_mesh_mode(BT_MESH_MODE_T mode)
{
    return meshif_set_mesh_mode((meshif_mesh_mode_t)mode);
}

INT32 linuxbt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout)
{
#if ENABLE_BLE_MESH_HEARTBEAT
    return meshif_set_heartbeat_period(num, hb_timeout);
#else
    return 0;
#endif
}

INT32 linuxbt_mesh_ota_initiator_operation(BT_MESH_OTA_OPERATION_PARAMS_T *params)
{
    meshif_ota_operation_params_t ota_params;

    ARG_CHECK_NULL_AND_RETURN_CODE(params);

    BT_DBG_NORMAL(BT_DEBUG_MESH,"%s OTA OPCODE:0x%x", __func__, params->opcode);

    memset(&ota_params, 0, sizeof(meshif_ota_operation_params_t));
    ota_params.opcode = params->opcode;

    switch (ota_params.opcode)
    {
        case MESHIF_OTA_INITIATOR_OP_REG_MSG_HANDLER:
        {
            UINT16 dist_client_handle = 0, update_client_handle = 0;
            BT_MESH_MODEL_HANDLER_ENTRY_T entry;
            memset(&entry, 0, sizeof(BT_MESH_MODEL_HANDLER_ENTRY_T));
            entry.opcode_cnt = 0;
            entry.handler = params->params.msg_handler.ota_msg_handler;
            meshif_ota_get_client_model_handle(&dist_client_handle, &update_client_handle);
            entry.model_handle = dist_client_handle;
            if (entry.model_handle != 0)
            {
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            else
            {
                BT_DBG_ERROR(BT_DEBUG_MESH,"%s, handle=0x%x", __func__, entry.model_handle);
            }

            entry.model_handle = update_client_handle;
            if (entry.model_handle != 0)
            {
                _linuxbt_mesh_add_model_handler_entry(&entry);
            }
            else
            {
                BT_DBG_ERROR(BT_DEBUG_MESH,"%s, handle=0x%x", __func__, entry.model_handle);
            }
            ota_params.params.msg_handler.appkey_index = params->params.msg_handler.appkey_index;
            ota_params.params.msg_handler.ota_msg_handler = _linuxbt_mesh_model_access_msg_dispatch;
            break;
        }
        case MESHIF_OTA_INITIATOR_OP_FW_INFO_GET:
        {
            ota_params.params.fw_info_get.node_addr = params->params.fw_info_get.node_addr;
            break;
        }
        case MESHIF_OTA_INITIATOR_OP_START_DISTRIBUTION:
        {
            snprintf(ota_params.params.start_params.obj_file, sizeof(ota_params.params.start_params.obj_file), "%s", params->params.start_params.obj_file);
            ota_params.params.start_params.obj_size = params->params.start_params.obj_size;
            memcpy(ota_params.params.start_params.obj_id, params->params.start_params.obj_id, sizeof(params->params.start_params.obj_id));
            ota_params.params.start_params.new_fw_id = params->params.start_params.new_fw_id;
            ota_params.params.start_params.appkey_index = params->params.start_params.appkey_index;
            ota_params.params.start_params.distributor_addr = params->params.start_params.distributor_addr;
            ota_params.params.start_params.group_addr = params->params.start_params.group_addr;
            ota_params.params.start_params.updaters = params->params.start_params.updaters;    //sdk allocate buffer and copy these info
            ota_params.params.start_params.updaters_num = params->params.start_params.updaters_num;
            ota_params.params.start_params.manual_apply = params->params.start_params.manual_apply;
            break;
        }
        case MESHIF_OTA_INITIATOR_OP_STOP_DISTRIBUTION:
        {
            ota_params.params.stop_params.new_fw_id = params->params.stop_params.new_fw_id;
            ota_params.params.stop_params.distributor_addr = params->params.stop_params.distributor_addr;
            break;
        }
        case MESHIF_OTA_INITIATOR_OP_APPLY_DISTRIBUTION:
        {
            break;
        }
        default:
            BT_DBG_ERROR(BT_DEBUG_MESH,"Invalid opcode %d\n", params->opcode);
            return -1;
    }
    return meshif_ota_initiator_operation(&ota_params);
}

INT32 linuxbt_mesh_ota_get_client_model_handle(UINT16 *dist_client, UINT16 *update_client)
{
    meshif_ota_get_client_model_handle(dist_client, update_client);
    return 0;
}

INT32 linuxbt_mesh_set_special_pkt_params(BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params)
{
    meshif_special_pkt_params_t params;
    memset(&params, 0, sizeof(meshif_special_pkt_params_t));
    params.sn_increase_flag = pkt_params->sn_increase_flag;
    params.sn_increase_interval = pkt_params->sn_increase_interval;
    params.adv_interval = pkt_params->adv_interval;
    params.adv_period= pkt_params->adv_period;
    return meshif_set_special_pkt_params(&params);
}

#ifdef MTK_GATT_BEARER_SUPPORT
INT32 linuxbt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type)
{
    meshif_ble_addr_t target;

    ARG_CHECK_NULL_AND_RETURN_CODE(addr);
    linuxbt_mesh_btaddr_stoh(addr->addr, target.addr);
    target.addr_type = addr->addr_type;
    return meshif_gatt_connect(&target, type);
}

INT32 linuxbt_mesh_gatt_disconnect(VOID)
{
    return meshif_gatt_disconnect();
}
#endif

#else   /*ENABLE_BLE_MESH*/
INT32 linuxbt_mesh_init(VOID)
{
    return 0;
}

INT32 linuxbt_mesh_deinit(VOID)
{
    return 0;
}

VOID linuxbt_mesh_light_model_get_element_index(UINT32 model_id, UINT16 *element_idx)
{
    return;
}

UINT16 linuxbt_mesh_get_model_handle_by_elementIdx_and_modeId(UINT32 model_id, UINT16 element_idx)
{
    return 0xFFFF;
}

INT32 linuxbt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md)
{
    return 0;
}

INT32 linuxbt_mesh_enable(BT_MESH_INIT_PARAMS_T *init_params)
{
    return 0;
}

INT32 linuxbt_mesh_disable(VOID)
{
    return 0;
}

INT32 linuxbt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey)
{
    return 0;
}

INT32 linuxbt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey)
{
    return 0;
}

VOID linuxbt_mesh_unprov_dev_scan(BOOL start, UINT32 duration)
{
    return;
}

INT32 linuxbt_mesh_invite_provisioning(UINT8 *target_uuid, BT_MESH_PROV_INVITE_T *invite)
{
    return 0;
}

INT32 linuxbt_mesh_start_provisioning(BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode)
{
    return 0;
}

INT32 linuxbt_mesh_set_prov_factor(BT_MESH_PROV_FACTOR_T *factor)
{
    return 0;
}

INT32 linuxbt_mesh_model_cc_msg_tx(BT_MESH_CONFIGURATION_MSG_TX_T *msg)
{
    return 0;
}

INT32 linuxbt_mesh_send_packet(BT_MESH_TX_PARAMS_T *params)
{
    return 0;
}

INT32 linuxbt_mesh_send_packet_ex(BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op)
{
    return 0;
}

INT32 linuxbt_mesh_data_reset(UINT32 record)
{
    return 0;
}

INT32 linuxbt_mesh_data_save(VOID)
{
    return 0;
}

INT32 linuxbt_mesh_data_set(BT_MESH_RECORD_T *mesh_data)
{
    return 0;
}

INT32 linuxbt_mesh_version(CHAR *buf)
{
    return 0;
}

VOID linuxbt_mesh_dump(BT_MESH_DUMP_TYPE_T type)
{
    return;
}

UINT16 linuxbt_mesh_get_element_address(UINT16 element_index)
{
    return 0;
}

VOID linuxbt_mesh_set_default_ttl(UINT8 def_ttl)
{
    return;
}

UINT8 linuxbt_mesh_get_default_ttl(VOID)
{
    return 0;
}

INT32 linuxbt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index)
{
    return 0;
}

INT32 linuxbt_mesh_access_model_reply(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T *msg, BT_MESH_ACCESS_MESSAGE_TX_T *reply)
{
    return 0;
}

INT32 linuxbt_mesh_bearer_adv_set_params(BT_MESH_BEARER_ADV_PARAMS_T *adv_params, BT_MESH_BEARER_SCAN_PARAMS_T *scan_params)
{
    return 0;
}

VOID linuxbt_mesh_switch_adv(BOOL enable)
{
    return;
}

VOID linuxbt_mesh_log_setlevel(UINT32 level)
{
    return;
}

INT32 linuxbt_mesh_generate_uuid(UINT8 *uuid_buf)
{
    return 0;
}

INT32 linuxbt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info)
{
    return 0;
}

INT32 linuxbt_mesh_set_mesh_mode(BT_MESH_MODE_T mode)
{
    return 0;
}

INT32 linuxbt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout)
{
    return 0;
}

INT32 linuxbt_mesh_ota_initiator_operation(BT_MESH_OTA_OPERATION_PARAMS_T *params)
{
    return 0;
}

INT32 linuxbt_mesh_ota_get_client_model_handle(UINT16 *dist_client, UINT16 *update_client)
{
    return 0;
}

INT32 linuxbt_mesh_set_special_pkt_params(BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params)
{
    return 0;
}

#ifdef MTK_GATT_BEARER_SUPPORT
INT32 linuxbt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type)
{
    return 0;
}

INT32 linuxbt_mesh_gatt_disconnect(VOID)
{
    return 0;
}
#endif

#endif  /*ENABLE_BLE_MESH*/


