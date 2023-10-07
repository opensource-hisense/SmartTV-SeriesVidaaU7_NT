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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_spp_if.h"
#include "mtk_bt_service_spp_wrapper.h"
#include "u_bt_mw_common.h"
#include "u_bt_mw_spp.h"

CHAR* print_spp_event(BT_SPP_EVENT bt_event)
{
    switch (bt_event)
    {
        case BT_SPP_CONNECT:
            return "BT_SPP_CONNECT";
        case BT_SPP_DISCONNECT:
            return "BT_SPP_DISCONNECT";
        case BT_SPP_RECV_DATA:
            return "BT_SPP_RECV_DATA";
        case BT_SPP_CONNECT_FAIL:
            return "BT_SPP_CONNECT_FAIL";
        case BT_SPP_DISCONNECT_FAIL:
            return "BT_SPP_DISCONNECT_FAIL";
        default:
            break;
    }

    return "";
}

static void print_spp_connection_info(BT_SPP_CONNECTION_INFO_DB *info_db)
{
    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        BTMW_RPC_TEST_Logi("[SPP][%d] bd_addr:%s\n", i, info_db->spp_connection_info[i].bd_addr);
        BTMW_RPC_TEST_Logi("[SPP][%d] uuid:%s\n", i, info_db->spp_connection_info[i].uuid);
        BTMW_RPC_TEST_Logi("[SPP][%d] spp_if:%d\n", i, info_db->spp_connection_info[i].spp_if);
        BTMW_RPC_TEST_Logi("[SPP][%d] is_used:%d\n", i, info_db->spp_connection_info[i].is_used);
        BTMW_RPC_TEST_Logi("[SPP][%d] is_server:%d\n", i, info_db->spp_connection_info[i].is_server);
    }
    return;
}

static void btmw_rpc_test_gap_spp_callback(BT_SPP_CBK_STRUCT *pt_spp_struct, void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GAP] %s()\n",  __func__);
    BTMW_RPC_TEST_Logi("[SPP] event: %s\n", print_spp_event(pt_spp_struct->event));
    BTMW_RPC_TEST_Logi("[SPP] bd_addr:%s\n", pt_spp_struct->bd_addr);
    BTMW_RPC_TEST_Logi("[SPP] uuid:%s\n", pt_spp_struct->uuid);
    BTMW_RPC_TEST_Logi("[SPP] spp_data:%s\n", pt_spp_struct->spp_data);
    BTMW_RPC_TEST_Logi("[SPP] uuid_len:%d\n", pt_spp_struct->uuid_len);
    BTMW_RPC_TEST_Logi("[SPP] spp_data_len:%d\n", pt_spp_struct->spp_data_len);
    BTMW_RPC_TEST_Logi("[SPP] spp_if:%d\n", pt_spp_struct->spp_if);
    return;
}

