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


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <string.h>
#include "bluetooth.h"
#include "ble_scanner.h"
#include "bt_mw_common.h"
#include "linuxbt_ble_scanner_if.h"
#include "linuxbt_common.h"
#include "bt_mw_ble_scanner.h"
#include <vector>
#include <base/bind.h>
#include <base/callback.h>
#include <map>

using bluetooth::Uuid;
using base::Bind;
using std::map;
using std::pair;
using std::vector;

#define CHECK_BLE_SCANNER_INTERFACE() if (NULL == linuxbt_ble_scanner_interface)\
{\
    BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER, "[ble scanner] linuxbt_ble_scanner_interface not init.");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

#define CHECK_BLE_SCANNER_BT_ADDR(bt_addr) if (NULL == bt_addr)\
{\
    BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER,"[ble scanner] null pointer");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

typedef struct
{
     unsigned char type;
     unsigned char len;
     UINT8 *value;
} adv_data_item_t;

typedef struct
{
    adv_data_item_t data[10];
} adv_data_t;

static BleScannerInterface *linuxbt_ble_scanner_interface = NULL;

BT_BLE_SCANNER_APP_FILTER_DATA app_filt_data[BT_BLE_SCANNER_MAX_REG_NUM];
UINT32 filter_num[BT_BLE_SCANNER_MAX_REG_NUM] = {0};

void linuxbt_uuid_to_array(const Uuid& app_uuid, char* uuid)
{
    uint8_t p_uuid[16] = {0};
    memcpy(p_uuid, app_uuid.To128BitBE().data(), Uuid::kNumBytes128);
    int i = 0, j = 0;
    char *ptr;
    ptr = uuid;
    for (i = 0; i <= 15; i++)
    {
        j = sprintf(ptr, "%02X", p_uuid[i]);
        if (j < 0)
        {
            return;
        }
        ptr+=2;
        if (i == 3 || i == 5 || i == 7 || i == 9)
        {
            *ptr = '-';
            ptr++;
        }
    }
    *ptr = '\0';
}

void uuid_little_to_big(UINT8 *src, UINT8 *dst, UINT8 len)
{
    int i = 0, j = 0;
    if (len > 0)
    {
        for (i = len-1 ; i >= 0 ; i--)
        {
            dst[j] = src[i];
            j++;
        }
    }
    else
    {
        return;
    }
}

static int uuid_str_to_byte(CHAR *src, UINT32 *dst)
{
    int n = 0;
    while(*src)
    {
        if ('-' == *src)
        {
            src++;
            continue;
        }
        n = sscanf(src, "%02x", dst);
        if (n == EOF)
        {
            return -1;
        }
        src += 2;
        dst++;
    }
    return 0;
}

BOOL masked_equals(UINT8 *data, UINT32 *uuid, UINT32 *uuid_mask, UINT8 len)
{
    for (int i = 0; i < len; i++) {
        if (!(((UINT32)(data[i]) & (uuid_mask[i])) == ((uuid[i]) & (uuid_mask[i]))))
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL manu_srvc_data_masked_equals(UINT8 *parse_data, UINT8 *data, UINT8 *data_mask, UINT16 data_len)
{
    for (int i = 0; i < data_len; i++) {
        if (!(((parse_data[i]) & (data_mask[i])) == ((data[i]) & (data_mask[i]))))
        {
            return FALSE;
        }
    }
    return TRUE;
}


VOID free_data (UINT8 *parse_data, UINT8 *data_temp)
{
    if (parse_data != NULL)
    {
        free(parse_data);
    }
    if (data_temp != NULL)
    {
        free(data_temp);
    }
}

void decode_adv_data (UINT8* adv_data, adv_data_t *parse_data)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "decode_adv_date");
    UINT8* ptr = adv_data;
    UINT8 count = 0;
    UINT8* value = NULL;
    while (count < 10)
    {
        value = NULL;
        UINT8 length = *ptr;
        if (length == 0) break;
        UINT8 type = *(ptr + 1);
        UINT8 value_len = length - 1;
        value = (UINT8*)malloc(value_len);
        if (NULL == value)
        {
            BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER, "[GATT] malloc fail!");
            return;
        }
        memset(value, 0, value_len);
        memcpy(value, ptr+2, value_len);

        parse_data->data[count].type = type;
        parse_data->data[count].len = value_len;
        parse_data->data[count].value = value;

        ptr = ptr + length + 1;
        count++;
    }
}

