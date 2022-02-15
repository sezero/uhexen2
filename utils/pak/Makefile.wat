# makefile to build hexen2 pak tools for Win32 using Open Watcom:
#   wmake -f Makefile.wat

# PATH SETTINGS:
!ifndef __UNIX__
PATH_SEP=\
UHEXEN2_TOP=..\..
UTILS_TOP=..
COMMONDIR=$(UTILS_TOP)\common
UHEXEN2_SHARED=$(UHEXEN2_TOP)\common
LIBS_DIR=$(UHEXEN2_TOP)\libs
OSLIBS=$(UHEXEN2_TOP)\oslibs
!else
PATH_SEP=/
UHEXEN2_TOP=../..
UTILS_TOP=..
COMMONDIR=$(UTILS_TOP)/common
UHEXEN2_SHARED=$(UHEXEN2_TOP)/common
LIBS_DIR=$(UHEXEN2_TOP)/libs
OSLIBS=$(UHEXEN2_TOP)/oslibs
!endif

# Names of the binaries
PAKX=pakx.exe
PAKLIST=paklist.exe

# Compiler flags
CFLAGS = -zq -wx -bm -bt=nt -5s -sg -otexan -fp5 -fpi87 -ei -j -zp8
# newer OpenWatcom versions enable W303 by default
CFLAGS+= -wcd=303
!ifdef DEBUG
CFLAGS+= -d2
!else
CFLAGS+= -DNDEBUG=1
!endif
CFLAGS+= -DWIN32_LEAN_AND_MEAN

INCLUDES= -I. -I$(COMMONDIR) -I$(UHEXEN2_SHARED)

#############################################################

.c: $(COMMONDIR);$(UHEXEN2_SHARED)

.c.obj:
	wcc386 $(INCLUDES) $(CFLAGS) -fo=$^@ $<

# Objects
OBJ_COMMON= qsnprint.obj &
	strlcat.obj &
	strlcpy.obj &
	cmdlib.obj &
	util_io.obj &
	crc.obj &
	q_endian.obj &
	byteordr.obj &
	pakfile.obj
OBJ_PAKX= pakx.obj
OBJ_PAKL= paklist.obj

all: $(PAKX) $(PAKLIST)

$(PAKX): $(OBJ_COMMON) $(OBJ_PAKX)
	wlink N $@ SYS NT OP q F {$(OBJ_COMMON) $(OBJ_PAKX)}

$(PAKLIST): $(OBJ_COMMON) $(OBJ_PAKL)
	wlink N $@ SYS NT OP q F {$(OBJ_COMMON) $(OBJ_PAKL)}

INCLUDES+= -I"$(OSLIBS)/windows/misc/include"
clean: .symbolic
	rm -f *.obj *.res *.err
distclean: clean .symbolic
	rm -f $(PAKX) $(PAKLIST)
