
#include <config.h>

#include <direct/debug.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <fusion/conf.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/pool.h>

#include <core/core.h>
#include <core/surface_pool.h>
#include <misc/conf.h>
#include <MsCommon.h>

typedef struct {
     void *addr;
     void *phys;
     int   pitch;
     void *addr_aligned;
     void *phys_aligned;
} PreallocInVidAllocationData;

typedef struct {
    unsigned long miu0_phys_base;
    unsigned long miu1_phys_base;
    unsigned long miu0_addr_base;  //Mapped Virtual Address of MIU0
    unsigned long miu1_addr_base;  //Mapped Virtual Address of MIU1
    unsigned long miu0_mem_length;
    unsigned long miu1_mem_length;
}PreallocMemoryBase;

static PreallocMemoryBase preallocMemoryBase;

#ifdef PAGESIZE_LOG

#define PAGESIZE (0x1<<PAGESIZE_LOG)
#if PAGESIZE > 0x2000
#define PAGE_SIZE PAGESIZE
#else
#define PAGE_SIZE 0x2000
#endif

#else
#define PAGE_SIZE 0x2000
#endif
/**********************************************************************************************************************/

static int
preallocInVidAllocationDataSize( void )
{
     return sizeof(PreallocInVidAllocationData);
}

static DFBResult
preallocInVidInitPool( CoreDFB                    *core,
                  CoreSurfacePool            *pool,
                  void                       *pool_data,
                  void                       *pool_local,
                  void                       *system_data,
                  CoreSurfacePoolDescription *ret_desc )
{

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( ret_desc != NULL );

     //Map Video Memory:
     //MsOS_MPool_Init();
     MsOS_MPool_Get(0,0,0, true);

     ret_desc->caps              = CSPCAPS_NONE;
     ret_desc->access[CSAID_CPU] = CSAF_READ | CSAF_WRITE;
     ret_desc->access[CSAID_GPU] = CSAF_READ | CSAF_WRITE;
     ret_desc->types             = CSTF_PREALLOCATED_IN_VIDEO | CSTF_INTERNAL;
     ret_desc->priority          = CSPP_DEFAULT;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "Preallocated In Video Memory" );

     return DFB_OK;
}

static DFBResult
preallocInVidDestroyPool( CoreSurfacePool            *pool,
                  void                       *pool_data,
                  void                       *pool_local )
{
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     return DFB_OK;
}

static DFBResult
preallocInVidTestConfig( CoreSurfacePool         *pool,
                    void                    *pool_data,
                    void                    *pool_local,
                    CoreSurfaceBuffer       *buffer,
                    const CoreSurfaceConfig *config )
{
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( config != NULL );

     return (config->flags & CSCONF_PREALLOCATED_IN_VIDEO) ? DFB_OK : DFB_UNSUPPORTED;
}

static DFBResult
preallocInVidAllocateBuffer( CoreSurfacePool       *pool,
                        void                  *pool_data,
                        void                  *pool_local,
                        CoreSurfaceBuffer     *buffer,
                        CoreSurfaceAllocation *allocation,
                        void                  *alloc_data )
{
     int                     index;
     CoreSurface            *surface;
     PreallocInVidAllocationData *alloc = alloc_data;

     int fd;
     void *mem_addr;
     unsigned int  mem_phys_aligned;
     unsigned int   mem_length;

     void * miu_physaddr = NULL;


     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     for (index=0; index<MAX_SURFACE_BUFFERS; index++) {
          if (surface->buffers[index] == buffer)
               break;
     }

     if (index == MAX_SURFACE_BUFFERS)
          return DFB_BUG;

     if (!(surface->config.flags & CSCONF_PREALLOCATED_IN_VIDEO))
          return DFB_BUG;

     if (!surface->config.preallocated[index].addr ||
          surface->config.preallocated[index].pitch < DFB_BYTES_PER_LINE(surface->config.format,
                                                                         surface->config.size.w))
          return DFB_INVARG;

     alloc->phys = surface->config.preallocated[index].addr;

     alloc->pitch = surface->config.preallocated[index].pitch;

     allocation->flags = CSALF_PREALLOCATED | CSALF_VOLATILE;
     allocation->size  = surface->config.preallocated[index].pitch *
                         DFB_PLANE_MULTIPLY( surface->config.format, surface->config.size.h );



     if(alloc->phys >=dfb_config->mst_miu1_cpu_offset)
     {
        miu_physaddr = dfb_config->mst_miu1_hal_offset + alloc->phys - dfb_config->mst_miu1_cpu_offset;
     }
     else
     {
        miu_physaddr = dfb_config->mst_miu0_hal_offset + alloc->phys - dfb_config->mst_miu0_cpu_offset;
     }

    if(NULL == MsOS_MPool_PA2KSEG1(miu_physaddr))
    {

        // in this case ,map the preallocatedin video memory when lock  is called
        alloc->addr_aligned = NULL;
        alloc->addr = NULL;
        alloc->phys_aligned = NULL;
    }
    else
    {
        alloc->phys_aligned = NULL;
        alloc->addr = MsOS_MPool_PA2KSEG1(miu_physaddr);
        alloc->addr_aligned = NULL;

    }


    return DFB_OK;
}

