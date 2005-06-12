################ PLEASE edit below to select what you would like to build

# BUILDGL :	yes = OpenGL support, no = software mode
BUILDGL=yes

# USEMIDI :	MIDI needs SDL_mixer (built with Timidity support)
#		and Timidity patches
USEMIDI=yes

# USEALSA :	no   =	Don't include alsa support.
#		yes  =	Include ALSA sound support. Requires alsa-lib
#			and alsa-kernel modules >= 1.0.1
#		On platforms other than linux, it will already be disabled
USEALSA=yes

# USE_SDLCD:	no   =	Don't use SDL cdaudio (default); platform spesific
#			cdrom code will be used.  The only problem with
#			SDL_cdrom is that it lacks proper volume controls.
#		yes  =	Use SDL for cdaudio instead of platform spesific code
USE_SDLCD=no

# To build for the demo version, run with make DEMO=yes [other stuff]


################ You shouldn't need to edit anything below here.

ifeq "$(BUILDGL)" "yes"
BINARY=glhwcl
BINARY_STATIC=glhwcl.static
else
BINARY=hwcl
binary_STATIC=hwcl.static
endif

ifndef CC
CC = gcc
endif
LINKER = $(CC)
CFLAGS = -O2 -march=i586 \
	-fomit-frame-pointer -ffast-math -fexpensive-optimizations \
	-falign-loops=2 -falign-jumps=2 -falign-functions=2
ifdef DEBUG
CFLAGS+= -DDEBUG -g -Wall
endif
GAMEFLAGS = -DPLATFORM_UNIX -DH2W
SDLFLAGS = `sdl-config --cflags`
SDLLINK = `sdl-config --libs`
LDFLAGS =
GLFLAGS = -L/usr/X11R6/lib

LDSTATIC = -Wl,-Bstatic -L/usr/lib -lSDL_mixer -lSDL -L/usr/X11R6/lib -lX11 -lXext -ldl -L/usr/X11R6/lib -lXxf86dga -lXxf86vm -lXv -L/usr/lib/ -lsmpeg -lvorbisfile -lvorbis -logg -lm -Wl,-Bdynamic -lpthread

ifeq "$(BUILDGL)" "yes"
GAMEFLAGS+= -DGLQUAKE
endif

ifeq "$(USEMIDI)" "yes"
GAMEFLAGS+= -DUSE_MIDI
SDLLINK+= -lSDL_mixer
endif

ifeq "$(USEALSA)" "no"
GAMEFLAGS+= -DNO_ALSA
endif

ifeq "$(USE_SDLCD)" "yes"
GAMEFLAGS+= -DWITH_SDLCD
endif

ifdef DEMO
GAMEFLAGS+= -DDEMOBUILD
endif

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(SDLFLAGS) $(GAMEFLAGS)
%.o: %.asm
	nasm -f elf -o $@ $<

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
	vid_sdl.o \
	d_draw.o \
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
	surf16.o \
	surf8.o

GLOBJS = gl_draw.o \
	gl_mesh.o \
	gl_model.o \
	gl_ngraph.o \
	gl_refrag.o \
	gl_rlight.o \
	gl_rmain.o \
	gl_rmisc.o \
	gl_rsurf.o \
	gl_screen.o \
	gl_test.o \
	gl_warp.o \
	gl_vidsdl.o

COMMONOBJS = cd_unix.o \
	cl_cam.o \
	cl_demo.o \
	cl_effect.o \
	cl_ents.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_pred.o \
	cl_tent.o \
	cmd.o \
	common.o \
	console.o \
	crc.o \
	cvar.o \
	in_sdl.o \
	keys.o \
	mathlib.o \
	menu.o \
	midi_sdl.o \
	net_wins.o \
	net_chan.o \
	pmove.o \
	pmovetst.o \
	r_part.o \
	sbar.o \
	skin.o \
	snd_dma.o \
	snd_mix.o \
	snd_mem.o \
	snd_oss.o \
	snd_sdl.o \
	snd_alsa.o \
	strings.o \
	sys_unix.o \
	view.o \
	wad.o \
	zone.o \
	math.o \
	snd_mixa.o \
	sys_wina.o

all: $(BINARY) 

ifeq "$(BUILDGL)" "no"
$(BINARY) : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(SDLLINK) $(SOFTOBJS) $(COMMONOBJS)
else
$(BINARY) : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(GLFLAGS) $(SDLLINK) $(GLOBJS) $(COMMONOBJS)
endif

ifeq "$(BUILDGL)" "no"
$(BINARY_STATIC) : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(SOFTOBJS) $(COMMONOBJS) $(LDSTATIC)
else
$(BINARY_STATIC) : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(GLOBJS) $(COMMONOBJS) $(LDSTATIC)
endif

ifeq "$(BUILDGL)" "no"
dynamic : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(SDLLINK) $(SOFTOBJS) $(COMMONOBJS)
else
dynamic : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(GLFLAGS) $(SDLLINK) $(GLOBJS) $(COMMONOBJS)
endif

ifeq "$(BUILDGL)" "no"
static : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(SOFTOBJS) $(COMMONOBJS) $(LDSTATIC)
else
static : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(GLOBJS) $(COMMONOBJS) $(LDSTATIC)
endif

clean:
	rm -f *.o core

