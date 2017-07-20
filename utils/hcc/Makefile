# GNU Makefile for hexen2 hcc tool using GCC.
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
BINARY:=hcc$(exe_ext)

# Compiler flags

# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
CPUFLAGS=$(CPU_X86)

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1
endif
# avoid the compiler warnings in pr_lex.c for now
CFLAGS += -Wno-char-subscripts

LDFLAGS =
LDLIBS  =
INCLUDES= -I. -I$(COMMONDIR) -I$(UHEXEN2_SHARED)

# Other build flags

ifeq ($(TARGET_OS),dos)
INCLUDES+= -I$(OSLIBS)/dos
LDFLAGS += $(OSLIBS)/dos/djtime/djtime.a -lc
endif
ifeq ($(TARGET_OS),os2)
INCLUDES+= -I$(OSLIBS)/os2/emx/include
CFLAGS  += -Zmt
ifndef DEBUG
LDFLAGS += -s
endif
LDFLAGS += -Zmt
endif
ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
CFLAGS  += -m32
LDFLAGS += -m32 -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
CFLAGS  += -m64
LDFLAGS += -m64 -mconsole
endif
ifeq ($(TARGET_OS),aros)
# nothing extra is needed
endif
ifeq ($(TARGET_OS),morphos)
CFLAGS  += -noixemul
LDFLAGS += -noixemul
endif
ifeq ($(TARGET_OS),amigaos)
CFLAGS  += -noixemul -m68020-60 -m68881
LDFLAGS += -noixemul -m68020 -m68881
# -lm is needed for atof()
LDLIBS  += -lm
ifndef DEBUG
CFLAGS  += -fno-omit-frame-pointer
endif
# for extra missing headers
INCLUDES += -I$(OSLIBS)/amigaos/include
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
# nothing extra is needed
endif

# Targets
all : $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
OBJECTS= qsnprint.o \
	strlcat.o \
	strlcpy.o \
	cmdlib.o \
	util_io.o \
	q_endian.o \
	byteordr.o \
	crchash.o \
	expr.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	stmt.o

$(BINARY) : $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(BINARY)

