/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <neo@directfb.org>.

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

//#define DIRECT_ENABLE_DEBUG

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <direct/clock.h>
#include <direct/messages.h>

#include <directfb.h>
#include <directfb_util.h>

#include <core/windows_internal.h>

#ifdef HAVE_FUSIONDALE
#include <fusiondale.h>
#endif

#include <sawman.h>
#include <sawman_manager.h>

#define MAX_WINDOWS 4
#define MAX_LAYOUTS 4

#define NUM_LAYERS  3

typedef struct __SampleAppMan_TVManager   TVManager;
typedef struct __SampleAppMan_Layout      Layout;
typedef struct __SampleAppMan_LayoutGroup LayoutGroup;
typedef struct __SampleAppMan_Application Application;

/**********************************************************************************************************************/

typedef enum {
     TVMET_STOP_PID
} TVManagerEventType;

struct __SampleAppMan_LayoutGroup {
     DFBWindowStackingClass  stacking;

     SaWManWindow           *windows[MAX_WINDOWS];
     int                     num_windows;

     int                     current_layout;
     bool                    toggle_on_switch;
};

struct __SampleAppMan_TVManager {
     int                magic;

     ISaWMan           *saw;
     ISaWManManager    *manager;

     SaWManScalingMode  scaling_mode;

     LayoutGroup        groups[NUM_LAYERS];

     const Layout      *layouts[MAX_LAYOUTS];
     int                num_layouts;

     DirectLink        *applications;
};

struct __SampleAppMan_Layout {
     void **data;

     void (*Relayout)    ( TVManager    *tm,
                           LayoutGroup  *group,
                           void         *layout_data );

     void (*AddWindow)   ( TVManager    *tm,
                           LayoutGroup  *group,
                           void         *layout_data,
                           SaWManWindow *window );

     void (*RemoveWindow)( TVManager    *tm,
                           LayoutGroup  *group,
                           void         *layout_data,
                           SaWManWindow *window,
                           int           index );

     void (*ToggleWindow)( TVManager    *tm,
                           LayoutGroup  *group,
                           void         *layout_data,
                           SaWManWindow *window,
                           int           index );
};

struct __SampleAppMan_Application {
     DirectLink         link;

     int                magic;

     const char        *name;
     const char        *args[64];

     bool               started;

     long long          start_time;
     pid_t              pid;
     SaWManProcess     *process;
     SaWManWindow      *window;    /* FIXME: support multiple application windows */
};

/**********************************************************************************************************************/

static IDirectFB             *dfb    = NULL;
static IDirectFBDisplayLayer *layer  = NULL;
static IDirectFBWindow       *window = NULL;
static IDirectFBEventBuffer  *events = NULL;

/**********************************************************************************************************************/

#ifdef HAVE_FUSIONDALE
static IFusionDale           *pDale      = NULL;
static IFusionDaleMessenger  *pMessenger = NULL;

static FDMessengerEventID     gAppRunEventID;
static FDMessengerEventID     gAppKillEventID;
#endif

/**********************************************************************************************************************/

static const SaWManCallbacks  tvManagerCallbacks;
static const Layout           mosaicLayout;
static const Layout           pipLayout;

/**********************************************************************************************************************/

static Application apps[] = {
     {
          name: "Slideview",
          args: {   "./slideview",
                    "images/image1.png",
                    "images/image2.png",
                    "images/image3.png",
                    "images/image4.png",
                    "images/image5.png",
                    "images/image6.png",
                    "images/image7.png",
                    NULL
               }
     },

     {
          name: "Opera Browser",
          args: {   "./opera/run-Opera",
                    NULL
               }
     },

     { 
          name: "Penguin Demo",
          args: {   "./df_andi",
                    NULL
               }
     },

     {
          name: "Spinning Icons",
          args: {   "./df_neo",
                    NULL
               }
     },

     {
          name: "ClanBomber",
          args: {   "./clanbomber2",
                    NULL
               }
     },

     {
          name: "Geki3",
          args: {   "./geki3",
                    NULL
               }
     },
/*
     {
          name: "Burning Screen",
          args: {   "./df_fire",
                    NULL
               }
     },

     {
          name: "Screen Saver",
          args: {   "./sampleSaver",
                    NULL
               }
     },
*/
};

/**********************************************************************************************************************/

static void InitDirectFB( int *argc, char **argv[] );
static void InitFusionDale();
static void InitSaWMan( TVManager *manager, int *argc, char **argv[] );

static void AddApplication( TVManager   *manager,
                            Application *app );

/**********************************************************************************************************************/

