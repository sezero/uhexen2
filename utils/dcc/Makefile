# if building a debug version :		make DEBUG=1 [other stuff]

# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../..
# where to put the binaries
DESTDIR =../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq "$(TARGET_OS)" "WIN32"
BINARY=dhcc.exe
else
BINARY=dhcc
endif

# Compiler flags

# Overrides for the default CPUFLAGS
#CPUFLAGS:=
# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifndef DEBUG
OPTFLAGS:= -O2
#CFLAGS := $(CPUFLAGS) $(OPTFLAGS) -Wall
# gcc-4 builds with -O2 are broken. gcc-4.1 builds
# even with -O1 are also broken: no optimizations.
# besides, the i586 cpuflag is causing a ~two-fold~
# slowdown! not using it either...
CFLAGS := -Wall
endif

# Other build flags

ifeq "$(TARGET_OS)" "WIN32"
INCLUDES:= -I$(MINGWDIR)/include -idirafter .
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -idirafter .
LDFLAGS :=
LDLIBS  :=
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
DHCCOBJS= cmdlib.o \
	dcc.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o

# Targets
all : $(BINARY)

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(DHCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o .tmp *.tmp core

