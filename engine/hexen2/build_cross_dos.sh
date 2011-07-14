#!/bin/sh

# build h2dos.exe with djgpp cross toolchain

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
	MAKE_CMD=gmake
	;;
linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

if test "$1" = "h2ded"; then
	$MAKE_CMD -f Makefile.sv $2 $3 $4 $5 $6 || exit 1
	exit 0
fi
if test "$1" = "all"; then
	$MAKE_CMD clean
	$MAKE_CMD $2 $3 $4 $5 $6 h2  || exit 1
	$MAKE_CMD clean
	$MAKE_CMD -f Makefile.sv $2 $3 $4 $5 $6 || exit 1
	$MAKE_CMD clean
	exit 0
fi

exec $MAKE_CMD $*

