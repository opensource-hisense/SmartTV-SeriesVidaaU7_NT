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
#include <dlfcn.h>

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
#include <core/system.h>

#include <misc/util.h>

static DirectFBPixelFormatNames( format_names );

#define CMD_WIN_RAISETOTOP   "-raisetotop"
#define CMD_WIN_LOWTOBOTTOM  "-lowtobottom"
#define CMD_SET_OPACITY "-set-opacity"
#define CMD_LAYER       "-layer"
#define CMD_WIN         "-win"
#define CMD_SETLEVEL    "-setlevel"
#define CMD_WIDTH       "-width"
#define CMD_HEIGHT      "-height"
#define CMD_BUFFERMODE      "-buffermode"
#define CMD_FULLUPDATE  "-fullupdate"
#define CMD_CAPS        "-caps"
#define PRINT_WINDOW_OPTION(OP, FLAG) if (OP & DWOP_##FLAG) printf(" %s ", #FLAG)
#define PRINT_WINDOW_CAPS(OP, FLAG)   if (OP & DWCAPS_##FLAG) printf(" %s ", #FLAG)
#define SINGLE_BUFFER 1
#define DOUBLE_BUFFER 2
#define TRIPLE_BUFFER 3
/**********************************************************************************************************************/
typedef struct _SufaceDumpInfo
{
    struct MemoryUsage{
         int video;
         int system;
         int presys;
    } memory ;

    bool bCall2nd;
}SufaceDumpInfo;

/**********************************************************************************************************************/


static bool show_shm = false;
static bool show_pools = false;
static bool show_allocs = false;
static int  dump_layer = 0;       /* ref or -1 (all) or 0 (none) */
static int  dump_surface = 0;     /* ref or -1 (all) or 0 (none) */

int dumpsurface_upperbound = 0;

DFB_DUMPQUERYINFO dumpQueryInfo;
bool bInitQuery = false;

struct surface_list
{
    int val;
    struct surface_list *next;
};
struct surface_list *surface_list_head = 0;


/***** SHOW / HIDE Window *****/
static int  show_win                  = -1;  //init
static int  show_which_layer          = -1;   //init
static int  show_which_win            = -1;   //init
static int  show_which_win_upperbound = -1;   //init

static int change_win_order = 0;

/***** layer level *****/
static int level_layer = 1;
static int layer_width = 0;
static int layer_height = 0;

static int layer_buffermode = 0;
          
static int fullupdate_layer = -1;
static bool bfullupdate = false;


static bool bCall_fullupdate = false;
static bool bGetCAPS = false;
/**********************************************************************************************************************/

static DFBBoolean parse_command_line( int argc, char *argv[] );

/**********************************************************************************************************************/

static inline int
buffer_size( CoreSurface                *surface,
             CoreSurfaceBuffer          *buffer,
             bool                        video )
{
    int                    i, mem = 0;
    CoreSurfaceAllocation *allocation;

    fusion_vector_foreach (allocation, i, buffer->allocs)
    {
        int size = allocation->size;

        if (allocation->flags & CSALF_ONEFORALL)
            size /= surface->num_buffers;

        if (video)
        {
            if (allocation->access[CSAID_GPU])
                mem += size;
        }
        else if (!allocation->access[CSAID_GPU])
            mem += size;
    }

    return mem;
}

static int
buffer_sizes( CoreSurface               *surface,
              bool                       video )
{
    int i, mem = 0;

    for (i = 0; i < surface->num_buffers; i++)
    {
        CoreSurfaceBuffer *buffer = surface->buffers[i];

        mem += buffer_size( surface, buffer, video );
    }

    return mem;
}

static int
buffer_locks( CoreSurface               *surface,
              bool                       video )
{
    int i, locks = 0;

    for (i = 0; i < surface->num_buffers; i++)
    {
        CoreSurfaceBuffer *buffer = surface->buffers[i];

        locks += buffer->locked;
    }

    return locks;
}

