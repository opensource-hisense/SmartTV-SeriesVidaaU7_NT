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

#include <config.h>

#include <fusion/shmalloc.h>

#include <directfb.h>
#include <directfb_util.h>

#include <core/core.h>

#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <gfx/convert.h>

#include "pmem.h"

#include "surfacemanager.h"

D_DEBUG_DOMAIN( SurfMan, "SurfaceManager", "DirectFB Surface Manager" );




static Chunk *split_chunk ( SurfaceManager *manager,
                            Chunk          *chunk,
                            int             length );

static Chunk *free_chunk  ( SurfaceManager *manager,
                            Chunk          *chunk );

static Chunk *occupy_chunk( SurfaceManager        *manager,
                            Chunk                 *chunk,
                            CoreSurfaceAllocation *allocation,
                            int                    length,
                            int                    pitch );




bool dfb_surfacemanager_bMemBlockUsed(int block_index,
                                                   SurfaceManager          *manager,
                                                   void               *pool_local)
{
   DevPMemPoolLocalData *local  = pool_local;
   DevPMemPoolSharedData  *shared = local->sharedData;
   Chunk *chunk;
   void  *next;

    D_DEBUG_AT( SurfMan, "%s( %p )\n", __FUNCTION__, manager );

    D_MAGIC_ASSERT( manager, SurfaceManager );
    D_ASSERT(block_index>=0 && block_index<POOL_BLOCKS_MAX);

    /* Deallocate all video chunks. */
    chunk = shared->chunk_head[block_index];
    if(!chunk)
          return false;
     D_MAGIC_ASSERT(chunk, Chunk );
     D_ASSERT(chunk->block_index == block_index);
     D_ASSERT(!chunk->prev || chunk->prev->block_index != block_index);

    while (chunk&&(chunk->block_index==block_index))
    {
          next = chunk->next;
          if(NULL!=chunk->buffer)
          {
                    return true;
           }
         chunk = next;
    }

   return false;

}

void dfb_surfacemanager_deleteMemBlock(int block_index,
                                               SurfaceManager          *manager,
                                               void               *pool_local)
{
     Chunk *chunk;
     Chunk  *next;
     int block_size = 0;
     DevPMemPoolLocalData *local  = pool_local;
     DevPMemPoolSharedData  *shared = local->sharedData;
     D_DEBUG_AT( SurfMan, "%s( %p )\n", __FUNCTION__, manager );

     D_MAGIC_ASSERT( manager, SurfaceManager );

     D_ASSERT(block_index>=0 && block_index<POOL_BLOCKS_MAX);

     /* Deallocate all video chunks. */

     chunk = shared->chunk_head[block_index];
     D_INFO("delete block %d, head trunk %08x\n", block_index, chunk);
     D_ASSERT(chunk);
     D_MAGIC_ASSERT(chunk, Chunk );
     D_ASSERT(chunk->block_index == block_index);
     D_ASSERT(!chunk->prev || chunk->prev->block_index != block_index);


    // printf("\nthe delete block size is %x:block_index :%x\n",block_size,block_index);
     while (chunk&&(chunk->block_index==block_index)) {

          D_ASSERT(!chunk->buffer);
          block_size += chunk->length;
          next = chunk->next;


         if(manager->chunks == chunk)
         {
             D_ASSERT(chunk->prev == NULL);
             manager->chunks = next;
         }

          if(next)
               next->prev = chunk->prev;
          if(chunk->prev)
              chunk->prev->next = next;

          D_MAGIC_CLEAR( chunk );
          SHFREE( manager->shmpool, chunk );
          chunk = next;
          if(chunk)
           {
              D_MAGIC_ASSERT(chunk, Chunk );
           }
     }

     shared->chunk_head[block_index] = NULL;


    manager->avail -= block_size;
    manager->length -= block_size;

}

