## Makefile.am for SaWMan/wm/sawman

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_builddir)/src		\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/lib             \
	-I$(top_srcdir)/wm/SaWMan/src \
	-I$(top_srcdir)/wm/SaWMan/include \
	$(DFB_CFLAGS)


wmdir = $(MODULEDIR)/wm

if BUILD_STATIC
wm_DATA = libdirectfbwm_sawman.o
endif

wm_LTLIBRARIES = libdirectfbwm_sawman.la


libdirectfbwm_sawman_la_LDFLAGS = \
	-avoid-version	\
	-module

libdirectfbwm_sawman_la_SOURCES = \
	sawman_wm.c

libdirectfbwm_sawman_la_LIBADD = \
	 $(top_builddir)/lib/direct/libdirect.la \
        $(top_builddir)/src/libdirectfb.la \
        $(top_builddir)/lib/fusion/libfusion.la \
	../../src/libsawman.la	\
	$(DFB_LIBS)


include $(top_srcdir)/rules/libobject.make
