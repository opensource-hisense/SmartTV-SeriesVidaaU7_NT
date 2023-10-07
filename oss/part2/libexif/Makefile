LIBEXIF_OUTPUT := $(OSS_OUTPUT)/libexif
LIBEXIF_SOURCE := $(LIBEXIF_OUTPUT)/libexif-$(LIBEXIF_VERSION)
LIBEXIF_BUILD  := $(LIBEXIF_OUTPUT)/libexif-$(LIBEXIF_VERSION)-build
LIBEXIF_FILE := libexif-$(LIBEXIF_VERSION).tar.gz

LD_LIBRARY_PATH :=
JOBS ?= $(shell grep processor /proc/cpuinfo | wc -l)
LIBEXIF_CFLAGS := -O2 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s

all: libexif_prepare
	if [ ! -d $(LIBEXIF_BUILD) ]; then \
		mkdir -p $(LIBEXIF_BUILD)	; \
	fi
	cd $(LIBEXIF_BUILD) && \
	CC="$(CROSS_COMPILE)gcc" CFLAGS="$(LIBEXIF_CFLAGS) $(PLATFORM_CFLAGS)" LD="$(CROSS_COMPILE)ld" AR="$(CROSS_COMPILE)ar" RANLIB="$(CROSS_COMPILE)ranlib" \
	$(LIBEXIF_SOURCE)/configure \
		--prefix=$(OSS_LIB_ROOT)/libexif/$(LIBEXIF_VERSION) \
		--disable-static --disable-docs \
		--host=arm-linux \
		--target=arm-linux \
		--build=i686-pc-linux-gnu
	make -j $(JOBS) -C $(LIBEXIF_BUILD)

install:
	make -C $(LIBEXIF_BUILD) install

clean:
	rm -rf $(LIBEXIF_OUTPUT)

libexif_prepare:
	echo "Prepare..."
	if [ ! -d $(LIBEXIF_SOURCE) ]; then \
		mkdir -p $(LIBEXIF_SOURCE); \
		tar zxf $(LIBEXIF_FILE) -C $(LIBEXIF_OUTPUT); \
				chmod -R 755 ${LIBEXIF_SOURCE} ; \
		cd $(LIBEXIF_SOURCE); \
		autoreconf -i; \
	fi

notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/libexif_NOTICE.txt

info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/libexif_INFO.txt

