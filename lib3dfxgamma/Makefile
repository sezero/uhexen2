# GNU Makefile for lib3dfxgamma using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/lib3dfxgamma/Makefile,v 1.6 2007-05-15 14:25:16 sezero Exp $
#

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
CFLAGS:= -O2 -fPIC -Wall
LDFLAGS:= -shared
LDLIBS:=

all : lib3dfxgamma.so

clean : 
	rm -f *.o *.so core

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

lib3dfxgamma.so : lib3dfxgamma.o
	$(LINKER) lib3dfxgamma.o $(LDFLAGS) $(LDLIBS) -o lib3dfxgamma.so

