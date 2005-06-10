# Quick'n'Dirty Makefile for Hexen2 Launcher

#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes

#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

#  if you want debug symbols compiled in ( -g ), run with
#  make DEBUG=1

prefix = /usr
#prefix = /usr/local

PACKAGE = h2launcher
ifdef DEMO
TMPNAME = h2demo
else
TMPNAME = h2launcher
endif
ifdef GTK1
H2LAUNCH = ${TMPNAME}.gtk1
GTK_CFLAGS = `gtk-config --cflags`
GTK_LIBS = `gtk-config --libs`
else
#H2LAUNCH = ${TMPNAME}.gtk2
H2LAUNCH = ${TMPNAME}
GTK_CFLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`
endif

VERSION = 0.6.0
HOT_VERSION = 1.2.5

PROGRAMS = $(H2LAUNCH)

SOURCES1= main.c com_sys.c config_file.c launch_bin.c interface.c \
	callbacks.c support.c
SOURCES2= com_sys.h launcher_defs.h config_file.h launch_bin.h \
	interface.h widget_defs.h callbacks.h support.h
SOURCES = ${SOURCES1} ${SOURCES2}

# activate the line below if you activate NLS support
#SUPPORT_OBJ= support.o
OBJECTS = main.o com_sys.o config_file.o launch_bin.o \
	interface.o callbacks.o ${SUPPORT_OBJ}

CFLAGS = -Wall -O2 -march=i586 -I. ${GTK_CFLAGS} -DHOTL_VER=\"${VERSION}\" -DHOT_VER=\"${HOT_VERSION}\"
ifdef DEBUG
CFLAGS+=-g
endif
ifdef DEMO
CFLAGS+=-DDEMOBUILD
endif
ifdef GTK1
CFLAGS+=-DWITH_GTK1
endif
LDFLAGS =

ifndef CC
CC = gcc
endif

COMPILE = $(CC) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) -o $@ $(LDFLAGS) ${GTK_LIBS}

.SUFFIXES: .c .o

.o:
	$(COMPILE) -c $<

$(H2LAUNCH): $(OBJECTS)
	@rm -f $(H2LAUNCH)
	$(LINK) $(LDFLAGS) $(OBJECTS)

all: $(PROGRAMS)

clean:
	-rm -f *.o core *.core

cleaner:
	-rm -f *.o core *.core $(PROGRAMS)

