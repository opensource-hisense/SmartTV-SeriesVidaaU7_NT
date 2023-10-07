#! /bin/sh

################OSS ICU build steps:#####################
#step 1,build icu
#echo 'step 1,build icu'
#

#icu_ver=59.1
work_path=`pwd`

if [ ! -n "$1" ];then
	icu_ver=67.1
else
	icu_ver=$1
fi

icu_path=$work_path/$icu_ver
harfbuzz_ver=1.7.1
tool_chain=1021
securityBuild=yes
SECU_FLAGS=
HOST=arm-linux

if [ "$securityBuild" = "yes" ];then
	SECU_FLAGS=-fstack-protector-all\ -D_FORTIFY_SOURCE=2\ -Wl,-z,noexecstack\ -Wl,-z,noexecheap\ -Wl,-z,relro\ -Wl,-z,now\ -s
fi

echo icu_ver=$icu_ver
echo icu_path=$icu_path
echo tool_chain=$tool_chain
echo securityBuild=$securityBuild
echo SECU_FLAGS=$SECU_FLAGS

#rm -rf $icu_path/buildA $icu_path/buildB $icu_path/icu/pre-install
#rm -rf $icu_path/../icu-le-hb-1.0.3/src/*.o $icu_path/../icu-le-hb-1.0.3/src/*.so $icu_path/../icu-le-hb-1.0.3/pre-install $icu_path/../icu-le-hb-1.0.3/src/.libs
#if [ "clean" == $2 ];then
#	exit
#fi

export CFLAGS=$SECU_FLAGS
export CXXFLAGS=$SECU_FLAGS
export CPPFLAGS=$SECU_FLAGS
export LDFLAGS=$SECU_FLAGS

mkdir -p $icu_path
mkdir -p $icu_path/buildA
mkdir -p $icu_path/buildB
chmod +x $icu_path/icu/source/runConfigureICU
chmod +x $icu_path/icu/source/configure
chmod +x $icu_path/icu/source/install-sh
cd $icu_path/buildA ;
../icu/source/runConfigureICU Linux/gcc ;

make RANLIB=ranlib AR=ar 
cd ../buildB;

if [ "$tool_chain" = "550" ];then
export CROSS_COMPILE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export C_COMPILER=${CROSS_COMPILE}gcc
export CXX_COMPILER=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export OSS_LIB_DIR=$work_path/../../library/gnuarm-5.5.0_neon_ca9
elif [ "$tool_chain" = "482" ];then
export CROSS_COMPILE=/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/i686/bin/armv7a-mediatek482_001_neon-linux-gnueabi-
export C_COMPILER=${CROSS_COMPILE}gcc
export CXX_COMPILER=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export OSS_LIB_DIR=$work_path/../../library/gnuarm-4.8.2_neon_ca9
elif [ "$tool_chain" = "1021" ];then
export CROSS_COMPILE="$work_path/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-"
export C_COMPILER=${CROSS_COMPILE}gcc
export CXX_COMPILER=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export OSS_LIB_DIR=$work_path/../../library/gnuarm-10.2.1_neon_ca9
fi

HARFBUZZ_LIB_ROOT=$OSS_LIB_DIR/harfbuzz/${harfbuzz_ver}
echo 'harfbuzz lib root is $HARFBUZZ_LIB_ROOT'

#step 2,build icu-le-hb
echo 'step 2,build icu-le-hb'
##################################################
#          icu-le-hb build steps:
##################################################
###0. download icu-le-hb tar ball in www.freedesktop.org/software/harfbuzz/release
cd $work_path/icu-le-hb-1.0.3
export CFLAGS=-O2\ $SECU_FLAGS
export CXXFLAGS=-std=gnu++11\ -O2\ $SECU_FLAGS
echo "tool_chain=$tool_chain"
	./configure CC="$C_COMPILER" CXX="$CXX_COMPILER" CXXFLAGS="-std=gnu++11 -O2 $SECU_FLAGS" LDFLAGS="-Wl,-rpath=$HARFBUZZ_LIB_ROOT/pre-install/lib $SECU_FLAGS" HARFBUZZ_CFLAGS="-I$HARFBUZZ_LIB_ROOT/pre-install/include/harfbuzz"  HARFBUZZ_LIBS="-L$HARFBUZZ_LIB_ROOT/pre-install/lib -lharfbuzz" ICU_CFLAGS="-I$icu_path/icu/pre-install/include" ICU_LIBS="$icu_path/icu/pre-install/lib -licuuc" --host=$HOST prefix=$(pwd)/pre-install
	echo "./configure CC="$C_COMPILER" CXX="$CXX_COMPILER" LDFLAGS="-Wl,-rpath=$HARFBUZZ_LIB_ROOT/pre-install/lib $SECU_FLAGS" HARFBUZZ_CFLAGS="-I$HARFBUZZ_LIB_ROOT/pre-install/include/harfbuzz"  HARFBUZZ_LIBS="-L$HARFBUZZ_LIB_ROOT/pre-install/lib -lharfbuzz" ICU_CFLAGS="-I$icu_path/icu/pre-install/include" ICU_LIBS="$icu_path/icu/pre-install/lib -licuuc" --host=$HOST prefix=$(pwd)/pre-install"

