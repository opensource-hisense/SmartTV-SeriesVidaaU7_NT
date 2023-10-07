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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include <sched.h>

#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#include "rw_init_mtk_bt_service.h"
#include "mtk_bt_service_gap_wrapper.h"

#define VERSION "1.0"

#define BT_MANAGER_LOG(_stmt...) \
        do{ \
            if (1) {    \
                printf("[btmanager]Func:%s Line:%d --->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

/**************************************************************************
 *                 G L O B A L   V A R I A B L E S                        *
***************************************************************************/

/**************************************************************************
 *                         F U N C T I O N S                              *
***************************************************************************/

enum
{
    NONE,
    WAIT,
    STACK_EXIT,
    STACK_START,
    STACK_STOP,
    BT_ON,
    BT_OFF,
    END,
};

static pthread_mutex_t bt_manager_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bt_manager_signal = PTHREAD_COND_INITIALIZER;

static char bt_manager_pv_tag[2] = {0};
static int bt_manager_action;

#define BT_MANAGER_DBG_FIFO "/tmp/bt_manager_cmd_fifo"
static pthread_t bt_manager_dbg_thread;

static MTKRPCAPI_BT_APP_CB_FUNC g_gap_func;
static pid_t stack_pid = -1;

#undef PATH_MAX
#define PATH_MAX 128
#ifndef BT_STACK_PATH
#define BT_STACK_PATH
#endif

#define LOG_SHRMEM 1
#if LOG_SHRMEM
typedef struct
{
    unsigned int key;
    unsigned int pos;
    unsigned int full;
} BT_LOG_SHR_MEM_HEADER;
#define LOG_SHR_MEM_KEY BT_CONF_PATH
#define LOG_SHR_MEM_TOTAL_SIZE (1024*20)
#define LOG_SHR_MEM_LOG_SIZE (LOG_SHR_MEM_TOTAL_SIZE - sizeof(BT_LOG_SHR_MEM_HEADER))
static int bt_get_shrmem(void **addr)
{
    key_t key;
    int id = 0;

    key = ftok(LOG_SHR_MEM_KEY, 0xCC);
    if (key == -1)
    {
        BT_MANAGER_LOG("ftok(%s) failed! error = %s", LOG_SHR_MEM_KEY, strerror(errno));
        return -1;
    }

    id = shmget(key, LOG_SHR_MEM_TOTAL_SIZE, IPC_CREAT | 0666);
    if (id == -1)
    {
        BT_MANAGER_LOG("shmget failed! error = %s", strerror(errno));
        return -1;
    }

    *addr = shmat(id, NULL, 0);
    if (*addr == (void *)-1)
    {
        BT_MANAGER_LOG("shmat failed! error = %s", strerror(errno));
        return -1;
    }

    BT_MANAGER_LOG("shmat addr = %p", *addr);
    return 0;
}

static void bt_get_stack_last_log(void *log_shrmem, char *log_path)
{
    int fd = -1;
    char file[PATH_MAX] = {0};
    BT_LOG_SHR_MEM_HEADER *hdr = log_shrmem;
    char *log_pos = NULL;
    char backup_path[PATH_MAX] = {0};

    char ts_buffer[24];
    char buffer[24] = {0};
    struct timeval tv;
    struct tm *ltime;

    if (hdr == NULL || hdr->key != 0xCCCCCCCC)
    {
        BT_MANAGER_LOG("log_shrmem is invalid");
        return;
    }

    BT_MANAGER_LOG("key = %x", hdr->key);
    BT_MANAGER_LOG("pos = %d", hdr->pos);
    BT_MANAGER_LOG("full = %d", hdr->full);

    snprintf(backup_path, sizeof(backup_path), "%s/../backup/", log_path);
    if (mkdir(backup_path, 0774) != 0 && errno != EEXIST)
    {
        BT_MANAGER_LOG("create folder %s failed. error = %s", backup_path, strerror(errno));
        return;
    }

    gettimeofday(&tv, NULL);
    ltime = localtime(&tv.tv_sec);
    memset(ts_buffer, 0, sizeof(ts_buffer));
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", ltime);
    snprintf(ts_buffer, sizeof(ts_buffer), "%s%03ld", buffer, tv.tv_usec / 1000);

    snprintf(file, PATH_MAX, "%sbt_stack.log.%s.last", backup_path, ts_buffer);
    file[PATH_MAX - 1] = '\0';
    fd = open(file, O_RDWR | O_CREAT, 0777);
    if (fd < 0)
    {
        BT_MANAGER_LOG("open file(%s) failed. error = %s",
                file, strerror(errno));
        return;
    }

    log_pos = log_shrmem + sizeof(BT_LOG_SHR_MEM_HEADER);
    if (hdr->full == 0)
    {
        if (hdr->pos != 0)
            write(fd, log_pos, hdr->pos);
    }
    else
    {
        write(fd, log_pos + hdr->pos, (LOG_SHR_MEM_LOG_SIZE - hdr->pos));
        if (hdr->pos != 0)
            write(fd, log_pos, hdr->pos);
    }
    close(fd);
    hdr->key = 0;
}
#endif

#define BTSNOOP_LOG_PATH_KEY "BtSnoopFileName"
static void bt_manager_get_log_path(char *log_path, unsigned int len)
{
    FILE* fp = NULL;
    char line[1024];
    char *ptr = NULL;

    memset(log_path, 0, len);

    fp = fopen(BT_CONF_PATH"/bt_stack.conf", "rt");
    if (!fp)
    {
        BT_MANAGER_LOG("open stack conf failed(%s)", BT_CONF_PATH"/bt_stack.conf");
        memcpy(log_path, "/data/misc/bluetooth/logs", strlen("/data/misc/bluetooth/logs"));
        return;
    }

    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), fp))
    {
        ptr = strstr(line, BTSNOOP_LOG_PATH_KEY);
        if (ptr)
        {
            ptr = strstr(ptr + strlen(BTSNOOP_LOG_PATH_KEY), "=");
            if (ptr)
            {
                ptr++;
                while (*ptr != 0)
                {
                    if (!isspace(*ptr))
                        *log_path = *ptr;
                    log_path++;
                    ptr++;
                }
            }
        }
        memset(line, 0, sizeof(line));
    }
}

