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


/* FILE NAME:  bt_mw_common.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth common structure to wrap.
 * NOTES:
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "bluetooth.h"
#include "linuxbt_common.h"

static const UINT8  linuxbt_base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                       0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

INT32 linuxbt_check_and_convert_addr(CHAR *addr, RawAddress &rawAddr)
{
  if (addr && RawAddress::FromString(addr, rawAddr))
    return BT_SUCCESS;
  else
    return BT_ERR_STATUS_PARM_INVALID;
}

INT32 linuxbt_btaddr_stoh(CHAR *btaddr_s, RawAddress *bdaddr_h)
{
    INT32 i;
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
    return BT_SUCCESS;
}

INT32 linuxbt_mesh_btaddr_stoh(CHAR *btaddr_s, UINT8 *bdaddr_h)
{
    INT32 i;
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (i = 0; i <6; i++)
    {
        bdaddr_h[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
    return BT_SUCCESS;
}

INT32 linuxbt_mesh_btaddr_htos(UINT8 *bdaddr_h, CHAR *btaddr_s)
{
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    (void)snprintf(btaddr_s, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             bdaddr_h[0],
             bdaddr_h[1],
             bdaddr_h[2],
             bdaddr_h[3],
             bdaddr_h[4],
             bdaddr_h[5]);
    btaddr_s[17] = '\0';

    return BT_SUCCESS;
}

INT32 linuxbt_btaddr_htos(RawAddress *bdaddr_h, CHAR *btaddr_s)
{
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    (void)snprintf(btaddr_s, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             bdaddr_h->address[0],
             bdaddr_h->address[1],
             bdaddr_h->address[2],
             bdaddr_h->address[3],
             bdaddr_h->address[4],
             bdaddr_h->address[5]);
    btaddr_s[17] = '\0';

    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 ascii_2_hex(CHAR *p_ascii, INT32 len, UINT8 *p_hex)
{
    INT32     x;
    UINT8     c;

    if (NULL == p_ascii || NULL == p_hex)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (x = 0; (x < len) && (*p_ascii); x++)
    {
        if (isdigit (*p_ascii))
            c = (*p_ascii - '0') << 4;
        else
            c = (toupper(*p_ascii) - 'A' + 10) << 4;
        p_ascii++;
        if (*p_ascii)
        {
            if (isdigit (*p_ascii))
                c |= (*p_ascii - '0');
            else
                c |= (toupper(*p_ascii) - 'A' + 10);
            p_ascii++;
        }
        *p_hex++ = c;
    }
    return x;
}

BT_ERR_STATUS_T linuxbt_return_value_convert(int ret)
{
    BT_ERR_STATUS_T status = BT_SUCCESS;
    if (BT_STATUS_SUCCESS == ret)
    {
        status = BT_SUCCESS;
    }
    else if (BT_STATUS_FAIL == ret)
    {
        status = BT_ERR_STATUS_FAIL;
    }
    else if (BT_STATUS_NOT_READY== ret)
    {
        status = BT_ERR_STATUS_NOT_READY;
    }
    else if (BT_STATUS_NOMEM== ret)
    {
        status = BT_ERR_STATUS_NOMEM;
    }
    else if (BT_STATUS_BUSY== ret)
    {
        status = BT_ERR_STATUS_BUSY;
    }
    else if (BT_STATUS_DONE== ret)
    {
        status = BT_ERR_STATUS_DONE;
    }
    else if (BT_STATUS_UNSUPPORTED== ret)
    {
        status = BT_ERR_STATUS_UNSUPPORTED;
    }
    else if (BT_STATUS_PARM_INVALID== ret)
    {
        status = BT_ERR_STATUS_PARM_INVALID;
    }
    else if (BT_STATUS_UNHANDLED== ret)
    {
        status = BT_ERR_STATUS_UNHANDLED;
    }
    else if (BT_STATUS_AUTH_FAILURE== ret)
    {
        status = BT_ERR_STATUS_AUTH_FAILURE;
    }
    else if (BT_STATUS_RMT_DEV_DOWN== ret)
    {
        status = BT_ERR_STATUS_RMT_DEV_DOWN;
    }
    return status;
}

const char *linuxbt_get_bt_status_str(const bt_status_t status)
{
    switch (status)
    {
        LINUXBT_CASE_RETURN_STR(BT_STATUS_SUCCESS, "success");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_FAIL, "fail");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_NOT_READY, "not ready");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_NOMEM, "no mem");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_DONE, "done");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_BUSY, "busy");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_UNSUPPORTED, "unspported");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_PARM_INVALID, "param invalid");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_UNHANDLED, "unhandle");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_AUTH_FAILURE, "auth fail");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_RMT_DEV_DOWN, "dev down");
        LINUXBT_CASE_RETURN_STR(BT_STATUS_AUTH_REJECTED, "auth rej");
        default: return "unknown";
    }
}

