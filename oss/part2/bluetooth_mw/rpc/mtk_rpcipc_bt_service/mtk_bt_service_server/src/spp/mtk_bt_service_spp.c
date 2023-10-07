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

#include "mtk_bt_service_spp.h"
#include "c_bt_mw_spp.h"
#include "ri_common.h"


#define BT_RC_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static void *g_spp_app_pvtag = NULL;

static mtkrpcapi_BtAppSppCbk mtkrpcapi_BtSppEventCbk = NULL;

VOID MWBtSppEventCbk(BT_SPP_CBK_STRUCT *param)
{
    if (mtkrpcapi_BtSppEventCbk)
    {
        mtkrpcapi_BtSppEventCbk(param, g_spp_app_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtSppEventCbk is null\n");
    }
}

INT32 x_mtkapi_spp_register_callback(mtkrpcapi_BtAppSppCbk func, VOID* pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != func)
    {
        g_spp_app_pvtag = pv_tag;
        mtkrpcapi_BtSppEventCbk = func;

        i4_ret = c_btm_spp_register_callback(MWBtSppEventCbk);
    }
    else
    {
        if (NULL != g_spp_app_pvtag)
        {
            ri_free_cb_tag(g_spp_app_pvtag);
            g_spp_app_pvtag = NULL;
        }
        i4_ret = c_btm_spp_register_callback(NULL);
    }

    if (i4_ret != 0)
    {
        BT_RC_LOG("x_mtkapi_spp_register_callback fail\n");
    }

    return i4_ret;
}


INT32 x_mtkapi_spp_connect(BT_SPP_CONNECT_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_connect(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_connect fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_connect success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_disconnect(BT_SPP_DISCONNECT_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_disconnect(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_disconnect fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_disconnect success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_send_data(BT_SPP_SEND_DATA_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_send_data(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_send_data fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_send_data success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_start_server(BT_SPP_START_SVR_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_start_server(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_start_server fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_start_server success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_stop_server(BT_SPP_STOP_SVR_PARAM *param)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_stop_server(param);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_stop_server fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_stop_server success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_get_connection_info(BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_get_connection_info(spp_connection_info_db);
    return i4_ret;
}

