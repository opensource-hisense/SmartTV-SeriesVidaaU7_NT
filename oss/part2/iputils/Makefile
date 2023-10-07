
SECURE_OPTION = true
TOOL_CHAIN = 5.5.0
ENABLE_CA9_NEON = true
#ENABLE_CA9 = true
KERNEL_VER ?= linux-4.9
HOST=arm-linux-gnueabi
CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/$(HOST)-

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/linux_mts/build/mak
include $(DTV_LINUX_MAK_ROOT)/path.mak
include $(DTV_LINUX_MAK_ROOT)/common_inc.mak

CURRENT_ROOT:=$(shell pwd)
OSS_LIB_ROOT  ?= $(CURRENT_ROOT)/../../library/$(TOOL_CHAIN)
OSS_OUTPUT    := $(CURRENT_ROOT)
IPUTILS_VER   ?= iputils-s20101006

MODULE_VER  := $(IPUTILS_VER)
MODULE_NAME := iputils

MODULE_OUTPUT := $(OSS_OUTPUT)
MODULE_SRC_BUILD_PATH := $(MODULE_OUTPUT)/$(MODULE_VER)
MODULE_TAR := $(MODULE_VER).tar.gz
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)

MODULE_TARGETS=tracepath ping clockdiff rdisc arping tftpd rarpd tracepath6 traceroute6 ping6

OPEN_SSL_INC_PATH=$(OSS_LIB_ROOT)/openssl/$(OPENSSL_VERSION)/include
OPEN_SSL_LIB_PATH=$(OSS_LIB_ROOT)/openssl/$(OPENSSL_VERSION)/lib
OPENSSL_LIB=$(OPEN_SSL_LIB_PATH)/libresolv.so \
            $(OPEN_SSL_LIB_PATH)/libcrypto.so

OPENSSL_LDFLAGS=-lresolv -L$(OPEN_SSL_LIB_PATH) -lcrypto
export OPENSSL_LDFLAGS

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++

CFLAGS += -D_GNU_SOURCE
CFLAGS += -I$(OPEN_SSL_INC_PATH)

ifeq "$(SECURE_OPTION)" "true"
CFLAGS += -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie
endif

all: prepare
	if [ ! -d $(OPEN_SSL_INC_PATH) ]; then \
		echo Error: ${OPEN_SSL_INC_PATH} not exist; \
		echo Error: you should build openssl first; \
		exit 1; \
	fi
	make -C ${MODULE_SRC_BUILD_PATH} CFLAGS="$(CFLAGS)" LDFLAGS="$(CFLAGS)"

install:
	mkdir -p ${MODULE_INSTALL_PATH}/bin
	for i in $(MODULE_TARGETS); do \
	cp ${MODULE_SRC_BUILD_PATH}/$$i $(MODULE_INSTALL_PATH)/bin/$$i; \
	done

clean:
	rm -rf $(MODULE_SRC_BUILD_PATH)
	rm -rf $(MODULE_INSTALL_PATH)

prepare:
	if [ ! -d $(MODULE_OUTPUT) ]; then \
		mkdir -p $(MODULE_OUTPUT); \
	fi
	tar -zxvf $(MODULE_TAR) -C $(MODULE_OUTPUT)
	cp -rf $(MODULE_VER)_patch/* $(MODULE_SRC_BUILD_PATH)/


notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

