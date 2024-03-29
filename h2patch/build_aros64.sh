#!/bin/sh

# for x86_64-AROS cross-builds

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs.aros64

if [ "$1" = "strip" ]; then
	$STRIPPER -S h2patch
	exit 0
fi

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake ;;
linux)	MAKE_CMD=make ;;
*)	MAKE_CMD=make ;;
esac

exec $MAKE_CMD -f Makefile.amiga NOIXEMUL=0 AROS_X64=1 $*

