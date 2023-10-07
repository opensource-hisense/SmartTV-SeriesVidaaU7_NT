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
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "mtk_bt_service_ble_scanner.h"
#include "mtk_bt_service_ble_scanner_handle.h"
#include "mtk_bt_service_ble_scanner_ipcrpc_struct.h"
#include "ri_common.h"
#include "util_ble_scanner_timer.h"
#include <pthread.h>
#include <time.h>
#include "mtk_bt_service_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Ble_Scanner]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Ble_Scanner]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static pthread_mutex_t g_bt_mw_rpc_ble_scanner_lock;
#define BT_MW_RPC_BLE_SCANNER_LOCK() \
            do { \
                pthread_mutex_lock(&(g_bt_mw_rpc_ble_scanner_lock)); \
            }while(0)

#define BT_MW_RPC_BLE_SCANNER_UNLOCK() \
            do { \
                pthread_mutex_unlock(&(g_bt_mw_rpc_ble_scanner_lock));\
            }while(0)

static VOID bt_app_ble_scanner_batchscan_report_timer_proc(UINT8 timer_id, VOID *pv_args);

struct dl_list ble_scanner_list = {&ble_scanner_list, &ble_scanner_list};
BT_BLE_SCANNER_FILT_INDEX total_filt_indexs[BT_BLE_SCANNER_MAX_SCAN_FILT_NUM];
UTIL_BLE_SCANNER_TIMER_T batchscan_report_timer = {-1, NULL, 5000, UTIL_BLE_SCANNER_TIMER_REPEAT_TYPE_REPEAT,
    (UTIL_BLE_SCANNER_TIMER_HANDLER)bt_app_ble_scanner_batchscan_report_timer_proc, NULL};

BOOL agent_regular_scan_flag = FALSE;
BOOL agent_batch_scan_flag = FALSE;
BOOL scan_flag = FALSE; //control whether to call start scan api
BOOL star_scan_flag = FALSE; //control scan status
BOOL batch_scan_timer_flag = FALSE; //control batch scan timer status

INT8 regular_scan_filter_add_index[BT_BLE_SCANNER_MAX_REG_NUM] = {0}; //control one pair filter data and param of regular scan
INT8 batch_scan_filter_add_index[BT_BLE_SCANNER_MAX_REG_NUM] = {0}; //control one pair filter data and param of batch scan
INT32 add_filt_data_flag[BT_BLE_SCANNER_MAX_REG_NUM] = {0}; //record every app's filter data add status
BT_BLE_SCANNER_APP_FILT app_filters [BT_BLE_SCANNER_MAX_REG_NUM]; //record every app's index


VOID ble_scanner_mutex_init(VOID)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_bt_mw_rpc_ble_scanner_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    return;
}

static VOID generate_random_uuid(CHAR * uuid) {
    INT32 i = 0, n = 0;
    UINT8 max = 0xFF;
    UINT8 min = 0x00;
    CHAR *ptr;
    CHAR  temp_uuid[16] = {0};
    if (NULL == uuid)
    {
        return;
    }

    time_t seconds = time(0);
    if (seconds == (time_t)(-1))
    {
        return;
    }
    else
    {
        srand(seconds);
    }

    for (i = 0; i < 16; i++)
    {
        temp_uuid[i]= rand() % (max -min) + min;
    }

    ptr = uuid;
    for (i = 15; i >= 0; i--)
    {
        n = sprintf(ptr, "%02X", temp_uuid[i]);
        if (n < 0)
        {
            return;
        }
        ptr+=2;
        if (i == 12 || i == 10 || i == 8 || i == 6)
        {
            *ptr = '-';
            ptr++;
        }
    }
    *ptr = '\0';
}

static BOOL is_valid_uuid(CHAR* uuid)
{
    int uuid_size = strlen(uuid);
    if (uuid_size == 36)
    {
        if (uuid[8] != '-' || uuid[13] != '-' || uuid[18] != '-' ||
            uuid[23] != '-') {
          return FALSE;
        }
        int c;
        CHAR uuid_t[16] = {0};
        int rc =
            sscanf(uuid,
                   "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx"
                   "-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%n",
                   &uuid_t[0], &uuid_t[1], &uuid_t[2], &uuid_t[3], &uuid_t[4], &uuid_t[5], &uuid_t[6], &uuid_t[7], &uuid_t[8],
                   &uuid_t[9], &uuid_t[10], &uuid_t[11], &uuid_t[12], &uuid_t[13], &uuid_t[14], &uuid_t[15], &c);
        if (rc != 16) return FALSE;
        if (c != 36) return FALSE;
        return TRUE;
    }
    else if (uuid_size == 8)
    {
        int c;
        CHAR uuid_t[4] = {0};
        int rc = sscanf(uuid, "%02hhx%02hhx%02hhx%02hhx%n", &uuid_t[0], &uuid_t[1], &uuid_t[2], &uuid_t[3], &c);
        if (rc != 4) return FALSE;
        if (c != 8) return FALSE;
        return TRUE;
    }
    else if (uuid_size == 4)
    {
        int c;
        CHAR uuid_t[2] = {0};
        int rc = sscanf(uuid, "%02hhx%02hhx%n", &uuid_t[0], &uuid_t[1], &c);
        if (rc != 2) return FALSE;
        if (c != 4) return FALSE;
        return TRUE;
    }
    return FALSE;
}


static BOOL is_valid_address(CHAR* address)
{
    int address_size = strlen(address);
    if(address_size != 17)
    {
        BT_RC_LOG("is not valid address");
        return FALSE;
    }
    if (address[2] != ':' || address[5] != ':' || address[8] != ':' || address[11] != ':' || address[14] != ':')
    {
        BT_RC_LOG("is not valid address");
        return FALSE;
    }
    for (int i = 0; i < address_size; i++)
    {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
        {
            continue;
        }
        if (address[i] >= '0' && address[i] <= '9')
        {
            continue;
        }
        if (address[i] >= 'A' && address[i] <= 'F')
        {
            continue;
        }
        if (address[i] >= 'a' && address[i] <= 'f')
        {
            continue;
        }
        BT_RC_LOG("is not valid address");
        return FALSE;
    }
    return TRUE;
}

static INT32 ble_scanner_list_remove_by_uuid(struct dl_list* list, CHAR* uuid)
{
    BT_RH_LOG("ble_scanner_list_remove_by_uuid");
    if (NULL == list || NULL == uuid)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }
    if (0 == dl_list_len(list))
    {
        BT_RH_LOG("No available node remove.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (item_t->ble_scanner_info == NULL)
        {
            return -1;
        }
        if (0 == strncmp(uuid, item_t->ble_scanner_info->scanner_uuid, strlen(item_t->ble_scanner_info->scanner_uuid)))
        {
            if (NULL != item_t->ble_scanner_info)
            {
                free(item_t->ble_scanner_info);
                item_t->ble_scanner_info = NULL;
            }
            dl_list_del(&item_t->node);
            free(item_t);
            BT_RH_LOG("Remove by uuid success. uuid = %s", uuid);
            return 0;
        }
    }

    BT_RC_LOG("ble_scanner_list_remove_by_uuid fail!");
    return -1;
}

static BLE_SCANNER_INFO* ble_scanner_list_update_scanner_id(struct dl_list* list,
                                                           BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return NULL;
    }
    BT_RH_LOG("update scanner_id %d", cb_param->scanner_id);

    BT_BLE_SCANNER_REG_EVT_DATA bt_reg_scanner_data = cb_param->data.register_data;
    if (bt_reg_scanner_data.status != 0) // register scanner_id fail, need remove scanner info by uuid
    {
        BT_RH_LOG("register scanner_id fail, remove scanner node from link list, status = %d.", bt_reg_scanner_data.status);
        if (ble_scanner_list_remove_by_uuid(list, bt_reg_scanner_data.app_uuid) < 0)
        {
            BT_RC_LOG("remove scanner node fail");
        }
        return NULL;
    }


    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        BT_RH_LOG("item_t->ble_scanner_info->scanner_uuid = %s \n", item_t->ble_scanner_info->scanner_uuid);
        if (0 == strncmp(bt_reg_scanner_data.app_uuid, item_t->ble_scanner_info->scanner_uuid,
                         strlen(item_t->ble_scanner_info->scanner_uuid)))
        {
            item_t->ble_scanner_info->scanner_id = cb_param->scanner_id;
            BT_RH_LOG("update scanner_id %d success.", cb_param->scanner_id);
            return item_t->ble_scanner_info;
        }
    }

    BT_RC_LOG("ble_scanner_list_update_scanner_id %d fail.", cb_param->scanner_id);
    return NULL;
}

