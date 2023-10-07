#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_mesh_if.h"
#include "mtk_bt_service_mesh_wrapper.h"
#include "u_bt_mw_mesh.h"

//mesh header
#include "util_transform.h"
#include "util_dlist.h"
#include "util_queue.h"
#include "util_timer.h"

/**  @brief dump log. */
#define BTMW_RPC_DUMP_PDU(pdu_name, pdu_len, pdu) \
    { \
        UINT8 i; \
        BTMW_RPC_TEST_Logi("%s[%d] :", pdu_name, pdu_len); \
        for(i = 0; i< pdu_len; i++) { \
            BTMW_RPC_TEST_Logi(" %02x", pdu[i]); \
        } \
        BTMW_RPC_TEST_Logi("\n"); \
    }

#define BTMW_RPC_MESH_CHECK_NULL_AND_FREE(value)  \
    if (value != NULL) { \
        free(value); \
        value = NULL; \
    }

#define BTMW_RPC_TEST_MESH_DLIST_ALLOC(list)    \
    do                                                                                      \
    {                                                                                       \
        if (NULL == list)                                                                   \
        {                                                                                   \
            list = (VOID *)util_dlist_alloc();                                              \
            if (NULL == list)                                                               \
            {                                                                               \
                BTMW_RPC_TEST_Loge("[mesh] allocate list fail\n");                          \
                return 0;                                                                   \
            }                                                                               \
            else                                                                            \
            {                                                                               \
                BTMW_RPC_TEST_Logi("[mesh] alloc list: 0x%x, Line %d\n", list, __LINE__);   \
            }                                                                               \
        }                                                                                   \
    }while (0)

#define OTA_FLASH_OFFSET 0x80000

static BTMW_RPC_TEST_MESH_NODE_ENTRY_T g_curr_prov_node;
static pthread_t _resend_msg_thread = 0;
static UINT8 _resend_msg_thread_exit = FALSE;
static pthread_mutex_t resend_msg_mutex;
static VOID *btmw_rpc_test_resend_msg_list = NULL;
static BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *btmw_rpc_test_resend_msg_list_tail = NULL;
static UINT32 g_resend_msg_id = 0;
static UINT8 g_tid = 1;
static BOOL fg_pre_init = FALSE;
static BOOL fg_init = FALSE;
static UINT8 g_auth_method = BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_NO_OOB;
static BT_MESH_ROLE_T btmw_rpc_mesh_role = BT_MESH_ROLE_PROVISIONEE;
static BTMW_RPC_TEST_MESH_ONOFF_SERVER_MODEL_T btmw_rpc_onoff_server_model;
static UINT16 btmw_rpc_primary_element_idx = 0;
static UINT16 btmw_rpc_appidx = 0x124;
static UINT8 btmw_rpc_appkey[BT_MESH_KEY_SIZE] =
{
    0x63, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48
};
static UINT8 btmw_rpc_netkey[BT_MESH_KEY_SIZE] =
{
    0x18, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

static UINT8 g_new_primary_netkey[BT_MESH_KEY_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0xff};
static UINT8 btmw_rpc_deviceUuid[BT_MESH_UUID_SIZE] =
{
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};
static UINT8 btmw_remote_deviceUuid[BT_MESH_UUID_SIZE] =
{
    0x08, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static BTMW_RPC_TEST_MESH_DEVICE_DB_T device_db[] =
{
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x8f, 0x13, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:13:8f",   \
     {0xb7, 0xbd, 0x70, 0x3c, 0xf3, 0xc1, 0xe8, 0x13, 0x85, 0x11, 0x56, 0xea, 0x32, 0xae, 0x9d, 0x65}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x81, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:81",   \
     {0x8b, 0x36, 0x1f, 0xba, 0xe3, 0x81, 0xfe, 0xf2, 0x29, 0x5d, 0xa4, 0xd3, 0x1d, 0x2a, 0x82, 0xc4}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x71, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:71",   \
     {0xe2, 0xd4, 0x4e, 0x4e, 0xa8, 0xc7, 0xdb, 0xfa, 0x2c, 0x9d, 0x06, 0x85, 0x95, 0xbb, 0x14, 0xd4}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x9f, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:9f",   \
    {0xdf, 0xac, 0x10, 0x9a, 0x68, 0xa7, 0xec, 0x20, 0x89, 0xcc, 0x3f, 0x7a, 0x0b, 0x52, 0x72, 0x9a}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x83, 0x15, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:15:83",   \
     {0x53, 0x65, 0x95, 0x21, 0x66, 0xef, 0x9c, 0xb8, 0x22, 0xdd, 0xa8, 0x46, 0xa1, 0xce, 0x4c, 0xf7}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x15, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:15",   \
     {0x6a, 0xdf, 0xdd, 0xfe, 0xaf, 0x3c, 0x5e, 0x12, 0xfb, 0xc4, 0x8a, 0x2f, 0xdd, 0x3c, 0x88, 0x0f}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x28, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:28",   \
     {0xbe, 0x7a, 0x9e, 0xea, 0xb8, 0xe0, 0x1e, 0x28, 0xe1, 0x80, 0x28, 0xcb, 0x92, 0xf9, 0x9e, 0x3c}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x47, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:47",   \
     {0x0d, 0xc5, 0x94, 0x68, 0x94, 0xd5, 0x9d, 0xf8, 0x3a, 0x9c, 0x3d, 0xd7, 0xe6, 0xe4, 0x83, 0x5d}},
    {{0xa8, 0x01, 0x71, 0x01, 0x00, 0x00, 0x00, 0x96, 0x14, 0x11, 0x07, 0xda, 0x78, 0x00, 0x00, 0x00}, \
     "78:da:07:11:14:96",   \
     {0xfe, 0xa9, 0x69, 0x1f, 0xe6, 0xe9, 0xab, 0xb3, 0x94, 0x0c, 0xa6, 0x5d, 0xe5, 0x61, 0xbd, 0x49}},
    //add more here...

    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, \
     NULL,   \
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
};

static UINT16 btmw_rpc_prov_cur_netidx = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
static UINT16 btmw_rpc_config_client_element_idx = BTMW_RPC_TEST_MESH_INVALID_ELEMENT_INDEX;

static UINT16 btmw_rpc_handle_onoff_client;
static UINT16 btmw_rpc_handle_generic_power_onoff_client;
static UINT16 btmw_rpc_handle_lightness_client;
static UINT16 btmw_rpc_handle_ctl_client;
static UINT16 btmw_rpc_handle_hsl_client;
static UINT16 btmw_rpc_handle_sensor_client;

static BOOL g_GattBearer = FALSE;

static BT_MESH_PROV_PROVISIONER_PARAMS_T btmw_rpc_prov_params;

static BTMW_RPC_TEST_MESH_LIGHTING_LIGNTNESS_SERVER_T gLightness_server;
static BTMW_RPC_TEST_MESH_LIGHTING_CTL_SERVER_T gCTL_server;
static BTMW_RPC_TEST_MESH_LIGHTING_CTL_TEMP_SERVER_T gCTL_temperature_server;
static BTMW_RPC_TEST_MESH_LIGHTING_HSL_SERVER_T gHSL_server;
static BTMW_RPC_TEST_MESH_LIGHTING_HSL_HUE_SERVER_T gHSL_hue_server;
static BTMW_RPC_TEST_MESH_LIGHTING_HSL_SAT_SERVER_T gHSL_saturation_server;

static VOID *binding_model_list = NULL; // AB_queue of switch_mesh_binding_model_t
static BT_MESH_COMP_ELEMENT_T comp_elements;

static VOID *g_node_list = NULL;    //stores all the currently alive nodes in the network
static VOID *g_netkey_list = NULL;
static VOID *g_appkey_list = NULL;
// for app set mesh_data that save in app layer or cloud
static BT_MESH_RECORD_T *mesh_data;
static VOID _btmw_rpc_test_mesh_timer_proc(UINT32 timer_id, VOID *pv_args);
static VOID _btmw_rpc_test_mesh_bind_ctl_temperature(UINT16 state, UINT8 binding);
static VOID _btmw_rpc_test_mesh_bind_ctl_lightness(UINT16 state, UINT8 binding);
static VOID _btmw_rpc_test_mesh_generic_onoff_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable);

static VOID _btmw_rpc_test_mesh_vendor_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);

static VOID _btmw_rpc_test_mesh_sig_msg_server_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_generic_onoff_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_lightness_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_ctl_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_hsl_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_config_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_health_client_msg_handler(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event, VOID *pv_tag);
static VOID _btmw_rpc_test_mesh_sensor_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);

static UTIL_TIMER_T btmw_rpc_test_mesh_timer[] =
{
    {BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID, NULL, 10000, FALSE, _btmw_rpc_test_mesh_timer_proc, NULL},  //key distribution timer
    {BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID, NULL, 10000, FALSE, _btmw_rpc_test_mesh_timer_proc, NULL},  //key refresh phase set timer
    {BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID, NULL, 60000, FALSE, _btmw_rpc_test_mesh_timer_proc, NULL},  //key refresh phase set timer
    //add more...
};

const BT_MESH_ACCESS_OPCODE_HANDLER_T btmw_rpc_onoff_server_handler[] =
{
    {{BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_GET, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sig_msg_server_handler},
    {{BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sig_msg_server_handler},
    {{BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET_UNRELIABLE, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sig_msg_server_handler},
};
const BT_MESH_ACCESS_OPCODE_HANDLER_T btmw_rpc_onoff_client_handler[] =
{
    {{BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_generic_onoff_client_msg_handler},
};

const BT_MESH_ACCESS_OPCODE_HANDLER_T btmw_rpc_vendor_message_handler[] =
{
    {{BTMW_RPC_TEST_MESH_VENDOR_OPCODE_1, BTMW_RPC_TEST_MESH_VENDOR_OPCODE_1}, _btmw_rpc_test_mesh_vendor_msg_handler},
    {{BTMW_RPC_TEST_MESH_VENDOR_OPCODE_2, BTMW_RPC_TEST_MESH_VENDOR_OPCODE_1}, _btmw_rpc_test_mesh_vendor_msg_handler},
};

const BT_MESH_ACCESS_OPCODE_HANDLER_T btmw_rpc_sensor_client_handler[] =
{
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
    {{BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_STATUS, BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE}, _btmw_rpc_test_mesh_sensor_client_msg_handler},
};

static BT_MESH_ADDRESS_TYPE_T _btmw_rpc_test_get_addr_type(UINT16 addr)
{
    if (addr == BTMW_RPC_TEST_MESH_ADDR_UNASSIGNED_VALUE)
    {
        return BT_MESH_ADDRESS_TYPE_UNASSIGNED;
    }
    else if ((addr & BTMW_RPC_TEST_MESH_ADDR_UNICAST_MASK) == BTMW_RPC_TEST_MESH_ADDR_UNICAST_VALUE)
    {
        return BT_MESH_ADDRESS_TYPE_UNICAST;
    }
    else
    {
        if ((addr & BTMW_RPC_TEST_MESH_ADDR_VIRTUAL_MASK) == BTMW_RPC_TEST_MESH_ADDR_VIRTUAL_VALUE)
        {
            return BT_MESH_ADDRESS_TYPE_VIRTUAL;
        }
        else
        {
            return BT_MESH_ADDRESS_TYPE_GROUP;
        }
    }
}

static VOID _btmw_rpc_test_mesh_timer_proc(UINT32 timer_id, VOID *pv_args)
{
    switch (timer_id)
    {
        case BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID:
        {
            BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T*)pv_args;
            BTMW_RPC_TEST_Loge("[mesh] Key distribution for Netkey 0x%x timeout\n", netkey->keyidx);
            //Error handle... todo TBD
            netkey->state = BT_MESH_KEY_REFRESH_STATE_NONE;
            BT_MESH_NETKEY_T netk;
            memset(&netk, 0, sizeof(netk));
            netk.opcode = BT_MESH_KEY_REVOKE_OLD_NETKEY;
            netk.key_index = netkey->keyidx;
            netk.network_key = netkey->temp_netkey;  //key value is useless here actually
            a_mtkapi_bt_mesh_set_netkey(&netk);
            util_dlist_empty(netkey->kr_node_list, TRUE);
            util_dlist_empty(netkey->kr_node_ack_list, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID:
        {
            BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T*)pv_args;
            BTMW_RPC_TEST_Loge("[mesh] Key refresh phase set for Netkey 0x%x timeout\n", netkey->keyidx);
            //Error handle... todo TBD
            netkey->state = BT_MESH_KEY_REFRESH_STATE_NONE;
            BT_MESH_NETKEY_T netk;
            memset(&netk, 0, sizeof(netk));
            netk.opcode = BT_MESH_KEY_REVOKE_OLD_NETKEY;
            netk.key_index = netkey->keyidx;
            netk.network_key = netkey->temp_netkey;  //key value is useless here actually
            a_mtkapi_bt_mesh_set_netkey(&netk);
            util_dlist_empty(netkey->kr_node_list, TRUE);
            util_dlist_empty(netkey->kr_node_ack_list, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID:
        {
            BTMW_RPC_TEST_Loge("[mesh] Config node 0x%04x timeout\n", g_curr_prov_node.addr);
            g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE;
            break;
        }
        default:
            break;
    }
}

static BTMW_RPC_TEST_MESH_NODE_ENTRY_T * _btmw_rpc_test_mesh_get_node_by_addr(VOID* list, UINT16 addr)
{
    BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;

    node= (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(list);
    while (node != NULL)
    {
        if (node->addr == addr)
        {
            BTMW_RPC_TEST_Logi("[mesh] Node 0x%x is found\n", addr);
            return node;
        }
        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(list, node);
    }
    BTMW_RPC_TEST_Loge("[mesh] Node 0x%x is NOT found\n", addr);
    return NULL;
}

static BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T * _btmw_rpc_test_mesh_get_netkey_by_idx(VOID *list, UINT16 idx)
{
    BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = NULL;

    netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *)util_dlist_first(list);
    while (netkey != NULL)
    {
        if (netkey->keyidx == idx)
        {
            BTMW_RPC_TEST_Logi("[mesh] Netkey 0x%x is found\n", idx);
            return netkey;
        }
        netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *)util_dlist_next(list, netkey);
    }
    BTMW_RPC_TEST_Loge("[mesh] Netkey 0x%x is NOT found\n", idx);
    return NULL;
}

static VOID _btmw_rpc_test_resend_msg(BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *msg)
{
    if (NULL == msg)
    {
        BTMW_RPC_TEST_Loge("[mesh] Error, msg is NULL\n");
        return;
    }

    switch (msg->opcode)
    {
        case BT_MESH_ACCESS_MSG_CONFIG_BEACON_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_BEACON_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_FRIEND_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_FRIEND_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_RELAY_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_RELAY_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE:
        case BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD:
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_DELETE:
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE:
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD:
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE:
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_DELETE:
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND:
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET:
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_GET:
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET:
        {
            INT32 i4_ret = a_mtkapi_bt_mesh_model_cc_msg_tx((BT_MESH_CONFIGURATION_MSG_TX_T *)(msg->buf));
            BTMW_RPC_TEST_Logi("[mesh] resend msg (opcode: 0x%04x) (msg_id: 0x%x)return %d\n", msg->opcode, msg->msg_id, i4_ret);
            break;
        }
        default:
            BTMW_RPC_TEST_Loge("[mesh] UNKNOWN OPCODE:0x%x\n", msg->opcode);
            break;
    }

}

static VOID _btmw_rpc_test_remove_resend_msg_entry(VOID *list, BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *removed, BOOL destroy)
{
    BTMW_RPC_TEST_Logi("[mesh] remove_resend_msg_entry, opcode: 0x%04x, id: 0x%x, destory: %d\n", removed->opcode, removed->msg_id, destroy);

    util_dlist_remove(list, removed);

    if (btmw_rpc_test_resend_msg_list_tail == removed)
    {
        btmw_rpc_test_resend_msg_list_tail = NULL;
    }

    if (TRUE == destroy)    //resend enough times, remove the entry from the list and destroy it
    {
        if (removed->opcode == BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET)
        {
            free(((BT_MESH_CONFIGURATION_MSG_TX_T *)(removed->buf))->data.hb_pub_set.publication);
        }
        free(removed->buf);
        util_dlist_entry_free(removed);
    }
    else    //move the entry to the list tail
    {
        if (NULL == btmw_rpc_test_resend_msg_list_tail)
        {
            util_dlist_insert(btmw_rpc_test_resend_msg_list, removed);
        }
        else
        {
            util_dlist_insert_after(btmw_rpc_test_resend_msg_list, btmw_rpc_test_resend_msg_list_tail, removed);
        }
        btmw_rpc_test_resend_msg_list_tail = removed;
    }
}

static VOID _btmw_rpc_test_add_mesh_resend_msg_entry(UINT16 opcode, UINT16 src, UINT16 dst, VOID *buf, UINT32 buf_len)
{
    BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *entry = NULL;

    if ((NULL == buf) || (0 == buf_len))
    {
        BTMW_RPC_TEST_Logi("[mesh] add_mesh_resend_msg_entry, buf_len:%d, nothing to add\n", buf_len);
        return;
    }

    entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_entry_alloc(sizeof(BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T));
    if (NULL == entry)
    {
        BTMW_RPC_TEST_Loge("[mesh] fail @ L %d\n", __LINE__);
        return;
    }

    entry->buf = malloc(buf_len);
    if (NULL == entry->buf)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc entry->buf failed\n");
        util_dlist_entry_free(entry);
        return;
    }

    pthread_mutex_lock(&resend_msg_mutex);
    entry->msg_id = g_resend_msg_id++;
    entry->resend_cnt = BTMW_RPC_TEST_MESH_MSG_RESEND_COUNT;
    entry->opcode = opcode;
    entry->src = src;
    entry->dst = dst;
    clock_gettime(CLOCK_MONOTONIC, &entry->send_time);
    memcpy(entry->buf, buf, buf_len);
    entry->buf_len = buf_len;

    //Always insert the new entry to the list tail. For the msgs which share the same ack msg, we think the ack is for the first one in the resend list.
    //e.g. We send Config AppKey Add and Config Appkey Update msgs sequentially. When a Config AppKey Status msg is received, we think it is for Config Appkey Add msg.
    //This is not safe since the first ACK could be lost. However, we have no other method to handle such a case.
    if (NULL == btmw_rpc_test_resend_msg_list_tail)
    {
        util_dlist_insert(btmw_rpc_test_resend_msg_list, entry);
    }
    else
    {
        util_dlist_insert_after(btmw_rpc_test_resend_msg_list, btmw_rpc_test_resend_msg_list_tail, entry);
    }
    btmw_rpc_test_resend_msg_list_tail = entry;
    BTMW_RPC_TEST_Logi("[mesh] add_mesh_resend_msg_entry: msg_id[0x%x], opcode[0x%04x], src[0x%x], dst[0x%x], buf_len[0x%x], send_time: %ld.%ld, list_count: %d\n", \
        entry->msg_id, entry->opcode, entry->src, entry->dst, entry->buf_len, entry->send_time.tv_sec, \
        entry->send_time.tv_nsec, util_dlist_count(btmw_rpc_test_resend_msg_list));
    pthread_mutex_unlock(&resend_msg_mutex);
}

static BOOL _btmw_rpc_test_mesh_check_msg_resend_stop(UINT16 opcode, UINT16 ack_src, UINT16 ack_dst)
{
    BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *resend_msg_entry = NULL;
    BOOL found = FALSE;

    pthread_mutex_lock(&resend_msg_mutex);
    resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
    while (resend_msg_entry != NULL)
    {
        if (resend_msg_entry->opcode == opcode)
        {
            if ((resend_msg_entry->dst == ack_src)
                && (resend_msg_entry->src == ack_dst)
                && (resend_msg_entry->resend_cnt > 0))
            {
                BTMW_RPC_TEST_Logd("[mesh] Peer replies msg [0x%04x][id:0x%x], stop resend it\n", resend_msg_entry->opcode, resend_msg_entry->msg_id);
                //just set the resend_cnt to 0, it will be removed in the msg resend thread
                resend_msg_entry->resend_cnt = 0;
                found = TRUE;
                break;
            }
        }
        resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_next(btmw_rpc_test_resend_msg_list, resend_msg_entry);
    }
    if (resend_msg_entry == NULL)
    {
        BTMW_RPC_TEST_Loge("[mesh] Peer replies msg , entry is NULL\n");
    }
    pthread_mutex_unlock(&resend_msg_mutex);
    return found;
}

static VOID *_btmw_rpc_test_resend_msg_thread(VOID *arg)
{
    BTMW_RPC_TEST_Logi("[mesh] %s\n", __func__);

    pthread_detach(pthread_self());

    while (!_resend_msg_thread_exit)
    {
        BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *entry = NULL;
        struct timespec curr_time;

        clock_gettime(CLOCK_MONOTONIC, &curr_time);

        usleep(2000);

        pthread_mutex_lock(&resend_msg_mutex);
        entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
        if (entry != NULL)
        {
            if (entry->resend_cnt > 0)
            {
                if ((curr_time.tv_sec - entry->send_time.tv_sec) >= BTMW_RPC_TEST_MESH_MSG_RESEND_INTERVAL) //2s passed
                {
                    pthread_mutex_unlock(&resend_msg_mutex);
                    _btmw_rpc_test_resend_msg(entry);
                    pthread_mutex_lock(&resend_msg_mutex);
                    //move the msg to the list tail
                    entry->send_time.tv_sec = curr_time.tv_sec;
                    entry->send_time.tv_nsec = curr_time.tv_nsec;
                    entry->resend_cnt--;
                    _btmw_rpc_test_remove_resend_msg_entry(btmw_rpc_test_resend_msg_list, entry, FALSE);
                }
            }
            else
            {
                if ((curr_time.tv_sec - entry->send_time.tv_sec) >= BTMW_RPC_TEST_MESH_MSG_RESEND_INTERVAL) //2s passed from the last resend, remove the message
                {
                    _btmw_rpc_test_remove_resend_msg_entry(btmw_rpc_test_resend_msg_list, entry, TRUE);
                }
            }
        }
        else
        {
            //BTMW_RPC_TEST_Logd("[mesh] Resend msg entry is NULL\n");
        }
        pthread_mutex_unlock(&resend_msg_mutex);
    }

    pthread_exit(0);

    return NULL;
}

/* SIG config messages start */
static VOID _btmw_rpc_test_mesh_cc_config_composition_data_get(UINT16 dst_addr, UINT8 page)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET;
    msg.data.composition_data_get.page = page;
    msg.data.composition_data_get.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.composition_data_get.meta.dst_addr = dst_addr;
    msg.data.composition_data_get.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.composition_data_get.meta.msg_netkey_index = btmw_rpc_prov_cur_netidx;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.composition_data_get.meta.src_addr,
                                        msg.data.composition_data_get.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);
    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_composition_data_get ret: %d\n", ret);
}

static VOID _btmw_rpc_test_mesh_cc_config_appkey_add(UINT16 appkeyIdx, UINT16 netkeyIdx, UINT8*appkey, UINT16 dst_addr)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD;
    msg.data.appkey_add.appkey_index = appkeyIdx;
    msg.data.appkey_add.netkey_index = netkeyIdx;
    msg.data.appkey_add.appkey = appkey;
    msg.data.appkey_add.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.appkey_add.meta.dst_addr = dst_addr;
    msg.data.appkey_add.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.appkey_add.meta.msg_netkey_index = netkeyIdx;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.appkey_add.meta.src_addr,
                                        msg.data.appkey_add.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_appkey_add ret: %d\n", ret);
}

static VOID _btmw_rpc_test_mesh_cc_config_model_app_bind(UINT16 element_addr, UINT16 appkeyidx, UINT16 dst_addr, UINT32 model_id)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND;
    msg.data.model_app_bind.element_addr = element_addr;
    msg.data.model_app_bind.appkey_index = appkeyidx;
    msg.data.model_app_bind.model_id = model_id;
    msg.data.model_app_bind.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.model_app_bind.meta.dst_addr = dst_addr;
    msg.data.model_app_bind.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.model_app_bind.meta.msg_netkey_index = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.appkey_add.meta.src_addr,
                                        msg.data.appkey_add.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_model_app_bind ret: %d\n", ret);
}

