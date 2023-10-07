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
#include "mtk_bt_service_ble_scanner_wrapper.h"

typedef struct
{
     unsigned char type;
     unsigned char len;
     UINT8 *value;
}ble_scanner_test_adv_data_item_t;

typedef struct
{
    ble_scanner_test_adv_data_item_t data[10];
}ble_scanner_test_adv_data_t;

UINT8 ble_scanner_id = 0;
UINT8 scan_type = 0;
UINT32 gatt_client_if = 0;
CHAR device_addr[MAX_BDADDR_LEN] = {0}; //its peer device addr, can be get from scan result

BT_GATTC_WRITE_TYPE write_type = BT_GATTC_WRITE_TYPE_REQ;
CHAR g_long_value[BT_GATT_MAX_VALUE_LEN] = "12345678901234567890123456789abccdefjjj";

CHAR g_value[BT_GATT_MAX_VALUE_LEN] = "test12";
BT_GATTC_EVENT_GET_GATT_DB_DATA g_db_data;
CHAR adv_name[BT_GATT_MAX_NAME_LEN] = "test";//this adv name will be used for to scan the device with name "test"

static BT_BLE_SCANNER_SCAN_FILT_DATA g_ble_scanner_filt_data[] =
{
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "Test", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"11:22:33:44:55:66", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "ffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "ffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345", "1234", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "12345", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345", "1234", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "12345", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0},
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, {0xDA,0x8F,0x06,0x01}, {0xDA,0x8F,0x06,0x01}, 4},
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0x0001, 0xFFFF, {0xDA,0x8F,0x06,0x01}, {0xDA,0x8F,0x06,0x01}, 4, "\0", "\0", 0},
    //add more...
};