static INT32 ble_scanner_list_add(struct dl_list* list,
                                      BLE_SCANNER_REG_CB_LIST* item)
{
    BT_RH_LOG("ble_scanner_list_add");
    if (NULL == list || NULL == item)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }
    if (dl_list_len(list) > BT_BLE_SCANNER_MAX_REG_NUM)
    {
        BT_RC_LOG("ble_scanner_list_add exceeds the maximum %d.", BT_BLE_SCANNER_MAX_REG_NUM);
        return -1;
    }

    dl_list_add_tail(list, &item->node);

    BT_RH_LOG("Add ble_scanner info success.");
    return 0;
}

static INT32 ble_scanner_list_remove_by_scanner_id(struct dl_list* list,
                                                            INT32 scanner_id)
{
    BT_RH_LOG("ble_scanner_list_remove_by_scanner_id: %d", scanner_id);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }
    if (0 == dl_list_len(list))
    {
        BT_RC_LOG("No available node remove.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (item_t->ble_scanner_info == NULL)
        {
            return -1;
        }
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            if (NULL != item_t->ble_scanner_info)
            {
                free(item_t->ble_scanner_info);
                item_t->ble_scanner_info = NULL;
            }
            dl_list_del(&item_t->node);
            free(item_t);
            BT_RC_LOG("Remove scanner info by server_id success. scanner_id = %d", scanner_id);
            return 0;
        }
    }

    BT_RC_LOG("ble_scanner_list_remove_by_scanner_id fail!");
    return -1;
}

static INT32 ble_scanner_list_update_scan_param(struct dl_list* list,
                                                         BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param)
{
    BT_RH_LOG("ble_scanner_list_update_scan_param");
    if (NULL == list || NULL == start_scan_param)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }
    UINT8 scanner_id = start_scan_param->scanner_id;
    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            //BT_BLE_SCANNER_START_SCAN_PARAM *scan_param = &item_t->ble_scanner_info->scan_param;
            //scan_param = start_scan_param;
            item_t->ble_scanner_info->scan_param = *start_scan_param;
            BT_RH_LOG("ble_scanner_list_update_scan_param success.");
            return 0;
        }

    }

    BT_RC_LOG("ble_scanner_list_update_scan_param fail! scanner_id = %d!", scanner_id);
    return -1;
}

static INT32 ble_scanner_list_update_regular_scan_status(struct dl_list* list,
                                                                  INT32 scanner_id, BOOL regular_scan_flag)
{
    BT_RH_LOG("ble_scanner_list_update_regular_scan_status, scanner_id = %d, regular_scan_flag = %d", scanner_id, regular_scan_flag);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            item_t->ble_scanner_info->regular_scan_status = regular_scan_flag;
            BT_RH_LOG("Update regular scan status success, scanner_id = %d, regular_scan_flag = %d", scanner_id, regular_scan_flag);
            return 0;
        }
    }

    BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail! scanner_id = %d!", scanner_id);
    return -1;
}

static INT32 ble_scanner_list_update_batch_scan_status(struct dl_list* list,
                                                                 INT32 scanner_id, BOOL batch_scan_flag)
{
    BT_RH_LOG("ble_scanner_list_update_batch_scan_status, scanner_id = %d, batch_scan_flag = %d", scanner_id, batch_scan_flag);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            item_t->ble_scanner_info->batch_scan_status = batch_scan_flag;
            BT_RH_LOG("Update batchscan status success, scanner_id = %d, batch_scan_flag = %d", scanner_id, batch_scan_flag);
            return 0;
        }
    }

    BT_RH_LOG("Update batch scan status fail! scanner_id = %d!", scanner_id);
    return -1;
}

static BOOL ble_scanner_list_get_regular_scan_status_by_scanner_id(struct dl_list* list,
                                                                    INT32 scanner_id)
{
    BT_RH_LOG("ble_scanner_list_get_regular_scan_status_by_scanner_id: %d", scanner_id);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            BT_RH_LOG("scanner_id=%d, regular_scan_status = %d", scanner_id, item_t->ble_scanner_info->regular_scan_status);
            return item_t->ble_scanner_info->regular_scan_status;
        }
    }

    BT_RC_LOG("ble scanner_id = %d not register.", scanner_id);
    return FALSE;
}

static BOOL ble_scanner_list_get_batch_scan_status_by_scanner_id(struct dl_list* list,
                                                                              INT32 scanner_id)
{
    BT_RH_LOG("ble_scanner_list_get_batch_scan_status_by_scanner_id: %d", scanner_id);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return FALSE;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            BT_RH_LOG("scanner_id=%d, batch_scan_status = %d", scanner_id, item_t->ble_scanner_info->batch_scan_status);
            return item_t->ble_scanner_info->batch_scan_status;
        }
    }

    BT_RC_LOG("ble scanner_id = %d not register.", scanner_id);
    return FALSE;
}

static INT32 ble_scanner_list_find_regular_scan_num(struct dl_list* list)
{
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;
    INT32 count = 0;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (item_t->ble_scanner_info->regular_scan_status)
        {
            count++;
        }
    }

    BT_RH_LOG("ble regular scan num is %d \n", count);
    return count;
}

static INT32 ble_scanner_list_find_batch_scan_num(struct dl_list* list)
{
    if (NULL == list)
    {
        BT_RH_LOG("Invalid parameter.");
        return -1;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;
    INT32 count = 0;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (item_t->ble_scanner_info->batch_scan_status)
        {
            count++;
        }
    }

    BT_RH_LOG("ble batchscan num is %d \n", count);
    return count;
}

static VOID ble_scanner_list_show(struct dl_list* list)
{
    BT_RH_LOG("ble_scanner_list_show");
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        BT_RH_LOG("rpc_id = %d", item_t->ble_scanner_info->t_id);
        BT_RH_LOG("scanner_id = %d", item_t->ble_scanner_info->scanner_id);
        BT_RH_LOG("regular_scan_status = %d", item_t->ble_scanner_info->regular_scan_status);
        BT_RH_LOG("batch_scan_status = %d", item_t->ble_scanner_info->batch_scan_status);
        BT_RH_LOG("scanner_uuid = %s", item_t->ble_scanner_info->scanner_uuid);
    }
}

static BOOL ble_scanner_list_check_uuid_exist(struct dl_list* list, CHAR * uuid)
{
    BT_RH_LOG("ble_scanner_list_check_uuid_exist");
    if (NULL == list || NULL == uuid)
    {
        BT_RC_LOG("Invalid parameter.");
        return FALSE;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (0 == strncmp(item_t->ble_scanner_info->scanner_uuid, uuid, strlen(uuid)))
        {
            BT_RH_LOG("ble_scanner uuid exist. uuid = %s", uuid);
            return TRUE;
        }
    }

    BT_RH_LOG("ble_scanner uuid is not exist.");
    return FALSE;
}

static BOOL ble_scanner_list_check_scanner_id_exist(struct dl_list* list,
                                                           INT32 scanner_id)
{
    BT_RH_LOG("ble_scanner_list_check_scanner_id_exist: %d", scanner_id);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return FALSE;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            BT_RH_LOG("ble scanner scanner_id exist");
            return TRUE;
        }
    }

    BT_RC_LOG("ble scanner_id not register.");
    return FALSE;
}

static BLE_SCANNER_INFO* ble_scanner_list_find_scannerinfo_by_scanner_id(struct dl_list* list,
                                                                                         UINT8 scanner_id)
{
    BT_RH_LOG("ble_scanner_list_find_scannerinfo_by_scanner_id: %d", scanner_id);
    if (NULL == list)
    {
        BT_RC_LOG("Invalid parameter.");
        return NULL;
    }

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (scanner_id == item_t->ble_scanner_info->scanner_id)
        {
            BT_RH_LOG("ble scanner find ble scanner info by scanner_id %d success.", scanner_id);
            return item_t->ble_scanner_info;
        }
    }

    BT_RC_LOG("ble scanner find ble scanner info by scanner_id %d fail!", scanner_id);
    return NULL;
}

