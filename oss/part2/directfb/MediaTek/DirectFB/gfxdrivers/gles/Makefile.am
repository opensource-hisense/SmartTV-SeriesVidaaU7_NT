## Makefile.am for DirectFB/gfxcards/mstar_gles2

GL_PATH=$(DEPENDENCY_LIB_ROOT)/include
# need modify, the GL_LIBS PATH is changed.
GL_LIBS=-L$(DEPENDENCY_LIB_ROOT)/lib -lGLESv2 -lEGL

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/systems \
	-I$(GL_PATH)


mstar_gles2_LTLIBRARIES = libdirectfb_mstar_gles2.la

if BUILD_STATIC
mstar_gles2_DATA = $(mstar_gles2_LTLIBRARIES:.la=.o)
endif

mstar_gles2dir = $(MODULEDIR)/gfxdrivers


libdirectfb_mstar_gles2_la_SOURCES =	\
	mstar_gles2.c \
	mstar_gles2.h \
	mstar_gles2_priv.h

libdirectfb_mstar_gles2_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(GL_LIBS)


DEFS+=-DMSOS_TYPE_LINUX

#libdirectfb_mstar_gles2_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(top_builddir)/lib/fusion/libfusion.la

include $(top_srcdir)/rules/libobject.make
