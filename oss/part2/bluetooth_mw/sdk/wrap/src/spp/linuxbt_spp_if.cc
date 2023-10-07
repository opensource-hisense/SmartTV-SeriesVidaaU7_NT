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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_sock.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_spp_if.h"
#include "bt_mw_spp.h"
//#include "bluetooth_sync.h"

using bluetooth::Uuid;

static btsock_interface_t *g_bt_sock_interface = NULL;
int data_fdr = -1;
int data_fdw = -1;
int listen_fdr = -1;
int listen_fdw = -1;

#define MAX_PFDS        (SPP_MAX_NUMBER + 1)
#define MAX_POLL        64
#define MAX_MSG_SIZE    8192

typedef struct _BT_SPP_STRUCT
{
    CHAR    bd_addr[MAX_BDADDR_LEN];
    int     sock_fd;
    int     channel_id;
    UINT8   spp_if;
    CHAR    uuid[MAX_UUID_LEN];
    int     uuid_len;
    BOOL    is_used;
    int     poll_num;
    BOOL    is_server;
}BT_SPP_STRUCT;

typedef struct _BT_SPP_LISTEN_STRUCT
{
    int     listen_fd;
    int     channel_id;
    UINT8   spp_if;
    CHAR    server_name[MAX_NAME_LEN];
    CHAR    uuid[MAX_UUID_LEN];
    int     uuid_len;
    BOOL    is_used;
    int     revent_num;
}BT_SPP_LISTEN_STRUCT;

BT_SPP_STRUCT           spp_struct[SPP_MAX_NUMBER];
BT_SPP_LISTEN_STRUCT    spp_listen_struct[SPP_MAX_NUMBER];

#define spp_print_events(events) do { \
    BT_DBG_INFO(BT_DEBUG_SPP, "print poll event:%x ", events); \
    if (events & POLLIN)  BT_DBG_INFO(BT_DEBUG_SPP, "   POLLIN "); \
    if (events & POLLPRI) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLPRI "); \
    if (events & POLLOUT) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLOUT "); \
    if (events & POLLERR) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLERR "); \
    if (events & POLLHUP) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLHUP "); \
    if (events & POLLNVAL) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLNVAL "); \
    if (events & POLLRDHUP) BT_DBG_INFO(BT_DEBUG_SPP, "   POLLRDHUP"); \
    } while(0)

#define B_EXCEPTION(e)  ((e) & (POLLHUP | POLLRDHUP | POLLERR | POLLNVAL))
#define B_READ(e)       ((e) & POLLIN)
#define B_WRITE(e)      ((e) & POLLOUT)
#define POLL_WAKEUP     1
#define THREAD_EXIT     2

typedef struct
{
    int id;
    int fd;
} poll_cmd_t;

VOID bt_print_spp_struct(VOID)
{
    BT_DBG_MINOR(BT_DEBUG_SPP, "%s.", __func__);

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].bd_addr=%s. ", i, spp_struct[i].bd_addr);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].sock_fd=%d. ", i, spp_struct[i].sock_fd);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].channel_id=%d. ", i, spp_struct[i].channel_id);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].spp_if=%d. ", i, spp_struct[i].spp_if);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].uuid=%s. ", i, spp_struct[i].uuid);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].uuid_len=%d. ", i, spp_struct[i].uuid_len);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].is_used=%d. ", i, spp_struct[i].is_used);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].poll_num=%d. ", i, spp_struct[i].poll_num);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_struct[%d].is_server=%d. ", i, spp_struct[i].is_server);
    }
}

VOID bt_print_spp_listen_struct(VOID)
{
    BT_DBG_MINOR(BT_DEBUG_SPP, "%s.", __func__);

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].listen_fd=%d. ", i, spp_listen_struct[i].listen_fd);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].channel_id=%d. ", i, spp_listen_struct[i].channel_id);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].spp_if=%d. ", i, spp_listen_struct[i].spp_if);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].server_name=%s. ", i, spp_listen_struct[i].server_name);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].uuid=%s. ", i, spp_listen_struct[i].uuid);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].uuid_len=%d. ", i, spp_listen_struct[i].uuid_len);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].is_used=%d. ", i, spp_listen_struct[i].is_used);
        BT_DBG_MINOR(BT_DEBUG_SPP, "spp_listen_struct[%d].revent_num=%d. ", i, spp_listen_struct[i].revent_num);
    }
}

INT32 bt_set_spp_struct(UINT8 index, char *bd_addr, int fd,
    int chnl_id, char *uuid, int uuid_len, BOOL b_server, UINT8 spp_if)
{
    FUNC_ENTRY;
    if (index >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_PARM_INVALID;

    if (bd_addr != NULL)
    {
        strncpy(spp_struct[index].bd_addr, bd_addr, MAX_BDADDR_LEN);
        spp_struct[index].bd_addr[MAX_BDADDR_LEN - 1] = '\0';
    }

    spp_struct[index].sock_fd = fd;
    spp_struct[index].channel_id = chnl_id;

    if (uuid != NULL)
    {
        strncpy(spp_struct[index].uuid, uuid, uuid_len);
        spp_struct[index].uuid_len = uuid_len;
    }

    spp_struct[index].is_server = b_server;
    spp_struct[index].spp_if= spp_if;

    bt_print_spp_struct();

    return BT_SUCCESS;
}

INT32 bt_remove_spp_struct(UINT8 index)
{
    FUNC_ENTRY;
    int ret = 0;
    BT_DBG_INFO(BT_DEBUG_SPP, "index=%d.", index);

    if (index >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_PARM_INVALID;

    if (spp_struct[index].sock_fd != -1)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "Close sock fd:%d.", spp_struct[index].sock_fd);
        ret = shutdown(spp_struct[index].sock_fd, SHUT_RDWR);
        BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP]shutdown fd=%d, ret=%d, errno=%s.",
                spp_struct[index].sock_fd, ret, strerror(errno));
        ret = close(spp_struct[index].sock_fd);
        BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP]close fd=%d, ret=%d, errno=%s.",
                spp_struct[index].sock_fd, ret, strerror(errno));
    }

    memset(&spp_struct[index], 0, sizeof(BT_SPP_STRUCT));
    spp_struct[index].sock_fd = -1;

    return BT_SUCCESS;
}

BOOL bt_get_available_id(UINT8* id, BT_SPP_CONNECT_PARAM *param)
{
    FUNC_ENTRY;
    int i = 0;

    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (spp_struct[i].is_used && param->spp_if == spp_struct[i].spp_if &&
            (0 == strncasecmp(param->uuid, spp_struct[i].uuid, spp_struct[i].uuid_len)))
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "same spp struct already existed");
            *id = (UINT8)i;
            return FALSE;
        }
        if (!spp_struct[i].is_used)
            break;
    }

    BT_DBG_NORMAL(BT_DEBUG_SPP, "Dispatch the id=%d.", i);
    if (i == SPP_MAX_NUMBER)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "No available id, full=%d.", i);
        return FALSE;
    }

    spp_struct[i].is_used = TRUE;
    *id = i;
    return TRUE;
}

