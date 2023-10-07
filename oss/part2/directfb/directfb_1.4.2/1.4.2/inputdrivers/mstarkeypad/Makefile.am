## Makefile.am for DirectFB/inputdrivers/mstarkeypad

CFLAGS += -Werror

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

mstarkeypad_LTLIBRARIES = libdirectfb_mstarkeypad.la

if BUILD_STATIC
mstarkeypad_DATA = $(mstarkeypad_LTLIBRARIES:.la=.o)
endif

mstarkeypaddir = $(MODULEDIR)/inputdrivers

libdirectfb_mstarkeypad_la_SOURCES =	\
	mstarkeypad.c

libdirectfb_mstarkeypad_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_mstarkeypad_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la 

libdirectfb__la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)mstarkeypad/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make

