# GNU Makefile for hexen2 mapping tools using GCC.
# $Id$
#
# To cross-compile for Win32 on Unix: either pass the W32BUILD=1
# argument to make, or export it.  Also see build_cross_win32.sh.
# Requires: a mingw or mingw-w64 compiler toolchain.
#
# To cross-compile for Win64 on Unix: either pass the W64BUILD=1
# argument to make, or export it. Also see build_cross_win64.sh.
# Requires: a mingw-w64 compiler toolchain.
#
# To (cross-)compile for DOS: either pass the DOSBUILD=1 argument
# to make, or export it. Also see build_cross_dos.sh. Requires: a
# djgpp compiler toolchain.
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs
# local directory for os-specific headers and libraries
OSLIBS:=$(UHEXEN2_TOP)/oslibs
# where the common sources/objects are
COMMONDIR:=../common

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
VIS:=vis$(exe_ext)

# Compiler flags

# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
CPUFLAGS=$(CPU_X86)

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1 -ffast-math
endif
CFLAGS += -DDOUBLEVEC_T

LDFLAGS =
LDLIBS  =
INCLUDES= -I. -I$(COMMONDIR) -I$(LIBS_DIR)/common

# Other build flags

ifeq ($(TARGET_OS),dos)
INCLUDES+= -I$(OSLIBS)/dos
LDFLAGS += -lc -lgcc -lm
endif
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
ifeq ($(HOST_OS),morphos)
CFLAGS  += -noixemul
LDFLAGS += -noixemul
endif
ifeq ($(TARGET_OS),unix)
LDLIBS  += -lm
# threads: use sprocsp code for IRIX, pthreads for others.
ifeq (,$(findstring irix,$(HOST_OS)))
USE_PTHREADS=yes
endif
ifeq ($(USE_PTHREADS),yes)
CFLAGS  += -DUSE_PTHREADS
CFLAGS  += $(shell sh $(UHEXEN2_TOP)/scripts/pthread.sh $(UHEXEN2_TOP)/scripts/config.guess --cflags)
LDLIBS  += $(shell sh $(UHEXEN2_TOP)/scripts/pthread.sh $(UHEXEN2_TOP)/scripts/config.guess --libs)
endif
endif

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
OBJ_COMMON= qsnprint.o \
	strlcat.o \
	strlcpy.o \
	cmdlib.o \
	q_endian.o \
	byteordr.o \
	util_io.o \
	pathutil.o \
	mathlib.o \
	bspfile.o

OBJ_VIS= threads.o \
	flow.o \
	soundpvs.o \
	vis.o

$(VIS) : $(OBJ_COMMON) $(OBJ_VIS)
	$(LINKER) $(OBJ_COMMON) $(OBJ_VIS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(VIS)

