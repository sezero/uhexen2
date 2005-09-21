#!/bin/sh

# Note:
# Do NOT use clean or strip as an argument for this script.
# This script already performs stripping. As for clean, use
# the clean_win32.sh script, instead.

PREFIX=/usr/local/cross-tools
TARGET=i386-mingw32msvc
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH

MAKEFILE=Makefile.mingw

echo "Building HexenWorld Server"
make -C Server -f $MAKEFILE WIN32CC=$TARGET-gcc MINGWDIR=$PREFIX/$TARGET $*

echo "" && echo "Building HexenWorld Master Server"
make -C Master -f $MAKEFILE WIN32CC=$TARGET-gcc MINGWDIR=$PREFIX/$TARGET $*

echo "" && echo "Building HexenWorld Client (Software renderer)"
make -C Client -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres MINGWDIR=$PREFIX/$TARGET $* hw_exe

echo "" && echo "Building HexenWorld Client (OpenGL renderer)"
make -C Client -f $MAKEFILE clean
make -C Client -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres MINGWDIR=$PREFIX/$TARGET $* glhw_exe

echo "" && echo "Stripping all HexenWorld binaries"
make -C Server -f $MAKEFILE W32STRIP=$TARGET-strip strip
make -C Master -f $MAKEFILE W32STRIP=$TARGET-strip strip
make -C Client -f $MAKEFILE W32STRIP=$TARGET-strip strip

