DEBUG=no

BINARY=hwmaster
BINARY_STATIC=hwmaster.static

CC = gcc
LINKER = gcc
CFLAGS = -I. -O2 -march=i586 -DPLATFORM_UNIX -Wall
ifeq "$(DEBUG)" "yes"
CFLAGS += -g
endif
LDFLAGS = -lm -lpthread
LDSTATIC = -lm -lpthread -static

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

MASTEROBJS = cmds.o net_test.o net.o

all: $(BINARY) 

$(BINARY) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(MASTEROBJS)
	
dynamic : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(LDFLAGS) $(MASTEROBJS)

$(BINARY_STATIC) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY_STATIC) $(MASTEROBJS) $(LDSTATIC)
	
static : $(MASTEROBJS)
	$(LINKER) -o $(BINARY_STATIC) $(MASTEROBJS) $(LDSTATIC)

clean:
	rm -f $(MASTEROBJS) core

