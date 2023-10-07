.PHONY: all install clean

THIS_DIR := $(shell pwd)

#for test
CC = /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64/bin/armv7a-mediatek482_001_neon-linux-gnueabi-gcc
CROSS_COMPILE = /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64/bin/armv7a-mediatek482_001_neon-linux-gnueabi-
OSS_LIB_ROOT = ${THIS_DIR}/../../../oss/library/gnuarm-4.8.2_neon_ca9
##

all:
	echo 'Make jansson lib...'
	make -C ../jansson all install
	echo 'Make jsonrpc client...'
	$(CC) $(CFLAGS) jsonrpc.c -c -fpic -L. -ljansson -o jsonrpc.o
	$(CC) $(CFLAGS) -shared -o libjsonrpc.so jsonrpc.o
	$(CC) $(CFLAGS) jsonrpc_server.c -L. -ljansson -ljsonrpc -o jsonrpc
	$(CC) $(CFLAGS) jsonrpc_client.c -o jsonrpc_client	

install:
	echo "Install jsonrpc library. OSS_LIB_ROOT=$(OSS_LIB_ROOT) THIS_DIR=$(THIS_DIR)"
	rm -rf $(OSS_LIB_ROOT)/jsonrpc
	mkdir -p $(OSS_LIB_ROOT)/jsonrpc
	mkdir -p $(OSS_LIB_ROOT)/jsonrpc/include
	mkdir -p $(OSS_LIB_ROOT)/jsonrpc/lib
	cp Makefile.lib $(OSS_LIB_ROOT)/jsonrpc/Makefile
	cp -r $(THIS_DIR)/*.h $(OSS_LIB_ROOT)/jsonrpc/include
	cp -r $(THIS_DIR)/*.so $(OSS_LIB_ROOT)/jsonrpc/lib
	cp -r $(THIS_DIR)/libjansson.so* $(OSS_LIB_ROOT)/jsonrpc/lib
	cp -r $(THIS_DIR)/jsonrpc $(OSS_LIB_ROOT)/jsonrpc
	cp -r $(THIS_DIR)/jsonrpc_client $(OSS_LIB_ROOT)/jsonrpc
	echo 'Release jsonrpc finished.'

clean:
	make -C ../jansson clean
	rm ./jsonrpc
	rm ./libjsonrpc.so
	rm ./jsonrpc_client
	rm -f `find . -name "*.o"`

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/jsonrpc_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/jsonrpc_INFO.txt