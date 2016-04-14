# GNU Makefile for Hexen II Dedicated Server (h2ded) using GCC.
# $Id$
#
# Remember to "make clean" between different types of builds or targets.
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
# To use a compiler other than gcc:	make CC=compiler_name [other stuff]
#
# To build for the demo version:	make DEMO=1 [other stuff]
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# PATH SETTINGS:
UHEXEN2_TOP:=../../..
ENGINE_TOP:=../..
COMMONDIR:=$(ENGINE_TOP)/h2shared
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# GENERAL OPTIONS (customize as required)

# compile only as a 32 bit binary even on a 64 bit platform?
COMPILE_32BITS=no

# use WinSock2 instead of WinSock-1.1? (disabled for w32 for compat.
# with old Win95 machines.) (enabled for Win64 in the win64 section.)
USE_WINSOCK2=no

# DOS Serial driver is useless in h2ded
USE_SERIAL=no
# use WatTCP (WATT-32) for DOS UDP networking?
USE_WATT32=yes
# use Beame & Whiteside TCP for DOS networking?
USE_BWTCP=yes
# use MPATH for DOS UDP networking under Win9x?
USE_MPATH=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
BINARY:=h2ded$(exe_ext)

#############################################################
# Compiler flags
#############################################################

ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
# Overrides for the default CPUFLAGS
CPUFLAGS=$(CPU_X86)

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
# optimization flags
CFLAGS += -O2 -DNDEBUG=1 -ffast-math -fomit-frame-pointer
endif

CPPFLAGS=
LDFLAGS =

# compiler includes
INCLUDES= -I. -I.. -I$(COMMONDIR) -I$(UHEXEN2_SHARED)

ifeq ($(COMPILE_32BITS),yes)
CFLAGS += -m32
LDFLAGS+= -m32
endif

# end of compiler flags
#############################################################


#############################################################
# Other build flags
#############################################################
CPPFLAGS+= -DSERVERONLY

ifdef DEMO
CPPFLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates some extra code in hexen2/hexenworld C source
CPPFLAGS+= -DDEBUG=1 -DDEBUG_BUILD=1
endif


#############################################################
# DOS flags/settings
#############################################################
ifeq ($(TARGET_OS),dos)

INCLUDES += -I$(OSLIBS)/dos

ifeq ($(USE_SERIAL),yes)
CPPFLAGS+= -DUSE_SERIAL
endif
ifeq ($(USE_BWTCP),yes)
CPPFLAGS+= -DUSE_BWTCP
endif
ifeq ($(USE_MPATH),yes)
CPPFLAGS+= -DUSE_MPATH
endif
ifeq ($(USE_WATT32),yes)
CPPFLAGS+= -DUSE_WATT32 -DWATT32_NO_OLDIES
INCLUDES+= -I$(OSLIBS)/dos/watt32/inc
LDFLAGS += -L$(OSLIBS)/dos/watt32/lib -lwatt
endif

LDFLAGS += -lc -lgcc
#LDFLAGS += -lm

endif
# End of DOS settings
#############################################################


#############################################################
# Win32 flags/settings
#############################################################
ifeq ($(TARGET_OS),win32)

CPPFLAGS+= -DWIN32_LEAN_AND_MEAN

ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += $(LIBWINSOCK) -lwinmm -mconsole

endif
# End of Win32 settings
#############################################################


#############################################################
# Win64 flags/settings
#############################################################
ifeq ($(TARGET_OS),win64)

CPPFLAGS+= -DWIN32_LEAN_AND_MEAN

# use winsock2 for win64
USE_WINSOCK2=yes

ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

INCLUDES+= -I$(OSLIBS)/windows/misc/include
LDFLAGS += $(LIBWINSOCK) -lwinmm -mconsole

endif
# End of Win64 settings
#############################################################


#############################################################
# Mac OS X flags/settings
#############################################################
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

endif
# End of Mac OS X settings
#############################################################


#############################################################
# Unix flags/settings
#############################################################
ifeq ($(TARGET_OS),unix)
# common unix:

ifeq ($(HOST_OS),qnx)
LDFLAGS += -lsocket
endif
ifeq ($(HOST_OS),sunos)
LDFLAGS += -lsocket -lnsl -lresolv
endif
LDFLAGS += -lm

endif
# End of Unix settings
#############################################################


#############################################################
# MorphOS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),morphos)

CFLAGS  += -noixemul
LDFLAGS += -noixemul
LDFLAGS += -lm

endif
# End of MorphOS settings
#############################################################


#############################################################
# AROS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),aros)


endif
# End of AROS settings
#############################################################


# AmigaOS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),amigaos)

CFLAGS  += -noixemul -m68020-60
LDFLAGS += -noixemul
LDFLAGS += -lm

# AmiTCP SDK
CFLAGS  += -I/usr/local/amiga/m68k-amigaos/netinclude

endif
# End of AmigaOS settings
#############################################################


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: ../%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
dos/%.o: ../dos/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<

# Objects

# Platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_NET := net_win.o net_wins.o net_wipx.o
SYSOBJ_SYS := sys_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_NET := net_win.o net_wins.o net_wipx.o
SYSOBJ_SYS := sys_win.o
endif
ifeq ($(TARGET_OS),dos)
DOSTCP :=
ifeq ($(USE_BWTCP),yes)
DOSTCP += dos/net_bw.o
endif
ifeq ($(USE_MPATH),yes)
DOSTCP += dos/net_mp.o dos/mplpc.o
endif
ifeq ($(USE_WATT32),yes)
DOSTCP += net_udp.o
else
# get inet_addr() and inet_ntoa() either from Watt-32
# or from our local implementation
DOSTCP += dos/dos_inet.o dos/inet_addr.o
endif
SYSOBJ_NET := dos/net_dos.o dos/net_ipx.o $(DOSTCP)
ifeq ($(USE_SERIAL),yes)
SYSOBJ_NET += dos/net_ser.o
endif
SYSOBJ_SYS := dos_v2.o sys_dos.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_unix.o
endif
ifeq ($(TARGET_OS),darwin)
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_unix.o
endif
ifeq ($(TARGET_OS),aros)
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_amiga.o
endif
ifeq ($(TARGET_OS),morphos)
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_amiga.o
endif
ifeq ($(TARGET_OS),amigaos)
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_amiga.o
endif

# Final list of objects
OBJECTS = \
	q_endian.o \
	link_ops.o \
	sizebuf.o \
	strlcat.o \
	strlcpy.o \
	qsnprint.o \
	msg_io.o \
	common.o \
	debuglog.o \
	quakefs.o \
	cmd.o \
	crc.o \
	cvar.o \
	mathlib.o \
	zone.o \
	$(SYSOBJ_NET) \
	net_dgrm.o \
	net_main.o \
	sv_model.o \
	host.o \
	host_cmd.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	host_string.o \
	sv_effect.o \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_user.o \
	world.o \
	$(SYSOBJ_SYS)


# Targets
.PHONY: clean distclean report

default: $(BINARY)
all: default

$(BINARY): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LDFLAGS) -o $@

# deps for *.c including another *.c
dos/mplpc.o: ../dos/mplib.c
dos/net_ser.o: ../dos/net_comx.c

clean:
	rm -f *.o *.res dos/*.o core
distclean: clean
	rm -f $(BINARY)

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)