#define START_STOP(saw,app)                            \
do {                                                   \
     if ((app)->started)                               \
          (saw)->Stop( saw, (app)->pid );              \
     else                                              \
          (saw)->Start( saw, (app)->name, NULL );      \
} while (0)

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     int       i;
     TVManager manager;

     /* Initialize DirectFB. */
     InitDirectFB( &argc, &argv );

     /* Initialize FusionDale. */
     InitFusionDale();

     /* Initialize SaWMan and our manager. */
     InitSaWMan( &manager, &argc, &argv );

     /* Add available applications. */
     for (i=0; i<D_ARRAY_SIZE(apps); i++)
          AddApplication( &manager, &apps[i] );

     /* Grab the keys we need. */
     window->GrabKey( window, DIKS_RED, 0 );
     window->GrabKey( window, DIKS_BLUE, 0 );
     window->GrabKey( window, DIKS_POWER, 0 );
     window->GrabKey( window, DIKS_1, 0 );
     window->GrabKey( window, DIKS_2, 0 );
     window->GrabKey( window, DIKS_3, 0 );
     window->GrabKey( window, DIKS_4, 0 );
     window->GrabKey( window, DIKS_5, 0 );
     window->GrabKey( window, DIKS_6, 0 );
     window->GrabKey( window, DIKS_7, 0 );
     window->GrabKey( window, DIKS_8, 0 );
     window->GrabKey( window, DIKS_9, 0 );
     window->GrabKey( window, DIKS_CUSTOM0, 0 );
     window->GrabKey( window, DIKS_TEXT, 0 );

     /* Main loop. */
     while (1) {
          DFBEvent event;

          events->WaitForEvent( events );

          /* Check for new events. */
          while (events->GetEvent( events, &event ) == DFB_OK) {
               switch (event.clazz) {
                    case DFEC_WINDOW:
                         switch (event.window.type) {
                              case DWET_KEYDOWN:
                                   switch (event.window.key_symbol) {
                                        case DIKS_RED:
                                             break;

                                        case DIKS_BLUE:
                                             break;

                                        case DIKS_POWER:
                                             break;

                                        case DIKS_1:
                                        case DIKS_2:
                                        case DIKS_3:
                                        case DIKS_4:
                                        case DIKS_5:
                                        case DIKS_6:
                                        case DIKS_7:
                                        case DIKS_8:
                                        case DIKS_9:
                                             i = event.window.key_symbol - DIKS_1;
                                             if (i < D_ARRAY_SIZE(apps))
                                                  START_STOP( manager.saw, &apps[i] );
                                             break;

                                        default:
                                             break;
                                   }
                                   break;

                              default:
                                   break;
                         }
                         break;

                    case DFEC_USER:
                         switch (event.user.type) {
                              case TVMET_STOP_PID:
                                   manager.saw->Stop( manager.saw, (ulong) event.user.data );
                                   break;

                              default:
                                   D_BUG( "unknown user event type %d", event.user.type );
                                   break;
                         }
                         break;

                    default:
                         break;
               }
          }
     }

     /* Shouldn't reach this. */
     return 0;
}

/******************************************************************************/

static void
InitDirectFB( int *argc, char **argv[] )
{
     DFBResult             ret;
     DFBWindowDescription  desc;
     DFBDisplayLayerConfig config;

     /* Initialize DirectFB including command line parsing. */
     ret = DirectFBInit( argc, argv );
     if (ret)
          DirectFBErrorFatal( "DirectFBInit() failed", ret );

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret)
          DirectFBErrorFatal( "DirectFBCreate() failed", ret );

     /* Get the primary display layer. */
     ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
     if (ret)
          DirectFBErrorFatal( "IDirectFB::GetDisplayLayer() failed", ret );

     /* Get the screen size etc. */
     layer->GetConfiguration( layer, &config );

     /* Fill the window description. */
     desc.flags  = DWDESC_POSX | DWDESC_POSY |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS;
     desc.posx   = 0;
     desc.posy   = 0;
     desc.width  = 1;
     desc.height = 1;
     desc.caps   = DWCAPS_INPUTONLY | DWCAPS_NODECORATION;

     /* Create the window. */
     ret = layer->CreateWindow( layer, &desc, &window );
     if (ret)
          DirectFBErrorFatal( "IDirectFBDisplayLayer::CreateWindow() failed", ret );

     /* Create an event buffer for all keyboard events. */
     ret = window->CreateEventBuffer( window, &events );
     if (ret)
          DirectFBErrorFatal( "IDirectFBWindow::CreateEventBuffer() failed", ret );
}

/**********************************************************************************************************************/

static void
InitFusionDale()
{
#ifdef HAVE_FUSIONDALE
     DFBResult ret;

     /* Initialize FusionDale config etc. */
     ret = FusionDaleInit( NULL, NULL );
     if (ret)
          FusionDaleErrorFatal( "FusionDaleInit", ret );

     /* Create the super interface. */
     ret = FusionDaleCreate( &pDale );
     if (ret)
          FusionDaleErrorFatal( "FusionDaleCreate", ret );

     /* Create the event manager. */
     ret = pDale->GetMessenger( pDale, &pMessenger );
     if (ret)
          FusionDaleErrorFatal( "IFusionDale::CreateMessenger", ret );

     /* Register a new event for tuning notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Application Run", &gAppRunEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );

     /* Register a new event for tuning notifications. */
     ret = pMessenger->RegisterEvent( pMessenger, "Application Kill", &gAppKillEventID );
     if (ret && ret != DFB_BUSY)
          FusionDaleErrorFatal( "IFusionDaleMessenger::RegisterEvent", ret );
#endif
}

/**********************************************************************************************************************/