static void bt_log_backup(char *path)
{
#define PATH_NUM 2
    char log_file[PATH_MAX] = {0};
    char backup_file[PATH_MAX] = {0};
    char log_path[PATH_NUM][PATH_MAX] = {0};
    char backup_path[PATH_NUM][PATH_MAX] = {0};
    struct dirent *p_file;
    DIR *p_dir = NULL;
    unsigned int idx = 0;

    snprintf(log_path[0], sizeof(log_path[0]), "%s", path);
    snprintf(log_path[1], sizeof(log_path[1]), "%s/bthci", log_path[0]);
    snprintf(backup_path[0], sizeof(backup_path[0]), "%s/../backup/", path);
    snprintf(backup_path[1], sizeof(backup_path[1]), "%s/bthci", backup_path[0]);

    for (idx = 0; idx < PATH_NUM; idx++)
    {
        p_dir = opendir(log_path[idx]);
        if (p_dir != NULL)
        {
            if (mkdir(backup_path[idx], 0774) != 0 && errno != EEXIST)
            {
                BT_MANAGER_LOG("create folder %s failed! error = %s", backup_path[idx], strerror(errno));
                return;
            }

            while ((p_file = readdir(p_dir)) != NULL)
            {
                if (strncmp(p_file->d_name, "..", 2) == 0
                    || strncmp(p_file->d_name, ".", 1) == 0)
                {
                    continue;
                }

                memset(log_file, 0, sizeof(log_file));
                memset(backup_file, 0, sizeof(backup_file));
                snprintf(log_file, sizeof(log_file), "%s/%s", log_path[idx], p_file->d_name);
                snprintf(backup_file, sizeof(backup_file), "%s/%s", backup_path[idx], p_file->d_name);
                if (rename(log_file, backup_file))
                {
                    BT_MANAGER_LOG("unable to rename '%s' to '%s'. err = %s",
                        log_file, backup_file, strerror(errno));
                }
            }
            closedir(p_dir);
        }
        else
        {
            BT_MANAGER_LOG("open %s failed. err = %s",
                            log_path[idx], strerror(errno));
        }
    }
}

