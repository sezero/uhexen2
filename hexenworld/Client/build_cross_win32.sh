#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER hwcl.exe glhwcl.exe
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

if [ "$1" = "all" ]
then
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 glhw || exit 1
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 hw || exit 1
$MAKE_CMD clean
exit 0
fi

exec $MAKE_CMD $SENDARGS $*

