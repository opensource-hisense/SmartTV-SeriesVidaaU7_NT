#!/bin/sh

#DFB_PATH="/home/yu.zhang/dfb_1.4.2_bionic/dfb1.4.2"
ANDROID_ROOT="/home/yu.zhang/android5.1"
MST_PREFIX=/system
export DESTDIR=`pwd`/output
NDK_DIR="/home/jermy.liu/ndk/android-ndk-r10c"
NDK_SYSROOT="$ANDROID_ROOT/prebuilts/ndk/current/platforms/android-21/arch-arm"

#export PATH=$NDK_DIR/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin:$PATH
export PATH=/home/yu.zhang/arm-linux-androideabi-4.8/bin:/tools/r2-elf-linux-1.3.5.14/bin:$PATH
test -z $ANDROID_ROOT && echo "the ANDROID_ROOT must be set!!" && exit 0


DFB_NEEDED_PATH="$ANDROID_ROOT/external"
ANDROID_LIB_PATH="$ANDROID_ROOT/out/target/product/ponkan/system/lib"
UTOPIA_INCLUDE_PATH="$ANDROID_ROOT/device/mstar/ponkan/libraries/utopia/include"
NDK_LIB_PATH="$NDK_SYSROOT/usr/lib"
LD_DFB_PATH="$DESTDIR/system/lib"
LD_LIB_PATH="-L$LD_DFB_PATH -L$ANDROID_LIB_PATH -L$NDK_LIB_PATH"

DESTDIR=`pwd`/../output
DFB_PATH="$DESTDIR/system/include"
PREFIX_PATH="/system"

CFLAGS_DEFINE="-DHAVE_CONFIG_H -DHAVE_STDBOOL_H -D_GNU_SOURCE -D_REENTRANT -DHAVE_SIGNAL_H -DDIRECT_BUILD_NO_PTHREAD_CANCEL -DDIRECT_BUILD_NO_PTHREAD_CONDATTR=1 -DDIRECT_BUILD_NO_SA_SIGINFO=1 -DDIRECT_BUILD_NO_SIGQUEUE=1 -DGLES2_PVR2D -DPTHREADMINIT -DDIVINE_MAJOR_VERSION=1 -DDIVINE_MINOR_VERSION=6 -DANDROID_NDK -DFT2_BUILD_LIBRARY=1  -DHAVE_ANDROID_OS"
CFLAGS_INCLUDE="-I$DFB_NEEDED_PATH/libpng -I$DFB_NEEDED_PATH/zlib -I$DFB_NEEDED_PATH/jpeg -I$DFB_NEEDED_PATH/freetype/include -I$UTOPIA_INCLUDE_PATH -I/$NDK_SYSROOT/usr/include -I$NDK_DIR/sources/android/native_app_glue"

CFLAGS_SETTING=" -v -fno-rtti -fno-exceptions $CFLAGS_DEFINE $CFLAGS_INCLUDE -pie -fPIE"
CPPFLAGS_SETTING="-nostdlib -v -fno-rtti -fno-exceptions -fno-strict-aliasing"

LOCAL_LDFLAGS="-Wl,-v -lEGL -lGLESv2 -ljpeg -llog -ldl -lc -lstdc++ -Wl,-rpath,$ANDROID_LIB_PATH -landroid -lm $NDK_LIB_PATH/crtend_android.o -pie -fPIE"

DIRECTFB_CFLAGS_SETTING="-D_REENTRANT -I$DFB_PATH/include -I$DFB_PATH/directfb -I$DFB_PATH/directfb-internal"
DIRECTFB_LIBS_SETTING="-L$DESTDIR/system/lib -ldirect -lfusion -ldirectfb"

UTOPIA_LIB="-lutopia"

echo "Running autoconf & automake"
autoreconf --force --install
autoconf
automake

./configure     CC="arm-linux-androideabi-gcc --sysroot=$NDK_SYSROOT" \
                LD="arm-linux-androideabi-ld" \
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="$LD_LIB_PATH $LOCAL_LDFLAGS" \
                DIRECTFB_CFLAGS="$DIRECTFB_CFLAGS_SETTING"              \
                DIRECTFB_LIBS="$DIRECTFB_LIBS_SETTING"                  \
                --prefix=$PREFIX_PATH/directfb_examples                 \
                --host=arm-linux-androideabi \
                --build=i386-linux \
                --enable-debug
./fix_prefix.sh