//this data base can be got from get_gatt_db_callback, follow data is an example
BT_GATT_ATTR g_db_data_attr[] =
{
    {"00001111-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_PRIMARY_SERVICE,0,0,0,40},
    {"0000aab0-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_CHARACTERISTIC,GATT_ATTR_PROP_NOTIFY,0,GATT_ATTR_PERM_READ,42},
    {"00003333-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_DESCRIPTOR,0,0,GATT_ATTR_PERM_READ,43},
    {"00004444-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_CHARACTERISTIC,GATT_ATTR_PROP_WRITE,0,GATT_ATTR_PERM_WRITE,46},
    {"00005555-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_DESCRIPTOR,0,0,GATT_ATTR_PERM_WRITE,47},
    {"00008888-0000-1000-8000-aabbccddeeff",GATT_ATTR_TYPE_CHARACTERISTIC,GATT_ATTR_PROP_READ,0,GATT_ATTR_PERM_READ,49},
};

#define GATT_LIENT_TEST_CASE_RETURN_STR(_const,str) case _const: return str

void gatt_ble_scanner_test_decode_adv_data (UINT8* adv_data, ble_scanner_test_adv_data_t *parse_data)
{
    UINT8* ptr = adv_data;
    unsigned char count = 0;
    UINT8 *value = NULL;
    while (1)
    {
        if (value != NULL)
        {
            free(value);
            value = NULL;
        }
        char length = *ptr;
        if (length == 0) break;
        unsigned char type = *(ptr+1);
        unsigned char value_len = length-1;
        value = (UINT8*)malloc(value_len + 1);
        if (NULL == value)
        {
            return;
        }
        memset(value, 0, value_len + 1);
        memcpy(value, ptr+2, value_len);
        value[value_len] = '\0';
        if (count < 10)
        {
            parse_data->data[count].type = type;
            parse_data->data[count].len= value_len;
            parse_data->data[count].value= value;
        }
        ptr = ptr + length + 1;
        switch (type)
        {
            case 0x01: //Flags
                printf("Flags : %02X\n", value[0]);
                break;
            case 0x02: //16-bit uuid
            case 0x03: //16-bit uuid
            case 0x14: //16-bit service sol uuid
                {
                    printf("16-bit length: %d UUID: %02X%02X\n", value_len, value[1], value[0]);
                }
                break;
            case 0x04: //32-bit uuid
            case 0x05: //32-bit uuid
            case 0x1F: //32-bit service sol uuid
                {
                    printf("32-bit Service Class length: %d UUIDs : %02X%02X%02X%02X\n", value_len,
                        value[3], value[2], value[1], value[0]);
                }
                break;
            case 0x06: //128-bit uuid
            case 0x07: //128-bit uuid
            case 0x15: //128-bit service sol uuid
                {
                    printf("128-bit Service Class length: %d UUIDs : %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                        value_len, value[15], value[14], value[13], value[12], value[11], value[10], value[9], value[8], value[7], value[6],
                        value[5], value[4], value[3], value[2], value[1], value[0]);
                }
                break;
            case 0x08: //Shortened Local Name
                printf("Shortened Local length: %d Name : %s\n", value_len, value);
                break;
            case 0x09: //Complete Local Name
                printf("Complete Local length: %d Name : %s\n", value_len, value);
                break;
            case 0x0A: //Tx Power Level
                printf("Tx Power Level : %d\n", value[0]);
                break;
            case 0x1B: //LE Bluetooth Device Address
                {
                    printf("LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                    value[5], value[4], value[3],
                    value[2], value[1], value[0]);
                }
                break;
            case 0xFF: //Manufacturer Specific Data
                {
                    UINT8 i = 0;
                    int n = 0;
                    char *temp = NULL;
                    temp = (char *)malloc(value_len*2 - 3);
                    if (temp == NULL)
                    {
                        free(value);
                        return;
                    }
                    for (i = 2; i < value_len; i++)
                    {
                        n = sprintf(temp+(i-2)*2,"%02x",value[i]);
                        if ( n < 0)
                        {
                            free(temp);
                            free(value);
                            return;
                        }
                    }
                    printf("Company:%02x%02x Manu_data_len:%d Manu_Data: %s\n",
                        value[1], value[0], value_len, temp);
                    free(temp);
                }
                break;
            default:
                {
                    UINT8 j = 0;
                    int n = 0;
                    char *tmp = NULL;
                    tmp = (char *)malloc(value_len*2 - 3);
                    if (tmp == NULL)
                    {
                        free(value);
                        return;
                    }
                    for (j = 2; j < value_len; j++)
                    {
                        n = sprintf(tmp+(j-2)*2,"%02x",value[j]);
                        if ( n < 0)
                        {
                            free(tmp);
                            free(value);
                            return;
                        }
                    }
                    printf("Srvc_data_uuid:%02x%02x Srvc_data_len:%d Srvc_data: %s\n",
                        value[1], value[0], value_len, tmp);
                    free(tmp);
                }
                break;
        }
        count++;
    }
}

static void gatt_ble_scanner_test_parse_truncated_results(INT32 num_records, INT32 data_len, UINT8 *data)
{
    int position = 0;
    while (position < data_len) {
        //ba_addr
        printf("%s LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
                                                     data[5], data[4], data[3],data[2], data[1], data[0]);
        position += 11;
    }
}

static void gatt_ble_scanner_test_parse_full_results(INT32 num_records, INT32 data_len, UINT8 *data)
{
    int position = 0;
    while (position < data_len) {
        //ba_addr
        printf("%s LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
                                                     data[5], data[4], data[3],data[2], data[1], data[0]);
        position += 6;
        // Skip address type.
        position++;
        // Skip tx power level.
        position++;
        //rssi
        position++;
        //skip timestamp
        position += 2;

        //advertise packet
        int adv_packet_Len = data[position++];
        UINT8 *adv_packet = (UINT8 *)malloc(adv_packet_Len + 1);
        if (adv_packet == NULL)
        {
            return;
        }
        memset(adv_packet, 0, adv_packet_Len + 1);
        memcpy(adv_packet, data + position, adv_packet_Len);

        ble_scanner_test_adv_data_t adv_data;
        gatt_ble_scanner_test_decode_adv_data(adv_packet, &adv_data);
        if (NULL != adv_packet)
        {
            free(adv_packet);
        }
        //scan response packet.
        position += adv_packet_Len;
        int scan_response_packet_Len = data[position++];
        position += scan_response_packet_Len;
    }
}

static int gatt_ble_scanner_test_get_feature_selection(const BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data)
{
    int selc = 0;
    if (0 != strlen(scan_filt_data->bt_addr))
    {
        selc |= (1 << SCAN_FILT_TYPE_ADDRESS);
    }
    if (0 != strlen(scan_filt_data->srvc_uuid) && 0 != strlen(scan_filt_data->srvc_uuid_mask))
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_UUID);
    }
    if (0 != strlen(scan_filt_data->srvc_sol_uuid) && 0 != strlen(scan_filt_data->srvc_sol_uuid_mask))
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_SOL_UUID);
    }
    if (0 != strlen(scan_filt_data->local_name))
    {
        selc |= (1 << SCAN_FILT_TYPE_LOCAL_NAME);
    }
    if (0 != scan_filt_data->manu_data_len)
    {
        selc |= (1 << SCAN_FILT_TYPE_MANU_DATA);
    }
    if (0 != scan_filt_data->srvc_data_len)
    {
        selc |= (1 << SCAN_FILT_TYPE_SRVC_DATA);
    }
    return selc;
}


//ble scanner callback
static void gatt_ble_scanner_test_register_callback(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    if (NULL == cb_param)
    {
        return;
    }
    ble_scanner_id = cb_param->scanner_id;
    printf("%s scanner_id = %d\n", __func__, ble_scanner_id);
}

static void gatt_ble_scanner_test_scan_result_callback(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    if (NULL == cb_param)
    {
        return;
    }
    BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA *pt_scan_result = &cb_param->data.scan_result_data;
    strncpy(device_addr, pt_scan_result->bd_addr, MAX_BDADDR_LEN - 1);
    device_addr[MAX_BDADDR_LEN - 1] = '\0';
    printf("%s scanner_id = %d device_address = %s\n", __FUNCTION__, cb_param->scanner_id, pt_scan_result->bd_addr);
    ble_scanner_test_adv_data_t adv_data;
    gatt_ble_scanner_test_decode_adv_data(pt_scan_result->adv_data, &adv_data);
}

static void gatt_ble_scanner_test_batchscan_report_callback(BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA *pt_batchscan_report)
{
    if (NULL == pt_batchscan_report)
    {
        return;
    }
    if (pt_batchscan_report->report_format == BATCH_SCAN_MODE_TRUNCATED)
    {
        gatt_ble_scanner_test_parse_truncated_results(pt_batchscan_report->num_records,
            pt_batchscan_report->data_len,pt_batchscan_report->data);
    }
    else if (pt_batchscan_report->report_format == BATCH_SCAN_MODE_FULL)
    {
        if (pt_batchscan_report->data_len > 0)
        {
            printf("LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                                                     pt_batchscan_report->data[5], pt_batchscan_report->data[4],
                                                     pt_batchscan_report->data[3],pt_batchscan_report->data[2],
                                                     pt_batchscan_report->data[1], pt_batchscan_report->data[0]);
        }
        gatt_ble_scanner_test_parse_full_results(pt_batchscan_report->num_records,
            pt_batchscan_report->data_len, pt_batchscan_report->data);
    }
    else
    {
        printf("unknown report_format\n");
    }
}

static void gatt_ble_scanner_test_track_adv_callback(BT_BLE_SCANNER_TRACK_ADV_EVT_DATA *pt_track_adv)
{
    if (NULL == pt_track_adv)
    {
        return;
    }
    printf("%s pt_track_adv->btaddr =%s\n", __FUNCTION__, pt_track_adv->bd_addr);
    ble_scanner_test_adv_data_t adv_data;
    gatt_ble_scanner_test_decode_adv_data(pt_track_adv->p_adv_pkt_data, &adv_data);
}

static VOID gatt_ble_scanner_test_event_handle(BT_BLE_SCANNER_CALLBACK_PARAM *param, VOID *pv_tag)
{
    printf("%s scanner_id:%d, event: %d, size:%d\n",
        __FUNCTION__, param->scanner_id, param->event,
        sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));

    switch (param->event)
    {
        case BT_BLE_SCANNER_EVT_REGISTER:
            gatt_ble_scanner_test_register_callback(param);
            break;
        case BT_BLE_SCANNER_EVT_SCAN_RESULT:
            gatt_ble_scanner_test_scan_result_callback(param);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_REPORT:
            gatt_ble_scanner_test_batchscan_report_callback(&param->data.batchscan_report_data);
            break;
        case BT_BLE_SCANNER_EVT_TRACK_ADV:
            gatt_ble_scanner_test_track_adv_callback(&param->data.track_adv_data);
            break;
        default:
            printf("ble scanner callback event = %d", param->event);
            break;
    }
    return;
}

