#!/bin/sh

# Change this script to meet your needs and/or environment.

#TARGET=i386-mingw32msvc
#TARGET=i686-w64-mingw32
TARGET=i686-pc-mingw32
#PREFIX=/opt/cross_win32
PREFIX=/usr/local/cross-win32

PATH="$PREFIX/bin:$PATH"
export PATH

W32BUILD=1

CC="$TARGET-gcc"
AS="$TARGET-as"
RANLIB="$TARGET-ranlib"
AR="$TARGET-ar"
WINDRES="$TARGET-windres"
export CC AS RANLIB AR WINDRES

STRIPPER="$TARGET-strip"

if [ "$1" = "strip" ]
then
$STRIPPER h2patch.exe
exit 0
fi

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake
	;;
linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

exec $MAKE_CMD -f Makefile.mingw32 $*

