           How to build DFB lib for the supernova target

1.check the build server environmnet
check automake £¬autoconf £¬autoreconf ,libtool has installed
if on fedora build server ,these tools have not installed 
yum install automake
yum install autoconf
yum install autoreconf
yum install libtool

if on Ubuntu buildserver
apt-get install automake
apt-get install autoconf
apt-get install autoreconf
apt-get install libtool

copy the file ltmain.sh corresponding to the libtool to the $THEALE/RedLion/Lunar/directfb/1.4.2
ex: cp /usr/share/libtool/config/ltmain.sh $THEALE/RedLion/Lunar/directfb/1.4.2

2.	sync codes.
Build the lib need the sync the following files. We have the assumption that DAILEO path is $DAILEO, THEALE path is $THEALE
1)$THEALE/RedLion/Lunar/directfb/1.4.2
2)$THEALE/RedLion/2.6.32.8
2)$DAILEO/Supernova/develop
3)$DAILEO/Supernova/target/mslib


3.	Release the depending  on lib & head file,  including(jpeg png freetype z)
<1> cd DAILEO/Supernova/projects/
<2> source  corresponding env.cfg
<3>make extralibs


Then all the depending on lib will be placed in the right place

4.	Build DFB lib
1)	set the environment variable in mstconfigure_muf.sh:
PHOTOSPHERE_ROOT     THEALE_PATH  CHIP KERNEL_PATH
For examples:
PHOTOSPHERE_ROOT="/home/mstar/Perforce/DAILEO/Supernova"
THEALE_PATH="/home/mstar/Perforce/THEALE"
CHIP=u4
KERNEL_PATH="$THEALE_PATH/RedLion/2.6.35.8"
2)$>cd $THEALE/RedLion/Lunar/directfb/1.4.2
   $>./mstconfigure_muf.sh
			Need check whether the configure status
   $>make all install
   Then all the dfb lib will be install in the /vendor directory.
   $>make package
   Package the target from /vendor directory.

5. Build dfb examples
1)	set the environment variable in the mstconfigure_muf.sh :
PHOTOSPHERE_ROOT     THEALE_PATH  CHIP KERNEL_PATH
For examples:
PHOTOSPHERE_ROOT="/home/mstar/Perforce/DAILEO/Supernova"
THEALE_PATH="/home/mstar/Perforce/THEALE"
CHIP=u4
KERNEL_PATH="$THEALE_PATH/RedLion/2.6.35.8"
2)$>cd $THEALE/RedLion/Lunar/directfb/1.4.2/examples
   $>./mstconfigure_muf.sh
			Need check whether the configure status
   $>make all install
   Then all the dfb lib will be install in the /vendor/directfb_examples directory.







   								how to build DFB lib for the obama target
1.sync codes
Build the lib need the sync the following files. We have the assumption that DAILEO path is $DAILEO, THEALE path is $THEALE
1)$THEALE/RedLion/Lunar/directfb/1.4.2
2)$THEALE/RedLion/2.6.28.9
3)$THEALE/RedLion/Lunar/freetype
4)$THEALE/RedLion/Lunar/libjpeg
5)$THEALE/RedLion/Lunar/zlib
6)$THEALE/RedLion/Lunar/libpng
7)$DAILEO/Obama

In order to build codes ,so please use the root account.

2.set the environment variable
MST_PREFIX must be /opt. P4_DIRECTORY is the perforce path .
we need to set the following environment variable.

export MST_PREFIX=/opt
export P4_DIRECTORY=/home/mstar/PERFORCE
export KERNEL_SRC=$P4_DIRECTORY/THEALE/RedLion/2.6.28.9
export MST_SDK_PATH=$P4_DIRECTORY/DAILEO/Obama/SDK
export MST_SDK_CCID=t3
export PKG_CONFIG_PATH=$MST_PREFIX/lib/pkgconfig
export TOOL_CHAIN_PREFIX=mips-linux-gnu-


3. build the depending on lib (include jpeg png freetype zlib )

1)build pnglib
$>cd $THEALE/RedLion/Lunar/libpng/1.2.32
$>./mstconfigure_titania2.sh
$>make all install

2)build zlib
$>cd $THEALE/RedLion/Lunar/zlib/1.2.3
$>./mstconfigure_titania2.sh
$>make all install

3)build jpeglib
$>cd $THEALE/RedLion/Lunar/libjpeg/6b
$>./mstconfigure_titania2.sh
$>make all install

4)build freetype
$>cd $THEALE/RedLion/Lunar/freetype/2.3.7
$>./mstconfigure_titania2.sh
$>make all install




 4.buid Obama
build Obama is in order to release some lib which only belong to the obama system,and make directfb can refer it
refer to the  $DAILEO/Obama/Document/[Obama]Build_System.doc
we need to the followings steps to get what we want
cd $DAILEO/Obama
1)rm -rf release
2)./menuconfig
3)./mkrootfs.sh
4)make SDK
5)make release-all

5.build directfb lib
1)cd $THEALE/RedLion/Lunar/directfb/1.4.2
2)$>./mstconfigure_oba.sh
please check the config status
3)$>make all install
then all the directfb lib will be in the directory /opt/lib

//by yu.zhang 
test autobuild  MI32&MI64



