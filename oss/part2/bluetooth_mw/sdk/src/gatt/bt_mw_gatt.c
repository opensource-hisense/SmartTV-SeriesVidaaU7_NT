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

/* INCLUDE FILE DECLARATIONS
 */

#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "bt_mw_log.h"
#include "bt_mw_gatt.h"
#include "bt_mw_gap.h"
#include "linuxbt_gatt_if.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_GATT_LOCK() do{                           \
        pthread_mutex_lock(&s_bt_mw_gatt_lock);      \
    } while(0)

#define BT_MW_GATT_UNLOCK() do{                         \
        pthread_mutex_unlock(&s_bt_mw_gatt_lock);    \
    } while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    tBTMW_GATT_EVENT_HDR           *reg;       /* registration structures */
    BOOL                         is_reg;     /* registration structures */
} tBTMW_GATT_NTY_CB;


typedef struct
{
    INT32 ref_cnt;
} BT_MW_GATT_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static INT32 bt_mw_gatt_nty_queue_init_new(VOID);
static INT32 bt_mw_gatt_nty_queue_uninit(VOID);
static VOID bt_mw_gatt_nty_hdl_register(tBTMW_GATT_EVENT_HDR *p_reg);
static VOID bt_mw_gatt_init_profile(VOID);
static VOID bt_mw_gatt_deinit_profile(VOID);
static VOID bt_mw_gatt_notify_thread_exit(VOID);
static VOID bt_mw_gatt_nty_handle(tBTMW_GATT_MSG *p_msg);
/* STATIC VARIABLE DECLARATIONS
 */
static BT_MW_GATT_CB g_bt_mw_gatt_cb;

static pthread_t  s_bt_mw_gatt_nty_thread;
static INT32      s_bt_mw_gatt_nty_msg_queue_id = 0;
static tBTMW_GATT_NTY_CB s_bt_mw_gatt_nty_cb;

static pthread_mutex_t s_bt_mw_gatt_lock = PTHREAD_MUTEX_INITIALIZER;

/* EXPORTED SUBPROGRAM BODIES
 */

INT32 bt_mw_gatt_init(VOID)
{
    profile_operator_t gatt_op;
    BT_MW_GATT_LOCK();
    if (0 != g_bt_mw_gatt_cb.ref_cnt)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT, "inited");
        g_bt_mw_gatt_cb.ref_cnt++;
        BT_MW_GATT_UNLOCK();
        return BT_ERR_STATUS_DONE;
    }

    memset((void*)&g_bt_mw_gatt_cb, 0, sizeof(g_bt_mw_gatt_cb));

    bt_mw_gatt_nty_hdl_register(bt_mw_gatt_nty_handle);
    memset(&gatt_op, 0, sizeof(gatt_op));
    gatt_op.init = bt_mw_gatt_init_profile;
    gatt_op.deinit = bt_mw_gatt_deinit_profile;
    gatt_op.notify_acl_state = NULL;

    bt_mw_gap_register_profile(BTWM_ID_GATT, &gatt_op);
    bt_mw_gatt_nty_queue_init_new();
    g_bt_mw_gatt_cb.ref_cnt++;
    BT_MW_GATT_UNLOCK();
    return BT_SUCCESS;
}

INT32 bt_mw_gatt_deinit(VOID)
{
    BT_MW_GATT_LOCK();
    g_bt_mw_gatt_cb.ref_cnt--;
    if (0 != g_bt_mw_gatt_cb.ref_cnt)
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT, "deinited");
        BT_MW_GATT_UNLOCK();
        return BT_ERR_STATUS_DONE;
    }

    memset((void*)&g_bt_mw_gatt_cb, 0, sizeof(g_bt_mw_gatt_cb));

    bt_mw_gatt_deinit_profile();

    bt_mw_gap_register_profile(BTWM_ID_GATT, NULL);

    bt_mw_gatt_notify_thread_exit();

    bt_mw_gatt_nty_queue_uninit();
    BT_MW_GATT_UNLOCK();

    return BT_SUCCESS;
}

INT32 bt_mw_gatt_nty_send_msg(tBTMW_GATT_MSG* msg)
{
    //BT_DBG_ERROR(BT_DEBUG_GATT, "Enter %s", __FUNCTION__);

    INT32 i4Ret;
    tBTMW_GATT_MSG_T gatt_msg_struct = {0};
    size_t tx_size = 0;
    gatt_msg_struct.tMsgType = 1;
    memcpy(&(gatt_msg_struct.body), msg, sizeof(tBTMW_GATT_MSG));

    if (msg->hdr.len != 0)
    {
        tx_size = msg->hdr.len + sizeof(msg->hdr);
    }
    else
    {
        tx_size = sizeof(tBTMW_GATT_MSG);
    }

    i4Ret = msgsnd(s_bt_mw_gatt_nty_msg_queue_id, (VOID*)&gatt_msg_struct, tx_size, 0 );
    if (-1 == i4Ret)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"send %zu failed %s(%ld)!", tx_size, strerror(errno), (long)errno);
        return BT_ERR_STATUS_FAIL;
    }
#if 0
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GATT,"send %zu bytes!", tx_size);
    }
#endif
    return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */

