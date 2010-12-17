#!/bin/sh

# Change this script to meet your needs and/or environment.

#TARGET=i386-mingw32msvc
#TARGET=i686-w64-mingw32
TARGET=i686-pc-mingw32
#PREFIX=/opt/cross_win32
PREFIX=/usr/local/cross-win32

PATH="$PREFIX/bin:$PATH"
export PATH

CC="$TARGET-gcc"
AS="$TARGET-as"
AR="$TARGET-ar"
RANLIB="$TARGET-ranlib"
export CC AS AR RANLIB

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

exec $MAKE_CMD $*

