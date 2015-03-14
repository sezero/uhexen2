# GNU Makefile for hwrcon and hwterm using GCC.
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

# PATH SETTINGS:
UHEXEN2_TOP:=../..
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# use WinSock2 instead of WinSock-1.1? (disabled for w32 for compat.
# with old Win95 machines.) (enabled for Win64 below.)
USE_WINSOCK2=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

ifeq ($(TARGET_OS),win64)
# use winsock2 for win64
USE_WINSOCK2=yes
endif

# Names of the binaries
HWRCON:=hwrcon$(exe_ext)
HWTERM:=hwterm$(exe_ext)

# Compiler flags

ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i386
endif
# Overrides for the default CPUFLAGS
CPUFLAGS=$(CPU_X86)

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1
endif

CPPFLAGS=
LDFLAGS =

# compiler includes
INCLUDES= -I. -I$(UHEXEN2_SHARED)

ifeq ($(USE_WINSOCK2),yes)
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

# Other build flags

ifeq ($(TARGET_OS),win32)
CPPFLAGS+= -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
endif
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += $(LIBWINSOCK) -mconsole
endif

ifeq ($(TARGET_OS),win64)
CPPFLAGS+= -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
endif
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += $(LIBWINSOCK) -mconsole
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
ifeq ($(HOST_OS),qnx)
LDFLAGS += -lsocket
endif
ifeq ($(HOST_OS),sunos)
LDFLAGS += -lsocket -lnsl -lresolv
endif
endif

ifeq ($(TARGET_OS),morphos)
CFLAGS += -noixemul
LDFLAGS += -noixemul
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJ =qsnprint.o
HUFF_OBJS = huffman.o
RCON_OBJS = hwrcon.o
TERM_OBJS = hwterm.o

# Targets
.PHONY: clean distclean

all: $(HWRCON) $(HWTERM)
default: all

$(HWRCON) : $(COMMONOBJ) $(RCON_OBJS)
	$(LINKER) $(COMMONOBJ) $(RCON_OBJS) $(LDFLAGS) -o $@

$(HWTERM) : $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS)
	$(LINKER) $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS) $(LDFLAGS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(HWRCON) $(HWTERM)