//when call stop scan, remove all filter data of this scanner id
VOID remove_scan_filter(BLE_SCANNER_INFO **bleScannerInfo)
{
    BLE_SCANNER_INFO *bleScannerInfo_p = *bleScannerInfo;
    INT32 i4_ret = 0, i = 0;
    UINT32 k = 0;
    INT32 scanner_id = 0;
    scanner_id = bleScannerInfo_p->scanner_id;
    k = scanner_id - 1;
    if (app_filters[k].scanner_id == scanner_id)
    {
        for (i=0; i< BT_BLE_SCANNER_MAX_SCAN_FILT_NUM; i++)
        {
            if (app_filters[k].filt_indexs[i].in_use && total_filt_indexs[i].in_use)
            {
                total_filt_indexs[i].in_use = FALSE;
                app_filters[k].filt_indexs[i].in_use = FALSE;
                //filter param setup [delete]
                BT_RC_LOG("remove_scanner_id = %d total _filt_index= %d filt_index= %d",scanner_id,
                total_filt_indexs[i].filter_index, app_filters[k].filt_indexs[i].filter_index);
                i4_ret = x_mtkapi_bt_ble_scanner_scan_filter_param_setup(scanner_id, APCF_ACTION_DELETE,
                               app_filters[k].filt_indexs[i].filter_index, NULL);
                if (i4_ret != 0)
                {
                    BT_RC_LOG("[.c][%s][%d]x_mtkapi_bt_ble_scanner_scan_filter_param_setup(remove) fail\n",
                                                                    __FUNCTION__, scanner_id);
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        return;
    }
    *bleScannerInfo = bleScannerInfo_p;
}


static VOID config_filt_param(BLE_SCANNER_INFO *bleScannerInfo)
{
    BT_RH_LOG("config_filt_param");
    INT32 i4_ret = 0;
    UINT32 k = 0;
    UINT8 scanner_id = bleScannerInfo->scan_param.scanner_id;
    UINT8 filtes_num = 0;
    INT8 scan_type = 0;
    BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filt_param = NULL;
    k = scanner_id - 1;
    if (bleScannerInfo->scan_param.scan_type == REGULAR_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM *regular_scan_filt_param =
                    &bleScannerInfo->scan_param.scan_param.regular_scan_filt_param;
        filtes_num = regular_scan_filt_param->scan_filt_num;
        scan_filt_param = regular_scan_filt_param->scan_filt_param;
        scan_type = REGULAR_SCAN_WITH_FILT;
    }
    else if (bleScannerInfo->scan_param.scan_type == BATCH_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM *batch_scan_filt_param =
                    &bleScannerInfo->scan_param.scan_param.batch_scan_filt_param;
        filtes_num = batch_scan_filt_param->scan_filt_num;
        scan_filt_param = batch_scan_filt_param->scan_filt_param;
        scan_type = BATCH_SCAN_WITH_FILT;
    }
    else
    {
        BT_RH_LOG("not regular or batch scan filt type: %d", bleScannerInfo->scan_param.scan_type);
        return;
    }

    UINT8 valid_filter_num = (filtes_num < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM)
                              ? filtes_num : BT_BLE_SCANNER_MAX_SCAN_FILT_NUM;
    for (int i = 0; i < valid_filter_num; i++)
    {
        if (!bleScannerInfo->filt_indexs[i].in_use)
        {
            continue;
        }
        if (scan_type == REGULAR_SCAN_WITH_FILT)
        {
            //filter param setup [add]
            i4_ret = x_mtkapi_bt_ble_scanner_scan_filter_param_setup(scanner_id, APCF_ACTION_ADD,
                                                   regular_scan_filter_add_index[k], &(scan_filt_param[i]));
            if (i4_ret == 0)
            {
                BT_RH_LOG("x_mtkapi_bt_ble_scanner_scan_filter_param_setup success");
            }
            else
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_scan_filter_param_setup fail");
            }
            regular_scan_filter_add_index[k] = -1;
            break;
        }
        if (scan_type == BATCH_SCAN_WITH_FILT)
        {
            //filter param setup [add]
            i4_ret = x_mtkapi_bt_ble_scanner_scan_filter_param_setup(scanner_id, APCF_ACTION_ADD,
                                                   batch_scan_filter_add_index[k], &(scan_filt_param[i]));
            if (i4_ret == 0)
            {
                BT_RH_LOG("x_mtkapi_bt_ble_scanner_scan_filter_param_setup success");
            }
            else
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_scan_filter_param_setup fail");
            }
            batch_scan_filter_add_index[k] = -1;
            break;
        }
    }
}


static INT32 config_add_filt_data(BLE_SCANNER_INFO **bleScannerInfo)
{
    BT_RH_LOG("config_add_filt_data");
    BLE_SCANNER_INFO *bleScannerInfo_t = *bleScannerInfo;
    INT32 i4_ret = 0;
    UINT8 scanner_id = bleScannerInfo_t->scan_param.scanner_id;
    UINT8 filtes_num = 0, i = 0, k = 0, j = 0;
    //BT_BLE_SCANNER_SCAN_FILT_PARAM *scan_filt_param = NULL;
    BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data = NULL;
    INT8 scan_type = 0;
    k = scanner_id - 1;
    INT32 result = CONFIG_ADD_FILTE_DATA_DONE;

    if (bleScannerInfo_t->scan_param.scan_type == REGULAR_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM *regular_scan_filt_param =
                    &bleScannerInfo_t->scan_param.scan_param.regular_scan_filt_param;
        filtes_num = regular_scan_filt_param->scan_filt_num;
        //scan_filt_param = regular_scan_filt_param->scan_filt_param;
        scan_filt_data = regular_scan_filt_param->scan_filt_data;
        scan_type = REGULAR_SCAN_WITH_FILT;
        regular_scan_filter_add_index[k] = -1;
    }
    else if (bleScannerInfo_t->scan_param.scan_type == BATCH_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM *batch_scan_filt_param =
                    &bleScannerInfo_t->scan_param.scan_param.batch_scan_filt_param;
        filtes_num = batch_scan_filt_param->scan_filt_num;
        //scan_filt_param = batch_scan_filt_param->scan_filt_param;
        scan_filt_data = batch_scan_filt_param->scan_filt_data;
        scan_type = BATCH_SCAN_WITH_FILT;
        k = scanner_id - 1;
        batch_scan_filter_add_index[k] = -1;
    }
    else
    {
        BT_RH_LOG("scan type is not filt type: %d", bleScannerInfo_t->scan_param.scan_type);
        i4_ret = -1;
        return i4_ret;
    }
    //app_filtes_num[k] = filtes_num;
    if (filtes_num > BT_BLE_SCANNER_MAX_SCAN_FILT_NUM)
    {
        BT_RC_LOG("scan_filter_add fail");
        result = CONFIG_ADD_FILTE_DATA_FAIL;
    }
    for (i = 0; i < filtes_num; i++)
    {
        if ((bleScannerInfo_t->filt_indexs[i].in_use))
        {
            continue;
        }
        for (j = 0; j < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM; j++)
        {
            if (total_filt_indexs[j].in_use)
            {
                continue;
            }
            total_filt_indexs[j].in_use = TRUE;
            break;
        }
        if (j == BT_BLE_SCANNER_MAX_SCAN_FILT_NUM)
        {
            BT_RC_LOG("config_add_filt_data, reach max filt num");
            return CONFIG_ADD_FILTE_DATA_FAIL;
        }
        bleScannerInfo_t->filt_indexs[i].in_use = TRUE;
        bleScannerInfo_t->filt_indexs[i].filter_index = total_filt_indexs[j].filter_index;

        if (scan_type == REGULAR_SCAN_WITH_FILT)
        {
            regular_scan_filter_add_index[k] = total_filt_indexs[j].filter_index;
            app_filters[k].scanner_id = scanner_id;
            app_filters[k].filt_indexs[j].in_use = TRUE;
            app_filters[k].filt_indexs[j].filter_index = total_filt_indexs[j].filter_index;
            BT_RC_LOG("scanner_id = %d total_filt_index= %d filt_index= %d",app_filters[k].scanner_id,
                total_filt_indexs[j].filter_index, app_filters[k].filt_indexs[j].filter_index);
            //filter add
            i4_ret = x_mtkapi_bt_ble_scanner_scan_filter_add(scanner_id,
                                                 regular_scan_filter_add_index[k], &(scan_filt_data[i]));

            if (i4_ret == 0)
            {
                BT_RH_LOG("regular_scan_filter_add success");
                result = CONFIG_ADD_FILTE_DATA_CONTINUE;
            }
            else
            {
                BT_RC_LOG("regular_scan_filter_add fail");
                result = CONFIG_ADD_FILTE_DATA_FAIL;
            }
        }
        if (scan_type == BATCH_SCAN_WITH_FILT)
        {
            batch_scan_filter_add_index[k] = total_filt_indexs[j].filter_index;
            app_filters[k].scanner_id = scanner_id;
            app_filters[k].filt_indexs[j].in_use = TRUE;
            app_filters[k].filt_indexs[j].filter_index = total_filt_indexs[j].filter_index;
            BT_RC_LOG("scanner_id = %d total_filt_index= %d filt_index= %d",app_filters[k].scanner_id,
                total_filt_indexs[j].filter_index, app_filters[k].filt_indexs[j].filter_index);
            //filter add
            i4_ret = x_mtkapi_bt_ble_scanner_scan_filter_add(scanner_id,
                                                 batch_scan_filter_add_index[k], &(scan_filt_data[i]));
            if (i4_ret == 0)
            {
                BT_RH_LOG("batch_scan_filter_add success");
                add_filt_data_flag[k] = CONFIG_ADD_FILTE_DATA_CONTINUE;
            }
            else
            {
                BT_RC_LOG("batch_scan_filter_add fail");
                add_filt_data_flag[k] = CONFIG_ADD_FILTE_DATA_FAIL;
            }
        }
        break;
    }
    *bleScannerInfo = bleScannerInfo_t;
    return result;
}

static VOID config_scan_param(BLE_SCANNER_INFO *bleScannerInfo)
{
    BT_RH_LOG("config_scan_param");
    INT32 i4_ret = 0;
    UINT8 scanner_id = bleScannerInfo->scan_param.scanner_id;
    //BT_BLE_SCANNER_REGULAR_SCAN_PARAM *regular_scan_param = NULL;
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *batch_scan_param = NULL;
    INT32 scan_interval = 0;
    INT32 scan_window = 0;
    UINT8 batch_scan_full_max = 0;
    UINT8 batch_scan_trunc_max = 0;
    UINT8 batch_scan_notify_threshold = 0;
    switch (bleScannerInfo->scan_param.scan_type) {
        case REGULAR_SCAN:
            //regular_scan_param = &bleScannerInfo->scan_param.scan_param.regular_scan_param;
            scan_interval = bleScannerInfo->scan_param.scan_param.regular_scan_param.scan_interval;
            scan_window = bleScannerInfo->scan_param.scan_param.regular_scan_param.scan_windows;
            //scan param
            i4_ret = x_mtkapi_bt_ble_scanner_set_scan_param(scanner_id, scan_interval, scan_window);
            if (i4_ret != 0)
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_set_scan_param fail");
            }
            break;
        case REGULAR_SCAN_WITH_FILT:
            //regular_scan_param = &bleScannerInfo->scan_param.scan_param.regular_scan_filt_param.regular_scan_param;
            scan_interval = bleScannerInfo->scan_param.scan_param.regular_scan_param.scan_interval;
            scan_window = bleScannerInfo->scan_param.scan_param.regular_scan_param.scan_windows;
            //scan param
            i4_ret = x_mtkapi_bt_ble_scanner_set_scan_param(scanner_id, scan_interval, scan_window);
            if (i4_ret != 0)
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_set_scan_param fail");
            }
            break;
        case BATCH_SCAN:
            batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_param;
            batch_scan_full_max = batch_scan_param->batch_scan_full_max;
            batch_scan_trunc_max = batch_scan_param->batch_scan_trunc_max;
            batch_scan_notify_threshold = batch_scan_param->batch_scan_notify_threshold;
            //batchscan_cfg_storage
            i4_ret = x_mtkapi_bt_ble_scanner_batchscan_cfg_storage(scanner_id, batch_scan_full_max, batch_scan_trunc_max,
                                                                        batch_scan_notify_threshold);
            if (i4_ret != 0)
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_batchscan_cfg_storage fail");
            }
            break;
        case BATCH_SCAN_WITH_FILT:
            batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_filt_param.batch_scan_param;
            batch_scan_full_max = batch_scan_param->batch_scan_full_max;
            batch_scan_trunc_max = batch_scan_param->batch_scan_trunc_max;
            batch_scan_notify_threshold = batch_scan_param->batch_scan_notify_threshold;
            //batchscan_cfg_storage
            i4_ret = x_mtkapi_bt_ble_scanner_batchscan_cfg_storage(scanner_id, batch_scan_full_max, batch_scan_trunc_max,
                                                                        batch_scan_notify_threshold);
            if (i4_ret != 0)
            {
                BT_RC_LOG("x_mtkapi_bt_ble_scanner_batchscan_cfg_storage fail");
            }
            break;
        default:
            BT_RC_LOG("unknown scan type");
            RPC_RETURN_VOID;
    }
}

