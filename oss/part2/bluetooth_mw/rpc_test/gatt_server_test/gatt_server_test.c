#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "rw_init_mtk_bt_service.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatt_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_ble_advertiser_wrapper.h"

#define BT_BLE_ADV_MAX_ADV_DATA_DATA_LEN 29
#define INVALID_TX_POWER 40

typedef struct
{
    INT32 server_if;
    BT_GATT_STATUS status;
    CHAR server_name[BT_GATT_MAX_NAME_LEN];
}BT_GATTS_SERVER;

typedef struct
{
    CHAR local_name[BT_BLE_ADV_LOCAL_NAME_MAX_LEN];
    CHAR server_uuid[BT_GATT_MAX_UUID_LEN];
    INT8 tx_power;
    char srvc_uuid[BT_GATT_MAX_UUID_LEN];
    UINT8 srvc_data[BT_BLE_ADV_MAX_ADV_DATA_DATA_LEN];
    UINT16 srvc_data_len;
    UINT16 manu_id;
    UINT8 manu_data[BT_BLE_ADV_MAX_ADV_DATA_DATA_LEN];
    UINT16 manu_data_len;
}ADV_DATA_DATA;

typedef enum
{
    GATT_TEST_LEGACY_ADV,
    GATT_TEST_EXT_ADV,
    GATT_TEST_PERIODIC_ADV,
}ADV_TYPE;

ADV_TYPE adv_flag = GATT_TEST_LEGACY_ADV;

ADV_DATA_DATA legacy_data = {"test1","1234",2,"6789",{0x34},1,1,{0x56},1};
ADV_DATA_DATA ext_data = {"test2","4567",2,"6789",{0x34},1,1,{0x56},1};
ADV_DATA_DATA peri_data = {"test3","5678",2,"6789",{0x34},1,1,{0x56},1};

INT32 g_server_if = 0;
UINT8 g_adv_id = 0;
char addr[MAX_BDADDR_LEN] = {0};
BOOL adv_enable_flag = 0;
BOOL peri_enable_flag = 0;

// set wifi service
char wifi_service_uuid[5] = "8E20";
char wifi_pri_uuid[5] = "3698";
char wifi_name_uuid[5] = "1234";
char wifi_user_name[5] = "wifi";
char wifi_password_uuid[5] = "4567";
char wifi_password[9] = "88888888";

//set wifi value
char wifi_user_value[BT_GATT_MAX_VALUE_LEN] = "abcd";
char wifi_password_value[BT_GATT_MAX_VALUE_LEN] = "12345678";
BT_GATTS_EVENT_SERVICE_ADD_DATA g_server_data;

#define GATT_LIENT_TEST_CASE_RETURN_STR(_const,str) case _const: return str

static CHAR* get_attr_type_str(GATT_ATTR_TYPE type)
{
    switch((int)type)
    {
        GATT_LIENT_TEST_CASE_RETURN_STR(GATT_ATTR_TYPE_PRIMARY_SERVICE, "primary");
        GATT_LIENT_TEST_CASE_RETURN_STR(GATT_ATTR_TYPE_SECONDARY_SERVICE, "secondary");
        GATT_LIENT_TEST_CASE_RETURN_STR(GATT_ATTR_TYPE_INCLUDED_SERVICE, "include");
        GATT_LIENT_TEST_CASE_RETURN_STR(GATT_ATTR_TYPE_CHARACTERISTIC, "char");
        GATT_LIENT_TEST_CASE_RETURN_STR(GATT_ATTR_TYPE_DESCRIPTOR, "descriptor");
        default: return "UNKNOWN_TYPE";
   }
}

static int build_adv_data(BT_BLE_ADV_DATA_PARAM *adv_data, ADV_DATA_DATA *data)
{
    printf("[gatt_server_test]%s\n", __func__);
    int ret = 0;
    if (adv_data == NULL || data == NULL)
    {
        return -1;
    }
    if (strlen(data->local_name) != 0)
    {
        ret = a_mtkapi_bt_ble_adv_build_name(adv_data, data->local_name);
    }
    if (strlen(data->server_uuid) != 0)
    {
        ret = a_mtkapi_bt_ble_adv_build_service_uuid(adv_data, data->server_uuid);
    }
    if (data->tx_power != INVALID_TX_POWER)
    {
        ret = a_mtkapi_bt_ble_adv_build_tx_power(adv_data, data->tx_power);
    }
    if (data->srvc_data_len != 0)
    {
        ret = a_mtkapi_bt_ble_adv_build_service_data(adv_data, data->srvc_uuid,
            data->srvc_data, data->srvc_data_len);
    }
    if (data->manu_data_len != 0)
    {
        ret = a_mtkapi_bt_ble_adv_build_manu_data(adv_data, data->manu_id,
            data->manu_data, data->manu_data_len);
    }
    return ret;
}

static void uuid_to_128bit(CHAR *UUID, CHAR *uuid_temp)
{
    int len = 0;
    len = strlen(UUID);
    if (len == 4)
    {
        memcpy(&uuid_temp[4],UUID,len);
    }
    else
    {
        memcpy(uuid_temp,UUID,len);
    }
}

