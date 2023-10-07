## Makefile.am for DirectFB/src/core/gfxcards/mstar_g2


INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/systems \
	-I$(srcdir)/../mstar_gles2

mstar_g2_LTLIBRARIES = libdirectfb_mstar_g2.la

if BUILD_STATIC
mstar_g2_DATA = $(mstar_g2_LTLIBRARIES:.la=.o)
endif

mstar_g2dir = $(MODULEDIR)/gfxdrivers

libdirectfb_mstar_g2_la_SOURCES =	\
	mstar.c		\
	mstar.h		\
	mstar_layer.c		\
	mstar_layer.h		\
	mstar_screen.c		\
	mstar_screen.h	\
	mstar_types.h

libdirectfb_mstar_g2_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS) \
	$(MSTAR_UTOPIA_LIBS)

DEFS+=-DMSOS_TYPE_LINUX

libdirectfb_mstar_g2_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(top_builddir)/lib/fusion/libfusion.la

include $(top_srcdir)/rules/libobject.make

