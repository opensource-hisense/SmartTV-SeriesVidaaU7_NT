/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
#define DIRECT_ENABLE_DEBUG

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>
#include <directfb_util.h>

#include <direct/clock.h>
#include <direct/debug.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <gfx/convert.h>

#include "util.h"

#define DELIM  " \t"

#define CHECK(x)                                  \
     do {                                         \
          DFBResult ret = (x);                    \
          if (ret)                                \
               DirectFBErrorFatal(#x,ret);        \
     } while (0)

D_DEBUG_DOMAIN( Quip_Object,    "Quip/Object",    "Quip Object" );
D_DEBUG_DOMAIN( Quip_Thumbnail, "Quip/Thumbnail", "Quip Thumbnail Object" );
D_DEBUG_DOMAIN( Quip_Update,    "Quip/Update",    "Quip Updates" );

/**********************************************************************************************************************/

typedef struct __Quip_QuipScene  QuipScene;
typedef struct __Quip_QuipObject QuipObject;

/**********************************************************************************************************************/

typedef struct {
     IDirectFBWindow       *window;
     IDirectFBSurface      *surface;

     int                    width;
     int                    height;
     DFBSurfacePixelFormat  format;
} QuipOutput;

struct __Quip_QuipScene {
     DirectLink          link;

     int                 magic;

     QuipOutput         *output;

     DFBDimension        coords;
     DFBDimension        size;

     char               *name;
     DirectLink         *objects;

     QuipObject         *entered;
     QuipObject         *grabber;

     DFBUpdates          updates;
     DFBRegion           update_regions[8];
};

/**********************************************************************************************************************/

typedef enum {
     QOT_IMAGE,
     QOT_SLIDEBAR,
     QOT_THUMBNAIL,
     QOT_TEXT
} QuipObjectType;

typedef enum {
     QOF_FADEIN   = 0x01,
     QOF_ZOOMIN   = 0x02,
     QOF_FADEOUT  = 0x04,
     QOF_ZOOMOUT  = 0x08,
     QOF_REALIZED = 0x10
} QuipObjectFlags;

typedef enum {
     QOS_COME,
     QOS_STAY,
     QOS_GO
} QuipObjectState;

struct __Quip_QuipObject {
     DirectLink          link;

     int                 magic;

     QuipScene          *scene;
     QuipObject         *parent;
     DirectLink         *children;

     QuipObjectType      type;     /* Image or Text? */
     QuipObjectFlags     flags;    /* Transition effects */
     QuipObjectState     state;    /* Come, Stay, Go */

     DFBPoint            position; /* main position (during stay) */
     DFBDimension        size;     /* actual size of the content */
     DFBPoint            center;   /* calculated center of the above */
     DFBColor            color;    /* object color, e.g. text color */

     DFBRectangle        bounds;   /* current position/size */
     int                 opacity;  /* current opacity */

     int                 age;      /* current age */

     int                 stay;     /* normal lifetime, -1 means unlimited */

     int                 t_arrive; /* age when fade/zoom in is done */
     int                 t_leave;  /* age when fade/zoom out is started */
     int                 t_gone;   /* age when fade/zoom out is done */

     bool                go_active;
     DFBRectangle        go_target;
     int                 go_start;
     int                 go_time;
     DFBRectangle        go_from;  /* previous position/size */

     bool                fade_active;
     int                 fade_target;
     int                 fade_start;
     int                 fade_time;
     int                 fade_from;  /* previous opacity */

     bool (*Realize)( QuipObject *object );
     bool (*Tick)   ( QuipObject *object, int ms );
     bool (*Motion) ( QuipObject *object, int x, int y );
     bool (*Leave)  ( QuipObject *object );
     bool (*Press)  ( QuipObject *object, DFBInputDeviceButtonIdentifier button );
     bool (*Unpress)( QuipObject *object, DFBInputDeviceButtonIdentifier button );
     void (*Draw)   ( QuipObject *object, const DFBRectangle *bounds, IDirectFBSurface *surface );
     void (*Destroy)( QuipObject *object );
};

/**********************************************************************************************************************/

typedef struct {
     QuipObject          object;

     IDirectFBSurface   *surface;
     DFBImageDescription image_desc;
} QuipImage;

typedef enum {
     QSBE_LEFT,
     QSBE_TOP,
     QSBE_RIGHT,
     QSBE_BOTTOM
} QuipSlidebarEdge;

typedef struct {
     QuipObject          object;

     QuipSlidebarEdge    edge;
     IDirectFBSurface   *surface;
     DFBImageDescription image_desc;

     QuipScene          *scene;
     char               *scene_name;

     bool                active;

     QuipOutput          output;
} QuipSlidebar;

typedef struct {
     QuipObject          object;

     IDirectFBSurface   *surface;

     bool                zoomed;
     bool                pressed;

     QuipImage          *icons[8];
} QuipThumbnail;

typedef struct {
     QuipObject          object;

     char               *message;
} Text;

/**********************************************************************************************************************/

static IDirectFB             *dfb;
static IDirectFBFont         *font;
static IDirectFBSurface      *cursor;
static IDirectFBEventBuffer  *events;
static IDirectFBDisplayLayer *layer;
static int                    font_ascender;
static bool                   bounds_debug = false;
static bool                   show_timefps = true;

/**********************************************************************************************************************/

static DirectLink            *quip_scenes;
static long                   quip_time;
static DFBColor               quip_color = { 0xff, 0xee, 0xee, 0xee };
static QuipScene             *quip_scene;
static QuipScene             *quip_overlay;
static FPSData                quip_fps;
static int                    quip_cx = 0;
static int                    quip_cy = 0;
static char                  *quip_path;
static bool                   quip_run;

typedef struct {
     long                     start;
     long                     frames;
} Bench;

static Bench icon_bench;
static Bench thumb_bench;
static Bench slide_bench;

/**********************************************************************************************************************/

static DFBResult load_image( IDirectFB              *dfb,
                             const char             *filename,
                             DFBSurfacePixelFormat   pixelformat,
                             IDirectFBSurface      **surface,
                             int                    *width,
                             int                    *height,
                             DFBImageDescription    *desc,
                             const QuipScene        *scene );

/**********************************************************************************************************************/

static void
BenchStart( Bench *bench )
{
     bench->start  = quip_time;
     bench->frames = quip_fps.total_frames;
}

static void
BenchStop( Bench *bench, const char *text )
{
     if (bench->start) {
          int diff   = quip_time - bench->start;
          int frames = quip_fps.total_frames - bench->frames;
          int fps10  = frames * 10000 / (diff ? : 1);

          bench->start = 0;

          printf( "%s: %d ms, %d frames -> %d.%d FPS\n", text, diff, frames, fps10 / 10, fps10 % 10 );
     }
}

static void
ObjectToFront( QuipObject *object )
{
     QuipScene *scene;

     D_MAGIC_ASSERT( object, QuipObject );

     D_DEBUG_AT( Quip_Object, "%s( %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 object, DFB_RECTANGLE_VALS( &object->bounds ) );

     scene = object->scene;
     D_MAGIC_ASSERT( scene, QuipScene );

     if (object->parent) {
          QuipObject *parent = object->parent;

          D_MAGIC_ASSERT( parent, QuipObject );

          direct_list_remove( &parent->children, &object->link );
          direct_list_append( &parent->children, &object->link );
     }
     else {
          direct_list_remove( &scene->objects, &object->link );
          direct_list_append( &scene->objects, &object->link );
     }
}

static void
ObjectOffset( QuipObject *object, int *ret_x, int *ret_y )
{
     int x = 0, y = 0;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( ret_x != NULL );
     D_ASSERT( ret_y != NULL );

     while (object->parent) {
          QuipObject *parent = object->parent;

          D_MAGIC_ASSERT( parent, QuipObject );

          x += parent->position.x;
          y += parent->position.y;

          object = parent;
     }

     *ret_x = x;
     *ret_y = y;
}

static void
InitObject( QuipObject     *object,
            int             x,
            int             y,
            int             w,
            int             h,
            QuipObjectType  type )
{
     D_DEBUG_AT( Quip_Object, "%s( %p [%d,%d-%dx%d] )\n", __FUNCTION__, object, x, y, w, h );

     object->position.x = x;
     object->position.y = y;
     object->size.w     = w;
     object->size.h     = h;
     object->center.x   = x + w / 2;
     object->center.y   = y + h / 2;
     object->color      = quip_color;
             
     object->type     = type;
     object->bounds.x = x;
     object->bounds.y = y;
     object->bounds.w = w;
     object->bounds.h = h;

     object->opacity  = 255;

     D_MAGIC_SET( object, QuipObject );
}

static bool
ReshapeObject( QuipObject         *object,
               const DFBRectangle *target,
               int                 duration )
{
     D_MAGIC_ASSERT( object, QuipObject );
     DFB_RECTANGLE_ASSERT( target );

     D_DEBUG_AT( Quip_Object, "%s( %p [%d,%d-%dx%d] -> [%d,%d-%dx%d] in %d ms )\n", __FUNCTION__,
                 object, DFB_RECTANGLE_VALS( &object->bounds ), DFB_RECTANGLE_VALS( target ), duration );

     if (object->go_active) {
          if (DFB_RECTANGLE_EQUAL( object->go_target, *target ))
               return true;
     }
     else if (DFB_RECTANGLE_EQUAL( object->bounds, *target ))
          return false;

     object->go_target = *target;

     object->go_start = quip_time;
     object->go_time  = duration;

     object->go_active = true;

     object->go_from = object->bounds;

     return true;
}

static bool
FadeObject( QuipObject *object,
            int         opacity,
            int         duration )
{
     D_MAGIC_ASSERT( object, QuipObject );

     D_DEBUG_AT( Quip_Object, "%s( %p [%d] -> [%d] in %d ms )\n", __FUNCTION__,
                 object, object->opacity, opacity, duration );

     if (object->fade_active) {
          if (object->fade_target == opacity)
               return true;
     }
     else if (object->opacity == opacity)
          return false;

     object->fade_target = opacity;

     object->fade_start = quip_time;
     object->fade_time  = duration;

     object->fade_active = true;

     object->fade_from = object->opacity;

     return true;
}

static QuipObject *
SceneObjectAt( QuipScene *scene, int *x, int *y )
{
     DirectLink *objects;
     QuipObject *object;

     D_MAGIC_ASSERT( scene, QuipScene );

     objects = scene->objects;

     do {
          direct_list_foreach_reverse (object, objects) {
               D_MAGIC_ASSERT( object, QuipObject );

               if (object->bounds.x <= *x &&
                   object->bounds.y <= *y &&
                   object->bounds.x + object->bounds.w > *x &&
                   object->bounds.y + object->bounds.h > *y)
               {
                    objects = object->children;

                    *x -= object->bounds.x;
                    *y -= object->bounds.y;

                    break;
               }
          }
     } while (objects && object);

     return object;
}

/**********************************************************************************************************************/

static void
DrawObjects( DirectLink *objects, int x, int y, IDirectFBSurface *surface )
{
     QuipObject *object;

     D_DEBUG_AT( Quip_Object, "%s( %p, %d,%d )\n", __FUNCTION__, objects, x, y );

     direct_list_foreach (object, objects) {
          DFBRectangle bounds;

          D_MAGIC_ASSERT( object, QuipObject );

          if (!(object->flags & QOF_REALIZED)) {
               if (object->Realize && !object->Realize( object ))
                    continue;

               object->flags |= QOF_REALIZED;
          }

          bounds.x = object->bounds.x + x;
          bounds.y = object->bounds.y + y;
          bounds.w = object->bounds.w;
          bounds.h = object->bounds.h;

          object->Draw( object, &bounds, surface );

          if (bounds_debug) {
               surface->SetDrawingFlags( surface, DSDRAW_NOFX );
               surface->SetColor( surface, 0xff, 0x66, 0x11, 0xff );
               surface->DrawRectangle( surface, bounds.x, bounds.y, bounds.w, bounds.h );
          }

          if (object->children)
               DrawObjects( object->children, x + object->position.x, y + object->position.y, surface );
     }
}

static void
DrawScene( QuipScene *scene )
{
     D_MAGIC_ASSERT( scene, QuipScene );
     D_ASSERT( scene->output != NULL );

     DrawObjects( scene->objects, 0, 0, scene->output->surface );
}

static void
DrawSceneOn( QuipScene *scene, QuipOutput *output )
{
     D_MAGIC_ASSERT( scene, QuipScene );

     DrawObjects( scene->objects, 0, 0, output->surface );
}

/**********************************************************************************************************************/

static void
SceneAddUpdate( QuipScene *scene, int x, int y, int w, int h )
{
     DFBRegion region = { x, y, x + w - 1, y + h - 1 };

     D_DEBUG_AT( Quip_Update, "%s( %d,%d - %dx%d )\n", __FUNCTION__, x, y, w, h );

     D_MAGIC_ASSERT( scene, QuipScene );
     DFB_REGION_ASSERT( &region );

     if (!dfb_region_intersect( &region, 0, 0, scene->size.w, scene->size.h ))
          return;

     dfb_updates_add( &scene->updates, &region );
}

#if 0
static void
SceneAddUpdateRect( QuipScene *scene, const DFBRectangle *rect )
{
     DFBRegion region;

     D_MAGIC_ASSERT( scene, QuipScene );
     DFB_RECTANGLE_ASSERT( rect );

     region = DFB_REGION_INIT_FROM_RECTANGLE( rect );

     if (!dfb_region_intersect( &region, 0, 0, scene->size.w, scene->size.h ))
          return;

     dfb_updates_add( &scene->updates, &region );
}
#endif

static void
SceneAddUpdateObject( QuipScene *scene, QuipObject *object, int dx, int dy )
{
     DFBRegion region;

     D_MAGIC_ASSERT( scene, QuipScene );
     D_MAGIC_ASSERT( object, QuipObject );

     region = DFB_REGION_INIT_FROM_RECTANGLE( &object->bounds );

     dfb_region_translate( &region, dx, dy );

     if (!dfb_region_intersect( &region, 0, 0, scene->size.w, scene->size.h ))
          return;

     D_DEBUG_AT( Quip_Update, "%s( %d,%d - %dx%d )\n", __FUNCTION__,
                 DFB_RECTANGLE_VALS_FROM_REGION( &region ) );

     dfb_updates_add( &scene->updates, &region );
}

#if 0
static void
SceneAddUpdateRegion( QuipScene *scene, const DFBRegion *region )
{
     DFBRegion clipped;

     D_MAGIC_ASSERT( scene, QuipScene );
     DFB_REGION_ASSERT( region );

     clipped = *region;

     if (!dfb_region_intersect( &clipped, 0, 0, scene->size.w, scene->size.h ))
          return;

     dfb_updates_add( &scene->updates, &clipped );
}
#endif

static void
SceneProcessUpdates( QuipScene *scene )
{
     const DFBRegion  *regions;
     int               num_regions;
     int               i, n, d;
     int               total, bounding;
     QuipOutput       *output;
     IDirectFBSurface *surface;
     IDirectFBWindow  *window;

     D_MAGIC_ASSERT( scene, QuipScene );
     D_ASSERT( scene->output != NULL );

     if (!scene->updates.num_regions)
          return;

     dfb_updates_stat( &scene->updates, &total, &bounding );

     n = scene->updates.max_regions - scene->updates.num_regions + 1;
     d = n + 1;

     /* FIXME: depend on buffer mode, hw accel etc. */
     /*if (total > tier->size.w * tier->size.h * 3 / 5) {
          DFBRegion region = { 0, 0, tier->size.w - 1, tier->size.h - 1 };

          repaint_tier( sawman, tier, &region, 1, flags );
     }
     else*/
     if (scene->updates.num_regions < 2 || total < bounding * n / d) {
          regions     = scene->updates.regions;
          num_regions = scene->updates.num_regions;
     }
     else {
          regions     = &scene->updates.bounding;
          num_regions = 1;
     }

     dfb_updates_reset( &scene->updates );

     output  = scene->output;
     surface = output->surface;
     window  = output->window;

     for (i=0; i<num_regions; i++) {
          surface->SetClip( surface, &regions[i] );

          if (scene == quip_overlay)
               surface->Clear( surface, 0, 0, 0, 0 );

          DrawScene( scene );

          if (scene == quip_overlay && show_timefps) {
               //char buf[32];

               surface->SetDrawingFlags( surface, DSDRAW_NOFX );
               surface->SetColor( surface, 0x20, 0x70, 0x80, 0xff );

               //snprintf( buf, sizeof(buf), "%ld:%02ld.%03ld", quip_time / 60000, (quip_time / 1000) % 60, quip_time % 1000 );
               //surface->DrawString( surface, buf, -1, 20, 10, DSTF_TOPLEFT );

               surface->DrawString( surface, quip_fps.fps_string, -1, scene->size.w-20, 10, DSTF_TOPRIGHT );

               if (quip_cx || quip_cy) {
                    surface->SetBlittingFlags( surface, DSBLIT_NOFX );

                    surface->Blit( surface, cursor, NULL, quip_cx-12, quip_cy-12 );
               }
          }
     }

     for (i=0; i<num_regions; i++)
          surface->Flip( surface, &regions[i], DSFLIP_NONE );

     window->SetOpacity( window, 0xff );
}

static void
SceneAddObject( QuipScene       *scene,
                QuipObject      *object,
                int              come,
                int              stay,
                int              go,
                QuipObjectFlags  flags,
                QuipObject      *parent )
{
     int dx, dy;

     D_MAGIC_ASSERT( scene, QuipScene );
     D_MAGIC_ASSERT( object, QuipObject );

     object->scene    = scene;
     object->flags    = flags;
     object->stay     = stay;

     object->t_arrive = come;
     object->t_leave  = come + stay;
     object->t_gone   = come + stay + go;

     if (!come)
          object->state = QOS_STAY;

     if (parent) {
          D_MAGIC_ASSERT( parent, QuipObject );

          object->parent = parent;

          direct_list_append( &parent->children, &object->link );
     }
     else
          direct_list_append( &scene->objects, &object->link );

     ObjectOffset( object, &dx, &dy );
     SceneAddUpdateObject( scene, object, dx, dy );
}

static void
SceneRemoveObject( QuipScene *scene, QuipObject *object )
{
     int dx, dy;

     D_MAGIC_ASSERT( scene, QuipScene );
     D_MAGIC_ASSERT( object, QuipObject );

     ObjectOffset( object, &dx, &dy );
     SceneAddUpdateObject( scene, object, dx, dy );

     if (object->parent) {
          QuipObject *parent = object->parent;

          D_MAGIC_ASSERT( parent, QuipObject );

          direct_list_remove( &parent->children, &object->link );
     }
     else
          direct_list_remove( &scene->objects, &object->link );
}

static QuipScene *
CreateScene( const char *name, int width, int height, QuipOutput *output )
{
     QuipScene *scene;
     DFBRegion  region = { 0, 0, output->width - 1, output->height - 1 };

     D_ASSERT( name != NULL );

     scene = D_CALLOC( 1, sizeof(QuipScene) );
     if (!scene) {
          D_OOM();
          return NULL;
     }

     scene->output = output;

     scene->size.w = output->width;
     scene->size.h = output->height;

     scene->coords.w = (width  > 0) ? width  : scene->size.w;
     scene->coords.h = (height > 0) ? height : scene->size.h;

     scene->name = D_STRDUP( name );
     if (!scene->name) {
          D_OOM();
          D_FREE( scene );
          return NULL;
     }

     dfb_updates_init( &scene->updates, scene->update_regions, D_ARRAY_SIZE(scene->update_regions) );

     dfb_updates_add( &scene->updates, &region );

     D_MAGIC_SET( scene, QuipScene );

     return scene;
}

/**********************************************************************************************************************/

static inline void
LinearZoom( const DFBPoint     *position,
            const DFBPoint     *center,
            const DFBDimension *size,
            int                 value,
            int                 max,
            DFBRectangle       *ret_bounds )
{
     ret_bounds->x = (position->x * value + center->x * (max - value)) / max;
     ret_bounds->y = (position->y * value + center->y * (max - value)) / max;
     ret_bounds->w = (size->w     * value +             (max - value)) / max;
     ret_bounds->h = (size->h     * value +             (max - value)) / max;

     if (ret_bounds->w < 1)
          ret_bounds->w = 1;

     if (ret_bounds->h < 1)
          ret_bounds->h = 1;
}

static inline void
LinearMove( const DFBRectangle *from,
            const DFBRectangle *to,
            int                 value,
            int                 max,
            DFBRectangle       *ret_bounds )
{
     ret_bounds->x = (to->x * value + from->x * (max - value)) / max;
     ret_bounds->y = (to->y * value + from->y * (max - value)) / max;
     ret_bounds->w = (to->w * value + from->w * (max - value)) / max;
     ret_bounds->h = (to->h * value + from->h * (max - value)) / max;
}

static inline void
QuadMove( const DFBRectangle *from,
          const DFBRectangle *to,
          int                 value,
          int                 max,
          DFBRectangle       *ret_bounds )
{
     int qmax = max * max;
     int qval = value * value;

     ret_bounds->x = (to->x * qval + from->x * (qmax - qval)) / qmax;
     ret_bounds->y = (to->y * qval + from->y * (qmax - qval)) / qmax;
     ret_bounds->w = (to->w * qval + from->w * (qmax - qval)) / qmax;
     ret_bounds->h = (to->h * qval + from->h * (qmax - qval)) / qmax;
}

static inline void
QuadMoveInv( const DFBRectangle *from,
             const DFBRectangle *to,
             int                 value,
             int                 max,
             DFBRectangle       *ret_bounds )
{
     int qmax = max * max;
     int qval = (max - value) * (max - value);

     ret_bounds->x = (from->x * qval + to->x * (qmax - qval)) / qmax;
     ret_bounds->y = (from->y * qval + to->y * (qmax - qval)) / qmax;
     ret_bounds->w = (from->w * qval + to->w * (qmax - qval)) / qmax;
     ret_bounds->h = (from->h * qval + to->h * (qmax - qval)) / qmax;
}

static inline void
CubeMove( const DFBRectangle *from,
          const DFBRectangle *to,
          int                 value,
          int                 max,
          DFBRectangle       *ret_bounds )
{
     int qmax = max * max * max;
     int qval = value * value * value;

     ret_bounds->x = (to->x * qval + from->x * (qmax - qval)) / qmax;
     ret_bounds->y = (to->y * qval + from->y * (qmax - qval)) / qmax;
     ret_bounds->w = (to->w * qval + from->w * (qmax - qval)) / qmax;
     ret_bounds->h = (to->h * qval + from->h * (qmax - qval)) / qmax;
}

static inline void
CubeMoveInv( const DFBRectangle *from,
             const DFBRectangle *to,
             int                 value,
             int                 max,
             DFBRectangle       *ret_bounds )
{
     long long qmax = max * max * max;
     long long qval = (max - value) * (max - value) * (max - value);

     ret_bounds->x = (from->x * qval + to->x * (qmax - qval)) / qmax;
     ret_bounds->y = (from->y * qval + to->y * (qmax - qval)) / qmax;
     ret_bounds->w = (from->w * qval + to->w * (qmax - qval)) / qmax;
     ret_bounds->h = (from->h * qval + to->h * (qmax - qval)) / qmax;
}

static inline void
CubeInterInv( int  from,
              int  to,
              int  value,
              int  max,
              int *ret_value )
{
     long long qmax = max * max * max;
     long long qval = (max - value) * (max - value) * (max - value);

     *ret_value = (from * qval + to * (qmax - qval)) / qmax;
}

/**********************************************************************************************************************/

static bool
TickObjects( DirectLink *objects, int ms, int x, int y )
{
     DirectLink *l;
     QuipObject *object;
     bool        active = false;

     D_DEBUG_AT( Quip_Object, "%s( %p, %d ms, %d,%d )\n", __FUNCTION__, objects, ms, x, y );

     direct_list_foreach_safe (object, l, objects) {
          D_MAGIC_ASSERT( object, QuipObject );

          if (!(object->flags & QOF_REALIZED)) {
               if (object->Realize && !object->Realize( object )) {
                    D_ERROR( "Quip: Failed to realize object!\n" );
                    quip_run = false;
                    return false;
               }

               object->flags |= QOF_REALIZED;
          }

          object->age += ms;

          if (object->Tick( object, ms ))
               active = true;

          switch (object->state) {
               case QOS_COME:
                    SceneAddUpdateObject( object->scene, object, x, y );

                    if (object->age >= object->t_arrive) {
                         object->state    = QOS_STAY;
                         object->opacity  = 255;
                         object->bounds.x = object->position.x;
                         object->bounds.y = object->position.y;
                         object->bounds.w = object->size.w;
                         object->bounds.h = object->size.h;

                         SceneAddUpdateObject( object->scene, object, x, y );

                         active = true;
                         break;
                    }

                    if (object->flags & QOF_FADEIN)
                         object->opacity = 255 * object->age / object->t_arrive;

                    if (object->flags & QOF_ZOOMIN)
                         LinearZoom( &object->position, &object->center, &object->size,
                                     object->age, object->t_arrive, &object->bounds );

                    SceneAddUpdateObject( object->scene, object, x, y );

                    active = true;
                    break;

               case QOS_STAY:
                    if (object->stay >= 0 && object->age >= object->t_leave) {
                         object->state = QOS_GO;
                         active = true;
                    }
                    break;

               case QOS_GO:
                    if (object->age >= object->t_gone) {
                         SceneRemoveObject( object->scene, object );
                         object->Destroy( object );
                         break;
                    }

                    if (object->flags & QOF_FADEOUT)
                         object->opacity = 255 * (object->t_gone - object->age) / (object->t_gone - object->t_leave);

                    if (object->flags & QOF_ZOOMOUT) {
                         SceneAddUpdateObject( object->scene, object, x, y );

                         LinearZoom( &object->position, &object->center, &object->size,
                                     object->t_gone - object->age, object->t_gone - object->t_leave, &object->bounds );
                    }

                    SceneAddUpdateObject( object->scene, object, x, y );

                    active = true;
                    break;
          }

          if (object->fade_active) {
               int opacity = object->opacity;
               int diff    = quip_time - object->fade_start;

               if (diff >= object->fade_time) {
                    object->fade_active = false;
                    object->opacity     = object->fade_target;
               }
               else
                    CubeInterInv( object->fade_from, object->fade_target, diff, object->fade_time, &object->opacity );

               if (opacity != object->opacity)
                    SceneAddUpdateObject( object->scene, object, x, y );

               active = true;
          }

          if (object->go_active) {
               int diff = quip_time - object->go_start;

               SceneAddUpdateObject( object->scene, object, x, y );

               if (diff >= object->go_time) {
                    object->go_active = false;
                    object->bounds    = object->go_target;

                    BenchStop( &thumb_bench, "Thumbnail" );
                    BenchStop( &slide_bench, "Slidebar" );
               }
               else
                    CubeMoveInv( &object->go_from, &object->go_target, diff, object->go_time, &object->bounds );

               SceneAddUpdateObject( object->scene, object, x, y );

               active = true;
          }

          if (object->children) {
               if (TickObjects( object->children, ms, x + object->position.x, y + object->position.y ))
                    active = true;
          }
     }

     D_DEBUG_AT( Quip_Object, "  -> %sactive\n", active ? "" : "NOT " );

     return active;
}

static bool
TickScene( QuipScene *scene, int diff_ms )
{
     D_MAGIC_ASSERT( scene, QuipScene );

     return TickObjects( scene->objects, diff_ms, 0, 0 );
}

/**********************************************************************************************************************/

static bool
ImageTick( QuipObject *object, int ms )
{
     D_MAGIC_ASSERT( object, QuipObject );

     return false;
}

static void
ImageDraw( QuipObject *object, const DFBRectangle *bounds, IDirectFBSurface *surface )
{
     QuipImage               *image = (QuipImage*) object;
     DFBSurfaceBlittingFlags  flags = DSBLIT_NOFX;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( image->surface != NULL );

     if (object->opacity != 0xff) {
          surface->SetColor( surface, 0xff, 0xff, 0xff, object->opacity );

          flags |= DSBLIT_BLEND_COLORALPHA;
     }

     if (image->image_desc.caps & DICAPS_ALPHACHANNEL)
          flags |= DSBLIT_BLEND_ALPHACHANNEL;

     surface->SetBlittingFlags( surface, flags );

     //direct_log_printf( NULL, "%dx%d -> %dx%d\n", object->size.w, object->size.h, bounds->w, bounds->h );

     if (object->size.w == bounds->w && object->size.h == bounds->h)
          surface->Blit( surface, image->surface, NULL, bounds->x, bounds->y );
     else
          surface->StretchBlit( surface, image->surface, NULL, bounds );
}

static void
DestroyImage( QuipObject *object )
{
     QuipImage *image = (QuipImage*) object;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( image->surface != NULL );

     BenchStop( &icon_bench, "Icons" );

     image->surface->Release( image->surface );

     D_MAGIC_CLEAR( object );

     D_FREE( image );
}

static QuipImage *
CreateImage( QuipScene *scene, const char *filename, int x, int y )
{
     DFBResult  ret;
     QuipImage *image;
     char       buf[256];
     int        width;
     int        height;

     D_ASSERT( filename != NULL );

     image = D_CALLOC( 1, sizeof(QuipImage) );
     if (!image) {
          D_OOM();
          return NULL;
     }

     snprintf( buf, sizeof(buf), "%s%s", quip_path, filename );

     ret = load_image( dfb, buf, scene->output->format,
                       &image->surface, &width, &height, &image->image_desc, scene );
     if (ret) {
          D_DERROR( ret, "load_image( '%s' )!\n", buf );
          D_FREE( image );
          return NULL;
     }

     image->object.Tick    = ImageTick;
     image->object.Draw    = ImageDraw;
     image->object.Destroy = DestroyImage;

     x = x * scene->size.w / scene->coords.w;
     y = y * scene->size.h / scene->coords.h;

     InitObject( &image->object, x, y, width, height, QOT_IMAGE );

     return image;
}

/**********************************************************************************************************************/

static void
Trapezoidalization( IDirectFBSurface *surface )
{
     DFBResult              ret;
     void                  *data;
     int                    pitch;
     int                    x, y, w, h;
     DFBSurfacePixelFormat  format;
     long long              start;

     surface->GetSize( surface, &w, &h );
     surface->GetPixelFormat( surface, &format );

     ret = surface->Lock( surface, DSLF_READ | DSLF_WRITE, &data, &pitch );
     if (ret) {
          D_DERROR( ret, "Quip: Could not lock surface for trapezoidalization!\n" );
          return;
     }

     start = direct_clock_get_millis();

     switch (DFB_BYTES_PER_PIXEL(format)) {
          case 2:
               /* Line by line... */
               for (y=0; y<h; y++) {
                    /* Calculate border (up to 10% on each side). */
                    int border = w * (h-y) / (10 * h);

                    /* Calculate scaling factor. */
                    int  n, f = (w << 16) / (w - border * 2);

                    u16  line[w];
                    u16 *dst = data;

                    /* Fetch line. */
                    memcpy( line, data, w * 2 );

                    /* Clear borders. */
                    for (x=0; x<border; x++)
                         dst[x] = 0;
                    for (x=w-border; x<w; x++)
                         dst[x] = 0;

                    /* Scale line. */
                    for (x=border, n=0; x<w-border; x++, n+=f)
                         dst[x] = line[n>>16];

                    data += pitch;
               }
               break;
     }

     printf( "Trapezoidalization: %lld ms (%dx%d)\n", direct_clock_get_millis() - start, w, h );

     surface->Unlock( surface );
}

static bool
SlidebarRealize( QuipObject *object )
{
     QuipScene        *scene;
     QuipSlidebar     *slidebar = (QuipSlidebar*) object;
     QuipOutput       *output   = &slidebar->output;
     IDirectFBSurface *surface;

     D_MAGIC_ASSERT( object, QuipObject );

     if (!slidebar->scene_name)
          return true;

     direct_list_foreach (scene, quip_scenes) {
          if (!strcmp( scene->name, slidebar->scene_name )) {
               slidebar->scene = scene;
               break;
          }
     }

     if (!slidebar->scene) {
          D_ERROR( "QuipSlidebar: Scene with name '%s' not found!\n", slidebar->scene_name );
          return false;
     }

     surface = slidebar->surface;

     /* Query size and format of the surface. */
     surface->GetSize( surface, &output->width, &output->height );
     surface->GetPixelFormat( surface, &output->format );

     /* Set the font. */
     CHECK( surface->SetFont( surface, font ) );

     output->window  = NULL;
     output->surface = surface;

     TickScene( slidebar->scene, 0 );
     DrawSceneOn( slidebar->scene, &slidebar->output );

     Trapezoidalization( surface );

     return true;
}

static bool
SlidebarTick( QuipObject *object, int ms )
{
     D_MAGIC_ASSERT( object, QuipObject );

     return false;
}

static bool
SlidebarMotion( QuipObject *object, int x, int y )
{
     DFBRectangle  bounds;
     QuipScene    *scene;
     QuipSlidebar *slidebar = (QuipSlidebar*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     scene = object->scene;
     D_MAGIC_ASSERT( scene, QuipScene );

     if (slidebar->active)
          return true;

     bounds = object->bounds;

     switch (slidebar->edge) {
          case QSBE_LEFT:
               bounds.x = 0;
               break;

          case QSBE_TOP:
               bounds.y = 0;
               break;

          case QSBE_RIGHT:
               bounds.x = scene->size.w - slidebar->object.size.w;
               break;

          case QSBE_BOTTOM:
               bounds.y = scene->size.h - slidebar->object.size.h;
               break;

          default:
               D_BUG( "unexpected edge" );
     }

     slidebar->active = true;

     BenchStart( &slide_bench );

     FadeObject( object, 255, 500 );

     return ReshapeObject( object, &bounds, 500 );
}

static bool
SlidebarLeave( QuipObject *object )
{
     DFBRectangle  bounds;
     QuipSlidebar *slidebar = (QuipSlidebar*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     bounds = object->bounds;

     bounds.x = slidebar->object.position.x;
     bounds.y = slidebar->object.position.y;

     slidebar->active = false;

     BenchStart( &slide_bench );

     FadeObject( object, 160, 700 );

     return ReshapeObject( object, &bounds, 700 );
}

static bool
SlidebarPress( QuipObject *object, DFBInputDeviceButtonIdentifier button )
{
     QuipSlidebar *slidebar = (QuipSlidebar*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     if (slidebar->scene) {
          quip_scene = slidebar->scene;
          SceneAddUpdate( quip_scene, 0, 0, quip_scene->size.w, quip_scene->size.h );
     }

     return true;
}

static void
SlidebarDraw( QuipObject *object, const DFBRectangle *bounds, IDirectFBSurface *surface )
{
     QuipSlidebar            *slidebar = (QuipSlidebar*) object;
     DFBSurfaceBlittingFlags  flags    = DSBLIT_NOFX;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( slidebar->surface != NULL );

     if (object->opacity != 0xff) {
          surface->SetColor( surface, 0xff, 0xff, 0xff, object->opacity );
          flags |= DSBLIT_BLEND_COLORALPHA;
     }

     if (slidebar->image_desc.caps & DICAPS_ALPHACHANNEL)
          flags |= DSBLIT_BLEND_ALPHACHANNEL;

     surface->SetBlittingFlags( surface, flags );

     if (object->size.w == bounds->w && object->size.h == bounds->h)
          surface->Blit( surface, slidebar->surface, NULL, bounds->x, bounds->y );
     else
          surface->StretchBlit( surface, slidebar->surface, NULL, bounds );
}

static void
DestroySlidebar( QuipObject *object )
{
     QuipSlidebar *slidebar = (QuipSlidebar*) object;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( slidebar->surface != NULL );

     slidebar->surface->Release( slidebar->surface );

     if (slidebar->scene)
          D_FREE( slidebar->scene );

     D_MAGIC_CLEAR( object );

     D_FREE( slidebar );
}

static QuipSlidebar *
CreateSlidebar( QuipScene *scene, const char *filename, QuipSlidebarEdge edge, const char *target )
{
     DFBResult     ret;
     QuipSlidebar *slidebar;
     char          buf[256];
     int           x = 0;
     int           y = 0;
     int           width;
     int           height;

     slidebar = D_CALLOC( 1, sizeof(QuipSlidebar) );
     if (!slidebar) {
          D_OOM();
          return NULL;
     }

     slidebar->edge = edge;

     if (target) {
          slidebar->scene_name = D_STRDUP( target );
          if (!slidebar->scene_name) {
               D_OOM();
               D_FREE( slidebar );
               return NULL;
          }
     }

     if (filename) {
          snprintf( buf, sizeof(buf), "%s%s", quip_path, filename );

          ret = load_image( dfb, buf, scene->output->format,
                            &slidebar->surface, &width, &height, &slidebar->image_desc, scene );
          if (ret) {
               D_DERROR( ret, "load_image( '%s' )!\n", buf );
               if (slidebar->scene)
                    D_FREE( slidebar->scene );
               D_FREE( slidebar );
               return NULL;
          }
     }
     else {
          DFBSurfaceDescription desc;

          width  = scene->output->width;
          height = scene->output->height / 2;

          desc.flags  = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
          desc.width  = width;
          desc.height = height;

          if (DFB_PIXELFORMAT_HAS_ALPHA(scene->output->format))
               desc.pixelformat = scene->output->format;
          else
               desc.pixelformat = DSPF_ARGB;

          ret = dfb->CreateSurface( dfb, &desc, &slidebar->surface );
          if (ret) {
               D_DERROR( ret, "QuipSlidebar: Could not create %dx%d %s surface!\n",
                         width, height, dfb_pixelformat_name(desc.pixelformat) );
               if (slidebar->scene)
                    D_FREE( slidebar->scene );
               D_FREE( slidebar );
               return NULL;
          }

          slidebar->image_desc.caps = DICAPS_ALPHACHANNEL;
     }

     switch (edge) {
          case QSBE_LEFT:
               x = -width * 70 / 100;
               y = (scene->size.h - height) / 2;
               break;

          case QSBE_TOP:
               x = (scene->size.w - width) / 2;
               y = -height * 70 / 100;
               break;

          case QSBE_RIGHT:
               x = scene->size.w - width * 30 / 100;
               y = (scene->size.h - height) / 2;
               break;

          case QSBE_BOTTOM:
               x = (scene->size.w - width) / 2;
               y = scene->size.h - height * 30 / 100;
               break;

          default:
               D_BUG( "unexpected edge" );
     }

     slidebar->object.Realize = SlidebarRealize;
     slidebar->object.Tick    = SlidebarTick;
     slidebar->object.Motion  = SlidebarMotion;
     slidebar->object.Leave   = SlidebarLeave;
     slidebar->object.Press   = SlidebarPress;
     slidebar->object.Draw    = SlidebarDraw;
     slidebar->object.Destroy = DestroySlidebar;

     InitObject( &slidebar->object, x, y, width, height, QOT_SLIDEBAR );

     slidebar->object.opacity = 160;

     return slidebar;
}

/**********************************************************************************************************************/

static bool
ThumbnailTick( QuipObject *object, int ms )
{
     D_MAGIC_ASSERT( object, QuipObject );

     return false;
}

static bool
ThumbnailMotion( QuipObject *object, int x, int y )
{
     QuipThumbnail *thumbnail = (QuipThumbnail*) object;
     DFBRectangle   bounds;

     D_MAGIC_ASSERT( object, QuipObject );

     if (thumbnail->zoomed)
          return false;

     bounds.x = object->center.x - object->size.w;
     bounds.y = object->center.y - object->size.h;
     bounds.w = 2 * object->size.w;
     bounds.h = 2 * object->size.h;

     ObjectToFront( object );

     thumbnail->zoomed = true;

     BenchStart( &thumb_bench );

     return ReshapeObject( object, &bounds, 400 );
}

static bool
ThumbnailLeave( QuipObject *object )
{
     QuipThumbnail *thumbnail = (QuipThumbnail*) object;
     DFBRectangle   bounds;

     D_MAGIC_ASSERT( object, QuipObject );

     D_ASSUME( thumbnail->zoomed );

     bounds.x = object->position.x;
     bounds.y = object->position.y;
     bounds.w = object->size.w;
     bounds.h = object->size.h;

     thumbnail->zoomed = false;

     return ReshapeObject( object, &bounds, 600 );
}

static QuipImage *
AddThumbnailIcon( QuipThumbnail *thumbnail, const char *filename, int cx, int cy, int w, int h )
{
     QuipScene    *scene;
     QuipImage    *image;
     DFBRectangle  bounds;

     scene = thumbnail->object.scene;

     bounds.x = thumbnail->object.position.x + (thumbnail->object.size.w - w) / 2;
     bounds.y = thumbnail->object.position.y + (thumbnail->object.size.h - h) / 2;
     bounds.w = w;
     bounds.h = h;

     image = CreateImage( quip_overlay, filename,
                          bounds.x * scene->coords.w / scene->size.w,
                          bounds.y * scene->coords.h / scene->size.h );

     bounds.x += cx;
     bounds.y += cy;

     bounds.w = bounds.w * scene->size.w / scene->coords.w,
     bounds.h = bounds.h * scene->size.h / scene->coords.h;

     ReshapeObject( &image->object, &bounds, 500 );

     SceneAddObject( quip_overlay, &image->object,
                     500, -1, 0, QOF_FADEOUT, NULL );//&thumbnail->object );

     return image;
}

static bool
ThumbnailPress( QuipObject *object, DFBInputDeviceButtonIdentifier button )
{
     QuipThumbnail *thumbnail = (QuipThumbnail*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     if (thumbnail->pressed || icon_bench.start)
          return false;

     thumbnail->pressed = true;

     thumbnail->icons[0] = AddThumbnailIcon( thumbnail, "icon01.dfiff",    0, -105, 96, 96 );
     thumbnail->icons[1] = AddThumbnailIcon( thumbnail, "icon01.dfiff",   78,  -78, 96, 96 );
     thumbnail->icons[2] = AddThumbnailIcon( thumbnail, "icon01.dfiff",  105,    0, 96, 96 );
     thumbnail->icons[3] = AddThumbnailIcon( thumbnail, "icon01.dfiff",   78,   78, 96, 96 );
     thumbnail->icons[4] = AddThumbnailIcon( thumbnail, "icon01.dfiff",    0,  105, 96, 96 );
     thumbnail->icons[5] = AddThumbnailIcon( thumbnail, "icon01.dfiff",  -78,   78, 96, 96 );
     thumbnail->icons[6] = AddThumbnailIcon( thumbnail, "icon01.dfiff", -105,    0, 96, 96 );
     thumbnail->icons[7] = AddThumbnailIcon( thumbnail, "icon01.dfiff",  -78,  -78, 96, 96 );

     BenchStart( &icon_bench );

     return true;
}

static bool
ThumbnailUnpress( QuipObject *object, DFBInputDeviceButtonIdentifier button )
{
     int            i;
     QuipThumbnail *thumbnail = (QuipThumbnail*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     if (!thumbnail->pressed)
          return false;

     thumbnail->pressed = false;

     for (i=0; i<8; i++) {
          QuipImage *icon = thumbnail->icons[i];

          D_ASSUME( icon != NULL );

          if (!icon)
               continue;

          icon->object.stay = 0;

          icon->object.t_leave = icon->object.age;
          icon->object.t_gone  = icon->object.age + 1500;

          thumbnail->icons[i] = NULL;
     }

     return true;
}

static void
ThumbnailDraw( QuipObject *object, const DFBRectangle *bounds, IDirectFBSurface *surface )
{
     QuipThumbnail           *thumbnail = (QuipThumbnail*) object;
     DFBSurfaceBlittingFlags  flags    = DSBLIT_NOFX;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( thumbnail->surface != NULL );

     if (object->opacity != 0xff) {
          surface->SetColor( surface, 0xff, 0xff, 0xff, object->opacity );

          flags |= DSBLIT_BLEND_COLORALPHA;
     }

     surface->SetBlittingFlags( surface, flags );

/*     if (object->size.w == bounds->w && object->size.h == bounds->h)
          surface->Blit( surface, thumbnail->surface, NULL, bounds->x, bounds->y );
     else*/
          surface->StretchBlit( surface, thumbnail->surface, NULL, bounds );
}

static void
DestroyThumbnail( QuipObject *object )
{
     QuipThumbnail *thumbnail = (QuipThumbnail*) object;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( thumbnail->surface != NULL );

     thumbnail->surface->Release( thumbnail->surface );

     D_MAGIC_CLEAR( object );

     D_FREE( thumbnail );
}

static QuipThumbnail *
CreateThumbnail( QuipScene *scene, const char *filename, int x, int y, int w, int h )
{
     DFBResult      ret;
     QuipThumbnail *thumbnail;
     char           buf[256];
     int            width;
     int            height;

     D_ASSERT( filename != NULL );

     thumbnail = D_CALLOC( 1, sizeof(QuipThumbnail) );
     if (!thumbnail) {
          D_OOM();
          return NULL;
     }

     snprintf( buf, sizeof(buf), "%s%s", quip_path, filename );

     ret = load_image( dfb, buf, scene->output->format,
                       &thumbnail->surface, &width, &height, NULL, scene );
     if (ret) {
          D_DERROR( ret, "load_image( '%s' )!\n", buf );
          D_FREE( thumbnail );
          return NULL;
     }

     thumbnail->object.Tick    = ThumbnailTick;
     thumbnail->object.Motion  = ThumbnailMotion;
     thumbnail->object.Leave   = ThumbnailLeave;
     thumbnail->object.Press   = ThumbnailPress;
     thumbnail->object.Unpress = ThumbnailUnpress;
     thumbnail->object.Draw    = ThumbnailDraw;
     thumbnail->object.Destroy = DestroyThumbnail;

     x = x * scene->size.w / scene->coords.w;
     y = y * scene->size.h / scene->coords.h;

     if (w > 0)
          w = w * scene->size.w  / scene->coords.w;

     if (h > 0)
          h = h * scene->size.h / scene->coords.h;

     InitObject( &thumbnail->object, x, y, w < 0 ? width : w, h < 0 ? height : h, QOT_THUMBNAIL );

     return thumbnail;
}

/**********************************************************************************************************************/

static bool
TextTick( QuipObject *object, int ms )
{
     D_MAGIC_ASSERT( object, QuipObject );

     return false;
}

static void
TextDraw( QuipObject *object, const DFBRectangle *bounds, IDirectFBSurface *surface )
{
     Text *text = (Text*) object;

     D_MAGIC_ASSERT( object, QuipObject );

     if (object->opacity != 0xff)
          surface->SetDrawingFlags( surface, DSDRAW_BLEND );
     else
          surface->SetDrawingFlags( surface, DSDRAW_NOFX );

     surface->SetColor( surface, object->color.r, object->color.g, object->color.b, object->opacity );
     surface->DrawString( surface, text->message, -1, bounds->x, bounds->y, DSTF_TOPLEFT );
}

static void
DestroyText( QuipObject *object )
{
     Text *text = (Text*) object;

     D_MAGIC_ASSERT( object, QuipObject );
     D_ASSERT( text->message != NULL );

     D_FREE( text->message );

     D_MAGIC_CLEAR( object );

     D_FREE( text );
}

static Text *
CreateText( QuipScene *scene, const char *message, int x, int y )
{
     Text         *text;
     DFBRectangle  bounds;

     text = D_CALLOC( 1, sizeof(Text) );
     if (!text) {
          D_OOM();
          return NULL;
     }

     text->message = D_STRDUP( message );
     if (!text->message) {
          D_OOM();
          D_FREE( text );
          return NULL;
     }


     font->GetStringExtents( font, message, -1, NULL, &bounds );

     bounds.x += x;
     bounds.y += y + font_ascender;

     text->object.Tick    = TextTick;
     text->object.Draw    = TextDraw;
     text->object.Destroy = DestroyText;

     x = x * scene->size.w  / scene->coords.w;
     y = y * scene->size.h / scene->coords.h;

     InitObject( &text->object, x, y, bounds.w, bounds.h, QOT_TEXT );

     text->object.bounds = bounds;

     return text;
}

/**********************************************************************************************************************/

static QuipScene *
ReadQuipFile( const char *filename,
              QuipOutput *output )
{
     FILE      *file;
     char       path[256];
     char       line[256];
     char      *slash;
     QuipScene *scene = NULL;
     int        num   = 0;

     if (quip_scenes)
          D_UNIMPLEMENTED();

     file = fopen( filename, "r" );
     if (!file) {
          D_PERROR( "Could not open '%s'!\n", filename );
          return NULL;
     }

     slash = strrchr( filename, '/' );
     if (slash)
          snprintf( path, MIN( sizeof(path), slash - filename + 2 ), filename );
     else
          path[0] = 0;

     quip_path = D_STRDUP( path );

     /* Read line by line... */
     while (fgets( line, sizeof(line), file )) {
          int   len = strlen( line );
          char *next, *end, *command, *keyword, *keyval, *data = NULL, *name = NULL;
          int   come = 0, stay = -1, go = 0, x = 0, y = 0, w = -1, h = -1, n;
          int              value = 0;
          QuipObjectFlags  flags = 0;
          QuipSlidebarEdge edge  = QSBE_LEFT;

          num++;

          /* Ignore empty lines. */
          while (len > 0 && line[len-1] == '\n')
               line[--len] = 0;

          if (!len)
               continue;

          /* Get the command. */
          command = strtok_r( line, DELIM, &next );
          if (!command || command[0] == '#')
               continue;

          /* Parse all keyword / value pairs. */
          while ((keyword = strtok_r( NULL, DELIM, &next )) != NULL) {
               D_ASSERT( next != NULL );

               next += strspn( next, DELIM );

               /* Parse quoted text ourself. */
               if (next[0] == '"') {
                    len = strlen( ++next );

                    keyval = next;

                    for (n=0; n<len; n++) {
                         if (next[n] == '"') {
                              next[n] = 0;

                              next += n + 1;
                         }
                    }
               }
               else {
                    /* Just take the next token as the value. */
                    keyval = strtok_r( NULL, DELIM, &next );
                    if (!keyval) {
                         D_ERROR( "%s(): Keyword '%s' without value!\n", __FUNCTION__, keyword );
                         break;
                    }
               }

               /* Convert to integer for various (not all) cases below. */
               value = (keyval[0] == '-') ? -strtoul( keyval+1, &end, 10 ) : strtoul( keyval, &end, 10 );

               /* Always handle all keywords for now.
                * FIXME: Should split code for different commands?
                */
               if (!strcmp( keyword, "fadezoom" )) {             /** image, text **/
                    if (stay) {
                         flags |= QOF_FADEOUT | QOF_ZOOMOUT;
                         go = value;
                    }
                    else {
                         flags |= QOF_FADEIN | QOF_ZOOMIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "fade" )) {
                    if (stay) {
                         flags |= QOF_FADEOUT;
                         go = value;
                    }
                    else {
                         flags |= QOF_FADEIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "zoom" )) {
                    if (stay) {
                         flags |= QOF_ZOOMOUT;
                         go = value;
                    }
                    else {
                         flags |= QOF_ZOOMIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "time" )) {
                    stay = value;
               }
               else if (!strcmp( keyword, "pos" )) {
                    x = value;
                    y = strtoul( end+1, &end, 10 );
                    strtok_r( NULL, DELIM, &next );
               }
               else if (!strcmp( keyword, "size" ) ||
                        !strcmp( keyword, "coords" ))
               {
                    w = value;
                    h = strtoul( end+1, &end, 10 );
                    strtok_r( NULL, DELIM, &next );
               }
               else if (!strcmp( keyword, "rgb" )) {             /** color **/
                    value = strtoul( keyval, &end, 16 );
               }
               else if (!strcmp( keyword, "edge" )) {
                    if (!strcmp( keyval, "left" ))
                         edge = QSBE_LEFT;
                    else if (!strcmp( keyval, "top" ))
                         edge = QSBE_TOP;
                    else if (!strcmp( keyval, "right" ))
                         edge = QSBE_RIGHT;
                    else if (!strcmp( keyval, "bottom" ))
                         edge = QSBE_BOTTOM;
                    else
                         D_WARN( "invalid edge '%s' at line %d", keyval, num );
               }
               else if (!strcmp( keyword, "scene" ) ||
                        !strcmp( keyword, "name" ))
               {
                    name = keyval;
               }
               else if (!strcmp( keyword, "data" ) ||
                        !strcmp( keyword, "image" ))
               {
                    data = keyval;
               }
          }

          /* Now run the command with the collected information. */
          if (!strcmp( command, "scene" )) {
               scene = CreateScene( name, w, h, output );

               direct_list_append( &quip_scenes, &scene->link );
          }
          else if (!strcmp( command, "image" )) {
               QuipImage *image = CreateImage( scene, data, x, y );

               SceneAddObject( scene, &image->object, come, stay, go, flags, NULL );
          }
          else if (!strcmp( command, "slidebar" )) {
               QuipSlidebar *slidebar = CreateSlidebar( scene, data, edge, name );

               SceneAddObject( scene, &slidebar->object, come, stay, go, flags, NULL );
          }
          else if (!strcmp( command, "thumbnail" )) {
               QuipThumbnail *thumbnail = CreateThumbnail( scene, data, x, y, w, h );

               SceneAddObject( scene, &thumbnail->object, come, stay, go, flags, NULL );
          }
          else if (!strcmp( command, "text" )) {
               Text *text = CreateText( scene, data, x, y );

               SceneAddObject( scene, &text->object, come, stay, go, flags, NULL );
          }
          else if (!strcmp( command, "color" )) {
               quip_color.r = value >> 16;
               quip_color.g = (value >> 8) & 0xff;
               quip_color.b = value & 0xff;
          }
     }

     return (QuipScene*) quip_scenes;
}

/**********************************************************************************************************************/

static bool
DispatchMotion( QuipScene *scene, int x, int y )
{
     QuipObject *object;

     if (scene->grabber) {
          object = scene->grabber;
          D_MAGIC_ASSERT( object, QuipObject );
     }
     else {
          object = SceneObjectAt( scene, &x, &y );
          D_MAGIC_ASSERT_IF( object, QuipObject );
     }

     if (scene->entered != object) {
          if (scene->entered && scene->entered->Leave)
               scene->entered->Leave( scene->entered );

          scene->entered = object;
     }

     if (object && object->Motion)
          return object->Motion( object, x, y );

     return false;
}

static bool
DispatchButton( QuipScene *scene, DFBInputDeviceButtonIdentifier button, bool press )
{
     QuipObject *object;

     if (scene->grabber) {
          object = scene->grabber;
          D_MAGIC_ASSERT( object, QuipObject );
     }
     else if (scene->entered) {
          object = scene->entered;
          D_MAGIC_ASSERT( object, QuipObject );
     }
     else
          return false;

     if (press) {
          scene->grabber = object;

          if (object->Press)
               return object->Press( object, button );
     }
     else {
          scene->grabber = NULL;

          if (object->Unpress)
               return object->Unpress( object, button );
     }

     return false;
}

/**********************************************************************************************************************/

static void
InitOutput( QuipOutput            *output,
            int                    width,
            int                    height,
            DFBSurfacePixelFormat  format,
            bool                   overlay )
{
     DFBWindowDescription  desc;
     IDirectFBWindow      *window;
     IDirectFBSurface     *surface;

     D_ASSERT( output != NULL );

     /* Fill surface description. */
     desc.flags       = DWDESC_WIDTH | DWDESC_HEIGHT   | DWDESC_PIXELFORMAT |
                        DWDESC_CAPS  | DWDESC_STACKING | DWDESC_OPTIONS;
     desc.width       = width;
     desc.height      = height;
     desc.pixelformat = format;

     if (overlay) {
          desc.caps     = DWCAPS_ALPHACHANNEL;
          desc.stacking = DWSC_UPPER;
          desc.options  = DWOP_ALPHACHANNEL | DWOP_GHOST;
     }
     else {
          desc.caps     = DWCAPS_NONE;
          desc.stacking = DWSC_MIDDLE;
          desc.options  = DWOP_NONE;
     }

     /* Create the window. */
     CHECK( layer->CreateWindow( layer, &desc, &window ) );

     /* Get its surface. */
     CHECK( window->GetSurface( window, &surface ) );

     /* Query size and format of the surface. */
     surface->GetSize( surface, &output->width, &output->height );
     surface->GetPixelFormat( surface, &output->format );

     /* Set the font. */
     CHECK( surface->SetFont( surface, font ) );

     output->window  = window;
     output->surface = surface;
}

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     DFBFontDescription fdsc;
     QuipOutput         main_out;
     QuipOutput         overlay;

     /* Initialize DirectFB including command line parsing. */
     CHECK( DirectFBInit( &argc, &argv ) );

     /* Create the super interface. */
     CHECK( DirectFBCreate( &dfb ) );

     /* Get the primary layer. */
     CHECK( dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ) );

     /* Fill font description. */
     fdsc.flags  = /*DFDESC_WIDTH |*/ DFDESC_HEIGHT;
     fdsc.width  = 40;
     fdsc.height = 33;

     /* Create the font. */
     CHECK( dfb->CreateFont( dfb, "data/FreeSans.ttf", &fdsc, &font ) );

     font->GetAscender( font, &font_ascender );

     /* Create an event buffer for all keyboard events. */
     CHECK( dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS | DICAPS_AXES | DICAPS_BUTTONS, DFB_TRUE, &events ) );

     InitOutput( &main_out, 852, 480, DSPF_ARGB4444, false );

     InitOutput( &overlay, 852, 480, DSPF_ARGB4444, true );

     CHECK( load_image( dfb, "data/quip/cursor.dfiff", overlay.format, &cursor, NULL, NULL, NULL, NULL ));

#if 0
     surface->SetPorterDuff( surface, DSPD_SRC_OVER );
#endif

     /*
      * Read the QUIP file
      */
     quip_scene = ReadQuipFile( "data/quip/demo.quip", &main_out );

     /*
      * Create the overlay scene
      */
     quip_overlay = CreateScene( "Overlay", quip_scene->coords.w, quip_scene->coords.h, &overlay );

     /*
      * Run the main loop
      */
     quip_run  = true;
     quip_time = direct_clock_get_millis();

     fps_init( &quip_fps );

     while (quip_run) {
          DFBInputEvent event;
          bool          active = false;
          long long     now    = direct_clock_get_millis();
          long long     diff   = now - quip_time;
          int           ocx    = quip_cx;
          int           ocy    = quip_cy;

          quip_time = now;

          if (!quip_fps.frames)
               SceneAddUpdate( quip_overlay, quip_overlay->size.w - 120, 10, 100, 30 );

          SceneProcessUpdates( quip_scene );
          SceneProcessUpdates( quip_overlay );

          fps_count( &quip_fps, 400 );

          while (events->GetEvent( events, DFB_EVENT(&event) ) == DFB_OK) {
               switch (event.type) {
                    case DIET_AXISMOTION:
                         if (event.flags & DIEF_AXISABS) {
                              switch (event.axis) {
                                   case DIAI_X:
                                        quip_cx = event.axisabs;
                                        break;
                                   case DIAI_Y:
                                        quip_cy = event.axisabs;
                                        break;
                                   default:
                                        break;
                              }
                         }
                         if (event.flags & DIEF_AXISREL) {
                              switch (event.axis) {
                                   case DIAI_X:
                                        quip_cx += event.axisrel;
                                        break;
                                   case DIAI_Y:
                                        quip_cy += event.axisrel;
                                        break;
                                   default:
                                        break;
                              }
                         }
                         break;

                    case DIET_BUTTONPRESS:
                         active = DispatchButton( quip_scene, event.button, true ) || active;
                         break;

                    case DIET_BUTTONRELEASE:
                         active = DispatchButton( quip_scene, event.button, false ) || active;
                         break;

                    case DIET_KEYPRESS:
                         switch (event.key_symbol) {
                              case DIKS_ESCAPE:
                              case DIKS_POWER:
                              case DIKS_MENU:
                              case DIKS_BACK:
                                   quip_run = false;
                                   break;

                              case DIKS_1:
                                   show_timefps = !show_timefps;
                                   SceneAddUpdate( quip_overlay, quip_overlay->size.w - 120, 10, 100, 30 );
                                   break;

                              case DIKS_OK:
                              case DIKS_ENTER:
                                   active = DispatchButton( quip_scene, DIBI_LEFT, true ) || active;
                                   active = DispatchButton( quip_scene, DIBI_LEFT, false ) || active;
                                   break;

                              case DIKS_0:
                                   active = DispatchButton( quip_scene, DIBI_LEFT, true ) || active;
                                   break;

                              case DIKS_8:
                                   active = DispatchButton( quip_scene, DIBI_LEFT, false ) || active;
                                   break;

                              case DIKS_CURSOR_LEFT:
                                   quip_cx -= 42;
                                   break;

                              case DIKS_CURSOR_RIGHT:
                                   quip_cx += 42;
                                   break;

                              case DIKS_CURSOR_UP:
                                   quip_cy -= 42;
                                   break;

                              case DIKS_CURSOR_DOWN:
                                   quip_cy += 42;
                                   break;

                              default:
                                   break;
                         }

                    default:
                         break;
               }
          }

          if (quip_cx < 0)
               quip_cx = 0;
          else if (quip_cx >= quip_overlay->size.w)
               quip_cx = quip_overlay->size.w - 1;

          if (quip_cy < 0)
               quip_cy = 0;
          else if (quip_cy >= quip_overlay->size.h)
               quip_cy = quip_overlay->size.h - 1;

          if (ocx != quip_cx || ocy != quip_cy) {
               SceneAddUpdate( quip_overlay, ocx     - 12, ocy     - 12, 24, 24 );
               SceneAddUpdate( quip_overlay, quip_cx - 12, quip_cy - 12, 24, 24 );

               active = DispatchMotion( quip_scene, quip_cx, quip_cy ) || active;
          }

          active = TickScene( quip_scene,   diff ) || active;
          active = TickScene( quip_overlay, diff ) || active;

          if (active || quip_scene->updates.num_regions || quip_overlay->updates.num_regions)
               continue;

          if (events->WaitForEventWithTimeout( events, 0, 666 ) == DFB_TIMEOUT) {
               fps_init( &quip_fps );
               quip_time = direct_clock_get_millis();
          }
     }

     /* Shutdown. */
     events->Release( events );
     font->Release( font );
     layer->Release( layer );
     dfb->Release( dfb );

     return 0;
}

/**********************************************************************************************************************/

static DFBResult
load_image( IDirectFB              *dfb,
            const char             *filename,
            DFBSurfacePixelFormat   pixelformat,
            IDirectFBSurface      **surface,
            int                    *width,
            int                    *height,
            DFBImageDescription    *desc,
            const QuipScene        *scene )
{
     DFBResult               ret;
     DFBSurfaceDescription   dsc;
     DFBImageDescription     idsc;
     IDirectFBSurface       *image;
     IDirectFBImageProvider *provider;

     if (!surface)
          return DFB_INVARG;

     /* Create an image provider for loading the file */
     ret = dfb->CreateImageProvider (dfb, filename, &provider);
     if (ret) {
          fprintf (stderr,
                   "load_image: CreateImageProvider for '%s': %s\n",
                   filename, DirectFBErrorString (ret));
          return ret;
     }

     /* Retrieve a surface description for the image */
     ret = provider->GetSurfaceDescription (provider, &dsc);
     if (ret) {
          fprintf (stderr,
                   "load_image: GetSurfaceDescription for '%s': %s\n",
                   filename, DirectFBErrorString (ret));
          provider->Release (provider);
          return ret;
     }

     if (scene) {
          dsc.width  = dsc.width  * scene->size.w / scene->coords.w;
          dsc.height = dsc.height * scene->size.h / scene->coords.h;
     }

     provider->GetImageDescription( provider, &idsc );

     /* Use the specified pixelformat if the image's pixelformat is not ARGB */
     if (pixelformat != DSPF_UNKNOWN && (!(idsc.caps & DICAPS_ALPHACHANNEL) || DFB_PIXELFORMAT_HAS_ALPHA(pixelformat)))
          dsc.pixelformat = pixelformat;

#if 0
     if (DFB_PIXELFORMAT_HAS_ALPHA(dsc.pixelformat)) {
          if (!(dsc.flags & DSDESC_CAPS)) {
               dsc.flags |= DSDESC_CAPS;
               dsc.caps   = DSCAPS_NONE;
          }

          dsc.caps |= DSCAPS_PREMULTIPLIED;
     }
#endif

     /* Create a surface using the description */
     ret = dfb->CreateSurface (dfb, &dsc, &image);
     if (ret) {
          fprintf (stderr,
                   "load_image: CreateSurface %dx%d: %s\n",
                   dsc.width, dsc.height, DirectFBErrorString (ret));
          provider->Release (provider);
          return ret;
     }

     /* Render the image to the created surface */
     ret = provider->RenderTo (provider, image, NULL);
     if (ret) {
          fprintf (stderr,
                   "load_image: RenderTo for '%s': %s\n",
                   filename, DirectFBErrorString (ret));
          image->Release (image);
          provider->Release (provider);
          return ret;
     }

     /* Return surface */
     *surface = image;

     /* Return width? */
     if (width)
          *width = dsc.width;

     /* Return height? */
     if (height)
          *height  = dsc.height;

     /* Retrieve the image description? */
     if (desc)
          provider->GetImageDescription (provider, desc);

     /* Release the provider */
     provider->Release (provider);

     return DFB_OK;
}

