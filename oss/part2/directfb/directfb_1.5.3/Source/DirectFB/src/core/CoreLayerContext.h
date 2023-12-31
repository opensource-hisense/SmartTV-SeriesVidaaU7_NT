/*
   (c) Copyright 2001-2011  The world wide DirectFB Open Source Community (directfb.org)
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

#ifndef ___CoreLayerContext__H___
#define ___CoreLayerContext__H___

#include "CoreLayerContext_includes.h"

/**********************************************************************************************************************
 * CoreLayerContext
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreLayerContext_CreateWindow(
                    CoreLayerContext                          *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window);

DFBResult CoreLayerContext_SetConfiguration(
                    CoreLayerContext                          *obj,
                    const DFBDisplayLayerConfig               *config);

DFBResult CoreLayerContext_GetPrimaryRegion(
                    CoreLayerContext                          *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region);


void *CoreLayerContext_Init_Dispatch(
                    CoreDFB              *core,
                    CoreLayerContext     *obj,
                    FusionCall           *call
);

void  CoreLayerContext_Deinit_Dispatch(FusionCall *call);


#ifdef __cplusplus
}


namespace DirectFB {


/*
 * CoreLayerContext Calls
 */
typedef enum {
    CoreLayerContext_CreateWindow = 1,
    CoreLayerContext_SetConfiguration = 2,
    CoreLayerContext_GetPrimaryRegion = 3,
} CoreLayerContextCall;

/*
 * CoreLayerContext_CreateWindow
 */
typedef struct {
    DFBWindowDescription                       description;
    bool                                       parent_set;
    u32                                        parent_id;
    bool                                       toplevel_set;
    u32                                        toplevel_id;
} CoreLayerContextCreateWindow;

typedef struct {
    DFBResult                                  result;
    u32                                        window_id;
} CoreLayerContextCreateWindowReturn;


/*
 * CoreLayerContext_SetConfiguration
 */
typedef struct {
    DFBDisplayLayerConfig                      config;
} CoreLayerContextSetConfiguration;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetConfigurationReturn;


/*
 * CoreLayerContext_GetPrimaryRegion
 */
typedef struct {
    bool                                       create;
} CoreLayerContextGetPrimaryRegion;

typedef struct {
    DFBResult                                  result;
    u32                                        region_id;
} CoreLayerContextGetPrimaryRegionReturn;





class ILayerContext : public Interface
{
public:
    ILayerContext( CoreDFB *core )
        :
        Interface( core )
    {
    }

public:
    virtual DFBResult CreateWindow(
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window
    ) = 0;

    virtual DFBResult SetConfiguration(
                    const DFBDisplayLayerConfig               *config
    ) = 0;

    virtual DFBResult GetPrimaryRegion(
                    bool                                       create,
                    CoreLayerRegion                          **ret_region
    ) = 0;

};



class ILayerContext_Real : public ILayerContext
{
private:
    CoreLayerContext *obj;

public:
    ILayerContext_Real( CoreDFB *core, CoreLayerContext *obj )
        :
        ILayerContext( core ),
        obj( obj )
    {
    }

public:
    virtual DFBResult CreateWindow(
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window
    );

    virtual DFBResult SetConfiguration(
                    const DFBDisplayLayerConfig               *config
    );

    virtual DFBResult GetPrimaryRegion(
                    bool                                       create,
                    CoreLayerRegion                          **ret_region
    );

};



class ILayerContext_Requestor : public ILayerContext
{
private:
    CoreLayerContext *obj;

public:
    ILayerContext_Requestor( CoreDFB *core, CoreLayerContext *obj )
        :
        ILayerContext( core ),
        obj( obj )
    {
    }

public:
    virtual DFBResult CreateWindow(
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window
    );

    virtual DFBResult SetConfiguration(
                    const DFBDisplayLayerConfig               *config
    );

    virtual DFBResult GetPrimaryRegion(
                    bool                                       create,
                    CoreLayerRegion                          **ret_region
    );

};



class CoreLayerContextDispatch
{

public:
    CoreLayerContextDispatch( CoreDFB *core, ILayerContext *real )
        :
        core( core ),
        real( real )
    {
    }


    virtual DFBResult Dispatch( FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length );

private:
    CoreDFB              *core;
    ILayerContext        *real;
};


}

#endif

#endif
