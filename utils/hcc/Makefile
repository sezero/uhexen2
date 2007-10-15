# GNU Makefile for hexen2 hcc tool using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/hcc/Makefile,v 1.11 2007-10-15 12:15:31 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../..
# where the common sources/objects are
COMMONDIR=../common
# where to put the binaries
DESTDIR =../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY=hcc.exe
else
BINARY=hcc
endif

# Compiler flags

CPUFLAGS:=
# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPUFLAGS:=-march=i586
endif

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG
CFLAGS := -g -Wall
else
CFLAGS = $(CPUFLAGS) -O2 -Wall
endif
# avoid the compiler warnings in pr_lex.c for now
CFLAGS+= -Wno-char-subscripts

# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(MINGWDIR)/include -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  :=
endif

# Targets
all : $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
HCCOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/crc.o \
	expr.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	pr_parse.o \
	stmt.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(HCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(HCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o .tmp *.tmp core

