#!/bin/sh


DFB_PATH=`pwd`
DEPENDENCY_LIB_ROOT="$DFB_PATH/DependencyLib/tmp"


SHARED_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib"
DFB_NEEDED_PATH="$DEPENDENCY_LIB_ROOT/include"
UTOPIA_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib/utopia/"
UTOPIA_INCLUDE_PATH="$DEPENDENCY_LIB_ROOT/include/utopia"


DFB_PATH="$DFB_PATH/output/vendor"
PREFIX_PATH="/vendor"


cd examples

echo "======= DFB needed library ======="
echo $DFB_PATH
echo $DEPENDENCY_LIB_ROOT
echo $SHARED_LIB_PATH
echo $DFB_NEEDED_PATH
echo $UTOPIA_LIB_PATH
echo $UTOPIA_INCLUDE_PATH
echo "=================================="


LD_LIB_PATH+=" -L$SHARED_LIB_PATH"
LD_LIB_PATH+=" -L$UTOPIA_LIB_PATH -lz -lm"


CFLAGS_SETTING+=" -EL"
CFLAGS_SETTING+=" -pthread"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/png"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/zlib"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/jpeg"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype/freetype2"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype"


CPPFLAGS_SETTING+=" -EL"


DIRECTFB_CFLAGS_SETTING+=" -D_REENTRANT"
DIRECTFB_CFLAGS_SETTING+=" -I$DFB_PATH/include"
DIRECTFB_CFLAGS_SETTING+=" -I$DFB_PATH/include/directfb"
DIRECTFB_CFLAGS_SETTING+=" -I$DFB_PATH/include/directfb-internal"


DIRECTFB_LIBS_SETTING+=" -L$DFB_PATH/lib"
DIRECTFB_LIBS_SETTING+=" -lfusion"
DIRECTFB_LIBS_SETTING+=" -ldirectfb"
DIRECTFB_LIBS_SETTING+=" -lpthread"
DIRECTFB_LIBS_SETTING+=" -ldirect"


# **********************************************
# Reference Libs
# **********************************************
MSTAR_LIB+=" -lrt"
MSTAR_LIB+=" -lpthread"
MSTAR_LIB+=" -ldrvMVOP" 
MSTAR_LIB+=" -lapiGFX"
MSTAR_LIB+=" -lapiGOP"
MSTAR_LIB+=" -llinux"
MSTAR_LIB+=" -ldrvVE"
MSTAR_LIB+=" -lapiXC"
MSTAR_LIB+=" -lapiPNL"


HW_DECODEJPEG_LIBS+=" -lapiVDEC"
HW_DECODEJPEG_LIBS+=" -ldrvIPAUTH" 
HW_DECODEJPEG_LIBS+=" -lapiJPEG"


HW_DECODEGIF_LIBS="-lapiGPD"


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
                --disable-debug

./fix_prefix.sh
