#!/bin/sh

# Change this script to meet your needs and/or environment.

#TARGET=x86_64-pc-mingw32
TARGET=x86_64-w64-mingw32
PREFIX=/opt/cross_win64

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