static bool
surface_callback( FusionObjectPool      *pool,
                  FusionObject          *object,
                  void                  *ctx )
{
    DirectResult ret;
    int          i;
    int          refs;
    CoreSurface *surface = (CoreSurface*) object;
    SufaceDumpInfo *dumpsurface     = ctx;
    int          vmem = 0;
    int          smem = 0;
     
    if (object->state != FOS_ACTIVE)
        return true;

    ret = fusion_ref_stat( &object->ref, &refs );
    if (ret)
    {
        printf( "Fusion error %d!\n", ret );
        return false;
    }


    vmem = buffer_sizes( surface, true );
    smem = buffer_sizes( surface, false );

    dumpsurface->memory.video += vmem;

    /* FIXME: assumes all buffers have this flag (or none) */
    /*if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
          mem->presys += smem;
    else*/
    dumpsurface->memory.system += smem;

    if (vmem && vmem < 1024)
        vmem = 1024;

    if (smem && smem < 1024)
        smem = 1024;

    //dump -ds A,B,C,...
    struct surface_list* temp = surface_list_head;
    struct surface_list* prev = temp;
    while(temp)
    {
        if(temp->val ==  object->ref.multi.id)
        {
            //raise flag
            dumpsurface_upperbound = temp->val;
            dump_surface = temp->val;
            //free temp
            if(temp == surface_list_head)
                surface_list_head = temp->next;
            prev->next = temp->next;
            free(temp);

            break;
        }
        else
        {
            prev = temp;
            temp = temp->next;
        }
    }


    int PID = 0;
    int PPID = 0;

    bool bGetPID = false;
    bool bGetPPID = false;

    if (bInitQuery)
    {
        DumpInfoType tmpForPID = {object, &PID};
        bGetPID = dfb_dumpQueryInfo(DF_DUMP_GET_PROCESS_ID, &tmpForPID);

        DumpInfoType tmpForPPID = {object, &PPID};
        bGetPPID = dfb_dumpQueryInfo(DF_DUMP_GET_PARENT_PROCESS_ID, &tmpForPPID);
    }



    bool isDumpAll = (dump_surface < 0)                                 &&
                     (surface->type & (CSTF_SHARED | CSTF_EXTERNAL));


    bool isDumpBetween = (dump_surface <= object->ref.multi.id)         &&
                         (dumpsurface_upperbound >= object->ref.multi.id);



    if (dump_surface                    &&
        (isDumpAll || isDumpBetween )   &&
        surface->num_buffers            &&
        (smem == 0))
    {
        char buf[64];
        char PIDbuf[64]={0};

        if (bGetPID)
            snprintf( PIDbuf, sizeof(PIDbuf), "pid%d_",  PID);

        snprintf( buf, sizeof(buf), "dfb_surface_0x%08x", object->ref.multi.id );

        switch(surface->num_buffers)
        {
            default:

            case 3:
                snprintf( buf, sizeof(buf), "%sdfb_surface_idle_0x%08x", PIDbuf, object->ref.multi.id );
                dfb_surface_dump_buffer( surface, CSBR_IDLE, ".", buf );

            case 2:
                snprintf( buf, sizeof(buf), "%sdfb_surface_back_0x%08x", PIDbuf, object->ref.multi.id );
                dfb_surface_dump_buffer( surface, CSBR_BACK, ".", buf );

            case 1:
                snprintf( buf, sizeof(buf), "%sdfb_surface_front_0x%08x", PIDbuf, object->ref.multi.id );
                dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
                break;
         }
    }


#if FUSION_BUILD_MULTI
    if (bInitQuery && bGetPID && bGetPPID)
        printf( "\n0x%08x [%5d][%4d] : ", object->ref.multi.id, PID, PPID );
    else
        printf( "\n0x%08x [%5s][%3lx] : ", object->ref.multi.id, "----", object->ref.multi.creator );

#else
    printf( "\nN/A              : " );

#endif
    if (!dumpsurface->bCall2nd)
    {
        printf( "%3d   ", refs );

        printf( "%6d x %6d   ", surface->config.size.w, surface->config.size.h );

        printf( "%5dk%c  ", vmem >> 10, buffer_locks( surface, true ) ? '*' : ' ' );
        printf( "%5dk%c  ", smem >> 10, buffer_locks( surface, false ) ? '*' : ' ' );

        for (i=0; format_names[i].format; i++)
        {
            if (surface->config.format == format_names[i].format)

            printf( "%8s ", format_names[i].name );
        }

        if (surface->config.caps & DSCAPS_SYSTEMONLY)
            printf( "  system only" );

        else if (surface->config.caps & DSCAPS_VIDEOONLY)
            printf( "  video only " );

        else if (surface->config.caps & DSCAPS_VIDEOHIGH)
            printf( "  video high " );

        else
            printf( "             " );

        if (surface->config.caps & DSCAPS_DOUBLE)
            printf( "  double     " );

        if (surface->config.caps & DSCAPS_TRIPLE)
            printf( "  triple     " );
    }

     /* FIXME: assumes all buffers have this flag (or none) */
//     if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
//          printf( "preallocated " );
    if (dumpsurface->bCall2nd)
    {

        if (surface->config.caps & DSCAPS_INTERLACED)
            printf( " interlaced   " );

        if (surface->config.caps & DSCAPS_PREMULTIPLIED)
            printf( " premultiplied" );

        if (surface->config.caps & DSCAPS_STATIC_ALLOC)
            printf(" static_alloc  ");

        if (surface->config.caps & DSCAPS_PREALLOCATED)
            printf(" preallocated  ");

        if (surface->config.caps & DSCAPS_PRIMARY)
            printf(" primary       " );

        if (surface->config.caps & DSCAPS_SHARED)
            printf(" shared        " );

        if (surface->config.caps & DSCAPS_SUBSURFACE)
            printf(" subsurface    " );

        if (surface->config.caps & DSCAPS_SEPARATED)
            printf(" separated     " );

        if (surface->config.caps & DSCAPS_STATIC_POS)
            printf(" static_pos    " );

        if (surface->config.caps & DSCAPS_DEPTH)
            printf(" depth         " );

        if (surface->config.caps & DSCAPS_ROTATED)
            printf(" rotated       " );

        if (( surface->config.caps & DSCAPS_DOUBLE)      &&
           (  surface->config.caps & DSCAPS_TRIPLE))
            printf(" flipping      " );

        if (surface->config.caps == NULL )
            printf("               " );
    }
     
    printf( "\n" );

    return true;
}



static void
dump_surfaces( void )
{
    SufaceDumpInfo dumpsurface;

    memset(&dumpsurface, 0, sizeof(dumpsurface));

    printf( "\n"
            "\n-----------------------------[ Surfaces ]----------------------------------------------------------\n" );

    if (bInitQuery)
    printf( "\nReference    PID   PPID . Refs    Width  Height    Video    System    Format    Memory       Buffer\n" );
    else
    printf( "\nReference   FID  . Refs  Width Height    Video    System    Format   Capabilities\n" );

    printf( "\n---------------------------------------------------------------------------------------------------\n" );

    dfb_core_enum_surfaces( NULL, surface_callback, &dumpsurface );
     
    printf( "\n                                                   ------   ------\n" );
    printf( "                                                  %6dk  %6dk   -> %dk total\n",
            dumpsurface.memory.video >> 10, (dumpsurface.memory.system + dumpsurface.memory.presys) >> 10,
            (dumpsurface.memory.video + dumpsurface.memory.system + dumpsurface.memory.presys) >> 10);
    printf( "                                                  %6dk video available   -> %6dk video total\n", dfb_system_video_memory_available(NULL) >> 10, (dfb_config->video_length + dfb_config->video_length_secondary) >> 10 );


    if(bGetCAPS)
    {
        dumpsurface.bCall2nd=true;
        printf( "\n"
                "\n-----------------------------[ Surfaces caps info ]---------------------------\n" );

        if (bInitQuery)
            printf( "\nReference    PID   PPID .  Capabilities\n" );

        printf( "\n------------------------------------------------------------------------------\n" );

        dfb_core_enum_surfaces( NULL, surface_callback, &dumpsurface );
        dumpsurface.bCall2nd=false;
    }
}

