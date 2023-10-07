#!/bin/bash
TOOL_CHAIN_PREFIX="${CROSS_COMPILE}"
CFLAGS_SETTINGS="${MSTAR_CFLAG}"
LDFLAGS_SETTINGS="${MSTAR_LDFLAG} -lz -lm"
HOST="${CROSS_COMPILE/%-/}"
MIPS_FLAGS="-DVISIBILITY_HIDDEN -EL -pthread -DMSOS_TYPE_LINUX  -DPAGESIZE_LOG=12"
ARM_FLAGS="-DVISIBILITY_HIDDEN -DMSOS_PROCESS_SAFT_MUTEX -mcpu=cortex-a9 -mfpu=neon -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -fno-strict-aliasing -fno-optimize-sibling-calls -fno-exceptions -ffunction-sections -fdata-sections -O2 -fno-peephole2 -pthread -DMSOS_TYPE_LINUX"
BUILD="i386-linux"

test -z "$MSTAR_CFLAG" && echo "  Please use \"source /tools/ToolChainSetting [toolchain]\" to set toolchain!" && exit 0

function configure()
{
test -z "$MST_PREFIX" && echo "  The MST_PREFIX must be set to proceed!!" && exit 0
echo "MST_PREFIX=$MST_PREFIX"

if [ "$MVK_VERSION" != "RELEASE" ]; then
    CFLAGS_SETTINGS="${CFLAGS_SETTINGS} ${DEBUG_FLAGS}"
fi

if [[ ${TOOL_CHAIN_PREFIX} =~ "mips" ]]
then
CFLAGS_SETTINGS="${CFLAGS_SETTINGS} ${MIPS_FLAGS}"
_CC_="${TOOL_CHAIN_PREFIX}gcc -EL -fvisibility=hidden"
else
CFLAGS_SETTINGS="${CFLAGS_SETTINGS} ${ARM_FLAGS}"
_CC_="${TOOL_CHAIN_PREFIX}gcc -mlittle-endian -fvisibility=hidden"
fi

echo $CFLAGS_SETTING | grep VISIBILITY_HIDDEN


echo "CC=$_CC_"

MSTAR_LIB=" -lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux  -ldrvVE -lapiXC -lapiPNL -ldrvWDT -ldrvSAR -lapiSWI2C -ldrvGPIO -ldrvCPU "
HW_DECODEJPEG_LIBS=" -lapiVDEC -ldrvIPAUTH -lapiJPEG "
HW_DECODEGIF_LIBS="-lapiGPD"

echo "Running autoconf & automake"
libtoolize -f
autoreconf
autoconf
automake

./configure     CC="$_CC_" \
                LD="${TOOL_CHAIN_PREFIX}ld -EL"\
                CFLAGS="-I$MST_PREFIX/include -I$MST_PREFIX/include/freetype2 $CFLAGS_SETTINGS" \
                LDFLAGS="-L$MST_PREFIX/lib $LDFLAGS_SETTINGS" \
                LIBS="$MSTAR_LIB $HW_DECODEJPEG_LIBS" \
                FREETYPE_CFLAGS="-I$MST_PREFIX/include" \
                FREETYPE_LIBS="-lfreetype -lz" \
                --prefix=$MST_PREFIX \
                --build=$BUILD \
                --host=$HOST \
                --sysconfdir=/config \
                --enable-jpeg \
                --enable-zlib \
                --enable-png \
                --enable-gif \
                --enable-gopc \
                --enable-vec \
                --enable-freetype \
                --disable-debug \
                --disable-profiling \
                --disable-unique \
                --with-gfxdrivers=mstargfx,mstargfx_g2 \
                --with-inputdrivers=mstarlinuxinput,mstarir,mstarkeypad,mstarloopbackinput \
                --enable-shared \
                --enable-static \
                --enable-multi \
                --with-tests=yes \
                --disable-fbdev \
                --disable-x11 \
                --disable-vnc \
                --disable-sdl \
                --enable-devmem \
                --enable-ion \
                --disable-pmem \
                --enable-hwdecodejpeg\
                --enable-hwdecodepng\
                --disable-OBAMA_BUILD
}