BOOL  bt_get_available_id_by_index(UINT8* id, UINT8 spp_i)
{
    FUNC_ENTRY;
    int i = 0;

    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (spp_struct[i].is_used && spp_struct[spp_i].spp_if == spp_struct[i].spp_if &&
            (0 == strncasecmp(spp_struct[spp_i].uuid, spp_struct[i].uuid, spp_struct[i].uuid_len)))
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "same spp struct already existed");
            *id = (UINT8)i;
            return FALSE;
        }
        if (!spp_struct[i].is_used)
            break;
    }

    BT_DBG_NORMAL(BT_DEBUG_SPP, "Dispatch the id=%d.", i);
    if (i == SPP_MAX_NUMBER)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "No available id, full=%d.", i);
        return FALSE;
    }

    spp_struct[i].is_used = TRUE;
    *id = i;
    return TRUE;
}

BOOL bt_send_spp_prepare_fd(struct pollfd* pfd, int fd)
{
    FUNC_ENTRY;
    memset(pfd, 0, sizeof(struct pollfd));
    pfd->fd = fd;
    pfd->events = POLLOUT;
    pfd->revents = 0;
    return TRUE;
}

BOOL bt_spp_prepare_fds(struct pollfd* pfds, int *count)
{
    FUNC_ENTRY;
    int i = 1;
    int j = 0;
    int num = 0;
    BOOL ret = FALSE;
    int spp_i = 0;

    memset(pfds, 0, sizeof(pfds[0])*MAX_PFDS);

    if (data_fdr != -1)
    {
        BT_DBG_MINOR(BT_DEBUG_SPP, "data_fdr=%d.", data_fdr);
        pfds[0].fd = data_fdr;
        pfds[0].events = POLLIN | POLLHUP | POLLRDHUP | POLLERR | POLLNVAL;
        pfds[0].revents = 0;
        num++;
    }
    for (i = 1; i < MAX_PFDS; i++)
    {
        for (j = spp_i; j < SPP_MAX_NUMBER; j++)
        {
            if (spp_struct[j].is_used && spp_struct[j].sock_fd != -1)
            {
                BT_DBG_MINOR(BT_DEBUG_SPP, "pfds[%d], spp_struct[%d].sock_fd=%d.",
                                                     i, j, spp_struct[j].sock_fd);
                pfds[i].fd = spp_struct[j].sock_fd;
                pfds[i].events = POLLIN | POLLHUP | POLLRDHUP | POLLERR | POLLNVAL;
                pfds[i].revents = 0;
                num++;
                spp_i = j + 1;
                ret = TRUE;
                break;
            }
        }

        if (SPP_MAX_NUMBER == j)
            break;
    }
    *count = num;
    BT_DBG_MINOR(BT_DEBUG_SPP, "count=%d.", *count);

    return ret;
}

BOOL bt_get_spp_struct_index_by_fd(int fd, UINT8 *index)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "input fd = %d.", fd);

    if (fd == -1)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "input fd = -1.");
        return FALSE;
    }

    bt_print_spp_struct();

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (spp_struct[i].sock_fd == fd)
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", i);
            *index = (UINT8)i;
            return TRUE;
        }
    }

    BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");

    return FALSE;
}

BOOL bt_get_spp_struct_index_by_mac_uuid_spp_if(CHAR* bd_addr, CHAR* uuid, UINT8 spp_if, UINT8 *index)
{
    FUNC_ENTRY;

    if (bd_addr == NULL || uuid == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "input error parameters.");
        return FALSE;
    }
    BT_DBG_INFO(BT_DEBUG_SPP, "bd_addr=%s, uuid=%s.", bd_addr, uuid);

    bt_print_spp_struct();

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (0 == strncasecmp(bd_addr, spp_struct[i].bd_addr, MAX_BDADDR_LEN) &&
            0 == strncasecmp(uuid, spp_struct[i].uuid, strlen(uuid)) &&
            spp_if == spp_struct[i].spp_if)
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", i);
            *index = (UINT8)i;
            return TRUE;
        }
    }

    BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");

    return FALSE;
}

BOOL bt_get_spp_struct_index_by_uuid_spp_if(CHAR* uuid, UINT8 spp_if, BOOL b_server, int *index)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "uuid=%s, b_server=%d.", uuid, b_server);

    if (uuid == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "input error parameters.");
        return FALSE;
    }

    bt_print_spp_struct();

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if ((0 == strncasecmp(uuid, spp_struct[i].uuid, strlen(uuid))) &&
            spp_struct[i].is_server == b_server &&
            spp_if == spp_struct[i].spp_if)
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", i);
            *index = i;
            return TRUE;
        }
    }

    BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");

    return FALSE;
}

INT32 bt_listen_set_spp_struct(UINT8 index, char *server_name, int fd,
    int chnl_id, char *uuid, int uuid_len, UINT8 spp_if)
{
    FUNC_ENTRY;
    if (index >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_PARM_INVALID;

    if (server_name != NULL)
    {
        strncpy(spp_listen_struct[index].server_name, server_name, MAX_NAME_LEN-1);
        spp_listen_struct[index].server_name[MAX_NAME_LEN-1] = '\0';
    }

    spp_listen_struct[index].listen_fd = fd;
    spp_listen_struct[index].channel_id = chnl_id;

    if (uuid != NULL)
    {
        strncpy(spp_listen_struct[index].uuid, uuid, uuid_len);
        spp_listen_struct[index].uuid_len = uuid_len;
    }
    spp_listen_struct[index].spp_if = spp_if;

    bt_print_spp_listen_struct();

    return BT_SUCCESS;
}

INT32 bt_listen_remove_spp_struct(UINT8 index)
{
    FUNC_ENTRY;
    if (index >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_PARM_INVALID;

    if (spp_listen_struct[index].listen_fd != -1)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "Close sock fd:%d.", spp_listen_struct[index].listen_fd);
        shutdown(spp_listen_struct[index].listen_fd, SHUT_RDWR);
        close(spp_listen_struct[index].listen_fd);
    }

    memset(&spp_listen_struct[index], 0, sizeof(BT_SPP_LISTEN_STRUCT));
    spp_listen_struct[index].listen_fd = -1;

    bt_print_spp_listen_struct();

    return BT_SUCCESS;
}

BOOL bt_listen_get_available_id(UINT8* id, BT_SPP_START_SVR_PARAM *param)
{
    FUNC_ENTRY;
    int i = 0;

    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (spp_listen_struct[i].is_used && param->spp_if == spp_listen_struct[i].spp_if &&
            (0 == strncasecmp(param->uuid, spp_listen_struct[i].uuid, spp_listen_struct[i].uuid_len)))
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "same spp server already activated");
            *id = (UINT8)i;
            return FALSE;
        }
        if (!spp_listen_struct[i].is_used)
            break;
    }

    BT_DBG_NORMAL(BT_DEBUG_SPP, "Dispatch the id=%d.", i);
    if (i == SPP_MAX_NUMBER)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "No available id, full=%d.", i);
        return FALSE;
    }

    spp_listen_struct[i].is_used = TRUE;
    *id = i;
    return TRUE;
}

