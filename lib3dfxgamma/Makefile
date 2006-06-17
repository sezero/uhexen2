
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
ifndef CC_OVERRIDE
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

