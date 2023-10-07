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


/* FILE NAME:  bt_mw_msg.c
 * AUTHOR: Changlu Yi
 * PURPOSE:
 *      It provides bluetooth common structure to MSG.
 * NOTES:
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/prctl.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include"u_bt_mw_common.h"
#include"bt_mw_log.h"
#include"bt_mw_common.h"
#include"bt_mw_message_queue.h"
//#include"bt_mw_common.h"
//#include"bluetooth_sync.h"
//#include"linuxbt_common.h"

#define LINUX_BT_MSG_ID 100
#define BT_NTY_MSG_ID 111

//#define BT_MW_DBG_FIFO "/tmp/bt_cmd_fifo"
//#define BT_MW_DBG_LIB "libbtmw-test.so"

typedef struct
{
    tBTMW_EVENT_HDR           *reg[BTWM_ID_MAX];       /* registration structures */
    BOOLEAN                 is_reg[BTWM_ID_MAX];     /* registration structures */
} tBTMW_CB;

typedef struct
{
    tBTMW_EVENT_HDR           *reg[BTWM_ID_MAX];       /* registration structures */
    BOOLEAN                          is_reg[BTWM_ID_MAX];     /* registration structures */
} tBTMW_NTY_CB;


/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/
static pthread_t  h_linuxbt_thread;
static int linuxbt_thread_should_stop = 0;
static INT32   linuxbt_msg_queue_id = 0;
tBTMW_CB btmw_cb;

static pthread_t  h_mw_nty_thread;
static int mw_nty_thread_should_stop = 0;
//static pthread_t  h_mw_dbg_thread;
static INT32   bt_nty_msg_queue_id = 0;
tBTMW_NTY_CB btmw_nty_cb;

void linuxbt_hdl_register(UINT8 id, tBTMW_EVENT_HDR *p_reg)
{
    if(id >= BTWM_ID_MAX)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM,"Enter %s , error register id = %d ", __FUNCTION__, id);
        return;
    }
    BT_DBG_NORMAL(BT_DEBUG_COMM,"Enter %s , register id = %d ", __FUNCTION__, id);

    btmw_cb.reg[id] = (tBTMW_EVENT_HDR *) p_reg;
    if (NULL != p_reg)
    {
        btmw_cb.is_reg[id] = TRUE;
    }
    else
    {
        btmw_cb.is_reg[id] = FALSE;
    }
}

INT32 linuxbt_send_msg(tBTMW_MSG* msg)
{
    //BT_DBG_ERROR(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);
    INT32 i4Ret;
    tBTMW_MSG_T msg_struct = { 0 };
    size_t tx_size = 0;
    msg_struct.tMsgType = 1;
    memcpy(&(msg_struct.body), msg, sizeof(tBTMW_MSG) );

    if (msg->hdr.len != 0)
    {
        tx_size = msg->hdr.len + sizeof(msg->hdr);
    }
    else
    {
        tx_size = sizeof(tBTMW_MSG);
    }
    i4Ret = msgsnd(linuxbt_msg_queue_id, &msg_struct, tx_size, 0 );
    if (-1 == i4Ret)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM,"send %zu(%zu) failed %s(%ld)!", tx_size, msg->hdr.len, strerror(errno), (long)errno);
        return BT_ERR_STATUS_FAIL;
    }
    else
    {
        //BT_DBG_NORMAL(BT_DEBUG_COMM,"send %d bytes!", tx_size);
    }

    return BT_SUCCESS;
}


VOID linuxbt_msg_handler(tBTMW_MSG* msg)
{


    //BOOLEAN     freebuf = TRUE;
    //BTMW_HDR *p_msg = ( BTMW_HDR *)msg;
    //BT_DBG_ERROR(BT_DEBUG_COMM,"Enter %s, BTMW got EVENT:%d", __FUNCTION__, p_msg->event);

    UINT16 id =  (msg->hdr.event >> 8);

    if((id < BTWM_ID_MAX) && (btmw_cb.reg[id] != NULL))
    {
        (*(btmw_cb.reg[id]))(msg);
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM,"BTMW got unregistered event id:%d", id);
    }

}