static VOID _btmw_rpc_test_mesh_cc_config_node_reset(UINT16 dst, UINT16 netkey_idx)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET;
    msg.data.node_reset.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.node_reset.meta.dst_addr = dst;
    msg.data.node_reset.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.node_reset.meta.msg_netkey_index = netkey_idx;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.node_reset.meta.src_addr,
                                        msg.data.node_reset.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_node_reset ret: %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_cc_config_netkey_update(UINT16 dst, UINT16 netkey_idx, UINT8 *netkey)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE;
    msg.data.netkey_update.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.netkey_update.meta.dst_addr = dst;
    msg.data.netkey_update.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.netkey_update.meta.msg_netkey_index = netkey_idx;
    msg.data.netkey_update.netkey_index = netkey_idx;
    msg.data.netkey_update.netkey = netkey;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.netkey_update.meta.src_addr,
                                        msg.data.netkey_update.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_netkey_update ret: %d\n", ret);

}
static VOID _btmw_rpc_test_mesh_cc_config_key_refresh_phase_set(UINT16 dst, UINT16 netkey_idx, UINT8 transition)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET;
    msg.data.key_ref_pha_set.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.key_ref_pha_set.meta.dst_addr = dst;
    msg.data.key_ref_pha_set.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.key_ref_pha_set.meta.msg_netkey_index = netkey_idx;
    msg.data.key_ref_pha_set.netkey_index = netkey_idx;
    msg.data.key_ref_pha_set.transition = transition;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.key_ref_pha_set.meta.src_addr,
                                        msg.data.key_ref_pha_set.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_key_refresh_phase_set ret: %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_cc_config_model_suscription_add(UINT16 dst, UINT16 netkey_idx, UINT16 element_addr, UINT16 addr, UINT32 model_id)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD;
    msg.data.model_sub_add.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.model_sub_add.meta.dst_addr = dst;
    msg.data.model_sub_add.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.model_sub_add.meta.msg_netkey_index = netkey_idx;
    msg.data.model_sub_add.element_addr = element_addr;
    msg.data.model_sub_add.address.type = BT_MESH_ADDRESS_TYPE_GROUP;
    msg.data.model_sub_add.address.value = addr;
    msg.data.model_sub_add.address.virtual_uuid = NULL;
    msg.data.model_sub_add.model_id = model_id;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.model_sub_add.meta.src_addr,
                                        msg.data.model_sub_add.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_model_suscription_add ret: %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_cc_config_hb_pub_set(UINT16 dst, UINT16 netkey_idx, UINT16 pub_dst,
                                UINT8 countLog, UINT8 periodLog, UINT8 ttl, UINT16 features, UINT16 targetNetkeyIdx)
{
    BT_MESH_CONFIGURATION_MSG_TX_T msg;
    BT_MESH_HEARTBEAT_PUBLICATION_T *hb_pub = NULL;

    hb_pub = (BT_MESH_HEARTBEAT_PUBLICATION_T *)malloc(sizeof(BT_MESH_HEARTBEAT_PUBLICATION_T));
    if (hb_pub == NULL)
    {
        BTMW_RPC_TEST_Loge("[mesh] alloc hb_pub fail\n");
        return;
    }
    msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET;
    msg.data.hb_pub_set.meta.src_addr = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_config_client_element_idx);
    msg.data.hb_pub_set.meta.dst_addr = dst;
    msg.data.hb_pub_set.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    msg.data.hb_pub_set.meta.msg_netkey_index = netkey_idx;
    msg.data.hb_pub_set.publication = hb_pub;
    hb_pub->destination = pub_dst; //could be group address or unicast address
    hb_pub->count_log = countLog;
    hb_pub->period_log = periodLog; // heartbeat message interval:2^(periodlog-1)
    hb_pub->ttl = ttl;
    hb_pub->features = features;
    hb_pub->netkey_index = targetNetkeyIdx;

    _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                        msg.data.model_sub_add.meta.src_addr,
                                        msg.data.model_sub_add.meta.dst_addr,
                                        &msg,
                                        sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

    INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

    BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_hb_pub_set ret: %d\n", ret);

}
/* SIG config messages end */

static VOID _btmw_rpc_test_mesh_composition_data_parse_element(UINT8 **buf, UINT16 *buf_len)
{
    if (*buf_len == 0) {
        BTMW_RPC_TEST_Loge("[mesh] no more elements.\n");
        return;
    }

    UINT8 i = 0;

    if (*buf_len >= 4) {
        comp_elements.loc = (*buf)[0] | ((*buf)[1] << 8);
        comp_elements.num_s = (*buf)[2];
        comp_elements.num_v = (*buf)[3];
        comp_elements.sig_models = malloc(comp_elements.num_s * 2); /* UINT16 */
        if (NULL == comp_elements.sig_models)
        {
            BTMW_RPC_TEST_Loge("[mesh] malloc comp_elements.sig_models failed\n");
            return;
        }
        comp_elements.vendor_models = malloc(comp_elements.num_v * 4); /* UINT32 */
        if (NULL == comp_elements.vendor_models)
        {
            BTMW_RPC_TEST_Loge("[mesh] malloc comp_elements.vendor_models failed\n");
            free(comp_elements.sig_models);
            comp_elements.sig_models = NULL;
            return;
        }
        *buf_len -= 4;
        *buf += 4;
        BTMW_RPC_TEST_Logd("[mesh] buflen: %d, [LOC:0x%04x, S:%d, V:%d]\n", *buf_len,
                           comp_elements.loc, comp_elements.num_s, comp_elements.num_v);
        //MESH_DEBUG_DUMP_PDU("Data", (*buf_len), (*buf));

        if (*buf_len >= (comp_elements.num_s * 2 + comp_elements.num_v * 4)) {
            memcpy(comp_elements.sig_models, *buf, comp_elements.num_s * 2);
            *buf_len -= comp_elements.num_s * 2;
            *buf += comp_elements.num_s * 2;
            memcpy(comp_elements.vendor_models, *buf, comp_elements.num_v * 4);
            *buf_len -= comp_elements.num_v * 4;
            *buf += comp_elements.num_v * 4;
            BTMW_RPC_TEST_Logd("[mesh] element %d is parsed.\n", i);
            //MESH_DEBUG_DUMP_PDU("SigModel", elements->num_s, elements->sig_models);
            //MESH_DEBUG_DUMP_PDU("VendorModel", elements->num_v, elements->vendor_models);
        } else {
            BTMW_RPC_TEST_Loge("[mesh] invalid model data.\n");
            free(comp_elements.sig_models);
            comp_elements.sig_models = NULL;
            free(comp_elements.vendor_models);
            comp_elements.vendor_models = NULL;
            *buf_len = 0;
        }
        i++;
    } else {
        BTMW_RPC_TEST_Loge("[mesh] invalid element data\n");
        *buf_len = 0;
    }

    return;
}

static BT_MESH_COMP_DATA_T *_btmw_rpc_test_mesh_model_parse_composition_data(UINT8 *buf, UINT16 buf_len)
{
    BT_MESH_COMP_DATA_T *result = (BT_MESH_COMP_DATA_T *)malloc(sizeof(BT_MESH_COMP_DATA_T));
    if (NULL == result)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc BT_MESH_COMP_DATA_T failed\n");
        return NULL;
    }
    result->cid = buf[0] | (buf[1] << 8);
    result->pid = buf[2] | (buf[3] << 8);
    result->vid = buf[4] | (buf[5] << 8);
    result->crpl = buf[6] | (buf[7] << 8);
    result->features = buf[8] | (buf[9] << 8);
    result->element_length = 0;

    /* parsing element data */
    BT_MESH_COMP_ELEMENT_T *last_entry = NULL, *entry = NULL;
    UINT16 *p_length = malloc(2);
    if (NULL == p_length)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc p_length failed\n");
        free(result);
        return NULL;
    }
    UINT8 **pp_buffer = malloc(sizeof(UINT8 *));
    if (NULL == pp_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc pp_buffer failed\n");
        free(p_length);
        free(result);
        return NULL;
    }
    *p_length = buf_len - 10;
    *pp_buffer = buf + 10;
    _btmw_rpc_test_mesh_composition_data_parse_element(pp_buffer, p_length);

    result->elements = util_dlist_alloc();

    entry = util_dlist_entry_alloc(sizeof(BT_MESH_COMP_ELEMENT_T));
    memcpy(entry, &comp_elements, sizeof(BT_MESH_COMP_ELEMENT_T));
    util_dlist_insert(result->elements, entry);
    result->element_length++;
    while (0 != *p_length) {
        _btmw_rpc_test_mesh_composition_data_parse_element(pp_buffer, p_length);
        last_entry = entry;
        entry = util_dlist_entry_alloc(sizeof(BT_MESH_COMP_ELEMENT_T));
        memcpy(entry, &comp_elements, sizeof(BT_MESH_COMP_ELEMENT_T));
        util_dlist_insert_after(result->elements, last_entry, entry);
        result->element_length++;
    }

    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(p_length);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(pp_buffer);

    return result;
}

static VOID _btmw_rpc_test_mesh_handle_composition_data(BT_MESH_COMP_DATA_T *data)
{
    BTMW_RPC_TEST_Logd("[mesh] CID: 0x%04x\n", data->cid);
    BTMW_RPC_TEST_Logd("[mesh] PID: 0x%04x\n", data->pid);
    BTMW_RPC_TEST_Logd("[mesh] VID: 0x%04x\n", data->vid);
    BTMW_RPC_TEST_Logd("[mesh] CRPL: 0x%04x\n", data->crpl);
    BTMW_RPC_TEST_Logd("[mesh] Features: 0x%04x\n", data->features);
    if (binding_model_list != NULL) {
        util_queue_free(binding_model_list);
    }
    binding_model_list = util_queue_alloc();

    if (data->elements != NULL && util_dlist_count(data->elements) > 0) {
        BTMW_RPC_TEST_MESH_BINDING_MODEL_T *model_entry = NULL;
        UINT32 i, j = 0;
        BT_MESH_COMP_ELEMENT_T *entry = (BT_MESH_COMP_ELEMENT_T *)util_dlist_first(data->elements);
        while (entry != NULL) {
            BTMW_RPC_TEST_Logd("    Element#%d, LOC: 0x%04x\n", j, entry->loc);
            BTMW_RPC_TEST_Logd("\tNumS: %d {", entry->num_s);
            if (entry->num_s > 0) {
                for(i= 0; i < entry->num_s ; i++) {
                    if (entry->sig_models[i] != BTMW_RPC_TEST_MESH_SIG_MODEL_ID_CONFIGURATION_SERVER) {
                        model_entry = util_queue_entry_alloc(sizeof(BTMW_RPC_TEST_MESH_BINDING_MODEL_T));
                        model_entry->element_index = j;
                        model_entry->model_id = entry->sig_models[i];
                        util_queue_push(binding_model_list, model_entry);
                        printf("%04x, ", entry->sig_models[i]);
                    }
                }
            }
            BTMW_RPC_TEST_Logd("}\n");
            BTMW_RPC_TEST_Logd("\tNumV: %d {", entry->num_v);
            if (entry->num_v > 0) {
                for(i= 0; i < entry->num_v ; i++) {
                    model_entry = util_queue_entry_alloc(sizeof(BTMW_RPC_TEST_MESH_BINDING_MODEL_T));
                    model_entry->element_index = j;
                    //model_entry->model_id = entry->vendor_models[i];
                    uint8_t *buf = (uint8_t *)(&entry->vendor_models[i]);
                    model_entry->model_id = ((buf[0] | (buf[1] << 8)) << 16) | (buf[2] | (buf[3] << 8));
                    util_queue_push(binding_model_list, model_entry);
                    printf("%08x, ", model_entry->model_id);
                }
            }
            BTMW_RPC_TEST_Logd("}\n");
            entry = util_dlist_next(data->elements, (VOID *)entry);
            j++;
        }
    }
}

static VOID _btmw_rpc_test_mesh_model_free_composition_data(BT_MESH_COMP_DATA_T *composition_data)
{
    if (composition_data != NULL) {
        if (composition_data->elements != NULL) {
            BT_MESH_COMP_ELEMENT_T *entry = (BT_MESH_COMP_ELEMENT_T *)util_dlist_first(composition_data->elements);
            while (entry != NULL) {
                BTMW_RPC_MESH_CHECK_NULL_AND_FREE(entry->sig_models);
                BTMW_RPC_MESH_CHECK_NULL_AND_FREE(entry->vendor_models);
                entry = util_dlist_next(composition_data->elements, entry);
            }
            util_dlist_free(composition_data->elements);
            composition_data->elements = NULL;
        }
    }
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(composition_data);
}

static VOID _btmw_rpc_test_mesh_add_node_to_list(VOID *list, BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node)
{
    BTMW_RPC_TEST_MESH_NODE_ENTRY_T *entry = NULL;

    if ((NULL == list) || (NULL == node))
    {
        BTMW_RPC_TEST_Loge("[mesh] error, List or node is NULL, return\n");
        return;
    }
    entry = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_entry_alloc(sizeof(BTMW_RPC_TEST_MESH_NODE_ENTRY_T));
    if (NULL == entry)
    {
        BTMW_RPC_TEST_Loge("[mesh] allocate entry failed\n");
    }
    else
    {
        memcpy(entry, node, sizeof(BTMW_RPC_TEST_MESH_NODE_ENTRY_T));
        util_dlist_insert(list, entry);
    }
}


static VOID _btmw_rpc_test_mesh_generic_on_off_status(UINT16 model_handle, UINT8 presentOnOff, UINT8 targetOnOff,
    UINT8 remainingTime, BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] Generic_OnOffStatus\n");
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_STATUS;
    reply.length = 3;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    reply.p_buffer[0] = presentOnOff;
    reply.p_buffer[1] = targetOnOff;
    reply.p_buffer[2] = remainingTime;

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] Generic_OnOffStatus, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_ctl_status(UINT16 model_handle, UINT16 present_ctl_lightness,
                                                              UINT16 present_ctl_temp, BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] CTL_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_STATUS;
    reply.length = 4;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &present_ctl_lightness, 2);
    memcpy(reply.p_buffer + 2, &present_ctl_temp, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] CTL_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_ctl_temp_status(UINT16 model_handle, UINT16 present_ctl_temp,
                                                              UINT16 present_ctl_delta_uv, BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] CTL_TEMP_Status\n");
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_STATUS;
    reply.length = 4;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &present_ctl_temp, 2);
    memcpy(reply.p_buffer + 2, &present_ctl_delta_uv, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] CTL_TEMP_Status, ret = %d\n", ret);
}

static VOID _btmw_rpc_test_mesh_light_ctl_temp_range_status(UINT16 model_handle, UINT16 range_min,
                                                              UINT16 range_max, BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] ctl_temp_range_status\n");
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_STATUS;
    reply.length = 5;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    reply.p_buffer[0] = 0; // status
    memcpy(reply.p_buffer + 1, &range_min, 2);
    memcpy(reply.p_buffer + 3, &range_max, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] ctl_temp_range_status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_ctl_default_status(UINT16 model_handle, UINT16 default_lightness,
                                                                       UINT16 default_temp, UINT16 defaut_delta_uv,
                                                                       BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] ctl_default_status\n");
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_STATUS;
    reply.length = 6;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &default_lightness, 2);
    memcpy(reply.p_buffer + 2, &default_temp, 2);
    memcpy(reply.p_buffer + 4, &defaut_delta_uv, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] ctl_default_status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_hsl_status(UINT16 model_handle, UINT16 present_hsl_lightness,
                                                              UINT16 present_hsl_hue, UINT16 present_hsl_sat,
                                                              BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_STATUS;
    reply.length = 6;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &present_hsl_lightness, 2);
    memcpy(reply.p_buffer + 2, &present_hsl_hue, 2);
    memcpy(reply.p_buffer + 4, &present_hsl_sat, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_target_status(UINT16 model_handle, UINT16 target_hsl_lightness,
                                                              UINT16 target_hsl_hue, UINT16 target_hsl_sat,
                                                              BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_target_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_STATUS;
    reply.length = 6;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &target_hsl_lightness, 2);
    memcpy(reply.p_buffer + 2, &target_hsl_hue, 2);
    memcpy(reply.p_buffer + 4, &target_hsl_sat, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_target_Status, ret = %d\n", ret);

}


static VOID _btmw_rpc_test_mesh_light_hsl_hue_status(UINT16 model_handle, UINT16 present_hsl_hue,
                                                                    BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_hue_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_STATUS;
    reply.length = 2;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &present_hsl_hue, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_hue_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_hsl_sat_status(UINT16 model_handle, UINT16 present_hsl_sat,
                                                                    BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_sat_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_STATUS;
    reply.length = 2;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &present_hsl_sat, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_sat_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_hsl_range_status(UINT16 model_handle, UINT16 hue_range_min,
                                                                       UINT16 hue_range_max, UINT16 sat_range_min,
                                                                       UINT16 sat_range_max, BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_range_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_STATUS;
    reply.length = 9;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    reply.p_buffer[0] = 0; // status
    memcpy(reply.p_buffer + 1, &hue_range_min, 2);
    memcpy(reply.p_buffer + 3, &hue_range_max, 2);
    memcpy(reply.p_buffer + 5, &sat_range_min, 2);
    memcpy(reply.p_buffer + 7, &sat_range_max, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_range_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_light_hsl_default_status(UINT16 model_handle, UINT16 default_lightness,
                                                                        UINT16 default_hue, UINT16 default_saturation,
                                                                        BT_MESH_ACCESS_MESSAGE_RX_T* msg)
{
    BTMW_RPC_TEST_Logi("[mesh] HSL_default_Status, model_handle = 0x%x\n", model_handle);
    INT32 ret = 0;

    BT_MESH_ACCESS_MESSAGE_TX_T reply;

    reply.opcode.company_id = BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE;
    reply.opcode.opcode = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_STATUS;
    reply.length = 6;
    reply.p_buffer = malloc(reply.length);
    if (NULL == reply.p_buffer)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc reply.p_buffer failed\n");
        return;
    }
    memcpy(reply.p_buffer, &default_lightness, 2);
    memcpy(reply.p_buffer + 2, &default_hue, 2);
    memcpy(reply.p_buffer + 4, &default_saturation, 2);

    ret = a_mtkapi_bt_mesh_access_model_reply(model_handle, msg, &reply);
    BTMW_RPC_MESH_CHECK_NULL_AND_FREE(reply.p_buffer);

    BTMW_RPC_TEST_Logd("[mesh] HSL_default_Status, ret = %d\n", ret);

}

static VOID _btmw_rpc_test_mesh_generic_onoff_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    if (btmw_rpc_onoff_server_model.TID != msg->buf[1]) //new message
    {
        btmw_rpc_onoff_server_model.TID = msg->buf[1];
        btmw_rpc_onoff_server_model.transTime = msg->buf[2];
        btmw_rpc_onoff_server_model.delay = msg->buf[3];
        BTMW_RPC_TEST_Logi("[mesh] TID %d transTime %d delay %d\n",
            btmw_rpc_onoff_server_model.TID, btmw_rpc_onoff_server_model.transTime, btmw_rpc_onoff_server_model.delay);
    }
    else
    {
        BTMW_RPC_TEST_Logi("[mesh] not new message, ignored\n");
    }

    if (reliable)
    {
        _btmw_rpc_test_mesh_generic_on_off_status(model_handle, btmw_rpc_onoff_server_model.onOff, msg->buf[0], 0, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_ctl_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    UINT16 temperature;

    memcpy(&temperature, &msg->buf[2], 2);
    if(temperature > 0x4E20 || temperature < 0x0320)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid temperature value\n");
        return;
    }

    gCTL_server.TID = msg->buf[6];
    gCTL_server.transition_time = msg->buf[7];
    gCTL_server.delay = msg->buf[8];

    memcpy(&gCTL_server.target_ctl_lightness, msg->buf, 2);
    memcpy(&gCTL_server.target_ctl_temperature, &msg->buf[2], 2);
    memcpy(&gCTL_server.target_ctl_delta_uv, &msg->buf[4], 2);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] Lightness   : %04x\n", gCTL_server.target_ctl_lightness);
    BTMW_RPC_TEST_Logd("[mesh] Temperature : %04x\n", gCTL_server.target_ctl_temperature);
    BTMW_RPC_TEST_Logd("[mesh] DeltaUV     : %04x\n", gCTL_server.target_ctl_delta_uv);

    memcpy(&gCTL_server.present_ctl_lightness, msg->buf, 2);
    memcpy(&gCTL_server.present_ctl_temperature, &msg->buf[2], 2);
    memcpy(&gCTL_server.present_ctl_delta_uv, &msg->buf[4], 2);

    _btmw_rpc_test_mesh_bind_ctl_lightness(BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS, BTMW_RPC_TEST_MESH_MODEL_BINDING_BOTH_VALUE);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_ctl_status(model_handle, gCTL_server.present_ctl_lightness, gCTL_server.present_ctl_temperature, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_ctl_temp_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    UINT16 temperature;

    memcpy(&temperature, msg->buf, 2);
    if(temperature > 0x4E20 || temperature < 0x0320)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid temperature value\n");
        return;
    }

    gCTL_temperature_server.TID = msg->buf[4];
    gCTL_temperature_server.transition_time = msg->buf[5];
    gCTL_temperature_server.delay = msg->buf[6];

    memcpy(&gCTL_temperature_server.target_ctl_temperature, msg->buf, 2);
    memcpy(&gCTL_temperature_server.target_ctl_delta_uv, &msg->buf[2], 2);

    BTMW_RPC_TEST_Logd("[mesh] Temperature : %04x\n", gCTL_temperature_server.target_ctl_temperature);
    BTMW_RPC_TEST_Logd("[mesh] DeltaUV     : %04x\n", gCTL_temperature_server.target_ctl_delta_uv);


    memcpy(&gCTL_temperature_server.present_ctl_temperature, msg->buf, 2);
    memcpy(&gCTL_temperature_server.present_ctl_delta_uv, &msg->buf[2], 2);

    _btmw_rpc_test_mesh_bind_ctl_temperature(BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE, BTMW_RPC_TEST_MESH_MODEL_BINDING_BOTH_VALUE);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_ctl_temp_status(model_handle, gCTL_temperature_server.present_ctl_temperature,
                                                  gCTL_temperature_server.present_ctl_delta_uv, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_ctl_temp_range_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    memcpy(&gCTL_server.range_min, msg->buf, 2);
    memcpy(&gCTL_server.range_max, &msg->buf[2], 2);

    if((gCTL_server.range_min > gCTL_server.range_max) ||
        (gCTL_server.range_max > 0x4E20) ||
        (gCTL_server.range_min < 0x0320))
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid temperature value\n");
        return;
    }

    BTMW_RPC_TEST_Logd("[mesh] range_min : %04x\n", gCTL_server.range_min);
    BTMW_RPC_TEST_Logd("[mesh] range_max     :%04x\n", gCTL_server.range_max);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_ctl_temp_range_status(model_handle, gCTL_server.range_min,
                                                  gCTL_server.range_max, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_ctl_default_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    UINT16 default_temperature;

    memcpy(&default_temperature, &msg->buf[2], 2);
    if(default_temperature > 0x4E20 || default_temperature < 0x320)
    {
        return;
    }

    memcpy(&gCTL_server.default_lightness, msg->buf, 2);
    memcpy(&gCTL_server.default_temperature, &msg->buf[2], 2);
    memcpy(&gCTL_server.default_delta_uv, &msg->buf[4], 2);

    BTMW_RPC_TEST_Logd("[mesh] default_lightness : %04x\n", gCTL_server.default_lightness);
    BTMW_RPC_TEST_Logd("[mesh] default_temperature     :%04x\n", gCTL_server.default_temperature);
    BTMW_RPC_TEST_Logd("[mesh] default_delta_uv     :%04x\n", gCTL_server.default_delta_uv);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_ctl_default_status(model_handle, gCTL_server.default_lightness,
                                                  gCTL_server.default_temperature, gCTL_server.default_delta_uv, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_hsl_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    gHSL_server.TID = msg->buf[6];
    gHSL_server.transition_time = msg->buf[7];
    gHSL_server.delay = msg->buf[8];

    memcpy(&gHSL_server.target_hsl_lightness, msg->buf, 2);
    memcpy(&gHSL_server.target_hsl_hue, &msg->buf[2], 2);
    memcpy(&gHSL_server.target_hsl_saturation, &msg->buf[4], 2);

    memcpy(&gHSL_server.present_hsl_lightness, msg->buf, 2);
    memcpy(&gHSL_server.present_hsl_hue, &msg->buf[2], 2);
    memcpy(&gHSL_server.present_hsl_saturation, &msg->buf[4], 2);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] Lightness   : %04x\n", gHSL_server.target_hsl_lightness);
    BTMW_RPC_TEST_Logd("[mesh] hue : %04x\n", gHSL_server.target_hsl_hue);
    BTMW_RPC_TEST_Logd("[mesh] sat     : %04x\n", gHSL_server.target_hsl_saturation);
    //to do hsl bind state
    //_bind_hsl_lightness(MESH_MODEL_STATE_LIGHTING_HSL_LIGHTNESS, MESH_MODEL_BINDING_BOTH_VALUE);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_hsl_status(model_handle, gHSL_server.present_hsl_lightness,
                                             gHSL_server.present_hsl_hue, gHSL_server.present_hsl_saturation, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_hsl_hue_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    gHSL_hue_server.TID = msg->buf[2];
    gHSL_hue_server.transition_time = msg->buf[3];
    gHSL_hue_server.delay = msg->buf[4];

    memcpy(&gHSL_hue_server.target_hsl_hue, msg->buf, 2);
    memcpy(&gHSL_hue_server.present_hsl_hue, msg->buf, 2);

    //todo bind hsl hue
    //_bind_hsl_hue(MESH_MODEL_STATE_LIGHTING_HSL_HUE, MESH_MODEL_BINDING_BOTH_VALUE);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] hue : %04x\n", gHSL_hue_server.target_hsl_hue);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_hsl_hue_status(model_handle, gHSL_hue_server.present_hsl_hue, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_hsl_sat_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    gHSL_saturation_server.TID = msg->buf[2];
    gHSL_saturation_server.transition_time = msg->buf[3];
    gHSL_saturation_server.delay = msg->buf[4];

    memcpy(&gHSL_saturation_server.target_hsl_saturation, msg->buf, 2);
    memcpy(&gHSL_saturation_server.present_hsl_saturation, msg->buf, 2);

    //todo bind hsl sat
    //_bind_hsl_saturation(MESH_MODEL_STATE_LIGHTING_HSL_SATURATION, MESH_MODEL_BINDING_TARGET_VALUE);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] saturation : %04x\n", gHSL_saturation_server.target_hsl_saturation);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_hsl_sat_status(model_handle, gHSL_saturation_server.present_hsl_saturation, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_hsl_range_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    UINT16 min, max;

    memcpy(&min, msg->buf, 2);
    memcpy(&max, &msg->buf[2], 2);

    if(min > max)
    {
        return;
    }

    memcpy(&min, &msg->buf[4], 2);
    memcpy(&max, &msg->buf[6], 2);

    if(min > max)
    {
        return;
    }

    memcpy(&gHSL_server.hue_range_min, msg->buf, 2);
    memcpy(&gHSL_server.hue_range_max, &msg->buf[2], 2);
    memcpy(&gHSL_server.saturation_range_min, &msg->buf[4], 2);
    memcpy(&gHSL_server.saturation_range_max, &msg->buf[6], 2);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] hue_range_min : %04x\n", gHSL_server.hue_range_min);
    BTMW_RPC_TEST_Logd("[mesh] hue_range_max : %04x\n", gHSL_server.hue_range_max);
    BTMW_RPC_TEST_Logd("[mesh] saturation_range_min : %04x\n", gHSL_server.saturation_range_min);
    BTMW_RPC_TEST_Logd("[mesh] saturation_range_max : %04x\n", gHSL_server.saturation_range_max);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_hsl_range_status(model_handle, gHSL_server.hue_range_min, gHSL_server.hue_range_max,
                                                 gHSL_server.saturation_range_min, gHSL_server.saturation_range_max, msg);
    }
}

static VOID _btmw_rpc_test_mesh_light_hsl_default_set_handler(UINT16 model_handle, BT_MESH_ACCESS_MESSAGE_RX_T* msg, const VOID* arg, BOOL reliable)
{
    memcpy(&gHSL_server.default_lightness, msg->buf, 2);
    memcpy(&gHSL_server.default_hue, &msg->buf[2], 2);
    memcpy(&gHSL_server.default_saturation, &msg->buf[4], 2);

    BTMW_RPC_TEST_Logd("[mesh] model_handle   : 0x%x\n", model_handle);
    BTMW_RPC_TEST_Logd("[mesh] default_lightness : %04x\n", gHSL_server.default_lightness);
    BTMW_RPC_TEST_Logd("[mesh] default_hue : %04x\n", gHSL_server.default_hue);
    BTMW_RPC_TEST_Logd("[mesh] default_saturation : %04x\n", gHSL_server.default_saturation);

    if(reliable)
    {
        _btmw_rpc_test_mesh_light_hsl_default_status(model_handle, gHSL_server.default_lightness, gHSL_server.default_hue,
                                                     gHSL_server.default_saturation, msg);
    }
}

static VOID _btmw_rpc_test_mesh_vendor_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{
    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }

    BTMW_RPC_TEST_Logi("[mesh] Vendor_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_VENDOR_OPCODE_1: {
            BTMW_RPC_TEST_Logi("[mesh] Model#1\n");
            break;
        }
        case BTMW_RPC_TEST_MESH_VENDOR_OPCODE_2: {
            BTMW_RPC_TEST_Logi("[mesh] Model#2\n");
            break;
        }

        default:
            return;
    }

    return;
}

