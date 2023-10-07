
SECURE_OPTION = true
#TOOL_CHAIN = gnuarm-5.5.0_neon_ca9
TOOL_CHAIN = gnuarm-10.2.1_neon_ca9
ENABLE_CA9_NEON = true
#ENABLE_CA9 = true
KERNEL_VER ?= linux-4.9
#HOST=arm-linux-gnueabi
HOST=arm-none-linux-gnueabihf
#CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/$(HOST)-
CROSS_COMPILE ?= /mfs/mcpslt0040/mtk18232/CAS_REAL/cas_2102_0217/merged/tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/$(HOST)-

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
#DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/linux_mts/build/mak
#include $(DTV_LINUX_MAK_ROOT)/path.mak
#include $(DTV_LINUX_MAK_ROOT)/common_inc.mak

CURRENT_ROOT:=$(shell pwd)

OSS_LIB_ROOT  ?= $(CURRENT_ROOT)/../../library/$(TOOL_CHAIN)
OSS_OUTPUT    := $(CURRENT_ROOT)
UTIL_LINUX_VER ?= util-linux-2.37.4

MODULE_VER  := $(UTIL_LINUX_VER)
MODULE_NAME := util-linux

MODULE_OUTPUT := $(OSS_OUTPUT)
MODULE_SRC_BUILD_PATH := $(MODULE_OUTPUT)/$(MODULE_VER)
MODULE_TAR := $(MODULE_VER).tar.gz
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)

LINUX_PAM_VER ?= Linux-PAM-1.3.1
PAM_TAR := $(LINUX_PAM_VER).tar.xz
PAM_SRC_BUILD_PATH := $(OSS_OUTPUT)/$(LINUX_PAM_VER)
PAM_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(LINUX_PAM_VER)

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++

export CFLAGS += -D_GNU_SOURCE -I$(PAM_INSTALL_PATH)/include -L$(PAM_INSTALL_PATH)/lib

ifeq "$(SECURE_OPTION)" "true"
CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie
endif

all: prepare linux_pam
	cd ${MODULE_SRC_BUILD_PATH}; \
	./autogen.sh; \
	./configure --prefix=${MODULE_INSTALL_PATH} --host=arm-linux  --disable-rpath --enable-shared=no --enable-static=yes --enable-login --without-ncurses --disable-use-tty-group --without-python --without-tinfo --disable-bash-completion --enable-static-programs=mount,umount,blkid --disable-makeinstall-chown
	make -C ${MODULE_SRC_BUILD_PATH}

install:
	make -C ${MODULE_SRC_BUILD_PATH} install

clean:
	rm -rf $(MODULE_SRC_BUILD_PATH)
	rm -rf $(MODULE_INSTALL_PATH)
	rm -rf $(PAM_SRC_BUILD_PATH)
	rm -rf $(PAM_INSTALL_PATH)

prepare:
	if [ ! -d $(MODULE_OUTPUT) ]; then \
		mkdir -p $(MODULE_OUTPUT); \
	fi
	tar -zxvf $(MODULE_TAR) -C $(MODULE_OUTPUT)
	if [ -d $(MODULE_VER)_patch ]; then \
		cp -rf $(MODULE_VER)_patch/* $(MODULE_SRC_BUILD_PATH)/; \
	fi

linux_pam:
	if [ ! -d $(PAM_SRC_BUILD_PATH) ]; then \
		mkdir -p $(PAM_SRC_BUILD_PATH); \
	fi
	tar -xf $(PAM_TAR) -C $(PAM_SRC_BUILD_PATH) --strip-components=1

	cd ${LINUX_PAM_VER}; \
	./configure --prefix=${PAM_INSTALL_PATH} --includedir=${PAM_INSTALL_PATH}/include/security --host=arm-linux --disable-rpath
	make -C ${PAM_SRC_BUILD_PATH} install

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