static void
InitSaWMan( TVManager *tm, int *argc, char ***argv )
{
     DFBResult ret;

     memset( tm, 0, sizeof(TVManager) );

     SaWManInit( argc, argv );

     tm->layouts[tm->num_layouts++] = &mosaicLayout;
     tm->layouts[tm->num_layouts++] = &pipLayout;

     /* Initialize groups. */
     tm->groups[0].stacking = DWSC_LOWER;
     tm->groups[1].stacking = DWSC_MIDDLE;
     tm->groups[2].stacking = DWSC_UPPER;

     /* Create the super interface. */
     ret = SaWManCreate( &tm->saw );
     if (ret)
          DirectFBErrorFatal( "SaWManCreate", ret );

     D_MAGIC_SET( tm, TVManager );

     /* Create a new manager using our callbacks. */
     ret = tm->saw->CreateManager( tm->saw, &tvManagerCallbacks, tm, &tm->manager );
     if (ret)
          DirectFBErrorFatal( "ISaWMan::CreateManager", ret );
}

/**********************************************************************************************************************/

static void
SendStartStopEvent( Application *app, bool start )
{
#ifdef HAVE_FUSIONDALE
     DFBResult  ret;
     void         *data;
     int           len = strlen( app->name ) + 1;

     ret = pMessenger->AllocateData( pMessenger, len, &data );
     if (ret) {
          FusionDaleError( "IFusionDaleMessenger::AllocateData", ret );
          return;
     }

     memcpy( data, app->name, len );

     ret = pMessenger->SendEvent( pMessenger,
                                  start ? gAppRunEventID : gAppKillEventID,
                                  app->pid, data, len );
     if (ret)
          FusionDaleError( "IFusionDaleMessenger::SendEvent", ret );
#endif
}

/**********************************************************************************************************************/

static void
AddApplication( TVManager   *tm,
                Application *app )
{
     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( app != NULL );
     D_ASSERT( app->name != NULL );
     D_ASSERT( app->args[0] != NULL );

     direct_list_append( &tm->applications, &app->link );

     D_MAGIC_SET( app, Application );
}

static Application *
LookupApplication( TVManager  *tm,
                   const char *name )
{
     Application *app;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( name != NULL );

     direct_list_foreach (app, tm->applications) {
          D_MAGIC_ASSERT( app, Application );

          if (!strcmp( app->name, name ))
               return app;
     }

     return NULL;
}

static Application *
LookupApplicationByPID( TVManager *tm,
                        pid_t      pid )
{
     Application *app;

     D_MAGIC_ASSERT( tm, TVManager );

     direct_list_foreach (app, tm->applications) {
          D_MAGIC_ASSERT( app, Application );

          if (app->pid == pid)
               return app;
     }

     return NULL;
}

/**********************************************************************************************************************/

typedef struct {
     SaWManWindow *full;
} MosaicData;

static void
MosaicRelayout( TVManager   *tm,
                LayoutGroup *group,
                void        *layout_data )
{
     int             i;
     int             hcenter;
     int             vcenter;
     ISaWManManager *manager;
     DFBRectangle    bounds[4];
     DFBDimension    size;
     MosaicData     *data = layout_data;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );

     if (!group->num_windows)
          return;

     if (data->full)
          group->toggle_on_switch = true;

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     manager->GetSize( manager, group->stacking, &size );

     hcenter = (size.w / 2) & ~1;
     vcenter = size.h / 2;

     switch (group->num_windows) {
          case 1:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = size.w;
               bounds[0].h = size.h;

               break;

          case 2:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = hcenter;
               bounds[0].h = size.h;

               bounds[1].x = hcenter;
               bounds[1].y = 0;
               bounds[1].w = size.w - hcenter;
               bounds[1].h = size.h;

               break;

          case 3:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = hcenter;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = hcenter;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = hcenter;
               bounds[2].y = 0;
               bounds[2].w = size.w - hcenter;
               bounds[2].h = size.h;

               break;

          case 4:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = hcenter;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = hcenter;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = hcenter;
               bounds[2].y = 0;
               bounds[2].w = size.w - hcenter;
               bounds[2].h = vcenter;

               bounds[3].x = hcenter;
               bounds[3].y = vcenter;
               bounds[3].w = size.w - hcenter;
               bounds[3].h = size.h - vcenter;

               break;

          default:
               D_BUG( "invalid number of windows (%d)", group->num_windows );
               break;
     }

     for (i=0; i<group->num_windows; i++) {
          CoreWindow   *corewindow;
          SaWManWindow *window = group->windows[i];

          D_MAGIC_ASSERT( window, SaWManWindow );

          corewindow = window->window;
          D_ASSERT( corewindow != NULL );

          if (window == data->full) {
               corewindow->config.bounds.x = 0;
               corewindow->config.bounds.y = 0;
               corewindow->config.bounds.w = size.w;
               corewindow->config.bounds.h = size.h;

               /* Reinsert window on top. */
               manager->InsertWindow( manager, window, NULL, DFB_TRUE );

               sawman_update_geometry( window );

               break;
          }

          corewindow->config.bounds = bounds[i];

          sawman_update_geometry( window );
     }

     manager->QueueUpdate( manager, group->stacking, NULL );
}

