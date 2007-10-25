# GNU Makefile for lib3dfxgamma using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/lib3dfxgamma/Makefile,v 1.8 2007-10-25 09:06:52 sezero Exp $
#

CC ?= gcc
ifeq ($(CC),)
CC := gcc
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

