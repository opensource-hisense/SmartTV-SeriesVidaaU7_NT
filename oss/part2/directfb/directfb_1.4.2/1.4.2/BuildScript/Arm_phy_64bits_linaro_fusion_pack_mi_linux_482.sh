#!/bin/sh

libtoolize -f

#Please modify utopia lib version

CHIP_CPU="ARM"
PHY_BITS="64"
KERNEL_TYPE="pack"

UTOPIA_LIB_VERSION="phy_"$PHY_BITS"bits_"$KERNEL_TYPE

rm -rf ./DependencyLib/tmp
mkdir ./DependencyLib/tmp
tar -zxvf ./DependencyLib/$CHIP_CPU/$UTOPIA_LIB_VERSION.tar.gz -C ./DependencyLib/tmp
tar -xzv -C ./DependencyLib/tmp/lib/utopia -f ./DependencyLib/$CHIP_CPU/mi.tar.gz --wildcards "lib*"
tar -xzv -C ./DependencyLib/tmp/include -f ./DependencyLib/$CHIP_CPU/mi.tar.gz --wildcards "*.h"
tar -xzv -C ./DependencyLib/tmp/lib/utopia -f ./DependencyLib/$CHIP_CPU/mi_mtk_482.tar.gz --wildcards "lib*"

DFB_PATH=`pwd`
export DEPENDENCY_LIB_ROOT="$DFB_PATH/DependencyLib/tmp"

if [ -d $DFB_PATH/../../../../misdk/mi/mi/modules ]; then
rm -rf $DEPENDENCY_LIB_ROOT/include/mi_*.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/cap/include/mi_cap.h $DEPENDENCY_LIB_ROOT/include/mi_cap.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/include/mi_common.h $DEPENDENCY_LIB_ROOT/include/mi_common.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/disp/include/mi_disp.h $DEPENDENCY_LIB_ROOT/include/mi_disp.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/fs/include/mi_fs.h $DEPENDENCY_LIB_ROOT/include/mi_fs.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/imgdec/include/mi_imgdec.h $DEPENDENCY_LIB_ROOT/include/mi_imgdec.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/os/include/mi_os.h $DEPENDENCY_LIB_ROOT/include/mi_os.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/osd/include/mi_osd.h $DEPENDENCY_LIB_ROOT/include/mi_osd.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/sar/include/mi_sar.h $DEPENDENCY_LIB_ROOT/include/mi_sar.h
ln -s $DFB_PATH/../../../../misdk/mi/mi/modules/sys/include/mi_sys.h $DEPENDENCY_LIB_ROOT/include/mi_sys.h
fi

#MST_PREFIX=/vendor
MTK_PREFIX=

export DESTDIR=`pwd`/output

