# GNU Makefile for hexen2 mapping tools using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/maputils/Makefile,v 1.18 2008-01-10 17:21:12 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..
# where the common sources/objects are
COMMONDIR:=../common
# where to put the binaries
DESTDIR  :=../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
QBSP:=qbsp.exe
LIGHT:=light.exe
VIS:=vis.exe
BSPINFO:=bspinfo.exe
endif
ifeq ($(TARGET_OS),unix)
QBSP:=qbsp
LIGHT:=light
VIS:=vis
BSPINFO:=bspinfo
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
CFLAGS := $(CPUFLAGS) -O2 -Wall -ffast-math -fexpensive-optimizations
endif
CFLAGS := $(CFLAGS) -DDOUBLEVEC_T

# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(W32STUFF) -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),unix)
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  := -lm
endif

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<
# special rule for mathlib because of DOUBLEVEC_T
mathlib.o: $(COMMONDIR)/mathlib.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<
bspfile.o: $(COMMONDIR)/bspfile.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	mathlib.o \
	bspfile.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

LIGHTOBJS= entities.o \
	light.o \
	ltface.o \
	threads.o \
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

BSPINFOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	bspfile.o \
	bspinfo.o

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(LIGHT) : $(COMMONOBJS) $(LIGHTOBJS)
	$(LINKER) -o $(DESTDIR)/$(LIGHT) $(COMMONOBJS) $(LIGHTOBJS) $(LDFLAGS) $(LDLIBS)

$(VIS) : $(COMMONOBJS) $(VISOBJS)
	$(LINKER) -o $(DESTDIR)/$(VIS) $(COMMONOBJS) $(VISOBJS) $(LDFLAGS) $(LDLIBS)

$(QBSP) : $(COMMONOBJS) $(QBSPOBJS)
	$(LINKER) -o $(DESTDIR)/$(QBSP) $(COMMONOBJS) $(QBSPOBJS) $(LDFLAGS) $(LDLIBS)

$(BSPINFO) : $(BSPINFOBJS)
	$(LINKER) -o $(DESTDIR)/$(BSPINFO) $(BSPINFOBJS) $(LDFLAGS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o core