BOOL filter_uuid(UINT8 *parse_value, CHAR *uuid, CHAR *uuid_mask, UINT8 *value_temp,
                    UINT32 *uuid_temp, UINT32 *uuid_mask_temp, UINT8 uuid_len, UINT8 len)
{
    UINT8 str_len = 0;
    str_len = strlen(uuid);
    if (str_len == uuid_len)
    {
        uuid_str_to_byte(uuid, uuid_temp);
        uuid_str_to_byte(uuid_mask, uuid_mask_temp);
        uuid_little_to_big(parse_value, value_temp, len);
        if (!masked_equals(value_temp, uuid_temp, uuid_mask_temp, len))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

BOOL filter_same_uuid(UINT8 *parse_value, UINT8 parse_len, int tag,
                                CHAR *uuid, CHAR *uuid_mask)
{
    int ret = 0;
    if (tag == 16)
    {
        if (parse_len > 2)
        {
            UINT8 len = 0;
            UINT32 srvc_uuid_temp[2] = {0};
            UINT32 srvc_uuid_mask_temp[2] = {0};
            UINT8 value_temp[2] = {0};
            UINT8 *value = NULL;
            value = parse_value;
            while (len <= parse_len)
            {
                ret = filter_uuid(value, uuid, uuid_mask, value_temp,
                            srvc_uuid_temp, srvc_uuid_mask_temp, 4, 2);
                if (ret == 0)
                {
                    value = parse_value + 2;
                    len = len + 2;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 16bit uuid pass");
                    return ret;
                }
            }
            return ret;
        }
        else
        {
            UINT32 srvc_uuid_temp[2] = {0};
            UINT32 srvc_uuid_mask_temp[2] = {0};
            UINT8 value_temp[2] = {0};
            ret = filter_uuid(parse_value, uuid, uuid_mask, value_temp,
                            srvc_uuid_temp, srvc_uuid_mask_temp, 4, 2);
            if (ret == 0)
            {
                BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 16bit srvc_uuid fail");
            }
            return ret;
        }
    }
    else if (tag == 32)
    {
        if (parse_len > 4)
        {
            UINT8 len = 0;
            UINT32 srvc_uuid_temp[4] = {0};
            UINT32 srvc_uuid_mask_temp[4] = {0};
            UINT8 value_temp[4] = {0};
            UINT8 *value = NULL;
            value = parse_value;
            while (len <= parse_len)
            {
                ret = filter_uuid(value, uuid, uuid_mask, value_temp,
                             srvc_uuid_temp, srvc_uuid_mask_temp, 8, 4);
                if (ret == 0)
                {
                    value = parse_value + 4;
                    len = len + 4;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 32bit srvc_uuid pass");
                    return ret;
                }
            }
            return ret;
        }
        else
        {
            UINT32 srvc_uuid_temp[4] = {0};
            UINT32 srvc_uuid_mask_temp[4] = {0};
            UINT8 value_temp[4] = {0};
            ret = filter_uuid(parse_value, uuid, uuid_mask, value_temp,
                     srvc_uuid_temp, srvc_uuid_mask_temp, 8, 4);
            if (ret == 0)
            {
                BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 32bit srvc_uuid fail");
            }
            return ret;
        }
    }
    else if (tag == 128)
    {
        if (parse_len > 16)
        {
            UINT8 len = 0;
            UINT32 srvc_uuid_temp[16] = {0};
            UINT32 srvc_uuid_mask_temp[16] = {0};
            UINT8 value_temp[16] = {0};
            UINT8 *value = NULL;
            value = parse_value;
            while (len <= parse_len)
            {
                ret = filter_uuid(value, uuid, uuid_mask, value_temp,
                     srvc_uuid_temp, srvc_uuid_mask_temp, 36, 16);
                if (ret == 0)
                {
                    value = parse_value + 16;
                    len = len + 16;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 32bit srvc_uuid pass");
                    return ret;
                }
            }
            return ret;
        }
        else
        {
            UINT32 srvc_uuid_temp[16] = {0};
            UINT32 srvc_uuid_mask_temp[16] = {0};
            UINT8 value_temp[16] = {0};
            ret = filter_uuid(parse_value, uuid, uuid_mask, value_temp,
                     srvc_uuid_temp, srvc_uuid_mask_temp, 36, 16);
            if (ret == 0)
            {
                BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "filter 128bit srvc_uuid fail");
            }
            return ret;
        }
    }
    return ret;
}

BOOL filter_manu_srvc_data(UINT8 *parse_value, BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data,
                                 UINT8 parse_data_type, UINT8 parse_len, UINT8 len)
{
    UINT8 *data_temp = NULL;
    data_temp = (UINT8 *)malloc(parse_len - len + 1);
    if (data_temp == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER, "data_temp malloc fail!");
        return FALSE;
    }
    data_temp[parse_len - len] = '\0';
    memset(data_temp, 0, parse_len - len);
    memcpy(data_temp, parse_value + len , parse_len - len);

    //filter manu_data
    if (parse_data_type == DATA_TYPE_MANUFACTURER_SPECIFIC_DATA)
    {
        if (filter_data->manu_data_len > parse_len - len)
        {
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "data_len mismatch");
            free_data(parse_value, data_temp);
            return FALSE;
        }
        if (memcmp(&filter_data->company, parse_value, len) != 0)
        {
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "company mismatch");
            free_data(parse_value, data_temp);
            return FALSE;
        }
        else if (!manu_srvc_data_masked_equals(data_temp, filter_data->manu_data,
                                                  filter_data->manu_data_mask,
                                                  filter_data->manu_data_len))
        {
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "manu_data_mismatch");
            free_data(parse_value, data_temp);
            return FALSE;
        }
    }
    //filter serv_data
    if(parse_data_type == DATA_TYPE_SERVICE_DATA_16_BIT ||
       parse_data_type == DATA_TYPE_SERVICE_DATA_32_BIT ||
       parse_data_type == DATA_TYPE_SERVICE_DATA_128_BIT)
    {
        if (filter_data->srvc_data_len > parse_len)
        {
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER,"data_len_mismatch");
            free_data(parse_value, data_temp);
            return FALSE;
        }
        else if (!manu_srvc_data_masked_equals(parse_value, filter_data->srvc_data,
                           filter_data->srvc_data_mask, filter_data->srvc_data_len))
        {
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "srvc_data_mismatch");
            free_data(parse_value, data_temp);
            return FALSE;
        }
    }
    free_data(parse_value, data_temp);
    return TRUE;
}

