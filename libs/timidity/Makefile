# Quick GNU Makefile for building a static timidity library using GCC.
# $Id$

CC ?= gcc
AR ?= ar
RANLIB ?= ranlib

INCLUDES = -I.
CFLAGS = -Wall -O2
#CFLAGS+= -DTIMIDITY_DEBUG
TARGETS:= libtimidity.a

OBJECTS = common.o \
	instrum.o \
	instrum_dls.o \
	mix.o \
	output.o \
	playmidi.o \
	readmidi.o \
	resample.o \
	stream.o \
	tables.o \
	timidity.o

all: $(TARGETS)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

libtimidity.a: $(OBJECTS)
	$(AR) cru libtimidity.a $(OBJECTS)
	$(RANLIB) libtimidity.a

clean:
	rm -f *.o

cleaner:
	rm -f *.o $(TARGETS)

