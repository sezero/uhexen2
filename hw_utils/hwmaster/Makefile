# GNU Makefile for hexenworld master server using GCC.
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
# To (cross-)compile for DOS: either pass the DOSBUILD=1 argument
# to make, or export it. Also see build_cross_dos.sh. Requires: a
# djgpp compiler toolchain.
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# PATH SETTINGS:
# main uhexen2 relative path
UHEXEN2_TOP:=../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs
# local directory for os-specific headers and libraries
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
BINARY:=hwmaster$(exe_ext)

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
INCLUDES= -I. -I$(LIBS_DIR)/common

ifeq ($(USE_WINSOCK2),yes)
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

# Other build flags

ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
endif
INCLUDES+= -I$(OSLIBS)/windows
LDFLAGS += $(LIBWINSOCK) -lwinmm -mconsole
endif

ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
endif
INCLUDES+= -I$(OSLIBS)/windows
LDFLAGS += $(LIBWINSOCK) -lwinmm -mconsole
endif

ifeq ($(TARGET_OS),unix)
ifeq ($(HOST_OS),qnx)
LDFLAGS += -lsocket
endif
ifeq ($(HOST_OS),sunos)
LDFLAGS += -lsocket -lnsl -lresolv
endif
endif

ifeq ($(HOST_OS),morphos)
CPPFLAGS+= -noixemul
LDFLAGS += -noixemul
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

# Objects

# Platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_SYS = sys_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_SYS = sys_win.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SYS = sys_unix.o
endif
ifeq ($(TARGET_OS),aros)
SYSOBJ_SYS = sys_amiga.o
endif
ifeq ($(TARGET_OS),morphos)
SYSOBJ_SYS = sys_amiga.o
endif

OBJECTS = qsnprint.o sizebuf.o msg_io.o cmds.o common.o net.o master.o $(SYSOBJ_SYS)

# Targets
.PHONY: clean distclean

default: $(BINARY)
all: default

$(BINARY) : $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(BINARY)

