###########################################################################
#
# Makefile for the DirectX libraries, using dlltool and definition files.
#
###########################################################################
#
# Some consistent rules for building libraries:

.SUFFIXES: .def

.def.a:
	dlltool -k --dllname `echo $< | sed 's/lib\(.*\).def/\1.dll/'` \
		--output-lib `echo $< | sed 's/.def/.a/'` --def $<

###########################################################################

CFLAGS = -I../inc
LIBS = libddraw.a libdsound.a libdinput.a libdplayx.a libdxguid.a

all: $(LIBS)

libdinput.a: dinput.o
	dlltool -k --dllname dinput.dll --output-lib $@ --def libdinput.def
	ar rv $@ $<
	rm -f $<
	
libdxguid.a: dxguid.o
	ar rv $@ $<
	rm -f $<

clean:

distclean: clean
	rm -f $(LIBS)

