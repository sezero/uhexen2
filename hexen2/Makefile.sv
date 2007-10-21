# GNU Makefile for Hexen II Dedicated Server (h2ded) using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hexen2/Makefile.sv,v 1.17 2007-10-21 16:48:09 sezero Exp $
#
# It is ESSENTIAL that you run make clean between different
# types of builds or different types of targets.
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
UHEXEN2_TOP=..

# General options (see explanations at the top)
OPT_EXTRA=yes
COMPILE_32BITS=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY=h2ded.exe
endif
ifeq ($(TARGET_OS),unix)
BINARY=h2ded
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
EXT_FLAGS:= -DSERVERONLY $(ARCHFLAGS)
INCLUDES:= -I./server -I.

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(MINGWDIR)/include $(INCLUDES)
LDFLAGS := -L$(MINGWDIR)/lib -lwinmm -lwsock32 -mconsole
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
sv_objs/%.o: server/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects
# Platform specific object settings
NET_UNIX = net_bsd.o \
	net_udp.o
NET_WIN32 = win_stuff/net_win.o \
	win_stuff/net_wins.o \
	win_stuff/net_wipx.o
SYS_UNIX = sv_objs/sys_unix.o
SYS_WIN32 = sv_objs/sys_win.o

ifeq ($(TARGET_OS),win32)
SYSOBJ_NET = $(NET_WIN32)
SYSOBJ_SYS = $(SYS_WIN32)
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_NET = $(NET_UNIX)
SYSOBJ_SYS = $(SYS_UNIX)
endif

# Final list of objects
H2DED_OBJS = q_endian.o \
	link_ops.o \
	sizebuf.o \
	strlcat.o \
	strlcpy.o \
	msg_io.o \
	common.o \
	debuglog.o \
	quakefs.o \
	cmd.o \
	crc.o \
	cvar.o \
	mathlib.o \
	zone.o \
	sv_objs/host.o \
	sv_objs/host_cmd.o \
	$(SYSOBJ_NET) \
	sv_objs/net_dgrm.o \
	sv_objs/net_main.o \
	sv_objs/model.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	pr_strng.o \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_effect.o \
	sv_user.o \
	world.o \
	$(SYSOBJ_SYS)


# Targets
default: $(BINARY)
all: default

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY): sanity $(H2DED_OBJS)
	$(LINKER) -o $(BINARY) $(H2DED_OBJS) $(LDFLAGS)

clean:
	rm -f *.o win_stuff/*.o sv_objs/*.o core

cleaner:
	rm -f *.o win_stuff/*.o sv_objs/*.o core $(BINARY)

