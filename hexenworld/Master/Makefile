# if building a debug version :		make DEBUG=1 [other stuff]

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
ifeq "$(TARGET_OS)" "WIN32"
BINARY=hwmaster.exe
else
BINARY=hwmaster
BINARY_STATIC=hwmaster.static
endif

# Compiler flags
ifndef DEBUG
CFLAGS := -O -march=i386 -Wall
endif

# Other build flags
EXT_FLAGS:=

ifeq "$(TARGET_OS)" "WIN32"
INCLUDES := -I$(MINGWDIR)/include
LDFLAGS  := -L$(MINGWDIR)/lib -lwsock32 -lwinmm -mconsole
else
INCLUDES :=
LDFLAGS  :=
EXT_FLAGS+= -DPLATFORM_UNIX
LDSTATIC := -static
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects
MASTEROBJS = cmds.o sys_main.o net.o

# Targets
default: $(BINARY)
all: default

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(MASTEROBJS) $(LDFLAGS)

# static builds are not tested on win32
ifneq "$(TARGET_OS)" "WIN32"

static: $(BINARY_STATIC)

$(BINARY_STATIC) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY_STATIC) $(MASTEROBJS) $(LDSTATIC)

# End of static binary targets
endif

clean:
	rm -f *.o core .tmp *.tmp

