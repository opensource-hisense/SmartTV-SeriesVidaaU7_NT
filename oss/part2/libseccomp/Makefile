TOP ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))/..
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
TOOLCHAIN_ROOT ?= $(abspath $(TOP))/tools/mtk_toolchain
OSS_SRC_ROOT ?= $(abspath $(TOP))/oss/source
TOOL_CHAIN ?= 10.2.1
ENABLE_CA9_NEON ?= true

ifeq "$(TOOL_CHAIN)" "5.5.0"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin
    TOOL_CHAIN_HOST := arm-linux-gnueabi
    OSS_LIB_ROOT := $(abspath $(TOP))/oss/library/gnuarm-5.5.0_neon_ca9
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-none-linux-gnueabihf-10.2.1/bin
    TOOL_CHAIN_HOST := arm-none-linux-gnueabihf
    OSS_LIB_ROOT := $(abspath $(TOP))/oss/library/gnuarm-10.2.1_neon_ca9
endif

ifndef CROSS_COMPILE
    ifneq (,$(findstring $(TOOL_CHAIN), 5.5.0))
            CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-linux-gnueabi-
    else ifneq (,$(findstring $(TOOL_CHAIN), 10.2.1))
            CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-none-linux-gnueabihf-
    endif
endif

export PATH := $(TOOL_CHAIN_BIN_PATH):$(PATH)

CC =     $(CROSS_COMPILE)gcc
CCC =    $(CROSS_COMPILE)g++
AS =     $(CROSS_COMPILE)as
LD =     $(CROSS_COMPILE)ld
STRIP =  $(CROSS_COMPILE)strip
RANLIB = $(CROSS_COMPILE)ranlib

MODULE_VER  := libseccomp-2.4.2
MODULE_NAME := libseccomp
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)
MODULE_ROOT := $(OSS_SRC_ROOT)/$(MODULE_NAME)
SECURE_HARDENING_FLAGS := -Wformat=2 -Wformat-security -O2 -fstack-protector-strong -s -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now

all:
	@echo "libseccomp Log : Building libseccomp";
	cd $(MODULE_ROOT)/$(MODULE_VER); \
	./autogen.sh;\
	chmod +x configure;\
	./configure --prefix=/ --host=$(TOOL_CHAIN_HOST) ; \
	make CFLAGS='$(SECURE_HARDENING_FLAGS)' && make DESTDIR=$(MODULE_ROOT)/$(MODULE_VER)/result install

install: preinstall
	@echo "libseccomp Log: Going to install libseccomp"
	cp -f `find $(MODULE_ROOT)/$(MODULE_VER)/result/lib/ -name "libseccomp.so\.[0-9]*\.[0-9]*\.[0-9]*"` $(MODULE_INSTALL_PATH)/lib
	cp -rf $(MODULE_ROOT)/$(MODULE_VER)/result/include $(MODULE_INSTALL_PATH)/

preinstall:
	@if [ ! -d $(MODULE_INSTALL_PATH)/lib ]; then \
		mkdir -p $(MODULE_INSTALL_PATH)/lib; \
	fi

clean:
	@echo "libseccomp Log : Cleaning libseccomp library"
	cd $(MODULE_ROOT)/$(MODULE_VER); \
	make DESTDIR=$(MODULE_ROOT)/$(MODULE_VER)/result clean && make DESTDIR=$(MODULE_ROOT)/$(MODULE_VER)/result uninstall ;\
	rm -rf result
	@echo "libseccomp Log : Cleaning output"
	rm -rf $(MODULE_INSTALL_PATH)/lib
	rm -rf $(MODULE_INSTALL_PATH)/include

uninstall:
	@echo "libseccomp Log : Cleaning output"
	rm -rf $(MODULE_INSTALL_PATH)/lib
	rm -rf $(MODULE_INSTALL_PATH)/include

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt
