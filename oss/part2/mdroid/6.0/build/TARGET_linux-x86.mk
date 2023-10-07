#
# File          TARGET_linux-x86.mk
# Title         Specify toolchain/compiler flag/link flag/... variables
#               about linux-x86 platform.
# Author        Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

ANDROID_DIR := $(TOPDIR)
TARGET_TOOLCHAIN_PREFIX := i686-unknown-linux-gnu-

# $(1): os/arch
define select-android-config-h
$(ANDROID_DIR)/include/build/core/combo/include/arch/$(1)/AndroidConfig.h
endef

#----------------------------------------------------------------
TARGET_GLOBAL_CFLAGS += \
			-O2 \
			-Ulinux \
			-Wa,--noexecstack \
			-Werror=format-security \
			-Wstrict-aliasing=2 \
			-fPIC -fPIE \
			-ffunction-sections \
			-finline-functions \
			-finline-limit=300 \
			-fno-inline-functions-called-once \
			-fno-short-enums \
			-fstrict-aliasing \
			-funswitch-loops \
			-funwind-tables \
			-fstack-protector

android_config_h := $(call select-android-config-h,target_linux-x86)
TARGET_ANDROID_CONFIG_CFLAGS := -include $(android_config_h) -I $(dir $(android_config_h))
TARGET_GLOBAL_CFLAGS += $(TARGET_ANDROID_CONFIG_CFLAGS)

# XXX: Not sure this is still needed. Must check with our toolchains.
TARGET_GLOBAL_CPPFLAGS += \
			-fno-use-cxa-atexit

# XXX: Our toolchain is normally configured to always set these flags by default
# however, there have been reports that this is sometimes not the case. So make
# them explicit here unless we have the time to carefully check it
#
TARGET_GLOBAL_CFLAGS += -mstackrealign -msse3 -mfpmath=sse -m32

# XXX: These flags should not be defined here anymore. Instead, the Android.mk
# of the modules that depend on these features should instead check the
# corresponding macros (e.g. ARCH_X86_HAVE_SSE2 and ARCH_X86_HAVE_SSSE3)
# Keep them here until this is all cleared up.
#
ifeq ($(ARCH_X86_HAVE_SSE2),true)
TARGET_GLOBAL_CFLAGS += -DUSE_SSE2
endif

ifeq ($(ARCH_X86_HAVE_SSSE3),true)   # yes, really SSSE3, not SSE3!
TARGET_GLOBAL_CFLAGS += -DUSE_SSSE3
endif

# XXX: This flag is probably redundant. I believe our toolchain always sets
# it by default. Consider for removal.
#
TARGET_GLOBAL_CFLAGS += -mbionic

# XXX: This flag is probably redundant. The macro should be defined by our
# toolchain binaries automatically (as a compiler built-in).
# Check with: $BINPREFIX-gcc -dM -E < /dev/null
#
# Consider for removal.
#
TARGET_GLOBAL_CFLAGS += -D__ANDROID__

# XXX: This flag is probably redundant since our toolchain binaries already
# generate 32-bit machine code. It probably dates back to the old days
# where we were using the host toolchain on Linux to build the platform
# images. Consider it for removal.
TARGET_GLOBAL_LDFLAGS += -m32

TARGET_GLOBAL_LDFLAGS += -Wl,-z,noexecstack
TARGET_GLOBAL_LDFLAGS += -Wl,-z,relro -Wl,-z,now
TARGET_GLOBAL_LDFLAGS += -Wl,--gc-sections

#----------------------------------------------------------------
define transform-o-to-shared-lib-inner
@$(TARGET_CXX) \
	-Wl,-soname,$(notdir $@) \
	-shared -Bsymbolic \
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
	-Wl,-z,nocopyreloc \
	-fPIE -pie \
	-o $@ \
	$(PRIVATE_ALL_OBJECTS) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
	$(LDFLAGS)
endef

define transform-o-to-static-executable-inner
@$(TARGET_CXX) \
	-Bstatic \
	-o $@ \
	$(LDFLAGS) \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--start-group \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	-Wl,--end-group
endef
