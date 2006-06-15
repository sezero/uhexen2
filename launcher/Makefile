# Quick'n'Dirty Makefile for Hexen2 Launcher

#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes

#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

#  if you want debug symbols compiled in ( -g ), run with
#  make DEBUG=1

ifdef DEMO
TMPNAME = h2demo
else
TMPNAME = h2launcher
endif

HOST_OS :=$(shell uname)

ifdef GTK1
H2LAUNCH = $(TMPNAME).gtk1
GTK_CFLAGS := $(shell gtk-config --cflags)
GTK_LIBS := $(shell gtk-config --libs)
else

#H2LAUNCH = $(TMPNAME).gtk2
H2LAUNCH = $(TMPNAME)
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-2.0)
endif

PROGRAMS = $(H2LAUNCH)

# activate the line below if you activate NLS support
#SUPPORT_OBJ= support.o
OBJECTS = main.o config_file.o launch_bin.o \
	interface.o callbacks.o $(SUPPORT_OBJ)

ifdef DEBUG
CFLAGS := -Wall -g -DDEBUG_BUILD
else
CFLAGS := -Wall -O2
endif
CFLAGS := $(CFLAGS) -I. $(GTK_CFLAGS)

ifdef DEMO
CFLAGS := $(CFLAGS) -DDEMOBUILD
endif
ifdef GTK1
CFLAGS := $(CFLAGS) -DWITH_GTK1
endif
ifeq "$(HOST_OS)" "SunOS"
CFLAGS := $(CFLAGS) -DSUNOS
endif
LDFLAGS =

CC ?= gcc

COMPILE = $(CC) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) -o $@ $(LDFLAGS) $(GTK_LIBS)

.SUFFIXES: .c .o

.o:
	$(COMPILE) -c $<

$(H2LAUNCH): $(OBJECTS)
	@rm -f $(H2LAUNCH)
	$(LINK) $(OBJECTS) $(LDFLAGS)

all: $(PROGRAMS)

clean:
	-rm -f *.o core

cleaner:
	-rm -f *.o core $(PROGRAMS)

