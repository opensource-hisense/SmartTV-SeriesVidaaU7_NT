## Makefile.am for DirectFB/inputdrivers/mstar_networkir

CFLAGS += -Werror

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

mstar_networkir_LTLIBRARIES = libdirectfb_mstar_networkir.la

if BUILD_STATIC
mstar_networkir_DATA = $(mstar_networkir_LTLIBRARIES:.la=.o)
endif

mstar_networkirdir = $(MODULEDIR)/inputdrivers

libdirectfb_mstar_networkir_la_SOURCES =	\
	mstar_networkir.c

libdirectfb_mstar_networkir_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_mstar_networkir_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make

