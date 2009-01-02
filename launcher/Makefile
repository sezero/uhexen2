# GNU Makefile for hexen2 gtk-launcher app using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/launcher/Makefile,v 1.51 2009-01-02 15:55:15 sezero Exp $
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

# whether we should use the gnu auto-tools support when
# building xdelta libraries: 0 or 1.
AUTOTOOLS ?= 0

HOST_OS:=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

CC ?= gcc
ifeq ($(CC),)
CC := gcc
endif

LINKER:= $(CC)

XDELTA_DIR := $(UHEXEN2_TOP)/xdelta11
XDELTA_FLAGS:=
XDELTA_LINK:= -L$(XDELTA_DIR)/.libs -lxdelta -L$(XDELTA_DIR)/libedsio/.libs -ledsio

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
PTHREAD_CFLAGS:=-D_REENTRANT
PTHREAD_LIBS:=-lpthread

ifdef DEBUG
CFLAGS := -Wall -g -DDEBUG=1 -DDEBUG_BUILD=1
else
CFLAGS := -Wall -O2 -DNDEBUG
endif

CFLAGS := $(CFLAGS) $(GTK_CFLAGS)

ifdef DEMO
CFLAGS := $(CFLAGS) -DDEMOBUILD
else
CFLAGS := $(CFLAGS) $(PTHREAD_CFLAGS) $(XDELTA_FLAGS) -I$(XDELTA_DIR) -I$(XDELTA_DIR)/libedsio -I. -I./xpatch
endif

LDFLAGS:=$(GTK_LIBS)
ifndef DEMO
LDFLAGS:= $(LDFLAGS) -lz $(PTHREAD_LIBS)
ifeq ($(AUTOTOOLS),1)
LDFLAGS:=-Wl,-Bstatic $(XDELTA_LINK) -Wl,-Bdynamic $(LDFLAGS)
endif
endif

ifdef DEMO
XPATCH_OBJS   :=
LIBXDELTA_OBJS:=
LIBEDSIO_OBJS :=

else
ifeq ($(AUTOTOOLS),0)
LIBEDSIO_OBJS :=$(XDELTA_DIR)/libedsio/library.o \
		$(XDELTA_DIR)/libedsio/simple.o \
		$(XDELTA_DIR)/libedsio/edsio.o \
		$(XDELTA_DIR)/libedsio/edsio_edsio.o \
		$(XDELTA_DIR)/libedsio/md5c.o \
		$(XDELTA_DIR)/libedsio/fh.o \
		$(XDELTA_DIR)/libedsio/generic.o \
		$(XDELTA_DIR)/libedsio/default.o \
		$(XDELTA_DIR)/libedsio/base64.o \
		$(XDELTA_DIR)/libedsio/maketime.o \
		$(XDELTA_DIR)/libedsio/partime.o
# SHA is unused in present code, removed it.
#		$(XDELTA_DIR)/libedsio/sha.o \

LIBXDELTA_OBJS:=$(XDELTA_DIR)/xdelta.o \
		$(XDELTA_DIR)/xdapply.o \
		$(XDELTA_DIR)/xd_edsio.o
else
LIBXDELTA_OBJS:=
LIBEDSIO_OBJS :=
endif

XPATCH_OBJS   :=xpatch/md5.o xpatch/loki_xdelta.o xpatch/apply_patch.o
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
ALL_OBJECTS  := $(LAUNCHER_OBJS) $(GUI_OBJS) $(XPATCH_OBJS) \
	$(LIBXDELTA_OBJS) $(LIBEDSIO_OBJS) $(LAUNCHER_MAIN)

$(BINARY): $(LAUNCHER_OBJS) $(LAUNCHER_MAIN) $(GUI_OBJS) $(XPATCH_OBJS)
	@rm -f $(BINARY)
	$(LINKER) -o $(BINARY) $(ALL_OBJECTS) $(LDFLAGS)

clean:
	-rm -f *.o xpatch/*.o core

cleaner:
	-rm -f *.o xpatch/*.o core $(BINARY)

