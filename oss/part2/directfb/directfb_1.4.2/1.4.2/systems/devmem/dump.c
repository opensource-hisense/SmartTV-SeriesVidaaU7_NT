/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/clock.h>
#include <direct/debug.h>

#include <fusion/build.h>
#include <fusion/fusion.h>
#include <fusion/object.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/shm.h>
#include <fusion/shm/shm_internal.h>

#include <core/core.h>
#include <core/layer_control.h>
#include <core/layer_context.h>
#include <core/layers.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/windows.h>
#include <core/windowstack.h>
#include <core/windows_internal.h>
#include <core/wm.h>

static DirectFBPixelFormatNames( format_names );

/**********************************************************************************************************************/

typedef struct {
     int video;
     int system;
     int presys;
} MemoryUsage;

/**********************************************************************************************************************/


static MemoryUsage mem;

static bool show_shm;
static bool show_pools;
static bool show_allocs;
static int  dump_layer;       /* ref or -1 (all) or 0 (none) */
static int  dump_surface;     /* ref or -1 (all) or 0 (none) */


/**********************************************************************************************************************/
#if !USE_SIZE_OPTIMIZATION

static inline int
mgr_buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, bool video )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

     fusion_vector_foreach (allocation, i, buffer->allocs) {
          int size = allocation->size;
          if (allocation->flags & CSALF_ONEFORALL)
               size /= surface->num_buffers;
          if (video) {
               if (allocation->access[CSAID_GPU])
                    mem += size;
          }
          else if (!allocation->access[CSAID_GPU])
               mem += size;
     }

     return mem;
}

static int
mgr_buffer_sizes( CoreSurface *surface, bool video )
{
     int i, mem = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          mem += mgr_buffer_size( surface, buffer, video );
     }

     return mem;
}

static int
mgr_buffer_locks( CoreSurface *surface, bool video )
{
     int i, locks = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          locks += buffer->locked;
     }

     return locks;
}

static bool
mgr_surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult ret;
     int          i;
     int          refs;
     CoreSurface *surface = (CoreSurface*) object;
     MemoryUsage *mem     = ctx;
     int          vmem;
     int          smem;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

#if FUSION_BUILD_MULTI
     printf( "\n0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "\nN/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     vmem = mgr_buffer_sizes( surface, true );
     smem = mgr_buffer_sizes( surface, false );

     mem->video += vmem;

     /* FIXME: assumes all buffers have this flag (or none) */
     /*if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
          mem->presys += smem;
     else*/
          mem->system += smem;

     if (vmem && vmem < 1024)
          vmem = 1024;

     if (smem && smem < 1024)
          smem = 1024;

     printf( "%5dk%c  ", vmem >> 10, mgr_buffer_locks( surface, true ) ? '*' : ' ' );
     printf( "%5dk%c  ", smem >> 10, mgr_buffer_locks( surface, false ) ? '*' : ' ' );

     /* FIXME: assumes all buffers have this flag (or none) */
//     if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
//          printf( "preallocated " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

    /* if (surface->config.caps & DSCAPS_VIDEOHIGH)
          printf( "video preferred   " );*/

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied     " );

     if(surface->config.caps & DSCAPS_STATIC_ALLOC)
         printf("static_alloc");

     printf( "\n" );

     return true;
}

static void
mgr_dump_surfaces( void )
{
     printf( "\n"
             "\n-----------------------------[ Surfaces ]-------------------------------------\n" );
     printf( "\nReference   FID  . Refs  Width Height  Format     Video   System  Capabilities\n" );
     printf( "\n------------------------------------------------------------------------------\n" );
     memset(&mem, 0 , sizeof(mem));

     dfb_core_enum_surfaces( NULL, mgr_surface_callback, &mem );

     printf( "\n                                                ------   ------\n" );
     printf( "                                               %6dk  %6dk   -> %dk total\n",
             mem.video >> 10, (mem.system + mem.presys) >> 10,
             (mem.video + mem.system + mem.presys) >> 10);
}

/**********************************************************************************************************************/

