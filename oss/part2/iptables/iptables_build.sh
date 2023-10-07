#! /bin/sh

echo "enter $(pwd) to build iptables"

if [ $# == 0 ]; then
    echo "please input ./iptables_build.sh [toolchain version] [neon/vfp] [ca9/ca15]"
    exit 0
fi

export TOOL_CHAIN=$1

case ${TOOL_CHAIN} in
4.8.2)
    if [ $2 = "vfp" ];then
        export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/vfp_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64"
    elif [ $3 = "ca9" ];then
        export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/x86_64"
    elif [ $3 = "ca15" ];then
        export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a15-ubuntu/x86_64"
    else
        echo "$2 $3 can't find"
        exit 0
    fi
    export CROSS_COMPILE="armv7a-mediatek482_001_$2-linux-gnueabi"
    ;;
4.9.1)
    export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/hard_4.9.1_armv7a-cros/x86_64/armv7a"
    export CROSS_COMPILE="armv7a-cros-linux-gnueabi"
    ;;
4.9.2)
    export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/hard_4.9.2-r116_armv7a-cros/x86_64/armv7a"
    export CROSS_COMPILE="armv7a-cros-linux-gnueabi"
    ;;
5.5.0)
    export TOOL_CHAIN_ROOT="/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64"
    export CROSS_COMPILE="arm-linux-gnueabi"
    ;;
*)
    echo "can't find ${TOOL_CHAIN} version"
    ;;
esac

if [ $2 = neon ];then
    export FLAG1="neon"
elif [ $2 = vfp ];then
    export FLAG1="vfp"
else
    export FLAG1="neon"
fi
if [ $3 = ca9 ];then
    export FLAG2="ca9"
elif [ $3 = ca15 ];then
    export FLAG2="ca15"
else
    export FLAG2="ca9"
fi

export TOOL_CHAIN_BIN_PATH=${TOOL_CHAIN_ROOT}"/bin"
export PATH=${TOOL_CHAIN_BIN_PATH}":$PATH"

export INSTALL_DIR=$(pwd)"/../../library/gnuarm-${TOOL_CHAIN}_${FLAG1}_${FLAG2}/"
echo $INSTALL_DIR

export IPTABLES_VER="1.6.0"
BZ2_EXTEND=".tar.bz2"

tar -xjvf iptables-${IPTABLES_VER}${BZ2_EXTEND}
if [ $? -ne 0 ]; then
    echo "xtar iptables error"
    exit -1;
fi

cd iptables-${IPTABLES_VER}

make clean
make distclean

export IPTABLES_INSTALL_DIR=${INSTALL_DIR}/iptables/${IPTABLES_VER}
echo "Install Dir $IPTABLES_INSTALL_DIR"
if [ ! -d "$IPTABLES_INSTALL_DIR" ]; then
    mkdir -p "$IPTABLES_INSTALL_DIR"
    echo "Install Dir creat"
fi

export CFLAGS="${CFLAGS} -fstack-protector -D_FORIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie"

./configure --enable-static --disable-shared --disable-largefile --disable-nftables \
--prefix=$(pwd)/builddir \
--host=${CROSS_COMPILE}

make \
CC=${CROSS_COMPILE}-gcc
MAKE_RET=$?
if [ $MAKE_RET -ne 0 ]; then
    echo "Iptables Build Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "Iptables Build OK..."
fi

make install
if [ $MAKE_RET -ne 0 ]; then
    echo "Iptables Install Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "Iptables Install OK..."
fi

cp builddir/sbin/* ${IPTABLES_INSTALL_DIR}/iptables-${IPTABLES_VER}/

exit 0
