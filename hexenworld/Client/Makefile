# PLEASE edit below to select what you would like to build
BUILDGL=yes
# MIDI doesn't work at this time for HexenWorld client
USEMIDI=no

# You shouldn't need to edit anything below here.

ifeq "$(BUILDGL)" "yes"
BINARY=glhwcl.dynamic
BINARY_STATIC=glhwcl
else
BINARY=hwcl.dynamic
binary_STATIC=hwcl
endif

CC = gcc
LINKER = gcc
CFLAGS = -fomit-frame-pointer -ffast-math -fexpensive-optimizations -malign-loops=2 -malign-jumps=2 -malign-functions=2 -g -O2 -DUSE_INTEL_ASM -DPLATFORM_UNIX -DRJNET -DQUAKE2RJ `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`
GLFLAGS = `sdl-config --libs` -L/usr/X11R6/lib -lGL -lGLU

LDSTATIC = -Wl,-Bstatic -L/usr/lib -lSDL_mixer -lSDL -L/usr/X11R6/lib -lX11 -lXext -ldl -L/usr/X11R6/lib -lXxf86dga -lXxf86vm -lXv -L/usr/lib/ -lsmpeg -lvorbisfile -lvorbis -logg -lm -Wl,-Bdynamic -lpthread
GLSTATIC = -Wl,-Bstatic -L/usr/lib -lSDL_mixer -lSDL -L/usr/X11R6/lib -lX11 -lXext -ldl -lGLU -L/usr/X11R6/lib -lXxf86dga -lXxf86vm -lXv -L/usr/lib/ -lsmpeg -lvorbisfile -lvorbis -logg -lm -Wl,-Bdynamic -lpthread -lGL

ifeq "$(BUILDGL)" "yes"
CFLAGS+= -DGLQUAKE
endif

ifeq "$(USEMIDI)" "yes"
CFLAGS+= -DUSE_MIDI
GLFLAGS+= -lSDL_mixer
LDFLAGS+= -lSDL_mixer
endif

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
%.o: %.S
	$(CC) -c -o $@ $< -save-temps $(CFLAGS)
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

COMMONOBJS = cd_linux.o \
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
	midi.o \
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
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(SOFTOBJS) $(COMMONOBJS)
else
$(BINARY) : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(GLFLAGS) $(GLOBJS) $(COMMONOBJS)	
endif

ifeq "$(BUILDGL)" "no"
$(BINARY_STATIC) : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(SOFTOBJS) $(COMMONOBJS) $(LDSTATIC)
else
$(BINARY_STATIC) : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(GLOBJS) $(COMMONOBJS) $(GLSTATIC)
endif

ifeq "$(BUILDGL)" "no"
dynamic : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(SOFTOBJS) $(COMMONOBJS)
else
dynamic : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(GLFLAGS) $(GLOBJS) $(COMMONOBJS)	
endif

ifeq "$(BUILDGL)" "no"
static : $(SOFTOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(SOFTOBJS) $(COMMONOBJS) $(LDSTATIC)
else
static : $(GLOBJS) $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(GLOBJS) $(COMMONOBJS) $(GLSTATIC)
endif

clean:
	rm -f *.o core

