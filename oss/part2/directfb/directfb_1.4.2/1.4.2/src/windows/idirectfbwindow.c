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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sched.h>

#include <sys/time.h>
#include <errno.h>

#include <pthread.h>

#include <directfb.h>

#include <idirectfb.h>

#include <core/CoreWindow.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/layers.h>
#include <core/palette.h>
#include <core/state.h>
#include <core/surface.h>
#include <core/windows.h>
#include <core/wm.h>
#include <core/windowstack.h>
#include <core/windows_internal.h> /* FIXME */

#include <display/idirectfbsurface.h>
#include <display/idirectfbsurface_window.h>

#include <input/idirectfbinputbuffer.h>

#include <misc/util.h>
#include <direct/interface.h>
#include <direct/mem.h>

#include <gfx/convert.h>

#include <windows/idirectfbwindow.h>


D_DEBUG_DOMAIN( IDirectFB_Window, "IDirectFBWindow", "DirectFB Window Interface" );


/*
 * adds an window event to the event queue
 */
static ReactionResult IDirectFBWindow_React( const void *msg_data,
                                             void       *ctx );



typedef struct {
     int                ref;
     CoreWindow        *window;
     CoreLayer         *layer;

     IDirectFBSurface  *surface;

     struct {
          IDirectFBSurface  *shape;
          int                hot_x;
          int                hot_y;
     } cursor;

     Reaction           reaction;

     bool               entered;

     bool               detached;
     bool               destroyed;

     CoreDFB           *core;
     IDirectFB         *idirectfb;
} IDirectFBWindow_data;


static void
IDirectFBWindow_Destruct( IDirectFBWindow *thiz )
{
     IDirectFBWindow_data *data = (IDirectFBWindow_data*)thiz->priv;

     D_DEBUG_AT( IDirectFB_Window, "IDirectFBWindow_Destruct()\n" );

     if (!data->detached) {
          D_DEBUG_AT( IDirectFB_Window, "  -> detaching...\n" );

          dfb_window_detach( data->window, &data->reaction );
     }

     /* this will destroy the fusion object and (eventually) the window */
     D_DEBUG_AT( IDirectFB_Window, "  -> unrefing...\n" );

     dfb_window_unref( data->window );

     D_DEBUG_AT( IDirectFB_Window, "  -> releasing surface...\n" );

     if (data->surface)
          data->surface->Release( data->surface );

     D_DEBUG_AT( IDirectFB_Window, "  -> releasing cursor shape...\n" );

     if (data->cursor.shape)
          data->cursor.shape->Release( data->cursor.shape );

     D_DEBUG_AT( IDirectFB_Window, "  -> done.\n" );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DirectResult
IDirectFBWindow_AddRef( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBWindow_Release( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (--data->ref == 0)
          IDirectFBWindow_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_CreateEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer **buffer )
{
     IDirectFBEventBuffer *b;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     DIRECT_ALLOCATE_INTERFACE( b, IDirectFBEventBuffer );

     IDirectFBEventBuffer_Construct( b, NULL, NULL );

     IDirectFBEventBuffer_AttachWindow( b, data->window );

     dfb_window_send_configuration( data->window );

     *buffer = b;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_AttachEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     IDirectFBEventBuffer_AttachWindow( buffer, data->window );

     dfb_window_send_configuration( data->window );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_DetachEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     return IDirectFBEventBuffer_DetachWindow( buffer, data->window );
}

static DFBResult
IDirectFBWindow_EnableEvents( IDirectFBWindow       *thiz,
                              DFBWindowEventType     mask )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (mask & ~DWET_ALL)
          return DFB_INVARG;

     return CoreWindow_ChangeEvents( data->window, DWET_NONE, mask );
}

static DFBResult
IDirectFBWindow_DisableEvents( IDirectFBWindow       *thiz,
                               DFBWindowEventType     mask )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (mask & ~DWET_ALL)
          return DFB_INVARG;

     return CoreWindow_ChangeEvents( data->window, mask, DWET_NONE );
}

