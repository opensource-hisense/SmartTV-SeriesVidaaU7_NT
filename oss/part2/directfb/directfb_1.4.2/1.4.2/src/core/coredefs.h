/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __CORE__COREDEFS_H__
#define __CORE__COREDEFS_H__
#include <direct/debug.h>
#include <pthread.h>

#ifdef PIC
#define DFB_DYNAMIC_LINKING
#endif

#define MAX_INPUTDEVICES          16
#define MAX_LAYERS                16
/*
 0~3 reserved for op, ip, ve, and oc
 4~9 for the use of fb dev
*/
#define MAX_SCREENS               10

#define MAX_INPUT_GLOBALS          8

#define MAX_SURFACE_BUFFERS        6
#define MAX_SURFACE_POOLS          8
#define MAX_SURFACE_POOL_BRIDGES   4

typedef enum
{
  DFB_LOCK_FOR_EXEC,
  DFB_LOCK_FOR_DESTROY
}DFB_GLOBAL_LOCK_TYPE;

//#define EXTERN_DFB_GLOBAL_LOCK_FLAG extern volatile unsigned int g_dfb_lock_flag;
extern pthread_mutex_t g_dfb_lock_counter_mutex;
extern pthread_mutex_t core_dfb_lock;
extern volatile unsigned int g_dfb_lock_flag;
#if 1
#define DFB_GLOBAL_LOCK( dfb_lock_type) \
    do{ \
        if(dfb_lock_type == DFB_LOCK_FOR_EXEC) \
        { \
                pthread_mutex_lock(&g_dfb_lock_counter_mutex); \
                if((g_dfb_lock_flag&0xffff)>100) \
                { \
                printf(" exec lock count overflow...%s:dfb_lock_flag:%x\n", __FUNCTION__,g_dfb_lock_flag); \
                D_ASSERT(0); \
                }\
               while(g_dfb_lock_flag&0xffff0000) \
               { \
                   pthread_mutex_unlock(&g_dfb_lock_counter_mutex); \
                   printf("dfb api blocked by dfb destroying thread...%s:%08x\n",__FUNCTION__, g_dfb_lock_flag); \
                   usleep(1000*30); \
                   pthread_mutex_lock(&g_dfb_lock_counter_mutex); \
               } \
               g_dfb_lock_flag++; \
               pthread_mutex_unlock(&g_dfb_lock_counter_mutex); \
        } \
        else if(dfb_lock_type == DFB_LOCK_FOR_DESTROY) \
        { \
              unsigned long tryTimes = 0; \
              D_ASSERT((g_dfb_lock_flag&0xffff0000)==0);\
              pthread_mutex_lock(&g_dfb_lock_counter_mutex); \
              g_dfb_lock_flag |= 0x80000000; \
              while(g_dfb_lock_flag& 0xffff && ++tryTimes<100) \
              { \
                  pthread_mutex_unlock(&g_dfb_lock_counter_mutex); \
                  usleep(100*1000); \
                  pthread_mutex_lock(&g_dfb_lock_counter_mutex); \
                } \
                if(tryTimes == 200) \
                      printf("\nwait for the api ending failed forcely destroy dfb core\n"); \
                g_dfb_lock_flag += 0x00010000; \
                pthread_mutex_unlock(&g_dfb_lock_counter_mutex); \
       } \
      }while(0)


#define DFB_GLOBAL_UNLOCK( dfb_lock_type) \
do{ \
        if(dfb_lock_type == DFB_LOCK_FOR_EXEC) \
        {  \
                pthread_mutex_lock(&g_dfb_lock_counter_mutex); \
                if((g_dfb_lock_flag&0xffff) == 0) \
                { \
                   printf("error:unlock count equal to zero...%s:%d\n", __FUNCTION__,__LINE__); \
                   D_ASSERT(0); \
                }\
              g_dfb_lock_flag--; \
              pthread_mutex_unlock(&g_dfb_lock_counter_mutex); \
        } \
        else if(dfb_lock_type == DFB_LOCK_FOR_DESTROY) \
        { \
           g_dfb_lock_flag = 0; \
        } \
}while(0)
#else
#define DFB_GLOBAL_LOCK( dfb_lock_type)
#define DFB_GLOBAL_UNLOCK( dfb_lock_type)
#endif
#endif


