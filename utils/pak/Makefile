# GNU Makefile for hexen2 pak tools using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/pak/Makefile,v 1.2 2010-02-23 12:40:12 sezero Exp $
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
UHEXEN2_TOP:=../..
# where the common sources/objects are
COMMONDIR:=../common
# where to put the binaries
DESTDIR  :=../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
PAKX:=pakx$(exe_ext)
PAKLIST:=paklist$(exe_ext)

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
all : $(PAKX) $(PAKLIST)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/strlcat.o \
	$(COMMONDIR)/strlcpy.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/crc.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/byteordr.o \
	pakfile.o
PAKXOBJS= pakx.o
PAKLISTOBJS= paklist.o

$(PAKX) : $(COMMONOBJS) $(PAKXOBJS)
	$(LINKER) -o $(DESTDIR)/$(PAKX) $(COMMONOBJS) $(PAKXOBJS) $(LDFLAGS) $(LDLIBS)

$(PAKLIST) : $(COMMONOBJS) $(PAKLISTOBJS)
	$(LINKER) -o $(DESTDIR)/$(PAKLIST) $(COMMONOBJS) $(PAKLISTOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o core