function build()
{
make clean
make
}

function install()
{
make install
}

LIBRARY_NAME="directfb"
LIBRARY_VERSION="1.4.2m"
PACKAGE_NAME="${LIBRARY_NAME}-${LIBRARY_VERSION}"
PACKAGE_PATH=../../

function package()
{
    if [ "$1" != "" ]; then
        PACKAGE_PATH=$1
    fi
    echo "PACKAGE_PATH=$PACKAGE_PATH"

    RETURN_PATH=`pwd`
    cd $PACKAGE_PATH
    mkdir -p $PACKAGE_NAME/bin
    mkdir -p $PACKAGE_NAME/include
    mkdir -p $PACKAGE_NAME/lib/pkgconfig
    mkdir -p $PACKAGE_NAME/share/man/man1
    mkdir -p $PACKAGE_NAME/share/man/man5
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/libdirect-1.4.so.0.2.0
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/libdirectfb-1.4.so.0.2.0
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/libfusion-1.4.so.0.2.0
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/gfxdrivers/libdirectfb_mstar_g2.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/gfxdrivers/libdirectfb_mstar.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/inputdrivers/libdirectfb_mstar_linux_input.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/inputdrivers/libdirectfb_mstar_loopback_input.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/inputdrivers/libdirectfb_mstarir.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/inputdrivers/libdirectfb_mstarkeypad.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/wm/libdirectfbwm_default.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/systems/libdirectfb_devmem.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/systems/libdirectfb_pmem.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBFont/libidirectfbfont_default.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBFont/libidirectfbfont_dgiff.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBFont/libidirectfbfont_ft2.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBFont/libidirectfbfont_mstarbmp.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_dfiff.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gif.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_jpeg.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gopc.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_mif.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_png.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_vec.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gif.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gopc.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_v4l.so
    ${TOOL_CHAIN_PREFIX}objcopy --add-section .mmodule_version=$RETURN_PATH/version_info $MST_PREFIX/lib/directfb-1.4-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_vec.so
    cp -vrfP $MST_PREFIX/directfb_examples $PACKAGE_NAME
    cp -vrfP $MST_PREFIX/bin/dfb* $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/bin/direct* $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/bin/directfb* $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/bin/fusion* $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/bin/mkdfiff $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/bin/mkdgiff $PACKAGE_NAME/bin
    cp -vrfP $MST_PREFIX/include/directfb $PACKAGE_NAME/include
    cp -vrfP $MST_PREFIX/include/directfb-internal $PACKAGE_NAME/include
    cp -vrfP $MST_PREFIX/lib/directfb-1.4-0 $PACKAGE_NAME/lib
    cp -vrfP $MST_PREFIX/lib/libdirect* $PACKAGE_NAME/lib
    cp -vrfP $MST_PREFIX/lib/libfusion* $PACKAGE_NAME/lib
    cp -vrfP $MST_PREFIX/lib/pkgconfig/direct* $PACKAGE_NAME/lib/pkgconfig
    cp -vrfP $MST_PREFIX/lib/pkgconfig/fusion* $PACKAGE_NAME/lib/pkgconfig
    cp -vrfP $MST_PREFIX/share/directfb-* $PACKAGE_NAME/share
    cp -vrfP $MST_PREFIX/share/man/man1/dfb* $PACKAGE_NAME/share/man/man1
    cp -vrfP $MST_PREFIX/share/man/man1/directfb* $PACKAGE_NAME/share/man/man1
    cp -vrfP $MST_PREFIX/share/man/man5/directfb* $PACKAGE_NAME/share/man/man5
    tar -zvcf $PACKAGE_NAME.tar.gz $PACKAGE_NAME
    rm -rf $PACKAGE_NAME
    cd $RETURN_PATH
}

case $1 in
"package")
    echo "################  pacakage $LIBRARY_NAME"
    package $2
    ;;
"make")
    echo "################  building $LIBRARY_NAME"
    build
    ;;
"install")
    echo "################  installing $LIBRARY_NAME"
    install
    ;;
*)
    echo "################  configuring $LIBRARY_NAME"
    configure
    ;;
esac