static VOID bt_mw_gatt_nty_handle(tBTMW_GATT_MSG *p_msg)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "bt_mw_gatt_nty_handle: event:%d", p_msg->hdr.event);

    switch(p_msg->hdr.event)
    {
        case BTMW_GATTC_EVENT:
        {
            BT_GATTC_EVENT_PARAM *gattc_msg = &p_msg->data.gattc_param;
            bt_mw_gattc_nty_handle(gattc_msg);
            break;
        }
        case BTMW_GATTS_EVENT:
        {
            BT_GATTS_EVENT_PARAM *gatts_msg = &p_msg->data.gatts_param;
            bt_mw_gatts_nty_handle(gatts_msg);
            break;
        }
        case BTMW_BLE_SCANER_EVENT:
        {
            BT_BLE_SCANNER_CALLBACK_PARAM *ble_scanner_msg = &p_msg->data.ble_scanner_param;
            bt_mw_ble_scanner_nty_handle(ble_scanner_msg);
            break;
        }
        case BTMW_BLE_ADVERTISER_EVENT:
        {
            BT_BLE_ADV_EVENT_PARAM *ble_adv_msg = &p_msg->data.ble_adv_param;
            bt_mw_ble_adv_nty_handle(ble_adv_msg);
            break;
        }
        default:
            break;
    }
}

static VOID bt_mw_gatt_init_profile(VOID)
{
    linuxbt_gatt_init();
    return;
}

static VOID bt_mw_gatt_deinit_profile(VOID)
{
    linuxbt_gatt_deinit();
    return;
}


static VOID bt_mw_gatt_notify_thread_exit(VOID)
{
    tBTMW_GATT_MSG msg = {0};
    msg.hdr.event = BTMW_GATT_THREAD_EXIT;
    bt_mw_gatt_nty_send_msg(&msg);
}



static VOID bt_mw_gatt_nty_hdl_register(tBTMW_GATT_EVENT_HDR *p_reg)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT,"p_reg = %p ", p_reg);

    s_bt_mw_gatt_nty_cb.reg = (tBTMW_GATT_EVENT_HDR *) p_reg;
    s_bt_mw_gatt_nty_cb.is_reg = TRUE;
}


static VOID bt_mw_gatt_nty_msg_handler(tBTMW_GATT_MSG* msg)
{
    //BT_DBG_ERROR(BT_DEBUG_GATT,"BTMW GATT NTY got EVENT:%d", msg->hdr.event);
    UINT16 id =  (msg->hdr.event >> 8);

    if((BTWM_ID_GATT == id) && (NULL != s_bt_mw_gatt_nty_cb.reg))
    {
        (*(s_bt_mw_gatt_nty_cb.reg))(msg);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"BTMW GATT got unregistered event id:%d", id);
    }

}

// this thread is used for recv msg sent from callback
static VOID* bt_mw_gatt_nty_msg_recv_thread(VOID * args)
{
    //BT_DBG_NORMAL(BT_DEBUG_GATT,"Enter %s", __FUNCTION__);

    INT32 ret = 0;
    tBTMW_GATT_MSG_T gatt_msg_struct;
    static tBTMW_GATT_MSG *t_gatt_msg;

    prctl(PR_SET_NAME, "bt_mw_gatt_nty_msg_recv_new", 0, 0, 0);

    while(1)
    {
        memset(&gatt_msg_struct, 0, sizeof(gatt_msg_struct));
        gatt_msg_struct.tMsgType = 1;
        ret = msgrcv(s_bt_mw_gatt_nty_msg_queue_id, (VOID*)&gatt_msg_struct, sizeof(tBTMW_GATT_MSG), 0, 0);
        //BT_DBG_NORMAL(BT_DEBUG_GATT, "recv msg size: %ld ", (long)ret);

        t_gatt_msg = &(gatt_msg_struct.body);
        if ( ret > 0 )
        {
            /* if receive BTMW_GATT_THREAD_EXIT, cancle the loop */
            if( t_gatt_msg->hdr.event == BTMW_GATT_THREAD_EXIT)
                break;
            bt_mw_gatt_nty_msg_handler(t_gatt_msg);
        }
        else if ( ret == 0 )
        {
            /*fprintf(stdout, "%s: no message received ", __FUNCTION__);*/
            BT_DBG_ERROR(BT_DEBUG_GATT, "%s: no message received ", __FUNCTION__);
        }
        else
        {
            /*fprintf(stdout, "%s: receive message failed %d", __FUNCTION__, errno);*/
            BT_DBG_ERROR(BT_DEBUG_GATT, "%s: receive message failed %d", __FUNCTION__, errno);
        }
    }
    if (0 != bt_mw_gatt_nty_queue_uninit())
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "uninit message queue fail!\n");
    }
    FUNC_EXIT;

    return NULL;
}

static INT32 bt_mw_gatt_nty_queue_init_new(VOID)
{
    //BT_DBG_NORMAL(BT_DEBUG_GATT,"Enter %s", __FUNCTION__);

    //static INT32 i4_linuxbt_inited = 0;
    //key_t t_key;
    UINT32 ui4_que_id;
    INT32 i4_ret = 0;

    /**
     * Create message queue
     */
    ui4_que_id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
    if ((-1) == ui4_que_id)
    {
        /*fprintf(stdout, "linuxbt_msg_queue_init get queue id failed!!");*/
        BT_DBG_ERROR(BT_DEBUG_GATT, " get queue id failed!!");
        //return BT_ERR_STATUS_FAIL;
        return -1;
    }
    s_bt_mw_gatt_nty_msg_queue_id = ui4_que_id;

    BT_DBG_NORMAL(BT_DEBUG_GATT, "++++++++++");
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&s_bt_mw_gatt_nty_thread,
                                          &attr,
                                          bt_mw_gatt_nty_msg_recv_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    pthread_attr_destroy(&attr);
    return 0;
}

static INT32 bt_mw_gatt_nty_queue_uninit(VOID)
{
    FUNC_ENTRY;
    if (-1 == msgctl(s_bt_mw_gatt_nty_msg_queue_id, IPC_RMID, NULL))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "msgctl failed!\n");
        return -1;
    }
    BT_DBG_NOTICE(BT_DEBUG_GATT, "end!\n");
    return 0;
}


