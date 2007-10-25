# GNU Makefile using GCC in a unix shell.
# $Header: /home/ozzie/Download/0000/uhexen2/scripts/makefile.inc,v 1.19 2007-10-25 09:06:52 sezero Exp $

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

# DJGPP directory:
DJGPPDIR ?=/djgpp

# OS and TARGET_OS detection:

HOST_OS :=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

# NOTE:	We only support these types of cross compilation:
#	Compiling for windows or dos (djgpp) on unix.
#	This means, unless cross compiling, the actual
#	TARGET_OS remains as the HOST_OS.

# NOTE:	DOS target is only tested/supported by cross compilation.

CROSS_COMPILE:=n

# See if we are running on a Windows installation.
#WIN_NATIVE :=$(shell if env | grep -i windir > /dev/null; then echo "yes"; fi)
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
# we are running on windows. set TARGET_OS to win32
# and set proper nasm binary name
# NOTE: re-visit here when we have MINGW64
ifeq (,$(findstring mingw32,$(HOST_OS)))
$(error MSYS is required for this build system on Windows)
endif
HOST_OS :=windows
TARGET_OS:=win32
NASM_BIN :=nasmw
endif

ifndef WIN_NATIVE
# we are running on a Unixish OS
ifdef WINBUILD
# cross compilation for windows
CROSS_COMPILE:=y
TARGET_OS:=win32
else
ifdef DOSBUILD
# cross compilation for dos
CROSS_COMPILE:=y
TARGET_OS:=dos
else
TARGET_OS:=unix
endif
endif
NASM_BIN :=nasm
endif

ifeq ($(TARGET_OS),unix)
MACH_TYPE:=$(shell sh $(UHEXEN2_TOP)/scripts/detect.sh arch)
else
# windows or dos. FIXME: windows does not have to be x86.
MACH_TYPE:=x86
endif

# Compiler selection:
CC ?= gcc
ifeq ($(CC),)
CC := gcc
endif

NASM ?= $(NASM_BIN)
WINDRES ?= windres

LINKER := $(CC)

# Helper function to check if gcc supports a given flag.
# this relies on gcc option of taking input from stdin
check_gcc = $(shell if echo | $(CC) $(1) -S -o /dev/null -xc - > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi;)


# Determine common build flags:

# Possible ARCH flags
ARCHFLAGS:=

ifeq ($(CROSS_COMPILE),n)
ifeq ($(HOST_OS),morphos)
ARCHFLAGS:=-noixemul
endif
endif

# NASM flags:
# These are required for IA32 assembly for hexen2/hexenworld clients
ifeq ($(TARGET_OS),win32)
NASMFLAGS:= -fwin32 --prefix _
endif
ifeq ($(TARGET_OS),dos)
NASMFLAGS:= -f coff --prefix _
endif
ifeq ($(TARGET_OS),unix)
ifeq ($(HOST_OS),darwin)
NASMFLAGS:= -f macho
else
NASMFLAGS:= -f elf
endif
endif

# the "sdl-config" command
SDL_CONFIG ?=sdl-config

# SDL flags:
# If a makefile needs SDL, it should set CHECK_SDL to 1
ifeq ($(CHECK_SDL),1)
ifeq ($(TARGET_OS),unix)
SDLFLAGS:= $(shell $(SDL_CONFIG) --cflags 2> /dev/null)
SDLLINK := $(shell $(SDL_CONFIG) --libs 2> /dev/null)
else
SDLFLAGS:=
SDLLINK :=
endif
endif

# -lsocket is needed on some arches
LIBSOCKET:=
ifeq ($(CROSS_COMPILE),n)
ifeq ($(HOST_OS),qnx)
LIBSOCKET:=-lsocket
endif
ifeq ($(HOST_OS),sunos)
LIBSOCKET:=-lsocket -lnsl
endif
endif

# MGL flags:
# These are required for hexen2/hexenworld win32-software
# renderer.
ifeq ($(TARGET_OS),win32)
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
ifeq ($(TARGET_OS),win32)
GL_LINK := -lopengl32
else
GL_LINK := -L$(X11BASE)/lib -lGL
endif

