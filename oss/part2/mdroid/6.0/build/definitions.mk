#
# File          definitions.mk
# Title         Specify compiling/assembly/archiving/linkng/... marcos.
# Author        Jimmy Hsu
#
# Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
#

EXECUTABLE_SUFFIX :=
SHARE_LIB_SUFFIX := .so
STATIC_LIB_SUFFIX := .a
comma := ,

built_shared_libraries = \
    $(addprefix $(LIBRARY_DIR)/, \
      $(addsuffix $(SHARE_LIB_SUFFIX), \
        $(LOCAL_SHARED_LIBRARIES)))

built_static_libraries = \
    $(foreach lib,$(LOCAL_STATIC_LIBRARIES), \
      $(LIBRARY_DIR)/$(lib)$(STATIC_LIB_SUFFIX))

built_whole_libraries = \
    $(foreach lib,$(LOCAL_WHOLE_STATIC_LIBRARIES), \
      $(LIBRARY_DIR)/$(lib)$(STATIC_LIB_SUFFIX))

all_objects = \
    $(ASM_OBJECTS) \
    $(CXX_OBJECTS) \
    $(CPP_OBJECTS)

PRIVATE_ALL_SHARED_LIBRARIES = $(built_shared_libraries)
PRIVATE_ALL_STATIC_LIBRARIES = $(built_static_libraries)
PRIVATE_ALL_WHOLE_STATIC_LIBRARIES = $(built_whole_libraries)
PRIVATE_ALL_OBJECTS = $(all_objects)

PRIVATE_GROUP_STATIC_LIBRARIES = $(LOCAL_GROUP_STATIC_LIBRARIES)

CFLAGS = $(PLAT_CFLAGS) $(PROJ_CFLAGS) $(LOCAL_CFLAGS)
CPPFLAGS = $(PLAT_CPPFLAGS) $(PROJ_CPPFLAGS) $(LOCAL_CPPFLAGS)
ASFLAGS = $(PLAT_ASFLAGS) $(PROJ_ASFLAGS) $(LOCAL_ASFLAGS)
LDFLAGS = $(PLAT_LDFLAGS) $(LOCAL_LDFLAGS)
ARFLAGS = $(PLAT_ARFLAGS) $(LOCAL_ARFLAGS)

###########################################################
## Convert "path/to/libXXX.so" to "-lXXX".
## Any "path/to/libXXX.a" elements pass through unchanged.
###########################################################

define normalize-libraries
$(foreach so,$(filter %.so,$(1)),-l$(patsubst lib%.so,%,$(notdir $(so))))\
$(filter-out %.so,$(1))
endef

define normalize-target-libraries
$(call normalize-libraries,$(1))
endef

###########################################################
## Commands for munging the dependency files GCC generates
###########################################################
define transform-d-to-p
@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	rm -f $(@:%.o=%.d)
endef

###########################################################
## Commands for running gcc to compile a C file
###########################################################
# $(1): extra flags
define transform-c-or-s-to-o-no-deps
@mkdir -p $(dir $@)
@$(TARGET_CC) \
	-c \
	$(CFLAGS) \
	$(1) \
	-MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
endef

define transform-c-to-o-no-deps
@echo ">> Compiling C <= $<"
$(call transform-c-or-s-to-o-no-deps, )
endef

define transform-s-to-o-no-deps
@echo ">> Assembly <= $<"
$(call transform-c-or-s-to-o-no-deps, $(ASFLAGS))
endef

define transform-c-to-o
$(transform-c-to-o-no-deps)
$(transform-d-to-p)
endef

define transform-s-to-o
$(transform-s-to-o-no-deps)
$(transform-d-to-p)
endef

###########################################################
## Commands for running gcc to compile a C++ file
###########################################################
define transform-cpp-to-o
@mkdir -p $(dir $@)
@echo ">> Compiling C++ <= $<"
@$(TARGET_CXX) \
	-c \
	$(CFLAGS) \
    $(CPPFLAGS) \
	-MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
$(transform-d-to-p)
endef


###########################################################
## Commands for running ar
###########################################################
define _concat-if-arg2-not-empty
$(if $(2),@ $(1) $(2))
endef

