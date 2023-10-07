# Global setup
COMPANY = mtk

MEDIATEK = $(TOP)/MediaTek
#lzhang May/12/2009
#DIRECTFBTOP = $(TOP)/../..

#modifed to the ramdisk
ifndef ROOTFS_NAME
ROOTFS_NAME = 5391_free
endif

#
# for variable "CHILING_ROOT", we include this makefile
#
export CHILING_ROOT ?= $(word 1, $(subst /linux_core/,/linux_core /, $(shell pwd)))


DESTDIR := $(ROOTFS_TARGET)
###lzhang, Mar/04/2009, the last slash "/" could NOT be deleted!

#ifndef DESTDIR
#DESTDIR = $(TOP)/../../opt/mt53
#endif

prefix  = /home
bindir  = /home
datadir = /home/data
libdir  = /lib
sysconfdir = $(LINUX_ROOTFS_PREFIX)/etc

#
# Toolchain location
ifndef CROSS_COMPILE
CROSS_COMPILE    = arm11_mtk_le-
endif

CC ?= $(CROSS_COMPILE)gcc
LD ?= $(CROSS_COMPILE)ld
NM ?= $(CROSS_COMPILE)nm
STRIP ?= $(CROSS_COMPILE)strip

#
# Configuration
ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
CPPFLAGS += -DHAVE_FUSIONDALE -DLINUX_2_6 -DPAGE_SHIFT=12 -Werror -Wno-unused-result #-DSAWMAN_NO_LAYER_DST_WINDOW 
else
CPPFLAGS += -DHAVE_FUSIONDALE -DLINUX_2_6 -DPAGE_SHIFT=12 -Werror #-DSAWMAN_NO_LAYER_DST_WINDOW 
endif

ifeq ($(GTV),true)
  CPPFLAGS += -DFONT=\"/system/fonts/DroidSans.ttf\"
else
  CPPFLAGS += -DFONT=\"$(datadir)/decker.ttf\"
endif

CPPFLAGS += -D_CPU_LITTLE_ENDIAN_ -DNDEBUG


#
# Compiler flags
ifeq ($(DEBUG),yes)
CFLAGS += -g -ggdb -O1 -fno-omit-frame-pointer 
else ifeq "$(ENABLE_CA9)" "true"
CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16
else ifeq "$(ENABLE_VFP)" "true"
CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv6z -mtune=arm1176jz-s -mfloat-abi=softfp -mfpu=vfp
else ifeq "$(ENABLE_CA9_NEON)" "true"
	ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
		CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv7-a -mtune=cortex-a9  -mfpu=neon-vfpv4 -marm
	else
		CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon-vfpv4
	endif
else ifeq "$(ENABLE_CA15_NEON)" "true"
CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv7-a -mtune=cortex-a15 -mfloat-abi=softfp -mfpu=neon-vfpv4
else
CFLAGS += -g -ggdb -O2 -ffast-math -fomit-frame-pointer -march=armv6j -mtune=arm1136jf-s
endif

#
# Compiler flags
CFLAGS += -DCC_OSD_USE_FBM
#CFLAGS += -DCC_GFX_IMPROVE
#CFLAGS += -DCC_GFX_LOG
#CFLAGS += -DCC_GFX_STATISTIC
CFLAGS += -DSTRETCH_WA
CFLAGS += -DCC_GFX_FLIP_MIRROR

# -Werror

#
# System module headers
INCLUDES += -I$(MEDIATEK)/DirectFB/systems

#
# FreeType / PNG location

#
# Different modules, drivers etc.
OBJECTS += \
	$(SOURCE)/DirectFB/interfaces/IDirectFBFont/idirectfbfont_ft2.o \
	$(SOURCE)/DirectFB/interfaces/IDirectFBImageProvider/idirectfbimageprovider_dfiff.o	\
	$(SOURCE)/DirectFB/interfaces/IDirectFBImageProvider/idirectfbimageprovider_gif.o	\
	$(SOURCE)/DirectFB/interfaces/IDirectFBImageProvider/idirectfbimageprovider_jpeg.o	\
	$(SOURCE)/DirectFB/interfaces/IDirectFBImageProvider/idirectfbimageprovider_png.o	\
	$(SOURCE)/DirectFB/wm/default/default.o							\
	$(MEDIATEK)/DirectFB/gfxdrivers/mt53/mt53_2d.o						\
	$(MEDIATEK)/DirectFB/gfxdrivers/mt53/mt53_gfxdriver.o					\
	$(MEDIATEK)/DirectFB/systems/mt53/mt53.o						\
	$(MEDIATEK)/DirectFB/systems/mt53/mt53_primary.o					\
	$(MEDIATEK)/DirectFB/systems/mt53/mt53_surface_pool.o					\
	$(MEDIATEK)/DirectFB/systems/mt53/surfacemanager.o					\
	$(MEDIATEK)/DirectFB/systems/mt53/mt53_vdp.o					\
	$(MEDIATEK)/DirectFB/systems/mt53/surfacemanager_vdp.o						

ifneq "$(ANDROID_TWO_WORLDS)" "true"
OBJECTS += $(MEDIATEK)/DirectFB/gfxdrivers/gles/mstar_gles2.o
CFLAGS  += -DDFB_OGLES_SUPPORT
CPPFLAGS += -DDFB_OGLES_SUPPORT
LIBS    += -lGLESv2 -lEGL
endif

OBJECTS += \
	$(MEDIATEK)/gfx/gfx_cmdque.o	\
	$(MEDIATEK)/gfx/gfx_debug.o	\
	$(MEDIATEK)/gfx/vgfx_matrix.o	\
	$(MEDIATEK)/gfx/vgfx_drvif.o	\
	$(MEDIATEK)/gfx/gfx_drvif.o	\
	$(MEDIATEK)/gfx/gfx_if.o	

OBJECTS += $(MEDIATEK)/DirectFB/inputdrivers/mtnet/mtnet.o

ifeq "$(KEY_FROM_DFB)" "y"
CFLAGS += -DKEY_FROM_DFB
ifeq ($(GTV),true)
  OBJECTS += $(MEDIATEK)/DirectFB/inputdrivers/mtipc/mtipc.o
else # !GTV

ifneq "$(ANDROID_TWO_WORLDS)" "true"
OBJECTS += $(SOURCE)/DirectFB/inputdrivers/linux_input/linux_input.o				
endif
OBJECTS += $(MEDIATEK)/DirectFB/inputdrivers/mtrc/mtrc.o


ifeq "$(DFB_USBMOUSE_SUPPORT)" "y"
OBJECTS += $(MEDIATEK)/DirectFB/inputdrivers/usbmouse/usbmouse.o
endif
ifeq "$(DFB_USBKEYBOARD_SUPPORT)" "y"
OBJECTS += $(MEDIATEK)/DirectFB/inputdrivers/usbkeyboard/usbkeyboard.o
endif

endif # GTV
endif


INCLUDES += -I$(MEDIATEK)/inc -I$(MEDIATEK)/x_inc -I$(X_INC_DIR)
INCLUDES += -I$(MEDIATEK)/mt53xx -I$(MEDIATEK)/gfx
LDFLAGS  += -L$(MEDIATEK)/lib

include $(MEDIATEK)/Tools/tool.mak

include Config_mt53/$(COMPANY).mak
