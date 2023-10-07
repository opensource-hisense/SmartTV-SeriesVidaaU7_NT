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
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_hh.h"
#include "u_bt_mw_hidh.h"
#if defined(MTK_LINUX_HIDH) && (MTK_LINUX_HIDH == TRUE)
#include "mtk_bt_hh.h"
#endif
#include "linuxbt_gap_if.h"
#include "linuxbt_hid_if.h"
#include "linuxbt_common.h"
#include "bt_mw_common.h"
#include "bt_mw_hidh.h"
#include "bt_mw_gap.h"
#include "bt_mw_message_queue.h"

static void linuxbt_hid_connection_state_cb(RawAddress* bd_addr, bthh_connection_state_t state)
{
    tBTMW_MSG btmw_msg;
    memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
    btmw_msg.hdr.event = BTMW_HIDH_STATE_EVENT;
    btmw_msg.hdr.len = sizeof(tBT_MW_HIDH_MSG);
    strncpy(btmw_msg.data.hidh_msg.addr, bd_addr->ToString().c_str(), MAX_BDADDR_LEN - 1);

    switch (state) {
        case BTHH_CONN_STATE_CONNECTED:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d' connected", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_CONNECTED;
            break;
        case BTHH_CONN_STATE_CONNECTING:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d' connecting", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_CONNECTING;
            break;
        case BTHH_CONN_STATE_DISCONNECTED:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d' disconnected", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_DISCONNECTED;
            break;
        case BTHH_CONN_STATE_DISCONNECTING:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d' disconnecting", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_DISCONNECTING;
            break;

        case BTHH_CONN_STATE_FAILED_GENERIC:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d' connnect fail", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_CONNECT_FAIL;
            break;
        default:
            BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] Connection State : '%d'", state);
            btmw_msg.data.hidh_msg.event = BTMW_HIDH_CONNECT_FAIL;
            break;
    }
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] send msg id = %d", btmw_msg.hdr.event);
    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_hid_virtual_unplug_cb(RawAddress* bd_addr, bthh_status_t hh_status)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() state: %s(%d)", __FUNCTION__, (hh_status == 0) ? "SUCCESS" : "FAILED", hh_status);
}

static void linuxbt_hid_info_cb(RawAddress* bd_addr, bthh_hid_info_t *hid_info)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() ", __FUNCTION__);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] attr_mask = 0x%x", hid_info->attr_mask);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] sub_class = 0x%x", hid_info->sub_class);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] app_id = 0x%x", hid_info->app_id);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] vendor_id = 0x%x", hid_info->vendor_id);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] product_id = 0x%x", hid_info->product_id);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] version = %d", hid_info->version);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ctry_code = %d", hid_info->ctry_code);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] dl_len = %d", hid_info->dl_len);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] dsc_list = %s", hid_info->dsc_list);
}

static void linuxbt_hid_protocol_mode_cb(RawAddress* bd_addr, bthh_status_t hh_status, bthh_protocol_mode_t mode)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() state: %s", __FUNCTION__, hh_status==0?"SUCCESS":"FAILED");
    BT_DBG_INFO(BT_DEBUG_HID, "[HID] mode = %s(%d)", mode==BTHH_REPORT_MODE?"REPORT_MODE":"BOOT_MODE", mode);
}

static void linuxbt_hid_idle_time_cb(RawAddress* bd_addr, bthh_status_t hh_status, int idle_rate)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() state: %s", __FUNCTION__, hh_status==0?"SUCCESS":"FAILED");
}

static void linuxbt_hid_get_report_cb(RawAddress* bd_addr, bthh_status_t hh_status, uint8_t* rpt_data, int rpt_size)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() state: %s", __FUNCTION__, hh_status==0?"SUCCESS":"FAILED");
    BT_DBG_NORMAL(BT_DEBUG_HID, "Data Len = %d", rpt_size);

    BT_DBG_NORMAL(BT_DEBUG_HID, "Data = 0x%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
                rpt_size>0?rpt_data[0]:0,
                rpt_size>1?rpt_data[1]:0,
                rpt_size>2?rpt_data[2]:0,
                rpt_size>3?rpt_data[3]:0,
                rpt_size>4?rpt_data[4]:0,
                rpt_size>5?rpt_data[5]:0,
                rpt_size>6?rpt_data[6]:0,
                rpt_size>7?rpt_data[7]:0,
                rpt_size>9?rpt_data[8]:0,
                rpt_size>10?rpt_data[9]:0,
                rpt_size>11?rpt_data[10]:0,
                rpt_size>12?rpt_data[11]:0,
                rpt_size>13?rpt_data[12]:0,
                rpt_size>14?rpt_data[13]:0,
                rpt_size>15?rpt_data[14]:0,
                rpt_size>16?rpt_data[15]:0);
  if (rpt_data[0] == 0)
  {
    BT_DBG_NORMAL(BT_DEBUG_HID, "Report ID is NULL.Report ID cannot be NULL. Invalid HID report recieved ");
  }
}

static void linuxbt_hid_handshake_cb(RawAddress* bd_addr, bthh_status_t hh_status)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] %s() state: %s", __FUNCTION__, (hh_status == 0) ? "SUCCESS" : "FAILED" );
}


