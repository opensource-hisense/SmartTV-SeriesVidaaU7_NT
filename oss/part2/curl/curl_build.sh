#! /bin/sh

echo "enter $(pwd) to build curl"

if [ $# == 0 ]; then
    echo "please input ./curl_build.sh [toolchain version] [neon/vfp] [ca9/ca15]"
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

export ZLIB_VER="1.2.12"
export CARES_VER="1.18.1"
export OPENSSL_VER="1.1.1q"
export NGHTTP2_VER="1.48.0"
export CURL_VER="7.86.0"
GZ_EXTEND=".tar.gz"

cd curl-${CURL_VER}
tar -xzvf curl-${CURL_VER}${GZ_EXTEND}
if [ $? -ne 0 ]; then
    echo "xtar curl error"
    exit -1;
fi

cd curl-${CURL_VER}

export ZLIB_INSTALL_DIR=${INSTALL_DIR}/zlib/${ZLIB_VER}/pre-install
export ARES_INSTALL_DIR=${INSTALL_DIR}/c-ares/${CARES_VER}
export SSL_INSTALL_DIR=${INSTALL_DIR}/openssl/${OPENSSL_VER}
export NGHTTP_INSTALL_DIR=${INSTALL_DIR}/nghttp2/${NGHTTP2_VER}

export LDFLAGS="-L${ZLIB_INSTALL_DIR}/lib -L${SSL_INSTALL_DIR}/lib -L${ARES_INSTALL_DIR}/lib -L${NGHTTP_INSTALL_DIR}/lib"
echo $LDFLAGS
export LIBS="-lcares -lssl -lcrypto -lz -lnghttp2"
export CPPFLAGS="-DDNS_QUERY_OPTION -I${ZLIB_INSTALL_DIR}/include -I${SSL_INSTALL_DIR}/include -I${ARES_INSTALL_DIR}/include -I${NGHTTP_INSTALL_DIR}/include"
export export CFLAGS="${CFLAGS} -fPIC"
export CFLAGS="${CFLAGS} -fstack-protector -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s "
export CPPFLAGS="${CPPFLAGS} -D_FORTIFY_SOURCE=2 "



make clean
make distclean

export CURL_INSTALL_DIR=${INSTALL_DIR}/curl/${CURL_VER}
echo "Install Dir $CURL_INSTALL_DIR"
if [ ! -d "$CURL_INSTALL_DIR" ]; then
    mkdir -p "$CURL_INSTALL_DIR"
    echo "Install Dir creat"
fi

export CC=${CROSS_COMPILE}-gcc

./configure \
--host=${CROSS_COMPILE}  \
--enable-shared \
--build=`uname -m`-linux \
--enable-debug \
--enable-ares \
--enable-ipv6 \
--disable-dict \
--disable-telnet \
--enable-file \
--disable-tftp \
--disable-ldap \
--disable-ftp \
--disable-sspi \
--disable-dependency-tracking \
--enable-thread \
--without-libidn \
--with-pic  \
--disable-manual \
--disable-verbose \
--without-libssh2 \
--with-random=dev/urandom \
--with-nghttp2=${NGHTTP_INSTALL_DIR} \
--with-openssl \
--prefix=${CURL_INSTALL_DIR}


make
MAKE_RET=$?
if [ $MAKE_RET -ne 0 ]; then
    echo "LIBCURL Build Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "LIBCURL Build OK..."
fi

make install
if [ $MAKE_RET -ne 0 ]; then
    echo "LIBCURL Install Fail....($MAKE_RET)"
    exit $MAKE_RET
else
    echo "LIBCURL Install OK..."
fi

exit 0