static DFBEnumerationResult
mgr_alloc_callback( CoreSurfaceAllocation *alloc,
                void                  *ctx )
{
     int                i, index;
     CoreSurface       *surface;
     CoreSurfaceBuffer *buffer;
     FusionObject     *object;
     D_MAGIC_ASSERT( alloc, CoreSurfaceAllocation );

     buffer  = alloc->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     object = ( FusionObject     *)surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     *(int *)ctx+= alloc->size;
     //printf( "\n%9lu %8d  ", alloc->offset, alloc->size );
     printf("\n0x%08x-0x%08x   0x%08x   ",alloc->offset,alloc->offset+alloc->size,alloc->size);

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     index = dfb_surface_buffer_index( alloc->buffer );

     printf( " %-5s ",
             (dfb_surface_get_buffer( surface, CSBR_FRONT ) == buffer) ? "front" :
             (dfb_surface_get_buffer( surface, CSBR_BACK  ) == buffer) ? "back"  :
             (dfb_surface_get_buffer( surface, CSBR_IDLE  ) == buffer) ? "idle"  : "" );

     printf( direct_serial_check(&alloc->serial, &buffer->serial) ? " * " : "   " );

     printf( "%d  %2lu  ", fusion_vector_size( &buffer->allocs ), surface->resource_id );

     if (surface->type & CSTF_SHARED)
          printf( "SHARED  " );
     else
          printf( "PRIVATE " );

     if (surface->type & CSTF_LAYER)
          printf( "LAYER " );

     if (surface->type & CSTF_WINDOW)
          printf( "WINDOW " );

     if (surface->type & CSTF_CURSOR)
          printf( "CURSOR " );

     if (surface->type & CSTF_FONT)
          printf( "FONT " );

     printf( " " );

     if (surface->type & CSTF_INTERNAL)
          printf( "INTERNAL " );

     if (surface->type & CSTF_EXTERNAL)
          printf( "EXTERNAL " );

     printf( " " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

    /* if (surface->config.caps & DSCAPS_VIDEOHIGH)
         printf( "video perferred   " );*/

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied    " );

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif


     printf( "\n" );

     return DFENUM_OK;
}

static DFBEnumerationResult
mgr_surface_pool_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int length;
     int totalMem = 0;

     printf( "\n" );
     printf( "\n--------------------[ Surface Buffer Allocations in %s ]-------------------%n\n", pool->desc.name, &length );
     printf( "\nRange                   Length       Width Height     Format  Role  Up nA ID  Usage   Type / Storage / Caps\n" );

     while (length--) {
         printf("-");
     }

     printf( "\n" );

     dfb_surface_pool_enumerate( pool, mgr_alloc_callback, &totalMem );
     printf("\n                        --------");
     printf("\n                        %dk\n",totalMem/1024);
     return DFENUM_OK;
}