BOOL match_filters (adv_data_t *parse_data, char *bd_addr, BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "match_filters");
    UINT8 i = 0;
    UINT8 str_len = 0;
    BOOL ret = 0;

    // address match
    if (0 != strlen(filter_data->bt_addr))
    {
        if (memcmp(filter_data->bt_addr, bd_addr, MAX_BDADDR_LEN) != 0)
        {
            ret = 0;
            free_data(parse_data->data[i].value, NULL);
            BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "Address match filter fail");
            return ret;
        }
        ret = 1;
        free_data(parse_data->data[i].value, NULL);
        BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, " Address match filter pass");
    }
    // srvc_uuid match
    if (0 != strlen(filter_data->srvc_uuid) && 0 != strlen(filter_data->srvc_uuid_mask))
    {
        int tag = 0;
        for (i = 0; i < BLE_SCANNER_MATCH_FILTER_DATA_TYPE; i++)
        {
            if (parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_16_BIT_PARTIAL ||
                parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_16_BIT_COMPLETE)
            {
                tag = 16;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_uuid, filter_data->srvc_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_32_BIT_PARTIAL ||
                    parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_32_BIT_COMPLETE)
            {
                tag = 32;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_uuid, filter_data->srvc_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_128_BIT_PARTIAL ||
                parse_data->data[i].type == DATA_TYPE_SERVICE_UUIDS_128_BIT_COMPLETE)
            {
                tag = 128;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_uuid, filter_data->srvc_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else
            {
                ret = 0;
                free_data(parse_data->data[i].value, NULL);
            }
        }
    }
    // srvc_sol_uuid match
    if (0 != strlen(filter_data->srvc_sol_uuid) && 0 != strlen(filter_data->srvc_sol_uuid_mask))
    {
        int tag = 0;
        for (i = 0; i < BLE_SCANNER_MATCH_FILTER_DATA_TYPE; i++)
        {
            if (parse_data->data[i].type == DATA_TYPE_SERVICE_SOLICITATION_UUIDS_16_BIT)
            {
                tag = 16;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_sol_uuid, filter_data->srvc_sol_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_SOLICITATION_UUIDS_32_BIT)
            {
                tag = 32;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_sol_uuid, filter_data->srvc_sol_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_SOLICITATION_UUIDS_128_BIT)
            {
                tag = 128;
                ret = filter_same_uuid(parse_data->data[i].value, parse_data->data[i].len,
                                   tag, filter_data->srvc_sol_uuid, filter_data->srvc_sol_uuid_mask);
                free_data(parse_data->data[i].value, NULL);
                break;
            }
            else
            {
                ret = 0;
                free_data(parse_data->data[i].value, NULL);
            }
        }
    }
    // local name match
    if (0 != strlen(filter_data->local_name))
    {
        for (i = 0; i < BLE_SCANNER_MATCH_FILTER_DATA_TYPE; i++)
        {
            if (parse_data->data[i].type == DATA_TYPE_LOCAL_NAME_SHORT ||
                parse_data->data[i].type == DATA_TYPE_LOCAL_NAME_COMPLETE)
            {
                str_len = strlen(filter_data->local_name);
                if (str_len == parse_data->data[i].len)
                {
                    if (memcmp(filter_data->local_name, parse_data->data[i].value, str_len) != 0)
                    {
                        BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter name fail");
                        free_data(parse_data->data[i].value, NULL);
                        return ret;
                    }
                    ret = 1;
                    free_data(parse_data->data[i].value, NULL);
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter name pass");
                }
                else
                {
                    ret = 0;
                    free_data(parse_data->data[i].value, NULL);
                    return ret;
                }
                break;
            }
            else
            {
                ret = 0;
                free_data(parse_data->data[i].value, NULL);
            }
        }
    }
    // manu_data match
    if (0 != filter_data->manu_data_len)
    {
        for (i = 0; i < BLE_SCANNER_MATCH_FILTER_DATA_TYPE; i++)
        {
            if (parse_data->data[i].type == DATA_TYPE_MANUFACTURER_SPECIFIC_DATA)
            {
                ret = filter_manu_srvc_data(parse_data->data[i].value, filter_data,
                                    parse_data->data[i].type, parse_data->data[i].len, 2);
                if (ret == 0)
                {
                    continue;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter manu_data pass");
                    break;
                }
            }
            else
            {
                ret = 0;
                free_data(parse_data->data[i].value, NULL);
            }
        }
    }
    //srvc_data match
    if (0 != filter_data->srvc_data_len)
    {
        for (i = 0; i < BLE_SCANNER_MATCH_FILTER_DATA_TYPE; i++)
        {
            if (parse_data->data[i].type == DATA_TYPE_SERVICE_DATA_16_BIT)
            {
                ret = filter_manu_srvc_data(parse_data->data[i].value, filter_data,
                                   parse_data->data[i].type, parse_data->data[i].len, 2);
                if (ret == 0)
                {
                    continue;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter serv_data_16_bit pass");
                    break;
                }
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_DATA_32_BIT)
            {
                ret = filter_manu_srvc_data(parse_data->data[i].value, filter_data,
                                    parse_data->data[i].type, parse_data->data[i].len, 4);
                if (ret == 0)
                {
                    continue;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter serv_data_32_bit pass");
                    break;
                }
            }
            else if (parse_data->data[i].type == DATA_TYPE_SERVICE_DATA_128_BIT)
            {
                ret = filter_manu_srvc_data(parse_data->data[i].value, filter_data,
                                   parse_data->data[i].type, parse_data->data[i].len, 16);
                if (ret == 0)
                {
                    continue;
                }
                else
                {
                    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "match_filter serv_data_128_bit pass");
                    break;
                }
            }
            else
            {
                ret = 0;
                free_data(parse_data->data[i].value, NULL);
            }
        }
    }
    return ret;
}


