# GNU Makefile for hexen2 bsp2wal tool using GCC.
# $Id$
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# To cross-compile for Win64 on Unix, you must pass the W64BUILD=1
# argument to make. Also see build_cross_win64.sh for details.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs
# where the common sources/objects are
COMMONDIR:=../../common
# where to put the binaries
DESTDIR  :=../../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
BINARY:=bsp2wal$(exe_ext)

# Compiler flags

CPUFLAGS:=
# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPUFLAGS:=-march=i586
endif

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG
CFLAGS := -g -Wall
else
CFLAGS = $(CPUFLAGS) -O2 -Wall -DNDEBUG
endif

CFLAGS := $(CFLAGS) $(ARCHFLAGS)


# Other build flags

ifeq ($(TARGET_OS),win32)
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),unix)
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  :=
endif

# Targets
all : $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
bspfile.o: $(COMMONDIR)/bspfile.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
BSP2WALOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/strlcat.o \
	$(COMMONDIR)/strlcpy.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/byteordr.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	bspfile.o \
	bsp2wal.o

$(BINARY) : $(BSP2WALOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(BSP2WALOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o core