// basic function API
static int gatt_ble_scanner_test_register(VOID)
{
    mtkrpcapi_BtAppBleScannerCbk *func =
        (mtkrpcapi_BtAppBleScannerCbk *)gatt_ble_scanner_test_event_handle;
    return a_mtkapi_bt_ble_scanner_register(func, NULL);
}

static int gatt_ble_scanner_test_start_simple_scan(VOID)
{
    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM start_simple_scan_param;
    memset(&start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    start_simple_scan_param.scanner_id = ble_scanner_id;
    start_simple_scan_param.scan_type = scan_type;//scan type can be regular_scan, regular_scan_with_filter, batch_scan or batch scan_with_filter
    printf("%s scan_type= %d\n", __func__, scan_type);
    switch (start_simple_scan_param.scan_type)
    {
        case REGULAR_SCAN:
            start_simple_scan_param.scan_param.regular_scan_param.scan_mode
                = SCAN_MODE_LOW_POWER;//scan mode can be LOW_POWER, BALCANCED or LOW_LATENCY
            break;
        case REGULAR_SCAN_WITH_FILT:
            start_simple_scan_param.scan_param.regular_scan_filt_param.regular_scan_param.scan_mode
                = SCAN_MODE_LOW_POWER;
            start_simple_scan_param.scan_param.regular_scan_filt_param.scan_filt_param.dely_mode
                = FILT_DELY_MODE_IMMEDIATE;//dely mode can be immediate, on_found or batched
            start_simple_scan_param.scan_param.regular_scan_filt_param.scan_filt_param.num_of_matches
                = MATCH_NUM_ONE_ADVERTISEMENT;//num_of_matches can be ONE_ADVERTISEMENT, FEW_ADVERTISEMENT or MAX_ADVERTISEMENT
            start_simple_scan_param.scan_param.regular_scan_filt_param.scan_filt_param.filt_match_mode
                = MATCH_MODE_AGGRESSIVE;//filt_match_mode can be AGGRESSIVE or STICKY
            start_simple_scan_param.scan_param.regular_scan_filt_param.scan_filt_num = 1;//max_number is 12
            start_simple_scan_param.scan_param.regular_scan_filt_param.scan_filt_data[0]
                = g_ble_scanner_filt_data[0];//filter data can set different filter such as g_ble_scanner_filt_data
            break;
        case BATCH_SCAN:
            start_simple_scan_param.scan_param.batch_scan_param.scan_mode = SCAN_MODE_LOW_POWER;
            start_simple_scan_param.scan_param.batch_scan_param.scan_result_type
                = BATCH_SCAN_RESULT_TYPE_FULL;//scan_result_type can be full or truncated
            break;
        case BATCH_SCAN_WITH_FILT:
            start_simple_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.scan_mode
                = SCAN_MODE_LOW_POWER;
            start_simple_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.scan_result_type
                = BATCH_SCAN_RESULT_TYPE_FULL;
            start_simple_scan_param.scan_param.batch_scan_filt_param.scan_filt_num = 1;
            start_simple_scan_param.scan_param.batch_scan_filt_param.scan_filt_data[0]
                = g_ble_scanner_filt_data[0];
            break;
        default:
            return -1;
    }
    return a_mtkapi_bt_ble_scanner_start_simple_scan(&start_simple_scan_param);
}

static int gatt_ble_scanner_test_start_scan(VOID)
{
    BT_BLE_SCANNER_START_SCAN_PARAM start_scan_param;
    memset(&start_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));
    start_scan_param.scanner_id = ble_scanner_id;
    start_scan_param.scan_type = scan_type;
    printf("%s scan_type= %d\n", __func__, scan_type);
    switch (start_scan_param.scan_type)
    {
        case REGULAR_SCAN:
            start_scan_param.scan_param.regular_scan_param.scan_interval = 6553;//recommand value
            start_scan_param.scan_param.regular_scan_param.scan_windows = 6553;//recommand value
            break;
        case REGULAR_SCAN_WITH_FILT:
            start_scan_param.scan_param.regular_scan_filt_param.regular_scan_param.scan_interval = 6553;
            start_scan_param.scan_param.regular_scan_filt_param.regular_scan_param.scan_windows = 6553;
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_data[0] = g_ble_scanner_filt_data[0];
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].feat_seln =
                gatt_ble_scanner_test_get_feature_selection(&(start_scan_param.scan_param.regular_scan_filt_param.scan_filt_data)[0]);
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].list_logic_type = 0x00ff;//it can be 0-65535
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].filt_logic_type = 0;// it can be 0 or 1
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].rssi_high_thres = 0;// it can be 0-255
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].rssi_low_thres = 0;//it can be 0-255
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].dely_mode = 0;// it can be 0 or 1
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].found_timeout = 1500;
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].lost_timeout = 10;// it can be 0-65535
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].found_timeout_cnt = 1;
            start_scan_param.scan_param.regular_scan_filt_param.scan_filt_param[0].num_of_tracking_entries = 5;
            break;
        case BATCH_SCAN:

            start_scan_param.scan_param.batch_scan_param.batch_scan_full_max = 100;
            start_scan_param.scan_param.batch_scan_param.batch_scan_trunc_max = 0;
            start_scan_param.scan_param.batch_scan_param.batch_scan_notify_threshold = 0;//it can be 0-100
            start_scan_param.scan_param.batch_scan_param.batch_scan_mode =
                                            BATCH_SCAN_READ_REPORT_MODE_FULL;
            start_scan_param.scan_param.batch_scan_param.batch_scan_windows = 2400;
            start_scan_param.scan_param.batch_scan_param.batch_scan_interval = 8000;
            start_scan_param.scan_param.batch_scan_param.own_address_type = 0;//0:public or 1:random
            start_scan_param.scan_param.batch_scan_param.batch_scan_discard_rule = 0;//it can be 0 or 1
            start_scan_param.scan_param.batch_scan_param.batch_scan_read_report_mode =
                                            BATCH_SCAN_READ_REPORT_MODE_FULL;
            break;
        case BATCH_SCAN_WITH_FILT:
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_full_max = 100;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_trunc_max = 0;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_notify_threshold = 0;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_mode =
                                                                    BATCH_SCAN_READ_REPORT_MODE_FULL;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_windows = 2400;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_interval = 8000;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.own_address_type = 0;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_discard_rule = 0;
            start_scan_param.scan_param.batch_scan_filt_param.batch_scan_param.batch_scan_read_report_mode =
                                                                    BATCH_SCAN_READ_REPORT_MODE_FULL;
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_num = 1;
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_data[0] = g_ble_scanner_filt_data[0];
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].feat_seln =
                gatt_ble_scanner_test_get_feature_selection(&(start_scan_param.scan_param.regular_scan_filt_param.scan_filt_data)[0]);
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].list_logic_type = 0x00ff;//it can be 0-65535
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].filt_logic_type = 0;// it can be 0 or 1
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].rssi_high_thres = 0;// it can be 0-255
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].rssi_low_thres = 0;//it can be 0-255
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].dely_mode = 0;// it can be 0 or 1
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].found_timeout = 1500;
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].lost_timeout = 10;// it can be 0-65535
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].found_timeout_cnt = 1;
            start_scan_param.scan_param.batch_scan_filt_param.scan_filt_param[0].num_of_tracking_entries = 5;
            break;
        default:
            return -1;
    }
    return a_mtkapi_bt_ble_scanner_start_scan(&start_scan_param);
}

