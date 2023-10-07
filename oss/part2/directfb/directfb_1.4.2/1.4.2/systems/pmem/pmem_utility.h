#ifndef __PMEM_PMEM_UTILITY_H__
#define __PMEM_PMEM_UTILITY_H__

typedef struct
{
   unsigned long offsetstart;
   unsigned long offsetend;

   unsigned total_block_size;
   unsigned total_used_mem;
   bool print;
}PMEM_Dump_Info;

void
pmem_dump_surface_buffers( int pool_idx, int block_idx );

#endif