/**********************************************************************************************************************/

static DFBEnumerationResult
alloc_callback( CoreSurfaceAllocation   *alloc,
                void                    *ctx )
{
     int                i, index;

     CoreSurface        *surface;
     CoreSurfaceBuffer  *buffer;
     FusionObject       *object;

     D_MAGIC_ASSERT( alloc, CoreSurfaceAllocation );

     buffer = alloc->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     object = (FusionObject*)surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     *(int *)ctx += alloc->size;
     //printf( "\n%9lu %8d  ", alloc->offset, alloc->size );
     printf( "\n0x%08x   ", object->ref.multi.id );
     printf("0x%08x-0x%08x   0x%08x   ", alloc->offset, alloc->offset+alloc->size, alloc->size);

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i = 0; format_names[i].format; i++)
     {
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
          printf( " SHARED  " );
     else
          printf( " PRIVATE " );

     if (surface->type & CSTF_LAYER)
          printf( " LAYER " );

     if (surface->type & CSTF_WINDOW)
          printf( " WINDOW " );

     if (surface->type & CSTF_CURSOR)
          printf( " CURSOR " );

     if (surface->type & CSTF_FONT)
          printf( " FONT " );


     if (surface->type & CSTF_INTERNAL)
          printf( " INTERNAL " );

     if (surface->type & CSTF_EXTERNAL)
          printf( " EXTERNAL " );

/*#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif
*/

     printf( "\n" );

     return DFENUM_OK;
}

static DFBEnumerationResult
surface_pool_callback( CoreSurfacePool  *pool,
                       void             *ctx )
{
     int length;
     int totalMem = 0;

     printf( "\n" );
     printf( "\n--------------------[ Surface Buffer Allocations in %s ]----------------------------%n\n", pool->desc.name, &length );
     printf( "\nReference    Range                   Length       Width Height     Format  Role  Up nA ID  Type\n" );

     while (length--) {
         printf("-");
     }

     printf( "\n" );

     dfb_surface_pool_enumerate( pool, alloc_callback, &totalMem );
     printf("\n                                     --------");
     printf("\n                                     %dk\n",totalMem/1024);

     return DFENUM_OK;
}

