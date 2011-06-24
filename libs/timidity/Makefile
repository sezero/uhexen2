# Quick GNU Makefile for building a static timidity library using GCC.
# $Id$
#
# if building a debug version:  make DEBUG=yes

UHEXEN2_TOP:=../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs

CC ?= gcc
AR ?= ar
RANLIB ?= ranlib

CFLAGS += -g -Wall
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1
endif
#CFLAGS+= -DTIMIDITY_DEBUG
INCLUDES= -I. -I$(LIBS_DIR)/common
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
#%.o: $(LIBS_DIR)/common/%.c
#	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

libtimidity.a: $(OBJECTS)
	$(AR) cru libtimidity.a $(OBJECTS)
	$(RANLIB) libtimidity.a

clean:
	rm -f *.o

cleaner:
	rm -f *.o $(TARGETS)

