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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/trace.h>
#include <direct/util.h>

#include <fusion/shmalloc.h>
#include <fusion/vector.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/gfxcard.h>
#include <core/layer_context.h>
#include <core/layer_region.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/palette.h>
#include <core/windows.h>
#include <core/windows_internal.h>
#include <core/windowstack.h>
#include <core/wm.h>

#include <gfx/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#include <core/wm_module.h>
#include <hwcursor/hwcursor.h>

D_DEBUG_DOMAIN( WM_Default, "WM/Default", "Default window manager module" );


DFB_WINDOW_MANAGER( default )

typedef struct {
     DirectLink                    link;

     DFBInputDeviceKeySymbol       symbol;
     DFBInputDeviceModifierMask    modifiers;

     CoreWindow                   *owner;
} GrabbedKey;

/**************************************************************************************************/

#define MAX_KEYS             16
#define MAX_UPDATE_REGIONS    8

typedef struct {
     CoreDFB                      *core;
} WMData;

typedef struct {
     int                           magic;

     CoreWindowStack              *stack;

     DFBUpdates                    updates;
     DFBRegion                     update_regions[MAX_UPDATE_REGIONS];

     DFBInputDeviceButtonMask      buttons;
     DFBInputDeviceModifierMask    modifiers;
     DFBInputDeviceLockState       locks;

     bool                          active;

     int                           wm_level;
     int                           wm_cycle;

     FusionVector                  windows;

     CoreWindow                   *pointer_window;     /* window grabbing the pointer */
     CoreWindow                   *keyboard_window;    /* window grabbing the keyboard */
     CoreWindow                   *focused_window;     /* window having the focus */
     CoreWindow                   *entered_window;     /* window under the pointer */
     CoreWindow                   *unselkeys_window;   /* window grabbing unselected keys */

     DirectLink                   *grabbed_keys;       /* List of currently grabbed keys. */

     struct {
          DFBInputDeviceKeySymbol      symbol;
          DFBInputDeviceKeyIdentifier  id;
          int                          code;
          CoreWindow                  *owner;
     } keys[MAX_KEYS];

     CoreSurface                  *cursor_bs;          /* backing store for region under cursor */
     bool                          cursor_bs_valid;
     DFBRegion                     cursor_region;
     bool                          cursor_drawn;

     int                           cursor_dx;
     int                           cursor_dy;
} StackData;

typedef struct {
     int                           magic;

     CoreWindow                   *window;

     StackData                    *stack_data;

     int                           priority;           /* derived from stacking class */

     CoreLayerRegionConfig         config;
} WindowData;

/**************************************************************************************************/

static DFBResult
restack_window( CoreWindow             *window,
                WindowData             *window_data,
                CoreWindow             *relative,
                WindowData             *relative_data,
                int                     relation,
                DFBWindowStackingClass  stacking );

static DFBResult
update_window( CoreWindow          *window,
               WindowData          *window_data,
               const DFBRegion     *region,
               DFBSurfaceFlipFlags  flags,
               bool                 force_complete,
               bool                 force_invisible,
               bool                 scale_region );

/**************************************************************************************************/

static int keys_compare( const void *key1,
                         const void *key2 )
{
     return *(const DFBInputDeviceKeySymbol*) key1 - *(const DFBInputDeviceKeySymbol*) key2;
}

/**************************************************************************************************/

static inline void
transform_point_in_window( CoreWindow *window,
                           int        *x,
                           int        *y )
{
     int _x = *x, _y = *y;

     switch (window->config.rotation) {
          default:
               D_BUG( "invalid rotation %d", window->config.rotation );
          case 0:
               break;

          case 90:
               *x = window->config.bounds.w - _y - 1;
               *y = _x;
               break;

          case 180:
               *x = window->config.bounds.w - _x - 1;
               *y = window->config.bounds.h - _y - 1;
               break;

          case 270:
               *x = _y;
               *y = window->config.bounds.h - _x - 1;
               break;
     }
}

static void
post_event( CoreWindow     *window,
            StackData      *data,
            DFBWindowEvent *event )
{
     D_ASSERT( window != NULL );
     D_ASSERT( window->stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );

     event->buttons   = data->buttons;
     event->modifiers = data->modifiers;
     event->locks     = data->locks;

     dfb_window_post_event( window, event );
}

static void
send_key_event( CoreWindow          *window,
                StackData           *data,
                const DFBInputEvent *event )
{
     DFBWindowEvent we;

     D_ASSERT( window != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );

     we.type       = (event->type == DIET_KEYPRESS) ? DWET_KEYDOWN : DWET_KEYUP;
	 we.flags      = (event->flags & DIEF_REPEAT) ? DWEF_REPEAT : 0;
	 we.flags      |= (event->flags & DIEF_NOREPEAT) ? DWEF_NOREPEAT : 0;
     we.key_code   = event->key_code;
     we.key_id     = event->key_id;
     we.key_symbol = event->key_symbol;

     post_event( window, data, &we );
}

static void
send_button_event( CoreWindow          *window,
                   StackData           *data,
                   const DFBInputEvent *event )
{
     DFBWindowEvent we;

     D_ASSERT( window != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );

     we.type   = (event->type == DIET_BUTTONPRESS) ? DWET_BUTTONDOWN : DWET_BUTTONUP;
     we.x      = window->stack->cursor.x - window->config.bounds.x;
     we.y      = window->stack->cursor.y - window->config.bounds.y;
     we.button = (data->wm_level & 2) ? (event->button + 2) : event->button;

     transform_point_in_window( window, &we.x, &we.y );

     post_event( window, data, &we );
}

/**************************************************************************************************/

static inline void
transform_window_to_stack( CoreWindow         *window,
                           const DFBRectangle *rect,
                           DFBRectangle       *ret_rect )
{
     DFB_RECTANGLE_ASSERT( rect );

     ret_rect->x = rect->x;
     ret_rect->y = rect->y;

     switch (window->config.rotation) {
          default:
               D_BUG( "invalid rotation %d", window->config.rotation );
          case 0:
          case 180:
               ret_rect->w = rect->w;
               ret_rect->h = rect->h;
               break;

          case 90:
          case 270:
               ret_rect->w = rect->h;
               ret_rect->h = rect->w;
               break;
     }
}

static inline int
get_priority( const CoreWindow *window )
{
     D_ASSERT( window != NULL );

     switch (window->config.stacking) {
          case DWSC_UPPER:
               return  1;

          case DWSC_MIDDLE:
               return  0;

          case DWSC_LOWER:
               return -1;
               

          default:
               D_BUG( "unknown stacking class" );
               break;
     }

     return 0;
}

static inline int
get_index( const StackData  *data,
           const CoreWindow *window )
{
     D_ASSERT( data != NULL );
     D_ASSERT( window != NULL );

     D_ASSERT( fusion_vector_contains( &data->windows, window ) );

     return fusion_vector_index_of( &data->windows, window );
}

static CoreWindow *
get_keyboard_window( CoreWindowStack     *stack,
                     StackData           *data,
                     const DFBInputEvent *event )
{
     DirectLink *l;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_KEYPRESS || event->type == DIET_KEYRELEASE );

     /* Check explicit key grabs first. */
     direct_list_foreach (l, data->grabbed_keys) {
          GrabbedKey *key = (GrabbedKey*) l;

          if (key->symbol    == event->key_symbol &&
              key->modifiers == data->modifiers)
               return key->owner;
     }

     /* Don't do implicit grabs on keys without a hardware index. */
     if (event->key_code == -1)
          return (data->keyboard_window ?
                  data->keyboard_window : data->focused_window);

     /* Implicitly grab (press) or ungrab (release) key. */
     if (event->type == DIET_KEYPRESS) {
          int         i;
          int         free_key = -1;
          CoreWindow *window;

          /* Check active grabs. */
          for (i=0; i<MAX_KEYS; i++) {
               /* Key is grabbed, send to owner (NULL if destroyed). */
               if (data->keys[i].code == event->key_code)
                    return data->keys[i].owner;

               /* Remember first free array item. */
               if (free_key == -1 && data->keys[i].code == -1)
                    free_key = i;
          }

          /* Key is not grabbed, check for explicit keyboard grab or focus. */
          window = data->keyboard_window ?
                   data->keyboard_window : data->focused_window;
          if (!window)
               return NULL;

          /* Check key selection. */
          switch (window->config.key_selection) {
               case DWKS_ALL:
                    break;

               case DWKS_LIST:
                    D_ASSERT( window->config.keys != NULL );
                    D_ASSERT( window->config.num_keys > 0 );

                    if (bsearch( &event->key_symbol,
                                 window->config.keys, window->config.num_keys,
                                 sizeof(DFBInputDeviceKeySymbol), keys_compare ))
                         break;

                    /* fall through */

               case DWKS_NONE:
                    return data->unselkeys_window;
          }

          /* Check if a free array item was found. */
          if (free_key == -1) {
               D_WARN( "maximum number of owned keys reached" );
               return NULL;
          }

          /* Implicitly grab the key. */
          data->keys[free_key].symbol = event->key_symbol;
          data->keys[free_key].id     = event->key_id;
          data->keys[free_key].code   = event->key_code;
          data->keys[free_key].owner  = window;

          return window;
     }
     else {
          int i;

          /* Lookup owner and ungrab the key. */
          for (i=0; i<MAX_KEYS; i++) {
               if (data->keys[i].code == event->key_code) {
                    data->keys[i].code = -1;

                    /* Return owner (NULL if destroyed). */
                    return data->keys[i].owner;
               }
          }
     }

     /* No owner for release event found, discard it. */
     return NULL;
}

static CoreWindow*
window_at_pointer( CoreWindowStack *stack,
                   StackData       *data,
                   WMData          *wmdata,
                   int              x,
                   int              y )
{
     int         i;
     CoreWindow *window;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );

     if (!stack->cursor.enabled) {
          fusion_vector_foreach_reverse (window, i, data->windows)
               if (window->config.opacity && !(window->config.options & DWOP_GHOST))
                    return window;

          return NULL;
     }

     if (x < 0)
          x = stack->cursor.x;
     if (y < 0)
          y = stack->cursor.y;

     fusion_vector_foreach_reverse (window, i, data->windows) {
          CoreWindowConfig *config  = &window->config;
          DFBWindowOptions  options = config->options;
          DFBRectangle      rotated;
          DFBRectangle     *bounds  = &rotated;

          transform_window_to_stack( window, &config->bounds, &rotated );

          if (!(options & DWOP_GHOST) && config->opacity &&
              x >= bounds->x  &&  x < bounds->x + bounds->w &&
              y >= bounds->y  &&  y < bounds->y + bounds->h)
          {
               int wx = x - bounds->x;
               int wy = y - bounds->y;

               if ( !(options & DWOP_SHAPED)  ||
                    !(options &(DWOP_ALPHACHANNEL|DWOP_COLORKEYING))
                    || !window->surface ||
                    ((options & DWOP_OPAQUE_REGION) &&
                     (wx >= config->opaque.x1  &&  wx <= config->opaque.x2 &&
                      wy >= config->opaque.y1  &&  wy <= config->opaque.y2)))
               {
                    return window;
               }
               else {
                    u8                     buf[8];
                    CoreSurface           *surface = window->surface;
                    DFBSurfacePixelFormat  format  = surface->config.format;
                    DFBRectangle           rect    = { wx, wy, 1, 1 };

                    if (dfb_surface_read_buffer( surface, CSBR_FRONT, buf, 8, &rect ) == DFB_OK) {
                         if (options & DWOP_ALPHACHANNEL) {
                              int alpha = -1;

                              D_ASSERT( DFB_PIXELFORMAT_HAS_ALPHA( format ) );

                              switch (format) {
                                   case DSPF_AiRGB:
                                        alpha = 0xff - (*(u32*)(buf) >> 24);
                                        break;
                                   case DSPF_ARGB:
                                   case DSPF_ABGR:
                                   case DSPF_AYUV:
                                        alpha = *(u32*)(buf) >> 24;
                                        break;
                                   case DSPF_RGBA5551:
                                        alpha = *(u16*)(buf) & 0x1;
                                        alpha = alpha ? 0xff : 0x00;
                                        break;
                                   case DSPF_ARGB1555:
                                   case DSPF_ARGB2554:
                                   case DSPF_ARGB4444:
                                        alpha = *(u16*)(buf) & 0x8000;
                                        alpha = alpha ? 0xff : 0x00;
                                        break;
                                   case DSPF_RGBA4444:
                                        alpha = *(u16*)(buf) & 0x0008;
                                        alpha = alpha ? 0xff : 0x00;
                                        break;
                                   case DSPF_ALUT44:
                                        alpha = *(u8*)(buf) & 0xf0;
                                        alpha |= alpha >> 4;
                                        break;
                                   case DSPF_LUT1:
                                   case DSPF_LUT2:
                                   case DSPF_LUT4:
                                   case DSPF_LUT8: {
                                        CorePalette *palette = surface->palette;
                                        u8           pix     = *((u8*) buf);

                                        if (palette && pix < palette->num_entries) {
                                             alpha = palette->entries[pix].a;
                                             break;
                                        }


                                        /* fall through */
                                   }

                                   default:
                                        D_ONCE( "unknown format 0x%x", surface->config.format );
                                        break;
                              }

                              if (alpha) /* alpha == -1 on error */
                                   return window;
                         }
                         if (options & DWOP_COLORKEYING) {
                              int pixel = 0;
                              u8 *p;
                              switch (format) {
                                   case DSPF_ARGB:
                                   case DSPF_ABGR:
                                   case DSPF_AiRGB:
                                   case DSPF_RGB32:
                                        pixel = *(u32*)(buf) & 0x00ffffff;
                                        break;

                                   case DSPF_RGB24:
                                        p = (buf);
#ifdef WORDS_BIGENDIAN
                                        pixel = (p[0] << 16) | (p[1] << 8) | p[2];
#else
                                        pixel = (p[2] << 16) | (p[1] << 8) | p[0];
#endif
                                        break;

                                   case DSPF_RGB16:
                                        pixel = *(u16*)(buf);
                                        break;

                                   case DSPF_ARGB4444:
                                   case DSPF_RGB444:
                                        pixel = *(u16*)(buf)
                                                & 0x0fff;
                                        break;

                                   case DSPF_RGBA4444:
                                        pixel = *(u16*)(buf)
                                                & 0xfff0;
                                        break;

                                   case DSPF_ARGB1555:
                                   case DSPF_RGB555:
                                   case DSPF_BGR555:
                                        pixel = *(u16*)(buf)
                                                & 0x7fff;
                                        break;

                                   case DSPF_RGBA5551:
                                        pixel = *(u16*)(buf)
                                                & 0xfffe;
                                        break;

                                   case DSPF_RGB332:
                                   case DSPF_LUT8:
                                        pixel = *(u8*)(buf);
                                        break;

                                   case DSPF_ALUT44:
                                        pixel = *(u8*)(buf)
                                                & 0x0f;
                                        break;

                                   default:
                                        D_ONCE( "unknown format 0x%x", surface->config.format );
                                        break;
                              }

                              if (pixel != config->color_key)
                                   return window;
                         }
                    }
               }
          }
     }

     return NULL;
}

static void
switch_focus( CoreWindowStack *stack,
              StackData       *data,
              CoreWindow      *to )
{
     DFBWindowEvent  evt;
     CoreWindow     *from;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );

     from = data->focused_window;

     if (from == to)
          return;

     if (to && to->caps & DWCAPS_NOFOCUS)
          return;

     if (from) {
          evt.type = DWET_LOSTFOCUS;

          post_event( from, data, &evt );
     }

     if (to) {
          if (to->surface && to->surface->palette && !stack->hw_mode) {
               CoreSurface *surface;

               D_ASSERT( to->primary_region != NULL );

               if (dfb_layer_region_get_surface( to->primary_region, &surface ) == DFB_OK) {
                    if (DFB_PIXELFORMAT_IS_INDEXED( surface->config.format ))
                         dfb_surface_set_palette( surface, to->surface->palette );

                    dfb_surface_unref( surface );
               }
          }

          evt.type = DWET_GOTFOCUS;

          post_event( to, data, &evt );
     }

     data->focused_window = to;
}

