/*----------------------------------------------------------------------------
 * No Warranty :  Except  as  may  be  otherwise  agreed  to  in  writing, no *
 * warranties  of  any  kind,  whether  express or  implied, are given by MTK *
 * with  respect  to  any  Confidential  Information  or any use thereof, and *
 * the  Confidential  Information  is  provided  on  an  "AS IS"  basis.  MTK *
 * hereby  expressly  disclaims  all  such  warranties, including any implied *
 * warranties   of  merchantability ,  non-infringement  and  fitness  for  a *
 * particular purpose and any warranties arising out of course of performance *
 * course  of dealing or usage of trade.  Parties further acknowledge that in *
 * connection  with  the Purpose, Company may, either presently and/or in the *
 * future,   instruct   MTK   to   assist  it  in  the  development  and  the *
 * implementation,  in accordance with Company's designs, of certain software *
 * relating  to  Company's  product(s)  (the  "Services").   Except as may be *
 * otherwise agreed to in writing, no warranties of any kind, whether express *
 * or  implied,  are  given by MTK with respect to the Services provided, and *
 * the  Services  are  provided  on  an  "AS  IS"  basis.   Company   further *
 * acknowledges  that  the  Services  may  contain  errors,  which testing is *
 * important  and  it  is  solely  responsible for fully testing the Services *
 * and/or   derivatives   thereof   before  they  are  used,  sublicensed  or *
 * distributed.   Should  there  be any third party action brought again MTK, *
 * arising  out  of  or  relating  to  the  Services,  Company agree to fully *
 * indemnify and hold MTK harmless.                                           *
 *---------------------------------------------------------------------------*/
/***************************************************************************/
/*                                                                         */
/*  ftserv.h                                                               */
/*                                                                         */
/*    The FreeType services (specification only).                          */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /*  Each module can export one or more `services'.  Each service is      */
  /*  identified by a constant string and modeled by a pointer; the latter */
  /*  generally corresponds to a structure containing function pointers.   */
  /*                                                                       */
  /*  Note that a service's data cannot be a mere function pointer because */
  /*  in C it is possible that function pointers might be implemented      */
  /*  differently than data pointers (e.g. 48 bits instead of 32).         */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTSERV_H__
#define __FTSERV_H__


FT_BEGIN_HEADER


  /*
   * @macro:
   *   FT_FACE_FIND_SERVICE
   *
   * @description:
   *   This macro is used to look up a service from a face's driver module.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   id ::
   *     A string describing the service as defined in the service's
   *     header files (e.g. FT_SERVICE_ID_MULTI_MASTERS which expands to
   *     `multi-masters').  It is automatically prefixed with
   *     `FT_SERVICE_ID_'.
   *
   * @output:
   *   ptr ::
   *     A variable that receives the service pointer.  Will be NULL
   *     if not found.
   */
