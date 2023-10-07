#!/bin/sh

DFB_PATH=`pwd`

#MST_PREFIX=/vendor
MTK_PREFIX=

if [ "$1" = "" ]; then
    echo "Build in $DFB_PATH/../out_tmp"
    mkdir -p $DFB_PATH/../out_tmp
    BUILD_PATH=$DFB_PATH/../out_tmp
    export DESTDIR=$DFB_PATH/output
else
    echo "Build in $1"
    mkdir -p $1/out_tmp
    BUILD_PATH=$1/out_tmp
    export DESTDIR=$BUILD_PATH/../output
fi

if [ "$2" != "" ]; then
    echo "kernel dir = $2"
    export KERNEL_DIR="$2"
fi

if [ "$3" != "" ]; then
    echo "misdk dir = $3"
    export MISDK_DIR="$3"
fi

export DEPENDENCY_LIB_ROOT="$BUILD_PATH/DependencyLib"
function build()
{

SHARED_LIB_PATH="$DEPENDENCY_LIB_ROOT/lib"
DFB_NEEDED_PATH="$DEPENDENCY_LIB_ROOT/include"

echo "=========== DFB needed library ==========="
echo DFB_PATH            : $DFB_PATH
echo UTOPIA_LIB_VERSION  : $UTOPIA_LIB_VERSION
echo SHARED_LIB_PATH     : $SHARED_LIB_PATH
echo DFB_NEEDED_PATH     : $DFB_NEEDED_PATH
echo "=========================================="


LD_LIB_PATH+=" -L$SHARED_LIB_PATH -lz -lm -lstdc++"
#LD_LIB_PATH+=" -L$UTOPIA_LIB_PATH"

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
CFLAGS_SETTING+=" -I$DFB_PATH -DMSOS_TYPE_LINUX"
CFLAGS_SETTING+=" -DMI_ENABLE_DBG=0"
CFLAGS_SETTING+=" -Wformat=2 -Wformat-security -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -fPIE -pie"
CFLAGS_SETTING+=" -funwind-tables"

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
    _CC_="arm-linux-gnueabi-gcc -mlittle-endian"
else
    _CC_="arm-linux-gnueabi-gcc -mlittle-endian"
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
    return
else
    echo "Directory $DESTDIR does not exists."
fi


#Please modify utopia lib version

CHIP_CPU="ARM"
PHY_BITS="64"
KERNEL_TYPE="pack"

UTOPIA_LIB_VERSION="phy_"$PHY_BITS"bits_"$KERNEL_TYPE

rm -rf $BUILD_PATH/DependencyLib
mkdir -p $BUILD_PATH/DependencyLib

cd $BUILD_PATH/DependencyLib
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/$UTOPIA_LIB_VERSION.tar.gz $BUILD_PATH/DependencyLib
gzip -d $UTOPIA_LIB_VERSION.tar.gz
tar -xvf $UTOPIA_LIB_VERSION.tar
rm -rf include/utopia lib/utopia
rm -rf $UTOPIA_LIB_VERSION.tar


mkdir mali;cd mali
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/mali.tgz mali.tar.gz
gzip -d mali.tar.gz
tar -xvf mali.tar
find . -name '*.h' -exec cp --parents \{\} ../include \;
cp -rf *.h ../include
cp -rf lib* ../lib
cd ..
rm -rf mali

mkdir sti;cd sti
cp -rf $DFB_PATH/DependencyLib/$CHIP_CPU/sti.tar.gz .
gzip -d sti.tar.gz
tar -xvf sti.tar
find . -name '*.h' -exec cp --parents \{\} ../include \;
find . -name '*.inc' -exec cp --parents \{\} ../include \;
find . -name 'lib*so*' -exec cp --parents \{\} ../lib \;
cd ..
rm -rf sti

if [ "$MISDK_DIR" != "" ] && [ -d $MISDK_DIR ]; then
ln -s -f $MISDK_DIR/mi/mi/core/modules/include/mi_common.h $DEPENDENCY_LIB_ROOT/include/mi_common.h
ln -s -f $MISDK_DIR/mi/mi/modules/render/primary/include/mi_render.h $DEPENDENCY_LIB_ROOT/include/mi_render.h
ln -s -f $MISDK_DIR/mi/mi/modules/mm/primary/include/mtk_iommu.h $DEPENDENCY_LIB_ROOT/include/mtk_iommu.h
# link to pq head files
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/pqmap_utility/data_tag_cus.inc $DEPENDENCY_LIB_ROOT/include/data_tag_cus.inc
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/pqmap_utility/node_tag_cus.inc $DEPENDENCY_LIB_ROOT/include/node_tag_cus.inc
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/pqmap_utility/pqmap_utility.h $DEPENDENCY_LIB_ROOT/include/pqmap_utility.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/pqmap_utility/pqmap_utility_graphic.h $DEPENDENCY_LIB_ROOT/include/pqmap_utility_graphic.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/pqmap_utility_loader/pqmap_utility_loader.h $DEPENDENCY_LIB_ROOT/include/pqmap_utility_loader.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/xc_alg/mapi_pq_cfd_gop_if.h $DEPENDENCY_LIB_ROOT/include/mapi_pq_cfd_gop_if.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/xc_alg/mapi_pq_output_format_if.h $DEPENDENCY_LIB_ROOT/include/mapi_pq_output_format_if.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/common/pqu_port.h $DEPENDENCY_LIB_ROOT/include/pqu_port.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/common/linux_user/pqu_port_os.h $DEPENDENCY_LIB_ROOT/include/pqu_port_os.h
ln -s -f $MISDK_DIR/mtktv_drivers/ree/include/common/linux_user/msos_types.h $DEPENDENCY_LIB_ROOT/include/msos_types.h
ln -s -f $MISDK_DIR/utopia/utpa2-secure/mxlib/include/TEE_HEADER/utpa2_XC.h $DEPENDENCY_LIB_ROOT/include/utpa2_XC.h
ln -s -f $MISDK_DIR/utopia/utpa2-secure/mxlib/include/TEE_HEADER/utpa2_Types.h $DEPENDENCY_LIB_ROOT/include/utpa2_Types.h
fi

if [ "$KERNEL_DIR" != "" ] && [ -d $KERNEL_DIR ]; then
ln -s -f $KERNEL_DIR/include/uapi/linux/mtk-v4l2-g2d.h $DEPENDENCY_LIB_ROOT/include/mtk-v4l2-g2d.h
ln -s -f $KERNEL_DIR/include/uapi/linux/videodev2.h $DEPENDENCY_LIB_ROOT/include/videodev2.h
ln -s -f $KERNEL_DIR/include/uapi/linux/dma-heap.h $DEPENDENCY_LIB_ROOT/include/dma-heap.h
ln -s -f $KERNEL_DIR/include/uapi/linux/mtk_iommu_dtv_api.h $DEPENDENCY_LIB_ROOT/include/mtk_iommu_dtv_api.h
# link to drm header file
ln -s -f $KERNEL_DIR/include/uapi/drm/mtk_tv_drm.h $DEPENDENCY_LIB_ROOT/include/mtk_tv_drm.h
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
                LD="arm-linux-gnueabi-ld -EL"\
                CFLAGS="$CFLAGS_SETTING" \
                LDFLAGS="-Wformat=2 -Wformat-security -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now $LD_LIB_PATH" \
                CPPFLAGS="$CPPFLAGS_SETTING" \
                FREETYPE_CFLAGS="-I$DFB_NEEDED_PATH/freetype " \
                FREETYPE_LIBS="-lfreetype -lz" \
                LIBPNG_LIBS="-lpng12" \
                LIBS="$MSTAR_LIB" \
                --prefix=$MTK_PREFIX \
                --sysconfdir=/config \
                --host=arm-linux-gnueabi \
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
                --with-gfxdrivers=mstargfx_sti,mstargfx_gles2 \
                --with-inputdrivers=mstarlinuxinput,mstarir,mstarloopbackinput,mikeypad \
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
                --enable-hwdecodepng \
                --enable-mtksti

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