static void
dump_surface_pools( void )
{
     dfb_surface_pools_enumerate( surface_pool_callback, NULL );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
surface_pool_info_callback( CoreSurfacePool *pool,
                            void            *ctx )
{
    int                    i;
    unsigned long          total = 0;
    CoreSurfaceAllocation *alloc = NULL;

    //printf("\n%s====\n",__FUNCTION__);
    //printf("\n pool->allocs count is %d pool->allocs elements is %x  pool is %x pool->allocs.magic:%d pool->allocs:%x\n",pool->allocs.count,pool->allocs.elements,pool,pool->allocs.magic,&pool->allocs);

    fusion_vector_foreach (alloc, i, pool->allocs)
        total += alloc->size;

    printf( "\n%-28s ", pool->desc.name );

    switch (pool->desc.priority)
    {
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


    for (i = CSAID_CPU; i <= CSAID_GPU; i++) {
        printf( " %c%c%c",
                (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
    }

    for (i = CSAID_LAYER0; i <= CSAID_LAYER2; i++) {
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
    printf( "\n-------------------------------------[ Surface Buffer Pools ]--------------------------------------------\n" );
    printf( "\nName                         Priority   Used/Capacity S I/E Resource Type Support     CPU GPU Layer 0 - 2\n" );
    printf( "\n----------------------------------------------------------------------------------------------------------\n" );

    dfb_surface_pools_enumerate( surface_pool_info_callback, NULL );
    printf( "\n\n" );
}

/**********************************************************************************************************************/

static bool context_callback( FusionObjectPool *pool, FusionObject *object,  void *ctx )
{
    DirectResult  ret;
    int  i = 0;
    int  refs = 0;
    int  level = 0;
    CoreLayer         *layer   = (CoreLayer*) ctx;
    CoreLayerContext  *context = (CoreLayerContext*) object;
    CoreLayerRegion   *region  = NULL;
    CoreSurface       *surface = NULL;
    CoreWindowStack  *stack = NULL;


    if(object == NULL || layer == NULL || context == NULL)
        return false;

    if (object->state != FOS_ACTIVE)
        return true;

    if (context->layer_id != dfb_layer_id( layer ))
        return true;

    ret = fusion_ref_stat( &object->ref, &refs );

    if (ret)
    {
        printf( "Fusion error %d!\n", ret );
        return false;
    }


    dfb_layer_context_get_primary_region( context, false, &region ) ;

    //printf("layer region flag %08x ", region->config.options);
    if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK)
    {
        printf("0x%08x  ", surface->object.ref.multi.id);
        dfb_surface_unref( surface );
    }

    if (dump_layer && (dump_layer < 0 || dump_layer == object->ref.multi.id))
    {
        if (dfb_layer_context_get_primary_region( context, false, &region ) == DFB_OK)
        {
            if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK)
            {
                if (surface->num_buffers)
                {
                    char buf[32];

                    snprintf( buf, sizeof(buf), "dfb_layer_context_0x%08x", object->ref.multi.id );

                    dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
                }

                    dfb_surface_unref( surface );
            }
        }
    }

#if FUSION_BUILD_MULTI
    printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );

#else
    printf( "N/A              : " );

#endif
    printf( "%3d   ", refs );

    printf( "%5d x %5d", context->config.width, context->config.height );

    for (i = 0; format_names[i].format; i++)
    {
        if (context->config.pixelformat == format_names[i].format) {
            printf( "%8s  ", format_names[i].name );
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

    printf( context->active ? "(*)   " : "      " );

    if (context == layer->shared->contexts.primary)
        printf( "SHARED   " );
    else
        printf( "PRIVATE  " );

    if (context->rotation)
        printf( "ROTATED %d ", context->rotation);

    if (dfb_layer_get_level( layer, &level ))
        printf( " N/A " );
    else
        printf( "%3d ", level );

    stack = dfb_layer_context_windowstack( context );
    if (stack)
        printf(" %d ", stack->bforce_fullupdate);
    else
        printf(" N/A ");

    printf( "\n\n");

    return true;
}

static void
dump_contexts( CoreLayer                *layer )
{
    if (fusion_vector_size( &layer->shared->contexts.stack ) == 0)
        return;

    printf( "\n"
            "\n----------------------------------[ Contexts of Layer %d ]---------------------------------------------------------------\n", dfb_layer_id( layer ));
    printf( "\nSurface-ID  Reference   FID  .  Refs  Width Height  Format   Location on screen  Regions  Active  Info  Level  Fullupdate\n" );
    printf( "\n-------------------------------------------------------------------------------------------------------------------------\n" );

    dfb_core_enum_layer_contexts( NULL, context_callback, layer );
}

static void print_window_options (int op)
{
    PRINT_WINDOW_OPTION(op, COLORKEYING);
    PRINT_WINDOW_OPTION(op, OPAQUE_REGION);
    PRINT_WINDOW_OPTION(op, SHAPED);
    PRINT_WINDOW_OPTION(op, KEEP_POSITION);
    PRINT_WINDOW_OPTION(op, KEEP_SIZE);
    PRINT_WINDOW_OPTION(op, KEEP_STACKING);
    PRINT_WINDOW_OPTION(op, GHOST);
    PRINT_WINDOW_OPTION(op, INDESTRUCTIBLE);
    PRINT_WINDOW_OPTION(op, SCALE);
    PRINT_WINDOW_OPTION(op, SCALE_WINDOW_AUTOFIT_LAYER);
    PRINT_WINDOW_OPTION(op, KEEP_ABOVE);
    PRINT_WINDOW_OPTION(op, KEEP_UNDER);
    PRINT_WINDOW_OPTION(op, FOLLOW_BOUNDS);
    PRINT_WINDOW_OPTION(op, MULTI_VIEW);
}


static void print_window_caps (int op)
{
    PRINT_WINDOW_CAPS(op, ALPHACHANNEL);
    PRINT_WINDOW_CAPS(op, DOUBLEBUFFER);
    PRINT_WINDOW_CAPS(op, NODECORATION);
    PRINT_WINDOW_CAPS(op, SUBWINDOW);
    PRINT_WINDOW_CAPS(op, COLOR);
    PRINT_WINDOW_CAPS(op, SUBWINDOW);
    PRINT_WINDOW_CAPS(op, INPUTONLY);
}

static DFBEnumerationResult
window_callback( CoreWindow             *window,
                 void                   *ctx )
{
    DirectResult      ret;
    int               refs;
    CoreWindowConfig *config = &window->config;
    DFBRectangle     *bounds = &config->bounds;

    ret = fusion_ref_stat( &window->object.ref, &refs );
    if (ret)
    {
        printf( "Fusion error %d!\n", ret );

        return DFENUM_OK;
    }

#if FUSION_BUILD_MULTI

    printf( "0x%08x [%3lx] : ", window->object.ref.multi.id, window->object.ref.multi.creator );
#else

    printf( "N/A              : " );
#endif

    printf( "%3d   ", refs );

    //calculate Surface-ID
    int Surface_ID= -1;  //init
    int next;

    Surface_ID = window->surface->object.ref.multi.id;
    printf( "0x%08x ", Surface_ID);

    printf( "  %4d, %4d   ", bounds->x, bounds->y );

    printf( "%5d x %5d    ", bounds->w, bounds->h );

    printf( "0x%02x ", config->opacity );

    if (DFB_WINDOW_FOCUSED( window ))
        printf( "    v" );
    else
        printf( "     " );

    printf( "%5d  ", window->id );

    switch (config->stacking)
    {
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
    print_window_caps (window->caps);

    print_window_options (config->options);

    if (DFB_WINDOW_DESTROYED( window ))
        printf( " DESTROYED" );

    if (window->config.rotation)
        printf( " ROTATED %d", window->config.rotation);

    printf( "\r\n" );  //allignment
     
    return DFENUM_OK;
}

static void
dump_windows( CoreLayer                 *layer )
{
    DFBResult         ret;
    CoreLayerShared  *shared;
    CoreLayerContext *context;
    CoreWindowStack  *stack;

    shared = layer->shared;

    ret = fusion_skirmish_prevail( &shared->lock );
    if (ret)
    {
        D_DERROR( ret, "DirectFB/Dump: Could not lock the shared layer data!\n" );
        return;
    }

    context = layer->shared->contexts.primary;
    if (!context)
    {
        fusion_skirmish_dismiss( &shared->lock );
        return;
    }

    stack = dfb_layer_context_windowstack( context );
    if (!stack)
    {
        fusion_skirmish_dismiss( &shared->lock );
        return;
    }


    dfb_windowstack_lock( stack );

    if (stack->num)
    {
        printf( "\n"
                "\n-----------------------------------[ Windows of Layer %d ]----------------------------------------------------------------\n", dfb_layer_id( layer ) );
        printf( "\nReference   FID  . Refs  Surface-ID      X     Y   Width Height  Opacity  Focus  ID      Caps & Opts\n" );
        printf( "\n--------------------------------------------------------------------------------------------------------------------------\n" );

        dfb_wm_enum_windows( stack, window_callback, NULL );
    }

    dfb_windowstack_unlock( stack );

    fusion_skirmish_dismiss( &shared->lock );
}

static DFBEnumerationResult
layer_callback( CoreLayer               *layer,
                void                    *ctx)
{
    dump_windows( layer );
    dump_contexts( layer );

    return DFENUM_OK;
}

static void
dump_layers( void )
{
    dfb_layers_enumerate( layer_callback, NULL );
}

bool dump_init_query_info()
{
    void *query_handle = NULL;
	const char* query_err;
    
    query_handle = dlopen ("libdirectfb.so", RTLD_LAZY);

    if (!query_handle)
    {
        printf("\33[0;34;43m[error] %s, %d  dlopen fail :%s\33[0m\n", __FUNCTION__, __LINE__,dlerror() );
        return false;
    }
    
    dumpQueryInfo = (DFB_DUMPQUERYINFO) dlsym (query_handle, "dfb_dumpQueryInfo");
    if (query_err = dlerror())
    {
        printf("\33[0;34;43m[error] %s, %d \33[0m\n", __FUNCTION__, __LINE__ );
        dlclose(query_handle);
        return false;
    }
    
    dlclose(query_handle);
    
    return true;
}

void dump_max_mem_usage(IDirectFB *dfb)
{
    if(dfb == NULL)
      return;

    int maxPeakMem=0;
    DumpInfoType tmp = {dfb, &maxPeakMem};

    dfb_dumpQueryInfo(DF_DUMP_MAXMEM, &tmp);

    printf( "\n"
            "\n--------------------------------[ Memory ]-------------------------------------\n" );
    printf( "                            ");
    printf( "\33[0;33mthe system : max peak memory usage = %dk \33[0m\n\n", maxPeakMem>>10);
    printf( "\33[0;32m*******************************************************************************\33[0m\n");
    printf( "\33[0;32m        if you want to know max peak mem usage of every process \33[0m\n");
    printf( "\33[0;32m        plz add mst_mem_peak_usage=MAX_PROCESS_ID in the directfbrc\33[0m\n");
    printf( "\33[0;32m        will list max peak mem usage info of all process [0, MAX_PROCESS_ID] \33[0m\n");
    printf( "\33[0;32m        ex. mst_mem_peak_usage=10000 \33[0m\n");
    printf( "\33[0;32m            list 0~9999 process : max peak mem usage; over 10000, be ignored \33[0m\n");
    printf( "\n--------------------------------------------------------------------------------\n" );
    
}

void dump_process_mem_usage(IDirectFB *dfb)
{
    if(dfb == NULL)
      return;

    DFBPeakMemInfo *PIDMem;

    DumpInfoType tmp = {dfb, (void*)&PIDMem};
    int i = 0;

    dfb_dumpQueryInfo(DF_DUMP_GET_PROCESS_MEM_INFO, &tmp);
}

/**********************************************************************************************************************/

#if FUSION_BUILD_MULTI
static DirectEnumerationResult
dump_shmpool( FusionSHMPool             *pool,
              void                      *ctx )
{
    DFBResult     ret;
    SHMemDesc    *desc = NULL;
    unsigned int  total = 0;
    int           length = 0;
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

    if (shared->allocs)
    {
        direct_list_foreach (desc, shared->allocs)
        {
            printf( " %9zu bytes at %p [%8lu] in %-30s [%3lx] (%s: %u)\n",
                        desc->bytes, desc->mem, (ulong)desc->mem - (ulong)shared->heap,
                        desc->func, desc->fid, desc->file, desc->line );

            total += desc->bytes;
        }

        printf( "   -------\n  %7dk total\n", total >> 10 );
    }

    printf( "\nShared memory file size: %dk\n", shared->heap->size >> 10 );
    printf("Shared memory usage: %d bytes\n", shared->heap->bytes_used);
    printf("Max peak shared memory usage: %d bytes\n", shared->heap->bytes_used_peak);

    fusion_skirmish_dismiss( &shared->lock );

    return DFENUM_OK;
}

static void
dump_shmpools( void )
{
    fusion_shm_enum_pools( dfb_core_world(NULL), dump_shmpool, NULL );
}
#endif

static void change_windows_opacity (IDirectFB *dfb)
{
    if(dfb == NULL)
        return;

    IDirectFBDisplayLayer  *layer = NULL;
    IDirectFBWindow *window = NULL;
    // dfbdump -set-opacity=0 -layer=0 -win=20,30,...
    if (surface_list_head)
    {
        dfb->GetDisplayLayer(dfb, show_which_layer, &layer );
         
        if (layer)
        {
            struct surface_list* temp = surface_list_head;
            struct surface_list* prev;
            while(temp)
            {
                layer->GetWindow(layer, (DFBWindowID)(temp->val), &window);
             
                if (window) {
                    window->SetOpacity(window, show_win);
                    window->Release(window);
                }
                 
                prev = temp;
                temp = temp->next;
                 //free(prev);  //
            }

            layer->Release(layer);
            layer = NULL;
        }
    }
    // dfbdump -set-opacity=0 -layer=0 -win=20
    // dfbdump -set-opacity=0 -layer=0 -win=[20,30]
    //set useful info
    else if ( (show_win != -1)              &&
              (show_which_layer != -1)      &&
              (show_which_win != -1)        &&
              (show_which_win_upperbound != -1) )
    {
        dfb->GetDisplayLayer(dfb, show_which_layer, &layer );
         
        if (layer)
        {
            int max_win, min_win;
            int i;
             
            max_win = show_which_win_upperbound >= show_which_win ? show_which_win_upperbound : show_which_win;
            min_win = show_which_win_upperbound <= show_which_win ? show_which_win_upperbound : show_which_win;
            // "only 1 win" or "a serial wins"
            for (i = min_win; i<= max_win; i++)
            {
                layer->GetWindow(layer, (DFBWindowID)i, &window);
             
                if (window) {
                    window->SetOpacity(window, show_win);
                    window->Release(window);
                }
            }

            layer->Release(layer);
            layer = NULL;
        }

    }
    else if ( (change_win_order != 0) ) 
    {
        printf("\n[DFBDUMP] layer=%d, win=%d\n", show_which_layer, show_which_win);
        dfb->GetDisplayLayer(dfb, show_which_layer, &layer );

        if (layer)
        {
            layer->GetWindow(layer, (DFBWindowID)show_which_win, &window);

            if (window) {
                if (change_win_order == 1)
                    window->RaiseToTop(window);
                else
                    window->LowerToBottom(window);

                window->Release(window);
            }

            layer->Release(layer);
            layer = NULL;
        }
    }
}

static void change_layer_level (IDirectFB *dfb)
{
    if (dfb == NULL || show_which_layer == -1)
        return;

    IDirectFBDisplayLayer  *layer = NULL;

    dfb->GetDisplayLayer( dfb, show_which_layer, &layer );
         
    if (layer)
    {
        layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);    
        layer->SetLevel( layer, level_layer );

        layer->Release(layer);
        layer = NULL;
    }
}

static void change_layer_size(IDirectFB *dfb, int layid, int width, int height)
{
    if(dfb == NULL)
        return;

    DFBDisplayLayerConfig config_new;
    IDirectFBDisplayLayer  *layer = NULL;

    config_new.flags = (DFBDisplayLayerConfigFlags)(DLCONF_HEIGHT | DLCONF_WIDTH);
    config_new.height = height;
    config_new.width  = width;

    dfb->GetDisplayLayer( dfb, layid, &layer );
    
    if (layer)
    {        
         layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE);            
         layer->SetConfiguration( layer, &config_new);

         layer->Release(layer);
         layer = NULL;
    }
}

static void change_layer_buffermode(IDirectFB *dfb, int layid, int mode)
{
    if(dfb == NULL)
        return;

    DFBDisplayLayerConfig config_new;
    IDirectFBDisplayLayer  *layer = NULL;
    DFBDisplayLayerBufferMode buffermode = DLBM_UNKNOWN;

    config_new.flags = (DFBDisplayLayerConfigFlags)(DLCONF_BUFFERMODE);

    switch(mode)
    {
        case SINGLE_BUFFER:
            config_new.buffermode = DLBM_FRONTONLY;
            break;
        case DOUBLE_BUFFER:
            config_new.buffermode = DLBM_BACKVIDEO;
            break;
        case TRIPLE_BUFFER:
            config_new.buffermode = DLBM_TRIPLE;
            break;
        default:
            config_new.buffermode = DLBM_BACKVIDEO;
            break;
    }

    dfb->GetDisplayLayer( dfb, layid, &layer );

    if (layer)
    {
         layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE);
         layer->SetConfiguration( layer, &config_new);

         layer->Release(layer);
         layer = NULL;
    }
}

static void set_ForceFullUpdate_layer(IDirectFB *dfb)
{
    if(dfb == NULL)
        return;

    IDirectFBDisplayLayer  *layer = NULL;
    dfb->GetDisplayLayer( dfb, fullupdate_layer, &layer );

    if (layer)
    {
        layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);
        layer->ForceFullUpdateWindowStack(layer, bfullupdate);
        printf("[Dfbdump][layer %d] set ForceFullUpdate = %d\n", fullupdate_layer, bfullupdate);

        layer->Release(layer);
        layer = NULL;
    }
}

static void
print_usage (const char *prg_name)
{
    fprintf (stderr, "\nDirectFB Dump (version %s)\n\n", DIRECTFB_VERSION);
    fprintf (stderr, "Usage: %s [options]\n\n", prg_name);
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "   -h,                                                           --help          Show this help message\n");
    fprintf (stderr, "   -v,                                                           --version       Print version information\n");
    fprintf (stderr, "   -s,                                                           --shm           Show shared memory pool content (if debug enabled)\n");
    fprintf (stderr, "   -p,                                                           --pools         Show information about surface pools\n");
    fprintf (stderr, "   -a,                                                           --allocs        Show surface buffer allocations in surface pools\n");
    fprintf (stderr, "   -dl,                                                          --dumplayer     Dump surfaces of layer contexts into files (dfb_layer_context_REFID...)\n");
    fprintf (stderr, "   -ds $(id),                                                    --dumpsurface   Dump surfaces (front buffers) into files (dfb_surface_REFID...)\n");
    fprintf (stderr, "   -ds all,                                                      --dumpsurface   Dump all surfaces (front buffers) into files (dfb_surface_REFID...)\n");
    fprintf (stderr, "   -ds [$(id),$(id)],                                            --dumpsurface   Dump surfaces in between 2 ids into files (dfb_surface_REFID...)\n");
    fprintf (stderr, "   -ds $(id1),$(id2),$(id3),                                     --dumpsurface   Dump multi surfaces specified into files (dfb_surface_REFID...)\n");
    fprintf (stderr, "   -set-opacity=0-0xff -layer=$(id) -win=$(A),                                   Change opacity on window(A) on layer(id)\n");
    fprintf (stderr, "   -set-opacity=0-0xff -layer=$(id) -win=[$(A),$(B)],                            Change opacity between win(A) & win(B) on layer(id)\n");
    fprintf (stderr, "   -set-opacity=0-0xff -layer=$(id) -win=$(A),$(B),...,                          Change opacity on multi windows on layer(id)\n");
    fprintf (stderr, "   -layer=$(id) -width=$(width) -height=$(height)                                Change layer  size, it must bigger than current setting.\n");
    fprintf (stderr, "   -layer=$(id) -fullupdate=0/1                                                  Enable/Disable fullupdate of the layer(id). 0:disable, 1:enable\n");
    fprintf (stderr, "   -caps,                                                                        Show detail Capabilities information\n");
    fprintf (stderr, "   -setlevel=1/-1/0 -layer=$(id)                                                 Set level of the layer(id).\n");
    fprintf (stderr, "                                                                                     1:Enable GOP,\n");
    fprintf (stderr, "                                                                                    -1:Disable GOP only,\n");
    fprintf (stderr, "                                                                                     0:Disable GOP until the next surface->Flip of a Layer/Window invoked\n");
    fprintf (stderr, "   -raisetotop  -layer=$(id) -win=$(A)             change window(A) order to bottom\n");
    fprintf (stderr, "   -lowtobottom -layer=$(id) -win=$(A)             change window(A) order to bottom\n");
    fprintf (stderr, "   -layer=$(id) -buffermode=1/2/3                                                Change layer buffermode.  1:Single buffer/ 2:Double buffer/ 3:Triple buffer\n");
    fprintf (stderr, "\n");
}

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
    IDirectFB *dfb = NULL;
    DFBResult ret;
    long long millis;
    long int  seconds, minutes, hours, days;
    char buffer[0x2000];

    
    
    setvbuf( stdout, buffer, _IOFBF, 0x2000 );

    /* Initialize DirectFB. */
    ret = DirectFBInit( &argc, &argv );
    if (ret)
    {
        DirectFBError( "DirectFBInit", ret );
      
        return -1;
    }

    /* Parse the command line. */
    if (!parse_command_line( argc, argv ))
    {
        print_usage (argv[0]);
        return -2;
    }

          

    /* Create the super interface. */
    ret = DirectFBCreate( &dfb );
    if (ret)
    {

        DirectFBError( "DirectFBCreate", ret );
        return -3;
    }

    bInitQuery = dump_init_query_info();
     
    millis = direct_clock_get_millis();

    seconds  = millis / 1000;
    millis  %= 1000;

    minutes  = seconds / 60;
    seconds %= 60;

    hours    = minutes / 60;
    minutes %= 60;

    days     = hours / 24;
    hours   %= 24;

    switch (days)
    {
        case 0:
            printf( "\nDirectFB uptime: %02ld:%02ld:%02ld\n",
                        hours, minutes, seconds );
            break;

        case 1:
            printf( "\nDirectFB uptime: %ld day, %02ld:%02ld:%02ld\n",
                        days, hours, minutes, seconds );
            break;

        default:
            printf( "\nDirectFB uptime: %ld days, %02ld:%02ld:%02ld\n",
                        days, hours, minutes, seconds );
            break;
    }

    /***** SHOW / HIDE Windows *****/
    // change windows opacity, do not show dfbdump-info. Please do not move the fun.
    change_windows_opacity (dfb);
     
    /***** set layer level *****/
    change_layer_level (dfb);
     
    if (show_which_layer > -1 && layer_width > 0 && layer_height > 0)
        change_layer_size(dfb, show_which_layer, layer_width, layer_height);

    if (show_which_layer > -1 && layer_buffermode != NULL)
        change_layer_buffermode(dfb, show_which_layer, layer_buffermode);

    if (bCall_fullupdate)
        set_ForceFullUpdate_layer(dfb);
     
    dump_surfaces();
    fflush( stdout );

    if(bInitQuery)
        dump_max_mem_usage(dfb);

    if(bInitQuery)
        dump_process_mem_usage(dfb);

    dump_layers();
    fflush( stdout );


