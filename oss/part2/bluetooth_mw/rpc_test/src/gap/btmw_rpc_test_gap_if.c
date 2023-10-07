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
#include <string.h>
#include <unistd.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_gap_if.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_hidh_wrapper.h"
#include "btmw_rpc_test_leaudio_bass_if.h"

#define BT_NAME_SUF_RPC_TEST_LEN              5

#define BTMW_RPC_TEST_DEFAULT_NAME	"Linux_Q"

MTKRPCAPI_BT_APP_CB_FUNC g_gap_func;
char g_gap_pv_tag[2] = {0};

typedef struct
{
    CHAR            bdAddr[MAX_BDADDR_LEN]; /* Bluetooth Address */
    CHAR            name[MAX_NAME_LEN];     /* Name of device */
    UINT32          cod;
    UINT32          service;                /* uuids in EIR */
    int             valid;
    int             need_print;
} INQ_RESULT;

#define INQ_RESULT_MAX 100
static INQ_RESULT inq_result[INQ_RESULT_MAX] = {0};
static int inq_max_dev = INQ_RESULT_MAX;
static UINT32 inq_times = 0;
static UINT32 inq_filter_type = 0;

typedef struct
{
    CHAR            bdAddr[MAX_BDADDR_LEN]; /* Bluetooth Address */
    int             bonded;
    int             connected;                /* uuids in EIR */
} CONN_DEV;
#define CONN_MAX 16
static CONN_DEV conn_dev[CONN_MAX] = {0};

extern void btmw_rpc_test_bms_clear();

static void btmw_rpc_update_conn_dev(CHAR *addr,
                                int bonded, int connected)
{
    unsigned int index, update_index = CONN_MAX;

    for (index = 0; index < CONN_MAX; index++)
    {
        if (strlen(conn_dev[index].bdAddr) == 0)
        {
            if (update_index == CONN_MAX)
                update_index = index;
        }
        else
        {
            if (strncmp(conn_dev[index].bdAddr, addr, strlen(conn_dev[index].bdAddr)) == 0)
            {
                update_index = index;
                break;
            }
        }
    }

    if (update_index < CONN_MAX)
    {
        if (strlen(conn_dev[update_index].bdAddr) == 0)
        {
            strncpy(conn_dev[update_index].bdAddr, addr, sizeof(conn_dev[update_index].bdAddr));
            conn_dev[update_index].bdAddr[MAX_BDADDR_LEN - 1] = '\0';
        }
        if (bonded != -1)
            conn_dev[update_index].bonded = bonded;
        if (connected != -1)
            conn_dev[update_index].connected = connected;
        if (conn_dev[update_index].bonded == 0 && conn_dev[update_index].connected == 0)
            memset(&conn_dev[update_index], 0, sizeof(conn_dev[update_index]));
    }
}

