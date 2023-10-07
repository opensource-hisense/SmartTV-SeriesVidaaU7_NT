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

#include <string.h>

#include <directfb_util.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <time.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>
#include <idirectfb.h>


#include <gfx/convert.h>

#include <misc/util.h>
#include <misc/conf.h>

#include <core/coretypes.h>
#include <core/surface.h>
#include <core/surface_buffer.h>


D_DEBUG_DOMAIN( DFB_Updates, "DirectFB/Updates", "DirectFB Updates" );

/**********************************************************************************************************************/

const DirectFBPixelFormatNames( dfb_pixelformat_names );
const DirectFBColorSpaceNames( dfb_colorspace_names );

/**********************************************************************************************************************/

#ifdef DFB_MEASURE_BOOT_TIME

DFBBootLog    bootInfo[DF_BOOT_ALL];
char         *getENVFlag;

/*
    How to measure the time of DFB Initialization?
    Step1: vi /etc/profile, add the setting as below
               export DFB_BOOTTIME_PROFILE=1
    Step2: Reboot and you will see the log showing in uart  in detail.

*/
void dfb_boot_initENV(void)
{
    getENVFlag = getenv("DFB_BOOTTIME_PROFILE");
}

void dfb_boot_getTime(  const char* name,
                        DFB_Boot_MeasureIntervals intervals,
                        DFB_Boot_MeasureFlag flag,
                        DFB_Boot_PrintLevel level )
{
    if (getENVFlag == 0)
        return;

    if ( intervals >= DF_BOOT_ALL )
        return;

    struct timespec ts;
    long ms = 0;

    if(bootInfo[intervals].name == NULL)
        bootInfo[intervals].name = D_STRDUP(name);

    bootInfo[intervals].level = level;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

    switch (flag)
    {
        case DF_MEASURE_START :
            bootInfo[intervals].startTime = ms;
            break;

        case DF_MEASURE_END :
            bootInfo[intervals].endTime = ms;
            break;

        default :
            break;
    }
}


void dfb_print_duration(DFB_Boot_MeasureFlag flag, const char* title)
{
    if (getENVFlag == 0)
        return;

    static struct timeval tlast;
    struct timeval tdiff;
    struct timeval tcurr;

    if(flag == DF_MEASURE_START)
    {
        gettimeofday(&tlast, NULL);
    }
    else
    {
        gettimeofday(&tcurr, NULL);
        timersub(&tcurr, &tlast, &tdiff);
        printf("[DFB] duration: <%s> %ld.%06ld\n", title, tdiff.tv_sec, tdiff.tv_usec);

        gettimeofday(&tlast, NULL);
    }
}

#define PRINT_TIME_INFO(X...)\
    do{\
        printf(X);\
        direct_debug_redirect_to_tmpfile(X);\
    }while(0)

void dfb_boot_printTimeInfo(void)
{
    if (getENVFlag == 0)
        return;

    int pid = getpid();

    printf("======================(PID: %d)==============================\n", pid);

    int i=0, lvCnt=0;
    for (i = 0 ; i < DF_BOOT_ALL ; i++)
    {
        if (NULL == bootInfo[i].name)
            continue;

        char    *log = "---";
        long     result = bootInfo[i].endTime - bootInfo[i].startTime;

        for (lvCnt = 1; lvCnt < bootInfo[i].level; lvCnt++)
        {
            printf("%s", log);
        }

        printf("\33[0;33;44m%s = %lu\33[0m\n" , bootInfo[i].name, result);

    }

    printf("======================(PID: %d)==============================\n", pid);

}

#endif

bool
dfb_region_rectangle_intersect( DFBRegion          *region,
                                const DFBRectangle *rect )
{
     int x2 = rect->x + rect->w - 1;
     int y2 = rect->y + rect->h - 1;

     if (region->x2 < rect->x ||
         region->y2 < rect->y ||
         region->x1 > x2 ||
         region->y1 > y2)
          return false;

     if (region->x1 < rect->x)
          region->x1 = rect->x;

     if (region->y1 < rect->y)
          region->y1 = rect->y;

     if (region->x2 > x2)
          region->x2 = x2;

     if (region->y2 > y2)
          region->y2 = y2;

     return true;
}

bool
dfb_unsafe_region_intersect( DFBRegion *region,
                             int x1, int y1, int x2, int y2 )
{
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     return dfb_region_intersect( region, x1, y1, x2, y2 );
}