make clean
make
make install

#step 3,build icu-le-lx
echo 'step 3,build icu-le-lx'
##################################################
#          Build icu-le-lx steps:
##################################################
#1. build and install ICU(refer to icu_build_cmd.log)
#2. build and install harfbuzz(..code....harfbuzz.so/library/gnuarm-4.8.2_neon_ca9/harfbuzz/1.4.2/pre-install/lib)
#3. build and install the icu-le-hb library.(refer to icu-le-hb1.0.3_build_cmd.txt)
#	 icu-le-hb..icu-le...so.......icu-le-hb...harfbuzz....layout...harfbuzz.
#	 a). libicu-le-hb.so.0.0.0...library/gnuarm-4.8.2_neon_ca9/icu/58.2/lib.
#	 b). ln -fs libicu-le-hb.so.0.0.0 libicule.so.58.2
#4. set pkg-config,set pkg config path contain icu-le-hb's pkgconfig dir. .icu-le-hb.pkgconfig.....PKG_CONFIG_PATH(icu-le.pc.....)
#	 PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$(pwd)/pkgconfig
#5. rebuild ICU again with :
#	--disable-layout --enable-layoutex
#	ICULEHB_CFLAGS="-I/proj/mtk07391/ossbuild/icu/lehb/icu-le-hb-1.0.3/pre-install/include/icu-le-hb" (ICU59.1 need)
#	ICULEHB_LIBS="-L/proj/mtk07391/ossbuild/icu/lehb/icu-le-hb-1.0.3/pre-install/lib -licu-le-hb" (ICU59.1 need)

cd $work_path
cp icu-le-hb-1.0.3/pre-install/lib/libicu-le-hb.so.0.0.0 $icu_ver/icu/pre-install/lib
cp $work_path/icu-le-hb-1.0.3/pre-install/include/icu-le-hb/layout/*.h $icu_path/icu/source/layoutex/layout

echo "jing cp $work_path/icu-le-hb-1.0.3/pre-install/include/icu-le-hb/layout/*.h $icu_path/icu/source/layoutex/layout"
#INC_H = -I$work_path/icu-le-hb-1.0.3/pre-install/include/icu-le-hb

cd $icu_ver/icu/pre-install/lib
ln -fs libicu-le-hb.so.0.0.0 libicule.so.$icu_ver
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$work_path/icu-le-hb-1.0.3/pre-install/lib/pkgconfig

cd $work_path/$icu_ver/buildB

export CFLAGS=-O2\ $SECU_FLAGS
export CXXFLAGS=-O2\ $SECU_FLAGS
export LDFLAGS=-O2\ $SECU_FLAGS

sh ../icu/source/configure CC="ccache $C_COMPILER" CXX="ccache $CXX_COMPILER" ICULEHB_CFLAGS="-I$work_path/icu-le-hb-1.0.3/pre-install/include/icu-le-hb" ICULEHB_LIBS="-L$work_path/icu-le-hb-1.0.3/pre-install/lib -licu-le-hb" prefix=`pwd`/../icu/pre-install --host=$HOST --disable-layout --enable-layoutex CFLAGS="-O2 $SECU_FLAGS" CXXFLAGS="-O2 $SECU_FLAGS" --with-cross-build=`pwd`/../buildA;

cp $icu_path/icu/source/layoutex/layout/*.h $icu_path/icu/pre-install/include/layout/

chmod +x ../buildA/bin/icupkg
make
make install


cp -rf $icu_path/icu/pre-install/include/ ${OSS_LIB_DIR}/icu/${icu_ver}/
cp -rf $icu_path/icu/pre-install/lib/ ${OSS_LIB_DIR}/icu/${icu_ver}/