static bool
update_focus( CoreWindowStack *stack,
              StackData       *data,
              WMData          *wmdata )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );

     /* if pointer is not grabbed */
     if (!data->pointer_window)
     {
        /*
        data->entered_window represents window under the cursor,
        data->focused_window represents window having the focus
        [Scenario]:when setting the opacity of all windows from 0 to 0xff, there are two "focused" window occurred.

        [Root cause]: During the time of making all windows hide (i.e. opacity = 0), it forces the last hidden window to be focused and no entered window left finally.
        According to DFB framework, entered window is used to represent the previous focusd window in the function "update_focus". But the above-mentioned case,
        the framework would never know the previous focus window, the function of "switch focus" cannot be correct definitely which caused more than one focus windows.

        [Solution]:If there is no entered window, use the current "focus" window to be the previous one in the function "update_focus".
        */

          CoreWindow *before = data->entered_window ? data->entered_window : data->focused_window ;
          CoreWindow *after  = window_at_pointer( stack, data, wmdata, -1, -1 );

          /* and the window under the cursor is another one now */
          if (before != after) {
               DFBWindowEvent we;

               /* send leave event */
               if (before) {
                    we.type = DWET_LEAVE;
                    we.x    = stack->cursor.x - before->config.bounds.x;
                    we.y    = stack->cursor.y - before->config.bounds.y;

                    transform_point_in_window( before, &we.x, &we.y );

                    post_event( before, data, &we );
               }

               /* switch focus and send enter event */
               switch_focus( stack, data, after );

               if (after) {
                    we.type = DWET_ENTER;
                    we.x    = stack->cursor.x - after->config.bounds.x;
                    we.y    = stack->cursor.y - after->config.bounds.y;

                    transform_point_in_window( after, &we.x, &we.y );

                    post_event( after, data, &we );
               }

               /* update pointer to window under the cursor */
               data->entered_window = after;

               return true;
          }
     }

     return false;
}

static void
ensure_focus( CoreWindowStack *stack,
              StackData       *data )
{
     int         i;
     CoreWindow *window;

     if (data->focused_window)
          return;

     fusion_vector_foreach_reverse (window, i, data->windows) {
          if (window->config.opacity && !(window->config.options & DWOP_GHOST)
               && !(window->caps & DWCAPS_NOFOCUS)) {
               switch_focus( stack, data, window );
               break;
          }
     }
}

/**************************************************************************************************/
/**************************************************************************************************/

static inline void
transform_stack_to_dest( CoreWindowStack *stack,
                         const DFBRegion *region,
                         DFBRegion       *ret_dest )
{
     DFBDimension size = { stack->width, stack->height };

     DFB_REGION_ASSERT( region );

     dfb_region_from_rotated( ret_dest, region, &size, stack->rotation );
}

#if USE_CURSOR
static void
draw_cursor( CoreWindowStack *stack, StackData *data, CardState *state, const DFBRegion *region )
{
     DFBRectangle            src;
     DFBRegion               dest;
     DFBSurfaceBlittingFlags flags = DSBLIT_BLEND_ALPHACHANNEL;

     D_ASSERT( stack != NULL );
     D_MAGIC_ASSERT( data, StackData );
     D_MAGIC_ASSERT( state, CardState );
     DFB_REGION_ASSERT( region );

     D_ASSUME( stack->cursor.opacity > 0 );

     /* Initialize destination region. */
     transform_stack_to_dest( stack, region, &dest );

     /* Initialize source rectangle. */
     src.x = region->x1 - stack->cursor.x + stack->cursor.hot.x;
     src.y = region->y1 - stack->cursor.y + stack->cursor.hot.y;
     src.w = region->x2 - region->x1 + 1;
     src.h = region->y2 - region->y1 + 1;

     /* Use global alpha blending. */
     if (stack->cursor.opacity != 0xFF) {
          flags |= DSBLIT_BLEND_COLORALPHA;

          /* Set opacity as blending factor. */
          if (state->color.a != stack->cursor.opacity) {
               state->color.a   = stack->cursor.opacity;
               state->modified |= SMF_COLOR;
          }
     }

     /* Different compositing methods depending on destination format. */
     if (flags & DSBLIT_BLEND_ALPHACHANNEL) {
          if (DFB_PIXELFORMAT_HAS_ALPHA( state->destination->config.format )) {
               /*
                * Always use compliant Porter/Duff SRC_OVER,
                * if the destination has an alpha channel.
                *
                * Cd = destination color  (non-premultiplied)
                * Ad = destination alpha
                *
                * Cs = source color       (non-premultiplied)
                * As = source alpha
                *
                * Ac = color alpha
                *
                * cd = Cd * Ad            (premultiply destination)
                * cs = Cs * As            (premultiply source)
                *
                * The full equation to calculate resulting color and alpha (premultiplied):
                *
                * cx = cd * (1-As*Ac) + cs * Ac
                * ax = Ad * (1-As*Ac) + As * Ac
                */
               dfb_state_set_src_blend( state, DSBF_ONE );

               /* Need to premultiply source with As*Ac or only with Ac? */
               if (! (stack->cursor.surface->config.caps & DSCAPS_PREMULTIPLIED))
                    flags |= DSBLIT_SRC_PREMULTIPLY;
               else if (flags & DSBLIT_BLEND_COLORALPHA)
                    flags |= DSBLIT_SRC_PREMULTCOLOR;

               /* Need to premultiply/demultiply destination? */
//               if (! (state->destination->caps & DSCAPS_PREMULTIPLIED))
//                    flags |= DSBLIT_DST_PREMULTIPLY | DSBLIT_DEMULTIPLY;
          }
          else {
               /*
                * We can avoid DSBLIT_SRC_PREMULTIPLY for destinations without an alpha channel
                * by using another blending function, which is more likely that it's accelerated
                * than premultiplication at this point in time.
                *
                * This way the resulting alpha (ax) doesn't comply with SRC_OVER,
                * but as the destination doesn't have an alpha channel it's no problem.
                *
                * As the destination's alpha value is always 1.0 there's no need for
                * premultiplication. The resulting alpha value will also be 1.0 without
                * exceptions, therefore no need for demultiplication.
                *
                * cx = Cd * (1-As*Ac) + Cs*As * Ac  (still same effect as above)
                * ax = Ad * (1-As*Ac) + As*As * Ac  (wrong, but discarded anyways)
                */
               if (stack->cursor.surface->config.caps & DSCAPS_PREMULTIPLIED) {
                    /* Need to premultiply source with Ac? */
                    if (flags & DSBLIT_BLEND_COLORALPHA)
                         flags |= DSBLIT_SRC_PREMULTCOLOR;

                    dfb_state_set_src_blend( state, DSBF_ONE );
               }
               else
                    dfb_state_set_src_blend( state, DSBF_SRCALPHA );
          }
     }

     /* Set blitting flags. */
     dfb_state_set_blitting_flags( state, flags | stack->rotated_blit );

     /* Set blitting source. */
     state->source    = stack->cursor.surface;
     state->modified |= SMF_SOURCE;

     /* Blit from the window to the region being updated. */
     dfb_gfxcard_blit( &src, dest.x1, dest.y1, state );

     /* Reset blitting source. */
     state->source    = NULL;
     state->modified |= SMF_SOURCE;
}
#endif

static DFBSurfaceBlittingFlags get_window_blit_flag( CardState *state, CoreWindow *window,
                                                                                                         bool alpha_channel,
                                                                                                         DFBSurfaceBlendFunction *srcblend)
{
     DFBSurfaceBlittingFlags  flags = DSBLIT_NOFX;
     CoreWindowConfig        *config;
     CoreSurface             *surface;
     int                      rotation;
     CoreWindowStack *stack = window->stack;

     D_MAGIC_ASSERT( state, CardState );
     D_MAGIC_ASSERT( stack, CoreWindowStack );

     surface = window->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     config = &window->config;
     /* Use per pixel alpha blending. */
     if (alpha_channel && (config->options & DWOP_ALPHACHANNEL))
          flags |= DSBLIT_BLEND_ALPHACHANNEL;
     /* Use global alpha blending. */
     if (config->opacity != 0xFF) {
          flags |= DSBLIT_BLEND_COLORALPHA;
     }
     /* Use source color keying. */
     if (config->options & DWOP_COLORKEYING) {
          flags |= DSBLIT_SRC_COLORKEY;
     }
     /* Use automatic deinterlacing. */
     if (surface->config.caps & DSCAPS_INTERLACED)
          flags |= DSBLIT_DEINTERLACE;

     //patch, if hw not support new alpha mode, use demultiply flag to divide the color
     //by the alpha before writing the data to the destination
     if ((dfb_config->mst_new_alphamode == 2) &&
         (dfb_config->mst_newalphamode_on_layer & (u8)(1 << window->primary_region->context->layer_id)))
     {
         DBG_LAYER_MSG("[DFB] Set window blit flag, mst_new_alphamode = %d, mst_newalphamode_on_layer=0x%x , layer_id=%d (%s, %s) \n",
             dfb_config->mst_new_alphamode, dfb_config->mst_newalphamode_on_layer, window->primary_region->context->layer_id, __FILE__, __FUNCTION__ );
         flags |= DSBLIT_DEMULTIPLY;
     }

     /* Different compositing methods depending on destination format. */
     if (flags & (DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA)) {
          if (DFB_PIXELFORMAT_HAS_ALPHA( state->destination->config.format )) {
               /*
                * Always use compliant Porter/Duff SRC_OVER,
                * if the destination has an alpha channel.
                *
                * Cd = destination color  (non-premultiplied)
                * Ad = destination alpha
                *
                * Cs = source color       (non-premultiplied)
                * As = source alpha
                *
                * Ac = color alpha
                *
                * cd = Cd * Ad            (premultiply destination)
                * cs = Cs * As            (premultiply source)
                *
                * The full equation to calculate resulting color and alpha (premultiplied):
                *
                * cx = cd * (1-As*Ac) + cs * Ac
                * ax = Ad * (1-As*Ac) + As * Ac
                */
               *srcblend = DSBF_ONE;

               /* Need to premultiply source with As*Ac or only with Ac? */
               if (! (surface->config.caps & DSCAPS_PREMULTIPLIED))
                    flags |= DSBLIT_SRC_PREMULTIPLY;
               else if (flags & DSBLIT_BLEND_COLORALPHA)
                    flags |= DSBLIT_SRC_PREMULTCOLOR;

               /* Need to premultiply/demultiply destination? */
//               if (! (state->destination->caps & DSCAPS_PREMULTIPLIED))
//                    flags |= DSBLIT_DST_PREMULTIPLY | DSBLIT_DEMULTIPLY;
          }
          else {
               /*
                * We can avoid DSBLIT_SRC_PREMULTIPLY for destinations without an alpha channel
                * by using another blending function, which is more likely that it's accelerated
                * than premultiplication at this point in time.
                *
                * This way the resulting alpha (ax) doesn't comply with SRC_OVER,
                * but as the destination doesn't have an alpha channel it's no problem.
                *
                * As the destination's alpha value is always 1.0 there's no need for
                * premultiplication. The resulting alpha value will also be 1.0 without
                * exceptions, therefore no need for demultiplication.
                *
                * cx = Cd * (1-As*Ac) + Cs*As * Ac  (still same effect as above)
                * ax = Ad * (1-As*Ac) + As*As * Ac  (wrong, but discarded anyways)
                */
               if (surface->config.caps & DSCAPS_PREMULTIPLIED) {
                    /* Need to premultiply source with Ac? */
                    if (flags & DSBLIT_BLEND_COLORALPHA)
                         flags |= DSBLIT_SRC_PREMULTCOLOR;
                   *srcblend = DSBF_ONE;
               }
               else
                    *srcblend = DSBF_SRCALPHA;
          }
     }

     rotation = (window->config.rotation + stack->rotation) % 360;
     switch (rotation) {
          default:
               D_BUG( "invalid rotation %d", rotation );
          case 0:
               break;

          case 90:
               flags |= DSBLIT_ROTATE90;
               break;

          case 180:
               flags |= DSBLIT_ROTATE180;
               break;

          case 270:
               flags |= DSBLIT_ROTATE270;
               break;
     }
     return flags;

}
static inline bool bypass_underdraw(CoreWindow *window, CardState *state, bool alpha_channel,
    int window_index)
{
    CoreWindowStack         *stack;
    DFBSurfaceBlendFunction src_blend = DSBF_UNKNOWN;  //Coverity-03132013
    DFBSurfaceBlittingFlags flags;

    D_ASSERT( window != NULL );
    D_MAGIC_ASSERT( state, CardState );
    if(window_index > 0)
         return false;
     stack = window->stack;
     D_MAGIC_ASSERT( stack, CoreWindowStack );
     if(stack->bg.mode!=DLBM_COLOR || stack->bg.color.r!=0 || stack->bg.color.g!=0 || stack->bg.color.b!=0
           ||  stack->bg.color.a!=0)
        {
           return false;
        }
     flags =  get_window_blit_flag(state, window,alpha_channel, &src_blend);
     if(flags & (DSBLIT_SRC_COLORKEY|DSBLIT_DST_COLORKEY|DSBLIT_XOR))
         return false;
     if(flags&(DSBLIT_BLEND_ALPHACHANNEL|DSBLIT_BLEND_COLORALPHA))
     {
          switch(src_blend)
            {
                 case DSBF_DESTALPHA:
                 case  DSBF_INVDESTALPHA:
                 case DSBF_DESTCOLOR:
                 case DSBF_INVDESTCOLOR:
                     return false;
                 default:
                      break;
            }
     }
     return true;
}

static bool doGPUCompose = false;

static bool check_format(DFBSurfacePixelFormat dfb_format)
{
    bool ret = false;

    switch(dfb_format)
    {
        case DSPF_ARGB:
	    case DSPF_ABGR:
        case DSPF_ARGB4444:
            ret = true;
            break;
    }

    return ret;
}

static bool check_GPU_compose(CardState *state, DFBRegion dest)
{
     if ( doGPUCompose &&
          check_format(state->source->config.format) &&
          check_format(state->destination->config.format) )
          return true;
     else
          return false;
}

static void
draw_window( CoreWindow *window, CardState *state,
             const DFBRegion *region, bool alpha_channel, bool bforce_disable_alphablend)
{
     DFBRegion                dest;
     DFBSurfaceBlittingFlags  flags = DSBLIT_NOFX;
     CoreWindowStack         *stack;
     CoreWindowConfig        *config;
     CoreSurface             *surface;
     int                      rotation;
     DFBSurfaceBlendFunction src_blend = DSBF_UNKNOWN; //Fix converity END
     DFBSurfaceBlendFunction org_dst_blend = DSBF_UNKNOWN; //Fix converity END

     D_ASSERT( window != NULL );
     D_MAGIC_ASSERT( state, CardState );
     DFB_REGION_ASSERT( region );

     stack = window->stack;
     D_MAGIC_ASSERT( stack, CoreWindowStack );

     surface = window->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     config = &window->config;

     /* Initialize destination region. */
     transform_stack_to_dest( stack, region, &dest );

     flags = get_window_blit_flag(state, window, alpha_channel, &src_blend);

     /* Use global alpha blending. */
     if ( flags & DSBLIT_BLEND_COLORALPHA) {
          /* Set opacity as blending factor. */
          if (state->color.a != config->opacity) {
               state->color.a   = config->opacity;
               state->modified |= SMF_COLOR;
          }
     }

     /* Use source color keying. */
     if (flags & DSBLIT_SRC_COLORKEY) {
          /* Set window color key. */
          dfb_state_set_src_colorkey( state, config->color_key );
     }
     dfb_state_set_src_blend( state, src_blend );
     if(bforce_disable_alphablend)
     {
          org_dst_blend = state->dst_blend;
          dfb_state_set_dst_blend( state, DSBF_ZERO );
     }

     /* Set blitting flags. */
     dfb_state_set_blitting_flags( state, flags );

     /* Set blitting source. */
     state->source    = surface;
     state->modified |= SMF_SOURCE;

     if (window->config.options & DWOP_SCALE) {
          DFBDimension size = { stack->width, stack->height };
          DFBRegion    clip = state->clip;
          DFBRectangle src  = { 0, 0, surface->config.size.w, surface->config.size.h };
          DFBRectangle dst;
          DFBRectangle bounds;

          transform_window_to_stack( window, &window->config.bounds, &bounds );

          dfb_rectangle_from_rotated( &dst, &bounds, &size, stack->rotation );

          /* Change clipping region. */
          dfb_state_set_clip( state, &dest );

          /* Scale window to the screen clipped by the region being updated. */
          if ( check_GPU_compose(state, dest) )
               dfb_gfxcard_stretchblit_gles2( &src, &dst, state );
          else
               dfb_gfxcard_stretchblit( &src, &dst, state );

          /* Restore clipping region. */
          dfb_state_set_clip( state, &clip );
     }
     else {
          DFBDimension size = { config->bounds.w, config->bounds.h };
          DFBRectangle rect, src;

          D_ASSERT( surface->config.size.w == config->bounds.w );
          D_ASSERT( surface->config.size.h == config->bounds.h );

          /* Initialize source rectangle. */
          dfb_rectangle_from_region( &rect, region );

          /* Subtract window offset. */
          rect.x -= config->bounds.x;
          rect.y -= config->bounds.y;

          /* Rotate back to window surface. */
          if (window->config.rotation == 90 || window->config.rotation == 270)
               D_UTIL_SWAP( size.w, size.h );

          dfb_rectangle_from_rotated( &src, &rect, &size, (360 - window->config.rotation) % 360 );

          /* Blit from the window to the region being updated. */
          if ( check_GPU_compose(state, dest) )
               dfb_gfxcard_blit_gles2( &src, dest.x1, dest.y1, state );
          else
               dfb_gfxcard_blit( &src, dest.x1, dest.y1, state );
     }

     if(bforce_disable_alphablend)
     {
          dfb_state_set_dst_blend( state, org_dst_blend );
     }
     /* Reset blitting source. */
     state->source    = NULL;
     state->modified |= SMF_SOURCE;
}

