

MODULE_VER  := $(IPTABLES_VERSION)
MODULE_NAME := iptables


export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++


MODULE_OUTPUT := $(OSS_OUTPUT)/$(MODULE_NAME)
MODULE_SRC_BUILD_PATH := $(MODULE_OUTPUT)/$(MODULE_NAME)-$(MODULE_VER)
MODULE_TAR := $(MODULE_NAME)-$(MODULE_VER).tar.bz2
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_NAME)-$(MODULE_VER)

all: prepare
	cd ${MODULE_SRC_BUILD_PATH};\
	./configure \
	--build=`uname -m`-linux \
	--host=${PLATFORM_ARCH} \
	--enable-debug \
	--with-random=dev/urandom \
	--prefix=${MODULE_INSTALL_PATH}

	make -C ${MODULE_SRC_BUILD_PATH}

install:
	make -C ${MODULE_SRC_BUILD_PATH} install

clean:
	rm -rf $(MODULE_SRC_BUILD_PATH)

prepare:
	if [ ! -d $(MODULE_OUTPUT) ]; then \
		mkdir -p $(MODULE_OUTPUT); \
	fi
	tar jxvf $(MODULE_TAR) -C $(MODULE_OUTPUT)

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt