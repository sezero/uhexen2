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
DESTDIR =./bin

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
ifndef DEBUG
CFLAGS := -march=i586 -O2 -Wall
endif

# Other build flags

ifeq "$(TARGET_OS)" "WIN32"
INCLUDES:= -I$(COMMONDIR) -I$(MINGWDIR)/include
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  :=
# _GNU_SOURCE is required for stpcpy()
#CFLAGS  += -D_GNU_SOURCE
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
DHCCOBJS= $(COMMONDIR)/cmdlib.o \
	pr_comp.o \
	pr_lex.o \
	dcc.o \
	hcc.o

# Targets
all : $(BINARY)

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(DHCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMMONDIR)/*.o .tmp *.tmp core

