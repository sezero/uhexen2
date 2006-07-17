# Quick'n'Dirty Makefile for Hexen2 Launcher

#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes

#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

#  if you want debug symbols compiled in ( -g ), run with
#  make DEBUG=1

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
ifndef CC_OVERRIDE
CC := $(CC_DEFAULT)
endif

LINKER:= $(CC)

ifdef DEMO
TMPNAME:= h2demo
else
TMPNAME:= h2launcher
endif

ifdef GTK1
H2LAUNCH:= $(TMPNAME).gtk1
GTK_CFLAGS:= $(shell gtk-config --cflags)
GTK_LIBS:= $(shell gtk-config --libs)
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

CFLAGS := $(CFLAGS) $(GTK_CFLAGS) -idirafter .

ifdef DEMO
CFLAGS := $(CFLAGS) -DDEMOBUILD
endif
ifdef GTK1
CFLAGS := $(CFLAGS) -DWITH_GTK1
endif
ifeq "$(HOST_OS)" "SunOS"
CFLAGS := $(CFLAGS) -DSUNOS
endif

LDFLAGS:=$(GTK_LIBS)

# activate the line below if you activate NLS support
#SUPPORT_OBJ= support.o
OBJECTS = main.o config_file.o launch_bin.o \
	interface.o callbacks.o $(SUPPORT_OBJ)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(H2LAUNCH): $(OBJECTS)
	@rm -f $(H2LAUNCH)
	$(LINKER) -o $(H2LAUNCH) $(OBJECTS) $(LDFLAGS)

all: $(H2LAUNCH)

clean:
	-rm -f *.o core

cleaner:
	-rm -f *.o core $(H2LAUNCH)

