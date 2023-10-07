
URIPARSER_VERSION := 0.9.3

URIPARSER_OUTPUT := $(OSS_OUTPUT)/uriparser
URIPARSER_SOURCE := $(URIPARSER_OUTPUT)/uriparser-$(URIPARSER_VERSION)
URIPARSER_BUILD  := $(URIPARSER_OUTPUT)/uriparser-$(URIPARSER_VERSION)-build
URIPARSER_FILE := uriparser-$(URIPARSER_VERSION).tar.bz2

LD_LIBRARY_PATH :=
JOBS ?= $(shell grep processor /proc/cpuinfo | wc -l)
URIPARSER_CFLAGS := -g -fPIC -O2 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now
	
all: uriparser_prepare
	if [ ! -d $(URIPARSER_BUILD) ]; then \
	    mkdir -p $(URIPARSER_BUILD)	; \
	fi 
	cd $(URIPARSER_BUILD) && \
	cmake -DCMAKE_BUILD_TYPE=Release \
	-DURIPARSER_BUILD_TESTS=OFF \
	-DURIPARSER_BUILD_DOCS=OFF \
	-DURIPARSER_BUILD_WCHAR_T=OFF \
	-DURIPARSER_BUILD_TOOLS=OFF \
	-DCMAKE_INSTALL_PREFIX=$(OSS_LIB_ROOT)/uriparser/$(URIPARSER_VERSION) \
	-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
	-DCMAKE_C_COMPILER_AR=$(CROSS_COMPILE)ar rus \
	-DCMAKE_C_COMPILER_RANLIB=$(CROSS_COMPILE)ranlib \
	-DCMAKE_C_FLAGS="$(URIPARSER_CFLAGS) $(PLATFORM_CFLAGS)"
	make -j $(JOBS) -C $(URIPARSER_BUILD)

install:
	make -C $(URIPARSER_BUILD) install

clean:
	make -C $(URIPARSER_BUILD) clean
	rm -rf $(URIPARSER_OUTPUT)

uriparser_prepare:
	if [ ! -d $(URIPARSER_SOURCE) ]; then \
		mkdir -p $(URIPARSER_SOURCE); \
		tar xvjf $(URIPARSER_FILE) -C $(URIPARSER_OUTPUT); \
		chmod -R 755 ${URIPARSER_SOURCE}; \
		module load md32/dev/cmake/3.10.1; \
	fi

notice:
	cp -rf NOTICE.$(URIPARSER_VERSION).txt $(OSS_NOTICE_OUTPUT)/uriparser_NOTICE.txt

info:
	cp -rf OSS_INFO.$(URIPARSER_VERSION).txt $(OSS_INFO_OUTPUT)/uriparser_INFO.txt