
SECURE_OPTION = true
TOOL_CHAIN = gnuarm-5.5.0_neon_ca9
#TOOL_CHAIN = gnuarm-10.2.1_neon_ca9
ENABLE_CA9_NEON = true
#ENABLE_CA9 = true
KERNEL_VER ?= linux-4.9
HOST=arm-linux-gnueabi
#HOST=arm-none-linux-gnueabihf
CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/$(HOST)-
#CROSS_COMPILE ?= ../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/$(HOST)-

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
#DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/linux_mts/build/mak
#include $(DTV_LINUX_MAK_ROOT)/path.mak
#include $(DTV_LINUX_MAK_ROOT)/common_inc.mak

CURRENT_ROOT:=$(shell pwd)
OSS_LIB_ROOT  ?= $(CURRENT_ROOT)/../../library/$(TOOL_CHAIN)
OSS_OUTPUT    := $(CURRENT_ROOT)
TAR_VER       ?= tar-1.17

MODULE_VER  := $(TAR_VER)
MODULE_NAME := tar

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
CFLAGS += -O2

ifeq "$(SECURE_OPTION)" "true"
CFLAGS += -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie
endif

all: prepare
	cd ${MODULE_SRC_BUILD_PATH}; \
	./configure --prefix=${MODULE_INSTALL_PATH} --host=$(HOST) CFLAGS="$(CFLAGS)"
	make -C ${MODULE_SRC_BUILD_PATH}

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
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/tar-1.17.mtk.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2010-0624-rtapelib-overflow.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2016-6321.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2018-20482.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2019-9923.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2007-4476.patch
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2021-20193.patch


notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

