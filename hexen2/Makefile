#
# Type "make help" to see the valid targets.
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
# USE_ALSA :	yes  =	Include ALSA sound support (default).
# (for Unix)	no   =	Don't include alsa support.
#		Requires alsa-lib and alsa-kernel modules >= 1.0.1
#		(0.9.8 and 1.0.0 should also work, but isn't supported.)
#		On any platform other than Linux, ALSA support will be
#		automatically be disabled.
#
# USE_SDLCD:	no   =	Don't use SDL cdaudio (default); platform specific
# (for Unix)		cdrom code will be used.  The only problem with
#			SDL_cdrom is that it lacks proper volume controls.
#		yes  =	Use SDL for cdaudio instead of platform specific code
#
# USE_MIDI :	yes  =	Include MIDI music support (default).
# (for Unix)	no   =	Don't include MIDI music support.
#		MIDI needs SDL_mixer (built with Timidity support)
#		and Timidity patches.
#
# WITH_SPLASHES	yes  =	(default) enable startup splash screens
# (for Win32)	no   =	disable startup splash screens
#
# LINK_GL_LIBS	no  =	(default) Don't link to the opengl libraries at compile
#			time. The binaries will dlopen() the necessary libs
#		yes =	link to the opengl libraries at compile time
#
# OPT_EXTRA	yes  =  Some extra optimization flags will be added (default)
#		no   =	No extra optimizations will be made
#
# USE_X86_ASM	yes  =  Use x86 assembly on ia32 machines which is faster.
#			Disable this manually for any other cpu.
#		no   =	Do not use intel assembly even on ix86.
#
# To build static binaries: You need to compile SDL_mixer with minimum features
# required ONLY for MIDI. Besides, you may need to change some hard-coded paths
# and X linkage flags (see the LDSTATIC setting)
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
USE_MIDI=yes
USE_ALSA=yes
USE_SDLCD=no
OPT_EXTRA=yes
LINK_GL_LIBS=no
USE_X86_ASM=yes

# build environment sanity checks
ifeq "$(USE_X86_ASM)" "yes"
CHECK_NASM=1
endif
CHECK_SDL=1
CHECK_WINDRES=1

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq "$(TARGET_OS)" "WIN32"
H2_BINARY=h2.exe
GLH2_BINARY=glh2.exe
else
H2_BINARY=hexen2
GLH2_BINARY=glhexen2
H2_STATIC=hexen2.static
GLH2_STATIC=glhexen2.static
endif

# Compiler flags

# Overrides for the default CPUFLAGS
#CPUFLAGS:=
# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifndef DEBUG
CFLAGS := $(CPUFLAGS) -O2 -Wall -ffast-math -fexpensive-optimizations

DISABLE_UNIT_AT_A_TIME ?=$(HAVE_GCC_4_0)
# this prevents some bad compilations with unit-at-a-time mode active.
# the bug seems to appear only with gcc-4.0.x, gcc-3.x/4.1 seems fine.
ifeq "$(DISABLE_UNIT_AT_A_TIME)" "yes"
CFLAGS := $(CFLAGS) $(call check_gcc,-fno-unit-at-a-time,)
else
ifeq "$(DISABLE_UNIT_AT_A_TIME)" "1"
CFLAGS := $(CFLAGS) $(call check_gcc,-fno-unit-at-a-time,)
endif
endif

ifeq "$(OPT_EXTRA)" "yes"
# Note: re-check these flags for non-ia32 machines
CFLAGS := $(CFLAGS) $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,-malign-loops=2 -malign-jumps=2 -malign-functions=2)
CFLAGS := $(CFLAGS) -fomit-frame-pointer
endif
endif
# end of compiler flags


# whether to use x86 assembly
ifeq "$(USE_X86_ASM)" "yes"
CFLAGS := $(CFLAGS) -DUSE_INTEL_ASM
endif

# Other build flags
EXT_FLAGS:=$(ARCHFLAGS)

