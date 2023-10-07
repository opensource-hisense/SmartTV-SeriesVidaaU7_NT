ifeq "$(ANDROID_TWO_WORLDS)" "true"
LOCAL_PATH := $(call my-dir)

include $(MTK_CLEAR_VARS)

$(PRODUCT_OUT)/tvconfig.img:$(TARGET_TVCONFIG_OUT)/config/directfbrc
$(TARGET_TVCONFIG_OUT)/config/directfbrc:$(abspath $(LOCAL_PATH))/directfbrc
$(TARGET_TVCONFIG_OUT)/config/directfbrc:BUILT_ENTRY := $(abspath $(LOCAL_PATH))

$(TARGET_TVCONFIG_OUT)/config/directfbrc:
	$(copy-file-to-target)
ifneq "$(ANDROID_VERSION)" ""
	sed -i -e 's/vendor\/lib/mnt\/vendor\/tvservice\/glibc/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i '/mst_gop_counts/d' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i '/mst_gop_available/d' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i '$$a mst_gop_counts=1' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i '$$a mst_gop_available[0]=3' $(TARGET_TVCONFIG_OUT)/config/directfbrc
endif

else

LOCAL_PATH := $(call my-dir)

include $(MTK_CLEAR_VARS)

$(PRODUCT_OUT)/tvconfig.ext4:$(TARGET_TVCONFIG_OUT)/config/directfbrc
$(TARGET_TVCONFIG_OUT)/config/directfbrc:$(abspath $(LOCAL_PATH))/directfbrc
$(TARGET_TVCONFIG_OUT)/config/directfbrc:BUILT_ENTRY := $(abspath $(LOCAL_PATH))

$(TARGET_TVCONFIG_OUT)/config/directfbrc:
	$(copy-file-to-target)

	sed -i 's/vendor\/lib/basic\/lib/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i 's/mst_gop_counts=./mst_gop_counts=2/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc

ifeq "$(MST_CHIP_NAME)" "mt5879"
ifneq "$(MODEL_NAME)" "tvs_mt9618_linux"
	sed -i '$$a mst_GPU_AFBC=true' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	# assign AFBC layer only.
	sed -i '$$a mst_AFBC_layer_enable=0x2' $(TARGET_TVCONFIG_OUT)/config/directfbrc
else
	sed -i 's/mst_new_ir_first_repeat_time=.*/mst_new_ir_first_repeat_time=120/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i 's/mst_new_ir_repeat_time=.*/mst_new_ir_repeat_time=120/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc
	sed -i 's/mst_usbir_repeat_time=.*/mst_usbir_repeat_time=120/g' $(TARGET_TVCONFIG_OUT)/config/directfbrc
endif
endif

endif
