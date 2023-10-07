export OSS_SRC=/.../apollo/oss/source
export OSS_LIB=/.../apollo/oss/library/gnuarm-5.5.0_neon_ca9/
export OSS_COMPILE_PRE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export PLATFORM_CFLAGS="-march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fPIC"

cd $OSS_SRC/libiconv

make all install \
	CROSS_COMPILE=$OSS_COMPILE_PRE \
	PLATFORM_ARCH=i686 \
	OSS_LIB_ROOT=$OSS_LIB \
	LIBICONV_VERSION=1.11.1 \
	OSS_OUTPUT=$OSS_SRC/libiconv/

# cd $OSS_LIB/libiconv/1.11.1 && rm ./lib/libcharset.a lib/lib*.so lib/lib*.so.[12] ./lib/*.la
# "$OSS_COMPILE_PRE"strip --strip-unneeded -R .comment lib/*.so*

