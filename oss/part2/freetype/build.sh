#!/bin/sh

TOOL_CHAIN="10.2.1"
rootpath=`pwd`
FREETYPE_VERSION="2.12.1"

FREETYPE_TAR_FILE="freetype-2.12.1.tar.gz"
FREETYPE_TAR_FOLDER="freetype-2.12.1"
FREETYPE_PATCH="freetype-2.12.1.patch"

LIB_PNG_VER="1.2.59"
LIB_ZLIB_VER="1.2.11"

echo "rootpagh is $rootpath"

if [ -d "$FREETYPE_TAR_FOLDER" ]
then
	rm -rf $FREETYPE_TAR_FOLDER
	sync
fi

tar -xzvf $FREETYPE_TAR_FILE

if [ ! -d "$FREETYPE_TAR_FOLDER" ]
then
	echo "$FREETYPE_TAR_FOLDER not exist, exit";
	exit;
fi

git apply $FREETYPE_PATCH

cd $FREETYPE_TAR_FOLDER

if [ "$TOOL_CHAIN" = "5.5.0" ]
then
	TOOL_CHAIN_PATH="/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin"
	T_GCC="arm-linux-gnueabi-gcc"
	T_CXX="arm-linux-gnueabi-g++"
	T_LD="arm-linux-gnueabi-ld"
	T_OSS_LIB=$rootpath/../../library/gnuarm-5.5.0_neon_ca9
elif [ "$TOOL_CHAIN" = "4.8.2" ]
then
	TOOL_CHAIN_PATH="/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64/bin"
	T_GCC="armv7a-mediatek482_001_neon-linux-gnueabi-gcc"
	T_CXX="armv7a-mediatek482_001_neon-linux-gnueabi-g++"
	T_LD="armv7a-mediatek482_001_neon-linux-gnueabi-ld"
	T_OSS_LIB=$rootpath/../../library/gnuarm-4.8.2_neon_ca9
elif [ "$TOOL_CHAIN" = "10.2.1" ]
then
	TOOL_CHAIN_PATH="$rootpath/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin"
	T_GCC="arm-none-linux-gnueabihf-gcc"
	T_CXX="arm-none-linux-gnueabihf-g++"
	T_LD="arm-none-linux-gnueabihf-ld"
	T_OSS_LIB=$rootpath/../../library/gnuarm-10.2.1_neon_ca9
else
	echo "Unknow tool chain($TOOL_CHAIN), exit."
	exit
fi

OSS_OUT_DIR=$rootpath/../../../out/mtk_linux/mt5870p_us_1g_mlr53/rootfs/basic/lib

LIB_SEC_FLAG=-fstack-protector\ -D_FORTIFY_SOURCE=2\ -Wl,-z,noexecstack\ -Wl,-z,noexecheap\ -Wl,-z,relro\ -Wl,-z,now\ -s
#C_FLAG=\"-I$T_OSS_LIB/png/$LIB_PNG_VER/pre-install/include -I$T_OSS_LIB/zlib/$LIB_ZLIB_VER/pre-install/include $LIB_SEC_FLAG\"
#C_FLAG+=$LIB_SEC_FLAG
#echo C_FLAG=$C_FLAG
#exit

export LDFLAGS=$LIB_SEC_FLAG
./configure CC=$TOOL_CHAIN_PATH/$T_GCC CXX=$TOOL_CHAIN_PATH/$T_CXX LD=$TOOL_CHAIN_PATH/$T_LD prefix=`pwd`/pre-install --host=arm-linux-gnueabi --without-bzip2 --without-harfbuzz CFLAGS="-O2 -I$T_OSS_LIB/png/$LIB_PNG_VER/pre-install/include -I$T_OSS_LIB/zlib/$LIB_ZLIB_VER/pre-install/include $LIB_SEC_FLAG" LIBPNG_LIBS=" -L$T_OSS_LIB/png/$LIB_PNG_VER/pre-install/lib -lpng" ZLIB_LIBS="-L$T_OSS_LIB/zlib/$LIB_ZLIB_VER/pre-install/lib -lz" CXXFLAGS="$LIB_SEC_FLAG"

echo "CC=$T_GCC"
echo "CXX=$T_CXX"
echo "LD=$T_LD"
echo "OSS_LIB=$T_OSS_LIB"
echo "OSS_OUT_DIR=$OSS_OUT_DIR"
make
make install

if [ -d "$T_OSS_LIB/freetype/$FREETYPE_VERSION" ]
then
	rm -rf $T_OSS_LIB/freetype/$FREETYPE_VERSION/*
else
	mkdir $T_OSS_LIB/freetype/$FREETYPE_VERSION
fi

#copy lib to oss/lib folder
cp -r `pwd`/pre-install $T_OSS_LIB/freetype/$FREETYPE_VERSION/

cp -r `pwd`/include/freetype/internal $T_OSS_LIB/freetype/$FREETYPE_VERSION/pre-install/include/freetype2/freetype

#create Makefile for freetype lib
T_OSS_LIB_MAKE="$T_OSS_LIB/freetype/$FREETYPE_VERSION/mediatek.mk"
echo -e "LOCAL_PATH := \$(call my-dir)" > $T_OSS_LIB_MAKE
echo -e "include \$(MTK_CLEAR_VARS)" >> $T_OSS_LIB_MAKE
echo -e "LOCAL_MODULE := libfreetype\n" >> $T_OSS_LIB_MAKE

echo -e "LOCAL_MODULE_CLASS := SHARED_LIBRARIES" >> $T_OSS_LIB_MAKE

echo -e "LOCAL_SRC_FILES := pre-install/lib/libfreetype.so.6.18.3" >> $T_OSS_LIB_MAKE

echo -e "LOCAL_CREATE_SYMLINK += libfreetype.so.6.18.3:libfreetype.so \\" >> $T_OSS_LIB_MAKE

echo -e "\t\t\tlibfreetype.so.6.18.3:libfreetype.so.6" >> $T_OSS_LIB_MAKE
echo -e "\n" >> $T_OSS_LIB_MAKE

echo -e "LOCAL_MODULE_PATH := \$(TARGET_BASIC_OUT)/lib" >> $T_OSS_LIB_MAKE
echo -e "LOCAL_EXPORT_C_INCLUDE_DIRS := \$(LOCAL_PATH)/pre-install/include   \\" >> $T_OSS_LIB_MAKE
echo -e "\t\t\t\$(LOCAL_PATH)/pre-install/include/freetype2\n" >> $T_OSS_LIB_MAKE
echo -e "LOCAL_SHARED_LIBRARIES += libpng libz\n" >> $T_OSS_LIB_MAKE
echo -e "include \$(MTK_BUILD_PREBUILT_BY_SYMLINK)" >> $T_OSS_LIB_MAKE

