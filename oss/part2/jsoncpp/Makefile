
#jsoncpp

TOOL_CHAIN := 10.2.1
VFP_SUFFIX := _neon_ca9

OSS_ROOT ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))
OSS_LIB_ROOT := $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)_neon_ca9
MY_DIR := $(OSS_ROOT)/source/jsoncpp
OSS_SOURCE_ROOT ?= $(OSS_ROOT)/source


JSONCPP_VERSION ?= 1.6.5
MODULE_NAME := jsoncpp

all:
	make -C $(MY_DIR)/$(JSONCPP_VERSION)/

install:
	cp $(MY_DIR)/$(JSONCPP_VERSION)/libjsoncpp.so $(OSS_LIB_ROOT)/jsoncpp/$(JSONCPP_VERSION)/lib/
	cp $(MY_DIR)/$(JSONCPP_VERSION)/libjsoncpp_static.a $(OSS_LIB_ROOT)/jsoncpp/$(JSONCPP_VERSION)/lib/
	cp -rf $(MY_DIR)/$(JSONCPP_VERSION)/include/* $(OSS_LIB_ROOT)/jsoncpp/$(JSONCPP_VERSION)/include/

clean:
	make -C $(MY_DIR)/$(JSONCPP_VERSION)/ clean

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt
