export SHELL := /bin/bash

# prepare environment variable
export OSS_LIB_ROOT ?= ../../../library/gnuarm-5.5.0_neon_ca9
export CROSS_COMPILE ?= /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export NM=${CROSS_COMPILE}nm
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip
export RANLIB=${CROSS_COMPILE}ranlib

CJSON_SRCS += cJSON.c

CJSON_OBJ_ROOT := .
CJSON_OBJECTS=$(addprefix $(CJSON_OBJ_ROOT)/, $(CJSON_SRCS:.c=.o))
CJSON_LD_FLAGS=-shared -Wall -fPIC -O2 -Wl,-export-dynamic -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s
CJSON_LINK_LIBS=

TARGET=$(CJSON_OBJ_ROOT)/libcJSON.so


all: $(TARGET)

$(TARGET): $(CJSON_OBJECTS)
	$(CC) $(CJSON_LD_FLAGS) $(CJSON_LINK_LIBS) $(CJSON_OBJECTS) -o $@
	
$(CJSON_OBJ_ROOT)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CJSON_LD_FLAGS) $(DEFINES) -c $< -o $@

install:
	mkdir -p $(OSS_LIB_ROOT)/cJSON/1.0.1/lib/
	mkdir -p $(OSS_LIB_ROOT)/cJSON/1.0.1/include/
	cp -rf $(CJSON_OBJ_ROOT)/libcJSON.so $(OSS_LIB_ROOT)/cJSON/1.0.1/lib/
	cp -rf $(CJSON_OBJ_ROOT)/cJSON.h $(OSS_LIB_ROOT)/cJSON/1.0.1/include/

clean:
	@rm -rf $(CJSON_OBJ_ROOT)/*.o
	@rm -rf $(CJSON_OBJ_ROOT)/*.so

