# GNU Makefile for hexen2 lmp2pcx tool using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/texutils/lmp2pcx/Makefile,v 1.7 2007-10-25 09:06:56 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../../..
# where the common sources/objects are
COMMONDIR:=../../common
# where to put the binaries
DESTDIR  :=../../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY:=lmp2pcx.exe
endif
ifeq ($(TARGET_OS),unix)
BINARY:=lmp2pcx
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

# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(MINGWDIR)/include -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),unix)
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
LMP2PCXOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	$(COMMONDIR)/q_endian.o \
	lmp2pcx.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(LMP2PCXOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(LMP2PCXOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o core