BOOL bt_listen_spp_prepare_fds(struct pollfd* pfds, int *count)
{
    FUNC_ENTRY;
    int i = 1;
    int j = 0;
    int num = 0;
    BOOL ret = FALSE;
    int spp_i = 0;

    memset(pfds, 0, sizeof(pfds[0])*MAX_PFDS);

    if (listen_fdr != -1)
    {
        BT_DBG_MINOR(BT_DEBUG_SPP, "listen_fdr=%d.", listen_fdr);
        pfds[0].fd = listen_fdr;
        pfds[0].events = POLLIN | POLLHUP | POLLRDHUP | POLLERR | POLLNVAL;
        pfds[0].revents = 0;
        num++;
    }

    for (i = 1; i < MAX_PFDS; i++)
    {
        for (j = spp_i; j < SPP_MAX_NUMBER; j++)
        {
            if (spp_listen_struct[j].is_used && spp_listen_struct[j].listen_fd != -1)
            {
                BT_DBG_MINOR(BT_DEBUG_SPP, "pfds[%d], spp_listen_struct[%d].listen_fd=%d.",
                                                     i, j, spp_listen_struct[j].listen_fd);
                pfds[i].fd = spp_listen_struct[j].listen_fd;
                pfds[i].events = POLLIN | POLLHUP | POLLRDHUP | POLLERR | POLLNVAL;
                pfds[i].revents = 0;
                num++;
                spp_i = j + 1;
                ret = TRUE;
                break;
            }
        }
        if (SPP_MAX_NUMBER == j)
            break;
    }

    *count = num;
    BT_DBG_MINOR(BT_DEBUG_SPP, "count=%d.", *count);

    return ret;
}

BOOL bt_get_spp_listen_struct_index_by_fd(int fd, UINT8 *index)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "input fd = %d.", fd);

    if (fd == -1)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "input fd = -1.");
        return FALSE;
    }

    bt_print_spp_listen_struct();

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if (spp_listen_struct[i].listen_fd == fd)
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", i);
            *index = (UINT8)i;
            return TRUE;
        }
    }

    BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");

    return FALSE;
}

BOOL bt_listen_get_index_by_uuid_spp_if(CHAR* uuid, UINT8 spp_if, int *index)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "uuid=%s.", uuid);

    if (uuid == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "input error parameters.");
        return FALSE;
    }

    bt_print_spp_listen_struct();

    int i = 0;
    for (i = 0; i < SPP_MAX_NUMBER; i++)
    {
        if ((0 == strncasecmp(uuid, spp_listen_struct[i].uuid, spp_listen_struct[i].uuid_len)) &&
            spp_if == spp_listen_struct[i].spp_if)
        {
            BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", i);
            *index = i;
            return TRUE;
        }
    }

    BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");

    return FALSE;
}

/**
 * FUNCTION NAME: linuxbt_spp_uuid_stoh
 * PURPOSE:
 *      The function is used to transfer format from CHAR to bt_uuid_t.
 * INPUT:
 *      uuid_s -- the CHAR uuid.
 *      bt_uuid_t -- the bt_uuid_t uuid.
 * OUTPUT:
 *      None
 * RETURN:
 *      VOID
 * NOTES:
 *      None
 */
//need refactor
#if 0
static VOID linuxbt_spp_uuid_stoh(CHAR *uuid_s,  bt_uuid_t *uuid)
{
    UINT32 i = 0,j = 0;
    UINT32 size = strlen(uuid_s);
    CHAR temp[3];
    temp[2] = '\0';
    while (i < size)
    {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0')
        {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        uuid->uu[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }

    if (size <= 8)   // 16bits uuid or 32bits uuid
    {
        if (size == 4)
        {
            uuid->uu[2] = uuid->uu[0];
            uuid->uu[3] = uuid->uu[1];
            uuid->uu[0] = 0;
            uuid->uu[1] = 0;
        }
        uuid->uu[4] = 0x00;
        uuid->uu[5] = 0x00;
        uuid->uu[6] = 0x10;
        uuid->uu[7] = 0x00;

        uuid->uu[8] = 0x80;
        uuid->uu[9] = 0x00;
        uuid->uu[10] = 0x00;
        uuid->uu[11] = 0x80;

        uuid->uu[12] = 0x5F;
        uuid->uu[13] = 0x9B;
        uuid->uu[14] = 0x34;
        uuid->uu[15] = 0xFB;
    }
}
#endif

int bt_process_cmd(int fdr)
{
    poll_cmd_t cmd = {-1, 0};

    if (recv(fdr, &cmd, sizeof(cmd), MSG_WAITALL) != sizeof(cmd))
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "recv cmd errno:%d", errno);
        return FALSE;
    }

    BT_DBG_NORMAL(BT_DEBUG_SPP, "cmd.id:%d, cmd.fd:%d", cmd.id, cmd.fd);

    switch(cmd.id)
    {
        case POLL_WAKEUP:
            break;
        case THREAD_EXIT:
            return FALSE;
        default:
            BT_DBG_ERROR(BT_DEBUG_SPP, "unknown cmd: %d", cmd.id);
            break;
    }

    return TRUE;
}

VOID bt_spp_data_get_scn(UINT8 index, int size)
{
    int scn = 0;
    int ret = 0;
    UINT8 spp_i = index;

    ret = recv(spp_struct[spp_i].sock_fd, &scn, size, 0);
    if (ret <= 0)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "sock fd:%d recv errno:%d, ret:%d.",
            spp_struct[spp_i].sock_fd, errno, ret);
    }
    spp_struct[spp_i].poll_num++;
    spp_struct[spp_i].channel_id = scn;
    BT_DBG_NORMAL(BT_DEBUG_SPP, "Get channel ID:%d, ret:%d.", spp_struct[spp_i].channel_id, ret);
}

void bt_spp_data_get_connection(UINT8 index, int size)
{
    int ret = 0;
    UINT8 spp_i = index;

    spp_struct[spp_i].poll_num++;
    sock_connect_signal_t connect_signal;;
    ret = recv(spp_struct[spp_i].sock_fd, &connect_signal, size, 0);
    if (ret <= 0)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "sock fd:%d recv errno:%d, ret:%d.",
            spp_struct[spp_i].sock_fd, errno, ret);
    }

    BT_DBG_NOTICE(BT_DEBUG_SPP, "Success. Address=%02X:%02X:%02X:%02X:%02X:%02X.", connect_signal.bd_addr.address[0],
        connect_signal.bd_addr.address[1], connect_signal.bd_addr.address[2], connect_signal.bd_addr.address[3],
        connect_signal.bd_addr.address[4], connect_signal.bd_addr.address[5]);
    BT_DBG_NOTICE(BT_DEBUG_SPP, "Success. size:%d, channel:%d, status:%d, max_tx_packet_size:%d, max_rx_packet_size:%d.",
        connect_signal.size, connect_signal.channel, connect_signal.status,
        connect_signal.max_tx_packet_size, connect_signal.max_rx_packet_size);

    char bd_addr[MAX_BDADDR_LEN];

    strncpy(bd_addr, connect_signal.bd_addr.ToString().c_str(), MAX_BDADDR_LEN - 1);
    bd_addr[MAX_BDADDR_LEN - 1] = '\0';

    BT_DBG_NORMAL(BT_DEBUG_SPP, "Get bd_addr:%s.", bd_addr);

    if (connect_signal.status == 0)
    {
        // SPP Connect Success
        bt_spp_event_cb(spp_struct[spp_i].bd_addr, spp_struct[spp_i].uuid,
            spp_struct[spp_i].uuid_len, spp_struct[spp_i].spp_if, BT_SPP_CONNECT);
    }
    else
    {
        // SPP Connect Fail
        BT_DBG_ERROR(BT_DEBUG_SPP, "received status = %d.", connect_signal.status);
        spp_struct[spp_i].poll_num = 0;
        bt_spp_event_cb(spp_struct[spp_i].bd_addr, spp_struct[spp_i].uuid,
            spp_struct[spp_i].uuid_len, spp_struct[spp_i].spp_if, BT_SPP_CONNECT_FAIL);
        bt_remove_spp_struct(spp_i);
    }
}

