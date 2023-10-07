
SECURE_OPTION = true
TOOL_CHAIN = gnuarm-5.5.0_neon_ca9
#TOOL_CHAIN = gnuarm-10.2.1_neon_ca9
ENABLE_CA9_NEON = true
#ENABLE_CA9 = true
KERNEL_VER ?= linux-4.9
HOST=arm-linux-gnueabi

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
#DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/linux_mts/build/mak
#include $(DTV_LINUX_MAK_ROOT)/path.mak
#include $(DTV_LINUX_MAK_ROOT)/common_inc.mak
DTV_LINUX_MAK_ROOT ?= $(VM_LINUX_ROOT)/../../../build
include $(DTV_LINUX_MAK_ROOT)/path.mk
#include $(DTV_LINUX_MAK_ROOT)/common_inc.mak

CURRENT_ROOT:=$(shell pwd)
ifeq "$(TOOL_CHAIN)" "gnuarm-5.5.0_neon_ca9"
CROSS_COMPILE ?= $(CURRENT_ROOT)/../../../tools/mtk_toolchain/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
else
CROSS_COMPILE ?= $(CURRENT_ROOT)/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
endif
OSS_LIB_ROOT  ?= $(CURRENT_ROOT)/../../library/$(TOOL_CHAIN)
OSS_OUTPUT    := $(CURRENT_ROOT)
#INETUTIL_VER  ?= inetutils-1.4.2
INETUTIL_VER  ?= inetutils-2.2

MODULE_VER  := $(INETUTIL_VER)
MODULE_NAME := inetutils

MODULE_OUTPUT := $(OSS_OUTPUT)
MODULE_SRC_BUILD_PATH := $(MODULE_OUTPUT)/$(MODULE_VER)
MODULE_TAR := $(MODULE_VER).tar.gz
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)
MODULE_SYSLOGD_KLOG_PATH = /dev/kmsg

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++

CFLAGS += -D_GNU_SOURCE

ifeq "$(SECURE_OPTION)" "true"
CFLAGS += -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie
endif

all: prepare
	cd ${MODULE_SRC_BUILD_PATH}; \
	./configure --prefix=${MODULE_INSTALL_PATH} --host=$(HOST) CFLAGS="$(CFLAGS)" --disable-ftp --disable-ftpd --disable-rexecd \
	--disable-rshd --disable-talkd --disable-uucpd --disable-rlogin --disable-rsh --disable-logger --disable-talk \
	--disable-whois \
	--enable-telnetd \
	inetutils_cv_path_procnet_dev=yes \
	--enable-ifconfig=yes \
	--with-path-klog="$(MODULE_SYSLOGD_KLOG_PATH)"
	make -C ${MODULE_SRC_BUILD_PATH}

install:
	make -C ${MODULE_SRC_BUILD_PATH} install
	cp ${MODULE_SRC_BUILD_PATH}/telnetd/telnetd ${MODULE_INSTALL_PATH}/bin/telnetd
ifeq "$(INETUTIL_VER)" "inetutils-1.4.2"
	cp ${MODULE_SRC_BUILD_PATH}/syslogd/syslogd ${MODULE_INSTALL_PATH}/bin/syslogd
else
	cp ${MODULE_SRC_BUILD_PATH}/src/syslogd ${MODULE_INSTALL_PATH}/bin/syslogd
endif

clean:
	rm -rf $(MODULE_SRC_BUILD_PATH)
	rm -rf $(MODULE_INSTALL_PATH)

prepare:
	if [ ! -d $(MODULE_OUTPUT) ]; then \
		mkdir -p $(MODULE_OUTPUT); \
	fi
	tar -zxvf $(MODULE_TAR) -C $(MODULE_OUTPUT)
ifeq "$(INETUTIL_VER)" "inetutils-1.4.2"
	cp -rf $(MODULE_VER)_patch/* $(MODULE_SRC_BUILD_PATH)/
endif

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt

