# GNU Makefile for hexen2 hcc tool (old version) using GCC.
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

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
BINARY:=hcc$(exe_ext)

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
CFLAGS += $(CPUFLAGS) -O2 -Wall -DNDEBUG
endif
# avoid the compiler warnings in pr_lex.c for now
CFLAGS += -Wno-char-subscripts

CFLAGS += $(ARCHFLAGS)

LDFLAGS =
LDLIBS  =
INCLUDES= -I. -I$(COMMONDIR) -I$(LIBS_DIR)/common

# Other build flags

ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),unix)
# nothing else needed
endif

# Targets
all : $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
HCCOBJS= cmdlib.o \
	strlcat.o \
	strlcpy.o \
	util_io.o \
	q_endian.o \
	byteordr.o \
	crchash.o \
	expr.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	pr_parse.o \
	stmt.o

$(BINARY) : $(HCCOBJS)
	$(LINKER) -o $(BINARY) $(HCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o core

