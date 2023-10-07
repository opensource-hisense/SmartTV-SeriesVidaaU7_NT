/*
   (c) Copyright 2006-2007  directfb.org

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>.

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

#include <stdio.h>
#include <unistd.h>

#include <direct/clock.h>
#include <direct/messages.h>

#include <fusiondale.h>


#define CHECK(x)                                  \
     do {                                         \
          DFBResult ret = (x);                    \
          if (ret && ret != DFB_BUSY)             \
               FusionDaleErrorFatal(#x,ret);      \
     } while (0)


static void
TuningCallback( FDMessengerEventID  event_id,
                int                 param,
                void               *data,
                int                 data_size,
                void               *context )
{
//     D_INFO( "EventCallback( %lu, %d, %p, %d, %p )\n",
  //           event_id, param, data, data_size, context );

     printf( "Tuned to PIDs  0x%x  0x%x\n", param, param + 3 );
}

static void
VolumeCallback( FDMessengerEventID  event_id,
                int                 param,
                void               *data,
                int                 data_size,
                void               *context )
{
//     D_INFO( "EventCallback( %lu, %d, %p, %d, %p )\n",
  //           event_id, param, data, data_size, context );

     printf( "Volume changed to %d\n", param );
}

int
main( int argc, char *argv[] )
{
     IFusionDale           *dale;
     IFusionDaleMessenger  *messenger;
     FDMessengerEventID     tuning_id;
     FDMessengerEventID     volume_id;
     FDMessengerListenerID  tuninglistener_id;
     FDMessengerListenerID  volumelistener_id;

     CHECK( FusionDaleInit( &argc, &argv ) );

     CHECK( FusionDaleCreate( &dale ) );

     CHECK( dale->GetMessenger( dale, &messenger ) );

     CHECK( messenger->RegisterEvent( messenger, "Tuning Event", &tuning_id ) );
     CHECK( messenger->RegisterEvent( messenger, "Volume Event", &volume_id ) );

     CHECK( messenger->RegisterListener( messenger, tuning_id, TuningCallback, NULL, &tuninglistener_id ) );
     CHECK( messenger->RegisterListener( messenger, volume_id, VolumeCallback, NULL, &volumelistener_id ) );

     pause();


     CHECK( messenger->UnregisterEvent( messenger, tuning_id ) );
     CHECK( messenger->UnregisterEvent( messenger, volume_id ) );

     messenger->Release( messenger );
     dale->Release( dale );

     return 0;
}
