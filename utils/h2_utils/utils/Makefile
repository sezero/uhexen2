
CC = gcc
LINKER = gcc

QBSP=qbsp
LIGHT=light
VIS=vis

DESTDIR=../bin
CFLAGS = 
LDFLAGS = 
LDLIBS= -lm

all : $(LIGHT) $(VIS) $(QBSP)

clean : 
	rm -f *.o core

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)


COMMONOBJS= cmdlib.o \
	mathlib.o \
	bspfile.o

LIGHTOBJS= entities.o \
	light.o \
	ltface.o \
	threads.o \
	trace.o

VISOBJS= flow.o \
	soundpvs.o \
	vis.o

QBSPOBJS= brush.c \
	csg4.o \
	map.o \
	merge.o \
	nodraw.o \
	outside.o \
	portals.o \
	qbsp.o \
	region.o \
	solidbsp.o \
	surfaces.o \
	tjunc.o \
	writebsp.o


$(LIGHT) : $(COMMONOBJS) $(LIGHTOBJS)
	$(LINKER) -o $(DESTDIR)/$(LIGHT) $(LDFLAGS) $(LDLIBS) $(COMMONOBJS) $(LIGHTOBJS)

$(VIS) : $(COMMONOBJS) $(VISOBJS)
	$(LINKER) -o $(DESTDIR)/$(VIS) $(LDFLAGS) $(LDLIBS) $(COMMONOBJS) $(VISOBJS)

$(QBSP) : $(COMMONOBJS) $(QBSPOBJS)
	$(LINKER) -o $(DESTDIR)/$(QBSP) $(LDFLAGS) $(LDLIBS) $(COMMONOBJS) $(QBSPOBJS)

