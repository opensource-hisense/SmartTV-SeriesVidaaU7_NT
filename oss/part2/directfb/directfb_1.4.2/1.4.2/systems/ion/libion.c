/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <config.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
     
#include <directfb.h>
#include <core/core.h>
#include <misc/conf.h>

#include "libion.h"

D_DEBUG_DOMAIN( DFBION_Surfaces, "DFBION/Surfaces", "DFBION Surface Pool" );

int ion_open()
{
        int fd = open("/dev/ion", O_RDWR);
        if (fd < 0)
                printf("open /dev/ion failed!\n");
        return fd;
}

int ion_close(int fd)
{
        return close(fd);
}

static int ion_ioctl(int fd, int req, void *arg)
{
        int ret = ioctl(fd, req, arg);
        if (ret < 0) {
                printf("ioctl 0x%x failed with code %d: %s\n", req,
                       ret, strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
          unsigned int flags, ion_handle_t *handle)
{
        int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .heap_id_mask = heap_mask,
                .flags = flags,
        };

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}

int ion_free(int fd, ion_handle_t handle)
{
        struct ion_handle_data data = {
                .handle = handle,
        };
        return ion_ioctl(fd, ION_IOC_FREE, &data);
}

int ion_share(int fd, ion_handle_t handle, int *shared_fd)
{
        int map_fd;
        struct ion_fd_data data = {
                .handle = handle,
        };

        int ret = ion_ioctl(fd, ION_IOC_SHARE, &data);
        if (ret < 0)
                return ret;
        *shared_fd = data.fd;
        if (*shared_fd < 0) {
                printf("share ioctl returned negative fd\n");
                return -1;
        }
        return ret;
}

int ion_import(int fd, int shared_fd, ion_handle_t *handle)
{
        struct ion_fd_data data = {
                .fd = shared_fd,
        };

        int ret = ion_ioctl(fd, ION_IOC_IMPORT, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}



//map share_fd to different process
int ion_cust_import_fd(int fd, int pid, int shared_fd, int *newfd)
{

    struct ion_cust_import_data import_data;
    import_data.fd = shared_fd;
    import_data.pid = pid;
        
    int ret = ion_ioctl(fd, ION_IOC_CUST_IMPORT, &import_data);
    if (ret < 0)
    {
        printf("ion_cust_import_fd fail  \n");
        return ret;
    }
    *newfd = import_data.newfd;
    return ret;
}


#ifdef DFB_ION_TLB
//get the hw info, check weather support tlb , whichi client support tlb, GE/GOP/MIU?
int ion_cust_get_tlbinfo(int fd, mtlb_hardware_info *hw_info)
{
    struct ion_custom_data custom_data;
    mtlb_hardware_info hw_data;

    custom_data.cmd = MTLB_CUST_IOC_GET_HWINO;
    custom_data.arg = &hw_data;

    int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
    if(ret < 0)
    {
        printf("ion_cust_get_tlbinfo fail\n");
        return ret;
    }
    memcpy(hw_info, &hw_data, sizeof(mtlb_hardware_info));
    return ret;
}


int ion_cust_tlb_init(int fd)
{
    struct ion_custom_data custom_data;

    custom_data.cmd = MTLB_CUST_IOC_INIT;
    custom_data.arg = 0;

    int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
    if(ret < 0)
    {
        printf("ion_cust_tlb_init fail\n");
    }
    return ret;
}

int ion_cust_tlb_enable(int fd, mtlb_tlbclient_enable *enable_data)
{
    struct ion_custom_data custom_data;

    custom_data.cmd = MTLB_CUST_IOC_ENABLE;
    custom_data.arg = enable_data;

    int ret =  ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
    if(ret < 0)
    {
        printf("ion_cust_tlb_enable fail\n");
    }
    return ret;
}
#endif

int ion_cust_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
    unsigned int flags, ion_handle_t *handle)
{
    int ret;
    struct ion_cust_allocation_data data = {
            .start = 0,
            .len = len,
            .align = align,
            .heap_id_mask = heap_mask,
            .flags = flags,
    };
    D_DEBUG_ION( DFBION_Surfaces, "ion_ioctl  ION_IOC_CUST_ALLOC  fd=%d,len=%d align=%d  %s(%d)\n",fd,len,align,__FUNCTION__,__LINE__);
    ret = ion_ioctl(fd, ION_IOC_CUST_ALLOC, &data);
    if (ret < 0)
    {
        D_DEBUG_ION( DFBION_Surfaces, "ion_ioctl fail ret=%d  %s(%d)\n",ret,__FUNCTION__,__LINE__);
            return ret;
    }
    *handle = data.handle;
    D_DEBUG_ION( DFBION_Surfaces, "ion_ioctl success handle=%d %s(%d)\n",data.handle,__FUNCTION__,__LINE__);
    return ret;
}

unsigned int  ion_get_user_data(int fd, ion_user_handle_t handle)
{
    int res;
    struct ion_user_data user_data;

    if (fd < 0) {
        return 0;
    }

    user_data.handle = handle;
    user_data.bus_addr = 0;
    D_DEBUG_ION( DFBION_Surfaces, "ion_ioctl  ION_IOC_CUST_ALLOC  fd=%d handle=%d  %s(%d)\n",fd,handle,__FUNCTION__,__LINE__);
    res = ion_ioctl(fd, ION_IOC_GET_CMA_BUFFER_INFO, &user_data);
    if (res < 0)
    {
        D_DEBUG_ION( DFBION_Surfaces, "ion get user data failed  %s(%d)\n",__FUNCTION__,__LINE__);
        return 0;
    }
    else
    {
        D_DEBUG_ION( DFBION_Surfaces, "ion get user data success  %s(%d)\n",__FUNCTION__,__LINE__);
    }

    return user_data.bus_addr;
}


int ion_map(int fd, ion_user_handle_t handle, int *shared_fd)
{
    int res;
    struct ion_fd_data fd_data;

    if (fd < 0) {
        return fd;
    }

    fd_data.handle = handle;
    res = ion_ioctl(fd, ION_IOC_MAP, &fd_data);
    if (res < 0)
    {
        D_DEBUG_ION(DFBION_Surfaces, "ion map failed %s(%d)\n",__FUNCTION__,__LINE__);
        return res;
    }
    else
    {
        D_DEBUG_ION(DFBION_Surfaces, "ion map handle %u success, fd=%d  %s(%d)\n",(unsigned int)handle, fd_data.fd,__FUNCTION__,__LINE__);
        *shared_fd = fd_data.fd;
    }

    return 0;
}