static VOID _btmw_rpc_test_mesh_sensor_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] Sensor_Client_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_DESCRIPTOR_STATUS, msg->model_handle = %d, sensor descriptor status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_STATUS, msg->model_handle = %d, sensor status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_COLUMN_STATUS, msg->model_handle = %d, sensor column status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_SERIES_STATUS, msg->model_handle = %d, sensor series status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_CADENCE_STATUS, msg->model_handle = %d, sensor cadence status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_SETTINGS_STATUS, msg->model_handle = %d, sensor settings status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        case BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]SENSOR_SETTING_STATUS, msg->model_handle = %d, sensor setting status = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        default:
            break;
    }
    return;
}

static VOID _btmw_rpc_test_mesh_sig_msg_server_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{
    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] Sig_Msg_Server_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_GET:
        {
            _btmw_rpc_test_mesh_generic_on_off_status(msg->model_handle, btmw_rpc_onoff_server_model.onOff, btmw_rpc_onoff_server_model.onOff, 0, msg->msg);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET:
        {
            _btmw_rpc_test_mesh_generic_onoff_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET_UNRELIABLE:
        {
            _btmw_rpc_test_mesh_generic_onoff_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_GET:
            _btmw_rpc_test_mesh_light_ctl_status(msg->model_handle, gCTL_server.present_ctl_lightness,
                                                 gCTL_server.present_ctl_temperature, msg->msg);
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_SET:
        {
            _btmw_rpc_test_mesh_light_ctl_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_SET_UNACKNOWLEDGED:
        {
            _btmw_rpc_test_mesh_light_ctl_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_SET:
        {
            _btmw_rpc_test_mesh_light_ctl_temp_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_SET_UNACKNOWLEDGED:
        {
            _btmw_rpc_test_mesh_light_ctl_temp_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_SET:
        {
            _btmw_rpc_test_mesh_light_ctl_temp_range_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_SET_UNACKNOWLEDGED:
        {
            _btmw_rpc_test_mesh_light_ctl_temp_range_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_SET:
        {
            _btmw_rpc_test_mesh_light_ctl_default_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_SET_UNACKNOWLEDGED:
        {
            _btmw_rpc_test_mesh_light_ctl_default_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET:
        {
            _btmw_rpc_test_mesh_light_hsl_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET_UNACKNOWLEDGED:
        {
             _btmw_rpc_test_mesh_light_hsl_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_GET:
            _btmw_rpc_test_mesh_light_target_status(msg->model_handle, gHSL_server.target_hsl_lightness,
                                                    gHSL_server.target_hsl_hue, gHSL_server.target_hsl_saturation,
                                                    msg->msg);
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET:
        {
            _btmw_rpc_test_mesh_light_hsl_hue_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET_UNACKNOWLEDGED:
        {
             _btmw_rpc_test_mesh_light_hsl_hue_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET:
        {
            _btmw_rpc_test_mesh_light_hsl_sat_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED:
        {
             _btmw_rpc_test_mesh_light_hsl_sat_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET:
        {
            _btmw_rpc_test_mesh_light_hsl_range_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET_UNACKNOWLEDGED:
        {
             _btmw_rpc_test_mesh_light_hsl_range_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_GET:
            break;
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET:
        {
            _btmw_rpc_test_mesh_light_hsl_default_set_handler(msg->model_handle, msg->msg, msg->arg, TRUE);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACKNOWLEDGED:
        {
             _btmw_rpc_test_mesh_light_hsl_default_set_handler(msg->model_handle, msg->msg, msg->arg, FALSE);
            break;
        }
        default:
            return;
    }

    return;

}

static VOID _btmw_rpc_test_mesh_generic_onoff_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] Generic_Onoff_Client_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] [AT]GENERIC_ONOFF_STATUS, msg->model_handle = %d, presentOnOff = %d\n",
                msg->model_handle, msg->msg->buf[0]);
            break;
        default:
            return;
    }

    return;
}

static VOID _btmw_rpc_test_mesh_lightness_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] Lightness_Client_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    switch (msg->msg->opcode.opcode)
    {
        default:
            break;
    }

    return;
}

static VOID _btmw_rpc_test_mesh_ctl_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] CTL_Client_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);
    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_STATUS:
        {
            UINT16 present_ctl_lightness;
            UINT16 present_ctl_temp;
            memcpy(&present_ctl_lightness, msg->msg->buf, 2);
            memcpy(&present_ctl_temp, msg->msg->buf + 2, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_CTL_STATUS, msg->model_handle = %d, present_ctl_lightness = %04x, present_ctl_temp = %04x\n",
                                msg->model_handle, present_ctl_lightness, present_ctl_temp);

            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_STATUS:
        {
            UINT16 present_ctl_temp;
            UINT16 present_ctl_delta_uv;
            memcpy(&present_ctl_temp, msg->msg->buf, 2);
            memcpy(&present_ctl_delta_uv, msg->msg->buf + 2, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_CTL_TEMPERATURE_STATUS, msg->model_handle = %d, present_ctl_temp = %04x, present_ctl_delta_uv = %04x\n",
                                msg->model_handle, present_ctl_temp, present_ctl_delta_uv);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_STATUS:
        {
            UINT16 range_min;
            UINT16 range_max;
            memcpy(&range_min, msg->msg->buf + 1, 2);
            memcpy(&range_max, msg->msg->buf + 3, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_CTL_TEMPERATURE_RANGE_STATUS, msg->model_handle = %d, range_min = %04x, range_max = %04x\n",
                                msg->model_handle, range_min, range_max);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_STATUS:
        {
            UINT16 default_lightness;
            UINT16 default_temp;
            memcpy(&default_lightness, msg->msg->buf, 2);
            memcpy(&default_temp, msg->msg->buf + 2, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_CTL_TEMPERATURE_RANGE_STATUS, msg->model_handle = %d, default_lightness = %04x, default_temp = %04x\n",
                                msg->model_handle, default_lightness, default_temp);
            break;
        }
        default:
            return;
    }

    return;

}

static VOID _btmw_rpc_test_mesh_hsl_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] HSL_Client_Msg_Rx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);
    switch (msg->msg->opcode.opcode)
    {
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_STATUS:
        {
            UINT16 present_hsl_lightness;
            UINT16 present_hsl_hue;
            UINT16 present_hsl_sat;
            memcpy(&present_hsl_lightness, msg->msg->buf, 2);
            memcpy(&present_hsl_hue, msg->msg->buf + 2, 2);
            memcpy(&present_hsl_sat, msg->msg->buf + 4, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_HSL_STATUS, msg->model_handle = 0x%x, present_hsl_lightness = %04x, present_hsl_hue = %04x, present_hsl_sat = %04x\n",
                                msg->model_handle, present_hsl_lightness, present_hsl_hue, present_hsl_sat);

            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_STATUS:
        {
            UINT16 target_hsl_lightness;
            UINT16 target_hsl_hue;
            UINT16 target_hsl_sat;
            memcpy(&target_hsl_lightness, msg->msg->buf, 2);
            memcpy(&target_hsl_hue, msg->msg->buf + 2, 2);
            memcpy(&target_hsl_sat, msg->msg->buf + 4, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_HSL_TARGET_STATUS, msg->model_handle = 0x%x, target_hsl_lightness = %04x, target_hsl_hue = %04x, target_hsl_sat = %04x\n",
                                msg->model_handle, target_hsl_lightness, target_hsl_hue, target_hsl_sat);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_STATUS:
        {
            UINT16 present_hsl_hue;
            memcpy(&present_hsl_hue, msg->msg->buf, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_HSL_HUE_STATUS, msg->model_handle = 0x%x, present_hsl_hue = %04x\n",
                                msg->model_handle, present_hsl_hue);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_STATUS:
        {
            UINT16 present_hsl_sat;
            memcpy(&present_hsl_sat, msg->msg->buf, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_HSL_SATURATION_STATUS, msg->model_handle = 0x%x, present_hsl_sat = %04x\n",
                                msg->model_handle, present_hsl_sat);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_STATUS:
        {
            UINT16 hue_range_min;
            UINT16 hue_range_max;
            UINT16 sat_range_min;
            UINT16 sat_range_max;
            memcpy(&hue_range_min, msg->msg->buf + 1, 2);
            memcpy(&hue_range_max, msg->msg->buf + 3, 2);
            memcpy(&sat_range_min, msg->msg->buf + 5, 2);
            memcpy(&sat_range_max, msg->msg->buf + 7, 2);
            BTMW_RPC_TEST_Logi("[mesh] LIGHT_HSL_RANGE_STATUS, msg->model_handle = %d, hue_range_min = %04x, hue_range_max = %04x\n",
                                msg->model_handle, hue_range_min, hue_range_max);
            break;
        }
        case BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_STATUS:
        {
            UINT16 default_hsl_lightness;
            UINT16 default_hsl_hue;
            UINT16 default_hsl_sat;
            memcpy(&default_hsl_lightness, msg->msg->buf, 2);
            memcpy(&default_hsl_hue, msg->msg->buf + 2, 2);
            memcpy(&default_hsl_sat, msg->msg->buf + 4, 2);
            BTMW_RPC_TEST_Logi("[mesh] BTMW_RPC_LIGHT_HSL_DEFAULT_STATUS, msg->model_handle = %d, default_hsl_lightness = %04x, default_hsl_hue = %04x, default_hsl_sat = %04x\n",
                                msg->model_handle, default_hsl_lightness, default_hsl_hue, default_hsl_sat);
            break;
        }
        default:
            return;
    }

    return;

}

static VOID _btmw_rpc_test_mesh_config_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{
    UINT16 src_addr = 0;
    BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *resend_msg_entry = NULL;

    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] ConfigClientRx opcode = 0x%04x, src:0x%x, dst:0x%x, appkey_index:0x%x, netkey_index:0x%x, ttl:%d\n", \
        msg->msg->opcode.opcode, msg->msg->meta_data.src_addr, msg->msg->meta_data.dst_addr, msg->msg->meta_data.appkey_index, \
        msg->msg->meta_data.netkey_index, msg->msg->meta_data.ttl);

    if (BT_MESH_ROLE_PROVISIONEE == btmw_rpc_mesh_role)
    {
        return;
    }

    src_addr = msg->msg->meta_data.src_addr;
    switch(msg->msg->opcode.opcode)
    {
        case BT_MESH_ACCESS_MSG_CONFIG_BEACON_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_BEACON_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_COMPOSITION_DATA_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);

                _btmw_rpc_test_mesh_check_msg_resend_stop(BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET, \
                                                        msg->msg->meta_data.src_addr,   \
                                                        msg->msg->meta_data.dst_addr);

            if ((src_addr == g_curr_prov_node.addr)
                && ((g_curr_prov_node.config_state != BTMW_RPC_TEST_MESH_CONFIG_STATE_GET_COMPOSITION_DATA)
                    &&(g_curr_prov_node.config_state != BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE)))
            {
                BTMW_RPC_TEST_Logd("[mesh] current prov node is in other config state, return\n");
                return;

            }
            else
            {
                if (binding_model_list != NULL) {
                    util_queue_free(binding_model_list);
                    binding_model_list = NULL;
                }
                BT_MESH_COMP_DATA_T *composition_data = _btmw_rpc_test_mesh_model_parse_composition_data(msg->msg->buf + 1, msg->msg->buf_len - 1);
                if (composition_data != NULL) {
                    _btmw_rpc_test_mesh_handle_composition_data(composition_data);

                    _btmw_rpc_test_mesh_model_free_composition_data(composition_data);
                }
            }

            if ((src_addr == g_curr_prov_node.addr) && (g_curr_prov_node.config_state == BTMW_RPC_TEST_MESH_CONFIG_STATE_GET_COMPOSITION_DATA))
            {
                g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_ADD_APPKEY;
                _btmw_rpc_test_mesh_cc_config_appkey_add(btmw_rpc_appidx, btmw_rpc_prov_cur_netidx,
                                                  btmw_rpc_appkey, g_curr_prov_node.addr);
            }

            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_DEFAULT_TTL_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_GATT_PROXY_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_FRIEND_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_FRIEND_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_MODEL_PUBLICATION_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_MODEL_SUBSCRIPTION_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);

            _btmw_rpc_test_mesh_check_msg_resend_stop(BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD, \
                                                    msg->msg->meta_data.src_addr,   \
                                                    msg->msg->meta_data.dst_addr);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_NETWORK_TRANSMIT_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_RELAY_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_RELAY_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_SIG_MODEL_SUBSCRIPTION_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_NETKEY_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_STATUS:
        {
            UINT16 netkey_index = msg->msg->buf[1] + (msg->msg->buf[2] << 8);

            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_NETKEY_STATUS, %s, status code=0x%02x, netkey_idx=0x%04x\n", \
                (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0], netkey_index);

            BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = NULL;
            BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;
            BOOL b_all_reply = FALSE;

            pthread_mutex_lock(&resend_msg_mutex);
            resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
            while (resend_msg_entry != NULL)
            {
                if ((resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD)
                    || (resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE)
                    || (resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_NETKEY_DELETE))
                {
                    if ((resend_msg_entry->dst == msg->msg->meta_data.src_addr)
                        && (resend_msg_entry->src == msg->msg->meta_data.dst_addr)
                        && (resend_msg_entry->resend_cnt > 0))
                    {
                        BTMW_RPC_TEST_Logd("[mesh] Peer replies msg [0x%04x][id:0x%x], stop resend it\n", resend_msg_entry->opcode, resend_msg_entry->msg_id);
                        //just set the resend_cnt to 0, it will be removed in the msg resend thread
                        resend_msg_entry->resend_cnt = 0;
                        break;
                    }
                }
                resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_next(btmw_rpc_test_resend_msg_list, resend_msg_entry);
            }
            if (resend_msg_entry == NULL)
            {
                BTMW_RPC_TEST_Loge("[mesh] Peer replies msg , entry is NULL\n");
            }
            pthread_mutex_unlock(&resend_msg_mutex);


            netkey = _btmw_rpc_test_mesh_get_netkey_by_idx(g_netkey_list, netkey_index);
            if (NULL == netkey)
            {
                BTMW_RPC_TEST_Loge("[mesh] Key not found!!!\n");
                return;
            }
            if (msg->msg->buf[0] == BTMW_RPC_TEST_MESH_ACCESS_MSG_STATUS_SUCCESS)
            {
                if (netkey->state == BT_MESH_KEY_REFRESH_STATE_1)
                {
                    node = _btmw_rpc_test_mesh_get_node_by_addr(netkey->kr_node_list, msg->msg->meta_data.src_addr);
                    if (NULL == node)
                    {
                        BTMW_RPC_TEST_Loge("[mesh] Node not found!!!\n");
                        return;
                    }
                    _btmw_rpc_test_mesh_add_node_to_list(netkey->kr_node_ack_list, node);
                    if (util_dlist_count(netkey->kr_node_ack_list) >= util_dlist_count(netkey->kr_node_list))
                    {
                        //check whether all the kr_node response ack.
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(netkey->kr_node_list);
                        while (node != NULL)
                        {
                            if (NULL == _btmw_rpc_test_mesh_get_node_by_addr(netkey->kr_node_ack_list, node->addr))
                            {
                                BTMW_RPC_TEST_Logi("[mesh] Node 0x%04x does not reply yet\n", node->addr);
                                break;
                            }
                            node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(netkey->kr_node_list, node);
                        }
                        if (NULL == node)   //All the nodes reply
                        {
                            b_all_reply = TRUE;
                        }
                    }
                    if (b_all_reply)
                    {
                        util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID].handle);
                        //Trigger to switch to Key Refresh Phase2,  BT_MESH_EVT_KEY_REFRESH event with BT_MESH_KEY_REFRESH_STATE_2 will be received.
                        BT_MESH_NETKEY_T netk;
                        memset(&netk, 0, sizeof(netk));
                        netk.opcode = BT_MESH_KEY_USE_NEW_NETKEY;
                        netk.key_index = netkey_index;
                        netk.network_key = netkey->temp_netkey;  //key value is useless here actually
                        a_mtkapi_bt_mesh_set_netkey(&netk);
                    }
                }
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_APPKEY_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_APPKEY_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);

            //check resend stop
                pthread_mutex_lock(&resend_msg_mutex);
                resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
                while (resend_msg_entry != NULL)
                {
                    if ((resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD) || (resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE))
                    {
                        if ((resend_msg_entry->dst == msg->msg->meta_data.src_addr)
                            && (resend_msg_entry->src == msg->msg->meta_data.dst_addr)
                            && (resend_msg_entry->resend_cnt > 0))
                        {
                            BTMW_RPC_TEST_Logd("[mesh] Peer replies msg [0x%04x][id:0x%x], stop resend it\n", resend_msg_entry->opcode, resend_msg_entry->msg_id);
                            //just set the resend_cnt to 0, it will be removed in the msg resend thread
                            resend_msg_entry->resend_cnt = 0;
                            break;
                        }
                    }
                    resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_next(btmw_rpc_test_resend_msg_list, resend_msg_entry);
                }
                if (resend_msg_entry == NULL)
                {
                    BTMW_RPC_TEST_Loge("[mesh] Peer replies msg , entry is NULL\n");
                }
                pthread_mutex_unlock(&resend_msg_mutex);

            if ((src_addr == g_curr_prov_node.addr) && (g_curr_prov_node.config_state == BTMW_RPC_TEST_MESH_CONFIG_STATE_ADD_APPKEY))
            {
                if (msg->msg->buf[0] == BTMW_RPC_TEST_MESH_ACCESS_MSG_STATUS_SUCCESS)
                {
                    /* start binding model and application key */
                    BTMW_RPC_TEST_MESH_BINDING_MODEL_T *entry = util_queue_first(binding_model_list);
                    if (entry != NULL)
                    {
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_MODEL_APP_BIND;
                        //The element address depends on the provisionee device. It is the element address which contains the model to be bound.
                        _btmw_rpc_test_mesh_cc_config_model_app_bind(g_curr_prov_node.addr + entry->element_index,
                                                                  btmw_rpc_appidx, g_curr_prov_node.addr, entry->model_id);

                        BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_model_app_bind, element_index = %d, model_id = %d\n",
                                           entry->element_index, entry->model_id);
                    }
                    else
                    {
                        util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID].handle);
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE;
                        BTMW_RPC_TEST_Loge("[mesh] no more model, all model binding is done\n");
                    }
                }
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_MODEL_APP_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            UINT16 element_addr = msg->msg->buf[1] + (msg->msg->buf[2] << 8);
            UINT16 appkey_idx = msg->msg->buf[3] + (msg->msg->buf[4] << 8);
            UINT32 model_id = 0;
            if (7 == msg->msg->buf_len) //SIG model
            {
                model_id = msg->msg->buf[5] + (msg->msg->buf[6] << 8);
                BTMW_RPC_TEST_Logd("[mesh] element_addr:0x%x, appkey_idx:0x%x, SIG model_id:0x%x\n", element_addr, appkey_idx, model_id);
            }
            else if (9 == msg->msg->buf_len)    //Vendor model
            {
                model_id = (msg->msg->buf[5] << 16 | (msg->msg->buf[6] << 24)) | ((msg->msg->buf[7]) | (msg->msg->buf[8] << 8));
                BTMW_RPC_TEST_Logd("[mesh] element_addr:0x%x, appkey_idx:0x%x, Vendor model_id:0x%x\n", element_addr, appkey_idx, model_id);
            }
            else
            {
                BTMW_RPC_TEST_Loge("[mesh] Invalid MODEL_APP_STATUS message\n");
            }

            if ((src_addr == g_curr_prov_node.addr) && (g_curr_prov_node.config_state == BTMW_RPC_TEST_MESH_CONFIG_STATE_MODEL_APP_BIND))
            {
                BTMW_RPC_TEST_MESH_BINDING_MODEL_T *current_bind_model = util_queue_first(binding_model_list);
                if (NULL != current_bind_model)
                {
                    if (current_bind_model->model_id != model_id)    //It shall NOT happen if no resend happens
                    {
                        BTMW_RPC_TEST_Loge("[mesh] Model dismatch [TX:0x%x vs RX:0x%x]\n", current_bind_model->model_id, model_id);
                        return;
                    }
                }
            }

            //check resend stop
            pthread_mutex_lock(&resend_msg_mutex);
            resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
            while (resend_msg_entry != NULL)
            {
                if ((resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND) || (resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND))
                {
                    if ((resend_msg_entry->dst == msg->msg->meta_data.src_addr)
                        && (resend_msg_entry->src == msg->msg->meta_data.dst_addr)
                        && (resend_msg_entry->resend_cnt > 0))
                    {
                        BTMW_RPC_TEST_Logd("[mesh] Peer replies msg [0x%04x][id:0x%x], stop resend it\n", resend_msg_entry->opcode, resend_msg_entry->msg_id);
                        //just set the resend_cnt to 0, it will be removed in the msg resend thread
                        resend_msg_entry->resend_cnt = 0;
                        break;
                    }
                }
                resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_next(btmw_rpc_test_resend_msg_list, resend_msg_entry);
            }
            if (resend_msg_entry == NULL)
            {
                BTMW_RPC_TEST_Loge("[mesh] Peer replies msg , entry is NULL\n");
            }
            pthread_mutex_unlock(&resend_msg_mutex);

            if ((src_addr == g_curr_prov_node.addr) && (g_curr_prov_node.config_state == BTMW_RPC_TEST_MESH_CONFIG_STATE_MODEL_APP_BIND))
            {
                if (msg->msg->buf[0] == BTMW_RPC_TEST_MESH_ACCESS_MSG_STATUS_SUCCESS)
                {
                        /* continue binding model and application key */
                        BTMW_RPC_TEST_MESH_BINDING_MODEL_T *entry = util_queue_pop(binding_model_list);
                        util_queue_entry_free(entry);
                        entry = util_queue_first(binding_model_list);
                    if (entry != NULL)
                    {
                            BTMW_RPC_TEST_Logd("[mesh] element_addr 0x%x model 0x%x app 0x%x bind SUCCESS, goto next --> element_index = %d, model_id = 0x%x\n",
                                               element_addr, model_id, appkey_idx, entry->element_index, entry->model_id);
                        _btmw_rpc_test_mesh_cc_config_model_app_bind(g_curr_prov_node.addr + entry->element_index,
                                                                  btmw_rpc_appidx, g_curr_prov_node.addr, entry->model_id);
                    }
                    else
                    {
                        util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID].handle);
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE;
                        BTMW_RPC_TEST_Logd("[mesh] [AT]configuration done, SUCCESS. device: 0x%04x\n", g_curr_prov_node.addr);
                        //Heartbeat publication set
                        _btmw_rpc_test_mesh_cc_config_hb_pub_set(src_addr, BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX, msg->msg->meta_data.dst_addr,
                                0xFF, 6, a_mtkapi_bt_mesh_get_default_ttl(), 0xF, BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
                        _btmw_rpc_test_mesh_cc_config_model_suscription_add(g_curr_prov_node.addr, BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX,
                            g_curr_prov_node.addr, 0xC001, BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER);
                    }
                }
                else
                {
                        BTMW_RPC_TEST_Loge("[mesh] [AT]configuration, FAILED, error code: %d,try next model\n", msg->msg->buf[0]);
                        BTMW_RPC_TEST_MESH_BINDING_MODEL_T *entry = util_queue_pop(binding_model_list);
                        util_queue_entry_free(entry);
                        entry = util_queue_first(binding_model_list);
                    if (entry != NULL)
                    {
                            BTMW_RPC_TEST_Logd("[mesh] element_addr 0x%x model 0x%x app 0x%x bind FAILED, goto next --> element_index = %d, model_id = 0x%x\n",
                                               element_addr, model_id, appkey_idx, entry->element_index, entry->model_id);
                        _btmw_rpc_test_mesh_cc_config_model_app_bind(g_curr_prov_node.addr + entry->element_index,
                                                                  btmw_rpc_appidx, g_curr_prov_node.addr, entry->model_id);
                    }
                    else
                    {
                        util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID].handle);
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE;
                        BTMW_RPC_TEST_Logd("[mesh] [AT]configuration done, FAILED. device: 0x%04x\n", g_curr_prov_node.addr);
                    }
                }
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_SIG_MODEL_APP_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_LIST:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_VENDOR_MODEL_APP_LIST, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_NODE_IDENTITY_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_NODE_RESET_STATUS\n");
            _btmw_rpc_test_mesh_check_msg_resend_stop(BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET, \
                                                        msg->msg->meta_data.src_addr,   \
                                                        msg->msg->meta_data.dst_addr);

            BT_MESH_DEVICE_INFO_T info;

            memset(&info, 0, sizeof(BT_MESH_DEVICE_INFO_T));
            info.opcode = BT_MESH_DEV_INFO_OP_DELETE_DEVKEY;
            info.data.devkey.unicast_addr = msg->msg->meta_data.src_addr;
            a_mtkapi_bt_mesh_set_device_info(&info);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_STATUS:
        {
            UINT16 netkey_index = msg->msg->buf[1] +(msg->msg->buf[2] << 8);
            UINT8 phase = msg->msg->buf[3];
            BOOL b_all_reply = FALSE;

            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_KEY_REFRESH_PHASE_STATUS, %s, status code=0x%02x, netkey_idx=0x%04x, phase:%d\n", \
                (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0], netkey_index, phase);

            BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = NULL;
            BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;

            pthread_mutex_lock(&resend_msg_mutex);
            resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_first(btmw_rpc_test_resend_msg_list);
            while (resend_msg_entry != NULL)
            {
                if ((resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET)
                    || (resend_msg_entry->opcode == BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET))
                {
                    if ((resend_msg_entry->dst == msg->msg->meta_data.src_addr)
                            && (resend_msg_entry->src == msg->msg->meta_data.dst_addr)
                            && (resend_msg_entry->resend_cnt > 0))
                    {
                        BTMW_RPC_TEST_Logd("[mesh] Peer replies msg [0x%04x][id:0x%x], stop resend it\n", resend_msg_entry->opcode, resend_msg_entry->msg_id);
                        //just set the resend_cnt to 0, it will be removed in the msg resend thread
                        resend_msg_entry->resend_cnt = 0;
                        break;
                    }
                }
                resend_msg_entry = (BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T *)util_dlist_next(btmw_rpc_test_resend_msg_list, resend_msg_entry);
            }
            if (resend_msg_entry == NULL)
            {
                BTMW_RPC_TEST_Loge("[mesh] Peer replies msg , entry is NULL\n");
            }
            pthread_mutex_unlock(&resend_msg_mutex);

            netkey = _btmw_rpc_test_mesh_get_netkey_by_idx(g_netkey_list, netkey_index);
            if (NULL == netkey)
            {
                BTMW_RPC_TEST_Loge("[mesh] Key not found!!!\n");
                return;
            }
            if (msg->msg->buf[0] == BTMW_RPC_TEST_MESH_ACCESS_MSG_STATUS_SUCCESS)
            {
                if ((netkey->state == BT_MESH_KEY_REFRESH_STATE_2) || (netkey->state == BT_MESH_KEY_REFRESH_STATE_NONE))
                {
                    node = _btmw_rpc_test_mesh_get_node_by_addr(netkey->kr_node_list, msg->msg->meta_data.src_addr);
                    if (NULL == node)
                    {
                        BTMW_RPC_TEST_Loge("[mesh] Node not found!!!\n");
                        return;
                    }
                    _btmw_rpc_test_mesh_add_node_to_list(netkey->kr_node_ack_list, node);
                    if (util_dlist_count(netkey->kr_node_ack_list) >= util_dlist_count(netkey->kr_node_list))
                    {
                        //check whether all the kr_node response ack.
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(netkey->kr_node_list);
                        while (node != NULL)
                        {
                            if (NULL == _btmw_rpc_test_mesh_get_node_by_addr(netkey->kr_node_ack_list, node->addr))
                            {
                                BTMW_RPC_TEST_Logi("[mesh] Node 0x%04x does not reply yet\n", node->addr);
                                break;
                            }
                            node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(netkey->kr_node_list, node);
                        }
                        if (NULL == node)   //All the nodes reply
                        {
                            b_all_reply = TRUE;
                        }
                    }
                    if (b_all_reply)
                    {
                        if (netkey->state == BT_MESH_KEY_REFRESH_STATE_2)
                        {
                            util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID].handle);
                            //Trigger to switch to Key Refresh Phase3,  BT_MESH_EVT_KEY_REFRESH event with BT_MESH_KEY_REFRESH_STATE_NONE will be received.
                            BT_MESH_NETKEY_T netk;
                            memset(&netk, 0, sizeof(netk));
                            netk.opcode = BT_MESH_KEY_REVOKE_OLD_NETKEY;
                            netk.key_index = netkey_index;
                            netk.network_key = netkey->temp_netkey;  //key value is useless here actually
                            a_mtkapi_bt_mesh_set_netkey(&netk);
                        }
                        else if (netkey->state == BT_MESH_KEY_REFRESH_STATE_NONE)
                        {
                            util_timer_stop(btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID].handle);
                            BTMW_RPC_TEST_Logd("[mesh] Key refresh for NetKey 0x%x SUCCESS\n", netkey->keyidx);
                            netkey->state = BT_MESH_KEY_REFRESH_STATE_NONE;
                            util_dlist_empty(netkey->kr_node_list, TRUE);
                            util_dlist_empty(netkey->kr_node_ack_list, TRUE);
                        }
                    }
                }
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_HEARTBEAT_PUBLICATION_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            if (FALSE == _btmw_rpc_test_mesh_check_msg_resend_stop(BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET, \
                                                        msg->msg->meta_data.src_addr,   \
                                                        msg->msg->meta_data.dst_addr))
            {
                _btmw_rpc_test_mesh_check_msg_resend_stop(BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET, \
                                                            msg->msg->meta_data.src_addr,   \
                                                            msg->msg->meta_data.dst_addr);
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS:
        {
            BTMW_RPC_TEST_Logd("[mesh] [AT]CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS, %s, status code=0x%02x\n", (0 == msg->msg->buf[0]) ? "SUCCESS" : "FAILED", msg->msg->buf[0]);
            break;
        }
        default:
        {
            return;
        }
    }
}

static VOID _btmw_rpc_test_mesh_health_client_msg_handler(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event, VOID *pv_tag)
{
    if ((NULL == event) || (NULL == event->msg) || (NULL == event->event))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }

    BTMW_RPC_TEST_Logi("[mesh] HealthClient msg->opcode = 0x%x", event->msg->opcode.opcode);
    switch (event->msg->opcode.opcode)
    {
        case BT_MESH_ACCESS_MSG_HEALTH_CURRENT_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] HEALTH_CURRENT_STATUS test_id:%d, fault_array_length=%d\n", \
                event->event->data.current_status.test_id, event->event->data.current_status.fault_array_length);
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_FAULT_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] HEALTH_FAULT_STATUS test_id:%d, fault_array_length=%d\n", \
                event->event->data.fault_status.test_id,  event->event->data.fault_status.fault_array_length);
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_PERIOD_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] HEALTH_PERIOD_STATUS fast_period_divisor:%d\n", event->event->data.period_status.fast_period_divisor);
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_STATUS:
            BTMW_RPC_TEST_Logi("[mesh] HEALTH_ATTENTION_STATUS attention:%d\n", event->event->data.attention_status.attention);
            break;
        default:
            break;
    }
}

