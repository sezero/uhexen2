#!/bin/sh

TARGET=i586-pc-msdosdjgpp
PREFIX=/usr/local/cross-djgpp

PATH="$PREFIX/bin:$PATH"
export PATH

DOSBUILD=1

CC="$TARGET-gcc"
AS="$TARGET-as"
RANLIB="$TARGET-ranlib"
AR="$TARGET-ar"

export CC AS RANLIB AR DOSBUILD

STRIPPER="$TARGET-strip"
if test "$1" = "strip"; then
	$STRIPPER h2dos.exe h2ded.exe
	exit 0
fi

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake ;;
linux)	MAKE_CMD=make ;;
*)	MAKE_CMD=make ;;
esac

if test "$1" = "h2ded"; then
	shift
	$MAKE_CMD -f Makefile.sv $* || exit 1
	exit 0
fi
if test "$1" = "all"; then
	shift
	$MAKE_CMD clean
	$MAKE_CMD h2 $*  || exit 1
	$MAKE_CMD clean
	$MAKE_CMD -f Makefile.sv $* || exit 1
	$MAKE_CMD clean
	exit 0
fi

exec $MAKE_CMD $*

