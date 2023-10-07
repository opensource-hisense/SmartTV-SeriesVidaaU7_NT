## Makefile.am for DirectFB/systems/mstartlb

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/mstartlb

internalinclude_HEADERS = \
	mstartlb.h		\
	surfacemanager.h \
        mdrv_mtlb_st.h \
        mdrv_mtlb_interface.h


systemsdir = $(MODULEDIR)/systems

if BUILD_STATIC
systems_DATA = libdirectfb_mstartlb.o
endif
systems_LTLIBRARIES = libdirectfb_mstartlb.la

libdirectfb_mstartlb_la_LDFLAGS = \
	-avoid-version	\
	-module

libdirectfb_mstartlb_la_SOURCES = \
	mstartlb.c		\
	mstartlb_surface_pool.c	\
	surfacemanager.c \
        mstar_mtlb_interface.c        \
	prealloc_surface_invideo_pool.c

libdirectfb_mstartlb_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
