1. Need to assign toolchain and NCT5 flag to build so.
   I.E  dockerq make NCT5_ENABLE=true TOOL_CHAIN=10.2.1
   
2. Install the lib/header to assigned toolchain library folder.
   I.E dockerq make NCT5_ENABLE=true TOOL_CHAIN=10.2.1 install
   
3. Delete temp object and lib
   dockerq make NCT5_ENABLE=true TOOL_CHAIN=10.2.1 clean
   

Note.
   If version changes,need to change oss_version as well
   For NCT4,oss_version is oss/source/mak/oss_version.mak
   For NCT5,oss_version is build/oss_version.mk

