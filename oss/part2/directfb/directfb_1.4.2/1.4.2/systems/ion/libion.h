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

#ifndef __LIB_ION_H
#define __LIB_ION_H


#include "3.10.40/ion.h"
#include "3.10.40/ion_customer.h"
#include "mdrv_mtlb_st.h"

typedef int ion_handle_t;

typedef struct 
{
    //input parameters
     ion_handle_t handle;
    
     //output parameters
     EN_MTLB_TLB_Client client;
     EN_MTLB_AddressType addressType;
     unsigned long mapaddr;
     unsigned char miu;
     unsigned long miu_offset;
    
     unsigned char tlb_table_change;
}ion_lock_data;

typedef struct
{
    size_t length;
    unsigned int heap_mask;
    int flag;
    ion_handle_t handle;
    int          shared_fd;
    int          pid;
    int         serial; //serial number
    void *cpu_addr;
    unsigned long phys;
    bool initzero;
}ion_alloc_data;

int ion_open();
int ion_close(int fd);
int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
          unsigned int flags, ion_handle_t *handle);

int ion_free(int fd, ion_handle_t handle);
int ion_share(int fd, ion_handle_t handle, int *shared_fd);
int ion_import(int fd, int shared_fd, ion_handle_t *handle);





int ion_cust_import_fd(int fd, int pid, int shared_fd, int* newfd); //map share_fd to different process

int ion_cust_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
    unsigned int flags, ion_handle_t *handle);


unsigned int  ion_get_user_data(int fd, ion_user_handle_t handle);

int ion_map(int fd, ion_user_handle_t handle, int *shared_fd);



#ifdef DFB_ION_TLB
int ion_cust_get_tlbinfo(int fd, mtlb_hardware_info *hw_info);
int ion_cust_tlb_init(int fd);
int ion_cust_tlb_enable(int fd, mtlb_tlbclient_enable *enable_data);
#endif /* DFB_ION_TLB */

#endif /* __LIB_ION_H */
