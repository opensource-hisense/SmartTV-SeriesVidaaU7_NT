#!/bin/bash

export OSS_SRC=$(pwd)/../

#For toolchain 5.5.0
export OSS_LIB=$(pwd)/../../library/gnuarm-5.5.0_neon_ca9/
export OSS_COMPILE_PRE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export PLATFORM_CFLAGS="-march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fPIC"

if [ ! -z $1 ] ; then
	if [ $1 == "10.2.1" ] ; then
		#For toolchain 10.2.1
		export OSS_LIB=$(pwd)/../../library/gnuarm-10.2.1_neon_ca9/
		export OSS_COMPILE_PRE=$(pwd)/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
		export PLATFORM_CFLAGS="-march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC"
	fi
fi

make all install \
	CROSS_COMPILE=$OSS_COMPILE_PRE \
	OSS_LIB_ROOT=$OSS_LIB \
	LIBEXIF_VERSION=0.6.23 \
	OSS_OUTPUT=$OSS_SRC/libexif/
