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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/*******************************************************************************
API Usage Example
    #1, BT Initialization Flow
        // GAP Callback functions
        static void BtAppGapMwEventCbk(tBTMW_GAP_STATE *bt_event, void* pv_tag){}
        static void BtAppGapMwGetPairingKeyCbk(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag){}
        static void BtAppGapMwInquiryResponseCbk(tBTMW_GAP_DEVICE_INFO* pt_result, void* pv_tag){}

        // SPP Callback functions
        static void BtAppMwSppEventCbk(BT_SPP_CBK_STRUCT *pt_spp_struct, VOID *pv_tag){}
        static void BtAppMwSppConnAuthCbk(CHAR* addr, BOOL* accept){}

        // HID Device Callback functions
        static void BtAppMwHiddEventCbk(BT_HIDD_CBK_STRUCT *pt_hidd_struct, VOID *pv_tag){}
        static void BtAppMwHiddConnAuthCbk(CHAR* addr, BOOL* accept){}

        // GATT Client Callback functions
        static void BtAppGATTCMwEventCbk(BT_GATTC_EVENT_T bt_gatt_event, BT_GATTC_CONNECT_STATE_OR_RSSI_T *bt_gattc_conect_state_or_rssi, void* pv_tag){}
        static void BtAppGATTCMwRegClientCbk(BT_GATTC_REG_CLIENT_T *pt_reg_client_result, void* pv_tag){}
        static void BtAppGATTCMwUnRegClientCbk(BT_GATTC_UNREG_CLIENT_T *pt_unreg_client_result, void* pv_tag){}
        static void BtAppGATTCMwScanCbk(BT_GATTC_SCAN_RST_T *pt_scan_result, void* pv_tag){}
        static void BtAppGATTCMwGetGattDbCbk(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result, void* pv_tag){}
        static void BtAppGATTCMwGetRegNotiCbk(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result, void* pv_tag){}
        static void BtAppGATTCMwNotifyCbk(BT_GATTC_GET_NOTIFY_T *pt_notify, void* pv_tag){}
        static void BtAppGATTCMwReadCharCbk(BT_GATTC_READ_CHAR_RST_T *pt_read_char, void* pv_tag){}
        static void BtAppGATTCMwWriteCharCbk(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char, void* pv_tag){}
        static void BtAppGATTCMwReadDescCbk(BT_GATTC_READ_DESCR_RST_T *pt_read_desc, void* pv_tag){}
        static void BtAppGATTCMwWriteDescCbk(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc, void* pv_tag){}
        static void BtAppGATTMwScanFilterParamCbk(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param, void* pv_tag){}
        static void BtAppGATTMwScanFilterStatusCbk(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status, void* pv_tag){}
        static void BtAppGATTMWScanFilterCfgCbk(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg, void* pv_tag){}
        static void BtAppGATTCMwAdvEnableCbk(BT_GATTC_ADV_ENABLED_T *pt_adv_enabled, void* pv_tag){}
        static void BtAppGATTCMwConfigMtuCbk(BT_GATTC_MTU_RST_T *pt_config_mtu_result, void* pv_tag){}

        // GATT Server Callback functions
        static void BtAppGATTSMwEventCbk(BT_GATTS_EVENT_T bt_gatts_event, void* pv_tag){}
        static void BtAppGATTSMwRegServerCbk(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server, void* pv_tag){}
        static void BtAppGATTSMwAddSrvcCbk(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc, void* pv_tag){}
        static void BtAppGATTSMwAddInclCbk(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl, void* pv_tag){}
        static void BtAppGATTSMwAddCharCbk(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char, void* pv_tag){}
        static void BtAppGATTSMwAddDescCbk(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc, void* pv_tag){}
        static void BtAppGATTSMwOpSrvcCbk(BT_GATTS_SRVC_OP_TYPE_T op_type, BT_GATTS_SRVC_RST_T *bt_gatts_srvc, void* pv_tag){}
        static void BtAppGATTSMwReqReadCbk(BT_GATTS_REQ_READ_RST_T *bt_gatts_read, void* pv_tag){}
        static void BtAppGATTSMwReqWriteCbk(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write, void* pv_tag){}
        static void BtAppGATTSMwIndSentCbk(INT32 conn_id, INT32 status, void* pv_tag){}
        static void BtAppGATTSMwConfigMtuCbk(BT_GATTS_CONFIG_MTU_RST_T *bt_gatts_config_mtu, void* pv_tag){}
        static void BtAppGATTSMwExecWrite(BT_GATTS_EXEC_WRITE_RST_T *bt_gatts_exec_write, void* pv_tag){}

        static MTKRPCAPI_BT_APP_CB_FUNC g_hb_mw_cb;
        static MTKRPCAPI_BT_SPP_APP_CB_FUNC g_hb_mw_spp_cb;
        static MTKRPCAPI_BT_HIDD_APP_CB_FUNC g_hb_mw_hidd_cb;
        static MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T g_hb_mw_gattc_cb;
        static MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T g_hb_mw_gatts_cb;

        // Initialize GAP
        g_hb_mw_cb.bt_event_cb = BtAppGapMwEventCbk;
        g_hb_mw_cb.bt_get_pairing_key_cb = BtAppGapMwGetPairingKeyCbk;
        g_hb_mw_cb.bt_dev_info_cb = BtAppGapMwInquiryResponseCbk;
        a_mtkapi_bt_gap_base_init(&g_hb_mw_cb, NULL);

        // Initialize SPP
        g_hb_mw_spp_cb.bt_spp_event_cb = BtAppMwSppEventCbk;
        g_hb_mw_spp_cb.bt_spp_conn_auth_cb = BtAppMwSppConnAuthCbk;
        a_mtkapi_spp_register_callback(&g_hb_mw_spp_cb, NULL);

        // Initialize HID Device
        g_hb_mw_hidd_cb.bt_hidd_event_cb = BtAppMwHiddEventCbk;
        g_hb_mw_hidd_cb.bt_hidd_conn_auth_cb = BtAppMwHiddConnAuthCbk;
        a_mtkapi_hidd_register_callback(&g_hb_mw_hidd_cb, NULL);

        // Initialize GATT Client
        bt_os_layer_memset(&g_hb_mw_gattc_cb, 0, sizeof(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T));
        g_hb_mw_gattc_cb.bt_gattc_event_cb = BtAppGATTCMwEventCbk;
        g_hb_mw_gattc_cb.bt_gattc_reg_client_cb = BtAppGATTCMwRegClientCbk;
        g_hb_mw_gattc_cb.bt_gattc_scan_cb = BtAppGATTCMwScanCbk;
        g_hb_mw_gattc_cb.bt_gattc_get_gatt_db_cb = BtAppGATTCMwGetGattDbCbk;
        g_hb_mw_gattc_cb.bt_gattc_get_reg_noti_cb = BtAppGATTCMwGetRegNotiCbk;
        g_hb_mw_gattc_cb.bt_gattc_notify_cb = BtAppGATTCMwNotifyCbk;
        g_hb_mw_gattc_cb.bt_gattc_read_char_cb = BtAppGATTCMwReadCharCbk;
        g_hb_mw_gattc_cb.bt_gattc_write_char_cb = BtAppGATTCMwWriteCharCbk;
        g_hb_mw_gattc_cb.bt_gattc_read_desc_cb = BtAppGATTCMwReadDescCbk;
        g_hb_mw_gattc_cb.bt_gattc_write_desc_cb = BtAppGATTCMwWriteDescCbk;
        g_hb_mw_gattc_cb.bt_gattc_scan_filter_param_cb = BtAppGATTMwScanFilterParamCbk;
        g_hb_mw_gattc_cb.bt_gattc_scan_filter_status_cb = BtAppGATTMwScanFilterStatusCbk;
        g_hb_mw_gattc_cb.bt_gattc_scan_filter_cfg_cb = BtAppGATTMWScanFilterCfgCbk;
        g_hb_mw_gattc_cb.bt_gattc_adv_enable_cb = BtAppGATTCMwAdvEnableCbk;
        g_hb_mw_gattc_cb.bt_gattc_config_mtu_cb = BtAppGATTCMwConfigMtuCbk;
        g_hb_mw_gattc_cb.bt_gattc_unreg_client_cb = BtAppGATTCMwUnRegClientCbk;
        a_mtkapi_bt_gattc_base_init(&g_hb_mw_gattc_cb, NULL);

        // Initialize GATT Server
        bt_os_layer_memset(&g_hb_mw_gatts_cb, 0, sizeof(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T));
        g_hb_mw_gatts_cb.bt_gatts_event_cb = BtAppGATTSMwEventCbk;
        g_hb_mw_gatts_cb.bt_gatts_reg_server_cb = BtAppGATTSMwRegServerCbk;
        g_hb_mw_gatts_cb.bt_gatts_add_srvc_cb = BtAppGATTSMwAddSrvcCbk;
        g_hb_mw_gatts_cb.bt_gatts_add_incl_cb = BtAppGATTSMwAddInclCbk;
        g_hb_mw_gatts_cb.bt_gatts_add_char_cb = BtAppGATTSMwAddCharCbk;
        g_hb_mw_gatts_cb.bt_gatts_add_desc_cb = BtAppGATTSMwAddDescCbk;
        g_hb_mw_gatts_cb.bt_gatts_op_srvc_cb = BtAppGATTSMwOpSrvcCbk;
        g_hb_mw_gatts_cb.bt_gatts_req_read_cb = BtAppGATTSMwReqReadCbk;
        g_hb_mw_gatts_cb.bt_gatts_req_write_cb = BtAppGATTSMwReqWriteCbk;
        g_hb_mw_gatts_cb.bt_gatts_ind_sent_cb = BtAppGATTSMwIndSentCbk;
        g_hb_mw_gatts_cb.bt_gatts_config_mtu_cb = BtAppGATTSMwConfigMtuCbk;
        g_hb_mw_gatts_cb.bt_gatts_exec_write_cb = BtAppGATTSMwExecWrite;
        a_mtkapi_bt_gatts_base_init(&g_hb_mw_gatts_cb, NULL);

        // Set local name
        a_mtkapi_bt_gap_set_name("LOCAL_NAME");

        // Set COD
        a_mtkapi_bt_gap_set_cod(0x000508)

    #2, BT Power On
        // BT On
        a_mtkapi_bt_gap_on_off(1);

        // Set EIR
        a_mtkapi_bt_gap_set_eir("EIR DATA", 8);

        // Set VID
        a_mtkapi_bt_gap_set_vendor_id(0x045e);

        // Set PID
        a_mtkapi_bt_gap_set_product_id(0x02e0);

        // Set Scan Mode
        a_mtkapi_bt_gap_set_connectable_and_discoverable(1,1);

    #3, BT Power Off
        a_mtkapi_bt_gap_on_off(0);

    #4, Start SPP
        a_mtkapi_spp_start_server((CHAR*)"Standard SPP", (CHAR*)"00001101-0000-1000-8000-00805f9b34fb");

    #5, GATT_S: Register GATT Server / Service / Characteristic / Descriptor
        // The following is a copy of Microsoft Designer Mouse. (BLE)
        BT_LOGD(MODULE_NAME, "Add Service 0x1800");
        // Generic Access, srvc_handle=1
        a_mtkapi_bt_gatts_add_service(0, (CHAR*)"1800", 1, 7);
        //      Device Name, attr_handle=3
        a_mtkapi_bt_gatts_add_char(0, 1, (CHAR*)"2A00", 0x0A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Appearance, attr_handle=5
        a_mtkapi_bt_gatts_add_char(0, 1, (CHAR*)"2A01", 0x02, BT_GATTS_REC_PERM_READABLE);
        //      Peripheral Preferred Connection Parameters, attr_handle=7
        a_mtkapi_bt_gatts_add_char(0, 1, (CHAR*)"2A04", 0x02, BT_GATTS_REC_PERM_READABLE);

        BT_LOGD(MODULE_NAME, "Add Service 0x1801");
        // Generic Attribute, srvc_handle=8
        a_mtkapi_bt_gatts_add_service(1, (CHAR*)"1801", 1, 1);

        BT_LOGD(MODULE_NAME, "Add Service 0x180A");
        // Device Information, srvc_handle=9
        a_mtkapi_bt_gatts_add_service(2, (CHAR*)"180A", 1, 5);
        //      Manufacturer String, attr_handle=11
        a_mtkapi_bt_gatts_add_char(2, 9, (CHAR*)"2A29", 0x02, BT_GATTS_REC_PERM_READABLE);
        //      PnP ID, attr_handle=13
        a_mtkapi_bt_gatts_add_char(2, 9, (CHAR*)"2A50", 0x02, BT_GATTS_REC_PERM_READABLE);

        BT_LOGD(MODULE_NAME, "Add Service 0x180F");
        // Battery Service, srv_handle=14
        a_mtkapi_bt_gatts_add_service(3, (CHAR*)"180F", 1, 4);
        //      Battery Level, attr_handle=16
        a_mtkapi_bt_gatts_add_char(3, 14, (CHAR*)"2A19", 0x12, BT_GATTS_REC_PERM_READABLE);
        //      Descriptors, descr_handle=17
        a_mtkapi_bt_gatts_add_desc(3, 14, (CHAR*)"2902", BT_GATTS_REC_PERM_READABLE);

        BT_LOGD(MODULE_NAME, "Add Service 0x1812");
        // Human Interface Device, srv_handle=18
        a_mtkapi_bt_gatts_add_service(4, (CHAR*)"1812", 1, 26);
        //      Protocol Mode, attr_handle=20
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4E", 0x06, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);

        //      HID Report, attr_handle=22
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4D", 0x1A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=23
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2902", BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=24
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2908", BT_GATTS_REC_PERM_READABLE);

        //      HID Report, attr_handle=26
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4D", 0x1A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=27
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2902", BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=28
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2908", BT_GATTS_REC_PERM_READABLE);

        //      HID Report, attr_handle=30
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4D", 0x0A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=31
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2908", BT_GATTS_REC_PERM_READABLE);

        //      HID Report, attr_handle= 33
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4D", 0x0A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=34
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2908", BT_GATTS_REC_PERM_READABLE);

        //     Report Map, attr_handle= 36
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4B", 0x02, BT_GATTS_REC_PERM_READABLE);

        //     Boot Mouse Input Report, attr_handle = 38
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A33", 0x1A, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);
        //      Descriptor, descr_handle=39
        a_mtkapi_bt_gatts_add_desc(4, 18, (CHAR*)"2902", BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE);

        //     HID Information, attr_handle= 41
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4A", 0x02, BT_GATTS_REC_PERM_READABLE);

        //     HID Control Point, attr_handle= 43
        a_mtkapi_bt_gatts_add_char(4, 18, (CHAR*)"2A4C", 0x04, BT_GATTS_REC_PERM_WRITABLE);

        BT_LOGD(MODULE_NAME, "Start Service 1");
        a_mtkapi_bt_gatts_start_service(0, 1, 2);
        BT_LOGD(MODULE_NAME, "Start Service 8");
        a_mtkapi_bt_gatts_start_service(1, 8, 2);
        BT_LOGD(MODULE_NAME, "Start Service 9");
        a_mtkapi_bt_gatts_start_service(2, 9, 2);
        BT_LOGD(MODULE_NAME, "Start Service 14");
        a_mtkapi_bt_gatts_start_service(3, 14, 2);
        BT_LOGD(MODULE_NAME, "Start Service 18");
        a_mtkapi_bt_gatts_start_service(4, 18, 2);


    #6, GATT_C: Register GATT Client
        UINT8 set_scan_rsp;
        UINT8 include_name;
        UINT8 include_txpower;
        UINT16 appearance = 0x03c4;                         // 0x03c4 for Gamepad
        UINT8 manufacturer_data[3] = {0x46, 0x00, 0x00};    // 0x0046 for Mediatek
        UINT8 service_uuid[2] = {0xf1, 0xff};               // 0xfff1 for service uuid
        UINT8 service_data[3] = {0xf2, 0xff, 0x00};         // 0xfff2 for service data uuid, 0x00 for service data

        BT_LOGD(MODULE_NAME, "set adv data");
        set_scan_rsp = 0;
        include_name = 0;
        include_txpower = 1;
        a_mtkapi_bt_gattc_multi_adv_setdata(1, set_scan_rsp, include_name, include_txpower, appearance,
                                                sizeof(manufacturer_data), (CHAR*)manufacturer_data,
                                                sizeof(service_data), (CHAR*)service_data,
                                                sizeof(service_uuid), (CHAR*)service_uuid);

    #7, GATT_C: Set Advertising Data
        UINT8 set_scan_rsp;
        UINT8 include_name;
        UINT8 include_txpower;
        UINT16 appearance = 0x03c4;                         // 0x03c4 for Gamepad
        UINT8 manufacturer_data[3] = {0x46, 0x00, 0x00};    // 0x0046 for Mediatek
        UINT8 service_uuid[2] = {0xf1, 0xff};               // 0xfff1 for service uuid
        UINT8 service_data[3] = {0xf2, 0xff, 0x00};         // 0xfff2 for service data uuid, 0x00 for service data

        BT_LOGD(MODULE_NAME, "set adv data");
        set_scan_rsp = 0;
        include_name = 0;
        include_txpower = 1;
        a_mtkapi_bt_gattc_multi_adv_setdata(1, set_scan_rsp, include_name, include_txpower, appearance,
                                                sizeof(manufacturer_data), (CHAR*)manufacturer_data,
                                                sizeof(service_data), (CHAR*)service_data,
                                                sizeof(service_uuid), (CHAR*)service_uuid);

        set_scan_rsp = 1;
        include_name = 1;
        include_txpower = 0;
        a_mtkapi_bt_gattc_multi_adv_setdata(1, set_scan_rsp, include_name, include_txpower, appearance,
                                                0, NULL,
                                                0, NULL,
                                                0, NULL);

    #8, GATT_C: Enable BLE Advertising
        INT32 client_if = 1;
        INT32 min_interval = 160;   // 100ms
        INT32 max_interval = 160;   // 100ms
        INT32 adv_type = 0;         // BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED
        INT32 chnl_map = 7;         // CH37 & CH38 & CH39
        INT32 tx_power = 20;        // 20dBm
        INT32 timeout_s = 0;        // not used

        BT_LOGD(MODULE_NAME, "adv enable");
        a_mtkapi_bt_gattc_multi_adv_enable(client_if, min_interval, max_interval,
                                           adv_type, chnl_map, tx_power, timeout_s);
 ******************************************************************************/

