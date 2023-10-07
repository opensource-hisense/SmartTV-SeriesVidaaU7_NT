#!/bin/bash

TOP=$(pwd)

if [ $# = "1" ]; then
  DFB_TOOL_CHAIN=$1
else
  DFB_TOOL_CHAIN=482
fi

# echo ${DFB_TOOL_CHAIN}
# echo ${DIRECTFB_VERSION}

if [ "${DFB_TOOL_CHAIN}" = "c4tv" ]; then
  unset SYSROOT
fi

source ${TOP}/ToolChainSetting Linaro_Arm_fusion_${DFB_TOOL_CHAIN}
source ${TOP}/BuildScript/Arm_phy_64bits_linaro_fusion_pack_mi_linux_${DFB_TOOL_CHAIN}.sh

make clean
mosesq make 2>&1 | tee make.log
make install
