# GNU Makefile for hexen2 gtk-launcher app using GCC.
# $Id$
#
#  gtk2 will be used default.
#  to compile for gtk-1.2 type:  make GTK1=yes
#
#  if building for the demo version, run with
#  make DEMO=yes [other stuff]
#
#  if building a debug version:  make DEBUG=yes
#

# main uhexen2 relative path
UHEXEN2_TOP:=..
LIBS_DIR:=$(UHEXEN2_TOP)/libs

HOST_OS:=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

CC ?= gcc
AR ?= ar
RANLIB ?= ranlib

LINKER= $(CC)

BINARY:= h2launcher

MD5DIR:= $(LIBS_DIR)/md5
MD5FLAGS:= -I$(MD5DIR) -I$(LIBS_DIR)/common

XDELTA_DIR:= $(LIBS_DIR)/xdelta3
XDFLAGS:= -DXD3_DEBUG=0
XDFLAGS+= -I$(XDELTA_DIR) -I$(LIBS_DIR)/common
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
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-2.0)
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
MD5FLAGS+=$(CFLAGS)
XDFLAGS +=$(CFLAGS)

CFLAGS += -I. -I$(LIBS_DIR)/common
CFLAGS += $(PTHREAD_CFLAGS)
LDFLAGS+= $(PTHREAD_LIBS)

ifdef DEMO
CFLAGS += -DDEMOBUILD
else
CFLAGS += -I$(XDELTA_DIR) -I$(MD5DIR)
endif

# Targets
all: $(BINARY)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Objects
# activate the line below if you add pixmap support, etc
#SUPPORT_OBJS := support.o
GUI_OBJS     := gtk_ui.o $(SUPPORT_OBJS)
LAUNCHER_OBJS:= crc.o q_endian.o games.o config_file.o launch_bin.o
LAUNCHER_MAIN:= main.o
ifndef DEMO
XPATCH_OBJS  := apply_patch.o md5.o xdelta3.o
endif
ALL_OBJECTS  := $(LAUNCHER_OBJS) $(LAUNCHER_MAIN) $(XPATCH_OBJS) $(GUI_OBJS)

# Targets
.PHONY: clean distclean

$(BINARY): $(ALL_OBJECTS)
	$(LINKER) -o $(BINARY) $(ALL_OBJECTS) $(GTK_LIBS) $(LDFLAGS)

MD5DEPS:= md5.c md5.h
MD5DEPS:= $(patsubst %,$(MD5DIR)/%,$(MD5DEPS))

XD3DEPS:= xdelta3-decode.h xdelta3-list.h xdelta3-main.h xdelta3-blkcache.h \
	xdelta3.c xdelta3.h
XD3DEPS:= $(patsubst %,$(XDELTA_DIR)/%,$(XD3DEPS))

md5.o: $(MD5DEPS)
	$(CC) $(MD5FLAGS) -c $(MD5DIR)/md5.c -o md5.o

xdelta3.o: $(XD3DEPS)
	$(CC) $(XDFLAGS) -c $(XDELTA_DIR)/xdelta3.c -o xdelta3.o

gtk_ui.o: gtk_ui.c gtk_ui.h launcher_defs.h launcher_ui.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c gtk_ui.c -o gtk_ui.o

clean:
	-rm -f *.o *.o core
distclean: clean
	-rm -f $(BINARY)