static VOID _btmw_rpc_test_mesh_ota_client_msg_handler(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag)
{
    if ((NULL == msg) || (NULL == msg->msg))
    {
        BTMW_RPC_TEST_Logi("[mesh] %s invalid ARG, return!!!\n", __func__);
        return;
    }

    BTMW_RPC_TEST_Logi("[mesh] otaClient msg->opcode = 0x%x", (msg->msg->opcode.opcode));
    switch (msg->msg->opcode.opcode)
    {
        case 0xB602:
            BTMW_RPC_TEST_Logi("[mesh] FW_INFO_STATUS data=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", \
                msg->msg->buf[0], msg->msg->buf[1],msg->msg->buf[2],msg->msg->buf[3],msg->msg->buf[4],msg->msg->buf[5]);
            break;
        default:
            break;
    }
}
#if 0
static _btmw_rpc_test_mesh_btaddr_htos(UINT8 *bdaddr_h, CHAR *btaddr_s)
{
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    snprintf(btaddr_s, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             bdaddr_h[0],
             bdaddr_h[1],
             bdaddr_h[2],
             bdaddr_h[3],
             bdaddr_h[4],
             bdaddr_h[5]);
    btaddr_s[17] = '\0';

    return BT_SUCCESS;
}
#endif

static VOID _btmw_rpc_test_mesh_event_callback(BT_MESH_BT_EVENT_T *bt_event, VOID *pv_tag)
{
    if (NULL == bt_event)
    {
        BTMW_RPC_TEST_Logi("[mesh] %s, event is NULL, return\n",__func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[mesh] %s(), bt_mesh_event =%d\n",  __func__, bt_event->evt_id);

    switch (bt_event->evt_id)
    {
        case BT_MESH_EVT_INIT_DONE:
            a_mtkapi_bt_mesh_generate_uuid(btmw_rpc_deviceUuid);
            BTMW_RPC_TEST_Logi("[mesh]uuid:%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n", \
                btmw_rpc_deviceUuid[0], btmw_rpc_deviceUuid[1], btmw_rpc_deviceUuid[2], btmw_rpc_deviceUuid[3], \
                btmw_rpc_deviceUuid[4], btmw_rpc_deviceUuid[5], btmw_rpc_deviceUuid[6], btmw_rpc_deviceUuid[7], \
                btmw_rpc_deviceUuid[8], btmw_rpc_deviceUuid[9], btmw_rpc_deviceUuid[10], btmw_rpc_deviceUuid[11], \
                btmw_rpc_deviceUuid[12], btmw_rpc_deviceUuid[13], btmw_rpc_deviceUuid[14], btmw_rpc_deviceUuid[15]);
            break;
        case BT_MESH_EVT_PROV_CAPABILITIES:
        {
            BT_MESH_EVT_PROV_CAPABILITIES_T *p = &(bt_event->evt.mesh_evt.mesh.prov_cap);
            BTMW_RPC_TEST_Logi("[mesh] BLE_MESH_EVT_PROV_CAPABILITIES\n");
            BTMW_RPC_TEST_Logi("[mesh] \tNumOfElements: 0x%02x\n", p->cap.number_of_elements);
            BTMW_RPC_TEST_Logi("[mesh] \tAlgorithms: 0x%04x\n", p->cap.algorithms);
            BTMW_RPC_TEST_Logi("[mesh] \tPublicKeyType: 0x%02x\n", p->cap.public_key_type);
            BTMW_RPC_TEST_Logi("[mesh] \tStaticOobType: 0x%02x\n", p->cap.static_oob_type);
            BTMW_RPC_TEST_Logi("[mesh] \tOutputOobSize: 0x%02x\n", p->cap.output_oob_size);
            BTMW_RPC_TEST_Logi("[mesh] \tOutputOobAction: 0x%04x\n", p->cap.output_oob_action);
            BTMW_RPC_TEST_Logi("[mesh] \tInputOobSize: 0x%02x\n", p->cap.input_oob_size);
            BTMW_RPC_TEST_Logi("[mesh] \tInputOobAction: 0x%04x\n", p->cap.input_oob_action);
            if (p->cap.static_oob_type == BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED)
            {
                btmw_rpc_prov_params.start.authentication_method = g_auth_method;
            }
            else
            {
                btmw_rpc_prov_params.start.authentication_method = BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_NO_OOB;
            }
            a_mtkapi_bt_mesh_start_provisioning(&btmw_rpc_prov_params, 0);
            break;
        }
        case BT_MESH_EVT_PROV_REQUEST_OOB_PUBLIC_KEY:
            break;
        case BT_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE:
        {
            BTMW_RPC_TEST_Logi("[mesh] BLE_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE\n");
            BT_MESH_EVT_PROV_REQUEST_AUTH_T *p = &(bt_event->evt.mesh_evt.mesh.prov_request_auth);
            BTMW_RPC_TEST_Logi("[mesh] \tMethod : %d\n", p->method);
            BTMW_RPC_TEST_Logi("[mesh] \tAction : %d\n", p->action);
            BTMW_RPC_TEST_Logi("[mesh] \tSize   : %d\n", p->size);
            switch(p->method)
            {
                case BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB:
                {
                    BTMW_RPC_TEST_Logi("[mesh] \tMethod : Static OOB\n");
                    UINT8 auth_value[16] =
                    {
                        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                    };
                    UINT8 i = 0;
                    while (device_db[i].bt_addr != NULL)
                    {
                        if (memcmp(device_db[i].uuid, g_curr_prov_node.uuid, BT_MESH_UUID_SIZE) == 0)
                        {
                            BTMW_RPC_TEST_Logi("[mesh] find auth value from device_db\n");
                            memcpy(auth_value, device_db[i].auth_value, BT_MESH_UUID_SIZE);
                            break;
                        }
                        i++;
                    }
                    BT_MESH_PROV_FACTOR_T factor;
                    factor.type = BT_MESH_PROV_FACTOR_AUTHEN_VALUE;
                    factor.buf = auth_value;
                    factor.buf_len = sizeof(auth_value);
                    a_mtkapi_bt_mesh_set_prov_factor(&factor);
                    break;
                }
                case BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_OUTPUT_OOB:
                    BTMW_RPC_TEST_Logi("[mesh] \tMethod : Ouput OOB\n");
                    break;
                case BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_INPUT_OOB:
                    BTMW_RPC_TEST_Logi("[mesh] \tMethod : Input OOB\n");
                    break;
                default:
                    BTMW_RPC_TEST_Logi("[mesh] \tMethod : %d, invalid\n", p->method);
                    break;
            }
            break;
        }
        case BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY:
        {
            BT_MESH_EVT_PROV_SHOW_PK_T *p = &(bt_event->evt.mesh_evt.mesh.prov_show_pk);
            BTMW_RPC_DUMP_PDU("PublicKey", 64, p->pk);
            break;
        }
        case BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE:
            break;
        case BT_MESH_EVT_PROV_DONE:
        {
            BT_MESH_EVT_PROV_DONE_T *p = &(bt_event->evt.mesh_evt.mesh.prov_done);
            BTMW_RPC_TEST_Logi("[mesh] BLE_MESH_EVT_PROV_DONE %s, reason %d\n", p->success ? "SUCCESS" : "FAILED", p->reason);
            if (p->success)
            {
                BTMW_RPC_TEST_Logi("[mesh] \tUnicastAddr: 0x%lx\n", p->address);
                BTMW_RPC_TEST_Logi("[mesh] \t");
                BTMW_RPC_DUMP_PDU("DeviceKey", BT_MESH_DEVKEY_SIZE, p->device_key);
                BTMW_RPC_TEST_Logi("[mesh] start to configure provisioned device...\n");
                g_GattBearer = p->gatt_bearer;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    BTMW_RPC_TEST_MESH_NODE_ENTRY_T new_node;
                    new_node.addr = p->address;
                    memcpy(new_node.uuid, g_curr_prov_node.uuid, BT_MESH_UUID_SIZE);
                    memcpy(new_node.dev_key, p->device_key, BT_MESH_DEVKEY_SIZE);
                    new_node.in_blacklist = FALSE;
                    _btmw_rpc_test_mesh_add_node_to_list(g_node_list, &new_node);

                    if (FALSE == p->gatt_bearer)
                    {
                        util_timer_start(&btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID]);
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_GET_COMPOSITION_DATA;
                        _btmw_rpc_test_mesh_cc_config_composition_data_get(g_curr_prov_node.addr, 0);
                    }
                    else
                    {
                        g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE;
                    }
                }
            }
            break;
        }
        case BT_MESH_EVT_PROV_SCAN_UD_RESULT:
        {
            BT_MESH_EVT_PROV_SCAN_UD_T ud;
            memset(&ud, 0, sizeof(BT_MESH_EVT_PROV_SCAN_UD_T));
            memcpy(&ud, &bt_event->evt.mesh_evt.mesh.prov_scan_ud, sizeof(BT_MESH_EVT_PROV_SCAN_UD_T));
            BTMW_RPC_TEST_Logi("[mesh] PROV_SCAN_UD_RESULT, current/total = (%d/%d)\n", ud.current_index, ud.total_count);
            BTMW_RPC_TEST_Logi("[mesh] [AT]PROV_SCAN_UD_RESULT, uuid = [%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X]\n",
                                ud.ud.uuid[0], ud.ud.uuid[1], ud.ud.uuid[2], ud.ud.uuid[3],
                                ud.ud.uuid[4], ud.ud.uuid[5], ud.ud.uuid[6], ud.ud.uuid[7],
                                ud.ud.uuid[8], ud.ud.uuid[9], ud.ud.uuid[10], ud.ud.uuid[11],
                                ud.ud.uuid[12], ud.ud.uuid[13], ud.ud.uuid[14], ud.ud.uuid[15]);
            break;
        }
        case BT_MESH_EVT_CONFIG_RESET:
            break;
        case BT_MESH_EVT_UD_RESULT_COMPLETE:
            break;
        case BT_MESH_EVT_PROV_FACTOR:
        {
            BTMW_RPC_TEST_Logi("[mesh] BT_MESH_EVT_PROV_FACTOR, factor type:%d, buf_len:%d\n", \
                bt_event->evt.mesh_evt.mesh.prov_factor.type, bt_event->evt.mesh_evt.mesh.prov_factor.buf_len);
            break;
        }
        case BT_MESH_EVT_ADV_REPORT:
        {
            BTMW_RPC_TEST_Logi("[mesh] report type = %d\n", \
                               bt_event->evt.mesh_evt.mesh.adv_report.type);
            BTMW_RPC_TEST_Logi("[mesh] addrtype = %d, peer_addr = %s\n", \
                               bt_event->evt.mesh_evt.mesh.adv_report.peer_addr.addr_type,
                               bt_event->evt.mesh_evt.mesh.adv_report.peer_addr.addr);
            break;
        }
        case BT_MESH_EVT_KEY_REFRESH:
        {
            BTMW_RPC_TEST_Logi("[mesh] BT_MESH_EVT_KEY_REFRESH, netkey_idx:0x%x, phase:%d\n", \
                bt_event->evt.mesh_evt.mesh.key_refresh.netkey_index, bt_event->evt.mesh_evt.mesh.key_refresh.phase);
            if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
            {
                BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = NULL;
                netkey = _btmw_rpc_test_mesh_get_netkey_by_idx(g_netkey_list, bt_event->evt.mesh_evt.mesh.key_refresh.netkey_index);
                if (NULL == netkey)
                {
                    BTMW_RPC_TEST_Loge("[mesh] Key not found!!!\n");
                    return;
                }

                if (BT_MESH_KEY_REFRESH_STATE_1 == bt_event->evt.mesh_evt.mesh.key_refresh.phase)
                {
                    BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;
                    UINT16 kr_node_count = 0;

                    netkey->state = bt_event->evt.mesh_evt.mesh.key_refresh.phase;

                    //add whitelist nodes to the netkey key refresh node list
                    BTMW_RPC_TEST_Logi("[mesh] BT_MESH_KEY_REFRESH_STATE_1 node count:%d\n", util_dlist_count(g_node_list));
                    node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(g_node_list);
                    while (node != NULL)
                    {
                        BTMW_RPC_TEST_Logi("[mesh] node->addr = 0x%x, in_blacklist:%d\n", node->addr, node->in_blacklist);
                        if (!node->in_blacklist)
                        {
                            _btmw_rpc_test_mesh_add_node_to_list(netkey->kr_node_list, node);
                        }
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(g_node_list, node);
                    }
                    // Distribute new key to the whitelist nodes, Note, we shall build the kr_node_list completely so that we can check the ACK safely
                    kr_node_count = util_dlist_count(netkey->kr_node_list);
                    BTMW_RPC_TEST_Logi("[mesh] netkey->kr_node_list count = %d\n", kr_node_count);

                    node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(netkey->kr_node_list);
                    while (node != NULL)
                    {
                        _btmw_rpc_test_mesh_cc_config_netkey_update(node->addr, bt_event->evt.mesh_evt.mesh.key_refresh.netkey_index, netkey->temp_netkey);
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(netkey->kr_node_list, node);
                    }
                    //The timeout value shall be adjusted according to the nodes number. And Each key refresh procedure shall owner a timer
                    btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID].pv_args = (VOID *)netkey;
                    util_timer_start(&btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID]);
                }
                else if (BT_MESH_KEY_REFRESH_STATE_2 == bt_event->evt.mesh_evt.mesh.key_refresh.phase)
                {
                    BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;

                    netkey->state = bt_event->evt.mesh_evt.mesh.key_refresh.phase;

                    util_dlist_empty(netkey->kr_node_ack_list, FALSE);

                    UINT8 temp[BT_MESH_KEY_SIZE] = {0};
                    memcpy(temp, netkey->netkey, BT_MESH_KEY_SIZE);
                    memcpy(netkey->netkey, netkey->temp_netkey, BT_MESH_KEY_SIZE);
                    memcpy(netkey->temp_netkey, temp, BT_MESH_KEY_SIZE);

                    // Send Config Key Refresh Phase Set (Transition = 0x02) to all the kr_node
                    node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(netkey->kr_node_list);
                    while (node != NULL)
                    {
                        _btmw_rpc_test_mesh_cc_config_key_refresh_phase_set(node->addr, \
                                                    bt_event->evt.mesh_evt.mesh.key_refresh.netkey_index, \
                                                    BT_MESH_KEY_REFRESH_STATE_2);
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(netkey->kr_node_list, node);
                    }
                    //The timeout value shall be adjusted according to the nodes number. And Each key refresh procedure shall owner a timer
                    btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID].pv_args = (VOID *)netkey;
                    util_timer_start(&btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID]);
                }
                else if (BT_MESH_KEY_REFRESH_STATE_NONE == bt_event->evt.mesh_evt.mesh.key_refresh.phase)
                {
                    BTMW_RPC_TEST_MESH_NODE_ENTRY_T *node = NULL;

                    if (netkey->state == BT_MESH_KEY_REFRESH_STATE_2)
                    {
                        util_dlist_empty(netkey->kr_node_ack_list, FALSE);
                        // Send Config Key Refresh Phase Set (Transition = 0x03) to all the kr_node
                        node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(netkey->kr_node_list);
                        while (node != NULL)
                        {
                            _btmw_rpc_test_mesh_cc_config_key_refresh_phase_set(node->addr, \
                                                        bt_event->evt.mesh_evt.mesh.key_refresh.netkey_index, \
                                                        BT_MESH_KEY_REFRESH_STATE_3);
                            node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(netkey->kr_node_list, node);
                        }
                        //The timeout value shall be adjusted according to the nodes number. And Each key refresh procedure shall owner a timer
                        btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID].pv_args = (VOID *)netkey;
                        util_timer_start(&btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID]);

                        netkey->state = bt_event->evt.mesh_evt.mesh.key_refresh.phase;
                    }
                }
                else
                {
                }
            }
            break;
        }
        case BT_MESH_EVT_HEARTBEAT:
        {
            BTMW_RPC_TEST_Logi("[mesh] BT_MESH_EVT_HEARTBEAT address = 0x%04x, active = 0x%02x\n", \
                               bt_event->evt.mesh_evt.mesh.heartbeat.address,
                               bt_event->evt.mesh_evt.mesh.heartbeat.active);
            break;
        }
        case BT_MESH_EVT_IV_UPDATE:
        {
            BTMW_RPC_TEST_Logi("[mesh] ivindxie = %d, state = %d, phase = %d\n", \
                               bt_event->evt.mesh_evt.mesh.iv_update.iv_index,
                               bt_event->evt.mesh_evt.mesh.iv_update.state,
                               bt_event->evt.mesh_evt.mesh.iv_update.iv_phase);
            break;
        }
        case BT_MESH_EVT_SEQ_CHANGE:
        {
            BTMW_RPC_TEST_Logi("[mesh] seq_num = %d\n", \
                               bt_event->evt.mesh_evt.mesh.seq_change.seq_num);
            break;
        }
        case BT_MESH_EVT_OTA_EVENT:
        {
            UINT16 success = 0, in_progress = 0, cancelled = 0, dfu_ready = 0;
            BT_MESH_EVT_OTA_T *ota_event = &bt_event->evt.mesh_evt.mesh.ota_evt;

            BTMW_RPC_TEST_Logd("[mesh] BT_MESH_EVT_OTA_EVENT\n");
            BTMW_RPC_TEST_Logi("[mesh] event_id: %d, error_code: %d, serial_num: 0x%x, fw_id: 0x%x, block:%d/%d, curr_chunk:%d, chunk_mask:0x%04x, time_escaped: %ds(%dh.%dm.%ds)\n", \
                ota_event->event_id, ota_event->error_code, ota_event->serial_number, ota_event->firmware_id, \
                ota_event->curr_block, ota_event->total_block, ota_event->curr_chunk, ota_event->chunk_mask, \
                ota_event->time_escaped, ota_event->time_escaped/3600, ota_event->time_escaped%3600/60, ota_event->time_escaped%60);
            for (UINT16 i = 0; i < ota_event->nodes_num; i++)
            {
                BTMW_RPC_TEST_Logi("[mesh] update node [0x%x] status %d\n", ota_event->nodes_status[i].addr, ota_event->nodes_status[i].status);
                switch (ota_event->nodes_status[i].status)
                {
                    case BT_MESH_OTA_NODE_UPDATE_STATUS_SUCCESS:
                        success++;
                        break;
                    case BT_MESH_OTA_NODE_UPDATE_STATUS_IN_PROGRESS:
                        in_progress++;
                        break;
                    case BT_MESH_OTA_NODE_UPDATE_STATUS_CANCELED:
                        cancelled++;
                        break;
                    case BT_MESH_OTA_NODE_UPDATE_STATUS_DFU_READY:
                        dfu_ready++;
                        break;
                    default:
                        break;
                }
            }
            BTMW_RPC_TEST_Logd("[mesh] success(%d)/in_progress(%d)/cancelled(%d)/dfu_ready(%d)/total(%d)\n", \
                success, in_progress, cancelled, dfu_ready, ota_event->nodes_num);

            switch (ota_event->event_id)
            {
                case BT_MESH_OTA_EVENT_DISTRIBUTION_STARTING:
                {
                    //OTA is starting, this is just a transform state
                    break;
                }
                case BT_MESH_OTA_EVENT_DISTRIBUTION_STARTED:
                {
                    //OTA started successfully. Next event shall be ONGOING
                    break;
                }
                case BT_MESH_OTA_EVENT_DISTRIBUTION_ONGOING:
                {
                    //ONGOING state, this event is notified every 10s until ota stop
                    break;
                }
                case BT_MESH_OTA_EVENT_DISTRIBUTION_STOP:
                {
                    //OTA stop due to success or user stop(user stop could be caused by several NG cases)
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case BT_MESH_EVT_FRIENDSHIP_STATUS:
        {
            BTMW_RPC_TEST_Logi("[mesh] lpn address = 0x%x, friendship status = %d\n", \
                                bt_event->evt.mesh_evt.mesh.friendship_status.address,
                                bt_event->evt.mesh_evt.mesh.friendship_status.status);
            break;
        }
        case BT_MESH_EVT_BEARER_GATT_STATUS:
        {
            BTMW_RPC_TEST_Logi("[mesh] gatt bearer handle = 0x%x, status = %d\n", \
                                bt_event->evt.mesh_evt.mesh.bearer_gatt_status.handle,
                                bt_event->evt.mesh_evt.mesh.bearer_gatt_status.status);
            break;
        }
        case BT_MESH_EVT_ERROR_CODE:
        {
            BTMW_RPC_TEST_Logi("[mesh] error code type = %d\n", \
                                bt_event->evt.mesh_evt.mesh.error_code.type);
            break;
        }
        default:
            break;
    }
    return;
}

static VOID _btmw_rpc_test_mesh_bind_ctl_temperature(UINT16 state, UINT8 binding)
{
    if((binding & BTMW_RPC_TEST_MESH_MODEL_BINDING_MASK) & BTMW_RPC_TEST_MESH_MODEL_BINDING_PRESENT_VALUE)
    {
        if(gCTL_temperature_server.present_ctl_temperature < gCTL_server.range_min)
            gCTL_temperature_server.present_ctl_temperature = gCTL_server.range_min;
        else if(gCTL_temperature_server.present_ctl_temperature > gCTL_server.range_max)
            gCTL_temperature_server.present_ctl_temperature = gCTL_server.range_max;

        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_GENERIC_LEVEL))
        {
            gCTL_temperature_server.level_server.present_level = ((gCTL_temperature_server.present_ctl_temperature-gCTL_server.range_min)*65535)/(gCTL_server.range_max-gCTL_server.range_min) - 32768;
        }

        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS))
        {
            gCTL_server.present_ctl_temperature = gCTL_temperature_server.present_ctl_temperature;
        }

    }
    if((binding & BTMW_RPC_TEST_MESH_MODEL_BINDING_MASK) & BTMW_RPC_TEST_MESH_MODEL_BINDING_TARGET_VALUE)
    {
        if(gCTL_temperature_server.target_ctl_temperature < gCTL_server.range_min)
            gCTL_temperature_server.target_ctl_temperature = gCTL_server.range_min;
        else if(gCTL_temperature_server.target_ctl_temperature > gCTL_server.range_max)
            gCTL_temperature_server.target_ctl_temperature = gCTL_server.range_max;

        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_GENERIC_LEVEL))
        {
            gCTL_temperature_server.level_server.target_level = ((gCTL_temperature_server.target_ctl_temperature-gCTL_server.range_min)*65535)/(gCTL_server.range_max-gCTL_server.range_min) - 32768;
        }

        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS))
        {
             gCTL_server.target_ctl_temperature = gCTL_temperature_server.target_ctl_temperature;
        }
    }

}