//when app crash, ble scanner free scanner_id, cb, filter index and stop scan
static void bt_app_ble_scanner_ipc_close_notify(RPC_ID_T *t_rpc_id)
{
    BT_RC_LOG("bt_app_ble_scanner_ipc_close_notify tid = %d", *t_rpc_id);
    INT32 scanner_id = 0;
    INT32 i4_ret = 0;
    BLE_SCANNER_REG_CB_LIST* item_t = NULL;
    BLE_SCANNER_REG_CB_LIST *tmp = NULL;
    BLE_SCANNER_INFO *bleScannerInfo  = NULL;

    BT_MW_RPC_BLE_SCANNER_LOCK();
    dl_list_for_each_safe(item_t, tmp, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (item_t != NULL && item_t->ble_scanner_info != NULL)
        {
            if (*t_rpc_id == item_t->ble_scanner_info->t_id)
            {
                scanner_id = item_t->ble_scanner_info->scanner_id;
                bleScannerInfo = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
                if (bleScannerInfo == NULL)
                {
                    BT_MW_RPC_BLE_SCANNER_UNLOCK();
                    return;
                }
                remove_scan_filter(&bleScannerInfo);

                //stop scan -only stop the scanners of specfic app. if other app still scan, do not call stop scan api
                if (ble_scanner_list_get_regular_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
                {
                    i4_ret = ble_scanner_list_update_regular_scan_status(&ble_scanner_list, scanner_id, FALSE);
                    if (i4_ret != 0)
                    {
                        BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
                        BT_MW_RPC_BLE_SCANNER_UNLOCK();
                        return;
                    }
                    i4_ret =  ble_scanner_list_find_regular_scan_num(&ble_scanner_list);
                    if (star_scan_flag)
                    {
                        star_scan_flag = FALSE;
                    }
                    if (i4_ret == 0)
                    {
                        x_mtkapi_bt_ble_scanner_stop_scan();
                        if (scan_flag)
                        {
                            scan_flag = FALSE;
                        }
                    }
                }
                else if(ble_scanner_list_get_batch_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
                {
                    i4_ret = ble_scanner_list_update_batch_scan_status(&ble_scanner_list, scanner_id, FALSE);
                    if (i4_ret != 0)
                    {
                        BT_RC_LOG("ble_scanner_list_update_batch_scan_status fail!");
                        BT_MW_RPC_BLE_SCANNER_UNLOCK();
                        return;
                    }
                    i4_ret =  ble_scanner_list_find_batch_scan_num(&ble_scanner_list);
                    if (star_scan_flag)
                    {
                        star_scan_flag = FALSE;
                    }
                    if (i4_ret == 0)
                    {
                        x_mtkapi_bt_ble_scanner_batchscan_disable(scanner_id);
                        if (scan_flag)
                        {
                            scan_flag = FALSE;
                        }
                        util_ble_scanner_timer_stop(batchscan_report_timer.handle);
                    }
                }

                x_mtkapi_bt_ble_scanner_unregister(scanner_id);
                ble_scanner_list_remove_by_scanner_id(&ble_scanner_list, scanner_id);
            }
        }
    }
    free(t_rpc_id);
    BT_MW_RPC_BLE_SCANNER_UNLOCK();
    return;
}

//when bt off, ble scanner free ble scanner cb list
VOID bt_app_ble_scanner_free_cbs(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RC_LOG("bt_app_ble_scanner_free_cbs");
    BLE_SCANNER_REG_CB_LIST* item_t = NULL;
    while (!dl_list_empty(&ble_scanner_list))
    {
        item_t = dl_list_first(&ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node);
        if (NULL != item_t)
        {
            if (NULL != item_t->ble_scanner_info)
            {
                memset(total_filt_indexs, 0, sizeof(total_filt_indexs));
                free(item_t->ble_scanner_info);
                item_t->ble_scanner_info = NULL;
            }
            dl_list_del(&item_t->node);
            free(item_t);
            item_t = NULL;
        }
    }
    return;
}

static VOID bt_app_ble_scanner_batchscan_report_timer_proc(UINT8 timer_id, VOID *pv_args)
{
    BT_RH_LOG("[.c]bt_app_ble_scanner_batchscan_report_timer_proc");
    INT32 i4_ret = 0;
    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(
                                                  &ble_scanner_list, timer_id);

    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        return;
    }
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *batch_scan_param = NULL;
    if (bleScannerInfo->scan_param.scan_type == BATCH_SCAN)
    {
        batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_param;
    }
    else if (bleScannerInfo->scan_param.scan_type == BATCH_SCAN_WITH_FILT)
    {
        batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_filt_param.batch_scan_param;
    }
    else
    {
        BT_RC_LOG("not batch scan");
        return;
    }
    i4_ret = x_mtkapi_bt_ble_scanner_batchscan_read_reports(timer_id, batch_scan_param->batch_scan_read_report_mode);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c]x_mtkapi_bt_ble_scanner_batchscan_read_reports fail!");
    }
}



