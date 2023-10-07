#! /bin/sh

oss_name=harfbuzz
oss_version=1.7.1
freetype_version=2.10.4
png_verison=1.2.59
zlib_version=1.2.11
icu_version=67.1
tool_chain=1021
work_path=`pwd`

location_correct=$(echo $work_path |grep "${oss_version}")

if ["$location_correct" = ""];then
echo "Please move build.sh to ${oss_name}-${oss_version} directory"
exit 1
else
echo "current path is ${work_path}"
fi


chmod 777 configure

if [ "$tool_chain" = '550' ];then
export CROSS_COMPILE="/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-"
export TARGET_NAME="arm-linux-gnueabi"
export TAR_INSTALL_PATH="$work_path/../../../library/gnuarm-5.5.0_neon_ca9"
elif [ "$tool_chain" = '482' ];then
export CROSS_COMPILE="/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/i686/bin/armv7a-mediatek482_001_neon-linux-gnueabi-"
export TARGET_NAME="armv7a-mediatek482_001_neon-linux-gnueabi"
export TAR_INSTALL_PATH="$work_path/../../../library/gnuarm-4.8.2_neon_ca9"
elif [ "$tool_chain" == '1021' ];then
export CROSS_COMPILE="$work_path/../../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-"
export TARGET_NAME="arm-none-linux-gnueabihf"
export TAR_INSTALL_PATH="$work_path/../../../library/gnuarm-10.2.1_neon_ca9"
fi

export HOST_NAME=`${CROSS_COMPILE}gcc -dumpmachine`

echo "TAR_INSTALL_PATH is ${TAR_INSTALL_PATH} build"
     

make clean


export FREETYPE_CFLAGS="-I${TAR_INSTALL_PATH}/freetype/${freetype_version}/pre-install/include/freetype2 -I${TAR_INSTALL_PATH}/png/${png_verison}/pre-install/include"
export FREETYPE_LIBS="-L${TAR_INSTALL_PATH}/freetype/${freetype_version}/pre-install/lib -lfreetype -L${TAR_INSTALL_PATH}/png/${png_verison}/pre-install/lib -lpng -L${TAR_INSTALL_PATH}/zlib/${zlib_version}/pre-install/lib -lz" 
export ICU_CFLAGS="-I${TAR_INSTALL_PATH}/icu/${icu_version}/include" 
export ICU_LIBS="-L${TAR_INSTALL_PATH}/icu/${icu_version}/lib -licui18n -licuuc" 
touch *\.*
touch src/*.hh

echo "CC is ${CROSS_COMPILE}gcc"
echo "HOST_NAME is ${HOST_NAME}"
echo "TARGET_NAME is ${TARGET_NAME}"

HFBZSECU_FLAGS=-fstack-protector-all\ -D_FORTIFY_SOURCE=2\ -Wl,-z,noexecstack\ -Wl,-z,noexecheap\ -Wl,-z,relro\ -Wl,-z,now\ -s

export CFLAGS=$HFBZSECU_FLAGS
export CXXFLAGS=$HFBZSECU_FLAGS
export LDFLAGS=$HFBZSECU_FLAGS

CC=${CROSS_COMPILE}gcc LD=${CROSS_COMPILE}ld CXX=${CROSS_COMPILE}g++ ./configure \
--host=${HOST_NAME} \
--target=${TARGET_NAME} \
--with-glib=no \
--with-gobject=no \
--with-cairo=no \
--with-icu=yes \
--with-freetype=yes \
--with-uniscribe=no \
--with-coretext=no \
--disable-gtk-doc \
--disable-gtk-doc-html \
--disable-gtk-doc-pdf \
--enable-shared \
--disable-static \
--disable-rpath \
--prefix=${TAR_INSTALL_PATH}/${oss_name}/${oss_version}/pre-install \
CFLAGS="-O2 $HFBZSECU_FLAGS" \
CXXFLAGS="-O2 $HFBZSECU_FLAGS" \

make clean
make
MAKE_RET=$?
if [ $MAKE_RET -ne 0 ]; then
    echo "harfbuzz Build Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "harfbuzz Build OK..."
fi

make install
if [ $MAKE_RET -ne 0 ]; then
    echo "harfbuzz Install Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "harfbuzz Install OK..."
fi



echo "build harfbuzz end"
