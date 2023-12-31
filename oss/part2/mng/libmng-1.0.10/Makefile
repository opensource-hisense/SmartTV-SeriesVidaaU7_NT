#
# For conditions of distribution and use, see copyright notice in libmng.h
#
# makefile for libmng - THE MNG library
# this makefile is suitable for Linux ELF with gcc
#
# (this file is heavily copied from makefile.linux in the libpng package)

# compiler
include ../../../../build/oss_version.mk

CROSS_COMPILE ?= arm11_mtk_le-

PATH=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin:$PATH
HOST=arm-linux-gnueabi
CROSS_COMPILE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
CC=${CROSS_COMPILE}gcc
CXX=${CROSS_COMPILE}g++
LD=${CROSS_COMPILE}ld
NM=${CROSS_COMPILE}nm
AR=${CROSS_COMPILE}ar
STRIP=${CROSS_COMPILE}strip
RANLIB=${CROSS_COMPILE}ranlib
AS=${CROSS_COMPILE}as

# default build options (this forces shared library compatibility!!)
#OPTIONS = -DMNG_BUILD_SO
#OPTIONS = -DMNG_BUILD_SO
ZLIBLIB=../../../../oss/library/gnuarm-5.5.0_neon_ca9/zlib/$(ZLIB_VERSION)/pre-install/lib
ZLIBINC=../../../../oss/library/gnuarm-5.5.0_neon_ca9/zlib/$(ZLIB_VERSION)/pre-install/include
JPEGLIB=../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/$(JPEG_VERSION)/pre-install/lib
JPEGINC=../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/$(JPEG_VERSION)/pre-install/include


# Where the lcms library and include files are located
#LCMSLIB=../lcms/lib
#LCMSINC=../lcms/source
LCMSLIB=/usr/local/lib
LCMSINC=/usr/local/include

ALIGN=
# for i386:
#ALIGN=-malign-loops=2 -malign-functions=2

WARNMORE=-Wwrite-strings -Wpointer-arith -Wshadow \
	-Wmissing-declarations -Wtraditional -Wcast-align \
	-Wstrict-prototypes -Wmissing-prototypes #-Wconversion

# for pgcc version 2.95.1, -O3 is buggy; don't use it.

CFLAGS+=-I$(ZLIBINC) -I$(JPEGINC) -I$(LCMSINC) -Wall -g -O0 -fPIC -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -funroll-loops \
	$(OPTIONS) $(ALIGN) # $(WARNMORE) -g
LDFLAGS=-L. -Wl,-rpath,. \
	-L$(ZLIBLIB) -Wl,-rpath,$(ZLIBLIB) \
	-L$(JPEGLIB) -Wl,-rpath,$(JPEGLIB) \
	-L$(LCMSLIB) -Wl,-rpath,$(LCMSLIB) \
	-lmng -lz -ljpeg -lm
# -llcms 

# current version numbers
MNGMAJ = 1
MNGMIN = 1.0.9
MNGVER = $(MNGMAJ).$(MNGMIN)

INCPATH=$(prefix)/include
LIBPATH=$(prefix)/lib

OBJS = \
	libmng_callback_xs.o \
	libmng_chunk_io.o \
	libmng_chunk_descr.o \
	libmng_chunk_prc.o \
	libmng_chunk_xs.o \
	libmng_cms.o \
	libmng_display.o \
	libmng_dither.o \
	libmng_error.o \
	libmng_filter.o \
	libmng_hlapi.o \
	libmng_jpeg.o \
	libmng_object_prc.o \
	libmng_pixels.o \
	libmng_prop_xs.o \
	libmng_read.o \
	libmng_trace.o \
	libmng_write.o \
	libmng_zlib.o

OBJSDLL = $(OBJS:.0=.pic.o)

.SUFFIXES:      .c .o .pic.o

.c.pic.o:
	$(CC) -c $(CFLAGS) -fPIC -o $@ $*.c

all: libmng.a libmng.so

libmng.a: $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

libmng.so: libmng.so.$(MNGMAJ)
	/bin/ln -sf libmng.so.$(MNGMAJ) libmng.so

libmng.so.$(MNGMAJ): libmng.so.$(MNGVER)
	/bin/ln -sf libmng.so.$(MNGVER) libmng.so.$(MNGMAJ)

libmng.so.$(MNGVER): $(OBJSDLL)
#	$(CC) -shared -Wl,-soname,libmng.so.$(MNGMAJ) -o libmng.so.$(MNGVER) \
#	$(OBJSDLL) -L$(ZLIBLIB) -L$(JPEGLIB) -L$(LCMSLIB) -lz -lm -lc
	$(CC) -shared -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -Wl,-soname,libmng.so.$(MNGMAJ) -o libmng.so.$(MNGVER) \
	$(OBJSDLL) -L$(ZLIBLIB) -L$(JPEGLIB) -ljpeg -L$(LCMSLIB) \
    -lz -lm -lc
#-llcms 

