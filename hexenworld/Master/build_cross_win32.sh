#!/bin/sh

PREFIX=/usr/local/cross-tools
TARGET=i386-mingw32msvc
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH
MAKEFILE=Makefile.mingw

exec make -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*
