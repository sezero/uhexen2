# GNU Makefile for the H2PATCH tool - Unix platforms.
#
# To build a debug version:		make DEBUG=yes
#

UHEXEN2_TOP:=..
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

CC ?= gcc
ifeq ($(origin CC),default)
# refuse the stupid 'cc' default of make
# which not necessarily exist everywhere
CC  = gcc
endif
LINKER = $(CC)

CPUFLAGS=
CFLAGS  = -Wall
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1
else
CFLAGS += -g
endif
LDFLAGS =

XDELTA_DIR:= $(LIBS_DIR)/xdelta3
XDFLAGS:= -DXD3_DEBUG=0
# make xdelta3 to use stdio:
XDFLAGS+= -DXD3_STDIO=1
XDFLAGS+= -I$(XDELTA_DIR) -I$(UHEXEN2_SHARED)
XDFLAGS+= $(CFLAGS)

CFLAGS  += -I. -I$(XDELTA_DIR) -I$(UHEXEN2_SHARED)

TARGETS:= h2patch

# Targets
.PHONY: clean distclean

all: $(TARGETS)

OBJECTS:= xdelta3.o qsnprint.o h2patch3.o

XD3DEPS:= xdelta3-decode.h xdelta3-list.h xdelta3-main.h xdelta3-blkcache.h \
	xdelta3.c xdelta3.h
XD3DEPS:= $(patsubst %,$(XDELTA_DIR)/%,$(XD3DEPS))

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o: $(UHEXEN2_SHARED)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

xdelta3.o: $(XD3DEPS)
	$(CC) $(XDFLAGS) -c $(XDELTA_DIR)/xdelta3.c -o xdelta3.o

h2patch: $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -f *.o *.res
distclean: clean
	rm -f $(TARGETS)

