#
# Global setup
TOP    = .
SOURCE = $(TOP)/directfb_$(DIRECTFB_VERSION)/Source
KERNEL = $(TOP)/directfb_$(DIRECTFB_VERSION)/Kernel

CONFIG ?= mt53

NM ?= nm

export OBJ_ROOT := $(BUILD_TARGET_OBJ_ROOT)

#
# for variable "CHILING_ROOT", we include this makefile
#


#
# Find the local cfg.mak, if existed, include it
#
CFG_MAK_NAME := cfg.mak

INC_CFG_MAK := $(shell if [ -e $(DFB_CFG_DIR)/$(CFG_MAK_NAME) ];then echo "$(DFB_CFG_DIR)/$(CFG_MAK_NAME)"; else echo ""; fi)

ifneq "$(INC_CFG_MAK)" ""
include $(INC_CFG_MAK)
#CFLAGS += $(DEFINES)
endif
CFLAGS += $(DEFINES)

#
# Version definition
MAJOR   = 1
MINOR   = 5
TINY    = 3
VERSION = $(MAJOR).$(MINOR).$(TINY)

FD_MAJOR = 8
FD_MINOR = 7
FD_TINY  = 0

SWM_MAJOR   = 1
SWM_MINOR   = 5
SWM_TINY    = 3

# Default target
default: all

CFLAGS += -pipe

#
# Compiler flags
CFLAGS += -fpic -pipe -O2 -DKEY_FROM_DFB

ifeq "$(TOOL_CHAIN)" "4.4.1"
CFLAGS += -fno-dwarf2-cfi-asm
else
ifeq "$(TOOL_CHAIN)" "4.2.1"
CFLAGS += -Wall
endif
endif

ifeq "$(TOOL_CHAIN)" "5.5.0"
CFLAGS += -funwind-tables
endif

ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
ZLIB_VERSION                  ?= 1.2.5
else
ZLIB_VERSION                  ?= 1.2.11
endif

$(warning PNG_VERSION=$(PNG_VERSION)000)
ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
PNG_VERSION                   ?= 1.2.46
else
PNG_VERSION                   ?= 1.2.43
endif

$(warning PNG_VERSION=$(PNG_VERSION)333)

JPEG_VERSION                  ?= 6b
#FREETYPE_VERSION                    ?= 2.3.9
ifeq "$(ICU_SUPPORT)" "true"
FREETYPE_VERSION       ?= 2.3.9
else
FREETYPE_VERSION       ?= 2.4.3
endif

#
# Linker flags

VFP_SUFFIX =

ifeq "$(TOOL_CHAIN)" "4.5.1"
	ifeq "$(ENABLE_VFP)" "true"
		VFP_SUFFIX = _vfp
	else 
		ifeq "$(ENABLE_CA9)" "true"
			VFP_SUFFIX = _vfp_ca9
		endif
	endif	
else
    ifneq (,$(findstring $(TOOL_CHAIN), 4.8.2 5.5.0))
		ifeq "$(ENABLE_CA9)" "true"
			VFP_SUFFIX = _vfp_ca9
		else ifeq "$(ENABLE_CA9_NEON)" "true"
				VFP_SUFFIX = _neon_ca9
		else ifeq "$(ENABLE_CA15_NEON)" "true"
                VFP_SUFFIX = _neon_ca15
		endif	
	else		
		ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
			ifeq "$(ENABLE_CA9_NEON)" "true"			
				VFP_SUFFIX = _neon_ca9
			endif		
		else
			ifeq "$(ENABLE_VFP)" "true"
				VFP_SUFFIX = _vfp
			endif
		endif
	endif	
endif

LIB_SUB_ROOT  = library/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
THIRDPARTY_LIB_ROOT = $(CHILING_ROOT)/../oss/$(LIB_SUB_ROOT)


