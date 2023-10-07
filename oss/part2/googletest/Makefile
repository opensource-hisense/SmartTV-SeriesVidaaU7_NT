
VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss

#include $(VM_LINUX_ROOT)/linux_mts/build/mak/host.mak
include $(OSS_ROOT)/source/mak/oss_version.mak

TOOL_CHAIN ?= 5.5.0
VFP_SUFFIX ?= _neon_ca9
OSS_SRC_ROOT = $(OSS_ROOT)/source
LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_LIB_ROOT = $(OSS_ROOT)/$(LIB_SUB_ROOT)

GOOGLE_TEST_VERSION := googletest-1.7.0
GOOGLE_TEST_VERSION_NUM := 1.7.0
GOOGLE_TEST_INSTALL_ROOT := $(OSS_LIB_ROOT)/googletest/$(GOOGLE_TEST_VERSION_NUM)/

PWD = $(shell pwd)
SOURCE = $(PWD)/$(GOOGLE_TEST_VERSION)
OUTPUT = $(SOURCE)/build
THIS_ROOT   := $(shell pwd)
OBJ_ROOT   ?= $(THIS_ROOT)/obj

CMAKE_PATH ?= /mtkoss/cmake/3.9.3-linux/x86_64/bin

.PHONY: all install clean

all:install

init:
ifeq "$(TOOL_CHAIN)" "4.8.2"
#4.8.2 tool-chain
	cp toolchain-config-4.8.2 $(SOURCE)/toolchain-config
else
ifeq "$(TOOL_CHAIN)" "5.5.0"
#5.5.0 32 bit tool-chain
	cp toolchain-config-5.5.0 $(SOURCE)/toolchain-config
else
ifeq "$(TOOL_CHAIN)" "10.2.1"
	cp toolchain-config-10.2.1 $(SOURCE)/toolchain-config
else
	$(error "Tool chain version($(TOOL_CHAIN)) is invalid!")
endif
endif
endif
	$(warning "HUCP google test $(CROSS_COMPILE)")
	rm -rf $(OUTPUT)
	mkdir -p $(OUTPUT)
	cp $(SOURCE)/make.sh $(OUTPUT)
	cd $(OUTPUT) && sh make.sh

install:init
	mkdir -p $(GOOGLE_TEST_INSTALL_ROOT)/
	mkdir -p $(GOOGLE_TEST_INSTALL_ROOT)/lib/
	cp -rf $(OUTPUT)/libgtest.so $(GOOGLE_TEST_INSTALL_ROOT)/lib/
	cp -rf $(SOURCE)/include $(GOOGLE_TEST_INSTALL_ROOT)

clean:
	rm -rf $(OUTPUT)

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/googletest_INFO.txt

notice:	
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/googletest_NOTICE.txt