static void gatt_ble_advertiser_test_start_adv_set_callback(
    BT_BLE_ADV_EVENT_START_SET_DATA *start_set_data, UINT8 adv_id)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (start_set_data == NULL)
    {
        return;
    }
    g_adv_id = adv_id;
    printf("status:%d, tx_power:%d, adv_name:%s\n", start_set_data->status,
        start_set_data->tx_power, start_set_data->adv_name);
}

static void gatt_ble_advertiser_test_set_param_callback(BT_BLE_ADV_EVENT_SET_PARAM_DATA *set_param_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (set_param_data == NULL)
    {
        return;
    }
    printf("status:%d, tx_power:%d\n", set_param_data->status, set_param_data->tx_power);
}

static void gatt_ble_advertiser_test_set_data_callback(BT_BLE_ADV_EVENT_SET_ADV_DATA *set_adv_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (set_adv_data == NULL)
    {
        return;
    }
    printf("status:%d\n", set_adv_data->status);
}

static void gatt_ble_advertiser_test_set_scan_rsp_callback(BT_BLE_ADV_EVENT_SET_SCAN_RSP_DATA *set_scan_rsp_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (set_scan_rsp_data == NULL)
    {
        return;
    }
    printf("status:%d\n", set_scan_rsp_data->status);
}

static void gatt_ble_advertiser_test_set_periodic_param_callback(
                                BT_BLE_ADV_EVENT_SET_PERIODIC_PARAM_DATA *set_periodic_param_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (set_periodic_param_data == NULL)
    {
        return;
    }
    printf("status:%d\n", set_periodic_param_data->status);
}

static void gatt_ble_advertiser_test_set_periodic_data_callback(
                                            BT_BLE_ADV_EVENT_SET_PERIODIC_DATA *set_periodic_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (set_periodic_data == NULL)
    {
        return;
    }
    printf("status:%d\n", set_periodic_data->status);
}

static void gatt_ble_advertiser_test_adv_enable_callback(BT_BLE_ADV_EVENT_ENABLE_DATA *enable_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (enable_data == NULL)
    {
        return;
    }
    printf("status:%d, enable:%d\n", enable_data->status, enable_data->enable);
}

static void gatt_ble_advertiser_test_periodic_enable_callback(
                                     BT_BLE_ADV_EVENT_ENABLE_PERIODIC_DATA *enable_periodic_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (enable_periodic_data == NULL)
    {
        return;
    }
    printf("status:%d, enable:%d\n", enable_periodic_data->status, enable_periodic_data->enable);
}

static void gatt_ble_advertiser_test_get_own_addr_callback(BT_BLE_ADV_EVENT_GET_ADDR_DATA *get_addr_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (get_addr_data == NULL)
    {
        return;
    }
    printf("addr_type:%d, addr:%s\n", get_addr_data->address_type, get_addr_data->addr);
}

static VOID gatt_ble_advertiser_test_event_handle(BT_BLE_ADV_EVENT_PARAM *param, VOID *pv_tag)
{
    printf("%s adv_id:%d, event: %d, size:%d\n",
        __FUNCTION__, param->adv_id, param->event,
        sizeof(BT_BLE_ADV_EVENT_PARAM));

    switch(param->event)
    {
        case BT_BLE_ADV_EVENT_START_ADV_SET:
            gatt_ble_advertiser_test_start_adv_set_callback(&param->data.start_set_data,
                                                            param->adv_id);
            break;
        case BT_BLE_ADV_EVENT_SET_PARAM:
            gatt_ble_advertiser_test_set_param_callback(&param->data.set_param_data);
            break;
        case BT_BLE_ADV_EVENT_SET_DATA:
            gatt_ble_advertiser_test_set_data_callback(&param->data.set_adv_data);
            break;
        case BT_BLE_ADV_EVENT_SET_SCAN_RSP:
            gatt_ble_advertiser_test_set_scan_rsp_callback(&param->data.set_scan_rsp_data);
            break;
        case BT_BLE_ADV_EVENT_SET_PERI_PARAM:
            gatt_ble_advertiser_test_set_periodic_param_callback(&param->data.set_periodic_param_data);
            break;
        case BT_BLE_ADV_EVENT_SET_PERI_DATA:
            gatt_ble_advertiser_test_set_periodic_data_callback(&param->data.set_periodic_data);
            break;
        case BT_BLE_ADV_EVENT_ENABLE:
            gatt_ble_advertiser_test_adv_enable_callback(&param->data.enable_data);
            break;
        case BT_BLE_ADV_EVENT_ENABLE_PERI:
            gatt_ble_advertiser_test_periodic_enable_callback(&param->data.enable_periodic_data);
            break;
        case BT_BLE_ADV_EVENT_GET_ADDR:
            gatt_ble_advertiser_test_get_own_addr_callback(&param->data.get_addr_data);
            break;
        default:
            break;
    }

    return;
}

static int gatt_ble_advertiser_test_start_adv_set()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    int ret = 0;
    char name[] = "adv1";

    BT_BLE_ADV_START_SET_PARAM start_adv_set_param;
    memset(&start_adv_set_param, 0 , sizeof(BT_BLE_ADV_START_SET_PARAM));
    if (adv_flag == GATT_TEST_LEGACY_ADV)
    {
        //set adv_param
        strncpy(start_adv_set_param.adv_name, name, strlen(name));
        start_adv_set_param.adv_param.props.connectable = 1;
        start_adv_set_param.adv_param.props.scannable = 1;
        start_adv_set_param.adv_param.props.legacy = 1;
        start_adv_set_param.adv_param.props.anonymous = 0;
        start_adv_set_param.adv_param.props.include_tx_power = 0;
        start_adv_set_param.adv_param.min_interval = 100;
        start_adv_set_param.adv_param.max_interval = 200;
        start_adv_set_param.adv_param.tx_power = 0;
        start_adv_set_param.adv_param.primary_adv_phy = 1;
        start_adv_set_param.adv_param.secondary_adv_phy = 1;
        //set adv _data, method 1
        ret = build_adv_data(&start_adv_set_param.adv_data, &legacy_data);
        if (ret != 0)
        {
            printf("set adv data fail\n");
        }
        //set adv _data, method 2
        #if 0
        UINT8 data_name[] = {0x09,0x09,0x79,0x67,0x74,0x65,0x73,0x74,0x00,0x00,0x03,0x02,0x34,0x12};
        start_adv_set_param.adv_data.len = 14;
        memcpy(start_adv_set_param.adv_data.data, data_name, sizeof(data_name));
        #endif
    }
    else if (adv_flag == GATT_TEST_EXT_ADV)
    {
        //set adv_param
        strncpy(start_adv_set_param.adv_name, name, strlen(name));
        start_adv_set_param.adv_param.props.connectable = 1;
        start_adv_set_param.adv_param.props.scannable = 0;
        start_adv_set_param.adv_param.props.legacy = 0;
        start_adv_set_param.adv_param.props.anonymous = 0;
        start_adv_set_param.adv_param.props.include_tx_power = 0;
        start_adv_set_param.adv_param.min_interval = 1600;
        start_adv_set_param.adv_param.max_interval = 1620;
        start_adv_set_param.adv_param.tx_power = 0;
        start_adv_set_param.adv_param.primary_adv_phy = 1;
        start_adv_set_param.adv_param.secondary_adv_phy = 1;
        //set adv _data
        ret = build_adv_data(&start_adv_set_param.adv_data, &ext_data);
        if (ret != 0)
        {
            printf("set adv data fail\n");
        }
    }
    else
    {
        //set adv_param
        strncpy(start_adv_set_param.adv_name, name, strlen(name));
        start_adv_set_param.adv_param.props.connectable = 0;
        start_adv_set_param.adv_param.props.scannable = 0;
        start_adv_set_param.adv_param.props.legacy = 0;
        start_adv_set_param.adv_param.props.anonymous = 0;
        start_adv_set_param.adv_param.props.include_tx_power = 0;
        start_adv_set_param.adv_param.min_interval = 400;
        start_adv_set_param.adv_param.max_interval = 450;
        start_adv_set_param.adv_param.tx_power = 0;
        start_adv_set_param.adv_param.primary_adv_phy = 1;
        start_adv_set_param.adv_param.secondary_adv_phy = 1;
        //set adv_data
        ret = build_adv_data(&start_adv_set_param.adv_data, &peri_data);
        if (ret != 0)
        {
            printf("set adv data fail\n");
        }
        //set perodic param
        start_adv_set_param.peri_param.include_tx_power = 0;
        start_adv_set_param.peri_param.max_interval = 250;
        start_adv_set_param.peri_param.min_interval = 200;
        //set periodic _data
        ret = build_adv_data(&start_adv_set_param.peri_data, &peri_data);
        if (ret != 0)
        {
            printf("set adv data fail\n");
        }
    }
    mtkrpcapi_BtAppBleAdvCbk func = (mtkrpcapi_BtAppBleAdvCbk)gatt_ble_advertiser_test_event_handle;

    return a_mtkapi_bt_ble_adv_start_set(&start_adv_set_param, func, NULL);;
}

static int gatt_ble_advertiser_test_adv_stop_set()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_STOP_SET_PARAM stop_adv_set_param;
    memset(&stop_adv_set_param, 0 , sizeof(BT_BLE_ADV_STOP_SET_PARAM));
    stop_adv_set_param.adv_id = g_adv_id;
    return a_mtkapi_bt_ble_adv_stop_set(&stop_adv_set_param);
}

static int gatt_ble_advertiser_test_adv_set_param()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_PARAM adv_param;
    memset(&adv_param, 0 , sizeof(BT_BLE_ADV_PARAM));
    adv_param.adv_id = g_adv_id;
    adv_param.props.connectable = 1;
    adv_param.props.legacy = 1;
    adv_param.props.anonymous = 0;
    adv_param.props.include_tx_power = 1;
    adv_param.props.scannable = 1;
    adv_param.max_interval = 500;
    adv_param.min_interval = 300;
    adv_param.tx_power = 0;
    adv_param.primary_adv_phy = 1;
    adv_param.secondary_adv_phy = 1;
    return a_mtkapi_bt_ble_adv_set_param(&adv_param);
}

static int gatt_ble_advertiser_test_adv_set_data()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_DATA_PARAM adv_data_param;
    memset(&adv_data_param, 0 , sizeof(BT_BLE_ADV_DATA_PARAM));
    UINT8 data_name[] = {0x09,0x09,0x79,0x67,0x74,0x65,0x73,0x74,0x00,0x00,0x03,0x02,0x56,0x12};
    adv_data_param.adv_id= g_adv_id;
    adv_data_param.len = 14;
    memcpy(adv_data_param.data, data_name, sizeof(data_name));
    return a_mtkapi_bt_ble_adv_set_data(&adv_data_param);
}

