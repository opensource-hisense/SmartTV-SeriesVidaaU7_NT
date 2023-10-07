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
//#define DIRECT_ENABLE_DEBUG

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>
#include <directfb_util.h>

#include <direct/clock.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/util.h>

#include "util.h"

#define DELIM  " \t"

#define CHECK(x)                                  \
     do {                                         \
          DFBResult ret = (x);                    \
          if (ret)                                \
               DirectFBErrorFatal(#x,ret);        \
     } while (0)

/**********************************************************************************************************************/

typedef struct __ShopFloor_SceneObject SceneObject;

/**********************************************************************************************************************/

typedef enum {
     SOT_IMAGE,
     SOT_TEXT
} SceneObjectType;

typedef enum {
     SOF_FADEIN  = 0x01,
     SOF_ZOOMIN  = 0x02,
     SOF_FADEOUT = 0x04,
     SOF_ZOOMOUT = 0x08
} SceneObjectFlags;

typedef enum {
     SOS_COME,
     SOS_STAY,
     SOS_GO
} SceneObjectState;

struct __ShopFloor_SceneObject {
     DirectLink          link;

     int                 magic;

     SceneObjectType     type;     /* Image or Text? */
     SceneObjectType     flags;    /* Transition effects */
     SceneObjectState    state;    /* Come, Stay, Go */

     DFBPoint            position; /* main position (during stay) */
     DFBDimension        size;     /* actual size of the content */
     DFBPoint            center;   /* calculated center of the above */
     DFBColor            color;    /* object color, e.g. text color */

     DFBRectangle        bounds;   /* current position/size */
     int                 opacity;  /* current opacity */

     int                 age;      /* current age */

     int                 t_arrive; /* age when fade/zoom in is done */
     int                 t_leave;  /* age when fade/zoom out is started */
     int                 t_gone;   /* age when fade/zoom out is done */


     bool (*Tick)   ( SceneObject *object, int ms );
     void (*Draw)   ( SceneObject *object );
     void (*Destroy)( SceneObject *object );
};

/**********************************************************************************************************************/

typedef struct {
     SceneObject         object;

     IDirectFBSurface   *surface;
} Image;

typedef struct {
     SceneObject         object;

     char               *message;
} Text;

/**********************************************************************************************************************/

static IDirectFB            *dfb;
static IDirectFBFont        *font;
static IDirectFBSurface     *surface;
static IDirectFBEventBuffer *events;
static int                   font_ascender;
static bool                  bounds_debug;
static bool                  show_timefps;
static DirectLink           *scene;

/**********************************************************************************************************************/

static FILE     *sfa_file;
static char      sfa_path[256];
static int       sfa_wait;
static long      sfa_time;
static DFBColor  sfa_color = { 0xff, 0xee, 0xee, 0xee };

/**********************************************************************************************************************/

static void
InitObject( SceneObject     *object,
            int              x,
            int              y,
            int              w,
            int              h,
            SceneObjectType  type )
{
     D_MAGIC_ASSERT( object, SceneObject );

     object->position.x = x;
     object->position.y = y;
     object->size.w     = w;
     object->size.h     = h;
     object->center.x   = x + w / 2;
     object->center.y   = y + h / 2;
     object->color      = sfa_color;
             
     object->type     = type;
     object->bounds.x = x;
     object->bounds.y = y;
     object->bounds.w = w;
     object->bounds.h = h;
}

static void
SceneAddObject( SceneObject *object, int come, int stay, int go, SceneObjectFlags flags )
{
     D_MAGIC_ASSERT( object, SceneObject );

     object->flags    = flags;

     object->t_arrive = come;
     object->t_leave  = come + stay;
     object->t_gone   = come + stay + go;

     object->opacity  = 255;

     direct_list_append( &scene, &object->link );
}

static void
SceneRemoveObject( SceneObject *object )
{
     D_MAGIC_ASSERT( object, SceneObject );

     direct_list_remove( &scene, &object->link );
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

/**********************************************************************************************************************/

static bool
TickImage( SceneObject *object, int ms )
{
     D_MAGIC_ASSERT( object, SceneObject );

     return true;
}

static void
DrawImage( SceneObject *object )
{
     Image *image = (Image*) object;

     D_MAGIC_ASSERT( object, SceneObject );
     D_ASSERT( image->surface != NULL );

     if (object->opacity != 0xff) {
          surface->SetColor( surface, 0xff, 0xff, 0xff, object->opacity );
          surface->SetBlittingFlags( surface, DSBLIT_BLEND_COLORALPHA );
     }
     else
          surface->SetBlittingFlags( surface, DSBLIT_NOFX );

     if (object->size.w == object->bounds.w && object->size.h == object->bounds.h)
          surface->Blit( surface, image->surface, NULL, object->bounds.x, object->bounds.y );
     else
          surface->StretchBlit( surface, image->surface, NULL, &object->bounds );
}

static void
DestroyImage( SceneObject *object )
{
     Image *image = (Image*) object;

     D_MAGIC_ASSERT( object, SceneObject );
     D_ASSERT( image->surface != NULL );

     image->surface->Release( image->surface );

     D_MAGIC_CLEAR( object );

     D_FREE( image );
}

static Image *
CreateImage( const char *filename, int x, int y )
{
     DFBResult  ret;
     Image     *image;
     char       buf[256];
     int        width;
     int        height;

     D_ASSERT( filename != NULL );

     image = D_CALLOC( 1, sizeof(Image) );
     if (!image) {
          D_OOM();
          return NULL;
     }

     snprintf( buf, sizeof(buf), "%s%s", sfa_path, filename );

     ret = util_load_image( dfb, buf, DSPF_ARGB4444, &image->surface,
                            &width, &height, NULL );
     if (ret) {
          D_DERROR( ret, "util_load_image( '%s' )!\n", buf );
          D_FREE( image );
          return NULL;
     }

     image->object.Tick     = TickImage;
     image->object.Draw     = DrawImage;
     image->object.Destroy  = DestroyImage;

     D_MAGIC_SET( &image->object, SceneObject );

     InitObject( &image->object, x, y, width, height, SOT_IMAGE );

     return image;
}

/**********************************************************************************************************************/

static bool
TickText( SceneObject *object, int ms )
{
     D_MAGIC_ASSERT( object, SceneObject );

     return true;
}

static void
DrawText( SceneObject *object )
{
     Text *text = (Text*) object;

     D_MAGIC_ASSERT( object, SceneObject );

     if (object->opacity != 0xff)
          surface->SetDrawingFlags( surface, DSDRAW_BLEND );
     else
          surface->SetDrawingFlags( surface, DSDRAW_NOFX );

     surface->SetColor( surface, object->color.r, object->color.g, object->color.b, object->opacity );
     surface->DrawString( surface, text->message, -1, object->position.x, object->position.y, DSTF_TOPLEFT );
}

static void
DestroyText( SceneObject *object )
{
     Text *text = (Text*) object;

     D_MAGIC_ASSERT( object, SceneObject );
     D_ASSERT( text->message != NULL );

     D_FREE( text->message );

     D_MAGIC_CLEAR( object );

     D_FREE( text );
}

static Text *
CreateText( const char *message, int x, int y )
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

     text->object.Tick      = TickText;
     text->object.Draw      = DrawText;
     text->object.Destroy   = DestroyText;

     D_MAGIC_SET( &text->object, SceneObject );

     InitObject( &text->object, x, y, bounds.w, bounds.h, SOT_TEXT );

     text->object.bounds = bounds;

     return text;
}

/**********************************************************************************************************************/

static void
ResetSFA()
{
     /* Seek to start of animation. */
     fseek( sfa_file, 0, SEEK_SET );

     /* Set all defaults here. */
     sfa_time  = 0;
     sfa_color = (DFBColor){ 0xff, 0xee, 0xee, 0xee };
}

static void
OpenSFA( const char *filename )
{
     char *slash;

     sfa_file = fopen( filename, "r" );
     if (!sfa_file) {
          D_PERROR( "Could not open '%s'!\n", filename );
          return;
     }

     slash = strrchr( filename, '/' );
     if (slash)
          snprintf( sfa_path, MIN( sizeof(sfa_path), slash - filename + 2 ), filename );
     else
          sfa_path[0] = 0;

     ResetSFA();
}

static void
ReadSFA( int ms )
{
     char line[256];

     sfa_time += ms;

     if (sfa_wait > 0) {
          sfa_wait -= ms;

          if (sfa_wait > 0)
               return;
     }

     /* Read line by line... */
     while (fgets( line, sizeof(line), sfa_file )) {
          int   wait;
          int   len = strlen( line );
          char *next, *end, *command, *keyword, *keyval, *data = NULL;
          int   come = 0, stay = 0, go = 0, x = 0, y = 0, n;
          unsigned long    value = 0;
          SceneObjectFlags flags = 0;

          /* Ignore empty lines. */
          while (len > 0 && line[len-1] == '\n')
               line[--len] = 0;

          if (!len)
               continue;

          /* Check for timeline data (relative time values in ms on the left) */
          if (sscanf( line, "%d", &wait ) == 1) {
               sfa_wait += wait;
               return;
          }

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
                         D_ERROR( "ReadSFA(): Keyword '%s' without value!\n", keyword );
                         break;
                    }
               }

               /* Convert to integer for various (not all) cases below. */
               value = strtoul( keyval, &end, 10 );

               /* Always handle all keywords for now.
                * FIXME: Should split code for different commands.
                */
               if (!strcmp( keyword, "fadezoom" )) {             /** image, text **/
                    if (stay) {
                         flags |= SOF_FADEOUT | SOF_ZOOMOUT;
                         go = value;
                    }
                    else {
                         flags |= SOF_FADEIN | SOF_ZOOMIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "fade" )) {
                    if (stay) {
                         flags |= SOF_FADEOUT;
                         go = value;
                    }
                    else {
                         flags |= SOF_FADEIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "zoom" )) {
                    if (stay) {
                         flags |= SOF_ZOOMOUT;
                         go = value;
                    }
                    else {
                         flags |= SOF_ZOOMIN;
                         come = value;
                    }
               }
               else if (!strcmp( keyword, "time" )) {
                    stay = value;
               }
               else if (!strcmp( keyword, "pos" )) {
                    x = value;
                    y = strtoul( end+1, &end, 10 );
               }
               else if (!strcmp( keyword, "data" )) {
                    data = keyval;
               }
               else if (!strcmp( keyword, "rgb" )) {             /** color **/
                    value = strtoul( keyval, &end, 16 );
               }
          }

          /* Now run the command with the collected information. */
          if (!strcmp( command, "image" )) {
               Image *image = CreateImage( data, x, y );

               SceneAddObject( &image->object, come, stay, go, flags );
          }
          else if (!strcmp( command, "text" )) {
               Text *text = CreateText( data, x, y );

               SceneAddObject( &text->object, come, stay, go, flags );
          }
          else if (!strcmp( command, "color" )) {
               sfa_color.r = value >> 16;
               sfa_color.g = (value >> 8) & 0xff;
               sfa_color.b = value & 0xff;
          }
     }

     /* Animation has ended, loop! */
     ResetSFA();
}