static DFBResult
IDirectFBWindow_GetID( IDirectFBWindow *thiz,
                       DFBWindowID     *id )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!id)
          return DFB_INVARG;

     *id = data->window->id;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_GetPosition( IDirectFBWindow *thiz,
                             int             *x,
                             int             *y )
{
     DFBInsets insets;
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );


     if (data->destroyed)
          return DFB_DESTROYED;

     if (!x && !y)
          return DFB_INVARG;

     dfb_windowstack_lock( data->window->stack );
     dfb_wm_get_insets( data->window->stack, data->window, &insets );
     dfb_windowstack_unlock( data->window->stack );

     if (x)
          *x = data->window->config.bounds.x-insets.l;

     if (y)
          *y = data->window->config.bounds.y-insets.t;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_GetSize( IDirectFBWindow *thiz,
                         int             *width,
                         int             *height )
{
     DFBInsets insets;
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!width && !height)
          return DFB_INVARG;

     dfb_windowstack_lock( data->window->stack );
     dfb_wm_get_insets( data->window->stack, data->window, &insets );
     dfb_windowstack_unlock( data->window->stack );

     if (width)
          *width = data->window->config.bounds.w-insets.l-insets.r;

     if (height)
          *height = data->window->config.bounds.h-insets.t-insets.b;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_GetSurface( IDirectFBWindow   *thiz,
                            IDirectFBSurface **surface )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!surface)
          return DFB_INVARG;

     if (data->window->caps & (DWCAPS_INPUTONLY | DWCAPS_COLOR))
          return DFB_UNSUPPORTED;

     if (!data->surface) {
          DFBResult ret;

          DIRECT_ALLOCATE_INTERFACE( *surface, IDirectFBSurface );

          ret = IDirectFBSurface_Window_Construct( *surface, NULL,
                                                   NULL, NULL, data->window,
                                                   DSCAPS_DOUBLE, data->core );
          if (ret)
               return ret;

          data->surface = *surface;
     }
     else
          *surface = data->surface;

     data->surface->AddRef( data->surface );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetProperty( IDirectFBWindow   *thiz,
                             const char        *key,
                             void              *value,
                             void            **old_value )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     /* Check arguments */
     if (data->destroyed)
          return DFB_DESTROYED;

     if (!key)
          return DFB_INVARG;

     dfb_windowstack_lock( data->window->stack );
     ret = dfb_wm_set_window_property( data->window->stack, data->window, key, value, old_value );
     dfb_windowstack_unlock( data->window->stack );

     return ret;
}

static DFBResult
IDirectFBWindow_GetProperty( IDirectFBWindow  *thiz,
                             const char       *key,
                             void            **ret_value )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!key)
          return DFB_INVARG;

     if (!ret_value)
          return DFB_INVARG;

     dfb_windowstack_lock( data->window->stack );
     ret = dfb_wm_get_window_property( data->window->stack, data->window, key, ret_value );
     dfb_windowstack_unlock( data->window->stack );

     return ret;
}

static DFBResult
IDirectFBWindow_RemoveProperty( IDirectFBWindow  *thiz,
                                const char       *key,
                                void            **ret_value )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!key)
          return DFB_INVARG;

     dfb_windowstack_lock( data->window->stack );
     ret = dfb_wm_remove_window_property( data->window->stack, data->window, key, ret_value );
     dfb_windowstack_unlock( data->window->stack );

     return ret;
}

static DFBResult
IDirectFBWindow_SetOptions( IDirectFBWindow  *thiz,
                            DFBWindowOptions  options )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     /* Check arguments */
     if (data->destroyed)
          return DFB_DESTROYED;

     if (options & ~DWOP_ALL)
          return DFB_INVARG;

     if (!(data->window->caps & DWCAPS_ALPHACHANNEL))
          options &= ~DWOP_ALPHACHANNEL;

     /* Set new options */
     return CoreWindow_ChangeOptions( data->window, DWOP_ALL, options );
}

