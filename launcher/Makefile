
# Quick'n'Dirty Makefile for Hexen2 Launcher

prefix = /usr
#prefix = /usr/local

GTK_CONFIG = ${prefix}/bin/gtk-config
GTK_CFLAGS = `gtk-config --cflags`
GTK_LIBS = `gtk-config --libs`
PACKAGE = h2launcher
VERSION = 0.2c9

#ORIG_SRC= support.c support.h
SOURCES = main.c ${ORIG_SRC} interface.c interface.h \
	callbacks.c callbacks.h launch_bin.c launch_bin.h \
	config_file.c config_file.h

PROGRAMS = h2launcher

#ORIG_OBJ= support.o
OBJECTS = main.o ${ORIG_OBJ} interface.o callbacks.o launch_bin.o \
	config_file.o

CFLAGS = -O2 -Wall -I. ${GTK_CFLAGS}
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

h2launcher: $(OBJECTS)
	@rm -f h2launcher
	$(LINK) $(LDFLAGS) $(OBJECTS)

all: $(PROGRAMS)

clean:
	-rm -f *.o core *.core
	-test -z "$(PROGRAMS)" || rm -f $(PROGRAMS)
