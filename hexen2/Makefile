# GNU Makefile for (gl)hexen2 client/server binaries using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/hexen2/Makefile,v 1.101 2010-08-23 00:20:31 sezero Exp $
#
# Type "make help" to see the valid targets.
#
# It is ESSENTIAL that you run make clean between different
# types of builds or different types of targets.
#
# Unix platforms require SDL !!!
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# To cross-compile for Win64 on Unix, you must pass the W64BUILD=1
# argument to make. Also see build_cross_win64.sh for details.
#
# To (cross-)compile for DOS, you must pass the DOSBUILD=1 argument
# to make, such as "DOSBUILD=1 make h2". You need djgpp compiler set.
# Also examine the build_cross_dos.sh script for cross compilation
# on linux / unix.
#
# Build Options:
#
# USE_SOUND :	yes  =	Include sound support (default).
#		no   =	Don't include sound support.
#
# USE_SDLAUDIO:	yes  =	Include SDL audio support (default).
# (for Unix)	no   =	Don't include SDL audio support.
#		SDL audio is enabled on all unix-like platforms.
#
# USE_ALSA :	yes  =	Include ALSA sound support (default).
# (for Unix)	no   =	Don't include alsa support.
#		Requires alsa-lib and alsa-kernel modules >= 1.0.1
#		(0.9.8 and 1.0.0 should also work, but isn't supported.)
#		On any platform other than Linux, ALSA support will be
#		automatically be disabled.
#
# USE_OSS :	yes  =	Include OSS audio support (default).
# (for Unix)	no   =	Don't include OSS support.
#		OSS is enabled on Linux and FreeBSD.
#		It is automatically disabled on other platforms.
#		See snd_sys.h for a more complete list.
#
# USE_SUNAUDIO:	yes  =	Include SUN audio support (default).
# (for Unix)	no   =	Don't include SUN audio support.
#		SUN audio is enabled on OpenBSD, NetBSD and SUN.
#		It is automatically disabled on other platforms.
#		See snd_sys.h for a more complete list.
#
# USE_CDAUDIO :	yes  =	Include CDAudio support (default).
#		no   =	Don't include CDAudio support.
#
# USE_SDLCD:	no   =	Don't use SDL cdaudio (default); platform specific
# (for Unix)		cdrom code will be used.  The only problem with
#			SDL_cdrom is that it lacks proper volume controls.
#			See cd_unix.h for the availability of platform
#			specific cdaudio drivers.
#		yes  =	Use SDL for cdaudio instead of platform specific code
#		NOTE :	SDL dropped support for cd audio in version 1.3.0 and
#			later!
#
# USE_MIDI :	yes  =	Include MIDI music support (default).
#		no   =	Don't include MIDI music support.
#		On Unix, MIDI needs SDL_mixer (built with Timidity support)
#		and Timidity patches.
#
# WITH_SPLASHES	yes  =	(default) enable startup splash screens
# (for Win32)	no   =	disable startup splash screens
#
# USE_MGL	yes  =	Use SciTech MGL for Win32 software video driver.
#			Defaults to yes for x86. Force-set to no for Win64.
# (for Win32)	no   =	Use the DIB/GDI-only compatibility software video
#			driver, which is the default for Win64.
#
# USE_WINSOCK2	yes  =	Use WinSock2 and link to ws2_32 instead of wsock32
# (for Win32)	no   =	(default) Use WinSock1.1 for compatibility with old
#			Windows 95 machines.
#
# LINK_DIRECTX	no  =	(default) Don't link to the dsound and dinput at compile
# (for Win32)		time. The binaries will dynamically load the necessary
#			dll files and functions at runtime. This ensures our exe
#			to run on ancient windows versions without any directx
#			installation.
#		yes =	link to the directx libraries at compile time
#
# LINK_GL_LIBS	no  =	(default) Don't link to the opengl libraries at compile
#			time. The binaries will dynamically load the necessary
#			libraries and functions at runtime.
#		yes =	link to the opengl libraries at compile time
#
# USE_3DFXGAMMA	yes =	enable 3dfx glide hacks for native hardware gamma for
# (Linux / FreeBSD)	the old Voodoo Graphics and Voodoo2 cards.
#		no  =	disable 3dfx glide gamma hacks (default).
#
# OPT_EXTRA	yes  =  Some extra optimization flags will be added (default)
#		no   =	No extra optimizations will be made
#
# USE_X86_ASM	yes  =  Use x86 assembly on ia32 machines which is faster.
#			Automatically disabled for any other cpu.
#		no   =	Do not use intel assembly even on ix86.
#
# COMPILE_32BITS yes =  Compile as a 32 bit binary. If you are on a 64 bit
#			platform and having problems with 64 bit compiled
#			binaries, set this option to yes. Default: no .
#			If you set this to yes, you need to have the 32 bit
#			versions of the libraries that you link against.
#		no   =	Compile for the native word size of your platform,
#			which is the default option.
#
# The default compiler is gcc
# To build with a different compiler:	make CC=compiler_name [other stuff]
#
# The default x86 assembler is nasm. To use a different nasm-compatible
# assembler, such as yasm	:	make NASM=yasm [other stuff]
#
# If X installation is somewhere other than /usr/X11R6, such as /usr/X11R7,
# specify it on the command line:	make X11BASE=/your/x11/path [other stuff]
#
# To build for the demo version:	make DEMO=1 [other stuff]
#
# if building a debug version :		make DEBUG=1 [other stuff]
#

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=..

