#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs_w64

if test "$1" = "strip"; then
	$STRIPPER glh2.exe h2ded.exe
	exit 0
fi

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

case "$HOST_OS" in
linux)
	MAKE_CMD=make
	;;
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake
	;;
*)
	MAKE_CMD=make
	;;
esac

if test "$1" = "h2" -o "$1" = "h2.exe" -o "$1" = "hexen2"; then
	echo "Software renderer h2.exe isn't supported for Win64 builds yet.."
	exit 1
fi

if test "$1" = "h2ded"; then
	$MAKE_CMD -f Makefile.sv $2 $3 $4 $5 $6 || exit 1
	exit 0
fi

if test "$1" = "all"; then
	$MAKE_CMD clean
	$MAKE_CMD -f Makefile.sv $2 $3 $4 $5 $6 || exit 1
	$MAKE_CMD clean
	$MAKE_CMD $2 $3 $4 $5 $6 glh2 || exit 1
	$MAKE_CMD clean
#	$MAKE_CMD $2 $3 $4 $5 $6 h2  || exit 1
#	$MAKE_CMD clean
	exit 0
fi

exec $MAKE_CMD $*

