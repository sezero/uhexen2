CC = gcc
LINKER = gcc
BINARY=hcc
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


HCCOBJS= cmdlib.o \
	expr.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	stmt.o

$(BINARY) : $(HCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(LDFLAGS) $(LDLIBS) $(HCCOBJS)