int btmw_rpc_test_spp_unregister_cb(int argc, char **argv)
{
    if (0 != argc)
    {
        BTMW_RPC_TEST_Logd("%s no need input params\n", __func__);
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (!g_cli_pts_mode)
    {
        a_mtkapi_spp_unregister_callback(btmw_rpc_test_gap_spp_callback, NULL);
    }
    return 0;
}

int btmw_rpc_test_spp_connect(int argc, char **argv)
{
    int i4_ret = 0;

    BT_SPP_CONNECT_PARAM *spp_connect_param =
        (BT_SPP_CONNECT_PARAM *)malloc(sizeof(BT_SPP_CONNECT_PARAM));
    if (NULL == spp_connect_param)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_connect_param fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_connect_param, 0 ,sizeof(BT_SPP_CONNECT_PARAM));

    if (3 == argc)
    {
        if (17 != strlen(argv[0]))
        {
            BTMW_RPC_TEST_Logd("%s mac length should be 17\n", __func__);
            if (NULL != spp_connect_param)
            {
                free(spp_connect_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }
        if (0 > atoi(argv[2]))
        {
            BTMW_RPC_TEST_Logd("%s spp if is invalid\n", __func__);
            if (NULL != spp_connect_param)
            {
                free(spp_connect_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }
        //spp bd_addr to connect
        strncpy(spp_connect_param->bd_addr, argv[0], MAX_BDADDR_LEN - 1);
        spp_connect_param->bd_addr[MAX_BDADDR_LEN - 1] = '\0';
        //spp uuid to connect
        strncpy(spp_connect_param->uuid, argv[1], MAX_UUID_LEN - 1);
        spp_connect_param->uuid[MAX_UUID_LEN - 1] = '\0';
        //spp app id
        spp_connect_param->spp_if = atoi(argv[2]);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input spp_connect [MAC address][UUID][SPP_IF]\n");
        if (NULL != spp_connect_param)
        {
            free(spp_connect_param);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    i4_ret = a_mtkapi_spp_connect(spp_connect_param);
    BTMW_RPC_TEST_Logd("i4_ret=%d\n", i4_ret);
    if (NULL != spp_connect_param)
    {
        free(spp_connect_param);
    }
    return i4_ret;
}

int btmw_rpc_test_spp_disconnect(int argc, char **argv)
{
    int i4_ret = 0;

    BT_SPP_DISCONNECT_PARAM *spp_disconnect_param =
        (BT_SPP_DISCONNECT_PARAM *)malloc(sizeof(BT_SPP_DISCONNECT_PARAM));
    if (NULL == spp_disconnect_param)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_disconnect_param fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_disconnect_param, 0 ,sizeof(BT_SPP_DISCONNECT_PARAM));

    if (3 == argc)
    {
        if (17 != strlen(argv[0]))
        {
            BTMW_RPC_TEST_Logd("%s mac length should be 17\n", __func__);
            if (NULL != spp_disconnect_param)
            {
                free(spp_disconnect_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }
        if (0 > atoi(argv[2]))
        {
            BTMW_RPC_TEST_Logd("%s spp if is invalid\n", __func__);
            if (NULL != spp_disconnect_param)
            {
                free(spp_disconnect_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }
        //spp bd_addr to disconnect
        strncpy(spp_disconnect_param->bd_addr, argv[0], MAX_BDADDR_LEN - 1);
        spp_disconnect_param->bd_addr[MAX_BDADDR_LEN - 1] = '\0';
        //spp uuid to disconnect
        strncpy(spp_disconnect_param->uuid, argv[1], MAX_UUID_LEN - 1);
        spp_disconnect_param->uuid[MAX_UUID_LEN - 1] = '\0';
        //spp app id
        spp_disconnect_param->spp_if = atoi(argv[2]);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input spp_disconnect [MAC address][UUID][SPP_IF]\n");
        if (NULL != spp_disconnect_param)
        {
            free(spp_disconnect_param);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    i4_ret = a_mtkapi_spp_disconnect(spp_disconnect_param);
    BTMW_RPC_TEST_Logd("i4_ret=%d\n", i4_ret);
    if (NULL != spp_disconnect_param)
    {
        free(spp_disconnect_param);
    }
    return i4_ret;
}

int btmw_rpc_test_spp_send_data(int argc, char **argv)
{
    int i4_ret = 0;

    BT_SPP_SEND_DATA_PARAM *spp_send_data_param =
        (BT_SPP_SEND_DATA_PARAM *)malloc(sizeof(BT_SPP_SEND_DATA_PARAM));
    if (NULL == spp_send_data_param)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_send_data_param fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_send_data_param, 0 ,sizeof(BT_SPP_SEND_DATA_PARAM));

    if (4 == argc)
    {
        if (17 != strlen(argv[0]))
        {
            BTMW_RPC_TEST_Logd("%s mac length should be 17\n", __func__);
            if (NULL != spp_send_data_param)
            {
                free(spp_send_data_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }

        if (strlen(argv[2]) > 127)
        {
            BTMW_RPC_TEST_Logd("%s string length shoud be < 128\n", __func__);
            if (NULL != spp_send_data_param)
            {
                free(spp_send_data_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }

        //spp bd_addr to send
        strncpy(spp_send_data_param->bd_addr, argv[0], MAX_BDADDR_LEN - 1);
        spp_send_data_param->bd_addr[MAX_BDADDR_LEN - 1] = '\0';
        //spp uuid to send
        strncpy(spp_send_data_param->uuid, argv[1], MAX_UUID_LEN - 1);
        spp_send_data_param->uuid[MAX_UUID_LEN- 1]= '\0';
        //spp data to send
        strncpy(spp_send_data_param->spp_data, argv[2], strlen(argv[2]));
        spp_send_data_param->spp_data_len = strlen(argv[2]);
        //spp app id
        spp_send_data_param->spp_if = atoi(argv[3]);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input spp_send_data [MAC address][UUID][String][SPP_IF]\n");
        if (NULL != spp_send_data_param)
        {
            free(spp_send_data_param);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    i4_ret = a_mtkapi_spp_send_data(spp_send_data_param);
    BTMW_RPC_TEST_Logd("i4_ret=%d\n", i4_ret);
    if (NULL != spp_send_data_param)
    {
        free(spp_send_data_param);
    }
    return i4_ret;
}

int btmw_rpc_test_spp_start_server(int argc, char **argv)
{
    int i4_ret = 0;

    BT_SPP_START_SVR_PARAM *spp_start_svr_param =
        (BT_SPP_START_SVR_PARAM *)malloc(sizeof(BT_SPP_START_SVR_PARAM));
    if (NULL == spp_start_svr_param)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_start_svr_param fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_start_svr_param, 0 ,sizeof(BT_SPP_START_SVR_PARAM));

    if (3 == argc)
    {
        if (strlen(argv[0]) > 254)
        {
            BTMW_RPC_TEST_Logd("%s name length should be < 255\n", __func__);
            if (NULL != spp_start_svr_param)
            {
                free(spp_start_svr_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }
        //spp server name
        strncpy(spp_start_svr_param->server_name, argv[0], strlen(argv[0]));
        //spp server uuid
        strncpy(spp_start_svr_param->uuid, argv[1], MAX_UUID_LEN - 1);
        spp_start_svr_param->uuid[MAX_UUID_LEN - 1] = '\0';
        //spp app id
        spp_start_svr_param->spp_if = atoi(argv[2]);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input start_svr [server name][UUID][SPP_IF]\n");
        if (NULL != spp_start_svr_param)
        {
            free(spp_start_svr_param);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    i4_ret = a_mtkapi_spp_start_server(spp_start_svr_param);
    BTMW_RPC_TEST_Logd("i4_ret=%d\n", i4_ret);
    if (NULL != spp_start_svr_param)
    {
        free(spp_start_svr_param);
    }
    return i4_ret;
}


int btmw_rpc_test_spp_stop_server(int argc, char **argv)
{
    int i4_ret = 0;

    BT_SPP_STOP_SVR_PARAM *spp_stop_svr_param =
        (BT_SPP_STOP_SVR_PARAM *)malloc(sizeof(BT_SPP_STOP_SVR_PARAM));
    if (NULL == spp_stop_svr_param)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_stop_svr_param fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_stop_svr_param, 0 ,sizeof(BT_SPP_STOP_SVR_PARAM));

    if (3 == argc)
    {
        if (strlen(argv[0]) > 254)
        {
            BTMW_RPC_TEST_Logd("%s name length should be < 255\n", __func__);
            if (NULL != spp_stop_svr_param)
            {
                free(spp_stop_svr_param);
            }
            return BT_ERR_STATUS_PARM_INVALID;
        }

        //spp server name
        strncpy(spp_stop_svr_param->server_name, argv[0], strlen(argv[0]));
        //spp server uuid
        strncpy(spp_stop_svr_param->uuid, argv[1], MAX_UUID_LEN - 1);
        spp_stop_svr_param->uuid[MAX_UUID_LEN - 1] = '\0';
        //spp app id
        spp_stop_svr_param->spp_if = atoi(argv[2]);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input stop_svr [server name][UUID][SPP_IF]\n");
        if (NULL != spp_stop_svr_param)
        {
            free(spp_stop_svr_param);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    i4_ret = a_mtkapi_spp_stop_server(spp_stop_svr_param);
    BTMW_RPC_TEST_Logd("i4_ret=%d\n", i4_ret);
    if (NULL != spp_stop_svr_param)
    {
        free(spp_stop_svr_param);
    }
    return i4_ret;
}

int btmw_rpc_test_spp_get_connection_info(int argc, char **argv)
{
    INT32 i4_ret = 0;
    BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db = (BT_SPP_CONNECTION_INFO_DB *)malloc(sizeof(BT_SPP_CONNECTION_INFO_DB));
    if (NULL == spp_connection_info_db)
    {
        BTMW_RPC_TEST_Logd("%s malloc spp_connection_info_db fail\n", __func__);
        return BT_ERR_STATUS_FAIL;
    }
    memset(spp_connection_info_db, 0, sizeof(BT_SPP_CONNECTION_INFO_DB));

    if (0 == argc)
    {
        i4_ret = a_mtkapi_spp_get_connection_info(spp_connection_info_db);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input get_connection_info only\n");
        if (NULL != spp_connection_info_db)
        {
            free(spp_connection_info_db);
        }
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    print_spp_connection_info(spp_connection_info_db);
    if (NULL != spp_connection_info_db)
    {
        free(spp_connection_info_db);
    }
    return i4_ret;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_spp_cli_commands[] = {
    { (const char *)"unregister_cb",      btmw_rpc_test_spp_unregister_cb,          (const char *)" = input callback func"},
    { (const char *)"connect",            btmw_rpc_test_spp_connect,                (const char *)" = input addr and uuid and spp_if"},
    { (const char *)"disconnect",         btmw_rpc_test_spp_disconnect,             (const char *)" = input addr and uuid and spp_if"},
    { (const char *)"send_data",          btmw_rpc_test_spp_send_data,              (const char *)" = input addr, uuid and data and spp_if"},
    { (const char *)"start_svr",          btmw_rpc_test_spp_start_server,           (const char *)" = input server name and uuid and spp_if"},
    { (const char *)"stop_svr",           btmw_rpc_test_spp_stop_server,            (const char *)" = input server name and uuid and spp_if"},
    { (const char *)"get_connection_info",btmw_rpc_test_spp_get_connection_info,    (const char *)" = input get_connection_info"},

    { NULL, NULL, NULL }
};

int btmw_rpc_test_spp_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_spp_cli_commands;

    BTMW_RPC_TEST_Logi("[SPP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_SPP, btmw_rpc_test_spp_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_spp_init()
{
    INT32 ret = 0;
    UINT8 spp_if = 0;
    BTMW_RPC_TEST_MOD spp_mod = {0};

    spp_mod.mod_id = BTMW_RPC_TEST_MOD_SPP;
    strncpy(spp_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_SPP, sizeof(spp_mod.cmd_key));
    spp_mod.cmd_handler = btmw_rpc_test_spp_cmd_handler;
    spp_mod.cmd_tbl = btmw_rpc_test_spp_cli_commands;

    ret = btmw_rpc_test_register_mod(&spp_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for SPP returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        spp_if = a_mtkapi_spp_register_callback(btmw_rpc_test_gap_spp_callback, NULL);
        BTMW_RPC_TEST_Logd("btmw_rpc_test_register_callback() spp_if: %d\n", spp_if);
    }

    return BT_SUCCESS;
}