static DFBResult
IDirectFBWindow_GetOptions( IDirectFBWindow  *thiz,
                            DFBWindowOptions *ret_options )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!ret_options)
          return DFB_INVARG;

     *ret_options = data->window->config.options;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetColor( IDirectFBWindow *thiz,
                          u8               r,
                          u8               g,
                          u8               b,
                          u8               a )
{
     DFBColor color = { a: a, r: r, g: g, b: b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     CoreWindow_SetColor( data->window, &color );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetColorKey( IDirectFBWindow *thiz,
                             u8               r,
                             u8               g,
                             u8               b )
{
     u32          key;
     CoreSurface *surface;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (data->window->caps & DWCAPS_INPUTONLY)
          return DFB_UNSUPPORTED;

     surface = data->window->surface;

     if (DFB_PIXELFORMAT_IS_INDEXED( surface->config.format ))
          key = dfb_palette_search( surface->palette, r, g, b, 0x80 );
     else
          key = dfb_color_to_pixel( surface->config.format, r, g, b );

     return CoreWindow_SetColorKey( data->window, key );
}

static DFBResult
IDirectFBWindow_SetColorKeyIndex( IDirectFBWindow *thiz,
                                  unsigned int     index )
{
     u32 key = index;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (data->window->caps & DWCAPS_INPUTONLY)
          return DFB_UNSUPPORTED;

     return CoreWindow_SetColorKey( data->window, key );
}

static DFBResult
IDirectFBWindow_SetOpaqueRegion( IDirectFBWindow *thiz,
                                 int              x1,
                                 int              y1,
                                 int              x2,
                                 int              y2 )
{
     DFBRegion region;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (x1 > x2 || y1 > y2)
          return DFB_INVAREA;

     region = (DFBRegion) { x1, y1, x2, y2 };

     return CoreWindow_SetOpaque( data->window, &region );
}

static DFBResult
IDirectFBWindow_SetOpacity( IDirectFBWindow *thiz,
                            u8               opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_SetOpacity( data->window, opacity );
}

static DFBResult
IDirectFBWindow_GetOpacity( IDirectFBWindow *thiz,
                            u8              *opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!opacity)
          return DFB_INVARG;

     *opacity = data->window->config.opacity;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetCursorShape( IDirectFBWindow  *thiz,
                                IDirectFBSurface *shape,
                                int               hot_x,
                                int               hot_y )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (data->cursor.shape) {
          data->cursor.shape->Release( data->cursor.shape );
          data->cursor.shape = NULL;
     }

     if (shape) {
          IDirectFBSurface_data *shape_data;
          CoreSurface           *shape_surface;

          shape_data = (IDirectFBSurface_data*) shape->priv;
          if (!shape_data)
               return DFB_DEAD;

          shape_surface = shape_data->surface;
          if (!shape_surface)
               return DFB_DESTROYED;

          ret = shape->AddRef( shape );
          if (ret)
               return ret;

          data->cursor.shape = shape;
          data->cursor.hot_x = hot_x;
          data->cursor.hot_y = hot_y;

          if (data->entered)
               return dfb_windowstack_cursor_set_shape( data->window->stack,
                                                        shape_surface, hot_x, hot_y );
     }

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_RequestFocus( IDirectFBWindow *thiz )
{
     CoreWindow *window;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     window = data->window;

     if (window->config.options & DWOP_GHOST)
          return DFB_UNSUPPORTED;

     if (!window->config.opacity && !(window->caps & DWCAPS_INPUTONLY))
          return DFB_UNSUPPORTED;

     return CoreWindow_RequestFocus( window );
}

static DFBResult
IDirectFBWindow_GrabKeyboard( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_KEYBOARD, true );
}

static DFBResult
IDirectFBWindow_UngrabKeyboard( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_KEYBOARD, false );
}

static DFBResult
IDirectFBWindow_GrabPointer( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_POINTER, true );
}

static DFBResult
IDirectFBWindow_UngrabPointer( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_POINTER, false );
}

static DFBResult
IDirectFBWindow_GrabKey( IDirectFBWindow            *thiz,
                         DFBInputDeviceKeySymbol     symbol,
                         DFBInputDeviceModifierMask  modifiers )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_GrabKey( data->window, symbol, modifiers );
}

static DFBResult
IDirectFBWindow_UngrabKey( IDirectFBWindow            *thiz,
                           DFBInputDeviceKeySymbol     symbol,
                           DFBInputDeviceModifierMask  modifiers )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_UngrabKey( data->window, symbol, modifiers );
}

static DFBResult
IDirectFBWindow_Move( IDirectFBWindow *thiz, int dx, int dy )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (dx == 0  &&  dy == 0)
          return DFB_OK;

     return CoreWindow_Move( data->window, dx, dy );
}

static DFBResult
IDirectFBWindow_MoveTo( IDirectFBWindow *thiz, int x, int y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_MoveTo( data->window, x, y );
}

static DFBResult
IDirectFBWindow_Resize( IDirectFBWindow *thiz,
                        int              width,
                        int              height )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (width < 1 || width > dfb_config->mst_ge_hw_limit || height < 1 || height > dfb_config->mst_ge_hw_limit)
     {
         D_ERROR("[DFB] (%s line %d) width = %d, height = %d, mst_ge_hw_limit = %d",__FUNCTION__,__LINE__, width, height, dfb_config->mst_ge_hw_limit);
         return DFB_INVARG;
     }
     
     return CoreWindow_Resize( data->window, width, height );
}

static DFBResult
IDirectFBWindow_Raise( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, data->window, 1 );
}

static DFBResult
IDirectFBWindow_SetStackingClass( IDirectFBWindow        *thiz,
                                  DFBWindowStackingClass  stacking_class )
{
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     switch (stacking_class) {
          case DWSC_MIDDLE:
          case DWSC_UPPER:
          case DWSC_LOWER:
               break;
          default:
               return DFB_INVARG;
     }

     config.stacking = stacking_class;

     return CoreWindow_SetStacking( data->window, stacking_class );
}

static DFBResult
IDirectFBWindow_Lower( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, data->window, -1 );
}

static DFBResult
IDirectFBWindow_RaiseToTop( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, NULL, 1 );
}

static DFBResult
IDirectFBWindow_LowerToBottom( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, NULL, -1 );
}