#define FT_FACE_FIND_SERVICE( face, ptr, id )                               \
  FT_BEGIN_STMNT                                                            \
    FT_Module    module = FT_MODULE( FT_FACE(face)->driver );               \
    /* the strange cast is to allow C++ compilation */                      \
    FT_Pointer*  Pptr   = (FT_Pointer*) &(ptr);                             \
                                                                            \
                                                                            \
    *Pptr = NULL;                                                           \
    if ( module->clazz->get_interface )                                     \
      *Pptr = module->clazz->get_interface( module, FT_SERVICE_ID_ ## id ); \
  FT_END_STMNT


  /*
   * @macro:
   *   FT_FACE_FIND_GLOBAL_SERVICE
   *
   * @description:
   *   This macro is used to look up a service from all modules.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   id ::
   *     A string describing the service as defined in the service's
   *     header files (e.g. FT_SERVICE_ID_MULTI_MASTERS which expands to
   *     `multi-masters').  It is automatically prefixed with
   *     `FT_SERVICE_ID_'.
   *
   * @output:
   *   ptr ::
   *     A variable that receives the service pointer.  Will be NULL
   *     if not found.
   */
#define FT_FACE_FIND_GLOBAL_SERVICE( face, ptr, id )               \
  FT_BEGIN_STMNT                                                   \
    FT_Module    module = FT_MODULE( FT_FACE(face)->driver );      \
    /* the strange cast is to allow C++ compilation */             \
    FT_Pointer*  Pptr   = (FT_Pointer*) &(ptr);                    \
                                                                   \
                                                                   \
    *Pptr = ft_module_get_service( module, FT_SERVICE_ID_ ## id ); \
  FT_END_STMNT


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****         S E R V I C E   D E S C R I P T O R S                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   *  The following structure is used to _describe_ a given service
   *  to the library.  This is useful to build simple static service lists.
   */
  typedef struct  FT_ServiceDescRec_
  {
    const char*  serv_id;     /* service name         */
    const void*  serv_data;   /* service pointer/data */

  } FT_ServiceDescRec;

  typedef const FT_ServiceDescRec*  FT_ServiceDesc;


  /*
   *  Parse a list of FT_ServiceDescRec descriptors and look for
   *  a specific service by ID.  Note that the last element in the
   *  array must be { NULL, NULL }, and that the function should
   *  return NULL if the service isn't available.
   *
   *  This function can be used by modules to implement their
   *  `get_service' method.
   */
  FT_BASE( FT_Pointer )
  ft_service_list_lookup( FT_ServiceDesc  service_descriptors,
                          const char*     service_id );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****             S E R V I C E S   C A C H E                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   *  This structure is used to store a cache for several frequently used
   *  services.  It is the type of `face->internal->services'.  You
   *  should only use FT_FACE_LOOKUP_SERVICE to access it.
   *
   *  All fields should have the type FT_Pointer to relax compilation
   *  dependencies.  We assume the developer isn't completely stupid.
   *
   *  Each field must be named `service_XXXX' where `XXX' corresponds to
   *  the correct FT_SERVICE_ID_XXXX macro.  See the definition of
   *  FT_FACE_LOOKUP_SERVICE below how this is implemented.
   *
   */
  typedef struct  FT_ServiceCacheRec_
  {
    FT_Pointer  service_POSTSCRIPT_FONT_NAME;
    FT_Pointer  service_MULTI_MASTERS;
    FT_Pointer  service_GLYPH_DICT;
    FT_Pointer  service_PFR_METRICS;
    FT_Pointer  service_WINFNT;

  } FT_ServiceCacheRec, *FT_ServiceCache;


  /*
   *  A magic number used within the services cache.
   */
#define FT_SERVICE_UNAVAILABLE  ((FT_Pointer)-2)  /* magic number */


  /*
   * @macro:
   *   FT_FACE_LOOKUP_SERVICE
   *
   * @description:
   *   This macro is used to lookup a service from a face's driver module
   *   using its cache.
   *
   * @input:
   *   face::
   *     The source face handle containing the cache.
   *
   *   field ::
   *     The field name in the cache.
   *
   *   id ::
   *     The service ID.
   *
   * @output:
   *   ptr ::
   *     A variable receiving the service data.  NULL if not available.
   */
#define FT_FACE_LOOKUP_SERVICE( face, ptr, id )                  \
  FT_BEGIN_STMNT                                                 \
    /* the strange cast is to allow C++ compilation */           \
    FT_Pointer*  pptr = (FT_Pointer*)&(ptr);                     \
    FT_Pointer   svc;                                            \
                                                                 \
                                                                 \
    svc = FT_FACE(face)->internal->services. service_ ## id ;    \
    if ( svc == FT_SERVICE_UNAVAILABLE )                         \
      svc = NULL;                                                \
    else if ( svc == NULL )                                      \
    {                                                            \
      FT_FACE_FIND_SERVICE( face, svc, id );                     \
                                                                 \
      FT_FACE(face)->internal->services. service_ ## id =        \
        (FT_Pointer)( svc != NULL ? svc                          \
                                  : FT_SERVICE_UNAVAILABLE );    \
    }                                                            \
    *pptr = svc;                                                 \
  FT_END_STMNT


  /*
   *  A macro used to define new service structure types.
   */

#define FT_DEFINE_SERVICE( name )            \
  typedef struct FT_Service_ ## name ## Rec_ \
    FT_Service_ ## name ## Rec ;             \
  typedef struct FT_Service_ ## name ## Rec_ \
    const * FT_Service_ ## name ;            \
  struct FT_Service_ ## name ## Rec_

  /* */

  /*
   *  The header files containing the services.
   */

#define FT_SERVICE_MULTIPLE_MASTERS_H  <freetype/internal/services/svmm.h>
#define FT_SERVICE_POSTSCRIPT_NAME_H   <freetype/internal/services/svpostnm.h>
#define FT_SERVICE_POSTSCRIPT_CMAPS_H  <freetype/internal/services/svpscmap.h>
#define FT_SERVICE_POSTSCRIPT_INFO_H   <freetype/internal/services/svpsinfo.h>
#define FT_SERVICE_GLYPH_DICT_H        <freetype/internal/services/svgldict.h>
#define FT_SERVICE_BDF_H               <freetype/internal/services/svbdf.h>
#define FT_SERVICE_XFREE86_NAME_H      <freetype/internal/services/svxf86nm.h>
#define FT_SERVICE_SFNT_H              <freetype/internal/services/svsfnt.h>
#define FT_SERVICE_PFR_H               <freetype/internal/services/svpfr.h>
#define FT_SERVICE_WINFNT_H            <freetype/internal/services/svwinfnt.h>
#define FT_SERVICE_TT_CMAP_H           <freetype/internal/services/svttcmap.h>

 /* */

FT_END_HEADER

#endif /* __FTSERV_H__ */


/* END */