DFBResult dfb_surfacemanager_appendMemblock(int block_index, unsigned addr_phys,
                                               int      block_size,
                                               SurfaceManager          *manager,
                                               void               *pool_local)
{
    DevPMemPoolLocalData *local  = pool_local;
    DevPMemPoolSharedData  *shared = local->sharedData;

    CoreDFB *core = local->core;
    FusionSHMPoolShared *pool;
    D_ASSERT(local!=NULL);
    D_ASSERT(core!=NULL);
    D_ASSERT(manager!=NULL);
    Chunk *c;
    Chunk               *chunk;

    manager->avail += block_size;
    manager->length += block_size;

// move the link pointer to the tail of the list
    c = manager->chunks;
    if(c)
    {
        while(NULL!=c->next)
       {
            c=c->next;
       }
     }

   pool = dfb_core_shmpool( core );
   chunk = SHCALLOC( pool, 1, sizeof(Chunk) );
   if (!chunk) {
      D_OOSHM();
      return DFB_NOSHAREDMEMORY;
   }


//append to the list
   chunk->block_index = block_index;
   chunk->length = block_size;
   chunk->offset = addr_phys;
   D_MAGIC_SET( chunk, Chunk );
   if(c)
   {
       c->next = chunk;
   }
   else
   {
       manager->chunks = chunk;
   }
   chunk->prev = c ;
   D_INFO("new trunk %08x for block %d\n", chunk, block_index);
   shared->chunk_head[block_index] = chunk;
   return DFB_OK;

}


DFBResult
dfb_surfacemanager_fragmentMerge(CoreDFB                *core, int pool_idx,
                                SurfaceManager          *manager,
                                void               *pool_local)

{

     DevPMemPoolLocalData *local  = pool_local;
     DevPMemPoolSharedData  *shared = local->sharedData;
     DevPMemData *devPMem =       local->systemData;
     Chunk *c;
     struct timeval start,end ;
     c = manager->chunks;

     if(!c)
     {
          D_ASSERT(manager->avail==0 && manager->length==0);
          return DFB_OK;
     }
     D_INFO("\ncoming into the fragment merge \n");
     gettimeofday( &start, NULL );

     while(NULL!=c->next)
     {
        c=c->next;
     }
     while(c)
     {
        if(c->buffer&&c->next&&(NULL==c->next->buffer)&&(c->block_index==c->next->block_index))
        {
           CoreSurfaceBuffer *buffer = c->buffer;
           CoreSurface *surface  = buffer->surface;
           if(!(surface->type&(CSTF_LAYER)) && !(surface->config.caps & DSCAPS_STATIC_POS))
           {
              DFBResult      ret = DFB_OK;
              Chunk *next = c->next;
              void * source;
              void * dest;

              if(DFB_OK != PMemAssureMappedMemory(devPMem, pool_idx, c->block_index))
              {
                         c = c->prev;
                         continue;
              }
              D_ASSERT(c->offset>=devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys
                  && c->offset+c->length<=devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys+
                  devPMem->pmemBlockData[pool_idx][c->block_index].last_mem_len);
              D_ASSERT(next->offset>=devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys
                  && next->offset+next->length<=devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys+
                  devPMem->pmemBlockData[pool_idx][c->block_index].last_mem_len);

              source = devPMem->pmemBlockData[pool_idx][c->block_index].pmem_vbase+ c->offset-devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys;
              dest  = devPMem->pmemBlockData[pool_idx][c->block_index].pmem_vbase +c->offset + next->length-devPMem->pmemBlockData[pool_idx][c->block_index].last_cpu_phys;
              //we need to use the fusionprevial here at the buffer level

             ret = dfb_surface_trylock( surface );
             if(ret)
             {
                 printf("\ntry to lock the currect allocation surface failed\n");
                 c = c->prev;
                 continue;
             }
             if(buffer->locked)
             {
                dfb_surface_unlock( surface );
                c = c->prev;

                continue;
             }
            if (c->allocation->accessed[CSAID_GPU] & (CSAF_WRITE|CSAF_READ)) {
               /* ...wait for the operation to finish. */
                dfb_gfxcard_wait_serial(&c->allocation->gfxSerial);
               c->allocation->accessed[CSAID_GPU] &= ~(CSAF_WRITE|CSAF_READ);
               /* Software read access after hardware write requires flush of the (bus) read cache. */
               //dfb_gfxcard_flush_read_cache();
           }

             /*
              //move the content of the memory
              printf("\nmove the content of chunk\n");
              printf("\nthe local->mem is %x\n",local->mem);
              printf("\nthe source is %x\n",source);
              printf("\nthe dest is %x\n",dest);
              */
            memmove(dest,source,c->length);

            //modify the c chunk info, next chunk info
            next->offset = c->offset;
            c->offset = c->offset + next->length;

            c->allocation->offset = c->offset;
            ((DevPMemAllocationData *)c->allocation->data)->offset = c->offset;


            //swap the chunk c and chunk next in the list
            if(next->next)
              next->next->prev = c;
            c->next = next->next;

            if(c->prev)
                c->prev->next = next;
            next->prev = c->prev;

            c->prev = next;
            next->next = c;
            //move the chunk point c to the prev chunk
            if(c == shared->chunk_head[c->block_index])
            {
               D_ASSERT(!next->prev || next->prev->block_index != next->block_index);
               shared->chunk_head[c->block_index] = next;
            }
            c = next;
            dfb_gfxcard_flush_texture_cache();
            dfb_surface_unlock( surface );

           }
        }
        else if((NULL==c->buffer)&&c->next&&(NULL==c->next->buffer)&&(c->block_index==c->next->block_index))
        {

          //printf("\nmergeing with the next chunk\n");
          Chunk *next = c->next;
          c->length += next->length;
          c->next = next->next;
          if (c->next)
               c->next->prev = c;

          D_MAGIC_CLEAR( next );

          SHFREE( manager->shmpool, next );
        }

        c=c->prev;
     }
    gettimeofday( &end, NULL );

    D_INFO("\n%ld s:%ld:us\n",end.tv_sec-start.tv_sec,end.tv_usec-start.tv_usec);
    return DFB_OK;
}

