## Makefile.am for DirectFB/src/core/gfxcards/mstar_sti


INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/systems \
	-I$(srcdir)/../mstar_gles2 \
	-I$(DEPENDENCY_LIB_ROOT)/include/libdrm

mstar_sti_LTLIBRARIES = libdirectfb_mstar_sti.la

if BUILD_STATIC
mstar_sti_DATA = $(mstar_sti_LTLIBRARIES:.la=.o)
endif

mstar_stidir = $(MODULEDIR)/gfxdrivers

libdirectfb_mstar_sti_la_SOURCES =	\
	mstar.c		\
	mstar.h		\
	mstar_layer.c		\
	mstar_layer.h		\
	mstar_screen.c		\
	mstar_screen.h	\
	mstar_types.h

libdirectfb_mstar_sti_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS) \
	-lkms -ldrm -lmi -lmtk-pq_utility_loader -lmtk-pqmaputil -lmtk-gfxcfd



DEFS+=-DMSOS_TYPE_LINUX

libdirectfb_mstar_sti_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(top_builddir)/lib/fusion/libfusion.la

include $(top_srcdir)/rules/libobject.make
