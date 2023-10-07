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

rm -rf $icu_path/buildA $icu_path/buildB $icu_path/icu/pre-install
#rm -rf $icu_path/../icu-le-hb-1.0.3/src/*.o $icu_path/../icu-le-hb-1.0.3/src/*.so $icu_path/../icu-le-hb-1.0.3/pre-install $icu_path/../icu-le-hb-1.0.3/src/.libs
if [ "clean" == $2 ];then
	exit
fi

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
elif [ "$tool_chain" = "482" ];then
export CROSS_COMPILE=/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/i686/bin/armv7a-mediatek482_001_neon-linux-gnueabi-
export C_COMPILER=${CROSS_COMPILE}gcc
export CXX_COMPILER=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
elif [ "$tool_chain" = "1021" ];then
export CROSS_COMPILE="$work_path/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-"
export C_COMPILER=${CROSS_COMPILE}gcc
export CXX_COMPILER=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
fi

sh ../icu/source/configure CC="ccache $C_COMPILER" CXX="ccache $CXX_COMPILER" \
							prefix=`pwd`/../icu/pre-install \
							--host=$HOST \
							CFLAGS="-O2 $SECU_FLAGS" \
							CXXFLAGS="-O2 $SECU_FLAGS" \
							--with-cross-build=`pwd`/../buildA;

chmod +x ../buildA/bin/icupkg;
echo "CXXFLAGS=$CXXFLAGS"
echo "CFLAGS=$CFLAGS"
echo "LDFLAGS=$LDFLAGS"

make;
make install
