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

#define NDEBUG 1
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

#include <fusion/hash.h>

#include <directfb.h>
#include <directfb_util.h>

#include <plfapisetup.h> // for starting the platform

#include <core/windows_internal.h>

#ifdef HAVE_FUSIONDALE
#include <fusiondale.h>
#endif

#include <sawman.h>
#include <sawman_manager.h>

#include "util.h"


#define MAX_WINDOWS 8
#define MAX_LAYOUTS 4

#define NUM_LAYERS  3

typedef struct __SampleAppMan_TVManager   TVManager;
typedef struct __SampleAppMan_Layout      Layout;
typedef struct __SampleAppMan_LayoutGroup LayoutGroup;
typedef struct __SampleAppMan_Application Application;
typedef struct __SampleAppMan_Instance    Instance;

/**********************************************************************************************************************/

typedef enum {
     TVMET_STOP_PID
} TVManagerEventType;

typedef enum {
     APP_NO_FLAGS   = 0x00000000,
     APP_ANIMATED   = 0x00000001
} ApplicationFlags;

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
     FusionHash        *instances;
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

     ApplicationFlags   flags;

     const char        *name;
     const char        *args[64];
};

struct __SampleAppMan_Instance {
     int                magic;

     Application       *app;

     long long          start_time;
     pid_t              pid;
     SaWManProcess     *process;
     SaWManWindow      *window;    /* FIXME: support multiple application windows */
};

/**********************************************************************************************************************/

static IDirectFB             *dfb     = NULL;
static IDirectFBDisplayLayer *layer   = NULL;
static IDirectFBWindow       *window  = NULL;
static IDirectFBSurface      *surface = NULL;
static IDirectFBEventBuffer  *events  = NULL;
static IDirectFBFont         *font    = NULL;
static IDirectFBSurface      *menu_bg = NULL;

/**********************************************************************************************************************/

static bool                   gMenuShown  = false;
static bool                   gMenuUpdate = true;

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
          name: "Image Viewer",
          args: {   "/home/slideview",
                    "/home/data/photo1.jpg",
                    "/home/data/photo2.jpg",
                    "/home/data/photo3.jpg",
                    "/home/data/photo4.jpg",
                    NULL
               },
          flags: APP_NO_FLAGS
     },

     {
          name: "Opera Browser",
          args: {   "/go-Opera",
                    NULL
               },
          flags: APP_NO_FLAGS
     },

     { 
          name: "Penguin Demo",
          args: {   "/home/df_andi",
                    NULL
               },
          flags: APP_ANIMATED
     },

     {
          name: "Spinning Icons",
          args: {   "/home/df_neo",
                    NULL
               },
          flags: APP_ANIMATED
     },

     {
          name: "ClanBomber",
          args: {   "/home/clanbomber2",
                    NULL
               },
          flags: APP_ANIMATED
     },

     {
          name: "Geki3",
          args: {   "/home/geki3",
                    NULL
               },
          flags: APP_ANIMATED
     },

     {
          name: "GTK-Demo",
          args: {   "/usr/local/bin/gtk-demo",
                    NULL
               },
          flags: APP_NO_FLAGS
     },

     {
          name: "Burning Screen",
          args: {   "/home/df_fire",
                    NULL
               }
     },

/*     {
          name: "Screen Saver",
          args: {   "/home/sampleSaver",
                    NULL
               }
     },
*/
};

/**********************************************************************************************************************/

static void InitDirectFB  ( int              *argc,
                            char            **argv[] );
static void InitFusionDale( void );
static void InitSaWMan    ( TVManager        *manager,
                            int              *argc,
                            char           ***argv );

static void AddApplication( TVManager        *manager,
                            Application      *app );

static void ToggleMenu    ( void );
static void HideMenu      ( void );
static void UpdateMenu    ( const DFBRegion  *region );

/**********************************************************************************************************************/

#define START_STOP(saw,app)                            \
do {                                                   \
     if ((app)->started)                               \
          (saw)->Stop( saw, (app)->pid );              \
     else                                              \
          (saw)->Start( saw, (app)->name, NULL );      \
} while (0)

/**********************************************************************************************************************/