#if FUSION_BUILD_MULTI
    if (show_shm)
    {
        printf( "\n" );
        dump_shmpools();
        fflush( stdout );
    }
#endif

    if (show_pools) {
        printf( "\n" );
        dump_surface_pool_info();
        fflush( stdout );
    }

    if (show_allocs) {
        printf( "\n" );
        dump_surface_pools();
        fflush( stdout );
    }

    printf( "\n" );
     
    /* DirectFB deinitialization. */
    if (dfb)
        dfb->Release( dfb );

    return ret;
}

/**********************************************************************************************************************/

static DFBBoolean
parse_command_line( int argc, char *argv[] )
{
    int n;

    for (n = 1; n < argc; n++)
    {
        const char *arg = argv[n];

        if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0)
        {
            return DFB_FALSE;
        }

        if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0)
        {
            fprintf (stderr, "dfbdump version %s\n", DIRECTFB_VERSION);
            return DFB_FALSE;
        }

        if (strcmp (arg, "-s") == 0 || strcmp (arg, "--shm") == 0)
        {
            show_shm = true;
            continue;
        }

        if (strcmp (arg, "-p") == 0 || strcmp (arg, "--pools") == 0)
        {
            show_pools = true;
            continue;
        }

        if (strcmp (arg, "-a") == 0 || strcmp (arg, "--allocs") == 0)
        {
            show_allocs = true;
            continue;
        }

        if (strcmp (arg, "-dl") == 0 || strcmp (arg, "--dumplayer") == 0)
        {
            dump_layer = -1;
            continue;
        }
          
        if (strcmp (arg, "-ds") == 0 || strcmp (arg, "--dumpsurface") == 0)
        {
            char* comma = 0;

            //if there is an arg after -ds
            if(n < argc - 1)
            {
                char* target = argv[n+1];

                //dfbdump -ds all
                if(!strcmp (target, "all"))
                    dump_surface  = -1;
                //dfbdump -ds [A, B]
                else if(*target == '[')
                {
                    dump_surface              = strtol ((target + 1), &comma, 16);
                    dumpsurface_upperbound    = strtol ((comma + 1), NULL, 16);
                }
                //dfbdump -ds A, B, C, D, ...
                else if(strstr (target, ",") )
                {
                    int tempint = strtol(target, &comma, 16);

                    if( comma   == 0 || tempint == 0 )
                    {
                        return DFB_FALSE;
                    }
                        surface_list_head           = (struct surface_list*) malloc (sizeof(struct surface_list));
                        surface_list_head->val      = tempint;
                        surface_list_head->next     = 0;
                        struct surface_list* temp   = surface_list_head;

                        while(*comma != 0)
                        {
                            tempint = strtol((comma + 1), &comma, 16);

                            if(tempint)
                            {
                                temp->next          = malloc (sizeof(struct surface_list));
                                temp                = temp->next;
                                temp->val           = tempint;
                                temp->next          = 0;
                            }
                            else
                                continue;
                        }
                }
                else
                    dump_surface = strtol (target, NULL, 16 );
                if( dump_surface > 0 && dumpsurface_upperbound == 0 )
                    dumpsurface_upperbound = dump_surface;
                continue;
            }
            else
            {
                return DFB_FALSE;
            }
        }
        // -set-opacity=ooxx
        if (strstr (arg, CMD_SET_OPACITY) )
        {
            show_win = strtol (arg + strlen(CMD_SET_OPACITY) + 1/*=*/, NULL, 16 );
            continue;
        }
        // -layer=ooxx
        if (strstr (arg, CMD_LAYER) )
        {
            show_which_layer = strtol (arg + strlen (CMD_LAYER) + 1/*=*/, NULL, 10 );
            continue;
        }

        if (strstr (arg, CMD_WIN) )
        {
            char* comma = 0;

            // -win=[A,B]
            if(strstr(arg,"["))
            {
                show_which_win = strtol (arg + strlen (CMD_WIN) + 2/*=[*/, &comma, 10);
                show_which_win_upperbound = strtol ((comma + 1), NULL, 10);
            }
            // -win=A,B,C,D...
            else if(strstr(arg,","))
            {
                int tempint = strtol(arg + strlen (CMD_WIN) + 1/*=*/, &comma, 10);

                if( comma   ==  0  || tempint ==  0)
                {
                    return DFB_FALSE;
                }

                surface_list_head       = (struct surface_list*)malloc(sizeof(struct surface_list));
                surface_list_head->val  = tempint;
                surface_list_head->next = 0;
                struct surface_list* temp = surface_list_head;

                while(*comma != 0)
                {
                    tempint = strtol ((comma + 1), &comma, /*16*/ 10);

                    if (tempint)
                    {
                        temp->next     = malloc (sizeof(struct surface_list));
                        temp           = temp->next;
                        temp->val      = tempint;
                        temp->next     = 0;
                    }
                    else
                        continue;
                }
            }
            // -win=A
            else
            {
                show_which_win =  strtol (arg + strlen (CMD_WIN) + 1/*=*/, NULL, 10 );
            }

            if( show_which_win >= 0         &&
                show_which_win_upperbound   == -1 )
            {
                show_which_win_upperbound = show_which_win;
            }
            continue;
        }

        // -setlevel=-1/0/1
        if (strstr (arg, CMD_SETLEVEL))
        {
            level_layer = strtol (arg + strlen (CMD_SETLEVEL) + 1/*=*/, NULL, 10 );

            if ((level_layer !=     -1)  &&
                (level_layer !=      0)  &&
                (level_layer !=      1))
            {
                return DFB_FALSE;
            }
            continue;
        }

        // change layer size.
        if (strstr (arg, CMD_WIDTH))
        {
            layer_width = strtol ( arg + strlen (CMD_WIDTH) + 1/*=*/, NULL, 10 );
            continue;
        }

        if (strstr (arg, CMD_HEIGHT))
        {
            layer_height = strtol ( arg + strlen (CMD_HEIGHT) + 1/*=*/, NULL, 10 );
            continue;
        }

        if (strstr (arg, CMD_BUFFERMODE))
        {
            layer_buffermode = strtol ( arg + strlen (CMD_BUFFERMODE) + 1/*=*/, NULL, 10 );
            continue;
        }

        // set layer force fullupdate.
        if (strstr (arg, CMD_FULLUPDATE))
        {
            bCall_fullupdate = true;
            bfullupdate = strtol (arg + strlen (CMD_FULLUPDATE) + 1/*=*/, NULL, 10 );
            continue;
        }

        // show Capabilities information.
        if (strstr (arg, CMD_CAPS))
        {
            bGetCAPS = true;
            continue;
        }

        if (strstr (arg, CMD_WIN_RAISETOTOP))
        {
            change_win_order = 1;
            continue;
        }

        if (strstr (arg, CMD_WIN_LOWTOBOTTOM))
        {
            change_win_order = -1;
            continue;
        }

    }

    return DFB_TRUE;
}

