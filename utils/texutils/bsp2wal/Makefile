# GNU Makefile for hexen2 bsp2wal tool using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/texutils/bsp2wal/Makefile,v 1.7 2007-10-25 09:06:56 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../../..
# where the common sources/objects are
COMMONDIR:=../../common
# where to put the binaries
DESTDIR  :=../../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY:=bsp2wal.exe
endif
ifeq ($(TARGET_OS),unix)
BINARY:=bsp2wal
endif

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
CFLAGS = $(CPUFLAGS) -O2 -Wall
endif

# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(MINGWDIR)/include -I$(COMMONDIR)
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
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<
bspfile.o: $(COMMONDIR)/bspfile.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
BSP2WALOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	bspfile.o \
	bsp2wal.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(BSP2WALOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(BSP2WALOBJS) $(LDFLAGS) $(LDLIBS)


clean : 
	rm -f *.o $(COMMONDIR)/*.o core

