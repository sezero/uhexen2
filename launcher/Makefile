# Quick'n'Dirty Makefile for Hexen2 Launcher

#  to compile for gtk-1.2, type:  make
#  to compile for gtk-2.0, type:  make GTK2=yes

#  if building for the demo version, run with
#  make DEMO=yes [other stuff]

#  if you want debug symbols compiled in ( -g CFLAG ), run with
#  make DEBUG=1

prefix = /usr
#prefix = /usr/local

PACKAGE = h2launcher
ifeq "${DEMO}" "yes"
TMPNAME = h2demo
else
TMPNAME = h2launcher
endif
ifeq "${GTK2}" "yes"
H2LAUNCH = ${TMPNAME}.gtk2
GTK_CFLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`
else
H2LAUNCH = ${TMPNAME}.gtk1
GTK_CFLAGS = `gtk-config --cflags`
GTK_LIBS = `gtk-config --libs`
endif

VERSION = 0.4.8

PROGRAMS = $(H2LAUNCH)

SOURCES1= main.c com_sys.c config_file.c launch_bin.c interface.c \
	callbacks.c support.c
SOURCES2= com_sys.h launcher_defs.h config_file.h launch_bin.h \
	interface.h widget_defs.h callbacks.h support.h
SOURCES = ${SOURCES1} ${SOURCES2}

#SUPPORT_OBJ= support.o
OBJECTS = main.o com_sys.o config_file.o launch_bin.o \
	interface.o callbacks.o ${SUPPORT_OBJ}

CFLAGS = -Wall -O2 -march=i386 -mcpu=i686 -I. ${GTK_CFLAGS} -DHOTL_VER=\"${VERSION}\"
ifeq "${DEMO}" "yes"
CFLAGS+=-DDEMOBUILD
endif
ifeq "${GTK2}" "yes"
CFLAGS+=-DHAVE_GTK2
endif
ifdef DEBUG
CFLAGS+=-g
endif
LDFLAGS =

ifndef CC
CC = gcc
endif

COMPILE = $(CC) $(CFLAGS)
CCLD = $(CC)
#LINK = $(CCLD) $(CFLAGS) $(LDFLAGS) ${GTK_LIBS} -o $@
LINK = $(CCLD) -o $@ $(LDFLAGS) ${GTK_LIBS}

.SUFFIXES: .S .c .o .s

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

$(H2LAUNCH): $(OBJECTS)
	@rm -f $(H2LAUNCH)
	$(LINK) $(LDFLAGS) $(OBJECTS)

all: $(PROGRAMS)

clean:
	-rm -f *.o core *.core

cleaner:
	-rm -f *.o core *.core $(PROGRAMS)

