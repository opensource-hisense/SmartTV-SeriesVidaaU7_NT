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
#include <time.h>
#include <string.h>
#include <errno.h>

#include <direct/list.h>
#include <direct/thread.h>

#include <directfb.h>

#include <misc/util.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/stream.h>
#include <direct/util.h>

#include <media/idirectfbdatabuffer.h>



static void
IDirectFBDataBuffer_File_Destruct( IDirectFBDataBuffer *thiz )
{
     IDirectFBDataBuffer_File_data *data =
          (IDirectFBDataBuffer_File_data*) thiz->priv;

     direct_stream_destroy( data->stream );

     direct_mutex_deinit( &data->mutex );

     IDirectFBDataBuffer_Destruct( thiz );
}

static DirectResult
IDirectFBDataBuffer_File_Release( IDirectFBDataBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     if (--data->ref == 0)
          IDirectFBDataBuffer_File_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_File_Flush( IDirectFBDataBuffer *thiz )
{
     return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDataBuffer_File_Finish( IDirectFBDataBuffer *thiz )
{
     return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDataBuffer_File_SeekTo( IDirectFBDataBuffer *thiz,
                                 unsigned int         offset )
{
     DFBResult ret;
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     if (!direct_stream_seekable( data->stream ))
          return DFB_UNSUPPORTED;
          
     direct_mutex_lock( &data->mutex );
     ret = direct_stream_seek( data->stream, offset );
     direct_mutex_unlock( &data->mutex );

     return ret;
}

static DFBResult
IDirectFBDataBuffer_File_GetPosition( IDirectFBDataBuffer *thiz,
                                      unsigned int        *offset )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     if (!offset)
          return DFB_INVARG;

     *offset = direct_stream_offset( data->stream );

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_File_GetLength( IDirectFBDataBuffer *thiz,
                                    unsigned int        *length )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     if (!length)
          return DFB_INVARG;

     *length = direct_stream_length( data->stream );

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_File_WaitForData( IDirectFBDataBuffer *thiz,
                                      unsigned int         length )
{
     DFBResult ret;
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     direct_mutex_lock( &data->mutex );          
     ret = direct_stream_wait( data->stream, length, NULL );
     direct_mutex_unlock( &data->mutex );
     
     return ret;
}

static DFBResult
IDirectFBDataBuffer_File_WaitForDataWithTimeout( IDirectFBDataBuffer *thiz,
                                                 unsigned int         length,
                                                 unsigned int         seconds,
                                                 unsigned int         milli_seconds )
{
     DFBResult      ret;
     struct timeval tv;
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     tv.tv_sec  = seconds;
     tv.tv_usec = milli_seconds*1000;

#ifndef WIN32
     while (direct_mutex_trylock( &data->mutex )) {
          struct timespec t = {0}, r = {0};
          
          if (errno != EBUSY)
               return errno2result( errno );

          t.tv_sec  = 0;
          t.tv_nsec = 10000;
          nanosleep( &t, &r );
          
          tv.tv_usec -= (t.tv_nsec - r.tv_nsec + 500) / 1000;
          if (tv.tv_usec < 0) {
               if (tv.tv_sec < 1)
                    return DFB_TIMEOUT;
               
               tv.tv_sec--;
               tv.tv_usec += 999999;
          }
     }
#else
     direct_mutex_lock( &data->mutex );
#endif

     ret = direct_stream_wait( data->stream, length, &tv );
     
     direct_mutex_unlock( &data->mutex );

     return ret;
}

static DFBResult
IDirectFBDataBuffer_File_GetData( IDirectFBDataBuffer *thiz,
                                  unsigned int         length,
                                  void                *data_buffer,
                                  unsigned int        *read_out )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     if (!data_buffer || !length)
          return DFB_INVARG;

     direct_mutex_lock( &data->mutex );
     ret = direct_stream_read( data->stream, length, data_buffer, read_out );
     direct_mutex_unlock( &data->mutex );

     return ret;
}

static DFBResult
IDirectFBDataBuffer_File_PeekData( IDirectFBDataBuffer *thiz,
                                   unsigned int         length,
                                   int                  offset,
                                   void                *data_buffer,
                                   unsigned int        *read_out )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)

     if (!data_buffer || !length)
          return DFB_INVARG;

     direct_mutex_lock( &data->mutex );
     ret = direct_stream_peek( data->stream, length,
                               offset, data_buffer, read_out );
     direct_mutex_unlock( &data->mutex );
     
     return ret;
}

