#ifndef CROSS_COMPILE
#CROSS_COMPILE = /mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
#endif


export LDFLAGS = -fPIC -O0 -fsigned-char -fshort-wchar -Wno-strict-aliasing -march=armv7-a -mtune=cortex-a9  -fPIC -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s 

export CFLAGS = -fPIC -O0 -fsigned-char -fshort-wchar -Wno-strict-aliasing -march=armv7-a -mtune=cortex-a9 -fPIC -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s 

NCT5_ENABLE ?= false
ifeq ($(NCT5_ENABLE), true)
OSS_ROOT ?= $(word 1, $(subst /oss/,/oss /, $(shell pwd)))
include $(OSS_ROOT)/../build_eng/oss_version.mk
else
VM_LINUX_ROOT ?= $(word 1, $(subst /apollo/,/apollo /, $(shell pwd)))
OSS_ROOT ?= $(VM_LINUX_ROOT)/oss
include $(OSS_ROOT)/source/mak/oss_version.mak
endif

TOOL_CHAIN ?= 10.2.1
VFP_SUFFIX ?= _neon_ca9
OSS_LIB_ROOT = $(OSS_ROOT)/library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
OSS_TC ?= gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
TOOLCHAIN_ROOT = $(OSS_ROOT)/../tools/mtk_toolchain

ifeq "$(TOOL_CHAIN)" "5.5.0"
    CROSS_COMPILE := $(TOOLCHAIN_ROOT)/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
    PLATFORM_ARCH := arm-linux-gnueabi
    LDFLAGS += -mfloat-abi=softfp -mfpu=vfpv3-d16
    CFLAGS += -mfloat-abi=softfp -mfpu=vfpv3-d16
else ifeq "$(TOOL_CHAIN)" "10.2.1"
    CROSS_COMPILE := $(TOOLCHAIN_ROOT)/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
    PLATFORM_ARCH := arm-none-linux-gnueabihf
    LDFLAGS += -mfloat-abi=hard -mfpu=neon-vfpv4
    CFLAGS += -mfloat-abi=hard -mfpu=neon-vfpv4
endif

LIBXML2_NAME  := libxml2
LIBXML2_VER   := v$(LIBXML2_VERSION)

.PHONY: all install

all:
	@echo 'libxml2: make libxml2'
	@echo ${CROSS_COMPILE}
	@echo ${OSS_LIB_ROOT}
	@echo ${PLATFORM_ARCH}
	@echo ${LIBXML2_NAME}-${LIBXML2_VER}
	@echo ${OSS_TC}
	
	if [ ! -d ${LIBXML2_NAME}-${LIBXML2_VER} ]; then \
		tar zxfp ${LIBXML2_NAME}-${LIBXML2_VER}.tar.gz ;		\
	fi
	if [ ! -d ${OSS_LIB_ROOT}/build ]; then \
		mkdir ${OSS_LIB_ROOT}/build; \
		chmod 777 ${OSS_LIB_ROOT}/build; \
	fi
	
	if [ ! -d ${OSS_LIB_ROOT}/build/${LIBXML2_NAME} ]; then \
		mkdir ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}; \
		chmod 777 ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}; \
	fi
	cd ./${LIBXML2_NAME}-${LIBXML2_VER} && \
 	CC=${CROSS_COMPILE}gcc \
	LD=${CROSS_COMPILE}ld \
	AR=${CROSS_COMPILE}ar \
	AR=${CROSS_COMPILE}ar \
	RANLIB=${CROSS_COMPILE}ranlib \
	Z_LIBS=-L$(PWD)/../../library/$(OSS_TC)/zlib/1.2.3/pre-install/lib \
	./autogen.sh --host=${PLATFORM_ARCH} --target=${PLATFORM_ARCH} --without-python --without-lzma --without-zlib \
	--prefix=${OSS_LIB_ROOT}/build/${LIBXML2_NAME}
	cd ./${LIBXML2_NAME}-${LIBXML2_VER} && \
	make
	
install:
	@echo 'libxml2: install'

	cd ./${LIBXML2_NAME}-${LIBXML2_VER} && \
	make install
	rm -f ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}/lib/libxml2.la
	cd ./${LIBXML2_NAME}-${LIBXML2_VER} && \
	cp -f libxml2.la  ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}/lib
	#${CROSS_COMPILE}ranlib ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}/lib/libxml2.a
	
	if [ ! -d ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION} ]; then \
		mkdir ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}; \
	fi
	
	cp -rf ${OSS_LIB_ROOT}/build/${LIBXML2_NAME}/* ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}
	
	cd ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION} && \
	${CROSS_COMPILE}strip ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/lib/*.so \
	${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/bin/xmlcatalog \
	${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/bin/xmllint
	rm -f ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/lib/libxml2.so
	rm -f ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/lib/libxml2.so.2
ifeq ($(NCT5_ENABLE), true)
	cp -f Mk_for_lib ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/mediatek.mk
else
	cp -f Makefile_for_lib ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/lib/Makefile
endif
	rm -rf ${OSS_LIB_ROOT}/${LIBXML2_NAME}/${LIBXML2_VERSION}/share
	
	@echo 'libxml2: install success'
	
clean:
	@echo 'libxml2: make clean'
	cd ./${LIBXML2_NAME}-${LIBXML2_VER} && \
	make clean
	rm -rf .deps/
	rm -rf ${LIBXML2_NAME}-${LIBXML2_VER}
	rm -rf ${OSS_LIB_ROOT}/build

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/libxml2_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/libxml2_INFO.txt