// Callback functions
void ble_scanner_register_cb(const Uuid& app_uuid, uint8_t scanner_id,
                                 uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_register_cb, scanner_id = %d, status = %d", scanner_id, status);

    if (status != 0)
    {
        BT_DBG_ERROR(BT_DEBUG_BLE_SCANNER, "ble_scanner_register_cb, register fail!");
        return;
    }

    char uuid[BT_BLE_SCANNER_MAX_UUID_LEN] = {0};
    linuxbt_uuid_to_array(app_uuid, uuid);
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_register_cb, uuid = %s", uuid);
    BT_BLE_SCANNER_REG_EVT_DATA register_data;
    memset(&register_data, 0, sizeof(BT_BLE_SCANNER_REG_EVT_DATA));
    register_data.status = status;
    memcpy(register_data.app_uuid, uuid, BT_BLE_SCANNER_MAX_UUID_LEN - 1);

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_REGISTER;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.register_data = register_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_set_scan_param_cb(uint8_t scanner_id, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_set_scan_param_cb, "
                                      "scanner_id = %d, status = %d", scanner_id, status);
    BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA scan_param_setup_data;
    memset(&scan_param_setup_data, 0, sizeof(BT_BLE_SCANNER_SCAN_PARAM_SETUP_EVT_DATA));
    scan_param_setup_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_SCAN_PARAM_SETUP;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.scan_param_setup_data = scan_param_setup_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_batchscan_cfg_storage_cb(uint8_t scanner_id, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_batchscan_cfg_storage_cb, "
                                      "scanner_id = %d, status = %d", scanner_id, status);
    BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA batchscan_cfg_data;
    memset(&batchscan_cfg_data, 0, sizeof(BT_BLE_SCANNER_BATCHSCAN_CONFIG_EVT_DATA));
    batchscan_cfg_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BATCHSCAN_CONFIG;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.batchscan_config_data = batchscan_cfg_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_batchscan_enable_cb(uint8_t scanner_id, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_batchscan_enable_cb, "
                                      "scanner_id = %d, status = %d", scanner_id, status);
    BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA batchscan_enable_data;
    memset(&batchscan_enable_data, 0, sizeof(BT_BLE_SCANNER_BATCHSCAN_ENABLE_EVT_DATA));
    batchscan_enable_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BATCHSCAN_ENABLE;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.batchscan_enable_data = batchscan_enable_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_batchscan_disable_cb(uint8_t scanner_id, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_batchscan_disable_cb, "
                                      "scanner_id = %d, status = %d", scanner_id, status);
    BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA batchscan_disable_data;
    memset(&batchscan_disable_data, 0, sizeof(BT_BLE_SCANNER_BATCHSCAN_DISABLE_EVT_DATA));
    batchscan_disable_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BATCHSCAN_DISABLE;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.batchscan_disable_data = batchscan_disable_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_scan_filter_param_setup_cb(uint8_t scanner_id, uint8_t avbl_space,
                                                        uint8_t action_type, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_scan_filter_param_setup_cb, "
                                      "scanner_id = %d, avbl_space = %d, action_type = %d, status = %d",
                                      scanner_id, avbl_space, action_type, status);
    BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA filt_param_setup_data;
    memset(&filt_param_setup_data, 0, sizeof(BT_BLE_SCANNER_FILT_PARAM_SETUP_EVT_DATA));
    filt_param_setup_data.avbl_space = avbl_space;
    filt_param_setup_data.action = action_type;
    filt_param_setup_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_FILT_PARAM_SETUP;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.filt_param_setup_data = filt_param_setup_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_scan_filter_add_cb(uint8_t scanner_id, uint8_t filt_type, uint8_t avbl_space,
                                                                uint8_t action, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_scan_filter_add_cb, "
                                      "scanner_id = %d, filt_type = %d, avbl_space = %d, action = %d, status = %d",
                                      scanner_id, filt_type, avbl_space, action, status);
    BT_BLE_SCANNER_FILT_ADD_EVT_DATA filt_add_data;
    memset(&filt_add_data, 0, sizeof(BT_BLE_SCANNER_FILT_ADD_EVT_DATA));
    filt_add_data.filt_type = filt_type;
    filt_add_data.avbl_space = avbl_space;
    filt_add_data.action = action;
    filt_add_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_FILT_ADD;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.filt_add_data = filt_add_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

void ble_scanner_scan_filter_clear_cb(uint8_t scanner_id, uint8_t filt_type, uint8_t avbl_space,
                                                                uint8_t action, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_scan_filter_clear_cb, "
                                      "scanner_id = %d, filt_type = %d, avbl_space = %d, action = %d, status = %d",
                                      scanner_id, filt_type, avbl_space, action, status);
    BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA filt_clear_data;
    memset(&filt_clear_data, 0, sizeof(BT_BLE_SCANNER_FILT_CLEAR_EVT_DATA));
    filt_clear_data.filt_type = filt_type;
    filt_clear_data.avbl_space = avbl_space;
    filt_clear_data.action = action;
    filt_clear_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_FILT_CLEAR;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.filt_clear_data = filt_clear_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}


