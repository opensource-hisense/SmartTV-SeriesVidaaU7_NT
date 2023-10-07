LOCAL_PATH := $(call my-dir)

#-- MODULE name : DIRECTFB

include $(MTK_CLEAR_VARS)
LOCAL_MODULE := libdirectfb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libfusion libdirect
LOCAL_MODULE_PATH := $(TARGET_BASIC_OUT)/lib/directfb
LOCAL_EXPORT_C_INCLUDE_DIRS=$(DFB_GEN_INC_DIR)

include $(MTK_BUILD_SYSTEM)/base_rules.mk

LOCAL_OUT_OBJ := $(BUILT_OUT_OBJ)/dfb

INSTALLED_DIRECTFB_LIB := $(LOCAL_OUT_OBJ)/lib/$(LOCAL_MODULE).so

$(INSTALLED_DIRECTFB_LIB):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ))
$(INSTALLED_DIRECTFB_LIB):BUILT_ENTRY := $(abspath $(LOCAL_PATH))

$(LOCAL_BUILT_MODULE):$(INSTALLED_DIRECTFB_LIB)
				$(copy-symlink-to-target)
ifneq (false,$(strip $(LOCAL_STRIP_MODULE)))
	@echo "Stripped: $@"
	$(hide) $(PRIVATE_STRIP) --strip-unneeded $@
endif

$(LOCAL_BUILT_MODULE):$(LOCAL_LINK_MODULE)
$(LOCAL_LINK_MODULE):$(INSTALLED_DIRECTFB_LIB)
	$(copy-file-to-target)
	if [ -n "$(SYMBOL_INSTALLED_MODULE)" ];then \
            mkdir -p $(dir $(SYMBOL_INSTALLED_MODULE)); \
            cp -af $(PRIVATE_BUILT_INTERMEDIATES)/LINKED/. $(dir $(SYMBOL_INSTALLED_MODULE));\
            cp -af $(PRIVATE_BUILT_INTERMEDIATES)/directfb-1.4-0/ $(dir $(SYMBOL_INSTALLED_MODULE));\
        fi;
#-- MODULE name : FUSION

include $(MTK_CLEAR_VARS)
LOCAL_MODULE := libfusion
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_BASIC_OUT)/lib/directfb
LOCAL_EXPORT_C_INCLUDE_DIRS=$(DFB_GEN_INC_DIR)

include $(MTK_BUILD_SYSTEM)/base_rules.mk

LOCAL_OUT_OBJ := $(BUILT_OUT_OBJ)/dfb

INSTALLED_FUSION_LIB := $(LOCAL_OUT_OBJ)/lib/$(LOCAL_MODULE).so

$(INSTALLED_FUSION_LIB):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ))
$(INSTALLED_FUSION_LIB):BUILT_ENTRY := $(abspath $(LOCAL_PATH))

$(LOCAL_BUILT_MODULE):$(INSTALLED_FUSION_LIB)
				$(copy-symlink-to-target)
ifneq (false,$(strip $(LOCAL_STRIP_MODULE)))
	@echo "Stripped: $@"
	$(hide) $(PRIVATE_STRIP) --strip-unneeded $@
endif

$(LOCAL_BUILT_MODULE):$(LOCAL_LINK_MODULE)
$(LOCAL_LINK_MODULE):$(INSTALLED_FUSION_LIB)
	$(copy-file-to-target)
	if [ -n "$(SYMBOL_INSTALLED_MODULE)" ];then \
            mkdir -p $(dir $(SYMBOL_INSTALLED_MODULE)); \
            cp -af $(PRIVATE_BUILT_INTERMEDIATES)/LINKED/. $(dir $(SYMBOL_INSTALLED_MODULE));\
        fi;
#-- MODULE name : DIRECT

include $(MTK_CLEAR_VARS)
LOCAL_MODULE := libdirect
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_BASIC_OUT)/lib/directfb
LOCAL_EXPORT_C_INCLUDE_DIRS=$(DFB_GEN_INC_DIR)

include $(MTK_BUILD_SYSTEM)/base_rules.mk

LOCAL_OUT_OBJ := $(BUILT_OUT_OBJ)/dfb

INSTALLED_DIRECT_LIB := $(LOCAL_OUT_OBJ)/lib/$(LOCAL_MODULE).so

$(INSTALLED_DIRECT_LIB):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ))
$(INSTALLED_DIRECT_LIB):BUILT_ENTRY := $(abspath $(LOCAL_PATH))

$(LOCAL_BUILT_MODULE):$(INSTALLED_DIRECT_LIB)
				$(copy-symlink-to-target)
ifneq (false,$(strip $(LOCAL_STRIP_MODULE)))
	@echo "Stripped: $@"
	$(hide) $(PRIVATE_STRIP) --strip-unneeded $@
endif

$(LOCAL_BUILT_MODULE):$(LOCAL_LINK_MODULE)
$(LOCAL_LINK_MODULE):$(INSTALLED_DIRECT_LIB)
	$(copy-file-to-target)
	if [ -n "$(SYMBOL_INSTALLED_MODULE)" ];then \
            mkdir -p $(dir $(SYMBOL_INSTALLED_MODULE)); \
            cp -af $(PRIVATE_BUILT_INTERMEDIATES)/LINKED/. $(dir $(SYMBOL_INSTALLED_MODULE));\
        fi;
#-- Build script
DFB_BUILD_TARGET := $(PRODUCT_OUT)/obj_glibc/dfb/dfb_build_target
DFB_BUILD_lock := $(PRODUCT_OUT)/obj_glibc/dfb/lock
$(DFB_GEN_INC_DIR) $(DFB_GEN_INC_FILE) : $(INSTALLED_DIRECTFB_LIB) $(INSTALLED_FUSION_LIB) $(INSTALLED_DIRECT_LIB)
$(INSTALLED_DIRECTFB_LIB) $(INSTALLED_FUSION_LIB) $(INSTALLED_DIRECT_LIB): $(DFB_BUILD_TARGET)
$(DFB_BUILD_TARGET):$(VAR_EXPORT)
	if [ -e $(DFB_BUILD_lock) ]; then \
            for ((i=1;i<300;i++)) ; do \
               if [ -e $(DFB_BUILD_lock) ]; then \
                   sleep 2; \
               else \
                   echo "dfb exit"; \
                   exit 0; \
               fi; \
            done; \
            if [ -e $(DFB_BUILD_lock) ]; then \
                echo "directfb 10m timeout"; \
                rm $(DFB_BUILD_lock); \
                exit 1; \
            fi; \
        else \
            mkdir -p $(dir $(DFB_BUILD_lock)); \
            touch $(DFB_BUILD_lock); \
        fi;
	@echo DFB_BUILD_TARGET start
ifneq "$(ANDROID_VERSION)" ""
ifeq "$(CHIP_TYPE)" "merak"
	source $(VAR_EXPORT); \
	cd $(BUILT_ENTRY)/directfb_1.4.2/1.4.2/; \
	source ToolChainSetting $(TOOL_CHAIN) $(TOOLCHAIN_ROOT); \
	export KERNELDIR=$(KERNELDIR); \
	export MISDK_DIR=$(MISDK_DIR); \
	export BUILD_PATH=$(BUILT_OUT_INTERMEDIATES)/out_tmp; \
	source BuildScript/Arm_phy_64bits_linaro_fusion_pack_mi_android_sti.sh; $(MAKE); $(MAKE) install
else
	source $(VAR_EXPORT); \
	cd $(BUILT_ENTRY)/directfb_1.4.2/1.4.2/; \
	source ToolChainSetting $(TOOL_CHAIN) $(TOOLCHAIN_ROOT); \
	export KERNELDIR=$(KERNELDIR); \
	export MISDK_DIR=$(MISDK_DIR); \
	export BUILD_PATH=$(BUILT_OUT_INTERMEDIATES)/out_tmp; \
        export CHIP_TYPE=$(CHIP_TYPE); \
	source BuildScript/Arm_phy_64bits_linaro_fusion_pack_mi_android.sh; $(MAKE); $(MAKE) install
endif
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cd $(BUILT_OUT_INTERMEDIATES)/output/vendor/linux_rootfs/include;cp -rfu directfb/* ./; cp -rfu directfb-internal/* ./
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/output/vendor/linux_rootfs/lib/ $(BUILT_OUT_INTERMEDIATES)/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/output/vendor/linux_rootfs/include/ $(abspath $(PRODUCT_OUT)/obj_glibc/dfb)
else
ifeq "$(CHIP_TYPE)" "merak"
	cd $(BUILT_ENTRY)/directfb_1.4.2/1.4.2/; \
	source ToolChainSetting $(TOOL_CHAIN) $(TOOLCHAIN_ROOT); \
	export KERNELDIR=$(KERNELDIR); \
	export MISDK_DIR=$(MISDK_DIR); \
	export BUILD_PATH=$(BUILT_OUT_INTERMEDIATES)/out_tmp; \
	source BuildScript/Arm_phy_64bits_linaro_fusion_pack_mi_linux_sti.sh; $(MAKE); $(MAKE) install
else
	cd $(BUILT_ENTRY)/directfb_1.4.2/1.4.2/; \
	source ToolChainSetting $(TOOL_CHAIN) $(TOOLCHAIN_ROOT); \
	export KERNELDIR=$(KERNELDIR); \
	export MISDK_DIR=$(MISDK_DIR); \
	export BUILD_PATH=$(BUILT_OUT_INTERMEDIATES)/out_tmp; \
        export CHIP_TYPE=$(CHIP_TYPE); \
	source BuildScript/Arm_phy_64bits_linaro_fusion_pack_mi_linux.sh; $(MAKE); $(MAKE) install
endif
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cd $(BUILT_OUT_INTERMEDIATES)/output/include;cp -rfu directfb/* ./; cp -rfu directfb-internal/* ./
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/output/lib/ $(BUILT_OUT_INTERMEDIATES)/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/output/include/ $(abspath $(PRODUCT_OUT)/obj_glibc/dfb)
endif
	mkdir -p $(TARGET_BASIC_OUT)/lib/directfb/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);find $(BUILT_OUT_INTERMEDIATES)/lib/ -name "*.la" | xargs rm -rf; \
	rm -rf  $(BUILT_OUT_INTERMEDIATES)/lib/pkgconfig;
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/lib/* $(TARGET_BASIC_OUT)/lib/directfb/; \
	find $(TARGET_BASIC_OUT)/lib/directfb/ -type f | xargs $(PRIVATE_STRIP) --strip-unneeded
	cd $(TARGET_BASIC_OUT)/lib/ ; \
	for i in `ls directfb/lib*`; do \
	ln -sf $$i `basename $$i`; \
	done

	mkdir -p $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libdirectfb_intermediates/
	mkdir -p $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libdirect_intermediates/
	mkdir -p $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libfusion_intermediates/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/lib/*  $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libdirectfb_intermediates/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/lib/*  $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libdirect_intermediates/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/lib/*  $(PRODUCT_OUT)/obj_glibc/SHARED_LIBRARIES/libfusion_intermediates/
	if [ -d $(MISDK_DIR)/extlibs/prebuild/opensource/directfb/1.4.2/ ]; then \
            cp -rf $(BUILT_OUT_INTERMEDIATES)/lib/*  $(MISDK_DIR)/extlibs/prebuild/opensource/directfb/1.4.2/lib/; \
            cp -rf $(abspath $(PRODUCT_OUT)/obj_glibc/dfb/include/*)  $(MISDK_DIR)/extlibs/prebuild/opensource/directfb/1.4.2/include/; \
        fi
	if [ -e $(DFB_BUILD_lock) ]; then \
            rm $(DFB_BUILD_lock); \
        fi
	mkdir -p $(TARGET_BASIC_OUT)/bin/
	source $(VAR_EXPORT);source $(ENV_SCRIPT);cp -rfu $(BUILT_OUT_INTERMEDIATES)/output/bin/dfbdump  $(TARGET_BASIC_OUT)/bin/; \
	find $(TARGET_BASIC_OUT)/bin/dfbdump -type f | xargs $(PRIVATE_STRIP) --strip-unneeded
	@echo DFB_BUILD_TARGET end
clean_libdirectfb clean_libfusion clean_libdirect:clean_customizaion_dfb
clean_customizaion_dfb:
	rm -rf $(BUILT_OUT_OBJ)/dfb

include $(LOCAL_PATH)/directfbrc/mediatek.mk
