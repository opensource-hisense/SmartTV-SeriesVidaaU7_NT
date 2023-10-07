## Makefile.am for DirectFB/inputdrivers/mstarir

CFLAGS += -Werror

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/include

mstarir_LTLIBRARIES = libdirectfb_mstarir.la

if BUILD_STATIC
mstarir_DATA = $(mstarir_LTLIBRARIES:.la=.o)
endif

mstarirdir = $(MODULEDIR)/inputdrivers

libdirectfb_mstarir_la_SOURCES =	\
	mstarir.c

libdirectfb_mstarir_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_mstarir_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make

