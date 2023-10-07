#!/bin/sh

#PHOTOSPHERE_ROOT=""
#CHIP=monaco
#PROJ_MODE=europe_dtv
#if [ "$1" != "" ]; then
#    CHIP=$1
#fi



SHARED_LIB_PATH="$PHOTOSPHERE_ROOT/target/$PROJ_MODE.$CHIP/mslib"
DFB_NEEDED_PATH="$PHOTOSPHERE_ROOT/develop/include"
UTOPIA_LIB_PATH="$PHOTOSPHERE_ROOT/target/$PROJ_MODE.$CHIP/mslib/utopia/"

DESTDIR=`pwd`/../output
DFB_PATH="$DESTDIR/vendor"
PREFIX_PATH="/vendor"
echo "======= DFB needed library ======="
echo $PHOTOSPHERE_ROOT
echo $SHARED_LIB_PATH
echo $DFB_NEEDED_PATH
echo $UTOPIA_LIB_PATH
echo "=================================="

LD_LIB_PATH="-L$SHARED_LIB_PATH -L$UTOPIA_LIB_PATH -lz -lm"
CFLAGS_SETTING="-DARCH_X86=0 -DVISIBILITY_HIDDEN -DMSOS_PROCESS_SAFT_MUTEX -mhard-float -mthumb -mlittle-endian  -march=armv7-a -mcpu=cortex-a9 -mfpu=neon  -mfloat-abi=hard -w -fno-strict-aliasing -fno-optimize-sibling-calls -fno-exceptions -ffunction-sections -fdata-sections -O2 -pthread -I$DFB_NEEDED_PATH/png -I$DFB_NEEDED_PATH/zlib -I$DFB_NEEDED_PATH/jpeg -I$DFB_NEEDED_PATH/freetype/freetype2 -I$DFB_NEEDED_PATH/freetype -I$UTOPIA_INCLUDE_PATH -I$DFB_PATH -DMSOS_TYPE_LINUX"
CPPFLAGS_SETTING=" -DMSOS_PROCESS_SAFT_MUTEX -mlittle-endian -mhard-float -mthumb -march=armv7-a -mcpu=cortex-a9 -mfpu=neon  -mfloat-abi=hard -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -fno-strict-aliasing -fno-optimize-sibling-calls -fno-exceptions -ffunction-sections -fdata-sections -O2 -pthread "
DIRECTFB_CFLAGS_SETTING="-D_REENTRANT -I$DFB_PATH/include -I$DFB_PATH/include/directfb -I$DFB_PATH/include/directfb-internal"
DIRECTFB_LIBS_SETTING="-L$DFB_PATH/lib -ldirect -lfusion -ldirectfb -lpthread"

# **********************************************
# Reference Libs
# **********************************************
if [ "$CHIP" == "t2" ]; then
    MSTAR_LIB=" -lrt -lpthread -lapiGFX -lapiGOP -ldrvPQ -llinux -ldrvSERFLASH"
else
    MSTAR_LIB="-lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux -ldrvVE -lapiXC -lapiPNL -ldrvWDT -ldrvSAR -lapiSWI2C -ldrvGPIO -ldrvCPU -ldirect"
fi
HW_DECODEJPEG_LIBS=" -lapiVDEC -ldrvIPAUTH -lapiJPEG "
echo "Running autoconf & automake"
autoreconf --force --install
autoconf
automake

./configure     CC="armv7a-cros-linux-gnueabi-gcc -mlittle-endian" \
                LD="armv7a-cros-linux-gnueabi-ld -EL"\
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="-Wl,-rpath,/mslib -Wl,-rpath,/lib $LD_LIB_PATH" \
                CPPFLAGS="$CPPFLAGS_SETTING" \
                FREETYPE_CFLAGS="-I$DFB_NEEDED_PATH/freetype " \
                LIBS="$MSTAR_LIB $HW_DECODEJPEG_LIBS" \
                DIRECTFB_CFLAGS="$DIRECTFB_CFLAGS_SETTING"              \
                DIRECTFB_LIBS="$DIRECTFB_LIBS_SETTING"                  \
                --prefix=$PREFIX_PATH/directfb_examples                 \
                --host=armv7a-cros-linux-gnueabi \
                --build=i386-linux \
                --enable-debug
./fix_prefix.sh
