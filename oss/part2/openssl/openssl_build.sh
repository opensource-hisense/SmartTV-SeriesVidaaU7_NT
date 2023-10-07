#! /bin/sh

echo "enter $(pwd) to build openssl"

if [ $# == 0 ]; then
    echo "please input ./openssl_build.sh [toolchain version] [neon/vfp] [ca9/ca15]"
    exit 0
fi

export TOOL_CHAIN=$1
export CURRENT_PATH=$(pwd)

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
10.2.1)
    export TOOL_CHAIN_ROOT=$CURRENT_PATH"/../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1"
    export CROSS_COMPILE="arm-none-linux-gnueabihf"
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

export OPENSSL_VER="1.1.1q"
GZ_EXTEND=".tar.gz"

cd ${OPENSSL_VER}
tar -xzvf openssl-${OPENSSL_VER}${GZ_EXTEND}
if [ $? -ne 0 ]; then
    echo "xtar openssl error"
    exit -1;
fi

cd "openssl-"${OPENSSL_VER}

export CFLAGS="-fPIC "

make clean
make distclean

chmod -R u+w *

export SSL_INSTALL_DIR=${INSTALL_DIR}/openssl/${OPENSSL_VER}
echo "Install Dir $SSL_INSTALL_DIR"
if [ ! -d "$SSL_INSTALL_DIR" ]; then
    mkdir -p "$SSL_INSTALL_DIR"
    echo "Install Dir creat"
fi

CROSS_COMPILE=${CROSS_COMPILE}- \
./Configure linux-generic32 no-asm no-camellia no-mdc2 no-idea no-hw no-rc5 shared no-ssl3 no-ssl2 --prefix=${SSL_INSTALL_DIR} \
-fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s


make depend
make
MAKE_RET=$?
if [ $MAKE_RET -ne 0 ]; then
    echo "OPENSSL Build Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "OPENSSL Build OK..."
fi

make install
if [ $MAKE_RET -ne 0 ]; then
    echo "OPENSSL Install Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "OPENSSL Install OK..."
fi

exit 0
