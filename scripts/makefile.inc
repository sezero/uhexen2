# GNU Makefile using GCC in a unix shell.
# $Id$

# This file is responsible for determining the Host OS and Target OS,
# setting common paths, selecting the compiler, assigning the compiler
# command as the linker, defining helper functions for compiler flags
# detection, setting the "sdl-config" command.

# We also define some common flags in here:
# Possible ARCH flags, flags for NASM and SDL, flags for opengl linkage.
# See them near the bottom of this file.

# Unless overriden, these settings will be used in any includer makefile
# in the uhexen2 tree. The intention is to make the actual Makefiles some
# more readable and uniform.

# Common unix shell tools, such as uname, which, sed and tr, are required
# for successful operation. If you are running on windows, you must run
# under MSYS, otherwise this makefile system will fail.


################################
# Helper function to check availability of a given command

check_cmd=$(shell if which $(1) > /dev/null 2>&1; then echo "";else echo $(1);fi)


################################
# Common paths:

# X directory
X11BASE ?=/usr/X11R6

# local directory for os-specific headers and libraries:
OSLIBS :=$(UHEXEN2_TOP)/oslibs


################################
# OS and TARGET_OS detection:

CROSS_COMPILE:=n

HOST_OS0:=$(shell uname)
HOST_OS :=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

# NOTE:	We only support these types of cross compilation:
#	Compiling for windows or dos (djgpp) on unix.
#	This means, unless cross compiling, the actual
#	TARGET_OS remains as the HOST_OS.

# NOTE:	DOS target is only tested by cross compilation on
#	linux, so the following thing is not supported.
#ifneq (,$(findstring DOS,$(HOST_OS0)))
#HOST_OS :=dos
#TARGET_OS:=dos
#endif

ifneq (,$(findstring mingw32,$(HOST_OS)))
HOST_OS :=windows
endif

ifeq ($(HOST_OS),windows)
# running on Windows

ifdef DOSBUILD
# compiling for dos
CROSS_COMPILE:=y
TARGET_OS:=dos
else
# compiling for windows
ifdef W64BUILD
CROSS_COMPILE:=y
TARGET_OS:=win64
else
TARGET_OS:=win32
endif
endif

endif  # Windows


ifneq ($(HOST_OS),windows)
ifneq ($(HOST_OS),dos)

# running on a Unixish OS

ifdef W32BUILD
# cross compilation for windows
CROSS_COMPILE:=y
TARGET_OS:=win32
endif
ifdef W64BUILD
# cross compilation for windows
CROSS_COMPILE:=y
TARGET_OS:=win64
endif
ifdef DOSBUILD
# cross compilation for dos
CROSS_COMPILE:=y
TARGET_OS:=dos
endif
ifeq ($(CROSS_COMPILE),n)
# compiling for the host os.
TARGET_OS:=unix
endif

endif
endif


ifeq ($(TARGET_OS),unix)
MACH_TYPE:=$(shell sh $(UHEXEN2_TOP)/scripts/detect.sh arch)
endif
ifeq ($(TARGET_OS),dos)
MACH_TYPE:=x86
endif
ifeq ($(TARGET_OS),win32)
# FIXME: win32 does not have to be x86.
MACH_TYPE:=x86
endif
ifeq ($(TARGET_OS),win64)
# FIXME: uname from msys might not work here
#MACH_TYPE:=ia64
MACH_TYPE:=x86_64
endif


################################
# Binary filename extension:

exe_ext:=
ifeq ($(TARGET_OS),dos)
exe_ext:=.exe
endif
ifeq ($(TARGET_OS),win32)
exe_ext:=.exe
endif
ifeq ($(TARGET_OS),win64)
exe_ext:=.exe
endif


################################
# Compiler selection:

CC ?= gcc
ifeq ($(CC),)
CC := gcc
endif
AS ?= as

AR ?= ar
RANLIB ?= ranlib

NASM ?= nasm
WINDRES ?= windres

LINKER = $(CC)


################################
# Helper function to check if gcc supports a given flag.
# this relies on gcc option of taking input from stdin

check_gcc = $(shell if echo | $(CC) $(1) -S -o /dev/null -xc - > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi;)


################################
# Determine common build flags:

# Possible ARCH flags
ARCHFLAGS=

ifeq ($(CROSS_COMPILE),n)
ifeq ($(HOST_OS),morphos)
ARCHFLAGS=-noixemul
endif
endif


################################
# the "sdl-config" command

SDL_CONFIG ?=sdl-config

################################
# SDL flags:

ifeq ($(TARGET_OS),unix)
SDLFLAGS:= $(shell $(SDL_CONFIG) --cflags 2> /dev/null)
SDLLINK := $(shell $(SDL_CONFIG) --libs 2> /dev/null)
else
SDLFLAGS:=
SDLLINK :=
endif


################################
# -lsocket is needed on some arches

LIBSOCKET=

ifeq ($(CROSS_COMPILE),n)
ifeq ($(HOST_OS),qnx)
LIBSOCKET=-lsocket
endif
ifeq ($(HOST_OS),sunos)
LIBSOCKET=-lsocket -lnsl -lresolv
endif
endif

