
CC = gcc
LINKER = gcc
BINARY=qfiles
DESTDIR=../bin
CFLAGS = 
LDFLAGS =
LDLIBS = 

all : $(BINARY)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)


QFILESOBJS= cmdlib.o \
	qfiles.o

$(BINARY) : $(QFILESOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(LDFLAGS) $(LDLIBS) $(QFILESOBJS)