DFBResult
dfb_surfacemanager_create( CoreDFB         *core,
                           SurfaceManager **ret_manager )
{
     FusionSHMPoolShared *pool;
     SurfaceManager      *manager;
     Chunk               *chunk;

     D_DEBUG_AT( SurfMan, "%s( %p)\n", __FUNCTION__, core );

     D_ASSERT( core != NULL );
     D_ASSERT( ret_manager != NULL );

     pool = dfb_core_shmpool( core );

     manager = SHCALLOC( pool, 1, sizeof(SurfaceManager) );
     if (!manager)
          return D_OOSHM();

     manager->shmpool = pool;
     manager->chunks  = NULL;
     manager->offset  = 0;
     manager->length  = 0;
     manager->avail   = 0;

     D_MAGIC_SET( manager, SurfaceManager );

     D_DEBUG_AT( SurfMan, "  -> %p\n", manager );

     *ret_manager = manager;

     return DFB_OK;
}

void
dfb_surfacemanager_destroy( SurfaceManager *manager )
{
     Chunk *chunk;
     void  *next;

     D_DEBUG_AT( SurfMan, "%s( %p )\n", __FUNCTION__, manager );

     D_MAGIC_ASSERT( manager, SurfaceManager );

     /* Deallocate all video chunks. */
     chunk = manager->chunks;
     while (chunk) {
          next = chunk->next;

          D_MAGIC_CLEAR( chunk );

          SHFREE( manager->shmpool, chunk );

          chunk = next;
     }

     D_MAGIC_CLEAR( manager );

     /* Deallocate manager struct. */
     SHFREE( manager->shmpool, manager );
}

/** public functions NOT locking the surfacemanger theirself,
    to be called between lock/unlock of surfacemanager **/

