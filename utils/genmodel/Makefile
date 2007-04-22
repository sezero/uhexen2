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
ifeq ($(TARGET_OS),WIN32)
BINARY=genmodel.exe
else
BINARY=genmodel
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

ifeq ($(TARGET_OS),WIN32)
INCLUDES:= -I$(MINGWDIR)/include -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  := -lm
endif

# Targets
all : $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<
# special rule for mathlib because of DOUBLEVEC_T
mathlib.o: $(COMMONDIR)/mathlib.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
GENMODELOBJS= $(COMMONDIR)/cmdlib.o \
	$(COMMONDIR)/q_endian.o \
	$(COMMONDIR)/util_io.o \
	$(COMMONDIR)/pathutil.o \
	$(COMMONDIR)/qdir.o \
	mathlib.o \
	loadtri.o \
	scriplib.o \
	token.o \
	genmodel.o
#	$(COMMONDIR)/strlcat.o \
#	$(COMMONDIR)/strlcpy.o \

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(GENMODELOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(GENMODELOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o .tmp *.tmp core

