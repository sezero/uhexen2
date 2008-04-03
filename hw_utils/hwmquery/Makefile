# GNU Makefile for hwmquery using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hw_utils/hwmquery/Makefile,v 1.14 2008-04-03 09:25:54 sezero Exp $
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

# Names of the binaries
ifeq ($(TARGET_OS),win32)
HWMQUERY:=hwmquery.exe
endif
ifeq ($(TARGET_OS),unix)
HWMQUERY:=hwmquery
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
endif

CFLAGS := $(CFLAGS) $(ARCHFLAGS)


# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES := -I$(W32STUFF) -I.
LDFLAGS  := -lwsock32 -mconsole
endif
ifeq ($(TARGET_OS),unix)
INCLUDES := -I.
LDFLAGS  := $(LIBSOCKET)
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
HWMQUERY_OBJS = qsnprint.o hwmquery.o

# Targets
all: $(HWMQUERY)
default: all

$(HWMQUERY) : $(HWMQUERY_OBJS)
	$(LINKER) -o $(HWMQUERY) $(HWMQUERY_OBJS) $(LDFLAGS)

clean:
	rm -f *.o core