bool
dfb_unsafe_region_rectangle_intersect( DFBRegion          *region,
                                       const DFBRectangle *rect )
{
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     return dfb_region_rectangle_intersect( region, rect );
}

bool
dfb_rectangle_intersect_by_unsafe_region( DFBRectangle *rectangle,
                                          DFBRegion    *region )
{
     /* validate region */
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     /* adjust position */
     if (region->x1 > rectangle->x) {
          rectangle->w -= region->x1 - rectangle->x;
          rectangle->x = region->x1;
     }

     if (region->y1 > rectangle->y) {
          rectangle->h -= region->y1 - rectangle->y;
          rectangle->y = region->y1;
     }

     /* adjust size */
     if (region->x2 < rectangle->x + rectangle->w - 1)
        rectangle->w = region->x2 - rectangle->x + 1;

     if (region->y2 < rectangle->y + rectangle->h - 1)
        rectangle->h = region->y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

bool
dfb_rectangle_intersect_by_region( DFBRectangle    *rectangle,
                                   const DFBRegion *region )
{
     /* adjust position */
     if (region->x1 > rectangle->x) {
          rectangle->w -= region->x1 - rectangle->x;
          rectangle->x = region->x1;
     }

     if (region->y1 > rectangle->y) {
          rectangle->h -= region->y1 - rectangle->y;
          rectangle->y = region->y1;
     }

     /* adjust size */
     if (region->x2 < rectangle->x + rectangle->w - 1)
        rectangle->w = region->x2 - rectangle->x + 1;

     if (region->y2 < rectangle->y + rectangle->h - 1)
        rectangle->h = region->y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

bool dfb_rectangle_intersect( DFBRectangle       *rectangle,
                              const DFBRectangle *clip )
{
     DFBRegion region = { clip->x, clip->y,
                          clip->x + clip->w - 1, clip->y + clip->h - 1 };

     /* adjust position */
     if (region.x1 > rectangle->x) {
          rectangle->w -= region.x1 - rectangle->x;
          rectangle->x = region.x1;
     }

     if (region.y1 > rectangle->y) {
          rectangle->h -= region.y1 - rectangle->y;
          rectangle->y = region.y1;
     }

     /* adjust size */
     if (region.x2 < rectangle->x + rectangle->w - 1)
          rectangle->w = region.x2 - rectangle->x + 1;

     if (region.y2 < rectangle->y + rectangle->h - 1)
          rectangle->h = region.y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

void dfb_rectangle_union ( DFBRectangle       *rect1,
                           const DFBRectangle *rect2 )
{
     if (!rect2->w || !rect2->h)
          return;

     /* FIXME: OPTIMIZE */

     if (rect1->w) {
          int temp = MIN (rect1->x, rect2->x);
          rect1->w = MAX (rect1->x + rect1->w, rect2->x + rect2->w) - temp;
          rect1->x = temp;
     }
     else {
          rect1->x = rect2->x;
          rect1->w = rect2->w;
     }

     if (rect1->h) {
          int temp = MIN (rect1->y, rect2->y);
          rect1->h = MAX (rect1->y + rect1->h, rect2->y + rect2->h) - temp;
          rect1->y = temp;
     }
     else {
          rect1->y = rect2->y;
          rect1->h = rect2->h;
     }
}

void
dfb_updates_init( DFBUpdates *updates,
                  DFBRegion  *regions,
                  int         max_regions )
{
     D_ASSERT( updates != NULL );
     D_ASSERT( regions != NULL );
     D_ASSERT( max_regions > 0 );

     updates->regions     = regions;
     updates->max_regions = max_regions;
     updates->num_regions = 0;

     D_MAGIC_SET( updates, DFBUpdates );
}

void
dfb_updates_add( DFBUpdates      *updates,
                 const DFBRegion *region )
{
     int i;

     D_MAGIC_ASSERT( updates, DFBUpdates );
     DFB_REGION_ASSERT( region );
     D_ASSERT( updates->regions != NULL );
     D_ASSERT( updates->num_regions >= 0 );
     D_ASSERT( updates->num_regions <= updates->max_regions );

     D_DEBUG_AT( DFB_Updates, "%s( %p, %4d,%4d-%4dx%4d )\n", __FUNCTION__, updates,
                 DFB_RECTANGLE_VALS_FROM_REGION(region) );

     if (updates->num_regions == 0) {
          D_DEBUG_AT( DFB_Updates, "  -> added as first\n" );

          updates->regions[0]  = updates->bounding = *region;
          updates->num_regions = 1;

          return;
     }

     for (i=0; i<updates->num_regions; i++) {
          if (dfb_region_region_extends( &updates->regions[i], region ) ||
              dfb_region_region_intersects( &updates->regions[i], region ))
          {
               D_DEBUG_AT( DFB_Updates, "  -> combined with [%d] %4d,%4d-%4dx%4d\n", i,
                           DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[i]) );

               dfb_region_region_union( &updates->regions[i], region );

               dfb_region_region_union( &updates->bounding, region );

               D_DEBUG_AT( DFB_Updates, "  -> resulting in  [%d] %4d,%4d-%4dx%4d\n", i,
                           DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[i]) );

               return;
          }
     }

     if (updates->num_regions == updates->max_regions) {
          dfb_region_region_union( &updates->bounding, region );

          updates->regions[0]  = updates->bounding;
          updates->num_regions = 1;

          D_DEBUG_AT( DFB_Updates, "  -> collapsing to [0] %4d,%4d-%4dx%4d\n",
                      DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[0]) );
     }
     else {
          updates->regions[updates->num_regions++] = *region;

          dfb_region_region_union( &updates->bounding, region );

          D_DEBUG_AT( DFB_Updates, "  -> added as      [%d] %4d,%4d-%4dx%4d\n", updates->num_regions - 1,
                      DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[updates->num_regions - 1]) );
     }
}

