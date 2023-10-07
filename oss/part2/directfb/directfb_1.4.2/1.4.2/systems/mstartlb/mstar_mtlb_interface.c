#include <config.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <misc/conf.h>
#include <stdlib.h>
#include "mstartlb.h"
#include "surfacemanager.h"

static int KPageSize;

DFBResult OpenHWTLBDevice(DevMstarTLBData    *data)
{
    int fd = -1;
    fd = open(DEV_MSTARTLB,O_RDWR | O_SYNC );

    if(fd > 0)
    {
        int val = fcntl(fd,F_GETFD);
        val |= FD_CLOEXEC;
        fcntl(fd,F_SETFD,val);

        data->fd = fd ;
        KPageSize = sysconf( _SC_PAGESIZE );
        return DFB_OK;
    }
    else
    {
        printf("\nopen mstar tlb driver failed\n");
        return DFB_INIT;
    }

}


void CloseHWTLBDevice(DevMstarTLBData    *data)
{
    if(data->fd > 0)
        close(data->fd);
}


DFBResult HWTLB_GetRange(DevMstarTLBData *data )
{
    int fd = data->fd;
    DevMstarTLBDataShared *shared = data->shared;
    MemRange memRange;

    if(ioctl(fd,MTLB_IOC_GET_RANGE, &memRange)<0)
    {
        printf("\ncall HWTLB_GET_TLBRANGE failed\n");
        close(fd);
        return DFB_INIT;
    }
    else
    {
        printf("MTLB_IOC_GET_RANGE: start = 0x%x, size =0x%x\n", memRange.start, memRange.size);

        shared->msttlb_TLBAddr_start  = memRange.start * KPageSize;
        shared->msttlb_TLBAddr_length = memRange.size * KPageSize; 
        shared->msttlb_videoMem_length = shared->msttlb_TLBAddr_length;
        
        printf("shared->msttlb_videoMem_length = 0x%x, KPageSize =0x%x\n",shared->msttlb_videoMem_length, KPageSize);
        return DFB_OK;
    }
 }

DFBResult MapHWTLBMem(DevMstarTLBData    *data)
{
    DevMstarTLBDataShared *shared = data->shared;
    int fd = data->fd ;

    //buf = (char *)mmap(0x40000000, 32*1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED, dev, 0);
    data->mem = mmap(0x40000000,shared->msttlb_videoMem_length,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    if(data->mem==MAP_FAILED)
    {
        printf("\nno virtual addr space  for tlbaddr  from: 0x%08x-> length: 0x%x\n",shared->msttlb_TLBAddr_start,shared->msttlb_TLBAddr_length);
        close(fd);
        return DFB_INIT;
    }
    printf("data->mem = 0x%x, pid = %d \n", data->mem, getpid());
    return DFB_OK;
}

void UnMapHWTLBMem(DevMstarTLBData    *data )
{
    if(data->mem!=NULL)
        munmap(data->mem, data->shared->msttlb_videoMem_length);
}

DFBResult HWTLB_Enable(DevMstarTLBData*data)
{
    int client = E_C_GE;

    if(ioctl(data->fd, MTLB_IOC_MTLB_ENABLE, &client))
    {
        printf("HWTLB_Enable failed");
        close(data->fd);
        return DFB_FAILURE;
    }
    else
    {     
     return DFB_OK;
    }
}

DFBResult HWTLB_TransferBuffer(DevMstarTLBData*data,MoveInfo *transfer)
{
    transfer->dest = transfer->dest / KPageSize;
    if(ioctl(data->fd,MTLB_IOC_MOVE_BUF,transfer)<0)
    {
        printf("\nHWTLB_TRANSFER_BUFFER failed\n");
        return DFB_FAILURE;
    }
    else
    {     
        return DFB_OK;
    }
}

DFBResult HWTLB_AllocateBuffer(DevMstarTLBData * data,BufferInfo * tlbAlloc)
{
    tlbAlloc->tlb_size =  tlbAlloc->tlb_size / KPageSize ;
    tlbAlloc->tlb_start =  tlbAlloc->tlb_start / KPageSize ;

    if(ioctl(data->fd,MTLB_IOC_ALLOC_BUF,tlbAlloc)<0)
    {
        return DFB_FAILURE;
    }
    else
    {
        return DFB_OK;
    }
}


DFBResult HWTLB_DeallocteBuffer(DevMstarTLBData * data,AllocationHandle handle)
{
    if(ioctl(data->fd,MTLB_IOC_FREE_BUF,&handle)<0)
    {
        return DFB_FAILURE;
    }
    else
    {
        return DFB_OK;
    }
}