ifeq "$(TARGET_OS)" "WIN32"
# Main win32 specific includes and flags
INCLUDES:= -I. -I$(UHEXEN2_TOP)/w32stuff/dxsdk/inc -I$(MINGWDIR)/include
LDFLAGS := -L$(MINGWDIR)/lib -lwinmm -lwsock32 -mwindows
else
# Main unix specific includes and flags
INCLUDES:= -I$(X11_PATH)/include
LDFLAGS := $(LIBSOCKET)
EXT_FLAGS+= -DPLATFORM_UNIX
endif

GL_DEFS := -DGLQUAKE
GLFLAGS :=

ifeq "$(LINK_GL_LIBS)" "no"
GL_DEFS+= -DGL_DLSYM
else
GLFLAGS+= $(GL_LINK)
endif

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates come extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG_BUILD
endif

ifeq "$(TARGET_OS)" "UNIX"
# Start Unix-specific extra flags/settings

ifeq "$(USE_ALSA)" "no"
EXT_FLAGS+= -DNO_ALSA
endif

ifeq "$(USE_SDLCD)" "yes"
EXT_FLAGS+= -DWITH_SDLCD
endif

ifeq "$(USE_MIDI)" "yes"
SDLLINK+= -lSDL_mixer
LDSTATIC= -Wl,-Bstatic -L/usr/lib -lSDL_mixer -lSDL -L$(X11_PATH)/lib -lX11 -lXext -ldl -L$(X11_PATH)/lib -lXxf86dga -lXxf86vm -lXv -L/usr/lib -lm -Wl,-Bdynamic -lpthread
else
EXT_FLAGS+= -DNO_MIDIMUSIC
LDSTATIC= -Wl,-Bstatic -L/usr/lib -lSDL -L$(X11_PATH)/lib -lX11 -lXext -ldl -L$(X11_PATH)/lib -lXxf86dga -lXxf86vm -lXv -L/usr/lib -lm -Wl,-Bdynamic -lpthread
endif
# End of Unix specific extras
endif

ifeq "$(TARGET_OS)" "WIN32"
# Start Win32-specific extra flags/settings

ifeq "$(WITH_SPLASHES)" "no"
EXT_FLAGS+= -DNO_SPLASHES
else
# gl version needs linking to comctl32.dll
# because of that progress bar thingy
GLFLAGS+=-lcomctl32
endif
# End of Win32 specific extras
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(INCLUDES) $(SDLFLAGS) -o $@ $<
%.o: %.asm
	$(NASM) $(NASMFLAGS) -o $@ $<
%.o: %.rc
	$(WINDRES) $(EXT_FLAGS) --include=$(MINGWDIR)/include --output=coff -o $@ $<

# Objects
# Platform specific object settings
SOFT_SDL_VID = vid_sdl.o
SOFT_WIN32VID = win_stuff/vid_win.o
GL_SDL_VID = gl_vidsdl.o \
	fx_gamma.o
GL_WIN32VID = win_stuff/gl_vidnt.o
INPUT_SDL = in_sdl.o
INPUT_WIN32 = win_stuff/in_win.o
NET_UNIX = net_bsd.o \
	net_udp.o
NET_WIN32 = win_stuff/net_win.o \
	win_stuff/net_wins.o \
	win_stuff/net_wipx.o
SND_UNIX = snd_oss.o \
	snd_sdl.o \
	snd_alsa.o \
	snd_sun.o \
	cd_null.o \
	cd_sdl.o \
	cd_bsd.o \
	cd_linux.o \
	midi_sdl.o
SND_WIN32 = win_stuff/snd_win.o \
	win_stuff/cd_win.o \
	win_stuff/midi.o \
	win_stuff/mstrconv.o
SYS_UNIX = sys_unix.o
SYS_WIN32 = win_stuff/conproc.o \
	win_stuff/sys_win.o \
	win_stuff/win32res.o

ifeq "$(TARGET_OS)" "WIN32"
SYSOBJ_NET = $(NET_WIN32)
SYSOBJ_SND = $(SND_WIN32)
SYSOBJ_SYS = $(SYS_WIN32)
SYSOBJ_INPUT = $(INPUT_WIN32)
SYSOBJ_GL_VID = $(GL_WIN32VID)
SYSOBJ_SOFT_VID = $(SOFT_WIN32VID)
else
SYSOBJ_NET = $(NET_UNIX)
SYSOBJ_SND = $(SND_UNIX)
SYSOBJ_SYS = $(SYS_UNIX)
SYSOBJ_INPUT = $(INPUT_SDL)
SYSOBJ_GL_VID = $(GL_SDL_VID)
SYSOBJ_SOFT_VID = $(SOFT_SDL_VID)
endif