static void
mgr_dump_surface_pools( void )
{
     dfb_surface_pools_enumerate( mgr_surface_pool_callback, NULL );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
mgr_surface_pool_info_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int                    i;
     unsigned long          total = 0;
     CoreSurfaceAllocation *alloc;

     //printf("\n%s====\n",__FUNCTION__);
     //printf("\n pool->allocs count is %d pool->allocs elements is %x  pool is %x pool->allocs.magic:%d pool->allocs:%x\n",pool->allocs.count,pool->allocs.elements,pool,pool->allocs.magic,&pool->allocs);

     fusion_vector_foreach (alloc, i, pool->allocs)
          total += alloc->size;

      printf( "\n%-20s ", pool->desc.name );

     switch (pool->desc.priority) {
          case CSPP_DEFAULT:
               printf( "DEFAULT  " );
               break;

          case CSPP_PREFERED:
               printf( "PREFERED " );
               break;

          case CSPP_ULTIMATE:
               printf( "ULTIMATE " );
               break;

          default:
               printf( "unknown  " );
               break;
     }

     printf( "%6lu/%6luk  ", total / 1024, pool->desc.size / 1024 );

     if (pool->desc.types & CSTF_SHARED)
          printf( "* " );
     else
          printf( "  " );


     if (pool->desc.types & CSTF_INTERNAL)
          printf( "INT " );

     if (pool->desc.types & CSTF_EXTERNAL)
          printf( "EXT " );

     if (!(pool->desc.types & (CSTF_INTERNAL | CSTF_EXTERNAL)))
          printf( "    " );


     if (pool->desc.types & CSTF_LAYER)
          printf( "LAYER " );
     else
          printf( "      " );

     if (pool->desc.types & CSTF_WINDOW)
          printf( "WINDOW " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_CURSOR)
          printf( "CURSOR " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_FONT)
          printf( "FONT " );
     else
          printf( "     " );


     for (i=CSAID_CPU; i<=CSAID_GPU; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     for (i=CSAID_LAYER0; i<=CSAID_LAYER2; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     printf( "\n" );


     return DFENUM_OK;
}

static void
dump_surface_pool_info( void )
{
     printf( "\n" );
     printf( "\n-------------------------------------[ Surface Buffer Pools ]------------------------------------\n" );
     printf( "\nName                 Priority   Used/Capacity S I/E Resource Type Support     CPU GPU Layer 0 - 2\n" );
     printf( "\n-------------------------------------------------------------------------------------------------\n" );

     dfb_surface_pools_enumerate( mgr_surface_pool_info_callback, NULL );
}

/**********************************************************************************************************************/

static bool
mgr_context_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult       ret;
     int                i;
     int                refs;
     int                level;
     CoreLayer         *layer   = (CoreLayer*) ctx;
     CoreLayerContext  *context = (CoreLayerContext*) object;
     CoreLayerRegion   *region  = NULL;
     CoreSurface       *surface = NULL;

     if (object->state != FOS_ACTIVE)
          return true;

     if (context->layer_id != dfb_layer_id( layer ))
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

     dfb_layer_context_get_primary_region( context, false, &region ) ;
     printf("layer region flag %08x ", region->config.options);
     if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK) {
            printf(" region surface id %08x ", surface->object.ref.multi.id);
            dfb_surface_unref( surface );
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d  ", context->config.width, context->config.height );

     for (i=0; format_names[i].format; i++) {
          if (context->config.pixelformat == format_names[i].format) {
               printf( "%-8s ", format_names[i].name );
               break;
          }
     }

     if (!format_names[i].format)
          printf( "unknown  " );

     printf( "%.1f, %.1f -> %.1f, %.1f   ",
             context->screen.location.x,  context->screen.location.y,
             context->screen.location.x + context->screen.location.w,
             context->screen.location.y + context->screen.location.h );

     printf( "%2d     ", fusion_vector_size( &context->regions ) );

     printf( context->active ? "(*)    " : "       " );

     if (context == layer->shared->contexts.primary)
          printf( "SHARED   " );
     else
          printf( "PRIVATE  " );

     if (context->rotation)
          printf( "ROTATED %d ", context->rotation);

     if (dfb_layer_get_level( layer, &level ))
          printf( "N/A" );
     else
          printf( "%3d", level );

     printf( "\n" );

     return true;
}

static void
mgr_dump_contexts( CoreLayer *layer )
{
     if (fusion_vector_size( &layer->shared->contexts.stack ) == 0)
          return;

     printf( "\n"
             "\n----------------------------------[ Contexts of Layer %d ]----------------------------------------\n", dfb_layer_id( layer ));
     printf( "\nReference   FID  . Refs  Width Height Format   Location on screen  Regions  Active  Info    Level\n" );
     printf( "\n-------------------------------------------------------------------------------------------------\n" );

     dfb_core_enum_layer_contexts( NULL, mgr_context_callback, layer );
}

static DFBEnumerationResult
mgr_window_callback( CoreWindow *window,
                 void       *ctx )
{
     DirectResult      ret;
     int               refs;
     CoreWindowConfig *config = &window->config;
     DFBRectangle     *bounds = &config->bounds;

     ret = fusion_ref_stat( &window->object.ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return DFENUM_OK;
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", window->object.ref.multi.id, window->object.ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d, %4d   ", bounds->x, bounds->y );

     printf( "%4d x %4d    ", bounds->w, bounds->h );

     printf( "0x%02x ", config->opacity );

     printf( "%5d  ", window->id );

     switch (config->stacking) {
          case DWSC_UPPER:
               printf( "^  " );
               break;
          case DWSC_MIDDLE:
               printf( "-  " );
               break;
          case DWSC_LOWER:
               printf( "v  " );
               break;
          default:
               printf( "?  " );
               break;
     }

     if (window->caps & DWCAPS_ALPHACHANNEL)
          printf( "alphachannel   " );

     if (window->caps & DWCAPS_INPUTONLY)
          printf( "input only     " );

     if (window->caps & DWCAPS_DOUBLEBUFFER)
          printf( "double buffer  " );

     if (config->options & DWOP_GHOST)
          printf( "GHOST          " );

     if (DFB_WINDOW_FOCUSED( window ))
          printf( "FOCUSED        " );

     if (DFB_WINDOW_DESTROYED( window ))
          printf( "DESTROYED      " );

     if (window->config.rotation)
          printf( "ROTATED %d     ", window->config.rotation);

     printf( "\n" );

     return DFENUM_OK;
}

static void
mgr_dump_windows( CoreLayer *layer )
{
     DFBResult         ret;
     CoreLayerShared  *shared;
     CoreLayerContext *context;
     CoreWindowStack  *stack;

     shared = layer->shared;

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "DirectFB/Dump: Could not lock the shared layer data!\n" );
          return;
     }

     context = layer->shared->contexts.primary;
     if (!context) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     stack = dfb_layer_context_windowstack( context );
     if (!stack) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     dfb_windowstack_lock( stack );

     if (stack->num) {
          printf( "\n"
                  "\n-----------------------------------[ Windows of Layer %d ]-----------------------------------------\n", dfb_layer_id( layer ) );
          printf( "\nReference   FID  . Refs     X     Y   Width Height Opacity   ID     Capabilities   State & Options\n" );
          printf( "\n--------------------------------------------------------------------------------------------------\n" );

          dfb_wm_enum_windows( stack, mgr_window_callback, NULL );
     }

     dfb_windowstack_unlock( stack );

     fusion_skirmish_dismiss( &shared->lock );
}

