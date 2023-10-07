/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jul 29 14:14:36 2021'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_MESH_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_MESH_IPCRPC_STRUCT__H_




/* Start of header pre-amble file 'preamble_file.h'. */

#include "u_rpc.h"


/* End of header pre-amble file 'preamble_file.h'. */


#define RPC_DESC_BTMW_LE_FEATURES  (__rpc_get_mesh_desc__ (0))


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_mesh_desc__ (1))


#define RPC_DESC_BT_MESH_PROV_CAPABILITIES_T  (__rpc_get_mesh_desc__ (2))


#define RPC_DESC_BT_MESH_EVT_PROV_CAPABILITIES_T  (__rpc_get_mesh_desc__ (3))


#define RPC_DESC_BT_MESH_EVT_PROV_REQUEST_AUTH_T  (__rpc_get_mesh_desc__ (4))


#define RPC_DESC_BT_MESH_EVT_PROV_SHOW_PK_T  (__rpc_get_mesh_desc__ (5))


#define RPC_DESC_BT_MESH_EVT_PROV_SHOW_AUTH_T  (__rpc_get_mesh_desc__ (6))


#define RPC_DESC_BT_MESH_EVT_PROV_DONE_T  (__rpc_get_mesh_desc__ (7))


#define RPC_DESC_BT_MESH_PROV_UNPROVISIONED_DEVICE_T  (__rpc_get_mesh_desc__ (8))


#define RPC_DESC_BT_MESH_EVT_PROV_SCAN_UD_T  (__rpc_get_mesh_desc__ (9))


#define RPC_DESC_BT_MESH_PROV_FACTOR_T  (__rpc_get_mesh_desc__ (10))


#define RPC_DESC_BT_MESH_BLE_ADDR_T  (__rpc_get_mesh_desc__ (11))


#define RPC_DESC_BT_MESH_EVT_ADV_REPORT_T  (__rpc_get_mesh_desc__ (12))


#define RPC_DESC_BT_MESH_EVT_KEY_REFRESH_T  (__rpc_get_mesh_desc__ (13))


#define RPC_DESC_BT_MESH_EVT_IV_UPDATE_T  (__rpc_get_mesh_desc__ (14))


#define RPC_DESC_BT_MESH_EVT_SEQ_CHANGE_T  (__rpc_get_mesh_desc__ (15))


#define RPC_DESC_BT_MESH_EVT_HEARTBEAT_T  (__rpc_get_mesh_desc__ (16))


#define RPC_DESC_BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T  (__rpc_get_mesh_desc__ (17))


#define RPC_DESC_BT_MESH_EVT_OTA_T  (__rpc_get_mesh_desc__ (18))


#define RPC_DESC_BT_MESH_EVT_FRIDSHIP_STATUS_T  (__rpc_get_mesh_desc__ (19))


#define RPC_DESC_BT_MESH_EVT_BEARER_GATT_STATUS_T  (__rpc_get_mesh_desc__ (20))


#define RPC_DESC_BT_MESH_EVT_ERROR_CODE_T  (__rpc_get_mesh_desc__ (21))


#define RPC_DESC_BT_MESH_EVT_T_mesh  (__rpc_get_mesh_desc__ (22))


#define RPC_DESC_BT_MESH_EVT_T  (__rpc_get_mesh_desc__ (23))


#define RPC_DESC_BT_MESH_BT_EVENT_T_evt  (__rpc_get_mesh_desc__ (24))


#define RPC_DESC_BT_MESH_BT_EVENT_T  (__rpc_get_mesh_desc__ (25))


#define RPC_DESC_BT_MESH_ADDRESS_T  (__rpc_get_mesh_desc__ (26))


#define RPC_DESC_BT_MESH_SECURITY_T  (__rpc_get_mesh_desc__ (27))


#define RPC_DESC_BT_MESH_TX_PARAMS_T  (__rpc_get_mesh_desc__ (28))


#define RPC_DESC_BT_MESH_ACCESS_OPCODE_T  (__rpc_get_mesh_desc__ (29))


#define RPC_DESC_BT_MESH_ACCESS_MESSAGE_RX_META_T  (__rpc_get_mesh_desc__ (30))


#define RPC_DESC_BT_MESH_ACCESS_MESSAGE_RX_T  (__rpc_get_mesh_desc__ (31))


#define RPC_DESC_BT_MESH_CBK_ACCESS_MSG_T  (__rpc_get_mesh_desc__ (32))


#define RPC_DESC_BT_MESH_ACCESS_OPCODE_HANDLER_T  (__rpc_get_mesh_desc__ (33))


#define RPC_DESC_BT_MESH_MODEL_ADD_PARAMS_T  (__rpc_get_mesh_desc__ (34))


