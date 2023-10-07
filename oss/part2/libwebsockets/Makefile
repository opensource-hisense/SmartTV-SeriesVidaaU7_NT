
VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss

include $(VM_LINUX_ROOT)/linux_mts/build/mak/host.mak
include $(OSS_ROOT)/source/mak/oss_version.mak

TOOL_CHAIN ?= 5.5.0
VFP_SUFFIX ?= _neon_ca9
OSS_SRC_ROOT = $(OSS_ROOT)/source
LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_LIB_ROOT = $(OSS_ROOT)/$(LIB_SUB_ROOT)

WEBSOCKETS_VERSION := libwebsockets-3.0-stable
WEBSOCKETS_VERSION_NUM := 3.0.0
WEBSOCKETS_INSTALL_ROOT := $(OSS_LIB_ROOT)/libwebsockets/$(WEBSOCKETS_VERSION_NUM)/

PWD = $(shell pwd)
SOURCE = $(PWD)/$(WEBSOCKETS_VERSION)
OUTPUT = $(SOURCE)/build

CMAKE_PATH ?= /mtkoss/cmake/3.9.3-linux/x86_64/bin
OBJ_ROOT = $(PWD)/../../../sys_build/mtk_linux/mt5886_us_linux/output/rel/
 
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
	$(error "Tool chain version($(TOOL_CHAIN)) is invalid!")
endif
endif
	$(warning "HUCP libwebsockets toolchain $(CROSS_COMPILE)") 
	rm -rf $(OUTPUT)
	mkdir -p $(OUTPUT)
	echo -e ".PHONY: all  \n\
	all:    \n\t\
	$(CMAKE_PATH)/cmake $(SOURCE) -DCMAKE_TOOLCHAIN_FILE=$(SOURCE)/toolchain-config \
	-DCMAKE_INSTALL_PREFIX=$(OUTPUT) \
	-DLWS_WITH_LWSWS=1 \
	-DLWS_WITH_SSL=0 \
	-DLWS_WITH_MBEDTLS=0 \
	-DLWS_LIBUV_LIBRARIES=$(OSS_LIB_ROOT)/libuv/1.20.3/lib/libuv.so \
	-DLWS_LIBUV_INCLUDE_DIRS=$(OSS_LIB_ROOT)/libuv/1.20.3/include \
	-DLWS_ZLIB_LIBRARIES=$(OBJ_ROOT)/basic/lib/libz.so \
	-DLWS_ZLIB_INCLUDE_DIRS=$(OSS_LIB_ROOT)/zlib/1.2.3/pre-install/include/" > $(OUTPUT)/Makefile
	make -C $(OUTPUT)
	make -C $(OUTPUT) all

install:init
	mkdir -p $(WEBSOCKETS_INSTALL_ROOT)/
	cp -rf $(OUTPUT)/lib $(WEBSOCKETS_INSTALL_ROOT)
	cp -rf $(OUTPUT)/include $(WEBSOCKETS_INSTALL_ROOT)

clean:
	rm -rf $(OUTPUT)

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/libwebsockets_INFO.txt

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/libwebsockets_NOTICE.txt
