#-----------------------------------------------------------------------------
# Copyright (c) 2010, MediaTek Inc.
# All rights reserved.
#
# Unauthorized use, practice, perform, copy, distribution, reproduction,
# or disclosure of this information in whole or in part is prohibited.
#-----------------------------------------------------------------------------

THIS_DIR := $(shell pwd)

.PHONY: all install clean
all: install
	@echo 'Create Bluetooth Middleware!'
	mkdir -p $(OSS_LIB_ROOT)/bluetooth_mw/include
	mkdir -p $(OSS_LIB_ROOT)/bluetooth_mw/include_rpc
	cp -r $(THIS_DIR)/inc/*.h $(OSS_LIB_ROOT)/bluetooth_mw/include
	cp -r $(THIS_DIR)/inc/config/*.h $(OSS_LIB_ROOT)/bluetooth_mw/include
	cp -r $(THIS_DIR)/inc_rpc/*.h $(OSS_LIB_ROOT)/bluetooth_mw/include_rpc
	@echo 'Release bluedroid library finished.'
	
clean:
		
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/bluetooth_mw_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/bluetooth_mw_INFO.txt

install:
	
