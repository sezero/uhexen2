# GNU Makefile for hexen2 mapping tools using GCC.
# $Id$
#
# To cross-compile for Win32 on Unix: either pass the W32BUILD=1
# argument to make, or export it.  Also see build_cross_win32.sh.
# Requires: a mingw or mingw-w64 compiler toolchain.
#
# To cross-compile for Win64 on Unix: either pass the W64BUILD=1
# argument to make, or export it. Also see build_cross_win64.sh.
# Requires: a mingw-w64 compiler toolchain.
#
# To cross-compile for MacOSX on Unix: either pass the OSXBUILD=1
# argument to make, or export it.  You would also need to pass a
# suitable MACH_TYPE=xxx (ppc, x86, x86_64, or ppc64) argument to
# make. Also see build_cross_osx.sh.
#
# To (cross-)compile for DOS: either pass the DOSBUILD=1 argument
# to make, or export it. Also see build_cross_dos.sh. Requires: a
# djgpp compiler toolchain.
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# Path settings:
UHEXEN2_TOP:=../..
UTILS_TOP:=..
COMMONDIR:=$(UTILS_TOP)/common
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
QBSP:=qbsp$(exe_ext)

# Compiler flags

# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
CPUFLAGS=$(CPU_X86)

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1 -ffast-math
endif
CFLAGS += -DDOUBLEVEC_T

LDFLAGS =
LDLIBS  =
INCLUDES= -I. -I$(COMMONDIR) -I$(UHEXEN2_SHARED)

# Other build flags

ifeq ($(TARGET_OS),dos)
INCLUDES+= -I$(OSLIBS)/dos
LDFLAGS += -lc -lgcc -lm
endif
ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),morphos)
CFLAGS  += -noixemul
LDFLAGS += -noixemul
LDLIBS  += -lm
endif
ifeq ($(TARGET_OS),darwin)
CPUFLAGS=
# require 10.5 for 64 bit builds
ifeq ($(MACH_TYPE),x86_64)
CFLAGS  +=-mmacosx-version-min=10.5
LDFLAGS +=-mmacosx-version-min=10.5
endif
ifeq ($(MACH_TYPE),ppc64)
CFLAGS  +=-mmacosx-version-min=10.5
LDFLAGS +=-mmacosx-version-min=10.5
endif
endif
ifeq ($(TARGET_OS),unix)
LDLIBS  += -lm
endif

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
OBJ_COMMON= qsnprint.o \
	strlcat.o \
	strlcpy.o \
	cmdlib.o \
	q_endian.o \
	byteordr.o \
	util_io.o \
	pathutil.o \
	mathlib.o \
	bspfile.o

OBJ_QBSP= brush.o \
	csg4.o \
	map.o \
	merge.o \
	nodraw.o \
	outside.o \
	portals.o \
	qbsp.o \
	region.o \
	solidbsp.o \
	surfaces.o \
	tjunc.o \
	writebsp.o

$(QBSP) : $(OBJ_COMMON) $(OBJ_QBSP)
	$(LINKER) $(OBJ_COMMON) $(OBJ_QBSP) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(QBSP)