static void
Mosaic_MakeFullScreen( ISaWManManager *manager,
                       SaWManWindow   *window,
                       LayoutGroup    *group )
{
     DFBDimension  size;
     CoreWindow   *corewindow;

     D_ASSERT( manager != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager->GetSize( manager, group->stacking, &size );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     corewindow->config.bounds.x = 0;
     corewindow->config.bounds.y = 0;
     corewindow->config.bounds.w = size.w;
     corewindow->config.bounds.h = size.h;

     /* Reinsert window on top. */
     manager->InsertWindow( manager, window, NULL, DFB_TRUE );

     sawman_update_geometry( window );

     manager->QueueUpdate( manager, group->stacking, NULL );
}

static void
MosaicAddWindow( TVManager    *tm,
                 LayoutGroup  *group,
                 void         *layout_data,
                 SaWManWindow *window )
{
     ISaWManManager *manager;
     MosaicData     *data = layout_data;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Add window to our own list of managed windows. */
     group->windows[group->num_windows++] = window;


     if (data->full) {
          Mosaic_MakeFullScreen( manager, window, group );

          data->full = window;

          /* Switch focus to new window. */
          manager->SwitchFocus( manager, window );
     }
     else {
          /* Insert window into layout. */
          manager->InsertWindow( manager, window, NULL, DFB_TRUE );

          /* Relayout everything. */
          MosaicRelayout( tm, group, layout_data );
     }
}

static void
MosaicRemoveWindow( TVManager    *tm,
                    LayoutGroup  *group,
                    void         *layout_data,
                    SaWManWindow *window,
                    int           index )
{
     ISaWManManager *manager;
     MosaicData     *data = layout_data;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Remove window from layout. */
     manager->RemoveWindow( manager, window );

     /* Fullscreen window? */
     if (data->full) {
          if (data->full == window) {
               /* Find a new fullscreen window. */
               if (group->num_windows) {
                    window = group->windows[group->num_windows - 1];

                    D_MAGIC_ASSERT( window, SaWManWindow );

                    Mosaic_MakeFullScreen( manager, window, group );

                    data->full = window;

                    /* Switch focus to new fullscreen window. */
                    manager->SwitchFocus( manager, window );
               }
          }
     }
     else {
          /* Relayout everything that's left. */
          MosaicRelayout( tm, group, layout_data );
     }
}

static void
MosaicToggleWindow( TVManager    *tm,
                    LayoutGroup  *group,
                    void         *layout_data,
                    SaWManWindow *window,
                    int           index )
{
     ISaWManManager *manager;
     MosaicData     *data = layout_data;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* No toggle with one window. */
     if (group->num_windows < 2)
          return;

     if (data->full == window) {
          data->full = NULL;

          group->toggle_on_switch = false;

          MosaicRelayout( tm, group, layout_data );
     }
     else {
          Mosaic_MakeFullScreen( manager, window, group );

          data->full = window;

          group->toggle_on_switch = true;
     }
}

static MosaicData mosaic_data[NUM_LAYERS];

static void *mosaic_datas[NUM_LAYERS] =
{ /* FIXME: Use loop to init. */
     &mosaic_data[0],
     &mosaic_data[1],
     &mosaic_data[2],
};

static const Layout mosaicLayout = {
     data:          mosaic_datas,
     Relayout:      MosaicRelayout,
     AddWindow:     MosaicAddWindow,
     RemoveWindow:  MosaicRemoveWindow,
     ToggleWindow:  MosaicToggleWindow
};

/**********************************************************************************************************************/

typedef struct {
     int  last_toggle;
} PipData;

static void
CalcPipBounds( TVManager    *tm,
               DFBRectangle *bounds,
               LayoutGroup  *group )
{
     int             c3;
     int             h3;
     ISaWManManager *manager;
     DFBDimension    size;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( bounds != NULL );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     manager->GetSize( manager, group->stacking, &size );

     c3 = (size.w*2/3) & ~1;
     h3 = size.h / 3;

     bounds[0].x = 0;
     bounds[0].y = 0;
     bounds[0].w = size.w;
     bounds[0].h = size.h;

     bounds[1].x = c3;
     bounds[1].y = 0;
     bounds[1].w = size.w - c3;
     bounds[1].h = h3;

     bounds[2].x = c3;
     bounds[2].y = h3;
     bounds[2].w = size.w - c3;
     bounds[2].h = h3;

     bounds[3].x = c3;
     bounds[3].y = h3*2;
     bounds[3].w = size.w - c3;
     bounds[3].h = size.h - h3*2;
}

static void
PipRelayout( TVManager   *tm,
             LayoutGroup *group,
             void        *layout_data )
{
     int             i;
     ISaWManManager *manager;
     DFBRectangle    bounds[4];

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );

     if (!group->num_windows)
          return;

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     CalcPipBounds( tm, bounds, group );

     for (i=0; i<group->num_windows; i++) {
          SaWManWindow *window = group->windows[i];
          CoreWindow   *corewindow;

          D_MAGIC_ASSERT( window, SaWManWindow );

          corewindow = window->window;
          D_ASSERT( corewindow != NULL );

          corewindow->config.bounds = bounds[i];

          manager->InsertWindow( manager, window, NULL, DFB_TRUE );

          sawman_update_geometry( window );
     }

     manager->QueueUpdate( manager, group->stacking, NULL );
}

