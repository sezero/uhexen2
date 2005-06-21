
ifndef CC
CC = gcc
endif
LINKER = $(CC)
BINARY=genmodel
DESTDIR=../bin
CFLAGS = -O2 -Wall
LDFLAGS = 
LDLIBS= -lm

all : $(BINARY)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


GENMODELOBJS= cmdlib.o \
	mathlib.o \
	loadtri.o \
	scriplib.o \
	token.o \
	genmodel.o

$(BINARY) : $(GENMODELOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(GENMODELOBJS) $(LDFLAGS) $(LDLIBS)
