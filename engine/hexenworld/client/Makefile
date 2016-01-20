# GNU Makefile for hexenworld clients using GCC.
# $Id$
#
# Remember to "make clean" between different types of builds or targets.
# Unix platforms require SDL !!!
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
# To see valid targets:			make help
#
# To use a compiler other than gcc:	make CC=compiler_name [other stuff]
#
# To use a different nasm-compatible assembler, such as yasm:
#					make NASM=yasm [other stuff]
#
# To specify X installation at somewhere other than /usr/X11R6
# such as /usr/X11R7 :			make X11BASE=/your/x11/path [other stuff]
#
# To build for the demo version:	make DEMO=1 [other stuff]
#
# To build a debug version:		make DEBUG=1 [other stuff]
#

# PATH SETTINGS:
UHEXEN2_TOP:=../../..
ENGINE_TOP:=../..
HW_TOP:=..
COMMONDIR:=$(ENGINE_TOP)/h2shared
COMMON_HW:=$(HW_TOP)/shared
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# GENERAL OPTIONS (customize as required)

# X directory
X11BASE    =/usr/X11R6
# the sdl-config command
SDL_CONFIG =sdl-config
SDL_CFLAGS = $(shell $(SDL_CONFIG) --cflags 2> /dev/null)
SDL_LIBS   = $(shell $(SDL_CONFIG) --libs 2> /dev/null)

# compile only as a 32 bit binary even on a 64 bit platform?
COMPILE_32BITS=no

# use fast x86 assembly on ia32 machines? (auto-disabled for
# any other cpu.)
USE_X86_ASM=yes

# link to the opengl libraries at compile time? (defaults
# to no, so the binaries will dynamically load the necessary
# libraries and functions at runtime.)
LINK_GL_LIBS=no

# enable evil 3dfx glide hacks for native hardware gamma for
# the old Voodoo Graphics and Voodoo2 cards?  (Linux/FreeBSD)
USE_3DFXGAMMA=no

# enable sound support?
USE_SOUND=yes
# ALSA audio support? (req: alsa-lib and alsa-kernel modules
# >= 1.0.1.  v0.9.8 and v1.0.0 might work, but not supported.
# If not Linux, ALSA will be automatically be disabled.)
USE_ALSA=yes
# OSS audio support? (for Unix. enabled on Linux and FreeBSD.
# automatically disabled on other platforms: see snd_sys.h)
USE_OSS=yes
# SUN audio support? (enabled on OpenBSD, NetBSD and SUN.
# automatically disabled on others: see snd_sys.h)
USE_SUNAUDIO=yes
# SDL audio support? (enabled on all unix-like platforms.)
USE_SDLAUDIO=yes

# include target's MIDI driver if available?
USE_MIDI=yes

# CDAudio support?
USE_CDAUDIO=yes
# use SDL cdaudio? (otherwise platform specific cdrom code will
# be used.  The only problem with SDL_cdrom is that it lacks
# proper volume controls.  See cd_unix.h for the availability of
# platform specific cdaudio drivers.
# NOTE: SDL dropped cdaudio support in version 1.3.0 and later!)
USE_SDLCD=no

# link to the directx libraries at compile time? (otherwise, link
# to the dynamically load the necessary dll files and functions at
# runtime which ensures our exe to run on ancient windows versions
# without a directx installation.)
LINK_DIRECTX=no

# enable startup splash screens? (windows)
WITH_SPLASHES=yes

# use SciTech MGL for Win32 software video driver? (Default to yes
# for x86, auto-disabled for Win64.  if disabled, the DIB/GDI-only
# compatibility software video driver will be used.)
USE_MGL=yes

# use WinSock2 instead of WinSock-1.1? (disabled for w32 for compat.
# with old Win95 machines.) (enabled for Win64 in the win64 section.)
USE_WINSOCK2=no

# Enable/disable codecs for streaming music support:
USE_CODEC_WAVE=yes
USE_CODEC_FLAC=no
USE_CODEC_MP3=yes
USE_CODEC_VORBIS=yes
USE_CODEC_OPUS=no
# either mikmod (preferred) or modplug (both possible
# but not needed nor meaningful.)
USE_CODEC_MIKMOD=no
USE_CODEC_MODPLUG=no
USE_CODEC_UMX=no
# either timidity (preferred) or wildmidi (both possible
# but not needed nor meaningful)
USE_CODEC_TIMIDITY=yes
USE_CODEC_WILDMIDI=no
# compile timidity with DLS instruments support? (no:
# the dls code isn't good enough and isn't used in unix
# where timidity is normally needed, either.)
TIMIDITY_USE_DLS=no
# which library to use for mp3 decoding: mad or mpg123
MP3LIB=mad
# which library to use for ogg decoding: vorbis or tremor
VORBISLIB=vorbis
# whether the codecs allocate on the zone instead of
# system memory (set to yes for DOS builds, for example)
CODECS_USE_ZONE=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
SW_BINARY:=hwcl$(exe_ext)
GL_BINARY:=glhwcl$(exe_ext)

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
INCLUDES= -I. -I$(COMMON_HW) -I$(COMMONDIR) -I$(UHEXEN2_SHARED)
# nasm includes: the trailing directory separator matters
NASM_INC= -I./ -I$(COMMON_HW)/ -I$(COMMONDIR)/
# windows resource compiler includes
RES_INC = -I. -I$(COMMON_HW) -I$(COMMONDIR)