static void
draw_background( CoreWindowStack *stack, CardState *state, const DFBRegion *region )
{
     DFBRegion dest;

     D_ASSERT( stack != NULL );
     D_MAGIC_ASSERT( state, CardState );
     DFB_REGION_ASSERT( region );

     if (stack->bg.mode == DLBM_IMAGE && stack->bg.bRelease_image)
         return;

     D_ASSERT( stack->bg.image != NULL || (stack->bg.mode != DLBM_IMAGE &&
                                           stack->bg.mode != DLBM_TILE) );

     /* Initialize destination region. */
     transform_stack_to_dest( stack, region, &dest );

     if (!dfb_region_intersect( &dest, 0, 0,
                                state->destination->config.size.w - 1, state->destination->config.size.h - 1 ))
         return;

     switch (stack->bg.mode) {
          case DLBM_COLOR: {
               DFBRectangle  rect  = DFB_RECTANGLE_INIT_FROM_REGION( &dest );
               CoreSurface  *dst   = state->destination;
               DFBColor     *color = &stack->bg.color;

               D_MAGIC_ASSERT( dst, CoreSurface );

               /* Set the background color. */
               if (DFB_PIXELFORMAT_IS_INDEXED( dst->config.format ))
                    dfb_state_set_color_index( state,  /* FIXME: don't search every time */
                                               dfb_palette_search( dst->palette, color->r,
                                                                   color->g, color->b, color->a ) );
               else
                    dfb_state_set_color( state, color );

               /* Simply fill the background. */
               dfb_gfxcard_fillrectangles( &rect, 1, state );

               DBG_LAYER_MSG("[DFB] %s: stack->bg.mode = DLBM_COLOR\n", __FUNCTION__);
               break;
          }

          case DLBM_IMAGE: {
               CoreSurface  *bg   = stack->bg.image;
               DFBRegion     clip = state->clip;
               DFBRectangle  src  = { 0, 0, bg->config.size.w, bg->config.size.h };
               DFBRectangle  dst  = { 0, 0, stack->rotated_width, stack->rotated_height };

               D_MAGIC_ASSERT( bg, CoreSurface );

               /* Set blitting source. */
               state->source    = bg;
               state->modified |= SMF_SOURCE;

               /* Set blitting flags. */
               dfb_state_set_blitting_flags( state, stack->rotated_blit );

               /* Set clipping region. */
               dfb_state_set_clip( state, &dest );

               /* Blit background image. */
               dfb_gfxcard_stretchblit( &src, &dst, state );

               /* Restore clipping region. */
               dfb_state_set_clip( state, &clip );

               /* Reset blitting source. */
               state->source    = NULL;
               state->modified |= SMF_SOURCE;

               DBG_LAYER_MSG("[DFB] %s: stack->bg.mode = DLBM_IMAGE\n", __FUNCTION__);

               break;
          }

          case DLBM_TILE: {
               CoreSurface  *bg   = stack->bg.image;
               DFBRegion     clip = state->clip;
               DFBRectangle  src  = { 0, 0, bg->config.size.w, bg->config.size.h };

               D_MAGIC_ASSERT( bg, CoreSurface );

               /* Set blitting source. */
               state->source    = bg;
               state->modified |= SMF_SOURCE;

               /* Set blitting flags. */
               dfb_state_set_blitting_flags( state, stack->rotated_blit );

               /* Change clipping region. */
               dfb_state_set_clip( state, &dest );

               /* Tiled blit (aligned). */
               dfb_gfxcard_tileblit( &src,
                                     (region->x1 / src.w) * src.w,
                                     (region->y1 / src.h) * src.h,
                                     (region->x2 / src.w + 1) * src.w,
                                     (region->y2 / src.h + 1) * src.h,
                                     state );

               /* Restore clipping region. */
               dfb_state_set_clip( state, &clip );

               /* Reset blitting source. */
               state->source    = NULL;
               state->modified |= SMF_SOURCE;

               DBG_LAYER_MSG("[DFB] %s: stack->bg.mode = DLBM_TILE\n", __FUNCTION__);

               break;
          }

          case DLBM_DONTCARE:
               DBG_LAYER_MSG("[DFB] %s: stack->bg.mode = DLBM_DONTCARE\n", __FUNCTION__);
               break;

          default:
               D_BUG( "unknown background mode" );
               break;
     }
}

static void
update_region( CoreWindowStack *stack,
               StackData       *data,
               CardState       *state,
               int              start,
               int              x1,
               int              y1,
               int              x2,
               int              y2 )
{
     int       i      = start;
     DFBRegion region = { x1, y1, x2, y2 };

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_MAGIC_ASSERT( state, CardState );
     D_ASSERT( start < fusion_vector_size( &data->windows ) );
     D_ASSERT( x1 <= x2 );
     D_ASSERT( y1 <= y2 );

     /* Find next intersecting window. */
     while (i >= 0) {
          CoreWindow *window = fusion_vector_at( &data->windows, i );

          if (VISIBLE_WINDOW( window )) {
               DFBRectangle rotated;

               transform_window_to_stack( window, &window->config.bounds, &rotated );

               if (dfb_region_intersect( &region,
                                         DFB_REGION_VALS_FROM_RECTANGLE( &rotated )))
                    break;
          }

          i--;
     }

     /* Intersecting window found? */
     if (i >= 0) {
          CoreWindow       *window = fusion_vector_at( &data->windows, i );
          CoreWindowConfig *config = &window->config;
          bool bypass_under_draw =
                                bypass_underdraw(window, state, true,  i);

          if (D_FLAGS_ARE_SET( config->options, DWOP_ALPHACHANNEL | DWOP_OPAQUE_REGION )) {
               DFBRegion opaque = DFB_REGION_INIT_TRANSLATED( &config->opaque,
                                                              config->bounds.x,
                                                              config->bounds.y );

                if (!dfb_region_region_intersect( &opaque, &region )) {

                      if(!bypass_under_draw)
                            update_region( stack, data, state, i-1, x1, y1, x2, y2 );
                      else
                      {
                             //update un-conjected area aroud this window
                             /* left */
                             if (region.x1 != x1)
                                  update_region( stack, data, state, i-1, x1, region.y1, region.x1-1, region.y2 );

                             /* upper */
                             if (region.y1 != y1)
                                      update_region( stack, data, state, i-1, x1, y1, x2, region.y1-1 );

                             /* right */
                             if (region.x2 != x2)
                                  update_region( stack, data, state, i-1, region.x2+1, region.y1, x2, region.y2 );

                             /* lower */
                             if (region.y2 != y2)
                                  update_region( stack, data, state, i-1, x1, region.y2+1, x2, y2 );
                      }

                       draw_window( window, state, &region, true, bypass_under_draw);
               }
               else {
                    if(!bypass_under_draw)
                    {
                          if ((config->opacity < 0xff) || (config->options & DWOP_COLORKEYING)) {
                           /* draw everything below */
                                update_region( stack, data, state, i-1, x1, y1, x2, y2 );
                          }
                           else {
                              //update un-conjected area aroud this window's opaque area
                                /* left */
                                if (opaque.x1 != x1)
                                {
                                     update_region( stack, data, state, i-1, x1, opaque.y1, opaque.x1-1, opaque.y2 );
                                }

                                /* upper */
                                if (opaque.y1 != y1)
                                     update_region( stack, data, state, i-1, x1, y1, x2, opaque.y1-1 );

                                /* right */
                                if (opaque.x2 != x2)
                                     update_region( stack, data, state, i-1, opaque.x2+1, opaque.y1, x2, opaque.y2 );

                                /* lower */
                                if (opaque.y2 != y2)
                                     update_region( stack, data, state, i-1, x1, opaque.y2+1, x2, y2 );
                           }
                     }
                    else {
                           //update un-conjected area aroud this window
                             /* left */
                             if (region.x1 != x1)
                                  update_region( stack, data, state, i-1, x1, region.y1, region.x1-1, region.y2 );

                             /* upper */
                             if (region.y1 != y1)
                                      update_region( stack, data, state, i-1, x1, y1, x2, region.y1-1 );

                             /* right */
                             if (region.x2 != x2)
                                  update_region( stack, data, state, i-1, region.x2+1, region.y1, x2, region.y2 );

                             /* lower */
                             if (region.y2 != y2)
                                  update_region( stack, data, state, i-1, x1, region.y2+1, x2, y2 );
                     }

                    /* left */
                    if (opaque.x1 != region.x1) {
                         DFBRegion r = { region.x1, opaque.y1, opaque.x1 - 1, opaque.y2 };
                         draw_window( window, state, &r, true, bypass_under_draw);
                    }

                    /* upper */
                    if (opaque.y1 != region.y1) {
                         DFBRegion r = { region.x1, region.y1, region.x2, opaque.y1 - 1 };
                         draw_window( window, state, &r, true , bypass_under_draw);
                    }

                    /* right */
                    if (opaque.x2 != region.x2) {
                         DFBRegion r = { opaque.x2 + 1, opaque.y1, region.x2, opaque.y2 };
                         draw_window( window, state, &r, true, bypass_under_draw );
                    }

                    /* lower */
                    if (opaque.y2 != region.y2) {
                         DFBRegion r = { region.x1, opaque.y2 + 1, region.x2, region.y2 };
                         draw_window( window, state, &r, true , bypass_under_draw);
                    }

                    /* inner */
                    draw_window( window, state, &opaque, false, bypass_under_draw );
               }
          }
          else {
                    if (!bypass_under_draw && TRANSLUCENT_WINDOW( window )) {
                         /* draw everything below */
                         update_region( stack, data, state, i-1, x1, y1, x2, y2 );
                    }
                    else {
                         /* left */
                         if (region.x1 != x1)
                              update_region( stack, data, state, i-1, x1, region.y1, region.x1-1, region.y2 );

                         /* upper */
                         if (region.y1 != y1)
                              update_region( stack, data, state, i-1, x1, y1, x2, region.y1-1 );

                         /* right */
                         if (region.x2 != x2)
                              update_region( stack, data, state, i-1, region.x2+1, region.y1, x2, region.y2 );

                         /* lower */
                         if (region.y2 != y2)
                              update_region( stack, data, state, i-1, x1, region.y2+1, x2, y2 );
                    }
                    draw_window( window, state, &region, true , bypass_under_draw);
          }
     }
     else
          draw_background( stack, state, &region );
}

/**************************************************************************************************/
/**************************************************************************************************/

static int count_visible_windows(CoreWindowStack *stack, StackData *data)
{
    if(stack == NULL || data == NULL)
        return 0;

    int i = 0;
    int count = 0;
    CoreWindow *window = NULL;

    fusion_vector_foreach(window, i, data->windows)
    {
        if(VISIBLE_WINDOW( window ))
            count++;
    }

    return count;
}

