#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
	echo "Stripping binaries"
	$STRIPPER ../bin/qbsp.exe ../bin/bspinfo.exe ../bin/light.exe ../bin/vis.exe ../bin/bsp2wal.exe
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

if [ "$1" = "clean" ]
then
	$MAKE_CMD -s clean
	exit 0
fi

exec $MAKE_CMD $SENDARGS $*

