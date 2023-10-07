LOCAL_PATH := $(call my-dir)

include $(MTK_BUILD_ROOT)/oss_version.mk

include $(MTK_CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcrypto libssl libnl-3 libnl-genl
LOCAL_MODULE_PATH := $(TARGET_3RD_OUT)/bin/wpa_supplicant/common
include $(MTK_BUILD_SYSTEM)/base_rules.mk
INSTALLED_wpa_supplicant := $(LOCAL_OUT_OBJ)/wpa_supplicant/wpa_supplicant_cfg80211/wpa_supplicant/bin/wpa_supplicant
wpa_supplicant_build_target := $(LOCAL_OUT_OBJ)/wpa_supplicant/fake_target
$(INSTALLED_wpa_supplicant):DEPEND_C_INCLUDE := $(foreach inc,$(LOCAL_C_INCLUDES) $(LOCAL_IMPORTED_INCLUDES),$(addprefix -I,$(abspath $(inc))))
$(INSTALLED_wpa_supplicant):DEPEND_SHARED_LIBRARIES := $(foreach lib,$(BUILT_SHARED_LIBRARIES),$(abspath $(lib)))
$(INSTALLED_wpa_supplicant):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ)/wpa_supplicant)
$(INSTALLED_wpa_supplicant):BUILT_ENTRY := $(abspath $(LOCAL_PATH))
$(LOCAL_BUILT_MODULE):$(INSTALLED_wpa_supplicant)
	$(copy-file-to-target)	
$(INSTALLED_wpa_supplicant): $(BUILT_DEPEND_LIBRARIES) $(VAR_EXPORT) FORCE_TARGET	
		
include $(MTK_CLEAR_VARS)
LOCAL_MODULE := hostapd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcrypto libssl libnl-3 libnl-genl
LOCAL_MODULE_PATH := $(TARGET_3RD_OUT)/bin/wpa_supplicant/common
include $(MTK_BUILD_SYSTEM)/base_rules.mk
INSTALLED_hostapd := $(LOCAL_OUT_OBJ)/wpa_supplicant/wpa_supplicant_cfg80211/hostapd/bin/hostapd
$(INSTALLED_hostapd):DEPEND_C_INCLUDE := $(foreach inc,$(LOCAL_C_INCLUDES) $(LOCAL_IMPORTED_INCLUDES),$(addprefix -I,$(abspath $(inc))))
$(INSTALLED_hostapd):DEPEND_SHARED_LIBRARIES := $(foreach lib,$(BUILT_SHARED_LIBRARIES),$(abspath $(lib)))
$(INSTALLED_hostapd):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ)/wpa_supplicant)
$(INSTALLED_hostapd):BUILT_ENTRY := $(abspath $(LOCAL_PATH))
$(LOCAL_BUILT_MODULE):$(INSTALLED_hostapd)
	$(copy-file-to-target)
$(INSTALLED_hostapd): $(BUILT_DEPEND_LIBRARIES) $(VAR_EXPORT) FORCE_TARGET		
	
		
include $(MTK_CLEAR_VARS)
LOCAL_MODULE := wpa_cli
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libcrypto libssl libnl-3 libnl-genl
LOCAL_MODULE_PATH := $(TARGET_3RD_OUT)/bin/wpa_supplicant/common
include $(MTK_BUILD_SYSTEM)/base_rules.mk
INSTALLED_wpa_cli := $(LOCAL_OUT_OBJ)/wpa_supplicant/wpa_supplicant_cfg80211/wpa_supplicant/bin/wpa_cli
$(INSTALLED_wpa_cli):DEPEND_C_INCLUDE := $(foreach inc,$(LOCAL_C_INCLUDES) $(LOCAL_IMPORTED_INCLUDES),$(addprefix -I,$(abspath $(inc))))
$(INSTALLED_wpa_cli):DEPEND_SHARED_LIBRARIES := $(foreach lib,$(BUILT_SHARED_LIBRARIES),$(abspath $(lib)))
$(INSTALLED_wpa_cli):BUILT_OUT_INTERMEDIATES:= $(abspath $(LOCAL_OUT_OBJ)/wpa_supplicant)
$(INSTALLED_wpa_cli):BUILT_ENTRY := $(abspath $(LOCAL_PATH))
$(LOCAL_BUILT_MODULE):$(INSTALLED_wpa_cli)
	$(copy-file-to-target)
$(INSTALLED_wpa_cli): $(BUILT_DEPEND_LIBRARIES) $(VAR_EXPORT) FORCE_TARGET		
	

$(INSTALLED_hostapd) $(INSTALLED_wpa_supplicant) $(INSTALLED_wpa_cli):$(wpa_supplicant_build_target)

$(wpa_supplicant_build_target): $(BUILT_DEPEND_LIBRARIES) $(VAR_EXPORT) FORCE_TARGET		

$(wpa_supplicant_build_target): 
	$(MAKE) -C $(BUILT_ENTRY) all CROSS_COMPILE=$(CROSS_COMPILE) DEPEND_C_INCLUDE="$(DEPEND_C_INCLUDE)" DEPEND_SHARED_LIBRARIES="$(DEPEND_SHARED_LIBRARIES)"   BUILD_TARGET_OBJ_ROOT=$(BUILT_OUT_INTERMEDIATES) WIFI_DONGLE="$(WIFI_DONGLE)" CC="$(CC)" STRIP="$(STRIP)" MTK_BUILD_ROOT=$(MTK_BUILD_ROOT)