static DFBResult
IDirectFBWindow_PutAtop( IDirectFBWindow *thiz,
                         IDirectFBWindow *lower )
{
     IDirectFBWindow_data *lower_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!lower)
          return DFB_INVARG;

     lower_data = (IDirectFBWindow_data*) lower->priv;
     if (!lower_data)
          return DFB_DEAD;

     if (!lower_data->window)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, lower_data->window, 1 );
}

static DFBResult
IDirectFBWindow_PutBelow( IDirectFBWindow *thiz,
                           IDirectFBWindow *upper )
{
     IDirectFBWindow_data *upper_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!upper)
          return DFB_INVARG;

     upper_data = (IDirectFBWindow_data*) upper->priv;
     if (!upper_data)
          return DFB_DEAD;

     if (!upper_data->window)
          return DFB_DESTROYED;

     return CoreWindow_Restack( data->window, upper_data->window, -1 );
}

static DFBResult
IDirectFBWindow_Close( IDirectFBWindow *thiz )
{
     DFBWindowEvent evt;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     evt.type = DWET_CLOSE;

     dfb_window_post_event( data->window, &evt );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_Destroy( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     D_DEBUG_AT( IDirectFB_Window, "IDirectFBWindow_Destroy()\n" );

     CoreWindow_Destroy( data->window );

     if(dfb_config->mst_fixed_mem_leak_patch_enable && data->detached)
        dfb_window_detach( data->window, &data->reaction );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetBounds( IDirectFBWindow *thiz,
                           int              x,
                           int              y,
                           int              width,
                           int              height )
{
     DFBRectangle rect = {
          x, y, width, height
     };

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     D_DEBUG_AT( IDirectFB_Window, "IDirectFBWindow_SetBounds( %d, %d - %dx%d )\n", x, y, width, height );

     return CoreWindow_SetBounds( data->window, &rect );
}

static DFBResult
IDirectFBWindow_ResizeSurface( IDirectFBWindow *thiz,
                               int              width,
                               int              height )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (width < 1 || width > dfb_config->mst_ge_hw_limit || height < 1 || height > dfb_config->mst_ge_hw_limit)
     {
         D_ERROR("[DFB] (%s line %d) width = %d, height = %d, mst_ge_hw_limit = %d",__FUNCTION__,__LINE__, width, height, dfb_config->mst_ge_hw_limit);
         return DFB_INVARG;
     }

     return dfb_surface_reformat( data->window->surface, width, height, data->window->surface->config.format );
}

static DFBResult
IDirectFBWindow_SetKeySelection( IDirectFBWindow               *thiz,
                                 DFBWindowKeySelection          selection,
                                 const DFBInputDeviceKeySymbol *keys,
                                 unsigned int                   num_keys )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     /* What a lovely switch */
     switch (selection) {
         case DWKS_ALL:
         case DWKS_NONE:
             break;
         case DWKS_LIST:
             if (!keys || num_keys == 0)
         default:
                 return DFB_INVARG;
     }

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_SetKeySelection( data->window, selection, keys, num_keys );
}

static DFBResult
IDirectFBWindow_GrabUnselectedKeys( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_UNSELECTED_KEYS, true );
}

static DFBResult
IDirectFBWindow_UngrabUnselectedKeys( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_ChangeGrab( data->window, CWMGT_UNSELECTED_KEYS, false );
}

static DFBResult
IDirectFBWindow_Bind( IDirectFBWindow *thiz,
                      IDirectFBWindow *source,
                      int              x,
                      int              y )
{
     IDirectFBWindow_data *source_data;
 
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     DIRECT_INTERFACE_GET_DATA_FROM(source, source_data, IDirectFBWindow);

     if (source_data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Bind( data->window, source_data->window, x, y );
}

static DFBResult
IDirectFBWindow_Unbind( IDirectFBWindow *thiz,
                        IDirectFBWindow *source )
{
     IDirectFBWindow_data *source_data;
 
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     DIRECT_INTERFACE_GET_DATA_FROM(source, source_data, IDirectFBWindow);

     if (source_data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_Unbind( data->window, source_data->window );
}

static DFBResult
CheckGeometry( const DFBWindowGeometry *geometry )
{
     if (!geometry)
          return DFB_INVARG;

     switch (geometry->mode) {
          case DWGM_DEFAULT:
          case DWGM_FOLLOW:
               break;

          case DWGM_RECTANGLE:
               if (geometry->rectangle.x < 0 ||
                   geometry->rectangle.y < 0 ||
                   geometry->rectangle.w < 1 ||
                   geometry->rectangle.h < 1)
                    return DFB_INVARG;
               break;

          case DWGM_LOCATION:
               if (geometry->location.x < 0.0f ||
                   geometry->location.y < 0.0f ||
                   geometry->location.w > 1.0f ||
                   geometry->location.h > 1.0f ||
                   geometry->location.w <= 0.0f ||
                   geometry->location.h <= 0.0f ||
                   geometry->location.x + geometry->location.w > 1.0f ||
                   geometry->location.y + geometry->location.h > 1.0f)
                    return DFB_INVARG;
               break;

          default:
               return DFB_INVARG;
     }

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetSrcGeometry( IDirectFBWindow         *thiz,
                                const DFBWindowGeometry *geometry )
{
     DFBResult        ret;
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     ret = CheckGeometry( geometry );
     if (ret)
          return ret;

     if (data->destroyed)
          return DFB_DESTROYED;

     config.src_geometry = *geometry;

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_SRC_GEOMETRY );
}

static DFBResult
IDirectFBWindow_SetDstGeometry( IDirectFBWindow         *thiz,
                                const DFBWindowGeometry *geometry )
{
     DFBResult        ret;
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     ret = CheckGeometry( geometry );
     if (ret)
          return ret;

     if (data->destroyed)
          return DFB_DESTROYED;

     config.dst_geometry = *geometry;

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_DST_GEOMETRY );
}

static DFBResult
IDirectFBWindow_SetRotation( IDirectFBWindow *thiz,
                             int              rotation )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     return CoreWindow_SetRotation( data->window, rotation % 360 );
}

