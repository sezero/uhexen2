# GNU Makefile for hexenworld server using GCC.
# $Id$
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# To cross-compile for Win64 on Unix, you must pass the W64BUILD=1
# argument to make. Also see build_cross_win64.sh for details.
#
# Build Options:
#
# OPT_EXTRA	yes  =  Some extra optimization flags will be added (default)
#		no   =	No extra optimizations will be made
#
# USE_WINSOCK2	yes  =	Use WinSock2 and link to ws2_32 instead of wsock32
# (for Win32)	no   =	(default) Use WinSock1.1 for compatibility with old
#			Windows 95 machines.
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
UHEXEN2_TOP:=../../..
# common sources path:
COMMONDIR:=../../h2shared
COMMON_HW:=../shared

# General options (see explanations at the top)
OPT_EXTRA=yes
COMPILE_32BITS=no
USE_WINSOCK2=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

ifeq ($(TARGET_OS),win64)
# use winsock2 for win64
USE_WINSOCK2=yes
endif

# Names of the binaries
BINARY:=hwsv$(exe_ext)

#############################################################
# Compiler flags
#############################################################

CPUFLAGS:=
# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPUFLAGS:=-march=i586
endif

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG
# Debug type flags

CFLAGS := -g -Wall

else
# Release type flags

CFLAGS := $(CPUFLAGS) -O2 -Wall -DNDEBUG -ffast-math -fexpensive-optimizations

ifeq ($(OPT_EXTRA),yes)
ifeq ($(MACH_TYPE),x86)
ALIGN_OPT:= $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,)
ifeq ($(ALIGN_OPT),)
ALIGN_OPT:= $(call check_gcc,-malign-loops=2 -malign-jumps=2 -malign-functions=2,)
endif
CFLAGS := $(CFLAGS) $(ALIGN_OPT)
endif

ifeq ($(MACH_TYPE),x86_64)
ALIGN_OPT:= $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,)
ifeq ($(ALIGN_OPT),)
ALIGN_OPT:= $(call check_gcc,-malign-loops=2 -malign-jumps=2 -malign-functions=2,)
endif
CFLAGS := $(CFLAGS) $(ALIGN_OPT)
endif

CFLAGS := $(CFLAGS) -fomit-frame-pointer
endif
#
endif

ifeq ($(COMPILE_32BITS),yes)
CFLAGS := $(CFLAGS) -m32
endif

CFLAGS := $(CFLAGS) $(ARCHFLAGS)
# end of compiler flags
#############################################################


#############################################################
# Other build flags
#############################################################
EXT_FLAGS:= -DH2W -DSERVERONLY
INCLUDES:= -I. -I$(COMMON_HW) -I$(COMMONDIR)

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates some extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG=1 -DDEBUG_BUILD=1
endif


#############################################################
# Win32 flags/settings
#############################################################
ifeq ($(TARGET_OS),win32)

CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

INCLUDES:= -I$(W32STUFF) $(INCLUDES)
LDFLAGS := $(LIBWINSOCK) -lwinmm -mconsole

endif
# End of Win32 settings
#############################################################


#############################################################
# Win64 flags/settings
#############################################################
ifeq ($(TARGET_OS),win64)

CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

INCLUDES:= -I$(W32STUFF) $(INCLUDES)
LDFLAGS := $(LIBWINSOCK) -lwinmm -mconsole

endif
# End of Win64 settings
#############################################################


#############################################################
# Unix flags/settings
#############################################################
ifeq ($(TARGET_OS),unix)
LDFLAGS := $(LIBSOCKET) -lm

endif
# End of Unix settings
#############################################################


#############################################################
# any other settings/flags
#############################################################
ifeq ($(COMPILE_32BITS),yes)
LDFLAGS := $(LDFLAGS) -m32
endif

# End of other settings/flags
#############################################################


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMON_HW)/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) -o $@ $<

# Objects

# Platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_SYS = sys_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_SYS = sys_win.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SYS = sys_unix.o
endif

# Final list of objects
HWSV_OBJS = \
	q_endian.o \
	link_ops.o \
	sizebuf.o \
	strlcat.o \
	strlcpy.o \
	msg_io.o \
	common.o \
	quakefs.o \
	info_str.o \
	cmd.o \
	crc.o \
	cvar.o \
	pr_strng.o \
	mathlib.o \
	zone.o \
	huffman.o \
	net_wins.o \
	net_chan.o \
	pmove.o \
	pmovetst.o \
	model.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	sv_effect.o \
	sv_ccmds.o \
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

$(BINARY): $(HWSV_OBJS)
	$(LINKER) -o $(BINARY) $(HWSV_OBJS) $(LDFLAGS)

clean:
	rm -f *.o cl_objs/*.o core

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)

