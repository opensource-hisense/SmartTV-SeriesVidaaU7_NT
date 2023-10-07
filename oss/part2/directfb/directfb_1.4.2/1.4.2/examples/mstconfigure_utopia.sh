#!/bin/bash
test -z $MST_PREFIX && echo "  The MST_PREFIX must be set to proceed!!" && exit 0
test -z $MST_SDK_PATH && echo "  The MST_SDK_PATH must be set to proceed!!" && exit 0
test -z $TOOL_CHAIN_PREFIX && export TOOL_CHAIN_PREFIX=mips2_fp_le-

echo "MST_PREFIX=$MST_PREFIX" 
echo "MST_SDK_PATH=$MST_SDK_PATH"
echo "TOOL_CHAIN_PREFIX=$TOOL_CHAIN_PREFIX" 

LD_LIB_PATH="-Wl,--rpath-link -Wl,$MST_SDK_PATH/ROOTFS/lib -L$MST_PREFIX/lib"

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
		--prefix=$MST_PREFIX/test \
		CFLAGS="-I$MST_PREFIX/include -msoft-float -pthread" \
		LDFLAGS="$LD_LIB_PATH" \
		LIBS=""
