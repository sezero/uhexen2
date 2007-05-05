# if building a debug version :		make DEBUG=1 [other stuff]

# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../../..
# where the common sources/objects are
COMMONDIR=../../common
# where to put the binaries
DESTDIR =../../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),WIN32)
BINARY=bsp2wal.exe
else
BINARY=bsp2wal
endif

# Compiler flags

# Overrides for the default CPUFLAGS
#CPUFLAGS:=
# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifndef DEBUG
CFLAGS = $(CPUFLAGS) -O2 -Wall
endif

# Other build flags

ifeq ($(TARGET_OS),WIN32)
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
bspfile.o: $(COMMONDIR)/bspfile.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
BSP2WALOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	bspfile.o \
	bsp2wal.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(BSP2WALOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(BSP2WALOBJS) $(LDFLAGS) $(LDLIBS)


clean : 
	rm -f *.o $(COMMONDIR)/*.o .tmp *.tmp core

