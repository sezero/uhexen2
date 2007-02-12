# Quick'n'Dirty Makefile for Hexen2 Launcher

#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes

#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

#  if you want debug symbols compiled in ( -g ), run with
#  make DEBUG=1

# main uhexen2 relative path
UHEXEN2_TOP=..

HOST_OS:=$(shell uname)

CC_DEFAULT:=gcc
ifeq ($(origin CC),environment)
CC_OVERRIDE:=1
ifeq ($(CC),)
CC_OVERRIDE:=
endif
endif
ifeq ($(origin CC),command line)
CC_OVERRIDE:=1
endif
ifeq ($(CC_OVERRIDE),)
CC := $(CC_DEFAULT)
endif

LINKER:= $(CC)

XDELTA_DIR := $(UHEXEN2_TOP)/xdelta11
XDELTA_FLAGS:=
XDELTA_LINK:= -L$(XDELTA_DIR)/.libs -lxdelta -L$(XDELTA_DIR)/libedsio/.libs -ledsio

ifdef DEMO
TMPNAME:= h2demo
else
TMPNAME:= h2launcher
endif

ifdef GTK1
H2LAUNCH:= $(TMPNAME).gtk1
GTK_CFLAGS:= $(shell gtk-config --cflags)
GTK_LIBS:= $(shell gtk-config --libs)
XDELTA_FLAGS:=-D_XD_USE_GLIB1
else
H2LAUNCH:= $(TMPNAME)
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-2.0)
endif

ifdef DEBUG
CFLAGS := -Wall -g -DDEBUG_BUILD
else
CFLAGS := -Wall -O2
endif

CFLAGS := $(CFLAGS) $(GTK_CFLAGS)

ifdef DEMO
CFLAGS := $(CFLAGS) -DDEMOBUILD
else
CFLAGS := $(CFLAGS) $(XDELTA_FLAGS) -I$(XDELTA_DIR) -I$(XDELTA_DIR)/libedsio -I./xpatch
endif
ifdef GTK1
CFLAGS := $(CFLAGS) -DWITH_GTK1
endif
ifeq ($(HOST_OS),SunOS)
CFLAGS := $(CFLAGS) -DSUNOS
endif

ifdef DEMO
LDFLAGS:=$(GTK_LIBS)
else
LDFLAGS:=-Wl,-Bstatic $(XDELTA_LINK) -Wl,-Bdynamic $(GTK_LIBS) -lz -lpthread
endif

# activate the line below if you activate NLS support
#SUPPORT_OBJ= support.o

ifdef DEMO
XPATCH_OBJ :=
else
XPATCH_OBJ := xpatch/md5.o xpatch/loki_xdelta.o xpatch/apply_patch.o
endif

OBJECTS = main.o config_file.o launch_bin.o \
	interface.o callbacks.o $(XPATCH_OBJ) $(SUPPORT_OBJ)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(H2LAUNCH): $(OBJECTS)
	@rm -f $(H2LAUNCH)
	$(LINKER) -o $(H2LAUNCH) $(OBJECTS) $(LDFLAGS)

all: $(H2LAUNCH)

clean:
	-rm -f *.o xpatch/*.o core

cleaner:
	-rm -f *.o xpatch/*.o core $(H2LAUNCH)

