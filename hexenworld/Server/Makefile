
#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

BINARY=hwsv
BINARY_STATIC=hwsv.static

AS = nasm
ifndef CC
CC = gcc
endif
LINKER = $(CC)
CFLAGS = -O2 -march=i586 -Wall -DPLATFORM_UNIX
ifdef DEBUG
CFLAGS += -DDEBUG -g
endif
ASFLAGS = -f elf
GAMEFLAGS = -DH2W -DSERVERONLY
SVINCS = -I../Client/ -I.
SDLFLAG = `sdl-config --cflags`
SDLLINK = `sdl-config --libs`

ifdef DEMO
GAMEFLAGS+= -DDEMOBUILD
endif
LDFLAGS = -lm -lpthread
LDSTATIC = -lm -lpthread -static

%.o: %.c
	$(CC) -c $(SVINCS) $(CFLAGS) $(GAMEFLAGS) -o $@ $<
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

COMMONOBJS = model.o \
	../Client/cmd.o \
	../Client/common.o \
	../Client/crc.o \
	../Client/cvar.o \
	../Client/mathlib.o \
	../Client/net_chan.o \
	../Client/net_wins.o \
	../Client/pmove.o \
	../Client/pmovetst.o \
	../Client/strings.o \
	../Client/zone.o \
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