static int gatt_ble_advertiser_test_adv_set_scan_rsp()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_DATA_PARAM scan_rsp;
    memset(&scan_rsp, 0 , sizeof(BT_BLE_ADV_DATA_PARAM));
    UINT8 data_name[] = {0x09,0x09,0x79,0x67,0x74,0x65,0x73,0x74,0x00,0x00,0x03,0x02,0x34,0x56};
    scan_rsp.adv_id = g_adv_id;
    scan_rsp.len = 14;
    memcpy(scan_rsp.data, data_name, sizeof(data_name));
    return a_mtkapi_bt_ble_adv_set_scan_rsp(&scan_rsp);
}

static int gatt_ble_advertiser_test_adv_enable()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_ENABLE_PARAM adv_enable_param;
    memset(&adv_enable_param, 0 , sizeof(BT_BLE_ADV_ENABLE_PARAM));
    adv_enable_param.adv_id = g_adv_id;
    adv_enable_param.duration = 0;
    adv_enable_param.enable = adv_enable_flag;
    adv_enable_param.max_ext_adv_events = 0;
    return a_mtkapi_bt_ble_adv_enable(&adv_enable_param);
}

static int gatt_ble_advertiser_test_adv_set_periodic_param()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_PERIODIC_PARAM periodic_param;
    memset(&periodic_param, 0 , sizeof(BT_BLE_ADV_PERIODIC_PARAM));
    periodic_param.adv_id = g_adv_id;
    periodic_param.enable = TRUE;
    periodic_param.include_tx_power = 1;
    periodic_param.max_interval = 150;
    periodic_param.min_interval = 100;
    return a_mtkapi_bt_ble_adv_set_periodic_param(&periodic_param);
}

static int gatt_ble_advertiser_test_adv_set_periodic_data()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_DATA_PARAM periodic_data;
    memset(&periodic_data, 0, sizeof(BT_BLE_ADV_DATA_PARAM));
    UINT8 data_name[] = {0x09,0x09,0x79,0x67,0x74,0x65,0x73,0x74,0x00,0x00,0x03,0x02,0x78,0x56};
    periodic_data.adv_id = g_adv_id;
    periodic_data.len = 14;
    memcpy(periodic_data.data, data_name, sizeof(data_name));
    memset(&periodic_data, 0 , sizeof(BT_BLE_ADV_DATA_PARAM));
    return a_mtkapi_bt_ble_adv_set_periodic_data(&periodic_data);
}

static int gatt_ble_advertiser_test_adv_periodic_enable()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_PERIODIC_ENABLE_PARAM periodic_enable_param;
    memset(&periodic_enable_param, 0 , sizeof(BT_BLE_ADV_PERIODIC_ENABLE_PARAM));
    periodic_enable_param.adv_id = g_adv_id;
    periodic_enable_param.enable = peri_enable_flag;
    return a_mtkapi_bt_ble_adv_periodic_enable(&periodic_enable_param);
}

static int gatt_ble_advertiser_test_adv_get_own_address()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_BLE_ADV_GET_ADDR_PARAM get_adv_addr_param;
    memset(&get_adv_addr_param, 0 , sizeof(BT_BLE_ADV_GET_ADDR_PARAM));
    get_adv_addr_param.adv_id = g_adv_id;
    return a_mtkapi_bt_ble_adv_get_own_address(&get_adv_addr_param);
}

static VOID gatt_server_test_register_server_callback(BT_GATTS_EVENT_REGISTER_DATA *register_data,
                                                               INT32 server_if)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (register_data == NULL)
    {
        printf("register_data is NULL\n");
        return;
    }
    if (register_data->status == BT_GATT_STATUS_OK)
    {
        g_server_if = server_if;
        printf("%s register success\n", register_data->server_name);
    }
    else
    {
        printf("%s register fail\n", register_data->server_name);
    }
}

static VOID gatt_server_test_connection_callback(BT_GATTS_EVENT_CONNECTION_DATA *connection_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (connection_data == NULL)
    {
        printf("connection_data is NULL\n");
        return;
    }
    if (connection_data->connected == 0)
    {
        printf("gatt server disconnect\n");
        memset(connection_data->addr, 0 , MAX_BDADDR_LEN);
    }
    else
    {
        strncpy(addr, connection_data->addr, MAX_BDADDR_LEN - 1);
        addr[MAX_BDADDR_LEN - 1] = '\0';
        printf("gatt server connet success--addr:%s\n", addr);
    }
    return;
}