static DFBResult
IDirectFBWindow_SetAssociation( IDirectFBWindow *thiz,
                                DFBWindowID      window_id )
{
     DFBResult         ret;
     CoreWindowConfig  config;
     CoreWindow       *window;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     config.association = window_id;

     ret = dfb_core_get_window( data->core, window_id, &window );
     if (ret)
          return ret;

     ret = CoreWindow_SetConfig( data->window, &config, NULL, 0, window, CWCF_ASSOCIATION );

     dfb_window_unref( window );

     return ret;
}

static DFBResult
IDirectFBWindow_SetApplicationID( IDirectFBWindow *thiz,
                                  unsigned long    application_id )
                                  
{
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     config.application_id = application_id;

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_APPLICATION_ID );
}

static DFBResult
IDirectFBWindow_GetApplicationID( IDirectFBWindow *thiz,
                                  unsigned long   *ret_application_id )
                                  
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!ret_application_id)
          return DFB_INVARG;

     *ret_application_id = data->window->config.application_id;

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_BeginUpdates( IDirectFBWindow *thiz,
                              const DFBRegion *update )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_BeginUpdates( data->window, update );
}

static DFBResult
IDirectFBWindow_SendEvent( IDirectFBWindow      *thiz,
                           const DFBWindowEvent *event )
{
     DFBWindowEvent evt;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s\n", __FUNCTION__ );

     if (!event)
          return DFB_INVARG;

     if (data->destroyed)
          return DFB_DESTROYED;

     evt = *event;

     dfb_window_post_event( data->window, &evt );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetCursorFlags( IDirectFBWindow      *thiz,
                                DFBWindowCursorFlags  flags )
{
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s( 0x%04x )\n", __FUNCTION__, flags );

     if (flags & ~DWCF_ALL)
          return DFB_INVARG;

     if (data->destroyed)
          return DFB_DESTROYED;

     config.cursor_flags = flags;

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_CURSOR_FLAGS );
}

static DFBResult
IDirectFBWindow_SetCursorResolution( IDirectFBWindow    *thiz,
                                     const DFBDimension *resolution )
{
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     if (resolution)
          D_DEBUG_AT( IDirectFB_Window, "%s( %dx%d )\n", __FUNCTION__, resolution->w, resolution->h );
     else
          D_DEBUG_AT( IDirectFB_Window, "%s( NULL )\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (resolution)
          config.cursor_resolution = *resolution;
     else {
          config.cursor_resolution.w = 0;
          config.cursor_resolution.h = 0;
     }

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_CURSOR_RESOLUTION );
}

static DFBResult
IDirectFBWindow_SetCursorPosition( IDirectFBWindow    *thiz,
                                   int                 x,
                                   int                 y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s( %d,%d )\n", __FUNCTION__, x, y );

     if (data->destroyed)
          return DFB_DESTROYED;

     return CoreWindow_SetCursorPosition( data->window, x, y );
}

static DFBResult
IDirectFBWindow_SetStereoDepth( IDirectFBWindow *thiz,
                                int              z )
{
     CoreWindowConfig config;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (z < -DLSO_FIXED_LIMIT || z > DLSO_FIXED_LIMIT)
          return DFB_INVARG;

     if (!(data->window->caps & DWCAPS_LR_MONO) &&
         !(data->window->caps & DWCAPS_STEREO)) {
          return DFB_INVARG;
     }

     config.z = z;

     return CoreWindow_SetConfig( data->window, &config, NULL, 0, NULL, CWCF_STEREO_DEPTH );
}