static void
repaint_stack( CoreWindowStack     *stack,
               StackData           *data,
               CoreLayerRegion     *region,
               const DFBRegion     *updates,
               int                  num_updates,
               DFBSurfaceFlipFlags  flags )
{
     int          i;
     CoreLayer   *layer;
     CardState   *state;
     CoreSurface *surface;
     DFBRegion    flips[num_updates];
     int          num_flips = 0;
     DFBRegion    clip = {0,0,0,0};
     DFBResult         ret;

     void * shareContext = 0;

#ifdef GET_COMPOSE_TIME
     long long comp_start = 0;
     long long comp_end = 0;
#endif
     D_ASSERT( stack != NULL );
     D_ASSERT( stack->context != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( region != NULL );
     D_ASSERT( num_updates > 0 );

     layer   = dfb_layer_at( stack->context->layer_id );
     state   = &layer->state;
     if ( LAYER_UP_SCALING_CHECK( stack->context->layer_id ) )
         surface = region->temp_surface;
     else
         surface = region->surface;



     if (!data->active || !surface)
          return;

     if(stack->display_mode!=DLDM_NORMAL &&
          region->config.buffermode==DLBM_FRONTONLY)
     {
         printf("Serious Error, 3D UI mode can not been enabled in under layer single buffer mode, switch back to normal display\n");
         stack->display_mode = DLDM_NORMAL;
     }
     D_DEBUG_AT( WM_Default, "repaint_stack( %d region(s), flags %x )\n", num_updates, flags );

#ifdef GET_COMPOSE_TIME
     comp_start = direct_clock_get_micros();
     printf("[%s][line=%d] : start time :%lld\n", __FUNCTION__, __LINE__,  comp_start );
#endif

     /* check to do GPU window compose or not. */
     if (dfb_config->mst_GPU_window_compose) {
         if (dfb_config->mst_GPU_AFBC) {
             /* enable AFBC, and do GPU window compose only in AFBC layer enable. */
             if (dfb_config->mst_AFBC_layer_enable & (1 << stack->context->layer_id))
                 doGPUCompose = true;
             else
                 doGPUCompose = false;
         }
         else /* just only GPU window compose, no AFBC */
             doGPUCompose = true;
     }
     else /* no any GPU compose */
         doGPUCompose = false;

     if (doGPUCompose) {
         DBG_GLES2_MSG("[DFB] %s, layer id = %d, do GPU Compose, sharecontext = %p\n", __FUNCTION__, stack->context->layer_id, shareContext);
         direct_mutex_lock(&dfb_config->GPU_compose_mutex);
         dfb_gfxcard_blit_gles2_makecurrent( state, &shareContext );
     }

     /* Set destination. */
     state->destination  = surface;
     state->modified    |= SMF_DESTINATION;

     for (i=0; i<num_updates; i++) {
          DFBRegion        dest;
          const DFBRegion *update = &updates[i];

          DFB_REGION_ASSERT( update );

          D_DEBUG_AT( WM_Default, "  -> %d, %d - %dx%d  (%d)\n",
                      DFB_RECTANGLE_VALS_FROM_REGION( update ), i );

          transform_stack_to_dest( stack, update, &dest );

          if (!dfb_region_intersect( &dest, 0, 0, surface->config.size.w - 1, surface->config.size.h - 1 ))
               continue;

          /* Set clipping region. */
          dfb_state_set_clip( state, &dest );

          /* Compose updated region. */
          update_region( stack, data, state,
                         fusion_vector_size( &data->windows ) - 1,
                         DFB_REGION_VALS( update ) );

          flips[num_flips++] = dest;

#if USE_CURSOR
          /* Update cursor? */
          if (data->cursor_drawn)
          {
               WindowSharedData* shared_data = stack->shared_data;
               /*
                   If use hw cursor, no need to do draw_cursor.
                   because the GOP which hw cursor uses is not as same as the UI. Therefore, it doesn't affect to each other.
               */
               if (!(shared_data && shared_data->bhwcursor))
               {
                   DFBRegion cursor_rotated;

                   D_ASSUME( data->cursor_bs_valid );

                   transform_stack_to_dest( stack, &data->cursor_region, &cursor_rotated );

                   if (dfb_region_region_intersect( &dest, &cursor_rotated ))
                   {
                        DFBRectangle rect = DFB_RECTANGLE_INIT_FROM_REGION( &dest );

                        dfb_gfx_copy_to( surface, data->cursor_bs, &rect,
                                         rect.x - cursor_rotated.x1,
                                         rect.y - cursor_rotated.y1, true );

                        draw_cursor( stack, data, state, &data->cursor_region );
                   }
               }
          }
#endif
     }

     /* Reset destination. */
     state->destination  = NULL;
     state->modified    |= SMF_DESTINATION;


     /* Software cursor code relies on a valid back buffer. */
     if (stack->cursor.enabled && dfb_config->mst_hw_cursor < 0)
          flags |= DSFLIP_BLIT;



    // union all the flip region to  one ,and set it as clip region
    if((stack->display_mode == DLDM_STEREO_LEFTRIGHT_3DUI)||
        (stack->display_mode == DLDM_STEREO_TOPBOTTOM_3DUI)||
        (stack->display_mode == DLDM_STEREO_LEFTRIGHT_TOPHALF_3DUI)||
        (stack->display_mode == DLDM_STEREO_TOPHALF_3DUI))
    {
        for (i=0; i<num_flips; i++)
        {
            DFBRegion *flip = &flips[i];

            DFB_REGION_ASSERT( flip );

            dfb_region_region_union(&clip,flip);
        }
    }
    else if(stack->display_mode == DLDM_NORMAL)
    {
        if(stack->bforce_fullupdate)
        {
            flags &= ~DSFLIP_BLIT;
            for (i=0; i<num_flips; i++)
            {
                DFBRegion *flip = &flips[i];
                DFB_REGION_ASSERT( flip );
                dfb_region_region_union(&clip,flip);
            }
        }
    }
    else if(stack->display_mode == DLDM_STEREO_FRAME_PACKING || stack->display_mode == DLDM_STEREO_FRAME_PACKING_FHD)
    {
        flags |= DSFLIP_BLIT;
    }

    
    if (doGPUCompose) {
        /* call for GPU window compose to finish. */
        DBG_GLES2_MSG("%s, %d, call glfinish, [pid=%d][tid=0x%x]\n", __FUNCTION__, __LINE__, getpid(), direct_gettid());
        dfb_gfxcard_blit_gles2_finish(state, shareContext);
        direct_waitqueue_broadcast(&dfb_config->GPU_compose_cond);
        direct_mutex_unlock(&dfb_config->GPU_compose_mutex);
    }
#ifdef GET_COMPOSE_TIME
    else
        dfb_gfxcard_sync();

    comp_end = direct_clock_get_micros();
    printf("[%s][line=%d] end time: %lld,  total time :%lld\n", __FUNCTION__, __LINE__, comp_end, (comp_end - comp_start) );
#endif


    switch(stack->display_mode)
    {
        case DLDM_STEREO_LEFTRIGHT_3DUI:
        {
             dfb_layer_buffer_lock( region, surface, CSBR_BACK );

             DFBRegion  flip = {0,0,surface->config.size.w-1,surface->config.size.h-1} ;
             DFBRegion regiondst;
             DFBRegion clipRegion;
             regiondst.x1 = 0;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w/2 -1;
             regiondst.y2= surface->config.size.h-1;

             clipRegion.x1 = clip.x1/2;
             clipRegion.x2 = (clip.x2+1)/2;
             clipRegion.y1 = clip.y1;
             clipRegion.y2 = clip.y2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);


             regiondst.x1 = surface->config.size.w/2;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w-1;
             regiondst.y2 = surface->config.size.h-1;

             clipRegion.x1 += surface->config.size.w/2;
             clipRegion.x2 += surface->config.size.w/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion, flags);

             /* Unlock region buffer since the lock is no longer needed. */
             dfb_layer_buffer_unlock(region, true);

             break;
        }

        case DLDM_STEREO_TOPBOTTOM_3DUI:
        {

             dfb_layer_buffer_lock( region, surface, CSBR_BACK );

             DFBRegion flip =  {0,0,surface->config.size.w-1,surface->config.size.h-1} ;
             DFBRegion regiondst;
             DFBRegion clipRegion;
             regiondst.x1 = 0;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w-1;
             regiondst.y2 =  surface->config.size.h/2-1;

             clipRegion.x1 = clip.x1;
             clipRegion.x2 = clip.x2;
             clipRegion.y1 = clip.y1/2;
             clipRegion.y2 = (clip.y2+1)/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);


             regiondst.x1 = 0;
             regiondst.y1 = surface->config.size.h/2;
             regiondst.x2 = surface->config.size.w-1;
             regiondst.y2 = surface->config.size.h-1;


             clipRegion.y1 += surface->config.size.h/2;
             clipRegion.y2 += surface->config.size.h/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);

             /* Unlock region buffer since the lock is no longer needed. */
             dfb_layer_buffer_unlock(region, true);

             break;
        }

        case DLDM_STEREO_LEFTRIGHT_TOPHALF_3DUI:
        {
             dfb_layer_buffer_lock( region, surface, CSBR_BACK );

             DFBRegion  flip = {0,0,surface->config.size.w-1,surface->config.size.h-1} ;
             DFBRegion regiondst;
             DFBRegion clipRegion;
             regiondst.x1 = 0;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w/2 -1;
             regiondst.y2= surface->config.size.h/2-1;

             clipRegion.x1 = clip.x1/2;
             clipRegion.x2 = (clip.x2+1)/2;
             clipRegion.y1 = clip.y1/2;
             clipRegion.y2 = (clip.y2+1)/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);


             regiondst.x1 = surface->config.size.w/2;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w-1;
             regiondst.y2 = surface->config.size.h/2-1;

             clipRegion.x1 += surface->config.size.w/2;
             clipRegion.x2 += surface->config.size.w/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion, flags);

             /* Unlock region buffer since the lock is no longer needed. */
             dfb_layer_buffer_unlock(region, true);

             break;
        }

        case DLDM_STEREO_TOPHALF_3DUI:
        {
             dfb_layer_buffer_lock( region, surface, CSBR_BACK );

             DFBRegion flip =  {0,0,surface->config.size.w-1,surface->config.size.h-1};
             DFBRegion regiondst;
             DFBRegion clipRegion;
             regiondst.x1 = 0;
             regiondst.y1 = 0;
             regiondst.x2 = surface->config.size.w-1;
             regiondst.y2 =  surface->config.size.h/2-1;

             clipRegion.x1 = clip.x1;
             clipRegion.x2 = clip.x2;
             clipRegion.y1 = clip.y1/2;
             clipRegion.y2 = (clip.y2+1)/2;
             dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);

             /* Unlock region buffer since the lock is no longer needed. */
             dfb_layer_buffer_unlock(region, true);

             break;
        }

        case DLDM_NORMAL:
        {
             if (dfb_config->mst_enable_layer_autoscaledown == true)
             {
                  int dst_width  = region->config.dest.w;
                  int dst_height = region->config.dest.h;
                  int src_width  =  surface->config.size.w;
                  int src_height = surface->config.size.h;

                  D_INFO("[DFB] %s, screen width:%d, height:%d\n", __FUNCTION__, dst_width, dst_height);

                  if (dst_width < src_width && dst_height < src_height)
                  {
                      D_INFO("[DFB] surface->config.size.w:%d, surface->config.size.h:%d \n", src_width, src_height);

                      dfb_layer_buffer_lock( region, surface, CSBR_BACK );

                      DFBRegion flip =  {0, 0, src_width-1, src_height-1} ;
                      DFBRegion regiondst = {0, 0, dst_width-1, dst_height-1};
                      DFBRegion clipRegion;

                      clipRegion.x1 = regiondst.x1;
                      clipRegion.x2 = regiondst.x2;
                      clipRegion.y1 = regiondst.y1;
                      clipRegion.y2 = regiondst.y2;

                      D_INFO("[DFB] ne clip [%d, %d - %dx%d]\n", DFB_RECTANGLE_VALS_FROM_REGION(&clipRegion));

                      dfb_layer_region_stretchblit_update_with_clipRegion(region, &regiondst, &flip, &clipRegion,flags);

                      /* Unlock region buffer since the lock is no longer needed. */
                      dfb_layer_buffer_unlock(region, true);

                      break;
                  }
             }
        }
        case DLDM_STEREO_FRAME_PACKING:
        case DLDM_STEREO_FRAME_PACKING_FHD:
        {
            if(stack->bforce_fullupdate)
            {
                dfb_layer_region_flip_update( region, &clip, flags );
            }
            else
            {
                if (dfb_config->mst_enable_gop_gwin_partial_update)
                {
                      DFBRegion bound = {0, 0, 0, 0};
                      DFBRectangle rect_union = {0, 0, 0, 0};
                      if (stack->cursor.enabled || stack->bg.mode >= DLBM_IMAGE)
                      {
                              D_INFO("[DFB] cursor is enabled\n");
                              bound.x1 = 0;
                              bound.y1 = 0;
                              bound.x2 = region->config.width;
                              bound.y2 = region->config.height;
                      }
                      else
                      {
                             for(i = 0; i < fusion_vector_size( &data->windows ); i++)
                             {
                                  CoreWindow *window = fusion_vector_at( &data->windows, i );
                                  if (VISIBLE_WINDOW( window ))
                                  {
                                      dfb_rectangle_union(&rect_union, &window->config.bounds);
                                  }
                             }

                             bound =  DFB_REGION_INIT_FROM_RECTANGLE(&rect_union);
                       }

                      for (i=0; i<num_flips; i++)
                      {
                          DFBRegion *flip = &flips[i];
                          dfb_layer_region_flip_update_with_boundary( region, flip, &bound, flags );
                      }

                }
                else
                {
                       for (i=0; i<num_flips; i++)
                        {
                          DFBRegion *flip = &flips[i];
                          dfb_layer_region_flip_update( region, flip, flags );
                        }
                }
            }

            break;
        }

        default :
        {
            printf("\nerror display mode\n");
            break ;
        }

    }


}

static DFBResult
process_updates( StackData           *data,
                 WMData              *wmdata,
                 CoreWindowStack     *stack,
                 CoreLayerRegion     *region,
                 DFBSurfaceFlipFlags  flags )
{
     DFBResult         ret;
     int               n, d;
     int               total;
     int               bounding;
     CoreLayerContext *context;
     CoreLayerRegion  *primary = region;

     D_ASSERT( data != NULL );
     D_ASSERT( wmdata != NULL );
     D_ASSERT( stack != NULL );

     context = stack->context;
     D_ASSERT( context != NULL );

     if (!data->updates.num_regions)
          return DFB_OK;

     /* Get the primary region. */
     if (!region) {
          ret = dfb_layer_context_get_primary_region( stack->context, false, &primary );
          if (ret)
               return ret;
     }


     dfb_updates_stat( &data->updates, &total, &bounding );

     n = data->updates.max_regions - data->updates.num_regions + 1;
     d = n + 1;

     /* FIXME: depend on buffer mode, hw accel etc. */
     if (total > stack->width * stack->height *  dfb_config->full_update_numerator / dfb_config->full_update_denominator ) {
          DFBRegion region = { 0, 0, stack->width - 1, stack->height - 1 };

//          direct_log_printf( NULL, "%s() <- %d regions, total %d, bounding %d (%d/%d: %d), FULL UPDATE\n",
//                             __FUNCTION__, data->updates.num_regions, total, bounding, n, d, bounding*n/d );

//          if (context->config.buffermode == DLBM_FRONTONLY)
//               dfb_region_transpose(&region, context->rotation);

          repaint_stack( stack, data, primary, &region, 1, flags );
     }
     else if (data->updates.num_regions < 2 || total < bounding * n / d)
          repaint_stack( stack, data, primary, data->updates.regions, data->updates.num_regions, flags );
     else {
//          direct_log_printf( NULL, "%s() <- %d regions, total %d, bounding %d (%d/%d: %d)\n",
//                             __FUNCTION__, data->updates.num_regions, total, bounding, n, d, bounding*n/d );

          repaint_stack( stack, data, primary, &data->updates.bounding, 1, flags );
     }


     dfb_updates_reset( &data->updates );

     /* Unref primary region. */
     if (!region)
          dfb_layer_region_unref( primary );

     return DFB_OK;
}

/*
     skipping opaque windows that are above the window that changed
*/
static void
wind_of_change( CoreWindowStack     *stack,
                StackData           *data,
                CoreLayerRegion     *region,
                DFBRegion           *update,
                DFBSurfaceFlipFlags  flags,
                int                  current,
                int                  changed )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( region != NULL );
     D_ASSERT( update != NULL );

     /*
          loop through windows above
     */
     for (; current > changed; current--) {
          CoreWindow       *window;
          CoreWindowConfig *config;
          DFBRegion         opaque;
          DFBRectangle      rotated;
          DFBRectangle     *bounds = &rotated;
          DFBWindowOptions  options;

          D_ASSERT( changed >= 0 );
          D_ASSERT( current >= changed );
          D_ASSERT( current < fusion_vector_size( &data->windows ) );

          window  = fusion_vector_at( &data->windows, current );
          config  = &window->config;
          options = config->options;

          transform_window_to_stack( window, &config->bounds, &rotated );

          /*
               can skip opaque region
          */
          if ((
              //can skip all opaque window?
              (config->opacity == 0xff) &&
              !(options & (DWOP_COLORKEYING | DWOP_ALPHACHANNEL)) &&
              (opaque=*update,dfb_region_intersect( &opaque,
                                                    bounds->x, bounds->y,
                                                    bounds->x + bounds->w - 1,
                                                    bounds->y + bounds->h -1 ) )
              )||(
                 //can skip opaque region?
                 (options & DWOP_ALPHACHANNEL) &&
                 (options & DWOP_OPAQUE_REGION) &&
                 (config->opacity == 0xff) &&
                 !(options & DWOP_COLORKEYING) &&
                 (opaque=*update,dfb_region_intersect( &opaque,
                                                       bounds->x + config->opaque.x1,
                                                       bounds->y + config->opaque.y1,
                                                       bounds->x + config->opaque.x2,
                                                       bounds->y + config->opaque.y2 ))
                 ))
          {
               /* left */
               if (opaque.x1 != update->x1) {
                    DFBRegion left = { update->x1, opaque.y1, opaque.x1-1, opaque.y2};
                    wind_of_change( stack, data, region, &left, flags, current-1, changed );
               }
               /* upper */
               if (opaque.y1 != update->y1) {
                    DFBRegion upper = { update->x1, update->y1, update->x2, opaque.y1-1};
                    wind_of_change( stack, data, region, &upper, flags, current-1, changed );
               }
               /* right */
               if (opaque.x2 != update->x2) {
                    DFBRegion right = { opaque.x2+1, opaque.y1, update->x2, opaque.y2};
                    wind_of_change( stack, data, region, &right, flags, current-1, changed );
               }
               /* lower */
               if (opaque.y2 != update->y2) {
                    DFBRegion lower = { update->x1, opaque.y2+1, update->x2, update->y2};
                    wind_of_change( stack, data, region, &lower, flags, current-1, changed );
               }

               return;
          }
     }

     dfb_updates_add( &data->updates, update );
}

static void
repaint_stack_for_window( CoreWindowStack     *stack,
                          StackData           *data,
                          CoreLayerRegion     *region,
                          DFBRegion           *update,
                          DFBSurfaceFlipFlags  flags,
                          int                  window )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( region != NULL );
     D_ASSERT( update != NULL );
     D_ASSERT( window >= 0 );
     D_ASSERT( window < fusion_vector_size( &data->windows ) );

     if (fusion_vector_has_elements( &data->windows ) && window >= 0) {
          int num = fusion_vector_size( &data->windows );

          D_ASSERT( window < num );

          wind_of_change( stack, data, region, update, flags, num - 1, window );
     }
     else
          dfb_updates_add( &data->updates, update );
}

/**************************************************************************************************/

static DFBResult
update_window( CoreWindow          *window,
               WindowData          *window_data,
               const DFBRegion     *region,
               DFBSurfaceFlipFlags  flags,
               bool                 force_complete,
               bool                 force_invisible,
               bool                 scale_region )
{
     DFBRegion        area;
     DFBRegion        update;
     StackData       *data;
     CoreWindowStack *stack;
     DFBRectangle    *bounds;
     DFBDimension     size;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );
     D_ASSERT( window_data->stack_data->stack != NULL );

     DFB_REGION_ASSERT_IF( region );

     data  = window_data->stack_data;
     stack = data->stack;

     if (!VISIBLE_WINDOW(window) && !force_invisible)
          return DFB_OK;

     if (stack->hw_mode)
          return DFB_OK;

     if ( (window->config.options & DWOP_SCALE) &&
	      (window->config.options & DWOP_SCALE_WINDOW_AUTOFIT_LAYER)) {
          window->config.bounds.w = stack->width;
          window->config.bounds.h = stack->height;
     }

     bounds = &window->config.bounds;
     size.w = bounds->w;
     size.h = bounds->h;

     if (region) {
          if (scale_region && (window->config.options & DWOP_SCALE)) {
               int sw = window->surface->config.size.w;
               int sh = window->surface->config.size.h;

            #if 0 
                 area.x1 = 0 ;
                 area.y1 = 0 ;
                 area.x2 = bounds->w -1 ;
                 area.y2 = bounds->h -1;
                 printf("\ncoming to use the update all\n");
               
            #else
               /* horizontal */
    
               if (bounds->w > sw) {
                    /* upscaling */
                    area.x1 = (region->x1 - 1) * bounds->w / sw ;
                    area.x2 = (region->x2 + 1) * bounds->w / sw ;
               }
               else {
                    /* downscaling */
                    area.x1 = region->x1 * bounds->w / sw - 1;
                    area.x2 = region->x2 * bounds->w / sw + 1 ;
               }

               /* vertical */
               if (bounds->h > sh) {
                    /* upscaling */
                    area.y1 = (region->y1 - 1) * bounds->h / sh ;
                    area.y2 = (region->y2 + 1) * bounds->h / sh ;
               }
               else {
                    /* downscaling */
                    area.y1 = region->y1 * bounds->h / sh - 1 ;
                    area.y2 = region->y2 * bounds->h / sh + 1;
               }
               
               if (false == dfb_config->mst_disable_window_scale_patch) 
               {
                       area.x1 = area.x1 -2; 
                       area.x2 = area.x2 +2; 
                       area.y1 = area.y1 -2; 
                       area.y2 = area.y2 +2; 
                       //printf("\ncoming to use the patch 1\n");
               }

                   
              #endif

               /* limit to window area */
               dfb_region_clip( &area, 0, 0, bounds->w - 1, bounds->h - 1 );
          }
          else
               area = *region;
     }
     else {
          area.x1 = area.y1 = 0;
          area.x2 = bounds->w - 1;
          area.y2 = bounds->h - 1;
     }

     dfb_region_from_rotated( &update, &area, &size, window->config.rotation );

     /* screen offset */
     dfb_region_translate( &update, bounds->x, bounds->y );

     if (!dfb_unsafe_region_intersect( &update, 0, 0, stack->width - 1, stack->height - 1 ))
          return DFB_OK;

     if (force_complete)
          dfb_updates_add( &data->updates, &update );
     else
          repaint_stack_for_window( stack, data, window->primary_region,
                                    &update, flags, get_index( data, window ) );

     return DFB_OK;
}