static VOID gatt_server_test_service_add_callback(BT_GATTS_EVENT_SERVICE_ADD_DATA *add_service_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    UINT32 i = 0;
    if (add_service_data == NULL)
    {
        printf("add_service_data is NULL\n");
        return;
    }
    if (add_service_data->status == BT_GATT_STATUS_OK)
    {
        g_server_data.attr_cnt = add_service_data->attr_cnt;
        strncpy(g_server_data.service_uuid, add_service_data->service_uuid,
                                             BT_GATT_MAX_UUID_LEN - 1);
        g_server_data.service_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
        printf("server_uuid:%s\n",add_service_data->service_uuid);

        for (i = 0 ; i < add_service_data->attr_cnt; i++)
        {
            g_server_data.attrs[i].handle = add_service_data->attrs[i].handle;
            strncpy(g_server_data.attrs[i].uuid, add_service_data->attrs[i].uuid,
                                             BT_GATT_MAX_UUID_LEN - 1);
            g_server_data.attrs[i].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
            g_server_data.attrs[i].properties = add_service_data->attrs[i].properties;
            g_server_data.attrs[i].key_size = add_service_data->attrs[i].key_size;
            // Permissions are not discoverable using the attribute protocol. app get permission value is 0
            // Core 5.0, Part F, 3.2.5 Attribute Permissions
            g_server_data.attrs[i].permissions = add_service_data->attrs[i].permissions;
            g_server_data.attrs[i].type = add_service_data->attrs[i].type;

            printf("attr_type= %s handle= %d uuid= %s properties=%hhu key_size= %d\n",
                get_attr_type_str(add_service_data->attrs[i].type),
                g_server_data.attrs[i].handle, g_server_data.attrs[i].uuid,
                g_server_data.attrs[i].properties, g_server_data.attrs[i].key_size);
        }
    }
}

static VOID gatt_server_test_read_req_callback(BT_GATTS_EVENT_READ_REQ_DATA *read_req_data)
{
    printf("[gatt_server_test]--->%s\n", __func__);
    UINT32 i = 0;
    int len = 0,ret = 0;
    char uuid_name_temp[BT_GATT_MAX_UUID_LEN] = "00000000-0000-1000-8000-00805f9b34fb";
    uuid_name_temp[BT_GATT_MAX_UUID_LEN -1] = '\0';
    char uuid_password_temp[BT_GATT_MAX_UUID_LEN] = "00000000-0000-1000-8000-00805f9b34fb";
    uuid_password_temp[BT_GATT_MAX_UUID_LEN -1] = '\0';

    BT_GATTS_RESPONSE_PARAM resp_param;
    memset(&resp_param, 0, sizeof(BT_GATTS_RESPONSE_PARAM));
    if (read_req_data == NULL)
    {
        printf("read_req_data is NULL\n");
        return;
    }
    printf("addr:%s handle:%d trans_id:%d offset:%d\n",read_req_data->addr,
            read_req_data->handle, read_req_data->trans_id, read_req_data->offset);
    strncpy(resp_param.addr, read_req_data->addr, MAX_BDADDR_LEN - 1);
    resp_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    resp_param.server_if = g_server_if;
    resp_param.trans_id = read_req_data->trans_id;
    resp_param.offset = read_req_data->offset;
    for(i = 0; i < g_server_data.attr_cnt; i++)
    {
        if (read_req_data->handle == g_server_data.attrs[i].handle)
        {
            resp_param.handle = read_req_data->handle;
            uuid_to_128bit(wifi_name_uuid, uuid_name_temp);
            uuid_to_128bit(wifi_password_uuid, uuid_password_temp);
            //char read response
            if (!strncasecmp(g_server_data.attrs[i].uuid, uuid_name_temp, BT_GATT_MAX_UUID_LEN -1))
            {
                len = strlen(wifi_user_value);
                if (read_req_data->offset >= len)
                {
                    resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                    resp_param.len = 0;
                    return;
                }
                else
                {
                    resp_param.status = BT_GATT_STATUS_OK;
                    resp_param.len = len - read_req_data->offset;
                    if (BT_GATT_MAX_VALUE_LEN < resp_param.len)
                    {
                        resp_param.len = BT_GATT_MAX_VALUE_LEN;
                    }
                    memcpy((void*)resp_param.value,
                        &wifi_user_value[read_req_data->offset],
                        resp_param.len);
                    printf("send_response-->read_char_handle:%d\n",resp_param.handle);
                    ret = a_mtkapi_bt_gatts_send_response(&resp_param);
                    if (ret != 0)
                    {
                        printf("send response fail\n");
                    }
                    return;
                }
            }

            //desc read response
            if (!strncasecmp(g_server_data.attrs[i].uuid, uuid_password_temp, BT_GATT_MAX_UUID_LEN -1))
            {
                len = strlen(wifi_password_value);
                if (read_req_data->offset >= len)
                {
                    resp_param.status = BT_GATT_STATUS_INVALID_OFFSET;
                    resp_param.len = 0;
                    return;
                }
                else
                {
                    resp_param.status = BT_GATT_STATUS_OK;
                    resp_param.len = len - read_req_data->offset;
                    if (BT_GATT_MAX_VALUE_LEN < resp_param.len)
                    {
                        resp_param.len = BT_GATT_MAX_VALUE_LEN;
                    }
                    memcpy((void*)resp_param.value,
                        &wifi_password_value[read_req_data->offset],
                        resp_param.len);
                    printf("send_response-->read_desc_handle:%d\n",resp_param.handle);
                    ret = a_mtkapi_bt_gatts_send_response(&resp_param);
                    if (ret != 0)
                    {
                        printf("send response fail\n");
                    }
                    return;
                }
            }
        }
    }
}