void
dfb_updates_stat( DFBUpdates *updates,
                  int        *ret_total,
                  int        *ret_bounding )
{
     int i;

     D_MAGIC_ASSERT( updates, DFBUpdates );
     D_ASSERT( updates->regions != NULL );
     D_ASSERT( updates->num_regions >= 0 );
     D_ASSERT( updates->num_regions <= updates->max_regions );

     if (updates->num_regions == 0) {
          if (ret_total)
               *ret_total = 0;

          if (ret_bounding)
               *ret_bounding = 0;

          return;
     }

     if (ret_total) {
          int total = 0;

          for (i=0; i<updates->num_regions; i++) {
               const DFBRegion *r = &updates->regions[i];

               total += (r->x2 - r->x1 + 1) * (r->y2 - r->y1 + 1);
          }

          *ret_total = total;
     }

     if (ret_bounding)
          *ret_bounding = (updates->bounding.x2 - updates->bounding.x1 + 1) *
                          (updates->bounding.y2 - updates->bounding.y1 + 1);
}

void
dfb_updates_get_rectangles( DFBUpdates   *updates,
                            DFBRectangle *ret_rects,
                            int          *ret_num )
{
     D_MAGIC_ASSERT( updates, DFBUpdates );
     D_ASSERT( updates->regions != NULL );
     D_ASSERT( updates->num_regions >= 0 );
     D_ASSERT( updates->num_regions <= updates->max_regions );

     switch (updates->num_regions) {
          case 0:
               *ret_num = 0;
               break;

          default: {
               int n, d, total, bounding;

               dfb_updates_stat( updates, &total, &bounding );

               n = updates->max_regions - updates->num_regions + 1;
               d = n + 1;

               /* Try to optimize updates. Use individual regions only if not too much overhead. */
               if (total < bounding * n / d) {
                    *ret_num = updates->num_regions;

                    for (n=0; n<updates->num_regions; n++) {
                         ret_rects[n].x = updates->regions[n].x1;
                         ret_rects[n].y = updates->regions[n].y1;
                         ret_rects[n].w = updates->regions[n].x2 - updates->regions[n].x1;
                         ret_rects[n].h = updates->regions[n].y2 - updates->regions[n].y1;
                    }

                    break;
               }
          }
          /* fall through */

          case 1:
               *ret_num = 1;

               ret_rects[0].x = updates->bounding.x1;
               ret_rects[0].y = updates->bounding.y1;
               ret_rects[0].w = updates->bounding.x2 - updates->bounding.x1;
               ret_rects[0].h = updates->bounding.y2 - updates->bounding.y1;
               break;
     }
}

const char *
dfb_pixelformat_name( DFBSurfacePixelFormat format )
{
     int i = 0;

     do {
          if (format == dfb_pixelformat_names[i].format)
               return dfb_pixelformat_names[i].name;
     } while (dfb_pixelformat_names[i++].format != DSPF_UNKNOWN);

     return "<invalid>";
}
 
