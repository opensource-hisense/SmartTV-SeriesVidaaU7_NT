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
ifndef VM_LINUX_ROOT
VM_LINUX_ROOT := $(word 1, $(subst /vm_linux/,/vm_linux /, $(shell pwd)))
endif

#include $(VM_LINUX_ROOT)/oss/source/mak/target.mak
OSS_LIB_ROOT = $(VM_LINUX_ROOT)/oss/library/gnuarm-$(TOOL_CHAIN)_neon_ca9
include $(MTK_BUILD_ROOT)/oss_version.mk

JOBS ?= 20

export WPA_SUPPLICANT_BINDIR=./bin

get_wpa_supplicant_source_tree = "aaaa/bbbb/$(WIFI_DONGLE)"

$(warning =============================================)
$(warning get_wpa_supplicant_source_tree=$(get_wpa_supplicant_source_tree))
$(warning =============================================)

ifneq ($(filter $(WIFI_DONGLE), MT7668U MT7663 MT7603U MT7961),)
       get_wpa_supplicant_source_tree = "wpa_supplicant_cfg80211/wpa_supplicant"
endif

$(warning =============================================)
$(warning get_wpa_supplicant_source_tree=$(get_wpa_supplicant_source_tree))
$(warning =============================================)

HOSTAPD_PATH:= ./$(get_wpa_supplicant_source_tree)/../hostapd

export HOSTAPD_BIN := $(BUILD_TARGET_OBJ_ROOT)/$(get_wpa_supplicant_source_tree)/../hostapd/
export WPA_SUPPLICANT_BIN := $(BUILD_TARGET_OBJ_ROOT)/$(get_wpa_supplicant_source_tree)/

.PHONY: all wpa_supplicant install clean wifi_target clean

all: wifi_target 


wpa_supplicant:	
	make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree) -f Makefile install BINDIR=$(WPA_SUPPLICANT_BINDIR)
	if [ -e $(HOSTAPD_PATH) ] ; then  \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree) -f Makefile clean ; \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree)/../hostapd -f Makefile install BINDIR=$(WPA_SUPPLICANT_BINDIR) ; \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree)/../hostapd -f Makefile clean ; \
	fi
	$(STRIP) --strip-unneeded $(WPA_SUPPLICANT_BIN)/bin/*
	if [ -e $(HOSTAPD_PATH) ] ; then  \
		$(STRIP) --strip-unneeded $(HOSTAPD_BIN)/bin/* ; \
	fi
	
wifi_target: wpa_supplicant
#	mkdir -p $(OSS_LIB_ROOT)/wpa_supplicant/common/
#	cp -a $(WPA_SUPPLICANT_BIN)/bin/wpa_supplicant  $(OSS_LIB_ROOT)/wpa_supplicant/common/
#	cp -a $(WPA_SUPPLICANT_BIN)/bin/wpa_cli         $(OSS_LIB_ROOT)/wpa_supplicant/common/
#	if [ -e $(HOSTAPD_PATH) ] ; then  \
		cp -a $(HOSTAPD_BIN)/bin/hostapd         $(OSS_LIB_ROOT)/wpa_supplicant/bin/ ; \
	fi

install_wpa_supplicant:
	make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree) -f Makefile install BINDIR=$(WPA_SUPPLICANT_BINDIR)
	if [ -e $(HOSTAPD_PATH) ] ; then  \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree) -f Makefile clean ; \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree)/../hostapd -f Makefile install BINDIR=$(WPA_SUPPLICANT_BINDIR) ; \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree)/../hostapd -f Makefile clean ; \
	fi
	$(STRIP) --strip-unneeded $(WPA_SUPPLICANT_BIN)/bin/*
	if [ -e $(HOSTAPD_PATH) ] ; then  \
		$(STRIP) --strip-unneeded $(HOSTAPD_BIN)/bin/* ; \
	fi
	
clean_wpa_supplicant clean:
	make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree) -f Makefile BINDIR=$(WPA_SUPPLICANT_BINDIR) clean -s --no-print-directory
	if [ -e $(HOSTAPD_PATH) ] ; then  \
		make -j $(JOBS) -C ./$(get_wpa_supplicant_source_tree)/../hostapd -f Makefile BINDIR=$(WPA_SUPPLICANT_BINDIR) clean -s --no-print-directory ; \
	fi
	if [ -e $(HOSTAPD_BIN) ] ; then  \
		rm -fr $(HOSTAPD_BIN) ; \
	fi
	if [ -e $(WPA_SUPPLICANT_BIN) ] ; then  \
		rm -fr $(WPA_SUPPLICANT_BIN) ; \
	fi
	
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/wpa_supplicant_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/wpa_supplicant_INFO.txt