void ble_scanner_filter_enable_cb(uint8_t scanner_id, uint8_t action, uint8_t status)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "ble_scanner_filter_enable_cb, "
                                      "scanner_id = %d, action = %d, status = %d",
                                      scanner_id, action, status);
    BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA filt_enable_data;
    memset(&filt_enable_data, 0, sizeof(BT_BLE_SCANNER_FILT_ENABLE_EVT_DATA));
    filt_enable_data.action = action;
    filt_enable_data.status = status;

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_FILT_ENABLE;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.filt_enable_data = filt_enable_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

static void linuxbt_ble_scan_result_callback(uint16_t event_type, uint8_t addr_type,
                                     RawAddress* bda, uint8_t primary_phy,
                                     uint8_t secondary_phy,
                                     uint8_t advertising_sid, int8_t tx_power,
                                     int8_t rssi, uint16_t periodic_adv_int,
                                     std::vector<uint8_t> adv_data)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "linuxbt_ble_scan_result_callback, "
                                      "event_type = %d, addr_type = %d, primary_phy = %d , primary_phy = %d, "
                                      "secondary_phy = %d, advertising_sid = %d, tx_power = %d , rssi = %d, "
                                      "periodic_adv_int = %d",
                                      event_type, addr_type, primary_phy, primary_phy, secondary_phy,advertising_sid,
                                      tx_power, rssi, periodic_adv_int);
    int ret = 1;
    char bd_addr[MAX_BDADDR_LEN] = {0};
    linuxbt_btaddr_htos(bda, bd_addr);
    BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA scan_result_data;
    memset(&scan_result_data, 0, sizeof(BT_BLE_SCANNER_SCAN_RESULT_EVT_DATA));
    scan_result_data.event_type = event_type;
    scan_result_data.addr_type = addr_type;
    memcpy(scan_result_data.bd_addr, bd_addr, MAX_BDADDR_LEN);
    scan_result_data.primary_phy = primary_phy;
    scan_result_data.secondary_phy = secondary_phy;
    scan_result_data.advertising_sid = advertising_sid;
    scan_result_data.tx_power = tx_power;
    scan_result_data.rssi = rssi;
    scan_result_data.periodic_adv_int = periodic_adv_int;
    scan_result_data.adv_data_len = adv_data.size();
    memcpy(scan_result_data.adv_data, adv_data.data(), adv_data.size() < BT_BLE_SCANNER_MAX_ATTR_LE
                                                     ? adv_data.size() : BT_BLE_SCANNER_MAX_ATTR_LE);

#if defined(LINUX_BLE_SCANNER_MATCH_FILTER) && (LINUX_BLE_SCANNER_MATCH_FILTER == TRUE)
    adv_data_t parse_data;
    UINT8 i = 0, j = 0;
    uint8_t scanner_id = 0;
    memset(&parse_data, 0, sizeof(parse_data));
    BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data = NULL;

    //regular scan/ batch scan
    for (i = 0; i < BT_BLE_SCANNER_MAX_REG_NUM; i++)
    {
        scanner_id = app_filt_data[i].scanner_id;
        if (scanner_id == 0)
        {
            if (i == (BT_BLE_SCANNER_MAX_REG_NUM - 1) && scanner_id == 0)
            {
                BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
                memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
                ble_scanner_msg.event = BT_BLE_SCANNER_EVT_SCAN_RESULT;
                ble_scanner_msg.data.scan_result_data = scan_result_data;
                bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
            }
        }
        else
        {
            break;
        }
    }

    //filter scan
    for (i = 0; i < BT_BLE_SCANNER_MAX_REG_NUM; i++)
    {
        scanner_id = app_filt_data[i].scanner_id;
        if (scanner_id != 0 && i == (scanner_id - 1))
        {
            for (j = 0; j < filter_num[i]; j++)
            {
                BT_DBG_NORMAL(BT_DEBUG_BLE_SCANNER, "filter_num= %d scanner_id: %d", filter_num[i], scanner_id);
                filter_data = &(app_filt_data[i].scan_filt_data[j]);
                decode_adv_data(scan_result_data.adv_data, &parse_data);
                ret = match_filters(&parse_data, bd_addr, filter_data);
                filter_data = NULL;
                if (ret == 0)
                {
                    break;
                }
            }
            BT_DBG_NORMAL(BT_DEBUG_BLE_SCANNER, "Scan_result_match_filter_end status: %d", ret);
            if (ret == 1)
            {
                BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
                memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
                ble_scanner_msg.event = BT_BLE_SCANNER_EVT_SCAN_RESULT;
                ble_scanner_msg.scanner_id = scanner_id;
                ble_scanner_msg.data.scan_result_data = scan_result_data;
                bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
            }
        }
    }
#endif
}

