LOCAL_PATH := $(call my-dir)

include $(MTK_CLEAR_VARS)
LOCAL_MODULE                := libtinyalsa-master
LOCAL_SRC_FILES             := ./src/mixer.c \
                               ./src/mixer_hw.c \
                               ./src/mixer_plugin.c \
                               ./src/pcm.c \
                               ./src/pcm_hw.c \
                               ./src/pcm_plugin.c \
                               ./src/snd_card_plugin.c
LOCAL_C_INCLUDES            += $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_CFLAGS                += -Werror \
                               -Wno-stringop-truncation \
                               -Wno-macro-redefined \
                               -DTINYALSA_USES_PLUGINS \
                               -DSNDRV_PCM_IOCTL_TTSTAMP
LOCAL_LDFLAGS               := -lc \
                               -ldl
LOCAL_VENDOR_MODULE         := true
ifneq ($(filter true, $(LINUX_ONLY)),)
LOCAL_MODULE_PATH           := $(TARGET_ROOTFS_OUT)/lib
else
LOCAL_MODULE_PATH           := $(TARGET_TVSERVICE_OUT)/glibc
endif
include $(MTK_BUILD_SHARED_LIBRARY)

include $(MTK_CLEAR_VARS)
LOCAL_MODULE                := tinyplay-master
LOCAL_SRC_FILES             := ./utils/tinyplay.c
LOCAL_CFLAGS                += -Werror
LOCAL_SHARED_LIBRARIES      := libtinyalsa-master
LOCAL_VENDOR_MODULE         := true
LOCAL_MODULE_PATH           := $(TARGET_ROOTFS_OUT)
include $(MTK_BUILD_EXECUTABLE)

include $(MTK_CLEAR_VARS)
LOCAL_MODULE                := tinycap-master
LOCAL_SRC_FILES             := ./utils/tinycap.c
LOCAL_CFLAGS                += -Werror
LOCAL_SHARED_LIBRARIES      := libtinyalsa-master
LOCAL_VENDOR_MODULE         := true
LOCAL_MODULE_PATH           := $(TARGET_ROOTFS_OUT)
include $(MTK_BUILD_EXECUTABLE)

include $(MTK_CLEAR_VARS)
LOCAL_MODULE                := tinymix-master
LOCAL_SRC_FILES             := ./utils/tinymix.c
LOCAL_CFLAGS                += -Werror \
                               -Wall
LOCAL_SHARED_LIBRARIES      := libtinyalsa-master
LOCAL_VENDOR_MODULE         := true
LOCAL_MODULE_PATH           := $(TARGET_ROOTFS_OUT)
include $(MTK_BUILD_EXECUTABLE)

include $(MTK_CLEAR_VARS)
LOCAL_MODULE                := tinypcminfo-master
LOCAL_SRC_FILES             := ./utils/tinypcminfo.c
LOCAL_CFLAGS                += -Werror
LOCAL_SHARED_LIBRARIES      := libtinyalsa-master
LOCAL_VENDOR_MODULE         := true
LOCAL_MODULE_PATH           := $(TARGET_ROOTFS_OUT)
include $(MTK_BUILD_EXECUTABLE)
