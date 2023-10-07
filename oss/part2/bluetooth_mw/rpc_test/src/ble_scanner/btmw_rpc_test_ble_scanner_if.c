/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include "list.h"
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_ble_scanner_if.h"
#include "mtk_bt_service_ble_scanner_wrapper.h"
#include "mtk_bt_service_gatt_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"

static UINT8 ble_scanner_scan_mode[] =
{
    SCAN_MODE_LOW_POWER,
    SCAN_MODE_BALANCED,
    SCAN_MODE_LOW_LATENCY
};

static UINT8 ble_scanner_scan_result_type[] =
{
    BATCH_SCAN_RESULT_TYPE_FULL,
    BATCH_SCAN_RESULT_TYPE_TRUNCATED
};

static UINT8 ble_scanner_dely_mode[] =
{
    FILT_DELY_MODE_IMMEDIATE,
    FILT_DELY_MODE_ON_FOUND
};

static UINT8 ble_scanner_num_of_match[] =
{
    MATCH_NUM_ONE_ADVERTISEMENT,
    MATCH_NUM_FEW_ADVERTISEMENT,
    MATCH_NUM_MAX_ADVERTISEMENT
};

static UINT8 ble_scanner_filt_match_mode[] =
{
    MATCH_MODE_AGGRESSIVE,
    MATCH_MODE_STICKY
};

static BT_BLE_SCANNER_SCAN_FILT_DATA ble_scanner_filt_data[] =
{
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "Test", 0, 0, "\0", "\0", 0, "\0", "\0", 0},  // index: 0
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "BleScannerBleScannerBleScanner", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 1
    {"11:22:33:44:55:66", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 2
    {"11122:33:44:55:66", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 3
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "ffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 4
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "0000ffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 5
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "0000ffff-0000-1000-8000-00805F9B34FB", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 6
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678", "ffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 7
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678", "ffffffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 8
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678", "ffffffff-0000-1000-8000-00805F9B34FB", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 9
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678-0001-1000-8000-00805F9B34FB", "ffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 10
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678-0001-1000-8000-00805F9B34FB", "ffffffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 11
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345678-0001-1000-8000-00805F9B34FB", "ffffffff-ffff-ffff-ffff-ffffffffffff", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 12
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "ffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 13
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "0000ffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 14
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "0000ffff-0000-1000-8000-00805F9B34FB", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 15
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678", "ffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 16
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678", "ffffffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 17
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678", "ffffffff-0000-1000-8000-00805F9B34FB", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 18
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678-0001-1000-8000-00805F9B34FB", "ffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 19
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678-0001-1000-8000-00805F9B34FB", "ffffffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 20
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345678-0001-1000-8000-00805F9B34FB", "ffffffff-ffff-ffff-ffff-ffffffffffff", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 21
    {"\0", SCAN_FILT_RANDOM_TYPE, "12345", "1234", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 22
    {"\0", SCAN_FILT_RANDOM_TYPE, "1234", "12345", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 23
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "12345", "1234", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 24
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "1234", "12345", "\0", 0, 0, "\0", "\0", 0, "\0", "\0", 0}, // index: 25
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, {0xDA,0x8F,0x06,0x01,0x02,0x00,0x30}, {0xDA,0x8F,0x06,0x01,0x02,0x00,0x30}, 7}, // index: 26
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0, 0, "\0", "\0", 0, {0xDB,0x8F}, {0xDB,0x8F}, 2}, // index: 27
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0x0001, 0xFFFF, {0xDA,0x8F,0x06,0x01,0x02,0x00,0x30}, {0xDA,0x8F,0x06,0x01,0x02,0x00,0x30}, 7, "\0", "\0", 0}, // index: 28
    {"\0", SCAN_FILT_RANDOM_TYPE, "\0", "\0", "\0", "\0", "\0", 0x0001, 0xFFFF, {0xDA,0x8F}, {0xDA,0x8F}, 2, "\0", "\0", 0} // index: 29
    //add more...
};

static INT32 ble_scanner_filt_data_multi[] =
{
    0x00000011, // 2 filt data, ble_scanner_filt_data index 0, 4
    0x00000031, // 4 filt data, ble_scanner_filt_data index 0, 4, 5
    0x00000071, // 5 filt data, ble_scanner_filt_data index 0, 4, 5, 6
    0x00002071, // 6 filt data, ble_scanner_filt_data index 0, 4, 5, 6, 13
    0x00006071, // 7 filt data, ble_scanner_filt_data index 0, 4, 5, 6, 13, 14
    0x0000e071, // 8 filt data, ble_scanner_filt_data index 0, 4, 5, 6, 13, 14, 15
    0x0400e071, // 9 filt data, ble_scanner_filt_data index 0, 4, 5, 6, 13, 14, 15, 26
    0x2400e071 // 10 filt data, ble_scanner_filt_data index 0, 4, 5, 6, 13, 14, 15, 26, 29
    //add more...
};

typedef struct
{
     unsigned char type;
     unsigned char len;
     UINT8 *value;
} btmw_rpc_test_gatt_adv_data_item_t;

typedef struct
{
    btmw_rpc_test_gatt_adv_data_item_t data[10];
} btmw_rpc_test_gatt_adv_data_t;

typedef struct
{
    CHAR bt_addr[MAX_BDADDR_LEN];
    struct dl_list node;
} BLE_SCANNER_ADDR_LIST;

typedef struct
{
    UINT8 scanner_id;
    struct dl_list addr_list;
}BLE_SCANNER_DEVICE_ADDR;

BLE_SCANNER_DEVICE_ADDR s_app_device_addr[BT_BLE_SCANNER_MAX_REG_NUM];

