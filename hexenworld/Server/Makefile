# GNU Makefile for hexenworld server using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Server/Makefile,v 1.46 2008-04-02 17:55:23 sezero Exp $
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# Build Options:
#
# OPT_EXTRA	yes  =  Some extra optimization flags will be added (default)
#		no   =	No extra optimizations will be made
#
# COMPILE_32BITS yes =  Compile as a 32 bit binary. If you are on a 64 bit
#			platform and having problems with 64 bit compiled
#			binaries, set this option to yes. Default: no .
#			If you set this to yes, you need to have the 32 bit
#			versions of the libraries that you link against.
#		 no  =	Compile for the native word size of your platform,
#			which is the default option.
#
# The default compiler is gcc
# To build with a different compiler:	make CC=compiler_name [other stuff]
#
# To build for the demo version:	make DEMO=1 [other stuff]
#
# if building a debug version :		make DEBUG=1 [other stuff]
#

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..

# General options (see explanations at the top)
OPT_EXTRA=yes
COMPILE_32BITS=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY:=hwsv.exe
endif
ifeq ($(TARGET_OS),unix)
BINARY:=hwsv
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

ifdef NO_UNIT_AT_A_TIME
CFLAGS := $(CFLAGS) $(call check_gcc,-fno-unit-at-a-time,)
endif

ifeq ($(OPT_EXTRA),yes)
ifeq ($(MACH_TYPE),x86)
CFLAGS := $(CFLAGS) $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,-malign-loops=2 -malign-jumps=2 -malign-functions=2)
endif
ifeq ($(MACH_TYPE),x86_64)
CFLAGS := $(CFLAGS) $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,-malign-loops=2 -malign-jumps=2 -malign-functions=2)
endif
CFLAGS := $(CFLAGS) -fomit-frame-pointer
endif
endif

ifeq ($(COMPILE_32BITS),yes)
CFLAGS := $(CFLAGS) -m32
endif
# end of compiler flags

# Other build flags
EXT_FLAGS:= -DH2W -DSERVERONLY $(ARCHFLAGS)
INCLUDES:= -I. -I../Client

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(W32STUFF) $(INCLUDES)
LDFLAGS := -lwinmm -lwsock32 -mconsole
endif
ifeq ($(TARGET_OS),unix)
LDFLAGS := $(LIBSOCKET) -lm
endif

ifeq ($(COMPILE_32BITS),yes)
LDFLAGS := $(LDFLAGS) -m32
endif

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates come extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG_BUILD
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<
cl_objs/%.o: ../Client/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects
# Platform specific object settings
SYS_UNIX = sys_unix.o
SYS_WIN32 = sys_win.o

ifeq ($(TARGET_OS),win32)
SYSOBJ_SYS = $(SYS_WIN32)
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SYS = $(SYS_UNIX)
endif

# Final list of objects
COMMONOBJS = cl_objs/q_endian.o \
	cl_objs/link_ops.o \
	cl_objs/sizebuf.o \
	cl_objs/strlcat.o \
	cl_objs/strlcpy.o \
	cl_objs/msg_io.o \
	cl_objs/common.o \
	cl_objs/quakefs.o \
	cl_objs/info_str.o \
	cl_objs/cmd.o \
	cl_objs/crc.o \
	cl_objs/cvar.o \
	cl_objs/pr_strng.o \
	cl_objs/mathlib.o \
	cl_objs/huffman.o \
	cl_objs/net_wins.o \
	cl_objs/net_chan.o \
	cl_objs/pmove.o \
	cl_objs/pmovetst.o \
	cl_objs/zone.o \
	model.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	sv_ccmds.o \
	sv_effect.o \
	sv_ents.o \
	sv_init.o \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_send.o \
	sv_user.o \
	world.o \
	$(SYSOBJ_SYS)


# Targets
.PHONY: clean cleaner report

default: $(BINARY)
all: default

$(BINARY): report $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(COMMONOBJS) $(LDFLAGS)

clean:
	rm -f *.o cl_objs/*.o core

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)