static VOID _btmw_rpc_test_mesh_bind_ctl_lightness(UINT16 state, UINT8 binding)
{
    if((binding & BTMW_RPC_TEST_MESH_MODEL_BINDING_MASK) & BTMW_RPC_TEST_MESH_MODEL_BINDING_PRESENT_VALUE)
    {
        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_LIGHTNESS_ACTUAL))
        {
            gLightness_server.present_lightness = gCTL_server.present_ctl_lightness;
            //todo
            //_btmw_rpc_test_bind_lightness_actual(BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS | state, BTMW_RPC_TEST_MESH_MODEL_BINDING_PRESENT_VALUE);
        }

        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE))
        {
             gCTL_temperature_server.present_ctl_temperature = gCTL_server.present_ctl_temperature;
        }
    }
    if((binding & BTMW_RPC_TEST_MESH_MODEL_BINDING_MASK) & BTMW_RPC_TEST_MESH_MODEL_BINDING_TARGET_VALUE)
    {
        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_LIGHTNESS_ACTUAL))
        {
            gLightness_server.target_lightness = gCTL_server.target_ctl_lightness;
            //todo
            //_btmw_rpc_test_bind_lightness_actual(BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS | state, BTMW_RPC_TEST_MESH_MODEL_BINDING_TARGET_VALUE);
        }
        if(!((state & BTMW_RPC_TEST_MESH_MODEL_STATE_MASK) & BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE))
        {
             gCTL_temperature_server.target_ctl_temperature = gCTL_server.target_ctl_temperature;
        }
    }
}

static int _btmw_rpc_test_mesh_create_device(VOID)
{
    UINT8 composition_data_header[10] =
    {
        0x8A, 0x01, // cid
        0x1A, 0x00, // pid
        0x01, 0x00, // vid
        0x08, 0x00, // crpl
        BT_MESH_FEATURE_RELAY | BT_MESH_FEATURE_PROXY | BT_MESH_FEATURE_FRIEND, 0x00, // features
    };
    UINT16 element_index = -1;
    UINT16 model_handle = -1;
    BT_MESH_MODEL_DATA_T md;
    BT_MESH_MODEL_ADD_PARAMS_T *pmodel_params = (BT_MESH_MODEL_ADD_PARAMS_T *)malloc(sizeof(BT_MESH_MODEL_ADD_PARAMS_T));

    BTMW_RPC_TEST_Logi("[mesh] btmw_rpc_test_mesh_create_device\n");

    if (NULL == pmodel_params)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc pmodel_params failed\n");
        return 0;
    }

    //set composition_data
    memset(&md,0,sizeof(md));
    md.opcode = BT_MESH_MD_OP_SET_COMPOSITION_DATA_HEADER;
    md.data.composition_data.buf = composition_data_header;
    md.data.composition_data.buf_len = BTMW_RPC_TEST_MESH_COMPOSITION_DATA_LEN;
    a_mtkapi_bt_mesh_set_model_data(&md);
    //add a element
    memset(&md,0,sizeof(md));
    md.opcode = BT_MESH_MD_OP_ADD_ELEMENT;
    md.data.element.element_index = &element_index;
    md.data.element.location = BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_FRONT;
    a_mtkapi_bt_mesh_set_model_data(&md);
    BTMW_RPC_TEST_Logi("[mesh] element_index = %u\n", element_index);
    btmw_rpc_primary_element_idx = element_index;
    //add configuration server
    memset(&md,0,sizeof(md));
    md.opcode = BT_MESH_MD_OP_ADD_CONFIGURATION_SERVER;
    md.data.configuration_server.callback= NULL;
    md.data.configuration_server.model_handle = &model_handle;
    a_mtkapi_bt_mesh_set_model_data(&md);
    BTMW_RPC_TEST_Logi("[mesh] configuration server model_handle = 0x%x\n", model_handle);
    //add health server
    memset(&md,0,sizeof(md));
    md.opcode = BT_MESH_MD_OP_ADD_HEALTH_SERVER;
    md.data.health_server.model_handle = &model_handle;
    md.data.health_server.element_index = element_index;
    md.data.health_server.callback= NULL;
    a_mtkapi_bt_mesh_set_model_data(&md);
    BTMW_RPC_TEST_Logi("[mesh] health server model_handle = 0x%x\n", model_handle);

    if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
    {
        //add generic onoff client
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_MODEL;
        memset(pmodel_params, 0, sizeof(BT_MESH_MODEL_ADD_PARAMS_T));
        md.data.model_data.model_params = pmodel_params;
        md.data.model_data.model_params->element_index = element_index;
        md.data.model_data.model_params->model_id = BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT;
        md.data.model_data.model_params->opcode_handlers = btmw_rpc_onoff_client_handler;
        md.data.model_data.model_params->opcode_count = sizeof(btmw_rpc_onoff_client_handler)/sizeof(BT_MESH_ACCESS_OPCODE_HANDLER_T);
        md.data.model_data.model_handle = &btmw_rpc_handle_onoff_client;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] generic onoff client model_handle = 0x%x\n", btmw_rpc_handle_onoff_client);
        //add configuration client
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_CONFIGURATION_CLIENT;
        md.data.configuration_client.element_index = element_index;
        md.data.configuration_client.callback = _btmw_rpc_test_mesh_config_client_msg_handler;
        md.data.configuration_client.model_handle = &model_handle;
        a_mtkapi_bt_mesh_set_model_data(&md);
        btmw_rpc_config_client_element_idx = element_index;
        BTMW_RPC_TEST_Logi("[mesh] configuration_client model_handle = 0x%x, callback=%p\n", model_handle, md.data.configuration_client.callback);

        //add health client
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_HEALTH_CLIENT;
        md.data.health_client.element_index = element_index;
        md.data.health_client.callback = _btmw_rpc_test_mesh_health_client_msg_handler;
        md.data.health_client.model_handle = &model_handle;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] health client model handle = 0x%x, callback=%p\n", model_handle, md.data.health_client.callback);

        //add generic power onoff client model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_GENERIC_POWER_ONOFF_CLIENT;
        md.data.generic_power_onoff_client.model_handle = &btmw_rpc_handle_generic_power_onoff_client;
        md.data.generic_power_onoff_client.element_index = element_index;
        md.data.generic_power_onoff_client.callback = _btmw_rpc_test_mesh_generic_onoff_client_msg_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] generic power onoff client model_handle = 0x%x, element_index=%d\n",
                            btmw_rpc_handle_generic_power_onoff_client,
                            md.data.generic_power_onoff_client.element_index);

        //add lightness client model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_LIGHTNESS_CLIENT;
        md.data.lightness_client.model_handle = &btmw_rpc_handle_lightness_client;
        md.data.lightness_client.element_index = element_index;
        md.data.lightness_client.callback = _btmw_rpc_test_mesh_lightness_client_msg_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] lightness client model_handle = 0x%x, element_index=%d\n",
                            btmw_rpc_handle_lightness_client, md.data.lightness_client.element_index);

        //add CTL client model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_CTL_CLIENT;
        md.data.ctl_client.model_handle = &btmw_rpc_handle_ctl_client;
        md.data.ctl_client.element_index = element_index;
        md.data.ctl_client.callback = _btmw_rpc_test_mesh_ctl_client_msg_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] ctl client model_handle = 0x%x, element_index=%d\n",
                            btmw_rpc_handle_ctl_client, md.data.ctl_client.element_index);

        //add HSL client model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_HSL_CLIENT;
        md.data.hsl_client.model_handle = &btmw_rpc_handle_hsl_client;
        md.data.hsl_client.element_index = element_index;
        md.data.hsl_client.callback = _btmw_rpc_test_mesh_hsl_client_msg_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] hsl client model_handle = 0x%x, element_index=%d\n",
                            btmw_rpc_handle_hsl_client, md.data.hsl_client.element_index);

        //add sensor client model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_MODEL;
        memset(pmodel_params, 0, sizeof(BT_MESH_MODEL_ADD_PARAMS_T));
        md.data.model_data.model_params = pmodel_params;
        md.data.model_data.model_params->element_index = element_index;
        md.data.model_data.model_params->model_id = BTMW_RPC_TEST_MESH_SIG_MODEL_ID_SENSOR_CLIENT;
        md.data.model_data.model_params->opcode_handlers = btmw_rpc_sensor_client_handler;
        md.data.model_data.model_params->opcode_count = sizeof(btmw_rpc_sensor_client_handler)/sizeof(BT_MESH_ACCESS_OPCODE_HANDLER_T);
        md.data.model_data.model_handle = &btmw_rpc_handle_sensor_client;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] sensor client model_handle = 0x%x\n", btmw_rpc_handle_sensor_client);

        //set addr,just for set mesh data test,need to mark
#if 0
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_SET_ELEMENT_ADDR;
        md.data.element_addr.unicast_addr = 0x1;
        a_mtkapi_bt_mesh_set_model_data(&md);
#endif
    }
    else if (BT_MESH_ROLE_PROVISIONEE == btmw_rpc_mesh_role)
    {
        //add generic onoff server
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_GENERIC_ONOFF_SERVER;
        md.data.generic_onoff_server.model_handle = &model_handle;
        md.data.generic_onoff_server.element_index = element_index;
        md.data.generic_onoff_server.callback = _btmw_rpc_test_mesh_sig_msg_server_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] generic onoff server model_handle = 0x%x, callback=%p\n",
                           model_handle, md.data.generic_onoff_server.callback);

        //add vendor server
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_MODEL;
        memset(pmodel_params, 0, sizeof(BT_MESH_MODEL_ADD_PARAMS_T));
        md.data.model_data.model_params = pmodel_params;
        md.data.model_data.model_params->element_index = element_index;
        md.data.model_data.model_params->model_id = BTMW_RPC_TEST_MESH_VENDOR_MODEL_ID(BTMW_RPC_TEST_MESH_VENDOR_COMPANY_ID, BTMW_RPC_TEST_MESH_VENDOR_MODEL_ID1);
        md.data.model_data.model_params->opcode_handlers = btmw_rpc_vendor_message_handler;
        md.data.model_data.model_params->opcode_count = sizeof(btmw_rpc_vendor_message_handler)/sizeof(BT_MESH_ACCESS_OPCODE_HANDLER_T);
        md.data.model_data.model_handle = &model_handle;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] vendor_server model_handle = 0x%x\n", model_handle);

        memset(&gCTL_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_CTL_SERVER_T));
        memset(&gCTL_temperature_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_CTL_TEMP_SERVER_T));
        memset(&gLightness_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_LIGNTNESS_SERVER_T));
        memset(&gHSL_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_HSL_SERVER_T));
        memset(&gHSL_hue_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_HSL_HUE_SERVER_T));
        memset(&gHSL_saturation_server, 0, sizeof(BTMW_RPC_TEST_MESH_LIGHTING_HSL_SAT_SERVER_T));
        gCTL_server.range_min = 0x320;
        gCTL_server.range_max = 0x4E20;
        gCTL_server.element_index = 0xFFFF;
        gCTL_temperature_server.element_index = 0xFFFF;

        gLightness_server.range_min = 0x1;
        gLightness_server.range_max = 0xFFFF;

        gHSL_server.hue_range_min = 0;
        gHSL_server.hue_range_max = 0xFFFF;

        gHSL_server.saturation_range_min = 0;
        gHSL_server.saturation_range_max = 0xFFFF;

        gHSL_server.element_index = 0xFFFF;
        gHSL_hue_server.element_index = 0xFFFF;
        gHSL_saturation_server.element_index = 0xFFFF;

        //add a element
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_ELEMENT;
        md.data.element.element_index = &element_index;
        md.data.element.location = BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_FRONT;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] element_index = 0x%x\n", element_index);

        //add ctl setup server model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_CTL_SETUP_SERVER;
        md.data.ctl_setup_server.model_handle = &model_handle;
        md.data.ctl_setup_server.element_index = element_index;
        md.data.ctl_setup_server.callback = _btmw_rpc_test_mesh_sig_msg_server_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] ctl setup server model_handle = 0x%x, element_index = %d, callback=%p\n",
                            model_handle, element_index, md.data.ctl_setup_server.callback);

        gCTL_server.element_index = md.data.ctl_setup_server.element_index;
        gCTL_temperature_server.element_index = gCTL_server.element_index + 1;

        _btmw_rpc_test_mesh_bind_ctl_temperature(BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE, BTMW_RPC_TEST_MESH_MODEL_BINDING_BOTH_VALUE);


        //add a element
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_ELEMENT;
        md.data.element.element_index = &element_index;
        md.data.element.location = BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_FRONT;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] element index = 0x%x\n", element_index);

        //add hsl setup server model
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_ADD_HSL_SETUP_SERVER;
        md.data.hsl_setup_server.model_handle = &model_handle;
        md.data.hsl_setup_server.element_index = element_index;
        md.data.hsl_setup_server.callback = _btmw_rpc_test_mesh_sig_msg_server_handler;
        a_mtkapi_bt_mesh_set_model_data(&md);
        BTMW_RPC_TEST_Logi("[mesh] hsl setup server model_handle = 0x%x, element_index = %d, callback=%p\n",
                            model_handle, element_index, md.data.hsl_setup_server.callback);

        gHSL_server.element_index = md.data.hsl_setup_server.element_index;
        gHSL_hue_server.element_index = gHSL_server.element_index+1;
        gHSL_saturation_server.element_index = gHSL_hue_server.element_index + 1;
        //todo bind hsl state
        //_bind_hsl_hue(MESH_MODEL_STATE_LIGHTING_HSL_HUE, MESH_MODEL_BINDING_BOTH_VALUE);
        //_bind_hsl_saturation(MESH_MODEL_STATE_LIGHTING_HSL_SATURATION, MESH_MODEL_BINDING_BOTH_VALUE);
    }
    if (pmodel_params)
        free(pmodel_params);

    return 0;
}