install: libmng.a libmng.so.$(MNGVER)
	-@mkdir $(INCPATH) $(LIBPATH)
	cp libmng.h libmng_conf.h libmng_types.h libmng_data.h $(INCPATH)
	chmod 644 $(INCPATH)/libmng.h $(INCPATH)/libmng_conf.h $(INCPATH)/libmng_types.h
	cp libmng.a libmng.so.$(MNGVER) $(LIBPATH)
	chmod 755 $(LIBPATH)/libmng.so.$(MNGVER)
	-@/bin/rm -f $(LIBPATH)/libmng.so.$(MNGMAJ) $(LIBPATH)/libmng.so
	(cd $(LIBPATH); ln -sf libmng.so.$(MNGVER) libmng.so.$(MNGMAJ); \
	 /bin/ln -sf libmng.so.$(MNGMAJ) libmng.so)

clean:
	/bin/rm -f *.o libmng.a libmng.so*

# DO NOT DELETE THIS LINE -- make depend depends on it.

libmng_hlapi.o libmng_hlapi.pic.o: libmng_hlapi.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_objects.h libmng_object_prc.h \
	libmng_chunks.h libmng_memory.h libmng_error.h libmng_trace.h libmng_read.h \
        libmng_write.h libmng_display.h libmng_zlib.h libmng_cms.h libmng_zlib.h
libmng_callback_xs.o libmng_callback_xs.pic.o: libmng_callback_xs.c libmng.h \
        libmng_conf.h libmng_types.h libmng_data.h libmng_error.h libmng_trace.h
libmng_prop_xs.o libmng_prop_xs.pic.o: libmng_prop_xs.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_error.h libmng_trace.h libmng_cms.h
libmng_chunk_xs.o libmng_chunk_xs.pic.o: libmng_chunk_xs.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_chunks.h libmng_chunk_prc.h \
        libmng_error.h libmng_trace.h
libmng_read.o libmng_read.pic.o: libmng_read.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_objects.h libmng_object_prc.h \
	libmng_chunks.h libmng_chunk_prc.h libmng_chunk_io.h libmng_memory.h \
        libmng_error.h libmng_trace.h libmng_read.h libmng_display.h
libmng_write.o libmng_write.pic.o: libmng_write.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_error.h libmng_trace.h libmng_write.h
libmng_display.o libmng_display.pic.o: libmng_display.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_objects.h libmng_object_prc.h libmng_memory.h \
	libmng_error.h libmng_trace.h libmng_zlib.h libmng_cms.h libmng_pixels.h \
        libmng_display.h
libmng_object_prc.o libmng_object_prc.pic.o: libmng_object_prc.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_objects.h libmng_object_prc.h libmng_memory.h \
        libmng_error.h libmng_trace.h libmng_display.h libmng_pixels.h
libmng_chunk_descr.o libmng_chunk_descr.pic.o: libmng_chunk_descr.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_chunks.h libmng_chunk_descr.h libmng_memory.h \
        libmng_chunk_prc.h libmng_error.h libmng_trace.h
libmng_chunk_prc.o libmng_chunk_prc.pic.o: libmng_chunk_prc.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_chunks.h libmng_chunk_prc.h libmng_memory.h \
        libmng_error.h libmng_trace.h
libmng_chunk_io.o libmng_chunk_io.pic.o: libmng_chunk_io.c libmng.h libmng_conf.h \
        libmng_types.h libmng_data.h libmng_objects.h libmng_object_prc.h \
        libmng_chunks.h libmng_chunk_io.h libmng_chunk_prc.h libmng_memory.h libmng_error.h \
        libmng_trace.h libmng_display.h libmng_zlib.h libmng_pixels.h
libmng_error.o libmng_error.pic.o: libmng_error.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_error.h libmng_trace.h
libmng_trace.o libmng_trace.pic.o: libmng_trace.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_error.h libmng_trace.h
libmng_pixels.o libmng_pixels.pic.o: libmng_pixels.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_objects.h libmng_memory.h libmng_error.h libmng_trace.h \
        libmng_cms.h libmng_filter.h libmng_pixels.h
libmng_filter.o libmng_filter.pic.o: libmng_filter.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_error.h libmng_trace.h libmng_filter.h
libmng_dither.o libmng_dither.pic.o: libmng_dither.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_error.h libmng_trace.h libmng_dither.h
libmng_zlib.o libmng_zlib.pic.o: libmng_zlib.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_memory.h libmng_error.h libmng_trace.h libmng_pixels.h \
        libmng_filter.h libmng_zlib.h
libmng_jpeg.o libmng_jpeg.pic.o: libmng_jpeg.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_memory.h libmng_error.h libmng_trace.h libmng_pixels.h libmng_jpeg.h
libmng_cms.o libmng_cms.pic.o: libmng_cms.c libmng.h libmng_conf.h libmng_types.h \
	libmng_data.h libmng_objects.h libmng_error.h libmng_trace.h libmng_cms.h

