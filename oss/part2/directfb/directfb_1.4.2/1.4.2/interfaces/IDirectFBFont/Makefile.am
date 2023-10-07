## Makefile.am for DirectFB/interfaces/IDirectFBFont

idirectfbfontdir = $(MODULEDIR)/interfaces/IDirectFBFont

if FREETYPE_PROVIDER
FREETYPE_PROVIDER_LTLIB = libidirectfbfont_ft2.la
else
FREETYPE_PROVIDER_LTLIB =
endif

#if MVF_PROVIDER
#MSTAR_MVF_PROVIDER_LTLIB = libidirectfbfont_mvf.la
#else
#MSTAR_MVF_PROVIDER_LTLIB =
#endif

if LINOTYPE_PROVIDER
LINOTYPE_PROVIDER_LTLIB = libidirectfbfont_lino.la
else
LINOTYPE_PROVIDER_LTLIB =
endif

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src 	\
	-DDATADIR=\"${RUNTIME_SYSROOT}@DATADIR@\"

AM_CFLAGS = $(FREETYPE_CFLAGS)

idirectfbfont_LTLIBRARIES = \
	$(FREETYPE_PROVIDER_LTLIB)	\
	$(LINOTYPE_PROVIDER_LTLIB)	\
	libidirectfbfont_dgiff.la

if BUILD_STATIC
idirectfbfont_DATA = $(idirectfbfont_LTLIBRARIES:.la=.o)
endif


#libidirectfbfont_default_la_SOURCES = idirectfbfont_default.c default_font.h

#libidirectfbfont_default_la_LDFLAGS = -avoid-version -module

#libidirectfbfont_default_la_LIBADD = \
#	$(top_builddir)/lib/direct/libdirect.la \
#	$(top_builddir)/src/libdirectfb.la


libidirectfbfont_dgiff_la_SOURCES = idirectfbfont_dgiff.c

libidirectfbfont_dgiff_la_LDFLAGS = -avoid-version -module

libidirectfbfont_dgiff_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la


libidirectfbfont_ft2_la_SOURCES = idirectfbfont_ft2.c

libidirectfbfont_ft2_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(FREETYPE_LIBS)

libidirectfbfont_ft2_la_LDFLAGS = -avoid-version -module

libidirectfbfont_lino_la_SOURCES = idirectfbfont_lino.c

libidirectfbfont_lino_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(LINOTYPE_LIBS)

libidirectfbfont_lino_la_LDFLAGS = -avoid-version -module

#libidirectfbfont_mvf_la_SOURCES = idirectfbfont_mvf.c mstar_mvf_grays.c mstar_mvf_base.c mstar_mvf.h

#libidirectfbfont_mvf_la_LIBADD = \
#	$(top_builddir)/lib/direct/libdirect.la \
#	$(top_builddir)/src/libdirectfb.la

#libidirectfbfont_mvf_la_LDFLAGS = -avoid-version -module


#libidirectfbfont_mstarbmp_la_SOURCES = idirectfbfont_mstarbmp.c mstar_bmpfont.h

#libidirectfbfont_mstarbmp_la_LIBADD = \
#	$(top_builddir)/lib/direct/libdirect.la \
#	$(top_builddir)/src/libdirectfb.la

#libidirectfbfont_mstarbmp_la_LDFLAGS = -avoid-version -module

include $(top_srcdir)/rules/libobject.make