static int gatt_ble_scanner_test_stop_scan(VOID)
{
    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_scan_param;
    memset(&stop_scan_param, 0, sizeof(BT_BLE_SCANNER_STOP_SCAN_PARAM));
    stop_scan_param.scanner_id = ble_scanner_id;
    return a_mtkapi_bt_ble_scanner_stop_scan(&stop_scan_param);
}

static int gatt_ble_scanner_test_unregister(VOID)
{
    BT_BLE_SCANNER_UNREG_PARAM unreg_param;
    memset(&unreg_param, 0, sizeof(BT_BLE_SCANNER_UNREG_PARAM));
    unreg_param.scanner_id = ble_scanner_id;
    ble_scanner_id = 0;
    memset(device_addr, 0, sizeof(device_addr));
    return a_mtkapi_bt_ble_scanner_unregister(&unreg_param);
}

static int value_uint_to_str(UINT8 *value, UINT16 value_len)
{
    UINT8 i = 0;
    int n = 0;
    char *temp = NULL;
    temp = (char *)malloc(value_len*2 + 1);
    if (temp == NULL)
    {
        return -1;
    }
    temp[value_len*2] = '\0';
    for (i = 0; i < value_len; i++)
    {
        n = sprintf(temp + i*2, "%02x", value[i]);
        if (n < 0)
        {
            free(temp);
            return -1;
        }
    }
    printf("value: %s  vlaue_len: %d\n", temp, value_len);
    free(temp);
    return 0;
}

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

//callback event handle
static void gatt_client_test_register_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.register_data.status == BT_GATT_STATUS_OK)
    {
        gatt_client_if = param->client_if;
        printf("client_name: %s client_if: %d status: %d\n", param->data.register_data.client_name,
            gatt_client_if, param->data.register_data.status);
    }
    else
    {
        printf("registr fail\n");
        return;
    }
}

static void gatt_client_test_connection_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.connection_data.status == BT_GATT_STATUS_OK)
    {
        if (param->data.connection_data.connected)
        {
            printf("client_if: %d addr:%s connected\n", param->client_if,
                param->data.connection_data.addr);
        }
        else
        {
            printf("client_if: %d addr:%s disconnected\n", param->client_if,
                param->data.connection_data.addr);
        }
    }
    else
    {
        printf("connect/disconnect fail\n");
        return;
    }
}

static void gatt_client_test_notify_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT16 handle = 0, value_len = 0;
    int ret = 0;
    if (param == NULL)
    {
       return;
    }
    handle = param->data.notif_data.handle;
    value_len = param->data.notif_data.value_len;
    ret = value_uint_to_str(param->data.notif_data.value, value_len);
    if (ret == 0)
    {
        printf("client_if: %d addr:%s handle: %d is_notify: %d\n", param->client_if,
            param->data.notif_data.addr, handle, param->data.notif_data.is_notify);
    }
    else
    {
        printf("notify fail\n");
        return;
    }
}

static void gatt_client_test_read_char_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT16  handle = 0, value_len = 0;
    int ret = 0;
    if (param == NULL)
    {
       return;
    }
    if (param->data.read_char_rsp_data.status == BT_GATT_STATUS_OK)
    {
        handle = param->data.read_char_rsp_data.handle;
        value_len = param->data.read_char_rsp_data.value_len;
        ret = value_uint_to_str(param->data.read_char_rsp_data.value, value_len);
        if (ret == 0)
        {
            printf("client_if: %d addr:%s handle: %d\n",param->client_if,
                param->data.read_char_rsp_data.addr, handle);
        }
        else
        {
            printf("read char fail\n");
            return;
        }
    }
    else
    {
        printf("read char fail\n");
        return;
    }
}

static void gatt_client_test_write_char_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT16  handle = 0;
    if (param == NULL)
    {
       return;
    }
    if (param->data.write_char_rsp_data.status == BT_GATT_STATUS_OK)
    {
        handle = param->data.write_char_rsp_data.handle;
        printf("client_if: %d handle: %d addr: %s\n", param->client_if,handle,
            param->data.write_char_rsp_data.addr);
    }
    else
    {
        printf("write char fail\n");
        return;
    }
}

