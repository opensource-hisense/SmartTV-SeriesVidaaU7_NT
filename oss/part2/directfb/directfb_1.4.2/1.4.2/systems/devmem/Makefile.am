## Makefile.am for DirectFB/systems/devmem

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src


internalincludedir = $(INTERNALINCLUDEDIR)/devmem

internalinclude_HEADERS = \
	devmem.h		\
  surfacemanager.h    \
  msos_mma_system.h \
  mtk_secure.h


systemsdir = $(MODULEDIR)/systems

if BUILD_STATIC
systems_DATA = libdirectfb_devmem.o
endif
systems_LTLIBRARIES = libdirectfb_devmem.la


if GFX_MSTAR_G2
libdirectfb_devmem_la_LDFLAGS = \
	-avoid-version	\
	-module
endif

if GFX_MSTAR_MI
libdirectfb_devmem_la_LDFLAGS = \
	-avoid-version	\
	-module \
	-lmi \
	-lteec
endif

if GFX_MSTAR_STI
libdirectfb_devmem_la_LDFLAGS = \
	-avoid-version	\
	-module \
	-lteec
endif



if GFX_MSTAR_G2
libdirectfb_devmem_la_SOURCES = \
	devmem.c		\
	devmem_surface_pool.c	\
	surfacemanager.c        \
	prealloc_surface_invideo_pool.c \
	utopia_memory_backend.c    \
	msos_mma_pool.c
endif

if GFX_MSTAR_MI
libdirectfb_devmem_la_SOURCES = \
	devmem.c		\
	devmem_surface_pool.c	\
	surfacemanager.c        \
	prealloc_surface_invideo_pool.c \
	mi_memory_backend.c    \
	msos_mma_pool.c \
	mtk_secure.c
endif

if GFX_MSTAR_STI
libdirectfb_devmem_la_SOURCES = \
	devmem.c		\
	devmem_surface_pool.c	\
	surfacemanager.c        \
	prealloc_surface_invideo_pool.c \
	sti_memory_backend.c    \
	msos_mma_pool.c \
	mtk_secure.c
endif

libdirectfb_devmem_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