/**************************************************************************************************/
/**************************************************************************************************/

static void
insert_window( CoreWindowStack *stack,
               StackData       *data,
               CoreWindow      *window,
               WindowData      *window_data )
{
     int         index;
     CoreWindow *other;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );

     /*
      * Iterate from bottom to top,
      * stopping at the first window with a higher priority.
      */
     fusion_vector_foreach (other, index, data->windows) {
          WindowData *other_data = other->window_data;

          D_ASSERT( other->window_data != NULL );

          if (other_data->priority > window_data->priority)
               break;
     }

     /* Insert the window at the acquired position. */
     fusion_vector_insert( &data->windows, window, index );
}

static void
withdraw_window( CoreWindowStack *stack,
                 StackData       *data,
                 CoreWindow      *window,
                 WindowData      *window_data )
{
     int i;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );

     D_ASSERT( window->stack != NULL );

     D_ASSERT( DFB_WINDOW_INITIALIZED( window ) );

     /* No longer be the 'entered window'. */
     if (data->entered_window == window)
          data->entered_window = NULL;

     /* Remove focus from window. */
     if (data->focused_window == window)
          data->focused_window = NULL;

     /* Release explicit keyboard grab. */
     if (data->keyboard_window == window)
          data->keyboard_window = NULL;

     /* Release explicit pointer grab. */
     if (data->pointer_window == window)
          data->pointer_window = NULL;

     /* Release all implicit key grabs. */
     for (i=0; i<MAX_KEYS; i++) {
          if (data->keys[i].code != -1 && data->keys[i].owner == window) {
               if (!DFB_WINDOW_DESTROYED( window )) {
                    DFBWindowEvent we;

                    we.type       = DWET_KEYUP;
                    we.key_code   = data->keys[i].code;
                    we.key_id     = data->keys[i].id;
                    we.key_symbol = data->keys[i].symbol;

                    post_event( window, data, &we );
               }

               data->keys[i].code  = -1;
               data->keys[i].owner = NULL;
          }
     }

     /* Release grab of unselected keys. */
     if (data->unselkeys_window == window)
          data->unselkeys_window = NULL;
}

static void
remove_window( CoreWindowStack *stack,
               StackData       *data,
               CoreWindow      *window,
               WindowData      *window_data )
{
     DirectLink *l, *n;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );

     D_ASSERT( window->config.opacity == 0 );
     D_ASSERT( DFB_WINDOW_INITIALIZED( window ) );

     D_ASSERT( fusion_vector_contains( &data->windows, window ) );

     /* Release implicit grabs, focus etc. */
     withdraw_window( stack, data, window, window_data );

     /* Release all explicit key grabs. */
     direct_list_foreach_safe (l, n, data->grabbed_keys) {
          GrabbedKey *key = (GrabbedKey*) l;

          if (key->owner == window) {
               direct_list_remove( &data->grabbed_keys, &key->link );
               SHFREE( stack->shmpool, key );
          }
     }

     fusion_vector_remove( &data->windows, fusion_vector_index_of( &data->windows, window ) );
}

/**************************************************************************************************/

static DFBResult
move_window( CoreWindow *window,
             WindowData *data,
             int         dx,
             int         dy )
{
     DFBResult       ret;
     DFBWindowEvent  evt;
     DFBRectangle   *bounds = &window->config.bounds;

     if (window->region) {
          data->config.dest.x += dx;
          data->config.dest.y += dy;

          ret = dfb_layer_region_set_configuration( window->region, &data->config, CLRCF_DEST );
          if (ret) {
               data->config.dest.x -= dx;
               data->config.dest.y -= dy;

               return ret;
          }

          bounds->x += dx;
          bounds->y += dy;
     }
     else {
          update_window( window, data, NULL, 0, false, false, false );

          bounds->x += dx;
          bounds->y += dy;

          update_window( window, data, NULL, 0, false, false, false );
     }

     /* Send new position */
     evt.type = DWET_POSITION;
     evt.x    = bounds->x;
     evt.y    = bounds->y;

     post_event( window, data->stack_data, &evt );

     return DFB_OK;
}

static DFBResult
resize_window( CoreWindow *window,
               WMData     *wm_data,
               WindowData *data,
               int         width,
               int         height )
{
     DFBResult        ret;
     DFBWindowEvent   evt;
     CoreWindowStack *stack  = window->stack;
     DFBRectangle    *bounds = &window->config.bounds;
     int              ow     = bounds->w;
     int              oh     = bounds->h;

     D_DEBUG_AT( WM_Default, "resize_window( %d, %d )\n", width, height );

     D_ASSERT( wm_data != NULL );

     D_MAGIC_ASSERT( data, WindowData );

     D_ASSERT( width > 0 );
     D_ASSERT( height > 0 );

     if (width > dfb_config->mst_ge_hw_limit || height > dfb_config->mst_ge_hw_limit || width <= 0 || height <= 0)
     {
          D_ERROR("[DFB] (%s line %d) width = %d, height = %d, mst_ge_hw_limit = %d",__FUNCTION__,__LINE__, width, height, dfb_config->mst_ge_hw_limit);
          return DFB_LIMITEXCEEDED;
     }

     if (window->surface && !(window->config.options & DWOP_SCALE)) {
          CoreSurfaceConfig config;
          memset(&config, 0, sizeof(CoreSurfaceConfig)); //Fix converity END
          config.flags  = CSCONF_SIZE;
          config.size.w = width;
          config.size.h = height;
          

          ret = dfb_surface_reconfig( window->surface, &config );
          if (ret)
               return ret;
     }

     if (window->region) {
          data->config.dest.w = data->config.source.w = data->config.width  = width;
          data->config.dest.h = data->config.source.h = data->config.height = height;

          ret = dfb_layer_region_set_configuration( window->region, &data->config,
                                                    CLRCF_WIDTH | CLRCF_HEIGHT | CLRCF_SURFACE |
                                                    CLRCF_DEST  | CLRCF_SOURCE );
          if (ret) {
               data->config.dest.w = data->config.source.w = data->config.width  = bounds->w = ow;
               data->config.dest.h = data->config.source.h = data->config.height = bounds->h = oh;

               return ret;
          }
     }
     else {
          dfb_region_intersect( &window->config.opaque, 0, 0, width - 1, height - 1 );

          if (VISIBLE_WINDOW (window)) {
               if (ow > width) {
                    DFBRegion region = { width, 0, ow - 1, MIN(height, oh) - 1 };

                    update_window( window, data, &region, 0, false, false, false );
               }

               if (oh > height) {
                    DFBRegion region = { 0, height, MAX(width, ow) - 1, oh - 1 };

                    update_window( window, data, &region, 0, false, false, false );
               }
          }
     }

     bounds->w = width;
     bounds->h = height;

     /* Send new size */
     evt.type = DWET_SIZE;
     evt.w    = bounds->w;
     evt.h    = bounds->h;

     post_event( window, data->stack_data, &evt );

     update_focus( stack, data->stack_data, wm_data );

     return DFB_OK;
}

static DFBResult
set_window_bounds( CoreWindow *window,
                   WMData     *wm_data,
                   WindowData *data,
                   int         x,
                   int         y,
                   int         width,
                   int         height)
{
     DFBResult        ret;
     DFBWindowEvent   evt;
     CoreWindowStack *stack = window->stack;
     DFBRegion        old_region;
     DFBRegion        new_region;

     D_DEBUG_AT( WM_Default, "%s( %p [%d] %d, %d - %dx%d )\n", __FUNCTION__, window, window->id, x, y, width, height );

     D_ASSERT( wm_data != NULL );

     D_MAGIC_ASSERT( data, WindowData );

     D_ASSERT( width > 0 );
     D_ASSERT( height > 0 );

     if (width > dfb_config->mst_ge_hw_limit || height > dfb_config->mst_ge_hw_limit || width <= 0 || height <= 0)
     {
          D_ERROR("[DFB] (%s line %d) width = %d, height = %d, mst_ge_hw_limit = %d",__FUNCTION__,__LINE__, width, height, dfb_config->mst_ge_hw_limit);
          return DFB_LIMITEXCEEDED;
     }

     if (window->surface && !(window->config.options & DWOP_SCALE)) {
          ret = dfb_surface_reformat( window->surface,
                                      width, height, window->surface->config.format );
          if (ret)
               return ret;
     }

     old_region.x1 = window->config.bounds.x - x;
     old_region.y1 = window->config.bounds.y - y;
     old_region.x2 = old_region.x1 + window->config.bounds.w - 1;
     old_region.y2 = old_region.y1 + window->config.bounds.h - 1;

     window->config.bounds.x = x;
     window->config.bounds.y = y;
     window->config.bounds.w = width;
     window->config.bounds.h = height;

     new_region.x1 = 0;
     new_region.y1 = 0;
     new_region.x2 = width  - 1;
     new_region.y2 = height - 1;

     if (!dfb_region_region_intersect( &window->config.opaque, &new_region ))
          window->config.opaque = new_region;

     /* Update exposed area. */
     if (VISIBLE_WINDOW( window )) {
          if (dfb_region_region_intersect( &new_region, &old_region )) {
               /* left */
               if (new_region.x1 > old_region.x1) {
                    DFBRegion region = { old_region.x1, old_region.y1,
                                         new_region.x1 - 1, new_region.y2 };

                    update_window( window, data, &region, 0, false, false, false );
               }

               /* upper */
               if (new_region.y1 > old_region.y1) {
                    DFBRegion region = { old_region.x1, old_region.y1,
                                         old_region.x2, new_region.y1 - 1 };

                    update_window( window, data, &region, 0, false, false, false );
               }

               /* right */
               if (new_region.x2 < old_region.x2) {
                    DFBRegion region = { new_region.x2 + 1, new_region.y1,
                                         old_region.x2, new_region.y2 };

                    update_window( window, data, &region, 0, false, false, false );
               }

               /* lower */
               if (new_region.y2 < old_region.y2) {
                    DFBRegion region = { old_region.x1, new_region.y2 + 1,
                                         old_region.x2, old_region.y2 };

                    update_window( window, data, &region, 0, false, false, false );
               }
          }
          else
               update_window( window, data, &old_region, 0, false, false, false );
     }

     /* Send new position and size */
     evt.type = DWET_POSITION_SIZE;
     evt.x    = window->config.bounds.x;
     evt.y    = window->config.bounds.y;
     evt.w    = window->config.bounds.w;
     evt.h    = window->config.bounds.h;

     post_event( window, data->stack_data, &evt );

     update_focus( stack, data->stack_data, wm_data );

     return DFB_OK;
}

static DFBResult
restack_window( CoreWindow             *window,
                WindowData             *window_data,
                CoreWindow             *relative,
                WindowData             *relative_data,
                int                     relation,
                DFBWindowStackingClass  stacking )
{
     StackData *data;
     int        old;
     int        index;
     int        priority;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     D_ASSERT( relative == NULL || relative_data != NULL );

     D_ASSERT( relative == NULL || relative == window || relation != 0);

     data = window_data->stack_data;

     /* Change stacking class. */
     if (stacking != window->config.stacking) {
          window->config.stacking = stacking;

          window_data->priority = get_priority( window );
     }

     /* Get the (new) priority. */
     priority = window_data->priority;

     /* Get the old index. */
     old = get_index( data, window );

     /* Calculate the desired index. */
     if (relative) {
          index = get_index( data, relative );

          if (relation > 0) {
               if (old < index)
                    index--;
          }
          else if (relation < 0) {
               if (old > index)
                    index++;
          }

          index += relation;

          if (index < 0)
               index = 0;
          else if (index > fusion_vector_size( &data->windows ) - 1)
               index = fusion_vector_size( &data->windows ) - 1;
     }
     else if (relation > 0)
          index = fusion_vector_size( &data->windows ) - 1;
     else
          index = 0;

     /* Assure window won't be above any window with a higher priority. */
     while (index > 0) {
          int         below      = (old < index) ? index : index - 1;
          CoreWindow *other      = fusion_vector_at( &data->windows, below );
          WindowData *other_data = other->window_data;

          D_ASSERT( other->window_data != NULL );

          if (priority < other_data->priority)
               index--;
          else
               break;
     }

     /* Assure window won't be below any window with a lower priority. */
     while (index < fusion_vector_size( &data->windows ) - 1) {
          int         above      = (old > index) ? index : index + 1;
          CoreWindow *other      = fusion_vector_at( &data->windows, above );
          WindowData *other_data = other->window_data;

          D_ASSERT( other->window_data != NULL );

          if (priority > other_data->priority)
               index++;
          else
               break;
     }

     /* Return if index hasn't changed. */
     if (index == old)
          return DFB_OK;

     /* Actually change the stacking order now. */
     fusion_vector_move( &data->windows, old, index );

     update_window( window, window_data, NULL, DSFLIP_NONE, (index < old), false, false );

     return DFB_OK;
}

static void
set_opacity( CoreWindow                             *window,
             WindowData                             *window_data,
             WMData                                 *wmdata,
             u8                                      opacity )
{
    u8               old;
    StackData       *data;
    CoreWindowStack *stack;

    D_ASSERT( window != NULL );
    D_ASSERT( window_data != NULL );
    D_ASSERT( window_data->stack_data != NULL );
    D_ASSERT( window_data->stack_data->stack != NULL );

    old   = window->config.opacity;
    data  = window_data->stack_data;
    stack = data->stack;

    if (!stack->hw_mode && !dfb_config->translucent_windows && opacity)
        opacity = 0xFF;

    if (old != opacity)
    {
        bool show = !old && opacity;
        bool hide = old && !opacity;

        window->config.opacity = opacity;

        if (window->region)
        {
            window_data->config.opacity = opacity;

            dfb_layer_region_set_configuration( window->region, &window_data->config, CLRCF_OPACITY );
        }
        else
            update_window( window, window_data, NULL, DSFLIP_NONE, false, true, false );


        /* Check focus after window appeared or disappeared */
        if (show || hide)
            update_focus( stack, data, wmdata );

        /* If window disappeared... */
        if (hide)
        {
            /* Ungrab pointer/keyboard */
            withdraw_window( stack, data, window, window_data );

            /* Always try to have a focused window */
            ensure_focus( stack, data );
        }
    }
}

static DFBResult
grab_keyboard( CoreWindow *window,
               WindowData *window_data )
{
     StackData *data;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     data = window_data->stack_data;

     if (data->keyboard_window)
          return DFB_LOCKED;

     data->keyboard_window = window;

     return DFB_OK;
}

static DFBResult
ungrab_keyboard( CoreWindow *window,
                 WindowData *window_data )
{
     StackData *data;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     data = window_data->stack_data;

     if (data->keyboard_window == window)
          data->keyboard_window = NULL;

     return DFB_OK;
}

static DFBResult
grab_pointer( CoreWindow *window,
              WindowData *window_data )
{
     StackData *data;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     data = window_data->stack_data;

     if (data->pointer_window)
          return DFB_LOCKED;

     data->pointer_window = window;

     return DFB_OK;
}

static DFBResult
ungrab_pointer( CoreWindow *window,
                WindowData *window_data,
                WMData     *wmdata )
{
     StackData       *data;
     CoreWindowStack *stack;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     data  = window_data->stack_data;
     stack = data->stack;

     if (data->pointer_window == window) {
          data->pointer_window = NULL;

          /* Possibly change focus to window that's now under the cursor */
          update_focus( stack, data, wmdata );
     }

     return DFB_OK;
}

static DFBResult
grab_key( CoreWindow                 *window,
          WindowData                 *window_data,
          DFBInputDeviceKeySymbol     symbol,
          DFBInputDeviceModifierMask  modifiers )
{
     int              i;
     StackData       *data;
     GrabbedKey      *grab;
     CoreWindowStack *stack;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );
     D_ASSERT( window_data->stack_data->stack != NULL );

     data  = window_data->stack_data;
     stack = data->stack;

     /* Reject if already grabbed. */
     direct_list_foreach (grab, data->grabbed_keys)
          if (grab->symbol == symbol && grab->modifiers == modifiers)
               return DFB_LOCKED;

     /* Allocate grab information. */
     grab = SHCALLOC( stack->shmpool, 1, sizeof(GrabbedKey) );

     /* Fill grab information. */
     grab->symbol    = symbol;
     grab->modifiers = modifiers;
     grab->owner     = window;

     /* Add to list of key grabs. */
     direct_list_append( &data->grabbed_keys, &grab->link );

     /* Remove implicit grabs for this key. */
     for (i=0; i<MAX_KEYS; i++)
          if (data->keys[i].code != -1 && data->keys[i].symbol == symbol)
               data->keys[i].code = -1;

     return DFB_OK;
}