VOID bt_spp_data_received(UINT8 index, int size)
{
    int ret = 0;
    UINT8 spp_i = index;
    char buffer[MAX_SPP_DATA_LEN];
    int read_index = 0;
    int cnt = 0;

    read_index = ((size % (MAX_SPP_DATA_LEN - 1) == 0) ? (size / (MAX_SPP_DATA_LEN - 1)) : (size / (MAX_SPP_DATA_LEN - 1) + 1));
    for (cnt = 0; cnt < read_index; cnt++)
    {
        memset(buffer, 0, sizeof(buffer));
        ret = recv(spp_struct[spp_i].sock_fd, &buffer, MAX_SPP_DATA_LEN - 1, 0);
        buffer[MAX_SPP_DATA_LEN - 1] = '\0';
        if (ret <= 0)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "sock fd:%d recv errno:%d, ret:%d.",
                spp_struct[spp_i].sock_fd, errno, ret);
        }
        else
        {
            BT_DBG_INFO(BT_DEBUG_SPP, "Received buffer:%s, ret:%d.", buffer, ret);
            bt_spp_receive_data(spp_struct[spp_i].bd_addr, spp_struct[spp_i].uuid,
                spp_struct[spp_i].uuid_len, spp_struct[spp_i].spp_if, buffer, ret);
        }
    }
}

INT32 bt_process_data(struct pollfd* pfds, int count)
{
    if (count >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_FAIL;

    int i = 1;
    int size = 0;
    UINT8 spp_i = 0;

    for (i = 1; i < MAX_PFDS; i++)
    {
        if (pfds[i].revents)
        {
            size = 0;
            spp_print_events(pfds[i].revents);

            BT_DBG_INFO(BT_DEBUG_SPP, "pfds.fd=%d.", pfds[i].fd);

            if (!bt_get_spp_struct_index_by_fd(pfds[i].fd, &spp_i))
            {
                BT_DBG_ERROR(BT_DEBUG_SPP, "!bt_get_spp_struct_index_by_fd, pfds.fd not equal spp_struct.fd.");
                continue;
            }

            BT_DBG_INFO(BT_DEBUG_SPP, "spp_struct[%d].fd=%d.", spp_i, spp_struct[spp_i].sock_fd);

            BT_DBG_INFO(BT_DEBUG_SPP, "bd_addr=%s, channel_id=%d, uuid=%s, uuid_len=%d, spp_if=%d, is_used=%d.",
                spp_struct[spp_i].bd_addr, spp_struct[spp_i].channel_id, spp_struct[spp_i].uuid,
                spp_struct[spp_i].uuid_len, spp_struct[spp_i].spp_if, spp_struct[spp_i].is_used);

            if (B_READ(pfds[i].revents))
            {
                BT_DBG_INFO(BT_DEBUG_SPP, "SOCK_THREAD_FD_RD");
                if (ioctl(spp_struct[spp_i].sock_fd, FIONREAD, &size) == 0)
                {
                    BT_DBG_INFO(BT_DEBUG_SPP, "size:%d.", size);
                }
                else
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "%s unable to determine bytes remaining to be read on fd %d: %s.", __func__, spp_struct[spp_i].sock_fd, strerror(errno));
                    continue;
                }

                BT_DBG_INFO(BT_DEBUG_SPP, "poll number=%d, size=%d, server=%d.",
                    spp_struct[spp_i].poll_num, size, spp_struct[spp_i].is_server);

                if (spp_struct[spp_i].is_server)
                {
                    bt_spp_data_received(spp_i, size);
                }
                else
                {
                    // 1. first receive the channel id
                    if (spp_struct[spp_i].poll_num == 0 && size == 4)
                    {
                        bt_spp_data_get_scn(spp_i, size);
                    }

                    // 2. second receive the Connection Info
                    else if (spp_struct[spp_i].poll_num == 1 && size == 20)
                    {
                        bt_spp_data_get_connection(spp_i, size);
                    }

                    // 3. third receive the data
                    else if (spp_struct[spp_i].poll_num == 2 && size != 0)
                    {
                        bt_spp_data_received(spp_i, size);
                    }
                    else
                    {
                        BT_DBG_ERROR(BT_DEBUG_SPP, "Error FLow, not expected. poll number=%d, size=%d.", spp_struct[spp_i].poll_num, size);
                    }
                }
            }

            if (B_WRITE(pfds[i].revents))
            {
                BT_DBG_ERROR(BT_DEBUG_SPP, "SOCK_THREAD_FD_WR");
            }

            if (B_EXCEPTION(pfds[i].revents))
            {
                BT_DBG_NORMAL(BT_DEBUG_SPP, "SOCK_THREAD_FD_EXCEPTION");
                bt_spp_event_cb(spp_struct[spp_i].bd_addr, spp_struct[spp_i].uuid, spp_struct[spp_i].uuid_len,
                    spp_struct[spp_i].spp_if, BT_SPP_DISCONNECT);
                bt_remove_spp_struct(spp_i);
            }
        }
    }

    return BT_SUCCESS;
}

