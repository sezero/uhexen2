# GNU Makefile for hexen2 dhcc tool using GCC.
# $Header: /home/ozzie/Download/0000/uhexen2/utils/dcc/Makefile,v 1.36 2008-12-22 12:58:07 sezero Exp $
#
# if building a debug version :		make DEBUG=1 [other stuff]
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../..
# where the common sources/objects are
COMMONDIR:=../common
# where to put the binaries
DESTDIR  :=../bin

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
BINARY:=dhcc$(exe_ext)

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

CFLAGS := $(CFLAGS) $(ARCHFLAGS)


# Other build flags

ifeq ($(TARGET_OS),win32)
INCLUDES:= -I$(W32STUFF) -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),win64)
INCLUDES:= -I$(W32STUFF) -I$(COMMONDIR)
LDFLAGS := -mconsole
LDLIBS  :=
endif
ifeq ($(TARGET_OS),unix)
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
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(COMLIB_DIR)/%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# Objects
COMMONOBJ= $(COMLIB_DIR)/cmdlib.o \
	$(COMLIB_DIR)/strlcat.o \
	$(COMLIB_DIR)/strlcpy.o \
	$(COMLIB_DIR)/util_io.o \
	$(COMLIB_DIR)/q_endian.o \
	$(COMLIB_DIR)/byteordr.o \
	$(COMLIB_DIR)/crc.o
DHCCOBJS= dcc.o \
	hcc.o \
	pr_comp.o \
	pr_lex.o \
	pr_parse.o

$(COMLIB_DIR):
	@test -d $@ || mkdir -p $@

$(BINARY) : $(COMLIB_DIR) $(COMMONOBJ) $(DHCCOBJS)
	$(LINKER) -o $(DESTDIR)/$(BINARY) $(COMMONOBJ) $(DHCCOBJS) $(LDFLAGS) $(LDLIBS)

clean : 
	rm -f *.o $(COMLIB_DIR)/*.o core
	rm -rf $(COMLIB_DIR)