static VOID gatt_server_test_write_req_callback(BT_GATTS_EVENT_WRITE_REQ_DATA *write_req_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    int ret = 0;
    BT_GATTS_RESPONSE_PARAM resp_param;
    memset(&resp_param, 0, sizeof(BT_GATTS_RESPONSE_PARAM));
    if (write_req_data == NULL)
    {
        printf("write_req_data is NULL\n");
        return;
    }
    printf("addr:%s, handle:%d, offset:%d, len:%d, is_prep:%d, need_rsp:%d, trans_id:%d\n",
            write_req_data->addr, write_req_data->handle, write_req_data->offset,
            write_req_data->value_len, write_req_data->is_prep, write_req_data->need_rsp,
            write_req_data->trans_id);
    //write response
    if (write_req_data->need_rsp == 1)
    {
        strncpy(resp_param.addr, write_req_data->addr, MAX_BDADDR_LEN - 1);
        resp_param.addr[MAX_BDADDR_LEN - 1] = '\0';
        resp_param.server_if = g_server_if;
        resp_param.handle = write_req_data->handle;
        resp_param.trans_id = write_req_data->trans_id;
        resp_param.offset = write_req_data->offset;
        resp_param.status = BT_GATT_STATUS_OK;
        resp_param.len = write_req_data->value_len;
        memcpy((void*)resp_param.value, write_req_data->value, resp_param.len);
        printf("send_response-->write_desc_handle:%d\n",resp_param.handle);
        ret = a_mtkapi_bt_gatts_send_response(&resp_param);
        if (ret != 0)
        {
            printf("send response fail\n");
        }
    }
}

static VOID gatt_server_test_write_exe_req_callback(BT_GATTS_EVENT_WRITE_EXE_REQ_DATA *write_exe_req_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (write_exe_req_data == NULL)
    {
        printf("write_exe_req_data is NULL\n");
        return;
    }
    printf("addr:%s, trans_id:%d, exec_write:%d\n", write_exe_req_data->addr,
        write_exe_req_data->trans_id, write_exe_req_data->exec_write);
}

static VOID gatt_server_test_ind_send_callback(BT_GATTS_EVENT_IND_SENT_DATA *ind_sent_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (ind_sent_data == NULL)
    {
        printf("ind_sent_data is NULL\n");
        return;
    }
    if (ind_sent_data->status == BT_GATT_STATUS_OK)
    {
        printf("addr:%s\n", ind_sent_data->addr);
    }
}

static VOID gatt_server_test_mtu_change_callback(BT_GATTS_EVENT_MTU_CHG_DATA *mtu_chg_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (mtu_chg_data == NULL)
    {
        printf("mtu_chg_data is NULL\n");
        return;
    }
    printf("mtu change success, add:%s, mtu:%d\n", mtu_chg_data->addr, mtu_chg_data->mtu);
}

static VOID gatt_server_test_phy_read_callback(BT_GATTS_EVENT_READ_PHY_DATA *read_phy_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (read_phy_data == NULL)
    {
        printf("read_phy_data is NULL\n");
        return;
    }
    if (read_phy_data->status == BT_GATT_STATUS_OK)
    {
        printf("read_phy success, addr:%s, rx_phy:%d, tx_phy:%d\n", read_phy_data->addr,
            read_phy_data->rx_phy, read_phy_data->tx_phy);
    }
}

static VOID gatt_server_test_phy_update_callback(BT_GATTS_EVENT_PHY_UPDATE_DATA *phy_upd_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (phy_upd_data == NULL)
    {
        printf("phy_upd_data is NULL\n");
        return;
    }
    if (phy_upd_data->status == BT_GATT_STATUS_OK)
    {
        printf("phy_update success, addr:%s, rx_phy:%d, tx_phy:%d\n", phy_upd_data->addr,
            phy_upd_data->rx_phy, phy_upd_data->tx_phy);
    }
}

static VOID gatt_server_test_connection_update_callback(BT_GATTS_EVENT_CONN_UPDATE_DATA *conn_upd_data)
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    if (conn_upd_data == NULL)
    {
        printf("read_phy_data is NULL\n");
        return;
    }
    if (conn_upd_data->status == BT_GATT_STATUS_OK)
    {
        printf("connection_update_success, addr:%s, interval:%d, latency:%d, timeout:%d\n",
            conn_upd_data->addr, conn_upd_data->interval, conn_upd_data->latency,
            conn_upd_data->timeout);
    }
}

static VOID gatt_server_test_event_handle(BT_GATTS_EVENT_PARAM *param, VOID *pv_tag)
{
    printf("%s server_if:%d, event: %d, size:%d\n",
        __FUNCTION__, param->server_if, param->event,
        sizeof(BT_GATTS_EVENT_PARAM));
    if (param == NULL)
    {
        return;
    }
    switch(param->event)
    {
        case BT_GATTS_EVENT_REGISTER:
            gatt_server_test_register_server_callback(&param->data.register_data,
                                                       param->server_if);
            break;
        case BT_GATTS_EVENT_CONNECTION:
            gatt_server_test_connection_callback(&param->data.connection_data);
            break;
        case BT_GATTS_EVENT_SERVICE_ADD:
            gatt_server_test_service_add_callback(&param->data.add_service_data);
            break;
        case BT_GATTS_EVENT_READ_REQ:
            gatt_server_test_read_req_callback(&param->data.read_req_data);
            break;
        case BT_GATTS_EVENT_WRITE_REQ:
            gatt_server_test_write_req_callback(&param->data.write_req_data);
            break;
        case BT_GATTS_EVENT_WRITE_EXE_REQ:
            gatt_server_test_write_exe_req_callback(&param->data.write_exe_req_data);
            break;
        case BT_GATTS_EVENT_IND_SENT:
            gatt_server_test_ind_send_callback(&param->data.ind_sent_data);
            break;
        case BT_GATTS_EVENT_MTU_CHANGE:
            gatt_server_test_mtu_change_callback(&param->data.mtu_chg_data);
            break;
        case BT_GATTS_EVENT_PHY_READ:
            gatt_server_test_phy_read_callback(&param->data.read_phy_data);
            break;
        case BT_GATTS_EVENT_PHY_UPDATE:
            gatt_server_test_phy_update_callback(&param->data.phy_upd_data);
            break;
        case BT_GATTS_EVENT_CONN_UPDATE:
            gatt_server_test_connection_update_callback(&param->data.conn_upd_data);
            break;
        default:
            break;
    }

    return;
}