// this thread is used for recv msg sent from callback
static VOID* linuxbt_msg_recv_thread(VOID * args)
{
    //BT_DBG_NORMAL(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    INT32 ret = 0;
    tBTMW_MSG_T msg_struct;
    tBTMW_MSG *t_msg;

    prctl(PR_SET_NAME, "linuxbt_msg_recv_new", 0, 0, 0);


    while(1)
    {
        memset(&msg_struct, 0, sizeof(msg_struct));
        ret = msgrcv( linuxbt_msg_queue_id, &msg_struct, sizeof(tBTMW_MSG), 0, 0 );
        //BT_DBG_NORMAL(BT_DEBUG_COMM,"linuxbt_msg_recv_thread recv msg size: %ld ", (long)ret);

        t_msg = &(msg_struct.body);
        if ( ret > 0 )
        {
            linuxbt_msg_handler(t_msg);
        }
        else if ( ret == 0 )
        {
            /*fprintf(stdout, "%s: no message received ", __FUNCTION__);*/
            BT_DBG_ERROR(BT_DEBUG_COMM, "%s: no message received ", __FUNCTION__);
        }
        else
        {
            /*fprintf(stdout, "%s: receive message failed %d", __FUNCTION__, errno);*/
            BT_DBG_ERROR(BT_DEBUG_COMM, "%s: receive message failed %d", __FUNCTION__, errno);
        }

        if (linuxbt_thread_should_stop == 1)
            break;
    }
    FUNC_EXIT;

    return NULL;
}

INT32 linuxbt_msg_queue_init_new(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    key_t t_key;
    UINT32 ui4_que_id;
    INT32 i4_ret = 0;

    t_key = ftok(".", LINUX_BT_MSG_ID );
    if (-1 == t_key)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "linuxbt_msg_queue_init_new get key failed %ld!!", (long)errno);
        return BT_ERR_STATUS_FAIL;
    }

    ui4_que_id = msgget(t_key, IPC_CREAT | 0777);
    if ((-1) == ui4_que_id)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "linuxbt_msg_queue_init_new get queue id failed!!");
        return BT_ERR_STATUS_FAIL;
    }
    linuxbt_msg_queue_id = ui4_que_id;
    linuxbt_thread_should_stop = 0;

    BT_DBG_NORMAL(BT_DEBUG_COMM, "+++ linuxbt_msg_queue_init_new +++");
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_linuxbt_thread,
                                          &attr,
                                          linuxbt_msg_recv_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }
    pthread_attr_destroy(&attr);

    return BT_SUCCESS;
}

INT32 linuxbt_msg_queue_deinit_new(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_COMM, "Enter %s", __FUNCTION__);

    linuxbt_thread_should_stop = 1;
    pthread_join(h_linuxbt_thread, NULL);

    return 0;
}

void bt_mw_nty_hdl_register(UINT8 id, tBTMW_EVENT_HDR *p_reg)
{
    if(id >= BTWM_ID_MAX)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM,"error register id = %d ",  id);
        return ;
    }
    BT_DBG_ERROR(BT_DEBUG_COMM,"register id = %d ", id);

    btmw_nty_cb.reg[id] = (tBTMW_EVENT_HDR *) p_reg;
    btmw_nty_cb.is_reg[id] = TRUE;
}

INT32 bt_mw_nty_send_msg(tBTMW_MSG* msg)
{
    //BT_DBG_INFO(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    INT32 i4Ret;
    tBTMW_MSG_T msg_struct = { 0 };
    size_t tx_size = 0;
    msg_struct.tMsgType = 1;
    memcpy(&(msg_struct.body), msg, sizeof(tBTMW_MSG) );

    if (msg->hdr.len != 0)
    {
        tx_size = msg->hdr.len + sizeof(msg->hdr);
    }
    else
    {
        tx_size = sizeof(tBTMW_MSG);
    }
    i4Ret = msgsnd(bt_nty_msg_queue_id, &msg_struct, tx_size, 0 );
    if (-1 == i4Ret)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM,"send %zu(%zu) failed %s(%ld)!", tx_size, msg->hdr.len, strerror(errno), (long)errno);
        return BT_ERR_STATUS_FAIL;
    }
    else
    {
        //BT_DBG_NORMAL(BT_DEBUG_COMM,"send %d bytes!", tx_size);
    }
    //return BT_SUCCESS;
    return 0;
}

VOID bt_mw_nty_msg_handler(tBTMW_MSG* msg)
{
    BT_DBG_INFO(BT_DEBUG_COMM,"BTMW NTY got EVENT:%d", msg->hdr.event);

    UINT16 id =  (msg->hdr.event >> 8);

    if((id < BTWM_ID_MAX) && (btmw_nty_cb.reg[id] != NULL))
    {
        (*(btmw_nty_cb.reg[id]))(msg);
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM,"BTMW got unregistered event id:%d", id);
    }

}

