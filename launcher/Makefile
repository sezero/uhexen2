# GNU Makefile for hexen2 gtk-launcher app using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/launcher/Makefile,v 1.42 2007-07-10 18:48:19 sezero Exp $
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
UHEXEN2_TOP=..

# whether we should use the gnu auto-tools support when
# building xdelta libraries:
AUTOTOOLS ?= 0

HOST_OS:=$(shell uname)

CC_DEFAULT:=gcc
ifeq ($(origin CC),environment)
CC_OVERRIDE:=1
ifeq ($(CC),)
CC_OVERRIDE:=
endif
endif
ifeq ($(origin CC),command line)
CC_OVERRIDE:=1
endif
ifeq ($(CC_OVERRIDE),)
CC := $(CC_DEFAULT)
endif

LINKER:= $(CC)

XDELTA_DIR := $(UHEXEN2_TOP)/xdelta11
XDELTA_FLAGS:=
XDELTA_LINK:= -L$(XDELTA_DIR)/.libs -lxdelta -L$(XDELTA_DIR)/libedsio/.libs -ledsio

ifdef DEMO
TMPNAME:= h2demo
else
TMPNAME:= h2launcher
endif

ifdef GTK1
H2LAUNCH:= $(TMPNAME).gtk1
GTK_CFLAGS:= $(shell gtk-config --cflags)
GTK_LIBS:= $(shell gtk-config --libs)
XDELTA_FLAGS:=-D_XD_USE_GLIB1
else
H2LAUNCH:= $(TMPNAME)
GTK_CFLAGS:= $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:= $(shell pkg-config --libs gtk+-2.0)
endif

ifdef DEBUG
CFLAGS := -Wall -g -DDEBUG_BUILD
else
CFLAGS := -Wall -O2
endif

CFLAGS := $(CFLAGS) $(GTK_CFLAGS)

ifdef DEMO
CFLAGS := $(CFLAGS) -DDEMOBUILD
else
CFLAGS := $(CFLAGS) $(XDELTA_FLAGS) -I$(XDELTA_DIR) -I$(XDELTA_DIR)/libedsio -I. -I./xpatch
endif
ifdef GTK1
CFLAGS := $(CFLAGS) -D_H2L_USE_GTK1
endif
ifeq ($(HOST_OS),SunOS)
CFLAGS := $(CFLAGS) -D__SUNOS__
endif

LDFLAGS:=$(GTK_LIBS)
ifndef DEMO
LDFLAGS:= $(LDFLAGS) -lz -lpthread
ifneq ($(AUTOTOOLS),0)
LDFLAGS:=-Wl,-Bstatic $(XDELTA_LINK) -Wl,-Bdynamic $(LDFLAGS)
endif
endif

# activate the line below if you activate NLS support
#SUPPORT_OBJ= support.o

ifdef DEMO
XPATCH_OBJ :=
LIBXDELTA_OBJS:=
LIBEDSIO_OBJS :=
else
LIBEDSIO_OBJS:= $(XDELTA_DIR)/libedsio/library.o \
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
#		libedsio/sha.o \

LIBXDELTA_OBJS:=$(XDELTA_DIR)/xdelta.o \
		$(XDELTA_DIR)/xdapply.o \
		$(XDELTA_DIR)/xd_edsio.o

XPATCH_OBJ := xpatch/md5.o xpatch/loki_xdelta.o xpatch/apply_patch.o
endif

# Targets
all: $(H2LAUNCH)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Objects
LAUNCHER_OBJS = crc.o games.o config_file.o launch_bin.o interface.o \
		$(XPATCH_OBJ) $(SUPPORT_OBJ) main.o

ifeq ($(AUTOTOOLS),0)
ALL_OBJECTS:= $(LAUNCHER_OBJS) $(LIBXDELTA_OBJS) $(LIBEDSIO_OBJS)
else
ALL_OBJECTS:= $(LAUNCHER_OBJS)
endif

$(H2LAUNCH): $(LAUNCHER_OBJS)
	@rm -f $(H2LAUNCH)
	$(LINKER) -o $(H2LAUNCH) $(ALL_OBJECTS) $(LDFLAGS)

clean:
	-rm -f *.o xpatch/*.o core

cleaner:
	-rm -f *.o xpatch/*.o core $(H2LAUNCH)