#define RPC_DESC_BT_MESH_PROV_PROVISIONEE_PARAMS_T  (__rpc_get_mesh_desc__ (35))


#define RPC_DESC_BT_MESH_CONFIG_INIT_PARAMS_T  (__rpc_get_mesh_desc__ (36))


#define RPC_DESC_BT_MESH_FRIEND_INIT_PARAMS_T  (__rpc_get_mesh_desc__ (37))


#define RPC_DESC_BT_MESH_DEBUG_INIT_PARAMS_T  (__rpc_get_mesh_desc__ (38))


#define RPC_DESC_BT_MESH_CUSTOMIZE_PARA_T  (__rpc_get_mesh_desc__ (39))


#define RPC_DESC_BT_MESH_INIT_PARAMS_T  (__rpc_get_mesh_desc__ (40))


#define RPC_DESC_BT_MESH_PROV_INVITE_T  (__rpc_get_mesh_desc__ (41))


#define RPC_DESC_BT_MESH_PROV_START_T  (__rpc_get_mesh_desc__ (42))


#define RPC_DESC_BT_MESH_PROV_DATA_T  (__rpc_get_mesh_desc__ (43))


#define RPC_DESC_BT_MESH_PROV_PROVISIONER_PARAMS_T  (__rpc_get_mesh_desc__ (44))


#define RPC_DESC_BT_MESH_MODEL_PUBLICATION_STATE_T  (__rpc_get_mesh_desc__ (45))


#define RPC_DESC_BT_MESH_HEARTBEAT_PUBLICATION_T  (__rpc_get_mesh_desc__ (46))


#define RPC_DESC_BT_MESH_HEARTBEAT_SUBSCRIPTION_T  (__rpc_get_mesh_desc__ (47))


#define RPC_DESC_BT_MESH_NETKEY_FLASH_DATA_T  (__rpc_get_mesh_desc__ (48))


#define RPC_DESC_BT_MESH_FRIEND_FLASH_DATA_T  (__rpc_get_mesh_desc__ (49))


#define RPC_DESC_BT_MESH_APPKEY_FLASH_DATA_T  (__rpc_get_mesh_desc__ (50))


#define RPC_DESC_BT_MESH_MODEL_FLASH_DATA_T  (__rpc_get_mesh_desc__ (51))


#define RPC_DESC_BT_MESH_BLE_MESH_MODEL_PUBLICATION_T  (__rpc_get_mesh_desc__ (52))


#define RPC_DESC_BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T  (__rpc_get_mesh_desc__ (53))


#define RPC_DESC_BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T  (__rpc_get_mesh_desc__ (54))


#define RPC_DESC_BT_MESH_DEVICE_FLASH_DATA_T  (__rpc_get_mesh_desc__ (55))


#define RPC_DESC_BT_MESH_NETWORK_TRANSMIT_T  (__rpc_get_mesh_desc__ (56))


#define RPC_DESC_BT_MESH_RELAY_TRANSMIT_T  (__rpc_get_mesh_desc__ (57))


#define RPC_DESC_BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T  (__rpc_get_mesh_desc__ (58))


#define RPC_DESC_BT_MESH_HEALTH_PERIOD_T  (__rpc_get_mesh_desc__ (59))


#define RPC_DESC_BT_MESH_HEALTH_SERVER_FLASH_DATA_T  (__rpc_get_mesh_desc__ (60))


#define RPC_DESC_BT_MESH_SEQNUM_FLASH_DATA_T  (__rpc_get_mesh_desc__ (61))


#define RPC_DESC_BT_MESH_RECORD_T  (__rpc_get_mesh_desc__ (62))


#define RPC_DESC_BT_MESH_CONFIG_META_T  (__rpc_get_mesh_desc__ (63))


#define RPC_DESC_BT_MESH_CONFIG_BEACON_GET_T  (__rpc_get_mesh_desc__ (64))


#define RPC_DESC_BT_MESH_CONFIG_DEFAULT_TTL_GET_T  (__rpc_get_mesh_desc__ (65))


#define RPC_DESC_BT_MESH_CONFIG_GATT_PROXY_GET_T  (__rpc_get_mesh_desc__ (66))


#define RPC_DESC_BT_MESH_CONFIG_FRIEND_GET_T  (__rpc_get_mesh_desc__ (67))


#define RPC_DESC_BT_MESH_CONFIG_RELAY_GET_T  (__rpc_get_mesh_desc__ (68))


#define RPC_DESC_BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T  (__rpc_get_mesh_desc__ (69))


#define RPC_DESC_BT_MESH_CONFIG_NETKEY_GET_T  (__rpc_get_mesh_desc__ (70))


#define RPC_DESC_BT_MESH_CONFIG_NODE_RESET_T  (__rpc_get_mesh_desc__ (71))


