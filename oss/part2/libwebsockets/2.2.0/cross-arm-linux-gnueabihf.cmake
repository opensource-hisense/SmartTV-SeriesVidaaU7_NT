#
# CMake Toolchain file for crosscompiling on ARM.
#
# This can be used when running cmake in the following way:
#  cd build/
#  cmake .. -DCMAKE_TOOLCHAIN_FILE=../cross-arm-linux-gnueabihf.cmake
#

set(CROSS_PATH /mtkoss/gnuarm/neon_4.8.2_2.6.35_cortex-a9-ubuntu/i686)

# Target operating system name.
set(CMAKE_SYSTEM_NAME Linux)

# Name of C compiler.
set(CMAKE_C_COMPILER "${CROSS_PATH}/bin/armv7a-mediatek482_001_neon-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILER "${CROSS_PATH}/bin/armv7a-mediatek482_001_neon-linux-gnueabi-g++")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH "${CROSS_PATH}")

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only.
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search headers and libraries in the target environment only.
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set Zlib
set(ZLIB_LIBRARY $ENV{ZLIB_LIB_DIR}/lib/libz.so)
set(ZLIB_INCLUDE_DIR $ENV{ZLIB_LIB_DIR}/include)

#set(OPENSSL_ROOT_DIR helloroot)
#set(OPENSSL_LIBRARIES $ENV{OPENSSL_LIB_DIR}/lib)
#set(LWS_OPENSSL_LIBRARIES $ENV{OPENSSL_LIB_DIR}/lib)
set(OPENSSL_INCLUDE_DIR $ENV{OPENSSL_LIB_DIR}/include)
set(OPENSSL_CRYPTO_LIBRARY $ENV{OPENSSL_LIB_DIR}/lib/libcrypto.so)
set(OPENSSL_SSL_LIBRARY $ENV{OPENSSL_LIB_DIR}/lib/libssl.so)
set(OPENSSL_EXECUTABLE $ENV{OPENSSL_LIB_DIR}/bin/openssl)
#
