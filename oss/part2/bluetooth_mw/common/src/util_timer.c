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
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/


#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>

#include "util_timer.h"

#define UTIL_TIMER_CODE_OK              (0)
#define UTIL_TIMER_CODE_FAIL            (-1)
#define UTIL_TIMER_CODE_INV_ARG         (-2)
#define UTIL_TIMER_CODE_NO_RESOURCE     (-3)

#define time_before(a, b) ((a).tv_sec < (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_nsec < (b).tv_nsec))

#define ASSERT(x)           \
    do                      \
    {                       \
        int y = (int)(x);   \
        assert(y);          \
    } while(0)

#define VERIFY(x)       ASSERT(x)

#define FREE_SAFE(value) \
    if (value != NULL) { \
        free(value); \
        value = NULL; \
    }

typedef struct _TIMER_ENTRY_T
{
    struct _TIMER_ENTRY_T *previous;
    struct _TIMER_ENTRY_T *next;
    struct timespec abstime;
    struct timespec interval;
    UTIL_TIMER_HANDLER pf_callback;
    void *cb_args;  //set by the timer starter, and also shall be freed by the caller
    uint32_t id;
    UTIL_TIMER_REPEAT_TYPE_T repeat;
} TIMER_ENTRY_T;

static pthread_t s_timer_thread;
static BOOL s_timer_quit;
static pthread_cond_t s_timer_list_cond;
static pthread_mutex_t s_timer_list_mutex;
static TIMER_ENTRY_T *s_timer_list;
static UINT32 s_timer_res;

static void timer_list_add(TIMER_ENTRY_T *pt_timer)
{
    if (s_timer_list != NULL)
    {
        TIMER_ENTRY_T *p = s_timer_list;
        if (time_before(pt_timer->abstime, p->abstime))
        {
            s_timer_list = pt_timer;
        }
        else
        {
            for (p = p->next; p != s_timer_list; p = p->next)
                if (time_before(pt_timer->abstime, p->abstime))
                    break;
        }
        pt_timer->previous = p->previous;
        pt_timer->next = p;
        p->previous->next = pt_timer;
        p->previous = pt_timer;
    }
    else
    {
        s_timer_list = pt_timer->next = pt_timer->previous = pt_timer;
    }
}


static void timer_list_remove(TIMER_ENTRY_T *pt_timer)
{
    if (pt_timer->previous == pt_timer)
    {
        s_timer_list = NULL;
    }
    else
    {
        pt_timer->previous->next = pt_timer->next;
        pt_timer->next->previous = pt_timer->previous;
        if (s_timer_list == pt_timer)
        {
            s_timer_list = pt_timer->next;
        }
    }
    pt_timer->previous = pt_timer->next = NULL;
}


static void *TimerProc(void *arg)
{
    TIMER_ENTRY_T *pt_timer;
    struct timespec abstime;
    int ret;
    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    while (!s_timer_quit)
    {
        while (s_timer_list == NULL)
        {
            ret = pthread_cond_wait(&s_timer_list_cond, &s_timer_list_mutex);
            VERIFY(ret == 0);
        }

        VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);

        while (s_timer_list && time_before(s_timer_list->abstime, abstime))
        {
            pt_timer = s_timer_list;
            VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
            if(pt_timer->pf_callback)
            {
                pt_timer->pf_callback(pt_timer->id, pt_timer->cb_args);
            }
            VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
            timer_list_remove(pt_timer);

            if (pt_timer->interval.tv_sec != 0 || pt_timer->interval.tv_nsec != 0)
            {
                pt_timer->abstime.tv_sec += pt_timer->interval.tv_sec;
                pt_timer->abstime.tv_nsec += pt_timer->interval.tv_nsec;
                if (pt_timer->abstime.tv_nsec >= 1000000000)
                {
                    pt_timer->abstime.tv_nsec -= 1000000000;
                    pt_timer->abstime.tv_sec++;
                }
                timer_list_add(pt_timer);
            }
        }
        if (s_timer_list != NULL)
        {
            ret = pthread_cond_timedwait(&s_timer_list_cond, &s_timer_list_mutex, &s_timer_list->abstime);
            VERIFY(ret == 0 || ret == ETIMEDOUT);
        }
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    return NULL;
}

