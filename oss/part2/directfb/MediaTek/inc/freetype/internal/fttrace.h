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
/*  fttrace.h                                                              */
/*                                                                         */
/*    Tracing handling (specification only).                               */
/*                                                                         */
/*  Copyright 2002, 2004 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /* definitions of trace levels for FreeType 2 */

  /* the first level must always be `trace_any' */
FT_TRACE_DEF( any )

  /* base components */
FT_TRACE_DEF( calc )      /* calculations            (ftcalc.c)   */
FT_TRACE_DEF( memory )    /* memory manager          (ftobjs.c)   */
FT_TRACE_DEF( stream )    /* stream manager          (ftstream.c) */
FT_TRACE_DEF( io )        /* i/o interface           (ftsystem.c) */
FT_TRACE_DEF( list )      /* list management         (ftlist.c)   */
FT_TRACE_DEF( init )      /* initialization          (ftinit.c)   */
FT_TRACE_DEF( objs )      /* base objects            (ftobjs.c)   */
FT_TRACE_DEF( outline )   /* outline management      (ftoutln.c)  */
FT_TRACE_DEF( glyph )     /* glyph management        (ftglyph.c)  */

FT_TRACE_DEF( raster )    /* monochrome rasterizer   (ftraster.c) */
FT_TRACE_DEF( smooth )    /* anti-aliasing raster    (ftgrays.c)  */
FT_TRACE_DEF( mm )        /* MM interface            (ftmm.c)     */
FT_TRACE_DEF( raccess )   /* resource fork accessor  (ftrfork.c)  */

/* Cache sub-system */
FT_TRACE_DEF( cache )     /* cache sub-system        (ftcache.c, etc.) */

/* SFNT driver components */
FT_TRACE_DEF( sfobjs )    /* SFNT object handler     (sfobjs.c)   */
FT_TRACE_DEF( ttcmap )    /* charmap handler         (ttcmap.c)   */
FT_TRACE_DEF( ttload )    /* basic TrueType tables   (ttload.c)   */
FT_TRACE_DEF( ttpost )    /* PS table processing     (ttpost.c)   */
FT_TRACE_DEF( ttsbit )    /* TrueType sbit handling  (ttsbit.c)   */

/* TrueType driver components */
FT_TRACE_DEF( ttdriver )  /* TT font driver          (ttdriver.c) */
FT_TRACE_DEF( ttgload )   /* TT glyph loader         (ttgload.c)  */
FT_TRACE_DEF( ttinterp )  /* bytecode interpreter    (ttinterp.c) */
FT_TRACE_DEF( ttobjs )    /* TT objects manager      (ttobjs.c)   */
FT_TRACE_DEF( ttpload )   /* TT data/program loader  (ttpload.c)  */
FT_TRACE_DEF( ttgxvar )   /* TrueType GX var handler (ttgxvar.c)  */

/* Type 1 driver components */
FT_TRACE_DEF( t1driver )
FT_TRACE_DEF( t1gload )
FT_TRACE_DEF( t1hint )
FT_TRACE_DEF( t1load )
FT_TRACE_DEF( t1objs )
FT_TRACE_DEF( t1parse )

/* PostScript helper module `psaux' */
FT_TRACE_DEF( t1decode )
FT_TRACE_DEF( psobjs )

/* PostScript hinting module `pshinter' */
FT_TRACE_DEF( pshrec )
FT_TRACE_DEF( pshalgo1 )
FT_TRACE_DEF( pshalgo2 )

/* Type 2 driver components */
FT_TRACE_DEF( cffdriver )
FT_TRACE_DEF( cffgload )
FT_TRACE_DEF( cffload )
FT_TRACE_DEF( cffobjs )
FT_TRACE_DEF( cffparse )

/* Type 42 driver component */
FT_TRACE_DEF( t42 )

/* CID driver components */
FT_TRACE_DEF( cidafm )
FT_TRACE_DEF( ciddriver )
FT_TRACE_DEF( cidgload )
FT_TRACE_DEF( cidload )
FT_TRACE_DEF( cidobjs )
FT_TRACE_DEF( cidparse )

/* Windows fonts component */
FT_TRACE_DEF( winfnt )

/* PCF fonts components */
FT_TRACE_DEF( pcfdriver )
FT_TRACE_DEF( pcfread )

/* BDF fonts component */
FT_TRACE_DEF( bdfdriver )
FT_TRACE_DEF( bdflib )

/* PFR fonts component */
FT_TRACE_DEF( pfr )


/* END */
