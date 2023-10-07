## Makefile.am for DirectFB/src/core/gfxcards/mstar_mi


INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/systems \
	-I$(srcdir)/../mstar_gles2

mstar_mi_LTLIBRARIES = libdirectfb_mstar_mi.la

if BUILD_STATIC
mstar_mi_DATA = $(mstar_mi_LTLIBRARIES:.la=.o)
endif

mstar_midir = $(MODULEDIR)/gfxdrivers

libdirectfb_mstar_mi_la_SOURCES =	\
	mstar.c		\
	mstar.h		\
	mstar_screen.c \
	mstar_screen.h \
	mstar_types.h  \
	mstar_fbdev.c  \
	mstarFb.h \
	fbdev.h  \
	fb.h  \
	mstar_fbdev_surface_pool.c  \
	surfacemanager.c  \
	surfacemanager.h

libdirectfb_mstar_mi_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS) \
	$(MSTAR_UTOPIA_LIBS) \
        -lmi



DEFS+=-DMSOS_TYPE_LINUX

libdirectfb_mstar_mi_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(top_builddir)/lib/fusion/libfusion.la

include $(top_srcdir)/rules/libobject.make