# Final list of objects
SOFTOBJS = d_edge.o \
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
	draw.o \
	model.o \
	r_aclip.o \
	r_alias.o \
	r_bsp.o \
	r_draw.o \
	r_edge.o \
	r_efrag.o \
	r_light.o \
	r_main.o \
	r_misc.o \
	r_sky.o \
	r_sprite.o \
	r_surf.o \
	r_vars.o \
	screen.o \
	nonintel.o \
	$(SYSOBJ_SOFT_VID)

GLOBJS = gl_draw.o \
	gl_mesh.o \
	gl_model.o \
	gl_refrag.o \
	gl_rlight.o \
	gl_rmain.o \
	gl_rmisc.o \
	gl_rsurf.o \
	gl_screen.o \
	gl_test.o \
	gl_warp.o \
	$(SYSOBJ_GL_VID)

COMMONOBJS = chase.o \
	cl_demo.o \
	cl_effect.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_tent.o \
	cl_cmd.o \
	cmd.o \
	common.o \
	console.o \
	crc.o \
	cvar.o \
	host.o \
	host_cmd.o \
	strings.o \
	$(SYSOBJ_INPUT) \
	keys.o \
	mathlib.o \
	menu.o \
	$(SYSOBJ_NET) \
	net_dgrm.o \
	net_loop.o \
	net_main.o \
	net_vcr.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	r_part.o \
	sbar.o \
	snd_sys.o \
	snd_dma.o \
	snd_mix.o \
	snd_mem.o \
	$(SYSOBJ_SND) \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_effect.o \
	sv_user.o \
	view.o \
	wad.o \
	world.o \
	zone.o \
	$(SYSOBJ_SYS)

# Intel asm objects
ifeq "$(USE_X86_ASM)" "yes"
COMMON_ASM = math.o \
	snd_mixa.o \
	worlda.o \
	sys_wina.o

SOFT_ASM = d_draw.o \
	d_draw16.o \
	d_draw16t.o \
	d_parta.o \
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


else

SOFT_ASM = 
COMMON_ASM = 
endif

# Targets
.PHONY: help clean cleaner

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
ifneq "$(TARGET_OS)" "WIN32"
	@echo "===== Static binaries: ====="
	@echo "* $(MAKE) h2_static   : hexen2 binary, software renderer"
	@echo "* $(MAKE) glh2_static : hexen2 binary, opengl renderer"
	@echo
endif

hexen2: h2
glhexen2: glh2

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

h2: EXT_FLAGS+=$(MGL_FLAG)
h2: INCLUDES+=$(MGL_INCL)
h2: sanity $(SOFTOBJS) $(SOFT_ASM) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(H2_BINARY) $(SOFTOBJS) $(SOFT_ASM) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(SDLLINK) $(MGL_LINK)

glh2: EXT_FLAGS+= $(GL_DEFS)
glh2: sanity $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(GLH2_BINARY) $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(GLFLAGS) $(SDLLINK)

# static builds are not tested on win32
ifneq "$(TARGET_OS)" "WIN32"

h2_static : sanity $(SOFTOBJS) $(SOFT_ASM) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(H2_STATIC) $(SOFTOBJS) $(SOFT_ASM) $(COMMON_ASM) $(COMMONOBJS) $(LDSTATIC)

glh2_static : EXT_FLAGS+= $(GL_DEFS)
glh2_static : sanity $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(GLH2_STATIC) $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDSTATIC) $(GLFLAGS)

# End of static binary targets
endif

clean:
	rm -f *.o win_stuff/*.o core .tmp *.tmp

cleaner:
	rm -f *.o win_stuff/*.o core .tmp *.tmp \
		$(H2_BINARY) $(GLH2_BINARY) $(H2_STATIC) $(GLH2_STATIC)

