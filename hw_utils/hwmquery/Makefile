# GNU Makefile for hwmquery using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hw_utils/hwmquery/Makefile,v 1.8 2007-10-15 12:15:29 sezero Exp $
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
HWMQUERY=hwmquery.exe
else
HWMQUERY=hwmquery
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
HWMQUERY_OBJS = qsnprint.o hwmquery.o

# Targets
all: $(HWMQUERY)
default: all

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(HWMQUERY) : $(HWMQUERY_OBJS)
	$(LINKER) -o $(HWMQUERY) $(HWMQUERY_OBJS) $(LDFLAGS)

clean:
	rm -f *.o core .tmp *.tmp