static DFBResult
IDirectFBWindow_GetStereoDepth( IDirectFBWindow *thiz,
                                int             *z )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     if (data->destroyed)
          return DFB_DESTROYED;

     if (!(data->window->caps & DWCAPS_LR_MONO) &&
         !(data->window->caps & DWCAPS_STEREO)) {
          return DFB_INVARG;
     }

     if (!z)
          return DFB_INVARG;

     *z = data->window->config.z;

     return DFB_OK;
}

/*
WRAP the IDirectFBWindow_xxxx to _IDirectFBWindow_xxxx  for safe call
*/


static DirectResult
_IDirectFBWindow_AddRef( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_AddRef(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DirectResult
_IDirectFBWindow_Release( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_CreateEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer **buffer )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_CreateEventBuffer(thiz,buffer);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_AttachEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer  *buffer )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_AttachEventBuffer(thiz,buffer);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_DetachEventBuffer( IDirectFBWindow       *thiz,
                                   IDirectFBEventBuffer  *buffer )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_DetachEventBuffer(thiz,buffer);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_EnableEvents( IDirectFBWindow       *thiz,
                              DFBWindowEventType     mask )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_EnableEvents(thiz,mask);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_DisableEvents( IDirectFBWindow       *thiz,
                               DFBWindowEventType     mask )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_DisableEvents(thiz,mask);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetID( IDirectFBWindow *thiz,
                       DFBWindowID     *id )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetID(thiz,id);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetPosition( IDirectFBWindow *thiz,
                             int             *x,
                             int             *y )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetPosition(thiz,x,y);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetSize( IDirectFBWindow *thiz,
                         int             *width,
                         int             *height )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetSize(thiz,width,height);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetSurface( IDirectFBWindow   *thiz,
                            IDirectFBSurface **surface )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetSurface(thiz,surface);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetProperty( IDirectFBWindow   *thiz,
                             const char        *key,
                             void              *value,
                             void            **old_value )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetProperty(thiz,key,value,old_value);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetProperty( IDirectFBWindow  *thiz,
                             const char       *key,
                             void            **ret_value )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetProperty(thiz,key,ret_value);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_RemoveProperty( IDirectFBWindow  *thiz,
                                const char       *key,
                                void            **ret_value )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_RemoveProperty(thiz,key,ret_value);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetOptions( IDirectFBWindow  *thiz,
                            DFBWindowOptions  options )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetOptions(thiz,options);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;

}

