## Makefile.am for DirectFB/interfaces/IDirectFBImageProvider

idirectfbimageproviderdir = $(MODULEDIR)/interfaces/IDirectFBImageProvider

if GIF_PROVIDER
GIF_PROVIDER_LTLIB = libidirectfbimageprovider_gif.la
else
GIF_PROVIDER_LTLIB =
endif

if JPEG_PROVIDER
JPEG_PROVIDER_LTLIB = libidirectfbimageprovider_jpeg.la
else
JPEG_PROVIDER_LTLIB =
endif

if PNG_PROVIDER
PNG_PROVIDER_LTLIB = libidirectfbimageprovider_png.la
else
PNG_PROVIDER_LTLIB =
endif

if BMP_PROVIDER
BMP_PROVIDER_LTLIB = libidirectfbimageprovider_bmp.la
else
BMP_PROVIDER_LTLIB =
endif

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src

AM_CPPFLAGS = -DDATADIR=\"${RUNTIME_SYSROOT}@DATADIR@\" $(LIBJPEG_CFLAGS) $(LIBPNG_CFLAGS)

idirectfbimageprovider_LTLIBRARIES = \
	libidirectfbimageprovider_dfiff.la	\
	$(GIF_PROVIDER_LTLIB)			\
	$(JPEG_PROVIDER_LTLIB)			\
	$(BMP_PROVIDER_LTLIB)           \
	$(PNG_PROVIDER_LTLIB)

if BUILD_STATIC
idirectfbimageprovider_DATA = $(idirectfbimageprovider_LTLIBRARIES:.la=.o)
endif


libidirectfbimageprovider_png_la_SOURCES = idirectfbimageprovider_png.c

libidirectfbimageprovider_png_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(LIBPNG_LIBS)

if GFX_MSTAR_G2
libidirectfbimageprovider_png_la_LDFLAGS = -avoid-version -module
else
if GFX_MSTAR_MI
libidirectfbimageprovider_png_la_LDFLAGS = -avoid-version -module -lmi
endif
endif

libidirectfbimageprovider_dfiff_la_SOURCES = idirectfbimageprovider_dfiff.c

libidirectfbimageprovider_dfiff_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la

if GFX_MSTAR_G2
libidirectfbimageprovider_dfiff_la_LDFLAGS = -avoid-version -module
else
if GFX_MSTAR_MI
libidirectfbimageprovider_dfiff_la_LDFLAGS = -avoid-version -module -lmi
endif
endif

libidirectfbimageprovider_bmp_la_SOURCES = idirectfbimageprovider_bmp.c

libidirectfbimageprovider_bmp_la_LDFLAGS = -avoid-version -module $(LIBS)

libidirectfbimageprovider_bmp_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(top_builddir)/lib/fusion/libfusion.la


libidirectfbimageprovider_gif_la_SOURCES = idirectfbimageprovider_gif.c

libidirectfbimageprovider_gif_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la

if GFX_MSTAR_G2
libidirectfbimageprovider_gif_la_LDFLAGS = -avoid-version -module
else
if GFX_MSTAR_MI
libidirectfbimageprovider_gif_la_LDFLAGS = -avoid-version -module -lmi
endif
endif

libidirectfbimageprovider_jpeg_la_SOURCES = idirectfbimageprovider_jpeg.c

libidirectfbimageprovider_jpeg_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la \
	$(LIBJPEG)

if GFX_MSTAR_G2
libidirectfbimageprovider_jpeg_la_LDFLAGS = -avoid-version -module
else
if GFX_MSTAR_MI
libidirectfbimageprovider_jpeg_la_LDFLAGS = -avoid-version -module -lmi
endif
endif

include $(top_srcdir)/rules/libobject.make