static void btmw_rpc_test_gap_event_callback(BTMW_GAP_EVT *bt_event, void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GAP] bt_gap_event =%d\n",  bt_event->state);
    switch (bt_event->state)
     {
     case GAP_STATE_ON:
        BTMW_RPC_TEST_Logi("[GAP] bt_gap_event = state on\n");
        break;
     case GAP_STATE_OFF:
        BTMW_RPC_TEST_Logi("[GAP] bt_gap_event = state off\n");
        break;

     case GAP_STATE_ACL_CONNECTED:
        BTMW_RPC_TEST_Logi("[GAP] bt_gap_event = acl connected:%s link_type:%d\n",
                            bt_event->bd_addr, bt_event->link_type);
        btmw_rpc_update_conn_dev(bt_event->bd_addr, -1, 1);
        break;

     case GAP_STATE_ACL_DISCONNECTED:
        if(bt_event->reason)
        {
             if (0x08 == bt_event->reason)
             {
                 BTMW_RPC_TEST_Logi("[GAP] bt_gap_event = connect lost:%s link_type:%d\n",
                                    bt_event->bd_addr, bt_event->link_type);
             }
        }
        else
        {
            BTMW_RPC_TEST_Logi("[GAP] bt_gap_event = acl is disconnected:%s link_type:%d\n",
                                bt_event->bd_addr, bt_event->link_type);
        }
        btmw_rpc_update_conn_dev(bt_event->bd_addr, -1, 0);
        break;

     case GAP_STATE_BONDED:
        BTMW_RPC_TEST_Logi("[GAP] addr:%s, bt_gap_event = BOND SUCCESS\n",
                           bt_event->bd_addr);
        btmw_rpc_update_conn_dev(bt_event->bd_addr, 1, -1);
        btmw_rpc_test_bass_connect_bonded_device(bt_event->bd_addr);
        break;
     case GAP_STATE_BONDING:
        break;
     case GAP_STATE_NO_BOND:
        BTMW_RPC_TEST_Logi("[GAP] addr:%s, bt_gap_event = BOND FAIL\n",
                           bt_event->bd_addr);
        btmw_rpc_update_conn_dev(bt_event->bd_addr, 0, -1);
        break;
     case GAP_STATE_UNPAIR_SUCCESS:
        BTMW_RPC_TEST_Logi("[GAP] addr:%s, bt_gap_event = UNBOND SUCCESS\n",
                           bt_event->bd_addr);
        btmw_rpc_update_conn_dev(bt_event->bd_addr, 0, -1);
        break;
     case GAP_STATE_DISCOVERY_STARTED:
         BTMW_RPC_TEST_Logi("[GAP] GAP_STATE_DISCOVERY_STARTED\n");
        break;
     case GAP_STATE_DISCOVERY_STOPED:
         BTMW_RPC_TEST_Logi("[GAP] GAP_STATE_DISCOVERY_STOPED\n");
         if (inq_times != 0)
         {
            inq_times--;
            BTMW_RPC_TEST_Logi("[GAP] continue inquiry, left %u times\n", inq_times);
            a_mtkapi_bt_gap_start_inquiry(inq_filter_type);
         }
        break;
     default:
        BTMW_RPC_TEST_Logi("[GAP] undefined bt_gap_event \n");
        break;
     }

    return;
}

static void btmw_rpc_test_gap_get_pairing_key_callback(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GAP] bt_pairing_key->pin_code =%s,  key_value = %06u, fg_accept = %u\n", bt_pairing_key->pin_code, bt_pairing_key->key_value, *fg_accept);
    return;
}

static void btmw_rpc_test_gap_app_inquiry_callback(BTMW_GAP_DEVICE_INFO *pt_result, void* pv_tag)
{
    int ret = 0;
    unsigned int index, add_index = INQ_RESULT_MAX;
    int bonded = 0;

    if (NULL == pt_result)
    {
        return;
    }

    for (index = 0; index < inq_max_dev; index++)
    {
        if (inq_result[index].valid == 0)
        {
            add_index = index;
        }
        else
        {
            if (strncmp(inq_result[index].bdAddr, pt_result->device.bdAddr, sizeof(inq_result[index].bdAddr)) == 0)
            {
                add_index = index;
                break;
            }
            else
            {
                continue;
            }
        }
    }

    if (add_index == INQ_RESULT_MAX)
    {
        return;
    }
    else
    {
        strncpy(inq_result[add_index].bdAddr, pt_result->device.bdAddr, sizeof(pt_result->device.bdAddr));
        inq_result[add_index].bdAddr[MAX_BDADDR_LEN - 1] = '\0';
        memset(inq_result[add_index].name, 0, sizeof(inq_result[add_index].name));
        if (strlen(pt_result->device.name))
        {
            strncpy(inq_result[add_index].name, pt_result->device.name, sizeof(pt_result->device.name));
            inq_result[add_index].name[MAX_NAME_LEN - 1] = '\0';
        }
        inq_result[add_index].cod = pt_result->device.cod;
        inq_result[add_index].service = pt_result->device.service;
        inq_result[add_index].valid = 1;
    }

    ret = a_mtkapi_bt_gap_get_bond_state(pt_result->device.bdAddr);

    if (BT_SUCCESS == ret)
    {
        bonded = 1;
        if (pt_result->device.service == 0x100000)  // 1 << BT_GAP_HID_SERVICE_ID
        {
            BTMW_RPC_TEST_Logi(" %s is HID device, auto connect...\n", pt_result->device.bdAddr);
            a_mtkapi_hidh_connect(pt_result->device.bdAddr);
        }
        else
        {
            BTMW_RPC_TEST_Logi(" %s is not HID device, don't connect...\n", pt_result->device.bdAddr);
        }
    } else if (BT_ERR_STATUS_FAIL == ret) {
        bonded = 0;
    }
    btmw_rpc_update_conn_dev(pt_result->device.bdAddr, bonded, -1);

    BTMW_RPC_TEST_Logi(" addr: %s, rssi: %d, service: 0x%x, kind: %d, cod: 0x%2lx, name: %s, %s",
      pt_result->device.bdAddr,
      pt_result->device.rssi,
      pt_result->device.service,
      pt_result->device_kind,
      pt_result->device.cod,
      pt_result->device.name,
      bonded ? "bonded" : "unbond");
}