static VOID bt_app_ble_scanner_register_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_register_cbk_agent");
    if (ble_scanner_list_update_scanner_id(&ble_scanner_list, cb_param))
    {
        BT_RH_LOG("BT_BLE_SCANNER_EVT_REGISTER, update scanner_id success");
    }
    else
    {
        BT_RC_LOG("BT_BLE_SCANNER_EVT_REGISTER, update scanner_id fail");
        return;
    }

    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, cb_param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, cb_param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (0 == strncmp(cb_param->data.register_data.app_uuid, item_t->ble_scanner_info->scanner_uuid,
                         strlen(item_t->ble_scanner_info->scanner_uuid)))
        {
            if (NULL != item_t->ble_scanner_info->app_func)
            {
                pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
                pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
                RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
            }
        }
    }
}

static VOID bt_app_ble_scanner_filt_enable_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RH_LOG("bt_app_ble_scanner_filt_enable_cbk_agent");
    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(
                                                  &ble_scanner_list, param->scanner_id);

    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        return;
    }
    if (star_scan_flag == TRUE)
    {
        memset(bleScannerInfo->filt_indexs, 0, sizeof(BT_BLE_SCANNER_FILT_INDEX));
        config_add_filt_data(&bleScannerInfo);
    }
}


static VOID bt_app_ble_scanner_filt_add_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RH_LOG("bt_app_ble_scanner_filt_add_cbk_agent");
    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(
                                                  &ble_scanner_list, param->scanner_id);
    UINT8 scanner_id = param->scanner_id;
    UINT32 k = 0;
    k = scanner_id - 1;
    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        return;
    }
    if ((bleScannerInfo->scan_param.scan_type == REGULAR_SCAN_WITH_FILT && regular_scan_filter_add_index[k] != -1)
        || (bleScannerInfo->scan_param.scan_type == BATCH_SCAN_WITH_FILT && batch_scan_filter_add_index[k] != -1))
    {
        config_filt_param(bleScannerInfo);
    }
    else
    {
        BT_RC_LOG("scan_type = %d, regular_scan_filter_add_index = %d, batch_scan_filter_add_index = %d",
              bleScannerInfo->scan_param.scan_type, regular_scan_filter_add_index[k], batch_scan_filter_add_index[k]);
    }
}

static VOID bt_app_ble_scanner_filt_param_setup_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *cb_param)
{
    BT_RH_LOG("bt_app_ble_scanner_filt_param_setup_cbk_agent");
    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(
                                                  &ble_scanner_list, cb_param->scanner_id);
    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        return;
    }

    if (cb_param->data.filt_param_setup_data.action != APCF_ACTION_ADD)
    {
        return;
    }

    if (config_add_filt_data(&bleScannerInfo) == CONFIG_ADD_FILTE_DATA_DONE)
    {
        config_scan_param(bleScannerInfo);
    }
}

static VOID bt_app_ble_scanner_scan_param_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RH_LOG("bt_app_ble_scanner_scan_param_cbk_agent");
    int i4_ret = 0;
    if (scan_flag == FALSE)
    {
        i4_ret = x_mtkapi_bt_ble_scanner_start_scan();
        if (i4_ret == 0)
        {
            BT_MW_RPC_BLE_SCANNER_LOCK();
            scan_flag = TRUE;
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
        }
        else
        {
            BT_RC_LOG("x_mtkapi_bt_ble_scanner_start_scan fail");
        }
    }
}


static VOID bt_app_ble_scanner_batchscan_config_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RH_LOG("bt_app_ble_scanner_batchscan_config_cbk_agent");
    int i4_ret = 0;
    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(
                                                  &ble_scanner_list, param->scanner_id);
    BT_BLE_SCANNER_BATCH_SCAN_PARAM *batch_scan_param = NULL;
    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        return;
    }
    if (bleScannerInfo->scan_param.scan_type == BATCH_SCAN)
    {
        batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_param;
    }
    else if (bleScannerInfo->scan_param.scan_type == BATCH_SCAN_WITH_FILT)
    {
        batch_scan_param = &bleScannerInfo->scan_param.scan_param.batch_scan_filt_param.batch_scan_param;
    }
    else
    {
        BT_RC_LOG("scan type is not batch scan");
        return;
    }

    UINT8 scanner_id = bleScannerInfo->scanner_id;
    UINT8 batch_scan_mode = batch_scan_param->batch_scan_mode;
    INT32 batch_scan_windows = batch_scan_param->batch_scan_windows;
    INT32 batch_scan_interval = batch_scan_param->batch_scan_interval;
    UINT8 own_address_type = batch_scan_param->own_address_type;
    UINT8 batch_scan_discard_rule = batch_scan_param->batch_scan_discard_rule;
    //UINT8 batch_scan_read_report_mode = batch_scan_param->batch_scan_read_report_mode;
    //batchscan enable
    i4_ret = x_mtkapi_bt_ble_scanner_batchscan_enable(scanner_id, batch_scan_mode, batch_scan_interval,
                                  batch_scan_windows, own_address_type, batch_scan_discard_rule);
    if (i4_ret != 0)
    {
        BT_RC_LOG("x_mtkapi_bt_ble_scanner_batchscan_enable fail");
    }
}


static VOID bt_app_ble_scanner_batchscan_enable_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param)
{
    BT_RH_LOG("bt_app_ble_scanner_batchscan_enable_cbk_agent");
    // Timer
    INT32 scanner_id = param->scanner_id;
    batchscan_report_timer.id = scanner_id;
    util_ble_scanner_timer_start(&batchscan_report_timer);
}

static VOID bt_app_ble_scanner_regular_scan_result_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_regular_scan_result_cbk_agent");

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && item_t->ble_scanner_info->regular_scan_status)
        {
            if (NULL != item_t->ble_scanner_info->app_func)
            {
                RPC_DECL_VOID(2);
                RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
                param->scanner_id = item_t->ble_scanner_info->scanner_id;
                RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
                    RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
                RPC_ARG_IO(ARG_TYPE_REF_DESC, param);
                RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
                pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
                pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
                RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
            }
        }
    }
}


static VOID bt_app_ble_scanner_batch_scan_report_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_batch_scan_report_cbk_agent");
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && item_t->ble_scanner_info->batch_scan_status)
        {
            if (NULL != item_t->ble_scanner_info->app_func)
            {
                pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
                pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
                RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
            }
        }
    }
}


static VOID bt_app_ble_scanner_track_adv_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_track_adv_cbk_agent");
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && item_t->ble_scanner_info->regular_scan_status)
        {
            if (NULL != item_t->ble_scanner_info->app_func)
            {
                pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
                pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
                RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
            }
        }
    }
}


static VOID bt_app_ble_scanner_batchscan_disable_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_batchscan_disable_cbk_agent");
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && NULL != item_t->ble_scanner_info->app_func)
        {
            pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
            pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
            RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
        }
    }
}


static VOID bt_app_ble_scanner_batchscan_threshold_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_batchscan_threshold_cbk_agent");
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && NULL != item_t->ble_scanner_info->app_func)
        {
            pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
            pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
            RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
        }
    }
}


static VOID bt_app_ble_scanner_filt_clear_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    BT_RH_LOG("bt_app_ble_scanner_filt_clear_cbk_agent");
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_BLE_SCANNER_CALLBACK_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BLE_SCANNER_REG_CB_LIST* item_t = NULL;

    dl_list_for_each(item_t, &ble_scanner_list, BLE_SCANNER_REG_CB_LIST, node)
    {
        if (NULL != item_t->ble_scanner_info && NULL != item_t->ble_scanner_info->app_func)
        {
            pt_nfy_tag->t_id = item_t->ble_scanner_info->t_id;
            pt_nfy_tag->pv_cb_addr = item_t->ble_scanner_info->app_func;
            RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_ble_scanner_event_cbk", pt_nfy_tag->pv_cb_addr);
        }
    }
}


