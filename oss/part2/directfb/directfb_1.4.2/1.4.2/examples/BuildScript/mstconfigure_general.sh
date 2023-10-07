#!/bin/sh


test -z $DEPENDENCY_LIB_ROOT && echo "Error!!! Please source dfb1.4.2/BuildScript/xxx.sh first if DFB LIB has been built. Or build DFB LIB first." && return

test -z $CROSS_COMPILE && echo "Error!!! Please source /tools/ToolChainSetting first." && return

test -z $OUTPUT_DIR && echo "Error!!! Please export the env OUTPUT_DIR in dfb1.4.2/BuildScript/xxx.sh first." && return

DFB_PATH=$OUTPUT_DIR
UTOPIA_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib/utopia"
DESTDIR=`pwd`/output


echo "======= DFB example dependency ======="
echo DFB    Path : $DFB_PATH
echo Utopia Path : $UTOPIA_LIB_PATH
echo Output Path : $DESTDIR
echo "=================================="


CFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX "
CFLAGS_SETTING+=" -DMSOS_TYPE_LINUX "
CFLAGS_SETTING+=" $MSTAR_CFLAG "
CFLAGS_SETTING+=" $MSTAR_CFLAG_OPT "
CFLAGS_SETTING+=" -Wformat=2 -Wformat-security -fstack-protector -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie "


CPPFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX "
CPPFLAGS_SETTING+=" -DMSOS_TYPE_LINUX "
CPPFLAGS_SETTING+=" $MSTAR_CPPFLAG "
CPPFLAGS_SETTING+=" -Wformat=2 -Wformat-security -fstack-protector -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie "


DIRECTFB_CFLAGS_SETTING="-D_REENTRANT -I$DFB_PATH/include -I$DFB_PATH/include/directfb -I$DFB_PATH/include/directfb-internal"
DIRECTFB_LIBS_SETTING="-L$UTOPIA_LIB_PATH  -L$DEPENDENCY_LIB_ROOT/lib -lz -L$DFB_PATH/lib -ldirect -lfusion -ldirectfb -lpthread"

CC=$CROSS_COMPILE"-gcc"
CXX=$CROSS_COMPILE"-g++"
LD=$CROSS_COMPILE"-ld"
HOST=${CROSS_COMPILE%?}


echo "Running autoconf & automake"
autoreconf --force --install
autoconf
automake

./configure     CC="$CC" \
                CXX="$CXX"\
                LD="$LD -EL"\
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="-Wl,-rpath,/mslib -Wl,-rpath,/lib -Wl,-rpath,$UTOPIA_LIB_PATH" \
                CPPFLAGS="$CPPFLAGS_SETTING" \
                DIRECTFB_CFLAGS="$DIRECTFB_CFLAGS_SETTING" \
                DIRECTFB_LIBS="$DIRECTFB_LIBS_SETTING" \
                --prefix=/directfb_examples \
                --host=$HOST \
                --build=i386-linux \
                --disable-debug
./fix_prefix.sh