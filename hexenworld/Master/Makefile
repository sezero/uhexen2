# GNU Makefile for hexenworld master server using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/Makefile,v 1.30 2010-05-03 12:03:56 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# To cross-compile for Win64 on Unix, you must pass the W64BUILD=1
# argument to make. Also see build_cross_win64.sh for details.
#
# Build Options:
#
# USE_WINSOCK2	yes  =	Use WinSock2 and link to ws2_32 instead of wsock32
# (for Win32)	no   =	(default) Use WinSock1.1 for compatibility with old
#			Windows 95 machines.
#

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..

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

CPUFLAGS:=
# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPUFLAGS:=-march=i386
endif

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG

CFLAGS := -g -Wall

else

CFLAGS := $(CPUFLAGS) -O2 -Wall -DNDEBUG

ifdef NO_UNIT_AT_A_TIME
CFLAGS := $(CFLAGS) $(call check_gcc,-fno-unit-at-a-time,)
endif
endif

CFLAGS := $(CFLAGS) $(ARCHFLAGS)

ifeq ($(USE_WINSOCK2),yes)
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

# Other build flags
EXT_FLAGS:=

ifeq ($(TARGET_OS),win32)
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
endif
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := $(LIBWINSOCK) -lwinmm -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
endif
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := $(LIBWINSOCK) -lwinmm -mconsole
endif
ifeq ($(TARGET_OS),unix)
INCLUDES := -I.
LDFLAGS  := $(LIBSOCKET)
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects
MASTEROBJS = sizebuf.o msg_io.o cmds.o common.o net.o master.o sys_main.o

# Targets
default: $(BINARY)
all: default

$(BINARY) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(MASTEROBJS) $(LDFLAGS)

clean:
	rm -f *.o core

