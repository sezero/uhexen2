#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER hwcl.exe glhwcl.exe
exit 0
fi

HOST_OS=`uname`

case "$HOST_OS" in
FreeBSD|OpenBSD|NetBSD)
	MAKE_CMD=gmake
	;;
Linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

if [ "$1" = "all" ]
then
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 glhw
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 hw
$MAKE_CMD clean
exit 0
fi

exec $MAKE_CMD $SENDARGS $*