#define RPC_DESC_BT_MESH_CONFIG_HB_PUB_GET_T  (__rpc_get_mesh_desc__ (72))


#define RPC_DESC_BT_MESH_CONFIG_HB_SUB_GET_T  (__rpc_get_mesh_desc__ (73))


#define RPC_DESC_BT_MESH_CONFIG_BEACON_SET_T  (__rpc_get_mesh_desc__ (74))


#define RPC_DESC_BT_MESH_CONFIG_COMPOSITION_DATA_GET_T  (__rpc_get_mesh_desc__ (75))


#define RPC_DESC_BT_MESH_CONFIG_DEFAULT_TTL_SET_T  (__rpc_get_mesh_desc__ (76))


#define RPC_DESC_BT_MESH_CONFIG_GATT_PROXY_SET_T  (__rpc_get_mesh_desc__ (77))


#define RPC_DESC_BT_MESH_CONFIG_FRIEND_SET_T  (__rpc_get_mesh_desc__ (78))


#define RPC_DESC_BT_MESH_CONFIG_RELAY_SET_T  (__rpc_get_mesh_desc__ (79))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_PUB_GET_T  (__rpc_get_mesh_desc__ (80))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_PUB_SET_T  (__rpc_get_mesh_desc__ (81))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_SUB_ADD_T  (__rpc_get_mesh_desc__ (82))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_SUB_DEL_T  (__rpc_get_mesh_desc__ (83))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_SUB_OW_T  (__rpc_get_mesh_desc__ (84))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T  (__rpc_get_mesh_desc__ (85))


#define RPC_DESC_BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T  (__rpc_get_mesh_desc__ (86))


#define RPC_DESC_BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T  (__rpc_get_mesh_desc__ (87))


#define RPC_DESC_BT_MESH_CONFIG_NETKEY_ADD_T  (__rpc_get_mesh_desc__ (88))


#define RPC_DESC_BT_MESH_CONFIG_NETKEY_UPDATE_T  (__rpc_get_mesh_desc__ (89))


#define RPC_DESC_BT_MESH_CONFIG_NETKEY_DEL_T  (__rpc_get_mesh_desc__ (90))


#define RPC_DESC_BT_MESH_CONFIG_APPKEY_ADD_T  (__rpc_get_mesh_desc__ (91))


#define RPC_DESC_BT_MESH_CONFIG_APPKEY_UPDATE_T  (__rpc_get_mesh_desc__ (92))


#define RPC_DESC_BT_MESH_CONFIG_APPKEY_DEL_T  (__rpc_get_mesh_desc__ (93))


#define RPC_DESC_BT_MESH_CONFIG_APPKEY_GET_T  (__rpc_get_mesh_desc__ (94))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_APP_BIND_T  (__rpc_get_mesh_desc__ (95))


#define RPC_DESC_BT_MESH_CONFIG_MODEL_APP_UNBIND_T  (__rpc_get_mesh_desc__ (96))


#define RPC_DESC_BT_MESH_CONFIG_SIG_MODEL_APP_GET_T  (__rpc_get_mesh_desc__ (97))


#define RPC_DESC_BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T  (__rpc_get_mesh_desc__ (98))


#define RPC_DESC_BT_MESH_CONFIG_NODE_IDENTITY_GET_T  (__rpc_get_mesh_desc__ (99))


#define RPC_DESC_BT_MESH_CONFIG_NODE_IDENTITY_SET_T  (__rpc_get_mesh_desc__ (100))


#define RPC_DESC_BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T  (__rpc_get_mesh_desc__ (101))


#define RPC_DESC_BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T  (__rpc_get_mesh_desc__ (102))


#define RPC_DESC_BT_MESH_CONFIG_HB_PUB_SET_T  (__rpc_get_mesh_desc__ (103))


#define RPC_DESC_BT_MESH_CONFIG_HB_SUB_SET_T  (__rpc_get_mesh_desc__ (104))


#define RPC_DESC_BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T  (__rpc_get_mesh_desc__ (105))


#define RPC_DESC_BT_MESH_CONFIGURATION_MSG_TX_T_data  (__rpc_get_mesh_desc__ (106))


#define RPC_DESC_BT_MESH_CONFIGURATION_MSG_TX_T  (__rpc_get_mesh_desc__ (107))


#define RPC_DESC_BT_MESH_COMPOSITION_DATA_T  (__rpc_get_mesh_desc__ (108))


#define RPC_DESC_BT_MESH_ELEMENT_DATA_T  (__rpc_get_mesh_desc__ (109))


#define RPC_DESC_BT_MESH_HEALTH_SERVER_DATA_T  (__rpc_get_mesh_desc__ (110))


#define RPC_DESC_BT_MESH_CONFIGURATION_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (111))


