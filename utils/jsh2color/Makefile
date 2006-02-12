
ifndef CC
CC = gcc
endif
LINKER = $(CC)

BINARY=jsh2colour

CFLAGS = -march=i586 -O2 -Wall -ffast-math -fexpensive-optimizations
LDFLAGS = 
LDLIBS= -lm

all : $(BINARY)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


JS_OBJS= bspfile.o \
	cmdlib.o \
	entities.o \
	ltface.o \
	mathlib.o \
	tex.o \
	threads.o \
	trace.o \
	tyrlite.o

$(BINARY) : $(JS_OBJS)
	$(LINKER) -o $(BINARY) $(JS_OBJS) $(LDFLAGS) $(LDLIBS)

