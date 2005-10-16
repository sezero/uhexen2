
#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

BINARY=hwsv
BINARY_STATIC=hwsv.static

ifndef CC
CC = gcc
endif
LINKER = $(CC)

ifdef DEBUG
CFLAGS = -DDEBUG_BUILD -g -Wall -DPLATFORM_UNIX
else
CFLAGS = -O2 -march=i586 -Wall -ffast-math -fexpensive-optimizations -DPLATFORM_UNIX
endif

GAMEFLAGS = -DH2W -DSERVERONLY
SVINCS = -I../Client -I.
SDLFLAG = `sdl-config --cflags`
SDLLINK = `sdl-config --libs`

ifdef DEMO
GAMEFLAGS+= -DDEMOBUILD
endif
LDFLAGS = -lm -lpthread
LDSTATIC = -lm -lpthread -static

%.o: %.c
	$(CC) -c $(SVINCS) $(CFLAGS) $(GAMEFLAGS) -o $@ $<
cl_objs/%.o: ../Client/%.c
	$(CC) -c $(SVINCS) $(CFLAGS) $(GAMEFLAGS) -o $@ $<

COMMONOBJS = model.o \
	cl_objs/cmd.o \
	cl_objs/common.o \
	cl_objs/crc.o \
	cl_objs/cvar.o \
	cl_objs/mathlib.o \
	cl_objs/net_chan.o \
	cl_objs/net_wins.o \
	cl_objs/pmove.o \
	cl_objs/pmovetst.o \
	cl_objs/strings.o \
	cl_objs/zone.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	sv_ccmds.o \
	sv_effect.o \
	sv_ents.o \
	sv_init.o \
	sv_main.o \
	sv_move.o \
	sv_phys.o \
	sv_send.o \
	sv_user.o \
	sys_unix.o \
	world.o

all: $(BINARY)
static: $(BINARY_STATIC)

$(BINARY) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(COMMONOBJS) $(LDFLAGS)

$(BINARY_STATIC) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(COMMONOBJS) $(LDSTATIC)

clean:
	rm -f $(COMMONOBJS) core

