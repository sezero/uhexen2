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

# Targets
.PHONY: clean distclean

all: $(TARGETS)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
#%.o: $(LIBS_DIR)/common/%.c
#	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# object dependencies:
common.o: common.c common.h options.h timidity.h timidity_internal.h
instrum.o: instrum.c common.h instrum.h instrum_dls.h options.h resample.h tables.h timidity.h timidity_internal.h
instrum_dls.o: instrum_dls.c common.h dls1.h dls2.h instrum.h instrum_dls.h options.h tables.h timidity.h timidity_internal.h
mix.o: mix.c instrum.h mix.h options.h output.h playmidi.h resample.h tables.h timidity.h timidity_internal.h
output.o: output.c options.h output.h timidity.h timidity_internal.h
playmidi.o: playmidi.c instrum.h mix.h options.h output.h playmidi.h tables.h timidity.h timidity_internal.h
readmidi.o: readmidi.c common.h instrum.h options.h playmidi.h timidity.h timidity_internal.h
resample.o: resample.c common.h instrum.h options.h playmidi.h resample.h tables.h timidity.h timidity_internal.h
stream.o: stream.c common.h timidity.h timidity_internal.h
tables.o: tables.c tables.h timidity.h
timidity.o: timidity.c common.h instrum.h options.h output.h playmidi.h readmidi.h tables.h timidity.h timidity_internal.h

# rule for target:
libtimidity.a: $(OBJECTS)
	$(AR) cru libtimidity.a $(OBJECTS)
	$(RANLIB) libtimidity.a

clean:
	rm -f *.o libtimidity.a

distclean: clean

