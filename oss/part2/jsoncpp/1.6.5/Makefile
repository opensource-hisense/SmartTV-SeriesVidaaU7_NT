export SHELL := /bin/bash
export OSS_LIB=./../../library/gnuarm-10.2.1_neon_ca9

# prepare environment variable
export CROSS_COMPILE=../../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export NM=${CROSS_COMPILE}nm
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip
export RANLIB=${CROSS_COMPILE}ranlib

CJSON_SRCS += src/json_reader.cpp   \
              src/json_value.cpp    \
              src/json_writer.cpp

CJSON_OBJ_ROOT := .
CJSON_OBJECTS=$(addprefix $(CJSON_OBJ_ROOT)/, $(CJSON_SRCS:.cpp=.o))
CJSON_CC_FLAGS=-shared -Wall -fPIC -Wl,-export-dynamic -O2 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s
CJSON_LD_FLAGS=-shared -Wall -fPIC -Wl,-export-dynamic -Wl,-z,relro
CJSON_LINK_LIBS=-L$(PHASE2_OBJ_ROOT)/lib
CJSON_C_FLAGS=-I./include


TARGET=$(CJSON_OBJ_ROOT)/libjsoncpp.so

TARGET_STATIC=$(CJSON_OBJ_ROOT)/libjsoncpp_static.a


all: $(TARGET) $(TARGET_STATIC)

$(TARGET): $(CJSON_OBJECTS)
	$(CC) $(CJSON_LD_FLAGS) $(CJSON_LINK_LIBS) $(CJSON_OBJECTS) -o $@
	
$(TARGET_STATIC): $(CJSON_OBJECTS)
	$(AR) rv -o $@ $(CJSON_OBJECTS)
	
$(CJSON_OBJ_ROOT)/%.o : %.cpp
#	mkdir -p $(dir $@)
	$(CC) $(CJSON_C_FLAGS) $(CJSON_CC_FLAGS) $(DEFINES) -c $< -o $@

install:
#	cp -rf $(CJSON_OBJ_ROOT)/libjsoncpp.so $(CJSON_OBJ_ROOT)

clean:
	@rm -rf $(CJSON_OBJECTS)
	@rm -rf $(TARGET)
	@rm -rf $(TARGET_STATIC)

