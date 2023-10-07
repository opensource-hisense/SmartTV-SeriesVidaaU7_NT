#!/bin/sh

#CHIP=a5
if [ "$1" != "" ]; then
    CHIP=$1
fi


#PHOTOSPHERE_ROOT="$HOME/P4/DAILEO/Supernova"
test -z $PHOTOSPHERE_ROOT && echo "the PHOTOSPHERE_ROOT must be set!!" && exit 0

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
CFLAGS_SETTING="-EL -pthread  -I$DFB_NEEDED_PATH/png -I$DFB_NEEDED_PATH/zlib -I$DFB_NEEDED_PATH/jpeg -I$DFB_NEEDED_PATH/freetype/freetype2 -I$DFB_NEEDED_PATH/freetype"
CPPFLAGS_SETTING="-EL"
DIRECTFB_CFLAGS_SETTING="-D_REENTRANT -I$DFB_PATH/include -I$DFB_PATH/include/directfb -I$DFB_PATH/include/directfb-internal"
DIRECTFB_LIBS_SETTING="-L$DFB_PATH/lib -lfusion -ldirectfb -lpthread -ldirect"

# **********************************************
# Reference Libs
# **********************************************
if [ "$CHIP" == "t2" ]; then
    MSTAR_LIB="-lrt -lpthread -lapiGFX -lapiGOP -ldrvPQ -llinux -ldrvSERFLASH"
elif [ "$CHIP" == "u4" ]; then
    MSTAR_LIB="-lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux -ldrvSERFLASH -ldrvSYS -ldrvVE -lapiXC -lapiPNL"
else
    MSTAR_LIB="-lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux -ldrvVE -lapiXC -lapiPNL"
fi
HW_DECODEJPEG_LIBS="-lapiVDEC -ldrvIPAUTH -lapiJPEG"

echo "Running autoconf & automake"
autoreconf
autoconf
automake

./configure     CC="mips-linux-gnu-gcc -EL"                             \
                CFLAGS="$CFLAGS_SETTING"                                \
                LDFLAGS="$LD_LIB_PATH"                                  \
                CPPFLAGS="$CPPFLAGS_SETTING"                            \
                FREETYPE_CFLAGS="-I$DFB_NEEDED_PATH/freetype"           \
                LIBS="$MSTAR_LIB $HW_DECODEJPEG_LIBS "                  \
                DIRECTFB_CFLAGS="$DIRECTFB_CFLAGS_SETTING"              \
                DIRECTFB_LIBS="$DIRECTFB_LIBS_SETTING"                  \
                --prefix=$PREFIX_PATH/directfb_examples                 \
                --host=mips-linux-gnu                                   \
                --build=i386-linux                                      \
                --enable-debug=yes

./fix_prefix.sh
