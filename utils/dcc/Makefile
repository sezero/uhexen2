ifndef CC
CC = gcc
endif
LINKER = $(CC)
BINARY=dhcc
DESTDIR= ./bin
#CFLAGS = -O2 -Wall
CFLAGS = -O2
LDFLAGS = 
LDLIBS= -lm

all : $(BINARY)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

DHCCOBJS= cmdlib.o \
	pr_comp.o \
	pr_lex.o \
	dcc.o \
	hcc.o

$(BINARY) : $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(LDFLAGS) $(LDLIBS) $(DHCCOBJS)
