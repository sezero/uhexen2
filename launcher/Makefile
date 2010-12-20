# GNU Makefile for hexen2 gtk-launcher app using GCC.
# $Id$
#
#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes
#
#  if building for the demo version, run with
#  make DEMO=yes [other stuff]
#
#  if you want debug symbols compiled in ( -g ), run with
#  make DEBUG=1

# main uhexen2 relative path
UHEXEN2_TOP:=..
LIBS_DIR:=$(UHEXEN2_TOP)/libs

HOST_OS:=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

CC ?= gcc
ifeq ($(CC),)
CC := gcc
endif

AR ?= ar
RANLIB ?= ranlib

LINKER= $(CC)

XDELTA_DIR := $(LIBS_DIR)/xdelta11
XDELTA_FLAGS:=
XDELTA_LINK:= -L$(XDELTA_DIR) -lxdelta

BINARY:= h2launcher

ifdef GTK1
GTK_CFLAGS:= $(shell gtk-config --cflags)
GTK_LIBS:= $(shell gtk-config --libs)
XDELTA_FLAGS:=-D_XD_USE_GLIB1
else
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-2.0)
endif

# pthread flags: do note that these change accross
# different platforms!!!
PTHREAD_CFLAGS=-D_REENTRANT
PTHREAD_LIBS=-lpthread

ifdef DEBUG
CFLAGS += -Wall -g -DDEBUG=1 -DDEBUG_BUILD=1
else
CFLAGS += -Wall -O2 -DNDEBUG
endif

CFLAGS += $(GTK_CFLAGS) $(PTHREAD_CFLAGS) -I.
LDFLAGS+= $(GTK_LIBS) $(PTHREAD_LIBS)

ifdef DEMO
XDDEPS :=
CFLAGS += -DDEMOBUILD
else
XDDEPS := $(XDELTA_DIR)/libxdelta.a
CFLAGS += $(XDELTA_FLAGS) -I$(XDELTA_DIR) -I$(XDELTA_DIR)/libedsio -I./xpatch
LDFLAGS+= $(XDELTA_LINK) -lz
endif

# Targets
all: $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Objects
# activate the line below if you add pixmap support, etc
#SUPPORT_OBJS := support.o
GUI_OBJS     := gtk_ui.o $(SUPPORT_OBJS)
LAUNCHER_OBJS:= crc.o q_endian.o games.o config_file.o launch_bin.o
LAUNCHER_MAIN:= main.o
ifndef DEMO
XPATCH_OBJS  :=xpatch/md5.o xpatch/loki_xdelta.o xpatch/apply_patch.o
endif
ALL_OBJECTS  := $(LAUNCHER_OBJS) $(LAUNCHER_MAIN) $(GUI_OBJS) $(XPATCH_OBJS)

$(BINARY): $(ALL_OBJECTS) $(XDDEPS)
	$(LINKER) -o $(BINARY) $(ALL_OBJECTS) $(LDFLAGS)

$(XDELTA_DIR)/libxdelta.a:
	$(MAKE) -C $(XDELTA_DIR) -f Makefile.xd CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" libxdelta.a

clean:
	-rm -f *.o xpatch/*.o core

cleaner:
	-rm -f *.o xpatch/*.o core $(BINARY)

