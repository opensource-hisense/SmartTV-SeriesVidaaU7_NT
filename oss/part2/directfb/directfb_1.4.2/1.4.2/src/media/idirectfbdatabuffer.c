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

#include <string.h>
#include <errno.h>

#include <direct/interface.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/thread.h>

#include <directfb.h>

#include <misc/util.h>

#include <media/idirectfbdatabuffer.h>
#include <media/idirectfbfont.h>
#include <media/idirectfbimageprovider.h>
#include <media/idirectfbvideoprovider.h>


void
IDirectFBDataBuffer_Destruct( IDirectFBDataBuffer *thiz )
{
     IDirectFBDataBuffer_data *data = (IDirectFBDataBuffer_data*) thiz->priv;

     if (data->filename)
          D_FREE( data->filename );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DirectResult
IDirectFBDataBuffer_AddRef( IDirectFBDataBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBDataBuffer_Release( IDirectFBDataBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     if (--data->ref == 0)
          IDirectFBDataBuffer_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Flush( IDirectFBDataBuffer *thiz )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_Finish( IDirectFBDataBuffer *thiz )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_SeekTo( IDirectFBDataBuffer *thiz,
                            unsigned int         offset )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_GetPosition( IDirectFBDataBuffer *thiz,
                                 unsigned int        *offset )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_GetLength( IDirectFBDataBuffer *thiz,
                               unsigned int        *length )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_WaitForData( IDirectFBDataBuffer *thiz,
                                 unsigned int         length )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_WaitForDataWithTimeout( IDirectFBDataBuffer *thiz,
                                            unsigned int         length,
                                            unsigned int         seconds,
                                            unsigned int         milli_seconds )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_GetData( IDirectFBDataBuffer *thiz,
                             unsigned int         length,
                             void                *data,
                             unsigned int        *read )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_PeekData( IDirectFBDataBuffer *thiz,
                              unsigned int         length,
                              int                  offset,
                              void                *data,
                              unsigned int        *read )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_HasData( IDirectFBDataBuffer *thiz )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_PutData( IDirectFBDataBuffer *thiz,
                             const void          *data,
                             unsigned int         length )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBDataBuffer_CreateImageProvider( IDirectFBDataBuffer     *thiz,
                                         IDirectFBImageProvider **interface_ptr )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     /* Check arguments */
     if (!interface_ptr)
          return DFB_INVARG;

#if !DIRECTFB_BUILD_PURE_VOODOO
     return IDirectFBImageProvider_CreateFromBuffer( thiz, data->core, data->idirectfb, interface_ptr );
#else
     D_BUG( "%s in pure Voodoo build", __FUNCTION__ );
     return DFB_BUG;
#endif
}

static DFBResult
IDirectFBDataBuffer_CreateVideoProvider( IDirectFBDataBuffer     *thiz,
                                         IDirectFBVideoProvider **interface_ptr )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     /* Check arguments */
     if (!interface_ptr)
          return DFB_INVARG;

#if !DIRECTFB_BUILD_PURE_VOODOO
     return IDirectFBVideoProvider_CreateFromBuffer( thiz, data->core, interface_ptr );
#else
     D_BUG( "%s in pure Voodoo build", __FUNCTION__ );
     return DFB_BUG;
#endif
}

static DFBResult
IDirectFBDataBuffer_CreateFont( IDirectFBDataBuffer       *thiz,
                                const DFBFontDescription  *desc,
                                IDirectFBFont            **interface_ptr )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     /* Check arguments */
     if (!interface_ptr || !desc)
          return DFB_INVARG;

#if !DIRECTFB_BUILD_PURE_VOODOO
     return IDirectFBFont_CreateFromBuffer( thiz, data->core, desc, interface_ptr );
#else
     D_BUG( "%s in pure Voodoo build", __FUNCTION__ );
     return DFB_BUG;
#endif
}


/*
Wrap the IDirectFBDataBuffer_xxx to _IDirectFBDataBuffer_xxx for safe call
*/

