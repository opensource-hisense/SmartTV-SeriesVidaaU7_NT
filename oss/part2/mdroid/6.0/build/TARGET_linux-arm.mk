#
# File          TARGET_linux-arm.mk
# Title         Specify toolchain/compiler flag/link flag/... variables
#               about linux-armv7-a-neon platform.
# Author        Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

ANDROID_DIR := $(TOPDIR)

ifeq ($(EXTLIBS_TOOLCHAIN_VERSION), 4.8.2)
TARGET_TOOLCHAIN_PREFIX := armv7a-mediatek482_001_neon-linux-gnueabi-
arch_variant_cflags := -mfloat-abi=softfp -fpic
else ifeq ($(EXTLIBS_TOOLCHAIN_VERSION), clang_9.0.0)
TARGET_TOOLCHAIN_PREFIX := armv7a-cros-linux-gnueabihf-
arch_variant_cflags := -mfloat-abi=hard -fpic
else ifeq ($(EXTLIBS_TOOLCHAIN_VERSION), 4.9.3)
TARGET_TOOLCHAIN_PREFIX := arm-linux-gnueabihf-
arch_variant_cflags := -mfloat-abi=hard
else ifeq ($(EXTLIBS_TOOLCHAIN_VERSION), 5.5)
TARGET_TOOLCHAIN_PREFIX := arm-linux-gnueabi-
arch_variant_cflags := -mfloat-abi=softfp -fpic
else
TARGET_TOOLCHAIN_PREFIX := arm-none-linux-gnueabi-
arch_variant_cflags := -mfloat-abi=softfp
TARGET_GLOBAL_CFLAGS := -msoft-float -fpic
endif


# thum mode support, but use arm mode to build.
ARCH_ARM_HAVE_THUMB_SUPPORT := true
LOCAL_ARM_MODE := arm

# $(1): os/arch
define select-android-config-h
$(ANDROID_DIR)/include/build/core/combo/include/arch/$(1)/AndroidConfig.h
endef

arch_variant_cflags += \
    -march=armv7-a \
    -mfpu=neon \
    -mtune=cortex-a9

#arch_variant_ldflags := \
	-Wl,--fix-cortex-a8

#Add more for Security
arch_variant_ldflags := \
    -Wl,--fix-cortex-a8 \
    -Wformat=2 -Wformat-security \
    -fstack-protector -D_FORTIFY_SOURCE=2 \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,-z,now -s \
    -fPIE -pie


#----------------------------------------------------------------
TARGET_arm_CFLAGS :=    -O2 \
                        -fomit-frame-pointer \
                        -fstrict-aliasing    \
                        -funswitch-loops

# Modules can choose to compile some source as thumb. As
# non-thumb enabled targets are supported, this is treated
# as a 'hint'. If thumb is not enabled, these files are just
# compiled as ARM.
ifeq ($(ARCH_ARM_HAVE_THUMB_SUPPORT),true)
TARGET_thumb_CFLAGS :=  -mthumb \
                        -Os \
                        -fomit-frame-pointer \
                        -fno-strict-aliasing
else
TARGET_thumb_CFLAGS := $(TARGET_arm_CFLAGS)
endif

# Set FORCE_ARM_DEBUGGING to "true" in your buildspec.mk
# or in your environment to force a full arm build, even for
# files that are normally built as thumb; this can make
# gdb debugging easier.  Don't forget to do a clean build.
#
# NOTE: if you try to build a -O0 build with thumb, several
# of the libraries (libpv, libwebcore, libkjs) need to be built
# with -mlong-calls.  When built at -O0, those libraries are
# too big for a thumb "BL <label>" to go from one end to the other.
ifeq ($(FORCE_ARM_DEBUGGING),true)
  TARGET_arm_CFLAGS += -fno-omit-frame-pointer -fno-strict-aliasing
  TARGET_thumb_CFLAGS += -marm -fno-omit-frame-pointer
endif

# STDLIBC_INTEGER
TARGET_GLOBAL_CFLAGS += \
			-ffunction-sections \
			-fdata-sections \
			-funwind-tables \
			-fstack-protector \
			-Wa,--noexecstack \
			-Werror=format-security \
			-D_FORTIFY_SOURCE=2 \
			-fno-short-enums \
			$(arch_variant_cflags)

android_config_h := $(call select-android-config-h,linux-arm)
TARGET_ANDROID_CONFIG_CFLAGS := -include $(android_config_h) -I $(dir $(android_config_h))
TARGET_GLOBAL_CFLAGS += $(TARGET_ANDROID_CONFIG_CFLAGS)

# This warning causes dalvik not to build with gcc 4.6.x and -Werror.
# We cannot turn it off blindly since the option is not available
# in gcc-4.4.x.  We also want to disable sincos optimization globally
# by turning off the builtin sin function.
ifneq ($(filter 4.6 4.6 4.7 4.7.% 4.8,$ $(shell $(TARGET_CC) --version)),)
TARGET_GLOBAL_CFLAGS += -Wno-unused-but-set-variable -fno-builtin-sin \
			-fno-strict-volatile-bitfields
endif

# This is to avoid the dreaded warning compiler message:
#   note: the mangling of 'va_list' has changed in GCC 4.4
#
# The fact that the mangling changed does not affect the NDK ABI
# very fortunately (since none of the exposed APIs used va_list
# in their exported C++ functions). Also, GCC 4.5 has already
# removed the warning from the compiler.
#
TARGET_GLOBAL_CFLAGS += -Wno-psabi

TARGET_GLOBAL_LDFLAGS += \
			-Wl,-z,noexecstack \
			-Wl,-z,relro \
			-Wl,-z,now \
			-Wl,--build-id=md5 \
			-Wl,--warn-shared-textrel \
			-Wl,--fatal-warnings \
			-Wl,--hash-style=gnu \
			$(arch_variant_ldflags)
#                       STDLIBC_INTEGER
#                       -Wl,-icf=safe

# We only need thumb interworking in cases where thumb support
# is available in the architecture, and just to be sure, (and
# since sometimes thumb-interwork appears to be default), we
# specifically disable when thumb support is unavailable.
ifeq ($(ARCH_ARM_HAVE_THUMB_SUPPORT),true)
TARGET_GLOBAL_CFLAGS += -mthumb-interwork
else
TARGET_GLOBAL_CFLAGS += -mno-thumb-interwork
endif

# STDLIBC_INTEGER
TARGET_GLOBAL_CPPFLAGS += -fvisibility-inlines-hidden -std=c++11

# More flags/options can be added here
# STDLIBC_INTEGER
TARGET_RELEASE_CFLAGS := \
			-DNDEBUG \
			-g \
			-Wstrict-aliasing=3 \
			-fgcse-after-reload \
			-frerun-cse-after-loop \
			-frename-registers

#----------------------------------------------------------------
ifeq ($(strip $(LOCAL_ARM_MODE)),)
    TARGET_ARCH_CFLAGS := $(TARGET_thumb_CFLAGS)
else
    TARGET_ARCH_CFLAGS := $(TARGET_arm_CFLAGS)
endif

#----------------------------------------------------------------
define transform-o-to-shared-lib-inner
@$(TARGET_CXX) \
	-Wl,-soname,$(notdir $@) \
	-Wl,--gc-sections \
	-shared -Wl,-Bsymbolic \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--whole-archive \
	$(call normalize-target-libraries,$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES)) \
	-Wl,--no-whole-archive \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	-o $@ \
	$(LDFLAGS)
endef

define transform-o-to-executable-inner
@$(TARGET_CXX) \
	-Bdynamic -fPIE -pie \
	-Wl,--gc-sections \
	-Wl,-z,nocopyreloc \
	-o $@ \
	-Wl,-rpath-link=$(LIBRARY_DIR) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	$(PRIVATE_ALL_OBJECTS) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
	$(LDFLAGS)
endef

define transform-o-to-static-executable-inner
@$(TARGET_CXX) \
	-Bstatic \
	-Wl,--gc-sections \
	-o $@ \
	$(LDFLAGS) \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--start-group \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	-Wl,--end-group
endef
