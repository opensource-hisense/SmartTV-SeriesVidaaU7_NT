.PHONY: all install clean
THIS_DIR := $(shell pwd)
CC = /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64/bin/armv7a-mediatek482_001_neon-linux-gnueabi-gcc
OSS_LIB_ROOT= ${THIS_DIR}/../../library/gnuarm-4.8.2_neon_ca9
PNG_VERSION=1.2.43
zLIB_VERSION=1.2.3

PKG_CONFIG_PATH=${OSS_LIB_ROOT}/png/${PNG_VERSION}/pre-install/lib/pkgconfig 
png_CFLAGS+=-I${OSS_LIB_ROOT}/png/${PNG_VERSION}/pre-install/include/libpng 
png_CFLAGS+=-I${OSS_LIB_ROOT}/zlib/${zLIB_VERSION}/pre-install/include  
png_LIBS=-L${OSS_LIB_ROOT}/zlib/${zLIB_VERSION}/pre-install/lib -L${OSS_LIB_ROOT}/png/${PNG_VERSION}/pre-install/lib -lpng -lz -lm
 
CFLAGS += ${png_CFLAGS}
CFLAGS += ${png_LIBS}
CFLAGS += -Wall -g -O2
QRENCODE_VER = 3.4.2
OUTPUT = ./output


all:
	@echo "make qrencode" 
	chmod -R 777 ./qrencode-${QRENCODE_VER}
	cd ./qrencode-${QRENCODE_VER} && ./configure CC=${CC} --host=arm
	make -C ./qrencode-3.4.2 CFLAGS="${CFLAGS}"
	
install:
	#make -C ./qrencode-3.4.2 install
	mkdir ${OUTPUT}
	cp -f ./qrencode-3.4.2/qrencode ${THIS_DIR}/${OUTPUT}
	cp -f ./qrencode-3.4.2/.libs/libqrencode* $(THIS_DIR)/${OUTPUT}
	
clean:	
	rm -rf ${OUTPUT}
	make -C ./qrencode-3.4.2 clean

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/qrencode_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/qrencode_INFO.txt