static pid_t bt_start_app(char *app, char *param)
{
    pid_t pid = fork();
    char *argc[2] = {0, 0};

    switch (pid)
    {
    case -1:
        BT_MANAGER_LOG("fork failed");
        exit(1);
        break;
    case 0:
        /* child process */
        BT_MANAGER_LOG(" start %s(%p) in child process", app, param);
        if (param)
            argc[0] = param;
        execvp(app, argc);
        break;
    default:
        BT_MANAGER_LOG(" start %s OK. return child process pid(%d)", app, pid);
        break;
    }
    return pid;
}

static int bt_stop_app(pid_t pid)
{
    if (pid != -1)
        return kill(pid, SIGKILL);
    return -1;
}

static void bt_manager_gap_event_callback(BTMW_GAP_EVT *bt_event, void* pv_tag)
{
    switch (bt_event->state)
    {
    case GAP_STATE_ON:
        BT_MANAGER_LOG("BT is ON");
        break;
    case GAP_STATE_OFF:
        BT_MANAGER_LOG("BT is OFF");
        break;
    default:
        /* BT_MANAGER_LOG("undefined bt_gap_event "); */
        break;
    }
}

static void bt_manager_gap_get_pairing_key_callback(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{
    BT_MANAGER_LOG(" ");
}

static void bt_manager_gap_app_inquiry_callback(BTMW_GAP_DEVICE_INFO *pt_result, void* pv_tag)
{
    BT_MANAGER_LOG(" ");
}

static VOID* bt_manager_cmd_recv_thread(VOID * args)
{
    INT32 ret = 0;
    INT32 i4_ret = 0;
    INT32 fifoFd = -1;
    char cmd_str[257] = {0};
    char fifo_name[PATH_MAX] = BT_MANAGER_DBG_FIFO;

    if (NULL != args)
    {
        strncpy(fifo_name, (char*)args, PATH_MAX - 1);
        fifo_name[PATH_MAX - 1] = '\0';
    }

    if (remove(fifo_name)) {
        BT_MANAGER_LOG( "%s can't remove", fifo_name);
    } else {
        BT_MANAGER_LOG("%s removed", fifo_name);
    }

    i4_ret = mkfifo(fifo_name, 0777);
    if (i4_ret < 0) {
        BT_MANAGER_LOG("mkfifo %s fail:%d", fifo_name, i4_ret);
        return NULL;
    }
    BT_MANAGER_LOG("mkfifo success");

    prctl(PR_SET_NAME, "bt_manager_msg_recv_thread", 0, 0, 0);
    BT_MANAGER_LOG("Enter %s fifo_name: %s", __FUNCTION__, fifo_name);

    while (1)
    {
        memset(cmd_str, 0, sizeof(cmd_str));
        if (-1 == fifoFd)
        {
            fifoFd = open(fifo_name, O_RDONLY);
            if (fifoFd < 0)
            {
                BT_MANAGER_LOG("%s fifo open fail:%s", fifo_name, strerror(errno));
                return NULL;
            }
        }

        ret = read(fifoFd, cmd_str, 256);
        cmd_str[256] = '\0';
        if (ret <= 0) {
            close(fifoFd);
            fifoFd = -1;
            continue;
        }

        if (cmd_str[ret - 1] <= 0x20) cmd_str[ret - 1] = '\0';
        else cmd_str[ret] = '\0';

        BT_MANAGER_LOG("cmd_str=%s", cmd_str);
#if 0
        if (strncmp(cmd_str, "BT START", strlen("BT START")) == 0)
        {
            bt_manager_action = STACK_START;
            pthread_mutex_lock(&bt_manager_lock);
            pthread_cond_signal(&bt_manager_signal);
            pthread_mutex_unlock(&bt_manager_lock);
        }
        if (strncmp(cmd_str, "BT STOP", strlen("BT STOP")) == 0)
        {
            bt_manager_action = STACK_STOP;
            pthread_mutex_lock(&bt_manager_lock);
            pthread_cond_signal(&bt_manager_signal);
            pthread_mutex_unlock(&bt_manager_lock);
        }
#endif
        if (strncmp(cmd_str, "BT ON", strlen("BT ON")) == 0)
        {
            bt_manager_action = BT_ON;
            pthread_mutex_lock(&bt_manager_lock);
            pthread_cond_signal(&bt_manager_signal);
            pthread_mutex_unlock(&bt_manager_lock);
        }
        if (strncmp(cmd_str, "BT OFF", strlen("BT OFF")) == 0)
        {
            bt_manager_action = BT_OFF;
            pthread_mutex_lock(&bt_manager_lock);
            pthread_cond_signal(&bt_manager_signal);
            pthread_mutex_unlock(&bt_manager_lock);
        }
    }

    return NULL;
}

static int bt_manager_loop_run_init(void)
{
    INT32 i4_ret = 0;
    BT_MANAGER_LOG("Enter %s", __FUNCTION__);

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 1;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_MANAGER_LOG( "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }

    i4_ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (0 != i4_ret)
    {
        BT_MANAGER_LOG("pthread_attr_setschedpolicy i4_ret:%d", i4_ret);
    }

    i4_ret = pthread_attr_setschedparam(&attr, &param);
    if (0 != i4_ret)
    {
        BT_MANAGER_LOG("pthread_attr_setschedparam i4_ret:%d", i4_ret);
    }

    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret)
    {
        if (0 != (i4_ret = pthread_create(&bt_manager_dbg_thread,
                                          &attr,
                                          bt_manager_cmd_recv_thread,
                                          NULL)))
        {
            BT_MANAGER_LOG("pthread_create i4_ret:%ld", (long)i4_ret);
        }
    }
    else
    {
        BT_MANAGER_LOG("pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    pthread_attr_destroy(&attr);

    return 0;
}

static void bt_manager_stack_exit_cb(void *pv_tag)
{
    BT_MANAGER_LOG("pv_tag=%p, action=%d", pv_tag, bt_manager_action);
    if (bt_manager_action == WAIT)
    {
        bt_manager_action = STACK_EXIT;
        pthread_mutex_lock(&bt_manager_lock);
        pthread_cond_signal(&bt_manager_signal);
        pthread_mutex_unlock(&bt_manager_lock);
    }
}

static void print_help(void)
{
    printf("btmanager - ver %s\n", VERSION);
    printf("Usage:\n"
        "\t btmanager -p <PATH> -r on|off \n");
    printf("\t\t -p: set path of btservice\n");
    printf("\t\t -r: btservice will auto restart or not after exit.\n");
    printf("\t\t\t Default is ON if not set\n");
}

static void bt_manager_signal_handle(int sig)
{
    BT_MANAGER_LOG(" Recv sig(%d)", sig);
    (void)bt_stop_app(stack_pid);
    exit(0);
}


int main(int argc, char *argv[])
{
    int ret = 0;
    int opt;
    char *path = NULL;
    char run_stack[PATH_MAX] = {0};
    char log_path[PATH_MAX] = {0};
    stack_exit_cb func = bt_manager_stack_exit_cb;
    pthread_condattr_t condattr;
    static int stack_flag = 0;
    int auto_stack = 0;

    bt_manager_get_log_path(log_path, sizeof(log_path));

#if LOG_SHRMEM
    char *log_shrmem = NULL;
    BT_LOG_SHR_MEM_HEADER *hdr_shrmem = NULL;
    ret = bt_get_shrmem((void **)&log_shrmem);
    if (ret < 0)
    {
        BT_MANAGER_LOG("create share memory failed");
    }
    else
    {
        hdr_shrmem = (BT_LOG_SHR_MEM_HEADER *)log_shrmem;
        hdr_shrmem->key = 0;
        BT_MANAGER_LOG("log_shrmem = %p", log_shrmem);
    }
#endif
    while ((opt = getopt(argc, argv, "p:r:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            path = optarg;
            BT_MANAGER_LOG("Config stack path: %s", path);
            break;
        case 'r':
            if (strncmp(optarg, "on", strlen("on")) == 0
                    || strncmp(optarg, "ON", strlen("ON")) == 0)
            {
                auto_stack = 1;
            }
            else if (strncmp(optarg, "off", strlen("off")) == 0
                    || strncmp(optarg, "OFF", strlen("OFF")) == 0)
            {
                auto_stack = 0;
            }
            else
            {
                print_help();
                return 0;
            }
            break;
        default:
            print_help();
            return 0;
        }
    }

    (void)signal(SIGTERM, bt_manager_signal_handle);

    BT_MANAGER_LOG("path = %s, auto_stack = %d", path, auto_stack);

    if (path == NULL)
        snprintf(run_stack, PATH_MAX - 1, BT_STACK_PATH"btservice");
    else
        snprintf(run_stack, PATH_MAX - 1, "%s/btservice", path);

    run_stack[PATH_MAX - 1] = '\0';
    BT_MANAGER_LOG("Run stack: %s", run_stack);

    ret = pthread_condattr_init(&condattr);
    if (ret < 0)
    {
        BT_MANAGER_LOG("init cond fail:%d", ret);
    }
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    ret = pthread_cond_init(&bt_manager_signal, &condattr);

    bt_manager_loop_run_init();

    while (1)
    {
        BT_MANAGER_LOG("BT stack startup and backup log");
#if LOG_SHRMEM
        bt_get_stack_last_log(log_shrmem, log_path);
#endif
        bt_log_backup(log_path);
        if (stack_flag != 0 && auto_stack == 0)
        {
            BT_MANAGER_LOG("stack = %s, auto_stack = %d", run_stack, auto_stack);
            stack_pid = -1;
            pause();
        }

        stack_pid = bt_start_app(run_stack, NULL);
        sleep(3);

        BT_MANAGER_LOG("IPC/RPC initialize");
        a_mtk_bt_service_init();
        if (a_mtk_bt_register_stack_exit_cb(func, NULL) < 0)
        {
            BT_MANAGER_LOG("Connect BT stack failed");
            (void)bt_stop_app(stack_pid);
            a_mtk_bt_service_terminate();
            continue;
        }

        memset(&g_gap_func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
        g_gap_func.bt_event_cb = bt_manager_gap_event_callback;
        g_gap_func.bt_get_pairing_key_cb = bt_manager_gap_get_pairing_key_callback;
        g_gap_func.bt_dev_info_cb = bt_manager_gap_app_inquiry_callback;
        if (a_mtkapi_bt_gap_base_init(&g_gap_func, (void *)bt_manager_pv_tag) < 0)
        {
            BT_MANAGER_LOG("base init failed");
            (void)bt_stop_app(stack_pid);
            a_mtk_bt_service_terminate();
            continue;
        }

        (void)signal(SIGCLD, SIG_IGN); /* Don't wait child process */

        bt_manager_action = NONE;
        pthread_mutex_lock(&bt_manager_lock);
        while (1)
        {
            bt_manager_action = WAIT;
            pthread_cond_wait(&bt_manager_signal, &bt_manager_lock);
            BT_MANAGER_LOG("action=%d", bt_manager_action);
            if (bt_manager_action == STACK_EXIT)
            {
                bt_manager_action = NONE;
                a_mtk_bt_service_terminate();
                stack_flag = 1;
                break;
            }
#if 0
            if (bt_manager_action == STACK_START)
            {
                if (stack_pid)
                {
                    BT_MANAGER_LOG("Stop bt stack first(pid:%d)", stack_pid);
                    /*
                 * stack_pid = bt_stop_app(stack_pid);
                 * a_mtk_bt_service_terminate();
                 */
                } else {
                    break;
                }
            }
            if (bt_manager_action == STACK_STOP)
            {
                stack_pid = bt_stop_app(stack_pid);
                a_mtk_bt_service_terminate();
            }
#endif
            if (bt_manager_action == BT_ON)
            {
                a_mtkapi_bt_gap_on_off(1);
            }
            if (bt_manager_action == BT_OFF)
            {
                a_mtkapi_bt_gap_on_off(0);
            }
            bt_manager_action = NONE;
        }
        pthread_mutex_unlock(&bt_manager_lock);
    }
    return 0;
}