static void
PipAddWindow( TVManager    *tm,
              LayoutGroup  *group,
              void         *layout_data,
              SaWManWindow *window )
{
     ISaWManManager *manager;
     DFBRectangle    bounds[4];
     CoreWindow     *corewindow;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     /* Insert window into layout (on top of last window). */
     manager->InsertWindow( manager, window, 
                            group->num_windows ? group->windows[group->num_windows-1] : NULL, DFB_TRUE );

     /* Add window to our own list of managed windows. */
     group->windows[group->num_windows++] = window;

     /* FIXME: don't recalculate each time */
     CalcPipBounds( tm, bounds, group );

     /* Set the window's bounds according to its index. */
     corewindow->config.bounds = bounds[group->num_windows-1];

     sawman_update_geometry( window );
}

static void
PipRemoveWindow( TVManager    *tm,
                 LayoutGroup  *group,
                 void         *layout_data,
                 SaWManWindow *window,
                 int           index )
{
     ISaWManManager *manager;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Remove window from layout. */
     manager->RemoveWindow( manager, window );

     /* Relayout everything that's left. */
     PipRelayout( tm, group, layout_data );
}

static void
PipToggleWindow( TVManager    *tm,
                 LayoutGroup  *group,
                 void         *layout_data,
                 SaWManWindow *window,
                 int           index )
{
     ISaWManManager *manager;
     SaWManWindow   *tmp;
     PipData        *data = layout_data;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( layout_data != NULL );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Remember last PIP index from side bar. */
     if (index)
          data->last_toggle = index;
     /* Fixup index of last toggle window. */
     else if (data->last_toggle > group->num_windows - 1)
          data->last_toggle = group->num_windows - 1;

     /* No toggle possible? */
     if (data->last_toggle < 1)
          return;

     /* Swap main window and PIP from side bar. */
     tmp = group->windows[0];
     group->windows[0] = group->windows[data->last_toggle];
     group->windows[data->last_toggle] = tmp;

     /* Relayout (and restack) everything according to the new order. */
     PipRelayout( tm, group, layout_data );
}

static PipData pip_data[NUM_LAYERS] =
{ /* FIXME: Use loop to init. */
     { last_toggle: 1 },
     { last_toggle: 1 },
     { last_toggle: 1 }
};

static void *pip_datas[NUM_LAYERS] =
{ /* FIXME: Use loop to init. */
     &pip_data[0],
     &pip_data[1],
     &pip_data[2],
};

static const Layout pipLayout = {
     data:          pip_datas,
     Relayout:      PipRelayout,
     AddWindow:     PipAddWindow,
     RemoveWindow:  PipRemoveWindow,
     ToggleWindow:  PipToggleWindow
};

/**********************************************************************************************************************/

static DFBResult
LayoutWindowAdd( TVManager    *tm,
                 SaWManWindow *window )
{
     int           index;
     LayoutGroup  *group;
     const Layout *layout;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     D_ASSERT( window->priority >= 0 );
     D_ASSERT( window->priority < NUM_LAYERS );

     index  = window->priority;
     group  = &tm->groups[index];

     D_ASSERT( group->current_layout >= 0 );
     D_ASSERT( group->current_layout < tm->num_layouts );

     layout = tm->layouts[group->current_layout];

     D_ASSERT( layout != NULL );
     D_ASSERT( layout->AddWindow != NULL );

     if (window->parent)
          return DFB_OK;

     if (group->num_windows == MAX_WINDOWS) {
          D_WARN( "maximum number (%d) of managed windows exceeded", MAX_WINDOWS );
          return DFB_LIMITEXCEEDED;
     }

     /* Call the layout implementation. */
     layout->AddWindow( tm, group, layout->data[index], window );

     return DFB_OK;
}

static DFBResult
LayoutWindowRemove( TVManager    *tm,
                    SaWManWindow *window )
{
     int           i;
     int           index;
     LayoutGroup  *group;
     const Layout *layout;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     D_ASSERT( window->priority >= 0 );
     D_ASSERT( window->priority < NUM_LAYERS );

     index  = window->priority;
     group  = &tm->groups[index];

     D_ASSERT( group->current_layout >= 0 );
     D_ASSERT( group->current_layout < tm->num_layouts );

     layout = tm->layouts[group->current_layout];

     D_ASSERT( layout != NULL );
     D_ASSERT( layout->RemoveWindow != NULL );

     if (window->parent)
          return DFB_OK;

     for (i=0; i<group->num_windows; i++) {
          D_MAGIC_ASSERT( group->windows[i], SaWManWindow );

          if (group->windows[i] == window)
               break;
     }

     if (i == MAX_WINDOWS) {
          D_BUG( "could not find window %p", window );
          return DFB_BUG;
     }

     /* Remove window from our own list of managed windows. */
     for (; i<group->num_windows-1; i++)
          group->windows[i] = group->windows[i+1];

     group->windows[i] = NULL;

     group->num_windows--;

     /* Call the layout implementation. */
     layout->RemoveWindow( tm, group, layout->data[index], window, i );

     return DFB_OK;
}

/**********************************************************************************************************************/

