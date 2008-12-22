# GNU Makefile for hwrcon and hwterm using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hw_utils/hwrcon/Makefile,v 1.18 2008-12-22 12:58:07 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
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
USE_WINSOCK2=yes
endif

# Names of the binaries
HWRCON:=hwrcon$(exe_ext)
HWTERM:=hwterm$(exe_ext)

# Whether to use Huffman encoding with the simple hwrcon tool
# Note: The hwterm tool already uses Huffman in order to read
# and correctly echo the server response.
USE_HUFFMAN ?=0

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
CFLAGS := $(CPUFLAGS) -O2 -Wall
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
ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
endif
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := $(LIBWINSOCK) -mconsole
endif
ifeq ($(TARGET_OS),win64)
EXT_FLAGS+= -D_USE_WINSOCK2
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := $(LIBWINSOCK) -mconsole
endif
ifeq ($(TARGET_OS),unix)
INCLUDES := -I.
LDFLAGS  := $(LIBSOCKET)
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJ = qsnprint.o
HUFF_OBJS = huffman.o
RCON_OBJS = hwrcon.o
TERM_OBJS = hwterm.o

ifeq ($(USE_HUFFMAN),yes)
RCON_OBJS:= $(HUFF_OBJS) $(RCON_OBJS)
EXT_FLAGS:= $(EXT_FLAGS) -DUSE_HUFFMAN
else
ifeq ($(USE_HUFFMAN),1)
RCON_OBJS:= $(HUFF_OBJS) $(RCON_OBJS)
EXT_FLAGS:= $(EXT_FLAGS) -DUSE_HUFFMAN
endif
endif

# Targets
all: $(HWRCON) $(HWTERM)
default: all

$(HWRCON) : $(COMMONOBJ) $(RCON_OBJS)
	$(LINKER) -o $(HWRCON) $(COMMONOBJ) $(RCON_OBJS) $(LDFLAGS)

$(HWTERM) : $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS)
	$(LINKER) -o $(HWTERM) $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS) $(LDFLAGS)

clean:
	rm -f *.o core

