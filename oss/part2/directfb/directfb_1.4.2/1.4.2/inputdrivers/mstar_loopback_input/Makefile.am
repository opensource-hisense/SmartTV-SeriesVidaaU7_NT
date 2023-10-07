## Makefile.am for DirectFB/inputdrivers/mstar_loopback_input

CFLAGS += -Werror

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/systems

mstar_loopback_input_LTLIBRARIES = libdirectfb_mstar_loopback_input.la

if BUILD_STATIC
mstar_loopback_input_DATA = $(mstar_loopback_input_LTLIBRARIES:.la=.o)
endif

mstar_loopback_inputdir = $(MODULEDIR)/inputdrivers

libdirectfb_mstar_loopback_input_la_SOURCES = \
	input_fake.h		\
	mstar_loopback_input.c

libdirectfb_mstar_loopback_input_la_LDFLAGS = \
	-module			\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_mstar_loopback_input_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make

