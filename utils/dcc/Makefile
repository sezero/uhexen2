# GNU Makefile for hexen2 dhcc tool using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/dcc/Makefile,v 1.28 2007-10-18 16:55:53 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the WINBUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP=../..
# where the common sources/objects are
COMMONDIR=../common
# where to put the binaries
DESTDIR =../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# include file for sanity checks
include $(UHEXEN2_TOP)/scripts/sanity1.inc

# Names of the binaries
ifeq ($(TARGET_OS),win32)
BINARY=dhcc.exe
else
BINARY=dhcc
endif

# Compiler flags

CPUFLAGS:=
# Overrides for the default CPUFLAGS
# NOTE: the i586 cpuflag is causing a ~two-fold~ slowdown!
#CPUFLAGS:=

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG
CFLAGS := -g -Wall
else
CFLAGS := $(CPUFLAGS) -O2 -Wall
endif

# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(MINGWDIR)/include -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
else
INCLUDES:= -I$(COMMONDIR)
LDFLAGS :=
LDLIBS  :=
endif

# Targets
all : $(BINARY)

# directory for common objects (our flags may differ
# from the flags that common objects were compiled.
COMLIB_DIR:=cmnlibs

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<
$(COMLIB_DIR)/%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(ARCHFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJ= $(COMLIB_DIR)/cmdlib.o \
	$(COMLIB_DIR)/util_io.o \
	$(COMLIB_DIR)/q_endian.o \
	$(COMLIB_DIR)/crc.o
DHCCOBJS= dcc.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	pr_parse.o

# include file for sanity check target
include $(UHEXEN2_TOP)/scripts/sanity2.inc

$(COMLIB_DIR):
	@test -d $@ || mkdir -p $@

$(BINARY) : $(COMLIB_DIR) $(COMMONOBJ) $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(COMMONOBJ) $(DHCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMLIB_DIR)/*.o core
	rm -rf $(COMLIB_DIR)

