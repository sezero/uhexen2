
ifndef CC
CC = gcc
endif
LINKER = $(CC)
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
	$(CC) -c $(CFLAGS) -o $@ $<


QFILESOBJS= cmdlib.o \
	qfiles.o

$(BINARY) : $(QFILESOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(QFILESOBJS) $(LDFLAGS) $(LDLIBS)