/* TODO: CloseSFA() */

/**********************************************************************************************************************/

static void
TickScene( int ms )
{
     DirectLink  *l;
     SceneObject *object;

     ReadSFA( ms );

     direct_list_foreach_safe (object, l, scene) {
          D_MAGIC_ASSERT( object, SceneObject );

          object->age += ms;

          if (!object->Tick( object, ms )) {
               SceneRemoveObject( object );

               object->Destroy( object );

               continue;
          }

          switch (object->state) {
               case SOS_COME:
                    if (object->age >= object->t_arrive) {
                         object->state    = SOS_STAY;
                         object->opacity  = 255;
                         object->bounds.x = object->position.x;
                         object->bounds.y = object->position.y;
                         object->bounds.w = object->size.w;
                         object->bounds.h = object->size.h;

                         break;
                    }

                    if (object->flags & SOF_FADEIN)
                         object->opacity = 255 * object->age / object->t_arrive;

                    if (object->flags & SOF_ZOOMIN)
                         LinearZoom( &object->position, &object->center, &object->size,
                                     object->age, object->t_arrive, &object->bounds );
                    break;

               case SOS_STAY:
                    if (object->age >= object->t_leave)
                         object->state = SOS_GO;
                    break;

               case SOS_GO:
                    if (object->age >= object->t_gone) {
                         SceneRemoveObject( object );

                         object->Destroy( object );

                         break;
                    }

                    if (object->flags & SOF_FADEOUT)
                         object->opacity = 255 * (object->t_gone - object->age) / (object->t_gone - object->t_leave);

                    if (object->flags & SOF_ZOOMOUT)
                         LinearZoom( &object->position, &object->center, &object->size,
                                     object->t_gone - object->age, object->t_gone - object->t_leave, &object->bounds );
                    break;
          }
     }
}

