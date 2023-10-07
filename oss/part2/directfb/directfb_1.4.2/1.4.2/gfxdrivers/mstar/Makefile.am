## Makefile.am for DirectFB/src/core/gfxcards/mstar


INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/systems


mstar_LTLIBRARIES = libdirectfb_mstar.la

if BUILD_STATIC
mstar_DATA = $(mstar_LTLIBRARIES:.la=.o)
endif

mstardir = $(MODULEDIR)/gfxdrivers

libdirectfb_mstar_la_SOURCES =	\
	mstar.c		\
	mstar.h		\
	mstar_layer.c		\
	mstar_layer.h		\
	mstar_screen.c		\
	mstar_screen.h	\
	mstar_types.h

libdirectfb_mstar_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS) \
	$(MSTAR_UTOPIA_LIBS)

DEFS+=-DMSOS_TYPE_LINUX

libdirectfb_mstar_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la

include $(top_srcdir)/rules/libobject.make