VOID bt_listen_get_data_fd(UINT8 index)
{
    FUNC_ENTRY;
    UINT8 spp_i = index;
    int ret = 0;
    int data_fd = -1;
    int i4_ret = 0;

    struct msghdr msg;
    sock_connect_signal_t connect_signal;
    memset(&msg, 0, sizeof(msg));

    struct cmsghdr *cmsg;
    char msgbuf[CMSG_SPACE(1)];

    // Add any pending outbound file descriptors to the message
    // See "man cmsg" really
    msg.msg_control = msgbuf;
    msg.msg_controllen = sizeof msgbuf;

    struct iovec iv;
    memset(&iv, 0, sizeof(iv));

    iv.iov_base = &connect_signal;
    iv.iov_len = sizeof(connect_signal);

    msg.msg_iov = &iv;
    msg.msg_iovlen = 1;

    ret = recvmsg(spp_listen_struct[spp_i].listen_fd, &msg, 0);
    if (ret <= 0)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "listen sock fd:%d recv errno:%d, ret:%d.",
            spp_listen_struct[spp_i].listen_fd, errno, ret);
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if ((cmsg != NULL) && (cmsg->cmsg_len == CMSG_LEN(sizeof(int))))
    {
        if (cmsg->cmsg_level != SOL_SOCKET)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "cmsg_level != SOL_SOCKET, =%d.", cmsg->cmsg_level);
        }

        if (cmsg->cmsg_type != SCM_RIGHTS)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "cmsg_level != SOL_SOCKET, =%d.", cmsg->cmsg_type);
        }

        data_fd = *((int*)CMSG_DATA(cmsg));
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Success. data_fd:%d.", data_fd);
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Success. Address=%02X:%02X:%02X:%02X:%02X:%02X.", connect_signal.bd_addr.address[0],
            connect_signal.bd_addr.address[1], connect_signal.bd_addr.address[2], connect_signal.bd_addr.address[3],
            connect_signal.bd_addr.address[4], connect_signal.bd_addr.address[5]);
        BT_DBG_NOTICE(BT_DEBUG_SPP, "Success. size:%d, channel:%d, status:%d, max_tx_packet_size:%d, max_rx_packet_size:%d.",
            connect_signal.size, connect_signal.channel, connect_signal.status, connect_signal.max_tx_packet_size, connect_signal.max_rx_packet_size);

        char bd_addr[MAX_BDADDR_LEN];

        strncpy(bd_addr, connect_signal.bd_addr.ToString().c_str(), MAX_BDADDR_LEN - 1);
        bd_addr[MAX_BDADDR_LEN - 1] = '\0';

        if (connect_signal.status == 0)
        {
            // SPP Connect Success
            bt_spp_event_cb(bd_addr, spp_listen_struct[spp_i].uuid, spp_listen_struct[spp_i].uuid_len, spp_struct[spp_i].spp_if, BT_SPP_CONNECT);

            UINT8 data_i = 0;
            if (!bt_get_available_id_by_index(&data_i, spp_i))
            {
                if (data_i == SPP_MAX_NUMBER)
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "Can not get available id.");
                }
                else
                    BT_DBG_ERROR(BT_DEBUG_SPP, "exist available id %d.", data_i);
            }

            if (data_fd != -1)
            {
                BT_DBG_NORMAL(BT_DEBUG_SPP, "Add sock fd to spp_struct, index=%d.", data_i);
                bt_set_spp_struct(data_i, bd_addr, data_fd, connect_signal.channel, spp_listen_struct[spp_i].uuid,
                    spp_listen_struct[spp_i].uuid_len, TRUE, spp_listen_struct[spp_i].spp_if);
                poll_cmd_t cmd = {POLL_WAKEUP, data_fd};
                int len = (int)MAX_MSG_SIZE;
                i4_ret = setsockopt(data_fd, SOL_SOCKET, SO_SNDBUF, (char*)&len, (int)sizeof(len));
                if (i4_ret < 0)
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]sock_fd:%d, i4_ret:%d, errno=%s.", data_fd, i4_ret, strerror(errno));
                }
                ret = send(data_fdw, &cmd, sizeof(cmd), 0);
                if (ret < 0)
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]sock_fd:%d, ret:%d, errno=%s.", data_fdw, ret, strerror(errno));
                }
            }
            else
            {
                BT_DBG_ERROR(BT_DEBUG_SPP, "sock_fd = -1.");
            }
        }
        else
        {
            // SPP Connect Fail
            BT_DBG_ERROR(BT_DEBUG_SPP, "received status = %d.", connect_signal.status);
            bt_spp_event_cb(bd_addr, spp_listen_struct[spp_i].uuid, spp_listen_struct[spp_i].uuid_len,
                spp_listen_struct[spp_i].spp_if, BT_SPP_CONNECT_FAIL);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "CMSG_FIRSTHDR Fail.");
    }
}

INT32 bt_listen_process_data(struct pollfd* pfds, int count)
{
    if (count >= SPP_MAX_NUMBER)
        return BT_ERR_STATUS_FAIL;

    int i = 1;
    int ret = 0;
    UINT8 spp_i = 0;

    for (i = 1; i < MAX_PFDS; i++)
    {
        if (pfds[i].revents)
        {
            spp_print_events(pfds[i].revents);

            BT_DBG_NORMAL(BT_DEBUG_SPP, "pfds.fd=%d.", pfds[i].fd);

            if (!bt_get_spp_listen_struct_index_by_fd(pfds[i].fd, &spp_i))
            {
                BT_DBG_ERROR(BT_DEBUG_SPP, "!bt_get_spp_listen_struct_index_by_fd, pfds.fd not equal spp_struct.fd.");
                continue;
            }

            BT_DBG_NOTICE(BT_DEBUG_SPP, "spp_listen_struct[%d].listen_fd=%d.", spp_i, spp_listen_struct[spp_i].listen_fd);
            BT_DBG_NOTICE(BT_DEBUG_SPP, "server_name=%s, channel_id=%d, uuid=%s, uuid_len=%d, is_used=%d, revent_num=%d.",
                spp_listen_struct[spp_i].server_name, spp_listen_struct[spp_i].channel_id, spp_listen_struct[spp_i].uuid,
                spp_listen_struct[spp_i].uuid_len, spp_listen_struct[spp_i].is_used, spp_listen_struct[spp_i].revent_num);

            if (B_READ(pfds[i].revents))
            {
                if (spp_listen_struct[spp_i].revent_num == 0)
                {
                    BT_DBG_NORMAL(BT_DEBUG_SPP, "First received.");
                    spp_listen_struct[spp_i].revent_num++;
                    int size = 0;
                    int scn = 0;
                    if (ioctl(spp_listen_struct[spp_i].listen_fd, FIONREAD, &size) == 0)
                    {
                        BT_DBG_NORMAL(BT_DEBUG_SPP, "size:%d.", size);
                    }
                    else
                    {
                        BT_DBG_ERROR(BT_DEBUG_SPP, "%s unable to determine bytes remaining to be read on fd %d: %s.",
                            __func__, spp_listen_struct[spp_i].listen_fd, strerror(errno));
                    }

                    ret = recv(spp_listen_struct[spp_i].listen_fd, &scn, size, 0);
                    if (ret <= 0)
                    {
                        BT_DBG_ERROR(BT_DEBUG_SPP, "sock fd:%d recv errno:%d, ret:%d.",
                            spp_listen_struct[spp_i].listen_fd, errno, ret);
                    }

                    spp_listen_struct[spp_i].channel_id = scn;
                    BT_DBG_NORMAL(BT_DEBUG_SPP, "Received scn:%d, ret:%d.", scn, ret);
                    continue;
                }

                bt_listen_get_data_fd(spp_i);
            }

            if (B_WRITE(pfds[i].revents))
            {
                BT_DBG_ERROR(BT_DEBUG_SPP, "SOCK_THREAD_FD_WR");
            }

            if (B_EXCEPTION(pfds[i].revents))
            {
                BT_DBG_NORMAL(BT_DEBUG_SPP, "SOCK_THREAD_FD_EXCEPTION");
                bt_listen_remove_spp_struct(spp_i);
            }
        }
    }

    return BT_SUCCESS;
}