static void linuxbt_ble_batchscan_reports_callback(int scanner_id, int status,
                                           int report_format, int num_records,
                                           std::vector<uint8_t> data)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "linuxbt_ble_batchscan_reports_callback, "
                                      "scanner_if= %d, status= %d, report_format= %d, "
                                      "num_records = %d, data_len = %d",
                                      scanner_id, status, report_format, num_records, data.size());

    BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA batchscan_report_data;
    memset(&batchscan_report_data, 0, sizeof(BT_BLE_SCANNER_BATCHSCAN_REPORT_EVT_DATA));
    batchscan_report_data.status = status;
    batchscan_report_data.report_format = report_format;
    batchscan_report_data.num_records = num_records;
    batchscan_report_data.data_len= data.size();
    memcpy(batchscan_report_data.data, data.data(), data.size() < BT_BLE_SCANNER_MAX_ATTR_LE
                                                    ? data.size() : BT_BLE_SCANNER_MAX_ATTR_LE);

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BATCHSCAN_REPORT;
    ble_scanner_msg.scanner_id = scanner_id;
    ble_scanner_msg.data.batchscan_report_data = batchscan_report_data;
    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}


static void linuxbt_ble_batchscan_threshold_callback(int scanner_id)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "linuxbt_ble_batchscan_threshold_callback");

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BATCHSCAN_THRESHOLD;
    ble_scanner_msg.scanner_id = scanner_id;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}

static void linuxbt_ble_track_adv_event_callback(btgatt_track_adv_info_t* p_track_adv_info)
{
    BT_DBG_INFO(BT_DEBUG_BLE_SCANNER, "linuxbt_ble_track_adv_event_callback");

    char bd_addr[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(&p_track_adv_info->bd_addr, bd_addr);
    BT_BLE_SCANNER_TRACK_ADV_EVT_DATA track_adv_data;
    memset(&track_adv_data, 0, sizeof(BT_BLE_SCANNER_TRACK_ADV_EVT_DATA));
    track_adv_data.filt_index = p_track_adv_info->filt_index;
    track_adv_data.advertiser_state = p_track_adv_info->advertiser_state;
    track_adv_data.advertiser_info_present = p_track_adv_info->advertiser_info_present;
    track_adv_data.addr_type = p_track_adv_info->addr_type;
    track_adv_data.tx_power = p_track_adv_info->tx_power;
    track_adv_data.rssi_value = p_track_adv_info->rssi_value;
    track_adv_data.time_stamp = p_track_adv_info->time_stamp;
    memcpy(track_adv_data.bd_addr, bd_addr, MAX_BDADDR_LEN);
    track_adv_data.adv_pkt_len = p_track_adv_info->adv_pkt_len;
    memcpy(track_adv_data.p_adv_pkt_data, p_track_adv_info->p_adv_pkt_data, p_track_adv_info->adv_pkt_len);
    track_adv_data.scan_rsp_len = p_track_adv_info->scan_rsp_len;
    memcpy(track_adv_data.p_scan_rsp_data, p_track_adv_info->p_scan_rsp_data, p_track_adv_info->adv_pkt_len);

    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_TRACK_ADV;
    ble_scanner_msg.scanner_id = p_track_adv_info->client_if;
    ble_scanner_msg.data.track_adv_data = track_adv_data;

    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
}


btgatt_scanner_callbacks_t linuxbt_ble_scanner_callbacks =
{
    linuxbt_ble_scan_result_callback,
    linuxbt_ble_batchscan_reports_callback,
    linuxbt_ble_batchscan_threshold_callback,
    linuxbt_ble_track_adv_event_callback,
};


int linuxbt_ble_scanner_init(BleScannerInterface *pt_interface)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner] linuxbt_ble_scanner_init");
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    linuxbt_ble_scanner_interface = pt_interface;
    memset(app_filt_data, 0, sizeof(app_filt_data));
    return ret;
}


int linuxbt_ble_scanner_deinit(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner] linuxbt_ble_scanner_deinit");
    BT_BLE_SCANNER_CALLBACK_PARAM ble_scanner_msg;
    memset(app_filt_data, 0, sizeof(app_filt_data));
    memset(&ble_scanner_msg, 0, sizeof(BT_BLE_SCANNER_CALLBACK_PARAM));
    ble_scanner_msg.event = BT_BLE_SCANNER_EVT_BT_OFF;
    bt_mw_ble_scanner_notify_app(&ble_scanner_msg);
    return BT_SUCCESS;
}

int linuxbt_ble_scanner_register(char *pt_uuid)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_register");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    bool is_valid = FALSE;
    if (pt_uuid == NULL)
    {
        ret = BT_STATUS_FAIL;
        return linuxbt_return_value_convert(ret);
    }
    Uuid uuid = Uuid::FromString(std::string(pt_uuid, Uuid::kString128BitLen), &is_valid);
    if (!is_valid)
    {
        ret = BT_STATUS_FAIL;
        return linuxbt_return_value_convert(ret);
    }

    linuxbt_ble_scanner_interface->RegisterScanner(Bind(&ble_scanner_register_cb, uuid));

    return linuxbt_return_value_convert(ret);
}


int linuxbt_ble_scanner_scan(BOOL start)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_scan");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->Scan(start);

    return linuxbt_return_value_convert(ret);

}

