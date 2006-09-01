# if building a debug version :		make DEBUG=1 [other stuff]

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
ifeq "$(TARGET_OS)" "WIN32"
QBSP=qbsp.exe
LIGHT=light.exe
VIS=vis.exe
BSPINFO=bspinfo.exe
else
QBSP=qbsp
LIGHT=light
VIS=vis
BSPINFO=bspinfo
endif

# Compiler flags

# Overrides for the default CPUFLAGS
#CPUFLAGS:=
# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifndef DEBUG
CFLAGS := $(CPUFLAGS) -O2 -Wall -ffast-math -fexpensive-optimizations
endif

# Other build flags

ifeq "$(TARGET_OS)" "WIN32"
INCLUDES:= -I$(MINGWDIR)/include -idirafter $(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -idirafter $(COMMONDIR)
LDFLAGS :=
LDLIBS  := -lm
endif

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/strlcat.o \
	$(COMMONDIR)/strlcpy.o \
	mathlib.o \
	bspfile.o

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

BSPINFOBJS= bspinfo.o

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(LIGHT) : $(COMMONOBJS) $(LIGHTOBJS)
	$(LINKER) -o $(DESTDIR)/$(LIGHT) $(COMMONOBJS) $(LIGHTOBJS) $(LDFLAGS) $(LDLIBS)

$(VIS) : $(COMMONOBJS) $(VISOBJS)
	$(LINKER) -o $(DESTDIR)/$(VIS) $(COMMONOBJS) $(VISOBJS) $(LDFLAGS) $(LDLIBS)

$(QBSP) : $(COMMONOBJS) $(QBSPOBJS)
	$(LINKER) -o $(DESTDIR)/$(QBSP) $(COMMONOBJS) $(QBSPOBJS) $(LDFLAGS) $(LDLIBS)

$(BSPINFO) : $(COMMONOBJS) $(BSPINFOBJS)
	$(LINKER) -o $(DESTDIR)/$(BSPINFO) $(COMMONOBJS) $(BSPINFOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o .tmp *.tmp core

