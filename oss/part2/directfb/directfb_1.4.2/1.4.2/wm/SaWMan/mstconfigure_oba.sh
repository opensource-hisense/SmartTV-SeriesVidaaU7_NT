#!/bin/bash

# Chip
CHIP="titania2"

test -z $KERNEL_SRC && echo "  The KERNEL_SRC must be set to proceed!!" && exit 0

test -z $MST_PREFIX && echo "  The MST_PREFIX must be set to proceed!!" && exit 0

test -z $MST_SDK_PATH && echo "  The MST_SDK_PATH must be set to proceed!!" && exit 0

test -z $MST_SDK_CCID && echo "  The MST_SDK_CCID must be set to proceed!!" && exit 0

test -z $TOOL_CHAIN_PREFIX && export TOOL_CHAIN_PREFIX=mips2_fp_le-


echo "KERNEL_SRC=$KERNEL_SRC"
echo "MST_PREFIX=$MST_PREFIX"
echo "MST_SDK_PATH=$MST_SDK_PATH"
echo "MST_SDK_CCID=$MST_SDK_CCID"
echo "TOOL_CHAIN_PREFIX=$TOOL_CHAIN_PREFIX"

LD_LIB_PATH=" -L$MST_PREFIX/lib -L$MST_SDK_PATH/ROOTFS/lib -L$MST_SDK_PATH/opt/lib -L$MST_SDK_PATH/vendor/common/lib -L$MST_SDK_PATH/vendor/$MST_SDK_CCID/lib"

echo "LD_LIB_PATH=$LD_LIB_PATH"


echo "Running autoconf & automake"
autoreconf
autoconf
automake

# **********************************************
# Tool Chain
# **********************************************
CROSS_TOOL_PREFIX=$TOOL_CHAIN_PREFIX

if [ "$CROSS_TOOL_PREFIX" == "mips-linux-gnu-" ]; then
    CC=${CROSS_TOOL_PREFIX}"gcc -EL"
else
    CC=${CROSS_TOOL_PREFIX}"gcc"
fi

export CC


./configure	--host=mipsel-unknown-linux-gnu \
		--build=i386-linux \
		--prefix=$MST_PREFIX \
		CFLAGS="-I$MST_PREFIX/include -I$KERNEL_SRC/include -I$KERNEL_SRC/drivers/mstar/include -I$MST_SDK_PATH/conf -I$MST_SDK_PATH/vendor/common/include -I$MST_SDK_PATH/vendor/$MST_SDK_CCID/include -DMSOS_TYPE_LINUX" \
		LDFLAGS="$LD_LIB_PATH" \
		LIBS="-lrt -ldrvSYS -llinux -lmadp -ldbus-1 -lpthread" \
		MSTAR_UTOPIA_LIBS="-lapiGFX -lapiGOP -ldrvSERFLASH -ldrvVE -lapiXC -lapiPNL" \
		--enable-shared \
		--disable-static \
		--enable-multi \
		--without-tests \
		--disable-fbdev \
		--disable-x11 \
		--disable-vnc \
		--disable-sdl \
		--enable-devmem \
		--with-gfxdrivers="mstargfx" \
		--disable-text \
		--disable-network \
		--enable-jpeg \
		--enable-png \
		--enable-gif \
		--enable-freetype \
		--enable-MVF  \
		--enable-OBAMA_BUILD \
		--with-inputdrivers=linuxinput,mstarir \
		--enable-video4linux=yes