static VOID bt_app_ble_scanner_event_cbk_agent(BT_BLE_SCANNER_CALLBACK_PARAM *param, void* pv_tag)
{
    if (NULL == param)
    {
        BT_RH_LOG("bt_app_ble_scanner_event_cbk_agent, Invalid parameter.");
        return;
    }

    BT_RH_LOG("bt_app_ble_scanner_event_cbk_agent, event is %d", param->event);

    switch (param->event) {
        case BT_BLE_SCANNER_EVT_FILT_ENABLE:  // filt enable callback
            bt_app_ble_scanner_filt_enable_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_FILT_ADD:  // filt add callback
            bt_app_ble_scanner_filt_add_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_FILT_CLEAR:  // filt clear callback
            bt_app_ble_scanner_filt_clear_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_FILT_PARAM_SETUP:  // filt param setup callback
            bt_app_ble_scanner_filt_param_setup_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_SCAN_PARAM_SETUP:  // scan param callback
            bt_app_ble_scanner_scan_param_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_CONFIG:  // batchscan config storage callback
            bt_app_ble_scanner_batchscan_config_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_ENABLE:  // batchscan enable callback
            bt_app_ble_scanner_batchscan_enable_cbk_agent(param);
            break;
        case BT_BLE_SCANNER_EVT_REGISTER:  // register callback
            bt_app_ble_scanner_register_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_SCAN_RESULT:  // regular scan callback
            bt_app_ble_scanner_regular_scan_result_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_REPORT: // batch scan report callback
            bt_app_ble_scanner_batch_scan_report_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_TRACK_ADV: // on_Found_Lost callback
            bt_app_ble_scanner_track_adv_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_DISABLE:   //batchscan disable callbak
            bt_app_ble_scanner_batchscan_disable_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_BATCHSCAN_THRESHOLD: //batchscan threshold callbak
            bt_app_ble_scanner_batchscan_threshold_cbk_agent(param, pv_tag);
            break;
        case BT_BLE_SCANNER_EVT_BT_OFF: //ble scanner bt off event
            bt_app_ble_scanner_free_cbs(param);
            break;
        default:
          BT_RC_LOG("bt_app_ble_scanner_event_cbk_agent, Unknown event.");
          RPC_RETURN_VOID;
    }
    RPC_RETURN_VOID;
}


static INT32 _hndlr_x_mtkapi_bt_ble_scanner_register(RPC_ID_T     t_rpc_id,
                                                        const CHAR*  ps_cb_type,
                                                        UINT32       ui4_num_args,
                                                        ARG_DESC_T*  pt_args,
                                                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_ble_scanner_register");

    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = 0;
    RPC_ID_T *t_rpc_id_cb;
    t_rpc_id_cb = (RPC_ID_T *)malloc(sizeof(RPC_ID_T));
    if (t_rpc_id_cb == NULL)
    {
        return RPCR_ERROR;
    }
    *t_rpc_id_cb = t_rpc_id;
    INT32 i4_ret;

    /*******************************************************************/
    /*       The first server need create pGattsLinkList when register callback           */
    /*******************************************************************/
    if (dl_list_empty(&ble_scanner_list)) {
        memset(total_filt_indexs, 0, sizeof(total_filt_indexs));
        memset(app_filters, 0, sizeof(app_filters));
        for (int i = 0; i < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM; i++)
        {
            total_filt_indexs[i].filter_index = i;
        }
        RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
        VOID * apv_cb_addr[1] = {0};
        mtkrpcapi_BtAppBleScannerCbk bt_app_cb_func = bt_app_ble_scanner_event_cbk_agent;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, pt_args[1].u.pv_arg);
        i4_ret = x_mtkapi_bt_ble_scanner_register_callback(bt_app_cb_func, pt_nfy_tag);
        if (i4_ret && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }

    /*******************************************************************/
    /*        Add ble_scanner callback to  pBleScannerLinkList                                 */
    /*******************************************************************/
    mtkrpcapi_BtAppBleScannerCbk* ble_scanner_app_cb_func = (mtkrpcapi_BtAppBleScannerCbk*)pt_args[0].u.pv_desc;
    BLE_SCANNER_INFO* p_ble_scanner_info = (BLE_SCANNER_INFO*)malloc(sizeof(BLE_SCANNER_INFO));
    if (NULL == p_ble_scanner_info)
    {
        BT_RC_LOG("malloc BLE_SCANNER_INFO fail.");
        free(t_rpc_id_cb);
        return RPCR_ERROR;
    }
    memset(p_ble_scanner_info, 0, sizeof(BLE_SCANNER_INFO));
    p_ble_scanner_info->t_id = t_rpc_id;
    p_ble_scanner_info->scanner_id = -1;
    p_ble_scanner_info->regular_scan_status = FALSE;
    p_ble_scanner_info->batch_scan_status = FALSE;
    memset(p_ble_scanner_info->filt_indexs, 0, sizeof(p_ble_scanner_info->filt_indexs));
    // generate random uuid
    CHAR app_uuid[BT_BLE_SCANNER_MAX_UUID_LEN] = {0};
    do
    {
        generate_random_uuid(app_uuid);
    } while (ble_scanner_list_check_uuid_exist(&ble_scanner_list, app_uuid));

    strncpy(p_ble_scanner_info->scanner_uuid, app_uuid, strlen(app_uuid));
    memset(&(p_ble_scanner_info->scan_param), 0, sizeof(BT_BLE_SCANNER_START_SCAN_PARAM));

    if (NULL != ble_scanner_app_cb_func)
    {
        p_ble_scanner_info->app_func = ble_scanner_app_cb_func;
    }

    BLE_SCANNER_REG_CB_LIST* item = (BLE_SCANNER_REG_CB_LIST*)malloc(sizeof(BLE_SCANNER_REG_CB_LIST));
    if (NULL == item)
    {
        BT_RC_LOG("malloc BLE_SCANNER_REG_CB_LIST fail.");
        free(p_ble_scanner_info);
        free(t_rpc_id_cb);
        return RPCR_ERROR;
    }
    memset(item, 0, sizeof(BLE_SCANNER_REG_CB_LIST));
    item->ble_scanner_info = p_ble_scanner_info;
    i4_ret = ble_scanner_list_add(&ble_scanner_list, item);
    if (i4_ret != 0)
    {
        if (NULL != p_ble_scanner_info)
        {
            free(p_ble_scanner_info);
        }
        if (NULL != item)
        {
            free(item);
        }
        if (NULL != t_rpc_id_cb)
        {
            free(t_rpc_id_cb);
        }
        BT_RC_LOG("ble_scanner register callback fail!");
        return RPCR_ERROR;
    }
    else
    {
        BT_RC_LOG("bt_register_app_ipc_close_handle t_rpc_id=%d t_rpc_id_for_free=%d",
                 t_rpc_id , *t_rpc_id_cb);
        bt_register_app_ipc_close_handle(t_rpc_id, t_rpc_id_cb, (ipc_close_fct)bt_app_ble_scanner_ipc_close_notify);
    }

    ble_scanner_list_show(&ble_scanner_list);

    /*******************************************************************/
    /*        register scanner_id                                                                          */
    /*******************************************************************/
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_register(app_uuid);
    return RPCR_OK;
}


