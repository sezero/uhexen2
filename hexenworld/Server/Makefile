# To build for the demo version:	make DEMO=1 [other stuff]
# if building a debug version :		make DEBUG=1 [other stuff]

# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../..

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq "$(TARGET_OS)" "WIN32"
BINARY=hwsv.exe
else
BINARY=hwsv
BINARY_STATIC=hwsv.static
endif

# Compiler flags
ifndef DEBUG
CFLAGS := -O2 -march=i586 -Wall -ffast-math -fexpensive-optimizations

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
endif

# Other build flags
EXT_FLAGS:= -DH2W -DSERVERONLY
INCLUDES := -I../Client -I.

ifeq "$(TARGET_OS)" "WIN32"
INCLUDES += -I$(MINGWDIR)/include
LDFLAGS  := -L$(MINGWDIR)/lib -lm -lwsock32 -mconsole
else
EXT_FLAGS+= -DPLATFORM_UNIX
LDFLAGS  := -lm -lpthread
LDSTATIC := -lm -lpthread -static
endif

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates come extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG_BUILD
endif


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(EXT_FLAGS) -o $@ $<
cl_objs/%.o: ../Client/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(EXT_FLAGS) -o $@ $<

# Objects
# Platform specific object settings
SYS_UNIX = sys_unix.o
SYS_WIN32 = win_stuff/sys_win.o

ifeq "$(TARGET_OS)" "WIN32"
SYSOBJ_SYS = $(SYS_WIN32)
else
SYSOBJ_SYS = $(SYS_UNIX)
endif

# Final list of objects
COMMONOBJS = model.o \
	cl_objs/cmd.o \
	cl_objs/common.o \
	cl_objs/crc.o \
	cl_objs/cvar.o \
	cl_objs/strings.o \
	cl_objs/mathlib.o \
	cl_objs/huffman.o \
	cl_objs/net_wins.o \
	cl_objs/net_chan.o \
	cl_objs/pmove.o \
	cl_objs/pmovetst.o \
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
	world.o \
	$(SYSOBJ_SYS)


# Targets
default: $(BINARY)
all: default

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(BINARY) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY) $(COMMONOBJS) $(LDFLAGS)

# static builds are not tested on win32
ifneq "$(TARGET_OS)" "WIN32"

static: $(BINARY_STATIC)

$(BINARY_STATIC) : $(COMMONOBJS)
	$(LINKER) -o $(BINARY_STATIC) $(COMMONOBJS) $(LDSTATIC)

# End of static binary targets
endif

clean:
	rm -f *.o cl_objs/*.o win_stuff/*.o core .tmp *.tmp