static void
DrawScene()
{
     SceneObject *object;

     direct_list_foreach (object, scene) {
          D_MAGIC_ASSERT( object, SceneObject );

          object->Draw( object );

          if (bounds_debug) {
               surface->SetDrawingFlags( surface, DSDRAW_NOFX );
               surface->SetColor( surface, 0xff, 0x66, 0x11, 0xff );
               surface->DrawRectangle( surface, object->bounds.x, object->bounds.y, object->bounds.w, object->bounds.h );
          }
     }
}

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     DFBFontDescription    fdsc;
     DFBSurfaceDescription desc;
     FPSData               fps;
     int                   width;
     int                   height;
     bool                  running;
     long long             stamp;

     /* Initialize DirectFB including command line parsing. */
     CHECK( DirectFBInit( &argc, &argv ) );

     /* Create the super interface. */
     CHECK( DirectFBCreate( &dfb ) );

     /* Fill font description. */
     fdsc.flags  = /*DFDESC_WIDTH |*/ DFDESC_HEIGHT;
     fdsc.width  = 40;
     fdsc.height = 33;

     /* Create the primary surface. */
     CHECK( dfb->CreateFont( dfb, "data/FreeSans.ttf", &fdsc, &font ) );

     font->GetAscender( font, &font_ascender );

     /* Create an event buffer for all keyboard events. */
     CHECK( dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS, DFB_FALSE, &events ) );

     /* Request exclusive layer context. */
     dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

     /* Fill surface description. */
     desc.flags       = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
     desc.width       = 1366;
     desc.height      = 768;
     desc.caps        = DSCAPS_TRIPLE | DSCAPS_PRIMARY;// | DSCAPS_SYSTEMONLY;
     desc.pixelformat = DSPF_ARGB4444;

     /* Create the primary surface. */
     CHECK( dfb->CreateSurface( dfb, &desc, &surface ) );

     /* Query size of the screen. */
     CHECK( surface->GetSize( surface, &width, &height ) );

     /* Set the font. */
     CHECK( surface->SetFont( surface, font ) );

     /*
      * Open the animation
      */
     OpenSFA( "data/shopfloor/demo.sfa" );

     /*
      * Run the main loop
      */
     running = true;
     stamp   = direct_clock_get_millis();

     fps_init( &fps );

     while (running) {
          DFBInputEvent event;
          long long now  = direct_clock_get_millis();
          long long diff = now - stamp;

          TickScene( diff );

          surface->Clear( surface, 0, 0, 0, 0 );

          DrawScene();

          if (show_timefps) {
               char buf[32];

               surface->SetDrawingFlags( surface, DSDRAW_NOFX );
               surface->SetColor( surface, 0x66, 0xff, 0x11, 0xff );

               snprintf( buf, sizeof(buf), "%ld:%02ld.%03ld", sfa_time / 60000, (sfa_time / 1000) % 60, sfa_time % 1000 );
               surface->DrawString( surface, buf, -1, 20, 10, DSTF_TOPLEFT );

               surface->DrawString( surface, fps.fps_string, -1, width-20, 10, DSTF_TOPRIGHT );
          }

          surface->Flip( surface, NULL, DSFLIP_ONSYNC );

          fps_count( &fps, 400 );

          while (events->GetEvent( events, DFB_EVENT(&event) ) == DFB_OK) {
               switch (event.type) {
                    case DIET_KEYPRESS:
                         switch (event.key_symbol) {
                              case DIKS_ESCAPE:
                              case DIKS_POWER:
                              case DIKS_MENU:
                              case DIKS_BACK:
                                   running = false;
                                   break;

                              case DIKS_OK:
                              case DIKS_ENTER:
                                   show_timefps = !show_timefps;
                                   break;

                              default:
                                   break;
                         }

                    default:
                         break;
               }
          }

          stamp = now;
     }

     /* Shutdown. */
     surface->Release( surface );
     events->Release( events );
     font->Release( font );
     dfb->Release( dfb );

     return 0;
}