static DFBResult
input_filter( void          *context,
              DFBInputEvent *event );

int
main( int argc, char *argv[] )
{
     int       i;
     TVManager manager;

     int powState;

     /********************** POWER UP THE PLATFORM ******************************/
     // IMPORTANT: this call is needed to bring the platform out of semi-standby

     plfapisetup_Init(argc,argv);
     plfapisetup_pow_GetTvPower(&powState);
     if( powState != plfapisetup_pow_PowerOn ) {
         printf("Attempting to power up platform...\n");
         plfapisetup_pow_SetTvPower( plfapisetup_pow_PowerOn );
         plfapisetup_pow_GetTvPower( &powState);
         plfapisetup_key_Enable();
         printf("Platform should be powered up. (current powerstate:%d)\n", powState);
      }
      else
         printf("Platform is already powered up.\n");
     /***************************************************************************/

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
                                             if (0) {
                                                  DFBInputEvent event;

                                                  event.clazz  = DFEC_INPUT;
                                                  event.type   = DIET_KEYPRESS;
                                                  event.key_symbol = DIKS_F10;

                                                  input_filter( &manager, &event );
                                                  break;
                                             }
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
                                                  manager.saw->Start( manager.saw, apps[i].name, NULL );
                                             HideMenu();
                                             break;

                                        case DIKS_CUSTOM0:
                                        case DIKS_TEXT:
                                             ToggleMenu();
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
                                   manager.saw->Stop( manager.saw, (pid_t) event.user.data );
                                   break;

                              default:
                                   D_BUG( "unknown user event type %d", event.user.type );
                                   break;
                         }

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
     DFBFontDescription    fdsc;
     DFBWindowDescription  desc;
     DFBDisplayLayerConfig config;
     DFBSurfacePixelFormat format;

     /* Initialize DirectFB including command line parsing. */
     ret = DirectFBInit( argc, argv );
     if (ret)
          DirectFBErrorFatal( "DirectFBInit() failed", ret );

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret)
          DirectFBErrorFatal( "DirectFBCreate() failed", ret );

     /* Fill the font description. */
     fdsc.flags  = DFDESC_HEIGHT;
     fdsc.height = 32;

     /* Load the font. */
     ret = dfb->CreateFont( dfb, DATADIR"/decker.ttf", &fdsc, &font );
     if (ret)
          DirectFBErrorFatal( "IDirectFB::CreateFont() failed", ret );

     /* Get the primary display layer. */
     ret = dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer );
     if (ret)
          DirectFBErrorFatal( "IDirectFB::GetDisplayLayer() failed", ret );

     /* Get the screen size etc. */
     layer->GetConfiguration( layer, &config );

     /* Fill the window description. */
     desc.flags  = DWDESC_POSX | DWDESC_POSY |
                   DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS;
     desc.width  = 380;
     desc.height = 580;
     desc.posx   = ((config.width  - desc.width)  / 2) & ~1;
     desc.posy   = (config.height - desc.height) / 2;
     desc.caps   = DWCAPS_NODECORATION; /* Don't manage ourself :) */

     /* Create the window. */
     ret = layer->CreateWindow( layer, &desc, &window );
     if (ret)
          DirectFBErrorFatal( "IDirectFBDisplayLayer::CreateWindow() failed", ret );

     /* Get the window surface. */
     ret = window->GetSurface( window, &surface );
     if (ret)
          DirectFBErrorFatal( "IDirectFBWindow::GetSurface() failed", ret );

     /* Set the font. */
     surface->SetFont( surface, font );

     /* Create an event buffer for all keyboard events. */
     ret = window->CreateEventBuffer( window, &events );
     if (ret)
          DirectFBErrorFatal( "IDirectFBWindow::CreateEventBuffer() failed", ret );

     /* Move to upper stacking class. */
     window->SetStackingClass( window, DWSC_UPPER );

     window->SetColorKey( window, 0x10, 0xc0, 0x20 );
     window->SetOptions( window, DWOP_COLORKEYING | DWOP_GHOST );

     surface->GetPixelFormat( surface, &format );

     ret = util_load_image( dfb, DATADIR"/menu_bg.png", format, &menu_bg, NULL, NULL, NULL );
     if (ret)
          DirectFBErrorFatal( "Could not not image", ret );
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

     /* Initialize hash table for instances. */
     ret = fusion_hash_create_local( HASH_INT, HASH_PTR, 7, &tm->instances );
     if (ret)
          DirectFBErrorFatal( "Could not create application instance hash table\n", ret );

     /* Initialize available layouts. */
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
ToggleMenu()
{
     /* Hide? */
     if (gMenuShown) {
          gMenuShown = false;

          window->SetOpacity( window, 0 );
     }
     else {
          /* Draw? */
          if (gMenuUpdate) {
               gMenuUpdate = false;

               UpdateMenu( NULL );
          }

          /* Finally show it! */
          window->SetOpacity( window, 0xff );

          gMenuShown = true;
     }
}