static void gatt_client_test_exec_write_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.exec_write_rsp_data.status == BT_GATT_STATUS_OK)
    {
        printf("addr: %s\n", param->data.exec_write_rsp_data.addr);
    }
    else
    {
        printf("exec write fail\n");
        return;
    }
}

static void gatt_client_test_read_desc_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT16  handle = 0, value_len = 0;
    int ret = 0;
    if (param == NULL)
    {
       return;
    }
    handle = param->data.read_desc_rsp_data.handle;
    value_len = param->data.read_desc_rsp_data.value_len;
    ret = value_uint_to_str(param->data.read_desc_rsp_data.value, value_len);
    if (ret == 0)
    {
        printf("client_if: %d addr:%s handle: %d\n", param->client_if,
                            param->data.read_desc_rsp_data.addr, handle);
    }
    else
    {
        printf("read_desc fail\n");
        return;
    }
}

static void gatt_client_test_write_desc_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT16  handle = 0;
    if (param == NULL)
    {
       return;
    }
    if (param->data.write_desc_rsp_data.status == BT_GATT_STATUS_OK)
    {
        handle = param->data.write_desc_rsp_data.handle;
        printf("client_if: %d addr: %s handle: %d\n", param->client_if,
            param->data.write_desc_rsp_data.addr, handle);
    }
    else
    {
        printf("write desc fail\n");
        return;
    }
}

static void gatt_client_test_read_rssi_rsp_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.read_rssi_data.status == BT_GATT_STATUS_OK)
    {
        printf("client_if: %d addr: %s rssi: %d\n", param->client_if,
            param->data.read_rssi_data.addr, param->data.read_rssi_data.rssi);
    }
    else
    {
        printf("read rssi fail\n");
        return;
    }
}

static void gatt_client_test_mtu_change_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.mtu_chg_data.status == BT_GATT_STATUS_OK)
    {
        printf("client_if: %d addr: %s mtu: %d\n", param->client_if,
            param->data.mtu_chg_data.addr, param->data.mtu_chg_data.mtu);
    }
    else
    {
        printf("mtu change fail\n");
        return;
    }
}

static void gatt_client_test_phy_read_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.phy_read_data.status == BT_GATT_STATUS_OK)
    {
        printf("client_if: %d addr: %s tx_phy: %d rx_phy: %d\n", param->client_if,
            param->data.phy_read_data.addr, param->data.phy_read_data.tx_phy,
            param->data.phy_read_data.rx_phy);
    }
    else
    {
        printf("read phy fail\n");
        return;
    }
}

static void gatt_client_test_phy_update_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.phy_upd_data.status == BT_GATT_STATUS_OK)
    {
        printf("client_if: %d addr: %s tx_phy: %d rx_phy: %d\n", param->client_if,
            param->data.phy_upd_data.addr, param->data.phy_upd_data.tx_phy,
            param->data.phy_upd_data.rx_phy);
    }
    else
    {
        printf("phy update fail\n");
        return;
    }
}

static void gatt_client_test_conn_update_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    if (param->data.conn_upd_data.status == BT_GATT_STATUS_OK)
    {
        printf("client_if: %d addr: %s interval: %d latency: %d timeout: %d\n",
            param->client_if, param->data.conn_upd_data.addr,
            param->data.conn_upd_data.interval, param->data.conn_upd_data.latency,
            param->data.conn_upd_data.timeout);
    }
    else
    {
        printf("conn update fail\n");
        return;
    }
}

static void gatt_client_test_get_gatt_db_callback(BT_GATTC_EVENT_PARAM *param)
{
    UINT32 i = 0;
    if (param == NULL)
    {
       return;
    }
    memset(&g_db_data, 0 , sizeof(BT_GATTC_EVENT_GET_GATT_DB_DATA));
    printf("addr = %s is_first= %d\n", param->data.get_gatt_db_data.addr,
        param->data.get_gatt_db_data.is_first);
    g_db_data.attr_cnt = param->data.get_gatt_db_data.attr_cnt;
    g_db_data.is_first = param->data.get_gatt_db_data.is_first;
    strncpy(g_db_data.addr, param->data.get_gatt_db_data.addr, MAX_BDADDR_LEN - 1);
    g_db_data.addr[MAX_BDADDR_LEN -1] = '\0';

    for (i = 0; i < g_db_data.attr_cnt; i++)
    {
        g_db_data.attrs[i].handle = param->data.get_gatt_db_data.attrs[i].handle;
        strncpy(g_db_data.attrs[i].uuid, param->data.get_gatt_db_data.attrs[i].uuid,
                                                         BT_GATT_MAX_UUID_LEN - 1);
        g_db_data.attrs[i].uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
        g_db_data.attrs[i].properties = param->data.get_gatt_db_data.attrs[i].properties;
        g_db_data.attrs[i].key_size = param->data.get_gatt_db_data.attrs[i].key_size;
        // Permissions are not discoverable using the attribute protocol. app get permission value is 0
        // Core 5.0, Part F, 3.2.5 Attribute Permissions
        g_db_data.attrs[i].permissions = param->data.get_gatt_db_data.attrs[i].permissions;
        g_db_data.attrs[i].type = param->data.get_gatt_db_data.attrs[i].type;

        printf("attr_type= %s handle= %d uuid= %s properties=%hhu key_size= %d\n",
            get_attr_type_str(param->data.get_gatt_db_data.attrs[i].type),
            g_db_data.attrs[i].handle, g_db_data.attrs[i].uuid,
            g_db_data.attrs[i].properties, g_db_data.attrs[i].key_size);
    }
}
static void gatt_client_test_discover_compl_callback(BT_GATTC_EVENT_PARAM *param)
{
    if (param == NULL)
    {
       return;
    }
    printf("client_if: %d addr: %s\n", param->client_if,
        param->data.discover_cmpl_data.addr);
}


