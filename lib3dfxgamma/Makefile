# quick'n'dirty (and silly) makefile for lib3dfxgamma.so
# sigh, do we even need one for this!?

CC ?= gcc
LINKER = $(CC)
FXLIB=lib3dfxgamma.so
CFLAGS = -O2 -Wall -shared
LDFLAGS = 
LDLIBS=

all : $(FXLIB)

clean : 
	rm -f *.o *.so core

FXSRC=lib3dfxgamma.c

$(FXLIB) : $(FXSRC)
	$(LINKER) -o $(FXLIB) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(FXSRC)
