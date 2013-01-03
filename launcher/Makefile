# GNU Makefile for uhexen2 gtk-launcher app using GCC.
# $Id$
#
# GTK-2.x will be used default.
# To build against GTK-1.2:	make GTK1=yes
# To build against GTK-3.x:	make GTK3=yes
#
# To build for demo version:	make DEMO=yes [other stuff]
#
# To build a debug version:	make DEBUG=yes
#

# main uhexen2 relative path
UHEXEN2_TOP:=..
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

HOST_OS:=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

CC ?= gcc
ifeq ($(origin CC),default)
# refuse the stupid 'cc' default of make
# which not necessarily exist everywhere
CC  = gcc
endif
AR ?= ar
RANLIB ?= ranlib

LINKER= $(CC)

BINARY:= h2launcher

XDELTA_DIR:= $(LIBS_DIR)/xdelta3
XDFLAGS:= -DXD3_DEBUG=0
XDFLAGS+= -I$(XDELTA_DIR) -I$(UHEXEN2_SHARED)
# when XD3_USE_LARGEFILE64 is not defined, xdelta3 defaults
# to 64 bit xoff_t.
# define XD3_USE_LARGEFILE64 as 0 or 1.
XDFLAGS+= -DXD3_USE_LARGEFILE64=1
# make xdelta3 to use stdio for file i/o:
XDFLAGS+= -DXD3_STDIO=1

ifeq ($(GTK1),yes)
GTK_CFLAGS:= $(shell gtk-config --cflags)
GTK_LIBS:= $(shell gtk-config --libs)
else
ifeq ($(GTK3),yes)
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-3.0)
else
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-2.0)
endif
endif

# pthread flags: do note that these are different across
# different platforms!!!
PTHREAD_CFLAGS=-D_REENTRANT
PTHREAD_LIBS=-lpthread

CPUFLAGS=

CFLAGS += -g -Wall
CFLAGS += $(CPUFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=1
else
CFLAGS += -O2 -DNDEBUG=1
endif
XDFLAGS +=$(CFLAGS)

CFLAGS += -I. -I$(UHEXEN2_SHARED)
CFLAGS += $(PTHREAD_CFLAGS)
LDFLAGS+= $(PTHREAD_LIBS)

ifdef DEMO
CFLAGS += -DDEMOBUILD
else
CFLAGS += -I$(XDELTA_DIR)
endif

# Targets
all: $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Objects
GUI_OBJS     := gtk_ui.o
LAUNCHER_OBJS:= crc.o q_endian.o games.o config_file.o launch_bin.o
LAUNCHER_MAIN:= main.o
ifndef DEMO
XPATCH_OBJS  := apply_patch.o xdelta3.o
endif
ALL_OBJECTS  := $(LAUNCHER_OBJS) $(LAUNCHER_MAIN) $(XPATCH_OBJS) $(GUI_OBJS)

# Targets
.PHONY: clean distclean

$(BINARY): $(ALL_OBJECTS)
	$(LINKER) $(ALL_OBJECTS) $(GTK_LIBS) $(LDFLAGS) -o $@

XD3DEPS:= xdelta3-decode.h xdelta3-list.h xdelta3-main.h xdelta3-blkcache.h \
	xdelta3.c xdelta3.h
XD3DEPS:= $(patsubst %,$(XDELTA_DIR)/%,$(XD3DEPS))

xdelta3.o: $(XD3DEPS)
	$(CC) $(XDFLAGS) -c $(XDELTA_DIR)/xdelta3.c -o xdelta3.o

gtk_ui.o: gtk_ui.c gtk_ui.h launcher_defs.h launcher_ui.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c gtk_ui.c -o gtk_ui.o
support.o: support.c support.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c support.c -o support.o

clean:
	-rm -f *.o *.o core
distclean: clean
	-rm -f $(BINARY)

