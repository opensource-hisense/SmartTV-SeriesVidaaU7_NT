## Makefile.am for DirectFB/inputdrivers/mtnet

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

mtnet_LTLIBRARIES = libdirectfb_mtnet.la

if BUILD_STATIC
mtnet_DATA = $(mtnet_LTLIBRARIES:.la=.o)
endif

mtnetdir = $(MODULEDIR)/inputdrivers

libdirectfb_mtnet_la_SOURCES =	\
	mtnet.c

libdirectfb_mtnet_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_mtnet_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la

libdirectfb__la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)mtnet/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
