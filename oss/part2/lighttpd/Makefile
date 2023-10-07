
# build lighttpd

TOOL_CHAIN:= 5.5.0
VFP_SUFFIX := _neon_ca9

OSS_ROOT ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))
OSS_SOURCE_ROOT := $(OSS_ROOT)/source
OSS_LIB_ROOT := $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)_neon_ca9

include $(OSS_SOURCE_ROOT)/mak/oss_version.mak


LIGHTTPD_VERSION ?= 1.4.35
PCRE_VERSION := 8.33
LOCAL_MODULE_NAME := lighttpd
LOCAL_MODULE_VERSION := $(LIGHTTPD_VERSION)
LOCAL_MODULE_PATH := $(shell pwd)

LOCAL_SOURCE_PATH := $(OSS_ROOT)/source/$(LOCAL_MODULE_NAME)

LOCAL_OUTPUT_PATH := $(OSS_ROOT)/source/$(LOCAL_MODULE_NAME)/output

INSTALL_ROOT := $(OSS_LIB_ROOT)/lighttpd

APOLLO_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))

export CROSS_COMPILE=$(APOLLO_ROOT)/tools/mtk_toolchain/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export NM=${CROSS_COMPILE}nm
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip
export RANLIB=${CROSS_COMPILE}ranlib


MODULE_NAME := lighttpd

OPENSSL_PATH := $(OSS_LIB_ROOT)/openssl/$(OPENSSL_VERSION)


.PHONY: all source_code clean install

all: prepare jansson fastcgi pcre lighttpd



LIGHTTPD_EXTRA_ENV := PCRECONFIG=$(INSTALL_ROOT)/bin/pcre-config
LIGHTTPD_EXTRA_ENV += PCRE_LIB="-L$(INSTALL_ROOT)/lib/ -lpcre"
LIGHTTPD_EXTRA_ENV += INCLUDES=-I$(INSTALL_ROOT)/include
LIGHTTPD_EXTRA_ENV += LDFLAGS=-L$(INSTALL_ROOT)/lib/
LIGHTTPD_EXTRA_ENV += CFLAGS="-DHAVE_LIBPCRE=1 -DHAVE_PCRE_H=1 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie"

prepare:
	if [ ! -d $(INSTALL_ROOT) ];then mkdir -p $(INSTALL_ROOT); fi
#	if [ ! -d $(INSTALL_ROOT)/$(LOCAL_MODULE_VERSION) ];then mkdir -p $(INSTALL_ROOT)/$(LOCAL_MODULE_VERSION); fi
#	if [ ! -d $(INSTALL_ROOT)/$(LOCAL_MODULE_VERSION)/lib ];then mkdir -p $(INSTALL_ROOT)/$(LOCAL_MODULE_VERSION)/lib; fi
#	if [ ! -d $(LOCAL_OUTPUT_PATH) ];then mkdir -p $(LOCAL_OUTPUT_PATH); fi

LOCAL_LIGHTTPD_SOURCE := $(LOCAL_SOURCE_PATH)/lighttpd-${LIGHTTPD_VERSION}
lighttpd:
ifeq ($(LIGHTTPD_VERSION), 1.4.35)
	make -C $(LOCAL_SOURCE_PATH) lighttpd_1_4_35
endif
ifeq ($(LIGHTTPD_VERSION), 1.4.54)
	make -C $(LOCAL_SOURCE_PATH) lighttpd_1_4_54
endif

lighttpd_1_4_35: 
	cd $(LOCAL_SOURCE_PATH); tar -zxf lighttpd-${LIGHTTPD_VERSION}.tar.gz
	cd ${LOCAL_LIGHTTPD_SOURCE}; $(LIGHTTPD_EXTRA_ENV) \
	${LOCAL_LIGHTTPD_SOURCE}/configure --prefix=$(INSTALL_ROOT)/ \
              --host=arm-linux \
              --build=x86_64 \
              --enable-shared \
              --disable-static \
              --disable-lfs \
              --without-zlib \
              --without-bzip2 \
              --with-pcre \
              --with-openssl \
                  --with-openssl-includes=${OPENSSL_PATH}/include \
                  --with-openssl-libs=${OPENSSL_PATH}/lib \
              --disable-ipv6
	cd ${LOCAL_LIGHTTPD_SOURCE};patch -p1 < $(LOCAL_MODULE_PATH)/$(LIGHTTPD_VERSION)/network.c.patch

	make -C ${LOCAL_LIGHTTPD_SOURCE}
	make -C ${LOCAL_LIGHTTPD_SOURCE} install