static DFBResult
ungrab_key( CoreWindow                 *window,
            WindowData                 *window_data,
            DFBInputDeviceKeySymbol     symbol,
            DFBInputDeviceModifierMask  modifiers )
{
     DirectLink      *l;
     StackData       *data;
     CoreWindowStack *stack;

     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );
     D_ASSERT( window_data->stack_data->stack != NULL );

     data  = window_data->stack_data;
     stack = data->stack;

     direct_list_foreach (l, data->grabbed_keys) {
          GrabbedKey *key = (GrabbedKey*) l;

          if (key->symbol == symbol && key->modifiers == modifiers && key->owner == window) {
               direct_list_remove( &data->grabbed_keys, &key->link );
               SHFREE( stack->shmpool, key );
               return DFB_OK;
          }
     }

     return DFB_IDNOTFOUND;
}

static DFBResult
request_focus( CoreWindow *window,
               WindowData *window_data )
{
     StackData       *data;
     CoreWindowStack *stack;
     CoreWindow      *entered;

     D_ASSERT( window != NULL );
     D_ASSERT( !(window->config.options & DWOP_GHOST) );
     D_ASSERT( window_data != NULL );
     D_ASSERT( window_data->stack_data != NULL );

     data  = window_data->stack_data;
     stack = data->stack;

     switch_focus( stack, data, window );

     entered = data->entered_window;

     if (entered && entered != window) {
          DFBWindowEvent we;

          we.type = DWET_LEAVE;
          we.x    = stack->cursor.x - entered->config.bounds.x;
          we.y    = stack->cursor.y - entered->config.bounds.y;

          transform_point_in_window( entered, &we.x, &we.y );

          post_event( entered, data, &we );

          data->entered_window = NULL;
     }

     return DFB_OK;
}

/**************************************************************************************************/
/**************************************************************************************************/

static bool
handle_wm_key( CoreWindowStack     *stack,
               StackData           *data,
               WMData              *wmdata,
               const DFBInputEvent *event )
{
     int         i, num;
     CoreWindow *entered;
     CoreWindow *focused;
     CoreWindow *window;
     DFBRegion   region;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( data->wm_level > 0 );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_KEYPRESS );

     entered = data->entered_window;
     focused = data->focused_window;

     switch (DFB_LOWER_CASE(event->key_symbol)) {
          case DIKS_SMALL_X:
               num = fusion_vector_size( &data->windows );

               if (data->wm_cycle <= 0)
                    data->wm_cycle = num;

               if (num) {
                    int looped = 0;
                    int index  = MIN( num, data->wm_cycle );

                    while (index--) {
                         CoreWindow *window = fusion_vector_at( &data->windows, index );

                         if ((window->config.options & (DWOP_GHOST | DWOP_KEEP_STACKING)) ||
                             ! VISIBLE_WINDOW(window) || window == data->focused_window)
                         {
                              if (index == 0 && !looped) {
                                   looped = 1;
                                   index  = num - 1;
                              }

                              continue;
                         }

                         restack_window( window, window->window_data,
                                         NULL, NULL, 1, window->config.stacking );
                         request_focus( window, window->window_data );

                         break;
                    }

                    data->wm_cycle = index;
               }
               break;

          case DIKS_SMALL_S:
               fusion_vector_foreach (window, i, data->windows) {
                    if (VISIBLE_WINDOW(window) && window->config.stacking == DWSC_MIDDLE &&
                       ! (window->config.options & (DWOP_GHOST | DWOP_KEEP_STACKING)))
                    {
                         restack_window( window, window->window_data,
                                         NULL, NULL, 1, window->config.stacking );
                         request_focus( window, window->window_data );

                         break;
                    }
               }
               break;

          case DIKS_SMALL_C:
               if (entered) {
                    DFBWindowEvent event;

                    event.type = DWET_CLOSE;

                    post_event( entered, data, &event );
               }
               break;

          case DIKS_SMALL_E:
               update_focus( stack, data, wmdata );
               break;

          case DIKS_SMALL_A:
               if (focused && !(focused->config.options & DWOP_KEEP_STACKING)) {
                    restack_window( focused, focused->window_data,
                                    NULL, NULL, 0, focused->config.stacking );
                    update_focus( stack, data, wmdata );
               }
               break;

          case DIKS_SMALL_W:
               if (focused && !(focused->config.options & DWOP_KEEP_STACKING))
                    restack_window( focused, focused->window_data,
                                    NULL, NULL, 1, focused->config.stacking );
               break;

          case DIKS_SMALL_D:
               if (entered && !(entered->config.options & DWOP_INDESTRUCTIBLE))
                    dfb_window_destroy( entered );

               break;

          case DIKS_SMALL_P:
               /* Enable and show cursor. */
               if (stack->cursor.set) {
                    dfb_windowstack_cursor_set_opacity( stack, 0xff );
                    dfb_windowstack_cursor_enable( wmdata->core, stack, true );
               }

               /* Ungrab pointer. */
               data->pointer_window = NULL;

               /* TODO: set new cursor shape, the current one might be completely transparent */
               break;

          case DIKS_SMALL_R:
               if (focused && !(focused->config.options & DWOP_KEEP_POSITION))
                    dfb_window_set_rotation( focused, (focused->config.rotation + 90) % 360 );
               break;

          case DIKS_PRINT:
               if (dfb_config->screenshot_dir && focused && focused->surface)
                    dfb_surface_dump_buffer( focused->surface, CSBR_FRONT, dfb_config->screenshot_dir, "dfb_window" );
               break;

          case DIKS_F12:
               region.x1 = 0;
               region.y1 = 0;
               region.x2 = stack->width;
               region.y2 = stack->height;

               dfb_updates_reset( &data->updates );
               dfb_updates_add( &data->updates, &region );
               break;

          default:
               return false;
     }

     return true;
}

static bool
is_wm_key( DFBInputDeviceKeySymbol key_symbol )
{
     switch (DFB_LOWER_CASE(key_symbol)) {
          case DIKS_SMALL_X:
          case DIKS_SMALL_S:
          case DIKS_SMALL_C:
          case DIKS_SMALL_E:
          case DIKS_SMALL_A:
          case DIKS_SMALL_W:
          case DIKS_SMALL_D:
          case DIKS_SMALL_P:
          case DIKS_SMALL_R:
          case DIKS_PRINT:
               break;

          default:
               return false;
     }

     return true;
}


/**************************************************************************************************/

static DFBResult
handle_key_press( CoreWindowStack     *stack,
                  StackData           *data,
                  WMData              *wmdata,
                  const DFBInputEvent *event )
{
     CoreWindow *window;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_KEYPRESS );

     if (data->wm_level) {
          switch (event->key_symbol) {
               case DIKS_META:
                    data->wm_level |= 1;
                    break;

               case DIKS_CONTROL:
                    data->wm_level |= 2;
                    break;

               case DIKS_ALT:
                    data->wm_level |= 4;
                    break;

               default:
                    if (dfb_config->mst_handle_wm_key) {
                         if (handle_wm_key( stack, data, wmdata, event ))
                              return DFB_OK;
                    }
                    break;
          }
     }
     else if (event->key_symbol == DIKS_META) {
          data->wm_level |= 1;
          data->wm_cycle  = 0;
     }

     window = get_keyboard_window( stack, data, event );
     if (window)
          send_key_event( window, data, event );

     return DFB_OK;
}

static DFBResult
handle_key_release( CoreWindowStack     *stack,
                    StackData           *data,
                    const DFBInputEvent *event )
{
     CoreWindow *window;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_KEYRELEASE );

     if (data->wm_level) {
          switch (event->key_symbol) {
               case DIKS_META:
                    data->wm_level &= ~1;
                    break;

               case DIKS_CONTROL:
                    data->wm_level &= ~2;
                    break;

               case DIKS_ALT:
                    data->wm_level &= ~4;
                    break;

               default:
                    if (is_wm_key( event->key_symbol ))
                         return DFB_OK;

                    break;
          }
     }

     window = get_keyboard_window( stack, data, event );
     if (window)
          send_key_event( window, data, event );

     return DFB_OK;
}

/**************************************************************************************************/

static DFBResult
handle_button_press( CoreWindowStack     *stack,
                     StackData           *data,
                     const DFBInputEvent *event )
{
     CoreWindow *window;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_BUTTONPRESS );

     if (!stack->cursor.enabled)
          return DFB_OK;

     switch (data->wm_level) {
          case 1:
               window = data->entered_window;
               if (window && !(window->config.options & DWOP_KEEP_STACKING))
                    dfb_window_raisetotop( data->entered_window );

               break;

          default:
               window = data->pointer_window ? data->pointer_window : data->entered_window;
               if (window)
                    send_button_event( window, data, event );

               break;
     }

     return DFB_OK;
}

static DFBResult
handle_button_release( CoreWindowStack     *stack,
                       StackData           *data,
                       const DFBInputEvent *event )
{
     CoreWindow *window;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_BUTTONRELEASE );

     if (!stack->cursor.enabled)
          return DFB_OK;

     switch (data->wm_level) {
          case 1:
               break;

          default:
               window = data->pointer_window ? data->pointer_window : data->entered_window;
               if (window)
                    send_button_event( window, data, event );

               break;
     }

     return DFB_OK;
}

/**************************************************************************************************/

static void
perform_motion( CoreWindowStack *stack,
                StackData       *data,
                WMData          *wmdata,
                int              dx,
                int              dy )
{
     int               old_cx, old_cy;
     DFBWindowEvent    we;
     CoreWindow       *entered;
     CoreWindowConfig *config = NULL;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );

     // fixed mantis 0892187: [OpenBrowser] Abnormal Mouse Cursor Behaviour when using a physical mouse for some websites.
     if(true == dfb_config->enable_cursor_mouse_optimize)
         if (!stack->cursor.enabled)
            return;

     old_cx = stack->cursor.x;
     old_cy = stack->cursor.y;

     dfb_windowstack_cursor_warp( stack, old_cx + dx, old_cy + dy );

     dx = stack->cursor.x - old_cx;
     dy = stack->cursor.y - old_cy;

     if (!dx && !dy)
          return;


     entered = data->entered_window;
     if (entered)
          config = &entered->config;

     switch (data->wm_level) {
          case 7:
          case 6:
          case 5:
          case 4:
               if (entered) {
                    int opacity = config->opacity + dx;

                    if (opacity < 8)
                         opacity = 8;
                    else if (opacity > 255)
                         opacity = 255;

                    dfb_window_set_opacity( entered, opacity );
               }

               break;

          case 3:
          case 2:
               if (entered && !(config->options & DWOP_KEEP_SIZE)) {
                    int width  = config->bounds.w + dx;
                    int height = config->bounds.h + dy;

                    if (width  <   48) width  = 48;
                    if (height <   48) height = 48;
                    if (width  > 2048) width  = 2048;
                    if (height > 2048) height = 2048;

                    dfb_window_resize( entered, width, height );
               }

               break;

          case 1:
               if (entered && !(config->options & DWOP_KEEP_POSITION))
                    dfb_window_move( entered, dx, dy, true );

               break;

          case 0:
               if (data->pointer_window) {
                    CoreWindow *window = data->pointer_window;

                    we.type = DWET_MOTION;
                    we.x    = stack->cursor.x - window->config.bounds.x;
                    we.y    = stack->cursor.y - window->config.bounds.y;

                    transform_point_in_window( window, &we.x, &we.y );

                    post_event( window, data, &we );
               }
               else if (!update_focus( stack, data, wmdata ) && data->entered_window) {
                    CoreWindow *window = data->entered_window;

                    we.type = DWET_MOTION;
                    we.x    = stack->cursor.x - window->config.bounds.x;
                    we.y    = stack->cursor.y - window->config.bounds.y;

                    transform_point_in_window( window, &we.x, &we.y );

                    post_event( window, data, &we );
               }

               break;

          default:
               ;
     }
}

static void
flush_motion( CoreWindowStack *stack,
              StackData       *data,
              WMData          *wmdata )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( wmdata != NULL );

     if (data->cursor_dx || data->cursor_dy) {
          perform_motion( stack, data, wmdata, data->cursor_dx, data->cursor_dy );

          data->cursor_dx = 0;
          data->cursor_dy = 0;
     }
}

static void
handle_wheel( CoreWindowStack *stack,
              StackData       *data,
              int              dz )
{
     DFBWindowEvent we;
     CoreWindow *window = NULL;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );

     if (!stack->cursor.enabled)
          return;

     window = data->pointer_window ? data->pointer_window : data->entered_window;

     if (window) {
          if (data->wm_level) {
               int opacity = window->config.opacity + dz*7;

               if (opacity < 0x01)
                    opacity = 1;
               if (opacity > 0xFF)
                    opacity = 0xFF;

               dfb_window_set_opacity( window, opacity );
          }
          else {
               we.type = DWET_WHEEL;
               we.x    = stack->cursor.x - window->config.bounds.x;
               we.y    = stack->cursor.y - window->config.bounds.y;
               we.step = dz;

               transform_point_in_window( window, &we.x, &we.y );

               post_event( window, data, &we );
          }
     }
}