//Function api sampol
static int gatt_server_test_register_server()
{
    printf("[gatt_server_test]--->%s:\n", __func__);

    char server_name[] = "gatt-server-test";
    mtkrpcapi_BtAppGATTSCbk fun = (mtkrpcapi_BtAppGATTSCbk)gatt_server_test_event_handle;
    return a_mtkapi_bt_gatts_register(server_name, fun, NULL);
}

static int gatt_server_test_unregister_server()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    INT32 server_if = 0;
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    server_if = g_server_if;
    return a_mtkapi_bt_gatts_unregister(server_if);
}

static int gatt_server_test_connect()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_CONNECT_PARAM conn_param;
    memset(&conn_param, 0 , sizeof(BT_GATTS_CONNECT_PARAM));
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    conn_param.server_if = g_server_if;
    strncpy(conn_param.addr, addr, MAX_BDADDR_LEN - 1);
    conn_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_param.is_direct = 1; // 0-bg connection, 1-direct connect
    //GATTS_TRANSPORT_AUTO, GATTS_TRANSPORT_BREDR, GATTS_TRANSPORT_LE
    conn_param.transport = GATT_TRANSPORT_TYPE_LE;
    return a_mtkapi_bt_gatts_connect(&conn_param);
}

static int gatt_server_test_disconnect()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_DISCONNECT_PARAM disc_param;
    memset(&disc_param, 0 , sizeof(BT_GATTS_DISCONNECT_PARAM));
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    disc_param.server_if = g_server_if;
    strncpy(disc_param.addr, addr, MAX_BDADDR_LEN - 1);
    disc_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gatts_disconnect(&disc_param);
}

static int gatt_server_test_add_service()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATT_ATTR attrs[3] = {0};
    BT_GATTS_SERVICE_ADD_PARAM service_add_param;
    memset(&service_add_param, 0 , sizeof(BT_GATTS_SERVICE_ADD_PARAM));
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    service_add_param.server_if = g_server_if;
    strncpy(service_add_param.service_uuid, wifi_service_uuid, strlen(wifi_service_uuid));
    service_add_param.service_uuid[strlen(wifi_service_uuid)] = '\0';
    service_add_param.attr_cnt = 3;
    //add primary service
    attrs[0].type = GATT_ATTR_TYPE_PRIMARY_SERVICE;
    strncpy(attrs[0].uuid, wifi_pri_uuid, strlen(wifi_pri_uuid));
    //add char
    attrs[1].type = GATT_ATTR_TYPE_CHARACTERISTIC;
    strncpy(attrs[1].uuid, wifi_name_uuid, strlen(wifi_name_uuid));
    attrs[1].properties = 0x3A;
    attrs[1].permissions = 0x11;
    //add desc
    attrs[2].type = GATT_ATTR_TYPE_DESCRIPTOR;
    strncpy(attrs[2].uuid, wifi_password_uuid,
                            strlen(wifi_password_uuid));
    attrs[2].permissions = 0x11;
    service_add_param.attrs = attrs;
    return a_mtkapi_bt_gatts_add_service(&service_add_param);
}

static int gatt_server_test_del_service()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_SERVICE_DEL_PARAM del_param;
    memset(&del_param, 0, sizeof(BT_GATTS_SERVICE_DEL_PARAM));
    del_param.server_if = g_server_if;
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    del_param.service_handle = 1; // wifi_service handle
    return a_mtkapi_bt_gatts_del_service(&del_param);
}

static int gatt_server_test_send_indication()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_IND_PARAM ind_param;
    UINT32 i = 0;
    memset(&ind_param, 0, sizeof(BT_GATTS_IND_PARAM));
    ind_param.server_if = g_server_if;
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    strncpy(ind_param.addr, addr, MAX_BDADDR_LEN - 1);
    ind_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    for(i = 0; i < g_server_data.attr_cnt; i++)
    {
        if (!memcmp(g_server_data.attrs[i].uuid, wifi_name_uuid, sizeof(wifi_name_uuid)))
        {
            ind_param.char_handle = g_server_data.attrs[i].handle;
            ind_param.need_confirm = 0;
            ind_param.value_len = strlen(wifi_user_name);
            break;
        }
    }

    if (ind_param.need_confirm == 0)
    {
        printf("send notification\n");
    }
    else
    {
        printf("send indication\n");
    }
    return a_mtkapi_bt_gatts_send_indication(&ind_param);
}

