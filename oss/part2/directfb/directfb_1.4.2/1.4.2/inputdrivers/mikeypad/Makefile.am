## Makefile.am for DirectFB/inputdrivers/mikeypad

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

mikeypad_LTLIBRARIES = libdirectfb_mikeypad.la

if BUILD_STATIC
mikeypad_DATA = $(mikeypad_LTLIBRARIES:.la=.o)
endif

mikeypaddir = $(MODULEDIR)/inputdrivers

libdirectfb_mikeypad_la_SOURCES =	\
	mikeypad.c

libdirectfb_mikeypad_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)


libdirectfb_mikeypad_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la 

libdirectfb__la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)mikeypad/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make