const char *
dfb_colorspace_name( DFBSurfaceColorSpace colorspace )
{
     int i = 0;

     do {
          if (colorspace == dfb_colorspace_names[i].colorspace)
               return dfb_colorspace_names[i].name;
     } while (dfb_colorspace_names[i++].colorspace != DSCS_UNKNOWN);

     return "<invalid>";
}

DFBSurfacePixelFormat
dfb_pixelformat_for_depth( int depth )
{
     switch (depth) {
          case 2:
               return DSPF_LUT2;
          case 8:
               return DSPF_LUT8;
          case 12:
               return DSPF_ARGB4444;
          case 14:
               return DSPF_ARGB2554;
          case 15:
               return DSPF_ARGB1555;
          case 16:
               return DSPF_RGB16;
          case 18:
               return DSPF_RGB18;
          case 24:
               return DSPF_RGB24;
          case 32:
               return DSPF_RGB32;
     }

     return DSPF_UNKNOWN;
}

bool dfb_dumpQueryInfo(int index, DumpInfoType* info)
{
    switch(index)
    {
        case DF_DUMP_MAXMEM :
        {
            IDirectFB       *dfb    = NULL;
            IDirectFB_data  *data   = NULL;
            CoreDFB         *core   = NULL;
            CoreDFBShared   *shared;

            dfb = info->input;
            data = (IDirectFB_data*)dfb->priv;
            core = data->core;

            shared = core->shared;
            *(int*)info->output = shared->maxPeakMem;
        }   
            break;

        case DF_DUMP_GET_PROCESS_ID :
        {
            FusionObject *object = (FusionObject*)info->input;
            *(int*)info->output = object->PID;
        }
            break;

        case DF_DUMP_GET_PARENT_PROCESS_ID :
        {
            FusionObject *object = (FusionObject*)info->input;
            *(int*)info->output = object->PPID;
        }
            break;

        case DF_DUMP_SET_DUMPSURFACE_BY_PROCESS_ID:
        {
            IDirectFB       *dfb    = NULL;
            IDirectFB_data  *data   = NULL;
            CoreDFB         *core   = NULL;
            CoreDFBShared   *shared = NULL;
            DumpSurfaceProcessInfo *process_info = NULL;

            if(info->input == 0 || info->output == 0)
            {
                D_ERROR("[DFB][DF_DUMP_SET_DUMPSURFACE_BY_PROCESS_ID] Invalid prameters input. Please check it again!!!\n");
                return false;
            }

            process_info = (DumpSurfaceProcessInfo *) info->input;

            dfb = (IDirectFB*) info->output;
            data = (IDirectFB_data*)dfb->priv;
            core = data->core;
            shared = core->shared;

            if(process_info->pid_start >= 0 && process_info->pid_end >= process_info->pid_start)
            {
                fusion_skirmish_prevail( &shared->lock );

                shared->dump_pid_start = process_info->pid_start;
                shared->dump_pid_end = process_info->pid_end;

                fusion_skirmish_dismiss( &shared->lock );
            }
            else
            {
                D_ERROR("[DFB][DF_DUMP_SET_DUMPSURFACE_BY_PROCESS_ID] Invalid prameters input. Please check it again!!!\n");
                return false;
            }

        }
        break;

        case DF_DUMP_GET_PROCESS_MEM_INFO :
        {
            IDirectFB       *dfb    = NULL;
            IDirectFB_data  *data   = NULL;
            CoreDFB         *core   = NULL;
            CoreDFBShared   *shared;

            dfb = info->input;
            data = (IDirectFB_data*)dfb->priv;
            core = data->core;

            shared = core->shared;
            
            //(DFBPeakMemInfo*)info->output = shared->memInfo;    
            //DFBPeakMemInfo ** p = (DFBPeakMemInfo**)info->output;
            //*p = shared->memInfo;

            int i = 0;
            printf( "\n"
                    "\n-----------------------------[ Every Process Peak Memory Usage ]-------------------\n" );
            printf( "\tPID \t\tPPID \t\tSurface \tLayer\n");

            for (i = 0 ; i < dfb_config->mst_mem_peak_usage ; i++)
            {
                if ( shared->memInfo[i].bUse == true )
                {
                    printf( "\33[0;33m\t%d \t\t%d \t\t%dk \t\t%dk \n\33[0m",
                            i,
                            shared->memInfo[i].PPID,
                            shared->memInfo[i].maxPeakMem >> 10,
                            shared->memInfo[i].layerSurfaceMem >> 10 );

                }
            }

            printf( "\n----------------------------------------------------------------------------------\n" );
            
        }
            break;



        default :
            return false;            

    }


    return true;
}