static DFBResult
start_request( void       *context,
               const char *name,
               pid_t      *ret_pid )
{
     TVManager   *tm = context;
     Application *app;
     pid_t        pid;

     D_INFO( "SampleAppMan: Start request for '%s'!\n", name );

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( name != NULL );
     D_ASSERT( ret_pid != NULL );

     app = LookupApplication( tm, name );
     if (!app)
          return DFB_ITEMNOTFOUND;

     if (app->started && !waitpid( app->pid, NULL, WNOHANG )) {
          D_DEBUG( "Already running '%s' (%d)!", name, app->pid );
          return DFB_BUSY;
     }

     app->started = true;

     app->start_time = direct_clock_get_millis();

     pid = vfork();

     switch (pid) {
          case -1:
               perror("vfork");
               return DFB_FAILURE;

          case 0:
               setsid();

               execvp( app->args[0], (char**) app->args );
               perror("execvp");
               _exit(0);

          default:
               app->pid = pid;

               SendStartStopEvent( app, true );
               break;
     }

     *ret_pid = pid;

     return DFB_OK;
}

static DFBResult
StopApplication( Application *app )
{
     D_MAGIC_ASSERT( app, Application );

     /* Already died before attaching? */
     if (waitpid( app->pid, NULL, WNOHANG )) {
          app->started = false;
          app->pid     = 0;

          return DFB_OK;
     }

     /* Not attached yet? */
     if (!app->process) {
          D_ERROR( "Application with pid %d did not attach yet!\n", app->pid );
          return DFB_NOCONTEXT;
     }

     SendStartStopEvent( app, false );

     /* FIXME: avoid signals */
     kill( app->pid, 15 );

//     if (app->window)
//          manager->CloseWindow( manager, app->window );

     return DFB_OK;
}

static DFBResult
stop_request( void     *context,
              pid_t     pid,
              FusionID  caller )
{
     TVManager      *tm = context;
     Application    *app;
     ISaWManManager *manager;

     D_INFO( "SampleAppMan: Stop request for pid %d!\n", pid );

     D_MAGIC_ASSERT( tm, TVManager );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     if (pid) {
          app = LookupApplicationByPID( tm, pid );
          if (!app)
               return DFB_ITEMNOTFOUND;

          return StopApplication( app );
     }

     direct_list_foreach (app, tm->applications) {
          SaWManProcess *process;

          D_MAGIC_ASSERT( app, Application );

          process = app->process;
          D_MAGIC_ASSERT( process, SaWManProcess );

          if (process->fusion_id != caller)
               StopApplication( app );
     }

     return DFB_OK;
}

static DFBResult
process_added( void          *context,
               SaWManProcess *process )
{
     TVManager   *tm = context;
     Application *app;

     D_INFO( "SampleAppMan: Process added (%d) [%lu]!\n", process->pid, process->fusion_id );

     D_MAGIC_ASSERT( tm, TVManager );

     app = LookupApplicationByPID( tm, process->pid );
     if (!app)
          return DFB_ITEMNOTFOUND;

     if (app->process) {
          D_BUG( "Already attached '%s' (%d)!", app->name, app->pid );
          return DFB_BUG;
     }

     app->process = process;

     return DFB_OK;
}

static DFBResult
process_removed( void          *context,
                 SaWManProcess *process )
{
     TVManager   *tm = context;
     Application *app;

     D_INFO( "SampleAppMan: Process removed (%d) [%lu]!\n", process->pid, process->fusion_id );

     D_MAGIC_ASSERT( tm, TVManager );

     app = LookupApplicationByPID( tm, process->pid );
     if (!app)
          return DFB_ITEMNOTFOUND;

     if (app->process != process) {
          D_BUG( "Process mismatch %p != %p of '%s' (%d)!", app->process, process, app->name, app->pid );
          return DFB_BUG;
     }

     if (waitpid( app->pid, NULL, 0 ) < 0)
          perror("waitpid");

     app->process = NULL;
     app->started = false;
     app->pid     = 0;

     return DFB_OK;
}

static bool
window_is_visible( SaWManWindow *window )
{
     bool showing = false;

     D_MAGIC_ASSERT( window, SaWManWindow );

     sawman_showing_window( window->sawman, window, &showing );

     return showing;
}

static SaWManWindow *
next_focus( TVManager *tm,
            int        group_index,
            int        focus_index )
{
     int          g = group_index;
     int          f = focus_index;
     LayoutGroup *group;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( group_index >= 0 );
     D_ASSERT( group_index < NUM_LAYERS );

     group = &tm->groups[group_index];

     D_ASSERT( focus_index >= 0 );
     D_ASSERT( focus_index < group->num_windows );

     do {
          if (++f == group->num_windows) {
               f = -1;
               g = (g + 1) % NUM_LAYERS;

               group = &tm->groups[g];
          }
          else {
               D_ASSERT( f >= 0 );
               D_ASSERT( f < group->num_windows );
               D_MAGIC_ASSERT( group->windows[f], SaWManWindow );

               /* Support switching fullscreen windows as well... */
               if ((group->toggle_on_switch && g == group_index) || window_is_visible( group->windows[f] ))
                   return group->windows[f];
          }
     } while (g != group_index || f != focus_index);

     /* Should have returned the already focused window at least. */
     D_BUG( "no more window?" );

     return NULL;
}