#define RPC_DESC_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T  (__rpc_get_mesh_desc__ (112))


#define RPC_DESC_BT_MESH_CTL_SETUP_SERVER_DATA_T  (__rpc_get_mesh_desc__ (113))


#define RPC_DESC_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (114))


#define RPC_DESC_BT_MESH_LIGHTNESS_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (115))


#define RPC_DESC_BT_MESH_CTL_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (116))


#define RPC_DESC_BT_MESH_HSL_SETUP_SERVER_DATA_T  (__rpc_get_mesh_desc__ (117))


#define RPC_DESC_BT_MESH_HSL_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (118))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T  (__rpc_get_mesh_desc__ (119))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T  (__rpc_get_mesh_desc__ (120))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T  (__rpc_get_mesh_desc__ (121))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T  (__rpc_get_mesh_desc__ (122))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_T_data  (__rpc_get_mesh_desc__ (123))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_T  (__rpc_get_mesh_desc__ (124))


#define RPC_DESC_BT_MESH_CBK_HEALTH_CLIENT_EVT_T  (__rpc_get_mesh_desc__ (125))


#define RPC_DESC_BT_MESH_HEALTH_CLIENT_DATA_T  (__rpc_get_mesh_desc__ (126))


#define RPC_DESC_BT_MESH_MODEL_T  (__rpc_get_mesh_desc__ (127))


#define RPC_DESC_BT_MESH_ELEMENT_ADDR_T  (__rpc_get_mesh_desc__ (128))


#define RPC_DESC_BT_MESH_CONFIGURATION_SERVER_DATA_T  (__rpc_get_mesh_desc__ (129))


#define RPC_DESC_BT_MESH_MODEL_DATA_T_data  (__rpc_get_mesh_desc__ (130))


#define RPC_DESC_BT_MESH_MODEL_DATA_T  (__rpc_get_mesh_desc__ (131))


#define RPC_DESC_BT_MESH_NETKEY_T  (__rpc_get_mesh_desc__ (132))


#define RPC_DESC_BT_MESH_APPKEY_T  (__rpc_get_mesh_desc__ (133))


#define RPC_DESC_BT_MESH_ACCESS_MESSAGE_TX_T  (__rpc_get_mesh_desc__ (134))


#define RPC_DESC_BT_MESH_BEARER_ADV_PARAMS_T  (__rpc_get_mesh_desc__ (135))


#define RPC_DESC_BT_MESH_BEARER_SCAN_PARAMS_T  (__rpc_get_mesh_desc__ (136))


#define RPC_DESC_BT_APP_MESH_CB_FUNC_T  (__rpc_get_mesh_desc__ (137))


#define RPC_DESC_BT_MESH_COMP_DATA_T  (__rpc_get_mesh_desc__ (138))


#define RPC_DESC_BT_MESH_COMP_ELEMENT_T  (__rpc_get_mesh_desc__ (139))


#define RPC_DESC_BT_MESH_MODEL_OPERATION_T  (__rpc_get_mesh_desc__ (140))


#define RPC_DESC_BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T  (__rpc_get_mesh_desc__ (141))


#define RPC_DESC_BT_MESH_DEVKEY_INFO_T  (__rpc_get_mesh_desc__ (142))


#define RPC_DESC_BT_MESH_DEVICE_INFO_T  (__rpc_get_mesh_desc__ (143))


#define RPC_DESC_BT_MESH_OTA_INITIATOR_MSG_HANDLER_T  (__rpc_get_mesh_desc__ (144))


#define RPC_DESC_BT_MESH_OTA_INITIATOR_FW_INFO_GET_T  (__rpc_get_mesh_desc__ (145))


#define RPC_DESC_BT_MESH_OTA_INITIATOR_START_PARAMS_T  (__rpc_get_mesh_desc__ (146))


#define RPC_DESC_BT_MESH_OTA_INITIATOR_STOP_PARAMS_T  (__rpc_get_mesh_desc__ (147))


#define RPC_DESC_BT_MESH_OTA_OPERATION_PARAMS_T_params  (__rpc_get_mesh_desc__ (148))


#define RPC_DESC_BT_MESH_OTA_OPERATION_PARAMS_T  (__rpc_get_mesh_desc__ (149))


#define RPC_DESC_BT_MESH_SPECIAL_PKT_PARAMS_T  (__rpc_get_mesh_desc__ (150))


#define RPC_DESC_MTKRPCAPI_BT_APP_MESH_CB_FUNC_T  (__rpc_get_mesh_desc__ (151))


#define RPC_DESC_MTKRPCAPI_BT_MESH_CB_FUNC_T  (__rpc_get_mesh_desc__ (152))



extern const RPC_DESC_T* __rpc_get_mesh_desc__ (UINT32  ui4_idx);


#endif