static BOOL check_start_scan_param(BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param)
{
    UINT32 scanner_id = 0;
    scanner_id = start_scan_param->scanner_id;
    if (scanner_id <= 0)
    {
        BT_RC_LOG("scanner_id should > 0\n");
        return FALSE;
    }
    if (start_scan_param->scan_type == REGULAR_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_REGULAR_SCAN_WITH_FILT_PARAM *regular_scan_filt_param
            = &start_scan_param->scan_param.regular_scan_filt_param;
        int size = regular_scan_filt_param->scan_filt_num < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM
                   ? regular_scan_filt_param->scan_filt_num : BT_BLE_SCANNER_MAX_SCAN_FILT_NUM;
        for (int i = 0; i < size; i ++)
        {

            BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data = &(regular_scan_filt_param->scan_filt_data)[i];
            //check bt_addr
            if (strlen(scan_filt_data->bt_addr)> 0 && !is_valid_address(scan_filt_data->bt_addr))
            {
                BT_RC_LOG("scan_filt_data bt_addr is invaild");
                return FALSE;
            }
            //check srvc_uuid
            if (strlen(scan_filt_data->srvc_uuid)> 0 && !is_valid_uuid(scan_filt_data->srvc_uuid))
            {
                BT_RC_LOG("scan_filt_data srvc_uuid is invaild");
                return FALSE;
            }
            //check srvc_uuid_mask
            if (strlen(scan_filt_data->srvc_uuid_mask)> 0 && !is_valid_uuid(scan_filt_data->srvc_uuid_mask))
            {
                BT_RC_LOG("scan_filt_data srvc_uuid_mask is invaild");
                return FALSE;
            }
            //check srvc_sol_uuid
            if (strlen(scan_filt_data->srvc_sol_uuid)> 0 && !is_valid_uuid(scan_filt_data->srvc_sol_uuid))
            {
                BT_RC_LOG("scan_filt_data srvc_sol_uuid is invaild");
                return FALSE;
            }
            //check srvc_uuid_mask
            if (strlen(scan_filt_data->srvc_sol_uuid_mask)> 0 && !is_valid_uuid(scan_filt_data->srvc_sol_uuid_mask))
            {
                BT_RC_LOG("scan_filt_data srvc_sol_uuid_mask is invaild");
                return FALSE;
            }
            //compare manu_data and manu_data_mask length
            if ((memcmp(scan_filt_data->manu_data, scan_filt_data->manu_data_mask, BT_BLE_SCANNER_MAX_DATA_LEN) != 0) ||
                (scan_filt_data->manu_data_len > BT_BLE_SCANNER_MAX_DATA_LEN))
            {
                BT_RC_LOG("scan_filt_data manu_data and manu_data_mask diff len or data_len too long");
                return FALSE;
            }
            //compare srvc_data and srvc_data_mask length
            if ((memcmp(scan_filt_data->srvc_data, scan_filt_data->srvc_data_mask, BT_BLE_SCANNER_MAX_DATA_LEN)) != 0 ||
                (scan_filt_data->srvc_data_len > BT_BLE_SCANNER_MAX_DATA_LEN))
            {
                BT_RC_LOG("scan_filt_data srvc_data and srvc_data_mask diff len or data_len too long");
                return FALSE;
            }
        }

    }
    else if (start_scan_param->scan_type == BATCH_SCAN_WITH_FILT)
    {
        BT_BLE_SCANNER_BATCH_SCAN_WITH_FILT_PARAM *batch_scan_filt_param
            = &start_scan_param->scan_param.batch_scan_filt_param;
        int size = batch_scan_filt_param->scan_filt_num < BT_BLE_SCANNER_MAX_SCAN_FILT_NUM
                   ? batch_scan_filt_param->scan_filt_num : BT_BLE_SCANNER_MAX_SCAN_FILT_NUM;
        for (int i = 0; i < size; i ++)
        {
            BT_BLE_SCANNER_SCAN_FILT_DATA *scan_filt_data = &(batch_scan_filt_param->scan_filt_data)[i];
            //check bt_addr
            if (strlen(scan_filt_data->bt_addr)> 0 && !is_valid_address(scan_filt_data->bt_addr))
            {
                BT_RC_LOG("scan_filt_data bt_addr is invaild");
                return FALSE;
            }
            //check srvc_uuid
            if (strlen(scan_filt_data->srvc_uuid)> 0 && !is_valid_uuid(scan_filt_data->srvc_uuid))
            {
                BT_RC_LOG("scan_filt_data srvc_uuid is invaild");
                return FALSE;
            }
            //check srvc_uuid_mask
            if (strlen(scan_filt_data->srvc_uuid_mask)> 0 && !is_valid_uuid(scan_filt_data->srvc_uuid_mask))
            {
                BT_RC_LOG("scan_filt_data srvc_uuid_mask is invaild");
                return FALSE;
            }
            //check srvc_sol_uuid
            if (strlen(scan_filt_data->srvc_sol_uuid)> 0 && !is_valid_uuid(scan_filt_data->srvc_sol_uuid))
            {
                BT_RC_LOG("scan_filt_data srvc_sol_uuid is invaild");
                return FALSE;
            }
            //check srvc_uuid_mask
            if (strlen(scan_filt_data->srvc_sol_uuid_mask)> 0 && !is_valid_uuid(scan_filt_data->srvc_sol_uuid_mask))
            {
                BT_RC_LOG("scan_filt_data srvc_sol_uuid_mask is invaild");
                return FALSE;
            }
            //compare manu_data and manu_data_mask length
            if ((memcmp(scan_filt_data->manu_data, scan_filt_data->manu_data_mask, BT_BLE_SCANNER_MAX_DATA_LEN) != 0) ||
                (scan_filt_data->manu_data_len > BT_BLE_SCANNER_MAX_DATA_LEN))
            {
                BT_RC_LOG("scan_filt_data manu_data and manu_data_mask diff len or data_len too long");
                return FALSE;
            }
            //compare srvc_data and srvc_data_mask length
            if ((memcmp(scan_filt_data->srvc_data, scan_filt_data->srvc_data_mask, BT_BLE_SCANNER_MAX_DATA_LEN)) != 0 ||
                (scan_filt_data->srvc_data_len > BT_BLE_SCANNER_MAX_DATA_LEN))
            {
                BT_RC_LOG("scan_filt_data srvc_data and srvc_data_mask diff len or data_len too long");
                return FALSE;
            }
        }
    }
    return TRUE;
}


