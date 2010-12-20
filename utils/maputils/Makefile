# GNU Makefile for hexen2 mapping tools using GCC.
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

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs
# where the common sources/objects are
COMMONDIR:=../common
# where to put the binaries
DESTDIR  :=../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
QBSP:=qbsp$(exe_ext)
LIGHT:=light$(exe_ext)
VIS:=vis$(exe_ext)
BSPINFO:=bspinfo$(exe_ext)

# Compiler flags

# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
CPUFLAGS=$(CPU_X86)

# Overrides for the default ARCHFLAGS
#ARCHFLAGS=

ifdef DEBUG
CFLAGS += -g -Wall
else
CFLAGS += $(CPUFLAGS) -O2 -Wall -DNDEBUG -ffast-math -fexpensive-optimizations
endif

CFLAGS += $(ARCHFLAGS)
CFLAGS += -DDOUBLEVEC_T

LDFLAGS =
LDLIBS  =
INCLUDES= -I. -I$(COMMONDIR)

# Other build flags

ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(W32STUFF)
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(W32STUFF)
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),unix)
LDLIBS  += -lm
endif

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJS= cmdlib.o \
	strlcat.o \
	strlcpy.o \
	q_endian.o \
	byteordr.o \
	util_io.o \
	pathutil.o \
	mathlib.o \
	bspfile.o

LIGHTOBJS= threads.o \
	entities.o \
	light.o \
	ltface.o \
	trace.o

VISOBJS= flow.o \
	soundpvs.o \
	vis.o

QBSPOBJS= brush.o \
	csg4.o \
	map.o \
	merge.o \
	nodraw.o \
	outside.o \
	portals.o \
	qbsp.o \
	region.o \
	solidbsp.o \
	surfaces.o \
	tjunc.o \
	writebsp.o

BSPINFOBJS= cmdlib.o \
	strlcat.o \
	strlcpy.o \
	q_endian.o \
	byteordr.o \
	util_io.o \
	pathutil.o \
	bspfile.o \
	bspinfo.o

$(LIGHT) : $(COMMONOBJS) $(LIGHTOBJS)
	$(LINKER) -o $(DESTDIR)/$(LIGHT) $(COMMONOBJS) $(LIGHTOBJS) $(LDFLAGS) $(LDLIBS)

$(VIS) : $(COMMONOBJS) $(VISOBJS)
	$(LINKER) -o $(DESTDIR)/$(VIS) $(COMMONOBJS) $(VISOBJS) $(LDFLAGS) $(LDLIBS)

$(QBSP) : $(COMMONOBJS) $(QBSPOBJS)
	$(LINKER) -o $(DESTDIR)/$(QBSP) $(COMMONOBJS) $(QBSPOBJS) $(LDFLAGS) $(LDLIBS)

$(BSPINFO) : $(BSPINFOBJS)
	$(LINKER) -o $(DESTDIR)/$(BSPINFO) $(BSPINFOBJS) $(LDFLAGS)

clean : 
	rm -f *.o core