static VOID _btmw_rpc_test_mesh_data_load(VOID)
{
    BT_MESH_RECORD_T btmw_mesh_data;
    INT8 i = 0;
    if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
    {
        memset(&btmw_mesh_data, 0xFF, sizeof(BT_MESH_RECORD_T));
        mesh_data = &btmw_mesh_data;
        // set a netkey
        for (i = 0; i < 1; i++)
        {
            mesh_data->netkey[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->netkey[i].keyidx = 0;
            memcpy(mesh_data->netkey[i].key, btmw_rpc_netkey, sizeof(mesh_data->netkey[i].key));
            mesh_data->netkey[i].ivphase = BT_MESH_FLASH_INVALID_DATA;
            mesh_data->netkey[i].ivIndex = 1;
            mesh_data->netkey[i].phase = BT_MESH_FLASH_INVALID_DATA;
            mesh_data->netkey[i].node_identity = BT_MESH_FLASH_INVALID_DATA;
            memset(mesh_data->netkey[i].tmpkey, 0x0, sizeof(mesh_data->netkey[i].tmpkey));
        }
        // set friend
        for (i = 0; i < BT_MESH_FRIEND_RECORD_NUMBER; i++)
        {
            mesh_data->friend_info[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->friend_info[i].netkeyIdx = i;
            if (i ==  BT_MESH_NET_KEY_RECORD_NUMBER)
            {
                mesh_data->friend_info[i].netkeyIdx = BT_MESH_NET_KEY_RECORD_NUMBER -1;
            }
            memset(mesh_data->friend_info[i].lpn_addr, i+1, sizeof(mesh_data->friend_info[i].lpn_addr));
            memset(mesh_data->friend_info[i].friend_addr, i+1, sizeof(mesh_data->friend_info[i].friend_addr));
            memset(mesh_data->friend_info[i].lpn_counter, i+1, sizeof(mesh_data->friend_info[i].lpn_counter));
            memset(mesh_data->friend_info[i].friend_counter, i+1, sizeof(mesh_data->friend_info[i].friend_counter));
        }
        // set a appkey
        for (i = 0; i < 1; i++)
        {
            mesh_data->appkey[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->appkey[i].appkeyIdx = 0x124;
            memcpy(mesh_data->appkey[i].key, btmw_rpc_appkey, sizeof(mesh_data->appkey[i].key));
            mesh_data->appkey[i].netkeyIdx = 0;
            mesh_data->appkey[i].phase = 0;
            memset(mesh_data->appkey[i].tmpkey, 0x0, sizeof(mesh_data->appkey[i].tmpkey));
        }
        // set bind model
        for (i = 0; i < 2; i++)
        {
            mesh_data->model[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->model[i].appkeyIdx = 0x124;
            mesh_data->model[i].idLength = 2; // sig model:2 vendor model:4
            mesh_data->model[i].model_id = BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT + i*2;
            mesh_data->model[i].unicast_addr = 1;
        }
        // set publication
        for (i = 0; i < BT_MESH_MODEL_PUBLICATION_RECORD_NUMBER; i++)
        {
            mesh_data->publication[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->publication[i].model_id = BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_CLIENT;
            mesh_data->publication[i].unicast_addr = 1;
            mesh_data->publication[i].model_publication.addr = 0xFC00;
            mesh_data->publication[i].model_publication.appkey_index = 0x124;
            mesh_data->publication[i].model_publication.flag = 0;
            mesh_data->publication[i].model_publication.rfu = 0;
            mesh_data->publication[i].model_publication.ttl = 3;
            mesh_data->publication[i].model_publication.period = 3;
            mesh_data->publication[i].model_publication.retransmit_count = 1;
            mesh_data->publication[i].model_publication.retransmit_interval_steps = 0;
        }
        // set subscription
        for (i = 0; i < 2; i++)
        {
            mesh_data->subscription[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->subscription[i].model_id = BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT + i*2;
            mesh_data->subscription[i].unicast_addr = 1;
            mesh_data->subscription[i].subscriptionAddr = 0xFC00;
        }
        // set local_deviceInfo
        for (i = 0; i < BT_MESH_LOCAL_DEVICE_INFO_RECORD_NUMBER; i++)
        {
            mesh_data->local_deviceInfo[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            memcpy(mesh_data->local_deviceInfo[i].uuid, btmw_rpc_deviceUuid, BT_MESH_UUID_SIZE);
            memset(mesh_data->local_deviceInfo[i].deviceKey, 0x0, BT_MESH_DEVKEY_SIZE);
            mesh_data->local_deviceInfo[i].unicast_addr = 0x1;
        }
        // set seq
        for (i = 0; i < 1; i++)
        {
            mesh_data->seq_info[i].isValidData = BT_MESH_FLASH_VALID_DATA;
            mesh_data->seq_info[i].seq_num = 0x1111F0;
        }

        a_mtkapi_bt_mesh_data_set(mesh_data);
    }
    return;

}

static VOID _btmw_rpc_test_mesh_init(BT_MESH_ROLE_T role, UINT32 debug_level, UINT8 fgsave2flash)
{
    BTMW_RPC_TEST_Logi("[mesh] Mesh initialising %s...\n", (BT_MESH_ROLE_PROVISIONER == role) ? "PROVISIONER" : "PROVISIONEE");

    BT_MESH_INIT_PARAMS_T *initparams = malloc(sizeof(BT_MESH_INIT_PARAMS_T));
    if (NULL == initparams)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc initparams failed\n");
        return;
    }
    memset(initparams, 0, sizeof(BT_MESH_INIT_PARAMS_T));

    // init debug parameter
    BT_MESH_DEBUG_INIT_PARAMS_T debug_param = {
        .verbose_level = debug_level,
        .info_level = debug_level,
        .notify_level = debug_level,
        .warning_level = debug_level
    };
    BT_MESH_CUSTOMIZE_PARA_T cust_para = {
        .max_remote_node_cnt = 1*BT_MESH_REMOTE_DEVICE_INFO_RECORD_NUMBER,
        .save2flash = fgsave2flash
    };

    BT_MESH_FRIEND_INIT_PARAMS_T friend_params = {
        .lpn_number = 10,
        .queue_size = 10,
        .subscription_list_size = 5
    };
    initparams->customize_params = &cust_para;

    initparams->debug = &debug_param;
    initparams->friend_params = &friend_params;

    if (BT_MESH_ROLE_PROVISIONER == role)
    {
        initparams->role = BT_MESH_ROLE_PROVISIONER;
        // no need to init provisionee params
        initparams->provisionee = NULL;
    }
    else
    {
        initparams->role = BT_MESH_ROLE_PROVISIONEE;
        // init provisionee parameter
        initparams->provisionee = (BT_MESH_PROV_PROVISIONEE_PARAMS_T *)malloc(sizeof(BT_MESH_PROV_PROVISIONEE_PARAMS_T));
        if (NULL == initparams->provisionee)
        {
            BTMW_RPC_TEST_Loge("[mesh] malloc initparams->provisionee failed\n");
            free(initparams);
            return;
        }
        initparams->provisionee->cap.number_of_elements = 1;
        initparams->provisionee->cap.algorithms = BTMW_RPC_TEST_MESH_PROV_CAPABILITY_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE; // bit 0: P-256, bit 1~15: RFU
        initparams->provisionee->cap.public_key_type = BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_PUBLIC_KEY_TYPE_INBAND;
        initparams->provisionee->cap.static_oob_type = BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED;
        initparams->provisionee->cap.output_oob_size = 0x00;
        initparams->provisionee->cap.output_oob_action = 0x0000;
        initparams->provisionee->cap.input_oob_size = 0x00;
        initparams->provisionee->cap.input_oob_action = 0x0000;
    }

    // init config parameter
    initparams->config = (BT_MESH_CONFIG_INIT_PARAMS_T *)malloc(sizeof(BT_MESH_CONFIG_INIT_PARAMS_T));
    if (NULL == initparams->config)
    {
        BTMW_RPC_TEST_Loge("[mesh] malloc initparams->config failed\n");
        if (initparams->provisionee)
        {
            free(initparams->provisionee);
        }
        free(initparams);
        return;
    }
    memset(initparams->config, 0, sizeof(BT_MESH_CONFIG_INIT_PARAMS_T));
    memcpy(initparams->config->device_uuid, btmw_rpc_deviceUuid, BT_MESH_UUID_SIZE);
    initparams->config->oob_info = BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_NUMBER | BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_STRING;
    initparams->config->uri = NULL;
    initparams->config->default_ttl = 4;
    initparams->feature_mask |= BT_MESH_FEATURE_HEARTBEAT;

    BTMW_RPC_TEST_Logi("[mesh] initparams->provisionee = %p\n", initparams->provisionee);
    BTMW_RPC_TEST_Logi("[mesh] initparams->config = %p\n", initparams->config);
    BTMW_RPC_TEST_Logi("[mesh] initparams->feature_mask = %d\n", initparams->feature_mask);


    a_mtkapi_bt_mesh_enable(initparams);
    if (BT_MESH_ROLE_PROVISIONEE == role)
    {
        free(initparams->provisionee);
    }
    free(initparams->config);
    free(initparams);
}

int btmw_rpc_test_mesh_pre_init(VOID)
{
    MTKRPCAPI_BT_APP_MESH_CB_FUNC_T func;
    char pv_tag[2] = {0};
    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_MESH_CB_FUNC_T));
    func.bt_mesh_bt_event_cb = _btmw_rpc_test_mesh_event_callback;

    INT32 i4_ret = a_mtkapi_bt_mesh_init(&func, pv_tag);
    BTMW_RPC_TEST_Logi("[mesh] btmw_rpc_test_mesh_pre_init, return %d\n", i4_ret);
    fg_pre_init = TRUE;
    return 0;
}

static int btmw_rpc_test_mesh_version_handler(int argc, char *argv[])
{
    char mesh_version[BT_MESH_VERSION_LEN] = {0};
    a_mtkapi_bt_mesh_version(mesh_version);
    BTMW_RPC_TEST_Logi("[mesh] Version %s\n", mesh_version);

    return 0;
}

static int btmw_rpc_test_mesh_log_handler(int argc, char *argv[])
{
    unsigned int log_level = 0;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc == 1)
    {
        log_level = strtol(argv[0], NULL, 16);
        BTMW_RPC_TEST_Logi("btmw rpc mesh log level= %d", log_level);
    }
    else
    {
        BTMW_RPC_TEST_Logd("Usage :\n");
        BTMW_RPC_TEST_Logd("  log <bitmap>\n");
        BTMW_RPC_TEST_Logd("     Bit 0 - MESH_LOG_LV_VBS\n");
        BTMW_RPC_TEST_Logd("     Bit 1 - MESH_LOG_LV_INF\n");
        BTMW_RPC_TEST_Logd("     Bit 2 - MESH_LOG_LV_DBG\n");
        BTMW_RPC_TEST_Logd("     Bit 3 - MESH_LOG_LV_WRN\n");
        BTMW_RPC_TEST_Logd("     Bit 4 - MESH_LOG_LV_ERR\n");
        BTMW_RPC_TEST_Logd("     Bit 5 - MESH_LOG_LV_RAW\n");
        BTMW_RPC_TEST_Logd("\n");
        BTMW_RPC_TEST_Logd("     Bit 8 - MESH_LOG_FLAG_COLOR\n");
        BTMW_RPC_TEST_Logd("     Bit 9 - MESH_LOG_FLAG_TIMESTAMP\n");
        BTMW_RPC_TEST_Logd("  ex. full log and not save to file: log 0x033f\n");
    }

    a_mtkapi_bt_mesh_log_setlevel(log_level);

    return 0;
}

static int btmw_rpc_test_mesh_init_handler(int argc, char *argv[])
{
    UINT32 debug_level = 0xFFF;
    UINT8 fgsave2flash = 0;
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge("USAGE: MESH init <provisioner> [debug_level] [save2flash]\n");
        return 0;
    }

    if (fg_init)
    {
        BTMW_RPC_TEST_Loge("[mesh] already inited\n");
        return 0;
    }

    if (atoi(argv[0]))
    {
        btmw_rpc_mesh_role = BT_MESH_ROLE_PROVISIONER;
    }
    if (argc > 1)
    {
        debug_level = strtol(argv[1], NULL, 16);
        BTMW_RPC_TEST_Loge("[mesh] debug_level: 0x%x\n", debug_level);
    }
    if (argc > 2)
    {
        fgsave2flash = atoi(argv[2]);
        BTMW_RPC_TEST_Loge("[mesh] fgsave2flash: 0x%x\n", fgsave2flash);
    }

    BTMW_RPC_TEST_MESH_DLIST_ALLOC(g_node_list);
    BTMW_RPC_TEST_MESH_DLIST_ALLOC(g_netkey_list);
    BTMW_RPC_TEST_MESH_DLIST_ALLOC(g_appkey_list);

    if (FALSE == fg_pre_init)
    {
        btmw_rpc_test_mesh_pre_init();
    }

    _btmw_rpc_test_mesh_create_device();
    if (btmw_rpc_mesh_role == BT_MESH_ROLE_PROVISIONER)
    {
        //_btmw_rpc_test_mesh_data_load();
    }
    a_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_ON);
    _btmw_rpc_test_mesh_init(btmw_rpc_mesh_role, debug_level, fgsave2flash);
    fg_init = TRUE;

    return 0;
}
static int btmw_rpc_test_mesh_deinit_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey = NULL;
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (!fg_init)
    {
        BTMW_RPC_TEST_Loge("[mesh] not inited\n");
        return 0;
    }
    a_mtkapi_bt_mesh_disable();
    a_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_OFF);
    a_mtkapi_bt_mesh_deinit();

    util_dlist_free(g_node_list);
    g_node_list = NULL;
    //free netkey resource
    if (g_netkey_list)
    {
        netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *)util_dlist_first(g_netkey_list);
        while (netkey != NULL)
        {
            if (netkey->appkey_list)   util_dlist_free(netkey->appkey_list);
            if (netkey->temp_appkey_list)   util_dlist_free(netkey->temp_appkey_list);
            if (netkey->kr_node_list)   util_dlist_free(netkey->kr_node_list);
            if (netkey->kr_node_ack_list)   util_dlist_free(netkey->kr_node_ack_list);
            netkey = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *)util_dlist_next(g_netkey_list, netkey);
        }

        util_dlist_free(g_netkey_list);
        g_netkey_list = NULL;
    }
    util_dlist_free(g_appkey_list);
    g_appkey_list = NULL;

    fg_pre_init = FALSE;
    fg_init = FALSE;

    return 0;
}

static int btmw_rpc_test_mesh_config_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (!strcmp(argv[0], "addr"))
    {
        if (argc < 2)
        {
            BTMW_RPC_TEST_Loge("[mesh] USAGE: config addr <addr>\n");
            return 0;
        }
        UINT16 addr = 0;
        Util_Transform_Str2u16Num(argv[1], &addr);
        BTMW_RPC_TEST_Logi("[mesh] set elemnt addr = 0x%04x\n", addr);
        //add a element
        BT_MESH_MODEL_DATA_T md;
        memset(&md,0,sizeof(md));
        md.opcode = BT_MESH_MD_OP_SET_ELEMENT_ADDR;
        md.data.element_addr.unicast_addr = addr;
        a_mtkapi_bt_mesh_set_model_data(&md);
    }
    else if (!strcmp(argv[0], "ttl"))
    {
        if (argc < 2)
        {
            BTMW_RPC_TEST_Logi("[mesh] USAGE: config ttl <ttl>\n");
            return 0;
        }
        UINT8 ttl = 0;
        Util_Transform_Str2u8Num(argv[1], &ttl);

        a_mtkapi_bt_mesh_set_default_ttl(ttl);
    }
    else if (!strcmp(argv[0], "bind"))
    {
        if (argc < 1)
        {
            BTMW_RPC_TEST_Loge("[mesh] USAGE: config bind\n");
            return 0;
        }
        INT32 ret;
        //todo
        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_onoff_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_onoff_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_onoff_client);

        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_generic_power_onoff_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_generic_power_onoff_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_generic_power_onoff_client);

        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_lightness_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_lightness_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_lightness_client);

        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_ctl_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_ctl_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_ctl_client);

        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_hsl_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_hsl_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_hsl_client);

        ret = a_mtkapi_bt_mesh_model_app_bind(btmw_rpc_handle_sensor_client, btmw_rpc_appidx);
        BTMW_RPC_TEST_Logi("[mesh] ret: %d, btmw_rpc_appidx = %d, btmw_rpc_handle_sensor_client = %d\n",
                           ret, btmw_rpc_appidx, btmw_rpc_handle_sensor_client);
    }

    return 0;
}

static int btmw_rpc_test_mesh_delete_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logd("[mesh] delete all mesh record.\n");
        a_mtkapi_bt_mesh_data_reset(BT_MESH_FLASH_RECORD_ROLE_PROVISIONER | BT_MESH_FLASH_RECORD_ALL);
        a_mtkapi_bt_mesh_data_reset(BT_MESH_FLASH_RECORD_ROLE_PROVISIONEE | BT_MESH_FLASH_RECORD_ALL);
        return 0;
    }

    UINT32 record = strtol(argv[0], NULL, 16);
    BTMW_RPC_TEST_Logd("[mesh] delete mesh record = 0x%08x\n", record);

    a_mtkapi_bt_mesh_data_reset(record);

    return 0;
}


static int btmw_rpc_test_mesh_prov_list_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    UINT32 scan_time = 5000;

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("use default scan_time %ld ms\n", scan_time);
    }
    else
    {
        Util_Transform_Str2Num(argv[0], &scan_time);
    }
    a_mtkapi_bt_mesh_unprov_dev_scan(TRUE, scan_time);

    return 0;
}

static int btmw_rpc_test_mesh_prov_invite_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge("USAGE: prov invite <uuid> [<attention_duration>]\n");
        return 0;
    }

    UINT8 uuid[BT_MESH_UUID_SIZE] = {0};
    BT_MESH_PROV_INVITE_T params;
    // uuid
    if (strlen(argv[0]) != 2 * BT_MESH_UUID_SIZE)
    {
        BTMW_RPC_TEST_Loge("[mesh] <uuid> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
        return 0;
    }
    Util_Transform_Str2u8HexNumArray(argv[0], uuid);

    // invite
    if (argc < 2)
    {
        params.attention_duration = 0;
    }
    else
    {
        Util_Transform_Str2u8Num(argv[1], &params.attention_duration);
    }

    a_mtkapi_bt_mesh_invite_provisioning(uuid, &params);

    return 0;
}

static int btmw_rpc_test_mesh_prov_start_handler(int argc, char *argv[])
{
    UINT8 mode = 0;

    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 5)
    {
        BTMW_RPC_TEST_Loge("USAGE: prov start <netkey> <netidx> <ivindex> <address> <flags> [mode]\n");
        return 0;
    }

    BT_MESH_PROV_PROVISIONER_PARAMS_T params;
    // start
    params.start.algorithm = BTMW_RPC_TEST_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE;
    params.start.public_key = BTMW_RPC_TEST_MESH_PROV_START_PUBLIC_KEY_NO_OOB;
    params.start.authentication_method = BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
    params.start.authentication_action = 0;
    params.start.authentication_size = 0;

    // data
    if (strlen(argv[0]) != 2 * BT_MESH_KEY_SIZE)
    {
        BTMW_RPC_TEST_Loge("[mesh] <netkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
        return -1;
    }
    Util_Transform_Str2u8HexNumArray(argv[0], params.data.netkey);

    Util_Transform_Str2u16Num(argv[1], &params.data.netkey_index);
    Util_Transform_Str2Num(argv[2], &params.data.iv_index);
    Util_Transform_Str2u16Num(argv[3], &params.data.address);
    Util_Transform_Str2u8Num(argv[4], &params.data.flags);
    if (argc == 6)
    {
        Util_Transform_Str2u8Num(argv[5], &mode);
    }

    btmw_rpc_prov_cur_netidx = params.data.netkey_index;
    g_curr_prov_node.addr = params.data.address;

    a_mtkapi_bt_mesh_start_provisioning(&params, mode);

    return 0;
}

static int btmw_rpc_test_mesh_prov_run_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logd("[mesh] %s()\n", __func__);

    if (argc < 6)
    {
        BTMW_RPC_TEST_Logd("USAGE: prov_run <uuid> <netkey> <netidx> <ivindex> <address> <flags> [<attention_duration>] [auth_method 0: no oob, 1: static oob]\n");
        return 0;
    }

    UINT8 uuid[BT_MESH_UUID_SIZE] = {0};
    BT_MESH_PROV_INVITE_T params;

    // uuid
    if (strlen(argv[0]) != 2 * BT_MESH_UUID_SIZE)
    {
        BTMW_RPC_TEST_Logd("[mesh] <uuid> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
        return 0;
    }
    Util_Transform_Str2u8HexNumArray(argv[0], uuid);

    // start
    btmw_rpc_prov_params.start.algorithm = BTMW_RPC_TEST_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE;
    btmw_rpc_prov_params.start.public_key = BTMW_RPC_TEST_MESH_PROV_START_PUBLIC_KEY_NO_OOB;
    btmw_rpc_prov_params.start.authentication_method = BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
    btmw_rpc_prov_params.start.authentication_action = 0;
    btmw_rpc_prov_params.start.authentication_size = 0;

    // data
    if (strlen(argv[1]) != 2 * BT_MESH_KEY_SIZE)
    {
        BTMW_RPC_TEST_Logd("[mesh] <netkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
        return 0;
    }
    Util_Transform_Str2u8HexNumArray(argv[1], btmw_rpc_prov_params.data.netkey);

    Util_Transform_Str2u16Num(argv[2], &btmw_rpc_prov_params.data.netkey_index);
    Util_Transform_Str2Num(argv[3], &btmw_rpc_prov_params.data.iv_index);
    Util_Transform_Str2u16Num(argv[4], &btmw_rpc_prov_params.data.address);
    Util_Transform_Str2u8Num(argv[5], &btmw_rpc_prov_params.data.flags);

    btmw_rpc_prov_cur_netidx = btmw_rpc_prov_params.data.netkey_index;
    g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_PROVISIONING;
    g_curr_prov_node.addr = btmw_rpc_prov_params.data.address;
    memcpy(g_curr_prov_node.uuid, uuid, BT_MESH_UUID_SIZE);

    // invite
    if (argc < 7)
    {
        params.attention_duration = 0;
    }
    else
    {
        Util_Transform_Str2u8Num(argv[6], &params.attention_duration);
    }
    if (argc == 8)
    {
        UINT8 ui1_auth_method = 0xFF;
        Util_Transform_Str2u8Num(argv[7], &ui1_auth_method);
        if (ui1_auth_method > 4)
            BTMW_RPC_TEST_Loge("[mesh] %s() invalid auth method %d, use default method %d\n", __func__, ui1_auth_method, g_auth_method);
        else
            g_auth_method = ui1_auth_method;
    }

    a_mtkapi_bt_mesh_invite_provisioning(uuid, &params);

    return 0;
}
static int btmw_rpc_test_mesh_key_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 4)
    {
        return 0;
    }

    if (!strcmp(argv[0], "add"))
    {
        if (!strcmp(argv[1], "net"))
        {
            if (argc < 4)
            {
                BTMW_RPC_TEST_Loge("USAGE: key add net <netkey> <keyidx>");
                return 0;
            }

            INT32 ret = 0;
            UINT8 key[BT_MESH_KEY_SIZE];
            UINT16 keyidx;
            BT_MESH_NETKEY_T netkey;
            BTMW_RPC_TEST_Logi("[mesh] netkey= %s\n", argv[2]);
            if (BT_MESH_KEY_SIZE != Util_Transform_Str2u8HexNumArray(argv[2], key))
            {
                return 0;
            }
            BTMW_RPC_TEST_Logi("[mesh] key = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]",
                                key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                                key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);

            BTMW_RPC_TEST_Logi("[mesh] keyidx= %s\n", argv[3]);
            Util_Transform_Str2u16Num(argv[3], &keyidx);
            memset(&netkey, 0, sizeof(netkey));
            netkey.opcode = BT_MESH_KEY_ADD;
            netkey.key_index = keyidx;
            netkey.network_key = key;
            ret = a_mtkapi_bt_mesh_set_netkey(&netkey);
            if (ret)
            {
                BTMW_RPC_TEST_Loge("[mesh] ERROR!!!! [%02x]\n", ret);
            }
            else
            {
                BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T * netkey_entry = NULL;
                netkey_entry = (BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *)util_dlist_entry_alloc(sizeof(BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T));
                if (NULL == netkey_entry)
                {
                    BTMW_RPC_TEST_Loge("[mesh] allocate netkey_entry failed\n");
                }
                else
                {
                    memset(netkey_entry, 0, sizeof(BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T));
                    netkey_entry->keyidx = keyidx;
                    memcpy(netkey_entry->netkey, key, BT_MESH_KEY_SIZE);
                    netkey_entry->state = BT_MESH_KEY_REFRESH_STATE_NONE;
                    BTMW_RPC_TEST_MESH_DLIST_ALLOC(netkey_entry->appkey_list);
                    BTMW_RPC_TEST_MESH_DLIST_ALLOC(netkey_entry->temp_appkey_list);
                    BTMW_RPC_TEST_MESH_DLIST_ALLOC(netkey_entry->kr_node_list);
                    BTMW_RPC_TEST_MESH_DLIST_ALLOC(netkey_entry->kr_node_ack_list);
                    util_dlist_insert(g_netkey_list, netkey_entry);
                    BTMW_RPC_TEST_Logi("[mesh] Add netkey 0x%x to the g_netkey_list\n", keyidx);
                }

            }
        }
        else if (!strcmp(argv[1], "app"))
        {
            //ex:key add app 22222222222222222222222222222222 0x123 0x456
            //for appkey index = 0x123, netkey index = 0x456
            if (argc < 5)
            {
                BTMW_RPC_TEST_Loge("USAGE: key add app <appkey> <appidx> <netidx>");
                return -1;
            }

            INT32 ret = 0;
            UINT8 key[BT_MESH_KEY_SIZE];
            UINT16 appidx, netidx;
            BT_MESH_APPKEY_T appkey;
            if (BT_MESH_KEY_SIZE != Util_Transform_Str2u8HexNumArray(argv[2], key))
            {
                return -1;
            }
            Util_Transform_Str2u16Num(argv[3], &appidx);
            Util_Transform_Str2u16Num(argv[4], &netidx);
            memset(&appkey, 0, sizeof(appkey));
            appkey.opcode = BT_MESH_KEY_ADD;
            appkey.appkey_index = appidx;
            appkey.netkey_index = netidx;
            appkey.application_key = key;
            BTMW_RPC_TEST_Logi("[mesh] appkey.application_key= %p\n", appkey.application_key);
            ret = a_mtkapi_bt_mesh_set_appkey(&appkey);
            if (ret)
            {
                BTMW_RPC_TEST_Loge("[mesh] ERROR!!!! [%02x]\n", ret);
            }
            else
            {
                btmw_rpc_appidx = appidx;
                memcpy(btmw_rpc_appkey, key, sizeof(key));
            }
        }
        return 0;
    }
    else if (!strcmp(argv[0], "update"))
    {
        if (!strcmp(argv[1], "net"))
        {
            if (argc < 4) {
                BTMW_RPC_TEST_Loge("USAGE: key update net <netkey> <keyidx>");
                return -1;
            }

            INT32 ret = 0;
            UINT8 key[BT_MESH_KEY_SIZE];
            UINT16 keyidx;
            BT_MESH_NETKEY_T netkey;
            BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey_entry = NULL;

            if (BT_MESH_KEY_SIZE != Util_Transform_Str2u8HexNumArray(argv[2], key))
            {
                return -1;
            }
            Util_Transform_Str2u16Num(argv[3], &keyidx);
            netkey_entry = _btmw_rpc_test_mesh_get_netkey_by_idx(g_netkey_list, keyidx);
            if (NULL == netkey_entry)
            {
                BTMW_RPC_TEST_Loge("[mesh] Netkey index 0x%x is Not found\n", keyidx);
                return -1;
            }

            memset(&netkey, 0, sizeof(netkey));
            netkey.opcode = BT_MESH_KEY_UPDATE;
            netkey.key_index = keyidx;
            netkey.network_key = key;
            BTMW_RPC_TEST_Logi("[mesh] netkey.network_key= %p\n", netkey.network_key);
            memcpy(netkey_entry->temp_netkey, key, BT_MESH_KEY_SIZE);
            ret = a_mtkapi_bt_mesh_set_netkey(&netkey);
            if (ret)
            {
                BTMW_RPC_TEST_Loge("[mesh] ERROR!!!! [%02x]\n", ret);
            }
        }
        else if (!strcmp(argv[1], "app"))
        {
            if (argc < 5)
            {
                BTMW_RPC_TEST_Loge("USAGE: key update app <appkey> <appidx> <netidx>");
                return -1;
            }

            INT32 ret = 0;
            UINT8 key[BT_MESH_KEY_SIZE];
            UINT16 appidx, netidx;
            BT_MESH_APPKEY_T appkey;

            if (BT_MESH_KEY_SIZE != Util_Transform_Str2u8HexNumArray(argv[2], key))
            {
                return -1;
            }
            Util_Transform_Str2u16Num(argv[3], &appidx);
            Util_Transform_Str2u16Num(argv[4], &netidx);
            memset(&appkey, 0, sizeof(appkey));
            appkey.opcode = BT_MESH_KEY_UPDATE;
            appkey.appkey_index = appidx;
            appkey.netkey_index = netidx;
            appkey.application_key = key;
            BTMW_RPC_TEST_Logi("[mesh] appkey.application_key= %p\n", appkey.application_key);
            ret = a_mtkapi_bt_mesh_set_appkey(&appkey);
            if (ret)
            {
                BTMW_RPC_TEST_Loge("[mesh] ERROR!!!! [%02x]\n", ret);
            }
            else
            {
                btmw_rpc_appidx = appidx;
                memcpy(btmw_rpc_appkey, key, sizeof(key));
            }
        }
    }

    return 0;
}

static int btmw_rpc_test_mesh_save2flash_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    a_mtkapi_bt_mesh_data_save();

    return 0;
}

static int btmw_rpc_test_mesh_set_bearer_adv_para_handler(int argc, char *argv[])
{
    BT_MESH_BEARER_ADV_PARAMS_T adv_params;
    BT_MESH_BEARER_SCAN_PARAMS_T scan_params;

    BTMW_RPC_TEST_Logi("[mesh] %s()\n", __func__);

    if (argc < 6)
    {
        BTMW_RPC_TEST_Logi("[mesh] USAGE: set_adv_para <adv_period> <min_interval> <max_interval> <scan_period> <scan_interval> <scan_window>\n");
        return 0;
    }

    Util_Transform_Str2Num(argv[0], &adv_params.adv_period);
    Util_Transform_Str2u16Num(argv[1], &adv_params.min_interval);
    Util_Transform_Str2u16Num(argv[2], &adv_params.max_interval);
    Util_Transform_Str2Num(argv[3], &scan_params.scan_period);
    Util_Transform_Str2u16Num(argv[4], &scan_params.scan_interval);
    Util_Transform_Str2u16Num(argv[5], &scan_params.scan_window);

    BTMW_RPC_TEST_Logi("[mesh] adv_period:%d, min_interval:0x%02x, max_interval:0x%02x, scan_period:%d, scan_interval:0x%02x, scan_window:0x%02x\n",
            adv_params.adv_period, adv_params.min_interval, adv_params.max_interval,
            scan_params.scan_period, scan_params.scan_interval, scan_params.scan_window);

    a_mtkapi_bt_mesh_bearer_adv_set_params(&adv_params, &scan_params);
    return 0;
}