static VOID* _bt_spp_data_thread(VOID * args)
{
    FUNC_ENTRY;
    int ret = 0;
    int count = 0;

    prctl(PR_SET_NAME, "bt_spp_data", 0, 0, 0);

    struct pollfd pfds[MAX_PFDS];
    memset(pfds, 0, sizeof(pfds));

    while (1)
    {
        count = 0;
        ret = 0;
        bt_spp_prepare_fds(pfds, &count);

        BT_DBG_NORMAL(BT_DEBUG_SPP, "before data poll fds, count=%d.", count);

        ret = poll(pfds, count, -1);

        BT_DBG_NORMAL(BT_DEBUG_SPP, "after data poll fds. ret=%d.", ret);

        if (ret == -1)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "poll ret -1, exit the thread, errno:%d, err:%s.", errno, strerror(errno));
            break;
        }

        if (ret != 0)
        {
            int need_process_data_fd = TRUE;
            if (pfds[0].revents) //cmd fd always is the first one
            {
                spp_print_events(pfds[0].revents);
                BT_DBG_NORMAL(BT_DEBUG_SPP, "pfds[0].fd=%d.", pfds[0].fd);
                if (pfds[0].fd != data_fdr)
                {
                     BT_DBG_ERROR(BT_DEBUG_SPP, "cmd fd not equal!");
                     break;
                }

                if (!bt_process_cmd(data_fdr))
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "SPP data thread exit...!");
                    break;
                }

                if (ret == 1)
                    need_process_data_fd = FALSE;
                else ret--; //exclude the cmd fd
            }

            if (need_process_data_fd)
                bt_process_data(pfds, ret);
        }
        else
        {
            BT_DBG_MINOR(BT_DEBUG_SPP, "no data, select ret: %d.", ret);
        }
    }

    FUNC_EXIT;
    pthread_exit(NULL);
    return NULL;
}

INT32 bt_spp_data_CreateThread(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = 0;
    pthread_t h_msg_handler_thread;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
       BT_DBG_ERROR(BT_DEBUG_SPP,"pthread_attr_init i4_ret:%d", i4_ret);
       return -1;
    }

    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_msg_handler_thread,
                                          &attr,
                                          _bt_spp_data_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

static VOID* _bt_spp_listen_thread(VOID * args)
{
    FUNC_ENTRY;
    int count = 0;
    int ret = 0;

    prctl(PR_SET_NAME, "bt_spp_listen", 0, 0, 0);
    struct pollfd pfds[MAX_POLL];
    memset(pfds, 0, sizeof(pfds));

    while (1)
    {
        count = 0;
        ret = 0;
        bt_listen_spp_prepare_fds(pfds, &count);

        BT_DBG_NORMAL(BT_DEBUG_SPP, "before listen poll fds, count=%d.", count);

        ret = poll(pfds, count, -1);

        BT_DBG_NORMAL(BT_DEBUG_SPP, "after listen poll fds. ret=%d.", ret);

        if (ret == -1)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "poll ret -1, exit the thread, errno:%d, err:%s.", errno, strerror(errno));
            break;
        }

        if (ret != 0)
        {
            int need_process_data_fd = TRUE;
            if (pfds[0].revents) //cmd fd always is the first one
            {
                spp_print_events(pfds[0].revents);
                BT_DBG_NORMAL(BT_DEBUG_SPP, "pfds[0].fd=%d.", pfds[0].fd);
                if (pfds[0].fd != listen_fdr)
                {
                     BT_DBG_ERROR(BT_DEBUG_SPP, "cmd fd not equal!");
                     break;
                }

                if (!bt_process_cmd(listen_fdr))
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "SPP listen thread exit...!");
                    break;
                }

                if (ret == 1)
                    need_process_data_fd = FALSE;
                else ret--; //exclude the cmd fd
            }

            if (need_process_data_fd)
                bt_listen_process_data(pfds, ret);
        }
        else
        {
            BT_DBG_MINOR(BT_DEBUG_SPP, "no data, select ret: %d.", ret);
        }
    }

    FUNC_EXIT;
    pthread_exit(NULL);
    return NULL;
}

INT32 bt_spp_listen_createThread(VOID)
{
    FUNC_ENTRY;
    INT32 i4_ret = 0;
    pthread_t h_msg_handler_thread;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP,"pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return -1;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&h_msg_handler_thread,
                                          &attr,
                                          _bt_spp_listen_thread,
                                          NULL)))
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

INT32 bt_spp_init_data()
{
    FUNC_ENTRY;
    int socket[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "socketpair failed: %s", strerror(errno));
        return BT_ERR_STATUS_FAIL;
    }

    BT_DBG_MINOR(BT_DEBUG_SPP, "socket[0]=%d, socket[1]=%d.", socket[0], socket[1]);

    data_fdr = socket[0];
    data_fdw = socket[1];
    BT_DBG_NORMAL(BT_DEBUG_SPP, "data_fdr:%d, data_fdw:%d", data_fdr, data_fdw);

    return BT_SUCCESS;
}

INT32 bt_spp_init_listen()
{
    FUNC_ENTRY;
    int socket[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "socketpair failed: %s", strerror(errno));
        return BT_ERR_STATUS_FAIL;
    }

    BT_DBG_MINOR(BT_DEBUG_SPP, "socket[0]=%d, socket[1]=%d.", socket[0], socket[1]);

    listen_fdr = socket[0];
    listen_fdw = socket[1];
    BT_DBG_NORMAL(BT_DEBUG_SPP, "listen_fdr:%d, listen_fdw:%d", listen_fdr, listen_fdw);

    return BT_SUCCESS;
}

INT32 bt_spp_init()
{
    FUNC_ENTRY;
    int i = 0;
    memset(spp_struct, 0, SPP_MAX_NUMBER*sizeof(BT_SPP_STRUCT));

    for(i = 0; i < SPP_MAX_NUMBER; i++)
    {
        spp_struct[i].sock_fd = -1;
    }

    memset(spp_listen_struct, 0, SPP_MAX_NUMBER*sizeof(BT_SPP_LISTEN_STRUCT));

    for(i = 0; i < SPP_MAX_NUMBER; i++)
    {
        spp_listen_struct[i].listen_fd = -1;
    }

    bt_spp_init_data();
    bt_spp_init_listen();

    return BT_SUCCESS;
}