// this thread is used for recv msg sent from callback
static VOID* bt_nty_msg_recv_thread(VOID * args)
{
    BT_DBG_INFO(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    INT32 ret = 0;
    tBTMW_MSG_T msg_struct;
    tBTMW_MSG *t_msg;

    prctl(PR_SET_NAME, "bt_mw_nty_msg_recv_new", 0, 0, 0);

    while(1)
    {
        memset(&msg_struct, 0, sizeof(msg_struct));
        ret = msgrcv( bt_nty_msg_queue_id, &msg_struct, sizeof(tBTMW_MSG), 0, 0 );
        //BT_DBG_NORMAL(BT_DEBUG_COMM,"recv msg size: %ld ", (long)ret);

        t_msg = &(msg_struct.body);
        if ( ret > 0 )
        {
            bt_mw_nty_msg_handler(t_msg);
        }
        else if ( ret == 0 )
        {
            /*fprintf(stdout, "%s: no message received ", __FUNCTION__);*/
            BT_DBG_ERROR(BT_DEBUG_COMM, "%s: no message received ", __FUNCTION__);
        }
        else
        {
            /*fprintf(stdout, "%s: receive message failed %d", __FUNCTION__, errno);*/
            BT_DBG_ERROR(BT_DEBUG_COMM, "%s: receive message failed %d", __FUNCTION__, errno);
        }

        if (mw_nty_thread_should_stop == 1)
            break;
    }
    FUNC_EXIT;

    return NULL;
}

#if 0
static VOID* bt_dbg_msg_recv_thread(VOID * args)
{
    INT32 ret = 0;
    INT32 fifoFd = -1;
    char cmd_str[257] = {0};

    prctl(PR_SET_NAME, "bt_mw_dbg_msg_recv_new", 0, 0, 0);
    BT_DBG_INFO(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    while(1)
    {
        memset(cmd_str, 0, sizeof(cmd_str));
        if (-1 == fifoFd)
        {
            fifoFd = open(BT_MW_DBG_FIFO, O_RDONLY);
            if (fifoFd < 0)
            {
                BT_DBG_ERROR(BT_DEBUG_COMM, "%s fifo open fail:%d\n", BT_MW_DBG_FIFO, ret);
                return NULL;
            }
        }

        BT_DBG_NORMAL(BT_DEBUG_COMM, "start to read\n");
        ret = read(fifoFd, cmd_str, 256);
        cmd_str[256] = '\0';
        if (ret <= 0) {
            BT_DBG_ERROR(BT_DEBUG_COMM, "read result%d\n ", ret);
            close(fifoFd);
            fifoFd = -1;
            continue;
        }

        if (256 != ret) {
            //the cmd data is not equal to 128bytes. Drop it.
            BT_DBG_ERROR(BT_DEBUG_COMM, "%s size (%d) <> 256)\n", __func__, ret);
            continue;
        }

        BT_DBG_NORMAL(BT_DEBUG_COMM, "cmd_str=%s\n", cmd_str);
        bt_mw_dbg_handle_cmd(cmd_str);
    }
    FUNC_EXIT;

    return NULL;
}
#endif

INT32 bt_mw_nty_queue_init_new(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    UINT32 ui4_que_id;
    INT32 i4_ret = 0;

    ui4_que_id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
    if ((-1) == ui4_que_id)
    {
        /*fprintf(stdout, "linuxbt_msg_queue_init get queue id failed!!");*/
        BT_DBG_ERROR(BT_DEBUG_COMM, " get queue id failed!!");
        //return BT_ERR_STATUS_FAIL;
        return -1;
    }
    bt_nty_msg_queue_id = ui4_que_id;
    mw_nty_thread_should_stop = 0;

    BT_DBG_NORMAL(BT_DEBUG_COMM, "++++++++++");
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_mw_nty_thread,
                                          &attr,
                                          bt_nty_msg_recv_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    pthread_attr_destroy(&attr);
    //return BT_SUCCESS;
    return 0;
}

INT32 bt_mw_nty_queue_deinit_new(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_COMM, "Enter %s", __FUNCTION__);

    mw_nty_thread_should_stop = 1;
    pthread_join(h_mw_nty_thread, NULL);

    return 0;
}
#if 0

INT32 bt_mw_dbg_queue_init_new(VOID)
{
    INT32 i4_ret = 0;
    BT_DBG_NORMAL(BT_DEBUG_COMM,"Enter %s", __FUNCTION__);

    i4_ret = mkfifo(BT_MW_DBG_FIFO, 0777);
    if (i4_ret < 0) {
        if (errno != EEXIST) {
            BT_DBG_ERROR(BT_DEBUG_COMM, "mkfifo "BT_MW_DBG_FIFO" fail:%d\n", i4_ret);
            return -1;
        }
    }

    bt_mw_dbg_load_dbg_func();

    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_mw_dbg_thread,
                                          &attr,
                                          bt_dbg_msg_recv_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    pthread_attr_destroy(&attr);

    return 0;
}
#endif