function build()
{
test -z $DFB_PATH && echo "the DFB_PATH must be set!!" && exit 0
test -z $DEPENDENCY_LIB_ROOT && echo "the DEPENDENCY_LIB_ROOT must be set!!" && exit 0


SHARED_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib"
DFB_NEEDED_PATH="$DEPENDENCY_LIB_ROOT/include"
UTOPIA_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib/utopia/"
UTOPIA_INCLUDE_PATH="$DEPENDENCY_LIB_ROOT/include/utopia"

echo "=========== DFB needed library ==========="
echo DFB_PATH            : $DFB_PATH
echo UTOPIA_LIB_VERSION  : $UTOPIA_LIB_VERSION
echo SHARED_LIB_PATH     : $SHARED_LIB_PATH
echo DFB_NEEDED_PATH     : $DFB_NEEDED_PATH
echo UTOPIA_LIB_PATH     : $UTOPIA_LIB_PATH
echo UTOPIA_INCLUDE_PATH : $UTOPIA_INCLUDE_PATH
echo "=========================================="


LD_LIB_PATH+=" -L$SHARED_LIB_PATH"
LD_LIB_PATH+=" -L$UTOPIA_LIB_PATH -lz -lm -lstdc++"

CFLAGS_SETTING+=" ${MSTAR_CFLAG}"
CFLAGS_SETTING+=" -DDIRECTFB_CHANGE_ID=\\\"`git log --pretty=format:"ID:[%h]" -1`\\\""
CFLAGS_SETTING+=" -DVISIBILITY_HIDDEN"
CFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX"
CFLAGS_SETTING+=" -mlittle-endian"
CFLAGS_SETTING+=" -march=armv7-a"
CFLAGS_SETTING+=" -mcpu=cortex-a9"
CFLAGS_SETTING+=" -mtune=cortex-a9"
CFLAGS_SETTING+=" -mfpu=neon-vfpv4"
CFLAGS_SETTING+=" -fPIC"
CFLAGS_SETTING+=" -fvisibility=default"
CFLAGS_SETTING+=" -mfpu=neon"
CFLAGS_SETTING+=" -mfloat-abi=softfp"
CFLAGS_SETTING+=" -w"
CFLAGS_SETTING+=" -fno-strict-aliasing"
CFLAGS_SETTING+=" -fno-optimize-sibling-calls"
CFLAGS_SETTING+=" -fno-exceptions"
CFLAGS_SETTING+=" -ffunction-sections"
CFLAGS_SETTING+=" -fdata-sections"
CFLAGS_SETTING+=" -O2"
CFLAGS_SETTING+=" -fno-peephole2"
CFLAGS_SETTING+=" -pthread"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/png"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/zlib"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/jpeg"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype/freetype2"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype"
CFLAGS_SETTING+=" -I$UTOPIA_INCLUDE_PATH"
CFLAGS_SETTING+=" -I$DFB_PATH -DMSOS_TYPE_LINUX"
CFLAGS_SETTING+=" -DMI_ENABLE_DBG=0"

echo $CFLAGS_SETTING | grep VISIBILITY_HIDDEN

CPPFLAGS_SETTING+=" ${MSTAR_CPPFLAG}"
CPPFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX"
#CPPFLAGS_SETTING+=" -mlittle-endian"
#CPPFLAGS_SETTING+=" -march=armv7-a"
#CPPFLAGS_SETTING+=" -mcpu=cortex-a9"
#CPPFLAGS_SETTING+=" -mtune=cortex-a9"
#CPPFLAGS_SETTING+=" -mfpu=neon-vfpv4"
#CPPFLAGS_SETTING+=" -fPIC"
#CPPFLAGS_SETTING+=" -fvisibility=default"
#CPPFLAGS_SETTING+=" -mfpu=neon"
#CPPFLAGS_SETTING+=" -mfloat-abi=softfp"
CPPFLAGS_SETTING+=" -Wall"
CPPFLAGS_SETTING+=" -Wpointer-arith"
CPPFLAGS_SETTING+=" -Wstrict-prototypes"
CPPFLAGS_SETTING+=" -Winline"
CPPFLAGS_SETTING+=" -Wundef"
CPPFLAGS_SETTING+=" -fno-strict-aliasing"
CPPFLAGS_SETTING+=" -fno-optimize-sibling-calls"
CPPFLAGS_SETTING+=" -fno-exceptions"
CPPFLAGS_SETTING+=" -ffunction-sections"
CPPFLAGS_SETTING+=" -fdata-sections"
#CPPFLAGS_SETTING+=" -O2"
CPPFLAGS_SETTING+=" -fno-peephole2"
CPPFLAGS_SETTING+=" -pthread"


if [ $? == 0 ];  then
    _CC_="armv7a-mediatek482_001_neon-linux-gnueabi-gcc -mlittle-endian"
else
    _CC_="armv7a-mediatek482_001_neon-linux-gnueabi-gcc -mlittle-endian"
fi


# **********************************************
# Reference Libs
# **********************************************
MSTAR_LIB+=" -lfreetype"
MSTAR_LIB+=" -lrt"
MSTAR_LIB+=" -lpthread"

#**********************************************
# For MI link UND symbol
#**********************************************
MSTAR_LIB+=" -lmi"


echo "Running autoconf & automake"
autoreconf
autoconf
automake

./configure     CC="$_CC_" \
                LD="armv7a-mediatek482_001_neon-linux-gnueabi-ld -EL"\
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="-Wl,-rpath,/lib -Wl,-rpath,/vendor/lib/utopia -Wl,-rpath,/vendor/lib -Wl,-rpath,/vendor/lib/directfb -Wl,-rpath-link,$UTOPIA_LIB_PATH  $LD_LIB_PATH" \
                CPPFLAGS="$CPPFLAGS_SETTING" \
                FREETYPE_CFLAGS="-I$DFB_NEEDED_PATH/freetype " \
                FREETYPE_LIBS="-lfreetype -lz" \
                LIBS="$MSTAR_LIB" \
                --prefix=$MTK_PREFIX \
                --sysconfdir=/config \
                --host=armv7a-mediatek482_001_neon-linux-gnueabi \
                --build=i386-linux \
                --enable-jpeg \
                --enable-zlib \
                --enable-png \
                --enable-gif \
                --enable-gopc \
                --disable-vec \
                --enable-freetype \
                --disable-debug \
                --disable-profiling \
                --disable-unique \
                --with-gfxdrivers=mstargfx_mi \
                --with-inputdrivers=mstarlinuxinput,mstarir,mstarloopbackinput,mikeypad \
                --enable-shared \
                --disable-static \
                --enable-multi \
                --with-tests=yes \
                --disable-fbdev \
                --disable-x11 \
                --disable-vnc \
                --disable-sdl \
                --enable-devmem \
                --disable-pmem \
                --enable-hwdecodejpeg \
                --disable-hwdecodegif \
                --enable-imageprovider-mi \
                --enable-mstarmi \
                --enable-hwdecodepng

#sed -i 's/ECHO/echo/g' libtool
}

LIBRARY_NAME="directfb"
LIBRARY_VERSION="1.4.2m"
PACKAGE_NAME="${LIBRARY_NAME}-${LIBRARY_VERSION}"
PACKAGE_PATH=../../


export OUTPUT_DIR="${DESTDIR}/${MTK_PREFIX}"
echo $OUTPUT_DIR
echo $MTK_PREFIX

case $1 in
"package")
    echo "################  pacakage $LIBRARY_NAME"
        package $2
        ;;
*)
    echo "################  building $LIBRARY_NAME"
        build $1
        ;;
esac

