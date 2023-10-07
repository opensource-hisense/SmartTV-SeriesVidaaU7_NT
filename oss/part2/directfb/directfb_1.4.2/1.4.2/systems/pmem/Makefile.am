## Makefile.am for DirectFB/systems/devpmem

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/pmem

internalinclude_HEADERS = \
	pmem.h		\
	surfacemanager.h


systemsdir = $(MODULEDIR)/systems

if BUILD_STATIC
systems_DATA = libdirectfb_pmem.o
endif
systems_LTLIBRARIES = libdirectfb_pmem.la

libdirectfb_pmem_la_LDFLAGS = \
	-avoid-version	\
	-module

libdirectfb_pmem_la_SOURCES = \
	pmem.c		\
	pmem_surface_pool.c	\
	surfacemanager.c \
	pmem_utility.c

libdirectfb_pmem_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
