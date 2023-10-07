/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:14:36 2021'. */
/* Do NOT modify this source file. */



/* Start of source pre-amble file 'src_header_file.h'. */

#include "u_bt_mw_mesh.h"
#include "mtk_bt_service_mesh_wrapper.h"
#include "mtk_bt_service_mesh_ipcrpc_struct.h"



/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES;
static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_CAPABILITIES_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_CAPABILITIES_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_REQUEST_AUTH_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SHOW_PK_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SHOW_AUTH_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_DONE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_UNPROVISIONED_DEVICE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SCAN_UD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_FACTOR_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BLE_ADDR_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_ADV_REPORT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_KEY_REFRESH_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_IV_UPDATE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_SEQ_CHANGE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_HEARTBEAT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_OTA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_FRIDSHIP_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_BEARER_GATT_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_ERROR_CODE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_T_mesh;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BT_EVENT_T_evt;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BT_EVENT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ADDRESS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_SECURITY_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_TX_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_OPCODE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_META_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CBK_ACCESS_MSG_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_OPCODE_HANDLER_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_ADD_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_PROVISIONEE_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_INIT_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_FRIEND_INIT_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_DEBUG_INIT_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CUSTOMIZE_PARA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_INIT_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_INVITE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_START_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_PROVISIONER_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_PUBLICATION_STATE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEARTBEAT_PUBLICATION_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEARTBEAT_SUBSCRIPTION_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_NETKEY_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_FRIEND_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_APPKEY_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BLE_MESH_MODEL_PUBLICATION_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVICE_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_NETWORK_TRANSMIT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_RELAY_TRANSMIT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_PERIOD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_SERVER_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_SEQNUM_FLASH_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_RECORD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_META_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_BEACON_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_FRIEND_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_RELAY_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_RESET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_PUB_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_SUB_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_BEACON_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_COMPOSITION_DATA_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_FRIEND_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_RELAY_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_ADD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_OW_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_ADD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_UPDATE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_DEL_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_ADD_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_UPDATE_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_DEL_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_BIND_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_UNBIND_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_APP_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_PUB_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_SUB_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T_data;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_COMPOSITION_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ELEMENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_SERVER_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CTL_SETUP_SERVER_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_LIGHTNESS_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CTL_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HSL_SETUP_SERVER_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HSL_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T_data;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CBK_HEALTH_CLIENT_EVT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ELEMENT_ADDR_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_DATA_T_data;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_NETKEY_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_APPKEY_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_TX_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BEARER_ADV_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_BEARER_SCAN_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_APP_MESH_CB_FUNC_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_COMP_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_COMP_ELEMENT_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_OPERATION_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVKEY_INFO_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVICE_INFO_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_MSG_HANDLER_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_FW_INFO_GET_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_START_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_STOP_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T_params;
static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_MESH_SPECIAL_PKT_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_MESH_CB_FUNC_T;
static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_MESH_CB_FUNC_T;



