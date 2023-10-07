ifndef LINUX_ROOT
export LINUX_ROOT  ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
endif


ifndef THIRDPARTY_ROOT
THIRDPARTY_ROOT         := $(LINUX_ROOT)/third_party
endif

export TOOL_CHAIN ?= 4.8.2
export VFP_SUFFIX ?= _neon_ca9
export ENABLE_CA9 ?= false
export ENABLE_CA9_NEON ?= true

DTV_LINUX_ROOT := $(LINUX_ROOT)/linux_mts
include $(DTV_LINUX_ROOT)/mak/toolchain.mak
#include $(DTV_LINUX_ROOT)/mak/common_inc.mak
include $(LINUX_ROOT)/oss/source/mak/oss_version.mak
include $(LINUX_ROOT)/third_party/source/mak/third_party_version.mak


ifndef THIRDPARTY_LIB_ROOT
export THIRDPARTY_LIB_ROOT ?=$(THIRDPARTY_ROOT)/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
endif

ifndef OSS_LIB_ROOT
export OSS_LIB_ROOT ?= $(LINUX_ROOT)/oss/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
endif

ifndef MTK_UTIL_LIB_ROOT
MTK_UTIL_LIB_ROOT ?= $(DTV_LINUX_ROOT)/mtk_util/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
endif

#Curl 
export CURL_LIB_DIR     = $(OSS_LIB_ROOT)/curl/curl-$(CURL_VERSION)

#OpenSSL
export OPENSSL_LIB_DIR = $(OSS_LIB_ROOT)/openssl/$(OPENSSL_VERSION)

#Playready
export PLAYREADY_LIB_DIR = $(THIRDPARTY_LIB_ROOT)/playready/$(PLAYREADY_VERSION)/common

#Care
export CARES_LIB_DIR = $(OSS_LIB_ROOT)/c-ares/cares-$(CARES_VERSION)

#Zlib
export ZLIB_LIB_DIR =$(OSS_LIB_ROOT)/zlib/$(ZLIB_VERSION)/pre-install

#Mtal
export MTAL_DIR = $(LINUX_ROOT)/linux_core/app_if/mtal

#Securestorage
export SECURESTORAGE_DIR= $(MTK_UTIL_LIB_ROOT)/securestorage

#freetype for DFB
export FREETYPE_DIR= $(OSS_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install

CFLAGS  := -Wall $(MTK_TOOLCHAIN_CC_FLAG)
SOURCES := $(wildcard *.cpp)
OBJS    := $(SOURCES:cpp=o)
CSOURCES  := $(wildcard *.c)
COBJS := $(CSOURCES:c=o)

CFLAGS += -fPIC
CFLAGS += -DDRM_BUILD_PROFILE=900

ifeq ($(BUILD_CFG), debug)
    CFLAGS += -g -O3 -DDEBUG
    CFLAGS += -D_DEBUG
	export BUILD_DIR=debug
else
    CFLAGS += -O3
    CFLAGS += -DNDEBUG
	export BUILD_DIR=rel
endif

ifeq ($(ENDIAN), LITTLE)
    CFLAGS += -DLITTLE_ENDIAN
endif