int btmw_rpc_test_gap_set_power_cli(int argc, char **argv)
{
    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("0 means off, 1 means on\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    if (0 == strcmp("0" , argv[0]))
    {
        btmw_rpc_test_bms_clear();
        return a_mtkapi_bt_gap_on_off(FALSE);
    }
    else if (0 == strcmp("1" , argv[0]))
    {
        return a_mtkapi_bt_gap_on_off(TRUE);
    }
    else
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_name_cli(int argc, char **argv)
{
    INT32 i4_ret = BT_SUCCESS, i = 0;
    CHAR name[64];

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logd("please attach name after commond just append one string\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    memset(name, 0, sizeof(name));
    for (i = 0; i < argc; i++)
    {
        (void)snprintf(name+strlen(name), sizeof(name) - strlen(name), "%s", argv[i]);
        if (i < (argc - 1))
            (void)snprintf(name+strlen(name), sizeof(name) - strlen(name), "%s", " ");
    }

    BTMW_RPC_TEST_Logd(" name is: %s\n", name);
    i4_ret = a_mtkapi_bt_gap_set_name(name);
    if (BT_SUCCESS == i4_ret)
    {
        BTMW_RPC_TEST_Logd("set name ok!\n");
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_conn_disc_cli(int argc, char **argv)
{
    if (2 == argc)
    {
        if (0 == strcmp("1" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, FALSE);
            }
            else
            {
                BTMW_RPC_TEST_Loge("input error, 0 means off, 1 means on\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
        }
        else if (0 == strcmp("0" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
            }
            else
            {
                BTMW_RPC_TEST_Loge("input error, 0 means off, 1 means on\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
        }
    }
    else
    {
        BTMW_RPC_TEST_Loge("0 means off, other integer:on");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_get_dev_info_cli(int argc, char **argv)
{
    BLUETOOTH_DEVICE dev_info;
    INT32 i4_ret = BT_SUCCESS;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("addr is null \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    i4_ret = a_mtkapi_bt_gap_get_dev_info(&dev_info, argv[0]);
    if (i4_ret == BT_SUCCESS)
    {
        i4_ret = a_mtkapi_bt_gap_get_bond_state(argv[0]);
        BTMW_RPC_TEST_Logd("get device info ok!\n");
        BTMW_RPC_TEST_Logd("bdAddr: %s\n", dev_info.bdAddr);
        BTMW_RPC_TEST_Logd("name: %s\n", dev_info.name);
        BTMW_RPC_TEST_Logd("bonded: %s\n", (i4_ret == BT_SUCCESS) ? "yes" : "no");
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_get_local_dev_info_cli(int argc, char **argv)
{
    BT_LOCAL_DEV ps_dev_info;
    INT32 i4_ret = BT_SUCCESS;
    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);

    if (BT_SUCCESS == i4_ret)
    {
        BTMW_RPC_TEST_Logd("get local device info ok!\n");
        BTMW_RPC_TEST_Logd("addr:        %s\n", ps_dev_info.addr);
        BTMW_RPC_TEST_Logd("name:        %s\n", ps_dev_info.name);
        BTMW_RPC_TEST_Logd("powered:     %s\n", (ps_dev_info.state == GAP_STATE_ON) ? "on" : "off");
        BTMW_RPC_TEST_Logd("scan_mode:   %d\n", ps_dev_info.scan_mode);
    }
    else
    {
        BTMW_RPC_TEST_Logd("get local device info failed!\n");
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_inquiry_cli(int argc, char **argv)
{
    UINT32 ui4_filter_type = BT_INQUIRY_FILTER_TYPE_ALL;
    if ((argc != 1) && (argc != 3))
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    memset(inq_result, 0, sizeof(inq_result));
    inq_max_dev = INQ_RESULT_MAX;

    /* <all|src|sink|hfp|hid> */
    if (0 == strcmp(argv[0], "all"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_ALL;
    }
    else if (0 == strcmp(argv[0], "src"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_A2DP_SRC;
    }
    else if (0 == strcmp(argv[0], "sink"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_A2DP_SNK;
    }
    else if (0 == strcmp(argv[0], "hfp"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_HFP;
    }
    else if (0 == strcmp(argv[0], "hid"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_HID;
    }
    else if (0 == strcmp(argv[0], "bmr"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_BMR;
    }
    else if (0 == strcmp(argv[0], "umr"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_UMR;
    }
    else
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    if (argc == 3)
    {
        if (0 == strcmp(argv[1], "num"))
        {
            inq_max_dev = atol(argv[2]);
            if (inq_max_dev > INQ_RESULT_MAX)
            {
                BTMW_RPC_TEST_Logd("inq_max_dev is %d, set to %d\n", inq_max_dev, INQ_RESULT_MAX);
                inq_max_dev = INQ_RESULT_MAX;
            }
        }
        else if (0 == strcmp(argv[1], "addr"))
        {
            if (17 != strlen(argv[2]))
            {
                BTMW_RPC_TEST_Logd("mac length should be 17\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
            inq_max_dev = 1;
            strncpy(inq_result[0].bdAddr, argv[2], sizeof(inq_result[0].bdAddr));
            inq_result[0].bdAddr[MAX_BDADDR_LEN - 1] = '\0';
            inq_result[0].name[0] = 1;
            inq_result[0].cod = (UINT32)(-1);
            inq_result[0].service = (UINT32)(-1);
            inq_result[0].valid = 1;
        }
        else if (0 == strcmp(argv[1], "times"))
        {
            inq_times = (UINT32)atoi(argv[2]);
            inq_filter_type = ui4_filter_type;
        }
        else
        {
            BTMW_RPC_TEST_Loge("no parameter in this command\n");
            return BT_ERR_STATUS_INVALID_PARM_NUMS;
        }
    }

    a_mtkapi_bt_gap_start_inquiry(ui4_filter_type);
    return 0;
}

int btmw_rpc_test_gap_stop_inquiry_cli(int argc, char **argv)
{
    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    inq_times = 0;
    inq_filter_type = 0;

    return a_mtkapi_bt_gap_stop_inquiry();
}

int btmw_rpc_test_gap_pair_cli (int argc, char **argv)
{
    UINT32 transport;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logd("parameter num error,please enter two parameter: addr and transport \n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("addr is null \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    transport = atol(argv[1]);
    return a_mtkapi_bt_gap_pair(argv[0], transport);

}

int btmw_rpc_test_gap_remove_paired_dev_cli(int argc, char **argv)
{
    if (argc != 1)
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    return a_mtkapi_bt_gap_unpair(argv[0]);
}

int btmw_rpc_test_gap_cancel_pair_cli(int argc, char **argv)
{
    if (argc != 1)
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    return a_mtkapi_bt_gap_cancel_pair(argv[0]);
}

int btmw_rpc_test_gap_get_rssi_cli (int argc, char **argv)
{
    CHAR ps_target_mac[18];
    INT16 rssi_value = 0;

    if (1 == argc)
    {
        memset(ps_target_mac, 0, sizeof(ps_target_mac));
        if (17 != strlen(argv[0]))
        {
            BTMW_RPC_TEST_Logd("mac length should be 17\n");
            return BT_ERR_STATUS_PARM_INVALID;
        }
        strncpy(ps_target_mac,argv[0],sizeof(ps_target_mac)-1);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input get_rssi [MAC address]\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    a_mtkapi_bt_gap_get_rssi(ps_target_mac, &rssi_value);
    //you can use the rssi_value  callback here
    BTMW_RPC_TEST_Logd("rpc test app get_rssi=%d\n", rssi_value);
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_send_hci(int argc, char **argv)
{
    CHAR *hci_cmd;

    if (argc != 1)
        return BT_ERR_STATUS_INVALID_PARM_NUMS;

    hci_cmd = (CHAR *)argv[0];
    a_mtkapi_bt_gap_send_hci(hci_cmd);
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_factory_reset(int argc, char **argv)
{
    if (argc != 0)
        return BT_ERR_STATUS_INVALID_PARM_NUMS;

    a_mtkapi_bt_gap_factory_reset();
    return BT_SUCCESS;
}

INT32 btmw_rpc_test_gap_test_mode(int argc, char *argv[])
{
    UINT32 test_cnt = 0;
    UINT32 interval_ms = 0;
    test_cnt = atoi(argv[1]);
    interval_ms = atoi(argv[2]);
    BTMW_RPC_TEST_Logd("test_cnt=%d, interval_ms=%d\n", test_cnt, interval_ms);

    if (0 == strcmp(argv[0], "power"))
    {
        while(test_cnt)
        {
            BTMW_RPC_TEST_Logd("start test_cnt=%d, on_off=%d\n", test_cnt, test_cnt&1);
            a_mtkapi_bt_gap_on_off(test_cnt&1);
            BTMW_RPC_TEST_Logd("end test_cnt=%d, on_off=%d\n", test_cnt, test_cnt&1);

            /* if it's power on */
            if (test_cnt&1)
            {
                if (argc > 3)
                {
                    if (0 == strcmp(argv[3], "a2dp_conn"))
                    {
                        BTMW_RPC_TEST_Logd("a2dp conn %s\n", argv[4]);
                        a_mtkapi_a2dp_connect(argv[4], BT_A2DP_ROLE_SINK);
                    }
                }
                //a_mtkapi_a2dp_sink_start_player();
            }
            BTMW_RPC_TEST_Logd("start sleep interval_ms=%d\n", interval_ms);
            usleep(interval_ms*1000);
            BTMW_RPC_TEST_Logd("end sleep interval_ms=%d\n", interval_ms);
            test_cnt--;
        }
    }

    return BT_SUCCESS;
}

INT32 btmw_rpc_test_gap_get_all_dev_info_cli(int argc, char *argv[])
{
    unsigned int index, cnt = 1;
    BLUETOOTH_DEVICE dev_info;

    for (index = 0; index < CONN_MAX; index++)
    {
        if (strlen(conn_dev[index].bdAddr) != 0)
        {
            memset(&dev_info, 0, sizeof(dev_info));
            (void)a_mtkapi_bt_gap_get_dev_info(&dev_info, conn_dev[index].bdAddr);
            BTMW_RPC_TEST_Logi(" Device %d, addr: %s, bonded: %s," \
                " connected: %s, linktype: %s, name: %s\n",
                cnt++, conn_dev[index].bdAddr, conn_dev[index].bonded == 1 ? "yes" : " no",
                conn_dev[index].connected == 1 ? "yes" : " no",
                dev_info.devicetype == 1 ?
                    "BREDR" : (dev_info.devicetype == 2 ?
                    "  BLE" : (dev_info.devicetype == 3 ? " DUAL" : "   no")),
                dev_info.name);
        }
    }
    return BT_SUCCESS;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_gap_cli_commands[] = {
    { (const char *)"power",                btmw_rpc_test_gap_set_power_cli,              (const char *)" = power_on local device, 0:off, 1:on"},
    { (const char *)"name",                 btmw_rpc_test_gap_set_name_cli,               (const char *)" = bt set local dev name, name <xxx>"},
    { (const char *)"set_conn_disc",        btmw_rpc_test_gap_set_conn_disc_cli,          (const char *)" = set device connectable and discoverable, set_conn_disc <1|0> <1|0>, 1:enable, 0: unable"},
    { (const char *)"get_dev_info",         btmw_rpc_test_gap_get_dev_info_cli,           (const char *)" = get device info "},
    { (const char *)"info",                 btmw_rpc_test_gap_get_local_dev_info_cli,     (const char *)" = info <local|update>"},
    { (const char *)"inquiry",              btmw_rpc_test_gap_inquiry_cli,                (const char *)" = start device discovery <all|src|sink|hfp|hid>"},
    { (const char *)"stop_inquiry",         btmw_rpc_test_gap_stop_inquiry_cli,           (const char *)" = stop device discovery"},
    { (const char *)"pair",                 btmw_rpc_test_gap_pair_cli,                   (const char *)" = pair a remote device <addr> <transport> 0:unknown,1:BR/EDR,2:BLE"},
    { (const char *)"unpair",               btmw_rpc_test_gap_remove_paired_dev_cli,      (const char *)" = remove paired device <addr>"},
    { (const char *)"cancel_pair",          btmw_rpc_test_gap_cancel_pair_cli,            (const char *)" = cancel pair a remote device <addr>"},
    { (const char *)"get_rssi",             btmw_rpc_test_gap_get_rssi_cli,               (const char *)" = get_rssi <addr>"},
    { (const char *)"send_hci",             btmw_rpc_test_gap_send_hci,                   (const char *)" = send_hci <buffer>"},
    { (const char *)"factory_reset",        btmw_rpc_test_gap_factory_reset,              (const char *)" = delete bluetooth files for factory reset"},
    { (const char *)"test",                 btmw_rpc_test_gap_test_mode,                  (const char *)" = test { power <N> } <interval:ms> [a2dp_conn <addr>]"},
    { (const char *)"get_all_dev_info",     btmw_rpc_test_gap_get_all_dev_info_cli,                  (const char *)" = test { power <N> } <interval:ms> [a2dp_conn <addr>]"},
    { NULL, NULL, NULL }
};

int btmw_rpc_test_gap_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_gap_cli_commands;

    BTMW_RPC_TEST_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GAP, btmw_rpc_test_gap_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_gap_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD btmw_rpc_test_gap_mod = {0};
    BT_LOCAL_DEV dev = {0};
    char name[128] = {0};

    btmw_rpc_test_gap_mod.mod_id = BTMW_RPC_TEST_MOD_GAP;
    strncpy(btmw_rpc_test_gap_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GAP, sizeof(btmw_rpc_test_gap_mod.cmd_key));
    btmw_rpc_test_gap_mod.cmd_handler = btmw_rpc_test_gap_cmd_handler;
    btmw_rpc_test_gap_mod.cmd_tbl = btmw_rpc_test_gap_cli_commands;

    ret = btmw_rpc_test_register_mod(&btmw_rpc_test_gap_mod);
    BTMW_RPC_TEST_Logi("[GAP] returns: %d\n", ret);

    memset(conn_dev, 0, sizeof(conn_dev));
    memset(inq_result, 0, sizeof(inq_result));

    memset(&g_gap_func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
    g_gap_func.bt_event_cb = btmw_rpc_test_gap_event_callback;
    g_gap_func.bt_get_pairing_key_cb = btmw_rpc_test_gap_get_pairing_key_callback;
    g_gap_func.bt_dev_info_cb = btmw_rpc_test_gap_app_inquiry_callback;
    a_mtkapi_bt_gap_base_init(&g_gap_func, (void *)g_gap_pv_tag);
    a_mtkapi_bt_gap_on_off(TRUE);
    if (a_mtkapi_bt_gap_get_local_dev_info(&dev) == BT_SUCCESS)
    {
        snprintf(name, sizeof(name) - 1, BTMW_RPC_TEST_DEFAULT_NAME"(%s)", &dev.addr[12]);
        name[127] = '\0';
        a_mtkapi_bt_gap_set_name(name);
    }
    else
    {
        BTMW_RPC_TEST_Logi("[GAP] get local dev info failed\n");
        a_mtkapi_bt_gap_set_name(BTMW_RPC_TEST_DEFAULT_NAME);
    }
    a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
    return 0;
}

int btmw_rpc_test_gap_deinit()
{
    a_mtkapi_bt_gap_base_uninit(&g_gap_func, (void *)g_gap_pv_tag);
    return 0;
}

