
# Quick'n'Dirty Makefile for Hexen2 Launcher

#  to compile for gtk-1.2, type:  make
#  to compile for gtk-2.0, type:  make GTK2=yes

#  if building for the demo version, run with
#  make DEMOBUILD=yes

#  if you want debug symbols compiled in ( -g CFLAG ), run with
#  make DEBUG=1

prefix = /usr
#prefix = /usr/local

PACKAGE = h2launcher
ifeq "${DEMOBUILD}" "yes"
TMPNAME = h2demo
else
TMPNAME = h2launcher
endif
ifeq "${GTK2}" "yes"
H2LAUNCH = ${TMPNAME}.gtk2
GTK_CFLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`
else
H2LAUNCH = ${TMPNAME}
GTK_CFLAGS = `gtk-config --cflags`
GTK_LIBS = `gtk-config --libs`
endif
VERSION = 0.3.0

PROGRAMS = $(H2LAUNCH)

#ORIG_SRC= support.c support.h
SOURCES = main.c ${ORIG_SRC} interface.c interface.h \
	callbacks.c callbacks.h launch_bin.c launch_bin.h \
	config_file.c config_file.h


#ORIG_OBJ= support.o
OBJECTS = main.o ${ORIG_OBJ} interface.o callbacks.o launch_bin.o \
	config_file.o

CFLAGS = -Wall -O2 -march=i386 -mcpu=i686 -I. ${GTK_CFLAGS}
ifeq "${DEMOBUILD}" "yes"
CFLAGS+=-DDEMOBUILD
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
