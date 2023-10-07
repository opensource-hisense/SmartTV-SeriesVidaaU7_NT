#!/bin/sh

DFB_PATH=`pwd`

#MST_PREFIX=/vendor
MTK_PREFIX=/vendor/linux_rootfs
SUB='directfb'

if [[ "$BUILD_PATH" == *"$SUB"* ]] || [ "$BUILD_PATH" = "" ]; then
    echo "Build in $DFB_PATH/../out_tmp"
    BUILD_PATH=$DFB_PATH/../out_tmp
    export DESTDIR=$DFB_PATH/output
else
    echo "Build in $BUILD_PATH"
    export DESTDIR=$BUILD_PATH/../output
fi
export DEPENDENCY_LIB_ROOT="$BUILD_PATH/DependencyLib"

function build()
{
SHARED_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib"
DFB_NEEDED_PATH="$DEPENDENCY_LIB_ROOT/include"
UTOPIA_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib/utopia/"
UTOPIA_INCLUDE_PATH="$DEPENDENCY_LIB_ROOT/include/utopia"

echo "=========== DFB needed library ==========="
echo DFB_PATH            : $DFB_PATH
echo UTOPIA_LIB_VERSION  : $UTOPIA_LIB_VERSION
echo SHARED_LIB_PATH     : $SHARED_LIB_PATH
echo DFB_NEEDED_PATH     : $DFB_NEEDED_PATH
echo UTOPIA_LIB_PATH     : $UTOPIA_LIB_PATH
echo UTOPIA_INCLUDE_PATH : $UTOPIA_INCLUDE_PATH
echo "=========================================="


LD_LIB_PATH+=" -L$SHARED_LIB_PATH"
LD_LIB_PATH+=" -L$UTOPIA_LIB_PATH -lz -lm -lstdc++"

CFLAGS_SETTING+=" ${MSTAR_CFLAG}"
CFLAGS_SETTING+=" -DDIRECTFB_CHANGE_ID=\\\"`git log --pretty=format:"ID:[%h]" -1`\\\""
CFLAGS_SETTING+=" -DVISIBILITY_HIDDEN"
CFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX"
#CFLAGS_SETTING+=" -mlittle-endian"
#CFLAGS_SETTING+=" -march=armv7-a"
#CFLAGS_SETTING+=" -mcpu=cortex-a9"
#CFLAGS_SETTING+=" -mtune=cortex-a9"
#CFLAGS_SETTING+=" -mfpu=neon-vfpv4"
#CFLAGS_SETTING+=" -fPIC"
#CFLAGS_SETTING+=" -fvisibility=default"
#CFLAGS_SETTING+=" -mfpu=neon"
#CFLAGS_SETTING+=" -mfloat-abi=softfp"
CFLAGS_SETTING+=" -w"
CFLAGS_SETTING+=" -fno-strict-aliasing"
CFLAGS_SETTING+=" -fno-optimize-sibling-calls"
CFLAGS_SETTING+=" -fno-exceptions"
CFLAGS_SETTING+=" -ffunction-sections"
CFLAGS_SETTING+=" -fdata-sections"
CFLAGS_SETTING+=" -fno-peephole2"
CFLAGS_SETTING+=" -pthread"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/png"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/zlib"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/jpeg"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype/freetype2"
CFLAGS_SETTING+=" -I$DFB_NEEDED_PATH/freetype"
CFLAGS_SETTING+=" -I$UTOPIA_INCLUDE_PATH"
CFLAGS_SETTING+=" -I$DFB_PATH -DMSOS_TYPE_LINUX"
CFLAGS_SETTING+=" -DMI_ENABLE_DBG=0"
CFLAGS_SETTING+=" -Wformat=2 -Wformat-security -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -fPIE -pie"
CFLAGS_SETTING+=" -funwind-tables"
if [ "$CHIP_TYPE" == "fusion" ]; then
CFLAGS_SETTING+=" -DMI_FUSION"
fi


echo $CFLAGS_SETTING | grep VISIBILITY_HIDDEN

CPPFLAGS_SETTING+=" ${MSTAR_CPPFLAG}"
CPPFLAGS_SETTING+=" -DMSOS_PROCESS_SAFT_MUTEX"
#CPPFLAGS_SETTING+=" -mlittle-endian"
#CPPFLAGS_SETTING+=" -march=armv7-a"
#CPPFLAGS_SETTING+=" -mcpu=cortex-a9"
#CPPFLAGS_SETTING+=" -mtune=cortex-a9"
#CPPFLAGS_SETTING+=" -mfpu=neon-vfpv4"
#CPPFLAGS_SETTING+=" -fPIC"
#CPPFLAGS_SETTING+=" -fvisibility=default"
#CPPFLAGS_SETTING+=" -mfpu=neon"
#CPPFLAGS_SETTING+=" -mfloat-abi=softfp"
CPPFLAGS_SETTING+=" -Wall"
CPPFLAGS_SETTING+=" -Wpointer-arith"
CPPFLAGS_SETTING+=" -Wstrict-prototypes"
CPPFLAGS_SETTING+=" -Winline"
CPPFLAGS_SETTING+=" -Wundef"
CPPFLAGS_SETTING+=" -fno-strict-aliasing"
CPPFLAGS_SETTING+=" -fno-optimize-sibling-calls"
CPPFLAGS_SETTING+=" -fno-exceptions"
CPPFLAGS_SETTING+=" -ffunction-sections"
CPPFLAGS_SETTING+=" -fdata-sections"
CPPFLAGS_SETTING+=" -fno-peephole2"
CPPFLAGS_SETTING+=" -pthread"
CPPFLAGS_SETTING+=" -Wformat=2 -Wformat-security -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -fPIE -pie"

if [ $? == 0 ];  then
    _CC_="${CROSS_COMPILE_COMPILER} -mlittle-endian"
else
    _CC_="${CROSS_COMPILE_COMPILER} -mlittle-endian"
fi


# **********************************************
# Reference Libs
# **********************************************
MSTAR_LIB+=" -lfreetype"
MSTAR_LIB+=" -lrt"
MSTAR_LIB+=" -lpthread"

if [ -d $DESTDIR ]; then
    echo "Directory $DESTDIR exists."
    cd $BUILD_PATH/
    if [ "$(cat DFB_RECORD)" = "$DFB_PATH" ]; then
        echo "Root not changed! Return!"
        return
    else
        rm -rf $BUILD_PATH/
        echo "Root changed! Rebuild.."
    fi
else
    echo "Directory $DESTDIR does not exists."
fi
mkdir -p $BUILD_PATH
echo $DFB_PATH > $BUILD_PATH/DFB_RECORD

#Please modify utopia lib version

CHIP_CPU="ARM"
PHY_BITS="64"
KERNEL_TYPE="pack"

UTOPIA_LIB_VERSION="phy_"$PHY_BITS"bits_"$KERNEL_TYPE

mkdir -p $BUILD_PATH/DependencyLib

cd $BUILD_PATH/DependencyLib
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/$UTOPIA_LIB_VERSION.tar.gz $BUILD_PATH/DependencyLib
gzip -d $UTOPIA_LIB_VERSION.tar.gz
tar -xvf $UTOPIA_LIB_VERSION.tar
rm -rf $UTOPIA_LIB_VERSION.tar

mkdir mi;cd mi
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/mi.tar.gz .
gzip -d mi.tar.gz
tar -xvf mi.tar
cp -rf lib* ../lib/utopia
cp -rf *.h ../include
cd ..
rm -rf mi/

mkdir mi_mtk_482;cd mi_mtk_482
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/mi_mtk_482.tar.gz .
gzip -d mi_mtk_482.tar.gz
tar -xvf mi_mtk_482.tar -C ../lib/utopia
cd ..
rm -rf mi_mtk_482

mkdir mali;cd mali
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/mali.tgz mali.tar.gz
gzip -d mali.tar.gz
tar -xvf mali.tar
find . -name '*.h' -exec cp --parents \{\} ../include \;
cp -rf *.h ../include
cp -rf lib* ../lib
cd ..
rm -rf mali


if [ -d $DFB_PATH/../../../../../misdk/mi/mi/modules ]; then
rm -rf $DEPENDENCY_LIB_ROOT/include/mi_*.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/cap/include/mi_cap.h $DEPENDENCY_LIB_ROOT/include/mi_cap.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/include/mi_common.h $DEPENDENCY_LIB_ROOT/include/mi_common.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/disp/include/mi_disp.h $DEPENDENCY_LIB_ROOT/include/mi_disp.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/fs/include/mi_fs.h $DEPENDENCY_LIB_ROOT/include/mi_fs.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/imgdec/include/mi_imgdec.h $DEPENDENCY_LIB_ROOT/include/mi_imgdec.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/os/include/mi_os.h $DEPENDENCY_LIB_ROOT/include/mi_os.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/osd/include/mi_osd.h $DEPENDENCY_LIB_ROOT/include/mi_osd.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/sar/include/mi_sar.h $DEPENDENCY_LIB_ROOT/include/mi_sar.h
ln -s $DFB_PATH/../../../../../misdk/mi/mi/modules/sys/include/mi_sys.h $DEPENDENCY_LIB_ROOT/include/mi_sys.h
fi

if [ "$KERNELDIR" != "" ] && [ -d $KERNELDIR ]; then
ln -s -f $KERNELDIR/include/uapi/linux/mtk_iommu_dtv_api.h $DEPENDENCY_LIB_ROOT/include/mtk_iommu_dtv_api.h
fi

if [ -d $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d ]; then
rm -rf $DEPENDENCY_LIB_ROOT/include/jpeg/j*.h
ln -s $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d/pre-install/include/jconfig.h $DEPENDENCY_LIB_ROOT/include/jpeg/jconfig.h
ln -s $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d/pre-install/include/jerror.h $DEPENDENCY_LIB_ROOT/include/jpeg/jerror.h
ln -s $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d/pre-install/include/jmorecfg.h $DEPENDENCY_LIB_ROOT/include/jpeg/jmorecfg.h
ln -s $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d/pre-install/include/jpeglib.h $DEPENDENCY_LIB_ROOT/include/jpeg/jpeglib.h
rm -rf $DEPENDENCY_LIB_ROOT/lib/libjpeg*
ln -s $DFB_PATH/../../../../../oss/library/gnuarm-5.5.0_neon_ca9/jpeg/9d/pre-install/lib/libjpeg.so.9.4.0 $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9.4.0
ln -s $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9.4.0 $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9
ln -s $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9.4.0 $DEPENDENCY_LIB_ROOT/lib/libjpeg.so
else
ln -s $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9.4.0 $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9
ln -s $DEPENDENCY_LIB_ROOT/lib/libjpeg.so.9.4.0 $DEPENDENCY_LIB_ROOT/lib/libjpeg.so
fi

if [ "$SECURITY_DIR" != "" ] && [ -d $SECURITY_DIR ]; then
ln -s -f $SECURITY_DIR/optee_src/3.x/optee_client/public/tee_client_api.h $DEPENDENCY_LIB_ROOT/include/tee_client_api.h
fi

test -z $DFB_PATH && echo "the DFB_PATH must be set!!" && exit 0
test -z $DEPENDENCY_LIB_ROOT && echo "the DEPENDENCY_LIB_ROOT must be set!!" && exit 0

cp $DFB_PATH/versionInfo.sh $BUILD_PATH
cd $BUILD_PATH
$DFB_PATH/configure     CC="$_CC_" \
                LD="${CROSS_COMPILE_LD} -EL"\
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="-Wformat=2 -Wformat-security -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -Wl,-rpath-link,$UTOPIA_LIB_PATH  $LD_LIB_PATH" \
                CPPFLAGS="$CPPFLAGS_SETTING" \
                FREETYPE_CFLAGS="-I$DFB_NEEDED_PATH/freetype " \
                FREETYPE_LIBS="-lfreetype -lz" \
                LIBPNG_LIBS="-lpng12" \
                LIBS="$MSTAR_LIB" \
                --prefix=$MTK_PREFIX \
                --sysconfdir=/vendor/tvconfig/config \
                --host=$CROSS_COMPILE \
                --build=i386-linux \
                --enable-jpeg \
                --enable-zlib \
                --enable-png \
                --enable-gif \
                --enable-gopc \
                --disable-vec \
                --enable-freetype \
                --disable-debug \
                --disable-profiling \
                --disable-unique \
                --with-gfxdrivers=mstargfx_mi,mstargfx_gles2 \
                --with-inputdrivers=mtnet \
                --enable-shared \
                --disable-static \
                --enable-multi \
                --with-tests=yes \
                --disable-fbdev \
                --disable-x11 \
                --disable-vnc \
                --disable-sdl \
                --enable-devmem \
                --disable-pmem \
                --enable-hwdecodejpeg \
                --disable-hwdecodegif \
                --enable-imageprovider-mi \
                --enable-mstarmi \
                --enable-hwdecodepng \
                --enable-ansupport

#sed -i 's/ECHO/echo/g' libtool
sed -i 's/-rpath[ ]/-rpath-link /g' libtool
}

LIBRARY_NAME="directfb"
LIBRARY_VERSION="1.4.2m"
PACKAGE_NAME="${LIBRARY_NAME}-${LIBRARY_VERSION}"
PACKAGE_PATH=../../


export OUTPUT_DIR="${DESTDIR}/${MTK_PREFIX}"
echo $OUTPUT_DIR
echo $MTK_PREFIX

case $1 in
"package")
    echo "################  pacakage $LIBRARY_NAME"
        package $2
        ;;
*)
    echo "################  building $LIBRARY_NAME"
        build $1
        ;;
esac