static int gatt_server_test_read_phy()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_PHY_READ_PARAM phy_read_param;
    memset(&phy_read_param, 0, sizeof(BT_GATTS_PHY_READ_PARAM));
    phy_read_param.server_if = g_server_if;
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    strncpy(phy_read_param.addr, addr, MAX_BDADDR_LEN - 1);
    phy_read_param.addr[MAX_BDADDR_LEN - 1] = '\0';

    return a_mtkapi_bt_gatts_read_phy(&phy_read_param);
}

static int gatt_server_test_set_prefer_phy()
{
    printf("[gatt_server_test]--->%s:\n", __func__);
    BT_GATTS_PHY_SET_PARAM phy_param;
    memset(&phy_param, 0 , sizeof(BT_GATTS_PHY_SET_PARAM));
    phy_param.server_if = g_server_if;
    if (g_server_if <= 0)
    {
        printf("no server registered\n");
        return 0;
    }
    strncpy(phy_param.addr, addr, MAX_BDADDR_LEN - 1);
    phy_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    phy_param.rx_phy = 0x01; //bit 0: LE phy 1M, bit 1: LE PHY 2M, bit 2: LE coded
    phy_param.tx_phy = 0x02; //bit 0: LE phy 1M, bit 1: LE PHY 2M, bit 2: LE coded
    phy_param.phy_options = 0x01; //0x01: prefer s=2 coding, 0x02:prefer s=8 coding

    return a_mtkapi_bt_gatts_set_prefer_phy(&phy_param);
}

/* this test flow just a demo, customer can reference this test flow
     to designed function flow*/
void test_flow()
{
    //flow 1, test register server and add service function
    printf("[gatt_server_test]flow 1:begin test register server and add service function\n");
    gatt_server_test_register_server();
    while(1)
    {
        if (g_server_if == 0)
        {
            sleep(1);
        }
        else
        {
            break;
        }
    }
    gatt_server_test_add_service();
    //flow 2, start an adv
    printf("[gatt_server_test]flow 2:start an legacy adv\n");
    gatt_ble_advertiser_test_start_adv_set(); //start adv success
    sleep(60);
    //flow 3, server basic function
    //before do reconnect, use client to connect and bond, then disconnect
    gatt_server_test_connect();
    sleep(3);
    gatt_server_test_send_indication();
    sleep(3);
    gatt_server_test_read_phy();
    sleep(3);
    gatt_server_test_set_prefer_phy();
    sleep(3);
    gatt_server_test_del_service();
    sleep(3);
    gatt_server_test_disconnect();
    sleep(3);

    gatt_ble_advertiser_test_adv_enable(); // disable adv
    sleep(3);
    gatt_ble_advertiser_test_adv_set_param();//change adv param
    sleep(3);
    //flow 4, change adv param and adv data
    printf("[gatt_server_test]flow 4:test change adv param and adv data\n");
    adv_enable_flag = 1;
    gatt_ble_advertiser_test_adv_enable();//enable adv
    sleep(3);
    gatt_ble_advertiser_test_adv_set_data();//change adv data
    sleep(3);
    gatt_ble_advertiser_test_adv_set_scan_rsp();//set scan resp data
    sleep(30);
    gatt_ble_advertiser_test_adv_stop_set();
    sleep(30);

    //flow 5, start an extend adv
    printf("[gatt_server_test]flow 5:start an extend adv\n");
    adv_flag = GATT_TEST_EXT_ADV;
    gatt_ble_advertiser_test_start_adv_set();//start an ext adv
    sleep(60);
    gatt_ble_advertiser_test_adv_stop_set();
    sleep(30);
    //flow 6, start an periodic adv
    printf("[gatt_server_test]flow 6:start an periodic adv\n");
    adv_flag = GATT_TEST_PERIODIC_ADV;
    gatt_ble_advertiser_test_start_adv_set();//start an periodic adv
    sleep(60);
    gatt_ble_advertiser_test_adv_periodic_enable();//disable peri adv
    sleep(3);
    gatt_ble_advertiser_test_adv_set_periodic_param();//change adv peri param
    sleep(3);
    peri_enable_flag = 1;
    gatt_ble_advertiser_test_adv_periodic_enable();//enable peri adv
    sleep(3);
    gatt_ble_advertiser_test_adv_set_periodic_data();//change peri adv data
    sleep(30);
    gatt_ble_advertiser_test_adv_get_own_address();
    sleep(3);

    gatt_ble_advertiser_test_adv_stop_set();
    sleep(3);
    gatt_server_test_unregister_server();
}

void signal_handle(int sig_num)
{
    _exit(1);
}

int main(void)
{
    int ret = 0;
    MTKRPCAPI_BT_APP_CB_FUNC gap_func;
    char gap_pv_tag[2] = {0};
    a_mtk_bt_service_init();
    ret = a_mtkapi_bt_gap_base_init(&gap_func, (void *)gap_pv_tag);
    ret = a_mtkapi_bt_gap_on_off(TRUE);
    ret = a_mtkapi_bt_gap_set_name("gatt_server_test");
    ret = a_mtkapi_bt_gatt_profile_init();

    if(ret != 0)
    {
        printf("gatts base init fail\n");
    }
    test_flow();
    if (signal(SIGINT, signal_handle) == SIG_ERR)
    {
        return -1;
    }
    pause();
    return 0;
}