#ifndef _MTK_BT_SERVICE_GAP_WRAPPER_H_
#define _MTK_BT_SERVICE_GAP_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_gap.h"

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* GAP Type definitions for callback functions                                */
/******************************************************************************/
typedef VOID (*mtkrpcapi_BtAppGapEventCbk)(BTMW_GAP_EVT *bt_event, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGapGetPairingKeyCbk)(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGapInquiryResponseCbk)(BTMW_GAP_DEVICE_INFO* pt_result, void* pv_tag);

typedef struct
{
    mtkrpcapi_BtAppGapEventCbk bt_event_cb;
    mtkrpcapi_BtAppGapGetPairingKeyCbk bt_get_pairing_key_cb;
    mtkrpcapi_BtAppGapInquiryResponseCbk bt_dev_info_cb;
} MTKRPCAPI_BT_APP_CB_FUNC;

/*******************************************************************************
** Function         a_mtkapi_bt_gap_base_init
** Description      Initialize of the callback function pointer.
**                  This is the first API be invoked after system boot up.
**                  Note 1: This API does only involve with callback function
**                          pointer registration.
**                          This API does NOT involve with BT controller.
**                  Note 2: Users invokes this API for only "one time". When BT
**                          powering Off and On, no need to invoke this API again.
** Parameters       [in]MTKRPCAPI_BT_APP_CB_FUNC*: Callback function pointers.
**                  [in]VOID* pv_tag: MW will store this pointer and pass
**                          this parameter when callback to APP layer.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_base_init(MTKRPCAPI_BT_APP_CB_FUNC *func, VOID *pv_tag);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_base_uninit
** Description      Initialize of the callback function pointer.
**                  This is the first API be invoked when want to do bt uninit.
**                  Note 1: This API does only involve with callback function
**                          pointer registration.
**                          This API does NOT involve with BT controller on N9.
**                  Note 2: Before call the API, please make sure init has twice,
                            If has only one the uninit will return fail.
                            Thus make sure upper has one init at least one.
** Parameters       [in]MTKRPCAPI_BT_APP_CB_FUNC*: Callback function pointers.
**                  [in]VOID* pv_tag: Hummingbird will store this pointer and pass
**                          this parameter when callback to APP layer.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_base_uninit(MTKRPCAPI_BT_APP_CB_FUNC * func, VOID* pv_tag);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_on_off
** Description      BT power On / Off.
**                  When power on, this function returns value synchronization.
**                  When power off, this function returns value asynchronous and receive callback bt_event_cb(BT_POWER_OFF_CNF).
**                  Note 1: This API should be invoked after a_mtkapi_bt_gap_base_init.
**                  Note 2: If customization of Device Name is necessary, this
**                          API should be invoked after a_mtkapi_bt_gap_set_name.
** Parameters       [in]fg_on
**                    0   : BT Power Off
**                    else: BT Power On
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_on_off(BOOL fg_on);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_factory_reset
** Description      BT factory reset.
** Parameters       N/A
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_factory_reset(VOID);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_set_name
** Description      Set local device name.
**                  Note! This API should be invoked before BT Power On:
**                  a_mtkapi_bt_gap_on_off().
** Parameters       [in]name
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_set_name(CHAR *name);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_set_connectable_and_discoverable
** Description      Enable/Disable of Page_Scan or Inquiry_Scan.
**                  NOTE: This API should be invoked after BT Power On.
** Parameters       [in]fg_conn: for Page_Scan
**                  [in]fg_disc: for Inquiry_Scan
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_get_dev_info
** Description      Get peer device info by address.
** Parameters       [out]dev_info: device information such as name, cod, rssi, device type etc.
**                  [in]addr: device address.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_get_dev_info(BLUETOOTH_DEVICE *dev_info, CHAR *addr);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_get_bond_state
** Description      Get bond state by address.
** Parameters       [in]addr: device address.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_get_bond_state(CHAR *addr);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_get_local_dev_info
** Description      Get local device information.
** Parameters       [out]dev_info: device information.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_get_local_dev_info(BT_LOCAL_DEV *ps_dev_info);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_start_inquiry
** Description      Start inquiry for at most 12.8 seconds or at most 64 devices.
** Parameters       [in]ui4_filter_type: filter type.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_start_inquiry(UINT32 ui4_filter_type);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_stop_inquiry
** Description      Stop inquiry immediately.
**                  This API is used to stop inquiry before inquiry complete.
** Parameters       N/A
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_stop_inquiry(VOID);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_pair
** Description      Pair remote bluetooth device.
**                  This function returns value asynchronous.
**                  bt_event_cb callback with event GAP_STATE_BONDING/GAP_STATE_BONDED/GAP_STATE_NO_BOND
** Parameters       [in]addr: remote device address in "string" format.
**                            ex, e.g. "AA:BB:CC:DD:EE:FF" [NAP-AA:BB][UAP-CC][LAP-DD:EE:FF]
**                  [in]transport: BT_TRANSPORT_TYPE_BR_EDR or BT_TRANSPORT_TYPE_BLE.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_pair(CHAR *addr, int transport);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_unpair
** Description      Un-Pair remote bluetooth device.
**                  Note: This API only involve with "remove link key from pairing table".
**                        This API does NOT involve with "disconnect".
**                        Caller of this API is responsible for disconnect
**                        remote device before invoking this API.
** Parameters       [in]addr: remote device address in "string" format.
**                            ex, "11:22:33:44:55:66"
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_unpair(CHAR *addr);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_unpair
** Description      Cancel-Pair remote bluetooth device.
** Parameters       [in]addr: remote device address in "string" format.
**                            ex, "11:22:33:44:55:66"
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_cancel_pair(CHAR *addr);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_interop_database_clear
** Description      Clear interop database.
** Parameters       N/A
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_interop_database_clear(VOID);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_interop_database_add
** Description      Add a new device interoperability workaround for a remote device whose
**                  first |len| bytes of the its device address match |address |
** Parameters       [in]addr: remote device address in "string" format.
**                            ex, "11:22:33:44:55:66"
**                  [in]feature: interop feature.
**                  [in]len: the first |len| bytes of the its device address.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_interop_database_add(CHAR *addr, BTMW_GAP_INTEROP_FEATURE feature, UINT8 len);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_get_rssi
** Description      Get rssi info by specified device address.
** Parameters       [in]addr: remote device address in "string" format.
**                            ex, "11:22:33:44:55:66"
**                  [out]rssi_value: rssi value.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_get_rssi(CHAR *addr, INT16 *rssi_value);

/*******************************************************************************
** Function         a_mtkapi_bt_gap_send_hci
** Description      Send HCI command to BT controller.
** Parameters       [in]buffer: HCI command raw data in "string" format.
**                      ex, For HCI_RESET, "01030c00" should be used.
**                          This API would translate it to 0x01030c00.
** Returns          BT_ERR_STATUS_T
*******************************************************************************/
extern INT32 a_mtkapi_bt_gap_send_hci(CHAR *buffer);

extern INT32 c_rpc_reg_mtk_bt_service_gap_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