static VOID gatt_client_test_event_handle(BT_GATTC_EVENT_PARAM *param, VOID *pv_tag)
{
    printf("%s client_if:%d, event: %d, size:%d\n",
        __FUNCTION__, param->client_if, param->event,
        sizeof(BT_GATTC_EVENT_PARAM));

    switch(param->event)
    {
        case BT_GATTC_EVENT_REGISTER:
            gatt_client_test_register_callback(param);
            break;
        case BT_GATTC_EVENT_CONNECTION:
            gatt_client_test_connection_callback(param);
            break;
        case BT_GATTC_EVENT_NOTIFY:
            gatt_client_test_notify_callback(param);
            break;
        case BT_GATTC_EVENT_READ_CHAR_RSP:
            gatt_client_test_read_char_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_WRITE_CHAR_RSP:
            gatt_client_test_write_char_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_EXEC_WRITE_RSP:
            gatt_client_test_exec_write_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_READ_DESC_RSP:
            gatt_client_test_read_desc_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_WRITE_DESC_RSP:
            gatt_client_test_write_desc_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_READ_RSSI_RSP:
            gatt_client_test_read_rssi_rsp_callback(param);
            break;
        case BT_GATTC_EVENT_MTU_CHANGE:
            gatt_client_test_mtu_change_callback(param);
            break;
        case BT_GATTC_EVENT_PHY_READ:
            gatt_client_test_phy_read_callback(param);
            break;
        case BT_GATTC_EVENT_PHY_UPDATE:
            gatt_client_test_phy_update_callback(param);
            break;
        case BT_GATTC_EVENT_CONN_UPDATE:
            gatt_client_test_conn_update_callback(param);
            break;
        case BT_GATTC_EVENT_GET_GATT_DB:
            gatt_client_test_get_gatt_db_callback(param);
            break;
        case BT_GATTC_EVENT_DISCOVER_COMPL:
            gatt_client_test_discover_compl_callback(param);
            break;
        default:
            break;
    }

    return;
}

static int gatt_client_test_register()
{
    CHAR client_name[BT_GATT_MAX_NAME_LEN] = "client-test";
    return a_mtkapi_bt_gattc_register(client_name,gatt_client_test_event_handle, NULL);
}

static int gatt_client_test_unregister()
{
    INT32 client_if = 0;
    client_if = gatt_client_if;
    gatt_client_if = 0;
    memset(device_addr, 0, sizeof(device_addr));
    return a_mtkapi_bt_gattc_unregister(client_if);
}