static DFBResult
input_filter( void          *context,
              DFBInputEvent *event )
{
     int             i = 0, n;
     TVManager      *tm = context;
     ISaWManManager *manager;

     LayoutGroup    *group = 0;
     int             group_index  = -1;
     int             focus_index  = -1;
     SaWManWindow   *focus_window = NULL;

//     D_INFO( "SampleAppMan: Input filter (%x)!\n", event->type );

     D_MAGIC_ASSERT( tm, TVManager );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Lookup focused window/group. */
     for (n=NUM_LAYERS-1; n>=0 && group_index<0; n--) {
          group = &tm->groups[n];

          for (i=0; i<group->num_windows; i++) {
               SaWManWindow *window = group->windows[i];

               D_MAGIC_ASSERT( window, SaWManWindow );
               D_ASSERT( window->window != NULL );
               D_ASSERT( window->priority == n );

               if (window->window->flags & CWF_FOCUSED) {
                    group_index = n;
                    focus_index = i;
                    focus_window = window;
                    break;
               }
          }
     }

     /* No focus? */
     if (group_index < 0)
          group = NULL;

     /* Handle the key. */
     switch (event->type) {
          case DIET_KEYPRESS:
               switch (event->key_symbol) {
                    /* Switch focus */
                    case DIKS_F9:
                    case DIKS_CUSTOM1:
                         if (group) {
                              focus_window = next_focus( tm, group_index, focus_index );
                              if (!focus_window)
                                   return DFB_BUSY;

                              D_MAGIC_ASSERT( focus_window, SaWManWindow );

                              if (group->toggle_on_switch) {
                                   const Layout *layout = tm->layouts[group->current_layout];

                                   D_ASSERT( layout != NULL );
                                   D_ASSERT( focus_window != NULL );

                                   if (layout->ToggleWindow)
                                        layout->ToggleWindow( tm, group, layout->data[focus_window->priority], focus_window, i );
                              }

                              manager->SwitchFocus( manager, focus_window );
                         }
                         return DFB_BUSY;

                    /* Switch layouts. */
                    case DIKS_F10:
                    case DIKS_CUSTOM3:  /* [1]|[2] */
                         if (group && tm->num_layouts > 1) {
                              const Layout *layout;

                              group->toggle_on_switch = false;

                              if (++group->current_layout == tm->num_layouts)
                                   group->current_layout = 0;

                              layout = tm->layouts[group->current_layout];

                              D_ASSERT( layout != NULL );
                              D_ASSERT( layout->Relayout != NULL );

                              layout->Relayout( tm, group, layout->data[group_index] );
                         }
                         return DFB_BUSY;

                    /* Toggle fullscreen/main. */
                    case DIKS_F11:
                    case DIKS_GREEN:
                         if (group) {
                              const Layout *layout = tm->layouts[group->current_layout];

                              D_ASSERT( layout != NULL );
                              D_ASSERT( focus_window != NULL );

                              if (layout->ToggleWindow)
                                   layout->ToggleWindow( tm, group, layout->data[group_index], focus_window, i );
                         }
                         return DFB_BUSY;

                    /* Enable/disable smooth scaling. */
                    case DIKS_F12:
                    case DIKS_YELLOW:
                         tm->scaling_mode = (tm->scaling_mode == SWMSM_SMOOTH_SW) ? SWMSM_STANDARD : SWMSM_SMOOTH_SW;
                         manager->SetScalingMode( manager, tm->scaling_mode );
                         return DFB_BUSY;

                    case DIKS_RED:
                         if (focus_window) {
                              SaWManProcess *process = focus_window->process;
                              DFBUserEvent   event;

                              D_MAGIC_ASSERT( process, SaWManProcess );

                              event.clazz = DFEC_USER;
                              event.type  = TVMET_STOP_PID;
                              event.data  = (void*) (ulong) process->pid;

                              events->PostEvent( events, DFB_EVENT(&event) );
                         }
                         break;

                    default:
                         break;
               }

          case DIET_KEYRELEASE:
               switch (event->key_symbol) {
                    case DIKS_RED:
                    case DIKS_GREEN:
                    case DIKS_YELLOW:
                    case DIKS_CUSTOM1:
                    case DIKS_CUSTOM3:
                    case DIKS_F9:
                    case DIKS_F10:
                    case DIKS_F11:
                    case DIKS_F12:
                         return DFB_BUSY;

                    default:
                         break;
               }

          default:
               break;
     }

     return DFB_OK;
}

static DFBResult
window_preconfig( void       *context,
                  CoreWindow *window )
{
#if D_DEBUG_ENABLED
     TVManager    *tm = context;
#endif
     SaWManWindow *sawwin;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( window != NULL );
     D_ASSERT( window->window_data != NULL );

     sawwin = window->window_data;

     D_INFO( "SampleAppMan: Window preconfig (%d,%d-%dx%d, parent id %d)!\n",
             DFB_RECTANGLE_VALS( &window->config.bounds ), window->parent_id );

     return DFB_OK;
}

