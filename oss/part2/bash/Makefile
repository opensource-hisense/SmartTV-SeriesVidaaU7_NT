
SECURE_OPTION = true
TOOL_CHAIN = gnuarm-5.5.0_neon_ca9
ENABLE_CA9_NEON = true
#ENABLE_CA9 = true
KERNEL_VER ?= linux-4.9
HOST=arm-linux-gnueabi
CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/$(HOST)-

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
#DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/linux_mts/build/mak
#include $(DTV_LINUX_MAK_ROOT)/path.mak
#include $(DTV_LINUX_MAK_ROOT)/common_inc.mak

CURRENT_ROOT:=$(shell pwd)


OSS_LIB_ROOT  ?= $(CURRENT_ROOT)/../../library/$(TOOL_CHAIN)
OSS_OUTPUT    := $(CURRENT_ROOT)
BASH_VER      ?= bash-3.2.57

MODULE_VER  := $(BASH_VER)
MODULE_NAME := bash

MODULE_OUTPUT := $(OSS_OUTPUT)
MODULE_SRC_BUILD_PATH := $(MODULE_OUTPUT)/$(MODULE_VER)
MODULE_TAR := $(MODULE_VER).tar.gz
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)


export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++

CFLAGS += -D_GNU_SOURCE

ifeq "$(SECURE_OPTION)" "true"
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie
endif

all: prepare
	cd ${MODULE_SRC_BUILD_PATH}; \
	./configure --prefix=${MODULE_INSTALL_PATH} --build=i686-pc-linux-gnu --host=$(HOST) CFLAGS="$(CFLAGS)"
	make -C ${MODULE_SRC_BUILD_PATH} V=1

install:
	make -C ${MODULE_SRC_BUILD_PATH} install

clean:
	rm -rf $(MODULE_SRC_BUILD_PATH)
	rm -rf $(MODULE_INSTALL_PATH)

prepare:
	if [ ! -d $(MODULE_OUTPUT) ]; then \
		mkdir -p $(MODULE_OUTPUT); \
	fi
	tar -zxvf $(MODULE_TAR) -C $(MODULE_OUTPUT)
	patch -d $(MODULE_SRC_BUILD_PATH) -p0 < ./bash-3.2-patches/bash43-048-mtk
	patch -d $(MODULE_SRC_BUILD_PATH) -p0 < ./bash-3.2-patches/bash44-006-mtk
	patch -d $(MODULE_SRC_BUILD_PATH) -p2 < ./bash-3.2-patches/bash32-mtk-patch-001
	patch -d $(MODULE_SRC_BUILD_PATH) -p2 < ./bash-3.2-patches/bash-mtk-builtins_Makefile.in-patch
	patch -d $(MODULE_SRC_BUILD_PATH) -p1 < ./bash-3.2-patches/CVE-2019-18276.patch

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

