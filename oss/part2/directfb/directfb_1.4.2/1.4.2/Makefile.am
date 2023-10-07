## Makefile.am for DirectFB

ACLOCAL_AMFLAGS = -I m4

if HAVE_LINUX
LINUXONLY_DIRS = inputdrivers gfxdrivers
endif

if BUILD_TOOLS
TOOLS_DIR = tools
endif

if BUILD_TESTS
TESTS_DIR = tests
endif

if ENABLE_VOODOO
PROXY_DIR = proxy
endif

SUBDIRS = \
	data			\
	include 		\
	lib			\
	src 			\
	systems			\
	$(TOOLS_DIR)		\
	wm			\
	interfaces 		\
	$(PROXY_DIR)		\
	$(LINUXONLY_DIRS)	\
	$(TESTS_DIR)		\
	patches			\
	rules

bin_SCRIPTS = directfb-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = directfb.pc directfb-internal.pc

EXTRA_DIST = \
	autogen.sh	\
	fb.modes 		\
	directfb.spec.in	\
	directfb.spec


## Some special rules that may be useful ...


# Compile the directfb-csource utility:
directfb-csource:
	make -C tools directfb-csource

package_dir_=directfb-1.4.2m
package_name_=$(package_dir_).tar.gz
package:
	@echo "Package the target $(package_name_)..."
	@echo "run auto insert GIT change-ID by versionInfo.sh"
	@-./versionInfo.sh
	@rm -rf $(package_dir_)
	@rm -rf $(package_name_)
	@mkdir $(package_dir_)
	@cp -arf $(DESTDIR)/$(prefix)/bin $(DESTDIR)/$(prefix)/include $(DESTDIR)/$(prefix)/lib $(DESTDIR)/$(prefix)/share $(package_dir_)
	@echo "add CL Version Label to Library"
	@for file in `find $(package_dir_) -name '*.so'`; \
	do $(host_alias)-objcopy --add-section .mmodule_version=./version_info $$file; \
	done
	@tar -zcf $(package_name_) $(package_dir_)
	@echo "Done!"


.PHONY: directfb-csource
