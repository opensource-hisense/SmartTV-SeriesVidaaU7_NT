## Makefile.am for DirectFB/systems/ion

INCLUDES = \
    -I$(top_srcdir)/include \
    -I$(top_builddir)/include \
    -I$(top_builddir)/lib \
    -I$(top_srcdir)/lib \
    -I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/ion

internalinclude_HEADERS = \
    ion_system.h \
    libion.h \
    mdrv_mtlb_st.h 


systemsdir = $(MODULEDIR)/systems

if BUILD_STATIC
systems_DATA = libdirectfb_ion.o
endif
systems_LTLIBRARIES = libdirectfb_ion.la

libdirectfb_ion_la_LDFLAGS = \
    -avoid-version \
    -module

libdirectfb_ion_la_SOURCES = \
    ion_system.c \
    ion_surface_pool.c \
    prealloc_surface_invideo_pool.c \
    libion.c      
    

libdirectfb_ion_la_LIBADD = \
    $(top_builddir)/lib/direct/libdirect.la \
    $(top_builddir)/lib/fusion/libfusion.la \
    $(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