lighttpd_1_4_54:
	cd $(LOCAL_SOURCE_PATH); tar -zxf lighttpd-${LIGHTTPD_VERSION}.tar.gz
	cd ${LOCAL_LIGHTTPD_SOURCE}; $(LIGHTTPD_EXTRA_ENV) \
	${LOCAL_LIGHTTPD_SOURCE}/configure --prefix=$(INSTALL_ROOT)/ \
              --host=arm-linux \
              --build=x86_64 \
              --enable-shared \
              --disable-static \
              --disable-lfs \
              --without-zlib \
              --without-bzip2 \
              --with-pcre \
              --with-openssl \
                  --with-openssl-includes=${OPENSSL_PATH}/include \
                  --with-openssl-libs=${OPENSSL_PATH}/lib \
              --disable-ipv6
	cp mod_openssl.c ${LOCAL_LIGHTTPD_SOURCE}/src
	cp configfile.c ${LOCAL_LIGHTTPD_SOURCE}/src
	make -C ${LOCAL_LIGHTTPD_SOURCE}
	make -C ${LOCAL_LIGHTTPD_SOURCE} install

LOCAL_FASTCGI_SOURCE := $(LOCAL_SOURCE_PATH)/fcgi-2.4.1-SNAP-0311112127
fastcgi:
	cd $(LOCAL_SOURCE_PATH);tar -zxf fcgi.tar.gz
	cd $(LOCAL_FASTCGI_SOURCE); $(LOCAL_FASTCGI_SOURCE)/configure \
		--prefix=$(INSTALL_ROOT)/ \
        --host=arm-linux \
        --build=x86_64 \
        CFLAGS="-g -O0 -Wall -fPIC -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s" \
        CXXFLAGS="-g -O0 -Wall -fPIC"
	cd $(LOCAL_FASTCGI_SOURCE);patch -p1 < ../fcgio.cpp.patch
	make -C $(LOCAL_FASTCGI_SOURCE)
	make -C $(LOCAL_FASTCGI_SOURCE) install

LOCAL_PCRE_PACKAGE := pcre-$(PCRE_VERSION).tar.gz
LOCAL_PCRE_SOURCE_PATH := $(LOCAL_SOURCE_PATH)/pcre-$(PCRE_VERSION)
pcre:
	cd $(LOCAL_SOURCE_PATH);tar -zxf $(LOCAL_PCRE_PACKAGE)
	cd $(LOCAL_PCRE_SOURCE_PATH); \
	$(LOCAL_PCRE_SOURCE_PATH)/configure --prefix=$(INSTALL_ROOT)/ \
              --host=arm-linux \
              --build=x86_64 \
	      CFLAGS="-fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s"
	make -C $(LOCAL_PCRE_SOURCE_PATH)
	make -C $(LOCAL_PCRE_SOURCE_PATH) install

LOCAL_JANSSON_SOURCE := $(LOCAL_SOURCE_PATH)/jansson-2.7
jansson:
	cd $(LOCAL_SOURCE_PATH);tar -zxf jansson-2.7.tar.gz
	cd $(LOCAL_JANSSON_SOURCE); \
	$(LOCAL_JANSSON_SOURCE)/configure --prefix=$(INSTALL_ROOT)/ \
              --host=arm-linux \
              --build=x86_64 \
	      CFLAGS="-fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s"

	make -C $(LOCAL_JANSSON_SOURCE)
	make -C $(LOCAL_JANSSON_SOURCE) install


clean:
	rm -rf $(LOCAL_JANSSON_SOURCE)
	rm -rf $(LOCAL_PCRE_SOURCE)
	rm -rf $(LOCAL_FASTCGI_SOURCE)
	rm -rf $(LOCAL_LIGHTTPD_SOURCE)
	rm -rf $(LOCAL_OUTPUT_PATH)

install:
	@echo "Already installed in building process"

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt


