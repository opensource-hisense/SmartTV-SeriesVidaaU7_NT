#-----------------------------------------------------------------------------
# Copyright (c) 2011, MediaTek Inc.
# All rights reserved.
#
# Unauthorized use, practice, perform, copy, distribution, reproduction,
# or disclosure of this information in whole or in part is prohibited.
#-----------------------------------------------------------------------------
# $RCSfile:  $
# $Revision:
# $Date:
# $Author:  $
# $CCRevision:  $
# $SWAuthor:  $
# $MD5HEX:  $
#
# Description:
#        A set of files describing the framebuffer interface and core types.
#---------------------------------------------------------------------------*/

COMPILE_OPTS =		$(INCLUDES) -I. -fPIC -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DENABLE_MTK_RTSP        
ifeq "$(DBG)" "true"
COMPILE_OPTS +=		-g -O0
else
COMPILE_OPTS +=		-O2
endif
C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1
OBJ =			o
LINK =			$(CROSS_COMPILE)g++ -o
LINK_OPTS =		
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)g++ -shared -fPIC -o 
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX =			so
#LIBRARY_LINK =		$(CROSS_COMPILE)ar cr 
#LIBRARY_LINK_OPTS =	$(LINK_OPTS)
#LIB_SUFFIX =			a
#LIBRARY_SHARE =		$(CROSS_COMPILE)g++ -shared -fPIC -o  
#LIBRARY_SHARE_OPTS =	
#LIB_SHARE_SUFFIX = so
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

LIVEMEDIA_DIR = liveMedia
GROUPSOCK_DIR = groupsock
USAGE_ENVIRONMENT_DIR = UsageEnvironment
BASIC_USAGE_ENVIRONMENT_DIR = BasicUsageEnvironment

TESTPROGS_DIR = testProgs

MEDIA_SERVER_DIR = mediaServer 

include ../mak/target.mak
export TOOL_CHAIN = 10.2.1

ifndef OSS_PATH 
OSS_PATH  =$(OSS_LIB_ROOT)/liveMedia
endif 

.PHONY: all livemedia_target livemedia_compile clean

all: livemedia_target

livemedia_compile:
	echo 	CROSS_COMPILE=$(CROSS_COMPILE)
	cd $(LIVEMEDIA_DIR) ; $(MAKE) DBG=$(DBG)
	cd $(GROUPSOCK_DIR) ; $(MAKE) DBG=$(DBG)
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE) DBG=$(DBG)
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE) DBG=$(DBG)
	cd $(TESTPROGS_DIR) ; $(MAKE) DBG=$(DBG)
	cd $(MEDIA_SERVER_DIR) ; $(MAKE) DBG=$(DBG)
	
livemedia_target: livemedia_compile
	echo 	CROSS_COMPILE=$(CROSS_COMPILE)
	cp -rf BasicUsageEnvironment/*.so $(OSS_PATH)/
	cp -rf groupsock/*.so $(OSS_PATH)/
	cp -rf liveMedia/*.so $(OSS_PATH)/
	cp -rf UsageEnvironment/*.so $(OSS_PATH)/
	
ifeq "$(DBG)" "true"
	@echo "ilding debug mode"
else
	$(CROSS_COMPILE)strip --strip-unneeded  $(OSS_PATH)/*.so
endif

clean:
	cd $(LIVEMEDIA_DIR) ; $(MAKE) clean
	cd $(GROUPSOCK_DIR) ; $(MAKE) clean
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(TESTPROGS_DIR) ; $(MAKE) clean
	cd $(MEDIA_SERVER_DIR) ; $(MAKE) clean
	
notice:
	cp -rf NOTICE.2011.06.12.txt $(OSS_NOTICE_OUTPUT)/liveMedia_NOTICE.txt
    
info:
	cp -rf OSS_INFO.2011.06.12.txt $(OSS_INFO_OUTPUT)/liveMedia_INFO.txt