static int gatt_client_test_connect()
{
    BT_GATTC_CONNECT_PARAM conn_param;
    memset(&conn_param, 0, sizeof(BT_GATTC_CONNECT_PARAM));
    conn_param.client_if = gatt_client_if;
    strncpy(conn_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    conn_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_param.is_direct = 1; //1: connect_mode
    conn_param.initiating_phys = GATT_PHY_SETTING_LE_1M; //bit0-1m, bit1-2m, bit2-LE coded
    conn_param.opportunistic = 0; //first time to connect ,set 0
    conn_param.transport = GATT_TRANSPORT_TYPE_LE;//three type --auto, BR/EDR, LE
    return a_mtkapi_bt_gattc_connect(&conn_param);
}

static int gatt_client_test_disconnect()
{
    BT_GATTC_DISCONNECT_PARAM disc_param;
    memset(&disc_param, 0, sizeof(BT_GATTC_DISCONNECT_PARAM));
    disc_param.client_if = gatt_client_if;
    strncpy(disc_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    disc_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_disconnect(&disc_param);
}

static int gatt_client_test_refresh()
{
    BT_GATTC_REFRESH_PARAM refresh_param;
    memset(&refresh_param, 0, sizeof(BT_GATTC_REFRESH_PARAM));
    refresh_param.client_if = gatt_client_if;
    strncpy(refresh_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    refresh_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_refresh(&refresh_param);
}

static int gatt_client_test_read_char()
{
    UINT32 i = 0;
    int ret = 0;
    BT_GATTC_READ_CHAR_PARAM read_param;
    memset(&read_param, 0, sizeof(BT_GATTC_READ_CHAR_PARAM));
    read_param.client_if = gatt_client_if;
    strncpy(read_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    read_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    read_param.auth_req = BT_GATTC_AUTH_REQ_NONE; //NONE, MITM, NO MITTM,
    for (i = 0; i < g_db_data.attr_cnt; i++)
    {
        if (g_db_data.attrs[i].type == GATT_ATTR_TYPE_CHARACTERISTIC &&
            g_db_data.attrs[i].properties == 2 &&
            g_db_data.attrs[i].handle > 40)
        {
            read_param.handle = g_db_data.attrs[i].handle;
            ret = a_mtkapi_bt_gattc_read_char(&read_param);
            if (ret != 0)
            {
                printf("this char handle can not be read\n");
            }
        }
    }
    return 0;
}

static int gatt_client_test_read_char_by_uuid()
{
    UINT32 i = 0;
    int ret = 0;
    BT_GATTC_READ_BY_UUID_PARAM read_param;
    memset(&read_param, 0, sizeof(BT_GATTC_READ_BY_UUID_PARAM));
    read_param.client_if = gatt_client_if;
    strncpy(read_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    read_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    read_param.auth_req = BT_GATTC_AUTH_REQ_NONE;
    for (i = 0; i < g_db_data.attr_cnt; i++)
    {
        if (g_db_data.attrs[i].type == GATT_ATTR_TYPE_CHARACTERISTIC &&
            g_db_data.attrs[i].properties == 2 &&
            g_db_data.attrs[i].handle > 40)
        {
            strncpy(read_param.char_uuid, g_db_data.attrs[i].uuid,
                        BT_GATT_MAX_UUID_LEN - 1); //uuid can be get from get db callback
            read_param.char_uuid[BT_GATT_MAX_UUID_LEN - 1] = '\0';
            read_param.s_handle = g_db_data.attrs[i].handle -1;
            read_param.e_handle = g_db_data.attrs[i].handle;
            ret = a_mtkapi_bt_gattc_read_char_by_uuid(&read_param);
            if (ret != 0)
            {
                printf("this char handle can not be read\n");
            }
        }
    }
    return 0;
}

static int gatt_client_test_read_desc()
{
    UINT32 i = 0;
    int ret = 0;
    BT_GATTC_READ_DESC_PARAM read_param;
    memset(&read_param, 0, sizeof(BT_GATTC_READ_DESC_PARAM));
    read_param.client_if = gatt_client_if;
    strncpy(read_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    read_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    read_param.auth_req = BT_GATTC_AUTH_REQ_NONE; //NONE, MITM, NO MITTM
    for (i = 0; i < g_db_data.attr_cnt; i++)
    {
        if (g_db_data.attrs[i].type == GATT_ATTR_TYPE_DESCRIPTOR &&
            g_db_data.attrs[i].handle > 40)
        {
            read_param.handle = g_db_data.attrs[i].handle;
            ret = a_mtkapi_bt_gattc_read_desc(&read_param);
            if (ret != 0)
            {
                printf("this desc handle can not be read\n");
            }
        }
    }
    return 0;
}

static int gatt_client_test_write_char()
{
    UINT32 i = 0;
    int ret = 0;
    BT_GATTC_WRITE_CHAR_PARAM write_param;
    memset(&write_param, 0, sizeof(BT_GATTC_WRITE_CHAR_PARAM));
    write_param.client_if = gatt_client_if;
    strncpy(write_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    write_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_param.auth_req = BT_GATTC_AUTH_REQ_NONE;
    if (write_type == BT_GATTC_WRITE_TYPE_REQ)
    {
        write_param.write_type = write_type;
        write_param.value_len = strlen(g_value);
        strncpy(write_param.value, g_value, write_param.value_len);
    }
    else if (write_type == BT_GATTC_WRITE_TYPE_PREPARE)
    {
        write_param.write_type = write_type;
        write_param.value_len = strlen(g_long_value);
        strncpy(write_param.value, g_long_value, write_param.value_len);
    }

    for (i = 0; i < g_db_data.attr_cnt; i++)
    {
        if (g_db_data.attrs[i].type == GATT_ATTR_TYPE_CHARACTERISTIC &&
            g_db_data.attrs[i].properties == 8 &&
            g_db_data.attrs[i].handle > 40)
        {
            write_param.handle = g_db_data.attrs[i].handle;
            ret = a_mtkapi_bt_gattc_write_char(&write_param);
            if (ret != 0)
            {
                printf("this char handle can not be wrote\n");
            }
        }
    }

    return 0;
}

static int gatt_client_test_write_desc()
{
    BT_GATTC_WRITE_DESC_PARAM write_param;
    memset(&write_param, 0, sizeof(BT_GATTC_WRITE_DESC_PARAM));
    write_param.client_if = gatt_client_if;
    strncpy(write_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    write_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    write_param.auth_req = BT_GATTC_AUTH_REQ_NONE;
    write_param.value_len = strlen(g_value);
    strncpy(write_param.value, g_value, write_param.value_len);

    write_param.handle = g_db_data_attr[4].handle;
    return a_mtkapi_bt_gattc_write_desc(&write_param);
}

static int gatt_client_test_exec_write()
{
    BT_GATTC_EXEC_WRITE_PARAM exec_write_param;
    memset(&exec_write_param, 0, sizeof(BT_GATTC_EXEC_WRITE_PARAM));
    exec_write_param.client_if = gatt_client_if;
    strncpy(exec_write_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    exec_write_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    exec_write_param.execute = 1; //0-cancle execute, 1-execute
    return a_mtkapi_bt_gattc_exec_write(&exec_write_param);
}

static int gatt_client_test_reg_notification()
{
    BT_GATTC_REG_NOTIF_PARAM reg_notif_param;
    memset(&reg_notif_param, 0, sizeof(BT_GATTC_REG_NOTIF_PARAM));
    reg_notif_param.client_if = gatt_client_if;
    strncpy(reg_notif_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    reg_notif_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    reg_notif_param.handle = 42;//handle can be get from get db callback
    reg_notif_param.registered = 1;// 1-register, 0-unregister
    return a_mtkapi_bt_gattc_reg_notification(&reg_notif_param);
}

static int gatt_client_test_read_rssi()
{
    BT_GATTC_READ_RSSI_PARAM read_rssi_param;
    memset(&read_rssi_param, 0, sizeof(BT_GATTC_READ_RSSI_PARAM));
    read_rssi_param.client_if = gatt_client_if;
    strncpy(read_rssi_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    read_rssi_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_read_rssi(&read_rssi_param);
}

static int gatt_client_test_get_dev_type()
{
    BT_GATTC_GET_DEV_TYPE_PARAM get_dev_type_param;
    memset(&get_dev_type_param, 0, sizeof(BT_GATTC_GET_DEV_TYPE_PARAM));
    get_dev_type_param.client_if = gatt_client_if;
    strncpy(get_dev_type_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    get_dev_type_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_get_dev_type(&get_dev_type_param);
}

static int gatt_client_test_change_mtu()
{
    BT_GATTC_CHG_MTU_PARAM chg_mtu_param;
    memset(&chg_mtu_param, 0, sizeof(BT_GATTC_CHG_MTU_PARAM));
    chg_mtu_param.client_if = gatt_client_if;
    strncpy(chg_mtu_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    chg_mtu_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    chg_mtu_param.mtu = 100; //it is value can be 23-517, default value 23
    return a_mtkapi_bt_gattc_change_mtu(&chg_mtu_param);
}

static int gatt_client_test_conn_update()
{
    BT_GATTC_CONN_UPDATE_PARAM conn_update_paramt;
    memset(&conn_update_paramt, 0, sizeof(BT_GATTC_CONN_UPDATE_PARAM));
    conn_update_paramt.client_if = gatt_client_if;
    strncpy(conn_update_paramt.addr, device_addr, MAX_BDADDR_LEN - 1);
    conn_update_paramt.addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_update_paramt.min_interval = 100; /* unit: 1.25ms, 7.5ms ~ 4000ms */
    conn_update_paramt.max_interval = 200; /* unit: 1.25ms, 7.5ms ~ 4000ms */
    conn_update_paramt.latency = 10; /* unit: connection event, 0~499 */
    conn_update_paramt.timeout = 300; /* unit: 10ms, 100~32000ms */
    conn_update_paramt.min_ce_len = 0xFF; /* unit: 0.625ms, 0~0xFFFF */
    conn_update_paramt.max_ce_len = 0xFFF; /* unit: 0.625ms, 0~0xFFFF */
    return a_mtkapi_bt_gattc_conn_update(&conn_update_paramt);
}

static int gatt_client_test_set_prefer_phy()
{
    BT_GATTC_PHY_SET_PARAM phy_set_param;
    memset(&phy_set_param, 0, sizeof(BT_GATTC_PHY_SET_PARAM));
    phy_set_param.client_if = gatt_client_if;
    strncpy(phy_set_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    phy_set_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    phy_set_param.rx_phy = GATT_PHY_SETTING_LE_1M;
    phy_set_param.tx_phy = GATT_PHY_SETTING_LE_2M;
    phy_set_param.phy_options = GATT_PHY_OPT_CODED_S2;// S2, S8, GATT_PHY_OPT_CODED_NO_PREF
    return a_mtkapi_bt_gattc_set_prefer_phy(&phy_set_param);
}

static int gatt_client_test_read_phy()
{
    BT_GATTC_PHY_READ_PARAM read_phy_param;
    memset(&read_phy_param, 0, sizeof(BT_GATTC_PHY_READ_PARAM));
    read_phy_param.client_if = gatt_client_if;
    strncpy(read_phy_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    read_phy_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_read_phy(&read_phy_param);
}

static int gatt_client_test_get_gatt_db()
{
    BT_GATTC_GET_GATT_DB_PARAM get_gatt_db_param;
    memset(&get_gatt_db_param, 0, sizeof(BT_GATTC_GET_GATT_DB_PARAM));
    get_gatt_db_param.client_if = gatt_client_if;
    strncpy(get_gatt_db_param.addr, device_addr, MAX_BDADDR_LEN - 1);
    get_gatt_db_param.addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_gattc_get_gatt_db(&get_gatt_db_param);
}

/* this test flow just a demo, customer can reference this test flow
     to designed function flow*/
void test_flow()
{
    //flow 1, test simple scan function
    printf("=============begin test flow 1\n");
    gatt_ble_scanner_test_register();
    while(1)
    {
        if (ble_scanner_id == 0)
        {
            sleep(1);
        }
        else
        {
            break;
        }
    }
    //case1: test regular scan
    gatt_ble_scanner_test_start_simple_scan();
    sleep(3);
    gatt_ble_scanner_test_stop_scan();
    //case2: test regular filter scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_simple_scan();
    sleep(3);
    gatt_ble_scanner_test_stop_scan();
    //case3: test batch scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_simple_scan();
    sleep(6);
    gatt_ble_scanner_test_stop_scan();
    //case 4: test batch filter scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_simple_scan();
    sleep(6);
    gatt_ble_scanner_test_stop_scan();
    scan_type = 0;

    //flow 2: test normal scan function
    printf("=============begin test flow 2\n");
    //case1: test regular scan
    gatt_ble_scanner_test_start_scan();
    sleep(3);
    gatt_ble_scanner_test_stop_scan();
    //case2: test regular filter scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_scan();
    sleep(3);
    gatt_ble_scanner_test_stop_scan();
    //case3: test batch scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_scan();
    sleep(6);
    gatt_ble_scanner_test_stop_scan();
    //case 4: test batch filter scan
    scan_type = scan_type + 1;
    gatt_ble_scanner_test_start_scan();
    sleep(6);
    gatt_ble_scanner_test_stop_scan();
    scan_type = 0;

    gatt_ble_scanner_test_unregister();

    //flow 3: test gatt client function
    printf("=============begin test flow 3\n");
    gatt_ble_scanner_test_register();
    while(1)
    {
        if (ble_scanner_id == 0)
        {
            sleep(1);
        }
        else
        {
            break;
        }
    }
    gatt_client_test_register();
    while(1)
    {
        if (gatt_client_if == 0)
        {
            sleep(1);
        }
        else
        {
            break;
        }
    }
    scan_type = 1;
    gatt_ble_scanner_test_start_simple_scan(); //filter an adv with name Test
    sleep(3);
    gatt_ble_scanner_test_stop_scan();
    gatt_client_test_connect();
    sleep(3);
    gatt_client_test_refresh();
    sleep(3);
    gatt_client_test_read_rssi();
    sleep(3);
    gatt_client_test_get_dev_type();
    sleep(3);
    gatt_client_test_change_mtu();
    sleep(3);
    gatt_client_test_conn_update();
    sleep(3);
    gatt_client_test_read_phy();
    sleep(3);
    gatt_client_test_set_prefer_phy();
    sleep(3);

    //flow 4: test gatt client read/write
    printf("=============begin test flow 4\n");
    gatt_client_test_get_gatt_db();
    sleep(3);
    gatt_client_test_read_char();
    sleep(3);
    gatt_client_test_read_char_by_uuid();
    sleep(3);
    gatt_client_test_read_desc();
    sleep(3);
    gatt_client_test_write_char();
    sleep(3);
    gatt_client_test_write_desc();
    sleep(3);
    gatt_client_test_reg_notification();
    sleep(3);

    //test flow 5: test gatt client exec_write
    printf("=============begin test flow 5\n");
    write_type = BT_GATTC_WRITE_TYPE_PREPARE;
    gatt_client_test_write_char();
    sleep(3);
    gatt_client_test_write_char();
    sleep(3);
    gatt_client_test_exec_write();
    sleep(3);
    gatt_client_test_disconnect();
    sleep(3);
    gatt_client_test_unregister();
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
    ret = a_mtkapi_bt_gap_set_name("gatt_client");
    ret = a_mtkapi_bt_gatt_profile_init();
    if (ret != 0)
    {
        printf("gattc base init fail\n");
    }

    test_flow();
    if (signal(SIGINT, signal_handle) == SIG_ERR)
    {
        return -1;
    }
    pause();
    return 0;
}
