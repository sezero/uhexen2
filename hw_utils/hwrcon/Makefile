# GNU Makefile for hwrcon and hwterm using GCC.
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
LIBS_DIR:=$(UHEXEN2_TOP)/libs

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

# Whether to use Huffman encoding with the simple hwrcon tool
# Note: The hwterm tool already uses Huffman in order to read
# and correctly echo the server response.
USE_HUFFMAN ?=0

# Compiler flags

ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i386
endif
# Overrides for the default CPUFLAGS
CPUFLAGS=$(CPU_X86)

# Overrides for the default ARCHFLAGS
#ARCHFLAGS=

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS) $(ARCHFLAGS)
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
LDFLAGS += $(LIBWINSOCK) -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
endif
INCLUDES+= -I$(OSLIBS)/windows
LDFLAGS += $(LIBWINSOCK) -mconsole
endif
ifeq ($(TARGET_OS),unix)
LDFLAGS += $(LIBSOCKET)
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJ = qsnprint.o
HUFF_OBJS = huffman.o
RCON_OBJS = hwrcon.o
TERM_OBJS = hwterm.o

ifeq ($(USE_HUFFMAN),yes)
RCON_OBJS+= $(HUFF_OBJS)
CPPFLAGS+= -DUSE_HUFFMAN
else
ifeq ($(USE_HUFFMAN),1)
RCON_OBJS+= $(HUFF_OBJS)
CPPFLAGS+= -DUSE_HUFFMAN
endif
endif

# Targets
.PHONY: clean distclean

all: $(HWRCON) $(HWTERM)
default: all

$(HWRCON) : $(COMMONOBJ) $(RCON_OBJS)
	$(LINKER) -o $(HWRCON) $(COMMONOBJ) $(RCON_OBJS) $(LDFLAGS)

$(HWTERM) : $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS)
	$(LINKER) -o $(HWTERM) $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS) $(LDFLAGS)

clean:
	rm -f *.o core
clean: clean
	rm -f $(HWRCON) $(HWTERM)

