#
# File          TARGET_linux-mips.mk
# Title         Specify toolchain/compiler flag/link flag/... variables
#               about linux-misp platform.
# Author        Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

ANDROID_DIR := $(TOPDIR)
TARGET_TOOLCHAIN_PREFIX := mips-linux-gnu

# $(1): os/arch
define select-android-config-h
$(ANDROID_DIR)/include/build/core/combo/include/arch/$(1)/AndroidConfig.h
endef

#----------------------------------------------------------------
TARGET_mips_CFLAGS :=	-O2 \
			-fomit-frame-pointer \
			-fno-strict-aliasing    \
			-funswitch-loops

# Set FORCE_MIPS_DEBUGGING to "true" in your buildspec.mk
# or in your environment to gdb debugging easier.
# Don't forget to do a clean build.
ifeq ($(FORCE_MIPS_DEBUGGING),true)
  TARGET_mips_CFLAGS += -fno-omit-frame-pointer
endif
TARGET_GLOBAL_CFLAGS += \
			$(TARGET_mips_CFLAGS) \
			-Ulinux -U__unix -U__unix__ -Umips \
			-fpic \
			-ffunction-sections \
			-fdata-sections \
			-funwind-tables \
			-Werror=format-security \
			$(arch_variant_cflags)

android_config_h := $(call select-android-config-h,linux-mips)
TARGET_ANDROID_CONFIG_CFLAGS := -include $(android_config_h) -I $(dir $(android_config_h))
TARGET_GLOBAL_CFLAGS += $(TARGET_ANDROID_CONFIG_CFLAGS)

# This warning causes dalvik not to build with gcc 4.6.x and -Werror.
# We cannot turn it off blindly since the option is not available
# in gcc-4.4.x.
ifneq ($(filter 4.6 4.6.%, $(shell $(TARGET_CC) --version)),)
TARGET_GLOBAL_CFLAGS += -Wno-unused-but-set-variable \
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

ifneq ($(ARCH_MIPS_PAGE_SHIFT),)
TARGET_GLOBAL_CFLAGS += -DPAGE_SHIFT=$(ARCH_MIPS_PAGE_SHIFT)
endif

TARGET_GLOBAL_LDFLAGS += \
			$(arch_variant_ldflags)

TARGET_GLOBAL_CPPFLAGS += -fvisibility-inlines-hidden \
				-fno-use-cxa-atexit

# More flags/options can be added here
TARGET_RELEASE_CFLAGS := \
			-DNDEBUG \
			-g \
			-Wstrict-aliasing=2 \
			-fgcse-after-reload \
			-frerun-cse-after-loop \
			-frename-registers

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
	-Bdynamic \
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
