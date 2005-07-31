DEBUG=no

BINARY=hwmaster
BINARY_STATIC=hwmaster.static

ifndef CC
CC = gcc
endif
LINKER = $(CC)

ifeq "$(DEBUG)" "yes"
CFLAGS = -I. -DDEBUG_BUILD -g -Wall -DPLATFORM_UNIX
else
CFLAGS = -I. -O2 -march=i586 -Wall -DPLATFORM_UNIX
endif

LDFLAGS = -lm -lpthread
LDSTATIC = -lm -lpthread -static

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

MASTEROBJS = cmds.o net_test.o net.o

all: $(BINARY)
static: $(BINARY_STATIC)

$(BINARY) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY) $(MASTEROBJS) $(LDFLAGS)

$(BINARY_STATIC) : $(MASTEROBJS)
	$(LINKER) -o $(BINARY_STATIC) $(MASTEROBJS) $(LDSTATIC)

clean:
	rm -f $(MASTEROBJS) core

