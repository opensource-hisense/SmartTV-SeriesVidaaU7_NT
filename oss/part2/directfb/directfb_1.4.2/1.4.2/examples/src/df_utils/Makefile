# **********************************************
# Include Path
# **********************************************
THEALE_PATH=$(HOME)/THEALE

PHOTOSPHERE_ROOT=$(HOME)/DAILEO/Supernova


SHARED_LIB_PATH=$(PHOTOSPHERE_ROOT)/target/$(PROJ_MODE).$(CHIP)/mslib

DFB_NEEDED_PATH=$(PHOTOSPHERE_ROOT)/develop/include
DFB_PUBLIC_HEADER_PATH=$(PHOTOSPHERE_ROOT)/develop/include/dfb/target/directfb

UTOPIA_LIB_PATH=$(PHOTOSPHERE_ROOT)/target/$(PROJ_MODE).$(CHIP)/mslib/utopia/
UTOPIA_INCLUDE_PATH=$(PHOTOSPHERE_ROOT)/develop/include/utopia/$(CHIP)



# For 1.4.2/examples & 1.4.2/tools

KERNEL_VERSION=2.6.35.8
KERNEL_PATH=$(THEALE_PATH)/RedLion/$(KERNEL_VERSION)
KERNEL_INC_PATH=$(KERNEL_PATH)/include

DIRECTFB_EXAMPLE_SRC=$(THEALE_PATH)/RedLion/Lunar/directfb/1.4.2/examples/src
DIRECTFB_ROOT=$(THEALE_PATH)/RedLion/Lunar/directfb/1.4.2
DIRECTFB_SRC=$(DIRECTFB_ROOT)/src


# **********************************************
# Reference Libs
# **********************************************

ifeq ($(CHIP), napoli)
	MSTAR_LIB=-lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux  -ldrvVE -lapiXC -lapiPNL -ldrvWDT -ldrvSAR -lapiSWI2C -ldrvGPIO -ldrvCPU
else
	MSTAR_LIB=-lrt -lpthread -ldrvMVOP -lapiGFX -lapiGOP -llinux  -ldrvVE -lapiXC -lapiPNL -ldrvWDT -ldrvSAR
endif


HW_DECODEJPEG_LIBS=-lapiVDEC -ldrvIPAUTH -lapiJPEG
HW_DECODEGIF_LIBS=-lapiGPD
LD_LIB_PATH=-L$(SHARED_LIB_PATH) -L$(UTOPIA_LIB_PATH) -lz -lm
DIRECTFB_LIBS_SETTING=-L$(SHARED_LIB_PATH)/lib -ldirect -lfusion -ldirectfb -lpthread


# **********************************************
# Resource
# **********************************************
FONT=./rec/decker.ttf
DATADIR=./rec


# **********************************************
# Rules
# **********************************************
ifeq ($(TARGET_CPU), mips)
    ifeq ($(TOOLCHAIN), mips-glibc)
    TOOLCHAIN_ALIAS = mips-linux-gnu
    CXX  = $(TOOLCHAIN_ALIAS)-g++ -EL
    CC  = $(TOOLCHAIN_ALIAS)-gcc -EL
    endif
    ifeq ($(TOOLCHAIN), mips-uclibc)
    TOOLCHAIN_ALIAS = mips-linux-gnu-uclibc
    CXX  = $(TOOLCHAIN_ALIAS)-g++ -EL
    CC  = $(TOOLCHAIN_ALIAS)-gcc -EL
    endif
endif

ifeq ($(TARGET_CPU), arm)
    ifeq ($(TOOLCHAIN), arm-gnueabi)
    TOOLCHAIN_ALIAS = arm-none-linux-gnueabi
    CXX  = $(TOOLCHAIN_ALIAS)-g++
    CC  = $(TOOLCHAIN_ALIAS)-gcc
    endif
    ifeq ($(TOOLCHAIN), arm-uclibc)
    TOOLCHAIN_ALIAS = arm-linux-uclibc
    CXX  = $(TOOLCHAIN_ALIAS)-g++
    CC  = $(TOOLCHAIN_ALIAS)-gcc
    endif
endif


QUIET = @
ECHO  = echo
CPPFLAGS=-g -DDATADIR=\"$(DATADIR)\" -DFONT=\"$(FONT)\" -I$(DIRECTFB_ROOT) -I$(DFB_NEEDED_PATH) -I$(DFB_PUBLIC_HEADER_PATH) -I$(KERNEL_INC_PATH) -I./ -I$(UTOPIA_INCLUDE_PATH) -I$(DIRECTFB_EXAMPLE_SRC) -I$(DIRECTFB_SRC)
CFLAGS=-g -DDATADIR=\"$(DATADIR)\" -DFONT=\"$(FONT)\" -I$(DIRECTFB_ROOT) -I$(DFB_NEEDED_PATH) -I$(DFB_PUBLIC_HEADER_PATH) -I$(KERNEL_INC_PATH) -I./ -I$(UTOPIA_INCLUDE_PATH) -I$(DIRECTFB_EXAMPLE_SRC) -I$(DIRECTFB_SRC)
LDFLAGS=$(LD_LIB_PATH) $(DIRECTFB_LIBS_SETTING) $(MSTAR_LIB) $(HW_DECODEJPEG_LIBS) $(HW_DECODEGIF_LIBS)
RM = rm -rf
TARGET = test
OBJS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))


all: $(TARGET)

$(TARGET): $(OBJS) 
	$(QUIET)$(ECHO) "Building " $@
	$(QUIET)$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(QUIET)$(ECHO) "Compliling " $<
	$(QUIET)$(CXX) $(CPPFLAGS) -c $< -o $@


%.o: %.c
	$(QUIET)$(ECHO) "Compliling " $<
	$(QUIET)$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(QUIET)$(RM) $(OBJS) $(TARGET)
