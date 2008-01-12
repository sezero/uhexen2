# GNU Makefile for hexenworld master server using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/Makefile,v 1.23 2008-01-12 08:15:46 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY:=hwmaster.exe
endif
ifeq ($(TARGET_OS),unix)
BINARY:=hwmaster
endif

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

ifdef NO_UNIT_AT_A_TIME
CFLAGS := $(CFLAGS) $(call check_gcc,-fno-unit-at-a-time,)
endif
endif

# Other build flags
EXT_FLAGS:=$(ARCHFLAGS)

ifeq ($(TARGET_OS),win32)
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := -lwsock32 -lwinmm -mconsole
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

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(MASTEROBJS) $(LDFLAGS)

clean:
	rm -f *.o core