static DFBResult
_IDirectFBWindow_GetOptions( IDirectFBWindow  *thiz,
                            DFBWindowOptions *ret_options )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetOptions(thiz,ret_options);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetColor( IDirectFBWindow *thiz,
                          u8               r,
                          u8               g,
                          u8               b,
                          u8               a )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetColor(thiz,r,g,b,a);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetColorKey( IDirectFBWindow *thiz,
                             u8               r,
                             u8               g,
                             u8               b )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetColorKey(thiz,r,g,b);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetColorKeyIndex( IDirectFBWindow *thiz,
                                  unsigned int     index )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetColorKeyIndex(thiz,index);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetOpaqueRegion( IDirectFBWindow *thiz,
                                 int              x1,
                                 int              y1,
                                 int              x2,
                                 int              y2 )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetOpaqueRegion(thiz,x1,y1,x2,y2);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetOpacity( IDirectFBWindow *thiz,
                            u8               opacity )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetOpacity(thiz,opacity);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetOpacity( IDirectFBWindow *thiz,
                            u8              *opacity )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetOpacity(thiz,opacity);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetCursorShape( IDirectFBWindow  *thiz,
                                IDirectFBSurface *shape,
                                int               hot_x,
                                int               hot_y )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetCursorShape(thiz,shape,hot_x,hot_y);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_RequestFocus( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_RequestFocus(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GrabKeyboard( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GrabKeyboard(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_UngrabKeyboard( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_UngrabKeyboard(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GrabPointer( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GrabPointer(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_UngrabPointer( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_UngrabPointer(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GrabKey( IDirectFBWindow            *thiz,
                         DFBInputDeviceKeySymbol     symbol,
                         DFBInputDeviceModifierMask  modifiers )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GrabKey(thiz,symbol,modifiers);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_UngrabKey( IDirectFBWindow            *thiz,
                           DFBInputDeviceKeySymbol     symbol,
                           DFBInputDeviceModifierMask  modifiers )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_UngrabKey(thiz,symbol,modifiers);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Move( IDirectFBWindow *thiz, int dx, int dy )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Move(thiz,dx,dy);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_MoveTo( IDirectFBWindow *thiz, int x, int y )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_MoveTo(thiz,x,y);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;

}

static DFBResult
_IDirectFBWindow_Resize( IDirectFBWindow *thiz,
                        int              width,
                        int              height )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Resize(thiz,width,height);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Raise( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Raise(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetStackingClass( IDirectFBWindow        *thiz,
                                  DFBWindowStackingClass  stacking_class )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetStackingClass(thiz,stacking_class);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Lower( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Lower(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_RaiseToTop( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_RaiseToTop(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_LowerToBottom( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_LowerToBottom(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_PutAtop( IDirectFBWindow *thiz,
                         IDirectFBWindow *lower )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_PutAtop(thiz,lower);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_PutBelow( IDirectFBWindow *thiz,
                           IDirectFBWindow *upper )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_PutBelow(thiz,upper);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Close( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Close(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Destroy( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Destroy(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetBounds( IDirectFBWindow *thiz,
                           int              x,
                           int              y,
                           int              width,
                           int              height )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetBounds(thiz,x,y,width,height);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_ResizeSurface( IDirectFBWindow *thiz,
                               int              width,
                               int              height )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_ResizeSurface(thiz,width,height);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetKeySelection( IDirectFBWindow               *thiz,
                                 DFBWindowKeySelection          selection,
                                 const DFBInputDeviceKeySymbol *keys,
                                 unsigned int                   num_keys )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetKeySelection(thiz,selection,keys,num_keys);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GrabUnselectedKeys( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GrabUnselectedKeys(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_UngrabUnselectedKeys( IDirectFBWindow *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_UngrabUnselectedKeys(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Bind( IDirectFBWindow *thiz,
                      IDirectFBWindow *source,
                      int              x,
                      int              y )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Bind(thiz,source,x,y);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_Unbind( IDirectFBWindow *thiz,
                        IDirectFBWindow *source )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_Unbind(thiz,source);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}



static DFBResult
_IDirectFBWindow_SetSrcGeometry( IDirectFBWindow         *thiz,
                                const DFBWindowGeometry *geometry )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetSrcGeometry(thiz,geometry);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetDstGeometry( IDirectFBWindow         *thiz,
                                const DFBWindowGeometry *geometry )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetDstGeometry(thiz,geometry);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetRotation( IDirectFBWindow *thiz,
                             int              rotation )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetRotation(thiz,rotation);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetAssociation( IDirectFBWindow *thiz,
                                DFBWindowID      window_id )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetAssociation(thiz,window_id);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_SetApplicationID( IDirectFBWindow *thiz,
                                  unsigned long    application_id )

{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_SetApplicationID(thiz,application_id);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBWindow_GetApplicationID( IDirectFBWindow *thiz,
                                  unsigned long   *ret_application_id )

{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBWindow_GetApplicationID(thiz,ret_application_id);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}


DFBResult
IDirectFBWindow_Construct( IDirectFBWindow *thiz,
                           CoreWindow      *window,
                           CoreLayer       *layer,
                           CoreDFB         *core,
                           IDirectFB       *idirectfb )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBWindow)

     D_DEBUG_AT( IDirectFB_Window, "IDirectFBWindow_Construct() <- %d, %d - %dx%d\n",
                 DFB_RECTANGLE_VALS( &window->config.bounds ) );

     data->ref       = 1;
     data->window    = window;
     data->layer     = layer;
     data->core      = core;
     data->idirectfb = idirectfb;

     dfb_window_attach( window, IDirectFBWindow_React, data, &data->reaction );

     thiz->AddRef = _IDirectFBWindow_AddRef;
     thiz->Release = _IDirectFBWindow_Release;
     thiz->CreateEventBuffer = _IDirectFBWindow_CreateEventBuffer;
     thiz->AttachEventBuffer = _IDirectFBWindow_AttachEventBuffer;
     thiz->DetachEventBuffer = _IDirectFBWindow_DetachEventBuffer;
     thiz->EnableEvents = _IDirectFBWindow_EnableEvents;
     thiz->DisableEvents = _IDirectFBWindow_DisableEvents;
     thiz->GetID = _IDirectFBWindow_GetID;
     thiz->GetPosition = _IDirectFBWindow_GetPosition;
     thiz->GetSize = _IDirectFBWindow_GetSize;
     thiz->GetSurface = _IDirectFBWindow_GetSurface;
     thiz->SetProperty = _IDirectFBWindow_SetProperty;
     thiz->GetProperty = _IDirectFBWindow_GetProperty;
     thiz->RemoveProperty = _IDirectFBWindow_RemoveProperty;
     thiz->SetOptions = _IDirectFBWindow_SetOptions;
     thiz->GetOptions = _IDirectFBWindow_GetOptions;
     thiz->SetColor = _IDirectFBWindow_SetColor;
     thiz->SetColorKey = _IDirectFBWindow_SetColorKey;
     thiz->SetColorKeyIndex = _IDirectFBWindow_SetColorKeyIndex;
     thiz->SetOpaqueRegion = _IDirectFBWindow_SetOpaqueRegion;
     thiz->SetOpacity = _IDirectFBWindow_SetOpacity;
     thiz->GetOpacity = _IDirectFBWindow_GetOpacity;
     thiz->SetCursorShape = _IDirectFBWindow_SetCursorShape;
     thiz->RequestFocus = _IDirectFBWindow_RequestFocus;
     thiz->GrabKeyboard = _IDirectFBWindow_GrabKeyboard;
     thiz->UngrabKeyboard = _IDirectFBWindow_UngrabKeyboard;
     thiz->GrabPointer = _IDirectFBWindow_GrabPointer;
     thiz->UngrabPointer = _IDirectFBWindow_UngrabPointer;
     thiz->GrabKey = _IDirectFBWindow_GrabKey;
     thiz->UngrabKey = _IDirectFBWindow_UngrabKey;
     thiz->Move = _IDirectFBWindow_Move;
     thiz->MoveTo = _IDirectFBWindow_MoveTo;
     thiz->Resize = _IDirectFBWindow_Resize;
     thiz->SetStackingClass = _IDirectFBWindow_SetStackingClass;
     thiz->Raise = _IDirectFBWindow_Raise;
     thiz->Lower = _IDirectFBWindow_Lower;
     thiz->RaiseToTop = _IDirectFBWindow_RaiseToTop;
     thiz->LowerToBottom = _IDirectFBWindow_LowerToBottom;
     thiz->PutAtop = _IDirectFBWindow_PutAtop;
     thiz->PutBelow = _IDirectFBWindow_PutBelow;
     thiz->Close = _IDirectFBWindow_Close;
     thiz->Destroy = _IDirectFBWindow_Destroy;
     thiz->SetBounds = _IDirectFBWindow_SetBounds;
     thiz->ResizeSurface = _IDirectFBWindow_ResizeSurface;
     thiz->Bind = _IDirectFBWindow_Bind;
     thiz->Unbind = _IDirectFBWindow_Unbind;
     thiz->SetKeySelection = _IDirectFBWindow_SetKeySelection;
     thiz->GrabUnselectedKeys = _IDirectFBWindow_GrabUnselectedKeys;
     thiz->UngrabUnselectedKeys = _IDirectFBWindow_UngrabUnselectedKeys;
     thiz->SetSrcGeometry = _IDirectFBWindow_SetSrcGeometry;
     thiz->SetDstGeometry = _IDirectFBWindow_SetDstGeometry;
     thiz->SetRotation = _IDirectFBWindow_SetRotation;
     thiz->SetAssociation = _IDirectFBWindow_SetAssociation;
     thiz->SetApplicationID = _IDirectFBWindow_SetApplicationID;
     thiz->GetApplicationID = _IDirectFBWindow_GetApplicationID;
     thiz->BeginUpdates = IDirectFBWindow_BeginUpdates;
     thiz->SendEvent = IDirectFBWindow_SendEvent;
     thiz->SetCursorFlags = IDirectFBWindow_SetCursorFlags;
     thiz->SetCursorResolution = IDirectFBWindow_SetCursorResolution;
     thiz->SetCursorPosition = IDirectFBWindow_SetCursorPosition;
     thiz->GetStereoDepth = IDirectFBWindow_GetStereoDepth;
     thiz->SetStereoDepth = IDirectFBWindow_SetStereoDepth;

     return DFB_OK;
}


/* internals */

static ReactionResult
IDirectFBWindow_React( const void *msg_data,
                       void       *ctx )
{
     const DFBWindowEvent *evt  = msg_data;
     IDirectFBWindow_data *data = ctx;

     D_DEBUG_AT( IDirectFB_Window, "%s()\n", __FUNCTION__ );

     switch (evt->type) {
          case DWET_DESTROYED:
               D_DEBUG_AT( IDirectFB_Window, "  -> window destroyed\n" );
               
               data->detached = true;
               data->destroyed = true;
               
               return RS_REMOVE;

          case DWET_LEAVE:
               data->entered = false;
               break;

          case DWET_ENTER:
               data->entered = true;

               if (data->cursor.shape) {
                    IDirectFBSurface_data* shape_data;

                    shape_data = (IDirectFBSurface_data*) data->cursor.shape->priv;
                    if (!shape_data)
                         break;

                    if (!shape_data->surface)
                         break;

                    dfb_windowstack_cursor_set_shape( data->window->stack,
                                                      shape_data->surface,
                                                      data->cursor.hot_x,
                                                      data->cursor.hot_y );

                    dfb_windowstack_cursor_set_opacity( data->window->stack, 0xff );
               }

               break;

          case DWET_GOTFOCUS:
          case DWET_LOSTFOCUS:
               IDirectFB_SetAppFocus( data->idirectfb,
                                      evt->type == DWET_GOTFOCUS );

          default:
               break;
     }

     return RS_OK;
}