static DFBResult
IDirectFBDataBuffer_File_HasData( IDirectFBDataBuffer *thiz )
{
     struct timeval tv = {0,0};
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_File)
        
     return direct_stream_wait( data->stream, 1, &tv );
}

static DFBResult
IDirectFBDataBuffer_File_PutData( IDirectFBDataBuffer *thiz,
                                  const void          *data_buffer,
                                  unsigned int         length )
{
     return DFB_UNSUPPORTED;
}


/*Wrap IDirectFBDataBuffer_File_xxxx to _IDirectFBDataBuffer_File_xxxx for safe call*/


static DirectResult
_IDirectFBDataBuffer_File_Release( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_Flush( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_Flush(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_Finish( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_Finish(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_SeekTo( IDirectFBDataBuffer *thiz,
                                 unsigned int         offset )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_SeekTo(thiz,offset);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_GetPosition( IDirectFBDataBuffer *thiz,
                                      unsigned int        *offset )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_GetPosition(thiz,offset);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_GetLength( IDirectFBDataBuffer *thiz,
                                    unsigned int        *length )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_GetLength(thiz,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_WaitForData( IDirectFBDataBuffer *thiz,
                                      unsigned int         length )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_WaitForData(thiz,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_WaitForDataWithTimeout( IDirectFBDataBuffer *thiz,
                                                 unsigned int         length,
                                                 unsigned int         seconds,
                                                 unsigned int         milli_seconds )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_WaitForDataWithTimeout(thiz,length,seconds,milli_seconds);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_GetData( IDirectFBDataBuffer *thiz,
                                  unsigned int         length,
                                  void                *data_buffer,
                                  unsigned int        *read_out )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_GetData(thiz,length,data_buffer,read_out);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_PeekData( IDirectFBDataBuffer *thiz,
                                   unsigned int         length,
                                   int                  offset,
                                   void                *data_buffer,
                                   unsigned int        *read_out )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_PeekData(thiz,length,offset,data_buffer,read_out);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_HasData( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_HasData(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_File_PutData( IDirectFBDataBuffer *thiz,
                                  const void          *data_buffer,
                                  unsigned int         length )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_File_PutData(thiz,data_buffer,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}



DFBResult
IDirectFBDataBuffer_File_Construct( IDirectFBDataBuffer *thiz,
                                    const char          *filename,
                                    CoreDFB             *core,
                                    IDirectFB           *idirectfb )
{
     DFBResult ret;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBDataBuffer_File)

     ret = IDirectFBDataBuffer_Construct( thiz, filename, core, idirectfb );
     if (ret)
          return ret;

     ret = direct_stream_create( filename, &data->stream );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     direct_mutex_init( &data->mutex );

     thiz->Release                = _IDirectFBDataBuffer_File_Release;
     thiz->Flush                  = _IDirectFBDataBuffer_File_Flush;
     thiz->Finish                 = _IDirectFBDataBuffer_File_Finish;
     thiz->SeekTo                 = _IDirectFBDataBuffer_File_SeekTo;
     thiz->GetPosition            = _IDirectFBDataBuffer_File_GetPosition;
     thiz->GetLength              = _IDirectFBDataBuffer_File_GetLength;
     thiz->WaitForData            = _IDirectFBDataBuffer_File_WaitForData;
     thiz->WaitForDataWithTimeout = _IDirectFBDataBuffer_File_WaitForDataWithTimeout;
     thiz->GetData                = _IDirectFBDataBuffer_File_GetData;
     thiz->PeekData               = _IDirectFBDataBuffer_File_PeekData;
     thiz->HasData                = _IDirectFBDataBuffer_File_HasData;
     thiz->PutData                = _IDirectFBDataBuffer_File_PutData;

     return DFB_OK;
}