static DFBResult
window_added( void         *context,
              SaWManWindow *window )
{
     DFBResult      ret;
     TVManager     *tm = context;
     CoreWindow    *corewindow;
     SaWManProcess *process;
     Application   *app;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     D_INFO( "SampleAppMan: Window added (%d,%d-%dx%d) [%u] - %d!\n",
             DFB_RECTANGLE_VALS( &corewindow->config.bounds ), window->id, window->priority );

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_NOIMPL;  /* to let sawman insert the window */

     process = window->process;
     D_MAGIC_ASSERT( process, SaWManProcess );

     /* Set application window. */
     app = LookupApplicationByPID( tm, process->pid );
     if (app && !app->window)
          app->window = window;

     if (window->parent)
          return DFB_NOIMPL;  /* to let sawman insert the window */

     /* Already showing window? (reattaching) */
     if (corewindow->config.opacity) {
          /* Activate scaling. */
          corewindow->config.options |= DWOP_SCALE;

          ret = LayoutWindowAdd( tm, window );
          if (ret)
               return ret;
     }

     return DFB_OK;
}

static DFBResult
window_removed( void         *context,
                SaWManWindow *window )
{
     TVManager     *tm = context;
     CoreWindow    *corewindow;
     SaWManProcess *process;
     Application   *app;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     D_INFO( "SampleAppMan: Window removed (%d,%d-%dx%d%s) [%u]!\n",
             DFB_RECTANGLE_VALS( &corewindow->config.bounds ),
             corewindow->config.opacity ? " VISIBLE" : "", window->id );

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_NOIMPL;  /* to let sawman remove the window */

     process = window->process;
     D_MAGIC_ASSERT( process, SaWManProcess );

     /* Still showing window? */
     if (corewindow->config.opacity)
          LayoutWindowRemove( tm, window );

     /* Remove application window. */
     app = LookupApplicationByPID( tm, process->pid );
     if (app && app->window == window)
          app->window = NULL;

     return DFB_OK;
}

static DFBResult
window_config( void         *context,
               SaWManWindow *window )
{
     DFBResult         ret;
     TVManager        *tm = context;
     CoreWindowConfig *current;
     CoreWindowConfig *request;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_OK;

     current = &window->config.current;
     request = &window->config.request;

     if (window->config.flags & CWCF_POSITION) {
          D_INFO( "SampleAppMan: Window config - ignoring position (%d,%d)!\n", request->bounds.x, request->bounds.y );
          window->config.flags &= ~CWCF_POSITION;
     }

     if (window->config.flags & CWCF_SIZE) {
          D_INFO( "SampleAppMan: Window config - ignoring size (%dx%d)!\n", request->bounds.w, request->bounds.h );
          window->config.flags &= ~CWCF_SIZE;
     }

     if (window->config.flags & CWCF_STACKING) {
          D_INFO( "SampleAppMan: Window config - ignoring stacking (%d)!\n", request->stacking );
          window->config.flags &= ~CWCF_STACKING;
     }

     if (window->config.flags & CWCF_OPACITY) {
          /* Show? */
          if (request->opacity && !current->opacity) {
               /* Activate scaling. */
               window->config.flags |= CWCF_OPTIONS;
               request->options     |= DWOP_SCALE;

               ret = LayoutWindowAdd( tm, window );
               if (ret)
                    return ret;
          }
          /* Hide? */
          else if (!request->opacity && current->opacity) {
               LayoutWindowRemove( tm, window );
          }
     }

     return DFB_OK;
}

static DFBResult
window_restack( void         *context,
                SaWManWindow *window )
{
     D_MAGIC_ASSERT( window, SaWManWindow );

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_OK;

     D_INFO( "SampleAppMan: Window restack - refusing!\n" );

     return DFB_ACCESSDENIED;
}

static DFBResult
stack_resized( void         *context,
               DFBDimension *size )
{
     int           i;
     const Layout *layout;
     TVManager    *tm = context;

     D_INFO( "SampleAppMan: Stack resized (%dx%d)!\n", size->w, size->h );

     D_MAGIC_ASSERT( tm, TVManager );

     for (i=0; i<NUM_LAYERS; i++) {
          layout = tm->layouts[tm->groups[i].current_layout];

          D_ASSERT( layout != NULL );
          D_ASSERT( layout->Relayout != NULL );

          layout->Relayout( tm, &tm->groups[i], layout->data[i] );
     }

     return DFB_OK;
}

static DFBResult
switch_focus( void         *context,
              SaWManWindow *window )
{
     D_MAGIC_ASSERT( window, SaWManWindow );

     D_INFO( "SampleAppMan: Switch focus to %p [%u]\n", window, window->id );

     return DFB_OK;
}

static const SaWManCallbacks tvManagerCallbacks = {
     Start:              start_request,
     Stop:               stop_request,
     ProcessAdded:       process_added,
     ProcessRemoved:     process_removed,
     InputFilter:        input_filter,
     WindowPreConfig:    window_preconfig,
     WindowAdded:        window_added,
     WindowRemoved:      window_removed,
     WindowConfig:       window_config,
     WindowRestack:      window_restack,
     StackResized:       stack_resized,
     SwitchFocus:        switch_focus
};
