TOP ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))/..
CUSTOMER   ?= mtk
MODEL_NAME ?= mtk
TOOLCHAIN_ROOT ?= $(abspath $(TOP))/tools/mtk_toolchain
OSS_SRC_ROOT ?= $(abspath $(TOP))/oss/source
DTV_LINUX_POLICY_ROOT ?= $(abspath $(TOP))/linux_core/rootfs/common/sepolicy
TOOL_CHAIN ?= 10.2.1
SELinux_non_android ?= true
THIRD_PARTY_APP_AS_NON_ROOT ?= true
include $(abspath $(TOP))/build_eng/oss_version.mk
ifdef LIBPCRE_VERSION
export LIBPCRE_VERSION
export LIBPCRE_MODULE_ROOT=$(OSS_SRC_ROOT)/PCRE2/$(LIBPCRE_VERSION)
else
$(error LIBPCRE_VERSION isn't be defined!)
endif

ifeq "$(TOOL_CHAIN)" "5.5.0"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin
    OSS_LIB_ROOT := $(abspath $(TOP))/oss/library/gnuarm-5.5.0_neon_ca9
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    TOOL_CHAIN_BIN_PATH := $(TOOLCHAIN_ROOT)/gcc-arm-none-linux-gnueabihf-10.2.1/bin
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

export CC =     $(CROSS_COMPILE)gcc
export CCC =    $(CROSS_COMPILE)g++
export AS =     $(CROSS_COMPILE)as
export LD =     $(CROSS_COMPILE)ld
export STRIP =  $(CROSS_COMPILE)strip
export RANLIB = $(CROSS_COMPILE)ranlib

ifdef SELINUX_VERSION
MODULE_VER=$(SELINUX_VERSION)
else
$(error SELINUX_VERSION isn't be defined!)
endif
MODULE_NAME := selinux
MODULE_INSTALL_PATH := $(OSS_LIB_ROOT)/$(MODULE_NAME)/$(MODULE_VER)
MODULE_ROOT := $(OSS_SRC_ROOT)/$(MODULE_NAME)

all:
	@echo "SELinux Log : Building libselinux"; \
	cd $(MODULE_ROOT)/libselinux/src; \
	make all
	@echo "SELinux Log : Building libsepol"; \
	cd $(MODULE_ROOT)/libsepol/src; \
	make all
	@echo "SELinux Log : Building libselinux utils"; \
	cd $(MODULE_ROOT)/libselinux/utils; \
	make all
	@echo "SELinux Log : Building policycore utils"; \
	cd $(MODULE_ROOT)/policycoreutils; \
	make all

install: preinstall
	cp -f $(MODULE_ROOT)/libselinux/src/libselinux.so.1 $(MODULE_INSTALL_PATH)/lib
	cp -f $(MODULE_ROOT)/libsepol/src/libsepol.so.1 $(MODULE_INSTALL_PATH)/lib
	cp -f $(MODULE_ROOT)/libselinux/utils/getenforce $(MODULE_INSTALL_PATH)/bin
	cp -f $(MODULE_ROOT)/libselinux/utils/getsebool $(MODULE_INSTALL_PATH)/bin
	cp -f $(MODULE_ROOT)/libselinux/utils/setenforce $(MODULE_INSTALL_PATH)/bin
	cp -f $(MODULE_ROOT)/policycoreutils/restorecon/restorecon $(MODULE_INSTALL_PATH)/bin
	cp -f $(MODULE_ROOT)/policycoreutils/load_policy/load_policy $(MODULE_INSTALL_PATH)/bin
	cp -f $(MODULE_ROOT)/policycoreutils/setsebool/setsebool $(MODULE_INSTALL_PATH)/bin

preinstall:
	@if [ ! -d $(MODULE_INSTALL_PATH)/lib ]; then \
		mkdir -p $(MODULE_INSTALL_PATH)/lib; \
		mkdir -p $(MODULE_INSTALL_PATH)/bin; \
	fi

clean:
	@echo "SELinux Log : Cleaning libselinux"; \
	cd $(MODULE_ROOT)/libselinux/src; \
	make clean
	@echo "SELinux Log : Cleaning libsepol"; \
	cd $(MODULE_ROOT)/libsepol/src; \
	make clean
	@echo "SELinux Log : Cleaning libselinux utils"; \
	cd $(MODULE_ROOT)/libselinux/utils; \
	make clean
	@echo "SELinux Log : Cleaning policycore utils"; \
	cd $(MODULE_ROOT)/policycoreutils; \
	make clean
	@echo "SELinux Log : Cleaning output"
	rm -rf $(MODULE_INSTALL_PATH)/lib

uninstall:
	@echo "SELinux Log : Cleaning output"
	rm -rf $(MODULE_INSTALL_PATH)/lib

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/$(MODULE_NAME)_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/$(MODULE_NAME)_INFO.txt