LDFLAGS += -L.
LDFLAGS += -L$(THIRDPARTY_LIB_ROOT)/zlib/$(ZLIB_VERSION)/pre-install/lib
LDFLAGS += -L$(THIRDPARTY_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install/lib
LDFLAGS += -L$(THIRDPARTY_LIB_ROOT)/jpeg/$(JPEG_VERSION)/pre-install/lib
LDFLAGS += -L$(THIRDPARTY_LIB_ROOT)/png/$(PNG_VERSION)/pre-install/lib
LDFLAGS += -L$(OSS_LIB_ROOT)/expat/$(EXPAT_VERSION)/lib
LDFLAGS += -L$(OSS_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install/lib
LDFLAGS += -L$(OBJ_ROOT)


#
# Configuration header files
INCLUDES += -I$(TOP)/Config_$(CONFIG)

#
# Fusion Kernel module headers
INCLUDES += -I$(KERNEL)/linux-fusion/linux/include
INCLUDES += -I$(THIRDPARTY_LIB_ROOT)/zlib/$(ZLIB_VERSION)/pre-install/include
INCLUDES += -I$(THIRDPARTY_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install/include
INCLUDES += -I$(THIRDPARTY_LIB_ROOT)/freetype/$(FREETYPE_VERSION)/pre-install/include/freetype2
INCLUDES += -I$(THIRDPARTY_LIB_ROOT)/jpeg/$(JPEG_VERSION)/pre-install/include
INCLUDES += -I$(THIRDPARTY_LIB_ROOT)/png/$(PNG_VERSION)/pre-install/include
INCLUDES += -I$(CHILING_ROOT)/app_if/mtal/include
INCLUDES += -I$(CHILING_ROOT)/app_if/mtal/mtal_inc

#
# Preprocessor flags
CPPFLAGS += \
	-DPIC								\
	-DHAVE_CONFIG_H							\
	-DHAVE_STDBOOL_H						\
	-D_GNU_SOURCE							\
	-D_REENTRANT							\
	-DVERSION=\"$(VERSION)\"					\
	-DBUILDTIME="\"$(buildtime)\""\
	-DHF_MW_MODIFY

#
# Debug option

BUILD_CFG ?=

ifeq ($(BUILD_CFG),debug)
  CPPFLAGS += -DENABLE_DEBUG=1
  CFLAGS   += -g3
else
  CPPFLAGS += -DENABLE_DEBUG=0
endif

CPPFLAGS += -DCC_HW_WINDOW_SUPPORT=1
PARALLEL  ?= 12

#
# Trace option
ifeq ($(TRACE),yes)
  CFLAGS   += -finstrument-functions
  CPPFLAGS += -DENABLE_TRACE=1
else
  CPPFLAGS += -DENABLE_TRACE=0
endif

ifeq "$(USRDRV)" "true"
DEFINES += -D__USERDRV__
endif

#
# Include the specific configuration #########################
-include $(TOP)/Config_$(CONFIG)/makefile

CPPFLAGS += \
	-DDATADIR=\"$(datadir)\"	\
	-DMODULEDIR=\"$(moddir)\"	\
	-DSYSCONFDIR=\"$(sysconfdir)\"

buildtime = $(shell sh -c date -u +%Y-%m-%d %H:%M)

-include $(TOP)/directfb_$(DIRECTFB_VERSION)/directfb.mak
-include $(TOP)/directfb_$(DIRECTFB_VERSION)/sawman.mak
-include $(TOP)/directfb_$(DIRECTFB_VERSION)/divine.mak
-include $(TOP)/directfb_$(DIRECTFB_VERSION)/fusiondale.mak


# -include $(TOP)/directfb_$(DIRECTFB_VERSION)/fusiondale.mak

#
# Build list of all objects


OBJECTS +=\
	$(LIB_DIRECT_OBJECTS)\
	$(LIB_FUSION_OBJECTS)\
	$(LIB_FUSION_OBJECTS_MULTI)\
	$(LIB_VOODOO_OBJECTS)\
	$(DIRECTFB_OBJECTS)\
	$(PROXY_OBJECTS)\
	$(SAWMAN_OBJECTS)\
	$(DIVINE_PROXY_OBJECTS)\
	$(DIVINE_OBJECTS)\
	$(FUSIONDALE_OBJECTS)



#
# Build complete include path
INCLUDES += \
	$(DIRECTFB_INCLUDES)	\
	$(SAWMAN_INCLUDES) \
	$(FUSIONDALE_INCLUDES) \
	$(DIVINE_INCLUDES)	



#
# All libraries
LIBS += \
	-lfreetype							\
	-lpng								\
	-ljpeg								\
	-lz								\
	-lm								\
	-lpthread							\
	-ldl
ifneq (,$(findstring $(TOOL_CHAIN), 4.9.1 4.9.2))
LIBS += -lpthread_nonshared
endif 

$(warning PNG_VERSION=$(PNG_VERSION)555)
$(warning PNG_PATJ= $(THIRDPARTY_LIB_ROOT)/png/$(PNG_VERSION)/pre-install/lib)
TOOLS += \
# 	$(SOURCE)/DirectFB/tools/dfbdump
#	$(SOURCE)/DirectFB/tools/fusion_bench\
#	$(SOURCE)/DirectFB/tests/fusion_reactor\
#	$(SOURCE)/SaWMan/tools/swmdump\
#	$(SOURCE)/DirectFB/tools/dfbinfo\

#
# List of generated header files
GEN_HEADERS = 

OBJECTS     := $(addprefix $(OBJ_ROOT)/,$(OBJECTS))
GEN_HEADERS := $(addprefix $(OBJ_ROOT)/,$(GEN_HEADERS))
TOOLS_OBJS  := $(addsuffix .o, $(addprefix $(OBJ_ROOT)/,$(TOOLS)))
TOOLS 		:= $(addprefix $(OBJ_ROOT)/tools/, $(notdir $(TOOLS)))


V ?= 0

ifeq "$(V)" "0"
QUIET = @
else
QUIET = 
endif

define rule_cc_o_c
	@/bin/echo -e "  CC\t$@"
	@mkdir -p $(dir $@)
	@/bin/echo -E '$(CC) $(CFLAGS) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<' > $@.cmd
	$(QUIET)$(CC) $(CFLAGS) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<
endef

define rule_cc_o_cpp
	@/bin/echo -e "  CXX\t$@"
	@mkdir -p $(dir $@)
	@/bin/echo -E '$(CXX) $(CFLAGS) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<' > $@.cmd
	$(QUIET)$(CXX) $(CFLAGS) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<
endef

define rule_cc_o_ld
	@/bin/echo -e "  LD\t$@"
	$(QUIET)$(CC) $(LDFLAGS) -o $@ $< -ldirectfb -lstdc++ $(INCLUDES) $(LIBS)
endef

#
# Rules
all:  gen_obj 
	$(MAKE) libdirectfb.so fusion.ko $(TOOLS)

gen_obj:$(GEN_HEADERS)
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/mt53_fb.h
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/mt53xx_fb.h
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/gfx_common_if.h
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/gfx/gfx_common_if.h
	rm -rf $(CHILING_ROOT)/driver/directfb/Source
	if [ ! -d $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx ]; then \
		mkdir -p $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx; \
	fi
	if [ ! -d $(OBJ_ROOT)/Source/DirectFB/include ]; then \
		mkdir -p $(OBJ_ROOT)/Source/DirectFB/; \
		cp -au $(TOP)/directfb_$(DIRECTFB_VERSION)/Source/DirectFB/include $(OBJ_ROOT)/Source/DirectFB/include ; \
		cp -au $(TOP)/directfb_$(DIRECTFB_VERSION)/Source/DirectFB/lib $(OBJ_ROOT)/Source/DirectFB/lib ; \
	fi
	if [ ! -d $(OBJ_ROOT)/Source/FusionDale/include ]; then \
		mkdir -p $(OBJ_ROOT)/Source/FusionDale/; \
		cp -au $(TOP)/directfb_$(DIRECTFB_VERSION)/Source/FusionDale/include $(OBJ_ROOT)/Source/FusionDale/include ; \
	fi
	if [ ! -d $(OBJ_ROOT)/tools ]; then \
		mkdir -p $(OBJ_ROOT)/tools/; \
	fi
	ln -s $(CHILING_ROOT)/driver/directfb/directfb_$(DIRECTFB_VERSION)/Source $(CHILING_ROOT)/driver/directfb/
	ln -s $(CHILING_ROOT)/../target/open/gfx/gfx6896/gfx_common_if.h  $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/gfx_common_if.h
	ln -s $(CHILING_ROOT)/../target/open/gfx/gfx6896/gfx_common_if.h  $(CHILING_ROOT)/driver/directfb/MediaTek/gfx/gfx_common_if.h
	ln -s $(CHILING_ROOT)/driver/linux/native/mt53xx_fb/mt53_fb.h  $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/mt53_fb.h
	$(MAKE) $(if $(filter -j,$(MAKEFLAGS)),,-j $(PARALLEL)) parallel_obj --no-print-directory
	
parallel_obj: $(OBJECTS) $(TOOLS_OBJS)

libdirectfb.o:  $(GEN_HEADERS) $(OBJECTS)
	@/bin/echo -e "  LD\t$@"
	$(QUIET)$(LD) $(LDFLAGS) -r -o $(OBJ_ROOT)/$@ $(OBJECTS)

libdirectfb.a:  $(GEN_HEADERS) $(OBJECTS)
	@/bin/echo -e "  AR\t$@"
	$(QUIET)$(AR) -ru $(OBJ_ROOT)/$@ $(OBJECTS)

libdirectfb.so: $(GEN_HEADERS) $(OBJECTS)
	@/bin/echo -e "  LD\t$@"
	$(QUIET)$(CC) -shared -Wl,-soname,libdirectfb.so $(LDFLAGS)  -o $(OBJ_ROOT)/$@ $(OBJECTS) $(LIBS)

libdirectfb.so.nm: libdirectfb.so
	@/bin/echo -e "  NM\t$@"
	$(QUIET)$(NM) -n $(OBJ_ROOT)/$< > $(OBJ_ROOT)/$@

$(OBJ_ROOT)/%.o: %.c
	$(rule_cc_o_c)

$(OBJ_ROOT)/%.o: %.cpp
	$(rule_cc_o_cpp)
	
$(OBJ_ROOT)/$(SOURCE)/DirectFB/tests/%: $(OBJ_ROOT)/$(SOURCE)/DirectFB/tests/%.o libdirectfb.so
	$(rule_cc_o_ld)

$(OBJ_ROOT)/$(SOURCE)/DirectFB/tools/%: $(OBJ_ROOT)/$(SOURCE)/DirectFB/tools/%.o libdirectfb.so
	$(rule_cc_o_ld)

$(OBJ_ROOT)/$(SOURCE)/SaWMan/tools/%: $(OBJ_ROOT)/$(SOURCE)/SaWMan/tools/%.o libdirectfb.so
	$(rule_cc_o_ld)

$(OBJ_ROOT)/tools/%: $(OBJ_ROOT)/$(MEDIATEK)/Tools/%.o libdirectfb.so
	$(rule_cc_o_ld)

%.nm: %
	@/bin/echo -e "  NM\t$@"
	@mkdir -p $(dir $@)
	$(QUIET)$(NM) -n $< > $@

fusion.ko:
	$(MAKE) -C $(KERNEL) $(if $(filter -j,$(MAKEFLAGS)),,-j $(PARALLEL)) $(KERNEL_MAKE_OPTION)

.PHONY: clean install gen_obj

clean::
	$(FCLEAN) $(OBJ_ROOT)
	make -C $(KERNEL) clean
	rm -f libdirectfb.*o* $(TOOLS) $(TEST)
	rm -f libdirectfb.a
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/mt53_fb.h
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx/gfx_common_if.h
	rm -f $(CHILING_ROOT)/driver/directfb/MediaTek/gfx/gfx_common_if.h
	rm -rf $(CHILING_ROOT)/driver/directfb/MediaTek/mt53xx
	rm -rf $(CHILING_ROOT)/driver/directfb/Source
	find -name "*.o" | xargs rm -rf
	find -name "*.so" | xargs rm -rf
	find -name "*.nm" | xargs rm -rf
	find -name "modules.order" | xargs rm -rf
	find -name "Module.markers" | xargs rm -rf
	find -name ".tmp_versions" | xargs rm -rf


ifdef KERNEL_CROSS_COMPILE
STRIP := $(lastword $(KERNEL_CROSS_COMPILE)strip)
endif

install::
	@(if test -z "$(DESTDIR)"; then echo "**** DESTDIR not set! ****"; exit 1; fi)
	install -d $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(libdir)
	install -d $(DESTDIR)$(moddir)
	install -d $(DESTDIR)$(moddir)/inputdrivers
	install -d $(DESTDIR)$(moddir)/interfaces
	install -d $(DESTDIR)$(moddir)/gfxdrivers
	install -d $(DESTDIR)$(moddir)/systems
	install -d $(DESTDIR)$(moddir)/wm
	

#	install -c -m 755 $(TEST) $(TOOLS) $(DESTDIR)$(bindir)/

ifneq "$(STATIC_LIB)" "true"
	install -c -m 755 $(OBJ_ROOT)/libdirectfb.so $(DESTDIR)$(libdir)/
else
	rm -f $(OBJ_ROOT)/libdirectfb.so
endif

ifneq "$(UTOPIA)" "true"
	install -c -m 755 $(TOOLS) $(DESTDIR)$(bindir)/
endif

#	@touch $(OBJ_ROOT)/fusion.ko
#	install -c -m 755 $(OBJ_ROOT)/fusion.ko $(DESTDIR)$(bindir)/
ifneq "$(STATIC_LIB)" "true"
	$(STRIP) --strip-unneeded $(DESTDIR)$(libdir)/libdirectfb.so
ifneq "$(UTOPIA)" "true"
	$(STRIP) --strip-unneeded $(addprefix $(DESTDIR)$(bindir)/, $(notdir $(TEST) $(TOOLS)))
endif
	echo ------------

endif

DFBINC  = $(SOURCE)/DirectFB/include

