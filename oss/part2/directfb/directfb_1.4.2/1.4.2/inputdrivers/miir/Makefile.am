## Makefile.am for DirectFB/inputdrivers/miir

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

miir_LTLIBRARIES = libdirectfb_miir.la

if BUILD_STATIC
miir_DATA = $(miir_LTLIBRARIES:.la=.o)
endif

miirdir = $(MODULEDIR)/inputdrivers

libdirectfb_miir_la_SOURCES =	\
	miir.c

libdirectfb_miir_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)          \
        -lmi



libdirectfb_miir_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la

libdirectfb__la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)miir/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
