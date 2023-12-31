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
/*  ftincrem.h                                                             */
/*                                                                         */
/*    FreeType incremental loading (specification).                        */
/*                                                                         */
/*  Copyright 2002, 2003 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTINCREM_H__
#define __FTINCREM_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


 /***************************************************************************
  *
  * @type:
  *   FT_Incremental
  *
  * @description:
  *   An opaque type describing a user-provided object used to implement
  *   "incremental" glyph loading within FreeType.  This is used to support
  *   embedded fonts in certain environments (e.g. Postscript interpreters),
  *   where the glyph data isn't in the font file, or must be overridden by
  *   different values.
  *
  * @note:
  *   It is up to client applications to create and implement @FT_Incremental
  *   objects, as long as they provide implementations for the methods
  *   @FT_Incremental_GetGlyphDataFunc, @FT_Incremental_FreeGlyphDataFunc
  *   and @FT_Incremental_GetGlyphMetricsFunc.
  *
  *   See the description of @FT_Incremental_InterfaceRec to understand how
  *   to use incremental objects with FreeType.
  */
  typedef struct FT_IncrementalRec_*  FT_Incremental;


 /***************************************************************************
  *
  * @struct:
  *   FT_Incremental_Metrics
  *
  * @description:
  *   A small structure used to contain the basic glyph metrics returned
  *   by the @FT_Incremental_GetGlyphMetricsFunc method.
  *
  * @fields:
  *   bearing_x ::
  *     Left bearing, in font units.
  *
  *   bearing_y ::
  *     Top bearing, in font units.
  *
  *   advance ::
  *     Glyph advance, in font units.
  *
  * @note:
  *   These correspond to horizontal or vertical metrics depending on the
  *   value of the 'vertical' argument to the function
  *   @FT_Incremental_GetGlyphMetricsFunc.
  */
  typedef struct  FT_Incremental_MetricsRec_
  {
    FT_Long  bearing_x;
    FT_Long  bearing_y;
    FT_Long  advance;

  } FT_Incremental_MetricsRec, *FT_Incremental_Metrics;


 /***************************************************************************
  *
  * @type:
  *   FT_Incremental_GetGlyphDataFunc
  *
  * @description:
  *   A function called by FreeType to access a given glyph's data bytes
  *   during @FT_Load_Glyph or @FT_Load_Char if incremental loading is
  *   enabled.
  *
  *   Note that the format of the glyph's data bytes depends on the font
  *   file format.  For TrueType, it must correspond to the raw bytes within
  *   the 'glyf' table.  For Postscript formats, it must correspond to the
  *   *unencrypted* charstring bytes, without any 'lenIV' header.  It is
  *   undefined for any other format.
  *
  * @input:
  *   incremental ::
  *     Handle to an opaque @FT_Incremental handle provided by the client
  *     application.
  *
  *   glyph_index ::
  *     Index of relevant glyph.
  *
  * @output:
  *   adata ::
  *     A structure describing the returned glyph data bytes (which will be
  *     accessed as a read-only byte block).
  *
  * @return:
  *   FreeType error code.  0 means success.
  *
  * @note:
  *   If this function returns succesfully the method
  *   @FT_Incremental_FreeGlyphDataFunc will be called later to release
  *   the data bytes.
  *
  *   Nested calls to @FT_Incremental_GetGlyphDataFunc can happen for
  *   compound glyphs.
  */
  typedef FT_Error
  (*FT_Incremental_GetGlyphDataFunc)( FT_Incremental  incremental,
                                      FT_UInt         glyph_index,
                                      FT_Data*        adata );


 /***************************************************************************
  *
  * @type:
  *   FT_Incremental_FreeGlyphDataFunc
  *
  * @description:
  *   A function used to release the glyph data bytes returned by a
  *   successful call to @FT_Incremental_GetGlyphDataFunc.
  *
  * @input:
  *   incremental ::
  *     A handle to an opaque @FT_Incremental handle provided by the client
  *     application.
  *
  *   data ::
  *     A structure describing the glyph data bytes (which will be accessed
  *     as a read-only byte block).
  */
  typedef void
  (*FT_Incremental_FreeGlyphDataFunc)( FT_Incremental  incremental,
                                       FT_Data*        data );


 /***************************************************************************
  *
  * @type:
  *   FT_Incremental_GetGlyphMetricsFunc
  *
  * @description:
  *   A function used to retrieve the basic metrics of a given glyph index
  *   before accessing its data.  This is necessary because, in certain
  *   formats like TrueType, the metrics are stored in a different place from
  *   the glyph images proper.
  *
  * @input:
  *   incremental ::
  *     A handle to an opaque @FT_Incremental handle provided by the client
  *     application.
  *
  *   glyph_index ::
  *     Index of relevant glyph.
  *
  *   vertical ::
  *     If true, return vertical metrics.
  *
  *   ametrics ::
  *     This parameter is used for both input and output.
  *     The original glyph metrics, if any, in font units.  If metrics are
  *     not available all the values must be set to zero.
  *
  * @output:
  *   ametrics ::
  *     The replacement glyph metrics in font units.
  *
  */
  typedef FT_Error
  (*FT_Incremental_GetGlyphMetricsFunc)
                      ( FT_Incremental              incremental,
                        FT_UInt                     glyph_index,
                        FT_Bool                     vertical,
                        FT_Incremental_MetricsRec  *ametrics );


  /**************************************************************************
   *
   * @struct:
   *   FT_Incremental_FuncsRec
   * 
   * @description:
   *   A table of functions for accessing fonts that load data
   *   incrementally.  Used in @FT_Incremental_InterfaceRec.
   * 
   * @fields:
   *   get_glyph_data ::
   *     The function to get glyph data.  Must not be null.
   *
   *   free_glyph_data ::
   *     The function to release glyph data.  Must not be null.
   *
   *   get_glyph_metrics ::
   *     The function to get glyph metrics.  May be null if the font does
   *     not provide overriding glyph metrics. 
   */
  typedef struct  FT_Incremental_FuncsRec_
  {
    FT_Incremental_GetGlyphDataFunc     get_glyph_data;
    FT_Incremental_FreeGlyphDataFunc    free_glyph_data;
    FT_Incremental_GetGlyphMetricsFunc  get_glyph_metrics;

  } FT_Incremental_FuncsRec;


 /***************************************************************************
  *
  * @struct:
  *   FT_Incremental_InterfaceRec
  *
  * @description:
  *   A structure to be used with @FT_Open_Face to indicate that the user
  *   wants to support incremental glyph loading.  You should use it with
  *   @FT_PARAM_TAG_INCREMENTAL as in the following example:
  *
  *     {
  *       FT_Incremental_InterfaceRec  inc_int;
  *       FT_Parameter                 parameter;
  *       FT_Open_Args                 open_args;
  *
  *
  *       // set up incremental descriptor
  *       inc_int.funcs  = my_funcs;
  *       inc_int.object = my_object;
  *
  *       // set up optional parameter
  *       parameter.tag  = FT_PARAM_TAG_INCREMENTAL;
  *       parameter.data = &inc_int;
  *
  *       // set up FT_Open_Args structure
  *       open_args.flags      = FT_OPEN_PATHNAME | FT_OPEN_PARAMS;
  *       open_args.pathname   = my_font_pathname;
  *       open_args.num_params = 1;
  *       open_args.params     = &parameter; // we use one optional argument
  *
  *       // open the font
  *       error = FT_Open_Face( library, &open_args, index, &face );
  *       ...
  *     }
  */
  typedef struct  FT_Incremental_InterfaceRec_
  {
    const FT_Incremental_FuncsRec*  funcs;
    FT_Incremental                  object;
  
  } FT_Incremental_InterfaceRec;


 /***************************************************************************
  *
  * @constant:
  *   FT_PARAM_TAG_INCREMENTAL
  *
  * @description:
  *   A constant used as the tag of @FT_Parameter structures to indicate
  *   an incremental loading object to be used by FreeType.
  *
  */
#define FT_PARAM_TAG_INCREMENTAL  FT_MAKE_TAG( 'i', 'n', 'c', 'r' )

 /* */  

FT_END_HEADER

#endif /* __FTINCREM_H__ */


/* END */
