#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

BIN_DIR=../bin

if test "$1" = "strip"; then
	echo "Stripping binaries"
	$STRIPPER $BIN_DIR/qbsp.exe	\
		$BIN_DIR/bspinfo.exe	\
		$BIN_DIR/light.exe $BIN_DIR/vis.exe
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

if test "$1" = "clean"; then
	$MAKE_CMD -s clean
	exit 0
fi

exec $MAKE_CMD $*

