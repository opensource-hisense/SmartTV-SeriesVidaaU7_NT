#/bin/sh
PWD=`pwd`
GOOGLE_TEST_VERSION=googletest-1.7.0
SOURCE=$PWD/
OUTPUT=$SOURCE/build

CMAKE_PATH=/mtkoss/cmake/3.9.3-linux/x86_64/bin

$CMAKE_PATH/cmake $SOURCE -Dgtest_build_tests=ON -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DBUILD_SHARED_LIBS=ON  -DCMAKE_TOOLCHAIN_FILE=$SOURCE/toolchain-config

make all

