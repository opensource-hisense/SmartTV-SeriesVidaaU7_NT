
EXPAT_ROOT=$(shell pwd)

ifndef OSS_ROOT
OSS_ROOT ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))
endif

ifndef THIRDPARTY_ROOT
THIRDPARTY_ROOT := $(OSS_ROOT)/../third_party
endif

ifndef DTV_LINUX_ROOT
DTV_LINUX_ROOT := $(OSS_ROOT)/../linux_mts
endif

ifndef DTV_LINUX_MAK_ROOT
DTV_LINUX_MAK_ROOT := $(DTV_LINUX_ROOT)/build/mak
endif
#include $(DTV_LINUX_ROOT)/build/mak/common_inc.mak
TOOL_CHAIN ?= 10.2.1
ENABLE_CA9_NEON ?= true
EXPAT_VERSION ?= 2.5.0
OSS_OUTPUT ?= $(EXPAT_ROOT)
LINUX_TOOLCHAIN_TOP ?= $(OSS_ROOT)/../tools/mtk_toolchain

ifeq "$(TOOL_CHAIN)" "4.8.2"
    ifeq "$(ENABLE_CA9)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/vfp_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek482_001_vfp-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC
    else ifeq "$(ENABLE_CA9_NEON)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek482_001_neon-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fPIC
    else ifeq "$(ENABLE_CA15_NEON)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a15-ubuntu/x86_64
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek482_001_neon-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a15 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC
    else
        $(error "ENABLE_CA9" or "ENABLE_CA9_NEON" must be set to true)
    endif
#else ifeq "$(TOOL_CHAIN)" "4.5.1"
#    ifeq "$(ENABLE_CA9)" "true"
#        #export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/vfp_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64
#        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/vfp_4.5.1_2.6.27_cortex-a9-rhel4/i686
#
#        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
#
#        #export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek482_001_vfp-linux-gnueabi-
#        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek451_001_vfp-linux-gnueabi-
#
#        CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC
#    endif
else ifeq "$(TOOL_CHAIN)" "4.9.1"
    ifeq "$(ENABLE_CA9_NEON)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/hard_4.9.1_armv7a-cros/x86_64/armv7a
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-cros-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC
    endif
else ifeq "$(TOOL_CHAIN)" "4.9.2"
    ifeq "$(ENABLE_CA9_NEON)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/hard_4.9.2-r116_armv7a-cros/x86_64/armv7a
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-cros-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC
    endif
else ifeq "$(TOOL_CHAIN)" "5.5.0"
    ifeq "$(ENABLE_CA9_NEON)" "true"
        export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-linux-gnueabi-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fPIC
    endif
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    ifeq "$(ENABLE_CA9_NEON)" "true"
        export TOOL_CHAIN_ROOT = $(LINUX_TOOLCHAIN_TOP)/gcc-arm-none-linux-gnueabihf-10.2.1
        export TOOL_CHAIN_BIN_PATH := $(TOOL_CHAIN_ROOT)/bin
        export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/arm-none-linux-gnueabihf-

        MTK_TOOLCHAIN_CC_FLAG += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC
    endif
else
    $(error TOOL_CHAIN=$(TOOL_CHAIN) is not support support)
endif

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export CXX=${CROSS_COMPILE}g++

ifneq (,$(findstring $(TOOL_CHAIN), 4.8.2 5.5.0 10.2.1))
    ifeq "$(ENABLE_CA9)" "true"
        export VFP_SUFFIX = _vfp_ca9
    else ifeq "$(ENABLE_CA9_NEON)" "true"
        export VFP_SUFFIX = _neon_ca9
    else ifeq "$(ENABLE_CA15_NEON)" "true"
        export VFP_SUFFIX = _neon_ca15
    endif
else ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
    ifeq "$(ENABLE_CA9_NEON)" "true"
        export VFP_SUFFIX = _neon_ca9
    endif
#else ifeq "$(TOOL_CHAIN)" "4.5.1"
#    ifeq "$(ENABLE_CA9)" "true"
#        export VFP_SUFFIX = _vfp_ca9
#    endif
endif

OSS_LIB_ROOT ?= $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)

EXPAT_OUTPUT := $(OSS_OUTPUT)/expat
EXPAT_SOURCE := $(EXPAT_OUTPUT)/expat-$(EXPAT_VERSION)
EXPAT_BUILD := $(EXPAT_OUTPUT)/expat-$(EXPAT_VERSION)-build
EXPAT_FILE := $(EXPAT_VERSION)/expat-$(EXPAT_VERSION).tar.bz2
MTK_TOOLCHAIN_CC_FLAG += -D_CPU_LITTLE_ENDIAN -g -s
MTK_TOOLCHAIN_CC_FLAG += -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now

all: expat_prepare
	if [ ! -d $(EXPAT_BUILD) ]; then \
		mkdir -p $(EXPAT_BUILD); \
	fi
	cd $(EXPAT_BUILD) && \
	CC="$(CROSS_COMPILE)gcc" RANLIB="$(CROSS_COMPILE)ranlib" CFLAGS="$(MTK_TOOLCHAIN_CC_FLAG)" \
	$(EXPAT_SOURCE)/configure \
	--host=arm-mtk-linux \
	--prefix=$(OSS_LIB_ROOT)/expat/$(EXPAT_VERSION) \
	--without-docbook
	make -j $(JOBS) -C $(EXPAT_BUILD)

install:
	make -C $(EXPAT_BUILD) install

install-strip:
	make -C $(EXPAT_BUILD) install-strip

clean:
	rm -rf $(EXPAT_OUTPUT)

# oss/source/expat/expat/expat-$(EXPAT_VERSION)/COPYING
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/expat_NOTICE.txt

# oss/source/expat/expat/expat-$(EXPAT_VERSION)/README.md
info:
	cp -rf OSS_INFO.$(EXPAT_VERSION).txt $(OSS_INFO_OUTPUT)/expat_INFO.txt

expat_prepare:
	if [ ! -d $(EXPAT_SOURCE) ]; then \
		mkdir -p $(EXPAT_SOURCE); \
		tar xjf $(EXPAT_FILE) -C $(EXPAT_OUTPUT); \
		chmod -R 755 $(EXPAT_SOURCE); \
	fi
#tar.gz use tar zxf