int linuxbt_ble_scanner_set_scan_param(uint8_t scanner_id, int32_t scan_interval, int32_t scan_window)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_set_scan_parameters");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->SetScanParameters(scan_interval, scan_window,
                                              Bind(&ble_scanner_set_scan_param_cb, scanner_id));

    return linuxbt_return_value_convert(ret);

}


int linuxbt_ble_scanner_batchscan_cfg_storage(uint8_t scanner_id, uint8_t batch_scan_full_max,
                                                  uint8_t batch_scan_trunc_max,
                                                  uint8_t batch_scan_notify_threshold)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_batchscan_cfg_storage");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->BatchscanConfigStorage(scanner_id, batch_scan_full_max, batch_scan_trunc_max,
                                              batch_scan_notify_threshold,
                                              Bind(&ble_scanner_batchscan_cfg_storage_cb, scanner_id));

    return linuxbt_return_value_convert(ret);

}


int linuxbt_ble_scanner_batchscan_enable(uint8_t scanner_id, uint8_t scan_mode, int32_t scan_interval,
                                                   int32_t scan_window, uint8_t addr_type, uint8_t discard_rule)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_batchscan_enable, scan_interval = %d, scan_window = %d", scan_interval, scan_window);

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->BatchscanEnable(scan_mode, scan_interval, scan_window,
                                              addr_type,discard_rule,
                                              Bind(&ble_scanner_batchscan_enable_cb, scanner_id));

    return linuxbt_return_value_convert(ret);
}

int linuxbt_ble_scanner_batchscan_disable(uint8_t scanner_id)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_batchscan_disable");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->BatchscanDisable(Bind(&ble_scanner_batchscan_disable_cb, scanner_id));

    return linuxbt_return_value_convert(ret);
}



int linuxbt_ble_scanner_batchscan_read_reports(uint8_t scanner_id, uint8_t scan_mode)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_batchscan_read_reports");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->BatchscanReadReports(scanner_id, scan_mode);

    return linuxbt_return_value_convert(ret);

}


int linuxbt_ble_scanner_scan_filter_param_setup(uint8_t scanner_id, uint8_t action, uint8_t filt_index,
                                                                    BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filter_param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_scan_filter_param_setup");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();
    if (action == APCF_ACTION_ADD)
    {
        auto filt_params = std::make_unique<btgatt_filt_param_setup_t>();
        filt_params->feat_seln = scan_filter_param->feat_seln;
        filt_params->list_logic_type = scan_filter_param->list_logic_type;
        filt_params->filt_logic_type = scan_filter_param->filt_logic_type;
        filt_params->rssi_high_thres = scan_filter_param->rssi_high_thres;
        filt_params->rssi_low_thres = scan_filter_param->rssi_low_thres;
        filt_params->dely_mode = scan_filter_param->dely_mode;
        filt_params->found_timeout = scan_filter_param->found_timeout;
        filt_params->lost_timeout = scan_filter_param->lost_timeout;
        filt_params->found_timeout_cnt = scan_filter_param->found_timeout_cnt;
        filt_params->num_of_tracking_entries = scan_filter_param->num_of_tracking_entries;
        linuxbt_ble_scanner_interface->ScanFilterParamSetup(scanner_id, action, filt_index,
                                                  std::move(filt_params),
                                                  Bind(&ble_scanner_scan_filter_param_setup_cb, scanner_id));
    }
    else if (action == APCF_ACTION_DELETE)
    {
        //delet match filter_data
        UINT32 i = 0;
        i = scanner_id - 1;
        UINT32 scan_filt_index = filter_num[i];
        if (scanner_id == app_filt_data[i].scanner_id)
        {
            BT_DBG_NORMAL(BT_DEBUG_BLE_SCANNER, "scanner_id= %d filter_index = %d", scanner_id, filt_index);
            memset(&(app_filt_data[i].scan_filt_data[scan_filt_index]), 0 , sizeof(app_filt_data[i].scan_filt_data[scan_filt_index]));
            filter_num[i] = filter_num[i] - 1;
            if (filter_num[i] == 0)
            {
                app_filt_data[i].scanner_id = 0;
            }
        }
        linuxbt_ble_scanner_interface->ScanFilterParamSetup(scanner_id, action, filt_index,
                                                  nullptr,
                                                  Bind(&ble_scanner_scan_filter_param_setup_cb, scanner_id));

    }
    else if (action == APCF_ACTION_CLEAR)
    {
        linuxbt_ble_scanner_interface->ScanFilterParamSetup(scanner_id, APCF_ACTION_CLEAR, 0/* index, unused */,
                                               nullptr, Bind(&ble_scanner_scan_filter_param_setup_cb, scanner_id));
    }
    else
    {
        ret= BT_STATUS_FAIL;
    }

    return linuxbt_return_value_convert(ret);

}

int linuxbt_ble_scanner_scan_filter_add(uint8_t scanner_id, uint8_t filter_index,
                                                    BT_BLE_SCANNER_SCAN_FILT_DATA *filter_data)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_scan_filter_add filter_index:%d", filter_index);
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_BLE_SCANNER_INTERFACE();

    std::vector<ApcfCommand> filt_datas;

    //add match filter_data
    UINT32 i = 0;
    i = scanner_id - 1;
    app_filt_data[i].scanner_id = scanner_id;
    UINT32 scan_filt_index = filter_num[i];
    BT_DBG_NORMAL(BT_DEBUG_BLE_SCANNER, "scanner_id= %d filter_index = %d", app_filt_data[i].scanner_id, filter_index);
    memset(&(app_filt_data[i].scan_filt_data[scan_filt_index]), 0 , sizeof(app_filt_data[i].scan_filt_data[scan_filt_index]));
    memcpy(&(app_filt_data[i].scan_filt_data[scan_filt_index]), filter_data, sizeof(BT_BLE_SCANNER_SCAN_FILT_DATA));
    filter_num[i] = filter_num[i] + 1;
    if (NULL != filter_data)
    {
        if (0 != strlen(filter_data->bt_addr))
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_ADDRESS;
            LINUXBT_CHECK_AND_CONVERT_ADDR(filter_data->bt_addr, curr.address);
            curr.addr_type = filter_data->addr_type;
            filt_datas.push_back(curr);
        }
        if (0 != strlen(filter_data->srvc_uuid) && 0 != strlen(filter_data->srvc_uuid_mask))
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_SRVC_UUID;
            bool is_valid = FALSE;
            curr.uuid = Uuid::FromString(std::string(filter_data->srvc_uuid, strlen(filter_data->srvc_uuid)), &is_valid);
            if (!is_valid)
            {
                ret = BT_STATUS_FAIL;
                return linuxbt_return_value_convert(ret);
            }
            curr.uuid_mask = Uuid::FromString(std::string(filter_data->srvc_uuid_mask, strlen(filter_data->srvc_uuid_mask)), &is_valid);
            if (!is_valid)
            {
                ret = BT_STATUS_FAIL;
                return linuxbt_return_value_convert(ret);
            }
            filt_datas.push_back(curr);
        }
        if (0 != strlen(filter_data->srvc_sol_uuid) && 0 != strlen(filter_data->srvc_sol_uuid_mask))
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_SRVC_SOL_UUID;
            bool is_valid = FALSE;
            curr.uuid = Uuid::FromString(std::string(filter_data->srvc_sol_uuid, strlen(filter_data->srvc_sol_uuid)), &is_valid);
            if (!is_valid)
            {
                ret = BT_STATUS_FAIL;
                return linuxbt_return_value_convert(ret);
            }
            curr.uuid_mask = Uuid::FromString(std::string(filter_data->srvc_sol_uuid_mask, strlen(filter_data->srvc_sol_uuid_mask)), &is_valid);
            if (!is_valid)
            {
                ret = BT_STATUS_FAIL;
                return linuxbt_return_value_convert(ret);
            }
            filt_datas.push_back(curr);
        }
        if (0 != strlen(filter_data->local_name))
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_LOCAL_NAME;
            std::vector<uint8_t> name_t(filter_data->local_name, (filter_data->local_name) + strlen(filter_data->local_name));
            curr.name.assign(name_t.begin(), name_t.end());
            filt_datas.push_back(curr);
        }
        if (0 != filter_data->manu_data_len)
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_MANU_DATA;
            curr.company = filter_data->company;
            curr.company_mask = filter_data->company_mask;
            std::vector<uint8_t> data_t(filter_data->manu_data, (filter_data->manu_data) + filter_data->manu_data_len);
            curr.data.assign(data_t.begin(), data_t.end());
            std::vector<uint8_t> data_mask_t(filter_data->manu_data_mask, (filter_data->manu_data_mask) + filter_data->manu_data_len);
            curr.data_mask.assign(data_mask_t.begin(), data_mask_t.end());
            filt_datas.push_back(curr);
        }
        if (0 != filter_data->srvc_data_len)
        {
            ApcfCommand curr = {0};
            curr.type = SCAN_FILT_TYPE_SRVC_DATA;
            std::vector<uint8_t> data_t(filter_data->srvc_data, (filter_data->srvc_data) + filter_data->srvc_data_len);
            curr.data.assign(data_t.begin(), data_t.end());
            std::vector<uint8_t> data_mask_t(filter_data->srvc_data_mask, (filter_data->srvc_data_mask) + filter_data->srvc_data_len);
            curr.data_mask.assign(data_mask_t.begin(), data_mask_t.end());
            filt_datas.push_back(curr);
        }
    }

    linuxbt_ble_scanner_interface->ScanFilterAdd(filter_index, std::move(filt_datas),
                                              Bind(&ble_scanner_scan_filter_add_cb, scanner_id));
    return linuxbt_return_value_convert(ret);

}

int linuxbt_ble_scanner_scan_filter_clear(uint8_t scanner_id, uint8_t filt_index)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_scan_filter_clear");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->ScanFilterClear(filt_index, Bind(&ble_scanner_scan_filter_clear_cb, scanner_id));

    return linuxbt_return_value_convert(ret);

}



INT32 linuxbt_ble_scanner_scan_filter_enable(uint8_t scanner_id, BOOL enable)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_scan_filter_enable");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->ScanFilterEnable(enable, Bind(&ble_scanner_filter_enable_cb, scanner_id));

    return linuxbt_return_value_convert(ret);

}


INT32 linuxbt_ble_scanner_unregister(uint8_t scanner_id)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_BLE_SCANNER, "[ble_scanner]linuxbt_ble_scanner_unregister");

    bt_status_t ret= BT_STATUS_SUCCESS;

    CHECK_BLE_SCANNER_INTERFACE();

    linuxbt_ble_scanner_interface->Unregister(scanner_id);

    return linuxbt_return_value_convert(ret);

}