# General options (see explanations at the top)
USE_SOUND=yes
USE_ALSA=yes
USE_OSS=yes
USE_SUNAUDIO=yes
USE_SDLAUDIO=yes
USE_MIDI=yes
USE_CDAUDIO=yes
USE_SDLCD=no
WITH_SPLASHES=yes
USE_MGL=yes
USE_WINSOCK2=no
LINK_DIRECTX=no
OPT_EXTRA=yes
LINK_GL_LIBS=no
USE_3DFXGAMMA=no
USE_X86_ASM=yes
COMPILE_32BITS=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

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
ifeq ($(TARGET_OS),dos)
# x86 assembly if is needed for dos
USE_X86_ASM=yes
endif

ifeq ($(TARGET_OS),win64)
# cant use MGL for Win64
USE_MGL=no
# use winsock2 for win64
USE_WINSOCK2=yes
endif

# Names of the binaries
ifeq ($(TARGET_OS),win32)
H2_BINARY:=h2.exe
GLH2_BINARY:=glh2.exe
endif
ifeq ($(TARGET_OS),win64)
H2_BINARY:=h2.exe
GLH2_BINARY:=glh2.exe
endif
ifeq ($(TARGET_OS),dos)
H2_BINARY:=h2dos.exe
endif
ifeq ($(TARGET_OS),unix)
H2_BINARY:=hexen2$(exe_ext)
GLH2_BINARY:=glhexen2$(exe_ext)
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

CFLAGS := $(CPUFLAGS) -O2 -Wall -DNDEBUG -ffast-math -fexpensive-optimizations

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

CFLAGS := $(CFLAGS) $(ARCHFLAGS)
# end of compiler flags


# Other build flags
EXT_FLAGS:=

# whether to use x86 assembly
ifeq ($(USE_X86_ASM),yes)
EXT_FLAGS:= $(EXT_FLAGS) -DUSE_INTEL_ASM
endif

ifeq ($(USE_WINSOCK2),yes)
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

ifneq ($(USE_MGL),yes)
MGL_FLAG:=
MGL_INCL:=
MGL_LINK:=
endif

ifeq ($(TARGET_OS),win32)
# Main win32 specific includes and flags
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(W32STUFF)/dxsdk/inc -I.
LDFLAGS := $(LIBWINSOCK) -lwinmm -mwindows
endif
ifeq ($(TARGET_OS),win64)
# Main win64 specific includes and flags
CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(W32STUFF)/dxsdk_new/inc -I.
LDFLAGS := $(LIBWINSOCK) -lwinmm -mwindows
endif
ifeq ($(TARGET_OS),dos)
# Main dos specific includes and flags
INCLUDES:= -I.
LDFLAGS := -lc -lgcc -lm
ifndef DEBUG
LDFLAGS := $(LDFLAGS) -s
endif
endif
ifeq ($(TARGET_OS),unix)
# Main unix specific includes and flags
INCLUDES:= -I$(X11BASE)/include -I.
LDFLAGS := $(LIBSOCKET)
endif

ifeq ($(COMPILE_32BITS),yes)
LDFLAGS := $(LDFLAGS) -m32
endif

GL_DEFS := -DGLQUAKE
GLFLAGS :=

# allow 3dfx gamma hack only on linux and freebsd
ifneq ($(HOST_OS),linux)
ifneq ($(HOST_OS),freebsd)
USE_3DFXGAMMA=no
endif
endif
ifeq ($(USE_3DFXGAMMA),yes)
GL_DEFS+= -DUSE_3DFXGAMMA
endif

ifeq ($(LINK_GL_LIBS),no)
GL_DEFS+= -DGL_DLSYM
else
GLFLAGS+= $(GL_LINK)
endif

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates come extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG=1 -DDEBUG_BUILD=1
endif

ifeq ($(TARGET_OS),unix)
# Start Unix-specific extra flags/settings

# We rely on SDL:  SDLQUAKE must be defined for all SDL using
# platforms / targets.
EXT_FLAGS+= -DSDLQUAKE

ifeq ($(USE_SOUND),yes)
ifeq ($(USE_ALSA),no)
EXT_FLAGS+= -DNO_ALSA_AUDIO
endif
ifeq ($(USE_OSS),no)
EXT_FLAGS+= -DNO_OSS_AUDIO
endif
ifeq ($(USE_SUNAUDIO),no)
EXT_FLAGS+= -DNO_SUN_AUDIO
endif
ifeq ($(USE_SDLAUDIO),no)
EXT_FLAGS+= -DNO_SDL_AUDIO
endif
endif

ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
EXT_FLAGS+= -DWITH_SDLCD
endif
endif

# End of Unix specific extras
endif


ifeq ($(TARGET_OS),win32)
# Start Win32-specific extra flags/settings

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
endif

ifeq ($(LINK_DIRECTX),no)
EXT_FLAGS+= -DDX_DLSYM
else
LDFLAGS += -L$(W32STUFF)/dxsdk/lib -ldsound -ldinput -ldxguid
endif

ifeq ($(WITH_SPLASHES),no)
EXT_FLAGS+= -DNO_SPLASHES
else
# gl version needs linking to comctl32.dll because of that progress
# bar thingy. it is necessary for the InitCommonControls call, only
# when USE_HEXEN2_PALTEX_CODE is defined as 0 in h2option.h
#GLFLAGS+=-lcomctl32
endif
# End of Win32 specific extras
endif


ifeq ($(TARGET_OS),win64)
# Start Win64-specific extra flags/settings

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
endif

ifeq ($(LINK_DIRECTX),no)
EXT_FLAGS+= -DDX_DLSYM
else
LDFLAGS += -ldsound -ldinput -ldxguid
endif

ifeq ($(WITH_SPLASHES),no)
EXT_FLAGS+= -DNO_SPLASHES
else
# gl version needs linking to comctl32.dll because of that progress
# bar thingy. it is necessary for the InitCommonControls call, only
# when USE_HEXEN2_PALTEX_CODE is defined as 0 in h2option.h
#GLFLAGS+=-lcomctl32
endif
# End of Win64 specific extras
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(SDLFLAGS) $(INCLUDES) -o $@ $<
%.o: %.asm
	$(NASM) $(NASMFLAGS) -o $@ $<
%.res: %.rc
	$(WINDRES) -DWIN32_LEAN_AND_MEAN $(EXT_FLAGS) --output-format=coff -o $@ $<

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
WORLD_ASM = worlda.o

else

SOFT_ASM =
COMMON_ASM =
SOUND_ASM =
WORLD_ASM =

endif

# Sound objects
ifneq ($(USE_SOUND),yes)
SOUND_ASM :=
EXT_FLAGS += -D_NO_SOUND
SYSOBJ_SND :=
COMOBJ_SND := snd_null.o
else
COMOBJ_SND := snd_sys.o snd_dma.o snd_mix.o $(SOUND_ASM) snd_mem.o
ifeq ($(TARGET_OS),win32)
SYSOBJ_SND := win_stuff/snd_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_SND := win_stuff/snd_win.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_SND := dos/snd_dos.o dos/snd_gus.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SND := snd_oss.o snd_alsa.o snd_sun.o snd_sdl2.o
endif
# end of Sound objects
endif

# MIDI objects
ifneq ($(USE_MIDI),yes)
SYSOBJ_MIDI:= midi_nul.o
EXT_FLAGS += -DNO_MIDIMUSIC
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_MIDI:= win_stuff/midi.o win_stuff/mstrconv.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_MIDI:= win_stuff/midi.o win_stuff/mstrconv.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_MIDI:= midi_nul.o
EXT_FLAGS += -DNO_MIDIMUSIC
endif
ifeq ($(TARGET_OS),unix)
ifeq ($(HOST_OS),darwin)
SYSOBJ_MIDI:= midi_mac.o
else
SYSOBJ_MIDI:= midi_sdl.o
SDLLINK+= -lSDL_mixer
EXT_FLAGS += -D_MIDI_SDLMIXER
endif
endif
# end of MIDI objects
endif

