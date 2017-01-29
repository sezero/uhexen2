# GNU Makefile using GCC in a unix shell.
# $Id$

# This file is responsible for determining the Host OS and Target OS,
# selecting the compiler, assigning the compiler command as the linker,
# defining helper functions for compiler flags detection.

# Common unix shell tools, such as uname, which, sed and tr, are required
# for successful operation. If you are running on windows, you must run
# under MSYS, otherwise this makefile system will fail.

################################
# Helper function to check availability of a given command
check_cmd=$(shell if which $(1) > /dev/null 2>&1; then echo "";else echo $(1);fi)

################################
# OS and TARGET_OS detection:

CROSS_COMPILE=n

HOST_OS0=$(shell uname)
HOST_OS =$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

# NOTE:	We only support these types of cross compilation:
#	Compile for windows, dos (djgpp), osx or morphos on unix.
#	This means, unless cross compiling, the actual
#	TARGET_OS remains as the HOST_OS.

# NOTE:	DOS target is only tested by cross compilation on
#	linux, so the following thing is not supported.
#ifneq (,$(findstring DOS,$(HOST_OS0)))
#HOST_OS  =dos
#TARGET_OS=dos
#endif

ifneq (,$(findstring OS/2,$(HOST_OS0)))
HOST_OS  =os2
TARGET_OS=os2
MACH_TYPE=x86
endif

ifneq (,$(findstring mingw32,$(HOST_OS)))
HOST_OS =windows
endif

ifeq ($(HOST_OS),windows)
# running on Windows

ifdef DOSBUILD
# compiling for dos
CROSS_COMPILE=y
TARGET_OS=dos
else
# compiling for windows
ifdef W64BUILD
CROSS_COMPILE=y
TARGET_OS=win64
else
TARGET_OS=win32
endif
endif
endif #

ifneq ($(HOST_OS),windows)
ifneq ($(HOST_OS),dos)
ifneq ($(HOST_OS),os2)
# assuming a unix host

ifdef W32BUILD
# cross compilation for windows
CROSS_COMPILE=y
TARGET_OS=win32
endif
ifdef W64BUILD
# cross compilation for windows
CROSS_COMPILE=y
TARGET_OS=win64
endif
ifdef DOSBUILD
# cross compilation for dos
CROSS_COMPILE=y
TARGET_OS=dos
endif
ifdef OSXBUILD
# cross compilation for mac os x
CROSS_COMPILE=y
TARGET_OS=darwin
endif
ifdef MOSBUILD
# cross compilation for morphos
CROSS_COMPILE=y
TARGET_OS=morphos
endif
ifdef AROSBUILD
# cross compilation for aros
CROSS_COMPILE=y
TARGET_OS=aros
# override MACH_TYPE on the make command-line, if necessary
MACH_TYPE=x86
endif
ifdef AMIGABUILD
# cross compilation for amigaos
CROSS_COMPILE=y
TARGET_OS=amigaos
endif
ifdef AMIGA4BUILD
# cross compilation for amigaos4
CROSS_COMPILE=y
TARGET_OS=amigaos4
endif
ifeq ($(CROSS_COMPILE),n)
# compiling for the host os.
ifneq (,$(findstring darwin,$(HOST_OS)))
# separate darwin and unix:
TARGET_OS=darwin
else
TARGET_OS=unix
endif
endif

endif
endif
endif #

ifeq ($(TARGET_OS),unix)
MACH_TYPE=$(shell sh $(UHEXEN2_TOP)/scripts/detect.sh arch)
endif
ifeq ($(TARGET_OS),darwin)
ifeq ($(CROSS_COMPILE),n)
MACH_TYPE=$(shell sh $(UHEXEN2_TOP)/scripts/detect.sh arch)
endif
endif
ifeq ($(TARGET_OS),morphos)
MACH_TYPE=ppc
endif
ifeq ($(TARGET_OS),amigaos)
MACH_TYPE=m68k
endif
ifeq ($(TARGET_OS),amigaos4)
MACH_TYPE=ppc
endif
ifeq ($(TARGET_OS),dos)
MACH_TYPE=x86
endif
ifeq ($(TARGET_OS),win32)
# FIXME: win32 does not have to be x86.
MACH_TYPE=x86
endif
ifeq ($(TARGET_OS),win64)
# FIXME: uname from msys might not work here
MACH_TYPE=x86_64
endif

ifneq (,$(findstring AROS,$(HOST_OS0)))
HOST_OS  =aros
TARGET_OS=aros
CROSS_COMPILE=n
endif

ifneq (,$(findstring morphos,$(HOST_OS)))
HOST_OS  =morphos
TARGET_OS=morphos
MACH_TYPE=ppc
CROSS_COMPILE=n
endif

ifneq (,$(findstring amigaos,$(HOST_OS)))
ifeq ($(shell uname -m),ppc)
HOST_OS =amigaos4
TARGET_OS=amigaos4
MACH_TYPE=ppc
else
ifeq ($(shell uname -m),powerpc)
HOST_OS =amigaos4
TARGET_OS=amigaos4
MACH_TYPE=ppc
else
HOST_OS =amigaos
TARGET_OS=amigaos
MACH_TYPE=m68k
endif
endif
CROSS_COMPILE=n
endif

################################
# Binary filename extension:

exe_ext=
ifeq ($(TARGET_OS),dos)
exe_ext=.exe
endif
ifeq ($(TARGET_OS),os2)
exe_ext=.exe
endif
ifeq ($(TARGET_OS),win32)
exe_ext=.exe
endif
ifeq ($(TARGET_OS),win64)
exe_ext=.exe
endif

################################
# Compiler selection:

CC ?= gcc
ifeq ($(origin CC),default)
# refuse the stupid 'cc' default of make
# which not necessarily exist everywhere
CC  = gcc
endif
AS ?= as
AR ?= ar
RANLIB ?= ranlib
NASM ?= nasm
WINDRES ?= windres
RC ?= $(WINDRES)
LIPO ?= lipo
LINKER = $(CC)