ifeq ($(COMPILE_32BITS),yes)
CFLAGS += -m32
LDFLAGS+= -m32
endif

# end of compiler flags
#############################################################


#############################################################
# Other build flags
#############################################################
CPPFLAGS+= -DH2W

# disable x86 assembly if it is not an x86.
ifneq ($(MACH_TYPE),x86)
ifeq ($(MACH_TYPE),x86_64)
#  but keep the ia32 assembly on x86_64 if
#  compiling as 32 bits for the s/w renderer
ifneq ($(COMPILE_32BITS),yes)
USE_X86_ASM=no
endif
else
USE_X86_ASM=no
endif
endif

ifdef DEMO
CPPFLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates some extra code in hexen2/hexenworld C source
CPPFLAGS+= -DDEBUG=1 -DDEBUG_BUILD=1
endif


#############################################################
# OpenGL settings
#############################################################
GL_DEFS = -DGLQUAKE
GL_LIBS =

#############################################################
# streaming music initial setup
#############################################################
ifneq ($(USE_SOUND),yes)
USE_CODEC_WAVE=no
USE_CODEC_FLAC=no
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no
USE_CODEC_MIKMOD=no
USE_CODEC_MODPLUG=no
USE_CODEC_UMX=no
USE_CODEC_MP3=no
USE_CODEC_VORBIS=no
USE_CODEC_OPUS=no
endif
# sanity checking for decoder library options
ifneq ($(VORBISLIB),vorbis)
ifneq ($(VORBISLIB),tremor)
$(error Invalid VORBISLIB setting)
endif
endif
ifneq ($(MP3LIB),mpg123)
ifneq ($(MP3LIB),mad)
$(error Invalid MP3LIB setting)
endif
endif


#############################################################
# Mac OS X flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),darwin)

NASMFLAGS=-f macho
CPUFLAGS=
ifeq ($(MACH_TYPE),x86)
# x86 requires 10.4. gcc adds the 10.4 flag by itself.
endif
ifeq ($(MACH_TYPE),ppc)
# require 10.2 for ppc builds (midi_osx.c requirement.)
CFLAGS  +=-mmacosx-version-min=10.2
LDFLAGS +=-mmacosx-version-min=10.2
endif
ifeq ($(MACH_TYPE),ppc64)
# require 10.5 for ppc64 (actually SDL doesn't support ppc64.)
CFLAGS  +=-mmacosx-version-min=10.5
LDFLAGS +=-mmacosx-version-min=10.5
endif
ifeq ($(MACH_TYPE),x86_64)
# require 10.6 for amd64 builds (not 10.5: SDL's requirement.)
# bundle1.o is needed for dyld_stub_binding_helper
CFLAGS  +=-mmacosx-version-min=10.6
LDFLAGS +=-mmacosx-version-min=10.6 -Wl,-lbundle1.o
endif

ifeq ($(USE_SOUND),yes)
# enable the extra codecs
USE_CODEC_FLAC=yes
USE_CODEC_OPUS=yes
USE_CODEC_MIKMOD=yes
USE_CODEC_UMX=yes
endif
# no need for timidity, we have a midi driver
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no
ifeq ($(USE_MIDI),yes)
# note: midi_mac.c requires -Wl,-framework,QuickTime which we are not using anymore.
LDFLAGS += -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices
endif

ifeq ($(USE_SOUND),yes)
ifneq ($(USE_SDLAUDIO),yes)
CPPFLAGS+= -DNO_SDL_AUDIO
endif
endif

USE_SDLCD=yes
ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
CPPFLAGS+= -DWITH_SDLCD
endif
endif