static DFBEnumerationResult
mgr_layer_callback( CoreLayer *layer,
                void      *ctx)
{
     mgr_dump_windows( layer );
     mgr_dump_contexts( layer );

     return DFENUM_OK;
}

static void
mgr_dump_layers( void )
{
     dfb_layers_enumerate( mgr_layer_callback, NULL );
}

/**********************************************************************************************************************/

#if FUSION_BUILD_MULTI
static DirectEnumerationResult
mgr_dump_shmpool( FusionSHMPool *pool,
              void          *ctx )
{
     DFBResult     ret;
     SHMemDesc    *desc;
     unsigned int  total = 0;
     int           length;
     FusionSHMPoolShared *shared = pool->shared;

     printf( "\n" );
     printf( "\n----------------------------[ Shared Memory in %s ]----------------------------%n\n", shared->name, &length );
     printf( "\n      Size          Address      Offset      Function                     FusionID\n" );

     while (length--) {
         printf("-");
     }

     printf("\n");

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "Could not lock shared memory pool!\n" );
          return DFENUM_OK;
     }

     if (shared->allocs) {
          direct_list_foreach (desc, shared->allocs) {
               printf( " %9zu bytes at %p [%8lu] in %-30s [%3lx] (%s: %u)\n",
                       desc->bytes, desc->mem, (ulong)desc->mem - (ulong)shared->heap,
                       desc->func, desc->fid, desc->file, desc->line );

               total += desc->bytes;
          }

          printf( "   -------\n  %7dk total\n", total >> 10 );
     }

     printf( "\nShared memory file size: %dk\n", shared->heap->size >> 10 );

     fusion_skirmish_dismiss( &shared->lock );

     return DFENUM_OK;
}

static void
mgr_dump_shmpools( void )
{
     fusion_shm_enum_pools( dfb_core_world(NULL), mgr_dump_shmpool, NULL );
}
#endif

/**********************************************************************************************************************/
void
mgr_mgr_dump()
{

     mgr_dump_surfaces();
     mgr_dump_surface_pools();
     mgr_dump_layers();

}

#else // flag of USE_SIZE_OPTIMIZATION

#define PRINT_FUNC_LINE() \
        printf("[DFB] dummy function : %s, %s, %d \n", __FILE__, __FUNCTION__, __LINE__ );

void
mgr_mgr_dump()
{
    PRINT_FUNC_LINE();
}

#endif // end of USE_SIZE_OPTIMIZATION
