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
/*  tttags.h                                                               */
/*                                                                         */
/*    Tags for TrueType tables (specification only).                       */
/*                                                                         */
/*  Copyright 1996-2001, 2004 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTAGS_H__
#define __TTAGS_H__


#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


#define TTAG_avar  FT_MAKE_TAG( 'a', 'v', 'a', 'r' )
#define TTAG_bdat  FT_MAKE_TAG( 'b', 'd', 'a', 't' )
#define TTAG_bhed  FT_MAKE_TAG( 'b', 'h', 'e', 'd' )
#define TTAG_bloc  FT_MAKE_TAG( 'b', 'l', 'o', 'c' )
#define TTAG_CFF   FT_MAKE_TAG( 'C', 'F', 'F', ' ' )
#define TTAG_cmap  FT_MAKE_TAG( 'c', 'm', 'a', 'p' )
#define TTAG_cvar  FT_MAKE_TAG( 'c', 'v', 'a', 'r' )
#define TTAG_cvt   FT_MAKE_TAG( 'c', 'v', 't', ' ' )
#define TTAG_DSIG  FT_MAKE_TAG( 'D', 'S', 'I', 'G' )
#define TTAG_EBDT  FT_MAKE_TAG( 'E', 'B', 'D', 'T' )
#define TTAG_EBLC  FT_MAKE_TAG( 'E', 'B', 'L', 'C' )
#define TTAG_EBSC  FT_MAKE_TAG( 'E', 'B', 'S', 'C' )
#define TTAG_fpgm  FT_MAKE_TAG( 'f', 'p', 'g', 'm' )
#define TTAG_fvar  FT_MAKE_TAG( 'f', 'v', 'a', 'r' )
#define TTAG_gasp  FT_MAKE_TAG( 'g', 'a', 's', 'p' )
#define TTAG_glyf  FT_MAKE_TAG( 'g', 'l', 'y', 'f' )
#define TTAG_GSUB  FT_MAKE_TAG( 'G', 'S', 'U', 'B' )
#define TTAG_gvar  FT_MAKE_TAG( 'g', 'v', 'a', 'r' )
#define TTAG_hdmx  FT_MAKE_TAG( 'h', 'd', 'm', 'x' )
#define TTAG_head  FT_MAKE_TAG( 'h', 'e', 'a', 'd' )
#define TTAG_hhea  FT_MAKE_TAG( 'h', 'h', 'e', 'a' )
#define TTAG_hmtx  FT_MAKE_TAG( 'h', 'm', 't', 'x' )
#define TTAG_kern  FT_MAKE_TAG( 'k', 'e', 'r', 'n' )
#define TTAG_loca  FT_MAKE_TAG( 'l', 'o', 'c', 'a' )
#define TTAG_LTSH  FT_MAKE_TAG( 'L', 'T', 'S', 'H' )
#define TTAG_maxp  FT_MAKE_TAG( 'm', 'a', 'x', 'p' )
#define TTAG_MMFX  FT_MAKE_TAG( 'M', 'M', 'F', 'X' )
#define TTAG_MMSD  FT_MAKE_TAG( 'M', 'M', 'S', 'D' )
#define TTAG_name  FT_MAKE_TAG( 'n', 'a', 'm', 'e' )
#define TTAG_OS2   FT_MAKE_TAG( 'O', 'S', '/', '2' )
#define TTAG_OTTO  FT_MAKE_TAG( 'O', 'T', 'T', 'O' )
#define TTAG_PCLT  FT_MAKE_TAG( 'P', 'C', 'L', 'T' )
#define TTAG_post  FT_MAKE_TAG( 'p', 'o', 's', 't' )
#define TTAG_prep  FT_MAKE_TAG( 'p', 'r', 'e', 'p' )
#define TTAG_true  FT_MAKE_TAG( 't', 'r', 'u', 'e' )
#define TTAG_ttc   FT_MAKE_TAG( 't', 't', 'c', ' ' )
#define TTAG_ttcf  FT_MAKE_TAG( 't', 't', 'c', 'f' )
#define TTAG_VDMX  FT_MAKE_TAG( 'V', 'D', 'M', 'X' )
#define TTAG_vhea  FT_MAKE_TAG( 'v', 'h', 'e', 'a' )
#define TTAG_vmtx  FT_MAKE_TAG( 'v', 'm', 't', 'x' )


FT_END_HEADER

#endif /* __TTAGS_H__ */


/* END */