static const RPC_DESC_T t_rpc_decl_BTMW_LE_FEATURES =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BTMW_LE_FEATURES),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_CAPABILITIES_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_CAPABILITIES_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_CAPABILITIES_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_CAPABILITIES_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_REQUEST_AUTH_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_REQUEST_AUTH_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SHOW_PK_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_SHOW_PK_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_EVT_PROV_SHOW_PK_T, pk)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SHOW_AUTH_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_SHOW_AUTH_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_DONE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_DONE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_UNPROVISIONED_DEVICE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_UNPROVISIONED_DEVICE_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_PROV_UNPROVISIONED_DEVICE_T, uri_hash)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_PROV_SCAN_UD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_PROV_SCAN_UD_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_PROV_UNPROVISIONED_DEVICE_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_EVT_PROV_SCAN_UD_T, ud)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_FACTOR_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_FACTOR_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_PROV_FACTOR_T, buf)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BLE_ADDR_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_BLE_ADDR_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_ADV_REPORT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_ADV_REPORT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_KEY_REFRESH_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_KEY_REFRESH_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_IV_UPDATE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_IV_UPDATE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_SEQ_CHANGE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_SEQ_CHANGE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_HEARTBEAT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_HEARTBEAT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_OTA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_OTA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_EVT_OTA_T, nodes_status)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_FRIDSHIP_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_FRIDSHIP_STATUS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_BEARER_GATT_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_BEARER_GATT_STATUS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_ERROR_CODE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_ERROR_CODE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_T_mesh =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_EVT_T*) NULL)->mesh),
    .ui4_num_entries = 4,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_EVT_PROV_SHOW_PK_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "mesh.prov_show_pk"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_EVT_PROV_SCAN_UD_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "mesh.prov_scan_ud"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_PROV_FACTOR_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "mesh.prov_factor"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_EVT_OTA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "mesh.ota_evt"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EVT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EVT_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_EVT_T_mesh,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_EVT_T, mesh)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BT_EVENT_T_evt =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_BT_EVENT_T*) NULL)->evt),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_EVT_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "evt.mesh_evt"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BT_EVENT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_BT_EVENT_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_BT_EVENT_T_evt,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_BT_EVENT_T, evt)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ADDRESS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ADDRESS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC | ARG_DIR_INP,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_ADDRESS_T, virtual_uuid)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_SECURITY_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_SECURITY_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_SECURITY_T, device_key)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_TX_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_TX_PARAMS_T),
    .ui4_num_entries = 3,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ADDRESS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_TX_PARAMS_T, dst)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC | ARG_DIR_INP,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_TX_PARAMS_T, data)
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_SECURITY_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_TX_PARAMS_T, security)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_OPCODE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ACCESS_OPCODE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_META_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ACCESS_MESSAGE_RX_META_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ACCESS_MESSAGE_RX_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_ACCESS_MESSAGE_RX_T, buf)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CBK_ACCESS_MSG_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CBK_ACCESS_MSG_T),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CBK_ACCESS_MSG_T, msg)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CBK_ACCESS_MSG_T, arg)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_OPCODE_HANDLER_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ACCESS_OPCODE_HANDLER_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_ADD_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_ADD_PARAMS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC | ARG_DIR_INP,
            .pt_desc         = &t_rpc_decl_BT_MESH_ACCESS_OPCODE_HANDLER_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_MODEL_ADD_PARAMS_T, opcode_handlers)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_PROVISIONEE_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_PROVISIONEE_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_INIT_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_INIT_PARAMS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC | ARG_DIR_INP,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_INIT_PARAMS_T, uri)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_FRIEND_INIT_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_FRIEND_INIT_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_DEBUG_INIT_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_DEBUG_INIT_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CUSTOMIZE_PARA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CUSTOMIZE_PARA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_INIT_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_INIT_PARAMS_T),
    .ui4_num_entries = 5,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_PROV_PROVISIONEE_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_INIT_PARAMS_T, provisionee)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_INIT_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_INIT_PARAMS_T, config)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_FRIEND_INIT_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_INIT_PARAMS_T, friend_params)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_DEBUG_INIT_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_INIT_PARAMS_T, debug)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CUSTOMIZE_PARA_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_INIT_PARAMS_T, customize_params)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_INVITE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_INVITE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_START_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_START_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_PROV_PROVISIONER_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_PROV_PROVISIONER_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_PUBLICATION_STATE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_PUBLICATION_STATE_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ADDRESS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_MODEL_PUBLICATION_STATE_T, publish_address)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEARTBEAT_PUBLICATION_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEARTBEAT_PUBLICATION_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEARTBEAT_SUBSCRIPTION_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEARTBEAT_SUBSCRIPTION_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_NETKEY_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_NETKEY_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_FRIEND_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_FRIEND_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_APPKEY_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_APPKEY_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BLE_MESH_MODEL_PUBLICATION_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_BLE_MESH_MODEL_PUBLICATION_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVICE_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_DEVICE_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_NETWORK_TRANSMIT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_NETWORK_TRANSMIT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_RELAY_TRANSMIT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_RELAY_TRANSMIT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_PERIOD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_PERIOD_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_SERVER_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_SERVER_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_SEQNUM_FLASH_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_SEQNUM_FLASH_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_RECORD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_RECORD_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_META_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_META_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_BEACON_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_BEACON_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_DEFAULT_TTL_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_GATT_PROXY_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_FRIEND_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_FRIEND_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_RELAY_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_RELAY_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETKEY_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_RESET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NODE_RESET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_PUB_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_HB_PUB_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_SUB_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_HB_SUB_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_BEACON_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_BEACON_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_COMPOSITION_DATA_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_COMPOSITION_DATA_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_DEFAULT_TTL_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_GATT_PROXY_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_FRIEND_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_FRIEND_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_RELAY_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_RELAY_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_PUB_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_PUB_SET_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_MODEL_PUBLICATION_STATE_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_MODEL_PUB_SET_T, state)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_ADD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_SUB_ADD_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ADDRESS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_MODEL_SUB_ADD_T, address)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_SUB_DEL_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ADDRESS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_MODEL_SUB_DEL_T, address)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_OW_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_SUB_OW_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ADDRESS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_MODEL_SUB_OW_T, address)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_ADD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETKEY_ADD_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_NETKEY_ADD_T, netkey)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_UPDATE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETKEY_UPDATE_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_NETKEY_UPDATE_T, netkey)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETKEY_DEL_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETKEY_DEL_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_ADD_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_APPKEY_ADD_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_APPKEY_ADD_T, appkey)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_UPDATE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_APPKEY_UPDATE_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_APPKEY_UPDATE_T, appkey)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_DEL_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_APPKEY_DEL_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_APPKEY_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_APPKEY_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_BIND_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_APP_BIND_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_UNBIND_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_MODEL_APP_UNBIND_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_APP_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_SIG_MODEL_APP_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NODE_IDENTITY_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NODE_IDENTITY_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_PUB_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_HB_PUB_SET_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEARTBEAT_PUBLICATION_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_HB_PUB_SET_T, publication)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_HB_SUB_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_HB_SUB_SET_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEARTBEAT_SUBSCRIPTION_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIG_HB_SUB_SET_T, subscription)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T_data =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_CONFIGURATION_MSG_TX_T*) NULL)->data),
    .ui4_num_entries = 10,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_SET_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.model_pub_set"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_ADD_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.model_sub_add"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.model_sub_del"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_OW_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.model_sub_ow"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_NETKEY_ADD_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.netkey_add"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_NETKEY_UPDATE_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.netkey_update"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_APPKEY_ADD_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.appkey_add"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_APPKEY_UPDATE_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.appkey_update"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_HB_PUB_SET_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.hb_pub_set"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIG_HB_SUB_SET_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.hb_sub_set"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIGURATION_MSG_TX_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T_data,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIGURATION_MSG_TX_T, data)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_COMPOSITION_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_COMPOSITION_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_COMPOSITION_DATA_T, buf)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ELEMENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ELEMENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_ELEMENT_DATA_T, element_index)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_SERVER_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_SERVER_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HEALTH_SERVER_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIGURATION_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIGURATION_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_GENERIC_ONOFF_SERVER_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_GENERIC_ONOFF_SERVER_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CTL_SETUP_SERVER_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CTL_SETUP_SERVER_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CTL_SETUP_SERVER_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_LIGHTNESS_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_LIGHTNESS_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_LIGHTNESS_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CTL_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CTL_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CTL_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HSL_SETUP_SERVER_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HSL_SETUP_SERVER_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HSL_SETUP_SERVER_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HSL_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HSL_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HSL_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T, fault_array)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T, fault_array)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T_data =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_HEALTH_CLIENT_EVT_T*) NULL)->data),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.current_status"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.fault_status"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_EVT_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T_data,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HEALTH_CLIENT_EVT_T, data)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CBK_HEALTH_CLIENT_EVT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CBK_HEALTH_CLIENT_EVT_T),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CBK_HEALTH_CLIENT_EVT_T, msg)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CBK_HEALTH_CLIENT_EVT_T, event)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_HEALTH_CLIENT_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_HEALTH_CLIENT_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_HEALTH_CLIENT_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_T),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_MODEL_T, model_handle)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_MODEL_ADD_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_MODEL_T, model_params)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ELEMENT_ADDR_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ELEMENT_ADDR_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_CONFIGURATION_SERVER_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_CONFIGURATION_SERVER_DATA_T, model_handle)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_DATA_T_data =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_MODEL_DATA_T*) NULL)->data),
    .ui4_num_entries = 14,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_COMPOSITION_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.composition_data"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_ELEMENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.element"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.configuration_server"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CONFIGURATION_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.configuration_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_SERVER_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.health_server"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HEALTH_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.health_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.generic_onoff_server"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CTL_SETUP_SERVER_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.ctl_setup_server"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.generic_power_onoff_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_LIGHTNESS_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.lightness_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_CTL_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.ctl_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HSL_SETUP_SERVER_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.hsl_setup_server"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_HSL_CLIENT_DATA_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.hsl_client"
            }
        },
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_MODEL_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "data.model_data"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_MODEL_DATA_T_data,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_MODEL_DATA_T, data)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_NETKEY_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_NETKEY_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_NETKEY_T, network_key)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_APPKEY_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_APPKEY_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_APPKEY_T, application_key)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_ACCESS_MESSAGE_TX_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_ACCESS_MESSAGE_TX_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_ACCESS_MESSAGE_TX_T, p_buffer)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BEARER_ADV_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_BEARER_ADV_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_BEARER_SCAN_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_BEARER_SCAN_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_APP_MESH_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_APP_MESH_CB_FUNC_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_COMP_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_COMP_DATA_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_COMP_DATA_T, elements)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_COMP_ELEMENT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_COMP_ELEMENT_T),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_COMP_ELEMENT_T, sig_models)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_COMP_ELEMENT_T, vendor_models)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_MODEL_OPERATION_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_MODEL_OPERATION_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVKEY_INFO_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_DEVKEY_INFO_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_DEVICE_INFO_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_DEVICE_INFO_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_MSG_HANDLER_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_INITIATOR_MSG_HANDLER_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_FW_INFO_GET_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_INITIATOR_FW_INFO_GET_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_START_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_INITIATOR_START_PARAMS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_OTA_INITIATOR_START_PARAMS_T, updaters)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_INITIATOR_STOP_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_INITIATOR_STOP_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T_params =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (((BT_MESH_OTA_OPERATION_PARAMS_T*) NULL)->params),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_OTA_INITIATOR_START_PARAMS_T,
            .ui4_num_entries = 1,

            {
                .ps_field_name = "params.start_params"
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_OTA_OPERATION_PARAMS_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_DESC,
            .pt_desc         = &t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T_params,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_MESH_OTA_OPERATION_PARAMS_T, params)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_MESH_SPECIAL_PKT_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_MESH_SPECIAL_PKT_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_MESH_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_MESH_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (MTKRPCAPI_BT_MESH_CB_FUNC_T),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BTMW_LE_FEATURES,
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_MESH_PROV_CAPABILITIES_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_CAPABILITIES_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_REQUEST_AUTH_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_SHOW_PK_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_SHOW_AUTH_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_DONE_T,
    &t_rpc_decl_BT_MESH_PROV_UNPROVISIONED_DEVICE_T,
    &t_rpc_decl_BT_MESH_EVT_PROV_SCAN_UD_T,
    &t_rpc_decl_BT_MESH_PROV_FACTOR_T,
    &t_rpc_decl_BT_MESH_BLE_ADDR_T,
    &t_rpc_decl_BT_MESH_EVT_ADV_REPORT_T,
    &t_rpc_decl_BT_MESH_EVT_KEY_REFRESH_T,
    &t_rpc_decl_BT_MESH_EVT_IV_UPDATE_T,
    &t_rpc_decl_BT_MESH_EVT_SEQ_CHANGE_T,
    &t_rpc_decl_BT_MESH_EVT_HEARTBEAT_T,
    &t_rpc_decl_BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T,
    &t_rpc_decl_BT_MESH_EVT_OTA_T,
    &t_rpc_decl_BT_MESH_EVT_FRIDSHIP_STATUS_T,
    &t_rpc_decl_BT_MESH_EVT_BEARER_GATT_STATUS_T,
    &t_rpc_decl_BT_MESH_EVT_ERROR_CODE_T,
    &t_rpc_decl_BT_MESH_EVT_T_mesh,
    &t_rpc_decl_BT_MESH_EVT_T,
    &t_rpc_decl_BT_MESH_BT_EVENT_T_evt,
    &t_rpc_decl_BT_MESH_BT_EVENT_T,
    &t_rpc_decl_BT_MESH_ADDRESS_T,
    &t_rpc_decl_BT_MESH_SECURITY_T,
    &t_rpc_decl_BT_MESH_TX_PARAMS_T,
    &t_rpc_decl_BT_MESH_ACCESS_OPCODE_T,
    &t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_META_T,
    &t_rpc_decl_BT_MESH_ACCESS_MESSAGE_RX_T,
    &t_rpc_decl_BT_MESH_CBK_ACCESS_MSG_T,
    &t_rpc_decl_BT_MESH_ACCESS_OPCODE_HANDLER_T,
    &t_rpc_decl_BT_MESH_MODEL_ADD_PARAMS_T,
    &t_rpc_decl_BT_MESH_PROV_PROVISIONEE_PARAMS_T,
    &t_rpc_decl_BT_MESH_CONFIG_INIT_PARAMS_T,
    &t_rpc_decl_BT_MESH_FRIEND_INIT_PARAMS_T,
    &t_rpc_decl_BT_MESH_DEBUG_INIT_PARAMS_T,
    &t_rpc_decl_BT_MESH_CUSTOMIZE_PARA_T,
    &t_rpc_decl_BT_MESH_INIT_PARAMS_T,
    &t_rpc_decl_BT_MESH_PROV_INVITE_T,
    &t_rpc_decl_BT_MESH_PROV_START_T,
    &t_rpc_decl_BT_MESH_PROV_DATA_T,
    &t_rpc_decl_BT_MESH_PROV_PROVISIONER_PARAMS_T,
    &t_rpc_decl_BT_MESH_MODEL_PUBLICATION_STATE_T,
    &t_rpc_decl_BT_MESH_HEARTBEAT_PUBLICATION_T,
    &t_rpc_decl_BT_MESH_HEARTBEAT_SUBSCRIPTION_T,
    &t_rpc_decl_BT_MESH_NETKEY_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_FRIEND_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_APPKEY_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_MODEL_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_BLE_MESH_MODEL_PUBLICATION_T,
    &t_rpc_decl_BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T,
    &t_rpc_decl_BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_DEVICE_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_NETWORK_TRANSMIT_T,
    &t_rpc_decl_BT_MESH_RELAY_TRANSMIT_T,
    &t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_HEALTH_PERIOD_T,
    &t_rpc_decl_BT_MESH_HEALTH_SERVER_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_SEQNUM_FLASH_DATA_T,
    &t_rpc_decl_BT_MESH_RECORD_T,
    &t_rpc_decl_BT_MESH_CONFIG_META_T,
    &t_rpc_decl_BT_MESH_CONFIG_BEACON_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_FRIEND_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_RELAY_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETKEY_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NODE_RESET_T,
    &t_rpc_decl_BT_MESH_CONFIG_HB_PUB_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_HB_SUB_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_BEACON_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_COMPOSITION_DATA_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_DEFAULT_TTL_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_GATT_PROXY_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_FRIEND_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_RELAY_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_PUB_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_ADD_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_OW_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T,
    &t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETKEY_ADD_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETKEY_UPDATE_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETKEY_DEL_T,
    &t_rpc_decl_BT_MESH_CONFIG_APPKEY_ADD_T,
    &t_rpc_decl_BT_MESH_CONFIG_APPKEY_UPDATE_T,
    &t_rpc_decl_BT_MESH_CONFIG_APPKEY_DEL_T,
    &t_rpc_decl_BT_MESH_CONFIG_APPKEY_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_BIND_T,
    &t_rpc_decl_BT_MESH_CONFIG_MODEL_APP_UNBIND_T,
    &t_rpc_decl_BT_MESH_CONFIG_SIG_MODEL_APP_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NODE_IDENTITY_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T,
    &t_rpc_decl_BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_HB_PUB_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_HB_SUB_SET_T,
    &t_rpc_decl_BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T,
    &t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T_data,
    &t_rpc_decl_BT_MESH_CONFIGURATION_MSG_TX_T,
    &t_rpc_decl_BT_MESH_COMPOSITION_DATA_T,
    &t_rpc_decl_BT_MESH_ELEMENT_DATA_T,
    &t_rpc_decl_BT_MESH_HEALTH_SERVER_DATA_T,
    &t_rpc_decl_BT_MESH_CONFIGURATION_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T,
    &t_rpc_decl_BT_MESH_CTL_SETUP_SERVER_DATA_T,
    &t_rpc_decl_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_LIGHTNESS_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_CTL_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_HSL_SETUP_SERVER_DATA_T,
    &t_rpc_decl_BT_MESH_HSL_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T_data,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_EVT_T,
    &t_rpc_decl_BT_MESH_CBK_HEALTH_CLIENT_EVT_T,
    &t_rpc_decl_BT_MESH_HEALTH_CLIENT_DATA_T,
    &t_rpc_decl_BT_MESH_MODEL_T,
    &t_rpc_decl_BT_MESH_ELEMENT_ADDR_T,
    &t_rpc_decl_BT_MESH_CONFIGURATION_SERVER_DATA_T,
    &t_rpc_decl_BT_MESH_MODEL_DATA_T_data,
    &t_rpc_decl_BT_MESH_MODEL_DATA_T,
    &t_rpc_decl_BT_MESH_NETKEY_T,
    &t_rpc_decl_BT_MESH_APPKEY_T,
    &t_rpc_decl_BT_MESH_ACCESS_MESSAGE_TX_T,
    &t_rpc_decl_BT_MESH_BEARER_ADV_PARAMS_T,
    &t_rpc_decl_BT_MESH_BEARER_SCAN_PARAMS_T,
    &t_rpc_decl_BT_APP_MESH_CB_FUNC_T,
    &t_rpc_decl_BT_MESH_COMP_DATA_T,
    &t_rpc_decl_BT_MESH_COMP_ELEMENT_T,
    &t_rpc_decl_BT_MESH_MODEL_OPERATION_T,
    &t_rpc_decl_BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T,
    &t_rpc_decl_BT_MESH_DEVKEY_INFO_T,
    &t_rpc_decl_BT_MESH_DEVICE_INFO_T,
    &t_rpc_decl_BT_MESH_OTA_INITIATOR_MSG_HANDLER_T,
    &t_rpc_decl_BT_MESH_OTA_INITIATOR_FW_INFO_GET_T,
    &t_rpc_decl_BT_MESH_OTA_INITIATOR_START_PARAMS_T,
    &t_rpc_decl_BT_MESH_OTA_INITIATOR_STOP_PARAMS_T,
    &t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T_params,
    &t_rpc_decl_BT_MESH_OTA_OPERATION_PARAMS_T,
    &t_rpc_decl_BT_MESH_SPECIAL_PKT_PARAMS_T,
    &t_rpc_decl_MTKRPCAPI_BT_APP_MESH_CB_FUNC_T,
    &t_rpc_decl_MTKRPCAPI_BT_MESH_CB_FUNC_T
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_mesh_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 153) ? at_rpc_desc_list [ui4_idx] : NULL);
}