static DFBResult
handle_axis_motion( CoreWindowStack     *stack,
                    StackData           *data,
                    WMData              *wmdata,
                    const DFBInputEvent *event )
{
     CoreLayerContext *context;
     DFBInputEvent     rotated_event;

     D_ASSERT( stack != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( event != NULL );
     D_ASSERT( event->type == DIET_AXISMOTION );

     context = stack->context;
     D_MAGIC_ASSERT( context, CoreLayerContext );

     if (event->flags & DIEF_AXISREL) {
          rotated_event = *event;
          event = &rotated_event;

          if (event->axis == DIAI_X) {
               if (context->rotation == 90) {
                    rotated_event.axis = DIAI_Y;
               }
               else if (context->rotation == 180) {
                    rotated_event.axisrel = -rotated_event.axisrel;
               }
               else if (context->rotation == 270) {
                    rotated_event.axis = DIAI_Y;
                    rotated_event.axisrel = -rotated_event.axisrel;
               }
          }
          else if (event->axis == DIAI_Y) {
               if (context->rotation == 90) {
                    rotated_event.axis = DIAI_X;
                    rotated_event.axisrel = -rotated_event.axisrel;
               }
               else if (context->rotation == 180) {
                    rotated_event.axisrel = -rotated_event.axisrel;
               }
               else if (context->rotation == 270) {
                    rotated_event.axis = DIAI_X;
               }
          }
     }
     else if (event->flags & DIEF_AXISABS) {
          rotated_event = *event;
          event = &rotated_event;

          if (event->axis == DIAI_X) {
               if (context->rotation == 90) {
                    rotated_event.axis = DIAI_Y;
               }
               else if (context->rotation == 180) {
                    rotated_event.axisabs = stack->rotated_width - rotated_event.axisabs;
               }
               else if (context->rotation == 270) {
                    rotated_event.axis = DIAI_Y;
                    rotated_event.axisabs = stack->rotated_width - rotated_event.axisabs;
               }
          }
          else if (event->axis == DIAI_Y) {
               if (context->rotation == 90) {
                    rotated_event.axis = DIAI_X;
                    rotated_event.axisabs = stack->rotated_height - rotated_event.axisabs;
               }
               else if (context->rotation == 180) {
                    rotated_event.axisabs = stack->rotated_height - rotated_event.axisabs;
               }
               else if (context->rotation == 270) {
                    rotated_event.axis = DIAI_X;
               }
          }
     }

     if (event->flags & DIEF_AXISREL) {
          int rel = event->axisrel;

          /* handle cursor acceleration */
          if (rel > stack->cursor.threshold)
               rel += (rel - stack->cursor.threshold)
                      * stack->cursor.numerator
                      / stack->cursor.denominator;
          else if (rel < -stack->cursor.threshold)
               rel += (rel + stack->cursor.threshold)
                      * stack->cursor.numerator
                      / stack->cursor.denominator;

          switch (event->axis) {
               case DIAI_X:
                    data->cursor_dx += rel;
                    break;

               case DIAI_Y:
                    data->cursor_dy += rel;
                    break;

               case DIAI_Z:
                    flush_motion( stack, data, wmdata );

                    handle_wheel( stack, data, - event->axisrel );
                    break;

               default:
                    ;
          }
     }
     else if (event->flags & DIEF_AXISABS) {
          int axismin = 0;
          int axisabs = event->axisabs;

          if (event->flags & DIEF_MIN) {
               axismin = event->min;

               axisabs -= axismin;
          }

          switch (event->axis) {
               case DIAI_X:
                    if (event->flags & DIEF_MAX)
                         axisabs = axisabs * stack->width / (event->max - axismin + 1);

                    data->cursor_dx = axisabs - stack->cursor.x;
                    break;

               case DIAI_Y:
                    if (event->flags & DIEF_MAX)
                         axisabs = axisabs * stack->height / (event->max - axismin + 1);

                    data->cursor_dy = axisabs - stack->cursor.y;
                    break;

               default:
                    ;
          }
     }

     return DFB_OK;
}

/**************************************************************************************************/
/**************************************************************************************************/

static void
wm_get_info( CoreWMInfo *info )
{
     info->version.major  = 0;
     info->version.minor  = 3;
     info->version.binary = 2;

     snprintf( info->name, DFB_CORE_WM_INFO_NAME_LENGTH, "Default" );
     snprintf( info->vendor, DFB_CORE_WM_INFO_VENDOR_LENGTH, "directfb.org" );

     info->wm_data_size     = sizeof(WMData);
     info->stack_data_size  = sizeof(StackData);
     info->window_data_size = sizeof(WindowData);
     info->wm_shared_size = sizeof(WindowSharedData);
}

#if (USE_MTK_STI==0)
static FusionCallHandlerResult
hw_cursor_reference_watcher( int caller, int call_arg, void *call_ptr, void *ctx, unsigned int serial, int *ret_val )
{
    WindowSharedData *shared = ctx;
    DFBResult ret = DFB_OK;

#if FUSION_BUILD_KERNEL
    if (caller) {
        DBG_CURSOR_MSG( "Call not from Fusion/Kernel (caller %d)", caller );
        return FCHR_RETURN;
    }
#endif

    /* Lock the pool. */
    if (fusion_skirmish_prevail( &shared->cursor_lock ))
        return FCHR_RETURN;

    DBG_CURSOR_MSG("[DFB]%s,%d, pid=%d,id=%d\n",__FUNCTION__,__LINE__, getpid(), shared->cusor_gop_id);

    /* Call the destructor. */
    ret = dfb_wm_destroy_hwcursor(shared->cusor_gop_id);
    if(ret){
        DBG_CURSOR_MSG("[DFB] %s %d destroy MI handle fail, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
    }


    /* Unlock the pool. */
    ret = fusion_skirmish_dismiss( &shared->cursor_lock );
    if(ret){
        DBG_CURSOR_MSG("[DFB] %s %d fail, ret=%s\n",__FUNCTION__,__LINE__,DirectResultString( ret ));
    }

    return FCHR_RETURN;
}

static DFBResult
wm_init_hw_cursor( CoreDFB *core, void *wm_data, void *shared_data )
{
     WindowSharedData *shared = shared_data;
     if (!shared)
     {
        DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d shared data is NULL!!!! \33[0m \n",__FUNCTION__,__LINE__);
        return DFB_INVARG;
     }
     DFBResult ret = DFB_OK;
     bool enable_hwcursor = false;
    /*
     three steps to check if using hw cursor or not
     1. DFB_SUPPORT_AN is false which means current platform is Linux.
        DFB hw curso doesn't need support the AN platform, it's support by HWC
     2. parsing /config/OSD/OsdDefine.ini, if it has defined hw cursor or not?
     3. parsing /config/OSD/OsdDefine.ini, check if the gop index for cursor using
        is overlap to FBDEV or not?
    */
    if (DFB_SUPPORT_AN){
        enable_hwcursor = false;
    }
    else
    {
        if(dfb_config->mst_hw_cursor < 0)
            enable_hwcursor = false;
        else
            enable_hwcursor = true;
    }


    /*
    If enable_hwcursor is true, init corresponding fusion control, such as skirmish and call.
    */
    if ( enable_hwcursor)
    {
        ret =fusion_skirmish_init( &shared->cursor_lock, "HW CURSOR", dfb_core_world(core) );
        if (ret){
            DBG_CURSOR_MSG("[DFB] %s, %d fail\n",__FUNCTION__,__LINE__);
            return ret;
        }


        /* Destruction call from Fusion. */
        ret = fusion_call_init( &shared->call, hw_cursor_reference_watcher, shared,  dfb_core_world(core) );
        if (ret){
            DBG_CURSOR_MSG("[DFB] %s, %d fail\n",__FUNCTION__,__LINE__);
            return ret;
        }

        shared->cusor_gop_id = dfb_config->mst_hw_cursor;
        shared->bhwcursor = true;
        shared->hw_cursor_shape[0] =NULL;
        shared->hw_cursor_shape[1] =NULL;
    }
    else
        shared->bhwcursor = false;

    DBG_CURSOR_MSG("\33[0;33;44m[DFB] %s, %d , bhwcursor=%d\33[0m ,gop id=%d, is AN platform=%d, pid=%d\n",
        __FUNCTION__,__LINE__, shared->bhwcursor, shared->cusor_gop_id, DFB_SUPPORT_AN, getpid());


    return DFB_OK;

}

static DFBResult
wm_deinit_hw_cursor(void *shared_data )
{

    WindowSharedData *shared = shared_data;

     if (shared->bhwcursor){
         DBG_CURSOR_MSG("[DFB] %s, %d,start to destroy cursor lock, refs and call, pid=%d  \n",__FUNCTION__,__LINE__, getpid());
         fusion_call_destroy( &shared->call );
         fusion_skirmish_destroy (&shared->cursor_lock);
         DBG_CURSOR_MSG("[DFB] %s, %d, destroy cursor lock, refs and call end!!!, pid=%d  \n",__FUNCTION__,__LINE__, getpid());
     }

    return DFB_OK;
}
#endif

static DFBResult
wm_initialize( CoreDFB *core, void *wm_data, void *shared_data )
{
    WMData *data = wm_data;

    data->core = core;
#if (USE_MTK_STI==0)
    wm_init_hw_cursor( core, wm_data, shared_data );
#endif

    return DFB_OK;
}

static DFBResult
wm_join( CoreDFB *core, void *wm_data, void *shared_data )
{
     WMData *data = wm_data;


     data->core = core;
     WindowSharedData *shared = shared_data;
     DBG_CURSOR_MSG("[DFB] %s, %d , bhwcursor=%d, gop id=%d, pid=%d \n",__FUNCTION__,__LINE__, shared->bhwcursor,shared->cusor_gop_id, getpid());

     return DFB_OK;
}

static DFBResult
wm_shutdown( bool emergency, void *wm_data, void *shared_data )
{
#if (USE_MTK_STI==0)
     wm_deinit_hw_cursor(shared_data );
#endif
     return DFB_OK;
}

static DFBResult
wm_leave( bool emergency, void *wm_data, void *shared_data )
{
     return DFB_OK;
}

static DFBResult
wm_suspend( void *wm_data, void *shared_data )
{
     return DFB_OK;
}

static DFBResult
wm_resume( void *wm_data, void *shared_data )
{
     return DFB_OK;
}

static DFBResult
wm_post_init( void *wm_data, void *shared_data )
{
     return DFB_OK;
}

/**************************************************************************************************/

static DFBResult
wm_init_stack( CoreWindowStack *stack,
               void            *wm_data,
               void            *stack_data )
{
     int        i;
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     data->stack = stack;

     /* Initialize update manager. */
     dfb_updates_init( &data->updates, data->update_regions, MAX_UPDATE_REGIONS );

     fusion_vector_init( &data->windows, 64, stack->shmpool );

     for (i=0; i<MAX_KEYS; i++)
          data->keys[i].code = -1;

     D_MAGIC_SET( data, StackData );

     return DFB_OK;
}

static DFBResult
wm_close_stack( CoreWindowStack *stack,
                void            *wm_data,
                void            *stack_data )
{
     DirectLink *l, *next;
     StackData  *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     D_MAGIC_ASSERT( data, StackData );
     D_MAGIC_CLEAR( data );

     D_ASSUME( fusion_vector_is_empty( &data->windows ) );

     if (fusion_vector_has_elements( &data->windows )) {
          int         i;
          CoreWindow *window;

          fusion_vector_foreach (window, i, data->windows) {
               D_WARN( "setting window->stack = NULL" );
               window->stack = NULL;
          }
     }

     fusion_vector_destroy( &data->windows );

     /* Destroy backing store of software cursor. */
     if (data->cursor_bs)
          dfb_surface_unlink( &data->cursor_bs );

     /* Free grabbed keys. */
     direct_list_foreach_safe (l, next, data->grabbed_keys)
          SHFREE( stack->shmpool, l );

     return DFB_OK;
}

static DFBResult
wm_set_active( CoreWindowStack *stack,
               void            *wm_data,
               void            *stack_data,
               bool             active )
{
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     D_MAGIC_ASSERT( data, StackData );

     D_ASSUME( data->active != active );

     if (data->active == active)
          return DFB_OK;

     data->active = active;

     if (active)
          return dfb_windowstack_repaint_all( stack );

     /* Force release of all pressed keys. */
     return wm_flush_keys( stack, wm_data, stack_data );
}

static DFBResult
wm_resize_stack( CoreWindowStack *stack,
                 void            *wm_data,
                 void            *stack_data,
                 int              width,
                 int              height )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     return DFB_OK;
}

static DFBResult
wm_process_input( CoreWindowStack     *stack,
                  void                *wm_data,
                  void                *stack_data,
                  const DFBInputEvent *event )
{
     DFBResult  ret;
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( event != NULL );

     D_MAGIC_ASSERT( data, StackData );

     D_DEBUG_AT( WM_Default, "Processing input event (device %d, type 0x%08x, flags 0x%08x)...\n",
                 event->device_id, event->type, event->flags );

     /* FIXME: handle multiple devices */
     if (event->flags & DIEF_BUTTONS)
          data->buttons = event->buttons;

     if (event->flags & DIEF_MODIFIERS)
          data->modifiers = event->modifiers;

     if (event->flags & DIEF_LOCKS)
          data->locks = event->locks;

     if (event->type != DIET_AXISMOTION)
          flush_motion( stack, data, wm_data );

     switch (event->type) {
          case DIET_KEYPRESS:
               ret = handle_key_press( stack, data, wm_data, event );
               break;

          case DIET_KEYRELEASE:
               ret = handle_key_release( stack, data, event );
               break;

          case DIET_BUTTONPRESS:
               ret = handle_button_press( stack, data, event );
               break;

          case DIET_BUTTONRELEASE:
               ret = handle_button_release( stack, data, event );
               break;

          case DIET_AXISMOTION:
               ret = handle_axis_motion( stack, data, wm_data, event );
               break;

          default:
               D_ONCE( "unknown input event type" );
               ret = DFB_UNSUPPORTED;
               break;
     }

     if (!D_FLAGS_IS_SET( event->flags, DIEF_FOLLOW ))
          flush_motion( stack, data, wm_data );

     process_updates( data, wm_data, stack, NULL, DSFLIP_NONE );

     return ret;
}

static DFBResult
wm_flush_keys( CoreWindowStack *stack,
               void            *wm_data,
               void            *stack_data )
{
     int        i;
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     D_MAGIC_ASSERT( data, StackData );

     for (i=0; i<MAX_KEYS; i++) {
          if (data->keys[i].code != -1) {
               DFBWindowEvent we;

               we.type       = DWET_KEYUP;
               we.key_code   = data->keys[i].code;
               we.key_id     = data->keys[i].id;
               we.key_symbol = data->keys[i].symbol;

               post_event( data->keys[i].owner, data, &we );

               data->keys[i].code = -1;
          }
     }

     return DFB_OK;
}

static DFBResult
wm_window_at( CoreWindowStack  *stack,
              void             *wm_data,
              void             *stack_data,
              int               x,
              int               y,
              CoreWindow      **ret_window )
{
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( ret_window != NULL );

     *ret_window = window_at_pointer( stack, data, wm_data, x, y );

     return DFB_OK;
}

static DFBResult
wm_window_lookup( CoreWindowStack  *stack,
                  void             *wm_data,
                  void             *stack_data,
                  DFBWindowID       window_id,
                  CoreWindow      **ret_window )
{
     int         i;
     CoreWindow *window = NULL;
     StackData  *data   = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( ret_window != NULL );

     D_MAGIC_ASSERT( data, StackData );

     fusion_vector_foreach_reverse (window, i, data->windows) {
          if (window->id == window_id) {
               break;
          }
     }

     *ret_window = window;
     return DFB_OK;
}

static DFBResult
wm_enum_windows( CoreWindowStack      *stack,
                 void                 *wm_data,
                 void                 *stack_data,
                 CoreWMWindowCallback  callback,
                 void                 *callback_ctx )
{
     int         i;
     CoreWindow *window = NULL;
     StackData  *data   = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( callback != NULL );

     D_MAGIC_ASSERT( data, StackData );

     fusion_vector_foreach_reverse (window, i, data->windows) {
          if (callback( window, callback_ctx ) != DFENUM_OK)
               break;
     }

     return DFB_OK;
}

/**************************************************************************************************/

static DFBResult
wm_get_insets( CoreWindowStack *stack,
               CoreWindow      *window,
               DFBInsets       *insets )
{
     insets->l = 0;
     insets->t = 0;
     insets->r = 0;
     insets->b = 0;

     return DFB_OK;
}


static DFBResult
wm_preconfigure_window( CoreWindowStack *stack,
               void            *wm_data,
               void            *stack_data,
               CoreWindow      *window,
               void            *window_data )
{
     if (window->config.association)
          return DFB_UNIMPLEMENTED;

     return DFB_OK;
}

static DFBResult
wm_set_window_property( CoreWindowStack  *stack,
                        void             *wm_data,
                        void             *stack_data,
                        CoreWindow       *window,
                        void             *window_data,
                        const char       *key,
                        void             *value,
                        void            **ret_old_value )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( key != NULL );

     fusion_object_set_property((FusionObject*)window,
                     key,value,ret_old_value);
     return DFB_OK;
}

static DFBResult
wm_get_window_property( CoreWindowStack  *stack,
                        void             *wm_data,
                        void             *stack_data,
                        CoreWindow       *window,
                        void             *window_data,
                        const char       *key,
                        void            **ret_value )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( key != NULL );
     D_ASSERT( ret_value != NULL );

     *ret_value = fusion_object_get_property((FusionObject*)window,key);
     return DFB_OK;
}


static DFBResult
wm_remove_window_property( CoreWindowStack  *stack,
                           void             *wm_data,
                           void             *stack_data,
                           CoreWindow       *window,
                           void             *window_data,
                           const char       *key,
                           void            **ret_value )
{
     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( key != NULL );

     fusion_object_remove_property((FusionObject*)window,key,ret_value);
     return DFB_OK;
}

static DFBResult
wm_add_window( CoreWindowStack *stack,
               void            *wm_data,
               void            *stack_data,
               CoreWindow      *window,
               void            *window_data )
{
     WindowData *data  = window_data;
     StackData  *sdata = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );

     D_MAGIC_ASSERT( sdata, StackData );

     /* Initialize window data. */
     data->window     = window;
     data->stack_data = stack_data;
     data->priority   = get_priority( window );

     if (window->region)
          dfb_layer_region_get_configuration( window->region, &data->config );

     D_MAGIC_SET( data, WindowData );

     /* Actually add the window to the stack. */
     insert_window( stack, sdata, window, data );

     /* Possibly switch focus to the new window. */
     update_focus( stack, sdata, wm_data );

     process_updates( sdata, wm_data, stack, window->primary_region, DSFLIP_NONE );

     return DFB_OK;
}

static DFBResult
wm_remove_window( CoreWindowStack *stack,
                  void            *wm_data,
                  void            *stack_data,
                  CoreWindow      *window,
                  void            *window_data )
{
     WindowData *data  = window_data;
     StackData  *sdata = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( window != NULL );
     D_ASSERT( window_data != NULL );

     D_MAGIC_ASSERT( data, WindowData );
     D_MAGIC_ASSERT( sdata, StackData );

     remove_window( stack, sdata, window, data );

     /* Free key list. */
     if (window->config.keys) {
          SHFREE( stack->shmpool, window->config.keys );

          window->config.keys     = NULL;
          window->config.num_keys = 0;
     }

     D_MAGIC_CLEAR( data );

     return DFB_OK;
}