# Unix builds rely on SDL:
# SDLQUAKE must be defined for all SDL using platforms/targets
CPPFLAGS+= -DSDLQUAKE
# not relying on sdl-config command and assuming
# /Library/Frameworks/SDL.framework is available
SDL_CFLAGS =-D_GNU_SOURCE=1 -D_THREAD_SAFE
SDL_CFLAGS+=-DSDL_FRAMEWORK -DNO_SDL_CONFIG
SDL_LIBS   =
# default to our local SDL[2].framework for build
SDL_FRAMEWORK_PATH ?=$(OSLIBS)/macosx
ifneq ($(SDL_FRAMEWORK_PATH),)
SDL_LIBS  +=-F$(SDL_FRAMEWORK_PATH)
SDL_CFLAGS+=-F$(SDL_FRAMEWORK_PATH)
endif
SDL_LIBS  +=-Wl,-framework,SDL -Wl,-framework,Cocoa
CFLAGS  += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LIBS)

GL_LINK=-Wl,-framework,OpenGL

ifeq ($(USE_CODEC_FLAC),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
ifeq ($(USE_CODEC_OPUS),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
ifeq ($(USE_CODEC_MIKMOD),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
ifeq ($(USE_CODEC_MODPLUG),yes)
CODEC_INC = -I$(OSLIBS)/macosx/codecs/include
CODEC_LINK= -L$(OSLIBS)/macosx/codecs/lib
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of Mac OS X settings
#############################################################


#############################################################
# Unix flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),unix)
# common unix:

NASMFLAGS=-f elf -d_NO_PREFIX

ifneq ($(X11BASE),)
INCLUDES+= -I$(X11BASE)/include
endif
ifeq ($(HOST_OS),qnx)
LDFLAGS += -lsocket
endif
ifeq ($(HOST_OS),sunos)
LDFLAGS += -lsocket -lnsl -lresolv
endif
LDFLAGS += -lm

ifneq ($(X11BASE),)
GL_LINK=-L$(X11BASE)/lib -lGL
else
GL_LINK=-lGL
endif

ifeq ($(USE_SOUND),yes)
ifneq ($(HOST_OS),linux)
# alsa is only for linux
USE_ALSA=no
endif
ifneq ($(USE_ALSA),yes)
CPPFLAGS+= -DNO_ALSA_AUDIO
else
# snd_alsa uses dlopen() & co.
LDFLAGS += -ldl
endif
ifneq ($(USE_OSS),yes)
CPPFLAGS+= -DNO_OSS_AUDIO
endif
ifneq ($(USE_SUNAUDIO),yes)
CPPFLAGS+= -DNO_SUN_AUDIO
endif
ifneq ($(USE_SDLAUDIO),yes)
CPPFLAGS+= -DNO_SDL_AUDIO
endif
endif

ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
CPPFLAGS+= -DWITH_SDLCD
endif
endif

# Unix builds rely on SDL:
# SDLQUAKE must be defined for all SDL using platforms/targets
CPPFLAGS+= -DSDLQUAKE
CFLAGS  += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LIBS)

ifeq ($(USE_CODEC_OPUS),yes)
# opus and opusfile put their *.h under <includedir>/opus,
# but they include the headers without the opus directory
# prefix and rely on pkg-config. ewww...
INCLUDES+= $(shell pkg-config --cflags opusfile)
LDFLAGS += $(shell pkg-config --libs   opusfile)
endif

endif
# End of Unix settings
#############################################################


#############################################################
# DOS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),dos)

NASMFLAGS=-f coff
# Use x86 assembly for DOS
USE_X86_ASM=yes

USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no
USE_CODEC_MIKMOD=no
USE_CODEC_MODPLUG=no
USE_CODEC_UMX=no
# no need for Opus on DOS
USE_CODEC_OPUS=no
# no need for Flac on DOS
USE_CODEC_FLAC=no
# use Tremor as Vorbis decoder for DOS?
VORBISLIB=tremor
# make DOS decoders allocate on the zone
CODECS_USE_ZONE=yes

INCLUDES += -I$(OSLIBS)/dos

# WATT-32 is needed for DOS
CPPFLAGS+= -DUSE_WATT32 -DWATT32_NO_OLDIES
INCLUDES+= -I$(OSLIBS)/dos/watt32/inc
LDFLAGS += -L$(OSLIBS)/dos/watt32/lib -lwatt

# DOS GL experiments:
INCLUDES += -I$(OSLIBS)/dos/opengl/include
INCLUDES += -I$(OSLIBS)/dos/glide3/include
GL_LINK = -L$(OSLIBS)/dos/opengl/lib/$(GLDIR) -lgl
# enable dynamic loading of gl functions
LINK_GL_LIBS=no
# enable 3dfx gamma hacks
USE_3DFXGAMMA=yes
ifeq ($(LINK_GL_LIBS),yes)
# compile which gl driver in: mesa, sage, or fxmesa.
DOSGL_DRIVER=fxmesa
ifeq ($(DOSGL_DRIVER),fxmesa)
CPPFLAGS+= -DREFGL_FXMESA
endif
ifeq ($(DOSGL_DRIVER),mesa)
CPPFLAGS+= -DREFGL_MESA
endif
ifeq ($(DOSGL_DRIVER),sage)
CPPFLAGS+= -DREFGL_SAGE
endif
GLDIR=$(DOSGL_DRIVER)
else
SYSOBJ_GL_DXE=yes
endif

LDFLAGS += -lc -lgcc -lm

# PCI sound card support through libau
USE_PCIAUDIO=yes
# PCI sound card support as a DXE module
PCIAUDIO_DXE=yes
ifeq ($(USE_SOUND),yes)
ifneq ($(USE_PCIAUDIO),yes)
CPPFLAGS+= -DNO_PCI_AUDIO
else
INCLUDES+= -I$(OSLIBS)/dos/libau/include
ifeq ($(PCIAUDIO_DXE),yes)
SYSOBJ_SOFT_DXE=yes
SYSOBJ_GL_DXE=yes
CPPFLAGS+= -DSNDPCI_DXE
else
LDFLAGS += -L$(OSLIBS)/dos/libau/lib -lau
endif
endif
endif

ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
ifeq ($(USE_CODEC_FLAC),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
ifeq ($(USE_CODEC_OPUS),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
ifeq ($(USE_CODEC_MIKMOD),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
ifeq ($(USE_CODEC_MODPLUG),yes)
CODEC_INC = -I$(OSLIBS)/dos/codecs/include
CODEC_LINK= -L$(OSLIBS)/dos/codecs/lib
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of DOS settings
#############################################################


#############################################################
# Win32 flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),win32)

NASMFLAGS=-f win32
GL_LINK=-lopengl32

ifeq ($(USE_SOUND),yes)
# enable the extra codecs
USE_CODEC_FLAC=yes
USE_CODEC_OPUS=yes
USE_CODEC_MIKMOD=yes
USE_CODEC_UMX=yes
endif
# timidity not needed, we have a midi driver. if you want it,
# comment out the following
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no

ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

CPPFLAGS+= -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include -I$(OSLIBS)/windows/dxsdk/include
LDFLAGS += $(LIBWINSOCK) -lwinmm -mwindows

ifneq ($(LINK_DIRECTX),yes)
CPPFLAGS+= -DDX_DLSYM
else
LDFLAGS += -L$(OSLIBS)/windows/dxsdk/x86 -ldsound -ldinput -ldxguid
endif

ifneq ($(WITH_SPLASHES),yes)
CPPFLAGS+= -DNO_SPLASHES
endif

ifeq ($(USE_CODEC_FLAC),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
ifeq ($(USE_CODEC_OPUS),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
ifeq ($(USE_CODEC_MIKMOD),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
ifeq ($(USE_CODEC_MODPLUG),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x86
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of Win32 settings
#############################################################


#############################################################
# Win64 flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),win64)

NASMFLAGS=-f win64 -d_NO_PREFIX
GL_LINK=-lopengl32

ifeq ($(USE_SOUND),yes)
# enable the extra codecs
USE_CODEC_FLAC=yes
USE_CODEC_OPUS=yes
USE_CODEC_MIKMOD=yes
USE_CODEC_UMX=yes
endif
# timidity not needed, we have a midi driver. if you want it,
# comment out the following
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no

# cant use MGL for Win64
USE_MGL=no
# use winsock2 for win64
USE_WINSOCK2=yes

ifeq ($(USE_WINSOCK2),yes)
CPPFLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

CPPFLAGS+= -DWIN32_LEAN_AND_MEAN
INCLUDES+= -I$(OSLIBS)/windows/misc/include -I$(OSLIBS)/windows/dxsdk/include
LDFLAGS += $(LIBWINSOCK) -lwinmm -mwindows

ifneq ($(LINK_DIRECTX),yes)
CPPFLAGS+= -DDX_DLSYM
else
LDFLAGS += -L$(OSLIBS)/windows/dxsdk/x64 -ldsound -ldinput -ldxguid
endif

ifneq ($(WITH_SPLASHES),yes)
CPPFLAGS+= -DNO_SPLASHES
endif

ifeq ($(USE_CODEC_FLAC),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
ifeq ($(USE_CODEC_OPUS),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
ifeq ($(USE_CODEC_MIKMOD),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
ifeq ($(USE_CODEC_MODPLUG),yes)
CODEC_INC = -I$(OSLIBS)/windows/codecs/include
CODEC_LINK= -L$(OSLIBS)/windows/codecs/x64
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of Win64 settings
#############################################################


#############################################################
# MorphOS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),morphos)

# MorphOS builds can use SDL or native code
USE_SDL=no
ifneq ($(USE_SDL),yes)
USE_SDLCD=no
USE_SDLAUDIO=no
# no native cdaudio code yet
USE_CDAUDIO=no
endif

CFLAGS  += -noixemul
LDFLAGS += -noixemul
LDFLAGS += -lm

GL_LINK=-lGL
# dynamic loading of ogl functions doesn't work
LINK_GL_LIBS=yes

USE_CODEC_FLAC=no
USE_CODEC_VORBIS=yes
USE_CODEC_MP3=yes
USE_CODEC_OPUS=no
USE_CODEC_MIKMOD=no
USE_CODEC_MODPLUG=no
USE_CODEC_UMX=no

ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/morphos/codecs/include
CODEC_LINK= -L$(OSLIBS)/morphos/codecs/lib
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

ifeq ($(USE_SOUND),yes)
ifneq ($(USE_SDLAUDIO),yes)
CPPFLAGS+= -DNO_SDL_AUDIO
endif
endif

ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
CPPFLAGS+= -DWITH_SDLCD
endif
endif

ifeq ($(USE_SDL),yes)
# SDLQUAKE must be defined for all SDL using platforms/targets
CPPFLAGS+= -DSDLQUAKE
CFLAGS  += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LIBS)
endif

endif
# End of MorphOS settings
#############################################################


#############################################################
# AROS flags/settings and overrides:
#############################################################
ifeq ($(TARGET_OS),aros)

# AROS builds can use SDL or native code
USE_SDL=no
ifneq ($(USE_SDL),yes)
USE_SDLCD=no
USE_SDLAUDIO=no
# no native cdaudio code yet
USE_CDAUDIO=no
# native ogl code need this:
LINK_GL_LIBS=yes
endif

# Don't use x86 assembly for AROS
USE_X86_ASM=no

GL_LINK=-lGL

# both WildMidi and the local timidity work nice on AROS
USE_CODEC_WILDMIDI=no
USE_CODEC_TIMIDITY=yes

ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC = -I$(OSLIBS)/aros-$(MACH_TYPE)/codecs/include
CODEC_LINK= -L$(OSLIBS)/aros-$(MACH_TYPE)/codecs/lib
endif
ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC = -I$(OSLIBS)/aros-$(MACH_TYPE)/codecs/include
CODEC_LINK= -L$(OSLIBS)/aros-$(MACH_TYPE)/codecs/lib
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

ifeq ($(USE_SOUND),yes)
ifneq ($(USE_SDLAUDIO),yes)
CPPFLAGS+= -DNO_SDL_AUDIO
endif
endif

ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
CPPFLAGS+= -DWITH_SDLCD
endif
endif

ifeq ($(USE_SDL),yes)
# SDLQUAKE must be defined for all SDL using platforms/targets
CPPFLAGS+= -DSDLQUAKE
CFLAGS  += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LIBS)
endif

endif
# End of AROS settings
#############################################################


#############################################################
# Streaming music settings
#############################################################
ifeq ($(MP3LIB),mad)
mp3_obj=snd_mp3.o
lib_mp3dec=-lmad
endif
ifeq ($(MP3LIB),mpg123)
mp3_obj=snd_mpg123.o
lib_mp3dec=-lmpg123
endif
ifeq ($(VORBISLIB),vorbis)
cpp_vorbisdec=
ifneq ($(TARGET_OS),morphos)
lib_vorbisdec=-lvorbisfile -lvorbis -logg
endif
endif
ifeq ($(VORBISLIB),tremor)
cpp_vorbisdec=-DVORBIS_USE_TREMOR
lib_vorbisdec=-lvorbisidec -logg
endif

ifeq ($(USE_CODEC_FLAC),yes)
CPPFLAGS+= -DUSE_CODEC_FLAC
LDFLAGS+=  -lFLAC
endif
ifeq ($(USE_CODEC_WAVE),yes)
CPPFLAGS+= -DUSE_CODEC_WAVE
endif
ifeq ($(USE_CODEC_OPUS),yes)
CPPFLAGS+= -DUSE_CODEC_OPUS
ifneq ($(TARGET_OS),unix)
# we use pkg-config on unix
LDFLAGS+= -lopusfile -lopus -logg
endif
endif
ifeq ($(USE_CODEC_VORBIS),yes)
CPPFLAGS+= -DUSE_CODEC_VORBIS $(cpp_vorbisdec)
LDFLAGS+= $(lib_vorbisdec)
endif
ifeq ($(USE_CODEC_MP3),yes)
CPPFLAGS+= -DUSE_CODEC_MP3
LDFLAGS+= $(lib_mp3dec)
endif
ifeq ($(USE_CODEC_MIKMOD),yes)
CPPFLAGS+= -DUSE_CODEC_MIKMOD
LDFLAGS+=  -lmikmod
endif
ifeq ($(USE_CODEC_MODPLUG),yes)
CPPFLAGS+= -DUSE_CODEC_MODPLUG
LDFLAGS+=  -lmodplug
endif
ifeq ($(USE_CODEC_UMX),yes)
CPPFLAGS+= -DUSE_CODEC_UMX
endif
ifeq ($(USE_CODEC_TIMIDITY),yes)
CPPFLAGS+= -DUSE_CODEC_TIMIDITY
ifeq ($(TIMIDITY_USE_DLS),yes)
CPPFLAGS+= -DTIMIDITY_USE_DLS
endif
LDFLAGS+= -L$(LIBS_DIR)/timidity -ltimidity
TIMIDEPS=$(LIBS_DIR)/timidity/libtimidity.a
else
TIMIDEPS=
endif
ifeq ($(USE_CODEC_WILDMIDI),yes)
CPPFLAGS+= -DUSE_CODEC_WILDMIDI
LDFLAGS+= -lWildMidi
endif

ifeq ($(CODECS_USE_ZONE),yes)
CPPFLAGS+=-DCODECS_USE_ZONE
endif

# End of streaming music settings
#############################################################

#############################################################
# Finalize things after the system specific overrides:
#############################################################

ifeq ($(USE_X86_ASM),yes)
CPPFLAGS+= -DUSE_INTEL_ASM
endif

ifeq ($(USE_3DFXGAMMA),yes)
# linux 3dfx gamma evil hack
GL_DEFS+= -DUSE_3DFXGAMMA
endif
ifneq ($(LINK_GL_LIBS),yes)
GL_DEFS+= -DGL_DLSYM
else
GL_LIBS+= $(GL_LINK)
endif
#
#############################################################


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(COMMON_HW)/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: %.m
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(COMMONDIR)/%.m
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.m
	$(CC) -c $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ $<
%.o: %.asm
	$(NASM) $(NASM_INC) $(NASMFLAGS) -o $@ $<
%.o: $(COMMON_HW)/%.asm
	$(NASM) $(NASM_INC) $(NASMFLAGS) -o $@ $<
%.o: $(COMMONDIR)/%.asm
	$(NASM) $(NASM_INC) $(NASMFLAGS) -o $@ $<
%.res: %.rc
	$(WINDRES) $(RES_INC) $(CPPFLAGS) --output-format=coff -o $@ $<
%.res: $(COMMON_HW)/%.rc
	$(WINDRES) $(RES_INC) $(CPPFLAGS) --output-format=coff -o $@ $<
%.res: $(COMMONDIR)/%.rc
	$(WINDRES) $(RES_INC) $(CPPFLAGS) --output-format=coff -o $@ $<

# Objects

# Intel asm objects
ifeq ($(USE_X86_ASM),yes)
COMMON_ASM= math.o \
	sys_ia32.o

SOFT_ASM = \
	d_draw.o \
	d_draw16.o \
	d_draw16t.o \
	d_parta.o \
	d_partb.o \
	d_polysa.o \
	d_polysa2.o \
	d_polysa3.o \
	d_polysa4.o \
	d_polysa5.o \
	d_scana.o \
	d_spr8.o \
	d_spr8t.o \
	d_spr8t2.o \
	d_varsa.o \
	r_aclipa.o \
	r_aliasa.o \
	r_drawa.o \
	r_edgea.o \
	r_edgeb.o \
	r_varsa.o \
	surf8.o \
	surf16.o

SOUND_ASM = snd_mixa.o

else

SOFT_ASM =
COMMON_ASM =
SOUND_ASM =

endif

# Sound objects
ifneq ($(USE_SOUND),yes)
MUSIC_OBJS:= bgmnull.o
SOUND_ASM :=
CPPFLAGS += -D_NO_SOUND
SYSOBJ_SND :=
COMOBJ_SND := snd_null.o $(MUSIC_OBJS)
else
MUSIC_OBJS:= bgmusic.o \
	snd_codec.o \
	snd_flac.o \
	snd_wave.o \
	snd_vorbis.o \
	snd_opus.o \
	$(mp3_obj) \
	snd_mikmod.o \
	snd_modplug.o \
	snd_umx.o \
	snd_timidity.o \
	snd_wildmidi.o
COMOBJ_SND := snd_sys.o snd_dma.o snd_mix.o $(SOUND_ASM) snd_mem.o $(MUSIC_OBJS)
ifeq ($(TARGET_OS),win32)
SYSOBJ_SND := snd_win.o snd_dsound.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_SND := snd_win.o snd_dsound.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_SND := snd_sb.o snd_gus.o snd_pci.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SND := snd_oss.o snd_alsa.o snd_sun.o snd_sdl.o
endif
ifeq ($(TARGET_OS),darwin)
SYSOBJ_SND := snd_sdl.o
endif
ifeq ($(TARGET_OS),aros)
SYSOBJ_SND := snd_ahi.o snd_sdl.o
endif
ifeq ($(TARGET_OS),morphos)
SYSOBJ_SND := snd_ahi.o snd_sdl.o
endif
# end of Sound objects
endif

# MIDI objects
ifneq ($(USE_MIDI),yes)
SYSOBJ_MIDI:= midi_nul.o
CPPFLAGS += -D_NO_MIDIDRV
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_MIDI:= midi_win.o mid2strm.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_MIDI:= midi_win.o mid2strm.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_MIDI:= midi_nul.o
CPPFLAGS += -D_NO_MIDIDRV
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_MIDI:= midi_nul.o
CPPFLAGS += -D_NO_MIDIDRV
endif
ifeq ($(TARGET_OS),darwin)
# not using midi_mac.c
SYSOBJ_MIDI:= midi_osx.o
endif
ifeq ($(TARGET_OS),aros)
SYSOBJ_MIDI:= midi_nul.o
CPPFLAGS += -D_NO_MIDIDRV
endif
ifeq ($(TARGET_OS),morphos)
SYSOBJ_MIDI:= midi_nul.o
CPPFLAGS += -D_NO_MIDIDRV
endif
# end of MIDI objects
endif

# CDAudio objects
ifneq ($(USE_CDAUDIO),yes)
SYSOBJ_CDA:= cd_null.o
CPPFLAGS += -D_NO_CDAUDIO
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_CDA := cd_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_CDA := cd_win.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_CDA := cd_dos.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_CDA := cd_sdl.o cd_bsd.o cd_linux.o
endif
ifeq ($(TARGET_OS),darwin)
SYSOBJ_CDA := cd_sdl.o
endif
ifeq ($(TARGET_OS),aros)
SYSOBJ_CDA := cd_sdl.o
endif
ifeq ($(TARGET_OS),morphos)
SYSOBJ_CDA := cd_sdl.o
endif
# end of CDAudio objects
endif

# Other platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_INPUT := in_win.o
SYSOBJ_GL_VID:= gl_vidnt.o
ifeq ($(USE_MGL),yes)
SYSOBJ_SOFT_VID:= vid_mgl4.o
else
SYSOBJ_SOFT_VID:= vid_win.o
endif
SYSOBJ_SYS := sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_INPUT := in_win.o
SYSOBJ_GL_VID:= gl_vidnt.o
SYSOBJ_SOFT_VID:= vid_win.o
SYSOBJ_SYS := sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_INPUT := in_dos.o
SYSOBJ_SOFT_VID:= vid_vga.o vid_ext.o vid_dos.o vregset.o
SYSOBJ_GL_VID:= dos_dmesa.o dos_fxmesa.o dos_sage.o gl_viddos.o
ifeq ($(SYSOBJ_GL_DXE),yes)
SYSOBJ_GL_VID+= dxe.o
else
SYSOBJ_GL_VID+= no_dxe.o
endif
ifeq ($(USE_3DFXGAMMA),yes)
SYSOBJ_GL_VID+= fx_gamma.o
endif
ifeq ($(USE_X86_ASM),yes)
SYSOBJ_SOFT_VID+= d_copy.o
endif
ifeq ($(SYSOBJ_SOFT_DXE),yes)
SYSOBJ_SOFT_VID+= dxe.o
else
SYSOBJ_SOFT_VID+= no_dxe.o
endif
SYSOBJ_SYS := dos_v2.o sys_dos.o
#	dosasm.o
endif
ifeq ($(TARGET_OS),unix)
# unix clients use SDL:
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o
ifeq ($(USE_3DFXGAMMA),yes)
SYSOBJ_GL_VID+= fx_gamma.o
endif
SYSOBJ_SOFT_VID:= vid_sdl.o
SYSOBJ_SYS := sys_unix.o
SYSOBJ_SYS += sys_sdl.o
endif
ifeq ($(TARGET_OS),darwin)
# OSX clients use SDL:
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o
SYSOBJ_SOFT_VID:= vid_sdl.o
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_unix.o
SYSOBJ_SYS += sys_osx.o
SYSOBJ_SYS += SDLMain.o
endif
ifeq ($(TARGET_OS),aros)
ifeq ($(USE_SDL),yes)
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o
SYSOBJ_SOFT_VID:= vid_sdl.o
else
SYSOBJ_INPUT := in_amiga.o
SYSOBJ_GL_VID:= gl_vidamiga.o
SYSOBJ_SOFT_VID:= vid_cgx.o
endif
SYSOBJ_SYS := sys_amiga.o
endif
ifeq ($(TARGET_OS),morphos)
ifeq ($(USE_SDL),yes)
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o
SYSOBJ_SOFT_VID:= vid_sdl.o
else
SYSOBJ_INPUT := in_amiga.o
SYSOBJ_GL_VID:= gl_vidamiga.o
SYSOBJ_SOFT_VID:= vid_cgx.o
endif
SYSOBJ_SYS := sys_amiga.o
endif

# Final list of objects
SOFTOBJS = \
	d_edge.o \
	d_fill.o \
	d_init.o \
	d_modech.o \
	d_part.o \
	d_polyse.o \
	d_scan.o \
	d_sky.o \
	d_sprite.o \
	d_surf.o \
	d_vars.o \
	d_zpoint.o \
	r_aclip.o \
	r_alias.o \
	r_bsp.o \
	r_draw.o \
	r_edge.o \
	r_efrag.o \
	r_light.o \
	r_main.o \
	r_misc.o \
	r_part.o \
	r_sky.o \
	r_sprite.o \
	r_surf.o \
	r_vars.o \
	screen.o \
	$(SYSOBJ_SOFT_VID) \
	draw.o \
	model.o

GLOBJS = \
	gl_refrag.o \
	gl_rlight.o \
	gl_rmain.o \
	gl_rmisc.o \
	gl_ngraph.o \
	r_part.o \
	gl_rsurf.o \
	gl_screen.o \
	gl_warp.o \
	$(SYSOBJ_GL_VID) \
	gl_draw.o \
	gl_mesh.o \
	gl_model.o

COMMONOBJS = \
	$(SYSOBJ_INPUT) \
	$(COMOBJ_SND) \
	$(SYSOBJ_SND) \
	$(SYSOBJ_CDA) \
	$(SYSOBJ_MIDI) \
	huffman.o \
	net_udp.o \
	net_chan.o \
	cl_cam.o \
	cl_demo.o \
	cl_effect.o \
	cl_ents.o \
	cl_inlude.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_pred.o \
	cl_tent.o \
	cl_cmd.o \
	console.o \
	keys.o \
	menu.o \
	sbar.o \
	skin.o \
	view.o \
	wad.o \
	cmd.o \
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
	info_str.o \
	crc.o \
	cvar.o \
	cfgfile.o \
	host_string.o \
	mathlib.o \
	pmove.o \
	pmovetst.o \
	zone.o \
	$(SYSOBJ_SYS)


# Targets
.PHONY: help clean distclean localclean report $(TIMIDEPS)

#default: glhw
#all: default

help: report
	@echo
	@echo "Valid targets: (read/edit the makefile for several options)"
	@echo
	@echo "* $(MAKE) help   : this help"
	@echo "* $(MAKE) clean  : delete all files produced by the build"
	@echo "* $(MAKE) hw     : hexenworld client, software renderer"
	@echo "* $(MAKE) glhw   : hexenworld client, opengl renderer"
	@echo

hw: $(SW_BINARY)
glhw: $(GL_BINARY)

ifeq ($(TARGET_OS),win32)
ifeq ($(USE_MGL),yes)
$(SW_BINARY): CPPFLAGS+= -DMGL_DLL
$(SW_BINARY): INCLUDES+= -I$(OSLIBS)/windows/scitech/include
$(SW_BINARY): LDFLAGS += -L$(OSLIBS)/windows/scitech/x86 -lmglfx
endif
endif
$(GL_BINARY): CPPFLAGS+= $(GL_DEFS)

# extra rule for snd_timidity.c because of timidity.h and libtimidity.a
snd_timidity.o: $(COMMONDIR)/snd_timidity.c
	$(CC) -c $(INCLUDES) -I$(LIBS_DIR)/timidity $(CPPFLAGS) $(CFLAGS) -DTIMIDITY_STATIC=1 -o $@ $(COMMONDIR)/snd_timidity.c

include $(LIBS_DIR)/timidity/_timi.mak

$(SW_BINARY): $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS) $(TIMIDEPS)
	$(LINKER) $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) -o $@

$(GL_BINARY): $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(TIMIDEPS)
	$(LINKER) $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(GL_LIBS) -o $@

localclean:
	rm -f *.o *.res core
clean: localclean timi_clean
distclean: clean
	rm -f $(SW_BINARY) $(GL_BINARY)

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)

