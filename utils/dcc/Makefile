ifndef CC
CC = gcc
endif
LINKER = $(CC)
BINARY=dhcc
DESTDIR= ./bin
CFLAGS = -O2 -Wall
#CFLAGS+= -D_GNU_SOURCE
LDFLAGS = 
LDLIBS= 

all : $(BINARY)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

DHCCOBJS= cmdlib.o \
	pr_comp.o \
	pr_lex.o \
	dcc.o \
	hcc.o

$(BINARY) : $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(DHCCOBJS) $(LDFLAGS) $(LDLIBS)