static DFBResult
wm_set_window_config( CoreWindow             *window,
                      void                   *wm_data,
                      void                   *window_data,
                      const CoreWindowConfig *config,
                      CoreWindowConfigFlags   flags )
{
     DFBResult        ret;
     CoreWindowStack *stack;

     D_ASSERT( window != NULL );
     D_ASSERT( window->stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( config != NULL );

     stack = window->stack;
     D_ASSERT( stack != NULL );

     if (flags & CWCF_OPTIONS) {
          if ((window->config.options & DWOP_SCALE) && !(config->options & DWOP_SCALE) && window->surface) {
               if (window->config.bounds.w != window->surface->config.size.w ||
                   window->config.bounds.h != window->surface->config.size.h)
               {
                    ret = dfb_surface_reformat( window->surface,
                                                window->config.bounds.w,
                                                window->config.bounds.h,
                                                window->surface->config.format );
                    if (ret) {
                         D_DERROR( ret, "WM/Default: Could not resize surface "
                                        "(%dx%d -> %dx%d) to remove DWOP_SCALE!\n",
                                   window->surface->config.size.w,
                                   window->surface->config.size.h,
                                   window->config.bounds.w,
                                   window->config.bounds.h );
                         return ret;
                    }
               }
          }

          window->config.options = config->options;
     }

     if (flags & CWCF_EVENTS)
          window->config.events = config->events;

     if (flags & CWCF_COLOR)
          return DFB_UNSUPPORTED;

     if (flags & CWCF_COLOR_KEY)
          window->config.color_key = config->color_key;

     if (flags & CWCF_OPAQUE)
          window->config.opaque = config->opaque;

     if (flags & CWCF_OPACITY && !config->opacity)
          set_opacity( window, window_data, wm_data, config->opacity );

     if (flags == (CWCF_POSITION | CWCF_SIZE)) {
          ret = set_window_bounds (window, wm_data, window_data,
                                   config->bounds.x, config->bounds.y,
                                   config->bounds.w, config->bounds.h);
          if (ret)
              return ret;
     }
     else {
          if (flags & CWCF_POSITION) {
               ret = move_window( window, window_data,
                                  config->bounds.x - window->config.bounds.x,
                                  config->bounds.y - window->config.bounds.y );
               if (ret)
                    return ret;

               if(window->config.options & DWOP_MULTI_VIEW) {
                    return DFB_OK;
               }
          }

          if (flags & CWCF_SIZE) {
               ret = resize_window( window, wm_data, window_data, config->bounds.w, config->bounds.h );
               if (ret)
                    return ret;

               if(window->config.options & DWOP_MULTI_VIEW) {
                    return DFB_OK;
               }
          }
     }

     if (flags & CWCF_ROTATION) {
          update_window( window, window_data, NULL, DSFLIP_NONE, false, false, false );

          window->config.rotation = config->rotation;

          update_window( window, window_data, NULL, DSFLIP_NONE, false, false, false );
     }

     if (flags & CWCF_STACKING)
          restack_window( window, window_data, window, window_data, 0, config->stacking );

     if (flags & CWCF_OPACITY && config->opacity)
          set_opacity( window, window_data, wm_data, config->opacity );

     if (flags & CWCF_KEY_SELECTION) {
          if (config->key_selection == DWKS_LIST) {
               unsigned int             bytes = sizeof(DFBInputDeviceKeySymbol) * config->num_keys;
               DFBInputDeviceKeySymbol *keys;

               D_ASSERT( config->keys != NULL );
               D_ASSERT( config->num_keys > 0 );

               keys = SHMALLOC( window->stack->shmpool, bytes );
               if (!keys) {
                    D_ERROR( "WM/Default: Could not allocate %d bytes for list "
                             "of selected keys (%d)!\n", bytes, config->num_keys );
                    return D_OOSHM();
               }

               direct_memcpy( keys, config->keys, bytes );

               qsort( keys, config->num_keys, sizeof(DFBInputDeviceKeySymbol), keys_compare );

               if (window->config.keys)
                    SHFREE( window->stack->shmpool, window->config.keys );

               window->config.keys     = keys;
               window->config.num_keys = config->num_keys;
          }
          else if (window->config.keys) {
               SHFREE( window->stack->shmpool, window->config.keys );

               window->config.keys     = NULL;
               window->config.num_keys = 0;
          }

          window->config.key_selection = config->key_selection;
     }

     process_updates( stack->stack_data, wm_data, stack, window->primary_region, DSFLIP_NONE );

     return DFB_OK;
}

static DFBResult
wm_restack_window( CoreWindow             *window,
                   void                   *wm_data,
                   void                   *window_data,
                   CoreWindow             *relative,
                   void                   *relative_data,
                   int                     relation )
{
     DFBResult   ret;
     WindowData *data = window_data;
     StackData  *sdata;

     D_ASSERT( window != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );

     D_MAGIC_ASSERT( data, WindowData );

     D_ASSERT( relative == NULL || relative_data != NULL );

     D_ASSERT( relative == NULL || relative == window || relation != 0);

     sdata = data->stack_data;
     D_ASSERT( sdata != NULL );
     D_ASSERT( sdata->stack != NULL );

     ret = restack_window( window, window_data, relative,
                           relative_data, relation, window->config.stacking );
     if (ret)
          return ret;

     /* Possibly switch focus to window now under the cursor */
     update_focus( sdata->stack, sdata, wm_data );

     process_updates( sdata, wm_data, window->stack, window->primary_region, DSFLIP_NONE );

     return DFB_OK;
}

static DFBResult
wm_grab( CoreWindow *window,
         void       *wm_data,
         void       *window_data,
         CoreWMGrab *grab )
{
     StackData  *sdata;
     WindowData *wdata = window_data;

     D_ASSERT( window != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( grab != NULL );
     D_ASSERT( wdata->stack_data != NULL );

     sdata = wdata->stack_data;

     switch (grab->target) {
          case CWMGT_KEYBOARD:
               return grab_keyboard( window, window_data );

          case CWMGT_POINTER:
               return grab_pointer( window, window_data );

          case CWMGT_KEY:
               return grab_key( window, window_data, grab->symbol, grab->modifiers );

          case CWMGT_UNSELECTED_KEYS:
               if (sdata->unselkeys_window)
                    return DFB_LOCKED;

               sdata->unselkeys_window = window;
               return DFB_OK;

          default:
               D_BUG( "unknown grab target" );
               break;
     }

     return DFB_BUG;
}

static DFBResult
wm_ungrab( CoreWindow *window,
           void       *wm_data,
           void       *window_data,
           CoreWMGrab *grab )
{
     StackData  *sdata;
     WindowData *wdata = window_data;

     D_ASSERT( window != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );
     D_ASSERT( grab != NULL );
     D_ASSERT( wdata->stack_data != NULL );

     sdata = wdata->stack_data;

     switch (grab->target) {
          case CWMGT_KEYBOARD:
               return ungrab_keyboard( window, window_data );

          case CWMGT_POINTER:
               return ungrab_pointer( window, window_data, wm_data );

          case CWMGT_KEY:
               return ungrab_key( window, window_data, grab->symbol, grab->modifiers );

          case CWMGT_UNSELECTED_KEYS:
               if (sdata->unselkeys_window == window)
                    sdata->unselkeys_window = NULL;

               return DFB_OK;

          default:
               D_BUG( "unknown grab target" );
               break;
     }

     return DFB_BUG;
}

static DFBResult
wm_request_focus( CoreWindow *window,
                  void       *wm_data,
                  void       *window_data )
{
     D_ASSERT( window != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );

     return request_focus( window, window_data );
}

/**************************************************************************************************/

static DFBResult
wm_update_stack( CoreWindowStack     *stack,
                 void                *wm_data,
                 void                *stack_data,
                 const DFBRegion     *region,     /* stack coordinates */
                 DFBSurfaceFlipFlags  flags )
{
     StackData *data = stack_data;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );
     D_ASSERT( region != NULL );

     D_MAGIC_ASSERT( data, StackData );

     dfb_updates_add( &data->updates, region );

     process_updates( data, wm_data, stack, NULL, flags );

     return DFB_OK;
}

static DFBResult
wm_update_window( CoreWindow          *window,
                  void                *wm_data,
                  void                *window_data,
                  const DFBRegion     *region,    /* surface coordinates! */
                  const DFBRegion     *right_region,   /* surface coordinates! */
                  DFBSurfaceFlipFlags  flags )
{
     CoreWindowStack *stack;

     D_ASSERT( window != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( window_data != NULL );

     DFB_REGION_ASSERT_IF( region );

     stack = window->stack;
     D_ASSERT( stack != NULL );

     update_window( window, window_data, region, flags, false, false, true );

     process_updates( stack->stack_data, wm_data, stack, window->primary_region, flags );

     return DFB_OK;
}

#if USE_CURSOR
static DFBResult
wm_update_cursor( CoreWindowStack       *stack,
                  void                  *wm_data,
                  void                  *stack_data,
                  CoreCursorUpdateFlags  flags )
{


     DFBResult         ret;
     DFBRegion         old_dest;
     DFBRegion         old_region;
     WMData           *wmdata   = wm_data;
     StackData        *data     = stack_data;
     bool              restored = false;
     CoreLayerContext *context;
     CoreLayerRegion  *primary;
     CoreSurface      *surface;

     D_ASSERT( stack != NULL );
     D_ASSERT( wm_data != NULL );
     D_ASSERT( stack_data != NULL );

     D_MAGIC_ASSERT( data, StackData );

     old_region = data->cursor_region;

     transform_stack_to_dest( stack, &old_region, &old_dest );

     if (flags & (CCUF_ENABLE | CCUF_POSITION | CCUF_SIZE)) {
          data->cursor_bs_valid  = false;

          data->cursor_region.x1 = stack->cursor.x - stack->cursor.hot.x;
          data->cursor_region.y1 = stack->cursor.y - stack->cursor.hot.y;
          data->cursor_region.x2 = data->cursor_region.x1 + stack->cursor.size.w - 1;
          data->cursor_region.y2 = data->cursor_region.y1 + stack->cursor.size.h - 1;

          if (!dfb_region_intersect( &data->cursor_region, 0, 0, stack->width - 1, stack->height - 1 )) {
               D_BUG( "invalid cursor region" );
               return DFB_BUG;
          }
     }

     /* Optimize case of invisible cursor moving. */
     if (!(flags & ~(CCUF_POSITION | CCUF_SHAPE)) && (!stack->cursor.opacity || !stack->cursor.enabled))
          return DFB_OK;
#if (USE_MTK_STI==0)
     WindowSharedData* shared_data =stack->shared_data;

     /*
       how to enable hw cursor: just insert the following text to /config/OSD/OsdDefine.ini
       --------------------------
       [HWCURSOR]
       HWCURSOR_GOPINDEX=X // X is the GOP id cursor can use

       1. The setting as mentioned above will be effective when you restart the system and can't be modify dynamically.
       2. If the user decides to use hwcusor and check eligibility by dfb_wm_probe_cursor funciotn. DFB will not go to sw flow even though it call dfb_wm_update_hwcursor failed.

     */
     if (shared_data && shared_data->bhwcursor)
     {
         ret = dfb_wm_update_hwcursor(  wmdata->core, stack, flags, &data->cursor_region, shared_data);

         if (stack->cursor.opacity)
             data->cursor_drawn = true;
         else
             data->cursor_drawn = false;

         if (ret)
            DBG_CURSOR_MSG("\33[0;33;44m[DFB] HW cursor has something wrong!!!!\33[0m  ( %s, %d) ret:%s\n", __FUNCTION__, __LINE__, DirectResultString( ret ));

         return DFB_OK;

     }
#endif
     //SW cursor flow

     context = stack->context;
     D_ASSERT( context != NULL );

     if (!data->cursor_bs) {
          CoreSurface            *cursor_bs;
          DFBSurfaceCapabilities  caps = DSCAPS_NONE;
          DFBDimension            size = stack->cursor.size;

          D_ASSUME( flags & CCUF_ENABLE );

          dfb_surface_caps_apply_policy( stack->cursor.policy, &caps );

          if (stack->rotation == 90 || stack->rotation == 270)
               D_UTIL_SWAP( size.w, size.h );

          /* Create the cursor backing store surface. */
          ret = dfb_surface_create_simple( wmdata->core, size.w, size.h,
                                           context->config.pixelformat, context->config.colorspace,
                                           caps, CSTF_SHARED | CSTF_CURSOR,
                                           0, /* FIXME: no shared cursor objects, no cursor id */
                                           NULL, &cursor_bs );
          if (ret) {
               D_ERROR( "WM/Default: Failed creating backing store for cursor!\n" );
               return ret;
          }

          ret = dfb_surface_globalize( cursor_bs );
          D_ASSERT( ret == DFB_OK );

          data->cursor_bs = cursor_bs;
     }

     D_ASSERT( data->cursor_bs != NULL );

     /* Get the primary region. */
     ret = dfb_layer_context_get_primary_region( context, false, &primary );
     if (ret)
          return ret;

     surface = primary->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );






     if ((flags & CCUF_ENABLE)&&(stack->display_mode == DLDM_NORMAL ||
                                  ((stack->display_mode != DLDM_NORMAL)&&stack->cursor.opacity)))
        {
         /* Ensure valid back buffer. From now on swapping is prevented until cursor is disabled.
          * FIXME: Keep a flag to know when back/front have been swapped and need a sync.
          */
         switch (context->config.buffermode) {
              case DLBM_BACKVIDEO:
              case DLBM_TRIPLE:
                   dfb_gfx_copy( surface, surface, NULL );
                   break;

              default:
                   break;
         }
     }


     //sync the cursor related region from front buffer to back buffer
    if(stack->bforce_fullupdate)
    {
        DFBRegion  dest;
        DFBRectangle rect;
        transform_stack_to_dest( stack, &data->cursor_region, &dest );
        dfb_region_region_union( &dest, &old_dest );
        rect = DFB_RECTANGLE_INIT_FROM_REGION( &dest );        
        dfb_gfx_copy( surface, surface, &rect );
    }

     /* restore region under cursor */
     if (data->cursor_drawn && dfb_region_intersect( &old_dest, 0, 0,
                                                     surface->config.size.w - 1, surface->config.size.h - 1 ))
     {
          DFBRectangle rect = { 0, 0,
                                old_dest.x2 - old_dest.x1 + 1,
                                old_dest.y2 - old_dest.y1 + 1 };

          D_ASSERT( stack->cursor.opacity || (flags & CCUF_OPACITY) );

          dfb_gfx_copy_to( data->cursor_bs, surface, &rect, old_dest.x1, old_dest.y1, false );

          data->cursor_drawn = false;

          restored = true;
     }

     if (flags & CCUF_SIZE) {
          DFBDimension size = stack->cursor.size;

          if (stack->rotation == 90 || stack->rotation == 270)
               D_UTIL_SWAP( size.w, size.h );

          ret = dfb_surface_reformat( data->cursor_bs, size.w, size.h, data->cursor_bs->config.format );
          if (ret)
               D_DERROR( ret, "WM/Default: Failed resizing backing store for cursor from %dx%d to %dx%d!\n",
                         data->cursor_bs->config.size.w, data->cursor_bs->config.size.h,
                         stack->cursor.size.w, stack->cursor.size.h );
     }

     if (flags & CCUF_DISABLE) {
          dfb_surface_unlink( &data->cursor_bs );
     }
     else if (stack->cursor.opacity) {
          DFBRegion  dest;
          CoreLayer *layer = dfb_layer_at( context->layer_id );
          CardState *state = &layer->state;

          transform_stack_to_dest( stack, &data->cursor_region, &dest );

          if (!dfb_region_intersect( &dest, 0, 0, surface->config.size.w - 1, surface->config.size.h - 1 )) {
               if (restored)
                    dfb_layer_region_flip_update( primary, &old_dest, DSFLIP_BLIT );
               dfb_layer_region_unref( primary );
               return DFB_OK;
          }

          /* backup region under cursor */
          if (!data->cursor_bs_valid) {
               DFBRectangle rect = DFB_RECTANGLE_INIT_FROM_REGION( &dest );

               D_ASSERT( !data->cursor_drawn );

               /* FIXME: this requires using blitted flipping all the time,
                  but fixing it seems impossible, for now DSFLIP_BLIT is forced
                  in repaint_stack() when the cursor is enabled. */
               dfb_gfx_copy_to( surface, data->cursor_bs, &rect, 0, 0, true );

               data->cursor_bs_valid = true;
          }

          /* Set destination. */
          state->destination  = surface;
          state->modified    |= SMF_DESTINATION;

          /* Set clipping region. */
          dfb_state_set_clip( state, &dest );

          /* draw cursor */
          draw_cursor( stack, data, state, &data->cursor_region );

          /* Reset destination. */
          state->destination  = NULL;
          state->modified    |= SMF_DESTINATION;

          data->cursor_drawn = true;
          stack->bforce_fullupdate = (dfb_config->mst_cursor_swap_mode) ? true : false;

          if (restored) {
               if (dfb_region_region_intersects( &old_dest, &dest ))
                    dfb_region_region_union( &old_dest, &dest );
               else
                    dfb_layer_region_flip_update( primary, &dest, DSFLIP_BLIT );

               dfb_layer_region_flip_update( primary, &old_dest, DSFLIP_BLIT );
          }
          else
               dfb_layer_region_flip_update( primary, &dest, DSFLIP_BLIT );

          stack->bforce_fullupdate = dfb_config->mst_layer_bfullupdate[stack->context->layer_id];

          /* Pan to follow the cursor? */
          if (primary->config.source.w < surface->config.size.w || primary->config.source.h < surface->config.size.h) {
               DFBRectangle source = primary->config.source;

               if (stack->rotation)
                    D_UNIMPLEMENTED();

               if (source.x > stack->cursor.x)
                    source.x = stack->cursor.x;
               else if (source.x + source.w - 1 < stack->cursor.x)
                    source.x = stack->cursor.x - source.w + 1;

               if (source.y > stack->cursor.y)
                    source.y = stack->cursor.y;
               else if (source.y + source.h - 1 < stack->cursor.y)
                    source.y = stack->cursor.y - source.h + 1;

               dfb_layer_context_set_sourcerectangle( context, &source );
          }

     }
     else if (restored)     
          dfb_layer_region_flip_update( primary, &old_dest, DSFLIP_BLIT );
       

     /* Unref primary region. */
     dfb_layer_region_unref( primary );


     return DFB_OK;
}
#endif
