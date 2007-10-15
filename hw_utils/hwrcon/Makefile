# GNU Makefile for hwrcon and hwterm using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hw_utils/hwrcon/Makefile,v 1.11 2007-10-15 12:15:29 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../..

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
HWRCON=hwrcon.exe
HWTERM=hwterm.exe
else
HWRCON=hwrcon
HWTERM=hwterm
endif

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

# Other build flags
EXT_FLAGS:=$(ARCHFLAGS)

ifeq ($(TARGET_OS),win32)
INCLUDES := -I$(MINGWDIR)/include -I.
LDFLAGS  := -L$(MINGWDIR)/lib -lwsock32 -mconsole
else
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

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(HWRCON) : $(COMMONOBJ) $(RCON_OBJS)
	$(LINKER) -o $(HWRCON) $(COMMONOBJ) $(RCON_OBJS) $(LDFLAGS)

$(HWTERM) : $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS)
	$(LINKER) -o $(HWTERM) $(COMMONOBJ) $(HUFF_OBJS) $(TERM_OBJS) $(LDFLAGS)

clean:
	rm -f *.o core .tmp *.tmp