# CDAudio objects
ifneq ($(USE_CDAUDIO),yes)
SYSOBJ_CDA:= cd_null.o
EXT_FLAGS += -D_NO_CDAUDIO
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_CDA := win_stuff/cd_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_CDA := win_stuff/cd_win.o
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_CDA := dos/cd_dos.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_CDA := cd_sdl.o cd_bsd.o cd_linux.o
endif
# end of CDAudio objects
endif

# Other platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_INPUT := win_stuff/in_win.o
SYSOBJ_GL_VID:= win_stuff/gl_vidnt.o
ifeq ($(USE_MGL),yes)
SYSOBJ_SOFT_VID:= win_stuff/vid_mgl4.o
else
SYSOBJ_SOFT_VID:= win_stuff/vid_win.o
endif
SYSOBJ_NET := win_stuff/net_win.o win_stuff/net_wins.o win_stuff/net_wipx.o
SYSOBJ_SYS := win_stuff/conproc.o win_stuff/sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_INPUT := win_stuff/in_win.o
SYSOBJ_GL_VID:= win_stuff/gl_vidnt.o
SYSOBJ_SOFT_VID:= win_stuff/vid_win.o
SYSOBJ_NET := win_stuff/net_win.o win_stuff/net_wins.o win_stuff/net_wipx.o
SYSOBJ_SYS := win_stuff/conproc.o win_stuff/sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),dos)
SYSOBJ_INPUT := dos/in_dos.o
SYSOBJ_SOFT_VID:= dos/vid_vga.o dos/vid_ext.o d_copy.o dos/vid_dos.o dos/vregset.o
SYSOBJ_NET := dos/net_dos.o dos/net_ser.o dos/net_bw.o dos/dos_inet.o dos/inet_addr.o \
	dos/net_ipx.o dos/net_mp.o dos/mplib.o dos/mplpc.o
SYSOBJ_SYS := dos/dos_v2.o dos/sys_dos.o
#	dosasm.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o fx_gamma.o
SYSOBJ_SOFT_VID:= vid_sdl.o
SYSOBJ_NET := net_bsd.o net_udp.o
SYSOBJ_SYS := sys_unix.o
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
	$(SYSOBJ_NET) \
	net_dgrm.o \
	net_loop.o \
	net_main.o \
	chase.o \
	cl_demo.o \
	cl_effect.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_tent.o \
	cl_cmd.o \
	console.o \
	keys.o \
	menu.o \
	sbar.o \
	view.o \
	wad.o \
	cmd.o \
	q_endian.o \
	link_ops.o \
	sizebuf.o \
	strlcat.o \
	strlcpy.o \
	msg_io.o \
	common.o \
	debuglog.o \
	quakefs.o \
	crc.o \
	cvar.o \
	cfgfile.o \
	host.o \
	host_cmd.o \
	pr_strng.o \
	mathlib.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	sv_effect.o \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_user.o \
	$(WORLD_ASM) \
	world.o \
	zone.o \
	$(SYSOBJ_SYS)


# Targets
.PHONY: help clean cleaner report

#default: glh2
#all: default

help:
	@echo
	@echo "Valid targets: (read/edit the makefile for several options)"
	@echo
	@echo "* $(MAKE) help        : this help"
	@echo "* $(MAKE) clean       : delete all files produced by the build"
	@echo "* $(MAKE) h2          : hexen2 binary, software renderer"
	@echo "* $(MAKE) glh2        : hexen2 binary, opengl renderer"
	@echo

hexen2: h2
glhexen2: glh2

h2: EXT_FLAGS+=$(MGL_FLAG)
h2: INCLUDES+=$(MGL_INCL)
h2: $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(H2_BINARY) $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(SDLLINK) $(MGL_LINK)

glh2: EXT_FLAGS+= $(GL_DEFS)
glh2: $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(GLH2_BINARY) $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(GLFLAGS) $(SDLLINK)

clean:
	rm -f *.o *.res win_stuff/*.o dos/*.o sv_objs/*.o core

cleaner: clean
	rm -f $(H2_BINARY) $(GLH2_BINARY)

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)