DFBResult dfb_surfacemanager_allocate( CoreDFB                *core,
                                       SurfaceManager         *manager,
                                       CoreSurfaceBuffer      *buffer,
                                       CoreSurfaceAllocation  *allocation,
                                       Chunk                 **ret_chunk )
{
     int pitch;
     int length;
     Chunk *c;
     CoreGraphicsDevice *device;

     Chunk *best_free = NULL;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     if (ret_chunk)
          D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     else
          D_ASSUME( allocation == NULL );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
                 buffer->surface->config.size.w, buffer->surface->config.size.h,
                 dfb_pixelformat_name( buffer->surface->config.format ) );

     if (manager->suspended)
        return DFB_SUSPENDED;


     /* FIXME: Only one global device at the moment. */
     device = dfb_core_get_part( core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );

     dfb_gfxcard_calc_buffer_size( device, buffer, &pitch, &length );

     D_DEBUG_AT( SurfMan, "  -> pitch %d, length %d\n", pitch, length );

     if (manager->avail < length)
     {
         D_INFO("\nthe required length is %d ,the avail is %d, need to aquire more video memory\n",length,manager->avail);
         return DFB_TEMPUNAVAIL;
     }

     D_ASSERT(length);

     /* examine chunks */
     c = manager->chunks;

     D_ASSERT(c);//if c equal to null, avail should be zero


     while (c) {

          D_MAGIC_ASSERT( c, Chunk );

          if (!c->buffer && c->length >= length) {
               /* NULL means check only. */
               if (!ret_chunk)
                    return DFB_OK;

               /* found a nice place to chill */
               if (!best_free  ||  best_free->length > c->length)
                    /* first found or better one? */
                    best_free = c;

               if (c->length == length)
                    break;
          }

          c = c->next;
     }


     if (best_free) {
          D_DEBUG_AT( SurfMan, "  -> found free (%d)\n", best_free->length );

          /* NULL means check only. */
          if (ret_chunk)
               *ret_chunk = occupy_chunk( manager, best_free, allocation, length, pitch );
          return DFB_OK;
     }

     D_DEBUG_AT( SurfMan, "  -> failed (%d/%d avail)\n", manager->avail, manager->length );

     /* no luck */


         D_INFO("\n====> the available memory is enough but can not find the free place to allocate memeory!!!!!!!!!\n");
         D_INFO("\nthe request memory length is %d\n",length);
         D_INFO("\nthe avail memory is %d\n",manager->avail);




     return DFB_NOVIDEOMEMORY;
}