static INT32 _hndlr_x_mtkapi_bt_ble_scanner_start_scan(RPC_ID_T     t_rpc_id,
                                                        const CHAR*  ps_cb_type,
                                                        UINT32       ui4_num_args,
                                                        ARG_DESC_T*  pt_args,
                                                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_ble_start_scan");
    if (ui4_num_args != 1)
    {
        BT_RC_LOG("Invalid ARGS: %d\n", ui4_num_args);
        pt_return->u.i4_arg = RPCR_INV_ARGS;
        return RPCR_INV_ARGS;
    }

    BT_BLE_SCANNER_START_SCAN_PARAM *start_scan_param =
        (BT_BLE_SCANNER_START_SCAN_PARAM *)pt_args[0].u.pv_desc;

    if (!check_start_scan_param(start_scan_param))
    {
        BT_RC_LOG("start scan param invalid");
        pt_return->u.i4_arg = RPCR_INV_ARGS;
        return RPCR_INV_ARGS;
    }

    INT8 scanner_id = start_scan_param->scanner_id;
    if (!ble_scanner_list_check_scanner_id_exist(&ble_scanner_list, scanner_id))
    {
        BT_RC_LOG("scanner_id = %d not register", scanner_id);
        pt_return->u.i4_arg = RPCR_OK;
        return RPCR_OK;
    }

    UINT32 i4_ret = 0;

    if (ble_scanner_list_get_regular_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        BT_RC_LOG("app scanner_id = %d regular scanning", scanner_id);
        pt_return->u.i4_arg = RPCR_OK;
        return RPCR_OK;
    }

    if (ble_scanner_list_get_batch_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        BT_RC_LOG("app scanner_id = %d batch scanning", scanner_id);
        pt_return->u.i4_arg = RPCR_OK;
        return RPCR_OK;
    }

    i4_ret = ble_scanner_list_update_scan_param(&ble_scanner_list, start_scan_param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("ble_scanner_list_update_scan_param fail!");
    }

    BLE_SCANNER_INFO *bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
    pt_return->e_type   = ARG_TYPE_INT32;
    if (NULL == bleScannerInfo)
    {
        BT_RC_LOG("bleScannerInfo is NULL");
        pt_return->u.i4_arg = RPCR_ERROR;
        return RPCR_ERROR;
    }
    BT_MW_RPC_BLE_SCANNER_LOCK();
    switch (start_scan_param->scan_type) {
        case REGULAR_SCAN:
            BT_RH_LOG("[_hndlr_]bt_ble_start_scan: regular scan");
            i4_ret = ble_scanner_list_update_regular_scan_status(&ble_scanner_list, scanner_id, TRUE);
            if (i4_ret != 0)
            {
                BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            }

            i4_ret = ble_scanner_list_find_regular_scan_num(&ble_scanner_list);
            if (i4_ret > 0)
            {
                if (!star_scan_flag)
                {
                    star_scan_flag = TRUE;
                }
                //config regular scan param
                config_scan_param(bleScannerInfo);
                pt_return->u.i4_arg = RPCR_OK;
            }
            else
            {
                pt_return->u.i4_arg = RPCR_OK;
                BT_RH_LOG("ble regular scanning");
            }
            break;
        case REGULAR_SCAN_WITH_FILT:
            BT_RH_LOG("[_hndlr_]bt_ble_start_scan: regular scan with filt");

            i4_ret = ble_scanner_list_update_regular_scan_status(&ble_scanner_list, scanner_id, TRUE);
            if (i4_ret != 0)
            {
                BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            }

            i4_ret = ble_scanner_list_find_regular_scan_num(&ble_scanner_list);
            if (i4_ret > 0)
            {
                if (!star_scan_flag)
                {
                    star_scan_flag = TRUE;
                }
                x_mtkapi_bt_ble_scanner_scan_filter_enable(scanner_id, TRUE);
                pt_return->u.i4_arg = RPCR_OK;
            }
            else
            {
                pt_return->u.i4_arg = RPCR_OK;
                BT_RC_LOG("ble regular scanning");
            }

            break;
        case BATCH_SCAN:
            BT_RH_LOG("[_hndlr_]bt_ble_start_scan: batch scan");

            i4_ret = ble_scanner_list_update_batch_scan_status(&ble_scanner_list, scanner_id, TRUE);
            if (i4_ret != 0)
            {
                BT_RC_LOG("ble_scanner_list_update_batch_scan_status fail!");
            }

            i4_ret = ble_scanner_list_find_batch_scan_num(&ble_scanner_list);
            if (i4_ret > 0)
            {
                if (!star_scan_flag)
                {
                    star_scan_flag = TRUE;
                }
                //config regular scan param
                config_scan_param(bleScannerInfo);
                pt_return->u.i4_arg = RPCR_OK;
            }
            else
            {
                pt_return->u.i4_arg = RPCR_OK;
                BT_RC_LOG("ble batch scanning");
            }
            break;
        case BATCH_SCAN_WITH_FILT:
            BT_RH_LOG("[_hndlr_]bt_ble_start_scan: batch scan with filt");

            i4_ret = ble_scanner_list_update_batch_scan_status(&ble_scanner_list, scanner_id, TRUE);
            if (i4_ret != 0)
            {
                BT_RC_LOG("ble_scanner_list_update_batch_scan_status fail!");
            }
            i4_ret = ble_scanner_list_find_batch_scan_num(&ble_scanner_list);
            if (i4_ret > 0)
            {
                if (!star_scan_flag)
                {
                    star_scan_flag = TRUE;
                }
                x_mtkapi_bt_ble_scanner_scan_filter_enable(scanner_id, TRUE);
                pt_return->u.i4_arg = RPCR_OK;
            }
            else
            {
                pt_return->u.i4_arg = RPCR_OK;
                BT_RC_LOG("ble batch scanning");
            }
            break;
        default:
            BT_RC_LOG("Unknown scan_type");
            pt_return->u.i4_arg = RPCR_INV_ARGS;
    }
    BT_MW_RPC_BLE_SCANNER_UNLOCK();
    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_ble_scanner_stop_scan(RPC_ID_T     t_rpc_id,
                                                        const CHAR*  ps_cb_type,
                                                        UINT32       ui4_num_args,
                                                        ARG_DESC_T*  pt_args,
                                                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_ble_scanner_stop_scan");
    UINT32 i = 0;
    if (ui4_num_args != 1)
    {
        BT_RC_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_OK;
    }
    BT_BLE_SCANNER_STOP_SCAN_PARAM *stop_scan_param =
        (BT_BLE_SCANNER_STOP_SCAN_PARAM *)pt_args[0].u.pv_desc;
    UINT32 scanner_id = stop_scan_param->scanner_id;
    i = scanner_id - 1;
    if (!ble_scanner_list_check_scanner_id_exist(&ble_scanner_list, scanner_id))
    {
        BT_RC_LOG("scanner_id = %d not register", scanner_id);
        return RPCR_OK;
    }

    BLE_SCANNER_INFO *bleScannerInfo  = NULL;
    pt_return->e_type   = ARG_TYPE_INT32;
    INT32 i4_ret = 0;
    BT_MW_RPC_BLE_SCANNER_LOCK();
    if (ble_scanner_list_get_regular_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        i4_ret = ble_scanner_list_update_regular_scan_status(&ble_scanner_list, scanner_id, FALSE);
        if (i4_ret != 0)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
        if (NULL == bleScannerInfo)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        i4_ret =  ble_scanner_list_find_regular_scan_num(&ble_scanner_list);
        if (star_scan_flag)
        {
            star_scan_flag = FALSE;
        }
        add_filt_data_flag[i] = CONFIG_ADD_FILTE_DATA_FAIL;
        //removeScanFilters
        remove_scan_filter(&bleScannerInfo);
        x_mtkapi_bt_ble_scanner_scan_filter_enable(scanner_id, FALSE);
        if (i4_ret == 0)
        {
            pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_stop_scan();
            if (scan_flag)
            {
                scan_flag = FALSE;
            }
        }
    }
    else if(ble_scanner_list_get_batch_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        i4_ret = ble_scanner_list_update_batch_scan_status(&ble_scanner_list, scanner_id, FALSE);
        if (i4_ret != 0)
        {
            BT_RC_LOG("ble_scanner_list_update_batch_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
        if (NULL == bleScannerInfo)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        i4_ret =  ble_scanner_list_find_batch_scan_num(&ble_scanner_list);
        add_filt_data_flag[i] = CONFIG_ADD_FILTE_DATA_FAIL;
        if (star_scan_flag)
        {
            star_scan_flag = FALSE;
        }
        //removeScanFilters
        remove_scan_filter(&bleScannerInfo);
        if (i4_ret == 0)
        {
            pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_batchscan_disable(scanner_id);
            if (scan_flag)
            {
                scan_flag = FALSE;
            }
            util_ble_scanner_timer_stop(batchscan_report_timer.handle);
        }
    }
    else
    {
        BT_RC_LOG("app scanner_id = %d is not scan status", scanner_id);
        pt_return->u.i4_arg = 0;
    }
    BT_MW_RPC_BLE_SCANNER_UNLOCK();
    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_ble_scanner_unregister(RPC_ID_T     t_rpc_id,
                                                        const CHAR*  ps_cb_type,
                                                        UINT32       ui4_num_args,
                                                        ARG_DESC_T*  pt_args,
                                                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_ble_scanner_unregister");
    UINT32 i = 0;
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    BT_BLE_SCANNER_UNREG_PARAM *scanner_unreg =
        (BT_BLE_SCANNER_UNREG_PARAM *)pt_args[0].u.pv_desc;
    INT32 scanner_id = scanner_unreg->scanner_id;
    i = scanner_id - 1;
    if (dl_list_empty(&ble_scanner_list))
    {
        BT_RC_LOG("ble_scanner_list is empty");
        return RPCR_OK;
    }

    if (!ble_scanner_list_check_scanner_id_exist(&ble_scanner_list, scanner_id))
    {
        BT_RC_LOG("scanner_id = %d not register", scanner_id);
        return RPCR_OK;
    }

    BLE_SCANNER_INFO *bleScannerInfo  = NULL;
    INT32 i4_ret = 0;
    BT_MW_RPC_BLE_SCANNER_LOCK();
    if (ble_scanner_list_get_regular_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        i4_ret = ble_scanner_list_update_regular_scan_status(&ble_scanner_list, scanner_id, FALSE);
        if (i4_ret != 0)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
        if (NULL == bleScannerInfo)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        i4_ret =  ble_scanner_list_find_regular_scan_num(&ble_scanner_list);

        if (agent_regular_scan_flag && i4_ret == 0)
        {
            agent_regular_scan_flag = FALSE;
            //removeScanFilters
            remove_scan_filter(&bleScannerInfo);
            pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_stop_scan();
        }
    }
    if (ble_scanner_list_get_batch_scan_status_by_scanner_id(&ble_scanner_list, scanner_id))
    {
        i4_ret = ble_scanner_list_update_batch_scan_status(&ble_scanner_list, scanner_id, FALSE);
        if (i4_ret != 0)
        {
            BT_RC_LOG("ble_scanner_list_update_batch_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        bleScannerInfo  = ble_scanner_list_find_scannerinfo_by_scanner_id(&ble_scanner_list, scanner_id);
        if (NULL == bleScannerInfo)
        {
            BT_RC_LOG("ble_scanner_list_update_regular_scan_status fail!");
            BT_MW_RPC_BLE_SCANNER_UNLOCK();
            return RPCR_OK;
        }

        i4_ret =  ble_scanner_list_find_batch_scan_num(&ble_scanner_list);

        if (i < BT_BLE_SCANNER_MAX_REG_NUM)
        {
            if (agent_batch_scan_flag && i4_ret == 0)
            {
                agent_batch_scan_flag = FALSE;
                pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_batchscan_disable(scanner_id);

                //removeScanFilters
                remove_scan_filter(&bleScannerInfo);
                util_ble_scanner_timer_stop(batchscan_report_timer.handle);
            }
       }
    }
    ble_scanner_list_remove_by_scanner_id(&ble_scanner_list, scanner_id);

    ble_scanner_list_show(&ble_scanner_list);

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_ble_scanner_unregister(scanner_id);
    BT_MW_RPC_BLE_SCANNER_UNLOCK();
    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_ble_scanner_op_hndlrs(VOID)
{
    ble_scanner_mutex_init();
    RPC_REG_OP_HNDLR(x_mtkapi_bt_ble_scanner_register);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_ble_scanner_start_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_ble_scanner_stop_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_ble_scanner_unregister);
    return RPCR_OK;
}
