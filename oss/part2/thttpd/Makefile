.PHONY: all install clean

THIS_DIR := $(shell pwd)
THTTPD_VER := 2.25b
#for test
OSS_LIB_ROOT := ${THIS_DIR}/../../../oss/library/gnuarm-4.8.2_neon_ca9
##

all:
	@echo "make thttpd" 
	tar -xvf ./thttpd-2.25b.tar.gz
	make -C ./back_thttpd-2.25b
	
install:
	mkdir -p  $(OSS_LIB_ROOT)/thttpd/$(THTTPD_VER)
	cp -f ./back_thttpd-2.25b/thttpd $(OSS_LIB_ROOT)/thttpd/$(THTTPD_VER)/
	
clean:
	rm -rf  $(OSS_LIB_ROOT)/thttpd/$(THTTPD_VER)/thttpd/$(THTTPD_VER)
	make -C ./back_thttpd-2.25b clean
	
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/thttpd_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/thttpd_INFO.txt