static bthh_interface_t *g_bt_hid_interface = NULL;
static bthh_callbacks_t g_bt_hid_callbacks =
{
    sizeof(bthh_callbacks_t),
    linuxbt_hid_connection_state_cb,
    linuxbt_hid_info_cb,
    linuxbt_hid_protocol_mode_cb,
    linuxbt_hid_idle_time_cb,
    linuxbt_hid_get_report_cb,
    linuxbt_hid_virtual_unplug_cb,
    linuxbt_hid_handshake_cb,
};

int linuxbt_hid_connect_int_handler(char *pbt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (g_bt_hid_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID connect to %s", pbt_addr);

    ret = g_bt_hid_interface->connect(&bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] connect ret=%d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_disconnect_handler(char *pbt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID disconnect to %s", pbt_addr);

    ret = g_bt_hid_interface->disconnect(&bdaddr);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_set_output_report_handler(char *pbt_addr, char *preport_data)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == preport_data)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of preport_data");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if(NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID set output report to %s", pbt_addr);

    ret = g_bt_hid_interface->set_report(&bdaddr, BTHH_OUTPUT_REPORT, preport_data);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_get_input_report_handler(char *pbt_addr, int reportId, int bufferSize)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID get input report from %s", pbt_addr);

    ret = g_bt_hid_interface->get_report(&bdaddr, BTHH_INPUT_REPORT, reportId, bufferSize);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_get_output_report_handler(char *pbt_addr, int reportId, int bufferSize)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID get output report from %s", pbt_addr);

    ret = g_bt_hid_interface->get_report(&bdaddr, BTHH_OUTPUT_REPORT, reportId, bufferSize);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_set_input_report_handler(char *pbt_addr, char *preport_data)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == preport_data)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of preport_data");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if(NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID set input report to %s", pbt_addr);

    ret = g_bt_hid_interface->set_report(&bdaddr, BTHH_INPUT_REPORT, preport_data);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_get_feature_report_handler(char *pbt_addr, int reportId, int bufferSize)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID get feature report from %s", pbt_addr);

    ret = g_bt_hid_interface->get_report(&bdaddr, BTHH_FEATURE_REPORT, reportId, bufferSize);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_set_protocol_handler(char *pbt_addr, int protocol_mode)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    RawAddress bdaddr;

    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    /* protocol mode ( 0:boot protocol, 1:report protocol)  */
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_HID, "HID set protocol to %s", pbt_addr);

    ret = g_bt_hid_interface->set_protocol(&bdaddr, (bthh_protocol_mode_t)protocol_mode);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_get_protocol_handler(char *pbt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    RawAddress bd_addr;
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bd_addr);
    //BT_DBG_INFO(BT_DEBUG_HID, "HID get_protocol %02X:%02X:%02X:%02X:%02X:%02X",
        //bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        //bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]); //need refactor
    ret = g_bt_hid_interface->get_protocol(&bd_addr, BTHH_UNSUPPORTED_MODE);
    return linuxbt_return_value_convert(ret);
}


int linuxbt_hid_virtual_unplug_handler(char *pbt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    RawAddress bd_addr;
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bd_addr);
    //BT_DBG_INFO(BT_DEBUG_HID, "HID virtual_upl %02X:%02X:%02X:%02X:%02X:%02X",
        //bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        //bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]); //need refactor
    ret = g_bt_hid_interface->virtual_unplug(&bd_addr); //need refactor
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_set_feature_report_handler(char *pbt_addr, char *preport_data)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == preport_data)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of preport_data");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if(g_bt_hid_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }
    RawAddress bd_addr;
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bd_addr);
    ret = g_bt_hid_interface->set_report(&bd_addr, BTHH_FEATURE_REPORT, preport_data);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_send_data_handler(char *pbt_addr, char *data)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == data)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of preport_data");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }
    RawAddress bd_addr;
    LINUXBT_CHECK_AND_CONVERT_ADDR(pbt_addr, bd_addr);
    ret = g_bt_hid_interface->send_data(&bd_addr, data);

    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_init(void)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL != g_bt_hid_interface) return BT_ERR_STATUS_DONE;

    // Get HID interface
    g_bt_hid_interface = (bthh_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_HIDHOST_ID);
    if (NULL == g_bt_hid_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to get HID interface");
        return BT_ERR_STATUS_FAIL;
    }

    // Init HID interface
    ret = g_bt_hid_interface->init(&g_bt_hid_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] success to init HID interface");
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] already init HID interface");
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "[HID] Failed to init HID interface");
    }

    return linuxbt_return_value_convert(ret);
}

int linuxbt_hid_deinit(void)
{
    BT_DBG_NORMAL(BT_DEBUG_HID, "[HID] ");
    bt_status_t ret = BT_STATUS_SUCCESS;
    // Deinit HID interface
    if (g_bt_hid_interface != NULL)
    {
        g_bt_hid_interface->cleanup();
        g_bt_hid_interface = NULL;
    }

    return linuxbt_return_value_convert(ret);
}
