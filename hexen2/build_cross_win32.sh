#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER h2.exe glh2.exe h2ded.exe
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

if [ "$1" = "h2ded" ]
then
$MAKE_CMD $SENDARGS -f Makefile.sv $2 $3 $4
exit 0
fi

if [ "$1" = "all" ]
then
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 h2
$MAKE_CMD clean
$MAKE_CMD $SENDARGS $2 $3 $4 glh2
$MAKE_CMD clean
$MAKE_CMD $SENDARGS -f Makefile.sv $2 $3 $4
$MAKE_CMD clean
exit 0
fi

exec $MAKE_CMD $SENDARGS $*