static int btmw_rpc_test_mesh_start_adv_handler(int argc, char *argv[])
{
    BOOL flag_start = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge("[mesh] USAGE: start_adv <0:1|stop:start>\n");
        return 0;
    }
    flag_start = atoi(argv[0]);
    BTMW_RPC_TEST_Logd("[mesh] %s adv\n", (flag_start == 0) ? "stop" : "start");

    a_mtkapi_bt_mesh_switch_adv(flag_start);
    return 0;
}

static int btmw_rpc_test_mesh_msg_config_handler(int argc, char *argv[])
{
    UINT16 msg_opcode = 0;
    UINT16 src_addr = 0;
    UINT16 dst_addr = 0;
    UINT8 ttl = 0;
    UINT16 netkey_idx = 0;


    BTMW_RPC_TEST_Logd("[mesh] %s()\n", __func__);

    if (argc < 5)
    {
        goto invalid_cmd;
    }

    Util_Transform_Str2u16Num(argv[0], &msg_opcode);
    Util_Transform_Str2u16Num(argv[1], &src_addr);
    Util_Transform_Str2u16Num(argv[2], &dst_addr);
    Util_Transform_Str2u8Num(argv[3], &ttl);
    Util_Transform_Str2u16Num(argv[4], &netkey_idx);

    switch (msg_opcode)
    {
        case BT_MESH_ACCESS_MSG_CONFIG_BEACON_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_BEACON_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET:
        {
            if (argc < 6)
            {
                goto invalid_cmd;
            }
            else
            {
                UINT8 page = 0;
                Util_Transform_Str2u8Num(argv[5], &page);
                util_timer_start(&btmw_rpc_test_mesh_timer[BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID]);
                g_curr_prov_node.config_state = BTMW_RPC_TEST_MESH_CONFIG_STATE_GET_COMPOSITION_DATA;
                _btmw_rpc_test_mesh_cc_config_composition_data_get(dst_addr, page);
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_FRIEND_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_FRIEND_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD:
        {
            if (argc < 8)
            {
                goto invalid_cmd;
            }
            else
            {
                UINT16 element_addr = 0;
                UINT16 group_addr = 0;
                UINT32 model_id = 0xFFFFFFFF;
                Util_Transform_Str2u16Num(argv[5], &element_addr);
                Util_Transform_Str2u16Num(argv[6], &group_addr);
                Util_Transform_Str2u32Num(argv[7], &model_id);
                _btmw_rpc_test_mesh_cc_config_model_suscription_add(dst_addr, netkey_idx, element_addr, group_addr, model_id);
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_RELAY_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_RELAY_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_LOW_POWER_NODE_POLL_TIMEOUT_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_DELETE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_GET:
        {
            BT_MESH_CONFIGURATION_MSG_TX_T msg;
            msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_NETKEY_GET;
            msg.data.netkey_get.meta.src_addr = src_addr;
            msg.data.netkey_get.meta.dst_addr = dst_addr;
            msg.data.netkey_get.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
            msg.data.netkey_get.meta.msg_netkey_index = netkey_idx;

            _btmw_rpc_test_add_mesh_resend_msg_entry(msg.opcode,
                                                msg.data.netkey_get.meta.src_addr,
                                                msg.data.netkey_get.meta.dst_addr,
                                                &msg,
                                                sizeof(BT_MESH_CONFIGURATION_MSG_TX_T));

            INT32 ret = a_mtkapi_bt_mesh_model_cc_msg_tx(&msg);

            BTMW_RPC_TEST_Logd("[mesh] _btmw_rpc_test_mesh_cc_config_netkey_get ret: %d\n", ret);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD:
        {
            UINT16 appkey_index;
            UINT8 key[BT_MESH_KEY_SIZE];
            BT_MESH_CONFIGURATION_MSG_TX_T msg;

            if (argc < 7)
            {
                goto invalid_cmd;
            }

            Util_Transform_Str2u16Num(argv[5], &appkey_index);
            Util_Transform_Str2u8HexNumArray(argv[6], key);

            msg.opcode = BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD;
            msg.data.appkey_add.meta.src_addr = src_addr;
            msg.data.appkey_add.meta.dst_addr = dst_addr;
            msg.data.appkey_add.meta.ttl = a_mtkapi_bt_mesh_get_default_ttl();
            msg.data.appkey_add.meta.msg_netkey_index = netkey_idx;
            msg.data.appkey_add.netkey_index = netkey_idx;
            msg.data.appkey_add.appkey_index = appkey_index;
            msg.data.appkey_add.appkey = key;
            _btmw_rpc_test_mesh_cc_config_appkey_add(appkey_index, netkey_idx, key, dst_addr);
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_DELETE:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET:
        {
            _btmw_rpc_test_mesh_cc_config_node_reset(dst_addr, netkey_idx);

            UINT16 whitelist_node_count = 0;

            whitelist_node_count = util_dlist_count(g_node_list);
            BTMW_RPC_TEST_MESH_NODE_ENTRY_T *reset_node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_first(g_node_list);
            while (reset_node != NULL)
            {
                if (reset_node->addr == dst_addr)
                {
                    BTMW_RPC_TEST_Logd("[mesh] node 0x%04x will be removed later\n", reset_node->addr);
                    reset_node->in_blacklist = TRUE;
                    whitelist_node_count--;
                }
                reset_node = (BTMW_RPC_TEST_MESH_NODE_ENTRY_T *)util_dlist_next(g_node_list, reset_node);
            }
            BTMW_RPC_TEST_Logd("[mesh] whitelist_node_count = %d\n", whitelist_node_count);

            if (whitelist_node_count > 0)
            {
                //Remove the node (start Key Refresh Procedure), Only update primary netkey for demo
                BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T *netkey_entry = NULL;
                netkey_entry = _btmw_rpc_test_mesh_get_netkey_by_idx(g_netkey_list, BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
                if (NULL == netkey_entry)
                {
                    BTMW_RPC_TEST_Loge("[mesh] Netkey index 0x%x is Not found\n", BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
                    return 0;
                }
                memcpy(netkey_entry->temp_netkey, g_new_primary_netkey, BT_MESH_KEY_SIZE);

                BT_MESH_NETKEY_T netkey;
                memset(&netkey, 0, sizeof(netkey));
                netkey.opcode = BT_MESH_KEY_UPDATE;
                netkey.key_index = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
                netkey.network_key = netkey_entry->temp_netkey;
                a_mtkapi_bt_mesh_set_netkey(&netkey);
            }
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_GET:
            break;
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET:
            break;

    }
    return 0;

invalid_cmd:
        BTMW_RPC_TEST_Loge("[mesh] USAGE: msg_config <opcode> <src> <dst> <ttl> <netkey_idx> [fied1] [filed2] [filed3]...\n");
        BTMW_RPC_TEST_Loge("[mesh] USAGE: Please refer to MESH Profile SPEC for detailed field info\n");
        return 0;
}

static VOID btmw_rpc_test_mesh_msg_onoff_handler(int argc, char *argv[])
{
    UINT8 action = 0;
    UINT8 onOff = 0;
    UINT8 tid = 0;
    UINT8 transTime = 0;
    UINT8 delay = 0;
    BOOL  reliable = FALSE;
    BT_MESH_TX_PARAMS_T param;
    BT_MESH_MODEL_OPERATION_T model_op;
#ifdef ENABLE_SPECIAL_MSG
    BT_MESH_BEARER_ADV_PARAMS_T adv_params;
#endif

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_onoff <action> [<onoff> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] <action> 0: get, 1: set\n");
        return;
    }

    memset(&param, 0, sizeof(BT_MESH_TX_PARAMS_T));
    memset(&model_op, 0, sizeof(BT_MESH_MODEL_OPERATION_T));

    param.dst.value = BTMW_RPC_TEST_MESH_ADDR_GROUP_NODES_VALUE;
    param.dst.type = BT_MESH_ADDRESS_TYPE_GROUP;
#ifdef ENABLE_SPECIAL_MSG
    param.ttl = 0x84;//a_mtkapi_bt_mesh_get_default_ttl();
#else
    param.ttl = a_mtkapi_bt_mesh_get_default_ttl();
#endif
    param.security.appidx = btmw_rpc_appidx;
    param.security.netidx = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    param.security.device_key = NULL;

#ifdef ENABLE_SPECIAL_MSG
    //just for special message test
    adv_params.adv_period = 1200; //just only modify this param, min_interval and max_interval don't modify
    adv_params.min_interval = 0x10;
    adv_params.max_interval = 0x10;
    adv_params.resend = 1;

    a_mtkapi_bt_mesh_bearer_adv_set_params(&adv_params, NULL);
#endif

    action = atoi(argv[0]);
    switch (action)
    {
        case 0:
        {
            UINT8 payload[2];
            if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
            {
                param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);//btmw_rpc_primary_element_addr;
            }
            else
            {
                param.src = a_mtkapi_bt_mesh_get_element_address(0);
            }
            param.data_len = 2;

            payload[0] = (BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_GET & 0x00FF;

            param.data = payload;

            if (argc > 1)
            {
                Util_Transform_Str2u16Num(argv[1], &(param.dst.value));
                param.dst.type = _btmw_rpc_test_get_addr_type(param.dst.value);
            }

            INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
            BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
            break;
        }
        case 1:
        {
            onOff = atoi(argv[1]);
            tid = atoi(argv[2]);
            transTime = atoi(argv[3]);
            delay = atoi(argv[4]);
            reliable = atoi(argv[5]);

            BTMW_RPC_TEST_Logi("[mesh] onOff:%02x tid:%02x transTime:%02x delay:%02x reliable:%02x, ttl = %02x\n",
                            onOff, tid, transTime, delay, reliable, param.ttl);

            UINT8 payload[6];
            if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
            {
                param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx); //btmw_rpc_primary_element_addr;
            }
            else
            {
                param.src = a_mtkapi_bt_mesh_get_element_address(0);
            }
            param.data_len = 6;
            if (reliable)
            {
                payload[0] = (BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET & 0x00FF;
            }
            else
            {
                payload[0] = (BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET_UNRELIABLE & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET_UNRELIABLE & 0x00FF;
            }
            payload[2] = onOff;
            payload[3] = g_tid++; //tid;    //tid should change for different msg
            payload[4] = transTime;
            payload[5] = delay;
            param.data = payload;

            if (argc > 6)
            {
                Util_Transform_Str2u16Num(argv[6], &(param.dst.value));
                param.dst.type = _btmw_rpc_test_get_addr_type(param.dst.value);
            }

            INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
            BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
            break;
        }
        default:
            BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
            break;
    }

}

static VOID btmw_rpc_test_mesh_msg_ctl_handler(int argc, char *argv[])
{
    UINT8 action = 0;
    BOOL  reliable = FALSE;
    BT_MESH_TX_PARAMS_T param;
    BT_MESH_MODEL_OPERATION_T model_op;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_ctl actual <action> [<lightness> <temp> <delta_uv> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_ctl temp <action> [<temp> <delta_uv> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_ctl temprange <action> [<range_min> <range_max> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_ctl default <action> [<lightness> <temp> <delta_uv> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] <action> 0: get, 1: set\n");
        return;
    }

    memset(&param, 0, sizeof(BT_MESH_TX_PARAMS_T));
    memset(&model_op, 0, sizeof(BT_MESH_MODEL_OPERATION_T));

    param.dst.value = BTMW_RPC_TEST_MESH_ADDR_GROUP_NODES_VALUE;
    param.dst.type = BT_MESH_ADDRESS_TYPE_GROUP;
    param.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    param.security.appidx = btmw_rpc_appidx;
    param.security.netidx = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    param.security.device_key = NULL;

    action = atoi(argv[1]);

    if (!strcmp(argv[0], "actual"))
    {
        switch (action)
        {
            case 0:
            {
                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_GET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[9];
                param.data_len = 9;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)payload);
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[4], (UINT16*)(payload + 4));
                payload[6] = g_tid++;
                Util_Transform_Str2u8Num(argv[6], payload + 7);
                Util_Transform_Str2u8Num(argv[7], payload + 8);

                reliable = atoi(argv[8]);
                BTMW_RPC_TEST_Logi("[mesh] lightness:%04x temp:%04x delta_uv:%04x reliable:%02x\n",
                                *((UINT16*)payload), *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)), reliable);

                param.data = payload;

                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_SET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "temp"))
    {
        switch (action)
        {
            case 0:
            {
                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_GET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[7];
                param.data_len = 7;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)payload);
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 2));
                payload[4] = g_tid++;
                Util_Transform_Str2u8Num(argv[5], payload + 5);
                Util_Transform_Str2u8Num(argv[6], payload + 6);

                reliable = atoi(argv[7]);
                BTMW_RPC_TEST_Logi("[mesh] temp:%04x delta_uv:%04x reliable:%02x\n",
                                *((UINT16*)payload), *((UINT16*)(payload + 2)), reliable);

                param.data = payload;

                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_SET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "temprange"))
    {
        switch (action)
        {
            case 0:
            {
                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_RANGE_GET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[4];
                param.data_len = 4;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)payload);
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 2));

                reliable = atoi(argv[4]);
                BTMW_RPC_TEST_Logi("[mesh] range min:%04x range max:%04x reliable:%02x\n",
                                *((UINT16*)payload), *((UINT16*)(payload + 2)), reliable);

                param.data = payload;

                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_RANGE_SET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "default"))
    {
        switch (action)
        {
            case 0:
            {
                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_DEFAULT_GET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[6];
                param.data_len = 6;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)payload);
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[4], (UINT16*)(payload + 4));

                reliable = atoi(argv[5]);
                BTMW_RPC_TEST_Logi("[mesh] lightness:%04x temp:%04x delta_uv:%04x reliable:%02x\n",
                                *((UINT16*)payload), *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)), reliable);

                param.data = payload;

                model_op.op_type = BT_MESH_MODEL_LIGHT_CTL_DEFAULT_SET;
                model_op.model_handle = btmw_rpc_handle_ctl_client;
                model_op.reliable = reliable;
                INT32 ret = a_mtkapi_bt_mesh_send_packet_ex(&param, &model_op);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }

}

static VOID btmw_rpc_test_mesh_msg_hsl_handler(int argc, char *argv[])
{
    UINT8 action = 0;
    BOOL  reliable = FALSE;
    BT_MESH_TX_PARAMS_T param;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl actual <action> [<lightness> <hue> <sat> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl target <action>\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl hue <action> [<hue> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl sat <action> [<saturation> <tid> <transTime> <delay> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl range <action> [<hue_min> <hue_max> <saturation_min> <saturation_max> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_hsl default <action> [<lightness> <hue> <saturation> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] <action> 0: get, 1: set\n");
        return;
    }

    memset(&param, 0, sizeof(BT_MESH_TX_PARAMS_T));

    param.dst.value = BTMW_RPC_TEST_MESH_ADDR_GROUP_NODES_VALUE;
    param.dst.type = BT_MESH_ADDRESS_TYPE_GROUP;
    param.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    param.security.appidx = btmw_rpc_appidx;
    param.security.netidx = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    param.security.device_key = NULL;

    action = atoi(argv[1]);

    if (!strcmp(argv[0], "actual"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);//btmw_rpc_primary_element_addr;
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[11];
                param.data_len = 11;
                reliable = atoi(argv[8]);
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET_UNACKNOWLEDGED & 0x00FF;
                }
                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 4));
                Util_Transform_Str2u16Num(argv[4], (UINT16*)(payload + 6));
                payload[8] = g_tid++;
                Util_Transform_Str2u8Num(argv[6], payload + 9);
                Util_Transform_Str2u8Num(argv[7], payload + 10);


                BTMW_RPC_TEST_Logi("[mesh] lightness:%04x hue:%04x saturation:%04x reliable:%02x\n",
                                *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)), *((UINT16*)(payload + 6)), reliable);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "target"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "hue"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[7];
                param.data_len = 7;
                reliable = atoi(argv[6]);

                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET_UNACKNOWLEDGED & 0x00FF;
                }

                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                payload[4] = g_tid++;
                Util_Transform_Str2u8Num(argv[4], payload + 5);
                Util_Transform_Str2u8Num(argv[5], payload + 6);

                BTMW_RPC_TEST_Logi("[mesh] hsl hue:%04x reliable:%02x\n",
                                   *((UINT16*)(payload + 2)), reliable);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "sat"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[7];
                param.data_len = 7;
                reliable = atoi(argv[6]);

                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED & 0x00FF;
                }

                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                payload[4] = g_tid++;
                Util_Transform_Str2u8Num(argv[4], payload + 5);
                Util_Transform_Str2u8Num(argv[5], payload + 6);

                BTMW_RPC_TEST_Logi("[mesh] hsl saturation:%04x reliable:%02x\n",
                                   *((UINT16*)(payload + 2)), reliable);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "range"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[10];
                param.data_len = 10;
                reliable = atoi(argv[6]);

                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED & 0x00FF;
                }
                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 4));
                Util_Transform_Str2u16Num(argv[4], (UINT16*)(payload + 6));
                Util_Transform_Str2u16Num(argv[5], (UINT16*)(payload + 8));

                BTMW_RPC_TEST_Logi("[mesh] hue range min:%04x hue range max:%04x sat range min:%04x sat range max:%04x reliable:%02x\n",
                                *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)),
                                *((UINT16*)(payload + 6)), *((UINT16*)(payload + 8)), reliable);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "default"))
    {
        switch (action)
        {
            case 0:
            {
                UINT8 payload[2];
                param.data_len = 2;
                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_GET & 0x00FF;

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret);
                break;
            }
            case 1:
            {
                UINT8 payload[8];
                param.data_len = 8;
                reliable = atoi(argv[5]);

                if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);
                }
                else
                {
                    param.src = a_mtkapi_bt_mesh_get_element_address(0);
                }

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACKNOWLEDGED & 0x00FF;
                }
                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 4));
                Util_Transform_Str2u16Num(argv[4], (UINT16*)(payload + 6));

                BTMW_RPC_TEST_Logi("[mesh] default lightness:%04x default hue:%04x default sat:%04x reliable:%02x\n",
                                *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)),
                                *((UINT16*)(payload + 6)), reliable);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d, tid: %d\n", ret, g_tid);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }

}

static VOID btmw_rpc_test_mesh_msg_ex_handler(int argc, char *argv[])
{
    BT_MESH_TX_PARAMS_T param;
    UINT8 *payload = NULL;
    UINT8 uuid[BT_MESH_UUID_SIZE] = {0};
    UINT8 devkey[BT_MESH_KEY_SIZE] = {0};

    if (argc < 8)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_ex <dst type> <dst value> <v_uuid> <src> <ttl> <netkey idx> <appkey idx> <devkey> [data0] [data1] [data2]...\n");

        return;
    }

    memset(&param, 0, sizeof(BT_MESH_TX_PARAMS_T));

    param.dst.type = atoi(argv[0]);
    Util_Transform_Str2u16Num(argv[1], &param.dst.value);
    // virtual uuid
    if (strcmp("null", argv[2]) && strcmp("NULL", argv[2]))
    {
        if (strlen(argv[2]) != 2 * BT_MESH_UUID_SIZE)
        {
            BTMW_RPC_TEST_Loge("[mesh] <v_uuid> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
            return;
        }
        Util_Transform_Str2u8HexNumArray(argv[2], uuid);
        param.dst.virtual_uuid = uuid;
    }
    Util_Transform_Str2u16Num(argv[3], &param.src);
    Util_Transform_Str2u8Num(argv[4], &param.ttl);
    Util_Transform_Str2u16Num(argv[5], &param.security.netidx);
    Util_Transform_Str2u16Num(argv[6], &param.security.appidx);
    // device key
    if (strcmp("null", argv[7]) && strcmp("NULL", argv[7]))
    {
        if (strlen(argv[7]) != 2 * BT_MESH_KEY_SIZE)
        {
            BTMW_RPC_TEST_Loge("[mesh] <device key> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
            return;
        }
        Util_Transform_Str2u8HexNumArray(argv[7], devkey);
        param.security.device_key = devkey;
    }

    if (argc > 8)
    {
        param.data_len = argc - 8;
        payload = malloc(param.data_len);
        if (NULL == payload)
        {
            BTMW_RPC_TEST_Loge("[mesh] allocate payload mem fail\n");
            return;
        }
        for (int i = 0; i < param.data_len; i++)
        {
            Util_Transform_Str2u8Num(argv[i+8], &payload[i]);
        }
        param.data = payload;

    }
    BTMW_RPC_TEST_Logi("[mesh]send msg: dst.type --> %d; dst.value --> 0x%x; dst.v_uuid --> %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n", param.dst.type, param.dst.value, \
        uuid[0],uuid[1],uuid[2],uuid[3],uuid[4],uuid[5],uuid[6],uuid[7],uuid[8],uuid[9],uuid[10],uuid[11],uuid[12],uuid[13],uuid[14],uuid[15]);
    BTMW_RPC_TEST_Logi("[mesh]          src --> 0x%x; ttl --> %d, data_len --> %d\n", param.src, param.ttl, param.data_len);
    BTMW_RPC_TEST_Logi("[mesh]          netkey_idx --> 0x%x; appkey_idx --> 0x%x; device_key --> %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n", \
        param.security.netidx, param.security.appidx, \
        devkey[0],devkey[1],devkey[2],devkey[3],devkey[4],devkey[5],devkey[6],devkey[7], \
        devkey[8],devkey[9],devkey[10],devkey[11],devkey[12],devkey[13],devkey[14],devkey[15]);
    if (param.data_len > 0)
    {
        for (int i = 0; i < param.data_len; i++)
        {
            BTMW_RPC_TEST_Logi("[mesh]          data[%d] --> %x\n", i, param.data[i]);
        }
    }

    UINT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
    BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    free(payload);
}
static int btmw_rpc_test_mesh_dump_handler(int argc, char *argv[])
{
    BT_MESH_DUMP_TYPE_T type = BT_MESH_DUMP_TYPE_ALL;

    if (argc >= 1)
    {
        if (!strcmp(argv[0], "uuid"))
        {
            type = BT_MESH_DUMP_TYPE_UUID;
        }
        else if (!strcmp(argv[0], "net"))
        {
            type = BT_MESH_DUMP_TYPE_NETWORK;
        }
        else if (!strcmp(argv[0], "trans"))
        {
            type = BT_MESH_DUMP_TYPE_TRANSPORT;
        }
        else if (!strcmp(argv[0], "config"))
        {
            type = BT_MESH_DUMP_TYPE_CONFIG;
        }
        else if (!strcmp(argv[0], "model"))
        {
            type = BT_MESH_DUMP_TYPE_MODEL;
        }
        else if (!strcmp(argv[0], "lpn"))
        {
            type = BT_MESH_DUMP_TYPE_LPN;
        }
        else if (!strcmp(argv[0], "proxy"))
        {
            type = BT_MESH_DUMP_TYPE_PROXY;
        }
        else if (!strcmp(argv[0], "adv"))
        {
            type = BT_MESH_DUMP_TYPE_ADV;
        }
    }

    a_mtkapi_bt_mesh_dump(type);
    return 0;
}

static int btmw_rpc_test_mesh_set_hbperiod_handler(int argc, char *argv[])
{
    int num = 0;
    UINT32 hb_timeout = 0; //unit:s

    if (argc < 2)
    {
        BTMW_RPC_TEST_Loge("[mesh] USAGE: set_hbperiod <num 0x01~0x11> <hb_timeout>\n");
        return 0;
    }

    num = atoi(argv[0]);
    hb_timeout = atoi(argv[1]);
    if ((num > 0x11) || (num < 0x01))
    {
        BTMW_RPC_TEST_Loge("[mesh] USAGE: set_hbperiod <num 0x01~0x11> <hb_timeout>\n");
        return 0;
    }

    BTMW_RPC_TEST_Logd("[mesh] set heartbeat period: %d, hb_timeout = %d\n", num, hb_timeout);
    a_mtkapi_bt_mesh_set_heartbeat_period(num, hb_timeout);
    return 0;
}

static int btmw_rpc_test_mesh_gatt_connect_handler(int argc, char *argv[])
{
#ifdef MTK_GATT_BEARER_SUPPORT
    BT_MESH_BLE_ADDR_T target;
    BT_MESH_GATT_SERVICE_T service_type;

    if (argc < 3)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] gatt_connect <addr> <addr_type> <service_type>\n");
        return 0;
    }
    strncpy(target.addr, argv[0], MAX_BDADDR_LEN);
    target.addr[MAX_BDADDR_LEN - 1] = '\0';
    target.addr_type = atoi(argv[1]);
    service_type = atoi(argv[2]);
    BTMW_RPC_TEST_Logd("[mesh] %s addr %02x:%02x:%02x:%02x:%02x:%02x addr_type %02x service_type %02x\n", __func__,
                    target.addr[0], target.addr[1], target.addr[2],
                    target.addr[3], target.addr[4], target.addr[5],
                    target.addr_type, service_type);
    a_mtkapi_bt_mesh_gatt_connect(&target, service_type);
    BTMW_RPC_TEST_Logd("[mesh] %s exit\n", __func__);
