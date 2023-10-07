#!/bin/bash
echo the first script param is: $1
echo the second script param is: $2

make="make"
clean="clean"

mkdir ./output
##export CROSS_COMPILE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi-
export CROSS_COMPILE=/proj/mtk20683/apollo-dev-2102-2148-t38-ToolChainUpgrade-mtk17496/tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
##export CROSS_COMPILE=./../../../tools/mtk_toolchain/gcc-arm-none-linux-gnueabihf-10.2.1/bin/arm-none-linux-gnueabihf-
export OSS_PATH=../output
export TOOL_CHAIN=10.2.1

if [[ $1 == $make ]];
then
cd liveMedia ; pwd ; dtvcmdq make -L ODB=true TOOL_CHAIN=10.2.1 && make install
cd ../groupsock ; pwd ; dtvcmdq make -L ODB=true TOOL_CHAIN=10.2.1 && make install
cd ../UsageEnvironment ; pwd ; dtvcmdq make -L ODB=true TOOL_CHAIN=10.2.1 && make install
cd ../BasicUsageEnvironment ; pwd ;dtvcmdq make -L ODB=true TOOL_CHAIN=10.2.1 && make install
elif [[ $1 == $clean ]];
then
cd liveMedia ; pwd ; dtvcmdq make clean
cd ../groupsock ; pwd ; dtvcmdq make clean
cd ../UsageEnvironment ; pwd ; dtvcmdq make clean
cd ../BasicUsageEnvironment ; pwd ;dtvcmdq make clean
rm -rf ../output/*.so
else
cd liveMedia ; pwd ; dtvcmdq make && make install
cd ../groupsock ; pwd ; dtvcmdq make && make install
cd ../UsageEnvironment ; pwd ; dtvcmdq make && make install
cd ../BasicUsageEnvironment ; pwd ;dtvcmdq make && make install
fi