static DirectResult
_IDirectFBDataBuffer_AddRef( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_AddRef(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DirectResult
_IDirectFBDataBuffer_Release( IDirectFBDataBuffer *thiz )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_Release(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_Flush( IDirectFBDataBuffer *thiz )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_Flush(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_Finish( IDirectFBDataBuffer *thiz )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_Finish(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_SeekTo( IDirectFBDataBuffer *thiz,
                            unsigned int         offset )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_SeekTo(thiz,offset);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_GetPosition( IDirectFBDataBuffer *thiz,
                                 unsigned int        *offset )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_GetPosition(thiz,offset);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_GetLength( IDirectFBDataBuffer *thiz,
                               unsigned int        *length )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_GetLength(thiz,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_WaitForData( IDirectFBDataBuffer *thiz,
                                 unsigned int         length )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_WaitForData(thiz,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_WaitForDataWithTimeout( IDirectFBDataBuffer *thiz,
                                            unsigned int         length,
                                            unsigned int         seconds,
                                            unsigned int         milli_seconds )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_WaitForDataWithTimeout(thiz,length,seconds,milli_seconds);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_GetData( IDirectFBDataBuffer *thiz,
                             unsigned int         length,
                             void                *data,
                             unsigned int        *read )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_GetData(thiz,length,data,read);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_PeekData( IDirectFBDataBuffer *thiz,
                              unsigned int         length,
                              int                  offset,
                              void                *data,
                              unsigned int        *read )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_PeekData(thiz,length,offset,data,read);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_HasData( IDirectFBDataBuffer *thiz )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_HasData(thiz);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_PutData( IDirectFBDataBuffer *thiz,
                             const void          *data,
                             unsigned int         length )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_PutData(thiz,data,length);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_CreateImageProvider( IDirectFBDataBuffer     *thiz,
                                         IDirectFBImageProvider **interface )
{
     DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_CreateImageProvider(thiz,interface);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

static DFBResult
_IDirectFBDataBuffer_CreateVideoProvider( IDirectFBDataBuffer     *thiz,
                                         IDirectFBVideoProvider **interface )
{
    DFBResult ret = DFB_OK;
    DFB_GLOBAL_LOCK(DFB_LOCK_FOR_EXEC);
    ret = IDirectFBDataBuffer_CreateVideoProvider(thiz,interface);
    DFB_GLOBAL_UNLOCK(DFB_LOCK_FOR_EXEC);
    return ret;
}

DFBResult
IDirectFBDataBuffer_Construct( IDirectFBDataBuffer *thiz,
                               const char          *filename,
                               CoreDFB             *core,
                               IDirectFB           *idirectfb )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBDataBuffer)

     data->ref       = 1;
     data->core      = core;
     data->idirectfb = idirectfb;

     if (filename)
          data->filename = D_STRDUP( filename );

     thiz->AddRef                 = _IDirectFBDataBuffer_AddRef;
     thiz->Release                = _IDirectFBDataBuffer_Release;
     thiz->Flush                  = _IDirectFBDataBuffer_Flush;
     thiz->Finish                 = _IDirectFBDataBuffer_Finish;
     thiz->SeekTo                 = _IDirectFBDataBuffer_SeekTo;
     thiz->GetPosition            = _IDirectFBDataBuffer_GetPosition;
     thiz->GetLength              = _IDirectFBDataBuffer_GetLength;
     thiz->WaitForData            = _IDirectFBDataBuffer_WaitForData;
     thiz->WaitForDataWithTimeout = _IDirectFBDataBuffer_WaitForDataWithTimeout;
     thiz->GetData                = _IDirectFBDataBuffer_GetData;
     thiz->PeekData               = _IDirectFBDataBuffer_PeekData;
     thiz->HasData                = _IDirectFBDataBuffer_HasData;
     thiz->PutData                = _IDirectFBDataBuffer_PutData;
     thiz->CreateImageProvider    = _IDirectFBDataBuffer_CreateImageProvider;
     thiz->CreateVideoProvider    = _IDirectFBDataBuffer_CreateVideoProvider;
     thiz->CreateFont             = IDirectFBDataBuffer_CreateFont;
     
     return DFB_OK;
}