# Split long argument list into smaller groups and call the command repeatedly
#
# $(1): the command without arguments
# $(2): the arguments
define split-long-arguments
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 1,500,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 501,1000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 1001,1500,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 1501,2000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 2001,2500,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 2501,3000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 3001,99999,$(2)))
endef

# $(1): the full path of the source static library.
define _extract-and-include-single-target-whole-static-lib
@echo "preparing StaticLib: [including $(1)]"
@ ldir=$(OBJECT_DIR)/WHOLE/$(basename $(notdir $(1)))_objs;\
    rm -rf $$ldir; \
    mkdir -p $$ldir; \
    filelist=; \
    for f in `$(TARGET_AR) t $(1)`; do \
        $(TARGET_AR) p $(1) $$f > $$ldir/$$f; \
        filelist="$$filelist $$ldir/$$f"; \
    done ; \
    $(TARGET_AR) $(ARFLAGS) $@ $$filelist

endef

define extract-and-include-target-whole-static-libs
$(foreach lib,$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES), \
    $(call _extract-and-include-single-target-whole-static-lib, $(lib)))
endef

# Explicitly delete the archive first so that ar doesn't
# try to add to an existing archive.
define transform-o-to-static-lib
@mkdir -p $(dir $@)
@rm -f $@
$(extract-and-include-target-whole-static-libs)
@echo "Archiving StaticLib: ($@)"
$(call split-long-arguments,$(TARGET_AR) $(TARGET_GLOBAL_ARFLAGS) $(PRIVATE_ARFLAGS) $@,$(filter %.o, $^))
endef

###########################################################
## Commands for running gcc to link a shared library or package
###########################################################

#echo >$@.vers "{"; \
#echo >>$@.vers " global:"; \
#$(BUILD_SYSTEM)/filter_symbols.sh $(TARGET_NM) "  " ";" $(filter %.o,$^) | sort -u >>$@.vers; \
#echo >>$@.vers " local:"; \
#echo >>$@.vers "  *;"; \
#echo >>$@.vers "};"; \

#	-Wl,--version-script=$@.vers \

# ld just seems to be so finicky with command order that we allow
# it to be overriden en-masse see combo/linux-arm.make for an example.
ifneq ($(TARGET_CUSTOM_LD_COMMAND),true)
define transform-o-to-shared-lib-inner
@$(TARGET_CXX) \
	$(LDFLAGS) \
	-Wl,-rpath-link=$(LIBRARY_DIR) \
	-shared -Wl,-soname,$(notdir $@) \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--whole-archive \
	$(call normalize-target-libraries,$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES)) \
	-Wl,--no-whole-archive \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	-o $@
endef
endif

define transform-o-to-shared-lib
@mkdir -p $(dir $@)
@echo ">> Linkng SharedLib: ($@)"
$(transform-o-to-shared-lib-inner)
endef

###########################################################
## Commands for running gcc to link an executable
###########################################################
ifneq ($(TARGET_CUSTOM_LD_COMMAND),true)
define transform-o-to-executable-inner
@$(TARGET_CXX) \
	$(LDFLAGS) \
	-Wl,-rpath-link=$(LIBRARY_DIR) \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--whole-archive \
	$(call normalize-target-libraries,$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES)) \
	-Wl,--no-whole-archive \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--start-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_STATIC_LIBRARIES)) \
	$(if $(PRIVATE_GROUP_STATIC_LIBRARIES),-Wl$(comma)--end-group) \
	$(call normalize-target-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	-o $@
endef
endif

define transform-o-to-executable
@mkdir -p $(dir $@)
@echo ">> Linkng Executable: ($@)"
$(transform-o-to-executable-inner)
endef

###########################################################
## Commands for running gcc to link a statically linked
## executable.  In practice, we only use this on arm, so
## the other platforms don't have the
## transform-o-to-static-executable defined
###########################################################
ifneq ($(TARGET_CUSTOM_LD_COMMAND),true)
define transform-o-to-static-executable-inner
endef
endif

define transform-o-to-static-executable
@mkdir -p $(dir $@)
@echo ">> Linkng StaticExecutable: ($@)"
$(transform-o-to-static-executable-inner)
endef
