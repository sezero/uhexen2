# GNU Makefile using GCC in a unix shell.
# $Header: /home/ozzie/Download/0000/uhexen2/scripts/makefile.inc,v 1.13 2007-07-12 19:41:20 sezero Exp $

# This file is responsible for determining the Host OS and Target OS,
# setting common paths, selecting the compiler, assigning the compiler
# command as the linker, defining helper functions for compiler flags
# detection, setting the "sdl-config" command.

# We also define some common flags in here:
# Possible ARCH flags, flags for NASM, SDL and MGL, flags for opengl linkage.
# See them near the bottom of this file.

# Unless overriden, these settings will be used in any includer makefile
# in the uhexen2 tree. The intention is to make the actual Makefiles some
# more readable and uniform.


# Common paths:
# X directory
X11BASE ?=/usr/X11R6

# MinGW32 directory:
MINGWDIR ?=/mingw

# OS and TARGET_OS detection:

HOST_OS :=$(shell uname -s)

# NOTE:	We only support one type of cross compilation:
#	Compiling for windows on unix (linux).  This
#	means, unless cross compiling (detected by the
#	WINBUILD definition on a UNIX host, see below),
#	the actual TARGET_OS remains as the HOST_OS.

# See if we are running on a Windows installation.
#WIN_NATIVE :=$(shell if env | grep -i windir > __tmp.tmp; then echo "yes"; fi)
ifdef windir
WIN_NATIVE := yes
endif
ifdef WINDIR
WIN_NATIVE := yes
endif
ifdef WinDir
WIN_NATIVE := yes
endif

ifdef WIN_NATIVE
# we are running on windows. set TARGET_OS to WIN32
# and set proper nasm binary name
# NOTE: re-visit here when we have MINGW64
ifeq (,$(findstring MINGW32,$(HOST_OS)))
$(error MSys is required for this build system on Windows)
endif
HOST_OS :=Windows
TARGET_OS:=WIN32
NASM_BIN :=nasmw
else
# we are running on a Unixish OS
ifdef WINBUILD
# cross compilation for windows
TARGET_OS:=WIN32
else
TARGET_OS:=UNIX
endif
NASM_BIN :=nasm
endif

ifeq ($(TARGET_OS),WIN32)
# FIXME: windows does not have to be x86.
MACH_TYPE:=x86
else
MACH_TYPE:=$(shell sh $(UHEXEN2_TOP)/scripts/detect.sh arch)
endif

# Compiler selection:
# refuse cc for CC which is the default for make
# accept environment or command line overrides.
CC_DEFAULT:=gcc
ifeq ($(origin CC),environment)
CC_OVERRIDE:=1
# watch for stale environment defs
ifeq ($(CC),)
CC_OVERRIDE:=
endif
endif
ifeq ($(origin CC),command line)
CC_OVERRIDE:=1
endif
ifeq ($(CC_OVERRIDE),)
CC := $(CC_DEFAULT)
endif

NASM ?= $(NASM_BIN)
WINDRES ?= windres

LINKER := $(CC)

# Helper function to check if gcc supports a given flag.
ifdef WIN_NATIVE
# /dev/null stuff will not work on windows ;)
check_gcc = $(shell echo > .tmp; if $(CC) $(1) -S -o _tmp.tmp -xc .tmp > __tmp.tmp 2>&1; then echo "$(1)"; else echo "$(2)"; fi)
else
check_gcc = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)
endif


# Determine common build flags:

# Possible ARCH flags
ARCHFLAGS:=

ifndef WINBUILD
ifeq ($(HOST_OS),MorphOS)
ARCHFLAGS:=-noixemul
endif
endif

# NASM flags:
# These are required for IA32 assembly for hexen2/hexenworld clients
ifeq ($(TARGET_OS),WIN32)
NASMFLAGS:= -fwin32 -D__WIN32__ --prefix _
else
NASMFLAGS:= -f elf
endif

# the "sdl-config" command
CMD_SDLCFG ?=sdl-config

# SDL flags:
# If a makefile needs SDL, it should set CHECK_SDL to 1
ifeq ($(CHECK_SDL),1)
ifeq ($(TARGET_OS),UNIX)
SDLFLAGS:= $(shell $(CMD_SDLCFG) --cflags)
SDLLINK := $(shell $(CMD_SDLCFG) --libs)
else
SDLFLAGS:=
SDLLINK :=
endif
endif

# -lsocket is needed on some arches
LIBSOCKET:=
ifndef WINBUILD
ifeq ($(HOST_OS),QNX)
LIBSOCKET:=-lsocket
endif
ifeq ($(HOST_OS),SunOS)
LIBSOCKET:=-lsocket -lnsl
endif
endif

# MGL flags:
# These are required for hexen2/hexenworld win32-software
# renderer.
ifeq ($(TARGET_OS),WIN32)
MGL_FLAG:= -DMGL_DLL
MGL_INCL:= -I$(UHEXEN2_TOP)/w32stuff/scitech
MGL_LINK:= -L$(UHEXEN2_TOP)/w32stuff/scitech -lmglfx
else
MGL_FLAG:=
MGL_INCL:=
MGL_LINK:=
endif

# GL Linkage flags:
# These flags will be used for linking to opengl library
ifeq ($(TARGET_OS),WIN32)
GL_LINK := -lopengl32
else
GL_LINK := -L$(X11BASE)/lib -lGL
endif