DFBResult dfb_surfacemanager_displace( CoreDFB           *core,
                                       SurfaceManager    *manager,
                                       CoreSurfaceBuffer *buffer )
{
     int                    length;
     Chunk                 *multi_start = NULL;
     int                    multi_size  = 0;
     int                    multi_tsize = 0;
     int                    multi_count = 0;
     Chunk                 *bestm_start = NULL;
     int                    bestm_count = 0;
     int                    bestm_size  = 0;
     int                    min_toleration;
     Chunk                 *chunk;
     CoreGraphicsDevice    *device;
     CoreSurfaceAllocation *smallest = NULL;

     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );

     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
                 buffer->surface->config.size.w, buffer->surface->config.size.h,
                 dfb_pixelformat_name( buffer->surface->config.format ) );

     /* FIXME: Only one global device at the moment. */
     device = dfb_core_get_part( core, DFCP_GRAPHICS );
     D_ASSERT( device != NULL );

     dfb_gfxcard_calc_buffer_size( dfb_core_get_part( core, DFCP_GRAPHICS ), buffer, NULL, &length );

     min_toleration = manager->min_toleration/8 + 2;

     D_DEBUG_AT( SurfMan, "  -> %7d required, min toleration %d\n", length, min_toleration );

     chunk = manager->chunks;
     while (chunk) {
          CoreSurfaceAllocation *allocation;

          D_MAGIC_ASSERT( chunk, Chunk );

          allocation = chunk->allocation;
          if (allocation) {
               CoreSurfaceBuffer *other;
               int                size;

               D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
               D_ASSERT( chunk->buffer == allocation->buffer );
               D_ASSERT( chunk->length >= allocation->size );

               other = allocation->buffer;
               D_MAGIC_ASSERT( other, CoreSurfaceBuffer );

               if (other->locked) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d locked %dx\n", allocation->size, other->locked );
                    goto next_reset;
               }

               if (other->policy > buffer->policy) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d policy %d > %d\n", allocation->size, other->policy, buffer->policy );
                    goto next_reset;
               }

               if (other->policy == CSP_VIDEOONLY) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d policy videoonly\n", allocation->size );
                    goto next_reset;
               }

               chunk->tolerations++;
               if (chunk->tolerations > 0xff)
                    chunk->tolerations = 0xff;

               if (other->policy == buffer->policy && chunk->tolerations < min_toleration) {
                    D_DEBUG_AT( SurfMan, "  ++ %7d tolerations %d/%d\n",
                                allocation->size, chunk->tolerations, min_toleration );
                    goto next_reset;
               }

               size = allocation->size;

               if (chunk->prev && !chunk->prev->allocation)
                    size += chunk->prev->length;

               if (chunk->next && !chunk->next->allocation)
                    size += chunk->next->length;

               if (size >= length) {
                    if (!smallest || smallest->size > allocation->size) {
                         D_DEBUG_AT( SurfMan, "  => %7d [%d] < %d, tolerations %d\n",
                                     allocation->size, size, smallest ? smallest->size : 0, chunk->tolerations );

                         smallest = allocation;
                    }
                    else
                         D_DEBUG_AT( SurfMan, "  -> %7d [%d] > %d\n", allocation->size, size, smallest->size );
               }
               else
                    D_DEBUG_AT( SurfMan, "  -> %7d [%d]\n", allocation->size, size );
          }
          else
               D_DEBUG_AT( SurfMan, "  -  %7d free\n", chunk->length );


          if (!smallest) {
               if (!multi_start) {
                    multi_start = chunk;
                    multi_tsize = chunk->length;
                    multi_size  = chunk->allocation ? chunk->length : 0;
                    multi_count = chunk->allocation ? 1 : 0;
               }
               else {
                    multi_tsize += chunk->length;
                    multi_size  += chunk->allocation ? chunk->length : 0;
                    multi_count += chunk->allocation ? 1 : 0;

                    while (multi_tsize >= length && multi_count > 1) {
                         if (!bestm_start || bestm_size > multi_size * multi_count / bestm_count) {
                              D_DEBUG_AT( SurfMan, "                =====> %7d, %7d %2d used [%7d %2d]\n",
                                          multi_tsize, multi_size, multi_count, bestm_size, bestm_count );

                              bestm_size  = multi_size;
                              bestm_start = multi_start;
                              bestm_count = multi_count;
                         }
                         else
                              D_DEBUG_AT( SurfMan, "                -----> %7d, %7d %2d used\n",
                                          multi_tsize, multi_size, multi_count );

                         if (multi_count <= 2)
                              break;

                         if (!multi_start->allocation) {
                              multi_tsize -= multi_start->length;
                              multi_start  = multi_start->next;
                         }

                         D_ASSUME( multi_start->allocation != NULL );

                         multi_tsize -= multi_start->length;
                         multi_size  -= multi_start->allocation ? multi_start->length : 0;
                         multi_count -= multi_start->allocation ? 1 : 0;
                         multi_start  = multi_start->next;
                    }
               }
          }

          chunk = chunk->next;

          continue;


next_reset:
          multi_start = NULL;

          chunk = chunk->next;
     }

     if (smallest) {
          D_MAGIC_ASSERT( smallest, CoreSurfaceAllocation );
          D_MAGIC_ASSERT( smallest->buffer, CoreSurfaceBuffer );

          smallest->flags |= CSALF_MUCKOUT;

          D_DEBUG_AT( SurfMan, "  -> offset %lu, size %d\n", smallest->offset, smallest->size );

          return DFB_OK;
     }

     if (bestm_start) {
          chunk = bestm_start;

          while (bestm_count) {
               CoreSurfaceAllocation *allocation = chunk->allocation;

               if (allocation) {
                    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
                    D_MAGIC_ASSERT( allocation->buffer, CoreSurfaceBuffer );

                    allocation->flags |= CSALF_MUCKOUT;

                    bestm_count--;
               }

               D_DEBUG_AT( SurfMan, "  ---> offset %d, length %d\n", chunk->offset, chunk->length );

               chunk = chunk->next;
          }

          return DFB_OK;
     }

     return DFB_NOVIDEOMEMORY;
}

