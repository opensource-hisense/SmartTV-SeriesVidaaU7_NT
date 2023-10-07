RNGTOOLS_OUTPUT := $(OSS_OUTPUT)/rng-tools
RNGTOOLS_SOURCE := $(RNGTOOLS_OUTPUT)/rng-tools-5
RNGTOOLS_BUILD  := $(RNGTOOLS_OUTPUT)/build
RNGTOOLS_FILE   := rng-tools-5.tar.gz

LD_LIBRARY_PATH :=
JOBS ?= $(shell grep processor /proc/cpuinfo | wc -l)
GLIBC_CFLAGS := -g -fno-exceptions -O2 
	
all: rngtools_prepare
	if [ ! -d $(RNGTOOLS_BUILD) ]; then \
	    mkdir -p $(RNGTOOLS_BUILD)	; \
	fi 
	cd $(RNGTOOLS_BUILD) && \
	CC="$(CROSS_COMPILE)gcc" CFLAGS="$(GLIBC_CFLAGS) $(PLATFORM_CFLAGS)" AR="$(CROSS_COMPILE)ar rus" RANLIB="$(CROSS_COMPILE)ranlib" \
	$(RNGTOOLS_SOURCE)/configure \
		--prefix=$(OSS_LIB_ROOT)/rng-tools \
		--host=$(PLATFORM_ARCH) 			\
    --target=$(PLATFORM_ARCH) \
    --build=`uname -m`-linux
    
	make -j $(JOBS) -C $(RNGTOOLS_BUILD)
		
install:
	make -C $(RNGTOOLS_BUILD) install

clean:
	rm -rf $(RNGTOOLS_OUTPUT)

rngtools_prepare:
	if [ ! -d $(RNGTOOLS_SOURCE) ]; then \
		mkdir -p $(RNGTOOLS_SOURCE); \
		tar zxf $(RNGTOOLS_FILE) -C $(RNGTOOLS_OUTPUT); \
                chmod -R 755 ${RNGTOOLS_SOURCE} ; \
	fi

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/rng-tools_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/rng-tools_INFO.txt