INT32 util_timer_start(UTIL_TIMER_T *pt_timer)
{
    TIMER_ENTRY_T *entry = NULL;
    struct timespec delay;

    if (pt_timer == NULL)
    {
        return UTIL_TIMER_CODE_INV_ARG;
    }
    else if (pt_timer->handle == NULL)  //the timer is not created yet, create it
    {
        if ((pt_timer->pf_handler == NULL) || (pt_timer->delay_ms == 0) ||
            ((pt_timer->repeat != UTIL_TIMER_REPEAT_TYPE_ONCE) && (pt_timer->repeat != UTIL_TIMER_REPEAT_TYPE_REPEAT)))
        {
            return UTIL_TIMER_CODE_INV_ARG;
        }
        //Allocate a timer entry for this timer
        entry = (TIMER_ENTRY_T *)calloc(1, sizeof(TIMER_ENTRY_T));
        if (entry == NULL)
        {
            return UTIL_TIMER_CODE_NO_RESOURCE;
        }
        pt_timer->handle = (VOID *)entry;
        entry->repeat = pt_timer->repeat;
        entry->id = pt_timer->id;
    }
    else    //the timer entry exists
    {
        entry = (TIMER_ENTRY_T *)pt_timer->handle;
    }

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    //@ if pt_timer has been start before,  remove formal one.
    if (entry->previous != NULL)
    {
        timer_list_remove(entry);
        VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);

    delay.tv_sec = pt_timer->delay_ms / 1000;
    delay.tv_nsec = (pt_timer->delay_ms % 1000) * 1000000;

    VERIFY(clock_gettime(CLOCK_MONOTONIC, &entry->abstime) == 0);
    entry->abstime.tv_sec += delay.tv_sec;
    entry->abstime.tv_nsec += delay.tv_nsec;
    if (entry->abstime.tv_nsec >= 1000000000)
    {
        entry->abstime.tv_nsec -= 1000000000;
        entry->abstime.tv_sec++;
    }
    if (entry->repeat == UTIL_TIMER_REPEAT_TYPE_ONCE)
    {
        entry->interval.tv_sec = 0;
        entry->interval.tv_nsec = 0;
    }
    else
    {
        entry->interval.tv_sec = delay.tv_sec;
        entry->interval.tv_nsec = delay.tv_nsec;
    }

    entry->pf_callback = pt_timer->pf_handler;
    entry->cb_args = pt_timer->pv_args;


    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    timer_list_add(entry);
    VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    return UTIL_TIMER_CODE_OK;
}

INT32 util_timer_stop (VOID * h_timer)
{
    TIMER_ENTRY_T *pt_timer;
    struct timespec abstime;

    if (NULL == h_timer)
    {
        return UTIL_TIMER_CODE_FAIL;
    }
    pt_timer = (TIMER_ENTRY_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        timer_list_remove(pt_timer);
        VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
        VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
        pt_timer->abstime.tv_sec -= abstime.tv_sec;
        if (pt_timer->abstime.tv_nsec < abstime.tv_nsec)
        {
            pt_timer->abstime.tv_nsec += 1000000000;
            pt_timer->abstime.tv_sec--;
        }
        pt_timer->abstime.tv_nsec -= abstime.tv_nsec;
    }
    else
    {
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    }
    return UTIL_TIMER_CODE_OK;
}


INT32 util_timer_delete (VOID * h_timer)
{
    TIMER_ENTRY_T *pt_timer;

    pt_timer = (TIMER_ENTRY_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        timer_list_remove(pt_timer);
        VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);

    free(pt_timer);
    return UTIL_TIMER_CODE_OK;
}


INT32 util_timer_resume (VOID * h_timer)
{
    TIMER_ENTRY_T *pt_timer;
    struct timespec abstime;

    pt_timer = (TIMER_ENTRY_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
        return UTIL_TIMER_CODE_FAIL;
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    pt_timer->abstime.tv_sec  += abstime.tv_sec;
    pt_timer->abstime.tv_nsec += abstime.tv_nsec;
    if (pt_timer->abstime.tv_nsec >= 1000000000)
    {
        pt_timer->abstime.tv_nsec -= 1000000000;
        pt_timer->abstime.tv_sec++;
    }
    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    timer_list_add(pt_timer);
    VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);

    return UTIL_TIMER_CODE_OK;
}

#if 0
UINT32 u_os_get_sys_tick (VOID)
{
    struct timespec abstime;
    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    return abstime.tv_sec * (1000 / s_timer_res) + abstime.tv_nsec / (1000000 * s_timer_res);
}


UINT32 u_os_get_sys_tick_period (VOID)
{
    return s_timer_res;
}
#endif

INT32 util_timer_init (VOID)
{
    struct timespec res;
    pthread_condattr_t condattr;
    pthread_attr_t     attr;

    VERIFY(clock_getres(CLOCK_MONOTONIC, &res) == 0);
    s_timer_res = (res.tv_nsec + 500000) / 1000000;

    VERIFY(pthread_condattr_init(&condattr) == 0);
    VERIFY(pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) == 0);
    VERIFY(pthread_cond_init(&s_timer_list_cond, &condattr) == 0);
    VERIFY(pthread_mutex_init(&s_timer_list_mutex, NULL) == 0);

    VERIFY(pthread_attr_init(&attr) == 0);
    VERIFY(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);

    VERIFY(pthread_create(&s_timer_thread, &attr, TimerProc, NULL) == 0);
    return UTIL_TIMER_CODE_OK;
}