extern INT16 get_feature_selection(const BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data);


void btmw_rpc_test_ble_scanner_decode_adv_data (UINT8* adv_data, btmw_rpc_test_gatt_adv_data_t *parse_data);

void rpc_free_param(void *param)
{
    if (param != NULL)
    {
        free(param);
    }
}

void btmw_rpc_test_ble_scanner_decode_adv_data (UINT8* adv_data, btmw_rpc_test_gatt_adv_data_t *parse_data)
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
            printf("[GATT] malloc fail!\n");
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
                printf("Flags: %02X\n", value[0]);
                break;
            case 0x02: //16-bit uuid
            case 0x03: //16-bit uuid
            case 0x14: //16-bit service sol uuid
                {
                    printf("UUID: %02X%02X\n", value[1], value[0]);
                }
                break;
            case 0x04: //32-bit uuid
            case 0x05: //32-bit uuid
            case 0x1F: //32-bit service sol uuid
                {
                    printf("UUID: %02X%02X%02X%02X\n",
                        value[3], value[2], value[1], value[0]);
                }
                break;
            case 0x06: //128-bit uuid
            case 0x07: //128-bit uuid
            case 0x15: //128-bit service sol uuid
                {
                    printf("UUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                        value[15], value[14], value[13], value[12], value[11], value[10], value[9], value[8], value[7],
                        value[6], value[5], value[4], value[3], value[2], value[1], value[0]);
                }
                break;
            case 0x08: //Shortened Local Name
                printf("Name: %s\n", value);
                break;
            case 0x09: //Complete Local Name
                printf("Name: %s\n", value);
                break;
            case 0x0A: //Tx Power Level
                printf("Tx Power Level: %d\n", value[0]);
                break;
            case 0x1B: //LE Bluetooth Device Address
                {
                    printf("Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
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

static void parse_full_results(INT32 data_len, UINT8 *data)
{
    int position = 0;
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

    btmw_rpc_test_gatt_adv_data_t adv_data;
    btmw_rpc_test_ble_scanner_decode_adv_data(adv_packet, &adv_data);
    if (NULL != adv_packet)
    {
        free(adv_packet);
    }

    //scan response packet.
    position += adv_packet_Len;
    int scan_response_packet_Len = data[position++];
    position += scan_response_packet_Len;
}

static void list_remove_by_scanner_id(BLE_SCANNER_DEVICE_ADDR *device_addr, UINT8 scanner_id)
{
    if (NULL == device_addr)
    {
        BTMW_RPC_TEST_Logi("Invalid parameter.\n");
        return;
    }
    if (0 == dl_list_len(&device_addr->addr_list))
    {
        BTMW_RPC_TEST_Logi("No available node remove.\n");
        return;
    }

    BLE_SCANNER_ADDR_LIST *addr_item = NULL;

    if (scanner_id == device_addr->scanner_id)
    {
        while (!dl_list_empty(&device_addr->addr_list))
        {
            addr_item = dl_list_first(&device_addr->addr_list, BLE_SCANNER_ADDR_LIST, node);
            if (NULL != addr_item)
            {
                dl_list_del(&addr_item->node);
                free(addr_item);
            }
        }
        BTMW_RPC_TEST_Logi("ble_scanner_list_remove_by_scanner_id success!\n");
    }
    else
    {
        BTMW_RPC_TEST_Logi("ble_scanner_list_remove_by_scanner_id fail!\n");
        return;
    }
}

static void btmw_rpc_test_ble_scanner_show_device(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA *pt_scan_result = &cb_param->data.scan_result_data;
    printf("==============================Scan-Result-Device-Info============================\n");
    printf("Device info -- the process pid: %d scanner_id: %d device_addr: %s\n", getpid(),
    cb_param->scanner_id, pt_scan_result->bd_addr);
    btmw_rpc_test_gatt_adv_data_t adv_data;
    btmw_rpc_test_ble_scanner_decode_adv_data(pt_scan_result->adv_data, &adv_data);
    printf("=======================================end======================================\n");
}

static void btmw_rpc_test_ble_scanner_list_add_show_device(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    UINT32 i  = 0;
    if (NULL == cb_param)
    {
        BTMW_RPC_TEST_Loge("cb_param is NULL\n");
        return;
    }
    if (cb_param->scanner_id < 1)
    {
        return;
    }
    i = cb_param->scanner_id;

    BLE_SCANNER_ADDR_LIST *addr_item = NULL;
    BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA *pt_scan_result = &cb_param->data.scan_result_data;

    if (cb_param->scanner_id != s_app_device_addr[i].scanner_id)
    {
        addr_item = (BLE_SCANNER_ADDR_LIST *)malloc(sizeof(BLE_SCANNER_ADDR_LIST));
        if (addr_item == NULL)
        {
            return;
        }
        memset(addr_item, 0, sizeof(BLE_SCANNER_ADDR_LIST));

        s_app_device_addr[i].scanner_id = cb_param->scanner_id;
        strncpy(addr_item->bt_addr, pt_scan_result->bd_addr, MAX_BDADDR_LEN - 1);
        addr_item->bt_addr[MAX_BDADDR_LEN - 1] = '\0';
        dl_list_add(&s_app_device_addr[i].addr_list,&addr_item->node);
        btmw_rpc_test_ble_scanner_show_device(cb_param);
        return;
    }
    else
    {
        dl_list_for_each(addr_item, &s_app_device_addr[i].addr_list, BLE_SCANNER_ADDR_LIST, node)
        {
            if (!strncasecmp(addr_item->bt_addr, pt_scan_result->bd_addr, MAX_BDADDR_LEN - 1))
            {
                return;
            }
            else
            {
                continue;
            }
        }
    }
    addr_item = (BLE_SCANNER_ADDR_LIST *)malloc(sizeof(BLE_SCANNER_ADDR_LIST));
    if (addr_item == NULL)
    {
        return;
    }
    memset(addr_item, 0, sizeof(BLE_SCANNER_ADDR_LIST));
    strncpy(addr_item->bt_addr, pt_scan_result->bd_addr, MAX_BDADDR_LEN -1);
    addr_item->bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    dl_list_add(&s_app_device_addr[i].addr_list,&addr_item->node);
    btmw_rpc_test_ble_scanner_show_device(cb_param);
}

static void btmw_rpc_test_ble_scanner_batch_scan_show_device(UINT8 scanner_id, CHAR* data,
    BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA *pt_batchscan_report)
{
    UINT32 i  = 0;
    i = scanner_id;
    BLE_SCANNER_ADDR_LIST *addr_item = NULL;
    addr_item = (BLE_SCANNER_ADDR_LIST *)malloc(sizeof(BLE_SCANNER_ADDR_LIST));
    if (addr_item == NULL)
    {
        return;
    }
    memset(addr_item, 0, sizeof(BLE_SCANNER_ADDR_LIST));
    s_app_device_addr[i].scanner_id = scanner_id;
    memcpy(addr_item->bt_addr, data, MAX_BDADDR_LEN -1);
    addr_item->bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    dl_list_add(&s_app_device_addr[i].addr_list,&addr_item->node);
    if (data != NULL)
    {
        free(data);
    }
    printf("==============================Scan-Result-Device-Info============================\n");
    printf("Device info == the process pid: %d scanner_id: %d device_addr: %s\n", getpid(),
                                    scanner_id, addr_item->bt_addr);
    if (pt_batchscan_report->report_format == BATCH_SCAN_MODE_TRUNCATED)
    {
        printf("=======================================end======================================\n");
        return;
    }
    else if (pt_batchscan_report->report_format == BATCH_SCAN_MODE_FULL)
    {
        parse_full_results(pt_batchscan_report->data_len, pt_batchscan_report->data);
        printf("=======================================end======================================\n");
        return;
    }
}

static void btmw_rpc_test_ble_scanner_batch_scan_list_add_show_device(UINT8 scanner_id,
                        CHAR* data, BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA *pt_batchscan_report)
{
    UINT32 i  = 0;
    i = scanner_id;
    BLE_SCANNER_ADDR_LIST *addr_item = NULL;
    if (scanner_id != s_app_device_addr[i].scanner_id)
    {
        btmw_rpc_test_ble_scanner_batch_scan_show_device(scanner_id, data, pt_batchscan_report);
        return;
    }
    else
    {
        dl_list_for_each(addr_item, &s_app_device_addr[i].addr_list, BLE_SCANNER_ADDR_LIST, node)
        {
            if (memcmp(&addr_item->bt_addr, data, MAX_BDADDR_LEN - 1) == 0)
            {
                return;
            }
            else
            {
                continue;
            }
        }
    }
    btmw_rpc_test_ble_scanner_batch_scan_show_device(scanner_id, data, pt_batchscan_report);
}

static void btmw_rpc_test_ble_scanner_register_callback(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    if (NULL == cb_param)
    {
        BTMW_RPC_TEST_Loge("cb_param is NULL\n");
        return;
    }
    BTMW_RPC_TEST_Logi("scanner_id = %d\n", cb_param->scanner_id);
}

static void btmw_rpc_test_ble_scanner_scan_result_callback(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    if (NULL == cb_param)
    {
        BTMW_RPC_TEST_Loge("cb_param is NULL\n");
        return;
    }
    btmw_rpc_test_ble_scanner_list_add_show_device(cb_param);

}

static void btmw_rpc_test_ble_scanner_batchscan_report_callback(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    int n = 0;
    if (NULL == cb_param)
    {
        BTMW_RPC_TEST_Loge("cb_param is NULL\n");
        return;
    }
    BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA *pt_batchscan_report =
                &cb_param->data.batchscan_report_data;
    if (pt_batchscan_report->data_len == 0)
    {
        BTMW_RPC_TEST_Loge("No device discovered\n");
        return;
    }
    UINT8 *data = pt_batchscan_report->data;
    char *temp = (char *)malloc(MAX_BDADDR_LEN);
    if (temp == NULL)
    {
        return;
    }
    n = snprintf(temp, MAX_BDADDR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X",
             data[5], data[4], data[3], data[2], data[1], data[0]);
    if (n < 0)
    {
        free(temp);
        return;
    }
    temp[MAX_BDADDR_LEN - 1] = '\0';
    btmw_rpc_test_ble_scanner_batch_scan_list_add_show_device(cb_param->scanner_id,
        temp, pt_batchscan_report);
}

static void btmw_rpc_test_ble_scanner_track_adv_callback(BT_BLE_SCANNER_TRACK_ADV_EVT_DATA *pt_track_adv)
{
    if (NULL == pt_track_adv)
    {
        BTMW_RPC_TEST_Loge("pt_track_adv is NULL\n");
        return;
    }
    BTMW_RPC_TEST_Logi("pt_track_adv_btaddr: %s\n", pt_track_adv->bd_addr);
    btmw_rpc_test_gatt_adv_data_t adv_data;
    btmw_rpc_test_ble_scanner_decode_adv_data(pt_track_adv->p_adv_pkt_data, &adv_data);
}

//Callback
static void btmw_rpc_test_ble_scanner_callback(BT_BLE_SCANNER_CALLBACK_PARAM *callback_param, VOID *pv_tag)
{
    if (NULL == callback_param)
    {
        BTMW_RPC_TEST_Loge("callback_param is NULL\n");
        return;
    }
    switch (callback_param->event) {
        case BT_BLE_SCANNER_EVT_REGISTER:
            btmw_rpc_test_ble_scanner_register_callback(callback_param);
            break;
        case BT_BLE_SCANNER_EVT_SCAN_RESULT:
            btmw_rpc_test_ble_scanner_scan_result_callback(callback_param);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_REPORT:
            btmw_rpc_test_ble_scanner_batchscan_report_callback(callback_param);
            break;
        case BT_BLE_SCANNER_EVT_TRACK_ADV:
            btmw_rpc_test_ble_scanner_track_adv_callback(&callback_param->data.track_adv_data);
            break;
        default:
            BTMW_RPC_TEST_Logd("ble scanner callback event = %d", callback_param->event);
            break;
    }
    return;
}


//Basic ble scanner function
static int btmw_rpc_test_ble_scanner_register(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return -1;
    }

    char pv_tag[2] = {0};
    mtkrpcapi_BtAppBleScannerCbk *func = (mtkrpcapi_BtAppBleScannerCbk *)btmw_rpc_test_ble_scanner_callback;
    return a_mtkapi_bt_ble_scanner_register(func, (void *)pv_tag);
}


static int btmw_rpc_test_ble_scanner_start_simple_regular_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;

    if (argc != 2)
    {
         BTMW_RPC_TEST_Logd("Usage :\n");
         BTMW_RPC_TEST_Logd("BLE_SCANNER: simple_regular_scan <scanner_id> <scan_mode_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = REGULAR_SCAN;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan_param
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param =
                      &start_simple_scan_param->scan_param.regular_scan_param;
    regular_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);
    rpc_free_param(start_simple_scan_param);

    return ret;
}


static int btmw_rpc_test_ble_scanner_start_simple_batch_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 scan_result_type_index = 0;

    if (argc != 3)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_batch_scan <scanner_id> <scan_mode_index> <scan_result_type_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    scan_result_type_index = atoi(argv[2]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_result_type_index < 0 || scan_result_type_index > (sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_result_type_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = BATCH_SCAN;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan_param
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM *batch_scan_param =
                      &start_simple_scan_param->scan_param.batch_scan_param;
    batch_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];
    batch_scan_param->scan_result_type = ble_scanner_scan_result_type[scan_result_type_index];

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);
    rpc_free_param(start_simple_scan_param);

    return ret;
}


