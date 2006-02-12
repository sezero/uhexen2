#!/bin/sh

# Valid arguments:
# clean : performs a "make clean" operation in all utils directories.
# strip : performs a "make strip" in all utils directories (strips all pre-built binaries)

PREFIX=/usr/local/cross-tools
TARGET=i386-mingw32msvc
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH

MAKEFILE=Makefile.mingw

make -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $*
