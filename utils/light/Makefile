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
# To cross-compile for MacOSX on Unix: either pass the OSXBUILD=1
# argument to make, or export it.  You would also need to pass a
# suitable MACH_TYPE=xxx (ppc, x86, x86_64, or ppc64) argument to
# make. Also see build_cross_osx.sh.
#
# To (cross-)compile for DOS: either pass the DOSBUILD=1 argument
# to make, or export it. Also see build_cross_dos.sh. Requires: a
# djgpp compiler toolchain.
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# Path settings:
UHEXEN2_TOP:=../..
UTILS_TOP:=..
COMMONDIR:=$(UTILS_TOP)/common
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
LIGHT:=light$(exe_ext)

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
INCLUDES= -I. -I$(COMMONDIR) -I$(UHEXEN2_SHARED)

# Other build flags

ifeq ($(TARGET_OS),dos)
INCLUDES+= -I$(OSLIBS)/dos
LDFLAGS += -lc -lgcc
#LDFLAGS += -lm
endif
ifeq ($(TARGET_OS),win32)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),win64)
CFLAGS  += -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += -mconsole
endif
ifeq ($(TARGET_OS),morphos)
CFLAGS  += -noixemul
LDFLAGS += -noixemul
LDLIBS  += -lm
endif
ifeq ($(TARGET_OS),darwin)
CPUFLAGS=
# require 10.5 for 64 bit builds
ifeq ($(MACH_TYPE),x86_64)
CFLAGS  +=-mmacosx-version-min=10.5
LDFLAGS +=-mmacosx-version-min=10.5
endif
ifeq ($(MACH_TYPE),ppc64)
CFLAGS  +=-mmacosx-version-min=10.5
LDFLAGS +=-mmacosx-version-min=10.5
endif
USE_PTHREADS=yes
ifeq ($(USE_PTHREADS),yes)
PTHREAD_CFLAGS= -D_THREAD_SAFE
PTHREAD_LIBS  = -pthread
CFLAGS  += $(PTHREAD_CFLAGS) -DUSE_PTHREADS
LDLIBS  += $(PTHREAD_LIBS)
endif
endif
ifeq ($(TARGET_OS),unix)
LDLIBS  += -lm
# threads: use sprocsp code for IRIX, pthreads for others.
ifeq (,$(findstring irix,$(HOST_OS)))
USE_PTHREADS=yes
endif
ifeq ($(USE_PTHREADS),yes)
TARGET_TRIPLET= $(shell sh $(UHEXEN2_TOP)/scripts/config.guess 2>/dev/null)
PTHREAD_CFLAGS= $(shell sh $(UHEXEN2_TOP)/scripts/pthread.sh $(TARGET_TRIPLET) --cflags 2>/dev/null)
PTHREAD_LIBS  = $(shell sh $(UHEXEN2_TOP)/scripts/pthread.sh $(TARGET_TRIPLET) --libs   2>/dev/null)
CFLAGS  += $(PTHREAD_CFLAGS) -DUSE_PTHREADS
LDLIBS  += $(PTHREAD_LIBS)
endif
endif

# Targets
all : $(LIGHT) $(VIS) $(QBSP) $(BSPINFO)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
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

OBJ_LIGHT= threads.o \
	entities.o \
	light.o \
	ltface.o \
	trace.o

$(LIGHT) : $(OBJ_COMMON) $(OBJ_LIGHT)
	$(LINKER) $(OBJ_COMMON) $(OBJ_LIGHT) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f *.o core
distclean: clean
	rm -f $(LIGHT)