static int btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 dely_mode_index = 0;
    INT16 num_of_match_index = 0;
    INT16 filt_match_mode_index = 0;
    INT16 scan_filt_data_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 6)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_regular_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                          " <num_of_match_index> <filt_match_mode_index> <scan_filt_data_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    dely_mode_index = atoi(argv[2]);
    num_of_match_index = atoi(argv[3]);
    filt_match_mode_index = atoi(argv[4]);
    scan_filt_data_index = atoi(argv[5]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (dely_mode_index < 0 || dely_mode_index > (sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("dely_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (num_of_match_index < 0 || num_of_match_index > (sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("num_of_matches_index should be 0 ~ %d\n", sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (filt_match_mode_index < 0 || filt_match_mode_index > (sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("filt_match_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_filt_data_index < 0 || scan_filt_data_index > (sizeof(ble_scanner_filt_data)/sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_filt_data_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_data)/sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = REGULAR_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.regular_scan_param;
    regular_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];

    //scan filt param
    BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM *scan_filt_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_param;
    scan_filt_param->dely_mode = ble_scanner_dely_mode[dely_mode_index];
    scan_filt_param->num_of_matches = ble_scanner_num_of_match[num_of_match_index];
    scan_filt_param->filt_match_mode= ble_scanner_filt_match_mode[filt_match_mode_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_data;
    scan_filt_data[0] = ble_scanner_filt_data[scan_filt_data_index];
    scan_filt_num = 1;

    //scan filt num
    start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_num = scan_filt_num;

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);
    rpc_free_param(start_simple_scan_param);

    return ret;
}


static int btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_addr_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 dely_mode_index = 0;
    INT16 num_of_match_index = 0;
    INT16 filt_match_mode_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 6)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_regular_addr_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                          " <num_of_match_index> <filt_match_mode_index> <addr>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    dely_mode_index = atoi(argv[2]);
    num_of_match_index = atoi(argv[3]);
    filt_match_mode_index = atoi(argv[4]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (dely_mode_index < 0 || dely_mode_index > (sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("dely_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (num_of_match_index < 0 || num_of_match_index > (sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("num_of_matches_index should be 0 ~ %d\n", sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (filt_match_mode_index < 0 || filt_match_mode_index > (sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("filt_match_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = REGULAR_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.regular_scan_param;
    regular_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];

    //scan filt param
    BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM *scan_filt_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_param;
    scan_filt_param->dely_mode = ble_scanner_dely_mode[dely_mode_index];
    scan_filt_param->num_of_matches = ble_scanner_num_of_match[num_of_match_index];
    scan_filt_param->filt_match_mode= ble_scanner_filt_match_mode[filt_match_mode_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_data;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data_addr;
    memset(&scan_filt_data_addr, 0, sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA));
    scan_filt_data_addr.addr_type = SCAN_FILT_ALL_TYPE;
    if (strlen(argv[5]) == (MAX_BDADDR_LEN - 1))
    {
        strncpy(scan_filt_data_addr.bt_addr, argv[5], MAX_BDADDR_LEN - 1);
    }
    else
    {
        BTMW_RPC_TEST_Logd("invalid address!");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    scan_filt_data[0] = scan_filt_data_addr;
    scan_filt_num = 1;

    //scan filt num
    start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_num = scan_filt_num;

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);

    rpc_free_param(start_simple_scan_param);

    return ret;
}



static int btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_multi_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 dely_mode_index = 0;
    INT16 num_of_match_index = 0;
    INT16 filt_match_mode_index = 0;
    INT16 scan_filt_data_multi_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 6)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_regular_multi_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                          " <num_of_match_index> <filt_match_mode_index> <scan_filt_data_multi_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    dely_mode_index = atoi(argv[2]);
    num_of_match_index = atoi(argv[3]);
    filt_match_mode_index = atoi(argv[4]);
    scan_filt_data_multi_index = atoi(argv[5]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (dely_mode_index < 0 || dely_mode_index > (sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("dely_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_dely_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (num_of_match_index < 0 || num_of_match_index > (sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("num_of_matches_index should be 0 ~ %d\n", sizeof(ble_scanner_num_of_match)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (filt_match_mode_index < 0 || filt_match_mode_index > (sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("filt_match_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_match_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_filt_data_multi_index < 0 || scan_filt_data_multi_index > (sizeof(ble_scanner_filt_data_multi)/sizeof(INT32) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_filt_data_multi_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_data_multi)/sizeof(INT32) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = REGULAR_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_REGULAR_SIMPLE_SCAN_PARAM *regular_scan_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.regular_scan_param;
    regular_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];

    //scan filt param
    BT_BLE_SCANNER_SIMPLE_SCAN_FILT_PARAM *scan_filt_param =
                      &start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_param;
    scan_filt_param->dely_mode = ble_scanner_dely_mode[dely_mode_index];
    scan_filt_param->num_of_matches = ble_scanner_num_of_match[num_of_match_index];
    scan_filt_param->filt_match_mode= ble_scanner_filt_match_mode[filt_match_mode_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_data;

    int scan_filt_data_multi = ble_scanner_filt_data_multi[scan_filt_data_multi_index];
    for (int i = 0; i < 30; i++ )
    {
        if (((scan_filt_data_multi >> i) & 0x01) == 1)
        {
            scan_filt_data[scan_filt_num] = ble_scanner_filt_data[i];
            scan_filt_num++;
            if (scan_filt_num >= BT_BLE_SCANNER_MAX_SCAN_FILT_NUM)
            {
                break;
            }
        }
    }

    //scan filt num
    start_simple_scan_param->scan_param.regular_scan_filt_param.scan_filt_num = scan_filt_num;
    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);

    rpc_free_param(start_simple_scan_param);

    return ret;
}


static int btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 scan_result_type_index = 0;
    INT16 scan_filt_data_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 4)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_batch_filt_scan <scanner_id> <scan_mode_index> <scan_result_type_index> <scan_filt_data_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    scan_result_type_index = atoi(argv[2]);
    scan_filt_data_index = atoi(argv[3]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_result_type_index < 0 || scan_result_type_index > (sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_result_type_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_filt_data_index < 0 || scan_filt_data_index > (sizeof(ble_scanner_filt_data)/sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_filt_data_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_data)/sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = BATCH_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM *batch_scan_param =
                      &start_simple_scan_param->scan_param.batch_scan_filt_param.batch_scan_param;
    batch_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];
    batch_scan_param->scan_result_type = ble_scanner_scan_result_type[scan_result_type_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_data;

    scan_filt_data[0] = ble_scanner_filt_data[scan_filt_data_index];
    scan_filt_num = 1;

    //scan filt num
    start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_num = scan_filt_num;

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);

    rpc_free_param(start_simple_scan_param);
    return ret;
}


static int btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_addr_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 scan_result_type_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 4)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_batch_addr_filt_scan <scanner_id> <scan_mode_index> <scan_result_type_index> <addr>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    scan_result_type_index = atoi(argv[2]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_result_type_index < 0 || scan_result_type_index > (sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_result_type_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = BATCH_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM *batch_scan_param =
                      &start_simple_scan_param->scan_param.batch_scan_filt_param.batch_scan_param;
    batch_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];
    batch_scan_param->scan_result_type = ble_scanner_scan_result_type[scan_result_type_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_data;
    BT_BLE_SCANNER_SCAN_FILT_DATA scan_filt_data_addr;
    memset(&scan_filt_data_addr, 0, sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA));
    scan_filt_data_addr.addr_type = SCAN_FILT_ALL_TYPE;
    if (strlen(argv[3]) == (MAX_BDADDR_LEN - 1))
    {
        strncpy(scan_filt_data_addr.bt_addr, argv[3], MAX_BDADDR_LEN - 1);
    }
    else
    {
        BTMW_RPC_TEST_Logd("invalid address!");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    scan_filt_data[0] = scan_filt_data_addr;
    scan_filt_num = 1;

    //scan filt num
    start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_num = scan_filt_num;

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);
    rpc_free_param(start_simple_scan_param);

    return ret;
}



static int btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_multi_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 scan_mode_index = 0;
    INT16 scan_result_type_index = 0;
    INT16 scan_filt_data_multi_index = 0;
    INT16 scan_filt_num = 0;

    if (argc != 4)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: simple_batch_multi_filt_scan <scanner_id> <scan_mode_index>"
                                                      " <scan_result_type_index> <scan_filt_data_multi_index>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *start_simple_scan_param =
           (BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));
    if (NULL == start_simple_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_simple_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SIMPLE_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_mode_index = atoi(argv[1]);
    scan_result_type_index = atoi(argv[2]);
    scan_filt_data_multi_index = atoi(argv[3]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_mode_index < 0 || scan_mode_index > (sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_mode_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_mode)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_result_type_index < 0 || scan_result_type_index > (sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_result_type_index should be 0 ~ %d\n", sizeof(ble_scanner_scan_result_type)/sizeof(INT8) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }
    if (scan_filt_data_multi_index < 0 || scan_filt_data_multi_index > (sizeof(ble_scanner_filt_data_multi)/sizeof(INT32) - 1))
    {
        BTMW_RPC_TEST_Logd("scan_filt_data_multi_index should be 0 ~ %d\n", sizeof(ble_scanner_filt_data_multi)/sizeof(INT32) - 1);
        rpc_free_param(start_simple_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_simple_scan_param->scan_type = BATCH_SCAN_WITH_FILT;

    //scanner_id
    start_simple_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_BATCH_SIMPLE_SCAN_PARAM *batch_scan_param =
                      &start_simple_scan_param->scan_param.batch_scan_filt_param.batch_scan_param;
    batch_scan_param->scan_mode = ble_scanner_scan_mode[scan_mode_index];
    batch_scan_param->scan_result_type = ble_scanner_scan_result_type[scan_result_type_index];

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_data;

    int scan_filt_data_multi = ble_scanner_filt_data_multi[scan_filt_data_multi_index];

    for (int i = 0; i < 30; i++ )
    {
        if (((scan_filt_data_multi >> i) & 0x01) == 1)
        {
            scan_filt_data[scan_filt_num] = ble_scanner_filt_data[i];
            scan_filt_num++;
            if (scan_filt_num >= BT_BLE_SCANNER_MAX_SCAN_FILT_NUM)
            {
                break;
            }
        }
    }

    //scan filt num
    start_simple_scan_param->scan_param.batch_scan_filt_param.scan_filt_num = scan_filt_num;

    ret = a_mtkapi_bt_ble_scanner_start_simple_scan(start_simple_scan_param);

    rpc_free_param(start_simple_scan_param);

    return ret;
}

static int btmw_rpc_test_ble_scanner_start_regular_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT32 scan_windows = 0;
    INT32 scan_interval = 0;

    if (argc != 3)
    {
        BTMW_RPC_TEST_Loge("Usage :\n");
        BTMW_RPC_TEST_Loge("BLE_SCANNER: regular_scan <scanner_id> <scan_windows> <scan_interval>\n");
        return -1;
    }
    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param =
           (BT_BLE_SCANNER_START_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));
    if (NULL == start_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    scan_windows = atoi(argv[1]);
    scan_interval = atoi(argv[2]);

    if (scanner_id < 0 || scan_windows < 0 || scan_interval < 0)
    {
        BTMW_RPC_TEST_Loge("scan param invaild\n");
        rpc_free_param(start_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_scan_param->scan_type = REGULAR_SCAN;

    //scanner_id
    start_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_REGULAR_SCAN_PARAM *regular_scan_param =
                      &start_scan_param->scan_param.regular_scan_param;
    regular_scan_param->scan_windows = scan_windows;
    regular_scan_param->scan_interval = scan_interval;
    BTMW_RPC_TEST_Logd("scan_windows: %d scan_interval: %d", scan_windows, scan_interval);
    ret = a_mtkapi_bt_ble_scanner_start_scan(start_scan_param);

    rpc_free_param(start_scan_param);

    return ret;
}

static int btmw_rpc_test_ble_scanner_start_regular_scan_with_filt(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 delay_mode = 0;
    INT16 list_logic_type = 0;
    INT16 filt_logic_type = 0;
    INT16 rssi_high_thres = 0;
    INT16 rssi_low_thres = 0;
    INT16 lost_timeout = 0;

    if (argc != 7)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: regular_filt_scan <scanner_id> <delay_mode> <list_logic_type>"
                                          " <filt_logic_type> <rssi_high_thres> <rssi_low_thres> <lost_timeout>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param =
           (BT_BLE_SCANNER_START_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));
    if (NULL == start_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    delay_mode = atoi(argv[1]);
    list_logic_type = atoi(argv[2]);
    filt_logic_type = atoi(argv[3]);
    rssi_high_thres = atoi(argv[4]);
    rssi_low_thres = atoi(argv[5]);
    lost_timeout = atoi(argv[6]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (delay_mode < 0 || delay_mode > 1)
    {
        BTMW_RPC_TEST_Logd("delay_mode should be 0 or 1\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (list_logic_type < 0 || list_logic_type > 0xFFFF)
    {
        BTMW_RPC_TEST_Logd("list_logic_type should be 0 ~ 65535\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (filt_logic_type < 0 || filt_logic_type > 1)
    {
        BTMW_RPC_TEST_Logd("filt_logic_type should be 0 or 1\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (rssi_high_thres < 0 || rssi_high_thres > 0xFF)
    {
        BTMW_RPC_TEST_Logd("rssi_high_thres should be 0 ~ 255\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (rssi_low_thres < 0 || rssi_low_thres > 0xFF)
    {
        BTMW_RPC_TEST_Logd("rssi_low_thres should be 0 ~ 255\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (lost_timeout < 0 || lost_timeout > 0xFFFF)
    {
        BTMW_RPC_TEST_Logd("rssi_low_thres should be 0 ~ 65535\n");
        rpc_free_param(start_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_scan_param->scan_type = REGULAR_SCAN_WITH_FILT;

    //scanner_id
    start_scan_param->scanner_id = scanner_id;

    //scan param
    BT_BLE_SCANNER_REGULAR_SCAN_PARAM *regular_scan_param =
                      &start_scan_param->scan_param.regular_scan_filt_param.regular_scan_param;
    regular_scan_param->scan_windows = 6553;
    regular_scan_param->scan_interval = 6553;

    //scan filt data
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data =
                      start_scan_param->scan_param.regular_scan_filt_param.scan_filt_data;
    scan_filt_data[0] = ble_scanner_filt_data[5];

    //scan filt num
    start_scan_param->scan_param.regular_scan_filt_param.scan_filt_num = 1;

    //scan filt param
    BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filt_param =
                      start_scan_param->scan_param.regular_scan_filt_param.scan_filt_param;
    scan_filt_param->feat_seln = get_feature_selection(&(scan_filt_data)[0]);
    scan_filt_param->list_logic_type = list_logic_type;
    scan_filt_param->filt_logic_type = filt_logic_type;
    scan_filt_param->rssi_high_thres = rssi_high_thres;
    scan_filt_param->rssi_low_thres = rssi_low_thres;
    scan_filt_param->dely_mode = delay_mode;
    scan_filt_param->found_timeout = 1500;
    scan_filt_param->lost_timeout = lost_timeout;
    scan_filt_param->found_timeout_cnt = 1;
    scan_filt_param->num_of_tracking_entries = 5;

    ret = a_mtkapi_bt_ble_scanner_start_scan(start_scan_param);

    rpc_free_param(start_scan_param);

    return ret;
}


static int btmw_rpc_test_ble_scanner_start_batch_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Loge("[BLE_SCANNER] %s()\n", __func__);

    INT16 scanner_id = 0;
    INT16 batch_scan_notify_threshold = 0;
    INT16 batch_scan_discard_rule = 0;
    INT16 batch_scan_own_address_type = 0;

    if (argc != 4)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: batch_scan <scanner_id> <batch_scan_notify_threshold> <batch_scan_discard_rule> <batch_scan_own_address_type>\n");
         return -1;
    }

    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param =
           (BT_BLE_SCANNER_START_SCAN_PARAM *)malloc(sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));
    if (NULL == start_scan_param)
    {
        BTMW_RPC_TEST_Loge("malloc fail!\n");
        return -1;
    }
    memset(start_scan_param, 0, sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));

    scanner_id = atoi(argv[0]);
    batch_scan_notify_threshold = atoi(argv[1]);
    batch_scan_discard_rule = atoi(argv[2]);
    batch_scan_own_address_type = atoi(argv[3]);

    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (batch_scan_notify_threshold < 0 || batch_scan_notify_threshold > 100)
    {
        BTMW_RPC_TEST_Logd("delay_mode should be 0 ~ 100\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (batch_scan_discard_rule < 0 || batch_scan_discard_rule > 1)
    {
        BTMW_RPC_TEST_Logd("list_logic_type should be 0 or 1\n");
        rpc_free_param(start_scan_param);
        return -1;
    }
    if (batch_scan_own_address_type < 0 || batch_scan_own_address_type > 1)
    {
        BTMW_RPC_TEST_Logd("filt_logic_type should be 0 or 1\n");
        rpc_free_param(start_scan_param);
        return -1;
    }

    INT32 ret = 0;
    //scan type
    start_scan_param->scan_type = BATCH_SCAN;

    //scanner_id
    start_scan_param->scanner_id = scanner_id;

    //scan_param
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *batch_scan_param =
                      &start_scan_param->scan_param.batch_scan_param;
    batch_scan_param->batch_scan_full_max = 100;
    batch_scan_param->batch_scan_trunc_max = 0;
    batch_scan_param->batch_scan_notify_threshold = batch_scan_notify_threshold;
    batch_scan_param->batch_scan_mode = BATCH_SCAN_READ_REPORT_MODE_FULL;
    batch_scan_param->batch_scan_windows = 2400;
    batch_scan_param->batch_scan_interval = 8000;
    batch_scan_param->own_address_type = batch_scan_own_address_type;
    batch_scan_param->batch_scan_discard_rule = batch_scan_discard_rule;
    batch_scan_param->batch_scan_read_report_mode = BATCH_SCAN_READ_REPORT_MODE_FULL;

    ret = a_mtkapi_bt_ble_scanner_start_scan(start_scan_param);
    rpc_free_param(start_scan_param);

    return ret;
}

static int btmw_rpc_test_ble_scanner_stop_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);

    UINT8 scanner_id = 0, i = 0;

    if (argc != 1)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: stop_scan <scanner_id>\n");
         return -1;
    }
    scanner_id = atoi(argv[0]);
    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        return -1;
    }

    BT_BLE_SCANNER_STOP_SCAN_PARAM stop_scan_param;
    memset(&stop_scan_param, 0, sizeof(BT_BLE_SCANNER_STOP_SCAN_PARAM));
    stop_scan_param.scanner_id = scanner_id;
    i = scanner_id;
    list_remove_by_scanner_id(&s_app_device_addr[i], scanner_id);
    return a_mtkapi_bt_ble_scanner_stop_scan(&stop_scan_param);
}


static int btmw_rpc_test_ble_scanner_unregister(int argc, char **argv)
{
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] %s()\n", __func__);
    if (argc != 1)
    {
         BTMW_RPC_TEST_Loge("Usage :\n");
         BTMW_RPC_TEST_Loge("BLE_SCANNER: unregister <scanner_id>\n");
         return -1;
    }
    INT8 scanner_id = atoi(argv[0]);
    if (scanner_id < 0)
    {
        BTMW_RPC_TEST_Loge("scanner_id should > 0\n");
        return -1;
    }

    BT_BLE_SCANNER_UNREG_PARAM scanner_unreg;
    memset(&scanner_unreg, 0, sizeof(BT_BLE_SCANNER_UNREG_PARAM));
    scanner_unreg.scanner_id = scanner_id;

    return a_mtkapi_bt_ble_scanner_unregister(&scanner_unreg);
}


static BTMW_RPC_TEST_CLI btmw_rpc_test_ble_scanner_cli_commands[] =
{
    {(const char *)"register",                            btmw_rpc_test_ble_scanner_register,                                  (const char *)" = register"},
    {(const char *)"simple_regular_scan",                 btmw_rpc_test_ble_scanner_start_simple_regular_scan,                 (const char *)" = simple_regular_scan <scanner_id> <scan_mode_index>"},
    {(const char *)"simple_batch_scan",                   btmw_rpc_test_ble_scanner_start_simple_batch_scan,                   (const char *)" = simple_batch_scan <scanner_id> <scan_mode_index> <scan_result_type_index>"},
    {(const char *)"simple_regular_filt_scan",            btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_filt,       (const char *)" = simple_regular_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                                                                                                                                 " <num_of_match_index> <filt_match_mode_index> <scan_filt_data_index>"},
    {(const char *)"simple_regular_addr_filt_scan",       btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_addr_filt,  (const char *)" = simple_regular_addr_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                                                                                                                                 " <num_of_match_index> <filt_match_mode_index> <addr>"},
    {(const char *)"simple_regular_multi_filt_scan",      btmw_rpc_test_ble_scanner_start_simple_regular_scan_with_multi_filt, (const char *)" = simple_regular_multi_filt_scan <scanner_id> <scan_mode_index> <dely_mode_index>"
                                                                                                                                                 " <num_of_match_index> <filt_match_mode_index> <scan_filt_data_multi_index>"},
    {(const char *)"simple_batch_filt_scan",              btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_filt,         (const char *)" = simple_batch_filt_scan <scanner_id> <scan_mode_index> <scan_result_type_index> <scan_filt_data_index>"},
    {(const char *)"simple_batch_addr_filt_scan",         btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_addr_filt,    (const char *)" = simple_batch_addr_filt_scan <scanner_id> <scan_mode_index> <scan_result_type_index> <addr>"},
    {(const char *)"simple_batch_multi_filt_scan",        btmw_rpc_test_ble_scanner_start_simple_batch_scan_with_multi_filt,   (const char *)" = simple_batch_multi_filt_scan <scanner_id> <scan_mode_index> <scan_result_type_index> <scan_filt_data_multi_index>"},
    {(const char *)"regular_scan",                        btmw_rpc_test_ble_scanner_start_regular_scan,                        (const char *)" = regular_scan <scanner_id> <scan_windows> <scan_interval>"},
    {(const char *)"regular_filt_scan",                   btmw_rpc_test_ble_scanner_start_regular_scan_with_filt,              (const char *)" = regular_filt_scan <scanner_id> <delay_mode> <list_logic_type>"
                                                                                                                                                 " <filt_logic_type> <rssi_high_thres> <rssi_low_thres> <lost_timeout>"},
    {(const char *)"batch_scan",                          btmw_rpc_test_ble_scanner_start_batch_scan,                          (const char *)" = batch_scan <scanner_id> <batch_scan_notify_threshold> <batch_scan_discard_rule> <batch_scan_own_address_type>"},
    {(const char *)"stop_scan",                           btmw_rpc_test_ble_scanner_stop_scan,                                 (const char *)" = stop_scan <scanner_id>"},
    {(const char *)"unregister",                          btmw_rpc_test_ble_scanner_unregister,                                (const char *)" = unregister <scanner_id>"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_ble_scanner_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_ble_scanner_cli_commands;

    BTMW_RPC_TEST_Logd("[BLE_SCANNER] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logd("[BLE_SCANNER] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_BLE_SCANNER, btmw_rpc_test_ble_scanner_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}


int btmw_rpc_test_ble_scanner_init()
{
    UINT32 i = 0;
    BTMW_RPC_TEST_Logi("[BLE_SCANNER] The process pid is %d \n", getpid());
    BTMW_RPC_TEST_Logi("[BLE_SCANNER] btmw_rpc_test_ble_scanner_init");
    int ret = 0;
    BTMW_RPC_TEST_MOD ble_scanner_mod = {0};

    // Register command to CLI
    ble_scanner_mod.mod_id = BTMW_RPC_TEST_MOD_BLE_SCANNER;
    strncpy(ble_scanner_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_BLE_SCANNER, sizeof(ble_scanner_mod.cmd_key));
    ble_scanner_mod.cmd_handler = btmw_rpc_test_ble_scanner_cmd_handler;
    ble_scanner_mod.cmd_tbl = btmw_rpc_test_ble_scanner_cli_commands;

    ret = btmw_rpc_test_register_mod(&ble_scanner_mod);
    BTMW_RPC_TEST_Logi("[BLE_SCANNER] btmw_rpc_test_register_mod() returns: %d\n", ret);
    BTMW_RPC_TEST_Logd("[BLE_SCANNER] g_cli_pts_mode: %d\n", g_cli_pts_mode);
    //if (!g_cli_pts_mode)

    //init scan_result addr list
    for (i = 0; i < BT_BLE_SCANNER_MAX_REG_NUM; i++)
    {
        memset(&s_app_device_addr[i], 0, sizeof(BLE_SCANNER_DEVICE_ADDR));
        dl_list_init(&s_app_device_addr[i].addr_list);
    }
    return ret;
}

int btmw_rpc_test_ble_scanner_deinit()
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}
