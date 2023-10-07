export SHELL := /bin/bash

VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss

CJSON_VERSION ?= 1.7.7
TOOL_CHAIN ?= 5.5.0
VFP_SUFFIX ?= _neon_ca9
OSS_SRC_ROOT = $(OSS_ROOT)/source
LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_LIB_ROOT = $(OSS_ROOT)/$(LIB_SUB_ROOT)
JSON_INSTALL_PATH = $(OSS_LIB_ROOT)/cJSON/$(CJSON_VERSION)
JSON_LIB_PATH=$(JSON_INSTALL_PATH)/lib
JSON_INC_PATH=$(JSON_INSTALL_PATH)/include/cjson


# prepare environment variable
#4.8.2 TOOLCHAIN
#export CROSS_COMPILE ?= armv7a-mediatek482_001_neon-linux-gnueabi-
#5.5.0 ToolChain 64 bit
#export CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-aarch64-linux-gnu-5.5.0-ubuntu/x86_64/bin/aarch64-linux-gnu-

#5.5.0 Tool chain 32 bit
export CROSS_COMPILE ?=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-

export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export NM=${CROSS_COMPILE}nm
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip
export RANLIB=${CROSS_COMPILE}ranlib


CJSON_SRCS += cJSON.c cJSON_Utils.c

CJSON_OBJ_ROOT := .
CJSON_OBJECTS=$(addprefix $(CJSON_OBJ_ROOT)/, $(CJSON_SRCS:.c=.o))
CJSON_LD_FLAGS=-shared -Wall -fPIC -O2 -Wl,-export-dynamic -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s
#CJSON_LINK_LIBS=-L$(PHASE2_OBJ_ROOT)/lib

TARGET=$(CJSON_OBJ_ROOT)/libcJSON.so


all: $(TARGET)

$(TARGET): $(CJSON_OBJECTS)
	$(CC) $(CJSON_LD_FLAGS) $(CJSON_LINK_LIBS) $(CJSON_OBJECTS) -o $@
	
$(CJSON_OBJ_ROOT)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CJSON_LD_FLAGS) $(DEFINES) -c $< -o $@

install:
	mkdir -p $(JSON_LIB_PATH)
	mkdir -p $(JSON_INC_PATH)
	cp -rf $(CJSON_OBJ_ROOT)/libcJSON.so $(JSON_LIB_PATH)
	cp -rf $(CJSON_OBJ_ROOT)/cJSON.h $(JSON_INC_PATH)

clean:
	@rm -rf $(CJSON_OBJ_ROOT)/$(CJSON_OBJECTS)