DFBResult dfb_surfacemanager_deallocate( SurfaceManager *manager,
                                         Chunk          *chunk )
{
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( manager, SurfaceManager );



     D_MAGIC_ASSERT( chunk, Chunk );

     buffer = chunk->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( buffer->surface, CoreSurface );


     D_DEBUG_AT( SurfMan, "%s( %p ) <- %dx%d %s\n", __FUNCTION__, buffer,
                 buffer->surface->config.size.w, buffer->surface->config.size.h,
                 dfb_pixelformat_name( buffer->surface->config.format ) );

     free_chunk( manager, chunk );



     return DFB_OK;
}

/** internal functions NOT locking the surfacemanager **/

static Chunk *
split_chunk( SurfaceManager *manager, Chunk *c, int length )
{
     Chunk *newchunk;

     D_MAGIC_ASSERT( c, Chunk );

     if (c->length == length)          /* does not need be splitted */
          return c;

     newchunk = (Chunk*) SHCALLOC( manager->shmpool, 1, sizeof(Chunk) );
     if (!newchunk) {
          D_OOSHM();
          return NULL;
     }

     /* calculate offsets and lengths of resulting chunks */
     newchunk->offset = c->offset + c->length - length;
     newchunk->length = length;
     newchunk->block_index = c->block_index;
     c->length -= newchunk->length;

     /* insert newchunk after chunk c */
     newchunk->prev = c;
     newchunk->next = c->next;
     if (c->next)
          c->next->prev = newchunk;
     c->next = newchunk;

     D_MAGIC_SET( newchunk, Chunk );

     return newchunk;
}

static Chunk *
free_chunk( SurfaceManager *manager, Chunk *chunk )
{
     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( chunk, Chunk );

     if (!chunk->buffer) {
          D_BUG( "freeing free chunk" );
          return chunk;
     }
     else {
          D_DEBUG_AT( SurfMan, "Deallocating %d bytes at offset %d.\n", chunk->length, chunk->offset );
     }

     //if (chunk->buffer->policy == CSP_VIDEOONLY)
          manager->avail += chunk->length;

     chunk->allocation = NULL;
     chunk->buffer     = NULL;

     manager->min_toleration--;

     if (chunk->prev  &&  !chunk->prev->buffer && (chunk->block_index==chunk->prev->block_index)) {
          Chunk *prev = chunk->prev;

          //D_DEBUG_AT( SurfMan, "  -> merging with previous chunk at %d\n", prev->offset );

          prev->length += chunk->length;

          prev->next = chunk->next;
          if (prev->next)
               prev->next->prev = prev;

          //D_DEBUG_AT( SurfMan, "  -> freeing %p (prev %p, next %p)\n", chunk, chunk->prev, chunk->next);

          D_MAGIC_CLEAR( chunk );

          SHFREE( manager->shmpool, chunk );
          chunk = prev;
     }

     if (chunk->next  &&  !chunk->next->buffer && (chunk->block_index==chunk->next->block_index)) {
          Chunk *next = chunk->next;

          //D_DEBUG_AT( SurfMan, "  -> merging with next chunk at %d\n", next->offset );

          chunk->length += next->length;

          chunk->next = next->next;
          if (chunk->next)
               chunk->next->prev = chunk;


          D_MAGIC_CLEAR( next );
          SHFREE( manager->shmpool, next );
     }

     return chunk;
}

static Chunk *
occupy_chunk( SurfaceManager *manager, Chunk *chunk, CoreSurfaceAllocation *allocation, int length, int pitch )
{
     D_MAGIC_ASSERT( manager, SurfaceManager );
     D_MAGIC_ASSERT( chunk, Chunk );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( allocation->buffer, CoreSurfaceBuffer );

     //if (allocation->buffer->policy == CSP_VIDEOONLY)
          //manager->avail -= length;

     chunk = split_chunk( manager, chunk, length );
     if (!chunk)
          return NULL;

     D_DEBUG_AT( SurfMan, "Allocating %d bytes at offset %d.\n", chunk->length, chunk->offset );

     chunk->allocation = allocation;
     chunk->buffer     = allocation->buffer;
     chunk->pitch      = pitch;

     manager->min_toleration++;
     manager->avail -= length;

     return chunk;
}