static void
HideMenu()
{
     /* Hide? */
     if (gMenuShown) {
          gMenuShown = false;

          window->SetOpacity( window, 0 );
     }
}

static void
UpdateMenu( const DFBRegion *region )
{
     int i;

     surface->SetClip( surface, region );

     surface->Blit( surface, menu_bg, NULL, 0, 0 );

     surface->SetColor( surface, 0xff, 0xff, 0xff, 0xff );

     for (i=0; i<D_ARRAY_SIZE(apps); i++) {
          char buf[4];

          snprintf( buf, sizeof(buf), "%d.", i+1 );

          surface->DrawString( surface, buf, -1, 54, 123 + i * 48, DSTF_TOPRIGHT );

          surface->DrawString( surface, apps[i].name, -1, 78, 123 + i * 48, DSTF_TOPLEFT );
     }

     surface->Flip( surface, region, DSFLIP_NONE );
}

/**********************************************************************************************************************/

static void
SendStartStopEvent( Instance *instance, bool start )
{
#ifdef HAVE_FUSIONDALE
     DFBResult  ret;
     void         *data;
     Application  *app;
     int           len;

     D_MAGIC_ASSERT( instance, Instance );

     app = instance->app;
     D_MAGIC_ASSERT( app, Application );

     len = strlen( app->name ) + 1;

     ret = pMessenger->AllocateData( pMessenger, len, &data );
     if (ret) {
          FusionDaleError( "IFusionDaleMessenger::AllocateData", ret );
          return;
     }

     memcpy( data, app->name, len );

     ret = pMessenger->SendEvent( pMessenger,
                                  start ? gAppRunEventID : gAppKillEventID,
                                  instance->pid, data, len );
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

static Instance *
LookupInstanceByPID( TVManager *tm,
                     pid_t      pid )
{
     Instance *instance;

     D_MAGIC_ASSERT( tm, TVManager );

     instance = fusion_hash_lookup( tm->instances, (void*) pid );

     D_MAGIC_ASSERT_IF( instance, Instance );

     return instance;
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
     int             h3, h4;
     int             hcenter;
     int             vcenter;
     ISaWManManager *manager;
     DFBRectangle    bounds[MAX_WINDOWS];
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

     h4      = (size.w / 4) & ~1;
     h3      = (size.w / 3) & ~1;
     hcenter = (size.w / 2) & ~1;
     vcenter =  size.h / 2;

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

          case 5:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = h3;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = h3;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = h3;
               bounds[2].y = 0;
               bounds[2].w = h3;
               bounds[2].h = vcenter;

               bounds[3].x = h3;
               bounds[3].y = vcenter;
               bounds[3].w = h3;
               bounds[3].h = size.h - vcenter;

               bounds[4].x = h3*2;
               bounds[4].y = 0;
               bounds[4].w = size.w - h3*2;
               bounds[4].h = size.h;

               break;

          case 6:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = h3;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = h3;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = h3;
               bounds[2].y = 0;
               bounds[2].w = h3;
               bounds[2].h = vcenter;

               bounds[3].x = h3;
               bounds[3].y = vcenter;
               bounds[3].w = h3;
               bounds[3].h = size.h - vcenter;

               bounds[4].x = h3*2;
               bounds[4].y = 0;
               bounds[4].w = size.w - h3*2;
               bounds[4].h = vcenter;

               bounds[5].x = h3*2;
               bounds[5].y = vcenter;
               bounds[5].w = size.w - h3*2;
               bounds[5].h = size.h - vcenter;

               break;

          case 7:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = h4;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = h4;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = h4;
               bounds[2].y = 0;
               bounds[2].w = h4;
               bounds[2].h = vcenter;

               bounds[3].x = h4;
               bounds[3].y = vcenter;
               bounds[3].w = h4;
               bounds[3].h = size.h - vcenter;

               bounds[4].x = h4*2;
               bounds[4].y = 0;
               bounds[4].w = h4;
               bounds[4].h = vcenter;

               bounds[5].x = h4*2;
               bounds[5].y = vcenter;
               bounds[5].w = h4;
               bounds[5].h = size.h - vcenter;

               bounds[6].x = h4*3;
               bounds[6].y = 0;
               bounds[6].w = size.w - h4*3;
               bounds[6].h = size.h;

               break;

          case 8:
               bounds[0].x = 0;
               bounds[0].y = 0;
               bounds[0].w = h4;
               bounds[0].h = vcenter;

               bounds[1].x = 0;
               bounds[1].y = vcenter;
               bounds[1].w = h4;
               bounds[1].h = size.h - vcenter;

               bounds[2].x = h4;
               bounds[2].y = 0;
               bounds[2].w = h4;
               bounds[2].h = vcenter;

               bounds[3].x = h4;
               bounds[3].y = vcenter;
               bounds[3].w = h4;
               bounds[3].h = size.h - vcenter;

               bounds[4].x = h4*2;
               bounds[4].y = 0;
               bounds[4].w = h4;
               bounds[4].h = vcenter;

               bounds[5].x = h4*2;
               bounds[5].y = vcenter;
               bounds[5].w = h4;
               bounds[5].h = size.h - vcenter;

               bounds[6].x = h4*3;
               bounds[6].y = 0;
               bounds[6].w = size.w - h4*3;
               bounds[6].h = vcenter;

               bounds[7].x = h4*3;
               bounds[7].y = vcenter;
               bounds[7].w = size.w - h4*3;
               bounds[7].h = size.h - vcenter;

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
     int             c3, c23;
     int             h3;
     ISaWManManager *manager;
     DFBDimension    size;

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( bounds != NULL );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     manager->GetSize( manager, group->stacking, &size );

     c3  = (size.w/3) & ~1;
     c23 = (size.w*2/3) & ~1;
     h3  = size.h / 3;

     bounds[0].x = 0;
     bounds[0].y = 0;
     bounds[0].w = size.w;
     bounds[0].h = size.h;

     bounds[1].x = c23;
     bounds[1].y = 0;
     bounds[1].w = size.w - c23;
     bounds[1].h = h3;

     bounds[2].x = c23;
     bounds[2].y = h3;
     bounds[2].w = size.w - c23;
     bounds[2].h = h3;

     bounds[3].x = c23;
     bounds[3].y = h3*2;
     bounds[3].w = size.w - c23;
     bounds[3].h = size.h - h3*2;

     bounds[4].x = c3;
     bounds[4].y = 0;
     bounds[4].w = c3;
     bounds[4].h = h3;

     bounds[5].x = c3;
     bounds[5].y = h3;
     bounds[5].w = c3;
     bounds[5].h = h3;

     bounds[6].x = c3;
     bounds[6].y = h3*2;
     bounds[6].w = c3;
     bounds[6].h = size.h - h3*2;

     bounds[7].x = 0;
     bounds[7].y = 0;
     bounds[7].w = c3;
     bounds[7].h = h3;
}

static void
PipRelayout( TVManager   *tm,
             LayoutGroup *group,
             void        *layout_data )
{
     int             i;
     ISaWManManager *manager;
     DFBRectangle    bounds[MAX_WINDOWS];

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
     DFBRectangle    bounds[MAX_WINDOWS];
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

     /* If window is attached, just set bounds to parent's and don't manage further. */
     if (window->parent) {
          DFBRegion       update;
          CoreWindow     *win     = window->window;
          CoreWindow     *parent  = window->parent->window;
          ISaWManManager *manager = tm->manager;

          win->config.bounds = parent->config.bounds;

          sawman_update_geometry( window );

          dfb_region_from_rectangle( &update, &win->config.bounds );

          manager->QueueUpdate( manager, win->config.stacking, &update );

          return DFB_OK;
     }

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
     Instance    *instance;
     pid_t        pid;

     D_INFO( "SampleAppMan: Start request for '%s'!\n", name );

     D_MAGIC_ASSERT( tm, TVManager );
     D_ASSERT( name != NULL );
     D_ASSERT( ret_pid != NULL );

     app = LookupApplication( tm, name );
     if (!app)
          return DFB_ITEMNOTFOUND;

/*
     if (app->started && !waitpid( app->pid, NULL, WNOHANG )) {
          D_DEBUG( "Already running '%s' (%d)!", name, app->pid );
          return DFB_BUSY;
     }

     app->started = true;
*/

     instance = D_CALLOC( 1, sizeof(Instance) );
     if (!instance)
          return D_OOM();

     D_MAGIC_SET( instance, Instance );

     instance->app        = app;
     instance->start_time = direct_clock_get_millis();
     instance->pid        = pid = vfork();

     switch (pid) {
          case -1:
               perror("vfork");
               D_MAGIC_CLEAR( instance );
               D_FREE( instance );
               return DFB_FAILURE;

          case 0:
               setsid();

               execvp( app->args[0], (char**) app->args );
               perror("execvp");
               _exit(0);

          default:
               SendStartStopEvent( instance, true );
               break;
     }

     *ret_pid = pid;

     return DFB_OK;
}

static DFBResult
DeleteInstance( TVManager *tm, Instance *instance )
{
     void *value;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( instance, Instance );

     fusion_hash_remove( tm->instances, (void*) instance->pid, NULL, &value );

     D_MAGIC_CLEAR( instance );

     D_FREE( instance );

     return DFB_OK;
}

static DFBResult
StopInstance( TVManager *tm, Instance *instance )
{
     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( instance, Instance );

     /* Already died before attaching? */
     if (waitpid( instance->pid, NULL, WNOHANG ))
          return DeleteInstance( tm, instance );

     /* Not attached yet? */
     if (!instance->process) {
          D_ERROR( "Application with pid %d did not attach yet!\n", instance->pid );
          return DFB_NOCONTEXT;
     }

     SendStartStopEvent( instance, false );

     /* FIXME: avoid signals */
     kill( instance->pid, 15 );

//     if (instance->window)
//          manager->CloseWindow( manager, instance->window );

     return DFB_OK;
}

typedef struct {
     TVManager *tm;
     FusionID   caller;
} KillallContext;

static bool
killall_instances( FusionHash *hash,
                   void       *key,
                   void       *value,
                   void       *ctx )
{
     SaWManProcess  *process;
     Instance       *instance = value;
     KillallContext *context  = ctx;

     D_MAGIC_ASSERT( instance, Instance );

     process = instance->process;
     if (process) {
          D_MAGIC_ASSERT( process, SaWManProcess );

          if (process->fusion_id != context->caller)
               StopInstance( context->tm, instance );
     }
     else
          StopInstance( context->tm, instance );

     return true;
}

static DFBResult
stop_request( void     *ctx,
              pid_t     pid,
              FusionID  caller )
{
     TVManager      *tm = ctx;
     Instance       *instance;
     ISaWManManager *manager;
     KillallContext  context;

     D_INFO( "SampleAppMan: Stop request for pid %d!\n", pid );

     D_MAGIC_ASSERT( tm, TVManager );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     /* Kill one app? */
     if (pid) {
          instance = LookupInstanceByPID( tm, pid );
          if (!instance) {
               if (kill( pid, 9 )) {
                    if (errno == ESRCH)
                         return DFB_ITEMNOTFOUND;

                    return DFB_FAILURE;
               }
               return DFB_OK;
          }

          return StopInstance( tm, instance );
     }

     /* Kill all (other) apps? */
     context.tm     = tm;
     context.caller = caller;

     fusion_hash_iterate( tm->instances, killall_instances, &context );

     return DFB_OK;
}

static DFBResult
process_added( void          *context,
               SaWManProcess *process )
{
     TVManager   *tm = context;
     Instance    *instance;
     Application *app;

     D_INFO( "SampleAppMan: Process added (%d) [%lu]!\n", process->pid, process->fusion_id );

     D_MAGIC_ASSERT( tm, TVManager );

     instance = LookupInstanceByPID( tm, process->pid );
     if (!instance)
          return DFB_ITEMNOTFOUND;

     D_MAGIC_ASSERT( instance, Instance );

     app = instance->app;
     D_MAGIC_ASSERT( app, Application );

     if (instance->process) {
          D_BUG( "Already attached '%s' (%d)!", app->name, instance->pid );
          return DFB_BUG;
     }

     instance->process = process;

     return DFB_OK;
}

static DFBResult
process_removed( void          *context,
                 SaWManProcess *process )
{
     TVManager   *tm = context;
     Instance    *instance;
     Application *app;

     D_INFO( "SampleAppMan: Process removed (%d) [%lu]!\n", process->pid, process->fusion_id );

     D_MAGIC_ASSERT( tm, TVManager );

     instance = LookupInstanceByPID( tm, process->pid );
     if (!instance)
          return DFB_ITEMNOTFOUND;

     D_MAGIC_ASSERT( instance, Instance );

     app = instance->app;
     D_MAGIC_ASSERT( app, Application );

     if (instance->process != process) {
          D_BUG( "Process mismatch %p != %p of '%s' (%d)!",
                 instance->process, process, app->name, instance->pid );
          return DFB_BUG;
     }

     if (waitpid( instance->pid, NULL, 0 ) < 0)
          perror("waitpid");

     DeleteInstance( tm, instance );

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
     int             i  = 0, n;
     TVManager      *tm = context;
     ISaWManManager *manager;

     LayoutGroup    *group        = NULL;
     int             group_index  = -1;
     int             focus_index  = -1;
     SaWManWindow   *focus_window = NULL;

     D_INFO( "SampleAppMan: Input filter (type %x, symbol %x)!\n", event->type, event->key_symbol );

     D_MAGIC_ASSERT( tm, TVManager );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     if (manager->Lock( manager ))
          return DFB_FUSION;

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
                    case DIKS_EPG:
                         if (group) {
                              focus_window = next_focus( tm, group_index, focus_index );
                              if (!focus_window) {
                                   manager->Unlock( manager );
                                   return DFB_BUSY;
                              }

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
                         manager->Unlock( manager );
                         return DFB_BUSY;

                    /* Switch layouts. */
                    case DIKS_F10:
                    case DIKS_CUSTOM3:  /* [1]|[2] */
                    case DIKS_MODE:
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
                         manager->Unlock( manager );
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
                         manager->Unlock( manager );
                         return DFB_BUSY;

                    /* Enable/disable smooth scaling. */
                    case DIKS_F12:
                    case DIKS_YELLOW:
                         tm->scaling_mode = (tm->scaling_mode == SWMSM_SMOOTH_SW) ? SWMSM_STANDARD : SWMSM_SMOOTH_SW;
                         manager->SetScalingMode( manager, tm->scaling_mode );
                         manager->Unlock( manager );
                         return DFB_BUSY;

                    case DIKS_RED:
                         if (focus_window) {
                              SaWManProcess *process = focus_window->process;
                              DFBUserEvent   event;

                              D_MAGIC_ASSERT( process, SaWManProcess );

                              event.clazz = DFEC_USER;
                              event.type  = TVMET_STOP_PID;
                              event.data  = (void*) process->pid;

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
                         manager->Unlock( manager );
                         return DFB_BUSY;

                    default:
                         break;
               }

          default:
               break;
     }

     manager->Unlock( manager );

     return DFB_OK;
}


static SaWManWindow *
find_window( SaWManWindow **windows,
             int            num_windows,
             DFBWindowID    window_id )
{
     int i;

     D_ASSERT( windows != NULL );
     D_ASSERT( num_windows >= 0 );
     D_ASSUME( window_id != 0 );

     for (i=0; i<num_windows; i++) {
          SaWManWindow *window = windows[i];

          D_MAGIC_ASSERT( window, SaWManWindow );

          if (window->id == window_id)
               return window;

          if (window->children.count)
               return find_window( (SaWManWindow **) window->children.elements,
                                   window->children.count, window_id );
     }

     return NULL;
}

static DFBResult
window_preconfig( void       *context,
                  CoreWindow *window )
{
     TVManager    *tm = context;
     SaWManWindow *sawwin;

     (void) tm;

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
     DFBResult       ret;
     TVManager      *tm = context;
     CoreWindow     *corewindow;
     SaWManProcess  *process;
     Instance       *instance;
     ISaWManManager *manager;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     D_INFO( "SampleAppMan: Window added (%d,%d-%dx%d) [%u] - %d!\n",
             DFB_RECTANGLE_VALS( &corewindow->config.bounds ), window->id, window->priority );

     if (window->parent)
          return DFB_NOIMPL;  /* to let sawman insert the window */

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_NOIMPL;  /* to let sawman insert the window */

     if (manager->Lock( manager ))
          return DFB_FUSION;

     process = window->process;
     D_MAGIC_ASSERT( process, SaWManProcess );

     /* Set application window. */
     instance = LookupInstanceByPID( tm, process->pid );
     if (instance && !instance->window)
          instance->window = window;

     /* Already showing window? (reattaching) */
     if (corewindow->config.opacity) {
          /* Activate scaling. */
          corewindow->config.options |= DWOP_SCALE;

          ret = LayoutWindowAdd( tm, window );
          if (ret) {
               manager->Unlock( manager );
               return ret;
          }
     }

     manager->Unlock( manager );

     return DFB_OK;
}

static DFBResult
window_removed( void         *context,
                SaWManWindow *window )
{
     TVManager      *tm = context;
     CoreWindow     *corewindow;
     SaWManProcess  *process;
     Instance       *instance;
     ISaWManManager *manager;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     corewindow = window->window;
     D_ASSERT( corewindow != NULL );

     D_INFO( "SampleAppMan: Window removed (%d,%d-%dx%d%s) [%u]!\n",
             DFB_RECTANGLE_VALS( &corewindow->config.bounds ),
             corewindow->config.opacity ? " VISIBLE" : "", window->id );

     if (window->parent)
          return DFB_NOIMPL;  /* to let remove insert the window */

     if (manager->Lock( manager ))
          return DFB_FUSION;

     process = window->process;
     D_MAGIC_ASSERT( process, SaWManProcess );

     /* Still showing window? */
     if (corewindow->config.opacity)
          LayoutWindowRemove( tm, window );

     /* Remove application window. */
     instance = LookupInstanceByPID( tm, process->pid );
     if (instance && instance->window == window)
          instance->window = NULL;

     manager->Unlock( manager );

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
     ISaWManManager   *manager;

     D_MAGIC_ASSERT( tm, TVManager );
     D_MAGIC_ASSERT( window, SaWManWindow );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     if (window->caps & DWCAPS_NODECORATION)
          return DFB_OK;

     if (manager->Lock( manager ))
          return DFB_FUSION;

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
               if (ret) {
                    manager->Unlock( manager );
                    return ret;
               }
          }
          /* Hide? */
          else if (!request->opacity && current->opacity) {
               LayoutWindowRemove( tm, window );
          }
     }

     manager->Unlock( manager );

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
     int             i;
     const Layout   *layout;
     TVManager      *tm = context;
     ISaWManManager *manager;

     D_INFO( "SampleAppMan: Stack resized (%dx%d)!\n", size->w, size->h );

     D_MAGIC_ASSERT( tm, TVManager );

     manager = tm->manager;
     D_ASSERT( manager != NULL );

     if (manager->Lock( manager ))
          return DFB_FUSION;

     for (i=0; i<NUM_LAYERS; i++) {
          layout = tm->layouts[tm->groups[i].current_layout];

          D_ASSERT( layout != NULL );
          D_ASSERT( layout->Relayout != NULL );

          layout->Relayout( tm, &tm->groups[i], layout->data[i] );
     }

     manager->Unlock( manager );

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