/**
 * FUNCTION NAME: linuxbt_spp_init
 * PURPOSE:
 *      The function is used to init SPP interface.
 * INPUT:
 *      VOID
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_init(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP] %s()", __FUNCTION__);

    // Get sock interface
    g_bt_sock_interface = (btsock_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_SOCKETS_ID);
    if (NULL == g_bt_sock_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP] Failed to get SPP interface");
        return BT_ERR_STATUS_FAIL;
    }

    bt_spp_init();

    bt_spp_data_CreateThread();
    bt_spp_listen_createThread();

    return BT_SUCCESS;
}

/**
 * FUNCTION NAME: linuxbt_spp_deinit
 * PURPOSE:
 *      The function is used to deinit SPP interface.
 * INPUT:
 *      VOID
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_deinit(VOID)
{
    FUNC_ENTRY;

    if (data_fdr >= 0)
    {
        shutdown(data_fdr, SHUT_RDWR);
        close(data_fdr);
        data_fdr = -1;
    }

    if (data_fdw >= 0)
    {
        shutdown(data_fdw, SHUT_RDWR);
        close(data_fdw);
        data_fdw = -1;
    }

    if (listen_fdr >= 0)
    {
        shutdown(listen_fdr, SHUT_RDWR);
        close(listen_fdr);
        listen_fdr = -1;
    }

    if (listen_fdw >= 0)
    {
        shutdown(listen_fdw, SHUT_RDWR);
        close(listen_fdw);
        listen_fdw = -1;
    }

    g_bt_sock_interface = NULL;
    return BT_SUCCESS;
}

/**
 * FUNCTION NAME: linuxbt_spp_activate
 * PURPOSE:
 *      The function is used to active SPP with uuid.
 * INPUT:
 *      ptr         -- the device address that to active.
 *      puuid_128   -- the uuid.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_activate(BT_SPP_START_SVR_PARAM *param)
{
    BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP] %s()", __FUNCTION__);

    if (NULL == g_bt_sock_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Failed to get SPP interface.");
        return BT_ERR_STATUS_UNSUPPORTED;
    }

    bool is_valid = true;
    Uuid app_uuid;
    std::string uuid_str;
    if (strlen(param->uuid))
    {
        uuid_str = param->uuid;
        app_uuid = Uuid::FromString(uuid_str, &is_valid);
        if (false == is_valid)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "invalid app uuid %s", param->uuid);
            return BT_ERR_STATUS_PARM_INVALID;
        }
    }
    else
    {
        app_uuid = Uuid::FromString(SPP_DEFAULT_UUID);
    }

    BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP] start server uuid %s", app_uuid.ToString().c_str());
    int ret = BT_SUCCESS;
    int temp_sock_fd = -1;
    UINT8 index = 0;

    if (0 == strlen(param->server_name))
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Null Server Name.");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (!bt_listen_get_available_id(&index, param))
    {
        if(index == SPP_MAX_NUMBER)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "Can not get available id.");
            return BT_ERR_STATUS_NOMEM;
        }
        else
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "Same spp server exist.");
            return BT_SUCCESS;
        }
    }

    btsock_type_t type = BTSOCK_RFCOMM;
    int channel = 0;
    int flags = BTSOCK_FLAG_ENCRYPT;
    int callingUid = param->spp_if;

    ret = g_bt_sock_interface->listen(type, param->server_name, &app_uuid, channel, &temp_sock_fd, flags, callingUid);

    BT_DBG_NORMAL(BT_DEBUG_SPP,  "listen sock_fd:%d, ret: %d.", temp_sock_fd, ret);

    if (ret == BT_STATUS_SUCCESS && temp_sock_fd != -1)
    {
        bt_listen_set_spp_struct(index, param->server_name, temp_sock_fd, channel, param->uuid, strlen(param->uuid), param->spp_if);
        poll_cmd_t cmd = {POLL_WAKEUP, temp_sock_fd};
        if (send(listen_fdw, &cmd, sizeof(cmd), 0) < 0)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]send errno=%s @%d.", strerror(errno), __LINE__);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "SPP start server Fail");
        if (temp_sock_fd != -1)
        {
            shutdown(temp_sock_fd, SHUT_RDWR);
            close(temp_sock_fd);
            temp_sock_fd = -1;
        }
        bt_listen_remove_spp_struct(index);

        ret = BT_ERR_STATUS_FAIL;
    }

    return ret;
}

/**
 * FUNCTION NAME: linuxbt_spp_deactivate
 * PURPOSE:
 *      The function is used to deactive SPP with uuid.
 * INPUT:
 *      puuid_128   -- the uuid that to deactive.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_deactivate(BT_SPP_STOP_SVR_PARAM *param)
{
    BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP] %s()", __FUNCTION__);

    int ret = BT_SUCCESS;
    int index = 0;
    int listen_index = 0;
    BOOL b_ret = FALSE;

    if (0 == strlen(param->uuid))
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "NULL UUID.");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    b_ret = bt_get_spp_struct_index_by_uuid_spp_if(param->uuid, param->spp_if, TRUE, &index);

    if (b_ret)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "find index=%d.", index);
        if (spp_struct[index].sock_fd != -1)
        {
            ret = shutdown(spp_struct[index].sock_fd, SHUT_RDWR);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));

            ret = close(spp_struct[index].sock_fd);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Can not find the record!");
    }

    b_ret = bt_listen_get_index_by_uuid_spp_if(param->uuid, param->spp_if, &listen_index);

    if (b_ret)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "find listen index=%d.", listen_index);
        if (spp_listen_struct[listen_index].listen_fd != -1)
        {
            ret = shutdown(spp_listen_struct[listen_index].listen_fd, SHUT_RDWR);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));

            ret = close(spp_listen_struct[listen_index].listen_fd);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Not find the record.");
        ret = BT_ERR_STATUS_FAIL;
    }

    return ret;
}

/**
 * FUNCTION NAME: linuxbt_spp_connect
 * PURPOSE:
 *      The function is used to SPP connect with uuid.
 * INPUT:
 *      bt_addr -- the device address that to connect.
 *      uuid    -- using this uuid to connect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_connect(BT_SPP_CONNECT_PARAM *param)
{
    BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP] %s()", __FUNCTION__);

    if (NULL == g_bt_sock_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Failed to get SPP interface.");
        return BT_ERR_STATUS_UNSUPPORTED;
    }

    bool is_valid = true;
    Uuid app_uuid;
    std::string uuid_str;
    RawAddress bdaddr;
    memset(&bdaddr, 0, sizeof(RawAddress));
    int ret = BT_SUCCESS;
    UINT8 index = 0;
    int i4_ret = 0;

    if (0 == strlen(param->bd_addr))
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "Null bt address.");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (strlen(param->uuid))
    {
        uuid_str = param->uuid;
        app_uuid = Uuid::FromString(uuid_str, &is_valid);
        if (false == is_valid)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "invalid app uuid %s", param->uuid);
            return BT_ERR_STATUS_PARM_INVALID;
        }
    }
    else
    {
        app_uuid = Uuid::FromString(SPP_DEFAULT_UUID);
    }

    if (!bt_get_available_id(&index, param))
    {
        if (index == SPP_MAX_NUMBER)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "Can not get available id.");
            return BT_ERR_STATUS_NOMEM;
        }
        else
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "Same spp uuid connected.");
            return BT_STATUS_SUCCESS;
        }
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(param->bd_addr, bdaddr);
    BT_DBG_INFO(BT_DEBUG_SPP,  "SPP connect to %s", param->bd_addr);
    BT_DBG_INFO(BT_DEBUG_SPP,  "SPP connect to uuid %s", app_uuid.ToString().c_str());

    btsock_type_t type = BTSOCK_RFCOMM;
    int channel = 0;
    int flags = BTSOCK_FLAG_ENCRYPT;
    int callingUid = 0;
    int temp_sock_fd = -1;

    ret = g_bt_sock_interface->connect(&bdaddr, type, &app_uuid, channel, &temp_sock_fd, flags, callingUid);

    BT_DBG_NORMAL(BT_DEBUG_SPP,  "connect ret: %d, sock_fd:%d.", ret, temp_sock_fd);

    if (ret == BT_STATUS_SUCCESS && temp_sock_fd != -1)
    {
        bt_set_spp_struct(index, param->bd_addr, temp_sock_fd, channel, param->uuid, strlen(param->uuid), FALSE, param->spp_if);
        poll_cmd_t cmd = {POLL_WAKEUP, temp_sock_fd};
        int len = (int)MAX_MSG_SIZE;
        i4_ret = setsockopt(temp_sock_fd , SOL_SOCKET, SO_SNDBUF, (char*)&len, (int)sizeof(len));
        if (i4_ret < 0)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]sock_fd:%d, i4_ret:%d, errno=%s.", temp_sock_fd, i4_ret, strerror(errno));
        }
        if (send(data_fdw, &cmd, sizeof(cmd), 0) < 0)
        {
            BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]send errno=%s @%d.", strerror(errno), __LINE__);
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "SPP Connect Fail");

        if (temp_sock_fd != -1)
        {
            shutdown(temp_sock_fd, SHUT_RDWR);
            close(temp_sock_fd);
            temp_sock_fd = -1;
        }

        bt_remove_spp_struct(index);
        ret = BT_ERR_STATUS_FAIL;
    }

    return ret;
}

/**
 * FUNCTION NAME: linuxbt_spp_disconnect
 * PURPOSE:
 *      The function is used to SPP disconnect with port id.
 * INPUT:
 *      addr -- the device address that to disconnect.
 *      uuid    -- using this uuid to disconnect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_disconnect(BT_SPP_DISCONNECT_PARAM *param)
{
    FUNC_ENTRY;
    BT_DBG_INFO(BT_DEBUG_SPP, "[SPP]bd_addr=%s, uuid=%s.",
        param->bd_addr, param->uuid);

    int ret = BT_SUCCESS;
    UINT8 index = 0;
    BOOL b_ret = FALSE;

    b_ret = bt_get_spp_struct_index_by_mac_uuid_spp_if(param->bd_addr, param->uuid, param->spp_if, &index);

    if (b_ret)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP]index=%d, sock_fd:%d.", index, spp_struct[index].sock_fd);
        if (spp_struct[index].sock_fd != -1)
        {
            ret = shutdown(spp_struct[index].sock_fd, SHUT_RDWR);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));

            ret = close(spp_struct[index].sock_fd);
            if (ret < 0)
                 BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]ret=%d, errno=%s.", ret, strerror(errno));
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP] Can not find the record!");
        ret = BT_ERR_STATUS_FAIL;
    }

    return ret;
}

/**
 * FUNCTION NAME: linuxbt_spp_send_data
 * PURPOSE:
 *      The function is used to send data.
 * INPUT:
 *      bd_addr -- the device to send.
 *      uuid -- the uuid to send.
 *      pdata -- the string to send.
 *      length -- the string length to send.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
INT32 linuxbt_spp_send_data(BT_SPP_SEND_DATA_PARAM *param)
{
    FUNC_ENTRY;
    BT_DBG_INFO(BT_DEBUG_SPP, "[SPP]bd_addr=%s, uuid=%s, pdata=%s, length=%d, spp_if=%d.",
        param->bd_addr, param->uuid, param->spp_data, param->spp_data_len, param->spp_if);
    struct pollfd pfd;

    int send_fd = 0;
    int ret = BT_SUCCESS;
    int fd_event_ret = 0;
    UINT8 index = 0;
    int data_left_len = param->spp_data_len;
    BOOL b_ret = FALSE;

    b_ret = bt_get_spp_struct_index_by_mac_uuid_spp_if(param->bd_addr, param->uuid, param->spp_if, &index);

    if (b_ret)
    {
        BT_DBG_NORMAL(BT_DEBUG_SPP, "[SPP]index=%d, sock_fd:%d.", index, spp_struct[index].sock_fd);
        send_fd = spp_struct[index].sock_fd;

        if (send_fd != -1)
        {
            bt_send_spp_prepare_fd(&pfd, send_fd);
            while (data_left_len)
            {
                BT_DBG_NORMAL(BT_DEBUG_SPP, "before data poll fds");
                fd_event_ret = poll((struct pollfd*)&pfd, 1, -1); //send_fd event in coming
                BT_DBG_NORMAL(BT_DEBUG_SPP, "after data poll fds");
                if (fd_event_ret == -1)
                {
                    BT_DBG_ERROR(BT_DEBUG_SPP, "poll ret -1, exit send, errno:%d, err:%s.", errno, strerror(errno));
                    ret = BT_ERR_STATUS_FAIL;
                    break;
                }
                if (fd_event_ret != 0 && B_WRITE(pfd.revents))
                {
                    // send() shall return the number of bytes sent. Otherwise, -1 shall bt returned and errno set to indicate the error.
                    ret = send(send_fd, param->spp_data, data_left_len, MSG_DONTWAIT);

                    if (ret < 0)
                    {
                        BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]sock_fd:%d, ret:%d, errno=%s.", send_fd, ret, strerror(errno));
                    }
                    else
                    {
                        data_left_len -= ret;
                    }
                }
            }
        }
        else
        {
            ret = BT_ERR_STATUS_FAIL;
            BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP]Found the fd = -1!");
        }
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_SPP, "[SPP] Can not find the record!");
        ret = BT_ERR_STATUS_PARM_INVALID;
    }

    BT_DBG_MINOR(BT_DEBUG_SPP, "[SPP]ret:%d.", ret);

    return ret;
}

INT32 linuxbt_spp_get_connection_info(BT_SPP_CONNECTION_INFO_DB *spp_connection_info_db)
{
    FUNC_ENTRY;
    for (int i = 0; i < SPP_MAX_NUMBER; i++)
    {
        memcpy(spp_connection_info_db->spp_connection_info[i].bd_addr, spp_struct[i].bd_addr,
            MAX_BDADDR_LEN);
        spp_connection_info_db->spp_connection_info[i].bd_addr[MAX_BDADDR_LEN - 1] = '\0';
        memcpy(spp_connection_info_db->spp_connection_info[i].uuid, spp_struct[i].uuid,
            MAX_UUID_LEN);
        spp_connection_info_db->spp_connection_info[i].uuid[MAX_UUID_LEN - 1] = '\0';
        spp_connection_info_db->spp_connection_info[i].spp_if = spp_struct[i].spp_if;
        spp_connection_info_db->spp_connection_info[i].is_server = spp_struct[i].is_server;
        spp_connection_info_db->spp_connection_info[i].is_used = spp_struct[i].is_used;
        BT_DBG_INFO(BT_DEBUG_SPP, "spp_struct[%d].bd_addr=%s. ", i, spp_connection_info_db->spp_connection_info[i].bd_addr);
        BT_DBG_INFO(BT_DEBUG_SPP, "spp_struct[%d].uuid=%s. ", i, spp_connection_info_db->spp_connection_info[i].uuid);
        BT_DBG_INFO(BT_DEBUG_SPP, "spp_struct[%d].spp_if=%d. ", i, spp_connection_info_db->spp_connection_info[i].spp_if);
        BT_DBG_INFO(BT_DEBUG_SPP, "spp_struct[%d].is_server=%d. ", i, spp_connection_info_db->spp_connection_info[i].is_server);
    }
    BT_DBG_WARNING(BT_DEBUG_SPP, "[SPP]spp_get_connection_info done\n");
    return BT_SUCCESS;
}

