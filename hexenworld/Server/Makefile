
BINARY=hwsv
BINARY_STATIC=hwsv.static
DEBUG=no

CC = gcc
LINKER = gcc
CFLAGS = -I../Client/ -I. -O2 -march=i386 -mcpu=i686 -DUSE_INTEL_ASM -DPLATFORM_UNIX -DRJNET -DQUAKE2RJ -DSERVERONLY `sdl-config --cflags`
ifeq "$(DEBUG)" "yes"
CFLAGS += -g
endif
ifeq "${DEMOBUILD}" "yes"
CFLAGS+= -DDEMOBUILD
endif
LDFLAGS = -lm -lpthread
LDSTATIC = -lm -lpthread -static

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
%.o: %.S
	$(CC) -c -o $@ $< -save-temps $(CFLAGS)
%.o: %.asm
	nasm -f elf -o $@ $<

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
	sys_win.o \
	world.o

all: $(BINARY) 

$(BINARY) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(COMMONOBJS)
	
dynamic : $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(COMMONOBJS)

$(BINARY_STATIC) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(COMMONOBJS) $(LDSTATIC)
	
static : $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(COMMONOBJS) $(LDSTATIC)

clean:
	rm -f $(COMMONOBJS) core

