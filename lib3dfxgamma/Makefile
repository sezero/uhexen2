# GNU Makefile for lib3dfxgamma using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/lib3dfxgamma/Makefile,v 1.7 2007-07-06 11:01:58 sezero Exp $
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
LDFLAGS:= -ldl -shared
LDLIBS:=

all : lib3dfxgamma.so

clean : 
	rm -f *.o *.so core

lib3dfxgamma.o:
	$(CC) -c $(CFLAGS) -o lib3dfxgamma.o fx_gamma.c

lib3dfxgamma.so : lib3dfxgamma.o
	$(LINKER) lib3dfxgamma.o $(LDFLAGS) $(LDLIBS) -o lib3dfxgamma.so