static DFBResult
preallocInVidDeallocateBuffer( CoreSurfacePool       *pool,
                          void                  *pool_data,
                          void                  *pool_local,
                          CoreSurfaceBuffer     *buffer,
                          CoreSurfaceAllocation *allocation,
                          void                  *alloc_data )
{


     unsigned int   mem_length;
     PreallocInVidAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );

/*
     if((alloc->addr_aligned != NULL) &&(alloc->phys_aligned != NULL))
     {
         mem_length =  alloc->phys - alloc->phys_aligned + allocation->size;
         munmap( alloc->addr_aligned, mem_length );

     }
*/

     return DFB_OK;
}

static DFBResult
preallocInVidLock( CoreSurfacePool       *pool,
              void                  *pool_data,
              void                  *pool_local,
              CoreSurfaceAllocation *allocation,
              void                  *alloc_data,
              CoreSurfaceBufferLock *lock )
{
     PreallocInVidAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );


     int fd;
     void *mem_addr;
     unsigned int  mem_phys_aligned;
     unsigned int   mem_length;
     void * miu_physaddr = NULL;

     if(NULL==alloc->addr)
     {
         fd = open( "/dev/mem", O_RDWR | O_SYNC );
         if (fd < 0) {
              D_PERROR( "preallocate buffer: System/DevMem: Opening '%s' failed!\n", "/dev/mem" );
              return DFB_INIT;
         }
         mem_phys_aligned = (unsigned int)(alloc->phys) & (~(unsigned int)(PAGE_SIZE-1));
         mem_length =  alloc->phys - mem_phys_aligned + allocation->size;
         mem_addr = mmap( NULL, mem_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mem_phys_aligned );

         close( fd );

         if (mem_addr == MAP_FAILED) {

             D_PERROR( "System/DevMem: Mapping %d bytes at 0x%08lx via '%s' failed!\n", mem_length, mem_phys_aligned, "/dev/mem" );
             printf( "System/DevMem: Mapping %d bytes at 0x%08lx via '%s' failed!\n", mem_length, mem_phys_aligned, "/dev/mem" );
             return DFB_INIT;
         }

        alloc->phys_aligned = (void *)mem_phys_aligned;
        alloc->addr = mem_addr + ((unsigned int)alloc->phys - mem_phys_aligned);
        alloc->addr_aligned = mem_addr;
    }



    lock->addr  = alloc->addr;
    lock->phys = (u32)alloc->phys;
    lock->pitch = alloc->pitch;

     return DFB_OK;
}

static DFBResult
preallocInVidUnlock( CoreSurfacePool       *pool,
                void                  *pool_data,
                void                  *pool_local,
                CoreSurfaceAllocation *allocation,
                void                  *alloc_data,
                CoreSurfaceBufferLock *lock )
{
     PreallocInVidAllocationData *alloc = alloc_data;
     unsigned int   mem_length;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );


// if get vitural address by  map the address when lock, unmap it.
     if((alloc->addr_aligned!=NULL) &&(alloc->phys_aligned != NULL))
     {

         mem_length =  alloc->phys - alloc->phys_aligned + allocation->size;
         munmap( alloc->addr_aligned, mem_length );
         alloc->addr_aligned = NULL;
         alloc->phys_aligned = NULL;
         alloc->addr = NULL;
     }


     return DFB_OK;
}

const SurfacePoolFuncs IonPreallocSurfacePoolFuncs = {
     .AllocationDataSize = preallocInVidAllocationDataSize,
     .InitPool           = preallocInVidInitPool,
     .DestroyPool        = preallocInVidDestroyPool,
     .TestConfig         = preallocInVidTestConfig,

     .AllocateBuffer     = preallocInVidAllocateBuffer,
     .DeallocateBuffer   = preallocInVidDeallocateBuffer,

     .Lock               = preallocInVidLock,
     .Unlock             = preallocInVidUnlock,
};

