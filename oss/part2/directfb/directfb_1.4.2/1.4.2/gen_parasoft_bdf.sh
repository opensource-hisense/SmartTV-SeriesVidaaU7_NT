#!/bin/sh

#sync source
#fsync //DAILEO/Supernova
#fsync //THEALE/RedLion/2.6.32.15/
#fsync //THEALE/RedLion/Lunar/directfb/1.4.2/

cd /home/jason-km.chen/PERFORCE/;
chmod 777 DAILEO/Supernova/ -R;
chmod 777 THEALE/RedLion/2.6.32.15/ -R;
chmod 777 THEALE/RedLion/Lunar/directfb/1.4.2/ -R;

#make SN
cd DAILEO/Supernova/projects/;
source ./buildsettings/build_T12_116A.sh;
make rebuild_all;
make image_all;

#make DFB
cd ../../../THEALE/RedLion/Lunar/directfb/1.4.2;
export THEALE_PATH=$(pwd)/../../../..;
echo $THEALE_PATH;
sed -i 's/#PHOTOSPHERE\_ROOT=.*/PHOTOSPHERE\_ROOT=\"$THEALE\_PATH\/..\/DAILEO\/Supernova\"/g' mstconfigure_x86_ut.sh;
sed -i 's/KERNEL\_PATH=.*/KERNEL\_PATH=\"\$THEALE\_PATH\/RedLion\/2.6.32.15\/kernel\"/g' mstconfigure_x86_ut.sh;
sed -i 's/CHIP=.*/CHIP=t12/g' mstconfigure_x86_ut.sh;
./mstconfigure_x86_ut.sh;
sed -i 's/echo/ECHO/g' libtool;
sed -i 's/\"ECHO\"/\"echo\"/g' libtool;
sed -i 's/`ECHO/`echo/g' libtool;
make all;

cd gfxdrivers/mstar_g2/;
make;
cd ../../interfaces/IDirectFBImageProvider/;
sed -i 's/-lapiGOP//' Makefile;
sed -i 's/-ldrvVE//' Makefile;
make;
cd ../IDirectFBVideoProvider/;
sed -i 's/-lapiGOP//' Makefile;
sed -i 's/-ldrvVE//' Makefile;
make;
cd ../..
rm -rf `find -iname "*cpptestscan.bdf"`;
make clean;
make all;

./genBdf.sh;
