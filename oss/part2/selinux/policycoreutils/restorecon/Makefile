SECURE_HARDENING_FLAGS ?= -O2 -fstack-protector-all -D_FORTIFY_SOURCE=2 -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -s -fPIE -pie

override CFLAGS += $(SECURE_HARDENING_FLAGS) -I../../libselinux/include -I../../libsepol/include -I$(LIBPCRE_MODULE_ROOT)/result/include
override LDLIBS += -L../../libsepol/src -lsepol -L../../libselinux/src -lselinux -L$(LIBPCRE_MODULE_ROOT)/result/lib -lpcre

all: restorecon

clean:
	-rm -f restorecon *.o
