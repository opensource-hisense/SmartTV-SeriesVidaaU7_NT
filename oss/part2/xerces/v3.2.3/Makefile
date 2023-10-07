XERCES_VERSION := 3.2.3

#------------
#CMD "module load md32/dev/cmake/3.10.1" to use cmake
#or? CMAKE_PATH = /mtkoss/cmake/3.9.3-linux/x86_64/bin
#LOCAL_TOOLCHAIN_VERSION := 5.5.0
#CROSS_COMPILE := /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
LOCAL_TOOLCHAIN_VERSION := 10.2.1
CROSS_COMPILE := /proj/ot904776/casws/2102_xerces_build/merged/tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-

OSS_OUTPUT := /proj/ot904776/casws/2102_xerces_build/merged/oss/source/xerces
OSS_LIB_ROOT := /proj/ot904776/casws/2102_xerces_build/merged/oss/library/gnuarm-$(LOCAL_TOOLCHAIN_VERSION)_neon_ca9
PLATFORM_ARCH := arm-none-linux-gnueabihf
XERCES_OUTPUT := $(OSS_OUTPUT)/v$(XERCES_VERSION)
XERCES_SOURCE := $(XERCES_OUTPUT)
XERCES_BUILD  := $(XERCES_OUTPUT)/build

LD_LIBRARY_PATH :=
JOBS ?= $(shell grep processor /proc/cpuinfo | wc -l)
XERCES_CFLAGS := -std=c++11 -g -fPIC -O2 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now

all: xerces_prepare
	$(info$(CROSS_COMPILE))
	if [ ! -d $(XERCES_BUILD) ]; then \
	    mkdir -p $(XERCES_BUILD)	; \
	fi
	cd $(XERCES_BUILD) && \
	CC="$(CROSS_COMPILE)gcc" CFLAGS="$(XERCES_CFLAGS) $(PLATFORM_CFLAGS)" CXXFLAGS="$(XERCES_CFLAGS)" AR="$(CROSS_COMPILE)ar rus" RANLIB="$(CROSS_COMPILE)ranlib" \
	LD="$(CROSS_COMPILE)ld" NM="$(CROSS_COMPILE)nm" CXX="$(CROSS_COMPILE)g++" $(XERCES_SOURCE)/configure \
		--prefix=$(OSS_LIB_ROOT)/xerces/$(XERCES_VERSION) \
        --build=i686-pc-linux       \
		--host=$(PLATFORM_ARCH)     \
        --enable-shared=yes         \
		--enable-static=no          \
        --disable-network           \
        --disable-sse2              \
        --enable-transcoder-iconv
	make -j $(JOBS) -C $(XERCES_BUILD)

install:
	@echo "$(XERCES_VERSION)"
	@echo "xxxxxxxx"
	@echo "$(OSS_OUTPUT)"
	@echo "$(CROSS_COMPILE)"
	make -C $(XERCES_BUILD)/src install

xerces_prepare:
	@echo "AutoMake...."
	libtoolize
	aclocal
	autoheader
	autoconf
	automake --add-missing

check:
	make -C $(XERCES_BUILD) check

clean:
	make -C $(XERCES_BUILD) clean

distclean:
	make -C $(XERCES_BUILD) distclean
	rm -r $(XERCES_BUILD)/*
	rm aclocal.m4 config.h.in configure Makefile.in autom4te.cache/ -r