#else
    BTMW_RPC_TEST_Logi("[mesh] not support\n");
#endif
     return 0;
}

static int btmw_rpc_test_mesh_gatt_disconnect_handler(int argc, char *argv[])
{
#ifdef MTK_GATT_BEARER_SUPPORT
    BTMW_RPC_TEST_Logd("[mesh] %s gatt disconnect enter\n", __func__);
    a_mtkapi_bt_mesh_gatt_disconnect();
    BTMW_RPC_TEST_Logd("[mesh] %s gatt disconnect exist\n", __func__);
#else
    BTMW_RPC_TEST_Logi("[mesh] not support\n");
#endif
     return 0;
}

static int btmw_rpc_test_mesh_ota_operation_handler(int argc, char *argv[])
{
    BT_MESH_OTA_OPERATION_PARAMS_T params;
    BTMW_RPC_TEST_Logd("[mesh] %s()\n", __func__);

    if (argv < 1)
    {
        goto usage_tip;
    }

    memset(&params, 0, sizeof(BT_MESH_OTA_OPERATION_PARAMS_T));

    if (0 == strcmp("reg_cbk", argv[0]))
    {
        params.opcode = BT_MESH_OTA_INITIATOR_OP_REG_MSG_HANDLER;
        params.params.msg_handler.appkey_index = btmw_rpc_appidx;
        params.params.msg_handler.ota_msg_handler = _btmw_rpc_test_mesh_ota_client_msg_handler;
        a_mtkapi_bt_mesh_ota_initiator_operation(&params);
    }
    else if (0 == strcmp("fw_info_get", argv[0]))
    {
        if (argc < 2)
        {
            goto usage_tip;
        }
        Util_Transform_Str2u16Num(argv[1], &params.params.fw_info_get.node_addr);
        params.opcode = BT_MESH_OTA_INITIATOR_OP_FW_INFO_GET;
        a_mtkapi_bt_mesh_ota_initiator_operation(&params);
    }
    else if (0 == strcmp("start", argv[0]))
    {
        UINT32 image_size = 0;
        if (argc < 9)
        {
            goto usage_tip;
        }

        params.opcode = BT_MESH_OTA_INITIATOR_OP_START_DISTRIBUTION;
        //snprintf(params.params.start_params.obj_file, sizeof(params.params.start_params.ne_file), "%s", argv[1]);
        Util_Transform_Str2u32Num(argv[2], &image_size);
        if (image_size < 0x3000)
        {
            BTMW_RPC_TEST_Logd("error: insufficient image size\n");
            goto usage_tip;
        }
        Util_Transform_Str2u32Num(argv[3], &params.params.start_params.new_fw_id);
        Util_Transform_Str2u16Num(argv[4], &params.params.start_params.appkey_index);
        Util_Transform_Str2u16Num(argv[5], &params.params.start_params.distributor_addr);
        Util_Transform_Str2u16Num(argv[6], &params.params.start_params.group_addr);
        Util_Transform_Str2u16Num(argv[7], &params.params.start_params.manual_apply);
        Util_Transform_Str2u16Num(argv[8], &params.params.start_params.updaters_num);
        if (params.params.start_params.updaters_num > (argc - 9))
        {
            BTMW_RPC_TEST_Logd("error: insufficient nodes address\n");
            goto usage_tip;
        }

        UINT16 *nodes = (UINT16 *)malloc(2 * params.params.start_params.updaters_num);
        if (nodes == NULL)
        {
            BTMW_RPC_TEST_Loge("nodes is null\n");
            return 0;
        }
        //parse image file and create obj file
        {
            UINT32 area1_start_addr = 0, area1_length = 0, area1_version = 0;
            UINT32 area2_start_addr = 0, area2_length = 0, area2_version = 0;
            UINT32 area2_checksum = 0;
            UINT8* new_fw_buf = NULL;
            UINT8* obj_buf = NULL;

            if (NULL == argv[1])
            {
                BTMW_RPC_TEST_Loge("[mesh]path is null\n");
                free(nodes);
                return 0;
            }

            BTMW_RPC_TEST_Logd("[mesh] %s, new_firmware:%s\n", __func__, argv[1]);

            if (0 != access(argv[1], F_OK))
            {
                BTMW_RPC_TEST_Loge("[mesh]file:%s not exist!\n", argv[1]);
                free(nodes);
                return 0;
            }
            else
            {
                UINT32 num = 0;
                FILE *fp = NULL;

                fp = fopen(argv[1], "r");
                if (NULL == fp)
                {
                    BTMW_RPC_TEST_Loge("[mesh]Failed to Read %s(%s)", argv[1], strerror(errno));
                    free(nodes);
                    return 0;
                }
                else
                {
                    new_fw_buf = malloc(image_size + 4);  //the additional 4 bytes is for storing the checksum of ota object
                    if (NULL == new_fw_buf)
                    {
                        BTMW_RPC_TEST_Loge("[mesh]allocate new fw buf failed\n");
                        fclose(fp);
                        free(nodes);
                        return 0;
                    }
                    memset(new_fw_buf, 0, image_size);
                    num = fread(new_fw_buf, 1, image_size, fp);
                    if ((0 == num) || (num < image_size))
                    {
                        BTMW_RPC_TEST_Loge("[mesh]num = 0x%x, may fail!!!\n", num);
                        fclose(fp);
                        free(new_fw_buf);
                        free(nodes);
                        return 0;
                    }
                    else
                    {
                        BTMW_RPC_TEST_Logd("[mesh]read %s return num = 0x%x, expect: 0x%x\n", argv[1], num, image_size);
                        if (num != image_size)
                        {
                            fclose(fp);
                            free(new_fw_buf);
                            free(nodes);
                            return 0;
                        }
                    }
                    fclose(fp);
                }
            }

            /****************Start to parse the fw image****************/

            //code area1 data address in the flash stores in the offset of 0x25e7 of the fw image file.
            area1_start_addr = *(UINT32 *)(new_fw_buf + 0x25e7);
            if ((area1_start_addr < OTA_FLASH_OFFSET) || ((area1_start_addr - OTA_FLASH_OFFSET + 4) >= image_size))
            {
                BTMW_RPC_TEST_Loge("[mesh]Invalid area1 start address 0x%x\n", area1_start_addr);
                free(new_fw_buf);
                free(nodes);
                return 0;
            }
            //code area1 data address in the image file is (area1_start_addr - OTA_FLASH_OFFSET)
            area1_length = *(UINT32 *)(new_fw_buf + (area1_start_addr - OTA_FLASH_OFFSET));
            area1_version = *(UINT32 *)(new_fw_buf + (area1_start_addr - OTA_FLASH_OFFSET) + 4);
            //code area2 data follows the area1 data
            area2_start_addr = area1_start_addr - OTA_FLASH_OFFSET + area1_length;
            if ((area2_start_addr + 4) >= image_size)
            {
                BTMW_RPC_TEST_Loge("[mesh]Invalid area2 start address 0x%x\n", area2_start_addr);
                free(new_fw_buf);
                free(nodes);
                return 0;
            }
            area2_length = *(UINT32 *)(new_fw_buf + area2_start_addr);
            area2_version = *(UINT32 *)(new_fw_buf + area2_start_addr + 4);
            if ((area2_start_addr + area2_length) >= image_size)
            {
                BTMW_RPC_TEST_Loge("[mesh]Invalid area2 length 0x%x\n", area2_length);
                free(new_fw_buf);
                free(nodes);
                return 0;
            }
            params.params.start_params.obj_size = area2_length + 4; //area2 is the OTA object, the length shall add 4 bytes of checksum
            obj_buf = new_fw_buf + area2_start_addr;

            for (UINT32 i = 0; i < area2_length; i += 4)
            {
                area2_checksum += *(UINT32 *)(obj_buf + i);
            }

            //make the obj checksum and version as the obj id
            memcpy(params.params.start_params.obj_id, &area2_checksum, 4);
            memcpy(&params.params.start_params.obj_id[4], &area2_version, 4);

            *(UINT32 *)(obj_buf + area2_length) = area2_checksum;    //add the checksum to the end of the ota_obj_data buffer

            BTMW_RPC_TEST_Logd("[mesh] area1 start_addr: 0x%x, length: 0x%x, version: 0x%x\n", \
                (area1_start_addr - OTA_FLASH_OFFSET), area1_length, area1_version);
            BTMW_RPC_TEST_Logd("[mesh] area2 start_addr: 0x%x, length: 0x%x, version: 0x%x, checksum = 0x%x\n", \
                area2_start_addr, params.params.start_params.obj_size, area2_version, area2_checksum);

            //create the obj file
            FILE *obj_file = NULL;
            obj_file = fopen("/data/ota_obj_file.bin", "wb+");
            if (obj_file == NULL)
            {
                BTMW_RPC_TEST_Loge("[mesh] Create /data/ota_obj_file.bin fail\n");
                free(new_fw_buf);
                free(nodes);
                return 0;
            }
            UINT32 wn = fwrite(obj_buf, 1, params.params.start_params.obj_size, obj_file);
            if (wn != params.params.start_params.obj_size)
            {
                BTMW_RPC_TEST_Loge("[mesh] write size = %d\n", wn);
                fclose(obj_file);
                free(new_fw_buf);
                free(nodes);
                return 0;
            }
            fclose(obj_file);
            free(new_fw_buf);
            snprintf(params.params.start_params.obj_file, sizeof(params.params.start_params.obj_file), "%s", "/data/ota_obj_file.bin");
        }

        for (UINT16 i = 0; i < params.params.start_params.updaters_num; i++)
        {
            Util_Transform_Str2u16Num(argv[9 + i], &nodes[i]);
        }
        params.params.start_params.updaters = nodes;

        a_mtkapi_bt_mesh_ota_initiator_operation(&params);

        free(nodes);
    }
    else if (0 == strcmp("stop", argv[0]))
    {
        if (argc < 3)
        {
            goto usage_tip;
        }

        params.opcode = BT_MESH_OTA_INITIATOR_OP_STOP_DISTRIBUTION;
        Util_Transform_Str2u32Num(argv[1], &params.params.stop_params.new_fw_id);
        Util_Transform_Str2u16Num(argv[2], &params.params.stop_params.distributor_addr);

        a_mtkapi_bt_mesh_ota_initiator_operation(&params);
    }
    else if (0 == strcmp("apply", argv[0]))
    {
        params.opcode = BT_MESH_OTA_INITIATOR_OP_APPLY_DISTRIBUTION;

        a_mtkapi_bt_mesh_ota_initiator_operation(&params);
    }
    else
    {
        goto usage_tip;
    }

    return 0;

usage_tip:
    BTMW_RPC_TEST_Logd("USAGE: ota_op [start|stop|apply] ...\
                                 start <new_fw_file> <new_fw_size> <new_fw_id> <appkey_idx> <distributor_addr> <group_addr> <manual_apply> <node_num> <node#1> <node#2> ...\
                                 stop <new_fw_id> <distributor_addr> \
                                 apply\n");
    return 0;
}

static VOID btmw_rpc_test_mesh_msg_sensor_handler(int argc, char *argv[])
{
    UINT8 action = 0;
    BOOL  reliable = FALSE;
    BT_MESH_TX_PARAMS_T param;

    if (argc < 1)
    {
        BTMW_RPC_TEST_Loge("[mesh] invalid parameter\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor descr_get [<property_id>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor cadence <action> <prop_id> [<period_divisor> <trigger_type> <delta_down> <delta_up> <min_interval> <cadence_low> <cadence_high> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor settings_get <property_id>\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor setting <action> <prop_id> <setting_prop_id> [<setting_raw> <reliable>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor get [<property_id>]\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor column_get <property_id> <raw_value_x>\n");
        BTMW_RPC_TEST_Loge("[mesh] msg_sensor series_get <property_id> [<raw_value_x1> <raw_value_x2>]\n");
        BTMW_RPC_TEST_Loge("[mesh] <action> 0: get, 1: set\n");
        return;
    }

    memset(&param, 0, sizeof(BT_MESH_TX_PARAMS_T));

    param.dst.value = BTMW_RPC_TEST_MESH_ADDR_GROUP_NODES_VALUE;
    param.dst.type = BT_MESH_ADDRESS_TYPE_GROUP;
    param.ttl = a_mtkapi_bt_mesh_get_default_ttl();
    param.security.appidx = btmw_rpc_appidx;
    param.security.netidx = BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    param.security.device_key = NULL;

    if (BT_MESH_ROLE_PROVISIONER == btmw_rpc_mesh_role)
    {
        param.src = a_mtkapi_bt_mesh_get_element_address(btmw_rpc_primary_element_idx);//btmw_rpc_primary_element_addr;
    }
    else
    {
        param.src = a_mtkapi_bt_mesh_get_element_address(0);
    }

    if (!strcmp(argv[0], "descr_get"))
    {
        if (argc == 2)
        {
            UINT8 payload[4];
            param.data_len = 4;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_GET & 0x00FF;

            Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
            param.data = payload;
        }
        else if (argc == 1)
        {
            UINT8 payload[2];
            param.data_len = 2;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_GET & 0x00FF;
            param.data = payload;
        }
        INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
        BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    }
    else if (!strcmp(argv[0], "settings_get"))
    {
        UINT8 payload[4];
        param.data_len = 4;
        payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_GET & 0xFF00) >> 8;
        payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_GET & 0x00FF;

        Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
        param.data = payload;

        INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
        BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    }
    else if (!strcmp(argv[0], "get"))
    {
        if (argc == 2)
        {
            UINT8 payload[4];
            param.data_len = 4;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_GET & 0x00FF;

            Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
            param.data = payload;
        }
        else if (argc == 1)
        {
            UINT8 payload[2];
            param.data_len = 2;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_GET & 0x00FF;
            param.data = payload;
        }
        INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
        BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    }
    else if (!strcmp(argv[0], "column_get"))
    {
        UINT8 payload[6];
        param.data_len = 6;
        payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_GET & 0xFF00) >> 8;
        payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_GET & 0x00FF;

        Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
        Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 4));
        param.data = payload;

        INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
        BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    }
    else if (!strcmp(argv[0], "series_get"))
    {
        if (argc == 4)
        {
            UINT8 payload[8];
            param.data_len = 8;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_GET & 0x00FF;

            Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
            Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 4));
            Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 6));
            param.data = payload;
        }
        else if (argc == 2)
        {
            UINT8 payload[4];
            param.data_len = 4;
            payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_GET & 0xFF00) >> 8;
            payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_GET & 0x00FF;
            Util_Transform_Str2u16Num(argv[1], (UINT16*)(payload + 2));
            param.data = payload;
        }
        INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
        BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
    }
    else if (!strcmp(argv[0], "cadence"))
    {
        action = atoi(argv[1]);

        switch (action)
        {
            case 0: //get
            {
                UINT8 payload[4];
                param.data_len = 4;

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_GET & 0x00FF;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
                break;
            }
            case 1: //set
            {
                UINT8 payload[10];
                param.data_len = 10;
                reliable = atoi(argv[10]);

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET_UNACKNOWLEDGED & 0x00FF;
                }

                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                payload[4] = ((atoi(argv[3]) & 0x7F) | ((atoi(argv[4]) & 0x01) << 7));
                BTMW_RPC_TEST_Logd("[mesh] payload[4]: %d\n", payload[4]);
                Util_Transform_Str2u8Num(argv[5], payload + 5);
                Util_Transform_Str2u8Num(argv[6], payload + 6);
                Util_Transform_Str2u8Num(argv[7], payload + 7);
                Util_Transform_Str2u8Num(argv[8], payload + 8);
                Util_Transform_Str2u8Num(argv[9], payload + 9);

                BTMW_RPC_TEST_Logi("[mesh] prop_id:%04x delta_down:%02x delta_up:%02x min_interval:%02x cadence_low:%02x cadence_high:%02x\n",
                                *((UINT16*)(payload + 2)), payload[5], payload[6], payload[7], payload[8], payload[9]);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
    else if (!strcmp(argv[0], "setting"))
    {
        action = atoi(argv[1]);
        switch (action)
        {
            case 0: //get
            {
                UINT8 payload[6];
                param.data_len = 6;

                payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_GET & 0xFF00) >> 8;
                payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_GET & 0x00FF;
                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2));
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 4));

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
                break;
            }
            case 1: //set
            {
                UINT8 payload[7];
                param.data_len = 7;
                reliable = atoi(argv[5]);

                if (reliable)
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET & 0x00FF;
                }
                else
                {
                    payload[0] = (BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET_UNACKNOWLEDGED & 0xFF00) >> 8;
                    payload[1] = BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET_UNACKNOWLEDGED & 0x00FF;
                }

                Util_Transform_Str2u16Num(argv[2], (UINT16*)(payload + 2)); //prop_id
                Util_Transform_Str2u16Num(argv[3], (UINT16*)(payload + 4)); //setting_prop_id
                Util_Transform_Str2u8Num(argv[4], payload + 6);

                BTMW_RPC_TEST_Logi("[mesh] prop_id:%04x setting_prop_id:%04x setting_raw:%02x\n",
                                *((UINT16*)(payload + 2)), *((UINT16*)(payload + 4)), payload[6]);

                param.data = payload;
                INT32 ret = a_mtkapi_bt_mesh_send_packet(&param);
                BTMW_RPC_TEST_Logd("[mesh] result: %d\n", ret);
                break;
            }
            default:
                BTMW_RPC_TEST_Loge("[mesh] action is invalid\n");
                break;
        }
    }
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_mesh_cli_commands[] = {
    { (const char *)"version",              btmw_rpc_test_mesh_version_handler,            (const char *)" = get mesh sdk version:version"},
    { (const char *)"log",                  btmw_rpc_test_mesh_log_handler,                (const char *)" = set log level:log <bitmap>"},
    { (const char *)"init",                 btmw_rpc_test_mesh_init_handler,               (const char *)" = init <provisioner> [debug_level] [save2flash]"},
    { (const char *)"deinit",               btmw_rpc_test_mesh_deinit_handler,             (const char *)" = deinit"},
    { (const char *)"config",               btmw_rpc_test_mesh_config_handler,             (const char *)" = (1)config addr <addr>, (2)config ttl <ttl>, (3)config bind"},
    { (const char *)"delete",               btmw_rpc_test_mesh_delete_handler,             (const char *)" = delete [record]"},
    { (const char *)"prov_list",            btmw_rpc_test_mesh_prov_list_handler,          (const char *)" = prov_list [<scan_time>]"},
    { (const char *)"prov_invite",          btmw_rpc_test_mesh_prov_invite_handler,        (const char *)" = prov_invite <uuid> [<attention_duration>]"},
    { (const char *)"prov_start",           btmw_rpc_test_mesh_prov_start_handler,         (const char *)" = prov_start <netkey> <netidx> <ivindex> <address> <flags>"},
    { (const char *)"prov_run",             btmw_rpc_test_mesh_prov_run_handler,           (const char *)" = prov_run <uuid> <netkey> <netidx> <ivindex> <address> <flags> [<attention_duration>] [auth_method]"},
    { (const char *)"key",                  btmw_rpc_test_mesh_key_handler,                (const char *)" = (1)key add|update net <netkey> <keyidx>, (2)key add|update app <appkey> <appidx> <netidx>"},
    { (const char *)"save_info",            btmw_rpc_test_mesh_save2flash_handler,         (const char *)" = save_info"},
    { (const char *)"set_adv_para",         btmw_rpc_test_mesh_set_bearer_adv_para_handler,(const char *)" = set_adv_para <adv_period> <min_interval> <max_interval> <scan_period> <scan_interval> <scan_window>"},
    { (const char *)"start_adv",            btmw_rpc_test_mesh_start_adv_handler,          (const char *)" = start_adv <0:1|stop:start>"},
    { (const char *)"msg_config",           btmw_rpc_test_mesh_msg_config_handler,         (const char *)" = msg_config <opcode> <src> <dst> <ttl> <netkey_idx> [fied1] [filed2] [filed3]..."},
    { (const char *)"msg_onoff",            btmw_rpc_test_mesh_msg_onoff_handler,          (const char *)" = msg_onoff <action> [<onoff> <tid> <transTime> <delay> <reliable>]"},
    { (const char *)"msg_ctl",              btmw_rpc_test_mesh_msg_ctl_handler,            (const char *)" = (1)msg_ctl actual <action> [<lightness> <temp> <delta_uv> <tid> <transTime> <delay> <reliable>], \
                                                                                                             (2)msg_ctl temp <action> [<temp> <delta_uv> <tid> <transTime> <delay> <reliable>], \
                                                                                                             (3)msg_ctl temprange <action> [<range_min> <range_max> <reliable>], \
                                                                                                             (4)msg_ctl default <action> [<lightness> <temp> <delta_uv> <reliable>]"},
    { (const char *)"msg_hsl",              btmw_rpc_test_mesh_msg_hsl_handler,            (const char *)" = (1)msg_hsl actual <action> [<lightness> <hue> <sat> <tid> <transTime> <delay> <reliable>], \
                                                                                                             (2)msg_hsl target <action>, \
                                                                                                             (3)msg_hsl hue <action> [<hue> <tid> <transTime> <delay> <reliable>], \
                                                                                                             (4)msg_hsl sat <action> [<saturation> <tid> <transTime> <delay> <reliable>], \
                                                                                                             (5)msg_hsl range <action> [<hue_min> <hue_max> <saturation_min> <saturation_max> <reliable>], \
                                                                                                             (6)msg_hsl default <action> [<lightness> <hue> <saturation> <reliable>]"},
    { (const char *)"msg_ex",               btmw_rpc_test_mesh_msg_ex_handler,             (const char *)" = msg_ex <onoff> <tid> <transTime> <delay> <reliable>"},
    { (const char *)"dump",                 btmw_rpc_test_mesh_dump_handler,               (const char *)" = dump [uuid | net | trans | config | model | lpn | adv]"},
    { (const char *)"set_hbperiod",         btmw_rpc_test_mesh_set_hbperiod_handler,       (const char *)" = set_hbperiod <num 0x01~0x11> <hb_timeout>"},
    { (const char *)"gatt_connect",         btmw_rpc_test_mesh_gatt_connect_handler,       (const char *)" = gatt_connect <addr> <addr_type> <service_type>"},
    { (const char *)"gatt_disconnect",      btmw_rpc_test_mesh_gatt_disconnect_handler,    (const char *)" = gatt_disconnect"},
    { (const char *)"ota_op",               btmw_rpc_test_mesh_ota_operation_handler,      (const char *)" = ota_op [reg_cbk|fw_info_get|start|stop|apply] ...\
                                                                                                                 reg_cbk\
                                                                                                                 fw_info_get <node_addr>\
                                                                                                                 start <new_fw_file> <new_fw_size> <new_fw_id> <appkey_idx> <distributor_addr> <group_addr> <manual_apply> <node_num> <node#1> <node#2> ...\
                                                                                                                 stop <new_fw_id> <distributor_addr> \
                                                                                                                 apply"},
    { (const char *)"msg_sensor",           btmw_rpc_test_mesh_msg_sensor_handler,            (const char *)" = (1)msg_sensor descr_get [<property_id>], \
                                                                                                                (2)msg_sensor cadence <action> <prop_id> [<period_divisor> <trigger_type> <delta_down> <delta_up> <min_interval> <cadence_low> <cadence_high> <reliable>], \
                                                                                                                (3)msg_sensor settings_get [<property_id>], \
                                                                                                                (4)msg_sensor setting <action> <prop_id> <setting_prop_id> [<setting_raw> <reliable>], \
                                                                                                                (5)msg_sensor get [<property_id>], \
                                                                                                                (6)msg_sensor column_get <property_id> <raw_value_x>, \
                                                                                                                (6)msg_sensor series_get <property_id> [<raw_value_x1> <raw_value_x2>]"},
    { NULL, NULL, NULL }
};

int btmw_rpc_test_mesh_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_mesh_cli_commands;

    BTMW_RPC_TEST_Logi("[mesh] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_MESH, btmw_rpc_test_mesh_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_mesh_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD btmw_rpc_test_mesh_mod = {0};

    btmw_rpc_test_mesh_mod.mod_id = BTMW_RPC_TEST_MOD_MESH;
    strncpy(btmw_rpc_test_mesh_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_MESH, sizeof(btmw_rpc_test_mesh_mod.cmd_key));
    btmw_rpc_test_mesh_mod.cmd_handler = btmw_rpc_test_mesh_cmd_handler;
    btmw_rpc_test_mesh_mod.cmd_tbl = btmw_rpc_test_mesh_cli_commands;

    ret = btmw_rpc_test_register_mod(&btmw_rpc_test_mesh_mod);
    BTMW_RPC_TEST_Logi("[mesh] btmw_rpc_test_register_mod() returns: %d\n", ret);

    util_timer_init();

    pthread_mutex_init(&resend_msg_mutex, NULL);

    BTMW_RPC_TEST_MESH_DLIST_ALLOC(btmw_rpc_test_resend_msg_list);

    //Create msg resend thread.
    if (0 == _resend_msg_thread)
    {
        if (pthread_create(&_resend_msg_thread, NULL, _btmw_rpc_test_resend_msg_thread, NULL))
        {
            BTMW_RPC_TEST_Loge("[mesh] _btmw_rpc_test_resend_msg_thread fail\n");
            _resend_msg_thread = 0;
        }
    }

    if (!g_cli_pts_mode)
    {
        if (FALSE == fg_pre_init)
        {
            btmw_rpc_test_mesh_pre_init();
        }
    }
    return 0;
}

