
#OSS_OUTPUT
#CROSS_COMPILE
#OSS_LIB_ROOT
#FREETYPE_VERSION
#PNG_VERSION
#ZLIB_VERSION
#example:
# cd $(FREETYPE_OUTPUT)
# mosesq make all install \
#        CROSS_COMPILE=/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64/bin/armv7a-mediatek482_001_neon-linux-gnueabi- \
#		 OSS_LIB_ROOT=/proj/mtk16112/apollo-prod-1901/apollo/oss/library/gnuarm-4.8.2_neon_ca9 \
#		 FREETYPE_VERSION=2.7.1 \
#		 PNG_VERSION=1.2.43 \
#		 ZLIB_VERSION=1.2.3 \
#		 OSS_OUTPUT=/proj/mtk16112/apollo-prod-1901/apollo/oss/library/gnuarm-4.8.2_neon_ca9

FREETYPE_OUTPUT := $(OSS_OUTPUT)/freetype
FREETYPE_SOURCE := $(FREETYPE_OUTPUT)/freetype-$(FREETYPE_VERSION)
FREETYPE_BUILD  := $(FREETYPE_OUTPUT)/freetype-$(FREETYPE_VERSION)-build
FREETYPE_FILE := freetype-$(FREETYPE_VERSION).tar.gz

JOBS ?= $(shell grep processor /proc/cpuinfo | wc -l) 
	
all: freetype_prepare
	if [ ! -d $(FREETYPE_BUILD) ]; then \
	    mkdir -p $(FREETYPE_BUILD)	; \
	fi 
	
	cd $(FREETYPE_SOURCE) && sh autogen.sh
	
	chmod 777 $(FREETYPE_SOURCE)/configure && \
	cd $(FREETYPE_BUILD) && \
	CC="$(CROSS_COMPILE)gcc" CCX=$(CROSS_COMPILE)g++  LD=$(CROSS_COMPILE)ld CFLAGS="-I$(OSS_LIB_ROOT)/png/$(PNG_VERSION)/pre-install/include -I$(OSS_LIB_ROOT)/zlib/$(ZLIB_VERSION)/pre-install/include"  \
	LIBPNG_LIBS="-L$(OSS_LIB_ROOT)/png/$(PNG_VERSION)/pre-install/lib " \
	ZLIB_LIBS="-L$(OSS_LIB_ROOT)/zlib/$(ZLIB_VERSION)/pre-install/lib " \
	$(FREETYPE_SOURCE)/configure  \
		--prefix=$(OSS_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install  \
		--host=arm-linux --without-bzip2 --without-harfbuzz
		
		
		           
	make -j $(JOBS) -C $(FREETYPE_BUILD)
		
install:
	make -C $(FREETYPE_BUILD) install

clean:
	rm -rf $(FREETYPE_OUTPUT)

freetype_prepare:
	if [ ! -d $(FREETYPE_SOURCE) ]; then \
		mkdir -p $(FREETYPE_SOURCE); \
		tar zxf $(FREETYPE_FILE) -C $(FREETYPE_OUTPUT); \
                chmod -R 755 ${FREETYPE_SOURCE} ; \
	fi

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/freetype_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/freetype_INFO.txt

